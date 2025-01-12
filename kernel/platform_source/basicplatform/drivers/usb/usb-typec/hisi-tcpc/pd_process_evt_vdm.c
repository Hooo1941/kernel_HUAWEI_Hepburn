/*
 * pd_process_evt_vdm.c
 *
 * Power Delivery Process Event For VDM
 *
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


#define LOG_TAG "[evt_vdm]"

#include "include/pd_core.h"
#include "include/pd_tcpm.h"
#include "include/tcpci_event.h"
#include "include/pd_process_evt.h"
#include "include/pd_dpm_core.h"

#define VDM_CMD_STATE(cmd, cmd_type)	\
	(((cmd) & 0x1f) | (((cmd_type) & 0x03) << 6))

#define VDM_CMD_INIT_STATE(cmd, next_state)	\
	{ VDM_CMD_STATE(cmd, CMDT_INIT), next_state }

#define VDM_CMD_ACK_STATE(cmd, next_state)	\
	{ VDM_CMD_STATE(cmd, CMDT_RSP_ACK), next_state }

#define VDM_CMD_NACK_STATE(cmd, next_state)	\
	{ VDM_CMD_STATE(cmd, CMDT_RSP_NAK), next_state }

#define VDM_CMD_BUSY_STATE(cmd, next_state)	\
	{ VDM_CMD_STATE(cmd, CMDT_RSP_BUSY), next_state }

/* UFP PD VDM Command's reactions */

DECL_PE_STATE_TRANSITION(PD_UFP_VDM_CMD) = {

	VDM_CMD_INIT_STATE(CMD_DISCOVER_IDENT, PE_UFP_VDM_GET_IDENTITY),
	VDM_CMD_INIT_STATE(CMD_DISCOVER_SVID, PE_UFP_VDM_GET_SVIDS),
	VDM_CMD_INIT_STATE(CMD_DISCOVER_MODES, PE_UFP_VDM_GET_MODES),
	VDM_CMD_INIT_STATE(CMD_ENTER_MODE, PE_UFP_VDM_EVALUATE_MODE_ENTRY),
	VDM_CMD_INIT_STATE(CMD_EXIT_MODE, PE_UFP_VDM_MODE_EXIT),

#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
	VDM_CMD_INIT_STATE(CMD_DP_STATUS, PE_UFP_VDM_DP_STATUS_UPDATE),
	VDM_CMD_INIT_STATE(CMD_DP_CONFIG, PE_UFP_VDM_DP_CONFIGURE),
#endif
};
DECL_PE_STATE_REACTION(PD_UFP_VDM_CMD);

/* DFP PD VDM Command's reactions */

DECL_PE_STATE_TRANSITION(PD_DFP_VDM_DISCOVER_ID) = {
	VDM_CMD_ACK_STATE(CMD_DISCOVER_IDENT, PE_DFP_UFP_VDM_IDENTITY_ACKED),
	VDM_CMD_NACK_STATE(CMD_DISCOVER_IDENT, PE_DFP_UFP_VDM_IDENTITY_NAKED),
	VDM_CMD_BUSY_STATE(CMD_DISCOVER_IDENT, PE_DFP_UFP_VDM_IDENTITY_NAKED),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_DISCOVER_ID);

DECL_PE_STATE_TRANSITION(PD_DFP_VDM_DISCOVER_SVID) = {
	VDM_CMD_ACK_STATE(CMD_DISCOVER_SVID, PE_DFP_VDM_SVIDS_ACKED),
	VDM_CMD_NACK_STATE(CMD_DISCOVER_SVID, PE_DFP_VDM_SVIDS_NAKED),
	VDM_CMD_BUSY_STATE(CMD_DISCOVER_SVID, PE_DFP_VDM_SVIDS_NAKED),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_DISCOVER_SVID);

DECL_PE_STATE_TRANSITION(PD_DFP_VDM_DISCOVER_MODES) = {
	VDM_CMD_ACK_STATE(CMD_DISCOVER_MODES, PE_DFP_VDM_MODES_ACKED),
	VDM_CMD_NACK_STATE(CMD_DISCOVER_MODES, PE_DFP_VDM_MODES_NAKED),
	VDM_CMD_BUSY_STATE(CMD_DISCOVER_MODES, PE_DFP_VDM_MODES_NAKED),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_DISCOVER_MODES);

