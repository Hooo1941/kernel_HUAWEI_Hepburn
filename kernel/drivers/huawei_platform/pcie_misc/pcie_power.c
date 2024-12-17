/*
 * predev_misc driver for GP
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/regulator/consumer.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#define MAX_NUM 10
#define GPIO_LOW 0
#define GPIO_HIGH 1

struct power_misc {
	struct device *dev;

	/* regulators info */
	u32 regulators_num;
	char *regulator_names_list[MAX_NUM];
	struct regulator *supplys_list[MAX_NUM];
	u32 regulator_values[MAX_NUM];
	u32 s3_disable_supplys_num;
	u32 s3_disable_supplys_index_list[MAX_NUM];

	/* reset gpios info */
	u32 rst_gpios_num;
	u32 rst_gpios_list[MAX_NUM];
	u32 rst_times_list[MAX_NUM];
	u32 s3_down_rst_gpios_num;
	u32 s3_down_rst_gpios_index_list[MAX_NUM];

	/* power gpios info */
	u32 power_gpios_num;
	u32 power_gpios_list[MAX_NUM];
	u32 s3_down_power_gpios_num;
	u32 s3_down_power_gpios_index_list[MAX_NUM];
};

static int init_all_regulator_supplys(struct platform_device *pdev)
{
	int i, ret;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (of_property_read_u32(pmisc->dev->of_node, "regulators_num", &pmisc->regulators_num) != 0) {
		pr_err("[%s] Get regulator_name_list_nums from DTS error.\n", __func__);
		return -1;
	}
	if (pmisc->regulators_num > MAX_NUM) {
		pr_err("[%s], Regulator Nums Overflow in dts!\n", __func__);
		pmisc->regulators_num = MAX_NUM;
	}

	if (pmisc->regulators_num <= 0)
		return -1;

	ret = of_property_read_string_array(pmisc->dev->of_node, "regulator_names_list",
		(const char **)&pmisc->regulator_names_list[0], pmisc->regulators_num);
	if (ret < 0) {
		pr_err("[%s], get regulator names fail in dts: %d!\n", __func__, ret);
		return -1;
	}

	ret = of_property_read_u32_array(pmisc->dev->of_node, "regulator_values",
		&pmisc->regulator_values[0], pmisc->regulators_num);
	if (ret < 0)
		pr_err("[%s], get regulator_values fail in dts: %d!\n", __func__, ret);

	for (i = 0; i < pmisc->regulators_num; i++) {
		pr_info("[%s] regulator_names_list[%d] = %s\n", __func__, i, pmisc->regulator_names_list[i]);
		pmisc->supplys_list[i] = devm_regulator_get(pmisc->dev, pmisc->regulator_names_list[i]);
		if (IS_ERR(pmisc->supplys_list[i])) {
			ret = PTR_ERR(pmisc->supplys_list[i]);
			if (ret != -EPROBE_DEFER)
				pr_err("[%s], Failed to get %s regulator: %d\n",
					__func__, pmisc->regulator_names_list[i], ret);
			break;
		}
	}

	if (of_property_read_u32(pmisc->dev->of_node, "s3_disable_supplys_num", &pmisc->s3_disable_supplys_num) != 0) {
		pr_err("[%s], doesn't have s3_disable_supplys_num property\n", __func__);
	} else if (pmisc->s3_disable_supplys_num > pmisc->regulators_num) {
		pr_err("[%s], s3_disable_supplys_num Overflow in dts!\n", __func__);
		pmisc->s3_disable_supplys_num = pmisc->regulators_num;
	}

	if (pmisc->s3_disable_supplys_num > 0) {
		ret = of_property_read_u32_array(pmisc->dev->of_node, "s3_disable_supplys_index_list",
				&pmisc->s3_disable_supplys_index_list[0], pmisc->s3_disable_supplys_num);
		if (ret)
			pr_err("[%s], doesn't have s3_disable_supplys_index_list property\n", __func__);
	}

	return 0;
}

static int enable_all_regulators(struct platform_device *pdev)
{
	int i, ret;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (pmisc->regulators_num == 0 || pmisc->regulator_names_list[0] == NULL) {
		pr_err("regulator_names_list is NULL\n");
		return -1;
	}

	for (i = 0; i < pmisc->regulators_num; i++) {
		if (pmisc->supplys_list[i] == NULL)
			break;

		if (pmisc->regulator_values[i] > 0) {
			pr_info("Regulator %s set voltage %d uV\n", pmisc->regulator_names_list[i], pmisc->regulator_values[i]);
			regulator_set_voltage(pmisc->supplys_list[i],
				pmisc->regulator_values[i], pmisc->regulator_values[i]);
		}

		pr_info("[%s] enable %s regulator\n", __func__, pmisc->regulator_names_list[i]);
		ret = regulator_enable(pmisc->supplys_list[i]);
		if (ret < 0) {
			pr_err("[%s], Failed to enable %s regulator: %d\n",
				__func__, pmisc->regulator_names_list[i], ret);
			break;
		}
	}

	return 0;
}

