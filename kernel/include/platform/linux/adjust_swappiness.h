/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: adjust swappiness for dr and kswapd.
 * Author: misasa
 * Create: 2023-09-28
 */
#ifndef _ADJUST_SWAPPINESS_H
#define _ADJUST_SWAPPINESS_H

enum track_policy {
	TRACK_BIG_CPU = 1,
	TRACK_FAST_CPU,
	TRACK_ALL_CPU,
};

int adjust_swappiness(struct scan_control *sc, int swappiness);

#ifdef CONFIG_HYPERHOLD
int get_direct_vm_swappiness_hp(void);
#endif

int get_direct_vm_swappiness(void);
int get_third_order_swappiness_ratio(void);
int get_kswapd_track_load_policy(void);
int get_kswapd_high_load_swappiness(void);
int get_kswapd_high_load_threshold_value(void);

#ifdef CONFIG_KSWAPD_DEBUG
void inc_kswapd_called_cnt(void);
void inc_kswapd_in_dr_cnt(void);
void inc_kswapd_high_load_cnt(void);
#else
static inline void inc_kswapd_called_cnt(void) {}
static inline void inc_kswapd_in_dr_cnt(void) {}
static inline void inc_kswapd_high_load_cnt(void) {}
#endif

#endif