DECL_PE_STATE_TRANSITION(PD_DFP_VDM_ENTER_MODE) = {
	VDM_CMD_ACK_STATE(CMD_ENTER_MODE, PE_DFP_VDM_MODE_ENTRY_ACKED),
	VDM_CMD_NACK_STATE(CMD_ENTER_MODE, PE_DFP_VDM_MODE_ENTRY_NAKED),
	VDM_CMD_BUSY_STATE(CMD_ENTER_MODE, PE_DFP_VDM_MODE_ENTRY_NAKED),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_ENTER_MODE);

DECL_PE_STATE_TRANSITION(PD_DFP_VDM_EXIT_MODE) = {
	VDM_CMD_ACK_STATE(CMD_EXIT_MODE, PE_DFP_VDM_MODE_ENTRY_ACKED),
	VDM_CMD_NACK_STATE(CMD_EXIT_MODE, PE_DFP_VDM_MODE_ENTRY_NAKED),
	VDM_CMD_BUSY_STATE(CMD_EXIT_MODE, PE_VIRT_HARD_RESET),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_EXIT_MODE);

DECL_PE_STATE_TRANSITION(PD_DFP_VDM_ATTENTION) = {
	VDM_CMD_INIT_STATE(CMD_ATTENTION, PE_DFP_VDM_ATTENTION_REQUEST),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_ATTENTION);

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT

DECL_PE_STATE_TRANSITION(PD_DFP_VDM_DP_STATUS) = {
	VDM_CMD_ACK_STATE(CMD_DP_STATUS, PE_DFP_VDM_DP_STATUS_UPDATE_ACKED),
	VDM_CMD_NACK_STATE(CMD_DP_STATUS, PE_DFP_VDM_DP_STATUS_UPDATE_NAKED),
	VDM_CMD_BUSY_STATE(CMD_DP_STATUS, PE_DFP_VDM_DP_STATUS_UPDATE_NAKED),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_DP_STATUS);

DECL_PE_STATE_TRANSITION(PD_DFP_VDM_DP_CONFIG) = {
	VDM_CMD_ACK_STATE(CMD_DP_CONFIG, PE_DFP_VDM_DP_CONFIGURATION_ACKED),
	VDM_CMD_NACK_STATE(CMD_DP_CONFIG, PE_DFP_VDM_DP_CONFIGURATION_NAKED),
	VDM_CMD_BUSY_STATE(CMD_DP_CONFIG, PE_DFP_VDM_DP_CONFIGURATION_NAKED),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_DP_CONFIG);

#endif /* CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT */


/* HW Event reactions */

DECL_PE_STATE_TRANSITION(PD_HW_MSG_TX_FAILED) = {
#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
	{PE_SRC_VDM_IDENTITY_REQUEST, PE_SRC_VDM_IDENTITY_NAKED},
#endif /* PD_CAP_SRC_STARTUP_DISCOVERY_ID */

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	{PE_DFP_CBL_VDM_IDENTITY_REQUEST, PE_DFP_CBL_VDM_IDENTITY_NAKED},
#endif /* CONFIG_USB_PD_DFP_READY_DISCOVER_ID */

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
	{PE_DFP_UVDM_SEND, PE_DFP_UVDM_NAKED},
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */
};
DECL_PE_STATE_REACTION(PD_HW_MSG_TX_FAILED);

/* DPM Event reactions */

DECL_PE_STATE_TRANSITION(PD_DPM_MSG_ACK) = {
	{ PE_UFP_VDM_GET_IDENTITY, PE_UFP_VDM_SEND_IDENTITY },
	{ PE_UFP_VDM_GET_SVIDS, PE_UFP_VDM_SEND_SVIDS },
	{ PE_UFP_VDM_GET_MODES, PE_UFP_VDM_SEND_MODES },
	{ PE_UFP_VDM_MODE_EXIT, PE_UFP_VDM_MODE_EXIT_ACK},
	{ PE_UFP_VDM_EVALUATE_MODE_ENTRY, PE_UFP_VDM_MODE_ENTRY_ACK },
};
DECL_PE_STATE_REACTION(PD_DPM_MSG_ACK);

