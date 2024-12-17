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

#include "hidprx_irq.h"
#include "hidprx_core.h"
#include "hidprx_reg.h"
#include "hidprx_dbg.h"

static void dprx_irq_power_ctl(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	uint32_t phy_value;
	uint8_t dpcd_value;

	dpcd_value = dprx_read_dpcd(dprx, 0x600);
	dprx_pr_info("[DPRX] DPRX_IRQ_AUX_WR_600: powerstate cur:%d, last:%d \n", dpcd_value, dprx->dpcds.power_state);
	if ((dpcd_value == 0x2 || dpcd_value == 0x5) && (dprx->dpcds.power_state == 1)) {  // enter P3
		dprx_pr_info("[DPRX] enter Low power consumption\n");
		// 1、close video
		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_VIDEO_CTRL);
		reg &= ~VIDEO_OUT_EN;
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_VIDEO_CTRL, reg);
		// 2、close hs_det en
		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_HS_DET_CTRL0);
		reg &= ~DPRX_HS_DET_EN;
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_HS_DET_CTRL0, reg);
		// 3、soft ctl lane
		phy_value = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
		phy_value |= LANE_POWERDOWN_SEL;
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, phy_value);
		// 4、enter P3
		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RWR_CTRL);
		reg &= ~DPRX_PHY_LANE_POWERDOWN;
		reg |= DPRX_4LANE_ENTER_P3; // 0:P0   1:P1   2:P2   3:P3, 0~3 lane
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RWR_CTRL, reg);
		// 5、todo : wait 200um
		// 6、restore hardware ctl lane
		phy_value &= ~LANE_POWERDOWN_SEL;
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, phy_value);
	}
	if (dpcd_value == 0x1) {
		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RWR_CTRL);
		dprx_pr_info("[DPRX] reg : 0x%x\n", reg);
		if ((dprx->dpcds.power_state == 2 || dprx->dpcds.power_state == 5) ||
			((reg & DPRX_PHY_LANE_POWERDOWN) == DPRX_4LANE_ENTER_P3)) { // if P3 then exist P3
			dprx_pr_info("[DPRX] Exit Low Power Consumption\n", reg);
			// 1、soft ctl lane
			phy_value = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
			phy_value |= LANE_POWERDOWN_SEL;
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, phy_value);
			// 2、enter P0
			reg &= ~DPRX_PHY_LANE_POWERDOWN;
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RWR_CTRL, reg);
			// 3、todo : wait 50um
			udelay(50);
			// 4、restore hardware ctl lane
			phy_value &= ~LANE_POWERDOWN_SEL;
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, phy_value);
			// 5、open hs_det en
			reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_HS_DET_CTRL0);
			reg |= DPRX_HS_DET_EN;
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_HS_DET_CTRL0, reg);
		}
	}
	dprx->dpcds.power_state = dpcd_value;
}

static void dprx_irq_wakeup_phy(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	uint8_t dpcd_value;
	uint32_t phyifctrl;
	uint32_t value;

	reg = dprx_readl(dprx, 0x100);
	dpcd_value = (reg & GENMASK(23, 16)) >> 16;
	dprx_pr_info("[DPRX] DPCD103-100 = 0x%x, last102 = 0x%x, TPS Pattern Sequence 0x%x\n",
		reg, dprx->dpcds.link_cfgs[2], dpcd_value & GENMASK(3, 0));
	if ((dprx->dpcds.link_cfgs[2] & GENMASK(3, 0)) != (dpcd_value & GENMASK(3, 0))) {
		// asic is not need
		if ((dpcd_value & GENMASK(3, 0)) == 1 && !is_asic_process()) {
			phyifctrl = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
			phyifctrl &= ~DPRX_PHY_LANE_RESET;
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, phyifctrl);
			phyifctrl |= DPRX_PHY_LANE_RESET;
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, phyifctrl);
			dprx_pr_info("[DPRX] TPS1 Sel, Phy reset\n");
		}
	}

	if (is_asic_process() && (1 == (dpcd_value & GENMASK(3, 0)))) {
		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_SCTRL_DFX_DEBUG_SELECT);
		reg &= ~DPRX_DBG_PHY_SEL_MASK;
		reg |= (1 << 4);
		dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_SCTRL_DFX_DEBUG_SELECT, reg);

		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_SCTRL_DFX_DEBUG_BUS);
		value = ((reg & DPRX_DBG_PHY_CTRL_MASK) >> 16);
		dprx_pr_info("[DPRX] cur phy state 0x%x\n",value);
		if (value == 2) {
			dprx_phy_wakeup_phy(dprx);
			udelay(200);
			reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_SCTRL_ERR_INTR_MASKED_STATUS);
			value = ((reg & DPRX_MASKED_LANE_PHYSTATUS_MASK) >> 2);
			dprx_pr_info("[DPRX] wakeup phy status 0x%x\n",value);
		}
	}

	dprx->dpcds.link_cfgs[2] = dpcd_value;
}

static void dprx_irq_fifo_rst(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	dprx_pr_info("[DPRX] close video\n");
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_VIDEO_CTRL);
	reg &= ~VIDEO_OUT_EN;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_VIDEO_CTRL, reg);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + SDP_FIFO_CLR);
	reg |= ICFG_SDP_FIFO_CLR;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + SDP_FIFO_CLR, reg);
	reg &= ~ICFG_SDP_FIFO_CLR;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + SDP_FIFO_CLR, reg);
	dprx_pr_info("[DPRX] fifo reset\n");
	dprx->dsc_en = 0;
	reg = dprx_readl(dprx, 0x210);
	dprx_pr_info("[DPRX] 0x210 = 0x%x\n", reg);
	reg = dprx_readl(dprx, 0x214);
	dprx_pr_info("[DPRX] 0x214 = 0x%x\n", reg);
}

