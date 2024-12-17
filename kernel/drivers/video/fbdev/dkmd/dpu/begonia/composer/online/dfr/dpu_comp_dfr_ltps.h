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

#ifndef COMPOSER_DFR_LTPS_H
#define COMPOSER_DFR_LTPS_H

#include "dpu_comp_dfr.h"

void dfr_ltps_setup_data(struct dpu_comp_dfr_ctrl *dfr_ctrl);
int32_t dfr_ltps_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate);
int32_t dfr_ltps_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl);
int32_t dfr_ltps_send_dcs_cmds(struct dpu_comp_dfr_ctrl *dfr_ctrl, struct disp_effect_params *effect_params);
int32_t dfr_ltps_update_frm_rate_isr_handler(struct dpu_comp_dfr_ctrl *dfr_ctrl);

#endif
