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
#ifndef DKMD_FFD_H
#define DKMD_FFD_H

#include <linux/types.h>

#define FFD_ROI_CNT_MAX 32
#define FFD_ROI_AREA_MAX 2048

enum ffd_mode {
	FFD_FRAME = 0,
	FFD_ROI0,
	FFD_ROI1,
	FFD_MODE_CNT,
};

struct roi_area {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct ffd_roi_cfg {
	bool roi_en;
	struct roi_area roi_area;
};

struct dkmd_ffd_cfg {
	bool ffd_en;
	enum ffd_mode ffd_mode;
	uint32_t mode0_roi_cfg_bytes; // ROI0 mode
	struct ffd_roi_cfg roi_cfg[FFD_ROI_CNT_MAX]; // ROI1 mode
};

#endif
