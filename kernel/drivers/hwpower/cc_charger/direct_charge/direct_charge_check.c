/*
 * direct_charge_check.c
 *
 * direct charge check driver
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
 *
 */

#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_pmode.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_test.h>
#include <chipset_common/hwpower/hardware_monitor/btb_check.h>
#include <chipset_common/hwpower/hardware_monitor/uscp.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_pwr_ctrl.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_sysfs.h>
#include <chipset_common/hwpower/charger/direct_charge_turbo.h>
#include <chipset_common/hwpower/adapter/adapter_detect.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG direct_charge_check
HWLOG_REGIST();

#define RETYR_TIMES              3
#define MODE_NEED_RECHK          1

static bool g_dc_can_enter = true;
static bool g_dc_in_mode_check;

enum fsm_event {
	EVT_CHARGING,
	EVT_NOT_CHARGING,
};

static void direct_charge_check_set_succ_flag(int mode)
{
	switch (mode) {
	case LVC_MODE:
		dc_mmi_set_succ_flag(mode, DC_SUCC);
		return;
	case SC_MODE:
		dc_mmi_set_succ_flag(mode, DC_ERROR_START_CHARGE);
		return;
	default:
		return;
	}
}

void direct_charge_set_can_enter_status(bool status)
{
	hwlog_info("can_enter_status=%d\n", status);
	g_dc_can_enter = status;
}

bool direct_charge_get_can_enter_status(void)
{
	return g_dc_can_enter;
}

static enum fsm_event dc_get_charging_event(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return EVT_NOT_CHARGING;

	if ((l_di->sysfs_enable_charger == 0) &&
		!(l_di->sysfs_mainsc_enable_charger ^ l_di->sysfs_auxsc_enable_charger)) {
		dc_mmi_set_succ_flag(l_di->working_mode, DC_ERROR_CHARGE_DISABLED);
		hwlog_info("%d is disabled\n", l_di->working_mode);
		return EVT_NOT_CHARGING;
	}

	return EVT_CHARGING;
}

bool dc_need_exit(void)
{
	return dc_get_charging_event() != EVT_CHARGING;
}

static int direct_charge_skip_check_resistance(int mode)
{
	if (dc_adpt_skip_security_check(mode)) {
		if (dc_get_cable_inherent_info(DC_CABLE_DETECT_OK) == CABLE_DETECT_NOK)
			dc_send_icon_uevent();
		hwlog_info("iwatt lvc skip check full path resist and nonstd_cable will send icon\n");
		return 1;
	}

	if (power_cmdline_is_factory_mode() && dc_sysfs_get_ignore_full_path_res(mode)) {
		dc_sysfs_set_ignore_full_path_res(mode, false);
		hwlog_info("skip check full path resist\n");
		return 1;
	}
	return 0;
}

bool direct_charge_check_switch_sc4_to_sc_mode(void)
{
	int charge_done_status = charge_get_done_type();
	struct direct_charge_device *sc_di = NULL;
	struct direct_charge_device *sc4_di = NULL;

	sc_get_di(&sc_di);
	sc4_get_di(&sc4_di);

	if (charge_done_status == CHARGE_DONE)
		return false;

	if ((sc4_di && (sc4_di->dc_stage == DC_STAGE_CHARGE_DONE)) &&
		(sc_di && (sc_di->dc_stage != DC_STAGE_CHARGE_DONE)))
		return true;

	return false;
}

bool direct_charge_check_charge_done(void)
{
	int charge_done_status = charge_get_done_type();
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	struct direct_charge_device *sc4_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);
	sc4_get_di(&sc4_di);

	if (!lvc_di && !sc_di && !sc4_di)
		return true;

	if (charge_done_status == CHARGE_DONE) {
		hwlog_info("charge done");
		return true;
	}

	if (lvc_di && (lvc_di->dc_stage == DC_STAGE_CHARGE_DONE)) {
		hwlog_info("lvc charge done");
		return true;
	}

	if (sc_di && (sc_di->dc_stage == DC_STAGE_CHARGE_DONE)) {
		hwlog_info("sc charge done");
		return true;
	}

	if (sc4_di && (sc4_di->dc_stage == DC_STAGE_CHARGE_DONE) && (sc4_di->charge_time == 0)) {
		hwlog_info("sc4 charge done");
		return true;
	}

	return false;
}

