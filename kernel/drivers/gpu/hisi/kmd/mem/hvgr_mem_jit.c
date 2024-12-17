/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_mem_jit.h"

#include <linux/highmem.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/rwsem.h>
#include <linux/mm.h>
#include <linux/rcupdate.h>

#include "hvgr_kmd_defs.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_mem_api.h"
#include "hvgr_mem_zone.h"
#include "hvgr_mem_opts.h"
#include "hvgr_kv.h"
#include "hvgr_mem_space.h"
#include "hvgr_memory_page.h"
#include "hvgr_log_api.h"

#define JIT_DEFAULT_SIZE (1 * 1024 * 1024 * 1024)
#define HVGR_JIT_MAX_COUNT 512u

struct hvgr_mem_jit_cb {
	struct hvgr_mem_space va_space;

	uint32_t large_size;
	uint32_t large_cnt;
	uint32_t large_cur_cnt;
	uint32_t medium_size;
	uint32_t medium_cnt;
	uint32_t medium_cur_cnt;
	uint32_t small_size;
	uint32_t small_cnt;
	uint32_t small_cur_cnt;
	uint32_t default_size;

	uint64_t *large_array;
	uint64_t *medium_array;
	uint64_t *small_array;

	uint64_t va_start;
	uint64_t va_size;

	spinlock_t array_lock;
};

static long hvgr_mem_jit_cfg_init(struct hvgr_mem_jit_cb *mem_jit,
	struct hvgr_mem_cfg_para *cfg_para)
{
	uint64_t *va_array = NULL;
	uint32_t jit_cnts = cfg_para->jit_large_cnt + cfg_para->jit_medium_cnt +
		cfg_para->jit_small_cnt;
	uint64_t va_start = mem_jit->va_start;
	uint32_t i;

	if (cfg_para->jit_default_size == JIT_DEFAULT_SIZE) {
		mem_jit->default_size = cfg_para->jit_default_size;
		mem_jit->va_space.start = mem_jit->va_start;
		mem_jit->va_space.size = mem_jit->va_size;
		return hvgr_mem_space_init(&mem_jit->va_space);
	}

	va_array = kzalloc(sizeof(uint64_t) * jit_cnts, GFP_KERNEL);
	if (unlikely(va_array == NULL))
		return -1;

	mem_jit->large_array = va_array;
	mem_jit->large_size = cfg_para->jit_large_size;
	mem_jit->large_cnt = cfg_para->jit_large_cnt;
	mem_jit->large_cur_cnt = mem_jit->large_cnt;

	mem_jit->medium_array = mem_jit->large_array + mem_jit->large_cnt;
	mem_jit->medium_size = cfg_para->jit_medium_size;
	mem_jit->medium_cnt = cfg_para->jit_medium_cnt;
	mem_jit->medium_cur_cnt = mem_jit->medium_cnt;

	mem_jit->small_array = mem_jit->medium_array + mem_jit->medium_cnt;
	mem_jit->small_size = cfg_para->jit_small_size;
	mem_jit->small_cnt = cfg_para->jit_small_cnt;
	mem_jit->small_cur_cnt = mem_jit->small_cnt;

	for (i = 0; i < mem_jit->large_cnt; i++) {
		mem_jit->large_array[i] = va_start;
		va_start += mem_jit->large_size;
	}

	for (i = 0; i < mem_jit->medium_cnt; i++) {
		mem_jit->medium_array[i] = va_start;
		va_start += mem_jit->medium_size;
	}

	for (i = 0; i < mem_jit->small_cnt; i++) {
		mem_jit->small_array[i] = va_start;
		va_start += mem_jit->small_size;
	}

	spin_lock_init(&mem_jit->array_lock);

	return 0;
}

HVGR_STATIC struct hvgr_mem_jit_cb * HVGR_WEAK hvgr_mem_jit_get_reserve(
	struct hvgr_mem_ctx * const pmem, struct hvgr_mem_cfg_para *cfg_para)
{
	unsigned long gva;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_jit_cb *mem_jit = NULL;
	uint64_t va_size;
	uint64_t va_size_tmp;

	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	if (cfg_para->jit_default_size == JIT_DEFAULT_SIZE) {
		va_size = cfg_para->jit_default_size;
	} else {
		va_size = cfg_para->jit_large_size;
		va_size *= cfg_para->jit_large_cnt;

		va_size_tmp = cfg_para->jit_medium_size;
		va_size_tmp *= cfg_para->jit_medium_cnt;
		va_size += va_size_tmp;

		va_size_tmp = cfg_para->jit_small_size;
		va_size_tmp *= cfg_para->jit_small_cnt;
		va_size += va_size_tmp;
	}

