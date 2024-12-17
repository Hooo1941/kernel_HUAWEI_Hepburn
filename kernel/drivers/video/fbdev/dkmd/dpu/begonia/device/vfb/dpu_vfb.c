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

#include <linux/dma-buf.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <securec.h>

#include "dkmd_log.h"
#include "dkmd_dpu.h"
#include "dkmd_vfb.h"
#include "dkmd_chrdev.h"
#include "dkmd_acquire_fence.h"
#include "gfxdev_utils.h"

#include "dpu_vfb_fence.h"
#include "dpu_vfb_bufmgr.h"
#include "dpu_vfb.h"

#include "../virtio/virtio_iommu.h"

#define DTS_VFB_DEV_NAME "hisilicon,dpuvfb"
#define VFB_DEVICE_MAX_COUNT 1
#define VFB_DEVICE_NAME "vfb"
#define VFB_NAME_SIZE 32
#define VFB_ACQUIRE_FENCE_TIMEOUT_MSEC (1440000)
#define VFB_BUF_NUM_DEFAULT 4
#define VFB_BUF_XRES_DEFAULT 1920
#define VFB_BUF_YRES_DEFAULT 720
#define VFB_BUF_NUM_MAX 10

struct device_vfb {
	atomic_t ref_cnt;
	uint32_t index;
	struct mutex lock;
	struct dkmd_chrdev chrdev;
	struct vfb_bufmgr bufmgr;
	bool config_acquired;
	struct product_config config;
	uint32_t xres;
	uint32_t yres;
};

struct vfb_devno_info {
	dev_t devno;
	struct device_vfb vfb;
};

static uint32_t g_vfb_devno_info_index = 0;
static struct vfb_devno_info g_vfb_devno_info[VFB_DEVICE_MAX_COUNT];
static char g_vfb_device_name[VFB_DEVICE_MAX_COUNT][VFB_NAME_SIZE] = {VFB_DEVICE_NAME};

void vfb_fill_product_config(int index, struct vfb_product_config *vfb_config)
{
	int ret;
	dpu_check_and_no_retval(index >= VFB_DEVICE_MAX_COUNT, err, "invalid vfb_index!");
	dpu_check_and_no_retval(!vfb_config, err, "vfb config is null pointer!");

	struct product_config *config = &(g_vfb_devno_info[index].vfb.config);

	config->width = vfb_config->width;
	config->height = vfb_config->height;
	config->drv_feature.bits.drv_framework = DRV_VIRTUAL_FB;
	config->drv_feature.bits.is_pluggable = false;
	config->display_count = vfb_config->display_count;
	config->dim_info_count = vfb_config->dim_info_count;
	config->dim_info[DIMENSION_NORMAL].width = vfb_config->dim_info[DIMENSION_NORMAL].width;
	config->dim_info[DIMENSION_NORMAL].height = vfb_config->dim_info[DIMENSION_NORMAL].height;
	config->dsc_out_width = vfb_config->dsc_out_width;
	config->dsc_out_height = vfb_config->dsc_out_height;
	config->fps_info_count = vfb_config->fps_info_count;
	config->fps_info[FPS_NORMAL] = vfb_config->fps_info[FPS_NORMAL];
	config->color_mode_count = 3;
	config->color_modes[0] = COLOR_MODE_NATIVE;
	config->color_modes[1] = COLOR_MODE_SRGB;
	config->color_modes[2] = COLOR_MODE_DISPLAY_P3;
	dpu_pr_info("width = %d, height = %d", config->width, config->height);
	g_vfb_devno_info[index].vfb.config_acquired = true;
}