static void direct_charge_check_recharge(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	struct direct_charge_device *sc4_di = NULL;
	int vbat = hw_battery_voltage(BAT_ID_MAX);

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);
	sc4_get_di(&sc4_di);

	if ((vbat <= 0) || (!lvc_di && !sc_di && !sc4_di))
		return;

	if (lvc_di && (lvc_di->dc_stage == DC_STAGE_CHARGE_DONE) &&
		(vbat < (int)(lvc_di->recharge_vbat_th - lvc_di->vterm_dec))) {
		lvc_di->dc_stage = DC_STAGE_DEFAULT;
		return;
	}

	if (sc_di && (sc_di->dc_stage == DC_STAGE_CHARGE_DONE) &&
		(vbat < (int)(sc_di->recharge_vbat_th - sc_di->vterm_dec))) {
		sc_di->dc_stage = DC_STAGE_DEFAULT;
		return;
	}

	if (sc4_di && (sc4_di->dc_stage == DC_STAGE_CHARGE_DONE) &&
		(vbat < (int)(sc4_di->recharge_vbat_th - sc4_di->vterm_dec))) {
		sc4_di->dc_stage = DC_STAGE_DEFAULT;
		return;
	}
}

bool direct_charge_check_port_fault(void)
{
	bool cc_vbus_short = false;
	bool cc_moisture_status = false;
	bool intruded_status = false;

	/* cc rp 3.0 can not do high voltage charge */
	cc_vbus_short = pd_dpm_check_cc_vbus_short();
	if (cc_vbus_short)
		hwlog_err("cc vbus short, can not do sc charge\n");

	cc_moisture_status = pd_dpm_get_cc_moisture_status();
	if (cc_moisture_status)
		hwlog_err("cc moisture detected\n");

	intruded_status = water_detect_get_intruded_status();
	if (intruded_status)
		hwlog_err("water detect intruded detected\n");

	return !cc_vbus_short && !cc_moisture_status && !intruded_status;
}

static void direct_charge_rework_priority_inversion(void)
{
	struct direct_charge_device *lvc_di = direct_charge_get_di_by_mode(LVC_MODE);
	struct direct_charge_device *sc_di = direct_charge_get_di_by_mode(SC_MODE);
	struct direct_charge_device *sc4_di = direct_charge_get_di_by_mode(SC4_MODE);

	if ((!lvc_di && !sc_di) || (!lvc_di && !sc4_di) || (!sc_di && !sc4_di))
		return;

	/* if any path error occur, disable pri_inversion function */
	if (lvc_di && lvc_di->dc_err_report_flag)
		lvc_di->pri_inversion = false;
	if (sc_di && sc_di->dc_err_report_flag)
		sc_di->pri_inversion = false;
	if (sc4_di && sc4_di->dc_err_report_flag)
		sc4_di->pri_inversion = false;
}

static int direct_charge_rt_set_aux_mode(struct direct_charge_device *di)
{
	if (power_cmdline_is_factory_mode() &&
		((di->working_mode == SC_MODE) || (di->working_mode == SC4_MODE)) &&
		((di->sysfs_auxsc_enable_charger == 1) &&
		(di->sysfs_mainsc_enable_charger == 0))) {
		if (di->multi_ic_mode_para.ic_error_cnt[CHARGE_IC_TYPE_AUX] < DC_MULTI_ERR_CNT_MAX) {
			di->cur_mode = CHARGE_IC_AUX;
			hwlog_info("RT aux sc charge mode test\n");
		} else {
			hwlog_err("aux sc error happened\n");
			return -EPERM;
		}
	}
	return 0;
}

static int st_start_chk_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	if (di->working_mode == LVC_MODE) {
		hwlog_info("lvc mode check begin\n");
	} else if (di->working_mode == SC_MODE) {
		hwlog_info("sc mode check begin\n");
	} else if (di->working_mode == SC4_MODE) {
		hwlog_info("sc4 mode check begin\n");
	} else {
		hwlog_info("illegal mode\n");
		return -EPERM;
	}

	dc_init_cable_curr(di->working_mode);
	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_CHECK_START, NULL);
	return 0;
}

static void dc_exceed_err_cnt_handler(struct direct_charge_device *di)
{
	int full_res_err_cnt;

	hwlog_info("error exceed %d times, dc is disabled\n", DC_ERR_CNT_MAX);
	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_CHARGE_DISABLED);
	wired_connect_send_icon_uevent(ICON_TYPE_NORMAL);

	if ((di->dc_err_report_flag == FALSE) &&
		(di->dc_open_retry_cnt <= DC_OPEN_RETRY_CNT_MAX)) {
		dc_show_eh_buf(di->dsm_buff);
		if (dc_get_cable_inherent_info(DC_CABLE_DETECT_OK) == CABLE_DETECT_OK) {
			full_res_err_cnt = dc_get_err_tpye_cnt(di->err_type_cnt,
				DC_EH_FULL_PATH_RESISTANCE);
			if (full_res_err_cnt < DC_FULL_RES_ERR_MAX)
				dc_report_eh_buf(di->dsm_buff,
					POWER_DSM_DIRECT_CHARGE_ERR_WITH_STD_CABLE);
		} else {
			dc_report_eh_buf(di->dsm_buff,
				POWER_DSM_DIRECT_CHARGE_ERR_WITH_NONSTD_CABLE);
		}

		dc_clean_eh_buf(di->dsm_buff, sizeof(di->dsm_buff));
		di->dc_err_report_flag = TRUE;
	}

}

