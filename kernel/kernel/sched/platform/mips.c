/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * mips.c
 *
 * for hisilicon efficinecy control algorithm mips load tracking.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include <platform_include/cee/linux/mips.h>
#include "sched.h"
#include "phase/phase_perf.h"
#include "phase/phase_feature.h"
#include <asm/phase_perf.h>

#define CREATE_TRACE_POINTS
#include <trace/events/mips.h>

__read_mostly unsigned int mips_enable;
__read_mostly unsigned int mips_task_report_policy = MIPS_RECENT;
__read_mostly unsigned int mips_cpu_report_policy = MIPS_AVG;
__read_mostly unsigned int mips_window_size = 4000000;

#define MIPS_SHIFT	18
#define MIPS_SCALE	(1 << MIPS_SHIFT)
DEFINE_PER_CPU(unsigned long, mips_scale) = MIPS_SCALE;

void topology_set_mips_scale(unsigned int cpu, unsigned long policy_max_freq)
{
	per_cpu(mips_scale, cpu) = per_cpu(cpu_scale, cpu) * MIPS_SCALE / policy_max_freq;
}

static inline unsigned long topology_get_mips_scale(int cpu)
{
	return per_cpu(mips_scale, cpu);
}

static inline u64 demand_recent(struct task_struct *p)
{
	return p->tmipsd->scale_inst_nr_history[1];
}

static inline u64 demand_avg(struct task_struct *p)
{
	u64 demand = 0;
	int i;

	for (i = 1; i < MIPS_HIST_SIZE_MAX; i++)
		demand += p->tmipsd->scale_inst_nr_history[i];
	return demand / (MIPS_HIST_SIZE_MAX - 1);
}

static inline u64 demand_max(struct task_struct *p)
{
	u64 demand = 0;
	int i;

	for (i = 1; i < MIPS_HIST_SIZE_MAX; i++)
		demand = max(demand, p->tmipsd->scale_inst_nr_history[i]);
	return demand;
}

u64 task_inst_demand(struct task_struct *p)
{
	if (!p->tmipsd || !mips_task_enable)
		return 0;

	u64 demand = 0;
	unsigned int report_policy = mips_task_report_policy;

	if (report_policy & MIPS_RECENT)
		demand = max(demand, demand_recent(p));
	if (report_policy & MIPS_AVG)
		demand = max(demand, demand_avg(p));
	if (report_policy & MIPS_MAX)
		demand = max(demand, demand_max(p));
	return demand;
}

u64 cpu_inst_demand(int cpu)
{
	if (!mips_task_enable)
		return 0;
	struct rq *rq = cpu_rq(cpu);
	u64 demand = 0;
	unsigned int report_policy = mips_cpu_report_policy;

	if (report_policy & MIPS_RECENT)
		demand = rq->rmipsd.curr_inst;
	if (report_policy & MIPS_AVG)
		demand = (rq->rmipsd.curr_inst + rq->rmipsd.prev_inst) / 2;
	if (report_policy & MIPS_MAX)
		demand = max_t(u64, rq->rmipsd.prev_inst, rq->rmipsd.curr_inst);
	return demand;
}

/*
 * Translate absolute cycle of task on a CPU to scale
 * where arch_get_mips_scale is one cycle of capacity
 * history [0] is the curr window before updating the window,
 * update ipc by inst and cycle's history[0] window
 */
static void update_history_mips(struct task_mips_demand *tmipsd, u64 scale_inst,
				u64 scale_cycle, int nr_windows, int cpu)
{
	int i;
	u32 real_cycle = tmipsd->scale_cycle_nr_history[0] / arch_get_mips_scale(cpu);
	u32 real_inst = tmipsd->scale_inst_nr_history[0];

	if (real_cycle != 0)
		tmipsd->curr_avg_ipc = (u64)real_inst * INST_PER_KINST / real_cycle;

	for (i = MIPS_HIST_SIZE_MAX - 1; i >= nr_windows; i--) {
		tmipsd->scale_inst_nr_history[i] = tmipsd->scale_inst_nr_history[i - nr_windows];
		tmipsd->scale_cycle_nr_history[i] = tmipsd->scale_cycle_nr_history[i - nr_windows];
	}
	for (i = i; i >= 0; i--) {
		tmipsd->scale_inst_nr_history[i] = scale_inst;
		tmipsd->scale_cycle_nr_history[i] = scale_cycle;
	}
}

