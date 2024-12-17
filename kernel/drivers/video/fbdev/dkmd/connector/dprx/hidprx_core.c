/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "hidprx_core.h"
#include "hidprx_reg.h"
#include "hidprx_edid.h"
#include "hidprx_dbg.h"
#include <linux/gpio.h>
#include "hidprx_ctrl.h"

static void dprx_dis_reset(struct dprx_ctrl *dprx, bool benable)
{
	uint32_t reg;
	uint32_t phy_reg_offset;
	dpu_check_and_no_retval((dprx == NULL), err, "dprx is NULL!\n");
	dprx_pr_info("[DPRX] dprx_dis_reset: dprx%u, benable %d +\n", dprx->id, benable);

	if (benable) { // on
		if (dprx->id == 0) {
			reg = inp32(dprx->hsdt1_crg_base + DPRX0_DIS_RESET_OFFSET); /* 0xEB045000 + 0x64 */
			reg |= DPRX0_APB_DIS_RESET_BIT;
			reg |= DPRX0_CTRL_DIS_RESET_BIT;
			reg |= DPRX0_EDID_DIS_RESET_BIT;
			reg |= DPRX0_APB_BRIDGE_DIS_RESET_BIT;
			outp32(dprx->hsdt1_crg_base + DPRX0_DIS_RESET_OFFSET, reg);

			reg = inp32(dprx->hsdt1_crg_base + DPRX0_CLK_GATE_OFFSET); /* 0xEB045000 + 0x10 */
			reg |= DPRX0_APB_CLK_GATE_BIT;
			reg |= DPRX0_EDID_CLK_GATE_BIT;
			reg |= DPRX0_APB_BRIDGE_CLK_GATE_BIT;
			reg |= DPRX0_AUX_CLK_GATE_BIT;
			outp32(dprx->hsdt1_crg_base + DPRX0_CLK_GATE_OFFSET, reg);
		} else {
			// dprx1
			reg = inp32(dprx->hsdt1_crg_base + DPRX1_DIS_RESET_OFFSET); /* 0xEB045000 + 0x64 */
			reg |= DPRX1_APB_DIS_RESET_BIT;
			reg |= DPRX1_CTRL_DIS_RESET_BIT;
			reg |= DPRX1_EDID_DIS_RESET_BIT;
			reg |= DPRX1_APB_BRIDGE_DIS_RESET_BIT;
			outp32(dprx->hsdt1_crg_base + DPRX1_DIS_RESET_OFFSET, reg);

			reg = inp32(dprx->hsdt1_crg_base + DPRX1_CLK_GATE_OFFSET); /* 0xEB045000 + 0x10 */
			reg |= DPRX1_APB_CLK_GATE_BIT;
			reg |= DPRX1_EDID_CLK_GATE_BIT;
			reg |= DPRX1_APB_BRIDGE_CLK_GATE_BIT;
			reg |= DPRX1_AUX_CLK_GATE_BIT;
			outp32(dprx->hsdt1_crg_base + DPRX1_CLK_GATE_OFFSET, reg);
		}
	} else { // off
		dprx_pr_info("[DPRX] dprx%u reset\n", dprx->id);
		if (is_asic_process()) {
			phy_reg_offset = DPRX_PHY_REG_OFFSET;
			reg = inp32(dprx->base + phy_reg_offset + PMA_AUX);
			reg &= ~REG_DPRX_AUX_PWDNB;
			outp32(dprx->base + phy_reg_offset + PMA_AUX, reg);
		}

		if (dprx->id == 0) {
			reg = inp32(dprx->hsdt1_crg_base + DPRX0_RESET_OFFSET); /* 0xEB045000 + 0x60 */
			reg |= DPRX0_APB_RESET_BIT;
			reg |= DPRX0_CTRL_RESET_BIT;
			reg |= DPRX0_EDID_RESET_BIT;
			reg |= DPRX0_APB_BRIDGE_RESET_BIT;
			outp32(dprx->hsdt1_crg_base + DPRX0_RESET_OFFSET, reg);

			reg = inp32(dprx->hsdt1_crg_base + DPRX0_DISABLE_CLK_OFFSET); /* 0xEB045000 + 0x14 */
			reg |= DPRX0_GT_CLK_HIDPRX0PHY_AUX;
			reg |= DPRX0_GT_CLK_HIDPRX0PHY_REF;
			reg |= DPRX0_GT_PCLK_HIDPRX0;
			reg |= DPRX0_GT_PCLK_HIDPRX0_AUX;
			reg |= DPRX_GT_PCLK_EIDI;
			reg |= DPRX_GT_PCLK_DPRX_ASYNC_BRIDGE;
			outp32(dprx->hsdt1_crg_base + DPRX0_DISABLE_CLK_OFFSET, reg);
		} else {
			// dprx1
			reg = inp32(dprx->hsdt1_crg_base + DPRX1_RESET_OFFSET);
			reg |= DPRX1_APB_RESET_BIT;
			reg |= DPRX1_CTRL_RESET_BIT;
			reg |= DPRX1_EDID_RESET_BIT;
			reg |= DPRX1_APB_BRIDGE_RESET_BIT;
			outp32(dprx->hsdt1_crg_base + DPRX1_RESET_OFFSET, reg);

			reg = inp32(dprx->hsdt1_crg_base + DPRX1_DISABLE_CLK_OFFSET);
			reg |= DPRX1_GT_CLK_HIDPRX1PHY_AUX;
			reg |= DPRX1_GT_CLK_HIDPRX1PHY_REF;
			reg |= DPRX1_GT_PCLK_HIDPRX1;
			reg |= DPRX1_GT_PCLK_HIDPRX1_AUX;
			reg |= DPRX_GT_PCLK_EIDI;
			reg |= DPRX_GT_PCLK_DPRX_ASYNC_BRIDGE;
			outp32(dprx->hsdt1_crg_base + DPRX1_DISABLE_CLK_OFFSET, reg);
		}
	}
	dprx_pr_info("[DPRX] dprx_dis_reset -\n");
}

