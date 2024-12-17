/*
 * pid_freq.h
 *
 * pid freq sched trace events
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
#define TRACE_SYSTEM pid_freq

#if !defined(_TRACE_PID_FREQ_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_PID_FREQ_H

TRACE_EVENT(pid_freq_event,
	TP_PROTO(int cpu, u64 cpu_cycles, int cpu_usage, struct pid_control_cpu_info *pid_control_cpu,
		 int lastfreq),
	TP_ARGS(cpu, cpu_cycles, cpu_usage, pid_control_cpu, lastfreq),
	TP_STRUCT__entry(
	__field(int,	cpu			)
	__field(u64,	cpu_cycles		)
	__field(int,	cpu_usage		)
	__field(int,	target_load_out		)
	__field(int,	lastfreq		)
	__field(int,	window_avg_freq		)
),

TP_fast_assign(
	__entry->cpu = cpu;
	__entry->cpu_cycles = cpu_cycles;
	__entry->cpu_usage = cpu_usage;
	__entry->target_load_out = pid_control_cpu->target_load_out;
	__entry->lastfreq = lastfreq;
	__entry->window_avg_freq = pid_control_cpu->window_avg_freq;
),

TP_printk("cpu=%d, raw-cpu-cycles=%llu, cpu_usage=%d, nowTarget=%d, lastfreq=%d,"
	  " avg_freq=%d",
	__entry->cpu, __entry->cpu_cycles, __entry->cpu_usage, __entry->target_load_out,
	__entry->lastfreq, __entry->window_avg_freq)
);


TRACE_EVENT(calculate_pid_freq,
	TP_PROTO(int cpu, int curr_util, int target_util, struct pid_control_cpu_info *pid_control_cpu,
		 int p_freq, int i_freq, int d_freq, int normalized_util_error, int freq_out, int cpu_capacity_orig),
	TP_ARGS(cpu, curr_util, target_util, pid_control_cpu, p_freq, i_freq, d_freq, normalized_util_error, freq_out, cpu_capacity_orig),
	TP_STRUCT__entry(
	__field(int,	cpu				)
	__field(int,	curr_util			)
	__field(int,	target_util			)
	__field(int,	freq_err_sum			)
	__field(int,	p_freq				)
	__field(int,	i_freq				)
	__field(int,	d_freq				)
	__field(int,	window_avg_freq			)
	__field(int,	target_load_out			)
	__field(int,	normalized_util_error		)
	__field(int,	freq_out		)
	__field(int,	cpu_capacity_orig		)
),

TP_fast_assign(
	__entry->cpu = cpu;
	__entry->curr_util = curr_util;
	__entry->target_util = target_util;
	__entry->freq_err_sum = pid_control_cpu->freq_err_sum;
	__entry->p_freq = p_freq;
	__entry->i_freq = i_freq;
	__entry->d_freq = d_freq;
	__entry->window_avg_freq = pid_control_cpu->window_avg_freq;
	__entry->target_load_out = pid_control_cpu->target_load_out;
	__entry->normalized_util_error = normalized_util_error;
	__entry->freq_out = freq_out;
	__entry->cpu_capacity_orig = cpu_capacity_orig;
),

TP_printk("cpu=%d curr_util=%d target_util=%d pid_control_cpu->freq_err_sum=%d "
	"p_freq=%d i_freq=%d d_freq=%d avg_freq=%d speed_final_target=%d "
	"normalized_util_error=%d freq_out=%d cpu_capacity_orig=%d",
	__entry->cpu, __entry->curr_util, __entry->target_util, __entry->freq_err_sum, __entry->p_freq, __entry->i_freq,
	__entry->d_freq, __entry->window_avg_freq, __entry->target_load_out, __entry->normalized_util_error, __entry->freq_out,
	__entry->cpu_capacity_orig)
);

TRACE_EVENT(usage_speed_pid,
	TP_PROTO(int cpu, int speed, struct pid_control_cpu_info *pid_control_cpu,
		 int p_speed, int i_speed, int d_speed, int speed_target),
	TP_ARGS(cpu, speed, pid_control_cpu, p_speed, i_speed, d_speed, speed_target),
	TP_STRUCT__entry(
	__field(int,	cpu				)
	__field(int,	speed				)
	__field(int,	usage_speed_err_sum		)
	__field(int,	p_speed				)
	__field(int,	i_speed				)
	__field(int,	d_speed				)
	__field(int,	speed_target			)
),

TP_fast_assign(
	__entry->cpu = cpu;
	__entry->speed = speed;
	__entry->usage_speed_err_sum = pid_control_cpu->usage_speed_err_sum;
	__entry->p_speed = p_speed;
	__entry->i_speed = i_speed;
	__entry->d_speed = d_speed;
	__entry->speed_target = speed_target;
),

TP_printk("cpu is %d speed=%d usage_speed_err_sum=%d "
	"p_speed=%d i_speed=%d d_speed=%d speed_target=%d",
	__entry->cpu, __entry->speed, __entry->usage_speed_err_sum,
	__entry->p_speed, __entry->i_speed, __entry->d_speed, __entry->speed_target)
);

TRACE_EVENT(update_avg_freq,
	TP_PROTO(int cpu, int cur_avg, int cur_freq, u64 now, u64 last_time),
	TP_ARGS(cpu, cur_avg, cur_freq, now, last_time),
	TP_STRUCT__entry(
	__field(int,	cpu)
	__field(int,	cur_avg)
	__field(int,	cur_freq)
	__field(u64,	now)
	__field(u64,	last_time)
),

TP_fast_assign(
	__entry->cpu = cpu;
	__entry->cur_avg = cur_avg;
	__entry->cur_freq = cur_freq;
	__entry->now = now;
	__entry->last_time = last_time;
),

TP_printk("cpu is %d cur_avg=%d cur_freq=%d now=%llu last_time=%llu",
	__entry->cpu, __entry->cur_avg,
	__entry->cur_freq, __entry->now, __entry->last_time)
);

#endif /* _TRACE_PID_FREQ_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
