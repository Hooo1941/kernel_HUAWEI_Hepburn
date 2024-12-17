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

#include "dkmd_log.h"

#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <dkmd_dpu.h>
#include <dpu/soc_dpu_define.h>
#include <dpu/dpu_itfsw.h>

#include "gfxdev_mgr.h"
#include "dkmd_object.h"
#include "dkmd_comp.h"
#include "dpu_gfx.h"
#include "dkmd_res_mgr.h"
#include "dkmd_base_frame.h"
#include "dkmd_network.h"
#include "dpu_comp_mgr.h"
#include "gfxdev_utils.h"
#include "dpu_gfx_ion_mem.h"

struct gfx_devno_info {
	dev_t devno;
	struct device_gfx *gfx_dev;
};

static struct gfx_devno_info g_gfx_devno_info[DEVICE_COMP_MAX_COUNT];
static uint32_t g_gfx_devno_info_index;

#ifdef CONFIG_DKMD_DEBUG_ENABLE
static ssize_t dpu_gfx_debug_show(struct device *dev, struct device_attribute *attr, char* buf)
{
	int32_t ret;
	struct composer *comp = NULL;
	struct device_gfx *gfx_dev = NULL;

	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");
	dpu_pr_info("dev->kobj.name: %s", dev->kobj.name);
	gfx_dev = (struct device_gfx *)dev_get_drvdata(dev);
	dpu_check_and_return(!gfx_dev, -1, err, "device_gfx is null pointer");
	comp = gfx_dev->composer;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", comp->power_on);
	buf[strlen(buf) + 1] = '\0';

	return ret;
}

static ssize_t dpu_gfx_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int32_t ret;
	int32_t blank_mode;
	struct device_gfx *gfx_dev = NULL;

	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");
	gfx_dev = (struct device_gfx *)dev_get_drvdata(dev);
	dpu_check_and_return(!gfx_dev, -1, err, "device_gfx is null pointer");

	ret = sscanf(buf, "%d", &blank_mode);
	if (ret == 0) {
		dpu_pr_err("get buf (%s) blank_mode fail\n", buf);
		return -1;
	}
	dpu_pr_info("blank_mode=%d", blank_mode);

	(void)dpu_gfxdev_blank(gfx_dev->composer, blank_mode);

	return (ssize_t)count;
}

static DEVICE_ATTR(gfx_debug, S_IRUSR | S_IRGRP | S_IWUSR, dpu_gfx_debug_show, dpu_gfx_debug_store);
#endif

static int32_t dpu_gfx_open(struct inode *inode, struct file *filp)
{
	uint32_t i;
	int32_t ret = 0;
	struct device_gfx *gfx_dev = NULL;
	struct composer *comp = NULL;
	struct dpu_composer *dpu_comp = NULL;
	bool fastboot_enable = false;

	dpu_check_and_return(unlikely(!inode), -EINVAL, err, "inode is null");
	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");

	if (!filp->private_data) {
		for (i = 0; i < DEVICE_COMP_MAX_COUNT; ++i) {
			if (inode->i_rdev == g_gfx_devno_info[i].devno) {
				filp->private_data = g_gfx_devno_info[i].gfx_dev;
				break;
			}
		}
	}
	gfx_dev = (struct device_gfx *)filp->private_data;
	if (gfx_dev == NULL) {
		dpu_pr_err("gfx_dev is null, error");
		return -EINVAL;
	}
	comp = gfx_dev->composer;
	if (unlikely(!comp))
		return -EINVAL;

	dpu_pr_info("ref_cnt = %d comp index = %u enter", atomic_read(&gfx_dev->ref_cnt), comp->index);
	dpu_comp = to_dpu_composer(comp);
	if (atomic_read(&gfx_dev->ref_cnt) == 0 &&
		dpu_comp->conn_info->base.pipe_sw_itfch_idx == PIPE_SW_PRE_ITFCH1 &&
		comp->index == DEVICE_COMP_BUILTIN_ID) {
		dpu_comp_active_vsync(dpu_comp);
		fastboot_enable = dpu_gfxdev_set_fastboot(comp);
		dpu_comp_deactive_vsync(dpu_comp);
		if (!fastboot_enable)
			ret = dpu_gfxdev_blank(comp, FB_BLANK_UNBLANK);
	}

	atomic_inc(&gfx_dev->ref_cnt);
	dpu_pr_warn("ref_cnt=%d, comp index = %u exit",
		atomic_read(&gfx_dev->ref_cnt), comp->index);
	return ret;
}

