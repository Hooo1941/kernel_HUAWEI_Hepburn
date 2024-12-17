/*
 *
 * lpcpu cpuidle menu gov
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/cpuidle.h>
#include <linux/cpumask.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/tick.h>
#include <linux/cpufreq.h>
#include <linux/hrtimer.h>
#include <linux/compiler_attributes.h>
#include <platform_include/cee/linux/lpcpu_idle_drv.h>
#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
#define CREATE_TRACE_POINTS
#include <trace/events/cpuidle.h>
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include "lpcpu_cpuidle_menu.h"

#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
/*
 * CPU incorrectly goto deep C-state multi times in a short time.
 * In order to fix the problem, record the cpu idle history info.
 * the max num of history info is HIST_SIZE.
 * if more than HIST_FAILED_COUNT history record have less idle time
 * than the pre-select C-stat, reselect again.
 * The valid window width of history record is HIST_DATA_INV_GAP_US.
 * Earlier record not be used.
 */
#define HIST_FAILED_COUNT 2
#define HIST_DATA_INV_GAP_NS (20000ULL * NSEC_PER_USEC)

#define HIST_TASK_RUNTIME_NS	(1000ULL * NSEC_PER_USEC)
#define HIST_MIN_RESIDENCY_NS	(4000ULL * NSEC_PER_USEC)
#define HIST_RESIDENCY_NS	(500ULL * NSEC_PER_USEC)

#define HIST_INV_PREDICT_CN 2
#define HIST_INV_REPEAT_CN 2

#define MENU_HRTIMER_DELTA_TIME_NS	(500ULL * NSEC_PER_USEC)
#define MENU_HRTIMER_MIN_TIME_NS	(4000ULL * NSEC_PER_USEC)
#define MENU_HRTIMER_MAX_TIME_NS	(20000ULL * NSEC_PER_USEC)

#ifdef CONFIG_ARCH_HAS_CPU_RELAX
#define CPUIDLE_DRIVER_STATE_START	1
#else
#define CPUIDLE_DRIVER_STATE_START	0
#endif /* CONFIG_ARCH_HAS_CPU_RELAX */

#endif /* CONFIG_CPUIDLE_MENU_GOV_HIST */

/* 60 * 60 > STDDEV_THRESH * INTERVALS = 400 * 8 */
#define MAX_DEVIATION	(60ULL *  NSEC_PER_USEC)
static DEFINE_PER_CPU(int, hrtimer_status); //lint !e129
/*lint -e528 -esym(528,*)*/
static DEFINE_PER_CPU(struct hrtimer, menu_hrtimer);
/*lint -e528 +esym(528,*)*/
static unsigned int menu_switch_profile __read_mostly;
static struct cpumask menu_cpumask;

/* menu hrtimer mode */
enum {
	MENU_HRTIMER_STOP,
	MENU_HRTIMER_REPEAT,
	MENU_HRTIMER_GENERAL,
#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	MENU_HRTIMER_INV,
#endif
};

static unsigned int perfect_cstate_ms __read_mostly = 30;
module_param(perfect_cstate_ms, uint, 0000);
static unsigned int menu_hrtimer_enable __read_mostly;

/* add cpufreq notify block for dvfs target profile */
/*lint -e715*/
static int menu_cpufreq_callback(struct notifier_block *nb __always_unused,
				       unsigned long event, void *data)
{
	struct cpufreq_freqs *freq = data;

	if (!cpumask_intersects(freq->policy->cpus, &menu_cpumask))
		return 0;

	if (event != CPUFREQ_POSTCHANGE)
		return 0;
	if (menu_switch_profile <= freq->new)
		menu_hrtimer_enable = 1;
	else
		menu_hrtimer_enable = 0;
	return 0;
}
/*lint +e715*/
/*lint -e785*/
static struct notifier_block menu_cpufreq_notifier = {
	.notifier_call  = menu_cpufreq_callback,
};
/*lint +e785*/
int __init register_menu_cpufreq_notifier(void)
{
	int ret;

	ret = cpufreq_register_notifier(&menu_cpufreq_notifier,
		CPUFREQ_TRANSITION_NOTIFIER);
	return ret;
}
/*lint -e64 -e507 -e530 */
/* Cancel the hrtimer if it is not triggered yet */
void menu_hrtimer_cancel(void)
{
	unsigned int cpu = smp_processor_id();
	struct hrtimer *hrtmr = &per_cpu(menu_hrtimer, cpu);

	/* The timer is still not time out */
	if (per_cpu(hrtimer_status, cpu)) {
		hrtimer_cancel(hrtmr);
		per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_STOP;
	}
}
EXPORT_SYMBOL_GPL(menu_hrtimer_cancel);

