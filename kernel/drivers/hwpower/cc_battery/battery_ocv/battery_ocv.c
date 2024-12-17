// SPDX-License-Identifier: GPL-2.0
/*
 * battery_ocv.c
 *
 * battery ocv driver
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/sort.h>
#include <linux/spinlock.h>
#include <securec.h>
#include <chipset_common/hwpower/battery/battery_ocv.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>

#define HWLOG_TAG battery_ocv
HWLOG_REGIST();

#define DEFAULT_TABLE_MAX_SIZE 16
#define OCV_TABLE_MAX_SIZE   128
#define BAT_OCV_BUF_MAX_SIZE 256

enum battery_ocv_sysfs_type {
	BATTERY_OCV_SYSFS_BEGIN = 0,
	BATTERY_OCV_SYSFS_UPDATE_OCV_TABLE = BATTERY_OCV_SYSFS_BEGIN,
	BATTERY_OCV_SYSFS_END,
};

enum {
	BAT_OCV_TABLE_BEGIN = 0,
	BAT_OCV_TABLE_CAPACITY = BAT_OCV_TABLE_BEGIN,
	BAT_OCV_TABLE_VOLTAGE,
	BAT_OCV_TABLE_END,
};

struct ocv_table {
	int cap;
	int voltage;
};

struct battery_ocv_dev {
	struct device *dev;
	spinlock_t data_lock;
	struct ocv_table *dynamic_ocv_table;
	int dynamic_table_size;
	struct ocv_table *default_table;
	int default_table_size;
};

static struct battery_ocv_dev *g_battery_ocv_dev;

static int battery_ocv_get_ocv_from_table(struct ocv_table *table, int size,
	int cap, int *ocv)
{
	int i;

	if (!table || (size <= 0))
		return -EINVAL;

	for (i = 0; i < size; i++) {
		if (table[i].cap == cap) {
			*ocv = table[i].voltage;
			return 0;
		}
	}

	return -EINVAL;
}

int battery_ocv_get_ocv_by_cap(int cap, int *ocv)
{
	int ret;
	struct battery_ocv_dev *di = g_battery_ocv_dev;

	if (!di || !ocv)
		return -ENODEV;

	spin_lock(&di->data_lock);
	ret = battery_ocv_get_ocv_from_table(di->dynamic_ocv_table,
		di->dynamic_table_size, cap, ocv);
	spin_unlock(&di->data_lock);
	if (ret)
		return battery_ocv_get_ocv_from_table(di->default_table,
			di->default_table_size, cap, ocv);
	return 0;
}

static int battery_ocv_cmp(const void *a, const void *b)
{
	const struct ocv_table *x = a;
	const struct ocv_table *y = b;

	return x->cap - y->cap;
}

static int battery_ocv_update_ocv_table(struct ocv_table *table, int size)
{
	int i, ret;
	struct ocv_table *temp_table = NULL;
	struct battery_ocv_dev *di = g_battery_ocv_dev;

	if (!di || !table)
		return -EINVAL;

	temp_table = kzalloc(sizeof(struct ocv_table) * size, GFP_KERNEL);
	if (!temp_table)
		return -ENOMEM;

	ret = memcpy_s(temp_table, sizeof(struct ocv_table) * size,
		table, sizeof(struct ocv_table) * size);
	if (ret != EOK) {
		kfree(temp_table);
		return -ENOMEM;
	}

	sort(temp_table, size, sizeof(struct ocv_table), battery_ocv_cmp, NULL);
	for (i = 0; i < size - 1; i++) {
		if (temp_table[i + 1].voltage < temp_table[i].voltage)
			temp_table[i + 1].voltage = temp_table[i].voltage;
	}
	for (i = 0; i < size; i++)
		hwlog_info("table[%d] %d %d\n", i,
			temp_table[i].cap, temp_table[i].voltage);

	spin_lock(&di->data_lock);
	if (di->dynamic_ocv_table)
		kfree(di->dynamic_ocv_table);
	di->dynamic_ocv_table = temp_table;
	di->dynamic_table_size = size;
	spin_unlock(&di->data_lock);
	power_event_bnc_notify(POWER_BNT_BATTERY, POWER_NE_BATTERY_OCV_CHANGE, NULL);
	return 0;
}

static int battery_ocv_parse_dynamic_ocv_table(char *buf)
{
	char *tmp1 = NULL;
	char *tmp2 = NULL;
	int count = 0;
	struct ocv_table table[OCV_TABLE_MAX_SIZE] = { 0 };

	if (!buf)
		return -EINVAL;

	/* The correct string format is "-1@3000,0@3100,1@3200" */
	tmp1 = strsep(&buf, ",");
	while (tmp1) {
		tmp2 = strsep(&tmp1, "@");
		if (!tmp2 || !tmp1) {
			hwlog_info("tmp1 or tmp2 is null\n");
			return -EINVAL;
		}
		if (kstrtoint(tmp2, 10, &table[count].cap)) {
			hwlog_err("kstrtoint tmp2 failed, count=%d\n", count);
			return -EINVAL;
		}
		if (kstrtoint(tmp1, 10, &table[count].voltage)) {
			hwlog_err("kstrtoint tmp1 failed, count=%d\n", count);
			return -EINVAL;
		}
		count++;

		if (!buf)
			break;
		tmp1 = strsep(&buf, ",");
	}

	if (count <= 0) {
		hwlog_err("parse table error, count=%d\n", count);
		return -EINVAL;
	}

	if (battery_ocv_update_ocv_table(table, count)) {
		hwlog_err("update ocv table error\n");
		return -EINVAL;
	}
	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t battery_ocv_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t battery_ocv_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info battery_ocv_sysfs_field_tbl[] = {
	power_sysfs_attr_wo(battery_ocv, 0220,
		BATTERY_OCV_SYSFS_UPDATE_OCV_TABLE, update_ocv_table),
};

