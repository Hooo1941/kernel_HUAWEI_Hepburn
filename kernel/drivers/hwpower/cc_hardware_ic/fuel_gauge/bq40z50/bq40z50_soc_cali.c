// SPDX-License-Identifier: GPL-2.0
/*
 * bq40z50_jump_cali.c
 *
 * bq40z50 soc calibration
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

#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#ifdef CONFIG_HUAWEI_DSM
#include <log/hiview_hievent.h>
#endif
#include "bq40z50_soc_cali.h"

#define HWLOG_TAG bq40z50_soc_cali
HWLOG_REGIST();

struct bq40z50_soc_cali_info g_jump_cali_info;
static const int g_est_soc_points[] = { 1500, 1000, 500, 0 };

int bq40z50_get_soc_est(void)
{
	return g_jump_cali_info.soc_est / BQ40Z50_PERCENT;
}

static void bq40z50_set_last_soc(struct bq40z50_soc_cali_info *info)
{
	int last_soc;
	struct bq40z50_jump_cali_ops *ops = info->ops;
	void *di = info->ops->dev_data;

	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME, POWER_SUPPLY_PROP_CAPACITY,
		&last_soc, info->soc_cali / BQ40Z50_PERCENT, 1);
	hwlog_info("%s last_soc=%d", __func__, last_soc);
	if ((last_soc > BQ40Z50_SOC_FULL) || (last_soc < BQ40Z50_SOC_EMPTY))
		last_soc = info->soc_cali / BQ40Z50_PERCENT;
	ops->set_last_soc(di, last_soc);
}

static void bq40z50_update_battery_info(struct bq40z50_soc_cali_info *info,
	unsigned short soc_coul)
{
	struct bq40z50_jump_cali_ops *ops = info->ops;
	void *di = info->ops->dev_data;

	info->rm_prev = info->rm;
	info->curr = ops->get_curr(di);
	info->avg_curr = abs(ops->get_avg_curr(di));
	info->volt1 = ops->get_volt1(di);
	info->volt2 = ops->get_volt2(di);
	info->temp1 = ops->get_temp1(di);
	info->temp2 = ops->get_temp2(di);
	info->rm = ops->get_rm(di) * BQ40Z50_BATTERY_COUNT;
	info->cycle = ops->get_cycle(di);
	info->fcc = ops->get_fcc(di) * BQ40Z50_BATTERY_COUNT;
	info->soc_coul = ops->get_soc(di) * BQ40Z50_PERCENT;
	bq40z50_set_last_soc(info);

	hwlog_info("%s curr=%d, avgcurr=%d, V1=%d, V2=%d, T1=%d, T2=%d, rm=%d, cycle=%d,"
		" fcc=%d, soc_coul=%d, dc=%d, pre_rm=%d\n", __func__, info->curr, info->avg_curr,
		info->volt1, info->volt2, info->temp1, info->temp2, info->rm, info->cycle,
		info->fcc, soc_coul, info->fcc_design, info->rm_prev);
}

static int bq40z50_calculate_smooth_time(struct bq40z50_soc_cali_info *info)
{
	int time_to_empty;

	/* time_to_empty(s) = soc_est(%) * fcc(mAs) / avg_curr(mA) */
	time_to_empty = (info->soc_est / BQ40Z50_PERCENT) * info->fcc * BQ40Z50_SEC_PER_HOUR
		/ (info->avg_curr * BQ40Z50_PERCENT);

	hwlog_info("%s tte=%d\n", __func__, time_to_empty);
	/* Double rate than soc_coul decrease */
	return time_to_empty / BQ40Z50_DOUBLE;
}

