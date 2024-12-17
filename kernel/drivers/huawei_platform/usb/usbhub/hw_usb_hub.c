/*
 * hw_usb_hub.c
 *
 * usb hub driver
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/usb.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/i2c.h>
#include <linux/syscore_ops.h>
#include <dfx/hiview_hisysevent.h>
#include <linux/errno.h>
#include <linux/printk.h>
#include <linux/time64.h>
#include <linux/workqueue.h>

#define pr_fmt(fmt) "[usb_hub]: " fmt

#define GPIO_LOW 0
#define GPIO_HIGH 1
#define MAX_CMD_NUM 6
#define MAX_HUB_RESET_TIMES 3

/* add regulator i2c transfer define */
#define REG_SEND_ADDR		0x38
#define REG_SEND_ADAPTER    0x07

#define ECHUB_SEND_BYTE_LEN		0x04
#define BIT_OFFSET				0x08
#define REG_DATE_LEN 			0x01
#define ECHUB_SEND_TRANS_LEN	0x02

#define SEC_TO_MILLISEC 1000
#define MILLISEC_TO_NANOSEC (1000 * 1000)

struct usb_hub {
	struct device *dev;

	/* Usb-related power gpio info */
	u32 hub_bpwr_gpio;
	u32 back_hub_reset_gpio;
	u32 usbhub_vdd_on_gpio;
	u32 usbhub_1v05_gpio;
	u32 usbcam_pwr_en_gpio;
	u32 usbnet_power_gpio;

	/* Save the usb camera power gpio status set in the BIOS phase */
	u32 bios_usbcam_pwr_status;
	u32 bios_usbnet_pwr_status;

	/* Some delay time required by the timing */
	u32 reset_or_bpwr_delay_time;
	u32 hub_1v05_gpio_delay_time;
	u32 hub_vdd_gpio_delay_time;
	u32 hub_cam_gpio_delay_time;
	u32 hub_net_gpio_delay_time;
	u32 usb_device_poweron_wait_time;
	u32 poweroff_time;
	u32 poweron_time;

	/* Info used to control the power supply on the EC */
	u32 ec_addr;
	u32 ec_cmd_nums;
	u32 ec_cmd_off_list[MAX_CMD_NUM];
	u32 ec_cmd_on_list[MAX_CMD_NUM];
	u32 ec_cmd_delay_time_list[MAX_CMD_NUM];

	/* Flag indicating whether S3 is powered off */
	u32 s3_is_off;

	/* Usb-related sysfs class */
	struct class *usb_hub_class;

	struct notifier_block usbhub_nb;
	int notify_bus_add_count;

	struct delayed_work power_work;
};

static struct usb_hub *g_uhub;

static int echub_i2c_write_regulator(u16 regoffset, char regdata)
{
	int ret;
	struct i2c_adapter *adap;
	struct i2c_msg msg[2];
	unsigned char data[4];
	unsigned char value[2];
	const unsigned char devaddr = REG_SEND_ADDR;

	adap = i2c_get_adapter(REG_SEND_ADAPTER);
	/* check adapter is valid */
	if (adap == NULL) {
		pr_err("%s adap error\n", __func__);
		return -ENODEV;
	}

	/* write data buffer */
	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_SEND_BYTE_LEN;
	msg[0].buf = data;
	data[0] = (u8)(regoffset >> BIT_OFFSET);
	data[1] = (u8)(regoffset & 0xFF);
	data[2] = REG_DATE_LEN;
	data[3] = regdata;

	/* read data buffer */
	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = ECHUB_SEND_TRANS_LEN;
	msg[1].buf = value;

	ret = i2c_transfer(adap, msg, ECHUB_SEND_TRANS_LEN);
	i2c_put_adapter(adap);

	/* check i2c transfer result */
	if (ret != ECHUB_SEND_TRANS_LEN) {
		pr_err("%s TRANS_LEN error\n", __func__);
		return -ENODEV;
	}
	if (value[0]) {
		pr_err("%s value[0] error\n", __func__);
		return -EIO;
	}

	return 0;
}

