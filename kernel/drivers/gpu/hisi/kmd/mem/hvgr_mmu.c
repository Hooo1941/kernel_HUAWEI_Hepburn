/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mmu_api.h"

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <securec.h>

#include "hvgr_regmap.h"
#include "hvgr_mem_api.h"
#include "hvgr_mmu_struct.h"
#include "hvgr_mmu_driver.h"
#include "hvgr_mmu_entry.h"
#include "hvgr_mmu_base.h"
#include "hvgr_mmu_pt_driver.h"
#include "hvgr_log_api.h"

static void hvgr_mmu_revert_insert(struct hvgr_ctx * const ctx, uint64_t vpfn_start,
	uint64_t vpfn_end)
{
	while (vpfn_start < vpfn_end) {
		uint32_t cnt = MMU_ENTRY_NUM - (uint32_t)(vpfn_start & MMU_PTE_MASK);

		if (cnt > (uint32_t)(vpfn_end - vpfn_start))
			cnt = (uint32_t)(vpfn_end - vpfn_start);

		(void)hvgr_mmu_invalid_entry(ctx, vpfn_start, &cnt);

		if (cnt > (uint32_t)(vpfn_end - vpfn_start))
			break;
		vpfn_start += cnt;
	}
}

long hvgr_mmu_set_up(struct hvgr_ctx *ctx, struct hvgr_mmu_setup_paras *paras)
{
	uint64_t left_pages = paras->pages;
	uint32_t cnt;
	uint64_t vpfn;
	uint64_t cur_vpfn;
	long ret = 0;
	struct hvgr_mmu_setup_paras tmp_para = {0};

	if (ctx == NULL)
		return -1;

	mutex_lock(&ctx->mem_ctx.mmu_mutex);

	vpfn = va_to_vpfn(paras->gva);
	cur_vpfn = vpfn;
	tmp_para.mmu_flag = paras->mmu_flag;
	tmp_para.flags = paras->flags;

	while (left_pages != 0) {
		cnt = MMU_ENTRY_NUM - (uint32_t)(cur_vpfn & MMU_PTE_MASK);
		if (cnt > left_pages)
			cnt = (uint32_t)left_pages;

		tmp_para.gva = cur_vpfn;
		tmp_para.page_array = paras->page_array;
		tmp_para.pages = cnt;

		ret = hvgr_mmu_insert_entry(ctx, &tmp_para);
		if (ret != 0) {
			if (left_pages != paras->pages)
				hvgr_mmu_revert_insert(ctx, vpfn, cur_vpfn);

			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u pfn 0x%lx/0x%lx insert %u entry fail.",
				ctx->id, vpfn, cur_vpfn, cnt);
			break;
		}

		left_pages -= cnt;
		cur_vpfn += cnt;
		paras->page_array += cnt;
	}

	mutex_unlock(&ctx->mem_ctx.mmu_mutex);

	return ret;
}

long hvgr_mmu_take_down(struct hvgr_ctx * const ctx, uint64_t va, uint32_t pages)
{
	uint32_t left_pages = pages;
	uint32_t cnt;
	uint64_t vpfn = va_to_vpfn(va);
	uint64_t cur_vpfn = vpfn;
	long ret = 0;

	if (ctx == NULL)
		return -1;

	mutex_lock(&ctx->mem_ctx.mmu_mutex);

	while (left_pages != 0) {
		cnt = MMU_ENTRY_NUM - (uint32_t)(cur_vpfn & MMU_PTE_MASK);
		if (cnt > left_pages)
			cnt = left_pages;

		ret = hvgr_mmu_invalid_entry(ctx, cur_vpfn, &cnt);
		if (ret != 0)
			break;

		if (cnt > left_pages)
			break;

		cur_vpfn += cnt;
		left_pages -= cnt;
	}

	mutex_unlock(&ctx->mem_ctx.mmu_mutex);

	return ret;
}

long hvgr_mmu_init_proc(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_ctx *pmem = &ctx->mem_ctx;
	long ret;

	mutex_lock(&pmem->mmu_mutex);
	pmem->mmu_buf = vzalloc(PAGE_SIZE * MMU_LEVEL_NUM);
	if (unlikely(pmem->mmu_buf == NULL)) {
		mutex_unlock(&pmem->mmu_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u MMU alloc buf fail", ctx->id);
		return -1;
	}

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	pmem->sc_data.sc_ps_cfg = ctx->gdev->mem_dev.sc_data.sc_ps_cfg;
	pmem->sc_data.pgd_cache_enable = ctx->gdev->mem_dev.sc_data.pgd_cache_enable;
	pmem->sc_data.pgd_mem_gid = ctx->gdev->mem_dev.sc_data.pgd_mem_gid;
#endif
	mutex_unlock(&pmem->mmu_mutex);

	do {
		ret = hvgr_mmu_grow_pool(ctx, MMU_LEVEL_3);
		if (ret != 0) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u MMU init grow pool fail", ctx->id);
			return -1;
		}

		mutex_lock(&pmem->mmu_mutex);
		pmem->pgd_pa = hvgr_mmu_alloc_entry(ctx);
		mutex_unlock(&pmem->mmu_mutex);
	} while (pmem->pgd_pa == 0u);

	return 0;
}

void hvgr_mmu_term_proc(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_ctx *pmem = &ctx->mem_ctx;

	mutex_lock(&pmem->mmu_mutex);
	if (pmem->pgd_pa != 0) {
		hvgr_mmu_free_pgd(ctx);
		pmem->pgd_pa = 0;
	}

	if (pmem->mmu_buf != NULL) {
		vfree(pmem->mmu_buf);
		pmem->mmu_buf = NULL;
	}
	mutex_unlock(&pmem->mmu_mutex);
}

void hvgr_mmu_flush_tlb(struct hvgr_ctx * const ctx)
{
	(void)hvgr_mmu_hal_enable(ctx);
}
