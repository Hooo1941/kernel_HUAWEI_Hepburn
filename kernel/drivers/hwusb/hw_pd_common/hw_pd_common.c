/*
 * hw_pd_common.c
 *
 * Source file of common interface for huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/console.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/timex.h>
#include <securec.h>

#include <chipset_common/hwusb/hw_usb.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_sysfs.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_core.h>

#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/usb/hw_pd/hw_pd_misc.h>
#include <huawei_platform/usb/hw_pd/hw_pd_power.h>
#include <huawei_platform/usb/hw_pd/hw_pd_event.h>
#include <huawei_platform/usb/hw_pd/hw_pd_vbus.h>
#include <huawei_platform/usb/usb_extra_modem.h>

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_pd_common
HWLOG_REGIST();
#endif

#define TIME_T_MAX              (time64_t)((1UL << ((sizeof(time64_t) << 3) - 1)) - 1)
#define PD_ROLE_DFP             1
#define TCP_VBUS_CTRL_PD_DETECT (1 << 7)

/* Global pd driver context  */
static struct pd_dpm_info *g_pd_di;

void hw_pd_set_source_sink(enum power_supply_sinksource_type type)
{
	hwlog_info("%s %d\n", __func__, type);
	pd_state(sink_source_type) = type;
	hw_pd_power_set_source_sink_state(type);
}

void hw_pd_emark_detect_finish(void)
{
	int typec_state = PD_DPM_USB_TYPEC_HOST_ATTACHED;

	if (pd_state(emark_finish_disable))
		return;
	hw_pd_get_typec_state(&typec_state);
	if (typec_state != PD_DPM_USB_TYPEC_HOST_ATTACHED)
		return;

	hw_pd_emark_finish();
}

int hw_pd_disable_pd(bool disable)
{
	hwlog_info("%s\n", __func__);

	if (uem_check_online_status())
		return -EPERM;

	return hw_pd_disable_pd_comm(disable);
}

void hw_pd_enable_drp(void)
{
	/*
	 * Bugfix for Hi65xx: DRP lost Cable(without
	 * adapter) plugin during Wireless.
	 * set 1 as PD_PDM_RE_ENABLE_DRP
	 */
	hw_pd_set_drp_state(1);
}

int hw_pd_notify_dc_status(bool dc)
{
	return hw_pd_set_dc_status(dc);
}

void hw_pd_issue_vbus_low(void)
{
	hwlog_info("%s: vbus low\n", __func__);
	if (g_pd_di && !pd_state(pd_source_vbus)) {
		hw_pd_vbus_event_notify(CHARGER_TYPE_NONE, NULL);
		hwlog_info("%s: issue a charger disconnect event\n", __func__);
	}
}

void hw_pd_get_typec_state(int *typec_state)
{
	if (g_pd_di && typec_state) {
		hwlog_info("%s state = %d\n", __func__, pd_state(cur_typec_state));
		*typec_state = pd_state(cur_typec_state);
	}
	return;
}

bool hw_pd_judge_is_cover(void)
{
	if ((pd_state(product_type) == PD_PDT_WIRELESS_COVER) ||
		(pd_state(product_type) == PD_PDT_WIRELESS_COVER_TWO))
		return true;

	return false;
}

void hw_pd_set_max_power(int max_power)
{
	if (pogopin_typec_chg_ana_audio_support() &&
		(pogopin_5pin_get_pogo_status() == POGO_CHARGER)) {
		hwlog_info("pogo charging, ignore max power notify\n");
		return;
	}

	hw_pd_power_set_max_power(max_power);
}

void hw_pd_set_product_id(unsigned int vid, unsigned int pid, unsigned int bcd)
{
	hwlog_info("%s vid = 0x%x, pid = 0x%x, bcd = 0x%x\n", __func__, vid, pid, bcd);
	uem_set_product_id_info(vid, pid);
	hw_pd_misc_handle_wireless_cover_event(vid, pid, bcd);
}

int hw_pd_vbus_event_notify(unsigned long event, void *data)
{
	struct pd_dpm_info *di = g_pd_di;

	if (event == CHARGER_TYPE_NONE) {
		pd_state(high_power_charging) = false;
		pd_state(optional_max_power) = false;
		pd_state(optional_wireless_cover) = false;
		pd_state(high_voltage_charging) = false;
	}
	if (event != PD_DPM_VBUS_TYPE_TYPEC)
		hw_pd_set_charge_state(event, data);
	return blocking_notifier_call_chain(&di->pd_evt_nh, event, data);
}

int hw_pd_support_cable_auth(void)
{
	return 0;
}

bool hw_pd_check_ccvbus_short(void)
{
	int ret;
	unsigned int cc1 = 0;
	unsigned int cc2 = 0;
	bool check = false;

	/*
	 * Get result from chip module directly,
	 * Only for HISI_PD at present.
	 */
	ret = hw_pd_check_cc_short(&check);
	if (!ret) {
		hwlog_err("%s - hisi pd\n", __func__);
		return check;
	}

	ret = hw_pd_get_cc_state(&cc1, &cc2);
	if (ret)
		return false;

	hwlog_info("%s: cc1:%d, cc2:%d\n", __func__, cc1, cc2);

	if ((cc1 == PD_DPM_CC_3_0) && (cc2 > PD_DPM_CC_OPEN))
		return true;

	if ((cc2 == PD_DPM_CC_3_0) && (cc1 > PD_DPM_CC_OPEN))
		return true;

	return false;
}

