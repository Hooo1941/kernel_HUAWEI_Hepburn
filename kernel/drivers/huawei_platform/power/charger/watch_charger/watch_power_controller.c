// SPDX-License-Identifier: MIT
/*
 * watch_power_controller.c
 *
 * watch controller for power module
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <linux/reboot.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <securec.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/log/hw_log.h>
#include "ext_sensorhub_api.h"

#define HWLOG_TAG power_controller
HWLOG_REGIST();

#define DEVICE_SERVICE_ID 0x01
#define DEVICE_COMMAND_ID 0x85
#define PAYLOAD_LEN 2
#define SHIP_MODE_RW_BUF_SIZE 16
#define DELAY_TIME 100

enum ship_mode_sysfs_type {
	SHIP_MODE_SYSFS_BEGIN = 0,
	SHIP_MODE_SYSFS_WORK_MODE,
	SHIP_MODE_SYSFS_END,
};

enum ship_mode_work_mode {
	SHIP_MODE_NOT_IN_SHIP = 0,
	SHIP_MODE_IN_SHIP,
	SHIP_MODE_IN_SHUTDOWN_SHIP,
};

enum mcu_power_state {
	WATCH_POWER_OFF = 0x00,       /* whole machine shutdown */
	WATCH_POWER_RESET = 0x03,     /* whole machine reset */
	WATCH_POWER_SHIPMODE = 0x05,  /* enter ship_mode */
};

struct ship_mode_para {
	unsigned int work_mode;
};

struct power_controller_dev {
	struct device *dev;
	struct ship_mode_para para;
};

static struct power_controller_dev *g_power_controller_info;

static const char * const g_ship_mode_op_user_table[] = {
	[0] = "shell",
	[1] = "atcmd",
	[2] = "hidl",
};

#ifdef CONFIG_SYSFS
static ssize_t ship_mode_sysfs_show(struct device *dev,
				    struct device_attribute *attr, char *buf);
static ssize_t ship_mode_sysfs_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count);

static struct power_sysfs_attr_info ship_mode_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(ship_mode, 0660,
			    SHIP_MODE_SYSFS_WORK_MODE, work_mode)
};

#define SHIP_MODE_SYSFS_ATTRS_SIZE  ARRAY_SIZE(ship_mode_sysfs_field_tbl)

static struct attribute *ship_mode_sysfs_attrs[SHIP_MODE_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group ship_mode_sysfs_attr_group = {
	.attrs = ship_mode_sysfs_attrs,
};

static struct power_controller_dev *power_controller_get_dev(void)
{
	if (!g_power_controller_info) {
		hwlog_err("g_power_controller_info is null\n");
		return NULL;
	}
	return g_power_controller_info;
}

static int ship_mode_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_ship_mode_op_user_table); i++) {
		if (!strcmp(str, g_ship_mode_op_user_table[i]))
			return i;
	}
	hwlog_err("invalid user_str=%s\n", str);
	return -EPERM;
}

static ssize_t ship_mode_sysfs_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_controller_dev *l_dev = NULL;

	l_dev = power_controller_get_dev();
	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
				       ship_mode_sysfs_field_tbl,
				       SHIP_MODE_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case SHIP_MODE_SYSFS_WORK_MODE:
		return scnprintf(buf, PAGE_SIZE, "%u\n", l_dev->para.work_mode);
	default:
		return 0;
	}
}

static ssize_t ship_mode_sysfs_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_controller_dev *l_dev = NULL;
	char user[SHIP_MODE_RW_BUF_SIZE] = { 0 };
	int value;

	l_dev = power_controller_get_dev();
	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
				       ship_mode_sysfs_field_tbl,
				       SHIP_MODE_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (SHIP_MODE_RW_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 2: the fields of "user value" */
	if (sscanf_s(buf, "%s %d", user, SHIP_MODE_RW_BUF_SIZE, &value) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (ship_mode_get_op_user(user) < 0)
		return -EINVAL;

	hwlog_info("ship mode sysfs store set: name=%d, user=%s, value=%d\n",
		   info->name, user, value);

	switch (info->name) {
	case SHIP_MODE_SYSFS_WORK_MODE:
		if (value != SHIP_MODE_NOT_IN_SHIP &&
		    value != SHIP_MODE_IN_SHIP &&
		    value != SHIP_MODE_IN_SHUTDOWN_SHIP) {
			hwlog_err("invalid value=%d\n", value);
			return -EINVAL;
		}
		l_dev->para.work_mode = value;
		break;
	default:
		break;
	}
	return count;
}

static struct device *power_controller_sysfs_create_group(void)
{
	power_sysfs_init_attrs(ship_mode_sysfs_attrs,
			       ship_mode_sysfs_field_tbl,
			       SHIP_MODE_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "ship_mode",
					&ship_mode_sysfs_attr_group);
}

static void ship_mode_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &ship_mode_sysfs_attr_group);
}

