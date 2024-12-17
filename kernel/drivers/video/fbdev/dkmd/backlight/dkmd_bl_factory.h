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
#ifndef DPU_BL_FACTORY_H__
#define DPU_BL_FACTORY_H__

#include "dkmd_dpu.h"
#include "dpu_conn_mgr.h"
#include "panel_dev.h"
#include "panel_mgr.h"
#include "panel_utils.h"

enum bl_control_mode {
	REG_ONLY_MODE = 1,
	PWM_ONLY_MODE,
	MUTI_THEN_RAMP_MODE,
	RAMP_THEN_MUTI_MODE,
	I2C_ONLY_MODE = 6,
	BLPWM_AND_CABC_MODE,
	COMMON_IC_MODE = 8,
	AMOLED_NO_BL_IC_MODE = 9,
	BLPWM_MODE = 10,
};

enum mipi_brightness_para_type {
	MIPI_BL_PARA1_DBV0, // parameter1 start from DBV0
	MIPI_BL_PARA1_DBV0_AND_PARA2_DBV8, // parameter1 start from DBV0, parameter2 start from DBV8
	MIPI_BL_PARA1_DBV8_AND_PARA2_DBV0, // parameter1 start from DBV8, parameter2 start from DBV0
};

enum BLPWM_PRECISION_TYPE {
	BLPWM_PRECISION_DEFAULT_TYPE = 0,
	BLPWM_PRECISION_10000_TYPE = 1,
	BLPWM_PRECISION_2048_TYPE = 2,
	BLPWM_PRECISION_4096_TYPE = 3,
	BLPWM_PRECISION_TYPE_MAX,
};

void bl_factory_on(struct panel_drv_private *priv,
	uint32_t bl_type);
void bl_factory_off(struct panel_drv_private *priv,
	uint32_t bl_type);
void bl_factory_set_backlight(struct panel_drv_private *priv,
	uint32_t bl_type, uint32_t value);

#endif