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

#ifndef DPU_ISR_SDP_H
#define DPU_ISR_SDP_H

#include "dkmd_isr.h"

struct composer_manager;

irqreturn_t dpu_sdp_isr(int32_t irq, void *ptr);
void dkmd_sdp_isr_enable(struct composer_manager *comp_mgr);
void dkmd_sdp_isr_disable(struct composer_manager *comp_mgr);
void dkmd_sdp_isr_hiace_enable(struct dpu_composer *dpu_comp);
void dkmd_sdp_isr_hiace_disable(struct dpu_composer *dpu_comp);

#endif
