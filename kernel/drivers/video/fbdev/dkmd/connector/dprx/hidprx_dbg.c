/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WArrANTY; without even the implied warranty of
 * MErCHANTABILITY or FITNESS FOr A PArTICULAr PUrPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hidprx.h"
#include "hidprx_reg.h"
#include <securec.h>
#include "hidprx_dbg.h"

int g_dprx_asic_en = 1;
int g_dprx_ctrl_debug_enable = 0;
int g_dprx_phy_debug_enable = 0;
int g_dprx_lane_num_debug = 4;
int g_dprx_max_linkrate_debug = 0x1E;
int g_dprx_fec_en_debug = 1;
int g_dprx_adaptive_sync_en_debug = 1;
int g_dprx_enhance_frame_en_debug = 1;
int g_dprx_dsc_support_debug = 1;
int g_dprx_phy_rate_debug = 1;
int g_dprx0_switch_phy1_debug = 0;
int g_dprx_open_aux_monitor = 0;
int g_dprx_switch_phy_para = 1;
int g_dprx_ppi_reg_set_debug = 0; // 0x00
int g_dprx_ppi_reg_set2_debug = 0; // 0x4
int g_dprx_pma_aux_debug = 0xF003B07C; // 0x24
int g_dprx_da_rate_r0_debug = 0x13121315; // r0
int g_dprx_da_rate_r0_2_debug = 0x51AAAAAA;
int g_dprx_da_rate_r0_3_debug = 0x011B6854;
int g_dprx_da_rate_r0_4_debug = 0x00000120;
int g_dprx_da_rate_r1_debug = 0x0F101405; // r1
int g_dprx_da_rate_r1_2_debug = 0x41AAAAAA;
int g_dprx_da_rate_r1_3_debug = 0x01D8688C;
int g_dprx_da_rate_r1_4_debug = 0x00000120;
int g_dprx_da_rate_r2_debug = 0x0F0F1416; // r2
int g_dprx_da_rate_r2_2_debug = 0x01AAAAAA;
int g_dprx_da_rate_r2_3_debug = 0x03B1608C;
int g_dprx_da_rate_r2_4_debug = 0x000000C3;
int g_dprx_da_rate_r3_debug = 0x10101416; // r3
int g_dprx_da_rate_r3_2_debug = 0x21AAAAAA;
int g_dprx_da_rate_r3_3_debug = 0x058960D2;
int g_dprx_da_rate_r3_4_debug = 0x000000C3;
int g_dprx_da_rate_r4_debug = 0x0F0C1407; // r4
int g_dprx_da_rate_r4_2_debug = 0x01AAAAAA;
int g_dprx_da_rate_r4_3_debug = 0x017A6870;
int g_dprx_da_rate_r4_4_debug = 0x00000120;
int g_dprx_da_rate_r5_debug = 0x0F0C1408; // r5
int g_dprx_da_rate_r5_2_debug = 0x01AAAAAA;
int g_dprx_da_rate_r5_3_debug = 0x01A9687E;
int g_dprx_da_rate_r5_4_debug = 0x00000120;
int g_dprx_da_rate_r6_debug = 0x0F0C1408; // r6
int g_dprx_da_rate_r6_2_debug = 0x01AAAAAA;
int g_dprx_da_rate_r6_3_debug = 0x02376054;
int g_dprx_da_rate_r6_4_debug = 0x00000120;
int g_dprx_da_rate_r7_debug = 0x0F0C1408; // r7
int g_dprx_da_rate_r7_2_debug = 0x01AAAAAA;
int g_dprx_da_rate_r7_3_debug = 0x02F46070;
int g_dprx_da_rate_r7_4_debug = 0x00000161;
int g_dprx_da_test_en_debug = 0x01000000; // 0x30
int g_dprx_da_test_en_2_debug = 0x00018000; // 0x34
int g_dprx_reg_t_da_2_debug = 0x2222bbbb; // 0x44
int g_dprx_reg_t_da_10_debug = 0; // reg 0x64
int g_dprx_reg_t_da_13_debug = 0x18618600; // reg 0x70

