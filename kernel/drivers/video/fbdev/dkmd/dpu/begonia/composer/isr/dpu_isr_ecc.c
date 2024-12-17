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

#include <linux/of_irq.h>
#include <linux/kernel.h>
#include "dkmd_listener.h"
#include "dkmd_isr.h"
#include "dkmd_log.h"
#include "dpu_isr.h"
#include "dpu_isr_ecc.h"
#include "dpu_comp_mgr.h"
#include <dpu/soc_dpu_define.h>
#include <soc_intr_hub_interface.h>
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>

irqreturn_t dpu_ecc_detect_isr(int32_t irq, void *ptr)
{
	uint32_t isr_level2_intr_status = 0;
	uint32_t mask = 0;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)ptr;

	dpu_pr_info("+");
	dpu_smmu_tbu_ecc_clear_interrupts(isr_ctrl->parent);

	isr_level2_intr_status = inp32(SOC_INTR_HUB_L2_INTR_STATUS_S_n_ADDR(INTR_HUB_MED1_BASE_OFFSET,
									M1_INTR_TBU_ERR_DETECT_L2_5_DSS));
	if ((isr_level2_intr_status & DPU_TBU_ERR_DETECT_0) == DPU_TBU_ERR_DETECT_0) {
		mask |= (0x1 << 0);
		bbox_diaginfo_record(ECC_DSS_NONRDA, NULL, "dpu_tbu 0 ecc 1-bit error");
		dpu_pr_err("DSS detect 1bit: DPU_TBU_ERR_DETECT_0");
	}

	if ((isr_level2_intr_status & DPU_TBU_ERR_DETECT_1) == DPU_TBU_ERR_DETECT_1) {
		mask |= (0x1 << 1);
		bbox_diaginfo_record(ECC_DSS_NONRDA, NULL, "dpu_tbu 1 ecc 1-bit error");
		dpu_pr_err("DSS detect 1bit: DPU_TBU_ERR_DETECT_1");
	}

	outp32(SOC_INTR_HUB_L2_INTR_MASKCLR_S_n_ADDR(INTR_HUB_MED1_BASE_OFFSET, M1_INTR_TBU_ERR_DETECT_L2_5_DSS), mask);
	dpu_pr_info("-");

	return IRQ_HANDLED;
}

irqreturn_t dpu_ecc_multpl_isr(int32_t irq, void *ptr)
{
	uint32_t isr_level2_intr_status = 0;
	uint32_t mask = 0;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)ptr;

	dpu_pr_info("+");
	dpu_smmu_tbu_ecc_clear_interrupts(isr_ctrl->parent);
	isr_level2_intr_status = inp32(SOC_INTR_HUB_L2_INTR_STATUS_S_n_ADDR(INTR_HUB_MED1_BASE_OFFSET,
								   M1_INTR_TBU_ERR_MULTPL_L2_11_DSS));
	if ((isr_level2_intr_status & DPU_TBU_ERR_MULTPL_0) == DPU_TBU_ERR_MULTPL_0) {
		mask |= (0x1 << 0);
		bbox_diaginfo_record(ECC_DSS_NONRDA, NULL, "dpu_tbu 0 ecc 2-bit error");
		dpu_pr_err("DSS detect 2bit: DPU_TBU_ERR_MULTPL_0");
	}

	if ((isr_level2_intr_status & DPU_TBU_ERR_MULTPL_1) == DPU_TBU_ERR_MULTPL_1) {
		mask |= (0x1 << 1);
		bbox_diaginfo_record(ECC_DSS_NONRDA, NULL, "dpu_tbu 1 ecc 2-bit error");
		dpu_pr_err("DSS detect 2bit: DPU_TBU_ERR_MULTPL_1");
	}

	outp32(SOC_INTR_HUB_L2_INTR_MASKCLR_S_n_ADDR(INTR_HUB_MED1_BASE_OFFSET, M1_INTR_TBU_ERR_MULTPL_L2_11_DSS), mask);
	dpu_pr_info("-");

	return IRQ_HANDLED;
}

