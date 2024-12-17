/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MMU_PT_DRIVER_H
#define HVGR_MMU_PT_DRIVER_H

#include <linux/types.h>
#include "hvgr_mmu_struct.h"

struct hvgr_ctx;
struct hvgr_mmu_setup_paras;

void hvgr_mmu_free_pgd(struct hvgr_ctx * const ctx);

long hvgr_mmu_invalid_entry(struct hvgr_ctx * const ctx, uint64_t vpfn, uint32_t *pcnt);

long hvgr_mmu_insert_entry(struct hvgr_ctx * const ctx, struct hvgr_mmu_setup_paras *paras);

#endif
