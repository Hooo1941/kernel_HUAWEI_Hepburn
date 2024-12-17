/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_gpu.h"

#include <linux/slab.h>

#include "hvgr_mem_zone.h"
#include "hvgr_mem_api.h"
#include "hvgr_kv.h"
#include "hvgr_mem_space.h"
#include "hvgr_log_api.h"

struct hvgr_mem_gpu_cb {
	struct hvgr_mem_space va_space;
};

static bool hvgr_mem_gpu_init(struct hvgr_mem_zone * const zone,
	void * const para)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_gpu_cb *mem_gpu = NULL;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
		return true;

	mem_gpu = kzalloc(sizeof(*mem_gpu), GFP_KERNEL);
	if (unlikely(mem_gpu == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu mem init kzalloc fail.", ctx->id);
		return false;
	}

	mem_gpu->va_space.start = HVGR_MEM_GPU_START;
	mem_gpu->va_space.size = HVGR_MEM_GPU_SIZE;
	if (hvgr_mem_space_init(&mem_gpu->va_space) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu mem init space fail.", ctx->id);
		kfree(mem_gpu);
		return false;
	}

	zone->prv_data = mem_gpu;
	return true;
}

static bool hvgr_mem_gpu_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	if (((area->attribute & HVGR_MEM_MASK_KMD_ACCESS) != 0) ||
		((area->attribute & HVGR_MEM_MASK_GPU_ACCESS) == 0) ||
		((area->attribute & HVGR_MEM_ATTR_MEM_SPARSE) != 0) ||
		((area->attribute & HVGR_MEM_ATTR_GPU_EXEC) != 0) ||
		((area->attribute & HVGR_MEM_MASK_COHERENT) != 0)) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu memory attribute INVALID. 0x%lx",
			ctx->id, area->attribute);
		return false;
	}

	return true;
}

static uint64_t hvgr_mem_gpu_alloc_va(struct hvgr_mem_area * const area)
{
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_gpu_cb *mem_gpu = NULL;
	uint64_t size;
	uint64_t addr;

	pmem = area->zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
		return hvgr_mem_alloc_va(area, false);

	mem_gpu = (struct hvgr_mem_gpu_cb *)area->zone->prv_data;
	if (unlikely(mem_gpu == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu alloc prv is null", ctx->id);
		return 0;
	}

	size = area->max_pages << PAGE_SHIFT;
	if (hvgr_mem_space_split(&mem_gpu->va_space, 0, size, &addr) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu alloc va FAIL. %lu/%lu",
			ctx->id, area->pages, area->max_pages);
		return 0;
	}

	return addr;
}

static void hvgr_mem_gpu_free_va(struct hvgr_mem_area * const area)
{
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_gpu_cb *mem_gpu = NULL;
	uint64_t addr;
	uint64_t size;

	pmem = area->zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		(void)hvgr_mem_free_va(area);
		return;
	}

	mem_gpu = (struct hvgr_mem_gpu_cb *)area->zone->prv_data;
	if (unlikely(mem_gpu == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu free prv is null", ctx->id);
		return;
	}

	addr = area->gva;
	size = area->max_pages << PAGE_SHIFT;
	if (hvgr_mem_space_joint(&mem_gpu->va_space, addr, size) != 0)
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu free joint addr/size:0x%lx/0x%lx fail",
			ctx->id, addr, size);
}

static bool hvgr_mem_gpu_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	if (!hvgr_mem_alloc_pages(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu alloc pages FAIL. %lu/%lu",
			ctx->id, area->pages, area->max_pages);
		return false;
	}

	area->uva = hvgr_mem_gpu_alloc_va(area);
	if (area->uva == 0) {
		(void)hvgr_mem_free_pages(area);
		return false;
	}

	area->gva = area->uva;
	hvgr_mem_attr2flag(area);

	if (!hvgr_mem_map_gva(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu alloc map gva FAIL. 0x%lx/%lu/%lu",
			ctx->id, area->gva, area->pages, area->max_pages);
		hvgr_mem_gpu_free_va(area);
		(void)hvgr_mem_free_pages(area);
		return false;
	}

	area->uva = 0;
	return true;
}

static bool hvgr_mem_gpu_free(struct hvgr_mem_zone * const zone,
		struct hvgr_mem_area * const area)
{
	hvgr_mem_unmap_gva(area);
	hvgr_mem_gpu_free_va(area);
	(void)hvgr_mem_free_pages(area);

	return true;
}

static bool hvgr_mem_gpu_term(struct hvgr_mem_zone * const zone)
{
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
		return true;

	if (zone->prv_data != NULL) {
		struct hvgr_mem_gpu_cb *mem_gpu = zone->prv_data;

		hvgr_mem_space_term(&mem_gpu->va_space);
		kfree(mem_gpu);
		zone->prv_data = NULL;
	}

	return true;
}

static const struct hvgr_zone_opts g_mem_zone_gpu = {
	.init = hvgr_mem_gpu_init,
	.term = hvgr_mem_gpu_term,
	.check_para = hvgr_mem_gpu_check_para,
	.alloc = hvgr_mem_gpu_alloc,
	.free = hvgr_mem_gpu_free,
};

bool hvgr_mem_gpu_config(struct hvgr_mem_ctx * const pmem)
{
	if (unlikely(pmem == NULL))
		return false;

	return hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_GPU_MEM,
		&g_mem_zone_gpu, NULL);
}

int hvgr_gpu_mem_change_attr(struct hvgr_ctx *ctx,
	struct hvgr_mem_area *area, uint64_t attr)
{
	uint64_t temp_attr;

	if (unlikely(ctx == NULL || area == NULL || attr == 0))
		return -ENOENT;

	/* nothing to do */
	if (area->attribute == attr)
		return 0;

	if ((area->attribute & (~HVGR_MEM_ATTR_GPU_CACHEABLE)) !=
		(attr & (~HVGR_MEM_ATTR_GPU_CACHEABLE))) {
		hvgr_err(ctx->gdev, HVGR_MEM, "gpu mem attribute change to much");
		return -EFAULT;
	}

	temp_attr = area->attribute;
	area->attribute = attr;

	hvgr_mem_attr2flag(area);
	if (!hvgr_mem_map_gva(area)) {
		area->attribute = temp_attr;
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u gpu mem attribute change map fail",
			ctx->id);
		return -EFAULT;
	}

	return 0;
}