static void dprx_swich_phy_parm(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	void __iomem *phy_base;

	phy_base = dprx->base + DPRX_PHY_REG_OFFSET;
	reg = dprx_readl(dprx, 0x100);
	dprx_pr_info("[DPRX] set rate 0x%x \n", (reg & 0xff));
	if ((reg & 0xff) == 0x1E) { // HBR3
		dprx_pr_info("[DPRX] switch to HBR3 \n");
			dprx_pr_info("[DPRX] DA_TEST_EN_2 = 0x%x \n", dprx_get_phy_da_test_en_2());
			outp32(phy_base + DA_TEST_EN_2, dprx_get_phy_da_test_en_2());
			dprx_pr_info("[DPRX] REG_T_DA_2 = 0x%x \n", dprx_get_phy_reg_t_da_2());
			outp32(phy_base + REG_T_DA_2, dprx_get_phy_reg_t_da_2());
			dprx_pr_info("[DPRX] REG_T_DA_2 = 0x%x \n", dprx_get_phy_reg_t_da_13());
			outp32(phy_base + REG_T_DA_13, dprx_get_phy_reg_t_da_13());
	} else {
			dprx_pr_info("[DPRX] switch normal rate \n");
			outp32(phy_base + DA_TEST_EN_2, 0x00000000);
			outp32(phy_base + REG_T_DA_2, 0x00000000);
			outp32(phy_base + REG_T_DA_13, 0x00000000);
	}
}

static void dprx_contrl_lane_enter_p3(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	uint32_t phy_value;
	reg = dprx_readl(dprx, 0x100);
	int i;
	int lane_count;
	lane_count = (reg >> 8) & 0xf;
	dprx_pr_info("[DPRX] lane_count = %d \n", lane_count);
	if (lane_count >= 4)
		return;
	/* dp linkTraining only 1/2/4 lane */
	phy_value = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL);
	if (lane_count == 1) {
		phy_value |= DPRX_LANE1_POWERDOWN_BIT;
		phy_value |= DPRX_LANE2_POWERDOWN_BIT;
		phy_value |= DPRX_LANE3_POWERDOWN_BIT;
	} else {
		phy_value |= DPRX_LANE2_POWERDOWN_BIT;
		phy_value |= DPRX_LANE3_POWERDOWN_BIT;
	}
	dprx_pr_info("[DPRX] phy_value = 0x%x \n", phy_value);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, phy_value);

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RWR_CTRL);
	if (lane_count == 1) {
		reg &= ~DPRX_LANE1_POWERDOWN;
		reg &= ~DPRX_LANE2_POWERDOWN;
		reg &= ~DPRX_LANE3_POWERDOWN;
		reg |= (DPRX_LANE_STATUS_P3 << 4) | (DPRX_LANE_STATUS_P3 << 8) | (DPRX_LANE_STATUS_P3 << 12);
	} else {
		reg &= ~DPRX_LANE2_POWERDOWN;
		reg &= ~DPRX_LANE3_POWERDOWN;
		reg |= (DPRX_LANE_STATUS_P3 << 8) | (DPRX_LANE_STATUS_P3 << 12);
	}
	dprx_pr_info("[DPRX] reg = 0x%x \n", reg);
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RWR_CTRL, reg);

	if (lane_count == 1) {
		phy_value &= ~DPRX_LANE1_POWERDOWN_BIT;
		phy_value &= ~DPRX_LANE2_POWERDOWN_BIT;
		phy_value &= ~DPRX_LANE3_POWERDOWN_BIT;
	} else {
		phy_value &= ~DPRX_LANE2_POWERDOWN_BIT;
		phy_value &= ~DPRX_LANE3_POWERDOWN_BIT;
	}
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PHY_RST_CTRL, phy_value);
	return;
}

static void dprx_link_print_lane0_swing(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, 0x100);
	dprx_pr_info("[DPRX] lane0:swing %d, pre %d, dpcd 0x100 0x%x\n", (reg >> 24) & 0x3 , (reg >> 27) & 0x3, reg);
}

static void dprx_link_print_lane1_swing(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, 0x104);
	dprx_pr_info("[DPRX] lane1:swing %d, pre %d, dpcd 0x104 0x%x\n", (reg >> 0) & 0x3, (reg >> 3) & 0x3, reg);
}

static void dprx_link_print_lane2_swing(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, 0x104);
	dprx_pr_info("[DPRX] lane2:swing %d, pre %d, dpcd 0x104 0x%x\n", (reg >> 8) & 0x3, (reg >> 11) & 0x3, reg);
}

static void dprx_link_print_lane3_swing(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, 0x104);
	dprx_pr_info("[DPRX] lane3:swing %d, pre %d, dpcd 0x104 0x%x\n", (reg >> 16) & 0x3, (reg >> 19) & 0x3, reg);
}
struct dprx_description_handling g_link_trainning[] = {
	{DPRX_IRQ_AUX_WR_600,             "[DPRX] DPRX_IRQ_AUX_WR_600",             dprx_irq_power_ctl},
	{DPRX_IRQ_AUX_WR_100,             "[DPRX] DPRX_IRQ_AUX_WR_100",             dprx_swich_phy_parm},
	{DPRX_IRQ_AUX_WR_101,             "[DPRX] DPRX_IRQ_AUX_WR_101",             NULL},
	{DPRX_IRQ_AUX_WR_102,             "[DPRX] DPRX_IRQ_AUX_WR_102",             dprx_irq_wakeup_phy},
	{DPRX_IRQ_AUX_WR_103,             "[DPRX] DPRX_IRQ_AUX_WR_103",             dprx_link_print_lane0_swing},
	{DPRX_IRQ_AUX_WR_104,             "[DPRX] DPRX_IRQ_AUX_WR_104",             dprx_link_print_lane1_swing},
	{DPRX_IRQ_AUX_WR_105,             "[DPRX] DPRX_IRQ_AUX_WR_105",             dprx_link_print_lane2_swing},
	{DPRX_IRQ_AUX_WR_106,             "[DPRX] DPRX_IRQ_AUX_WR_106",             dprx_link_print_lane3_swing},
	{DPRX_IRQ_ATC_TRAIN_TIMEOUT,      "[DPRX] DPRX_IRQ_ATC_TRAIN_TIMEOUT",      NULL},
	{DPRX_IRQ_DESKEW_TIMEOUT,         "[DPRX] DPRX_IRQ_DESKEW_TIMEOUT",         NULL},
	{DPRX_IRQ_DESKEW_DONE,            "[DPRX] DPRX_IRQ_DESKEW_DONE",            dprx_contrl_lane_enter_p3},
	{DPRX_IRQ_ATC_TRAIN_DONE,         "[DPRX] DPRX_IRQ_ATC_TRAIN_DONE",         NULL},
	{DPRX_IRQ_ATC_TRAIN_LOST,         "[DPRX] DPRX_IRQ_ATC_TRAIN_LOST",         dprx_irq_fifo_rst},
	{DPRX_IRQ_ATC_PWR_DONE_LOST,      "[DPRX] DPRX_IRQ_ATC_PWR_DONE_LOST",      NULL},
	{DPRX_IRQ_MAIN_LINK_STS_DET_LOST, "[DPRX] DPRX_IRQ_MAIN_LINK_STS_DET_LOST", NULL},
	{DPRX_IRQ_TPS234_DET,             "[DPRX] DPRX_IRQ_TPS234_DET",             NULL},
};