DECL_PE_STATE_TRANSITION(PD_DPM_MSG_NAK) = {
	{PE_UFP_VDM_GET_IDENTITY, PE_UFP_VDM_GET_IDENTITY_NAK},
	{PE_UFP_VDM_GET_SVIDS, PE_UFP_VDM_GET_SVIDS_NAK},
	{PE_UFP_VDM_GET_MODES, PE_UFP_VDM_GET_MODES_NAK},
	{PE_UFP_VDM_MODE_EXIT, PE_UFP_VDM_MODE_EXIT_NAK},
	{PE_UFP_VDM_EVALUATE_MODE_ENTRY, PE_UFP_VDM_MODE_ENTRY_NAK},
};
DECL_PE_STATE_REACTION(PD_DPM_MSG_NAK);

/* Discover Cable ID */

#ifdef CONFIG_PD_DISCOVER_CABLE_ID
DECL_PE_STATE_TRANSITION(PD_DPM_MSG_DISCOVER_CABLE) = {
# ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
	{ PE_SRC_STARTUP, PE_SRC_VDM_IDENTITY_REQUEST},
	{ PE_SRC_DISCOVERY, PE_SRC_VDM_IDENTITY_REQUEST},
# endif

# ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	{ PE_SRC_READY, PE_DFP_CBL_VDM_IDENTITY_REQUEST},
	{ PE_SNK_READY, PE_DFP_CBL_VDM_IDENTITY_REQUEST},
# endif
};
DECL_PE_STATE_REACTION(PD_DPM_MSG_DISCOVER_CABLE);
#endif

/* Source Startup Discover Cable ID */
#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
DECL_PE_STATE_TRANSITION(PD_SRC_VDM_DISCOVER_CABLE) = {
	VDM_CMD_ACK_STATE(CMD_DISCOVER_IDENT, PE_SRC_VDM_IDENTITY_ACKED),
	VDM_CMD_NACK_STATE(CMD_DISCOVER_IDENT, PE_SRC_VDM_IDENTITY_NAKED),
	VDM_CMD_BUSY_STATE(CMD_DISCOVER_IDENT, PE_SRC_VDM_IDENTITY_NAKED),
};
DECL_PE_STATE_REACTION(PD_SRC_VDM_DISCOVER_CABLE);
#endif /* CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID */

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
DECL_PE_STATE_TRANSITION(PD_DFP_VDM_DISCOVER_CABLE) = {
	VDM_CMD_ACK_STATE(CMD_DISCOVER_IDENT, PE_DFP_CBL_VDM_IDENTITY_ACKED),
	VDM_CMD_NACK_STATE(CMD_DISCOVER_IDENT, PE_DFP_CBL_VDM_IDENTITY_NAKED),
	VDM_CMD_BUSY_STATE(CMD_DISCOVER_IDENT, PE_DFP_CBL_VDM_IDENTITY_NAKED),
};
DECL_PE_STATE_REACTION(PD_DFP_VDM_DISCOVER_CABLE);

#endif /* CONFIG_USB_PD_DFP_READY_DISCOVER_ID */

