/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: interface for coul module
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

#ifdef CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY
#include <chipset_common/hwpower/battery/battery_type_identify.h>
#endif
#include <platform_include/basicplatform/linux/power/platform/coul/coul_private_interface.h>
#include "coul_nv.h"
#include "coul_temp.h"
#include "coul_interface.h"

#define SOC_VARY_LOW                    10
#define SOC_VARY_HIGH                   90
#define CHARGING_BEGIN_SOC_UPPER        80
#define CURRENT_FULL_SOC_UPPER          90
#define FCC_DEBOUNCE_LOWER              85
#define FCC_DEBOUNCE_UPPER              115
#define TEMP_TENTH_DEGREE               10
#define DEFAULT_SOC_MONITOR_TEMP_MIN    10
#define DEFAULT_SOC_MONITOR_TEMP_MAX    45
#define MIN_PER_HOUR                    60
#define CAPACITY_MIN                    0
#define CAPACITY_CRITICAL               5
#define CAPACITY_LOW                    15
#define CAPACITY_HIGH                   95
#define CAPACITY_FULL                   100

#define TEMP_TOO_HOT            60
#define TEMP_TOO_COLD           (-20)

static int coul_get_low_temp_opt(void *data)
{
	return 0;
}

/* Return: 0: Unknown, 1:Good, 2:Overheat, 3:Dead, 4:Over voltage, 5:Unspecified failure, 6:Cold */
static int coul_get_battery_health(void *data)
{
	int status, temp;
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	if (!coul_core_is_battery_exist(di))
		return 0;

	status = POWER_SUPPLY_HEALTH_GOOD;
	temp = di->batt_temp / TENTH;
	if (di->batt_under_voltage_flag)
		status = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
	else if (temp < TEMP_TOO_COLD)
		status = POWER_SUPPLY_HEALTH_COLD;
	else if (temp > TEMP_TOO_HOT)
		status = POWER_SUPPLY_HEALTH_OVERHEAT;

	coul_core_debug("%s: batt_temp %d, status %d\n", __func__, di->batt_temp, status);
	return status;
}

/*
 * get voltage on ID pin by HKADC
 * should be modified to static after del battery_plug_in
 */
static void coul_get_battery_id_voltage_real(struct smartstar_coul_device *di)
{
	int volt;

	if (di == NULL) {
		coul_core_err("%s: di is null\n", __func__);
		return;
	}

#ifdef CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY
	bat_type_apply_mode(BAT_ID_VOLTAGE);
#endif
	volt = lpm_adc_get_adc(di->adc_batt_id);
#ifdef CONFIG_HUAWEI_BATTERY_TYPE_IDENTIFY
	bat_type_release_mode(false);
#endif
	/* volt negative means get adc fail */
	if (volt < 0) {
		coul_core_err("%s: HKADC get battery id fail\n", __func__);
		volt = 0;
	}
	di->batt_id_vol = (unsigned int)volt;
	coul_core_info("batt %d, get battery id voltage is %u mv\n",
		di->batt_index, di->batt_id_vol);
}

static int coul_get_battery_id_vol(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;

	coul_get_battery_id_voltage_real(di);
	return di->batt_id_vol;
}

 /* check wheather coul is ready 1: OK 0:not ready */
static int coul_is_ready(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di != NULL)
		return 1;
	return 0;
}

/* check whether battery uah reach threshold 0:not, 4: lower than warning_lev, 8: lower than Low_Lev */
static int coul_is_battery_reach_threshold(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_info("NULL point in %s\n", __func__);
		return -1;
	}

	if (!coul_core_is_battery_exist(di))
		return 0;

	if (di->batt_soc > BATTERY_CC_WARNING_LEV)
		return 0;
	if (di->batt_soc >= (int)di->wakelock_low_batt_soc)
		return BQ_FLAG_SOC1;
	return BQ_FLAG_LOCK;
}

static char *coul_get_battery_brand(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return "default";

	if (di->batt_data->id_status == BAT_ID_VALID)
		return di->batt_data->batt_brand;

	return "default";
}

