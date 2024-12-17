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

#ifndef DUP_COMP_DFR_TE_SKIP_ACPU_H
#define DUP_COMP_DFR_TE_SKIP_ACPU_H

#include "dpu_comp_dfr.h"

void dfr_te_skip_acpu_setup_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl);
void dfr_te_skip_acpu_release_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl);
int32_t dfr_te_skip_acpu_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate);
int32_t dfr_te_skip_acpu_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl);
int32_t dfr_te_skip_acpu_set_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate);
void dfr_te_skip_acpu_exit_idle_status(struct dpu_comp_dfr_ctrl *dfr_ctrl);
#endif
