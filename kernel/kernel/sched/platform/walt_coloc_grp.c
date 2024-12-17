// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * Colocate group, a group of tasks that acts like one task in scheduler.
 */
#include "sched.h"
#include <trace/events/walt.h>

#define COLOC_GRP_HIST_SIZE 3
struct coloc_group {
	spinlock_t lock;
	u64 last_update;
	u64 window_start;
	u32 window_size;
	u32 nr_running[NR_CPUS];
	u32 total_nr_running;
	cpumask_t prefer_cluster;
	u32 util;
	u32 curr_load;
	u32 history[COLOC_GRP_HIST_SIZE];
	u8 id;
	s8 boost;
	u8 window_stats_policy;
	unsigned long prefer_cluster_last_kick;
	u64 last_cpufreq_update_mark;
};

static struct coloc_group g_coloc_groups[MAX_COLOC_GRP_ID];

static u32 get_grp_nr_running(struct coloc_group *grp)
{
	return grp->total_nr_running;
}

static void add_grp_nr_running(struct coloc_group *grp,
			       int cpu, int nr_running)
{
	grp->nr_running[cpu] += nr_running;
	WARN_ON_ONCE((s32)grp->nr_running[cpu] < 0);

	grp->total_nr_running += nr_running;
}

/*
 * There can be various implementations, based on what is the purpose.
 * In general, this function wants to mark that a grp task on the cpu
 * has been updated. So the cpu is active.
 *
 * Our pupose at this implementation is simple: we adjust cpufreq on
 * prefer cluster only and wants to know if prefer cluster has grp tasks
 * updated recently. Otherwise we don't want to vote the cpufreq.
 */
static void kick_grp_active(struct coloc_group *grp, int cpu)
{
	if (cpumask_test_cpu(cpu, &grp->prefer_cluster))
		grp->prefer_cluster_last_kick = jiffies;
}

/*
 * Considering the case:
 *   Prefer cluster is little cluster, based on the real load.
 *   All grp tasks are boosted to middle/big cluster.
 * We don't like to vote little cluster's cpufreq in this case. That's
 * all this function wants to do.
 * And note that in this implementation we have no trouble racing with
 * grp's prefer cluster changing.
 */
static bool prefer_cluster_active(struct coloc_group *grp)
{
	return time_before(jiffies, grp->prefer_cluster_last_kick +
				    nsecs_to_jiffies(grp->window_size) + 1);
}

/*
 * There can be two ways:
 * 1. Always mark util change if cpu is in the prefer_cluster and cap_curr
 *    doesn't satisfy our util.
 *    That's the current implementation. One problem is that we may always
 *    mark util change when target load is higher than 100. But we don't
 *    like to have target load > 100 now. And the MIN_INTERVAL can help.
 * 2. Only mark util change when grp window rollover and util > cap_curr.
 *    If cpu is not in the prefer_cluster, choose any cpu there and
 *    call cpufreq_gov_update() directly.
 *    But, in one word, that's much more complex in fact. There're many
 *    cases we can't handle. We don't know what happens in future in the
 *    prefer cluster at the point of grp rolling over on a remote cpu.
 */
#define KICK_CPUFREQ_MIN_INTERVAL (4 * NSEC_PER_MSEC)
static void kick_cpufreq_update(struct coloc_group *grp, int cpu, u64 now)
{
	if (cpumask_test_cpu(cpu, &grp->prefer_cluster)) {
		u32 util = min_t(u32, grp->util, capacity_orig_of(cpu));
		s64 delta_last_mark = now - grp->last_cpufreq_update_mark;

		if (util > capacity_curr_of(cpu) &&
		    delta_last_mark > KICK_CPUFREQ_MIN_INTERVAL) {
			sugov_mark_util_change(cpu, COLOC_GRP_NEED_UPDATE);
			grp->last_cpufreq_update_mark = now;
		}
	}
}

static struct coloc_group *task_coloc_grp(struct task_struct *p)
{
	struct coloc_group *grp = NULL;

	if (is_idle_task(p))
		return NULL;

	grp = p->coloc_grp;
	if (grp)
		return grp;

	/* Take p's coloc_grp prior to uclamp coloc_grp. */
	grp = task_uclamp_group(p, coloc_grp);
	if (grp)
		return grp;

