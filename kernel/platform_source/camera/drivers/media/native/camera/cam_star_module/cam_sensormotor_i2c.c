// SPDX-License-Identifier: GPL-2.0
/*
 * cam_sensormotor_i2c.c
 *
 * i2c interface for cam_sensormotor module
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

#include "cam_sensormotor_i2c.h"
#include "cam_sensormotor_dmd_report.h"
#include "cam_star_module_adapter.h"
#include <linux/delay.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG cam_sensormotor_i2c
HWLOG_REGIST();

/*
 * cam_usleep - sleep for an approximate time
 * @delay: total time in usecs to sleep
 *
 * Notes that if delay > 20ms, please use cam_msleep instead.
 */
void cam_usleep(unsigned int delay)
{
	unsigned int delay_max;

	if (delay <= DT_USLEEP_2MS)
		delay_max = delay + DT_USLEEP_100US;
	else if (delay <= DT_USLEEP_5MS)
		delay_max = delay + DT_USLEEP_500US;
	else
		delay_max = delay + DT_USLEEP_1MS;

	hwlog_info("cam_star usleep delay start:%u\n", delay);
	usleep_range(delay, delay_max);
	hwlog_info("cam_star usleep delay end:%u\n", delay);
}

int i2c_read_block_without_cmd(struct i2c_client *client,
	u8 *buf, u8 buf_len)
{
	struct i2c_msg msg;
	struct cam_star_module_device_info *di = NULL;

	if (!client || !buf) {
		hwlog_err("client or buf is null\n");
		return -EINVAL;
	}
	if (!((&client->dev)->driver_data)) {
		hwlog_err("client driver_data is null\n");
		return -EINVAL;
	}
	di = (struct cam_star_module_device_info *)((&client->dev)->driver_data);

	/* flags: I2C_M_RD is read data, from slave to master */
	msg.addr = client->addr;
	msg.flags = I2C_M_RD;
	msg.buf = buf;
	msg.len = buf_len;

	if (i2c_transfer(client->adapter, &msg, 1) != 1) {
		hwlog_err("addr=%x read fail\n", client->addr);
		cam_sensormotor_dsm_i2c_err(di->dsm_client);
		return -EIO;
	}

	return 0;
}

int i2c_write_block(struct i2c_client *client, u8 *buf, u8 buf_len)
{
	struct i2c_msg msg[LEN_WR_MSG];
	struct cam_star_module_device_info *di = NULL;

	if (!client || !buf) {
		hwlog_err("client or buf is null\n");
		return -EINVAL;
	}

	if (!((&client->dev)->driver_data)) {
		hwlog_err("client driver_data is null\n");
		return -EINVAL;
	}
	di = (struct cam_star_module_device_info *)((&client->dev)->driver_data);
	/* flags: 0 is write data, from master to slave */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].buf = buf;
	msg[0].len = buf_len;

	/* i2c_transfer return number of messages transferred */
	if (i2c_transfer(client->adapter, msg, LEN_WR_MSG) != LEN_WR_MSG) {
		hwlog_err("addr=%x write fail\n", client->addr);
		cam_sensormotor_dsm_i2c_err(di->dsm_client);
		return -EIO;
	}

	return 0;
}

int i2c_read_block(struct i2c_client *client, u8 *cmd, u8 cmd_len,
	u8 *buf, u8 buf_len)
{
	struct i2c_msg msg[LEN_RD_MSG];
	struct cam_star_module_device_info *di = NULL;

	if (!client || !cmd || !buf) {
		hwlog_err("client or cmd or buf is null\n");
		return -EINVAL;
	}
	if (!((&client->dev)->driver_data)) {
		hwlog_err("client driver_data is null\n");
		return -EINVAL;
	}
	di = (struct cam_star_module_device_info *)((&client->dev)->driver_data);

	/* the cmd to be written before reading data */
	/* flags: 0 is write data, from master to slave */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].buf = cmd;
	msg[0].len = cmd_len;

	/* flags: I2C_M_RD is read data, from slave to master */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = buf_len;

	/* i2c_transfer return number of messages transferred */
	if (i2c_transfer(client->adapter, msg, LEN_RD_MSG) != LEN_RD_MSG) {
		hwlog_err("addr=%x read fail\n", client->addr);
		cam_sensormotor_dsm_i2c_err(di->dsm_client);
		return -EIO;
	}

	return 0;
}

int i2c_u8_write_byte(struct i2c_client *client, u8 reg, u8 data)
{
	u8 buf[LEN_U8_REG + LEN_U8_DAT] = { 0 };
	int rc = 0;
	struct cam_star_module_device_info *di = NULL;
	di = (struct cam_star_module_device_info *)((&client->dev)->driver_data);

	/*
	 * buf[0]: 8bit slave register address
	 * buf[1]: 8bit data to be written
	 */
	buf[0] = reg;
	buf[1] = data;

	if (i2c_write_block(client, buf, sizeof(buf))) {
		if (di && di->api_func_tbl->read_mcu_i2c_debug_data)
			rc = di->api_func_tbl->read_mcu_i2c_debug_data(di);
		return -EIO;
	}

	return 0;
}

int i2c_u8_read_byte(struct i2c_client *client, u8 reg, u8 *data)
{
	u8 cmd[LEN_U8_REG] = { 0 };
	u8 buf[LEN_U8_DAT] = { 0 };
	int rc = 0;
	struct cam_star_module_device_info *di = NULL;
	di = (struct cam_star_module_device_info *)((&client->dev)->driver_data);

	/* cmd[0]: 8bit register */
	cmd[0] = reg;

	if (i2c_read_block(client, cmd, sizeof(cmd), buf, sizeof(buf))) {
		if (di && di->api_func_tbl->read_mcu_i2c_debug_data)
			rc = di->api_func_tbl->read_mcu_i2c_debug_data(di);
		return -EIO;
	}

	/* buf[0]: 8bit data */
	*data = buf[0];
	return 0;
}
