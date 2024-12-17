/*
 * vib_log.h
 *
 * code for vibrator
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#ifndef _VIB_LOG_H_
#define _VIB_LOG_H_

#include <linux/printk.h>

#define HWVIB_LOG_TAG "hwvibrator"

#define vib_err(format, ...) \
	pr_err("[%s]%s: " format "\n", HWVIB_LOG_TAG, ##__VA_ARGS__)

#define vib_warn(format, ...) \
	pr_warn("[%s]%s: " format "\n", HWVIB_LOG_TAG, ##__VA_ARGS__)

#define vib_info(format, ...) \
	pr_info("[%s]%s: " format "\n", HWVIB_LOG_TAG, ##__VA_ARGS__)

#define vib_dbg(format, ...) \
	pr_debug("[%s]%s: " format "\n", HWVIB_LOG_TAG, ##__VA_ARGS__)

#endif //_VIB_LOG_H_
