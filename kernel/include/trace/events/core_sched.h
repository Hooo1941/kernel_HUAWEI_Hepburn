/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * core_sched.h
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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
#define TRACE_SYSTEM core_sched

#if !defined(_TRACE_CORE_SCHED_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_CORE_SCHED_H

#include <linux/sched.h>
#include <linux/tracepoint.h>
#include <securec.h>

TRACE_EVENT(task_prio_compare,

	TP_PROTO(struct task_struct *a, int prio_a,
		 unsigned long vruntime_a, unsigned long deadline_a,
		 struct task_struct *b, int prio_b,
		 unsigned long vruntime_b, unsigned long deadline_b),

	TP_ARGS(a, prio_a, vruntime_a, deadline_a,
		b, prio_b, vruntime_b, deadline_b),

	TP_STRUCT__entry(
		__array(char,	comm_a,	TASK_COMM_LEN	)
		__array(char,	comm_b,	TASK_COMM_LEN	)
		__field(pid_t,	pid_a			)
		__field(pid_t,	pid_b			)
		__field(int,	prio_a			)
		__field(int,	prio_b			)
		__field(unsigned long, vruntime_a)
		__field(unsigned long, vruntime_b)
		__field(unsigned long, deadline_a)
		__field(unsigned long, deadline_b)
	),

	TP_fast_assign(
		memcpy_s(__entry->comm_a, TASK_COMM_LEN, a->comm, TASK_COMM_LEN);
		memcpy_s(__entry->comm_b, TASK_COMM_LEN, b->comm, TASK_COMM_LEN);
		__entry->pid_a		= a->pid;
		__entry->pid_b		= b->pid;
		__entry->prio_a		= prio_a;
		__entry->prio_b		= prio_b;
		__entry->vruntime_a	= vruntime_a;
		__entry->vruntime_b	= vruntime_b;
		__entry->deadline_a	= deadline_a;
		__entry->deadline_b	= deadline_b;
	),

	TP_printk("(%s/%d;%d,%lu,%lu) ?< (%s/%d;%d,%lu,%lu)",
		  __entry->comm_a, __entry->pid_a, __entry->prio_a,
		  __entry->vruntime_a, __entry->deadline_a,
		  __entry->comm_b, __entry->pid_b, __entry->prio_b,
		  __entry->vruntime_b, __entry->deadline_b)
);

TRACE_EVENT(sched_core_find,

	TP_PROTO(int cpu, unsigned long cookie, struct task_struct *picked),

	TP_ARGS(cpu, cookie, picked),

	TP_STRUCT__entry(
		__field(int,	cpu		)
		__field(unsigned long,	cookie)
		__array(char,	comm,	TASK_COMM_LEN)
		__field(pid_t,	pid		)
		__field(unsigned long, core_cookie)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->cookie		= cookie;
		memcpy_s(__entry->comm, TASK_COMM_LEN, picked->comm, TASK_COMM_LEN);
		__entry->pid		= picked->pid;
		__entry->core_cookie	= picked->core_cookie;
	),

	TP_printk("cpu%d find cookie %lx match: %s/%d %lx",
		  __entry->cpu, __entry->cookie, __entry->comm,
		  __entry->pid, __entry->core_cookie)
);

TRACE_EVENT(core_pick_pre_select,

	TP_PROTO(int cpu, unsigned int core_task_seq, unsigned int core_pick_seq,
		 unsigned int core_sched_seq, struct task_struct *picked),

	TP_ARGS(cpu, core_task_seq, core_pick_seq, core_sched_seq, picked),

	TP_STRUCT__entry(
		__field(int,	cpu		)
		__field(unsigned int, core_task_seq)
		__field(unsigned int, core_pick_seq)
		__field(unsigned int, core_sched_seq)
		__array(char,	comm,	TASK_COMM_LEN)
		__field(pid_t,	pid		)
		__field(unsigned long, core_cookie)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->core_task_seq	= core_task_seq;
		__entry->core_pick_seq	= core_pick_seq;
		__entry->core_sched_seq	= core_sched_seq;
		memcpy_s(__entry->comm, TASK_COMM_LEN, picked->comm, TASK_COMM_LEN);
		__entry->pid		= picked->pid;
		__entry->core_cookie	= picked->core_cookie;
	),

	TP_printk("cpu%d pick pre selected (%u %u %u): %s/%d %lx",
		  __entry->cpu, __entry->core_task_seq, __entry->core_pick_seq,
		  __entry->core_sched_seq, __entry->comm, __entry->pid, __entry->core_cookie)
);

