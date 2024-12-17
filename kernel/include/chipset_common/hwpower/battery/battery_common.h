/* SPDX-License-Identifier: GPL-2.0 */
/*
 * battery_common.h
 *
 * common interface, variables, definition etc of battery
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

#ifndef _BATTERY_COMMON_H_
#define _BATTERY_COMMON_H_

enum battery_connect_mode {
	BAT_PARALLEL_MODE = 0,
	BAT_SERIAL_MODE,
};

#endif /* _BATTERY_COMMON_H_ */