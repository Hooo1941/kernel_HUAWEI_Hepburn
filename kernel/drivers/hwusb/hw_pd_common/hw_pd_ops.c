/*
 * hw_pd_ops.c
 *
 * Source file of external operation interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>

#include <huawei_platform/log/hw_log.h>

#include <chipset_common/hwusb/hw_pd/hw_pd_state.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_core.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_ops.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_interface.h>

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_pd_ops
HWLOG_REGIST();
#endif

/* External functions for operating to PD/TYPEC chip */
static struct pd_dpm_ops *g_pd_dpm_ops = NULL;
/* Context for external functions above */
static void *g_pd_dpm_client = NULL;

/* External function to check whether water is in typec port */
static struct water_det_ops* g_water_det_ops = NULL;

/* External function to check typec cc pins */
static struct cc_check_ops* g_cc_check_ops = NULL;

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
/* External function to check cable vdo */
static struct cable_vdo_ops *g_cable_vdo_ops = NULL;
#endif

void hw_pd_start_data_role_swap(void)
{
	if (!g_pd_dpm_ops || !g_pd_dpm_ops->data_role_swap || !g_pd_dpm_client)
		return;

	g_pd_dpm_ops->data_role_swap(g_pd_dpm_client);
}

void hw_pd_emark_detect(void)
{
	hwlog_err("%s\n", __func__);

	if (g_pd_dpm_ops && g_pd_dpm_ops->pd_dpm_detect_emark_cable)
		g_pd_dpm_ops->pd_dpm_detect_emark_cable(g_pd_dpm_client);
}

void hw_pd_emark_finish(void)
{
	if (!g_pd_dpm_ops || !g_pd_dpm_ops->data_role_swap ||
		!g_pd_dpm_client || !g_pd_dpm_ops->pd_dpm_inquire_usb_comm)
		return;

	if (g_pd_dpm_ops->pd_dpm_inquire_usb_comm(g_pd_dpm_client) &&
		!g_pd_dpm_ops->data_role_swap(g_pd_dpm_client)) {
		hwlog_info("emark finish data_role_swap\n");
		msleep(50); /* wait 50ms for swap complete */
	}
}

int hw_pd_disable_pd_comm(bool disable)
{
	hwlog_info("%s\n", __func__);

	if (!g_pd_dpm_ops) {
		hwlog_err("%s g_pd_dpm_ops is NULL\n", __func__);
		return -EPERM;
	}

	if (!g_pd_dpm_ops->pd_dpm_disable_pd) {
		hwlog_err("%s pd_dpm_disable_pd is NULL\n", __func__);
		return -EPERM;
	}

	return g_pd_dpm_ops->pd_dpm_disable_pd(g_pd_dpm_client, disable);
}

int hw_pd_set_cc_mode(int mode)
{
	static int cur_mode = PD_DPM_CC_MODE_DRP;

	hwlog_info("%s cur=%d, new=%d\n", __func__, cur_mode, mode);
	if (cur_mode == mode)
		return 0;

	if (!g_pd_dpm_ops || !g_pd_dpm_ops->pd_dpm_set_cc_mode) {
		hwlog_err("%s func not set\n", __func__);
		return HW_PD_INVALID_STATE;
	}

	g_pd_dpm_ops->pd_dpm_set_cc_mode(mode);
	cur_mode = mode;
	return 0;
}

/*
 * bugfix: For Hi65xx
 * DRP lost Cable(without adapter) plugin during Wireless.
 */
void hw_pd_set_drp_state(int mode)
{
	if (g_pd_dpm_ops && g_pd_dpm_ops->pd_dpm_enable_drp)
		g_pd_dpm_ops->pd_dpm_enable_drp(mode);
}

void hw_pd_reinit_chip(void)
{
	if (g_pd_dpm_ops && g_pd_dpm_ops->pd_dpm_reinit_chip)
		g_pd_dpm_ops->pd_dpm_reinit_chip(g_pd_dpm_client);
}

bool hw_pd_get_hw_docksvidexist(void)
{
	if (g_pd_dpm_ops && g_pd_dpm_ops->pd_dpm_get_hw_dock_svid_exist)
		return g_pd_dpm_ops->pd_dpm_get_hw_dock_svid_exist(g_pd_dpm_client);

	return false;
}

