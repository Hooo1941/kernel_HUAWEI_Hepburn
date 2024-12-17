/*
 * Copyright (C) 2020 Richtek Inc.
 *
 * TCPC Type-C Driver for Richtek
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

#include <linux/delay.h>
#include <linux/cpu.h>

#ifdef CONFIG_TCPC_RT1715
#include <huawei_platform/usb/hw_pd_dev.h>
#endif

#ifdef CONFIG_HUAWEI_TCPC_HUSB311
#include <linux/timex.h>
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */

#include "inc/tcpci.h"
#include "inc/tcpm.h"
#include "inc/tcpci_typec.h"
#include "inc/tcpci_timer.h"

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
#define CONFIG_TYPEC_CAP_TRY_STATE
#endif

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
#undef CONFIG_TYPEC_CAP_TRY_STATE
#define CONFIG_TYPEC_CAP_TRY_STATE
#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

#define SRC_CAP_DEFAULT_VOL       5000

enum TYPEC_WAIT_PS_STATE {
	TYPEC_WAIT_PS_DISABLE = 0,
	TYPEC_WAIT_PS_SNK_VSAFE5V,
	TYPEC_WAIT_PS_SRC_VSAFE0V,
	TYPEC_WAIT_PS_SRC_VSAFE5V,
#ifdef CONFIG_TYPEC_CAP_DBGACC
	TYPEC_WAIT_PS_DBG_VSAFE5V,
#endif	/* CONFIG_TYPEC_CAP_DBGACC */
};

enum TYPEC_ROLE_SWAP_STATE {
	TYPEC_ROLE_SWAP_NONE = 0,
	TYPEC_ROLE_SWAP_TO_SNK,
	TYPEC_ROLE_SWAP_TO_SRC,
};

#if TYPEC_INFO2_ENABLE
static const char *const typec_wait_ps_name[] = {
	"Disable",
	"SNK_VSafe5V",
	"SRC_VSafe0V",
	"SRC_VSafe5V",
	"DBG_VSafe5V",
};
#endif	/* TYPEC_INFO2_ENABLE */

#ifdef CONFIG_HUAWEI_TCPC_HUSB311
#define TIMER_CNT_ONE_MS 600
#define TIMER_CNT_TWO_MS 1200
#define ABNORMAL_ADP_DET_CNT_ZERO 0
#define ABNORMAL_ADP_DET_CNT_ONE 1
#define ABNORMAL_ADP_DET_CNT_TWO 2
#define ABNORMAL_ADP_DET_CNT_THR 3
struct variable_husb311 {
	int abnormal_adp_det_cnt;
	struct timespec64 ts_time;
	long long begin_1;
	long long end_1;
	long long begin_2;
	long long end_2;
};
struct variable_husb311 g_abnormal_var = {
	.abnormal_adp_det_cnt = ABNORMAL_ADP_DET_CNT_ZERO
};
static inline long long timespec_to_ms(struct timespec64 tv)
{
	/* 1000 * tv.tv_sec second : seconds to milliseconds */
	/* tv.tv_nsec / 1000000 : nanoseconds to milliseconds */
	return 1000 * tv.tv_sec + tv.tv_nsec / 1000000;
}
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */

static inline void typec_wait_ps_change(struct tcpc_device *tcpc,
					enum TYPEC_WAIT_PS_STATE state)
{
#if TYPEC_INFO2_ENABLE
	uint8_t old_state = tcpc->typec_wait_ps_change;
	uint8_t new_state = (uint8_t) state;

	if (new_state != old_state)
		TYPEC_INFO2("wait_ps=%s\n", typec_wait_ps_name[new_state]);
#endif	/* TYPEC_INFO2_ENABLE */

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
	if (state == TYPEC_WAIT_PS_SRC_VSAFE0V) {
		mutex_lock(&tcpc->access_lock);
		tcpci_enable_force_discharge(tcpc, true, 0);
		tcpc_enable_timer(tcpc, TYPEC_RT_TIMER_SAFE0V_TOUT);
		mutex_unlock(&tcpc->access_lock);
	}
#endif	/* CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT */

	if (tcpc->typec_wait_ps_change == TYPEC_WAIT_PS_SRC_VSAFE0V
		&& state != TYPEC_WAIT_PS_SRC_VSAFE0V) {
#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_DELAY
		tcpc_disable_timer(tcpc, TYPEC_RT_TIMER_SAFE0V_DELAY);
#endif	/* CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_DELAY */

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
		mutex_lock(&tcpc->access_lock);
		tcpc_disable_timer(tcpc, TYPEC_RT_TIMER_SAFE0V_TOUT);
		tcpci_enable_force_discharge(tcpc, false, 0);
		mutex_unlock(&tcpc->access_lock);
#endif	/* CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT */
	}

	tcpc->typec_wait_ps_change = (uint8_t) state;
}

#define typec_check_cc1(cc)	\
	(typec_get_cc1() == cc)

#define typec_check_cc2(cc)	\
	(typec_get_cc2() == cc)

#define typec_check_cc(cc1, cc2)	\
	(typec_check_cc1(cc1) && typec_check_cc2(cc2))

#define typec_check_cc_both(res)	\
	(typec_check_cc(res, res))

#define typec_check_cc_any(res)		\
	(typec_check_cc1(res) || typec_check_cc2(res))

#define typec_is_drp_toggling() \
	(typec_get_cc1() == TYPEC_CC_DRP_TOGGLING)

#define typec_is_cc_open()	\
	typec_check_cc_both(TYPEC_CC_VOLT_OPEN)

#define typec_is_sink_with_emark()	\
	(typec_get_cc1() + typec_get_cc2() == \
	TYPEC_CC_VOLT_RA+TYPEC_CC_VOLT_RD)

#ifdef CONFIG_TCPC_RT1715
#define typec_is_cc_ra()	\
	typec_check_cc_both(TYPEC_CC_VOLT_RA)
#endif

#define typec_is_cc_no_res()	\
	(typec_is_drp_toggling() || typec_is_cc_open())

static inline int typec_enable_vconn(struct tcpc_device *tcpc)
{
	if (!typec_is_sink_with_emark())
		return 0;

	if (tcpc->tcpc_vconn_supply == TCPC_VCONN_SUPPLY_NEVER)
		return 0;

	return tcpci_set_vconn(tcpc, true);
}

/*
 * [BLOCK] TYPEC Connection State Definition
 */

#if TYPEC_INFO_ENABLE || TCPC_INFO_ENABLE || TYPEC_DBG_ENABLE
static const char *const typec_state_names[] = {
	"Disabled",
	"ErrorRecovery",

	"Unattached.SNK",
	"Unattached.SRC",

	"AttachWait.SNK",
	"AttachWait.SRC",

	"Attached.SNK",
	"Attached.SRC",

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	"Try.SRC",
	"TryWait.SNK",
	"TryWait.SNK.PE",
#endif	/* CONFIG_TYPEC_CAP_TRY_SOURCE */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	"Try.SNK",
	"TryWait.SRC",
	"TryWait.SRC.PE",
#endif	/* CONFIG_TYPEC_CAP_TRY_SINK */

	"AudioAccessory",
#ifdef CONFIG_TYPEC_CAP_DBGACC
	"DebugAccessory",
#endif	/* CONFIG_TYPEC_CAP_DBGACC */

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK
	"DBGACC.SNK",
#endif	/* CONFIG_TYPEC_CAP_DBGACC_SNK */

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC
	"Custom.SRC",
#endif	/* CONFIG_TYPEC_CAP_CUSTOM_SRC */

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	"NoRp.SRC",
#endif	/* CONFIG_TYPEC_CAP_NORP_SRC */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	"RoleSwap",
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	"UnattachWait.PE",
};
#endif /* TYPEC_INFO_ENABLE || TCPC_INFO_ENABLE || TYPEC_DBG_ENABLE */

static inline void typec_transfer_state(struct tcpc_device *tcpc,
					enum TYPEC_CONNECTION_STATE state)
{
	TYPEC_INFO("** %s\n",
		   state < ARRAY_SIZE(typec_state_names) ?
		   typec_state_names[state] : "Unknown");
	tcpc->typec_state = (uint8_t) state;
}

#define TYPEC_NEW_STATE(state)  \
	(typec_transfer_state(tcpc, state))

/*
 * [BLOCK] TypeC Alert Attach Status Changed
 */

#if TYPEC_INFO_ENABLE || TYPEC_DBG_ENABLE
static const char *const typec_attach_names[] = {
	"NULL",
	"SINK",
	"SOURCE",
	"AUDIO",
#ifdef CONFIG_TCPC_RT1715
	"ATTACHED_VBUS_ONLY",
	"DETTACHED_VBUS_ONLY",
#endif
	"DEBUG",

	"DBGACC_SNK",
	"CUSTOM_SRC",
	"NORP_SRC",
	"PROTECTION",
};
#endif /* TYPEC_INFO_ENABLE || TYPEC_DBG_ENABLE */

