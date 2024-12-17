/*
 * lcd_kit_disp.c
 *
 * lcdkit display function for lcd driver
 *
 * Copyright (c) 2022-2024 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_disp.h"
#include "lcd_kit_panel.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_sysfs_dkmd.h"
#include "lcd_kit_power.h"
#include "lcd_kit_ioctl.h"
#include "lcd_kit_local_hbm.h"
#include "lcd_kit_panel_pfm.h"
#include "lcd_kit_ext_power.h"
#include "lcd_kit_power_key.h"
#include "lcd_kit_displayengine.h"
#include "dpu_aod_device.h"
#include "lcd_kit_elvss_control.h"
#include "lcd_kit_safe_frm_rate.h"
#ifdef LCD_KIT_DEBUG_ENABLE
#include "lcd_kit_dbg.h"
#endif
#ifdef CONFIG_DFX_ZEROHUNG
#include <platform/trace/hooks/zerohung.h>
#endif
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_fac.h"
#endif
#include "dkmd_lcd_interface.h"
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_INPUTHUB_20
#include <huawei_platform/inputhub/sensor_feima_ext.h>
#endif
#ifdef CONFIG_POWER_DUBAI
#include <huawei_platform/log/log_jank.h>
#endif
#include <linux/errno.h>
#include <linux/printk.h>
#ifdef CONFIG_HISYSEVENT
#include <dfx/hiview_hisysevent.h>
#endif
enum {
	LCD_KIT_SET_DUBAI_HISYSEVENT_POWER_OFF = 0,
	LCD_KIT_SET_DUBAI_HISYSEVENT_POWER_ON
};
#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *lcd_dclient = NULL;
#endif
static int g_init_order[MAX_PANEL] = { PRIMARY_PANEL, SECONDARY_PANEL };
static char *g_disp_compatibles[MAX_PANEL] = { "lcd_panel_type", "sec_lcd_panel_type" };
static struct lcd_kit_disp_info g_lcd_kit_disp_info[MAX_PANEL];
static struct dpu_panel_info g_dpu_pinfo[MAX_PANEL];
struct lcd_kit_public_info g_lcd_kit_public_info = {
	NORMAL_TYPE,
	PRIMARY_PANEL,
};
static struct lcd_kit_disp_lock g_lcd_kit_disp_lock[MAX_PANEL];
static struct lcd_kit_panel_params g_lcd_kit_panels[] = {
	{3000, 0x00, "190_c07_7p847"},
	{3000, 0x400, "190_c07_1_7p847"},
	{3000, 0x04, "310_c07_7p847"},
	{3000, 0x404, "310_c07_1_7p847"},
	{3000, 0x04, "310_c07_7p847"},
	{3000, 0x15, "default_7p847"},
};

static int lcd_kit_updt_fps(int panel_id, unsigned int fps);
static int lcd_kit_updt_fps_te_mode(int panel_id, unsigned int fps, bool is_need_delay);
static int lcd_kit_fake_power_off(int panel_id);

static int lcd_kit_get_init_display_length()
{
	if (PUBLIC_INFO->product_type == NORMAL_TYPE)
		return 1;
	else
		return MAX_PANEL;
}

#if defined(CONFIG_DKMD_DPU_AOD)
extern uint32_t dpu_aod_get_panel_id(void);
#endif

void lcd_kit_sensor_notify_panel_version(uint32_t panel_id,
	char *panel_version)
{
	if (!panel_version) {
		LCD_KIT_ERR("invaild params\n");
		return;
	}

	if (strncpy_s((char *)panel_version,
		sizeof(panel_version),
		(char *)disp_info->panel_version.lcd_panel_version,
		sizeof(disp_info->panel_version.lcd_panel_version)) != EOK) {
		LCD_KIT_ERR("strncpy_s fail\n");
		return;
	}
}

int lcd_kit_get_active_panel_id(void)
{
	int panel_len = lcd_kit_get_init_display_length();
	int i;
	int panel_id;

	for (i = 0; i < panel_len; i++) {
		panel_id = g_init_order[i];
		if (lcd_kit_get_panel_on_state(panel_id))
			return panel_id;
	}
#if defined(CONFIG_DKMD_DPU_AOD)
	return dpu_aod_get_panel_id();
#else
	return PRIMARY_PANEL;
#endif
}

int lcd_kit_get_display_type(void)
{
	return PUBLIC_INFO->product_type;
}

struct lcd_kit_disp_info *lcd_kit_get_disp_info(int panel_id)
{
	return &g_lcd_kit_disp_info[panel_id];
}

struct dpu_panel_info *lcd_kit_get_dpu_pinfo(int panel_id)
{
	if (panel_id >= MAX_PANEL || panel_id < 0) {
		LCD_KIT_ERR("not support panel_id %d \n", panel_id);
		return NULL;
	}

	return &g_dpu_pinfo[panel_id];
}

struct lcd_kit_public_info *lcd_kit_get_public_info(void)
{
	return &g_lcd_kit_public_info;
}

struct lcd_kit_disp_lock *lcd_kit_get_disp_lock(int panel_id)
{
	return &g_lcd_kit_disp_lock[panel_id];
}

static int lcd_kit_set_fastboot(int panel_id)
{
	// lcd panel version
	if (disp_info->panel_version.support) {
		if (!lcd_kit_panel_version_init(panel_id))
			LCD_KIT_INFO("read panel version successful\n");
		else
			LCD_KIT_INFO("read panel version fail\n");
	}
	return LCD_KIT_OK;
}

static void lcd_kit_fps_handle_on(int panel_id)
{
	struct dpu_panel_info *pinfo = NULL;

	pinfo = lcd_kit_get_dpu_pinfo(panel_id);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	if (!disp_info->fps.support) {
		LCD_KIT_DEBUG("not support fps\n");
		return;
	}
	if (pinfo->dfr_info.oled_info.oled_type == LTPS)
		lcd_kit_updt_fps(panel_id, disp_info->fps.current_fps);

	if (disp_info->updt_fps_te.support)
		lcd_kit_updt_fps_te_mode(panel_id, disp_info->fps.current_fps, true);
}

static void lcd_kit_hs_handle(int panel_id)
{
	/* fps on handle */
	lcd_kit_fps_handle_on(panel_id);
#if defined(CONFIG_DKMD_DPU_AOD)
	lcd_kit_set_aod_panel_state(panel_id, 1);
#endif
	check_panel_on_state();
}

void lcd_on_time_record(int panel_id)
{
	static struct timespec64 end_time;
	uint32_t delta;
	int i;

	ktime_get_real_ts64(&end_time);
	/* change s to ms */
	delta = (uint32_t)((end_time.tv_sec - disp_info->lcd_on_time.start_time.tv_sec) * 1000 +
		(end_time.tv_nsec - disp_info->lcd_on_time.start_time.tv_nsec) / 1000000);
	LCD_KIT_INFO("delta is %u\n", delta);
	for (i = TIME_COUNT - 1; i > 0; i--)
		disp_info->lcd_on_time.delta[i] = disp_info->lcd_on_time.delta[i - 1];
	disp_info->lcd_on_time.delta[0] = delta;
}

