/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mmu_driver.h"

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <securec.h>

#include "hvgr_regmap.h"
#include "hvgr_dm_api.h"
#include "dm/fcp/hvgr_fcp.h"
#include "hvgr_mem_api.h"
#include "hvgr_mmu_base.h"
#include "hvgr_mmu_entry.h"
#include "hvgr_mmu_struct.h"
#include "hvgr_log_api.h"

#define LOCK_REGION_MAX 63ULL
#define LOCK_REGION_MIN 11ULL

char *hvgr_mmu_other_fault_process(struct hvgr_ctx *ctx, struct hvgr_mmu_as *mmu_as)
{
	char *reset_reason = NULL;
	uint32_t fault_code = (mmu_as->fault_stat >> AS_EXCEPTION_CODE_OFFSET) &
		AS_EXCEPTION_CODE_MASK;

	switch (fault_code) {
	case AS_EXCEPTION_CODE_PERMISSION_FAULT:
		reset_reason = "Permission fault";
		break;
	case AS_EXCEPTION_CODE_TRANSTAB_BUS_FAULT:
		reset_reason = "Translation table bus fault";
		break;
	case AS_EXCEPTION_CODE_ACCESS_FLAG:
		dev_warn(ctx->gdev->dev, "Access flag unexpectedly set");
		break;
	case AS_EXCEPTION_CODE_ADDRESS_SIZE_FAULT:
		reset_reason = "Unknown fault code0";
		break;
	case AS_EXCEPTION_CODE_MEMORY_ATTRIBUTES_FAULT:
		reset_reason = "Unknown fault code1";
		break;
	default:
		reset_reason = "Unknown fault code";
		break;
	}

	return reset_reason;
}

void hvgr_mmu_hal_cfg_transcfg(struct hvgr_ctx *ctx, uint64_t *trans_val)
{
	struct hvgr_device * const gdev = ctx->gdev;
	uint32_t transcfg;

	/* 1.transcfg */
	transcfg = hvgr_read_reg(gdev,
		mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_TRANSCFG_LO)));
	if (hvgr_mem_is_cpu_coherent(gdev)) {
		transcfg &= ~AS_TRANSCFG_PTW_MEMATTR_SHAREABLE_MASK;
		transcfg |= AS_TRANSCFG_PTW_MEMATTR_OUT_SHAREABLE;
	}
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	if (ctx->mem_ctx.sc_data.pgd_mem_gid != 0) {
		transcfg &= ~AS_TRANSCFG_PTW_SC_MASK;
		transcfg |= as_transcfg_ptw_sc_id(ctx->mem_ctx.sc_data.pgd_mem_gid);
	}

	transcfg &= ~AS_TRANSCFG_PTW_MEMATTR_MASK;
	if (ctx->mem_ctx.sc_data.pgd_cache_enable != 0)
		transcfg |= AS_TRANSCFG_PTW_MEMATTR_WRITE_BACK;
	else
		transcfg |= AS_TRANSCFG_PTW_MEMATTR_NON_CACHEABLE;
#endif

	transcfg &= ~AS_TRANSCFG_ADRMODE_MASK;
	transcfg |= (uint32_t)(*trans_val & 0xFFFFFFFFu);

	hvgr_write_reg(gdev, mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_TRANSCFG_LO)),
		transcfg);
	*trans_val = transcfg;
}

static uint64_t hvgr_mmu_hal_get_lock_addr(uint64_t gva, uint32_t pages)
{
	uint64_t lock_addr = gva;
	uint32_t width;

	if (pages == 0) {
		lock_addr |= LOCK_REGION_MIN;
	} else {
		width = (LOCK_REGION_MIN - 1U) + (uint32_t)fls((int)pages);
		if (pages != ((uint32_t)1 << (width - LOCK_REGION_MIN)))
			/* go up to the next pow2 */
			width += 1;

		lock_addr |= width;
	}

	return lock_addr;
}

void hvgr_mmu_hal_cfg_lock(struct hvgr_ctx *ctx, uint64_t gva, uint32_t pages)
{
	struct hvgr_device * const gdev = ctx->gdev;
	uint64_t lock_addr = hvgr_mmu_hal_get_lock_addr(gva, pages);

	hvgr_write_reg(gdev, mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_LOCKADDR_LO)),
		u64_low_32bit(lock_addr));
	hvgr_write_reg(gdev, mmu_control_reg(gdev, mmu_as_reg(ctx->asid, AS_LOCKADDR_HI)),
		u64_hi_32bit(lock_addr));
}

void hvgr_mmu_hal_get_enable_cfg(struct hvgr_ctx *ctx, struct hvgr_mmu_cfg *mmu_cfg)
{
	uint64_t pgd = ptr_to_u64(ctx->mem_ctx.pgd_pa);

	mmu_cfg->transcfg = AS_TRANSCFG_LPAE_READ_INNER | AS_TRANSCFG_ADRMODE_LEGACY_TABLE;
	mmu_cfg->memattr = AS_MEMATTR_LPAE;
	mmu_cfg->transtab = (pgd & (((uint64_t)0xFFFF << 32) | AS_TRANSTAB_LPAE_ADDR_SPACE_MASK));
}

void hvgr_mmu_hal_get_disable_cfg(struct hvgr_mmu_cfg *mmu_cfg)
{
	mmu_cfg->transcfg = AS_TRANSCFG_ADRMODE_UNMAPPED;
	mmu_cfg->memattr = AS_MEMATTR_LPAE;
}

void hvgr_mmu_hal_get_mode_cfg(struct hvgr_device *gdev, uint32_t *value)
{
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	*value |= gdev->mem_dev.sc_data.pbha_mode << MMU_SC_MODE_OFFSET;
#endif
}

int hvgr_mmu_init(struct hvgr_device * const gdev)
{
	return 0;
}

void hvgr_mmu_term(struct hvgr_device * const gdev)
{
	int i;
	struct hvgr_mmu_as *mmu_as = NULL;

	for (i = 0; i < MMU_ADDR_SPACE_MAX; i++) {
		mmu_as = &gdev->mem_dev.mmu_as[i];
		if (IS_ERR_OR_NULL(mmu_as->fault_wq))
			continue;

		destroy_workqueue(mmu_as->fault_wq);
	}
}