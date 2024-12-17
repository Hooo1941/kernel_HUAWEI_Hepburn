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
#include "itfsw_cmd_data.h"
#include "config/dpu_opr_config.h"
#include "../opr_cmd_data.h"
#include "../opr_format.h"
#include "cmdlist_interface.h"
#include "smmu/dpu_comp_smmu.h"
#include "dkmd_log.h"

void opr_set_itfsw_dm_data(uint32_t itf_sel, uint32_t fmt, struct dpu_dm_itfsw_info *itfsw_info)
{
	itfsw_info->itf_input_fmt.reg.itf_input_fmt = fmt;
	itfsw_info->itf_input_fmt.reg.itf_layer_id = POST_LAYER_ID; /* 31 means no relationship to layer */
	itfsw_info->itf_input_fmt.reg.itf_sel = itf_sel;
	itfsw_info->itf_input_fmt.reg.itf_hidic_en = 0;
}