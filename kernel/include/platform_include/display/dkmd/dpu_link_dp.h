/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef DPU_LINK_DP_H
#define DPU_LINK_DP_H

#include <linux/types.h>
#define EDID_LEN 128

struct screen_info {
	uint32_t width;
	uint32_t height;
	uint32_t h_active_pixels;
	uint32_t v_active_pixels;
};

struct dp_ext_disp_info {
	uint8_t *edid;
	int port_id;
	uint32_t screen_num;
	uint32_t edid_len;
	uint32_t edid_blocks;
	struct screen_info *sinfo;
};

struct dp_connect_ctrl_info {
	bool is_dprx_ready;
	int port_id;
	uint32_t screen_num;
	uint32_t edid_len;
	uint32_t max_brightness;
	void(*set_backlight)(int port_id, uint32_t bl_level);

	int(*get_ext_disp_info)(struct dp_ext_disp_info *, int);
	int(*notify_ser_dp_vp_sync)(int port_id);
	int(*notify_ser_dp_unplug)(int port_id);
	int (*dp_connect_ctrl_reset_dprx)(int port_id);
};

// for dptx obtain dprx info such as edid that not support
extern int dpu_dptx_register_ext_disp_callback(struct dp_connect_ctrl_info *dp_info,
	uint32_t irq_type, uint32_t connect_type);

#endif