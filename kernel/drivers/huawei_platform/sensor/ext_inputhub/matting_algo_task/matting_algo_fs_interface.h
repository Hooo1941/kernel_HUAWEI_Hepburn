/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023.
 * All rights reserved.
 * Description: Noise Calculation and Denoising Algorithm Task
 * Author: yangxiaopeng@huawei.com
 * Create: 2023-2-22
 */
#ifndef MATTING_ALGO_FS_INTERFACE_H
#define MATTING_ALGO_FS_INTERFACE_H

#include <linux/device.h>

ssize_t dev_reg_matting_algo_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t dev_reg_matting_algo_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t dev_reg_matting_algo_crop_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t dev_reg_matting_algo_crop_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t dev_reg_matting_algo_lux_value_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t dev_reg_matting_algo_lux_value_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t dev_reg_matting_algo_als_param_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t dev_reg_matting_algo_als_param_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
void matting_algo_set_lux_value(uint32_t lux_value);
ssize_t interface_matting_algo_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t interface_matting_algo_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t interface_matting_algo_crop_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t interface_matting_algo_crop_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t interface_matting_algo_lux_value_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t interface_matting_algo_lux_value_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t interface_matting_algo_als_param_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t interface_matting_algo_als_param_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
#endif
