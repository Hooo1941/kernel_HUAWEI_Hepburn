// SPDX-License-Identifier: GPL-2.0
/*
 * sc8562_i2c.c
 *
 * sc8562 i2c interface
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "sc8562_i2c.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG sc8562_i2c
HWLOG_REGIST();

int sc8562_write_byte(struct sc8562_device_info *di, u8 reg, u8 value)
{
	if (!di || di->is_reboot || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

int sc8562_read_byte(struct sc8562_device_info *di, u8 reg, u8 *value)
{
	if (!di || !value || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

int sc8562_read_word(struct sc8562_device_info *di, u8 reg, s16 *value)
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

int sc8562_read_block(struct sc8562_device_info *di, u8 reg, u8 *value, u8 len)
{
	if (!di || !value || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_read_block(di->client, &reg, 1, value, len);
}

int sc8562_write_mask(struct sc8562_device_info *di, u8 reg, u8 mask, u8 shift, u8 value)
{
	if (!di || di->is_reboot || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte_mask(di->client, reg, value, mask, shift);
}

int sc8562_read_mask(struct sc8562_device_info *di, u8 reg, u8 mask, u8 shift, u8 *value)
{
	if (!di || !value || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte_mask(di->client, reg, value, mask, shift);
}

int sc8562_write_multi_mask(struct sc8562_device_info *di, u8 reg, u8 mask, u8 value)
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
