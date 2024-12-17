/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <securec.h>
#include "dpu_comp_bl.h"
#include "dpu_comp_mgr.h"
#include "dkmd_log.h"
#include "dkmd_peri.h"
#include "present/dpu_comp_present.h"

#define MSEC_PER_FRAME 17

enum dpu_bl_setting_mode {
	/* panel with fixed frame rate, or other bl setting type, blpmw etc.. */
	BL_SETTING_MODE_DIRECT,

	/* ltpo panel with dynamic frame rate need use this mode,
	because bl setting will lead to refresh, we need update
	it to dacc, and send bl ddic cmd at riscv-capture time */
	BL_SETTING_MODE_BY_RISCV,

	BL_SETTING_MODE_INVALID
};

static int dpu_bl_select_setting_mode(struct dkmd_connector_info *pinfo)
{
	struct dfr_info *dfr_info = dkmd_get_dfr_info(pinfo);
	if (!dfr_info)
		return BL_SETTING_MODE_DIRECT;
	if (dfr_info->oled_info.oled_type == PANEL_OLED_LTPO &&
		(dfr_info->dfr_mode == DFR_MODE_TE_SKIP_BY_MCU || dfr_info->dfr_mode == DFR_MODE_LONGH_TE_SKIP_BY_MCU) &&
		pinfo->bl_info.bl_type == BL_SET_BY_MIPI)
		return BL_SETTING_MODE_BY_RISCV;

	return BL_SETTING_MODE_DIRECT;
}

static int32_t dpu_backlight_adjust_bl_value(struct bl_info *bl_info, uint32_t bl_value)
{
	uint32_t bl_value_adjust = bl_value;
	if ((bl_value < bl_info->bl_min) && (bl_value != 0))
		bl_value_adjust = bl_info->bl_min;

	if (bl_value > bl_info->bl_max)
		bl_value_adjust = bl_info->bl_max;
	return (int32_t)bl_value_adjust;
}

void dpu_backlight_debug_level(struct disp_effect_params *effect_params)
{
	dpu_pr_info("backlight debug level\n");
	effect_params->effect_num = 1;
	effect_params->delay = 0;
	effect_params->params[0].effect_len = 1;
	effect_params->params[0].effect_type = DISP_EFFECT_WITH_BL;
	effect_params->params[0].effect_values[0] = (int32_t)g_debug_dpu_send_dcs_cmds;
}

void dpu_backlight_update_level(struct dpu_bl_ctrl *bl_ctrl, struct disp_effect_params *effect_params)
{
	struct bl_info *bl_info = &bl_ctrl->conn_info->bl_info;
	uint32_t bl_value;
	uint32_t i;
	if (!bl_info) {
		dpu_pr_err("bl_info is NULL\n");
		return;
	}
	for (i = 0; i < effect_params->effect_num; i++) {
		if (effect_params->params[i].effect_type != DISP_EFFECT_WITH_BL)
			continue;
		down(&bl_ctrl->bl_sem);
		bl_value = (uint32_t)effect_params->params[i].effect_values[0];
		bl_ctrl->bl_level = (uint32_t)dpu_backlight_adjust_bl_value(bl_info, bl_value);
		effect_params->params[i].effect_values[0] = (int32_t)bl_ctrl->bl_level;
		bl_ctrl->bl_level_old = bl_ctrl->bl_level;
		up(&bl_ctrl->bl_sem);
		break;
	}
	return;
}

static int dpu_bl_set_backlight(struct dpu_bl_ctrl *bl_ctrl, uint32_t bl_lvl, bool enforce)
{
	enum dpu_bl_setting_mode setting_mode;
	int32_t bl_ret;
	if (bl_ctrl == NULL || bl_ctrl->conn_info == NULL) {
		dpu_pr_err("bl_ctrl is NULL\n");
		return -EINVAL;
	}

	/* before frame display only update user's value,  bl_level_old is the value before */
	if ((bl_ctrl->bl_updated == 0) && (!enforce)) {
		bl_ctrl->bl_level = bl_lvl;
		dpu_pr_debug("[backlight] return: only update bl_level %d!", bl_lvl);
		return 0;
	}

	if (bl_ctrl->bl_level_old == bl_lvl && (!enforce)) {
		dpu_pr_debug("[backlight] return: bl_level_old %d, bl_level = %d is same to the last, enforce = %d!",
					bl_ctrl->bl_level_old, bl_lvl, enforce);
		return 0;
	}

	dpu_pr_debug("[backlight] last_bl_level %d, new backlight level = %d\n", bl_ctrl->bl_level_old, bl_lvl);

	bl_ctrl->bl_level = bl_lvl;
	bl_ctrl->bl_level_old = bl_lvl;
	bl_ctrl->bl_timestamp = ktime_get();
	dpu_comp_active_vsync(bl_ctrl->parent_composer);

	setting_mode = dpu_bl_select_setting_mode(bl_ctrl->conn_info);
	if (setting_mode == BL_SETTING_MODE_DIRECT) {
		pipeline_next_ops_handle(bl_ctrl->conn_info->conn_device, bl_ctrl->conn_info, SET_BACKLIGHT, &bl_ctrl->bl_level);
	} else if (setting_mode == BL_SETTING_MODE_BY_RISCV) {
		bl_ret = composer_present_dfr_send_dcs_by_riscv(bl_ctrl->parent_composer, bl_lvl);
		if (bl_ret == -1)
			dpu_pr_err("set backlight error\n");
	} else {
		dpu_comp_deactive_vsync(bl_ctrl->parent_composer);
		return -1;
	}
	dpu_comp_deactive_vsync(bl_ctrl->parent_composer);

	return 0;
}