static int typec_alert_attach_state_change(struct tcpc_device *tcpc)
{
	int ret = 0;

	if (tcpc->typec_attach_old == tcpc->typec_attach_new) {
#ifdef CONFIG_TCPC_RT1715
		if (tcpc->typec_attach_new < ARRAY_SIZE(typec_state_names))
			TYPEC_DBG("Attached-> %s(repeat)\n", typec_attach_names[tcpc->typec_attach_new]);
		else
			TYPEC_DBG("Attached type is %d (repeat)\n", tcpc->typec_attach_new);
#else
		TYPEC_DBG("Attached-> %s(repeat)\n",
			typec_attach_names[tcpc->typec_attach_new]);
#endif
		return 0;
	}
#ifdef CONFIG_TCPC_RT1715
	if (tcpc->typec_attach_new < ARRAY_SIZE(typec_state_names))
		TYPEC_INFO("Attached-> %s\n", typec_attach_names[tcpc->typec_attach_new]);
	else
		TYPEC_INFO("Attached type is %d\n", tcpc->typec_attach_new);
#else
	TYPEC_INFO("Attached-> %s\n",
		   typec_attach_names[tcpc->typec_attach_new]);
#endif
	/* Report function */
	ret = tcpci_report_usb_port_changed(tcpc);

	tcpc->typec_attach_old = tcpc->typec_attach_new;
	return ret;
}

static inline void typec_enable_low_power_mode(struct tcpc_device *tcpc)
{
	tcpc->typec_lpm = true;
	tcpc->typec_lpm_tout = 0;
	tcpc_enable_lpm_timer(tcpc, true);
}

static inline int typec_disable_low_power_mode(struct tcpc_device *tcpc)
{
	tcpc_enable_lpm_timer(tcpc, false);
	return tcpci_set_low_power_mode(tcpc, false);
}

/*
 * [BLOCK] NoRpSRC Entry
 */

#ifdef CONFIG_TYPEC_CAP_NORP_SRC

#ifdef CONFIG_TCPC_RT1715
static void typec_notify_attached_vbus_only(struct tcpc_device *tcpc)
{
	TYPEC_INFO("%s + \n", __func__);
	tcpc->typec_attach_new = TYPEC_ATTACHED_VBUS_ONLY;
	tcpci_notify_typec_state(tcpc);
}

static void typec_notify_unattached_vbus_only(struct tcpc_device *tcpc)
{
	struct pd_dpm_typec_state tc_state = {0};
	int cc_status = 0;

	TYPEC_INFO("%s + \n", __func__);
	tcpc->typec_attach_new = TYPEC_UNATTACHED_VBUS_ONLY;
	tc_state.new_state = PD_DPM_TYPEC_UNATTACHED_VBUS_ONLY;
	pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE, (void*)&tc_state);

	cc_status = rt1711_get_cc_status(tcpc);
	TCPC_INFO("%s cc_status:%x\n", __func__, cc_status);
	if (cc_status == 0)
		tcpci_set_cc(tcpc, TYPEC_CC_DRP);
}
#endif

static bool typec_try_enter_norp_src(struct tcpc_device *tcpc)
{
	if (tcpc->typec_state == typec_unattached_snk &&
	    tcpci_check_vbus_valid(tcpc) && typec_is_cc_no_res()) {
		TYPEC_INFO("norp_src=1\n");
		tcpc_enable_timer(tcpc, TYPEC_TIMER_NORP_SRC);
		return true;
	}

	return false;
}

static void typec_unattach_wait_pe_idle_entry(struct tcpc_device *tcpc);
static bool typec_try_exit_norp_src(struct tcpc_device *tcpc)
{
#ifdef CONFIG_TCPC_RT1715
	if (!typec_is_cc_ra() && !typec_is_cc_no_res()) {
		TYPEC_INFO("norp_src=0\r\n");
		tcpc_disable_timer(tcpc, TYPEC_TIMER_NORP_SRC);
		return true;
	}

	return false;
#else
	if (tcpc->typec_state == typec_attached_norp_src &&
	    !(tcpci_check_vbus_valid(tcpc) && typec_is_cc_no_res())) {
		TYPEC_INFO("norp_src=0\n");
		typec_unattach_wait_pe_idle_entry(tcpc);
		typec_alert_attach_state_change(tcpc);
		return true;
	}

	return false;
#endif
}

#ifdef CONFIG_TCPC_RT1715
static bool typec_norp_src_detached_entry(struct tcpc_device *tcpc_dev)
{
	TYPEC_INFO("%s:entry\n", __func__);
	if (tcpc_dev->no_rpsrc_state == 1) {
		tcpc_dev->no_rpsrc_state = 0;
		typec_notify_unattached_vbus_only(tcpc_dev);
		return true;
	} else {
		tcpc_disable_timer(tcpc_dev, TYPEC_TIMER_NORP_SRC);
	}

	return false;
}
#endif

static inline int typec_norp_src_attached_entry(struct tcpc_device *tcpc)
{
	TYPEC_INFO("%s:entry\n", __func__);
	typec_disable_low_power_mode(tcpc);

#ifdef NOT_USED_CODE
	TYPEC_NEW_STATE(typec_attached_norp_src);
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);
	tcpc->typec_attach_new = TYPEC_ATTACHED_NORP_SRC;

#ifdef CONFIG_TYPEC_CAP_A2C_C2C
	tcpc->typec_a2c_cable = true;
#endif	/* CONFIG_TYPEC_CAP_A2C_C2C */

	tcpci_report_power_control(tcpc, true);
	tcpci_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, 500);

	typec_alert_attach_state_change(tcpc);
#else
#ifdef CONFIG_TCPC_RT1715
/* vbus_only */
#ifdef CONFIG_TYPEC_CAP_A2C_C2C
	tcpc->typec_a2c_cable = true;
#endif	/* CONFIG_TYPEC_CAP_A2C_C2C */

	if (tcpc->no_rpsrc_state == 0) {
		tcpc->no_rpsrc_state = 1;
		typec_notify_attached_vbus_only(tcpc);
	}
#endif
#endif
	return 0;
}

#ifdef CONFIG_TCPC_RT1715
static inline int typec_handle_norp_src_debounce_timeout(struct tcpc_device *tcpc)
{
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	if ((typec_is_cc_no_res() || typec_is_cc_ra()) && tcpci_check_vbus_valid(tcpc))
		typec_norp_src_attached_entry(tcpc);
#endif    /* CONFIG_TYPEC_CAP_NORP_SRC */
	return 0;
}
#endif  /* CONFIG_TCPC_RT1715 */
#endif	/* CONFIG_TYPEC_CAP_NORP_SRC */

/*
 * [BLOCK] Unattached Entry
 */

static void typec_unattached_power_entry(struct tcpc_device *tcpc)
{
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

	if (tcpc->typec_power_ctrl) {
		tcpci_set_vconn(tcpc, false);
		tcpci_disable_vbus_control(tcpc);
		tcpci_report_power_control(tcpc, false);
	}
}

static inline void typec_unattached_src_and_drp_entry(struct tcpc_device *tcpc)
{
	TYPEC_NEW_STATE(typec_unattached_src);
	tcpci_set_cc(tcpc, TYPEC_CC_RP);
	tcpc_enable_timer(tcpc, TYPEC_TIMER_DRP_SRC_TOGGLE);
}

static inline void typec_unattached_snk_and_drp_entry(struct tcpc_device *tcpc)
{
	TYPEC_NEW_STATE(typec_unattached_snk);
	tcpci_set_cc(tcpc, TYPEC_CC_DRP);
	typec_enable_low_power_mode(tcpc);
}

static inline void typec_unattached_cc_entry(struct tcpc_device *tcpc)
{
#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc->typec_during_role_swap) {
		TYPEC_NEW_STATE(typec_role_swap);
		return;
	}
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	if (!typec_is_cc_no_res()) {
		tcpci_set_cc(tcpc, TYPEC_CC_OPEN);
		usleep_range(20000, 30000);
	}

	tcpc->typec_role = tcpc->typec_role_new;

	switch (tcpc->typec_role) {
	case TYPEC_ROLE_SNK:
		TYPEC_NEW_STATE(typec_unattached_snk);
		tcpci_set_cc(tcpc, TYPEC_CC_RD);
		typec_enable_low_power_mode(tcpc);
		break;
	case TYPEC_ROLE_SRC:
		TYPEC_NEW_STATE(typec_unattached_src);
		tcpci_set_cc(tcpc, TYPEC_CC_RP);
		typec_enable_low_power_mode(tcpc);
		break;
	case TYPEC_ROLE_TRY_SRC:
		if (tcpc->typec_state == typec_errorrecovery) {
			typec_unattached_src_and_drp_entry(tcpc);
			break;
		}
		/* fall through */
	default:
		switch (tcpc->typec_state) {
		case typec_attachwait_snk:
		case typec_audioaccessory:
			typec_unattached_src_and_drp_entry(tcpc);
			break;
		default:
			typec_unattached_snk_and_drp_entry(tcpc);
			break;
		}
		break;
	}
}

static void typec_unattached_entry(struct tcpc_device *tcpc)
{

	tcpc->typec_usb_sink_curr = CONFIG_TYPEC_SNK_CURR_DFT;

	if (tcpc->typec_power_ctrl)
		tcpci_set_vconn(tcpc, false);
	typec_unattached_cc_entry(tcpc);
	typec_unattached_power_entry(tcpc);
}

static bool typec_is_in_protection_states(struct tcpc_device *tcpc)
{
	return false;
}

static void typec_attach_new_unattached(struct tcpc_device *tcpc)
{
	tcpc->typec_attach_new = TYPEC_UNATTACHED;
	tcpc->typec_remote_rp_level = TYPEC_CC_VOLT_SNK_DFT;
#ifndef CONFIG_TCPC_RT1715
	tcpc->typec_polarity = false;
#endif
}

static void typec_unattach_wait_pe_idle_entry(struct tcpc_device *tcpc)
{
	typec_attach_new_unattached(tcpc);

#ifdef CONFIG_USB_POWER_DELIVERY
	if (tcpc->pd_pe_running) {
		TYPEC_NEW_STATE(typec_unattachwait_pe);
		return;
	}
#endif	/* CONFIG_USB_POWER_DELIVERY */

	typec_unattached_entry(tcpc);
}