// reg_lane 0-3
int g_dprx_ctle_adapt_debug = 0x00180404;  // lane0
int g_dprx_ctle_adapt1_debug = 0x00180404;  // lane1
int g_dprx_ctle_adapt2_debug = 0x00180404;  // lane2
int g_dprx_ctle_adapt3_debug = 0x00180404;  // lane3

#ifdef CONFIG_DKMD_DEBUG_ENABLE
module_param_named(dprx_asic_en, g_dprx_asic_en, int, 0640);
MODULE_PARM_DESC(dprx_asic_en, "dprx_asic_en");

module_param_named(dprx_ctrl_debug_enable, g_dprx_ctrl_debug_enable, int, 0640);
MODULE_PARM_DESC(dprx_ctrl_debug_enable, "dprx_ctrl_debug_enable");

module_param_named(dprx_phy_debug_enable, g_dprx_phy_debug_enable, int, 0640);
MODULE_PARM_DESC(dprx_phy_debug_enable, "dprx_phy_debug_enable");

module_param_named(dprx_lane_num, g_dprx_lane_num_debug, int, 0640);
MODULE_PARM_DESC(dprx_lane_num, "dprx_lane_num");

module_param_named(dprx_max_linkrate, g_dprx_max_linkrate_debug, int, 0640);
MODULE_PARM_DESC(dprx_max_linkrate, "dprx_max_linkrate");

module_param_named(dprx_fec_en, g_dprx_fec_en_debug, int, 0640);
MODULE_PARM_DESC(dprx_fec_en, "dprx_fec_en");

module_param_named(dprx_adaptive_sync_en, g_dprx_adaptive_sync_en_debug, int, 0640);
MODULE_PARM_DESC(dprx_adaptive_sync_en, "dprx_adaptive_sync_en");

module_param_named(dprx_enhance_frame_en, g_dprx_enhance_frame_en_debug, int, 0640);
MODULE_PARM_DESC(dprx_enhance_frame_en, "dprx_enhance_frame_en");

module_param_named(dprx_dsc_support, g_dprx_dsc_support_debug, int, 0640);
MODULE_PARM_DESC(dprx_dsc_support, "dprx_dsc_support");

module_param_named(dprx_phy_rate, g_dprx_phy_rate_debug, int, 0640);
MODULE_PARM_DESC(dprx_phy_rate, "dprx_phy_rate");

module_param_named(dprx0_switch_phy1, g_dprx0_switch_phy1_debug, int, 0640);
MODULE_PARM_DESC(dprx0_switch_phy1, "dprx0_switch_phy1");

module_param_named(dprx_open_aux_monitor, g_dprx_open_aux_monitor, int, 0640);
MODULE_PARM_DESC(dprx_open_aux_monitor, "dprx_open_aux_monitor");

module_param_named(dprx_switch_phy_para, g_dprx_switch_phy_para, int, 0640);
MODULE_PARM_DESC(dprx_switch_phy_para, "dprx_switch_phy_para");

module_param_named(dprx_ppi_reg_set, g_dprx_ppi_reg_set_debug, int, 0640);
MODULE_PARM_DESC(dprx_ppi_reg_set, "dprx_ppi_reg_set");

module_param_named(dprx_ppi_reg_set2, g_dprx_ppi_reg_set2_debug, int, 0640);
MODULE_PARM_DESC(dprx_ppi_reg_set2, "dprx_ppi_reg_set2");

module_param_named(dprx_pma_aux, g_dprx_pma_aux_debug, int, 0640);
MODULE_PARM_DESC(dprx_pma_aux, "dprx_pma_aux");

module_param_named(dprx_da_rate_r0, g_dprx_da_rate_r0_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r0, "dprx_da_rate_r0");

