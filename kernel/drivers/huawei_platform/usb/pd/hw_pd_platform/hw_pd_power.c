/*
 * hw_pd_power.c
 *
 * Source file of power operation interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/usb/hw_pd/hw_pd_power.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_WIRELESS_CHARGER
#include <huawei_platform/power/wireless/wireless_charger.h>
#endif
#include <chipset_common/hwusb/hw_pd/hw_pd_core.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/common_module/power_icon.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_cable.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#ifdef CONFIG_BOOST_5V
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#endif

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_pd_power
HWLOG_REGIST();
#endif /* HWLOG_TAG */

#define PD_DPM_POWER_QUICK_CHG_THR      18000000
#define PD_DPM_POWER_WIRELESS_CHG_THR   9000000
#define VBUS_VOL_9000MV 9000

#define UCDM_WORK_MAX_CNT 30

static struct notifier_block g_chrg_wake_unlock_nb;

#ifdef CONFIG_UVDM_CHARGER
static struct work_struct g_uvdm_work;
static unsigned int g_uvdm_work_cnt;
#endif

#ifdef CONFIG_WIRELESS_CHARGER
void hw_pd_power_vbus_ch_open(void)
{
	if (pd_state(otg_channel)) {
		pd_state(ignore_vbuson) = false;
		vbus_ch_open(VBUS_CH_USER_PD, VBUS_CH_TYPE_BOOST_GPIO, false);
	} else {
		pd_state(ignore_vbuson) = true;
		if (pogopin_is_support() && pogopin_otg_from_buckboost())
			vbus_ch_open(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST, true);
		else
			hw_pd_set_source_sink(POWER_SUPPLY_START_SOURCE);
		hw_pd_vbus_event_notify(PLEASE_PROVIDE_POWER, NULL);
	}
}

void hw_pd_power_vbus_ch_close(void)
{
	if (pd_state(otg_channel)) {
		pd_state(ignore_vbusoff) = false;
		vbus_ch_close(VBUS_CH_USER_PD, VBUS_CH_TYPE_BOOST_GPIO, false, false);
	} else {
		pd_state(ignore_vbusoff) = true;
		if (pogopin_is_support() && pogopin_otg_from_buckboost())
			vbus_ch_close(VBUS_CH_USER_PD,
				VBUS_CH_TYPE_POGOPIN_BOOST, true, false);
		else
			hw_pd_set_source_sink(POWER_SUPPLY_STOP_SOURCE);
		hw_pd_vbus_event_notify(CHARGER_TYPE_NONE, NULL);
	}
}
#else
void hw_pd_power_vbus_ch_open(void)
{
	if (pogopin_is_support() && pogopin_otg_from_buckboost()) {
		vbus_ch_open(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST, true);
	} else {
		pd_state(ignore_vbuson) = true;
		hw_pd_set_source_sink(POWER_SUPPLY_START_SOURCE);
	}
	hw_pd_vbus_event_notify(PLEASE_PROVIDE_POWER, NULL);
}

void hw_pd_power_vbus_ch_close(void)
{
	if (pogopin_is_support() && pogopin_otg_from_buckboost()) {
		vbus_ch_close(VBUS_CH_USER_PD, VBUS_CH_TYPE_POGOPIN_BOOST, true, false);
	} else {
		pd_state(ignore_vbusoff) = true;
		hw_pd_set_source_sink(POWER_SUPPLY_STOP_SOURCE);
	}
	hw_pd_vbus_event_notify(CHARGER_TYPE_NONE, NULL);
}
#endif /* CONFIG_WIRELESS_CHARGER */

void hw_pd_power_set_state(enum PD_POWER_TYPE pwr_type, int on)
{
	unsigned int type = VBUS_CH_TYPE_BOOST_GPIO;
	int ret;

	if (pwr_type == PD_POWER_POGOPIN)
		type = VBUS_CH_TYPE_POGOPIN_BOOST;

	if (on)
		ret = vbus_ch_open(VBUS_CH_USER_PD, type, true);
	else
		ret = vbus_ch_close(VBUS_CH_USER_PD, type, true, false);

	hwlog_info("%s set %u to %d, ret %d\n", __func__, type, on, ret);
}

void hw_pd_power_set_vconn(int enable)
{
#ifdef CONFIG_BOOST_5V
	boost_5v_enable((enable ? true : false), BOOST_CTRL_PD_VCONN);
#endif /* CONFIG_BOOST_5V */
}

void hw_pd_power_set_max_power(int max_power)
{
	int icon_type;

	hwlog_info("%s max_power=%d\n", __func__, max_power);
	/* wireless cover */
	if ((pd_state(product_type) == PD_PDT_WIRELESS_COVER) ||
		(pd_state(product_type) == PD_PDT_WIRELESS_COVER_TWO)) {
		if (max_power >= PD_DPM_POWER_WIRELESS_CHG_THR) {
			icon_type = ICON_TYPE_WIRELESS_QUICK;
			power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
		}
		hwlog_info("%s wireless cover is on\n", __func__);
		return;
	}

	if (max_power >= PD_DPM_POWER_WIRELESS_CHG_THR)
		pd_state(optional_wireless_cover) = true;
	else
		pd_state(optional_wireless_cover) = false;

	/* wired */
	if (max_power >= PD_DPM_POWER_QUICK_CHG_THR) {
		pd_state(optional_max_power) = true;
		if (power_icon_inquire() != ICON_TYPE_SUPER) {
			icon_type = ICON_TYPE_QUICK;
			power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
		}
	} else {
		pd_state(optional_max_power) = false;
	}
}

