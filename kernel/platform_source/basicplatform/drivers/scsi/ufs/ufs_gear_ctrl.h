/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: gear ctrl header file
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __UFS_GEAR_CTRL_H__
#define __UFS_GEAR_CTRL_H__

/* for pwc change latency check */
#define PWC_CHANGE_OTTIME_MS_THRESHOLD 4
#define PWC_CHANGE_OTVALUE_THRESHOLD 30
#define PWC_CHANGE_OTVALUE_PERADD 10
#define PWC_CHANGE_OTVALUE_PERMIN 1
#define UFSDSS_LOG_SIZE 512

typedef enum {
	UFS_CTRL_GEAR_LEVEL_MIN,
	UFS_CTRL_GEAR_LEVEL_0 = UFS_CTRL_GEAR_LEVEL_MIN,
	UFS_CTRL_GEAR_LEVEL_1,
	UFS_CTRL_GEAR_LEVEL_MAX = UFS_CTRL_GEAR_LEVEL_1
} ufs_ctrl_gear_level_t;

typedef enum {
	UFS_CTRL_GEAR_TYPE_GEAR,
	UFS_CTRL_GEAR_TYPE_LANE,
	UFS_CTRL_GEAR_TYPE_MAX = UFS_CTRL_GEAR_TYPE_LANE
} ufs_ctrl_gear_type_t;

void ufshcd_gear_ctrl_op_register(struct ufs_hba *hba);
void ufshcd_gear_ctrl_set_type(struct ufs_hba *hba, ufs_ctrl_gear_type_t type);
ufs_ctrl_gear_type_t ufshcd_gear_ctrl_get_type(void);
void mas_blk_disable_gear_ctrl(bool enable);
bool mas_blk_check_gear_ctrl_abandon_on_device(void);
void mas_blk_auto_gear_ctrl(bool enable);
void mas_gear_ctrl_flow_config(u32 size_per_100us, u32 cnt_per_ms);
int ufshcd_switch_latency_check(struct ufs_hba *hba, enum ufs_dss_latency_enum scenario, int gear, int reason);

#endif
