/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */


#ifndef HVGR_ASSERT_H
#define HVGR_ASSERT_H
#include <linux/kernel.h>

#ifdef CONFIG_HVGR_DEBUG
#define HVGR_DEBUG_ASSERT_ENABLE 1
#else
#define HVGR_DEBUG_ASSERT_ENABLE 0
#endif

#if HVGR_DEBUG_ASSERT_ENABLE
#define hvgr_assert(expr) \
do { \
	if (unlikely(!(expr))) { \
		pr_err("HVGR<ASSERT>: In file: %s line: %d Assertion failed!\n", \
			__FILE__, __LINE__); \
		panic(#expr); \
	} \
} while (0)
#else
#define hvgr_assert(expr) do { } while (0)
#endif

#endif /* HVGR_ASSERT_H */