static int st_anomaly_chk_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	if (di->error_cnt >= DC_ERR_CNT_MAX) {
		dc_exceed_err_cnt_handler(di);
		return -EPERM;
	}

	dc_set_stop_charging_flag(false);

	if (multi_ic_check_select_init_mode(&di->multi_ic_mode_para, &di->cur_mode)) {
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_BTB_CHECK);
		return -EPERM;
	}
	/* rt test set aux sc mode */
	if (direct_charge_rt_set_aux_mode(di))
		return -EPERM;

	if (uscp_is_in_protect_mode()) {
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_CHARGE_DISABLED);
		hwlog_err("%d is disabled by uscp\n", di->working_mode);
		return -EPERM;
	}

	return 0;
}

static int st_evt_pre_rept_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;
	int max_pwr = dc_pmode_get_optimal_mode_max_pwr(QUICK_CHECK);

	if (dc_get_cable_inherent_info(DC_CABLE_DETECT_OK) == CABLE_DETECT_OK)
		dc_turbo_send_max_power(max_pwr);

	return 0;
}

static int st_antifake_chk_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	if (dc_check_adapter_antifake(di->local_mode, di->dc_volt_ratio) < 0) {
		dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_ADAPTER_ANTI_FAKE);
		dc_turbo_send_max_power(0);
		di->error_cnt += 1;
		if (dc_adpt_get_mode_info(di->local_mode, ADAP_ANTIFAKE_EXE_EN))
			return -EPERM;
	}

	return 0;
}

static int st_prot_chk_entry(void *p)
{
	if (dc_is_scp_superior()) {
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ADAPTER_PROTOCOL,
			POWER_IF_SYSFS_ADAPTER_PROTOCOL, SYSFS_PROTOCOL_SCP);
		charge_set_fcp_disable_flag(true, FCP_DISABLE_PLATFORM_VOTER);
		dc_clear_cable_inherent_info();
		charge_request_charge_monitor();
		return -EPERM;
	}
	return 0;
}

static int st_evt_rept_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_ADAPTER_DETECT);
	dc_update_ui_cable_type();
	dc_update_cable_resistance_thld(di->local_mode);
	dc_send_soc_decimal_uevent();
	dc_send_cable_type_uevent(di->local_mode);
	dc_send_max_power_uevent();
	dc_turbo_send_max_power(di->max_pwr);

	if (dc_get_cable_inherent_info(DC_CABLE_TYPE) == DC_STD_CABLE)
		wltx_dc_adaptor_handler();

	return 0;
}

static int st_mode_rechk_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	if (di->local_mode != dc_pmode_get_optimal_mode(NORMAL_CHECK, di->local_mode, false)) {
		hwlog_err("mode need recheck\n");
		return -EPERM;
	}
	return 0;
}

static int st_upd_adap_info_entry(void *p)
{
	dc_update_adapter_info();
	return 0;
}

static int direct_charge_tbat_chk(struct direct_charge_device *di)
{
	int bat_temp = 0;
	int bat_temp_cur_max;

	di->tbat_id = (u32)multi_ic_check_select_tbat_id(&di->multi_ic_mode_para);
	hwlog_info("tbat_id=%u\n", di->tbat_id);
	bat_temp_get_temperature(di->tbat_id, &bat_temp);
	di->bat_temp_before_charging = bat_temp;
	if ((bat_temp < DC_LOW_TEMP_MAX + (int)di->low_temp_hysteresis) ||
		(bat_temp >= DC_HIGH_TEMP_MAX - (int)di->high_temp_hysteresis))
		goto fail;

	bat_temp_cur_max = direct_charge_battery_temp_handler(bat_temp);
	if (bat_temp_cur_max == 0)
		goto fail;

	return 0;
fail:
	hwlog_err("can not do direct charging, temp=%d out of range[%d %d]\n", bat_temp,
		DC_LOW_TEMP_MAX + di->low_temp_hysteresis,
		DC_HIGH_TEMP_MAX - di->high_temp_hysteresis);
	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_BAT_TEMP);
	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_TEMP_ERR, NULL);
	return -EPERM;
}

