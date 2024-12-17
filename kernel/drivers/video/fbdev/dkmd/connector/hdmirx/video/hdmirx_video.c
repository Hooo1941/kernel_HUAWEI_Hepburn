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

#include "hdmirx_video.h"
#include "hdmirx_common.h"
#include <linux/delay.h>
#include "hdmirx_init.h"
#include "control/hdmirx_ctrl.h"
#include "hdmirx_video_dsc.h"
#include "packet/hdmirx_packet.h"

static hdmirx_video_ctx g_hdmirx_video_ctx;

uint32_t g_test_flag = 0;
void hdmirx_video_test_set_flag(uint32_t flag)
{
	g_test_flag = flag;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL_GPL(hdmirx_video_test_set_flag);
#endif

static hdmirx_video_ctx *hdmirx_video_get_ctx(void)
{
	return &g_hdmirx_video_ctx;
}

unsigned int hdmirx_video_get_hblank(void)
{
	hdmirx_video_ctx *video_ctx = hdmirx_video_get_ctx();

	disp_pr_info("htotal %d, hactive %d\n", video_ctx->hdmirx_timing_info.htotal, video_ctx->hdmirx_timing_info.hactive);
	return (video_ctx->hdmirx_timing_info.htotal - video_ctx->hdmirx_timing_info.hactive);
}

unsigned int hdmirx_video_get_vblank(void)
{
	hdmirx_video_ctx *video_ctx = hdmirx_video_get_ctx();

	disp_pr_info("vtotal %d, vactive %d\n", video_ctx->hdmirx_timing_info.vtotal, video_ctx->hdmirx_timing_info.vactive);
	return (video_ctx->hdmirx_timing_info.vtotal - video_ctx->hdmirx_timing_info.vactive);
}

unsigned int hdmirx_video_get_htotal(void)
{
	hdmirx_video_ctx *video_ctx = hdmirx_video_get_ctx();

	disp_pr_info("htotal %d\n", video_ctx->hdmirx_timing_info.htotal);
	return video_ctx->hdmirx_timing_info.htotal;
}

void hdmirx_video_detect_irq_mask(struct hdmirx_ctrl_st *hdmirx)
{
	// hv irq mask
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50C0, 1, 1, 3);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50C0, 1, 1, 8);
}

void hdmirx_video_gather_irq_mask(struct hdmirx_ctrl_st *hdmirx)
{
	// hv irq mask not set
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50C0, 0, 1, 3);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50C0, 0, 1, 8);

	// use gather irq
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x40, 1, 1, 14);

	disp_pr_info("gather video irq\n");
}

void hdmirx_video_detect_enable(struct hdmirx_ctrl_st *hdmirx)
{
	// hv detect enable
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50BC, 1, 1, 3);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50BC, 1, 1, 8);

	// hv irq mask
	hdmirx_video_detect_irq_mask(hdmirx);

	// hv detect threshold
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x5044, 0x4444, 16, 0);
}

bool is_video_change(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t temp;

	temp = inp32(hdmirx->hdmirx_pwd_base + 0x18);
	if (temp & 0x2)
		return true;

	temp = inp32(hdmirx->hdmirx_sysctrl_base + 0x44);
	if (temp & (1UL << 14))
		return true;

	return false;
}