TRACE_EVENT(reverse_force_idle,

	TP_PROTO(int cpu, struct task_struct *prev, struct task_struct *next),

	TP_ARGS(cpu, prev, next),

	TP_STRUCT__entry(
		__field(int,	cpu		)
		__array(char,	prev_comm,	TASK_COMM_LEN)
		__field(pid_t,	prev_pid	)
		__array(char,	next_comm,	TASK_COMM_LEN)
		__field(pid_t,	next_pid	)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		memcpy_s(__entry->prev_comm, TASK_COMM_LEN, prev->comm, TASK_COMM_LEN);
		__entry->prev_pid	= prev->pid;
		memcpy_s(__entry->next_comm, TASK_COMM_LEN, next->comm, TASK_COMM_LEN);
		__entry->next_pid	= next->pid;
	),

	TP_printk("cpu%d predict reverse forceidle %s/%d -> %s/%d",
		  __entry->cpu, __entry->prev_comm, __entry->prev_pid,
		  __entry->next_comm, __entry->next_pid)
);

TRACE_EVENT(core_pick_cpu_select,

	TP_PROTO(int cpu, struct task_struct *picked),

	TP_ARGS(cpu, picked),

	TP_STRUCT__entry(
		__field(int,	cpu		)
		__array(char,	comm,	TASK_COMM_LEN)
		__field(pid_t,	pid		)
		__field(unsigned long, core_cookie)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		memcpy_s(__entry->comm, TASK_COMM_LEN, picked->comm, TASK_COMM_LEN);
		__entry->pid		= picked->pid;
		__entry->core_cookie	= picked->core_cookie;
	),

	TP_printk("cpu%d selected: %s/%d %lx",
		  __entry->cpu, __entry->comm, __entry->pid, __entry->core_cookie)
);

TRACE_EVENT(core_pick_cpu_no_sync,

	TP_PROTO(int cpu, struct task_struct *picked),

	TP_ARGS(cpu, picked),

	TP_STRUCT__entry(
		__field(int,	cpu		)
		__array(char,	comm,	TASK_COMM_LEN)
		__field(pid_t,	pid		)
		__field(unsigned long, core_cookie)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		memcpy_s(__entry->comm, TASK_COMM_LEN, picked->comm, TASK_COMM_LEN);
		__entry->pid		= picked->pid;
		__entry->core_cookie	= picked->core_cookie;
	),

	TP_printk("cpu%d pick: %s/%d cookie=%lx",
		  __entry->cpu, __entry->comm, __entry->pid,
		  __entry->core_cookie)
);

TRACE_EVENT(core_pick_max_select,

	TP_PROTO(int cpu, struct task_struct *old_max, struct task_struct *new_max),

	TP_ARGS(cpu, old_max, new_max),

	TP_STRUCT__entry(
		__field(int,	cpu		)
		__array(char,	old_comm,	TASK_COMM_LEN)
		__field(pid_t,	old_pid	)
		__field(unsigned long, old_cookie)
		__array(char,	new_comm,	TASK_COMM_LEN)
		__field(pid_t,	new_pid	)
		__field(unsigned long, new_cookie)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		memcpy_s(__entry->old_comm, TASK_COMM_LEN, old_max ? old_max->comm : "NULL", TASK_COMM_LEN);
		__entry->old_pid	= old_max ? old_max->pid : -1;
		__entry->old_cookie	= old_max ? old_max->core_cookie : 0;
		memcpy_s(__entry->new_comm, TASK_COMM_LEN, new_max->comm, TASK_COMM_LEN);
		__entry->new_pid	= new_max->pid;
		__entry->new_cookie	= new_max->core_cookie;
	),

	TP_printk("cpu%d max %s/%d %lu -> %s/%d %lu",
		  __entry->cpu, __entry->old_comm, __entry->old_pid, __entry->old_cookie,
		  __entry->new_comm, __entry->new_pid, __entry->new_cookie)
);

