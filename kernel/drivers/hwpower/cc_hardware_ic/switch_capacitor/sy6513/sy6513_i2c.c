// SPDX-License-Identifier: GPL-2.0
/*
 * sy6513_i2c.c
 *
 * sy6513 i2c interface
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

#include "sy6513_i2c.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG sy6513_i2c
HWLOG_REGIST();

int sy6513_write_byte(struct sy6513_device_info *di, u8 reg, u8 value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

int sy6513_read_byte(struct sy6513_device_info *di, u8 reg, u8 *value)
{
	if (!di || !value || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

int sy6513_read_word(struct sy6513_device_info *di, u8 reg, s16 *value)
{
	u16 data = 0;

	if (!di || !value || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	if (power_i2c_u8_read_word(di->client, reg, &data, true))
		return -EIO;

	*value = (s16)data;
	return 0;
}

int sy6513_read_block(struct sy6513_device_info *di, u8 reg, u8 *value, u8 len)
{
	if (!di || !value || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_read_block(di->client, &reg, 1, value, len);
}

int sy6513_write_mask(struct sy6513_device_info *di, u8 reg, u8 mask, u8 shift, u8 value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte_mask(di->client, reg, value, mask, shift);
}

int sy6513_read_mask(struct sy6513_device_info *di, u8 reg, u8 mask, u8 shift, u8 *value)
{
	if (!di || !value || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte_mask(di->client, reg, value, mask, shift);
}

int sy6513_write_multi_mask(struct sy6513_device_info *di, u8 reg, u8 mask, u8 value)
{
	u8 data = 0;
	int ret;
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	ret = power_i2c_u8_read_byte(di->client, reg, &data);
	if (ret)
		return ret;

	value = (data & ~mask) | (value & mask);
	return power_i2c_u8_write_byte(di->client, reg, value);
}
