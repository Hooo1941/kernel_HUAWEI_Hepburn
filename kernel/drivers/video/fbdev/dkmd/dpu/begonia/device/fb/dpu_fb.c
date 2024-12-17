/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#include "dkmd_log.h"

#include <linux/leds.h>
#include <linux/fb.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <dkmd_dpu.h>
#include <dkmd_ffd.h>

#include "dkmd_base_frame.h"
#include "dkmd_network.h"
#include "dkmd_dpu.h"
#include "dkmd_object.h"
#include "gfxdev_mgr.h"
#include "dpu_fb.h"
#include "dpu_ion_mem.h"
#include "dkmd_comp.h"
#include "res_mgr.h"
#include "dpu_comp_mgr.h"
#include "peri/dkmd_peri.h"
#include "gfxdev_utils.h"

#define FB_BUFFER_MAX_COUNT 3

#ifdef CONFIG_LEDS_CLASS
#define BL_BACKLIGHT_NAME "lcd_backlight0"
#endif

static int32_t dpu_fb_get_product_config(struct device_fb *dpu_fb, void __user *argp)
{
	int32_t ret = 0;
	struct product_config config = {0};
	struct composer *comp = dpu_fb->composer;

	if (!comp) {
		dpu_pr_err("comp is nullptr");
		return -1;
	}
	if (!argp) {
		dpu_pr_err("argp is nullptr");
		return -1;
	}
	if (!comp->get_product_config) {
		dpu_pr_err("comp get product config function is nullptr");
		return -1;
	}

	ret = comp->get_product_config(comp, &config);
	if (unlikely(ret != 0)) {
		dpu_pr_err("%s get product config fail", comp->base.name);
		return -1;
	}

	config.drv_feature.bits.drv_framework = DRV_FB;
	config.drv_feature.bits.is_pluggable = 0;

	config.scene_info = comp->base.scene_info;
	config.opr_policy = comp->base.opr_policy;
	config.compose_policy = comp->base.compose_policy;
	dpu_pr_info("%s comp scene_count:%u, scene_id[0]:%u, opr_policy[%#x] compose_policy[%#x]",
		dpu_fb->pinfo->name, comp->base.scene_info.scene_count, comp->base.scene_info.scene_id[0],
		comp->base.opr_policy, comp->base.compose_policy);
	config.feature_switch.bits.enable_lbuf_reserve = comp->base.enable_lbuf_reserve;
	dpu_pr_info("%s comp enable_lbuf_reserve value:%u", dpu_fb->pinfo->name,
		comp->base.enable_lbuf_reserve);

	if (copy_to_user(argp, &config, sizeof(config)) != 0) {
		dpu_pr_err("copy product config to user fail");
		return -1;
	}

	return 0;
}

static int dpu_fb_get_hdr_mean(struct device_fb *fb_dev, void __user *argp)
{
	int32_t ret = -1;
	uint32_t hdr_mean = 0;
	struct composer *comp = fb_dev->composer;
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");

	if (!comp) {
		dpu_pr_info("comp is nullptr");
		return -1;
	}

	if (!comp->get_hdr_mean) {
		dpu_pr_err("comp get hdr mean function is nullptr");
		return -1;
	}

	ret = comp->get_hdr_mean(comp, &hdr_mean);
	if (unlikely(ret != 0)) {
		dpu_pr_err("%s get hdr mean fail", comp->base.name);
		return -1;
	}

	ret = (int32_t)copy_to_user(argp, &hdr_mean, sizeof(uint32_t));
	if (ret) {
		dpu_pr_err("copy_to_user failed ret=%d.\n", ret);
		return -1;
	}

	return 0;
}

