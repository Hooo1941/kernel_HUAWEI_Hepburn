/*
 * check_usbmode.c
 *
 * usb mode check driver
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_drivers/usb/tca.h>
#include <linux/fs.h>
#include <linux/power_supply.h>
#include <securec.h>

#define HOST_MODE_STR "host"
#define DEVICE_MODE_STR "device"
#define DTS_HOST_MODE_STR "usb_dp"
#define DTS_DEVICE_MODE_STR "usb_device"
#define DEVICE_USB_OUT 0
#define DEVICE_USB_IN 1
#define MAX_USBMODE_SIZE 7
#define max(a, b) ((a) > (b) ? (a) : (b))

enum usb_init_mode {
	COMBOPHY_INIT_DEFAULT_MODE,
	COMBOPHY_INIT_USB_HOST_MODE,
	COMBOPHY_INIT_USB_DEVICE_MODE
};

static int major = 0;
static struct class *class = NULL;
static int vxboot_gpio = 0;
char usb_mode[MAX_USBMODE_SIZE] = {0};

static int check_usbmode_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	if ((inode == NULL) || (file == NULL)) {
		pr_err("[USBMODE] %s, invalid arguments\n", __func__);
		return -EINVAL;
	}
	ret = nonseekable_open(inode, file);
	if (ret < 0) {
		pr_err("[USBMODE] %s nonseekable_open ret = %d failed\n", __func__, ret);
		return ret;
	}
	return ret;
}

static ssize_t check_usbmode_write(struct file *file,
				const char __user *buffer,
				size_t size, loff_t *off)
{
	if ((size > MAX_USBMODE_SIZE) || (file == NULL) || (buffer == NULL)) {
		pr_err("[USBMODE] %s input parameter is invalid\n", __func__);
		return -EINVAL;
	}

	if (memset_s(usb_mode, sizeof(usb_mode), 0, sizeof(usb_mode)) != EOK) {
		pr_err("%s [USBMODE] memset_s usb_mode fail.\n", __func__);
		return -EINVAL;
	}
 
	if (copy_from_user(usb_mode, buffer, size)) {
		pr_err("[USBMODE] %s copy_from_user failed\n", __func__);
		return -EINVAL;
	}

	if (strncmp(usb_mode, HOST_MODE_STR, max(sizeof(HOST_MODE_STR), size)) == 0) {
		if (gpio_is_valid(vxboot_gpio))
			gpio_direction_output(vxboot_gpio, 1);
		// device out
		pd_event_notify(TCA_IRQ_HPD_OUT, TCPC_NC, TCA_CHARGER_DISCONNECT_EVENT, TYPEC_ORIEN_POSITIVE);
		// host in
		pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_AND_DP_2LINE, TCA_ID_FALL_EVENT, TYPEC_ORIEN_POSITIVE);
		pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_AND_DP_2LINE, TCA_DP_IN, TYPEC_ORIEN_POSITIVE);
		pr_info("[USBMODE] %s check_usbmode to host success!\n", __func__);
	} else if (strncmp(usb_mode, DEVICE_MODE_STR, max(sizeof(DEVICE_MODE_STR), size)) == 0) {
		if (gpio_is_valid(vxboot_gpio))
			gpio_direction_output(vxboot_gpio, 0);
		// host out
		pd_event_notify(TCA_IRQ_HPD_OUT, TCPC_NC, TCA_DP_OUT, TYPEC_ORIEN_POSITIVE);
		pd_event_notify(TCA_IRQ_HPD_OUT, TCPC_NC, TCA_ID_RISE_EVENT, TYPEC_ORIEN_POSITIVE);
		// device in
		pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_CONNECTED, TCA_CHARGER_CONNECT_EVENT, TYPEC_ORIEN_POSITIVE);
		pr_info("[USBMODE] %s check_usbmode to device success!\n", __func__);
	} else {
		pr_err("[USBMODE] %s invalid usbmode from user\n", __func__);
		return -EINVAL;
	}
	return size;
}

static ssize_t check_usbmode_read(struct file *file, char __user *ubuf, size_t size, loff_t *off)
{
	if ((file == NULL) || (ubuf == NULL)) {
		pr_err("[USBMODE] %s input parameter is invalid\n", __func__);
		return -EINVAL;
	}

	if (copy_to_user(ubuf, usb_mode, sizeof(usb_mode))) {
		pr_err("[USBMODE] %s copy_to_user failed\n", __func__);
		return -EINVAL;
	}

	pr_info("[USBMODE] %s read usbmode = %s success\n", __func__, usb_mode);
	return 0;
}

static int usb_in_out_notify(struct notifier_block *nb,
				      unsigned long val, void *data)
{
	struct power_supply *psy = data;
	union power_supply_propval propval;
	if (psy == NULL) {
		pr_err("[USBMODE] %s psy is null\n", __func__);
		return NOTIFY_DONE;
	}
	if ((val != PSY_EVENT_PROP_CHANGED) || (psy->desc->type != POWER_SUPPLY_TYPE_MAINS))
		return NOTIFY_DONE;

	if (strncmp(usb_mode, DEVICE_MODE_STR, max(sizeof(DEVICE_MODE_STR), sizeof(usb_mode))) == 0) {
		if (power_supply_get_property(psy, POWER_SUPPLY_PROP_ONLINE, &propval) != EOK) {
			pr_err("[USBMODE] %s power_supply_get_property fail\n", __func__);
			return NOTIFY_DONE;
		}
		pr_info("[USBMODE] %s power_supply_get_property propval.intval = %d\n", __func__, propval.intval);

		switch (propval.intval) {
		case DEVICE_USB_OUT:
			pd_event_notify(TCA_IRQ_HPD_OUT, TCPC_NC, TCA_CHARGER_DISCONNECT_EVENT, TYPEC_ORIEN_POSITIVE);
			pr_info("[USBMODE] %s device -> off\n", __func__);
			break;
		case DEVICE_USB_IN:
			pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_CONNECTED, TCA_CHARGER_CONNECT_EVENT, TYPEC_ORIEN_POSITIVE);
			pr_info("[USBMODE] %s off -> device\n", __func__);
			break;
		default:
			pr_err("[USBMODE] %s propval.intval = %d is invalid\n", __func__, propval.intval);
			break;
		}
	} else {
		pr_err("[USBMODE] %s usbmode is invalid\n", __func__);
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}

static struct file_operations check_usbmode_fops = {
	.owner = THIS_MODULE,
	.open = check_usbmode_open,
	.write = check_usbmode_write,
	.read = check_usbmode_read,
};

static struct notifier_block usb_in_out_notifier = {
	.notifier_call = usb_in_out_notify,
};

static int read_init_mode(struct device_node *node, char *str)
{
	const char *init_mode = NULL;
	int mode;
	if (of_property_read_string(node, str, &init_mode) != EOK) {
		pr_err("[USBMODE] %s init-mode property is not found\n", __func__);
		return COMBOPHY_INIT_DEFAULT_MODE;
	}

	if (strncmp(init_mode, DTS_HOST_MODE_STR, sizeof(DTS_HOST_MODE_STR)) == 0)
		mode = COMBOPHY_INIT_USB_HOST_MODE;
	else if (strncmp(init_mode, DTS_DEVICE_MODE_STR, sizeof(DTS_DEVICE_MODE_STR)) == 0)
		mode = COMBOPHY_INIT_USB_DEVICE_MODE;
	else
		mode = COMBOPHY_INIT_DEFAULT_MODE;
	
	pr_info("[USBMODE] %s read mode = %d success\n", __func__, mode);
	return mode;
}

static int check_usbmode_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	int mode;

	major = register_chrdev(0, "check_usbmode", &check_usbmode_fops);
	class = class_create(THIS_MODULE, "check_usbmode");
	device_create(class, NULL, MKDEV(major, 0), NULL, "check_usbmode");

	vxboot_gpio = of_get_named_gpio(node, "vxboot_gpio", 0);
	pr_info("[USBMODE] %s: vxboot_gpio %d.\n", __func__, vxboot_gpio);
	if (gpio_is_valid(vxboot_gpio))
		gpio_request(vxboot_gpio, "usbmode");

	mode = read_init_mode(node, "init-mode");
	if (mode == COMBOPHY_INIT_USB_HOST_MODE) {
		if (gpio_is_valid(vxboot_gpio))
			gpio_direction_output(vxboot_gpio, 1);

		pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_AND_DP_2LINE, TCA_ID_FALL_EVENT, TYPEC_ORIEN_POSITIVE);
		pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_AND_DP_2LINE, TCA_DP_IN, TYPEC_ORIEN_POSITIVE);

		if (strcpy_s(usb_mode, sizeof(usb_mode), HOST_MODE_STR) != EOK) {
			pr_err("[USBMODE] %s strcpy_s usb_mode to host fail.\n", __func__);
			return -EINVAL;
		}
	} else if (mode == COMBOPHY_INIT_USB_DEVICE_MODE) {
		if (gpio_is_valid(vxboot_gpio))
			gpio_direction_output(vxboot_gpio, 0);

		pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_CONNECTED, TCA_CHARGER_CONNECT_EVENT, TYPEC_ORIEN_POSITIVE);

		if (strcpy_s(usb_mode, sizeof(usb_mode), DEVICE_MODE_STR) != EOK) {
			pr_err("[USBMODE] %s strcpy_s usb_mode to device fail.\n", __func__);
			return -EINVAL;
		}
	}
	pr_info("[USBMODE] %s usb mode init success, init-mode:%d\n", __func__, mode);

	if (power_supply_reg_notifier(&usb_in_out_notifier) != EOK) {
		pr_err("[USBMODE] %s usb_in_out_notifier reg failed\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int check_usbmode_remove(struct platform_device *pdev)
{
	pr_info("[USBMODE] check usb mode remove\n");
	return 0;
}

static const struct of_device_id check_usbmode_match_table[] = {
	{ .compatible = "huawei,check_usbmode", },
	{},
};
MODULE_DEVICE_TABLE(of, check_usbmode_match_table);

static struct platform_driver check_usbmode_driver = {
	.probe = check_usbmode_probe,
	.remove = check_usbmode_remove,
	.driver = {
		.name = "huawei,check_usbmode",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(check_usbmode_match_table),
	},
};

static int __init check_usbmode_init(void)
{
	return platform_driver_register(&check_usbmode_driver);
}

static void __exit check_usbmode_exit(void)
{
	platform_driver_unregister(&check_usbmode_driver);
}

device_initcall_sync(check_usbmode_init);
module_exit(check_usbmode_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("check usbmode");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