static void typec_postpone_state_change(struct tcpc_device *tcpc)
{
	TYPEC_DBG("Postpone AlertChange\n");
	tcpc_enable_timer(tcpc, TYPEC_RT_TIMER_STATE_CHANGE);
}

static void typec_cc_open_entry(struct tcpc_device *tcpc, uint8_t state)
{
	typec_attach_new_unattached(tcpc);
	TYPEC_NEW_STATE(state);
	tcpci_set_cc(tcpc, TYPEC_CC_OPEN);
	if (tcpc->typec_state == typec_disabled)
		typec_enable_low_power_mode(tcpc);
	else
		typec_disable_low_power_mode(tcpc);
	typec_unattached_power_entry(tcpc);
	typec_alert_attach_state_change(tcpc);
	if (typec_is_in_protection_states(tcpc)) {
		tcpc->typec_attach_new = TYPEC_PROTECTION;
		typec_postpone_state_change(tcpc);
	}
}

static inline void typec_error_recovery_entry(struct tcpc_device *tcpc)
{
	typec_cc_open_entry(tcpc, typec_errorrecovery);
	tcpc_reset_typec_debounce_timer(tcpc);
	tcpc_enable_timer(tcpc, TYPEC_TIMER_ERROR_RECOVERY);
}

static inline void typec_disable_entry(struct tcpc_device *tcpc)
{
	typec_cc_open_entry(tcpc, typec_disabled);
}

/*
 * [BLOCK] Attached Entry
 */

static inline int typec_set_polarity(struct tcpc_device *tcpc,
					bool polarity)
{
	tcpc->typec_polarity = polarity;
	return tcpci_set_polarity(tcpc, polarity);
}

static inline int typec_set_plug_orient(struct tcpc_device *tcpc,
				uint8_t pull, bool polarity)
{
	int ret = typec_set_polarity(tcpc, polarity);

	if (ret)
		return ret;

	return tcpci_set_cc(tcpc, pull);
}

static void typec_source_attached_with_vbus_entry(struct tcpc_device *tcpc)
{
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);
	tcpc->typec_attach_new = TYPEC_ATTACHED_SRC;
}

static inline void typec_source_attached_entry(struct tcpc_device *tcpc)
{
	TYPEC_NEW_STATE(typec_attached_src);
	tcpc->typec_is_attached_src = true;
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_SRC_VSAFE5V);

	tcpc_disable_timer(tcpc, TYPEC_TRY_TIMER_DRP_TRY);

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc->typec_during_role_swap) {
		tcpc->typec_during_role_swap = TYPEC_ROLE_SWAP_NONE;
		tcpc_disable_timer(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
	}
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	typec_set_plug_orient(tcpc,
		TYPEC_CC_PULL(tcpc->typec_local_rp_level, TYPEC_CC_RP),
		typec_check_cc2(TYPEC_CC_VOLT_RD));

	tcpci_report_power_control(tcpc, true);
	typec_enable_vconn(tcpc);
	tcpci_source_vbus(tcpc,
			TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SOURCE_5V, -1);

#ifdef CONFIG_HUAWEI_TCPC_HUSB311
	if (chip_id_is_husb311_did()) {
		/* otg-default hold lock step 1 */
		TYPEC_INFO("otg-default hold lock step 1:src_attached \n");
		g_abnormal_var.abnormal_adp_det_cnt = ABNORMAL_ADP_DET_CNT_ONE;
		ktime_get_real_ts64(&g_abnormal_var.ts_time);
		g_abnormal_var.begin_1 = timespec_to_ms(g_abnormal_var.ts_time);
	}
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */
}

static inline void typec_sink_attached_entry(struct tcpc_device *tcpc)
{
	TYPEC_NEW_STATE(typec_attached_snk);
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

	tcpc->typec_attach_new = TYPEC_ATTACHED_SNK;

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
	if (tcpc->typec_role >= TYPEC_ROLE_DRP)
		tcpc_reset_typec_try_timer(tcpc);
#endif	/* CONFIG_TYPEC_CAP_TRY_STATE */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc->typec_during_role_swap) {
		tcpc->typec_during_role_swap = TYPEC_ROLE_SWAP_NONE;
		tcpc_disable_timer(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
	}
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	typec_set_plug_orient(tcpc, TYPEC_CC_RD,
		!typec_check_cc2(TYPEC_CC_VOLT_OPEN));
	tcpc->typec_remote_rp_level = typec_get_cc_res();

	tcpci_report_power_control(tcpc, true);
	tcpci_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);

#ifdef CONFIG_HUAWEI_TCPC_HUSB311
	if (chip_id_is_husb311_did()) {
		/* otg-default hold lock step 4 */
		TYPEC_INFO("otg-default hold lock step 4:snk_attached \n");
		g_abnormal_var.abnormal_adp_det_cnt = ABNORMAL_ADP_DET_CNT_ZERO;
	}
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */
}

static inline void typec_custom_src_attached_entry(
	struct tcpc_device *tcpc)
{
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK
	TYPEC_DBG("[Warning] Same Rp (%d)\n", typec_get_cc1());
#else
	TYPEC_DBG("[Warning] CC Both Rp\n");
#endif

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC
	TYPEC_NEW_STATE(typec_attached_custom_src);
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);
	tcpc->typec_attach_new = TYPEC_ATTACHED_CUSTOM_SRC;

	tcpc->typec_remote_rp_level = typec_get_cc1();

	tcpci_report_power_control(tcpc, true);
	tcpci_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
#endif	/* CONFIG_TYPEC_CAP_CUSTOM_SRC */
}

#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK

static inline uint8_t typec_get_sink_dbg_acc_rp_level(
	int cc1, int cc2)
{
	if (cc2 == TYPEC_CC_VOLT_SNK_DFT)
		return cc1;

	return TYPEC_CC_VOLT_SNK_DFT;
}

static inline void typec_sink_dbg_acc_attached_entry(
	struct tcpc_device *tcpc)
{
	bool polarity;
	uint8_t rp_level;

	uint8_t cc1 = typec_get_cc1();
	uint8_t cc2 = typec_get_cc2();

	if (cc1 == cc2) {
		typec_custom_src_attached_entry(tcpc);
		return;
	}

	TYPEC_NEW_STATE(typec_attached_dbgacc_snk);
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);
	tcpc->typec_attach_new = TYPEC_ATTACHED_DBGACC_SNK;

	polarity = cc2 > cc1;

	if (polarity)
		rp_level = typec_get_sink_dbg_acc_rp_level(cc2, cc1);
	else
		rp_level = typec_get_sink_dbg_acc_rp_level(cc1, cc2);

	typec_set_plug_orient(tcpc, TYPEC_CC_RD, polarity);
	tcpc->typec_remote_rp_level = rp_level;

	tcpci_report_power_control(tcpc, true);
	tcpci_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
}
#else
static inline void typec_sink_dbg_acc_attached_entry(
	struct tcpc_device *tcpc)
{
	typec_custom_src_attached_entry(tcpc);
}
#endif	/* CONFIG_TYPEC_CAP_DBGACC_SNK */


/*
 * [BLOCK] Try.SRC / TryWait.SNK
 */

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE

static inline bool typec_role_is_try_src(
	struct tcpc_device *tcpc)
{
	if (tcpc->typec_role != TYPEC_ROLE_TRY_SRC)
		return false;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc->typec_during_role_swap)
		return false;
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	return true;
}

static inline void typec_try_src_entry(struct tcpc_device *tcpc)
{
	TYPEC_NEW_STATE(typec_try_src);
	tcpc->typec_drp_try_timeout = false;

	tcpci_set_cc(tcpc, TYPEC_CC_RP);
	tcpc_enable_timer(tcpc, TYPEC_TRY_TIMER_DRP_TRY);
}

static inline void typec_trywait_snk_entry(struct tcpc_device *tcpc)
{
	TYPEC_NEW_STATE(typec_trywait_snk);
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

	tcpci_set_vconn(tcpc, false);
	tcpci_set_cc(tcpc, TYPEC_CC_RD);
	tcpci_source_vbus(tcpc,
			TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SOURCE_0V, 0);
	tcpc_disable_timer(tcpc, TYPEC_TRY_TIMER_DRP_TRY);

	tcpc_enable_timer(tcpc, TYPEC_TIMER_PDDEBOUNCE);
}

static inline void typec_trywait_snk_pe_entry(struct tcpc_device *tcpc)
{
	typec_attach_new_unattached(tcpc);

#ifdef CONFIG_USB_POWER_DELIVERY
	if (tcpc->typec_attach_old) {
		TYPEC_NEW_STATE(typec_trywait_snk_pe);
		return;
	}
#endif

	typec_trywait_snk_entry(tcpc);
}

#endif /* CONFIG_TYPEC_CAP_TRY_SOURCE */

/*
 * [BLOCK] Try.SNK / TryWait.SRC
 */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK

static inline bool typec_role_is_try_sink(
	struct tcpc_device *tcpc)
{
	if (tcpc->typec_role != TYPEC_ROLE_TRY_SNK)
		return false;

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc->typec_during_role_swap)
		return false;
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

	return true;
}

static inline void typec_try_snk_entry(struct tcpc_device *tcpc)
{
	TYPEC_NEW_STATE(typec_try_snk);
	tcpc->typec_drp_try_timeout = false;

	tcpci_set_cc(tcpc, TYPEC_CC_RD);
	tcpc_enable_timer(tcpc, TYPEC_TRY_TIMER_DRP_TRY);
}

