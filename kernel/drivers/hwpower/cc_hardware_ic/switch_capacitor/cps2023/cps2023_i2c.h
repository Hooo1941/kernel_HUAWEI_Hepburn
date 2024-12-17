/* SPDX-License-Identifier: GPL-2.0 */
/*
 * cps2023_i2c.h
 *
 * cps2023 i2c header file
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

#ifndef _CPS2023_I2C_H_
#define _CPS2023_I2C_H_

#include "cps2023.h"

int cps2023_write_block(struct cps2023_device_info *di, u8 reg, u8 *value, unsigned int num_bytes);
int cps2023_read_block(struct cps2023_device_info *di, u8 reg, u8 *value, unsigned int num_bytes);
int cps2023_write_byte(struct cps2023_device_info *di, u8 reg, u8 value);
int cps2023_read_byte(struct cps2023_device_info *di, u8 reg, u8 *value);
int cps2023_write_mask(struct cps2023_device_info *di, u8 reg, u8 mask, u8 shift, u8 value);
int cps2023_read_word(struct cps2023_device_info *di, u8 reg, u16 *value);
int cps2023_write_multi_mask(struct cps2023_device_info *di, u8 reg, u8 mask, u8 value);

#endif /* _CPS2023_I2C_H_ */
