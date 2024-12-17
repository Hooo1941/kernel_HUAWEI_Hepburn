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

#ifndef DKMD_MIPI_PANEL_INFO_H
#define DKMD_MIPI_PANEL_INFO_H

#include <linux/atomic.h>
#include <linux/types.h>

enum mipi_frame_rate_mode_type {
	MIPI_FRM_RATE_30,
	MIPI_FRM_RATE_60,
	MIPI_FRM_RATE_90,
	MIPI_FRM_RATE_120,
	MIPI_FRM_RATE_144,
	MIPI_FRM_RATE_45,
	MIPI_FRM_RATE_48,
	MIPI_FRM_RATE_72,
	MIPI_FRM_RATE_NUM_MAX
};

enum {
	DSI_16BITS_1 = 0,
	DSI_16BITS_2,
	DSI_16BITS_3,
	DSI_18BITS_1,
	DSI_18BITS_2,
	DSI_24BITS_1,
	DSI_24BITS_2,
	DSI_24BITS_3,
	DSI_DSC24_COMPRESSED_DATA = 0xF,
};

enum ENUM_LDI_VSTATE {
	LDI_VSTATE_IDLE = 0x1,
	LDI_VSTATE_VSW = 0x2,
	LDI_VSTATE_VBP = 0x4,
	LDI_VSTATE_VACTIVE0 = 0x8,
	LDI_VSTATE_VACTIVE_SPACE = 0x10,
	LDI_VSTATE_VACTIVE1 = 0x20,
	LDI_VSTATE_VFP = 0x40,
	LDI_VSTATE_V_WAIT_TE0 = 0x80,
	LDI_VSTATE_V_WAIT_TE1 = 0x100,
	LDI_VSTATE_V_WAIT_TE_EN = 0x200,
	LDI_VSTATE_V_WAIT_GPU = 0x400,
};

struct mipi_timing_detail {
	uint32_t hsa;
	uint32_t hbp;
	uint32_t dpi_hsize;
	uint32_t hline_time;

	uint32_t vsa;
	uint32_t vbp;
	uint32_t vfp;
};

struct mipi_timing_mode {
	uint32_t frame_rate;
	uint32_t dsi_bit_clk_default;
	uint32_t dsi_bit_clk_val1;
	uint32_t dsi_bit_clk_val2;
	uint32_t dsi_bit_clk_val3;
	uint32_t dsi_bit_clk_val4;
	uint32_t dsi_bit_clk_val5;
	struct mipi_timing_detail mipi_timing;
};

enum {
	DSI_1_1_VERSION = 0,
	DSI_1_2_VERSION,
};

enum {
	DSI_NON_BURST_SYNC_PULSES = 0,
	DSI_NON_BURST_SYNC_EVENTS,
	DSI_BURST_SYNC_PULSES_1,
	DSI_BURST_SYNC_PULSES_2,
};

enum {
	DSI_1_LANES = 0,
	DSI_2_LANES,
	DSI_3_LANES,
	DSI_4_LANES,
};

enum lcd_orientation {
	LCD_LANDSCAPE = 0,
	LCD_PORTRAIT,
};

enum lcd_format {
	LCD_RGB888 = 0,
	LCD_RGB101010,
	LCD_RGB565,
};

enum lcd_rgb_order {
	LCD_RGB = 0,
	LCD_BGR,
};

struct mipi_panel_info {
	uint8_t dsi_version;
	uint8_t vc;
	uint8_t lane_nums;
	uint8_t lane_nums_select_support;
	uint8_t color_mode;
	uint8_t non_continue_en;
	uint8_t txoff_rxulps_en;
	uint32_t dsi_bit_clk; /* clock lane(p/n) */
	uint32_t dsi_bit_clk_default;
	uint32_t burst_mode;
	uint32_t max_tx_esc_clk;
	int32_t frame_rate;
	int32_t take_effect_delayed_frm_cnt;

	uint32_t hsa;
	uint32_t hbp;
	uint32_t dpi_hsize;
	uint32_t width;
	uint32_t hline_time;
	uint32_t need_convert_pixel2cycle;

	uint32_t vsa;
	uint32_t vbp;
	uint32_t vactive_line;
	uint32_t vfp;

	uint32_t dsi_bit_clk_upt_support;
	uint32_t dsi_bit_clk_val1;
	uint32_t dsi_bit_clk_val2;
	uint32_t dsi_bit_clk_val3;
	uint32_t dsi_bit_clk_val4;
	uint32_t dsi_bit_clk_val5;
	uint32_t dsi_bit_clk_upt;

	uint32_t mipi_frm_rate_mode_num;
	struct mipi_timing_mode mipi_timing_modes[MIPI_FRM_RATE_NUM_MAX];

	atomic_t dsi_bit_clk_upt_flag;
	uint32_t hs_wr_to_time;

	/* dphy config parameter adjust */
	uint32_t clk_post_adjust;
	uint32_t clk_pre_adjust;
	uint32_t clk_pre_delay_adjust;
	int32_t clk_t_hs_exit_adjust;
	int32_t clk_t_hs_trial_adjust;
	uint32_t clk_t_hs_prepare_adjust;
	int32_t clk_t_lpx_adjust;
	uint32_t clk_t_hs_zero_adjust;
	uint32_t data_post_delay_adjust;
	int32_t data_t_lpx_adjust;
	uint32_t data_t_hs_prepare_adjust;
	uint32_t data_t_hs_zero_adjust;
	int32_t data_t_hs_trial_adjust;
	uint32_t rg_vrefsel_vcm_adjust;
	uint32_t rg_vrefsel_eq_adjust;
	uint32_t support_de_emphasis;
	uint32_t rg_vrefsel_lptx_adjust;
	uint32_t rg_lptx_sri_adjust;
	int32_t data_lane_lp2hs_time_adjust;

	uint32_t phy_mode;  /* 0: DPHY, 1:CPHY */
	uint32_t lp11_flag; /* 0: nomal_lp11, 1:short_lp11, 2:disable_lp11 */
	uint32_t phy_m_n_count_update;  /* 0:old ,1:new can get 988.8M */
	uint32_t eotp_disable_flag; /* 0: eotp enable, 1:eotp disable */

	uint8_t mininum_phy_timing_flag; /* 1:support entering lp11 with minimum clock */

	uint32_t dynamic_dsc_en; /* used for dfr */

	uint64_t pxl_clk_rate;
	uint32_t pxl_clk_rate_div;
};

struct dsi_cmd_desc {
	uint32_t dtype;
	uint32_t vc;
	uint32_t wait;
	uint32_t waittype;
	uint32_t dlen;
	char *payload;
};
#endif
