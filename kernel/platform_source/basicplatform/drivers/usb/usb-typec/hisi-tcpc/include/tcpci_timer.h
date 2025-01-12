/*
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _TCPC_TIMER_H
#define _TCPC_TIMER_H

#include <linux/kernel.h>
#include "include/tcpci_config.h"

struct tcpc_device;

enum {
#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
	PD_TIMER_BIST_CONT_MODE = 0,
	PD_TIMER_DISCOVER_ID,
	PD_TIMER_HARD_RESET_COMPLETE,
	PD_TIMER_NO_RESPONSE,
	PD_TIMER_PS_HARD_RESET,
	PD_TIMER_PS_SOURCE_OFF,  // 5
	PD_TIMER_PS_SOURCE_ON,
	PD_TIMER_PS_TRANSITION,
	PD_TIMER_SENDER_RESPONSE,
	PD_TIMER_SINK_ACTIVITY,
	PD_TIMER_SINK_REQUEST,  // 10
	PD_TIMER_SINK_WAIT_CAP,
	PD_TIMER_SOURCE_ACTIVITY,
	PD_TIMER_SOURCE_CAPABILITY,
	PD_TIMER_SOURCE_START,
	PD_TIMER_VCONN_ON, // 15
	PD_TIMER_VDM_MODE_ENTRY,
	PD_TIMER_VDM_MODE_EXIT,
	PD_TIMER_VDM_RESPONSE,
	PD_TIMER_SOURCE_TRANSITION,
	PD_TIMER_SRC_RECOVER, // 20
	PD_TIMER_VSAFE0V_DELAY,
	PD_TIMER_VSAFE0V_TOUT,
	PD_TIMER_DISCARD,
	PD_TIMER_VBUS_STABLE,
	PD_TIMER_VBUS_PRESENT,  // 25
	PD_TIMER_UVDM_RESPONSE,
	PD_TIMER_DFP_FLOW_DELAY,
	PD_TIMER_UFP_FLOW_DELAY,
	PD_PE_VDM_POSTPONE,
	PD_TIMER_PE_IDLE_TOUT,  // 30
	PD_TIMER_SRC_CAP_EXT_TOUT,
	PD_PE_TIMER_END_ID,

	/* TYPEC-RT-TIMER */
	TYPEC_RT_TIMER_START_ID = PD_PE_TIMER_END_ID,
	TYPEC_RT_TIMER_PE_IDLE = TYPEC_RT_TIMER_START_ID,
	TYPEC_RT_TIMER_SAFE0V_DELAY,
	TYPEC_RT_TIMER_SAFE0V_TOUT,
	TYPEC_RT_TIMER_SAFE5V_TOUT,  // 35
	TYPEC_RT_TIMER_ROLE_SWAP_START,
	TYPEC_RT_TIMER_ROLE_SWAP_STOP,
	TYPEC_RT_TIMER_LEGACY,
	TYPEC_RT_TIMER_NOT_LEGACY,

	/* TYPEC-TRY-TIMER */
	TYPEC_TRY_TIMER_START_ID,  // 40
	TYPEC_TRY_TIMER_DRP_TRY = TYPEC_TRY_TIMER_START_ID,
	TYPEC_TRY_TIMER_DRP_TRYWAIT,

	/* TYPEC-DEBOUNCE-TIMER */
	TYPEC_TIMER_START_ID,
	TYPEC_TIMER_CCDEBOUNCE = TYPEC_TIMER_START_ID,
	TYPEC_TIMER_PDDEBOUNCE,
	TYPEC_TIMER_ERROR_RECOVERY,
	TYPEC_TIMER_DRP_SRC_TOGGLE,  // 45
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	TYPEC_TIMER_NORP_SRC,
#endif
#else
	TYPEC_RT_TIMER_START_ID = 0,
	TYPEC_RT_TIMER_SAFE0V_DELAY = TYPEC_RT_TIMER_START_ID,
	TYPEC_RT_TIMER_SAFE0V_TOUT,
	TYPEC_RT_TIMER_SAFE5V_TOUT,
	TYPEC_RT_TIMER_ROLE_SWAP_START,
	TYPEC_RT_TIMER_ROLE_SWAP_STOP,
	TYPEC_RT_TIMER_LEGACY,
	TYPEC_RT_TIMER_NOT_LEGACY,

	TYPEC_TRY_TIMER_START_ID,
	TYPEC_TRY_TIMER_DRP_TRY = TYPEC_TRY_TIMER_START_ID,
	TYPEC_TRY_TIMER_DRP_TRYWAIT,

	TYPEC_TIMER_START_ID,
	TYPEC_TIMER_CCDEBOUNCE = TYPEC_TIMER_START_ID,
	TYPEC_TIMER_PDDEBOUNCE,
	TYPEC_TIMER_DRP_SRC_TOGGLE,
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	TYPEC_TIMER_NORP_SRC,
#endif
#endif /* CONFIG_USB_POWER_DELIVERY_SUPPORT */
	PD_TIMER_NR,
};

int hisi_tcpci_timer_init(struct tcpc_device *tcpc);
int hisi_tcpci_timer_deinit(struct tcpc_device *tcpc);
void hisi_tcpc_restart_timer(struct tcpc_device *tcpc, uint32_t timer_id);
void hisi_tcpc_enable_timer(struct tcpc_device *tcpc, uint32_t timer_id);
void hisi_tcpc_disable_timer(struct tcpc_device *tcpc, uint32_t timer_id);
void hisi_tcpc_reset_typec_try_timer(struct tcpc_device *tcpc);
void hisi_tcpc_reset_typec_debounce_timer(struct tcpc_device *tcpc);
void hisi_tcpc_reset_pe_timer(struct tcpc_device *tcpc);
bool hisi_tcpc_timer_enabled(struct tcpc_device *tcpc, uint32_t timer_id);

#endif
