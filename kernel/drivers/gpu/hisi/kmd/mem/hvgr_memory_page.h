/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEMORY_PAGE_H
#define HVGR_MEMORY_PAGE_H

#include <linux/mm.h>
#include <linux/device.h>

void hvgr_mem_free_page(struct device *dev, struct page **pages, uint32_t nums,
	uint32_t order, uint32_t policy_id);

long hvgr_mem_alloc_page(struct device *dev, struct page **pages,
	uint32_t nums, uint32_t order, uint32_t policy_id);

#ifdef CONFIG_HVGR_ASYNC_MEM_RECYCLE
struct hvgr_device;
int hvgr_mem_recycle_init(struct hvgr_device * const gdev);
void hvgr_mem_recycle_term(struct hvgr_device * const gdev);
#endif

#endif
