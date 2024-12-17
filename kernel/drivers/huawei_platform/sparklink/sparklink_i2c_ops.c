/*
 * Huawei hi1162 Driver
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
#include <linux/delay.h>
#include "sparklink.h"
#include "sparklink_i2c_ops.h"

int sparklink_i2c_write_byte(const struct i2c_client *client, u8 reg, u8 value)
{
	int ret = -EINVAL;
	int i;

	if (!client) {
		sparklink_errmsg("client is null\n");
		return ret;
	}

	for (i = 0; (i < HI1162_I2C_RETRY) && (ret < 0); i++) {
		ret = i2c_smbus_write_byte_data(client, reg, value);
		if (ret < 0) {
			sparklink_errmsg("write_reg failed[%x]\n", reg);
			usleep_range(1000, 1100); /* sleep 1ms */
		}
	}

	return ret;
}

int sparklink_i2c_read_byte(const struct i2c_client *client, u8 reg)
{
	int ret = -EINVAL;
	int i;

	if (!client) {
		sparklink_errmsg("client is null\n");
		return ret;
	}

	for (i = 0; (i < HI1162_I2C_RETRY) && (ret < 0); i++) {
		ret = i2c_smbus_read_byte_data(client, reg);
		if (ret < 0) {
			sparklink_errmsg("read_reg failed[%x]\n", reg);
			usleep_range(1000, 1100); /* sleep 1ms */
		}
	}
	return ret;
}

int sparklink_i2c_read_block(const struct i2c_client *client, u8 reg, u8 length, u8 *value_out)
{
	int i, read_len;

	if (!client || !value_out || length > I2C_SMBUS_BLOCK_MAX) {
		sparklink_errmsg("client is null or value is null or over length\n");
		return -EINVAL;
	}

	for (i = 0; i < HI1162_I2C_RETRY; i++) {
		read_len = i2c_smbus_read_i2c_block_data(client, reg, length, value_out);
		if (read_len != length) {
			sparklink_errmsg("read block fail read_len:%d\n", read_len);
			usleep_range(1000, 1100); /* sleep 1ms */
		} else {
			return 0; /* read block data success */
		}
	}
	return -1;
}

/* i2c_smbus_write_i2c_block_data can write max 32 byte */
int sparklink_i2c_write_block(const struct i2c_client *client, u8 reg, u8 length, u8 *value_in)
{
	int ret, i;

	if (!client || !value_in || length > I2C_SMBUS_BLOCK_MAX) {
		sparklink_errmsg("client is null or value_in is null or over length\n");
		return -EINVAL;
	}

	for (i = 0; i < HI1162_I2C_RETRY; i++) {
		ret = i2c_smbus_write_i2c_block_data(client, reg, length, value_in);
		if (ret < 0) {
			sparklink_errmsg("write block fail ret:%d\n", ret);
			usleep_range(1000, 1100); /* sleep 1ms */
		} else {
			return 0; /* write block success */
		}
	}
	return ret;
}