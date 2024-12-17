/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: fault event handler function for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _COUL_ALGO_FAULT_EVT_OPS_H_
#define _COUL_ALGO_FAULT_EVT_OPS_H_

#include <platform_include/basicplatform/linux/power/platform/coul/coul_core.h>
void coul_core_lock_init(struct smartstar_coul_device *di);
void coul_core_lock_trash(struct smartstar_coul_device *di);
void coul_core_fault_work(struct work_struct *work);

#endif
