/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#ifndef COMPOSER_DFR_CONSTANT_H
#define COMPOSER_DFR_CONSTANT_H

#include "dpu_comp_dfr.h"

void dfr_constant_setup_data(struct dpu_comp_dfr_ctrl *dfr_ctrl);
int32_t dfr_constant_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate);
int32_t dfr_constant_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl);
int32_t dfr_constant_send_dcs_cmds(struct dpu_comp_dfr_ctrl *dfr_ctrl, struct disp_effect_params *effect_params);
int32_t dfr_constant_send_dcs_cmds_with_refresh(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t bl_level);

#endif
