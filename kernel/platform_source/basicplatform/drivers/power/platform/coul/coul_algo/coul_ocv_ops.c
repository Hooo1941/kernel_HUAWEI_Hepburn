/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ocv operation functions for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <securec.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_private_interface.h>
#include "coul_ocv_ops.h"

static u8 g_test_delta_soc_renew_ocv;

struct ocv_judge_data {
	int sleep_time;
	int wake_time;
	int sleep_current;
	int big_current_10min;
	int temp;
	int duty_ratio;
};

static struct ocv_level_param ocv_level_para[OCV_LEVEL_MAX] = {
	{ 0, 50, 570, 9999, 9999, 0, 0, 0, 0, 1, 1 },
	{ 95, 50, 240, 30, 9999, 0, 0, 0, 0, 1, 1 },
	{ 95, 50, 120, 15, 500, 0, 0, 0, 0, 1, 1 },
	{ 98, 50, 60, 4, 250, 0, 0, 0, 0, 1, 1 },
	{ 95, 50, 60, 4, 250, 0, 3, 3600, 0, 0, 1 },
	{ 95, 50, 30, 4, 250, 0, 6, 7200, 0, 0, 1 },
	{ 95, 50, 30, 4, 500, 100, 8, 10800, 0, 0, 1 },
	{ 90, 50, 3, 4, 250, 100, 10, 18000, 0, 0, 1 }
};

void coul_judge_eco_leak_uah(struct smartstar_coul_device *di, int soc)
{
	if (di->g_eco_leak_uah == 0) {
		coul_core_info("NOT EXIT FROM ECO,SOC_NEW = %d\n", soc);
	} else {
		coul_core_info("EXIT FROM ECO,SOC_NEW = %d\n", soc);
		di->g_eco_leak_uah = 0;
	}
}

void coul_set_ocv_cali_level(struct smartstar_coul_device *di, unsigned char last_ocv_level)
{
	di->g_coul_ocv_cali_info[di->g_ocv_cali_index].cali_ocv_level = last_ocv_level;
}

void coul_update_ocv_cali_rbatt(struct smartstar_coul_device *di, int avg_c,
	int rbatt_calc)
{
	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;

	if ((!di->g_ocv_cali_rbatt_valid_flag) &&
		(di->coul_dev_ops->calculate_cc_uah(di->batt_index) / UA_PER_MA <
			CALI_RBATT_CC_MAX) &&
		(avg_c > CALI_RBATT_CURR_MIN)) {
		di->g_coul_ocv_cali_info[di->g_ocv_cali_index].cali_rbatt = rbatt_calc;
		di->g_ocv_cali_rbatt_valid_flag = 1;
	}
}

/* clear sleep and wakeup global variable */
static void coul_clear_sr_time_array(struct smartstar_coul_device *di)
{
	(void)memset_s(di->g_sr_time_sleep,
		sizeof(di->g_sr_time_sleep), 0, sizeof(di->g_sr_time_sleep));
	(void)memset_s(di->g_sr_time_wakeup,
		sizeof(di->g_sr_time_wakeup), 0, sizeof(di->g_sr_time_wakeup));

	di->g_sr_index_sleep = 0;
	di->g_sr_index_wakeup = 0;
}

void coul_record_ocv_cali_info(struct smartstar_coul_device *di)
{
	char timestamp[TIMESTAMP_STR_SIZE] = {0};
	int ret;

	if (di == NULL)
		return;

	/* clean buff */
	(void)memset_s(&di->g_coul_ocv_cali_info[di->g_ocv_cali_temp_index],
		sizeof(struct coul_ocv_cali_info),
		0, sizeof(struct coul_ocv_cali_info));

	coul_get_timestamp(timestamp, TIMESTAMP_STR_SIZE);

	ret = sprintf_s(di->g_coul_ocv_cali_info[di->g_ocv_cali_temp_index].cali_timestamp,
		TIMESTAMP_STR_SIZE, "%s", timestamp);
	if (ret < 0) {
		coul_core_err("sprintf_s failed, ret=%d\n", ret);
		return;
	}

	/* save log: ocv, ocv_temp, cc, level */
	di->g_coul_ocv_cali_info[di->g_ocv_cali_temp_index].cali_ocv_uv = di->batt_ocv;
	di->g_coul_ocv_cali_info[di->g_ocv_cali_temp_index].cali_ocv_temp = di->batt_ocv_temp;
	di->g_coul_ocv_cali_info[di->g_ocv_cali_temp_index].cali_cc_uah =
		di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	di->g_coul_ocv_cali_info[di->g_ocv_cali_temp_index].cali_ocv_level = di->last_ocv_level;

	/* Save the currently used buff number to the global variable */
	di->g_ocv_cali_index = di->g_ocv_cali_temp_index;
	di->g_ocv_cali_rbatt_valid_flag = 0;

	/* switch buffer number */
	di->g_ocv_cali_temp_index++;
	di->g_ocv_cali_temp_index = di->g_ocv_cali_temp_index % INDEX_MAX;
}

