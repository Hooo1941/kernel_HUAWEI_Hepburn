/*
 * lcd_kit_local_hbm.c
 *
 * display engine dkmd LHBM function in lcd
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 */

#include "lcd_kit_local_hbm.h"

#include "lcd_kit_adapt.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_disp.h"

#ifdef LCD_FINGERPRINT_ENABLE
#include "huawei_platform/fingerprint_interface/fingerprint_interface.h"
#endif

static const int alpha_cmd_index = 2;
static const int payload_index_min = 1;
static const int payload_index_max = 3;
static uint32_t display_engine_brightness_get_mipi_level(void);
static int display_engine_local_hbm_get_circle_cmd_index(int panel_id);
static bool display_engine_local_hbm_set_circle_cmds_valid(int panel_id);
static int display_engine_local_hbm_set_circle_by_index(int panel_id, void *hld,
	bool is_on, int index);
static int display_engine_local_hbm_set_circle_color_vendor1(int panel_id, int grayscale);
static int display_engine_local_hbm_set_circle_color_vendor2(int panel_id, int grayscale);
static int display_engine_local_hbm_set_circle_color_vendor3(int panel_id, int grayscale);
static int display_engine_local_hbm_set_circle_color_by_grayscale(int panel_id, void *hld,
	int grayscale);
static void display_engine_local_hbm_cmds_print(int panel_id);
static int display_engine_local_hbm_set_mcu(int panel_id, void *hld, bool is_on);
static int display_engine_local_hbm_change_gamma(int panel_id, void *hld);

#define BACKLIGHT_HIGH_LEVEL 1
#define BACKLIGHT_LOW_LEVEL  2
#define LHBM_ON_WAIT_VSYNC_COUNT 2
#define LHBM_SAFE_FRAMERATE_RM_DELAY_US 9000
#define LHBM_SAFE_FRAMERATE_XHS_DELAY_US 29000
#define HBM_THRESHOD 255
#define HBM_MAX 500

/* local hbm circle cmds index for different brightness level */
enum circle_cmd {
	CIRCLE_FIRST_CMD = 0,
	CIRCLE_SECOND_CMD = 1,
	CIRCLE_THIRD_CMD = 2,
	CIRCLE_CMD_NUM = 3,
};

enum local_hbm_ddic {
	DDIC_VENDOR1 = 0,
	DDIC_VENDOR2 = 1,
	DDIC_VENDOR3 = 2,
	DDIC_NUM = 3,
};

/* Force alpha enable with different state */
enum force_alpha_enable_state {
	NO_FORCING = -1,
	FORCE_DISABLE,
	FORCE_ENABLE
};

/* Displayengine local hbm related parameters */
struct de_context_local_hbm {
	unsigned int last_level_in;
	int fingerprint_backlight_type;
	unsigned short lum_lut[LHBM_ALPHA_LUT_LENGTH];
};

static struct de_context_local_hbm g_de_context = {
	.last_level_in = 0,
	.fingerprint_backlight_type = BACKLIGHT_LOW_LEVEL,
};

static bool display_engine_local_hbm_set_circle_cmds_valid(int panel_id)
{
	if (!common_info->hbm.circle_exit_first_cmds.cmds) {
		LCD_KIT_ERR("circle_exit_first_cmds.cmds is NULL\n");
		return false;
	}
	if (!common_info->hbm.circle_exit_second_cmds.cmds) {
		LCD_KIT_ERR("circle_exit_second_cmds.cmds is NULL\n");
		return false;
	}
	if (!common_info->hbm.circle_enter_first_cmds.cmds) {
		LCD_KIT_ERR("circle_enter_first_cmds.cmds is NULL\n");
		return false;
	}
	if (!common_info->hbm.circle_enter_second_cmds.cmds) {
		LCD_KIT_ERR("circle_enter_second_cmds.cmds is NULL\n");
		return false;
	}
	switch (common_info->hbm.local_hbm_ddic) {
	case DDIC_VENDOR1:
	case DDIC_VENDOR3:
		return true;
	case DDIC_VENDOR2:
		if (!common_info->hbm.circle_exit_third_cmds.cmds) {
			LCD_KIT_ERR("circle_exit_third_cmds.cmds is NULL\n");
			return false;
		}
		if (!common_info->hbm.circle_enter_third_cmds.cmds) {
			LCD_KIT_ERR("circle_enter_third_cmds.cmds is NULL\n");
			return false;
		}
		return true;
	default:
		break;
	}
	return false;
}

