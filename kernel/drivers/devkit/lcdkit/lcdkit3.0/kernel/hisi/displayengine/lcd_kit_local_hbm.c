/*
 * lcd_kit_local_hbm.c
 *
 * display engine LHBM function in lcd
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 */

#include "lcd_kit_local_hbm.h"
#include "lcd_kit_parse.h"

#define ALPHA_DEFAULT 0xFFF

static const int color_cmd_index = 1;
static const int alpha_cmd_index = 1;
static const int dbv_cmd_index = 2;
static const int alpha_dbv_index = 1;
static const int payload_index_min = 1;
static const int payload_index_max = 3;
static const uint32_t fps_medium = 90;
static const uint32_t fps_high = 120;
static char before_value_90hz;
static char before_value_120hz;
static char before_ext_90hz;
static char before_ext_120hz;

enum color_cmds_index_c08 {
	CMDS_RED_BYTE_C08 = 1,
	CMDS_GREEN_BYTE_C08 = 2,
	CMDS_BLUE_BYTE_C08 = 3,
	CMDS_COLOR_BYTE_MAX_C08 = 4
};

enum local_hbm_ddic_type {
	DDIC_DEFAULT = 0,
	DDIC_TYPE1 = 1, // c08
	DDIC_TYPE2 = 2, // f01
	DDIC_MAX = 3
};

// A1 120HZ, A2 90HZ
enum color_cmds_index_f01 {
	CMDS_RED_HIGH_A1_F01 = 1,
	CMDS_RED_LOW_A1_F01 = 2,
	CMDS_GREEN_HIGH_A1_F01 = 3,
	CMDS_GREEN_LOW_A1_F01 = 4,
	CMDS_BLUE_HIGH_A1_F01 = 5,
	CMDS_BLUE_LOW_A1_F01 = 6,
	CMDS_RED_HIGH_A2_F01 = 7,
	CMDS_RED_LOW_A2_F01 = 8,
	CMDS_GREEN_HIGH_A2_F01 = 9,
	CMDS_GREEN_LOW_A2_F01 = 10,
	CMDS_BLUE_HIGH_A2_F01 = 11,
	CMDS_BLUE_LOW_A2_F01 = 12,
	CMDS_COLOR_MAX_F01 = 13
};

enum hbm_gamma_index_f01 {
	GAMMA_INDEX_RED_HIGH_F01 = 0,
	GAMMA_INDEX_RED_LOW_F01 = 1,
	GAMMA_INDEX_GREEN_HIGH_F01 = 50,
	GAMMA_INDEX_GREEN_LOW_F01 = 51,
	GAMMA_INDEX_BLUE_HIGH_F01 = 100,
	GAMMA_INDEX_BLUE_LOW_F01 = 101
};

struct gamma_linear_interpolation_info {
	uint32_t grayscale_before;
	uint32_t grayscale;
	uint32_t grayscale_after;
	uint32_t gamma_node_value_before;
	uint32_t gamma_node_value;
	uint32_t gamma_node_value_after;
};

