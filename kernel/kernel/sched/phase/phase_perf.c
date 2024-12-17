/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * phase_perf.c
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/kernel.h>
#include <linux/perf_event.h>
#include <linux/percpu-defs.h>
#include <linux/slab.h>
#include <linux/stop_machine.h>
#include <platform_include/cee/linux/phase.h>
#include <../kernel/sched/sched.h>
#include "phase_perf.h"
#include <linux/completion.h>

#define EVENT_TYPE_FLEXIBLE		0
#define EVENT_TYPE_PINNED		1

int *phase_perf_pevents = NULL;
int *phase_perf_fevents = NULL;

struct phase_cpu {
	struct task_struct *create_thread;
	struct task_struct *release_thread;
	struct kthread_worker create_worker;
	struct kthread_worker release_worker;
	struct kthread_work create_work;
	struct kthread_work release_work;
};

struct phase_cpu_done {
	atomic_t nr_todo; /* nr left to execute */
	int ret; /* collected return value */
	struct completion completion; /* fired if nr_todo reaches 0 */
};

struct phase_cpu_done g_create_done;
struct phase_cpu_done g_release_done;

static DEFINE_PER_CPU(struct phase_cpu, phase_cpu);
static DEFINE_PER_CPU(__typeof__(struct perf_event *)[PHASE_PEVENT_NUM], cpu_phase_perf_events_pinned);
static DEFINE_PER_CPU(__typeof__(struct perf_event *)[PHASE_FEVENT_NUM], cpu_phase_perf_events_flexible);

/* Helpers for phase perf event */
static inline struct perf_event *perf_event_of_cpu(int cpu, int index, int pinned)
{
	if (pinned)
		return per_cpu(cpu_phase_perf_events_pinned, cpu)[index];
	else
		return per_cpu(cpu_phase_perf_events_flexible, cpu)[index];
}

static inline struct perf_event **perf_events_of_cpu(int cpu, int pinned)
{
	if (pinned)
		return per_cpu(cpu_phase_perf_events_pinned, cpu);
	else
		return per_cpu(cpu_phase_perf_events_flexible, cpu);
}

/* Helpers for cpu counters */
static inline u64 read_cpu_counter(int cpu, int index, int pinned)
{
	struct perf_event *event = perf_event_of_cpu(cpu, index, pinned);

	if (!event || !event->pmu)
		return 0;

	return perf_event_local_pmu_read(event);
}

static struct perf_event_attr *alloc_attr(int event_id, int pinned)
{
	struct perf_event_attr *attr = NULL;

	attr = kzalloc(sizeof(struct perf_event_attr), GFP_KERNEL);
	if (!attr)
		return ERR_PTR(-ENOMEM);

	attr->type = PERF_TYPE_RAW;
	attr->config = (u64)event_id;
	attr->size = sizeof(struct perf_event_attr);
	attr->pinned = (u64)pinned;
	attr->disabled = 1;
	attr->exclude_hv = sysctl_phase_exclude_hv;
	attr->exclude_idle = sysctl_phase_exclude_idle;
	attr->exclude_kernel = sysctl_phase_exclude_kernel;

	return attr;
}

static int create_cpu_counter(int cpu, int event_id, int index, int pinned)
{
	struct perf_event_attr *attr = NULL;
	struct perf_event **events = perf_events_of_cpu(cpu, pinned);
	struct perf_event *event = NULL;

	attr = alloc_attr(event_id, pinned);
	if (IS_ERR(attr))
		return PTR_ERR(attr);

	event = perf_event_create_kernel_counter(attr, cpu, NULL, NULL, NULL);
	if (IS_ERR(event)) {
		pr_err("unable to create perf event (cpu:%i-type:%d-pinned:%d-config:0x%llx) : %ld",
		       cpu, attr->type, attr->pinned, attr->config, PTR_ERR(event));
		kfree(attr);
		return PTR_ERR(event);
	}

	events[index] = event;
	perf_event_enable(events[index]);
	if (pinned && event->hw.idx == -1) {
		pr_err("pinned event unable to get onto hardware, perf event (cpu:%i-type:%d-config:0x%llx)",
		       cpu, attr->type, attr->config);
		kfree(attr);
		return -EINVAL;
	}
	pr_debug("create perf_event (cpu:%i-idx:%d-type:%d-pinned:%d-exclude_hv:%d"
		 "-exclude_idle:%d-exclude_kernel:%d-config:0x%llx-addr:%llx)",
		 event->cpu, event->hw.idx,
		 event->attr.type, event->attr.pinned, event->attr.exclude_hv,
		 event->attr.exclude_idle, event->attr.exclude_kernel,
		 event->attr.config, event);

	kfree(attr);
	return 0;
}