static void lcd_update_power_seq(int panel_id)
{
	disp_info->lcd_power_seq = LCD_KIT_POWER_ON_SEQ;
	ktime_get_real_ts64(&disp_info->lcd_on_time.start_time);
}

#ifdef CONFIG_HISYSEVENT
static void lcd_kit_hiview_hisysevent(const char *domain, const char *casename, int level, const char *value)
{
	struct hiview_hisysevent *event = NULL;
	int ret;

	event = hisysevent_create(domain, casename, BEHAVIOR);
	if (!event) {
		LCD_KIT_ERR("failed to create event\n");
		return;
	}

	if (value == NULL)
		ret = hisysevent_put_integer(event, "MODULE", level);
	else
		ret = hisysevent_put_string(event, "MODULE", value);

	if (ret != 0) {
		LCD_KIT_ERR("failed to put value to event, ret=%d\n", ret);
		goto hisysevent_end;
	}

	ret = hisysevent_write(event);
	if (ret != 0)
		LCD_KIT_ERR("lcd_kit_hiview_hisysevent fail, %s\n", casename);

hisysevent_end:
	hisysevent_destroy(&event);
}
#endif

static void lcd_kit_set_dubai_hisysevent_power_state(int panel_id, int enable)
{
#ifdef CONFIG_POWER_DUBAI
	if (enable)
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_ON, "%s", "LCD_POWER_ON");
	else
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_OFF, "%s", "LCD_POWER_OFF");
#endif
#ifdef CONFIG_HISYSEVENT
	if (enable)
		lcd_kit_hiview_hisysevent("LCD", "LCD_POWER_ON", 0, common_info->panel_name);
	else
		lcd_kit_hiview_hisysevent("LCD", "LCD_POWER_OFF", 0, common_info->panel_name);
#endif
}

static void lcd_kit_set_dubai_hisysevent_backlight(int panel_id, unsigned int bl_level)
{
	if ((disp_info->jank_last_bl_level == 0) && (bl_level != 0)) {
#ifdef CONFIG_POWER_DUBAI
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_ON, "LCD_BACKLIGHT_ON,%u", bl_level);
#endif
#ifdef CONFIG_HISYSEVENT
		lcd_kit_hiview_hisysevent("LCD", "LCD_BACKLIGHT_ON", (int)bl_level, NULL);
#endif
		disp_info->jank_last_bl_level = bl_level;
	} else if ((disp_info->jank_last_bl_level != 0) && (bl_level == 0)) {
#ifdef CONFIG_POWER_DUBAI
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_OFF, "LCD_BACKLIGHT_OFF");
#endif
#ifdef CONFIG_HISYSEVENT
		lcd_kit_hiview_hisysevent("LCD", "LCD_BACKLIGHT_OFF", (int)bl_level, NULL);
#endif
		disp_info->jank_last_bl_level = bl_level;
	}
}

static void lcd_kit_prepare_enable(int panel_id)
{
	if (disp_info->lcd_power_seq == LCD_KIT_POWER_OFF) {
		disp_info->lcd_power_seq = LCD_KIT_POWER_ON_SEQ;
	} else if ((disp_info->lcd_power_seq == LCD_KIT_POWER_ON ||
		disp_info->lcd_power_seq == LCD_KIT_FAKE_POWER_OFF) &&
		common_ops->panel_mipi_switch) {
			disp_info->lcd_power_seq = LCD_KIT_POWER_DBL_ON_SEQ;
			if (common_ops->start_temper_hrtimer &&
				common_info->temper_thread.enable &&
				common_info->temper_thread.dark_enable) {
				common_ops->start_temper_hrtimer(panel_id);
				LCD_KIT_INFO("fake power on, temper timer is starting\n");
			}
		}
	LCD_KIT_INFO("disp_info->lcd_power_seq is %u\n", disp_info->lcd_power_seq);
	if (disp_info->lcd_power_seq == LCD_KIT_POWER_DBL_ON_SEQ) {
		if (lcd_kit_get_power_status(panel_id) == 0) {
			disp_info->lcd_power_seq = LCD_KIT_POWER_ON_SEQ;
		} else if (common_ops->panel_on_tp) {
			LCD_KIT_INFO("tp on notify\n");
			common_ops->panel_on_tp(panel_id, NULL);
		}
	}
}

static int lcd_kit_on(int panel_id)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("fb%d, +!\n", panel_id);
	if (disp_info->lcd_power_seq == LCD_KIT_POWER_OFF)
		lcd_update_power_seq(panel_id);

	lcd_kit_prepare_enable(panel_id);

	switch (disp_info->lcd_power_seq) {
	case LCD_KIT_POWER_ON_SEQ:
		wait_power_off_done(panel_id);
		/* send power on */
		if (common_ops->panel_power_on)
			ret = common_ops->panel_power_on(panel_id, NULL);
		lcd_kit_set_dubai_hisysevent_power_state(panel_id, LCD_KIT_SET_DUBAI_HISYSEVENT_POWER_ON);
		/* set next step */
		disp_info->lcd_power_seq = LCD_KIT_POWER_ON_LP_SEQ;
		break;
	case LCD_KIT_POWER_ON_LP_SEQ:
		/* send mipi command by low power */
		if (common_ops->panel_on_lp)
			ret = common_ops->panel_on_lp(panel_id, NULL);

		/* set next step */
		disp_info->lcd_power_seq = LCD_KIT_POWER_ON_HS_SEQ;
		break;
	case LCD_KIT_POWER_ON_HS_SEQ:
		/* send mipi command by high speed */
		if (common_ops->panel_on_hs)
			ret = common_ops->panel_on_hs(panel_id, NULL);
		/* set next step */
		disp_info->lcd_power_seq = LCD_KIT_POWER_ON;
		lcd_kit_hs_handle(panel_id);
		lcd_on_time_record(panel_id);
		break;
	case LCD_KIT_POWER_DBL_ON_SEQ:
		disp_info->lcd_power_seq = LCD_KIT_POWER_DBL_ON_LP_SEQ;
		ret = common_ops->panel_mipi_switch(panel_id, NULL);
		lcd_kit_set_dubai_hisysevent_power_state(panel_id, LCD_KIT_SET_DUBAI_HISYSEVENT_POWER_ON);
		break;
	case LCD_KIT_POWER_DBL_ON_LP_SEQ:
		disp_info->lcd_power_seq = LCD_KIT_POWER_DBL_ON_HS_SEQ;
		break;
	case LCD_KIT_POWER_DBL_ON_HS_SEQ:
		disp_info->lcd_power_seq = LCD_KIT_POWER_ON;
		break;
	default:
		break;
	}

	// clean skip power off
	lcd_kit_set_skip_power_on_off(false);
	LCD_KIT_INFO("fb%d, -!\n", panel_id);
	return ret;
}

static void lcd_kit_hs_off_handle(int panel_id)
{
#if defined(CONFIG_DKMD_DPU_AOD)
	/* if aod no panel off, clear lcd status */
	if (lcd_kit_get_power_status(panel_id))
		lcd_kit_set_aod_panel_state(panel_id, 0);
#endif
}

