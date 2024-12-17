/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_IMPORT_H
#define HVGR_MEM_IMPORT_H

#include "hvgr_defs.h"

bool hvgr_mem_import_config(struct hvgr_mem_ctx * const pmem);

uint64_t hvgr_mem_import_alloc_va(struct hvgr_mem_area * const area);

void hvgr_mem_import_free_pages(struct hvgr_mem_area *area);

void hvgr_ion_mem_reclaim_init(void);

void hvgr_add_bg_dmabuf_ctx_to_global_list(struct hvgr_ctx * const ctx);

void hvgr_del_bg_dmabuf_ctx_from_global_list(struct hvgr_ctx * const ctx);

void hvgr_mem_import_deconfig(struct hvgr_mem_ctx * const pmem);


/* Release tlb and then free physical pages. */
#define HVGR_DMA_RELEASE_TLB   0
/* If release tlb failed, rollback the tlb and the physical pages will not be freed. */
#define HVGR_DMA_ROLLBACK_TLB  1
/* New physical pages allocated and then resotre the tlb. */
#define HVGR_DMA_RESTORE_TLB   2

#endif
