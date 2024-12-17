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

#ifndef __HDMITX_DBG_H__
#define __HDMITX_DBG_H__

#include <linux/kernel.h>
#include <linux/types.h>

extern int g_hdmitx_colorbar_enable_debug;
extern int g_hdmitx_hdmi_dvi_mode_debug;
extern int g_hdmitx_display_mode_vic_debug;
extern int g_hdmitx_allm_debug;
extern int g_hdmitx_all_vsif_enable_debug;
extern int g_hdmitx_avi_enable_debug;
extern int g_hdmitx_gcp_enable_debug;
extern int g_hdmitx_gmp_enable_debug;
extern int g_hdmitx_static_hdr_enable_debug;
extern int g_hdmitx_vrr_emp_enable_debug;
extern int g_hdmitx_err_cnt_enable_debug;
extern int g_hdmitx_plug_retry_cnt_debug;
extern int g_hdmitx_frl_tmds_mode_debug;
extern int g_frl_xg_xl_max_debug;
extern int g_frl_xg_xl_min_debug;
extern int g_hdmitx_color_depth_debug;
extern int g_frl_ffe_level_debug;
extern int g_frl_ffe_debug;
extern bool g_frl_dphy_spec_en[3];
extern uint32_t g_frl_aphy_spec_params[9];
extern uint32_t g_frl_dphy_ffe0[4];
extern uint32_t g_frl_dphy_ffe1[4];
extern uint32_t g_frl_dphy_ffe2[4];
extern uint32_t g_frl_dphy_ffe3[4];
extern int g_frl_aphy_cs_0123_debug;
extern uint32_t g_frl_aphy_cs_0123_offset[12];
extern int g_aphy_cs_8_debug;
extern uint32_t g_aphy_cs_8_offset[15];
extern int g_tmds_aphy_cs_012_debug;
extern uint32_t g_tmds_aphy_cs_012_offset[12];
extern int g_tmds_aphy_cs_3_debug;
extern uint32_t g_tmds_aphy_cs_3_offset[12];
extern int g_tmds_data_ffe_debug;
extern bool g_tmds_data_dphy_en[3];
extern uint32_t g_tmds_data_dphy_ffe[4];
extern int g_tmds_clk_ffe_debug;
extern bool g_tmds_clk_dphy_en[3];
extern uint32_t g_tmds_clk_dphy_ffe[4];

#endif