void dpu_backlight_update(struct dpu_bl_ctrl *bl_ctrl, bool enforce)
{
	unsigned long backlight_duration = 1 * MSEC_PER_FRAME;
	struct bl_info *bl_info = NULL;

	if (bl_ctrl == NULL || bl_ctrl->conn_info == NULL) {
		dpu_pr_err( "[backlight] bl_ctrl or conn_info is NULL\n");
		return;
	}

	if (enforce) {
		down(&bl_ctrl->bl_sem);
		dpu_bl_set_backlight(bl_ctrl, bl_ctrl->bl_level, true);
		up(&bl_ctrl->bl_sem);
		return;
	}

	dpu_pr_debug("[backlight] bkl_lvl %d, bl_updated %d enforce = %d",
				bl_ctrl->bl_level, bl_ctrl->bl_updated, enforce);

	/* make sure only after the first frame refresh, backlight will be set */
	if (bl_ctrl->bl_updated == 0) {
		bl_info = &bl_ctrl->conn_info->bl_info;
		if (bl_info->delay_set_bl_support)
			backlight_duration = bl_info->delay_set_bl_thr * MSEC_PER_FRAME;

		schedule_delayed_work(&bl_ctrl->bl_worker, msecs_to_jiffies((uint32_t)backlight_duration));
	} else {
		down(&bl_ctrl->bl_sem);
		bl_ctrl->bl_updated = 1;
		dpu_bl_set_backlight(bl_ctrl, bl_ctrl->bl_level, false);
		up(&bl_ctrl->bl_sem);
	}
}

void dpu_backlight_cancel(struct dpu_bl_ctrl *bl_ctrl)
{
	uint32_t bl_level;

	if (bl_ctrl == NULL || bl_ctrl->conn_info == NULL) {
		dpu_pr_debug("[backlight] bl_ctrl is NULL\n");
		return;
	}
	dpu_pr_debug("[backlight] bl_updated 0 and level 0\n");

	cancel_delayed_work(&bl_ctrl->bl_worker);

	down(&bl_ctrl->bl_sem);
	bl_ctrl->bl_updated = 0;

	bl_level = bl_ctrl->bl_level;
	dpu_bl_set_backlight(bl_ctrl, 0, true);
	bl_ctrl->bl_level = bl_level;
	up(&bl_ctrl->bl_sem);
}

static ssize_t dpu_bl_brightness_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct dpu_bl_ctrl *bl_ctrl = get_bl_ctrl_from_device(dev);

	if (bl_ctrl == NULL) {
		dpu_pr_err("bl_ctrl is null");
		return -1;
	}

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u", bl_ctrl->bl_level_old);
	if (ret < 0)
		dpu_pr_err("snprintf_s failed, ret value is %d\n", ret);

	buf[strlen(buf) + 1] = '\0';
	return ret;
}

static ssize_t dpu_bl_brightness_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	uint32_t bl_value;
	struct dpu_bl_ctrl *bl_ctrl = get_bl_ctrl_from_device(dev);
	struct bl_info *bl_info = NULL;

	if (bl_ctrl == NULL) {
		dpu_pr_err("bl_ctrl is null");
		return -1;
	}

	if (bl_ctrl->conn_info == NULL) {
		dpu_pr_err("conn_info is null");
		return -1;
	}

	ret = sscanf_s(buf, "%u", &bl_value);
	if (ret == 0) {
		dpu_pr_err("get bl_value err");
		return -1;
	}

	bl_info = &bl_ctrl->conn_info->bl_info;
	if ((bl_value < bl_info->bl_min) && (bl_value != 0))
		bl_value = bl_info->bl_min;

	if (bl_value > bl_info->bl_max)
		bl_value = bl_info->bl_max;

	down(&bl_ctrl->parent_composer->comp_mgr->power_sem);
	if (!composer_check_power_status(bl_ctrl->parent_composer)) {
		dpu_pr_warn("already power off, set backlight:%u failed!", bl_value);
		bl_ctrl->bl_level = bl_value;
		up(&bl_ctrl->parent_composer->comp_mgr->power_sem);
		return (ssize_t)count;
	}

	down(&bl_ctrl->bl_sem);
	dpu_bl_set_backlight(bl_ctrl, bl_value, true);
	up(&bl_ctrl->bl_sem);
	up(&bl_ctrl->parent_composer->comp_mgr->power_sem);

	dpu_pr_debug("set brightness value:%u", bl_value);
	return (ssize_t)count;
}