/* avg current_ma in 200 seconds */
static int coul_get_battery_current_avg_ma(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	return di->last_iavg_ma;
}

static int coul_get_battery_voltage_uv(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;
	int ibat_ua = 0;
	int vbat_uv = 0;

	if (di == NULL) {
		coul_core_info("NULL point in %s\n", __func__);
		return -1;
	}

	coul_core_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);

	return vbat_uv;
}

/* tte: time to empty value in min or -1----charging */
static int coul_get_battery_tte(void *data)
{
	int cc, cur;
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;

	if (!coul_core_is_battery_exist(di))
		return 0;

	cur = di->coul_dev_ops->get_battery_current_ua(di->batt_index);
	/* charging now */
	if (cur <= 0)
		return -1;

	cc = di->batt_ruc - di->batt_uuc;
	if (cc <= 0)
		return -1;

	return cc * MIN_PER_HOUR / cur;
}

/* time to full value in min or -1----discharging */
static int coul_get_battery_ttf(void *data)
{
	int cc, cur;
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;

	if (!coul_core_is_battery_exist(di))
		return 0;

	cur = di->coul_dev_ops->get_battery_current_ua(di->batt_index);
	/* discharging now */
	if (cur >= 0)
		return -1;

	cc = di->batt_fcc - di->batt_ruc;
	/* cur is < 0 */
	return cc * MIN_PER_HOUR / (-cur);
}

static int coul_get_battery_technology(void *data)
{
	/* Default technology is "Li-poly" */
	return POWER_SUPPLY_TECHNOLOGY_LIPO;
}

/* 0: invalid battery, 1: successed */
static struct chrg_para_lut *coul_get_battery_charge_params(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_err("NULL point in %s\n", __func__);
		return NULL;
	}
	return di->batt_data->chrg_para;
}

static int coul_get_battery_ifull(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	return di->batt_data->ifull;
}

/* battery vbat max vol */
static int coul_get_battery_vbat_max(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	return di->batt_data->vbatt_max;
}

static int coul_get_battery_limit_fcc(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	return di->batt_limit_fcc / PERMILLAGE;
}

static int coul_battery_cycle_count(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	return di->batt_chargecycles / PERCENT;
}

static int coul_battery_fcc_design(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	return di->batt_data->fcc;
}

/* check whether coul is ok 0: success  1: fail */
static int coul_device_check(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	return di->coul_dev_ops->coul_dev_check();
}

static bool check_soc_vary(struct smartstar_coul_device *di,
	int soc_changed_abs, int last_record_soc, bool temp_stablity)
{
	if ((soc_changed_abs >= di->soc_monitor_limit) &&
		((last_record_soc > SOC_VARY_LOW) &&
		(last_record_soc < SOC_VARY_HIGH)) &&
		(temp_stablity == TRUE)) {
		coul_core_debug("%s: batt %d\n", __func__, di->batt_index);
		return TRUE;
		}
	return FALSE;
}

static int judge_soc_vary_valid(struct smartstar_coul_device *di,
	int monitor_flag, int *deta_soc)
{
	int soc_changed_abs, current_record_soc, soc_changed;
	bool temp_stablity = FALSE;
	int ret = -1; /* ret 0 data is valid, others invalid */

	/* Start up or resume, refresh record soc and return invalid data */
	if (di->soc_monitor_flag != STATUS_RUNNING) {
		di->last_record_soc = di->soc_unlimited;
		di->soc_monitor_flag = STATUS_RUNNING;
		return -1;
	}

	coul_core_debug("%s: batt %d\n", __func__, di->batt_index);
	if ((di->batt_temp > DEFAULT_SOC_MONITOR_TEMP_MIN) &&
		(di->batt_temp < DEFAULT_SOC_MONITOR_TEMP_MAX))
		temp_stablity = TRUE;
	else
		temp_stablity = FALSE;

	if (monitor_flag == 1) {
		current_record_soc = di->soc_unlimited;
		soc_changed = current_record_soc - di->last_record_soc;
		if (soc_changed < 0)
			soc_changed_abs = -soc_changed;
		else
			soc_changed_abs = soc_changed;
		di->last_record_soc = current_record_soc;
		/* if needed, report soc error */
		if (check_soc_vary(di, soc_changed_abs,
			di->last_record_soc, temp_stablity)) {
			*deta_soc = soc_changed;
			ret = 0;
			coul_core_err("batt %d, soc vary fast! soc_changed is %d\n",
				di->batt_index, soc_changed);
		}
	} else {
		if (temp_stablity == TRUE)
			ret = 0;
	}
	return ret;
}

