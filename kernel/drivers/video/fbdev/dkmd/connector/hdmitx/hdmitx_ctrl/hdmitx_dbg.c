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
#include <linux/types.h>
#include "hdmitx_dbg.h"
#include "hdmitx_ctrl.h"
#include "hdmitx_ddc.h"
#include "dkmd_log.h"
#include "hdmitx_phy_config.h"
#include "hdmitx_ctrl_dev.h"
#include "hdmitx_avgen.h"

/*
 * for debug
 * /sys/module/hdmitx_ctrl/parameters
 */

int g_hdmitx_colorbar_enable_debug = 1; // 0: disable  1: enable
int g_hdmitx_hdmi_dvi_mode_debug = 1; // 0: dvi  1: hdmi
int g_hdmitx_display_mode_vic_debug = 16; // 0: disable debug 4: 1280x720p@60hz 16:1920x1080p@60hz
int g_hdmitx_avi_enable_debug = 1;
int g_hdmitx_all_vsif_enable_debug = 0;
int g_hdmitx_allm_debug = 0;
int g_hdmitx_gcp_enable_debug = 0;
int g_hdmitx_gmp_enable_debug = 0;
int g_hdmitx_static_hdr_enable_debug = 0;
int g_hdmitx_vrr_emp_enable_debug = 0;
int g_hdmitx_err_cnt_enable_debug = 0;
int g_hdmitx_plug_retry_cnt_debug = 1;
int g_hdmitx_frl_tmds_mode_debug = 1; // 0: TMDS 1:FRL
int g_frl_xg_xl_max_debug = 1; // 1:debug 3G3L, 6G3L,6G4L,8G4L,10G4L;
int g_frl_xg_xl_min_debug = 1; // 1:debug 3G3L, 6G3L,6G4L,8G4L,10G4L;
int g_hdmitx_color_depth_debug = 0; // 0:HDMITX_BPC_24  1:HDMITX_BPC_30  2:HDMITX_BPC_36 -- not support
int g_frl_ffe_level_debug = 3; // 1:debug ffe level
int g_frl_ffe_debug = 0; // 1:debug ffe
bool g_frl_dphy_spec_en[3] = {false, true, true};  // post2_en, post1_en, pre_en
int g_frl_dphy_spec_en_len = 0;
uint g_frl_dphy_ffe0[4] = {0x00, 0x19, 0x20, 0x11};  // ffe0 post2, post1, main, pre
int g_frl_dphy_ffe0_len = 0;
uint g_frl_dphy_ffe1[4] = {0x00, 0x19, 0x20, 0x11};  // ffe1 post2, post1, main, pre
int g_frl_dphy_ffe1_len = 0;
uint g_frl_dphy_ffe2[4] = {0x00, 0x19, 0x20, 0x11};  // ffe2 post2, post1, main, pre
int g_frl_dphy_ffe2_len = 0;
uint g_frl_dphy_ffe3[4] = {0x00, 0x19, 0x20, 0x11};  // ffe3 post2, post1, main, pre
int g_frl_dphy_ffe3_len = 0;
int g_frl_aphy_cs_0123_debug = 0; // 1:debug mode
// offset 0~B
uint g_frl_aphy_cs_0123_offset[12] = {0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55};
int g_frl_aphy_cs_0123_offset_len = 0;
int g_aphy_cs_8_debug = 0; // 1:debug mode
// offset 0~E
uint g_aphy_cs_8_offset[15] = {0xff, 0x31, 0x00, 0x1, 0x81, 0x55, 0x12, 0x09, 0xA4, 0x80, 0x5, 0xE0, 0x0C, 0xFF, 0x11};
int g_aphy_cs_8_offset_len = 0;
int g_tmds_aphy_cs_012_debug = 0; // 1:debug mode
// offset 0~B
uint g_tmds_aphy_cs_012_offset[12] = {0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55};
int g_tmds_aphy_cs_012_offset_len = 0;
int g_tmds_aphy_cs_3_debug = 0; // 1:debug mode
// offset 0~B
uint g_tmds_aphy_cs_3_offset[12] = {0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55};
int g_tmds_aphy_cs_3_offset_len = 0;
int g_tmds_data_ffe_debug = 0; // 1:debug ffe
bool g_tmds_data_dphy_en[3] = {false, false, false};  // post2_en, post1_en, pre_en
int g_tmds_data_dphy_en_len = 0;
uint g_tmds_data_dphy_ffe[4] = {0x00, 0x00, 0x14, 0x00};  // ffe0 post2, post1, main, pre
int g_tmds_data_dphy_ffe_len = 0;
int g_tmds_clk_ffe_debug = 0; // 1:debug ffe
bool g_tmds_clk_dphy_en[3] = {false, false, false};  // post2_en, post1_en, pre_en
int g_tmds_clk_dphy_en_len = 0;
uint g_tmds_clk_dphy_ffe[4] = {0x00, 0x00, 0x14, 0x00};  // ffe0 post2, post1, main, pre
int g_tmds_clk_dphy_ffe_len = 0;

