// SPDX-License-Identifier: GPL-2.0
/*
 * ION Memory Allocator system heap exporter
 *
 * Copyright (C) 2011 Google, Inc.
 */

#include <asm/page.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/kthread.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/ion/mm_ion.h>
#include <linux/seq_file.h>
#include <linux/sched/cputime.h>
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_kdump.h>
#ifdef CONFIG_MM_AMA
#include <platform_include/basicplatform/linux/mem_ama.h>
#endif

#include "ion_page_pool.h"
#include "../ion_buffer_reuse.h"
#include "../ion.h"
#include "../ion_media_heap.h"

#define CREATE_TRACE_POINTS
#include "sys_heap_trace.h"

#define NUM_ORDERS ARRAY_SIZE(orders)

static gfp_t high_order_gfp_flags = (GFP_HIGHUSER | __GFP_ZERO | __GFP_NOWARN |
				     __GFP_NORETRY) & ~__GFP_RECLAIM;
static gfp_t low_order_gfp_flags  = GFP_HIGHUSER | __GFP_ZERO;
static const unsigned int orders[] = {9, 8, 4, 0};
static bool install;

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

struct ion_system_heap {
	struct ion_heap heap;
	struct ion_page_pool *pools[NUM_ORDERS];
#ifdef CONFIG_ION_MEDIA_HEAP
	struct ion_page_pool *cma_pools[NUM_ORDERS];
	atomic_t mode_switch;
#endif
	unsigned long pool_watermark;
	struct task_struct *sys_pool_thread;
	wait_queue_head_t sys_pool_wait;
	atomic_t wait_flag;
	struct mutex pool_lock;
#ifdef CONFIG_ZONE_MEDIA_OPT
	atomic64_t cma_page_num;
#endif
};

static struct ion_system_heap *ion_sys_heap;

static int ion_sys_pool_count(struct ion_system_heap *heap)
{
	struct ion_page_pool *pool = NULL;
	int nr_pool_total = 0;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		pool = heap->pools[i];
#ifdef CONFIG_ION_MEDIA_HEAP
		nr_pool_total += ion_page_pool_total(pool, true);
#else
		nr_pool_total += ion_page_pool_total(pool, false);
#endif
	}

	return nr_pool_total;
}

static int fill_pool_once(struct ion_page_pool *pool)
{
	struct page *page = NULL;

#ifdef CONFIG_MM_AMA
	page = ama_alloc_meida_pages(pool->gfp_mask,
					pool->order);
#else
	page = alloc_pages(pool->gfp_mask, pool->order);
#endif
	if (!page)
		return -ENOMEM;

	ion_page_pool_free(pool, page);

	return 0;
}

static void fill_pool_watermark(struct ion_page_pool **pools,
				unsigned long watermark)
{
	unsigned int i;
	unsigned long count;

	for (i = 0; i < NUM_ORDERS; i++) {
		while (watermark) {
#ifdef CONFIG_ION_BUFFER_REUSE
			/*
			 * Skip 0-order pages,
			 * to prevent extra memory pressure
			 */
			if (pools[i]->order == 0)
				break;
#endif
			if (fill_pool_once(pools[i]))
				break;

			count = 1UL << pools[i]->order;
			if (watermark >= count)
				watermark -= count;
			else
				watermark = 0;
		}
	}
}

static void ion_sys_pool_wakeup(void)
{
	struct ion_system_heap *heap = ion_sys_heap;

	atomic_set(&heap->wait_flag, 1);
	wake_up_interruptible(&heap->sys_pool_wait);
}

static int ion_sys_pool_kthread(void *p)
{
	struct ion_system_heap *heap = NULL;
	int ret;
#ifdef CONFIG_ZONE_MEDIA_OPT
	long nr_fill_count = 0;
#endif

	if (!p)
		return -EINVAL;

	heap = (struct ion_system_heap *)p;
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(heap->sys_pool_wait,
						atomic_read(&heap->wait_flag));
		if (ret)
			continue;

		atomic_set(&heap->wait_flag, 0);

#ifdef CONFIG_ZONE_MEDIA_OPT
		nr_fill_count = heap->pool_watermark -
				(unsigned long)ion_sys_pool_count(heap);
#ifdef CONFIG_ION_MEDIA_HEAP
		nr_fill_count -= (unsigned long)ion_media_cma_pool_count(heap->cma_pools);
#endif
		if (nr_fill_count <= 0)
			continue;
		mutex_lock(&heap->pool_lock);

#ifdef CONFIG_ION_MEDIA_HEAP
		fill_media_pool_watermark(heap->pools, heap->cma_pools,
					heap->pool_watermark, nr_fill_count);
#else
		fill_pool_watermark(heap->pools, nr_fill_count);
#endif
		mutex_unlock(&heap->pool_lock);
#else
		mutex_lock(&heap->pool_lock);
		if (heap->pool_watermark) {
#ifdef CONFIG_ION_MEDIA_HEAP
			fill_media_pool_watermark(heap->pools, heap->cma_pools,
					heap->pool_watermark, heap->pool_watermark);
#else
			fill_pool_watermark(heap->pools,
					    heap->pool_watermark);
#endif
		}
		heap->pool_watermark = 0;
		mutex_unlock(&heap->pool_lock);
#endif
	}

	return 0;
}