/*
 * Function:    hw_pd_get_cc_state
 * Description:  get cc1 and cc2 state
 *         open    56k    22k    10k
 *  cc1    00       01     10     11
 *  cc2    00       01     10     11
 * Input:          cc1: value of cc1  cc2: value of cc2
 * Output:         cc1: value of cc1  cc2: value of cc2
 * Return:         success: 0   fail: -1
*/
int hw_pd_get_cc_state(unsigned int *cc1, unsigned int *cc2)
{
	unsigned int cc = 0;
	int ret;

	if (!cc1 || !cc2)
		return -EINVAL;

	ret = hw_pd_query_cc_state(&cc);
	if (ret) {
		hwlog_err("%s - not support\n", __func__);
		return ret;
	}

	*cc1 = cc & PD_DPM_CC_STATUS_MASK;
	*cc2 = (cc >> PD_DPM_CC2_STATUS_OFFSET) & PD_DPM_CC_STATUS_MASK;

	return 0;
}

static void pd_report_source_vconn(const void *data)
{
	if (data) {
		hw_pd_power_set_vconn(*(int *)data);
		hwlog_info("%s - %d\n", __func__, (*(int *)data));
	}
}

static void pd_source_vbus_err(void *data)
{
	if (pd_state(otg_channel)) {
		hw_pd_power_set_state(PD_POWER_BOOST, 0);
#ifdef CONFIG_POGO_PIN
	} else if (pogopin_is_support() && (pogopin_otg_from_buckboost())) {
		hw_pd_power_set_state(PD_POWER_POGOPIN, 0);
#endif /* CONFIG_POGO_PIN */
	} else {
		hw_pd_vbus_event_notify(CHARGER_TYPE_NONE, data);
		hw_pd_set_source_sink(POWER_SUPPLY_STOP_SOURCE);
	}
}

static void pd_source_vbus_ok(void *data)
{
	pd_state(pd_source_vbus) = true;

	if (pd_state(otg_channel)) {
		hw_pd_power_set_state(PD_POWER_BOOST, 1);
#ifdef CONFIG_POGO_PIN
	} else if (pogopin_is_support() && (pogopin_otg_from_buckboost())) {
		hw_pd_power_set_state(PD_POWER_POGOPIN, 1);
#endif /* CONFIG_POGO_PIN */
	} else {
		hw_pd_vbus_event_notify(PLEASE_PROVIDE_POWER, data);
		hw_pd_set_source_sink(POWER_SUPPLY_START_SOURCE);
	}
}

static void pd_report_source_vbus(struct pd_dpm_info *di, void *data)
{
	struct pd_dpm_vbus_state *vbus_state = data;

	mutex_lock(&di->sink_vbus_lock);
	if (pd_state(external_pd_flag)) {
		if (pmic_vbus_irq_is_enabled())
			pd_state(ignore_bc12_vbuson) = true;
	} else {
		pd_state(ignore_bc12_vbuson) = true;
	}

	if (vbus_state->vbus_type & TCP_VBUS_CTRL_PD_DETECT) {
		pd_state(pd_finish_flag) = true;
		pd_state(ctc_cable_flag) = true;
	}

	if (vbus_state->mv == 0) {
		hwlog_info("%s : Disable\n", __func__);
		pd_source_vbus_err(data);
	} else {
		hwlog_info("%s : Source %d mV, %d mA\n", __func__, vbus_state->mv, vbus_state->ma);
		pd_source_vbus_ok(data);
	}
	mutex_unlock(&di->sink_vbus_lock);
}

static void pd_report_sink_vbus(struct pd_dpm_info *di, void *data)
{
	unsigned long event;
	struct pd_dpm_vbus_state *vbus_state = data;

	mutex_lock(&di->sink_vbus_lock);
	pd_state(remote_rp_level) = vbus_state->remote_rp_level;

	if (vbus_state->vbus_type & TCP_VBUS_CTRL_PD_DETECT) {
		adapter_test_set_result(AT_TYPE_PD, AT_PROTOCOL_FINISH_SUCC);
		if (pmic_vbus_irq_is_enabled())
			pd_state(ignore_bc12_vbuson) = true;
		pd_state(pd_finish_flag) = true;
		pd_state(ctc_cable_flag) = true;
	}

	if (pd_state(pd_finish_flag))
		event = PD_DPM_VBUS_TYPE_PD;
	else
		event = PD_DPM_VBUS_TYPE_TYPEC;

	vbus_state = data;

	if (vbus_state->mv == 0) {
		if (event == PD_DPM_VBUS_TYPE_PD) {
			hwlog_info("%s : Disable\n", __func__);
			hw_pd_vbus_event_notify(CHARGER_TYPE_NONE, data);
			hw_pd_set_source_sink(POWER_SUPPLY_STOP_SINK);
		}
	} else {
		pd_state(pd_source_vbus) = false;
		hw_pd_power_set_charge_state(vbus_state->mv, vbus_state->ma);

		if (pd_state(cur_typec_state) != PD_DPM_USB_TYPEC_DETACHED)
			hw_pd_set_source_sink(POWER_SUPPLY_START_SINK);
		hw_pd_vbus_event_notify(event, data);
	}

	mutex_unlock(&di->sink_vbus_lock);
}

void hw_pd_wakelock_ctrl(unsigned long event)
{
	int ret = -EINVAL;

	if ((event == PD_WAKE_LOCK) || (event == PD_WAKE_UNLOCK))
		ret = atomic_notifier_call_chain(&g_pd_di->pd_wake_unlock_evt_nh, event, NULL);
	hwlog_info("%s : event %d, ret %d\n", __func__, (int)event, ret);
}

