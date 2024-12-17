/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: interface for coul module
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

#ifndef _COUL_ALGO_INTERFACE_H_
#define _COUL_ALGO_INTERFACE_H_

#include <platform_include/basicplatform/linux/power/platform/coul/coul_core.h>

struct coul_merge_drv_ops *get_coul_interface_ops(void);
void calculate_soc_work(void *data);
int coul_core_suspend(void *data);
int coul_core_resume(void *data);
void coul_core_shutdown(void *data);

#endif