static void update_rq_ipc(struct rq *rq)
{
	rq->rmipsd.prev_inst = phase_data_of_pevent(&rq->pcount_nr_history[1], INST_RETIRED);
	rq->rmipsd.curr_inst = phase_data_of_pevent(&rq->pcount_nr_history[0], INST_RETIRED);
	rq->rmipsd.curr_avg_ipc = div_u64_protected(rq->rmipsd.curr_inst * INST_PER_KINST,
						    phase_data_of_pevent(&rq->pcount_nr_history[0], CYCLES));
}

static inline void print_mips_cpu_hist(struct rq *rq, int flags)
{
	int hist_index;
	int *pevents = phase_perf_pinned_events();

	for (hist_index = 0; hist_index < NR_MIPS_WINDOW; hist_index++)
		trace_perf(mips_cpu_hist, cpu_of(rq), flags, hist_index, pevents,
				   &rq->pcount_nr_history[hist_index]);
}

/* phase update data before updating window, so it adds history[0] */
void phase_update_rq_rmipsd(struct rq *rq, struct phase_event_pcount *delta, int flags)
{
	if (!mips_enable)
		return;
	struct phase_event_pcount *curr_counter = &rq->pcount_nr_history[0];

	phase_count_merge(curr_counter, delta);
	print_mips_cpu_hist(rq, flags);
}

/* Reflect task activity on its mips demand */
void phase_update_task_tmipsd(struct task_struct *p, int cpu, u64 prev_inst,
			      u64 prev_cycle, int flags)
{
	if (!mips_enable || !p->tmipsd)
		return;

	struct task_mips_demand *tmipsd = p->tmipsd;
	u32 scale_cycle = (u32)prev_cycle * arch_get_mips_scale(cpu); // capacity per cycle
	u32 scale_inst = (u32)prev_inst;

	tmipsd->scale_inst_nr_history[0] += scale_inst;
	tmipsd->scale_cycle_nr_history[0] += scale_cycle;
	trace_perf(mips_task_inst_hist, p, flags, p->tmipsd->scale_inst_nr_history);
	trace_perf(mips_task_cycle_hist, p, flags, p->tmipsd->scale_cycle_nr_history);
}

int update_mips_window_start(u64 *window_start, u64 window_size, u64 now)
{
	s64 delta;
	int nr_windows;

	delta = now - *window_start;
	/* If the MPM global timer is cleared, set delta as 0 to avoid kernel BUG happening */
	if (delta < 0) {
		delta = 0;
		WARN_ONCE(1, "WALT wallclock appears to have gone backwards or reset\n");
	}

	if (delta < window_size)
		return 0;

	nr_windows = div64_u64(delta, window_size);
	*window_start += (u64)nr_windows * (u64)window_size;
	return nr_windows;
}

/*
 * when cpu and task update the window, it adds a empty window
 * the recent full window is the history[1] after updating window
 * history[0] is next window mips, so phase updates history[0] window
 */
void update_mips_window(struct task_struct *p, struct rq *rq, u64 now, int flags)
{
	if (!mips_enable || !p->tmipsd)
		return;

	int nr_windows = 0;
	struct task_mips_demand *tmipsd = p->tmipsd;
	int cpu = cpu_of(rq);

	/* if now is tick, cpu and task must update a new window */
	if (flags == PHASE_TASK_TICK) {
		update_history_mips(p->tmipsd, 0, 0, 1, cpu);
		update_rq_ipc(rq);
		update_history_pcount(cpu, NULL, 1);
		p->tmipsd->window_start = now;
		rq->rmipsd.mips_window_start = now;
	} else {
		/*
		 *  ws = window_start
		 *  lut = last_update_time
		 *
		 *  if now is not tick, cpu and task update new window by last_update_time
		 *
		 *	lut ws_tmp                          ws now
		 *	 |  |                               |   |
		 *	 V  V                               V   V
		 *	 ---|-------|-------|-------|-------|------
		 *	    |                               |
		 *	    |<------ nr_full_windows ------>|
		 */
		nr_windows = update_mips_window_start(&cpu_rq(cpu)->rmipsd.mips_window_start, mips_window_size, now);
		/*
		 * we do not count zero in task's mips
		 * so we only need update a new window
		 * ws > task's lut makes sure task is in a new window
		 */
		if (p->phase_info && (now - p->tmipsd->window_start > mips_window_size)) {
			update_history_mips(p->tmipsd, 0, 0, 1, cpu);
			p->tmipsd->window_start += div64_u64(now - p->tmipsd->window_start, mips_window_size) * mips_window_size;
		}
		/* we count zero in cpu's mips, so we need update new nr windows */
		if (rq->rmipsd.mips_window_start >= rq->rmipsd.mips_last_update)
			update_history_pcount(cpu, NULL, nr_windows);
	}
	if (flags == PHASE_TASK_TICK || nr_windows > 0)
		check_l3_ddr_freq_change(cpu);
	trace_perf(mips_window_time, p, rq, now, flags);
}