static int display_engine_local_hbm_set_circle_by_index(int panel_id, void *hld,
	bool is_on, int index)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	LCD_KIT_INFO("index: %d\n", index);
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("cannot get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!is_on) {
		switch (index) {
		case CIRCLE_FIRST_CMD:
			ret = adapt_ops->mipi_tx(panel_id, hld,
				&common_info->hbm.circle_exit_first_cmds);
			break;
		case CIRCLE_SECOND_CMD:
			ret = adapt_ops->mipi_tx(panel_id, hld,
				&common_info->hbm.circle_exit_second_cmds);
			break;
		case CIRCLE_THIRD_CMD:
			ret = adapt_ops->mipi_tx(panel_id, hld,
				&common_info->hbm.circle_exit_third_cmds);
			break;
		default:
			ret = LCD_KIT_FAIL;
			break;
		}
	} else {
		switch (index) {
		case CIRCLE_FIRST_CMD:
			ret = adapt_ops->mipi_tx(panel_id, hld,
				&common_info->hbm.circle_enter_first_cmds);
			break;
		case CIRCLE_SECOND_CMD:
			ret = adapt_ops->mipi_tx(panel_id, hld,
				&common_info->hbm.circle_enter_second_cmds);
			break;
		case CIRCLE_THIRD_CMD:
			ret = adapt_ops->mipi_tx(panel_id, hld,
				&common_info->hbm.circle_enter_third_cmds);
			break;
		default:
			ret = LCD_KIT_FAIL;
			break;
		}
	}
	return ret;
}

static int display_engine_local_hbm_set_circle_color_vendor1(int panel_id, int grayscale)
{
	/* circle_color_cmds.cmd_cnt's maximum value is 5 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 5) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	/* circle_color_cmds.cmds[r|g|b] index is [1|2|3] */
	/* circle_color_cmds.cmds[r|g|b].dlen maximum value is 2 */
	if ((int)common_info->hbm.circle_color_cmds.cmds[1].dlen < 2 ||
		(int)common_info->hbm.circle_color_cmds.cmds[2].dlen < 2 ||
		(int)common_info->hbm.circle_color_cmds.cmds[3].dlen < 2) {
		LCD_KIT_ERR("array index out of range, cmds.dlen\n");
		return LCD_KIT_FAIL;
	}

	/* circle_color_cmds.cmds[r|g|b].payload value index is 1 */
	common_info->hbm.circle_color_cmds.cmds[1].payload[1] = grayscale;
	common_info->hbm.circle_color_cmds.cmds[2].payload[1] = grayscale;
	common_info->hbm.circle_color_cmds.cmds[3].payload[1] = grayscale;
	return LCD_KIT_OK;
}

static int display_engine_local_hbm_set_circle_color_vendor2(int panel_id, int grayscale)
{
	/* circle_color_cmds.cmd_cnt's maximum value is 7 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 7) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match, cnt: \n",
			common_info->hbm.circle_color_cmds.cmd_cnt);
		return LCD_KIT_FAIL;
	}

	/* circle_color_cmds.cmds[r|g|b] index is [1|3|5] */
	/* circle_color_cmds.cmds[r|g|b].dlen maximum value is 5 */
	if ((int)common_info->hbm.circle_color_cmds.cmds[1].dlen < 5 ||
		(int)common_info->hbm.circle_color_cmds.cmds[3].dlen < 5 ||
		(int)common_info->hbm.circle_color_cmds.cmds[5].dlen < 5) {
		LCD_KIT_ERR("array index out of range, cmds.dlen\n");
		return LCD_KIT_FAIL;
	}

	/* circle_color_cmds.cmds[r|g|b].payload value index is 1 & 2 */
	common_info->hbm.circle_color_cmds.cmds[1].payload[1] =
		(((grayscale + 1) << 4) - 1) & 0xff;
	common_info->hbm.circle_color_cmds.cmds[1].payload[2] =
		((((grayscale + 1) << 4) - 1) >> 8) & 0x0f;
	common_info->hbm.circle_color_cmds.cmds[3].payload[1] =
		(((grayscale + 1) << 4) - 1) & 0xff;
	common_info->hbm.circle_color_cmds.cmds[3].payload[2] =
		((((grayscale + 1) << 4) - 1) >> 8) & 0x0f;
	common_info->hbm.circle_color_cmds.cmds[5].payload[1] =
		(((grayscale + 1) << 4) - 1) & 0xff;
	common_info->hbm.circle_color_cmds.cmds[5].payload[2] =
		((((grayscale + 1) << 4) - 1) >> 8) & 0x0f;
	return LCD_KIT_OK;
}