int display_engine_gamma_code_print_for_c08(int panel_id)
{
	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	if ((int)common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen <
		CMDS_COLOR_BYTE_MAX_C08) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("circle_color_cmds.cmds[1].payload[1]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_BYTE_C08]);
	LCD_KIT_INFO("circle_color_cmds.cmds[1].payload[2]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_BYTE_C08]);
	LCD_KIT_INFO("circle_color_cmds.cmds[1].payload[3]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_BYTE_C08]);
	return LCD_KIT_OK;
}

int display_engine_circle_code_print_for_c08(int panel_id)
{
	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.enter_circle_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}
	/* enter_circle_cmds.cmds[dbv_cmd_index].dlen mum value is 3 */
	if ((int)common_info->hbm.enter_circle_cmds.cmds[dbv_cmd_index].dlen < 3) {
		LCD_KIT_ERR("array index out of range, cmds[dbv].dlen:%d\n",
			common_info->hbm.enter_circle_cmds.cmds[dbv_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}
	/* enter_circle_cmds.cmds[alpha_cmd_index].dlen mum value is 4 */
	if ((int)common_info->hbm.enter_circle_cmds.cmds[alpha_cmd_index].dlen < 4) {
		LCD_KIT_ERR("array index out of range, cmds[alpha].dlen:%d\n",
			common_info->hbm.enter_circle_cmds.cmds[alpha_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}
	/* enter or exit circle_cmds.cmds[dbv_index].payload[1-2] value is dbv value */
	LCD_KIT_INFO("enter_circle_cmds.cmds[%d].payload[1]=%d\n", dbv_cmd_index,
		common_info->hbm.enter_circle_cmds.cmds[dbv_cmd_index].payload[1]);
	LCD_KIT_INFO("enter_circle_cmds.cmds[%d].payload[2]=%d\n", dbv_cmd_index,
		common_info->hbm.enter_circle_cmds.cmds[dbv_cmd_index].payload[2]);
	LCD_KIT_INFO("exit_circle_cmds.cmds[%d].payload[1]=%d\n", dbv_cmd_index,
		common_info->hbm.exit_circle_cmds.cmds[dbv_cmd_index].payload[1]);
	LCD_KIT_INFO("exit_circle_cmds.cmds[%d].payload[2]=%d\n", dbv_cmd_index,
		common_info->hbm.exit_circle_cmds.cmds[dbv_cmd_index].payload[2]);
	/* enter or exit circle_cmds.cmds[alpha_index].payload[1-3] value is alpha value */
	LCD_KIT_INFO("enter_circle_cmds.cmds[%d].payload[1]=%d\n", alpha_cmd_index,
		common_info->hbm.enter_circle_cmds.cmds[alpha_cmd_index].payload[1]);
	LCD_KIT_INFO("enter_circle_cmds.cmds[%d].payload[2]=%d\n", alpha_cmd_index,
		common_info->hbm.enter_circle_cmds.cmds[alpha_cmd_index].payload[2]);
	LCD_KIT_INFO("enter_circle_cmds.cmds[%d].payload[3]=%d\n", alpha_cmd_index,
		common_info->hbm.enter_circle_cmds.cmds[alpha_cmd_index].payload[3]);
	LCD_KIT_INFO("exit_circle_cmds.cmds[%d].payload[1]=%d\n", alpha_cmd_index,
		common_info->hbm.exit_circle_cmds.cmds[alpha_cmd_index].payload[1]);
	LCD_KIT_INFO("exit_circle_cmds.cmds[%d].payload[2]=%d\n", alpha_cmd_index,
		common_info->hbm.exit_circle_cmds.cmds[alpha_cmd_index].payload[2]);
	LCD_KIT_INFO("exit_circle_cmds.cmds[%d].payload[3]=%d\n", alpha_cmd_index,
		common_info->hbm.exit_circle_cmds.cmds[alpha_cmd_index].payload[3]);
	return LCD_KIT_OK;
}

int display_engine_set_grayscale_to_common_info_for_c08(int panel_id, int grayscale)
{
	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	if ((int)common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen <
		CMDS_COLOR_BYTE_MAX_C08) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("lcdkit grayscale = %d\n", grayscale);
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_BYTE_C08] = (char)grayscale;
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_BYTE_C08] = (char)grayscale;
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_BYTE_C08] = (char)grayscale;
	return LCD_KIT_OK;
}

int display_engine_local_hbm_set_dbv_for_c08(int panel_id, uint32_t dbv, bool is_on)
{
	LCD_KIT_INFO("set dbv, dbv value:%d\n", dbv);
	if (common_info->hbm.enter_circle_cmds.cmds == NULL ||
		common_info->hbm.exit_circle_cmds.cmds == NULL) {
		LCD_KIT_ERR("enter_circle_cmds.cmds or exit_circle_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* enter or exit circle_cmds.cmds[dbv_cmd_index].dlen mum value is 3 */
	if ((int)common_info->hbm.enter_circle_cmds.cmds[dbv_cmd_index].dlen < 3 ||
		(int)common_info->hbm.exit_circle_cmds.cmds[dbv_cmd_index].dlen < 3) {
		LCD_KIT_ERR("array index out of range, enter cmds[dbv_cmd_index].dlen:%d, \
			exit cmds[dbv_cmd_index].dlen:%d.\n",
			common_info->hbm.enter_circle_cmds.cmds[dbv_cmd_index].dlen,
			common_info->hbm.exit_circle_cmds.cmds[dbv_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}

	/* Sets the upper eight bits of the dbv, 1 is dbv payload */
	display_engine_circle_cmds_set_payload(
		panel_id, dbv_cmd_index, 1, (dbv >> 8) & 0x0f, is_on);
	/* Sets the lower eight bits of the dbv, 2 is dbv payload */
	display_engine_circle_cmds_set_payload(panel_id, dbv_cmd_index, 2, dbv & 0xff, is_on);
	LCD_KIT_INFO("set is_on=%d, dbv value=%d\n", is_on, dbv);
	LCD_KIT_INFO("enter.dbv.payload[1]=%d, enter.dbv.payload[2]=%d. \
		exit.dbv.payload[1]=%d, exit.dbv.payload[2]=%d.\n",
		common_info->hbm.enter_circle_cmds.cmds[dbv_cmd_index].payload[1],
		common_info->hbm.enter_circle_cmds.cmds[dbv_cmd_index].payload[2],
		common_info->hbm.exit_circle_cmds.cmds[dbv_cmd_index].payload[1],
		common_info->hbm.exit_circle_cmds.cmds[dbv_cmd_index].payload[2]);
	return LCD_KIT_OK;
}

int display_engine_ddic_alpha_is_on_for_c08(int panel_id, void *hld, uint32_t alpha, bool is_on)
{
	int payload_num;
	int enter_cmd_cnt;
	int exit_cmd_cnt;
	if (common_info->hbm.enter_circle_cmds.cmds == NULL ||
		common_info->hbm.exit_circle_cmds.cmds == NULL) {
		LCD_KIT_ERR("enter or exit circle_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	enter_cmd_cnt = common_info->hbm.enter_circle_cmds.cmd_cnt;
	exit_cmd_cnt = common_info->hbm.exit_circle_cmds.cmd_cnt;
	/* enter or exit cmd_cnt mum value is 3 */
	if (enter_cmd_cnt <= 0 || enter_cmd_cnt > 3 || exit_cmd_cnt <= 0 || exit_cmd_cnt > 3) {
		LCD_KIT_ERR("enter or exit circle_cmds's, enter_cmd_cnt:%d,exit_cmd_cnt:%d.\n",
		enter_cmd_cnt, exit_cmd_cnt);
		return LCD_KIT_FAIL;
	}
	payload_num = (int)common_info->hbm.enter_circle_cmds.cmds[alpha_cmd_index].dlen;
	LCD_KIT_INFO("enter_cmd_cnt = %d, payload_num = %d\n", enter_cmd_cnt, payload_num);
	/* Sets the upper eight bits of the alpha. 5-11 bit is hight eight bits */
	display_engine_circle_cmds_set_payload(
		panel_id, alpha_cmd_index, payload_num -2, (alpha >> 5) & 0xff, is_on);
	/* Sets the lower eight bits of the alpha. 0-4 bit is low eight bits */
	display_engine_circle_cmds_set_payload(
		panel_id, alpha_cmd_index, payload_num -1, alpha & 0x1f, is_on);
	LCD_KIT_INFO("alpha_cmd_index[%d], enter payload[2]=%d.payload[3]=%d. \
		exit payload[2]=%d.payload[3]=%d\n", alpha_cmd_index,
		common_info->hbm.enter_circle_cmds.cmds[alpha_cmd_index].payload[payload_num - 2],
		common_info->hbm.enter_circle_cmds.cmds[alpha_cmd_index].payload[payload_num - 1],
		common_info->hbm.exit_circle_cmds.cmds[alpha_cmd_index].payload[payload_num - 2],
		common_info->hbm.exit_circle_cmds.cmds[alpha_cmd_index].payload[payload_num - 1]);
	return LCD_KIT_OK;
}

int display_engine_ddic_alpha_is_on_ctrl(int panel_id, void *hld, uint32_t alpha, bool is_on)
{
	if (!display_engine_local_hbm_ddic_valid(panel_id))
		return LCD_KIT_FAIL;
	if (common_info->hbm.local_hbm_ddic == DDIC_TYPE1)
		return display_engine_ddic_alpha_is_on_for_c08(panel_id, hld, alpha, is_on);
	else if (common_info->hbm.local_hbm_ddic == DDIC_TYPE2)
		return display_engine_ddic_alpha_is_on_for_f01(panel_id, hld, alpha, is_on);
	else
		return LCD_KIT_OK;
}

int display_engine_set_grayscale_to_common_info_ctrl(int panel_id, void *hld, int grayscale)
{
	int ret;
	if (!display_engine_local_hbm_ddic_valid(panel_id))
		return LCD_KIT_FAIL;
	if (common_info->hbm.local_hbm_ddic == DDIC_TYPE1) {
		ret = display_engine_set_grayscale_to_common_info_for_c08(panel_id, grayscale);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_ERR("display_engine_set_grayscale_to_common_info_for_c08 error\n");
			return LCD_KIT_FAIL;
		}
		ret = display_engine_gamma_code_print_for_c08(panel_id);
	} else if (common_info->hbm.local_hbm_ddic == DDIC_TYPE2) {
		ret = display_engine_gamma_code_print_for_f01(panel_id);
	} else {
		return LCD_KIT_OK;
	}
	return ret;
}

int display_engine_local_hbm_set_dbv_ctrl(int panel_id, uint32_t dbv,
	uint32_t dbv_threshold, bool is_on)
{
	if (!display_engine_local_hbm_ddic_valid(panel_id))
		return LCD_KIT_FAIL;
	if (common_info->hbm.local_hbm_ddic == DDIC_TYPE1)
		return display_engine_local_hbm_set_dbv_for_c08(panel_id, dbv, is_on);
	else if (common_info->hbm.local_hbm_ddic == DDIC_TYPE2)
		return display_engine_local_hbm_set_dbv_for_f01(panel_id, dbv, dbv_threshold, is_on);
	else
		return LCD_KIT_OK;
}

static int display_engine_circle_code_print_for_f01(int panel_id)
{
	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.enter_circle_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("enter_circle_cmds.cmds[1].payload[0]=%2X\n",
		common_info->hbm.enter_circle_cmds.cmds[1].payload[0]);
	LCD_KIT_INFO("enter_circle_cmds.cmds[1].payload[1]=%2X,"
		"payload[2]=%2X,payload[3]=%2X,payload[4]=%2X,\n",
		common_info->hbm.enter_circle_cmds.cmds[1].payload[1],
		common_info->hbm.enter_circle_cmds.cmds[1].payload[2],
		common_info->hbm.enter_circle_cmds.cmds[1].payload[3],
		common_info->hbm.enter_circle_cmds.cmds[1].payload[4]);
	LCD_KIT_INFO("exit_circle_cmds.cmds[1].payload[0]=%d\n",
		common_info->hbm.exit_circle_cmds.cmds[1].payload[0]);
	return LCD_KIT_OK;
}

int display_engine_circle_code_print_ctrl(int panel_id)
{
	if (!display_engine_local_hbm_ddic_valid(panel_id))
		return LCD_KIT_FAIL;
	if (common_info->hbm.local_hbm_ddic == DDIC_TYPE1)
		return display_engine_circle_code_print_for_c08(panel_id);
	else if (common_info->hbm.local_hbm_ddic == DDIC_TYPE2)
		return display_engine_circle_code_print_for_f01(panel_id);
	else
		return LCD_KIT_OK;
}

static void lcd_kit_parse_local_hbm_ddic(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,local-hbm-ddic",
		&common_info->hbm.local_hbm_ddic, 0);
}

static void lcd_kit_parse_local_hbm_90hz_gamma_read_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-90-hz-gamma-read-cmds",
		"lcd-kit,hbm-90-hz-gamma-read-cmds-state",
		&common_info->hbm_ext.hbm_90_hz_gamma_read_cmds);
}

static void lcd_kit_parse_local_hbm_120hz_gamma_read_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-120-hz-gamma-read-cmds",
		"lcd-kit,hbm-120-hz-gamma-read-cmds-state",
		&common_info->hbm_ext.hbm_120_hz_gamma_read_cmds);
}

static void lcd_kit_parse_local_hbm_120hz_gamma_read_ext_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-120-hz-gamma-read-ext-cmds",
		"lcd-kit,hbm-120-hz-gamma-read-cmds-state",
		&common_info->hbm_ext.hbm_120_hz_gamma_read_ext_cmds);
}

static void lcd_kit_parse_local_hbm_90hz_gamma_read_ext_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-90-hz-gamma-read-ext-cmds",
		"lcd-kit,hbm-90-hz-gamma-read-cmds-state",
		&common_info->hbm_ext.hbm_90_hz_gamma_read_ext_cmds);
}