static int hw_usb_device_power_on(const struct usb_hub *uhub)
{
	if (uhub == NULL)
		return -ENODEV;

	pr_info("%s ++\n", __func__);
	if (uhub->usbcam_pwr_en_gpio > 0)
		gpio_set_value(uhub->usbcam_pwr_en_gpio, uhub->bios_usbcam_pwr_status);
	if (uhub->hub_cam_gpio_delay_time > 0)
		msleep(uhub->hub_cam_gpio_delay_time);

	if (uhub->usbnet_power_gpio > 0)
		gpio_set_value(uhub->usbnet_power_gpio, uhub->bios_usbnet_pwr_status);
	if (uhub->hub_net_gpio_delay_time > 0)
		msleep(uhub->hub_net_gpio_delay_time);
	pr_info("%s --\n", __func__);

	return 0;
}

static int hw_usb_device_power_off(const struct usb_hub *uhub)
{
	int ret;

	if (uhub == NULL)
		return -ENODEV;

	pr_info("%s ++\n", __func__);
	if (uhub->usbcam_pwr_en_gpio > 0) {
		ret = gpio_direction_output(uhub->usbcam_pwr_en_gpio, GPIO_LOW);
		if (ret)
			pr_err("could not set gpio %d as output, ret = %d\n", uhub->usbcam_pwr_en_gpio, ret);
	}
	if (uhub->hub_cam_gpio_delay_time > 0)
		msleep(uhub->hub_cam_gpio_delay_time);

	if (uhub->usbnet_power_gpio > 0) {
		ret = gpio_direction_output(uhub->usbnet_power_gpio, GPIO_LOW);
		if (ret)
			pr_err("could not set gpio %d as output, ret = %d\n", uhub->usbnet_power_gpio, ret);
	}
	if (uhub->hub_net_gpio_delay_time > 0)
		msleep(uhub->hub_net_gpio_delay_time);

	pr_info("%s --\n", __func__);
	return ret;
}

static void usb_device_poweron_work(struct work_struct *work)
{
	int ret;
	struct usb_hub *uhub = container_of(work, struct usb_hub, power_work.work);

	if (uhub) {
		ret = hw_usb_device_power_on(uhub);
		if (ret)
			pr_err("usb device power on failed\n");
	}
}

static int hw_usb_hub_power_on(const struct usb_hub *uhub)
{
	int i, ret;

	if (uhub == NULL)
		return -ENODEV;

	pr_info("%s ++\n", __func__);
	if (uhub->ec_addr > 0 && uhub->ec_cmd_nums > 0) {
		pr_info("ec register is %x\n", uhub->ec_addr);
		for (i = 0; i < uhub->ec_cmd_nums && uhub->ec_cmd_on_list[i] > 0; i++) {
			pr_info("ec_cmd_on_list[%d] = %x\n", i, uhub->ec_cmd_on_list[i]);
			ret = echub_i2c_write_regulator(uhub->ec_addr, uhub->ec_cmd_on_list[i]);
			if (ret)
				pr_err("power on error, cmd is %x, ret = %d\n", uhub->ec_cmd_on_list[i], ret);
			if (uhub->ec_cmd_delay_time_list[i] > 0)
				msleep(uhub->ec_cmd_delay_time_list[i]);
		}
	}

	if (uhub->usbhub_1v05_gpio > 0)
		gpio_set_value(uhub->usbhub_1v05_gpio, GPIO_HIGH);
	if (uhub->hub_1v05_gpio_delay_time > 0)
		msleep(uhub->hub_1v05_gpio_delay_time);

	if (uhub->usbhub_vdd_on_gpio > 0)
		gpio_set_value(uhub->usbhub_vdd_on_gpio, GPIO_HIGH);
	if (uhub->hub_vdd_gpio_delay_time > 0)
		msleep(uhub->hub_vdd_gpio_delay_time);

	if (uhub->poweron_time > 0)
		msleep(uhub->poweron_time);
	pr_info("%s --\n", __func__);

	return 0;
}