static int display_engine_local_hbm_set_circle_color_vendor3(int panel_id, int grayscale)
{
	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	/* circle_color_cmds.cmds[r|g|b].dlen maximum value is 4 */
	if ((int)common_info->hbm.circle_color_cmds.cmds[1].dlen < 4) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].dlen);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("lcdkit grayscale = %d\n", grayscale);

	/* circle_color_cmds.cmds[r|g|b] index is [1|2|3] */
	common_info->hbm.circle_color_cmds.cmds[1].payload[1] = (char)grayscale;
	common_info->hbm.circle_color_cmds.cmds[1].payload[2] = (char)grayscale;
	common_info->hbm.circle_color_cmds.cmds[1].payload[3] = (char)grayscale;
	return LCD_KIT_OK;
}

static int display_engine_local_hbm_set_circle_color_by_grayscale(int panel_id, void *hld,
	int grayscale)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("cannot get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->hbm.circle_color_cmds.cmds) {
		LCD_KIT_ERR("circle_color_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	switch (common_info->hbm.local_hbm_ddic) {
	case DDIC_VENDOR1:
		ret = display_engine_local_hbm_set_circle_color_vendor1(panel_id, grayscale);
		break;
	case DDIC_VENDOR2:
		ret = display_engine_local_hbm_set_circle_color_vendor2(panel_id, grayscale);
		break;
	case DDIC_VENDOR3:
		ret = display_engine_local_hbm_set_circle_color_vendor3(panel_id, grayscale);
		break;
	default:
		ret = LCD_KIT_FAIL;
		break;
	}
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("set circle color error\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.circle_color_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);
	return ret;
}

static bool display_engine_local_hbm_get_alpha_index(int panel_id,
	struct de_context_local_hbm *de_context, uint32_t *dbv_alpha_index)
{
	if (common_info->hbm.local_hbm_ddic != DDIC_VENDOR3) {
		LCD_KIT_DEBUG("get alpha index bypass\n");
		return true;
	}

	static bool is_first_getting = true;
	uint32_t i = 0;

	if (!is_first_getting)
		return true;
	for (i = 0; i < LHBM_ALPHA_LUT_LENGTH; i++) {
		if (de_context->lum_lut[i] == LCD_KIT_ALPHA_DEFAULT) {
			*dbv_alpha_index = i;
			is_first_getting = false;
			break;
		}
	}

	/* 4095 indicates the maximum dbv */
	if (*dbv_alpha_index > 4095)
		return false;
	return true;
}

static void display_engine_local_hbm_alpha_log_print(
	struct de_context_local_hbm *de_context, uint32_t dbv_alpha_index)
{
	/* 0 indicates the minimum dbv, and 4095 indicates the maximum dbv. */
	if (dbv_alpha_index == 0)
		LCD_KIT_INFO("alpha map alpha_map[%d]:%d, alpha_map[%d]:%d, alpha_map[%d]:%d\n",
			dbv_alpha_index, de_context->lum_lut[dbv_alpha_index],
			dbv_alpha_index + 1, de_context->lum_lut[dbv_alpha_index + 1],
			dbv_alpha_index + 2, de_context->lum_lut[dbv_alpha_index + 2]);
	else if (dbv_alpha_index == 4095)
		LCD_KIT_INFO("alpha map alpha_map[0]:%d, alpha_map[1]:%d, alpha_map[2]:%d, "
			"alpha_map[%d]:%d, alpha_map[%d]:%d, alpha_map[%d]:%d\n",
			de_context->lum_lut[0], de_context->lum_lut[1],
			de_context->lum_lut[2], dbv_alpha_index - 2,
			de_context->lum_lut[dbv_alpha_index - 2], dbv_alpha_index - 1,
			de_context->lum_lut[dbv_alpha_index - 1], dbv_alpha_index,
			de_context->lum_lut[dbv_alpha_index]);
	else
		LCD_KIT_INFO("alpha map alpha_map[0]:%d, alpha_map[1]:%d, alpha_map[2]:%d, "
			"alpha_map[%d]:%d, alpha_map[%d]:%d, alpha_map[%d]:%d\n",
			de_context->lum_lut[0], de_context->lum_lut[1],
			de_context->lum_lut[2], dbv_alpha_index - 1,
			de_context->lum_lut[dbv_alpha_index - 1], dbv_alpha_index,
			de_context->lum_lut[dbv_alpha_index], dbv_alpha_index + 1,
			de_context->lum_lut[dbv_alpha_index + 1]);
}

static void display_engine_local_hbm_get_alpha(int panel_id,
	uint32_t dbv, uint32_t dbv_alpha_index, uint16_t *alpha)
{
	if (common_info->hbm.local_hbm_ddic != DDIC_VENDOR3) {
		*alpha = LCD_KIT_ALPHA_DEFAULT;
		LCD_KIT_DEBUG("get alpha bypass\n");
		return;
	}

	LCD_KIT_INFO("dbv:%d, dbv_alpha_index:%d\n", dbv, dbv_alpha_index);
	display_engine_local_hbm_alpha_log_print(&g_de_context, dbv_alpha_index);
	if (dbv >= dbv_alpha_index) {
		*alpha = LCD_KIT_ALPHA_DEFAULT;
	} else {
		if (dbv >= sizeof(g_de_context.lum_lut) / sizeof(uint16_t)) {
			LCD_KIT_ERR("The dbv value cannot be mapped to alpha\n");
			*alpha = LCD_KIT_ALPHA_DEFAULT;
		} else {
			LCD_KIT_INFO("alpha:%d\n", g_de_context.lum_lut[dbv]);
			*alpha = g_de_context.lum_lut[dbv];
		}
	}
}

static int display_engine_alpha_set_inner(int panel_id, void *hld, uint32_t alpha, bool is_on)
{
	int ret = LCD_KIT_OK;
	static uint32_t last_alpha;
	int forcing_alpha_enable;

	if (common_info->hbm.local_hbm_ddic != DDIC_VENDOR3) {
		LCD_KIT_DEBUG("set alpha inner bypass\n");
		return LCD_KIT_OK;
	}

	if (is_on)
		forcing_alpha_enable = FORCE_ENABLE;
	else
		forcing_alpha_enable = FORCE_DISABLE;

	LCD_KIT_DEBUG("alpha:%d, forcing_alpha_enable:%d\n", alpha, forcing_alpha_enable);
	if (display_engine_alpha_get_support_in_lcd_kit() == 0) {
		LCD_KIT_DEBUG("alpha_support's value is 0\n");
		return ret;
	}
	if (last_alpha == alpha && forcing_alpha_enable == NO_FORCING) {
		LCD_KIT_DEBUG("alpha repeated setting\n");
		return ret;
	}
	if (alpha == 0) {
		alpha = LCD_KIT_ALPHA_DEFAULT;
		LCD_KIT_WARNING("Reinitialize the alpha when the alpha is 0\n");
	}
	if (forcing_alpha_enable == NO_FORCING) {
		if (alpha != LCD_KIT_ALPHA_DEFAULT)
			ret = display_engine_ddic_alpha_is_on_for_c08(panel_id, hld, alpha, true);
		else
			ret = display_engine_ddic_alpha_is_on_for_c08(panel_id, hld, alpha, false);
	} else if (forcing_alpha_enable == FORCE_DISABLE) {
		ret = display_engine_ddic_alpha_is_on_for_c08(panel_id, hld, alpha, false);
	} else if (forcing_alpha_enable == FORCE_ENABLE) {
		ret = display_engine_ddic_alpha_is_on_for_c08(panel_id, hld, alpha, true);
	} else {
		LCD_KIT_ERR("Invalid force_set_alpha_value, forcing_alpha_enable = %d\n",
			forcing_alpha_enable);
		return LCD_KIT_FAIL;
	}

	last_alpha = alpha;
	return ret;
}

static bool display_engine_local_hbm_set_alpha_for_c08(int panel_id,
	void *hld, uint32_t dbv, bool is_on)
{
	if (common_info->hbm.local_hbm_ddic != DDIC_VENDOR3) {
		LCD_KIT_DEBUG("set alpha bypass\n");
		return true;
	}

	static uint32_t dbv_alpha_index = (uint32_t)(-1);
	uint16_t alpha;

	/* Get alpha index */
	if (!display_engine_local_hbm_get_alpha_index(panel_id, &g_de_context, &dbv_alpha_index)) {
		LCD_KIT_ERR("invalid dbv_alpha_index\n");
		return false;
	}

	/* Get alpha value */
	display_engine_local_hbm_get_alpha(panel_id, dbv, dbv_alpha_index, &alpha);

	/* Set alpha */
	display_engine_alpha_set_inner(panel_id, hld, alpha, is_on);
	return true;
}

static int display_engine_local_hbm_set_circle(int panel_id, void *hld, bool is_on)
{
	int ret;
	uint32_t dbv;
	int circle_cmd_index;

	if (!display_engine_local_hbm_set_circle_cmds_valid(panel_id)) {
		LCD_KIT_INFO("set circle cmds invalid\n");
		return LCD_KIT_FAIL;
	}
	dbv = display_engine_brightness_get_mipi_level();
	circle_cmd_index = display_engine_local_hbm_get_circle_cmd_index(panel_id);
	LCD_KIT_INFO("dbv:%d, circle_cmd_index:%d\n", dbv, circle_cmd_index);

	/* Set the alpha corresponding to dbv for c08 ICs, which is without DC mode */
	display_engine_local_hbm_set_alpha_for_c08(panel_id, hld, dbv, is_on);

	ret = display_engine_local_hbm_set_circle_by_index(panel_id, hld, is_on, circle_cmd_index);
	return ret;
}

static int display_engine_local_hbm_set_mcu(int panel_id, void *hld, bool is_on)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!common_info->hbm.circle_mcu_open_cmds.cmds) {
		LCD_KIT_DEBUG("circle_mcu_open_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->hbm.circle_mcu_close_cmds.cmds) {
		LCD_KIT_DEBUG("circle_mcu_close_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("cannot get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	/* close mcu to set circle color by grayscale */
	if (!is_on)
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.circle_mcu_close_cmds);
	else
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.circle_mcu_open_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);
	return ret;
}

