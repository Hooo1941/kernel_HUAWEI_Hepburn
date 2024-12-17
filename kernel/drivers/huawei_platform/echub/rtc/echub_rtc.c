/*
 * echub_rtc.c
 *
 * The RTC driver for EC's or MCU's rtc
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

#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/rtc.h>
#include <linux/bcd.h>

#include "../echub.h"
#include "../echub_i2c.h"

#define CMD_GET_TIME 0x0218
#define CMD_SET_TIME 0x0208

struct echub_rtc_data {
	struct echub_i2c_dev *echub_dev;
	struct rtc_device *rtc;
};

static int echub_rtc_get_time(struct device *dev, struct rtc_time *tm)
{
	int ret;
	unsigned char rb[7] = {0};
	struct echub_rtc_data *echub_rtc = dev_get_drvdata(dev);

	if (echub_rtc == NULL) {
		echub_err("Get echub_rtc_data failed while get mcu rtc time\n");
		return -EINVAL;
	}

	ret = echub_rtc->echub_dev->transfer_func(echub_rtc->echub_dev, CMD_GET_TIME, NULL, 0, rb, sizeof(rb));
	if (ret) {
		echub_err("mcu rtc read time failed, ret = %d\n", ret);
		return ret;
	}
	tm->tm_sec = bcd2bin(rb[0]);
	tm->tm_min = bcd2bin(rb[1]);
	tm->tm_hour = bcd2bin(rb[2]);
	tm->tm_wday = bcd2bin(rb[3]);
	tm->tm_mday = bcd2bin(rb[4]);
	tm->tm_mon = bcd2bin(rb[5]) - 1;
	tm->tm_year = bcd2bin(rb[6]) + 100;

	return 0;
}

static int echub_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	int ret;
	unsigned char wb[7] = {0};
	struct echub_rtc_data *echub_rtc = dev_get_drvdata(dev);

	if (echub_rtc == NULL) {
		echub_err("Get echub_rtc_data failed while set mcu rtc time\n");
		return -EINVAL;
	}

	wb[0] = bin2bcd(tm->tm_sec);
	wb[1] = bin2bcd(tm->tm_min);
	wb[2] = bin2bcd(tm->tm_hour);
	wb[3] = bin2bcd(tm->tm_wday);
	wb[4] = bin2bcd(tm->tm_mday);
	wb[5] = bin2bcd(tm->tm_mon + 1);
	wb[6] = bin2bcd(tm->tm_year - 100);

	ret = echub_rtc->echub_dev->transfer_func(echub_rtc->echub_dev, CMD_SET_TIME, wb, sizeof(wb), NULL, 0);
	if (ret) {
		echub_err("mcu rtc set time failed, ret = %d\n", ret);
		return ret;
	}

	return 0;
}

static struct rtc_class_ops echub_rtc_ops = {
	.read_time = echub_rtc_get_time,
	.set_time = echub_rtc_set_time,
};

static int echub_rtc_probe(struct platform_device *pdev)
{
	int ret;
	struct echub_rtc_data *echub_rtc;
	struct echub_i2c_dev *echub_dev = dev_get_drvdata(pdev->dev.parent);

	echub_info("Echub rtc probe enter\n");

	if (echub_dev == NULL) {
		echub_err("echub_dev is null, get echub_dev form tmp\n");
		echub_dev = get_echub_dev();
		if (echub_dev == NULL) {
			echub_err("echub_dev is NULL return\n");
			return -ENODEV;
		}
	}

	echub_rtc = devm_kzalloc(&pdev->dev, sizeof(struct echub_rtc_data),
			      GFP_KERNEL);
	if (!echub_rtc)
		return -ENOMEM;

	echub_rtc->echub_dev = echub_dev;
	platform_set_drvdata(pdev, echub_rtc);

	echub_rtc->rtc = devm_rtc_allocate_device(&pdev->dev);
	if (IS_ERR(echub_rtc->rtc))
		return PTR_ERR(echub_rtc->rtc);

	echub_rtc->rtc->ops = &echub_rtc_ops;
	ret = rtc_register_device(echub_rtc->rtc);
	if (ret)
		return ret;

	return 0;
}

static const struct of_device_id echub_rtc_of_match[] = {
	{ .compatible = "huawei,echub-rtc" },
	{},
};
MODULE_DEVICE_TABLE(of, echub_rtc_of_match);

static struct platform_driver echub_rtc_driver = {
	.driver = {
		.name = "echub_rtc",
		.of_match_table = of_match_ptr(echub_rtc_of_match),
	},
	.probe = echub_rtc_probe,
};
module_platform_driver(echub_rtc_driver);

MODULE_ALIAS("RTC driver");
MODULE_DESCRIPTION("MCU or EC RTC driver");
MODULE_LICENSE("GPL");