/*
 * during wake-up, monitor the soc variety Input: monitor flag
 * 0:monitoring in one period 1:one period done
 * Return: data valid:0: data is valid( soc err happened) others: data is invalid
 */
static int coul_get_soc_vary_flag(void *data, int monitor_flag, int *deta_soc)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_info("NULL point in %s\n", __func__);
		return -1;
	}

	if (!coul_core_is_battery_exist(di)) {
		coul_core_info("battery not exist!\n");
		return -1;
	}

	return judge_soc_vary_valid(di, monitor_flag, deta_soc);
}

/* battery temperature in centigrade for charger */
static int coul_core_get_battery_temperature_for_charger(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;
	int temp;

	if (di == NULL) {
		coul_core_err("error, di is NULL, return default temp for charger\n");
		return DEFAULT_TEMP;
	}
	temp = coul_get_temperature_stably(di, USER_CHARGER);
	return (temp / TENTH);
}

static int coul_core_get_raw_battery_temperature(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;
	int temp;

	if (di == NULL) {
		coul_core_err("error, di is NULL, return default temp for charger\n");
		return DEFAULT_TEMP;
	}
	temp = coul_battery_retry_temp_permille(di, USER_COUL);
	return temp;
}

static int coul_get_battery_fifo_curr(void *data, unsigned int index)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->get_battery_cur_ua_from_fifo == NULL))
		return 0;
	return di->coul_dev_ops->get_battery_cur_ua_from_fifo(di->batt_index, (short)index);
}

static int coul_get_battery_fifo_vol(void *data, unsigned int index)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->get_battery_vol_uv_from_fifo == NULL))
		return 0;
	return di->coul_dev_ops->get_battery_vol_uv_from_fifo(di->batt_index, (short)index);
}

static int coul_get_battery_fifo_depth(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->get_fifo_depth == NULL))
		return 0;
	return di->coul_dev_ops->get_fifo_depth();
}

static int coul_get_battery_ufcapacity_tenth(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	return di->batt_soc_real;
}

static int coul_convert_regval2ua(void *data, unsigned int reg_val)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->convert_regval2ua == NULL))
		return -1;
	return di->coul_dev_ops->convert_regval2ua(di->batt_index, reg_val);
}

static int coul_convert_regval2uv(void *data, unsigned int reg_val)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->convert_regval2uv == NULL))
		return -1;
	return di->coul_dev_ops->convert_regval2uv(di->batt_index, reg_val);
}

static int coul_convert_regval2temp(void *data, unsigned int reg_val)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->convert_regval2temp == NULL))
		return -1;
	return di->coul_dev_ops->convert_regval2temp(reg_val);
}

static int coul_convert_mv2regval(void *data, int vol_mv)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->coul_dev_ops == NULL) ||
		(di->coul_dev_ops->convert_uv2regval == NULL))
		return -1;
	return (int)di->coul_dev_ops->convert_uv2regval(di->batt_index, vol_mv * PERMILLAGE);
}

static int get_batt_removed_flag(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_err("di is null found in %s\n", __func__);
		return -1;
	}
	return di->batt_removed_flag;
}

/* new battery, clear record fcc */
static void clear_record_fcc(struct smartstar_coul_device *di)
{
	int index;
	struct ss_coul_nv_info *pinfo = NULL;

	if (di == NULL) {
		coul_core_err("%s, input param NULL\n", __func__);
		return;
	}
	pinfo = &di->nv_info;
	/* clear learn fcc index check su */
	for (index = 0; index < MAX_RECORDS_CNT; index++)
		pinfo->real_fcc_record[index] = 0;
	pinfo->latest_record_index = 0;
	pinfo->fcc_check_sum_ext = 0;
	/* set save nv flag, clear nv */
	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);
}