static int dpu_fb_get_alsc_info(struct device_fb *fb_dev, void __user *argp)
{
	int32_t ret = -1;
	struct alsc_info info = {0};
	struct composer *comp = fb_dev->composer;
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");

	if (!comp) {
		dpu_pr_info("comp is nullptr");
		return -1;
	}

	if (!comp->get_alsc_info) {
		dpu_pr_debug("the comp %s not support alsc", comp->base.name);
		return 0;
	}

	ret = comp->get_alsc_info(comp, &info);
	if (unlikely(ret != 0)) {
		dpu_pr_err("%s get alsc info fail", comp->base.name);
		return -1;
	}

	ret = (int32_t)copy_to_user(argp, &info, sizeof(struct alsc_info));
	if (ret) {
		dpu_pr_err("copy_to_user failed ret=%d.\n", ret);
		return -1;
	}

	return 0;
}

static int dpu_fb_get_fusa_info(struct device_fb *fb_dev, void __user *argp)
{
	int32_t ret = -1;
	struct dkmd_ffd_cfg info = {0};
	struct composer *comp = fb_dev->composer;
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");

	if (!comp) {
		dpu_pr_info("comp is nullptr");
		return -1;
	}

	if (!comp->get_fusa_info) {
		dpu_pr_debug("the comp %s not support fusa", comp->base.name);
		return 0;
	}

	ret = comp->get_fusa_info(comp, &info);
	if (unlikely(ret != 0)) {
		dpu_pr_err("%s get fusa info fail", comp->base.name);
		return -1;
	}

	ret = (int32_t)copy_to_user(argp, &info, sizeof(struct dkmd_ffd_cfg));
	if (ret) {
		dpu_pr_err("copy_to_user failed ret=%d.\n", ret);
		return -1;
	}

	return 0;
}

// ============================================================================

static int32_t dpu_fb_open(struct fb_info *info, int32_t user)
{
	int32_t ret = 0;
	struct composer *comp = NULL;
	struct device_fb *dpu_fb = NULL;
	struct dpu_composer *dpu_comp = NULL;
	bool fastboot_enable = false;

	if (unlikely(!info))
		return -ENODEV;

	comp = (struct composer *)info->par;
	if (unlikely(!comp))
		return -EINVAL;

	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return -EINVAL;
	}

	dpu_pr_info("ref_cnt=%d enter", atomic_read(&dpu_fb->ref_cnt));

	/* fb have been opened */
	if (atomic_read(&dpu_fb->ref_cnt) == 0) {
		/* if fastboot display enabled, skip blank
		 * else, need unblank dpu
		 */
		dpu_comp = to_dpu_composer(comp);
		dpu_comp_active_vsync(dpu_comp);
		fastboot_enable = dpu_gfxdev_set_fastboot(dpu_fb->composer);
		dpu_comp_deactive_vsync(dpu_comp);
		if (!fastboot_enable)
			ret = dpu_gfxdev_blank(dpu_fb->composer, FB_BLANK_UNBLANK);
	}
	atomic_inc(&dpu_fb->ref_cnt);

	dpu_pr_info("ref_cnt=%d exit", atomic_read(&dpu_fb->ref_cnt));
	return ret;
}

static int32_t dpu_fb_release(struct fb_info *info, int32_t user)
{
	int32_t ret = 0;
	struct composer *comp = NULL;
	struct device_fb *dpu_fb = NULL;

	if (unlikely(!info))
		return -ENODEV;

	comp = (struct composer *)info->par;
	if (unlikely(!comp))
		return -EINVAL;

	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return -EINVAL;
	}
	dpu_pr_info("ref_cnt=%d enter", atomic_read(&dpu_fb->ref_cnt));

	/* fb have not been opened, return */
	if (atomic_read(&dpu_fb->ref_cnt) == 0) {
		dpu_pr_warn("gfx%u is not opened, cannot release", dpu_fb->index);
		return 0;
	}

	if (!atomic_sub_and_test(1, &dpu_fb->ref_cnt)) {
		dpu_pr_info("gfx%u not need release, cnt = %u", dpu_fb->index, dpu_fb->ref_cnt);
		return 0;
	}

	/* ref_cnt is 0, need blank */
	ret = dpu_gfxdev_blank(dpu_fb->composer, DISP_BLANK_POWERDOWN);
	if (ret)
		dpu_pr_err("gfx%u, blank power down error, ret=%d", dpu_fb->index, ret);
	dpu_free_fb_buffer(info);

	if (dpu_fb->composer->release)
		dpu_fb->composer->release(dpu_fb->composer);

	return ret;
}