int coul_core_dsm_report_ocv_cali_info(struct smartstar_coul_device *di,
	int err_num, const char *buff)
{
#ifdef CONFIG_HUAWEI_DSM
	char timestamp[TIMESTAMP_STR_SIZE] = {0};
	char dsm_buf[DSM_BUFF_SIZE_MAX] = {0};
	int i;
	int tmp_len = 0;

	if ((di == NULL) || (buff == NULL))
		return -1;

	coul_get_timestamp(timestamp, TIMESTAMP_STR_SIZE);
	tmp_len += sprintf_s(dsm_buf, DSM_BUFF_SIZE_MAX, "%s\n", timestamp);

	/* common info: brand, cycles */
	tmp_len += sprintf_s(dsm_buf + tmp_len, DSM_BUFF_SIZE_MAX - tmp_len,
		"batteryName:%s, chargeCycles:%u\n",
		di->batt_data->batt_brand,
		di->batt_chargecycles / PERCENT);

	/* key info */
	tmp_len += sprintf_s(dsm_buf + tmp_len, DSM_BUFF_SIZE_MAX - tmp_len,
		"%s\n", buff);

	/* OCV history calibration information */
	for (i = 0; i < INDEX_MAX; i++) {
		tmp_len += sprintf_s(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "[OCV calibration]%s ",
			di->g_coul_ocv_cali_info[i].cali_timestamp);
		tmp_len += sprintf_s(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "OCV:%duV ",
			di->g_coul_ocv_cali_info[i].cali_ocv_uv);
		tmp_len += sprintf_s(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "temp:%d ",
			di->g_coul_ocv_cali_info[i].cali_ocv_temp);
		tmp_len += sprintf_s(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "CC:%duAh ",
			(int)(di->g_coul_ocv_cali_info[i].cali_cc_uah));
		tmp_len += sprintf_s(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "rbatt:%d ",
			di->g_coul_ocv_cali_info[i].cali_rbatt);
		tmp_len += sprintf_s(dsm_buf + tmp_len,
			DSM_BUFF_SIZE_MAX - tmp_len, "ocv_cali_level:%u\n",
			di->g_coul_ocv_cali_info[i].cali_ocv_level);
	}

	/* report */
	return power_dsm_report_dmd(POWER_DSM_BATTERY, err_num, dsm_buf);
#else
	return 0;
#endif
}


#ifdef CONFIG_PM
#ifdef CONFIG_DFX_DEBUG_FS
int coul_print_multi_ocv_threshold(struct smartstar_coul_device *di)
{
	int i;

	coul_core_info("%s++++", __func__);
	coul_core_info("duty_ratio_limit|sleep_time_limit|wake_time_limit|"
			"max_avg_curr_limit|temp_limit|ocv_gap_time_limit|"
			"delta_soc_limit|ocv_update_anyway|allow_fcc_update|is_enabled|\n");
	for (i = 0; i < OCV_LEVEL_MAX; i++)
		coul_core_info("%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|\n",
			di->g_ocv_level_para[i].duty_ratio_limit,
			di->g_ocv_level_para[i].sleep_time_limit,
			di->g_ocv_level_para[i].wake_time_limit,
			di->g_ocv_level_para[i].max_avg_curr_limit,
			di->g_ocv_level_para[i].temp_limit,
			di->g_ocv_level_para[i].ocv_gap_time_limit,
			di->g_ocv_level_para[i].delta_soc_limit,
			di->g_ocv_level_para[i].ocv_update_anyway,
			di->g_ocv_level_para[i].allow_fcc_update,
			di->g_ocv_level_para[i].is_enabled);
	coul_core_info("%s----", __func__);
	return 0;
}

u8 coul_get_delta_soc(void)
{
	return g_test_delta_soc_renew_ocv;
}

u8 coul_set_delta_soc(u8 delta_soc)
{
	g_test_delta_soc_renew_ocv = delta_soc;
	coul_core_info("delta_soc is set[%d]\n", delta_soc);
	return g_test_delta_soc_renew_ocv;
}
#endif
#endif

void coul_core_clear_cc_register(struct smartstar_coul_device *di)
{
	int cc_temp;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;
	cc_temp = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	di->iscd_full_update_cc += cc_temp;
	di->coul_dev_ops->clear_cc_register(di->batt_index);
}

