/*
 * lcd_kit_panel.h
 *
 * lcdkit panel function for lcd driver head file
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

#ifndef _LCD_KIT_PANEL_H_
#define _LCD_KIT_PANEL_H_
#include "lcd_kit_utils.h"

/*
 * panel compatible
 */
#define PANEL_190_H01_13P2 "190_h01_13p2"
#define PANEL_310_H01_13P2 "310_h01_13p2"
#define PANEL_190_H01_13P2_V3 "190_h01_1_13p2"
#define PANEL_310_H01_13P2_V3 "310_h01_1_13p2"
#define PANEL_350_H01_12P2 "350_h01_12p2"

/*
 * struct
 */
struct lcd_kit_panel_ops {
	bool (*lcd_kit_need_update_fw)(uint32_t panel_id);
	int (*lcd_kit_update_fw)(uint32_t panel_id);
};

struct lcd_kit_panel_map {
	char *compatible;
	int (*callback)(struct platform_device *pdev);
};

/*
 * function declare
 */
struct lcd_kit_panel_ops *lcd_kit_panel_get_ops(void);
int lcd_kit_panel_init(uint32_t panel_id, struct platform_device *pdev);
int lcd_kit_panel_ops_register(struct lcd_kit_panel_ops *ops);
int lcd_kit_panel_ops_unregister(struct lcd_kit_panel_ops *ops);
#endif
