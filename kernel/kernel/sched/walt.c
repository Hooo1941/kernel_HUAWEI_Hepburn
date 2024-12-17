/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * Window Assisted Load Tracking (WALT) implementation credits:
 * Srivatsa Vaddagiri, Steve Muckle, Syed Rameez Mustafa, Joonwoo Park,
 * Pavan Kumar Kondeti, Olav Haugan
 *
 * 2016-03-06: Integration with EAS/refactoring by Vikram Mulukutla
 *             and Todd Kjos
 */

#include <linux/acpi.h>
#include <linux/syscore_ops.h>
#include <trace/events/sched.h>
#include "sched.h"
#include <linux/cpufreq.h>
#include <platform_include/cee/linux/hisi_rtg.h>
#ifdef CONFIG_HW_VIP_THREAD
#include <chipset_common/hwcfs/hwcfs_common.h>
#endif

#define CREATE_TRACE_POINTS
#include <trace/events/walt.h>
#undef CREATE_TRACE_POINTS

static __read_mostly unsigned int walt_ravg_hist_size = 5;
static __read_mostly unsigned int walt_window_stats_policy =
	WINDOW_STATS_MAX_RECENT_AVG;
static __read_mostly unsigned int walt_account_wait_time = 1;
static __read_mostly unsigned int walt_freq_account_wait_time = 0;
static __read_mostly unsigned int walt_io_is_busy = 0;

__read_mostly unsigned int sysctl_sched_use_walt_cpu_util = 1;
__read_mostly unsigned int sysctl_sched_use_walt_task_util = 1;
__read_mostly unsigned int sysctl_sched_walt_cpu_high_irqload =
    (2 * NSEC_PER_SEC / CONFIG_HZ);
__read_mostly unsigned int sysctl_sched_walt_cpu_overload_irqload =
    (3 * NSEC_PER_SEC / CONFIG_HZ);
unsigned int sysctl_sched_walt_init_task_load_pct = 40;

#ifdef CONFIG_SCHED_WALT_WINDOW_SIZE_TUNABLE
/* true -> use PELT based load stats, false -> use window-based load stats */
bool __read_mostly walt_disabled = false;

/*
 * Window size (in ns). Adjust for the tick size so that the window
 * rollover occurs just before the tick boundary.
 */
__read_mostly unsigned int walt_ravg_window =
					    (20000000 / TICK_NSEC) * TICK_NSEC;
#define MIN_SCHED_RAVG_WINDOW ((10000000 / TICK_NSEC) * TICK_NSEC)
#define MAX_SCHED_RAVG_WINDOW ((1000000000 / TICK_NSEC) * TICK_NSEC)
#else
const bool walt_disabled = false;
const unsigned int walt_ravg_window =
				(20000000 / TICK_NSEC) * TICK_NSEC;
#endif

#ifdef CONFIG_SCHED_PRED_LOAD
__read_mostly unsigned int predl_enable;
__read_mostly unsigned int predl_window_size =
					(16000000 / TICK_NSEC) * TICK_NSEC;
__read_mostly unsigned int predl_do_predict_ctrl;
#endif

#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
__read_mostly unsigned int new_walt_ravg_window = (20000000 / TICK_NSEC) * TICK_NSEC;
unsigned int sysctl_walt_ravg_window_nr_ticks = (HZ / NR_WINDOWS_PER_SEC);

static DEFINE_SPINLOCK(walt_ravg_window_lock);

static atomic64_t walt_latest_window_start;
#endif

static unsigned int sync_cpu;
static ktime_t ktime_last;
static __read_mostly bool walt_ktime_suspended;

static unsigned int task_load(struct task_struct *p)
{
	return p->ravg.demand;
}

static inline int exiting_task(struct task_struct *p)
{
	return p->flags & PF_EXITING;
}

#ifdef CONFIG_SCHED_TOP_TASK
static inline unsigned int task_load_freq(struct task_struct *p)
{
	return p->ravg.load_avg;
}

#ifdef CONFIG_SCHED_DOWNMIGRATE_LOWER_LOAD
#define TASK_LOAD_FREQ_AVG_HIST_SIZE 3
static unsigned int task_load_freq_avg(struct task_struct *p)
{
	u32 *hist;
	int widx;
	u64 sum = 0;
	unsigned int hist_size;

	hist = &p->ravg.load_sum_history[0];
	hist_size = TASK_LOAD_FREQ_AVG_HIST_SIZE;

	for (widx = 0; widx < hist_size; widx++)
		sum += hist[widx];

	return sum / hist_size;
}
#endif

static inline u8 curr_table(struct rq *rq)
{
	return rq->curr_table;
}

static inline u8 prev_table(struct rq *rq)
{
	return 1 - rq->curr_table;
}

static inline int reversed_index(int index)
{
	if (unlikely(index >= NUM_LOAD_INDICES))
		index = NUM_LOAD_INDICES - 1;
	return NUM_LOAD_INDICES - 1 - index;
}

/*
 * Let lower index means higher load, so we can use find_next_bit
 * to get the next highest load.
 *
 * Note: max load is limited as:
 *       walt_ravg_window / NUM_LOAD_INDICES * (NUM_LOAD_INDICES - 1)
 */
static int load_to_index(u32 load)
{
	return reversed_index(load / SCHED_LOAD_GRANULE);
}

static unsigned long index_to_load(int index)
{
	return reversed_index(index) * SCHED_LOAD_GRANULE;
}

static unsigned long __top_task_load(struct rq *rq, u8 table)
{
	struct top_task_entry *top_task_table = rq->top_tasks[table];
	int top_index = rq->top_task_index[table];

	/*
	 * Top_index might be NUM_LOAD_INDICES.
	 * Check before use it as array index.
	 */
	if (top_index >= NUM_LOAD_INDICES)
		return 0;

	/*
	 * Just consider no top task load when the top task is
	 * not a preferidle one. It will take some cost to find
	 * the top preferidle task, which may not be very necessary.
	 */
	if (!top_task_table[top_index].preferidle_count)
		return 0;

	return index_to_load(top_index);
}

static unsigned long top_task_load(struct rq *rq)
{
	return max(__top_task_load(rq, curr_table(rq)),
		   __top_task_load(rq, prev_table(rq)));
}

unsigned long top_task_util(struct rq *rq)
{
	unsigned long capacity = capacity_orig_of(cpu_of(rq));

	unsigned long load = top_task_load(rq);
	unsigned long util = (load << SCHED_CAPACITY_SHIFT) / walt_ravg_window;

	return (util >= capacity) ? capacity : util;
}

/*
 * Limit overflow case.
 * For top_task_table[index].count, increase and decrease
 * for any index are always in pair.
 * For top_task_table[index].preferidle_count, we may lose
 * +1 or -1 sometimes so it's not accurate. Disallow overflow
 * will be helpful.
 */
static inline bool inc_top_task_count(u8 *count)
{
	bool will_overflow = (*count == (u8)(~0));

	if (!will_overflow)
		(*count)++;

	return will_overflow;
}

static inline bool dec_top_task_count(u8 *count)
{
	bool will_overflow = (*count == 0);

	if (!will_overflow)
		(*count)--;

	return will_overflow;
}

static u32 add_top_task_load(struct rq *rq, u8 table,
				u32 load, bool prefer_idle)
{
	struct top_task_entry *top_task_table = rq->top_tasks[table];
	int index = load_to_index(load);
	u32 delta_load = 0;

	if (index == ZERO_LOAD_INDEX)
		return delta_load;

	inc_top_task_count(&top_task_table[index].count);

	/*
	 * if 0->1.
	 * This means it's the first task that fill this load index.
	 * Set the bit in top_task_bitmap and check if top_task_index
	 * needs to be updated.
	 */
	if (top_task_table[index].count == 1) {
		__set_bit(index, rq->top_tasks_bitmap[table]);

		if (index < rq->top_task_index[table]) {
			delta_load = (rq->top_task_index[table] - index)
					* SCHED_LOAD_GRANULE;
			rq->top_task_index[table] = index;
		}
	}

	if (prefer_idle)
		inc_top_task_count(&top_task_table[index].preferidle_count);

	return delta_load;
}

static void del_top_task_load(struct rq *rq, u8 table,
				u32 load, bool prefer_idle)
{
	struct top_task_entry *top_task_table = rq->top_tasks[table];
	int index = load_to_index(load);

	if (index == ZERO_LOAD_INDEX)
		return;

	dec_top_task_count(&top_task_table[index].count);

	/*
	 * if 1->0.
	 * This means currently there's no task has this load index.
	 * Clear the bit in top_task_bitmap and update top_task_index
	 * if necessary.
	 */
	if (top_task_table[index].count == 0) {
		__clear_bit(index, rq->top_tasks_bitmap[table]);

		/* Always clear preferidle_count when we are sure there's no task */
		top_task_table[index].preferidle_count = 0;

		if (index == rq->top_task_index[table]) {
			rq->top_task_index[table] = find_next_bit(
				rq->top_tasks_bitmap[table], NUM_LOAD_INDICES, index);
		}
	}

	if (prefer_idle)
		dec_top_task_count(&top_task_table[index].preferidle_count);

	return;
}

#define TOP_TASK_IGNORE_UTIL 80
#define TOP_TASK_IGNORE_LOAD \
	((u64)walt_ravg_window * TOP_TASK_IGNORE_UTIL / SCHED_CAPACITY_SCALE)

static void add_top_task(struct task_struct *p, struct rq *rq)
{
	u32 delta_load;
	u32 load = task_load_freq(p);
	bool is_top_task = (uclamp_top_task(p) > 0) && !predl_predict_hwvip_boost(p);

	if (exiting_task(p) || is_idle_task(p))
		return;

	if (load < TOP_TASK_IGNORE_LOAD)
		return;

	p->ravg.curr_load = load;

	delta_load = add_top_task_load(rq, rq->curr_table, load, is_top_task);

	if (delta_load > TOP_TASK_IGNORE_LOAD &&
	    top_task_util(rq) > capacity_curr_of(cpu_of(rq)))
		sugov_mark_util_change(cpu_of(rq), ADD_TOP_TASK);

	trace_perf(walt_update_top_task, rq, p);
}

static void
migrate_top_task(struct task_struct *p, struct rq *src_rq, struct rq *dest_rq)
{
	bool is_top_task = (uclamp_top_task(p) > 0) && !predl_predict_hwvip_boost(p);

#ifdef CONFIG_SCHED_DOWNMIGRATE_LOWER_LOAD
	/* Clear p's top task load in downmigrate case */
	if (capacity_orig_of(cpu_of(src_rq)) > capacity_orig_of(cpu_of(dest_rq))) {
		del_top_task_load(src_rq, curr_table(src_rq), p->ravg.curr_load, is_top_task);
		del_top_task_load(src_rq, prev_table(src_rq), p->ravg.prev_load, is_top_task);
		p->ravg.curr_load = p->ravg.prev_load = 0;

		trace_perf(walt_update_top_task, src_rq, p);
		return;
	}
#endif

	add_top_task_load(dest_rq, curr_table(dest_rq), p->ravg.curr_load, is_top_task);
	add_top_task_load(dest_rq, prev_table(dest_rq), p->ravg.prev_load, is_top_task);

	del_top_task_load(src_rq, curr_table(src_rq), p->ravg.curr_load, is_top_task);
	del_top_task_load(src_rq, prev_table(src_rq), p->ravg.prev_load, is_top_task);

	trace_perf(walt_update_top_task, src_rq, p);
	trace_perf(walt_update_top_task, dest_rq, p);
}

void top_task_exit(struct task_struct *p, struct rq *rq)
{
	/*
	 * We'd better not clear p in top task here, unless we do walt update
	 * before top_task_exit. For now, leave exit task as if it was still
	 * on that cpu.
	 */
	return;
}