static int create_cpu_group_counters(int cpu, int *event_ids, int num, int *index, int pinned)
{
	int err;
	int idx = *index;
	int event_id;
	struct perf_event *event = NULL;
	struct perf_event **events = perf_events_of_cpu(cpu, pinned);
	struct perf_event_attr *attr = NULL;

	/* Create group leader first */
	err = create_cpu_counter(cpu, event_ids[idx], idx, pinned);
	if (err) {
		pr_err("create phase perf group leader failed\n");
		return err;
	}

	/* Then the group members */
	for (idx = idx + 1; idx < num; idx++) {
		event_id = event_ids[idx];
		if (event_id == PHASE_EVENT_GROUP_TERMINATOR)
			break;

		attr = alloc_attr(event_id, pinned);
		if (IS_ERR(attr))
			return PTR_ERR(attr);

		event = perf_event_create_kernel_group_counter(attr, cpu, NULL, events[*index], NULL, NULL);
		if (IS_ERR(event)) {
			pr_err("unable to create perf event (cpu:%i-type:%d-pinned:%d-config:0x%llx) : %ld\n",
			       cpu, attr->type, attr->pinned, attr->config, PTR_ERR(event));
			kfree(attr);
			return PTR_ERR(event);
		}

		events[idx] = event;
		perf_event_enable(event);
		pr_debug("create perf_event (cpu:%i-idx:%d-type:%d-pinned:%d-exclude_hv:%d"
			 "-exclude_idle:%d-exclude_kernel:%d-config:0x%llx-leader:%llx)",
			 event->cpu, event->hw.idx,
			 event->attr.type, event->attr.pinned, event->attr.exclude_hv,
			 event->attr.exclude_idle, event->attr.exclude_kernel,
			 event->attr.config, event->group_leader);
		kfree(attr);
	}

	*index = idx;
	return 0;
}

static int release_cpu_counter(int cpu, int event_id, int index, int pinned)
{
	struct perf_event **events = perf_events_of_cpu(cpu, pinned);
	struct perf_event *event = NULL;

	event = events[index];
	if (!event)
		return 0;

	pr_debug("release perf_event (cpu:%i-idx:%d-type:%d-pinned:%d-exclude_hv:%d"
		 "-exclude_idle:%d-exclude_kernel:%d-config:0x%llx)\n",
		 event->cpu, event->hw.idx,
		 event->attr.type, event->attr.pinned, event->attr.exclude_hv,
		 event->attr.exclude_idle, event->attr.exclude_kernel,
		 event->attr.config);

	perf_event_release_kernel(event);
	events[index] = NULL;

	return 0;
}

static int release_cpu_group_counters(int cpu, int *event_ids, int num, int *index, int pinned)
{
	int idx = *index;
	int event_id;

	/* release group member events */
	for (idx = idx + 1; idx < num; idx++) {
		event_id = event_ids[idx];
		if (event_id == PHASE_EVENT_GROUP_TERMINATOR)
			break;
		release_cpu_counter(cpu, event_id, idx, pinned);
	}
	/* release group leader event */
	release_cpu_counter(cpu, event_id, *index, pinned);

	*index = idx;
	return 0;
}

/* Helpers for phase perf */
static int do_pevents(int (*fn)(int, int, int, int), int cpu)
{
	int index;
	int err;

	for_each_phase_pevents(index, phase_perf_pevents) {
		err = fn(cpu, phase_perf_pevents[index], index, EVENT_TYPE_PINNED);
		if (err)
			return err;
	}

	return 0;
}

static int do_fevents(int (*fn)(int, int*, int, int*, int), int cpu)
{
	int index;
	int err;

	for_each_phase_fevents(index, phase_perf_fevents) {
		err = fn(cpu, phase_perf_fevents, PHASE_FEVENT_NUM, &index, EVENT_TYPE_FLEXIBLE);
		if (err)
			return err;
	}

	return 0;
}

static void phase_init_done(struct phase_cpu_done *done, unsigned int nr_todo)
{
	(void)memset_s(done, sizeof(*done), 0, sizeof(*done));
	atomic_set(&done->nr_todo, nr_todo);
	init_completion(&done->completion);
}