static ssize_t dpu_fb_read(struct fb_info *info, char __user *buf,
	size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dpu_fb_write(struct fb_info *info, const char __user *buf,
	size_t count, loff_t *ppos)
{
	int err;

	if (unlikely(!buf || !ppos)) {
		dpu_pr_err("buf or ppos is null!");
		return -ENODEV;
	}

	if (unlikely(count == 0)) {
		dpu_pr_err("count is 0!");
		return -ENODEV;
	}

	if (!info)
		return -ENODEV;

	if (!buf)
		return -ENODEV;

	if (!ppos)
		return -ENODEV;

	if (lock_fb_info(info) == 0)
		return -ENODEV;

	if (!info->screen_base) {
		unlock_fb_info(info);
		return -ENODEV;
	}

	err = fb_sys_write(info, buf, count, ppos);

	unlock_fb_info(info);

	return err;
}

static int dpu_fb_check_var(gfxdev_var_screeninfo *var, struct fb_info *info)
{
	struct composer *comp = NULL;
	struct device_fb *dpu_fb = NULL;

	if (unlikely(!var))
		return -EINVAL;

	if (unlikely(!info))
		return -ENODEV;

	comp = (struct composer *)info->par;
	if (unlikely(!comp))
		return -EINVAL;

	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return -EINVAL;
	}

	if (var->rotate != FB_ROTATE_UR) {
		dpu_pr_err("error rotate %u", var->rotate);
		return -EINVAL;
	}

	if (var->grayscale != info->var.grayscale) {
		dpu_pr_err("error grayscale %u", var->grayscale);
		return -EINVAL;
	}

	if (unlikely((var->xres_virtual == 0) || (var->yres_virtual == 0))) {
		dpu_pr_err("xres_virtual=%u yres_virtual=%u out of range", var->xres_virtual, var->yres_virtual);
		return -EINVAL;
	}

	if (unlikely((var->xres == 0) || (var->yres == 0))) {
		dpu_pr_err("xres=%u, yres=%u is invalid", var->xres, var->yres);
		return -EINVAL;
	}

	if (var->xoffset > (var->xres_virtual - var->xres)) {
		dpu_pr_err("xoffset=%u is invalid, xres_virtual=%u xres=%u", var->xoffset, var->xres_virtual, var->xres);
		return -EINVAL;
	}

	if (var->yoffset > (var->yres_virtual - var->yres)) {
		dpu_pr_err("yoffset=%u is invalid, yres_virtual=%u yres=%u", var->yoffset, var->yres_virtual, var->yres);
		return -EINVAL;
	}

	return 0;
}

static int dpu_fb_set_par(struct fb_info *info)
{
	if (!info)
		return -ENODEV;

	return 0;
}

static int32_t dpu_fb_blank(int32_t blank_mode, struct fb_info *info)
{
	int32_t ret = 0;
	struct composer *comp = NULL;
	struct device_fb *dpu_fb = NULL;

	if (unlikely(!info))
		return -ENODEV;

	comp = (struct composer *)info->par;
	if (unlikely(!comp))
		return -EINVAL;

	/* obtain dpu_fb to process private data if needed */
	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return -EINVAL;
	}

	/* TODO:
	 * other modules listen to the blank event, FB_EARLY_EVENT_BLANK or FB_EVENT_BLANK
	 * such as, AOD. those event had been sended at fbmen.c
	 */
	ret = dpu_gfxdev_blank(dpu_fb->composer, blank_mode);
	if (ret)
		dpu_pr_info("gfx%u blank sub fail", dpu_fb->index);

	return ret;
}

