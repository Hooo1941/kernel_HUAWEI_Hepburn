/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include "dpu_dp_dbg.h"
#include "dp_ctrl.h"
#include "dp_aux.h"
#include <huawei_platform/dp_source/dp_source_switch.h>

/*******************************************************************************
 * DP GRAPHIC DEBUG TOOL
 */

static uint32_t g_dp_id = 0;
/* 27M, 720*480 */
uint32_t g_pixel_clock = 27000;
uint32_t g_video_code = 3;
uint32_t g_hblank = 138;
/*
 * for debug, S_IRUGO
 * /sys/module/dp_ctrl/parameters
 */
int g_dp_debug_mode_enable = 0;
int g_dp_lane_num_debug = 4;
int g_dp_lane_rate_debug = 2;
int g_dp_vactive_pixels_debug = 0;
int g_dp_hactive_pixels_debug = 3840;
int g_dp_ssc_enable_debug = 1;
int g_dp_fec_enable_debug = 0;
int g_dp_dsc_enable_debug = 0;
int g_dp_mst_enable_debug = 0;
int g_dp_efm_enable_debug = 1;
uint g_dp_aux_ctrl = 0x11630; // 0x77630;
uint g_pll1ckgctrlr0 = 0x054610df; // 0x054610df
uint g_pll1ckgctrlr1 = 0x046510da; // 0x046510da
uint g_pll1ckgctrlr2 = 0x046500d5; // 0x046500d5
int g_dp_same_source_debug = 1;
static uint32_t g_dp_preemphasis_swing[DPTX_COMBOPHY_PARAM_NUM];
int g_dp_pe_sw_length = 0;
/* pll1sscg_ctrl_initial_r(2), pll1sscg_cnt_r(2), pll1sscg_cnt2_r(2), PLL1SSCG_CTRL */
uint32_t g_hbr2_pll1_sscg[4] = {0x8CA00000, 0x011c0119, 0x000131, 0x7ff0e0d};
int g_hbr2_pll1_sscg_length = 0;
/* rate and ssc config, rate_coef0, rate_coef1, rate_coef2 */
uint32_t g_rbr_pll1_rate0_coef[3] = {0x2A300000, 0x4E015A62, 0x0011882A};
int g_rbr_pll1_rate0_coef_length = 0;
uint32_t g_hbr_pll1_rate1_coef[3] = {0x23280000, 0x4E412262, 0x00110023};
int g_hbr_pll1_rate1_coef_length = 0;
uint32_t g_hbr2_pll1_rate2_coef[3] = {0x23280000, 0x4E412262, 0x00100023};
int g_hbr2_pll1_rate2_coef_length = 0;
uint32_t g_hbr3_pll1_rate3_coef[3] = {0x34BC0000, 0x4E01AA62, 0x00100834};
int g_hbr3_pll1_rate3_coef_length = 0;
int g_clk_hblank_code_enable = 0;

#ifdef CONFIG_DKMD_DEBUG_ENABLE
module_param_named(dp_debug_mode_enable, g_dp_debug_mode_enable, int, 0640);
MODULE_PARM_DESC(dp_debug_mode_enable, "dp_debug_mode_enable");

module_param_named(dp_lane_num, g_dp_lane_num_debug, int, 0640);
MODULE_PARM_DESC(dp_lane_num, "dp_lane_num");

module_param_named(dp_lane_rate, g_dp_lane_rate_debug, int, 0640);
MODULE_PARM_DESC(dp_lane_rate, "dp_lane_rate");

module_param_named(dp_vactive_pixels, g_dp_vactive_pixels_debug, int, 0640);
MODULE_PARM_DESC(dp_vactive_pixels, "dp_vactive_pixels");

module_param_named(dp_hactive_pixels, g_dp_hactive_pixels_debug, int, 0640);
MODULE_PARM_DESC(dp_hactive_pixels, "dp_hactive_pixels");

module_param_named(dp_ssc_enable, g_dp_ssc_enable_debug, int, 0640);
MODULE_PARM_DESC(dp_ssc_enable, "dp_ssc_enable");

module_param_named(dp_fec_enable, g_dp_fec_enable_debug, int, 0640);
MODULE_PARM_DESC(dp_fec_enable, "dp_fec_enable");

module_param_named(dp_dsc_enable, g_dp_dsc_enable_debug, int, 0640);
MODULE_PARM_DESC(dp_dsc_enable, "dp_dsc_enable");

module_param_named(dp_mst_enable, g_dp_mst_enable_debug, int, 0640);
MODULE_PARM_DESC(dp_mst_enable, "dp_mst_enable");

