/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: macro of prefetch memory instructions.
 */
#ifndef __PREFECTCH_H
#define __PREFECTCH_H

#include <linux/stddef.h>
#include <linux/cache.h>
#include <linux/kernel.h>

/* offset must be a multiple of 8 in range [0, 32760] */
#define PRFM_OFFSET_MASK 0x7FF8

#define prfm_data(prfop, ptr, offset) \
	__asm__ volatile("prfm " prfop ", [%0, %1]\n\t"::"r"(ptr), "i"(offset))

#define prfm_member(prfop, ptr, class, member) \
	prfm_data(prfop, ptr, offsetof(class, member) & PRFM_OFFSET_MASK)

#define prfm_cacheline_relative(prfop, ptr, offset, cacheline) \
	prfm_data(prfop, ptr, ((offset) + (cacheline) * SMP_CACHE_BYTES))

#define prfm_cacheline(prfop, ptr, cacheline) \
	prfm_cacheline_relative(prfop, ptr, 0, cacheline)

#define prfm_struct_relative(prfop, ptr, offset, size) \
	do { \
		for (int i = 0; i < DIV_ROUND_UP(size, SMP_CACHE_BYTES); ++i) { \
			prfm_cacheline_relative(prfop, ptr, offset, i); \
		} \
	} while (0)

#define prfm_struct(prfop, ptr, size) \
	prfm_struct_relative(prfop, ptr, 0, size)

#endif