module_param_named(dprx_da_rate_r0_2, g_dprx_da_rate_r0_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r0_2, "dprx_da_rate_r0_2");

module_param_named(dprx_da_rate_r0_3, g_dprx_da_rate_r0_3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r0_3, "dprx_da_rate_r0_3");

module_param_named(dprx_da_rate_r0_4, g_dprx_da_rate_r0_4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r0_4, "dprx_da_rate_r0_4");

module_param_named(dprx_da_rate_r1, g_dprx_da_rate_r1_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r1, "dprx_da_rate_r0");

module_param_named(dprx_da_rate_r1_2, g_dprx_da_rate_r1_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r1_2, "dprx_da_rate_r1_2");

module_param_named(dprx_da_rate_r1_3, g_dprx_da_rate_r1_3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r1_3, "dprx_da_rate_r1_3");

module_param_named(dprx_da_rate_r1_4, g_dprx_da_rate_r1_4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r1_4, "dprx_da_rate_r1_4");

module_param_named(dprx_da_rate_r2, g_dprx_da_rate_r2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r2, "dprx_da_rate_r2");

module_param_named(dprx_da_rate_r2_2, g_dprx_da_rate_r2_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r2_2, "dprx_da_rate_r2_2");

module_param_named(dprx_da_rate_r2_3, g_dprx_da_rate_r2_3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r2_3, "dprx_da_rate_r2_3");

module_param_named(dprx_da_rate_r2_4, g_dprx_da_rate_r2_4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r2_4, "dprx_da_rate_r2_4");

module_param_named(dprx_da_rate_r3, g_dprx_da_rate_r3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r3, "dprx_da_rate_r3");

module_param_named(dprx_da_rate_r3_2, g_dprx_da_rate_r3_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r3_2, "dprx_da_rate_r3_2");

module_param_named(dprx_da_rate_r3_3, g_dprx_da_rate_r3_3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r3_3, "dprx_da_rate_r3_3");

module_param_named(dprx_da_rate_r3_4, g_dprx_da_rate_r3_4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r3_4, "dprx_da_rate_r3_4");

module_param_named(dprx_da_rate_r4, g_dprx_da_rate_r4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r4, "dprx_da_rate_r4");

module_param_named(dprx_da_rate_r4_2, g_dprx_da_rate_r4_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r4_2, "dprx_da_rate_r4_2");

module_param_named(dprx_da_rate_r4_3, g_dprx_da_rate_r4_3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r4_3, "dprx_da_rate_r4_3");

module_param_named(dprx_da_rate_r4_4, g_dprx_da_rate_r4_4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r4_4, "dprx_da_rate_r4_4");

module_param_named(dprx_da_rate_r5, g_dprx_da_rate_r5_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r5, "dprx_da_rate_r5");

module_param_named(dprx_da_rate_r5_2, g_dprx_da_rate_r5_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r5_2, "dprx_da_rate_r5_2");

module_param_named(dprx_da_rate_r5_3, g_dprx_da_rate_r5_3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r5_3, "dprx_da_rate_r5_3");

module_param_named(dprx_da_rate_r5_4, g_dprx_da_rate_r5_4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r5_4, "dprx_da_rate_r5_4");

module_param_named(dprx_da_rate_r6, g_dprx_da_rate_r6_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r6, "dprx_da_rate_r6");

module_param_named(dprx_da_rate_r6_2, g_dprx_da_rate_r6_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r6_2, "dprx_da_rate_r6_2");

module_param_named(dprx_da_rate_r6_3, g_dprx_da_rate_r6_3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r6_3, "dprx_da_rate_r6_3");

module_param_named(dprx_da_rate_r6_4, g_dprx_da_rate_r6_4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r6_4, "dprx_da_rate_r6_4");

module_param_named(dprx_da_rate_r7, g_dprx_da_rate_r7_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r7, "dprx_da_rate_r7");

