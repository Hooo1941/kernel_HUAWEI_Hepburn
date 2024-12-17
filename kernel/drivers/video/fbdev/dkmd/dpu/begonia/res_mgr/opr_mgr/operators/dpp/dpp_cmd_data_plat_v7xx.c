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
#include "dpp_cmd_data.h"
#include "../opr_format.h"
#include "config/dpu_config_utils.h"
#include "config/dpu_opr_config.h"
#include "cmdlist_interface.h"
#include "dkmd_log.h"

void set_dpp_sel_info(uint32_t dpp_sel, uint32_t dpp_order, struct dpu_dm_dpp_info *dpp_info)
{
	dpp_info->dpp_sel.reg.dpp_sel = dpp_sel;
	dpp_info->dpp_sel.reg.dpp_order1 = OPR_INVALID;
	dpp_info->dpp_sel.reg.dpp_order0 = dpp_order;
	dpp_info->dpp_sel.reg.dpp_layer_id = POST_LAYER_ID;
}

void opr_set_dpp_cfg_data(struct opr_cmd_data_base *data)
{
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id, DPU_DPP_GMP_EN_ADDR(data->module_offset), 0x2);

	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id, DPU_DPP_ROIGAMA0_EN_ADDR(data->module_offset), 0);
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id, DPU_DPP_ROIGAMA1_EN_ADDR(data->module_offset), 0);
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id, DPU_DPP_NLXCC_EN_ADDR(data->module_offset), 0);
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id, DPU_DPP_XCC_EN_ADDR(data->module_offset), 0);
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id, DPU_DPP_CLIP_EN_ADDR(data->module_offset), 0);
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id, DPU_DPP_ALSC_DEGAMMA_EN_ADDR(data->module_offset), 0);
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id, DPU_DPP_ALSC_EN_ADDR(data->module_offset), 0);

	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id,
		DPU_DPP_REG_CTRL_ADDR(data->module_offset), (uint32_t)data->scene_id);
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id,
		DPU_DPP_REG_CTRL_FLUSH_EN_ADDR(data->module_offset), 1);
}