static DEFINE_PER_CPU(struct menu_device, menu_devices);

/* Call back for hrtimer is triggered */
static enum hrtimer_restart menu_hrtimer_notify(struct hrtimer *phrtimer)
{
	unsigned int cpu = smp_processor_id();
	struct menu_device *data = &per_cpu(menu_devices, cpu);
#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	int wakeup_status;
#endif

	if (!phrtimer)
		return HRTIMER_NORESTART;

	/* In general case, the expected residency is much larger than
	 *  deepest C-state target residency, but prediction logic still
	 *  predicts a small predicted residency, so the prediction
	 *  history is totally broken if the timer is triggered.
	 *  So reset the correction factor.
	 */

#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	wakeup_status = per_cpu(hrtimer_status, cpu);
	per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_STOP;

	if (wakeup_status == MENU_HRTIMER_GENERAL)
		data->correction_factor[data->bucket] = RESOLUTION * DECAY;
	else if (wakeup_status == MENU_HRTIMER_INV)
		data->hist_inv_flag = true;
	else if (wakeup_status == MENU_HRTIMER_REPEAT)
		data->hist_inv_repeat++;
	else
		return HRTIMER_NORESTART;

	data->hrtime_out = true;

	trace_perf(menu_hrtimerout, cpu, wakeup_status, data->hrtime_ns);
#else
	if (per_cpu(hrtimer_status, cpu) == MENU_HRTIMER_GENERAL)
		data->correction_factor[data->bucket] = RESOLUTION * DECAY;
	per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_STOP;
#endif

	return HRTIMER_NORESTART;
}

#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
/**
 * menu_hist_calc_stat_info - statistics the history idle info for
 *    reselect idle stat.including:
 *    a) number and time length of same pre-select idle state
 *       with less idle time than residency_thresh.
 *    b) busy status info, as total running time/ idle time, last run time.
 * @residency_thresh: residency threshold for select excessive deep idle state.
 * @stat_info: the result of history idle state info
 */
static void menu_hist_calc_stat_info(u64 residency_thresh_ns,
			struct menu_hist_state_info *stat_info)
{
	int i;
	int hist_ptr;
	u64 min_residency_ns = HIST_DATA_INV_GAP_NS;
	s64 stime;
	s64 enter_time;
	s64 first_enter_time;
	s64 last_exit_time = 0;
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	struct menu_hist_cstate_data *hist_data;

	stime = ktime_get();
	first_enter_time = stime;
	hist_ptr = data->hist_ptr;

	for (i = 0 ; i < HIST_SIZE ; i++) {
		hist_ptr--;
		if (hist_ptr < 0)
			hist_ptr += HIST_SIZE;
		hist_data = &data->hist_data[hist_ptr];

		if (hist_data->residency_ns == 0)
			continue;

		/* calc last exit time for last run time. */
		if (last_exit_time < hist_data->exit_time_ns)
			last_exit_time = hist_data->exit_time_ns;
		if (min_residency_ns > hist_data->residency_ns)
			min_residency_ns = hist_data->residency_ns;

		enter_time = hist_data->exit_time_ns - hist_data->residency_ns;
		if (enter_time < 0)
			continue;
		if (enter_time + HIST_DATA_INV_GAP_NS < stime)
			continue;

		stat_info->total_count++;
		stat_info->total_residency_ns += hist_data->residency_ns;

		if (first_enter_time > enter_time)
			first_enter_time = enter_time;

		if (hist_data->residency_ns < residency_thresh_ns) {
			if (hist_data->org_state_idx == data->last_state_idx) {
				stat_info->same_state_failed_count++;
				stat_info->same_state_failed_ns +=
					hist_data->residency_ns;
			}
		}

		trace_perf(menu_hist_data, hist_ptr, hist_data);
	}

	stat_info->total_ns =
		min_t(u64, HIST_DATA_INV_GAP_NS, stime - first_enter_time);
	stat_info->total_residency_ns =
		min_t(u64, HIST_DATA_INV_GAP_NS, stat_info->total_residency_ns);

	stat_info->last_run_ns = stime - last_exit_time;
	stat_info->min_residency_ns = min_residency_ns;

