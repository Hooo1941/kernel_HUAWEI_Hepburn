/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#ifndef __DP_CTRL_DEFAULT_CONFIG_H__
#define __DP_CTRL_DEFAULT_CONFIG_H__
#include "dp_ctrl.h"
#include "dp_drv.h"

#define DPTX_PHYIF_CTRL_MAX_RATE 0x2

#define DEFAULT_MIDIA_PPLL7_CLOCK_FREQ 1290000000UL
#define MIDIA_PPLL7_CTRL0 0x838
#define MIDIA_PPLL7_CTRL1 0x83c

#define DPTX_CFG_DP_ENABLE_BIT BIT(5)
#define DPTX_GTC_RST_N 0

#define MAX_HACTIVE_PIXELS 3840
#define MAX_VACTIVE_PIXELS 2160

#define PREFERRED_MAX_FPS_OF_MONITOR 60

#define DPTX_MAX_TU_OF_MONITOR 65

#define FHD_TIMING_H_ACTIVE 1920
#define FHD_TIMING_V_ACTIVE 1080

/* dtsi reg base */
enum {
	REG_HIDPTX_BASE                 = 0,
	REG_HSDT1_CRG_BASE              = 1,
	REG_COMBOPHY_BASE               = 2,
	REG_DP_SCTRL_BASE               = 3,
	REG_HIDPTX_EXTERNAL_BASE        = 4,
	REG_COMBOPHY_EXTERNAL_BASE      = 5,
};

enum {
	DPTX0_PIXEL_CLOCK               = 0,
	DPTX1_PIXEL_CLOCK               = 1,
};

void dp_ctrl_config_init(struct dp_ctrl *dptx);
int32_t init_dp_sctrl(struct dp_private *priv, struct device_node *np);
int32_t init_hsdt1_sysctrl_base(struct dp_private *priv, struct device_node *np);
int32_t init_sub_harden_crg_base(struct dp_private *priv, struct device_node *np);
int32_t init_dpctrl_pixel_clk(struct dp_ctrl *dptx0, struct dp_ctrl *dptx1, struct device_node *np);
int32_t init_dpctrl_16m_clk(struct dp_ctrl *dptx, struct device_node *np);
int dp_pm_suspend(struct device *dev);
int dptx_pixel_pll_preinit(void);
int dptx_pixel_pll_init(struct dp_ctrl *dptx, uint64_t pixel_clock);
int dptx_pixel_pll_deinit(struct dp_ctrl *dptx);
void enable_dptx_timing_gen(struct dp_ctrl *dptx);
bool dp_ctrl_is_bypass_by_pg(uint32_t dp_id);

#endif