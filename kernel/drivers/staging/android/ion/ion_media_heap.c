// SPDX-License-Identifier: GPL-2.0
/*
 * ION Memory Allocator system heap exporter
 *
 * Copyright (C) 2011 Google, Inc.
 */

#include <asm/page.h>
#include <linux/err.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/of_reserved_mem.h>
#include "heaps/ion_page_pool.h"
#include "heaps/ion_system_heap.h"
#include "ion.h"
#include "ion_media_heap.h"
#ifdef CONFIG_MM_VIP
#include <platform_include/basicplatform/linux/mmvip_scene.h>
#endif

#define NUM_ORDERS ARRAY_SIZE(orders)
#define CAM_WATER_MARK (SZ_1M * 236) /* 236MB */
static const unsigned int orders[] = {9, 8, 4, 0};

static int order_to_index(unsigned int order)
{
	int i;

	for (i = 0; i < NUM_ORDERS; i++)
		if (order == orders[i])
			return i;
	BUG();
	return -1;
}

static inline unsigned int order_to_size(int order)
{
	return PAGE_SIZE << order;
}

int ion_media_cma_pool_count(struct ion_page_pool **cma_pools)
{
	struct ion_page_pool *cma_pool = NULL;
	int nr_pool_total = 0;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		cma_pool = cma_pools[i];
		nr_pool_total += ion_page_pool_total(cma_pool, true);
	}

	return nr_pool_total;
}
static int fill_pool_once(unsigned long pool_watermark,
			struct ion_page_pool *pool,
			struct ion_page_pool *cma_pool)
{
	struct page *page = NULL;
	gfp_t gfp_mask = 0;

	if (pool_watermark == CAM_WATER_MARK / PAGE_SIZE)
		gfp_mask = cma_pool->gfp_mask;
	else
		gfp_mask = pool->gfp_mask;
#ifdef CONFIG_MM_VIP
	if (is_mm_vip_scene())
		gfp_mask |= ___GFP_VIP;
#endif
#ifdef CONFIG_ION_BUFFER_REUSE
	/*
	 * Skip 0-order pages,
	 * to prevent extra memory pressure
	 */
	if (pool->order == 0)
		return -ENOMEM;
#endif
	page = alloc_pages(gfp_mask, pool->order);
	if (!page)
		return -ENOMEM;

	if (page_is_cma(page))
		ion_page_pool_free(cma_pool, page);
	else
		ion_page_pool_free(pool, page);

	return 0;
}

void fill_media_pool_watermark(struct ion_page_pool **pools, struct ion_page_pool **cma_pools,
				unsigned long pool_watermark, unsigned long watermark)
{
	unsigned int i;
	unsigned long count;
	struct ion_page_pool *pool = NULL;
	struct ion_page_pool *cma_pool = NULL;

	for (i = 0; i < NUM_ORDERS; i++) {
		pool = pools[i];
		cma_pool = cma_pools[i];

		while (watermark) {
			if (fill_pool_once(pool_watermark, pool, cma_pool))
				break;

			count = 1UL << orders[i];
			if (watermark >= count)
				watermark -= count;
			else
				watermark = 0;
		}
	}
}

void ion_media_cma_pool_page_empty(struct ion_page_pool **cma_pools, atomic_t *mode_switch)
{
	int nr_to_free = ion_media_cma_pool_count(cma_pools);
	int nr_to_scan = 0;
	int freed = 0;
	int i;

	if (atomic_read(mode_switch) || !nr_to_free)
		return;
retry:
	for (i = 0; i < NUM_ORDERS; i++) {
		nr_to_scan = ion_page_pool_total(cma_pools[i], true);
		freed += ion_page_pool_shrink(cma_pools[i], 0, nr_to_scan);
	}

	if (ion_media_cma_pool_count(cma_pools)) {
		pr_err("%s: freed = %d, nr_to_free = %d, now all = %d\n",
			__func__, freed, nr_to_free,
			ion_media_cma_pool_count(cma_pools));
		goto retry;
	}
}

void set_media_heap_mode(size_t mode)
{
	atomic_t *mode_switch  = get_ion_sys_heap_mode_switch();
	struct ion_page_pool **cma_pools = get_ion_sys_heap_cam_pools();

	pr_err("%s: input = 0x%x\n", __func__, mode);
	if (mode == MEDIA_MODE1) {
		atomic_set(mode_switch, 1);
	} else if (mode == MEDIA_MODE2) {
		atomic_set(mode_switch, 0);
		ion_media_cma_pool_page_empty(cma_pools, mode_switch);
	} else {
		set_sys_pool_watermark(mode);
	}
}

void free_media_page_common(struct ion_page_pool **pools,
			struct page *page, pgprot_t pgprot)
{
	struct ion_page_pool *pool = NULL;
	unsigned int order = compound_order(page);

	pool = pools[order_to_index(order)];
	if (order == 0 || page_is_cma(page)) {
		ion_page_pool_free_immediate(pool, page);
		return;
	}

	ion_heap_pages_zero(page, PAGE_SIZE << compound_order(page), pgprot);
	ion_page_pool_free(pool, page);
}

void free_media_page_pool(struct ion_page_pool **pools, struct ion_page_pool **cma_pools,
			struct page *page, pgprot_t pgprot)
{
	struct ion_page_pool *pool = NULL;
	struct ion_page_pool *pool_cma = NULL;
	unsigned int order = compound_order(page);

	pool = pools[order_to_index(order)];
	pool_cma = cma_pools[order_to_index(order)];

	ion_heap_pages_zero(page, PAGE_SIZE << compound_order(page), pgprot);
	if (page_is_cma(page))
		ion_page_pool_free(pool_cma, page);
	else
		ion_page_pool_free(pool, page);
}

#ifndef CONFIG_ION_SYS_HEAP_BULK_SUPPORT
static struct page *alloc_buffer_page_order(struct ion_page_pool **cma_pools,
			struct ion_buffer *buffer, unsigned long size,
			unsigned int max_order, bool use_cma_pool)
{
	struct page *page = NULL;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (size < order_to_size(orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		if (use_cma_pool)
			page = ion_page_pool_get(cma_pools[i]);
		else
			page = media_alloc_buffer_page(buffer, orders[i]);
		if (!page)
			continue;

		return page;
	}

	return NULL;
}
#endif

bool check_alloc_path(struct ion_page_pool **cma_pools, unsigned long flag)
{
#ifdef CONFIG_ZONE_MEDIA_OPT
	if (!(flag & ION_FLAG_CAM_CMA_BUFFER))
		return false;
#endif

	if (!ion_media_cma_pool_count(cma_pools))
		return false;

	return true;
}

#ifndef CONFIG_ION_SYS_HEAP_BULK_SUPPORT
struct page *alloc_media_largest_available(struct ion_page_pool **cma_pools,
					    struct ion_buffer *buffer,
					    unsigned long size,
					    unsigned int max_order)
{
	struct page *page = NULL;

	if (check_alloc_path(cma_pools, buffer->flags))
		page = alloc_buffer_page_order(cma_pools, buffer, size,
				max_order, true);

	if (!page)
		page = alloc_buffer_page_order(cma_pools, buffer, size,
				max_order, false);

	if (!page)
		pr_err("%s: normal size 0x%llx\n", __func__, size);

	return page;
}
#endif
