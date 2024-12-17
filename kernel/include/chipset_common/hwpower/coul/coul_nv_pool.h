/* SPDX-License-Identifier: GPL-2.0 */
/*
 * coul_nv_pool.h
 *
 * nv pool for coul
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef _COUL_NV_POOL_H_
#define _COUL_NV_POOL_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

enum {
	COUL_NV_POOL_BEGIN = 0,
	COUL_NV_POOL_BK_BATTERY_CURR_CALI = COUL_NV_POOL_BEGIN,
	COUL_NV_POOL_OCV_INDEX,
	COUL_NV_POOL_TEMP_CYCLE,
	COUL_NV_POOL_TOTAL_CYCLE,
	COUL_NV_POOL_END,
};

enum {
	COUL_NV_POOL_IC_TYPE_MAIN = 0,
	COUL_NV_POOL_IC_TYPE_AUX,
	COUL_NV_POOL_IC_TYPE_MAX,
};

struct coul_nv_pool_dev {
	struct device *dev;
	struct delayed_work once_work;
};

int coul_nv_pool_get_para(int coul_type, int mode, int *value);
int coul_nv_pool_set_para(int coul_type, int mode, int value);

#endif /* _COUL_NV_POOL_H_ */
