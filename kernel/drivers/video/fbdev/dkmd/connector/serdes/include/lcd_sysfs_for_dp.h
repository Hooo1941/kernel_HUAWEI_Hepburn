/* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef LCD_SYSFS_DP_H
#define LCD_SYSFS_DP_H

#define COLUMN_NUM 4
#define REG_BUF_LEN 10
#define HEXADECIMAL 16
#define DECIMAL 10

#define APB_CTL     0x48
#define APB_ADR0    0x49
#define APB_ADR1    0x4A
#define APB_DATA0   0x4B
#define APB_DATA1   0x4C
#define APB_DATA2   0x4D
#define APB_DATA3   0x4E

#define SER_MAIN_REG_MAXCNT 0xf5

int lcd_sysfs_init_for_dp(struct i2c_client *client);

#endif /* LCD_SYSFS_DP_H */