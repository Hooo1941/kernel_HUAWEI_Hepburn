// SPDX-License-Identifier: GPL-2.0-only
/*
 * cpu_hwp_ctrl_trace.h
 *
 * cpu hwp ctrl trace
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
#define TRACE_SYSTEM cpu_hwp_ctrl_trace

#if !defined(_TRACE_CPU_HWP_CTRL_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_CPU_HWP_CTRL_H

#include <linux/tracepoint.h>

TRACE_EVENT(set_cpu_hwp_reg,
	TP_PROTO(int game_id, int cluster_id, int active),
	TP_ARGS(game_id, cluster_id, active),
	TP_STRUCT__entry(
		__field(int, game_id)
		__field(int, cluster_id)
		__field(int, active)
	),
	TP_fast_assign(
		__entry->game_id = game_id;
		__entry->cluster_id = cluster_id;
		__entry->active = active;
	),

	TP_printk("game_id=%d cluster_id=%d active=%d",
		  __entry->game_id, __entry->cluster_id, __entry->active)
);

TRACE_EVENT(perf_ctrl_set_task_cpu_hwp,
	TP_PROTO(int raw_game_id, int game_id),
	TP_ARGS(raw_game_id, game_id),
	TP_STRUCT__entry(
		__field(int, raw_game_id)
		__field(int, game_id)
	),
	TP_fast_assign(
		__entry->raw_game_id = raw_game_id;
		__entry->game_id = game_id;
	),

	TP_printk("raw_game_id=%d game_id=%d",
		  __entry->raw_game_id, __entry->game_id)
);

#endif /* _TRACE_CPU_HWP_CTRL_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
