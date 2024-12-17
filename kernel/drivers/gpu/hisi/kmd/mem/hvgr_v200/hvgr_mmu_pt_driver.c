/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include <securec.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <securec.h>

#include "hvgr_log_api.h"
#include "hvgr_defs.h"
#include "hvgr_regmap.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_mmu_base.h"
#include "hvgr_mmu_entry.h"
#include "hvgr_mmu_pt_driver.h"

/*
 * 48bit VA to MMU.
 *
 * |47  ~  39|38  ~  30|29  ~  21|20  ~  12|11  ~  0|
 * | level0  | level1  | level2  | level3  | offset |
 * | PGD     | PMD     | PMD     | PTE     | offset |
 */
static void hvgr_mmu_free_pmd_entry(struct hvgr_ctx * const ctx, phys_addr_t entry,
	uint32_t level, uint64_t *pt_buf)
{
	uint32_t i;
	union hvgr_mmu_entry *pt = NULL;
	struct page *page = phys_to_page(entry);

	if (level >= MMU_LEVEL_NUM)
		return;

	pt = kmap_atomic(page);
	if (unlikely(pt == NULL))
		return;

	if (memcpy_s(pt_buf, (size_t)((MMU_LEVEL_NUM - level) * PAGE_SIZE), pt, PAGE_SIZE) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u MMU memcpy fail", ctx->id);
		kunmap_atomic(pt);
		return;
	}
	kunmap_atomic(pt);

	pt = (union hvgr_mmu_entry *)pt_buf;

	for (i = 0; i < MMU_ENTRY_NUM; i++) {
		if (hvgr_mmu_entry_is_pmd(&pt[i]))
			hvgr_mmu_free_pmd_entry(ctx, entry_to_phys(&pt[i]), level + 1,
				(pt_buf + MMU_ENTRY_NUM));
	}

	hvgr_mmu_free_entry(ctx, entry);
}

void hvgr_mmu_free_pgd(struct hvgr_ctx * const ctx)
{
	hvgr_mmu_free_pmd_entry(ctx, ctx->mem_ctx.pgd_pa, MMU_LEVEL_0, ctx->mem_ctx.mmu_buf);
}

static bool hvrg_mmu_pte_is_invalid(union hvgr_mmu_entry *pt, uint32_t idx, uint32_t count)
{
	uint32_t i;

	/* check 0 ~ idx - 1 is invalid */
	for (i = 0; i < idx; i++) {
		/* value 0x2 is invalid entry */
		if (pt[i].value != 0x2)
			return false;
	}

	/* check idx + count ~ MMU_ENTRY_NUM is invalid */
	for (i = (idx + count); i < MMU_ENTRY_NUM; i++) {
		/* value 0x2 is invalid entry */
		if (pt[i].value != 0x2)
			return false;
	}

	return true;
}

long hvgr_mmu_invalid_entry(struct hvgr_ctx * const ctx, uint64_t vpfn, uint32_t *pcnt)
{
	phys_addr_t entry = ctx->mem_ctx.pgd_pa;
	union hvgr_mmu_entry *pt = NULL;
	uint32_t idx;
	uint32_t count;
	uint32_t i;
	struct page *page = NULL;
	uint32_t last_idx;
	struct page *last_page = NULL;

	for (i = MMU_LEVEL_0; i <= MMU_LEVEL_3; i++) {
		idx = (uint32_t)mmu_get_level_idx(vpfn, i);
		page = phys_to_page(entry);
		pt = kmap(page);
		if (unlikely(pt == NULL))
			return -EFAULT;

		if (hvgr_mmu_entry_is_pte(&pt[idx]))
			break;

		if (!hvgr_mmu_entry_is_pmd(&pt[idx])) {
			*pcnt = 1U << mmu_level_offset(i);
			goto exit;
		}

		entry = entry_to_phys(&pt[idx]);
		kunmap(page);

		last_page = page;
		last_idx = idx;
	}

	if (i == MMU_LEVEL_3) {
		count = *pcnt;
	} else if (i == MMU_LEVEL_2) {
		if (*pcnt < MMU_ENTRY_NUM)
			goto exit;

		count = 1;
	} else {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u pfn 0x%lx cant be at level%u.",
			ctx->id, vpfn, i);
		kunmap(page);
		return -EFAULT;
	}

	if (count == MMU_ENTRY_NUM || hvrg_mmu_pte_is_invalid(pt, idx, count)) {
		kunmap(page);
		hvgr_mmu_free_entry(ctx, entry);
		ctx->gdev->mem_dev.statics.mmu_free++;

		pt = kmap(last_page);
		if (unlikely(pt == NULL))
			return -EFAULT;

		if (hvgr_mmu_entry_is_pmd(&pt[last_idx])) {
			hvgr_mmu_entry_invalid(&pt[last_idx]);
			hvgr_mmu_sync_pte(ctx, last_page, (last_idx * MMU_ENTRY_SIZE),
				MMU_ENTRY_SIZE);
		}
		kunmap(last_page);
		return 0;
	}

	for (i = 0; i < count; i++)
		hvgr_mmu_entry_invalid(&pt[idx + i]);

	hvgr_mmu_sync_pte(ctx, page, (idx * MMU_ENTRY_SIZE),
		(size_t)(count * MMU_ENTRY_SIZE));