static void __phase_perf_create(struct kthread_work *work)
{
	int err;
	int cpu = raw_smp_processor_id();

	/* create pinned events */
	err = do_pevents(create_cpu_counter, cpu);
	if (err) {
		pr_err("create pinned events failed\n");
		do_pevents(release_cpu_counter, cpu);
		goto out;
	}

	/* create flexible event groups */
	err = do_fevents(create_cpu_group_counters, cpu);
	if (err) {
		pr_err("create flexible events failed\n");
		do_fevents(release_cpu_group_counters, cpu);
		goto out;
	}

	pr_info("CPU%d phase class event create success\n", cpu);

out:
	g_create_done.ret += err;
	if (atomic_dec_and_test(&g_create_done.nr_todo))
		complete(&g_create_done.completion);
}

int phase_perf_create(int *pevents, int *fevents, const struct cpumask *cpus)
{
	int cpu;
	int ret;

	phase_perf_pevents = pevents;
	phase_perf_fevents = fevents;

	phase_init_done(&g_create_done, cpumask_weight(cpus));
	for_each_cpu(cpu, cpus) {
		struct phase_cpu *pa_cpu = &per_cpu(phase_cpu, cpu);

		ret = kthread_queue_work(&pa_cpu->create_worker, &pa_cpu->create_work);
		if (!ret) {
			pr_err("phase create kthread err %d\n", ret);
			return -ENOSYS;
		}
	}
	wait_for_completion(&g_create_done.completion);

	return g_create_done.ret;
}

static void __phase_perf_release(struct kthread_work *work)
{
	int cpu = raw_smp_processor_id();

	/* release pinned events */
	clear_cpu_phase_count(cpu);
	do_pevents(release_cpu_counter, cpu);

	/* release flexible event groups */
	do_fevents(release_cpu_group_counters, cpu);
	if (atomic_dec_and_test(&g_release_done.nr_todo))
		complete(&g_release_done.completion);
	pr_info("CPU%d phase class event release success\n", cpu);
}

void phase_perf_release(const struct cpumask *cpus)
{
	int cpu;

	phase_init_done(&g_release_done, cpumask_weight(cpus));
	for_each_cpu(cpu, cpus) {
		struct phase_cpu *pa_cpu = &per_cpu(phase_cpu, cpu);

		kthread_queue_work(&pa_cpu->release_worker, &pa_cpu->release_work);
	}
	wait_for_completion(&g_release_done.completion);
}

int *phase_perf_pinned_events(void)
{
	return phase_perf_pevents;
}

int *phase_perf_flexible_events(void)
{
	return phase_perf_fevents;
}

void phase_perf_read_events(int cpu, u64 *pdata, u64 *fdata)
{
	int event_id;
	int index;

	for_each_phase_pevents(index, phase_perf_pevents)
		pdata[index] = read_cpu_counter(cpu, index, EVENT_TYPE_PINNED);

	for_each_phase_fevents(index, phase_perf_fevents) {
		event_id = phase_perf_fevents[index];
		if (event_id == PHASE_EVENT_GROUP_TERMINATOR)
			continue;
		fdata[index] = read_cpu_counter(cpu, index, EVENT_TYPE_FLEXIBLE);
	}
}

#ifdef CONFIG_PHASE_TASK_PERF
static inline u64 read_task_counter(struct task_struct *p, int index, int pinned)
{
	struct perf_event *event = pinned ? p->phase_info->pevents[index] :
					p->phase_info->fevents[index];

	if (!event || !event->pmu)
		return 0;

	return perf_event_local_pmu_read(event);
}

static int create_task_counter(struct task_struct *p, int event_id, int index, int pinned)
{
	struct perf_event_attr *attr = NULL;
	struct perf_event **events = pinned ? p->phase_info->pevents :
					p->phase_info->fevents;
	struct perf_event *event = NULL;

	attr = alloc_attr(event_id, pinned);
	if (IS_ERR(attr))
		return PTR_ERR(attr);

	if (event_id == sysctl_phase_task_event)
		attr->sample_period = sysctl_phase_task_event_delta;

	event = perf_event_create_kernel_counter(attr, -1, p, NULL, NULL);
	if (IS_ERR(event)) {
		pr_err("phase unable to create perf event (task:%s-pid%d-type:%d-pinned:%d-config:0x%llx) : %ld",
		       p->comm, p->pid, attr->type, attr->pinned, attr->config, PTR_ERR(event));
		kfree(attr);
		return PTR_ERR(event);
	}

	events[index] = event;
	perf_event_enable(events[index]);
	kfree(attr);
	return 0;
}

