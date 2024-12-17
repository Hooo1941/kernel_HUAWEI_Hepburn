/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MMU_ENTRY_H
#define HVGR_MMU_ENTRY_H

#include <linux/types.h>
#include <linux/mm.h>
#include "hvgr_mmu_struct.h"

void hvgr_mmu_entry_set_pte(union hvgr_mmu_entry *entry, struct page *page,
	uint64_t mmu_flag, uint64_t flags);

void hvgr_mmu_entry_set_pmd(union hvgr_mmu_entry *entry, phys_addr_t phy_addr);

void hvgr_mmu_entry_invalid(union hvgr_mmu_entry *entry);

bool hvgr_mmu_entry_is_pte(union hvgr_mmu_entry *entry);

bool hvgr_mmu_entry_is_pmd(union hvgr_mmu_entry *entry);

bool hvgr_mmu_entry_is_table(union hvgr_mmu_entry *entry, uint32_t level);

#endif