int display_engine_local_hbm_change_gamma(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!common_info->hbm.circle_gamma_change_cmds.cmds) {
		LCD_KIT_DEBUG("circle_gamma_change_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("cannot get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	/* change gamma to set circle color by grayscale */
	ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.circle_gamma_change_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);
	return ret;
}

static void display_engine_local_hbm_delay(int panel_id)
{
	LCD_KIT_INFO("safe framerate delay +");
	if (common_info->hbm.local_hbm_ddic == DDIC_VENDOR1)
		usleep_range(LHBM_SAFE_FRAMERATE_RM_DELAY_US, LHBM_SAFE_FRAMERATE_RM_DELAY_US);
	if (common_info->hbm.local_hbm_ddic == DDIC_VENDOR2)
		usleep_range(LHBM_SAFE_FRAMERATE_XHS_DELAY_US, LHBM_SAFE_FRAMERATE_XHS_DELAY_US);
	if (common_info->hbm.local_hbm_ddic == DDIC_VENDOR3)
		usleep_range(LHBM_SAFE_FRAMERATE_RM_DELAY_US, LHBM_SAFE_FRAMERATE_RM_DELAY_US);
	LCD_KIT_INFO("safe framerate delay -");
}

static int display_engine_local_hbm_set_inner(int panel_id, int grayscale)
{
	int ret = LCD_KIT_OK;
	unsigned int dsi0_index;
	unsigned int connector_id;

	connector_id = DPU_PINFO->connector_id;
	ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
	if (ret) {
		LCD_KIT_ERR("get dsi0 index error\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("grayscale: %d\n", grayscale);
	if (grayscale > 0) {
		/* enter LHBM close safe frame */
		dkmd_dfr_disable_safe_frm_rate(dsi0_index, DPU_PINFO->type, LHBM_DISABLE);
		/* max grayscale no need close mcu */
		if (grayscale == 255)
			display_engine_local_hbm_set_mcu(panel_id, NULL, true);
		else
			display_engine_local_hbm_set_mcu(panel_id, NULL, false);
		display_engine_local_hbm_set_circle_color_by_grayscale(panel_id, NULL, grayscale);
		ret = display_engine_local_hbm_set_circle(panel_id, NULL, true);
		/* max grayscale no need change gamma */
		if (grayscale != 255)
			display_engine_local_hbm_change_gamma(panel_id, NULL);
		g_de_context.fingerprint_backlight_type = BACKLIGHT_HIGH_LEVEL;
		/* close dimming, start refresh */
		dkmd_dfr_send_refresh(dsi0_index, DPU_PINFO->type);
	} else {
		ret = display_engine_local_hbm_set_circle(panel_id, NULL, false);
		g_de_context.fingerprint_backlight_type = BACKLIGHT_LOW_LEVEL;
		display_engine_local_hbm_delay(panel_id);
		if (!lcd_kit_get_panel_on_state(panel_id)) {
			LCD_KIT_ERR("panel is power off\n");
			return LCD_KIT_FAIL;
		}
		/* exit LHBM open safe frame */
		dkmd_dfr_enable_safe_frm_rate(dsi0_index, DPU_PINFO->type);
		/* close dimming, start refresh */
		dkmd_dfr_send_refresh(dsi0_index, DPU_PINFO->type);
	}
	return ret;
}

int display_engine_brightness_update(unsigned int bl_level)
{
	g_de_context.last_level_in = bl_level;
	return LCD_KIT_OK;
}

static uint32_t display_engine_brightness_get_mipi_level(void)
{
	return g_de_context.last_level_in;
}

static int display_engine_local_hbm_get_circle_cmd_index(int panel_id)
{
	int i = 0;
	uint32_t dbv;

	if (!common_info->hbm.local_hbm_dbv_threshold.buf)
		return i;
	dbv = display_engine_brightness_get_mipi_level();
	for (i = 0; i < common_info->hbm.local_hbm_dbv_threshold.cnt; i++) {
		if (dbv < common_info->hbm.local_hbm_dbv_threshold.buf[i])
			return i;
	}
	return i - 1;
}

static void display_engine_local_hbm_cmds_print(int panel_id)
{
	LCD_KIT_INFO("LocalHBMSupport: %d, LocalHBMThreshold: <%d, %d, %d>, LocalHBMDdic: %d.\n",
		common_info->hbm.local_hbm_support, common_info->hbm.local_hbm_dbv_threshold.buf[0],
		common_info->hbm.local_hbm_dbv_threshold.buf[1],
		common_info->hbm.local_hbm_dbv_threshold.buf[2],
		common_info->hbm.local_hbm_ddic);
	LCD_KIT_INFO("FirstCommand cnt: %d, SecondCommand cnt: %d, ThirdCommand cnt: %d.\n",
		common_info->hbm.circle_enter_first_cmds.cmd_cnt,
		common_info->hbm.circle_enter_second_cmds.cmd_cnt,
		common_info->hbm.circle_enter_third_cmds.cmd_cnt);
}

void display_engine_local_hbm_workqueue_handler(struct kthread_work *work)
{
	static uint32_t vsync_count = 0;

	if (!work) {
		LCD_KIT_ERR("work is NULL.\n");
		return;
	}

	if (!display_engine_local_hbm_get_support()) {
		LCD_KIT_DEBUG("local hbm not support.\n");
		return;
	}

	if (g_de_context.fingerprint_backlight_type == BACKLIGHT_HIGH_LEVEL) {
		vsync_count += 1;
		if (vsync_count == LHBM_ON_WAIT_VSYNC_COUNT) {
			LCD_KIT_INFO("Fingerprint LHBM notify...\n");
			vsync_count = 0;
#ifdef LCD_FINGERPRINT_ENABLE
			ud_fp_on_hbm_completed();
#endif
		}
	} else {
		vsync_count = 0;
	}
}

static void lcd_kit_parse_local_hbm_support(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,local-hbm-support",
		&common_info->hbm.local_hbm_support, 0);
}

static void lcd_kit_parse_local_hbm_threshold(int panel_id, struct device_node *np)
{
	lcd_kit_parse_array_data(np, "lcd-kit,local-hbm-threshold",
		&common_info->hbm.local_hbm_dbv_threshold);
}

static void lcd_kit_parse_local_hbm_ddic(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,local-hbm-ddic",
		&common_info->hbm.local_hbm_ddic, 0);
}