#ifdef CONFIG_HUAWEI_DSM
static void bq40z50_report_battery_data(struct bq40z50_soc_cali_info *info)
{
	int ret;
	struct hiview_hievent *hi_event = NULL;

	hi_event = hiview_hievent_create(BQ40Z50_DMD_REPORT_ID);
	if (hi_event == NULL) {
		hwlog_err("%s create hievent fail\n", __func__);
		return;
	}

	ret = hiview_hievent_put_integral(hi_event,
		"soc_coul", info->soc_coul / BQ40Z50_PERCENT);
	ret += hiview_hievent_put_integral(hi_event,
		"soc_est", info->soc_est / BQ40Z50_PERCENT);
	ret += hiview_hievent_put_integral(hi_event,
		"soc_err", abs(info->soc_coul - info->soc_est) / BQ40Z50_PERCENT);
	ret += hiview_hievent_put_integral(hi_event, "fcc", info->fcc);
	ret += hiview_hievent_put_integral(hi_event, "cycle", info->cycle);
	if (ret < 0)
		hwlog_err("%s put integral fail ret_put=%d\n", __func__, ret);

	ret = hiview_hievent_report(hi_event);
	if (ret < 0)
		hwlog_err("%s report fail ret=%d\n", __func__, ret);

	hiview_hievent_destroy(hi_event);

	hwlog_info("%s soc_coul=%d soc_est=%d soc_err=%d fcc=%d cycle=%d\n",
		__func__, info->soc_coul / BQ40Z50_PERCENT, info->soc_est / BQ40Z50_PERCENT,
		abs(info->soc_coul - info->soc_est) / BQ40Z50_PERCENT, info->fcc, info->cycle);
}
#endif

static void bq40z50_discharge_adjust_soc_cali(struct bq40z50_soc_cali_info *info,
	time64_t delta_time)
{
	int smooth_time, delta_soc, prev_soc;

	if (!info)
		return;

	delta_soc = info->soc_coul - info->soc_est;
	prev_soc = info->soc_cali;

	hwlog_info("%s adjust begin, soc_coul=%d.%02d, soc_est=%d.%02d, delta_time=%d\n",
		__func__, info->soc_coul / BQ40Z50_PERCENT, info->soc_coul % BQ40Z50_PERCENT,
		info->soc_est / BQ40Z50_PERCENT, info->soc_est % BQ40Z50_PERCENT, delta_time);

	/* Display soc_coul when delta_soc < 5% and not in cali stage */
	if ((delta_soc < info->smooth_soc0) && !info->discharge_smooth_flag) {
		info->soc_cali = info->soc_coul;
		return;
	}

	/* When the avg_curr is 0, soc_cali remains unchanged. */
	if (info->avg_curr == 0)
		return;

	smooth_time = bq40z50_calculate_smooth_time(info);
	if (smooth_time == 0)
		smooth_time = BQ40Z50_ONE_SECOND;
	info->soc_cali -= delta_time * delta_soc / smooth_time;

	/* if soc_coul decrease faster than soc_cali, display the soc_coul */
	if (info->soc_cali > info->soc_coul)
		info->soc_cali = info->soc_coul;

	/* Smooth complete when soc_cali = soc_est */
	if (info->soc_cali < info->soc_est)
		info->soc_cali = info->soc_est;

#ifdef CONFIG_HUAWEI_DSM
	/* DMD report */
	if (info->report_flag && ((info->soc_coul - info->soc_est) > info->smooth_soc0)) {
		bq40z50_report_battery_data(info);
		info->report_flag = BQ40Z50_END_DMD_REPORT;
	}
#endif

	hwlog_info("%s adjust finish, pre soc_cali = %d.%02d, new soc_cali = %d.%02d\n",
		__func__, prev_soc / BQ40Z50_PERCENT, prev_soc % BQ40Z50_PERCENT,
		info->soc_cali / BQ40Z50_PERCENT, info->soc_cali % BQ40Z50_PERCENT);
}

static void bq40z50_volt_soc_tbl_cali(struct bq40z50_soc_cali_info *info,
	struct volt_soc_tbl *volt_soc_tbl)
{
	int curr1, curr2;

	if (!info->fcc_design)
		return;