#ifdef CONFIG_DKMD_DEBUG_ENABLE
module_param_named(g_hdmitx_colorbar_enable, g_hdmitx_colorbar_enable_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_colorbar_enable, "g_hdmitx_colorbar_enable");

module_param_named(g_hdmitx_hdmi_dvi_mode, g_hdmitx_hdmi_dvi_mode_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_hdmi_dvi_mode, "g_hdmitx_hdmi_dvi_mode");

module_param_named(g_hdmitx_display_mode_vic, g_hdmitx_display_mode_vic_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_display_mode_vic, "g_hdmitx_display_mode_vic");

module_param_named(g_hdmitx_avi_enable, g_hdmitx_avi_enable_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_avi_enable, "g_hdmitx_avi_enable");

module_param_named(g_hdmitx_all_vsif_enable, g_hdmitx_all_vsif_enable_debug, int, 0640);
MODULE_PARM_DESC( g_hdmitx_all_vsif, "g_hdmitx_all_vsif");

module_param_named(g_hdmitx_allm, g_hdmitx_allm_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_allm, "g_hdmitx_allm");

module_param_named(g_hdmitx_gcp_enable, g_hdmitx_gcp_enable_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_gcp_enable, "g_hdmitx_gcp_enable");

module_param_named(g_hdmitx_gmp_enable, g_hdmitx_gmp_enable_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_gmp_enable, "g_hdmitx_gmp_enable");

module_param_named(g_hdmitx_static_hdr_enable, g_hdmitx_static_hdr_enable_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_static_hdr_enable, "g_hdmitx_static_hdr_enable");

module_param_named(g_hdmitx_vrr_emp_enable, g_hdmitx_vrr_emp_enable_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_vrr_emp_enable, "g_hdmitx_vrr_emp_enable");

module_param_named(g_hdmitx_err_cnt_enable, g_hdmitx_err_cnt_enable_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_err_cnt_enable, "g_hdmitx_err_cnt_enable");

module_param_named(g_hdmitx_plug_retry_cnt, g_hdmitx_plug_retry_cnt_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_plug_retry_cnt, "g_hdmitx_plug_retry_cnt");

module_param_named(g_hdmitx_frl_tmds_mode, g_hdmitx_frl_tmds_mode_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_frl_tmds_mode, "g_hdmitx_frl_tmds_mode");

module_param_named(g_frl_xg_xl_max, g_frl_xg_xl_max_debug, int, 0640);
MODULE_PARM_DESC(g_frl_xg_xl_max, "g_frl_xg_xl_max");

module_param_named(g_frl_xg_xl_min, g_frl_xg_xl_min_debug, int, 0640);
MODULE_PARM_DESC(g_frl_xg_xl_min, "g_frl_xg_xl_min");

module_param_named(g_hdmitx_color_depth, g_hdmitx_color_depth_debug, int, 0640);
MODULE_PARM_DESC(g_hdmitx_color_depth, "g_hdmitx_color_depth");

module_param_named(g_frl_ffe_level, g_frl_ffe_level_debug, int, 0640);
MODULE_PARM_DESC(g_frl_ffe_level, "g_frl_ffe_level");

module_param_named(g_frl_ffe, g_frl_ffe_debug, int, 0640);
MODULE_PARM_DESC(g_frl_ffe, "g_frl_ffe");