static int enable_regulators(struct platform_device *pdev)
{
	int i, ret;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (pmisc->s3_disable_supplys_num == 0) {
		pr_err("[%s], no regulator needs to be enabled\n", __func__);
		return -1;
	}

	for (i = 0; i < pmisc->s3_disable_supplys_num; i++) {
		if (pmisc->supplys_list[pmisc->s3_disable_supplys_index_list[i]] == NULL)
			break;

		pr_info("[%s] enable %s regulator\n", __func__,
			pmisc->regulator_names_list[pmisc->s3_disable_supplys_index_list[i]]);

		ret = regulator_enable(pmisc->supplys_list[pmisc->s3_disable_supplys_index_list[i]]);
		if (ret < 0) {
			pr_err("[%s], Failed to enable %s regulator: %d\n",
				__func__, pmisc->regulator_names_list[pmisc->s3_disable_supplys_index_list[i]], ret);
			break;
		}
	}

	return 0;
}

static int disable_regulators(struct platform_device *pdev)
{
	int i, ret;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (pmisc->s3_disable_supplys_num <= 0) {
		pr_err("[%s], no regulator needs to be disabled\n", __func__);
		return -1;
	}

	for (i = 0; i < pmisc->s3_disable_supplys_num; i++) {
		if (pmisc->supplys_list[pmisc->s3_disable_supplys_index_list[i]] == NULL)
			break;

		pr_info("[%s] disable %s regulator\n", __func__,
			pmisc->regulator_names_list[pmisc->s3_disable_supplys_index_list[i]]);

		ret = regulator_disable(pmisc->supplys_list[pmisc->s3_disable_supplys_index_list[i]]);
		if (ret < 0) {
			pr_err("[%s], Failed to disable %s regulator: %d\n",
				__func__, pmisc->regulator_names_list[pmisc->s3_disable_supplys_index_list[i]], ret);
			break;
		}
	}

	return 0;
}

static int init_all_power_gpios(struct platform_device *pdev)
{
	int ret;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (of_property_read_u32(pmisc->dev->of_node, "power_gpios_num", &pmisc->power_gpios_num) != 0) {
		pr_err("[%s], doesn't have power_gpios_num property\n", __func__);
		return -1;
	}
	if (pmisc->power_gpios_num > MAX_NUM) {
		pr_err("[%s], power_gpios_num Overflow in dts!\n", __func__);
		pmisc->power_gpios_num = MAX_NUM;
	}

	if (pmisc->power_gpios_num <= 0)
		return -1;

	ret = of_property_read_u32_array(pmisc->dev->of_node, "power_gpios_list",
			&pmisc->power_gpios_list[0], pmisc->power_gpios_num);
	if (ret) {
		pr_err("[%s], doesn't have power_gpios_list property\n", __func__);
		return -1;
	}

	if (of_property_read_u32(pmisc->dev->of_node, "s3_down_power_gpios_num", &pmisc->s3_down_power_gpios_num) != 0) {
		pr_err("[%s], doesn't have s3_down_power_gpios_num property\n", __func__);
	} else if (pmisc->s3_down_power_gpios_num > pmisc->power_gpios_num) {
		pr_err("[%s], s3_down_power_gpios_num Overflow in dts!\n", __func__);
		pmisc->s3_down_power_gpios_num = pmisc->power_gpios_num;
	}

	if (pmisc->s3_down_power_gpios_num > 0) {
		ret = of_property_read_u32_array(pmisc->dev->of_node, "s3_down_power_gpios_index_list",
				&pmisc->s3_down_power_gpios_index_list[0], pmisc->s3_down_power_gpios_num);
		if (ret)
			pr_err("[%s], doesn't have s3_down_power_gpios_index_list property\n", __func__);
	}

	return 0;
}

static int power_on_all_gpios(struct platform_device *pdev)
{
	int i, ret;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (pmisc->power_gpios_num <= 0) {
		pr_err("power_gpios_list is empty\n");
		return -1;
	}

	for (i = 0; i < pmisc->power_gpios_num; i++) {
		if (pmisc->power_gpios_list[i] == 0)
			break;

		pr_info("[%s] power_gpios_list[%d] = %d\n", __func__, i, pmisc->power_gpios_list[i]);
		if (!gpio_is_valid(pmisc->power_gpios_list[i])) {
			pr_err("power_gpio %d is not valid\n", pmisc->power_gpios_list[i]);
			break;
		}

		if (devm_gpio_request(pmisc->dev, pmisc->power_gpios_list[i], "power_gpio")) {
			pr_err("Could not request %d gpio\n", pmisc->power_gpios_list[i]);
			break;
		} else if (gpio_direction_output(pmisc->power_gpios_list[i], GPIO_HIGH)) {
			pr_err("Could not set gpio %d as output\n", pmisc->power_gpios_list[i]);
			break;
		}
	}

	return 0;
}

