// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Description: Debug improper memory alloc behaviours
 * Author: Li Xilun <lixilun1@huawei.com>
 * Create: 2023-03-23
 */

#include <platform/linux/improper_alloc.h>

#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/sysctl.h>

#include <log/log_usertype.h>

#include <improper_alloc_core.h>

#ifdef CONFIG_IMPROPER_ALLOC_FAULT
#include <improper_alloc_fault.h>
#endif

/* Set max interval to 24 hours */
static int max_interval = 24 * 60 * 60;

#define IMPROPER_TYPE_TABLE(x) { \
	.procname	= g_improper_types[(x)].name, \
	.data		= &g_improper_types[(x)].rl.interval, \
	.maxlen		= sizeof(g_improper_types[(x)].rl.interval), \
	.mode		= 0660, \
	.proc_handler	= proc_dointvec_jiffies, \
	.extra1		= SYSCTL_ZERO, \
	.extra2		= &max_interval, \
}

static struct ctl_table interval_table[] = {
	IMPROPER_TYPE_TABLE(IMPROPER_HIGH_ORDER),
	IMPROPER_TYPE_TABLE(IMPROPER_ATOMIC),
	IMPROPER_TYPE_TABLE(IMPROPER_LARGE_SLAB),
	{ }
};

struct ctl_table improper_alloc_table[] = {
	{
		.procname	= "enable",
		.data		= &g_improper_alloc_enable,
		.maxlen		= sizeof(g_improper_alloc_enable),
		.mode		= 0660,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
	{
		.procname	= "interval",
		.mode		= 0440,
		.child		= interval_table,
	},
#ifdef CONFIG_IMPROPER_ALLOC_FAULT
	{
		.procname	= "fault",
		.data		= &g_fault_thread_running,
		.maxlen		= sizeof(g_fault_thread_running),
		.mode		= 0660,
		.proc_handler	= improper_fault_handler,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
#endif
	{ }
};

static struct ctl_table vm_table[] = {
	{
		.procname	= "improper_alloc",
		.mode		= 0440,
		.child		= improper_alloc_table,
	},
	{ }
};

static struct ctl_table sysctl_base_table[] = {
	{
		.procname	= "vm",
		.mode		= 0555,
		.child		= vm_table,
	},
	{ }
};

static int __init improper_alloc_sysctl_init(void)
{
	register_sysctl_table(sysctl_base_table);
	return 0;
}

module_init(improper_alloc_sysctl_init);
