/*
 * lcd_kit_get_safe_frm_rate_by_bl.c
 *
 * display engine dkmd safe frm rate function in lcd
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
 *
 */

#include "lcd_kit_safe_frm_rate.h"

#include "lcd_kit_adapt.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"

#define FPS_90HZ 90
#define FPS_120HZ 120
#define FPS_144HZ 144

static bool low_brightness_flag = false;

/* get real safe frm rate by current fps */
static unsigned int lcd_kit_get_real_safe_frm_by_fps(int panel_id, unsigned int current_fps)
{
	if (!low_brightness_flag) {
		switch (current_fps) {
		case FPS_144HZ:
			return disp_info->safe_frm_decision.fps_144_safe_frm;
		case FPS_120HZ:
			return disp_info->safe_frm_decision.fps_120_safe_frm;
		case FPS_90HZ:
			return disp_info->safe_frm_decision.fps_90_safe_frm;
		default:
			return current_fps;
		}
	}

	return current_fps;
}

/* update safe frm rate by low brightness status and current fps */
static unsigned int lcd_kit_upt_safe_frm_rate_by_fps(int panel_id, unsigned int safe_frm_rate, unsigned int current_fps)
{
	if (safe_frm_rate != 0) {
		/* notify low brightness status change */
		if (safe_frm_rate == FPS_120HZ) {
			low_brightness_flag = true;
			LCD_KIT_INFO("notify low brightness scene enter\n");
		} else {
			low_brightness_flag = false;
			LCD_KIT_INFO("notify low brightness scene exit\n");
		}
	}
	return lcd_kit_get_real_safe_frm_by_fps(panel_id, current_fps);
}

/* update safe frm rate by tp */
static unsigned int lcd_kit_upt_safe_frm_rate_by_tp(unsigned int safe_frm_rate)
{
	return safe_frm_rate;
}

/* update final safe frm rate */
static unsigned int lcd_kit_upt_safe_frm_rate(int panel_id, const struct panel_update_safe_frm_rate_info *safe_frm_info)
{
	int ret;
	unsigned int dsi0_index;
	unsigned int connector_id;
	unsigned int final_safe_frm_rate;

	if (!safe_frm_info) {
		LCD_KIT_ERR("safe_frm_info is null\n");
		return 0;
	}

	final_safe_frm_rate = safe_frm_info->update_safe_frm_rate;
	/* update safe frame rate by TP */
	if (disp_info->safe_frm_decision.tp_support)
		final_safe_frm_rate = lcd_kit_upt_safe_frm_rate_by_tp(safe_frm_info->update_safe_frm_rate);

	/* update safe frame rate by FPS */
	if (disp_info->safe_frm_decision.fps_support)
		final_safe_frm_rate = lcd_kit_upt_safe_frm_rate_by_fps(panel_id,
			final_safe_frm_rate, safe_frm_info->update_frm_rate);

	LCD_KIT_INFO("lcdkit decision mode support tp:%u fps:%u, final safe frm rate:%u\n",
		disp_info->safe_frm_decision.tp_support, disp_info->safe_frm_decision.fps_support, final_safe_frm_rate);

	return final_safe_frm_rate;
}

/* send final safe frm rate */
static unsigned int lcd_kit_send_safe_frm_rate(int panel_id, unsigned int safe_frm_rate)
{
	int ret;
	unsigned int dsi0_index;
	unsigned int connector_id;

	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return LCD_KIT_FAIL;
	}
	dkmd_dfr_send_safe_frm_rate(dsi0_index, DPU_PINFO->type, safe_frm_rate);

	return LCD_KIT_OK;
}

/* safe frame rate second decision, time delay is not allowed in this function */
int lcd_kit_safe_frm_rate_ctrl(int panel_id, const struct panel_update_safe_frm_rate_info *safe_frm_info)
{
	int ret;
	unsigned int final_safe_frm_rate;

	final_safe_frm_rate = lcd_kit_upt_safe_frm_rate(panel_id, safe_frm_info);
	if (!final_safe_frm_rate) {
		LCD_KIT_ERR("update safe frm rate fail\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_send_safe_frm_rate(panel_id, final_safe_frm_rate);
	if (ret) {
		LCD_KIT_ERR("send safe frm rate fail\n");
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

/* parse safe frm rate decsion info */
void lcd_kit_parse_safe_frm_decision_info(int panel_id, struct device_node *np)
{
	LCD_KIT_INFO("DISPLAY_ENGINE_SAFE_FRM_INFO parse panel xml data");
	lcd_kit_parse_u32(np, "lcd-kit,safe-frm-decision-by-tp-support", &disp_info->safe_frm_decision.tp_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,safe-frm-decision-by-fps-support", &disp_info->safe_frm_decision.fps_support, 0);
	if (disp_info->safe_frm_decision.fps_support) {
		lcd_kit_parse_u32(np, "lcd-kit,fps-144-safe-frm", &disp_info->safe_frm_decision.fps_144_safe_frm, 0);
		lcd_kit_parse_u32(np, "lcd-kit,fps-120-safe-frm", &disp_info->safe_frm_decision.fps_120_safe_frm, 0);
		lcd_kit_parse_u32(np, "lcd-kit,fps-90-safe-frm", &disp_info->safe_frm_decision.fps_90_safe_frm, 0);
	}
}