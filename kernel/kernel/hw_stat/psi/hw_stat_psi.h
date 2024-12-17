/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat psi header
 * Author: Fan Weiwei <fanweiwei1@huawei.com>
 * Create: 2023-03-16
 */
#ifndef __HW_STAT_PSI_H
#define __HW_STAT_PSI_H

#include <linux/proc_fs.h>

#define HW_STAT_PSI_PERIOD  (2 * HZ) /* 2 sec intervals */
#define HW_STAT_EXP_10S 	1677 /* 1/exp(2s/10s) as fixed-point */
#define HW_STAT_EXP_60S 	1981 /* 1/exp(2s/60s) */
#define HW_STAT_EXP_300S 	2034 /* 1/exp(2s/300s) */

enum psi_time_type {
	RECENT_10S,
	RECENT_60S,
	RECENT_300S,
	PSI_TIME_NUM
};

void hw_stat_psi_init(struct proc_dir_entry *parent);

#endif
