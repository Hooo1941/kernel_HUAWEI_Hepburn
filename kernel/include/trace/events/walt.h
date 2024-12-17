/*
 * walt.h
 *
 * walt trace
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

#undef TRACE_SYSTEM
#define TRACE_SYSTEM walt

#if !defined(_TRACE_WALT_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_WALT_H

#include <linux/tracepoint.h>
#include <securec.h>

#ifdef CONFIG_SCHED_WALT_WINDOW_SIZE_TUNABLE
extern unsigned int walt_ravg_window;
#else
extern const unsigned int walt_ravg_window;
#endif

#define walt_util(util_var, demand_sum) { \
	u64 sum = demand_sum << SCHED_CAPACITY_SHIFT; \
	do_div(sum, walt_ravg_window); \
	util_var = (typeof(util_var))sum; \
}

struct rq;

TRACE_EVENT(walt_update_task_ravg,

	TP_PROTO(struct task_struct *p, struct rq *rq, int evt,
		 u64 wallclock, u64 irqtime),

	TP_ARGS(p, rq, evt, wallclock, irqtime),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	pid_t,	cur_pid			)
		__field(	u64,	wallclock		)
		__field(	u64,	mark_start		)
		__field(	u64,	delta_m			)
		__field(	u64,	win_start		)
		__field(	u64,	delta			)
		__field(	u64,	irqtime			)
		__array(	char,	evt, 16			)
		__field(unsigned int,	demand			)
		__field(unsigned int,	sum			)
		__field(	 int,	cpu			)
		__field(	u64,	cs			)
		__field(	u64,	ps			)
		__field(	u32,	curr_window		)
		__field(	u32,	prev_window		)
		__field(	u64,	nt_cs			)
		__field(	u64,	nt_ps			)
		__field(	u32,	active_windows		)
	),

	TP_fast_assign(
		static const char* walt_event_names[] = {
			"PUT_PREV_TASK",
			"PICK_NEXT_TASK",
			"TASK_WAKE",
			"TASK_MIGRATE",
			"TASK_UPDATE",
			"IRQ_UPDATE"
		};
		__entry->wallclock      = wallclock;
		__entry->win_start      = rq->window_start;
		__entry->delta          = (wallclock - rq->window_start);
		strcpy(__entry->evt, walt_event_names[evt]);
		__entry->cpu            = rq->cpu;
		__entry->cur_pid        = rq->curr->pid;
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->mark_start     = p->ravg.mark_start;
		__entry->delta_m        = (wallclock - p->ravg.mark_start);
		__entry->demand         = p->ravg.demand;
		__entry->sum            = p->ravg.sum;
		__entry->irqtime        = irqtime;
		__entry->cs             = rq->curr_runnable_sum;
		__entry->ps             = rq->prev_runnable_sum;
		__entry->curr_window	= p->ravg.curr_window;
		__entry->prev_window	= p->ravg.prev_window;
		__entry->nt_cs		= rq->nt_curr_runnable_sum;
		__entry->nt_ps		= rq->nt_prev_runnable_sum;
		__entry->active_windows	= p->ravg.active_windows;
	),

	TP_printk("wallclock=%llu window_start=%llu delta=%llu event=%s cpu=%d cur_pid=%d pid=%d comm=%s"
		" mark_start=%llu delta=%llu demand=%u sum=%u irqtime=%llu"
		" curr_runnable_sum=%llu prev_runnable_sum=%llu cur_window=%u"
		" prev_window=%u nt_curr_runnable_sum=%llu nt_prev_runnable_sum=%llu active_windows=%u",
		__entry->wallclock, __entry->win_start, __entry->delta,
		__entry->evt, __entry->cpu, __entry->cur_pid,
		__entry->pid, __entry->comm, __entry->mark_start,
		__entry->delta_m, __entry->demand,
		__entry->sum, __entry->irqtime,
		__entry->cs, __entry->ps,
		__entry->curr_window, __entry->prev_window,
		__entry->nt_cs, __entry->nt_ps,
		__entry->active_windows
		)
);

TRACE_EVENT(walt_update_history,

	TP_PROTO(struct rq *rq, struct task_struct *p, u32 runtime, int samples,
			int evt),

	TP_ARGS(rq, p, runtime, samples, evt),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(unsigned int,	runtime			)
		__field(	 int,	samples			)
		__field(	 int,	evt			)
		__field(	 u64,	demand			)
		__field(unsigned int,	walt_avg		)
		__field(unsigned int,	pelt_avg		)
		__array(	 u32,	hist, RAVG_HIST_SIZE_MAX)
		__field(	 int,	cpu			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->runtime        = runtime;
		__entry->samples        = samples;
		__entry->evt            = evt;
		__entry->demand         = p->ravg.demand;
		walt_util(__entry->walt_avg,__entry->demand);
		__entry->pelt_avg	= p->se.avg.util_avg;
		memcpy(__entry->hist, p->ravg.sum_history,
					RAVG_HIST_SIZE_MAX * sizeof(u32));
		__entry->cpu            = rq->cpu;
	),

	TP_printk("pid=%d comm=%s runtime=%u samples=%d event=%d demand=%llu ravg_window=%u"
		" walt=%u pelt=%u hist0=%u hist1=%u hist2=%u hist3=%u hist4=%u cpu=%d",
		__entry->pid, __entry->comm,
		__entry->runtime, __entry->samples, __entry->evt,
		__entry->demand,
		walt_ravg_window,
		__entry->walt_avg,
		__entry->pelt_avg,
		__entry->hist[0], __entry->hist[1],
		__entry->hist[2], __entry->hist[3],
		__entry->hist[4], __entry->cpu)
);

TRACE_EVENT(walt_migration_update_sum,

	TP_PROTO(struct rq *rq, struct task_struct *p),

	TP_ARGS(rq, p),

	TP_STRUCT__entry(
		__field(int,		cpu			)
		__field(int,		pid			)
		__field(	u64,	cs			)
		__field(	u64,	ps			)
		__field(	s64,	nt_cs			)
		__field(	s64,	nt_ps			)
	),

	TP_fast_assign(
		__entry->cpu		= cpu_of(rq);
		__entry->cs		= rq->curr_runnable_sum;
		__entry->ps		= rq->prev_runnable_sum;
		__entry->nt_cs		= (s64)rq->nt_curr_runnable_sum;
		__entry->nt_ps		= (s64)rq->nt_prev_runnable_sum;
		__entry->pid		= p->pid;
	),

	TP_printk("cpu=%d curr_runnable_sum=%llu prev_runnable_sum=%llu nt_curr_runnable_sum=%lld nt_prev_runnable_sum=%lld pid=%d",
		  __entry->cpu, __entry->cs, __entry->ps,
		  __entry->nt_cs, __entry->nt_ps, __entry->pid)
);

TRACE_EVENT(walt_window_rollover,

	TP_PROTO(int cpu),

	TP_ARGS(cpu),

	TP_STRUCT__entry(
		__field(	int,	cpu	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
	),

	TP_printk("cpu=%d", __entry->cpu)
);

#ifdef CONFIG_SCHED_TOP_TASK
TRACE_EVENT(walt_update_top_task,

	TP_PROTO(struct rq *rq, struct task_struct *p),

	TP_ARGS(rq, p),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	 int,	curr_load		)
		__field(	 int,	prev_load		)
		__field(	 int,	curr_top		)
		__field(	 int,	prev_top		)
		__field(	 int,	cpu			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->curr_load      = p->ravg.curr_load;
		__entry->prev_load      = p->ravg.prev_load;
		__entry->curr_top       = rq->top_task_index[rq->curr_table];
		__entry->prev_top       = rq->top_task_index[1 - rq->curr_table];
		__entry->cpu            = rq->cpu;
	),

	TP_printk("cpu=%d pid=%d comm=%s curr_load=%d prev_load=%d curr_top=%d prev_top=%d",
		__entry->cpu, __entry->pid, __entry->comm,
		__entry->curr_load,
		__entry->prev_load,
		__entry->curr_top,
		__entry->prev_top)
);
#endif /* CONFIG_SCHED_TOP_TASK */

