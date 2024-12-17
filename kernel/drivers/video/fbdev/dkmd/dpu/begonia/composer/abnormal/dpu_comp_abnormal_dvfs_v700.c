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
#include <dpu/soc_dpu_define.h>
#include "dpu_comp_abnormal_handle.h"
#include "dvfs.h"

void dpu_comp_abnormal_dvfs(uint32_t comp_index)
{
	dpu_pr_info("The current underflow requires sw dvfs");
	dpu_dvfs_direct_vote(comp_index, DPU_CORE_LEVEL_ON, true);
}