	volt_soc_tbl->rate = abs(info->curr) * BQ40Z50_PERCENT / info->fcc_design;
	volt_soc_tbl->temp = power_min_positive(info->temp1, info->temp2);
	/* 0.2C rate current */
	curr1 = info->bat_volt_soc.tbl1.rate * info->fcc_design / BQ40Z50_PERCENT;
	/* 0.5C rate current */
	curr2 = info->bat_volt_soc.tbl2.rate * info->fcc_design / BQ40Z50_PERCENT;
	/* calculate the volt_soc_tbl according to current */
	volt_soc_tbl->soc15_volt = power_linear_interpolate(info->bat_volt_soc.tbl1.soc15_volt,
		curr1, info->bat_volt_soc.tbl2.soc15_volt, curr2, abs(info->curr));
	volt_soc_tbl->soc10_volt = power_linear_interpolate(info->bat_volt_soc.tbl1.soc10_volt,
		curr1, info->bat_volt_soc.tbl2.soc10_volt, curr2, abs(info->curr));
	volt_soc_tbl->soc5_volt = power_linear_interpolate(info->bat_volt_soc.tbl1.soc5_volt,
		curr1, info->bat_volt_soc.tbl2.soc5_volt, curr2, abs(info->curr));
	volt_soc_tbl->soc0_volt = power_linear_interpolate(info->bat_volt_soc.tbl1.soc0_volt,
		curr1, info->bat_volt_soc.tbl2.soc0_volt, curr2, abs(info->curr));
}

static void bq40z50_calculate_discharge_soc_est(struct bq40z50_soc_cali_info *info, int *soc_est)
{
	int temp_min, temp_max, vc_min, i;
	int soc_est1, soc_est2;
	struct volt_soc_tbl tbl;
	int *volt_soc_tbl = (int*)&tbl;

	temp_min = power_min_positive(info->temp1, info->temp2);
	temp_max = power_max_positive(info->temp1, info->temp2);
	if (temp_min < BQ40Z50_BAT_TBL_MIN_TEMP)
		memcpy(&tbl, &info->bat_volt_soc.tbl0, sizeof(struct volt_soc_tbl));
	else if (temp_max > BQ40Z50_BAT_TBL_MAX_TEMP)
		memcpy(&tbl, &info->bat_volt_soc.tbl1, sizeof(struct volt_soc_tbl));
	else
		bq40z50_volt_soc_tbl_cali(info, &tbl);

	hwlog_info("%s max_temp=%d,cur=%d,volt_soc_tbl:%d %d %d %d\n", __func__, temp_max,
		info->curr, tbl.soc15_volt, tbl.soc10_volt, tbl.soc5_volt, tbl.soc0_volt);

	vc_min = power_min_positive(info->volt1, info->volt2);
	if (vc_min > tbl.soc15_volt) {
		*soc_est = info->soc_coul;
		return;
	}

	if (vc_min <= tbl.soc0_volt) {
		*soc_est = 0;
		return;
	}

	for (i = BQ40Z50_TBL_VOLT_OFFSET + 1; i < BQ40Z50_BAT_TBL_LEN_MAX; i++) {
		if (vc_min > volt_soc_tbl[i]) {
			soc_est1 = g_est_soc_points[i - BQ40Z50_TBL_VOLT_OFFSET];
			soc_est2 = g_est_soc_points[i - 1 - BQ40Z50_TBL_VOLT_OFFSET];
			*soc_est = power_linear_interpolate(soc_est1, volt_soc_tbl[i],
				soc_est2, volt_soc_tbl[i - 1], vc_min);
			return;
		}
	}
}

static void bq40z50_soc_est_fifo_add(struct bq40z50_soc_cali_info *info, int soc_est)
{
	struct volt_soc_loop_fifo *soc_fifo = &info->soc_est_fifo;

	if (soc_fifo->len < BQ40Z50_SOC_EST_AVG_CNT)
		soc_fifo->len++;

	soc_fifo->fifo[soc_fifo->start] = soc_est;
	soc_fifo->start = (soc_fifo->start + 1) % BQ40Z50_SOC_EST_AVG_CNT;
}