static inline void clear_top_tasks_table(struct top_task_entry *table)
{
	memset(table, 0, NUM_LOAD_INDICES * sizeof(struct top_task_entry));
}

static inline void clear_top_tasks_bitmap(unsigned long *bitmap)
{
	memset(bitmap, 0, BITS_TO_LONGS(NUM_LOAD_INDICES) * sizeof(unsigned long));
}

static void rollover_top_task_table(struct rq *rq, int nr_full_windows)
{
	u8 curr_table = rq->curr_table;
	u8 prev_table = 1 - curr_table;

	/* Clear prev table */
	clear_top_tasks_table(rq->top_tasks[prev_table]);
	clear_top_tasks_bitmap(rq->top_tasks_bitmap[prev_table]);
	rq->top_task_index[prev_table] = ZERO_LOAD_INDEX;

	/* Also clear curr table if last window is empty */
	if (nr_full_windows) {
		rq->top_task_index[curr_table] = ZERO_LOAD_INDEX;
		clear_top_tasks_table(rq->top_tasks[curr_table]);
		clear_top_tasks_bitmap(rq->top_tasks_bitmap[curr_table]);
	}

	/* Finally exchange rq's curr_table & prev_table pointer.
	 * This means let curr to be prev, and let an empty one to be curr.
	 *
	 * new tables:               curr                prev
	 *                            |                   |
	 * old tables:   prev_table(must be cleared)  curr_table
	 */
	rq->curr_table = prev_table;
}

static void rollover_top_task_load(struct task_struct *p, int nr_full_windows)
{
	int curr_load = 0;

	if (!nr_full_windows)
		curr_load = p->ravg.curr_load;

	p->ravg.prev_load = curr_load;
	p->ravg.curr_load = 0;
}

void top_task_rq_init(struct rq *rq)
{
	int j;

	rq->curr_table = 0;

	for (j = 0; j < NUM_TRACKED_WINDOWS; j++) {
		rq->top_task_index[j] = ZERO_LOAD_INDEX;
		rq->top_tasks[j] = kcalloc(
			NUM_LOAD_INDICES, sizeof(struct top_task_entry), GFP_NOWAIT);

		/* No other choice */
		BUG_ON(!rq->top_tasks[j]);
	}

	rq->top_task_hist_size = DEFAULT_TOP_TASK_HIST_SIZE;
	rq->top_task_stats_policy = DEFAULT_TOP_TASK_STATS_POLICY;
	rq->top_task_stats_empty_window = DEFAULT_TOP_TASK_STATS_EMPTY_WINDOW;
}
#endif /* CONFIG_SCHED_TOP_TASK */

static inline void fixup_cum_window_demand(struct rq *rq, s64 delta)
{
#ifdef CONFIG_SCHED_CALC_CUM_WINDOW_DEMAND
	rq->cum_window_demand += delta;
	if (unlikely((s64)rq->cum_window_demand < 0))
		rq->cum_window_demand = 0;
#endif
}

#ifdef CONFIG_SCHED_PRED_LOAD
static void fixup_pred_load(struct rq *rq, s64 pred_load_delta);
#endif

void
walt_inc_cumulative_runnable_avg(struct rq *rq,
				 struct task_struct *p)
{
	rq->cumulative_runnable_avg += p->ravg.demand;

	/*
	 * Add a task's contribution to the cumulative window demand when
	 *
	 * (1) task is enqueued with on_rq = 1 i.e migration,
	 *     prio/cgroup/class change.
	 * (2) task is waking for the first time in this window.
	 */
	if (p->on_rq || (p->last_sleep_ts < rq->window_start))
		fixup_cum_window_demand(rq, p->ravg.demand);

#ifdef CONFIG_SCHED_PRED_LOAD
	fixup_pred_load(rq, p->ravg.predl);

	/*
	 * sugov_mark_util_change() will check if it need to respond to
	 * PRED_LOAD_ENQUEUE.
	 */
	if (predict_util_sum(rq) > capacity_curr_of(cpu_of(rq)))
		sugov_mark_util_change(cpu_of(rq), PRED_LOAD_ENQUEUE);
#endif
}

void
walt_dec_cumulative_runnable_avg(struct rq *rq,
				 struct task_struct *p)
{
	rq->cumulative_runnable_avg -= p->ravg.demand;
	BUG_ON((s64)rq->cumulative_runnable_avg < 0);

	/*
	 * on_rq will be 1 for sleeping tasks. So check if the task
	 * is migrating or dequeuing in RUNNING state to change the
	 * prio/cgroup/class.
	 */
	if (task_on_rq_migrating(p) || p->state == TASK_RUNNING)
		fixup_cum_window_demand(rq, -(s64)p->ravg.demand);

#ifdef CONFIG_SCHED_PRED_LOAD
	fixup_pred_load(rq, -(s64)p->ravg.predl);
#endif
}

void
walt_fixup_cumulative_runnable_avg(struct rq *rq,
				   struct task_struct *p, u64 new_task_load)
{
	s64 task_load_delta = (s64)new_task_load - task_load(p);

	rq->cumulative_runnable_avg += task_load_delta;
	if ((s64)rq->cumulative_runnable_avg < 0)
		panic("cra less than zero: tld: %lld, task_load(p) = %u\n",
			task_load_delta, task_load(p));

	fixup_cum_window_demand(rq, task_load_delta);
}

u64 walt_ktime_clock(void)
{
	if (unlikely(walt_ktime_suspended))
		return ktime_to_ns(ktime_last);
	return ktime_get_ns();
}

static void walt_resume(void)
{
	walt_ktime_suspended = false;
}

static int walt_suspend(void)
{
	ktime_last = ktime_get();
	walt_ktime_suspended = true;
	return 0;
}

static struct syscore_ops walt_syscore_ops = {
	.resume	= walt_resume,
	.suspend = walt_suspend
};

static int __init walt_init_ops(void)
{
	register_syscore_ops(&walt_syscore_ops);
	return 0;
}
late_initcall(walt_init_ops);

#ifdef CONFIG_CFS_BANDWIDTH
void walt_inc_cfs_cumulative_runnable_avg(struct cfs_rq *cfs_rq,
		struct task_struct *p)
{
	cfs_rq->cumulative_runnable_avg += p->ravg.demand;
}

void walt_dec_cfs_cumulative_runnable_avg(struct cfs_rq *cfs_rq,
		struct task_struct *p)
{
	cfs_rq->cumulative_runnable_avg -= p->ravg.demand;
}
#endif

#ifdef CONFIG_SCHED_WALT_WINDOW_SIZE_TUNABLE
static int __init set_walt_ravg_window(char *str)
{
	unsigned int adj_window;
	bool no_walt = walt_disabled;

	get_option(&str, &walt_ravg_window);

	/* Adjust for CONFIG_HZ */
	adj_window = (walt_ravg_window / TICK_NSEC) * TICK_NSEC;

	/* Warn if we're a bit too far away from the expected window size */
	WARN(adj_window < walt_ravg_window - NSEC_PER_MSEC,
	     "tick-adjusted window size %u, original was %u\n", adj_window,
	     walt_ravg_window);

	walt_ravg_window = adj_window;

	walt_disabled = walt_disabled ||
			(walt_ravg_window < MIN_SCHED_RAVG_WINDOW ||
			 walt_ravg_window > MAX_SCHED_RAVG_WINDOW);

	WARN(!no_walt && walt_disabled,
	     "invalid window size, disabling WALT\n");

	return 0;
}

early_param("walt_ravg_window", set_walt_ravg_window);
#endif

static bool
update_window_start(struct rq *rq, u64 wallclock)
{
	s64 delta;
	int nr_windows;
	bool walt_lws_forwarded = false;
#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
	u64 ret;
	u64 old_window_start = rq->window_start;
#endif

	delta = wallclock - rq->window_start;
	/* If the MPM global timer is cleared, set delta as 0 to avoid kernel BUG happening */
	if (delta < 0) {
		delta = 0;
		WARN_ONCE(1, "WALT wallclock appears to have gone backwards or reset\n");
	}

	if (delta < walt_ravg_window)
		return walt_lws_forwarded;

	nr_windows = div64_u64(delta, walt_ravg_window);
	rq->window_start += (u64)nr_windows * (u64)walt_ravg_window;

#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
	ret = atomic64_cmpxchg(&walt_latest_window_start, old_window_start, rq->window_start);
	if (ret == old_window_start)
		walt_lws_forwarded = true;
#endif

#ifdef CONFIG_SCHED_CALC_CUM_WINDOW_DEMAND
	rq->cum_window_demand = rq->cumulative_runnable_avg;
#endif

	return walt_lws_forwarded;
}

#ifdef CONFIG_SCHED_PRED_LOAD
/*
 * Be careful to have our predl_window_size tunnable.
 * To get all rq's predl_window_start synchronized(at least when
 * predl_window_size is 'steady'), let predl_window_start be
 * n * predl_window_size;
 */
static void
update_predl_window_start(struct rq *rq, u64 wallclock)
{
	s64 delta;
	u64 window_size = predl_window_size;

	delta = wallclock - rq->predl_window_start;
	if (delta < window_size)
		return;

	rq->predl_window_start = div64_u64(wallclock, window_size) * window_size;
}
#endif

static inline u32 max_runtime(struct task_struct *p)
{
	u64 max_util = get_task_max_util(p);

	if (likely(max_util == SCHED_CAPACITY_SCALE))
		return walt_ravg_window;
	return (max_util * walt_ravg_window) >> SCHED_CAPACITY_SHIFT;
}

#ifdef CONFIG_SCHED_PRED_LOAD
static inline u32 predl_max_runtime(struct task_struct *p)
{
	u64 max_util = get_task_max_util(p);

	if (likely(max_util == SCHED_CAPACITY_SCALE))
		return predl_window_size;
	return (max_util * predl_window_size) >> SCHED_CAPACITY_SHIFT;
}
#endif

/*
 * Translate absolute delta time accounted on a CPU
 * to a scale where 1024 is the capacity of the most
 * capable CPU running at FMAX
 */
static u64 scale_exec_time(u64 delta, struct rq *rq, s32 limit, int flag)
{
	unsigned long capcurr = capacity_curr_of(cpu_of(rq));

#ifdef CONFIG_SCHED_SMT
	/*
	 * TASK_DC:
	 * task_util(p) is compared with capacity_of(cpu). They are both
	 * scaled 65% when MT.
	 * CPU_DC:
	 * walt_cpu_util_freq(cpu) is defined as cpu's prs + sibling_cpu's
	 * prs. So that prs for a cpu is scaled 50% when MT to make
	 * walt_cpu_util_freq(cpu) as the non-scaled util of the physical cpu.
	 * TOP_TASK_DC:
	 * top_task_util(rq) is used together with walt_cpu_util_freq(cpu).
	 * They indicate if the prev cpufreq satisfies our need. We don't
	 * want the real cost in a window but just need to know if we should
	 * increase or decrease or keep our cpufreq if everything is the same
	 * in the next window.
	 * The top_taks_util() and walt_cpu_util_freq() are compared with
	 * capacity_curr_of(). None of them is scaled in MT.
	 * So TOP_TASK_DC should be 100%.
	 */
	if (rq->is_smt)
		capcurr = capcurr * rq->smt_cap_discount[flag] / 100;
#endif

	delta = (delta * capcurr) >> SCHED_CAPACITY_SHIFT;

	if (limit < 0)
		limit = 0;
	if (delta > limit)
		delta = limit;
	return delta;
}

