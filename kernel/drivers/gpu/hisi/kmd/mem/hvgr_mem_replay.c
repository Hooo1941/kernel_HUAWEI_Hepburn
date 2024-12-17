/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_mem_replay.h"

#include <linux/slab.h>
#include <linux/mman.h>

#include "hvgr_mem_zone.h"
#include "hvgr_mem_opts.h"
#include "hvgr_kv.h"
#include "hvgr_mem_space.h"
#include "hvgr_assert.h"
#include "hvgr_log_api.h"

#define HVGR_REPLAY_ALLOC_VA_TIMES 10
#define HVGR_REPLAY_SIZE_12G       HVGR_MEM_REPLAY_SIZE
#define HVGR_REPLAY_EXPECT_ADDR   ((u64)0x4000000000)

struct hvgr_mem_replay_cb {
	struct hvgr_mem_space va_space;
	atomic_t refcounter;
};

static int hvgr_mem_replay_get_va(struct hvgr_mem_zone * const zone, uint64_t va,
	uint64_t size, uint64_t *addr)
{
	struct hvgr_mem_replay_cb *replay_cb = NULL;
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	replay_cb = (struct hvgr_mem_replay_cb *)zone->prv_data;
	if (replay_cb == NULL)
		return -ENOENT;
	if (hvgr_mem_space_split(&replay_cb->va_space, va, size, addr) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay get va from space fail", ctx->id);
		return -EFAULT;
	}

	return 0;
}

static int hvgr_mem_replay_put_va(struct hvgr_mem_zone * const zone, uint64_t va, uint64_t size)
{
	struct hvgr_mem_replay_cb *replay_cb = NULL;
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	replay_cb = (struct hvgr_mem_replay_cb *)zone->prv_data;
	if (replay_cb == NULL)
		return -ENOENT;
	if (hvgr_mem_space_joint(&replay_cb->va_space, va, size) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay release va fail", ctx->id);
		return -EFAULT;
	}

	return 0;
}

HVGR_STATIC uint64_t HVGR_WEAK hvgr_mem_replay_find_reserve(struct hvgr_mem_ctx * const pmem,
	struct hvgr_mem_cfg_para *para)
{
	unsigned long uva;
	unsigned long flag;
	uint32_t idx;
	uint64_t size = HVGR_REPLAY_SIZE_12G;
	uint64_t expect_addr = HVGR_REPLAY_EXPECT_ADDR;
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
		return HVGR_MEM_REPLAY_START;

	/* Currently, if the size is not 12GB, the umd config may be incorrect. */
	if (unlikely(para->mem_replay_size != size)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay size %lx invalid",
			ctx->id, para->mem_replay_size);
		return 0;
	}

	flag = VM_MIXEDMAP | VM_DONTCOPY | VM_DONTDUMP | VM_DONTEXPAND;
	for (idx = 1; idx <= HVGR_REPLAY_ALLOC_VA_TIMES; idx++) {
		uva = vm_mmap(ctx->kfile, expect_addr, size, flag, MAP_SHARED, 0);
		if (IS_ERR_VALUE(uva)) {
			expect_addr -= HVGR_REPLAY_SIZE_12G;
			continue;
		}
		/* There's no enough space or expect va range is be used. */
		if (idx != 1 || uva != HVGR_REPLAY_EXPECT_ADDR)
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u replay va %lx, alloc %d times, replay may fail",
				ctx->id, uva, idx);

		return uva;
	}

	return 0;
}

