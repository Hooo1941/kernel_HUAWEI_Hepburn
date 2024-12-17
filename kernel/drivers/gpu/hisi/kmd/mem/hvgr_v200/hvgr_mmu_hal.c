/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include <linux/kernel.h>
#include <linux/bitops.h>

#include "hvgr_regmap.h"
#include "hvgr_kmd_defs.h"
#include "hvgr_dm_api.h"
#include "hvgr_mmu_base.h"
#include "hvgr_mmu_struct.h"
#include "hvgr_mmu_driver.h"
#include "dm/fcp/hvgr_fcp.h"
#include "hvgr_mem_api.h"
#include "hvgr_mmu_api.h"
#include "hvgr_log_api.h"
#include "hvgr_datan_api.h"

long hvgr_mmu_hal_wait_cmd_done(struct hvgr_ctx *ctx, uint32_t asid)
{
	struct hvgr_device * const gdev = ctx->gdev;
	uint32_t max_loops = 100000;
	uint32_t done;

	done = hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CMD_DONE));

	while (--max_loops && (done != AS_MMU_COMMAND_DONE))
		done = hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CMD_DONE));
	if (max_loops == 0) {
		hvgr_err(gdev, HVGR_MEM, "ctx_%u mmu as%u wait cmd done timeout, val 0x%x",
			ctx->id, asid, done);
		return -1;
	}

	return 0;
}

long hvgr_mmu_hal_command(struct hvgr_ctx *ctx, uint32_t asid, uint32_t cmd)
{
	struct hvgr_device * const gdev = ctx->gdev;
	long ret;

	ret = hvgr_mmu_hal_wait_cmd_done(ctx, asid);
	if (ret == 0) {
		hvgr_write_reg(gdev, mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_COMMAND)),
			cmd);
		hvgr_datan_jobtrace(ctx, JOBTRACE_TYPE_OBJ, KDATAN_TL_WRITE_REG, ctx,
			mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_COMMAND)), cmd);
	}
	else {
		hvgr_err(gdev, HVGR_MEM, "ctx_%u mmu as%u cmd%u fail", ctx->id, asid, cmd);
	}
	return ret;
}

long hvgr_mmu_hal_unlock(struct hvgr_ctx *ctx, uint32_t asid)
{
	unsigned long flags;
	long ret;
	struct hvgr_device * const gdev = ctx->gdev;

	spin_lock_irqsave(&gdev->mem_dev.mmu_hw_lock, flags);

	ret = hvgr_mmu_hal_command(ctx, asid, AS_COMMAND_UNLOCK);

	spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);
	return ret;
}

long hvgr_mmu_hal_update(struct hvgr_ctx *ctx, struct hvgr_mmu_cfg *mmu_cfg)
{
	struct hvgr_device * const gdev = ctx->gdev;
	long ret;
	unsigned long flags;

	spin_lock_irqsave(&gdev->mem_dev.mmu_hw_lock, flags);
	/* 1.transcfg */
	hvgr_mmu_hal_cfg_transcfg(ctx, &mmu_cfg->transcfg);

	/* 2.transtab */
	hvgr_write_reg(gdev, mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_TRANSTAB_LO)),
		u64_low_32bit(mmu_cfg->transtab));
	hvgr_write_reg(gdev, mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_TRANSTAB_HI)),
		u64_hi_32bit(mmu_cfg->transtab));

	/* 3.memattr */
	hvgr_write_reg(gdev, mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_MEMATTR_LO)),
		u64_low_32bit(mmu_cfg->memattr));
	hvgr_write_reg(gdev, mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_MEMATTR_HI)),
		u64_hi_32bit(mmu_cfg->memattr));

	hvgr_datan_jobtrace(ctx, JOBTRACE_TYPE_OBJ, KDATAN_TL_ATTRIB_AS_CONFIG,
		ctx, ctx->asid, mmu_cfg->transtab, mmu_cfg->memattr, mmu_cfg->transcfg);

	/* 4.update command */
	ret = hvgr_mmu_hal_command(ctx, ctx->asid, AS_COMMAND_UPDATE);
	if (ret != 0) {
		spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);
		hvgr_err(gdev, HVGR_MEM, "ctx_%u mmu update fail ret=%d", ctx->id, ret);
		return ret;
	}

	ret = hvgr_mmu_hal_wait_cmd_done(ctx, ctx->asid);
	spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);
	return ret;
}

