/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_REGISTER_CFG_H
#define HVGR_REGISTER_CFG_H

#include "hvgr_gpu_id.h"

static const struct hvgr_reg_base_cfg product_reg_cfg[] = {
	{
		.gpu_id = GPU_ID2_PRODUCT_TV200,
		.reg_base.gpu_reg_base = GPU_CONTROL_BASE,
		.reg_base.job_reg_base = JOB_CONTROL_BASE,
		.reg_base.mmu_stage1_reg_base = MEMORY_MANAGEMENT_BASE,
		.reg_base.cq_reg_base = CQ_REG_BASE,
		.reg_base.fcp_base.fcp_reg_base = FCP_BASE,
		.reg_base.fcp_base.fcp_itcm_offset = FCP_START_CODE,
		.reg_base.fcp_base.fcp_dtcm_offset = FCP_DTCM_START_ADDR,
		.reg_base.fcp_base.fcp_itcm_cq_offset = FCP_ITCM_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_dtcm0_cq_offset = FCP_DTCM0_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_dtcm1_cq_offset = FCP_DTCM1_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_reg_mb_offset = FCP_MB_MCU_KEY,
		.reg_base.fcp_base.fcp_reg_dma_offset = FCP_DMA_CHANEL_BASE,
		.reg_base.fcp_base.fcp_reg_timer_offset = FCP_TIMER_LOAD_BASE,
		.reg_base.fcp_base.fcp_reg_uart_offset = 0,
	},
	{
		.gpu_id = GPU_ID2_PRODUCT_TV210,
		.reg_base.gpu_reg_base = GPU_CONTROL_BASE,
		.reg_base.job_reg_base = JOB_CONTROL_BASE,
		.reg_base.mmu_stage1_reg_base = MEMORY_MANAGEMENT_BASE,
		.reg_base.cq_reg_base = CQ_REG_BASE,
		.reg_base.fcp_base.fcp_reg_base = FCP_BASE,
		.reg_base.fcp_base.fcp_itcm_offset = FCP_START_CODE,
		.reg_base.fcp_base.fcp_dtcm_offset = FCP_DTCM_START_ADDR,
		.reg_base.fcp_base.fcp_itcm_cq_offset = FCP_ITCM_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_dtcm0_cq_offset = FCP_DTCM0_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_dtcm1_cq_offset = FCP_DTCM1_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_reg_mb_offset = FCP_MB_MCU_KEY,
		.reg_base.fcp_base.fcp_reg_dma_offset = FCP_DMA_CHANEL_BASE,
		.reg_base.fcp_base.fcp_reg_timer_offset = FCP_TIMER_LOAD_BASE,
		.reg_base.fcp_base.fcp_reg_uart_offset = 0,
	},
	{
		.gpu_id = GPU_ID2_PRODUCT_TV211,
		.reg_base.gpu_reg_base = GPU_CONTROL_BASE,
		.reg_base.job_reg_base = JOB_CONTROL_BASE,
		.reg_base.mmu_stage1_reg_base = MEMORY_MANAGEMENT_BASE,
		.reg_base.cq_reg_base = CQ_REG_BASE,
		.reg_base.fcp_base.fcp_reg_base = FCP_BASE,
		.reg_base.fcp_base.fcp_itcm_offset = FCP_START_CODE,
		.reg_base.fcp_base.fcp_dtcm_offset = FCP_DTCM_START_ADDR,
		.reg_base.fcp_base.fcp_itcm_cq_offset = FCP_ITCM_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_dtcm0_cq_offset = FCP_DTCM0_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_dtcm1_cq_offset = FCP_DTCM1_CQ_START_ADDR,
		.reg_base.fcp_base.fcp_reg_mb_offset = FCP_MB_MCU_KEY,
		.reg_base.fcp_base.fcp_reg_dma_offset = FCP_DMA_CHANEL_BASE,
		.reg_base.fcp_base.fcp_reg_timer_offset = FCP_TIMER_LOAD_BASE,
		.reg_base.fcp_base.fcp_reg_uart_offset = 0,
	},
};

#endif