static int32_t dpu_gfx_release(struct inode *inode, struct file *filp)
{
	struct device_gfx *gfx_dev = NULL;
	int32_t ret = 0;

	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");
	dpu_check_and_return(unlikely(!filp->private_data), -EINVAL, err, "private_data is null");

	gfx_dev = (struct device_gfx *)filp->private_data;
	dpu_pr_warn("ref_cnt=%d", atomic_read(&gfx_dev->ref_cnt));

	if (unlikely(atomic_read(&gfx_dev->ref_cnt) == 0)) {
		dpu_pr_warn("gfx%u is not opened, cannot release", gfx_dev->index);
		return 0;
	}

	if (!atomic_sub_and_test(1, &gfx_dev->ref_cnt)) {
		dpu_pr_info("gfx%u not need release, cnt = %u", gfx_dev->index, gfx_dev->ref_cnt);
		return 0;
	}

	if (unlikely(!gfx_dev->composer)) {
		dpu_pr_err("composer is NULL");
		return 0;
	}

	ret = dpu_gfxdev_blank(gfx_dev->composer, DISP_BLANK_POWERDOWN);
	if (ret)
		dpu_pr_err("gfx%u, blank power down error, ret=%d", gfx_dev->index, ret);
	dpu_free_gfx_buffer(gfx_dev->composer);

	return ret;
}

static int32_t dpu_gfx_get_product_config(struct device_gfx *gfx_dev, void __user *argp)
{
	int32_t ret = 0;
	uint32_t i = 0;
	struct product_config config = {0};
	struct composer *comp = gfx_dev->composer;

	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");

	config.drv_feature.bits.drv_framework = DRV_CHR;
	if (!comp->get_product_config) {
		dpu_pr_info("comp get product config function is nullptr");
		return 0;
	}

	ret = comp->get_product_config(comp, &config);
	if (unlikely(ret != 0)) {
		dpu_pr_err("%s get product config fail", comp->base.name);
		return -1;
	}

	config.feature_switch.bits.enable_lbuf_reserve = comp->base.enable_lbuf_reserve;
	dpu_pr_info("%s comp enable_lbuf_reserve value:%u", gfx_dev->pinfo->name,
		comp->base.enable_lbuf_reserve);

	config.scene_info = comp->base.scene_info;
	config.opr_policy = comp->base.opr_policy;
	config.compose_policy = comp->base.compose_policy;
	dpu_pr_info("%s comp scene_count:%u, scene_id[0]:%u, opr_policy[%#x] compose_policy[%#x]",
		gfx_dev->pinfo->name, comp->base.scene_info.scene_count, comp->base.scene_info.scene_id[0],
		comp->base.opr_policy, comp->base.compose_policy);

	if ((strstr(gfx_dev->pinfo->name, DEV_NAME_DP) != NULL) || (strncmp(gfx_dev->pinfo->name, DEV_NAME_HDMI, 8) == 0)) {
		config.width = comp->base.width;
		config.height = comp->base.height;
		config.dim_info_count = 1;
		config.dim_info[0].width = (int32_t)(comp->base.xres);
		config.dim_info[0].height = (int32_t)(comp->base.yres);
		config.fps_info_count = 1;
		config.fps_info[0] = comp->base.fps;
		config.drv_feature.bits.is_pluggable = comp->base.is_pluggable; // hotpluggable

		if (comp->base.display_num != 0)
			config.display_count = (uint16_t)comp->base.display_num;

		dpu_pr_info("sub dim info count: %u, pluggable: %lu",
			comp->base.display_num, config.drv_feature.bits.is_pluggable);
		if (comp->base.display_num > 0 && comp->base.display_num < SPLIT_SCREEN_MAX) {
			for (i = 0; i < comp->base.display_num; i++) {
				config.sub_dim_info[i][DIMENSION_NORMAL].height = (int32_t)comp->base.display_info[i].yres;
				config.sub_dim_info[i][DIMENSION_NORMAL].width = (int32_t)comp->base.display_info[i].xres;
				dpu_pr_info("sub_dim_info[%d] : width: %d, hight: %d", i,
					config.sub_dim_info[i][DIMENSION_NORMAL].width,
					config.sub_dim_info[i][DIMENSION_NORMAL].height);
			}
		}
		dpu_pr_info("%s comp get xres[%d] yres[%d] fps[%d]",
			gfx_dev->pinfo->name, comp->base.xres, comp->base.yres, comp->base.fps);
	} else if (strncmp(gfx_dev->pinfo->name, "gfx_builtin", 11) == 0) {
		config.width = comp->base.width;
		config.height = comp->base.height;
		config.dim_info_count = 1;
		config.dim_info[0].width = (int32_t)(comp->base.xres);
		config.dim_info[0].height = (int32_t)(comp->base.yres);
		if (config.fps_info_count == 0) {
			config.fps_info[0] = comp->base.fps;
			config.fps_info_count = 1;
		}
		config.drv_feature.bits.is_pluggable = 0;

		dpu_pr_info("builtin comp get xres[%d] yres[%d] fps[%d]",
			comp->base.xres, comp->base.yres, comp->base.fps);
	}

	if (copy_to_user(argp, &config, sizeof(config)) != 0) {
		dpu_pr_err("copy product config to user fail");
		return -1;
	}

	return 0;
}