static bool sys_pool_watermark_check(struct ion_system_heap *heap,
				     unsigned long nr_watermark)
{
	unsigned long nr_pool_count = 0;

	if (!nr_watermark)
		return false;

	if (heap->pool_watermark >= nr_watermark)
		return false;

#ifdef CONFIG_ION_MEDIA_HEAP
	nr_pool_count += (unsigned long)ion_sys_pool_count(heap);
	nr_pool_count += (unsigned long)ion_media_cma_pool_count(heap->cma_pools);
#else
	nr_pool_count = (unsigned long)ion_sys_pool_count(heap);
#endif
	if (nr_pool_count >= nr_watermark)
		return false;

	return true;
}

void set_sys_pool_watermark(unsigned long watermark)
{
	struct ion_system_heap *heap = ion_sys_heap;
	unsigned long nr_watermark = watermark / PAGE_SIZE;
	bool pool_wakeup = true;

#ifdef CONFIG_ION_MEDIA_HEAP
	if (atomic_read(&heap->mode_switch))
		return;
#endif
	if (!wq_has_sleeper(&heap->sys_pool_wait))
		goto drain_pages;

	pool_wakeup = sys_pool_watermark_check(heap, nr_watermark);
	mutex_lock(&heap->pool_lock);
	/*
	 * Maximization principle.
	 */
	if (!nr_watermark || heap->pool_watermark < nr_watermark)
		heap->pool_watermark = nr_watermark;
	mutex_unlock(&heap->pool_lock);

	if (pool_wakeup)
		ion_sys_pool_wakeup();

drain_pages:
	/*
	 * Setting ZERO to watermark,
	 * Meaning that APP does not need the buffer
	 * So we drain all pages.
	 */
	if (!nr_watermark) {
#ifdef CONFIG_ION_MEDIA_HEAP
		ion_media_cma_pool_page_empty(heap->cma_pools, &heap->mode_switch);
#endif
		ion_heap_freelist_drain(&heap->heap, 0);
	}
}

static unsigned long sys_heap_keep_pages;
void fill_system_heap(unsigned long pre_alloc_size)
{
	unsigned long pool_pages;
	unsigned long nr_to_fill;
	unsigned long per_alloc_pages;
	int i;
	struct ion_system_heap *heap = ion_sys_heap;

	sys_heap_keep_pages = pre_alloc_size >> PAGE_SHIFT;
	pool_pages = (unsigned long)ion_sys_pool_count(heap);
	if (pool_pages >= sys_heap_keep_pages)
		return;

	nr_to_fill = sys_heap_keep_pages - pool_pages;
	mutex_lock(&heap->pool_lock);
	for (i = 0; i < NUM_ORDERS; i++) {
		per_alloc_pages = 1 << heap->pools[i]->order;
		while (nr_to_fill > 0 && nr_to_fill >= per_alloc_pages) {
			if (fill_pool_once(heap->pools[i]))
				break;

			nr_to_fill -= per_alloc_pages;

			if (!sys_heap_keep_pages)
				break;
		}

		if (!sys_heap_keep_pages)
			break;
	}

	mutex_unlock(&heap->pool_lock);
	trace_sys_heap_fill_stat(pre_alloc_size >> PAGE_SHIFT,
		(pre_alloc_size >> PAGE_SHIFT) - pool_pages - nr_to_fill);
}

unsigned long get_sys_pool_watermark(void)
{
	unsigned long watermark;
	struct ion_system_heap *heap = ion_sys_heap;

	mutex_lock(&heap->pool_lock);
	watermark = heap->pool_watermark * PAGE_SIZE;
	mutex_unlock(&heap->pool_lock);

	return watermark;
}

#ifndef CONFIG_ION_SYS_HEAP_BULK_SUPPORT
static struct page *alloc_buffer_page(struct ion_system_heap *heap,
				      struct ion_buffer *buffer,
				      unsigned long order)
{
	struct ion_page_pool *pool = heap->pools[order_to_index(order)];
	struct page *page = NULL;
#ifdef CONFIG_ZONE_MEDIA_OPT
	unsigned long cam_flag = buffer->flags & ION_FLAG_CAM_CMA_BUFFER;
	gfp_t gfp_mask = 0;

	if (cam_flag)
		gfp_mask = ___GFP_CMA;
	page = ion_page_pool_alloc_with_gfp(pool, gfp_mask);
	if (page && page_is_cma(page))
		atomic64_add(1 << compound_order(page), &heap->cma_page_num);
#else
	page = ion_page_pool_alloc(pool);
#endif

	CALL_HCK_VH(hck_dfx_kdump_ion_set_page_flag, (gfp_t)(true), page);
	return page;
}
#endif

