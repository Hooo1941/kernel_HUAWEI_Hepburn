/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * pid_freq.c
 *
 * for pid control update cpu freq.
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
#include <pid_freq.h>
#include <platform_include/cee/linux/mips.h>
#include <linux/cpufreq.h>
#include <linux/sched/frame_info.h>
#include "sched.h"
#include "phase/phase_perf.h"
#include "phase/phase_feature.h"
#include <asm/phase_perf.h>
#include <trace/events/power.h>
#define CREATE_TRACE_POINTS
#include <trace/events/pid_freq.h>

__read_mostly unsigned int pid_window_size = 4; // ms
__read_mostly u64 one_tick_time = 4000000; // ns
__read_mostly u64 one_tick_us_time = 4000; // us
__read_mostly u64 time_ms_to_us_num = 1000;
__read_mostly unsigned int pid_control_update = 0;
__read_mostly unsigned int pid_control_target_load = 0;
__read_mostly unsigned int freq_pid[3] = {0, 0, 0};
__read_mostly unsigned int speed_pid[3] = {0, 0, 0};

static DEFINE_PER_CPU(struct pid_control_cpu_info, cpu_info);

void init_rq_pid_freq(struct pid_control_cpu_info *pid_control_cpu, struct sched_cluster *cluster)
{
	int i;
	u64 now = ktime_get_ns();
	pid_control_cpu->window_avg_freq = cluster->min_freq;
	pid_control_cpu->current_freq = cluster->cur_freq;
	pid_control_cpu->last_usage_speed_err = 0;
	pid_control_cpu->usage_speed_err_sum = 0;
	pid_control_cpu->last_freq_err = 0;
	pid_control_cpu->freq_err_sum = 0;
	pid_control_cpu->last_window_time = now;
	pid_control_cpu->last_freq_time = now;
	pid_control_cpu->target_load_out = pid_control_target_load;
	pid_control_cpu->pid_freq_out = cluster->min_freq;
	for (i = 0; i < NR_PHASE_WINDOW; i++)
		pid_control_cpu->history_usage[i] = 0;
}

unsigned int get_rtg_util(int cpu, struct task_struct *p)
{
	struct related_thread_group *grp = NULL;
	struct frame_info *frame_info = NULL;
	int cpu_min_util;
	int cpu_max_util;
	int frame_util;

	rcu_read_lock();
	grp = task_related_thread_group(p);
	rcu_read_unlock();

	if (grp == NULL || grp->rtg_class == NULL)
		return 0;
	frame_info = (struct frame_info *)grp->private_data;
	if (!frame_info)
		return 0;
	cpu_min_util = freq_to_util(cpu, cpu_rq(cpu)->cluster->min_freq);
	cpu_max_util = cpu_rq(cpu)->cpu_capacity_orig;
	frame_util = frame_info->frame_util;

	if (frame_util >= cpu_min_util && frame_util <= cpu_max_util)
		return util_to_freq(cpu, frame_util);
	else
		return 0;
}

void update_integral(int error, u64 tick_us_num, int *integral, int max_num)
{
	int i;
	int integral_tmp = *integral;
	int tick_num = min(max((tick_us_num + time_ms_to_us_num / 2) / time_ms_to_us_num, 1), NR_PHASE_WINDOW);

	if (abs(error) < max_num / DETACH_THRESHOLD_RATIO)
		return;
	integral_tmp = min(max(integral_tmp, -max_num), max_num);
	for (i = 0; i < tick_num; i++) {
		if (integral_tmp > 0)
			integral_tmp = integral_tmp * (max_num - integral_tmp) / max_num;
		else
			integral_tmp = integral_tmp * (max_num + integral_tmp) / max_num;
	}
	*integral = integral_tmp;
}