void mips_migrate_update(struct task_struct *p, int cpu)
{
	if (!mips_enable)
		return;

	int src_cpu = task_cpu(p);
	if (!same_freq_domain(cpu, src_cpu)) {
		load_pred_mips_freq(src_cpu, p, MIPS_MIGRATE);
		mips_check_freq_update(src_cpu, MIPS_MIGRATE);
		load_pred_mips_freq(cpu, p, MIPS_MIGRATE);
		mips_check_freq_update(cpu, MIPS_MIGRATE);
	}
}

void get_mips_mem_info(int cpu, struct pcounts *mem_info, int nr_window)
{
	int i;

	struct rq *rq = cpu_rq(cpu);
	struct phase_event_pcount pcount_sum;
	phase_clear_count(&pcount_sum);
	for (i = 1; i <= nr_window && i < NR_MIPS_WINDOW; i++)
		phase_count_merge(&pcount_sum, &rq->pcount_nr_history[i]);
	mem_info->inst = phase_data_of_pevent(&pcount_sum, INST_RETIRED);
	mem_info->cycle = phase_data_of_pevent(&pcount_sum, CYCLES);
	mem_info->l2refill = phase_data_of_pevent(&pcount_sum, L2D_CACHE_REFILL);
	if (test_slow_cpu(cpu))
		mem_info->l3refill = phase_data_of_pevent(&pcount_sum, L3D_CACHE_REFILL_C0);
	else
		mem_info->l3refill = phase_data_of_pevent(&pcount_sum, L3D_CACHE_REFILL);
	mem_info->freq = mem_info->cycle / (nr_window * mips_window_size / NSEC_PER_MSEC);
}

void mips_tick_update(struct task_struct *p, int cpu)
{
	if (!mips_enable)
		return;

	load_pred_mips_freq(cpu, p, MIPS_UPDATE);
	mips_check_freq_update(cpu, MIPS_UPDATE);
}

void init_rq_mips(struct rq_mips_demand *rmipsd)
{
	rmipsd->prev_inst = 0;
	rmipsd->curr_inst = 0;
	rmipsd->curr_avg_ipc = 0;
	rmipsd->mips_window_start = 0;
	rmipsd->mips_last_update = 0;
}

int mips_fork(struct task_struct *p)
{
	struct task_mips_demand *tmipsd = NULL;

	tmipsd = kzalloc(sizeof(*tmipsd), GFP_KERNEL);
	if (!tmipsd)
		return -ENOMEM;

	(void)memset_s(tmipsd, sizeof(tmipsd), 0, sizeof(tmipsd));
	p->tmipsd = tmipsd;
	return 0;
}

void mips_free(struct task_struct *p)
{
	if (p->tmipsd) {
		kfree(p->tmipsd);
		p->tmipsd = NULL;
	}
}

int mips_start(void)
{
	int ret;

	ret = set_phase_state(false, NULL);
	if (ret < 0)
		return ret;
	sysctl_phase_exclude_hv = 0;
	sysctl_phase_exclude_idle = 0;
	sysctl_phase_exclude_kernel = 0;
	ret = set_phase_state(true, cpu_possible_mask);
	if (ret == 0) {
		mips_enable = true;
		set_mips_mem_state(true);
	}
	return ret;
}

int mips_stop(void)
{
	mips_enable = false;
	set_mips_mem_state(false);
	return set_phase_state(false, NULL);
}
