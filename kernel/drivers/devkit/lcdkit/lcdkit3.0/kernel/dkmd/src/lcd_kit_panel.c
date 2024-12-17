/*
 * lcd_kit_panel.c
 *
 * lcdkit panel function for lcd driver
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_panel.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_common.h"
#include "lcd_kit_sysfs_dkmd.h"
#include "lcd_kit_adapt.h"
#include <securec.h>
#include <linux/firmware.h>
#include <linux/ktime.h>

#include "panel/h01_13p2.c"

static struct lcd_kit_panel_map panel_map[] = {
	{ PANEL_190_H01_13P2, h01_13p2_probe },
	{ PANEL_310_H01_13P2, h01_13p2_probe },
	{ PANEL_190_H01_13P2_V3, h01_13p2_probe },
	{ PANEL_310_H01_13P2_V3, h01_13p2_probe },
	{ PANEL_350_H01_12P2, h01_13p2_probe },
};

struct lcd_kit_panel_ops *g_lcd_kit_panel_ops[MAX_PANEL];

int lcd_kit_panel_ops_register(struct lcd_kit_panel_ops *ops)
{
	int panel_id = lcd_kit_get_active_panel_id();
	if (!g_lcd_kit_panel_ops[panel_id]) {
		g_lcd_kit_panel_ops[panel_id] = ops;
		LCD_KIT_INFO("ops register success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("ops have been registered!\n");
	return LCD_KIT_FAIL;
}
int lcd_kit_panel_ops_unregister(struct lcd_kit_panel_ops *ops)
{
	int panel_id = lcd_kit_get_active_panel_id();
	if (g_lcd_kit_panel_ops[panel_id] == ops) {
		g_lcd_kit_panel_ops[panel_id] = NULL;
		LCD_KIT_INFO("ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_panel_ops *lcd_kit_panel_get_ops(void)
{
	int panel_id = lcd_kit_get_active_panel_id();
	return g_lcd_kit_panel_ops[panel_id];
}

int lcd_kit_panel_init(uint32_t panel_id, struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;
	int i = 0;

	if (!disp_info->compatible) {
		LCD_KIT_ERR("compatible is null\n");
		return LCD_KIT_FAIL;
	}

	for (i = 0; i < ARRAY_SIZE(panel_map); i++) {
		if (!strncmp(disp_info->compatible, panel_map[i].compatible,
			strlen(disp_info->compatible))) {
			ret = panel_map[i].callback(pdev);
			if (ret) {
				LCD_KIT_ERR("ops init fail\n");
				return LCD_KIT_FAIL;
			}
			break;
		}
	}

	if (i >= ARRAY_SIZE(panel_map))
		LCD_KIT_INFO("not find adapter ops\n");
	return LCD_KIT_OK;
}