static int direct_charge_vbat_chk(struct direct_charge_device *di)
{
	int volt_max = hw_battery_voltage(BAT_ID_MAX);
	int volt_min = hw_battery_voltage(BAT_ID_MIN);
	if ((volt_min >= di->min_dc_bat_vol) && (volt_max <= di->max_dc_bat_vol))
		return 0;

	hwlog_err("can not do direct charging, v_min=%d v_max=%d out of range[%d %d]\n",
		volt_min, volt_max, di->min_dc_bat_vol, di->max_dc_bat_vol);
	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_BAT_VOL);
	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_VOLTAGE_INVALID, NULL);
	return -EPERM;
}

static int direct_charge_smart_bat_chk(struct direct_charge_device *di)
{
	if (!power_platform_is_smart_battery())
		return 0;

	if (power_platform_get_desired_charging_current() == 0) {
		hwlog_err("can not do direct charging, smart battery abnormal\n");
		return -EPERM;
	}

	return 0;
}

static int st_bat_chk_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	if (direct_charge_tbat_chk(di))
		return -EPERM;
	if (direct_charge_vbat_chk(di))
		return -EPERM;
	if (direct_charge_smart_bat_chk(di))
		return -EPERM;

	return 0;
}

static int st_antifake_rechk_entry(void *p)
{
	if (dc_adpt_get_inherent_info(ADAP_ANTIFAKE_RESULT) == ADAPTER_ANTIFAKE_FAIL) {
		hwlog_err("current adapter antifake check failed\n");
		return -EPERM;
	}

	return 0;
}

static int st_switch_path_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	charge_set_usbpd_disable(true);
	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_SWITCH_PATH, NULL);
	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_SWITCH);
	di->scp_stop_charging_complete_flag = 0;
	if (direct_charge_switch_path_to_dc_charging()) {
		hwlog_err("direct_charge path switch failed\n");
		return -EPERM;
	}

	return 0;
}

static int st_init_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_INIT);
	if (direct_charge_init_adapter_and_device()) {
		hwlog_err("direct_charge init failed\n");
		multi_ic_check_set_ic_error_flag(di->cur_mode, &di->multi_ic_mode_para);
		return -EPERM;
	}

	return 0;
}

static int st_vadap_chk_entry(void *p)
{
	struct adapter_source_info source_info = { 0 };
	int ibat = 0;
	int vol_err;
	int i;
	int usb_vol = 0;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_ADAPTER_VOLTAGE_ACCURACY);
	di->adaptor_vset = di->init_adapter_vset;
	if (dc_set_adapter_voltage(di->working_mode, &di->adaptor_vset))
		return -EPERM;

	/* delay 500ms */
	power_msleep(DT_MSLEEP_500MS, DT_MSLEEP_25MS, dc_need_exit);

	/* keep communication with the device within 1 second */
	direct_charge_get_bat_current(&ibat);

	for (i = 0; i < RETYR_TIMES; ++i) {
		if (dc_get_adapter_source_info(BIT(ADAPTER_OUTPUT_VOLT), &source_info))
			return -EPERM;

		vol_err = abs(source_info.output_volt - di->adaptor_vset);
		power_supply_get_int_prop(POWER_PLATFORM_USB_PSY_NAME,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, &usb_vol, 0, POWER_UV_PER_MV);
		snprintf(tmp_buf, sizeof(tmp_buf),
			"[%d]: Verr=%d, Verr_th=%d, Vset=%d, Vread=%d, Vbus=%d\n",
			i, vol_err, di->vol_err_th, di->adaptor_vset, source_info.output_volt,
			usb_vol);
		hwlog_info("%s", tmp_buf);
		if (vol_err > di->vol_err_th)
			goto fail;
	}

	hwlog_info("adapter voltage accuracy check succ\n");
	return 0;
fail:
	/* flash active, exit dc. ignore dmd error */
	dc_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
		DC_EH_APT_VOLTAGE_ACCURACY, tmp_buf);
	power_dsm_report_dmd(POWER_DSM_BATTERY,
		POWER_DSM_DIRECT_CHARGE_VOL_ACCURACY, tmp_buf);

	hwlog_err("adapter voltage accuracy check fail\n");
	multi_ic_check_set_ic_error_flag(di->cur_mode, &di->multi_ic_mode_para);
	return -EPERM;
}

