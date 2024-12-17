/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_api.h"

#include <linux/kernel.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/lockdep.h>

#include "hvgr_kmd_defs.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_mem_zone.h"
#include "hvgr_mem_import.h"
#include "hvgr_log_api.h"
#include "hvgr_mem_interval_tree.h"

static void hvgr_mem_map_area_init(struct hvgr_mem_area * const map_area,
	struct hvgr_mem_area * const src_area,
	struct hvgr_mem_map_para const * const para)
{
	map_area->dep_area = src_area;
	map_area->zone = src_area->zone;
	map_area->property = para->property;
	map_area->attribute = para->attribute;
	map_area->gva = para->gva;
	map_area->pages = para->pages;
	map_area->max_pages = para->pages;
}

long hvgr_mem_map(struct hvgr_ctx * const ctx, struct hvgr_mem_map_para * const para, uint64_t *uva)
{
	struct hvgr_mem_area *map_area = NULL;
	struct hvgr_mem_area *src_area = NULL;
	struct hvgr_mem_ctx *pmem = &ctx->mem_ctx;
	uint32_t page_offset;

	mutex_lock(&pmem->area_mutex);
	src_area = hvgr_mem_zone_find_area_by_gva_range(pmem, para->gva);
	if (src_area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map no area for gva 0x%lx.",
			ctx->id, para->gva);
		goto unlock_out;
	}

#ifndef CONFIG_MEM_UVA_OFFSET
	if (((para->attribute & HVGR_MEM_ATTR_CPU_RD) != 0 &&
		(src_area->attribute & HVGR_MEM_ATTR_CPU_RD) == 0) ||
		((para->attribute & HVGR_MEM_ATTR_CPU_WR) != 0 &&
		(src_area->attribute & HVGR_MEM_ATTR_CPU_WR) == 0)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map cpu access err gva 0x%lx %lu:0x%lx 0x%lx",
			ctx->id, para->gva, src_area->property, src_area->attribute,
			para->attribute);
		goto unlock_out;
	}
#endif
	para->property = src_area->zone->proterty;
	if (src_area->zone->proterty == HVGR_MEM_PROPERTY_JIT ||
		src_area->zone->proterty == HVGR_MEM_PROPERTY_DRIVER) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map jit or driver area gva 0x%lx.",
			ctx->id, para->gva);
		goto unlock_out;
	}

	if (src_area->property == HVGR_MEM_PROPERTY_SPARSE ||
		src_area->property == HVGR_MEM_PROPERTY_ALIAS) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gva 0x%lx property is %lu not allowed map",
			ctx->id, para->gva, src_area->property);
		goto unlock_out;
	}
	if ((src_area->attribute & HVGR_MEM_ATTR_KMD_ACCESS) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u gva 0x%lx kmd access attribute is %lu not allowed map",
			ctx->id, para->gva, src_area->property);
		goto unlock_out;
	}
	if (atomic_read(&src_area->import.data.umm.free_tlb_cnt) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "dma buffer page was freed, memory map was not allowed");
		goto unlock_out;
	}
	if ((para->pages > src_area->pages) || !hvgr_mem_in_range(src_area->gva,
		PAGE_SIZE * src_area->pages, para->gva, PAGE_SIZE * para->pages)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map out of range 0x%lx:0x%lx 0x%lx:0x%lx.",
			ctx->id, para->gva, para->pages, src_area->gva, src_area->pages);
		goto unlock_out;
	}

	map_area = hvgr_mem_new_area(pmem);
	if (map_area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map alloc area fail.", ctx->id);
		goto unlock_out;
	}

	hvgr_mem_map_area_init(map_area, src_area, para);
	page_offset = (uint32_t)(map_area->gva - src_area->gva) >> PAGE_SHIFT;
	map_area->page_array = &src_area->page_array[page_offset];
	map_area->page_offset = page_offset;

	map_area->uva = hvgr_mem_alloc_va(map_area, true);
	if (map_area->uva == 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map alloc va fail for 0x%lx pages 0x%lx.",
			ctx->id, para->gva, para->pages);
		goto free_area;
	}

	atomic_inc(&src_area->ref_cnt);

	map_area->node.key = map_area->uva;
	map_area->node.size = map_area->pages << PAGE_SHIFT;
	(void)hvgr_kv_add(&pmem->map_rec, &map_area->node);
	hvgr_mem_uva_range_insert(ctx, map_area);

	*uva = map_area->uva;

	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u map mem prot=%lu:0x%lx, uva/gva=0x%lx gva 0x%lx to uva 0x%lx prot=%lu:0x%lx pages:0x%lx",
		ctx->id, src_area->property, src_area->attribute, src_area->gva,
		map_area->gva, map_area->uva, map_area->property, map_area->attribute,
		map_area->pages);

	mutex_unlock(&pmem->area_mutex);
	return 0;