static bool bq40z50_check_start_cal_soc_est(struct bq40z50_soc_cali_info *info, int volt)
{
	if (volt <= info->volt_cali_limit)
		info->cnt++;
	else
		info->cnt = 0;

	return (info->cnt >= BQ40Z50_CALI_JUDGE_MAX_NUM);
}

static void bq40z50_calculate_soc_est_fifo_avg(struct bq40z50_soc_cali_info *info)
{
	int soc_est_avg, i;
	int sum_soc_est = 0;

	for (i = 0; i < BQ40Z50_SOC_EST_AVG_CNT; i++)
		sum_soc_est += info->soc_est_fifo.fifo[i];

	soc_est_avg = sum_soc_est / BQ40Z50_SOC_EST_AVG_CNT;
	info->soc_est = power_min_positive(info->soc_est, soc_est_avg);
}

static void bq40z50_discharge_soc_jump_cali(struct bq40z50_soc_cali_info *info, time64_t delta_time)
{
	int volt;
	int soc_est = 0;

	if (info->soc_coul <= 0) {
		info->soc_cali = 0;
		return;
	}

	volt = power_min_positive(info->volt1, info->volt2);
	if (volt < BQ40Z50_BATT_VOLT_MIN)
		return;

	if (info->discharge_cali_flag == BQ40Z50_END_CAL_SOC_EST) {
		info->soc_cali = (info->discharge_smooth_flag ? info->soc_cali : info->soc_coul);
		/* soc_est cal starts when volt less than 3650mV for three consecutive times */
		if (!bq40z50_check_start_cal_soc_est(info, volt))
			return;
		info->discharge_cali_flag = BQ40Z50_START_CAL_SOC_EST;
		info->report_flag = BQ40Z50_START_DMD_REPORT;
		info->cnt = 0;
	}
	bq40z50_calculate_discharge_soc_est(info, &soc_est);
	bq40z50_soc_est_fifo_add(info, soc_est);

	if (info->soc_est_fifo.len < BQ40Z50_SOC_EST_AVG_CNT) {
		info->soc_cali = (info->discharge_smooth_flag ? info->soc_cali : info->soc_coul);
		return;
	}
	bq40z50_calculate_soc_est_fifo_avg(info);
	bq40z50_discharge_adjust_soc_cali(info, delta_time);

	info->discharge_smooth_flag = (info->soc_cali < info->soc_coul) ? 1 : 0;
}

static void bq40z50_soc_loop_fifo_init(struct bq40z50_soc_cali_info *info)
{
	info->soc_est_fifo.start = 0;
	info->soc_est_fifo.len = 0;
}

static void bq40z50_charge_soc_jump_cali(struct bq40z50_soc_cali_info *info)
{
	int prev_soc = info->soc_cali;
	int rm_delta = info->rm - info->rm_prev;
	info->rm_prev = info->rm;

	hwlog_info("%s delta_rm = %d\n", __func__, rm_delta);

	/* Init discharge cali state */
	info->cnt = 0;
	info->discharge_cali_flag = BQ40Z50_END_CAL_SOC_EST;
	info->soc_est = BQ40Z50_SOC_FULL * BQ40Z50_PERCENT;
	bq40z50_soc_loop_fifo_init(info);

	if (rm_delta < 0) {
		hwlog_info("%s do not update soc_cali\n", __func__);
		return;
	}

	if ((info->soc_cali < info->soc_coul) && info->fcc) {
		/* delta_soc_coul(%) = rm_delta(maA) * 100(%) / fcc(mhA) */
		info->soc_cali +=
			(BQ40Z50_DOUBLE * rm_delta * BQ40Z50_PERCENT * BQ40Z50_PERCENT / info->fcc);
	} else {
		info->soc_cali = info->soc_coul;
		info->discharge_smooth_flag = 0;
	}

	hwlog_info("%s soc_coul = %d, pre soc_cali = %d.%02d, new soc_cali = %d.%02d\n", __func__,
		info->soc_coul / BQ40Z50_PERCENT, prev_soc / BQ40Z50_PERCENT,
		prev_soc % BQ40Z50_PERCENT, info->soc_cali / BQ40Z50_PERCENT,
		info->soc_cali % BQ40Z50_PERCENT);
}

