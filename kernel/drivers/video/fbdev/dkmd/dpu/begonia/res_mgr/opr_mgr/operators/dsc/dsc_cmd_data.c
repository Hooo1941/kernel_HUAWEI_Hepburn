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

#include <linux/slab.h>
#include <dpu/dpu_dm.h>
#include "dsc_cmd_data.h"
#include "config/dpu_opr_config.h"
#include "../opr_cmd_data.h"
#include "../opr_format.h"
#include "cmdlist_interface.h"
#include "smmu/dpu_comp_smmu.h"
#include "dkmd_log.h"

struct opr_cmd_data *init_dsc_cmd_data(union dkmd_opr_id id)
{
	struct opr_cmd_data *cmd_data = NULL;

	cmd_data = kzalloc(sizeof(*cmd_data), GFP_KERNEL);
	if (unlikely(!cmd_data)) {
		dpu_pr_err("alloc cmd_data mem fail");
		return NULL;
	}

	cmd_data->data = kzalloc(sizeof(*cmd_data->data), GFP_KERNEL);
	if (unlikely(!cmd_data->data)) {
		dpu_pr_err("alloc cmd_data->data mem fail");
		kfree(cmd_data);
		cmd_data = NULL;
		return NULL;
	}

	cmd_data->set_data = opr_set_dsc_data;
	cmd_data->data->opr_id = id;
	cmd_data->data->module_offset = g_dsc_offset[id.info.ins];

	return cmd_data;
}

static void set_dsc_out_common_data(const struct dkmd_base_layer *base_layer, struct opr_cmd_data *cmd_data)
{
	struct dkmd_rect_coord out_rect = {
		.left = 0,
		.right = base_layer->dsc_out_width,
		.top = 0,
		.bottom = base_layer->dsc_out_height,
	};
	cmd_data->data->out_common_data.rect = out_rect;
	cmd_data->data->out_common_data.format =
		(base_layer->dsc_en != 0) ? DPU_FMT_RGB_10BIT : cmd_data->data->in_common_data.format;
	return;
}

static void opr_set_dsc_cfg_data(const struct opr_cmd_data_base *data)
{
	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id,
		DPU_DSC_REG_CTRL_ADDR(data->module_offset), data->scene_id);

	dkmd_set_reg((uint32_t)data->scene_id, data->reg_cmdlist_id,
		DPU_DSC_REG_CTRL_FLUSH_EN_ADDR(data->module_offset), 1);
}

int32_t opr_set_dsc_data(struct opr_cmd_data *cmd_data, const struct dkmd_base_layer *base_layer,
	const struct opr_cmd_data *pre_cmd_data, const struct opr_cmd_data **next_cmd_datas, uint32_t next_oprs_num)
{
	struct dpu_dm_param *dm_param = NULL;
	struct dpu_dm_dsc_info *dsc_info = NULL;
	struct opr_cmd_data_base *data = NULL;
	uint32_t width;
	uint32_t height;

	dpu_assert(!cmd_data);
	dpu_assert(!pre_cmd_data);
	dpu_assert(next_oprs_num < 1);

	data = cmd_data->data;
	dm_param = data->dm_param;
	dpu_assert(!dm_param);

	set_common_cmd_data(cmd_data, pre_cmd_data);
	set_dsc_out_common_data(base_layer, cmd_data);

	dsc_info = &dm_param->dsc_info[0];
	++dm_param->scene_info.layer_number.reg.dsc_number;

	width = rect_width(&data->in_common_data.rect);
	height = rect_height(&data->in_common_data.rect);
	if (width == 0 || height == 0) {
		dpu_pr_err("width=%u or height=%u is zero", width, height);
		return -1;
	}
	dsc_info->dsc_input_img_width.reg.dsc_input_img_width = width - 1;
	dsc_info->dsc_input_img_width.reg.dsc_input_img_height = height - 1;
	dsc_info->dsc_output_img_width.reg.dsc_output_img_width = base_layer->dsc_out_width - 1;
	dsc_info->dsc_output_img_width.reg.dsc_output_img_height = base_layer->dsc_out_height - 1;

	dsc_info->dsc_sel.reg.dsc_sel = BIT(cmd_data->data->opr_id.info.ins);
	dsc_info->dsc_sel.reg.dsc_order1 = OPR_INVALID;
	dsc_info->dsc_sel.reg.dsc_order0 = (uint32_t)opr_dpu_to_soc_type(next_cmd_datas[0]->data->opr_id.info.type);
	dsc_info->dsc_sel.reg.dsc_layer_id = POST_LAYER_ID;

	dsc_info->dsc_reserved.reg.dsc_output_fmt = (uint32_t)dpu_fmt_to_sdma(cmd_data->data->out_common_data.format);
	dsc_info->dsc_reserved.reg.dsc_input_fmt = (uint32_t)dpu_fmt_to_sdma(cmd_data->data->in_common_data.format);

	dpu_pr_debug("[DSC]: input rect[0x%x], out rect[0x%x], format[0x%x]",
		dsc_info->dsc_input_img_width.value, dsc_info->dsc_output_img_width.value, dsc_info->dsc_reserved.value);

	opr_set_dsc_cfg_data(data);

	return 0;
}