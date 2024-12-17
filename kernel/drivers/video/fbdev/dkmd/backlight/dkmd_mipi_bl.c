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
#include "dkmd_mipi_bl.h"

static void get_bl_level_adjust(uint32_t mipi_brightness_para_type, uint32_t bl_level, char *bl_level_adjust)
{
	switch(mipi_brightness_para_type) {
	case MIPI_BL_PARA1_DBV0:
		bl_level_adjust[1] = bl_level & 0xff;
		break;
	case MIPI_BL_PARA1_DBV0_AND_PARA2_DBV8:
		bl_level_adjust[1] = bl_level & 0xff;
		bl_level_adjust[2] = (bl_level >> 8) & 0xff;
		break;
	case MIPI_BL_PARA1_DBV8_AND_PARA2_DBV0:
		bl_level_adjust[1] = (bl_level >> 8) & 0xff;
		bl_level_adjust[2] = bl_level & 0xff;
		break;
	default:
		bl_level_adjust[1] = bl_level & 0xff;
		dpu_pr_warn("not support parameter type:%d, use default value", mipi_brightness_para_type);
		break;
	}
	dpu_pr_debug("mipi_brightness_para_type:%d, bl_level is %u, bl_level_adjust[1]=0x%x",
		mipi_brightness_para_type, bl_level, bl_level_adjust[1]);
}

int dkmd_mipi_bl_set_backlight(struct panel_drv_private *priv, uint32_t value)
{
	struct dpu_connector *connector = NULL;
	char bl_level_adjust[3] = {
		 0x51,
		 0x00,
		 0x00
	};
	struct dsi_cmd_desc lcd_bl_level_adjust[] = {
		{DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US, sizeof(bl_level_adjust), bl_level_adjust},
	};

	if (unlikely(!priv)) {
		dpu_pr_warn("priv is null");
		return -1;
	}

	connector = get_primary_connector(&priv->connector_info);

	get_bl_level_adjust(priv->mipi_brightness_para_type, value, bl_level_adjust);

	composer_active_vsync(connector->conn_info, true);

	if (connector->bind_connector) {
		mipi_dual_dsi_cmds_tx(lcd_bl_level_adjust, ARRAY_SIZE(lcd_bl_level_adjust),
			connector->connector_base, lcd_bl_level_adjust, ARRAY_SIZE(lcd_bl_level_adjust),
			connector->bind_connector->connector_base, EN_DSI_TX_NORMAL_MODE, true);
	} else {
		mipi_dsi_cmds_tx(lcd_bl_level_adjust, ARRAY_SIZE(lcd_bl_level_adjust),
			connector->connector_base);
	}

	composer_active_vsync(connector->conn_info, false);
	dpu_pr_debug("[backlight] bl_level is %u, bl_level_adjust[1]=0x%x", value, bl_level_adjust[1]);
	return 0;
}