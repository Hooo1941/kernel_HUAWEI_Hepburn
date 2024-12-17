// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_control.c
 *
 * control module for direct charge
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

#include <chipset_common/hwpower/battery/battery_charge_balance.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_test.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_pwr_ctrl.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>
#include <chipset_common/hwusb/hw_usb.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/adapter/adapter_detect.h>

#define HWLOG_TAG direct_charge_control
HWLOG_REGIST();

#define DC_ADAPTOR_ISET_STEP    1000
#define IBAT_ABNORMAL_MAX_TIME  10
#define IBAT_ABNORMAL_VOL_TH    100

struct dc_ctrl_dev {
	u32 first_cc_stage_timer_in_min;
};

struct dc_ctrl_base_data {
	time64_t direct_charge_start_time;
	unsigned long first_cc_stage_timeout;
};

static struct dc_ctrl_dev *g_dc_ctrl_di[DC_MODE_TOTAL];
static struct dc_ctrl_base_data g_dc_ctrl_base_data;

static struct dc_ctrl_dev *dc_ctrl_get_di(int mode)
{
	int idx = dc_comm_get_mode_idx(mode);
	if (idx < 0)
		return NULL;

	return g_dc_ctrl_di[idx];
}

static bool dc_ctrl_is_voltage_abnormal(int mode)
{
	int vusb = 0;
	struct adapter_source_info source_info = { 0 };

	(void)dc_get_adapter_source_info(BIT(ADAPTER_OUTPUT_VOLT), &source_info);
	(void)dcm_get_ic_vusb(mode, CHARGE_IC_MAIN, &vusb);

	return (source_info.output_volt - vusb < IBAT_ABNORMAL_VOL_TH);
}

static bool dc_ctrl_is_current_abnormal(int ibat_abnormal_th)
{
	struct adapter_source_info source_info = { 0 };

	(void)dc_get_adapter_source_info(BIT(ADAPTER_OUTPUT_CURR), &source_info);
	return (source_info.output_curr < ibat_abnormal_th);
}

static bool dc_ctrl_ibat_abnormal_check(int mode, int ibat_abnormal_th)
{
	if (dc_adpt_skip_security_check(mode))
		return (dc_ctrl_is_voltage_abnormal(mode) &&
			dc_ctrl_is_current_abnormal(ibat_abnormal_th));
	else
		return dc_ctrl_is_current_abnormal(ibat_abnormal_th);
}

bool dc_ctrl_is_ibat_abnormal(int mode, int ibat_abnormal_th, int *cnt)
{
	if (!cnt)
		return false;

	if (dc_ctrl_ibat_abnormal_check(mode, ibat_abnormal_th))
		(*cnt)++;
	else
		*cnt = 0;

	if (*cnt > IBAT_ABNORMAL_MAX_TIME) {
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_IBAT_ABNORMAL, NULL);
		return true;
	}

	return false;
}

