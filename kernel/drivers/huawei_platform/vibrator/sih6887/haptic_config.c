/*
  * haptic_config.c
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

#include <linux/module.h>
#include <linux/debugfs.h>
#include <asm/ioctls.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/leds.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/firmware.h>

#include "haptic_mid.h"
#include "haptic_regmap.h"
#include "sih688x_reg.h"
#include "haptic_config.h"


#define HAPTIC_CONFIG_FILE_NUM				3
#define HAPTIC_CONFIG_FILE_INDEX			0
#define HAPTIC_CONFIG_FILE_BUF_LEN			64

char sih_config_name[HAPTIC_CONFIG_FILE_NUM][HAPTIC_CONFIG_FILE_BUF_LEN] = {
	{"sih6887_config.bin"},
};

static void sih_config_load(
	const struct firmware *cont,
	void *context)
{
	int i = 0;
	sih_haptic_t *sih_haptic = context;
	uint8_t reg_addr = 0;
	uint8_t reg_value = 0;
	haptic_reg_config *reg_config = NULL;

	hp_info("%s: enter\n", __func__);

	if (!cont) {
		hp_err("%s: failed to read %s\n",
			__func__, sih_config_name[HAPTIC_CONFIG_FILE_INDEX]);
		release_firmware(cont);
		return;
	}

	reg_config = (haptic_reg_config *)cont->data;

	/* check config file */
	if (cont->size != reg_config->reg_num * 2 + 1) {
		hp_err("%s: file not valid!  reg_num:%d  cont size:%d\n",
			__func__, reg_config->reg_num, (int)cont->size);
		release_firmware(cont);
		return;
	}

	mutex_lock(&sih_haptic->lock);
	/* load config */
	for (i = 0; i < reg_config->reg_num; i++) {
		reg_addr = reg_config->reg_cont[i].reg_addr;
		reg_value = reg_config->reg_cont[i].reg_value;
		haptic_regmap_write(sih_haptic->regmapp.regmapping,
			reg_addr, SIH_I2C_OPERA_BYTE_ONE, &reg_value);
		hp_info("%s: 0x%02x:0x%02x\n", __func__, reg_addr, reg_value);
	}
	mutex_unlock(&sih_haptic->lock);

	hp_info("%s: chip config loaded!\n", __func__);

	release_firmware(cont);
}

int sih_chip_config(sih_haptic_t *sih_haptic)
{
	int len = 0;

	len = request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
		sih_config_name[HAPTIC_CONFIG_FILE_INDEX],
		sih_haptic->dev, GFP_KERNEL,
		sih_haptic, sih_config_load);

	return len;
}