static int st_ileak_chk_entry(void *p)
{
	struct adapter_source_info source_info = { 0 };
	int ibus = 0;
	int usb_vol = 0;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_USB_PORT_LEAKAGE_CURRENT);
	if (dc_adpt_skip_ileak_check())
		return 0;

	if (dc_get_device_ibus(&ibus) ||
		dc_get_adapter_source_info(BIT(ADAPTER_OUTPUT_CURR), &source_info))
		return -1;

	source_info.output_curr -= dc_get_gain_ibus();
	power_supply_get_int_prop(POWER_PLATFORM_USB_PSY_NAME,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &usb_vol, 0, POWER_UV_PER_MV);
	snprintf(tmp_buf, sizeof(tmp_buf),
		"Iapt=%d, Ileakage_cur_th=%d, Ibus_ls=%d, Vbus=%d\n",
		source_info.output_curr, di->adaptor_leakage_current_th, ibus,
		usb_vol);
	hwlog_info("%s\n", tmp_buf);

	if (source_info.output_curr <= di->adaptor_leakage_current_th) {
		hwlog_info("usb port leakage current check succ\n");
		return 0;
	}

	/* process error report */
	if (dc_get_cable_inherent_info(DC_CABLE_DETECT_OK) == CABLE_DETECT_OK) {
		dc_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
			DC_EH_USB_PORT_LEAGAGE_CURR, tmp_buf);
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			POWER_DSM_DIRECT_CHARGE_USB_PORT_LEAKAGE_CURRENT, tmp_buf);
	}

	hwlog_err("usb port leakage current check fail\n");
	multi_ic_check_set_ic_error_flag(di->cur_mode, &di->multi_ic_mode_para);
	return -EPERM;
}

static int st_open_chk_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_OPEN_CHARGE_PATH);
	if (direct_charge_open_charging_path()) {
		hwlog_err("open_path_check failed\n");
		multi_ic_check_set_ic_error_flag(di->cur_mode, &di->multi_ic_mode_para);
		return -EPERM;
	}

	power_event_bnc_notify(POWER_BNT_CHANNEL, POWER_NE_WIRED_DC_CHANNEL, &di->dc_volt_ratio);
	return 0;
}

static int st_res_chk_entry(void *p)
{
	struct adapter_source_info source_info = { 0 };
	int rpath = 0;
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	int rpath_th;
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	dc_mmi_set_succ_flag(di->working_mode, DC_ERROR_FULL_REISISTANCE);
	if (direct_charge_skip_check_resistance(di->local_mode))
		return 0;

	/* delay 500ms */
	power_msleep(DT_MSLEEP_500MS, DT_MSLEEP_25MS, dc_need_exit);

	/* keep communication with the adapter within 1 second */
	if (dc_get_adapter_source_info(BIT(ADAPTER_OUTPUT_VOLT), &source_info))
		return -1;

	/* keep communication with device within 1 second */
	if (direct_charge_get_device_close_status())
		return -1;

	/* delay 400ms */
	power_msleep(DT_MSLEEP_400MS, DT_MSLEEP_25MS, dc_need_exit);
	if (dc_calculate_path_resistance(di->local_mode, &rpath))
		return -1;

	rpath_th = (int)dc_get_cable_mode_info(di->local_mode, CABLE_FULL_ATH_RES_THLD);
	hwlog_info("average Rpath=%d, Rpath_threshold=[%d,%d]\n",
		rpath, -rpath_th, rpath_th);

	if (rpath <= rpath_th) {
		dc_resist_handler(di->working_mode, rpath);
		if (dc_get_cable_inherent_info(DC_CABLE_DETECT_OK) == CABLE_DETECT_NOK) {
			dc_send_icon_uevent();
			dc_send_soc_decimal_uevent();
			dc_send_max_power_uevent();
		}

		hwlog_err("full path resistance check succ\n");
		return 0;
	}

	dc_set_err_type_cnt(di->err_type_cnt, DC_EH_FULL_PATH_RESISTANCE);
	/* process error report */
	if (dc_get_cable_inherent_info(DC_CABLE_DETECT_OK)  == CABLE_DETECT_OK) {
		snprintf(tmp_buf, sizeof(tmp_buf), "full_res %d is out of[%d, %d]\n",
			rpath, -rpath_th, rpath_th);

		dc_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
			DC_EH_FULL_PATH_RESISTANCE, tmp_buf);
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			POWER_DSM_DIRECT_CHARGE_FULL_PATH_RESISTANCE, tmp_buf);
	}

	hwlog_err("full path resistance check fail\n");
	multi_ic_check_set_ic_error_flag(di->cur_mode, &di->multi_ic_mode_para);
	return -EPERM;
}

