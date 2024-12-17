/*
 * hw_pd_state.c
 *
 * Source file of global state record of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <chipset_common/hwusb/hw_pd/hw_pd_state.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_interface.h>
#include <huawei_platform/log/hw_log.h>
#include <securec.h>

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_pd_state
HWLOG_REGIST();
#endif /* HWLOG_TAG */

static struct hw_pd_state_info g_hw_pd_info = {
	._abnormal_cc_interval = PD_DPM_CC_CHANGE_INTERVAL,
	._product_type = -1,
	._support_dp = 1,
	._support_analog_audio = 1,
	._switch_manual_enable = 1,
	._usb_audio_state = PD_DPM_USB_TYPEC_NONE,
	._sink_source_type = POWER_SUPPLY_STOP_SINK,
	._remote_rp_level = PD_DPM_CC_VOLT_OPEN,
};

void hw_pd_get_charge_state(unsigned long *event, struct pd_dpm_vbus_state *state)
{
	hwlog_info("%s event =%ld\n", __func__, pd_state(charger_type_event));

	if (event)
		*event = pd_state(charger_type_event);
	if (state)
		(void)memcpy_s(state, sizeof(struct pd_dpm_vbus_state),
			&pd_state(g_vbus_state), sizeof(struct pd_dpm_vbus_state));
}

void hw_pd_set_charge_state(unsigned long event, struct pd_dpm_vbus_state *state)
{
	hwlog_info("%s event =%ld\n", __func__, event);

	if (state)
		(void)memcpy_s(&pd_state(g_vbus_state), sizeof(struct pd_dpm_vbus_state),
			state, sizeof(struct pd_dpm_vbus_state));
	pd_state(charger_type_event) = event;
}

struct hw_pd_state_info *hw_pd_state_get_context(void)
{
	return &g_hw_pd_info;
}

int hw_pd_state_dump(char *buf, int size)
{
	bool *bptr = (bool *)&g_hw_pd_info._high_power_charging;
	unsigned long *ulptr = &g_hw_pd_info._charger_type_event;
	unsigned int *uiptr = &g_hw_pd_info._abnormal_cc_detection;
	int *iptr = &g_hw_pd_info._emark_detect_enable;
	int ret = 0;

	ret += scnprintf((buf + ret), (size - ret), "\n--->dump pd state start");

	ret += scnprintf((buf + ret), (size - ret), "\nshow bool state:");
	for (; bptr < &g_hw_pd_info._b_stub; bptr++)
		ret += scnprintf((buf + ret), (size - ret), " %d", (int)(*bptr));

	ret += scnprintf((buf + ret), (size - ret), "\nshow ul state:");
	for (; ulptr < &g_hw_pd_info._ul_stub; ulptr++)
		ret += scnprintf((buf + ret), (size - ret), " %d", (int)(*ulptr));

	ret += scnprintf((buf + ret), (size - ret), "\nshow ui state:");
	for (; uiptr < &g_hw_pd_info._ui_stub; uiptr++)
		ret += scnprintf((buf + ret), (size - ret), " %d", (int)(*uiptr));

	ret += scnprintf((buf + ret), (size - ret), "\nshow int state:");
	for (; iptr < &g_hw_pd_info._i_stub; iptr++)
		ret += scnprintf((buf + ret), (size - ret), " %d", (int)(*iptr));

	ret += scnprintf((buf + ret), (size - ret),
		"\nsink_source_type:%d", (int)g_hw_pd_info._sink_source_type);
	ret += scnprintf((buf + ret), (size - ret),
		"\nremote_rp_level:%d", (int)g_hw_pd_info._remote_rp_level);
	ret += scnprintf((buf + ret), (size - ret),
		"\nvbus_state:%d %d %d %d %d", g_hw_pd_info._g_vbus_state.mv,
		g_hw_pd_info._g_vbus_state.ma, (int)g_hw_pd_info._g_vbus_state.vbus_type,
		(int)g_hw_pd_info._g_vbus_state.ext_power, g_hw_pd_info._g_vbus_state.remote_rp_level);
	ret += scnprintf((buf + ret), (size - ret), "\ncable_info:%d %d",
		(int)g_hw_pd_info._cable_info.cable_vdo,
		(int)g_hw_pd_info._cable_info.cable_vdo_ext);

	ret += scnprintf((buf + ret), (size - ret), "\n<---dump pd state finish\n");

	return ret;
}