static int32_t dpu_fb_pan_display(gfxdev_var_screeninfo *var, struct fb_info *info)
{
	struct composer *comp = NULL;
	struct device_fb *dpu_fb = NULL;
	void_unused(var);

	if (unlikely(!info)) {
		dpu_pr_err("info is null");
		return -EINVAL;
	}
	comp = (struct composer *)info->par;

	if (unlikely(!comp)) {
		dpu_pr_err("info->par is null");
		return -EINVAL;
	}

	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return -EINVAL;
	}

	return gfxdev_pan_display(&info->fix, &info->var, dpu_fb->pinfo, comp);
}

static int32_t dpu_fb_comp_lcdkit_ioctl(struct dpu_composer *dpu_comp, uint32_t cmd, unsigned long arg)
{
	if (unlikely(!dpu_comp)) {
		dpu_pr_err("dpu_comp is null");
		return -EINVAL;
	}

	if (unlikely(!dpu_comp->comp_mgr->dpu_lcdkit_ops)) {
		dpu_pr_warn("dpu_lcdkit_ops is null");
		return -EINVAL;
	}

	if (dpu_comp->comp_mgr->dpu_lcdkit_ops->fb_ioctl)
		return dpu_comp->comp_mgr->dpu_lcdkit_ops->fb_ioctl(cmd, arg);

	return 0;
}

static int32_t dpu_fb_hiace_get_hist(struct fb_info *info, void __user* argp)
{
	struct composer *comp = (struct composer *)info->par;

	if (!argp) {
		dpu_pr_err("argp is nullptr");
		return -1;
	}

	if (!comp->effect_hiace_get_hist) {
		dpu_pr_info("comp fb hiace get hist function is nullptr");
		return -1;
	}

	comp->effect_hiace_get_hist(comp, argp);
	return 0;
}

static int32_t dpu_fb_hiace_set_lut(struct composer *comp, void __user* argp)
{
	if (!comp->effect_hiace_set_lut) {
		dpu_pr_info("comp hiace set lut function is nullptr");
		return -1;
	}

	return comp->effect_hiace_set_lut(comp, argp);
}

static int32_t dpu_fb_wake_up_hiace_hist(struct composer *comp)
{
	if (!comp->effect_wake_up_hiace_hist) {
		dpu_pr_info("comp wake up hiace hist function is nullptr");
		return -1;
	}

	return comp->effect_wake_up_hiace_hist(comp);
}

static int32_t dpu_fb_ioctl(struct fb_info *info, uint32_t cmd, unsigned long arg)
{
	int32_t ret = 0;
	struct composer *comp = NULL;
	struct device_fb *dpu_fb = NULL;
	void __user *argp = (void __user *)(uintptr_t)arg;

	if (unlikely(!argp))
		return -EINVAL;

	if (unlikely(!info))
		return -ENODEV;

	comp = (struct composer *)info->par;
	if (unlikely(!comp))
		return -EINVAL;

	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return -EINVAL;
	}
	/* unlock fb to avoid blocking other ioctl, and lock fb when exit */
	unlock_fb_info(info);

	dpu_pr_debug("gfx%u ioctl, cmd=%#x", dpu_fb->index, cmd);
	switch (cmd) {
	case DISP_CREATE_FENCE:
		ret = dpu_gfxdev_create_fence(dpu_fb->composer, argp);
		break;
	case DISP_RELEASE_FENCE:
		ret = dpu_gfxdev_release_fence(dpu_fb->composer, argp);
		break;
	case DISP_PRESENT:
		ret = dpu_gfxdev_present(dpu_fb->composer, dpu_fb->pinfo, argp);
		break;
	case DISP_GET_PRODUCT_CONFIG:
		ret = dpu_fb_get_product_config(dpu_fb, argp);
		break;
	case DISP_GET_HDR_MEAN:
		ret = dpu_fb_get_hdr_mean(dpu_fb, argp);
		break;
	case DISP_GET_ALSC_INFO:
		ret = dpu_fb_get_alsc_info(dpu_fb, argp);
		break;
	case DISP_GET_FUSA_INFO:
		ret = dpu_fb_get_fusa_info(dpu_fb, argp);
		break;
	case DISP_GET_HIACE_HIST:
		ret = dpu_fb_hiace_get_hist(info, argp);
		break;
	case DISP_SET_HIACE_LUT:
		ret = dpu_fb_hiace_set_lut(comp, argp);
		break;
	case DISP_WAKE_UP_HIACE_HIST:
		ret = dpu_fb_wake_up_hiace_hist(comp);
		break;
	case DISP_SAFE_FRM_RATE:
		ret = dpu_gfxdev_set_safe_frm_rate(dpu_fb->composer, argp);
		break;
	case DISP_SET_ACTIVE_RECT:
		ret = dpu_gfxdev_set_active_rect(dpu_fb->composer, argp);
		break;
	default:
		if (dpu_fb_comp_lcdkit_ioctl(to_dpu_composer(comp), cmd, arg) >= 0) {
			ret = 0;
			break;
		}

		dpu_pr_warn("gfx%u ioctl fail unsupport cmd, cmd=%#x", dpu_fb->index, cmd);
		ret = -1;
		break;
	}

	(void)lock_fb_info(info);
	return ret;
}