static bool basp_check_sum(struct smartstar_coul_device *di)
{
	int i;
	int records_sum = 0;

	if (di == NULL)
		return FALSE;
	for (i = 0; i < MAX_RECORDS_CNT; i++) {
		records_sum += di->nv_info.real_fcc_record[i];
		coul_core_info(BASP_TAG"batt %d, check fcc records, %d:%dmAh\n",
			       di->batt_index, i, di->nv_info.real_fcc_record[i]);
	}
	if (records_sum != di->nv_info.fcc_check_sum_ext) {
		coul_core_info(BASP_TAG"batt %d, check learn fcc valid, records_sum=[%d],check_sum=%d\n",
			di->batt_index, records_sum, di->nv_info.fcc_check_sum_ext);
		return FALSE;
	}
	return TRUE;
}

static int coul_get_record_fcc(void *data, unsigned int size, unsigned int *records)
{
	int i;
	int records_num = 0;
	int index;
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (records == NULL) || (size != MAX_RECORDS_CNT)) {
		coul_core_err("%s null\n", __func__);
		return 0;
	}
	index = di->nv_info.latest_record_index;
	if ((index >= MAX_RECORDS_CNT) || (index < 0))
		return 0;

	if (!basp_check_sum(di)) {
		clear_record_fcc(di);
		return 0;
	}

	for (i = 0; i < MAX_RECORDS_CNT; i++) {
		if (di->nv_info.real_fcc_record[index] == 0)
			continue;
		records[i] = (unsigned int)di->nv_info.real_fcc_record[index];
		records_num++;
		index++;
		if (index >= MAX_RECORDS_CNT)
			index = 0;
	}
	return records_num;
}

static int coul_get_battery_rpcb(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_info("%s di is null\n", __func__);
		return 0;
	}
	return di->r_pcb;
}

/* be called when charge report current full */
static void coul_process_current_full(struct smartstar_coul_device *di)
{
	coul_core_info("[%s]+\n", __func__);
	if (di == NULL) {
		coul_core_err("[%s] di is null\n", __func__);
		return;
	}

	if ((di->charging_state != CHARGING_STATE_CHARGE_START) ||
		(di->charging_begin_soc / TENTH >=  CHARGING_BEGIN_SOC_UPPER) ||
		(di->batt_soc_real / TENTH <= CURRENT_FULL_SOC_UPPER)) {
		coul_core_info("[%s]batt %d, charging_state = %d,batt_soc = %d,"
			"charging_begin_soc=%d,do not update current_fcc_real!\n",
			__func__, di->batt_index, di->charging_state,
			di->batt_soc_real, di->charging_begin_soc);
		return;
	}
	di->batt_report_full_fcc_real = di->batt_ruc;
	coul_core_info("[%s] batt_report_full_fcc_real %d, "
		"batt_report_full_fcc_cal %d battrm %d, battruc %d\n",
		__func__, di->batt_report_full_fcc_real,
		di->batt_report_full_fcc_cal, di->batt_rm, di->batt_ruc);
	di->is_nv_need_save = 1;
	di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);
	coul_core_info("[%s]-\n", __func__);
}