static void dprx_handle_training_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t i;

	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_TRAINING_INTR_MASKED_STATUS);
	dprx_pr_info("training 0x%x\n", irq_sts);
	for (i = 0 ; i < ARRAY_SIZE(g_link_trainning); i++) {
		if (irq_sts & g_link_trainning[i].bit_pos) {
			dprx_pr_info("%s\n", g_link_trainning[i].description);
			if (g_link_trainning[i].oprate != NULL)
				g_link_trainning[i].oprate(dprx);
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_TRAINING_INTR_ORI_STATUS, g_link_trainning[i].bit_pos);
		}
	}
}

struct dprx_description_handling g_timing_irq[] = {
	{DPRX_IRQ_VSYNC,         "[DPRX] DPRX_IRQ_VSYNC",         NULL},
	{DPRX_IRQ_HSYNC,         "[DPRX] DPRX_IRQ_HSYNC",         NULL},
	{DPRX_IRQ_VBS,           "[DPRX] DPRX_IRQ_VBS",           NULL},
	{DPRX_IRQ_VSTART,        "[DPRX] DPRX_IRQ_VSTART",        NULL},
	{DPRX_IRQ_VSYNC_TO_DSS,  "[DPRX] DPRX_IRQ_VSYNC_TO_DSS",  NULL},
	{DPRX_IRQ_HSYNC_TO_DSS,  "[DPRX] DPRX_IRQ_HSYNC_TO_DSS",  NULL},
	{DPRX_IRQ_VBS_TO_DSS,    "[DPRX] DPRX_IRQ_VBS_TO_DSS",    NULL},
	{DPRX_IRQ_VSTART_TO_DSS, "[DPRX] DPRX_IRQ_VSTART_TO_DSS", NULL},
	{DPRX_IRQ_VFP,           "[DPRX] DPRX_IRQ_VFP",           NULL},
	{DPRX_IRQ_VACTIVE1,      "[DPRX] DPRX_IRQ_VACTIVE1",      NULL},
	{DPRX_IRQ_VBS_DPRX,      "[DPRX] DPRX_IRQ_VBS_DPRX",      NULL},
	{DPRX_IRQ_TIME_COUNT,    "[DPRX] DPRX_IRQ_TIME_COUNT",    NULL},
};

static void dprx_handle_timing_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t i;

	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_TIMING_INTR_MASKED_STATUS);
	for (i = 0; i < ARRAY_SIZE(g_timing_irq); i++) {
		if (irq_sts & g_timing_irq[i].bit_pos) {
			dprx_pr_debug("%s\n", g_timing_irq[i].description);
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_TIMING_INTR_ORI_STATUS, g_timing_irq[i].bit_pos);
		}
	}
}

static const struct pixel_enc_info {
	enum pixel_enc_type pix_enc;
	char desc[10];
} enc_info_list[PIXEL_ENC_INVALID] = {
	{ RGB, "RGB" },
	{ YCBCR420, "YUV420" },
	{ YCBCR422, "YUV422" },
	{ YCBCR444, "YUV444" },
	{ YONLY, "YONLY" },
	{ RAW, "RAW" },
};

static void dprx_handle_video_parmas(struct dprx_ctrl *dprx)
{
	if (dprx->v_params.video_format_valid == false || dprx->msa.msa_valid == false)
		return;

	dprx->video_input_params_ready = true;
	dprx_pr_info("Get Input Parameters:");
	if (dprx->v_params.pix_enc >= PIXEL_ENC_INVALID)
		dprx_pr_info("ColorSpace: Invalid\n");
	else
		dprx_pr_info("ColorSpace: %s\n", enc_info_list[dprx->v_params.pix_enc].desc);
	dprx_pr_info("Bpc: %u\n", dprx->v_params.bpc);

	dprx_pr_info("**************************************************************");
	dprx_pr_info("MSA MVID, NVID: %u, %u\n", dprx->msa.mvid, dprx->msa.nvid);
	dprx_pr_info("MSA h_total, h_width, h_start, hsw, hsp: %u, %u, %u, %u, %u\n",
				dprx->msa.h_total, dprx->msa.h_width, dprx->msa.h_start, dprx->msa.hsw, dprx->msa.hsp);
	dprx_pr_info("MSA v_total, v_height, v_start, vsw, vsp: %u, %u, %u, %u, %u\n",
				dprx->msa.v_total, dprx->msa.v_height, dprx->msa.v_start, dprx->msa.vsw, dprx->msa.vsp);
	dprx_pr_info("MSA misc0, misc1: 0x%x, 0x%x\n", dprx->msa.misc0, dprx->msa.misc1);
}

