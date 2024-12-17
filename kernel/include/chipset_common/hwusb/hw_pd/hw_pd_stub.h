/*
 * hw_pd_stub.h
 *
 * Header file of stub macro of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HW_PD_STUB_H_
#define _HW_PD_STUB_H_

#define pd_dpm_set_audio_power_no_hs_state(val) (pd_state(audio_power_no_hs) = (val))
#define pd_dpm_get_cc_voltage() (pd_state(remote_rp_level))
#define pd_dpm_ignore_vbuson_event() (pd_state(ignore_vbuson))
#define pd_dpm_ignore_vbusoff_event() (pd_state(ignore_vbusoff))
#define pd_dpm_set_ignore_vbuson_event(val) (pd_state(ignore_vbuson) = (val))
#define pd_dpm_set_ignore_vbusoff_event(val) (pd_state(ignore_vbusoff) = (val))
#define pd_dpm_set_ignore_bc12_event_when_vbuson(val) (pd_state(ignore_bc12_vbuson) = (val))
#define pd_dpm_get_high_power_charging_status() (pd_state(high_power_charging))
#define pd_dpm_get_high_voltage_charging_status() (pd_state(high_voltage_charging))
#define pd_dpm_get_optional_max_power_status() (pd_state(optional_max_power))
#define pd_dpm_set_optional_max_power_status(val) (pd_state(optional_max_power) = (val))
#define pd_dpm_set_emark_detect_enable(val) (pd_state(emark_detect_enable) = (val))
#define pd_dpm_get_emark_detect_enable() (pd_state(emark_detect_enable))
#define get_abnormal_cc_detection() (pd_state(abnormal_cc_detection))
#define pd_dpm_get_cc_orientation() (pd_state(cc_orientation))
#define pd_dpm_ignore_vbus_only_event(val) (pd_state(ignore_vbus_only) = (val))
#define pd_set_product_type(val) (pd_state(product_type) = (val))
#define pd_dpm_get_source_sink_state() (pd_state(sink_source_type))
#define pd_dpm_get_cc_moisture_status() (pd_state(cc_moisture_status))
#define pd_dpm_support_dp() (pd_state(support_dp))
#define pd_dpm_get_pd_source_vbus() (pd_state(pd_source_vbus))
#define pd_dpm_get_ctc_cable_flag() (pd_state(ctc_cable_flag))
#define pd_dpm_get_pd_finish_flag() (pd_state(pd_finish_flag))
#define pd_dpm_set_pd_finish_flag(val) (pd_state(pd_finish_flag) = (val))
#define pd_dpm_get_analog_hs_state() (pd_state(analog_hs_state))
#define pd_dpm_get_combophy_ready_status() (pd_state(combophy_ready_flag))
#define pd_dpm_get_combphy_status() (pd_state(combphy_mode))
#define pd_dpm_get_last_hpd_status() (pd_state(last_hpd_status))

#define register_pd_dpm_notifier hw_pd_reg_event_notifier
#define unregister_pd_dpm_notifier hw_pd_unreg_event_notifier
#define unregister_pd_wake_unlock_notifier hw_pd_unreg_wakeunlock_notifier
#define register_pd_wake_unlock_notifier hw_pd_reg_wakeunlock_notifier
#define unregister_pd_dpm_portstatus_notifier hw_pd_unreg_port_notifier
#define register_pd_dpm_portstatus_notifier hw_pd_reg_port_notifier
#define pd_dpm_handle_pe_event hw_pd_handle_pe_event
#define pd_dpm_get_typec_state hw_pd_get_typec_state
#define pd_dpm_set_max_power hw_pd_set_max_power
#define pd_dpm_disable_pd(d) hw_pd_disable_pd(d)
#define pd_dpm_set_cable_vdo hw_pd_set_cable_vdo
#define pd_set_product_id_info hw_pd_set_product_id
#define pd_dpm_get_otg_channel hw_pd_get_otg_channel
#define pd_dpm_detect_emark_cable_finish() hw_pd_emark_detect_finish()
#define pd_dpm_detect_emark_cable() hw_pd_emark_detect()
#define pd_dpm_cc_dynamic_protect hw_pd_cc_dynamic_protect
#define pd_dpm_set_source_sink_state hw_pd_set_source_sink
#define pd_dpm_wakelock_ctrl hw_pd_wakelock_ctrl
#define pd_dpm_vbus_ctrl hw_pd_vbus_ctrl
#define pd_judge_is_cover hw_pd_judge_is_cover
#define pd_dpm_get_cvdo_cur_cap hw_pd_get_vdo_cap
#define pd_dpm_check_cc_vbus_short() hw_pd_check_ccvbus_short()
#define pd_dpm_vbus_notifier_call hw_pd_vbus_event_notify
#define pd_dpm_support_cable_auth hw_pd_support_cable_auth
#define pd_dpm_set_combophy_ready_status() hw_pd_set_combophy_ready_status()
#define pd_dpm_set_combphy_status(mode) hw_pd_set_combphy_mode(mode)
#define pd_dpm_set_last_hpd_status(status) hw_pd_set_last_hpd_status(status)
#define pd_dpm_get_pd_event_num() hw_pd_get_combphy_event_num()
#define pd_dpm_handle_combphy_event(event) hw_pd_handle_combphy_event(event)
#define pd_pdm_enable_drp hw_pd_enable_drp
#define pd_dpm_start_data_role_swap hw_pd_start_data_role_swap
#define pd_dpm_get_hw_dock_svid_exist() hw_pd_get_hw_docksvidexist()
#define pd_dpm_reinit_chip() hw_pd_reinit_chip()
#define pd_dpm_ops_register hw_pd_reg_dpm_ops
#define water_det_ops_register hw_pd_reg_water_ops
#define cc_check_ops_register hw_pd_reg_cc_ops
#define pd_dpm_cable_vdo_ops_register hw_pd_reg_cablevdo_ops
#define pd_dpm_notify_direct_charge_status(dc) hw_pd_notify_dc_status(dc)
#define pd_dpm_get_charge_event hw_pd_get_charge_state
#define dp_dfp_u_notify_dp_configuration_done hw_pd_combphy_config_done
#define pd_dpm_usb_host_on() hw_pd_usb_host_on()
#define pd_dpm_usb_host_off() hw_pd_usb_host_off()

#endif