module_param_array_named(frl_dphy_spec_en, g_frl_dphy_spec_en, bool, &g_frl_dphy_spec_en_len, 0640);
MODULE_PARM_DESC(frl_dphy_spec_en, "frl_dphy_spec_en");

module_param_array_named(frl_dphy_ffe0, g_frl_dphy_ffe0, uint, &g_frl_dphy_ffe0_len, 0640);
MODULE_PARM_DESC(frl_dphy_ffe0, "frl_dphy_ffe0");

module_param_array_named(frl_dphy_ffe1, g_frl_dphy_ffe1, uint, &g_frl_dphy_ffe1_len, 0640);
MODULE_PARM_DESC(frl_dphy_ffe1, "frl_dphy_ffe1");

module_param_array_named(frl_dphy_ffe2, g_frl_dphy_ffe2, uint, &g_frl_dphy_ffe2_len, 0640);
MODULE_PARM_DESC(frl_dphy_ffe2, "frl_dphy_ffe2");

module_param_array_named(frl_dphy_ffe3, g_frl_dphy_ffe3, uint, &g_frl_dphy_ffe3_len, 0640);
MODULE_PARM_DESC(frl_dphy_ffe3, "frl_dphy_ffe3");

module_param_named(g_frl_aphy_cs_0123, g_frl_aphy_cs_0123_debug, int, 0640);
MODULE_PARM_DESC(g_frl_aphy_cs_0123, "g_frl_aphy_cs_0123");

module_param_array_named(frl_aphy_cs_0123_offset, g_frl_aphy_cs_0123_offset, uint,
						&g_frl_aphy_cs_0123_offset_len, 0640);
MODULE_PARM_DESC(frl_aphy_cs_0123_offset, "frl_aphy_cs_0123_offset");

module_param_named(g_aphy_cs_8, g_aphy_cs_8_debug, int, 0640);
MODULE_PARM_DESC(g_aphy_cs_8, "g_aphy_cs_8");

module_param_array_named(aphy_cs_8_offset, g_aphy_cs_8_offset, uint, &g_aphy_cs_8_offset_len, 0640);
MODULE_PARM_DESC(aphy_cs_8_offset, "aphy_cs_8_offset");

module_param_named(g_tmds_aphy_cs_012, g_tmds_aphy_cs_012_debug, int, 0640);
MODULE_PARM_DESC(g_tmds_aphy_cs_012, "g_tmds_aphy_cs_012");

module_param_array_named(tmds_aphy_cs_012_offset, g_tmds_aphy_cs_012_offset, uint,
						&g_tmds_aphy_cs_012_offset_len, 0640);
MODULE_PARM_DESC(tmds_aphy_cs_012_offset, "tmds_aphy_cs_012_offset");

module_param_named(g_tmds_aphy_cs_3, g_tmds_aphy_cs_3_debug, int, 0640);
MODULE_PARM_DESC(g_tmds_aphy_cs_3, "g_tmds_aphy_cs_3");

module_param_array_named(tmds_aphy_cs_3_offset, g_tmds_aphy_cs_3_offset, uint,
						&g_tmds_aphy_cs_3_offset_len, 0640);
MODULE_PARM_DESC(tmds_aphy_cs_3_offset, "tmds_aphy_cs_3_offset");

module_param_named(g_tmds_data_ffe, g_tmds_data_ffe_debug, int, 0640);
MODULE_PARM_DESC(g_tmds_data_ffe, "g_tmds_data_ffe");

module_param_array_named(tmds_data_dphy_en, g_frl_dphy_spec_en, bool, &g_tmds_data_dphy_en_len, 0640);
MODULE_PARM_DESC(tmds_data_dphy_en, "tmds_data_dphy_en");

module_param_array_named(tmds_data_dphy_ffe, g_tmds_data_dphy_ffe, uint, &g_tmds_data_dphy_ffe_len, 0640);
MODULE_PARM_DESC(tmds_data_dphy_ffe, "tmds_data_dphy_ffe");