void vfb_present_done(int index, struct virtio_fb_present_data *data)
{
	struct vfb_buffer *buffer = NULL;
	struct vfb_fence *present_fence = NULL;

	dpu_check_and_no_retval(!data, err, "data is null pointer!");
	buffer = dpu_vfb_get_buffer_by_data(data);
	dpu_check_and_no_retval(!buffer, err, "buffer is null pointer!");
	present_fence = buffer->present_fence;
	dpu_check_and_no_retval(!present_fence, err, "present_fence is null pointer!");
	dpu_check_and_no_retval((buffer->virtio_data.layer_nums > VIRTIO_FB_PRESENT_LAYER_NUM),
							err, "layer_nums is error!");

	dpu_pr_info("buffer[%u]: frame_no = %d, fence_fd = %d, buf_key = 0x%llx, signal fence and clear buffer",
				buffer->index, buffer->virtio_data.frame_no, present_fence->present_fence_fd,
				buffer->virtio_data.layer_infos[0].buf_key + VFB_BUF_KEY_BUF_OFFSET);
	dpu_vfb_present_fence_signal(present_fence);
	dpu_vfb_buffer_clear(buffer);
	decrease_buf_refcnt(buffer->virtio_data.layer_infos[0].buf_key);
}

static int32_t dpu_vfb_get_product_config(struct device_vfb *vfb, void __user *argp)
{
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");
	dpu_check_and_return(unlikely(!vfb), -EINVAL, err, "vfb is null pointer");
	dpu_check_and_return(unlikely(!vfb->config_acquired), -EINVAL, err, "product config not acquired");

	dpu_pr_info("width is %u, height is %u, drv_framework  is %d, is_pluggable is %d, display_count is %d",
				 vfb->config.width, vfb->config.height, vfb->config.drv_feature.bits.drv_framework,
				 vfb->config.drv_feature.bits.is_pluggable, vfb->config.display_count);
	dpu_pr_info("read from config vfb->xres is %d, vfb->yres is %d", vfb->xres, vfb->yres);
	if (vfb->config.width == 0 || vfb->config.height == 0) {
		vfb->config.width = vfb->xres;
		vfb->config.height = vfb->yres;
		vfb->config.dim_info[DIMENSION_NORMAL].width = (int32_t)vfb->xres;
		vfb->config.dim_info[DIMENSION_NORMAL].height = (int32_t)vfb->yres;
		vfb->config.fps_info_count = 1;
		vfb->config.fps_info[FPS_NORMAL] = 60;
	}
	if (copy_to_user(argp, &vfb->config, sizeof(struct product_config)) != 0) {
		dpu_pr_err("copy product config to user fail");
		return -1;
	}
	return 0;
}

static int32_t dpu_vfb_present_preprocess(struct vfb_buffer *buffer, struct vfb_present_data *vfb_data)
{
	uint32_t i;
	int32_t ret;
	struct dma_buf *buf = NULL;
	struct virtio_fb_present_data *virtio_data = NULL;

	dpu_check_and_return(unlikely(!buffer), -EINVAL, err, "buffer is null pointer");
	dpu_check_and_return(unlikely(!vfb_data), -EINVAL, err, "vfb_data is null pointer");

	virtio_data = &buffer->virtio_data;
	virtio_data->frame_no = vfb_data->frame_no;
	virtio_data->magic_num = vfb_data->magic_num;
	virtio_data->layer_nums = vfb_data->layer_nums;
	for (i = 0; i < vfb_data->layer_nums; ++i) {
		buf = dma_buf_get(vfb_data->layer_infos[i].share_fd);
		if (IS_ERR_OR_NULL(buf)) {
			dpu_pr_err("frame_no[%d] layer[%d]: share_fd%d get dmabuf failed",
						vfb_data->frame_no, i, vfb_data->layer_infos[i].share_fd);
			return -1;
		}
		virtio_data->layer_infos[i].format = vfb_data->layer_infos[i].format;
		virtio_data->layer_infos[i].stride = vfb_data->layer_infos[i].stride;
		virtio_data->layer_infos[i].buf_key = get_bufkey_by_dmabuf(buf);

		dpu_pr_info("bufkey = 0x%llx, map iova++", virtio_data->layer_infos[i].buf_key + VFB_BUF_KEY_BUF_OFFSET);
		ret = dpu_vfb_map_iova(buf, vfb_data->layer_infos[i].share_fd);
		dma_buf_put(buf);
		if (unlikely(ret != 0))
			return ret;
		dpu_pr_info("bufkey = 0x%llx, map iova--", virtio_data->layer_infos[i].buf_key + VFB_BUF_KEY_BUF_OFFSET);

		dpu_pr_info("wait acquire fence %d ++", vfb_data->layer_infos[i].acquired_fence);
		ret = dkmd_acquire_fence_wait_fd(vfb_data->layer_infos[i].acquired_fence, VFB_ACQUIRE_FENCE_TIMEOUT_MSEC);
		if (unlikely(ret != 0)) {
			dpu_pr_err("layer%u wait acquire fence %d fail",
						i, vfb_data->layer_infos[i].acquired_fence);
			return ret;
		}
		dpu_pr_info("wait acquire fence %d --", vfb_data->layer_infos[i].acquired_fence);

		/* fd need be close by HAL */
		vfb_data->layer_infos[i].acquired_fence = -1;

		dpu_pr_info("buffer[%u]: frame_no[%d] layer[%d]: fence_fd = %d,  buf_key = 0x%llx, preprocess success",
					buffer->index, vfb_data->frame_no, i, buffer->present_fence->present_fence_fd,
					virtio_data->layer_infos[i].buf_key + VFB_BUF_KEY_BUF_OFFSET);
	}
	return 0;
}

