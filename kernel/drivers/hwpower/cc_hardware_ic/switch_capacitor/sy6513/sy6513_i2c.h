/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sy6513_i2c.h
 *
 * sy6513 i2c header file
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

#ifndef _SY6513_I2C_H_
#define _SY6513_I2C_H_

#include "sy6513.h"

int sy6513_write_byte(struct sy6513_device_info *di, u8 reg, u8 value);
int sy6513_read_byte(struct sy6513_device_info *di, u8 reg, u8 *value);
int sy6513_read_word(struct sy6513_device_info *di, u8 reg, s16 *value);
int sy6513_read_block(struct sy6513_device_info *di, u8 reg, u8 *value, u8 len);
int sy6513_write_mask(struct sy6513_device_info *di, u8 reg, u8 mask, u8 shift, u8 value);
int sy6513_read_mask(struct sy6513_device_info *di, u8 reg, u8 mask, u8 shift, u8 *value);
int sy6513_write_multi_mask(struct sy6513_device_info *di, u8 reg, u8 mask, u8 value);

#endif /* _sy6513_I2C_H_ */