module_param_named(dp_efm_enable, g_dp_efm_enable_debug, int, 0640);
MODULE_PARM_DESC(dp_efm_enable, "dp_efm_enable");

module_param_named(dp_aux_ctrl, g_dp_aux_ctrl, uint, 0640);
MODULE_PARM_DESC(dp_aux_ctrl, "dp_aux_ctrl");

module_param_named(pll1ckgctrlr0, g_pll1ckgctrlr0, uint, 0640);
MODULE_PARM_DESC(pll1ckgctrlr0, "pll1ckgctrlr0");

module_param_named(pll1ckgctrlr1, g_pll1ckgctrlr1, uint, 0640);
MODULE_PARM_DESC(pll1ckgctrlr1, "pll1ckgctrlr1");

module_param_named(pll1ckgctrlr2, g_pll1ckgctrlr2, uint, 0640);
MODULE_PARM_DESC(pll1ckgctrlr2, "pll1ckgctrlr2");

module_param_named(dp_same_source_debug, g_dp_same_source_debug, int, 0640);
MODULE_PARM_DESC(dp_same_source_debug, "dp_same_source_debug");

module_param_array_named(dp_preemphasis_swing, g_dp_preemphasis_swing, uint, &g_dp_pe_sw_length, 0640);
MODULE_PARM_DESC(dp_preemphasis_swing, "dp_preemphasis_swing");

module_param_array_named(hbr2_pll1_sscg, g_hbr2_pll1_sscg, uint, &g_hbr2_pll1_sscg_length, 0640);
MODULE_PARM_DESC(hbr2_pll1_sscg, "hbr2_pll1_sscg");

module_param_array_named(rbr_pll1_rate0_coef, g_rbr_pll1_rate0_coef, uint, &g_rbr_pll1_rate0_coef_length, 0640);
MODULE_PARM_DESC(rbr_pll1_rate0_coef, "rbr_pll1_rate0_coef");

module_param_array_named(hbr_pll1_rate1_coef, g_hbr_pll1_rate1_coef, uint, &g_hbr_pll1_rate1_coef_length, 0640);
MODULE_PARM_DESC(hbr_pll1_rate1_coef, "hbr_pll1_rate1_coef");

module_param_array_named(hbr2_pll1_rate2_coef, g_hbr2_pll1_rate2_coef, uint, &g_hbr2_pll1_rate2_coef_length, 0640);
MODULE_PARM_DESC(hbr2_pll1_rate2_coef, "hbr2_pll1_rate2_coef");

module_param_array_named(hbr3_pll1_rate3_coef, g_hbr3_pll1_rate3_coef, uint, &g_hbr3_pll1_rate3_coef_length, 0640);
MODULE_PARM_DESC(hbr3_pll1_rate3_coef, "hbr3_pll1_rate3_coef");

module_param_named(dp_clk_hblank_code_enable, g_clk_hblank_code_enable, int, 0640);
MODULE_PARM_DESC(dp_clk_hblank_code_enable, "dp_clk_hblank_code_enable");
#endif

static struct dp_ctrl *g_dptx_debug[2];

void dp_graphic_debug_node_init(struct dp_ctrl *dptx)
{
	g_dptx_debug[dptx->id] = dptx;
	g_dp_debug_mode_enable = 0;
}

static bool dp_lane_num_valid(int dp_lane_num_debug)
{
	if (dp_lane_num_debug == 1 || dp_lane_num_debug == 2 || dp_lane_num_debug == 4)
		return true;

	return false;
}

static bool dp_lane_rate_valid(int dp_lane_rate_debug)
{
	if (dp_lane_rate_debug >= 0 && dp_lane_rate_debug <= 3)
		return true;

	return false;
}

void dptx_debug_set_params(struct dp_ctrl *dptx)
{
	int i;

	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is null pointor!\n");

	if (g_dp_debug_mode_enable == 0)
		return;

	if (dp_lane_num_valid(g_dp_lane_num_debug))
		dptx->max_lanes = (uint8_t)g_dp_lane_num_debug;

	if (dp_lane_rate_valid(g_dp_lane_rate_debug))
		dptx->max_rate = (uint8_t)g_dp_lane_rate_debug;

	dptx->mst = (bool)g_dp_mst_enable_debug;
	dptx->ssc_en = (bool)g_dp_ssc_enable_debug;
	dptx->fec = (bool)g_dp_fec_enable_debug;
	dptx->dsc = (bool)g_dp_dsc_enable_debug;
	dptx->efm_en = (bool)g_dp_efm_enable_debug;

	if (g_dp_pe_sw_length == DPTX_COMBOPHY_PARAM_NUM) {
		for (i = 0; i < g_dp_pe_sw_length; i++)
			dptx->combophy_pree_swing[DPTX_PREE_SWING_DEFAULT][i] =
				g_dp_preemphasis_swing[i];
	}
}

