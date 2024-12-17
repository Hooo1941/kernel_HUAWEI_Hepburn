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
#include <linux/module.h>
#include <linux/of_device.h>

#include "dkmd_log.h"
#include "dkmd_notify.h"
#include "gfxdev_mgr.h"
#include "gfxdev_utils.h"
#include "dpu_fb.h"
#include "dpu_drm.h"
#include "dpu_gfx.h"
#include "dkmd_dpu.h"
#include "dpu_comp_mgr.h"
#include "secure/dpu_comp_tui.h"

static struct composer *g_device_comp[DEVICE_COMP_MAX_COUNT];
static uint32_t g_disp_device_arch = FBDEV_ARCH;
static uint32_t g_fastboot_enable_flag;
static uint32_t g_fake_lcd_flag;

int32_t gfxdev_blank_power_on(struct composer *comp)
{
	int32_t ret = -1;

	if (unlikely(!comp)) {
		dpu_pr_err("comp is null\n");
		return -1;
	}

	dpu_tui_register(to_dpu_composer(comp));

	dpu_pr_info("blank power on current status %d", comp->power_on);

	if (comp->wait_for_blank) {
		ret = comp->wait_for_blank(comp);
		if (ret != 0)
			return 0;
	}

	/* TODO: other modules listen to the blank event, FB_EARLY_EVENT_BLANK or FB_EVENT_BLANK
	 * such as, AOD. those event had been sended at fbmem.c
	 */

	ret = comp->on(comp);
	if (ret) {
		dpu_pr_err("next composer power on fail");
		return ret;
	}

	/* power on timestamp for fps count */
	comp->power_on_timestamp =  ktime_get();

	return ret;
}

int32_t gfxdev_blank_power_off(struct composer *comp, int32_t off_mode)
{
	int32_t ret = -1;

	if (unlikely(!comp)) {
		dpu_pr_err("comp is null\n");
		return -1;
	}

	dpu_pr_info("blank power off current status %d", comp->power_on);

	if (comp->restore_fast_unblank_status)
		comp->restore_fast_unblank_status(comp);

	ret = comp->off(comp, off_mode);
	if (ret) {
		dpu_pr_warn("next composer power off fail");
		return ret;
	}

	dpu_tui_unregister(to_dpu_composer(comp));

	/* power off timestamp for fps count */
	comp->power_off_timestamp =  ktime_get();

	return ret;
}

int32_t gfxdev_blank_fake_off(struct composer *comp)
{
	int32_t ret = -1;

	if (unlikely(!comp)) {
		dpu_pr_err("comp is null\n");
		return -1;
	}

	dpu_pr_info("blank power off current status %d", comp->power_on);

	if (comp->restore_fast_unblank_status)
		comp->restore_fast_unblank_status(comp);

	down(&comp->blank_sem);
	comp->is_fake_power_off = true;
	up(&comp->blank_sem);

	ret = comp->off(comp, COMPOSER_OFF_MODE_BLANK);
	if (ret)
		dpu_pr_warn("next composer power off fail");

	dpu_tui_unregister(to_dpu_composer(comp));

	return ret;
}

void device_mgr_primary_frame_refresh(struct composer *comp, char *trigger)
{
	char *envp[2] = { "Refresh=1", NULL };

	if (!comp || !trigger) {
		dpu_pr_err("comp or trigger is NULL Pointer\n");
		return;
	}

	kobject_uevent_env(&(comp->base.peri_device->dev.kobj), KOBJ_CHANGE, envp);
	dpu_pr_info("%s need frame refresh!", trigger);
}

struct composer *get_comp_from_device(struct device *dev)
{
	if (!dev) {
		dpu_pr_err("input dev is null!");
		return NULL;
	}

	if (strncmp(dev->kobj.name, DEV_NAME_FB0, 2) == 0)
		return get_comp_from_fb_device(dev);

	return get_comp_from_gfx_device(dev);
}

/* dev_name: device name, such as fb0, gfx_dp, gfx_offline, gfx_hdmi */
struct composer *get_comp_from_device_name(uint32_t index)
{
	if (index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("input index is invalid!");
		return NULL;
	}