/* clear sleep /wakeup /cl_in /cl_out time */
void coul_core_clear_coul_time(struct smartstar_coul_device *di)
{
	unsigned int time_now;
	int sr_cur_state;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_err("[SR]%s %d: di is NULL\n", __func__, __LINE__);
		return;
	}

	sr_cur_state = di->g_sr_cur_state;

	time_now = di->coul_dev_ops->get_coul_time(di->batt_index);
	if (sr_cur_state == SR_DEVICE_WAKEUP) {
		di->sr_resume_time -= (int)time_now;
		if (di->sr_resume_time > 0) {
			coul_core_err("[SR]%s %d: batt %d, sr_resume_time = %d\n",
				__func__, __LINE__, di->batt_index, di->sr_resume_time);
			di->sr_resume_time = 0;
		}
		di->sr_suspend_time = 0;
	} else {
		di->sr_resume_time = 0;
		di->sr_suspend_time = 0;
		coul_core_err("[SR]%s %d: batt %d, sr_cur_state = %d\n",
			      __func__, __LINE__, di->batt_index, sr_cur_state);
	}
	di->charging_stop_time -= (int)time_now;
	di->coul_dev_ops->clear_coul_time(di->batt_index);
}

/* get ocv by soft way when shutdown time less 20min */
static int get_calc_ocv(struct smartstar_coul_device *di)
{
	int ocv, batt_temp, chargecycles, soc_rbatt;
	int rbatt, vbatt_uv, ibatt_ua;

	batt_temp = di->batt_temp;
	chargecycles = (int)(di->batt_chargecycles / PERCENT);
	vbatt_uv = di->coul_dev_ops->convert_ocv_regval2uv(
		di->batt_index, di->boot_info.calc_ocv_reg_v);
	ibatt_ua = di->coul_dev_ops->convert_ocv_regval2ua(
		di->batt_index, di->boot_info.calc_ocv_reg_c);
	soc_rbatt = coul_calculate_pc(di, vbatt_uv, batt_temp, chargecycles);
	soc_rbatt = soc_rbatt / TENTH;
	rbatt = coul_get_rbatt(di, soc_rbatt, batt_temp);
	ocv = vbatt_uv + ibatt_ua * rbatt / PERMILLAGE;

	coul_core_info("calc ocv, v_uv=%d, i_ua=%d, soc_rbatt=%d, rbatt=%d, ocv=%d\n",
		vbatt_uv, ibatt_ua, soc_rbatt, rbatt, ocv);
	return ocv;
}

/* get first ocv from register, hardware record it during system reset */
void coul_core_get_initial_ocv(struct smartstar_coul_device *di)
{
	unsigned short ocvreg;
	int ocv_uv;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return;
	}

	(void)memcpy_s(di->g_ocv_level_para, sizeof(struct ocv_level_param) * OCV_LEVEL_MAX,
		&ocv_level_para, sizeof(struct ocv_level_param) * OCV_LEVEL_MAX);

	ocvreg = (unsigned short)di->coul_dev_ops->get_ocv(di->batt_index);
	coul_core_info("[%s] batt = %d, ocvreg = 0x%x\n", __func__,
		di->batt_index, ocvreg);
	di->batt_ocv_valid_to_refresh_fcc = 1;

	if (ocvreg == (unsigned short)FLAG_USE_CLAC_OCV) {
		coul_core_info("using calc ocv\n");
		ocv_uv = get_calc_ocv(di);
		di->batt_ocv_valid_to_refresh_fcc = 0;
		/* ocv temp saves in fastboot */
		di->coul_dev_ops->save_ocv(di->batt_index, ocv_uv, NOT_UPDATE_FCC);
		di->is_nv_need_save = 0;
		di->last_ocv_level = INVALID_SAVE_OCV_LEVEL;
		di->coul_dev_ops->save_ocv_level(di->batt_index, di->last_ocv_level);
	} else if (di->coul_dev_ops->get_use_saved_ocv_flag(di->batt_index)) {
		if (di->coul_dev_ops->get_fcc_invalid_up_flag(di->batt_index))
			di->batt_ocv_valid_to_refresh_fcc = 0;
		di->is_nv_need_save = 0;
		ocv_uv = di->coul_dev_ops->convert_ocv_regval2uv(di->batt_index, ocvreg);
		coul_core_info("using save ocv\n");
	}  else {
		if (di->coul_dev_ops->get_fcc_invalid_up_flag(di->batt_index))
			di->batt_ocv_valid_to_refresh_fcc = 0;
		ocv_uv = di->coul_dev_ops->convert_ocv_regval2uv(di->batt_index, ocvreg);
		di->is_nv_need_save = 0;
		coul_core_info("using pmu ocv from fastboot\n");
	}

	di->batt_ocv_temp = di->coul_dev_ops->get_ocv_temp(di->batt_index);
	di->batt_ocv = ocv_uv;
	di->coul_dev_ops->get_ocv_level(di->batt_index, &(di->last_ocv_level));
#ifdef CONFIG_DFX_DEBUG_FS
	coul_print_multi_ocv_threshold(di);
#endif
	coul_core_info("initial OCV = %d, OCV_temp=%d, fcc_flag= %d, ocv_level: %d\n",
		di->batt_ocv, di->batt_ocv_temp,
		di->batt_ocv_valid_to_refresh_fcc, di->last_ocv_level);
}

