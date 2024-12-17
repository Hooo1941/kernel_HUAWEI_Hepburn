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

#ifndef DPU_COMP_FRAME_START_END_H
#define DPU_COMP_FRAME_START_END_H

#include <linux/types.h>
#include "dkmd_isr.h"

struct dpu_composer;

void dpu_comp_frame_start_isr_init(struct dkmd_isr *isr, struct dpu_composer *dpu_comp, uint32_t listening_bit);
void dpu_comp_frame_start_isr_deinit(struct dkmd_isr *isr, uint32_t listening_bit);
void dpu_comp_frame_end_isr_init(struct dkmd_isr *isr, struct dpu_composer *dpu_comp, uint32_t listening_bit);
void dpu_comp_frame_end_isr_deinit(struct dkmd_isr *isr, uint32_t listening_bit);

#endif