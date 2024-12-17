/* SPDX-License-Identifier: GPL-2.0 */
/*
 * nu2205_i2c.h
 *
 * nu2205 i2c header file
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

#ifndef _NU2205_I2C_H_
#define _NU2205_I2C_H_

#include "nu2205.h"

int nu2205_write_byte(struct nu2205_device_info *di, u8 reg, u8 value);
int nu2205_read_byte(struct nu2205_device_info *di, u8 reg, u8 *value);
int nu2205_read_word(struct nu2205_device_info *di, u8 reg, s16 *value);
int nu2205_write_mask(struct nu2205_device_info *di, u8 reg, u8 mask, u8 shift, u8 value);

#endif /* _NU2205_I2C_H_ */
