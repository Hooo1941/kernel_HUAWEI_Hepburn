/*
 * vfb.h
 *
 * vfb trace event
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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
#define TRACE_SYSTEM vfb

#if !defined(_TRACE_VFB_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_VFB_H

#include <linux/tracepoint.h>

TRACE_EVENT(vfb_present_kick,

	TP_PROTO(bool is_start, int pid, int frame_no, u64 time),

	TP_ARGS(is_start, pid, frame_no, time),

	TP_STRUCT__entry(
		__field(bool, is_start)
		__field(int, pid)
		__field(int, frame_no)
		__field(u64, time)
	),

	TP_fast_assign(
		__entry->is_start = is_start;
		__entry->pid = pid;
		__entry->frame_no = frame_no;
		__entry->time = time;
	),

	TP_printk("%s|%d|vfb_present_kick frame_no=%d time=%lld",
		  __entry->is_start ? "B" : "E",
		  __entry->pid,
		  __entry->frame_no,
		  __entry->time)
);

#endif /* _TRACE_VFB_H */

/* This part must be outside protection */
#include <trace/define_trace.h>