static int32_t dpu_vfb_present(struct device_vfb *vfb, void __user *argp)
{
	int32_t ret;
	int32_t present_fence_fd;
	struct vfb_present_data vfb_data;
	struct vfb_buffer *buffer = NULL;

	dpu_check_and_return(unlikely(!vfb), -EINVAL, err, "vfb is null pointer");
	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "argp is null pointer");
	if (copy_from_user(&vfb_data, argp, sizeof(struct vfb_present_data)) != 0) {
		dpu_pr_err("copy from user vfb_data fail");
		return -EINVAL;
	}

	dpu_check_and_return(unlikely(vfb_data.magic_num != VIRTIO_FB_MAGIC_NUM), -EINVAL, err, "magic_num is error");
	dpu_check_and_return(unlikely(vfb_data.layer_nums > VFB_PRESENT_LAYER_NUM), -EINVAL, err, "layer_num is error");

	/* 1. Cyclic get avaliable buffer */
	buffer = dpu_vfb_get_avaliable_buffer(&vfb->bufmgr);
	if (unlikely(buffer == NULL)) {
		dpu_pr_warn("get_avaliable_buffer fail, share_fd = %d", vfb_data.layer_infos[0].share_fd);
		return -EINVAL;
	}

	/* 2. Create present fence */
	ret = dpu_vfb_present_fence_create(buffer, vfb_data.frame_no, &present_fence_fd);
	if (unlikely(ret != 0))
		goto buffer_clear;

	/* 3. Preprocess traverse layers to get virtio present data and check whether
	 * dma-buf is mapped, if the address is not mapped, send sglist to backend
	 * by virtio-fb. Ensure that wait acquire fence is the last operation before
	 * send present data to virtio-fb.
	 */
	ret = dpu_vfb_present_preprocess(buffer, &vfb_data);
	if (unlikely(ret != 0))
		goto fence_release;

	vfb_data.out_present_fence = present_fence_fd;
	if (copy_to_user((struct vfb_present_data __user *)argp, &vfb_data, sizeof(struct vfb_present_data))) {
		dpu_pr_err("copy vfb_present_data to user fail");
		ret = -EINVAL;
		goto fence_release;
	}

	/* 4. Send to virtio-fb */
	ret = virtio_fb_present((int32_t)vfb->index, &buffer->virtio_data);
	if (unlikely(ret != 0))
		goto fence_release;

	set_buf_decrease_flag(buffer->virtio_data.layer_infos[0].buf_key);

	return 0;

fence_release:
	dpu_vfb_present_fence_release(buffer->present_fence);
buffer_clear:
	dpu_vfb_buffer_clear(buffer);
	return ret;
}

