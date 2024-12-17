/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_pool.h"

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/shrinker.h>
#include <linux/highmem.h>

#include "hvgr_defs.h"
#include "hvgr_pool.h"
#include "hvgr_mem_api.h"
#include "hvgr_memory_page.h"
#include "hvgr_log_api.h"
#include "hvgr_dm_api.h"

static struct hvgr_mem_pool *hvgr_mem_pool_get_ctx_pool(struct hvgr_ctx *ctx, uint64_t attr)
{
	struct hvgr_mem_pool *mem_pool = NULL;
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	uint32_t policy_id = hvgr_get_policy_id(attr);
	if (policy_id != 0)
		mem_pool = hvgr_sc_mem_find_pool(&ctx->mem_ctx.sc_data.mem_pool_list,
			policy_id);
#endif
	if (mem_pool == NULL)
		mem_pool = &ctx->mem_ctx.mem_pool;

	return mem_pool;
}

static struct hvgr_mem_pool *hvgr_mem_pool_get_dev_pool(struct hvgr_device *gdev,
	uint32_t policy_id)
{
	struct hvgr_mem_pool *mem_pool = NULL;

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	if (policy_id != 0)
		mem_pool = hvgr_sc_mem_find_pool(&gdev->mem_dev.sc_data.mem_pool_list,
			policy_id);
#endif
	if (mem_pool == NULL)
		mem_pool = &gdev->mem_dev.mem_pool;

	return mem_pool;
}

static long hvgr_mem_pool_kernel_alloc(void *target, uint32_t nums, void **array)
{
	struct hvgr_mem_pool *mem_pool = (struct hvgr_mem_pool *)target;
	struct page **pages = (struct page **)array;

	return hvgr_mem_alloc_page(mem_pool->gdev->dev, pages, nums, mem_pool->order,
		mem_pool->policy_id);
}

static void hvgr_mem_pool_kernel_free(void *target, uint32_t nums, void **array, uint32_t flag)
{
	struct hvgr_mem_pool *mem_pool = (struct hvgr_mem_pool *)target;
	struct page **pages = (struct page **)array;

	hvgr_mem_free_page(mem_pool->gdev->dev, pages, nums, mem_pool->order, mem_pool->policy_id);
}

static long hvgr_mem_pool_dev_alloc(void *target, uint32_t nums, void **array)
{
	struct hvgr_mem_pool *mem_pool = (struct hvgr_mem_pool *)target;
	struct hvgr_mem_pool *next_mem_pool =
		hvgr_mem_pool_get_dev_pool(mem_pool->gdev, mem_pool->policy_id);

	return hvgr_pool_alloc(&next_mem_pool->pool, nums, array);
}

static void hvgr_mem_pool_dev_free(void *target, uint32_t nums, void **array, uint32_t flag)
{
	struct hvgr_mem_pool *mem_pool = (struct hvgr_mem_pool *)target;
	struct hvgr_mem_pool *next_mem_pool =
		hvgr_mem_pool_get_dev_pool(mem_pool->gdev, mem_pool->policy_id);

	flag |= HVGR_PAGE_CLEAR;
	hvgr_pool_free(&next_mem_pool->pool, nums, array, flag);
}

static unsigned long hvgr_mem_pool_shrink_free(struct shrinker *shrink,
	struct shrink_control *shrink_c)
{
	struct hvgr_mem_pool *mem_pool = NULL;
	unsigned long free_size;
	uint32_t i;
	struct page *page = NULL;

	mem_pool = container_of(shrink, struct hvgr_mem_pool, sys_shrink);
	mutex_lock(&mem_pool->pool.pool_lock);
	if (mem_pool->refuse_shrink) {
		mutex_unlock(&mem_pool->pool.pool_lock);
		return 0;
	}

	free_size = min((unsigned long)mem_pool->pool.cur_size, shrink_c->nr_to_scan);
	for (i = 0; i < free_size; i++) {
		page = list_first_entry(&mem_pool->pool.list_head, struct page, lru);
		list_del_init(&page->lru);

		hvgr_mem_free_page(mem_pool->gdev->dev, &page, 1u, mem_pool->order,
			mem_pool->policy_id);
	}

	mem_pool->pool.cur_size -= (uint32_t)free_size;
	mutex_unlock(&mem_pool->pool.pool_lock);

	return free_size;
}

static unsigned long hvgr_mem_pool_shrink_count(struct shrinker *shrink,
	struct shrink_control *shrink_c)
{
	struct hvgr_mem_pool *mem_pool = NULL;

