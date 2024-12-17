/*
 * kswapd_acct.h
 *
 * Kswapd reclaim delay accounting
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd
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

#ifndef _KSWAPD_ACCT_H
#define _KSWAPD_ACCT_H

#include <linux/sched.h>

#define KSWAPDNODE_STR "kswapd_node"
#define SHRINKFILE_STR "shrink_file"
#define SHRINKANON_STR "shrink_anon"
#define SHRINKSLAB_STR "shrink_slab"

#define KSWAPD_DELAY_STR "delay"
#define KSWAPD_COUNT_STR "count"
#define KSWAPD_RECLAIMED_STR "reclaimed"
#define KSWAPD_SCANNED_STR "scanned"

#define LRU_INACTIVE_ANON_STR "lru_inactive_anon"
#define LRU_ACTIVE_ANON_STR "lru_active_anon"
#define LRU_INACTIVE_FILE_STR "lru_inactive_file"
#define LRU_ACTIVE_FILE_STR "lru_active_file"
#define LRU_UNEVICTABLE_STR "lru_unevictable"

/* unit:ms */
#define DEFAULT_DELAY_THRESHOLD 300
#define BLOCK_DELAY_THRESHOLD 50

void kswapdacct_start(void);
void kswapdacct_end(void);
void kswapd_stage_start(int type);
void kswapd_stage_end(int type);
void kswapd_stage_count(int type, u64 reclaimed_delta, u64 scanned_delta);
void kswapdacct_shrinker_start(void);
void kswapdacct_shrinker_end(uintptr_t shrinker_ptr);
void kswapdacct_collect_data(void);
void kswapdacct_clear_data(void);
void kswapdacct_get_nr_to_scan(const unsigned long *nr);
void kswapdacct_change_block_status(void);
u64 kswapdacct_get_data(int type, int level, int stub);

enum kswapacct_stubs {
	KSWAPD_SHRINKNODE = 0,
	KSWAPD_SHRINKFILE,
	KSWAPD_SHRINKANON,
	KSWAPD_SHRINKSLAB,
	KSWAPD_NUMS,
};

enum delay_levels {
	DELAY_LEVEL0 = 0,
	DELAY_LEVEL1 = 5000000,
	DELAY_LEVEL2 = 10000000, /* 10ms */
	DELAY_LEVEL3 = 50000000, /* 50ms */
	DELAY_LEVEL4 = 100000000, /* 100ms */
	DELAY_LEVEL5 = 500000000, /* 500ms */
	DELAY_LEVEL_NUMS = 6,
};

enum show_stubs {
	KSWAPD_DELAY = 0,
	KSWAPD_COUNT,
	KSWAPD_RECLAIMED,
	KSWAPD_SCANNED,
	KSWAPD_SHRINKER,
	KSWAPD_SHOW_NUMS,
};

#endif /* _KSWAPD_ACCT_H */