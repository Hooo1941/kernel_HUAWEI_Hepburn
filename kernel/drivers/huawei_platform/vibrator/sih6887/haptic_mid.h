/*
  * haptic_mid.h
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

#ifndef _HAPTIC_MID_H_
#define _HAPTIC_MID_H_

#include <linux/kernel.h>
#include <linux/cdev.h>
#include "haptic.h"

#define hp_err(format, ...) \
	pr_err("[sih_hp]" format, ##__VA_ARGS__)

#define hp_info(format, ...) \
	pr_info("[sih_hp]" format, ##__VA_ARGS__)

#define hp_dbg(format, ...) \
	pr_debug("[sih_hp]" format, ##__VA_ARGS__)


#define SIH_I2C_OPERA_BYTE_ONE				1
#define SIH_I2C_OPERA_BYTE_TWO				2
#define SIH_I2C_OPERA_BYTE_THREE			3
#define SIH_I2C_OPERA_BYTE_FOUR				4
#define SIH_I2C_OPERA_BYTE_FIVE				5
#define SIH_I2C_OPERA_BYTE_SIX				6
#define SIH_I2C_OPERA_BYTE_SEVEN			7
#define SIH_I2C_OPERA_BYTE_EIGHT			8

#define SIH_HP_RTP_NAME_MAX				64

#define MEMDEV_DATABLOCK_SIZE				1024

#define SIH_CHECK_RAM_DATA
#define SIH_READ_BINFILE_FLEXBALLY
#define SIH_PRIORITY_ACQUIRE_DTS
#define SIH_ENABLE_PIN_CONTROL

#define SIH_HAPTIC_COMPAT_688X				"silicon,sih_haptic_688X"
#define SIH_HAPTIC_NAME_688X				"sih_haptic_688X"
#define SIH_HAPTIC_NAME					"vibrator"

#define SIH_READ_CHIP_RETRY_TIME			8
#define SIH_READ_CHIP_RETRY_DELAY			2
/*********************************************************
 *
 * Function Call
 *
 *********************************************************/
typedef struct sih_chipid_reg_val {
	uint32_t chip_id_reg;
	uint32_t reg_val;
} sih_chipid_reg_val_t;

extern haptic_func_t *sih_688x_func(void);

extern int i2c_read_bytes(sih_haptic_t *, uint8_t, uint8_t *, uint32_t);
extern int i2c_write_bytes(sih_haptic_t *, uint8_t, uint8_t *, uint32_t);
extern int i2c_write_bits(sih_haptic_t *, uint8_t, uint32_t, uint8_t);
extern int sih_register_func(sih_haptic_t *);
extern uint8_t crc4_itu(uint8_t *data, uint8_t length);

#ifdef SIH_PRIORITY_OPEN
extern void sih_register_priority(sih_haptic_t *, struct device_node *);
extern bool sih_judge_priority(sih_haptic_t *, sih_broadcast_priority_e);
#endif
#endif