static int st_start_chg_entry(void *p)
{
	struct direct_charge_device *di = (struct direct_charge_device *)p;

	direct_charge_check_set_succ_flag(di->working_mode);
	di->dc_succ_flag = DC_SUCCESS;
	if (dc_ctrl_start_charging(di->working_mode))
		return -EPERM;

	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_CHECK_SUCC, NULL);
	return 0;
}

static int st_abend_retry_entry(void *p)
{
	direct_charge_set_stage_status(DC_STAGE_DEFAULT);
	return 0;
}

static int st_abend_1_entry(void *p)
{
	direct_charge_set_stage_status(DC_STAGE_DEFAULT);
	return 0;
}

static int st_abend_2_entry(void *p)
{
	unsigned int stage;

	dc_set_stop_charging_flag(true);
	direct_charge_stop_charging();
	stage = direct_charge_get_stage_status();
	hwlog_info("direct_charge stage=%d,%s\n", stage,
		direct_charge_get_stage_status_string(stage));
	return 0;
}

enum fsm_state {
	ST_BEGIN,
	ST_START_CHK,
	ST_ANOMALY_CHK,
	ST_EVT_PRE_REPT,
	ST_ANTIFAKE_CHK,
	ST_PROT_CHK,
	ST_EVT_REPT,
	ST_MODE_RECHK,
	ST_UPD_ADAP_INFO,
	ST_BAT_CHK,
	ST_ANTIFAKE_RECHK,
	ST_SWITCH_PATH,
	ST_INIT,
	ST_VADAP_CHK,
	ST_ILEAK_CHK,
	ST_OPEN_CHK,
	ST_RES_CHK,
	ST_START_CHA,
	ST_ABEND_RECHK,
	ST_ABEND_1, /* before path switch */
	ST_ABEND_2, /* after path switch */
	ST_END,
};

static const char * const g_fsm_state[ST_END + 1] = {
	[ST_BEGIN] = "st_begin",
	[ST_START_CHK] = "st_start_check",
	[ST_ANOMALY_CHK] = "st_anomaly_check",
	[ST_EVT_PRE_REPT] = "st_event_pre_report",
	[ST_ANTIFAKE_CHK] = "st_antifake_check",
	[ST_PROT_CHK] = "st_protocol_check",
	[ST_EVT_REPT] = "st_event_report",
	[ST_MODE_RECHK] = "st_mode_rechk",
	[ST_UPD_ADAP_INFO] = "st_update_adapter_info",
	[ST_BAT_CHK] = "st_bat_check",
	[ST_ANTIFAKE_RECHK] = "st_antifake_recheck",
	[ST_SWITCH_PATH] = "st_switch_path",
	[ST_INIT] = "st_init_adapter_and_device",
	[ST_VADAP_CHK] = "st_adapter_voltage_check",
	[ST_ILEAK_CHK] = "st_leak_current_check",
	[ST_OPEN_CHK] = "st_open_path_check",
	[ST_RES_CHK] = "st_resistance_check",
	[ST_START_CHA] = "st_start_charging",
	[ST_ABEND_RECHK] = "st_abnormal_exit:need retry check",
	[ST_ABEND_1] = "st_abnormal_exit_before_path_switch",
	[ST_ABEND_2] = "st_abnormal_exit_after_path_switch",
	[ST_END] = "st_end",
};

const char *get_state_string(unsigned int st)
{
	if ((st >= ST_BEGIN) && (st <= ST_END))
		return g_fsm_state[st];

	return "illegal state";
}

typedef int (*state_action)(void *);
struct fsm_map {
	enum fsm_state cur_state;
	state_action switch_action;
	enum fsm_state next_state;
	state_action fail_action;
	enum fsm_state fail_state;
};