/* interpolate ocv value by full charge capacity when charging done */
void coul_get_ocv_by_fcc(struct smartstar_coul_device *di)
{
	int new_ocv;
	int batt_temp_degc;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return;
	}
	batt_temp_degc = di->batt_temp / TENTH;
	/* looking for ocv value in the OCV-FCC table */
	new_ocv = coul_interpolate_ocv(di->batt_data->pc_temp_ocv_lut,
		batt_temp_degc, PC_UPPER_LIMIT);
	if (new_ocv < 0) {
		coul_core_err("coul_interpolate_ocv err\n");
		return;
	} else if (new_ocv > (CAPACITY_INVALID_AREA_4500 / UVOLT_PER_MVOLT)) {
		new_ocv = CAPACITY_INVALID_AREA_4500;
	} else {
		new_ocv = new_ocv * UVOLT_PER_MVOLT;
	}

	if ((new_ocv - di->batt_ocv) > 0) {
		di->dbg_ocv_cng_1++;
		coul_core_info("full charged, and OCV change, "
			"new_ocv = %d, old_ocv = %d\n", new_ocv, di->batt_ocv);
		di->batt_ocv = new_ocv;
		di->batt_ocv_temp = di->batt_temp;
		di->coul_dev_ops->save_ocv_temp(di->batt_index, (short)di->batt_ocv_temp);
		di->batt_ocv_valid_to_refresh_fcc = 1;
		di->coul_dev_ops->save_ocv(di->batt_index, new_ocv, IS_UPDATE_FCC);
		di->last_ocv_level = OCV_LEVEL_0;
		di->coul_dev_ops->save_ocv_level(di->batt_index, di->last_ocv_level);
		coul_core_clear_cc_register(di);
		coul_core_clear_coul_time(di);
	} else {
		di->dbg_ocv_fc_failed++;
		coul_core_err("full charged, but OCV don't change, new_ocv = %d, old_ocv = %d\n",
			new_ocv, di->batt_ocv);
	}
}

/* get array max value and min value */
static void max_min_value(const int array[], u32 size, int *min, int *max)
{
	u32 i;
	int max_value, min_value;

	if ((!size) || (max == NULL) || (min == NULL)) {
			coul_core_err("%s error\n", __func__);
			return;
	}

	min_value = array[0];
	max_value = array[0];

	for (i = 1; i < size; i++) {
		if (array[i] > max_value)
			max_value = array[i];

		if (array[i] < min_value)
			min_value = array[i];
	}

	*min = min_value;
	*max = max_value;
	coul_core_debug("%s, min %d, max %d\n", __func__, max_value, min_value);
}

/*
 * check vol data from fifo valid,  Return: -1:data invalid, 0:data valid.
 * Remark: 1 all data is same ,invalid. 2 error exceeding 5mv, invalid
 */
static int check_ocv_vol_data_valid(struct smartstar_coul_device *di,
	const int vol_data[], u32 data_cnt)
{
	int max = 0;
	int min = 0;

	if (data_cnt > VOL_FIFO_MAX)
		data_cnt = VOL_FIFO_MAX;

	max_min_value(vol_data, data_cnt, &min, &max);

	/* all data is same, invalid */
	if (max == min) {
		coul_core_err("[%s] batt %d, all vol data is same, invalid\n", __func__, di->batt_index);
		return -1;
	}
	/* Error exceeding 5mv, invalid */
	if (abs(max - min) > VOL_MAX_DIFF_UV) {
		coul_core_err("[%s] fifo vol difference is more than 5 millivolts, invalid\n",
			__func__);
		return -1;
	}
	coul_core_err("[%s] batt %d, ocv data valid\n", __func__, di->batt_index);
	return 0;
}