static int init_all_rst_gpios(struct platform_device *pdev)
{
	int ret;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (of_property_read_u32(pmisc->dev->of_node, "rst_gpios_num", &pmisc->rst_gpios_num) != 0) {
		pr_err("[%s], doesn't have rst_gpios_num property\n", __func__);
		return -1;
	}
	if (pmisc->rst_gpios_num > MAX_NUM) {
		pr_err("[%s], rst_gpios_num Overflow in dts!\n", __func__);
		pmisc->rst_gpios_num = MAX_NUM;
	}

	if (pmisc->rst_gpios_num <= 0)
		return -1;

	ret = of_property_read_u32_array(pmisc->dev->of_node, "rst_gpios_list",
			&pmisc->rst_gpios_list[0], pmisc->rst_gpios_num);
	if (ret) {
		pr_err("[%s], doesn't have rst_gpios_list property\n", __func__);
		return -1;
	}

	ret = of_property_read_u32_array(pmisc->dev->of_node, "rst_times_list",
			&pmisc->rst_times_list[0], pmisc->rst_gpios_num);
	if (ret) {
		pr_err("[%s], doesn't have rst_times_list property\n", __func__);
		return -1;
	}

	if (of_property_read_u32(pmisc->dev->of_node, "s3_down_rst_gpios_num", &pmisc->s3_down_rst_gpios_num) != 0) {
		pr_err("[%s], doesn't have s3_down_rst_gpios_num property\n", __func__);
	} else if (pmisc->s3_down_rst_gpios_num > pmisc->rst_gpios_num) {
		pr_err("[%s], s3_down_rst_gpios_num Overflow in dts!\n", __func__);
		pmisc->s3_down_rst_gpios_num = pmisc->rst_gpios_num;
	}

	if (pmisc->s3_down_rst_gpios_num > 0) {
		ret = of_property_read_u32_array(pmisc->dev->of_node, "s3_down_rst_gpios_index_list",
				&pmisc->s3_down_rst_gpios_index_list[0], pmisc->s3_down_rst_gpios_num);
		if (ret)
			pr_err("[%s], doesn't have s3_down_rst_gpios_index_list property\n", __func__);
	}

	return 0;
}

static int reset_all_gpios(struct platform_device *pdev)
{
	int i, ret;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (pmisc->rst_gpios_num <= 0) {
		pr_err("rst_gpios_list is empty\n");
		return -1;
	}

	for (i = 0; i < pmisc->rst_gpios_num; i++) {
		if (pmisc->rst_gpios_list[i] == 0 || pmisc->rst_times_list[i] == 0)
			break;

		pr_info("[%s] rst_gpios_list[%d] = %d, rst_times_list[%d] = %d\n",
			__func__, i, pmisc->rst_gpios_list[i], i, pmisc->rst_times_list[i]);

		if (!gpio_is_valid(pmisc->rst_gpios_list[i])) {
			pr_err("rst_gpio %d is not valid\n", pmisc->rst_gpios_list[i]);
			break;
		}

		if (devm_gpio_request(pmisc->dev, pmisc->rst_gpios_list[i], "rst_gpio")) {
			pr_err("Could not request %d gpio\n", pmisc->rst_gpios_list[i]);
			break;
		} else if (gpio_direction_output(pmisc->rst_gpios_list[i], GPIO_LOW)) {
			pr_err("Could not set gpio %d as output\n", pmisc->rst_gpios_list[i]);
			break;
		}

		gpio_set_value(pmisc->rst_gpios_list[i], GPIO_LOW);
		msleep(pmisc->rst_times_list[i]);
		gpio_set_value(pmisc->rst_gpios_list[i], GPIO_HIGH);
	}

	return 0;
}

static void pull_down_gpios(struct platform_device *pdev)
{
	int i;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (pmisc->s3_down_power_gpios_num > 0) {
		for (i = 0; i < pmisc->s3_down_power_gpios_num; i++) {
			if (pmisc->power_gpios_list[pmisc->s3_down_power_gpios_index_list[i]] == 0)
				break;

			pr_info("[%s] pull down power gpio %d\n", __func__,
				pmisc->power_gpios_list[pmisc->s3_down_power_gpios_index_list[i]]);

			gpio_set_value(pmisc->power_gpios_list[pmisc->s3_down_power_gpios_index_list[i]], GPIO_LOW);
		}
	}

	if (pmisc->s3_down_rst_gpios_num > 0) {
		for (i = 0; i < pmisc->s3_down_rst_gpios_num; i++) {
			if (pmisc->rst_gpios_list[pmisc->s3_down_rst_gpios_index_list[i]] == 0)
				break;

			pr_info("[%s] pull down rst gpio %d\n", __func__,
				pmisc->rst_gpios_list[pmisc->s3_down_rst_gpios_index_list[i]]);

			gpio_set_value(pmisc->rst_gpios_list[pmisc->s3_down_rst_gpios_index_list[i]], GPIO_LOW);
		}
	}
}