/* fix the problem of OTG output still valid when OTG is pluging out */
static void pd_vbus_ctrl_detached(unsigned long event)
{
	int typec_state = PD_DPM_USB_TYPEC_DETACHED;

	hw_pd_get_typec_state(&typec_state);
	if ((event != PLEASE_PROVIDE_POWER) ||
		(typec_state != PD_DPM_USB_TYPEC_DETACHED) ||
		pd_state(audio_power_no_hs))
		return;

	hwlog_err("typec is detached, should cut off otg output\n");
	hw_pd_power_vbus_ch_close();
}

void hw_pd_vbus_ctrl(unsigned long event)
{
	struct pd_dpm_info *di = g_pd_di;
	int typec_state = PD_DPM_USB_TYPEC_DETACHED;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	hw_pd_get_typec_state(&typec_state);
	if ((typec_state == PD_DPM_USB_TYPEC_DETACHED) &&
		!pd_state(audio_power_no_hs)) {
		hwlog_info("%s typec unattached\n", __func__);
		return;
	}

	if (event == PLEASE_PROVIDE_POWER)
		hw_pd_power_vbus_ch_open();
	else
		hw_pd_power_vbus_ch_close();

	pd_vbus_ctrl_detached(event);
	hwlog_info("%s event = %ld\n", __func__, event);
}

int hw_pd_reg_event_notifier(struct notifier_block *nb)
{
	int ret = 0;

	if (!nb)
		return -EINVAL;

	if (!g_pd_di)
		return ret;

	ret = blocking_notifier_chain_register(&g_pd_di->pd_evt_nh, nb);
	if (ret != 0)
		return ret;

	return ret;
}

int hw_pd_unreg_event_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&g_pd_di->pd_evt_nh, nb);
}

int hw_pd_reg_port_notifier(struct notifier_block *nb)
{
	int ret = HW_PD_INVALID_STATE;

	if (!nb)
		return -EINVAL;

	if (!g_pd_di)
		return ret;

	ret = blocking_notifier_chain_register(&g_pd_di->pd_port_status_nh, nb);
	if (ret != 0)
		return ret;

	return ret;
}

int hw_pd_unreg_port_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&g_pd_di->pd_port_status_nh, nb);
}

int hw_pd_reg_wakeunlock_notifier(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;

	return atomic_notifier_chain_register(&g_pd_di->pd_wake_unlock_evt_nh, nb);
}

int hw_pd_unreg_wakeunlock_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&g_pd_di->pd_wake_unlock_evt_nh, nb);
}

enum cur_cap hw_pd_get_vdo_cap(void)
{
	enum cur_cap cap;

	cap = (pd_state(cable_info).cable_vdo & CABLE_CUR_CAP_MASK) >>
		CABLE_CUR_CAP_SHIFT;
	if (cap == PD_DPM_CURR_5A)
		cap += (pd_state(cable_info).cable_vdo_ext &
			CABLE_EXT_CUR_CAP_MASK) >> CABLE_EXT_CUR_CAP_SHIFT;

	hwlog_info("%s, cur_cap = %d\n", __func__, cap);
	return cap;
}

/* Check whether cc_vbus_short protect is supported */
static bool pd_is_cc_protection(void)
{
	/* cc_dynamic_protect-0: disabled 1: only SC; 2: for SC and LVC */
	if (!pd_state(cc_dynamic_protect))
		return false;

#ifdef CONFIG_DIRECT_CHARGER
	if (direct_charge_in_charging_stage() != DC_IN_CHARGING_STAGE)
		return false;
#endif /* CONFIG_DIRECT_CHARGER */

	if (!hw_pd_check_ccvbus_short())
		return false;

	hwlog_info("cc short\n");
	return true;
}

static void pd_cc_short_work(struct work_struct *work)
{
	hwlog_info("cc_short_work\n");
	if (!pd_is_cc_protection())
		return;

	hw_pd_cc_short_action();
}

void hw_pd_cc_dynamic_protect(void)
{
	if (!pd_is_cc_protection())
		return;

	/* Porcess in pd_cc_short_work */
	mod_delayed_work(system_wq, &g_pd_di->cc_short_work,
		msecs_to_jiffies(50)); /* exec 50ms later */
}

void hw_pd_set_cable_vdo(void *data)
{
	int ret;
	if (!data || !g_pd_di)
		return;

	ret = memcpy_s(&(pd_state(cable_info)), sizeof(pd_state(cable_info)), data, sizeof(pd_state(cable_info)));
	if (ret != 0)
		hwlog_err("%s copy cable info fail\r\n", __func__);
	hwlog_info("%s cable_vdo=%x, add_vdo=%x\n", __func__,
		pd_state(cable_info).cable_vdo, pd_state(cable_info).cable_vdo_ext);
}

/*
 * Process PD_DPM_TYPEC_UNATTACHED typec event.
 * usb_event : new usb state needed to be updated
 * audio_event: new usb-audio state needed to be updated
 */