static void clear_elvss_ctrl(int panel_id)
{
	int bl_level = 0;
	struct dpu_elvss_state *el_state = NULL;

	/* el ctrl clear vsync backlight state */
	need_upate_bl = 0;
	if (lcd_kit_el_ctrl_is_support(panel_id, bl_level)) {
		el_state = get_elvss_state(panel_id);
		if (!el_state) {
			LCD_KIT_ERR("panel elvss_state is null");
			return;
		}
#if defined(CONFIG_DKMD_DPU_AOD)
		if (get_lcd_always_on() != 0 && el_state->cur_ddic_enable) {
			LCD_KIT_INFO("reset_elvss_state CONFIG_DKMD_DPU_AOD");
			lcd_kit_el_ctrl_close(panel_id);
		}
#endif
		if (disp_info->lcd_power_seq >= LCD_KIT_POWER_OFF_HS_SEQ)
			return;
		reset_elvss_state(panel_id);
	}
}

static void lcd_kit_prepare_disable(int panel_id)
{
	if (common_ops->stop_temper_hrtimer &&
		common_info->temper_thread.enable &&
		common_info->temper_thread.dark_enable)
		common_ops->stop_temper_hrtimer(panel_id);
	clear_elvss_ctrl(panel_id);
	if (disp_info->lcd_power_seq != LCD_KIT_POWER_ON) {
		LCD_KIT_INFO("not power on power seq\n");
		return;
	}
#if defined(CONFIG_DKMD_DPU_AOD)
	if (get_lcd_always_on() != 0) {
		lcd_kit_fake_power_off(panel_id);
	} else {
		common_info->fake_power_off_state = false;
	}
#endif
	if (disp_info->lcd_power_seq == LCD_KIT_POWER_ON)
		disp_info->lcd_power_seq = LCD_KIT_POWER_OFF_HS_SEQ;
}

static int lcd_kit_update_fw(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_tcon_update *upd = &disp_info->update_info;
	struct lcd_kit_panel_ops *ops = lcd_kit_panel_get_ops();

	if (!upd || !upd->support) {
		LCD_KIT_DEBUG("upd is null or not support\n");
		return LCD_KIT_FAIL;
	}

	if (upd->tcon_upd_switch == TCON_UPDATE_DISABLE) {
		LCD_KIT_INFO("upd switch is disable\n");
		return LCD_KIT_FAIL;
	}

	if (ops && ops->lcd_kit_update_fw) {
		ret = ops->lcd_kit_update_fw(panel_id);
		if (ret) {
			LCD_KIT_ERR("failed\n");
			return LCD_KIT_FAIL;
		}
	}

	LCD_KIT_INFO("success\n");
	return ret;
}

static int lcd_kit_off(int panel_id)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("fb%d, +!\n", panel_id);
	lcd_kit_prepare_disable(panel_id);

	switch (disp_info->lcd_power_seq) {
	case LCD_KIT_POWER_OFF_HS_SEQ:
		lcd_kit_update_fw(panel_id);
		/* send mipi command by high speed */
		if (common_ops->panel_off_hs)
			ret = common_ops->panel_off_hs(panel_id, NULL);

		/* set next step */
		disp_info->lcd_power_seq = LCD_KIT_POWER_OFF_LP_SEQ;
		lcd_kit_set_dubai_hisysevent_power_state(panel_id, LCD_KIT_SET_DUBAI_HISYSEVENT_POWER_OFF);
		break;
	case LCD_KIT_POWER_OFF_LP_SEQ:
		/* send mipi command by low power */
		if (common_ops->panel_off_lp)
			ret = common_ops->panel_off_lp(panel_id, NULL);

		/* set next step */
		disp_info->lcd_power_seq = LCD_KIT_POWER_OFF_SEQ;
		break;
	case LCD_KIT_POWER_OFF_SEQ:
		lcd_kit_power_off_optimize_handle(panel_id);
		/* send mipi command by high speed */
		if (common_ops->panel_power_off)
			ret = common_ops->panel_power_off(panel_id, NULL);
		lcd_kit_hs_off_handle(panel_id);
		/* set next step */
		disp_info->lcd_power_seq = LCD_KIT_POWER_OFF;
		break;
	default:
		break;
	}

	LCD_KIT_INFO("fb%d, -!\n", panel_id);
	return ret;
}

static int lcd_kit_fake_power_off(int panel_id)
{
	LCD_KIT_INFO("+\n");
	common_info->fake_power_off_state = true;
	if (disp_info->lcd_power_seq == LCD_KIT_FAKE_POWER_OFF)
		return LCD_KIT_OK;
	display_engine_fake_off_handle(panel_id);
	disp_info->lcd_power_seq = LCD_KIT_FAKE_POWER_OFF;
	lcd_kit_set_aod_panel_state(panel_id, 1);
	lcd_kit_set_dubai_hisysevent_backlight(panel_id, 0);
	lcd_kit_set_dubai_hisysevent_power_state(panel_id, LCD_KIT_SET_DUBAI_HISYSEVENT_POWER_OFF);
#ifdef CONFIG_DFX_ZEROHUNG
	trace_hck_hung_wp_screen_fake_power_off(0);
#endif
	if (common_ops->panel_off_tp) {
		LCD_KIT_INFO("tp off notify\n");
		common_ops->panel_off_tp(panel_id, NULL);
	}
	LCD_KIT_INFO("-\n");
	return LCD_KIT_OK;
}

static int lcd_kit_hbm_backlight_handle(int panel_id, unsigned int bl_level)
{
	int ret = LCD_KIT_OK;
	unsigned int hbm_level;
	unsigned int bl_max = common_info->backlight.bl_max;

	if (bl_level > common_info->hbm_backlight.normal_bl_max) {
		hbm_level = display_engine_get_hbm_bl(panel_id, bl_level);
		if (common_ops->enter_hbm && common_info->hbm_backlight.hbm_status == LCD_KIT_HBM_OFF)
			ret = common_ops->enter_hbm(panel_id, NULL);
		if (common_ops->set_hbm_backlight)
			ret = common_ops->set_hbm_backlight(panel_id, NULL, hbm_level);
	} else {
		if (common_ops->exit_hbm && common_info->hbm_backlight.hbm_status == LCD_KIT_HBM_ON)
			ret = common_ops->exit_hbm(panel_id, NULL);
		common_info->backlight.bl_max = common_info->hbm_backlight.normal_bl_max;
		ret = lcd_kit_mipi_set_backlight(panel_id, bl_level);
		common_info->backlight.bl_max = bl_max;
	}

	return ret;
}

