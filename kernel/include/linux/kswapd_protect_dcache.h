 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
  * Description: Protect some fs dcaches to speed up Kswapd
  * Author: Ye Wendong <yewendong1@huawei.com>
  * Create: 2023-01-18
  */
#ifndef _KSWAPD_PROTECT_DCACHE_
#define _KSWAPD_PROTECT_DCACHE_

#include <linux/mm_types.h>
#include <linux/dcache.h>
#include <linux/shrinker.h>

extern unsigned int procfs_cache_protect_enable __read_mostly;

// The function's caller needs to guarantee the dentry lock by itself
// if dentry->flags with DCACHE_KSWAPD_NOFREE, kswapd will not reclaim it but only once
static inline bool need_kswapd_protect_dcache(struct dentry *dentry)
{
	return dentry->d_flags & DCACHE_KSWAPD_NOFREE;
}

// The function's caller needs to guarantee the dentry lock by itself
static inline void set_kswapd_protect_dcache(struct dentry *dentry)
{
	dentry->d_flags |= DCACHE_KSWAPD_NOFREE;
}

// The function's caller needs to guarantee the dentry lock by itself
static inline void unset_kswapd_protect_dcache(struct dentry *dentry)
{
	dentry->d_flags &= ~DCACHE_KSWAPD_NOFREE;
}

long vfs_scan_count(struct super_block *sb, struct shrinker *shrink, long total_objects);
#endif
