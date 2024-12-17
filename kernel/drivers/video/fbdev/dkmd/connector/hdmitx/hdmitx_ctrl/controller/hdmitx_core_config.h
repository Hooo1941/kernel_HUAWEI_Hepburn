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

#ifndef __HDMITX_CORE_CONFIG_H__
#define __HDMITX_CORE_CONFIG_H__

#include <linux/types.h>
#include <linux/mutex.h>
#include "hdmitx_ctrl.h"

void core_hw_send_info_frame(void *hdmi_reg_base, const u8 *buffer, u8 len);
void core_set_vdp_packet(void *hdmi_reg_base, bool enabe);
void core_disable_deepcolor_for_dsc(void *hdmi_reg_base);
void core_set_color_depth(void *hdmi_reg_base, int in_color_depth);
void core_set_hdmi_mode(void *hdmi_reg_base, bool is_hdmi_mode);
void core_send_av_mute(void *hdmi_reg_base);
void core_send_av_unmute(void *hdmi_reg_base);
void core_enable_hdcp_clk(void *hsdt1_sub_harden_base);
void core_disable_hdcp_clk(void *hsdt1_sub_harden_base);
void core_enable_memory(void *hdmi_reg_base);
void core_disable_memory(void *hdmi_reg_base);
void core_reset_pwd(void *hdmi_reg_base, bool enable);
void core_hdmitx_controller_soft_reset(const struct hdmitx_ctrl *hdmitx);
void core_reset_req_all(void *hdmi_reg_base);
void core_set_audio_acr(void *hdmi_reg_base);
void core_set_video_rgb_solid_color(void *hdmi_reg_base, u32 solid_color_type, bool enable);
void core_set_video_color_bar(void *hdmi_reg_base, bool enable);
void core_hw_enable_tmds_scramble(void *hdmi_reg_base, bool enable);

#endif