	return g_device_comp[index];
}

int32_t device_mgr_create_gfxdev(struct composer *comp)
{
	if (!comp) {
		dpu_pr_err("input comp is null!");
		return -EINVAL;
	}

	dpu_pr_info("%s comp index %d", comp->base.name, comp->index);

	if (comp->index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("invalid comp index=%d!", comp->index);
		return -EINVAL;
	}

	if (comp != g_device_comp[comp->index]) {
		dpu_pr_err("%s input comp have not registed match for comp!",
			comp->base.name);
		return -EINVAL;
	}

	comp->fastboot_display_enabled = g_fastboot_enable_flag;
	comp->power_on = false;
	comp->is_fake_power_off = false;

	if (is_offline_panel(&comp->base) || is_dp_panel(&comp->base) ||
		is_hdmi_panel(&comp->base) || is_builtin_panel(&comp->base))
		return gfx_device_register(comp);

	if (g_disp_device_arch == FBDEV_ARCH)
		return fb_device_register(comp);
	else
		return drm_device_register(comp);
}

void device_mgr_destroy_gfxdev(struct composer *comp)
{
	if (!comp) {
		dpu_pr_err("input comp is null!");
		return;
	}

	if (comp->index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("invalid comp index=%u!", comp->index);
		return;
	}

	if (comp != g_device_comp[comp->index]) {
		dpu_pr_err("%s input comp have not registed match for comp!",
			comp->base.name);
		return;
	}
	g_device_comp[comp->index] = NULL;

	if (is_offline_panel(&comp->base) || is_dp_panel(&comp->base) ||
		is_hdmi_panel(&comp->base) || is_builtin_panel(&comp->base)) {
		gfx_device_unregister(comp);
		return;
	}

	if (g_disp_device_arch == FBDEV_ARCH)
		fb_device_unregister(comp);
	else
		drm_device_unregister(comp);
}

void device_mgr_shutdown_gfxdev(struct composer *comp)
{
	if (!comp) {
		dpu_pr_err("input comp is null!");
		return;
	}

	if (comp->index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("invalid comp index=%d!", comp->index);
		return;
	}

	if (comp != g_device_comp[comp->index]) {
		dpu_pr_err("%s input comp have not registed match for comp!",
			comp->base.name);
		return;
	}

	if (is_offline_panel(&comp->base) || is_dp_panel(&comp->base) ||
		is_builtin_panel(&comp->base) || is_hdmi_panel(&comp->base))
		return;

	if (g_disp_device_arch == FBDEV_ARCH)
		fb_device_shutdown(comp);
}

void device_mgr_suspend_gfxdev(struct composer *comp)
{
	struct dkmd_event event = {0};

	if (!comp) {
		dpu_pr_err("input comp is null!");
		return;
	}

	if (comp->index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("invalid comp index=%d!", comp->index);
		return;
	}

	if (comp != g_device_comp[comp->index]) {
		dpu_pr_err("%s input comp have not registed match for comp!",
			comp->base.name);
		return;
	}

	down(&comp->blank_sem);
	if (!comp->power_on) {
		dpu_pr_info("already power off\n");
		up(&comp->blank_sem);
		return;
	}
	up(&comp->blank_sem);

	event.data = NULL;
	event.panel_id = (int32_t)comp->base.id;
	event.value = DISP_BLANK_POWERDOWN;

	dpu_pr_warn("suspend will do abnormal power off\n");
	dkmd_notifier_call_chain(DKMD_EVENT_BLANK, (void *)&event);
	gfxdev_blank_power_off(comp, COMPOSER_OFF_MODE_SUSPEND);
}

void device_mgr_resume_gfxdev(struct composer *comp)
{
	struct dkmd_event event = {0};

	if (!comp) {
		dpu_pr_err("input comp is null!");
		return;
	}

	if (comp->index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("invalid comp index=%d!", comp->index);
		return;
	}

	if (comp != g_device_comp[comp->index]) {
		dpu_pr_err("%s input have not registed match!", comp->base.name);
		return;
	}

	down(&comp->blank_sem);
	if (comp->power_on) {
		dpu_pr_info("already power on\n");
		up(&comp->blank_sem);
		return;
	}
	up(&comp->blank_sem);

	event.data = NULL;
	event.panel_id = (int32_t)comp->base.id;
	event.value = DISP_BLANK_UNBLANK;

	dpu_pr_warn("resume will do abnormal power on\n");
	gfxdev_blank_power_on(comp);
	dkmd_notifier_call_chain(DKMD_EVENT_BLANK, (void *)&event);
}

