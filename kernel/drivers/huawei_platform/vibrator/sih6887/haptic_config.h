/*
  * haptic_config.h
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

#ifndef _HAPTIC_CONFIG_H
#define _HAPTIC_CONFIG_H

#include "haptic.h"

#define HAPTIC_CONFIG_FILE_PATH				"mnt"
#define HAPTIC_CONFIG_MAX_REG_NUM			256

typedef struct haptic_reg_format_t {
	uint8_t reg_addr;
	uint8_t reg_value;
} haptic_reg_format;

typedef struct haptic_reg_config_t {
	uint8_t reg_num;
	haptic_reg_format reg_cont[HAPTIC_CONFIG_MAX_REG_NUM];
} haptic_reg_config;

int sih_chip_config(sih_haptic_t *sih_haptic);
#endif /* _HAPTIC_REGMAP_H */

