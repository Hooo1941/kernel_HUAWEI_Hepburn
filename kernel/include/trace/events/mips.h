/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * mips.h
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

#undef TRACE_SYSTEM
#define TRACE_SYSTEM mips

#if !defined(_TRACE_MIPS_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MIPS_H

#include <platform_include/cee/linux/phase.h>
#include <linux/perf_event.h>
#include <linux/tracepoint.h>
#include <securec.h>

TRACE_EVENT(mips_task_inst_hist,

	TP_PROTO(struct task_struct *p, int mode, u32 *inst),

	TP_ARGS(p, mode , inst),

	TP_STRUCT__entry(
		__field(	pid_t,	pid			)
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	int,	mode			)
		__array(	u32,	inst,	6		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid    = p->pid;
		__entry->mode   = mode;
		memcpy(__entry->inst, inst, 6 * sizeof(u32));
	),

	TP_printk("comm=%s pid=%d mode=%d inst[0]=%u, inst[1]=%u, "
		  "inst[2]=%u, inst[3]=%u, inst[4]=%u, inst[5]=%u,",
		  __entry->comm, __entry->pid, __entry->mode,
		  __entry->inst[0], __entry->inst[1], __entry->inst[2],
		  __entry->inst[3], __entry->inst[4], __entry->inst[5])
);

TRACE_EVENT(mips_task_cycle_hist,

	TP_PROTO(struct task_struct *p, int mode, u32 *cycle),

	TP_ARGS(p, mode , cycle),

	TP_STRUCT__entry(
		__field(	pid_t,	pid			)
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	int,	mode			)
		__array(	u32,	cycle,	6		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid    = p->pid;
		__entry->mode   = mode;
		memcpy(__entry->cycle, cycle, 6 * sizeof(u32));
	),

	TP_printk("comm=%s pid=%d mode=%d cycle[0]=%u, cycle[1]=%u, "
		  "cycle[2]=%u, cycle[3]=%u, cycle[4]=%u, cycle[5]=%u",
		  __entry->comm, __entry->pid, __entry->mode,
		  __entry->cycle[0], __entry->cycle[1], __entry->cycle[2],
		  __entry->cycle[3], __entry->cycle[4], __entry->cycle[5])
);

TRACE_EVENT(mips_cpu_hist,

	TP_PROTO(int cpu, int mode, int index, int *pevents, struct phase_event_pcount *delta),

	TP_ARGS(cpu, mode, index, pevents, delta),

	TP_STRUCT__entry(
		__field(	int,	cpu			)
		__field(	int,	mode			)
		__field(	int,	index			)
		__array(	int,	pevents,	7	)
		__array(	u64,	delta,		7	)
	),

	TP_fast_assign(
		__entry->cpu    = cpu;
		__entry->mode   = mode;
		__entry->index   = index;
		memcpy(__entry->pevents, pevents, 7 * sizeof(int));
		memcpy(__entry->delta, delta->data, 7 * sizeof(u64));
	),

	TP_printk("cpu=%d mode=%d index=%d event_id=0x%x delta=%llu "
		  "event_id=0x%x delta=%llu event_id=0x%x delta=%llu "
		  "event_id=0x%x delta=%llu event_id=0x%x delta=%llu "
		  "event_id=0x%x delta=%llu event_id=0x%x delta=%llu ",
		  __entry->cpu, __entry->mode, __entry->index,
		  __entry->pevents[0], __entry->delta[0], __entry->pevents[1],
		  __entry->delta[1], __entry->pevents[2], __entry->delta[2],
		  __entry->pevents[3], __entry->delta[3], __entry->pevents[4],
		  __entry->delta[4], __entry->pevents[5], __entry->delta[5],
		  __entry->pevents[6], __entry->delta[6])
);

TRACE_EVENT(mips_window_time,

	TP_PROTO(struct task_struct *p, struct rq *rq, u64 now, int flag),

	TP_ARGS(p, rq, now, flag),

	TP_STRUCT__entry(
		__field(	int,	cpu			)
		__field(	pid_t,	pid			)
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	u64,	now			)
		__field(	u64,	task_start		)
		__field(	u64,	rq_start		)
		__field(	int,	flag			)
	),

	TP_fast_assign(
		__entry->cpu	= rq->cpu;
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid = p->pid;
		__entry->now = now;
		__entry->task_start = p->tmipsd->window_start;
		__entry->rq_start = rq->rmipsd.mips_window_start;
		__entry->flag = flag;
	),

	TP_printk("cpu=%d comm=%s pid=%d now=%llu taskstart=%llu cpustart=%llu flag=%d",
		  __entry->cpu, __entry->comm, __entry->pid, __entry->now,
		  __entry->task_start, __entry->rq_start, __entry->flag)
);

#endif /* _TRACE_MIPS_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
