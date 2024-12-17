/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_SPARSE_H
#define HVGR_MEM_SPARSE_H

#include "hvgr_defs.h"

struct hvgr_mem_sparse_node {
	/* sparse gva is the key in node */
	struct hvgr_kv_node node;
	/* sparse area */
	struct hvgr_mem_area *sparse_area;
	/* referenced area */
	struct hvgr_mem_area *ref_area;
};

struct hvgr_mem_sparse_cb {
	struct hvgr_kv_map sparse_rec;
};

bool hvgr_mem_sparse_config(struct hvgr_mem_ctx * const pmem);

void hvgr_mem_sparse_deconfig(struct hvgr_mem_ctx * const pmem);
/*
 * hvgr_mem_sparse_bind - sparse mem bind
 * @ctx: Pointer to context
 * @sparse_addr: sparse mem gpu va
 * @mem_addr: other type mem gpu va, this mem can not be sparse mem
 * @num: number of page to bind
 *
 * This function find hvgr_mem_area base on the input gpu va, then create a mapping
 * between sparse mem va and other mem's page on the GPU side.
 */
int hvgr_mem_sparse_bind(struct hvgr_ctx *ctx, u64 sparse_addr, u64 mem_addr, u64 num);

/*
 * hvgr_mem_sparse_unbind - sparse mem unbind
 * @ctx: Pointer to context
 * @sparse_addr: sparse mem gpu va
 * @num: number of page to unbind
 *
 * This function find hvgr_mem_area base on the input gpu va, then replace the old
 * sparse mem mapping with sparse mem va and zero page on the GPU side.
 */
int hvgr_mem_sparse_unbind(struct hvgr_ctx *ctx, u64 sparse_addr, u64 num);

/*
 * hvgr_mem_sparse_unbind_nolock - sparse mem unbind without lock
 * @ctx: Pointer to context
 * @sparse_addr: sparse mem gpu va
 * @num: number of page to unbind
 *
 * This function find hvgr_mem_area base on the input gpu va, then replace the old
 * sparse mem mapping with sparse mem va and zero page on the GPU side.
 */
int hvgr_mem_sparse_unbind_nolock(struct hvgr_ctx *ctx, u64 sparse_addr, u64 num);

/*
 * hvgr_mem_sparse_alloc_zero_page - alloc zero page
 * @gdev: Pointer to hvgr_device
 *
 * This function will alloc one page for sparse mem to use when mem is unbind.
 */
int hvgr_mem_sparse_alloc_zero_page(struct hvgr_device * const gdev);

/*
 * hvgr_mem_sparse_free_zero_page - free zero page
 * @gdev: Pointer to hvgr_device
 *
 * This function will free the page which is used by sparse mem.
 */
void hvgr_mem_sparse_free_zero_page(struct hvgr_device * const gdev);

void hvgr_mem_sparse_term(struct hvgr_ctx *ctx);

int hvgr_sparse_mem_change_attr(struct hvgr_ctx *ctx,
	struct hvgr_mem_area *area, uint64_t attr);
#endif