	return NULL;
}

/*
 * We can't sync change of p's task_coloc_grp since p's uclamp
 * cgroup coloc_grp can be changed dynamically. So, everytime
 * we access p's latest task_coloc_grp, we must take care of
 * its last_coloc_grp, for the grp_nr_running.
 * Note that walt.c won't call us for the same p concurrently.
 */
static struct coloc_group *get_task_coloc_grp(struct task_struct *p,
					      bool queued, int cpu)
{
	struct coloc_group *grp = task_coloc_grp(p);
	struct coloc_group *last_grp = p->last_coloc_grp;
	unsigned long flags;

	if (unlikely(last_grp != grp)) {
		if (queued) {
			if (last_grp) {
				spin_lock_irqsave(&last_grp->lock, flags);
				add_grp_nr_running(last_grp, cpu, -1);
				spin_unlock_irqrestore(&last_grp->lock, flags);
			}

			if (grp) {
				spin_lock_irqsave(&grp->lock, flags);
				if (get_grp_nr_running(grp) == 0)
					grp->last_update = max(grp->last_update,
							       p->ravg.mark_start);
				add_grp_nr_running(grp, cpu, 1);
				spin_unlock_irqrestore(&grp->lock, flags);
			}
		}

		p->last_coloc_grp = grp;
	}

	return grp;
}

/* No need of walt_set_window_start(). */
static u64 update_grp_window_start(struct coloc_group *grp, u64 now)
{
	u64 window_size = grp->window_size;

	if ((s64)(now - grp->window_start) >= window_size)
		grp->window_start = now / window_size * window_size;

	return grp->window_start;
}

static bool cpus_available(struct cpumask *cpus)
{
	cpumask_t online_cpus;

#ifdef CONFIG_CPU_ISOLATION_OPT
	cpumask_andnot(&online_cpus, cpu_online_mask, cpu_isolated_mask);
#else
	cpumask_copy(&online_cpus, cpu_online_mask);
#endif
	return cpumask_intersects(cpus, &online_cpus);
}

static void update_prefer_cluster(struct coloc_group *grp)
{
	struct sched_cluster *cluster = NULL;
	u32 boosted_util = util_margin(grp->util, grp->boost);

	for_each_sched_cluster(cluster) {
		int fcpu = cpumask_first(&cluster->cpus);
		u64 cap = capacity_orig_of(fcpu);
		u32 margin = cpu_capacity_margin(fcpu);

		if (unlikely(!cpus_available(&cluster->cpus)))
			continue;

		cpumask_copy(&grp->prefer_cluster, &cluster->cpus);
		if (fit_margin(boosted_util, cap, margin))
			return;
	}
}

/*
 * We don't scale exec time by p's cpu, but by grp's prefer_cluster if
 * there're grp tasks queued, or otherwise account the largeset cap cpu
 * who has grp tasks queued.
 *
 * mid:  nr>0  (prefer cluster)
 * big:  nr>0  <-- p
 * We must scale by mid in this case.
 * Why?
 * mid: | --  ---------|
 * big: |        -     |
 * That's the most common case.
 *
 * lit:  nr>0  <-- p
 * mid:  nr=0  (prefer cluster)
 * big:  nr>0
 * We should scale by big in this case. But it's rare to have no tasks
 * in prefer cluster.
 */
static int accouted_cpu(struct coloc_group *grp)
{
	int cpu;

	for_each_cpu(cpu, &grp->prefer_cluster)
		if (grp->nr_running[cpu] > 0)
			return cpu;

	for (cpu = nr_cpu_ids - 1; cpu >= 0; cpu--)
		if (grp->nr_running[cpu] > 0)
			return cpu;

	WARN_ONCE(1, "Impossible, total_nr_running=%u mismatch nr_running\n",
			grp->total_nr_running);
	return 0;
}

/*
 * 1. No smt discount.
 *    (1) We compare grp->util with capacity_orig_of()/capacity_curr_of(),
 *        but no capacity_of().
 *    (2) It lose accuracy in inter-cluster migration. But that's not the
 *        case we care.
 * 2. No uclamp_max.
 *    (1) We can't since p's uclamp_max is irrelevant to the coloc grp.
 *    (2) We really shouldn't need it.
 *    So let's define coloc_grp prior to uclamp_max.
 */