static long dpu_vfb_ioctl(struct file *filp, uint32_t cmd, unsigned long arg)
{
	int32_t ret;
	struct device_vfb *vfb = NULL;
	void __user *argp = (void __user *)(uintptr_t)arg;

	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null, cmd = 0x%x", cmd);
	dpu_check_and_return(unlikely(!filp->private_data), -EINVAL, err, "private_data is null, cmd = 0x%x", cmd);

	vfb = (struct device_vfb *)filp->private_data;
	dpu_check_and_return(unlikely(!vfb), -EINVAL, err, "vfb is null, cmd = 0x%x", cmd);

	mutex_lock(&vfb->lock);
	dpu_pr_info("vfb ioctl, cmd=%#x", cmd);
	switch (cmd) {
	case DISP_VIRTUAL_FB_PRESENT:
		ret = dpu_vfb_present(vfb, argp);
		break;
	case DISP_GET_PRODUCT_CONFIG:
		ret = dpu_vfb_get_product_config(vfb, argp);
		break;
	case FBIOBLANK:
		ret = 0;
		break;
	default:
		dpu_pr_err("vfb ioctl fail unsupport cmd, cmd=%#x", cmd);
		ret = -1;
		break;
	}

	mutex_unlock(&vfb->lock);
	return ret;
}

static int32_t dpu_vfb_open(struct inode *inode, struct file *filp)
{
	uint32_t i;
	struct device_vfb *vfb = NULL;

	dpu_check_and_return(unlikely(!inode), -EINVAL, err, "inode is null");
	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");

	if (!filp->private_data) {
		for (i = 0; i < VFB_DEVICE_MAX_COUNT; ++i) {
			if (inode->i_rdev == g_vfb_devno_info[i].devno) {
				filp->private_data = &g_vfb_devno_info[i].vfb;
				break;
			}
		}
	}

	vfb = (struct device_vfb *)filp->private_data;
	if (vfb != NULL) {
		atomic_inc(&vfb->ref_cnt);
		dpu_pr_info("vfb%d: ref_cnt = %d", vfb->index, atomic_read(&vfb->ref_cnt));
	} else {
		dpu_pr_err("vfb open fail, vfb is null");
		return -EINVAL;
	}

	return 0;
}

static int32_t dpu_vfb_release(struct inode *inode, struct file *filp)
{
	struct device_vfb *vfb = NULL;

	dpu_check_and_return(unlikely(!inode), -EINVAL, err, "inode is null");
	dpu_check_and_return(unlikely(!filp), -EINVAL, err, "filp is null");
	dpu_check_and_return(unlikely(!filp->private_data), -EINVAL, err, "private_data is null");

	vfb = (struct device_vfb *)filp->private_data;

	if (unlikely(atomic_read(&vfb->ref_cnt) == 0)) {
		dpu_pr_warn("vfb is not opened, cannot release");
		return 0;
	}

	if (!atomic_sub_and_test(1, &vfb->ref_cnt)) {
		dpu_pr_info("vfb not need release, ref_cnt = %u", atomic_read(&vfb->ref_cnt));
		return 0;
	}

	dpu_pr_info("vfb%d: ref_cnt = %d", vfb->index, atomic_read(&vfb->ref_cnt));
	return 0;
}

static struct file_operations dpu_vfb_fops = {
	.owner = THIS_MODULE,
	.open = dpu_vfb_open,
	.release = dpu_vfb_release,
	.unlocked_ioctl = dpu_vfb_ioctl,
	.compat_ioctl =  dpu_vfb_ioctl,
	.mmap = NULL,
};

static int32_t vfb_device_register(struct device_vfb *vfb)
{
	dpu_pr_info("vfb register +");
	dpu_check_and_return(unlikely(g_vfb_devno_info_index >= VFB_DEVICE_MAX_COUNT), -EINVAL, err, "vfb index invalid");

	vfb->index = g_vfb_devno_info_index;
	vfb->chrdev.name = g_vfb_device_name[vfb->index];
	vfb->chrdev.fops = &dpu_vfb_fops;
	vfb->chrdev.chr_class = NULL;
	vfb->chrdev.drv_data = vfb;
	mutex_init(&vfb->lock);
	if (unlikely(dkmd_create_chrdev(&vfb->chrdev) != 0)) {
		dpu_pr_err("create chr device failed");
		return -EINVAL;
	}
	dpu_pr_info("create device: %s, devno = %u", vfb->chrdev.name, vfb->chrdev.devno);

	g_vfb_devno_info[g_vfb_devno_info_index].devno = vfb->chrdev.devno;
	g_vfb_devno_info_index++;

	dpu_pr_info("vfb register -");
	return 0;
}