static void hw_pd_typec_unattached(int *usb_event, int *audio_event)
{
	hwlog_info("%s +\r\n", __func__);
	pd_state(ctc_cable_flag) = false;
	pd_state(optional_max_power) = false;
	(void)hw_pd_misc_pre_handle_analog(ANALOG_DETTACH);
	*audio_event = hw_pd_misc_handle_analog(ANALOG_DETTACH);
	if (*audio_event >= 0) {
		if (g_pd_di->pd_reinit_enable && g_pd_di->is_ocp && g_pd_di->pd_vdd_ldo) {
			hwlog_err("%s ignore dettach event\n", __func__);
			*audio_event = HW_PD_INVALID_STATE;
		}
		*usb_event = PD_DPM_USB_TYPEC_AUDIO_DETACHED;
		hwlog_info("%s AUDIO UNATTACHED\r\n", __func__);
	} else {
		*usb_event = PD_DPM_USB_TYPEC_DETACHED;

		if (pd_dpm_get_source_sink_state() == POWER_SUPPLY_START_SINK)
			hw_pd_set_source_sink(POWER_SUPPLY_STOP_SINK);
		hwlog_info("%s UNATTACHED\r\n", __func__);
	}
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	if (pd_state(support_smart_holder)) {
		if (pd_state(smart_holder))
			hishow_notify_android_uevent(HISHOW_DEVICE_OFFLINE, HISHOW_USB_DEVICE);
		pd_state(smart_holder) = false;
	}
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	reinit_typec_completion();
	pd_state(cc_orientation_factory) = CC_ORIENTATION_FACTORY_SET;
	pd_set_product_type(PD_DPM_INVALID_VAL);
	pd_state(cable_info).cable_vdo = 0;
	pd_state(cable_info).cable_vdo_ext = 0;
	uem_handle_detach_event();

	hwlog_info("%s -\r\n", __func__);
}

/*
 * Process PD_DPM_TYPEC_ATTACHED_VBUS_ONLY typec event.
 * usb_event : new usb state needed to be updated.
 * Return false if current event should not be processed.
 */
static bool hw_typec_attach_vbus_only(int *usb_event)
{
	int ret;

	hwlog_info("%s +\n", __func__);
	pd_state(cc_orientation_factory) = CC_ORIENTATION_FACTORY_SET;
	pd_state(vbus_only_status) = true;
	if (pd_state(ignore_vbus_only)) {
		hwlog_err("%s: ignore ATTACHED_VBUS_ONLY\n", __func__);
		return false;
	}
	if (pd_state(vbus_only_min_voltage)) {
		ret = charge_get_vusb();
		hwlog_info("%s: charge_get_vusb: %d mV\n", __func__, ret);
		if (ret < pd_state(vbus_only_min_voltage)) {
			hwlog_err("%s: vusb less 3.9V, bypass\n", __func__);
			return false;
		}
	}
	hw_pd_set_source_sink(POWER_SUPPLY_START_SINK);
	*usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
	hwlog_info("%s -\n", __func__);
	return true;
}

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
/*
 * Process PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC2 typec event.
 * usb_event : new usb state needed to be updated.
 */
static void hw_typec_attach_custom_src2(int *usb_event)
{
	hwlog_info("%s +\n", __func__);

	if (!pd_state(support_smart_holder))
		return;

	*usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
	hwlog_info("%s ATTACHED_CUSTOM_SRC2\r\n", __func__);
	if (hw_pd_is_custsrc2_cable()) {
		if (!pd_state(smart_holder))
			hishow_notify_android_uevent(HISHOW_DEVICE_ONLINE,
				HISHOW_USB_DEVICE);
		pd_state(smart_holder) = true;
	}
	typec_complete(COMPLETE_FROM_TYPEC_CHANGE);

	hwlog_info("%s -\n", __func__);
}
#endif

/*
 * Process new typec event sent from PD phisical driver.
 * usb_event : new usb state needed to be updated.
 */
static void pd_set_typec_state(int typec_state)
{
	hwlog_info("%s state = %d\n", __func__, typec_state);
	blocking_notifier_call_chain(&g_pd_di->pd_port_status_nh, typec_state, NULL);
	pd_state(cur_typec_state) = typec_state;

	if ((pd_state(cur_typec_state) == PD_DPM_USB_TYPEC_NONE) ||
		(pd_state(cur_typec_state) == PD_DPM_USB_TYPEC_DETACHED) ||
		(pd_state(cur_typec_state) == PD_DPM_USB_TYPEC_AUDIO_DETACHED)) {
		hw_pd_misc_typec_state(1);
	} else {
		hw_pd_misc_typec_state(0);
	}

	if (pogopin_is_support())
		pogopin_set_typec_state(pd_state(cur_typec_state));
}

/* Handle abnormal cc change event, like water-in or vbus-only. */
static void pd_handle_abnomal_cc_change(int event)
{
	return;
}

/* Reset cc moisture flag after system fb event comes */
static void cc_moisture_restore_work(struct work_struct *work)
{
	hwlog_err("%s %d,%d\n", __func__,
		  pd_state(cc_moisture_happened),
		  pd_state(cc_moisture_status));
	if (!pd_state(cc_moisture_happened))
		pd_state(cc_moisture_status) = false;
}

/*
 * Function in work to process system fb event.
 * Broadcast this event to other module like charger driver.
 * Start a delayed work to reset cc moisture flag.
 */
static void fb_unblank_work(struct work_struct *work)
{
	unsigned int flag = WD_NON_STBY_DRY;

	hwlog_info("%s set pd to drp\n", __func__);
	hw_pd_set_cc_mode(PD_DPM_CC_MODE_DRP);
	power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_REPORT_UEVENT, &flag);

	if (pd_state(cc_moisture_report) == 0)
		return;

	pd_state(cc_moisture_happened) = false;
	/* 120000: delay 2 minute to restore the flag */
	queue_delayed_work(g_pd_di->usb_wq,
		&g_pd_di->cc_moisture_flag_restore_work,
		msecs_to_jiffies(120000));
}

/* Callback function to deal with system fb event */
static void fb_unblank_proc_cb(unsigned int type, char *event_data)
{
	hwlog_err("%s FB_BLANK_UNBLANK come\n", __func__);
	schedule_work(&g_pd_di->fb_work);
}

