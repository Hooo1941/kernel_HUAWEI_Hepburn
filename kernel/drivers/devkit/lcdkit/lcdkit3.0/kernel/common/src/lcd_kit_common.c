/*
 * lcd_kit_common.c
 *
 * lcdkit common function for lcdkit
 *
 * Copyright (c) 2018-2022 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_common.h"
#include "lcd_kit_bridge.h"
#include "lcd_kit_parse.h"
#include <linux/thermal.h>
#ifdef LCD_KIT_DEBUG_ENABLE
#include "lcd_kit_dbg.h"
#endif
#ifdef LCD_KIT_DKMD
#include "display_engine_interface.h"
#include "lcd_kit_local_hbm.h"
#endif
#if defined(LCD_KIT_ALPHA_LHBM)
#include "lcd_kit_local_hbm.h"
#endif
#include <securec.h>

#if defined CONFIG_HUAWEI_DSM
extern struct dsm_client *lcd_dclient;
#endif
#ifdef LV_GET_LCDBK_ON
u32 mipi_level;
#endif

#define BL_MAX 256
#define SKIP_SEND_EVENT 1
#define MAX_BTB_IRQ_COUNT 2
#define ESD_DETECT_GPIO_ONLY_MODE 2
static int btb_irq_count = 0;
static int lcd_backlight_i2c_count = 0;
static int lcd_bias_i2c_count = 0;
static const uint32_t fps_normal = 60;
static const uint32_t fps_medium = 90;
#define SET_BL_BIT 0x8000
#define MAX_TEMPER 50
#define MIN_TEMPER 25
#define TEMPER_CMD 0x1A
#define TMEPER_VALUE 26
#define LCD_VER_INDEX 0
#define LCD_HOR_INDEX 1
#define TEMPER_POS_MAX 2
#define TMPER_ACCURACY 1000

/* The maximum number of enter_ddic_alpha commands is 16, you can change it if needed */
static const int cmd_cnt_max = 16;

/* The numbers of enter_alpha_cmds.cmds's payload, you can change them if needed */
static const int payload_num_min = 3;
static const int payload_num_max = 8;

static const int color_cmd_index = 1;

enum hbm_gamma_index {
	GAMMA_INDEX_RED_HIGH = 0,
	GAMMA_INDEX_RED_LOW = 1,
	GAMMA_INDEX_GREEN_HIGH = 24,
	GAMMA_INDEX_GREEN_LOW = 25,
	GAMMA_INDEX_BLUE_HIGH = 48,
	GAMMA_INDEX_BLUE_LOW = 49
};

enum color_cmds_index {
	CMDS_RED_HIGH = 9,
	CMDS_RED_LOW = 10,
	CMDS_GREEN_HIGH = 11,
	CMDS_GREEN_LOW = 12,
	CMDS_BLUE_HIGH = 13,
	CMDS_BLUE_LOW = 14,
	CMDS_COLOR_MAX = 15
};

int lcd_kit_msg_level = MSG_LEVEL_INFO;
/* common info */
struct lcd_kit_common_info g_lcd_kit_common_info[LCD_ACTIVE_PANEL_BUTT];
/* common ops */
struct lcd_kit_common_ops g_lcd_kit_common_ops;
/* power handle */
struct lcd_kit_power_desc g_lcd_kit_power_handle[LCD_ACTIVE_PANEL_BUTT];
/* power on/off sequence */
static struct lcd_kit_power_seq g_lcd_kit_power_seq[LCD_ACTIVE_PANEL_BUTT];
/* esd error info */
struct lcd_kit_esd_error_info g_esd_error_info;
/* hw adapt ops */
static struct lcd_kit_adapt_ops *g_adapt_ops;
/* common lock */
struct lcd_kit_common_lock g_lcd_kit_common_lock;
/* if in pwm range */
static bool is_in_pwm_range = false;

/* dsi test */
#define RECORD_BUFLEN_DSI 200
#define REC_DMD_NO_LIMIT_DSI (-1)
char record_buf_dsi[RECORD_BUFLEN_DSI] = {'\0'};
int cur_rec_time_dsi = 0;

static int lcd_kit_get_proxmity_status(int panel_id, int data);

int lcd_kit_adapt_register(struct lcd_kit_adapt_ops *ops)
{
	if (g_adapt_ops) {
		LCD_KIT_ERR("g_adapt_ops has already been registered!\n");
		return LCD_KIT_FAIL;
	}
	g_adapt_ops = ops;
	LCD_KIT_INFO("g_adapt_ops register success!\n");
	return LCD_KIT_OK;
}

int lcd_kit_adapt_unregister(struct lcd_kit_adapt_ops *ops)
{
	if (g_adapt_ops == ops) {
		g_adapt_ops = NULL;
		LCD_KIT_INFO("g_adapt_ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("g_adapt_ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_adapt_ops *lcd_kit_get_adapt_ops(void)
{
	return g_adapt_ops;
}

struct lcd_kit_common_info *lcd_kit_get_common_info(int panel_id)
{
	return &g_lcd_kit_common_info[panel_id];
}

struct lcd_kit_common_ops *lcd_kit_get_common_ops(void)
{
	return &g_lcd_kit_common_ops;
}

struct lcd_kit_power_desc *lcd_kit_get_power_handle(int panel_id)
{
	return &g_lcd_kit_power_handle[panel_id];
}

struct lcd_kit_power_seq *lcd_kit_get_power_seq(int panel_id)
{
	return &g_lcd_kit_power_seq[panel_id];
}

struct lcd_kit_power_desc *lcd_kit_get_panel_power_handle(
	uint32_t panel_id)
{
	if (panel_id >= LCD_ACTIVE_PANEL_BUTT) {
		LCD_KIT_ERR("panel_id %u!\n", panel_id);
		return NULL;
	}
	return &g_lcd_kit_power_handle[panel_id];
}

struct lcd_kit_power_seq *lcd_kit_get_panel_power_seq(
	uint32_t panel_id)
{
	if (panel_id >= LCD_ACTIVE_PANEL_BUTT) {
		LCD_KIT_ERR("panel_id %u!\n", panel_id);
		return NULL;
	}
	return &g_lcd_kit_power_seq[panel_id];
}

struct lcd_kit_common_lock *lcd_kit_get_common_lock(void)
{
	return &g_lcd_kit_common_lock;
}

int get_panel_id(void)
{
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return 0;
	}
	if (lcd_ops->get_panel_id)
		return lcd_ops->get_panel_id();
	return 0;
}

static int lcd_kit_set_bias_ctrl(int enable)
{
	int ret = LCD_KIT_OK;
	int panel_id = get_panel_id();
	struct lcd_kit_bias_ops *bias_ops = NULL;

	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	if (enable) {
		if (power_hdl->lcd_vsp.buf == NULL) {
			LCD_KIT_ERR("can not get lcd voltage!\n");
			return LCD_KIT_FAIL;
		}
		if (bias_ops->set_bias_voltage)
			/* buf[2]:set voltage value */
			ret = bias_ops->set_bias_voltage(
				power_hdl->lcd_vsp.buf[POWER_VOL],
				power_hdl->lcd_vsn.buf[POWER_VOL]);
		if (ret)
			LCD_KIT_ERR("set_bias failed\n");
	} else {
		if (power_hdl->lcd_power_down_vsp.buf == NULL) {
			LCD_KIT_INFO("PowerDownVsp is not configured in xml!\n");
			return LCD_KIT_FAIL;
		}
		if (bias_ops->set_bias_power_down)
			/* buf[2]:set voltage value */
			ret = bias_ops->set_bias_power_down(
				power_hdl->lcd_power_down_vsp.buf[POWER_VOL],
				power_hdl->lcd_power_down_vsn.buf[POWER_VOL]);
		if (ret)
			LCD_KIT_ERR("power_down_set_bias failed!\n");
	}
	return ret;
}

static int lcd_kit_vci_power_ctrl(int panel_id, int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_vci.buf) {
		LCD_KIT_ERR("can not get lcd vci!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_vci.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(panel_id, LCD_KIT_VCI);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(panel_id, LCD_KIT_VCI);
		}
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(panel_id, LCD_KIT_VCI, enable);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vci mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vci mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_iovcc_power_ctrl(int panel_id, int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_iovcc.buf) {
		LCD_KIT_ERR("can not get lcd iovcc!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_iovcc.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(panel_id, LCD_KIT_IOVCC);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(panel_id, LCD_KIT_IOVCC);
		}
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(panel_id, LCD_KIT_IOVCC, enable);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd iovcc mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd iovcc mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_vdd_power_ctrl(int panel_id, int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_vdd.buf) {
		LCD_KIT_ERR("can not get lcd iovcc!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_vdd.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(panel_id, LCD_KIT_VDD);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(panel_id, LCD_KIT_VDD);
		}
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(panel_id, LCD_KIT_VDD, enable);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vdd mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vdd mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static void lcd_kit_set_ic_disable(int panel_id, int enable)
{
	int ret;
	struct lcd_kit_bias_ops *bias_ops = NULL;

	if (enable)
		return;
	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("can not bias_ops!\n");
		return;
	}

	if (!bias_ops->set_ic_disable) {
		LCD_KIT_ERR("set_ic_disable is null!\n");
		return;
	}

	ret = bias_ops->set_ic_disable();
	if (ret) {
		LCD_KIT_ERR("ic disbale fail !\n");
		return;
	}
	LCD_KIT_INFO("ic disbale successful\n");
}

static int lcd_kit_vsp_power_ctrl(int panel_id, int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_vsp.buf) {
		LCD_KIT_ERR("can not get lcd vsp!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_vsp.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(panel_id, LCD_KIT_VSP);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(panel_id, LCD_KIT_VSP);
		}
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(panel_id, LCD_KIT_VSP, enable);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vsp mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vsp mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_set_ic_disable(panel_id, enable);
	return ret;
}

static int lcd_kit_vsn_power_ctrl(int panel_id, int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_vsn.buf) {
		LCD_KIT_ERR("can not get lcd vsn!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_vsn.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(panel_id, LCD_KIT_VSN);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(panel_id, LCD_KIT_VSN);
		}
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_enable)
			ret = adapt_ops->regulator_enable(panel_id, LCD_KIT_VSN, enable);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vsn mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vsn mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_aod_power_ctrl(int panel_id, int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_aod.buf) {
		LCD_KIT_ERR("can not get lcd lcd_aod!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_aod.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				adapt_ops->gpio_enable(panel_id, LCD_KIT_AOD);
		} else {
			if (adapt_ops->gpio_disable)
				adapt_ops->gpio_disable(panel_id, LCD_KIT_AOD);
		}
		break;
	case REGULATOR_MODE:
		if (adapt_ops->regulator_enable)
			adapt_ops->regulator_enable(panel_id, LCD_KIT_AOD, enable);
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd lcd_aod mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd lcd_aod mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
int lcd_kit_set_bias_voltage(void)
{
	struct lcd_kit_bias_ops *bias_ops = NULL;
	int ret = LCD_KIT_OK;
	int panel_id = get_panel_id();

	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("can not get bias_ops!\n");
		return LCD_KIT_FAIL;
	}
	/* set bias voltage */
	if (bias_ops->set_bias_voltage)
		ret = bias_ops->set_bias_voltage(power_hdl->lcd_vsp.buf[POWER_VOL],
			power_hdl->lcd_vsn.buf[POWER_VOL]);
	return ret;
}

static void lcd_kit_proximity_record_time(int panel_id)
{
	struct timespec64 *reset_tv = NULL;

	if (common_info == NULL)
		return;
	if (lcd_kit_get_proxmity_status(panel_id, LCD_RESET_HIGH) != TP_PROXMITY_ENABLE ||
		common_info->thp_proximity.after_reset_delay_min == 0)
		return;
	reset_tv = &(common_info->thp_proximity.lcd_reset_record_tv);
	ktime_get_real_ts64(reset_tv);
	LCD_KIT_INFO("record lcd reset power on time");
}
int lcd_kit_set_mipi_switch_ctrl(int panel_id, int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_mipi_switch.buf == NULL) {
		LCD_KIT_ERR("can not get lcd mipi switch!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_mipi_switch.buf[0]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable_nolock)
				adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_MIPI_SWITCH);
		} else {
			if (adapt_ops->gpio_disable_nolock)
				adapt_ops->gpio_disable_nolock(panel_id, LCD_KIT_MIPI_SWITCH);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd mipi switch is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd mipi switch is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
static int lcd_kit_reset_power_on(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_rst.buf) {
		LCD_KIT_ERR("can not get lcd reset!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_rst.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (adapt_ops->gpio_enable)
			ret = adapt_ops->gpio_enable(panel_id, LCD_KIT_RST);
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n", power_hdl->lcd_rst.buf[POWER_MODE]);
		break;
	}
	lcd_kit_proximity_record_time(panel_id);
	return ret;
}

static int lcd_kit_reset_power_off(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_rst.buf) {
		LCD_KIT_ERR("can not get lcd reset!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_rst.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (adapt_ops->gpio_disable)
			ret = adapt_ops->gpio_disable(panel_id, LCD_KIT_RST);
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n",
			power_hdl->lcd_rst.buf[POWER_MODE]);
		break;
	}
	return ret;
}

int lcd_kit_reset_power_ctrl(int panel_id, int enable)
{
	if (enable == LCD_RESET_HIGH)
		return lcd_kit_reset_power_on(panel_id);
	else
		return lcd_kit_reset_power_off(panel_id);
}

static int lcd_kit_pmic_ctrl_event(int panel_id, int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_pmic_ctrl.buf == NULL) {
		LCD_KIT_ERR("can not get lcd pmic ctrl!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_pmic_ctrl.buf[0]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable_nolock)
				adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_PMIC_CTRL);
		} else {
			if (adapt_ops->gpio_disable_nolock)
				adapt_ops->gpio_disable_nolock(panel_id, LCD_KIT_PMIC_CTRL);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd pmic ctrl is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd pmic ctrl is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_pmic_reset_event(int panel_id, int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_pmic_reset.buf == NULL) {
		LCD_KIT_ERR("can not get lcd pmic reset!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_pmic_reset.buf[0]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable_nolock)
				adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_PMIC_RESET);
		} else {
			if (adapt_ops->gpio_disable_nolock)
				adapt_ops->gpio_disable_nolock(panel_id, LCD_KIT_PMIC_RESET);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd pmic reset is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd pmic reset is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_pmic_init_event(int panel_id, int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_pmic_init.buf == NULL) {
		LCD_KIT_ERR("can not get lcd pmic init!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_pmic_init.buf[0]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable_nolock)
				adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_PMIC_INIT);
		} else {
			if (adapt_ops->gpio_disable_nolock)
				adapt_ops->gpio_disable_nolock(panel_id, LCD_KIT_PMIC_INIT);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd pmic init is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd pmic init is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_boost_enable_event(int panel_id, int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_boost_enable.buf == NULL) {
		LCD_KIT_ERR("can not get lcd boost enable!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_boost_enable.buf[0]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable_nolock)
				adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_BOOST_ENABLE);
		} else {
			if (adapt_ops->gpio_disable_nolock)
				adapt_ops->gpio_disable_nolock(panel_id, LCD_KIT_BOOST_ENABLE);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd boost enable is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd boost enable is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_boost_ctrl_event(int panel_id, int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_boost_ctrl.buf == NULL) {
		LCD_KIT_ERR("can not get lcd boost ctrl!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_boost_ctrl.buf[0]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable_nolock)
				adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_BOOST_CTRL);
		} else {
			if (adapt_ops->gpio_disable_nolock)
				adapt_ops->gpio_disable_nolock(panel_id, LCD_KIT_BOOST_CTRL);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd boost ctrl is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd boost ctrl is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_boost_bypass_event(int panel_id, int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_boost_bypass.buf == NULL) {
		LCD_KIT_ERR("can not get lcd boost bypass!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_boost_bypass.buf[0]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable_nolock)
				adapt_ops->gpio_enable_nolock(panel_id, LCD_KIT_BOOST_BYPASS);
		} else {
			if (adapt_ops->gpio_disable_nolock)
				adapt_ops->gpio_disable_nolock(panel_id, LCD_KIT_BOOST_BYPASS);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd boost bypass is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd boost bypass is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static void lcd_kit_mipp_add(int panel_id, void *hld)
{
	int ret;
	int cmds_add_numb;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!common_info->panel_software_id.support)
		return;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not get adapt_ops or mipi_rx\n");
		return;
	}

	cmds_add_numb = common_info->panel_software_id.number;

	if (cmds_add_numb >= SOFTWARE_ID_CMDS_ADD_NUM_MAX || cmds_add_numb < 0) {
		LCD_KIT_ERR("cmds_add_numb:%d is not right, use default 0\n", cmds_add_numb);
		cmds_add_numb = 0;
	}

	LCD_KIT_INFO("panel software id number is:%d!\n", cmds_add_numb);

	if (common_info->panel_software_id.panel_on_cmds_add[cmds_add_numb].cmds) {
		ret = adapt_ops->mipi_tx(panel_id, hld, &(common_info->panel_software_id.panel_on_cmds_add[cmds_add_numb]));
		if (ret) {
			LCD_KIT_ERR("send cmds error.\n");
			return;
		}
	}
	return;
}

static void lcd_kit_fps_on_cmds(int panel_id, void *hld, struct lcd_kit_adapt_ops *adapt_ops)
{
	int ret = LCD_KIT_OK;

	if (!common_info->fps_on.support) {
		LCD_KIT_INFO("send panel on cmds\n");
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->panel_on_cmds);
		if (ret)
			LCD_KIT_ERR("send panel on cmds error\n");
		/* panel software id : mipi add */
		lcd_kit_mipp_add(panel_id, hld);
		return;
	}
	if (common_info->fps_on.current_fps == 60) {
		LCD_KIT_INFO("send panel on 60 cmds\n");
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->panel_on_cmds);
		if (ret)
			LCD_KIT_ERR("send panel on 60 cmds error\n");
		/* panel software id : mipi add */
		lcd_kit_mipp_add(panel_id, hld);
	} else if (common_info->fps_on.current_fps == 90) {
		LCD_KIT_INFO("send panel on 90 cmds\n");
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->fps_on.panel_on_90_cmds);
		if (ret)
			LCD_KIT_ERR("send panel on 90 cmds error\n");
	} else if (common_info->fps_on.current_fps == 120) {
		LCD_KIT_INFO("send panel on 120 cmds\n");
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->fps_on.panel_on_120_cmds);
		if (ret)
			LCD_KIT_ERR("send panel on 120 cmds error\n");
	} else if (common_info->fps_on.current_fps == 144) {
		LCD_KIT_INFO("send panel on 144 cmds\n");
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->fps_on.panel_on_144_cmds);
		if (ret)
			LCD_KIT_ERR("send panel on 144 cmds error\n");
	} else {
		LCD_KIT_INFO("send panel on deault cmds\n");
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->panel_on_cmds);
		if (ret)
			LCD_KIT_ERR("send panel on default cmds error\n");
		/* panel software id : mipi add */
		lcd_kit_mipp_add(panel_id, hld);
	}
}

static int lcd_kit_ppc_on_cmds(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int display_id;
	int i;
	int master_index;
	int slave_index;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	if (!common_info->ppc_para.panel_partial_ctrl_support) {
		LCD_KIT_INFO("panel partial ctrl not support\n");
		return LCD_KIT_OK;
	}

	if (!adapt_ops->mipi_tx || !adapt_ops->dual_mipi_diff_cmd_tx) {
		LCD_KIT_ERR("not register mipi tx or mipi_diff_cmd_tx!\n");
		return LCD_KIT_FAIL;
	}

	display_id = common_info->ppc_para.ppc_config_id;
	LCD_KIT_INFO("panel partial ctrl id:%u\n", display_id);

	for (i = 0; i < common_info->ppc_para.cmds_cnt; i++) {
		master_index = display_id * common_info->ppc_para.dsi_cnt * common_info->ppc_para.cmds_cnt
			+ 0 * common_info->ppc_para.cmds_cnt + i; // 0:master dsi index
		slave_index = display_id * common_info->ppc_para.dsi_cnt * common_info->ppc_para.cmds_cnt
			+ 1 * common_info->ppc_para.cmds_cnt + i; // 1:slave dsi index

		if (common_info->ppc_para.dsi_cnt == 2) { // 2:dual dsi
			ret = adapt_ops->dual_mipi_diff_cmd_tx(panel_id, hld, &common_info->ppc_on_cmds[master_index],
				&common_info->ppc_on_cmds[slave_index]);
		} else if (common_info->ppc_para.dsi_cnt == 1) { // 1:single dsi
			ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->ppc_on_cmds[master_index]);
		}

		if (ret) {
			LCD_KIT_ERR("send ppc on cmds error\n");
			return ret;
		}
	}

	return ret;
}

static int lcd_kit_on_cmds(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	/* init code */
	if (adapt_ops->mipi_tx) {
		lcd_kit_fps_on_cmds(panel_id, hld, adapt_ops);
		/* send panel on effect code */
		if (!common_info->effect_on.support)
			return ret;
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->effect_on.cmds);
		if (ret)
			LCD_KIT_ERR("send effect on cmds error\n");
	}
	return ret;
}

static int lcd_kit_off_cmds(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	/* send panel off code */
	if (adapt_ops->mipi_tx)
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->panel_off_cmds);
	return ret;
}

