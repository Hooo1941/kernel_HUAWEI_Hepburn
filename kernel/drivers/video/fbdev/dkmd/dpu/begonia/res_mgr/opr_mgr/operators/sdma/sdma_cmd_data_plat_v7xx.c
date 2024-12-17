/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#include <linux/slab.h>

#include <dpu/dpu_dm.h>
#include <dpu_format.h>
#include "sdma_cmd_data.h"
#include "../opr_cmd_data.h"
#include "../opr_format.h"
#include "dkmd_log.h"

void opr_set_sdma_layer_fmt(uint32_t fmt, uint32_t compress_type, uint32_t dma_sel,
	struct dpu_dm_layer_info *layer_info)
{
	layer_info->layer_dma_sel.reg.layer_dma_fmt = (uint32_t)dpu_fmt_to_sdma((int32_t)fmt);
	layer_info->layer_dma_sel.reg.layer_fbc_type = compress_type;
	layer_info->layer_dma_sel.reg.layer_dma_sel = dma_sel;

	layer_info->layer_bot_crop.reg.layer_ov_fmt = (uint32_t)dpu_fmt_to_dynamic_dfc((int32_t)fmt);
	dpu_pr_info("format=%d compress_type=%d", fmt, compress_type);
}