static void dprx_handle_timing_change(struct dprx_ctrl *dprx)
{
	uint32_t reg;

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_MSA_STATUS);
	if (reg & DPRX_MSA_VALID_BIT) {
		/* for test, change to dprx_pr_debug */
		dprx_pr_info("MSA Valid\n");
		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_MSA_MVID);
		dprx->msa.mvid = reg & DPRX_MSA_MVID_MASK;

		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_MSA_NVID);
		dprx->msa.nvid = reg & DPRX_MSA_NVID_MASK;

		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_MISC);
		dprx->msa.misc0 = (reg & DPRX_MISC0_MASK) >> DPRX_MISC0_OFFSET;
		dprx->msa.misc1 = (reg & DPRX_MISC1_MASK) >> DPRX_MISC1_OFFSET;

		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_MSA_TIMING0);
		dprx->msa.v_total = (reg & DPRX_MSA_VTOTAL_MASK) >> DPRX_MSA_VTOTAL_OFFSET;
		dprx->msa.h_total = (reg & DPRX_MSA_HTOTAL_MASK) >> DPRX_MSA_HTOTAL_OFFSET;

		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_MSA_TIMING1);
		dprx->msa.v_start = (reg & DPRX_MSA_VSTART_MASK) >> DPRX_MSA_VSTART_OFFSET;
		dprx->msa.h_start = (reg & DPRX_MSA_HSTART_MASK) >> DPRX_MSA_HSTART_OFFSET;

		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_MSA_TIMING2);
		dprx->msa.vsw = (reg & DPRX_MSA_VSW_MASK) >> DPRX_MSA_VSW_OFFSET;
		dprx->msa.vsp= (reg & DPRX_MSA_VSP_MASK) >> DPRX_MSA_VSP_OFFSET;
		dprx->msa.hsw = (reg & DPRX_MSA_HSW_MASK) >> DPRX_MSA_HSW_OFFSET;
		dprx->msa.hsp= (reg & DPRX_MSA_HSP_MASK) >> DPRX_MSA_HSP_OFFSET;

		reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_MSA_TIMING3);
		dprx->msa.v_height = (reg & DPRX_MSA_VHEIGHT_MASK) >> DPRX_MSA_VHEIGHT_OFFSET;
		dprx->msa.h_width = (reg & DPRX_MSA_HWIDTH_MASK) >> DPRX_MSA_HWIDTH_OFFSET;

		dprx->msa.msa_valid = true;
	} else {
		dprx_pr_info("MSA Invalid\n");
		dprx->msa.msa_valid = false;
	}
}

static const enum color_depth color_depth_maping[DPRX_COLOR_DEPTH_INDEX_MAX] = {
	COLOR_DEPTH_6, COLOR_DEPTH_7, COLOR_DEPTH_8, COLOR_DEPTH_10,
	COLOR_DEPTH_12, COLOR_DEPTH_14, COLOR_DEPTH_16
};

static const enum pixel_enc_type pixel_enc_maping[DPRX_PIXEL_ENC_INDEX_MAX] = {
	RGB, YCBCR422, YCBCR420, YONLY, RAW, YCBCR444
};

static void dprx_handle_format_change(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	uint32_t value;

	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_LINK_STATUS);
	if (reg & DPRX_VIDEO_FMT_VALID) {
		/* for test, change to dprx_pr_debug */
		dprx_pr_info("Format Valid\n");

		value = (reg & DPRX_COLOR_SPACE_MASK) >> DPRX_COLOR_SPACE_OFFSET;
		if (value >= DPRX_PIXEL_ENC_INDEX_MAX)
			dprx->v_params.pix_enc = PIXEL_ENC_INVALID;
		else
			dprx->v_params.pix_enc = pixel_enc_maping[value];

		value = (reg & DPRX_BPC_NUM_MASK) >> DPRX_BPC_NUM_OFFSET;
		if (value >= DPRX_COLOR_DEPTH_INDEX_MAX)
			dprx->v_params.bpc = COLOR_DEPTH_INVALID;
		else
			dprx->v_params.bpc = color_depth_maping[value];

		if ((dprx->v_params.bpc == COLOR_DEPTH_8) ||
			(dprx->v_params.bpc == COLOR_DEPTH_10) ||
			(dprx->v_params.bpc == COLOR_DEPTH_6 && dprx->v_params.pix_enc == RGB)) {
			dprx->v_params.video_format_valid = true;
		} else {
			dprx_pr_err("Format Invalid\n");
			dprx->v_params.video_format_valid = false;
		}
	} else {
		dprx_pr_info("Format Invalid\n");
		dprx->v_params.video_format_valid = false;
	}
}

struct dprx_description_handling g_psr_irq[] = {
	{DPRX_PSR_ENTRY,     "[DPRX] psr entry success",     NULL},
	{DPRX_PSR_PRE_ENTRY, "[DPRX] psr pre entry",         NULL},
	{DPRX_PSR_UPDATE,    "[DPRX] psr update success",    NULL},
	{DPRX_PSR_SU_UPDATE, "[DPRX] psr su update success", NULL},
	{DPRX_PSR_EXIT,      "[DPRX] psr exit success",      NULL},
	{DPRX_SU_INFO,       "[DPRX] psr su info",           NULL},
	{DPRX_AUX_WR_00170H, "[DPRX] wr DPCD 170h",          NULL},
	{DPRX_PSR_ABORT,     "[DPRX] psr abort",             NULL},
};

static void dprx_handle_psr_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t reg;
	uint32_t i;
	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_PSR_INTR_MASKED_STATUS);
	for (i = 0; i < ARRAY_SIZE(g_psr_irq); i++) {
		if (irq_sts & g_psr_irq[i].bit_pos) {
			dprx_pr_info("%s\n", g_psr_irq[i].description);
			if (irq_sts & DPRX_AUX_WR_00170H) {
				reg = dprx_read_dpcd(dprx, 0x170);
				dprx_pr_err("[DPRX] DPCD 170h value:%d\n", reg);
			}
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_PSR_INTR_ORI_STATUS, g_psr_irq[i].bit_pos);
		}
	}
}

static void dprx_handle_start_sdp_work(struct dprx_ctrl *dprx)
{
	if (dprx->dprx_sdp_wq != NULL)
		queue_work(dprx->dprx_sdp_wq, &(dprx->dprx_sdp_work));
}

static void dprx_handle_sdp_err(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_ATC_STATUS);
	if (((reg >> 4) & ATC_TRAIN_STATUS) == ATC_STATUS_TRAIN_DONE)
		dprx_pr_info("[DPRX] DPRX_IRQ_SDP_ERR\n");
}

