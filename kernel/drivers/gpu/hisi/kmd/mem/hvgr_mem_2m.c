/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_mem_2m.h"

#include <linux/highmem.h>
#include <linux/mman.h>
#include <linux/vmalloc.h>
#include <linux/rwsem.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>

#include "hvgr_kmd_defs.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_mem_api.h"
#include "hvgr_mem_zone.h"
#include "hvgr_kv.h"
#include "hvgr_mem_space.h"
#include "hvgr_memory_page.h"
#include "hvgr_mmu_api.h"
#include "hvgr_log_api.h"

struct hvgr_mem_2m_cb {
	struct hvgr_mem_space va_space;
	uint64_t va_start;
};

#define HVGR_2M_SIZE_2M (2 * 1024 * 1024)
#define HVGR_2M_MAP_TRY_TIMES 100
#define HVGR_2M_PAGE_ORDER 9
#define HVGR_2M_PAGE_MASK 0x1FF

HVGR_STATIC struct hvgr_mem_2m_cb * HVGR_WEAK hvgr_mem_2m_get_reserve(
	struct hvgr_mem_ctx * const pmem)
{
	unsigned long uva_align;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_2m_cb *mem_2m = NULL;

	mem_2m = kzalloc(sizeof(*mem_2m), GFP_KERNEL);
	if (unlikely(mem_2m == NULL))
		return NULL;

	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		uva_align = HVGR_MEM_2M_START;
	} else {
		unsigned long uva = vm_mmap(ctx->kfile, 0, (HVGR_MEM_2M_SIZE + HVGR_2M_SIZE_2M),
			PROT_NONE, MAP_SHARED, 0);
		if (IS_ERR_VALUE(uva)) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M resvss fail", ctx->id);
			kfree(mem_2m);
			return NULL;
		}

		hvgr_mem_va_check((uva + (HVGR_MEM_2M_SIZE + HVGR_2M_SIZE_2M)));

		uva_align = (uva + (unsigned long)(HVGR_2M_SIZE_2M - 1)) &
			(~(unsigned long)(HVGR_2M_SIZE_2M - 1));
		mem_2m->va_start = uva;
	}

	mem_2m->va_space.start = uva_align;
	mem_2m->va_space.size = HVGR_MEM_2M_SIZE;
	(void)hvgr_mem_space_init(&mem_2m->va_space);

	return mem_2m;
}

static bool hvgr_mem_2m_init(struct hvgr_mem_zone * const zone, void * const para)
{
	struct hvgr_mem_2m_cb *mem_2m = NULL;
	struct hvgr_ctx *ctx = NULL;

	mem_2m = hvgr_mem_2m_get_reserve(zone->pmem);
	if (mem_2m == NULL) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M VA reserve FAIL.", ctx->id);
		return false;
	}

	zone->prv_data = mem_2m;

	return true;
}

static bool hvgr_mem_2m_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	if (((area->attribute & (~(HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_GPU_WR))) != 0) ||
		((area->pages & HVGR_2M_PAGE_MASK) != 0)) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M para invalid attr:0x%x pages:%lu/%lu",
			ctx->id, area->attribute, area->pages, area->max_pages);
		return false;
	}

	area->max_pages = area->pages;
	return true;
}

static bool hvgr_mem_2m_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	int ret;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_2m_cb *mem_2m = NULL;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	mem_2m = (struct hvgr_mem_2m_cb *)zone->prv_data;
	if (unlikely(mem_2m == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M prv is null", ctx->id);
		return false;
	}

	area->order = HVGR_2M_PAGE_ORDER;
	if (!hvgr_mem_alloc_pages(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M alloc %lu pages fail",
			ctx->id, area->pages);
		return false;
	}

	size = area->max_pages << PAGE_SHIFT;
	ret = hvgr_mem_space_split(&mem_2m->va_space, (uint64_t)0, size, &addr);
	if (ret != 0) {
		(void)hvgr_mem_free_pages(area);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M alloc va FAIL. %lu/%lu %d",
			ctx->id, area->pages, area->max_pages, ret);
		return false;
	}

	area->gva = addr;
	area->uva = area->gva;
	area->flags |= HVGR_MEM_AREA_FLAG_2M;
	hvgr_mem_attr2flag(area);

	if (!hvgr_mem_map_gva(area)) {
		(void)hvgr_mem_space_joint(&mem_2m->va_space, addr, size);
		(void)hvgr_mem_free_pages(area);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M map gva FAIL. 0x%lx %lu/%lu",
			ctx->id, addr, area->pages, area->max_pages);
		return false;
	}

	return true;
}

static bool hvgr_mem_2m_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	int ret;
	struct hvgr_mem_2m_cb *mem_2m = NULL;
	struct hvgr_ctx *ctx = NULL;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	mem_2m = (struct hvgr_mem_2m_cb *)zone->prv_data;
	if (unlikely(mem_2m == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M prv is null", ctx->id);
		/*
		 * when ctx was decfg, mem_2m is null. user has not chance to free this area.
		 * we have to free the area's pages here called from hvgr_mem_free_leak_area.
		 * hvgr_mem_free_leak_area ignore the 'return false'.
		 */
		hvgr_mem_unmap_gva(area);
		(void)hvgr_mem_free_pages(area);
		return false;
	}

	addr = area->gva;
	size = area->max_pages << PAGE_SHIFT;

	hvgr_mem_unmap_gva(area);
	(void)hvgr_mem_free_pages(area);
	ret = hvgr_mem_space_joint(&mem_2m->va_space, addr, size);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 2M free va FAIL. 0x%lx %lu/%lu %d",
			ctx->id, addr, area->pages, area->max_pages, ret);
		return false;
	}

	return true;
}

static bool hvgr_mem_2m_term(struct hvgr_mem_zone * const zone)
{
	struct hvgr_mem_2m_cb *mem_2m = (struct hvgr_mem_2m_cb *)zone->prv_data;

	if (mem_2m == NULL)
		return true;

	hvgr_mem_space_term(&mem_2m->va_space);
	kfree(mem_2m);
	zone->prv_data = NULL;
	return true;
}

static const struct hvgr_zone_opts g_mem_zone_2m = {
	.init = hvgr_mem_2m_init,
	.term = hvgr_mem_2m_term,
	.check_para = hvgr_mem_2m_check_para,
	.alloc = hvgr_mem_2m_alloc,
	.free = hvgr_mem_2m_free,
};

bool hvgr_mem_2m_config(struct hvgr_mem_ctx * const pmem)
{
	if (unlikely(pmem == NULL))
		return false;

	return hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_2M_PAGE,
		&g_mem_zone_2m, NULL);
}

void HVGR_WEAK hvgr_mem_2m_deconfig(struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_zone * const zone = &pmem->zones[HVGR_MEM_PROPERTY_2M_PAGE];
	struct hvgr_mem_2m_cb *mem_2m = NULL;
	unsigned long uva;

	mem_2m = (struct hvgr_mem_2m_cb *)zone->prv_data;
	if (unlikely(mem_2m == NULL))
		return;

	zone->prv_data = NULL;

	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		if (hvgr_mem_mm_is_valid(pmem)) {
			uva = mem_2m->va_start;
			(void)hvgr_mem_vm_munmap(uva, (HVGR_MEM_2M_SIZE + HVGR_2M_SIZE_2M));
		}
	}

	hvgr_mem_space_term(&mem_2m->va_space);
	kfree(mem_2m);
}

