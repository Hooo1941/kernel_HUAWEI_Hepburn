/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_alias.h"

#include <linux/slab.h>
#include <linux/uaccess.h>

#include "hvgr_mem_zone.h"
#include "hvgr_mem_opts.h"
#include "hvgr_mem_api.h"
#include "hvgr_mmu_api.h"
#include "hvgr_log_api.h"
#include "hvgr_kv.h"
#include "hvgr_mem_interval_tree.h"

static bool hvgr_mem_alias_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_area *ref_area = NULL;
	uint32_t i;

	hvgr_mem_unmap_gva(area);
	(void)hvgr_mem_free_va(area);

	if (area->alias.count == 0 || area->alias.info == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alias free, alias info error, %u",
			ctx->id, area->alias.count);
		return true;
	}

	for (i = 0; i < area->alias.count; i++) {
		ref_area = area->alias.areas[i];
		if (ref_area == NULL) {
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u alias free, can not find ref area, %u, 0x%lx",
				ctx->id, i, area->alias.info[i].gva);
			continue;
		}

		area->pages -= area->alias.info[i].pages;
		if (!ref_area->freed) {
			atomic_dec(&ref_area->ref_cnt);
			continue;
		}

		if (atomic_dec_and_test(&ref_area->ref_cnt)) {
			(void)hvgr_mem_free_pages(ref_area);
			hvgr_mem_delete_area(ref_area);
		}
	}
	/* alias area don't have real physical page */
	if (area->pages != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alias free, area page not empty, %lu, 0x%lx",
			ctx->id, area->pages, area->gva);
		area->pages = 0;
	}
	(void)hvgr_mem_free_pages(area);
	kvfree(area->alias.info);
	return true;
}

static const struct hvgr_zone_opts g_mem_zone_alias = {
	.init = hvgr_mem_opt_init,
	.term = hvgr_mem_opt_term,
	.check_para = hvgr_mem_opt_check_para,
	.alloc = hvgr_mem_opt_alloc,
	.free = hvgr_mem_alias_free,
};

bool hvgr_mem_alias_config(struct hvgr_mem_ctx * const pmem)
{
	if (pmem == NULL)
		return false;

	if (unlikely(!hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_ALIAS,
		&g_mem_zone_alias, NULL)))
		return false;

	return true;
}

static int hvgr_mem_alias_setup_pagearray(struct hvgr_ctx * const ctx,
	struct hvgr_mem_area * const area, struct hvgr_mem_area *aliasing_area,
	struct hvgr_mem_alias_info *info)
{
	uint64_t offset = info->offset;
	uint64_t pages = info->pages;
	uint64_t i;

	if (pages == 0 || aliasing_area->pages == 0 ||
		area->pages > (U64_MAX - pages) ||
		offset > (U64_MAX - pages) ||
		(area->pages + pages) > area->max_pages ||
		(offset + pages) > aliasing_area->pages ||
		((area->attribute & HVGR_MEM_ATTR_COHERENT_SYSTEM) !=
			(aliasing_area->attribute & HVGR_MEM_ATTR_COHERENT_SYSTEM))) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u aliased data error, %lu, %lu, %lu, %lu, %lu 0x%lx, 0x%lx",
			ctx->id, offset, pages, aliasing_area->pages, area->pages, area->max_pages,
			aliasing_area->attribute, area->attribute);
		return -EFAULT;
	}
	for (i = 0; i < pages; i++) {
		if (aliasing_area->page_array[offset + i] != NULL) {
			area->page_array[area->pages + i] =
				aliasing_area->page_array[offset + i];
			continue;
		}
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u aliased area's page not alloced, 0x%lx, %lu",
			ctx->id, info->gva, i);
		return -EFAULT;
	}
	atomic_inc(&aliasing_area->ref_cnt);
	area->pages += pages;

	return 0;
}

static int hvgr_mem_alias_setup_map(struct hvgr_ctx * const ctx,
	struct hvgr_mem_area * const area, struct hvgr_mem_alias_info *info, uint64_t count)
{
	struct hvgr_mem_area *aliasing_area = NULL;
	uint64_t i;
	uint64_t va_offset = 0;
	long ret;

