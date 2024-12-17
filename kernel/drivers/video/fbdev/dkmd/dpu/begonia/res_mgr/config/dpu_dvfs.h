/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#ifndef DPU_DVFS_H
#define DPU_DVFS_H

enum DVFS_VOTE_MODE {
	DVFS_HW_CHN_MODE,
	DVFS_SW_CHN_MODE,
};

#define DVFS_IDLE_LEVEL 1

void dpu_enable_core_clock(bool is_power_on);
void dpu_disable_core_clock(void);
bool is_dpu_dvfs_enable(void);
void dpu_qos_qic_media1_config(uint32_t level);
void dpu_dvfs_inter_process(uint32_t vote_level, bool is_inter_invoke);
void dpu_dvfs_intra_process(struct intra_frame_dvfs_info *dvfs_info, bool inter_locked);
void dpu_legacy_inter_frame_dvfs_vote(uint32_t vote_level, bool need_config_qos);
void dpu_dvfs_direct_process(uint32_t vote_level, bool need_config_qos);
uint32_t dpu_config_get_perf_level(uint64_t freq);

#endif /* DPU_DVFS_H */