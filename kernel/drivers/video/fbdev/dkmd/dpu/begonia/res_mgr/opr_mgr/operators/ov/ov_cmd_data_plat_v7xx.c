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
#include "ov_cmd_data.h"
#include "../opr_cmd_data.h"
#include "../opr_format.h"
#include "dkmd_log.h"

void ov_set_layer_fmt(uint32_t fmt, struct dpu_dm_layer_info *layer_info)
{
	layer_info->layer_bot_crop.reg.layer_ov_fmt = fmt;
}