static s64 scale_grp_exec_time(struct coloc_group *grp, u64 delta)
{
	u64 cap_curr = capacity_curr_of(accouted_cpu(grp));

	delta = (delta * cap_curr) >> SCHED_CAPACITY_SHIFT;
	return delta;
}

static void add_grp_load(struct coloc_group *grp, s64 delta)
{
	if (unlikely(delta < 0))
		delta = 0;

	delta = scale_grp_exec_time(grp, delta);
	grp->curr_load += delta;
}

static void update_grp_history(struct coloc_group *grp)
{
	u32 *hist = grp->history;
	int i;
	u32 sum = 0, max = 0, avg = 0;
	u8 policy = grp->window_stats_policy;
	u64 load = 0;

	for (i = COLOC_GRP_HIST_SIZE - 1; i >= 0; i--) {
		if (i > 0)
			hist[i] = hist[i-1];
		else
			hist[i] = grp->curr_load;

		sum += hist[i];
		if (hist[i] > max)
			max = hist[i];
	}

	grp->curr_load = 0;
	avg = sum / COLOC_GRP_HIST_SIZE;

	if (policy == WINDOW_STATS_RECENT)
		load = hist[0];
	else if (policy == WINDOW_STATS_MAX)
		load = max;
	else if (policy == WINDOW_STATS_MAX_RECENT_AVG)
		load = max(hist[0], avg);
	else if (policy == WINDOW_STATS_AVG)
		load = avg;

	grp->util = (load << SCHED_CAPACITY_SHIFT) / grp->window_size;

	update_prefer_cluster(grp);

	trace_perf(update_coloc_grp_history, grp->id, grp->history[0],
		   grp->util, grp->prefer_cluster.bits[0]);
}

static void update_grp_load(struct coloc_group *grp,
			    struct task_struct *p, u64 now)
{
	u64 window_start = update_grp_window_start(grp, now);
	u64 window_size = grp->window_size;
	u64 last_update = grp->last_update;
	bool new_window = ((s64)(window_start - last_update) > 0);
	u64 nr_full_windows = new_window ?
			      (window_start - last_update) / window_size : 0;
	bool busy = (get_grp_nr_running(grp) > 0);

	if (!last_update)
		goto done;

	if (nr_full_windows) {
		/*
		 * Ignore full busy windows, for simplicity.
		 * Ignore full empty windows unless past long time.
		 * That's exactly what we need and make the code short.
		 */
		last_update += nr_full_windows * window_size;

		if (!busy && nr_full_windows >= COLOC_GRP_HIST_SIZE)
			memset(grp->history, 0, sizeof(grp->history));
	}

	if (new_window && busy) {
		add_grp_load(grp, window_start - last_update);
		last_update = window_start;
	}

	if (new_window)
		update_grp_history(grp);

	if (busy)
		add_grp_load(grp, now - last_update);

done:
	grp->last_update = now;

	trace_perf(update_coloc_grp_load, grp->id, new_window, busy,
		   grp->curr_load, grp->history[0]);
}

void walt_update_coloc_grp_load(struct task_struct *p, int cpu,
				int event, u64 now)
{
	struct coloc_group *grp = NULL;
	bool enqueue_wakeup = (event == TASK_WAKE);
	bool dequeue_sleep = (event == PUT_PREV_TASK && !task_on_rq_queued(p));
	unsigned long flags;

	grp = get_task_coloc_grp(p, !enqueue_wakeup, cpu);
	if (likely(!grp))
		return;

	spin_lock_irqsave(&grp->lock, flags);

	update_grp_load(grp, p, now);

	if (enqueue_wakeup)
		add_grp_nr_running(grp, cpu, 1);
	else if (dequeue_sleep)
		add_grp_nr_running(grp, cpu, -1);

	kick_grp_active(grp, cpu);
	kick_cpufreq_update(grp, cpu, now);

	spin_unlock_irqrestore(&grp->lock, flags);
}

