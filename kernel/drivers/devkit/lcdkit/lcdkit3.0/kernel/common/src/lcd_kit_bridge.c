/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: lcdkit parse function for lcdkit
* Create: 2023-06-13
*/

#include "lcd_kit_bridge.h"
#include "lcd_kit_common.h"

static struct lcd_kit_bridge_ops *g_bridge_ops;

int lcd_kit_bridge_register(struct lcd_kit_bridge_ops *ops)
{
	if (g_bridge_ops) {
		LCD_KIT_ERR("g_bridge_ops has already been registered!\n");
		return LCD_KIT_FAIL;
	}
	g_bridge_ops = ops;
	LCD_KIT_INFO("g_bridge_ops register success!\n");
	return LCD_KIT_OK;
}

int lcd_kit_bridge_unregister(struct lcd_kit_bridge_ops *ops)
{
	if (g_bridge_ops == ops) {
		g_bridge_ops = NULL;
		LCD_KIT_INFO("g_bridge_ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("g_bridge_ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_bridge_ops *lcd_kit_get_bridge_ops(void)
{
	return g_bridge_ops;
}