	for (i = 0; i < count; i++) {
		/* Find the area to be aliased in area_rec */
		aliasing_area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, info[i].gva);
		if (aliasing_area == NULL) {
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u aliased area not find, %lu, %lu, 0x%lx",
				ctx->id, count, i, info[i].gva);
			goto fail;
		}
		if (aliasing_area->property != HVGR_MEM_PROPERTY_GPU_MEM &&
			aliasing_area->property != HVGR_MEM_PROPERTY_SAME_VA) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u aliasing property is %lu",
				ctx->id, aliasing_area->property);
			goto fail;
		}
		if ((area->attribute & HVGR_MEM_MASK_GPU_ACCESS) !=
			(aliasing_area->attribute & HVGR_MEM_MASK_GPU_ACCESS)) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alias gpu attribute is error %lx,%lx",
				ctx->id, area->attribute, aliasing_area->attribute);
			goto fail;
		}
		if (hvgr_mem_alias_setup_pagearray(ctx, area, aliasing_area, info + i) != 0)
			goto fail;

		area->alias.areas[i] = aliasing_area;
	}

	hvgr_mem_attr2flag(area);

	for (i = 0; i < count; i++) {
		struct hvgr_mmu_setup_paras mmu_para = {0};

		mmu_para.gva = area->gva + va_offset;
		mmu_para.page_array = area->page_array + (va_offset >> PAGE_SHIFT);
		mmu_para.pages = info[i].pages;
		mmu_para.flags = area->flags;
		mmu_para.mmu_flag = area->mmu_flag;
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		aliasing_area = area->alias.areas[i];
		mmu_para.mmu_flag &= ~MMU_SC_POLICY_MASK;
		mmu_para.mmu_flag |= (aliasing_area->mmu_flag & MMU_SC_POLICY_MASK);
#endif
		ret = hvgr_mmu_set_up(ctx, &mmu_para);
		if (ret != 0) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u setup alias mmu fail ret=%d",
				ctx->id, ret);
			while (i != 0) {
				i--;
				(void)hvgr_mmu_take_down(ctx,
					area->gva + va_offset - (info[i].pages << PAGE_SHIFT),
					(uint32_t)info[i].pages);
				va_offset -= (info[i].pages << PAGE_SHIFT);
			}
			i = count;
			goto fail;
		}

		va_offset += (info[i].pages << PAGE_SHIFT);
	}

	hvgr_mem_flush_pt(ctx, area->gva, area->pages, WITHOUT_FLUSH_L2);
	return 0;
fail:
	while (i != 0) {
		i--;
		aliasing_area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, info[i].gva);
		if (unlikely(aliasing_area == NULL)) {
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u alias area not find when term, %lu, %lu, 0x%lx",
				ctx->id, count, i, info[i].gva);
			continue;
		}
		atomic_dec(&aliasing_area->ref_cnt);
		area->pages -= info[i].pages;
	}
	return -EFAULT;
}

/*
 * Maximum number of members allowed to create views, this needs to be the
 * same as CMEM_VIEW_MAX_MEMBERS.
 */
#define HVGR_MEM_MAX_ALIAS_MEMBERS ((size_t)(2048 * 6 * 14 + 14))

struct hvgr_mem_area *hvgr_mem_alias(struct hvgr_ctx * const ctx,
	struct hvgr_mem_alias_para * const para, uint64_t *pages, uint64_t *gva)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_alias_info *info = NULL;

	if (para->stride == 0 || para->count == 0 || para->alias_info_ptr == 0 ||
		para->count > HVGR_MEM_MAX_ALIAS_MEMBERS ||
		(para->count > ((U64_MAX / PAGE_SIZE) / para->stride)) ||
		(para->attribute & HVGR_MEM_MASK_GPU_ACCESS) == 0 ||
		(para->attribute & (HVGR_MEM_MASK_UMD_ACCESS | HVGR_MEM_MASK_KMD_ACCESS)) != 0 ||
		(para->attribute & HVGR_MEM_ATTR_GPU_EXEC) != 0 ||
		(para->attribute & HVGR_MEM_ATTR_GROWABLE) != 0 ||
		(para->attribute & HVGR_MEM_ATTR_SECURE) != 0 ||
		(para->attribute & HVGR_MEM_ATTR_MEM_SPARSE) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alias para error, %lu, %lu, 0x%lx, 0x%lx",
			ctx->id, para->stride, para->count, para->alias_info_ptr, para->attribute);
		return NULL;
	}
	info = kvzalloc((sizeof(*info) + sizeof(struct hvgr_mem_area *)) * para->count, GFP_KERNEL);
	if (info == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alias malloc info fail, %lu",
			ctx->id, para->count);
		return NULL;
	}

	if (copy_from_user(info, u64_to_user_ptr(para->alias_info_ptr),
		sizeof(*info) * para->count)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alias copy info fail, %lu",
			ctx->id, para->count);
		kvfree(info);
		return NULL;
	}

	mutex_lock(&ctx->mem_ctx.area_mutex);
	area = hvgr_mem_new_area_with_va(ctx, HVGR_MEM_PROPERTY_ALIAS, para->attribute,
		para->count * para->stride);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alias get area fail. %lu, %lu, 0x%lx, 0x%lx",
			ctx->id, para->stride, para->count, para->alias_info_ptr, para->attribute);
		kvfree(info);
		goto out;
	}

	area->alias.info = info;
	area->alias.areas = (struct hvgr_mem_area **)(info + para->count);
	area->alias.count = (uint32_t)para->count;

	if (hvgr_mem_alias_setup_map(ctx, area, info, para->count) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alias set page fail. %lu, %lu, 0x%lx, 0x%lx",
			ctx->id, para->stride, para->count, para->alias_info_ptr, para->attribute);
		hvgr_kv_del(&ctx->mem_ctx.area_rec, area->node.key);
		hvgr_mem_uva_range_remove(ctx, area);
		(void)hvgr_mem_free_va(area);
		(void)hvgr_mem_free_pages(area);
		hvgr_mem_delete_area(area);
		area = NULL;
		kvfree(info);
		goto out;
	}

	area->user_buf = true;
	*pages = area->max_pages;
	*gva = area->gva;
	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u alias mem prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx",
		ctx->id, area->property, area->attribute, area->pages, area->max_pages, area->gva);

out:
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return area;
}