static int32_t dpu_fb_compat_ioctl(struct fb_info *info, uint32_t cmd, unsigned long arg)
{
	int ret;

	(void)lock_fb_info(info);
	ret = dpu_fb_ioctl(info, cmd, arg);
	unlock_fb_info(info);
	return ret;
}

static void fb_init_fbi_fix_info(struct device_fb *dpu_fb,
	struct fix_var_screeninfo *screen_info, gfxdev_fix_screeninfo *fix)
{
	int32_t ret = 0;
	struct composer *comp = dpu_fb->composer;

	if (unlikely(!comp || !screen_info || !fix)) {
		dpu_pr_err("input nullptr");
		return;
	}

	ret = gfxdev_init_fscreen_info(comp, screen_info, fix);
	if (ret != 0) {
		dpu_pr_err("init fscreen info err");
		return;
	}

	return;
}

static void fb_init_fbi_var_info(struct device_fb *fb,
	struct fix_var_screeninfo *screen_info, gfxdev_var_screeninfo *var)
{
	gfxdev_init_fbi_var_info(fb->pinfo, screen_info, var);
}

static struct fb_ops g_fb_ops = {
	.owner = THIS_MODULE,
	.fb_open = dpu_fb_open,
	.fb_release = dpu_fb_release,
	.fb_read = dpu_fb_read,
	.fb_write = dpu_fb_write,
	.fb_cursor = NULL,
	.fb_check_var = dpu_fb_check_var,
	.fb_set_par = dpu_fb_set_par,
	.fb_setcolreg = NULL,
	.fb_blank = dpu_fb_blank,
	.fb_pan_display = dpu_fb_pan_display,
	.fb_fillrect = NULL,
	.fb_copyarea = NULL,
	.fb_imageblit = NULL,
	.fb_sync = NULL,
	.fb_ioctl = dpu_fb_ioctl,
#ifdef CONFIG_COMPAT
	.fb_compat_ioctl = dpu_fb_compat_ioctl,
#endif
	.fb_mmap = dpu_fb_mmap,
#ifdef CONFIG_FB_KERNEL_LOGO
	.fb_buffer_alloc_k = dpu_fb_buffer_alloc_kernel,
#endif
};

struct composer *get_comp_from_fb_device(struct device *dev)
{
	struct fb_info *fbi = (struct fb_info *)dev_get_drvdata(dev);

	dpu_check_and_return(!fbi, NULL, err, "fbi is null pointer");

	return (struct composer *)fbi->par;
}

#ifdef CONFIG_LEDS_CLASS
static struct led_classdev bl_backlight_led = {
	.name = BL_BACKLIGHT_NAME,
	.brightness_set = bl_lcd_set_backlight,
};
#endif

