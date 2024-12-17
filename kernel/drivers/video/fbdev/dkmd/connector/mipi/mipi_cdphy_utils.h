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

#ifndef __MIPI_CDPHY_H__
#define __MIPI_CDPHY_H__

#include <linux/types.h>

#define DEFAULT_MAX_TX_ESC_CLK (10 * 1000000UL)
#define DEFAULT_MIPI_CLK_RATE  (192 * 100000L)
#define DEFAULT_MIPI_REF_CLK_RATE  (384 * 100000L)

#define VFP_TIME_MASK 0x7fff
#define VFP_TIME_OFFSET 10
#define VFP_DEF_TIME 80
#define MILLION_CONVERT 1000000
#define PCTRL_TRY_TIME 10
#define DSI_CLK_BW 1
#define DSI_CLK_BS 0

#define VCO_CLK_MIN_VALUE 2000

#define MIN_T3_PREPARE_PARAM 450
#define MIN_T3_LPX_PARAM 500
#define MIN_T3_PREBEGIN_PARAM 21
#define MIN_T3_POST_PARAM 77
#define MIN_T3_PREBEGIN_PHY_TIMING 0x21
#define MIN_T3_POST_PHY_TIMING 0xf

#define T3_PREPARE_PARAM 650
#define T3_LPX_PARAM 600
#define T3_PREBEGIN_PARAM 350
#define T3_POST_PARAM 224
#define T3_PREBEGIN_PHY_TIMING 0x3e
#define T3_POST_PHY_TIMING 0x1e

#define phy_reduce(x) ((x) > 0 ? ((x) - 1) : (x))

struct dpu_connector;
struct mipi_panel_info;
struct mipi_dsi_timing;
struct dpu_panel_info;

enum mipi_dsi_phy_mode {
	DPHY_MODE = 0,
	CPHY_MODE,
};

/* DSI PHY configuration */
struct mipi_dsi_phy_ctrl {
	uint64_t lane_byte_clk;
	uint64_t lane_word_clk;
	uint64_t lane_byte_clk_default;
	uint32_t clk_division;

	uint32_t clk_lane_lp2hs_time;
	uint32_t clk_lane_hs2lp_time;
	uint32_t data_lane_lp2hs_time;
	uint32_t data_lane_hs2lp_time;
	uint32_t clk2data_delay;
	uint32_t data2clk_delay;

	uint32_t clk_pre_delay;
	uint32_t clk_post_delay;
	uint32_t clk_t_lpx;
	uint32_t clk_t_hs_prepare;
	uint32_t clk_t_hs_zero;
	uint32_t clk_t_hs_trial;
	uint32_t clk_t_wakeup;
	uint32_t data_pre_delay;
	uint32_t data_post_delay;
	uint32_t data_t_lpx;
	uint32_t data_t_hs_prepare;
	uint32_t data_t_hs_zero;
	uint32_t data_t_hs_trial;
	uint32_t data_t_ta_go;
	uint32_t data_t_ta_get;
	uint32_t data_t_wakeup;

	uint32_t phy_stop_wait_time;

	uint32_t rg_lptx_sri;
	uint32_t rg_vrefsel_lptx;
	uint32_t rg_vrefsel_vcm;
	uint32_t rg_vrefsel_eq;
	uint32_t rg_hstx_ckg_sel;
	uint32_t rg_pll_fbd_div5f;
	uint32_t rg_pll_fbd_div1f;
	uint32_t rg_pll_fbd_2p;
	uint32_t rg_pll_enbwt;
	uint32_t rg_pll_fbd_p;
	uint32_t rg_pll_fbd_s;
	uint32_t rg_pll_pre_div1p;
	uint32_t rg_pll_pre_p;
	uint32_t rg_pll_vco_750m;
	uint32_t rg_pll_lpf_rs;
	uint32_t rg_pll_lpf_cs;
	uint32_t rg_pll_enswc;
	uint32_t rg_pll_chp;

	uint32_t rg_band_sel;
	uint32_t rg_cphy_div; // Q
	uint32_t rg_div; // M 0x4A[7:0]
	uint32_t rg_pre_div; // N 0x49[0]
	uint32_t rg_320m; // 0x48[2]
	uint32_t rg_2p5g; // 0x48[1]
	uint32_t rg_0p8v; // 0x48[0]
	uint32_t rg_lpf_r; // 0x46[5:4]
	uint32_t rg_cp; // 0x46[3:0]
	uint32_t rg_pll_fbkdiv;
	uint32_t rg_pll_prediv;
	uint32_t rg_pll_posdiv;
	uint32_t t_prepare;
	uint32_t t_lpx;
	uint32_t t_prebegin;
	uint32_t t_post;
};

void get_dsi_cphy_ctrl(struct dpu_connector* connector, struct mipi_dsi_phy_ctrl* phy_ctrl);
void get_dsi_dphy_ctrl(struct dpu_connector* connector, struct mipi_dsi_phy_ctrl* phy_ctrl);
bool mipi_phy_status_check(const char __iomem* mipi_dsi_base, uint32_t expected_value);
void mipi_config_phy_test_code(char __iomem* mipi_dsi_base, uint32_t test_code_addr, uint32_t test_code_parameter);
void mipi_config_cphy_spec1v0_parameter(const struct mipi_dsi_phy_ctrl* phy_ctrl,
	struct mipi_panel_info* mipi, char __iomem* mipi_dsi_base, int32_t fpga_flag);
void mipi_config_dphy_spec1v2_parameter(const struct mipi_dsi_phy_ctrl* phy_ctrl,
	struct mipi_panel_info* mipi, char __iomem* mipi_dsi_base);
void mipi_cdphy_init_config(struct dpu_connector *connector,
	struct mipi_dsi_phy_ctrl *phy_ctrl, char __iomem *mipi_dsi_base);
void mipi_dsi_phy_timing_config(struct mipi_dsi_phy_ctrl *phy_ctrl, char __iomem *mipi_dsi_base);
void get_mipi_dsi_timing_config_para(struct mipi_panel_info* mipi,
	struct mipi_dsi_phy_ctrl* phy_ctrl, struct mipi_dsi_timing* timing);
void mipi_dsi_reset_init(struct dpu_connector* connector);
void mipi_dsi_reset_deinit(struct dpu_connector* connector);
void mipi_dsi_convert_pxl2cycle(struct dpu_panel_info *pinfo);
#endif