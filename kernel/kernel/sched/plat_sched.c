// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * Platform implementation for scheduler (common, not for a specific sched
 * class). Aim to avoid duplicated code in oh kernel.
 */
#include "sched.h"
#include <trace/events/sched.h>
#include <linux/delay.h>

#if defined(CONFIG_HISI_EAS_SCHED) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))

/* Return 0 for not a valid margin. Must check. */
static unsigned int rtg_capacity_margin(struct task_struct *p)
{
#ifdef CONFIG_SCHED_RTG
	struct related_thread_group *grp = NULL;

	rcu_read_lock();
	grp = task_related_thread_group(p);
	rcu_read_unlock();

	if (grp && grp->preferred_cluster)
		return grp->capacity_margin;
#endif

	return 0;
}

bool fit_margin(u64 util, u64 cap, u32 margin)
{
	return (util * margin) <= (cap << SCHED_CAPACITY_SHIFT);
}

/*
 * This function is different from task_fit_cpu():
 * Task_fit_cpu() returns true if cpu is p's max possible choice, see the
 * for_each_cpu loop, even if p's util does not fit cpu cap.
 * That's good for deciding if a running task p needs to upmigrate. Or in
 * other words task_fit_cpu() tells if we need to mark a misfit flag for p.
 *
 * Get_task_fit_cpus() is quite simple:
 * Iterate all cpus and find the fit ones.
 * It simply cares about one thing: task util fit the cpu or not. Caller
 * should take the responsibility of finding a max possible one in the
 * misfit cpus if fit cpus are not available. And caller should take care
 * of p->cpus_ptr and cpu_online etc.
 *
 * BTW this function aims to reduce the very heavy cost of calling
 * task_fit_cpu() in a for_each_cpu loop.
 */
void get_task_fit_cpus(struct task_struct *p, struct cpumask *fit_cpus)
{
	struct sched_cluster *cluster = NULL;
	unsigned long task_base_util = uclamp_task_util(p);
	struct cpumask *rtg_target = rtg_and_colocgrp_prefer_cluster(p);
	unsigned int rtg_margin = rtg_capacity_margin(p);
	unsigned long rtg_target_cap = rtg_target ?
			capacity_orig_of(cpumask_first(rtg_target)) : 0;

	cpumask_clear(fit_cpus);

	for_each_sched_cluster(cluster) {
		int fcpu = cpumask_first(&cluster->cpus);
		unsigned long cap_orig = capacity_orig_of(fcpu);
		unsigned int margin = rtg_margin ?: cpu_capacity_margin(fcpu);
		unsigned long task_util = task_base_util;
		int i;

		if (rtg_target && cap_orig < rtg_target_cap)
			continue;

		if (use_task_pred_load(fcpu, p))
			task_util = max(task_util, task_pred_util(p));

		/* Assumes capacity_orig > capacity_of. */
		if (!fit_margin(task_util, cap_orig, margin))
			continue;

		if (!is_smt(fcpu) || task_expeller_type(p) > 0) {
			cpumask_or(fit_cpus, fit_cpus, &cluster->cpus);
			continue;
		}

		for_each_cpu(i, &cluster->cpus)
			if (fit_margin(task_util, capacity_of(i), margin))
				cpumask_set_cpu(i, fit_cpus);
	}
}

/*
 * Ofcourse we both need get_task_fit_cpus() and a task_fit_cpu(). Their
 * difference is explained in the above comment.
 * Task_fit_cpu() is an alternative of task_fits_max(). Task_fits_max()
 * should be deprecated in smt but it works for cfs now since there are
 * already many other related codes helping. Refactoring is needed but we
 * have to rebuild many logics which are beyond task_fits_max() itself.
 * So, keep cfs unchanged, at least at this moment. Introduce new interface
 * for rt, and for new code.
 */