	trace_perf(menu_hist_info, stat_info, residency_thresh_ns, stime);
}

static int menu_hist_sel_state_idx(struct cpuidle_driver *drv,
			struct cpuidle_device *dev,
			u64 target_residency_ns, u64 exit_latency_ns)
{
	int i;
	int last_state_idx = CPUIDLE_DRIVER_STATE_START;

	/* Use history data to choice new idle state */
	for (i = CPUIDLE_DRIVER_STATE_START + 1; i < drv->state_count; i++) {
		struct cpuidle_state_usage *lu = &dev->states_usage[i];
		struct cpuidle_state *l = &drv->states[i];

		if (lu->disable)
			continue;

		if (l->target_residency_ns > target_residency_ns)
			continue;

		if (l->exit_latency_ns > exit_latency_ns)
			continue;

		last_state_idx = i;
	}

	return last_state_idx;
}

/**
 * menu_hist_get_busy_flag - get cpu busy status
 * @stat_info: the statistics info of history idle
 */
static int menu_hist_get_busy_flag(struct menu_hist_state_info *stat_info)
{
	/*
	 * the last task has a long running time. and
	 * the min idle time is less than threshold.
	 */
	if (stat_info->last_run_ns > HIST_TASK_RUNTIME_NS &&
	    stat_info->min_residency_ns < HIST_MIN_RESIDENCY_NS)
		return 1;

	/*
	 * there are multi idle history records which has short idle time.
	 */
	if (stat_info->total_count > HIST_SIZE / 2 &&
	    stat_info->total_residency_ns <
	    stat_info->total_count * HIST_RESIDENCY_NS)
		return 1;

	/*
	 * task is busy, do not goto deep sleep.
	 */
	if (stat_info->total_residency_ns + HIST_DATA_INV_GAP_NS / 2 <
	    stat_info->total_ns ||
	    stat_info->total_residency_ns * 3 < stat_info->total_ns)
		return 1;

	return 0;
}

/**
 * menu_hist_busy_resel - reslect idle state by busy status.
 * @stat_info: the statistics info of history idle
 */
static inline void menu_hist_busy_resel(struct menu_device *data,
			struct menu_hist_state_info *stat_info)
{
	int hist_busy_flag;

	hist_busy_flag = menu_hist_get_busy_flag(stat_info);
	if (!hist_busy_flag)
		return;

	if (data->last_state_idx < 1)
		return;
	data->last_state_idx--;

	if (stat_info->total_count) {
		do_div(stat_info->total_residency_ns, stat_info->total_count);
		data->predicted_ns = min_t(u64, data->predicted_ns,
			stat_info->total_residency_ns);
	} else {
		data->predicted_ns = min_t(u64, data->predicted_ns,
			MENU_HRTIMER_MIN_TIME_NS);
	}
}

/**
 * menu_hist_check_state_idx - check state idx enable or disable
 * @drv: cpuidle driver containing state data
 * @dev: cpuidle_device
 * @state_idx: cpu idle state idx
 * return: 1: enable; 0: disabled
 */
static int menu_hist_check_state_idx(struct cpuidle_driver *drv,
		struct cpuidle_device *dev, int state_idx)
{
	if (state_idx >= drv->state_count)
		return 0;

	if (dev->states_usage[state_idx].disable)
		return 0;

	return 1;
}

/**
 * menu_hist_reselect - reselects the next idle state to enter
 *    with history idle info
 * @drv: cpuidle driver containing state data
 * @dev: the CPU
 * @latency_req: latency qos request
 */
static void menu_hist_reselect(struct cpuidle_driver *drv,
			struct cpuidle_device *dev, u64 latency_req_ns)
{
	unsigned int failed_cnt_th = HIST_FAILED_COUNT;
	struct menu_device *data = this_cpu_ptr(&menu_devices);

	data->org_state_idx = data->last_state_idx;

	if (data->last_state_idx > 0 &&
	    cpumask_test_cpu((int)dev->cpu, &menu_cpumask) &&
	    data->hist_inv_repeat  < HIST_INV_REPEAT_CN &&
	    data->hist_inv_predict < HIST_INV_PREDICT_CN) {
		struct menu_hist_state_info stat_info = {0};
		struct cpuidle_state *s = &drv->states[data->last_state_idx];

		menu_hist_calc_stat_info(
			(s->target_residency_ns + s->exit_latency_ns) / 2, &stat_info);

		if (data->last_state_idx + 1 == drv->state_count)
			failed_cnt_th++;

		if (stat_info.same_state_failed_count >= failed_cnt_th) {
			do_div(stat_info.same_state_failed_ns,
				stat_info.same_state_failed_count);

			data->predicted_ns = min_t(u64, data->predicted_ns,
				stat_info.same_state_failed_ns);

			data->last_state_idx = menu_hist_sel_state_idx(drv, dev,
				data->predicted_ns, latency_req_ns);
		} else if (data->last_state_idx + 1 == drv->state_count) {
			menu_hist_busy_resel(data, &stat_info);
		}
	}