static int cpu_is_waiting_on_io(struct rq *rq)
{
#ifdef CONFIG_CPU_FREQ_GOV_SCHEDUTIL_OPT
	if (!walt_io_is_busy && !sched_io_is_busy)
		return 0;
#else
	if (!walt_io_is_busy)
		return 0;
#endif
	return atomic_read(&rq->nr_iowait);
}

void walt_account_irqtime(int cpu, struct task_struct *curr,
				 u64 delta, u64 wallclock)
{
	struct rq *rq = cpu_rq(cpu);
	unsigned long flags, nr_windows;
	u64 cur_jiffies_ts;

	raw_spin_rq_lock_irqsave(rq, flags);

	/*
	 * cputime (wallclock) uses sched_clock so use the same here for
	 * consistency.
	 */
	delta += sched_clock() - wallclock;
	cur_jiffies_ts = get_jiffies_64();

	if (is_idle_task(curr))
		walt_update_task_ravg(curr, rq, IRQ_UPDATE, walt_ktime_clock(),
				 delta);

	nr_windows = cur_jiffies_ts - rq->irqload_ts;

	if (nr_windows) {
		if (nr_windows < 10) {
			/* Decay CPU's irqload by 3/4 for each window. */
			rq->avg_irqload *= (3 * nr_windows);
			rq->avg_irqload = div64_u64(rq->avg_irqload,
						    4 * nr_windows);
		} else {
			rq->avg_irqload = 0;
		}
		rq->avg_irqload += rq->cur_irqload;
		rq->cur_irqload = 0;
	}

	rq->cur_irqload += delta;
	rq->irqload_ts = cur_jiffies_ts;
	raw_spin_rq_unlock_irqrestore(rq, flags);
}

#define WALT_HIGH_IRQ_TIMEOUT (20L * CONFIG_HZ / MSEC_PER_SEC)

u64 walt_irqload(int cpu)
{
	struct rq *rq = cpu_rq(cpu);
	s64 delta;
	delta = get_jiffies_64() - rq->irqload_ts;

	/*
	 * Current context can be preempted by irq and rq->irqload_ts can be
	 * updated by irq context so that delta can be negative.
	 * But this is okay and we can safely return as this means there
	 * was recent irq occurrence.
	 */

	if (delta < WALT_HIGH_IRQ_TIMEOUT)
		return rq->avg_irqload;
	else
		return 0;
}

int walt_cpu_high_irqload(int cpu)
{
#ifdef CONFIG_SCHED_CHECK_IRQLOAD
	return walt_irqload(cpu) >= sysctl_sched_walt_cpu_high_irqload;
#else
	return 0;
#endif
}

int walt_cpu_overload_irqload(int cpu)
{
#ifdef CONFIG_SCHED_CHECK_IRQLOAD
	return walt_irqload(cpu) >= sysctl_sched_walt_cpu_overload_irqload;
#else
	return 0;
#endif
}

#ifdef CONFIG_SCHED_TOP_TASK
static void top_task_load_update_history(struct rq *rq, struct task_struct *p,
					 u32 runtime, int samples, int event)
{
	u32 *hist;
	int ridx, widx;
	u32 max = 0, avg, load_avg;
	u64 sum = 0;
	unsigned int hist_size, stats_policy;

	hist = &p->ravg.load_sum_history[0];
	hist_size = rq->top_task_hist_size;
	stats_policy = rq->top_task_stats_policy;

	/* Clear windows out of hist_size */
	widx = RAVG_HIST_SIZE_MAX - 1;
	for (; widx >= hist_size; --widx)
		hist[widx] = 0;

	/* Push new 'runtime' value onto stack */
	ridx = widx - samples;
	for (; ridx >= 0; --widx, --ridx) {
		hist[widx] = hist[ridx];
		sum += hist[widx];
		if (hist[widx] > max)
			max = hist[widx];
	}

	for (widx = 0; widx < samples && widx < hist_size; widx++) {
		hist[widx] = runtime;
		sum += hist[widx];
		if (hist[widx] > max)
			max = hist[widx];
	}

	p->ravg.load_sum = 0;

	if (stats_policy == WINDOW_STATS_RECENT) {
		load_avg = runtime;
	} else if (stats_policy == WINDOW_STATS_MAX) {
		load_avg = max;
	} else {
		avg = div64_u64(sum, hist_size);
		if (stats_policy == WINDOW_STATS_AVG)
			load_avg = avg;
		else
			load_avg = max(avg, runtime);
	}

	p->ravg.load_avg = load_avg;

	return;
}

static void update_top_task_load(struct task_struct *p, struct rq *rq,
				int event, u64 wallclock, bool account_busy)
{
	u64 mark_start = p->ravg.mark_start;
	u64 delta, window_start = rq->window_start;
	int nr_full_windows;
	u32 window_size = walt_ravg_window;

	/* No need to bother updating task load for exiting tasks
	 * or the idle task. */
	if (exiting_task(p) || is_idle_task(p))
		return;

	delta = window_start - mark_start;
	nr_full_windows = div64_u64(delta, window_size);

	if (!account_busy) {
		/* Complete last non-empty window */
		top_task_load_update_history(rq, p, p->ravg.load_sum, 1, event);

		/* Push empty windows into history if wanted */
		if (nr_full_windows && rq->top_task_stats_empty_window)
			top_task_load_update_history(rq, p, 0, nr_full_windows, event);

		return;
	}

	/* Temporarily rewind window_start to first window boundary
	 * after mark_start. */
	window_start -= (u64)nr_full_windows * (u64)window_size;

	/* Process (window_start - mark_start) first */
	p->ravg.load_sum += scale_exec_time(window_start - mark_start, rq,
				max_runtime(p) - p->ravg.load_sum, TOP_TASK_DC);

	/* Push new sample(s) into task's load history */
	top_task_load_update_history(rq, p, p->ravg.load_sum, 1, event);
	if (nr_full_windows)
		top_task_load_update_history(rq, p,
			scale_exec_time(window_size, rq, max_runtime(p), TOP_TASK_DC),
			nr_full_windows, event);

	/* Roll window_start back to current to process any remainder
	 * in current window. */
	window_start += (u64)nr_full_windows * (u64)window_size;

	/* Process (wallclock - window_start) next */
	mark_start = window_start;
	p->ravg.load_sum += scale_exec_time(wallclock - mark_start, rq,
				max_runtime(p) - p->ravg.load_sum, TOP_TASK_DC);
}
#endif

#ifdef CONFIG_SCHED_MIGRATE_SPREAD_LOAD
static void record_task_contribution(struct task_struct *p,
					int cpu, int new_window)
{
	if (!cpumask_test_cpu(cpu, &p->ravg.curr_cpus))
		cpumask_set_cpu(cpu, &p->ravg.curr_cpus);

	if (new_window)
		cpumask_set_cpu(cpu, &p->ravg.prev_cpus);
}

static void rollover_task_contribution(struct task_struct *p,
					int nr_full_windows)
{
	if (!nr_full_windows)
		cpumask_copy(&p->ravg.prev_cpus, &p->ravg.curr_cpus);
	else
		cpumask_clear(&p->ravg.prev_cpus);

	cpumask_clear(&p->ravg.curr_cpus);
}
#endif

static int account_busy_for_cpu_time(struct rq *rq, struct task_struct *p,
				     u64 irqtime, int event)
{
	if (is_idle_task(p)) {
		/* TASK_WAKE && TASK_MIGRATE is not possible on idle task! */
		if (event == PICK_NEXT_TASK)
			return 0;

		/* PUT_PREV_TASK, TASK_UPDATE && IRQ_UPDATE are left */
		return irqtime || cpu_is_waiting_on_io(rq);
	}

	if (event == TASK_WAKE)
		return 0;

	if (event == PUT_PREV_TASK || event == IRQ_UPDATE)
		return 1;

	/*
	 * TASK_UPDATE can be called on sleeping task, when it's moved between
	 * related groups
	 */
	if (event == TASK_UPDATE) {
		if (rq->curr == p)
			return 1;

		return p->on_rq ? walt_freq_account_wait_time : 0;
	}

	/* Only TASK_MIGRATE && PICK_NEXT_TASK left */
	return walt_freq_account_wait_time;
}

/* Prevent rasing sugov work in very low load case. */
static bool should_update_freq(int cpu)
{
	struct rq *rq = cpu_rq(cpu);

	if (rq->cluster->cur_freq > rq->cluster->min_freq)
		return true;

#ifdef CONFIG_CPU_FREQ_GOV_SCHEDUTIL_OPT
	if (freq_policy_util(cpu) >
	    (capacity_curr_of(cpu) >> 1))
		return true;

	if (sugov_iowait_boost_pending(cpu))
		return true;
#endif

	/*
	 * Lowest freq and low util and no iowait boost.
	 * No need to trigger freq update.
	 */
	return false;
}

static void
mark_util_change_for_rollover(struct task_struct *p, struct rq *rq)
{
	int cpu = cpu_of(rq);

	/* Rollover condition: p is curr task and p sees new window. */
	if (p != rq->curr)
		return;

#ifdef CONFIG_SCHED_PRED_LOAD
	if (predl_enable &&
	    p->ravg.mark_start < rq->predl_window_start) {
		if (should_update_freq(cpu))
			sugov_mark_util_change(cpu, PRED_LOAD_WINDOW_ROLLOVER);

		trace_perf(predl_window_rollover, cpu);
	}
#endif

	if (p->ravg.mark_start < rq->window_start) {
		if (should_update_freq(cpu))
			sugov_mark_util_change(cpu, WALT_WINDOW_ROLLOVER);

		trace_perf(walt_window_rollover, cpu);
	}
}

static u32 empty_windows[NR_CPUS];

/*
 * Account cpu activity in its busy time counters (rq->curr/prev_runnable_sum)
 */
