 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
  * Description: Protect some fs dcaches to speed up Kswapd
  * Author: Ye Wendong <yewendong1@huawei.com>
  * Create: 2023-01-18
  */
#include <linux/kswapd_protect_dcache.h>

#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sysctl.h>
#include <trace/events/vmscan.h>

unsigned int procfs_cache_protect_enable = 0;
static unsigned int procfs_cache_scan_ratio = 1;
static unsigned int procfs_cache_reclaim_threshold = 1;
static int one_hundred = 100;

// when ratio-->1 && threshold-->1, consistent with the original scan and reclaim strategy
static struct ctl_table kswapd_protect_dcache_table[] = {
	{
		.procname	= "procfs_cache_protect_enable",
		.data		= &procfs_cache_protect_enable,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0640,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ZERO,
		.extra2		= SYSCTL_ONE,
	},
	{
		.procname	= "procfs_cache_scan_ratio",
		.data		= &procfs_cache_scan_ratio,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0640,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ONE,
		.extra2		= &one_hundred,
	},
	{
		.procname	= "procfs_cache_reclaim_threshold",
		.data		= &procfs_cache_reclaim_threshold,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0640,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= SYSCTL_ONE,
	},
	{ }
};

static struct ctl_table fs_table[] = {
	{
		.procname	= "fs",
		.mode		= 0555,
		.child		= kswapd_protect_dcache_table,
	},
	{ }
};

long vfs_scan_count(struct super_block *sb, struct shrinker *shrink, long total_objects)
{
	// when total_objects returns 0, it won't reclaim
	if (procfs_cache_protect_enable && sb->s_magic == PROC_SUPER_MAGIC) {
		trace_procfs_super_cache_count(total_objects);
		if (total_objects < procfs_cache_reclaim_threshold)
			total_objects = 0;
		else if (total_objects > shrink->batch)
			total_objects /= procfs_cache_scan_ratio;
	} else {
		total_objects = vfs_pressure_ratio(total_objects);
	}
	return total_objects;
}

static int __init kswapd_protect_dcache_init(void)
{
	register_sysctl_table(fs_table);
	return 0;
}
module_init(kswapd_protect_dcache_init);