static void bq40z50_adjust_soc_cali(struct bq40z50_soc_cali_info *info)
{
	int delta_soc_cali;

	/* Follow soc_coul immediately, prevent battery overdischarge */
	if (info->soc_coul <= 0) {
		info->soc_cali = 0;
		info->soc_cali_prev = info->soc_cali;
		return;
	}

	/* Follow soc_coul to fill immediately */
	if (info->soc_coul >= BQ40Z50_SOC_95 * BQ40Z50_PERCENT)
		info->soc_cali = info->soc_coul;

	/* Prevent soc_cali > soc_coul */
	info->soc_cali = (info->soc_cali > info->soc_coul) ? info->soc_coul : info->soc_cali;

	delta_soc_cali = info->soc_cali - info->soc_cali_prev;
	if (info->curr < 0) {
		/* Prevent soc_cali increase when discharging */
		if (delta_soc_cali > 0)
			info->soc_cali = info->soc_cali_prev;
		/* Limit soc_cali decrease <= 1% every time */
		else if (abs(delta_soc_cali) > BQ40Z50_SOC_CHANGE_MAX * BQ40Z50_PERCENT)
			info->soc_cali = info->soc_cali_prev - 1 * BQ40Z50_PERCENT;
	} else {
		/* Prevent soc_cali decrease when charging */
		if (delta_soc_cali < 0)
			info->soc_cali = info->soc_cali_prev;
		/* Limit soc_cali increase <= 1% every time */
		else if (delta_soc_cali > BQ40Z50_SOC_CHANGE_MAX * BQ40Z50_PERCENT)
			info->soc_cali = info->soc_cali_prev + 1 * BQ40Z50_PERCENT;
	}

	info->soc_cali_prev = info->soc_cali;
}

static bool bq40z50_could_soc_cali(time64_t delta_time, struct bq40z50_soc_cali_info *info)
{
	/* The update interval must be greater than 10s */
	if (delta_time < BQ40Z50_CALI_INTERVAL)
		return false;
	/* Only single thread access is allowed */
	return atomic_dec_and_test(&info->jump_cali_lock);
}

int bq40z50_soc_jump_cali(unsigned short soc_coul)
{
	time64_t time_now;
	time64_t delta_time;
	struct bq40z50_soc_cali_info *info = &g_jump_cali_info;

	if (!info) {
		hwlog_err("%s get info pointer fail\n", __func__);
		return 0;
	}

	time_now = power_get_monotonic_boottime();
	delta_time = time_now - info->last_cali_time;
	hwlog_info("%s time_now = %lld, last_cali_time = %lld, delta_time = %lld\n", __func__,
		time_now, info->last_cali_time, delta_time);

	/* The update interval must be greater than 10s, only single thread access is allowed */
	if (!bq40z50_could_soc_cali(delta_time, info))
		return info->soc_cali / BQ40Z50_PERCENT;

	bq40z50_update_battery_info(info, soc_coul);
	if (info->curr < 0)
		bq40z50_discharge_soc_jump_cali(info, delta_time);
	else
		bq40z50_charge_soc_jump_cali(info);

	bq40z50_adjust_soc_cali(info);
	info->last_cali_time = time_now;
	atomic_set(&info->jump_cali_lock, BQ40Z50_JUMP_CAL_UNLOCK);

	return info->soc_cali / BQ40Z50_PERCENT;
}

static int check_func_ops(struct bq40z50_jump_cali_ops *ops)
{
	if (!ops || !ops->dev_data)
		return -EPERM;

	bq40z50_check_func_ops(ops, get_volt1);
	bq40z50_check_func_ops(ops, get_volt2);
	bq40z50_check_func_ops(ops, get_temp1);
	bq40z50_check_func_ops(ops, get_temp2);
	bq40z50_check_func_ops(ops, get_curr);
	bq40z50_check_func_ops(ops, get_avg_curr);
	bq40z50_check_func_ops(ops, get_soc);
	bq40z50_check_func_ops(ops, get_rm);
	bq40z50_check_func_ops(ops, get_fcc);
	bq40z50_check_func_ops(ops, get_dc);
	bq40z50_check_func_ops(ops, get_last_soc);
	bq40z50_check_func_ops(ops, set_last_soc);
	return 0;
}

