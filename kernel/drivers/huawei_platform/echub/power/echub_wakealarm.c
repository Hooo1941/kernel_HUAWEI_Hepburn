/*
 * echub_wakealarm.c
 *
 * EC wakes up the system periodically when the system is powered off.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/mfd/core.h>
#include <linux/of.h>
#include <securec.h>
#include "../echub.h"
#include "../echub_i2c.h"

#define UPDATE_TIMESTAMP_ADDR   0x02BB
#define SET_WAKEALRAM_TIME_ADDR 0x0610

#define TIMESTAMP_LEN           4

struct echub_wakealarm_info {
	struct device *dev;
	struct echub_i2c_dev *echub_dev;
	unsigned int wakealarm_time;
};

static ssize_t echub_wakealarm_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct echub_wakealarm_info *ew = platform_get_drvdata(pdev);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "wakearlam time:%d\n", ew->wakealarm_time);
}

static ssize_t echub_wakealarm_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int ret;
	time64_t alarm;
	struct timespec64 now;
	unsigned char time_stamp[TIMESTAMP_LEN] = {0};

	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct echub_wakealarm_info *ew = platform_get_drvdata(pdev);

	ktime_get_real_ts64(&now);
	ret = kstrtos64(buf, 0, &alarm);
	if (ret < 0)
		return ret;

	if (alarm >= INT_MAX ||  now.tv_sec >= INT_MAX || alarm <= now.tv_sec)
		return -EINVAL;

	ew->wakealarm_time = alarm;
	time_stamp[0] = now.tv_sec & 0xFF;
	time_stamp[1] = (now.tv_sec >> 0x8) & 0xFF;
	time_stamp[2] = (now.tv_sec >> 0x10) & 0xFF;
	time_stamp[3] = (now.tv_sec >> 0x18) & 0xFF;
	ret = ew->echub_dev->transfer_func(ew->echub_dev, UPDATE_TIMESTAMP_ADDR, time_stamp, TIMESTAMP_LEN, NULL, 0x0);
	if (ret < 0) {
		echub_err("i2c write 0x%x fail \n", UPDATE_TIMESTAMP_ADDR);
		return ret;
	}

	/* delay 10ms, send set_wakealarm_time cmd */
	msleep(10);

	time_stamp[0] = alarm & 0xFF;
	time_stamp[1] = (alarm >> 0x8) & 0xFF;
	time_stamp[2] = (alarm >> 0x10) & 0xFF;
	time_stamp[3] = (alarm >> 0x18) & 0xFF;
	ret = ew->echub_dev->transfer_func(ew->echub_dev, SET_WAKEALRAM_TIME_ADDR, time_stamp, TIMESTAMP_LEN, NULL, 0x0);
	if (ret < 0) {
		echub_err("i2c write 0x%x fail \n", SET_WAKEALRAM_TIME_ADDR);
		return ret;
	}
	return count;
}
static DEVICE_ATTR(wakealarm, 0660, echub_wakealarm_show, echub_wakealarm_store);

static struct attribute *echub_wakealarm_attrs[] = {
	&dev_attr_wakealarm.attr,
	NULL,
};

static const struct attribute_group echub_wakealarm_sysfs_attr_group = {
	.attrs = echub_wakealarm_attrs,
};

static int echub_wakealarm_probe(struct platform_device *pdev)
{
	int ret;
	struct echub_i2c_dev *echub_dev = dev_get_drvdata(pdev->dev.parent);
	struct echub_wakealarm_info *ew = NULL;

	if (echub_dev == NULL) {
		echub_err("echub_dev is null, get echub_dev form tmp\n");
		echub_dev = get_echub_dev();
		if (echub_dev == NULL)
			return -ENODEV;
	}

	ew = devm_kzalloc(&pdev->dev, sizeof(struct echub_wakealarm_info), GFP_KERNEL);
	if (!ew)
		return -ENOMEM;

	ew->dev = &pdev->dev;
	ew->echub_dev = echub_dev;
	ew->wakealarm_time = 0x0;

	platform_set_drvdata(pdev, ew);

	ret = sysfs_create_group(&pdev->dev.kobj, &echub_wakealarm_sysfs_attr_group);
	if (ret < 0)
		echub_err("register sysfs echub wakealarm failed\n");

	return 0;
}

static int echub_wakealarm_remove(struct platform_device *pdev)
{
	struct echub_wakealarm_info *ew = platform_get_drvdata(pdev);

	if (ew == NULL) {
		echub_err("[%s]ew is NULL!\n", __func__);
		return -ENODEV;
	}
	sysfs_remove_group(&pdev->dev.kobj, &echub_wakealarm_sysfs_attr_group);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id of_echub_wakealarm_match_tbl[] = {
	{ .compatible = "huawei,echub_wakealarm" },
	{},
};

static struct platform_driver echub_wakealarm_driver = {
	.driver = {
		.name = "echub_wakealarm",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(of_echub_wakealarm_match_tbl),
	},
	.probe = echub_wakealarm_probe,
	.remove = echub_wakealarm_remove,
};

/**********************************************************
*  Function:       echub_wakealarm_init
*  Discription:    module initialization
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int __init echub_wakealarm_init(void)
{
	echub_info("echub wakealrm init\n");
	return platform_driver_register(&echub_wakealarm_driver);
}

/**********************************************************
*  Function:       echub_wakealarm_exit
*  Discription:    module initialization
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static void __exit echub_wakealarm_exit(void)
{
	echub_info("echub wakealrm exit\n");
	platform_driver_unregister(&echub_wakealarm_driver);
}

module_init(echub_wakealarm_init);
module_exit(echub_wakealarm_exit);

MODULE_DESCRIPTION("echub wakealarm init driver");
MODULE_LICENSE("GPL");
