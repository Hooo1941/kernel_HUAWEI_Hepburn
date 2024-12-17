/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#ifndef __DDR_DVFS_H__
#define __DDR_DVFS_H__
#include "dkmd_sysfs.h"

void dpu_ddr_dvfs_init(struct dkmd_attr *attrs);
void dpu_ddr_dvfs_deinit(void);
void dpu_ddr_bandwidth_release(bool need_vote);
void dpu_ddr_bandwidth_recovery(bool need_vote);
#endif