static void lcd_kit_parse_local_hbm_node_grayscale(int panel_id, struct device_node *np)
{
	lcd_kit_parse_array_data(np, "lcd-kit,node-grayscale",
		&common_info->hbm_ext.gamma_info.node_grayscale);
}

void lcd_kit_local_hbm_cmds_parse(int panel_id, struct device_node *np)
{
	if (!common_info->hbm.local_hbm_support) {
		LCD_KIT_INFO("platform not support local hbm\n");
		return;
	}
	lcd_kit_parse_local_hbm_ddic(panel_id, np);
	if (common_info->hbm.local_hbm_ddic == DDIC_TYPE2) {
		lcd_kit_parse_local_hbm_90hz_gamma_read_cmds(panel_id, np);
		lcd_kit_parse_local_hbm_120hz_gamma_read_cmds(panel_id, np);
		lcd_kit_parse_local_hbm_120hz_gamma_read_ext_cmds(panel_id, np);
		lcd_kit_parse_local_hbm_90hz_gamma_read_ext_cmds(panel_id, np);
		lcd_kit_parse_local_hbm_node_grayscale(panel_id, np);
	}
}

bool display_engine_local_hbm_ddic_valid(int panel_id)
{
	int ddic_type;
	if (!common_info->hbm.local_hbm_support) {
		LCD_KIT_INFO("platform not support local hbm\n");
		return false;
	}
	ddic_type = common_info->hbm.local_hbm_ddic;
	if (ddic_type > DDIC_MAX || ddic_type <= 0) {
		LCD_KIT_INFO("local hbm not support this ddic\n");
		return false;
	}
	return true;
}

void display_engine_circle_cmds_set_payload(
	int panel_id, int index, int payload_num, char value, bool is_on)
{
	if (index > payload_index_max || index < payload_index_min) {
		LCD_KIT_ERR("circle cmds set payload index out\n");
		return;
	}
	if (!is_on) {
		common_info->hbm.exit_circle_cmds.cmds[index].payload[payload_num] = value;
	} else {
		common_info->hbm.enter_circle_cmds.cmds[index].payload[payload_num] = value;
	}
}

