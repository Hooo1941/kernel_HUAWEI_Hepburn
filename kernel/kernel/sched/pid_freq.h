/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * pid_freq.h
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
#ifndef _PID_FREQ_H
#define _PID_FREQ_H
#include <platform_include/cee/linux/phase.h>
#define MAX_CPU_USAGE 1024
#define MAX_SPEED 1024
#define PID_PARAM_NUM 3
#define DETACH_THRESHOLD_RATIO 5
#define PID_PARAM_MAX_RATIO 100
#define PID_PARAM_P_RATIO 80
#define PID_PARAM_D_RATIO 150
#define PID_INFO_ZOOM_TIMES 1000
#define PID_INFO_LOWER_NUM 10000
#define NR_TICK_LOSS_WIN 3

extern unsigned int pid_control_update;
extern unsigned int freq_pid[PID_PARAM_NUM];
extern unsigned int speed_pid[PID_PARAM_NUM];
extern unsigned int pid_control_target_load;

struct pid_control_cpu_info {
	u64 window_avg_freq;
	int current_freq;
	int last_usage_speed_err;
	int usage_speed_err_sum;
	int history_usage[NR_PHASE_WINDOW];
	int last_freq_err;
	int freq_err_sum;
	u64 last_window_time;
	u64 last_freq_time;
	int target_load_out;
	int pid_freq_out;
};

int update_pid_freq(int cpu, int flags, u64 cpu_cycles);
void update_avg_freq(int cpu);
unsigned int get_rtg_util(int cpu, struct task_struct *p);

#endif