	mem_pool = container_of(shrink, struct hvgr_mem_pool, sys_shrink);
	return mem_pool->pool.cur_size;
}

static void hvgr_mem_pool_callback(struct hvgr_pool *pool, void *element, uint32_t flag)
{
	dma_addr_t dma_addr;
	struct hvgr_mem_pool *mem_pool = container_of(pool, struct hvgr_mem_pool, pool);
	struct page *page = (struct page *)element;

	if (page == NULL)
		return;

	if ((flag & HVGR_PAGE_CLEAR) != 0) {
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		hvgr_sc_mem_clear_page(page);
#else
		clear_highpage(page);
#endif
		mem_pool->gdev->mem_dev.statics.pool_clear_pages++;
	}

	dma_addr = hvgr_mem_get_page_dma_addr(page);
	dma_sync_single_for_device(mem_pool->gdev->dev, dma_addr, PAGE_SIZE, DMA_BIDIRECTIONAL);

	if ((flag & HVGR_PAGE_SYNC) != 0)
		mem_pool->gdev->mem_dev.statics.pool_sync_pages++;
}

static long hvgr_mem_pool_init(struct hvgr_mem_pool *mem_pool, struct hvgr_pool_data *data,
	uint32_t pool_size, uint32_t page_order)
{
	int ret;

	mem_pool->policy_id = 0;
	mem_pool->order = page_order;
	mem_pool->sys_shrink.count_objects = hvgr_mem_pool_shrink_count;
	mem_pool->sys_shrink.scan_objects = hvgr_mem_pool_shrink_free;
	mem_pool->sys_shrink.seeks = DEFAULT_SEEKS;
	mem_pool->sys_shrink.batch = 0;

	data->priv = mem_pool;
	data->max_size = pool_size;

	ret = (int)hvgr_pool_init(&mem_pool->pool, data);
	if (ret != 0) {
		hvgr_err(mem_pool->gdev, HVGR_MEM, "dev pool init fail %d", ret);
		return ret;
	}

	ret = register_shrinker(&mem_pool->sys_shrink);
	if (ret != 0) {
		hvgr_pool_term(&mem_pool->pool);
		hvgr_err(mem_pool->gdev, HVGR_MEM, "dev pool regist shrinker fail %d", ret);
	}

	return ret;
}

long hvgr_mem_pool_dev_init(struct hvgr_device * const gdev, struct hvgr_mem_pool *mem_pool,
	uint32_t pool_size, uint32_t page_order)
{
	struct hvgr_pool_data data;

	if (gdev == NULL || mem_pool == NULL || pool_size == 0)
		return -EINVAL;

	mem_pool->gdev = gdev;
	data.alloc = hvgr_mem_pool_kernel_alloc;
	data.free = hvgr_mem_pool_kernel_free;
	data.callback = hvgr_mem_pool_callback;

	return hvgr_mem_pool_init(mem_pool, &data, pool_size, page_order);
}

long hvgr_mem_pool_ctx_init(struct hvgr_ctx * const ctx, struct hvgr_mem_pool *mem_pool,
	uint32_t pool_size, uint32_t page_order)
{
	struct hvgr_pool_data data;

	if (ctx == NULL || mem_pool == NULL || pool_size == 0)
		return -EINVAL;

	mem_pool->gdev = ctx->gdev;
	data.alloc = hvgr_mem_pool_dev_alloc;
	data.free = hvgr_mem_pool_dev_free;
	data.callback = hvgr_mem_pool_callback;

	return hvgr_mem_pool_init(mem_pool, &data, pool_size, page_order);
}

void hvgr_mem_pool_term(struct hvgr_mem_pool *mem_pool)
{
	struct hvgr_pool *pool = NULL;
	void **array = NULL;
	bool array_is_null = false;
	struct page *page = NULL;
	uint32_t i;

	if (mem_pool == NULL)
		return;

	unregister_shrinker(&mem_pool->sys_shrink);

	pool = &mem_pool->pool;
	if (pool->cur_size != 0) {
		array = vzalloc((pool->cur_size * sizeof(void *)));
		if (array == NULL)
			array_is_null = true;

		mutex_lock(&pool->pool_lock);
		for (i = 0; i < pool->cur_size; i++) {
			page = list_first_entry(&pool->list_head, struct page, lru);
			list_del_init(&page->lru);

			if (array_is_null)
				hvgr_mem_free_page(mem_pool->gdev->dev, &page, 1u, mem_pool->order,
					mem_pool->policy_id);
			else
				array[i] = page;
		}

		pool->cur_size = 0u;
		mutex_unlock(&pool->pool_lock);

		if (!array_is_null) {
			pool->free(pool->priv, i, array, false);
			kvfree(array);
		}
	}

	hvgr_pool_term(&mem_pool->pool);
}

