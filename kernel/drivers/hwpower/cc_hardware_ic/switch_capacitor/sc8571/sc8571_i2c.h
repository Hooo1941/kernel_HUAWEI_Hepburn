/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sc8571_i2c.h
 *
 * sc8571 i2c header file
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

#ifndef _SC8571_I2C_H_
#define _SC8571_I2C_H_

#include "sc8571.h"

int sc8571_write_byte(struct sc8571_device_info *di, u8 reg, u8 value);
int sc8571_read_byte(struct sc8571_device_info *di, u8 reg, u8 *value);
int sc8571_read_word(struct sc8571_device_info *di, u8 reg, s16 *value);
int sc8571_write_mask(struct sc8571_device_info *di, u8 reg, u8 mask, u8 shift, u8 value);

#endif /* _SC8571_I2C_H_ */
