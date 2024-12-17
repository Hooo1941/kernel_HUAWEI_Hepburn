// SPDX-License-Identifier: GPL-2.0
/*
 * cps2023_i2c.c
 *
 * cps2023 i2c interface
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

#include "cps2023.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cps2023_i2c
HWLOG_REGIST();

int cps2023_write_block(struct cps2023_device_info *di, u8 reg, u8 *value,
	unsigned int num_bytes)
{
	u8 *buf = NULL;
	int ret;

	if (!di || di->is_reboot || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	buf = kzalloc(num_bytes + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	buf[0] = reg;
	memcpy(&buf[1], value, num_bytes);

	ret = power_i2c_write_block(di->client, buf, num_bytes + 1);
	kfree(buf);

	return ret;
}

int cps2023_read_block(struct cps2023_device_info *di, u8 reg, u8 *value,
	unsigned int num_bytes)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_read_block(di->client, &reg, 1, value, num_bytes);
}

int cps2023_write_byte(struct cps2023_device_info *di, u8 reg, u8 value)
{
	if (!di || di->is_reboot || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

int cps2023_read_byte(struct cps2023_device_info *di, u8 reg, u8 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

int cps2023_write_mask(struct cps2023_device_info *di, u8 reg, u8 mask, u8 shift,
	u8 value)
{
	int ret;
	u8 val = 0;

	if (!di || di->is_reboot || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	ret = cps2023_read_byte(di, reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return cps2023_write_byte(di, reg, val);
}

int cps2023_read_word(struct cps2023_device_info *di, u8 reg, u16 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_word(di->client, reg, value, true);
}

int cps2023_write_multi_mask(struct cps2023_device_info *di, u8 reg, u8 mask, u8 value)
{
	u8 data = 0;
	int ret;

	if (!di || di->is_reboot || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	ret = power_i2c_u8_read_byte(di->client, reg, &data);
	if (ret)
		return ret;

	value = (data & ~mask) | (value & mask);
	return power_i2c_u8_write_byte(di->client, reg, value);
}