DECL_PE_STATE_TRANSITION(PD_PE_MSG_VDM_RESET) = {
	{ PE_DFP_VDM_MODE_ENTRY_REQUEST, PE_DFP_VDM_MODE_ENTRY_NAKED },
	{ PE_DFP_VDM_MODE_EXIT_REQUEST, PE_VIRT_HARD_RESET },
	{ PE_DFP_UFP_VDM_IDENTITY_REQUEST, PE_DFP_UFP_VDM_IDENTITY_NAKED },
	{ PE_DFP_VDM_SVIDS_REQUEST, PE_DFP_VDM_SVIDS_NAKED },
	{ PE_DFP_VDM_MODES_REQUEST, PE_DFP_VDM_MODES_NAKED },
#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
	{ PE_SRC_VDM_IDENTITY_REQUEST, PE_SRC_VDM_IDENTITY_NAKED },
#endif
#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	{ PE_DFP_CBL_VDM_IDENTITY_REQUEST, PE_DFP_CBL_VDM_IDENTITY_NAKED },
#endif
#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
	{ PE_DFP_VDM_DP_STATUS_UPDATE_REQUEST,
	  PE_DFP_VDM_DP_STATUS_UPDATE_NAKED },
	{ PE_DFP_VDM_DP_CONFIGURATION_REQUEST,
	  PE_DFP_VDM_DP_CONFIGURATION_NAKED },
#endif
#ifdef CONFIG_USB_PD_UVDM_SUPPORT
	{ PE_DFP_UVDM_SEND, PE_DFP_UVDM_NAKED },
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */
};
DECL_PE_STATE_REACTION(PD_PE_MSG_VDM_RESET);
/* Timer Event reactions */

DECL_PE_STATE_TRANSITION(PD_TIMER_VDM_MODE_ENTRY) = {
	{ PE_DFP_VDM_MODE_ENTRY_REQUEST, PE_DFP_VDM_MODE_ENTRY_NAKED },
};
DECL_PE_STATE_REACTION(PD_TIMER_VDM_MODE_ENTRY);

DECL_PE_STATE_TRANSITION(PD_TIMER_VDM_MODE_EXIT) = {
	{ PE_DFP_VDM_MODE_EXIT_REQUEST, PE_VIRT_HARD_RESET },
};
DECL_PE_STATE_REACTION(PD_TIMER_VDM_MODE_EXIT);

DECL_PE_STATE_TRANSITION(PD_TIMER_VDM_RESPONSE) = {
	{ PE_DFP_UFP_VDM_IDENTITY_REQUEST, PE_DFP_UFP_VDM_IDENTITY_NAKED },
	{ PE_DFP_VDM_SVIDS_REQUEST, PE_DFP_VDM_SVIDS_NAKED },
	{ PE_DFP_VDM_MODES_REQUEST, PE_DFP_VDM_MODES_NAKED },

#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
	{ PE_SRC_VDM_IDENTITY_REQUEST, PE_SRC_VDM_IDENTITY_NAKED },
#endif

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	{ PE_DFP_CBL_VDM_IDENTITY_REQUEST,
	  PE_DFP_CBL_VDM_IDENTITY_NAKED },
#endif

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
	{ PE_DFP_VDM_DP_STATUS_UPDATE_REQUEST,
	  PE_DFP_VDM_DP_STATUS_UPDATE_NAKED },
	{ PE_DFP_VDM_DP_CONFIGURATION_REQUEST,
	  PE_DFP_VDM_DP_CONFIGURATION_NAKED },
#endif
};
DECL_PE_STATE_REACTION(PD_TIMER_VDM_RESPONSE);

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
DECL_PE_STATE_TRANSITION(PD_TIMER_UVDM_RESPONSE) = {
	{ PE_DFP_UVDM_SEND, PE_DFP_UVDM_NAKED },
};
DECL_PE_STATE_REACTION(PD_TIMER_UVDM_RESPONSE);
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

/*
 * [BLOCK] Porcess Ctrl MSG
 */

