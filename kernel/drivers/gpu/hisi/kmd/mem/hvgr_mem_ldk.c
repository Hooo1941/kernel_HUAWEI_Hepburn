/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>

#include "hvgr_mem_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_memory_page.h"

#ifdef CONFIG_HVGR_ASYNC_MEM_RECYCLE

#define ASYNC_MEM_RECYCLE_THRESHOLD 64

void hvgr_mem_free_page(struct device *dev, struct page **pages, uint32_t nums,
	uint32_t order, uint32_t policy_id)
{
	dma_addr_t dma_addr;
	uint32_t steps = nums >> order;
	uint32_t step;
	struct page *page = NULL;
	struct hvgr_device *gdev = hvgr_get_device();

	if (steps >= ASYNC_MEM_RECYCLE_THRESHOLD) {
		spin_lock(&gdev->mem_dev.recycle_lock);
		for (step = 0; step < steps; step++) {
			page = pages[step << order];
			pages[step << order] = NULL;
			page->index = order;
			page->private = policy_id;
			list_add_tail(&page->lru, &gdev->mem_dev.recycle_list);
		}
		spin_unlock(&gdev->mem_dev.recycle_lock);
		wake_up(&gdev->mem_dev.recycle_wq);
		return;
	}

	for (step = 0; step < steps; step++) {
		page = pages[step << order];
		pages[step << order] = NULL;
		dma_addr = hvgr_mem_get_page_dma_addr(page);
		dma_unmap_page(dev, dma_addr, (PAGE_SIZE << order), DMA_BIDIRECTIONAL);
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		hvgr_sc_mem_free_page(page, policy_id, order);
#else
		__free_pages(page, order);
#endif
	}

	if (gdev != NULL) {
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		if (policy_id != 0)
			gdev->mem_dev.statics.sc_free_pages += nums;
		else
#endif
			gdev->mem_dev.statics.free_pages += nums;
	}
}

static int hvgr_mem_recycle(struct hvgr_device * const gdev, struct list_head *list)
{
	spin_lock(&gdev->mem_dev.recycle_lock);
	list_splice_tail_init(&gdev->mem_dev.recycle_list, list);
	spin_unlock(&gdev->mem_dev.recycle_lock);

	return !list_empty(list);
}

static int hvgr_mem_recycle_task(void *data)
{
	struct hvgr_device * const gdev = data;
	struct page *page = NULL;
	uint32_t order, policy_id;
	dma_addr_t addr;
	LIST_HEAD(list);

	while (!kthread_should_stop()) {
		wait_event(gdev->mem_dev.recycle_wq,
			hvgr_mem_recycle(gdev, &list) || kthread_should_stop());

		while (!list_empty(&list)) {
			page = list_first_entry(&list, struct page, lru);
			list_del_init(&page->lru);
			order = (uint32_t)page->index;
			policy_id = (uint32_t)page->private;
			addr = hvgr_mem_get_page_dma_addr(page);
			dma_unmap_page(gdev->dev, addr, (PAGE_SIZE << order), DMA_BIDIRECTIONAL);
#ifdef HVGR_FEATURE_SYSTEM_CACHE
			hvgr_sc_mem_free_page(page, policy_id, order);
			if (policy_id != 0)
				gdev->mem_dev.statics.sc_free_pages += 1 << order;
			else
				gdev->mem_dev.statics.free_pages += 1 << order;
#else
			__free_pages(page, order);
			gdev->mem_dev.statics.free_pages += 1 << order;
#endif
		}
	}

	return 0;
}

int hvgr_mem_recycle_init(struct hvgr_device * const gdev)
{
	spin_lock_init(&gdev->mem_dev.recycle_lock);
	INIT_LIST_HEAD(&gdev->mem_dev.recycle_list);
	init_waitqueue_head(&gdev->mem_dev.recycle_wq);

	gdev->mem_dev.recycle_task = kthread_run(hvgr_mem_recycle_task, (void *)gdev,
		"hvgr-recycle");
	if (IS_ERR(gdev->mem_dev.recycle_task)) {
		hvgr_err(gdev, HVGR_MEM, "failed to create recycle kthread");
		gdev->mem_dev.recycle_task = NULL;
		return -1;
	}

	return 0;
}

void hvgr_mem_recycle_term(struct hvgr_device * const gdev)
{
	if (gdev->mem_dev.recycle_task != NULL) {
		kthread_stop(gdev->mem_dev.recycle_task);
		gdev->mem_dev.recycle_task = NULL;
	}
}

#endif /* CONFIG_HVGR_ASYNC_MEM_RECYCLE */