#else
static inline struct device *ship_mode_sysfs_create_group(void)
{
	return NULL;
}

static inline void ship_mode_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int pm_notify_mcu(unsigned char power_status)
{
	int ret;
	struct command send_to_mcu = {0};
	unsigned char pay_load[PAYLOAD_LEN] = {power_status, 0x00};

	send_to_mcu.service_id = DEVICE_SERVICE_ID;
	send_to_mcu.command_id = DEVICE_COMMAND_ID;
	send_to_mcu.send_buffer = pay_load;
	send_to_mcu.send_buffer_len = PAYLOAD_LEN;
	ret = send_command(MNTN_CHANNEL, &send_to_mcu, false, NULL);
	if (ret < 0)
		hwlog_err("[%s], send mcu msg fail!\n", __func__);

	hwlog_info("[%s], power_status = %u!\n", __func__, power_status);
	return ret;
}

static int pm_watch_reboot_handler(struct notifier_block *this,
				   unsigned long mode, void *cmd)
{
	unsigned char power_status;
	unsigned int count = 0;
	int ret_notify = -1;

	if (cmd)
		hwlog_info("mode = %lu, cmd = %s\n", mode, cmd);
	if (mode == SYS_RESTART) {
		power_status = WATCH_POWER_RESET;
	} else if (mode == SYS_POWER_OFF) {
		power_status = WATCH_POWER_OFF;
		if (g_power_controller_info) {
			if (g_power_controller_info->para.work_mode ==
			    SHIP_MODE_IN_SHUTDOWN_SHIP)
				power_status = WATCH_POWER_SHIPMODE;
		}
	} else {
		hwlog_err("[%s], mode is valid!\n", __func__);
		return -ENODEV;
	}

	/* retry 2 times */
	while (count < 2) {
		ret_notify = pm_notify_mcu(power_status);
		if (ret_notify >= 0)
			break;
		mdelay(DELAY_TIME);
		count++;
	}
	hwlog_info("reboot_handler process type [%u]\n", power_status);
	return 0;
}

static struct notifier_block watch_reboot_nb = {
	.notifier_call = pm_watch_reboot_handler,
	.priority = INT_MAX,
};

static int power_controller_probe(struct platform_device *pdev)
{
	int ret;
	struct power_controller_dev *l_dev = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;
	l_dev->dev = power_controller_sysfs_create_group();
	g_power_controller_info = l_dev;
	platform_set_drvdata(pdev, l_dev);
	ret = register_reboot_notifier(&watch_reboot_nb);
	if (ret)
		hwlog_err("%s register_watch_reboot_handler fail\n", __func__);
	return ret;
}

static int power_controller_remove(struct platform_device *pdev)
{
	struct power_controller_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	ship_mode_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	unregister_reboot_notifier(&watch_reboot_nb);
	kfree(l_dev);
	g_power_controller_info = NULL;
	return 0;
}

static const struct of_device_id power_controller_match_table[] = {
	{
		.compatible = "huawei, power_controller",
		.data = NULL,
	}
};

static struct platform_driver power_controller_driver = {
	.probe = power_controller_probe,
	.remove = power_controller_remove,
	.driver = {
		.name = "huawei, power_controller",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(power_controller_match_table),
	},
};

static int __init power_controller_init(void)
{
	return platform_driver_register(&power_controller_driver);
}

static void __exit power_controller_exit(void)
{
	platform_driver_unregister(&power_controller_driver);
}

late_initcall(power_controller_init);
module_exit(power_controller_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei watch power controller module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