static void update_cpu_busy_time(struct task_struct *p, struct rq *rq,
	     int event, u64 wallclock, u64 irqtime)
{
	int new_window, nr_full_windows = 0;
	int p_is_curr_task = (p == rq->curr);
	u64 mark_start = p->ravg.mark_start;
	u64 window_start = rq->window_start;
	u32 window_size = walt_ravg_window;
	u64 delta;
	int cpu = rq->cpu;
	u64 *curr_runnable_sum = &rq->curr_runnable_sum;
	u64 *prev_runnable_sum = &rq->prev_runnable_sum;
#ifdef CONFIG_SCHED_RTG
	struct group_cpu_time *cpu_time;
	struct related_thread_group *grp;
#endif

	new_window = mark_start < window_start;
	if (new_window) {
		nr_full_windows = div64_u64((window_start - mark_start),
						window_size);
		if (p->ravg.active_windows < USHRT_MAX)
			p->ravg.active_windows++;

#ifdef CONFIG_SCHED_TOP_TASK
		if (p_is_curr_task)
			rollover_top_task_table(rq, nr_full_windows);
#endif
	}

	/* A new window has started. The RQ demand must be rolled
	 * over if p is the current task. */
	if (p_is_curr_task && new_window) {
		u64 curr_sum = rq->curr_runnable_sum;

		if (nr_full_windows)
			curr_sum = 0;

		rq->prev_runnable_sum = curr_sum;
		rq->curr_runnable_sum = 0;
	}

#ifdef CONFIG_SCHED_RTG
	rcu_read_lock();
	grp = task_related_thread_group(p);
	rcu_read_unlock();
	if (grp && grp->mode.freq_enabled && rq->cluster == grp->preferred_cluster) {
		cpu_time = group_update_cpu_time(rq, p->grp);
		if (cpu_time) {
			curr_runnable_sum = &cpu_time->curr_runnable_sum;
			prev_runnable_sum = &cpu_time->prev_runnable_sum;
		}
	}
#endif

	/* Handle per-task window rollover. We don't care about the idle
	 * task or exiting tasks. */
	if (new_window && !is_idle_task(p) && !exiting_task(p)) {
		u32 *curr_cpu_windows = empty_windows;
		u32 curr_window = 0;
		int i;

		if (!nr_full_windows) {
			curr_window = p->ravg.curr_window;
			curr_cpu_windows = p->ravg.curr_window_cpu;
		}

		p->ravg.prev_window = curr_window;
		p->ravg.curr_window = 0;

#ifdef CONFIG_SCHED_TOP_TASK
		rollover_top_task_load(p, nr_full_windows);
#endif

#ifdef CONFIG_SCHED_MIGRATE_SPREAD_LOAD
		rollover_task_contribution(p, nr_full_windows);
#endif

		/* Roll over individual CPU contributions */
		for (i = 0; i < nr_cpu_ids; i++) {
			p->ravg.prev_window_cpu[i] = curr_cpu_windows[i];
			p->ravg.curr_window_cpu[i] = 0;
		}
	}

	if (!account_busy_for_cpu_time(rq, p, irqtime, event)) {
		/* account_busy_for_cpu_time() = 0, so no update to the
		 * task's current window needs to be made. This could be
		 * for example
		 *
		 *   - a wakeup event on a task within the current
		 *     window (!new_window below, no action required),
		 *   - switching to a new task from idle (PICK_NEXT_TASK)
		 *     in a new window where irqtime is 0 and we aren't
		 *     waiting on IO */

		if (!new_window)
			return;

#ifdef CONFIG_SCHED_TOP_TASK
		update_top_task_load(p, rq, event, wallclock, false);
		add_top_task(p, rq);
#endif
		return;
	}

#ifdef CONFIG_SCHED_MIGRATE_SPREAD_LOAD
	record_task_contribution(p, cpu, new_window);
#endif

	if (!new_window) {
		/* account_busy_for_cpu_time() = 1 so busy time needs
		 * to be accounted to the current window. No rollover
		 * since we didn't start a new window. An example of this is
		 * when a task starts execution and then sleeps within the
		 * same window. */

		if (!irqtime || !is_idle_task(p) || cpu_is_waiting_on_io(rq))
			delta = wallclock - mark_start;
		else
			delta = irqtime;

#ifdef CONFIG_SCHED_TOP_TASK
		if (!is_idle_task(p) && !exiting_task(p))
			p->ravg.load_sum += scale_exec_time(delta, rq,
				max_runtime(p) - p->ravg.curr_window, TOP_TASK_DC);
#endif

		delta = scale_exec_time(delta, rq,
				max_runtime(p) - p->ravg.curr_window, CPU_DC);
		*curr_runnable_sum += delta;
		if (!is_idle_task(p) && !exiting_task(p)) {
			p->ravg.curr_window += delta;
			p->ravg.curr_window_cpu[cpu] += delta;
		}

		return;
	}

#ifdef CONFIG_SCHED_TOP_TASK
	update_top_task_load(p, rq, event, wallclock, true);
	add_top_task(p, rq);
#endif

	if (!p_is_curr_task) {
		/* account_busy_for_cpu_time() = 1 so busy time needs
		 * to be accounted to the current window. A new window
		 * has also started, but p is not the current task, so the
		 * window is not rolled over - just split up and account
		 * as necessary into curr and prev. The window is only
		 * rolled over when a new window is processed for the current
		 * task.
		 *
		 * Irqtime can't be accounted by a task that isn't the
		 * currently running task. */

		if (!nr_full_windows) {
			/* A full window hasn't elapsed, account partial
			 * contribution to previous completed window. */
			delta = scale_exec_time(window_start - mark_start, rq,
					max_runtime(p) - p->ravg.prev_window, CPU_DC);
			if (!exiting_task(p)) {
				p->ravg.prev_window += delta;
				p->ravg.prev_window_cpu[cpu] += delta;
			}
		} else {
			/* Since at least one full window has elapsed,
			 * the contribution to the previous window is the
			 * full window (window_size). */
			delta = scale_exec_time(window_size, rq, max_runtime(p), CPU_DC);
			if (!exiting_task(p)) {
				p->ravg.prev_window = delta;
				p->ravg.prev_window_cpu[cpu] = delta;
			}
		}
		*prev_runnable_sum += delta;

		/* Account piece of busy time in the current window. */
		delta = scale_exec_time(wallclock - window_start, rq,
				max_runtime(p), CPU_DC);
		*curr_runnable_sum += delta;
		if (!exiting_task(p)) {
			p->ravg.curr_window = delta;
			p->ravg.curr_window_cpu[cpu] = delta;
		}

		return;
	}

	if (!irqtime || !is_idle_task(p) || cpu_is_waiting_on_io(rq)) {
		/* account_busy_for_cpu_time() = 1 so busy time needs
		 * to be accounted to the current window. A new window
		 * has started and p is the current task so rollover is
		 * needed. If any of these three above conditions are true
		 * then this busy time can't be accounted as irqtime.
		 *
		 * Busy time for the idle task or exiting tasks need not
		 * be accounted.
		 *
		 * An example of this would be a task that starts execution
		 * and then sleeps once a new window has begun. */

		if (!nr_full_windows) {
			/* A full window hasn't elapsed, account partial
			 * contribution to previous completed window. */
			delta = scale_exec_time(window_start - mark_start, rq,
					max_runtime(p) - p->ravg.prev_window, CPU_DC);
			if (!is_idle_task(p) && !exiting_task(p)) {
				p->ravg.prev_window += delta;
				p->ravg.prev_window_cpu[cpu] += delta;
			}
		} else {
			/* Since at least one full window has elapsed,
			 * the contribution to the previous window is the
			 * full window (window_size). */
			delta = scale_exec_time(window_size, rq,
					max_runtime(p), CPU_DC);
			if (!is_idle_task(p) && !exiting_task(p)) {
				p->ravg.prev_window = delta;
				p->ravg.prev_window_cpu[cpu] = delta;
			}
		}
		/*
		 * Rollover for normal runnable sum is done here by overwriting
		 * the values in prev_runnable_sum and curr_runnable_sum.
		 * Rollover for new task runnable sum has completed by previous
		 * if-else statement.
		 */
		*prev_runnable_sum += delta;

		/* Account piece of busy time in the current window. */
		delta = scale_exec_time(wallclock - window_start, rq,
				max_runtime(p), CPU_DC);
		*curr_runnable_sum = delta;
		if (!is_idle_task(p) && !exiting_task(p)) {
			p->ravg.curr_window = delta;
			p->ravg.curr_window_cpu[cpu] = delta;
		}

		return;
	}

	if (irqtime) {
		/* account_busy_for_cpu_time() = 1 so busy time needs
		 * to be accounted to the current window. A new window
		 * has started and p is the current task so rollover is
		 * needed. The current task must be the idle task because
		 * irqtime is not accounted for any other task.
		 *
		 * Irqtime will be accounted each time we process IRQ activity
		 * after a period of idleness, so we know the IRQ busy time
		 * started at wallclock - irqtime. */

		BUG_ON(!is_idle_task(p));
		mark_start = wallclock - irqtime;

		/* Roll window over. If IRQ busy time was just in the current
		 * window then that is all that need be accounted. */
		if (mark_start > window_start) {
			*curr_runnable_sum = scale_exec_time(irqtime, rq, S32_MAX, CPU_DC);
			return;
		}

		/* The IRQ busy time spanned multiple windows. Process the
		 * busy time preceding the current window start first. */
		delta = window_start - mark_start;
		if (delta > window_size)
			delta = window_size;
		delta = scale_exec_time(delta, rq, S32_MAX, CPU_DC);
		*prev_runnable_sum += delta;

		/* Process the remaining IRQ busy time in the current window. */
		delta = wallclock - window_start;
		*curr_runnable_sum = scale_exec_time(delta, rq, S32_MAX, CPU_DC);

		return;
	}

	BUG();
}

static int account_busy_for_task_demand(struct task_struct *p, int event)
{
	/* No need to bother updating task demand for exiting tasks
	 * or the idle task. */
	if (exiting_task(p) || is_idle_task(p))
		return 0;

	/* When a task is waking up it is completing a segment of non-busy
	 * time. Likewise, if wait time is not treated as busy time, then
	 * when a task begins to run or is migrated, it is not running and
	 * is completing a segment of non-busy time. */
	if (event == TASK_WAKE || (!walt_account_wait_time &&
			 (event == PICK_NEXT_TASK || event == TASK_MIGRATE)))
		return 0;

	return 1;
}

/*
 * Called when new window is starting for a task, to record cpu usage over
 * recently concluded window(s). Normally 'samples' should be 1. It can be > 1
 * when, say, a real-time task runs without preemption for several windows at a
 * stretch.
 */
static void update_history(struct rq *rq, struct task_struct *p,
			 u32 runtime, int samples, int event)
{
	u32 *hist = &p->ravg.sum_history[0];
	int ridx, widx;
	u32 max = 0, avg, demand;
	u64 sum = 0;

	/* Ignore windows where task had no activity */
	if (!runtime || is_idle_task(p) || exiting_task(p) || !samples)
			goto done;

	/* Push new 'runtime' value onto stack */
	widx = walt_ravg_hist_size - 1;
	ridx = widx - samples;
	for (; ridx >= 0; --widx, --ridx) {
		hist[widx] = hist[ridx];
		sum += hist[widx];
		if (hist[widx] > max)
			max = hist[widx];
	}

	for (widx = 0; widx < samples && widx < walt_ravg_hist_size; widx++) {
		hist[widx] = runtime;
		sum += hist[widx];
		if (hist[widx] > max)
			max = hist[widx];
	}

	p->ravg.sum = 0;

	if (walt_window_stats_policy == WINDOW_STATS_RECENT) {
		demand = runtime;
	} else if (walt_window_stats_policy == WINDOW_STATS_MAX) {
		demand = max;
	} else {
		avg = div64_u64(sum, walt_ravg_hist_size);
		if (walt_window_stats_policy == WINDOW_STATS_AVG)
			demand = avg;
		else
			demand = max(avg, runtime);
	}

	/*
	 * A throttled deadline sched class task gets dequeued without
	 * changing p->on_rq. Since the dequeue decrements hmp stats
	 * avoid decrementing it here again.
	 *
	 * When window is rolled over, the cumulative window demand
	 * is reset to the cumulative runnable average (contribution from
	 * the tasks on the runqueue). If the current task is dequeued
	 * already, it's demand is not included in the cumulative runnable
	 * average. So add the task demand separately to cumulative window
	 * demand.
	 */
	if (!task_has_dl_policy(p) || !p->dl.dl_throttled) {
		if (task_on_rq_queued(p))
			p->sched_class->fixup_cumulative_runnable_avg(rq, p,
								      demand);
		else if (rq->curr == p)
			fixup_cum_window_demand(rq, demand);
	}

	p->ravg.demand = demand;

done:
	trace_perf(walt_update_history, rq, p, runtime, samples, event);
	return;
}

static void add_to_task_demand(struct rq *rq, struct task_struct *p,
				u64 delta)
{
	delta = scale_exec_time(delta, rq, S32_MAX, TASK_DC);
	p->ravg.sum += delta;
	if (unlikely(p->ravg.sum > walt_ravg_window))
		p->ravg.sum = walt_ravg_window;
}

#ifdef CONFIG_TASK_RAVG_SUM
static int account_busy_for_task_ravg_sum(struct task_struct *p, int event)
{
	/* No need to bother updating task demand for exiting tasks
	 * or the idle task. */
	if (exiting_task(p) || is_idle_task(p))
		return 0;

	/* When a task is waking up it is completing a segment of non-busy
	 * time. Likewise, if wait time is not treated as busy time, then
	 * when a task begins to run or is migrated, it is not running and
	 * is completing a segment of non-busy time. */
	if (event == TASK_WAKE || event == PICK_NEXT_TASK || event == TASK_MIGRATE)
		return 0;

	return 1;
}