int coul_get_ocv_vol_from_fifo(struct smartstar_coul_device *di)
{
	short i;
	int used = 0;
	int vol_fifo[VOL_FIFO_MAX] = {0};
	int current_ua, voltage_uv, totalvol, totalcur;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("[%s]di is null\n", __func__);
		return 0;
	}

	totalvol = 0;
	totalcur = 0;
	for (i = 0; i < di->coul_dev_ops->get_fifo_depth(); i++) {
		current_ua = di->coul_dev_ops->get_battery_cur_ua_from_fifo(di->batt_index, i);
		voltage_uv = di->coul_dev_ops->get_battery_vol_uv_from_fifo(di->batt_index, i);
		if ((current_ua >= CURRENT_LIMIT) ||
			(current_ua < CHARGING_CURRENT_OFFSET)) {
			di->dbg_invalid_vol++;
			coul_core_info("invalid current = %d ua\n", current_ua);
			continue;
		}
		if ((voltage_uv >= CAPACITY_INVALID_AREA_4500) ||
			(voltage_uv <= CAPACITY_INVALID_AREA_2500)) {
			di->dbg_invalid_vol++;
			coul_core_info("invalid voltage = %d uv\n", voltage_uv);
			continue;
		}
		di->dbg_valid_vol++;
		coul_core_info("batt %d, valid current = %d ua, vol = %d uv!\n",
			di->batt_index, current_ua, voltage_uv);
		totalvol += voltage_uv;
		totalcur += current_ua;
		vol_fifo[used % VOL_FIFO_MAX] = voltage_uv;
		used++;
	}

	coul_core_info("batt %d, used = %d\n", di->batt_index, used);
	if (used > MIN_VALID_VOL_DATA_NUM) {
		/* check vol inalid */
		if (check_ocv_vol_data_valid(di, vol_fifo, used) != 0)
			return 0;
		voltage_uv = totalvol / used;
		current_ua = totalcur / used;
		voltage_uv += current_ua / PERMILLAGE *
			(di->r_pcb / PERMILLAGE +
				DEFAULT_BATTERY_OHMIC_RESISTANCE);
		coul_core_debug("%s, batt %d, voltage_uv = %d\n", __func__, di->batt_index, voltage_uv);
		return voltage_uv;
	}
	return 0;
}

#ifdef CONFIG_PM
static int calc_wakeup_avg_current(struct smartstar_coul_device *di,
	int last_cc, int last_time, int curr_cc, int curr_time)
{
	int iavg_ma, delta_cc, delta_time;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("[%s]di is null\n", __func__);
		return 0;
	}

	if (di->wakeup_first_in) {
		iavg_ma = di->coul_dev_ops->get_battery_current_ua(di->batt_index) / UA_PER_MA;
		di->wakeup_first_in = 0;
	} else {
		delta_cc = curr_cc - last_cc;
		delta_time = curr_time - last_time;
		if (delta_time > 0)
			iavg_ma = (div_s64((s64)delta_cc * SEC_PER_HOUR,
				delta_time)) / UA_PER_MA;
		else
			iavg_ma = di->coul_dev_ops->get_battery_current_ua(di->batt_index) /
				UA_PER_MA;

		coul_core_info("wake_up delta_time=%ds, iavg_ma=%d\n",
			delta_time, iavg_ma);
	}

	if (abs(iavg_ma) >= WAKEUP_AVG_CUR_500MA) {
		di->g_curr2update_ocv[AVG_CURR_500MA].current_ma = abs(iavg_ma);
		di->g_curr2update_ocv[AVG_CURR_500MA].time = curr_time;
	} else if (abs(iavg_ma) >= WAKEUP_AVG_CUR_250MA) {
		di->g_curr2update_ocv[AVG_CURR_250MA].current_ma = abs(iavg_ma);
		di->g_curr2update_ocv[AVG_CURR_250MA].time = curr_time;
	}
	return iavg_ma;
}

void coul_record_wakeup_info(struct smartstar_coul_device *di, int wakeup_time)
{
	int wakeup_avg_current_ma = 0;

	if (di == NULL)
		return;

	if (wakeup_time > SR_MAX_RESUME_TIME) {
		coul_clear_sr_time_array(di);
		coul_core_info("[SR]%s %d: batt %d, wakeup_time %d > SR_MAX_RESUME_TIME %d\n",
			__func__, __LINE__, di->batt_index, wakeup_time, SR_MAX_RESUME_TIME);
		wakeup_avg_current_ma = calc_wakeup_avg_current(
			di, di->resume_cc, di->sr_resume_time,
			di->suspend_cc, di->sr_suspend_time);
	} else if (wakeup_time >= 0) {
		di->g_sr_time_wakeup[di->g_sr_index_wakeup] = wakeup_time;
		di->g_sr_index_wakeup++;
		di->g_sr_index_wakeup = di->g_sr_index_wakeup % SR_ARR_LEN;
		wakeup_avg_current_ma = calc_wakeup_avg_current(di,
			di->resume_cc, di->sr_resume_time,
			di->suspend_cc, di->sr_suspend_time);
	} else {
		coul_core_err("[SR]%s %d: batt %d, wakeup_time=%d, di->sr_suspend_time=%d, di->sr_resume_time=%d\n",
			__func__, __LINE__, di->batt_index, wakeup_time,
			di->sr_suspend_time, di->sr_resume_time);
	}
	coul_core_info("SUSPEND! batt %d, cc=%d, time=%d, wakeup_avg_current:%d\n",
		di->batt_index, di->suspend_cc, di->suspend_time, wakeup_avg_current_ma);
}

void coul_record_sleep_info(struct smartstar_coul_device *di, int sr_sleep_time)
{
	if (sr_sleep_time >= 0) {
		di->g_sr_time_sleep[di->g_sr_index_sleep] = sr_sleep_time;
		di->g_sr_index_sleep++;
		di->g_sr_index_sleep = di->g_sr_index_sleep % SR_ARR_LEN;
	} else {
		coul_core_err("[SR]%s %d: batt %d, sr_sleep_time = %d\n",
			__func__, __LINE__, di->batt_index, sr_sleep_time);
	}
}

