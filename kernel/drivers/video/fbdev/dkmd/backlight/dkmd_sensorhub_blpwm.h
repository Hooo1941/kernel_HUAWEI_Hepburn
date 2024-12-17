/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#ifndef DKMD_SENSORHUB_BLPWM_H__
#define DKMD_SENSORHUB_BLPWM_H__

#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include "dkmd_log.h"
#include "dkmd_bl_factory.h"

struct bl_sh_blpwm_data {
	struct file *file;
};

int dkmd_sh_blpwm_on(struct panel_drv_private *priv);
int dkmd_sh_blpwm_off(struct panel_drv_private *priv);
int dkmd_sh_blpwm_set_backlight(struct panel_drv_private *priv, uint32_t bl_level);

#endif