static void lcd_kit_parse_local_hbm_first_circle_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-circle-enter-first-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_enter_first_cmds);
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-circle-exit-first-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_exit_first_cmds);
}

static void lcd_kit_parse_local_hbm_second_circle_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-circle-enter-second-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_enter_second_cmds);
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-circle-exit-second-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_exit_second_cmds);
}

static void lcd_kit_parse_local_hbm_third_circle_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-circle-enter-third-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_enter_third_cmds);
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-circle-exit-third-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_exit_third_cmds);
}

static void lcd_kit_parse_local_hbm_circle_color_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-circle-color-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_color_cmds);
}

static void lcd_kit_parse_local_hbm_mcu_open_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-mcu-open-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_mcu_open_cmds);
}

static void lcd_kit_parse_local_hbm_mcu_close_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-mcu-close-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_mcu_close_cmds);
}

static void lcd_kit_parse_local_hbm_change_gamma_cmds(int panel_id, struct device_node *np)
{
	lcd_kit_parse_dcs_cmds(np,
		"lcd-kit,local-hbm-fp-panel-change-gamma-cmds-hisi",
		"lcd-kit,panel-circle-command-state",
		&common_info->hbm.circle_gamma_change_cmds);
}

void lcd_kit_local_hbm_cmds_parse(int panel_id, struct device_node *np)
{
	lcd_kit_parse_local_hbm_support(panel_id, np);
	if (!common_info->hbm.local_hbm_support) {
		LCD_KIT_INFO("platform not support local hbm\n");
		return;
	}
	lcd_kit_parse_local_hbm_threshold(panel_id, np);
	lcd_kit_parse_local_hbm_ddic(panel_id, np);
	lcd_kit_parse_local_hbm_first_circle_cmds(panel_id, np);
	lcd_kit_parse_local_hbm_second_circle_cmds(panel_id, np);
	lcd_kit_parse_local_hbm_third_circle_cmds(panel_id, np);
	lcd_kit_parse_local_hbm_circle_color_cmds(panel_id, np);
	lcd_kit_parse_local_hbm_mcu_open_cmds(panel_id, np);
	lcd_kit_parse_local_hbm_mcu_close_cmds(panel_id, np);
	lcd_kit_parse_local_hbm_change_gamma_cmds(panel_id, np);
}