void dpu_smmu_tbu_ecc_enable(struct composer_manager *comp_mgr)
{
	char __iomem *tbu_base = NULL;
	tbu_base = comp_mgr->dpu_base + DPU_SMMU_OFFSET;

	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_S_DR_OFFSET, 0x01, 1, SMMU_TBU_MEM_CTRL_S_ECC_ENABLE);
	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_S_SR_OFFSET, 0x01, 1, SMMU_TBU_MEM_CTRL_S_ECC_ENABLE);

	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_T_DR_OFFSET, 0x01, 1, SMMU_TBU_MEM_CTRL_T_ECC_ENABLE);
	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_T_SR_OFFSET, 0x01, 1, SMMU_TBU_MEM_CTRL_T_ECC_ENABLE);

	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_B_DR_OFFSET, 0x01, 1, SMMU_TBU_MEM_CTRL_B_ECC_ENABLE);
	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_B_SR_OFFSET, 0x01, 1, SMMU_TBU_MEM_CTRL_B_ECC_ENABLE);

	/* enable ecc 1/multiple bit err interrupts */
	set_reg(tbu_base + SMMU_TBU_IRPT_MASK_NS_OFFSET, 0x0, 2, SMMU_TBU_IRPT_MASK_NS_ECC_MSK);
}

void dpu_smmu_tbu_ecc_clear_interrupts(struct composer_manager *comp_mgr)
{
	char __iomem *tbu_base = NULL;
	tbu_base = comp_mgr->dpu_base + DPU_SMMU_OFFSET;

	set_reg(tbu_base + SMMU_TBU_IRPT_CLR_NS_OFFSET, 0x03, 2, 4);
}

void dpu_smmu_tbu_ecc_enable_and_wait_ready(char __iomem *tbu_base)
{
	uint32_t delay_count = 0;
	uint32_t delay_count_max = 1000; /* Read register status, maximum waiting time is 1ms */
	uint32_t ecc_status = 0;

	if (tbu_base == NULL) {
		dpu_pr_err("tbu_base is null");
		return;
	}

	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_S_DR_OFFSET, 0x01, 1, 25);
	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_S_SR_OFFSET, 0x01, 1, 25);

	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_T_DR_OFFSET, 0x01, 1, 15);
	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_T_SR_OFFSET, 0x01, 1, 15);

	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_B_DR_OFFSET, 0x01, 1, 13);
	set_reg(tbu_base + SMMU_TBU_MEM_CTRL_B_SR_OFFSET, 0x01, 1, 13);

	while ((ecc_status = (((inp32(tbu_base + SMMU_TBU_MEM_CTRL_S_DR_OFFSET) & 0x2000000) &
		(inp32(tbu_base + SMMU_TBU_MEM_CTRL_S_SR_OFFSET) & 0x2000000)) |
		((inp32(tbu_base + SMMU_TBU_MEM_CTRL_T_DR_OFFSET) & 0x8000) &
		(inp32(tbu_base + SMMU_TBU_MEM_CTRL_T_SR_OFFSET) & 0x8000)) |
		((inp32(tbu_base + SMMU_TBU_MEM_CTRL_B_DR_OFFSET) & 0x2000) &
		(inp32(tbu_base + SMMU_TBU_MEM_CTRL_B_SR_OFFSET) & 0x2000)))) != 0x200a000) {
		if (++delay_count > delay_count_max) {
			dpu_pr_warn("wait tbu ecc enable timeout, ecc_status = %#x", ecc_status);
			break;
		}
		udelay(1); /* need sleep 1us. */
	}

	/* enable ecc 1/multiple bit err interrupts */
	set_reg(tbu_base + SMMU_TBU_IRPT_MASK_NS_OFFSET, 0x0, 2, 4);
	dpu_pr_debug("ecc enable");
}