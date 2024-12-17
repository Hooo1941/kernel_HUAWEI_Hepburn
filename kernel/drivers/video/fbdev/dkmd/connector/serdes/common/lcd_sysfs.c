/* Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/kernel.h>

#include "log.h"
#include "lcd_sysfs.h"

static struct lcd_sysfs_ops *g_lcd_sysfs_ops[LCD_MAX];
static struct lcd_sysfs_lock *g_sysfs_lock[LCD_MAX];

static ssize_t lcd_ser_pattern_port0_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->ser_pattern_lock));
	if (sysfs_ops->ser_pattern_port0_show)
		ret = sysfs_ops->ser_pattern_port0_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->ser_pattern_lock));
	return ret;
}

static ssize_t lcd_deser_pattern_port0_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->deser_pattern_lock));
	if (sysfs_ops->deser_pattern_port0_show)
		ret = sysfs_ops->deser_pattern_port0_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->deser_pattern_lock));
	return (ssize_t)ret;
}

static ssize_t lcd_panel_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->panel_info_lock));
	if (sysfs_ops->panel_info_show)
		ret = sysfs_ops->panel_info_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->panel_info_lock));
	return ret;
}

static ssize_t lcd_ser_main_regs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->ser_main_regs_lock));
	if (sysfs_ops->ser_main_regs_show)
		ret = sysfs_ops->ser_main_regs_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->ser_main_regs_lock));
	return ret;
}

static ssize_t lcd_ser_main_regs_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->ser_main_regs_lock));
	if (sysfs_ops->ser_main_regs_store)
		ret = sysfs_ops->ser_main_regs_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->ser_main_regs_lock));
	return (ssize_t)ret;
}

static ssize_t lcd_deser_main_regs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->deser_main_regs_lock));
	if (sysfs_ops->deser_main_regs_show)
		ret = sysfs_ops->deser_main_regs_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->deser_main_regs_lock));
	return ret;
}

static ssize_t lcd_deser_main_regs_store(
	struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->deser_main_regs_lock));
	if (sysfs_ops->deser_main_regs_store)
		ret = sysfs_ops->deser_main_regs_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->deser_main_regs_lock));
	return (ssize_t)ret;
}

static ssize_t lcd_ser_panel_on_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->ser_panel_on_lock));
	if (sysfs_ops->ser_panel_on_store)
		ret = sysfs_ops->ser_panel_on_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->ser_panel_on_lock));
	return ret;
}

static ssize_t lcd_ser_panel_off_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->ser_panel_off_lock));
	if (sysfs_ops->ser_panel_off_store)
		ret = sysfs_ops->ser_panel_off_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->ser_panel_off_lock));
	return (ssize_t)ret;
}

static ssize_t lcd_ser_brightness_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->ser_brightness_lock));
	if (sysfs_ops->ser_brightness_show)
		ret = sysfs_ops->ser_brightness_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->ser_brightness_lock));
	return ret;
}

static ssize_t lcd_ser_brightness_store(
	struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->ser_brightness_lock));
	if (sysfs_ops->ser_brightness_store)
		ret = sysfs_ops->ser_brightness_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->ser_brightness_lock));
	return (ssize_t)ret;
}

static ssize_t lcd_ota_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->ser_brightness_lock));
	if (sysfs_ops->ser_brightness_show)
		ret = sysfs_ops->ser_brightness_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->ser_brightness_lock));
	return ret;
}

static ssize_t lcd_light_sensor_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->light_sensor_lock));
	if (sysfs_ops->light_sensor_show)
		ret = sysfs_ops->light_sensor_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->light_sensor_lock));
	return ret;
}

static ssize_t lcd_light_sensor_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->light_sensor_lock));
	if (sysfs_ops->light_sensor_store)
		ret = sysfs_ops->light_sensor_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->light_sensor_lock));

	return ret;
}

static ssize_t lcd_sn_code_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->lcd_sn_lock));
	if (sysfs_ops->lcd_sn_code_show)
		ret = sysfs_ops->lcd_sn_code_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->lcd_sn_lock));
	return ret;
}

static ssize_t lcd_sn_code_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->lcd_sn_lock));
	if (sysfs_ops->lcd_sn_code_store)
		ret = sysfs_ops->lcd_sn_code_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->lcd_sn_lock));

	return (ssize_t)ret;
}

static ssize_t lcd_factory_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->lcd_factory_lock));
	if (sysfs_ops->lcd_factory_show)
		ret = sysfs_ops->lcd_factory_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->lcd_factory_lock));

	return ret;
}

static ssize_t lcd_factory_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->lcd_factory_lock));
	if (sysfs_ops->lcd_factory_store)
		ret = sysfs_ops->lcd_factory_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->lcd_factory_lock));

	return (ssize_t)ret;
}

static ssize_t lcd_fail_det_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->lcd_fail_det_lock));
	if (sysfs_ops->lcd_fail_det_show)
		ret = sysfs_ops->lcd_fail_det_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->lcd_fail_det_lock));

	return ret;
}

static ssize_t lcd_default_test_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->lcd_default_test_lock));
	if (sysfs_ops->lcd_default_test_show)
		ret = sysfs_ops->lcd_default_test_show(dev, attr, buf);
	mutex_unlock(&(sysfs_lock->lcd_default_test_lock));

	return ret;
}

static ssize_t lcd_default_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = LCD_OK;
	struct lcd_sysfs_ops *sysfs_ops = NULL;
	struct lcd_sysfs_lock *sysfs_lock = NULL;

	sysfs_ops = lcd_get_sysfs_ops(dev);
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is NULL!\n");
		return LCD_FAIL;
	}

	sysfs_lock = lcd_get_sysfs_lock(dev);
	if (!sysfs_lock) {
		sysfs_err("sysfs_lock is NULL!\n");
		return LCD_FAIL;
	}

	mutex_lock(&(sysfs_lock->lcd_default_test_lock));
	if (sysfs_ops->lcd_default_test_store)
		ret = sysfs_ops->lcd_default_test_store(dev, attr, buf, count);
	mutex_unlock(&(sysfs_lock->lcd_default_test_lock));

	return count;
}

static DEVICE_ATTR(ser_pattern_port0, 0640, lcd_ser_pattern_port0_show, NULL);
static DEVICE_ATTR(deser_pattern_port0, 0640, lcd_deser_pattern_port0_show, NULL);
static DEVICE_ATTR(panel_info_show, 0640, lcd_panel_info_show, NULL);
static DEVICE_ATTR(ser_main_reg, 0640, lcd_ser_main_regs_show, lcd_ser_main_regs_store);
static DEVICE_ATTR(deser_main_reg, 0640, lcd_deser_main_regs_show, lcd_deser_main_regs_store);
static DEVICE_ATTR(ser_panel_on, 0640, NULL, lcd_ser_panel_on_store);
static DEVICE_ATTR(ser_panel_off, 0640, NULL, lcd_ser_panel_off_store);
static DEVICE_ATTR(ser_brightness, 0640, lcd_ser_brightness_show, lcd_ser_brightness_store);
static DEVICE_ATTR(ota_version, 0640, lcd_ota_version_show, NULL);
static DEVICE_ATTR(light_sensor, 0640, lcd_light_sensor_show, lcd_light_sensor_store);
static DEVICE_ATTR(lcd_sn_code, 0640, lcd_sn_code_show, lcd_sn_code_store);
static DEVICE_ATTR(lcd_factory_ops, 0640, lcd_factory_show, lcd_factory_store);
static DEVICE_ATTR(lcd_fail_det, 0640, lcd_fail_det_show, NULL);
static DEVICE_ATTR(lcd_default_test, 0640, lcd_default_test_show, lcd_default_test_store);

static struct attribute *lcd_sysfs_attrs_dsi[LCD_SYSFS_MAX] = {NULL};

static struct attribute *lcd_sysfs_attrs_dp[LCD_SYSFS_MAX] = {NULL};

static struct attribute *lcd_conf[] = {
	&dev_attr_ser_pattern_port0.attr,
	&dev_attr_deser_pattern_port0.attr,
	&dev_attr_panel_info_show.attr,
	&dev_attr_ser_main_reg.attr,
	&dev_attr_deser_main_reg.attr,
	&dev_attr_ser_panel_on.attr,
	&dev_attr_ser_panel_off.attr,
	&dev_attr_ser_brightness.attr,
	&dev_attr_ota_version.attr,
	&dev_attr_light_sensor.attr,
	&dev_attr_lcd_sn_code.attr,
	&dev_attr_lcd_factory_ops.attr,
	&dev_attr_lcd_fail_det.attr,
	&dev_attr_lcd_default_test.attr,
	NULL,
};

struct attribute_group lcd_sysfs_attr_group_dsi = {
	.attrs = lcd_sysfs_attrs_dsi,
};

struct attribute_group lcd_sysfs_attr_group_dp = {
	.attrs = lcd_sysfs_attrs_dp,
};

void lcd_lock_init(struct lcd_sysfs_lock *tmp_lock, int index)
{
	if (index >= LCD_MAX) {
		sysfs_err("lcd sysfs lock index is %d.\n", index);
		return;
	}
	g_sysfs_lock[index] = tmp_lock;

	mutex_init(&(g_sysfs_lock[index]->ser_pattern_lock));
	mutex_init(&(g_sysfs_lock[index]->deser_pattern_lock));
	mutex_init(&(g_sysfs_lock[index]->panel_info_lock));
	mutex_init(&(g_sysfs_lock[index]->ser_main_regs_lock));
	mutex_init(&(g_sysfs_lock[index]->deser_main_regs_lock));
	mutex_init(&(g_sysfs_lock[index]->ser_panel_on_lock));
	mutex_init(&(g_sysfs_lock[index]->ser_panel_off_lock));
	mutex_init(&(g_sysfs_lock[index]->ser_brightness_lock));
	mutex_init(&(g_sysfs_lock[index]->lcd_ota_version_lock));
	mutex_init(&(g_sysfs_lock[index]->light_sensor_lock));
	mutex_init(&(g_sysfs_lock[index]->lcd_factory_lock));
	mutex_init(&(g_sysfs_lock[index]->lcd_sn_lock));
	mutex_init(&(g_sysfs_lock[index]->lcd_default_test_lock));
}

int lcd_sysfs_ops_register(struct lcd_sysfs_ops *ops, int index)
{
	if (index >= LCD_MAX) {
		sysfs_err("lcd sysfs index is %d.\n", index);
		return LCD_FAIL;
	}

	if (g_lcd_sysfs_ops[index]) {
		sysfs_err("has already registered, i is %d.\n", index);
		return LCD_OK;
	}

	g_lcd_sysfs_ops[index] = ops;
	sysfs_info("register success is %d.\n", index);

	return LCD_OK;
}

int lcd_sysfs_ops_unregister(struct lcd_sysfs_ops *ops)
{
	int i = 0;
	for (i = 0; i < LCD_MAX; i++) {
		if (g_lcd_sysfs_ops[i] == ops) {
			g_lcd_sysfs_ops[i] = NULL;
			sysfs_info("lcd sysfs ops unregister success, i is %d.\n", i);
		}
	}

	return LCD_OK;
}

struct lcd_sysfs_lock *lcd_get_sysfs_lock(struct device *dev)
{
	int index = LCD_DSI_INDEX;
	struct i2c_client *ti981_client = get_ti981_client();
	if (!&(ti981_client->dev))
		return -1;

	if (&(ti981_client->dev) == dev) {
		index = LCD_DSI_INDEX;
		sysfs_info("lcd_get_sysfs_lock index equal to  LCD_DP_INDEX.\n");
	}

	sysfs_info("lcd_get_sysfs_lock index is %d.\n", index);
	return g_sysfs_lock[index];
}

struct lcd_sysfs_ops *lcd_get_sysfs_ops(struct device *dev)
{
	int index = LCD_DSI_INDEX;
	struct i2c_client *ti981_client = get_ti981_client();
	if (&(ti981_client->dev) == dev) {
		index = LCD_DSI_INDEX;
		sysfs_info("lcd_get_sysfs_ops index equal to  LCD_DSI_INDEX.\n");
	}
	sysfs_info("lcd_get_sysfs_ops index is %d.\n", index);

	return g_lcd_sysfs_ops[index];
}

static int lcd_check_support(struct i2c_client *client, int index)
{
	struct lcd_sysfs_ops *sysfs_ops = NULL;

	sysfs_ops = lcd_get_sysfs_ops(&(client->dev));
	if (!sysfs_ops) {
		sysfs_err("sysfs_ops is null\n");
		return LCD_OK;
	}
	if (sysfs_ops->check_support)
		return sysfs_ops->check_support(index);

	sysfs_info("not register config function\n");
	return LCD_OK;
}

int lcd_create_sysfs(struct i2c_client *client, int index)
{
	int rc;
	int i;
	int num = 0;
	for (i = 0; i < ARRAY_SIZE(lcd_conf); i++) {
		if (i >= LCD_SYSFS_MAX - 1) {
			sysfs_err("dev attr number exceed sysfs max\n");
			return LCD_FAIL;
		}

		rc = lcd_check_support(client, i);
		if (rc && num < LCD_SYSFS_MAX && (index == LCD_DSI_INDEX))
			lcd_sysfs_attrs_dsi[num++] = lcd_conf[i];

		if (rc && num < LCD_SYSFS_MAX && (index == LCD_DP_INDEX))
			lcd_sysfs_attrs_dp[num++] = lcd_conf[i];
	}

	if (index == LCD_DSI_INDEX) {
		rc = sysfs_create_group(&(client->dev.kobj), &lcd_sysfs_attr_group_dsi);
		if (rc)
			sysfs_err("lcd_sysfs_attr_group_dsi sysfs group creation failed, rc=%d\n", rc);

		sysfs_info("sysfs_create_group create dsi sucess\n");
	}

	if (index == LCD_DP_INDEX) {
		rc = sysfs_create_group(&(client->dev.kobj), &lcd_sysfs_attr_group_dp);
		if (rc)
			sysfs_err("lcd_sysfs_attr_group_dp sysfs group creation failed, rc=%d\n", rc);

		sysfs_info("sysfs_create_group create dp sucess\n");
	}

	return rc;
}
