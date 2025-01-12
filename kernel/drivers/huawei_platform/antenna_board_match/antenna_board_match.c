/*
 * antenna_board_match.c
 *
 * Check the antenna board match status.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/antenna_board_match/antenna_board_match.h>
#include <securec.h>
#define HWLOG_TAG antenna_board_match
HWLOG_REGIST();

struct antenna_device_ops *g_antenna_board_match_ops;

#define antenna_sysfs_field(_name, n, m, store) {	\
	.attr = __ATTR(_name, m, antenna_show, store),	\
	.name = ANTENNA_##n,	\
}

#define antenna_sysfs_field_ro(_name, n)	\
		antenna_sysfs_field(_name, n, 0444, NULL)

static ssize_t antenna_show(struct device *dev,
		struct device_attribute *attr, char *buf);

struct antenna_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

/* AT^GETANTBDVAL= ANTTYPE */
struct antenna_type {
	int type;
	int min;
	int max;
};

static struct antenna_sysfs_field_info antenna_tb[] = {
	antenna_sysfs_field_ro(antenna_board_match, BOARD_MATCH),
	antenna_sysfs_field_ro(antenna_board_voltage, BOARD_VOLTAGE),
	antenna_sysfs_field_ro(antenna_board_type, BOARD_TYPE),
};

static struct attribute *antenna_sysfs_attrs[ARRAY_SIZE(antenna_tb) + 1];

static const struct attribute_group antenna_sysfs_attr_group = {
	.attrs = antenna_sysfs_attrs,
};

static void antenna_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(antenna_tb);

	for (i = 0; i < limit; i++)
		antenna_sysfs_attrs[i] = &antenna_tb[i].attr.attr;
	antenna_sysfs_attrs[limit] = NULL;
}

static struct antenna_sysfs_field_info *antenna_board_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(antenna_tb);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, antenna_tb[i].attr.attr.name, strlen(name)))
			break;
	}
	if (i >= limit)
		return NULL;
	return &antenna_tb[i];
}

static int antenna_match_sysfs_create_group(struct antenna_device_info *di)
{
	antenna_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &antenna_sysfs_attr_group);
}

static inline void antenna_match_sysfs_remove_group(
	struct antenna_device_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &antenna_sysfs_attr_group);
}

static int antenna_type_check(int voltage)
{
	/* Voltages in 9 ranges, each range representing a type. [vol >= min; vol < max) */
	struct antenna_type ant_vol_tbl[] = {
		{ 0, 0, 100},
		{ 1, 100, 300},
		{ 2, 300, 500},
		{ 3, 500, 700},
		{ 4, 700, 900},
		{ 5, 1100, 1300},
		{ 6, 1300, 1500},
		{ 7, 1500, 1700},
		{ 8, 1700, 1875},
	};
	unsigned int index;

	size_t len = sizeof(ant_vol_tbl) / sizeof(struct antenna_type);
	for (index = 0; index < len; index++)
		if ((voltage >= ant_vol_tbl[index].min) && (voltage < ant_vol_tbl[index].max))
			return ant_vol_tbl[index].type;
	return -1;
}

static ssize_t antenna_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int adc_ret;
	struct antenna_sysfs_field_info *info = NULL;
	struct antenna_device_info *di = dev_get_drvdata(dev);

	if (di == NULL) {
		hwlog_err("%s: di is NULL!\n", __func__);
		return -ENODEV;
	}

	info = antenna_board_lookup(attr->attr.name);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case ANTENNA_BOARD_MATCH:
		adc_ret = di->ops->get_antenna_match_status();
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", adc_ret);
	case ANTENNA_BOARD_VOLTAGE:
		adc_ret = di->ops->get_antenna_board_voltage();
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", adc_ret);
	case ANTENNA_BOARD_TYPE:
		/* adc voltage, mv */
		adc_ret = di->ops->get_antenna_board_voltage();
		/* return antenna type */
		adc_ret = antenna_type_check(adc_ret);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", adc_ret);
	default:
		hwlog_err("%s: NODE ERR, HAVE NO THIS NODE: %d\n",
			__func__, info->name);
		break;
	}
	return 0;
}
static struct class *hw_antenna_class;
static struct class *antenna_board_match_class;
struct device *antenna_adc_dev;