static int lcd_kit_check_reg_report_dsm(int panel_id, void *hld,
	struct lcd_kit_check_reg_dsm *check_reg_dsm)
{
	int ret = LCD_KIT_OK;
	uint8_t read_value[MAX_REG_READ_COUNT] = {0};
	int i;
	char *expect_ptr = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

#ifdef LCD_KIT_DKMD
	if (!check_reg_dsm) {
#else
	if (!hld || !check_reg_dsm) {
#endif
		LCD_KIT_ERR("null pointer!\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!check_reg_dsm->support) {
		LCD_KIT_DEBUG("not support check reg dsm\n");
		return ret;
	}
	expect_ptr = (char *)check_reg_dsm->value.buf;
	if (adapt_ops->mipi_rx)
		ret = adapt_ops->mipi_rx(panel_id, hld, read_value, MAX_REG_READ_COUNT - 1,
			&check_reg_dsm->cmds);
	if (ret == LCD_KIT_OK) {
		for (i = 0; i < check_reg_dsm->value.cnt; i++) {
			if (!check_reg_dsm->support_dsm_report) {
				LCD_KIT_INFO("read_value[%d] = 0x%x!\n",
					i, read_value[i]);
				continue;
			}
			if ((char)read_value[i] != expect_ptr[i]) {
				ret = LCD_KIT_FAIL;
				LCD_KIT_ERR("read_value[%d] = 0x%x, but expect_ptr[%d] = 0x%x!\n",
					i, read_value[i], i, expect_ptr[i]);
#if defined CONFIG_HUAWEI_DSM
				dsm_client_record(lcd_dclient, "lcd register read_value[%d] = 0x%x, but expect_ptr[%d] = 0x%x!\n",
					i, read_value[i], i, expect_ptr[i]);
#endif
				break;
			}
			LCD_KIT_INFO("read_value[%d] = 0x%x same with expect value!\n",
				i, read_value[i]);
		}
	} else {
		LCD_KIT_ERR("mipi read error!\n");
	}
	if (ret != LCD_KIT_OK) {
		if (check_reg_dsm->support_dsm_report) {
#if defined CONFIG_HUAWEI_DSM
			if (dsm_client_ocuppy(lcd_dclient))
				return ret;
			dsm_client_notify(lcd_dclient, DSM_LCD_STATUS_ERROR_NO);
#endif
		}
	}
	return ret;
}

static void lcd_kit_proxmity_proc(int panel_id, int enable)
{
	long delta_time;
	int delay_margin;
	struct timespec64 tv;
	struct timespec64 *reset_tv = NULL;

	if (common_info == NULL)
		return;
	if (lcd_kit_get_proxmity_status(panel_id, enable) != TP_PROXMITY_ENABLE ||
		common_info->thp_proximity.after_reset_delay_min == 0)
		return;
	memset(&tv, 0, sizeof(struct timespec64));
	ktime_get_real_ts64(&tv);
	reset_tv = &(common_info->thp_proximity.lcd_reset_record_tv);
	/* change s to ns */
	delta_time = (tv.tv_sec - reset_tv->tv_sec) * 1000000000 +
		tv.tv_nsec - reset_tv->tv_nsec;
	/* change ns to ms */
	delta_time /= 1000000;
	if (delta_time >= common_info->thp_proximity.after_reset_delay_min)
		return;
	delay_margin = common_info->thp_proximity.after_reset_delay_min - delta_time;
	if (delay_margin > common_info->thp_proximity.after_reset_delay_min ||
		delay_margin > MAX_MARGIN_DELAY)
		return;
	lcd_kit_delay(delay_margin, LCD_KIT_WAIT_MS, true);
	LCD_KIT_INFO("delay_margin:%d ms\n", delay_margin);
}

static int lcd_kit_mipi_power_ctrl(int panel_id, void *hld, int enable)
{
	int ret = LCD_KIT_OK;
	int ret_check_reg;

	if (enable) {
		if (common_info->aod_no_need_init &&
			common_info->new_doze_state) {
			common_info->new_doze_state = false;
			LCD_KIT_INFO("aod no need init\n");
			return ret;
		}
		lcd_kit_proxmity_proc(panel_id, enable);
		ret = lcd_kit_on_cmds(panel_id, hld);
		ret_check_reg = lcd_kit_check_reg_report_dsm(panel_id, hld,
			&common_info->check_reg_on);
		if (ret_check_reg != LCD_KIT_OK)
			LCD_KIT_ERR("power on check reg error!\n");
	} else {
		ret = lcd_kit_off_cmds(panel_id, hld);
		ret_check_reg = lcd_kit_check_reg_report_dsm(panel_id, hld,
			&common_info->check_reg_off);
		if (ret_check_reg != LCD_KIT_OK)
			LCD_KIT_ERR("power off check reg error!\n");
	}
	return ret;
}

static int lcd_kit_aod_enter_ap_cmds(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	/* aod enter ap code */
	if (adapt_ops->mipi_tx) {
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->aod_exit_dis_on_cmds);
		if (ret)
			LCD_KIT_ERR("send aod exit disp on cmds error\n");
	}
	return ret;
}

static int lcd_kit_aod_mipi_ctrl(int panel_id, void *hld, int enable)
{
	int ret = LCD_KIT_OK;
	if (enable)
		ret = lcd_kit_aod_enter_ap_cmds(panel_id, hld);
	return ret;
}

static int lcd_kit_ext_ts_resume(int panel_id, int sync)
{
	int ret;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_multi_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_multi_power_notify(TS_RESUME_DEVICE,
			SHORT_SYNC, panel_id);
	else
		ret = ts_ops->ts_multi_power_notify(TS_RESUME_DEVICE,
			NO_SYNC, panel_id);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_resume(int panel_id, int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_ops->exec_ext_ts && lcd_ops->exec_ext_ts())
		return lcd_kit_ext_ts_resume(panel_id, sync);

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_RESUME_DEVICE, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_RESUME_DEVICE, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ext_ts_after_resume(int panel_id, int sync)
{
	int ret;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_multi_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_multi_power_notify(TS_AFTER_RESUME,
			SHORT_SYNC, panel_id);
	else
		ret = ts_ops->ts_multi_power_notify(TS_AFTER_RESUME,
			NO_SYNC, panel_id);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_after_resume(int panel_id, int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_ops->exec_ext_ts && lcd_ops->exec_ext_ts())
		return lcd_kit_ext_ts_after_resume(panel_id, sync);

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_AFTER_RESUME, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_AFTER_RESUME, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ext_ts_suspend(int panel_id, int sync)
{
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_multi_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync) {
		ts_ops->ts_multi_power_notify(TS_BEFORE_SUSPEND,
			SHORT_SYNC, panel_id);
		ts_ops->ts_multi_power_notify(TS_SUSPEND_DEVICE,
			SHORT_SYNC, panel_id);
	} else {
		ts_ops->ts_multi_power_notify(TS_BEFORE_SUSPEND,
			NO_SYNC, panel_id);
		ts_ops->ts_multi_power_notify(TS_SUSPEND_DEVICE,
			NO_SYNC, panel_id);
	}
	LCD_KIT_INFO("sync is %d\n", sync);
	return LCD_KIT_OK;
}

static int lcd_kit_ts_suspend(int panel_id, int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_ops->exec_ext_ts && lcd_ops->exec_ext_ts())
		return lcd_kit_ext_ts_suspend(panel_id, sync);

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync) {
		ts_ops->ts_power_notify(TS_BEFORE_SUSPEND, SHORT_SYNC);
		ts_ops->ts_power_notify(TS_SUSPEND_DEVICE, SHORT_SYNC);
	} else {
		ts_ops->ts_power_notify(TS_BEFORE_SUSPEND, NO_SYNC);
		ts_ops->ts_power_notify(TS_SUSPEND_DEVICE, NO_SYNC);
	}
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ext_ts_early_suspend(int panel_id, int sync)
{
	int ret;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_multi_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_multi_power_notify(TS_EARLY_SUSPEND,
			SHORT_SYNC, panel_id);
	else
		ret = ts_ops->ts_multi_power_notify(TS_EARLY_SUSPEND,
			NO_SYNC, panel_id);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static bool lcd_kit_is_power_event(uint32_t event)
{
	switch (event) {
	case EVENT_VCI:
	case EVENT_IOVCC:
	case EVENT_RESET:
	case EVENT_VDD:
		return true;
	default:
		break;
	}
	return false;
}

static int lcd_kit_ts_early_suspend(int panel_id, int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_ops->exec_ext_ts && lcd_ops->exec_ext_ts())
		return lcd_kit_ext_ts_early_suspend(panel_id, sync);

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_EARLY_SUSPEND, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_EARLY_SUSPEND, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_2nd_power_off(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_2ND_POWER_OFF, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_2ND_POWER_OFF, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_block_tprst(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_BLOCK_TPRST, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_BLOCK_TPRST, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_unblock_tprst(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_UNBLOCK_TPRST, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_UNBLOCK_TPRST, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_early_ts_event(int panel_id, int enable, int sync)
{
	if (enable)
		return lcd_kit_ts_resume(panel_id, sync);
	return lcd_kit_ts_early_suspend(panel_id, sync);
}

static int lcd_kit_later_ts_event(int panel_id, int enable, int sync)
{
	if (enable)
		return lcd_kit_ts_after_resume(panel_id, sync);
	return lcd_kit_ts_suspend(panel_id, sync);
}

static int lcd_kit_2nd_power_off_ts_event(int panel_id, int enable, int sync)
{
	return lcd_kit_ts_2nd_power_off(sync);
}

static int lcd_kit_block_ts_event(int enable, int sync)
{
	if (enable)
		return lcd_kit_ts_block_tprst(sync);
	return lcd_kit_ts_unblock_tprst(sync);
}

static int lcd_kit_eink_power_ctrl_event(int enable)
{
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (lcd_ops->eink_power_ctrl)
		return lcd_ops->eink_power_ctrl(enable);
	return LCD_KIT_OK;
}

static int lcd_kit_bridge_ctrl_event(int enable)
{
	struct lcd_kit_bridge_ops *bridge_ops = NULL;

	bridge_ops = lcd_kit_get_bridge_ops();
	if (!bridge_ops || !bridge_ops->pre_enable || !bridge_ops->enable)
		return LCD_KIT_FAIL;
	switch (enable) {
	case BRIDGE_DISABLE:
		if (bridge_ops->disable)
			bridge_ops->disable();
		else
			LCD_KIT_INFO("bridge disable is NULL\n");
		break;
	case BRIDGE_ENABLE:
		bridge_ops->pre_enable();
		bridge_ops->enable();
		break;
	case BRIDGE_REFCLK_ON:
		if (bridge_ops->refclk_on)
			bridge_ops->refclk_on();
		else
			LCD_KIT_INFO("bridge set refclk is NULL\n");
		break;
	case BRIDGE_REFCLK_OFF:
		if (bridge_ops->refclk_off)
			bridge_ops->refclk_off();
		else
			LCD_KIT_INFO("bridge disable refclk is NULL\n");
		break;
	default:
		LCD_KIT_ERR("lcd bridge switch is not normal\n");
		return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

int lcd_kit_get_pt_mode(int panel_id)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	int status = 0;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return 0;
	}
	if (lcd_ops->get_pt_station_status)
		status = lcd_ops->get_pt_station_status(panel_id);
	LCD_KIT_INFO("[pt_mode] get status %d\n", status);
	return status;
}

bool lcd_kit_get_thp_afe_status(struct timespec64 *record_tv)
{
	struct ts_kit_ops *ts_ops = NULL;
	bool thp_afe_status = true;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return thp_afe_status;
	}
	if (ts_ops->get_afe_status) {
		thp_afe_status = ts_ops->get_afe_status(record_tv);
		LCD_KIT_INFO("get afe status %d\n", thp_afe_status);
	}
	return thp_afe_status;
}

static int lcd_kit_get_proxmity_status(int panel_id, int data)
{
	struct ts_kit_ops *ts_ops = NULL;
	static bool ts_get_proxmity_flag = true;

	if (!common_info->thp_proximity.support) {
		LCD_KIT_INFO("[Proximity_feature] not support\n");
		return TP_PROXMITY_DISABLE;
	}
	if (data) {
		ts_get_proxmity_flag = true;
		LCD_KIT_INFO("[Proximity_feature] get status %d\n",
			common_info->thp_proximity.work_status);
		return common_info->thp_proximity.work_status;
	}
	if (ts_get_proxmity_flag == false) {
		LCD_KIT_INFO("[Proximity_feature] get status %d\n",
			common_info->thp_proximity.work_status);
		return common_info->thp_proximity.work_status;
	}
	ts_ops = ts_kit_get_ops();
	if (!ts_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return TP_PROXMITY_DISABLE;
	}
	if (ts_ops->get_tp_proxmity) {
		common_info->thp_proximity.work_status = (int)ts_ops->get_tp_proxmity();
		LCD_KIT_INFO("[Proximity_feature] get status %d\n",
				common_info->thp_proximity.work_status);
	}
	ts_get_proxmity_flag = false;
	return common_info->thp_proximity.work_status;
}

static int lcd_kit_gesture_mode(void)
{
	struct ts_kit_ops *ts_ops = NULL;
	int status = 0;
	int ret;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return 0;
	}
	if (ts_ops->get_tp_status_by_type) {
		ret = ts_ops->get_tp_status_by_type(TS_GESTURE_FUNCTION, &status);
		if (ret) {
			LCD_KIT_INFO("get gesture function fail\n");
			return 0;
		}
	}
	LCD_KIT_INFO("[gesture_mode] get status %d\n", status);
	return status;
}

static int lcd_kit_panel_is_power_on(int panel_id)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	int mode = 0;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return 0;
	}
	if (lcd_ops->get_panel_power_status)
		mode = lcd_ops->get_panel_power_status(panel_id);
	return mode;
}

static bool lcd_kit_event_skip_delay(int panel_id, void *hld,
	uint32_t event, uint32_t data)
{
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return false;
	}
	if (lcd_ops->panel_event_skip_delay)
		return lcd_ops->panel_event_skip_delay(panel_id, hld, event, data);
	return false;
}

static int lcd_kit_dvdd_hpm_ctrl(int panel_id)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	int ret = LCD_KIT_OK;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return 0;
	}

	if (lcd_ops->dvdd_hpm_ctrl)
		ret = lcd_ops->dvdd_hpm_ctrl();
	return ret;
}

static int lcd_kit_dvdd_vol_ctrl(int panel_id, void *hld)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	int ret = LCD_KIT_OK;
	if (!common_info->avs_dvdd.lcd_avs_support) {
		LCD_KIT_ERR("not support avs!\n");
		return LCD_KIT_FAIL;
	}
	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return LCD_KIT_FAIL;
	}
 
	if (lcd_ops->dvdd_avs_ctrl)
		ret = lcd_ops->dvdd_avs_ctrl(panel_id, hld);
	return ret;
}

static int lcd_kit_avdd_mipi_ctrl(int panel_id, void *hld, int enable)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	int ret = LCD_KIT_OK;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return 0;
	}
	if (lcd_ops->avdd_mipi_ctrl)
		ret = lcd_ops->avdd_mipi_ctrl(hld, enable);
	return ret;
}

static bool lcd_kit_event_skip_send(int panel_id, void *hld, uint32_t event,
	uint32_t enable)
{
	struct lcd_kit_ops *lcd_ops = lcd_kit_get_ops();

	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return false;
	}
	if (!lcd_ops->event_skip_send)
		return false;
	if (lcd_ops->event_skip_send(panel_id, hld, event, enable)) {
		LCD_KIT_INFO("skip power off!\n");
		return true;
	}
	return false;
}

static int lcd_kit_gesture_event_handler(int panel_id, uint32_t event, uint32_t data, uint32_t delay)
{
	int ret = LCD_KIT_OK;
	int type = 0;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("gesture event = %d, data = %d, delay = %d\n", event, data, delay);
	switch (event) {
	case EVENT_VSP:
		type = LCD_KIT_VSP;
		break;
	case EVENT_VSN:
		type = LCD_KIT_VSN;
		break;
	case EVENT_RESET:
		type = LCD_KIT_RST;
		break;
	case EVENT_EARLY_TS:
		type = LCD_KIT_TP_RST;
		break;
	default:
		return LCD_KIT_FAIL;
	}

	if (data == 0)
		adapt_ops->gpio_disable(panel_id, type);
	else
		adapt_ops->gpio_enable(panel_id, type);

	lcd_kit_delay(delay, LCD_KIT_WAIT_MS, true);
	return ret;
}

static int lcd_kit_ft_gesture_mode_power_on(int panel_id)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	struct lcd_kit_array_data *gesture_pevent = NULL;

	if (common_info->tp_gesture_sequence_flag && lcd_kit_gesture_mode()) {
		/* FT8201 gesture mode power on timing */
		gesture_pevent = power_seq->gesture_power_on_seq.arry_data;
		for (i = 0; i < power_seq->gesture_power_on_seq.cnt; i++) {
			if (!gesture_pevent || !gesture_pevent->buf) {
				LCD_KIT_ERR("gesture_pevent is null!\n");
				return LCD_KIT_FAIL;
			}
			ret = lcd_kit_gesture_event_handler(panel_id, gesture_pevent->buf[EVENT_NUM],
				gesture_pevent->buf[EVENT_DATA], gesture_pevent->buf[EVENT_DELAY]);
			if (ret) {
				LCD_KIT_ERR("send gesture_pevent 0x%x not exist!\n",
					gesture_pevent->buf[EVENT_NUM]);
				break;
			}
			gesture_pevent++;
		}
		return true;
	}
	return ret;
}

static int lcd_kit_event_should_send(int panel_id, void *hld, uint32_t event, uint32_t data)
{
	int ret = 0;

	if (lcd_kit_event_skip_send(panel_id, hld, event, data))
		return SKIP_SEND_EVENT;

	if ((event == EVENT_IOVCC) && (data != 0)) {
		ret = lcd_kit_ft_gesture_mode_power_on(panel_id);
		if (ret) {
			LCD_KIT_INFO("It is in gesture mode\n");
			return ret;
		}
	}

	switch (event) {
	case EVENT_VCI:
	case EVENT_IOVCC:
	case EVENT_VSP:
	case EVENT_VSN:
	case EVENT_VDD:
	case EVENT_BIAS:
	case EVENT_PMIC_CTRL:
	case EVENT_PMIC_INIT:
	case EVENT_PMIC_RESET:
	case EVENT_BOOST_ENABLE:
	case EVENT_BOOST_CTRL:
	case EVENT_BOOST_BYPASS:
		return (lcd_kit_get_pt_mode(panel_id) || lcd_kit_get_proxmity_status(panel_id, data) ||
			((uint32_t)lcd_kit_gesture_mode() && (common_info->ul_does_lcd_poweron_tp)));
	case EVENT_RESET:
		if (data && common_info->panel_on_always_need_reset) {
			return ret;
		} else {
			return (lcd_kit_get_pt_mode(panel_id) || lcd_kit_get_proxmity_status(panel_id, data) ||
				((uint32_t)lcd_kit_gesture_mode() && (common_info->ul_does_lcd_poweron_tp)));
		}
	case EVENT_MIPI:
		if (data)
			return lcd_kit_panel_is_power_on(panel_id);
		break;
	case EVENT_AOD_MIPI:
		if (data)
			return !lcd_kit_panel_is_power_on(panel_id);
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}

int lcd_kit_mipi_check_status(int panel_id, void *hld, uint32_t event)
{
	int ret_check_reg = LCD_KIT_OK;

	if (event == EVENT_MIPI) {
		ret_check_reg = lcd_kit_check_reg_report_dsm(panel_id, hld,
			&common_info->check_reg_on);
		if (ret_check_reg != LCD_KIT_OK)
			LCD_KIT_ERR("power on check reg error!\n");
	}
	return ret_check_reg;
}

static int lcd_kit_color_calibration(int panel_id, uint32_t data)
{
	int ret = LCD_KIT_OK;

	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	LCD_KIT_INFO("enter\n");

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops->mipi_tx) {
		LCD_KIT_ERR("no mipi tx!\n");
		return LCD_KIT_FAIL;
	}

	if (!common_info->c_calib.support || !common_info->c_calib.need_color_calib) {
		LCD_KIT_INFO("color calib not support\n");
		return LCD_KIT_OK;
	}

	LCD_KIT_INFO("set color_calib commands\n");
	ret = adapt_ops->mipi_tx(panel_id, NULL, &common_info->c_calib.set_cmd);

	return ret;
}