static int hw_usb_hub_power_off(const struct usb_hub *uhub)
{
	int i, ret;

	if (uhub == NULL)
		return -ENODEV;

	pr_info("%s ++\n", __func__);
	if (uhub->ec_addr > 0 && uhub->ec_cmd_nums > 0) {
		pr_info("ec register is %x\n", uhub->ec_addr);
		for (i = 0; i < uhub->ec_cmd_nums && uhub->ec_cmd_off_list[i] > 0; i++) {
			pr_info("ec_cmd_off_list[%d] = %x\n", i, uhub->ec_cmd_off_list[i]);
			ret = echub_i2c_write_regulator(uhub->ec_addr, uhub->ec_cmd_off_list[i]);
			if (ret)
				pr_err("power off error, cmd is %x, ret = %d\n", uhub->ec_cmd_off_list[i], ret);
			if (uhub->ec_cmd_delay_time_list[i] > 0)
				msleep(uhub->ec_cmd_delay_time_list[i]);
		}
	}

	ret = hw_usb_device_power_off(uhub);
	if (ret)
		pr_err("usb devices poweroff failed\n");

	if (uhub->usbhub_1v05_gpio > 0) {
		ret = gpio_direction_output(uhub->usbhub_1v05_gpio, GPIO_LOW);
		if (ret)
			pr_err("could not set gpio %d as output, ret = %d\n", uhub->usbhub_1v05_gpio, ret);
	}
	if (uhub->hub_1v05_gpio_delay_time > 0)
		msleep(uhub->hub_1v05_gpio_delay_time);

	if (uhub->usbhub_vdd_on_gpio > 0) {
		ret = gpio_direction_output(uhub->usbhub_vdd_on_gpio, GPIO_LOW);
		if (ret)
			pr_err("could not set gpio %d as output, ret = %d\n", uhub->usbhub_vdd_on_gpio, ret);
	}
	if (uhub->hub_vdd_gpio_delay_time > 0)
		msleep(uhub->hub_vdd_gpio_delay_time);

	if (uhub->poweroff_time > 0)
		msleep(uhub->poweroff_time);
	pr_info("%s --\n", __func__);

	return 0;
}

static int hw_usb_hub_reset(struct usb_hub *uhub)
{
	int ret;

	if (uhub == NULL)
		return -ENODEV;

	pr_info("%s ++\n", __func__);
	if (uhub->hub_bpwr_gpio > 0) {
		ret = gpio_direction_output(uhub->hub_bpwr_gpio, GPIO_HIGH);
		if (ret)
			pr_err("could not set gpio %d as output, ret = %d\n", uhub->hub_bpwr_gpio, ret);
	}
	if (uhub->back_hub_reset_gpio > 0) {
		ret = gpio_direction_output(uhub->back_hub_reset_gpio, GPIO_LOW);
		if (ret)
			pr_err("could not set gpio %d as output, ret = %d\n", uhub->back_hub_reset_gpio, ret);
	}

	if (uhub->reset_or_bpwr_delay_time > 0)
		msleep(uhub->reset_or_bpwr_delay_time);

	pr_info("%s --\n", __func__);

	return 0;
}

static int hw_usb_hub_dereset(struct usb_hub *uhub)
{
	if (uhub == NULL) {
		pr_err("%s, uhub is NULL\n", __func__);
		return -ENODEV;
	}

	pr_info("%s ++\n", __func__);

	if (uhub->hub_bpwr_gpio > 0)
		gpio_set_value(uhub->hub_bpwr_gpio, GPIO_LOW);
	if (uhub->back_hub_reset_gpio > 0)
		gpio_set_value(uhub->back_hub_reset_gpio, GPIO_HIGH);
	if (uhub->reset_or_bpwr_delay_time > 0)
		msleep(uhub->reset_or_bpwr_delay_time);

	pr_info("%s --\n", __func__);

	return 0;
}

static void force_usb_reset(struct usb_hub *uhub)
{
	if (hw_usb_hub_reset(uhub))
		pr_err("usb hub reset failed\n");

	if (hw_usb_hub_dereset(uhub))
		pr_err("usb hub dereset failed\n");
}

static ssize_t hub_reset_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t val = 0;
	int hub_reset_times = 0;

	if (g_uhub == NULL) {
		pr_err("g_uhub is null\n");
		return -1;
	}

	if (kstrtoull(buf, 10, &val) != 0) {
		pr_err("invalid parameters!\n");
		return -EINVAL;
	}

	if (val == 1 && hub_reset_times < MAX_HUB_RESET_TIMES) {
		force_usb_reset(g_uhub);
		hub_reset_times++;
		pr_info("hub_port_reset\n");
	}

	return count;
}

static DEVICE_ATTR(hub_reset, 0644, NULL, hub_reset_store);
static struct attribute *usb_hub_ctrl_attributes[] = {
	&dev_attr_hub_reset.attr,
	NULL
};

static const struct attribute_group usb_hub_attr_group = {
	.attrs = usb_hub_ctrl_attributes,
};