free_area:
	hvgr_mem_delete_area(map_area);
unlock_out:
	mutex_unlock(&pmem->area_mutex);
	return -1;
}

long hvgr_mem_unmap_area_nolock(struct hvgr_ctx * const ctx, struct hvgr_mem_area *map_area,
	bool fd_close)
{
	struct hvgr_mem_area *src_area = NULL;
	struct hvgr_mem_ctx *pmem = &ctx->mem_ctx;
	uint64_t uva = map_area->uva;

	lockdep_assert_held(&ctx->mem_ctx.area_mutex);
	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u unmap mem prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx",
		ctx->id, map_area->property, map_area->attribute, map_area->pages,
		map_area->max_pages, map_area->uva);

	src_area = map_area->dep_area;
	(void)hvgr_mem_free_va(map_area);
	if (!fd_close) {
		mutex_lock(&map_area->map_lock);
		if (map_area->map_cnt != 0) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%d, uva = 0x%lx, map cnt = %d.", ctx->id,
				map_area->uva, map_area->map_cnt);
			mutex_unlock(&map_area->map_lock);
			return -EINVAL;
		}
		mutex_unlock(&map_area->map_lock);
	}
	hvgr_kv_del(&pmem->map_rec, map_area->node.key);
	hvgr_mem_uva_range_remove(ctx, map_area);
	hvgr_mem_delete_area(map_area);

	if (atomic_read(&src_area->ref_cnt) == 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u unmap src area ref is 0.", ctx->id);
		return -EINVAL;
	}

	atomic_dec(&src_area->ref_cnt);

	if (atomic_read(&src_area->ref_cnt) != 0)
		return 0;

	if (!src_area->freed)
		return 0;

	if (src_area->property == HVGR_MEM_PROPERTY_IMPORT)
		hvgr_mem_import_free_pages(src_area);
	else
		(void)hvgr_mem_free_pages(src_area);

	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u unmap mem uva=0x%lx del src area gva=0x%lx uva=0x%lx",
		ctx->id, uva, src_area->gva, src_area->uva);

	hvgr_mem_delete_area(src_area);

	return 0;
}

long hvgr_mem_unmap(struct hvgr_ctx * const ctx, uint64_t uva)
{
	struct hvgr_mem_area *map_area = NULL;
	long ret;
	uint32_t area_prop;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	map_area = hvgr_mem_zone_find_area_by_uva_range(&ctx->mem_ctx, uva);
	if (map_area == NULL) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u unmap no area for va 0x%lx.", ctx->id, uva);
		return -EINVAL;
	}
	area_prop = (uint32_t)map_area->dep_area->zone->proterty;
	ret = hvgr_mem_unmap_area_nolock(ctx, map_area, false);
	mutex_unlock(&ctx->mem_ctx.area_mutex);

	if (area_prop == HVGR_MEM_PROPERTY_IMPORT)
		hvgr_del_bg_dmabuf_ctx_from_global_list(ctx);
	return ret;
}