/**
 * reset all inner module
 */
static void dprx_soft_reset_all(struct dprx_ctrl *dprx)
{
	uint32_t phyifctrl;

	dpu_check_and_no_retval((dprx == NULL), err, "dprx is NULL!\n");

	dprx_pr_info("[DPRX] dprx_soft_reset_all +\n");
	phyifctrl = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_RESET_CTRL);
	phyifctrl |= DPRX_CORE_RESET_BIT;
	phyifctrl |= DPRX_LANE_RESET_BIT;
	phyifctrl |= DPRX_AUX_RESET_BIT;
	/* for edp mode */
	if (dprx->mode == EDP_MODE)
		phyifctrl |= DPRX_PSR_RESET_BIT; // for test
	phyifctrl |= DPRX_ATC_RESET_BIT;
	phyifctrl |= DPRX_PHY_CTRL_RESET_BIT;
	phyifctrl |= DPRX_RX2TX_RESET_BIT;
	phyifctrl |= DPRX_AUDIO_LINK_RESET_BIT;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_RESET_CTRL, phyifctrl);
	dprx_pr_info("[DPRX] dprx_soft_reset_all -\n");
}

/**
 * enable inner clk gate
 */
static void dprx_clk_ctrl_enable(struct dprx_ctrl *dprx)
{
	uint32_t phyifctrl;

	dpu_check_and_no_retval((dprx == NULL), err, "dprx is NULL!\n");

	dprx_pr_info("[DPRX] dprx_clk_ctrl_enable +\n");
	phyifctrl = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_CLK_CTRL);
	phyifctrl |= DPRX_AUX_CLK_EN_BIT;
	phyifctrl |= DPRX_DSS_CORE_CLK_EN_BIT;
	/* for edp mode */
	if (dprx->mode == EDP_MODE)
		phyifctrl |= DPRX_PSR_CLK_EN_BIT; // for test
	phyifctrl |= DPRX_RX2TX_CLK_EN_BIT;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_CLK_CTRL, phyifctrl);
	dprx_pr_info("[DPRX] dprx_clk_ctrl_enable -\n");
}

/**
 * 1 byte for 1 dpcd, while apb write/read with 32bit
 * Ex: We need write dpcd 0x101, then we need write to DPCD_103_100's bit15-8
 */
uint8_t dprx_read_dpcd(struct dprx_ctrl *dprx, uint32_t addr)
{
	uint32_t start_addr;
	uint32_t offset;
	uint32_t reg;
	uint8_t val;

	start_addr = (addr / DPCD_WR_BYTE_LEN) * DPCD_WR_BYTE_LEN;
	offset = addr % DPCD_WR_BYTE_LEN;
	reg = dprx_readl(dprx, start_addr);
	reg &= GENMASK(offset * 8 + 7, offset * 8);
	val = reg >> (offset * 8);

	return val;
}

void dprx_write_dpcd(struct dprx_ctrl *dprx, uint32_t addr, uint8_t val)
{
	uint32_t start_addr;
	uint32_t offset;
	uint32_t reg;

	start_addr = (addr / DPCD_WR_BYTE_LEN) * DPCD_WR_BYTE_LEN;
	offset = addr % DPCD_WR_BYTE_LEN;
	reg = dprx_readl(dprx, start_addr);
	reg &= ~GENMASK(offset * 8 + 7, offset * 8);
	reg |= val << (offset * 8);
	dprx_writel(dprx, start_addr, reg);

	dprx_pr_info("val(0x%x);\n", reg);
}

static void dprx_init_rsv_value(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	/* OUI for test */
	dprx_write_dpcd(dprx, 0x400, 0xa4);
	dprx_write_dpcd(dprx, 0x401, 0xc0);
	dprx_write_dpcd(dprx, 0x402, 0x00);
	dprx_write_dpcd(dprx, 0x403, 0x12);

	/* for dpcd 0x205 */
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_SINKSTATUS_CTRL);
	reg |= BIT(0);
	reg |= BIT(1);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_SINKSTATUS_CTRL, reg);

	dprx_write_dpcd(dprx, 0x2204, 0x01);
	dprx_write_dpcd(dprx, 0x220A, 0x06);
}