static bool pd_process_ctrl_msg_good_crc(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
#ifdef CONFIG_USB_PD_DBG_DP_UFP_U_AUTO_ATTENTION
	if (pd_port->pe_state_curr == PE_UFP_VDM_DP_CONFIGURE) {
		PE_TRANSIT_STATE(pd_port, PE_UFP_VDM_ATTENTION_REQUEST);
		return true;
	}
#endif /* CONFIG_USB_PD_DBG_DP_UFP_U_AUTO_ATTENTION */
#endif /* CONFIG_USB_PD_ALT_MODE_SUPPORT */

	switch (pd_port->pe_state_curr) {
	case PE_UFP_VDM_SEND_IDENTITY:
	case PE_UFP_VDM_GET_IDENTITY_NAK:
	case PE_UFP_VDM_SEND_SVIDS:
	case PE_UFP_VDM_GET_SVIDS_NAK:

	case PE_UFP_VDM_SEND_MODES:
	case PE_UFP_VDM_GET_MODES_NAK:
	case PE_UFP_VDM_MODE_ENTRY_ACK:
	case PE_UFP_VDM_MODE_ENTRY_NAK:
	case PE_UFP_VDM_MODE_EXIT_ACK:
	case PE_UFP_VDM_MODE_EXIT_NAK:

#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
	case PE_UFP_VDM_DP_CONFIGURE:
	case PE_UFP_VDM_DP_STATUS_UPDATE:
#endif
		PE_TRANSIT_READY_STATE(pd_port);
		return true;

#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
	case PE_SRC_VDM_IDENTITY_REQUEST:
		pd_port->power_cable_present = true;
		return false;
#endif /* CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID */

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	case PE_DFP_CBL_VDM_IDENTITY_REQUEST:
		pd_port->power_cable_present = true;
		return false;
#endif /* CONFIG_USB_PD_DFP_READY_DISCOVER_ID */

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
	case PE_DFP_UVDM_SEND:
		if (!pd_port->uvdm_wait_resp) {
			PE_TRANSIT_STATE(pd_port, PE_DFP_UVDM_ACKED);
			return true;
		}
		break;
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

	default:
		break;
	}

	D("-\n");
	return false;
}

static bool pd_process_ctrl_msg(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ret = false;

	D("+\n");
	switch (pd_event->msg) {
	case PD_CTRL_GOOD_CRC:
		return pd_process_ctrl_msg_good_crc(pd_port, pd_event);

	default:
		break;
	}

	D("-\n");
	return ret;
}

/*
 * [BLOCK] Porcess Data MSG (UVDM)
 */

#ifdef CONFIG_USB_PD_UVDM_SUPPORT

static bool pd_process_uvdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");

	if (pd_port->data_role == PD_ROLE_UFP) {
		if (pd_check_pe_state_ready(pd_port)) {
			PE_TRANSIT_STATE(pd_port, PE_UFP_UVDM_RECV);
			return true;
		}
	} else { /* DFP */
		if (pd_port->pe_state_curr == PE_DFP_UVDM_SEND) {
			PE_TRANSIT_STATE(pd_port, PE_DFP_UVDM_ACKED);
			return true;
		}
		if (pd_event->pd_msg->frame_type == TCPC_TX_SOP)
			hisi_dfp_uvdm_receive_data(pd_event);
	}

	D("invalid, current status\n");
	D("-\n");
	return false;
}
#else
static inline bool pd_process_uvdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	return false;
}
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

/*
 * [BLOCK] Porcess Data MSG (VDM)
 */

static const char * const pe_vdm_cmd_name[] = {
	"DiscoverID",
	"DiscoverSVID",
	"DiscoverMode",
	"EnterMode",
	"ExitMode",
	"Attention",
};

static const char * const pe_vdm_dp_cmd_name[] = {
	"DPStatus",
	"DPConfig",
};

static const char * const pe_vdm_cmd_type_name[] = {
	"INIT",
	"ACK",
	"NACK",
	"BUSY",
};
static const char *assign_vdm_cmd_name(uint8_t cmd)
{
	D("+\n");
	if (cmd <= ARRAY_SIZE(pe_vdm_cmd_name) && cmd != 0) {
		cmd -= 1;
		return pe_vdm_cmd_name[cmd];
	}
	D("-\n");
	return NULL;
}
#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
static const char *assign_vdm_dp_cmd_name(uint8_t cmd)
{
	if (cmd >= CMD_DP_STATUS) {
		cmd -= CMD_DP_STATUS;
		if (cmd < ARRAY_SIZE(pe_vdm_dp_cmd_name))
			return pe_vdm_dp_cmd_name[cmd];
	}
	return NULL;
}
#endif     /* CONFIG_USB_PD_ALT_MODE_SUPPORT */

