// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023.
 * All rights reserved.
 * Description: Noise Calculation and Denoising Algorithm Task
 * Author: yangxiaopeng@huawei.com
 * Create: 2023-2-22
 */
#include <linux/kernel.h>
#include "lcd_kit_drm_panel.h"
#include "lcd_kit_hybrid_core.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_utils.h"
#include <huawei_platform/log/hw_log.h>
#include "page.h"
#include "securec.h"
#include "als_matting_algo.h"
#include "matting_algo_task.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG matting_algo_task
HWLOG_REGIST();

ssize_t dev_reg_matting_algo_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = EC_SUCCESS;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		hwlog_err("[%s] sysfs_ops is NULL!\n", __func__);
		return EC_FAILURE;
	}
	if (sysfs_ops->matting_algo_debug_show)
		ret = sysfs_ops->matting_algo_debug_show(dev, attr, buf);
	return ret;
}

ssize_t dev_reg_matting_algo_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		hwlog_err("[%s] sysfs_ops is NULL!\n", __func__);
		return EC_FAILURE;
	}

	if (!sysfs_ops->matting_algo_debug_store) {
		hwlog_err("[%s] matting_algo_debug_store is NULL!\n", __func__);
		return EC_FAILURE;
	}

	if (sysfs_ops->matting_algo_debug_store(dev, attr, buf, count)) {
		hwlog_err("[%s] matting_algo_debug_store error!\n", __func__);
		return EC_FAILURE;
	}

	return count;
}

ssize_t dev_reg_matting_algo_crop_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = EC_SUCCESS;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		hwlog_err("[%s] sysfs_ops is NULL!\n", __func__);
		return EC_FAILURE;
	}
	if (sysfs_ops->matting_algo_crop_debug_show)
		ret = sysfs_ops->matting_algo_crop_debug_show(dev, attr, buf);
	return ret;
}

ssize_t dev_reg_matting_algo_crop_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		hwlog_err("[%s] sysfs_ops is NULL!\n", __func__);
		return EC_FAILURE;
	}

	if (!sysfs_ops->matting_algo_crop_debug_store) {
		hwlog_err("[%s] matting_algo_crop_debug_store is NULL!\n", __func__);
		return EC_FAILURE;
	}

	if (sysfs_ops->matting_algo_crop_debug_store(dev, attr, buf, count)) {
		hwlog_err("[%s] matting_algo_crop_debug_store error!\n", __func__);
		return EC_FAILURE;
	}

	return count;
}

ssize_t dev_reg_matting_algo_lux_value_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = EC_SUCCESS;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		hwlog_err("[%s] sysfs_ops is NULL!\n", __func__);
		return EC_FAILURE;
	}
	if (sysfs_ops->matting_algo_lux_value_show)
		ret = sysfs_ops->matting_algo_lux_value_show(dev, attr, buf);
	return ret;
}

ssize_t dev_reg_matting_algo_lux_value_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		hwlog_err("[%s] sysfs_ops is NULL!\n", __func__);
		return EC_FAILURE;
	}

	if (!sysfs_ops->matting_algo_lux_value_store) {
		hwlog_err("[%s] matting_algo_lux_value_store is NULL!\n", __func__);
		return EC_FAILURE;
	}

	if (sysfs_ops->matting_algo_lux_value_store(dev, attr, buf, count)) {
		hwlog_err("[%s] matting_algo_lux_value_store return error!\n", __func__);
		return EC_FAILURE;
	}

	return count;
}

ssize_t dev_reg_matting_algo_als_param_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = EC_SUCCESS;
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		hwlog_err("[%s] sysfs_ops is NULL!\n", __func__);
		return EC_FAILURE;
	}
	if (sysfs_ops->matting_algo_als_param_show)
		ret = sysfs_ops->matting_algo_als_param_show(dev, attr, buf);
	return ret;
}

ssize_t dev_reg_matting_algo_als_param_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct lcd_kit_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_kit_get_sysfs_ops();
	if (!sysfs_ops) {
		hwlog_err("[%s] sysfs_ops is NULL!\n", __func__);
		return EC_FAILURE;
	}

	if (!sysfs_ops->matting_algo_als_param_store) {
		hwlog_err("[%s] matting_algo_als_param_store is NULL!\n", __func__);
		return EC_FAILURE;
	}

	if (sysfs_ops->matting_algo_als_param_store(dev, attr, buf, count)) {
		hwlog_err("[%s] matting_algo_als_param_store return error!\n", __func__);
		return EC_FAILURE;
	}

	return count;
}

ssize_t interface_matting_algo_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = EC_SUCCESS;
	struct qcom_panel_info *pinfo = NULL;
	uint32_t panel_id = lcd_get_active_panel_id();

	if (!buf || !attr) {
		hwlog_err("[%s] buf is NULL\n", __func__);
		return EC_FAILURE;
	}
	pinfo = lcm_get_panel_info(panel_id);
	if (!pinfo) {
		hwlog_err("[%s] pinfo is null\n", __func__);
		return EC_FAILURE;
	}

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n",
		pinfo->matting_algo_debug == 0 ? "0: disable" : "1: enable");
	return ret;
}

ssize_t interface_matting_algo_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = EC_SUCCESS;
	unsigned long mode = 0;
	struct qcom_panel_info *panel_info = NULL;
	uint32_t panel_id = lcd_get_active_panel_id();

	panel_info = lcm_get_panel_info(panel_id);
	if (panel_info == NULL) {
		hwlog_err("[%s] panel_info is null\n", __func__);
		return EC_FAILURE;
	}

	ret = kstrtoul(buf, 0, &mode);
	if (ret) {
		hwlog_err("[%s] invalid data!\n", __func__);
		return ret;
	}

	panel_info->matting_algo_debug = 0;
	if (mode != 0)
		panel_info->matting_algo_debug = 1;

	return ret;
}