static void dprx_config_dsd_capbility(struct dprx_ctrl *dprx)
{
	dprx_pr_info("[DPRX] dprx_config_dsd_capbility +\n");
	/* config DSD capbility */
	dprx_write_dpcd(dprx, 0x60, 0x1); // 0x1 DSD
	dprx->dpcds.rx_caps[0x60] = 0x1; // 0x1 DSD
	dprx_write_dpcd(dprx, 0x61, 0x21);
	dprx->dpcds.rx_caps[0x61] = 0x21;
	dprx_write_dpcd(dprx, 0x62, 0x0);
	dprx->dpcds.rx_caps[0x62] = 0x0;
	dprx_write_dpcd(dprx, 0x63, 0x20);
	dprx->dpcds.rx_caps[0x63] = 0x20;

	dprx_write_dpcd(dprx, 0x64, 0xB);
	dprx->dpcds.rx_caps[0x64] = 0xB;
	dprx_write_dpcd(dprx, 0x65, 0x2);
	dprx->dpcds.rx_caps[0x65] = 0x2;
	dprx_write_dpcd(dprx, 0x66, 0x1);
	dprx->dpcds.rx_caps[0x66] = 0x1;
	dprx_write_dpcd(dprx, 0x67, 0xD0);
	dprx->dpcds.rx_caps[0x67] = 0xD0;

	dprx_write_dpcd(dprx, 0x68, 0x1);
	dprx->dpcds.rx_caps[0x68] = 0x1;
	dprx_write_dpcd(dprx, 0x69, 0x1B);
	dprx->dpcds.rx_caps[0x69] = 0x1B;
	dprx_write_dpcd(dprx, 0x6A, 0x6);
	dprx->dpcds.rx_caps[0x6A] = 0x6;
	dprx_write_dpcd(dprx, 0x6B, 0x11);
	dprx->dpcds.rx_caps[0x6B] = 0x11;

	dprx_write_dpcd(dprx, 0x6C, 0xC);
	dprx->dpcds.rx_caps[0x6C] = 0xC;
	dprx_write_dpcd(dprx, 0x6D, 0x0);
	dprx->dpcds.rx_caps[0x6D] = 0x0;
	dprx_write_dpcd(dprx, 0x6E, 0x0);
	dprx->dpcds.rx_caps[0x6E] = 0x0;
	dprx_write_dpcd(dprx, 0x6F, 0x4);
	dprx->dpcds.rx_caps[0x6F] = 0x4;
	dprx->dsc_en = 0;
	dprx_pr_info("[DPRX] dprx_config_dsd_capbility -\n");
}

static void dprx_dpcd_debug_params(struct dprx_ctrl *dprx)
{
	if (dprx_debug_set_ctrl_params(dprx) != 0)
		return;
	dprx_pr_info("[DPRX] dprx_dpcd_debug_params +\n");
	dprx_write_dpcd(dprx, 0x1, dprx->debug_params.max_link_rate);
	dprx->dpcds.rx_caps[0x1] =dprx->debug_params.max_link_rate;
	dprx_write_dpcd(dprx, 0x2, (0x80 | dprx->debug_params.max_lane_num));
	dprx->dpcds.rx_caps[0x2] = (0x80 | dprx->debug_params.max_lane_num);
	if (dprx->debug_params.enhance_frame_en == 0) {
		// bit7 -> 0
		dprx->dpcds.rx_caps[0x2] = dprx->dpcds.rx_caps[0x2] & 0x7F;
		dprx_write_dpcd(dprx, 0x2, dprx->dpcds.rx_caps[0x2]);
	}
	// 30bit-bit7
	if (dprx->debug_params.adaptive_sync_en == 0) {
		dprx_write_dpcd(dprx, 0x7, 0x80);
		dprx->dpcds.rx_caps[0x7] = 0x80;
	}
	if (dprx->debug_params.dsc_support == 0) {
		dprx_write_dpcd(dprx, 0x60, 0x0);
		dprx->dpcds.rx_caps[0x60] = 0x0;
	}
	if ((dprx->debug_params.fec_en == 0) && (dprx->mode == DP_MODE)) {
		dprx_write_dpcd(dprx, 0x90, 0xBE);
		dprx->dpcds.rx_caps[0x90] = 0xBE;
	}
	dprx_pr_info("[DPRX] dprx_dpcd_debug_params -\n");
}

static void dprx_init_extern_dpcd(struct dprx_ctrl *dprx)
{
	int i;
	for (i = 0; i < EXTERN_DPCD_REPEAT_SCOPE; i++) {
		dprx_write_dpcd(dprx, EXTERN_DPCD_OFFSET + i, dprx->dpcds.rx_caps[i]);
		dprx->dpcds.extended_rx_caps[i] = dprx->dpcds.rx_caps[i];
	}
}

uint32_t g_edprx_link_rate_block[EDPRX_RATE_NUM * 2] = {EDPRX_LINK_RATE_RBR_BLOCK0, EDPRX_LINK_RATE_RBR_BLOCK1,
										EDPRX_LINK_RATE_R216_BLOCK0, EDPRX_LINK_RATE_R216_BLOCK1,
										EDPRX_LINK_RATE_R243_BLOCK0, EDPRX_LINK_RATE_R243_BLOCK1,
										EDPRX_LINK_RATE_HBR_BLOCK0, EDPRX_LINK_RATE_HBR_BLOCK1,
										EDPRX_LINK_RATE_R324_BLOCK0, EDPRX_LINK_RATE_R324_BLOCK1,
										EDPRX_LINK_RATE_R432_BLOCK0, EDPRX_LINK_RATE_R432_BLOCK1,
										EDPRX_LINK_RATE_HBR2_BLOCK0, EDPRX_LINK_RATE_HBR2_BLOCK1,
										EDPRX_LINK_RATE_HBR3_BLOCK0, EDPRX_LINK_RATE_HBR3_BLOCK1};

static void dprx_init_edp_supported_link_rate(struct dprx_ctrl *dprx)
{
	uint32_t i;
	for (i = 0; i < (EDPRX_RATE_NUM * 2); i++) {
		dprx_write_dpcd(dprx, 0x10 + i, g_edprx_link_rate_block[i]);
		dprx->dpcds.rx_caps[0x10 + i] = g_edprx_link_rate_block[i];
	}
}

