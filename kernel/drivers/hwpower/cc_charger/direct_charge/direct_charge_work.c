// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_work.c
 *
 * direct charge work module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/adapter/adapter_detect.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_pmode.h>

#define HWLOG_TAG direct_charge_work
HWLOG_REGIST();

enum {
	SWITCH_STRATEGY_MULTI_BAT_CHECK = 0,
	SWITCH_STRATEGY_MULTI_BAT_CHECK_OR_THERMAL,
	SWITCH_STRATEGY_MULTI_BAT_CHECK_AND_THERMAL,
};

static struct timespec64 g_last_kick_wtd_work_time;
static struct timespec64 g_last_kick_wtd_timer_func_time;

static int64_t dc_get_kick_wtd_delta_time(struct timespec64 *last_time)
{
	int64_t delta_time;
	struct timespec64 time_now = power_get_current_kernel_time64();

	if (last_time->tv_sec <= 0) {
		last_time->tv_sec = time_now.tv_sec;
		last_time->tv_nsec = time_now.tv_nsec;
	}

	delta_time = (time_now.tv_sec - last_time->tv_sec) * NSEC_PER_SEC +
		(time_now.tv_nsec - last_time->tv_nsec);
	delta_time = delta_time / NSEC_PER_MSEC;
	last_time->tv_sec = time_now.tv_sec;
	last_time->tv_nsec = time_now.tv_nsec;

	return delta_time;
}

static void dc_update_work_interval(int *time)
{
	/*
	 * UFCS protocol has a fast communication speed.
	 * In order to be consistent with SCP protocol,
	 * 200ms is selected as the time interval
	 */
	if (adapter_detect_get_runtime_protocol_type() == BIT(ADAPTER_PROTOCOL_UFCS))
		*time = 200;
}

void dc_control_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t, volt_ratio;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, control_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->stop_charging_flag_error || di->scp_stop_charging_flag_info ||
		(di->sysfs_enable_charger == 0) || di->force_disable)
		goto out;

	/* check if sc switch to lvc or lvc switch to sc */
	if (direct_charge_check_priority_inversion())
		goto out;

	if (di->local_mode != dc_pmode_get_optimal_mode(NORMAL_CHECK, di->local_mode, true)) {
		dc_comm_set_need_recheck_flag(true);
		goto out;
	}

	volt_ratio = di->dc_volt_ratio;
	if (mulit_ic_check(di->working_mode, di->cur_mode,
		&di->multi_ic_check_info, volt_ratio) < 0) {
		di->multi_ic_error_cnt++;
		dc_set_stop_charging_flag(true);
		goto out;
	}

	if ((DOUBLE_SIZE * di->stage_size == di->cur_stage) || direct_charge_check_timeout()) {
		hwlog_info("cur_stage=%d, vbat=%d, ibat=%d, direct charge done\n", di->cur_stage, di->vbat, di->ibat);
		direct_charge_set_stage_status(DC_STAGE_CHARGE_DONE);
		goto out;
	}

	dc_check_recharge_path();
	dc_select_charge_path();
	dc_charge_regulation();
	direct_charge_update_charge_info();

	t = di->charge_control_interval;
	dc_update_work_interval(&t);
	hrtimer_start(&di->control_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
	return;

out:
	direct_charge_stop_charging();
	return;
}

