/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_memory_page.h"

#include <linux/slab.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_MM_AMA
#include <platform_include/basicplatform/linux/mem_ama.h>
#endif

#include "hvgr_mem_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"

#ifndef CONFIG_HVGR_ASYNC_MEM_RECYCLE
void hvgr_mem_free_page(struct device *dev, struct page **pages, uint32_t nums,
	uint32_t order, uint32_t policy_id)
{
	dma_addr_t dma_addr;
	uint32_t steps = nums >> order;
	uint32_t step;
	struct page *page = NULL;
	struct hvgr_device *gdev = hvgr_get_device();

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
#endif

long hvgr_mem_alloc_page(struct device *dev, struct page **pages,
	uint32_t nums, uint32_t order, uint32_t policy_id)
{
	uint32_t step;
	uint32_t i;
	dma_addr_t dma_addr;
	gfp_t gfp = GFP_HIGHUSER | __GFP_ZERO;
	struct page *page = NULL;
	uint32_t steps = nums >> order;
	struct hvgr_device *gdev = hvgr_get_device();

	if (order != 0)
		gfp |= __GFP_NOWARN;

	for (step = 0; step < steps; step++) {
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		page = hvgr_sc_mem_alloc_page(policy_id, order, gfp);
#else
#ifdef CONFIG_MM_AMA
		page = ama_alloc_meida_pages(gfp, order);
#else
		page = alloc_pages(gfp, order);
#endif
#endif
		if (page == NULL) {
			hvgr_mem_free_page(dev, pages, (step << order), order, policy_id);
			hvgr_err(gdev, HVGR_MEM, "alloc step %u steps %u order %u nums %u fail",
				step, steps, order, nums);
			return -ENOMEM;
		}

		dma_addr = dma_map_page(dev, page, 0, (PAGE_SIZE << order), DMA_BIDIRECTIONAL);
		if (dma_mapping_error(dev, dma_addr)) {
#ifdef HVGR_FEATURE_SYSTEM_CACHE
			hvgr_sc_mem_free_page(page, policy_id, order);
#else
			__free_pages(page, order);
#endif
			hvgr_mem_free_page(dev, pages, (step << order), order, policy_id);
			hvgr_err(gdev, HVGR_MEM, "dma map step %u steps %u order %u nums %u fail",
				step, steps, order, nums);
			return -EFAULT;
		}

		WARN_ON(dma_addr != page_to_phys(page));
		for (i = 0; i < (1u << order); i++)
			pages[(step << order) + i] = page + i;
	}

	if (gdev != NULL) {
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		if (policy_id != 0)
			gdev->mem_dev.statics.sc_alloc_pages += nums;
		else
#endif
			gdev->mem_dev.statics.alloc_pages += nums;
	}
	return 0;
}

unsigned long hvgr_mem_statics_used_pages(void)
{
	struct hvgr_device *dev = hvgr_get_device();

	if (dev == NULL)
		return 0;

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	return (((dev->mem_dev.statics.alloc_pages > dev->mem_dev.statics.free_pages) ?
		(dev->mem_dev.statics.alloc_pages - dev->mem_dev.statics.free_pages) : 0) +
		((dev->mem_dev.statics.sc_alloc_pages > dev->mem_dev.statics.sc_free_pages) ?
		(dev->mem_dev.statics.sc_alloc_pages - dev->mem_dev.statics.sc_free_pages) : 0));
#else
	return ((dev->mem_dev.statics.alloc_pages > dev->mem_dev.statics.free_pages) ?
		(dev->mem_dev.statics.alloc_pages - dev->mem_dev.statics.free_pages) : 0);
#endif
}