static void dprx_init_dpcd(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	uint32_t dpcd_addr;
	dprx_pr_info("[DPRX] dprx_init_dpcd +\n");

	reg = dprx_readl(dprx, 0x100);
	reg = 0x40A;
	dprx_writel(dprx, 0x100, reg);
	dprx->dpcds.link_cfgs[2] = 0x0;

	dprx_write_dpcd(dprx, 0x0, 0x14);
	dprx->dpcds.rx_caps[0x0] = 0x14;
	dprx->dpcds.rx_caps[0x1] = is_asic_process() ? ((dprx->mode == EDP_MODE) ? 0x0 : 0x1E) : 0xA;
	dprx_write_dpcd(dprx, 0x1, dprx->dpcds.rx_caps[0x1]);

	dprx_write_dpcd(dprx, 0x2, 0xc4);
	dprx->dpcds.rx_caps[0x2] = 0xc4;
	dprx_write_dpcd(dprx, 0x3, 0x81);
	dprx->dpcds.rx_caps[0x3] = 0x81;

	dprx_write_dpcd(dprx, 0x4, 0x1);
	dprx->dpcds.rx_caps[0x4] = 0x1;
	dprx_write_dpcd(dprx, 0x5, 0x0);
	dprx->dpcds.rx_caps[0x5] = 0x0;
	dprx_write_dpcd(dprx, 0x6, 0x1);
	dprx->dpcds.rx_caps[0x6] = 0x1;
	dprx_write_dpcd(dprx, 0x7, 0xc0);
	dprx->dpcds.rx_caps[0x7] = 0xc0;

	dprx_write_dpcd(dprx, 0x8, 0x2);
	dprx->dpcds.rx_caps[0x8] = 0x2;
	dprx_write_dpcd(dprx, 0x9, 0x0);
	dprx->dpcds.rx_caps[0x9] = 0x0;
	dprx_write_dpcd(dprx, 0xA, 0x6);
	dprx->dpcds.rx_caps[0xA] = 0x6;
	dprx_write_dpcd(dprx, 0xB, 0x0);
	dprx->dpcds.rx_caps[0xB] = 0x0;

	dprx_write_dpcd(dprx, 0xC, 0x0);
	dprx->dpcds.rx_caps[0xC] = 0x0;
	dprx_write_dpcd(dprx, 0xD, 0x0);
	dprx->dpcds.rx_caps[0xD] = 0x0;
	dprx_write_dpcd(dprx, 0xE, 0x84);
	dprx->dpcds.rx_caps[0xE] = 0x84;
	dprx_write_dpcd(dprx, 0xF, 0x0);
	dprx->dpcds.rx_caps[0xF] = 0x0;
	dprx_init_extern_dpcd(dprx);
	if (dprx->mode == EDP_MODE)
		dprx_init_edp_supported_link_rate(dprx);
	/* config PSR or PSR2 */
	reg = dprx_read_dpcd(dprx, 0x70);
	reg &= ~BIT(1);
	reg &= ~BIT(0);
	dprx_write_dpcd(dprx, 0x70, reg);
	dprx->dpcds.rx_caps[0x70] = 0x0;
	dprx_write_dpcd(dprx, 0x71, reg);
	dprx->dpcds.rx_caps[0x71] = 0x0;
	dprx_write_dpcd(dprx, 0x72, reg);
	dprx->dpcds.rx_caps[0x72] = 0x0;
	dprx_write_dpcd(dprx, 0x73, reg);
	dprx->dpcds.rx_caps[0x73] = 0x0;

	dprx_config_dsd_capbility(dprx);

	/* fec capability */
	if (dprx->mode == DP_MODE) {
		dprx_write_dpcd(dprx, 0x90, 0xBF);
		dprx->dpcds.rx_caps[0x90] = 0xBF;
	}

	dprx_write_dpcd(dprx, 0x200, 0x1);
	dprx_write_dpcd(dprx, 0x2002, 0x1);

	/* init dpcp for cts */
	dprx_init_rsv_value(dprx);
	/* dbg: ctrl params adjustmen */
	dprx_dpcd_debug_params(dprx);
	/* for test */
	for (dpcd_addr = 0; dpcd_addr < 0x100; dpcd_addr++)
		dprx->dpcds.rx_caps[dpcd_addr] = dprx_read_dpcd(dprx, dpcd_addr);
	for (dpcd_addr = 0x100; dpcd_addr < 0x200; dpcd_addr++)
		dprx->dpcds.link_cfgs[dpcd_addr - 0x100] = dprx_read_dpcd(dprx, dpcd_addr);
	for (dpcd_addr = 0x700; dpcd_addr < 0x800; dpcd_addr++)
		dprx->dpcds.edp_spec[dpcd_addr - 0x700] = dprx_read_dpcd(dprx, dpcd_addr);
	for (dpcd_addr = 0x2000; dpcd_addr < 0x2200; dpcd_addr++)
		dprx->dpcds.event_status_indicator[dpcd_addr - 0x2000] = dprx_read_dpcd(dprx, dpcd_addr);
	for (dpcd_addr = 0x2200; dpcd_addr < 0x2300; dpcd_addr++)
		dprx->dpcds.extended_rx_caps[dpcd_addr - 0x2200] = dprx_read_dpcd(dprx, dpcd_addr);
	dprx->dpcds.power_state = dprx_read_dpcd(dprx, DPRX_POWER_CTRL);
	dprx_pr_info("[DPRX] dprx_init_dpcd -\n");
}