void display_engine_local_hbm_init(void)
{
#ifdef LCD_FINGERPRINT_ENABLE
	struct ud_fp_display_ops finger_print_callback = {NULL, NULL, NULL};

	LCD_KIT_INFO("+\n");
	finger_print_callback.fp_get_local_hbm_mode = display_engine_local_hbm_get_support;
	finger_print_callback.fp_set_brightness_level = display_engine_local_hbm_set;
	ud_fp_display_register(&finger_print_callback);
	LCD_KIT_INFO("-\n");
#endif
}

uint32_t display_engine_local_hbm_get_support(void)
{
	int panel_id = PRIMARY_PANEL;

	LCD_KIT_DEBUG("local hbm support: %d.\n", common_info->hbm.local_hbm_support);
	return common_info->hbm.local_hbm_support;
}

int display_engine_fake_off_handle(int panel_id)
{
	LCD_KIT_INFO("+\n");
	if (g_de_context.fingerprint_backlight_type == BACKLIGHT_HIGH_LEVEL)
		return display_engine_local_hbm_set(0);
	LCD_KIT_INFO("-\n");
	return LCD_KIT_OK;
}

int display_engine_local_hbm_set(int grayscale)
{
	int panel_id = PRIMARY_PANEL;

	LCD_KIT_INFO("+\n");
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		g_de_context.fingerprint_backlight_type = BACKLIGHT_LOW_LEVEL;
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}

	if (!display_engine_local_hbm_get_support()) {
		LCD_KIT_ERR("Local HBM is not supported\n");
		return LCD_KIT_FAIL;
	}
	if (common_info->temper_thread.enable) {
		if (grayscale == 255)
			common_info->temper_thread.temper_lhbm_flag = 1;
	}
	display_engine_local_hbm_set_inner(panel_id, grayscale);
	if (common_info->temper_thread.enable) {
		if (grayscale == 0)
			common_info->temper_thread.temper_lhbm_flag = 0;
	}
	LCD_KIT_INFO("-\n");
	return LCD_KIT_OK;
}