static void print_vdm_msg(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint8_t cmd;
	uint8_t cmd_type;
	uint16_t svid;
	const char *name = NULL;
	uint32_t vdm_hdr = pd_event->pd_msg->payload[0];

	cmd = PD_VDO_CMD(vdm_hdr);
	cmd_type = PD_VDO_CMDT(vdm_hdr);
	svid = PD_VDO_VID(vdm_hdr);

	name = assign_vdm_cmd_name(cmd);
#ifdef CONFIG_USB_PD_ALT_MODE_SUPPORT
	if (!name && svid == USB_SID_DISPLAYPORT)
		name = assign_vdm_dp_cmd_name(cmd);
#endif

	if (!name)
		return;

	if (cmd_type >= ARRAY_SIZE(pe_vdm_cmd_type_name))
		return;

	I("%s:%s\n", name, pe_vdm_cmd_type_name[cmd_type]);
	D("%s:%s\n", name, pe_vdm_cmd_type_name[cmd_type]);
}

static bool pd_process_ufp_vdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+-\n");
	if (!pd_check_pe_state_ready(pd_port)) {
		D("invalid, current status\n");
		D("pe state not ready\n");
		return false;
	}

	if (PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_UFP_VDM_CMD))
		return true;

	return false;
}

static bool pd_process_dfp_vdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	uint32_t vdm_hdr = pd_event->pd_msg->payload[0];

	D("+\n");
	if (PD_VDO_CMDT(vdm_hdr) == CMDT_INIT &&
			PD_VDO_CMD(vdm_hdr) == CMD_ATTENTION) {
		if (!pd_check_pe_state_ready(pd_port)) {
			D("pd_check_pe_state_ready false\n");
			return false;
		}

		if (PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_ATTENTION))
			return true;
	}

	switch (pd_port->pe_state_curr) {
	case PE_DFP_UFP_VDM_IDENTITY_REQUEST:
		return PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_DISCOVER_ID);

	case PE_DFP_VDM_SVIDS_REQUEST:
		return PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_DISCOVER_SVID);

	case PE_DFP_VDM_MODES_REQUEST:
		return PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_DISCOVER_MODES);

	case PE_DFP_VDM_MODE_ENTRY_REQUEST:
		return PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_ENTER_MODE);

	case PE_DFP_VDM_MODE_EXIT_REQUEST:
		return PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_ENTER_MODE);

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
	case PE_DFP_VDM_DP_STATUS_UPDATE_REQUEST:
		return PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_DP_STATUS);

	case PE_DFP_VDM_DP_CONFIGURATION_REQUEST:
		return PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_DP_CONFIG);
#endif

	default:
		if (pd_check_pe_state_ready(pd_port)) {
			PE_TRANSIT_STATE(pd_port, PE_DFP_VDM_UNKNOWN);
			return true;
		}
		break;
	}

	D("-\n");
	return false;
}

static bool pd_process_sop_vdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ret = false;

	D("+\n");
	if (pd_port->data_role == PD_ROLE_UFP)
		ret = pd_process_ufp_vdm(pd_port, pd_event);
	else
		ret = pd_process_dfp_vdm(pd_port, pd_event);

	if (!ret) {
		D("Unknown VDM\n");
		E("Unknown VDM\n");
	}

	D("-\n");
	return ret;
}

static bool pd_process_sop_prime_vdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	switch (pd_port->pe_state_curr) {
#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
	case PE_SRC_VDM_IDENTITY_REQUEST:
		if (PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_SRC_VDM_DISCOVER_CABLE))
			return true;
		/* fall through */
#endif /* CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID */

#ifdef CONFIG_USB_PD_DFP_READY_DISCOVER_ID
	case PE_DFP_CBL_VDM_IDENTITY_REQUEST:
		if (PE_MAKE_VDM_CMD_STATE_TRANSIT(PD_DFP_VDM_DISCOVER_CABLE))
			return true;
#endif /* CONFIG_USB_PD_DFP_READY_DISCOVER_ID */

	default:
		break;
	}

	D("-\n");
	return false;
}