static int hub_port_setup_sysfs(struct usb_hub *uhub)
{
	int ret;

	if (uhub == NULL)
		return -ENODEV;

	uhub->usb_hub_class = class_create(THIS_MODULE, "usb_hub");
	if (IS_ERR(uhub->usb_hub_class)) {
		pr_err("cannot create class!\n");
		ret = PTR_ERR(uhub->usb_hub_class);
		goto fail_create_class;
	}

	uhub->dev = device_create(uhub->usb_hub_class, NULL, 0, NULL, "hub");
	if (IS_ERR(uhub->dev)) {
		pr_err("sysfs device create failed!\n");
		ret = PTR_ERR(uhub->dev);
		goto fail_create_device;
	}

	ret = sysfs_create_group(&uhub->dev->kobj, &usb_hub_attr_group);
	if (ret < 0) {
		pr_err("sysfs group create failed! ret = %d\n", ret);
		goto fail_create_sysfs;
	}
	return ret;

fail_create_sysfs:
	device_destroy(uhub->usb_hub_class, uhub->dev->devt);

fail_create_device:
	class_destroy(uhub->usb_hub_class);
	uhub->usb_hub_class = NULL;

fail_create_class:
	return ret;
}

static void hub_port_destroy_sysfs(const struct platform_device *pdev)
{
	struct usb_hub *uhub = platform_get_drvdata(pdev);

	if (uhub == NULL)
		return;

	sysfs_remove_group(&uhub->dev->kobj, &usb_hub_attr_group);

	device_destroy(uhub->usb_hub_class, uhub->dev->devt);

	class_destroy(uhub->usb_hub_class);
	uhub->usb_hub_class = NULL;
}

static int of_hub_init_power_gpios(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct usb_hub *uhub = platform_get_drvdata(pdev);

	if (uhub == NULL)
		return -ENODEV;

	if (of_property_read_u32(np, "hub_bpwr_gpio", &uhub->hub_bpwr_gpio))
		pr_info("can't get hub_bpwr_gpio.\n");
	else if (devm_gpio_request(&pdev->dev, uhub->hub_bpwr_gpio, "hub_bpwr_gpio"))
		pr_err("request hub_bpwr_gpio %d failed.\n", uhub->hub_bpwr_gpio);

	if (of_property_read_u32(np, "back_hub_reset_gpio", &uhub->back_hub_reset_gpio))
		pr_info("can't get back_hub_reset_gpio.\n");
	else if (devm_gpio_request(&pdev->dev, uhub->back_hub_reset_gpio, "back_hub_reset_gpio"))
		pr_err("request back_hub_reset_gpio %d failed.\n", uhub->back_hub_reset_gpio);

	if (of_property_read_u32(np, "usbhub_vdd_on_gpio", &uhub->usbhub_vdd_on_gpio))
		pr_info("can't get usbhub_vdd_on_gpio.\n");
	else if (devm_gpio_request(&pdev->dev, uhub->usbhub_vdd_on_gpio, "usbhub_vdd_on_gpio"))
		pr_err("request usbhub_vdd_on_gpio %d failed.\n", uhub->usbhub_vdd_on_gpio);

	if (of_property_read_u32(np, "usbhub_1v05_gpio", &uhub->usbhub_1v05_gpio))
		pr_info("can't get usbhub_1v05_gpio.\n");
	else if (devm_gpio_request(&pdev->dev, uhub->usbhub_1v05_gpio, "usbhub_1v05_gpio"))
		pr_err("request usbhub_1v05_gpio %d failed.\n", uhub->usbhub_1v05_gpio);

	if (of_property_read_u32(np, "usbcam_pwr_en_gpio", &uhub->usbcam_pwr_en_gpio))
		pr_info("can't get usbcam_pwr_en_gpio.\n");
	else if (devm_gpio_request(&pdev->dev, uhub->usbcam_pwr_en_gpio, "usbcam_pwr_en_gpio"))
		pr_err("request usbcam_pwr_en_gpio %d failed.\n", uhub->usbcam_pwr_en_gpio);

	if (of_property_read_u32(np, "usbnet_power_gpio", &uhub->usbnet_power_gpio))
		pr_info("can't get usbnet_power_gpio.\n");
	else if (devm_gpio_request(&pdev->dev, uhub->usbnet_power_gpio, "usbnet_power_gpio"))
		pr_err("request usbnet_power_gpio %d failed.\n", uhub->usbnet_power_gpio);

	int usbcam_init_state = 0;
	if (of_property_read_u32(np, "usbcam_init_state", &usbcam_init_state))
		uhub->bios_usbcam_pwr_status = gpio_get_value(uhub->usbcam_pwr_en_gpio);
	else
		uhub->bios_usbcam_pwr_status = usbcam_init_state;

	int usbnet_init_state = 0;
	if (of_property_read_u32(np, "usbnet_init_state", &usbnet_init_state))
		uhub->bios_usbnet_pwr_status = gpio_get_value(uhub->usbnet_power_gpio);
	else
		uhub->bios_usbnet_pwr_status = usbnet_init_state;

	return 0;
}