static int lcd_kit_set_backlight_sub(int panel_id, unsigned int bl_level)
{
	int ret = LCD_KIT_OK;
	int bl_type;

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
#if defined(CONFIG_DKMD_DPU_AOD)
	if (bl_level == 0 && (disp_info->lcd_power_seq == LCD_KIT_FAKE_POWER_OFF ||
		get_lcd_always_on() != 0)) {
		LCD_KIT_INFO("lcd fake off\n");
		return LCD_KIT_OK;
	}
#endif

	bl_type = lcd_kit_get_bl_set_type(panel_id);

	switch (bl_type) {
	case BL_SET_BY_PWM:
		break;
	case BL_SET_BY_BLPWM:
		ret = lcd_kit_blpwm_set_backlight(panel_id, bl_level);
		break;
	case BL_SET_BY_MIPI:
		if (common_info->hbm_backlight.support)
			ret = lcd_kit_hbm_backlight_handle(panel_id, bl_level);
		else
			ret = lcd_kit_mipi_set_backlight(panel_id, bl_level);
		break;
	default:
		LCD_KIT_ERR("not support bl_type\n");
		ret = -1;
		break;
	}

	int tuned_level;
	tuned_level = display_engine_local_hbm_get_mipi_level();
	if (tuned_level == 0)
		tuned_level = bl_level;
	display_engine_compensation_set_dbv(bl_level, tuned_level, panel_id);
#ifdef CONFIG_DFX_ZEROHUNG
	if (ret == LCD_KIT_OK)
		trace_hck_hung_wp_screen_setbl(bl_level);
#endif
#ifdef CONFIG_INPUTHUB_20
#ifdef LCD_ALSC_ENABLE
	save_light_to_sensorhub_panel_id(bl_level, bl_level, panel_id);
#endif
#endif
	lcd_kit_set_dubai_hisysevent_backlight(panel_id, bl_level);
	LCD_KIT_INFO("backlight_level%d = %d\n", panel_id, bl_level);

	return ret;
}

static int lcd_kit_set_backlight(int panel_id, unsigned int bl_level)
{
	int ret = LCD_KIT_OK;
	unsigned int dsi0_index;
	unsigned int connector_id;

	el_ctrl_backlight = bl_level;
	if (lcd_kit_el_ctrl_is_support(panel_id, bl_level)) {
		LCD_KIT_DEBUG("el ctrl support\n");
		need_upate_bl = 1;
		return ret;
	}

	ret = lcd_kit_set_backlight_sub(panel_id, bl_level);
	if (ret == LCD_KIT_OK) {
		connector_id = DPU_PINFO->connector_id;
		ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
		if (ret) {
			LCD_KIT_ERR("get dsi0 index error\n");
			return LCD_KIT_FAIL;
		}
		dkmd_dfr_send_refresh(dsi0_index, DPU_PINFO->type);
	}
	return ret;
}

static int lcd_kit_vsync_trigger(int panel_id)
{
	int ret = LCD_KIT_OK;
	unsigned int current_bl_level = el_ctrl_backlight;
	bool need_refresh = false;
	unsigned int dsi0_index;
	unsigned int connector_id;

	if (!lcd_kit_el_ctrl_is_support(panel_id, current_bl_level)) {
		LCD_KIT_DEBUG("el ctrl not support\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_DEBUG("current backlight level = %d\n", current_bl_level);
	if (need_upate_bl) {
		ret = lcd_kit_set_backlight_sub(panel_id, current_bl_level);
		if (ret) {
			LCD_KIT_ERR("backlight tx failed\n");
			need_upate_bl = 0;
			return LCD_KIT_FAIL;
		}
		need_upate_bl = 0;
		need_refresh = true;
	}
	ret = lcd_kit_set_el_ctrl_cmds(panel_id, current_bl_level);
	if (!need_refresh && ret == LCD_KIT_OK)
		need_refresh = true;

	if (need_refresh) {
		connector_id = DPU_PINFO->connector_id;
		ret = lcd_kit_get_dsi_index(&dsi0_index, connector_id);
		if (ret) {
			LCD_KIT_ERR("get dsi0 index error\n");
			return LCD_KIT_FAIL;
		}
		dkmd_dfr_send_refresh(dsi0_index, DPU_PINFO->type);
	}

	return ret;
}

static int lcd_kit_get_ddic_cmds(int panel_id,
	const struct disp_effect_params* effect_params, struct dsi_cmds *dsi_cmds)
{
	int i;
	struct disp_effect_item item;
	int bl_level;

	if (!effect_params || !dsi_cmds) {
		LCD_KIT_ERR("effect_params or dsi_cmds is null\n");
		return LCD_KIT_FAIL;
	}

	if (effect_params->effect_num > EFFECT_PARAMS_MAX_NUM) {
		LCD_KIT_ERR("effect num abnormal, %u\n", effect_params->effect_num);
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("effect_num is %u\n", effect_params->effect_num);

	for (i = 0; i < effect_params->effect_num; i++) {
		item =  effect_params->params[i];
		if (item.effect_type == DISP_EFFECT_WITH_BL) {
			bl_level = item.effect_values[0];
			lcd_kit_get_bl_cmds(panel_id, bl_level, dsi_cmds);
		} else if (item.effect_type == DISP_EFFECT_TE2_FIXED) {
			lcd_kit_get_fixed_te2_cmds(panel_id, dsi_cmds);
		} else if (item.effect_type == DISP_EFFECT_TE2_FOLLOWED) {
			lcd_kit_get_follow_te2_cmds(panel_id, dsi_cmds);
		}
	}
	return LCD_KIT_OK;
}


static int lcd_kit_trace_screen_bl(int panel_id, unsigned int bl_level)
{
#ifdef CONFIG_DFX_ZEROHUNG
	trace_hck_hung_wp_screen_setbl(bl_level);
#endif
#ifdef CONFIG_INPUTHUB_20
#ifdef LCD_ALSC_ENABLE
	save_light_to_sensorhub_panel_id(bl_level, bl_level, panel_id);
#endif
#endif
	lcd_kit_set_dubai_hisysevent_backlight(panel_id, bl_level);
	LCD_KIT_INFO("backlight_level%d = %d\n", panel_id, bl_level);
	display_engine_brightness_update(bl_level);

	int tuned_level;
	tuned_level = display_engine_local_hbm_get_mipi_level();
	if (tuned_level == 0)
		tuned_level = bl_level;
	display_engine_compensation_set_dbv(bl_level, tuned_level, panel_id);
	return LCD_KIT_OK;
}

static int lcd_kit_post_process(struct device *dev)
{
	/* create sysfs */
	LCD_KIT_INFO("post process begin\n");
	lcd_kit_sysfs_init(dev);

	LCD_KIT_INFO("post process end\n");
	return LCD_KIT_OK;
}

static int lcd_kit_esd_check(int panel_id)
{
	int ret = LCD_KIT_OK;
	unsigned int esd_support;

	if (!disp_info->esd_enable) {
		LCD_KIT_INFO("ESD disable\n");
		return ret;
	}

	if (runmode_is_factory())
		esd_support = common_info->esd.fac_esd_support;
	else
		esd_support = common_info->esd.support;

	if (esd_support && common_ops->esd_handle)
		ret = common_ops->esd_handle(panel_id, NULL);

	if (runmode_is_factory() && disp_info->check_mipi.support) {
		LCD_KIT_INFO("check mipi start\n");
		lcd_kit_check_mipi_error(panel_id);
	}

	return ret;
}

static int lcd_kit_set_display_region(int panel_id, struct dkmd_rect *dirty)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("set display_region\n");
	if (common_ops->dirty_region_handle)
		ret = common_ops->dirty_region_handle(panel_id, NULL,
			(struct region_rect *)dirty);

	return ret;
}

static int lcd_kit_common_probe(int panel_id,
	struct device_node *np)
{
	struct dpu_panel_info *pinfo = NULL;

	pinfo = lcd_kit_get_dpu_pinfo(panel_id);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}

	memset(pinfo, 0, sizeof(struct dpu_panel_info));
	pinfo->product_type = PUBLIC_INFO->product_type;
	/* common init */
	if (common_ops->common_init)
		common_ops->common_init(panel_id, np);

	/* utils init */
	lcd_kit_utils_init(panel_id, np, pinfo);
#ifdef LCD_FACTORY_MODE
	lcd_factory_init(panel_id, np);
#endif
#ifdef LCD_KIT_DEBUG_ENABLE
	/* debugfs init */
	lcd_kit_debug_init();
	lcd_kit_debugfs_init();
#endif
	lcd_kit_register_power_key_notify(panel_id);

	return LCD_KIT_OK;
}

static int lcd_kit_set_fps_scence(int panel_id, unsigned int current_fps)
{
	LCD_KIT_INFO("current_fps = %u\n", current_fps);
	disp_info->fps.current_fps = current_fps;
	common_info->fps_on.current_fps = disp_info->fps.current_fps;
	return LCD_KIT_OK;
}

static int fps_cmd_send(int panel_id, struct lcd_kit_dsi_panel_cmds *fps_cmds)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->mipi_tx_no_lock == NULL) {
		LCD_KIT_ERR("mipi_tx_no_lock is NULL\n");
		return LCD_KIT_FAIL;
	}

	return adapt_ops->mipi_tx_no_lock(panel_id, NULL, fps_cmds);
}