int count_usage_speed(int cpu, int now_usage, u64 tick_us_num)
{
	int i;
	int usage_sum = now_usage;
	int speed = 0;
	int tmp_cycles;
	struct pid_control_cpu_info *pid_control_cpu = &per_cpu(cpu_info, cpu);
	int tick_num = max((tick_us_num + time_ms_to_us_num / 2) / time_ms_to_us_num, 1);

	if (tick_us_num == 0)
		return 0;

	// sometime scheduler tick not in the history need calculate
	for (i = NR_PHASE_WINDOW - 1; (i - tick_num) >= 0; i--) {
		pid_control_cpu->history_usage[i] = pid_control_cpu->history_usage[i - tick_num];
		usage_sum += pid_control_cpu->history_usage[i];
	}
	for (i = 1; i < min(tick_num, NR_PHASE_WINDOW); i++) {
		tmp_cycles = phase_data_of_pevent(&cpu_rq(cpu)->pcount_nr_history[i], CYCLES);
		pid_control_cpu->history_usage[i] = min(MAX_CPU_USAGE,
			(u64)(MAX_CPU_USAGE * tmp_cycles) / (u64)(pid_window_size * pid_control_cpu->window_avg_freq));
		usage_sum += pid_control_cpu->history_usage[i];
	}
	// history[0] means recent window
	pid_control_cpu->history_usage[0] = now_usage;
	if (usage_sum == 0)
		return 0;

	for (i = 0; i < NR_PHASE_WINDOW / 2; i++)
		speed += pid_control_cpu->history_usage[i] -
			pid_control_cpu->history_usage[NR_PHASE_WINDOW - i - 1];
	speed /= (NR_PHASE_WINDOW / 2);
	speed = min(max(speed * MAX_SPEED / usage_sum, -MAX_SPEED), MAX_SPEED);
	return speed;
}

int speed_pid_control(int cpu, int speed, u64 tick_us_num)
{
	int speed_target;
	int error = speed;
	int p_speed, i_speed, d_speed;
	struct pid_control_cpu_info *pid_control_cpu = &per_cpu(cpu_info, cpu);

	update_integral(error, tick_us_num, &(pid_control_cpu->usage_speed_err_sum), MAX_SPEED);
	pid_control_cpu->usage_speed_err_sum += error;
	p_speed = (int)speed_pid[0] * error;
	i_speed = (int)speed_pid[1] * pid_control_cpu->usage_speed_err_sum;
	d_speed = -(int)speed_pid[2] * (error - pid_control_cpu->last_usage_speed_err);
	speed_target = (p_speed + i_speed + d_speed) / MAX_SPEED;
	pid_control_cpu->last_usage_speed_err = error;
	trace_perf(usage_speed_pid, cpu, speed, pid_control_cpu, p_speed, i_speed, d_speed, speed_target);
	return speed_target;
}

void update_avg_freq(int cpu)
{
	struct pid_control_cpu_info *pid_control_cpu = &per_cpu(cpu_info, cpu);
	u64 now = ktime_get_ns();
	u64 tick_us_num = (now - pid_control_cpu->last_freq_time) / one_tick_us_time;

	if (tick_us_num > NR_PHASE_WINDOW * time_ms_to_us_num || pid_control_cpu->last_freq_time > now)
		pid_control_cpu->window_avg_freq = pid_control_cpu->current_freq;
	else
		pid_control_cpu->window_avg_freq +=
			(now - pid_control_cpu->last_freq_time) * (u64)(pid_control_cpu->current_freq) / one_tick_time;
	trace_perf(update_avg_freq, cpu, pid_control_cpu->window_avg_freq,
		pid_control_cpu->current_freq, now, pid_control_cpu->last_freq_time);
	pid_control_cpu->last_freq_time = now;
}