static inline void typec_trywait_src_entry(struct tcpc_device *tcpc)
{
	TYPEC_NEW_STATE(typec_trywait_src);
	tcpc->typec_drp_try_timeout = false;

	tcpci_set_cc(tcpc, TYPEC_CC_RP);
	tcpci_sink_vbus(tcpc, TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_0V, 0);
	tcpc_enable_timer(tcpc, TYPEC_TRY_TIMER_DRP_TRY);
}

#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

/*
 * [BLOCK] Attach / Detach
 */

static inline void typec_cc_snk_detect_vsafe5v_entry(
	struct tcpc_device *tcpc)
{
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

	if (!typec_check_cc_any(TYPEC_CC_VOLT_OPEN)) {	/* Both Rp */
		typec_sink_dbg_acc_attached_entry(tcpc);
		return;
	}

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	if (typec_role_is_try_src(tcpc)) {
		if (tcpc->typec_state == typec_attachwait_snk) {
			typec_try_src_entry(tcpc);
			return;
		}
	}
#endif /* CONFIG_TYPEC_CAP_TRY_SOURCE */

	typec_sink_attached_entry(tcpc);
}

static inline void typec_cc_snk_detect_entry(struct tcpc_device *tcpc)
{
#ifdef CONFIG_HUAWEI_TCPC_HUSB311
	if (chip_id_is_husb311_did()) {
		/* otg-default hold lock step 3 */
		TYPEC_INFO("otg-default hold lock step 3:snk_detect \n");
		if (g_abnormal_var.abnormal_adp_det_cnt == ABNORMAL_ADP_DET_CNT_TWO) {
			g_abnormal_var.abnormal_adp_det_cnt = ABNORMAL_ADP_DET_CNT_THR;
			ktime_get_real_ts64(&g_abnormal_var.ts_time);
			g_abnormal_var.begin_2 = timespec_to_ms(g_abnormal_var.ts_time);
		}
	}
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */

	/* If Port Partner act as Source without VBUS, wait vSafe5V */
	if (tcpci_check_vbus_valid(tcpc))
		typec_cc_snk_detect_vsafe5v_entry(tcpc);
	else
		typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_SNK_VSAFE5V);
}

static inline void typec_cc_src_detect_vsafe0v_entry(
	struct tcpc_device *tcpc)
{
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if (typec_role_is_try_sink(tcpc)) {
		if (tcpc->typec_state == typec_attachwait_src) {
			typec_try_snk_entry(tcpc);
			return;
		}
	}
#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

	typec_source_attached_entry(tcpc);
}

static inline void typec_cc_src_detect_entry(
	struct tcpc_device *tcpc)
{
#ifdef CONFIG_HUAWEI_TCPC_HUSB311
	if (chip_id_is_husb311_did()) {
		/* otg-default hold lock step 4 */
		TYPEC_INFO("otg-default hold lock step 4:src_detect \n");
		ktime_get_real_ts64(&g_abnormal_var.ts_time);
		g_abnormal_var.end_2 = timespec_to_ms(g_abnormal_var.ts_time);
		if ((g_abnormal_var.abnormal_adp_det_cnt == ABNORMAL_ADP_DET_CNT_THR) &&
			((g_abnormal_var.end_2 - g_abnormal_var.begin_2) < TIMER_CNT_TWO_MS))
			return;
		if (g_abnormal_var.abnormal_adp_det_cnt == ABNORMAL_ADP_DET_CNT_THR)
			g_abnormal_var.abnormal_adp_det_cnt = ABNORMAL_ADP_DET_CNT_ZERO;
	}
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */

	/* If Port Partner act as Sink with low VBUS, wait vSafe0v */
	if (tcpci_check_vsafe0v(tcpc))
		typec_cc_src_detect_vsafe0v_entry(tcpc);
	else
		typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_SRC_VSAFE0V);
}

static inline void typec_cc_src_remove_entry(struct tcpc_device *tcpc)
{
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

	tcpc->typec_is_attached_src = false;

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	if (typec_role_is_try_src(tcpc)) {
		switch (tcpc->typec_state) {
		case typec_attached_src:
			typec_trywait_snk_pe_entry(tcpc);
			return;
		case typec_try_src:
			typec_trywait_snk_entry(tcpc);
			return;
		}
	}
#endif	/* CONFIG_TYPEC_CAP_TRY_SOURCE */

#ifdef CONFIG_HUAWEI_TCPC_HUSB311
	if (chip_id_is_husb311_did()) {
		/* otg-default hold lock step 2 */
		TYPEC_INFO("otg-default hold lock step 2:src_remove \n");
		ktime_get_real_ts64(&g_abnormal_var.ts_time);
		g_abnormal_var.end_1 = timespec_to_ms(g_abnormal_var.ts_time);
		if (((g_abnormal_var.end_1 - g_abnormal_var.begin_1) < TIMER_CNT_ONE_MS) &&
			(g_abnormal_var.abnormal_adp_det_cnt == ABNORMAL_ADP_DET_CNT_ONE))
			g_abnormal_var.abnormal_adp_det_cnt = ABNORMAL_ADP_DET_CNT_TWO;
	}
#endif /* CONFIG_HUAWEI_TCPC_HUSB311 */

	typec_unattach_wait_pe_idle_entry(tcpc);
}

static inline void typec_cc_snk_remove_entry(struct tcpc_device *tcpc)
{
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if (tcpc->typec_state == typec_try_snk) {
		typec_trywait_src_entry(tcpc);
		return;
	}
#endif	/* CONFIG_TYPEC_CAP_TRY_SINK */

	typec_unattach_wait_pe_idle_entry(tcpc);
}

/*
 * [BLOCK] CC Change (after debounce)
 */

#ifdef CONFIG_TYPEC_CAP_DBGACC
static void typec_debug_acc_attached_with_vbus_entry(
		struct tcpc_device *tcpc)
{
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);
	tcpc->typec_attach_new = TYPEC_ATTACHED_DEBUG;
}
#endif	/* CONFIG_TYPEC_CAP_DBGACC */

static inline void typec_debug_acc_attached_entry(struct tcpc_device *tcpc)
{
#ifdef CONFIG_TYPEC_CAP_DBGACC
	TYPEC_DBG("[Debug] CC1&2 Both Rd\n");
	TYPEC_NEW_STATE(typec_debugaccessory);
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DBG_VSAFE5V);

	tcpci_report_power_control(tcpc, true);
	tcpci_source_vbus(tcpc,
			TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SOURCE_5V, -1);
#endif	/* CONFIG_TYPEC_CAP_DBGACC */
}

#ifdef CONFIG_TYPEC_CAP_AUDIO_ACC_SINK_VBUS
static inline bool typec_audio_acc_sink_vbus(
	struct tcpc_device *tcpc, bool vbus_valid)
{
	if (vbus_valid) {
		tcpci_report_power_control(tcpc, true);
		tcpci_sink_vbus(tcpc,
			TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, 500);
	} else {
		tcpci_sink_vbus(tcpc,
			TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_0V, 0);
		tcpci_report_power_control(tcpc, false);
	}

	return true;
}
#endif	/* CONFIG_TYPEC_CAP_AUDIO_ACC_SINK_VBUS */

static inline void typec_audio_acc_attached_entry(struct tcpc_device *tcpc)
{
	TYPEC_DBG("[Audio] CC1&2 Both Ra\n");
	TYPEC_NEW_STATE(typec_audioaccessory);
	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);
	tcpc->typec_attach_new = TYPEC_ATTACHED_AUDIO;

#ifdef CONFIG_TYPEC_CAP_AUDIO_ACC_SINK_VBUS
	if (tcpci_check_vbus_valid(tcpc))
		typec_audio_acc_sink_vbus(tcpc, true);
#endif	/* CONFIG_TYPEC_CAP_AUDIO_ACC_SINK_VBUS */
}

static inline bool typec_cc_change_source_entry(struct tcpc_device *tcpc)
{
	bool src_remove = false;

	switch (tcpc->typec_state) {
	case typec_attached_src:
		if (typec_get_cc_res() != TYPEC_CC_VOLT_RD)
			src_remove = true;
		break;
	case typec_audioaccessory:
		if (!typec_check_cc_both(TYPEC_CC_VOLT_RA))
			src_remove = true;
		break;
#ifdef CONFIG_TYPEC_CAP_DBGACC
	case typec_debugaccessory:
		if (!typec_check_cc_both(TYPEC_CC_VOLT_RD))
			src_remove = true;
		break;
#endif	/* CONFIG_TYPEC_CAP_DBGACC */
	default:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RD))
			typec_debug_acc_attached_entry(tcpc);
		else if (typec_check_cc_both(TYPEC_CC_VOLT_RA))
			typec_audio_acc_attached_entry(tcpc);
		else if (typec_check_cc_any(TYPEC_CC_VOLT_RD))
			typec_cc_src_detect_entry(tcpc);
		else
			src_remove = true;
		break;
	}

	if (src_remove)
		typec_cc_src_remove_entry(tcpc);

	return true;
}

static inline bool typec_attached_snk_cc_change(struct tcpc_device *tcpc)
{
	uint8_t cc_res = typec_get_cc_res();
#ifdef CONFIG_USB_POWER_DELIVERY
	struct pd_port *pd_port = &tcpc->pd_port;
#endif	/* CONFIG_USB_POWER_DELIVERY */

	if (cc_res != tcpc->typec_remote_rp_level) {
		TYPEC_INFO("RpLvl Change\n");
		tcpc->typec_remote_rp_level = cc_res;

#ifdef CONFIG_USB_PD_REV30
		if (pd_port->pe_data.pd_connected && pd_check_rev30(pd_port))
			pd_put_sink_tx_event(tcpc, cc_res);
#endif /* CONFIG_USB_PD_REV30 */

		tcpci_sink_vbus(tcpc,
				TCP_VBUS_CTRL_TYPEC, TCPC_VBUS_SINK_5V, -1);
	}

	return true;
}