bool task_fit_cpu(struct task_struct *p, int cpu)
{
	int i;
	cpumask_t allowed_cpus;
	struct cpumask *rtg_target = rtg_and_colocgrp_prefer_cluster(p);
	unsigned int rtg_margin = rtg_capacity_margin(p);
	unsigned int margin = rtg_margin ?: cpu_capacity_margin(cpu);
	unsigned long task_util = uclamp_task_util(p);
	unsigned long cap_orig = capacity_orig_of(cpu);
	bool task_is_smt_expeller = task_expeller_type(p) > 0;
	unsigned long cap;
	bool rtg_misfit = false;

	if (rtg_target &&
	    cap_orig < capacity_orig_of(cpumask_first(rtg_target)))
		rtg_misfit = true;

	if (use_task_pred_load(cpu, p))
		task_util = max(task_util, task_pred_util(p));

	if (task_is_smt_expeller)
		cap = cap_orig;
	else
		cap = capacity_of(cpu);

	if (!rtg_misfit && fit_margin(task_util, cap, margin))
		return true;

	/* Check if there're better choices. */
	cpumask_and(&allowed_cpus, p->cpus_ptr, cpu_online_mask);
	cpumask_clear_cpu(cpu, &allowed_cpus);
	for_each_cpu_reverse(i, &allowed_cpus) {
		if (task_is_smt_expeller) {
			if (capacity_orig_of(i) > cap)
				return false;
		} else {
			if (capacity_of(i) > cap * HIGHER_CPU_CAP_PCT / 100)
				return false;
		}
	}

	return true;
}

void get_avail_cpus(struct cpumask *cpus, struct task_struct *p)
{
	int i;

	if (cpumask_empty(cpus))
		return;

	cpumask_and(cpus, cpus, p->cpus_ptr);
	cpumask_and(cpus, cpus, cpu_online_mask);
#ifdef CONFIG_CPU_ISOLATION_OPT
	cpumask_andnot(cpus, cpus, cpu_isolated_mask);
#endif

	for_each_cpu(i, cpus) {
		if (walt_cpu_high_irqload(i))
			goto delete;

		if (cpu_should_thread_off(i))
			goto delete;

		if (rt_task(p)) {
			if (cpu_may_not_preemptible(i))
				goto delete;
		}

		continue;
delete:
		cpumask_clear_cpu(i, cpus);
	}
}

/*
 * Now find better candidates in cpus with same rt pri in the same cluster.
 * Why prefer max spare cap?
 * spare_cap(i) = capacity_of(i) - cpu_util(i)
 *              = core_cap(i) - cpu_util(sibling) - cpu_util(i)
 * If p fit cpus, max spare cap cpu hurt others less.
 * If p misfit, probably max capcity_of is preffered. But in fact, sibling
 * cpu won't enter idle when it finishes cpu_util(sibling). At least it will
 * pull loads from this cpu. In this perspective we still prefer max spare_cap.
 */
void get_max_spare_cap_cpus(struct cpumask *cpus, struct task_struct *p)
{
	int i;
	long max_spare = LONG_MIN;
	unsigned int min_exit_lat = UINT_MAX;
	cpumask_t best_cpus = CPU_MASK_NONE;
	bool task_is_smt_expeller = task_expeller_type(p) > 0;

	if (cpumask_weight(cpus) <= 1)
		return;

	for_each_cpu(i, cpus) {
		struct cpuidle_state *idle = idle_get_state(cpu_rq(i));
		unsigned int exit_lat = idle ? idle->exit_latency : 0;
		long cap = task_is_smt_expeller ? capacity_orig_of(i)
						: capacity_of(i);
		long spare_cap = max(cap - cpu_util_without(i, p), 0L);

		/*
		 * No spare cap left. Let the one with more tasks have
		 * less spare cap. That prevents rt tasks to cumulate
		 * on lower cpu ids and helps to stay away from an
		 * unbalanced cpu due to cpuset.
		 */
		if (spare_cap == 0)
			spare_cap -= cpu_rq(i)->rt.rt_nr_total;

		if (spare_cap > max_spare ||
		    (spare_cap == max_spare && exit_lat < min_exit_lat)) {
			max_spare = spare_cap;
			min_exit_lat = exit_lat;
			cpumask_clear(&best_cpus);
			cpumask_set_cpu(i, &best_cpus);
		} else if (spare_cap == max_spare && exit_lat == min_exit_lat) {
			cpumask_set_cpu(i, &best_cpus);
		}
	}

	cpumask_copy(cpus, &best_cpus);
}

void honor_prev_cpu(struct cpumask *cpus, struct task_struct *p)
{
	if (cpumask_test_cpu(task_cpu(p), cpus)) {
		cpumask_clear(cpus);
		cpumask_set_cpu(task_cpu(p), cpus);
	}
}