static int of_hub_init_power_ec(const struct platform_device *pdev)
{
	int ret;
	struct device_node *np = pdev->dev.of_node;
	struct usb_hub *uhub = platform_get_drvdata(pdev);

	if (uhub == NULL)
		return -ENODEV;

	ret = of_property_read_u32(np, "ec_addr", &uhub->ec_addr);
	if (ret) {
		pr_info("can't get ec_addr, ret = %d\n", ret);
		return ret;
	}

	ret = of_property_read_u32(np, "ec_cmd_nums", &uhub->ec_cmd_nums);
	if (ret) {
		pr_err("can't get ec_cmd_nums, ret = %d\n", ret);
		return ret;
	}

	if (uhub->ec_cmd_nums > MAX_CMD_NUM) {
		pr_err("ec_cmd_nums Overflow in dts!\n");
		uhub->ec_cmd_nums = MAX_CMD_NUM;
	}

	if (uhub->ec_cmd_nums <= 0) {
		pr_err("ec_cmd_nums is invalid: %d\n", uhub->ec_cmd_nums);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "ec_cmd_off_list", &uhub->ec_cmd_off_list[0], uhub->ec_cmd_nums);
	if (ret) {
		pr_err("can't get ec_cmd_off_list, ret = %d\n", ret);
		return ret;
	}
	ret = of_property_read_u32_array(np, "ec_cmd_on_list", &uhub->ec_cmd_on_list[0], uhub->ec_cmd_nums);
	if (ret) {
		pr_err("can't get ec_cmd_on_list, ret = %d\n", ret);
		return ret;
	}
	ret = of_property_read_u32_array(np, "ec_cmd_delay_time_list", &uhub->ec_cmd_delay_time_list[0], uhub->ec_cmd_nums);
	if (ret)
		pr_err("can't get ec_cmd_delay_time_list, ret = %d\n", ret);

	return 0;
}

static int of_hub_init_timing_paras(const struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct usb_hub *uhub = platform_get_drvdata(pdev);

	if (uhub == NULL)
		return -ENODEV;

	if (of_property_read_u32(np, "reset_or_bpwr_delay_time", &uhub->reset_or_bpwr_delay_time))
		pr_info("can't get reset_or_bpwr_delay_time.\n");

	if (of_property_read_u32(np, "hub_1v05_gpio_delay_time", &uhub->hub_1v05_gpio_delay_time))
		pr_info("can't get hub_1v05_gpio_delay_time.\n");

	if (of_property_read_u32(np, "hub_vdd_gpio_delay_time", &uhub->hub_vdd_gpio_delay_time))
		pr_info("can't get hub_vdd_gpio_delay_time.\n");

	if (of_property_read_u32(np, "hub_cam_gpio_delay_time", &uhub->hub_cam_gpio_delay_time))
		pr_info("can't get hub_cam_gpio_delay_time.\n");

	if (of_property_read_u32(np, "hub_net_gpio_delay_time", &uhub->hub_net_gpio_delay_time))
		pr_info("can't get hub_net_gpio_delay_time.\n");

	if (of_property_read_u32(np, "usb_device_poweron_wait_time", &uhub->usb_device_poweron_wait_time))
		pr_info("can't get usb_device_poweron_wait_time.\n");

	if (of_property_read_u32(np, "poweroff_time", &uhub->poweroff_time))
		pr_info("can't get poweroff_time.\n");

	if (of_property_read_u32(np, "poweron_time", &uhub->poweron_time))
		pr_info("can't get poweron_time\n");

	if (of_property_read_u32(np, "s3_is_off", &uhub->s3_is_off))
		pr_info("can't get s3_is_off.\n");

	return 0;
}