static inline bool typec_cc_change_sink_entry(struct tcpc_device *tcpc)
{
	bool snk_remove = false;

	switch (tcpc->typec_state) {
	case typec_attached_snk:
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK
	case typec_attached_dbgacc_snk:
#endif /* CONFIG_TYPEC_CAP_DBGACC_SNK */
		if (typec_get_cc_res() == TYPEC_CC_VOLT_OPEN)
			snk_remove = true;
		else
			typec_attached_snk_cc_change(tcpc);
		break;

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC
	case typec_attached_custom_src:
		if (typec_check_cc_any(TYPEC_CC_VOLT_OPEN))
			snk_remove = true;
		break;
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC */

	default:
		if (!typec_is_cc_open())
			typec_cc_snk_detect_entry(tcpc);
		else
			snk_remove = true;
	}

	if (snk_remove)
		typec_cc_snk_remove_entry(tcpc);

	return true;
}

bool tcpc_typec_is_act_as_sink_role(struct tcpc_device *tcpc)
{
	bool as_sink = true;
	uint8_t cc_sum;

	switch (TYPEC_CC_PULL_GET_RES(tcpc->typec_local_cc)) {
	case TYPEC_CC_RP:
		as_sink = false;
		break;
	case TYPEC_CC_RD:
		as_sink = true;
		break;
	case TYPEC_CC_DRP:
		cc_sum = typec_get_cc1() + typec_get_cc2();
		as_sink = (cc_sum >= TYPEC_CC_VOLT_SNK_DFT);
		break;
	}

	return as_sink;
}
EXPORT_SYMBOL(tcpc_typec_is_act_as_sink_role);

static inline bool typec_handle_cc_changed_entry(struct tcpc_device *tcpc)
{
	struct tcp_notify tcp_noti;

	TYPEC_INFO("[CC_Change] %d/%d\n", typec_get_cc1(), typec_get_cc2());

	tcpc->typec_attach_new = tcpc->typec_attach_old;

	if (tcpc_typec_is_act_as_sink_role(tcpc))
		typec_cc_change_sink_entry(tcpc);
	else
		typec_cc_change_source_entry(tcpc);

#ifdef CONFIG_TCPC_RT1715
	tcpc_check_notify_time(tcpc, &tcp_noti,
		TCP_NOTIFY_IDX_VBUS, TCP_NOTIFY_ABNORMAL_CC_CHANGE_HANDLER);
#endif

	typec_alert_attach_state_change(tcpc);
	return true;
}

/*
 * [BLOCK] Handle cc-change event (from HW)
 */

static inline void typec_attach_wait_entry(struct tcpc_device *tcpc)
{
	bool as_sink;
#ifdef CONFIG_USB_POWER_DELIVERY
	struct pd_port *pd_port = &tcpc->pd_port;
#endif /* CONFIG_USB_POWER_DELIVERY */

	if (tcpc->typec_attach_old == TYPEC_ATTACHED_SNK ||
	    tcpc->typec_attach_old == TYPEC_ATTACHED_DBGACC_SNK) {
#ifdef CONFIG_USB_PD_REV30
		if (pd_port->pe_data.pd_connected && pd_check_rev30(pd_port))
			pd_put_sink_tx_event(tcpc, typec_get_cc_res());
#endif	/* CONFIG_USB_PD_REV30 */
		tcpc_enable_timer(tcpc, TYPEC_TIMER_PDDEBOUNCE);
		TYPEC_DBG("RpLvl Alert\n");
		return;
	}

	if (tcpc->typec_attach_old ||
		tcpc->typec_state == typec_attached_src) {
		tcpc_reset_typec_debounce_timer(tcpc);
		TYPEC_DBG("Attached, Ignore cc_attach\n");
		typec_enable_vconn(tcpc);
		return;
	}

	switch (tcpc->typec_state) {

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	case typec_try_src:
		tcpc_enable_timer(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
		return;

	case typec_trywait_snk:
		tcpc_enable_timer(tcpc, TYPEC_TIMER_CCDEBOUNCE);
		return;
#endif

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_try_snk: /* typec_drp_try_timeout = true */
		tcpc_enable_timer(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
		return;

	case typec_trywait_src: /* typec_drp_try_timeout = unknown */
		tcpc_enable_timer(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
		return;
#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

#ifdef CONFIG_USB_POWER_DELIVERY
	case typec_unattachwait_pe:
		TYPEC_INFO("Force PE Idle\n");
		tcpc->pd_wait_pe_idle = false;
		tcpc_disable_timer(tcpc, TYPEC_RT_TIMER_PE_IDLE);
		typec_unattached_power_entry(tcpc);
		break;
#endif
	default:
		break;
	}

	as_sink = tcpc_typec_is_act_as_sink_role(tcpc);

#ifdef CONFIG_TYPEC_NOTIFY_ATTACHWAIT
	tcpci_notify_attachwait_state(tcpc, as_sink);
#endif /* CONFIG_TYPEC_NOTIFY_ATTACHWAIT */

	if (as_sink)
		TYPEC_NEW_STATE(typec_attachwait_snk);
	else {
		TYPEC_NEW_STATE(typec_attachwait_src);
		/* Advertise Rp level before Attached.SRC Ellisys 3.1.6359 */
		tcpci_set_cc(tcpc,
			TYPEC_CC_PULL(tcpc->typec_local_rp_level, TYPEC_CC_RP));
	}

	tcpc_enable_timer(tcpc, TYPEC_TIMER_CCDEBOUNCE);
}

static inline int typec_attached_snk_cc_detach(struct tcpc_device *tcpc)
{
	tcpc_reset_typec_debounce_timer(tcpc);
#ifdef CONFIG_USB_POWER_DELIVERY
	if (tcpc->pd_wait_hard_reset_complete) {
		TYPEC_INFO2("Detach_CC (HardReset)\n");
		tcpc_enable_timer(tcpc, TYPEC_TIMER_PDDEBOUNCE);
	} else if (tcpc->pd_port.pe_data.pd_prev_connected) {
		TYPEC_INFO2("Detach_CC (PD)\n");
		tcpc_enable_timer(tcpc, TYPEC_TIMER_PDDEBOUNCE);
	}
#endif /* CONFIG_USB_POWER_DELIVERY */
	return 0;
}

static inline void typec_detach_wait_entry(struct tcpc_device *tcpc)
{
	switch (tcpc->typec_state) {
	case typec_attached_snk:
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK
	case typec_attached_dbgacc_snk:
#endif /* CONFIG_TYPEC_CAP_DBGACC_SNK */
		typec_attached_snk_cc_detach(tcpc);
		break;

	case typec_attached_src:
		tcpc_enable_timer(tcpc, TYPEC_TIMER_SRCDISCONNECT);
		break;

	case typec_audioaccessory:
		tcpc_enable_timer(tcpc, TYPEC_TIMER_CCDEBOUNCE);
		break;

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	case typec_try_src:
		if (tcpc->typec_drp_try_timeout)
			tcpc_enable_timer(tcpc, TYPEC_TIMER_PDDEBOUNCE);
		else {
			tcpc_reset_typec_debounce_timer(tcpc);
			TYPEC_DBG("[Try] Ignore cc_detach\n");
		}
		break;
#endif /* CONFIG_TYPEC_CAP_TRY_SOURCE */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_trywait_src:
		if (tcpc->typec_drp_try_timeout)
			tcpc_enable_timer(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);
		else {
			tcpc_reset_typec_debounce_timer(tcpc);
			TYPEC_DBG("[Try] Ignore cc_detach\n");
		}
		break;
#endif /* CONFIG_TYPEC_CAP_TRY_SINK */
	default:
		tcpc_enable_timer(tcpc, TYPEC_TIMER_PDDEBOUNCE);
		break;
	}
}

static inline bool typec_is_cc_attach(struct tcpc_device *tcpc)
{
	bool cc_attach = false;
	int cc1 = typec_get_cc1();
	int cc2 = typec_get_cc2();
	int cc_res = typec_get_cc_res();

	switch (tcpc->typec_state) {
	case typec_attached_snk:
	case typec_attached_src:
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC
	case typec_attached_custom_src:
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC */
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK
		/* fall through */
	case typec_attached_dbgacc_snk:
#endif /* CONFIG_TYPEC_CAP_DBGACC_SNK */
		if ((cc_res != TYPEC_CC_VOLT_OPEN) &&
				(cc_res != TYPEC_CC_VOLT_RA))
			cc_attach = true;
		break;

	case typec_audioaccessory:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RA))
			cc_attach = true;
		break;

#ifdef CONFIG_TYPEC_CAP_DBGACC
	case typec_debugaccessory:
		if (typec_check_cc_both(TYPEC_CC_VOLT_RD))
			cc_attach = true;
		break;
#endif /* CONFIG_TYPEC_CAP_DBGACC */

	default: /* TYPEC_UNATTACHED */
		if (cc1 != TYPEC_CC_VOLT_OPEN)
			cc_attach = true;

		if (cc2 != TYPEC_CC_VOLT_OPEN)
			cc_attach = true;

		/* Cable Only, no device */
		if ((cc1+cc2) == TYPEC_CC_VOLT_RA) {
			cc_attach = false;
			TYPEC_INFO("[Cable] Ra Only\n");
		}
		break;
	}

	return cc_attach;
}

static inline int typec_enter_low_power_mode(struct tcpc_device *tcpc)
{
	int ret = 0;

	TYPEC_INFO("%s typec_lpm = %d\n", __func__, tcpc->typec_lpm);

	if (!tcpc->typec_lpm)
		return 0;

	ret = tcpci_set_low_power_mode(tcpc, true);
	if (ret < 0)
		tcpc_enable_lpm_timer(tcpc, true);

	return ret;
}

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
static inline int typec_handle_try_sink_cc_change(
	struct tcpc_device *tcpc)
{
	/*
	 * The port shall wait for tDRPTry and only then begin
	 * monitoring the CC1 and CC2 pins for the SNK.Rp state
	 */

	if (!tcpc->typec_drp_try_timeout) {
		TYPEC_INFO("[Try.SNK] Ignore CC_Alert\n");
		return 1;
	}

	if (!typec_is_cc_open()) {
		tcpci_notify_attachwait_state(tcpc, true);
		return 0;
	}

	return 0;
}
#endif	/* CONFIG_TYPEC_CAP_TRY_SINK */

int tcpc_typec_get_rp_present_flag(struct tcpc_device *tcpc)
{
	int rp_flag = 0;

	if (tcpc->typec_remote_cc[0] >= TYPEC_CC_VOLT_SNK_DFT
		&& tcpc->typec_remote_cc[0] != TYPEC_CC_DRP_TOGGLING)
		rp_flag |= 1;

	if (tcpc->typec_remote_cc[1] >= TYPEC_CC_VOLT_SNK_DFT
		&& tcpc->typec_remote_cc[1] != TYPEC_CC_DRP_TOGGLING)
		rp_flag |= 2;

	return rp_flag;
}

bool tcpc_typec_is_cc_open_state(struct tcpc_device *tcpc)
{
	TYPEC_DBG("%s %s\n", __func__,
		  tcpc->typec_state < ARRAY_SIZE(typec_state_names) ?
		  typec_state_names[tcpc->typec_state] : "Unknown");

	if (tcpc->typec_state == typec_disabled)
		return true;

	if (tcpc->typec_state == typec_errorrecovery)
		return true;

	if (typec_is_in_protection_states(tcpc))
		return true;

	return false;
}

static inline bool typec_is_ignore_cc_change(
	struct tcpc_device *tcpc, int rp_present)
{
	if (typec_is_drp_toggling())
		return true;

	if (tcpc_typec_is_cc_open_state(tcpc))
		return true;

#ifdef CONFIG_USB_POWER_DELIVERY
	if (tcpc->typec_state == typec_attachwait_snk &&
		tcpc_typec_get_rp_present_flag(tcpc) == rp_present) {
		TYPEC_DBG("[AttachWait] Ignore RpLvl Alert\n");
		return true;
	}

	if (tcpc->pd_wait_pr_swap_complete) {
		TYPEC_DBG("[PR.Swap] Ignore CC_Alert\n");
		return true;
	}
#endif /* CONFIG_USB_POWER_DELIVERY */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	if (tcpc->typec_state == typec_try_snk) {
		if (typec_handle_try_sink_cc_change(tcpc) > 0)
			return true;
	}

	if (tcpc->typec_state == typec_trywait_src_pe) {
		TYPEC_DBG("[Try.PE] Ignore CC_Alert\n");
		return true;
	}
#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	if (tcpc->typec_state == typec_trywait_snk_pe) {
		TYPEC_DBG("[Try.PE] Ignore CC_Alert\n");
		return true;
	}