static int lcd_kit_power_set(int panel_id, uint32_t data)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	char read_nve_value[LCD_NV_DATA_SIZE] = {0};

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops->mipi_tx) {
		LCD_KIT_ERR("no mipi tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!adapt_ops->read_nv_info) {
		LCD_KIT_ERR("no read nv info!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->read_nv_info(panel_id, LCD_NV_NUM, sizeof(read_nve_value),
		read_nve_value, sizeof(read_nve_value));
	if (ret) {
		LCD_KIT_ERR("read nv info fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("nv info: 0x%x\n", read_nve_value[SET_POWER_BY_NV_BYTE]);
	if (read_nve_value[SET_POWER_BY_NV_BYTE] == NO_NEED_SET_POWER_BY_NV) {
		LCD_KIT_INFO("no need set power\n");
		return LCD_KIT_OK;
	}

	LCD_KIT_INFO("send power set commands\n");
	ret = adapt_ops->mipi_tx(panel_id, NULL, &common_info->set_power_by_nv.cmds);

	return ret;
}

static int lcd_kit_aod_ext_cmd(int panel_id, uint32_t data)
{
	LCD_KIT_INFO("aod event, do nothing\n");
	return LCD_KIT_OK;
}

int lcd_kit_event_handler(int panel_id, void *hld,
		uint32_t event, uint32_t data, uint32_t delay)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("event = %d, data = %d, delay = %d\n", event, data, delay);
	if (lcd_kit_event_should_send(panel_id, hld, event, data)) {
		LCD_KIT_INFO("It is in pt mode or gesture mode\n");
		if (lcd_kit_mipi_check_status(panel_id, hld, event))
			LCD_KIT_ERR("lcd check 0A error\n");
		return ret;
	}
	switch (event) {
	case EVENT_VCI:
		ret = lcd_kit_vci_power_ctrl(panel_id, data);
		break;
	case EVENT_IOVCC:
		ret = lcd_kit_iovcc_power_ctrl(panel_id, data);
		break;
	case EVENT_VSP:
		ret = lcd_kit_vsp_power_ctrl(panel_id, data);
		break;
	case EVENT_VSN:
		ret = lcd_kit_vsn_power_ctrl(panel_id, data);
		break;
	case EVENT_RESET:
		ret = lcd_kit_reset_power_ctrl(panel_id, data);
		break;
	case EVENT_MIPI:
		ret = lcd_kit_mipi_power_ctrl(panel_id, hld, data);
		break;
	case EVENT_EARLY_TS:
		lcd_kit_early_ts_event(panel_id, data, delay);
		break;
	case EVENT_LATER_TS:
		lcd_kit_later_ts_event(panel_id, data, delay);
		break;
	case EVENT_VDD:
		ret = lcd_kit_vdd_power_ctrl(panel_id, data);
		break;
	case EVENT_AOD:
		ret = lcd_kit_aod_power_ctrl(panel_id, data);
		break;
	case EVENT_NONE:
		LCD_KIT_INFO("none event\n");
		break;
	case EVENT_BIAS:
		lcd_kit_set_bias_ctrl(data);
		break;
	case EVENT_AOD_MIPI:
		lcd_kit_aod_mipi_ctrl(panel_id, hld, data);
		break;
	case EVENT_MIPI_SWITCH:
		ret = lcd_kit_set_mipi_switch_ctrl(panel_id, data);
		break;
	case EVENT_AVDD_MIPI:
		lcd_kit_avdd_mipi_ctrl(panel_id, hld, data);
		break;
	case EVENT_2ND_POWER_OFF_TS:
		lcd_kit_2nd_power_off_ts_event(panel_id, data, delay);
		break;
	case EVENT_BLOCK_TS:
		lcd_kit_block_ts_event(data, delay);
		break;
	case EVENT_PMIC_CTRL:
		ret = lcd_kit_pmic_ctrl_event(panel_id, data);
		break;
	case EVENT_PMIC_RESET:
		ret = lcd_kit_pmic_reset_event(panel_id, data);
		break;
	case EVENT_PMIC_INIT:
		ret = lcd_kit_pmic_init_event(panel_id, data);
		break;
	case EVENT_PROCESS_HPM:
		lcd_kit_dvdd_hpm_ctrl(panel_id);
		break;
	case EVENT_BOOST_ENABLE:
		ret = lcd_kit_boost_enable_event(panel_id, data);
		break;
	case EVENT_BOOST_CTRL:
		ret = lcd_kit_boost_ctrl_event(panel_id, data);
		break;
	case EVENT_BOOST_BYPASS:
		ret = lcd_kit_boost_bypass_event(panel_id, data);
		break;
	case EVENT_COLOR_CALIB:
		lcd_kit_color_calibration(panel_id, data);
		break;
	case EVENT_SET_POWER:
		lcd_kit_power_set(panel_id, data);
		break;
	case EVENT_AOD_EXT_CMD:
		lcd_kit_aod_ext_cmd(panel_id, data);
		break;
	case EVENT_BRIDGE:
		lcd_kit_bridge_ctrl_event(data);
		break;
	case EVENT_DVDD_AVS:
		lcd_kit_dvdd_vol_ctrl(panel_id, hld);
		break;
	case EVENT_PPC_ON:
		lcd_kit_ppc_on_cmds(panel_id, hld);
		break;
	case EVENT_EINK_WAKEUP:
		lcd_kit_eink_power_ctrl_event(data);
		break;
	default:
		LCD_KIT_INFO("event not exist\n");
		if (common_info->eink_lcd)
			return ret;
		break;
	}
	/* In the case of aod exit, no delay is required */
	if (!lcd_kit_event_skip_delay(panel_id, hld, event, data))
		lcd_kit_delay(delay, LCD_KIT_WAIT_MS, true);
	return ret;
}

static int lcd_kit_panel_power_on(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->power_on_seq.arry_data;
	for (i = 0; i < power_seq->power_on_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

static int lcd_kit_event_handler_part(int panel_id, void *hld, uint32_t event, uint32_t data,
	uint32_t delay, bool before_mipi)
{
	int ret = LCD_KIT_OK;

	switch (event) {
	case EVENT_VCI:
	case EVENT_IOVCC:
	case EVENT_VSP:
	case EVENT_VSN:
	case EVENT_VDD:
	case EVENT_BIAS:
		if (before_mipi)
			ret = lcd_kit_event_handler(panel_id, hld, event, data, delay);
		break;
	case EVENT_RESET:
	case EVENT_MIPI:
	case EVENT_EARLY_TS:
	case EVENT_LATER_TS:
	case EVENT_AOD:
	case EVENT_AOD_MIPI:
	case EVENT_MIPI_SWITCH:
	case EVENT_AVDD_MIPI:
	case EVENT_2ND_POWER_OFF_TS:
		if (before_mipi == false)
			ret = lcd_kit_event_handler(panel_id, hld, event, data, delay);
		break;
	case EVENT_NONE:
	default:
		ret = lcd_kit_event_handler(panel_id, hld, event, data, delay);
		break;
	}
	return ret;
}

static int lcd_kit_event_handler_with_mipi(int panel_id, void *hld, uint32_t event, uint32_t data,
	uint32_t delay)
{
	return lcd_kit_event_handler_part(panel_id, hld, event, data, delay, false);
}

static int lcd_kit_event_handler_without_mipi(int panel_id, void *hld, uint32_t event, uint32_t data,
	uint32_t delay)
{
	return lcd_kit_event_handler_part(panel_id, hld, event, data, delay, true);
}

static int lcd_kit_panel_power_on_with_mipi(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;
	pevent = power_seq->power_on_seq.arry_data;
	for (i = 0; i < power_seq->power_on_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler_with_mipi(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

static int lcd_kit_panel_power_on_without_mipi(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;
	pevent = power_seq->power_on_seq.arry_data;
	for (i = 0; i < power_seq->power_on_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler_without_mipi(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

static int lcd_kit_panel_mipi_switch(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_mipi_switch_seq.arry_data;
	for (i = 0; i < power_seq->panel_mipi_switch_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}

	return ret;
}

static int lcd_kit_panel_off_tp(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_off_tp_seq.arry_data;
	for (i = 0; i < power_seq->panel_off_tp_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}

	return ret;
}

static int lcd_kit_panel_on_tp(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_on_tp_seq.arry_data;
	for (i = 0; i < power_seq->panel_on_tp_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}

	return ret;
}

static int lcd_kit_panel_on_lp(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_on_lp_seq.arry_data;
	for (i = 0; i < power_seq->panel_on_lp_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

static int lcd_kit_get_temper_handler(int *cur_thermal)
{
	int ret = LCD_KIT_OK;
	struct thermal_zone_device *thermal_zone = NULL;

	if (!cur_thermal) {
		LCD_KIT_ERR("current thermal is null\n");
		return LCD_KIT_FAIL;
	}
	thermal_zone = thermal_zone_get_zone_by_name("shell_front");
	if (IS_ERR_OR_NULL(thermal_zone)) {
		LCD_KIT_ERR("Failed getting thermal zone!\n");
		return LCD_KIT_FAIL;
	}
	ret = thermal_zone_get_temp(thermal_zone, cur_thermal);
	if (ret) {
		LCD_KIT_ERR("get_temperature fail!!");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_set_temper_cmd_handler(int panel_id, int cur_thermal)
{
	int ret = LCD_KIT_OK;
	int thermal;
	uint32_t horaxis;
	uint32_t vertaxis;
	if (common_info->temper_thread.temper_position.buf == NULL &&
			common_info->temper_thread.temper_enter_cmd.cmds == NULL) {
		LCD_KIT_INFO("position buffer is null\n");
		return LCD_KIT_FAIL;
	}
	if (common_info->temper_thread.temper_position.cnt != TEMPER_POS_MAX) {
		LCD_KIT_ERR("horaxis or vertaxis is null\n");
		return LCD_KIT_FAIL;
	}
	horaxis = common_info->temper_thread.temper_position.buf[LCD_HOR_INDEX];
	vertaxis = common_info->temper_thread.temper_position.buf[LCD_VER_INDEX];
 
	thermal = (int)(cur_thermal / TMPER_ACCURACY);
	if (thermal > MAX_TEMPER) {
		common_info->temper_thread.temper_enter_cmd.cmds[vertaxis].payload[horaxis] = MAX_TEMPER - TMEPER_VALUE + TEMPER_CMD;
	} else {
		common_info->temper_thread.temper_enter_cmd.cmds[vertaxis].payload[horaxis] = thermal - TMEPER_VALUE + TEMPER_CMD;
	}
	LCD_KIT_INFO("thermal = %d, temper_enter_cmd.cmds[%d].payload[%d] = %x\n",
				thermal, vertaxis, horaxis, common_info->temper_thread.temper_enter_cmd.cmds[vertaxis].payload[horaxis]);
	return ret;
}

static void lcd_kit_is_open_temper(int panel_id, int cur_thermal)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
 
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return;
	}
	if (cur_thermal > MIN_TEMPER) {
		ret = lcd_kit_set_temper_cmd_handler(panel_id, cur_thermal);
		if (ret) {
			LCD_KIT_ERR("set temper cmd fail\n");
			return;
		}
		if (common_info->temper_thread.temper_enter_cmd.cmds)
			ret = adapt_ops->mipi_tx(panel_id, NULL,
				&common_info->temper_thread.temper_enter_cmd);
		if (ret) {
			LCD_KIT_ERR("mipi_tx error\n");
			return;
		}
	} else if (cur_thermal < MIN_TEMPER) {
		if (common_info->temper_thread.temper_exit_cmd.cmds)
			ret = adapt_ops->mipi_tx(panel_id, NULL,
				&common_info->temper_thread.temper_exit_cmd);
		if (ret) {
			LCD_KIT_ERR("mipi_tx error\n");
			return;
		}
	}
}

static void lcd_kit_start_temper_hrtimer(int panel_id)
{
	int ret = LCD_KIT_OK;
	int cur_thermal;

	if (!hrtimer_active(&common_info->temper_thread.hrtimer)) {
		hrtimer_start(&common_info->temper_thread.hrtimer,
			ktime_set(TEMPER_THREAD_TIME_PERIOD / 1000, // change ms to s
			(TEMPER_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
			HRTIMER_MODE_REL);
		ret = lcd_kit_get_temper_handler(&cur_thermal);
		if (ret) {
			LCD_KIT_ERR("get thermal is error!\n");
			return;
		}
		LCD_KIT_INFO("lcd temper timer is starting, cur_thermal = %d\n", cur_thermal);
		if (common_info->temper_thread.temper_lhbm_flag) {
			LCD_KIT_ERR("LHBM is running, skip temper\n");
			return;
		}
		lcd_kit_is_open_temper(panel_id, cur_thermal);
	}
}

static void lcd_kit_stop_temper_hrtimer(int panel_id)
{
	if (hrtimer_active(&common_info->temper_thread.hrtimer)) {
		hrtimer_cancel(&common_info->temper_thread.hrtimer);
		LCD_KIT_INFO("lcd temper timer is stoping!\n");
	}
}

static int lcd_kit_panel_on_hs(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_on_hs_seq.arry_data;
	for (i = 0; i < power_seq->panel_on_hs_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	/* restart check thread */
	if (common_info->check_thread.enable)
		hrtimer_start(&common_info->check_thread.hrtimer,
			ktime_set(CHECK_THREAD_TIME_PERIOD / 1000, // change ms to s
			(CHECK_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
			HRTIMER_MODE_REL);
	if (common_info->temper_thread.enable && common_info->temper_thread.dark_enable)
		lcd_kit_start_temper_hrtimer(panel_id);
	return ret;
}

int lcd_kit_panel_off_hs(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_off_hs_seq.arry_data;
	for (i = 0; i < power_seq->panel_off_hs_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	/* stop check thread */
	if (common_info->check_thread.enable)
		hrtimer_cancel(&common_info->check_thread.hrtimer);
	return ret;
}

int lcd_kit_panel_off_lp(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_off_lp_seq.arry_data;
	for (i = 0; i < power_seq->panel_off_lp_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

int lcd_kit_panel_power_off(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->power_off_seq.arry_data;
	for (i = 0; i < power_seq->power_off_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	if (common_info->set_vss.support) {
		common_info->set_vss.power_off = 1;
		common_info->set_vss.new_backlight = 0;
	}
	if (common_info->set_power.support)
		common_info->set_power.get_thermal = 0;

	return ret;
}

int lcd_kit_panel_only_power_off(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->only_power_off_seq.arry_data;
	for (i = 0; i < power_seq->only_power_off_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(panel_id, hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

static int lcd_kit_hbm_enable(int panel_id, void *hld, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* enable hbm and open dimming */
	if (common_info->dfr_info.fps_lock_command_support &&
		(fps_status == LCD_KIT_FPS_HIGH)) {
		ret = adapt_ops->mipi_tx(panel_id, hld,
			&common_info->dfr_info.cmds[FPS_90_HBM_DIM]);
	} else if (common_info->hbm.enter_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.enter_cmds);
	}
	return ret;
}

static int lcd_kit_enter_fp_hbm_extern(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL || adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* enable fp hbm */
	if (common_info->hbm.fp_enter_extern_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.fp_enter_extern_cmds);
		LCD_KIT_INFO("fp hbm enter extern cmd already send, ret = %d!\n", ret);
	}
	return ret;
}

static int lcd_kit_exit_fp_hbm_extern(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL || adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* recover when exit fp hbm */
	if (common_info->hbm.fp_exit_extern_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.fp_exit_extern_cmds);
		LCD_KIT_INFO("fp hbm exit extern cmd already send, ret = %d!\n", ret);
	}
	return ret;
}

void lcd_kit_hbm_fps_handler(int panel_id, int level)
{
	/* Skip unsmooth hbm levels */
	LCD_KIT_DEBUG("level_in=%d!\n", level);
	level = (level > (int)common_info->hbm.hbm_level_skip_low &&
		level < (int)common_info->hbm.hbm_level_skip_high) ?
		(int)common_info->hbm.hbm_level_skip_low : level;
	LCD_KIT_DEBUG("level_out=%d!\n", level);
	/* Set hbm level cmd for 120Hz */
	common_info->hbm.hbm_cmds.cmds[3].payload[1] = ((unsigned int)level >> 8) & 0xf;
	common_info->hbm.hbm_cmds.cmds[3].payload[2] = (unsigned int)level & 0xff;
	/* Set hbm level cmd for 60Hz, hbm level = level << base - bias */
	common_info->hbm.hbm_cmds.cmds[1].payload[1] = ((((unsigned int)level <<
		common_info->hbm.hbm_level_base) - (common_info->hbm.hbm_level_bias <<
		common_info->hbm.hbm_level_base)) >> 8) & 0xf;
	common_info->hbm.hbm_cmds.cmds[1].payload[2] = (((unsigned int)level <<
		common_info->hbm.hbm_level_base) - (common_info->hbm.hbm_level_bias <<
		common_info->hbm.hbm_level_base)) & 0xff;
}

static int lcd_kit_hbm_disable(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* exit hbm */
	if (common_info->hbm.exit_cmds.cmds != NULL)
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.exit_cmds);
	return ret;
}

static int lcd_kit_hbm_set_level(int panel_id, void *hld, int level)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* prepare */
	if (common_info->hbm.hbm_prepare_cmds.cmds != NULL)
		ret = adapt_ops->mipi_tx(panel_id, hld,
			&common_info->hbm.hbm_prepare_cmds);
	/* set hbm level */
	if (common_info->hbm.hbm_cmds.cmds != NULL) {
		if (common_info->hbm.hbm_special_bit_ctrl ==
			LCD_KIT_HIGH_12BIT_CTL_HBM_SUPPORT) {
			/* Set high 12bit hbm level, low 4bit set zero */
			common_info->hbm.hbm_cmds.cmds[0].payload[1] =
				(level >> LCD_KIT_SHIFT_FOUR_BIT) & 0xff;
			common_info->hbm.hbm_cmds.cmds[0].payload[2] =
				(level << LCD_KIT_SHIFT_FOUR_BIT) & 0xf0;
		} else if (common_info->hbm.hbm_special_bit_ctrl ==
					LCD_KIT_8BIT_CTL_HBM_SUPPORT) {
			common_info->hbm.hbm_cmds.cmds[0].payload[1] = level & 0xff;
		} else if (common_info->hbm.hbm_fps_command_support) {
			/* Send different hbm levels at different freq and skip
			unsmooth hbm levels for wagner product */
			lcd_kit_hbm_fps_handler(panel_id, level);
		} else {
			/* change bl level to dsi cmds */
			common_info->hbm.hbm_cmds.cmds[0].payload[1] =
				(level >> 8) & 0xf;
			common_info->hbm.hbm_cmds.cmds[0].payload[2] =
				level & 0xff;
		}
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.hbm_cmds);
	}
	/* post */
	if (common_info->hbm.hbm_post_cmds.cmds != NULL)
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.hbm_post_cmds);
	return ret;
}

static int lcd_kit_hbm_dim_disable(int panel_id, void *hld, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if ((fps_status == LCD_KIT_FPS_HIGH) &&
		common_info->dfr_info.fps_lock_command_support) {
		ret = adapt_ops->mipi_tx(panel_id, hld,
			&common_info->dfr_info.cmds[FPS_90_NORMAL_NO_DIM]);
	} else if (common_info->hbm.exit_dim_cmds.cmds != NULL) {
		/* close dimming */
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.exit_dim_cmds);
	}
	return ret;
}

static int lcd_kit_hbm_dim_enable(int panel_id, void *hld, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* open dimming exit hbm */
	if ((fps_status == LCD_KIT_FPS_HIGH) &&
		common_info->dfr_info.fps_lock_command_support) {
		ret = adapt_ops->mipi_tx(panel_id, hld,
			&common_info->dfr_info.cmds[FPS_90_NORMAL_DIM]);
	} else if (common_info->hbm.enter_dim_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.enter_dim_cmds);
	}
	return ret;
}

static int lcd_kit_hbm_enable_no_dimming(int panel_id, void *hld, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* enable hbm and close dimming */
	if ((fps_status == LCD_KIT_FPS_HIGH) &&
		common_info->dfr_info.fps_lock_command_support) {
		ret = adapt_ops->mipi_tx(panel_id, hld,
			&common_info->dfr_info.cmds[FPS_90_HBM_NO_DIM]);
	} else if (common_info->hbm.enter_no_dim_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, hld,
			&common_info->hbm.enter_no_dim_cmds);
	}
	return ret;
}

static void lcd_kit_hbm_print_count(int last_hbm_level, int hbm_level)
{
	static int count;
	int level_delta = 60;

	if (abs(hbm_level - last_hbm_level) > level_delta) {
		if (count == 0)
			LCD_KIT_INFO("last hbm_level=%d!\n", last_hbm_level);
		count = 5;
	}
	if (count > 0) {
		count--;
		LCD_KIT_INFO("hbm_level=%d!\n", hbm_level);
	} else {
		LCD_KIT_DEBUG("hbm_level=%d!\n", hbm_level);
	}
}

static int check_if_fp_using_hbm(int panel_id)
{
	int ret = LCD_KIT_OK;

	if (common_info->hbm.hbm_fp_support) {
		if (common_info->hbm.hbm_if_fp_is_using)
			ret = LCD_KIT_FAIL;
	}

	return ret;
}

static int lcd_kit_hbm_set_handle(int panel_id, void *hld, int last_hbm_level,
	int hbm_dimming, int hbm_level, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if ((!hld) || (hbm_level < 0) || (hbm_level > HBM_SET_MAX_LEVEL)) {
		LCD_KIT_ERR("input param invalid, hbm_level %d!\n", hbm_level);
		return LCD_KIT_FAIL;
	}
	if (!common_info->hbm.support) {
		LCD_KIT_DEBUG("not support hbm\n");
		return ret;
	}
	mutex_lock(&COMMON_LOCK->hbm_lock);
	common_info->hbm.hbm_level_current = hbm_level;
	if (check_if_fp_using_hbm(panel_id) < 0) {
		LCD_KIT_INFO("fp is using, exit!\n");
		mutex_unlock(&COMMON_LOCK->hbm_lock);
		return ret;
	}
	if (hbm_level > 0) {
		if (last_hbm_level == 0) {
			/* enable hbm */
			lcd_kit_hbm_enable(panel_id, hld, fps_status);
			if (!hbm_dimming)
				lcd_kit_hbm_enable_no_dimming(panel_id, hld, fps_status);
		} else {
			lcd_kit_hbm_print_count(last_hbm_level, hbm_level);
		}
		 /* set hbm level */
		lcd_kit_hbm_set_level(panel_id, hld, hbm_level);
	} else {
		if (last_hbm_level == 0) {
			/* disable dimming */
			lcd_kit_hbm_dim_disable(panel_id, hld, fps_status);
		} else {
			/* exit hbm */
			if (hbm_dimming)
				lcd_kit_hbm_dim_enable(panel_id, hld, fps_status);
			else
				lcd_kit_hbm_dim_disable(panel_id, hld, fps_status);
			lcd_kit_hbm_disable(panel_id, hld);
		}
	}
	mutex_unlock(&COMMON_LOCK->hbm_lock);
	return ret;
}

static int lcd_kit_get_panel_name(int panel_id, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", common_info->panel_name);
}

static u32 lcd_kit_get_blmaxnit(int panel_id)
{
	u32 bl_max_nit = 0;
	u32 lcd_kit_brightness_ddic_info;

	lcd_kit_brightness_ddic_info =
		common_info->blmaxnit.lcd_kit_brightness_ddic_info;
	if (common_info->blmaxnit.get_blmaxnit_type == GET_BLMAXNIT_FROM_DDIC &&
		lcd_kit_brightness_ddic_info > BL_MIN &&
		lcd_kit_brightness_ddic_info < BL_MAX) {
		if (lcd_kit_brightness_ddic_info < BL_REG_NOUSE_VALUE)
			bl_max_nit = lcd_kit_brightness_ddic_info +
				common_info->bl_max_nit_min_value;
		else
			bl_max_nit = lcd_kit_brightness_ddic_info +
				common_info->bl_max_nit_min_value - 1;
	} else {
		bl_max_nit = common_info->actual_bl_max_nit;
	}
	return bl_max_nit;
}

static void get_panel_type(int panel_id, char *panel_type, int len)
{
	if (common_info->panel_type == LCD_TYPE)
		strncpy(panel_type, "LCD", strlen("LCD"));
	else if (common_info->panel_type == AMOLED_TYPE)
		strncpy(panel_type, "AMOLED", strlen("AMOLED"));
	else
		strncpy(panel_type, "INVALID", strlen("INVALID"));
}

void get_oled_type(int panel_id, char *oled_type, int len)
{
	if (common_info->panel_type != AMOLED_TYPE) {
		strncpy(oled_type, "LTPS", strlen("LTPS"));
		return;
	}
	if (common_info->oled_type == LTPS)
		strncpy(oled_type, "LTPS", strlen("LTPS"));
	else if (common_info->oled_type == LTPO &&
		common_info->ltpo_ver == LTPO_V1)
		strncpy(oled_type, "LTPO1", strlen("LTPO1"));
	else if (common_info->oled_type == LTPO &&
		common_info->ltpo_ver == LTPO_V2)
		strncpy(oled_type, "LTPO2", strlen("LTPO2"));
	else
		strncpy(oled_type, "LTPS", strlen("LTPS"));
}

static int lcd_kit_get_panel_info(int panel_id, char *buf)
{
#define PANEL_MAX 10
	int ret;
	char panel_type[PANEL_MAX] = {0};
	struct lcd_kit_bl_ops *bl_ops = NULL;
	char *bl_type = " ";
	char oled_type[PANEL_MAX] = {0};

	get_panel_type(panel_id, panel_type, PANEL_MAX);
	get_oled_type(panel_id, oled_type, PANEL_MAX);
	common_info->actual_bl_max_nit = lcd_kit_get_blmaxnit(panel_id);
	bl_ops = lcd_kit_get_bl_ops();
	if ((bl_ops != NULL) && (bl_ops->name != NULL))
		bl_type = bl_ops->name;
	ret = snprintf(buf, PAGE_SIZE,
		"blmax:%u,blmin:%u,blmax_nit_actual:%d,blmax_nit_standard:%d,lcdtype:%s,bl_type:%s,oled_type:%s\n",
		common_info->bl_level_max, common_info->bl_level_min,
		common_info->actual_bl_max_nit, common_info->bl_max_nit,
		panel_type, bl_type, oled_type);
	return ret;
}

static int lcd_kit_get_cabc_mode(int panel_id, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_info->cabc.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n", common_info->cabc.mode);
	return ret;
}

static int lcd_kit_set_cabc_mode(int panel_id, void *hld, u32 mode)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->cabc.support) {
		LCD_KIT_DEBUG("not support cabc\n");
		return ret;
	}
	switch (mode) {
	case CABC_OFF_MODE:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->cabc.cabc_off_cmds);
		break;
	case CABC_UI:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->cabc.cabc_ui_cmds);
		break;
	case CABC_STILL:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->cabc.cabc_still_cmds);
		break;
	case CABC_MOVING:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->cabc.cabc_moving_cmds);
		break;
	default:
		return LCD_KIT_FAIL;
	}
	common_info->cabc.mode = mode;
	LCD_KIT_INFO("cabc.support = %d,cabc.mode = %d\n",
		common_info->cabc.support, common_info->cabc.mode);
	return ret;
}

static void lcd_kit_clear_esd_error_info(void)
{
	memset(&g_esd_error_info, 0, sizeof(g_esd_error_info));
}
static void lcd_kit_record_esd_error_info(int read_reg_index, int read_reg_val,
	int expect_reg_val)
{
	int reg_index = g_esd_error_info.esd_error_reg_num;

	if ((reg_index + 1) <= MAX_REG_READ_COUNT) {
		g_esd_error_info.esd_reg_index[reg_index] = read_reg_index;
		g_esd_error_info.esd_error_reg_val[reg_index] = read_reg_val;
		g_esd_error_info.esd_expect_reg_val[reg_index] = expect_reg_val;
		g_esd_error_info.esd_error_reg_num++;
	}
}

int lcd_kit_judge_esd(unsigned char type, unsigned char read_val,
	unsigned char expect_val)
{
	int ret = 0;

	switch (type) {
	case ESD_UNEQUAL:
		if (read_val != expect_val)
			ret = 1;
		break;
	case ESD_EQUAL:
		if (read_val == expect_val)
			ret = 1;
		break;
	case ESD_BIT_VALID:
		if (read_val & expect_val)
			ret = 1;
		break;
	default:
		if (read_val != expect_val)
			ret = 1;
		break;
	}
	return ret;
}

int32_t lcd_kit_get_gpio_value(u32 gpio_num, const char *gpio_name)
{
	int32_t gpio_value;
	int32_t ret;

	ret = gpio_request(gpio_num, gpio_name);
	if (ret != 0) {
		LCD_KIT_ERR("esd_detect_gpio[%d] request fail!\n", gpio_num);
		return LCD_KIT_FAIL;
	}
	ret = gpio_direction_input(gpio_num);
	if (ret != 0) {
		gpio_free(gpio_num);
		LCD_KIT_ERR("esd_detect_gpio[%d] direction set fail!\n", gpio_num);
		return LCD_KIT_FAIL;
	}
	gpio_value = gpio_get_value(gpio_num);
	gpio_free(gpio_num);

	LCD_KIT_INFO("gpio_num:%d value:%d\n", gpio_num, gpio_value);
	return gpio_value;
}

static int lcd_kit_gpio_detect_event(int panel_id)
{
	int ret = LCD_KIT_OK;
	int i;
	u32 gpio_detect_value;
	struct ts_kit_ops *ts_ops = NULL;
	bool gpio_detect_state = true;

	if (common_info->esd.tp_esd_event) {
		ts_ops = ts_kit_get_ops();
		if (!ts_ops || !ts_ops->send_esd_event) {
			LCD_KIT_ERR("ts_ops or send_esd_event is null\n");
			return LCD_KIT_FAIL;
		}
	}
	for (i = 0; i < GPIO_CHECK_TIMES; i++) {
		gpio_detect_value = (u32)lcd_kit_get_gpio_value(
			common_info->esd.gpio_detect_num, "esd_detect_gpio");
		if (common_info->esd.gpio_double_detect)
			gpio_detect_state = ((u32)lcd_kit_get_gpio_value(common_info->esd.gpio_detect_num1, "esd_detect_gpio1")) ==
				common_info->esd.gpio_normal_value1;
		
		if (gpio_detect_state && (gpio_detect_value == common_info->esd.gpio_normal_value)) {
			break;
		} else if (!common_info->esd.tp_esd_event) {
			return LCD_KIT_FAIL;
		} else if (i == (GPIO_CHECK_TIMES - 1)) {
			ret = ts_ops->send_esd_event(gpio_detect_value);
			if (ret)
				LCD_KIT_ERR("ts_ops->send_esd_event faile\n");
			return LCD_KIT_FAIL;
		}
	}
	return LCD_KIT_OK;
}

static int lcd_kit_esd_handle(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	char read_value[MAX_REG_READ_COUNT] = {0};
	char expect_value, judge_type;
	u32 *esd_value = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int clear_esd_info_flag = false;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_rx! panel_id = %d\n", panel_id);
		return LCD_KIT_FAIL;
	}
	if (!common_info->esd.support) {
		LCD_KIT_DEBUG("not support esd, panel_id = %d\n", panel_id);
		return ret;
	}
	if (common_info->esd.status == ESD_STOP) {
		LCD_KIT_ERR("bypass esd check, panel_id = %d\n", panel_id);
		return LCD_KIT_OK;
	}
	if (common_info->esd.gpio_detect_support) {
		if (lcd_kit_gpio_detect_event(panel_id))
			return LCD_KIT_FAIL;

		/* esd detect gpio only */
		if (common_info->esd.gpio_detect_support == ESD_DETECT_GPIO_ONLY_MODE)
			return LCD_KIT_OK;
	}
	esd_value = common_info->esd.value.buf;
	ret = adapt_ops->mipi_rx(panel_id, hld, read_value, MAX_REG_READ_COUNT - 1,
		&common_info->esd.cmds);
	if (ret) {
		LCD_KIT_INFO("mipi_rx fail, panel_id = %d\n", panel_id);
		return ret;
	}
	for (i = 0; i < common_info->esd.value.cnt; i++) {
		judge_type = (esd_value[i] >> 8) & 0xFF;
		expect_value = esd_value[i] & 0xFF;
		if (lcd_kit_judge_esd(judge_type, read_value[i], expect_value)) {
			if (clear_esd_info_flag == false) {
				lcd_kit_clear_esd_error_info();
				clear_esd_info_flag = true;
			}
			lcd_kit_record_esd_error_info(i, (int)read_value[i],
				expect_value);
			LCD_KIT_ERR("read_value[%d] = 0x%x, but expect_value = 0x%x!\n",
				i, read_value[i], expect_value);
			ret = 1;
			break;
		}
		LCD_KIT_INFO("judge_type = %d, esd_value[%d] = 0x%x, read_value[%d] = 0x%x, expect_value = 0x%x\n",
			judge_type, i, esd_value[i], i, read_value[i], expect_value);
	}
	LCD_KIT_INFO("esd check result:%d, panel_id = %d\n", ret, panel_id);
	return ret;
}

static int lcd_kit_dsi_handle(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	char expect_value;
	u32 *dsi_value = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	char read_value[MAX_REG_READ_COUNT] = {0};

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_rx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->dsi.support) {
		LCD_KIT_DEBUG("not support dsi\n");
		return ret;
	}

	dsi_value = common_info->dsi.value.buf;
	ret = adapt_ops->mipi_rx(panel_id, hld, read_value, MAX_REG_READ_COUNT - 1,
		&common_info->dsi.cmds);
	if (ret) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return ret;
	}

	for (i = 0; i < common_info->dsi.value.cnt; i++) {
		expect_value = dsi_value[i] & 0xFF;
		if (read_value[i] != expect_value) {
			LCD_KIT_ERR("read_value[%d] = 0x%x, expect_value = 0x%x\n",
				i, read_value[i], expect_value);
			ret = snprintf(record_buf_dsi, RECORD_BUFLEN_DSI,
				"dsi read_value = 0x%x, 0x%x", read_value[0], read_value[1]);
			if (ret < 0) {
				LCD_KIT_ERR("snprintf happened error!\n");
				continue;
			}
#ifdef CONFIG_HUAWEI_DSM
			(void)lcd_dsm_client_record(lcd_dclient, record_buf_dsi,
				DSM_DSI_DETECT_ERROR_NO, REC_DMD_NO_LIMIT_DSI, &cur_rec_time_dsi);
#endif
			ret = 1;
			break;
		}
		LCD_KIT_INFO("dsi_value[%d] = 0x%x, read_value = 0x%x\n",
			i, dsi_value[i], read_value[i]);
	}
	LCD_KIT_INFO("dsi check result:%d\n", ret);
	return ret;
}

static int lcd_kit_get_ce_mode(int panel_id, char *buf)
{
	int ret = LCD_KIT_OK;

	if (buf == NULL) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	if (common_info->ce.support)
		ret = snprintf(buf, PAGE_SIZE,  "%d\n", common_info->ce.mode);
	return ret;
}

static int lcd_kit_set_ce_mode(int panel_id, void *hld, u32 mode)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->ce.support) {
		LCD_KIT_DEBUG("not support ce\n");
		return ret;
	}
	switch (mode) {
	case CE_OFF_MODE:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->ce.off_cmds);
		break;
	case CE_SRGB:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->ce.srgb_cmds);
		break;
	case CE_USER:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->ce.user_cmds);
		break;
	case CE_VIVID:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->ce.vivid_cmds);
		break;
	default:
		LCD_KIT_INFO("wrong mode!\n");
		ret = LCD_KIT_FAIL;
		break;
	}
	common_info->ce.mode = mode;
	LCD_KIT_INFO("ce.support = %d,ce.mode = %d\n", common_info->ce.support,
		common_info->ce.mode);
	return ret;
}