static int lcd_kit_updt_fps(int panel_id, unsigned int fps)
{
	int ret = LCD_KIT_OK;
	int table_num;
	int i;
	struct ltps_fps_cmd_table fps_lock_cmd[] = {
		{ FPS_30, &disp_info->fps.fps_to_30_cmds },
		{ FPS_60, &disp_info->fps.fps_to_60_cmds },
		{ FPS_90, &disp_info->fps.fps_to_90_cmds },
		{ FPS_120, &disp_info->fps.fps_to_120_cmds },
		{ FPS_144, &disp_info->fps.fps_to_144_cmds },
	};

	LCD_KIT_INFO("fps = %u\n", fps);
	table_num = sizeof(fps_lock_cmd) / sizeof(fps_lock_cmd[0]);
	for (i = 0; i < table_num; i++) {
		if (fps == fps_lock_cmd[i].val &&
			(fps_lock_cmd[i].cmds != NULL && fps_lock_cmd[i].cmds->cmds != NULL))
			ret = fps_cmd_send(panel_id, fps_lock_cmd[i].cmds);
	}
	LCD_KIT_INFO("ret = %d\n", ret);
	if (ret == LCD_KIT_OK) {
		disp_info->fps.current_fps = fps;
		common_info->fps_on.current_fps = disp_info->fps.current_fps;
	}
	return ret;
}

static int te_config_cmd_send(int panel_id, struct lcd_kit_dsi_panel_cmds *te_cmds)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}

	if (adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("mipi_tx_no_lock is NULL\n");
		return LCD_KIT_FAIL;
	}

	return adapt_ops->mipi_tx(panel_id, NULL, te_cmds);
}

static int lcd_kit_updt_fps_te_mode(int panel_id, unsigned int fps, bool is_need_delay)
{
	int ret = LCD_KIT_OK;
	int table_num;
	int i;
	struct ltps_fps_cmd_table te_config_cmd[] = {
		{ FPS_60, &disp_info->updt_fps_te.fps_60_cmds },
		{ FPS_120, &disp_info->updt_fps_te.fps_120_cmds },
		{ FPS_144, &disp_info->updt_fps_te.fps_144_cmds },
	};

	if (!disp_info->updt_fps_te.support) {
		LCD_KIT_INFO("not support updt_fps_te\n");
		return ret;
	}

	table_num = ARRAY_SIZE(te_config_cmd);
	for (i = 0; i < table_num; i++) {
		if (fps == te_config_cmd[i].val)
			ret = te_config_cmd_send(panel_id, te_config_cmd[i].cmds);
	}

	if (is_need_delay && disp_info->updt_fps_te.cmds_delay)
		lcd_kit_delay(disp_info->updt_fps_te.cmds_delay, LCD_KIT_WAIT_MS, true);

	return ret;
}

static int lcd_kit_updt_fps_pfm_mode(int panel_id, unsigned int current_fps)
{
	if (disp_info->panel_pfm.support) {
		LCD_KIT_INFO("current_fps:%d", current_fps);
		lcd_kit_fps_pfm_ctrl(panel_id, current_fps);
	}
	return LCD_KIT_OK;
}