static void charger_event_process(struct smartstar_coul_device *di,
	unsigned int event)
{
	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_CHARGING_EVENT:
		coul_core_info("batt %d, receive charge start event = 0x%x\n",
			di->batt_index, (int)event);
		/* record soc and cc value */
		di_lock();
		coul_core_charging_begin(di);
		di_unlock();
		break;
	case VCHRG_STOP_CHARGING_EVENT:
		coul_core_info("batt %d, receive charge stop event = 0x%x\n",
			di->batt_index, (int)event);
		di_lock();
		coul_core_charging_stop(di);
		di_unlock();
		break;
	case VCHRG_CURRENT_FULL_EVENT:
		coul_core_info("batt %d, receive current full event = 0x%x\n",
			di->batt_index, (int)event);
		di_lock();
		coul_process_current_full(di);
		di_unlock();
		break;
	case VCHRG_CHARGE_DONE_EVENT:
		coul_core_info("batt %d, receive charge done event = 0x%x\n",
			di->batt_index, (int)event);
		di_lock();
		coul_core_charging_done(di);
		di_unlock();
		break;
	case VCHRG_NOT_CHARGING_EVENT:
		di->charging_state = CHARGING_STATE_CHARGE_NOT_CHARGE;
		coul_core_err("batt %d, charging is stop by fault\n", di->batt_index);
		break;
	case VCHRG_POWER_SUPPLY_OVERVOLTAGE:
		di->charging_state = CHARGING_STATE_CHARGE_NOT_CHARGE;
		coul_core_err("batt %d, charging is stop by overvoltage\n", di->batt_index);
		break;
	case VCHRG_POWER_SUPPLY_WEAKSOURCE:
		di->charging_state = CHARGING_STATE_CHARGE_NOT_CHARGE;
		coul_core_err("batt %d, charging is stop by weaksource\n", di->batt_index);
		break;
	default:
		di->charging_state = CHARGING_STATE_CHARGE_NOT_CHARGE;
		coul_core_err("batt %d, unknow event %d\n", di->batt_index, (int)event);
		break;
	}
}

/* package charger_event_process, and be registered in scharger Model to get charge event */
static int coul_battery_charger_event_rcv(void *data, unsigned int evt)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_err("%s di is null\n", __func__);
		return -1;
	}
	if (!di->batt_exist) {
		coul_core_err("%s batt %d, batt_exist %d\n",
			__func__, di->batt_index, di->batt_exist);
		return 0;
	}

	if (!coul_core_is_battery_exist(di)) {
		coul_core_err("%s batt %d, batt_exist is 0\n",
			__func__, di->batt_index);
		return 0;
	}

	charger_event_process(di, evt);
	return 0;
}

/* check whether fcc is debounce 0: no  1: is debounce */
static int coul_is_fcc_debounce(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;
	int batt_fcc;
	int fcc;

	if (di == NULL)
		return FALSE;

	if (!coul_core_is_battery_exist(di))
		return FALSE;

	batt_fcc = coul_core_get_battery_fcc(di);
	fcc = coul_interpolate_fcc(di, di->batt_temp);
	if ((batt_fcc < fcc * FCC_DEBOUNCE_LOWER / PERCENT) ||
		(batt_fcc > fcc * FCC_DEBOUNCE_UPPER / PERCENT))
		coul_core_err("%s, fcc_from_temp=%d, batt_fcc=%d, ret=%d\n",
			__func__, fcc, batt_fcc, TRUE);

	return TRUE;
}

static int coul_get_ocv_by_soc(void *data, int temp, int soc)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->batt_data == NULL))
		return -EPERM;

	return coul_interpolate_ocv(di->batt_data->pc_temp_ocv_lut, temp, soc);
}

static int coul_get_soc_by_ocv(void *data, int temp, int ocv)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (di->batt_data == NULL))
		return -EPERM;

	return coul_interpolate_pc(di->batt_data->pc_temp_ocv_lut, temp, ocv);
}

static int coul_update_basp_policy(void *data, unsigned int level,
	unsigned int nondc_volt_dec)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL) {
		coul_core_err(BASP_TAG"[%s] di is NULL\n", __func__);
		return -1;
	}

	di->basp_level = level;
	di->nondc_volt_dec = nondc_volt_dec;
	di->qmax = coul_core_get_qmax(di);
	coul_core_info("batt %d, bsoh update policy level %u, nondc_volt_dec %u\n",
		di->batt_index, level, nondc_volt_dec);

	if (coul_battery_para_changed(di) < 0)
		coul_core_err("battery charge para fail\n");
	return 0;
}
static int coul_cal_uah_by_ocv(void *data, int ocv_uv, int *ocv_soc_uah)
{
	int pc;
	s64 qmax;
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if ((di == NULL) || (!ocv_soc_uah)) {
		coul_core_err("[%s] para is null\n", __func__);
		return ERROR;
	}
	qmax = coul_core_get_qmax(di);
	pc = coul_coul_interpolate_pc_high_precision(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp, ocv_uv / UVOLT_PER_MVOLT);

	coul_core_info("batt %d, qmax = %llduAh, pc = %d/100000, ocv_soc = %llduAh\n",
		di->batt_index, qmax, pc, qmax * pc / (SOC_FULL * PERMILLAGE));
	*ocv_soc_uah = (int)(qmax * pc / (SOC_FULL * PERMILLAGE));

	return ((*ocv_soc_uah > 0) ? SUCCESS : ERROR);
}

