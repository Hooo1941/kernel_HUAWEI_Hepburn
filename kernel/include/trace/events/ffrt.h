// SPDX-License-Identifier: GPL-2.0
/*
 * trace_ffrt.h
 *
 * ffrt trace header
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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
#define TRACE_SYSTEM ffrt

#if !defined(_TRACE_FFRT_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_FFRT_H

#include <linux/tracepoint.h>

TRACE_EVENT(ffrt_rtg_update_tgid_util,
	TP_PROTO(u64 tgid, u64 util),
	TP_ARGS(tgid, util),
	TP_STRUCT__entry(
		__field(u64, tgid)
		__field(u64, util)
	),
	TP_fast_assign(
		__entry->tgid        = tgid;
		__entry->util        = util;
	),
	TP_printk("Set RTG tgid[%lu] util[%lu]",
		__entry->tgid, __entry->util)
);

TRACE_EVENT(ffrt_rtg_update_group_load,
	TP_PROTO(u64 tgid, unsigned long long load, unsigned long long runtime),
	TP_ARGS(tgid, load, runtime),
	TP_STRUCT__entry(
		__field(u64, tgid)
		__field(unsigned long long, load)
		__field(unsigned long long, runtime)
	),
	TP_fast_assign(
		__entry->tgid        = tgid;
		__entry->load        = load;
		__entry->runtime        = runtime;
	),
	TP_printk("Get Load tgid[%lu] load[%llu] runtime[%llu]",
		__entry->tgid, __entry->load, __entry->runtime)
);

TRACE_EVENT(ffrt_qos_attr,
	TP_PROTO(int qos_level,
		u64 tid,
		int latency_nice,
		int uclamp_min,
		int uclamp_max,
		unsigned long affinity,
		unsigned char priority,
		unsigned char init_load,
		unsigned char prefer_idle),
	TP_ARGS(qos_level, tid, latency_nice, uclamp_min, uclamp_max, affinity, priority, init_load, prefer_idle),
	TP_STRUCT__entry(
		__field(int, qos_level)
		__field(u64, tid)
		__field(int, latency_nice)
		__field(int, uclamp_min)
		__field(int, uclamp_max)
		__field(unsigned long, affinity)
		__field(unsigned char, priority)
		__field(unsigned char, init_load)
		__field(unsigned char, prefer_idle)
	),
	TP_fast_assign(
		__entry->qos_level      = qos_level;
		__entry->tid            = tid;
		__entry->latency_nice   = latency_nice;
		__entry->uclamp_min     = uclamp_min;
		__entry->uclamp_max     = uclamp_max;
		__entry->affinity       = affinity;
		__entry->priority       = priority;
		__entry->init_load      = init_load;
		__entry->prefer_idle    = prefer_idle;
	),
	TP_printk("qos apply attr qos[%d] tid[%lu] lat_nice[%d] u_min[%d], u_max[%d],\
		affinity[0x%lx], prio[%u], init_load[%u], prefer_idle[%u]",
		__entry->qos_level, __entry->tid, __entry->latency_nice, __entry->uclamp_min, __entry->uclamp_max,
		__entry->affinity, __entry->priority, __entry->init_load, __entry->prefer_idle)
);

#endif /* _TRACE_FFRT_H */

/* This part must be outside protection */
#include <trace/define_trace.h>