static void dprx_handle_video_info_chg(struct dprx_ctrl *dprx)
{
	dprx_handle_format_change(dprx);
	dprx_handle_timing_change(dprx);
	dprx_handle_video_parmas(dprx);
}

static void dprx_sdp_fifo_clear(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + SDP_FIFO_CLR);
	reg |= ICFG_SDP_FIFO_CLR;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + SDP_FIFO_CLR, reg);
	reg &= ~ICFG_SDP_FIFO_CLR;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + SDP_FIFO_CLR, reg);
	dprx_pr_info("[DPRX] clear sdp fifo \n");
}
struct dprx_description_handling g_sdp_and_format_irq[] = {
	{DPRX_IRQ_VSYNC_REPORT_SDP,     "[DPRX] DPRX_IRQ_VSYNC_REPORT_SDP",     dprx_handle_start_sdp_work},
	{DPRX_IRQ_SDP_DB_FIFO_AFULL, "[DPRX] DPRX_IRQ_SDP_DB_FIFO_AFULL",         NULL},
	{DPRX_IRQ_SDP_HB_FIFO_AFULL,    "[DPRX] DPRX_IRQ_SDP_HB_FIFO_AFULL",    NULL},
	{DPRX_IRQ_SDP_ERR, "[DPRX] DPRX_IRQ_SDP_ERR", dprx_handle_sdp_err},
	{DPRX_IRQ_FORMAT_CHANGE | DPRX_IRQ_TIMING_CHANGE, "[DPRX] DPRX_IRQ_TIMING_FORMAT_CHANGE", dprx_handle_video_info_chg},
	{DPRX_IRQ_COMP_CHANGE, "[DPRX] DPRX_IRQ_COMP_CHANGE",          NULL},
	{DPRX_IRQ_VIDEO2IDLE,     "[DPRX] DPRX_IRQ_VIDEO2IDLE",             NULL},
	{DPRX_IRQ_IDLE2VIDEO,     "[DPRX] DPRX_IRQ_IDLE2VIDEO",             NULL},
	{DPRX_IRQ_SDP_DB_W_ERROR,     "[DPRX] DPRX_IRQ_SDP_DB_W_ERROR",             dprx_sdp_fifo_clear},
	{DPRX_IRQ_SDP_HB_W_ERROR,     "[DPRX] DPRX_IRQ_SDP_HB_W_ERROR",             dprx_sdp_fifo_clear},
};

static void dprx_handle_sdp_and_format_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t i;

	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_SDP_VIDEOFORMAT_MASKED_STATUS);
	for (i = 0; i < ARRAY_SIZE(g_sdp_and_format_irq); i++) {
		if (irq_sts & g_sdp_and_format_irq[i].bit_pos) {
			if (!(irq_sts & DPRX_IRQ_SDP_ERR))
				dprx_pr_info("%s\n", g_sdp_and_format_irq[i].description);
			if (g_sdp_and_format_irq[i].oprate != NULL)
				g_sdp_and_format_irq[i].oprate(dprx);
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_SDP_VIDEOFORMAT_ORI_STATUS, g_sdp_and_format_irq[i].bit_pos);
		}
	}
}

irqreturn_t dprx_threaded_irq_handler(int irq, void *dev)
{
	dpu_check_and_return(!dev, IRQ_HANDLED, err, "[DPRX] dev is NULL\n");

	return IRQ_HANDLED;
}

struct dprx_description_handling g_linkcfg_irq[] = {
	{BIT(0),  "[DPRX] DPRX_IRQ_AUX_WR_107",  NULL},
	{BIT(1),  "[DPRX] DPRX_IRQ_AUX_WR_108",  NULL},
	{BIT(2),  "[DPRX] DPRX_IRQ_AUX_WR_109",  NULL},
	{BIT(3),  "[DPRX] DPRX_IRQ_AUX_WR_10a",  NULL},
	{BIT(4),  "[DPRX] DPRX_IRQ_AUX_WR_10b", NULL},
	{BIT(5),  "[DPRX] DPRX_IRQ_AUX_WR_10c", NULL},
	{BIT(6),  "[DPRX] DPRX_IRQ_AUX_WR_10d", NULL},
	{BIT(7),  "[DPRX] DPRX_IRQ_AUX_WR_10e", NULL},
	{BIT(8),  "[DPRX] DPRX_IRQ_AUX_WR_111",  NULL},
	{BIT(9),  "[DPRX] DPRX_IRQ_AUX_WR_112",  NULL},
	{BIT(10), "[DPRX] DPRX_IRQ_AUX_WR_113",  NULL},
	{BIT(11), "[DPRX] DPRX_IRQ_AUX_WR_114",  NULL},
	{BIT(12), "[DPRX] DPRX_IRQ_AUX_WR_115",  NULL},
	{BIT(13), "[DPRX] DPRX_IRQ_AUX_WR_116",  NULL},
	{BIT(14), "[DPRX] DPRX_IRQ_AUX_WR_117",  NULL},
	{BIT(15), "[DPRX] DPRX_IRQ_AUX_WR_118",  NULL},
	{BIT(16), "[DPRX] DPRX_IRQ_AUX_WR_119",  NULL},
	{BIT(17), "[DPRX] DPRX_IRQ_AUX_WR_120",  NULL},
};

void dprx_handle_linkcfg_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t i;

	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + 0x00B1C);
	for (i = 0; i < ARRAY_SIZE(g_linkcfg_irq); i++) {
		if (irq_sts & g_linkcfg_irq[i].bit_pos) {
			dprx_pr_info("%s\n", g_linkcfg_irq[i].description);
			if (irq_sts & BIT(12))
				dprx_pr_info("[DPRX] DPRX_IRQ_AUX_WR_115:0x%x \n", dprx_read_dpcd(dprx, 0x115));
			if (g_linkcfg_irq[i].oprate != NULL)
				g_linkcfg_irq[i].oprate(dprx);
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB20, g_linkcfg_irq[i].bit_pos);
		}
	}
}

