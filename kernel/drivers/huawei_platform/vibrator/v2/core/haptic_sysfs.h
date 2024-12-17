/*
 * haptic_sysfs.h
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

#ifndef _HAPTIC_SYSFS_H_
#define _HAPTIC_SYSFS_H_

int haptic_create_sysfs_nodes(struct haptic *haptic);
void haptic_remove_sysfs_nodes(struct haptic *haptic);

#endif // _HAPTIC_SYSFS_H_