static int lcd_kit_get_acl_mode(int panel_id, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_info->acl.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n", common_info->acl.mode);
	return ret;
}

static int lcd_kit_set_acl_mode(int panel_id, void *hld, u32 mode)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->acl.support) {
		LCD_KIT_DEBUG("not support acl\n");
		return ret;
	}
	switch (mode) {
	case ACL_OFF_MODE:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->acl.acl_off_cmds);
		break;
	case ACL_HIGH_MODE:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->acl.acl_high_cmds);
		break;
	case ACL_MIDDLE_MODE:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->acl.acl_middle_cmds);
		break;
	case ACL_LOW_MODE:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->acl.acl_low_cmds);
		break;
	default:
		LCD_KIT_ERR("mode error\n");
		ret = LCD_KIT_FAIL;
		break;
	}
	common_info->acl.mode = mode;
	LCD_KIT_ERR("acl.support = %d,acl.mode = %d\n",
		common_info->acl.support, common_info->acl.mode);
	return ret;
}

static int lcd_kit_get_vr_mode(int panel_id, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_info->vr.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n", common_info->vr.mode);
	return ret;
}

static int lcd_kit_set_vr_mode(int panel_id, void *hld, u32 mode)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->vr.support) {
		LCD_KIT_DEBUG("not support vr\n");
		return ret;
	}
	switch (mode) {
	case VR_ENABLE:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->vr.enable_cmds);
		break;
	case  VR_DISABLE:
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->vr.disable_cmds);
		break;
	default:
		ret = LCD_KIT_FAIL;
		LCD_KIT_ERR("mode error\n");
		break;
	}
	common_info->vr.mode = mode;
	LCD_KIT_INFO("vr.support = %d, vr.mode = %d\n", common_info->vr.support,
		common_info->vr.mode);
	return ret;
}

static int lcd_kit_get_effect_color_mode(int panel_id, char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_info->effect_color.support ||
		(common_info->effect_color.mode & BITS(31)))
		ret = snprintf(buf, PAGE_SIZE, "%d\n",
			common_info->effect_color.mode);
	return ret;
}

static int lcd_kit_set_effect_color_mode(int panel_id, u32 mode)
{
	int ret = LCD_KIT_OK;

	if (common_info->effect_color.support)
		common_info->effect_color.mode = mode;
	LCD_KIT_INFO("effect_color.support = %d, effect_color.mode = %d\n",
		common_info->effect_color.support,
		common_info->effect_color.mode);
	return ret;
}

static void lcd_kit_set_backlight_cmds(int panel_id, u32 level)
{
	switch (common_info->backlight.order) {
	case BL_BIG_ENDIAN:
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level;
		} else if (common_info->backlight.ext_flag) {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = common_info->backlight.write_offset;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = (level >> 8) & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[3] = level & 0xFF;
		} else {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = ((level >> 8) & 0xFF) | common_info->backlight.write_offset;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = level & 0xFF;
		}
		break;
	case BL_LITTLE_ENDIAN:
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level;
		} else if (common_info->backlight.ext_flag) {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = (level >> 8) & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[3] = common_info->backlight.write_offset;
		} else {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = ((level >> 8) & 0xFF) | common_info->backlight.write_offset;
		}
		break;
	default:
		LCD_KIT_ERR("not support order\n");
		break;
	}
}

static bool lcd_kit_need_refresh_frame(int panel_id, u32 level)
{
	if (common_info->backlight.bl_need_refresh_frame <= 0 ||
		(common_info->backlight.bl_refresh_cmds.cmds == NULL))
		return false;
	if (is_in_pwm_range && level > common_info->backlight.bl_need_refresh_frame) {
		is_in_pwm_range = false;
		return true;
	} else if (!is_in_pwm_range && level <= common_info->backlight.bl_need_refresh_frame) {
		is_in_pwm_range = true;
		return true;
	} else {
		return false;
	}
}

static int lcd_kit_set_mipi_backlight(int panel_id, void *hld, u32 level)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return 0;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (common_info->backlight.set_bit)
		level = level | SET_BL_BIT;
	lcd_kit_set_backlight_cmds(panel_id, level);
	if (common_info->set_vss.support) {
		common_info->set_vss.new_backlight = level;
		if (lcd_ops->set_vss_by_thermal)
			lcd_ops->set_vss_by_thermal();
	}
	if (common_info->backlight.need_prepare)
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->backlight.prepare);
	if (lcd_kit_need_refresh_frame(panel_id, level))
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->backlight.bl_refresh_cmds);
	ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->backlight.bl_cmd);
	if ((common_info->bl_set_delay > 0) && (level == 0)) {
		LCD_KIT_INFO("set backlight delay %dms while level = %d\n", common_info->bl_set_delay, level);
		lcd_kit_delay(common_info->bl_set_delay, LCD_KIT_WAIT_MS, true);
	}
#ifdef LV_GET_LCDBK_ON
	LCD_KIT_INFO("mipi bl_level = %d\n", level);
	mipi_level = level;
#endif
	return ret;
}

static int lcd_kit_dirty_region_handle(int panel_id, void *hld, struct region_rect *dirty)
{
	int ret = LCD_KIT_OK;
	struct region_rect *dirty_region = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (dirty == NULL) {
		LCD_KIT_ERR("dirty is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (common_info->dirty_region.support) {
		dirty_region = (struct region_rect *)dirty;
		/* change region to dsi cmds */
		common_info->dirty_region.cmds.cmds[0].payload[1] =
			((dirty_region->x) >> 8) & 0xff;
		common_info->dirty_region.cmds.cmds[0].payload[2] =
			(dirty_region->x) & 0xff;
		common_info->dirty_region.cmds.cmds[0].payload[3] =
			((dirty_region->x + dirty_region->w - 1) >> 8) & 0xff;
		common_info->dirty_region.cmds.cmds[0].payload[4] =
			(dirty_region->x + dirty_region->w - 1) & 0xff;
		common_info->dirty_region.cmds.cmds[1].payload[1] =
			((dirty_region->y) >> 8) & 0xff;
		common_info->dirty_region.cmds.cmds[1].payload[2] =
			(dirty_region->y) & 0xff;
		common_info->dirty_region.cmds.cmds[1].payload[3] =
			((dirty_region->y + dirty_region->h - 1) >> 8) & 0xff;
		common_info->dirty_region.cmds.cmds[1].payload[4] =
			(dirty_region->y + dirty_region->h - 1) & 0xff;
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->dirty_region.cmds);
	}
	return ret;
}

void lcd_hardware_reset(int panel_id)
{
	/* reset pull low */
	lcd_kit_reset_power_ctrl(panel_id, LCD_RESET_LOW);
	msleep(300);
	/* reset pull high */
	lcd_kit_reset_power_ctrl(panel_id, LCD_RESET_HIGH);
}

