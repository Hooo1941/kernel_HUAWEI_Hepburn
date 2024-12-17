// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Description: Debug improper memory alloc behaviours
 * Author: Li Xilun <lixilun1@huawei.com>
 * Create: 2023-03-23
 */

#define pr_fmt(fmt) "improper_alloc: " fmt

#include <platform/linux/improper_alloc.h>

#include <linux/kobject.h>
#include <linux/seq_file.h>
#include <linux/stddef.h>
#include <linux/sysfs.h>

#include <log/log_usertype.h>

#include <improper_alloc_core.h>

/*
 * Hook for allocation. There are two cases:
 * 1. Hook in __alloc_pages_nodemask, size = 0
 * 2. Hook in kmalloc_order, size != 0
 */
void improper_alloc_hook(gfp_t gfp_mask, unsigned int order, size_t size)
{
	struct improper_alloc *type = NULL;

#if !defined(CONFIG_LOG_EXCEPTION) && defined(CONFIG_IMPROPER_ALLOC_SINGLE_FRAME)
	unsigned int log_usertype = BETA_USER;
#else
	unsigned int log_usertype = get_logusertype_flag();
#endif

	if (!g_improper_alloc_enable)
		return;

	if (log_usertype != BETA_USER && log_usertype != OVERSEA_USER)
		return;

	for_each_improper_type(type) {
		if (type->improper(gfp_mask, order, size))
			atomic_long_inc(&type->sum);
		else
			continue;

		if (!__ratelimit(&type->rl))
			continue;

		pr_info("gfp_mask=%#x(%pGg) order=%u type=%s sum=%ld\n", gfp_mask, &gfp_mask,
			order, type->name, atomic_long_read(&type->sum));
		type->extra_info(gfp_mask, order, size);
		dump_stack();
	}
}

void improper_alloc_show(struct seq_file *m)
{
	struct improper_alloc *type = NULL;

	if (!g_improper_alloc_enable)
		return;

	for_each_improper_type(type)
		seq_printf(m, "%s_sum:%ld\n", type->name, atomic_long_read(&type->sum));
}