ssize_t interface_matting_algo_crop_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = EC_SUCCESS;
	struct qcom_panel_info *pinfo = NULL;
	uint32_t panel_id = lcd_get_active_panel_id();

	if (!buf || !attr) {
		hwlog_err("[%s] buf is NULL\n", __func__);
		return EC_FAILURE;
	}
	pinfo = lcm_get_panel_info(panel_id);
	if (!pinfo) {
		hwlog_err("[%s] pinfo is null\n", __func__);
		return EC_FAILURE;
	}

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n",
		pinfo->matting_algo_crop_debug == 0 ? "0: disable" : "1: enable");
	return ret;
}

ssize_t interface_matting_algo_crop_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = EC_SUCCESS;
	unsigned long mode = 0;
	struct qcom_panel_info *panel_info = NULL;
	uint32_t panel_id = lcd_get_active_panel_id();

	panel_info = lcm_get_panel_info(panel_id);
	if (panel_info == NULL) {
		hwlog_err("[%s] panel_info is NULL\n", __func__);
		return EC_FAILURE;
	}

	ret = kstrtoul(buf, 0, &mode);
	if (ret) {
		hwlog_err("[%s] invalid data!\n", __func__);
		return ret;
	}

	panel_info->matting_algo_crop_debug = 0;
	if (mode != 0)
		panel_info->matting_algo_crop_debug = 1;

	return ret;
}

void matting_algo_set_lux_value(uint32_t lux_value)
{
	struct qcom_panel_info *pinfo = lcm_get_panel_info(lcd_get_active_panel_id());

	if (!pinfo) {
		hwlog_err("[%s] pinfo is null\n", __func__);
		return;
	}

	mutex_lock(&matting_algo_get_lock_st()->matting_algo_lux_value_lock);
	if (pinfo != NULL)
		pinfo->matting_algo_lux_value = lux_value;
	mutex_unlock(&matting_algo_get_lock_st()->matting_algo_lux_value_lock);
}

ssize_t interface_matting_algo_lux_value_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = EC_SUCCESS;
	struct qcom_panel_info *pinfo = NULL;
	uint32_t panel_id = lcd_get_active_panel_id();

	if (!buf || !attr) {
		hwlog_err("[%s] buf is NULL\n", __func__);
		return EC_FAILURE;
	}
	pinfo = lcm_get_panel_info(panel_id);
	if (!pinfo) {
		hwlog_err("[%s] pinfo is null\n", __func__);
		return EC_FAILURE;
	}

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n", pinfo->matting_algo_lux_value);
	// after read, change matting_algo_lux_value to invalid
	matting_algo_set_lux_value(UINT_MAX);
	return ret;
}

ssize_t interface_matting_algo_lux_value_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = EC_SUCCESS;
	unsigned long mode = 0;
	struct qcom_panel_info *panel_info = NULL;
	uint32_t panel_id = lcd_get_active_panel_id();

	panel_info = lcm_get_panel_info(panel_id);
	if (panel_info == NULL) {
		hwlog_err("[%s] panel_info is NULL\n", __func__);
		return EC_FAILURE;
	}

	ret = kstrtoul(buf, 0, &mode);
	if (ret) {
		hwlog_err("[%s] invalid data!\n", __func__);
		return ret;
	}

	matting_algo_set_lux_value(mode);
	return ret;
}

ssize_t interface_matting_algo_als_param_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = EC_SUCCESS;
	struct qcom_panel_info *pinfo = NULL;
	uint32_t panel_id = lcd_get_active_panel_id();
	char str[LCD_REG_LENGTH_MAX] = {0};

	if (!buf || !attr) {
		hwlog_err("[%s] buf is NULL\n", __func__);
		return EC_FAILURE;
	}
	pinfo = lcm_get_panel_info(panel_id);
	if (!pinfo) {
		hwlog_err("[%s] pinfo is null\n", __func__);
		return EC_FAILURE;
	}

	ret = snprintf_s(str, sizeof(str), sizeof(str) - 1,
		"alsLux: %d, alsChannel: %d %d %d %d %d, alsAgain: %d %d %d %d %d, alsAtime: %d",
		pinfo->matting_algo_als_param.als_lux,
		pinfo->matting_algo_als_param.als_channel[INDEX_ZERO],
		pinfo->matting_algo_als_param.als_channel[INDEX_ONE],
		pinfo->matting_algo_als_param.als_channel[INDEX_TWO],
		pinfo->matting_algo_als_param.als_channel[INDEX_THREE],
		pinfo->matting_algo_als_param.als_channel[INDEX_FOUR],
		pinfo->matting_algo_als_param.als_again[INDEX_ZERO],
		pinfo->matting_algo_als_param.als_again[INDEX_ONE],
		pinfo->matting_algo_als_param.als_again[INDEX_TWO],
		pinfo->matting_algo_als_param.als_again[INDEX_THREE],
		pinfo->matting_algo_als_param.als_again[INDEX_FOUR],
		pinfo->matting_algo_als_param.als_atime);

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", str);
	if (ret < 0) {
		hwlog_err("[%s] snprintf fail\n", __func__);
		return EC_FAILURE;
	}

	return ret;
}

ssize_t interface_matting_algo_als_param_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct qcom_panel_info *panel_info = NULL;
	uint32_t panel_id = lcd_get_active_panel_id();

	panel_info = lcm_get_panel_info(panel_id);
	if (panel_info == NULL) {
		hwlog_err("[%s] panel_info is NULL\n", __func__);
		return EC_FAILURE;
	}

	panel_info->matting_algo_als_param = *((struct sensor_hub_als_data_st *)buf);

	up(matting_algo_get_semaphore_als_report());

	return 0;
}
