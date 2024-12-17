/* Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef LCD_SYSFS_DSI_H
#define LCD_SYSFS_DSI_H

#define COLUMN_NUM 4
#define REG_BUF_LEN 10
#define HEXADECIMAL 16
#define DECIMAL 10

int lcd_sysfs_init_for_dsi(struct i2c_client *client);
void set_last_backlight(uint32_t last_backlight);
uint32_t get_last_backlight(void);

#endif
