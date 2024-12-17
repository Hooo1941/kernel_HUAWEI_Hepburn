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
#include "dkmd_bl_factory.h"
#include "dkmd_blpwm.h"
#include "dkmd_mipi_bl.h"

void bl_factory_on(struct panel_drv_private *priv,
	uint32_t bl_type)
{
	if (unlikely(!priv)) {
		dpu_pr_err("priv is null\n");
		return;
	}

	switch (bl_type) {
	case BL_SET_BY_BLPWM:
		dkmd_blpwm_on(priv);
		break;
	case BL_SET_BY_MIPI:
		break;
	default:
		dpu_pr_info("[bl_factory]: on default process, bl type %u",
			bl_type);
		return;
	}
}

void bl_factory_off(struct panel_drv_private *priv,
	uint32_t bl_type)
{
	if (unlikely(!priv)) {
		dpu_pr_err("priv is null\n");
		return;
	}

	switch (bl_type) {
	case BL_SET_BY_BLPWM:
		dkmd_blpwm_off(priv);
		break;
	case BL_SET_BY_MIPI:
		break;
	default:
		dpu_pr_info("[bl_factory]: off default process, bl type %u",
			bl_type);
		return;
	}
}

void bl_factory_set_backlight(struct panel_drv_private *priv,
	uint32_t bl_type, uint32_t value)
{
	if (unlikely(!priv)) {
		dpu_pr_err("priv is null\n");
		return;
	}

	switch (bl_type) {
	case BL_SET_BY_BLPWM:
		dkmd_blpwm_set_backlight(priv, value);
		break;
	case BL_SET_BY_MIPI:
		dkmd_mipi_bl_set_backlight(priv, value);
		break;
	default:
		dpu_pr_info("[bl_factory]: set_backlight default process,bl type %u",
			bl_type);
		return;
	}
}