/* Handle typec state event, return false to ignore it. */
static bool pd_handle_typec_event(
	struct pd_dpm_typec_state *typec_state, int *usb_event)
{
	int audio_event = HW_PD_INVALID_STATE;
	int ret;

	pd_state(cur_usb_event) = typec_state->new_state;
	pd_state(cc_orientation_factory) = CC_ORIENTATION_FACTORY_UNSET;

	switch (typec_state->new_state) {
	case PD_DPM_TYPEC_ATTACHED_SNK:
		(void)hw_pd_misc_handle_analog(ANALOG_DISABLE);
		*usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
		hwlog_info("%s ATTACHED_SINK\r\n", __func__);
		hw_pd_set_source_sink(POWER_SUPPLY_START_SINK);
		typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
		break;
	case PD_DPM_TYPEC_ATTACHED_SRC:
		*usb_event = PD_DPM_USB_TYPEC_HOST_ATTACHED;
		hwlog_info("%s ATTACHED_SOURCE\r\n", __func__);
		typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
		break;
	case PD_DPM_TYPEC_UNATTACHED:
		hw_pd_typec_unattached(usb_event, &audio_event);
		break;
	case PD_DPM_TYPEC_ATTACHED_AUDIO:
		audio_event = hw_pd_misc_handle_analog(ANALOG_ATTACH);
		if (audio_event >= 0) {
			*usb_event = PD_DPM_USB_TYPEC_AUDIO_ATTACHED;
			hwlog_info("%s ATTACHED_AUDIO\r\n", __func__);
		}
		typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
		break;
	case PD_DPM_TYPEC_ATTACHED_DBGACC_SNK:
	case PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC:
		*usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
		hw_pd_set_source_sink(POWER_SUPPLY_START_SINK);
		pd_state(remote_rp_level) = PD_DPM_CC_VOLT_SNK_DFT;
		hwlog_info("%s ATTACHED_CUSTOM_SRC\r\n", __func__);
		typec_complete(COMPLETE_FROM_TYPEC_CHANGE);
		break;
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	case PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC2:
		hw_typec_attach_custom_src2(usb_event);
		break;
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	case PD_DPM_TYPEC_ATTACHED_DEBUG:
		pd_state(remote_rp_level) = PD_DPM_CC_VOLT_SNK_DFT;
		break;
	case PD_DPM_TYPEC_ATTACHED_VBUS_ONLY:
		if (!hw_typec_attach_vbus_only(usb_event))
			return false;
		break;
	case PD_DPM_TYPEC_UNATTACHED_VBUS_ONLY:
		pd_state(cc_orientation_factory) = CC_ORIENTATION_FACTORY_SET;
		pd_state(vbus_only_status) = false;
		hwlog_info("%s UNATTACHED_VBUS_ONLY\r\n", __func__);
		pd_handle_abnomal_cc_change(PD_DPM_UNATTACHED_VBUS_ONLY);
		*usb_event = PD_DPM_USB_TYPEC_DETACHED;
		hw_pd_set_source_sink(POWER_SUPPLY_STOP_SINK);
		break;
	default:
		hwlog_info("%s can not detect typec state\r\n", __func__);
		break;
	}
	
	pd_set_typec_state(*usb_event);

	if (typec_state->polarity)
		pd_state(cc_orientation) = true;
	else
		pd_state(cc_orientation) = false;

	if (audio_event >= 0) {
		hwlog_info("%s report misc event %d\r\n", __func__, audio_event);
		hw_pd_misc_notify_analog(audio_event);
	}

	return true;
}

static int pd_check_typec_event(struct pd_dpm_typec_state *typec_state, int *usb_event)
{
	int ret = 0;

	if (!typec_state || !usb_event || !g_pd_di) {
		hwlog_info("%s data is null\r\n", __func__);
		return HW_PD_INVALID_STATE;
	}

	if (typec_state->new_state == PD_DPM_TYPEC_ATTACHED_AUDIO) {
		ret = hw_pd_misc_pre_handle_analog(ANALOG_ATTACH);
		if (ret != 0) {
			hwlog_info("%s ignore analog attach event\r\n", __func__);
			return ret;
		}
	}

	if (!pd_handle_typec_event(typec_state, usb_event)) {
		hwlog_info("%s no usb event report\n", __func__);
		ret = HW_PD_IGNORE_STATE;
	}

	return ret;
}

/* Process PD_DPM_PE_EVT_DIS_VBUS_CTRL PD event */
static void hw_pd_disable_vbus_control(void)
{
	struct pd_dpm_vbus_state vbus_state;

	hwlog_info("%s +\n", __func__);
	vbus_state.mv = 0;
	vbus_state.ma = 0;
	if (!pd_state(pd_finish_flag) && !pd_state(pd_source_vbus))
		return;

	hwlog_info("%s : Disable VBUS Control\n", __func__);
	if (pd_state(otg_channel) && pd_state(pd_source_vbus)) {
		hw_pd_power_set_state(PD_POWER_BOOST, 0);
#ifdef CONFIG_POGO_PIN
	} else if (pogopin_is_support() && (pogopin_otg_from_buckboost() == true) &&
		pd_state(pd_source_vbus)) {
		hw_pd_power_set_state(PD_POWER_POGOPIN, 0);
#endif /* CONFIG_POGO_PIN */
	} else {
		hw_pd_vbus_event_notify(CHARGER_TYPE_NONE, &vbus_state);
		if (pd_state(pd_source_vbus))
			hw_pd_set_source_sink(POWER_SUPPLY_STOP_SOURCE);
		else
			hw_pd_set_source_sink(POWER_SUPPLY_STOP_SINK);
	}

	mutex_lock(&g_pd_di->sink_vbus_lock);
	if (pd_state(bc12_event) != CHARGER_TYPE_NONE)
		pd_state(ignore_bc12_vbusoff) = true;

	pd_state(ignore_bc12_vbuson) = false;
	pd_state(pd_source_vbus) = false;
	pd_state(pd_finish_flag) = false;
	hw_pd_misc_power_off();
	mutex_unlock(&g_pd_di->sink_vbus_lock);

	hwlog_info("%s -\n", __func__);
}