static int lcd_kit_set_ppc_config_id(int panel_id, unsigned int ppc_config_id)
{
	if (!common_info->ppc_para.panel_partial_ctrl_support) {
		LCD_KIT_INFO("panel partial ctrl not support\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_INFO("panel partial ctrl id:%u\n", ppc_config_id);
	common_info->ppc_para.ppc_config_id = ppc_config_id;
	return LCD_KIT_OK;
}

static int lcd_kit_primary_set_fastboot(void)
{
	return lcd_kit_set_fastboot(PRIMARY_PANEL);
}

static int lcd_kit_primary_on(void)
{
	return lcd_kit_on(PRIMARY_PANEL);
}

static int lcd_kit_primary_off(void)
{
	return lcd_kit_off(PRIMARY_PANEL);
}

static int lcd_kit_primary_set_backlight(unsigned int bl_level)
{
	return lcd_kit_set_backlight(PRIMARY_PANEL, bl_level);
}

static int lcd_kit_primary_vsync_trigger(void)
{
	return lcd_kit_vsync_trigger(PRIMARY_PANEL);
}

struct dpu_panel_info *lcd_kit_get_primary_dpu_pinfo(void)
{
	return lcd_kit_get_dpu_pinfo(PRIMARY_PANEL);
}

static int lcd_kit_primary_post_process(struct device *dev)
{
	return lcd_kit_post_process(dev);
}

static int lcd_kit_primary_dump_info(void)
{
	check_panel_on_state();
	return LCD_KIT_OK;
}

static int lcd_kit_set_primary_fps_scence(unsigned int current_fps)
{
	return lcd_kit_set_fps_scence(PRIMARY_PANEL, current_fps);
}

static int lcd_kit_primary_get_ddic_cmds(const struct disp_effect_params* effect_params,
	struct dsi_cmds *dsi_cmds)
{
	return lcd_kit_get_ddic_cmds(PRIMARY_PANEL, effect_params, dsi_cmds);
}

static int lcd_kit_primary_trace_screen_bl(unsigned int bl_level)
{
	return lcd_kit_trace_screen_bl(PRIMARY_PANEL, bl_level);
}

static int lcd_kit_primary_updt_fps(unsigned int fps)
{
	return lcd_kit_updt_fps(PRIMARY_PANEL, fps);
}

static int lcd_kit_primary_esd_check(void)
{
	return lcd_kit_esd_check(PRIMARY_PANEL);
}

static int lcd_kit_primary_set_display_region(struct dkmd_rect *dirty)
{
	return lcd_kit_set_display_region(PRIMARY_PANEL, dirty);
}

static int lcd_kit_primary_fake_power_off(void)
{
	return lcd_kit_fake_power_off(PRIMARY_PANEL);
}

static int lcd_kit_primary_updt_fps_te_mode(unsigned int fps)
{
	return lcd_kit_updt_fps_te_mode(PRIMARY_PANEL, fps, false);
}

static int lcd_kit_primary_updt_fps_pfm_mode(unsigned int fps)
{
	return lcd_kit_updt_fps_pfm_mode(PRIMARY_PANEL, fps);
}

static int lcd_kit_primary_set_ppc_config_id(unsigned int ppc_config_id)
{
	return lcd_kit_set_ppc_config_id(PRIMARY_PANEL, ppc_config_id);
}

static int lcd_kit_primary_set_safe_frm_rate(const struct panel_update_safe_frm_rate_info *safe_frm_info)
{
	return lcd_kit_safe_frm_rate_ctrl(PRIMARY_PANEL, safe_frm_info);
}

static struct dpu_panel_ops primary_dpu_panel_data = {
	.set_fastboot = lcd_kit_primary_set_fastboot,
	.on = lcd_kit_primary_on,
	.off = lcd_kit_primary_off,
	.esd_handle = lcd_kit_primary_esd_check,
	.set_backlight = lcd_kit_primary_set_backlight,
	.send_cmds_at_vsync = lcd_kit_primary_vsync_trigger,
	.get_panel_info = lcd_kit_get_primary_dpu_pinfo,
	.disp_postprocess = lcd_kit_primary_post_process,
	.dump_exception_info = lcd_kit_primary_dump_info,
	.set_current_fps = lcd_kit_set_primary_fps_scence,
	.get_ddic_cmds = lcd_kit_primary_get_ddic_cmds,
	.trace_screen_bl = lcd_kit_primary_trace_screen_bl,
	.update_lcd_fps = lcd_kit_primary_updt_fps,
	.set_display_region = lcd_kit_primary_set_display_region,
	.fake_power_off = lcd_kit_primary_fake_power_off,
	.update_fps_te_mode = lcd_kit_primary_updt_fps_te_mode,
	.update_fps_pfm_mode = lcd_kit_primary_updt_fps_pfm_mode,
	.set_ppc_config_id = lcd_kit_primary_set_ppc_config_id,
	.set_safe_frm_rate = lcd_kit_primary_set_safe_frm_rate,
};

static int lcd_kit_io_crtl(uint32_t cmd, uint64_t arg)
{
	int ret = 0;
	void __user *argp = (void __user *)(uintptr_t)arg;
	if (unlikely(!argp))
		return -1;
	switch (cmd) {
	case LCDKIT_IOCTL_DISPLAY_ENGINE_PARAM_GET:
		ret = display_engine_get_param(argp);
		break;
	case LCDKIT_IOCTL_DISPLAY_ENGINE_PARAM_SET:
		ret = display_engine_set_param(argp);
		break;
	default:
		LCD_KIT_ERR("unknow cmd %u\n", cmd);
		return -1;
	}
	return ret;
}

static struct product_display_ops product_io_ops = {
	.fb_ioctl = lcd_kit_io_crtl,
};

#ifdef CONFIG_HUAWEI_DSM
/* uefi dmd report */
static void lcd_kit_primary_probe_dmd_report(void)
{
	if (!lcd_dclient)
		return;

	if (PUBLIC_INFO->dvdd_avs_dmd && strlen(PUBLIC_INFO->dvdd_avs_dmd) > 0) {
		if (dsm_client_ocuppy(lcd_dclient))
			return;
		/* lcdHpmDmd */
		LCD_KIT_ERR("report dvdd_avs_dmd\n");
		dsm_client_record(lcd_dclient,
			"lcd dvdd_avs_dmd hpm invalid:%s\n",
			PUBLIC_INFO->dvdd_avs_dmd);
		dsm_client_notify(lcd_dclient, DSM_LCD_POWER_STATUS_ERROR_NO);
	}
}
#endif

static void lcd_kit_completion_init(int panel_id)
{
	init_completion(&disp_info->lcd_completion.tx_no_lock_done);
	disp_info->lcd_completion.is_in_tx_no_lock = false;
}

static void lcd_kit_execute_lcd_ops(int panel_id)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd ops is NULL\n");
		return;
	}
	if (lcd_ops->get_sn_code) {
		ret = lcd_ops->get_sn_code(PRIMARY_PANEL);
		if (ret < 0)
			LCD_KIT_ERR("get sn code failed!\n");
	}
	if (lcd_ops->get_el_ctrl_info) {
		ret = lcd_ops->get_el_ctrl_info(PRIMARY_PANEL);
		if (ret < 0)
			LCD_KIT_ERR("get el ctrl failed!\n");
	}
}

