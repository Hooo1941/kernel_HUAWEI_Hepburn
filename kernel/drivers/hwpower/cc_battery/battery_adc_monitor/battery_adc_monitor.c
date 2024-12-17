// SPDX-License-Identifier: GPL-2.0
/*
 * battery_adc_monitor.c
 *
 * battery adc monitor driver
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/battery/battery_adc_monitor.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>

#define HWLOG_TAG batt_adc_monitor
HWLOG_REGIST();

struct batt_adc_monitor_info {
	struct device *dev;
	struct batt_adc_monitor_ops *ops;
};

static struct batt_adc_monitor_info *g_batt_adc_monitor_info;

enum batt_adc_monitor_type {
	BATT_ADC_SYSFS_IBAT_MAIN = 0,
	BATT_ADC_SYSFS_IBAT_AUX,
};

static struct batt_adc_monitor_ops g_batt_adc_monitor_ops[BATT_ID_MAX];

int batt_adc_monitor_get_prop(enum batt_id id, enum prop_type prop, int *val)
{
	struct batt_adc_monitor_info *di = g_batt_adc_monitor_info;

	if (!di)
		return -EINVAL;

	if ((id < BATT_ID_MAIN) || (id >= BATT_ID_MAX) || !val)
		return -EINVAL;

	if (!power_platform_is_battery_exit()) {
		hwlog_err("battery not exist\n");
		return -ENODEV;
	}

	if (prop == BATT_VOLTAGE) {
		if (!di->ops[id].get_vbat)
			return -ENODEV;
		*val = di->ops[id].get_vbat(di->ops[id].vbat_data);
	} else if (prop == BATT_CURRENT) {
		if (!di->ops[id].get_ibat)
			return -ENODEV;
		*val = di->ops[id].get_ibat(di->ops[id].ibat_data);
	} else {
		return -EINVAL;
	}
	return 0;
}

int batt_adc_monitor_ops_register(enum batt_id id, enum prop_type prop,
	battery_adc_monitor_get_func func, void *data)
{
	if (!func || !data) {
		hwlog_err("input arg err\n");
		return -EINVAL;
	}

	if ((id < BATT_ID_MAIN) || (id >= BATT_ID_MAX))
		return -EINVAL;

	if ((prop < BATT_CURRENT) || (prop >= BATT_PROP_MAX))
		return -EINVAL;

	if (prop == BATT_VOLTAGE) {
		if (g_batt_adc_monitor_ops[id].get_vbat) {
			hwlog_err("batt_%d vol ops already register!\n", id);
			return -EINVAL;
		}
		g_batt_adc_monitor_ops[id].get_vbat = func;
		g_batt_adc_monitor_ops[id].vbat_data = data;
	} else if (prop == BATT_CURRENT) {
		if (g_batt_adc_monitor_ops[id].get_ibat) {
			hwlog_err("batt_%d current ops already register!\n", id);
			return -EINVAL;
		}
		g_batt_adc_monitor_ops[id].get_ibat = func;
		g_batt_adc_monitor_ops[id].ibat_data = data;
	}

	hwlog_info("register batt_%d: prop_%d ok\n", id, prop);
	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t batt_adc_monitor_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info batt_adc_monitor_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(batt_adc_monitor, 0440, BATT_ADC_SYSFS_IBAT_MAIN, ibat_main),
	power_sysfs_attr_ro(batt_adc_monitor, 0440, BATT_ADC_SYSFS_IBAT_AUX, ibat_aux),
};

#define BATT_ADC_MONITOR_SYSFS_ATTRS_SIZE  ARRAY_SIZE(batt_adc_monitor_sysfs_field_tbl)

static struct attribute *batt_adc_monitor_sysfs_attrs[BATT_ADC_MONITOR_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group batt_adc_monitor_sysfs_attr_group = {
	.attrs = batt_adc_monitor_sysfs_attrs,
};

static ssize_t batt_adc_monitor_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	int batt_prop = 0;
	int len = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		batt_adc_monitor_sysfs_field_tbl, BATT_ADC_MONITOR_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case BATT_ADC_SYSFS_IBAT_MAIN:
		batt_adc_monitor_get_prop(BATT_ID_MAIN, BATT_CURRENT, &batt_prop);
		len = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", batt_prop);
		hwlog_info("%s: IBAT_MAIN = %d\n", __func__, batt_prop);
		break;
	case BATT_ADC_SYSFS_IBAT_AUX:
		batt_adc_monitor_get_prop(BATT_ID_AUX, BATT_CURRENT, &batt_prop);
		len = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", batt_prop);
		hwlog_info("%s: IBAT_AUX = %d\n", __func__, batt_prop);
		break;
	default:
		break;
	}

	return len;
}

static void batt_adc_monitor_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(batt_adc_monitor_sysfs_attrs,
		batt_adc_monitor_sysfs_field_tbl, BATT_ADC_MONITOR_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "batt_adc_monitor",
		dev, &batt_adc_monitor_sysfs_attr_group);
}

static void batt_adc_monitor_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "batt_adc_monitor",
		dev, &batt_adc_monitor_sysfs_attr_group);
}
#else
static inline void batt_adc_monitor_sysfs_create_group(struct device *dev)
{
}

static inline void batt_adc_monitor_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int batt_adc_monitor_probe(struct platform_device *pdev)
{
	struct batt_adc_monitor_info *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	di->ops = g_batt_adc_monitor_ops;
	platform_set_drvdata(pdev, di);

	batt_adc_monitor_sysfs_create_group(di->dev);
	g_batt_adc_monitor_info = di;
	hwlog_info("%s ok\n", __func__);
	return 0;
}

static int batt_adc_monitor_remove(struct platform_device *pdev)
{
	struct batt_adc_monitor_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	batt_adc_monitor_sysfs_remove_group(di->dev);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_batt_adc_monitor_info = NULL;

	return 0;
}

static const struct of_device_id batt_adc_monitor_match_table[] = {
	{
		.compatible = "huawei,batt_adc_monitor",
		.data = NULL,
	},
	{},
};

static struct platform_driver batt_adc_monitor_driver = {
	.probe = batt_adc_monitor_probe,
	.remove = batt_adc_monitor_remove,
	.driver = {
		.name = "huawei,batt_adc_monitor",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(batt_adc_monitor_match_table),
	},
};

static int __init batt_adc_monitor_init(void)
{
	return platform_driver_register(&batt_adc_monitor_driver);
}

static void __exit batt_adc_monitor_exit(void)
{
	platform_driver_unregister(&batt_adc_monitor_driver);
}

fs_initcall_sync(batt_adc_monitor_init);
module_exit(batt_adc_monitor_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery adc monitor module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
