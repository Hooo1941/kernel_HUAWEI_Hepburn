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

#ifndef DPU_BL_FACTORY_BLPWM_H__
#define DPU_BL_FACTORY_BLPWM_H__

#include "dkmd_blpwm_api.h"
#include "dkmd_log.h"
#include "dkmd_blpwm_drv.h"
#include "dkmd_bl_factory.h"

int dkmd_blpwm_on(struct panel_drv_private *priv);
int dkmd_blpwm_off(struct panel_drv_private *priv);
int dkmd_blpwm_set_backlight(struct panel_drv_private *priv, uint32_t bkl_lvl);

#endif
