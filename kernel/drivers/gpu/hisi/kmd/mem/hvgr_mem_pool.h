/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEMORY_POOL_H
#define HVGR_MEMORY_POOL_H

#include "hvgr_defs.h"

#define HVGR_MEM_POOL_DEV_SIZE ((64 * 1024 * 1024) >> 12)
#define HVGR_MEM_POOL_CTX_SIZE ((64 * 1024 * 1024) >> 12)

long hvgr_mem_pool_dev_init(struct hvgr_device * const gdev, struct hvgr_mem_pool *mem_pool,
	uint32_t pool_size, uint32_t page_order);

long hvgr_mem_pool_ctx_init(struct hvgr_ctx * const ctx, struct hvgr_mem_pool *mem_pool,
	uint32_t pool_size, uint32_t page_order);

void hvgr_mem_pool_term(struct hvgr_mem_pool *mem_pool);

long hvgr_mem_pool_ctx_alloc(struct hvgr_ctx *ctx, uint64_t nums, void **array, uint64_t attr);

void hvgr_mem_pool_ctx_free(struct hvgr_ctx *ctx, uint64_t nums, void **array, uint32_t flag,
	uint64_t attr);

struct page **hvgr_mem_alloc_page_array(struct hvgr_ctx *ctx, struct hvgr_mem_area * const area);

void hvgr_mem_free_page_array(struct hvgr_ctx *ctx, struct hvgr_mem_area * const area);

struct page *hvgr_mem_pool_ctx_alloc_pt(struct hvgr_ctx *ctx);

void hvgr_mem_pool_ctx_free_pt(struct hvgr_ctx * const ctx, struct page *pages);

long hvgr_mem_pool_ctx_grow_pt(struct hvgr_ctx * const ctx, uint32_t nums, struct page **pages);

#endif
