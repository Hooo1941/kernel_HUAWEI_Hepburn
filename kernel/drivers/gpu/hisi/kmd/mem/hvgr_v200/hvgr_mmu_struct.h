/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MMU_STRUCT_H
#define HVGR_MMU_STRUCT_H

#include <linux/types.h>

/*
 * 48bit VA to MMU.
 *
 * |47  ~  39|38  ~  30|29  ~  21|20  ~  12|11  ~  0|
 * | level0  | level1  | level2  | level3  | offset |
 * | PGD     | PMD     | PMD     | PTE     | offset |
 */
#define MMU_LEVEL_0 0u
#define MMU_LEVEL_1 1u
#define MMU_LEVEL_2 2u
#define MMU_LEVEL_3 3u
#define MMU_LEVEL_NUM 4u

#define MMU_ENTRY_NUM 512u
#define MMU_PTE_MASK 0x1FFULL
#define MMU_ENTRY_SIZE 8u

#define va_to_vpfn(va) ((va) >> 12)
#define mmu_level_offset(level) ((MMU_LEVEL_3 - (level)) * 9)
#define mmu_get_level_idx(vpfn, level) (((vpfn) >> mmu_level_offset(level)) & MMU_PTE_MASK)

#define entry_to_phys(p_entry) (((uint64_t)((p_entry)->entry.addr)) << PAGE_SHIFT)
#define entry_to_page(p_entry) (phys_to_page(entry_to_phys(p_entry)))

union hvgr_mmu_entry {
	struct {
		uint64_t flag : 2;
		uint64_t attr_idx : 3;
		uint64_t ignore1 : 1;
		uint64_t access_permisions : 2;
		uint64_t share_ablility : 2;
		uint64_t ignored2 : 2;
		uint64_t addr : 24;
		uint64_t sc_gid : 3; /* bit 36:38 */
	} entry;
	uint64_t value;
};
#endif