long hvgr_mem_pool_ctx_alloc(struct hvgr_ctx *ctx, uint64_t nums, void **array, uint64_t attr)
{
	long ret;
	struct hvgr_mem_pool *mem_pool = NULL;

	if (ctx == NULL || array == NULL)
		return -EINVAL;

	if (nums == 0)
		return 0;

	mem_pool = hvgr_mem_pool_get_ctx_pool(ctx, attr);

	mem_pool->refuse_shrink = true;
	ret = hvgr_pool_alloc(&mem_pool->pool, (uint32_t)nums, array);
	mem_pool->refuse_shrink = false;

	if (ret == 0) {
		atomic_add((int)nums, &ctx->mem_ctx.used_pages);
		atomic_add((int)nums, &ctx->gdev->mem_dev.used_pages);
	}

	hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u page alloc pages %u ret=%d attr=0x%lx",
		ctx->id, nums, ret, attr);
	return ret;
}

void hvgr_mem_pool_ctx_free(struct hvgr_ctx *ctx, uint64_t nums, void **array, uint32_t flag,
	uint64_t attr)
{
	struct hvgr_mem_pool *mem_pool = NULL;
	uint32_t idx;

	if (ctx == NULL || nums == 0 || array == NULL)
		return;

	hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u page free pages %u attr 0x%lx", ctx->id, nums, attr);
	mem_pool = hvgr_mem_pool_get_ctx_pool(ctx, attr);

	mem_pool->refuse_shrink = true;
	hvgr_pool_free(&mem_pool->pool, (uint32_t)nums, array, flag);
	/* clear page array */
	for (idx = 0; idx < nums; idx++)
		array[idx] = NULL;
	mem_pool->refuse_shrink = false;

	atomic_sub((int)nums, &ctx->mem_ctx.used_pages);
	atomic_sub((int)nums, &ctx->gdev->mem_dev.used_pages);
}

struct page *hvgr_mem_pool_ctx_alloc_pt(struct hvgr_ctx *ctx)
{
	long ret;
	struct page *pages = NULL;
	struct hvgr_mem_pool *mem_pool = NULL;

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	mem_pool = hvgr_sc_mem_find_pt_pool(ctx);
#endif
	if (mem_pool == NULL)
		mem_pool = &ctx->mem_ctx.mem_pool;

	mem_pool->refuse_shrink = true;
	ret = hvgr_pool_alloc_direct(&mem_pool->pool, 1, (void **)&pages);
	mem_pool->refuse_shrink = false;
	if (ret != 0)
		return NULL;

	atomic_add(1, &ctx->mem_ctx.used_pages);
	atomic_add(1, &ctx->gdev->mem_dev.used_pages);
	atomic_add(1, &ctx->mem_ctx.used_pt_pages);

	return pages;
}

void hvgr_mem_pool_ctx_free_pt(struct hvgr_ctx * const ctx, struct page *pages)
{
	struct hvgr_mem_pool *mem_pool = NULL;

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	mem_pool = hvgr_sc_mem_find_pt_pool(ctx);
#endif
	if (mem_pool == NULL)
		mem_pool = &ctx->mem_ctx.mem_pool;

	mem_pool->refuse_shrink = true;
	hvgr_pool_free(&mem_pool->pool, 1, (void **)&pages, HVGR_PAGE_SYNC);
	mem_pool->refuse_shrink = false;

	atomic_sub(1, &ctx->mem_ctx.used_pages);
	atomic_sub(1, &ctx->gdev->mem_dev.used_pages);
	atomic_sub(1, &ctx->mem_ctx.used_pt_pages);
}

