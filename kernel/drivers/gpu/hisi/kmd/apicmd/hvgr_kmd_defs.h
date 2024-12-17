/*
 *
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */


#ifndef HVGR_KMD_DEFS_H
#define HVGR_KMD_DEFS_H

#define HVGR_MEM_PAGE_SHIFT  12U
#define HVGR_MEM_PAGE_SIZE   (1UL << HVGR_MEM_PAGE_SHIFT)
#define HVGR_MEM_PAGE_MASK   (~(HVGR_MEM_PAGE_SIZE - 1U))

#define hvgr_mem_page_align(size) \
	(((size) + HVGR_MEM_PAGE_SIZE - 1U) & (~(HVGR_MEM_PAGE_SIZE - 1U)))
#define hvgr_page_nums(size)     (hvgr_mem_page_align(size) >> HVGR_MEM_PAGE_SHIFT)

#endif
