/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat psi lrb header
 * Author: Fan Weiwei <fanweiwei1@huawei.com>
 * Create: 2023-03-16
 */
#ifndef __HW_STAT_PSI_LRB_H
#define __HW_STAT_PSI_LRB_H

#include "hw_stat_psi.h"
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/types.h>

struct hw_stat_psi_lrb_mngt {
	struct mutex *avgs_lock;
	atomic64_t highload_count; /* highload counts from boot to now */
	atomic64_t total_count;    /* counts from boot to now */

	/* counts when last time update_psi */
	u64 last_highload_count;
	u64 last_total_count;

	u64 last_update_time;
	u64 next_update_time;

	u64 lrb_psi[PSI_TIME_NUM];
};
void update_psi_lrb(void);
void hw_stat_psi_lrb_init(struct proc_dir_entry *parent);

#endif