#define BATTERY_OCV_SYSFS_ATTRS_SIZE ARRAY_SIZE(battery_ocv_sysfs_field_tbl)

static struct attribute *battery_ocv_sysfs_attrs[BATTERY_OCV_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group battery_ocv_sysfs_attr_group = {
	.attrs = battery_ocv_sysfs_attrs,
};

static ssize_t battery_ocv_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct battery_ocv_dev *di = dev_get_drvdata(dev);

	if (!di)
		return -ENODEV;

	info = power_sysfs_lookup_attr(attr->attr.name,
		battery_ocv_sysfs_field_tbl, BATTERY_OCV_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	default:
		return 0;
	}
}

static ssize_t battery_ocv_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	char buff[BAT_OCV_BUF_MAX_SIZE] = { 0 };
	struct battery_ocv_dev *di = dev_get_drvdata(dev);

	if (!di)
		return -ENODEV;

	info = power_sysfs_lookup_attr(attr->attr.name,
		battery_ocv_sysfs_field_tbl, BATTERY_OCV_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case BATTERY_OCV_SYSFS_UPDATE_OCV_TABLE:
		if (snprintf_s(buff, BAT_OCV_BUF_MAX_SIZE,
			BAT_OCV_BUF_MAX_SIZE - 1, "%s", buf) < 0)
			return -EINVAL;
		hwlog_info("origin buf=%s, saved buf=%s\n", buf, buff);
		if (battery_ocv_parse_dynamic_ocv_table(buff))
			return -EINVAL;
		break;
	default:
		break;
	}

	return count;
}

static void battery_ocv_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(battery_ocv_sysfs_attrs,
		battery_ocv_sysfs_field_tbl, BATTERY_OCV_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "battery", "battery_ocv",
		dev, &battery_ocv_sysfs_attr_group);
}

static void battery_ocv_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "battery", "battery_ocv",
		dev, &battery_ocv_sysfs_attr_group);
}
#else
static inline void battery_ocv_sysfs_create_group(struct device *dev)
{
}

static inline void battery_ocv_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int battery_ocv_parse_ocv_table(struct device_node *np,
	struct battery_ocv_dev *di)
{
	int row, col, len;
	int idata[DEFAULT_TABLE_MAX_SIZE * BAT_OCV_TABLE_END] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"ocv_table", idata, DEFAULT_TABLE_MAX_SIZE, BAT_OCV_TABLE_END);
	if (len < 0) {
		hwlog_err("parse ocv_table failed\n");
		return -EINVAL;
	}
	len /= BAT_OCV_TABLE_END;

	di->default_table = kzalloc(sizeof(struct ocv_table) * len, GFP_KERNEL);
	if (!di->default_table)
		return -ENOMEM;

	for (row = 0; row < len; row++) {
		col = row * BAT_OCV_TABLE_END + BAT_OCV_TABLE_CAPACITY;
		di->default_table[row].cap = idata[col];
		col = row * BAT_OCV_TABLE_END + BAT_OCV_TABLE_VOLTAGE;
		di->default_table[row].voltage = idata[col];
		hwlog_info("default table[%d] %d %d\n", row,
			di->default_table[row].cap,
			di->default_table[row].voltage);
	}

	di->default_table_size = len;
	return 0;
}

static int battery_ocv_probe(struct platform_device *pdev)
{
	int ret;
	struct battery_ocv_dev *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;

	ret = battery_ocv_parse_ocv_table(np, di);
	if (ret) {
		hwlog_err("battery_ocv_parse_ocv_table failed\n");
		goto fail_free_mem;
	}

	spin_lock_init(&di->data_lock);
	battery_ocv_sysfs_create_group(di->dev);
	platform_set_drvdata(pdev, di);
	g_battery_ocv_dev = di;
	return 0;

fail_free_mem:
	kfree(di->default_table);
	devm_kfree(&pdev->dev, di);
	g_battery_ocv_dev = NULL;
	return ret;
}

static int battery_ocv_remove(struct platform_device *pdev)
{
	struct battery_ocv_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	battery_ocv_sysfs_remove_group(di->dev);
	kfree(di->default_table);
	kfree(di->dynamic_ocv_table);
	devm_kfree(&pdev->dev, di);
	g_battery_ocv_dev = NULL;
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id battery_ocv_match_table[] = {
	{
		.compatible = "huawei,battery_ocv",
		.data = NULL,
	},
	{},
};

static struct platform_driver battery_ocv_driver = {
	.probe = battery_ocv_probe,
	.remove = battery_ocv_remove,
	.driver = {
		.name = "huawei,battery_ocv",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(battery_ocv_match_table),
	},
};
module_platform_driver(battery_ocv_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery ocv driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