/* Notify usb attach */
static void pd_report_usb_attach(int new_state)
{
	switch (new_state) {
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
		hw_pd_report_device_attach();
		break;
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		hw_pd_report_host_attach();
		break;
	default:
		break;
	}
}

/* Notify usb dettach */
static void pd_report_usb_detach(int last_state)
{
	switch (last_state) {
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
		hw_pd_report_device_detach();
		break;
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		hw_pd_report_host_detach();
		break;
	default:
		break;
	}
	hw_usb_host_abnormal_event_notify(USB_HOST_EVENT_NORMAL);
}

/* Work for handling usb event, notify usb attach/detach */
static void pd_usb_update_work(struct work_struct *work)
{
	static int last_usb_event = PD_DPM_USB_TYPEC_NONE;
	int new_ev;
	int last_ev;
	struct pd_dpm_info *usb_cb_data = container_of(
		to_delayed_work(work), struct pd_dpm_info, usb_state_update_work);

	mutex_lock(&usb_cb_data->usb_lock);
	new_ev = usb_cb_data->pending_usb_event;
	mutex_unlock(&usb_cb_data->usb_lock);

	hwlog_info("%s: new=%d, last=%d\n", __func__, new_ev, last_usb_event);

	last_ev = last_usb_event;
	if (last_ev == new_ev)
		return;

	switch (new_ev) {
	case PD_DPM_USB_TYPEC_DETACHED:
		pd_report_usb_detach(last_ev);
		break;
	case PD_DPM_USB_TYPEC_DEVICE_ATTACHED:
	case PD_DPM_USB_TYPEC_HOST_ATTACHED:
		if (last_ev != PD_DPM_USB_TYPEC_DETACHED)
			pd_report_usb_detach(last_ev);
		pd_report_usb_attach(new_ev);
		break;
	default:
		return;
	}

	last_usb_event = new_ev;
}

/* Handle usb event update in pd_usb_update_work */
static void hw_pd_update_usb_state(int usb_event)
{
	hwlog_info("%s usb_event:%d, pending_usb_event:%d\n",
		__func__, usb_event, g_pd_di->pending_usb_event);

	if (usb_event != PD_DPM_USB_TYPEC_NONE) {
		mutex_lock(&g_pd_di->usb_lock);
		if (g_pd_di->pending_usb_event != usb_event) {
			cancel_delayed_work(&g_pd_di->usb_state_update_work);
			g_pd_di->pending_usb_event = usb_event;
			queue_delayed_work(g_pd_di->usb_wq, &g_pd_di->usb_state_update_work,
				msecs_to_jiffies(0));
		} else {
			hwlog_info("%s: ignore same event %d\n", __func__, usb_event);
		}
		mutex_unlock(&g_pd_di->usb_lock);
	}
}

int hw_pd_handle_pe_event(unsigned long event, void *data)
{
	int usb_event = PD_DPM_USB_TYPEC_NONE;
	int ret;

	if (!g_pd_di) {
		hwlog_err("%s g_pd_di is null\n", __func__);
		return HW_PD_INVALID_STATE;
	}

	if (pogopin_ignore_typec_event(event, data, pd_state(cur_usb_event))) {
		hwlog_info("pogo inset device ignore typec event:%d", event);
		return 0;
	}

	switch (event) {
	case PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER:
		if (pd_state(abnormal_cc_detection) &&
			(direct_charge_get_stage_status() < DC_STAGE_CHARGE_INIT))
			pd_handle_abnomal_cc_change(PD_DPM_ABNORMAL_CC_CHANGE);
		return 0;

	case PD_DPM_PE_EVT_TYPEC_STATE:
		ret = pd_check_typec_event((struct pd_dpm_typec_state *)data, &usb_event);
		if (ret)
			return ((ret == HW_PD_IGNORE_STATE) ? 0 : ret);
		break;

	case PD_DPM_PE_EVT_PD_STATE:
		break;

	case PD_DPM_PE_EVT_DIS_VBUS_CTRL:
		hwlog_info("%s : Disable VBUS Control  first \n", __func__);
		hw_pd_disable_vbus_control();
		break;

	case PD_DPM_PE_EVT_SINK_VBUS:
		if (data)
			pd_report_sink_vbus(g_pd_di, data);
		break;

	case PD_DPM_PE_EVT_SOURCE_VBUS:
		if (data)
			pd_report_source_vbus(g_pd_di, data);
		break;

	case PD_DPM_PE_EVT_SOURCE_VCONN:
		if (data)
			pd_report_source_vconn(data);
		break;

	case PD_DPM_PE_EVT_DR_SWAP:
		if (data) {
			struct pd_dpm_swap_state *swap_state = data;
			if (swap_state->new_role == PD_ROLE_DFP)
				usb_event = PD_DPM_USB_TYPEC_HOST_ATTACHED;
			else
				usb_event = PD_DPM_USB_TYPEC_DEVICE_ATTACHED;
			pd_set_typec_state(usb_event);
		}
		break;

	case PD_DPM_PE_EVT_PR_SWAP:
		uem_handle_pr_swap_end();
		break;

	case PD_DPM_PE_CABLE_VDO:
		hw_pd_set_cable_vdo(data);
		emark_detect_complete();
		break;
	default:
		hwlog_info("%s: unkonw event\n", __func__);
		break;
	};

	hw_pd_update_usb_state(usb_event);
	return 0;
}