	if (data->hist_inv_predict == 1 &&
	    data->last_state_idx < (drv->state_count - 1)) {
		data->last_state_idx++;

		if (data->last_state_idx < data->org_state_idx)
			data->last_state_idx++;
	}

	if (data->org_state_idx != data->last_state_idx) {
		if (!menu_hist_check_state_idx(drv, dev, data->last_state_idx))
			data->last_state_idx = data->org_state_idx;
	}

	if (data->hist_inv_predict >= HIST_INV_PREDICT_CN)
		data->hist_inv_predict = 0;

	if (data->hist_inv_repeat >= HIST_INV_REPEAT_CN)
		data->hist_inv_repeat = 0;
}

/**
 * menu_sel_hrtimer_algo - calc the hrtime length and type.
 * @dev: the CPU
 * @hrtimer_type: latency qos request
 */
static u64 menu_sel_hrtimer_algo(struct menu_device *data,
				struct cpuidle_driver *drv, int *hrtimer_type)
{
	u64 hrtime_ns = 0;
	u64 last_time_ns;
	u64 next_time_ns;

	struct cpuidle_state *s_next = NULL;
	struct cpuidle_state *s_last = NULL;

	if (data->last_state_idx + 1 >= drv->state_count)
		return 0;

	s_last = &drv->states[data->last_state_idx]; /*lint !e679*/
	s_next = &drv->states[data->last_state_idx + 1]; /*lint !e679*/

	last_time_ns = s_last->target_residency_ns + s_last->exit_latency_ns;
	next_time_ns = s_next->target_residency_ns + s_next->exit_latency_ns;

	/* Avoid new predictions result cause shallow sleep */
	if (data->last_state_idx != data->org_state_idx &&
	    data->next_timer_ns > last_time_ns + next_time_ns) {
		hrtime_ns = data->predicted_ns + MENU_HRTIMER_DELTA_TIME_NS;
		hrtime_ns = max(hrtime_ns, s_next->target_residency_ns / 2);
		hrtime_ns = min(hrtime_ns, s_next->target_residency_ns);
		hrtime_ns = max(hrtime_ns, s_last->target_residency_ns);

		hrtime_ns = max_t(u64, hrtime_ns, MENU_HRTIMER_MIN_TIME_NS);
		hrtime_ns = min_t(u64, hrtime_ns, MENU_HRTIMER_MAX_TIME_NS);

		*hrtimer_type = MENU_HRTIMER_INV;
		return hrtime_ns;
	}

	/* Avoid error prediction cause shallow sleep */
	if (data->next_timer_ns > perfect_cstate_ms * NSEC_PER_MSEC ||
	    data->next_timer_ns > data->predicted_ns + next_time_ns) {
		hrtime_ns = data->predicted_ns + MENU_HRTIMER_DELTA_TIME_NS;
		hrtime_ns = max(hrtime_ns, s_next->target_residency_ns / 2);
		hrtime_ns = min(hrtime_ns, s_next->target_residency_ns);
		hrtime_ns = max(hrtime_ns, s_last->target_residency_ns);

		hrtime_ns = max_t(u64, hrtime_ns, MENU_HRTIMER_MIN_TIME_NS);
		hrtime_ns = min_t(u64, hrtime_ns, MENU_HRTIMER_MAX_TIME_NS);

		if (data->repeat)
			*hrtimer_type = MENU_HRTIMER_REPEAT;
		else
			*hrtimer_type = MENU_HRTIMER_GENERAL;

		return hrtime_ns;
	}

	return 0;
}

static void menu_sel_hrtimer_start(struct cpuidle_driver *drv,
			struct cpuidle_device *dev)
{
	unsigned int cpu = dev->cpu;
	struct hrtimer *ek_hrtimer = NULL;
	struct menu_device *data = this_cpu_ptr(&menu_devices);
	int hrtimer_type = MENU_HRTIMER_STOP;