int hw_pd_set_dc_status(bool dc)
{
	hwlog_err("%s,%d", __func__, __LINE__);
	if (g_pd_dpm_ops && g_pd_dpm_ops->pd_dpm_notify_direct_charge_status) {
		hwlog_err("%s,%d", __func__, __LINE__);
		return g_pd_dpm_ops->pd_dpm_notify_direct_charge_status(g_pd_dpm_client, dc);
	}

	return false;
}

int hw_pd_query_cc_state(unsigned int *cc)
{
	if (!g_pd_dpm_ops || !g_pd_dpm_ops->pd_dpm_get_cc_state || !cc)
		return -1;

	*cc = g_pd_dpm_ops->pd_dpm_get_cc_state();
	return 0;
}

int hw_pd_check_cc_short(bool *check)
{
	if (g_pd_dpm_ops && g_pd_dpm_ops->pd_dpm_check_cc_vbus_short && check) {
		*check = g_pd_dpm_ops->pd_dpm_check_cc_vbus_short();
		hwlog_info("%s ret %d", __func__, (*check));
		return 0;
	}

	return HW_PD_INVALID_STATE;
}

int hw_pd_reg_dpm_ops(struct pd_dpm_ops *ops, void *client)
{
	int ret = 0;

	if (!ops) {
		hwlog_err("pd_dpm ops register fail\n");
		return -EPERM;
	}

	/*
	 * VXXX tcpc dirver lauchs earlier than rt1715 driver(external pd)
	 * and it shall call this function. Overwirte the callback handle
	 * while rt1715 driver calling this
	 */
	if (g_pd_dpm_ops && !pd_state(external_pd_flag)) {
		hwlog_err("pd_dpm ops register fail! g_pd_dpm_ops busy\n");
		return -EBUSY;
	}

	g_pd_dpm_ops = ops;
	g_pd_dpm_client = client;

	return ret;
}


int hw_pd_check_wather_detect(void)
{
	if (!g_water_det_ops || !g_water_det_ops->is_water_detected) {
		hwlog_info("%s no check ops\n", __func__);
		return HW_PD_INVALID_STATE;
	}

	/* If water is not detected, ignore current event */
	return g_water_det_ops->is_water_detected();
}

int hw_pd_reg_water_ops(struct water_det_ops* ops)
{
	int ret = 0;

	if (g_water_det_ops) {
		hwlog_err("water_det_ops register fail! g_water_det_ops busy\n");
		return -EBUSY;
	}

	if (ops) {
		g_water_det_ops = ops;
	} else {
		hwlog_err("water_det_ops register fail\n");
		ret = -EPERM;
	}
	return ret;
}

int hw_pd_dc_cable_detect(void)
{
	int ret;

	if (!g_cc_check_ops)
		return HW_PD_INVALID_STATE;
	ret = g_cc_check_ops->is_cable_for_direct_charge();
	if (ret) {
		hwlog_info("%s:cc_check fail\n", __func__);
		return HW_PD_INVALID_STATE;
	}
	return 0;
}

int hw_pd_reg_cc_ops(struct cc_check_ops* ops)
{
	int ret = 0;

	if (g_cc_check_ops) {
		hwlog_err("cc_check ops register fail! g_cc_check_ops busy\n");
		return -EBUSY;
	}

	if (ops) {
		g_cc_check_ops = ops;
	} else {
		hwlog_err("cc_check ops register fail\n");
		ret = -EPERM;
	}
	return ret;
}

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
bool hw_pd_is_custsrc2_cable(void)
{
	int ret;
	if (g_cable_vdo_ops && g_cable_vdo_ops->is_cust_src2_cable) {
		ret = g_cable_vdo_ops->is_cust_src2_cable();
		hwlog_info("%s: return %d\n", __func__, ret);
		return (ret != 0);
	}
	return false;
}

int hw_pd_reg_cablevdo_ops(struct cable_vdo_ops *ops)
{
	if (!ops) {
		hwlog_err("cable_vdo_ops ops register fail\n");
		return -EPERM;
	}

	g_cable_vdo_ops = ops;
	return 0;
}
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