static int dpu_gfx_get_hdr_mean(struct device_gfx *gfx_dev, void __user *argp)
{
	int32_t ret = -1;
	uint32_t hdr_mean = 0;
	struct composer *comp = gfx_dev->composer;
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");

	if (!comp) {
		dpu_pr_info("comp is nullptr");
		return -1;
	}

	if (!comp->get_hdr_mean) {
		dpu_pr_info("comp get hdr mean function is nullptr");
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

static int dpu_gfx_get_alsc_info(struct device_gfx *gfx_dev, void __user *argp)
{
	int32_t ret = -1;
	struct alsc_info info = {0};
	struct composer *comp = gfx_dev->composer;
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

static int32_t dpu_gfx_get_vscreen_info(struct device_gfx *gfx_dev, void __user *argp)
{
	struct fix_var_screeninfo *screen_info = get_fix_var_screeninfo();
	struct composer *comp = gfx_dev->composer;
	const struct dkmd_object_info *pinfo = gfx_dev->pinfo;

	if (unlikely(!argp || !comp || !screen_info || !pinfo))
		return -EINVAL;

	gfxdev_init_fbi_var_info(pinfo, &screen_info[GFXDEV_FORMAT_BGRA8888], &gfx_dev->gfx_var);

	if (copy_to_user(argp, &gfx_dev->gfx_var, sizeof(gfx_dev->gfx_var)) != 0) {
		dpu_pr_err("copy to user failed!");
		return -EFAULT;
	}

	return 0;
}

/* use for bms animation */
static int32_t dpu_gfx_put_vscreen_info(struct device_gfx *gfx_dev, void __user *argp)
{
	if (unlikely(!argp))
		return -EINVAL;

	if (copy_from_user(&gfx_dev->gfx_var, argp, sizeof(gfx_dev->gfx_var)))
		return -EFAULT;

	return 0;
}

static int32_t dpu_gfx_get_fscreen_info(struct device_gfx *gfx_dev, void __user *argp)
{
	struct fix_var_screeninfo *screen_info = get_fix_var_screeninfo();
	struct composer *comp = gfx_dev->composer;
	int32_t ret;

	if (unlikely(!argp || !comp || !screen_info))
		return -EINVAL;

	ret = gfxdev_init_fscreen_info(comp, &screen_info[GFXDEV_FORMAT_BGRA8888], &gfx_dev->gfx_fix);
	if (ret != 0) {
		dpu_pr_err("init fscreen info err");
		return -1;
	}

	dpu_pr_info("get screen xres = %u, yres = %u", comp->base.xres, comp->base.yres);
	if (copy_to_user(argp, &gfx_dev->gfx_fix, sizeof(gfx_dev->gfx_fix)) != 0) {
		dpu_pr_err("copy to user failed!");
		return -EFAULT;
	}

	return 0;
}

static int32_t dpu_gfx_pan_display(struct device_gfx *gfx_dev, void __user *argp)
{
	struct composer *comp = gfx_dev->composer;
	const struct dkmd_object_info *pinfo = gfx_dev->pinfo;

	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");
	if (copy_from_user(&gfx_dev->gfx_var, argp, sizeof(gfx_dev->gfx_var)) != 0) {
		dpu_pr_err("copy from user fb info fail");
		return -EINVAL;
	}
	if (!comp) {
		dpu_pr_err("comp is null");
		return -EINVAL;
	}

	return gfxdev_pan_display(&gfx_dev->gfx_fix, &gfx_dev->gfx_var, pinfo, comp);
}

static int32_t dpu_gfx_hiace_get_hist(struct composer *comp, void __user *argp)
{
	dpu_check_and_return(unlikely(!comp), -EINVAL, err, "comp is null pointer");
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");

	if (!comp->effect_hiace_get_hist) {
		dpu_pr_info("comp gfx hiace get hist function is nullptr");
		return -1;
	}

	return comp->effect_hiace_get_hist(comp, argp);
}

static int32_t dpu_gfx_hiace_set_lut(struct composer *comp, void __user *argp)
{
	dpu_check_and_return(unlikely(!comp), -EINVAL, err, "comp is null pointer");
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");

	if (!comp->effect_hiace_set_lut) {
		dpu_pr_info("comp hiace set lut function is nullptr");
		return -1;
	}

	return comp->effect_hiace_set_lut(comp, argp);
}

static int32_t dpu_gfx_wake_up_hiace_hist(struct composer *comp)
{
	dpu_check_and_return(unlikely(!comp), -EINVAL, err, "comp is null pointer");

	if (!comp->effect_wake_up_hiace_hist) {
		dpu_pr_info("comp wake up hiace hist function is nullptr");
		return -1;
	}

	return comp->effect_wake_up_hiace_hist(comp);
}

static int32_t dpu_gfx_hdcp_increase_counter(struct composer *comp)
{
	dpu_check_and_return(unlikely(!comp), -EINVAL, err, "comp is null pointer");

	if (!comp->hdcp_increase_counter) {
		dpu_pr_info("comp hdcp increase counter function is nullptr");
		return -1;
	}

	return comp->hdcp_increase_counter(comp);
}

static int32_t dpu_gfx_hdcp_decrease_counter(struct composer *comp)
{
	dpu_check_and_return(unlikely(!comp), -EINVAL, err, "comp is null pointer");

	if (!comp->hdcp_decrease_counter) {
		dpu_pr_info("comp hdcp decrease counter function is nullptr");
		return -1;
	}

	return comp->hdcp_decrease_counter(comp);
}

static long dpu_gfx_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct device_gfx *gfx_dev = NULL;
	int32_t ret = -1;

	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");

	gfx_dev = (struct device_gfx *)filp->private_data;
	dpu_check_and_return(unlikely(!gfx_dev), -EINVAL, err, "private_data is null");
	dpu_pr_debug("gfx%u ioctl, cmd=%#x", gfx_dev->index, cmd);

	switch (cmd) {
	case DISP_IOBLANK:
	case DISP_GFX_IOBLANK:
		ret = dpu_gfxdev_blank(gfx_dev->composer, (int32_t)arg);
		break;
	case DISP_PRESENT:
		ret = dpu_gfxdev_present(gfx_dev->composer, gfx_dev->pinfo, argp);
		break;
	case DISP_CREATE_FENCE:
		ret = dpu_gfxdev_create_fence(gfx_dev->composer, argp);
		break;
	case DISP_RELEASE_FENCE:
		ret = dpu_gfxdev_release_fence(gfx_dev->composer, argp);
		break;
	case DISP_GET_PRODUCT_CONFIG:
		ret = dpu_gfx_get_product_config(gfx_dev, argp);
		break;
	case DISP_GET_HDR_MEAN:
		ret = dpu_gfx_get_hdr_mean(gfx_dev, argp);
		break;
	case DISP_GFX_GET_VAR_INFO:
		mutex_lock(&gfx_dev->lock);
		ret = dpu_gfx_get_vscreen_info(gfx_dev, argp);
		mutex_unlock(&gfx_dev->lock);
		break;
	case DISP_GFX_PUT_VAR_INFO:
		mutex_lock(&gfx_dev->lock);
		ret = dpu_gfx_put_vscreen_info(gfx_dev, argp);
		mutex_unlock(&gfx_dev->lock);
		break;
	case DISP_GFX_GET_FIX_INFO:
		ret = dpu_gfx_get_fscreen_info(gfx_dev, argp);
		break;
	case DISP_GFX_PAN_DISPLAY:
		mutex_lock(&gfx_dev->lock);
		ret = dpu_gfx_pan_display(gfx_dev, argp);
		mutex_unlock(&gfx_dev->lock);
		break;
	case DISP_GET_ALSC_INFO:
		ret = dpu_gfx_get_alsc_info(gfx_dev, argp);
		break;
	case DISP_GET_HIACE_HIST:
		ret = dpu_gfx_hiace_get_hist(gfx_dev->composer, argp);
		break;
	case DISP_SET_HIACE_LUT:
		ret = dpu_gfx_hiace_set_lut(gfx_dev->composer, argp);
		break;
	case DISP_WAKE_UP_HIACE_HIST:
		ret = dpu_gfx_wake_up_hiace_hist(gfx_dev->composer);
		break;
	case DISP_SAFE_FRM_RATE:
		ret = dpu_gfxdev_set_safe_frm_rate(gfx_dev->composer, argp);
		break;
	case DISP_HDCP_IN:
		ret = dpu_gfx_hdcp_increase_counter(gfx_dev->composer);
		break;
	case DISP_HDCP_OUT:
		ret = dpu_gfx_hdcp_decrease_counter(gfx_dev->composer);
		break;
	case DISP_SET_ACTIVE_RECT:
		ret = dpu_gfxdev_set_active_rect(gfx_dev->composer, argp);
		break;
	default:
		dpu_pr_info("unsupported cmd=%#x", cmd);
		return -EINVAL;
	}

	return ret;
}

static struct file_operations dpu_gfx_fops = {
	.owner = THIS_MODULE,
	.open = dpu_gfx_open,
	.release = dpu_gfx_release,
	.unlocked_ioctl = dpu_gfx_ioctl,
	.compat_ioctl =  dpu_gfx_ioctl,
	.mmap = dpu_gfx_mmap,
};

struct composer *get_comp_from_gfx_device(struct device *dev)
{
	struct device_gfx *gfx = (struct device_gfx *)dev_get_drvdata(dev);

	dpu_check_and_return(!gfx, NULL, err, "device_gfx is null pointer");

	return gfx->composer;
}

int32_t gfx_device_register(struct composer *comp)
{
	struct device_gfx *gfx = NULL;
	struct dkmd_attr *comp_attr = NULL;
	struct dkmd_object_info *pinfo = &comp->base;

	if (g_gfx_devno_info_index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("g_gfx_devno_info_index=%u exceed max %d", g_gfx_devno_info_index, DEVICE_COMP_MAX_COUNT);
		return -EINVAL;
	}

	gfx = (struct device_gfx *)kzalloc(sizeof(*gfx), GFP_KERNEL);
	if (unlikely(!gfx)) {
		dpu_pr_err("alloc gfx device failed");
		return -EINVAL;
	}
	dpu_pr_info("%s register device: %s", comp->base.name, pinfo->name);

	gfx->index = comp->index;
	gfx->composer = comp;
	gfx->pinfo = pinfo;

	/* init chrdev info */
	gfx->chrdev.name = pinfo->name;
	gfx->chrdev.fops = &dpu_gfx_fops;
	gfx->chrdev.drv_data = gfx;
	if (unlikely(dkmd_create_chrdev(&gfx->chrdev) != 0)) {
		dpu_pr_err("create chr device failed");
		kfree(gfx);
		gfx = NULL;
		return -EINVAL;
	}

	if (comp->get_sysfs_attrs) {
		comp->get_sysfs_attrs(comp, &comp_attr);
		if (comp_attr) {
#ifdef CONFIG_DKMD_DEBUG_ENABLE
			dkmd_sysfs_attrs_append(comp_attr, &dev_attr_gfx_debug.attr);
#endif
			dkmd_sysfs_create(gfx->chrdev.chr_dev, comp_attr);
		}
	}

	mutex_init(&gfx->lock);
	comp->device_data = gfx;
	g_gfx_devno_info[g_gfx_devno_info_index].devno = gfx->chrdev.devno;
	g_gfx_devno_info[g_gfx_devno_info_index++].gfx_dev = gfx;

	return 0;
}

void gfx_device_unregister(struct composer *comp)
{
	struct device_gfx *gfx = NULL;
	struct dkmd_attr *comp_attr = NULL;

	dpu_pr_err("in gfx_device_unregister");
	if (unlikely(!comp))
		return;

	gfx = (struct device_gfx *)comp->device_data;
	if (unlikely(!gfx))
		return;

	if (comp->get_sysfs_attrs) {
		comp->get_sysfs_attrs(comp, &comp_attr);
		if (comp_attr)
			dkmd_sysfs_remove(gfx->chrdev.chr_dev, comp_attr);
	}
	dpu_free_gfx_buffer(comp);
	dkmd_destroy_chrdev(&gfx->chrdev);

	kfree(gfx);
	gfx = NULL;
	comp->device_data = NULL;
}