static void dprx_edid_init_config(struct dprx_ctrl *dprx)
{
	uint32_t phyifctrl;
	uint32_t reg;
	uint32_t tmp;
	uint16_t i;
	uint16_t j;
	uint8_t *block = NULL;

	dpu_check_and_no_retval((dprx == NULL), err, "dprx is NULL!\n");

	dprx_pr_info("[DPRX] dprx_edid_init_config +\n");

	phyifctrl = inp32(dprx->hsdt1_sys_ctrl_base + HSDT1_EDID_BASE_ADDR_CONFIG); /* 0xEB040000 + 0x508 */
	if (dprx->id == 0) {
		phyifctrl &= ~DPRX0_EDID_BASE_ADDR_CONFIG;
		phyifctrl |= (DPRX0_EDID_BASE_ADDR << DPRX0_EDID_BASE_ADDR_CONFIG_OFFSET);
	} else if (dprx->id == 1) {
		phyifctrl &= ~DPRX1_EDID_BASE_ADDR_CONFIG;
		phyifctrl |= (DPRX0_EDID_BASE_ADDR << DPRX1_EDID_BASE_ADDR_CONFIG_OFFSET);
	}
	outp32(dprx->hsdt1_sys_ctrl_base + HSDT1_EDID_BASE_ADDR_CONFIG, phyifctrl);

	dprx_pr_info("[DPRX] dprx->edid.valid_block_num %d.\n",dprx->edid.valid_block_num);

	for (i = 0; i < dprx->edid.valid_block_num; i++) {
		block = dprx->edid.block[i].payload;
		for (j = 0; j < EDID_BLOCK_SIZE; j += 4) {
			reg = 0;
			tmp = block[j + 3];
			reg |= tmp << 24;
			tmp = block[j + 2];
			reg |= tmp << 16;
			tmp = block[j + 1];
			reg |= tmp << 8;
			reg |= block[j];
			outp32(dprx->edid_base + i * EDID_BLOCK_SIZE + j, reg);
		}
	}

	dprx_pr_info("[DPRX] dprx_edid_init_config -\n");
}

static void dprx_global_intr_dis(struct dprx_ctrl *dprx)
{
	dprx_pr_info("[DPRX] dprx_global_intr_dis +\n");
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + SDP_RPT_EN, 0);

	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_INTR_ENABLE, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_TIMING_INTR_ENABLE, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_TRAINING_INTR_ENABLE, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PSR_INTR_ENABLE, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_SDP_VIDEOFORMAT_INTR_ENABLE, 0);
	dprx_pr_info("[DPRX] dprx_global_intr_dis -\n");
}

static void dprx_fpga_test_all_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_ien;
	irq_ien = DPRX_IRQ_VSYNC | DPRX_IRQ_VBS | DPRX_IRQ_VSTART | DPRX_IRQ_VFP | DPRX_IRQ_VACTIVE1 | DPRX_IRQ_TIME_COUNT;
	dprx_pr_info("[DPRX] 0x%x\n", irq_ien);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_TIMING_INTR_ENABLE, irq_ien);
	/* for test final del: test linkconfig irq */
	irq_ien = GENMASK(17, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB18, irq_ien); // LINKCFG INT

	irq_ien = GENMASK(13, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB24, irq_ien); // GTC INT

	irq_ien = GENMASK(1, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB54, irq_ien); // error INT

	irq_ien = 0;
	irq_ien = BIT(0) | BIT(1) | BIT(3) | BIT(4);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB60, irq_ien); // debug INT

	irq_ien = GENMASK(28, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB6C, irq_ien); // rsv INT

	irq_ien = GENMASK(23, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB78, irq_ien); // mon

	irq_ien = GENMASK(23, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB84, irq_ien); // mon en

	irq_ien = GENMASK(10, 0);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB88, irq_ien); // mon type
}

static void dprx_global_intr_en(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	uint32_t irq_ien;
	dprx_pr_info("[DPRX] dprx_global_intr_en +\n");
	/* enable sdp types */
	irq_ien = DPRX_ALL_SDP_EN;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + SDP_RPT_EN, irq_ien);
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + SDP_FIFO_CLR);
	reg |= SDP_REPORT_TIMER_EN;
	/* SDP_REPORT_TIMEOUT_VALUE default is 17ms */
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + SDP_FIFO_CLR, reg);

	/* enable first-level interrupts */
	irq_ien = DPRX_ALL_INTR_ENABLE;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_INTR_ENABLE, irq_ien);

	/* enable second-level interrupts: timing related irq */
	irq_ien = DPRX_IRQ_VSYNC | DPRX_IRQ_VBS | DPRX_IRQ_VSTART | DPRX_IRQ_VFP;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_TIMING_INTR_ENABLE, irq_ien);

	/* enable second-level interrupts: training related irq */
	irq_ien = DPRX_ALL_TRAINING_INTR;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_TRAINING_INTR_ENABLE, irq_ien);

	irq_ien = DPRX_ALL_PSR_INTR;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PSR_INTR_ENABLE, irq_ien);

	/* enable second-level interrupts: sdp/format related irq */
	irq_ien = DPRX_ALL_SDP_VIDEOFORMAT_INTR;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_SDP_VIDEOFORMAT_INTR_ENABLE, irq_ien);
	if (is_need_aux_monitor()) {
		irq_ien = GENMASK(1, 0);
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB78, irq_ien); // mon

		irq_ien = GENMASK(1, 0);
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB84, irq_ien); // mon en
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB88, 0); // mon type
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB8C, 0x206);
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB90, 0x206);
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB94, 0x207);
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB98, 0x207);
	}
	irq_ien = BIT(1);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB54, irq_ien); // error INT
	irq_ien = BIT(3);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB60, irq_ien); // debug INT
	irq_ien = BIT(22) | BIT(23) | BIT(26) | BIT(27);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB6C, irq_ien); // rsv INT
	dprx_pr_info("[DPRX] dprx_global_intr_en -\n");
}