unsigned int display_engine_get_hbm_bl(int panel_id, unsigned int bl_level)
{
	/* just a simple demo */
	int bl_src_interval = common_info->bl_level_max - HBM_THRESHOD;
	int bl_dst_interval = HBM_THRESHOD - common_info->hbm_backlight.hbm_node;
	int bl_diff = 0;
	int bl_dst = 0;

	if (bl_level <= HBM_THRESHOD)
		return bl_level;

	if (bl_src_interval <= 0 || bl_src_interval <= 0)
		return bl_level;

	bl_diff = bl_level - HBM_THRESHOD;
	bl_dst = (int) (bl_diff * bl_dst_interval / bl_src_interval + common_info->hbm_backlight.hbm_node);
	LCD_KIT_INFO("HBM backlight: %d\n", bl_dst);
	return bl_dst;
}

static void display_engine_brightness_set_alpha_lut_default(unsigned short *lut)
{
	int alpha_map_index = 0;

	for (; alpha_map_index < LHBM_ALPHA_LUT_LENGTH; alpha_map_index++)
		lut[alpha_map_index] = LCD_KIT_ALPHA_DEFAULT;
}

static int display_engine_brightness_check_lut(unsigned short *lut, int length)
{
	/* 1 indicates that check from the second value of the array */
	int lut_index = 1;

	/* Check if lut map contains zero */
	for (; lut_index < length; lut_index++) {
		if (lut[lut_index] == 0) {
			LCD_KIT_ERR("dkmd: lut[%d] is 0\n", lut_index);
			break;
		}
	}

	if (lut_index != length)
		return LCD_KIT_FAIL;
	else
		return LCD_KIT_OK;
}

