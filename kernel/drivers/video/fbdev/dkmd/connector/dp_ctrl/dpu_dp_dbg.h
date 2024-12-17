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

#ifndef __DPU_DP_DBG_H__
#define __DPU_DP_DBG_H__

#include <linux/kernel.h>
#include <linux/types.h>
#include <huawei_platform/dp_source/dp_source_switch.h>
#include <huawei_platform/dp_source/dp_debug.h>

/*******************************************************************************
 * DP GRAPHIC DEBUG TOOL
 */
struct dp_ctrl;
void dp_graphic_debug_node_init(struct dp_ctrl *dptx);
void dptx_debug_set_params(struct dp_ctrl *dptx);
extern int g_dp_debug_mode_enable;
extern int g_dp_vactive_pixels_debug;
extern int g_dp_hactive_pixels_debug;
extern uint g_dp_aux_ctrl;
extern uint g_pll1ckgctrlr0;
extern uint g_pll1ckgctrlr1;
extern uint g_pll1ckgctrlr2;
extern uint32_t g_hbr2_pll1_sscg[4];
extern uint32_t g_rbr_pll1_rate0_coef[3];
extern uint32_t g_hbr_pll1_rate1_coef[3];
extern uint32_t g_hbr2_pll1_rate2_coef[3];
extern uint32_t g_hbr3_pll1_rate3_coef[3];
extern int g_dp_same_source_debug;
extern int g_clk_hblank_code_enable;
extern uint32_t g_pixel_clock;
extern uint32_t g_video_code;
extern uint32_t g_hblank;

/* v740 use */
extern uint g_pll1ckgctrlr3;

uint32_t dptx_debug_get_combophy_rate_param(struct dp_ctrl *dptx, int dptx_phyif_ctrl_rate,
	uint32_t *ckgctrl, uint32_t *sscginital, uint32_t *sscgcnt, uint32_t *sscgcnt2);
bool dp_debug_get_clk_hblank_enable(struct dp_ctrl *dptx);
void dp_debug_get_clk_hblank_params(struct dp_ctrl *dptx, uint64_t *pixel_clock,
	uint32_t *video_code, uint16_t *hblank);
uint dp_debug_get_aux_ctrl_param(struct dp_ctrl *dptx);
uint32_t dp_debug_get_pll_value(void);
void dptx_debug_get_double_tx_sync_para(struct dp_ctrl *dptx);

int dpu_dp_get_current_dp_source_mode(void);
#endif /* __DPU_DP_DBG_H__ */
