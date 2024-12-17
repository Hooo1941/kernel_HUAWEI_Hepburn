/*
 * lcd_kit_panel_irc.c
 *
 * display engine dkmd IRC function in lcd
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 */

#include "lcd_kit_panel_od.h"

#include "lcd_kit_adapt.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"

void lcd_kit_parse_panel_od(int panel_id, struct device_node *np)
{
	/* panel irc code */
	LCD_KIT_INFO("DISPLAY_ENGINE_PNALE_OD parse panel xml data");
	lcd_kit_parse_u32(np, "lcd-kit,panel-od-support",
		&disp_info->panel_od.support, 0);
	if (!disp_info->panel_od.support)
			return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-od-on-cmds",
		"lcd-kit,panel-od-on-cmds-state",
		&disp_info->panel_od.on_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-od-off-cmds",
		"lcd-kit,panel-od-off-cmds-state",
		&disp_info->panel_od.off_cmds);
}

int display_engine_set_param_panel_od(int panel_id, bool panel_od_enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	LCD_KIT_INFO("DISPLAY_ENGINE_PNALE_OD %d for panel %d! +\n",
		panel_od_enable, panel_id);
	if (!disp_info->panel_od.support) {
		LCD_KIT_INFO("ap od ctrl not support\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("mipi tx not support!\n");
		return LCD_KIT_FAIL;
	}

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
	if (panel_od_enable) {
		LCD_KIT_INFO("panel od on cmds\n");
		ret = adapt_ops->mipi_tx(panel_id, NULL, &disp_info->panel_od.on_cmds);
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);
	} else {
		LCD_KIT_INFO("panel od off cmds\n");
		ret = adapt_ops->mipi_tx(panel_id, NULL, &disp_info->panel_od.off_cmds);
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);
	}
	LCD_KIT_INFO("DISPLAY_ENGINE_PNALE_OD %d for panel %d! -\n",
		panel_od_enable, panel_id);
	return ret;
}
