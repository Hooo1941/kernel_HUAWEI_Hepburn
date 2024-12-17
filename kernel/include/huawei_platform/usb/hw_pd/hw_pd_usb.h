/*
 * hw_pd_usb.h
 *
 * Header file of usb interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef HW_PD_USB_H
#define HW_PD_USB_H

#ifdef CONFIG_CONTEXTHUB_PD
#include <linux/hisi/usb/tca.h>
#endif

/* context of combphy, used to describe the event to control combphy, usb phy and dp phy */
struct pd_dpm_combphy_event {
	TCA_IRQ_TYPE_E irq_type;
	TCPC_MUX_CTRL_TYPE mode_type;
	TCA_DEV_TYPE_E dev_type;
	TYPEC_PLUG_ORIEN_E typec_orien;
};

/* send usb mode is host_on event to combphy */
void hw_pd_usb_host_on(void);
/* send usb mode is host_off event to combphy */
void hw_pd_usb_host_off(void);
/* config done to complete combphy */
void hw_pd_combphy_config_done(TCPC_MUX_CTRL_TYPE mode_type, int ack);
void hw_pd_usb_connect_init(void);

#ifdef CONFIG_CONTEXTHUB_PD
/* set combophy ready flag */
void hw_pd_set_combophy_ready_status(void);
/* set last_hpd_status for host_detach to send DP_CABLE_OUT_EVENT */
void hw_pd_set_last_hpd_status(bool hpd_status);
/* send now state event to combphy */
void hw_pd_set_combphy_mode(TCPC_MUX_CTRL_TYPE mode);
/* compare last_event and new_event, ignore invaild event, send event to combphy */
int hw_pd_handle_combphy_event(struct pd_dpm_combphy_event event);
#else
static inline void hw_pd_set_combophy_ready_status(void) {}
static inline void hw_pd_set_last_hpd_status(bool hpd_status) {}
static inline void hw_pd_set_combphy_mode(TCPC_MUX_CTRL_TYPE mode) {}
static inline int hw_pd_handle_combphy_event(struct pd_dpm_combphy_event event) {return 0;}
#endif /* CONFIG_CONTEXTHUB_PD */

#if defined(CONFIG_CONTEXTHUB_PD) && (defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS))
/* get combphy queue evnet number */
int hw_pd_get_combphy_event_num(void);
#else
static inline int hw_pd_get_combphy_event_num(void)
{
	return 0;
}
#endif /* CONFIG_CONTEXTHUB_PD && (CONFIG_TCPC_CLASS || CONFIG_HW_TCPC_CLASS) */

#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
/* usb mode is device and send charger connect event */
void hw_pd_report_device_attach(void);
/* usb mode is host and send charger connect event */
void hw_pd_report_host_attach(void);
/* usb mode is device and send charger disconnect event */
void hw_pd_report_device_detach(void);
/* usb mode is host and send charger disconnect event */
void hw_pd_report_host_detach(void);
#else
static inline void hw_pd_report_device_attach(void)
{
}

static inline void hw_pd_report_host_attach(void)
{
}

static inline void hw_pd_report_device_detach(void)
{
}

static inline void hw_pd_report_host_detach(void)
{
}
#endif /* CONFIG_TCPC_CLASS */

#endif