	data->hrtime_ns = 0;
	if (menu_hrtimer_enable &&
	    cpumask_test_cpu((int)cpu, &menu_cpumask) &&
	    data->last_state_idx + 1 < drv->state_count) {
		data->hrtime_ns = menu_sel_hrtimer_algo(data,
						drv, &hrtimer_type);
		per_cpu(hrtimer_status, cpu) = hrtimer_type;

		if (data->hrtime_ns) {
			ek_hrtimer = &per_cpu(menu_hrtimer, cpu);
			hrtimer_start(ek_hrtimer, ns_to_ktime(data->hrtime_ns),
				      HRTIMER_MODE_REL_PINNED);
		}
	}
}

static inline void menu_hist_update_inv_predict(struct menu_device *data)
{
	if (data->hist_inv_flag) {
		if (data->hist_inv_predict < HIST_INV_PREDICT_CN)
			data->hist_inv_predict++;
	} else {
		data->hist_inv_predict = 0;
	}
}

static inline void menu_hist_update_interval_ptr(struct menu_device *data)
{
	data->interval_ptr++;
	if (data->interval_ptr >= INTERVALS)
		data->interval_ptr = 0;
}

static inline void menu_hist_update_hist_ptr(struct menu_device *data)
{
	data->hist_ptr++;
	if (data->hist_ptr >= HIST_SIZE)
		data->hist_ptr = 0;
}

static inline void menu_hist_update_inv_repeat(struct menu_device *data)
{
	if (data->hist_inv_repeat > 0 &&
	    data->hist_inv_repeat_old == data->hist_inv_repeat)
		data->hist_inv_repeat--;

	data->hist_inv_repeat_old = data->hist_inv_repeat;
}

/**
 * menu_hist_update - update history info
 * @measured_ns: last idle time length
 */
static void menu_hist_update(struct menu_device *data, u64 measured_ns)
{
	if (data->hrtime_addtime) {
		data->intervals[data->interval_ptr] += ktime_to_us(measured_ns);
		data->hist_data[data->hist_ptr].residency_ns += measured_ns;
	} else {
		data->intervals[data->interval_ptr] = ktime_to_us(measured_ns);
		data->hist_data[data->hist_ptr].residency_ns = measured_ns;
	}

	/* Update history data */
	menu_hist_update_inv_predict(data);

	trace_perf(menu_update, data->hist_data[data->hist_ptr].residency_ns,
		data->hist_data[data->hist_ptr].idle_state_idx,
		data->hist_data[data->hist_ptr].exit_time_ns,
		data->hist_inv_flag,
		data->hist_inv_repeat,
		data->hist_inv_predict,
		data->hrtime_addtime);

	data->hist_inv_flag = false;
	data->hrtime_addtime = false;

	if (data->hrtime_out) {
		/*
		 * menu gov's hrtime call up core, set flag to add the next idle time.
		 * the sum of last idle and next idle is real idle time.
		 */
		data->hrtime_out = false;
		data->hrtime_addtime = true;
	} else {
		menu_hist_update_interval_ptr(data);

		menu_hist_update_hist_ptr(data);
	}

	menu_hist_update_inv_repeat(data);
}
#endif /* CONFIG_CPUIDLE_MENU_GOV_HIST */

int get_menu_switch_profile(void)
{
	struct device_node *np;
	int ret, cpu;
	unsigned int cpu_mask;

	menu_hrtimer_enable = 0;
	menu_switch_profile = 0;

	np = of_find_compatible_node(NULL, NULL, "menu-switch");
	if (!np)
		return -ENODEV;

	ret = of_property_read_u32(np, "cpu-mask", (u32 *)&cpu_mask);
	if (ret) {
		pr_err("get menu cpumask error!\n");
		return -EFAULT;
	}

	cpumask_clear(&menu_cpumask);
	for_each_online_cpu(cpu) {
		if (BIT(cpu) & cpu_mask)
			cpumask_set_cpu(cpu, &menu_cpumask);
	}

	ret = of_property_read_u32(np, "switch-profile", &menu_switch_profile);
	if (ret)
		return -EFAULT;
	return 0;
}