TRACE_EVENT(core_pick_core_state,

	TP_PROTO(int cpu, unsigned long core_cookie, u8 smt_expeller,
		 unsigned long core_prefer,
		 u8 lazy_forceidle, u64 lazy_forceidle_start,
		 unsigned int forceidle_count),

	TP_ARGS(cpu, core_cookie, smt_expeller, core_prefer,
		lazy_forceidle, lazy_forceidle_start, forceidle_count),

	TP_STRUCT__entry(
		__field(int,	cpu		)
		__field(unsigned long, core_cookie)
		__field(u8, smt_expeller)
		__field(unsigned long, core_prefer)
		__field(u8, lazy_forceidle)
		__field(u64, lazy_forceidle_start)
		__field(unsigned int, forceidle_count)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->core_cookie	= core_cookie;
		__entry->smt_expeller	= smt_expeller;
		__entry->core_prefer	= core_prefer;
		__entry->lazy_forceidle	= lazy_forceidle;
		__entry->lazy_forceidle_start = lazy_forceidle_start;
		__entry->forceidle_count = forceidle_count;
	),

	TP_printk("cpu%d cookie=%lx expeller=%u prefer=%lx fi_count=%d lazy_fi=%u lazy_fi_start=%llu",
		  __entry->cpu, __entry->core_cookie, __entry->smt_expeller,
		  __entry->core_prefer, __entry->forceidle_count,
		  __entry->lazy_forceidle, __entry->lazy_forceidle_start)
);

TRACE_EVENT(steal_cookie,

	TP_PROTO(struct task_struct *p, int src, int dst,
		 int idle_core_occupation, unsigned long cookie),

	TP_ARGS(p, src, dst, idle_core_occupation, cookie),

	TP_STRUCT__entry(
		__array(char,	comm,	TASK_COMM_LEN)
		__field(pid_t,	pid		)
		__field(int, src_cpu)
		__field(int, dst_cpu)
		__field(int, task_core_occupation)
		__field(int, idle_core_occupation)
		__field(unsigned long, cookie)
	),

	TP_fast_assign(
		memcpy_s(__entry->comm, TASK_COMM_LEN, p->comm, TASK_COMM_LEN);
		__entry->pid	= p->pid;
		__entry->src_cpu = src;
		__entry->dst_cpu = dst;
		__entry->task_core_occupation = p->core_occupation;
		__entry->idle_core_occupation = idle_core_occupation;
		__entry->cookie	= cookie;
	),

	TP_printk("core fill: %s/%d (%d->%d) %d %d %lx",
		  __entry->comm, __entry->pid, __entry->src_cpu, __entry->dst_cpu,
		  __entry->task_core_occupation, __entry->idle_core_occupation, __entry->cookie)
);

