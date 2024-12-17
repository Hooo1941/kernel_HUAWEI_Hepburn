/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include <securec.h>

#include "hvgr_mem_zone.h"
#include "hvgr_mem_api.h"
#include "hvgr_log_api.h"

bool hvgr_mem_zone_init_proc(struct hvgr_mem_ctx * const pmem)
{
	if (unlikely(pmem == NULL))
		return false;

	(void)memset_s(pmem->zones, sizeof(pmem->zones), 0, sizeof(pmem->zones));

	return true;
}

void hvgr_mem_zone_term_proc(struct hvgr_mem_ctx * const pmem)
{
	uint32_t idx;

	for (idx = 0; idx < HVGR_MEM_PROPERTY_MAX; idx++)
		hvgr_mem_zone_destroy(pmem, idx);
}

bool hvgr_mem_zone_create(struct hvgr_mem_ctx * const pmem,
	uint64_t prot, struct hvgr_zone_opts const * const opts,
	void * const para)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_zone *zone = NULL;

	if ((pmem == NULL) || (prot >= HVGR_MEM_PROPERTY_MAX) ||
		(opts == NULL))
		return false;

	zone = &pmem->zones[prot];
	zone->pmem = pmem;
	zone->opts = opts;
	zone->proterty = prot;
	if (!zone->opts->init(zone, para)) {
		ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u memory zone %d init FAIL.", ctx->id, prot);
		(void)memset_s(zone, sizeof(*zone), 0, sizeof(*zone));
		return false;
	}

	return true;
}

void hvgr_mem_zone_destroy(struct hvgr_mem_ctx * const pmem,
	uint64_t prot)
{
	struct hvgr_mem_zone *zone = NULL;

	zone = hvgr_mem_zone_get_zone(pmem, prot);
	if (unlikely(zone == NULL))
		return;

	(void)zone->opts->term(zone);

	(void)memset_s(zone, sizeof(*zone), 0, sizeof(*zone));
}

struct hvgr_mem_zone *hvgr_mem_zone_get_zone(
	struct hvgr_mem_ctx * const pmem, uint64_t prot)
{
	struct hvgr_mem_zone *zone = NULL;

	if (unlikely((pmem == NULL) || (prot >= HVGR_MEM_PROPERTY_MAX)))
		return NULL;

	zone = &pmem->zones[prot];
	if (zone->opts == NULL)
		return NULL;

	return zone;
}

static inline struct hvgr_mem_area *hvgr_mem_zone_find_area_va(
	struct hvgr_kv_map * const root, uint64_t va)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_kv_node *node = hvgr_kv_get(root, va);

	if (node != NULL)
		area = container_of(node, struct hvgr_mem_area, node);

	return area;
}

static inline struct hvgr_mem_area *hvgr_mem_zone_find_area_va_range(
	struct hvgr_kv_map * const root, uint64_t va)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_kv_node *node = hvgr_kv_find_range(root, va);

	if (node != NULL)
		area = container_of(node, struct hvgr_mem_area, node);

	return area;
}

struct hvgr_mem_area *hvgr_mem_zone_find_area_by_gva(
	struct hvgr_mem_ctx * const pmem, uint64_t gva)
{
	if (unlikely(pmem == NULL))
		return NULL;

	return hvgr_mem_zone_find_area_va(&pmem->area_rec, gva);
}

struct hvgr_mem_area *hvgr_mem_zone_find_area_by_gva_range(
	struct hvgr_mem_ctx * const pmem, uint64_t gva)
{
	if (unlikely(pmem == NULL))
		return NULL;

	return hvgr_mem_zone_find_area_va_range(&pmem->area_rec, gva);
}

struct hvgr_mem_area *hvgr_mem_zone_find_area_by_uva(
	struct hvgr_mem_ctx * const pmem, uint64_t uva)
{
	if (unlikely(pmem == NULL))
		return NULL;

	return hvgr_mem_zone_find_area_va(&pmem->map_rec, uva);
}

struct hvgr_mem_area *hvgr_mem_zone_find_area_by_uva_range(
	struct hvgr_mem_ctx * const pmem, uint64_t uva)
{
	if (unlikely(pmem == NULL))
		return NULL;

	return hvgr_mem_zone_find_area_va_range(&pmem->map_rec, uva);
}

bool hvgr_mem_zone_alloc_memory(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	/**
	 * Put the check_para at the general procedure because
	 * every implementation must check input parameters.
	 */
	if (!zone->opts->check_para(zone, area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mem alloc check para fail. %lu:0x%lx",
			ctx->id, area->property, area->attribute);
		return false;
	}

	if (!zone->opts->alloc(zone, area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mem alloc fail. %lu:0x%lx:0x%lx:0x%lx",
			ctx->id, area->property, area->attribute, area->pages, area->max_pages);
		return false;
	}

	if (area->attribute & HVGR_MEM_MASK_KMD_ACCESS) {
		if (!hvgr_mem_map_kva(area)) {
			if (zone->opts->free)
				zone->opts->free(zone, area);
			return false;
		}
	}

	return true;
}

void hvgr_mem_zone_free_memory(struct hvgr_mem_area * const area)
{
	struct hvgr_mem_zone *zone = NULL;
	struct hvgr_ctx *ctx = NULL;

	if (area->zone == NULL)
		return;

	zone = area->zone;

	if (area->attribute & HVGR_MEM_MASK_KMD_ACCESS)
		hvgr_mem_unmap_kva(area);

	if (!zone->opts->free(zone, area)) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u memory Free fail. gva = %llx",
			ctx->id, area->gva);

		return;
	}

	return;
}

bool hvgr_mem_zone_import_memory(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area, int fd)
{
	if ((area == NULL) || (zone == NULL) || (zone->opts == NULL) ||
		(zone->opts->import == NULL))
		return false;

	return zone->opts->import(zone, area, fd);
}

