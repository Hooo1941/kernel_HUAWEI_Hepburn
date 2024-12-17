/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __I2C_COMMON_H__
#define __I2C_COMMON_H__

#include <securec.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include "log.h"

#define REG_DEF_MSK (0xFF) // register default mask
#define NO_DELAY (0)
#define I2C_TRANSFER_READ_NUM 2
#define I2C_MSG_BIT 8
#define UDELAY_TIMES 10
#define I2C_RETRY_CNT 3
#define I2C_MSG_LEN_2 2

typedef enum tag_reg_op_type {
	REG_OP_W = 0x0, // write register
	REG_OP_R = 0x1, // read register
} reg_op_type_t;

typedef struct i2c_cmd_entry {
	unsigned char reg;
	unsigned char val;
	unsigned int  wait_ms; // ms unit
	unsigned char op_type;
	unsigned char mask; // maybe use in future
} ti_i2c_cmd_entry_t;

typedef struct __tag_i2c_cmd_entry {
	unsigned char addr;
	unsigned char reg;
	unsigned char val;
	unsigned int  wait_ms; // ms unit
	unsigned char op_type;
	unsigned char mask; // maybe use in future
} i2c_cmd_entry_t;

static int i2c_read_addr_len1(struct i2c_adapter *adapter, unsigned char slave_addr,
	unsigned char reg_addr, unsigned char *data, uint32_t length)
{
	int err = -1;
	unsigned char reg_value = 0x00;
	int retry_cnt = I2C_RETRY_CNT;

	if (!adapter) {
		cdc_err("null ptr of i2c_adapter!");
		return -1;
	}

	struct i2c_msg msgs[] = {
		{
			.addr = slave_addr,
			.flags = 0,
			.len = 1,
			.buf = &reg_addr,
		},
		{
			.addr = slave_addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = data,
		},
	};

	do { /* retry 3 times for i2c link */
		err = i2c_transfer(adapter, msgs, I2C_TRANSFER_READ_NUM);
		cdc_debug("i2c read chip_addr = 0x%x, reg = 0x%x", slave_addr, reg_addr);
	} while ((err < 0) && (--retry_cnt));

	if (err < 0) {
		cdc_err("i2c read error chip_addr = 0x%x, reg = 0x%x, err=%d",
			slave_addr, reg_addr, err);
		return err;
	}
	return 0;
}

static int i2c_write_addr_len1(struct i2c_adapter *adapter, unsigned char slave_addr,
	unsigned char reg_addr, const unsigned char *data, uint32_t length)
{
	int err = -1;
	struct i2c_msg msg[1];
	int retry_cnt = I2C_RETRY_CNT;

	if (!adapter) {
		cdc_err("null ptr of i2c_adapter!");
		return -1;
	}

	msg[0].addr = slave_addr;
	msg[0].flags = 0;
	msg[0].len = length + 1;
	msg[0].buf = (char *)data;

	do { /* retry 3 times for i2c link */
		err = i2c_transfer(adapter, msg, 1);
		cdc_debug("i2c write chip_addr = 0x%x, reg = 0x%x", slave_addr, reg_addr);
	} while ((err < 0) && (--retry_cnt));

	if (err < 0) {
		cdc_err("i2c write error chip_addr = 0x%x, reg = 0x%x, err=%d",
			slave_addr, reg_addr, err);
		return err;
	}
	return 0;
}

static int i2c_write_reg_with_len(struct i2c_adapter *adapter, unsigned char slave_addr,
		unsigned char reg_addr, const unsigned char *buf, uint32_t length)
{
	int err = -1;
	struct i2c_msg msg[1];
	int retry_cnt = I2C_RETRY_CNT;

	return_value_if_run_error(!adapter, -1, "null ptr of i2c_adapter!");

	char data[length + 1];
	data[0] = reg_addr;
	err = memcpy_s(data + 1, length, buf, length);
	log_secure_c_failed_if(err != EOK, -err);

	msg[0].addr = slave_addr;
	msg[0].flags = 0;
	msg[0].len = sizeof(data);
	msg[0].buf = (char *)data;

	do { /* retry 3 times for i2c link */
		err = i2c_transfer(adapter, msg, 1);
		cdc_info("i2c write chip_addr = 0x%x, reg = 0x%x", slave_addr, reg_addr);
	} while ((err < 0) && (--retry_cnt));

	return_value_if_run_error(err < 0, err, "i2c write error chip_addr = 0x%x, reg = 0x%x, err=%d",
		slave_addr, reg_addr, err);
	return 0;
}

static int i2c_read_reg(struct i2c_adapter *adapter, unsigned char slave_addr,
	unsigned char register_addr, unsigned char* preg_value)
{
	return i2c_read_addr_len1(adapter, slave_addr, register_addr, preg_value, 1);
}

static int i2c_read_reg_len2(struct i2c_adapter *adapter, unsigned char slave_addr,
	unsigned char register_addr, int *preg_value)
{
	int err = -1;
	unsigned char reg_value[I2C_MSG_LEN_2] = {0};

	err = i2c_read_addr_len1(adapter, slave_addr, register_addr, reg_value, I2C_MSG_LEN_2);
	*preg_value = ((int)reg_value[0] << I2C_MSG_BIT) | (int)reg_value[1];

	return err;
}

static int i2c_write_reg(struct i2c_adapter *adapter,
	unsigned int slave_addr, unsigned char reg, unsigned char val)
{
	unsigned char  buf[2] = { 0 };
	buf[0] = reg;
	buf[1] = val;

	return i2c_write_addr_len1(adapter, slave_addr, reg, buf, 1);
}

static int i2c_write_reg_len2(struct i2c_adapter *adapter,
	unsigned int slave_addr, unsigned char reg, int val)
{
	unsigned char buf[I2C_MSG_LEN_2 + 1] = { 0 };

	buf[0] = reg;
	buf[1] = (val >> I2C_MSG_BIT) & 0xff;
	buf[I2C_MSG_LEN_2] = val & 0xff;

	return i2c_write_addr_len1(adapter, slave_addr, reg, buf, I2C_MSG_LEN_2);
}

int i2c_addr_access(struct i2c_adapter *adapter, unsigned char addr,
	ti_i2c_cmd_entry_t *accesses, int accesses_count);

static int i2c_access(struct i2c_adapter *adapter, i2c_cmd_entry_t *accesses, int accesses_count)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < accesses_count; i++) {
		if (accesses[i].op_type == REG_OP_R) {
			cdc_err("read ops not support now");
		} else {
			ret = i2c_write_reg(adapter, accesses[i].addr, accesses[i].reg, accesses[i].val);
			if (ret)
				cdc_err("fail write slv_addr:0x%x reg:0x%x val:0x%x delaytime = %d ms index =%d",
					accesses[i].addr, accesses[i].reg, accesses[i].val, accesses[i].wait_ms, i);
		}

		if (accesses[i].wait_ms != 0)
			mdelay(accesses[i].wait_ms);
	}
	return ret;
}
#endif