struct dprx_description_handling g_gtc_irg[] = {
	{BIT(0),  "[DPRX] gtc_lock_done_int_enable",  NULL},
	{BIT(1),  "[DPRX] DPRX_IRQ_AUX_WR_154",  NULL},
	{BIT(2),  "[DPRX] DPRX_IRQ_AUX_WR_155",  NULL},
	{BIT(3),  "[DPRX] DPRX_IRQ_AUX_WR_156",  NULL},
	{BIT(4),  "[DPRX] DPRX_IRQ_AUX_WR_157", NULL},
	{BIT(5),  "[DPRX] DPRX_IRQ_AUX_WR_158", NULL},
	{BIT(6),  "[DPRX] DPRX_IRQ_AUX_WR_159", NULL},
	{BIT(7),  "[DPRX] DPRX_IRQ_AUX_WR_15a", NULL},
	{BIT(8),  "[DPRX] DPRX_IRQ_AUX_WR_15b",  NULL},
	{BIT(9),  "[DPRX] DPRX_IRQ_AUX_WR_15c",  NULL},
	{BIT(10), "[DPRX] DPRX_IRQ_AUX_WR_15d",  NULL},
	{BIT(11), "[DPRX] DPRX_IRQ_AUX_WR_15e",  NULL},
	{BIT(12), "[DPRX] DPRX_IRQ_AUX_WR_15f",  NULL},
	{BIT(13), "[DPRX] DPRX_IRQ_AUX_WR_160",  NULL},
	{BIT(14), "[DPRX] gtc_unlock_int_enable",  NULL},
};

void dprx_handle_gtc_irg(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t i;

	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + 0x00B1C);
	for (i = 0; i < ARRAY_SIZE(g_gtc_irg); i++) {
		if (irq_sts & g_gtc_irg[i].bit_pos) {
			dprx_pr_info("%s\n", g_gtc_irg[i].description);
			if (irq_sts & BIT(12))
				dprx_pr_info("[DPRX] DPRX_IRQ_AUX_WR_115:0x%x \n", dprx_read_dpcd(dprx, 0x115));
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB20, g_gtc_irg[i].bit_pos);
		}
	}
}

static void dprx_print_lane0_1_status(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, 0x200);
	dprx_pr_info("[DPRX] sink_cont %d \n", reg & 0x3f );
	dprx_pr_info("[DPRX]        cr_done  eq_done  symbol_locked\n");
	dprx_pr_info("[DPRX] lane0:[  %d,      %d,          %d      ] \n",
		(reg & BIT(16)) >> 16, (reg & BIT(17)) >> 17, (reg & BIT(18)) >> 18);
	dprx_pr_info("[DPRX] lane1:[  %d,      %d,          %d      ] \n",
		(reg & BIT(20)) >> 20, (reg & BIT(21)) >> 21, (reg & BIT(22)) >> 22);
}

static void dprx_print_lane2_3_status(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, 0x200);
	dprx_pr_info("[DPRX] sink_cont %d \n", reg & 0x3f );
	dprx_pr_info("[DPRX]        cr_done  eq_done  symbol_locked\n");
	dprx_pr_info("[DPRX] lane2:[  %d,      %d,          %d      ] \n",
		(reg & BIT(24)) >> 24, (reg & BIT(25)) >> 25, (reg & BIT(26)) >> 26);
	dprx_pr_info("[DPRX] lane3:[  %d,      %d,          %d      ] \n",
		(reg & BIT(28)) >> 28, (reg & BIT(29)) >> 29, (reg & BIT(30)) >> 30);
}
struct dprx_description_handling g_rsv_irq[] = {
	{BIT(0),  "[DPRX] DPRX_IRQ_ADAPTIVE_SYNC",  NULL},
	{BIT(1),  "[DPRX] DPRX_IRQ_AUX_WR_30",  NULL},
	{BIT(2),  "[DPRX] DPRX_IRQ_AUX_WR_31",  NULL},
	{BIT(3),  "[DPRX] DPRX_IRQ_AUX_WR_32",  NULL},
	{BIT(4),  "[DPRX] DPRX_IRQ_AUX_WR_33", NULL},
	{BIT(5),  "[DPRX] DPRX_IRQ_AUX_WR_34", NULL},
	{BIT(6),  "[DPRX] DPRX_IRQ_AUX_WR_35", NULL},
	{BIT(7),  "[DPRX] DPRX_IRQ_AUX_WR_36", NULL},
	{BIT(8),  "[DPRX] DPRX_IRQ_AUX_WR_37",  NULL},
	{BIT(9),  "[DPRX] DPRX_IRQ_AUX_WR_38",  NULL},
	{BIT(10), "[DPRX] DPRX_IRQ_AUX_WR_39",  NULL},
	{BIT(11), "[DPRX] DPRX_IRQ_AUX_WR_3a",  NULL},
	{BIT(12), "[DPRX] DPRX_IRQ_AUX_WR_3b",  NULL},
	{BIT(13), "[DPRX] DPRX_IRQ_AUX_WR_3c",  NULL},
	{BIT(14), "[DPRX] DPRX_IRQ_AUX_WR_3d",  NULL},
	{BIT(15), "[DPRX] DPRX_IRQ_AUX_WR_3e",  NULL},
	{BIT(16), "[DPRX] DPRX_IRQ_AUX_WR_3f",  NULL},
	{BIT(17), "[DPRX] DPRX_IRQ_AUX_WR_720", NULL},
	{BIT(18), "[DPRX] DPRX_IRQ_AUX_WR_721",  NULL},
	{BIT(19), "[DPRX] DPRX_IRQ_AUX_WR_722",  NULL},
	{BIT(20), "[DPRX] DPRX_IRQ_AUX_WR_723",  NULL},
	{BIT(21), "[DPRX] DPRX_IRQ_AUX_WR_724",  NULL},
	{BIT(22), "[DPRX] DPRX_IRQ_AUX_RE_202",  dprx_print_lane0_1_status},
	{BIT(23), "[DPRX] DPRX_IRQ_AUX_RE_203",  dprx_print_lane2_3_status},
	{BIT(24), "[DPRX] DPRX_IRQ_AUX_WR_20f",  NULL},
	{BIT(25), "[DPRX] DPRX_IRQ_AUX_WR_280",  NULL},
	{BIT(26), "[DPRX] DPRX_IRQ_AUX_WR_200c",  NULL},
	{BIT(27), "[DPRX] DPRX_IRQ_AUX_WR_200d",  NULL},
	{BIT(28), "[DPRX] DPRX_IRQ_AUX_WR_2011",  NULL},
};