module_param_named(dprx_da_rate_r7_2, g_dprx_da_rate_r7_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r7_2, "dprx_da_rate_r7_2");

module_param_named(dprx_da_rate_r7_3, g_dprx_da_rate_r7_3_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r7_3, "dprx_da_rate_r7_3");

module_param_named(dprx_da_rate_r7_4, g_dprx_da_rate_r7_4_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_rate_r7_4, "dprx_da_rate_r7_4");

module_param_named(dprx_da_test_en, g_dprx_da_test_en_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_test_en, "dprx_da_test_en");

module_param_named(dprx_da_test_en_2, g_dprx_da_test_en_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_da_test_en_2, "dprx_da_test_en_2");

module_param_named(dprx_reg_t_da_2, g_dprx_reg_t_da_2_debug, int, 0640);
MODULE_PARM_DESC(dprx_reg_t_da_2, "dprx_reg_t_da_2");

module_param_named(dprx_reg_t_da_10, g_dprx_reg_t_da_10_debug, int, 0640);
MODULE_PARM_DESC(dprx_reg_t_da_10, "dprx_reg_t_da_10");

module_param_named(dprx_reg_t_da_13, g_dprx_reg_t_da_13_debug, int, 0640);
MODULE_PARM_DESC(dprx_reg_t_da_13, "dprx_reg_t_da_13");

module_param_named(dprx_ctle_adapt, g_dprx_ctle_adapt_debug, int, 0640);
MODULE_PARM_DESC(dprx_ctle_adapt, "dprx_ctle_adapt");

module_param_named(dprx_ctle_adapt1, g_dprx_ctle_adapt1_debug, int, 0640);
MODULE_PARM_DESC(dprx_ctle_adapt1, "dprx_ctle_adapt1");

module_param_named(dprx_ctle_adapt2, g_dprx_ctle_adapt2_debug, int, 0640);
MODULE_PARM_DESC(dprx_ctle_adapt2, "dprx_ctle_adapt2");

module_param_named(dprx_ctle_adapt3, g_dprx_ctle_adapt3_debug, int, 0640);
MODULE_PARM_DESC(dprx_ctle_adapt3, "dprx_ctle_adapt3");
#endif

bool is_switch_phy_para(void)
{
	return g_dprx_switch_phy_para;
}

bool is_asic_process(void)
{
	return g_dprx_asic_en;
}

bool is_open_ctrl_debug(void)
{
	return g_dprx_ctrl_debug_enable;
}

bool is_rx0_phy_switch(void)
{
	return g_dprx0_switch_phy1_debug;
}

bool is_need_aux_monitor(void)
{
	return g_dprx_open_aux_monitor;
}

static bool is_valid_lane_num(int dprx_lane_num_debug)
{
	if (dprx_lane_num_debug == 1 || dprx_lane_num_debug == 2 || dprx_lane_num_debug == 4)
		return true;

	return false;
}

static bool is_valid_link_rate(int dprx_max_linkrate_debug)
{
	if (dprx_max_linkrate_debug == 0x06 || dprx_max_linkrate_debug == 0x0A ||
		dprx_max_linkrate_debug == 0x14 || dprx_max_linkrate_debug == 0x1E)
		return true;

	return false;
}

int dprx_debug_set_ctrl_params(struct dprx_ctrl *dprx)
{
	if (g_dprx_ctrl_debug_enable == 0)
		return -1;
	dprx_pr_info("[DPRX] dprx_debug_set_ctrl_params +\n");
	if (is_valid_lane_num(g_dprx_lane_num_debug))
		dprx->debug_params.max_lane_num = g_dprx_lane_num_debug;

	if (is_valid_link_rate(g_dprx_max_linkrate_debug))
		dprx->debug_params.max_link_rate = g_dprx_max_linkrate_debug;

	dprx->debug_params.adaptive_sync_en = g_dprx_adaptive_sync_en_debug;
	dprx->debug_params.fec_en = g_dprx_fec_en_debug;
	dprx->debug_params.enhance_frame_en = g_dprx_enhance_frame_en_debug;
	dprx->debug_params.dsc_support = g_dprx_dsc_support_debug;
	dprx_pr_info("[DPRX] set params: lane %d, rate %d, fecEn %d, vrrEn %d, enFrame %d, dscEn %d -\n",
		g_dprx_lane_num_debug, g_dprx_max_linkrate_debug, g_dprx_fec_en_debug,
		g_dprx_adaptive_sync_en_debug, g_dprx_enhance_frame_en_debug, g_dprx_dsc_support_debug);
	dprx_pr_info("[DPRX] dprx_debug_set_ctrl_params -\n");
	return 0;
}