static bool pd_process_data_msg(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ret = false;
	uint32_t vdm_hdr;
	pd_msg_t *pd_msg = pd_event->pd_msg;

	D("+\n");
	if (pd_event->msg != PD_DATA_VENDOR_DEF)
		return ret;

	vdm_hdr = pd_msg->payload[0];

	/* deal with unstructed vdm */
	if (!PD_VDO_SVDM(vdm_hdr))
		return pd_process_uvdm(pd_port, pd_event);

	/* From Port Partner, copy curr_state from pd_state */
	if (PD_VDO_CMDT(vdm_hdr) == CMDT_INIT) {
		pd_port->pe_vdm_state = pd_port->pe_pd_state;
		pd_port->pe_state_curr = pd_port->pe_pd_state;

		D("reset vdm_state\n");
		D("reset vdm_state\n");
	}

	print_vdm_msg(pd_port, pd_event);

	if (pd_msg->frame_type == TCPC_TX_SOP_PRIME)
		ret = pd_process_sop_prime_vdm(pd_port, pd_event); /* sop' */
	else
		ret = pd_process_sop_vdm(pd_port, pd_event); /* sop */

	D("-\n");
	return ret;
}

/*
 * [BLOCK] Porcess PDM MSG
 */

static bool pd_process_dpm_msg_ack(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("\n");
	if (pd_port->data_role == PD_ROLE_DFP) {
		switch (pd_port->pe_state_curr) {
		case PE_DFP_UFP_VDM_IDENTITY_ACKED:
		case PE_DFP_UFP_VDM_IDENTITY_NAKED:
		case PE_DFP_CBL_VDM_IDENTITY_ACKED:
		case PE_DFP_CBL_VDM_IDENTITY_NAKED:
		case PE_DFP_VDM_SVIDS_ACKED:
		case PE_DFP_VDM_SVIDS_NAKED:
		case PE_DFP_VDM_MODES_ACKED:
		case PE_DFP_VDM_MODES_NAKED:
		case PE_DFP_VDM_MODE_ENTRY_ACKED:
		case PE_DFP_VDM_MODE_EXIT_REQUEST:
		case PE_DFP_VDM_MODE_EXIT_ACKED:
		case PE_DFP_VDM_ATTENTION_REQUEST:
		case PE_DFP_VDM_UNKNOWN:
			PE_TRANSIT_READY_STATE(pd_port);
			return true;

#ifdef CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT
		case PE_DFP_VDM_DP_STATUS_UPDATE_ACKED:
		case PE_DFP_VDM_DP_STATUS_UPDATE_NAKED:
		case PE_DFP_VDM_DP_CONFIGURATION_ACKED:
		case PE_DFP_VDM_DP_CONFIGURATION_NAKED:
			PE_TRANSIT_READY_STATE(pd_port);
			return true;
#endif /* CONFIG_USB_PD_ALT_MODE_DFP_SUPPORT */

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
		case PE_DFP_UVDM_ACKED:
		case PE_DFP_UVDM_NAKED:
			PE_TRANSIT_READY_STATE(pd_port);
			return true;
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

		default:
			return false;
		}
	} else {
		return PE_MAKE_STATE_TRANSIT(PD_DPM_MSG_ACK);
	}
}

static bool pd_process_dpm_msg_vdm_request(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool is_dfp = NULL;
	bool is_attention = NULL;

	D("+\n");

	is_dfp = pd_port->data_role == PD_ROLE_DFP;
	is_attention = pd_event->msg_sec == PD_DPM_VDM_REQUEST_ATTENTION;

	if ((is_dfp && is_attention) || (!is_dfp && !is_attention)) {
		D("skip vdm_request, not dfp\n");
		return false;
	}

	PE_TRANSIT_STATE(pd_port, pd_event->msg_sec);
	D("-\n");
	return true;
}