int release_task_event(struct task_struct *p)
{
	struct perf_event **events = NULL;
	struct perf_event *event = NULL;
	int index;

	events = p->phase_info->pevents;
	for_each_phase_pevents(index, phase_perf_pevents) {
		event = events[index];

		if (!event)
			return 0;

		perf_event_release_kernel(event);
		events[index] = NULL;
	}

	events = p->phase_info->fevents;
	for_each_phase_fevents(index, phase_perf_fevents) {
		event = events[index];

		if (!event)
			return 0;

		perf_event_release_kernel(event);
		events[index] = NULL;
	}

	return 0;
}

int disable_task_event(struct task_struct *p)
{
	struct perf_event **events = NULL;
	struct perf_event *event = NULL;
	int index;

	events = p->phase_info->pevents;
	for_each_phase_pevents(index, phase_perf_pevents) {
		event = events[index];

		if (!event)
			return 0;

		perf_event_disable(event);
	}

	events = p->phase_info->fevents;
	for_each_phase_fevents(index, phase_perf_fevents) {
		event = events[index];

		if (!event)
			return 0;

		perf_event_disable(event);
	}

	return 0;
}

int create_task_event(struct task_struct *p)
{
	int index;
	int err = 0;

	for_each_phase_pevents(index, phase_perf_pevents) {
		err = create_task_counter(p, phase_perf_pevents[index], index, EVENT_TYPE_PINNED);
		if (err)
			return err;
	}

	for_each_phase_fevents(index, phase_perf_fevents) {
		err = create_task_counter(p, phase_perf_fevents[index], index, EVENT_TYPE_FLEXIBLE);
		if (err)
			return err;
	}

	p->phase_info->last_update_time = ktime_get_ns();

	return 0;
}

void phase_perf_task_read_events(struct task_struct *p, u64 *pdata, u64 *fdata)
{
	int event_id;
	int index;

	for_each_phase_pevents(index, phase_perf_pevents)
		pdata[index] = read_task_counter(p, index, EVENT_TYPE_PINNED);

	for_each_phase_fevents(index, phase_perf_fevents) {
		event_id = phase_perf_fevents[index];
		if (event_id == PHASE_EVENT_GROUP_TERMINATOR)
			continue;
		fdata[index] = read_task_counter(p, index, EVENT_TYPE_FLEXIBLE);
	}
}

int phase_set_task_track(struct task_struct *p, unsigned int track)
{
	if (!p || !p->phase_info)
		return -EINVAL;

	if ((track && p->phase_info->tracked) ||
	    (!track && !p->phase_info->tracked))
		return -EINVAL;

	if (track) {
		create_task_event(p);
		p->phase_info->tracked = true;
	} else {
		p->phase_info->tracked = false;
		disable_task_event(p);
		phase_task_event(p, PHASE_TASK_SWITCH);
		clear_task_phase_count(p);
		release_task_event(p);
	}

	return 0;
}
#endif
int phase_init(void)
{
	int cpu;
	struct task_struct *create_thread = NULL;
	struct task_struct *release_thread = NULL;
	struct sched_param param = { .sched_priority = MAX_USER_RT_PRIO / 2 };

	for_each_cpu(cpu, cpu_online_mask) {
		struct phase_cpu *pa_cpu = &per_cpu(phase_cpu, cpu);

		kthread_init_worker(&pa_cpu->release_worker);
		kthread_init_work(&pa_cpu->release_work, __phase_perf_release);
		release_thread = kthread_create_on_cpu(kthread_worker_fn, &pa_cpu->release_worker,
						       cpu, "phase_release");
		sched_setscheduler_nocheck(release_thread, SCHED_FIFO, &param);
		pa_cpu->release_thread = release_thread;
		wake_up_process(release_thread);

		kthread_init_worker(&pa_cpu->create_worker);
		kthread_init_work(&pa_cpu->create_work, __phase_perf_create);
		create_thread = kthread_create_on_cpu(kthread_worker_fn, &pa_cpu->create_worker,
						      cpu, "phase_create");
		sched_setscheduler_nocheck(create_thread, SCHED_FIFO, &param);
		pa_cpu->create_thread = create_thread;
		wake_up_process(create_thread);
	}
	return 0;
}

module_init(phase_init);