static int lcd_kit_primary_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	int ret;

	np = pdev->dev.of_node;
	if (!np) {
		LCD_KIT_ERR("not find device node\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("primary panel porbe+\n");
	lcd_kit_adapt_init();
	lcd_kit_completion_init(PRIMARY_PANEL);

	ret = lcd_kit_common_probe(PRIMARY_PANEL, np);
	if (ret) {
		LCD_KIT_ERR("primary panel common probe fail\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_power_init(PRIMARY_PANEL, pdev);
	/* register to dkmd */
	ret = register_panel(&primary_dpu_panel_data, PRIMARY_PANEL);
	if (ret) {
		LCD_KIT_ERR("dpu_add_driver failed!\n");
		goto err_device_put;
	}

	/* register product_io_ops to dkmd */
	ret = regitster_product_ops(&product_io_ops);
	if (ret) {
		LCD_KIT_ERR("regitster_product_ops failed!\n");
		goto err_device_put;
	}

#ifdef CONFIG_HUAWEI_DSM
	lcd_dclient = dkmd_get_dmd_client();
	lcd_kit_primary_probe_dmd_report();
#endif

	// lcd color calib check
	lcd_kit_color_calib_check(PRIMARY_PANEL);
	/* execute lcd ops */
	lcd_kit_execute_lcd_ops(PRIMARY_PANEL);

	lcd_kit_panel_init(PRIMARY_PANEL, pdev);
	dpu_elvss_info_init(PRIMARY_PANEL);
	clear_el_ctrl_input_info(PRIMARY_PANEL);
	LCD_KIT_INFO("primary panel porbe-\n");
	return LCD_KIT_OK;
err_device_put:
	return -EPROBE_DEFER;
}

static int lcd_kit_secondary_set_fastboot(void)
{
	return lcd_kit_set_fastboot(SECONDARY_PANEL);
}

static int lcd_kit_secondary_on(void)
{
	return lcd_kit_on(SECONDARY_PANEL);
}

static int lcd_kit_secondary_off(void)
{
	return lcd_kit_off(SECONDARY_PANEL);
}

static int lcd_kit_secondary_set_backlight(unsigned int bl_level)
{
	return lcd_kit_set_backlight(SECONDARY_PANEL, bl_level);
}

struct dpu_panel_info *lcd_kit_get_secondary_dpu_pinfo(void)
{
	return lcd_kit_get_dpu_pinfo(SECONDARY_PANEL);
}

static int lcd_kit_secondary_get_ddic_cmds(const struct disp_effect_params* effect_params,
	struct dsi_cmds *dsi_cmds)
{
	return lcd_kit_get_ddic_cmds(SECONDARY_PANEL, effect_params, dsi_cmds);
}

static int lcd_kit_secondary_dump_info(void)
{
	check_panel_on_state();
	return LCD_KIT_OK;
}

static int lcd_kit_set_secondary_fps_scence(unsigned int current_fps)
{
	return lcd_kit_set_fps_scence(SECONDARY_PANEL, current_fps);
}

static int lcd_kit_secondary_trace_screen_bl(unsigned int bl_level)
{
	return lcd_kit_trace_screen_bl(SECONDARY_PANEL, bl_level);
}

static int lcd_kit_second_updt_fps(unsigned int fps)
{
	return lcd_kit_updt_fps(SECONDARY_PANEL, fps);
}

static int lcd_kit_secondary_esd_check(void)
{
	return lcd_kit_esd_check(SECONDARY_PANEL);
}

static int lcd_kit_secondary_set_display_region(struct dkmd_rect *dirty)
{
	return lcd_kit_set_display_region(SECONDARY_PANEL, dirty);
}

static int lcd_kit_secondary_fake_power_off(void)
{
	return lcd_kit_fake_power_off(SECONDARY_PANEL);
}

static int lcd_kit_secondary_set_ppc_config_id(unsigned int ppc_config_id)
{
	return lcd_kit_set_ppc_config_id(SECONDARY_PANEL, ppc_config_id);
}

static int lcd_kit_secondary_set_safe_frm_rate(const struct panel_update_safe_frm_rate_info *safe_frm_info)
{
	return lcd_kit_safe_frm_rate_ctrl(SECONDARY_PANEL, safe_frm_info);
}

static struct dpu_panel_ops secondary_dpu_panel_data = {
	.set_fastboot = lcd_kit_secondary_set_fastboot,
	.on = lcd_kit_secondary_on,
	.off = lcd_kit_secondary_off,
	.esd_handle = lcd_kit_secondary_esd_check,
	.set_backlight = lcd_kit_secondary_set_backlight,
	.get_panel_info = lcd_kit_get_secondary_dpu_pinfo,
	.dump_exception_info = lcd_kit_secondary_dump_info,
	.set_current_fps = lcd_kit_set_secondary_fps_scence,
	.get_ddic_cmds = lcd_kit_secondary_get_ddic_cmds,
	.trace_screen_bl = lcd_kit_secondary_trace_screen_bl,
	.update_lcd_fps = lcd_kit_second_updt_fps,
	.set_display_region = lcd_kit_secondary_set_display_region,
	.fake_power_off = lcd_kit_secondary_fake_power_off,
	.set_ppc_config_id = lcd_kit_secondary_set_ppc_config_id,
	.set_safe_frm_rate = lcd_kit_secondary_set_safe_frm_rate,
};

static int lcd_kit_secondary_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	int ret;

	LCD_KIT_INFO("secondary panel porbe+");
	np = pdev->dev.of_node;
	if (!np) {
		LCD_KIT_ERR("not find device node\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_common_probe(SECONDARY_PANEL, np);
	if (ret) {
		LCD_KIT_ERR("secondary panel common probe fail\n");
		return LCD_KIT_FAIL;
	}

	/* register to dkmd */
	ret = register_panel(&secondary_dpu_panel_data, SECONDARY_PANEL);
	if (ret) {
		LCD_KIT_ERR("dpu_add_driver failed!\n");
		goto err_device_put;
	}

	lcd_kit_register_power_off_optimize();
	lcd_kit_completion_init(SECONDARY_PANEL);
	// lcd panel version
	lcd_kit_panel_version_init(SECONDARY_PANEL);
	LCD_KIT_INFO("secondary panel porbe-");
	return LCD_KIT_OK;
err_device_put:
	return -EPROBE_DEFER;
}

// primary panel probe match table
static struct of_device_id lcd_kit_primary_match_table[] = {
	{
		.compatible = "auo_otm1901a_5p2_1080p_video",
		.data = NULL,
	},
	{},
};

// secondary panel probe match table
static struct of_device_id lcd_kit_secondary_match_table[] = {
	{
		.compatible = "lcd_ext_panel_default",
		.data = NULL,
	},
	{},
};

// panel platform driver
static struct platform_driver lcd_kit_panel_driver[MAX_PANEL] = {
	{
		.probe = lcd_kit_primary_probe,
		.remove = NULL,
		.suspend = NULL,
		.resume = NULL,
		.shutdown = NULL,
		.driver = {
			.name = "lcd_kit_mipi_panel",
			.of_match_table = lcd_kit_primary_match_table,
		},
	},
	{
		.probe = lcd_kit_secondary_probe,
		.remove = NULL,
		.suspend = NULL,
		.resume = NULL,
		.shutdown = NULL,
		.driver = {
			.name = "lcd_kit_sec_mipi_panel",
			.of_match_table = lcd_kit_secondary_match_table,
		},
	},
};

static int lcd_kit_get_comp(int panel_id, struct device_node *np, int index)
{
	unsigned int pinid = 0;
	int i;
	unsigned int gpio_value;
	int panel_length = sizeof(g_lcd_kit_panels) / sizeof(g_lcd_kit_panels[0]);
	struct lcd_kit_array_data lcd_ids;
	unsigned int poweric_det_support = 0;
	unsigned int poweric_gpio = POWERIC_DEFAULT_GPIO;

	if (index == 0)
		lcd_kit_parse_array_data(np, "gpio_id", &lcd_ids);
	else
		lcd_kit_parse_array_data(np, "sec_gpio_id", &lcd_ids);

	for (i = 0; i < lcd_ids.cnt; i++) {
		gpio_value = lcd_kit_get_pinid(lcd_ids.buf[i]);
		pinid |= (gpio_value << (2 * i));
	}

	if (index == 0) {
		/* ext idpin config */
		lcd_kit_parse_u32(np, "poweric_det_support", &poweric_det_support, 0);
		lcd_kit_parse_u32(np, "poweric_gpio_id", &poweric_gpio, 0);
	} else {
		/* ext idpin config */
		lcd_kit_parse_u32(np, "sec_poweric_det_support", &poweric_det_support, 0);
		lcd_kit_parse_u32(np, "sec_poweric_gpio_id", &poweric_gpio, 0);
	}
	pinid |= lcd_kit_get_ext_pinid(np, poweric_det_support, poweric_gpio);

	/* sec panel panid */
	pinid |= (0x40 * index);

	LCD_KIT_INFO("pinid = %u\n", pinid);
	for (i = 0; i < panel_length; i++) {
		if (g_lcd_kit_panels[i].product_id == disp_info->product_id &&
			g_lcd_kit_panels[i].pin_id == pinid) {
			LCD_KIT_INFO("lcd has matched panel table, panel_name is %s\n",
				g_lcd_kit_panels[i].panel_name);
			disp_info->compatible = (char *)g_lcd_kit_panels[i].panel_name;
			return LCD_KIT_OK;
		}
	}

	return LCD_KIT_FAIL;
}

static int lcd_kit_parse_comp(int panel_id, struct device_node *np, int index)
{
	unsigned int uefi_bypass;
	int ret;

	lcd_kit_parse_u32(np, "uefi_bypass", &uefi_bypass, 0);
	if (uefi_bypass) {
		ret = lcd_kit_get_comp(panel_id, np, index);
		if (ret) {
			LCD_KIT_ERR("get lcd compatible error\n");
			return ret;
		}
	} else {
		disp_info->compatible = (char *)of_get_property(np,
			g_disp_compatibles[index], NULL);
		if (!disp_info->compatible) {
			LCD_KIT_ERR("can not get lcd kit compatible\n");
			return LCD_KIT_FAIL;
		}
	}

	return LCD_KIT_OK;
}

static void lcd_kit_parse_panel_software_id_number(struct device_node *np)
{
	int panel_id = PRIMARY_PANEL;
	lcd_kit_parse_u32(np, "panel_software_id_number", &common_info->panel_software_id.number, 0);
	LCD_KIT_INFO("panel_software_id_number = %u", common_info->panel_software_id.number);
}

static void lcd_kit_parse_ppc_software_id_number(struct device_node *np)
{
	int panel_id = PRIMARY_PANEL;
	lcd_kit_parse_u32(np, "ppc_software_id_number", &common_info->ppc_para.soft_id_number, 0);
	LCD_KIT_INFO("ppc_software_id_number = %d\n", common_info->ppc_para.soft_id_number);
}

static void lcd_kit_parse_public_info(struct device_node *np)
{
	/* parse product_type */
	lcd_kit_parse_u32(np, "product_type", &PUBLIC_INFO->product_type, 0);
	LCD_KIT_INFO("product_type = %d", PUBLIC_INFO->product_type);
	lcd_kit_parse_u32(np, "dvdd_avs_volt", &PUBLIC_INFO->dvdd_avs_volt, 0);
	LCD_KIT_INFO("dvdd_avs_volt = %d", PUBLIC_INFO->dvdd_avs_volt);
	lcd_kit_parse_u32(np, "dvdd_avs_vmin", &PUBLIC_INFO->dvdd_avs_vmin, 0);
	LCD_KIT_INFO("dvdd_avs_vmin = %d", PUBLIC_INFO->dvdd_avs_vmin);
	lcd_kit_parse_string(np, "dvdd_avs_dmd", (const char **)&PUBLIC_INFO->dvdd_avs_dmd);
	if (PUBLIC_INFO->dvdd_avs_dmd)
		LCD_KIT_INFO("dvdd_avs_dmd = %s", PUBLIC_INFO->dvdd_avs_dmd);
	lcd_kit_parse_u32(np, "need_color_calib", &PUBLIC_INFO->need_color_calib, 0);
	LCD_KIT_INFO("need_color_calib = %u", PUBLIC_INFO->need_color_calib);
	/* parse panel version */
	lcd_kit_parse_u32(np, "primary_panel_version",
		&PUBLIC_INFO->primary_panel_version, 0);
	lcd_kit_parse_u32(np, "secondary_panel_version",
		&PUBLIC_INFO->secondary_panel_version, 0);
	/* parse panel lcd_power_seq */
	lcd_kit_parse_u32(np, "lcd_power_seq",
		&PUBLIC_INFO->lcd_power_seq, LCD_KIT_POWER_OFF);
	LCD_KIT_INFO("lcd_power_seq = %d", PUBLIC_INFO->lcd_power_seq);
	lcd_kit_parse_u32(np, "sec_lcd_power_seq",
		&PUBLIC_INFO->sec_lcd_power_seq, LCD_KIT_POWER_OFF);
	LCD_KIT_INFO("sec_lcd_power_seq = %d", PUBLIC_INFO->sec_lcd_power_seq);
	lcd_kit_parse_u32(np, "el_ctrl_level",
		&PUBLIC_INFO->calibrate_level, 256);
	LCD_KIT_INFO("el_ctrl_level = %d", PUBLIC_INFO->calibrate_level);
	lcd_kit_parse_u32(np, "fake_compatible_panel",
		&PUBLIC_INFO->fake_compatible_panel, 1);
	LCD_KIT_INFO("fake_compatible_panel = %d", PUBLIC_INFO->fake_compatible_panel);
	/* panel software id */
	lcd_kit_parse_panel_software_id_number(np);
	/* ppc software id */
	lcd_kit_parse_ppc_software_id_number(np);
}

static int __init lcd_kit_init(void)
{
	int ret = LCD_KIT_OK;
	int init_length, len, i;
	int panel_id;
	struct device_node *np = NULL;

	if (!lcd_kit_support()) {
		LCD_KIT_INFO("not lcd_kit driver and return\n");
		return ret;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", DTS_COMP_LCD_KIT_PANEL_TYPE);
		return LCD_KIT_FAIL;
	}

	lcd_kit_parse_public_info(np);
	init_length = lcd_kit_get_init_display_length();

	for (i = 0; i < init_length; i++) {
		panel_id = g_init_order[i];

		lcd_kit_parse_u32(np, "board_version", &disp_info->board_version, 0);
		LCD_KIT_INFO("board_version = %u", disp_info->board_version);
		lcd_kit_parse_u32(np, "product_id", &disp_info->product_id, 0);
		LCD_KIT_INFO("product_id = %u", disp_info->product_id);

		ret = lcd_kit_parse_comp(panel_id, np, i);
		if (ret) {
			LCD_KIT_ERR("get compatible is null and return\n");
			return ret;
		}

		len = strlen(disp_info->compatible);

		memset((char *)lcd_kit_panel_driver[i].driver.of_match_table->compatible, 0, LCD_KIT_PANEL_COMP_LENGTH);

		strncpy((char *)lcd_kit_panel_driver[i].driver.of_match_table->compatible,
				disp_info->compatible, len > (LCD_KIT_PANEL_COMP_LENGTH - 1) ?
				(LCD_KIT_PANEL_COMP_LENGTH - 1) : len);

		/* register driver */
		ret = platform_driver_register(&lcd_kit_panel_driver[i]);
		if (ret) {
			LCD_KIT_ERR("driver register failed, error = %d!\n", ret);
			return ret;
		}

	}

	return LCD_KIT_OK;
}

module_init(lcd_kit_init);
