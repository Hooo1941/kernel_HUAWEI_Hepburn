 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
  * Description: internal macros of reclaim_acct.
  * Author: Gong Chen <gongchen4@huawei.com>
  * Create: 2021-12-12
  */
#ifndef _RECLAIMACCT_INTERNAL_
#define _RECLAIMACCT_INTERNAL_

/* unit:ms */
#define DEFAULT_DELAY_THRESHOLD 500
#define KWSAPD_BLOCK_DELAY_THRESHOLD 50
#define FS_TYPE_NAME_SIZE 32
#define SCHED_BLOCK_THRESHOLD 5

#define DIRECT_RECLAIM_STR "direct_reclaim"
#define KSWAPD_NODE_STR "kswapd_node"
#define ZSWAPD_NODE_STR "zswapd_node"
#define KSHRINKD_NODE_STR "kshrinkd_node"
#define DRAIN_ALL_PAGES_STR "drain_all_pages"
#define SHRINK_FILE_LIST_STR "shrink_filelist"
#define SHRINK_ANON_LIST_STR "shrink_anonlist"
#define SHRINK_SLAB_STR "shrink_slab"
#define SOFT_LIMIT_STR "soft_limit"

#define LRU_INACTIVE_ANON_STR "lru_inactive_anon"
#define LRU_ACTIVE_ANON_STR "lru_active_anon"
#define LRU_INACTIVE_FILE_STR "lru_inactive_file"
#define LRU_ACTIVE_FILE_STR "lru_active_file"
#define LRU_UNEVICTABLE_STR "lru_unevictable"

#define DELAY_LV0 5000000 /* 5ms */
#define DELAY_LV1 10000000 /* 10ms */
#define DELAY_LV2 50000000 /* 50ms */
#define DELAY_LV3 100000000 /* 100ms */
#define DELAY_LV4 2000000000 /* 2000ms */
#define DELAY_LV5 50000000000 /* 50000ms */
#define NR_DELAY_LV 6

struct reclaim_acct {
	u64 start[NR_RA_STUBS];
	u64 delay[NR_RA_STUBS];
	u64 count[NR_RA_STUBS];
	u64 freed[NR_RA_STUBS];
	u64 scanned[NR_RA_STUBS];
	u64 nr_to_scanned[NR_LRU_LISTS];
	unsigned long (*scan_objects)(struct shrinker *, struct shrink_control *sc);
	char fs_type[FS_TYPE_NAME_SIZE];
	char s_id[FS_TYPE_NAME_SIZE];
	u64 shrinker_delay_max;
#ifdef CONFIG_SCHEDSTATS
	u64 wait_sum;
	u64 sum_sleep_runtime;
	void *p_stack;
	u64 sched_blocked_max_time;
#endif
	unsigned int reclaim_type;
	bool is_blocked;
};

#endif