int find_best_candidate(struct task_struct *p,
			struct cpumask *candidate_cpus, bool boosted)
{
	struct sched_cluster *cluster = NULL;
	int cpu;
	int best_cpu = -1;
	unsigned long best_capacity = boosted ? 0 : ULONG_MAX;

	if (cpumask_empty(candidate_cpus))
		return -1;

	for_each_sched_cluster(cluster) {
		int fcpu = cpumask_first(&cluster->cpus);
		unsigned long capacity_orig = capacity_orig_of(fcpu);
		cpumask_t search_cpus;

		if ((boosted && capacity_orig < best_capacity) ||
		    (!boosted && capacity_orig > best_capacity))
			continue;

		cpumask_and(&search_cpus, candidate_cpus, &cluster->cpus);
		if (cpumask_empty(&search_cpus))
			continue;

		get_avail_cpus(&search_cpus, p);
		if (rt_task(p))
			get_lowest_pri_cpus(&search_cpus, p);
		get_max_spare_cap_cpus(&search_cpus, p);
		honor_prev_cpu(&search_cpus, p);

		cpu = cpumask_first(&search_cpus);
		if (cpu < nr_cpu_ids) {
			best_cpu = cpu;
			best_capacity = capacity_orig;
		}
	}

	return best_cpu;
}
#endif /* CONFIG_HISI_EAS_SCHED */

#ifdef CONFIG_SMT_MODE_GOV
void update_smt_capacity(struct rq *rq)
{
	unsigned long cap_orig, cap_mt, sibling_util, left_cap;

	if (!rq->is_smt)
		return;

	cap_orig = capacity_orig_of(cpu_of(rq));
	/*
	 * We don't have to use rq->smt_cap_discount here since we will
	 * check sibling_util. Things are better and simpler in this way.
	 */
	cap_mt = cap_orig * sysctl_task_cap_discount_in_mt / 100;

	/*
	 * Sibling cpu may take cap_mt capacity from the physic core at max.
	 * That is, finally, this cpu must has at least cap_mt capacity left.
	 */
	sibling_util = min(cpu_util(rq->smt_sibling), cap_mt);

	/*
	 * core_cap == cap_orig   when sibling_util == 0      (min core_cap)
	 * core_cap == cap_mt * 2 when sibling_util == cap_mt (max core_cap)
	 * Such that core_cap can be:
	 * min_core_cap + sibling_util_rate * (max_core_cap - min_core_cap)
	 * That is,
	 * core_cap = cap_orig + sibling_util / cap_mt * (cap_mt * 2 - cap_orig)
	 *
	 * And the capacity left for this cpu should be:
	 * core_cap - sibling_take_cap (i.e. sibling_util)
	 *
	 * Simplified as the following formula.
	 */
	left_cap = cap_orig - sibling_util * (cap_orig - cap_mt) / cap_mt;

	rq->smt_capacity = left_cap;
}
#endif

/*
 * Sleep for (usecs - 2ms) then delay for the remaining time. So that
 * scheduler latency won't be a problem if it is lower than 2ms in fact.
 *
 * Note that the udelay/ndelay use wfe() in their implementation.
 *
 * The 2ms SCHED_LATENCY_US here includes max cpu power up latency (for
 * the hrtimer + the task) and scheduler latency in light loaded case.
 * But noticed that scheduler latency can be very high for a cfs task if
 * the system is very heavy.
 * So the caller must set itself to rt priority if it wants to be accurate
 * in heavy cases.
 * And there are more efforts needed for the caller if it still wants to
 * work well in some corner cases.
 */
#define SCHED_LATENCY_US 2000
void usleep_accurate(unsigned long usecs)
{
	ktime_t target, expires, remain;

	if (usecs <= SCHED_LATENCY_US)
		return udelay(usecs);

	target = ktime_add_us(ktime_get(), usecs);
	expires = ktime_sub_us(target, SCHED_LATENCY_US);

	for (;;) {
		__set_current_state(TASK_UNINTERRUPTIBLE);
		/* Do not return before the requested sleep time has elapsed */
		if (!schedule_hrtimeout_range(&expires, 0, HRTIMER_MODE_ABS))
			break;
	}

	remain = ktime_sub(target, ktime_get());
	if (remain > 0)
		udelay(ktime_to_us(remain));
}