void hw_pd_power_set_source_sink_state(int type)
{
	power_supply_set_int_property_value("charger_detection",
		POWER_SUPPLY_PROP_CHG_PLUGIN, type);
}

void hw_pd_power_set_charge_state(int mv, int ma)
{
	bool high_power_charging = false;
	bool high_voltage_charging = false;

	hwlog_info("%s : Sink %d mV, %d mA\n", __func__, mv, ma);
	if ((mv * ma) >= PD_DPM_POWER_QUICK_CHG_THR) {
		hwlog_info("%s : over 18w\n", __func__);
		high_power_charging = true;
	}
	if (mv >= VBUS_VOL_9000MV) {
		hwlog_info("%s : over 9V\n", __func__);
		high_voltage_charging = true;
	}
	hwlog_info("%s : %d\n", __func__, (mv * ma));
	pd_state(high_power_charging) = high_power_charging;
	pd_state(high_voltage_charging) = high_voltage_charging;
}

static int charge_wake_unlock_notifier_call(
	struct notifier_block *chrg_wake_unlock_nb, unsigned long event, void *data)
{
	switch (event) {
	case POWER_NE_CHG_WAKE_UNLOCK:
		hw_pd_wakelock_ctrl(PD_WAKE_UNLOCK);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

/* responds to charging cable detection */
static int direct_charge_cable_detect_cb(void)
{
	return hw_pd_dc_cable_detect();
}

static int pd_set_rtb_success_cb(unsigned int val)
{
	if (!power_cmdline_is_factory_mode())
		return 0;

	return hw_pd_set_rtb_success(val);
}

static int pd_dpm_cable_err(void)
{
	return 0;
}

static struct dc_cable_ops g_cable_detect_ops = {
	.detect = direct_charge_cable_detect_cb,
	.cable_auth_det = pd_dpm_cable_err,
};

static struct power_if_ops g_pd_if_ops = {
	.set_rtb_success = pd_set_rtb_success_cb,
	.type_name = "pd",
};

void hw_pd_power_init(void)
{
	g_chrg_wake_unlock_nb.notifier_call = charge_wake_unlock_notifier_call;
#ifdef CONFIG_HUAWEI_CHARGER_AP
	(void)power_event_bnc_register(POWER_BNT_CHG, &g_chrg_wake_unlock_nb);
#endif

	dc_cable_ops_register(&g_cable_detect_ops);
	power_if_ops_register(&g_pd_if_ops);
}

void hw_pd_cc_short_action(void)
{
#ifdef CONFIG_DIRECT_CHARGER
	unsigned int notifier_type = POWER_ANT_LVC_FAULT;
	int mode = direct_charge_get_working_mode();
	/* cc_dynamic_protect-0: disabled 1: only SC; 2: for SC and LVC */
	if (mode == SC4_MODE)
		notifier_type = POWER_ANT_SC4_FAULT;
	else if (mode == SC_MODE)
		notifier_type = POWER_ANT_SC_FAULT;
	else if ((mode == LVC_MODE) && (pd_state(cc_dynamic_protect) == 2))
		notifier_type = POWER_ANT_LVC_FAULT;

	power_event_anc_notify(notifier_type, POWER_NE_DC_FAULT_CC_SHORT, NULL);
#endif /* CONFIG_DIRECT_CHARGER */
	hwlog_info("cc_short_action\n");
}

unsigned long hw_pd_power_get_chgtype(void)
{
	return (unsigned long)chip_get_charger_type();
}

void pd_send_wireless_cover_uevent(void)
{
	int cover_status = P_OK; /* wireless cover is ready */
	int icon_type;

	power_ui_event_notify(POWER_UI_NE_WL_COVER_STATUS, &cover_status);

	if (pd_state(optional_wireless_cover))
		icon_type = ICON_TYPE_WIRELESS_QUICK;
	else
		icon_type = ICON_TYPE_WIRELESS_NORMAL;

	power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
}

#ifdef CONFIG_UVDM_CHARGER
static void uvdm_send_work(struct work_struct *work)
{
	int type = P_ER;

	g_uvdm_work_cnt++;
	/* g_uvdm_work cycle not more than 30 times */
	if (g_uvdm_work_cnt > UCDM_WORK_MAX_CNT)
		return;

	adapter_get_adp_type(ADAPTER_PROTOCOL_UVDM, &type);
	switch (type) {
	case UVDM_CAHRGE_TYPE_5W:
	case UVDM_CAHRGE_TYPE_10W:
	case UVDM_CHARGE_TYPE_20W:
		uvdm_check(type);
		break;
	default:
		schedule_work(&g_uvdm_work);
		break;
	}
}

void uvdm_init_work(void)
{
	INIT_WORK(&g_uvdm_work, uvdm_send_work);
}

void uvdm_cancel_work(void)
{
	cancel_work_sync(&g_uvdm_work);
}

void uvdm_schedule_work(void)
{
	g_uvdm_work_cnt = 0;
	schedule_work(&g_uvdm_work);
}
#endif /* CONFIG_UVDM_CHARGER */
