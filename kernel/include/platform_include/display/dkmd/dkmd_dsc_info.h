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

#ifndef DKMD_DSC_INFO_H
#define DKMD_DSC_INFO_H

#include <linux/types.h>

enum IFBC_TYPE {
	IFBC_TYPE_NONE = 0,
	IFBC_TYPE_VESA2X_SINGLE,
	IFBC_TYPE_VESA3X_SINGLE,
	IFBC_TYPE_VESA2X_DUAL,
	IFBC_TYPE_VESA3X_DUAL,
	IFBC_TYPE_VESA3_75X_DUAL,
	IFBC_TYPE_VESA4X_SINGLE_SPR,
	IFBC_TYPE_VESA4X_DUAL_SPR,
	IFBC_TYPE_VESA2X_SINGLE_SPR,
	IFBC_TYPE_VESA2X_DUAL_SPR,
	IFBC_TYPE_VESA3_75X_SINGLE,
	IFBC_TYPE_MAX,
};

enum DSC_VER {
	DSC_VERSION_V_1_1 = 0,
	DSC_VERSION_V_1_2,
};

enum PIXEL_FORMAT {
	DSC_RGB = 0,
	DSC_YUV,
	DSC_YUV420,
	DSC_YUV422,
	DSC_YUV444,
};

enum GEN_RC_PARAMS {
	DSC_NOT_GENERATE_RC_PARAMETERS = 0,
	DSC_GENERATE_RC_PARAMETERS,
};

struct input_dsc_info {
	enum DSC_VER dsc_version;
	enum PIXEL_FORMAT format;
	uint32_t pic_width;
	uint32_t pic_height;
	uint32_t slice_width;
	uint32_t slice_height;
	uint32_t dsc_bpp;
	uint32_t dsc_bpc;
	uint32_t linebuf_depth;
	uint32_t block_pred_enable;
	uint32_t gen_rc_params;
};
#endif