/* calculate last SR_TOTAL_TIME seconds duty ratio */
static int sr_get_duty_ratio(struct smartstar_coul_device *di)
{
	int total_sleep_time = 0;
	int total_wakeup_time = 0;
	int last_sleep_idx = ((di->g_sr_index_sleep - 1) < 0) ?
		SR_ARR_LEN - 1 : di->g_sr_index_sleep - 1;
	int last_wakeup_idx = ((di->g_sr_index_wakeup - 1) < 0) ?
		SR_ARR_LEN - 1 : di->g_sr_index_wakeup - 1;
	int cnt = 0;
	int duty_ratio = 0;

	do {
		total_sleep_time += di->g_sr_time_sleep[last_sleep_idx];
		total_wakeup_time += di->g_sr_time_wakeup[last_wakeup_idx];

		last_sleep_idx = ((last_sleep_idx - 1) < 0) ?
			SR_ARR_LEN - 1 : last_sleep_idx - 1;
		last_wakeup_idx = ((last_wakeup_idx - 1) < 0) ?
			SR_ARR_LEN - 1 : last_wakeup_idx - 1;

		cnt++;
		if (cnt >= SR_ARR_LEN)
			break;
	} while (total_sleep_time + total_wakeup_time < SR_TOTAL_TIME);

	coul_core_debug("[SR]%s %d: batt %d, cnt %d, total_wakeup=%ds, total_sleep=%ds\n",
		__func__, __LINE__, di->batt_index, cnt, total_wakeup_time,	total_sleep_time);

	/* calculate duty ratio */
	if (total_sleep_time + total_wakeup_time >= SR_TOTAL_TIME) {
		duty_ratio = total_sleep_time * PERCENT /
			(total_sleep_time + total_wakeup_time);
		coul_core_info("[SR]%s %d: total_wakeup=%ds, total_sleep=%ds, duty_ratio=%d\n",
			__func__, __LINE__, total_wakeup_time,
			total_sleep_time, duty_ratio);
	}
	return duty_ratio;
}

static int get_big_current_10min(struct smartstar_coul_device *di, int time_now)
{
	u8 i;

	for (i = 0; i < AVG_CURR_MAX; i++) {
		if (time_now - di->g_curr2update_ocv[i].time < CURR2UPDATE_OCV_TIME)
			return di->g_curr2update_ocv[i].current_ma;
	}
	return -1;
}

static void clear_big_current_10min(struct smartstar_coul_device *di)
{
	u8 i;

	for (i = 0; i < AVG_CURR_MAX; i++) {
		di->g_curr2update_ocv[i].current_ma = 0;
		di->g_curr2update_ocv[i].time = 0;
	}
}

static int save_multi_ocv_and_level(struct smartstar_coul_device *di)
{
	int cc;

	if ((di == NULL) || (di->coul_dev_ops == NULL)) {
		coul_core_info("[%s]di is null\n", __func__);
		return 0;
	}
	di->g_eco_leak_uah = di->coul_dev_ops->calculate_eco_leak_uah(di->batt_index);
	di->coul_dev_ops->save_ocv_temp(di->batt_index, (short)di->batt_ocv_temp);
	if (di->g_ocv_level_para[di->last_ocv_level].allow_fcc_update) {
		di->batt_ocv_valid_to_refresh_fcc = 1;
		di->coul_dev_ops->save_ocv(di->batt_index, di->batt_ocv, IS_UPDATE_FCC);
	} else {
		di->batt_ocv_valid_to_refresh_fcc = 0;
		di->coul_dev_ops->save_ocv(di->batt_index, di->batt_ocv, NOT_UPDATE_FCC);
	}
	clear_big_current_10min(di);
	coul_core_clear_cc_register(di);
	coul_core_clear_coul_time(di);
	cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	cc = cc + di->g_eco_leak_uah;
	di->coul_dev_ops->save_cc_uah(di->batt_index, cc);
	di->coul_dev_ops->save_ocv_level(di->batt_index, di->last_ocv_level);
	coul_batt_notify(di, BATT_OCV_UPDATE);
	coul_core_info("awake from deep sleep, batt %d, new OCV = %d, ocv level = %u, fcc_flag = %d\n",
		       di->batt_index, di->batt_ocv, di->last_ocv_level,
		       di->batt_ocv_valid_to_refresh_fcc);
	return 0;
}