static bool pd_process_dpm_msg(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ret = false;

	D("+\n");
	switch (pd_event->msg) {
	case PD_DPM_ACK:
		ret = pd_process_dpm_msg_ack(pd_port, pd_event);
		break;

	case PD_DPM_NAK:
		ret = PE_MAKE_STATE_TRANSIT(PD_DPM_MSG_NAK);
		break;

	case PD_DPM_VDM_REQUEST:
		ret = pd_process_dpm_msg_vdm_request(pd_port, pd_event);
		break;

#ifdef CONFIG_PD_DISCOVER_CABLE_ID
	case PD_DPM_DISCOVER_CABLE_ID:
		ret = PE_MAKE_STATE_TRANSIT(PD_DPM_MSG_DISCOVER_CABLE);
		break;
#endif

	default:
		break;
	}

	D("-\n");
	return ret;
}

/*
 * [BLOCK] Porcess HW MSG
 */

static inline bool pd_process_hw_msg_retry_vdm(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+-\n");
	D("RetryVDM\n");
	return pd_process_sop_vdm(pd_port, pd_event);
}

static bool pd_process_hw_msg(
		pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ret = false;

	D("+\n");
	 /* discard count */
	if (pd_event->msg == PD_HW_TX_DISCARD &&
			(pd_port->vdm_discard_retry_count < 10)) { /* 10: vdm_discard_retry_count */
		I("vdm_discard_retry\n");
		pd_port->vdm_discard_retry_flag = true;
		pd_port->vdm_discard_retry_count++;
	}
	switch (pd_event->msg) {
	case PD_HW_TX_FAILED:
	case PD_HW_TX_DISCARD:
		ret = PE_MAKE_STATE_TRANSIT(PD_PE_MSG_VDM_RESET);
		break;

	case PD_HW_RETRY_VDM:
		ret = pd_process_hw_msg_retry_vdm(pd_port, pd_event);
		break;

	default:
		break;
	}
	D("-\n");
	return ret;
}
static bool pd_process_pe_msg(pd_port_t *pd_port, pd_event_t *pd_event)
{
	bool ret = false;

	D("+\n");
	switch (pd_event->msg) {
	case PD_PE_VDM_RESET:
		pd_port->reset_vdm_state = true;
		ret = PE_MAKE_STATE_TRANSIT(PD_PE_MSG_VDM_RESET);
		break;

	default:
		break;
	}

	D("-\n");
	return ret;
}

/*
 * [BLOCK] Porcess Timer MSG
 */

static bool pd_process_timer_msg(
			pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+-\n");
	switch (pd_event->msg) {
	case PD_TIMER_VDM_MODE_ENTRY:
		return PE_MAKE_STATE_TRANSIT(PD_TIMER_VDM_MODE_ENTRY);

	case PD_TIMER_VDM_MODE_EXIT:
		return PE_MAKE_STATE_TRANSIT_VIRT(PD_TIMER_VDM_MODE_EXIT);

	case PD_TIMER_VDM_RESPONSE:
		return PE_MAKE_STATE_TRANSIT(PD_TIMER_VDM_RESPONSE);

#ifdef CONFIG_USB_PD_UVDM_SUPPORT
	case PD_TIMER_UVDM_RESPONSE:
		return PE_MAKE_STATE_TRANSIT(PD_TIMER_UVDM_RESPONSE);
#endif /* CONFIG_USB_PD_UVDM_SUPPORT */

	default:
		return false;
	}
}

/*
 * [BLOCK] Process Policy Engine's VDM Message
 */

bool hisi_pd_process_event_vdm(pd_port_t *pd_port, pd_event_t *pd_event)
{
	D("+\n");
	pd_port->vdm_discard_retry_flag = false;
	switch (pd_event->event_type) {
	case PD_EVT_CTRL_MSG:
		return pd_process_ctrl_msg(pd_port, pd_event);

	case PD_EVT_DATA_MSG:
		return pd_process_data_msg(pd_port, pd_event);

	case PD_EVT_DPM_MSG:
		return pd_process_dpm_msg(pd_port, pd_event);

	case PD_EVT_HW_MSG:
		return pd_process_hw_msg(pd_port, pd_event);

	case PD_EVT_PE_MSG:
		return pd_process_pe_msg(pd_port, pd_event);

	case PD_EVT_TIMER_MSG:
		return pd_process_timer_msg(pd_port, pd_event);

	default:
		break;
	}

	D("-\n");
	return false;
}