long hvgr_mem_pool_ctx_grow_pt(struct hvgr_ctx * const ctx, uint32_t nums, struct page **pages)
{
	struct hvgr_mem_pool *mem_pool = NULL;
	long ret;
	uint32_t grow_nums;

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	mem_pool = hvgr_sc_mem_find_pt_pool(ctx);
#endif
	if (mem_pool == NULL)
		mem_pool = &ctx->mem_ctx.mem_pool;

	ret = hvgr_mem_alloc_page(ctx->gdev->dev, pages, nums, mem_pool->order,
		mem_pool->policy_id);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u grow pt pool alloc page fail",
			ctx->id);
		return -ENOMEM;
	}

	mem_pool->refuse_shrink = true;
	grow_nums = hvgr_pool_grow(&mem_pool->pool, nums, (void **)pages);
	mem_pool->refuse_shrink = false;
	if (grow_nums == nums)
		return 0;;

	hvgr_mem_free_page(ctx->gdev->dev, (pages + grow_nums), (nums - grow_nums),
		mem_pool->order, mem_pool->policy_id);
	return 0;
}

static void hvgr_mem_pool_free_pages(struct hvgr_ctx *ctx, struct page **pages,
	struct hvgr_mem_area * const area)
{
	uint64_t i;
	dma_addr_t dma_addr;
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	uint32_t policy_id = hvgr_get_policy_id(area->attribute);
#endif

	for (i = 0u; i < area->pages; i++) {
		if (pages[i] == NULL)
			break;
		dma_addr = hvgr_mem_get_page_dma_addr(pages[i]);
		dma_unmap_page(ctx->gdev->dev, dma_addr, PAGE_SIZE, DMA_BIDIRECTIONAL);

#ifdef HVGR_FEATURE_SYSTEM_CACHE
		hvgr_sc_mem_free_page(pages[i], policy_id, 0);
#else
		__free_pages(pages[i], 0);
#endif
	}
}

struct page **hvgr_mem_alloc_page_array(struct hvgr_ctx *ctx, struct hvgr_mem_area * const area)
{
	struct page **pages = NULL;
	long ret;
	uint32_t policy_id = 0;

	if (area->max_pages > (U64_MAX / sizeof(struct page *))) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alloc page array invalid para",
			ctx->id);
		return NULL;
	}

	if (area->max_pages <= MEM_PHY_ALLOC_THRESHOLD)
		pages = kcalloc(area->max_pages, sizeof(struct page *), GFP_KERNEL);
	else
		pages = vzalloc((area->max_pages * sizeof(struct page *)));
	if (pages == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alloc page array fail 0x%lx",
			ctx->id, area->max_pages);
		return NULL;
	}

	/*
	 * order 0: alloc discontinuous pages from pool.
	 * other order we need to alloc continuous pages from kernel.
	 */
	if (area->order == 0) {
		ret = hvgr_mem_pool_ctx_alloc(ctx, area->pages, (void **)pages,
			area->attribute);
		if (ret != 0)
			hvgr_mem_pool_free_pages(ctx, pages, area);
	} else {
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		policy_id = hvgr_get_policy_id(area->attribute);
#endif
		ret = hvgr_mem_alloc_page(ctx->gdev->dev, pages, (uint32_t)area->pages, area->order,
			policy_id);
		if (ret == 0) {
			atomic_add((int)area->pages, &ctx->mem_ctx.used_pages);
			atomic_add((int)area->pages, &ctx->gdev->mem_dev.used_pages);
		}
	}

	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alloc page fail %d", ctx->id, ret);
		kvfree(pages);
		pages = NULL;
	}

	return pages;
}

void hvgr_mem_free_page_array(struct hvgr_ctx *ctx, struct hvgr_mem_area * const area)
{
	uint32_t policy_id = 0;
	int growable_pages;

	if (area->page_array == NULL)
		return;

	if (area->pages == 0)
		goto free_page_array;

	/*
	 * order 0: free discontinuous pages to pool.
	 * other order we need to free continuous pages to kernel.
	 */
	if (area->order == 0) {
		hvgr_mem_pool_ctx_free(ctx, area->pages, (void **)area->page_array,
			(((area->attribute & HVGR_MEM_ATTR_CPU_CACHEABLE) != 0 ?
				HVGR_PAGE_SYNC : 0)), area->attribute);
	} else {
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		policy_id = hvgr_get_policy_id(area->attribute);
#endif
		hvgr_mem_free_page(ctx->gdev->dev, area->page_array, (uint32_t)area->pages, area->order,
			policy_id);
		atomic_sub((int)area->pages, &ctx->mem_ctx.used_pages);
		atomic_sub((int)area->pages, &ctx->gdev->mem_dev.used_pages);
	}

	growable_pages = atomic_read(&(area->growable_pages));
	if (growable_pages != 0)
		atomic_sub(growable_pages, &ctx->mem_ctx.used_growable_pages);

free_page_array:
	kvfree(area->page_array);
	area->page_array = NULL;
}

