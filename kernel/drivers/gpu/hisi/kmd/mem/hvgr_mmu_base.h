/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MMU_BASE_H
#define HVGR_MMU_BASE_H

#include "hvgr_defs.h"

#define u64_low_32bit(v) ((uint32_t)(v))
#define u64_hi_32bit(v) ((uint32_t)(((uint64_t)(v)) >> 32))

struct hvgr_mmu_cfg {
	uint64_t transcfg;
	uint64_t transtab;
	uint64_t memattr;
};

void hvgr_mmu_sync_pte(struct hvgr_ctx * const ctx, struct page *page, uint32_t offset,
	size_t size);

uint64_t hvgr_mmu_alloc_entry(struct hvgr_ctx * const ctx);

void hvgr_mmu_free_entry(struct hvgr_ctx * const ctx, phys_addr_t entry);

long hvgr_mmu_grow_pool(struct hvgr_ctx * const ctx, uint32_t nums);

#endif