#endif /* CONFIG_TYPEC_CAP_TRY_SOURCE */

	return false;
}

int tcpc_typec_handle_cc_change(struct tcpc_device *tcpc)
{
	int ret = 0, rp_present = tcpc_typec_get_rp_present_flag(tcpc);

	ret = tcpci_get_cc(tcpc);
	if (ret < 0)
		return ret;

#ifdef CONFIG_TCPC_RT1715
	if (typec_is_drp_toggling()) {
		TYPEC_INFO("[Waring] DRP Toggling\n");
		if (tcpc->typec_lpm)
			typec_enter_low_power_mode(tcpc);
		return 0;
	}
#endif
	TYPEC_INFO("[CC_Alert] %d/%d\n", typec_get_cc1(), typec_get_cc2());

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
#ifndef CONFIG_TCPC_RT1715
	if (!typec_try_enter_norp_src(tcpc))
		if (typec_try_exit_norp_src(tcpc))
			return 0;
#else
	typec_try_exit_norp_src(tcpc);
#endif
#endif /* CONFIG_TYPEC_CAP_NORP_SRC */

	if (typec_is_ignore_cc_change(tcpc, rp_present))
		return 0;

	if (tcpc->typec_state == typec_attachwait_snk
		|| tcpc->typec_state == typec_attachwait_src)
		typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

#ifdef CONFIG_TCPC_RT1715
	tcpc_usb_typec_cc_alert(typec_get_cc1(), typec_get_cc2());
#endif /* CONFIG_TCPC_RT1715 */

	if (typec_is_cc_attach(tcpc)) {
		typec_disable_low_power_mode(tcpc);
		typec_attach_wait_entry(tcpc);
	} else {
		typec_detach_wait_entry(tcpc);
	}

	return 0;
}

/*
 * [BLOCK] Handle timeout event
 */

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
static inline int typec_handle_drp_try_timeout(struct tcpc_device *tcpc)
{
	bool src_detect = false, en_timer;

	tcpc->typec_drp_try_timeout = true;
	tcpc_disable_timer(tcpc, TYPEC_TRY_TIMER_DRP_TRY);

	if (typec_is_drp_toggling()) {
		TYPEC_DBG("[Warning] DRP Toggling\n");
		return 0;
	}

	src_detect = typec_check_cc_any(TYPEC_CC_VOLT_RD);

	switch (tcpc->typec_state) {
#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	case typec_try_src:
		en_timer = !src_detect;
		break;
#endif /* CONFIG_TYPEC_CAP_TRY_SOURCE */

#ifdef CONFIG_TYPEC_CAP_TRY_SINK
	case typec_trywait_src:
		en_timer = !src_detect;
		break;

	case typec_try_snk:
		en_timer = true;
		if (!typec_is_cc_open())
			tcpci_notify_attachwait_state(tcpc, true);
		break;
#endif /* CONFIG_TYPEC_CAP_TRY_SINK */

	default:
		en_timer = false;
		break;
	}

	if (en_timer)
		tcpc_enable_timer(tcpc, TYPEC_TIMER_TRYCCDEBOUNCE);

	return 0;
}
#endif /* CONFIG_TYPEC_CAP_TRY_STATE */

static inline int typec_handle_debounce_timeout(struct tcpc_device *tcpc)
{
	if (typec_is_drp_toggling()) {
		TYPEC_DBG("[Warning] DRP Toggling\n");
		return 0;
	}

	typec_handle_cc_changed_entry(tcpc);
	return 0;
}

static inline int typec_handle_error_recovery_timeout(
						struct tcpc_device *tcpc)
{
#ifdef CONFIG_USB_POWER_DELIVERY
	tcpc->pd_wait_pe_idle = false;
#endif /* CONFIG_USB_POWER_DELIVERY */

	typec_unattach_wait_pe_idle_entry(tcpc);
	typec_alert_attach_state_change(tcpc);
	return 0;
}

#ifdef CONFIG_USB_POWER_DELIVERY
static inline int typec_handle_pe_idle(struct tcpc_device *tcpc)
{
	switch (tcpc->typec_state) {

#ifdef CONFIG_TYPEC_CAP_TRY_SOURCE
	case typec_trywait_snk_pe:
		typec_trywait_snk_entry(tcpc);
		break;
#endif

	case typec_unattachwait_pe:
		typec_unattached_entry(tcpc);
		break;

	default:
		TYPEC_DBG("Dummy pe_idle\n");
		break;
	}

	return 0;
}

#ifdef CONFIG_USB_PD_WAIT_BC12
static inline void typec_handle_pd_wait_bc12(struct tcpc_device *tcpc)
{
	int ret = 0;
	uint8_t type = TYPEC_UNATTACHED;
	union power_supply_propval val = {.intval = 0};

	mutex_lock(&tcpc->access_lock);

	type = tcpc->typec_attach_new;
	ret = power_supply_get_property(tcpc->usb_psy,
		POWER_SUPPLY_PROP_REAL_TYPE, &val);
	TYPEC_INFO("type=%d, ret,chg_type=%d,%d, count=%d\n", type,
		ret, val.intval, tcpc->pd_wait_bc12_count);

	if (type != TYPEC_ATTACHED_SNK && type != TYPEC_ATTACHED_DBGACC_SNK)
		goto out;

	if ((ret >= 0 && val.intval != POWER_SUPPLY_TYPE_UNKNOWN) ||
		tcpc->pd_wait_bc12_count >= 20) {
		__pd_put_cc_attached_event(tcpc, type);
	} else {
		tcpc->pd_wait_bc12_count++;
		tcpc_enable_timer(tcpc, TYPEC_RT_TIMER_PD_WAIT_BC12);
	}
out:
	mutex_unlock(&tcpc->access_lock);
}
#endif /* CONFIG_USB_PD_WAIT_BC12 */
#endif /* CONFIG_USB_POWER_DELIVERY */

static inline int typec_handle_src_reach_vsafe0v(struct tcpc_device *tcpc)
{
	typec_cc_src_detect_vsafe0v_entry(tcpc);
	typec_alert_attach_state_change(tcpc);
	return 0;
}