HVGR_STATIC struct hvgr_mem_replay_cb * HVGR_WEAK hvgr_mem_replay_create_reserve(
	struct hvgr_mem_ctx * const pmem, struct hvgr_mem_cfg_para *para)
{
	uint64_t uva;
	struct hvgr_mem_replay_cb *replay_cb = NULL;
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	replay_cb = kzalloc(sizeof(*replay_cb), GFP_KERNEL);
	if (unlikely(replay_cb == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay alloc cb fail", ctx->id);
		return NULL;
	}
	uva = hvgr_mem_replay_find_reserve(pmem, para);
	if (uva == 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay find reserve va region fail", ctx->id);
		goto fail;
	}
	replay_cb->va_space.start = uva;
	replay_cb->va_space.size = HVGR_REPLAY_SIZE_12G;
	if (hvgr_mem_space_init(&replay_cb->va_space) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay init space fail", ctx->id);
		if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
			(void)hvgr_mem_vm_munmap(uva, HVGR_REPLAY_SIZE_12G);
		goto fail;
	}

	return replay_cb;
fail:
	kfree(replay_cb);
	return NULL;
}

static struct hvgr_mem_replay_cb *hvgr_mem_replay_get_reserve(
	struct hvgr_mem_ctx * const pmem, struct hvgr_mem_cfg_para *para)
{
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_dev *dmem = &ctx->gdev->mem_dev;
	struct hvgr_kv_node *node = NULL;
	struct hvgr_mem_replay_cb *replay_cb = NULL;
	struct hvgr_mem_process_res_cb *res_cb = NULL;

	mutex_lock(&dmem->mem_process_res_lock);
#ifdef CONFIG_LIBLINUX
	node = hvgr_kv_get(&dmem->mem_process_res_rec, (uint64_t)ctx->tgid);
#else
	node = hvgr_kv_get(&dmem->mem_process_res_rec, (uint64_t)ctx->mem_ctx.process_mm);
#endif
	if (node == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay find reserve cb node fail", ctx->id);
		mutex_unlock(&dmem->mem_process_res_lock);
		return NULL;
	}

	res_cb = container_of(node, struct hvgr_mem_process_res_cb, kv_node);
	replay_cb = res_cb->mem_replay;
	if (replay_cb != NULL) {
		atomic_inc(&replay_cb->refcounter);
		mutex_unlock(&dmem->mem_process_res_lock);
		return replay_cb;
	}

	replay_cb = hvgr_mem_replay_create_reserve(pmem, para);
	if (replay_cb == NULL) {
		mutex_unlock(&dmem->mem_process_res_lock);
		return NULL;
	}
	res_cb->mem_replay = replay_cb;
	mutex_unlock(&dmem->mem_process_res_lock);

	return replay_cb;
}

static bool hvgr_mem_replay_init(struct hvgr_mem_zone * const zone,
	void * const para)
{
	struct hvgr_mem_replay_cb *replay_cb = NULL;
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	/* Alloc private data. */
	replay_cb = hvgr_mem_replay_get_reserve(zone->pmem,
		(struct hvgr_mem_cfg_para *)para);
	if (replay_cb == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay mem create reserve fail", ctx->id);
		return false;
	}

	zone->prv_data = replay_cb;

	return true;
}

HVGR_STATIC void HVGR_WEAK hvgr_mem_replay_put_reserve(struct hvgr_mem_ctx * const pmem,
	struct hvgr_mem_replay_cb * const replay_cb)
{
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_dev *dmem = &ctx->gdev->mem_dev;
	struct hvgr_kv_node *node = NULL;
	struct hvgr_mem_process_res_cb *res_cb = NULL;

	mutex_lock(&dmem->mem_process_res_lock);
	if (atomic_read(&replay_cb->refcounter) > 0) {
		atomic_dec(&replay_cb->refcounter);
		goto out;
	}

	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		if (hvgr_mem_mm_is_valid(pmem))
			(void)hvgr_mem_vm_munmap(replay_cb->va_space.start, HVGR_REPLAY_SIZE_12G);
	}

	hvgr_mem_space_term(&replay_cb->va_space);
	kfree(replay_cb);

#ifdef CONFIG_LIBLINUX
	node = hvgr_kv_get(&dmem->mem_process_res_rec, (uint64_t)ctx->tgid);
#else
	node = hvgr_kv_get(&dmem->mem_process_res_rec, (uint64_t)ctx->mem_ctx.process_mm);
#endif
	if (node == NULL)
		goto out;
	res_cb = container_of(node, struct hvgr_mem_process_res_cb, kv_node);
	res_cb->mem_replay = NULL;
out:
	mutex_unlock(&dmem->mem_process_res_lock);
}

