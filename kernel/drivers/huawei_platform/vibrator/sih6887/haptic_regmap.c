/*
  * haptic_regmap.c
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
#include <linux/i2c.h>
#include "haptic_regmap.h"
#include "haptic_mid.h"
#include "sih688x_reg.h"

int __maybe_unused haptic_regmap_read(
	struct regmap *regmap,
	unsigned int start_reg,
	unsigned int reg_num,
	char *buf)
{
	unsigned int val = 0;
	int ret = 0;

	if (regmap == NULL) {
		pr_err("%s: NULL == regmap\r\n", __func__);
		return -EINVAL;
	}

	if (buf == NULL) {
		pr_err("%s: NULL == buf\r\n", __func__);
		return -EINVAL;
	}

	if (reg_num == 1) {
		ret = regmap_read(regmap, start_reg, &val);
		if (0 == ret)
			*buf = (char)val;

		return ret;
	}

	return regmap_raw_read(regmap, start_reg, buf, reg_num);
}

int __maybe_unused haptic_regmap_write(
	struct regmap *regmap,
	unsigned int start_reg,
	unsigned int reg_num,
	const char *buf)
{
	unsigned int val = 0;

	if (regmap == NULL) {
		pr_err("%s: NULL == regmap\r\n", __func__);
		return -EINVAL;
	}

	if (buf == NULL) {
		pr_err("%s: NULL == buf\r\n", __func__);
		return -EINVAL;
	}

	if (reg_num == 1) {
		val = (unsigned int)(*buf);
		return regmap_write(regmap, start_reg, val);
	}

	regcache_cache_bypass(regmap, true);
	return regmap_raw_write(regmap, start_reg, buf, reg_num);
}

int haptic_regmap_bulk_write(
	struct regmap *regmap,
	unsigned int start_reg,
	unsigned int reg_num,
	const char *buf)
{
	if (regmap == NULL) {
		pr_err("%s: NULL == regmap\r\n", __func__);
		return -EINVAL;
	}

	if (buf == NULL) {
		pr_err("%s: NULL == buf\r\n", __func__);
		return -EINVAL;
	}

	regcache_cache_bypass(regmap, true);
	return regmap_bulk_write(regmap, start_reg, buf, reg_num);
}

int haptic_regmap_bulk_read(
	struct regmap *regmap,
	unsigned int start_reg,
	unsigned int reg_num,
	char *buf)
{
	if (regmap == NULL) {
		pr_err("%s: NULL == regmap\r\n", __func__);
		return -EINVAL;
	}

	if (buf == NULL) {
		pr_err("%s: NULL == buf\r\n", __func__);
		return -EINVAL;
	}

	return regmap_bulk_read(regmap, start_reg, buf, reg_num);
}

int haptic_regmap_update_bits(
	struct regmap *regmap,
	unsigned int reg,
	unsigned int mask,
	unsigned int val)
{
	if (regmap == NULL)
		hp_err("%s: NULL == regmap\r\n", __func__);

	return regmap_update_bits(regmap, reg, mask, val);
}

struct regmap *haptic_regmap_init(
	struct i2c_client *client,
	const struct regmap_config *config)
{
	if (client == NULL)
		return NULL;

	return devm_regmap_init_i2c(client, config);
}

void haptic_regmap_remove(struct regmap *regmap)
{
	if (!IS_ERR(regmap))
		regmap_exit(regmap);
}

