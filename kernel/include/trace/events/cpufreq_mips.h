/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * cpufreq_mips.h
 *
 * schedutil governor trace events
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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
#define TRACE_SYSTEM cpufreq_mips

#if !defined(_TRACE_CPUFREQ_MIPS_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_CPUFREQ_MIPS_H

TRACE_EVENT(mips_load_freq,

	TP_PROTO(struct task_struct *p, struct rq *rq, unsigned int cpufreq,
		 unsigned int taskfreq, unsigned int nextfreq, int flags),

	TP_ARGS(p, rq, cpufreq, taskfreq, nextfreq, flags),

	TP_STRUCT__entry(
		__field(	int,	cpu			)
		__field(	pid_t,	pid			)
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	unsigned int,	taskipc		)
		__field(	unsigned int,	cpuipc		)
		__field(	unsigned int,	taskfreq	)
		__field(	unsigned int,	cpufreq		)
		__field(	unsigned int,	nextfreq	)
		__field(	unsigned int,	flags		)
	),

	TP_fast_assign(
		__entry->cpu	= rq->cpu;
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid    = p->pid;
		__entry->taskipc = p->tmipsd->curr_avg_ipc;
		__entry->cpuipc = rq->rmipsd.curr_avg_ipc;
		__entry->taskfreq = taskfreq;
		__entry->cpufreq = cpufreq;
		__entry->nextfreq = nextfreq;
		__entry->flags = flags;
	),

	TP_printk("cpu=%d flags=%d, comm=%s pid=%d taskipc=%u cpuipc=%u taskfreq=%u cpufreq=%u nextfreq=%u",
		  __entry->cpu, __entry->flags, __entry->comm, __entry->pid, __entry->taskipc, __entry->cpuipc,
		  __entry->taskfreq, __entry->cpufreq, __entry->nextfreq)
);
TRACE_EVENT(mips_cpu_freq,

	TP_PROTO(struct rq *rq, u64 cycle, unsigned int cpufreq,
		 unsigned int nextfreq, int tl, u64 now, int flags),

	TP_ARGS(rq, cycle, cpufreq, nextfreq, tl, now, flags),

	TP_STRUCT__entry(
		__field(	int,	cpu			)
		__field(	u64,	cycle			)
		__field(	unsigned int,	cpufreq		)
		__field(	unsigned int,	nextfreq	)
		__field(	int,	tl			)
		__field(	u64,	now			)
		__field(	u64,	ws			)
		__field(	unsigned int,	flags		)
	),

	TP_fast_assign(
		__entry->cpu	= rq->cpu;
		__entry->cycle = cycle;
		__entry->cpufreq = cpufreq;
		__entry->nextfreq = nextfreq;
		__entry->tl = tl;
		__entry->now = now;
		__entry->ws = rq->mips_mem_window_start;
		__entry->flags = flags;
	),

	TP_printk("cpu=%d flags=%d, cycle=%llu cpufreq=%u nextfreq=%u tl=%d now=%llu, ws=%llu",
		  __entry->cpu, __entry->flags, __entry->cycle, __entry->cpufreq, __entry->nextfreq,
		  __entry->tl, __entry->now, __entry->ws)
);
TRACE_EVENT(cpufreq_mips_freq,

	TP_PROTO(int cpu, unsigned int mipsfreq, unsigned int prev_freq, unsigned int flags),

	TP_ARGS(cpu, mipsfreq, prev_freq, flags),

	TP_STRUCT__entry(
		__field(	int,	cpu			)
		__field(	unsigned int,	flags		)
		__field(	unsigned int,	mipsfreq	)
		__field(	unsigned int,	prev_freq	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
		__entry->flags = flags;
		__entry->mipsfreq = mipsfreq;
		__entry->prev_freq = prev_freq;
	),

	TP_printk("cpu=%d mipsfreq=%u prev_freq=%u flags=%u",
		  __entry->cpu, __entry->mipsfreq, __entry->prev_freq, __entry->flags)
);
#endif /* _TRACE_CPUFREQ_MIPS_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
