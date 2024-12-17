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

#include <linux/types.h>
#include "dpu_comp_dfr_te_skip.h"

void dfr_te_skip_setup_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}
void dfr_te_skip_release_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}
int32_t dfr_te_skip_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate)
{
	void_unused(dfr_ctrl);
	void_unused(frame_rate);
	return 0;
}
int32_t dfr_te_skip_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
	return 0;
}
int32_t dfr_te_skip_send_dcs_cmds(struct dpu_comp_dfr_ctrl *dfr_ctrl, struct disp_effect_params *effect_params)
{
	void_unused(dfr_ctrl);
	void_unused(effect_params);
	return 0;
}
int32_t dfr_te_skip_send_dcs_cmds_by_riscv(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t bl_level)
{
	void_unused(dfr_ctrl);
	void_unused(bl_level);
	return 0;
}
int32_t dfr_te_skip_set_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate)
{
	void_unused(dfr_ctrl);
	void_unused(safe_frm_rate);
	return 0;
}
void dfr_te_skip_self_refresh(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}
void dfr_te_skip_enable_dimming(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}
void dfr_te_skip_disable_dimming(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}

void dfr_te_skip_enable_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}

void dfr_te_skip_disable_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, int32_t disable_type)
{
	void_unused(dfr_ctrl);
	void_unused(disable_type);
}

void dfr_te_skip_acpu_setup_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}

void dfr_te_skip_acpu_release_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}

int32_t dfr_te_skip_acpu_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate)
{
	void_unused(dfr_ctrl);
	void_unused(frame_rate);
	return 0;
}

int32_t dfr_te_skip_acpu_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
	return 0;
}

int32_t dfr_te_skip_acpu_set_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate)
{
	void_unused(dfr_ctrl);
	void_unused(safe_frm_rate);
	return 0;
}

void dfr_te_skip_acpu_exit_idle_status(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}

void dfr_longh_te_skip_func_register(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	void_unused(dfr_ctrl);
}