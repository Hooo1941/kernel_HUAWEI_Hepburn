/*
 * echub_fnled.c
 *
 * Send command to EC for fn led
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

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mfd/core.h>
#include <linux/of.h>
#include "../echub.h"
#include "../echub_i2c.h"

#define COMMAND_TO_EC		0x02B2
#define EC_BIOS_END			0x81
#define EC_ACPI_ENABLE		0x90


static int echub_fnled_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct echub_i2c_dev *echub_dev = dev_get_drvdata(pdev->dev.parent);

	echub_info("%s enter\n", __func__);
	if (echub_dev == NULL) {
		echub_err("echub_dev is null, get echub_dev form tmp\n");
		echub_dev = get_echub_dev();
		if (echub_dev == NULL) {
			echub_err("echub_dev is NULL return\n");
			return -ENODEV;
		}
	}
	ret = echub_dev->write_func(echub_dev, COMMAND_TO_EC, EC_BIOS_END);
	if (ret)
		echub_err("bios end failed \n");
	ret = echub_dev->write_func(echub_dev, COMMAND_TO_EC, EC_ACPI_ENABLE);
	if (ret)
		echub_err("failed to restore the fn indicator status.\n");
	echub_info("%s enter\n", __func__);
	return 0;
	}


static const struct of_device_id of_echub_fnled_match_tbl[] = {
	{ .compatible = "huawei,echub-fnled" },
	{},
};

/* init platform_driver echub_fnled_driver */
static struct platform_driver echub_fnled_driver = {
	.driver = {
		.name = "echub-fnled",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(of_echub_fnled_match_tbl),
	},
	.probe = echub_fnled_probe,
};


static int __init fn_led_dev_init(void)
{
	return platform_driver_register(&echub_fnled_driver);
}

/**********************************************************
*  Function:	   fn_led_dev_exit
*  Discription:	module exit
*  Parameters:	 NULL
*  return value:   NULL
**********************************************************/
static void __exit fn_led_dev_exit(void)
{
	echub_info("echub_fnled_exit\n");
	platform_driver_unregister(&echub_fnled_driver);
}

module_init(fn_led_dev_init);
module_exit(fn_led_dev_exit);

MODULE_DESCRIPTION("fn LED driver");
MODULE_LICENSE("GPL");