struct class *hw_antenna_get_class(void)
{
	if (hw_antenna_class == NULL) {
		hw_antenna_class = class_create(THIS_MODULE, "hw_antenna");
		if (hw_antenna_class == NULL) {
			hwlog_err("hw_antenna_class create fail!");
			return NULL;
		}
	}
	return hw_antenna_class;
}

int antenna_match_ops_register(struct antenna_device_ops *ops)
{
	int ret = 0;

	if (ops != NULL) {
		g_antenna_board_match_ops = ops;
	} else {
		hwlog_err("antenna ops register fail!\n");
		ret = -EPERM;
	}
	return ret;
}

static int antenna_board_match_probe(struct platform_device *pdev)
{
	int ret;
	struct antenna_device_info *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	di->ops = g_antenna_board_match_ops;

	if ((di->ops == NULL) || (di->ops->get_antenna_match_status == NULL) ||
		(di->ops->get_antenna_board_voltage == NULL)) {
		hwlog_err("antenna_board_match ops is NULL!\n");
		goto Antenna_board_failed_0;
	}

	platform_set_drvdata(pdev, di);

	ret = antenna_match_sysfs_create_group(di);
	if (ret) {
		hwlog_err("can't create antenna_match sysfs entries\n");
		goto Antenna_board_failed_0;
	}
	antenna_board_match_class = hw_antenna_get_class();
	if (antenna_board_match_class) {
		if (antenna_adc_dev == NULL)
			antenna_adc_dev = device_create(
				antenna_board_match_class,
				NULL, 0, NULL, "antenna_board");
		if (IS_ERR(antenna_adc_dev)) {
			antenna_adc_dev = NULL;
			hwlog_err("create rf_dev failed!\n");
			goto Antenna_board_failed_1;
		}
		ret = sysfs_create_link(&antenna_adc_dev->kobj, &di->dev->kobj,
			"antenna_board_data");
		if (ret) {
			hwlog_err("create link to board_match fail\n");
			goto Antenna_board_failed_1;
		}
	} else {
		hwlog_err("get antenna_match_class fail\n");
		goto Antenna_board_failed_1;
	}
	hwlog_info("huawei antenna board match probe ok!\n");
	return 0;

Antenna_board_failed_1:
	antenna_match_sysfs_remove_group(di);
Antenna_board_failed_0:
	platform_set_drvdata(pdev, NULL);
	di->ops = NULL;
	kfree(di);
	di = NULL;
	return -1;
}

static int antenna_board_match_remove(struct platform_device *pdev)
{
	struct antenna_device_info *di = platform_get_drvdata(pdev);

	if (di == NULL) {
		hwlog_err("[%s]di is NULL!\n", __func__);
		return -ENODEV;
	}

	if (di->ops != NULL) {
		di->ops = NULL;
		g_antenna_board_match_ops = NULL;
	}
	kfree(di);
	di = NULL;
	return 0;
}
/* probe match table */
static const struct of_device_id antenna_board_table[] = {
	{
		.compatible = "huawei,antenna_board_match",
		.data = NULL,
	},
	{},
};

/* antenna board match driver */
static struct platform_driver antenna_board_match_driver = {
	.probe = antenna_board_match_probe,
	.remove = antenna_board_match_remove,
	.driver = {
		.name = "huawei,antenna_board_match",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(antenna_board_table),
	},
};

static int __init antenna_board_match_init(void)
{
	return platform_driver_register(&antenna_board_match_driver);
}

static void __exit antenna_board_match_exit(void)
{
	platform_driver_unregister(&antenna_board_match_driver);
}

device_initcall_sync(antenna_board_match_init);
module_exit(antenna_board_match_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei antenna board match driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