bool display_engine_local_hbm_is_gamma_read(int panel_id)
{
	LCD_KIT_INFO("is_gamma_read local_hbm_ddic=%u.\n", common_info->hbm.local_hbm_ddic);
	if (common_info->hbm.local_hbm_ddic == DDIC_TYPE1)
		return false;
	else
		return true;
}

int display_engine_copy_color_cmds_to_common_info_for_f01(int fps,
	struct color_cmds_rgb *color_cmds)
{
	int panel_id = get_panel_id();
	if (!color_cmds) {
		LCD_KIT_ERR("color_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* circle_color_cmds.cmd_cnt's maximum value is 2 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 2) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}
	if ((int)common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen <
		CMDS_COLOR_MAX_F01) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.
			cmds[color_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}
	if (fps == fps_medium) {
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_RED_HIGH_A2_F01] = (char)color_cmds->red_payload[0];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_RED_LOW_A2_F01] = (char)color_cmds->red_payload[1];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_GREEN_HIGH_A2_F01] = (char)color_cmds->green_payload[0];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_GREEN_LOW_A2_F01] = (char)color_cmds->green_payload[1];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_BLUE_HIGH_A2_F01] = (char)color_cmds->blue_payload[0];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_BLUE_LOW_A2_F01] = (char)color_cmds->blue_payload[1];
	} else if (fps == fps_high) {
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_RED_HIGH_A1_F01] = (char)color_cmds->red_payload[0];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_RED_LOW_A1_F01] = (char)color_cmds->red_payload[1];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_GREEN_HIGH_A1_F01] = (char)color_cmds->green_payload[0];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_GREEN_LOW_A1_F01] = (char)color_cmds->green_payload[1];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_BLUE_HIGH_A1_F01] = (char)color_cmds->blue_payload[0];
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
			payload[CMDS_BLUE_LOW_A1_F01] = (char)color_cmds->blue_payload[1];
	} else {
		LCD_KIT_INFO("color_cmds other fps\n");
	}
	return LCD_KIT_OK;
}

int display_engine_gamma_code_print_for_f01(int panel_id)
{
	if (!display_engine_local_hbm_ddic_valid(panel_id))
		return LCD_KIT_FAIL;
	/* circle_color_cmds.cmd_cnt's maximum value is 2 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 2) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	if ((int)common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen <
		CMDS_COLOR_MAX_F01) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.
			cmds[color_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("circle_color_cmds.cmds[color_cmd_index]"
		"A1 red high=%d,low=%d, green high=%d,low=%d, blue high=%d,low=%d,"
		"A2 red high=%d,low=%d, green high=%d,low=%d, blue high=%d,low=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_HIGH_A1_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_LOW_A1_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_HIGH_A1_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_LOW_A1_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_HIGH_A1_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_LOW_A1_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_HIGH_A2_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_LOW_A2_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_HIGH_A2_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_LOW_A2_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_HIGH_A2_F01],
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_LOW_A2_F01]);
	return LCD_KIT_OK;
}

int display_engine_ddic_alpha_is_on_for_f01(int panel_id, void *hld, uint32_t alpha, bool is_on)
{
	if (!hld) {
		LCD_KIT_ERR("hld is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!is_on)
		return LCD_KIT_OK;
	LCD_KIT_INFO("set alpha, alpha value:%u\n", alpha);
	if (alpha == ALPHA_DEFAULT)
		alpha++;
	LCD_KIT_DEBUG("set alpha, alpha++ value:%u\n", alpha);
	if (common_info->hbm.enter_circle_cmds.cmds == NULL ||
		common_info->hbm.exit_circle_cmds.cmds == NULL) {
		LCD_KIT_ERR("enter_circle_cmds.cmds or exit_circle_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* enter or exit circle_cmds.cmds[alpha_dbv_index].dlen mum value is 5 */
	if ((int)common_info->hbm.enter_circle_cmds.cmds[alpha_dbv_index].dlen < 5) {
		LCD_KIT_ERR("array index out of range, enter cmds[dbv_cmd_index].dlen:%d, \
			exit cmds[dbv_cmd_index].dlen:%d.\n",
			common_info->hbm.enter_circle_cmds.cmds[alpha_dbv_index].dlen);
		return LCD_KIT_FAIL;
	}

	/* Sets the upper eight bits of the dbv, 1 is dbv payload */
	display_engine_circle_cmds_set_payload(
		panel_id, alpha_dbv_index, 1, (alpha >> 8) & 0x1f, true);
	/* Sets the lower eight bits of the dbv, 2 is dbv payload */
	display_engine_circle_cmds_set_payload(panel_id, alpha_dbv_index, 2, alpha & 0xff, true);
	LCD_KIT_INFO("set is_on=%d, alpha value=%d\n", is_on, alpha);
	LCD_KIT_INFO("enter.alpha.payload[1]=%d, enter.alpha.payload[2]=%d.\n",
		common_info->hbm.enter_circle_cmds.cmds[alpha_dbv_index].payload[1],
		common_info->hbm.enter_circle_cmds.cmds[alpha_dbv_index].payload[2]);
	return LCD_KIT_OK;
}