#ifdef CONFIG_ION_MEDIA_HEAP
atomic_t *get_ion_sys_heap_mode_switch(void)
{
	return &ion_sys_heap->mode_switch;
}

struct ion_page_pool **get_ion_sys_heap_cam_pools(void)
{
	return ion_sys_heap->cma_pools;
}

#ifndef CONFIG_ION_SYS_HEAP_BULK_SUPPORT
struct page *media_alloc_buffer_page(struct ion_buffer *buffer,
				unsigned long order)
{
	struct ion_system_heap *heap = ion_sys_heap;
	struct page *page = NULL;

	page = alloc_buffer_page(heap, buffer, order);
	return page;
}
#endif
#endif

static void free_buffer_page(struct ion_system_heap *heap,
			     struct ion_buffer *buffer, struct page *page)
{
#ifndef CONFIG_ION_MEDIA_HEAP
	struct ion_page_pool *pool;
#endif
	unsigned int order = compound_order(page);
	pgprot_t pgprot;

#ifdef CONFIG_ZONE_MEDIA_OPT
	if (page_is_cma(page))
		atomic64_sub(1 << compound_order(page), &heap->cma_page_num);
#endif
	/* go to system */
	if (buffer->private_flags & ION_PRIV_FLAG_SHRINKER_FREE) {
		__free_pages(page, order);
		return;
	}

#ifdef CONFIG_ION_MEDIA_HEAP
	if (buffer->flags & ION_FLAG_CACHED)
		pgprot = PAGE_KERNEL;
	else
		pgprot = pgprot_writecombine(PAGE_KERNEL);

	if (atomic_read(&heap->mode_switch))
		free_media_page_pool(heap->pools, heap->cma_pools, page, pgprot);
	else
		free_media_page_common(heap->pools, page, pgprot);
#else
	pool = heap->pools[order_to_index(order)];

	if (order == 0 || page_is_cma(page)) {
		ion_page_pool_free_immediate(pool, page);
		return;
	}

	if (buffer->flags & ION_FLAG_CACHED)
		pgprot = PAGE_KERNEL;
	else
		pgprot = pgprot_writecombine(PAGE_KERNEL);

	ion_heap_pages_zero(page, PAGE_SIZE << compound_order(page), pgprot);

	ion_page_pool_free(pool, page);
#endif
}

