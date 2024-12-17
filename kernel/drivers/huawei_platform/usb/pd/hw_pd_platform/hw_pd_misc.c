/*
 * hw_pd_misc.c
 *
 * Source file of misc device interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/usb/hw_pd/hw_pd_misc.h>
#include <huawei_platform/usb/hw_pd/hw_pd_power.h>

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <devkit/audiokit/ana_hs_kit/ana_hs.h>
#else
#include "ana_hs_kit/ana_hs.h"
#endif
#include <huawei_platform/audio/usb_analog_hs_interface.h>
#include <huawei_platform/audio/usb_audio_power.h>
#include <huawei_platform/audio/usb_audio_power_v600.h>
#include <huawei_platform/audio/dig_hs.h>
#include <huawei_platform/audio/ana_hs_extern_ops.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_pd_misc
HWLOG_REGIST();
#endif /* HWLOG_TAG */

/* ignore typec event while charging */
static bool pogo_charger_ignore_typec_event(unsigned long event)
{
	return ((pogopin_5pin_get_pogo_status() == POGO_CHARGER) &&
		((event != PD_DPM_PE_EVT_TYPEC_STATE) &&
		(event != PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER))) ?
		true : false;
}

/* ignore typec event while otg */
static bool pogo_otg_ignore_typec_event(unsigned long event)
{
	return ((event == PD_DPM_PE_EVT_SOURCE_VBUS) ||
		(event == PD_DPM_PE_EVT_SOURCE_VCONN) ||
		(event == PD_DPM_PE_EVT_DR_SWAP) ||
		(event == PD_DPM_PE_EVT_PR_SWAP) ||
		(event == PD_DPM_PE_CABLE_VDO)) ? true : false;
}

/* checking the now_event status in audio mode */
static bool ana_audio_event(struct pd_dpm_typec_state *typec_state, int cur_usb_event)
{
	if (!typec_state)
		return false;

	return ((typec_state->new_state == PD_DPM_TYPEC_ATTACHED_AUDIO) ||
		((typec_state->new_state == PD_DPM_TYPEC_UNATTACHED) &&
		(cur_usb_event == PD_DPM_TYPEC_ATTACHED_AUDIO))) ?
		true : false;
}

/* checking the now_event status in otg mode */
static bool pogo_otg_typec_snk_event(struct pd_dpm_typec_state *typec_state, int cur_usb_event)
{
	if (!typec_state)
		return false;

	return (((typec_state->new_state == PD_DPM_TYPEC_ATTACHED_SNK) ||
		((typec_state->new_state == PD_DPM_TYPEC_UNATTACHED) &&
		(cur_usb_event == PD_DPM_TYPEC_ATTACHED_SNK))) &&
		(pogopin_5pin_get_pogo_status() == POGO_OTG)) ? true : false;
}

bool pogopin_ignore_typec_event(unsigned long event, void *data, int cur_usb_event)
{
	struct pd_dpm_typec_state *typec_state = NULL;

	if (!pogopin_typec_chg_ana_audio_support() ||
		(pogopin_5pin_get_pogo_status() == POGO_NONE) ||
		!data)
		return false;

	if (pogo_charger_ignore_typec_event(event)) {
		return true;
	} else if ((pogopin_5pin_get_pogo_status() == POGO_OTG) &&
		(event != PD_DPM_PE_EVT_TYPEC_STATE)) {
		if (pogo_otg_ignore_typec_event(event))
			return true;
		else
			return false;
	}

	typec_state = data;

	return (ana_audio_event(typec_state, cur_usb_event) ||
		pogo_otg_typec_snk_event(typec_state, cur_usb_event)) ? false : true;
}

void hw_pd_misc_power_off(void)
{
	usb_audio_power_scharger();
	usb_headset_plug_out();
	restore_headset_state();
	dig_hs_plug_out();
}

