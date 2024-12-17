/*
 * mcu.h
 *
 * mcu driver header
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
#ifndef _MCU_HC32L110_I2C_H_
#define _MCU_HC32L110_I2C_H_

#include <linux/rtc.h>
#include "mcu_hc32l110.h"

int mcu_hc32l110_write_block(struct mcu_hc32l110_device_info *di, u8 reg, u8 *value,
	unsigned int num_bytes);
int mcu_hc32l110_read_block(struct mcu_hc32l110_device_info *di, u8 reg, u8 *value,
	unsigned int num_bytes);
int mcu_hc32l110_write_byte(struct mcu_hc32l110_device_info *di, u8 reg, u8 value);
int mcu_hc32l110_read_byte(struct mcu_hc32l110_device_info *di, u8 reg, u8 *value);
int mcu_hc32l110_write_mask(struct mcu_hc32l110_device_info *di, u8 reg, u8 mask,
	u8 shift, u8 value);
int mcu_hc32l110_read_word_bootloader(struct mcu_hc32l110_device_info *di,
	u8 *buf, u8 buf_len);
int mcu_hc32l110_write_word_bootloader(struct mcu_hc32l110_device_info *di,
	u8 *cmd, u8 cmd_len);
int mcu_hc32l110_write_rtc_time(struct mcu_hc32l110_device_info *di, struct rtc_time *tm);
int mcu_hc32l110_read_rtc_time(struct mcu_hc32l110_device_info *di, struct rtc_time *tm);

#endif // MCU_HC32L110_I2C_H