void device_mgr_register_comp(struct composer *comp)
{
	if (!comp) {
		dpu_pr_err("input comp is null!");
		return;
	}

	if (is_dp_panel(&comp->base) && strcmp(comp->base.name, "gfx_dp") == 0) {
		comp->index = DEVICE_COMP_DP_ID;
	} else if (is_dp_panel(&comp->base) && strcmp(comp->base.name, "gfx_dp1") == 0) {
		comp->index = DEVICE_COMP_DP_ID_1;
	} else if (is_dp_panel(&comp->base) && strcmp(comp->base.name, "gfx_dp2") == 0) {
		comp->index = DEVICE_COMP_DP_ID_2;
	} else if (is_offline_panel(&comp->base)) {
		comp->index = DEVICE_COMP_VIRTUAL_ID;
	} else if (is_hdmi_panel(&comp->base)) {
		comp->index = DEVICE_COMP_HDMI_ID;
	} else if (is_builtin_panel(&comp->base)) {
		comp->index = DEVICE_COMP_BUILTIN_ID;
	} else if (is_primary_panel(&comp->base)) {
		comp->index = DEVICE_COMP_PRIMARY_ID;
	} else {
		comp->index = DEVICE_COMP_MAX_COUNT;
		dpu_pr_err("type(%u) invalid, please check", comp->base.type);
		return;
	}

	if (g_device_comp[comp->index] != NULL) {
		dpu_pr_err("%s register type = %#x, index = %u failed, fake panel = %u please check",
			comp->base.name, comp->base.type, comp->index, comp->base.fake_panel_flag);
		return;
	}

	g_device_comp[comp->index] = comp;
	dpu_pr_info("%s register index = %u, type = %#x success",
		comp->base.name, comp->index, comp->base.type);
}

static int32_t gfxdev_probe(struct platform_device *pdev)
{
	int32_t ret;

	ret = of_property_read_u32(pdev->dev.of_node, "disp_device_arch", &g_disp_device_arch);
	if (ret) {
		dpu_pr_info("get disp_device_arch failed!");
		g_disp_device_arch = FBDEV_ARCH;
	}
	dpu_pr_info("g_disp_device_arch=%#x", g_disp_device_arch);

	ret = of_property_read_u32(pdev->dev.of_node, "fastboot_enable_flag", &g_fastboot_enable_flag);
	if (ret) {
		dpu_pr_info("get fastboot_enable_flag failed!");
		g_fastboot_enable_flag = 0;
	}
	dpu_pr_info("g_fastboot_enable_flag=%#x", g_fastboot_enable_flag);

	ret = of_property_read_u32(pdev->dev.of_node, "fake_lcd_flag", &g_fake_lcd_flag);
	if (ret) {
		dpu_pr_info("get fake_lcd_flag failed!");
		g_fake_lcd_flag = 0;
	}
	dpu_pr_info("g_fake_lcd_flag=%#x", g_fake_lcd_flag);

	return 0;
}

static int32_t gfxdev_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id gfxdev_match_table[] = {
	{
		.compatible = "dkmd,dpu_device",
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, gfxdev_match_table);

static struct platform_driver gfxdev_driver = {
	.probe = gfxdev_probe,
	.remove = gfxdev_remove,
	.driver = {
		.name = "gfxdev",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(gfxdev_match_table),
	},
};

static int32_t __init gfxdev_driver_init(void)
{
	return platform_driver_register(&gfxdev_driver);
}

static void __exit gfxdev_driver_deinit(void)
{
	platform_driver_unregister(&gfxdev_driver);
}

module_init(gfxdev_driver_init);
module_exit(gfxdev_driver_deinit);

MODULE_DESCRIPTION("Display Graphics Driver");
MODULE_LICENSE("GPL");
