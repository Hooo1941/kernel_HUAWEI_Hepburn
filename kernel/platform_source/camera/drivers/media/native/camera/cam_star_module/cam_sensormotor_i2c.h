/* SPDX-License-Identifier: GPL-2.0 */
/* power_i2c.h
 *
 * i2c interface for power module
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

#ifndef _CAM_SENSORMOTOR_I2C_H_
#define _CAM_SENSORMOTOR_I2C_H_

#include <linux/i2c.h>

#define LEN_WR_MSG     1
#define LEN_RD_MSG     2
#define LEN_U8_REG     1
#define LEN_U16_REG    2
#define LEN_U8_DAT     1
#define LEN_U16_DAT    2
#define LEN_U32_DAT    4

#define DT_USLEEP_100US    100
#define DT_USLEEP_500US    500
#define DT_USLEEP_1MS      1000
#define DT_USLEEP_2MS      2000
#define DT_USLEEP_5MS      5000
#define DT_USLEEP_7MS      7000

void cam_usleep(unsigned int delay);
int i2c_read_block_without_cmd(struct i2c_client *client, u8 *buf, u8 buf_len);
int i2c_write_block(struct i2c_client *client, u8 *buf, u8 buf_len);
int i2c_read_block(struct i2c_client *client, u8 *cmd, u8 cmd_len,
	u8 *buf, u8 buf_len);
int i2c_u8_write_byte(struct i2c_client *client, u8 reg, u8 data);
int i2c_u8_read_byte(struct i2c_client *client, u8 reg, u8 *data);

#endif /* _CAM_SENSORMOTOR_I2C_H_ */