int hw_pd_set_rtb_success(unsigned int sec)
{
	struct pd_dpm_info *di = g_pd_di;
	int typec_state = PD_DPM_USB_TYPEC_NONE;

	/* set board running test result success */
	if (!di || (sec <= 0))
		return HW_PD_INVALID_STATE;

	hw_pd_get_typec_state(&typec_state);
	if ((typec_state == PD_DPM_USB_TYPEC_HOST_ATTACHED) ||
		(typec_state == PD_DPM_USB_TYPEC_DEVICE_ATTACHED) ||
		(typec_state == PD_DPM_USB_TYPEC_AUDIO_ATTACHED)) {
		hwlog_err("typec port is attached, can not open vbus\n");
		return HW_PD_INVALID_STATE;
	}

	hw_pd_power_vbus_ch_open();
	cancel_delayed_work_sync(&di->otg_restore_work);
	/* 1000: delay val*1000 ms */
	schedule_delayed_work(&di->otg_restore_work,
		msecs_to_jiffies(sec * 1000));

	return 0;
}

int hw_pd_get_otg_channel(void)
{
	if (pogopin_is_support() && (pogopin_otg_from_buckboost() == true))
		return 1; /* 1: pogopin is otg mode */

	return pd_state(otg_channel);
}

/* work to reset pd chip */
static void pd_reinit_work(struct work_struct *work)
{
	struct pd_dpm_info *di =
		container_of(work, struct pd_dpm_info, reinit_pd_work.work);

	if (!di)
		return;
	hwlog_info("pd_reinit_work start\n");

	di->is_ocp = true;
	if (di->vdd_ocp_lock->active)
		__pm_relax(di->vdd_ocp_lock);

	(void)regulator_disable(di->pd_vdd_ldo);
	msleep(50); /* Sleep 50ms */
	(void)regulator_enable(di->pd_vdd_ldo);
	hw_pd_reinit_chip();
	di->is_ocp = false;
	hwlog_info("pd_reinit_work end\n");
}

/* Callback function to handle over-charge-protect event */
static void pd_ocp_cb(unsigned int type, char *event_data)
{
	struct pd_dpm_info *di = g_pd_di;
	static unsigned int count;

	(void)type;
	(void)event_data;

	if (!di || !di->pd_reinit_enable)
		return;

	if (count > di->max_ocp_count)
		return;

	count++;

	if (!di->vdd_ocp_lock->active)
		__pm_stay_awake(di->vdd_ocp_lock);

	schedule_delayed_work(&di->reinit_pd_work, msecs_to_jiffies(di->ocp_delay_time));
}

/* Init work for pd chip reinit. Do reinit while some bad thing like over-charge occured */
static void pd_reinit_process(struct pd_dpm_info *di)
{
	int ret;

	if (!di->pd_reinit_enable)
		return;

	di->pd_vdd_ldo = devm_regulator_get(di->dev, "pd_vdd");
	if (IS_ERR(di->pd_vdd_ldo)) {
		hwlog_err("get pd vdd ldo failed\n");
		return;
	}

	(void)regulator_enable(di->pd_vdd_ldo);
	di->vdd_ocp_lock = wakeup_source_register(di->dev, "vdd_ocp_lock");
	if (!di->vdd_ocp_lock) {
		hwlog_err("%s wakeup source register failed\n", __func__);
		return;
	}

	INIT_DELAYED_WORK(&di->reinit_pd_work, pd_reinit_work);
	ret = hw_pd_event_reg(HW_PD_EVENT_TYPE_OCP, di->ldo_name, pd_ocp_cb);
	if (ret) {
		hwlog_err("%s reg ocp event failed\n", __func__);
		di->pd_reinit_enable = 0;
	}
}

/* Init some value while system boot */
static void pd_init_default_value(struct pd_dpm_info *di)
{
	pd_state(cur_typec_state) = PD_DPM_USB_TYPEC_DETACHED;
	pd_state(bc12_event) = hw_pd_power_get_chgtype();
	pd_state(charger_type_event) = pd_state(bc12_event);
	di->pending_usb_event = PD_DPM_USB_TYPEC_NONE;
}

/* work to close vbus in otg mode */
static void pd_otg_restore_work(struct work_struct *work)
{
	struct pd_dpm_info *di = g_pd_di;

	if (!di)
		return;

	hw_pd_power_vbus_ch_close();
}

/* Check whether ignore bc12 event. Return non-zero value to ignore current event */
static int ignore_bc12_charge_type(unsigned long event)
{
	if (pd_state(pd_source_vbus)) {
		hwlog_info("%s : line %d\n", __func__, __LINE__);
		return HW_PD_INVALID_STATE;
	}
	if (pd_state(ignore_bc12_vbuson) && (event == CHARGER_TYPE_NONE)) {
		hwlog_info("%s : bc1.2 event not match\n", __func__);
		return HW_PD_INVALID_STATE;
	}
	if (!pmic_vbus_irq_is_enabled() &&
		((pd_dpm_get_source_sink_state() == POWER_SUPPLY_STOP_SINK) && (event != CHARGER_TYPE_NONE))) {
		hwlog_info("%s : pd aready in STOP_SINK state,"
			"but bc1.2 event not CHARGER_TYPE_NONE, ignore\n", __func__);
		return HW_PD_INVALID_STATE;
	}
	if (!pmic_vbus_irq_is_enabled() && (event == CHARGER_TYPE_NONE) &&
		(!pogopin_is_support() || !pogopin_otg_from_buckboost())) {
		hwlog_info("%s : ignore CHARGER_TYPE_NONE\n", __func__);
		return HW_PD_INVALID_STATE;
	}

	return 0;
}

