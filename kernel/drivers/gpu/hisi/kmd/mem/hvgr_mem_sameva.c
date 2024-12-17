/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_sameva.h"

#include "hvgr_mem_zone.h"
#include "hvgr_mem_api.h"
#include "hvgr_log_api.h"

static bool hvgr_mem_sameva_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	if (((area->attribute & HVGR_MEM_MASK_GPU_ACCESS) == 0) ||
		((area->attribute & HVGR_MEM_ATTR_MEM_SPARSE) != 0)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sameva attribute INVALID. 0x%lx",
			ctx->id, area->attribute);
		return false;
	}

	return true;
}

static bool hvgr_mem_sameva_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	bool need_map = (area->attribute & HVGR_MEM_MASK_UMD_ACCESS) != 0;

	if (!hvgr_mem_alloc_pages(area))
		return false;

	area->uva = hvgr_mem_alloc_va(area, need_map);
	if (area->uva == 0) {
		(void)hvgr_mem_free_pages(area);
		return false;
	}

	area->gva = area->uva;
	hvgr_mem_attr2flag(area);

	if (!hvgr_mem_map_gva(area)) {
		(void)hvgr_mem_free_va(area);
		(void)hvgr_mem_free_pages(area);
		return false;
	}

	return true;
}

static bool hvgr_mem_sameva_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	hvgr_mem_unmap_gva(area);
	(void)hvgr_mem_free_va(area);
	(void)hvgr_mem_free_pages(area);

	return true;
}

static const struct hvgr_zone_opts g_mem_zone_sameva = {
	.init = hvgr_mem_opt_init,
	.term = hvgr_mem_opt_term,
	.check_para = hvgr_mem_sameva_check_para,
	.alloc = hvgr_mem_sameva_alloc,
	.free = hvgr_mem_sameva_free,
};

bool hvgr_mem_sameva_config(struct hvgr_mem_ctx * const pmem)
{
	bool ret = false;

	if (pmem == NULL)
		return false;

	ret = hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_SAME_VA,
		&g_mem_zone_sameva, NULL);
	if (!ret)
		return false;

	return true;
}