void dptx_debug_get_double_tx_sync_para(struct dp_ctrl *dptx)
{
	void_unused(dptx);

	return;
}

bool dp_debug_get_clk_hblank_enable(struct dp_ctrl *dptx)
{
	void_unused(dptx);

	return false;
}

void dp_debug_get_clk_hblank_params(struct dp_ctrl *dptx, uint64_t *pixel_clock,
	uint32_t *video_code, uint16_t *hblank)
{
	void_unused(dptx);
	void_unused(pixel_clock);
	void_unused(video_code);
	void_unused(hblank);
}

uint32_t dptx_debug_get_combophy_rate_param(struct dp_ctrl *dptx, int dptx_phyif_ctrl_rate,
	uint32_t *ckgctrl, uint32_t *sscginital, uint32_t *sscgcnt, uint32_t *sscgcnt2)
{
	void_unused(dptx);
	void_unused(dptx_phyif_ctrl_rate);
	void_unused(ckgctrl);
	void_unused(sscginital);
	void_unused(sscgcnt);
	void_unused(sscgcnt2);
	return 0;
}

uint dp_debug_get_aux_ctrl_param(struct dp_ctrl *dptx)
{
	void_unused(dptx);
	return 0;
}

int dp_debug_dptx_connected(void)
{
	if (!g_dptx_debug[g_dp_id])
		return 0;

	if (g_dptx_debug[g_dp_id]->video_transfer_enable)
		return 1;

	return 0;
}

int dp_debug_read_dpcd(uint32_t addr)
{
	uint8_t byte = 0;
	int retval;

	dpu_check_and_return(!g_dptx_debug[g_dp_id], -EINVAL, err, "[DP] dptx is NULL!\n");

	retval = dptx_read_dpcd(g_dptx_debug[g_dp_id], addr, &byte);
	if (retval) {
		   dpu_pr_err("[DP] read dpcd fail\n");
		   return retval;
	}
	return (int)byte;
}

int dp_debug_write_dpcd(uint32_t addr, uint8_t byte)
{
	int retval;

	dpu_check_and_return(!g_dptx_debug[g_dp_id], -EINVAL, err, "[DP] dptx is NULL!\n");

	retval = dptx_write_dpcd(g_dptx_debug[g_dp_id], addr, byte);
	if (retval) {
		   dpu_pr_err("[DP] write dpcd fail\n");
		   return retval;
	}
	return 0;
}

void set_dp_rx_tx_mode(uint32_t dp_rx_tx_mode)
{
	if (unlikely(g_dp_id >= 2)) {
		dpu_pr_err("g_dp_id=%u is out of range", g_dp_id);
		return;
	}

	if (!g_dptx_debug[g_dp_id])
		return;

	dpu_pr_info("[DP] set DPRX-DPTX Mode %s for dptx\n", dp_rx_tx_mode ? "on" : "off");
	g_dptx_debug[g_dp_id]->source_is_external = dp_rx_tx_mode;
}

void set_dp_rx_tx_arsr_mode(uint32_t dp_rx_tx_arsr_mode)
{
	if (!g_dptx_debug[g_dp_id])
		return;

	g_dptx_debug[g_dp_id]->arsr_enable = dp_rx_tx_arsr_mode;
}

void set_dp_mode(uint32_t mode)
{
	if (!g_dptx_debug[g_dp_id])
		return;
	if (mode >= DP_MODE_MAX)
		mode = DP_MODE;

	g_dptx_debug[g_dp_id]->connector->conn_info->base.mode = mode;
}

void set_clk_hblank_code(uint32_t clock, uint32_t hblank, uint32_t code)
{
	g_pixel_clock = clock;
	g_hblank = hblank;
	g_video_code = code;
}

void select_dp(uint32_t id)
{
	if (id >= 2)
		g_dp_id = 0;
	else
		g_dp_id = id;
}

int dpu_dp_get_current_dp_source_mode(void)
{
	return get_current_dp_source_mode();
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL_GPL(dp_debug_dptx_connected);
EXPORT_SYMBOL_GPL(dp_debug_read_dpcd);
EXPORT_SYMBOL_GPL(dp_debug_write_dpcd);
EXPORT_SYMBOL_GPL(set_dp_rx_tx_mode);
EXPORT_SYMBOL_GPL(set_dp_rx_tx_arsr_mode);
EXPORT_SYMBOL_GPL(set_dp_mode);
EXPORT_SYMBOL_GPL(set_clk_hblank_code);
EXPORT_SYMBOL_GPL(select_dp);
#endif