static void dprx_link_training_init_config(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	dprx_pr_info("[DPRX] dprx_link_training_init_config +\n");
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_ATC_CTRL0);
	reg |= DPRX_ATC_MODE;
	reg |= DPRX_ATC_LT_RST_GEN_EN;
	reg |= DPRX_ATC_PWR_DOWN_MODE;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_ATC_CTRL0, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_ATC_CTRL1);
	reg &= ~DPRX_FULL_EQ_LOOP_CNT;
	reg |= (0x3F << DPRX_FULL_EQ_LOOP_CNT_OFFSET); /* for test: 0x3F = (400us * 0.8 / 5) - 1 */
	reg &= ~DPRX_FAST_EQ_LOOP_CNT;
	reg |= 0xF;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_ATC_CTRL1, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PCS_CTRL0);
	reg &= ~DPRX_ICFG_BS_LOCK_NUM_OFFSET;
	reg |= (0x3 << DPRX_ICFG_BS_LOCK_NUM_OFFSET);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PCS_CTRL0, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_GTC_CTRL5);
	reg |= ICFG_TX_GTC_MISS_EN;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_GTC_CTRL5, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PCS_CTRL1);
	reg &= ~DPRX_TRAIN_LOST_SYM_ERR_MASK;
	reg |= (0x40 << DPRX_TRAIN_LOST_SYM_ERR_OFFSET);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PCS_CTRL1, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PCS_CTRL3);
	reg &= ~DPRX_ICFG_DESKEW_TIME_MASK;
	reg |= 0x40;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PCS_CTRL3, reg);

	if (dprx->mode == EDP_MODE) {
		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_HS_DET_CTRL0);
		reg &= ~DPRX_HS_DET_SYM_LEN;
		reg |= (0x20 << DPRX_HS_DET_SYM_LEN_OFFSET);
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_HS_DET_CTRL0, reg);

		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_HS_DET_CTRL1);
		reg &= ~DPRX_HS_DET_DLY_TIME;
		reg |= 0x2;
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_HS_DET_CTRL1, reg);
		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_ALPM_ENABLE);
		reg |= DPRX_HW_ALPM_EN;
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_ALPM_ENABLE, reg);
	}
	dprx_pr_info("[DPRX] dprx_link_training_init_config -\n");
}

static void dprx_phy_glb_reg_init(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	void __iomem *phy_base;
	dprx_pr_info("[DPRX] dprx_phy_glb_reg_init +\n");
	phy_base = dprx->base + DPRX_PHY_REG_OFFSET;
	reg = inp32(phy_base + PMA_AUX); /* 0x24 AUX */
	reg |= REG_DPRX_AUX_PWDNB;
	outp32(phy_base + PMA_AUX, reg);
	/* aux Software Manual */
	outp32(phy_base + PMA_AUX, 0xF003B07C);
	/* 0x30 */
	outp32(phy_base + DA_TEST_EN, 0x01000000);
	/* 0x44 */
	outp32(phy_base + REG_T_DA_2, 0x00000000);
	// r0---r7
	outp32(phy_base + DA_RATE_R0,   0x13121315);
	outp32(phy_base + DA_RATE_R0_2, 0x51AAAAAA);
	outp32(phy_base + DA_RATE_R0_3, 0x011B6854);
	outp32(phy_base + DA_RATE_R0_4, 0x00000120);

	outp32(phy_base + DA_RATE_R1,   0x0F101405);
	outp32(phy_base + DA_RATE_R1_2, 0x41AAAAAA);
	outp32(phy_base + DA_RATE_R1_3, 0x01D8688C);
	outp32(phy_base + DA_RATE_R1_4, 0x00000120);

	outp32(phy_base + DA_RATE_R2,   0x0F0F1416);
	outp32(phy_base + DA_RATE_R2_2, 0x01AAAAAA);
	outp32(phy_base + DA_RATE_R2_3, 0x03B1608C);
	outp32(phy_base + DA_RATE_R2_4, 0x000000C3);

	outp32(phy_base + DA_RATE_R3,   0x0f0f1416);
	outp32(phy_base + DA_RATE_R3_2, 0x21AAAAAA);
	outp32(phy_base + DA_RATE_R3_3, 0x058960D2);
	outp32(phy_base + DA_RATE_R3_4, 0x000000C3);

	outp32(phy_base + DA_RATE_R4,   0x0F0C1407);
	outp32(phy_base + DA_RATE_R4_2, 0x01AAAAAA);
	outp32(phy_base + DA_RATE_R4_3, 0x017A6870);
	outp32(phy_base + DA_RATE_R4_4, 0x00000120);

	outp32(phy_base + DA_RATE_R5,   0x0F0C1408);
	outp32(phy_base + DA_RATE_R5_2, 0x01AAAAAA);
	outp32(phy_base + DA_RATE_R5_3, 0x01A9687E);
	outp32(phy_base + DA_RATE_R5_4, 0x00000120);

	outp32(phy_base + DA_RATE_R6,   0x0F0C1408);
	outp32(phy_base + DA_RATE_R6_2, 0x01AAAAAA);
	outp32(phy_base + DA_RATE_R6_3, 0x02376054);
	outp32(phy_base + DA_RATE_R6_4, 0x00000120);

	outp32(phy_base + DA_RATE_R7,   0x0F0C1408);
	outp32(phy_base + DA_RATE_R7_2, 0x01AAAAAA);
	outp32(phy_base + DA_RATE_R7_3, 0x02F46070);
	outp32(phy_base + DA_RATE_R7_4, 0x00000161);
	dprx_debug_set_phy_params(dprx);
	dprx_pr_info("[DPRX] dprx_phy_glb_reg_init -\n");
}