#ifdef CONFIG_SCHED_PRED_LOAD
TRACE_EVENT(predl_adjust_runtime,

	TP_PROTO(struct task_struct *p, u32 orig_runtime, u32 boost_runtime),

	TP_ARGS(p, orig_runtime, boost_runtime),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	u32,	orig_runtime		)
		__field(	u32,	boost_runtime		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->orig_runtime	= orig_runtime;
		__entry->boost_runtime	= boost_runtime;
	),

	TP_printk("task %s(%d), bonus for full window, orig_runtime=%u new_runtime=%u",
		__entry->comm, __entry->pid, __entry->orig_runtime,
		__entry->boost_runtime)
);

TRACE_EVENT(predl_boost_runtime,

	TP_PROTO(struct task_struct *p, u32 orig_runtime, u32 boost_runtime),

	TP_ARGS(p, orig_runtime, boost_runtime),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	u32,	orig_runtime		)
		__field(	u32,	boost_runtime		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->orig_runtime	= orig_runtime;
		__entry->boost_runtime	= boost_runtime;
	),

	TP_printk("task %s(%d), boost for predicting, orig_runtime=%u boost_runtime=%u",
		__entry->comm, __entry->pid, __entry->orig_runtime,
		__entry->boost_runtime)
);

TRACE_EVENT(predl_window_rollover,

	TP_PROTO(int cpu),

	TP_ARGS(cpu),

	TP_STRUCT__entry(
		__field(	int,	cpu	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
	),

	TP_printk("cpu=%d", __entry->cpu)
);

TRACE_EVENT(predl_update_history,

	TP_PROTO(struct task_struct *p),

	TP_ARGS(p),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__array(	u32,	hist, RAVG_HIST_SIZE_MAX)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		memcpy(__entry->hist, p->ravg.predl_sum_history,
					RAVG_HIST_SIZE_MAX * sizeof(u32));
	),

	TP_printk("task %s(%d) hist (%u %u %u %u %u)",
		__entry->comm, __entry->pid,
		__entry->hist[0], __entry->hist[1],
		__entry->hist[2], __entry->hist[3],
		__entry->hist[4])
);