static void lcd_kit_panel_parse_effect(int panel_id, struct device_node *np)
{
	/* effect color */
	lcd_kit_parse_u32(np, "lcd-kit,effect-color-support",
		&common_info->effect_color.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,effect-color-mode",
		&common_info->effect_color.mode, 0);
	/* bl max level */
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max",
		&common_info->bl_level_max, 0);
	/* bl min level */
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-min",
		 &common_info->bl_level_min, 0);
	/* bl max nit */
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max-nit",
		&common_info->bl_max_nit, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-getblmaxnit-type",
		&common_info->blmaxnit.get_blmaxnit_type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,Does-lcd-poweron-tp",
		&common_info->ul_does_lcd_poweron_tp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,Tp-gesture-sequence-flag",
		&common_info->tp_gesture_sequence_flag, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-on-always-reset",
		&common_info->panel_on_always_need_reset, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-blmaxnit-min-value",
		&common_info->bl_max_nit_min_value, BL_NIT);
	lcd_kit_parse_u32(np, "lcd-kit,dbv-stat-support",
		&common_info->dbv_stat_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,min-hbm-dbv",
		&common_info->min_hbm_dbv, 0);
	/* backlight delay */
	lcd_kit_parse_u32(np, "lcd-kit,backlight-set-delay",
		&common_info->bl_set_delay, 0);

	/* get blmaxnit */
	if (common_info->blmaxnit.get_blmaxnit_type == GET_BLMAXNIT_FROM_DDIC)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-bl-maxnit-command",
			"lcd-kit,panel-bl-maxnit-command-state",
			&common_info->blmaxnit.bl_maxnit_cmds);
	/* cabc */
	lcd_kit_parse_u32(np, "lcd-kit,cabc-support",
		&common_info->cabc.support, 0);
	if (common_info->cabc.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cabc-off-cmds",
			"lcd-kit,cabc-off-cmds-state",
			&common_info->cabc.cabc_off_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cabc-ui-cmds",
			"lcd-kit,cabc-ui-cmds-state",
			&common_info->cabc.cabc_ui_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cabc-still-cmds",
			"lcd-kit,cabc-still-cmds-state",
			&common_info->cabc.cabc_still_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cabc-moving-cmds",
			"lcd-kit,cabc-moving-cmds-state",
			&common_info->cabc.cabc_moving_cmds);
	}
	/* ddic alpha */
	lcd_kit_parse_u32(np, "lcd-kit,alpha-support",
		&common_info->ddic_alpha.alpha_support, 0);
	if (common_info->ddic_alpha.alpha_support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-ddic-alpha-enter-cmds",
			"lcd-kit,local-fp-local-ddic-alpha-cmds-state",
			&common_info->ddic_alpha.enter_alpha_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-ddic-alpha-exit-cmds",
			"lcd-kit,local-fp-local-ddic-alpha-cmds-state",
			&common_info->ddic_alpha.exit_alpha_cmds);
	}
	/* alpha with enable flag */
	lcd_kit_parse_u32(np, "lcd-kit,alpha-with-enable-flag",
		&common_info->ddic_alpha.alpha_with_enable_flag, 0);
	/* force delta bl update support */
	lcd_kit_parse_u32(np, "lcd-kit,force-delta-bl-update-support",
		&common_info->force_delta_bl_update_support, 0);
	/* hbm */
	lcd_kit_parse_u32(np, "lcd-kit,hbm-support",
		&common_info->hbm.support, 0);
	if (common_info->hbm.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-enter-no-dim-cmds",
			"lcd-kit,hbm-enter-no-dim-cmds-state",
			&common_info->hbm.enter_no_dim_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-enter-cmds",
			"lcd-kit,hbm-enter-cmds-state",
			&common_info->hbm.enter_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fp-enter-extern-cmds",
			"lcd-kit,fp-enter-extern-cmds-state",
			&common_info->hbm.fp_enter_extern_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fp-exit-extern-cmds",
			"lcd-kit,fp-exit-extern-cmds-state",
			&common_info->hbm.fp_exit_extern_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds",
			"lcd-kit,hbm-prepare-cmds-state",
			&common_info->hbm.hbm_prepare_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds-fir",
			"lcd-kit,hbm-prepare-cmds-fir-state",
			&common_info->hbm.prepare_cmds_fir);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds-sec",
			"lcd-kit,hbm-prepare-cmds-sec-state",
			&common_info->hbm.prepare_cmds_sec);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds-thi",
			"lcd-kit,hbm-prepare-cmds-thi-state",
			&common_info->hbm.prepare_cmds_thi);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds-fou",
			"lcd-kit,hbm-prepare-cmds-fou-state",
			&common_info->hbm.prepare_cmds_fou);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-fir",
			"lcd-kit,hbm-exit-cmds-fir-state",
			&common_info->hbm.exit_cmds_fir);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-sec",
			"lcd-kit,hbm-exit-cmds-sec-state",
			&common_info->hbm.exit_cmds_sec);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-thi",
			"lcd-kit,hbm-exit-cmds-thi-state",
			&common_info->hbm.exit_cmds_thi);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-thi-new",
			"lcd-kit,hbm-exit-cmds-thi-new-state",
			&common_info->hbm.exit_cmds_thi_new);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-fou",
			"lcd-kit,hbm-exit-cmds-fou-state",
			&common_info->hbm.exit_cmds_fou);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-cmds",
			"lcd-kit,hbm-cmds-state",
			&common_info->hbm.hbm_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-post-cmds",
			"lcd-kit,hbm-post-cmds-state",
			&common_info->hbm.hbm_post_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds",
			"lcd-kit,hbm-exit-cmds-state",
			&common_info->hbm.exit_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,enter-dim-cmds",
			"lcd-kit,enter-dim-cmds-state",
			&common_info->hbm.enter_dim_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,exit-dim-cmds",
			"lcd-kit,exit-dim-cmds-state",
			&common_info->hbm.exit_dim_cmds);
		lcd_kit_parse_u32(np,
			"lcd-kit,hbm-special-bit-ctrl-support",
			&common_info->hbm.hbm_special_bit_ctrl, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,hbm-set-elvss-dim-lp",
			&common_info->hbm.hbm_set_elvss_dim_lp, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,hbm-elvss-dim-cmd-delay",
			&common_info->hbm.hbm_fp_elvss_cmd_delay, 0);
		lcd_kit_parse_dcs_cmds(np,
			"lcd-kit,panel-hbm-elvss-prepare-cmds",
			"lcd-kit,panel-hbm-elvss-prepare-cmds-state ",
			&common_info->hbm.elvss_prepare_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-hbm-elvss-read-cmds",
			"lcd-kit,panel-hbm-elvss-read-cmds-state",
			&common_info->hbm.elvss_read_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-hbm-elvss-write-cmds",
			"lcd-kit,panel-hbm-elvss-write-cmds-state",
			&common_info->hbm.elvss_write_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-hbm-elvss-post-cmds",
			"lcd-kit,panel-hbm-elvss-post-cmds-state",
			&common_info->hbm.elvss_post_cmds);
		lcd_kit_parse_u32(np, "lcd-kit,hbm-fp-support",
			&common_info->hbm.hbm_fp_support, 0);
		if (common_info->hbm.hbm_fp_support) {
			lcd_kit_parse_u32(np, "lcd-kit,hbm-level-max",
				&common_info->hbm.hbm_level_max, 0);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-fp-enter-cmds",
				"lcd-kit,hbm-fp-enter-cmds-state",
				&common_info->hbm.fp_enter_cmds);
			lcd_kit_parse_u32(np,
				"lcd-kit,hbm-elvss-dim-support",
				&common_info->hbm.hbm_fp_elvss_support, 0);
		}
		lcd_kit_parse_array_data(np, "lcd-kit,node-grayscale",
			&common_info->hbm.gamma_info.node_grayscale);

		lcd_kit_parse_u32(np, "lcd-kit,lcd-alpha-support",
			&common_info->hbm.local_hbm_support, 0);
		if (common_info->hbm.local_hbm_support) {
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alpha-enter-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.enter_alpha_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alpha-exit-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.exit_alpha_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-dbv-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.hbm_dbv_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-em-60hz-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.hbm_em_configure_60hz_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-em-90hz-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.hbm_em_configure_90hz_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-enter-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.enter_circle_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-exit-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.exit_circle_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alphacircle-enter-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.enter_alphacircle_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alphacircle-exit-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.exit_alphacircle_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-coordinate-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_coordinate_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-size-small-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_size_small_cmds); // local-hbm-fp-circle-radius-cmds
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-size-mid-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_size_mid_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-size-large-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_size_large_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-color-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_color_cmds);
			lcd_kit_parse_array_data(np, "lcd-kit,alpha-table",
				&common_info->hbm.alpha_table);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-60-hz-gamma-read-cmds",
				"lcd-kit,hbm-60-hz-gamma-read-cmds-state",
				&common_info->hbm.hbm_60_hz_gamma_read_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-90-hz-gamma-read-cmds",
				"lcd-kit,hbm-90-hz-gamma-read-cmds-state",
				&common_info->hbm.hbm_90_hz_gamma_read_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-circle-color-setting-cmds",
				"lcd-kit,hbm-circle-color-setting-cmds-state",
				&common_info->hbm.hbm_circle_color_setting_cmds);
		}
	}
	/* acl */
	lcd_kit_parse_u32(np, "lcd-kit,acl-support",
		&common_info->acl.support, 0);
	if (common_info->acl.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-enable-cmds",
			"lcd-kit,acl-enable-cmds-state",
			&common_info->acl.acl_enable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-high-cmds",
			"lcd-kit,acl-high-cmds-state",
			&common_info->acl.acl_high_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-low-cmds",
			"lcd-kit,acl-low-cmds-state",
			&common_info->acl.acl_low_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-middle-cmds",
			"lcd-kit,acl-middle-cmds-state",
			&common_info->acl.acl_middle_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-off-cmds",
			"lcd-kit,acl-off-cmds-state",
			&common_info->acl.acl_off_cmds);
	}
	/* vr */
	lcd_kit_parse_u32(np, "lcd-kit,vr-support",
		&common_info->vr.support, 0);
	if (common_info->vr.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,vr-enable-cmds",
			"lcd-kit,vr-enable-cmds-state",
			&common_info->vr.enable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,vr-disable-cmds",
			"lcd-kit,vr-disable-cmds-state",
			&common_info->vr.disable_cmds);
	}
	/* ce */
	lcd_kit_parse_u32(np, "lcd-kit,ce-support",
		&common_info->ce.support, 0);
	if (common_info->ce.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,ce-off-cmds",
			"lcd-kit,ce-off-cmds-state",
			&common_info->ce.off_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,ce-srgb-cmds",
			"lcd-kit,ce-srgb-cmds-state",
			&common_info->ce.srgb_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,ce-user-cmds",
			"lcd-kit,ce-user-cmds-state",
			&common_info->ce.user_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,ce-vivid-cmds",
			"lcd-kit,ce-vivid-cmds-state",
			&common_info->ce.vivid_cmds);
	}
	/* effect on */
	lcd_kit_parse_u32(np, "lcd-kit,effect-on-support",
		&common_info->effect_on.support, 0);
	if (common_info->effect_on.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,effect-on-cmds",
			"lcd-kit,effect-on-cmds-state",
			&common_info->effect_on.cmds);
	/* grayscale optimize */
	lcd_kit_parse_u32(np, "lcd-kit,grayscale-optimize-support",
		&common_info->grayscale_optimize.support, 0);
	if (common_info->grayscale_optimize.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,grayscale-optimize-cmds",
			"lcd-kit,grayscale-optimize-cmds-state",
			&common_info->grayscale_optimize.cmds);
	/* screen on default effect */
	lcd_kit_parse_u32(np, "lcd-kit,screen-on-effect-support",
		&common_info->screen_on_effect.support, 0);
	if (common_info->screen_on_effect.support) {
		lcd_kit_parse_dcs_cmds(np,
			"lcd-kit,screen-on-effect-prepare-cmds",
			"lcd-kit,screen-on-effect-prepare-cmds-state",
			&common_info->screen_on_effect.prepare_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,screen-on-effect-cmds",
			"lcd-kit,screen-on-effect-cmds-state",
			&common_info->screen_on_effect.cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,screen-on-effect-exit-cmds",
			"lcd-kit,screen-on-effect-exit-cmds-state",
			&common_info->screen_on_effect.exit_cmds);
	}
	/* local hbm alpha map support */
	lcd_kit_parse_u32(np, "lcd-kit,local-hbm-alpha-gain-support",
		&common_info->hbm.hbm_alpha_gain_support, 0);
}

static void lcd_kit_pcamera_position_para_parse(int panel_id, const struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,pre-camera-position-support",
		&common_info->p_cam_position.support, 0);
	if (common_info->p_cam_position.support)
		lcd_kit_parse_u32(np, "lcd-kit,pre-camera-position-end-y",
			&common_info->p_cam_position.end_y, 0);
}

static void lcd_kit_proximity_parse(int panel_id, const struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,thp-proximity-support",
		&common_info->thp_proximity.support, 0);
	if (common_info->thp_proximity.support) {
		common_info->thp_proximity.work_status = TP_PROXMITY_DISABLE;
		common_info->thp_proximity.panel_power_state = POWER_ON;
		lcd_kit_parse_u32(np, "lcd-kit,proximity-reset-delay-min",
			&common_info->thp_proximity.after_reset_delay_min, 0);
		memset(&common_info->thp_proximity.lcd_reset_record_tv,
			0, sizeof(struct timespec64));
	}
}

static void lcd_kit_panel_parse_model(int panel_id, const struct device_node *np)
{
	/* panel model */
	common_info->panel_model = (char *)of_get_property(np,
		"lcd-kit,panel-model", NULL);
}

static void lcd_kit_panel_parse_name(int panel_id, const struct device_node *np)
{
	/* panel name */
	common_info->panel_name = (char *)of_get_property(np,
		"lcd-kit,panel-name", NULL);
}

static void lcd_kit_panel_parse_type(int panel_id, const struct device_node *np)
{
	/* panel type */
	lcd_kit_parse_u32(np, "lcd-kit,panel-type",
		&common_info->panel_type, 0);
	/* oled type */
	lcd_kit_parse_u32(np, "lcd-kit,oled-type",
		&common_info->oled_type, 0);
	/* ltpo ver */
	lcd_kit_parse_u32(np, "lcd-kit,ltpo-ver",
		&common_info->ltpo_ver, 0);
}

static void lcd_kit_panel_parse_information(int panel_id, const struct device_node *np)
{
	/* panel information */
	common_info->module_info = (char *)of_get_property(np,
		"lcd-kit,module-info", NULL);
}

static void lcd_kit_panel_parse_dvdd_vol_command(int panel_id, const struct device_node *np)
{
	/* avs support */
	lcd_kit_parse_u32(np, "lcd-kit,lcd-avs-support",
		&common_info->avs_dvdd.lcd_avs_support, 0);
	if (!common_info->avs_dvdd.lcd_avs_support)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,dvdd-vmin-value",
		&common_info->avs_dvdd.dvdd_vmin_value);
	lcd_kit_parse_array_data(np, "lcd-kit,dvdd-vmin-cmds",
		&common_info->avs_dvdd.dvdd_vmin_cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,dvdd-on-tx-position",
		&common_info->avs_dvdd.dvdd_vmin_cmds_position);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,dvdd-reg-on-tx-cmds",
		"lcd-kit,dvdd-reg-on-tx-cmds-state", &common_info->avs_dvdd.dvdd_on_tx_cmds);
}

static void lcd_kit_panel_parse_off_command(int panel_id, const struct device_node *np)
{
	/* panel off command */
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-off-cmds",
		"lcd-kit,panel-off-cmds-state", &common_info->panel_off_cmds);
}

static void lcd_kit_panel_parse_on_command(int panel_id, const struct device_node *np)
{
	/* panel on command */
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-on-cmds",
		"lcd-kit,panel-on-cmds-state", &common_info->panel_on_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,display-bf-on-command",
		"lcd-kit,panel-on-cmds-state", &common_info->display_on_before_backlight_cmds);
}

static void lcd_kit_panel_parse_on_backup(int panel_id, const struct device_node *np)
{
	int32_t gpio_val;
	/* panel on backup */
	lcd_kit_parse_u32(np, "lcd-kit,pnl-on-cmd-bak-support",
		&common_info->panel_cmd_backup.panel_on_support, 0);
	if (!common_info->panel_cmd_backup.panel_on_support)
		return;
	lcd_kit_parse_u32(np, "lcd-kit,pnl-cmd-bak-det-gpio",
		&common_info->panel_cmd_backup.detect_gpio, 0);
	lcd_kit_parse_u32(np, "lcd-kit,pnl-cmd-bak-gpio-exp-val",
		&common_info->panel_cmd_backup.gpio_exp_val, 0);
	gpio_val = lcd_kit_get_gpio_value(
		common_info->panel_cmd_backup.detect_gpio,
		"panel_cmd_backup");
	common_info->panel_cmd_backup.change_flag = 0;
	if (common_info->panel_cmd_backup.gpio_exp_val != gpio_val)
		return;
	LCD_KIT_INFO("panel_cmd_backup gpio %d\n", gpio_val);
	/* store change status */
	common_info->panel_cmd_backup.change_flag = 1;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-on-cmds-backup",
		"lcd-kit,panel-on-cmds-state", &common_info->panel_on_cmds);
}

static void lcd_kit_panel_parse_esd_cmds(int panel_id, const struct device_node *np)
{
	/* esd */
	lcd_kit_parse_u32(np, "lcd-kit,esd-support",
		&common_info->esd.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,esd-recovery-bl-support",
		&common_info->esd.recovery_bl_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,esd-te-check-support",
		&common_info->esd.te_check_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fac-esd-support",
		&common_info->esd.fac_esd_support, 0);
	if (!common_info->esd.support)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,esd-reg-cmds",
		"lcd-kit,esd-reg-cmds-state", &common_info->esd.cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,esd-value",
		&common_info->esd.value);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,esd-reg-ext-cmds",
		"lcd-kit,esd-reg-cmds-state", &common_info->esd.ext_cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,esd-ext-value",
		&common_info->esd.ext_value);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,esd-mipi-err-cmds",
		"lcd-kit,esd-reg-cmds-state", &common_info->esd.mipi_err_cmds);
	lcd_kit_parse_u32(np, "lcd-kit,esd-mipi-err-cnt",
		&common_info->esd.mipi_err_cnt, 0);
}

static void lcd_kit_panel_parse_esd_gpio_detect_support(int panel_id, const struct device_node *np)
{
	/* esd */
	lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-detect-support",
		&common_info->esd.gpio_detect_support, 0);
	if (!common_info->esd.gpio_detect_support)
		return;
	lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-double-detect",
		&common_info->esd.gpio_double_detect, 0);
	lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-detect-num",
		&common_info->esd.gpio_detect_num, 0);
	lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-normal-value",
		&common_info->esd.gpio_normal_value, 0);
	lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-flag",
		&common_info->esd.gpio_flag, 0);
	lcd_kit_parse_u32(np, "lcd-kit,tp-esd-gpio-event",
		&common_info->esd.tp_esd_event, 0);
	lcd_kit_parse_u32(np, "lcd-kit,tp-report-detect-times",
		&common_info->esd.tp_report_detect_times, 0);
	if (common_info->esd.gpio_double_detect) {
		lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-detect-num1",
		&common_info->esd.gpio_detect_num1, 0);
		lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-normal-value1",
		&common_info->esd.gpio_normal_value1, 0);
	}
}

static void lcd_kit_panel_parse_dsi_detect(int panel_id, const struct device_node *np)
{
	/* dsi detect */
	lcd_kit_parse_u32(np, "lcd-kit,dsi-support",
		&common_info->dsi.support, 0);
	if (!common_info->dsi.support)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,dsi-reg-cmds",
		"lcd-kit,dsi-reg-cmds-state", &common_info->dsi.cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,dsi-value",
		&common_info->dsi.value);
}

static void lcd_kit_panel_parse_vss(int panel_id, const struct device_node *np)
{
	/* vss */
	lcd_kit_parse_u32(np, "lcd-kit,vss-support",
		&common_info->set_vss.support, 0);
	if (!common_info->set_vss.support)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,vss-cmds-fir",
		"lcd-kit,vss-cmds-fir-state",
		&common_info->set_vss.cmds_fir);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,vss-cmds-sec",
		"lcd-kit,vss-cmds-sec-state",
		&common_info->set_vss.cmds_sec);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,vss-cmds-thi",
		"lcd-kit,vss-cmds-thi-state",
		&common_info->set_vss.cmds_thi);
}

static void lcd_kit_panel_parse_dirty_region(int panel_id, const struct device_node *np)
{
	/* dirty region */
	lcd_kit_parse_u32(np, "lcd-kit,dirty-region-support",
		&common_info->dirty_region.support, 0);
	if (!common_info->dirty_region.support)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,dirty-region-cmds",
		"lcd-kit,dirty-region-cmds-state",
		&common_info->dirty_region.cmds);
}

static void lcd_kit_panel_parse_backlight(int panel_id, const struct device_node *np)
{
	/* backlight */
	lcd_kit_parse_u32(np, "lcd-kit,backlight-order",
		&common_info->backlight.order, 0);
	lcd_kit_parse_u32(np, "lcd-kit,backlight-set-bit",
		&common_info->backlight.set_bit, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-min",
		&common_info->backlight.bl_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max",
		&common_info->backlight.bl_max, 0);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,backlight-cmds",
		"lcd-kit,backlight-cmds-state", &common_info->backlight.bl_cmd);
	lcd_kit_parse_u32(np, "lcd-kit,backlight-write-offset",
		&common_info->backlight.write_offset, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-ext-flag",
		&common_info->backlight.ext_flag, 0);
	lcd_kit_parse_u32(np, "lcd-kit,bl-need-refresh-frame",
		&common_info->backlight.bl_need_refresh_frame, 0);
	lcd_kit_parse_u32(np, "lcd-kit,bl-need-prepare",
		&common_info->backlight.need_prepare, 0);
	if (!common_info->backlight.need_prepare)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,bl-prepare-cmds",
		"lcd-kit,bl-prepare-cmds-state",
		&common_info->backlight.prepare);
}

static void lcd_kit_panel_parse_proximity_flicker(int panel_id, const struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,te2-fixed-120hz-support",
		&common_info->proximity_flicker.support, 0);
	if (!common_info->proximity_flicker.support) {
		LCD_KIT_ERR("not support porximity flicker");
		return;
	}
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,te2-fixed-120hz-command",
		"lcd-kit,te2-fixed-120hz-command-state",
		&common_info->proximity_flicker.lcd_fixed_cmd);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,te2-fllow-frame-rate-command",
		"lcd-kit,te2-fllow-frame-rate-command-state",
		&common_info->proximity_flicker.lcd_follow_cmd);
}

static void lcd_kit_panel_parse_check_thread(int panel_id, const struct device_node *np)
{
	/* check thread */
	lcd_kit_parse_u32(np, "lcd-kit,check-thread-enable",
		&common_info->check_thread.enable, 0);
	if (!common_info->check_thread.enable)
		return;
	lcd_kit_parse_u32(np, "lcd-kit,check-bl-support",
		&common_info->check_thread.check_bl_support, 0);
}

static void lcd_kit_panel_parse_temper_thread(int panel_id, const struct device_node *np)
{
	/* temper thread */
	lcd_kit_parse_u32(np, "lcd-kit,temper-thread-enable",
		&common_info->temper_thread.enable, 0);
	if (!common_info->temper_thread.enable)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,temper-position",
		&common_info->temper_thread.temper_position);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,temper-enter-command",
		"lcd-kit,temper-enter-command-state", &common_info->temper_thread.temper_enter_cmd);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,temper-exit-command",
		"lcd-kit,temper-exit-command-state", &common_info->temper_thread.temper_exit_cmd);
}

static void lcd_kit_panel_parse_check_reg_on(int panel_id, const struct device_node *np)
{
	/* check reg on */
	lcd_kit_parse_u32(np, "lcd-kit,check-reg-on-support",
		&common_info->check_reg_on.support, 0);
	if (!common_info->check_reg_on.support)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,check-reg-on-cmds",
		"lcd-kit,check-reg-on-cmds-state",
		&common_info->check_reg_on.cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,check-reg-on-value",
		&common_info->check_reg_on.value);
	lcd_kit_parse_u32(np,
		"lcd-kit,check-reg-on-support-dsm-report",
		&common_info->check_reg_on.support_dsm_report, 0);
}

static void lcd_kit_panel_parse_thermal_power_configuration(int panel_id, const struct device_node *np)
{
	/* power */
	if (!common_info->set_power.support)
		lcd_kit_parse_u32(np, "lcd-kit,power-support",
			&common_info->set_power.support, 0);

	if (!common_info->set_power.support)
		return;
	common_info->set_power.get_thermal = 0;
	lcd_kit_parse_u32(np, "lcd-kit,power-thermal1",
		&common_info->set_power.thermal1, 0);
	lcd_kit_parse_u32(np, "lcd-kit,power-thermal2",
		&common_info->set_power.thermal2, 0);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,power-cmds-fir",
		"lcd-kit,vss-cmds-fir-state",
		&common_info->set_power.cmds_fir);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,power-cmds-sec",
		"lcd-kit,vss-cmds-sec-state",
		&common_info->set_power.cmds_sec);
}

static void lcd_kit_panel_parse_check_reg_off(int panel_id, const struct device_node *np)
{
	/* check reg off */
	lcd_kit_parse_u32(np, "lcd-kit,check-reg-off-support",
		&common_info->check_reg_off.support, 0);
	if (!common_info->check_reg_off.support)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,check-reg-off-cmds",
		"lcd-kit,check-reg-off-cmds-state",
		&common_info->check_reg_off.cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,check-reg-off-value",
		&common_info->check_reg_off.value);
	lcd_kit_parse_u32(np,
		"lcd-kit,check-reg-off-support-dsm-report",
		&common_info->check_reg_off.support_dsm_report, 0);
}

