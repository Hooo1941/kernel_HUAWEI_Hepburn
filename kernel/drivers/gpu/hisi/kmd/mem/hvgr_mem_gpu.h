/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_GPU_H
#define HVGR_MEM_GPU_H

#include "hvgr_defs.h"

bool hvgr_mem_gpu_config(struct hvgr_mem_ctx * const pmem);
int hvgr_gpu_mem_change_attr(struct hvgr_ctx *ctx,
	struct hvgr_mem_area *area, uint64_t attr);
#endif