int display_engine_set_param_aplha_map(int panel_id,
	struct display_engine_alpha_map *input_alpha_map)
{
	LCD_KIT_INFO("DISPLAY_ENGINE_ALPHAMAP for panel %d! +\n", panel_id);
	int ret = LCD_KIT_OK;

	if (!input_alpha_map) {
		LCD_KIT_ERR("input_alpha_map is NULL\n");
		return LCD_KIT_FAIL;
	}

	ret = copy_from_user(g_de_context.lum_lut, input_alpha_map->lum_lut,
		sizeof(g_de_context.lum_lut));
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("[effect] copy lum_lut failed, ret = %d\n", ret);
		return LCD_KIT_FAIL;
	}
	ret = display_engine_brightness_check_lut(g_de_context.lum_lut,
		LHBM_ALPHA_LUT_LENGTH);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("check alpha map failed, use default alpha value ret = %d\n", ret);
		display_engine_brightness_set_alpha_lut_default(g_de_context.lum_lut);
	}
	LCD_KIT_INFO("target_alpha_map[0 - 4]: [%u, %u, %u, %u, %u], [last]: [%u]",
		g_de_context.lum_lut[0], g_de_context.lum_lut[1],
		g_de_context.lum_lut[2], g_de_context.lum_lut[3],
		g_de_context.lum_lut[4], g_de_context.lum_lut[LHBM_ALPHA_LUT_LENGTH - 1]);
	LCD_KIT_INFO("DISPLAY_ENGINE_ALPHAMAP for panel %d! -\n", panel_id);
	return ret;
}

int display_engine_ddic_alpha_is_on_for_c08(int panel_id, void *hld, uint32_t alpha, bool is_on)
{
	int payload_num;
	int enter_cmd_cnt;
	int exit_cmd_cnt;
	if (common_info->hbm.circle_enter_first_cmds.cmds == NULL ||
		common_info->hbm.circle_exit_first_cmds.cmds == NULL) {
		LCD_KIT_ERR("enter or exit circle_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	enter_cmd_cnt = common_info->hbm.circle_enter_first_cmds.cmd_cnt;
	exit_cmd_cnt = common_info->hbm.circle_exit_first_cmds.cmd_cnt;
	if (enter_cmd_cnt <= 0 || enter_cmd_cnt > payload_index_max ||
		exit_cmd_cnt <= 0 || exit_cmd_cnt > payload_index_max) {
		LCD_KIT_ERR("enter or exit circle_cmds's, enter_cmd_cnt:%d,exit_cmd_cnt:%d.\n",
		enter_cmd_cnt, exit_cmd_cnt);
		return LCD_KIT_FAIL;
	}
	payload_num = (int)common_info->hbm.circle_enter_first_cmds.cmds[alpha_cmd_index].dlen;
	LCD_KIT_INFO("enter_cmd_cnt = %d, payload_num = %d\n", enter_cmd_cnt, payload_num);
	/* Sets the upper eight bits of the alpha. 5-11 bit is hight eight bits */
	display_engine_circle_cmds_set_payload_for_c08(
		panel_id, alpha_cmd_index, payload_num -2, (alpha >> 5) & 0xff, is_on);
	/* Sets the lower eight bits of the alpha. 0-4 bit is low eight bits */
	display_engine_circle_cmds_set_payload_for_c08(
		panel_id, alpha_cmd_index, payload_num -1, alpha & 0x1f, is_on);
	LCD_KIT_INFO("alpha_cmd_index[%d], enter payload[2]=%d.payload[3]=%d. \
		exit payload[2]=%d.payload[3]=%d\n", alpha_cmd_index,
		common_info->hbm.circle_enter_first_cmds.cmds[alpha_cmd_index].payload[payload_num - 2],
		common_info->hbm.circle_enter_first_cmds.cmds[alpha_cmd_index].payload[payload_num - 1],
		common_info->hbm.circle_exit_first_cmds.cmds[alpha_cmd_index].payload[payload_num - 2],
		common_info->hbm.circle_exit_first_cmds.cmds[alpha_cmd_index].payload[payload_num - 1]);
	return LCD_KIT_OK;
}

void display_engine_circle_cmds_set_payload_for_c08(
	int panel_id, int index, int payload_num, char value, bool is_on)
{
	if (index > payload_index_max || index < payload_index_min) {
		LCD_KIT_ERR("circle cmds set payload index out\n");
		return;
	}
	if (!is_on) {
		common_info->hbm.circle_exit_first_cmds.cmds[index].payload[payload_num] = value;
	} else {
		common_info->hbm.circle_enter_first_cmds.cmds[index].payload[payload_num] = value;
	}
}