int32_t fb_device_register(struct composer *comp)
{
	struct device_fb *fb = NULL;
	struct fb_info *fbi = NULL;
	struct dkmd_attr *comp_attr = NULL;
	struct dkmd_object_info *pinfo = &comp->base;
	struct dpu_composer *dpu_comp = to_dpu_composer(comp);
	struct fix_var_screeninfo* screeninfo = get_fix_var_screeninfo();

	dpu_pr_info("pdev register composer id:%d, device name: %s", comp->index, pinfo->name);

	fbi = framebuffer_alloc(sizeof(*fb), NULL);
	if (!fbi) {
		dpu_pr_err("alloc fbi fail");
		return -EINVAL;
	}
	fb = (struct device_fb *)fbi->par;
	fb->fbi_info = fbi;
	fb->index = comp->index;
	fb->composer = comp;
	fb->pinfo = pinfo;
	comp->device_data = fb;

	/* Refresh the pointer to composer */
	fbi->par = comp;
	fbi->fbops = &g_fb_ops;
	fbi->flags = FBINFO_FLAG_DEFAULT;
	fbi->pseudo_palette = NULL;

	fb_init_fbi_fix_info(fb, &screeninfo[GFXDEV_FORMAT_BGRA8888], &fbi->fix);
	fb_init_fbi_var_info(fb, &screeninfo[GFXDEV_FORMAT_BGRA8888], &fbi->var);

	if (register_framebuffer(fbi) < 0) {
		dpu_pr_err("gfx%u failed to register_framebuffer!", fb->index);
		dpu_free_fb_buffer(fbi);
		framebuffer_release(fbi);
		return -EINVAL;
	}

	/* lcdkit depends on fb dev */
	(void)pipeline_next_ops_handle(dpu_comp->conn_info->conn_device,
		dpu_comp->conn_info, REGIST_POSTPROCESS, fbi->dev);

	if (comp->get_sysfs_attrs) {
		comp->get_sysfs_attrs(comp, &comp_attr);
		if (comp_attr)
			dkmd_sysfs_create(fbi->dev, comp_attr);
	}

#ifdef CONFIG_LEDS_CLASS
	/* set backlight */
	if (led_classdev_register(fbi->dev, &bl_backlight_led)) {
		dpu_pr_err("led_classdev_register failed!");
		dpu_free_fb_buffer(fbi);
		framebuffer_release(fbi);
		return -EINVAL;
	}
#endif

	dpu_pr_info("Primary FrameBuffer[%u] at scene_id=%d %dx%d size=%d bytes is registered successfully!\n",
		fb->index, pinfo->pipe_sw_itfch_idx, fbi->var.xres, fbi->var.yres, fbi->fix.smem_len);

	return 0;
}

void fb_device_unregister(struct composer *comp)
{
	struct device_fb *fb = NULL;
	struct dkmd_attr *comp_attr = NULL;

	dpu_pr_err("in fb_device_unregister");
	if (!comp) {
		dpu_pr_err("comp is null!");
		return;
	}

	fb = (struct device_fb *)comp->device_data;
	if (!fb) {
		dpu_pr_err("fb is null!");
		return;
	}
	if (comp->get_sysfs_attrs) {
		comp->get_sysfs_attrs(comp, &comp_attr);
		if (comp_attr)
			dkmd_sysfs_remove(fb->fbi_info->dev, comp_attr);
	}
	unregister_framebuffer(fb->fbi_info);
	dpu_free_fb_buffer(fb->fbi_info);
	framebuffer_release(fb->fbi_info);

#ifdef CONFIG_LEDS_CLASS
	led_classdev_unregister(&bl_backlight_led);
#endif
}

void fb_device_shutdown(struct composer *comp)
{
	struct device_fb *fb = NULL;

	if (!comp) {
		dpu_pr_err("comp is null!");
		return;
	}

	fb = (struct device_fb *)comp->device_data;
	if (!fb) {
		dpu_pr_err("fb is null!");
		return;
	}

	dpu_gfxdev_blank(fb->composer, DISP_BLANK_POWERDOWN);
}
