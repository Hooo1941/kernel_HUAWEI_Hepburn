/* SPDX-License-Identifier: GPL-2.0 */
/*
 * battery_ocv.h
 *
 * head file for battery ocv driver
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#ifndef _BATTERY_OCV_H_
#define _BATTERY_OCV_H_

#define BATTERY_OCV_SHUTDOWN_CAPACITY        (-1)
#define BATTERY_OCV_CUTOFF_CAPACITY          0
#define BATTERY_OCV_ONE_PERCENT_CAPACITY     1

#ifdef CONFIG_HUAWEI_BATTERY_OCV
int battery_ocv_get_ocv_by_cap(int cap, int *ocv);
#else
static inline int battery_ocv_get_ocv_by_cap(int cap, int *ocv)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_BATTERY_OCV */

#endif /* _BATTERY_OCV_H_ */
