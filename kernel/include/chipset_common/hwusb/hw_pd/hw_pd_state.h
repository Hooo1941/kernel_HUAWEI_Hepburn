/*
 * hw_pd_state.h
 *
 * Header file of global state record of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef HW_PD_STATE_H
#define HW_PD_STATE_H

#include <linux/types.h>
#include <linux/of.h>
#include <linux/power_supply.h>

/* used to describe vbus status reported from policy engine */
struct pd_dpm_vbus_state {
	int mv; /* vbus value, unit mv */
	int ma; /* ibus value, unit ma */
	uint8_t vbus_type; /* vbus type: typec or pd */
	bool ext_power; /* remote device is self powered or external powered */
	int remote_rp_level; /* remote device cc rp level */
};

enum pd_dpm_cc_voltage_type {
	PD_DPM_CC_VOLT_OPEN = 0,
	PD_DPM_CC_VOLT_RA = 1,
	PD_DPM_CC_VOLT_RD = 2,

	PD_DPM_CC_VOLT_SNK_DFT = 5,
	PD_DPM_CC_VOLT_SNK_1_5 = 6,
	PD_DPM_CC_VOLT_SNK_3_0 = 7,

	PD_DPM_CC_DRP_TOGGLING = 15,
};

struct pd_dpm_cable_info {
	unsigned int cable_vdo;
	unsigned int cable_vdo_ext;
};

struct hw_pd_state_info {
	bool _high_power_charging;
	bool _high_voltage_charging;
	bool _optional_max_power;
	bool _optional_wireless_cover;
	bool _ignore_vbuson;
	bool _ignore_vbusoff;
	bool _ignore_bc12_vbuson;
	bool _ignore_bc12_vbusoff;
	bool _cc_moisture_status;
	bool _cc_moisture_happened;
	bool _audio_power_no_hs;
	bool _cc_orientation;
	bool _smart_holder;
	bool _ignore_vbus_only;
	bool _vbus_only_status;
	bool _pd_source_vbus;
	bool _ctc_cable_flag;
	bool _pd_finish_flag;
	bool _combophy_ready_flag;
	bool _last_hpd_status;
	bool _b_stub;

	unsigned long _charger_type_event;
	unsigned long _bc12_event;
	unsigned long _ul_stub;

	unsigned int _abnormal_cc_detection;
	unsigned int _abnormal_cc_interval;
	unsigned int _cc_dynamic_protect;
	unsigned int _cc_abnormal_dmd;
	unsigned int _ui_stub;

	int _emark_detect_enable;
	int _product_type;
	int _support_smart_holder;
	int _cc_orientation_factory;
	int _support_dp;
	int _support_analog_audio;
	int _cc_detect_moisture;
	int _cc_moisture_report;
	int _external_pd_flag;
	int _switch_manual_enable;
	int _otg_channel;
	int _emark_finish_disable;
	int _ibus_check;
	int _vbus_only_min_voltage;
	int _usb_audio_state;
	int _cur_usb_event;
	int _cur_typec_state;
	int _analog_hs_state;
	int _combphy_mode;
	int _i_stub;

	enum power_supply_sinksource_type _sink_source_type;
	enum pd_dpm_cc_voltage_type _remote_rp_level;
	struct pd_dpm_vbus_state _g_vbus_state;
	struct pd_dpm_cable_info _cable_info;
};

/* Get the pointer to global state context of PD driver */
struct hw_pd_state_info *hw_pd_state_get_context(void);
#define pd_state(param) (hw_pd_state_get_context()->_##param)

/* Get the state of current charger. event: type of charger module; state: detaild charger info */
void hw_pd_get_charge_state(unsigned long *event, struct pd_dpm_vbus_state *state);

/* Set the state of current charger. event: type of charger module; state: detaild charger info */
void hw_pd_set_charge_state(unsigned long event, struct pd_dpm_vbus_state *state);

/* dump the state data, return the size of buffer used to store dump datas */
int hw_pd_state_dump(char *buf, int size);

#endif