module_param_named(g_tmds_clk_ffe, g_tmds_clk_ffe_debug, int, 0640);
MODULE_PARM_DESC(g_tmds_clk_ffe, "g_tmds_clk_ffe");

module_param_array_named(tmds_clk_dphy_en, g_tmds_clk_dphy_en, bool, &g_tmds_clk_dphy_en_len, 0640);
MODULE_PARM_DESC(tmds_clk_dphy_en, "tmds_clk_dphy_en");

module_param_array_named(tmds_clk_dphy_ffe, g_tmds_clk_dphy_ffe, uint, &g_tmds_clk_dphy_ffe_len, 0640);
MODULE_PARM_DESC(tmds_clk_dphy_ffe, "tmds_clk_dphy_ffe");

int hdmitx_debug_read_scdc(uint32_t addr)
{
	uint8_t byte = 0;
	int retval;

	dpu_check_and_return(!g_hdmitx, -EINVAL, err, "[HDMI] g_hdmitx is NULL!\n");
	dpu_check_and_return(!g_hdmitx->ddc, -EINVAL, err, "[HDMI] g_hdmitx->ddc is NULL!\n");

	retval = hdmitx_ddc_scdc_readb(g_hdmitx->ddc, addr, &byte);
	if (retval) {
	       dpu_pr_err("[HDMI] read scdc fail\n");
	       return retval;
	}
	return (int)byte;
}

int hdmitx_debug_write_scdc(uint32_t addr, uint8_t byte)
{
	int retval;

	dpu_check_and_return(!g_hdmitx, -EINVAL, err, "[HDMI] g_hdmitx is NULL!\n");
	dpu_check_and_return(!g_hdmitx->ddc, -EINVAL, err, "[HDMI] g_hdmitx->ddc is NULL!\n");

	retval = hdmitx_ddc_scdc_writeb(g_hdmitx->ddc, addr, byte);
	if (retval) {
	       dpu_pr_err("[HDMI] write scdc fail\n");
	       return retval;
	}
	return 0;
}

uint32_t hdmitx_debug_read_pixel_clk(void)
{
	return hal_phy_get_pixel_clk(g_hdmitx->phy);
}

uint32_t hdmitx_debug_read_tmds_clk(void)
{
	return hal_phy_get_tmds_clk(g_hdmitx->phy);
}

uint32_t hdmitx_debug_read_frl_clk(void)
{
	return hal_phy_get_frl_clk(g_hdmitx->phy);
}

u8 dump_aphy_value(u32 aphy_cs, u8 aphy_offset)
{
	struct hdmitx_phy *hdmi_phy = g_hdmitx->phy;
	u8 data = 0;
	stb_readb(hdmi_phy, aphy_cs, aphy_offset, &data);
	return data;
}

void set_aphy_value(u32 aphy_cs, u8 aphy_offset, u8 data)
{
	struct hdmitx_phy *hdmi_phy = g_hdmitx->phy;
	stb_writeb(hdmi_phy, aphy_cs, aphy_offset, data);
	return;
}

void set_aphy_bits(u32 aphy_cs, u8 aphy_offset, u8 mask, u8 value)
{
	struct hdmitx_phy *hdmi_phy = g_hdmitx->phy;
	stb_set_bits(hdmi_phy, aphy_cs, aphy_offset, mask, value);
	return;
}

void avmute_control(bool enable)
{
	dpu_check_and_no_retval(!g_hdmitx, err, "[HDMI] g_hdmitx is NULL!\n");
	hdmitx_set_avmute(g_hdmitx, enable);
	return;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL_GPL(hdmitx_debug_read_scdc);
EXPORT_SYMBOL_GPL(hdmitx_debug_write_scdc);
EXPORT_SYMBOL_GPL(hdmitx_debug_read_pixel_clk);
EXPORT_SYMBOL_GPL(hdmitx_debug_read_tmds_clk);
EXPORT_SYMBOL_GPL(hdmitx_debug_read_frl_clk);
EXPORT_SYMBOL_GPL(dump_aphy_value);
EXPORT_SYMBOL_GPL(set_aphy_value);
EXPORT_SYMBOL_GPL(set_aphy_bits);
EXPORT_SYMBOL_GPL(avmute_control);
#endif

#endif