static int coul_get_charge_state(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	return di->charging_state;
}

static void iscd_coul_force_ocv_update(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return;

	coul_get_ocv_by_vol(di);
	coul_core_info("iscd force ocv update: ocv %d, ocv_temp %d\n",
		       di->batt_ocv, di->batt_ocv_temp);
}

static int coul_coul_interpolate_pc(void *data, int ocv)
{
	int pc;
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;
	pc = coul_coul_interpolate_pc_high_precision(di->batt_data->pc_temp_ocv_lut,
		di->batt_temp, ocv);
	return pc;
}

static int coul_get_last_soc(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;
	bool flag_soc_valid = false;
	short soc_temp = 0;
	int ret;

	if (di == NULL)
		return -1;

	di->coul_dev_ops->get_last_soc_flag(&flag_soc_valid);
	di->coul_dev_ops->get_last_soc(&soc_temp);
	coul_core_info("%s: flag=%d,di->batt_soc=%d,soc_temp=%d\n",
		__func__, flag_soc_valid, di->batt_soc, soc_temp);

	if (flag_soc_valid) {
		ret = soc_temp;
		coul_core_info("last_powerdown_soc=%d,flag=%d\n",
			soc_temp, flag_soc_valid);
	} else {
		ret = -1;
		coul_core_info("last_powerdown_soc invalid\n");
	}

	di->coul_dev_ops->clear_last_soc_flag();
	return ret;
}

static void coul_set_last_soc(void *data, int soc)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return;

	di->coul_dev_ops->save_last_soc((short)soc);
}

static u8 coul_get_last_ocv_level(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;

	return di->last_ocv_level;
}

static int coul_get_ocv_valid_refresh_fcc(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;

	return di->batt_ocv_valid_to_refresh_fcc;
}

static int coul_set_cali_info(void *data, struct coul_batt_cali_info *info)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return 0;

	return coul_cali_info_set(di, info);
}

static int coul_get_cali_info(void *data, struct coul_batt_cali_info *info)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return -1;

	return coul_cali_info_get(di, info);
}

static int coul_set_batt_mode(void *data, int mode)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return -1;

	if (di->coul_dev_ops && di->coul_dev_ops->set_batt_mode)
		return di->coul_dev_ops->set_batt_mode(mode);

	return -1;
}

static int coul_get_cali_adc_disable(void *data)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return -1;

	return di->cali_adc_disable;
}

static int coul_set_cali_adc_disable(void *data, int state)
{
	struct smartstar_coul_device *di = (struct smartstar_coul_device *)data;

	if (di == NULL)
		return -1;
	di->cali_adc_disable = state;

	return 0;
}

