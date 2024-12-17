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

#ifndef DPU_BL_FACTORY_BLPWM_DRV_H__
#define DPU_BL_FACTORY_BLPWM_DRV_H__

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of_address.h>
#include "dkmd_bl_factory.h"

struct blpwm_drv_private {
	char __iomem *blpwm_base;
	struct clk *blpwm_clk;
	uint32_t fpga_flag;
	uint32_t blpwm_on;
	struct platform_device *pdev;
};

int blpwm_drv_on(struct panel_drv_private *priv);
int blpwm_drv_off(struct panel_drv_private *priv);
int blpwm_drv_set_backlight(struct panel_drv_private *priv, uint32_t bkl_lvl);

#endif