/*
 * npu_pm_top_specify.h
 *
 * about npu hwts plat
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __NPU_PM_TOP_SPECIFY_H
#define __NPU_PM_TOP_SPECIFY_H
#include "npu_common.h"
int npu_top_specify_powerup(u32 work_mode, u32 subip_set);

int npu_top_sepcify_powerdown(u32 work_mode, u32 subip_set);

#endif