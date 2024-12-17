/*
 * haptic_misc.h
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

#ifndef _HAPTIC_MISC_H_
#define _HAPTIC_MISC_H_

struct haptic;
int haptic_register_misc_dev(struct haptic *haptic);
void haptic_unregister_misc_dev(void);

#endif // _HAPTIC_MISC_H_
