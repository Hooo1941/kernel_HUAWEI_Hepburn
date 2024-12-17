/*
 * reclaim_acct.h
 *
 * Memory reclaim delay accounting
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _RECLAIM_ACCT_H
#define _RECLAIM_ACCT_H

#include <linux/sched.h>
#include <linux/shrinker.h>

/* RA is the abbreviation of reclaim accouting */
enum reclaimacct_stubs {
	RA_RECLAIM = 0,
	RA_DRAINALLPAGES,
	RA_SHRINKFILE,
	RA_SHRINKANON,
	RA_SHRINKSLAB,
	RA_SOFTLIMIT,
};
#define NR_RA_STUBS (RA_SOFTLIMIT + 1)

enum ra_reclaim_type {
	DIRECT_RECLAIMS = 0,
	KSWAPD_RECLAIM,
	ZSWAPD_RECLAIM,
	KSHRINKD_RECLAIM,
};
#define RECLAIM_TYPES (KSHRINKD_RECLAIM + 1)

static inline bool is_system_reclaim(enum ra_reclaim_type type)
{
	return (type == KSWAPD_RECLAIM || type == ZSWAPD_RECLAIM ||
		type == KSHRINKD_RECLAIM);
}

void reclaimacct_tsk_init(struct task_struct *tsk);
void reclaimacct_init(void);

void reclaimacct_start(enum ra_reclaim_type type);
void reclaimacct_end(enum ra_reclaim_type type);
void reclaimacct_destroy(void);

void reclaimacct_substage_start(enum reclaimacct_stubs stub, struct shrinker *shrinker);
void reclaimacct_substage_end(enum reclaimacct_stubs stub, unsigned long freed,
	unsigned long scanned, struct shrinker *shrinker);

void reclaimacct_get_nr_to_scan(const unsigned long *nr);
void kswapd_change_block_status(void);

bool is_super_cache_scan(void *scan_objects);

#ifdef CONFIG_SCHEDSTATS
void get_ra_sched_blocked_info(struct task_struct *tsk, u64 time);
#endif

#endif /* _RECLAIM_ACCT_H */
