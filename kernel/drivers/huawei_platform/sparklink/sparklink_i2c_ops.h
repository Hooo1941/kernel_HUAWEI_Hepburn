/*
 * hi116x_driver.h
 *
 * interface for huawei hi116x_fwupdate driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
#ifndef _SPARKLINK_I2C_OPS_H_
#define _SPARKLINK_I2C_OPS_H_
#include <linux/i2c.h>

#define HI1162_I2C_RETRY 3
#define I2C_SMBUS_BLOCK_MAX 32

typedef struct _sparklink_reg {
	u32 addr;
	u32 size;
	u32 value;
} sparklink_reg;

int sparklink_i2c_read_byte(const struct i2c_client *client, u8 reg);
int sparklink_i2c_write_byte(const struct i2c_client *client, u8 reg, u8 value);
int sparklink_i2c_read_block(const struct i2c_client *client, u8 reg,
	u8 length, u8 *value_out);
int sparklink_i2c_write_block(const struct i2c_client *client, u8 reg,
	u8 length, u8 *value_in);
#endif /* _SPARKLINK_I2C_OPS_H_ */
