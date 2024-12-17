/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_datan_ct_hal.h"

#include "hvgr_dm_api.h"
#include "hvgr_regmap.h"

void hvgr_datan_ct_open_config(struct hvgr_device *gdev, struct hvgr_ctx *ctx,
	struct hvgr_datan_ct_inner_config *config)
{
	u32 prfcnt_config;
	u32 dump_mode;
	u32 as_nr = ctx->asid; // Address Space ID

	prfcnt_config = as_nr << PRFCNT_CONFIG_AS_SHIFT;
	if (config->set_id < PRFCNT_CONFIG_MAXSET_NUM)
		prfcnt_config |= config->set_id << PRFCNT_CONFIG_SETSELECT_SHIFT;

	dump_mode = config->dump_mode;

	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_BASE_LO), config->dump_buffer_base_low);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_BASE_HI), config->dump_buffer_base_high);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_JM_EN), config->fcm_enable);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_GPC_EN), config->gpc_enable);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_MMU_L2_EN), config->mmu_enable);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_BTC_EN), config->btc_enable);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_BVH_EN), config->bvh_enable);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_CONFIG), prfcnt_config | dump_mode);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND), GPU_COMMAND_PRFCNT_CONFIG);
}

void hvgr_datan_ct_close_config(struct hvgr_device *gdev)
{
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_JM_EN), 0);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_GPC_EN), 0);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_MMU_L2_EN), 0);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_BTC_EN), 0);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_BVH_EN), 0);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PRFCNT_CONFIG), 0);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND), GPU_COMMAND_PRFCNT_CONFIG);
}

void hvgr_datan_ct_dump_data(struct hvgr_device *gdev)
{
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND), GPU_COMMAND_PRFCNT_SAMPLE);
}

void hvgr_datan_ct_irq_enable(struct hvgr_device *gdev)
{
	u32 irq_mask;

	irq_mask = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_MASK));
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_MASK), irq_mask | PRFCNT_SAMPLE_COMPLETED);
}

void hvgr_datan_ct_irq_disable(struct hvgr_device *gdev)
{
	u32 irq_mask;

	irq_mask = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_MASK));
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_MASK), irq_mask & ~PRFCNT_SAMPLE_COMPLETED);
}
