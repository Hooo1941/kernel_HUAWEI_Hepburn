/*
 * lcd_kit_panel_temp_compensation.c
 *
 * display engine dkmd IRC function in lcd
 *
 * Copyright (c) 2023-2024 Huawei Technologies Co., Ltd.
 *
 */

#include "lcd_kit_panel_temper.h"
#include "lcd_kit_adapt.h"

int display_engine_set_param_panel_temper(int panel_id, bool panel_temper_enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->temper_thread.enable) {
		LCD_KIT_INFO("not support temper\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_INFO("DISPLAY_ENGINE_DDIC_TEMPER %d +\n", panel_temper_enable);
	common_info->temper_thread.dark_enable = panel_temper_enable;
	if (panel_temper_enable) {
		common_ops->start_temper_hrtimer(panel_id);
		LCD_KIT_INFO("temper timer is starting!\n");
	} else {
		common_ops->stop_temper_hrtimer(panel_id);
		LCD_KIT_INFO("temper timer is stoping!\n");
		if (common_info->temper_thread.temper_exit_cmd.cmds)
			ret = adapt_ops->mipi_tx(panel_id, NULL, &common_info->temper_thread.temper_exit_cmd);
		if (ret) {
			LCD_KIT_ERR("mipi_tx error\n");
			return LCD_KIT_FAIL;
		}
	}
	LCD_KIT_INFO("DISPLAY_ENGINE_DDIC_TEMPER %d -\n", panel_temper_enable);
	return LCD_KIT_OK;
}