long hvgr_mmu_hal_enable(struct hvgr_ctx *ctx)
{
	struct hvgr_mmu_cfg mmu_cfg = {0};

	hvgr_mmu_hal_get_enable_cfg(ctx, &mmu_cfg);
	return hvgr_mmu_hal_update(ctx, &mmu_cfg);
}

long hvgr_mmu_hal_flush_pt(struct hvgr_ctx *ctx, uint64_t gva, uint32_t pages)
{
	unsigned long flags;
	long ret;
	struct hvgr_device * const gdev = ctx->gdev;

	spin_lock_irqsave(&gdev->mem_dev.mmu_hw_lock, flags);

	hvgr_mmu_hal_cfg_lock(ctx, gva, pages);

	(void)hvgr_mmu_hal_command(ctx, ctx->asid, AS_COMMAND_LOCK);

	(void)hvgr_mmu_hal_command(ctx, ctx->asid, AS_COMMAND_UNLOCK);

	ret = hvgr_mmu_hal_wait_cmd_done(ctx, ctx->asid);
	spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);

	return ret;
}

static void hvgr_mmu_hal_flush_invalid(struct hvgr_ctx *ctx)
{
	long ret;

	ret = hvgr_mmu_hal_flush_pt(ctx, (uint64_t)0, ~(uint32_t)0);
	if (ret != 0)
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mmu flush invalid as%u fail. Gpu reset",
			ctx->id, ctx->asid);
		/* gpu reset */
}

long hvgr_mmu_hal_disable(struct hvgr_ctx *ctx)
{
	struct hvgr_mmu_cfg mmu_cfg = {0};

	hvgr_mmu_hal_flush_invalid(ctx);
	hvgr_mmu_hal_get_disable_cfg(&mmu_cfg);

	return hvgr_mmu_hal_update(ctx, &mmu_cfg);
}

void hvgr_mmu_cbit_config(struct hvgr_device *gdev)
{
	uint32_t reg_value;

	reg_value = hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CONFIG));
	hvgr_mmu_hal_get_mode_cfg(gdev, &reg_value);
	hvgr_write_reg(gdev, mmu_control_reg(gdev, MMU_CONFIG), reg_value);
}

void hvgr_mmu_flush_l2_caches(struct hvgr_device *gdev)
{
	struct hvgr_ctx *ctx = gdev->dm_dev.fcp_data->ctx;
	unsigned long flags;

	if (!hvgr_mmu_flush_l2_is_needed())
		return;

	if (ctx == NULL) {
		hvgr_err(gdev, HVGR_MEM, "mmu flush L2 ctx is null.");
		return;
	}

	spin_lock_irqsave(&gdev->mem_dev.mmu_hw_lock, flags);
	/* Lock one page */
	ctx->asid = 7;
	hvgr_mmu_hal_cfg_lock(ctx, 0ULL, 1U);

	(void)hvgr_mmu_hal_command(ctx, 7, AS_COMMAND_LOCK);
	if (hvgr_mmu_hal_wait_cmd_done(ctx, 7) != 0) {
		spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);
		hvgr_err(gdev, HVGR_MEM, "ctx_%u mmu as7 command lock failed.", ctx->id);
		return;
	}

	(void)hvgr_mmu_hal_command(ctx, 7, AS_COMMAND_FLUSH_PT);
	if (hvgr_mmu_hal_wait_cmd_done(ctx, 7) != 0)
		hvgr_err(gdev, HVGR_MEM, "ctx_%u mmu as7 command flush pt failed.", ctx->id);

	spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);
}

