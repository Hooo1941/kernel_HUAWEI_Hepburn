// SPDX-License-Identifier: GPL-2.0
/*
 * smt_mode_gov.c
 *
 * smt mode governor
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/cpuhotplug.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/kthread.h>
#include <linux/percpu.h>
#include <linux/math64.h>
#include <linux/version.h>
#include <linux/sched/rt.h>
#include <linux/sched/smt.h>
#include <linux/capability.h>
#include <linux/arch_topology.h>
#include <uapi/linux/sched/types.h>
#include <linux/core_ctl.h>
#include <securec.h>

#include <../kernel/sched/sched.h>
#include <../kernel/sched/walt.h>
#define CREATE_TRACE_POINTS
#include <platform_include/trace/events/smt.h>

struct smt_cpu_state {
	rwlock_t state_lock; /* protect following fields */
	cpumask_t smt_mask;
	cpumask_t offline_mask;
	cpumask_t isolated_mask;
	cpumask_t idle_mask;
	/* 0: Core idle, 1: ST mode, 2: MT mode */
	unsigned int curr_mode;
	unsigned int nr_switch;
	unsigned long curr_cpu_capacity;
	unsigned int task_cap_discount[NR_SMT_MODE];
	unsigned int top_task_cap_discount[NR_SMT_MODE];
	unsigned int cpu_cap_discount[NR_SMT_MODE];
};

unsigned int sysctl_task_cap_discount_in_mt = 65; /* 65% of st mode */
unsigned int sysctl_top_task_cap_discount_in_mt = 100; /* 100% of st mode */
unsigned int sysctl_cpu_cap_discount_in_mt = 50; /* 50% of st mode */

static DEFINE_PER_CPU(struct smt_cpu_state*, smt_data);

static void update_capacity(struct smt_cpu_state *data,
			    int cpu, unsigned int active_cpus)
{
	int i;
	struct rq_flags rf;
	struct rq *rq = cpu_rq(cpu_rq(cpu)->smt_sibling);
	unsigned int target = active_cpus > 1 ? SMT_MODE : ST_MODE;

	rq_lock_irqsave(rq, &rf);
#ifdef CONFIG_SCHED_LTS
	update_task_ravg(rq->curr, rq, TASK_UPDATE, sched_ktime_clock(), 0);
#else
	walt_update_task_ravg(rq->curr, rq, TASK_UPDATE, walt_ktime_clock(), 0);
#endif
	for_each_cpu(i, &data->smt_mask) {
		cpu_rq(i)->smt_cap_discount[TASK_DC] = data->task_cap_discount[target];
		cpu_rq(i)->smt_cap_discount[CPU_DC] = data->cpu_cap_discount[target];
		cpu_rq(i)->smt_cap_discount[TOP_TASK_DC] = data->top_task_cap_discount[target];
	}
	rq_unlock_irqrestore(rq, &rf);

	trace_smt_update_mode(&data->smt_mask,
			      target,
			      data->curr_cpu_capacity * data->task_cap_discount[target] / 100);
}

unsigned int smt_get_mode(int cpu)
{
	struct smt_cpu_state *cpu_data = NULL;

	if (cpu < 0 || cpu >= nr_cpu_ids)
		return 0;

	cpu_data = per_cpu(smt_data, cpu);
	if (!cpu_data)
		return 0;

	return cpu_data->curr_mode;
}

/* called with lock held */
static void update_mode(struct smt_cpu_state *data, int cpu)
{
	unsigned int nr_active =
			cpumask_weight(&data->smt_mask) -
			cpumask_weight(&data->idle_mask) -
			cpumask_weight(&data->offline_mask);

	if (nr_active > 1 || data->curr_mode > 1)
		update_capacity(data, cpu, nr_active);

	data->curr_mode = nr_active;
	data->nr_switch++;
}

static int cpuhp_smt_online(unsigned int cpu)
{
	struct smt_cpu_state *cpu_data = per_cpu(smt_data, cpu);

	if (!cpu_data)
		return 0;

	write_lock(&cpu_data->state_lock);
	cpumask_clear_cpu(cpu, &cpu_data->offline_mask);

	update_mode(cpu_data, cpu);
	write_unlock(&cpu_data->state_lock);

	return 0;
}

static int cpuhp_smt_offline(unsigned int cpu)
{
	struct smt_cpu_state *cpu_data = per_cpu(smt_data, cpu);

	if (!cpu_data)
		return 0;

	write_lock(&cpu_data->state_lock);
	cpumask_set_cpu(cpu, &cpu_data->offline_mask);

	update_mode(cpu_data, cpu);
	write_unlock(&cpu_data->state_lock);

	return 0;
}

static int update_isolated_cpu(struct notifier_block *nb,
			       unsigned long val, void *data)
{
	int cpu = *(int *)data;
	struct smt_cpu_state *cpu_data = per_cpu(smt_data, cpu);

	if (!cpu_data)
		return 0;

	write_lock(&cpu_data->state_lock);
	cpumask_and(&cpu_data->isolated_mask,
		    &cpu_data->smt_mask, cpu_isolated_mask);
	write_unlock(&cpu_data->state_lock);

	return NOTIFY_OK;
}

static struct notifier_block isolation_update_nb = {
	.notifier_call = update_isolated_cpu,
};

