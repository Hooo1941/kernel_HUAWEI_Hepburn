/*
 * lcd_kit_bias_core.h
 *
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_BIAS_CORE_H
#define __LCD_KIT_BIAS_CORE_H

#define LCD_BIAS_IC_NAME_LEN 20
#define LCD_BIAS_INIT_CMD_NUM 10

#define LCD_BIAS_VOL_400 4000000
#define LCD_BIAS_VOL_410 4100000
#define LCD_BIAS_VOL_420 4200000
#define LCD_BIAS_VOL_430 4300000
#define LCD_BIAS_VOL_440 4400000
#define LCD_BIAS_VOL_450 4500000
#define LCD_BIAS_VOL_460 4600000
#define LCD_BIAS_VOL_470 4700000
#define LCD_BIAS_VOL_480 4800000
#define LCD_BIAS_VOL_490 4900000
#define LCD_BIAS_VOL_500 5000000
#define LCD_BIAS_VOL_510 5100000
#define LCD_BIAS_VOL_520 5200000
#define LCD_BIAS_VOL_530 5300000
#define LCD_BIAS_VOL_540 5400000
#define LCD_BIAS_VOL_550 5500000
#define LCD_BIAS_VOL_560 5600000
#define LCD_BIAS_VOL_570 5700000
#define LCD_BIAS_VOL_580 5800000
#define LCD_BIAS_VOL_590 5900000
#define LCD_BIAS_VOL_600 6000000
#define LCD_BIAS_VOL_610 6100000

/* bias ic hw_rt4801 */
enum bias_common_val_type {
	BIAS_COMMON_VOL_40 = 0x00, /* 4.0V */
	BIAS_COMMON_VOL_41 = 0x01, /* 4.1V */
	BIAS_COMMON_VOL_42 = 0x02, /* 4.2V */
	BIAS_COMMON_VOL_43 = 0x03, /* 4.3V */
	BIAS_COMMON_VOL_44 = 0x04, /* 4.4V */
	BIAS_COMMON_VOL_45 = 0x05, /* 4.5V */
	BIAS_COMMON_VOL_46 = 0x06, /* 4.6V */
	BIAS_COMMON_VOL_47 = 0x07, /* 4.7V */
	BIAS_COMMON_VOL_48 = 0x08, /* 4.8V */
	BIAS_COMMON_VOL_49 = 0x09, /* 4.9V */
	BIAS_COMMON_VOL_50 = 0x0A, /* 5.0V */
	BIAS_COMMON_VOL_51 = 0x0B, /* 5.1V */
	BIAS_COMMON_VOL_52 = 0x0C, /* 5.2V */
	BIAS_COMMON_VOL_53 = 0x0D, /* 5.3V */
	BIAS_COMMON_VOL_54 = 0x0E, /* 5.4V */
	BIAS_COMMON_VOL_55 = 0x0F, /* 5.5V */
	BIAS_COMMON_VOL_56 = 0x10, /* 5.6V */
	BIAS_COMMON_VOL_57 = 0x11, /* 5.7V */
	BIAS_COMMON_VOL_58 = 0x12, /* 5.8V */
	BIAS_COMMON_VOL_59 = 0x13, /* 5.9V */
	BIAS_COMMON_VOL_60 = 0x14, /* 6.0V */
	BIAS_COMMON_VOL_MAX = 0x15, /* 6.1V */
};

struct bias_ic_voltage {
	unsigned int voltage;
	unsigned int value;
};

struct bias_ic_config {
	char name[LCD_BIAS_IC_NAME_LEN];
	unsigned char len;
	struct bias_ic_voltage *vol_table;
};

enum bias_reg_ops_mode {
	REG_READ_MODE = 0,
	REG_WRITE_MODE = 1,
	REG_UPDATE_MODE = 2,
	REG_NOT_SUPPORT = 0xff,
};

struct bias_ic_cmd {
	unsigned char ops_type; /* 0:read  1:write  2:update */
	unsigned char cmd_reg;
	unsigned char cmd_val; /* bits */
	unsigned char cmd_mask;
	unsigned char delay;
};

struct lcd_kit_bias_info {
	unsigned int num_of_init_cmd;
	struct bias_ic_cmd verify_cmds;
	struct bias_ic_cmd init_cmds[LCD_BIAS_INIT_CMD_NUM];
	struct bias_ic_cmd vsp_cmd;
	struct bias_ic_cmd vsn_cmd;
};

struct lcd_kit_bias_device {
	struct device *dev;
	struct i2c_client *client;
	struct lcd_kit_bias_info config;
};

#endif
