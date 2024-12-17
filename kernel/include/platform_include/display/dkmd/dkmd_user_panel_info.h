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

#ifndef DKMD_USER_PANEL_INFO_H
#define DKMD_USER_PANEL_INFO_H

#include <linux/types.h>

struct dkmd_rect {
	int32_t x;
	int32_t y;
	uint32_t w;
	uint32_t h;
};

struct dkmd_rect_coord {
	uint32_t left;
	uint32_t top;
	uint32_t right;
	uint32_t bottom;
};

struct user_panel_info {
	int32_t left_align;
	int32_t right_align;
	int32_t top_align;
	int32_t bottom_align;

	int32_t w_align;
	int32_t h_align;
	int32_t w_min;
	int32_t h_min;

	int32_t top_start;
	int32_t bottom_start;
	int32_t spr_overlap;
	uint8_t dirty_region_updt_support;
	uint8_t rsv[3];
};

#endif
