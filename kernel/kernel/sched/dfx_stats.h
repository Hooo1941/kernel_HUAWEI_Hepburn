/* SPDX-License-Identifier: GPL-2.0 */

#ifndef DFX_STATS_H
#define DFX_STATS_H

#ifdef CONFIG_DFX_STATS
static inline prio_type_t type_of_prio(int prio)
{
	prio_type_t prio_type = DFX_VIP;
	if (likely(prio >= MAX_RT_PRIO - 1)) {
		prio_type = DFX_CFS;
	} else if (prio < MAX_RT_PRIO - 1 - VIP_PRIO_WIDTH) {
		prio_type = DFX_RT;
	}

	return prio_type;
}

static inline int range_of_time(unsigned long long delta)
{
	int idx;
	for (idx = THREAD_TIME_RANGE_COUNT - 1; idx > 0; idx--) {
		if (unlikely(delta > time_distribute_range[idx]))
			return idx;
	}

	return 0;
}

/*
 * Save running time to task_struct and saving running times to rq
 */
static inline void runnable_info_update(struct rq *rq, struct task_struct *t, unsigned long long delta)
{
	prio_type_t prio_type;
	int time_index;

	t->task_dfx_info.runnable_sum += delta;
	if (likely(delta <= time_distribute_range[0]))
		return;

	prio_type = type_of_prio(t->prio);
	if (likely(prio_type == DFX_CFS && delta <= time_distribute_range[1]))
		return;

	time_index = range_of_time(delta);
	if (rq)
		rq->rq_dfx_info.runnable[prio_type].count[time_index]++;
}

static inline void dfx_info_dequeued(struct rq *rq, struct task_struct *t, unsigned long long delta)
{
	if (dfx_stats_on)
		runnable_info_update(rq, t, delta);
}

static inline void dfx_info_arrive(struct rq *rq, struct task_struct *t, unsigned long long delta)
{
	if (dfx_stats_on) {
		if (dfx_irq_preempt_stats_on && t->thread_info.preempt_count > 0)
			t->preempt_dis_start = sched_clock();
		runnable_info_update(rq, t, delta);
	}
}

/*
 * Save rt and vip running time on each cpu to rq
 */
static inline void dfx_info_depart(struct rq *rq, struct task_struct *t, unsigned long long delta)
{
	if (dfx_stats_on) {
		prio_type_t prio_type;

		if (dfx_irq_preempt_stats_on && t->thread_info.preempt_count > 0)
			t->preempt_dis_sum += sched_clock() - t->preempt_dis_start;

		prio_type = type_of_prio(t->prio);
		if (likely(prio_type == DFX_CFS))
			return;

		if (rq)
			rq->rq_dfx_info.running_time[prio_type] += delta;
	}
}

static inline void task_block_info_updata(struct task_struct *t, unsigned long long delta) {
	if (t->in_iowait) {
		t->task_dfx_info.iowait_count++;
		t->task_dfx_info.iowait_sum += delta;
	} else {
		t->task_dfx_info.block_count++;
		t->task_dfx_info.block_sum += delta;
	}
}

static inline void rq_block_info_updata(struct rq *rq, struct task_struct *t,
	unsigned long long delta, prio_type_t prio_type, int time_index)
{
	if (t->in_iowait) {
		rq->rq_dfx_info.iowait[prio_type].count[time_index]++;
		rq->rq_dfx_info.iowait[prio_type].time[time_index] += delta;
	} else {
		rq->rq_dfx_info.block[prio_type].count[time_index]++;
		rq->rq_dfx_info.block[prio_type].time[time_index] += delta;
	}
}

/*
 * Save rt block info to task_struct and rq
 */
static inline void dfx_rt_block_info_update(struct rq *rq, struct task_struct *t, unsigned long long delta)
{
	if (dfx_stats_on) {
		prio_type_t prio_type;
		int time_index;

		task_block_info_updata(t, delta);
		if (likely(delta <= time_distribute_range[1]))
			return;

		prio_type = type_of_prio(t->prio);
		time_index = range_of_time(delta);
		rq_block_info_updata(rq, t, delta, prio_type, time_index);
	}
}

static inline void dfx_fair_block_info_update(struct task_struct *t, unsigned long long delta)
{
	if (dfx_stats_on)
		task_block_info_updata(t, delta);
}

#else /* !CONFIG_DFX_STATS: */
# define dfx_info_arrive(rq, t, delta)	do { } while (0)
# define dfx_info_depart(rq, t, delta)	do { } while (0)
# define dfx_info_dequeued(rq, t, delta)       do { } while (0)
# define dfx_rt_block_info_update(rq, t, delta)	do { } while (0)
# define dfx_fair_block_info_update(t, delta)	do { } while (0)
#endif /* CONFIG_DFX_STATS: */
#endif