void dprx_handle_rsv_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t i;

	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + 0x00B70);
	for (i = 0; i < ARRAY_SIZE(g_rsv_irq); i++) {
		if (irq_sts & g_rsv_irq[i].bit_pos) {
			dprx_pr_info("%s\n", g_rsv_irq[i].description);
			if (g_rsv_irq[i].oprate != NULL)
				g_rsv_irq[i].oprate(dprx);
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB74, g_rsv_irq[i].bit_pos);
		}
	}
}

static void dprx_handle_hpd_int(struct dprx_ctrl *dprx)
{
	uint32_t reg;
	reg = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_HPD_CTRL0);
	reg |= DPRX_IRQ_HPD_REQ;
	dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_HPD_CTRL0, reg);
}

struct dprx_description_handling g_debug_irq[] = {
	{BIT(0),  "[DPRX] DPRX_IRQ_AUX_PHY_WAKE",  NULL},
	{BIT(1),  "[DPRX] DPRX_IRQ_AUX_ACCES_INT",  NULL},
	{BIT(2),  "[DPRX] DPRX_IRQ_AUX_1ST_LINE_INT",  NULL},
	{BIT(3),  "[DPRX] DPRX_IRQ_AUX_HPD_INT",  dprx_handle_hpd_int},
	{BIT(4),  "[DPRX]  DPRX_IRQ_AUX_WR_270", NULL},
};

void dprx_handle_debug_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t i;
	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + 0x00B64);
	for (i = 0; i < ARRAY_SIZE(g_debug_irq); i++) {
		if (irq_sts & g_debug_irq[i].bit_pos) {
			dprx_pr_info("%s\n", g_debug_irq[i].description);
			if (g_debug_irq[i].oprate != NULL)
				g_debug_irq[i].oprate(dprx);
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0x00B68, g_debug_irq[i].bit_pos);
		}
	}
}
void dprx_lane0_1_adjust_swing_or_pre(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	irq_sts = dprx_readl(dprx, 0x204);
	dprx_pr_info("AUX RE 206, dpcd 0x204 0x%x\n", irq_sts);
	dprx_pr_info("lane0: swing %d, pre %d\n", (irq_sts >> 16) & 0x3, (irq_sts >> 18) & 0x3);
	dprx_pr_info("lane1: swing %d, pre %d\n", (irq_sts >> 20) & 0x3, (irq_sts >> 22) & 0x3);
}
void dprx_lane2_3_adjust_swing_or_pre(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	irq_sts = dprx_readl(dprx, 0x204);
	dprx_pr_info("AUX RE 207, dpcd 0x204 0x%x\n", irq_sts);
	dprx_pr_info("lane2: swing %d, pre %d\n", (irq_sts >> 24) & 0x3, (irq_sts >> 26) & 0x3);
	dprx_pr_info("lane3: swing %d, pre %d\n", (irq_sts >> 28) & 0x3, (irq_sts >> 30) & 0x3);
}
struct dprx_description_handling g_mon_irq[] = {
	{BIT(0),  "[DPRX] DPRX_IRQ_AUX_MON0_206",  dprx_lane0_1_adjust_swing_or_pre},
	{BIT(1),  "[DPRX] DPRX_IRQ_AUX_MON1_207",  dprx_lane2_3_adjust_swing_or_pre},
	{BIT(2),  "[DPRX] DPRX_IRQ_AUX_MON2",  NULL},
	{BIT(3),  "[DPRX] DPRX_IRQ_AUX_MON3",  NULL},
	{BIT(4),  "[DPRX] DPRX_IRQ_AUX_MON4", NULL},
	{BIT(5),  "[DPRX] DPRX_IRQ_AUX_MON5", NULL},
	{BIT(6),  "[DPRX] DPRX_IRQ_AUX_MON6", NULL},
	{BIT(7),  "[DPRX] DPRX_IRQ_AUX_MON7", NULL},
	{BIT(8),  "[DPRX] DPRX_IRQ_AUX_MON8",  NULL},
	{BIT(9),  "[DPRX] DPRX_IRQ_AUX_MON9",  NULL},
	{BIT(10), "[DPRX] DPRX_IRQ_AUX_MON10",  NULL},
	{BIT(11), "[DPRX] DPRX_IRQ_AUX_MON11",  NULL},
	{BIT(12), "[DPRX] DPRX_IRQ_AUX_MON12",  NULL},
	{BIT(13), "[DPRX] DPRX_IRQ_AUX_MON13",  NULL},
	{BIT(14), "[DPRX] DPRX_IRQ_AUX_MON14",  NULL},
	{BIT(15), "[DPRX] DPRX_IRQ_AUX_MON15",  NULL},
	{BIT(16), "[DPRX] DPRX_IRQ_AUX_MON16",  NULL},
	{BIT(17), "[DPRX] DPRX_IRQ_AUX_MON17", NULL},
	{BIT(18), "[DPRX] DPRX_IRQ_AUX_MON18",  NULL},
	{BIT(19), "[DPRX] DPRX_IRQ_AUX_MON19",  NULL},
	{BIT(20), "[DPRX] DPRX_IRQ_AUX_MON20",  NULL},
	{BIT(21), "[DPRX] DPRX_IRQ_AUX_MON21",  NULL},
	{BIT(22), "[DPRX] DPRX_IRQ_AUX_MON22",  NULL},
	{BIT(23), "[DPRX] DPRX_IRQ_AUX_MON23",  NULL},
};

