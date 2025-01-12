/*
 * Copyright (C) 2016 Richtek Technology Corp.
 *
 * Author: TH <tsunghan_tsai@richtek.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_TCPC_CONFIG_H
#define __LINUX_TCPC_CONFIG_H

/* default config */
#define CONFIG_USB_PD_DBG_ALERT_STATUS
#define CONFIG_RT_REGMAP

#define CONFIG_TYPEC_USE_DIS_VBUS_CTRL
#define CONFIG_TYPEC_POWER_CTRL_INIT

#define CONFIG_TYPEC_CAP_TRY_SOURCE
#define CONFIG_TYPEC_CAP_TRY_SINK

#define CONFIG_TYPEC_CAP_DBGACC_SNK
#define CONFIG_TYPEC_CAP_CUSTOM_SRC
#define CONFIG_TYPEC_CAP_NORP_SRC

#define CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT

#define CONFIG_TYPEC_CAP_RA_DETACH
#define CONFIG_TYPEC_CAP_LPM_WAKEUP_WATCHDOG

#define CONFIG_TYPEC_CAP_POWER_OFF_CHARGE

#define CONFIG_TYPEC_CAP_A2C_C2C

#define CONFIG_TCPC_SHUTDOWN_CC_DETACH
#define CONFIG_TYPEC_CAP_ROLE_SWAP
#define CONFIG_TYPEC_CAP_ROLE_SWAP_TOUT		600

/*
	USB 2.0 & 3.0 current
	Unconfigured :	100 / 150 mA
	Configured :		500 / 900 mA
 */

#define CONFIG_TYPEC_SNK_CURR_DFT		150
#define CONFIG_TYPEC_SRC_CURR_DFT		500
#define CONFIG_TYPEC_SNK_CURR_LIMIT		0

#define CONFIG_TCPC_VSAFE0V_DETECT
#define CONFIG_TCPC_VSAFE0V_DETECT_IC
#define CONFIG_TCPC_LOW_POWER_MODE
#define CONFIG_TCPC_I2CRST_EN

#ifdef CONFIG_USB_POWER_DELIVERY
#define CONFIG_USB_PD_ALT_MODE
#define CONFIG_USB_PD_ALT_MODE_DFP

#define CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
#define CONFIG_USB_PD_DFP_READY_DISCOVER_ID
#define CONFIG_USB_PD_RESET_CABLE

#define CONFIG_USB_PD_RANDOM_FLOW_DELAY

#define CONFIG_USB_PD_DFP_FLOW_DELAY
#define CONFIG_USB_PD_DFP_FLOW_DELAY_DRSWAP

/* Only in startup */
#define CONFIG_USB_PD_UFP_FLOW_DELAY

#define CONFIG_USB_PD_ATTEMP_DISCOVER_ID
#define CONFIG_USB_PD_ATTEMP_DISCOVER_SVID

#define CONFIG_USB_PD_DISCOVER_CABLE_REQUEST_VCONN
#define CONFIG_USB_PD_DISCOVER_CABLE_RETURN_VCONN

#define CONFIG_USB_PD_UVDM
#define CONFIG_USB_PD_CUSTOM_DBGACC

#define CONFIG_USB_PD_SNK_DFT_NO_GOOD_CRC
#define CONFIG_USB_PD_IGNORE_PS_RDY_AFTER_PR_SWAP

#define CONFIG_USB_PD_DROP_REPEAT_PING
#define CONFIG_USB_PD_RETRY_CRC_DISCARD
#define CONFIG_USB_PD_PARTNER_CTRL_MSG_FIRST

#define CONFIG_USB_PD_SNK_HRESET_KEEP_DRAW
#define CONFIG_USB_PD_TRANSMIT_BIST2
#define CONFIG_USB_PD_POSTPONE_VDM
#define CONFIG_USB_PD_POSTPONE_RETRY_VDM
#define CONFIG_USB_PD_POSTPONE_FIRST_VDM
#define CONFIG_USB_PD_POSTPONE_OTHER_VDM
#define CONFIG_USB_PD_STOP_REPLY_VDM_IF_RX_BUSY

#define CONFIG_USB_PD_HANDLE_PRDR_SWAP

#define CONFIG_USB_PD_SAFE0V_TIMEOUT

#ifndef CONFIG_USB_PD_DFP_FLOW_RETRY_MAX
#define CONFIG_USB_PD_DFP_FLOW_RETRY_MAX 2
#endif	/* CONFIG_USB_PD_DFP_FLOW_RETRY_MAX */

#ifndef CONFIG_USB_PD_VBUS_STABLE_TOUT
#define CONFIG_USB_PD_VBUS_STABLE_TOUT 125
#endif	/* CONFIG_USB_PD_VBUS_STABLE_TOUT */

#ifndef CONFIG_USB_PD_VBUS_PRESENT_TOUT
#define CONFIG_USB_PD_VBUS_PRESENT_TOUT	40
#endif	/* CONFIG_USB_PD_VBUS_PRESENT_TOUT */

#ifndef CONFIG_USB_PD_UVDM_TOUT
#define CONFIG_USB_PD_UVDM_TOUT	100
#endif	/* CONFIG_USB_PD_UVDM_TOUT */

#endif /* CONFIG_USB_POWER_DELIVERY */

/* debug config */

#endif /* __LINUX_TCPC_CONFIG_H */