static int is_ocv_reach_renew_threshold(
	struct smartstar_coul_device *di, u8 current_ocv_level, int ocv_now,
	int time_now, int batt_temp)
{
	int pc_now;
	u8 last_ocv_level;

	coul_core_debug("%s\n", __func__);
	last_ocv_level = di->last_ocv_level;
	if (current_ocv_level <= last_ocv_level)
		return 1;
	pc_now = coul_calculate_pc(di, ocv_now, batt_temp,
		di->batt_chargecycles / PERCENT);
	if (g_test_delta_soc_renew_ocv == 0)
		di->g_delta_soc_renew_ocv = (u8)abs(pc_now - di->batt_soc_real) / TENTH;
	else
		di->g_delta_soc_renew_ocv = g_test_delta_soc_renew_ocv;
	coul_core_info("[SR]%s, batt %d (delta_soc:%d, time_now:%d)\n",
		__func__, di->batt_index, di->g_delta_soc_renew_ocv, time_now);
	if ((time_now >=
			di->g_ocv_level_para[current_ocv_level].ocv_gap_time_limit -
			di->g_ocv_level_para[last_ocv_level].ocv_gap_time_limit) &&
		(di->g_delta_soc_renew_ocv >=
			di->g_ocv_level_para[current_ocv_level].delta_soc_limit -
			di->g_ocv_level_para[last_ocv_level].delta_soc_limit))
		return 1;

	return 0;
}

static u8 judge_ocv_threshold(struct smartstar_coul_device *di, struct ocv_judge_data *data)
{
	u8 i;

	for (i = 0; i < OCV_LEVEL_MAX; i++) {
		if ((di->g_ocv_level_para[i].is_enabled == TRUE) &&
			(data->duty_ratio >=
				di->g_ocv_level_para[i].duty_ratio_limit) &&
			(data->sleep_time >=
				di->g_ocv_level_para[i].sleep_time_limit) &&
			(data->wake_time <=
				di->g_ocv_level_para[i].wake_time_limit) &&
			(data->sleep_current <=
				di->g_ocv_level_para[i].sleep_current_limit) &&
			(data->temp >= di->g_ocv_level_para[i].temp_limit) &&
			(data->big_current_10min <=
				di->g_ocv_level_para[i].max_avg_curr_limit)) {
			break;
		}
	}
	coul_core_info("[SR]%s batt %d, (LEVEL:%d): sleep_time=%ds, "
		"wake_time=%ds, sleep_current=%dma, duty_ratio=%d, "
		"temp=%d, big_current=%d\n", __func__, di->batt_index,
		i, data->sleep_time, data->wake_time, data->sleep_current,
		data->duty_ratio, data->temp, data->big_current_10min);
	return i;
}

static int calc_ocv_level(struct smartstar_coul_device *di,
	int time_now, int sleep_time, int wake_time, int sleep_current)
{
	/* judge if need update ocv */
	int ocv_now, big_current_10min, temp, duty_ratio;
	struct ocv_judge_data data = {0};
	u8 cur_level;

	coul_core_debug("%s:\n", __func__);
	temp = di->batt_temp;
	/* get big wakeup current in 10 min */
	big_current_10min = get_big_current_10min(di, time_now);
	/* get last SR_TOTAL_TIME seconds duty ratio */
	duty_ratio = sr_get_duty_ratio(di);

	data.sleep_time = sleep_time;
	data.wake_time = wake_time;
	data.sleep_current = sleep_current;
	data.big_current_10min = big_current_10min;
	data.temp = temp;
	data.duty_ratio = duty_ratio;

	cur_level = judge_ocv_threshold(di, &data);
	ocv_now = coul_get_ocv_vol_from_fifo(di);
	if (ocv_now == 0)
		return 0;

	if ((cur_level < LOW_PRECISION_OCV_LEVEL) ||
			((cur_level < OCV_LEVEL_MAX) && is_ocv_reach_renew_threshold(
				di, cur_level, ocv_now, time_now, temp))) {
		if ((!di->g_ocv_level_para[cur_level].ocv_update_anyway) &&
			(coul_is_in_capacity_dense_area(di, ocv_now) == TRUE)) {
			coul_core_info("%s: batt %d, do not update OCV %d\n",
				__func__, di->batt_index, ocv_now);
			return 0;
		}
		di->last_ocv_level = cur_level;
		di->batt_ocv = ocv_now;
		di->batt_ocv_temp = temp;
		return 1;
	}
	return 0;
}

static void get_last_time(struct smartstar_coul_device *di,
	int *last_wakeup_time, int *last_sleep_time)
{
	/* get last wakeup time */
	if ((di->g_sr_index_wakeup >= 0) && (di->g_sr_index_wakeup < SR_ARR_LEN)) {
		if (di->g_sr_index_wakeup - 1 >= 0)
			*last_wakeup_time = di->g_sr_time_wakeup[di->g_sr_index_wakeup - 1];
		else
			*last_wakeup_time = di->g_sr_time_wakeup[SR_ARR_LEN - 1];
	}
	/* get last sleep time */
	if ((di->g_sr_index_sleep >= 0) && (di->g_sr_index_sleep < SR_ARR_LEN)) {
		if (di->g_sr_index_sleep - 1 >= 0)
			*last_sleep_time = di->g_sr_time_sleep[di->g_sr_index_sleep - 1];
		else
			*last_sleep_time = di->g_sr_time_sleep[SR_ARR_LEN - 1];
	}
}

