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

extern struct workqueue_struct *mm_parad_wq;

struct mm_parad_work {
	struct work_struct work;
#define F_PARAD_RECLAIMING 0
	unsigned long flags;

	/* for shrink per memcg */
	struct pglist_data *pgdat;
	struct mem_cgroup *memcg;
	struct scan_control *sc;
	unsigned long nr[NR_LRU_LISTS];
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
};


#ifdef CONFIG_PARA_SWAPD
u64 parad_get_counter(void);
void parad_stat_add(enum parad_state state, u64 counter);
int get_parad_enable(void);
#else
static inline u64 parad_get_counter(void) { return 0; }
static inline void parad_stat_add(enum parad_state state, u64 counter) {}
#endif

#endif