int display_engine_local_hbm_set_dbv_for_f01(int panel_id, uint32_t dbv,
	uint32_t dbv_threshold, bool is_on)
{
	int payload_num;
	int enter_cmd_cnt;
	int exit_cmd_cnt;
	if (!is_on)
		return LCD_KIT_OK;
	LCD_KIT_INFO("set dbv, dbv value:%u, dbv_threshold:%u.\n", dbv, dbv_threshold);
	if (dbv < dbv_threshold)
		dbv = dbv_threshold;
	if (common_info->hbm.enter_circle_cmds.cmds == NULL ||
		common_info->hbm.exit_circle_cmds.cmds == NULL) {
		LCD_KIT_ERR("enter or exit circle_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	enter_cmd_cnt = common_info->hbm.enter_circle_cmds.cmd_cnt;
	exit_cmd_cnt = common_info->hbm.exit_circle_cmds.cmd_cnt;
	/* enter cmd_cnt mum value is 3 */
	if (enter_cmd_cnt <= 0 || enter_cmd_cnt > 3 || exit_cmd_cnt <= 0 || exit_cmd_cnt > 2) {
		LCD_KIT_ERR("enter or exit circle_cmds's, enter_cmd_cnt:%d,exit_cmd_cnt:%d.\n",
		enter_cmd_cnt, exit_cmd_cnt);
		return LCD_KIT_FAIL;
	}
	payload_num = (int)common_info->hbm.enter_circle_cmds.cmds[alpha_dbv_index].dlen;
	LCD_KIT_INFO("enter_cmd_cnt = %d, payload_num = %d\n", enter_cmd_cnt, payload_num);
	/* Sets the upper eight bits of the dbv. */
	display_engine_circle_cmds_set_payload(
		panel_id, alpha_dbv_index, payload_num -2, (dbv >> 8) & 0x0f, true);
	/* Sets the lower eight bits of the dbv. */
	display_engine_circle_cmds_set_payload(
		panel_id, alpha_dbv_index, payload_num -1, dbv & 0xff, true);
	LCD_KIT_INFO("dbv_index[%d], enter payload[3]=%d.payload[4]=%d.\n", alpha_dbv_index,
		common_info->hbm.enter_circle_cmds.cmds[alpha_dbv_index].payload[payload_num - 2],
		common_info->hbm.enter_circle_cmds.cmds[alpha_dbv_index].payload[payload_num - 1]);
	return LCD_KIT_OK;
}

void display_engine_local_hbm_mipi_rx_fps_cmd(
	void *hld, int fps, uint8_t *hbm_gamma, size_t rgb_size, bool is_ext)
{
	int panel_id = get_panel_id();
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return;
	}
	if (!common_info->hbm.local_hbm_support)
		LCD_KIT_ERR("local_hbm not support\n");

	if (fps == fps_medium) {
		if (is_ext) {
			if (adapt_ops->mipi_rx(panel_id, hld, hbm_gamma, rgb_size - 1,
				&common_info->hbm_ext.hbm_90_hz_gamma_read_ext_cmds)) {
				LCD_KIT_ERR("hbm 90 HZ gamma ext mipi_rx failed!\n");
				return;
			}
			return;
		}
		if (adapt_ops->mipi_rx(panel_id, hld, hbm_gamma, rgb_size - 1,
			&common_info->hbm_ext.hbm_90_hz_gamma_read_cmds)) {
			LCD_KIT_ERR("hbm 90 HZ gamma mipi_rx failed!\n");
			return;
		}
	} else if (fps == fps_high) {
		if (is_ext) {
			if (adapt_ops->mipi_rx(panel_id, hld, hbm_gamma, rgb_size - 1,
				&common_info->hbm_ext.hbm_120_hz_gamma_read_ext_cmds)) {
				LCD_KIT_ERR("hbm 90 HZ gamma ext mipi_rx failed!\n");
				return;
			}
			return;
		}
		if (adapt_ops->mipi_rx(panel_id, hld, hbm_gamma, rgb_size - 1,
			&common_info->hbm_ext.hbm_120_hz_gamma_read_cmds)) {
			LCD_KIT_ERR("hbm 120 HZ gamma mipi_rx failed!\n");
			return;
		}
	} else {
		LCD_KIT_INFO("not support other fps\n");
	}
}

static bool local_hbm_gamma_read_valid(int panel_id)
{
	int cmd_cnt = 0;
	if (!display_engine_local_hbm_ddic_valid(panel_id))
		return false;
	cmd_cnt = common_info->hbm_ext.hbm_90_hz_gamma_read_cmds.cmd_cnt;
	cmd_cnt &= common_info->hbm_ext.hbm_90_hz_gamma_read_ext_cmds.cmd_cnt;
	cmd_cnt &= common_info->hbm_ext.hbm_120_hz_gamma_read_cmds.cmd_cnt;
	cmd_cnt &= common_info->hbm_ext.hbm_120_hz_gamma_read_ext_cmds.cmd_cnt;
	LCD_KIT_INFO("hbm_90_hz_gamma_read_cmds.cmd_cnt=%d, read_ext_cmds.cmd_cnt=%d.\n",
		common_info->hbm_ext.hbm_90_hz_gamma_read_cmds.cmd_cnt,
		common_info->hbm_ext.hbm_90_hz_gamma_read_ext_cmds.cmd_cnt);
	LCD_KIT_INFO("hbm_120_hz_gamma_read_cmds.cmd_cnt=%d, read_ext_cmds.cmd_cnt=%d.\n",
		common_info->hbm_ext.hbm_120_hz_gamma_read_cmds.cmd_cnt,
		common_info->hbm_ext.hbm_120_hz_gamma_read_ext_cmds.cmd_cnt);
	if (cmd_cnt <= 0)
		return false;
	return true;
}

static void local_hbm_gamma_read_cmd_payload_set(int panel_id, int fps)
{
	// 2 is read gamma cmd index, 1 is addr
	if (fps == fps_medium) {
		LCD_KIT_INFO("90_hz_gamma_read_cmds.cmds[2].payload[0]=0x%02X\n",
			common_info->hbm_ext.hbm_90_hz_gamma_read_cmds.cmds[2].payload[0]);
		LCD_KIT_INFO("hbm_90_hz_gamma_read_ext_cmds.cmds[2].payload[0]=0x%02X\n",
			common_info->hbm_ext.hbm_90_hz_gamma_read_ext_cmds.cmds[2].payload[0]);
		common_info->hbm_ext.hbm_90_hz_gamma_read_cmds.cmds[2].payload[0]++;
		common_info->hbm_ext.hbm_90_hz_gamma_read_ext_cmds.cmds[2].payload[0]++;
	} else if (fps == fps_high) {
		LCD_KIT_INFO("120_hz_gamma_read_cmds.cmds[2].payload[0]=0x%02X\n",
			common_info->hbm_ext.hbm_120_hz_gamma_read_cmds.cmds[2].payload[0]);
		LCD_KIT_INFO("hbm_120_hz_gamma_read_ext_cmds.cmds[2].payload[0]=0x%02X\n",
			common_info->hbm_ext.hbm_120_hz_gamma_read_ext_cmds.cmds[2].payload[0]);
		common_info->hbm_ext.hbm_120_hz_gamma_read_cmds.cmds[2].payload[0]++;
		common_info->hbm_ext.hbm_120_hz_gamma_read_ext_cmds.cmds[2].payload[0]++;
	} else {
		LCD_KIT_INFO("other fps payload.\n");
	}
}

void display_engine_local_hbm_gamma_read_fps_for_f01(int panel_id, void *hld, int fps)
{
	static const size_t rgb_size = 16;
	static const size_t rgb_ext_size = 2;
	static const size_t rgb_sum_size = 150;
	uint8_t hbm_gamma[rgb_size];
	uint8_t hbm_gamma_ext[rgb_ext_size];
	uint8_t hbm_gamma_sum[rgb_sum_size];
	int cnt = 0;
	if (!local_hbm_gamma_read_valid(panel_id))
		return;
	// 2 is gamma read index
	before_value_90hz = common_info->hbm_ext.hbm_90_hz_gamma_read_cmds.cmds[2].payload[0];
	before_value_120hz = common_info->hbm_ext.hbm_120_hz_gamma_read_cmds.cmds[2].payload[0];
	before_ext_90hz = common_info->hbm_ext.hbm_90_hz_gamma_read_ext_cmds.cmds[2].payload[0];
	before_ext_120hz = common_info->hbm_ext.hbm_120_hz_gamma_read_ext_cmds.cmds[2].payload[0];
	// B0->BB  hex B-> dec 11
	for (int i = 0; i < 12; i++) { // 12 is read gamma B0 to BB addr
		if (i == 3 || i == 7 || i == 11) { // 3 7 11 is 255 grayscale register index
			display_engine_local_hbm_mipi_rx_fps_cmd(
				hld, fps, hbm_gamma_ext, rgb_ext_size, true);
		} else {
			display_engine_local_hbm_mipi_rx_fps_cmd(
				hld, fps, hbm_gamma, rgb_size, false);
		}
		for (int j = 0; j < 16; j++) { // 16 is read gamma 0h to Fh
			if ((i == 3 || i == 7 || i == 11) && j > 1)
				break;
			if (i == 3 || i == 7 || i == 11) { // 3 7 11 is 255 grayscale register index
				hbm_gamma_sum[cnt] = hbm_gamma_ext[j];
			} else {
				hbm_gamma_sum[cnt] = hbm_gamma[j];
			}
			LCD_KIT_DEBUG("hbm_gamma_sum[%d]=%d\n", cnt, (int)(hbm_gamma_sum[cnt]));
			cnt++;
		}
		local_hbm_gamma_read_cmd_payload_set(panel_id, fps);
	}
	// 2 is gamma read index
	common_info->hbm_ext.hbm_90_hz_gamma_read_cmds.cmds[2].payload[0] = before_value_90hz;
	common_info->hbm_ext.hbm_120_hz_gamma_read_cmds.cmds[2].payload[0] = before_value_120hz;
	common_info->hbm_ext.hbm_90_hz_gamma_read_ext_cmds.cmds[2].payload[0] = before_ext_90hz;
	common_info->hbm_ext.hbm_120_hz_gamma_read_ext_cmds.cmds[2].payload[0] = before_ext_120hz;
	if (display_engine_set_hbm_gamma_to_common_info_fps_for_f01(
		panel_id, fps, hbm_gamma_sum, rgb_sum_size)) {
		LCD_KIT_ERR("display_engine_set_hbm_gamma_to_common_info_fps_for_f01 error\n");
		return;
	}
}

static int display_engine_get_grayscale_index(int grayscale,
	const struct gamma_node_info_ext *gamma_info, int *index)
{
	int i;
	int cnt;

	if (!gamma_info || !index) {
		LCD_KIT_ERR("NULL pointer\n");
		return LCD_KIT_FAIL;
	}
	cnt = gamma_info->node_grayscale.cnt;
	if (grayscale <= (int)gamma_info->node_grayscale.buf[0]) {
		*index = 1;
	} else if (grayscale >= (int)gamma_info->node_grayscale.buf[cnt - 1]) {
		*index = cnt - 1;
	} else {
		for (i = 0; i < cnt; i++) {
			LCD_KIT_INFO("grayscale[%d]:%d\n", i, gamma_info->node_grayscale.buf[i]);
			if (grayscale <= gamma_info->node_grayscale.buf[i]) {
				*index = i;
				break;
			}
		}
	}
	return LCD_KIT_OK;
}

static int display_engine_linear_interpolation_calculation_gamma(
	struct gamma_linear_interpolation_info *gamma_liner_info)
{
	int gamma_value = -1;

	if (!gamma_liner_info) {
		LCD_KIT_ERR("gamma_liner_info is NULL\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("grayscale(before:%d, self:%d, after:%d)\n",
		gamma_liner_info->grayscale_before, gamma_liner_info->grayscale,
		gamma_liner_info->grayscale_after);
	if ((gamma_liner_info->grayscale_before - gamma_liner_info->grayscale_after) == 0) {
		gamma_liner_info->gamma_node_value = gamma_liner_info->gamma_node_value_before;
	} else {
		/* Multiply by 100 to avoid loss of precision, plus 50 to complete rounding */
		gamma_value = (100 * ((int)gamma_liner_info->grayscale -
			(int)gamma_liner_info->grayscale_after) *
			((int)gamma_liner_info->gamma_node_value_before -
			(int)gamma_liner_info->gamma_node_value_after) /
			((int)gamma_liner_info->grayscale_before -
			(int)gamma_liner_info->grayscale_after)) +
			(100 * ((int)gamma_liner_info->gamma_node_value_after));
		gamma_liner_info->gamma_node_value = (uint32_t)(gamma_value + 50) / 100;
	}
	LCD_KIT_INFO("gamma_node_value(before:%d, self:%d,after:%d), gamma_value:%d\n",
		gamma_liner_info->gamma_node_value_before, gamma_liner_info->gamma_node_value,
		gamma_liner_info->gamma_node_value_after, gamma_value);
	return LCD_KIT_OK;
}

static int display_engine_set_gamma_liner_info(const struct gamma_node_info_ext *gamma_info,
	const uint32_t *gamma_value_array, size_t array_size,
	struct gamma_linear_interpolation_info *gamma_liner_info, int grayscale)
{
	int index;
	int ret;
	size_t i;

	if (!gamma_info || !gamma_liner_info || !gamma_value_array) {
		LCD_KIT_ERR("NULL pointer\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < array_size; i++)
		LCD_KIT_DEBUG("gamma_value_array[%d]:%d\n", i, gamma_value_array[i]);
	ret = display_engine_get_grayscale_index(grayscale, gamma_info, &index);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("display_engine_get_grayscale_index error\n");
		return LCD_KIT_FAIL;
	}
	if ((((uint32_t)index) >= array_size) || (index >= gamma_info->node_grayscale.cnt)) {
		LCD_KIT_ERR("index out of range, index:%d, node_gc.cnt:%d, gamma_array size:%d\n",
			index, gamma_info->node_grayscale.cnt, array_size);
		return LCD_KIT_FAIL;
	}
	gamma_liner_info->grayscale_before = gamma_info->node_grayscale.buf[index - 1];
	gamma_liner_info->grayscale = (uint32_t)grayscale;
	gamma_liner_info->grayscale_after = gamma_info->node_grayscale.buf[index];
	gamma_liner_info->gamma_node_value_before = gamma_value_array[index - 1];
	gamma_liner_info->gamma_node_value_after = gamma_value_array[index];
	LCD_KIT_INFO("index:%d, gamma_node_value_before:%d, gamma_node_value_after:%d\n",
		index, gamma_liner_info->gamma_node_value_before,
		gamma_liner_info->gamma_node_value_after);
	return ret;
}

static int display_engine_set_color_cmds_value_by_grayscale(
	const struct gamma_node_info_ext *gamma_info, const uint32_t *gamma_value_array,
	size_t array_size, int *payload, int grayscale)
{
	int ret = LCD_KIT_OK;
	struct gamma_linear_interpolation_info gamma_liner_info;

	if (!payload) {
		LCD_KIT_ERR("payload is NULL\n");
		return LCD_KIT_FAIL;
	}
	ret = display_engine_set_gamma_liner_info(gamma_info, gamma_value_array, array_size,
		&gamma_liner_info, grayscale);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("display_engine_set_gamma_liner_info error\n");
		return LCD_KIT_FAIL;
	}
	display_engine_linear_interpolation_calculation_gamma(&gamma_liner_info);

	/* High 8 bits correspond payload[0], low 8 bits correspond payload[1] */
	payload[0] = (gamma_liner_info.gamma_node_value >> 8) & 0xff;
	payload[1] = gamma_liner_info.gamma_node_value & 0xff;
	LCD_KIT_INFO("payload[1]:%d, payload[2]:%d\n", payload[0], payload[1]);
	return ret;
}

static int display_engine_set_color_cmds_by_grayscale(
	uint32_t fps, struct color_cmds_rgb *color_cmds,
	const struct gamma_node_info_ext *gamma_info, int grayscale)
{
	LCD_KIT_INFO("grayscale = %d\n", grayscale);
	if (!color_cmds) {
		LCD_KIT_ERR("color_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!gamma_info) {
		LCD_KIT_ERR("gamma_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (fps == fps_medium) {
		if (display_engine_set_color_cmds_value_by_grayscale(gamma_info,
			gamma_info->red_90_hz, HBM_GAMMA_NODE_SIZE_EXT,
			color_cmds->red_payload, grayscale) ||
			display_engine_set_color_cmds_value_by_grayscale(gamma_info,
				gamma_info->green_90_hz, HBM_GAMMA_NODE_SIZE_EXT,
				color_cmds->green_payload, grayscale) ||
			display_engine_set_color_cmds_value_by_grayscale(gamma_info,
				gamma_info->blue_90_hz, HBM_GAMMA_NODE_SIZE_EXT,
				color_cmds->blue_payload, grayscale)) {
			LCD_KIT_ERR("display_engine_set_color_cmds_value error\n");
			return LCD_KIT_FAIL;
		}
		display_engine_copy_color_cmds_to_common_info_for_f01(fps, color_cmds);
	} else if (fps == fps_high) {
		if (display_engine_set_color_cmds_value_by_grayscale(gamma_info,
		gamma_info->red_120_hz, HBM_GAMMA_NODE_SIZE_EXT,
		color_cmds->red_payload, grayscale) ||
		display_engine_set_color_cmds_value_by_grayscale(gamma_info,
			gamma_info->green_120_hz, HBM_GAMMA_NODE_SIZE_EXT,
			color_cmds->green_payload, grayscale) ||
		display_engine_set_color_cmds_value_by_grayscale(gamma_info,
			gamma_info->blue_120_hz, HBM_GAMMA_NODE_SIZE_EXT,
			color_cmds->blue_payload, grayscale)) {
		LCD_KIT_ERR("display_engine_set_color_cmds_value error\n");
		return LCD_KIT_FAIL;
	}
	display_engine_copy_color_cmds_to_common_info_for_f01(fps, color_cmds);
	} else {
		LCD_KIT_ERR("fps not support, fps:%d\n", fps);
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int display_engine_set_gamma_node_info_for_f01(int panel_id, int fps,
	struct gamma_node_info_ext *gamma_info)
{
	int i;
	int cnt;
	if (!gamma_info) {
		LCD_KIT_ERR("gamma_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	cnt = gamma_info->node_grayscale.cnt;
	/* 2 hbm_gamma_values are converted to 1 gamma_node_value. */
	if ((cnt > HBM_GAMMA_NODE_SIZE_EXT) || ((2 * cnt) > HBM_GAMMA_SIZE_EXT)) {
		LCD_KIT_ERR("node_grayscale.cnt out of range,"
			"gamma_info->node_grayscale.cnt:%d, HBM_GAMMA_NODE_SIZE_EXT:%d,"
			"HBM_GAMMA_SIZE_EXT:%d\n",
			cnt, HBM_GAMMA_NODE_SIZE_EXT, HBM_GAMMA_SIZE_EXT);
		return LCD_KIT_FAIL;
	}

	/* The high 8 bits of a hex number are converted into a dec number multiplied by 256 */
	if (fps == fps_medium) {
		for (i = 0; i < cnt; i++) {
			gamma_info->red_90_hz[i] =
				((common_info->hbm_ext.hbm_gamma.red_90_hz[i * 2] * 256) +
				common_info->hbm_ext.hbm_gamma.red_90_hz[(i * 2) + 1]) >> 2;
			gamma_info->green_90_hz[i] =
				((common_info->hbm_ext.hbm_gamma.green_90_hz[i * 2] * 256) +
				common_info->hbm_ext.hbm_gamma.green_90_hz[(i * 2) + 1]) >> 2;
			gamma_info->blue_90_hz[i] =
				((common_info->hbm_ext.hbm_gamma.blue_90_hz[i * 2] * 256) +
				common_info->hbm_ext.hbm_gamma.blue_90_hz[(i * 2) + 1]) >> 2;
		}
	} else if (fps == fps_high) {
		for (i = 0; i < cnt; i++) {
			gamma_info->red_120_hz[i] =
				((common_info->hbm_ext.hbm_gamma.red_120_hz[i * 2] * 256) +
				common_info->hbm_ext.hbm_gamma.red_120_hz[(i * 2) + 1]) >> 2;
			gamma_info->green_120_hz[i] =
				((common_info->hbm_ext.hbm_gamma.green_120_hz[i * 2] * 256) +
				common_info->hbm_ext.hbm_gamma.green_120_hz[(i * 2) + 1]) >> 2;
			gamma_info->blue_120_hz[i] =
				((common_info->hbm_ext.hbm_gamma.blue_120_hz[i * 2] * 256) +
				common_info->hbm_ext.hbm_gamma.blue_120_hz[(i * 2) + 1]) >> 2;
		}
	} else {
		LCD_KIT_INFO("other fps\n");
	}
	return LCD_KIT_OK;
}

int display_engine_set_hbm_gamma_to_common_info_fps_for_f01(int panel_id,
	int fps, uint8_t *hbm_gamma, size_t rgb_size)
{
	size_t i;

	if (!hbm_gamma) {
		LCD_KIT_ERR("hbm_gamma is NULL\n");
		return LCD_KIT_FAIL;
	}
	if ((GAMMA_INDEX_BLUE_HIGH_F01 + HBM_GAMMA_SIZE_EXT) > rgb_size) {
		LCD_KIT_ERR("hbm_gamma index out of range\n");
		return LCD_KIT_FAIL;
	}
	if (fps == fps_medium) {
		/* The 2 values correspond to each other */
		for (i = 0; i < (HBM_GAMMA_SIZE_EXT / 2); i++) {
			common_info->hbm_ext.hbm_gamma.red_90_hz[i * 2] =
				hbm_gamma[GAMMA_INDEX_RED_HIGH_F01 + (i * 2)] & 0x0f;
			common_info->hbm_ext.hbm_gamma.red_90_hz[(i * 2) + 1] =
				hbm_gamma[GAMMA_INDEX_RED_LOW_F01 + (i * 2)] & 0xff;
			common_info->hbm_ext.hbm_gamma.green_90_hz[i * 2] =
				hbm_gamma[GAMMA_INDEX_GREEN_HIGH_F01 + (i * 2)] & 0x0f;
			common_info->hbm_ext.hbm_gamma.green_90_hz[(i * 2) + 1] =
				hbm_gamma[GAMMA_INDEX_GREEN_LOW_F01 + (i * 2)] & 0xff;
			common_info->hbm_ext.hbm_gamma.blue_90_hz[i * 2] =
				hbm_gamma[GAMMA_INDEX_BLUE_HIGH_F01 + (i * 2)] & 0x0f;
			common_info->hbm_ext.hbm_gamma.blue_90_hz[(i * 2) + 1] =
				hbm_gamma[GAMMA_INDEX_BLUE_LOW_F01 + (i * 2)] & 0xff;
		}
	} else if (fps == fps_high) {
		/* The 2 values correspond to each other */
		for (i = 0; i < (HBM_GAMMA_SIZE_EXT / 2); i++) {
			common_info->hbm_ext.hbm_gamma.red_120_hz[i * 2] =
				hbm_gamma[GAMMA_INDEX_RED_HIGH_F01 + (i * 2)] & 0x0f;
			common_info->hbm_ext.hbm_gamma.red_120_hz[(i * 2) + 1] =
				hbm_gamma[GAMMA_INDEX_RED_LOW_F01 + (i * 2)] & 0xff;
			common_info->hbm_ext.hbm_gamma.green_120_hz[i * 2] =
				hbm_gamma[GAMMA_INDEX_GREEN_HIGH_F01 + (i * 2)] & 0x0f;
			common_info->hbm_ext.hbm_gamma.green_120_hz[(i * 2) + 1] =
				hbm_gamma[GAMMA_INDEX_GREEN_LOW_F01 + (i * 2)] & 0xff;
			common_info->hbm_ext.hbm_gamma.blue_120_hz[i * 2] =
				hbm_gamma[GAMMA_INDEX_BLUE_HIGH_F01 + (i * 2)] & 0x0f;
			common_info->hbm_ext.hbm_gamma.blue_120_hz[(i * 2) + 1] =
				hbm_gamma[GAMMA_INDEX_BLUE_LOW_F01 + (i * 2)] & 0xff;
		}
	} else {
		LCD_KIT_INFO("not support other fps\n");
	}
	for (i = 0; i < rgb_size; i++)
		LCD_KIT_DEBUG("hbm fps=%d, gamma %u = 0x%02X\n", fps, i, hbm_gamma[i]);
	return LCD_KIT_OK;
}

void display_engine_local_hbm_gamma_read_ctrl(int panel_id, void *hld)
{
	// 90HZ
	display_engine_local_hbm_gamma_read_fps_for_f01(panel_id, hld, fps_medium);
	// 120HZ
	display_engine_local_hbm_gamma_read_fps_for_f01(panel_id, hld, fps_high);

	display_engine_set_gamma_node_info_for_f01(panel_id, fps_medium,
		&common_info->hbm_ext.gamma_info);
	display_engine_set_gamma_node_info_for_f01(panel_id, fps_high,
		&common_info->hbm_ext.gamma_info);
}

int display_engine_local_hbm_set_color_by_grayscale_ctrl(int panel_id, void *hld, int grayscale)
{
	int ret;
	struct color_cmds_rgb color_cmds;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	if (display_engine_local_hbm_is_gamma_read(panel_id)) {
		ret = display_engine_set_color_cmds_by_grayscale(fps_high, &color_cmds,
			&common_info->hbm_ext.gamma_info, grayscale);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_ERR("display_engine_set_color_cmds error\n");
			return LCD_KIT_FAIL;
		}
		ret = display_engine_set_color_cmds_by_grayscale(fps_medium, &color_cmds,
			&common_info->hbm_ext.gamma_info, grayscale);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_ERR("display_engine_set_color_cmds error\n");
			return LCD_KIT_FAIL;
		}
	}

	ret = display_engine_set_grayscale_to_common_info_ctrl(panel_id, hld, grayscale);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("display_engine_gamma_code_print error\n");
		return LCD_KIT_FAIL;
	}

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, hld, &(common_info->hbm.circle_color_cmds));
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx faild! ret = %d\n", ret);
	return ret;
}
