/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: cam intf header file.
 * Create: 2016-03-05
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HW_ALAN_KERNEL_CAMERA_OBJ_MDL_H__
#define __HW_ALAN_KERNEL_CAMERA_OBJ_MDL_H__

#include <linux/dma-buf.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <platform_include/camera/native/camera.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#define cam_cfg_err(fmt, arg...) \
	printk(KERN_ERR "%s[%d, %d]: " fmt "\n", \
		__func__, __LINE__, current->pid, ## arg)

#define cam_cfg_info(fmt, arg...) \
	printk(KERN_INFO "%s[%d, %d]: " fmt "\n", \
		__func__, __LINE__, current->pid, ## arg)

#define cam_cfg_debug(fmt, arg...) \
	printk(KERN_DEBUG "%s[%d, %d]: " fmt "\n", \
		__func__, __LINE__, current->pid, ## arg)

/* cam_dev interface definition begin */
int cam_dev_create(struct device *dev, int *dev_num);
/* cam_dev interface definition end */
char *gen_media_prefix(char *media_ent,
		       cam_device_id_constants_t dev_const, size_t dst_size);

/* cam_cfgdev interface definition begin */
int cam_cfgdev_register_subdev(struct v4l2_subdev *subdev,
			       cam_device_id_constants_t dev_const);
int cam_cfgdev_unregister_subdev(struct v4l2_subdev *subdev);
/* cam_cfgdev interface definition end */

int init_subdev_media_entity(struct v4l2_subdev *subdev,
			     cam_device_id_constants_t dev_const);

#endif /* __HW_ALAN_KERNEL_CAMERA_OBJ_MDL_H__ */
