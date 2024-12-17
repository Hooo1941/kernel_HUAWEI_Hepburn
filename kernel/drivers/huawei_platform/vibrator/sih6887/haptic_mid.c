/*
  * haptic_mid.c
  *
  * code for vibrator
  *
  * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#include <linux/i2c.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/errno.h>
#include "haptic_mid.h"
#include "sih688x_reg.h"
#include "haptic_regmap.h"

#ifdef SIH_PRIORITY_OPEN
static sih_work_priority_t default_pri = {
	.ram = SIH_BROADCAST_PRI_MIN,
	.loopram = SIH_BROADCAST_PRI_LOW,
	.cont = SIH_BROADCAST_PRI_MID,
	.trig = SIH_BROADCAST_PRI_MAX,
	.rtp = SIH_BROADCAST_PRI_HIGH,
};
#endif

/*********************************************************
 *
 * I2C Read/Write
 *
 *********************************************************/
int i2c_read_bytes(
	sih_haptic_t *sih_haptic,
	uint8_t reg_addr,
	uint8_t *buf,
	uint32_t len)
{
	int ret = -1;

	ret = i2c_master_send(sih_haptic->i2c, &reg_addr,
		SIH_I2C_OPERA_BYTE_ONE);
	if (ret < 0) {
		hp_err("%s: couldn't send addr:0x%02x, ret=%d\n",
			__func__, reg_addr, ret);
		return ret;
	}
	ret = i2c_master_recv(sih_haptic->i2c, buf, len);
	if (ret != len) {
		hp_err("%s: couldn't read data, ret=%d\n", __func__, ret);
		return ret;
	}
	return ret;
}

int i2c_write_bytes(
	sih_haptic_t *sih_haptic,
	uint8_t reg_addr,
	uint8_t *buf,
	uint32_t len)
{
	uint8_t *data = NULL;
	int ret = -1;

	data = kmalloc(len + 1, GFP_KERNEL);
	data[0] = reg_addr;
	memcpy(&data[1], buf, len);
	ret = i2c_master_send(sih_haptic->i2c, data, len + 1);
	if (ret < 0)
		hp_err("%s: i2c master send 0x%02x err\n", __func__, reg_addr);
	kfree(data);
	return ret;
}

int i2c_write_bits(
	sih_haptic_t *sih_haptic,
	uint8_t reg_addr,
	uint32_t mask,
	uint8_t reg_data)
{
	uint8_t reg_val = 0;
	int ret = -1;

	ret = i2c_read_bytes(sih_haptic, reg_addr,
		&reg_val, SIH_I2C_OPERA_BYTE_ONE);
	if (ret < 0) {
		hp_err("%s: i2c read error, ret=%d\n", __func__, ret);
		return ret;
	}
	reg_val &= mask;
	reg_val |= reg_data;
	ret = i2c_write_bytes(sih_haptic, reg_addr,
		&reg_val, SIH_I2C_OPERA_BYTE_ONE);
	if (ret < 0)
		hp_err("%s: i2c write error, ret=%d\n", __func__, ret);
	return ret;
}

int sih_register_func(sih_haptic_t *sih_haptic)
{
	sih_haptic->hp_func = sih_688x_func();
	sih_haptic->regmapp.config = &sih688x_regmap_config;
	return 0;
}

#ifdef SIH_PRIORITY_OPEN
void sih_register_priority(
	sih_haptic_t *sih_haptic,
	struct device_node *np)
{
#ifdef SIH_PRIORITY_ACQUIRE_DTS
	int ret = 0;
	uint32_t value_buf[6] = {0};

	ret = of_property_read_u32_array(np, "work_priority", value_buf, 5);
	if (ret) {
		memcpy(&sih_haptic->register_pri, &default_pri,
			sizeof(sih_work_priority_t));
	} else {
		sih_haptic->register_pri.ram = value_buf[0];
		sih_haptic->register_pri.loopram = value_buf[1];
		sih_haptic->register_pri.cont = value_buf[2];
		sih_haptic->register_pri.trig = value_buf[3];
		sih_haptic->register_pri.rtp = value_buf[4];
	}
#else
	memcpy(&sih_haptic->register_pri, &default_pri,
		sizeof(sih_work_priority_t));
#endif
}

bool sih_judge_priority(
	sih_haptic_t *sih_haptic,
	sih_broadcast_priority_e dest_pri)
{
	return (dest_pri >= sih_haptic->chip_ipara.cur_pri) ? true : false;
}

uint8_t crc4_itu(uint8_t *data, uint8_t length)
{
	uint8_t i;
	uint8_t crc = 0;
	while (length--) {
		crc ^= *data++;
		for (i = 0; i < 8; ++i) {
			if (crc & 1)
				crc = (crc >> 1) ^ 0x0C;
			else
				crc = (crc >> 1);
		}
	}
	return crc;
}

#endif
