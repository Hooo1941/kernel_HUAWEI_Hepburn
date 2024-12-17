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
#include "dpu_comp_mgr.h"
#include "dpu_isr.h"

#define HIACE_INT_STAT BIT(0)

static void dpu_dpp0_hiace_interrupt_mask(char __iomem *dpu_base)
{
	/* for dpp, 1 means mask */
	set_reg(DPU_DPP_INT_MSK_ADDR(dpu_base + DPU_DPP0_OFFSET), 1, 1, 0);

	/* for hiace, 0 means mask */
	set_reg(DPU_HIACE_ACE_INT_UNMASK_ADDR(dpu_base + DPU_DPP0_HIACE_OFFSET), 0, 1, 0);
}

static void dpu_dpp0_hiace_interrupt_unmask(char __iomem *dpu_base)
{
	/* for dpp, 0 means unmask */
	set_reg(DPU_DPP_INT_MSK_ADDR(dpu_base + DPU_DPP0_OFFSET), 0, 1, 0);

	/* for hiace, 1 means unmask */
	set_reg(DPU_HIACE_ACE_INT_UNMASK_ADDR(dpu_base + DPU_DPP0_HIACE_OFFSET), 1, 1, 0);
}

static void dpu_dpp0_hiace_interrupt_clear(char __iomem *dpu_base)
{
	/* clear dpp0 int */
	set_reg(DPU_DPP_INTS_ADDR(dpu_base + DPU_DPP0_OFFSET), 1, 1, 0);

	/* clear dpp0_hiace int */
	set_reg(DPU_HIACE_ACE_INT_STAT_ADDR(dpu_base + DPU_DPP0_HIACE_OFFSET), 1, 1, 0);
}

static void dpu_sdp_dpp0_hiace_handler(struct dkmd_isr *isr_ctrl, char __iomem *dpu_base)
{
	uint32_t isr_dpp0_state;
	uint32_t isr_ace_state;

	/* dpp0 int need to clear */
	isr_dpp0_state = inp32(DPU_DPP_INTS_ADDR(dpu_base + DPU_DPP0_OFFSET));
	outp32(DPU_DPP_INTS_ADDR(dpu_base + DPU_DPP0_OFFSET), isr_dpp0_state);

	/* get ace int stat and clear */
	isr_ace_state = inp32(DPU_HIACE_ACE_INT_STAT_ADDR(dpu_base + DPU_DPP0_HIACE_OFFSET));
	outp32(DPU_HIACE_ACE_INT_STAT_ADDR(dpu_base + DPU_DPP0_HIACE_OFFSET), isr_ace_state);

	dpu_pr_debug("isr dpp intr:%#x, isr hiace intr:%#x", isr_dpp0_state, isr_ace_state);

	if (isr_ace_state & HIACE_INT_STAT)
		dkmd_isr_notify_listener(isr_ctrl, DPU_DPP0_HIACE_NS_INT);
}

static void dpu_sdma_interrupt_mask(char __iomem *dpu_base)
{
	/* bit0~13 */
	set_reg(DPU_GLB_SMART_DMA0_NS_INT_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), 0x3FFF, 32, 0);
	set_reg(DPU_GLB_SMART_DMA1_NS_INT_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), 0x3FFF, 32, 0);
	set_reg(DPU_GLB_SMART_DMA2_NS_INT_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), 0x3FFF, 32, 0);
}

static void dpu_sdma_interrupt_unmask(char __iomem *dpu_base)
{
	set_reg(DPU_GLB_SMART_DMA0_NS_INT_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), 0, 32, 0);
	set_reg(DPU_GLB_SMART_DMA1_NS_INT_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), 0, 32, 0);
	set_reg(DPU_GLB_SMART_DMA2_NS_INT_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), 0, 32, 0);
}

static void dpu_sdma_interrupt_clear(char __iomem *dpu_base)
{
	/* bit0~13 */
	outp32(DPU_GLB_SMART_DMA0_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET), 0x3FFF);
	outp32(DPU_GLB_SMART_DMA1_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET), 0x3FFF);
	outp32(DPU_GLB_SMART_DMA2_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET), 0x3FFF);
}