static void pull_up_gpios(struct platform_device *pdev)
{
	int i;
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (pmisc->s3_down_power_gpios_num > 0) {
		for (i = 0; i < pmisc->s3_down_power_gpios_num; i++) {
			if (pmisc->power_gpios_list[pmisc->s3_down_power_gpios_index_list[i]] == 0)
				break;

			pr_info("[%s] pull up power gpio %d\n", __func__,
				pmisc->power_gpios_list[pmisc->s3_down_power_gpios_index_list[i]]);

			gpio_set_value(pmisc->power_gpios_list[pmisc->s3_down_power_gpios_index_list[i]], GPIO_HIGH);
		}
	}

	if (pmisc->s3_down_rst_gpios_num > 0) {
		for (i = 0; i < pmisc->s3_down_rst_gpios_num; i++) {
			if (pmisc->rst_gpios_list[pmisc->s3_down_rst_gpios_index_list[i]] == 0)
				break;

			pr_info("[%s] pull up rst gpio %d\n", __func__,
				pmisc->rst_gpios_list[pmisc->s3_down_rst_gpios_index_list[i]]);

			gpio_set_value(pmisc->rst_gpios_list[pmisc->s3_down_rst_gpios_index_list[i]], GPIO_HIGH);
		}
	}
}

static int power_misc_probe(struct platform_device *pdev)
{
	struct power_misc *pmisc = NULL;

	pr_info("%s start\n", __func__);
	pmisc = devm_kzalloc(&pdev->dev, sizeof(struct power_misc), GFP_KERNEL);
	if (!pmisc)
		return -ENOMEM;

	pmisc->dev = &pdev->dev;
	platform_set_drvdata(pdev, pmisc);

	if (init_all_regulator_supplys(pdev))
		pr_err("[%s] Get regulator_supplys_list from DTS error.\n", __func__);
	else if (enable_all_regulators(pdev))
		pr_err("[%s] Enable regulator supplys failed.\n", __func__);

	if (init_all_power_gpios(pdev))
		pr_err("[%s] Get power_gpios_list from DTS error.\n", __func__);
	else if (power_on_all_gpios(pdev))
		pr_err("[%s] power on gpios failed.\n", __func__);

	if (init_all_rst_gpios(pdev))
		pr_err("[%s] Get reset_gpios_list from DTS error.\n", __func__);
	else if (reset_all_gpios(pdev))
		pr_err("[%s], Reset gpios failed.\n", __func__);

	return 0;
}

static int power_misc_remove(struct platform_device *pdev)
{
	struct power_misc *pmisc = platform_get_drvdata(pdev);

	if (pmisc == NULL)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	kfree(pmisc);
	pmisc = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int power_misc_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	pr_info("%s\n", __func__);
	if (disable_regulators(pdev))
		pr_err("[%s], no regulator needs to be disabled.\n", __func__);

	pull_down_gpios(pdev);

	return 0;
}

static int power_misc_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	pr_info("%s\n", __func__);
	if (enable_regulators(pdev))
		pr_err("[%s], no regulator needs to be enabled.\n", __func__);

	pull_up_gpios(pdev);

	return 0;
}

static const struct dev_pm_ops power_misc_pm_ops = {
	.suspend = power_misc_suspend,
	.resume = power_misc_resume,
};
#endif

static const struct platform_device_id power_misc_id_table[] = {
	{"power_misc", 0},
	{}
};
MODULE_DEVICE_TABLE(platform, power_misc_id_table);

static const struct of_device_id of_power_misc_match[] = {
	{ .compatible = "armpc,power_misc", },
	{}
};
MODULE_DEVICE_TABLE(of, of_power_misc_match);

static struct platform_driver power_misc_driver = {
	.driver = {
		.name = "power_misc",
		.owner = THIS_MODULE,
#ifdef CONFIG_PM
		.pm = &power_misc_pm_ops,
#endif
		.of_match_table = of_match_ptr(of_power_misc_match),
	},

	.probe = power_misc_probe,
	.remove = power_misc_remove,
	.id_table = power_misc_id_table,
};

static int __init power_misc_init(void)
{
	return platform_driver_register(&power_misc_driver);
}

static void __exit power_misc_exit(void)
{
	platform_driver_unregister(&power_misc_driver);
}

fs_initcall_sync(power_misc_init);
module_exit(power_misc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Armpc power misc driver");
MODULE_AUTHOR("HW Technologies Co., Ltd.");
