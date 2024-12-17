/*
 * hw_thermal_watch.c
 *
 * hw_thermal for watch
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

#include "hw_thermal_watch.h"

#include "thermal_common.h"

static struct power_supply *g_batt_psy = NULL;
static struct power_supply *g_usb_psy = NULL;

static bool check_batt_psy(void)
{
	if (g_batt_psy)
		return true;

	g_batt_psy = power_supply_get_by_name(PSY_BATTERY_NAME);
	if (!g_batt_psy) {
		pr_err("Failed to get battery power supply");
		return false;
	}
	pr_info("Get battery power supply");
	return true;
}

static bool check_usb_psy(void)
{
	if (g_usb_psy)
		return true;

	g_usb_psy = power_supply_get_by_name(PSY_USB_NAME);
	if (!g_usb_psy) {
		pr_err("Failed to get usb power supply");
		return false;
	}
	pr_info("Get usb power supply");
	return true;
}

static int get_psy_property(struct power_supply *psy, enum power_supply_property psp,
	union power_supply_propval *val)
{
	int ret;

	ret = power_supply_get_property(psy, psp, val);
	if (ret != 0) {
		pr_err("Failed to get power supply property: %d, ret: %d", psp, ret);
		return -1;
	}
	return 0;
}

static int get_batt_int_prop(enum power_supply_property psp, int defval)
{
	int ret;
	union power_supply_propval propval;

	if (!check_batt_psy())
		return defval;

	ret = get_psy_property(g_batt_psy, psp, &propval);
	return (ret == 0) ? propval.intval : defval;
}

static int get_usb_int_prop(enum power_supply_property psp, int defval)
{
	int ret;
	union power_supply_propval propval;

	if (!check_usb_psy())
		return defval;

	ret = get_psy_property(g_usb_psy, psp, &propval);
	return (ret == 0) ? propval.intval : defval;
}

static int get_power_supply_info(int flag, char *buf)
{
	int value = 0;
	switch (flag) {
	case THERMAL_INFO_CHG_ON:
		value = get_usb_int_prop(POWER_SUPPLY_PROP_ONLINE, -1);
		break;
	case THERMAL_INFO_BAT_STAT:
		value = get_batt_int_prop(POWER_SUPPLY_PROP_STATUS, -1);
		if (value == POWER_SUPPLY_STATUS_CHARGING)
			return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", CHAGING_STAT);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", DISCHAGING_STAT);
	case THERMAL_INFO_IBUS:
		value = get_usb_int_prop(POWER_SUPPLY_PROP_CURRENT_NOW, -1);
		break;
	case THERMAL_INFO_VBUS:
		value = get_usb_int_prop(POWER_SUPPLY_PROP_VOLTAGE_NOW, -1);
		break;
	case THERMAL_INFO_IBAT:
		value = get_batt_int_prop(POWER_SUPPLY_PROP_CURRENT_NOW, -1);
		break;
	case THERMAL_INFO_VBAT:
		value = get_batt_int_prop(POWER_SUPPLY_PROP_VOLTAGE_NOW, -1);
		break;
	case THERMAL_INFO_CAP:
		value = get_batt_int_prop(POWER_SUPPLY_PROP_CAPACITY_RM, -1);
		break;
	default:
		pr_err("Exp node flag err.\n");
		value = 0;
	}
	return value;
}

static ssize_t hw_shell_show_exp_node(struct device *dev, struct device_attribute *devattr, char *buf)
{
	int value = 0;
	struct hw_shell_t *exp_node = NULL;

	if (dev == NULL || devattr == NULL || buf == NULL)
		return 0;

	if (dev->driver_data == NULL)
		return 0;
	exp_node = dev->driver_data;

	pr_info("shell exp node show, flag: %d", exp_node->flag);
	if (exp_node->flag <= THERMAL_INFO_CAP)
		value = get_power_supply_info(exp_node->flag, buf);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", value);
}

static ssize_t
hw_shell_store_exp_node(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
	return 0;
}

static DEVICE_ATTR(exp_node, S_IWUSR | S_IRUGO, hw_shell_show_exp_node, hw_shell_store_exp_node);

static int handle_exp_node(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	struct hw_shell_t *node_data;
	int ret;
	int flag = 0;

	node_data = kzalloc(sizeof(*node_data), GFP_KERNEL);
	if (node_data == NULL) {
		pr_err("Not enough memory for node_data.\n");
		return -ENODEV;
	}

	ret = of_property_read_s32(dev_node, "node_type", &flag);
	if (ret != 0) {
		pr_err("%s node_type read err.\n", __func__);
		goto free_mem;
	}
	node_data->is_transfer = IS_TRANSFER;
	node_data->flag = flag;

	platform_set_drvdata(pdev, node_data);
	ret = thermal_node_probe(pdev, &dev_attr_exp_node.attr);
	if (ret != 0) {
		pr_err("%s create thermal node err.\n", __func__);
		goto free_mem;
	}
	return ret;

free_mem:
	kfree(node_data);
	return ret;
}

static int hw_shell_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret;
	int is_transfer = 0;

	if (!of_device_is_available(dev_node)) {
		dev_err(dev, "HW shell dev not found\n");
		return -ENODEV;
	}

	ret = of_property_read_s32(dev_node, "is_transfer", &is_transfer);
	if (ret != 0) {
		pr_err("%s is_transfer read err.\n", __func__);
		return ret;
	}

	if (is_transfer == IS_TRANSFER)
		ret = handle_exp_node(pdev);
	return ret;
}

static int hw_shell_remove(struct platform_device *pdev)
{
	struct hw_shell_t *hw_shell = platform_get_drvdata(pdev);

	if (hw_shell != NULL) {
		platform_set_drvdata(pdev, NULL);
		kfree(hw_shell);
	}

	return 0;
}

static struct of_device_id hw_shell_of_match[] = {
	{ .compatible = "hw,shell-temp" },
	{},
};

MODULE_DEVICE_TABLE(of, hw_shell_of_match);

static struct platform_driver hw_shell_platdrv = {
	.driver = {
		.name = "hw-shell-temp",
		.owner = THIS_MODULE,
		.of_match_table = hw_shell_of_match,
	},
	.probe = hw_shell_probe,
	.remove = hw_shell_remove,
};


static int __init hw_shell_init(void)
{
	return platform_driver_register(&hw_shell_platdrv);
}

static void __exit hw_shell_exit(void)
{
	if (g_batt_psy) {
		power_supply_put(g_batt_psy);
		g_batt_psy = NULL;
	}
	if (g_usb_psy) {
		power_supply_put(g_usb_psy);
		g_usb_psy = NULL;
	}
	platform_driver_unregister(&hw_shell_platdrv);
}

late_initcall(hw_shell_init);
module_exit(hw_shell_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("thermal shell temp module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