static int bq40z50_parse_cali_dts_info(struct bq40z50_cali_para *dts_info,
	struct bq40z50_soc_cali_info *info)
{
	if (!info || !dts_info)
		return -EPERM;

	memcpy(&info->bat_volt_soc, &dts_info->volt_soc_tbl, sizeof(struct volt_soc_data));
	info->smooth_soc0 = dts_info->smooth_soc.soc0;
	info->smooth_soc1 = dts_info->smooth_soc.soc1;
	info->last_soc_enable = dts_info->last_soc_enable;
	info->startup_decrease_soc = dts_info->startup_decrease_soc;
	info->startup_increase_soc = dts_info->startup_increase_soc;
	info->volt_cali_limit = dts_info->volt_cali_limit;
	hwlog_info("%s smooth para:soc0=%d, soc1=%d, last_soc_enable=%d, startup_decrease_soc=%d,"
		" startup_increase_soc=%d, volt_cali_limit=%d\n", __func__, info->smooth_soc0,
		info->smooth_soc1, info->last_soc_enable, info->startup_decrease_soc,
		info->startup_increase_soc, info->volt_cali_limit);

	return 0;
}

static void bq40z50_get_startup_soc_cali(int last_soc, struct bq40z50_soc_cali_info *info)
{
	int delta_soc = last_soc - info->soc_coul / BQ40Z50_PERCENT;

	if (info->last_soc_enable && ((delta_soc >= info->startup_decrease_soc) ||
		((-delta_soc) > info->startup_increase_soc))) {
		info->soc_cali = info->soc_coul;
		return;
	}
	info->soc_cali = last_soc * BQ40Z50_PERCENT;
}

int bq40z50_soc_cali_init(struct bq40z50_jump_cali_ops *ops,
	struct bq40z50_cali_para *dts_info)
{
	void *di;
	int ret, last_soc;
	struct bq40z50_soc_cali_info *info = &g_jump_cali_info;

	if (!info)
		return -ENODEV;

	ret = check_func_ops(ops);
	if (ret) {
		hwlog_err("%s check func ops fail\n", __func__);
		return ret;
	}
	info->ops = ops;

	ret = bq40z50_parse_cali_dts_info(dts_info, info);
	if (ret) {
		hwlog_err("%s parse dts info fail\n", __func__);
		return ret;
	}

	info->last_cali_time = power_get_monotonic_boottime();
	bq40z50_soc_loop_fifo_init(info);
	di = (void*)info->ops->dev_data;
	info->fcc_design = info->ops->get_dc(di) * BQ40Z50_BATTERY_COUNT;
	info->rm = info->ops->get_rm(di) * BQ40Z50_BATTERY_COUNT;
	info->cnt = 0;
	atomic_set(&info->jump_cali_lock, BQ40Z50_JUMP_CAL_UNLOCK);
	info->discharge_cali_flag = BQ40Z50_END_CAL_SOC_EST;
	info->report_flag = BQ40Z50_END_DMD_REPORT;
	info->soc_est = BQ40Z50_SOC_FULL * BQ40Z50_PERCENT;
	info->soc_coul = info->ops->get_soc(di) * BQ40Z50_PERCENT;

	last_soc = info->ops->get_last_soc(di);
	if ((last_soc >= 0) && (last_soc <= BQ40Z50_SOC_FULL))
		bq40z50_get_startup_soc_cali(last_soc, info);
	else
		info->soc_cali = info->soc_coul;

	info->soc_cali_prev = info->soc_cali;
	info->discharge_smooth_flag = ((info->soc_cali < info->soc_coul) ? 1 : 0);
	return 0;
}