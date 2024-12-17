/*
 * vibrator_common.h
 *
 * vibrator_common driver
 *
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
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

#ifndef _VIBRATOR_COMMON_H
#define _VIBRATOR_COMMON_H
#include <linux/notifier.h>

bool vibrator_get_enter_recovery_mode(void);

#endif