static int of_usb_hub_init(struct platform_device *pdev)
{
	int ret;

	ret = of_hub_init_power_gpios(pdev);
	if (ret)
		pr_err("Init power gpios failed, ret = %d\n", ret);

	ret = of_hub_init_power_ec(pdev);
	if (ret)
		pr_err("Init power ec failed: %d, no ec parameters?\n", ret);

	ret = of_hub_init_timing_paras(pdev);
	if (ret)
		pr_err("Init timing paras failed, ret = %d\n", ret);

	return ret;
}

static int usbhub_dev_notify(struct notifier_block *self, unsigned long action, void *dev)
{
	int ret = 0;
	struct usb_hub *uhub = container_of(self, struct usb_hub, usbhub_nb);

	switch (action) {
	case USB_BUS_ADD:
		if (uhub->notify_bus_add_count <= 0) {
			ret = hw_usb_hub_power_on(uhub);
			if (ret)
				pr_err("usb hub poweron failed: %d\n", ret);
			ret = hw_usb_hub_dereset(uhub);
			if (ret)
				pr_err("usb hub dereset failed: %d\n", ret);
			schedule_delayed_work(&uhub->power_work,
				msecs_to_jiffies(uhub->usb_device_poweron_wait_time));
			uhub->notify_bus_add_count++;
		}
		break;
	case USB_BUS_REMOVE:
		/* fall-through */
	default:
		break;
	}
	return ret;
}

int hiview_hisysevent_pm_event(const char *name, const char *key)
{
	struct hiview_hisysevent *event = NULL;
	int ret = 0;
	int len_written = 0;
	struct timespec64 ts;
	long long millisecs = 0;

	pr_info("%s ++\n", __func__);

	if (name == NULL || key == NULL) {
		pr_err("name or key is null\n");
		return -EINVAL;
	}

	ktime_get_ts64(&ts);
	millisecs = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;

	event = hisysevent_create("KERNEL_WAKEUP", name, BEHAVIOR);
	if (event == NULL) {
		pr_err("failed to create event %s\n", name);
		return -EINVAL;
	}
	ret = hisysevent_put_integer(event, key, millisecs);
	if (ret != 0) {
		pr_err("failed to put key %s to event, ret=%d\n", key, ret);
		goto hisysevent_end;
	}
	len_written = hisysevent_write(event);
	if (len_written < 0) {
		ret = len_written;
		pr_err("failed to write to event, key %s, ret=%d\n", key, ret);
	}

hisysevent_end:
	hisysevent_destroy(&event);
	pr_info("%s --\n", __func__);

	return ret;
}

static int hw_usb_hub_pm_notify(struct notifier_block *notify_block,
			  unsigned long mode, void *unused)
{
	pr_info("%s get in", __func__);

	if (g_uhub == NULL) {
		pr_err("g_uhub is null\n");
		return -1;
	}

	switch (mode) {
	case PM_SUSPEND_PREPARE:
		pr_info("%s get in PM_SUSPEND_PREPARE", __func__);
		hw_usb_hub_power_off(g_uhub);
		break;
	case PM_POST_SUSPEND:
		pr_info("%s get in PM_POST_SUSPEND", __func__);
		hw_usb_hub_power_on(g_uhub);
		schedule_delayed_work(&g_uhub->power_work,
			msecs_to_jiffies(g_uhub->usb_device_poweron_wait_time));
		(void)hiview_hisysevent_pm_event("LID_WAKEUP_END", "KERNEL_END_TIME");
		break;
	default:
		break;
	}
	return 0;
}

static struct notifier_block hw_usb_hub_pm_notifier = {
	.notifier_call = hw_usb_hub_pm_notify,
	.priority = INT_MAX,
};

static void hw_usb_hub_syscore_resume(void)
{
	pr_info("%s\n", __func__);

	(void)hiview_hisysevent_pm_event("LID_WAKEUP_START", "KERNEL_START_TIME");
}

static struct syscore_ops hw_usb_hub_syscore_ops = {
	.resume  = hw_usb_hub_syscore_resume,
};