static void dpu_sdp_sdma_handler(char __iomem *dpu_base, uint32_t isr1_state)
{
	uint32_t isr_sdma0_state = 0;
	uint32_t isr_sdma1_state = 0;
	uint32_t isr_sdma2_state = 0;

	if (isr1_state & DPU_SMART_DMA0_NS_INT) {
		isr_sdma0_state = inp32(DPU_GLB_SMART_DMA0_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET));
		outp32(DPU_GLB_SMART_DMA0_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET), isr_sdma0_state);
	}

	if (isr1_state & DPU_SMART_DMA1_NS_INT) {
		isr_sdma1_state = inp32(DPU_GLB_SMART_DMA1_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET));
		outp32(DPU_GLB_SMART_DMA1_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET), isr_sdma1_state);
	}

	if (isr1_state & DPU_SMART_DMA2_NS_INT) {
		isr_sdma2_state = inp32(DPU_GLB_SMART_DMA2_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET));
		outp32(DPU_GLB_SMART_DMA2_NS_INT_O_ADDR(dpu_base + DPU_GLB0_OFFSET), isr_sdma2_state);
	}

	if (isr_sdma0_state != 0 || isr_sdma1_state != 0 || isr_sdma2_state != 0)
		dpu_pr_warn("sdma isr indicate error:sdma0=%#x, sdma1=%#x, sdma2=%#x",
			isr_sdma0_state, isr_sdma1_state, isr_sdma2_state);
}

irqreturn_t dpu_sdp_isr(int32_t irq, void *ptr)
{
	uint32_t isr1_state;
	char __iomem *dpu_base = NULL;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)ptr;
	struct composer_manager *comp_mgr = isr_ctrl->parent;

	dpu_check_and_return(!comp_mgr, IRQ_NONE, err, "comp_mgr is null!");
	dpu_base = comp_mgr->dpu_base;

	isr1_state = inp32(DPU_GLB_NS_SDP_TO_GIC_O_ADDR(dpu_base + DPU_GLB0_OFFSET));
	dpu_pr_debug("isr1 intr:%#x", isr1_state);

	if (isr1_state & DPU_DPP0_HIACE_NS_INT)
		dpu_sdp_dpp0_hiace_handler(isr_ctrl, dpu_base);

	dpu_sdp_sdma_handler(dpu_base, isr1_state);

	/* must clear sdp 1st level isr at last */
	outp32(DPU_GLB_NS_SDP_TO_GIC_O_ADDR(dpu_base + DPU_GLB0_OFFSET), isr1_state);

	return IRQ_HANDLED;
}

void dkmd_sdp_isr_enable(struct composer_manager *comp_mgr)
{
	uint32_t mask = ~0;
	char __iomem *dpu_base = comp_mgr->dpu_base;
	struct dkmd_isr *isr_ctrl = &comp_mgr->sdp_isr_ctrl;

	dpu_pr_info("+");

	/* 1. interrupt mask */
	outp32(DPU_GLB_NS_SDP_TO_GIC_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), mask);
	dpu_dpp0_hiace_interrupt_mask(dpu_base);
	dpu_sdma_interrupt_mask(dpu_base);

	/* 2. enable irq */
	isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_ENABLE);

	/* 3. interrupt clear */
	outp32(DPU_GLB_NS_SDP_TO_GIC_O_ADDR(dpu_base + DPU_GLB0_OFFSET), mask);
	dpu_dpp0_hiace_interrupt_clear(dpu_base);
	dpu_sdma_interrupt_clear(dpu_base);

	/* 4. interrupt umask */
	dpu_pr_info("unmask 0x%x\n", isr_ctrl->unmask);
	outp32(DPU_GLB_NS_SDP_TO_GIC_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), isr_ctrl->unmask);
	dpu_dpp0_hiace_interrupt_unmask(dpu_base);
	dpu_sdma_interrupt_unmask(dpu_base);

	dpu_pr_info("-");
}

void dkmd_sdp_isr_disable(struct composer_manager *comp_mgr)
{
	uint32_t mask = ~0;
	char __iomem *dpu_base = comp_mgr->dpu_base;
	struct dkmd_isr *isr_ctrl = &comp_mgr->sdp_isr_ctrl;

	dpu_pr_info("+");

	/* 1. interrupt mask */
	outp32(DPU_GLB_NS_SDP_TO_GIC_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), mask);
	dpu_dpp0_hiace_interrupt_mask(dpu_base);
	dpu_sdma_interrupt_mask(dpu_base);

	/* 2. disable irq */
	isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_DISABLE);

	dpu_pr_info("-");
}