	if (va_size > HVGR_MEM_JIT_SIZE) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit size 0x%lx out of range", ctx->id,
			va_size);
		return NULL;
	}

	mem_jit = kzalloc(sizeof(*mem_jit), GFP_KERNEL);
	if (unlikely(mem_jit == NULL))
		return NULL;

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		gva = HVGR_MEM_JIT_START;
	} else {
		gva = vm_mmap(ctx->kfile, 0, va_size, PROT_NONE, MAP_SHARED, 0);
		if (IS_ERR_VALUE(gva)) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit vm map fail %d", ctx->id,
				(int)gva);
			kfree(mem_jit);
			return NULL;
		}

		hvgr_mem_va_check((gva + va_size));
	}

	mem_jit->va_start = gva;
	mem_jit->va_size = va_size;

	if (hvgr_mem_jit_cfg_init(mem_jit, cfg_para) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit cfg init fail", ctx->id);
		if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
			(void)hvgr_mem_vm_munmap(gva, mem_jit->va_size);
		kfree(mem_jit);
		return NULL;
	}

	return mem_jit;
}

static bool hvgr_mem_jit_init(struct hvgr_mem_zone * const zone,
	void * const para)
{
	struct hvgr_mem_jit_cb *mem_jit = NULL;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_cfg_para *cfg_para = (struct hvgr_mem_cfg_para *)para;

	mem_jit = hvgr_mem_jit_get_reserve(zone->pmem, cfg_para);
	if (mem_jit == NULL) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u JIT VA reserve FAIL.", ctx->id);
		return false;
	}

	zone->prv_data = mem_jit;

	return true;
}

static bool hvgr_mem_jit_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	return true;
}

static uint64_t hvgr_mem_jit_alloc_va(struct hvgr_ctx *ctx, struct hvgr_mem_jit_cb *mem_jit,
	uint64_t size)
{
	unsigned long flags;
	uint64_t addr = 0;

	if (mem_jit->default_size == JIT_DEFAULT_SIZE) {
		if (hvgr_mem_space_split(&mem_jit->va_space, (uint64_t)0, size, &addr) == 0)
			return addr;
		else
			return 0;
	}

	spin_lock_irqsave(&mem_jit->array_lock, flags);
	if (size == mem_jit->large_size) {
		if (mem_jit->large_cur_cnt > 0) {
			addr = mem_jit->large_array[mem_jit->large_cur_cnt - 1u];
			mem_jit->large_cur_cnt--;
		}
	} else if (size == mem_jit->medium_size) {
		if (mem_jit->medium_cur_cnt > 0) {
			addr = mem_jit->medium_array[mem_jit->medium_cur_cnt - 1u];
			mem_jit->medium_cur_cnt--;
		}
	} else if (size == mem_jit->small_size) {
		if (mem_jit->small_cur_cnt > 0) {
			addr = mem_jit->small_array[mem_jit->small_cur_cnt - 1u];
			mem_jit->small_cur_cnt--;
		}
	} else {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit alloc va size err 0x%lx", ctx->id, size);
	}
	spin_unlock_irqrestore(&mem_jit->array_lock, flags);

	return addr;
}

static void hvgr_mem_jit_free_va(struct hvgr_ctx *ctx, struct hvgr_mem_jit_cb *mem_jit,
	uint64_t addr, uint64_t size)
{
	unsigned long flags;

	if (mem_jit->default_size == JIT_DEFAULT_SIZE) {
		(void)hvgr_mem_space_joint(&mem_jit->va_space, addr, size);
		return;
	}

	spin_lock_irqsave(&mem_jit->array_lock, flags);
	if (size == mem_jit->large_size) {
		if (mem_jit->large_cur_cnt == mem_jit->large_cnt) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit large va full", ctx->id);
		} else {
			mem_jit->large_array[mem_jit->large_cur_cnt] = addr;
			mem_jit->large_cur_cnt++;
		}
	} else if (size == mem_jit->medium_size) {
		if (mem_jit->medium_cur_cnt == mem_jit->medium_cnt) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit medium va full", ctx->id);
		} else {
			mem_jit->medium_array[mem_jit->medium_cur_cnt] = addr;
			mem_jit->medium_cur_cnt++;
		}
	} else if (size == mem_jit->small_size) {
		if (mem_jit->small_cur_cnt == mem_jit->small_cnt) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit small va full", ctx->id);
		} else {
			mem_jit->small_array[mem_jit->small_cur_cnt] = addr;
			mem_jit->small_cur_cnt++;
		}
	} else {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit free va size err 0x%lx", ctx->id, size);
	}
	spin_unlock_irqrestore(&mem_jit->array_lock, flags);
}