/*
 * Called in set_task_cpu()->walt_fixup_busy_time(). And after the
 * if (!p->on_rq && p->state != TASK_WAKING) check in walt_fixup_busy_time(),
 * p must be a queued task for us. When p->state == TASK_WAKING, walt must
 * have called TASK_WAKE before, that is a enqueue for walt. So we pass true
 * to get_task_coloc_grp() here.
 * Be very careful that we have the correct walt enqueue/dequeue for our
 * grp_nr_running. That's much different from the real enqueue/dequeue.
 * And if there're still bugs, we'd better add our own hooks in the real
 * enqueue/dequeue (and take care of throttling) and don't depend on walt
 * that much.
 */
void migrate_task_coloc_grp_contribution(struct task_struct *p,
					 int src_cpu, int dst_cpu)
{
	struct coloc_group *grp = NULL;
	unsigned long flags;

	grp = get_task_coloc_grp(p, true, src_cpu);
	if (likely(!grp))
		return;

	spin_lock_irqsave(&grp->lock, flags);
	add_grp_nr_running(grp, src_cpu, -1);
	add_grp_nr_running(grp, dst_cpu, 1);
	spin_unlock_irqrestore(&grp->lock, flags);
}

void init_task_coloc_grp(struct task_struct *p)
{
	p->last_coloc_grp = NULL;
	p->coloc_grp = NULL;
}

#define DEFAULT_COLOC_GRP_WINDOW_SIZE 16666666
void init_coloc_grps(void)
{
	int i;

	for (i = NO_COLOC_GRP_ID + 1; i < MAX_COLOC_GRP_ID; i++) {
		struct coloc_group *grp = &g_coloc_groups[i];

		memset(grp, 0, sizeof(*grp));

		spin_lock_init(&grp->lock);
		grp->id = i;
		grp->window_size = DEFAULT_COLOC_GRP_WINDOW_SIZE;
		grp->window_stats_policy = WINDOW_STATS_MAX_RECENT_AVG;
		grp->prefer_cluster = CPU_MASK_NONE;
	}
}

static struct cpumask *max_cluster(struct cpumask *cluster_cpus_1,
				   struct cpumask *cluster_cpus_2)
{
	if (!cluster_cpus_1)
		return cluster_cpus_2;
	if (!cluster_cpus_2)
		return cluster_cpus_1;

	return capacity_orig_of(cpumask_first(cluster_cpus_1)) >
	       capacity_orig_of(cpumask_first(cluster_cpus_2))
	       ? cluster_cpus_1 : cluster_cpus_2;
}

struct cpumask *rtg_and_colocgrp_prefer_cluster(struct task_struct *p)
{
	struct coloc_group *grp = task_coloc_grp(p);
	struct cpumask *prefer_cluster = grp ? &grp->prefer_cluster : NULL;

	if (prefer_cluster && cpumask_empty(prefer_cluster))
		prefer_cluster = NULL;

	return max_cluster(prefer_cluster, rtg_prefer_cluster(p));
}

/*
 * The simplest policy: let grp->util contribute to its prefer cluster's
 * cpufreq only. Probably we may develop better policy from feedbacks in
 * the real world tests in the future.
 */
void cpufreq_get_coloc_grp_util(struct cpumask *policy_cpus, u32 *util)
{
	int i;
	u32 max_util = 0;

	for (i = NO_COLOC_GRP_ID + 1; i < MAX_COLOC_GRP_ID; i++) {
		struct coloc_group *grp = &g_coloc_groups[i];

		if (cpumask_intersects(&grp->prefer_cluster, policy_cpus) &&
		    prefer_cluster_active(grp))
			max_util = max(max_util, grp->util);
	}

	*util = max_util;
}

u64 coloc_grp_id_read(struct cgroup_subsys_state *css, struct cftype *cft)
{
	struct coloc_group *grp = css_uc(css)->coloc_grp;

	return grp ? grp->id : NO_COLOC_GRP_ID;
}

int coloc_grp_id_write(struct cgroup_subsys_state *css,
		       struct cftype *cft, u64 val)
{
	if (val == NO_COLOC_GRP_ID)
		css_uc(css)->coloc_grp = NULL;
	else if (val < MAX_COLOC_GRP_ID)
		css_uc(css)->coloc_grp = &g_coloc_groups[val];
	else
		return -EINVAL;

	return 0;
}
