/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * gpu_scene_aware.h
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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
#define TRACE_SYSTEM gpu_scene_aware

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH platform_include/trace/events

#if !defined(_TRACE_GPU_SCENE_AWARE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_GPU_SCENE_AWARE_H

#include <linux/tracepoint.h>

TRACE_EVENT(devfreq_gpu_scene_aware,

	TP_PROTO(unsigned int scene_id,
        unsigned int util,
        unsigned long util_sum,
        unsigned long window_counter,
        int vsync,
        unsigned int target_load,
        unsigned long target_freq
        ),
	TP_ARGS(scene_id, util, util_sum, window_counter,
        vsync, target_load, target_freq),

	TP_STRUCT__entry(
		__field(unsigned int, scene		)
		__field(unsigned int, util		)
		__field(unsigned long, util_sum		)
		__field(unsigned long, window_counter	)
		__field(int,           vsync		)
		__field(unsigned int, target_load	)
		__field(unsigned long, target_freq	)
	),

	TP_fast_assign(
		__entry->scene          = scene_id;
		__entry->util	        = util;
		__entry->util_sum	= util_sum;
		__entry->window_counter = window_counter;
		__entry->vsync	        = vsync;
		__entry->target_load	= target_load;
		__entry->target_freq	= target_freq;
	),

	TP_printk("scene_id=%u cur_util=%u util_sum=%lu "
          "window_counter=%lu vsync=%d target_load=%u "
          "target_freq=%lu",
          __entry->scene, __entry->util,
          __entry->util_sum, __entry->window_counter,
          __entry->vsync, __entry->target_load,
          __entry->target_freq)
);

TRACE_EVENT(devfreq_gpu_scene_aware_apply_limits,

	TP_PROTO(int cl_boost,
        unsigned long cl_boost_freq,
        unsigned long user_set_freq,
        unsigned long target_freq
        ),
	TP_ARGS(cl_boost, cl_boost_freq,
        user_set_freq, target_freq),

	TP_STRUCT__entry(
		__field(int,        cl_boost            )
		__field(unsigned long,  cl_boost_freq   )
		__field(unsigned long,  user_set_freq   )
		__field(unsigned long,  target_freq     )
	),

	TP_fast_assign(
		__entry->cl_boost = cl_boost;
		__entry->cl_boost_freq = cl_boost_freq;
		__entry->user_set_freq = user_set_freq;
		__entry->target_freq = target_freq;
	),

	TP_printk("boost=%d boost_freq=%lu user_set_freq=%lu target_freq=%lu",
		  __entry->cl_boost, __entry->cl_boost_freq, __entry->user_set_freq,
          __entry->target_freq)
);
#endif /* _TRACE_GPU_SCENE_AWARE_H */

/* This part must be outside protection */
#include <trace/define_trace.h>