static void add_to_task_ravg_sum(struct rq *rq, struct task_struct *p,
				u64 delta, int event)
{
	if (!account_busy_for_task_ravg_sum(p, event))
		return ;

	delta = scale_exec_time(delta, rq, S32_MAX, TASK_DC);
	p->ravg.ravg_sum += delta;
#ifdef CONFIG_SCHED_CPULOAD
	atomic_long_add(delta, &p->signal->ravg_sum);
	rq->ravg_sum += delta;
#endif
}
#else
static inline void add_to_task_ravg_sum(struct rq *rq, struct task_struct *p, u64 delta, int event) {}
#endif

#ifdef CONFIG_SCHED_RTG
static void add_to_group_task_time(struct related_thread_group *grp,
				   struct rq *rq, struct task_struct *p,
				   u64 wallclock)
{
	u64 mark_start = p->ravg.mark_start;
	u64 window_start = grp->window_start;
	u64 delta_exec, delta_load;

	if (unlikely(wallclock <= mark_start))
		return;

	/* per task load tracking in RTG */
	if (likely(mark_start >= window_start)) {
		/*
		 *   ws   ms  wc
		 *   |    |   |
		 *   V    V   V
		 *   |---------------|
		 */
		delta_exec = wallclock - mark_start;
		p->ravg.curr_window_exec += delta_exec;

		delta_load = scale_exec_time(delta_exec, rq, S32_MAX, TASK_DC);
		p->ravg.curr_window_load += delta_load;
	} else {
		/*
		 *   ms   ws  wc
		 *   |    |   |
		 *   V    V   V
		 *   -----|----------
		 */
		/* prev window task statistic */
		delta_exec = window_start - mark_start;
		p->ravg.prev_window_exec += delta_exec;

		delta_load = scale_exec_time(delta_exec, rq, S32_MAX, TASK_DC);
		p->ravg.prev_window_load += delta_load;

		/* curr window task statistic */
		delta_exec = wallclock - window_start;
		p->ravg.curr_window_exec += delta_exec;

		delta_load = scale_exec_time(delta_exec, rq, S32_MAX, TASK_DC);
		p->ravg.curr_window_load += delta_load;
	}
}

static void add_to_group_time(struct related_thread_group *grp,
			      struct rq *rq, u64 wallclock)
{
	u64 delta_exec, delta_load;
	u64 mark_start = grp->mark_start;
	u64 window_start = grp->window_start;
	bool on_pref_cluster = (rq->cluster == grp->preferred_cluster);

	if (unlikely(wallclock <= mark_start))
		return;

	/* per group load tracking in RTG */
	if (likely(mark_start >= window_start)) {
		/*
		 *   ws   ms  wc
		 *   |    |   |
		 *   V    V   V
		 *   |---------------|
		 */
		delta_exec = wallclock - mark_start;
		grp->time.curr_window_exec += delta_exec;

		delta_load = scale_exec_time(delta_exec, rq, S32_MAX, CPU_DC);
		grp->time.curr_window_load += delta_load;

		if (on_pref_cluster) {
			grp->time_pref_cluster.curr_window_exec += delta_exec;
			grp->time_pref_cluster.curr_window_load += delta_load;
		}
	} else {
		/*
		 *   ms   ws  wc
		 *   |    |   |
		 *   V    V   V
		 *   -----|----------
		 */
		/* prev window statistic */
		delta_exec = window_start - mark_start;
		grp->time.prev_window_exec += delta_exec;

		delta_load = scale_exec_time(delta_exec, rq, S32_MAX, CPU_DC);
		grp->time.prev_window_load += delta_load;

		if (on_pref_cluster) {
			grp->time_pref_cluster.prev_window_exec += delta_exec;
			grp->time_pref_cluster.prev_window_load += delta_load;
		}

		/* curr window statistic */
		delta_exec = wallclock - window_start;
		grp->time.curr_window_exec += delta_exec;

		delta_load = scale_exec_time(delta_exec, rq, S32_MAX, CPU_DC);
		grp->time.curr_window_load += delta_load;

		if (on_pref_cluster) {
			grp->time_pref_cluster.curr_window_exec += delta_exec;
			grp->time_pref_cluster.curr_window_load += delta_load;
		}
	}
}

static inline void add_to_group_demand(struct related_thread_group *grp,
				       struct rq *rq, struct task_struct *p,
				       u64 wallclock)
{
	if (unlikely(wallclock <= grp->window_start))
		return;

	add_to_group_task_time(grp, rq, p, wallclock);
	add_to_group_time(grp, rq, wallclock);
}

static int account_busy_for_group_demand(struct task_struct *p, int event)
{
	/* No need to bother updating task demand for exiting tasks
	 * or the idle task. */
	if (exiting_task(p) || is_idle_task(p))
		return 0;

	if (event == TASK_WAKE || event == TASK_MIGRATE)
		return 0;

	return 1;
}

static void update_group_demand(struct task_struct *p, struct rq *rq,
				int event, u64 wallclock)
{
	struct related_thread_group *grp;

	if (!account_busy_for_group_demand(p, event))
		return;

	rcu_read_lock();
	grp = task_related_thread_group(p);
	if (!grp) {
		rcu_read_unlock();
		return;
	}

	raw_spin_lock(&grp->lock);
	if (!grp->mode.util_enabled)
		goto unlock;

	if (grp->nr_running == 1)
		grp->mark_start = max(grp->mark_start, p->ravg.mark_start);

	add_to_group_demand(grp, rq, p, wallclock);

	grp->mark_start = wallclock;

unlock:
	raw_spin_unlock(&grp->lock);

	rcu_read_unlock();
}

static void update_group_nr_running(struct task_struct *p, int event,
				    u64 wallclock)
{
	struct related_thread_group *grp;
	bool need_update = false;

	rcu_read_lock();
	grp = task_related_thread_group(p);
	if (!grp) {
		rcu_read_unlock();
		return;
	}

	raw_spin_lock(&grp->lock);

	if (event == PICK_NEXT_TASK)
		grp->nr_running++;
	else if (event == PUT_PREV_TASK)
		grp->nr_running--;

	if ((int)grp->nr_running < 0) {
		WARN_ON(1);
		grp->nr_running = 0;
	}

	/* update preferred cluster if no update long */
	if (wallclock - grp->last_util_update_time > grp->util_update_timeout)
		need_update = true;

	raw_spin_unlock(&grp->lock);

	rcu_read_unlock();

	if (need_update && grp->rtg_class &&
	    grp->rtg_class->sched_update_rtg_tick != NULL)
		grp->rtg_class->sched_update_rtg_tick(grp);
}

#else
static inline void update_group_demand(struct task_struct *p, struct rq *rq, int event, u64 wallclock) { }
static inline void update_group_nr_running(struct task_struct *p, int event, u64 wallclock) { }
#endif

#ifdef CONFIG_SCHED_PRED_LOAD
/*
 * bucket_increase - update the count of all buckets
 *
 * @buckets: array of buckets tracking busy time of a task
 * @idx: the index of bucket to be incremented
 *
 * Each time a complete window finishes, count of bucket that runtime
 * falls in (@idx) is incremented. Counts of all other buckets are
 * decayed. The rate of increase and decay could be different based
 * on current count in the bucket.
 *
 * Current implementation is quite simple: the bucket tells if the
 * task once fell into it in the past 30 windows. In other words a
 * load idx fade from history after 30 windows since last kick.
 */
#define BUCKET_FADE_WINDOWS 30
static void bucket_increase(struct task_struct *p, int idx)
{
	int i;
	u8 *buckets = p->ravg.predl_busy_buckets;
	unsigned long *bitmap = p->ravg.predl_busy_buckets_bitmap;

	if (buckets[idx] == 0)
		__set_bit(idx, bitmap);
	buckets[idx] = BUCKET_FADE_WINDOWS;

	for (i = -1;
	     i = find_next_bit(bitmap, NUM_BUSY_BUCKETS, i+1),
	     i < NUM_BUSY_BUCKETS;) {
		if (i == idx)
			continue;

		if (buckets[i] == 1)
			__clear_bit(i, bitmap);
		if (buckets[i] > 0)
			buckets[i]--;
	}

	trace_perf(bucket_increase, p, idx);
}

static int busy_to_bucket(u32 normalized_rt)
{
	int bidx;

	bidx = mult_frac(normalized_rt, NUM_BUSY_BUCKETS, predl_window_size);
	bidx = min(bidx, NUM_BUSY_BUCKETS - 1);

	return bidx;
}

unsigned int predl_jump_boost = 8;
static u32 predl_boosted_runtime(u32 runtime)
{
	u32 margin;

	margin = max_t(s32, predl_window_size - runtime, 0);
	margin = mult_frac(margin, predl_jump_boost, 100);
	return runtime + margin;
}

static bool slow_cpus_low_freq(void)
{
	struct sched_cluster *cluster = min_cap_cluster();

	return cluster->cur_freq < cluster->min_freq + cluster->min_freq / 2;
}

#ifdef CONFIG_SCHED_MIGRATE_SPREAD_LOAD
static bool task_only_stay_slow_cpus(struct task_struct *p)
{
	cpumask_t recent_cpus;
	cpumask_t slow_cpus;

	cpumask_or(&recent_cpus, &p->ravg.prev_cpus, &p->ravg.curr_cpus);
	get_slow_cpus(&slow_cpus);
	return cpumask_subset(&recent_cpus, &slow_cpus);
}
#else
/* We can find other ways if SCHED_MIGRATE_SPREAD_LOAD is disabled someday. */
static bool task_only_stay_slow_cpus(struct task_struct *p)
{
	return false;
}
#endif

#define IGNORE_LOW_UTIL 20UL

/*
 * The most important thing here is that the runtime is inaccurate, mainly
 * because the normalized runtime of the real world load is not frequency/
 * capacity invariant.
 * The best way would be tracking instructions and doing rollover by vsync.
 *
 * Now we return if the runtime is larger than upper_bound_of_highest_bucket
 * * 125% or larger than max_of_recent_5_windows * 133% when we know the task
 * runs at the loweset opps (so that the runtime is more accurate).
 * The first condition prevents us from mispredicting very often. When high
 * runtime occured once it will fill in the bucket and is not supposed to
 * boost for the same bucket load again until the bucket fades.
 * And the second condition helps us to boost when we really need it. The
 * app who needs us the most run at the lowest opp. The first condition
 * may not satisfy because of the granule or the mispredict in the past.
 * So we do another check carefully to help the case we want the most when
 * the first condition doesn't work. Note that it's possible to boost very
 * often, though I belive I've been careful enough.
 */
static bool deviate_from_history(struct task_struct *p, u32 runtime)
{
	unsigned long *bitmap = p->ravg.predl_busy_buckets_bitmap;
	int idx;

	if (((u64)runtime << SCHED_CAPACITY_SHIFT) <
	    (IGNORE_LOW_UTIL * predl_window_size))
		return false;

	idx = busy_to_bucket(runtime - runtime / 5);
	if (find_next_bit(bitmap, NUM_BUSY_BUCKETS, idx) >= NUM_BUSY_BUCKETS)
		return true;

	if (slow_cpus_low_freq() && task_only_stay_slow_cpus(p)) {
		u32 *hist = p->ravg.predl_sum_history;
		u32 hist_max = 0;
		int i;

		for (i = 0; i < RAVG_HIST_SIZE_MAX; i++)
			hist_max = max(hist_max, hist[i]);

		if (runtime > hist_max + hist_max / 3)
			return true;
	}

	return false;
}