static bool hvgr_mem_replay_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely(zone == NULL || area == NULL))
		return false;

	if (((~(HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_GPU_WR | HVGR_MEM_ATTR_GPU_CACHEABLE |
		HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR)) &
		area->attribute) != 0) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay memory attribute INVALID. 0x%lx",
			ctx->id, area->attribute);
		return false;
	}

	area->max_pages = area->pages;

	return true;
}

static bool hvgr_mem_replay_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	uint64_t addr = 0;
	struct hvgr_mem_replay_cb *replay_cb = NULL;
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	replay_cb = (struct hvgr_mem_replay_cb *)zone->prv_data;
	if (unlikely(replay_cb == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay alloc cb is NULL", ctx->id);
		return false;
	}

	if (hvgr_mem_alloc_pages(area) != true) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay alloc pages fail, 0x%llx, 0x%llx",
			ctx->id, area->pages, area->max_pages);
		return false;
	}

	if (hvgr_mem_replay_get_va(zone, area->gva,
		area->max_pages << PAGE_SHIFT, &addr) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay get va fail, 0x%llx",
			ctx->id, area->gva);
		goto fail;
	}

	area->gva = addr;
	hvgr_mem_attr2flag(area);

	if (hvgr_mem_map_gva(area) != true) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay map gva fail, 0x%llx, 0x%llx, 0x%llx",
			ctx->id, area->gva, area->pages, area->max_pages);
		if (hvgr_mem_replay_put_va(zone, addr, area->max_pages << PAGE_SHIFT) != 0)
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u replay put va fail, 0x%llx",
				ctx->id, addr);
		goto fail;
	}

	return true;
fail:
	(void)hvgr_mem_free_pages(area);
	return false;
}

static bool hvgr_mem_replay_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	struct hvgr_mem_replay_cb *replay_cb = NULL;

	replay_cb = (struct hvgr_mem_replay_cb *)zone->prv_data;
	if (replay_cb == NULL) {
		/*
		 * when ctx was decfg, replay_cb is null. user has not chance to free this area.
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
	(void)hvgr_mem_replay_put_va(zone, addr, size);
	(void)hvgr_mem_free_pages(area);

	return true;
}

static bool hvgr_mem_replay_term(struct hvgr_mem_zone * const zone)
{
	struct hvgr_mem_replay_cb *replay_cb = (struct hvgr_mem_replay_cb *)zone->prv_data;

	if (replay_cb == NULL)
		return true;

	zone->prv_data = NULL;
	hvgr_mem_replay_put_reserve(zone->pmem, replay_cb);
	return true;
}

static const struct hvgr_zone_opts g_mem_zone_replay = {
	.init = hvgr_mem_replay_init,
	.term = hvgr_mem_replay_term,
	.check_para = hvgr_mem_replay_check_para,
	.alloc = hvgr_mem_replay_alloc,
	.free = hvgr_mem_replay_free,
};

bool hvgr_mem_replay_config(struct hvgr_mem_ctx * const pmem,
	struct hvgr_mem_cfg_para *para)
{
	if (unlikely(pmem == NULL || para == NULL))
		return false;

	/* replay mem is not enabled */
	if (para->mem_replay_size == 0)
		return true;

	return hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_REPLAY,
		&g_mem_zone_replay, (void *)para);
}

void hvgr_mem_replay_deconfig(struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_mem_zone *zone = &pmem->zones[HVGR_MEM_PROPERTY_REPLAY];
	struct hvgr_mem_replay_cb *replay_cb = NULL;

	/* replay mem is not configed */
	if (zone->opts == NULL)
		return;

	replay_cb = (struct hvgr_mem_replay_cb *)zone->prv_data;
	if (replay_cb == NULL)
		return;
	zone->prv_data = NULL;

	hvgr_mem_replay_put_reserve(zone->pmem, replay_cb);
}