static int update_idle_cpu(struct notifier_block *nb,
			   unsigned long action, void *data)
{
	int cpu = smp_processor_id();
	int index = (int)action;
	struct smt_cpu_state *cpu_data = per_cpu(smt_data, cpu);

	this_rq()->idle_state_idx = index;

	if (!cpu_data)
		return NOTIFY_OK;

	write_lock(&cpu_data->state_lock);
	if (index < 0)
		cpumask_clear_cpu(cpu, &cpu_data->idle_mask);
	else
		cpumask_set_cpu(cpu, &cpu_data->idle_mask);

	update_mode(cpu_data, cpu);
	write_unlock(&cpu_data->state_lock);

	return NOTIFY_OK;
}

static struct notifier_block idle_update_nb = {
	.notifier_call = update_idle_cpu,
	.priority = 1,
};

/* ========================= sysfs interface =========================== */
static ssize_t show_status(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	ssize_t count = 0;
	struct smt_cpu_state *cpu_data = per_cpu(smt_data, dev->id);

	if (!cpu_data)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "\n");

	read_lock(&cpu_data->state_lock);
	count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1,
			  "all:%*pbl\n", cpumask_pr_args(&cpu_data->smt_mask));
	count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1,
			  "isolated:%*pbl\n", cpumask_pr_args(&cpu_data->isolated_mask));
	count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1,
			  "idle:%*pbl\n", cpumask_pr_args(&cpu_data->idle_mask));
	count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1,
			  "offline:%*pbl\n", cpumask_pr_args(&cpu_data->offline_mask));
	count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1,
			  "curr_mode:%s\n", (cpu_data->curr_mode == 0) ? "Core idle" :
						((cpu_data->curr_mode == 1) ? "ST" : "MT"));
	count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1,
			  "nr_switch:%u\n", cpu_data->nr_switch);
	count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1,
			  "forceidle_count:%u\n", cpu_rq(dev->id)->core_forceidle_count_sum);
	count += snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1,
			  "forceidle_time:%llu %llu\n", cpu_rq(dev->id)->core_forceidle_time[0],
			  cpu_rq(dev->id)->core_forceidle_time[1]);
	read_unlock(&cpu_data->state_lock);

	return count;
}

static DEVICE_ATTR(status, 0440, show_status, NULL);

static struct attribute *smt_attrs[] = {
	&dev_attr_status.attr,
	NULL
};

static struct attribute_group smt_attr_group = {
	.attrs = smt_attrs,
	.name = "smt",
};

int smt_add_interface(struct device *dev)
{
	return sysfs_create_group(&dev->kobj, &smt_attr_group);
}

static int cpu_smt_data_init(int cpu)
{
	int i;
	struct smt_cpu_state *cpu_data = NULL;

	if (per_cpu(smt_data, cpu))
		return 0;

	cpu_data = kzalloc(sizeof(struct smt_cpu_state), GFP_KERNEL);
	if (IS_ERR_OR_NULL(cpu_data)) {
		pr_err("%s: alloc smt data for CPU%d err\n", cpu);
		return -ENOMEM;
	}

	rwlock_init(&cpu_data->state_lock);
	cpu_data->curr_mode = ST_MODE; /* ST_MODE default */
	cpumask_copy(&cpu_data->smt_mask, cpu_smt_mask(cpu));
	cpumask_clear(&cpu_data->offline_mask);
	cpumask_clear(&cpu_data->isolated_mask);
	cpumask_clear(&cpu_data->idle_mask);

	cpu_data->curr_cpu_capacity = topology_get_cpu_scale(cpu);
	cpu_data->task_cap_discount[ST_MODE] = 100;
	cpu_data->task_cap_discount[SMT_MODE] = sysctl_task_cap_discount_in_mt;
	cpu_data->top_task_cap_discount[ST_MODE] = 100;
	cpu_data->top_task_cap_discount[SMT_MODE] = sysctl_top_task_cap_discount_in_mt;
	cpu_data->cpu_cap_discount[ST_MODE] = 100;
	cpu_data->cpu_cap_discount[SMT_MODE] = sysctl_cpu_cap_discount_in_mt;

	for_each_cpu(i, cpu_smt_mask(cpu))
		per_cpu(smt_data, i) = cpu_data;

	return 0;
}

static int smt_mode_init(void)
{
	int cpu, ret;
	struct device *cpu_dev = NULL;

	for_each_possible_cpu(cpu) {
		if (cpumask_weight(cpu_smt_mask(cpu)) < 2)
			continue;

		ret = cpu_smt_data_init(cpu);
		if (ret)
			return ret;

		cpu_dev = get_cpu_device(cpu);
		ret = smt_add_interface(cpu_dev);
		if (ret)
			return ret;
	}

	/* register notifiers */
	ret = cpuhp_setup_state_nocalls_cpuslocked(CPUHP_AP_ONLINE_DYN,
						   "sched_smt:online",
						   cpuhp_smt_online,
						   cpuhp_smt_offline);
	if (ret < 0)
		return ret;

	core_ctl_notifier_register(&isolation_update_nb);
	(void)atomic_notifier_chain_register(&cpuidle_state_notifier, &idle_update_nb);

	return 0;
}
late_initcall(smt_mode_init);