static void dprx_phy_crg_reg_set(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	dprx_pr_info("[DPRX] dprx_phy_crg_reg_set +\n");
	if (dprx->id == 0) {
		reg = inp32(dprx->hsdt1_crg_base + DPRX0_DIS_RESET_OFFSET); /* 0x64 */
		reg |= DPRX0_RST_HIDPRX0PHY_POR_BIT;
		reg |= DPRX0_IP_PRST_HIDPRX0PHY_BIT;
		outp32(dprx->hsdt1_crg_base + DPRX0_DIS_RESET_OFFSET, reg);

		reg = inp32(dprx->hsdt1_crg_base + DPRX0_CLK_GATE_OFFSET); /* 0x10 */
		reg |= DPRX0_GT_PCLK_HIDPRX0PHY;
		reg |= DPRX0_GT_CLK_HIDPRX0PHY_REF;
		reg |= DPRX0_GT_CLK_HIDPRX0PHY_AUX;
		outp32(dprx->hsdt1_crg_base + DPRX0_CLK_GATE_OFFSET, reg);
	} else {
		reg = inp32(dprx->hsdt1_crg_base + DPRX1_DIS_RESET_OFFSET); /* 0x64 */
		reg |= DPRX0_RST_HIDPRX1PHY_POR_BIT;
		reg |= DPRX0_IP_PRST_HIDPRX1PHY_BIT;
		outp32(dprx->hsdt1_crg_base + DPRX1_DIS_RESET_OFFSET, reg);

		reg = inp32(dprx->hsdt1_crg_base + DPRX1_CLK_GATE_OFFSET); /* 0x10 */
		reg |= DPRX0_GT_PCLK_HIDPRX1PHY;
		reg |= DPRX0_GT_CLK_HIDPRX1PHY_REF;
		reg |= DPRX0_GT_CLK_HIDPRX1PHY_AUX;
		outp32(dprx->hsdt1_crg_base + DPRX1_CLK_GATE_OFFSET, reg);
	}
	if (is_asic_process())
		dprx_phy_glb_reg_init(dprx);
	dprx_pr_info("[DPRX] dprx_phy_crg_reg_set -\n");
}

static void dprx_phy_cross_bar(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	uint32_t base_offset;
	void __iomem *phy_base;
	// default positive
	if (dprx->intface_type == TYPEC_INTERFACE && dprx->typec_orien == TYPEC_ORIEN_NEGATIVE) {
		dprx_pr_info("[DPRX] dprx_phy_cross_bar +\n");
		phy_base = dprx->base + DPRX_PHY_REG_OFFSET;
		reg = inp32(phy_base + XBAR_AUX_PULSE);
		reg &= ~REG_PHY_MODE_MASK;
		reg |= (9 << REG_PHY_MODE_OFFSET);
		outp32(phy_base + XBAR_AUX_PULSE, reg);
		dprx_pr_info("[DPRX] dprx_phy_cross_bar -\n");
	}
}

static int dprx_cal_phy_rate(struct dprx_ctrl *dprx)
{
	if(is_open_ctrl_debug())
		return dprx->debug_params.max_link_rate;
	else
		return 0x3;
}

void dprx_phy_wakeup_phy(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
	reg &= ~DPRX_PHY_LANE_POWER_CTRL_SEL;
	reg |= (0xF << DPRX_PHY_LANE_POWER_CTRL_SEL_OFFSET);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RWR_CTRL);
	reg &= ~GENMASK(15, 0); // for test, P0
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RWR_CTRL, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
	reg &= ~DPRX_PHY_LANE_POWER_CTRL_SEL;
	reg |= (0x0 << DPRX_PHY_LANE_POWER_CTRL_SEL_OFFSET);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, reg);
}

static void dprx_phy_init_config(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	uint32_t i;
	uint32_t status = 0;

	dprx_phy_crg_reg_set(dprx);

	dprx_phy_cross_bar(dprx);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
	reg |= DPRX_PHY_RESET;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, reg);
	if (!is_asic_process()) {
		reg = 0;
		reg |= BIT(0);
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_POWER_CTRL, reg);
		dprx_pr_info("[DPRX] hardware phy rst, reg = %x\n", reg);
	}
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
	reg |= DPRX_PHY_LANE_RESET;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, reg);
	dprx_pr_info("[DPRX] dprx_phy_init_config, reg = %x\n", reg);

	// wait rst finish
	for (i = 0; i < 5 && is_asic_process(); i++) {
		udelay(2);
		status = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_STATUS);
		dprx_pr_info("[DPRX] dprx_phy_init_config, status = %x\n", status & 0xf);
		if ((status & 0xf) == 0) {
			break;
		}
	}

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
	reg |= DPRX_PHY_RATE_SEL_BIT;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RATE_CTRL);
	if (is_asic_process())
		reg = dprx_cal_phy_rate(dprx);
	else
		reg |= 1; // RBR
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RATE_CTRL, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
	reg &= ~DPRX_PHY_RATE_SEL_BIT;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, reg);
	udelay(2);
	// already delete in Software Process
	if (!is_asic_process()) {
		dprx_phy_wakeup_phy(dprx);
		/* for new logic */
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_GTC_CTRL, 0x5000b);
	}
	dprx_pr_info("[DPRX] dprx_phy_init_config -\n");
}

