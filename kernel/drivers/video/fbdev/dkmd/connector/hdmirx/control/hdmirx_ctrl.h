/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HDMIRX_CTRL_H
#define HDMIRX_CTRL_H

#include "hdmirx_struct.h"

enum hisi_fb_pixel_format {
	HISI_FB_PIXEL_FORMAT_RGBA_8888 = 6,
	/* YUV Package */
	HISI_FB_PIXEL_FORMAT_YUYV_422_PKG = 23,
	/* 10bit */
	HISI_FB_PIXEL_FORMAT_RGBA_1010102 = 27,
	HISI_FB_PIXEL_FORMAT_YUV422_10BIT = 30,
	HISI_FB_PIXEL_FORMAT_YUVA444 = 39,
	HISI_FB_PIXEL_FORMAT_YUV444 = 40,
	HISI_FB_PIXEL_FORMAT_YUYV_420_PKG = 41,
	HISI_FB_PIXEL_FORMAT_YUYV_420_PKG_10BIT = 42,
	HISI_FB_PIXEL_FORMAT_D3_128 = 43,
	HISI_FB_PIXEL_FORMAT_MAX,
};

int hdmirx_ctrl_init(struct hdmirx_ctrl_st *hdmirx);
void hdmirx_ctrl_source_select_hdmi(struct hdmirx_ctrl_st *hdmirx);
void hdmirx_ctrl_set_sys_mute(struct hdmirx_ctrl_st *hdmirx, bool enable);
bool hdmirx_ctrl_get_sys_mute(struct hdmirx_ctrl_st *hdmirx);
void hdmirx_ctrl_mute_irq_mask(struct hdmirx_ctrl_st *hdmirx);
void hdmirx_ctrl_mute_proc(struct hdmirx_ctrl_st *hdmirx);
bool hdmirx_ctrl_is_hdr10(void);
uint32_t hdmirx_ctrl_get_layer_fmt(struct hdmirx_ctrl_st *hdmirx);
void hdmirx_ctrl_yuv422_set(struct hdmirx_ctrl_st *hdmirx, uint32_t enable);
int hdmirx_ctrl_get_timinginfo(struct rx_interface *rx, void __user *argp);
int hdmirx_ctrl_interface_init(struct rx_interface *impl);
void hdmirx_hpd_retry(struct hdmirx_ctrl_st *hdmirx);
#endif