static void lcd_kit_panel_parse_check_mipi(int panel_id, const struct device_node *np)
{
	/* check mipi */
	lcd_kit_parse_u32(np, "lcd-kit,mipi-check-support",
		&common_info->mipi_check.support, 0);
	if (!common_info->mipi_check.support)
		return;
	lcd_kit_parse_u32(np,
		"lcd-kit,panel-mipi-error-report-threshold",
		&common_info->mipi_check.mipi_error_report_threshold, 1);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,mipi-check-cmds",
		"lcd-kit,mipi-check-cmds-state",
		&common_info->mipi_check.cmds);
	lcd_kit_parse_array_data(np, "lcd-kit,mipi-check-value",
		&common_info->mipi_check.value);
}

static void lcd_kit_panel_parse_fps_drf_and_hbm_code(int panel_id, const struct device_node *np)
{
	/* fps drf and hbm code */
	lcd_kit_parse_u32(np, "lcd-kit,fps-lock-command-support",
		&common_info->dfr_info.fps_lock_command_support, 0);
	if (!common_info->dfr_info.fps_lock_command_support)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds-dimming",
		"lcd-kit,fps-to-60-cmds-state",
		&common_info->dfr_info.cmds[FPS_60P_NORMAL_DIM]);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds-hbm",
		"lcd-kit,fps-to-60-cmds-state",
		&common_info->dfr_info.cmds[FPS_60P_HBM_NO_DIM]);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds-hbm-dimming",
		"lcd-kit,fps-to-60-cmds-state",
		&common_info->dfr_info.cmds[FPS_60P_HBM_DIM]);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds-dimming",
		"lcd-kit,fps-to-90-cmds-state",
		&common_info->dfr_info.cmds[FPS_90_NORMAL_DIM]);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds-hbm",
		"lcd-kit,fps-to-90-cmds-state",
		&common_info->dfr_info.cmds[FPS_90_HBM_NO_DIM]);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds-hbm-dimming",
		"lcd-kit,fps-to-90-cmds-state",
		&common_info->dfr_info.cmds[FPS_90_HBM_DIM]);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds",
		"lcd-kit,fps-to-90-cmds-state",
		&common_info->dfr_info.cmds[FPS_90_NORMAL_NO_DIM]);
	init_waitqueue_head(&common_info->dfr_info.fps_wait);
	init_waitqueue_head(&common_info->dfr_info.hbm_wait);
	common_info->dfr_info.hbm_status = HBM_STATUS_IDLE;
	common_info->dfr_info.fps_dfr_status = FPS_DFR_STATUS_IDLE;
}

static void lcd_kit_panel_parse_sn_code(int panel_id, const struct device_node *np)
{
	/* sn code */
	lcd_kit_parse_u32(np, "lcd-kit,sn-code-support",
		&common_info->sn_code.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sn-code-reprocess-support",
		&common_info->sn_code.reprocess_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sn-code-check",
		&common_info->sn_code.check_support, 0);
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-info",
		&common_info->sn_code.sn_code_info);
}

static void lcd_kit_panel_parse_elvdd_detect(int panel_id, const struct device_node *np)
{
	/* elvdd detect */
	lcd_kit_parse_u32(np, "lcd-kit,elvdd-detect-support",
		&common_info->elvdd_detect.support, NOT_SUPPORT);
	if (!common_info->elvdd_detect.support)
		return;
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,open-elvdd-detect-cmds",
		"lcd-kit,open-elvdd-detect-cmds-state",
		&common_info->elvdd_detect.cmds);
}

static void lcd_kit_panel_parse_hbm_configuration(int panel_id, const struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,hbm-fps-command-support",
		&common_info->hbm.hbm_fps_command_support, 0);
	if (!common_info->hbm.hbm_fps_command_support)
		return;
	lcd_kit_parse_u32(np, "lcd-kit,hbm-level-base",
		&common_info->hbm.hbm_level_base, 0);
	lcd_kit_parse_u32(np, "lcd-kit,hbm-level-bias",
		&common_info->hbm.hbm_level_bias, 0);
	lcd_kit_parse_u32(np, "lcd-kit,hbm-level-skip-low",
		&common_info->hbm.hbm_level_skip_low, 0);
	lcd_kit_parse_u32(np, "lcd-kit,hbm-level-skip-high",
		&common_info->hbm.hbm_level_skip_high, 0);
}

static void lcd_kit_panel_parse_little_endian_support(int panel_id, const struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,little-endian-support",
		&common_info->little_endian_support, 0);
}

static void lcd_kit_panel_parse_aod_exit_disp_on_cmds(int panel_id, const struct device_node *np)
{
	/* aod exit disp on cmds */
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-aod-exit-dis-on-cmds",
		"lcd-kit,panel-aod-exit-dis-on-cmds-state",
		&common_info->aod_exit_dis_on_cmds);
}

static void lcd_kit_panel_parse_eink(int panel_id, const struct device_node *np)
{
	/* eink lcd */
	lcd_kit_parse_u32(np, "lcd-kit,eink-lcd",
		&common_info->eink_lcd, 0);
}

static void lcd_kit_panel_parse_alpm_set(int panel_id, const struct device_node *np)
{
	/* alpm set */
	lcd_kit_parse_u32(np, "lcd-kit,aod-no-need-init",
		&common_info->aod_no_need_init, 0);
}

static void lcd_kit_panel_parse_fps_on_command(int panel_id, const struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,fps-on-support",
		&common_info->fps_on.support, 0);
	if (common_info->fps_on.support) {
	/* fps panel on command */
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-on-90-cmds",
		"lcd-kit,panel-on-90-cmds-state", &common_info->fps_on.panel_on_90_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-on-120-cmds",
		"lcd-kit,panel-on-120-cmds-state", &common_info->fps_on.panel_on_120_cmds);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-on-144-cmds",
		"lcd-kit,panel-on-144-cmds-state", &common_info->fps_on.panel_on_144_cmds);
	}
}

static void lcd_kit_panel_parse_color_calib(int panel_id, const struct device_node *np)
{
	/* color calib */
	lcd_kit_parse_u32(np, "lcd-kit,color-calib-support",
		&common_info->c_calib.support, 0);
	if (common_info->c_calib.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,color-calib-cmds",
			"lcd-kit,color-calib-cmds-state",
			&common_info->c_calib.set_cmd);
}

static void lcd_kit_panel_parse_power_set(int panel_id, const struct device_node *np)
{
	/* power set */
	lcd_kit_parse_u32(np, "lcd-kit,set-power-by-nv-support",
		&common_info->set_power_by_nv.support, 0);
	if (common_info->set_power_by_nv.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,power-set-cmds",
			"lcd-kit,power-set-cmds-state",
			&common_info->set_power_by_nv.cmds);
}

static void lcd_kit_panel_parse_hbm_backlight(int panel_id, const struct device_node *np)
{
	/* hbm backlight */
	lcd_kit_parse_u32(np, "lcd-kit,hbm-backlight-support",
		&common_info->hbm_backlight.support, 0);
	if (common_info->hbm_backlight.support) {
		lcd_kit_parse_u32(np, "lcd-kit,hbm-node-value",
			&common_info->hbm_backlight.hbm_node, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,enter-hbm-cmds",
			"lcd-kit,enter-hbm-cmds-state",
			&common_info->hbm_backlight.enter_hbm_backlight_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,exit-hbm-cmds",
			"lcd-kit,exit-hbm-cmds-state",
			&common_info->hbm_backlight.exit_hbm_backlight_cmds);
		lcd_kit_parse_u32(np, "lcd-kit,normal-backlight-max",
			&common_info->hbm_backlight.normal_bl_max, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,set-hbm-cmds",
			"lcd-kit,set-hbm-cmds-state",
			&common_info->hbm_backlight.hbm_backlight_cmds);
	}
}

#define MAX_STR_LEN 64
static void lcd_kit_parse_panel_software_id(int panel_id, const struct device_node *np)
{
	char buf[MAX_STR_LEN] = {0};
	char state_buf[MAX_STR_LEN] = {0};
	int ret;

	lcd_kit_parse_u32(np, "lcd-kit,panel-software-id-support",
		&common_info->panel_software_id.support, 0);
	LCD_KIT_INFO("common_info->panel_software_id.support = %d\n", common_info->panel_software_id.support);
	if (common_info->panel_software_id.support) {
		for (int i = 0; i < SOFTWARE_ID_CMDS_ADD_NUM_MAX; i++) {
			ret = snprintf_s(buf, MAX_STR_LEN, MAX_STR_LEN - 1, "lcd-kit,panel-on-cmds-add%d", i);
			if (ret < 0) {
				LCD_KIT_ERR("snprintf_s cmd buf fail\n");
				return;
			}
			ret = snprintf_s(state_buf, MAX_STR_LEN, MAX_STR_LEN - 1, "lcd-kit,panel-on-cmds-add%d-state", i);
			if (ret < 0) {
				LCD_KIT_ERR("snprintf_s cmd state buf fail\n");
				return;
			}
			lcd_kit_parse_dcs_cmds(np, buf, state_buf, &common_info->panel_software_id.panel_on_cmds_add[i]);
		}
	}
	return;
}

static void lcd_kit_panel_parse_util(int panel_id, const struct device_node *np)
{
	lcd_kit_panel_parse_name(panel_id, np);
	lcd_kit_panel_parse_model(panel_id, np);
	lcd_kit_panel_parse_information(panel_id, np);
	lcd_kit_panel_parse_type(panel_id, np);
	lcd_kit_panel_parse_on_command(panel_id, np);
	lcd_kit_panel_parse_off_command(panel_id, np);
	lcd_kit_panel_parse_on_backup(panel_id, np);
	lcd_kit_panel_parse_esd_gpio_detect_support(panel_id, np);
	lcd_kit_panel_parse_esd_cmds(panel_id, np);
	lcd_kit_pcamera_position_para_parse(panel_id, np);
	lcd_kit_panel_parse_dsi_detect(panel_id, np);
	lcd_kit_panel_parse_dirty_region(panel_id, np);
	lcd_kit_panel_parse_backlight(panel_id, np);
	lcd_kit_panel_parse_check_thread(panel_id, np);
	lcd_kit_panel_parse_temper_thread(panel_id, np);
	lcd_kit_panel_parse_vss(panel_id, np);
	lcd_kit_panel_parse_thermal_power_configuration(panel_id, np);
	lcd_kit_panel_parse_check_reg_on(panel_id, np);
	lcd_kit_panel_parse_check_reg_off(panel_id, np);
	lcd_kit_panel_parse_elvdd_detect(panel_id, np);
	lcd_kit_panel_parse_check_mipi(panel_id, np);
	lcd_kit_panel_parse_dvdd_vol_command(panel_id, np);
	lcd_kit_panel_parse_proximity_flicker(panel_id, np);
	/* thp proximity */
	lcd_kit_proximity_parse(panel_id, np);
	lcd_kit_panel_parse_sn_code(panel_id, np);
	lcd_kit_panel_parse_hbm_configuration(panel_id, np);
	lcd_kit_panel_parse_fps_drf_and_hbm_code(panel_id, np);
	lcd_kit_panel_parse_aod_exit_disp_on_cmds(panel_id, np);
	lcd_kit_panel_parse_alpm_set(panel_id, np);
	lcd_kit_panel_parse_eink(panel_id, np);
	lcd_kit_panel_parse_little_endian_support(panel_id, np);
	lcd_kit_panel_parse_fps_on_command(panel_id, np);
	lcd_kit_panel_parse_color_calib(panel_id, np);
	lcd_kit_panel_parse_power_set(panel_id, np);
	lcd_kit_panel_parse_hbm_backlight(panel_id, np);
	/* panel software id */
	lcd_kit_parse_panel_software_id(panel_id, np);
}

static void lcd_kit_parse_btb_check(int panel_id, struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,lcd-btb-support",
		&common_info->btb_support, 0);

	if (common_info->btb_support) {
		lcd_kit_parse_u32(np, "lcd-kit,lcd-btb-check-type",
			&common_info->btb_check_type, 0);

		lcd_kit_parse_array_data(np, "lcd-kit,lcd-btb-gpio",
			&common_info->lcd_btb_gpio);
	}
}

static void lcd_kit_parse_power_seq(int panel_id, struct device_node *np)
{
	lcd_kit_parse_arrays_data(np, "lcd-kit,power-on-stage",
		&power_seq->power_on_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,lp-on-stage",
		&power_seq->panel_on_lp_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,hs-on-stage",
		&power_seq->panel_on_hs_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,gesture-power-on-stage",
		&power_seq->gesture_power_on_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,power-off-stage",
		&power_seq->power_off_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,lp-off-stage",
		&power_seq->panel_off_lp_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,hs-off-stage",
		&power_seq->panel_off_hs_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,only-power-off-stage",
		&power_seq->only_power_off_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,mipi-switch-stage",
		&power_seq->panel_mipi_switch_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,off-tp-stage",
		&power_seq->panel_off_tp_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,on-tp-stage",
		&power_seq->panel_on_tp_seq, SEQ_NUM);
}

static void lcd_kit_parse_power_iovcc(int panel_id, const struct device_node *np)
{
	/* iovcc */
	if (power_hdl->lcd_iovcc.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-iovcc",
		&power_hdl->lcd_iovcc);
}

static void lcd_kit_parse_power_vsp(int panel_id, const struct device_node *np)
{
	/* vsp */
	if (power_hdl->lcd_vsp.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-vsp",
		&power_hdl->lcd_vsp);
}

static void lcd_kit_parse_power_mipi_switch(int panel_id, const struct device_node *np)
{
	/* bias */
	if (power_hdl->lcd_mipi_switch.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-mipi-switch",
		&power_hdl->lcd_mipi_switch);
}

static void lcd_kit_parse_power_vsn(int panel_id, const struct device_node *np)
{
	/* vsn */
	if (power_hdl->lcd_vsn.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-vsn",
		&power_hdl->lcd_vsn);
}

static void lcd_kit_parse_power_vci(int panel_id, const struct device_node *np)
{
	/* vci */
	if (power_hdl->lcd_vci.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-vci",
		&power_hdl->lcd_vci);
}

static void lcd_kit_parse_power_reset(int panel_id, const struct device_node *np)
{
	/* lcd reset */
	if (power_hdl->lcd_rst.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-reset",
		&power_hdl->lcd_rst);
}

static void lcd_kit_parse_power_backlight(int panel_id, const struct device_node *np)
{
	/* backlight */
	if (power_hdl->lcd_backlight.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-backlight",
		&power_hdl->lcd_backlight);
}

static void lcd_kit_parse_power_te0(int panel_id, const struct device_node *np)
{
	/* TE0 */
	if (power_hdl->lcd_te0.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-te0",
		&power_hdl->lcd_te0);
}

static void lcd_kit_parse_power_tp_reset(int panel_id, const struct device_node *np)
{
	/* tp reset */
	if (power_hdl->tp_rst.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,tp-reset",
		&power_hdl->tp_rst);
}

static void lcd_kit_parse_power_vdd(int panel_id, const struct device_node *np)
{
	/* vdd */
	if (power_hdl->lcd_vdd.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-vdd",
		&power_hdl->lcd_vdd);
}

static void lcd_kit_parse_power_avs(int panel_id, const struct device_node *np)
{
	/* avs */
	if (power_hdl->lcd_avs.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-avs-ctrl",
		&power_hdl->lcd_avs);
}

static void lcd_kit_parse_power_aod(int panel_id, const struct device_node *np)
{
	if (power_hdl->lcd_aod.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-aod",
		&power_hdl->lcd_aod);
}

static void lcd_kit_parse_power_down_vsp(int panel_id, const struct device_node *np)
{
	/* bias */
	if (power_hdl->lcd_power_down_vsp.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-power-down-vsp",
		&power_hdl->lcd_power_down_vsp);
}

static void lcd_kit_parse_power_down_vsn(int panel_id, const struct device_node *np)
{
	/* bias */
	if (power_hdl->lcd_power_down_vsn.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-power-down-vsn",
		&power_hdl->lcd_power_down_vsn);
}

static void lcd_kit_parse_power_pmic_ctrl(int panel_id, const struct device_node *np)
{
	/* pmic ctrl */
	if (power_hdl->lcd_pmic_ctrl.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-pmic-ctrl",
		&power_hdl->lcd_pmic_ctrl);
}

static void lcd_kit_parse_power_pmic_reset(int panel_id, const struct device_node *np)
{
	/* pmic ctrl */
	if (power_hdl->lcd_pmic_reset.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-pmic-reset",
		&power_hdl->lcd_pmic_reset);
}

static void lcd_kit_parse_power_pmic_init(int panel_id, const struct device_node *np)
{
	/* pmic ctrl */
	if (power_hdl->lcd_pmic_init.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-pmic-init",
		&power_hdl->lcd_pmic_init);
}

static void lcd_kit_parse_power_boost_enable(int panel_id, const struct device_node *np)
{
	/* pmic ctrl */
	if (power_hdl->lcd_boost_enable.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-boost-enable",
		&power_hdl->lcd_boost_enable);
}

static void lcd_kit_parse_power_boost_ctrl(int panel_id, const struct device_node *np)
{
	/* pmic ctrl */
	if (power_hdl->lcd_boost_ctrl.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-boost-ctrl",
		&power_hdl->lcd_boost_ctrl);
}

static void lcd_kit_parse_power_boost_bypass(int panel_id, const struct device_node *np)
{
	/* pmic ctrl */
	if (power_hdl->lcd_boost_bypass.buf != NULL)
		return;
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-boost-bypass",
		&power_hdl->lcd_boost_bypass);
}

static void lcd_kit_parse_power(int panel_id, const struct device_node *np)
{
	lcd_kit_parse_power_vci(panel_id, np);
	lcd_kit_parse_power_iovcc(panel_id, np);
	lcd_kit_parse_power_vsp(panel_id, np);
	lcd_kit_parse_power_vsn(panel_id, np);
	lcd_kit_parse_power_reset(panel_id, np);
	lcd_kit_parse_power_backlight(panel_id, np);
	lcd_kit_parse_power_te0(panel_id, np);
	lcd_kit_parse_power_tp_reset(panel_id, np);
	lcd_kit_parse_power_vdd(panel_id, np);
	lcd_kit_parse_power_aod(panel_id, np);
	lcd_kit_parse_power_down_vsp(panel_id, np);
	lcd_kit_parse_power_down_vsn(panel_id, np);
	lcd_kit_parse_power_mipi_switch(panel_id, np);
	lcd_kit_parse_power_pmic_ctrl(panel_id, np);
	lcd_kit_parse_power_pmic_reset(panel_id, np);
	lcd_kit_parse_power_pmic_init(panel_id, np);
	lcd_kit_parse_power_boost_enable(panel_id, np);
	lcd_kit_parse_power_boost_ctrl(panel_id, np);
	lcd_kit_parse_power_boost_bypass(panel_id, np);
	lcd_kit_parse_power_avs(panel_id, np);
}

static void lcd_kit_parse_panel_product_type(int panel_id, const struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,panel-product-type-switch",
		&common_info->panel_product_type_switch, 0);
}

static int lcd_kit_panel_parse_dt(int panel_id, struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("np is null\n");
		return LCD_KIT_FAIL;
	}
	/* parse effect info */
	lcd_kit_panel_parse_effect(panel_id, np);
	/* parse normal info */
	lcd_kit_panel_parse_util(panel_id, np);
	/* parse power sequence */
	lcd_kit_parse_power_seq(panel_id, np);
	/* parse power */
	lcd_kit_parse_power(panel_id, np);
	/* btb check */
	lcd_kit_parse_btb_check(panel_id, np);
	/* panel product type switch */
	lcd_kit_parse_panel_product_type(panel_id, np);
	return LCD_KIT_OK;
}

