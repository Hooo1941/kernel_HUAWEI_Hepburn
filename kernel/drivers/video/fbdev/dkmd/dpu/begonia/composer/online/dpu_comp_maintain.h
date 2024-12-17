/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef DPU_COMP_MAINTAIN_H
#define DPU_COMP_MAINTAIN_H

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include "chrdev/dkmd_sysfs.h"
#include "isr/dkmd_isr.h"
#include "dkmd_lcd_interface.h"

#define PERIOD_US_1HZ (1 * 1000 * 1000)
#define PERIOD_US_10HZ (1 * 100 * 1000)
#define MAX_RECODE_DURATION_US (24 * 60 * 60 * 1000 * 1000UL) // 1 day
#define PRIMARY_SELF_REFRESH_PERIOD_US (1 * 1000 * 1000)
#define BUILTIN_SELF_REFRESH_PERIOD_US (1 * 1000 * 1000)

struct dpu_comp_maintain {
	wait_queue_head_t wait;
	struct mutex maintain_lock;

	ktime_t pre_refresh_timestamp;
	ktime_t curr_refresh_timestamp;

	ktime_t pre_refresh_timestamp_hw;
	ktime_t curr_refresh_timestamp_hw;

	bool routine_enabled;
	uint64_t total_count_time_us;
	uint32_t real_frame_rate;
	uint32_t first_count_flag;
	uint32_t self_refresh_period_us;
	uint32_t cur_te_rate;

	struct panel_refresh_statistic_info refresh_stat_info;
	struct kthread_work isr_handle_work;
	struct dpu_composer *dpu_comp;
};

void comp_mntn_refresh_stat_init(struct dkmd_isr *isr,
	struct dpu_comp_maintain *comp_maintain, uint32_t listening_bit);
void comp_mntn_refresh_stat_deinit(struct dkmd_isr *isr, uint32_t listening_bit);

static inline void comp_mntn_enable_routine(struct dpu_comp_maintain *comp_maintain)
{
	comp_maintain->routine_enabled = true;
}

static inline void comp_mntn_disable_routine(struct dpu_comp_maintain *comp_maintain)
{
	comp_maintain->routine_enabled = false;
}

#endif
