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

#ifndef DKMD_HIACE_INFO_H
#define DKMD_HIACE_INFO_H

#include "dkmd_user_panel_info.h"
#include <linux/types.h>

#define DKMD_DETAIL_WEIGHT_SIZE      9
#define DKMD_LOG_LUM_EOTF_LUT_SIZE   32
#define DKMD_LUMA_GAMA_LUT_SIZE      21

#define DKMD_XBLOCKNUM               6
#define DKMD_YBLOCKNUM               6
#define DKMD_HIACE_LHIST_RANK        16
#define DKMD_HIACE_GHIST_RANK        32
#define DKMD_HIACE_GAMMA_RANK        8
#define DKMD_HIACE_FNA_RANK          1
#define DKMD_HIACE_SKIN_COUNT_RANK   36
#define DKMD_CE_HIST_SIZE            (DKMD_HIACE_GHIST_RANK * 2 + \
	DKMD_YBLOCKNUM * DKMD_XBLOCKNUM * DKMD_HIACE_LHIST_RANK + \
	DKMD_YBLOCKNUM * DKMD_XBLOCKNUM * DKMD_HIACE_FNA_RANK + DKMD_HIACE_SKIN_COUNT_RANK + 1)
#define DKMD_CE_LUT_SIZE             (DKMD_YBLOCKNUM * DKMD_XBLOCKNUM * DKMD_HIACE_GAMMA_RANK)

#define DKMD_GLOBAL_HIST_SIZE        32
#define DKMD_SAT_GLOBAL_HIST_SIZE    32
#define DKMD_SKIN_COUNT_SIZE         18
#define DKMD_SKIN_COUNT_TABLE_SIZE   36
#define DKMD_LOCAL_HIST_SIZE         (DKMD_YBLOCKNUM * DKMD_XBLOCKNUM * DKMD_HIACE_LHIST_RANK)
#define DKMD_FNA_DATA_SIZE           (DKMD_YBLOCKNUM * DKMD_XBLOCKNUM * DKMD_HIACE_FNA_RANK)

#define DKMD_HIACE_DETAIL_WEIGHT_TABLE_SIZE 33
#define DKMD_HIACE_LOGLUM_EOTF_TABLE_SIZE   63
#define DKMD_HIACE_LUMA_GAMA_TABLE_SIZE     63
#define DKMD_HIACE_GAMA_LUT_SIZE            DKMD_CE_LUT_SIZE

#define DKMD_HIACE_HIST_VALID_LOCAL BIT(0)
#define DKMD_HIACE_HIST_VALID_FNA BIT(1)
#define DKMD_HIACE_HIST_VALID_GLOBAL BIT(2)

#define DKMD_HIACE_LUT_UPDATE_LOCAL_GAMMA BIT(0)
#define DKMD_HIACE_LUT_UPDATE_DETAIL_WEIGHT BIT(1)
#define DKMD_HIACE_LUT_UPDATE_LOG_LUMA BIT(2)
#define DKMD_HIACE_LUT_UPDATE_LUMA_GAMMA BIT(3)

enum {
	DKMD_HIACE_HIST_STATUS_ENABLE,
	DKMD_HIACE_HIST_STATUS_DISABLE,

	// Single Mode, read Hist onece and disable Hiace
	DKMD_HIACE_HIST_STATUS_SINGLE,
};

#define DKMD_HIACE_HIST_CAP_LOCAL BIT(0)
#define DKMD_HIACE_HIST_CAP_GLOBLE BIT(1)
#define DKMD_HIACE_HIST_CAP_FNA BIT(2)

struct dkmd_hiace_hist_config {
	uint32_t status;
	uint32_t caps;
	uint32_t local_hist_width; // 8 or 16 bits
};

enum folding_screen_mode {
	F_FOLDING_MODE = 1,
	M_FOLDING_MODE,
	G_FOLDING_MODE,
	DEFAULT_FOLDING_MODE,
};

struct dkmd_hiace_hist_data {
	uint32_t local_hist[DKMD_LOCAL_HIST_SIZE];
	uint32_t global_hist[DKMD_GLOBAL_HIST_SIZE];
	uint32_t sat_global_hist[DKMD_SAT_GLOBAL_HIST_SIZE];
	uint32_t fna_data[DKMD_FNA_DATA_SIZE];
	uint32_t skin_cnt[DKMD_SKIN_COUNT_TABLE_SIZE];
	uint32_t sum_sat;
	uint32_t valid_info; // bit0:local bit1:fna bit2:global
	uint32_t lhist_sft; // Number of bits shifted rightwards for local hist. The value range is [0, 7].
	uint32_t lhist_bit_width; // 8 or 16 bits

	// local hist block config, used for hist validate check
	uint32_t lhist_half_blk_width;
	uint32_t lhist_half_blk_height;

	uint32_t folding_mode;
	struct dkmd_rect_coord hist_rect;
};

struct dkmd_hiace_lut_data {
	uint32_t detail_weight_lut[DKMD_HIACE_DETAIL_WEIGHT_TABLE_SIZE];
	uint32_t log_lum_lut[DKMD_HIACE_LOGLUM_EOTF_TABLE_SIZE];
	uint32_t luma_gamma_lut[DKMD_HIACE_LUMA_GAMA_TABLE_SIZE];
	uint32_t gamma_lut[DKMD_HIACE_GAMA_LUT_SIZE];
	uint32_t update_info;
	uint32_t enable_info;
};

#endif // DKMD_HIACE_INFO_H