static int lcd_kit_get_bias_voltage(int panel_id, int *vpos, int *vneg)
{
	if (!vpos || !vneg) {
		LCD_KIT_ERR("vpos/vneg is null\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_vsp.buf)
		*vpos = power_hdl->lcd_vsp.buf[POWER_VOL];
	if (power_hdl->lcd_vsn.buf)
		*vneg = power_hdl->lcd_vsn.buf[POWER_VOL];
	return LCD_KIT_OK;
}

static void lcd_kit_temper_wq_handler(struct work_struct *work)
{
	int ret = LCD_KIT_OK;
	int cur_thermal;
	int panel_id = get_panel_id();
	ret = lcd_kit_get_temper_handler(&cur_thermal);
	if (ret) {
		LCD_KIT_ERR("get thermal is error!\n");
		return;
	}
	LCD_KIT_INFO("cur_thermal = %d\n", cur_thermal);
	if (common_info->temper_thread.temper_lhbm_flag) {
		LCD_KIT_ERR("LHBM is running, skip temper\n");
		return;
	}
	lcd_kit_is_open_temper(panel_id, cur_thermal);
}

static void lcd_kit_check_wq_handler(struct work_struct *work)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_bl_ops *bl_ops = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;
	int panel_id = get_panel_id();

	lcd_ops = lcd_kit_get_ops();
	bl_ops = lcd_kit_get_bl_ops();
	if (!common_info->check_thread.enable) {
		LCD_KIT_DEBUG("check_thread is disable\n");
		return;
	}
	if (common_info->set_vss.support) {
		if (lcd_ops && lcd_ops->set_vss_by_thermal) {
			ret = lcd_ops->set_vss_by_thermal();
			if (ret)
				LCD_KIT_ERR("set vss by thermal failed!\n");
		}
	}
	if (common_info->set_power.support) {
		if (lcd_ops && lcd_ops->set_power_by_thermal) {
			ret = lcd_ops->set_power_by_thermal();
			if (ret)
				LCD_KIT_ERR("set vss by thermal failed!\n");
		}
	}
	if (common_info->check_thread.check_bl_support) {
		/* check backlight */
		if (bl_ops && bl_ops->check_backlight) {
			ret = bl_ops->check_backlight();
			if (ret)
				LCD_KIT_ERR("backlight check abnomal!\n");
		}
	}
}

static enum hrtimer_restart lcd_kit_temper_hrtimer_fnc(struct hrtimer *timer)
{
	int panel_id = get_panel_id();
	if (common_info->temper_thread.enable) {
		schedule_delayed_work(&common_info->temper_thread.temper_work, 0);
		hrtimer_start(&common_info->temper_thread.hrtimer,
			ktime_set(TEMPER_THREAD_TIME_PERIOD / 1000, // chang ms to s
			(TEMPER_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
			HRTIMER_MODE_REL);
	}
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart lcd_kit_check_hrtimer_fnc(struct hrtimer *timer)
{
	int panel_id = get_panel_id();

	if (common_info->check_thread.enable) {
		schedule_delayed_work(&common_info->check_thread.check_work, 0);
		hrtimer_start(&common_info->check_thread.hrtimer,
			ktime_set(CHECK_THREAD_TIME_PERIOD / 1000, // chang ms to s
			(CHECK_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
			HRTIMER_MODE_REL);
	}
	return HRTIMER_NORESTART;
}

static void lcd_kit_temper_thread_register(int panel_id)
{
	if (common_info->temper_thread.enable) {
		INIT_DELAYED_WORK(&common_info->temper_thread.temper_work,
			lcd_kit_temper_wq_handler);
		hrtimer_init(&common_info->temper_thread.hrtimer,
			CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		common_info->temper_thread.hrtimer.function =
			lcd_kit_temper_hrtimer_fnc;
		if (common_info->temper_thread.dark_enable)
			hrtimer_start(&common_info->temper_thread.hrtimer,
				ktime_set(TEMPER_THREAD_TIME_PERIOD / 1000, // chang ms to s
				(TEMPER_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
				HRTIMER_MODE_REL);
	}
}

static void lcd_kit_check_thread_register(int panel_id)
{
	if (common_info->check_thread.enable) {
		INIT_DELAYED_WORK(&common_info->check_thread.check_work,
			lcd_kit_check_wq_handler);
		hrtimer_init(&common_info->check_thread.hrtimer,
			CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		common_info->check_thread.hrtimer.function =
			lcd_kit_check_hrtimer_fnc;
		hrtimer_start(&common_info->check_thread.hrtimer,
			ktime_set(CHECK_THREAD_TIME_PERIOD / 1000, // chang ms to s
			(CHECK_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
			HRTIMER_MODE_REL);
	}
}
static void lcd_kit_lock_init(int panel_id)
{
	/* init mipi lock */
	mutex_init(&COMMON_LOCK->mipi_lock);
	if (common_info->hbm.support)
		mutex_init(&COMMON_LOCK->hbm_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.model_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.type_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.panel_info_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.vol_enable_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.amoled_acl_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.amoled_vr_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.support_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.gamma_dynamic_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.frame_count_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.frame_update_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.mipi_dsi_clk_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.fps_scence_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.fps_order_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.alpm_function_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.alpm_setting_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.ddic_alpha_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.func_switch_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.reg_read_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.ddic_oem_info_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.bl_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.support_bl_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.effect_bl_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.ddic_lv_detect_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.hbm_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.panel_sn_code_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.pre_camera_position_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.camera_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.panel_dimming_switch_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.avg_on_lock);
}

static void lcd_kit_panel_hbm_status_init(int panel_id, struct device_node *np)
{
	if (common_info->hbm_backlight.support)
		common_info->hbm_backlight.hbm_status = LCD_KIT_HBM_OFF;
}

static int lcd_kit_common_init(int panel_id, struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node!\n");
		return LCD_KIT_FAIL;
	}

#ifdef LCD_KIT_DEBUG_ENABLE
	lcd_kit_debugfs_init();
#endif
	lcd_kit_panel_parse_dt(panel_id, np);
	lcd_kit_check_thread_register(panel_id);
	lcd_kit_temper_thread_register(panel_id);
	lcd_kit_lock_init(panel_id);
#ifdef LCD_KIT_DKMD
	/* parse local hbm cmds */
	lcd_kit_local_hbm_cmds_parse(panel_id, np);
	display_engine_init();
#endif
#ifdef LCD_KIT_ALPHA_LHBM
	lcd_kit_local_hbm_cmds_parse(panel_id, np);
#endif
	lcd_kit_panel_hbm_status_init(panel_id, np);
	return LCD_KIT_OK;
}

#ifdef CONFIG_HUAWEI_DSM
int lcd_dsm_client_record(struct dsm_client *lcd_dclient, char *record_buf,
	int lcd_dsm_error_no, int rec_num_limit, int *cur_rec_time)
{
	if (!lcd_dclient || !record_buf || !cur_rec_time) {
		LCD_KIT_ERR("null pointer!\n");
		return LCD_KIT_FAIL;
	}

	if ((rec_num_limit >= 0) && (*cur_rec_time > rec_num_limit)) {
		LCD_KIT_INFO("dsm record limit!\n");
		return LCD_KIT_OK;
	}

	if (!dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, record_buf);
		dsm_client_notify(lcd_dclient, lcd_dsm_error_no);
		(*cur_rec_time)++;
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("dsm_client_ocuppy failed!\n");
	return LCD_KIT_FAIL;
}
#endif

void lcd_kit_delay(int wait, int waittype, bool allow_sleep)
{
	if (!wait) {
		LCD_KIT_DEBUG("wait is 0\n");
		return;
	}
	if ((waittype == LCD_KIT_WAIT_US) ||
		(waittype == SELF_REFRESH_FLAG)) {
		udelay(wait);
	} else if (waittype & LCD_KIT_WAIT_MS) {
		if (wait > 10 && allow_sleep)
			usleep_range(wait * 1000, wait * 1000);
		else
			mdelay(wait);
	} else {
		if (allow_sleep)
			msleep(wait * 1000);
		else
			mdelay(wait * 1000);
	}
}

/* used to avoid a uint32_t happening overflow */
#define MAX_ERROR_TIMES 100000000
static void lcd_kit_mipi_check(int panel_id, void *pdata, char *panel_name,
	long display_on_record_time)
{
	int i, ret;
	uint32_t read_value[MAX_REG_READ_COUNT] = {0};
	static struct lcd_kit_mipierrors mipi_errors[MAX_REG_READ_COUNT];
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	uint32_t *expect_ptr = common_info->mipi_check.value.buf;

#if defined CONFIG_HUAWEI_DSM
	#define REC_LIMIT_TIMES (-1)
	#define RECORD_BUFLEN   200
	char record_buf[RECORD_BUFLEN] = {'\0'};
	static int recordtime;
	struct timespec64 tv = { 0, 0 };
	long diskeeptime = 0;
#endif

	if (!pdata || !panel_name || !adapt_ops || !expect_ptr) {
		LCD_KIT_ERR("mipi check happened parameter error!\n");
		return;
	}
	if (common_info->mipi_check.support == 0)
		return;
	if (!adapt_ops->mipi_rx) {
		LCD_KIT_ERR("mipi_rx function is null!\n");
		return;
	}
	memset(mipi_errors, 0,
		MAX_REG_READ_COUNT * sizeof(struct lcd_kit_mipierrors));
	ret = adapt_ops->mipi_rx(panel_id, pdata, (u8 *)read_value, MAX_REG_READ_COUNT - 1,
		&common_info->mipi_check.cmds);
	if (ret) {
		LCD_KIT_ERR("mipi read failed!\n");
		return;
	}
	for (i = 0; i < common_info->mipi_check.value.cnt; i++) {
		if (mipi_errors[i].total_errors >= MAX_ERROR_TIMES) {
			LCD_KIT_ERR("mipi error times is too large!\n");
			return;
		}
		mipi_errors[i].mipi_check_times++;
		if (read_value[i] != expect_ptr[i]) {
			mipi_errors[i].mipi_error_times++;
			mipi_errors[i].total_errors += read_value[i];
			LCD_KIT_ERR("mipi check error[%d]: current error times:%d! total error times:%d, check-error-times/check-times:%d/%d\n",
				i, read_value[i], mipi_errors[i].total_errors,
				mipi_errors[i].mipi_error_times,
				mipi_errors[i].mipi_check_times);
#if defined CONFIG_HUAWEI_DSM
			if (read_value[i] < common_info->mipi_check.mipi_error_report_threshold)
				continue;
			ktime_get_real_ts64(&tv);
			diskeeptime = tv.tv_sec - display_on_record_time;
			ret = snprintf(record_buf, RECORD_BUFLEN, "%s:display_on_keep_time=%lds, reg_val[%d]=0x%x!\n",
				"lcd", diskeeptime, common_info->mipi_check.cmds.cmds[i].payload[0], read_value[i]);
			if (ret < 0) {
				LCD_KIT_ERR("snprintf happened error!\n");
				continue;
			}
			(void)lcd_dsm_client_record(lcd_dclient, record_buf,
				DSM_LCD_MIPI_TRANSMIT_ERROR_NO, REC_LIMIT_TIMES, &recordtime);
#endif
			continue;
		}
		LCD_KIT_INFO("mipi check nomal[%d]: total error times:%d, check-error-times/check-times:%d/%d\n",
			i, mipi_errors[i].total_errors, mipi_errors[i].mipi_error_times, mipi_errors[i].mipi_check_times);
	}
}

static void lcd_kit_btb_check(int panel_id)
{
	int i;
	int btb_status = 0;
	unsigned int btb_gpio;

	if (!common_info->btb_support) {
		LCD_KIT_INFO("LCD BTB check not support\n");
		return;
	}

	for (i = 0; i < common_info->lcd_btb_gpio.cnt; i++) {
		btb_gpio = common_info->lcd_btb_gpio.buf[i] + common_info->gpio_offset;
		btb_status += gpio_get_value(btb_gpio);
	}

	if (btb_status > 0) {
		LCD_KIT_ERR("LCD BTB check ERROR = %d\n", btb_status);
#if defined CONFIG_HUAWEI_DSM
		dsm_client_record(lcd_dclient, "LCD BTB check ERROR = %d",
			btb_status);
		dsm_client_notify(lcd_dclient, DSM_LCD_BTB_CHECK_ERROR_NO);
#endif
	} else {
		LCD_KIT_INFO("LCD BTB check OK\n");
	}
}

static irqreturn_t lcd_kit_btb_check_handler(int irq, void *dev_id)
{
	int panel_id = get_panel_id();

	if (btb_irq_count < MAX_BTB_IRQ_COUNT) {
		lcd_kit_btb_check(panel_id);
	} else {
		LCD_KIT_ERR("LCD BTB check error is 2 times\n");
		disable_irq_nosync(irq);
	}
	btb_irq_count++;
	return IRQ_HANDLED;
}

static void lcd_kit_btb_init(int panel_id)
{
	int ret;
	int btb_gpio;
	int irq;

	if (!common_info->btb_support) {
		LCD_KIT_INFO("LCD BTB check not support\n");
		return;
	}

	LCD_KIT_INFO("btb type = %d\n", common_info->btb_check_type);

	if (common_info->btb_check_type == LCD_KIT_BTB_CHECK_IRQ) {
		/* btb0 is up btb check */
		btb_gpio = common_info->lcd_btb_gpio.buf[0] + common_info->gpio_offset;
		ret = gpio_request(btb_gpio, "btb_gpio0");
		if (ret < 0) {
			LCD_KIT_ERR("request gpio[%d] failed\n", btb_gpio);
			return;
		}

		gpio_direction_input(btb_gpio);
		irq = gpio_to_irq(btb_gpio);
		ret = request_irq(irq, lcd_kit_btb_check_handler,
			IRQF_TRIGGER_HIGH, "btb0", NULL);
		if (ret)
			LCD_KIT_ERR("btb request irq0 failed\n");

		/* btb1 is down btb check */
		btb_gpio = (int)(common_info->lcd_btb_gpio.buf[1] + common_info->gpio_offset);
		ret = gpio_request(btb_gpio, "btb_gpio1");
		if (ret < 0) {
			LCD_KIT_ERR("request gpio[%d] failed\n", btb_gpio);
			return;
		}

		gpio_direction_input(btb_gpio);
		irq = gpio_to_irq(btb_gpio);
		ret = request_irq(irq, lcd_kit_btb_check_handler,
			IRQF_TRIGGER_HIGH, "btb1", NULL);
		if (ret)
			LCD_KIT_ERR("btb request irq1 failed\n");
	}
}

void lcd_backlight_i2c_dmd(void)
{
	if (lcd_backlight_i2c_count < MAX_I2C_DMD_COUNT) {
#if defined CONFIG_HUAWEI_DSM
		dsm_client_record(lcd_dclient, "lcd backlight i2c error\n");
		dsm_client_notify(lcd_dclient, DSM_LCD_BACKLIGHT_I2C_ERROR_NO);
#endif
		lcd_backlight_i2c_count++;
	}
}
void lcd_bias_i2c_dmd(void)
{
	if (lcd_bias_i2c_count < MAX_I2C_DMD_COUNT) {
#if defined CONFIG_HUAWEI_DSM
		dsm_client_record(lcd_dclient, "lcd bias i2c error\n");
		dsm_client_notify(lcd_dclient, DSM_LCD_BIAS_I2C_ERROR_NO);
#endif
		lcd_bias_i2c_count++;
	}
}

static int lcd_kit_enter_hbm(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL || adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* enter hbm */
	if (common_info->hbm_backlight.enter_hbm_backlight_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm_backlight.enter_hbm_backlight_cmds);
		common_info->hbm_backlight.hbm_status = LCD_KIT_HBM_ON;
		LCD_KIT_INFO("enter hbm backlight cmds already send, ret = %d!\n", ret);
	}
	return ret;
}

static int lcd_kit_exit_hbm(int panel_id, void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL || adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* exit hbm */
	if (common_info->hbm_backlight.exit_hbm_backlight_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm_backlight.exit_hbm_backlight_cmds);
		common_info->hbm_backlight.hbm_status = LCD_KIT_HBM_OFF;
		LCD_KIT_INFO("exit hbm backlight cmds already send, ret = %d!\n", ret);
	}
	return ret;
}

static int lcd_kit_set_hbm_backlight(int panel_id, void *hld, u32 level)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL || adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}

	if (common_info->hbm_backlight.hbm_backlight_cmds.cmds == NULL) {
		LCD_KIT_ERR("set hbm backlight cmds is NULL!\n");
		return LCD_KIT_FAIL;
	}

	/* set hbm backlight */
	if (common_info->hbm_backlight.normal_bl_max <= 0xFF) {
		common_info->hbm_backlight.hbm_backlight_cmds.cmds[0].payload[1] = level;
	} else {
		common_info->hbm_backlight.hbm_backlight_cmds.cmds[0].payload[1] = ((level >> 8) & 0xFF);
		common_info->hbm_backlight.hbm_backlight_cmds.cmds[0].payload[2] = level & 0xFF;
	}
	ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm_backlight.hbm_backlight_cmds);
	LCD_KIT_INFO("set hbm backlight cmds already send, ret = %d!\n", ret);
	return ret;
}

/* common ops */
struct lcd_kit_common_ops g_lcd_kit_common_ops = {
	.common_init = lcd_kit_common_init,
	.panel_power_on = lcd_kit_panel_power_on,
	.panel_power_on_without_mipi = lcd_kit_panel_power_on_without_mipi,
	.panel_power_on_with_mipi = lcd_kit_panel_power_on_with_mipi,
	.panel_on_lp = lcd_kit_panel_on_lp,
	.panel_on_hs = lcd_kit_panel_on_hs,
	.panel_off_hs = lcd_kit_panel_off_hs,
	.panel_off_lp = lcd_kit_panel_off_lp,
	.panel_mipi_switch = lcd_kit_panel_mipi_switch,
	.panel_off_tp = lcd_kit_panel_off_tp,
	.panel_on_tp = lcd_kit_panel_on_tp,
	.panel_power_off = lcd_kit_panel_power_off,
	.panel_only_power_off = lcd_kit_panel_only_power_off,
	.get_panel_name = lcd_kit_get_panel_name,
	.get_panel_info = lcd_kit_get_panel_info,
	.get_cabc_mode = lcd_kit_get_cabc_mode,
	.set_cabc_mode = lcd_kit_set_cabc_mode,
	.get_acl_mode = lcd_kit_get_acl_mode,
	.set_acl_mode = lcd_kit_set_acl_mode,
	.get_vr_mode = lcd_kit_get_vr_mode,
	.set_vr_mode = lcd_kit_set_vr_mode,
	.esd_handle = lcd_kit_esd_handle,
	.dsi_handle = lcd_kit_dsi_handle,
	.dirty_region_handle = lcd_kit_dirty_region_handle,
	.set_ce_mode = lcd_kit_set_ce_mode,
	.get_ce_mode = lcd_kit_get_ce_mode,
	.hbm_set_handle = lcd_kit_hbm_set_handle,
	.fp_hbm_enter_extern = lcd_kit_enter_fp_hbm_extern,
	.fp_hbm_exit_extern = lcd_kit_exit_fp_hbm_extern,
	.set_ic_dim_on = lcd_kit_hbm_dim_enable,
	.set_effect_color_mode = lcd_kit_set_effect_color_mode,
	.get_effect_color_mode = lcd_kit_get_effect_color_mode,
	.set_mipi_backlight = lcd_kit_set_mipi_backlight,
	.get_bias_voltage = lcd_kit_get_bias_voltage,
	.mipi_check = lcd_kit_mipi_check,
	.btb_check = lcd_kit_btb_check,
	.btb_init = lcd_kit_btb_init,
	.enter_hbm = lcd_kit_enter_hbm,
	.exit_hbm = lcd_kit_exit_hbm,
	.set_hbm_backlight = lcd_kit_set_hbm_backlight,
	.is_power_event = lcd_kit_is_power_event,
	.check_reg_report_dsm = lcd_kit_check_reg_report_dsm,
	.start_temper_hrtimer = lcd_kit_start_temper_hrtimer,
	.stop_temper_hrtimer = lcd_kit_stop_temper_hrtimer,
};

struct gamma_linear_interpolation_info {
	uint32_t grayscale_before;
	uint32_t grayscale;
	uint32_t grayscale_after;
	uint32_t gamma_node_value_before;
	uint32_t gamma_node_value;
	uint32_t gamma_node_value_after;
};

static int display_engine_get_grayscale_index(int grayscale,
	const struct gamma_node_info *gamma_info, int *index)
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
			LCD_KIT_DEBUG("grayscale[%d]:%d\n", i, gamma_info->node_grayscale.buf[i]);
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

	LCD_KIT_DEBUG("grayscale(before:%d, self:%d, after:%d)\n",
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
	LCD_KIT_DEBUG("gamma_node_value(before:%d, self:%d,after:%d), gamma_value:%d\n",
		gamma_liner_info->gamma_node_value_before, gamma_liner_info->gamma_node_value,
		gamma_liner_info->gamma_node_value_after, gamma_value);
	return LCD_KIT_OK;
}

static int display_engine_set_gamma_liner_info(const struct gamma_node_info *gamma_info,
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
	LCD_KIT_DEBUG("index:%d, gamma_node_value_before:%d, gamma_node_value_after:%d\n",
		index, gamma_liner_info->gamma_node_value_before,
		gamma_liner_info->gamma_node_value_after);
	return ret;
}

static int display_engine_set_color_cmds_value_by_grayscale(
	const struct gamma_node_info *gamma_info, const uint32_t *gamma_value_array,
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
	LCD_KIT_DEBUG("payload[1]:%d, payload[2]:%d\n", payload[0], payload[1]);
	return ret;
}

static int display_engine_set_color_cmds_by_grayscale(uint32_t fps,
	struct color_cmds_rgb *color_cmds, const struct gamma_node_info *gamma_info, int grayscale)
{
	LCD_KIT_DEBUG("grayscale = %d\n", grayscale);
	if (!color_cmds) {
		LCD_KIT_ERR("color_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!gamma_info) {
		LCD_KIT_ERR("gamma_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (fps == fps_normal) {
		if (display_engine_set_color_cmds_value_by_grayscale(gamma_info,
			gamma_info->red_60_hz, HBM_GAMMA_NODE_SIZE,
			color_cmds->red_payload, grayscale) ||
			display_engine_set_color_cmds_value_by_grayscale(gamma_info,
				gamma_info->green_60_hz, HBM_GAMMA_NODE_SIZE,
				color_cmds->green_payload, grayscale) ||
			display_engine_set_color_cmds_value_by_grayscale(gamma_info,
				gamma_info->blue_60_hz,
				HBM_GAMMA_NODE_SIZE, color_cmds->blue_payload, grayscale)) {
			LCD_KIT_ERR("display_engine_set_color_cmds_value error\n");
			return LCD_KIT_FAIL;
		}
	} else if (fps == fps_medium) {
		if (display_engine_set_color_cmds_value_by_grayscale(gamma_info,
			gamma_info->red_90_hz, HBM_GAMMA_NODE_SIZE,
			color_cmds->red_payload, grayscale) ||
			display_engine_set_color_cmds_value_by_grayscale(gamma_info,
				gamma_info->green_90_hz, HBM_GAMMA_NODE_SIZE,
				color_cmds->green_payload, grayscale) ||
			display_engine_set_color_cmds_value_by_grayscale(gamma_info,
				gamma_info->blue_90_hz, HBM_GAMMA_NODE_SIZE,
				color_cmds->blue_payload, grayscale)) {
			LCD_KIT_ERR("display_engine_set_color_cmds_value error\n");
			return LCD_KIT_FAIL;
		}
	} else {
		LCD_KIT_ERR("fps not support, fps:%d\n", fps);
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int display_engine_set_60hz_color_cmds_value_by_index(int panel_id,
	uint32_t gamma_node_index)
{
	/* The interval between gamma nodes in the array is 4 */
	uint32_t start_pos = gamma_node_index * 4;
	if ((start_pos + 1) >= HBM_GAMMA_SIZE) {
		LCD_KIT_ERR("start_pos out of range, start_pos:%u,"
			"HBM_GAMMA_SIZE:%d\n", start_pos, HBM_GAMMA_SIZE);
		return LCD_KIT_FAIL;
	}

	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	if ((int)common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen <
		CMDS_COLOR_MAX) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.
			cmds[color_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}

	/* R */
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_HIGH] =
		common_info->hbm.hbm_gamma.red_60_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_LOW] =
		common_info->hbm.hbm_gamma.red_60_hz[start_pos + 1];

	/* G */
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_HIGH] =
		common_info->hbm.hbm_gamma.green_60_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_LOW] =
		common_info->hbm.hbm_gamma.green_60_hz[start_pos + 1];

	/* B */
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_HIGH] =
		common_info->hbm.hbm_gamma.blue_60_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_LOW] =
		common_info->hbm.hbm_gamma.blue_60_hz[start_pos + 1];

	return LCD_KIT_OK;
}

static int display_engine_set_90hz_color_cmds_value_by_index(int panel_id,
	uint32_t gamma_node_index)
{
	/* The interval between gamma nodes in the array is 4 */
	uint32_t start_pos = gamma_node_index * 4;
	if (start_pos >= HBM_GAMMA_SIZE) {
		LCD_KIT_ERR("start_pos out of range, start_pos:%u,"
		"HBM_GAMMA_SIZE:%d\n", start_pos, HBM_GAMMA_SIZE);
		return LCD_KIT_FAIL;
	}

	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	if ((int)common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen <
		CMDS_COLOR_MAX) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.
			cmds[color_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}

	/* R */
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_HIGH] =
		common_info->hbm.hbm_gamma.red_90_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_LOW] =
		common_info->hbm.hbm_gamma.red_90_hz[start_pos + 1];

	/* G */
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_HIGH] =
		common_info->hbm.hbm_gamma.green_90_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_LOW] =
		common_info->hbm.hbm_gamma.green_90_hz[start_pos + 1];

	/* B */
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_HIGH] =
		common_info->hbm.hbm_gamma.blue_90_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_LOW] =
		common_info->hbm.hbm_gamma.blue_90_hz[start_pos + 1];

	return LCD_KIT_OK;
}

