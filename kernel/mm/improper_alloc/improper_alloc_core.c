// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Description: Debug improper memory alloc behaviours
 * Author: Li Xilun <lixilun1@huawei.com>
 * Create: 2023-03-23
 */

#include <improper_alloc_core.h>

#include <linux/gfp.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/stddef.h>

#include <log/log_usertype.h>

unsigned int __read_mostly g_improper_alloc_enable = IMPROPER_ALLOC_ENABLE_DEFAULT;

static inline bool is_slab_large(size_t size)
{
	return !!size;
}

static inline bool high_order_improper(gfp_t gfp_mask, unsigned int order, size_t size)
{
	return !is_slab_large(size) && order > 0;
}

static inline bool atomic_improper(gfp_t gfp_mask, unsigned int order, size_t size)
{
	return !in_interrupt() && preempt_count() == 0 && (gfp_mask & __GFP_ATOMIC);
}

static inline bool large_slab_improper(gfp_t gfp_mask, unsigned int order, size_t size)
{
	return is_slab_large(size) && ((1 << (order + PAGE_SHIFT)) > size);
}

static inline void high_order_extra_info(gfp_t gfp_mask, unsigned int order, size_t size)
{
	return;
}

static inline void atomic_extra_info(gfp_t gfp_mask, unsigned int order, size_t size)
{
	pr_info("%s: irq=%s preempt=%d\n", __func__, in_interrupt() ? "i" : "o", preempt_count());
}

static inline void large_slab_extra_info(gfp_t gfp_mask, unsigned int order, size_t size)
{
	pr_info("%s: size=%zu waste=%lld\n", __func__, size,
		(long long)(1 << (order + PAGE_SHIFT)) - (long long)size);
}

#define DEF_INTERVAL (10 * 60)
#define DEF_BURST 1

#define DEFINE_IMPROPER_ALLOC(x) { \
	.name = __stringify(x), \
	.sum = ATOMIC_LONG_INIT(0), \
	.rl = RATELIMIT_STATE_INIT((x).ratelimit, DEF_INTERVAL * HZ, DEF_BURST), \
	.improper = x##_improper, \
	.extra_info = x##_extra_info, \
}

struct improper_alloc g_improper_types[IMPROPER_COUNT] = {
	DEFINE_IMPROPER_ALLOC(high_order),
	DEFINE_IMPROPER_ALLOC(atomic),
	DEFINE_IMPROPER_ALLOC(large_slab),
};
