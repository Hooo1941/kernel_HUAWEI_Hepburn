/*
  * haptic_regmap.h
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

#ifndef _HAPTIC_REGMAP_H
#define _HAPTIC_REGMAP_H

int haptic_regmap_read(struct regmap *regmap,
unsigned int start_reg, unsigned int reg_num, char *buf);
int haptic_regmap_write(struct regmap *regmap,
	unsigned int start_reg, unsigned int reg_num, const char *buf);
int haptic_regmap_bulk_read(struct regmap *regmap,
	unsigned int start_reg, unsigned int reg_num, char *buf);
int haptic_regmap_bulk_write(struct regmap *regmap,
	unsigned int start_reg, unsigned int reg_num, const char *buf);
int haptic_regmap_update_bits(struct regmap *regmap,
	unsigned int reg, unsigned int mask, unsigned int val);
struct regmap *haptic_regmap_init(struct i2c_client *client,
	const struct regmap_config *config);
void haptic_regmap_remove(struct regmap *regmap);

#endif /* _HAPTIC_REGMAP_H */

