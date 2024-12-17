/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: ufs perf header file
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

#ifndef __UFSHCD_PERF_H__
#define __UFSHCD_PERF_H__
#include "ufshcd.h"

#define WB_FLUSH_WORKER_INTEVAL (5  * 1000)
#define SAMSUNG_FLUSH_LOWER_LIMIT 7
#define SAMSUNG_FLUSH_UPPER_LIMIT 8
#define SAMSUNG_PNM_LEN  15

bool ufshcd_wb_allowed(struct ufs_hba *hba);
void ufshcd_wb_idle_handler(struct ufs_hba *hba, ktime_t now_time);
u8 ufshcd_wb_map_selector(struct ufs_hba *hba, enum query_opcode opcode,
				 u8 idn, u8 index, u8 selector);
ssize_t wb_permanent_disable_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t wb_permanent_disable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t len);
void ufshcd_wb_exception_event_handler(struct ufs_hba *hba);
int ufstt_do_scsi_cmd(struct ufs_hba *hba, struct scsi_cmnd *cmd,
			     unsigned int timeout, ktime_t start_time);
void hufs_wb_config(struct ufs_hba *hba);
#endif
