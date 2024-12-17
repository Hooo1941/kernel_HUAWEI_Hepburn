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

#ifndef DPU_ISR_DVFS_H
#define DPU_ISR_DVFS_H

#include "dkmd_isr.h"

struct composer_manager;

irqreturn_t dpu_dvfs_isr(int32_t irq, void *ptr);
void dkmd_dvfs_isr_enable(struct composer_manager *comp_mgr);
void dkmd_dvfs_isr_disable(struct composer_manager *comp_mgr);

#endif