void dprx_debug_set_phy_params(struct dprx_ctrl *dprx)
{
	if (g_dprx_phy_debug_enable == 0)
		return;
	dprx_pr_info("[DPRX] dprx_debug_set_phy_params +\n");
	void __iomem *phy_base;
	phy_base = dprx->base + DPRX_PHY_REG_OFFSET;
	// 0x00
	dprx_pr_info("[DPRX] dprx_ppi_reg_set \n");
	outp32(phy_base + PPI_REG_SET, g_dprx_ppi_reg_set_debug);
	// 0x04
	dprx_pr_info("[DPRX] dprx_ppi_reg_set2 \n");
	outp32(phy_base + PPI_REG_SET2, g_dprx_ppi_reg_set2_debug);
	// 0x24
	dprx_pr_info("[DPRX] dprx_pma_aux \n");
	outp32(phy_base + PMA_AUX, g_dprx_pma_aux_debug);
	// 0x30
	dprx_pr_info("[DPRX] dprx_da_test_en \n");
	outp32(phy_base + DA_TEST_EN, g_dprx_da_test_en_debug);
	// 0x34
	dprx_pr_info("[DPRX] dprx_da_test_en_2 \n");
	outp32(phy_base + DA_TEST_EN_2, g_dprx_da_test_en_2_debug);
	// 0x44
	dprx_pr_info("[DPRX] dprx_reg_t_da_2 \n");
	outp32(phy_base + REG_T_DA_2, g_dprx_reg_t_da_2_debug);
	// 0x64
	dprx_pr_info("[DPRX] dprx_reg_t_da_10 \n");
	outp32(phy_base + REG_T_DA_10, g_dprx_reg_t_da_10_debug);
	// 0x70
	dprx_pr_info("[DPRX] dprx_reg_t_da_13 \n");
	outp32(phy_base + REG_T_DA_13, g_dprx_reg_t_da_13_debug);
	// r0---r7
	dprx_pr_info("[DPRX] dprx_da_rate_r0-4 \n");
	outp32(phy_base + DA_RATE_R0, g_dprx_da_rate_r0_debug);
	outp32(phy_base + DA_RATE_R0_2, g_dprx_da_rate_r0_2_debug);
	outp32(phy_base + DA_RATE_R0_3, g_dprx_da_rate_r0_3_debug);
	outp32(phy_base + DA_RATE_R0_4, g_dprx_da_rate_r0_4_debug);
	dprx_pr_info("[DPRX] dprx_da_rate_r1-4 \n");
	outp32(phy_base + DA_RATE_R1, g_dprx_da_rate_r1_debug);
	outp32(phy_base + DA_RATE_R1_2, g_dprx_da_rate_r1_2_debug);
	outp32(phy_base + DA_RATE_R1_3, g_dprx_da_rate_r1_3_debug);
	outp32(phy_base + DA_RATE_R1_4, g_dprx_da_rate_r1_4_debug);
	dprx_pr_info("[DPRX] dprx_da_rate_r2-4 \n");
	outp32(phy_base + DA_RATE_R2, g_dprx_da_rate_r2_debug);
	outp32(phy_base + DA_RATE_R2_2, g_dprx_da_rate_r2_2_debug);
	outp32(phy_base + DA_RATE_R2_3, g_dprx_da_rate_r2_3_debug);
	outp32(phy_base + DA_RATE_R2_4, g_dprx_da_rate_r2_4_debug);
	dprx_pr_info("[DPRX] dprx_da_rate_r3-4 \n");
	outp32(phy_base + DA_RATE_R3, g_dprx_da_rate_r3_debug);
	outp32(phy_base + DA_RATE_R3_2, g_dprx_da_rate_r3_2_debug);
	outp32(phy_base + DA_RATE_R3_3, g_dprx_da_rate_r3_3_debug);
	outp32(phy_base + DA_RATE_R3_4, g_dprx_da_rate_r3_4_debug);
	dprx_pr_info("[DPRX] dprx_da_rate_r4-4 \n");
	outp32(phy_base + DA_RATE_R4, g_dprx_da_rate_r4_debug);
	outp32(phy_base + DA_RATE_R4_2, g_dprx_da_rate_r4_2_debug);
	outp32(phy_base + DA_RATE_R4_3, g_dprx_da_rate_r4_3_debug);
	outp32(phy_base + DA_RATE_R4_4, g_dprx_da_rate_r4_4_debug);
	dprx_pr_info("[DPRX] dprx_da_rate_r5-4 \n");
	outp32(phy_base + DA_RATE_R5, g_dprx_da_rate_r5_debug);
	outp32(phy_base + DA_RATE_R5_2, g_dprx_da_rate_r5_2_debug);
	outp32(phy_base + DA_RATE_R5_3, g_dprx_da_rate_r5_3_debug);
	outp32(phy_base + DA_RATE_R5_4, g_dprx_da_rate_r5_4_debug);
	dprx_pr_info("[DPRX] dprx_da_rate_r6-4 \n");
	outp32(phy_base + DA_RATE_R6, g_dprx_da_rate_r6_debug);
	outp32(phy_base + DA_RATE_R6_2, g_dprx_da_rate_r6_2_debug);
	outp32(phy_base + DA_RATE_R6_3, g_dprx_da_rate_r6_3_debug);
	outp32(phy_base + DA_RATE_R6_4, g_dprx_da_rate_r6_4_debug);
	dprx_pr_info("[DPRX] dprx_da_rate_r7-4 \n");
	outp32(phy_base + DA_RATE_R7, g_dprx_da_rate_r7_debug);
	outp32(phy_base + DA_RATE_R7_2, g_dprx_da_rate_r7_2_debug);
	outp32(phy_base + DA_RATE_R7_3, g_dprx_da_rate_r7_3_debug);
	outp32(phy_base + DA_RATE_R7_4, g_dprx_da_rate_r7_4_debug);
	dprx_pr_info("[DPRX] dprx_ctle_adapt0-3 \n");
	outp32(phy_base + DPRX_D_LANE_REG_OFFSET + DPRX_CTLE_ADAPT0, g_dprx_ctle_adapt_debug);
	outp32(phy_base + DPRX_D_LANE_REG_OFFSET + DPRX_CTLE_ADAPT1, g_dprx_ctle_adapt1_debug);
	outp32(phy_base + DPRX_D_LANE_REG_OFFSET + DPRX_CTLE_ADAPT2, g_dprx_ctle_adapt2_debug);
	outp32(phy_base + DPRX_D_LANE_REG_OFFSET + DPRX_CTLE_ADAPT3, g_dprx_ctle_adapt3_debug);
	dprx_pr_info("[DPRX] dprx_debug_set_phy_params -\n");
}

uint32_t dprx_get_phy_da_test_en_2(void)
{
	return g_dprx_da_test_en_2_debug;
}
uint32_t dprx_get_phy_reg_t_da_2(void)
{
	return g_dprx_reg_t_da_2_debug;
}
uint32_t dprx_get_phy_reg_t_da_13(void)
{
	return g_dprx_reg_t_da_13_debug;
}