/* event:charging */
static struct fsm_map fsm_charging_map[] = {
	{ ST_BEGIN, st_start_chk_entry, ST_START_CHK, st_abend_1_entry, ST_ABEND_1 },
	{ ST_START_CHK, st_anomaly_chk_entry, ST_ANOMALY_CHK, st_abend_1_entry, ST_ABEND_1 },
	{ ST_ANOMALY_CHK, st_evt_pre_rept_entry, ST_EVT_PRE_REPT, st_abend_1_entry, ST_ABEND_1 },
	{ ST_EVT_PRE_REPT, st_antifake_chk_entry, ST_ANTIFAKE_CHK, st_abend_1_entry, ST_ABEND_1 },
	{ ST_ANTIFAKE_CHK, st_prot_chk_entry, ST_PROT_CHK, st_abend_1_entry, ST_ABEND_1 },
	{ ST_PROT_CHK, st_evt_rept_entry, ST_EVT_REPT, st_abend_1_entry, ST_ABEND_1 },
	{ ST_EVT_REPT, st_mode_rechk_entry, ST_MODE_RECHK, st_abend_retry_entry, ST_ABEND_RECHK },
	{ ST_MODE_RECHK, st_upd_adap_info_entry, ST_UPD_ADAP_INFO, st_abend_1_entry, ST_ABEND_1 },
	{ ST_UPD_ADAP_INFO, st_bat_chk_entry, ST_BAT_CHK, st_abend_1_entry, ST_ABEND_1 },
	{ ST_BAT_CHK, st_antifake_rechk_entry, ST_ANTIFAKE_RECHK, st_abend_1_entry, ST_ABEND_1 },
	{ ST_ANTIFAKE_RECHK, st_switch_path_entry, ST_SWITCH_PATH, st_abend_2_entry, ST_ABEND_2 },
	{ ST_SWITCH_PATH, st_init_entry, ST_INIT, st_abend_2_entry, ST_ABEND_2 },
	{ ST_INIT, st_vadap_chk_entry, ST_VADAP_CHK, st_abend_2_entry, ST_ABEND_2 },
	{ ST_VADAP_CHK, st_ileak_chk_entry, ST_ILEAK_CHK, st_abend_2_entry, ST_ABEND_2 },
	{ ST_ILEAK_CHK, st_open_chk_entry, ST_OPEN_CHK, st_abend_2_entry, ST_ABEND_2 },
	{ ST_OPEN_CHK, st_res_chk_entry, ST_RES_CHK, st_abend_2_entry, ST_ABEND_2 },
	{ ST_RES_CHK, st_start_chg_entry, ST_START_CHA, st_abend_2_entry, ST_ABEND_2 },
	{ ST_START_CHA, NULL, ST_END, NULL, ST_END },
};

/* event:not charging */
static struct fsm_map fsm_notcharging_map[] = {
	{ ST_BEGIN, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_START_CHK, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_ANOMALY_CHK, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_EVT_PRE_REPT, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_ANTIFAKE_CHK, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_PROT_CHK, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_EVT_REPT, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_MODE_RECHK, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_UPD_ADAP_INFO, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_BAT_CHK, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_ANTIFAKE_RECHK, st_abend_1_entry, ST_ABEND_1, NULL, ST_ABEND_1 },
	{ ST_SWITCH_PATH, st_abend_2_entry, ST_ABEND_2, NULL, ST_ABEND_2 },
	{ ST_INIT, st_abend_2_entry, ST_ABEND_2, NULL, ST_ABEND_2 },
	{ ST_VADAP_CHK, st_abend_2_entry, ST_ABEND_2, NULL, ST_ABEND_2 },
	{ ST_ILEAK_CHK, st_abend_2_entry, ST_ABEND_2, NULL, ST_ABEND_2 },
	{ ST_OPEN_CHK, st_abend_2_entry, ST_ABEND_2, NULL, ST_ABEND_2 },
	{ ST_RES_CHK, st_abend_2_entry, ST_ABEND_2, NULL, ST_ABEND_2 },
	{ ST_START_CHA, st_abend_2_entry, ST_ABEND_2, NULL, ST_ABEND_2 },
};

/* fsm:finite-state machine */
struct dc_check_fsm {
	struct fsm_map *map;
	enum fsm_state cur_state;
	enum fsm_state next_state;
	int map_size;
};

static void dc_fcm_init_map(struct dc_check_fsm *fsm, enum fsm_event event)
{
	switch (event) {
	case EVT_CHARGING:
		fsm->map = fsm_charging_map;
		fsm->map_size = ARRAY_SIZE(fsm_charging_map);
		break;
	case EVT_NOT_CHARGING:
		hwlog_err("[check_fsm] event=%d\n", event);
		fsm->map = fsm_notcharging_map;
		fsm->map_size = ARRAY_SIZE(fsm_notcharging_map);
		break;
	default:
		fsm->map = NULL;
		fsm->map_size = 0;
		break;
	}
}

static void dc_fsm_handle(struct dc_check_fsm *fsm, void *p)
{
	int i;
	int ret = 0;
	struct fsm_map *map = fsm->map;

	for (i = 0; i < fsm->map_size; i++) {
		if (fsm->cur_state != map[i].cur_state)
			continue;

		hwlog_info("[check_fsm] switch to %s\n", get_state_string(map[i].next_state));
		if (map[i].switch_action)
			ret = map[i].switch_action(p);
		if (!ret) {
			fsm->next_state = map[i].next_state;
			break;
		}
		hwlog_err("[check_fsm] switch to %s fail, switch to %s\n",
			get_state_string(map[i].next_state),
			get_state_string(map[i].fail_state));
		fsm->next_state = map[i].fail_state;
		if (map[i].fail_action)
			(void)map[i].fail_action(p);
		break;
	}
}

