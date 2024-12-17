/*
  * sih688x_reg.c
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

#include <linux/regmap.h>
#include <linux/device.h>
#include "haptic_regmap.h"
#include "sih688x_reg.h"


static bool sih688x_writeable_register(
	struct device *dev,
	unsigned int reg)
{
	return true;
}

static bool sih688x_readable_register(
	struct device *dev,
	unsigned int reg)
{
	return true;
}

static bool sih688x_volatile_register(
	struct device *dev,
	unsigned int reg)
{
	return true;
}

const struct regmap_config sih688x_regmap_config = {
	.name = "sih688x",
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = SIH688X_REG_MAX,
	.writeable_reg = sih688x_writeable_register,
	.readable_reg = sih688x_readable_register,
	.volatile_reg = sih688x_volatile_register,
	.cache_type = REGCACHE_NONE,
};