/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2025. All rights reserved.
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

#ifndef DPU_DEBUG_DUMP_H
#define DPU_DEBUG_DUMP_H

#include "dkmd_rect.h"

enum LAYER_CAPABILITY {
	CAPS_SCL = BIT(0),
	CAPS_ROT = BIT(1),
	CAPS_HEBCE = BIT(2),
	CAPS_UVUP = BIT(3),
	CAPS_UVSWAP = BIT(4),
	CAPS_SDMA_VMIRROR = BIT(5), // used for sdma vmirror effect on clip or pad(vmirror shoud top-bottom reverse)
	CAPS_SRC_CLIP = BIT(6), // each layer need src clip in default, when clipped by a module, remove this cap
	CAPS_OV_CLIP_EXPAND = BIT(7), // to solve ov y-clip restriction, ov expand its output and clipped in DPP/ITF...
	CAPS_HDR = BIT(8),
	CAPS_ROG = BIT(9),
	CAPS_CLD = BIT(10),
	CAPS_DIM = BIT(11),
};

#define DM_SCL_REG_LENGTH 0x10
#define DM_CLD_REG_LENGTH 0x10
#define DM_USE_SCL_BEFORE_OV 0x0
#define DM_USE_SCL_AFTER_OV 0x1
#define DM_INVALID_LAYER_ID 0x1F
#define plus_one(x) ((x) + 1)

struct dkmd_dm_layer_info {
	struct dkmd_rect_coord src_aligned_rect;
	struct dkmd_rect_coord dst_rect;
	unsigned char layer_id;
	unsigned char format;
	unsigned char sdma_idx;
	unsigned char trasform;
	unsigned short caps;
	char rsv[2];
};

struct dkmd_dm_info {
	unsigned char layer_cnt;
	char rsv[3];
	struct dkmd_dm_layer_info *dm_layer_info;
};

int32_t dpu_parse_layer_info(char __iomem *dpu_base, uint32_t scene_id);

#endif