/* Send bc12 detect resut to other module like charger-driver */
static void pd_report_bc12_to_charger(unsigned long *event, unsigned long *last_event, void *data)
{
	if ((!pd_state(ignore_bc12_vbusoff) && (*event == CHARGER_TYPE_NONE)) ||
		(!pd_state(ignore_bc12_vbuson) && (*event != CHARGER_TYPE_NONE))) {
		if ((!pd_state(pd_finish_flag) && (*last_event != *event)) ||
			(pd_state(pd_finish_flag) && (*event == CHARGER_TYPE_DCP))) {
			hwlog_info("%s : notify event = %lu\n", __func__, *event);
			if (pd_state(usb_audio_state) == PD_DPM_TYPEC_ATTACHED_AUDIO) {
				*event = CHARGER_TYPE_SDP;
				data = NULL;
			}
			*last_event = *event;
			hw_pd_vbus_event_notify(*event, data);
		} else {
			hwlog_info("%s : ignore, current event=%lu, last event=%lu\n",
				__func__, *event, *last_event);
		}
	}
}

/* Callback function to process bc12 detect result event */
static void pd_report_bc12_cb(unsigned int type, char *event_data)
{
	struct pd_dpm_info *di = g_pd_di;
	static unsigned long last_event = CHARGER_TYPE_NONE;
	unsigned long event;
	void *data = NULL;
	int ret;

	if (!event_data) {
		hwlog_err("%s : invalid arg\n", __func__);
		return;
	}

	event = ((struct bc12_event *)event_data)->event;
	data = ((struct bc12_event *)event_data)->data;
	hwlog_info("%s : received event = %lu\n", __func__, event);

	if (pmic_vbus_is_connected()) {
		hw_pd_vbus_event_notify(event, data);
		return;
	}

	if ((event == CHARGER_TYPE_NONE) && !pd_state(pd_finish_flag) && !pd_state(pd_source_vbus)) {
		hwlog_info("%s : PD_WAKE_UNLOCK \n", __func__);
		hw_pd_wakelock_ctrl(PD_WAKE_UNLOCK);
	}

	hwlog_info("[sn]%s : bc12on %d,bc12off %d\n", __func__,
		pd_state(ignore_bc12_vbuson), pd_state(ignore_bc12_vbusoff));

	mutex_lock(&g_pd_di->sink_vbus_lock);
	pd_state(bc12_event) = event;

	if (event == PLEASE_PROVIDE_POWER) {
		mutex_unlock(&g_pd_di->sink_vbus_lock);
		return;
	}

	ret = ignore_bc12_charge_type(event);
	if (ret != 0)
		goto End;

	pd_report_bc12_to_charger(&event, &last_event, data);

End:
	if (event == CHARGER_TYPE_NONE) {
		last_event = CHARGER_TYPE_NONE;
		pd_state(ignore_bc12_vbusoff) = false;
		pd_state(ignore_bc12_vbuson) = false;
	}
	mutex_unlock(&g_pd_di->sink_vbus_lock);

	return;
}

/* Init resources and some other work while new instance is registered */
static void pd_init_resource(struct pd_dpm_info *di)
{
	int ret;

	mutex_init(&di->sink_vbus_lock);
	mutex_init(&di->usb_lock);
	BLOCKING_INIT_NOTIFIER_HEAD(&di->pd_evt_nh);
	BLOCKING_INIT_NOTIFIER_HEAD(&di->pd_port_status_nh);
	ATOMIC_INIT_NOTIFIER_HEAD(&di->pd_wake_unlock_evt_nh);

	di->usb_wq = create_workqueue("pd_dpm_usb_wq");
	INIT_DELAYED_WORK(&di->usb_state_update_work, pd_usb_update_work);
	INIT_DELAYED_WORK(&di->cc_moisture_flag_restore_work, cc_moisture_restore_work);
	INIT_DELAYED_WORK(&di->cc_short_work, pd_cc_short_work);

	/* use for board rt test */
	INIT_DELAYED_WORK(&di->otg_restore_work, pd_otg_restore_work);

	/* Register calback function for bc1.2 notification */
	hw_pd_event_reg(HW_PD_EVENT_TYPE_BC12, NULL, pd_report_bc12_cb);

	/* Register calback function for fb unblank notification */
	INIT_WORK(&g_pd_di->fb_work, fb_unblank_work);
	if (pd_state(abnormal_cc_detection))
		hw_pd_event_reg(HW_PD_EVENT_TYPE_FB, NULL, fb_unblank_proc_cb);
}

void hw_pd_register_instance(void *context)
{
	g_pd_di = (struct pd_dpm_info *)context;
	if (!g_pd_di) {
		hwlog_err("%s invalid param\n", __func__);
		return;
	}

	pd_init_default_value(g_pd_di);

	hw_pd_power_init();
	hw_pd_sysfs_create();
	hw_pd_usb_connect_init();

	pd_init_resource(g_pd_di);
	pd_reinit_process(g_pd_di);

	hwlog_info("%s end\n", __func__);
}