static ssize_t dpu_bl_max_brightness_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct dpu_bl_ctrl *bl_ctrl = get_bl_ctrl_from_device(dev);

	if (bl_ctrl == NULL) {
		dpu_pr_err("bl_ctrl is null");
		return -1;
	}

	if (bl_ctrl->conn_info == NULL) {
		dpu_pr_err("conn_info is null");
		return -1;
	}

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u", bl_ctrl->conn_info->bl_info.bl_max);
	if (ret < 0)
		dpu_pr_err("snprintf_s failed, ret value is %d\n", ret);

	buf[strlen(buf) + 1] = '\0';
	return ret;
}

static DEVICE_ATTR(brightness, 0640, dpu_bl_brightness_show, dpu_bl_brightness_store);
static DEVICE_ATTR(max_brightness, 0440, dpu_bl_max_brightness_show, NULL);

static void dpu_bl_workqueue_handler(struct work_struct *work)
{
	struct dpu_bl_ctrl *bl_ctrl = NULL;

	bl_ctrl = container_of(to_delayed_work(work), struct dpu_bl_ctrl, bl_worker); //lint !e666
	if (!bl_ctrl) {
		dpu_pr_err("[backlight] pbl_ctrl is NULL\n");
		return;
	}

	if (bl_ctrl->bl_updated)
		return;

	dpu_pr_debug("[backlight] bl_level %d, is_recovery_mode %d\n", bl_ctrl->bl_level, bl_ctrl->is_recovery_mode);

	down(&bl_ctrl->parent_composer->comp_mgr->power_sem);
	if (!composer_check_power_status(bl_ctrl->parent_composer)) {
		dpu_pr_warn("dpu_bl_workqueue_handler, already power off, set backlight failed!");
		up(&bl_ctrl->parent_composer->comp_mgr->power_sem);
		return;
	}

	down(&bl_ctrl->bl_sem);
	bl_ctrl->bl_updated = 1;

	/*
	 * fix the bug:recovery interface app can not set backlight successfully
	 * if backlight not be set at recovery, driver will set default value
	 * otherwise driver not set anymore
	*/
	if (bl_ctrl->is_recovery_mode) {
		if (bl_ctrl->bl_level == 0)
			bl_ctrl->bl_level = bl_ctrl->conn_info->bl_info.bl_default;
	}

	dpu_bl_set_backlight(bl_ctrl, bl_ctrl->bl_level, false);
	up(&bl_ctrl->bl_sem);
	up(&bl_ctrl->parent_composer->comp_mgr->power_sem);
}


#ifdef CONFIG_LEDS_CLASS
void bl_lcd_set_backlight(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	dpu_pr_warn("Old brightness file node detected, plz change it to new node in fb0\n");
	return;
}
#endif

void dpu_backlight_init(struct dpu_bl_ctrl *bl_ctrl, struct dkmd_attr *attrs, struct dpu_composer *dpu_comp)
{
	if ((bl_ctrl == NULL) || (attrs == NULL) || dpu_comp == NULL) {
		dpu_pr_err("bl_ctrl or attrs or conn_info is null");
		return;
	}

	dpu_pr_debug("the bl_type in dpu_backlight_init is: %d", dpu_comp->conn_info->bl_info.bl_type);
	if (dpu_comp->conn_info->bl_info.bl_type == 0)
		return;

	bl_ctrl->bl_updated = 0;
	bl_ctrl->bl_level = dpu_comp->conn_info->bl_info.bl_default;
	bl_ctrl->bl_level_old = 0;
#ifdef CONFIG_CMDLINE_PARSE
	bl_ctrl->is_recovery_mode = get_boot_into_recovery_flag();
#else
	bl_ctrl->is_recovery_mode = 0;
#endif
	bl_ctrl->conn_info = dpu_comp->conn_info;
	bl_ctrl->parent_composer = dpu_comp;
	sema_init(&bl_ctrl->bl_sem, 1);
	INIT_DELAYED_WORK(&bl_ctrl->bl_worker, dpu_bl_workqueue_handler);

	dkmd_sysfs_attrs_append(attrs, &dev_attr_brightness.attr);
	dkmd_sysfs_attrs_append(attrs, &dev_attr_max_brightness.attr);
}