int display_engine_local_hbm_set_color_cmds_value_by_index(const uint32_t fps, uint32_t gamma_node_index)
{
	int ret;
	int panel_id = get_panel_id();

	if (common_info->hbm.circle_color_cmds.cmds == NULL) {
		LCD_KIT_ERR("circle_color_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("gamma_node_index:%u, fps:%d\n", gamma_node_index, fps);
	if (fps == fps_normal) {
		ret = display_engine_set_60hz_color_cmds_value_by_index(panel_id, gamma_node_index);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_DEBUG("display_engine_set_60hz_color_cmds_value_by_index error\n");
			return LCD_KIT_FAIL;
		}
	} else if (fps == fps_medium) {
		ret = display_engine_set_90hz_color_cmds_value_by_index(panel_id, gamma_node_index);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_DEBUG("display_engine_set_90hz_color_cmds_value_by_index error\n");
			return LCD_KIT_FAIL;
		}
	} else {
		LCD_KIT_ERR("fps not support, fps:%d\n", fps);
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int display_engine_set_gamma_node_info(int panel_id, struct gamma_node_info *gamma_info)
{
	int i;
	int cnt;

	if (!gamma_info) {
		LCD_KIT_ERR("gamma_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	cnt = gamma_info->node_grayscale.cnt;

	/* 2 hbm_gamma_values are converted to 1 gamma_node_value. */
	if ((cnt > HBM_GAMMA_NODE_SIZE) || ((2 * cnt) > HBM_GAMMA_SIZE)) {
		LCD_KIT_ERR("node_grayscale.cnt out of range,"
			"gamma_info->node_grayscale.cnt:%d, HBM_GAMMA_NODE_SIZE:%d,"
			"HBM_GAMMA_SIZE:%d\n", cnt, HBM_GAMMA_NODE_SIZE, HBM_GAMMA_SIZE);
		return LCD_KIT_FAIL;
	}

	/* The high 8 bits of a hex number are converted into a dec number multiplied by 256 */
	for (i = 0; i < cnt; i++) {
		gamma_info->red_60_hz[i] = (common_info->hbm.hbm_gamma.red_60_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.red_60_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->red_60_hz[%d]:%d\n", i, gamma_info->red_60_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->green_60_hz[i] = (common_info->hbm.hbm_gamma.green_60_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.green_60_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->green_60_hz[%d]:%d\n", i, gamma_info->green_60_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->blue_60_hz[i] = (common_info->hbm.hbm_gamma.blue_60_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.blue_60_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->blue_60_hz[%d]:%d\n", i, gamma_info->blue_60_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->red_90_hz[i] = (common_info->hbm.hbm_gamma.red_90_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.red_90_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->red_90_hz[%d]:%d\n", i, gamma_info->red_90_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->green_90_hz[i] = (common_info->hbm.hbm_gamma.green_90_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.green_90_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->green_90_hz[%d]:%d\n", i, gamma_info->green_90_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->blue_90_hz[i] = (common_info->hbm.hbm_gamma.blue_90_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.blue_90_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->blue_90_hz[%d]:%d\n", i, gamma_info->blue_90_hz[i]);
	}
	return LCD_KIT_OK;
}

static int display_engine_set_hbm_gamma_60hz_to_common_info(int panel_id,
	uint8_t *hbm_gamma, size_t rgb_size)
{
	size_t i;

	if (!hbm_gamma) {
		LCD_KIT_ERR("hbm_gamma is NULL\n");
		return LCD_KIT_FAIL;
	}

	if ((GAMMA_INDEX_BLUE_HIGH + HBM_GAMMA_SIZE) > rgb_size) {
		LCD_KIT_ERR("hbm_gamma hbm_gamma index out of range\n");
		return LCD_KIT_FAIL;
	}

	/* The 2 values correspond to each other */
	for (i = 0; i < (HBM_GAMMA_SIZE / 2); i++) {
		common_info->hbm.hbm_gamma.red_60_hz[i * 2] =
			hbm_gamma[GAMMA_INDEX_RED_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.red_60_hz[(i * 2) + 1] =
			hbm_gamma[GAMMA_INDEX_RED_LOW + (i * 2)];
		common_info->hbm.hbm_gamma.green_60_hz[i * 2] =
			hbm_gamma[GAMMA_INDEX_GREEN_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.green_60_hz[(i * 2) + 1] =
			hbm_gamma[GAMMA_INDEX_GREEN_LOW + (i * 2)];
		common_info->hbm.hbm_gamma.blue_60_hz[i * 2] =
			hbm_gamma[GAMMA_INDEX_BLUE_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.blue_60_hz[(i * 2) + 1] =
			hbm_gamma[GAMMA_INDEX_BLUE_LOW + (i * 2)];
	}

	for (i = 0; i < rgb_size; i++)
		LCD_KIT_INFO("hbm 60 gamma %u = 0x%02X\n", i, hbm_gamma[i]);
	return LCD_KIT_OK;
}

static int display_engine_set_hbm_gamma_90hz_to_common_info(int panel_id,
	uint8_t *hbm_gamma, size_t rgb_size)
{
	size_t i;

	if (!hbm_gamma) {
		LCD_KIT_ERR("hbm_gamma is NULL\n");
		return LCD_KIT_FAIL;
	}
	if ((GAMMA_INDEX_BLUE_HIGH + HBM_GAMMA_SIZE) > rgb_size) {
		LCD_KIT_ERR("hbm_gamma index out of range\n");
		return LCD_KIT_FAIL;
	}

	/* The 2 values correspond to each other */
	for (i = 0; i < (HBM_GAMMA_SIZE / 2); i++) {
		common_info->hbm.hbm_gamma.red_90_hz[i * 2] =
			hbm_gamma[GAMMA_INDEX_RED_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.red_90_hz[(i * 2) + 1] =
			hbm_gamma[GAMMA_INDEX_RED_LOW + (i * 2)];
		common_info->hbm.hbm_gamma.green_90_hz[i * 2] =
			hbm_gamma[GAMMA_INDEX_GREEN_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.green_90_hz[(i * 2) + 1] =
			hbm_gamma[GAMMA_INDEX_GREEN_LOW + (i * 2)];
		common_info->hbm.hbm_gamma.blue_90_hz[i * 2] =
			hbm_gamma[GAMMA_INDEX_BLUE_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.blue_90_hz[(i * 2) + 1] =
			hbm_gamma[GAMMA_INDEX_BLUE_LOW + (i * 2)];
	}

	for (i = 0; i < rgb_size; i++)
		LCD_KIT_INFO("hbm 90 gamma %u = 0x%02X\n", i, hbm_gamma[i]);
	return LCD_KIT_OK;
}

void display_engine_local_hbm_gamma_read(void *hld)
{
	int panel_id = get_panel_id();
#if defined(LCD_KIT_ALPHA_LHBM)
	display_engine_local_hbm_gamma_read_ctrl(panel_id, hld);
	return;
#endif
	/* read rgb gamma, hbm_gamma structure: */
	/* [red_high, red_low, green_high, green_low, blue_high, blue_low] */
	/* read 60 hz gamma, number 3 corresponds to RGB */
	const size_t rgb_size = HBM_GAMMA_SIZE * 3;
	uint8_t hbm_gamma[rgb_size];
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!common_info->hbm.local_hbm_support)
		LCD_KIT_ERR("local_hbm not support\n");

	LCD_KIT_INFO("display_engine_local_hbm_gamma_read\n");
	/* init check_flag */
	common_info->hbm.hbm_gamma.check_flag = 0;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return;
	}

	if (adapt_ops->mipi_rx(panel_id, hld, hbm_gamma, rgb_size - 1,
		&common_info->hbm.hbm_60_hz_gamma_read_cmds)) {
		LCD_KIT_ERR("hbm 60 HZ gamma mipi_rx failed!\n");
		return;
	}
	if (display_engine_set_hbm_gamma_60hz_to_common_info(panel_id, hbm_gamma, rgb_size)) {
		LCD_KIT_ERR("display_engine_set_hbm_gamma_60hz_to_common_info error\n");
		return;
	}

	/* read 90 hz gamma */
	if (adapt_ops->mipi_rx(panel_id, hld, hbm_gamma, rgb_size - 1,
		&common_info->hbm.hbm_90_hz_gamma_read_cmds)) {
		LCD_KIT_ERR("hbm 90 HZ gamma mipi_rx failed!\n");
		return;
	}
	if (display_engine_set_hbm_gamma_90hz_to_common_info(panel_id, hbm_gamma, rgb_size)) {
		LCD_KIT_ERR("display_engine_set_hbm_gamma_90hz_to_common_info error\n");
		return;
	}
	display_engine_set_gamma_node_info(panel_id, &common_info->hbm.gamma_info);
}

static int display_engine_gamma_code_print(int panel_id)
{
	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	if ((int)common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen <
		CMDS_COLOR_MAX) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.
			cmds[color_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("circle_color_cmds.cmds[1].payload[9]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_HIGH]);
	LCD_KIT_DEBUG("circle_color_cmds.cmds[1].payload[10]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_LOW]);
	LCD_KIT_DEBUG("circle_color_cmds.cmds[1].payload[11]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_HIGH]);
	LCD_KIT_DEBUG("circle_color_cmds.cmds[1].payload[12]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_LOW]);
	LCD_KIT_DEBUG("circle_color_cmds.cmds[1].payload[13]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_HIGH]);
	LCD_KIT_DEBUG("circle_color_cmds.cmds[1].payload[14]=%d\n",
		common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_LOW]);
	return LCD_KIT_OK;
}

int display_engine_local_hbm_set_color_by_index(void *hld, uint32_t fps,
	uint32_t gamma_node_index)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int panel_id = get_panel_id();

	if (!common_info->hbm.local_hbm_support) {
		LCD_KIT_DEBUG("local_hbm not support\n");
		return LCD_KIT_OK;
	}
	if (common_info->hbm.circle_color_cmds.cmds == NULL ||
		common_info->hbm.hbm_em_configure_60hz_cmds.cmds == NULL ||
		common_info->hbm.hbm_em_configure_90hz_cmds.cmds == NULL) {
		LCD_KIT_ERR("cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	ret = display_engine_local_hbm_set_color_cmds_value_by_index(fps,
		gamma_node_index);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("display_engine_local_hbm_set_color_cmds_value_by_index"
			" error\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = display_engine_gamma_code_print(panel_id);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("display_engine_gamma_code_print error\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, hld, &(common_info->hbm.circle_color_cmds));
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx faild! ret = %d\n", ret);
	return ret;
}

static int display_engine_copy_color_cmds_to_common_info(int panel_id,
	struct color_cmds_rgb *color_cmds)
{
	if (!color_cmds) {
		LCD_KIT_ERR("color_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}

	/* circle_color_cmds.cmd_cnt's maximum value is 3 */
	if (common_info->hbm.circle_color_cmds.cmd_cnt != 3) {
		LCD_KIT_ERR("circle_color_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	if ((int)common_info->hbm.circle_color_cmds.cmds[color_cmd_index].dlen <
		CMDS_COLOR_MAX) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.
			cmds[color_cmd_index].dlen);
		return LCD_KIT_FAIL;
	}

	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_HIGH] = (char)color_cmds->red_payload[0];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_RED_LOW] = (char)color_cmds->red_payload[1];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_HIGH] = (char)color_cmds->green_payload[0];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_GREEN_LOW] = (char)color_cmds->green_payload[1];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_HIGH] = (char)color_cmds->blue_payload[0];
	common_info->hbm.circle_color_cmds.cmds[color_cmd_index].
		payload[CMDS_BLUE_LOW] = (char)color_cmds->blue_payload[1];

	return LCD_KIT_OK;
}

int display_engine_local_hbm_set_color_by_grayscale(void *hld, uint32_t fps,
	int grayscale)
{
	int ret;
	struct color_cmds_rgb color_cmds;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int panel_id = get_panel_id();
	if (common_info->hbm.circle_color_cmds.cmds == NULL) {
		LCD_KIT_ERR("circle_color_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("grayscale = %d\n", grayscale);
#if defined(LCD_KIT_ALPHA_LHBM)
	return display_engine_local_hbm_set_color_by_grayscale_ctrl(
		panel_id, hld, grayscale);
#endif
	ret = display_engine_set_color_cmds_by_grayscale(fps, &color_cmds,
		&common_info->hbm.gamma_info, grayscale);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("display_engine_set_color_cmds error\n");
		return LCD_KIT_FAIL;
	}

	ret = display_engine_copy_color_cmds_to_common_info(panel_id, &color_cmds);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("display_engine_copy_color_cmds_to_common_info error\n");
		return LCD_KIT_FAIL;
	}
	ret = display_engine_gamma_code_print(panel_id);
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

static int display_engine_set_em_configure(int panel_id, void *hld, uint32_t fps)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (fps == fps_normal) {
		ret = adapt_ops->mipi_tx(panel_id, hld,
			&(common_info->hbm.hbm_em_configure_60hz_cmds));
	} else if (fps == fps_medium) {
		ret = adapt_ops->mipi_tx(panel_id, hld,
			&(common_info->hbm.hbm_em_configure_90hz_cmds));
	} else {
		LCD_KIT_ERR("fps not support, fps:%d\n", fps);
		return LCD_KIT_FAIL;
	}
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);

	return ret;
}

int display_engine_local_hbm_set_dbv_in_lcd_kit(void *hld, uint32_t dbv, uint32_t fps)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int panel_id = get_panel_id();

	if (!hld) {
		LCD_KIT_ERR("hld is NULL\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("set dbv, dbv value:%d\n", dbv);
	if (common_info->hbm.hbm_dbv_cmds.cmds == NULL) {
		LCD_KIT_ERR("hbm_dbv_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}

	ret = display_engine_set_em_configure(panel_id, hld, fps);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("display_engine_set_em_configure error\n");
		return LCD_KIT_FAIL;
	}

	/* hbm_dbv_cmds.cmd_cnt's maximum value is 2 */
	if (common_info->hbm.hbm_dbv_cmds.cmd_cnt != 2) {
		LCD_KIT_ERR("hbm_dbv_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	/* 25 is the 16th of the payload of dbv */
	if ((int)common_info->hbm.hbm_dbv_cmds.cmds[1].dlen <= 25) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.hbm_dbv_cmds.cmds[1].dlen);
		return LCD_KIT_FAIL;
	}

	/* 24 and 25 are dbv payloads */
	common_info->hbm.hbm_dbv_cmds.cmds[1].payload[24] = (dbv >> 8) & 0x0f;
	common_info->hbm.hbm_dbv_cmds.cmds[1].payload[25] = dbv & 0xff;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.hbm_dbv_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx faild! ret = %d\n", ret);
	return ret;
}

int display_engine_local_hbm_set_dbv_is_on_in_lcd_kit(void *hld, uint32_t dbv,
	uint32_t dbv_threshold, uint32_t fps, bool is_on)
{
#if defined(LCD_KIT_ALPHA_LHBM)
	int panel_id = get_panel_id();
	display_engine_local_hbm_set_dbv_ctrl(panel_id, dbv, dbv_threshold, is_on);
#endif
	return LCD_KIT_OK;
}

int display_engine_local_hbm_set_circle_in_lcd_kit(void *hld, bool is_on)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int panel_id = get_panel_id();

	if (!hld) {
		LCD_KIT_ERR("hld is null\n");
		return LCD_KIT_FAIL;
	}

#if defined(LCD_KIT_ALPHA_LHBM)
	ret = display_engine_circle_code_print_ctrl(panel_id);
	if (ret != LCD_KIT_OK)
		return LCD_KIT_FAIL;
#endif
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!is_on) {
		LCD_KIT_DEBUG("circle exit\n");
		if (common_info->hbm.exit_circle_cmds.cmds == NULL) {
			LCD_KIT_ERR("enter_alpha_cmds.cmds is NULL\n");
			return LCD_KIT_FAIL;
		}
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.exit_circle_cmds);
	} else {
		if (common_info->hbm.enter_circle_cmds.cmds == NULL) {
			LCD_KIT_ERR("enter_alpha_cmds.cmds is NULL\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_DEBUG("circle enter\n");
		ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->hbm.enter_circle_cmds);
	}
	return ret;
}

u32 display_engine_local_hbm_get_mipi_level(void)
{
#ifdef LV_GET_LCDBK_ON
	return mipi_level;
#else
	return 0;
#endif
}

int display_engine_enter_ddic_alpha(void *hld, uint32_t alpha)
{
	int panel_id = get_panel_id();
#if defined(LCD_KIT_ALPHA_LHBM)
	return display_engine_ddic_alpha_is_on_ctrl(panel_id, hld, alpha, true);
#endif
	int ret;
	int payload_num;
	int cmd_cnt;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct lcd_kit_dsi_panel_cmds *enter_alpha_cmds = &common_info->ddic_alpha.enter_alpha_cmds;
	if (enter_alpha_cmds->cmds == NULL) {
		LCD_KIT_ERR("enter_alpha_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	cmd_cnt = enter_alpha_cmds->cmd_cnt;
	if (cmd_cnt <= 0 || cmd_cnt > cmd_cnt_max) {
		LCD_KIT_ERR("enter_alpha_cmds's cmd_cnt is invalid, cmd_cnt:%d\n", cmd_cnt);
		return LCD_KIT_FAIL;
	}
	payload_num = (int)enter_alpha_cmds->cmds[cmd_cnt - 1].dlen;
	if (payload_num < payload_num_min || payload_num > payload_num_max) {
		LCD_KIT_ERR("enter_alpha_cmds.cmds's payload_num is invalid, payload_num:%d\n",
			payload_num);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("cmd_cnt = %d, payload_num = %d\n", cmd_cnt, payload_num);

	/* Sets the upper eight bits of the alpha. */
	if (common_info->ddic_alpha.alpha_with_enable_flag == 0)
		enter_alpha_cmds->cmds[cmd_cnt - 1].payload[payload_num - 2] = (alpha >> 8) & 0x0f;
	else
		enter_alpha_cmds->cmds[cmd_cnt - 1].payload[payload_num - 2] =
			((alpha >> 8) & 0x0f) | 0x10;

	/* Sets the lower eight bits of the alpha. */
	enter_alpha_cmds->cmds[cmd_cnt - 1].payload[payload_num - 1] = alpha & 0xff;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, hld, enter_alpha_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("adapt_ops->mipi_tx error\n");

	return ret;
}

int display_engine_exit_ddic_alpha(void *hld, int alpha)
{
	int panel_id = get_panel_id();
#if defined(LCD_KIT_ALPHA_LHBM)
	return display_engine_ddic_alpha_is_on_ctrl(panel_id, hld, alpha, false);
#endif
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (common_info->ddic_alpha.exit_alpha_cmds.cmds == NULL) {
		LCD_KIT_ERR("enter_alpha_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(panel_id, hld, &common_info->ddic_alpha.exit_alpha_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("adapt_ops->mipi_tx error\n");
	return ret;
}

u32 display_engine_alpha_get_support_in_lcd_kit(void)
{
	int panel_id = get_panel_id();

	return common_info->ddic_alpha.alpha_support;
}

u32 display_engine_get_force_delta_bl_update_support(void)
{
	int panel_id = get_panel_id();

	return common_info->force_delta_bl_update_support;
}

u32 display_engine_local_hbm_get_support_in_lcd_kit(void)
{
	int panel_id = get_panel_id();
	LCD_KIT_INFO("lcdkit local_hbm_support=%u\n", common_info->hbm.local_hbm_support);
	return common_info->hbm.local_hbm_support;
}

u32 display_engine_hbm_alpha_gain_get_support_in_lcd_kit(void)
{
	int panel_id = get_panel_id();
	LCD_KIT_INFO("lcdkit hbm_alpha_gain_support=%u\n", common_info->hbm.hbm_alpha_gain_support);
	return common_info->hbm.hbm_alpha_gain_support;
}

int display_engine_local_hbm_alpha_circle_with_alpha_and_dbv(void *hld, uint32_t fps,
	bool is_on, uint32_t alpha, uint32_t dbv)
{
	int panel_id = get_panel_id();

	LCD_KIT_DEBUG("local hbm mipi_level src:%d\n", dbv);
	if (!hld) {
		LCD_KIT_ERR("hld is null\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("alpha = %d\n", alpha);
	if (!is_on) {
		LCD_KIT_DEBUG("alpha circle exit\n");
		display_engine_exit_ddic_alpha(hld, LCD_KIT_ALPHA_DEFAULT);
		display_engine_local_hbm_set_circle_in_lcd_kit(hld, false);
	} else {
		if (alpha > LCD_KIT_ALPHA_DEFAULT)
			alpha = LCD_KIT_ALPHA_DEFAULT;

		LCD_KIT_DEBUG("alpha circle enter\n");
		display_engine_local_hbm_set_dbv_in_lcd_kit(hld, dbv, fps);
		display_engine_enter_ddic_alpha(hld, alpha);
		display_engine_local_hbm_set_circle_in_lcd_kit(hld, true);
	}
	return LCD_KIT_OK;
}
