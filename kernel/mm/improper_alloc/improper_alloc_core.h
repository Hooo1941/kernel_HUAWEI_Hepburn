/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Description: Debug improper memory alloc behaviours
 * Author: Li Xilun <lixilun1@huawei.com>
 * Create: 2023-03-23
 */

#ifndef _IMPROPER_ALLOC_CORE_H
#define _IMPROPER_ALLOC_CORE_H

/* Common used in improper alloc */

#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/stddef.h>

extern unsigned int __read_mostly g_improper_alloc_enable;

#define MAX_NAME_LEN 16

struct improper_alloc {
	const char name[MAX_NAME_LEN];
	bool (*const improper)(gfp_t gfp_mask, unsigned int order, size_t size);
	void (*const extra_info)(gfp_t gfp_mask, unsigned int order, size_t size);
	atomic_long_t sum;
	struct ratelimit_state rl;
};

extern struct improper_alloc g_improper_types[];

enum improper_type {
	IMPROPER_HIGH_ORDER = 0,
	IMPROPER_ATOMIC,
	IMPROPER_LARGE_SLAB,
	IMPROPER_COUNT
};

#define for_each_improper_type(p) \
	for ((p) = g_improper_types; (p) <= &g_improper_types[IMPROPER_COUNT - 1]; ++(p))

#endif