/* check whether MULTI OCV can update, Return: 1: can update, 0: can not */
static int multi_ocv_could_update(struct smartstar_coul_device *di)
{
	int sleep_cc, sleep_time, time_now;
	int sleep_current = 0;
	int last_wakeup_time = 0;
	int last_sleep_time = 0;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return 0;

	sleep_cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	coul_core_debug("[SR]%s %d: sleep_cc = %d\n", __func__, __LINE__, sleep_cc);

	/* sleep uah */
	sleep_cc = sleep_cc - di->suspend_cc;
	time_now = (int)di->coul_dev_ops->get_coul_time(di->batt_index);
	sleep_time = time_now - (int)di->suspend_time;

	coul_core_debug("[SR]%s %d: sleep_cc = %d, sleep_time %d\n", __func__, __LINE__, sleep_cc, sleep_time);

	/* get sleep current */
	/* ma = ua/1000 = uas/s/1000 = uah*3600/s/1000 = uah*18/(s*5) */
	if (sleep_time > 0) {
		sleep_current = (sleep_cc * SEC_PER_HOUR_DIV_200) /
			(sleep_time * PERMILLAGE_DIV_200);
	} else {
		coul_core_err("[SR]%s %d: sleep_time = %d\n",
			__func__, __LINE__, sleep_time);
		return 0;
	}

	get_last_time(di, &last_wakeup_time, &last_sleep_time);
	coul_core_info("[%s], batt %d, sleep_time=%ds, sleep_current=%d ma\n",
		       __func__, di->batt_index, sleep_time, sleep_current);
#ifdef CONFIG_POWER_DUBAI
	HWDUBAI_LOGE("DUBAI_TAG_BATTERY_SLEEP_CURRENT", "batt %d, current=%d",
		di->batt_index, sleep_current);
#endif
	return calc_ocv_level(di, time_now, last_sleep_time, last_wakeup_time, sleep_current);
}

/*
 * get ocv after resuming Return: 1: can update, 0: can not
 * Remark: update condition----sleep_time > 10min && sleep_current < 50mA
 */
static void get_ocv_resume(struct smartstar_coul_device *di)
{
	int cc, cc_new;

	di->g_eco_leak_uah = di->coul_dev_ops->calculate_eco_leak_uah(di->batt_index);
	coul_get_ocv_by_vol(di);
	cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	cc_new = cc + di->g_eco_leak_uah;
	di->coul_dev_ops->save_cc_uah(di->batt_index, cc_new);

	coul_core_debug("[%s]batt %d, g_eco_leak_uah %d, cc %d, cc_new %d\n", __func__, di->batt_index,
		di->g_eco_leak_uah, cc, cc_new);

	/* for test */

	cc = di->coul_dev_ops->calculate_cc_uah(di->batt_index);
	coul_core_debug("[%s]batt %d, cc_new %d\n", __func__, di->batt_index, cc);
}

void coul_update_ocv_after_resume(struct smartstar_coul_device *di)
{
	int old_ocv;

	if ((di == NULL) || (di->coul_dev_ops == NULL))
		return;
	if ((di->sr_resume_time - di->charging_stop_time >
			OCV_UPDATE_DELTA_TIME * SEC_PER_MIN) &&
		(multi_ocv_could_update(di) != 0)) {
		coul_core_info("[%s]batt %d, multi ocv\n", __func__, di->batt_index);
		coul_record_ocv_cali_info(di);
		save_multi_ocv_and_level(di);
	} else if ((di->batt_delta_rc >
			(int)di->batt_data->fcc * ABNORMAL_DELTA_SOC * TENTH) &&
			(di->charging_state != CHARGING_STATE_CHARGE_START) &&
			(di->sr_resume_time - di->charging_stop_time >
				OCV_UPDATE_DELTA_TIME * SEC_PER_MIN)) {
		old_ocv = di->batt_ocv;
		coul_core_info("[%s]batt %d, Update ocv for delta_rc %d\n",
			       __func__, di->batt_index, di->batt_delta_rc);

		get_ocv_resume(di);
		di->last_ocv_level = INVALID_SAVE_OCV_LEVEL;
		di->coul_dev_ops->save_ocv_level(di->batt_index, di->last_ocv_level);
		coul_set_ocv_cali_level(di, di->last_ocv_level);
		if (old_ocv != di->batt_ocv) {
			/* for set NOT_UPDATE_fCC Flag */
			di->coul_dev_ops->save_ocv(di->batt_index, di->batt_ocv, NOT_UPDATE_FCC);
			di->batt_ocv_valid_to_refresh_fcc = 0;
		}
	}
}

#endif