#ifndef CONFIG_ION_SYS_HEAP_BULK_SUPPORT
static struct page *alloc_largest_available(struct ion_system_heap *heap,
					    struct ion_buffer *buffer,
					    unsigned long size,
					    unsigned int max_order)
{
	struct page *page;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (size < order_to_size(orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		page = alloc_buffer_page(heap, buffer, orders[i]);
		if (!page)
			continue;

		return page;
	}

	return NULL;
}
#endif

static void ion_system_heap_init_buffer_stats(struct ion_buffer *buffer)
{
#ifdef CONFIG_ION_WIDE_INFO
	int order_id;

	for (order_id = 0; order_id < NUM_ORDERS; order_id++)
		buffer->nr_pages_by_order[order_id] = 0;
#else
	(void) buffer;
#endif
}

static void ion_system_heap_update_buffer_stats(struct ion_buffer *buffer,
						int order, int nr)
{
#ifdef CONFIG_ION_WIDE_INFO
	buffer->nr_pages_by_order[order_to_index(order)] += nr;
#else
	(void) buffer;
	(void) order;
	(void) nr;
#endif
}

#ifdef CONFIG_ION_WIDE_INFO
static int ion_system_heap_get_heap_stats(struct ion_heap *heap,
					  struct ion_heap_stats *stats)
{
	int order_id;

	stats->nr_orders = min((int)NUM_ORDERS, ION_BUFFER_MAX_ORDERS);

	for (order_id = 0; order_id < stats->nr_orders; order_id++)
		stats->orders[order_id] = orders[NUM_ORDERS - 1 - order_id];

	spin_lock(&heap->stats_lock);
	stats->sz_allocated = heap->sz_allocated;
	stats->allocation_time = heap->allocation_time;
#ifdef CONFIG_ION_BUFFER_REUSE
	stats->sz_reused = heap->sz_reused;
	stats->reuse_time = heap->reuse_time;
#endif
	spin_unlock(&heap->stats_lock);

	return 0;
}

static int ion_system_heap_get_buffer_stats(struct ion_buffer *buffer,
					    struct ion_buffer_stats *stats)
{
	int order_id;

	stats->nr_orders = min((int)NUM_ORDERS, ION_BUFFER_MAX_ORDERS);

	for (order_id = 0; order_id < stats->nr_orders; order_id++) {
		stats->orders[order_id] = orders[NUM_ORDERS - 1 - order_id];
		stats->nr_pages[order_id] = buffer->nr_pages_by_order[NUM_ORDERS - 1 - order_id];
	}

	return 0;
}
#endif

#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
struct system_heap_alloc_stats {
	atomic64_t alloc_total_size;
	atomic64_t alloc_total_time_us;
	atomic64_t alloc_total_count;
	atomic64_t stat_less_4ms;
	atomic64_t stat_4to6ms;
	atomic64_t stat_6to8ms;
	atomic64_t stat_8to11ms;
	atomic64_t stat_11to16ms;
	atomic64_t stat_greater_16ms;
};

static struct system_heap_alloc_stats system_heap_stat;

#define SYSTEM_HEAP_ALLOC_TIME_THRESH_4MS 4000
#define SYSTEM_HEAP_ALLOC_TIME_THRESH_6MS 6000
#define SYSTEM_HEAP_ALLOC_TIME_THRESH_8MS 8000
#define SYSTEM_HEAP_ALLOC_TIME_THRESH_11MS 11000
#define SYSTEM_HEAP_ALLOC_TIME_THRESH_16MS 16000

static void system_heap_alloc_stat_count(struct system_heap_alloc_stats *stats,
		unsigned long alloc_time, unsigned long alloc_size)
{
	atomic64_add((s64)alloc_size, &stats->alloc_total_size);
	atomic64_add((s64)alloc_time, &stats->alloc_total_time_us);
	atomic64_inc(&stats->alloc_total_count);

	if (likely(alloc_time < SYSTEM_HEAP_ALLOC_TIME_THRESH_4MS))
		atomic64_inc(&stats->stat_less_4ms);
	else if (alloc_time < SYSTEM_HEAP_ALLOC_TIME_THRESH_6MS)
		atomic64_inc(&stats->stat_4to6ms);
	else if (alloc_time < SYSTEM_HEAP_ALLOC_TIME_THRESH_8MS)
		atomic64_inc(&stats->stat_6to8ms);
	else if (alloc_time < SYSTEM_HEAP_ALLOC_TIME_THRESH_11MS)
		atomic64_inc(&stats->stat_8to11ms);
	else if (alloc_time < SYSTEM_HEAP_ALLOC_TIME_THRESH_16MS)
		atomic64_inc(&stats->stat_11to16ms);
	else
		atomic64_inc(&stats->stat_greater_16ms);
}
#endif

#ifdef CONFIG_ION_SYS_HEAP_BULK_SUPPORT
static unsigned long alloc_buffer_pages_bulk(struct ion_system_heap *heap,
					     unsigned long flags,
					     unsigned long order,
					     unsigned long nr_pages,
					     struct list_head *list)
{
	unsigned long nr_allocated;
	struct ion_page_pool *pool = heap->pools[order_to_index(order)];
#ifdef CONFIG_ZONE_MEDIA_OPT
	gfp_t gfp_mask = 0;

	if (flags & ION_FLAG_CAM_CMA_BUFFER)
		gfp_mask = ___GFP_CMA;
	nr_allocated = ion_page_pool_alloc_bulk_with_gfp(pool, nr_pages, list, gfp_mask);
#else
	nr_allocated = ion_page_pool_alloc_bulk(pool, nr_pages, list);
#endif
	return nr_allocated;
}

static unsigned long alloc_buffer_bulk(struct ion_system_heap *heap,
				       struct ion_buffer *buffer,
				       unsigned long size,
				       unsigned int max_order,
				       struct list_head *list,
				       unsigned long *nr_pages,
				       bool clear_buffer_stats)
{
	int i;
	struct list_head chunk;
	unsigned long size_allocated = 0;
	unsigned long nr_allocated_total = 0;

	if (clear_buffer_stats)
		ion_system_heap_init_buffer_stats(buffer);

	for (i = 0; i < NUM_ORDERS; i++) {
		unsigned long nr_to_alloc;
		unsigned long nr_allocated;

		if (size < order_to_size(orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		INIT_LIST_HEAD(&chunk);
		nr_to_alloc = (size - size_allocated) / order_to_size(orders[i]);
		nr_allocated = alloc_buffer_pages_bulk(heap, buffer->flags, orders[i],
				nr_to_alloc, &chunk);
		if (!nr_allocated)
			continue;

		nr_allocated_total += nr_allocated;
		ion_system_heap_update_buffer_stats(buffer, orders[i], nr_allocated);

		list_splice_init(&chunk, list);
		size_allocated += order_to_size(orders[i]) * nr_allocated;
		if (size_allocated >= size)
			break;
	}
	*nr_pages = nr_allocated_total;

	return size_allocated;
}

#ifdef CONFIG_ION_MEDIA_HEAP
static unsigned long media_alloc_buffer_bulk(struct ion_system_heap *heap,
					     struct ion_buffer *buffer,
					     unsigned long size,
					     unsigned int max_order,
					     struct list_head *list,
					     unsigned long *nr_pages)
{
	int i;
	struct list_head chunk;
	unsigned long nr_allocated;
	unsigned long size_allocated = 0;
	unsigned long nr_allocated_total = 0;

	ion_system_heap_init_buffer_stats(buffer);

	if (!check_alloc_path(heap->cma_pools, buffer->flags))
		goto alloc_from_system_pool;

	for (i = 0; i < NUM_ORDERS; i++) {
		unsigned long nr_to_alloc;

		if (size < order_to_size(orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		nr_allocated = 0;
		INIT_LIST_HEAD(&chunk);
		nr_to_alloc = (size - size_allocated) / order_to_size(orders[i]);
		nr_allocated = ion_page_pool_get_bulk(heap->cma_pools[i], nr_to_alloc, &chunk);
		if (!nr_allocated)
			continue;

		nr_allocated_total += nr_allocated;
		ion_system_heap_update_buffer_stats(buffer, orders[i], nr_allocated);

		list_splice_init(&chunk, list);
		size_allocated += order_to_size(orders[i]) * nr_allocated;
		if (size_allocated >= size)
			break;
	}

alloc_from_system_pool:
	if (size_allocated < size) {
		nr_allocated = 0;
		INIT_LIST_HEAD(&chunk);
		size_allocated += alloc_buffer_bulk(heap, buffer,
				size - size_allocated,
				max_order, &chunk, &nr_allocated, false);
		if (nr_allocated) {
			nr_allocated_total += nr_allocated;
			list_splice_init(&chunk, list);
		}
	}
	*nr_pages = nr_allocated_total;

	return size_allocated;
}
#endif

static int ion_system_heap_allocate_bulk(struct ion_heap *heap,
					 struct ion_buffer *buffer,
					 unsigned long size,
					 unsigned long flags)
{
	struct ion_system_heap *sys_heap = container_of(heap,
							struct ion_system_heap,
							heap);
	struct sg_table *table = NULL;
	struct scatterlist *sg;
	struct list_head pages;
	struct page *page, *tmp_page;
	unsigned long i = 0;
	unsigned long size_allocated;
	unsigned long size_pg_aligned = PAGE_ALIGN(size);
	unsigned int max_order = orders[0];
#ifdef CONFIG_ZONE_MEDIA_OPT
	unsigned long nr_cma_pages = 0;
#endif
#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	unsigned long long run_time;
#endif

	if (size / PAGE_SIZE > totalram_pages() / 2)
		return -ENOMEM;

	INIT_LIST_HEAD(&pages);
#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	run_time = task_sched_runtime(current);
#endif
#ifdef CONFIG_ION_MEDIA_HEAP
	size_allocated = media_alloc_buffer_bulk(sys_heap, buffer, size_pg_aligned,
			max_order, &pages, &i);
#else
	size_allocated = alloc_buffer_bulk(sys_heap, buffer, size_pg_aligned,
			max_order, &pages, &i, true);
#endif
#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	run_time = ktime_us_delta(task_sched_runtime(current), run_time);
#endif
	if (size_allocated < size_pg_aligned)
		goto free_pages;

	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		goto free_pages;

	if (sg_alloc_table(table, i, GFP_KERNEL))
		goto free_table;

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
#ifdef CONFIG_ZONE_MEDIA_OPT
		if (page_is_cma(page)) {
			nr_cma_pages += (1 << compound_order(page));
#ifdef CONFIG_ION_BUFFER_REUSE
			buffer->nr_cma_pages += (1 << compound_order(page));
#endif
		}
#endif
		CALL_HCK_VH(hck_dfx_kdump_ion_set_page_flag, (gfp_t)(true), page);
		sg_set_page(sg, page, page_size(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}
#ifdef CONFIG_ZONE_MEDIA_OPT
	atomic64_add(nr_cma_pages, &sys_heap->cma_page_num);
#endif
	buffer->sg_table = table;
#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	system_heap_alloc_stat_count(&system_heap_stat, run_time, PAGE_ALIGN(size));
#endif
	return 0;

free_table:
	kfree(table);
free_pages:
	list_for_each_entry_safe(page, tmp_page, &pages, lru)
		free_buffer_page(sys_heap, buffer, page);
	return -ENOMEM;
}
#else
static int ion_system_heap_allocate(struct ion_heap *heap,
				    struct ion_buffer *buffer,
				    unsigned long size,
				    unsigned long flags)
{
	struct ion_system_heap *sys_heap = container_of(heap,
							struct ion_system_heap,
							heap);
	struct sg_table *table = NULL;
	struct scatterlist *sg;
	struct list_head pages;
	struct page *page, *tmp_page;
	int i = 0;
	unsigned long size_remaining = PAGE_ALIGN(size);
	unsigned int max_order = orders[0];
#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	unsigned long long run_time;
#endif

	if (size / PAGE_SIZE > totalram_pages() / 2)
		return -ENOMEM;

	ion_system_heap_init_buffer_stats(buffer);
	INIT_LIST_HEAD(&pages);
#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	run_time = task_sched_runtime(current);
#endif
	while (size_remaining > 0) {
#ifdef CONFIG_ION_MEDIA_HEAP
		page = alloc_media_largest_available(sys_heap->cma_pools, buffer,
						size_remaining, max_order);
#else
		page = alloc_largest_available(sys_heap, buffer, size_remaining,
					       max_order);
#endif
		if (!page)
			goto free_pages;
		list_add_tail(&page->lru, &pages);
		size_remaining -= page_size(page);
#if (defined(CONFIG_ZONE_MEDIA_OPT) && defined(CONFIG_ION_MEDIA_HEAP))
		if (page_is_cma(page)) {
			atomic64_add(1 << compound_order(page),
				&sys_heap->cma_page_num);
#ifdef CONFIG_ION_BUFFER_REUSE
			buffer->nr_cma_pages += (1 << compound_order(page));
#endif
		} else {
			max_order = compound_order(page);
		}
#else
		max_order = compound_order(page);
#endif
		i++;
		ion_system_heap_update_buffer_stats(buffer, max_order, 1);
	}
#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	run_time = ktime_us_delta(task_sched_runtime(current), run_time);
#endif
	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		goto free_pages;

	if (sg_alloc_table(table, i, GFP_KERNEL))
		goto free_table;

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		sg_set_page(sg, page, page_size(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}

	buffer->sg_table = table;
#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	system_heap_alloc_stat_count(&system_heap_stat, run_time, PAGE_ALIGN(size));
#endif

	return 0;

free_table:
	kfree(table);
free_pages:
	list_for_each_entry_safe(page, tmp_page, &pages, lru)
		free_buffer_page(sys_heap, buffer, page);
	return -ENOMEM;
}
#endif

static void ion_system_heap_free(struct ion_buffer *buffer)
{
	struct ion_system_heap *sys_heap = container_of(buffer->heap,
							struct ion_system_heap,
							heap);
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg;
	int i;

	for_each_sg(table->sgl, sg, table->nents, i)
		free_buffer_page(sys_heap, buffer, sg_page(sg));
	sg_free_table(table);
	kfree(table);
}

static int ion_system_heap_shrink(struct ion_heap *heap, gfp_t gfp_mask,
				  int nr_to_scan)
{
	struct ion_page_pool *pool;
	struct ion_system_heap *sys_heap;
	int nr_total = 0;
	int i, nr_freed;
	int only_scan = 0;
#ifdef CONFIG_ZONE_MEDIA_OPT
	unsigned long pool_pages = 0;
	long gt_watermark_count = 0; /* greater than watermark count */
#endif

	sys_heap = container_of(heap, struct ion_system_heap, heap);

	if (nr_to_scan && (ion_sys_pool_count(sys_heap) < sys_heap_keep_pages))
		return 0;

#ifdef CONFIG_ION_MEDIA_HEAP
	if (atomic_read(&sys_heap->mode_switch))
		return 0;
#endif
	if (!nr_to_scan) {
		only_scan = 1;
	} else {
#ifdef CONFIG_ZONE_MEDIA_OPT
		gt_watermark_count = (unsigned long)ion_sys_pool_count(sys_heap) -
				    sys_heap->pool_watermark;
		if (gt_watermark_count <= 0)
			return 0;
		if (nr_to_scan > gt_watermark_count)
			nr_to_scan = (int)gt_watermark_count;
#endif
	}

	for (i = 0; i < NUM_ORDERS; i++) {
		pool = sys_heap->pools[i];

		if (only_scan) {
			nr_total += ion_page_pool_shrink(pool,
							 gfp_mask,
							 nr_to_scan);

		} else {
			nr_freed = ion_page_pool_shrink(pool,
							gfp_mask,
							nr_to_scan);
			nr_to_scan -= nr_freed;
			nr_total += nr_freed;
			if (nr_to_scan <= 0)
				break;
		}
	}
#ifdef CONFIG_ZONE_MEDIA_OPT
	if (only_scan) {
		pool_pages = max(sys_heap->pool_watermark, sys_heap_keep_pages);
		nr_total -= min(nr_total, pool_pages);
	}
#endif
	return nr_total;
}

static struct ion_heap_ops system_heap_ops = {
#ifdef CONFIG_ION_SYS_HEAP_BULK_SUPPORT
	.allocate = ion_system_heap_allocate_bulk,
#else
	.allocate = ion_system_heap_allocate,
#endif
	.free = ion_system_heap_free,
	.map_kernel = ion_heap_map_kernel,
	.unmap_kernel = ion_heap_unmap_kernel,
	.map_user = ion_heap_map_user,
	.shrink = ion_system_heap_shrink,
};

static int ion_system_heap_debug_show(struct ion_heap *heap, struct seq_file *s,
				      void *unused)
{
	struct ion_system_heap *sys_heap = container_of(heap,
							struct ion_system_heap,
							heap);
	int i;
	struct ion_page_pool *pool;
#ifdef CONFIG_ION_MEDIA_HEAP
	struct ion_page_pool *cma_pool;
#endif
#ifdef CONFIG_ION_MEDIA_HEAP
	seq_printf(s, "CMA pools info:\n");
	for (i = 0; i < NUM_ORDERS; i++) {
		cma_pool = sys_heap->cma_pools[i];

		seq_printf(s, "%d order %u highmem pages %lu total\n",
			   cma_pool->high_count, cma_pool->order,
			   (PAGE_SIZE << cma_pool->order) * cma_pool->high_count);
		seq_printf(s, "%d order %u lowmem pages %lu total\n",
			   cma_pool->low_count, cma_pool->order,
			   (PAGE_SIZE << cma_pool->order) * cma_pool->low_count);
	}
#endif
	seq_printf(s, "Normal pools info:\n");
	for (i = 0; i < NUM_ORDERS; i++) {
		pool = sys_heap->pools[i];

		seq_printf(s, "%d order %u highmem pages %lu total\n",
			   pool->high_count, pool->order,
			   (PAGE_SIZE << pool->order) * pool->high_count);
		seq_printf(s, "%d order %u lowmem pages %lu total\n",
			   pool->low_count, pool->order,
			   (PAGE_SIZE << pool->order) * pool->low_count);
	}

#ifdef CONFIG_ZONE_MEDIA_OPT
	seq_printf(s, "%ld cma pages allocated by camera\n",
		   atomic64_read(&sys_heap->cma_page_num));
#endif
#ifdef CONFIG_ION_MEDIA_HEAP
	seq_printf(s, "%d mode_switch state\n",
		   atomic_read(&sys_heap->mode_switch));
#endif

#ifdef CONFIG_ION_SYSTEM_HEAP_STAT
	seq_printf(s, "%16s %22s %10s %10s %10s %10s %10s %10s %10s\n",
			"alloc_total_size", "total_alloc_time_us",
			"total_count", "less_4ms", "4to6ms", "6to8ms",
			"8to11ms", "11to16ms", "great16ms");
	seq_printf(s, "%16llu %22llu %10llu %10llu %10llu %10llu %10llu %10llu %10llu\n",
			atomic64_read(&system_heap_stat.alloc_total_size),
			atomic64_read(&system_heap_stat.alloc_total_time_us),
			atomic64_read(&system_heap_stat.alloc_total_count),
			atomic64_read(&system_heap_stat.stat_less_4ms),
			atomic64_read(&system_heap_stat.stat_4to6ms),
			atomic64_read(&system_heap_stat.stat_6to8ms),
			atomic64_read(&system_heap_stat.stat_8to11ms),
			atomic64_read(&system_heap_stat.stat_11to16ms),
			atomic64_read(&system_heap_stat.stat_greater_16ms));
#endif

	return 0;
}

static void ion_system_heap_destroy_pools(struct ion_page_pool **pools)
{
	int i;

	for (i = 0; i < NUM_ORDERS; i++)
		if (pools[i])
			ion_page_pool_destroy(pools[i]);
}

#ifdef CONFIG_ION_MEDIA_HEAP
static int ion_system_heap_create_pools(struct ion_page_pool **pools,
					bool is_cma_pool)
#else
static int ion_system_heap_create_pools(struct ion_page_pool **pools)
#endif
{
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		struct ion_page_pool *pool;
		gfp_t gfp_flags = low_order_gfp_flags;

		if (orders[i] >= 4)
			gfp_flags = high_order_gfp_flags;

		if (orders[i] >= 8)
			gfp_flags = high_order_gfp_flags & ~__GFP_RECLAIMABLE;

#ifdef CONFIG_ION_MEDIA_HEAP
		if (is_cma_pool)
			gfp_flags |= ___GFP_CMA;
#endif
		pool = ion_page_pool_create(gfp_flags, orders[i]);
		if (!pool)
			goto err_create_pool;
		pools[i] = pool;
	}

	return 0;

err_create_pool:
	ion_system_heap_destroy_pools(pools);
	return -ENOMEM;
}

struct ion_heap *ion_system_heap_create(struct ion_platform_heap *unused_data)
{
	struct ion_system_heap *heap;

	heap = kzalloc(sizeof(*heap), GFP_KERNEL);
	if (!heap)
		return ERR_PTR(-ENOMEM);
	heap->heap.ops = &system_heap_ops;
	heap->heap.type = ION_HEAP_TYPE_SYSTEM;
#ifdef CONFIG_ION_BUFFER_REUSE
	heap->heap.flags = ION_HEAP_FLAG_DEFER_FREE |
			   ION_HEAP_FLAG_FREE_BUF_REUSE;
#else
	heap->heap.flags = ION_HEAP_FLAG_DEFER_FREE;
#endif

#ifdef CONFIG_ION_MEDIA_HEAP
	if (ion_system_heap_create_pools(heap->pools, false))
		goto free_heap;

	if (ion_system_heap_create_pools(heap->cma_pools, true))
#else
	if (ion_system_heap_create_pools(heap->pools))
#endif
		goto free_heap;

	atomic_set(&heap->wait_flag, 0);
#ifdef CONFIG_ION_MEDIA_HEAP
	atomic_set(&heap->mode_switch, 0);
#endif
#ifdef CONFIG_ZONE_MEDIA_OPT
	atomic64_set(&heap->cma_page_num, 0);
#endif
	init_waitqueue_head(&heap->sys_pool_wait);
	heap->sys_pool_thread = kthread_run(ion_sys_pool_kthread, heap,
						"%s", "sys_pool");
	if (IS_ERR(heap->sys_pool_thread)) {
		pr_err("%s: kthread_create failed!\n", __func__);
		goto destroy_pools;
	}
	mutex_init(&heap->pool_lock);

	heap->heap.debug_show = ion_system_heap_debug_show;
#ifdef CONFIG_ION_WIDE_INFO
	heap->heap.get_heap_stats = ion_system_heap_get_heap_stats;
	heap->heap.get_buffer_stats = ion_system_heap_get_buffer_stats;
#endif
	ion_sys_heap = heap;

	return &heap->heap;

destroy_pools:
	ion_system_heap_destroy_pools(heap->pools);
#ifdef CONFIG_ION_MEDIA_HEAP
	ion_system_heap_destroy_pools(heap->cma_pools);
#endif
free_heap:
	kfree(heap);
	return ERR_PTR(-ENOMEM);
}

static int ion_system_contig_heap_allocate(struct ion_heap *heap,
					   struct ion_buffer *buffer,
					   unsigned long len,
					   unsigned long flags)
{
	int order = get_order(len);
	struct page *page;
	struct sg_table *table = NULL;
	unsigned long i;
	int ret;

	page = alloc_pages(low_order_gfp_flags | __GFP_NOWARN, order);
	if (!page)
		return -ENOMEM;

	split_page(page, order);

	len = PAGE_ALIGN(len);
	for (i = len >> PAGE_SHIFT; i < (1 << order); i++)
		__free_page(page + i);

	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table) {
		ret = -ENOMEM;
		goto free_pages;
	}

	ret = sg_alloc_table(table, 1, GFP_KERNEL);
	if (ret)
		goto free_table;

	sg_set_page(table->sgl, page, len, 0);

	buffer->sg_table = table;

	return 0;

free_table:
	kfree(table);
free_pages:
	for (i = 0; i < len >> PAGE_SHIFT; i++)
		__free_page(page + i);

	return ret;
}

static void ion_system_contig_heap_free(struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;
	struct page *page = sg_page(table->sgl);
	unsigned long pages = PAGE_ALIGN(buffer->size) >> PAGE_SHIFT;
	unsigned long i;

	for (i = 0; i < pages; i++)
		__free_page(page + i);
	sg_free_table(table);
	kfree(table);
}


static struct ion_heap_ops kmalloc_ops = {
	.allocate = ion_system_contig_heap_allocate,
	.free = ion_system_contig_heap_free,
	.map_kernel = ion_heap_map_kernel,
	.unmap_kernel = ion_heap_unmap_kernel,
	.map_user = ion_heap_map_user,
};

static struct ion_heap *__ion_system_contig_heap_create(void)
{
	struct ion_heap *heap;

	heap = kzalloc(sizeof(*heap), GFP_KERNEL);
	if (!heap)
		return ERR_PTR(-ENOMEM);
	heap->ops = &kmalloc_ops;
	heap->type = ION_HEAP_TYPE_SYSTEM_CONTIG;
	heap->name = "ion_system_contig_heap";
	return heap;
}

static int __init ion_system_contig_heap_cfg(char *param)
{
	pr_err("%s: param is =%s\n", __func__, param);
	if (strcmp(param, "y") == 0)
		install = true;
	else
		install = false;
	return 0;
}

early_param("contigheap", ion_system_contig_heap_cfg);

static int ion_system_contig_heap_create(void)
{
	struct ion_heap *heap;

	if (!install) {
		pr_err("%s: skip contig heap create.\n", __func__);
		return 0;
	}

	heap = __ion_system_contig_heap_create();
	if (IS_ERR(heap))
		return PTR_ERR(heap);

	ion_device_add_heap(heap);
	return 0;
}
device_initcall(ion_system_contig_heap_create);
