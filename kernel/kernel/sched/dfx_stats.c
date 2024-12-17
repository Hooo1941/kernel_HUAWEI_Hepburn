// SPDX-License-Identifier: GPL-2.0
/*
 * dfx_stats.c
 *
 * dfx irq/preempt stats and dfx show
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <linux/rtmutex.h>
#include "sched.h"

#define DFX_STATS_TOP 3
#define DFX_STATS_CALLBACK_DEPTH 4

struct tgid_entry {
	pid_t tgid;
	struct task_dfx_info	tgid_info;
	struct hlist_node 		hash;
};

struct disable_info {
	struct stats_info stats;
	u64 top_time[DFX_STATS_TOP];
	pid_t top_pid[DFX_STATS_TOP];
	unsigned long callback[DFX_STATS_TOP][DFX_STATS_CALLBACK_DEPTH];
	u32 min_index;

	u32	irq_no;
	pid_t irq_pid;
	u64 irq_disable_time;
};

static DEFINE_PER_CPU(struct disable_info, the_ps);
static DEFINE_PER_CPU(struct disable_info, the_is);

static DECLARE_HASHTABLE(dfx_hash_table, 10);
static DEFINE_RT_MUTEX(tgid_lock);

static char thread_prio_types[DFX_PRIO_TYPE_MAX][6] = { {"rt:"},
														{"vip:"},
														{"cfs:"} };

__read_mostly unsigned int dfx_stats_on;
__read_mostly unsigned int dfx_irq_preempt_stats_on;
static u32 dfx_no = 1;
EXPORT_SYMBOL_GPL(dfx_stats_on);

void dfx_start(void)
{
	int cpu;
	struct rq *rq;
	struct disable_info *ps, *is;
	for_each_possible_cpu(cpu) {
		rq = cpu_rq(cpu);

		ps = &per_cpu(the_ps, cpu);
		memset_s(ps, sizeof(struct disable_info), 0, sizeof(struct disable_info));
		is = &per_cpu(the_is, cpu);
		memset_s(is, sizeof(struct disable_info), 0, sizeof(struct disable_info));

		memset_s(&rq->rq_dfx_info, sizeof(struct dfx_info), 0, sizeof(struct dfx_info));
		rq->rq_dfx_info.nr_switches = rq->nr_switches;
	}
	dfx_stats_on = 1;
	dfx_no++;
}

void dfx_stop(void)
{
	int cpu;
	struct rq *rq;
	for_each_possible_cpu(cpu) {
		rq = cpu_rq(cpu);
		rq->rq_dfx_info.nr_switches = rq->nr_switches - rq->rq_dfx_info.nr_switches;
	}
	dfx_stats_on = 0;
}

static void update_top_info(struct disable_info *ps, u64 delta, pid_t pid)
{
	u32 index;
	ps->top_time[ps->min_index] = delta;
	ps->top_pid[ps->min_index] = pid;
	ps->callback[ps->min_index][0] = CALLER_ADDR0;
	ps->callback[ps->min_index][1] = CALLER_ADDR1;
	ps->callback[ps->min_index][2] = CALLER_ADDR2;
	ps->callback[ps->min_index][3] = CALLER_ADDR3;

	for (index = 0;index < DFX_STATS_TOP; index++) {
		if (ps->top_time[index] < ps->top_time[ps->min_index])
			ps->min_index = index;
	}
}

static void preempt_irq_update_common(struct disable_info *ps, u64 delta, pid_t pid)
{
	int t_idx;

	t_idx = range_of_time(delta);
	ps->stats.count[t_idx]++;
	ps->stats.time[t_idx] += delta;

	if (delta > ps->top_time[ps->min_index])
		update_top_info(ps, delta, pid);
}

void dfx_irq_disable_stats(void)
{
	if (dfx_irq_preempt_stats_on) {
		int cpu;
		struct disable_info *ps;

		if (is_idle_task(current))
			return;

		cpu = raw_smp_processor_id();
		ps = &per_cpu(the_is, cpu);
		ps->irq_disable_time = sched_clock();
		ps->irq_pid = current->pid;
		ps->irq_no = dfx_no;
	}
}
EXPORT_SYMBOL(dfx_irq_disable_stats);

void dfx_irq_enable_stats(void)
{
	if (dfx_irq_preempt_stats_on) {
		u64 delta;
		int cpu;
		struct disable_info *ps;

		cpu = raw_smp_processor_id();
		ps = &per_cpu(the_is, cpu);
		if (ps->irq_no != dfx_no)
			return;

		ps->irq_no = 0;
		delta = sched_clock() - ps->irq_disable_time;
		if (delta > time_distribute_range[0])
			preempt_irq_update_common(ps, delta, ps->irq_pid);
	}
}
EXPORT_SYMBOL(dfx_irq_enable_stats);

void dfx_preempt_disable_stats(u32 pc, int val)
{
	if (dfx_irq_preempt_stats_on && pc == val) {
		current->preempt_dis_sum = 0;
		current->preempt_dis_start = sched_clock();
		current->dfx_no = dfx_no;
	}
}
EXPORT_SYMBOL(dfx_preempt_disable_stats);

void dfx_preempt_enable_stats(u32 pc, int val)
{
	if (dfx_irq_preempt_stats_on && pc == val) {
		struct disable_info *ps;
		int cpu;

		if (current->dfx_no != dfx_no)
			return;

		cpu = raw_smp_processor_id();
		ps = &per_cpu(the_ps, cpu);
		current->preempt_dis_sum += sched_clock() - current->preempt_dis_start;

		if (current->preempt_dis_sum > time_distribute_range[0])
			preempt_irq_update_common(ps, current->preempt_dis_sum, current->pid);
	}
}
EXPORT_SYMBOL(dfx_preempt_enable_stats);

ssize_t dfx_thread_stats_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
	ssize_t ret = 0;
	int cpu;
	struct rq *rq;
	struct dfx_info dfx_info_all;
	u32 i = 0, j = 0;

	memset_s(&dfx_info_all, sizeof(struct dfx_info), 0, sizeof(struct dfx_info));

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "cpu rt_running_time vip_running_time switches\n");

	for_each_possible_cpu(cpu) {
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%d ", cpu);
		rq = cpu_rq(cpu);
		for(i = 0; i < DFX_PRIO_TYPE_MAX; i++) {
			for(j = 0; j < THREAD_TIME_RANGE_COUNT; j++) {
				dfx_info_all.runnable[i].count[j] += rq->rq_dfx_info.runnable[i].count[j];
				dfx_info_all.runnable[i].time[j] += rq->rq_dfx_info.runnable[i].time[j];
				dfx_info_all.block[i].count[j] += rq->rq_dfx_info.block[i].count[j];
				dfx_info_all.block[i].time[j] += rq->rq_dfx_info.block[i].time[j];
				dfx_info_all.iowait[i].count[j] += rq->rq_dfx_info.iowait[i].count[j];
				dfx_info_all.iowait[i].time[j] += rq->rq_dfx_info.iowait[i].time[j];
			}

			if (i != DFX_CFS)
				ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%llu ", rq->rq_dfx_info.running_time[i]);
		}

		if (dfx_stats_on)
			ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%llu \n", rq->nr_switches - rq->rq_dfx_info.nr_switches);
		else
			ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%llu \n", rq->rq_dfx_info.nr_switches);
	}

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "runnable: >4 >10 >100 sum_count\n");
	for(i = 0; i < DFX_PRIO_TYPE_MAX; i++) {
		unsigned int count = 0;
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%s ", thread_prio_types[i]);
		for(j = 0; j < THREAD_TIME_RANGE_COUNT; j++) {
			ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u ",
				dfx_info_all.runnable[i].count[j]);
				count += dfx_info_all.runnable[i].count[j];
		}
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u \n",
			count);
	}

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "block: >10 >100 sum_count sum_time\n");
	for(i = 0; i < DFX_PRIO_TYPE_MAX - 1; i++) {
		unsigned int count = 0;
		u64 time = 0;
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%s ", thread_prio_types[i]);
		for(j = 1; j < THREAD_TIME_RANGE_COUNT; j++) {
			count += dfx_info_all.block[i].count[j];
			time += dfx_info_all.block[i].time[j];
			ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u ", dfx_info_all.block[i].count[j]);
		}
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u %llu\n", count, time);
	}

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "iowait: >10 >100 sum_count sum_time\n");
	for(i = 0; i < DFX_PRIO_TYPE_MAX - 1; i++) {
		unsigned int count = 0;
		u64 time = 0;
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%s ", thread_prio_types[i]);
		for(j = 1; j < THREAD_TIME_RANGE_COUNT; j++) {
			count += dfx_info_all.iowait[i].count[j];
			time += dfx_info_all.iowait[i].time[j];
			ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u ", dfx_info_all.iowait[i].count[j]);
		}
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u %llu\n", count, time);
	}

	return ret;
}

static struct tgid_entry *find_tgid_entry(pid_t tgid)
{
	struct tgid_entry *tgid_entry;
	hash_for_each_possible(dfx_hash_table, tgid_entry, hash, tgid) {
		if (tgid_entry->tgid == tgid)
			return tgid_entry;
	}
	return NULL;
}

static struct tgid_entry *find_or_register_tgid(pid_t tgid)
{
	struct tgid_entry *tgid_entry;

	tgid_entry = find_tgid_entry(tgid);
	if (tgid_entry)
		return tgid_entry;

	tgid_entry = kzalloc(sizeof(struct tgid_entry), GFP_ATOMIC);
	if (!tgid_entry)
		return NULL;

	tgid_entry->tgid = tgid;
	hash_add(dfx_hash_table, &tgid_entry->hash, tgid);

	return tgid_entry;
}

ssize_t dfx_cpu_stats_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
	ssize_t ret = 0;
	struct tgid_entry *tgid_entry = NULL;
	struct task_struct *task, *temp;
	unsigned long bkt;
	pid_t tgid;

	rt_mutex_lock(&tgid_lock);

	hash_for_each(dfx_hash_table, bkt, tgid_entry, hash) {
		memset_s(&tgid_entry->tgid_info, sizeof(struct task_dfx_info), 0, sizeof(struct task_dfx_info));
	}

	rcu_read_lock();
	do_each_thread(temp, task) {
		tgid = task_tgid_nr(task);
		if (!tgid_entry || tgid_entry->tgid != tgid)
			tgid_entry = find_or_register_tgid(tgid);
		if (!tgid_entry) {
			rcu_read_unlock();
			rt_mutex_unlock(&tgid_lock);
			pr_err("%s: failed to find the tgid_entry for tgid %d\n",
				__func__, tgid);
			return -ENOMEM;
		}

		tgid_entry->tgid_info.runnable_sum += task->task_dfx_info.runnable_sum;
		tgid_entry->tgid_info.block_count += task->task_dfx_info.block_count;
		tgid_entry->tgid_info.block_sum += task->task_dfx_info.block_sum;
		tgid_entry->tgid_info.iowait_count += task->task_dfx_info.iowait_count;
		tgid_entry->tgid_info.iowait_sum += task->task_dfx_info.iowait_sum;
	} while_each_thread(temp, task);
	rcu_read_unlock();

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "tgid runnable_time block_count block_time iowait_count iowait_time\n");
	hash_for_each(dfx_hash_table, bkt, tgid_entry, hash) {
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u %llu %u %llu %u %llu \n",
			tgid_entry->tgid, tgid_entry->tgid_info.runnable_sum,
			tgid_entry->tgid_info.block_count, tgid_entry->tgid_info.block_sum,
			tgid_entry->tgid_info.iowait_count, tgid_entry->tgid_info.iowait_sum);
	}

	rt_mutex_unlock(&tgid_lock);
	return ret;
}

static void get_sum_from_per_cpu(struct disable_info *ps, struct disable_info *disable_info)
{
	u32 i, j;
	for(i = 0; i < THREAD_TIME_RANGE_COUNT; i++) {
		disable_info->stats.count[i] += ps->stats.count[i];
		disable_info->stats.time[i] += ps->stats.time[i];
	}

	for (i = 0; i < DFX_STATS_TOP; i++) {
		if (ps->top_time[i] <= disable_info->top_time[disable_info->min_index]) {
			continue;
		}

		disable_info->top_time[disable_info->min_index] = ps->top_time[i];
		disable_info->top_pid[disable_info->min_index] = ps->top_pid[i];
		memcpy_s(disable_info->callback[disable_info->min_index], sizeof(u64) * DFX_STATS_CALLBACK_DEPTH,
			ps->callback[i], sizeof(u64) * DFX_STATS_CALLBACK_DEPTH);
		for (j = 0; j < DFX_STATS_TOP; j++) {
			if (disable_info->top_time[j] < disable_info->top_time[disable_info->min_index])
				disable_info->min_index = j;
		}
	}
}

static ssize_t preempt_irq_stats_show(struct kobject *kobj, struct kobj_attribute *kattr,
	char *buf, struct disable_info *disable_info, char *fun)
{
	ssize_t ret = 0;
	u64 all_count = 0;
	u64 all_time = 0;
	u32 i, j;

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%sDisableTime: >4 >10 >100\n", fun);
	for(i = 0; i < THREAD_TIME_RANGE_COUNT; i++) {
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u ", disable_info->stats.count[i]);
		all_count += disable_info->stats.count[i];
		all_time += disable_info->stats.time[i];
	}
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "\n%sDisableTimeAvg: %llu\n",
		fun, all_time / all_count);

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "timeTop3: ");
	for (i = 0; i < DFX_STATS_TOP; i++) {
		if (disable_info->top_time[i])
			ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%llu ", disable_info->top_time[i]);
	}
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "\n");

	for (i = 0; i < DFX_STATS_TOP; i++) {
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "Call Stack %d, pid = %d:\n",
			i, disable_info->top_pid[i]);
		if (disable_info->top_time[i]) {
			for (j = 0; j < DFX_STATS_CALLBACK_DEPTH; j++)
				ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%pS\n", (void *)disable_info->callback[i][j]);
		}
	}
	return ret;
}

ssize_t dfx_preempt_disable_stats_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
	int cpu;
	struct disable_info disable_info_all;

	memset_s(&disable_info_all, sizeof(struct disable_info), 0, sizeof(struct disable_info));
	for_each_possible_cpu(cpu) {
		struct disable_info *ps = &per_cpu(the_ps, cpu);
		get_sum_from_per_cpu(ps, &disable_info_all);
	}
	return preempt_irq_stats_show(kobj, kattr, buf, &disable_info_all, "preempt");
}

ssize_t dfx_irq_disable_stats_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf)
{
	int cpu;
	struct disable_info disable_info_all;

	memset_s(&disable_info_all, sizeof(struct disable_info), 0, sizeof(struct disable_info));
	for_each_possible_cpu(cpu) {
		struct disable_info *ps = &per_cpu(the_is, cpu);
		get_sum_from_per_cpu(ps, &disable_info_all);
	}
	return preempt_irq_stats_show(kobj, kattr, buf, &disable_info_all, "irq");
}