static void dprx_set_hpd(struct dprx_ctrl *dprx, bool enable)
{
	uint32_t reg;
	dprx_pr_info("[DPRX] dprx_set_hpd +\n");
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_HPD_CTRL0);
	reg |= DPRX_HPD_EN;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_HPD_CTRL0, reg);

	/* for test: set rate/lane num to keep fpga 2.7G 4 lane */
	if (!is_asic_process()) {
		reg = dprx_readl(dprx, 0x100);
		reg = 0x40A;
		dprx_writel(dprx, 0x100, reg);
	}
	gpio_set_value(dprx->hpd_gpio, enable);

	dprx_pr_info("[DPRX] dprx_set_hpd -\n");
}

static void dprx_fec_init_config(struct dprx_ctrl *dprx)
{
	/* 1、Deassert Reset；2、Open the control； 3、Enable */
	uint32_t reg;
	dprx_pr_info("[DPRX] dprx_fec_init_config +\n");
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET);
	reg |= DPRX_FEC_RESET_BIT;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_CLK_CTRL);
	reg |= DPRX_FEC_CLK_BIT;
	reg |= DPRX_FEC_EUC_EN_BIT;
	reg |= DPRX_FEC_SEARCH_EN_BIT;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_CLK_CTRL, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_FEC_CTRL);
	reg &= ~DPRX_FEC_BYPASS_BIT;
	reg |= DPRX_FEC_EN_BIT;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_FEC_CTRL, reg);
	dprx_pr_info("[DPRX] dprx_fec_init_config -\n");
}

static void dprx_aux_init(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, DPRX_AUX_CTRL0);
	reg |= DPRX_SYNC_NUM_BIT;
	dprx_writel(dprx, DPRX_AUX_CTRL0, reg);
}

static void dprx_data_path_sel(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	dprx_pr_info("[DPRX] dprx_data_path_sel +\n");
	reg = inp32(dprx->hsdt1_sys_ctrl_base + DPRX_MUX_SEL); /* 0xEB040000 + 0x30 */
	reg &= ~DPRX_DPTX_SEL;
	if (dprx->id == 0) { // dprx0->dsd0
		reg &= ~DPRX_DSS0_SEL;
		reg |= (1 << DPRX0_DSS0_SEL_OFFSET);
		reg |= (1 << DPRX0_DPTX_SEL_OFFSET);
		if(is_rx0_phy_switch())
			reg |= DPRX0_PHY_SEL; // rx0 select ph1
	} else { // dprx1->dsd1
		reg &= ~DPRX_DSS1_SEL;
		reg |= (1 << DPRX1_DSS1_SEL_OFFSET);
		reg |= (1 << DPRX1_DPTX_SEL_OFFSET);
	}
	outp32(dprx->hsdt1_sys_ctrl_base + DPRX_MUX_SEL, reg);
	dprx_pr_info("[DPRX] dprx_data_path_sel -\n");
}

static void dprx_hdp_map_gpio(struct dprx_ctrl *dprx)
{
	dprx_pr_info("[DPRX] dprx_hdp_map_gpio, id %u +\n", dprx->id);
	if (dprx->id == 0)
		outp32(dprx->ioc_peri_base + IOMG_SE_10, 2); // gpio39-->dprx0 hpd
	else
		outp32(dprx->ioc_peri_base + IOMG_SE_12, 2); // gpio40-->dprx2 hpd
	dprx_pr_info("[DPRX] dprx_hdp_map_gpio -\n");
}

void dprx_core_on(struct dprx_ctrl *dprx)
{
	dpu_check_and_no_retval((dprx == NULL), err, "dprx is NULL!\n");

	dprx_pr_info("[DPRX] dprx_core_on +\n");
	dprx_dis_reset(dprx, true);
	dprx_edid_init_config(dprx);

	dprx_data_path_sel(dprx);

	dprx_soft_reset_all(dprx);
	dprx_clk_ctrl_enable(dprx);

	dprx_phy_init_config(dprx);

	dprx_init_dpcd(dprx);

	dprx_global_intr_dis(dprx);
	enable_irq(dprx->irq);
	/* Enable all top-level interrupts */
	dprx_global_intr_en(dprx);
	dprx_pr_info("[DPRX] dprx_global_intr_en2\n");

	dprx_link_training_init_config(dprx); // support FULL/ Fast LK

	/* fec config */
	if (dprx->mode == DP_MODE)
		dprx_fec_init_config(dprx);
	if (dprx->mode == EDP_MODE)
		dprx_aux_init(dprx);
	dprx_set_hpd(dprx, true);
	dprx_hdp_map_gpio(dprx);
	dprx_pr_info("[DPRX] dprx_core_on -\n");
}

void dprx_core_off(struct dprx_ctrl *dprx)
{
	dpu_check_and_no_retval((dprx == NULL), err, "dprx is NULL!\n");

	dprx_set_hpd(dprx, false);

	dprx_global_intr_dis(dprx);

	disable_irq(dprx->irq);

	/* reset */
	dprx_dis_reset(dprx, false);
	/* set high resistance */
	dprx_phy_ctrl_reg_resistance_high(dprx);
}
