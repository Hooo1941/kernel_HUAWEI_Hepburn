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

#ifndef GFXDEV_UTILS_PANDISPLAY_H
#define GFXDEV_UTILS_PANDISPLAY_H

#include <linux/fb.h>

typedef struct fb_var_screeninfo gfxdev_var_screeninfo;
typedef struct fb_fix_screeninfo gfxdev_fix_screeninfo;

enum {
	GFXDEV_FORMAT_BGRA8888,
	GFXDEV_FORMAT_RGB565,
	GFXDEV_FORMAT_YUV_422_I,
	GFXDEV_FOARMT_MAX
};

struct fix_var_screeninfo {
	uint32_t fix_type;
	uint32_t fix_xpanstep;
	uint32_t fix_ypanstep;
	uint32_t var_vmode;

	uint32_t var_blue_offset;
	uint32_t var_green_offset;
	uint32_t var_red_offset;
	uint32_t var_transp_offset;

	uint32_t var_blue_length;
	uint32_t var_green_length;
	uint32_t var_red_length;
	uint32_t var_transp_length;

	uint32_t var_blue_msb_right;
	uint32_t var_green_msb_right;
	uint32_t var_red_msb_right;
	uint32_t var_transp_msb_right;
	uint32_t bpp;
};

struct fix_var_screeninfo* get_fix_var_screeninfo(void);
int32_t gfxdev_init_fscreen_info(struct composer *comp, struct fix_var_screeninfo *screen_info,
	gfxdev_fix_screeninfo *fix);
int32_t build_display_info(const struct dkmd_object_info *object_info, gfxdev_fix_screeninfo *fix,
	gfxdev_var_screeninfo *var, struct composer *comp, struct dkmd_base_frame *frame, struct dkmd_network *network);
int32_t execute_compose(struct composer *comp, gfxdev_var_screeninfo *var,
	struct dkmd_base_frame *frame, struct dkmd_network *network);
void gfxdev_init_fbi_var_info(const struct dkmd_object_info *pinfo,
	struct fix_var_screeninfo *screen_info, gfxdev_var_screeninfo *var);
int32_t gfxdev_pan_display(gfxdev_fix_screeninfo *fix, gfxdev_var_screeninfo *var,
	const struct dkmd_object_info *pinfo, struct composer *comp);
#endif