/*
 * get_pred_busy - calculate predicted demand for a task on runqueue
 *
 * @p: task whose prediction is being updated
 * @start: starting bucket. returned prediction should not be lower than
 *         this bucket.
 * @runtime: runtime of the task. returned prediction should not be lower
 *           than this runtime.
 * Note: @start can be derived from @runtime. It's passed in only to
 * avoid duplicated calculation in some cases.
 *
 * A new predicted busy time is returned for task @p based on @runtime
 * passed in. The function searches through buckets that represent busy
 * time equal to or bigger than @runtime and attempts to find the bucket to
 * to use for prediction. Once found, it searches through historical busy
 * time and returns the latest that falls into the bucket. If no such busy
 * time exists, it returns the medium of that bucket.
 */
static u32 get_pred_busy(struct task_struct *p, int start, u32 runtime)
{
	int i;
	unsigned long *buckets_bitmap = p->ravg.predl_busy_buckets_bitmap;
	u32 *hist = p->ravg.predl_sum_history;
	u32 dmin, dmax, first;
	u32 ret = runtime;

	if (unlikely(is_new_task(p)))
		goto out;

	first = find_next_bit(buckets_bitmap, NUM_BUSY_BUCKETS, start);
	if (first >= NUM_BUSY_BUCKETS) {
		if (deviate_from_history(p, runtime)) {
			ret = predl_boosted_runtime(runtime);
			trace_perf(predl_boost_runtime, p, runtime, ret);
		}

		goto out;
	}

	dmin = mult_frac(first, predl_window_size, NUM_BUSY_BUCKETS);
	dmax = mult_frac(first + 1, predl_window_size, NUM_BUSY_BUCKETS);

	for (i = 0; i < RAVG_HIST_SIZE_MAX; i++) {
		if (hist[i] >= dmin && hist[i] < dmax) {
			ret = hist[i];
			break;
		}
	}

	if (ret < dmin)
		ret = dmin;

	ret = max(runtime, ret);

out:
	ret = min(predl_max_runtime(p), ret);
	trace_perf(predl_get_busy, p, runtime, start, first, ret);
	return ret;
}

unsigned long predict_util_sum(struct rq *rq)
{
	unsigned long util, limit;

	if (!predl_enable)
		return 0;

	util = (rq->sum_pred_load << SCHED_CAPACITY_SHIFT) / predl_window_size;

	/* Get rid of predicting the cluster's highest opp. */
	limit = capacity_orig_of(cpu_of(rq)) * 90 / 100;

	return min(util, limit);
}

unsigned long predl_max_hwvip_util(struct rq *rq)
{
	u32 load;

	if (!predl_enable)
		return 0;

	load = max(rq->prev_max_vip_load, rq->curr_max_vip_load);
	return ((u64)load << SCHED_CAPACITY_SHIFT) / predl_window_size;
}

unsigned long task_pred_util(struct task_struct *p)
{
	if (!predl_enable)
		return 0;

	return ((u64)p->ravg.predl << SCHED_CAPACITY_SHIFT) / predl_window_size;
}

/*
 * For each walt_update_task_ravg event happened in the current
 * window, pick the max of the task's runtime in curr window into
 * rq->curr_max_predls.
 * When a task sees rollover, rq->prev_max_predls will be updated.
 *
 * No prediction is accounted. And migration is not accounted.
 *
 * When window rolls over and a task is runnable, we won't see the
 * task's last runnable time until next event of the task.
 * It won't affect too much for our purpose. The running task's
 * runtime is perfectly accounted at window rollover.
 * We do it in that way because we don't want to see runtime in the
 * window before prev window, like top_task_load() do.
 */
unsigned long max_pred_ls(struct rq *rq)
{
	unsigned long util;

	if (!predl_enable)
		return 0;

	util = max(rq->curr_max_predls, rq->prev_max_predls);
	util = (util << SCHED_CAPACITY_SHIFT) / predl_window_size;

	return util;
}

unsigned long cpu_util_pred_min(struct rq *rq)
{
	int i;
	u64 min_cpu_load = ULONG_MAX;

	if (!predl_enable)
		return 0;

	for_each_cpu(i, &rq->cluster->cpus)
		min_cpu_load = min(min_cpu_load, cpu_rq(i)->predl_prev_busy_time);

	return (min_cpu_load << SCHED_CAPACITY_SHIFT) / predl_window_size;
}

static void fixup_pred_load(struct rq *rq, s64 pred_load_delta)
{
	rq->sum_pred_load += pred_load_delta;
	BUG_ON((s64)rq->sum_pred_load < 0);
}

unsigned int predl_jump_load = 90;

/*
 * Most of the time, when a task takes nearly the whole window's
 * time running, it means the task's needs is not satisfied.
 * In other words it requires more than what it has taken.
 * And this is a part of the predl_do_predict, not for the basic
 * predl load tracking.
 */
static u32 predl_rollover_adjust_runtime(struct rq *rq,
			  struct task_struct *p, u32 runtime)
{
	if (p->ravg.mark_start + predl_window_size < rq->predl_window_start)
		return runtime;

	if (p->ravg.predl_time_unscaled * 100 > predl_window_size * predl_jump_load) {
		u32 orig_runtime = runtime;

		runtime = predl_boosted_runtime(runtime);
		runtime = min(runtime, predl_max_runtime(p));

		trace_perf(predl_adjust_runtime, p, orig_runtime, runtime);
	}

	return runtime;
}

static void predl_update_history(struct task_struct *p, u32 runtime)
{
	u32 *hist = &p->ravg.predl_sum_history[0];
	int i;

	for (i = RAVG_HIST_SIZE_MAX - 1; i >= 1; --i)
		hist[i] = hist[i-1];
	hist[0] = runtime;

	p->ravg.predl_sum = 0;
	p->ravg.predl_time_unscaled = 0;

	trace_perf(predl_update_history, p);
}

/*
 * Do not account task load to rq max_predls if it run on other
 * clusters at the most of time.
 */
static const unsigned int sysctl_acount_max_predls_threshold = 25;
static bool task_has_contribution(struct rq *rq, struct task_struct *p)
{
	int i;
	u64 sum_contribution = 0;

	for_each_cpu(i, &rq->cluster->cpus)
		sum_contribution += p->ravg.predl_prev_window_cpu[i];

	return sum_contribution * 100 >=
		rq->predl_prev_busy_time * sysctl_acount_max_predls_threshold;
}

static void add_to_max_predls(struct rq *rq,
			      struct task_struct *p, bool rollover)
{
	u32 runtime = p->ravg.predl_sum;
	u32 *max_predls = rollover ? &rq->prev_max_predls
				: &rq->curr_max_predls;

	if (runtime > *max_predls &&
	    task_has_contribution(rq, p))
		*max_predls = runtime;
}

static void migrate_max_vip_load(struct task_struct *p,
				struct rq *src_rq, struct rq *dst_rq)
{
	if (predl_hwvip_boost(p)) {
		src_rq->curr_max_vip_load = 0;
		dst_rq->curr_max_vip_load = max(dst_rq->curr_max_vip_load,
						p->ravg.predl);
	}
}

bool predl_hwvip_boost(struct task_struct *p)
{
#ifdef CONFIG_HW_VIP_THREAD
	return test_task_walt_predict_vip_boost(p);
#else
	return false;
#endif
}

#define PREDICT_HWVIP_BOOST (1U << 0)
bool predl_predict_hwvip_boost(struct task_struct *p)
{
	return (predl_do_predict_ctrl & PREDICT_HWVIP_BOOST) &&
		predl_hwvip_boost(p);
}

static bool predl_do_predict(struct task_struct *p)
{
	if (predl_predict_hwvip_boost(p))
		return true;

	return false;
}

/*
 * Update predl using runtime and update buckets when rollover.
 */
static void predict_and_update(struct rq *rq,
			       struct task_struct *p, bool rollover)
{
	int bidx;
	u32 runtime = p->ravg.predl_sum;
	u32 pred_load;
	bool do_predict = predl_do_predict(p);

	add_to_max_predls(rq, p, rollover);

	if (!rollover) {
		if (!do_predict)
			return;
		if (runtime <= p->ravg.predl)
			return;
	}

	if (rollover && do_predict)
		runtime = predl_rollover_adjust_runtime(rq, p, runtime);

	if (do_predict) {
		bidx = busy_to_bucket(runtime);
		pred_load = get_pred_busy(p, bidx, runtime);

		if (rollover)
			bucket_increase(p, bidx);
	} else {
		pred_load = runtime;
	}

	if (rollover)
		predl_update_history(p, runtime);

	if (task_on_rq_queued(p))
		fixup_pred_load(rq, (s64)pred_load - p->ravg.predl);

	if (predl_hwvip_boost(p))
		rq->curr_max_vip_load = max(rq->curr_max_vip_load, pred_load);

	p->ravg.predl = pred_load;

	if (task_pred_util(p) > capacity_curr_of(cpu_of(rq)))
		sugov_mark_util_change(cpu_of(rq), PRED_LOAD_CHANGE);
}

static void add_to_task_predl(struct rq *rq, struct task_struct *p,
			      u64 delta, int event, bool prev_window)
{
	p->ravg.predl_time_unscaled += delta;

	delta = scale_exec_time(delta, rq,
			predl_max_runtime(p) - p->ravg.predl_sum, TASK_DC);

	p->ravg.predl_sum += delta;

	/*
	 * Task predl accounts waiting time.
	 * Accumulate non-waiting times for cpu busy time.
	 */
	if (event == PUT_PREV_TASK ||
	    (event == TASK_UPDATE && rq->curr == p)) {
		if (prev_window)
			rq->predl_prev_busy_time += delta;
		else
			rq->predl_curr_busy_time += delta;
	}

	/*
	 * Account p's predl_sum on this cpu.
	 * We have rolled it over before.
	 */
	if (prev_window)
		p->ravg.predl_prev_window_cpu[cpu_of(rq)] += delta;
	else
		p->ravg.predl_curr_window_cpu[cpu_of(rq)] += delta;
}

static void rollover_cpu_predl(struct rq *rq, bool full_windows)
{
	if (full_windows) {
		rq->curr_max_predls = 0;
		rq->predl_curr_busy_time = 0;
		rq->curr_max_vip_load = 0;
	}

	rq->prev_max_predls = rq->curr_max_predls;
	rq->curr_max_predls = 0;
	rq->predl_prev_busy_time = rq->predl_curr_busy_time;
	rq->predl_curr_busy_time = 0;
	rq->prev_max_vip_load = rq->curr_max_vip_load;
	rq->curr_max_vip_load = 0;
}

static void rollover_task_predl_window_cpu(struct task_struct *p,
						bool full_windows)
{
	unsigned int i;

	for (i = 0; i < nr_cpu_ids; i++) {
		u32 curr = 0;

		if (!full_windows)
			curr = p->ravg.predl_curr_window_cpu[i];

		p->ravg.predl_prev_window_cpu[i] = curr;
		p->ravg.predl_curr_window_cpu[i] = 0;
	}
}

static bool account_busy_for_predl(int event, struct task_struct *p)
{
	switch (event) {
	case TASK_WAKE:
		/* Sleep time. */
		return false;
	case TASK_MIGRATE:
	case PICK_NEXT_TASK:
		/*
		 * Wait time, i.e. runnable. For example when p wakes
		 * binder and is preempted by it, we should take the
		 * runnable time as busy.
		 * But for hwvip boost we must be very careful to have
		 * our load stable, since hwvip boost will boost cpufreq
		 * when load increases. So do not account runnable as
		 * busy in that special case.
		 */
		return !predl_predict_hwvip_boost(p);
	default:
		/* Running time. */
		return true;
	}
}