static void dc_cccv_special_regulation(struct direct_charge_device *di, int ibat)
{
	int gap, sign;
	int iadp_set = 0;
	int vstep = direct_charge_get_vstep(di->local_mode);

	if ((di->cur_stage % 2) && (di->vbat > di->cur_vbat_th)) {
		di->adaptor_vset += di->dc_volt_ratio * (di->cur_vbat_th - di->vbat);
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	if (ibat > di->cur_ibat_th_high) {
		di->adaptor_vset -= vstep;
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	gap = power_min_positive(dc_adpt_get_inherent_info(ADAP_MAX_CUR),
		di->cur_ibat_th_high / di->dc_volt_ratio) - di->adaptor_iset;
	sign = gap > 0 ? 1 : -1;
	gap = abs(gap);
	if (gap != 0) {
		di->adaptor_iset += sign * (gap > DC_ADAPTOR_ISET_STEP ? DC_ADAPTOR_ISET_STEP : gap);
		dc_set_adapter_current(di->local_mode, di->adaptor_iset);
		return;
	}

	/* adaptor cc protection */
	if (di->vadapt_diff_th && (di->adaptor_vset - di->vadapt >= di->vadapt_diff_th)) {
		hwlog_info("the difference of vadapt and vadapt_set exceed %d,do nothing\n", di->vadapt_diff_th);
		return;
	}

	dc_get_adapter_current_set(&iadp_set);
	if ((ibat < di->cur_ibat_th_high) && (ibat < iadp_set * di->dc_volt_ratio)) {
		di->adaptor_vset += vstep;
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	hwlog_info("do nothing\n");
}

static void dc_cccv_regulation_by_ibat(struct direct_charge_device *di, int ibat)
{
	int iadp_set = 0;
	int max_adap_cur;
	int vstep = direct_charge_get_vstep(di->local_mode);
	int delta_err = dc_adpt_update_delta_err(di->local_mode, di->delta_err);

	if ((di->cur_stage % 2) && (di->vbat > di->cur_vbat_th)) {
		di->adaptor_vset += di->dc_volt_ratio * (di->cur_vbat_th - di->vbat);
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	if ((ibat > di->cur_ibat_th_high) ||
		((di->ls_ibus * di->dc_volt_ratio) > (di->cur_ibat_th_high + delta_err))) {
		di->adaptor_vset -= vstep;
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	max_adap_cur = dc_adpt_get_inherent_info(ADAP_MAX_CUR);
	if (di->adaptor_iset < (max_adap_cur - DC_ADAPTOR_ISET_STEP)) {
		di->adaptor_iset += DC_ADAPTOR_ISET_STEP;
		dc_set_adapter_current(di->local_mode, di->adaptor_iset);
		return;
	}
	if (di->adaptor_iset < max_adap_cur) {
		di->adaptor_iset = max_adap_cur;
		dc_set_adapter_current(di->local_mode, di->adaptor_iset);
		return;
	}

	/* adaptor cc protection */
	if (di->vadapt_diff_th && (di->adaptor_vset - di->vadapt >= di->vadapt_diff_th)) {
		hwlog_info("the difference of vadapt and vadapt_set exceed %d,do nothing\n", di->vadapt_diff_th);
		return;
	}
	dc_get_adapter_current_set(&iadp_set);
	if (di->iadapt >= iadp_set) {
		hwlog_info("iadapt greater than iadapt_set,do nothing\n");
		return;
	}

	if ((ibat < di->cur_ibat_th_high) &&
		(di->ls_ibus * di->dc_volt_ratio - di->cur_ibat_th_high < delta_err)) {
		di->adaptor_vset += vstep;
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	hwlog_info("do nothing\n");
}

static void dc_cccv_regulation_by_ibus(struct direct_charge_device *di, int ibat)
{
	int vstep = direct_charge_get_vstep(di->local_mode);
	int delta_err = dc_adpt_update_delta_err(di->local_mode, di->delta_err);

	if ((di->cur_stage % 2) && (di->vbat > di->cur_vbat_th)) {
		di->adaptor_vset += di->dc_volt_ratio * (di->cur_vbat_th - di->vbat);
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	if ((di->iadapt > di->cur_ibat_th_high / di->dc_volt_ratio) ||
		(ibat > di->cur_ibat_th_high)) {
		di->adaptor_vset -= vstep;
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	if ((adapter_detect_get_runtime_protocol_type() == BIT(ADAPTER_PROTOCOL_UFCS)) &&
		(di->adaptor_iset > di->cur_ibat_th_high / di->dc_volt_ratio)) {
		di->adaptor_iset = di->cur_ibat_th_high / di->dc_volt_ratio;
		dc_set_adapter_current(di->local_mode, di->adaptor_iset);
		hwlog_info("turn down adaptor curr\n");
		return;
	}

	if (ibat > di->cur_ibat_th_high - delta_err) {
		hwlog_info("do nothing\n");
		return;
	}

	if (di->adaptor_iset < (di->cur_ibat_th_high - DC_ADAPTOR_ISET_STEP) / di->dc_volt_ratio) {
		di->adaptor_iset += DC_ADAPTOR_ISET_STEP / di->dc_volt_ratio;
		dc_set_adapter_current(di->local_mode, di->adaptor_iset);
		return;
	}
	if (di->adaptor_iset < di->cur_ibat_th_high / di->dc_volt_ratio) {
		di->adaptor_iset = di->cur_ibat_th_high / di->dc_volt_ratio;
		dc_set_adapter_current(di->local_mode, di->adaptor_iset);
		return;
	}

	if (di->iadapt < (di->cur_ibat_th_high - delta_err) / di->dc_volt_ratio) {
		di->adaptor_vset += vstep;
		dc_set_adapter_voltage(di->local_mode, &di->adaptor_vset);
		return;
	}

	return;
}

static void dc_update_regulation_data(struct direct_charge_device *di, int ibat)
{
	unsigned int flag;
	int iadp_set = 0;
	struct adapter_source_info data = { 0 };

	flag = BIT(ADAPTER_OUTPUT_VOLT) | BIT(ADAPTER_OUTPUT_CURR) | BIT(ADAPTER_DEV_TEMP);
	dc_get_adapter_source_info(flag, &data);
	di->iadapt = data.output_curr;
	di->vadapt = data.output_volt;
	di->tadapt = data.dev_temp;
	dc_get_adapter_current_set(&iadp_set);
	dc_get_device_ibus(&di->ls_ibus);
	direct_charge_get_device_vbus(&di->ls_vbus);
	direct_charge_get_device_temp(&di->tls);

	hwlog_info("cur_stage[%d]: vbat=%d vbat_th=%d\t"
		"ibat=%d ibat_th_high=%d ibat_th_low=%d\t"
		"vadp=%d iadp=%d iadp_set=%d\t"
		"ls_vbus=%d ls_ibus=%d tadp=%d tls=%d\n",
		di->cur_stage, di->vbat, di->cur_vbat_th,
		ibat, di->cur_ibat_th_high, di->cur_ibat_th_low,
		di->vadapt, di->iadapt, iadp_set,
		di->ls_vbus, di->ls_ibus, di->tadapt, di->tls);
}

static int dc_check_device_and_adapter_temp(struct direct_charge_device *di)
{
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	int max_tadap;

	/* check temperature with device */
	if (di->tls > di->max_tls) {
		snprintf(tmp_buf, sizeof(tmp_buf),
			"tls=%d > %d, stop direct charge\n",
			di->tls, di->max_tls);
		dc_fill_eh_buf(di->dsm_buff,
			sizeof(di->dsm_buff), DC_EH_TLS_ABNORMAL, tmp_buf);
		return -EPERM;
	}

	max_tadap = dc_adpt_get_mode_info(di->local_mode, ADAP_MAX_TEMP);
	/* check temperature with adapter */
	if (di->tadapt > max_tadap) {
		snprintf(tmp_buf, sizeof(tmp_buf),
			"tadp=%d > %d, stop direct charge\n", di->tadapt, max_tadap);
		dc_fill_eh_buf(di->dsm_buff,
			sizeof(di->dsm_buff), DC_EH_TADP_ABNORMAL, tmp_buf);

		di->adp_otp_cnt++;
		if (di->adp_otp_cnt >= ADP_OTP_CNT) {
			di->adp_otp_cnt = ADP_OTP_CNT;
			power_dsm_report_dmd(POWER_DSM_BATTERY,
				POWER_DSM_DIRECT_CHARGE_ADAPTER_OTP, tmp_buf);
		}
		return -EPERM;
	}
	return 0;
}

void dc_charge_regulation(void)
{
	int ibat = 0;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || (l_di->dc_volt_ratio == 0))
		return;

	direct_charge_get_bat_current(&ibat);
	dc_update_regulation_data(l_di, ibat);

	/* secondary resistance check */
	if (dc_get_cable_mode_info(l_di->local_mode, SEC_RES_CHECK_EN) &&
		(dc_get_cable_inherent_info(DC_CABLE_DETECT_OK) == CABLE_DETECT_OK) &&
		!dc_get_cable_mode_info(l_di->local_mode, SEC_RES_CHECK_OK))
		dc_calculate_second_path_resistance(l_di->local_mode, l_di->ls_ibus);

	if (dc_check_device_and_adapter_temp(l_di)) {
		dc_set_stop_charging_flag(true);
		return;
	}

	/* keep communication with device within 1 second */
	if (direct_charge_get_device_close_status())
		return;

	/* If neither ibus nor iadap can be collected, special regulation is used */
	if (dc_adpt_skip_security_check(l_di->local_mode)) {
		dc_cccv_special_regulation(l_di, ibat);
		return;
	}

	if (l_di->regulation_by_ibat)
		dc_cccv_regulation_by_ibat(l_di, ibat);
	else
		dc_cccv_regulation_by_ibus(l_di, ibat);
	return;
}

void dc_preparation_before_switch_to_singlepath(int working_mode, int ratio, int vdelt)
{
	int ibus = 0;
	int vbat = 0;
	struct adapter_source_info info = { 0 };
	int ibat_th = 0;
	int retry = 30; /* 30 : max retry times */

	if (!ratio)
		return;

	dcm_get_ic_max_ibat(working_mode, CHARGE_MULTI_IC, &ibat_th);
	if (!ibat_th)
		ibat_th = DC_SINGLEIC_CURRENT_LIMIT;

	dc_get_device_ibus(&ibus);
	direct_charge_get_bat_sys_voltage(&vbat);
	if (ibus > ibat_th / ratio) {
		if (dc_get_adapter_source_info(BIT(ADAPTER_OUTPUT_VOLT), &info))
			return;
		do {
			info.output_volt -= 200; /* voltage decreases by 200mv each time */
			dc_set_adapter_voltage(working_mode, &info.output_volt);
			power_usleep(DT_USLEEP_5MS);
			dc_get_device_ibus(&ibus);
			hwlog_info("[%d] set vadp=%d, ibus=%d\n", retry, info.output_volt, ibus);
			retry--;
		} while ((ibus >= ibat_th / ratio) && (retry != 0) && (info.output_volt > (vbat * ratio + vdelt)));
	}
}

void dc_preparation_before_stop(void)
{
	struct adapter_source_info source_info = { 0 };
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	/*
	 * fix a sc adapter hardware issue:
	 * adapter has not output voltage when direct charger charge done
	 * we will set the adapter voltage to 5500mv
	 * when adapter voltage up to 7500mv on charge done stage
	 */
	if (l_di->reset_adap_volt_enabled && (l_di->dc_stage == DC_STAGE_CHARGE_DONE) &&
		(l_di->adaptor_vset > 7500)) {
		l_di->adaptor_vset = 5500;
		dc_set_adapter_voltage(l_di->working_mode, &l_di->adaptor_vset);
		power_msleep(DT_MSLEEP_200MS, 0, NULL); /* delay 200ms at least */
		dc_get_adapter_source_info(BIT(ADAPTER_OUTPUT_VOLT), &source_info);
		hwlog_info("set adaptor_vset=%d when charge done, vadp=%d\n", l_di->adaptor_vset,
			source_info.output_volt);
		return;
	}

	if (l_di->cur_mode == CHARGE_MULTI_IC)
		dc_preparation_before_switch_to_singlepath(l_di->working_mode,
			l_di->dc_volt_ratio, l_di->init_delt_vset);
}

void dc_ctrl_fill_eh_buf_and_set_err_flag_true(int type, const char *str)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	dc_fill_eh_buf(l_di->dsm_buff, sizeof(l_di->dsm_buff), type, str);
	dc_set_stop_charging_flag(true);
}

static void dc_ctrl_set_first_stage_timeout(int mode)
{
	int msecs;
	struct dc_ctrl_dev *di = dc_ctrl_get_di(mode);
	if (!di)
		return;

	hwlog_info("first_cc_stage time=%u\n", di->first_cc_stage_timer_in_min);
	if (di->first_cc_stage_timer_in_min) {
		/* 8a maximum 5 min, ap will not suspend, so use jiffies */
		msecs = di->first_cc_stage_timer_in_min * 60 * MSEC_PER_SEC;
		g_dc_ctrl_base_data.first_cc_stage_timeout = jiffies + msecs_to_jiffies(msecs);
	}
}

void dc_ctrl_revise_stage(int *cur_stage)
{
	if (g_dc_ctrl_base_data.first_cc_stage_timeout > 0) {
		/* 0: cc stage, 1: cv stage, 2: max stage */
		if ((*cur_stage == 0) || (*cur_stage == 1)) {
			if (time_after(jiffies, g_dc_ctrl_base_data.first_cc_stage_timeout)) {
				hwlog_info("first_cc_stage_timeout=%u\n", g_dc_ctrl_base_data.first_cc_stage_timeout);
				*cur_stage = 2;
			}
		}
	}
}

static void dc_ctrl_select_comp_param(struct direct_charge_device *di)
{
	const char *ic_name = NULL;

	/* get vbat compensation of different ic */
	ic_name = dcm_get_ic_name(di->working_mode, CHARGE_IC_AUX);
	dc_get_vbat_comp_para(ic_name, &di->comp_para);
	memcpy(di->multi_ic_check_info.vbat_comp, di->comp_para.vbat_comp,
		sizeof(di->comp_para.vbat_comp));
}

static void dc_ctrl_update_charging_volt_param(
	struct direct_charge_volt_para *para, int stage_size)
{
	int i, bat_max_cur, bat_max_volt;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || !para)
		return;

	/* 1000ma : current threshold, 20mv : voltage threshold */
	bat_max_cur = para[0].cur_th_high + 1000;
	bat_max_volt = para[stage_size - 1].vol_th + 20;
	if (l_di->volt_para_test_flag) {
		for (i = 0; i < l_di->stage_size; i++) {
			if (l_di->volt_para[0].cur_th_high > bat_max_cur)
				l_di->volt_para[0].cur_th_high = bat_max_cur;
			if (l_di->volt_para[i].vol_th > bat_max_volt)
				l_di->volt_para[i].vol_th = bat_max_volt;
			hwlog_info("%4d %5d %5d\n", l_di->volt_para[i].vol_th,
				l_di->volt_para[i].cur_th_high, l_di->volt_para[i].cur_th_low);
		}
	} else {
		l_di->stage_size = stage_size;
		memcpy(l_di->volt_para, para, sizeof(l_di->volt_para));
	}
	memcpy(l_di->orig_volt_para, l_di->volt_para, sizeof(l_di->orig_volt_para));
}

static void dc_ctrl_select_charging_volt_param(struct direct_charge_device *di)
{
	int i;
	const char *brand = POWER_SUPPLY_DEFAULT_BRAND;
	int tbat;
	int group_cur = 0;

	(void)power_supply_get_str_property_value(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_BRAND, &brand);
	tbat = di->bat_temp_before_charging;

	hwlog_info("stage_group_size=%d, bat_brand=%s, bat_temp=%d\n",
		di->stage_group_size, brand, tbat);

	for (i = 0; i < di->stage_group_size; i++) {
		if (!di->orig_volt_para_p[i].bat_info.parse_ok)
			continue;
		if (!strstr(brand, di->orig_volt_para_p[i].bat_info.batid))
			continue;
		if ((tbat >= di->orig_volt_para_p[i].bat_info.temp_high) ||
			(tbat < di->orig_volt_para_p[i].bat_info.temp_low))
			continue;

		group_cur = i;
		hwlog_info("[%d]: bat_id=%s, temp_low=%d, temp_high=%d\n", i,
			di->orig_volt_para_p[group_cur].bat_info.batid,
			di->orig_volt_para_p[group_cur].bat_info.temp_low,
			di->orig_volt_para_p[group_cur].bat_info.temp_high);
		break;
	}

	dc_ctrl_update_charging_volt_param(di->orig_volt_para_p[group_cur].volt_info,
		di->orig_volt_para_p[group_cur].stage_size);
}

static void dc_ctrl_init_priority_inversion_param(
	struct direct_charge_device *di)
{
	di->pri_inversion = false;
}

static void dc_ctrl_set_start_time(void)
{
	time64_t cur_time = power_get_monotonic_boottime();

	g_dc_ctrl_base_data.direct_charge_start_time = cur_time;
}

time64_t dc_ctrl_get_start_time(void)
{
	return g_dc_ctrl_base_data.direct_charge_start_time;
}

int dc_ctrl_start_charging(int mode)
{
	int t;
	struct direct_charge_device *l_di = direct_charge_get_di();
	if (!l_di)
		return -EPERM;

	hwlog_info("start_charging\n");

	/* clear charge curr info */
	memset(&l_di->curr_info, 0, sizeof(l_di->curr_info));
	bat_chg_balance_init_rechg_info();

	power_wakeup_lock(l_di->charging_lock, false);

	charge_send_uevent(VCHRG_START_AC_CHARGING_EVENT);

	dc_ctrl_set_start_time();
	dc_ctrl_set_first_stage_timeout(mode);

	dc_adpt_init_curr(l_di->local_mode, l_di->adaptor_vset, l_di->dc_volt_ratio);
	dc_ctrl_select_comp_param(l_di);
	dc_ctrl_select_charging_volt_param(l_di);

	direct_charge_soh_policy();
	direct_charge_select_charging_stage();
	direct_charge_select_charging_param();

	if (dcm_config_ic_watchdog(l_di->working_mode, l_di->cur_mode, l_di->wtd_timeout)) {
		multi_ic_check_set_ic_error_flag(l_di->cur_mode, &l_di->multi_ic_mode_para);
		return -EINVAL;
	}

	dc_test_set_adapter_test_result(l_di->working_mode, AT_PROTOCOL_FINISH_SUCC);
	direct_charge_set_stage_status(DC_STAGE_CHARGING);
	dc_ctrl_init_priority_inversion_param(l_di);

	t = l_di->charge_control_interval;
	hrtimer_start(&l_di->control_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC), HRTIMER_MODE_REL);

	t = l_di->threshold_caculation_interval;
	hrtimer_start(&l_di->calc_thld_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC), HRTIMER_MODE_REL);

	t = l_di->kick_wtd_time;
	hrtimer_start(&l_di->kick_wtd_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC), HRTIMER_MODE_REL);

	t = DC_ANOMALY_DET_INTERVAL;
	hrtimer_start(&l_di->anomaly_det_timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC), HRTIMER_MODE_REL);

	return 0;
}

static void dc_ctrl_parse_para(struct device_node *np, struct dc_ctrl_dev *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"first_cc_stage_timer_in_min", &di->first_cc_stage_timer_in_min, 0);
}

void dc_ctrl_init(struct device *dev, int mode)
{
	struct dc_ctrl_dev *di = NULL;
	int idx = dc_comm_get_mode_idx(mode);
	if (idx < 0)
		return;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	hwlog_info("dc_ctrl_init mode %d\n", mode);

	g_dc_ctrl_di[idx] = di;
	dc_ctrl_parse_para(dev->of_node, g_dc_ctrl_di[idx]);
}

void dc_ctrl_remove(struct device *dev, int mode)
{
	struct dc_ctrl_dev *di = dc_ctrl_get_di(mode);
	if (!di)
		return;

	hwlog_info("dc_ctrl_remove mode %d\n", mode);
	kfree(di);
	di = NULL;
}