TRACE_EVENT(task_update_cookie,

	TP_PROTO(struct task_struct *p, unsigned long old_cookie, unsigned long new_cookie),

	TP_ARGS(p, old_cookie, new_cookie),

	TP_STRUCT__entry(
		__array(char,	comm,	TASK_COMM_LEN)
		__field(pid_t,	pid		)
		__field(unsigned long, old_cookie)
		__field(unsigned long, new_cookie)
	),

	TP_fast_assign(
		memcpy_s(__entry->comm, TASK_COMM_LEN, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->old_cookie	= old_cookie;
		__entry->new_cookie	= new_cookie;
	),

	TP_printk("%s/%d update cookie: %lx -> %lx",
		  __entry->comm, __entry->pid, __entry->old_cookie, __entry->new_cookie)
);

#ifdef CONFIG_SCHED_SMT_EXPELLING
TRACE_EVENT(smt_expel,

	TP_PROTO(int cpu, struct task_struct *expeller, unsigned int grp_expeller,
		struct task_struct *expellee, unsigned int grp_expellee),

	TP_ARGS(cpu, expeller, grp_expeller, expellee, grp_expellee),

	TP_STRUCT__entry(
		__field(int,	cpu				)
		__field(pid_t,	expeller_pid			)
		__array(char,	expeller_comm,	TASK_COMM_LEN	)
		__field(unsigned int,	grp_expeller		)
		__field(unsigned int,	static_expeller		)
		__field(unsigned int,	dynamic_expeller	)
		__field(pid_t,	expellee_pid			)
		__array(char,	expellee_comm,	TASK_COMM_LEN	)
		__field(unsigned int,	grp_expellee		)
		__field(unsigned int,	static_expellee		)
		__field(unsigned int,	dynamic_expellee	)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->expeller_pid	= expeller->pid;
		memcpy_s(__entry->expeller_comm, TASK_COMM_LEN, expeller->comm, TASK_COMM_LEN);
		__entry->grp_expeller = grp_expeller;
		__entry->static_expeller = expeller->smt_expeller;
		__entry->dynamic_expeller = expeller->dynamic_smt_expeller;
		__entry->expellee_pid	= expellee->pid;
		memcpy_s(__entry->expellee_comm, TASK_COMM_LEN, expellee->comm, TASK_COMM_LEN);
		__entry->grp_expellee = grp_expellee;
		__entry->static_expellee = expellee->smt_expeller;
		__entry->dynamic_expellee = expellee->dynamic_smt_expeller;
	),

	TP_printk("cpu %d expeller %d/%s expeller_type %u/%u/%u expellee %d/%s expellee_type %u/%u/%u",
		  __entry->cpu, __entry->expeller_pid, __entry->expeller_comm,
		  __entry->grp_expeller, __entry->static_expeller, __entry->dynamic_expeller,
		  __entry->expellee_pid, __entry->expellee_comm,
		  __entry->grp_expellee, __entry->static_expellee, __entry->dynamic_expellee)
);
#endif

#ifdef CONFIG_SCHED_CORE_PREFER
TRACE_EVENT(steal_prefer,

	TP_PROTO(struct task_struct *p, int src, int dst,
		 unsigned long cookie),

	TP_ARGS(p, src, dst, cookie),

	TP_STRUCT__entry(
		__array(char,	comm,	TASK_COMM_LEN)
		__field(pid_t,	pid		)
		__field(int, src_cpu)
		__field(int, dst_cpu)
		__field(unsigned long, cookie)
	),

	TP_fast_assign(
		memcpy_s(__entry->comm, TASK_COMM_LEN, p->comm, TASK_COMM_LEN);
		__entry->pid	= p->pid;
		__entry->src_cpu = src;
		__entry->dst_cpu = dst;
		__entry->cookie	= cookie;
	),

	TP_printk("core prefer: %s/%d (%d->%d) %lx",
		  __entry->comm, __entry->pid, __entry->src_cpu, __entry->dst_cpu, __entry->cookie)
);

TRACE_EVENT(find_core_prefer_cpu,

	TP_PROTO(struct task_struct *p, const struct cpumask *active_cpumask, int cpu),

	TP_ARGS(p, active_cpumask, cpu),

	TP_STRUCT__entry(
		__array(char, comm, TASK_COMM_LEN)
		__field(pid_t, pid)
		__bitmask(cpus,	num_possible_cpus())
		__field(int, cpu)
	),

	TP_fast_assign(
		__entry->pid = p->pid;
		memcpy_s(__entry->comm, TASK_COMM_LEN, p->comm, TASK_COMM_LEN);
		__assign_bitmask(cpus, cpumask_bits(active_cpumask), num_possible_cpus());
		__entry->cpu = cpu;
	),

	TP_printk("comm=%s pid=%d active_cpus=%s target_cpu=%d",
		__entry->comm, __entry->pid, __get_bitmask(cpus), __entry->cpu)
);
#endif

#endif /* _TRACE_CORE_SCHED_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