static void dc_fsm_swtich_state(struct dc_check_fsm *fsm)
{
	fsm->cur_state = fsm->next_state;
	fsm->next_state = ST_END;
}

static int direct_charge_mode_check(struct direct_charge_device *di)
{
	struct dc_check_fsm fsm;
	enum fsm_event event = dc_get_charging_event();

	fsm.cur_state = ST_BEGIN;
	fsm.next_state = ST_END;
	while ((fsm.cur_state != ST_END) && (fsm.cur_state != ST_ABEND_1) &&
		(fsm.cur_state != ST_ABEND_2) && (fsm.cur_state != ST_ABEND_RECHK)) {
		event = dc_get_charging_event();
		dc_fcm_init_map(&fsm, event);
		dc_fsm_handle(&fsm, (void *)di);
		dc_fsm_swtich_state(&fsm);
	}
	if (fsm.cur_state == ST_ABEND_RECHK)
		return MODE_NEED_RECHK;
	return 0;
}

bool direct_charge_in_mode_check(void)
{
	return g_dc_in_mode_check;
}

static int direct_charge_enter_specified_mode(int mode, unsigned int adp_mode)
{
	int ret;
	struct direct_charge_device *di = direct_charge_get_di_by_mode(mode);

	di->working_mode = mode;
	di->adapter_type = adp_mode;
	direct_charge_set_di(di);
	dc_test_set_adapter_test_result(mode, AT_DETECT_SUCC);

	g_dc_in_mode_check = true;
	ret = direct_charge_mode_check(di);
	g_dc_in_mode_check = false;
	if (ret)
		return ret;
	if (direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE) {
		if (mode == LVC_MODE)
			power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_LVC_CHARGING, NULL);
		else if (mode == SC_MODE)
			power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_SC_CHARGING, NULL);
	}
	return 0;
}

void direct_charge_check(void)
{
	int ret;
	unsigned int mode;
	unsigned int adp_mode = ADAPTER_SUPPORT_UNDEFINED;

	hwlog_info("check\n");
	dc_pmode_clear_data();
	direct_charge_rework_priority_inversion();
	direct_charge_check_recharge();
	if (dc_pmode_init_local_mode())
		return;

	if (dc_pmode_init_adp_mode(&adp_mode))
		goto retry;

	charge_reset_quicken_work_flag();
	dc_pmode_update_mode_power();
	mode = dc_pmode_get_optimal_mode(DEFAULT_CHECK, UNDEFINED_MODE, false);
	if (mode == UNDEFINED_MODE) {
		hwlog_err("mode not matched");
		goto exit;
	}

	direct_charge_set_can_enter_status(true);
	ret = direct_charge_enter_specified_mode(mode, adp_mode);
	if (ret == MODE_NEED_RECHK)
		goto retry;
exit:
	/* set adapter default vbus will drop to 5v, set only in ufcs */
	if ((direct_charge_in_charging_stage() != DC_IN_CHARGING_STAGE) &&
		(adapter_detect_get_runtime_protocol_type() == BIT(ADAPTER_PROTOCOL_UFCS)))
		dc_set_adapter_default();
	return;
retry:
	if (adp_mode == ADAPTER_TEST_MODE)
		charge_set_fcp_disable_flag(true, FCP_DISABLE_PLATFORM_VOTER);
	charge_update_quicken_work_flag();
}

int direct_charge_pre_check(void)
{
	unsigned int local_mode;
	unsigned int adp_mode = ADAPTER_SUPPORT_UNDEFINED;

	hwlog_info("pre_check\n");

	local_mode = direct_charge_get_local_mode();
	if (!dc_adpt_detect_ping()) {
		adp_mode = dc_adpt_get_adapter_support_mode();
		/* exit ufcs mode to enable adapter pd function */
		if (adapter_detect_get_runtime_protocol_type() == BIT(ADAPTER_PROTOCOL_UFCS)) {
			dc_set_adapter_default();
			power_msleep(DT_MSLEEP_500MS, 0, NULL);
		}
	}
	if (adp_mode & local_mode) {
		hwlog_info("adapter support lvc or sc\n");
		return 0;
	}

	hwlog_info("adapter not support lvc or sc\n");
	return -1;
}
