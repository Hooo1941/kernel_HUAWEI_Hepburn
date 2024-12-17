/*
 * parallel_swapd.h
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef MM_PARAD_H
#define MM_PARAD_H

#include <linux/workqueue.h>

#define PARAD_TASK_MAX 4

struct mm_parad_entry {
	struct list_head list;
#define F_PARAD_RECLAIMING 0
	unsigned long flags;

	/* for shrink per memcg */
	struct pglist_data *pgdat;
	struct mem_cgroup *memcg;
	struct scan_control *sc;
	unsigned long nr[NR_LRU_LISTS];
};

struct mm_parad_task {
	wait_queue_head_t parad_wait;
	atomic_t parad_wait_flag;
	struct task_struct *task;
	unsigned int idx;
};

enum parad_state {
	PARAD_ZSWAPD,
	PARAD_ZSWAPD_WORK,
	PARAD_ZSWAPD_NR_SCANNED,
	PARAD_ZSWAPD_NR_RECLAIMED,
	PARAD_ZSWAPD_WAKEUP,
	PARAD_ZSWAPD_REALWAKE,
	PARAD_ZSWAPD_SUITABLE,
	PARAD_ZSWAPD_INTERVAL,
	PARAD_ZSWAPD_REFAULT,
	PARAD_ZSWAPD_SHRINK_ANON,
	PARAD_KSWAPD,
	PARAD_KSWAPD_NR_SCANNED,
	PARAD_KSWAPD_NR_RECLAIMED,
	PARAD_KSWAPD_WORK,
};

#ifdef CONFIG_PARA_SWAPD
int get_parak_enable(void);
int get_parad_enable(void);
#endif

#if defined(CONFIG_PARA_SWAPD) && defined(CONFIG_DFX_DEBUG_FS)
u64 parad_get_counter(void);
void parad_stat_add(enum parad_state state, u64 counter);
#else
static inline u64 parad_get_counter(void) { return 0; }
static inline void parad_stat_add(enum parad_state state, u64 counter) {}
#endif

#endif
