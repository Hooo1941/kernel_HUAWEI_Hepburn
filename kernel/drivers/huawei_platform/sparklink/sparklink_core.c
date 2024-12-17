/*
 * Huawei sparklink core code
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
#include <securec.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/of.h>
#include "sparklink.h"

#define POWR_MAX_LEN 3
#define VERSION_MAX_LEN 20

static struct sparklink_uart_ops *g_uart_interface = NULL;
static struct sparklink_driver_ops *g_driver_interface = NULL;

static ssize_t sparklink_power_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int val = 0;
	int ret;

	if (!buf || count > POWR_MAX_LEN || !g_driver_interface) {
		sparklink_errmsg("power_store parameter is invalid\n");
		return -EINVAL;
	}

	if (SPARKLINK_STATE_UPDATING == g_driver_interface->device_get_state()) {
		sparklink_errmsg("not allow reset when 1162  upgrading firmware\n");
		return -EINVAL;
	}

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0 || val > SPARKLINK_STATE_MAX) {
		sparklink_errmsg("power_store err mode = %d, ret =%d\n", val, ret);
		return -EINVAL;
	}

	ret = g_driver_interface->device_powermode(val);
	if (ret < 0) {
		sparklink_errmsg("power_store set powermode fail ret = %d\n", ret);
		return ret;
	}

	return count;
}

static ssize_t sparklink_power_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;

	if (!buf || !g_driver_interface) {
		sparklink_errmsg("power_state parameter is invalid\n");
		return 0;
	}

	ret = g_driver_interface->device_get_state();
	return snprintf_s(buf, POWR_MAX_LEN + 2, POWR_MAX_LEN + 1, "%d\n", ret);
}

static DEVICE_ATTR(sparklink_power, S_IWUSR | S_IRUSR | S_IRUGO,
	sparklink_power_show, sparklink_power_store);

static ssize_t sparklink_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;

	if (!buf) {
		sparklink_errmsg("version show buf invalid\n");
		return 0;
	}

	if (g_driver_interface == NULL) {
		sparklink_errmsg("version show no device\n");
		return snprintf_s(buf, VERSION_MAX_LEN, VERSION_MAX_LEN - 1, "version:no_device");
	}

	ret = g_driver_interface->device_version();
	if (ret < 0)
		return snprintf_s(buf, VERSION_MAX_LEN, VERSION_MAX_LEN - 1, "version:NULL");

	return snprintf_s(buf, VERSION_MAX_LEN, VERSION_MAX_LEN - 1, "version:0x%x", ret);
}

static DEVICE_ATTR(sparklink_version, S_IRUSR | S_IRUGO, sparklink_version_show, NULL);

static ssize_t sparklink_log_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t sparklink_log_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return -EINVAL;
}

static DEVICE_ATTR(sparklink_log, S_IWUSR | S_IRUSR | S_IRUGO,
	sparklink_log_show, sparklink_log_store);

static ssize_t sparklink_uart_channel_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t sparklink_uart_channel_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int val = 0;
	int ret;

	if (!buf || count > SPARKLINK_UART_MAX_LEN || !g_driver_interface) {
		sparklink_errmsg("power_sparklink_uart_channel_store parameter is invalid\n");
		return -EINVAL;
	}

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0 || val >= SPARKLINK_UART_CHANNEL_MAX) {
		sparklink_errmsg("sparklink_uart_channel_store err mode = %d, ret = %d\n", val, ret);
		return -EINVAL;
	}

	ret = g_driver_interface->device_uart_switch_channel(val);
	if (ret < 0) {
		sparklink_errmsg("sparklink_uart_channel_store set uart channel fail ret = %d\n", ret);
		return ret;
	}

	return count;
}

static DEVICE_ATTR(sparklink_uart_channel, S_IWUSR | S_IRUSR | S_IRUGO,
	sparklink_uart_channel_show, sparklink_uart_channel_store);

static struct attribute *sparklink_attributes[] = {
	&dev_attr_sparklink_power.attr,
	&dev_attr_sparklink_version.attr,
	&dev_attr_sparklink_log.attr,
	&dev_attr_sparklink_uart_channel.attr,
	NULL
};

static const struct attribute_group sparklink_attr_group = {
	.attrs = sparklink_attributes,
};

int sparklink_uart_ops_register(struct sparklink_uart_ops *ops)
{
	if (!ops)
		return -EINVAL;

	g_uart_interface = ops;
	return 0;
}

int sparklink_uart_ops_unregister(void)
{
	g_uart_interface = NULL;
	return 0;
}

int spartlink_driver_ops_register(struct sparklink_driver_ops *ops)
{
	if (!ops)
		return -EINVAL;

	g_driver_interface = ops;
	return 0;
}

int spartlink_driver_ops_unregister(void)
{
	g_driver_interface = NULL;
	return 0;
}

struct sparklink_uart_ops *spartlink_get_uart_ops(void)
{
	return g_uart_interface;
}

static int sparklink_core_probe(struct platform_device *pdev)
{
	int err;

	sparklink_infomsg("sparklink_core_probe enter\n");
	if (!pdev) {
		sparklink_infomsg("pdev is NULL\n");
		return -ENODEV;
	}

	err = sysfs_create_group(&pdev->dev.kobj, &sparklink_attr_group);
	if (err < 0) {
		sparklink_errmsg("sysfs create error %d\n", err);
		goto sysfs_create_fail;
	}

	sparklink_infomsg("sparklink_core_probe successfully\n");

sysfs_create_fail:
	return err;
}

static int sparklink_core_remove(struct platform_device *pdev)
{
	if (!pdev)
		return -ENODEV;

	sysfs_remove_group(&pdev->dev.kobj, &sparklink_attr_group);
	return 0;
}

static const struct of_device_id spartlink_core_match_table[] = {
	{ .compatible = "huawei,sparklink", },
	{ },
};

MODULE_DEVICE_TABLE(of, spartlink_core_match_table);

struct platform_driver sparklink_core_drv_pf = {
	.probe = sparklink_core_probe,
	.remove = sparklink_core_remove,
	.driver = {
		.name = "sparklink_core_platform",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(spartlink_core_match_table),
	},
};

module_platform_driver(sparklink_core_drv_pf);

MODULE_AUTHOR("huawei");
MODULE_DESCRIPTION("huawei spartlink core");
MODULE_LICENSE("GPL v2");