static struct coul_merge_drv_ops g_coulometer_ops = {
	.is_coul_ready = coul_is_ready,
	.dev_check = coul_device_check,
	.is_battery_exist = coul_core_is_battery_exist,
	.is_battery_reach_threshold = coul_is_battery_reach_threshold,
	.battery_brand = coul_get_battery_brand,
	.battery_id_voltage = coul_get_battery_id_vol,
	.battery_voltage = coul_core_get_battery_voltage_mv,
	.battery_voltage_uv = coul_get_battery_voltage_uv,
	.battery_current = coul_core_get_battery_current_ma,
	.fifo_avg_current = coul_core_get_fifo_avg_current_ma,
	.battery_current_avg = coul_get_battery_current_avg_ma,
	.battery_resistance = coul_core_get_battery_resistance,
	.battery_unfiltered_capacity = coul_get_battery_unfiltered_capacity,
	.battery_capacity = coul_core_get_battery_capacity,
	.battery_temperature = coul_core_get_battery_temperature,
	.battery_temperature_for_charger =
		coul_core_get_battery_temperature_for_charger,
	.battery_raw_temperature = coul_core_get_raw_battery_temperature,
	.chip_temperature = coul_core_get_chip_temp,
	.battery_rm = coul_core_get_battery_rm,
	.battery_fcc = coul_core_get_battery_fcc,
	.battery_fcc_design = coul_battery_fcc_design,
	.battery_uuc = coul_core_get_battery_uuc,
	.battery_delta_rc = coul_core_get_battery_delta_rc,
	.battery_tte = coul_get_battery_tte,
	.battery_ttf = coul_get_battery_ttf,
	.battery_health = coul_get_battery_health,
	.battery_technology = coul_get_battery_technology,
	.battery_charge_params = coul_get_battery_charge_params,
	.battery_vbat_max = coul_get_battery_vbat_max,
	.battery_ifull = coul_get_battery_ifull,
	.battery_cycle_count = coul_battery_cycle_count,
	.charger_event_rcv = coul_battery_charger_event_rcv,
	.get_battery_limit_fcc = coul_get_battery_limit_fcc,
	.coul_is_fcc_debounce = coul_is_fcc_debounce,
	.set_hltherm_flag = NULL,
	.get_hltherm_flag = NULL,
	.battery_cc = coul_core_get_battery_cc,
	.battery_cc_cache = coul_core_get_battery_cc_cache,
	.get_soc_vary_flag = coul_get_soc_vary_flag,
	.coul_low_temp_opt = coul_get_low_temp_opt,
	.battery_fifo_vol = coul_get_battery_fifo_vol,
	.battery_fifo_curr = coul_get_battery_fifo_curr,
	.battery_rpcb = coul_get_battery_rpcb,
	.battery_fifo_depth = coul_get_battery_fifo_depth,
	.battery_ufcapacity_tenth = coul_get_battery_ufcapacity_tenth,
	.convert_regval2temp = coul_convert_regval2temp,
	.convert_mv2regval = coul_convert_mv2regval,
	.convert_regval2ua = coul_convert_regval2ua,
	.convert_regval2uv = coul_convert_regval2uv,
	.cal_uah_by_ocv = coul_cal_uah_by_ocv,
	.battery_removed_before_boot = get_batt_removed_flag,
	.get_qmax = coul_core_get_battery_qmax,
	.get_ocv_by_soc = coul_get_ocv_by_soc,
	.get_soc_by_ocv = coul_get_soc_by_ocv,
	.get_ocv = coul_core_get_battery_ocv,
	.get_ocv_level = coul_get_last_ocv_level,
	.ocv_valid_refresh_fcc = coul_get_ocv_valid_refresh_fcc,
	.update_basp_policy = coul_update_basp_policy,
	.get_record_fcc = coul_get_record_fcc,
	.force_ocv = iscd_coul_force_ocv_update,
	.coul_coul_interpolate_pc = coul_coul_interpolate_pc,
	.coul_calc_work = calculate_soc_work,
	.get_charge_state = coul_get_charge_state,
	.get_last_soc = coul_get_last_soc,
	.set_last_soc = coul_set_last_soc,
	.coul_suspend = coul_core_suspend,
	.coul_resume = coul_core_resume,
	.coul_shutdown = coul_core_shutdown,
	.coul_set_cali = coul_set_cali_info,
	.coul_get_cali = coul_get_cali_info,
	.set_batt_mode = coul_set_batt_mode,
	.set_cali_adc_disable = coul_set_cali_adc_disable,
	.get_cali_adc_disable = coul_get_cali_adc_disable,
};

struct coul_merge_drv_ops *get_coul_interface_ops(void)
{
	return &g_coulometer_ops;
}