int hdmirx_video_detect_process(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t temp;
	hdmirx_video_ctx *video_ctx;
	cfg_timing_info_st cfg_timing;

	if (!is_video_change(hdmirx))
		return 0;

	temp = inp32(hdmirx->hdmirx_pwd_base + 0x50C4);
	disp_pr_info("hdmirx_pwd_base + 0x50C4 is 0x%x\n", temp);
	if (temp & (1UL << 8))
		disp_pr_info("vactive change\n");

	if (temp & (1UL << 3))
		disp_pr_info("havtive change\n");

	cfg_timing.hactive_count = inp32(hdmirx->hdmirx_pwd_base + 0x50E0) & 0xffff;
	cfg_timing.htotal_count = inp32(hdmirx->hdmirx_pwd_base + 0x50E8) & 0xffff;
	cfg_timing.hfront_count = inp32(hdmirx->hdmirx_pwd_base + 0x50EC) & 0xffff;
	cfg_timing.hback_count = inp32(hdmirx->hdmirx_pwd_base + 0x50F0) & 0xffff;

	cfg_timing.vactive_count = inp32(hdmirx->hdmirx_pwd_base + 0x50F4) & 0xffff;
	cfg_timing.vtotal_count = inp32(hdmirx->hdmirx_pwd_base + 0x5100) & 0xffff;
	cfg_timing.vfront_count = inp32(hdmirx->hdmirx_pwd_base + 0x5108) & 0xffff;
	cfg_timing.vback_count = inp32(hdmirx->hdmirx_pwd_base + 0x5110) & 0xffff;

	cfg_timing.hsync_count = inp32(hdmirx->hdmirx_pwd_base + 0x50E4) & 0xffff;
	cfg_timing.vsync_count = inp32(hdmirx->hdmirx_pwd_base + 0x50F8) & 0xffff;
	cfg_timing.fps_count = inp32(hdmirx->hdmirx_pwd_base + 0x511C) & 0xffff;

	disp_pr_info("video detect: hactive_count %d, htotal_count %d, hfront_count %d, hback_count %d," \
		" vactive_count %d, vtotal_count %d, vfront_count %d, vback_count%d\n",
		cfg_timing.hactive_count, cfg_timing.htotal_count, cfg_timing.hfront_count, cfg_timing.hback_count,
		cfg_timing.vactive_count, cfg_timing.vtotal_count, cfg_timing.vfront_count, cfg_timing.vback_count);

	disp_pr_info("video sync: hsync_count %d, vsync_count %d, fps_count %d\n",
		cfg_timing.hsync_count, cfg_timing.vsync_count, cfg_timing.fps_count);

	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x507C, cfg_timing.hactive_count, 16, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x5084, cfg_timing.hfront_count, 16, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x5088, cfg_timing.hback_count, 16, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x508C, cfg_timing.htotal_count, 16, 0);

	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x5090, cfg_timing.vactive_count, 16, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x509C, cfg_timing.vfront_count, 16, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50A4, cfg_timing.vback_count, 16, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50AC, cfg_timing.vtotal_count, 16, 0);

	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x5080, cfg_timing.hsync_count, 16, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x5094, cfg_timing.vsync_count, 16, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x50B4, cfg_timing.fps_count, 16, 0);

	// rx-->dss
	disp_pr_info("start dss\n");
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, cfg_timing.vactive_count, 16, 16);
	hdmirx_display_hdmi_ready(hdmirx);

	if (g_test_flag == 0) {
		hdmirx_ctrl_set_sys_mute(hdmirx, false);
		disp_pr_info("clear mute\n");
	} else {
		disp_pr_info("not clear mute\n");
	}

	video_ctx = hdmirx_video_get_ctx();
	video_ctx->hdmirx_timing_info.hactive = cfg_timing.hactive_count;
	video_ctx->hdmirx_timing_info.vactive = cfg_timing.vactive_count;
	video_ctx->hdmirx_timing_info.htotal = cfg_timing.htotal_count;
	video_ctx->hdmirx_timing_info.vtotal = cfg_timing.vtotal_count;

	hdmirx_ctrl_irq_clear(hdmirx->hdmirx_pwd_base + 0x50C4);
	return 0;
}

void hdmirx_video_start_display(struct hdmirx_ctrl_st *hdmirx)
{
	// rx-->dss
	disp_pr_info("start display\n");
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 1, 1, 11);
	mdelay(33);
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 1, 2, 1);
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 1, 1, 3);

	disp_pr_info("video enable delay 200ms\n");
	mdelay(200);
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 0, 1, 0);
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 1, 1, 0);
	disp_pr_info("video enable\n");
}

void hdmirx_video_stop_display(struct hdmirx_ctrl_st *hdmirx)
{
	// rx-->dss
	disp_pr_info("stop display\n");
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 0, 1, 11);

	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 0, 2, 1);
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 0, 1, 3);

	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4C, 0, 1, 0);
	disp_pr_info("video disable\n");
}

int hdmirx_video_get_timinginfo(struct rx_interface *rx, struct disp_input_timing *timing)
{
	hdmirx_video_ctx *video_ctx;
	struct hdmirx_ctrl_st *hdmirx = container_of(rx, struct hdmirx_ctrl_st, interface_impl);

	video_ctx = hdmirx_video_get_ctx();
	hdmirx_video_dsc_pps_fetch(&(timing->dsc.pps));
	video_ctx->pixel_encoding = hdmirx_packet_avi_get_color_space(hdmirx);
	disp_pr_info("color_space 0x%x\n", video_ctx->pixel_encoding);
	video_ctx->input_width = hdmirx_packet_get_color_depth(hdmirx);
	disp_pr_info("bit_width 0x%x\n", video_ctx->input_width);

	timing->width = video_ctx->hdmirx_timing_info.hactive;
	timing->height = video_ctx->hdmirx_timing_info.vactive;

	timing->depth = video_ctx->input_width + 1;

	switch (video_ctx->pixel_encoding) {
	case HDMIRX_COLOR_SPACE_YCBCR422:
		timing->pixel_encoding = PIXEL_ENCODING_YUV422;
		timing->depth = COLOR_DEPTH_30BIT;
		disp_pr_info("HDMIRX_COLOR_SPACE_YCBCR422\n");
		break;
	case HDMIRX_COLOR_SPACE_YCBCR444:
		timing->pixel_encoding = PIXEL_ENCODING_YUV444;
		disp_pr_info("HDMIRX_COLOR_SPACE_YCBCR444\n");
		break;
	case HDMIRX_COLOR_SPACE_RGB:
		timing->pixel_encoding = video_ctx->pixel_encoding;
		disp_pr_info("HDMIRX_COLOR_SPACE_RGB\n");
		break;
	case HDMIRX_COLOR_SPACE_YCBCR420:
		timing->pixel_encoding = video_ctx->pixel_encoding;
		disp_pr_info("HDMIRX_COLOR_SPACE_YCBCR420\n");
		timing->width = timing->width * 2;
		break;
	default:
		disp_pr_err("hdmirx_video_get_timinginfo error : %d \n", video_ctx->pixel_encoding);
		break;
	}

	return 0;
}