/* Account runnable and not account empty windows. */
static void update_task_predl(struct task_struct *p,
		struct rq *rq, int event, u64 wallclock)
{
	u64 mark_start = p->ravg.mark_start;
	u64 window_start = rq->predl_window_start;
	u32 window_size = predl_window_size;
	bool new_window = (mark_start < window_start);
	bool account_busy = account_busy_for_predl(event, p);
	bool full_windows = (mark_start + window_size < window_start);

	if (!predl_enable)
		return;

	if (new_window && (p == rq->curr))
		rollover_cpu_predl(rq, full_windows);

	/*
	 * Just ignore exiting task. The rq's signal don't need to be
	 * very accurate.
	 */
	if (exiting_task(p) || is_idle_task(p))
		return;

	if (new_window)
		rollover_task_predl_window_cpu(p, full_windows);

	if (new_window && !account_busy) {
		predict_and_update(rq, p, true);
	} else if (new_window && account_busy) {
		/*
		 * It's possible to have full windows. However we do
		 * not really care about it.
		 * Treat as if it has run a full window time.
		 */
		if (full_windows) {
			p->ravg.predl_sum = 0;
			p->ravg.predl_time_unscaled = 0;
			mark_start = window_start - window_size;
		}

		add_to_task_predl(rq, p, window_start - mark_start, event, true);
		predict_and_update(rq, p, true);
		add_to_task_predl(rq, p, wallclock - window_start, event, false);
	} else if (!new_window && account_busy) {
		add_to_task_predl(rq, p, wallclock - mark_start, event, false);
		predict_and_update(rq, p, false);
	}
}
#endif /* CONFIG_SCHED_PRED_LOAD */

/*
 * Account cpu demand of task and/or update task's cpu demand history
 *
 * ms = p->ravg.mark_start;
 * wc = wallclock
 * ws = rq->window_start
 *
 * Three possibilities:
 *
 *	a) Task event is contained within one window.
 *		window_start < mark_start < wallclock
 *
 *		ws   ms  wc
 *		|    |   |
 *		V    V   V
 *		|---------------|
 *
 *	In this case, p->ravg.sum is updated *iff* event is appropriate
 *	(ex: event == PUT_PREV_TASK)
 *
 *	b) Task event spans two windows.
 *		mark_start < window_start < wallclock
 *
 *		ms   ws   wc
 *		|    |    |
 *		V    V    V
 *		-----|-------------------
 *
 *	In this case, p->ravg.sum is updated with (ws - ms) *iff* event
 *	is appropriate, then a new window sample is recorded followed
 *	by p->ravg.sum being set to (wc - ws) *iff* event is appropriate.
 *
 *	c) Task event spans more than two windows.
 *
 *		ms ws_tmp			   ws  wc
 *		|  |				   |   |
 *		V  V				   V   V
 *		---|-------|-------|-------|-------|------
 *		   |				   |
 *		   |<------ nr_full_windows ------>|
 *
 *	In this case, p->ravg.sum is updated with (ws_tmp - ms) first *iff*
 *	event is appropriate, window sample of p->ravg.sum is recorded,
 *	'nr_full_window' samples of window_size is also recorded *iff*
 *	event is appropriate and finally p->ravg.sum is set to (wc - ws)
 *	*iff* event is appropriate.
 *
 * IMPORTANT : Leave p->ravg.mark_start unchanged, as update_cpu_busy_time()
 * depends on it!
 */
static void update_task_demand(struct task_struct *p, struct rq *rq,
			       int event, u64 wallclock)
{
	u64 mark_start = p->ravg.mark_start;
	u64 delta, window_start = rq->window_start;
	int new_window, nr_full_windows;
	u32 window_size = walt_ravg_window;

	update_group_demand(p, rq, event, wallclock);

	new_window = mark_start < window_start;
	if (!account_busy_for_task_demand(p, event)) {
		if (new_window)
			/* If the time accounted isn't being accounted as
			 * busy time, and a new window started, only the
			 * previous window need be closed out with the
			 * pre-existing demand. Multiple windows may have
			 * elapsed, but since empty windows are dropped,
			 * it is not necessary to account those. */
			update_history(rq, p, p->ravg.sum, 1, event);
		return;
	}

	if (!new_window) {
		/* The simple case - busy time contained within the existing
		 * window. */
		add_to_task_demand(rq, p, wallclock - mark_start);
		add_to_task_ravg_sum(rq, p, wallclock - mark_start, event);
		return;
	}

	/* Busy time spans at least two windows. Temporarily rewind
	 * window_start to first window boundary after mark_start. */
	delta = window_start - mark_start;
	nr_full_windows = div64_u64(delta, window_size);
	window_start -= (u64)nr_full_windows * (u64)window_size;

	/* Process (window_start - mark_start) first */
	add_to_task_demand(rq, p, window_start - mark_start);
	add_to_task_ravg_sum(rq, p, window_start - mark_start, event);

	/* Push new sample(s) into task's demand history */
	update_history(rq, p, p->ravg.sum, 1, event);
	if (nr_full_windows)
		update_history(rq, p,
			       scale_exec_time(window_size, rq, S32_MAX, TASK_DC),
			       nr_full_windows, event);

	/* Roll window_start back to current to process any remainder
	 * in current window. */
	window_start += (u64)nr_full_windows * (u64)window_size;

	/* Process (wallclock - window_start) next */
	mark_start = window_start;
	add_to_task_demand(rq, p, wallclock - mark_start);
	add_to_task_ravg_sum(rq, p, wallclock - mark_start, event);
}

#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
static void update_walt_ravg_window(void)
{
	int cpu;
	u64 now;
	unsigned long flags;

	if (likely(walt_ravg_window == new_walt_ravg_window))
		return;

	spin_lock_irqsave(&walt_ravg_window_lock, flags);

	for_each_possible_cpu(cpu)
		raw_spin_lock(&cpu_rq(cpu)->walt_update_lock);

	now = walt_ktime_clock();

	/*
	 * Sync everyone's window_start with old walt_ravg_window.
	 * The now must be acquired inside walt_update_lock to make sure
	 * it's the latest so that every rq's window_start must be the
	 * same after we update them to now.
	 */
	for_each_possible_cpu(cpu)
		update_window_start(cpu_rq(cpu), now);

	/*
	 * If the current window roll over is delayed such that the
	 * mark_start (current wallclock with which roll over is done)
	 * of the current task went past the window start with the
	 * updated new window size, delay the update to the next
	 * window roll over. Otherwise the CPU counters (prs and crs) are
	 * not rolled over properly as mark_start > window_start.
	 */
	if (now < this_rq()->window_start + new_walt_ravg_window) {
		trace_perf(walt_ravg_window_change, walt_ravg_window, new_walt_ravg_window);
		walt_ravg_window = new_walt_ravg_window;
	}

	for_each_possible_cpu(cpu)
		raw_spin_unlock(&cpu_rq(cpu)->walt_update_lock);

	spin_unlock_irqrestore(&walt_ravg_window_lock, flags);
}
#else
static inline void update_walt_ravg_window(void)
{
}
#endif

/* Reflect task activity on its demand and cpu's busy time statistics */
void walt_update_task_ravg(struct task_struct *p, struct rq *rq,
	     int event, u64 wallclock, u64 irqtime)
{
	unsigned long flags;
	bool walt_lws_forwarded = false;

#ifdef CONFIG_SCHED_COLOC_GROUP
	walt_update_coloc_grp_load(p, cpu_of(rq), event, wallclock);
#endif

	if (walt_disabled || !rq->window_start)
		return;

	lockdep_assert_rq_held(rq);

	raw_spin_lock_irqsave(&rq->walt_update_lock, flags);

	walt_lws_forwarded = update_window_start(rq, wallclock);
#ifdef CONFIG_SCHED_PRED_LOAD
	update_predl_window_start(rq, wallclock);
#endif
	update_group_nr_running(p, event, wallclock);

	if (!p->ravg.mark_start)
		goto done;

	update_task_demand(p, rq, event, wallclock);
#ifdef CONFIG_SCHED_PRED_LOAD
	update_task_predl(p, rq, event, wallclock);
#endif
	update_cpu_busy_time(p, rq, event, wallclock, irqtime);

	mark_util_change_for_rollover(p, rq);

done:
	raw_spin_unlock_irqrestore(&rq->walt_update_lock, flags);
	trace_perf(walt_update_task_ravg, p, rq, event, wallclock, irqtime);

	p->ravg.mark_start = wallclock;

	if (walt_lws_forwarded)
		update_walt_ravg_window();
}

void reset_task_stats(struct task_struct *p)
{
	memset(&p->ravg, 0, sizeof(struct ravg));
}

void walt_mark_task_starting(struct task_struct *p)
{
	u64 wallclock;
	struct rq *rq = task_rq(p);

	if (!rq->window_start) {
		reset_task_stats(p);
		return;
	}

	/*
	 * Add the new task to top tasks.
	 * Called in wake_up_new_task(), after task load initialized.
	 */
#ifdef CONFIG_SCHED_TOP_TASK
	add_top_task(p, rq);
#endif

	wallclock = walt_ktime_clock();
	p->ravg.mark_start = wallclock;
#ifdef CONFIG_ED_TASK
	p->last_wake_ts = wallclock;
#endif
}

void walt_set_window_start(struct rq *rq, struct rq_flags *rf)
{
	if (likely(rq->window_start))
		return;

	if (cpu_of(rq) == sync_cpu) {
		rq->window_start = 1;
#ifdef CONFIG_SCHED_PRED_LOAD
		rq->predl_window_start = 1;
#endif
#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
		atomic64_set(&walt_latest_window_start, rq->window_start);
#endif
	} else {
		struct rq *sync_rq = cpu_rq(sync_cpu);

		rq_unpin_lock(rq, rf);
		double_lock_balance(rq, sync_rq);
		rq->window_start = sync_rq->window_start;
#ifdef CONFIG_SCHED_PRED_LOAD
		rq->predl_window_start = sync_rq->predl_window_start;
#endif
		rq->curr_runnable_sum = rq->prev_runnable_sum = 0;
		raw_spin_rq_unlock(sync_rq);
		rq_repin_lock(rq, rf);
	}

	rq->curr->ravg.mark_start = rq->window_start;
}

#ifdef CONFIG_CPU_ISOLATION_OPT
void walt_migrate_sync_cpu(int cpu, int new_cpu)
{
	if (cpu == sync_cpu)
		sync_cpu = new_cpu;
}
#else
void walt_migrate_sync_cpu(int cpu)
{
	if (cpu == sync_cpu)
		sync_cpu = smp_processor_id();
}
#endif

static inline bool in_range(u64 x, u64 begin, u64 end)
{
	return begin <= x && x < end;
}

static void
move_out_cpu_busy_time(struct rq *rq, struct task_struct *p)
{
	int cpu = cpu_of(rq);
	u64 mark_start = p->ravg.mark_start;
	u64 window_start = rq->window_start;
	u32 window_size = walt_ravg_window;
	u64 window_end = window_start + window_size;

	if (in_range(mark_start, window_start, window_end)) {
		rq->curr_runnable_sum -= p->ravg.curr_window_cpu[cpu];
		rq->prev_runnable_sum -= p->ravg.prev_window_cpu[cpu];
	} else if (in_range(mark_start, window_start - window_size, window_start))
		rq->prev_runnable_sum -= p->ravg.curr_window_cpu[cpu];
	else if (in_range(mark_start, window_end, window_end + window_size))
		rq->curr_runnable_sum -= p->ravg.prev_window_cpu[cpu];

	p->ravg.curr_window_cpu[cpu] = 0;
	p->ravg.prev_window_cpu[cpu] = 0;

	if ((s64)rq->prev_runnable_sum < 0) {
#ifndef CONFIG_HISI_EAS_SCHED
		BUG_ON((s64)rq->prev_runnable_sum < 0);
#endif
		rq->prev_runnable_sum = 0;
	}

	if ((s64)rq->curr_runnable_sum < 0) {
#ifndef CONFIG_HISI_EAS_SCHED
		BUG_ON((s64)rq->curr_runnable_sum < 0);
#endif
		rq->curr_runnable_sum = 0;
	}
}

