/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_SC_H
#define HVGR_MEM_SC_H

#include "hvgr_defs.h"

struct hvgr_sc_mem_pool {
	struct list_head node;
	uint32_t policy_id;
	struct hvgr_mem_pool pool;
};

struct hvgr_mem_pool *hvgr_sc_mem_find_pool(struct list_head *head, uint32_t policy_id);

struct hvgr_mem_pool *hvgr_sc_mem_find_pt_pool(struct hvgr_ctx *ctx);

long hvgr_sc_mem_init_dev_pool(struct hvgr_device * const gdev, uint32_t pool_size);

long hvgr_sc_mem_init_ctx_pool(struct hvgr_ctx *ctx, uint32_t pool_size);

void hvgr_sc_mem_deinit_ctx_pool(struct hvgr_ctx *ctx);

void hvgr_sc_mem_init(struct hvgr_device * const gdev);

void hvgr_sc_mem_deinit(struct hvgr_device * const gdev);

struct page *hvgr_sc_mem_alloc_page(uint32_t policy_id, uint32_t order, gfp_t gfp);

void hvgr_sc_mem_free_page(struct page *page, uint32_t policy_id, uint32_t order);

void hvgr_sc_mem_clear_page(struct page *page);

void hvgr_enable_sc_lb(struct hvgr_device *gdev, unsigned int enable);

#endif
