/*
 * lcd_kit_panel_pfm.c
 *
 * display engine dkmd PFM function in lcd, low brightness to close PFM
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 */
#include "lcd_kit_panel_pfm.h"

#include "lcd_kit_adapt.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"

static unsigned int last_bl_level = 0;
static bool pfm_on_flag = false; /* default close panel PFM */

static void pfm_mode_cmd_send_no_lock(int panel_id, struct lcd_kit_dsi_panel_cmds *cmds)
{
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return;
	}
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx_no_lock) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx_no_lock!\n");
		return;
	}
	int ret = adapt_ops->mipi_tx_no_lock(panel_id, NULL, cmds);
	if (ret)
		LCD_KIT_ERR("mipi_tx_no_lock failed!\n");
}

static void pfm_mode_cmd_send(int panel_id, struct lcd_kit_dsi_panel_cmds *cmds)
{
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return;
	}
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return;
	}
	int ret = adapt_ops->mipi_tx(panel_id, NULL, cmds);
	if (ret)
		LCD_KIT_ERR("mipi_tx failed!\n");
}

void lcd_kit_parse_panel_pfm(int panel_id, struct device_node *np)
{
	LCD_KIT_INFO("DISPLAY_ENGINE_DDIC_PFM parse panel xml data");
	lcd_kit_parse_u32(np, "lcd-kit,panel-pfm-support", &disp_info->panel_pfm.support, 0);
	if (disp_info->panel_pfm.support) {
		lcd_kit_parse_u32(np, "lcd-kit,panel-pfm-dbv", &disp_info->panel_pfm.dbv, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-pfm-on-cmds",
			"lcd-kit,panel-pfm-on-cmds-state", &disp_info->panel_pfm.on);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-pfm-off-cmds",
			"lcd-kit,panel-pfm-off-cmds-state", &disp_info->panel_pfm.off);
	}
}

/* fps change invoke pfm */
void lcd_kit_fps_pfm_ctrl(int panel_id, unsigned int current_fps)
{
	if (!disp_info->panel_pfm.support)
		return;
	if (current_fps != FPS_60) {
		if (pfm_on_flag) {
			pfm_mode_cmd_send_no_lock(panel_id, &disp_info->panel_pfm.off);
			pfm_on_flag = false;
			LCD_KIT_INFO("pfs change check panel pfm off. on_flag [%d]\n", pfm_on_flag);
		}
		return;
	}
	int cur_backlight = last_bl_level;

	if (cur_backlight == 0) {
		/* default close panel PFM */
		pfm_on_flag = false;
	} else if (cur_backlight <= disp_info->panel_pfm.dbv && pfm_on_flag) {
		LCD_KIT_INFO("bl_level [%d] <= DBV [%d], panel pfm off\n", cur_backlight,
			disp_info->panel_pfm.dbv);
		pfm_mode_cmd_send_no_lock(panel_id, &disp_info->panel_pfm.off);
		pfm_on_flag = false;
	} else if (cur_backlight > disp_info->panel_pfm.dbv && !pfm_on_flag) {
		LCD_KIT_INFO("bl_level [%d] > DBV [%d], panel pfm on\n", cur_backlight,
			disp_info->panel_pfm.dbv);
		pfm_mode_cmd_send_no_lock(panel_id, &disp_info->panel_pfm.on);
		pfm_on_flag = true;
	}
}

/* brightness change invoke pfm */
void lcd_kit_bl_pfm_ctrl(int panel_id, unsigned int bl_level, unsigned int current_fps)
{
	if (!disp_info->panel_pfm.support)
		return;
	if (last_bl_level != bl_level)
		last_bl_level = bl_level;
	if (current_fps != FPS_60) {
		if (pfm_on_flag) {
			pfm_mode_cmd_send(panel_id, &disp_info->panel_pfm.off);
			pfm_on_flag = false;
			LCD_KIT_INFO("brightness change check panel pfm off. on_flag [%d]\n", pfm_on_flag);
		}
		return;
	}

	if (bl_level == 0) {
		/* default close panel PFM */
		pfm_on_flag = false;
	} else if (bl_level <= disp_info->panel_pfm.dbv && pfm_on_flag) {
		LCD_KIT_INFO("bl_level [%d] <= DBV [%d], panel pfm off\n",
			bl_level, disp_info->panel_pfm.dbv);
		pfm_mode_cmd_send(panel_id, &disp_info->panel_pfm.off);
		pfm_on_flag = false;
	} else if (bl_level > disp_info->panel_pfm.dbv && !pfm_on_flag) {
		LCD_KIT_INFO("bl_level [%d] > DBV [%d], panel pfm on\n",
			bl_level, disp_info->panel_pfm.dbv);
		pfm_mode_cmd_send(panel_id, &disp_info->panel_pfm.on);
		pfm_on_flag = true;
	}
}