exit:
	kunmap(page);

	return 0;
}

static phys_addr_t hvgr_mmu_get_next_entry(struct hvgr_ctx * const ctx,
	phys_addr_t entry, uint64_t vpfn, uint32_t level)
{
	uint64_t next_entry = 0;
	uint32_t idx = (uint32_t)mmu_get_level_idx(vpfn, level);
	union hvgr_mmu_entry *pt = NULL;
	union hvgr_mmu_entry *tar_pt = NULL;
	struct page *page = phys_to_page(entry);

	pt = kmap(page);
	if (unlikely(pt == NULL))
		return 0;

	tar_pt = &pt[idx];

	if (!hvgr_mmu_entry_is_pmd(tar_pt)) {
		next_entry = hvgr_mmu_alloc_entry(ctx);
		if (next_entry == 0) {
			kunmap(page);
			return ENOMEM;
		}

		hvgr_mmu_entry_set_pmd(tar_pt, next_entry);

		hvgr_mmu_sync_pte(ctx, page, 0, PAGE_SIZE);
	} else {
		next_entry = entry_to_phys(tar_pt);
	}

	kunmap(page);
	return next_entry;
}

static phys_addr_t hvgr_mmu_get_entry(struct hvgr_ctx * const ctx, uint64_t vpfn,
	uint32_t level)
{
	uint32_t i;
	phys_addr_t entry = ctx->mem_ctx.pgd_pa;

	for (i = MMU_LEVEL_0; i < level; i++) {
		entry = hvgr_mmu_get_next_entry(ctx, entry, vpfn, i);
		if (entry == 0 || entry == ENOMEM)
			return entry;
	}

	return entry;
}

long hvgr_mmu_insert_entry(struct hvgr_ctx * const ctx, struct hvgr_mmu_setup_paras *paras)
{
	uint32_t idx = (uint32_t)(paras->gva & MMU_PTE_MASK);
	uint32_t cur_level = MMU_LEVEL_3;
	phys_addr_t entry = 0;
	union hvgr_mmu_entry *pt = NULL;
	uint32_t i;
	struct page *page = NULL;
	long ret;
	union hvgr_mmu_entry mmu_entry;
	uint32_t huge_idx;

	if ((paras->flags & HVGR_MEM_AREA_FLAG_2M) != 0) {
		cur_level = MMU_LEVEL_2;
		if (idx != 0)
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M vpfn 0x%lx unaligned",
			ctx->id, paras->gva);
	}

	do {
		entry = hvgr_mmu_get_entry(ctx, paras->gva, cur_level);
		if (entry != ENOMEM)
			break;

		mutex_unlock(&ctx->mem_ctx.mmu_mutex);
		ret = hvgr_mmu_grow_pool(ctx, MMU_LEVEL_3);
		mutex_lock(&ctx->mem_ctx.mmu_mutex);
	} while (ret == 0);

	if (entry == 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u err in level%u", ctx->id, cur_level);
		return -ENOMEM;
	}

	page = phys_to_page(entry);
	pt = kmap(page);

	if (cur_level == MMU_LEVEL_2) {
		huge_idx = (uint32_t)mmu_get_level_idx(paras->gva, MMU_LEVEL_2);
		mmu_entry = pt[huge_idx];
		hvgr_mmu_entry_set_pte(&pt[huge_idx], paras->page_array[0], paras->mmu_flag,
			paras->flags);
		hvgr_mmu_sync_pte(ctx, page, (idx * MMU_ENTRY_SIZE), (paras->pages * MMU_ENTRY_SIZE));
		if (hvgr_mmu_entry_is_pmd(&mmu_entry))
			hvgr_mmu_free_entry(ctx, entry_to_phys(&mmu_entry));
	} else {
		for (i = 0; i < paras->pages; i++)
			hvgr_mmu_entry_set_pte(&pt[idx + i], paras->page_array[i], paras->mmu_flag,
				paras->flags);
		hvgr_mmu_sync_pte(ctx, page, (idx * MMU_ENTRY_SIZE), (paras->pages * MMU_ENTRY_SIZE));
	}

	kunmap(page);

	return 0;
}
