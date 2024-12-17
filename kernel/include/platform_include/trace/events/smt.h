/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * smt.h
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
#define TRACE_SYSTEM smt

#if !defined(_TRACE_SMT_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_SMT_H

#include <linux/sched/smt.h>
#include <linux/tracepoint.h>

TRACE_EVENT(smt_update_mode,

	TP_PROTO(struct cpumask *mask, unsigned int target_mode,
		 unsigned int cpu_max_cap),

	TP_ARGS(mask, target_mode, cpu_max_cap),

	TP_STRUCT__entry(
		__bitmask(cpus,	num_possible_cpus()	)
		__field(unsigned int,	target_mode	)
		__field(unsigned int,	cpu_max_cap	)
	),

	TP_fast_assign(
		__assign_bitmask(cpus, cpumask_bits(mask), num_possible_cpus());
		__entry->target_mode		= target_mode;
		__entry->cpu_max_cap		= cpu_max_cap;
	),

	TP_printk("cpus=%s mode=%s cpu_cap=%d",
		  __get_bitmask(cpus),
		  __entry->target_mode == ST_MODE ? "ST" : "MT", __entry->cpu_max_cap)
);

#endif /* _TRACE_SMT_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
