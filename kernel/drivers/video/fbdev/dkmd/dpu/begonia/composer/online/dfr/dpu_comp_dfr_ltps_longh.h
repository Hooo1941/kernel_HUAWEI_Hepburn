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

#ifndef COMPOSER_DFR_LTPS_LONGH_H
#define COMPOSER_DFR_LTPS_LONGH_H

#include "dpu_comp_dfr.h"

int32_t dfr_ltps_longh_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate);
void dfr_ltps_longh_setup_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl);
void dfr_ltps_longh_release_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl);
int32_t dfr_ltps_longh_update_frm_rate_isr_handler(struct dpu_comp_dfr_ctrl *dfr_ctrl);
int32_t dfr_ltps_longh_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl);
#endif
