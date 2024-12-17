/*
 * touchpad.c
 *
 * touchpad driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>

struct touchpad_data {
	int reset_gpio;
	int post_power_delay_ms;
	bool wakeup_source;
	struct regulator_bulk_data supplies[2];
};

static void touchpad_parse_dts(struct device_node *np, struct touchpad_data *pdata)
{
	if (of_property_read_u32(np, "reset-gpio", (u32 *)&pdata->reset_gpio))
		pdata->reset_gpio = -1;

	pr_info("touchpad reset-gpio: %d\n", pdata->reset_gpio);

	if (of_property_read_u32(np, "post-power-delay-ms", (u32 *)&pdata->post_power_delay_ms))
		pdata->post_power_delay_ms = 0;

	pr_info("touchpad post-power-delay-ms: %d\n", pdata->post_power_delay_ms);

	pdata->wakeup_source = of_property_read_bool(np, "wakeup_source");
	pr_info("touchpad wakeup_source: %d\n", pdata->wakeup_source);
}

static int touchpad_probe(struct platform_device *pdev)
{
	int ret;
	struct touchpad_data *pdata = NULL;

	pdata = devm_kzalloc(&pdev->dev, sizeof(struct touchpad_data), GFP_KERNEL);
	if (pdata == NULL) {
		pr_err("failed to allocate mem for touchpad\n");
		return -ENOMEM;
	}

	touchpad_parse_dts(pdev->dev.of_node, pdata);
	platform_set_drvdata(pdev, pdata);

	pdata->supplies[0].supply = "vdd";
	pdata->supplies[1].supply = "vddl";
	ret = devm_regulator_bulk_get(&pdev->dev, ARRAY_SIZE(pdata->supplies), pdata->supplies);
	if (ret) {
		pr_err("query regulator failed, ret: %d\n", ret);
		devm_kfree(&pdev->dev, pdata);
		return ret;
	}

	ret = regulator_bulk_enable(ARRAY_SIZE(pdata->supplies), pdata->supplies);
	if (ret) {
		pr_err("enable regulator failed, ret: %d\n", ret);
		devm_kfree(&pdev->dev, pdata);
		return ret;
	}

	if (pdata->post_power_delay_ms)
		mdelay(pdata->post_power_delay_ms);

	if (gpio_is_valid(pdata->reset_gpio)) {
		gpio_request(pdata->reset_gpio, "touchpad_rest");
		gpio_direction_output(pdata->reset_gpio, 1);
	}

	return 0;
}

static int touchpad_remove(struct platform_device *pdev)
{
	struct touchpad_data *pdata = platform_get_drvdata(pdev);

	if (pdata == NULL)
		return 0;

	if (gpio_is_valid(pdata->reset_gpio)) {
		gpio_set_value(pdata->reset_gpio, 0);
		gpio_free(pdata->reset_gpio);
	}

	if (pdata->post_power_delay_ms)
		mdelay(pdata->post_power_delay_ms);

	regulator_bulk_disable(ARRAY_SIZE(pdata->supplies), pdata->supplies);
	return 0;
}

#ifdef CONFIG_PM
static int touchpad_suspend(struct platform_device *pdev, pm_message_t)
{
	struct touchpad_data *pdata = platform_get_drvdata(pdev);

	if (pdata == NULL)
		return 0;

	if (pdata->wakeup_source)
		return 0;

	if (gpio_is_valid(pdata->reset_gpio))
		gpio_set_value(pdata->reset_gpio, 0);

	if (pdata->post_power_delay_ms)
		mdelay(pdata->post_power_delay_ms);

	regulator_bulk_disable(ARRAY_SIZE(pdata->supplies), pdata->supplies);
	return 0;
}

static int touchpad_resume(struct platform_device *pdev)
{
	int ret;
	struct touchpad_data *pdata = platform_get_drvdata(pdev);

	if (pdata == NULL)
		return 0;

	if (pdata->wakeup_source)
		return 0;

	ret = regulator_bulk_enable(ARRAY_SIZE(pdata->supplies), pdata->supplies);
	if (ret) {
		pr_err("enable regulator failed when resume, ret: %d\n", ret);
		return ret;
	}

	if (pdata->post_power_delay_ms)
		mdelay(pdata->post_power_delay_ms);

	if (gpio_is_valid(pdata->reset_gpio))
		gpio_set_value(pdata->reset_gpio, 1);

	return 0;
}
#endif /* CONFIG_PM */

static struct of_device_id touchpad_match_table[] = {
	{ .compatible = "huawei,touchpad", },
	{},
};

static struct platform_driver touchpad_driver = {
	.probe        = touchpad_probe,
	.remove       = touchpad_remove,
#ifdef CONFIG_PM
	.suspend = touchpad_suspend,
	.resume = touchpad_resume,
#endif
	.driver       = {
		.name           = "huawei,touchpad",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(touchpad_match_table),
	},
};

int __init touchpad_init(void)
{
	return platform_driver_register(&touchpad_driver);
}

void __exit touchpad_exit(void)
{
	platform_driver_unregister(&touchpad_driver);
}

rootfs_initcall(touchpad_init);
module_exit(touchpad_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_DESCRIPTION("touchpad module driver");
MODULE_LICENSE("GPL");
