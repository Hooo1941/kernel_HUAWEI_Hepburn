/*
 * hw_pd_ops.h
 *
 * Header file of external operation interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HW_PD_OPS_H_
#define _HW_PD_OPS_H_

struct pd_dpm_ops {
	void (*pd_dpm_detect_emark_cable)(void *client); /* start detect emark cable(c2c) */
	bool (*pd_dpm_get_hw_dock_svid_exist)(void *client); /* get if device has huawei svid */
	int (*pd_dpm_notify_direct_charge_status)(void *client, bool mode); /* get direct charge status(in or out) */
	void (*pd_dpm_set_cc_mode)(int mode); /* set cc mode(DFP, UFP) */
	int (*pd_dpm_get_cc_state)(void); /* get cc mode(DFP, UFP) */
	int (*pd_dpm_disable_pd)(void *client, bool disable); /* disable pd state machine */
	bool (*pd_dpm_check_cc_vbus_short)(void); /* check if cc vbus short */
	void (*pd_dpm_enable_drp)(int mode); /* enable typec drp */
	void (*pd_dpm_reinit_chip)(void *client); /* reinit typec chip config */
	int (*data_role_swap)(void *client); /* start data role swap */
	bool (*pd_dpm_inquire_usb_comm)(void *client); /* Obtain the communication capability of the PD */
};

struct cc_check_ops {
	int (*is_cable_for_direct_charge)(void);
};

struct water_det_ops {
	int (*is_water_detected)(void);
};

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
struct cable_vdo_ops {
	int (*is_cust_src2_cable)(void);
};
#endif

/*
 * Following funtions are used for calling extern interfaces
 * (defined in pd_dpm_ops) for operating to PD/TYPEC chip.
 */
void hw_pd_start_data_role_swap(void); /* data_role_swap */
void hw_pd_emark_detect(void); /* pd_dpm_detect_emark_cable */
void hw_pd_emark_finish(void); /* pd_dpm_inquire_usb_comm & data_role_swap */
int hw_pd_disable_pd_comm(bool disable); /* pd_dpm_disable_pd */
int hw_pd_set_cc_mode(int mode); /* pd_dpm_set_cc_mode */
void hw_pd_set_drp_state(int mode); /* pd_dpm_enable_drp */
void hw_pd_reinit_chip(void); /* pd_dpm_reinit_chip */
bool hw_pd_get_hw_docksvidexist(void); /* pd_dpm_get_hw_dock_svid_exist */
int hw_pd_set_dc_status(bool dc); /* pd_dpm_notify_direct_charge_status */
int hw_pd_query_cc_state(unsigned int *cc); /* pd_dpm_get_cc_state */
int hw_pd_check_cc_short(bool *check); /* pd_dpm_check_cc_vbus_short */
int hw_pd_reg_dpm_ops(struct pd_dpm_ops *ops, void *client); /* Reg new callback functions */

/*
 * Following funtions are used for calling extern interfaces
 * (defined in water_det_ops) for doing wather check operation.
 */
int hw_pd_check_wather_detect(void); /* is_water_detected */
int hw_pd_reg_water_ops(struct water_det_ops* ops); /* Reg new callback functions */

/*
 * Following funtions are used for calling extern interfaces (defined
 * in cable_vdo_ops) for checking whether cable is direct charge.
 */
int hw_pd_dc_cable_detect(void); /* is_cable_for_direct_charge */
int hw_pd_reg_cc_ops(struct cc_check_ops* ops); /* Reg new callback functions */

/*
 * Following funtions are used for calling extern interfaces for
 * (defined in cc_check_ops) checking whether cable is custom designed.
 */
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
int hw_pd_reg_cablevdo_ops(struct cable_vdo_ops *ops); /* Reg new callback functions */
bool hw_pd_is_custsrc2_cable(void); /* is_cust_src2_cable */
#endif

#endif

