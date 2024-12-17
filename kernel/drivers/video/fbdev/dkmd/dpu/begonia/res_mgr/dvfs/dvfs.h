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

#ifndef DKMD_DPU_DVFS_H
#define DKMD_DPU_DVFS_H

#include <linux/types.h>
#include <linux/semaphore.h>
#include "dkmd_dpu.h"
#include "dkmd_comp.h"

struct dpu_dvfs_user_node_info {
	uint32_t voted_level;
	struct vote_freq_info vote_freq_info;
};

enum DVFS_TYPE {
	DVFS_INTRA_TYPE, // hw dvfs use hw channel
	DVFS_INTER_HW_TYPE, // hw dvfs only use sw channel
	DVFS_INTER_SW_TYPE, // sw dvfs use regulator function
};

struct dpu_dvfs {
	struct semaphore sem;
	uint32_t voted_level; // record last perf level
	uint32_t inter_dvfs_locked_count; // whether fix inter frame dvfs
	struct dpu_dvfs_user_node_info user_info[DEVICE_COMP_MAX_COUNT];
};

void dpu_res_register_dvfs(struct list_head *res_head);
void dpu_dvfs_inter_frame_vote(uint32_t comp_index, struct intra_frame_dvfs_info *info);
void dpu_dvfs_intra_frame_vote(uint32_t comp_index, struct intra_frame_dvfs_info *info,
	bool need_direct_vote);
void dpu_dvfs_direct_vote(uint32_t comp_index, uint32_t perf_level, bool need_config_qos);
void dpu_dvfs_enable_core_clock(bool need_config_qos);
void dpu_dvfs_disable_core_clock(void);
void dpu_dvfs_qos_qic_media1_config(uint32_t level);
void dpu_dvfs_decrease_count_locked(void);
void dpu_dvfs_increase_count_locked(void);
void dpu_dvfs_reset_comp_vote(uint32_t comp_index);
uint32_t dpu_dvfs_check_low_temperature(void);
void dpu_dvfs_reset_vote(uint32_t comp_index);
void dpu_dvfs_set_inter_vote_index_count(uint32_t count);
#endif