static bool hvgr_mem_jit_alloc(struct hvgr_mem_zone * const zone,
		struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_jit_cb *mem_jit = NULL;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	mem_jit = (struct hvgr_mem_jit_cb *)zone->prv_data;
	if (unlikely(mem_jit == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u JIT prv is null", ctx->id);
		return false;
	}

	if (!hvgr_mem_alloc_pages(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u JIT alloc %lu pages fail",
			ctx->id, area->pages);
		return false;
	}

	size = area->max_pages << PAGE_SHIFT;
	addr = hvgr_mem_jit_alloc_va(ctx, mem_jit, size);
	if (addr == 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u JIT alloc va %lu pages fail",
			ctx->id, area->pages);
		(void)hvgr_mem_free_pages(area);
		return false;
	}

	area->gva = addr;
	area->uva = area->gva;
	hvgr_mem_attr2flag(area);

	if (!hvgr_mem_map_gva(area)) {
		hvgr_mem_jit_free_va(ctx, mem_jit, addr, size);
		(void)hvgr_mem_free_pages(area);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u JIT map gva FAIL. 0x%lx %lu/%lu",
			ctx->id, addr, area->pages, area->max_pages);
		return false;
	}

	INIT_LIST_HEAD(&area->jit_node);

	return true;
}

static bool hvgr_mem_jit_free(struct hvgr_mem_zone * const zone,
		struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	struct hvgr_mem_jit_cb *mem_jit = NULL;
	struct hvgr_ctx *ctx = NULL;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	mutex_lock(&ctx->mem_ctx.jit_shrink_mutex);
	list_del(&area->jit_node);
	mutex_unlock(&ctx->mem_ctx.jit_shrink_mutex);

	mem_jit = (struct hvgr_mem_jit_cb *)zone->prv_data;
	if (unlikely(mem_jit == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u JIT prv is null", ctx->id);
		/*
		 * when ctx was decfg, mem_jit is null. user has not chance to free this area.
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
	hvgr_mem_jit_free_va(ctx, mem_jit, addr, size);

	return true;
}

static void hvgr_mem_jit_free_areas(struct hvgr_ctx * const ctx)
{
	hvgr_mem_free_area_by_property(ctx, &ctx->mem_ctx.area_rec, HVGR_MEM_PROPERTY_JIT);
}

static bool hvgr_mem_jit_term(struct hvgr_mem_zone * const zone)
{
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_jit_cb *mem_jit = (struct hvgr_mem_jit_cb *)zone->prv_data;
	unsigned long gva;

	if (mem_jit == NULL)
		return true;

	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		if (hvgr_mem_mm_is_valid(zone->pmem)) {
			gva = mem_jit->va_start;
			(void)vm_munmap(gva, mem_jit->va_size);
		}
	}

	if (mem_jit->default_size == JIT_DEFAULT_SIZE)
		hvgr_mem_space_term(&mem_jit->va_space);
	else
		kfree(mem_jit->large_array);

	kfree(mem_jit);
	zone->prv_data = NULL;
	return true;
}

static const struct hvgr_zone_opts g_mem_zone_jit = {
	.init = hvgr_mem_jit_init,
	.term = hvgr_mem_jit_term,
	.check_para = hvgr_mem_jit_check_para,
	.alloc = hvgr_mem_jit_alloc,
	.free = hvgr_mem_jit_free,
};

bool hvgr_mem_jit_config(struct hvgr_mem_ctx * const pmem, void * const para)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_cfg_para *cfg_para = (struct hvgr_mem_cfg_para *)para;

	if (unlikely(pmem == NULL || cfg_para == NULL))
		return false;

	/* UKU cnts is all zero, do nothing */
	if (cfg_para->jit_large_cnt == 0 && cfg_para->jit_medium_cnt == 0 &&
		cfg_para->jit_small_cnt == 0)
		return true;

	if (cfg_para->jit_large_cnt > HVGR_JIT_MAX_COUNT ||
		cfg_para->jit_medium_cnt > HVGR_JIT_MAX_COUNT ||
		cfg_para->jit_small_cnt > HVGR_JIT_MAX_COUNT) {
		ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit para INVALID. %u, %u, %u",
			ctx->id, cfg_para->jit_large_cnt, cfg_para->jit_medium_cnt,
			cfg_para->jit_small_cnt);
		return false;
	}
	return hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_JIT,
		&g_mem_zone_jit, para);
}

void HVGR_WEAK hvgr_mem_jit_deconfig(struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_zone * const zone = &pmem->zones[HVGR_MEM_PROPERTY_JIT];
	struct hvgr_mem_jit_cb *mem_jit = NULL;
	unsigned long gva;

	mem_jit = (struct hvgr_mem_jit_cb *)zone->prv_data;
	if (unlikely(mem_jit == NULL))
		return;

	hvgr_mem_jit_free_areas(ctx);

	zone->prv_data = NULL;

	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		if (hvgr_mem_mm_is_valid(pmem)) {
			gva = mem_jit->va_start;
			(void)hvgr_mem_vm_munmap(gva, mem_jit->va_size);
		}
	}

	if (mem_jit->default_size == JIT_DEFAULT_SIZE)
		hvgr_mem_space_term(&mem_jit->va_space);
	else
		kfree(mem_jit->large_array);

	kfree(mem_jit);
}

