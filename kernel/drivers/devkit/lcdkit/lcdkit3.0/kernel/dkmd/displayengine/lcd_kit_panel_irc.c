/*
 * lcd_kit_panel_irc.c
 *
 * display engine dkmd IRC function in lcd
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 */

#include "lcd_kit_panel_irc.h"

#include "lcd_kit_adapt.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"

void lcd_kit_parse_ddic_irc(int panel_id, struct device_node *np)
{
	/* panel irc code */
	LCD_KIT_INFO("DISPLAY_ENGINE_DDIC_IRC parse panel xml data");
	lcd_kit_parse_u32(np, "lcd-kit,irc-support",
		&disp_info->panel_irc.support, 0);
	if (!disp_info->panel_irc.support)
			return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,irc-on-cmds",
		"lcd-kit,irc-on-cmds-state",
		&disp_info->panel_irc.on);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,irc-off-cmds",
		"lcd-kit,irc-off-cmds-state",
		&disp_info->panel_irc.off);
}

bool display_engine_set_param_irc(int panel_id, bool ddic_irc_enable)
{
	LCD_KIT_INFO("DISPLAY_ENGINE_DDIC_IRC %d for panel %d! +\n",
		ddic_irc_enable, panel_id);
	int ret = LCD_KIT_OK;
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return false;
	}
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("cannot get adapt_ops!\n");
		return false;
	}
	if (!disp_info->panel_irc.on.cmds) {
		LCD_KIT_DEBUG("panel_irc.on.cmds is NULL\n");
		return false;
	}
	if (!disp_info->panel_irc.off.cmds) {
		LCD_KIT_DEBUG("panel_irc.off.cmds is NULL\n");
		return false;
	}
	if (ddic_irc_enable) {
		ret = adapt_ops->mipi_tx(panel_id, NULL, &disp_info->panel_irc.on);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);
			return false;
		}
	} else {
		ret = adapt_ops->mipi_tx(panel_id, NULL, &disp_info->panel_irc.off);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);
			return false;
		}
	}
	LCD_KIT_INFO("DISPLAY_ENGINE_DDIC_IRC %d for panel %d! -\n",
		ddic_irc_enable, panel_id);
	return ret;
}
