// SPDX-License-Identifier: GPL-2.0
/*
 * hc32l110_i2c.c
 *
 * hc32l110 i2c interface
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "mcu_hc32l110_i2c.h"
#include "mcu_hc32l110_fw.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <linux/string.h>
#include "securec.h"

#define HWLOG_TAG mcu_hc32l110_i2c
HWLOG_REGIST();

#define RTC_TIME_LEN		7

int mcu_hc32l110_write_block(struct mcu_hc32l110_device_info *di, u8 reg, u8 *value,
	unsigned int num_bytes)
{
	u8 *buf = NULL;

	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EINVAL;
	}

	buf = kzalloc(num_bytes + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	buf[0] = reg;
	if (memcpy_s(&buf[1], num_bytes, value, num_bytes) != EOK) {
		kfree(buf);
		hwlog_err("mcu memcpy err\n");
		return -EINVAL;
	}

	if (di->is_low_power_mode) {
		kfree(buf);
		return -EINVAL;
	}

	return power_i2c_write_block(di->client, buf, num_bytes + 1);
}

int mcu_hc32l110_read_block(struct mcu_hc32l110_device_info *di, u8 reg, u8 *value,
	unsigned int num_bytes)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EINVAL;
	}

	if (di->is_low_power_mode)
		return -EINVAL;

	return power_i2c_read_block(di->client, &reg, 1, value, num_bytes);
}

int mcu_hc32l110_write_byte(struct mcu_hc32l110_device_info *di, u8 reg, u8 value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EINVAL;
	}

	if (di->is_low_power_mode)
		return -EINVAL;

	return power_i2c_u8_write_byte(di->client, reg, value);
}

int mcu_hc32l110_read_byte(struct mcu_hc32l110_device_info *di, u8 reg, u8 *value)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EINVAL;
	}

	if (di->is_low_power_mode)
		return -EINVAL;

	return power_i2c_u8_read_byte(di->client, reg, value);
}

int mcu_hc32l110_write_mask(struct mcu_hc32l110_device_info *di, u8 reg, u8 mask,
	u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = mcu_hc32l110_read_byte(di, reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return mcu_hc32l110_write_byte(di, reg, val);
}

int mcu_hc32l110_read_word_bootloader(struct mcu_hc32l110_device_info *di,
	u8 *buf, u8 buf_len)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EINVAL;
	}

	if (di->is_low_power_mode)
		return -EINVAL;

	return power_i2c_read_block_without_cmd(di->client, buf, buf_len);
}

int mcu_hc32l110_write_word_bootloader(struct mcu_hc32l110_device_info *di,
	u8 *cmd, u8 cmd_len)
{
	if (!di || (di->chip_already_init == 0)) {
		hwlog_err("chip not init\n");
		return -EINVAL;
	}

	if (di->is_low_power_mode)
		return -EINVAL;

	return power_i2c_write_block(di->client, cmd, cmd_len);
}

int mcu_hc32l110_write_rtc_time(struct mcu_hc32l110_device_info *di, struct rtc_time *tm)
{
	int ret;
	int idx = 0;
	u8 rtc_time[RTC_TIME_LEN] = {0};

	if ((di == NULL) || (tm == NULL))
		return -EINVAL;

	rtc_time[idx++] = tm->tm_sec;
	rtc_time[idx++] = tm->tm_min;
	rtc_time[idx++] = tm->tm_hour;
	rtc_time[idx++] = tm->tm_wday;
	rtc_time[idx++] = tm->tm_mday;
	rtc_time[idx++] = tm->tm_mon;
	rtc_time[idx++] = tm->tm_year;

	ret = mcu_hc32l110_write_block(di, MCU_HC32L110_TIME_SEC, rtc_time, RTC_TIME_LEN);
	if (ret) {
		hwlog_info("write time to mcu err\n");
		return ret;
	}
	hwlog_info("write time to mcu succeed\n");
	return 0;
}

int mcu_hc32l110_read_rtc_time(struct mcu_hc32l110_device_info *di, struct rtc_time *tm)
{
	int ret;
	u8 rtc_time[RTC_TIME_LEN] = {0};
	int idx = 0;

	if ((di == NULL) || (tm == NULL))
		return -EINVAL;

	ret = mcu_hc32l110_read_block(di, MCU_HC32L110_TIME_SEC, rtc_time, RTC_TIME_LEN);
	if (ret) {
		hwlog_info("read time from mcu err\n");
		return ret;
	}

	tm->tm_sec = rtc_time[idx++];
	tm->tm_min = rtc_time[idx++];
	tm->tm_hour = rtc_time[idx++];
	tm->tm_wday = rtc_time[idx++];
	tm->tm_mday = rtc_time[idx++];
	tm->tm_mon = rtc_time[idx++];
	tm->tm_year = rtc_time[idx++];

	hwlog_info("read time from mcu succeed\n");
	return 0;
}