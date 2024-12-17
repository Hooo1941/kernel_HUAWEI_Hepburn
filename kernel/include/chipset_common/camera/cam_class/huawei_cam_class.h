/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: camera test class interface
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

#ifndef _HUAWEI_CAM_CLASS_H_
#define _HUAWEI_CAM_CLASS_H_

#include <linux/platform_device.h>

struct huawei_cam_class_driver {
	int (*probe)(struct platform_device *pdev, struct class *huawei_cam_class);
	int (*remove)(struct class *huawei_cam_class);
};

int huawei_cam_class_driver_register(
	const struct huawei_cam_class_driver *cam_class_driver);

int __init camera_class_init(void);
void __exit camera_class_exit(void);
#endif /* _CAM_TEST_CLASS_H_ */