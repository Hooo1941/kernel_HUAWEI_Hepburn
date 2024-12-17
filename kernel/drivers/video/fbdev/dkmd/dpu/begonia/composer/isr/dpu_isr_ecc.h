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

#ifndef DPU_ISR_ECC_H
#define DPU_ISR_ECC_H

#include "dkmd_isr.h"

struct composer_manager;

enum {
	DPU_TBU_ERR_DETECT_0 = BIT(0),
	DPU_TBU_ERR_DETECT_1 = BIT(1),
};

enum {
	DPU_TBU_ERR_MULTPL_0 = BIT(0),
	DPU_TBU_ERR_MULTPL_1 = BIT(1),
};

#define INTR_HUB_MED1_BASE_OFFSET 0xE02CA000U

#define M1_INTR_TBU_ERR_DETECT_L2_5_DSS 5
#define M1_INTR_TBU_ERR_MULTPL_L2_11_DSS 11

#define SMMU_TBU_MEM_CTRL_S_DR_OFFSET 0x0008
#define SMMU_TBU_MEM_CTRL_S_SR_OFFSET 0x002C
#define SMMU_TBU_MEM_CTRL_S_ECC_ENABLE BIT(25)

#define SMMU_TBU_MEM_CTRL_T_DR_OFFSET 0x0028
#define SMMU_TBU_MEM_CTRL_T_SR_OFFSET 0x0034
#define SMMU_TBU_MEM_CTRL_T_ECC_ENABLE BIT(15)

#define SMMU_TBU_MEM_CTRL_B_DR_OFFSET 0x000C
#define SMMU_TBU_MEM_CTRL_B_SR_OFFSET 0x0030
#define SMMU_TBU_MEM_CTRL_B_ECC_ENABLE BIT(13)

#define SMMU_TBU_IRPT_MASK_NS_OFFSET 0x0010
#define SMMU_TBU_IRPT_MASK_NS_ECC_MSK 4

#define SMMU_TBU_IRPT_CLR_NS_OFFSET 0x001C
#if defined(CONFIG_DPU_TBU_ECC)
irqreturn_t dpu_ecc_detect_isr(int32_t irq, void *ptr);
irqreturn_t dpu_ecc_multpl_isr(int32_t irq, void *ptr);
void dpu_smmu_tbu_ecc_enable(struct composer_manager *comp_mgr);
void dpu_smmu_tbu_ecc_clear_interrupts(struct composer_manager *comp_mgr);
void dpu_smmu_tbu_ecc_enable_and_wait_ready(char __iomem *tbu_base);
#else
static inline irqreturn_t dpu_ecc_detect_isr(int32_t irq, void *ptr) {
	return 0;
}
static inline irqreturn_t dpu_ecc_multpl_isr(int32_t irq, void *ptr) {
	return 0;
}
#define dpu_smmu_tbu_ecc_enable(comp_mgr)
#define dpu_smmu_tbu_ecc_clear_interrupts(comp_mgr)
#define dpu_smmu_tbu_ecc_enable_and_wait_ready(tbu_base)
#endif

#endif