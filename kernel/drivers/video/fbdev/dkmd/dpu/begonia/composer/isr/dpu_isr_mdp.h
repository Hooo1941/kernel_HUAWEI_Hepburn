/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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

#ifndef DPU_ISR_MDP_H
#define DPU_ISR_MDP_H

#include "dkmd_isr.h"

struct composer_manager;

irqreturn_t dpu_mdp_isr(int32_t irq, void *ptr);
void dkmd_mdp_isr_enable(struct composer_manager *comp_mgr);
void dkmd_mdp_isr_disable(struct composer_manager *comp_mgr);
void dpu_mdp_dacc_dfr_isr_handler(char __iomem *dpu_base, struct dkmd_isr *isr_ctrl);
void dpu_mdp_dacc_ppc_isr_handler(char __iomem *dpu_base, struct dkmd_isr *isr_ctrl);
#endif