static int usb_hub_probe(struct platform_device *pdev)
{
	int ret;
	struct usb_hub *uhub = NULL;

	pr_info("%s ++\n", __func__);
	uhub = devm_kzalloc(&pdev->dev, sizeof(struct usb_hub), GFP_KERNEL);
	if (uhub == NULL)
		return -ENOMEM;

	uhub->dev = &pdev->dev;
	platform_set_drvdata(pdev, uhub);

	ret = of_usb_hub_init(pdev);
	if (ret)
		pr_err("usb hub init failed: %d\n", ret);

	ret = hw_usb_hub_reset(uhub);
	if (ret)
		pr_err("usb hub reset failed: %d\n", ret);

	ret = hw_usb_hub_power_off(uhub);
	if (ret)
		pr_err("usb hub poweroff failed: %d\n", ret);

	uhub->notify_bus_add_count = 0;
	uhub->usbhub_nb.notifier_call = usbhub_dev_notify;
	usb_register_notify(&uhub->usbhub_nb);

	INIT_DELAYED_WORK(&uhub->power_work, usb_device_poweron_work);

	ret = hub_port_setup_sysfs(uhub);
	if (ret)
		pr_err("setup sysfs failed: %d\n", ret);

	register_pm_notifier(&hw_usb_hub_pm_notifier);
	register_syscore_ops(&hw_usb_hub_syscore_ops);

	g_uhub = uhub;
	pr_info("%s --\n", __func__);

	return 0;
}

static int usb_hub_remove(struct platform_device *pdev)
{
	pr_info("%s ++\n", __func__);
	struct usb_hub *uhub = platform_get_drvdata(pdev);

	usb_unregister_notify(&uhub->usbhub_nb);
	hub_port_destroy_sysfs(pdev);
	unregister_pm_notifier(&hw_usb_hub_pm_notifier);
	cancel_delayed_work(&uhub->power_work);

	pr_info("%s --\n", __func__);

	return 0;
}

static int usb_hub_pm_suspend(struct device *dev)
{
	int ret;
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hub *uhub = platform_get_drvdata(pdev);

	if (uhub == NULL)
		return -ENODEV;

	if (uhub->s3_is_off) {
		pr_info("%s ++\n", __func__);
		ret = hw_usb_hub_reset(uhub);
		if (ret)
			pr_err("usb hub reset failed: %d\n", ret);
		ret = hw_usb_hub_power_off(uhub);
		if (ret)
			pr_err("usb hub poweroff failed: %d\n", ret);
		pr_info("%s --\n", __func__);
	}
	return 0;
}

static int usb_hub_pm_resume(struct device *dev)
{
	int ret;
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hub *uhub = platform_get_drvdata(pdev);

	if (uhub == NULL)
		return -ENODEV;

	if (uhub->s3_is_off) {
		pr_info("%s ++\n", __func__);
		ret = hw_usb_hub_power_on(uhub);
		if (ret)
			pr_err("usb hub poweron failed: %d\n", ret);
		ret = hw_usb_hub_dereset(uhub);
		if (ret)
			pr_err("usb hub dereset failed: %d\n", ret);
		schedule_delayed_work(&uhub->power_work,
			msecs_to_jiffies(uhub->usb_device_poweron_wait_time));
		pr_info("%s --\n", __func__);
	}
	return 0;
}

static void usb_hub_shutdown(struct platform_device *pdev)
{
	pr_info("usb hub shutdown\n");
	struct usb_hub *uhub = platform_get_drvdata(pdev);

	if (uhub == NULL)
		return;

	if (hw_usb_hub_reset(uhub))
		pr_err("usb hub reset failed\n");
	if (hw_usb_hub_power_off(uhub))
		pr_err("usb hub poweroff failed\n");
}

static const struct dev_pm_ops usb_hub_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(usb_hub_pm_suspend, usb_hub_pm_resume)
};

static const struct of_device_id hw_usb_hub_match[] = {
	{ .compatible = "huawei,hw_usb_hub" },
	{},
};

static struct platform_driver usb_hub_driver = {
	.probe = usb_hub_probe,
	.remove = usb_hub_remove,
	.driver = {
		.name = "hw_usb_hub",
		.owner = THIS_MODULE,
		.pm = &usb_hub_pm_ops,
		.of_match_table = of_match_ptr(hw_usb_hub_match),
	},
	.shutdown = usb_hub_shutdown,
};

static int __init hw_usb_hub_init(void)
{
	return platform_driver_register(&usb_hub_driver);
}

static void __exit hw_usb_hub_exit(void)
{
	platform_driver_unregister(&usb_hub_driver);
}

fs_initcall(hw_usb_hub_init);
module_exit(hw_usb_hub_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei usb hub module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