void cpuidle_menu_select_op(struct cpuidle_driver *drv, struct cpuidle_device *dev,
			    bool *stop_tick, struct menu_device *data, int *idx,
			    ktime_t delta_next, s64 latency_req,
			    int low_predicted __maybe_unused)
{
	int i;
	unsigned int cpu = dev->cpu;
#ifdef CONFIG_CPUIDLE_SKIP_ALL_CORE_DOWN
	struct cpumask cluster_idle_cpu_mask;
#endif
#ifndef CONFIG_CPUIDLE_MENU_GOV_HIST
	u64 timer_ns = 0;
	u64 perfect_ns = 0;
	struct hrtimer *hrtmr = &per_cpu(menu_hrtimer, cpu);
#endif

	/*
	 * Don't stop the tick if the selected state is a polling one or if the
	 * expected idle duration is shorter than the tick period length.
	 */
	if (((drv->states[*idx].flags & CPUIDLE_FLAG_POLLING) ||
	     data->predicted_ns < TICK_NSEC) && !tick_nohz_tick_stopped()) {
		*stop_tick = false;

		if (*idx > 0 && drv->states[*idx].target_residency_ns > delta_next) {
			/*
			 * The tick is not going to be stopped and the target
			 * residency of the state to be returned is not within
			 * the time until the next timer event including the
			 * tick, so try to correct that.
			 */
			for (i = (*idx) - 1; i >= 0; i--) {
				if (dev->states_usage[i].disable)
					continue;

				*idx = i;
				if (drv->states[i].target_residency_ns <= delta_next)
					break;
			}
		}
	}

	data->last_state_idx = *idx;

#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	menu_hist_reselect(drv, dev, (u64)latency_req);

#ifdef CONFIG_CPUIDLE_SKIP_ALL_CORE_DOWN
	if (test_slow_cpu(cpu) && data->last_state_idx > 0) {
		spin_lock(&g_idle_spin_lock);
		if (num_core_idle_cpus() + num_idle_cpus() ==
			num_online_cpus() -1) {
			cpumask_and(&cluster_idle_cpu_mask, drv->cpumask,
				    &g_idle_cpus_mask);
			if (data->last_state_idx < drv->state_count -1 ||
				cpumask_weight(&cluster_idle_cpu_mask) <
				cpumask_weight(drv->cpumask) -1)
				data->last_state_idx = 0;
		}
		spin_unlock(&g_idle_spin_lock);
	}
#endif
	menu_sel_hrtimer_start(drv, dev);

	trace_perf(menu_select, dev->cpu,
		data->last_state_idx,
		data->predicted_ns,
		latency_req,
		per_cpu(hrtimer_status, cpu),
		data->org_state_idx,
		data->next_timer_ns,
		data->hrtime_ns,
		delta_next);
#else
	if ((menu_hrtimer_enable) && (low_predicted) && (cpumask_test_cpu((int)cpu, &menu_cpumask))) {
		/*
		 * Set a timer to detect whether this sleep is much
		 * longer than repeat mode predicted.  If the timer
		 * triggers, the code will evaluate whether to put
		 * the CPU into a deeper C-state.
		 * The timer is cancelled on CPU wakeup.
		 */
		timer_ns = 5 * (data->predicted_ns + MAX_DEVIATION);
		perfect_ns = perfect_cstate_ms * NSEC_PER_MSEC;

		if (data->repeat && (4 * timer_ns < data->next_timer_ns)) {
			hrtimer_start(hrtmr, ns_to_ktime(timer_ns),
				HRTIMER_MODE_REL_PINNED);
			/* In repeat case, menu hrtimer is started */
			per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_REPEAT;
		} else if (perfect_ns < data->next_timer_ns) {
			/*
			 * The next timer is long. This could be because
			 * we did not make a useful prediction.
			 * In that case, it makes sense to re-enter
			 * into a deeper C-state after some time.
			 */
			hrtimer_start(hrtmr, ns_to_ktime(timer_ns),
				HRTIMER_MODE_REL_PINNED);
			/* In general case, menu hrtimer is started */
			per_cpu(hrtimer_status, cpu) = MENU_HRTIMER_GENERAL;
		}
	}
#endif
}

void cpuidle_menu_update(struct cpuidle_device *dev, struct menu_device *data,
			 u64 measured_ns, int *hook_flag)
{
	if (cpumask_test_cpu((int)dev->cpu, &menu_cpumask)) {
		*hook_flag = 1;
#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
		menu_hist_update(data, measured_ns);
#endif
	}
}

void cpuidle_hrtimer_init(struct cpuidle_device *dev)
{
	struct hrtimer *t = &per_cpu(menu_hrtimer, dev->cpu);

	hrtimer_init(t, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	t->function = menu_hrtimer_notify;
}

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
