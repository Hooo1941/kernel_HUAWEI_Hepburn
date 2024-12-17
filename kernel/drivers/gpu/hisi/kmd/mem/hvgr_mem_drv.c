/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_drv.h"

#include "hvgr_mem_api.h"
#include "hvgr_mem_zone.h"
#include "hvgr_log_api.h"

static bool hvgr_mem_driver_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	if (((~(HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR | HVGR_MEM_ATTR_KMD_ACCESS)) &
		area->attribute) != 0) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u drv attribute INVALID. 0x%lx",
			ctx->id, area->attribute);
		return false;
	}

	area->max_pages = area->pages;

	return true;
}

static bool hvgr_mem_driver_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	if (!hvgr_mem_alloc_pages(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u drv alloc pages FAIL. %lu/%lu",
			ctx->id, area->pages, area->max_pages);
		return false;
	}

	area->uva = hvgr_mem_alloc_va(area, true);
	if (area->uva == 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u drv alloc va FAIL. %lu/%lu",
			ctx->id, area->pages, area->max_pages);
		(void)hvgr_mem_free_pages(area);
		return false;
	}

	/* Use gva field to record this area object */
	area->gva = area->uva;

	return true;
}

static bool hvgr_mem_driver_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	(void)hvgr_mem_free_va(area);
	(void)hvgr_mem_free_pages(area);

	return true;
}

static const struct hvgr_zone_opts g_mem_zone_driver = {
	.init = hvgr_mem_opt_init,
	.term = hvgr_mem_opt_term,
	.check_para = hvgr_mem_driver_check_para,
	.alloc = hvgr_mem_driver_alloc,
	.free = hvgr_mem_driver_free,
};

bool hvgr_mem_driver_config(struct hvgr_mem_ctx * const pmem)
{
	if (unlikely(pmem == NULL))
		return false;

	return hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_DRIVER,
		&g_mem_zone_driver, NULL);
}