void dc_calc_thld_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, calc_thld_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->stop_charging_flag_error || di->scp_stop_charging_flag_info ||
		(di->sysfs_enable_charger == 0)) {
		hwlog_info("direct charge stop, stop calc threshold\n");
		return;
	}

	if (di->pri_inversion || dc_comm_get_need_recheck_flag())
		return;

	direct_charge_soh_policy();
	direct_charge_select_charging_stage();
	direct_charge_select_charging_param();

	if (DOUBLE_SIZE * di->stage_size == di->cur_stage) {
		hwlog_info("direct charge done, stop calc threshold\n");
		return;
	}

	t = di->threshold_caculation_interval;
	dc_update_work_interval(&t);
	hrtimer_start(&di->calc_thld_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void dc_anomaly_det_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t = DC_ANOMALY_DET_INTERVAL;
	int ret;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, anomaly_det_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->stop_charging_flag_error || di->scp_stop_charging_flag_info ||
		(di->sysfs_enable_charger == 0)) {
		hwlog_info("direct charge stop, stop anomaly detect threshold\n");
		return;
	}

	ret = dc_adpt_22p5w_det(di->working_mode, di->cur_ibat_th_high, di->dc_volt_ratio, di->cur_stage);
	if (ret == DC_NO_NEED_ANOMALY_DET) {
		hwlog_info("not need direct charge anomaly detect, stop anomaly detect threshold\n");
		return;
	}

	hrtimer_start(&di->anomaly_det_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

void dc_kick_wtd_work(struct work_struct *work)
{
	struct direct_charge_device *di = NULL;
	int t;
	int ibat = 0;
	int64_t delta_time;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, kick_wtd_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	delta_time = dc_get_kick_wtd_delta_time(&g_last_kick_wtd_work_time);
	/* print if kick wtd work real interval exceeds kick_wtd_time 1000ms */
	if (delta_time >= di->kick_wtd_time + 1000)
		hwlog_err("%s: kick wtd work interval %ldms\n", __func__, delta_time);

	if (dcm_kick_ic_watchdog(di->working_mode, di->cur_mode))
		direct_charge_get_bat_current(&ibat);

	t = di->kick_wtd_time;
	hrtimer_start(&di->kick_wtd_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

enum hrtimer_restart dc_calc_thld_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, calc_thld_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->charging_wq, &di->calc_thld_work);

	return HRTIMER_NORESTART;
}

enum hrtimer_restart dc_anomaly_det_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, anomaly_det_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->charging_wq, &di->anomaly_det_work);

	return HRTIMER_NORESTART;
}

enum hrtimer_restart dc_control_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, control_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	queue_work(di->charging_wq, &di->control_work);

	return HRTIMER_NORESTART;
}

enum hrtimer_restart dc_kick_wtd_timer_func(struct hrtimer *timer)
{
	struct direct_charge_device *di = NULL;
	int64_t delta_time;

	if (!timer) {
		hwlog_err("timer is null\n");
		return HRTIMER_NORESTART;
	}

	di = container_of(timer, struct direct_charge_device, kick_wtd_timer);
	if (!di) {
		hwlog_err("di is null\n");
		return HRTIMER_NORESTART;
	}

	delta_time = dc_get_kick_wtd_delta_time(&g_last_kick_wtd_timer_func_time);
	/* print if kick wtd timer real interval exceeds kick_wtd_time 1000ms */
	if (delta_time >= di->kick_wtd_time + 1000)
		hwlog_err("%s: kick wtd timer interval %ldms\n", __func__, delta_time);

	queue_work(di->kick_wtd_wq, &di->kick_wtd_work);

	return HRTIMER_NORESTART;
}

void dc_init_work(int local_mode)
{
	struct direct_charge_device *l_di = direct_charge_get_di_by_mode(local_mode);

	if (!l_di)
		return;

	INIT_WORK(&l_di->calc_thld_work, dc_calc_thld_work);
	INIT_WORK(&l_di->control_work, dc_control_work);
	INIT_WORK(&l_di->kick_wtd_work, dc_kick_wtd_work);
	INIT_WORK(&l_di->anomaly_det_work, dc_anomaly_det_work);

	hrtimer_init(&l_di->calc_thld_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	l_di->calc_thld_timer.function = dc_calc_thld_timer_func;
	hrtimer_init(&l_di->control_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	l_di->control_timer.function = dc_control_timer_func;
	hrtimer_init(&l_di->kick_wtd_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	l_di->kick_wtd_timer.function = dc_kick_wtd_timer_func;
	hrtimer_init(&l_di->anomaly_det_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	l_di->anomaly_det_timer.function = dc_anomaly_det_timer_func;
}

void dc_reset_work_para(void)
{
	g_last_kick_wtd_work_time.tv_sec = 0;
	g_last_kick_wtd_timer_func_time.tv_sec = 0;
}