void dprx_handle_aux_access_int_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	uint32_t i;
	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + 0x00B7C);
	for (i = 0; i < ARRAY_SIZE(g_mon_irq); i++) {
		if (irq_sts & g_mon_irq[i].bit_pos) {
			dprx_pr_info("%s\n", g_mon_irq[i].description);
			if (g_mon_irq[i].oprate != NULL)
				g_mon_irq[i].oprate(dprx);
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0x00B80, g_mon_irq[i].bit_pos);
		}
	}
}

void dprx_handle_err_irq(struct dprx_ctrl *dprx)
{
	uint32_t irq_sts;
	irq_sts = dprx_readl(dprx, DPRX_SCTRL_OFFSET + 0x00B58);
	if (irq_sts & BIT(0))
		dprx_pr_info("[DPRX] DPRX_IRQ_DDC_TIMEOUT \n");

	if (irq_sts & BIT(1))
		dprx_pr_info("[DPRX] DPRX_IRQ_CHUNK_LEN_ERR \n");

	dprx_writel(dprx, DPRX_SCTRL_OFFSET + 0xB5C, irq_sts);
}

struct dprx_description_handling g_irq_handle[] = {
	{TIMING_INTR_ENABLE,  "[DPRX] TIMING_IRQ",  dprx_handle_timing_irq},
	{RSV_INTR_ENABLE,  "[DPRX]  RSV_IRQ", dprx_handle_rsv_irq},
	{AUX_RSV_INTR_ENABLE,  "[DPRX] AUX_RSV_IRQ", dprx_handle_aux_access_int_irq},
	{LINK_CFG_INTR_ENABLE,  "[DPRX] LINK_CFG_IRQ",  dprx_handle_linkcfg_irq},
	{TRAINING_INTR_ENABLE,  "[DPRX] TRAINING_IRQ",  dprx_handle_training_irq},
	{DEBUG_INTR_ENABLE,  "[DPRX] DEBUG_IRQ",  dprx_handle_debug_irq},
	{PSR_INTR_ENABLE,  "[DPRX] PSR_IRQ", dprx_handle_psr_irq},
	{SDP_VIDEO_FORMAT_INTR_ENABLE,  "[DPRX] SDP_VIDEO_FORMAT_IRQ",  dprx_handle_sdp_and_format_irq},
	{AUDIO_INTR_ENABLE,  "[DPRX] AUDIO_IRQ",  NULL},
	{ERR_INTR_ENABLE,  "[DPRX] ERR_IRQ",  dprx_handle_err_irq},
	{GTC_INTR_ENABLE,  "[DPRX] GTC_IRQ",  dprx_handle_gtc_irg},
};

irqreturn_t dprx_irq_handler(int irq, void *dev)
{
	struct dpu_dprx_device *dpu_dprx_dev = NULL;
	struct dprx_ctrl *dprx = NULL;
	uint32_t irq_status;
	uint32_t i;

	dpu_check_and_return(!dev, IRQ_HANDLED, err, "[DPRX] dev is NULL\n");
	dpu_dprx_dev = (struct dpu_dprx_device *)dev;
	dpu_check_and_return((dpu_dprx_dev == NULL), IRQ_HANDLED, err, "[DPRX] dpu_dprx_dev is NULL!\n");

	dprx = &dpu_dprx_dev->dprx;

	irq_status = dprx_readl(dprx, DPRX_SCTRL_OFFSET + DPRX_INTR_MASKED_STATUS);
	for (i = 0 ; i < ARRAY_SIZE(g_irq_handle); i++) {
		if (irq_status & g_irq_handle[i].bit_pos) {
			dprx_pr_debug("%s\n", g_irq_handle[i].description);
			if (g_irq_handle[i].oprate != NULL)
				g_irq_handle[i].oprate(dprx);
			dprx_writel(dprx, DPRX_SCTRL_OFFSET + DPRX_INTR_ORI_STATUS, g_irq_handle[i].bit_pos);
		}
	}
	return IRQ_HANDLED;
}

uint32_t  dprx_get_color_space(struct dprx_ctrl *dprx)
{
	dprx_pr_info("ColorSpace: %s\n", enc_info_list[dprx->v_params.pix_enc].desc);
	return enc_info_list[dprx->v_params.pix_enc].pix_enc;
}

static struct dprx_to_layer_format g_dp2layer_fmt[] = {
	{RGB,      COLOR_DEPTH_6,  HISI_FB_PIXEL_FORMAT_RGBA_8888},
	{RGB,      COLOR_DEPTH_8,  HISI_FB_PIXEL_FORMAT_RGBA_8888},
	{RGB,      COLOR_DEPTH_10, HISI_FB_PIXEL_FORMAT_RGBA_1010102},
	{YCBCR420, COLOR_DEPTH_8,  HISI_FB_PIXEL_FORMAT_YUYV_420_PKG},
	{YCBCR420, COLOR_DEPTH_10, HISI_FB_PIXEL_FORMAT_YUYV_420_PKG_10BIT},
	{YCBCR422, COLOR_DEPTH_8,  HISI_FB_PIXEL_FORMAT_YUYV_422_PKG},
	{YCBCR422, COLOR_DEPTH_10, HISI_FB_PIXEL_FORMAT_YUV422_10BIT},
	{YCBCR444, COLOR_DEPTH_8,  HISI_FB_PIXEL_FORMAT_YUV444},
	{YCBCR444, COLOR_DEPTH_10, HISI_FB_PIXEL_FORMAT_YUVA444},
};

uint32_t dprx_ctrl_dp2layer_fmt(uint32_t color_space, uint32_t bit_width)
{
	int i;
	int size;
	struct dprx_to_layer_format *p_dp2layer_fmt;

	size = sizeof(g_dp2layer_fmt) / sizeof(g_dp2layer_fmt[0]);
	p_dp2layer_fmt = g_dp2layer_fmt;

	for (i = 0; i < size; i++) {
		if ((p_dp2layer_fmt[i].color_space == color_space) &&
			(p_dp2layer_fmt[i].bit_width == bit_width))
			return p_dp2layer_fmt[i].layer_format;
	}
	dprx_pr_info("no find fmt, return default rgba8888\n");
	return HISI_FB_PIXEL_FORMAT_RGBA_8888;
}