void hw_pd_misc_typec_state(int detach)
{
	if (detach) {
		hwlog_info("%s report detach, stop ovp & start res detect\n", __func__);
		ana_hs_fsa4480_stop_ovp_detect(ANA_HS_TYPEC_DEVICE_DETACHED);
		ana_hs_fsa4480_start_res_detect(ANA_HS_TYPEC_DEVICE_DETACHED);
	} else {
		hwlog_info("%s report attach, stop res & satrt ovp detect\n", __func__);
		ana_hs_fsa4480_stop_res_detect(ANA_HS_TYPEC_DEVICE_ATTACHED);
		ana_hs_fsa4480_start_ovp_detect(ANA_HS_TYPEC_DEVICE_ATTACHED);
	}
}

void hw_pd_misc_notify_analog(int state)
{
	if (state) {
		usb_analog_hs_plug_in_out_handle(ANA_HS_PLUG_IN);
		ana_hs_plug_handle(ANA_HS_PLUG_IN);
	} else {
		usb_analog_hs_plug_in_out_handle(ANA_HS_PLUG_OUT);
		ana_hs_plug_handle(ANA_HS_PLUG_OUT);
	}
}

int hw_pd_misc_pre_handle_analog(enum ANALOG_STATE state)
{
	/* do not support analog audio but earphone is plugin */
	static int analog_audio_status = 0;

	hwlog_info("%s event %d\n", __func__, state);
	if (pd_state(support_analog_audio)) {
		hwlog_info("%s handle analog normally\n", __func__);
		return 0;
	}

	if (state == ANALOG_ATTACH) {
		analog_audio_status = 1;
		power_event_bnc_notify(POWER_BNT_HW_USB, POWER_NE_HW_USB_HEADPHONE, NULL);
		hwlog_err("%s post attch\n", __func__);
		return HW_PD_INVALID_STATE;
	} else if ((state == ANALOG_DETTACH) && analog_audio_status) {
		analog_audio_status = 0;
		power_event_bnc_notify(POWER_BNT_HW_USB, POWER_NE_HW_USB_HEADPHONE_OUT, NULL);
		hwlog_err("%s post dettach\n", __func__);
	}

	return 0;
}

int hw_pd_misc_handle_analog(enum ANALOG_STATE state)
{
	hwlog_info("%s event=%d, state=%d\n", __func__, state, pd_state(analog_hs_state));

	if (state == ANALOG_DETTACH) {
		if (pd_state(analog_hs_state) == 1) {
			pd_state(analog_hs_state) = 0;
			pogopin_5pin_set_ana_audio_status(false);
			hwlog_info("%s process dettach\n", __func__);
			return 0;
		}
	} else if (state == ANALOG_ATTACH) {
		pd_state(analog_hs_state) = 1;
		pogopin_5pin_set_ana_audio_status(true);
		hwlog_info("%s process attch\n", __func__);
		return 1;
	} else if (state == ANALOG_DISABLE) {
		pd_state(analog_hs_state) = 0;
	}

	return HW_PD_INVALID_STATE;
}

void hw_pd_misc_handle_wireless_cover_event(
	unsigned int vid, unsigned int pid, unsigned int bcd)
{
	int pd_product_type;

	if (bcd == PD_PID_COVER_ONE)
		pd_product_type = PD_PDT_WIRELESS_COVER;
	else if (bcd == PD_PID_COVER_TWO)
		pd_product_type = PD_PDT_WIRELESS_COVER_TWO;
	else
		return;

	pd_set_product_type(pd_product_type);
	hwlog_info("%s vid = 0x%x, pid = 0x%x, type = 0x%x, bcd = 0x%x\n",
		__func__, vid, pid, pd_product_type, bcd);

	switch (pd_product_type) {
	case PD_PDT_WIRELESS_COVER:
		if ((vid == PD_DPM_HW_VID) && (pid == PD_DPM_HW_CHARGER_PID)) {
			coul_drv_charger_event_rcv(WIRELESS_COVER_DETECTED);
			pd_send_wireless_cover_uevent();
		}
		break;
	case PD_PDT_WIRELESS_COVER_TWO:
		if ((vid == PD_DPM_HW_VID) && (pid == PD_DPM_HW_CHARGER_PID)) {
			coul_drv_charger_event_rcv(WIRELESS_COVER_DETECTED);
			pd_send_wireless_cover_uevent();
		}
		uvdm_schedule_work();
		break;
	default:
		hwlog_err("undefined type %d\n", pd_product_type);
		break;
	}
}