static void vfb_device_unregister(struct device_vfb *vfb)
{
	g_vfb_devno_info_index = (g_vfb_devno_info_index > 0) ?  (g_vfb_devno_info_index - 1) : 0;
	dkmd_destroy_chrdev(&vfb->chrdev);
}

static void vfbdev_init_by_dts(struct device_vfb *vfb)
{
	int ret;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, DTS_VFB_DEV_NAME);
	if (!np) {
		dpu_pr_err("not found device node %s, use default buffer num", DTS_VFB_DEV_NAME);
		goto default_num;
	}

	ret = of_property_read_u32(np, "buffer_num", &vfb->bufmgr.buffer_num);
	if ((ret != 0) || (vfb->bufmgr.buffer_num > VFB_BUF_NUM_MAX)) {
		dpu_pr_err("failed to get buffer_num resource, use default buffer num");
		goto default_num;
	}

	ret = of_property_read_u32(np, "xres", &vfb->xres);
	if (ret != 0) {
		dpu_pr_err("failed to get xres resource, use default xres");
		vfb->xres = VFB_BUF_XRES_DEFAULT;
	}

	ret = of_property_read_u32(np, "yres", &vfb->yres);
	if (ret != 0) {
		dpu_pr_err("failed to get yres resource, use default yres");
		vfb->yres = VFB_BUF_YRES_DEFAULT;
	}

	return;

default_num:
	vfb->bufmgr.buffer_num = VFB_BUF_NUM_DEFAULT;
}

static int32_t vfbdev_probe(struct platform_device *pdev)
{
	int32_t ret;
	struct device_vfb *vfb = NULL;

	if (pdev == NULL) {
		dpu_pr_err("pdev is NULL");
		return -EINVAL;
	}

	dpu_pr_info("vfb probe +");

	dpu_check_and_return(unlikely(g_vfb_devno_info_index >= VFB_DEVICE_MAX_COUNT), -EINVAL, err, "vfb index invalid");
	vfb = &g_vfb_devno_info[g_vfb_devno_info_index].vfb;

	/* 1. Create chrdev and register */
	ret = vfb_device_register(vfb);
	if (ret != 0) {
		dpu_pr_err("vfb device register fail!");
		return ret;
	}

	/* 2. Init vfb device */
	vfbdev_init_by_dts(vfb);

	/* 3. Init bufmgr */
	ret = dpu_vfb_bufmgr_init(&vfb->bufmgr);
	if (ret != 0) {
		vfb_device_unregister(vfb);
		dpu_pr_err("vfb bufmgr init fail!");
		return ret;
	}

	platform_set_drvdata(pdev, vfb);

	dpu_pr_info("vfb probe -");
	return 0;
}

static int32_t vfbdev_remove(struct platform_device *pdev)
{
	struct device_vfb *vfb = NULL;

	vfb = (struct device_vfb *)platform_get_drvdata(pdev);
	if (!vfb) {
		dpu_pr_err("vfb is null!");
		return -EINVAL;
	}
	dpu_vfb_bufmgr_release(&vfb->bufmgr);
	vfb_device_unregister(vfb);

	return 0;
}

static const struct of_device_id vfb_dev_match_table[] = {
	{
		.compatible = DTS_VFB_DEV_NAME,
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, vfb_dev_match_table);

static struct platform_driver this_driver = {
	.probe = vfbdev_probe,
	.remove = vfbdev_remove,
	.suspend = NULL,
	.resume = NULL,
	.driver = {
		.name = "vfbdev",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(vfb_dev_match_table),
	},
};

static int __init vfbdev_driver_init(void)
{
	int ret;

	dpu_pr_info("platform_driver_register");
	ret = platform_driver_register(&this_driver);
	if (ret != 0) {
		dpu_pr_err("platform_driver_register failed, error=%d!", ret);
		return ret;
	}
	return ret;
}

static void __exit vfbdev_driver_exit(void)
{
	platform_driver_unregister(&this_driver);
}

module_init(vfbdev_driver_init);
module_exit(vfbdev_driver_exit);

MODULE_DESCRIPTION("Virtual Frame Buffer Driver");
MODULE_LICENSE("GPL");