int count_cpu_usage(int cpu, u64 cpu_cycles, u64 now, u64 tick_us_num)
{
	int cpu_usage;
	struct pid_control_cpu_info *pid_control_cpu = &per_cpu(cpu_info, cpu);
	struct sched_cluster *cluster = cpu_rq(cpu)->cluster;

	if (tick_us_num > NR_PHASE_WINDOW * time_ms_to_us_num) {
		pid_control_cpu->window_avg_freq = cluster->cur_freq;
	} else {
		pid_control_cpu->window_avg_freq +=
			(now - pid_control_cpu->last_freq_time) * (u64)(cluster->cur_freq) / one_tick_time;
		pid_control_cpu->window_avg_freq = pid_control_cpu->window_avg_freq * time_ms_to_us_num / tick_us_num;
	}

	pid_control_cpu->window_avg_freq =
		max(min(pid_control_cpu->window_avg_freq, cluster->max_freq), cluster->min_freq);

	cpu_usage = min(MAX_CPU_USAGE,
		(u64)(MAX_CPU_USAGE * cpu_cycles) / (u64)(pid_window_size * pid_control_cpu->window_avg_freq));
	return cpu_usage;
}

void count_target_usage(int cpu, u64 tick_us_num, int cpu_usage)
{
	int speed, speed_target;
	struct pid_control_cpu_info *pid_control_cpu = &per_cpu(cpu_info, cpu);
	// use speed change final_target
	tick_us_num = min(tick_us_num, NR_PHASE_WINDOW * time_ms_to_us_num);

	speed = count_usage_speed(cpu, cpu_usage, tick_us_num);
	speed_target = speed_pid_control(cpu, speed, tick_us_num);
	// speed only use fast up
	pid_control_cpu->target_load_out
		= min(max(pid_control_target_load - speed_target, pid_control_target_load / 2), pid_control_target_load);
}

int calculate_pid_freq(int cpu, int cpu_usage, u64 tick_us_num)
{
	int curr_util, target_util, normalized_util_error;
	int p_freq, i_freq, d_freq, freq_out;
	struct pid_control_cpu_info *pid_control_cpu = &per_cpu(cpu_info, cpu);
	struct sched_cluster *cluster = cpu_rq(cpu)->cluster;
	int cluster_min_util = freq_to_util(cpu, cluster->min_freq);
	int cpu_capacity_orig = cpu_rq(cpu)->cpu_capacity_orig;
	int p_ratio = PID_PARAM_MAX_RATIO;
	int d_ratio = PID_PARAM_MAX_RATIO;
	int curr_freq;

	if (tick_us_num == 0)
		return pid_control_cpu->pid_freq_out;

	if (cluster->id == max_cap_cluster()->id) {
		curr_freq = pid_control_cpu->window_avg_freq;
		cpu_usage = max(cpu_usage, pid_control_cpu->history_usage[1]);
	} else {
		curr_freq = pid_control_cpu->pid_freq_out;
	}

	if (cpu_usage > pid_control_target_load)
		cpu_usage = pid_control_target_load + (cpu_usage - pid_control_target_load) *
			pid_control_target_load / (MAX_CPU_USAGE - pid_control_target_load);
	curr_util = (int)freq_to_util(cpu, curr_freq);
	target_util = curr_util * cpu_usage / pid_control_cpu->target_load_out;
	target_util = max(min(target_util, cpu_capacity_orig), cluster_min_util);
	normalized_util_error = (target_util - curr_util) * (int)SCHED_CAPACITY_SCALE / (int)(cpu_capacity_orig);

	pid_control_cpu->freq_err_sum += normalized_util_error;
	update_integral(normalized_util_error,
		tick_us_num, &(pid_control_cpu->freq_err_sum), (cpu_capacity_orig - cluster_min_util));

	if (pid_control_cpu->pid_freq_out >  (cluster->min_freq +
		(cluster->max_freq - cluster->min_freq) * pid_control_target_load / MAX_CPU_USAGE)) {
		p_ratio = PID_PARAM_P_RATIO;
		d_ratio = PID_PARAM_D_RATIO;
	}

	p_freq = (int)freq_pid[0] * normalized_util_error * p_ratio / PID_PARAM_MAX_RATIO;
	i_freq = (int)freq_pid[1] * pid_control_cpu->freq_err_sum;
	d_freq = -(int)freq_pid[2] * (normalized_util_error - pid_control_cpu->last_freq_err) *
		d_ratio / PID_PARAM_MAX_RATIO;

	freq_out = curr_freq +
		(long)(p_freq + i_freq + d_freq) * (long)cpu_rq(cpu)->cluster->max_freq / (long)max_cap_cluster()->max_freq;

	pid_control_cpu->last_freq_err = normalized_util_error;
	freq_out = max(min(freq_out, cpu_rq(cpu)->cluster->max_freq), cpu_rq(cpu)->cluster->min_freq);
	trace_printk("cpu=%d curr_util=%d target_util=%d pid_control_cpu->freq_err_sum=%d "
		"p_freq=%d i_freq=%d d_freq=%d avg_freq=%llu speed_final_target=%d normalized_util_error=%d "
		"freq_out=%d cpu_capacity_orig=%d cpu_usage=%d\n", cpu, curr_util, target_util, pid_control_cpu->freq_err_sum,
		p_freq, i_freq, d_freq, pid_control_cpu->window_avg_freq, pid_control_cpu->target_load_out,
		normalized_util_error, freq_out, (int)cpu_capacity_orig, cpu_usage);
	return freq_out;
}