static inline int typec_handle_src_toggle_timeout(struct tcpc_device *tcpc)
{
#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	if (tcpc->typec_during_role_swap)
		return 0;
#endif /* CONFIG_TYPEC_CAP_ROLE_SWAP */

	if (tcpc->typec_state == typec_unattached_src) {
		typec_unattached_snk_and_drp_entry(tcpc);
		typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);
	}

	return 0;
}

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
static inline int typec_handle_role_swap_start(struct tcpc_device *tcpc)
{
	uint8_t role_swap = tcpc->typec_during_role_swap;

	if (role_swap == TYPEC_ROLE_SWAP_TO_SNK) {
		TYPEC_INFO("Role Swap to Sink\n");
		tcpci_set_cc(tcpc, TYPEC_CC_RD);
		tcpc_enable_timer(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
	} else if (role_swap == TYPEC_ROLE_SWAP_TO_SRC) {
		TYPEC_INFO("Role Swap to Source\n");
		tcpci_set_cc(tcpc, TYPEC_CC_RP);
		tcpc_enable_timer(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_STOP);
	}

	return 0;
}

static inline int typec_handle_role_swap_stop(struct tcpc_device *tcpc)
{
	if (tcpc->typec_during_role_swap) {
		TYPEC_INFO("TypeC Role Swap Failed\n");
		tcpc->typec_during_role_swap = TYPEC_ROLE_SWAP_NONE;
		tcpc_enable_timer(tcpc, TYPEC_TIMER_PDDEBOUNCE);
	}

	return 0;
}
#endif	/* CONFIG_TYPEC_CAP_ROLE_SWAP */

int tcpc_typec_handle_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	int ret = 0;

	TYPEC_INFO("%s:start, timer_id=%d\n", __func__, timer_id);

	if (timer_id >= TYPEC_TIMER_START_ID &&
	    tcpc_is_timer_active(tcpc, TYPEC_TIMER_START_ID, PD_TIMER_NR)) {
		TYPEC_DBG("[Type-C] Ignore timer_evt\n");
		return 0;
	}

	if (timer_id == TYPEC_TIMER_ERROR_RECOVERY)
		return typec_handle_error_recovery_timeout(tcpc);
	else if (timer_id == TYPEC_RT_TIMER_STATE_CHANGE)
		return typec_alert_attach_state_change(tcpc);
	else if (tcpc_typec_is_cc_open_state(tcpc)) {
		TYPEC_DBG("[Open] Ignore timer_evt\n");
		return 0;
	}

#ifdef CONFIG_TYPEC_CAP_TRY_STATE
	if (timer_id == TYPEC_TRY_TIMER_DRP_TRY)
		return typec_handle_drp_try_timeout(tcpc);
#endif	/* CONFIG_TYPEC_CAP_TRY_STATE */

#ifdef CONFIG_USB_POWER_DELIVERY
	if (tcpc->pd_wait_pr_swap_complete) {
		TYPEC_DBG("[PR.Swap] Ignore timer_evt\n");
		return 0;
	}
#endif /* CONFIG_USB_POWER_DELIVERY */
	switch (timer_id) {
	case TYPEC_TIMER_CCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
	case TYPEC_TIMER_TRYCCDEBOUNCE:
	case TYPEC_TIMER_SRCDISCONNECT:
		ret = typec_handle_debounce_timeout(tcpc);
		break;
#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	case TYPEC_TIMER_NORP_SRC:
#ifdef CONFIG_TCPC_RT1715
		ret = typec_handle_norp_src_debounce_timeout(tcpc);
#endif
		break;
#endif /* CONFIG_TYPEC_CAP_NORP_SRC */
	case TYPEC_TIMER_DRP_SRC_TOGGLE:
		ret = typec_handle_src_toggle_timeout(tcpc);
		break;

#ifdef CONFIG_USB_POWER_DELIVERY
	case TYPEC_RT_TIMER_PE_IDLE:
		ret = typec_handle_pe_idle(tcpc);
		break;
#ifdef CONFIG_USB_PD_WAIT_BC12
	case TYPEC_RT_TIMER_PD_WAIT_BC12:
		typec_handle_pd_wait_bc12(tcpc);
		break;
#endif /* CONFIG_USB_PD_WAIT_BC12 */
#endif /* CONFIG_USB_POWER_DELIVERY */

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_DELAY
	case TYPEC_RT_TIMER_SAFE0V_DELAY:
		ret = typec_handle_src_reach_vsafe0v(tcpc);
		break;
#endif /* CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_DELAY */

	case TYPEC_RT_TIMER_LOW_POWER_MODE:
		typec_enter_low_power_mode(tcpc);
		break;

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT
	case TYPEC_RT_TIMER_SAFE0V_TOUT:
		TCPC_INFO("VSafe0V TOUT (%d)\n", tcpc->vbus_level);

		if (!tcpci_check_vbus_valid_from_ic(tcpc))
			ret = tcpc_typec_handle_vsafe0v(tcpc);
		break;
#endif /* CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_TIMEOUT */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
	case TYPEC_RT_TIMER_ROLE_SWAP_START:
		typec_handle_role_swap_start(tcpc);
		break;

	case TYPEC_RT_TIMER_ROLE_SWAP_STOP:
		typec_handle_role_swap_stop(tcpc);
		break;
#endif /* CONFIG_TYPEC_CAP_ROLE_SWAP */

	case TYPEC_RT_TIMER_DISCHARGE:
		if (!tcpc->typec_power_ctrl) {
			mutex_lock(&tcpc->access_lock);
			tcpci_enable_auto_discharge(tcpc, false);
			tcpci_enable_force_discharge(tcpc, false, 0);
			mutex_unlock(&tcpc->access_lock);
		}
		break;
	}

	return ret;
}

/*
 * [BLOCK] Handle ps-change event
 */

static inline int typec_handle_vbus_present(struct tcpc_device *tcpc)
{
	switch (tcpc->typec_wait_ps_change) {
	case TYPEC_WAIT_PS_SNK_VSAFE5V:
		typec_cc_snk_detect_vsafe5v_entry(tcpc);
		typec_alert_attach_state_change(tcpc);
		break;
	case TYPEC_WAIT_PS_SRC_VSAFE5V:
		typec_source_attached_with_vbus_entry(tcpc);
		typec_alert_attach_state_change(tcpc);
		break;
#ifdef CONFIG_TYPEC_CAP_DBGACC
	case TYPEC_WAIT_PS_DBG_VSAFE5V:
		typec_debug_acc_attached_with_vbus_entry(tcpc);
		typec_alert_attach_state_change(tcpc);
		break;
#endif /* CONFIG_TYPEC_CAP_DBGACC */
	}

	return 0;
}

static inline int typec_attached_snk_vbus_absent(struct tcpc_device *tcpc)
{
#ifdef CONFIG_USB_POWER_DELIVERY
#ifdef CONFIG_USB_PD_DIRECT_CHARGE
	if (tcpc->pd_during_direct_charge && !tcpci_check_vsafe0v(tcpc)) {
		TYPEC_DBG("Ignore vbus_absent(snk), DirectCharge\n");
		return 0;
	}
#endif /* CONFIG_USB_PD_DIRECT_CHARGE */

	if (tcpc->pd_wait_hard_reset_complete &&
	    typec_get_cc_res() != TYPEC_CC_VOLT_OPEN) {
		TYPEC_DBG("Ignore vbus_absent(snk), HReset & CC!=0\n");
		return 0;
	}

	if (tcpc->pd_port.pe_data.pd_prev_connected &&
	    tcpc->pd_exit_attached_snk_via_cc) {
		TYPEC_DBG("Ignore vbus_absent(snk), PD & CC!=0\n");
		return 0;
	}
#endif /* CONFIG_USB_POWER_DELIVERY */

	typec_unattach_wait_pe_idle_entry(tcpc);
	typec_alert_attach_state_change(tcpc);

	return 0;
}


static inline int typec_handle_vbus_absent(struct tcpc_device *tcpc)
{
#ifdef CONFIG_USB_POWER_DELIVERY
	if (tcpc->pd_wait_pr_swap_complete) {
		TYPEC_DBG("[PR.Swap] Ignore vbus_absent\n");
		return 0;
	}
#endif /* CONFIG_USB_POWER_DELIVERY */

	switch (tcpc->typec_state) {
	case typec_attached_snk:
#ifdef CONFIG_TYPEC_CAP_DBGACC_SNK
	case typec_attached_dbgacc_snk:
#endif /* CONFIG_TYPEC_CAP_DBGACC_SNK */
		typec_attached_snk_vbus_absent(tcpc);
		break;
	default:
		break;
	}

	return 0;
}

int tcpc_typec_handle_ps_change(struct tcpc_device *tcpc, int vbus_level)
{
	tcpci_notify_ps_change(tcpc, vbus_level);

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
#ifdef CONFIG_TCPC_RT1715
	if (typec_is_cc_no_res() || typec_is_cc_ra()) {
#endif
		if (!typec_try_enter_norp_src(tcpc)) {
			/* vbus_only */
			if (typec_norp_src_detached_entry(tcpc))
				return 0;
		}
	}
#endif /* CONFIG_TYPEC_CAP_NORP_SRC */

#ifdef CONFIG_TYPEC_CAP_AUDIO_ACC_SINK_VBUS
	if (tcpc->typec_state == typec_audioaccessory) {
		return typec_audio_acc_sink_vbus(
			tcpc, vbus_level >= TCPC_VBUS_VALID);
	}
#endif /* CONFIG_TYPEC_CAP_AUDIO_ACC_SINK_VBUS */

	if (vbus_level >= TCPC_VBUS_VALID)
		return typec_handle_vbus_present(tcpc);

	return typec_handle_vbus_absent(tcpc);
}

