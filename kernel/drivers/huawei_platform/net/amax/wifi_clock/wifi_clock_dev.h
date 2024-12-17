/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: This module is to support wifi multilink ioctl.
 * Create: 2023-03-31
 */
#ifndef AMAX_MULTILINK_DEV_H_
#define AMAX_MULTILINK_DEV_H_

#include <linux/cdev.h>
#include <linux/device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <securec.h>

#define DTS_NODE_HISI_MPXX            "hisilicon,hi110x"
#define DTS_PROP_ENABLE_SQUARE_WAVE   "hi110x,enable_square_wave"

typedef struct {
	struct cdev cdev;
	struct class *class;
	struct device *dev;
	dev_t devid;
	int major;
} wifi_clock_device;

#define HWLOG_TAG wifi_clock
HWLOG_REGIST();

#endif