int update_pid_freq(int cpu, int flags, u64 cpu_cycles)
{
	struct pid_control_cpu_info *pid_control_cpu = &per_cpu(cpu_info, cpu);
	int freq_out, cpu_usage, pid_mips_info;
	u64 tick_us_num;
	u64 now = ktime_get_ns();

	tick_us_num = (now - pid_control_cpu->last_window_time) / one_tick_us_time;
	if (flags != MIPS_UPDATE && tick_us_num >= NR_TICK_LOSS_WIN * time_ms_to_us_num)
		return 0;

	if (flags != MIPS_UPDATE || pid_control_cpu->last_window_time >= now)
		return pid_control_cpu->pid_freq_out;

	if (tick_us_num > NR_PHASE_WINDOW * time_ms_to_us_num)
		init_rq_pid_freq(pid_control_cpu, cpu_rq(cpu)->cluster);

	// step1: count avg freq and get cpu usage
	cpu_usage = count_cpu_usage(cpu, cpu_cycles, now, tick_us_num);
	// step2: use speed pid control change final_target
	count_target_usage(cpu, tick_us_num, cpu_usage);
	// step3: pid control calculate
	freq_out = calculate_pid_freq(cpu, cpu_usage, tick_us_num);

	trace_perf(pid_freq_event, cpu, cpu_cycles, cpu_usage, pid_control_cpu, cpu_rq(cpu)->cluster->cur_freq);
	// 10000: int lower 4 digits display freq_out, others display cpu_cycles
	pid_mips_info = (cpu_cycles / PID_INFO_LOWER_NUM) * PID_INFO_LOWER_NUM + (freq_out / PID_INFO_ZOOM_TIMES);
	trace_clock_set_rate("pid-mips-info", pid_mips_info, cpu);
	pid_control_cpu->last_window_time = now;
	pid_control_cpu->last_freq_time = now;
	pid_control_cpu->pid_freq_out = freq_out;
	pid_control_cpu->window_avg_freq = 0;
	return freq_out;
}

static int cpufreq_notifier_trans(struct notifier_block *nb,
	unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = (struct cpufreq_freqs *)data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))
	unsigned int cpu = freq->policy->cpu;
#else
	unsigned int cpu = freq->cpu;
#endif
	struct cpumask policy_cpus = cpu_rq(cpu)->freq_domain_cpumask;
	struct pid_control_cpu_info *pid_control_cpu;
	int i;

	if (val != CPUFREQ_POSTCHANGE)
		return 0;

	for_each_cpu(i, &policy_cpus) {
		update_avg_freq(i);
		pid_control_cpu = &per_cpu(cpu_info, i);
		pid_control_cpu->current_freq = freq->new;
	}

	return 0;
}

static struct notifier_block notifier_trans_block = {
	.notifier_call = cpufreq_notifier_trans
};

static int register_sched_callback(void)
{
	return cpufreq_register_notifier(&notifier_trans_block,
		CPUFREQ_TRANSITION_NOTIFIER);
}

core_initcall(register_sched_callback);
