/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef __SPR_H__
#define __SPR_H__

#include "dkmd_connector.h"
#include "dkmd_spr_info.h"

enum SPR_GAMMA_TYPE {
	SPR_GAMMA_R = 0,
	SPR_GAMMA_G,
	SPR_GAMMA_B,
	SPR_DEGAMMA_R,
	SPR_DEGAMMA_G,
	SPR_DEGAMMA_B,
	SPR_GAMMA_LUT_ROW
};

#define SPR_GAMMA_LUT_COLUMN 258
#define SPR_GAMMA_LUT_SIZE (SPR_GAMMA_LUT_ROW * SPR_GAMMA_LUT_COLUMN)
#define COEF_PER_REG 2
#define DSC_OUTPUT_MODE 24
#define SPR_CLOSE_BOARDER_GAIN 0x800080

#define SPR_ENABLE 1
#define TXIP_ENABLE 1
#define DATAPACK_ENABLE 1
#define DATAPACK_UNPACK 0
#define GMP_BITEXT_COPY_HIGH_TO_LOW 1

void spr_init(struct spr_info *spr, char __iomem * dpp_base, char __iomem * dsc_base);
bool is_spr_enabled(struct spr_info *spr);
#endif
