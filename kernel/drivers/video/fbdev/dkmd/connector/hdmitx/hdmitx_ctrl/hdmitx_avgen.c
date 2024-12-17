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
#include <linux/delay.h>
#include <securec.h>

#include "dkmd_log.h"
#include "hdmitx_ctrl.h"
#include "hdmitx_connector.h"
#include "hdmitx_ddc.h"
#include "hdmitx_frl_config.h"
#include "hdmitx_frl.h"
#include "hdmitx_core_config.h"
#include "hdmitx_infoframe.h"
#include "hdmitx_emp.h"
#include "hdmitx_avgen.h"

#define REG_TIMING_CTRL0 0x4C
#define REG_TIMING_CTRL0_M (0x1 << 0)

#define REG_H_FRONT 0x50
#define REG_H_SYNC 0x54
#define REG_H_BACK 0x58
#define REG_H_ACTIVE 0x5C

#define REG_V_FRONT 0x60
#define REG_V_SYNC 0x64
#define REG_V_BACK 0x68
#define REG_V_ACTIVE 0x6C

static s32 hdmitx_timing_select(struct hdmitx_ctrl *hdmitx)
{
	struct list_head *n = NULL;
	struct list_head *pos = NULL;
	struct hdmitx_display_mode *dis_mode = NULL;
	struct hdmitx_connector *connector = NULL;

	connector = hdmitx->hdmitx_connector;
	if (connector == NULL)
		return -1;

	if (list_empty(&connector->probed_modes))
		return -1;

	list_for_each_safe(pos, n, &connector->probed_modes) {
		dis_mode = list_entry(pos, struct hdmitx_display_mode, head);
		if (dis_mode == NULL)
			continue;

		if ((dis_mode->type & HDMITX_MODE_TYPE_PREFERRED) != 0) {
			memcpy_s(&(hdmitx->select_mode), sizeof(struct hdmitx_display_mode),
				dis_mode, sizeof(struct hdmitx_display_mode));
			return 0;
		}
	}
	return -1;
}

void hdmitx_timing_gen_enable(struct hdmitx_ctrl *hdmitx)
{
	dpu_check_and_no_retval(!hdmitx, err, "hdmitx is NULL");
	hdmi_clrset(hdmitx->sysctrl_base, REG_TIMING_CTRL0, REG_TIMING_CTRL0_M, 0x1);
}

void hdmitx_timing_gen_disable(struct hdmitx_ctrl *hdmitx)
{
	dpu_check_and_no_retval(!hdmitx, err, "hdmitx is NULL");
	hdmi_clrset(hdmitx->sysctrl_base, REG_TIMING_CTRL0, REG_TIMING_CTRL0_M, 0x0);
}

static void hdmitx_set_video_path(struct hdmitx_ctrl *hdmitx)
{
	// dpu video data transfer to HDMI must be RGB
	// vdp packet = metadata control
	core_set_vdp_packet(hdmitx->base, true);
	// to fix : according dpu color depth
	core_set_color_depth(hdmitx->base, g_hdmitx_color_depth_debug);
}

void hdmitx_set_mode(struct hdmitx_ctrl *hdmitx)
{
	dpu_check_and_no_retval(!hdmitx, err, "hdmitx is NULL");

	if (g_hdmitx_hdmi_dvi_mode_debug)
		core_set_hdmi_mode(hdmitx->base, true);
}

void hdmitx_video_config(struct hdmitx_ctrl *hdmitx)
{
	dpu_check_and_no_retval(!hdmitx, err, "hdmitx is NULL");

	hdmitx_set_infoframe(hdmitx);
	hdmitx_set_emp(hdmitx);
	hdmitx_set_video_path(hdmitx);
	// set hdmi mode
	hdmitx_set_mode(hdmitx);
	// udp to fix :enable tmds scramble
	hdmitx_ctrl_tmds_set_scramble(hdmitx);
}

void hdmitx_set_avmute(struct hdmitx_ctrl *hdmitx, bool enable)
{
	dpu_check_and_no_retval(!hdmitx, err, "hdmitx is NULL");
	if (enable) {
		core_send_av_mute(hdmitx->base);
		msleep(50);
	} else {
		core_send_av_unmute(hdmitx->base);
	}
}

static void hdmitx_timing_gen_config(struct hdmitx_ctrl *hdmitx,
	struct hdmitx_display_mode *mode)
{
	struct hdmi_detail *detail = NULL;

	dpu_check_and_no_retval(!hdmitx, err, "hdmitx is NULL");
	dpu_check_and_no_retval(!mode, err, "mode is NULL");

	detail = &mode->detail;

	hdmi_writel(hdmitx->sysctrl_base, REG_H_FRONT, detail->h_front);
	hdmi_writel(hdmitx->sysctrl_base, REG_H_SYNC, detail->h_sync);
	hdmi_writel(hdmitx->sysctrl_base, REG_H_BACK, detail->h_back);
	hdmi_writel(hdmitx->sysctrl_base, REG_H_ACTIVE, detail->h_active);

	hdmi_writel(hdmitx->sysctrl_base, REG_V_FRONT, detail->v_front);
	hdmi_writel(hdmitx->sysctrl_base, REG_V_SYNC, detail->v_sync);
	hdmi_writel(hdmitx->sysctrl_base, REG_V_BACK, detail->v_back);
	hdmi_writel(hdmitx->sysctrl_base, REG_V_ACTIVE, detail->v_active);
}

s32 hdmitx_timing_config_safe_mode(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_display_mode *safe_mode = NULL;

	dpu_check_and_return(!hdmitx, -EINVAL, err, "[HDMI] hdmitx is NULL");

	safe_mode = hdmitx_modes_create_mode_by_vic(VIC_640X480P60_4_3);
	hdmitx_timing_gen_config(hdmitx, safe_mode);
	hdmitx_timing_gen_enable(hdmitx);
	return 0;
}

s32 hdmitx_timing_config(struct hdmitx_ctrl *hdmitx)
{
	dpu_check_and_return(!hdmitx, -EINVAL, err, "[HDMI] hdmitx is NULL");

	hdmitx_timing_select(hdmitx);

	// vic mock for display mode
	if (g_hdmitx_display_mode_vic_debug)
		memcpy_s(&hdmitx->select_mode, sizeof(struct hdmitx_display_mode),
			hdmitx_modes_create_mode_by_vic(g_hdmitx_display_mode_vic_debug), sizeof(struct hdmitx_display_mode));

	// hdmitx timing gen disable
	hdmitx_timing_gen_config(hdmitx, &hdmitx->select_mode);
	return 0;
}

void hdmitx_timing_mock(struct hdmitx_ctrl *hdmitx)
{
	dpu_check_and_no_retval(!hdmitx, err, "[HDMI] hdmitx is NULL");
	if (g_hdmitx_display_mode_vic_debug)
		memcpy_s(&hdmitx->select_mode, sizeof(struct hdmitx_display_mode),
			hdmitx_modes_create_mode_by_vic(g_hdmitx_display_mode_vic_debug), sizeof(struct hdmitx_display_mode));
	return;
}