TRACE_EVENT(predl_get_busy,

	TP_PROTO(struct task_struct *p, u32 runtime, int start,
		u32 first, u32 predl),

	TP_ARGS(p, runtime, start, first, predl),

	TP_STRUCT__entry(
		__array(	char,	comm, TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	u32,	runtime			)
		__field(	int,	start			)
		__array(	u8,	bucket, 10		)
		__field(	u32,	first			)
		__field(	u32,	predl			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->runtime	= runtime;
		__entry->start		= start;
		memcpy(__entry->bucket, p->ravg.predl_busy_buckets, 10);
		__entry->first		= first;
		__entry->predl		= predl;
	),

	TP_printk("task %s(%d) runtime %u start %d buckets (%d %d %d %d %d "
		"%d %d %d %d %d) first %u predl %u",
		__entry->comm, __entry->pid, __entry->runtime, __entry->start,
		__entry->bucket[0], __entry->bucket[1], __entry->bucket[2],
		__entry->bucket[3], __entry->bucket[4], __entry->bucket[5],
		__entry->bucket[6], __entry->bucket[7], __entry->bucket[8],
		__entry->bucket[9], __entry->first, __entry->predl)
);

TRACE_EVENT(bucket_increase,

	TP_PROTO(struct task_struct *p, int idx),

	TP_ARGS(p, idx),

	TP_STRUCT__entry(
		__array(	char,	comm, TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	idx			)
		__array(	u8,	bucket, 10		)
		__bitmask(	bitmap,	NUM_BUSY_BUCKETS	)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->idx		= idx;
		memcpy(__entry->bucket, p->ravg.predl_busy_buckets, 10);
		__assign_bitmask(bitmap, p->ravg.predl_busy_buckets_bitmap,
					NUM_BUSY_BUCKETS);
	),

	TP_printk("task %s(%d) increase idx %d bitmap %s buckets (%u %u %u %u %u "
		"%u %u %u %u %u)",
		__entry->comm, __entry->pid, __entry->idx, __get_bitmask(bitmap),
		__entry->bucket[0], __entry->bucket[1], __entry->bucket[2],
		__entry->bucket[3], __entry->bucket[4], __entry->bucket[5],
		__entry->bucket[6], __entry->bucket[7], __entry->bucket[8],
		__entry->bucket[9])
);
#endif /* CONFIG_SCHED_PRED_LOAD */

#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
TRACE_EVENT(walt_ravg_window_change,

	TP_PROTO(unsigned int walt_ravg_window, unsigned int new_walt_ravg_window),

	TP_ARGS(walt_ravg_window, new_walt_ravg_window),

	TP_STRUCT__entry(
		__field(unsigned int, walt_ravg_window)
		__field(unsigned int, new_walt_ravg_window)
		__field(u64, change_time)
	),

	TP_fast_assign(
		__entry->walt_ravg_window	= walt_ravg_window;
		__entry->new_walt_ravg_window	= new_walt_ravg_window;
		__entry->change_time		= sched_ktime_clock();
	),

	TP_printk("from=%u to=%u at=%lu",
		__entry->walt_ravg_window, __entry->new_walt_ravg_window,
		__entry->change_time)
);
#endif

#ifdef CONFIG_SCHED_COLOC_GROUP
TRACE_EVENT(update_coloc_grp_load,

	TP_PROTO(s8 id, bool new_window, bool busy, u32 curr_load, u32 prev_load),

	TP_ARGS(id, new_window, busy, curr_load, prev_load),

	TP_STRUCT__entry(
		__field(	s8,	id			)
		__field(	bool,	new_window		)
		__field(	bool,	busy			)
		__field(	u32,	curr_load		)
		__field(	u32,	prev_load		)
	),

	TP_fast_assign(
		__entry->id		= id;
		__entry->new_window	= new_window;
		__entry->busy		= busy;
		__entry->curr_load	= curr_load;
		__entry->prev_load	= prev_load;
	),

	TP_printk("id=%d new_window=%d busy=%d curr_load=%u prev_load=%u",
		__entry->id, __entry->new_window, __entry->busy,
		__entry->curr_load, __entry->prev_load)
);

TRACE_EVENT(update_coloc_grp_history,

	TP_PROTO(s8 id, u32 load, u32 util, u64 prefer_cluster ),

	TP_ARGS(id, load, util, prefer_cluster),

	TP_STRUCT__entry(
		__field(	s8,	id			)
		__field(	u32,	load			)
		__field(	u32,	util			)
		__field(	u64,	prefer_cluster		)
	),

	TP_fast_assign(
		__entry->id		= id;
		__entry->load		= load;
		__entry->util		= util;
		__entry->prefer_cluster	= prefer_cluster;
	),

	TP_printk("id=%d load=%u final_util=%u prefer_cluster=%#lx",
		__entry->id, __entry->load, __entry->util,
		__entry->prefer_cluster)
);
#endif

#endif /* _TRACE_WALT_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