/*
 * [BLOCK] Handle PE event
 */

#ifdef CONFIG_USB_POWER_DELIVERY

int tcpc_typec_handle_pe_pr_swap(struct tcpc_device *tcpc)
{
	int ret = 0;

	tcpci_lock_typec(tcpc);
	switch (tcpc->typec_state) {
	case typec_attached_snk:
		TYPEC_NEW_STATE(typec_attached_src);
		tcpc->typec_is_attached_src = true;
		tcpc->typec_attach_new = TYPEC_ATTACHED_SRC;
		tcpci_set_cc(tcpc,
			TYPEC_CC_PULL(tcpc->typec_local_rp_level, TYPEC_CC_RP));
		break;
	case typec_attached_src:
		TYPEC_NEW_STATE(typec_attached_snk);
		tcpc->typec_is_attached_src = false;
		tcpc->typec_attach_new = TYPEC_ATTACHED_SNK;
		tcpci_set_cc(tcpc, TYPEC_CC_RD);
		break;
	default:
		break;
	}

	typec_alert_attach_state_change(tcpc);
	tcpci_unlock_typec(tcpc);
	return ret;
}

#endif /* CONFIG_USB_POWER_DELIVERY */

/*
 * [BLOCK] Handle reach vSafe0V event
 */

int tcpc_typec_handle_vsafe0v(struct tcpc_device *tcpc)
{
	if (tcpc->typec_wait_ps_change != TYPEC_WAIT_PS_SRC_VSAFE0V)
		return 0;

	typec_wait_ps_change(tcpc, TYPEC_WAIT_PS_DISABLE);

	if (typec_is_drp_toggling()) {
		TYPEC_DBG("[Warning] DRP Toggling\n");
		return 0;
	}

	if (tcpc_typec_is_cc_open_state(tcpc))
		return 0;

#ifdef CONFIG_TYPEC_ATTACHED_SRC_SAFE0V_DELAY
	tcpc_enable_timer(tcpc, TYPEC_RT_TIMER_SAFE0V_DELAY);
#else
	typec_handle_src_reach_vsafe0v(tcpc);
#endif

	return 0;
}

/*
 * [BLOCK] TCPCI TypeC I/F
 */

#if TYPEC_INFO_ENABLE
static const char *const typec_role_name[] = {
	"UNKNOWN",
	"SNK",
	"SRC",
	"DRP",
	"TrySRC",
	"TrySNK",
};
#endif /* TYPEC_INFO_ENABLE */

#ifdef CONFIG_TYPEC_CAP_ROLE_SWAP
int tcpc_typec_swap_role(struct tcpc_device *tcpc)
{
	if (tcpc->typec_role < TYPEC_ROLE_DRP)
		return TCPM_ERROR_NOT_DRP_ROLE;

	if (tcpc->typec_during_role_swap)
		return TCPM_ERROR_DURING_ROLE_SWAP;

	switch (tcpc->typec_attach_old) {
	case TYPEC_ATTACHED_SNK:
		tcpc->typec_during_role_swap = TYPEC_ROLE_SWAP_TO_SRC;
		break;
	case TYPEC_ATTACHED_SRC:
		tcpc->typec_during_role_swap = TYPEC_ROLE_SWAP_TO_SNK;
		break;
	}

	if (tcpc->typec_during_role_swap) {
		TYPEC_INFO("TypeC Role Swap Start\n");
		tcpci_set_cc(tcpc, TYPEC_CC_OPEN);
		tcpc_enable_timer(tcpc, TYPEC_RT_TIMER_ROLE_SWAP_START);
		return TCPM_SUCCESS;
	}

	return TCPM_ERROR_UNATTACHED;
}
#endif /* CONFIG_TYPEC_CAP_ROLE_SWAP */

int tcpc_typec_set_rp_level(struct tcpc_device *tcpc, uint8_t rp_lvl)
{
	switch (rp_lvl) {
	case TYPEC_RP_DFT:
	case TYPEC_RP_1_5:
	case TYPEC_RP_3_0:
		TYPEC_INFO("TypeC-Rp: %d\n", rp_lvl);
		tcpc->typec_local_rp_level = rp_lvl;
		break;
	default:
		TYPEC_INFO("TypeC-Unknown-Rp (%d)\n", rp_lvl);
		return -EINVAL;
	}

	return 0;
}

int tcpc_typec_error_recovery(struct tcpc_device *tcpc)
{
	if (tcpc->typec_state != typec_errorrecovery)
		typec_error_recovery_entry(tcpc);

	return 0;
}

int tcpc_typec_disable(struct tcpc_device *tcpc)
{
	if (tcpc->typec_state != typec_disabled)
		typec_disable_entry(tcpc);

	return 0;
}

int tcpc_typec_enable(struct tcpc_device *tcpc)
{
	if (tcpc->typec_state == typec_disabled)
		typec_unattached_entry(tcpc);

	return 0;
}

int tcpc_typec_change_role(
	struct tcpc_device *tcpc, uint8_t typec_role, bool postpone)
{
	if (typec_role == TYPEC_ROLE_UNKNOWN ||
		typec_role >= TYPEC_ROLE_NR) {
		TYPEC_INFO("Wrong TypeC-Role: %d\n", typec_role);
		return -EINVAL;
	}

	if (tcpc->typec_role_new == typec_role) {
		TYPEC_INFO("typec_new_role: %s is the same\n",
			typec_role_name[typec_role]);
		return 0;
	}
	tcpc->typec_role_new = typec_role;

	TYPEC_INFO("typec_new_role: %s\n", typec_role_name[typec_role]);

	if (tcpc_typec_is_cc_open_state(tcpc))
		return 0;

	if (!postpone || tcpc->typec_attach_old == TYPEC_UNATTACHED)
		return tcpc_typec_error_recovery(tcpc);
	else
		return 0;
}

#ifdef CONFIG_TCPC_RT1715
static int tcpm_pd_dpm_get_src_extcap(void *client)
{
	struct tcpc_device *tcpc = (struct tcpc_device *)client;

	if (!tcpc)
		return 0;

	return (tcpc->pd_port.local_src_cap_default.nr > 1) ? 1 : 0;
}

static void tcpm_pd_dpm_set_src_cap(void *client, struct otg_cap *allow_ext, int send)
{
	struct tcpc_device *tcpc = (struct tcpc_device *)client;
	int ret;

	if (!tcpc)
		return;

	TYPEC_INFO("set src cap, vol=%d, cur =%d, send=%d\n",
		allow_ext->otg_vol, allow_ext->otg_cur, send);

	tcpc->pd_port.ext_src_cap.otg_vol = allow_ext->otg_vol;
	tcpc->pd_port.ext_src_cap.otg_cur = allow_ext->otg_cur;

	if (tcpc->pd_port.ext_src_cap.otg_vol == SRC_CAP_DEFAULT_VOL)
		tcpc->pd_port.ps_rdy_delay = 0;

	if (send) {
		ret = tcpm_dpm_pd_source_cap(tcpc, NULL);
		TYPEC_INFO("set src cap, send src cap ret %d\n", ret);
	}
}

static void tcpm_pd_dpm_set_src_psrdelay(void *client, int delay)
{
	struct tcpc_device *tcpc = (struct tcpc_device *)client;

	if (!tcpc)
		return;

	TYPEC_INFO("set ps rdy delay, delay=%d\n", delay);
	tcpc->pd_port.ps_rdy_delay = delay;
}

static struct pd_dpm_ops tcpc_device_pd_dpm_ops = {
	.pd_dpm_notify_direct_charge_status = tcpm_set_direct_charge_enable,
	.pd_dpm_get_src_extcap = tcpm_pd_dpm_get_src_extcap,
	.pd_dpm_set_src_cap = tcpm_pd_dpm_set_src_cap,
	.pd_dpm_set_src_psrdelay = tcpm_pd_dpm_set_src_psrdelay,
};
#endif

int tcpc_typec_init(struct tcpc_device *tcpc, uint8_t typec_role)
{
	int ret = 0;

	if (typec_role == TYPEC_ROLE_UNKNOWN ||
		typec_role >= TYPEC_ROLE_NR) {
		TYPEC_INFO("Wrong TypeC-Role: %d\n", typec_role);
		return -EINVAL;
	}

#ifdef CONFIG_TCPC_RT1715
	TYPEC_INFO("pd_dpm_ops_register_rt1715\n");
	pd_dpm_ops_register(&tcpc_device_pd_dpm_ops, tcpc);
#endif
	TYPEC_INFO("typec_init: %s\n", typec_role_name[typec_role]);

	tcpc->typec_role = typec_role;
	tcpc->typec_role_new = typec_role;
	typec_attach_new_unattached(tcpc);
	tcpc->typec_attach_old = TYPEC_UNATTACHED;

	mutex_lock(&tcpc->access_lock);
	tcpc->wake_lock_pd = 0;
	mutex_unlock(&tcpc->access_lock);

	ret = tcpci_get_cc(tcpc);

#ifdef CONFIG_TYPEC_CAP_NORP_SRC
	tcpc->no_rpsrc_state = 0;
	if (!tcpci_check_vbus_valid(tcpc) || ret < 0)
		tcpc->typec_power_ctrl = true;
#else
	if (!tcpci_check_vbus_valid(tcpc) || ret < 0 || typec_is_cc_no_res())
		tcpc->typec_power_ctrl = true;
#endif /* CONFIG_TYPEC_CAP_NORP_SRC */

	typec_unattached_entry(tcpc);
	return ret;
}

void tcpc_typec_deinit(struct tcpc_device *tcpc)
{
}