static void
migrate_cpu_busy_time(struct task_struct *p,
		      struct rq *src_rq, struct rq *dest_rq)
{
	int new_cpu = cpu_of(dest_rq);
#ifdef CONFIG_SCHED_MIGRATE_SPREAD_LOAD
	cpumask_t prev_cpus, curr_cpus;
	u32 each_load;
#endif
	unsigned long flags;
	int i;

#ifdef CONFIG_SCHED_DOWNMIGRATE_LOWER_LOAD
	/* For task downmigrate, lower task's prev/curr window to prevent
	 * little cluster's freq increase too much. */
	if (capacity_orig_of(cpu_of(src_rq)) > capacity_orig_of(new_cpu)) {
		u32 task_load = task_load_freq_avg(p);

		if (unlikely(is_new_task(p)))
			task_load = UINT_MAX;

		p->ravg.curr_window = min(p->ravg.curr_window, task_load);
		p->ravg.prev_window = min(p->ravg.prev_window, task_load);
	}
#endif

	/* Add task's prev/curr window to dest */
#ifdef CONFIG_SCHED_MIGRATE_SPREAD_LOAD
	/* If p has run on dest cluster in prev/curr window, share
	 * p's load in these cpus. */
	cpumask_and(&prev_cpus, &p->ravg.prev_cpus, &dest_rq->cluster->cpus);
	cpumask_set_cpu(new_cpu, &prev_cpus);
	each_load = p->ravg.prev_window / cpumask_weight(&prev_cpus);

	for_each_cpu(i, &prev_cpus) {
		struct rq *rq = cpu_rq(i);
		raw_spin_lock_irqsave(&rq->walt_update_lock, flags);
		rq->prev_runnable_sum += each_load;
		raw_spin_unlock_irqrestore(&rq->walt_update_lock, flags);

		p->ravg.prev_window_cpu[i] = each_load;
	}

	cpumask_and(&curr_cpus, &p->ravg.curr_cpus, &dest_rq->cluster->cpus);
	cpumask_set_cpu(new_cpu, &curr_cpus);
	each_load = p->ravg.curr_window / cpumask_weight(&curr_cpus);

	for_each_cpu(i, &curr_cpus) {
		struct rq *rq = cpu_rq(i);
		raw_spin_lock_irqsave(&rq->walt_update_lock, flags);
		rq->curr_runnable_sum += each_load;
		raw_spin_unlock_irqrestore(&rq->walt_update_lock, flags);

		p->ravg.curr_window_cpu[i] = each_load;
	}
#else
	/* All load move to dest_rq */
	raw_spin_lock_irqsave(&dest_rq->walt_update_lock, flags);
	dest_rq->curr_runnable_sum += p->ravg.curr_window;
	dest_rq->prev_runnable_sum += p->ravg.prev_window;

	p->ravg.curr_window_cpu[new_cpu] = p->ravg.curr_window;
	p->ravg.prev_window_cpu[new_cpu] = p->ravg.prev_window;
	raw_spin_unlock_irqrestore(&dest_rq->walt_update_lock, flags);
#endif

	/* Delete task's prev/curr window from src */
	for_each_cpu(i, &src_rq->cluster->cpus) {
		struct rq *rq = cpu_rq(i);
		if (!p->ravg.curr_window_cpu[i] && !p->ravg.prev_window_cpu[i])
			continue;

		raw_spin_lock_irqsave(&rq->walt_update_lock, flags);
		move_out_cpu_busy_time(rq, p);
		raw_spin_unlock_irqrestore(&rq->walt_update_lock, flags);
	}

	trace_perf(walt_migration_update_sum, src_rq, p);
	trace_perf(walt_migration_update_sum, dest_rq, p);
}

#ifdef CONFIG_MIGRATION_NOTIFY
static inline bool
nearly_same_freq(struct rq *rq, unsigned int cur_freq, unsigned int freq_required)
{
	int delta = freq_required - cur_freq;

	if (freq_required > cur_freq)
		return delta < rq->freq_inc_notify;

	delta = -delta;
	return delta < rq->freq_dec_notify;
}

static inline unsigned int estimate_freq_required(int cpu)
{
	return util_to_freq(cpu, freq_policy_util(cpu));
}
#else
static inline bool
nearly_same_freq(struct rq *rq, unsigned int cur_freq, unsigned int freq_required)
{
	return true;
}
static inline unsigned int estimate_freq_required(int cpu)
{
	return 0;
}
#endif

static void
inter_cluster_migration_fixup(struct task_struct *p,
			      struct rq *src_rq, struct rq *dest_rq)
{
#ifdef CONFIG_MIGRATION_NOTIFY
	int src_cpu = cpu_of(src_rq), dest_cpu = cpu_of(dest_rq);
	unsigned int src_freq_before, dest_freq_before;
	unsigned int src_freq_after, dest_freq_after;
	unsigned int flags;

	src_freq_before = estimate_freq_required(src_cpu);
	dest_freq_before = estimate_freq_required(dest_cpu);
#endif

#ifdef CONFIG_SCHED_TOP_TASK
	migrate_top_task(p, src_rq, dest_rq);
#endif
	migrate_cpu_busy_time(p, src_rq, dest_rq);
#ifdef CONFIG_SCHED_PRED_LOAD
	migrate_max_vip_load(p, src_rq, dest_rq);
#endif

#ifdef CONFIG_MIGRATION_NOTIFY
	src_freq_after = estimate_freq_required(src_cpu);
	dest_freq_after = estimate_freq_required(dest_cpu);

	/*
	 * To lower down overhead of sugov_work, skip triggering src
	 * cpu freq update if:
	 * 1. migrating a small load, or
	 * 2. cur_freq == min_freq and estimated_freq_before <= min_freq
	 * Note that when a big load is moved to the cluster and quickly
	 * moved away, the freq increasing might have not completed so
	 * that we see cur_freq == min_freq. Freq update should not be
	 * skipped in such case so we double check estimated_freq_before
	 * must be less than min_freq.
	 */
	if ((src_rq->cluster->cur_freq != src_rq->cluster->min_freq ||
	     src_freq_before > src_rq->cluster->min_freq) &&
	    !nearly_same_freq(src_rq, src_freq_before, src_freq_after))
		sugov_mark_util_change(src_cpu, INTER_CLUSTER_MIGRATION_SRC);

	/*
	 * Skip triggering dest cpu freq update if:
	 * 1. migrating a small load, or
	 * 2. cur_freq == max_freq, or
	 * 3. min_freq satisfies estimated_freq_after
	 * Sometimes both src cluster and dest cluster's min_freq is locked
	 * to a high freq and there are loads migrate between clusters.
	 * This situation could last long and sugov_work can be skipped to
	 * save energy by checking the 3rd condition.
	 */
	if (dest_rq->cluster->cur_freq != dest_rq->cluster->max_freq &&
	    dest_freq_after > (dest_rq->cluster->min_freq >> 1) &&
	    !nearly_same_freq(dest_rq, dest_freq_before, dest_freq_after)) {
		flags = INTER_CLUSTER_MIGRATION_DST;

		if (top_task_util(dest_rq) > capacity_curr_of(dest_cpu))
			flags |= ADD_TOP_TASK;

		sugov_mark_util_change(dest_cpu, flags);
	}
#endif
}

void walt_fixup_busy_time(struct task_struct *p, int new_cpu)
{
	struct rq *src_rq = task_rq(p);
	struct rq *dest_rq = cpu_rq(new_cpu);
	u64 wallclock;
	int src_cpu = task_cpu(p);

	if (!p->on_rq && p->state != TASK_WAKING)
		return;

#ifdef CONFIG_SCHED_COLOC_GROUP
	migrate_task_coloc_grp_contribution(p, src_cpu, new_cpu);
#endif

	if (exiting_task(p)) {
#ifdef CONFIG_ED_TASK
		clear_ed_task(p, src_rq);
#endif
		return;
	}

	if (p->state == TASK_WAKING)
		double_rq_lock(src_rq, dest_rq);

	wallclock = walt_ktime_clock();

	walt_update_task_ravg(task_rq(p)->curr, task_rq(p),
			TASK_UPDATE, wallclock, 0);
	walt_update_task_ravg(dest_rq->curr, dest_rq,
			TASK_UPDATE, wallclock, 0);

	walt_update_task_ravg(p, task_rq(p), TASK_MIGRATE, wallclock, 0);

	/*
	 * When a task is migrating during the wakeup, adjust
	 * the task's contribution towards cumulative window
	 * demand.
	 */
	if (p->state == TASK_WAKING &&
	    p->last_sleep_ts >= src_rq->window_start) {
		fixup_cum_window_demand(src_rq, -(s64)p->ravg.demand);
		fixup_cum_window_demand(dest_rq, p->ravg.demand);
	}

	if (!same_freq_domain(new_cpu, src_cpu)) {
		inter_cluster_migration_fixup(p, src_rq, dest_rq);
	} else {
#ifdef CONFIG_SCHED_TOP_TASK
		/* Only need to migrate top task when same cluster */
		migrate_top_task(p, src_rq, dest_rq);
#endif
	}

#ifdef CONFIG_ED_TASK
	migrate_ed_task(p, src_rq, dest_rq, wallclock);
#endif
	if (p->state == TASK_WAKING)
		double_rq_unlock(src_rq, dest_rq);
}

/* utility function to update walt signals at wakeup */
void walt_try_to_wake_up(struct task_struct *p)
{
	struct rq *rq = cpu_rq(task_cpu(p));
	struct rq_flags rf;
	u64 wallclock;

	rq_lock_irqsave(rq, &rf);
	wallclock = walt_ktime_clock();
	walt_update_task_ravg(rq->curr, rq, TASK_UPDATE, wallclock, 0);
	walt_update_task_ravg(p, rq, TASK_WAKE, wallclock, 0);
#ifdef CONFIG_ED_TASK
	note_task_waking(p, wallclock);
#endif
	rq_unlock_irqrestore(rq, &rf);
}

void walt_init_new_task_load(struct task_struct *p)
{
	int i;
	u32 init_load_windows =
		div64_u64((u64)sysctl_sched_walt_init_task_load_pct *
			  (u64)walt_ravg_window, 100);

	memset(&p->ravg, 0, sizeof(struct ravg));
#ifdef CONFIG_SCHED_COMMON_QOS_CTRL
	if (test_tsk_thread_flag(p, TIF_IGN_INITIAL_LOAD))
		init_load_windows = 0;
#endif
	p->ravg.demand = init_load_windows;
	for (i = 0; i < RAVG_HIST_SIZE_MAX; ++i)
		p->ravg.sum_history[i] = init_load_windows;
}

#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
void sched_window_nr_ticks_change(void)
{
	unsigned long flags;

	spin_lock_irqsave(&walt_ravg_window_lock, flags);
	new_walt_ravg_window = mult_frac(sysctl_walt_ravg_window_nr_ticks,
						NSEC_PER_SEC, HZ);
	spin_unlock_irqrestore(&walt_ravg_window_lock, flags);

}
#endif

void walt_reset_new_task_load(struct task_struct *p)
{
	int i;

	p->ravg.demand = 0;
	for (i = 0; i < RAVG_HIST_SIZE_MAX; ++i)
		p->ravg.sum_history[i] = 0;
}
