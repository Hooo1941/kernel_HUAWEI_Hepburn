/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_mmu_entry.h"

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include "hvgr_defs.h"
#include "hvgr_regmap.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_dm_api.h"

#define ENTRY_TYPE_MASK         3ULL
#define ENTRY_IS_PTE            1ULL
#define ENTRY_IS_INVAL          2ULL
#define ENTRY_IS_PMD            3ULL

#define ENTRY_ATTR_ACE           ((uint64_t)3 << 2)
#define ENTRY_RD_BIT             ((uint64_t)1 << 6)
#define ENTRY_WR_BIT             ((uint64_t)1 << 7)
#define ENTRY_SHARE_BOTH_BITS    ((uint64_t)2 << 8)
#define ENTRY_SHARE_INNER_BITS   ((uint64_t)3 << 8)
#define ENTRY_PROT_BIT           ((uint64_t)1 << 42)
#define ENTRY_NX_BIT             ((uint64_t)1 << 54)
#define ENTRY_SPARSE_BITS        ((uint64_t)1 << 63)
#define ENTRY_GID_BIT_START      38U
#define ENTRY_GID_MASK           ((uint64_t)0xF << 38)    /* bits 38:41 */
#define ENTRY_PBHA_BIT_START     59U
#define ENTRY_PBHA_MASK          ((uint64_t)0xF << 59)    /* bits 59:62 */


#define PMD_RESERVED_MASK        (((uint64_t)0xFFFFFF << 40) | \
	(uint64_t)0xFFF) /* bits 63:40 bits 11:0 */
#define PTE_RESERVED_MASK        (((uint64_t)0xF << 55) | \
	((uint64_t)0x7FF << 43)) /* bits 58:55 bits 53:43 */

#ifdef HVGR_FEATURE_SYSTEM_CACHE
void hvgr_mmu_clear_sc_flag(struct hvgr_device *gdev, uint64_t *flag)
{
	if (gdev->mem_dev.sc_data.pbha_mode != 0)
		*flag &= (~ENTRY_PBHA_MASK);
	else
		*flag &= (~ENTRY_GID_MASK);
}

void hvgr_mmu_set_sc_flag(struct hvgr_device *gdev, uint64_t *flag, uint32_t sc_id)
{
	uint64_t sc_idx = sc_id & 0xF;

	if (gdev->mem_dev.sc_data.pbha_mode != 0)
		*flag |= (sc_idx << ENTRY_PBHA_BIT_START);
	else
		*flag |= (sc_idx << ENTRY_GID_BIT_START);
}
#endif

uint64_t hvgr_mmu_get_entry_flags(struct hvgr_device *gdev, uint64_t attr)
{
	if (!hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_00032))
		return 0;

	return (attr & HVGR_MEM_ATTR_SECURE) ? ENTRY_PROT_BIT : 0;
}

uint64_t hvgr_mmu_get_flags(struct hvgr_device *gdev, uint64_t attr)
{
	uint64_t flag = (attr & HVGR_MEM_ATTR_GPU_CACHEABLE) ? ENTRY_ATTR_ACE : 0;

	flag |= (attr & HVGR_MEM_ATTR_GPU_RD) ? ENTRY_RD_BIT : 0;
	flag |= (attr & HVGR_MEM_ATTR_GPU_WR) ? ENTRY_WR_BIT : 0;
	flag |= (attr & HVGR_MEM_ATTR_GPU_EXEC) ? 0 : ENTRY_NX_BIT;
	flag |= (attr & HVGR_MEM_ATTR_MEM_SPARSE) ? ENTRY_SPARSE_BITS : 0;
	flag |= hvgr_mmu_get_entry_flags(gdev, attr);
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	if (gdev->mem_dev.sc_data.pbha_mode != 0)
		flag |= hvgr_mem_get_sc_id(attr) << ENTRY_PBHA_BIT_START;
	else
		flag |= hvgr_mem_get_sc_id(attr) << ENTRY_GID_BIT_START;
#endif
	flag |= (attr & HVGR_MEM_ATTR_COHERENT_SYSTEM) ? ENTRY_SHARE_BOTH_BITS : 0;
	flag |= ((attr & HVGR_MEM_ATTR_COHERENT_SYSTEM) == 0 &&
		(attr & HVGR_MEM_ATTR_COHERENT_LOCAL) != 0) ? ENTRY_SHARE_INNER_BITS : 0;

	return flag;
}

void hvgr_mmu_entry_set_pte(union hvgr_mmu_entry *entry, struct page *page,
	uint64_t mmu_flag, uint64_t flags)
{
	uint64_t value = (uint64_t)page_to_phys(page) & PAGE_MASK;
	/*
	 * If ENTRY_SPARSE_BITS is setted before means current page
	 * is sparse zero page. HVGR_MEM_AREA_FLAG_SPARSE_CACHE is set means
	 * current operation is to change the attribute of area, mapping
	 * of zero page will not be modified.
	 */
	if ((entry->value & ENTRY_SPARSE_BITS) != 0 &&
		(flags & HVGR_MEM_AREA_FLAG_SPARSE_CACHE) != 0)
		return;

	value |= mmu_flag;
	value |= ENTRY_IS_PTE;
	value &= ~(PTE_RESERVED_MASK);

	entry->value = value;
}

void hvgr_mmu_entry_set_pmd(union hvgr_mmu_entry *entry, phys_addr_t phy_addr)
{
	uint64_t value = (phy_addr & ~(PMD_RESERVED_MASK)) | ENTRY_IS_PMD;

	entry->value = value;
}

void hvgr_mmu_entry_invalid(union hvgr_mmu_entry *entry)
{
	entry->value = ENTRY_IS_INVAL;
}

bool hvgr_mmu_entry_is_pte(union hvgr_mmu_entry *entry)
{
	return (entry->value & ENTRY_TYPE_MASK) == ENTRY_IS_PTE;
}

bool hvgr_mmu_entry_is_pmd(union hvgr_mmu_entry *entry)
{
	return (entry->value & ENTRY_TYPE_MASK) == ENTRY_IS_PMD;
}

bool hvgr_mmu_entry_is_table(union hvgr_mmu_entry *entry, uint32_t level)
{
	return (entry->value & ENTRY_TYPE_MASK) == ENTRY_IS_PMD;
}