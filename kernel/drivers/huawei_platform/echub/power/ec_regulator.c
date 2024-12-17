/*
 * ite83201-regulator.c
 *
 * Regulator driver for ITE83201 PMIC
 *
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/of.h>
#include <linux/slab.h>
#include "../echub_i2c.h"
#include <linux/regulator/of_regulator.h>

#define COMMAND_TO_EC           0x02B2
#define EC_1V2_EN_REG           0x50
#define EC_DSI_VCCIO_REG        0x52
#define EC_HDMI_VDD_ON_REG      0x54
#define EC_1V8_EN_REG           0x56
#define EC_1V1_EN_REG           0x58
#define EC_3V3_EN_REG           0x5A
#define EC_CAM_PWR_REG          0x5C

#define ENABLE                  1
#define DISABLE                 0
#define RE_COUNTS               3
enum {
	EC_1V2_EN = 1,
	EC_DSI_VCCIO_ON,
	EC_HDMI_VDD_ON,
	EC_1V8_EN,
	EC_1V1_EN,
	EC_3V3_EN,
	EC_CAM_PWR_EN,
};

struct ec_regulator_data {
	int id;
	struct regulator_init_data *initdata;
	struct device_node *reg_node;
};

struct echub_power {
	struct device *dev;
	struct echub_i2c_dev *echub_dev;
	struct ec_regulator_data *regulators;
	int num_regulators;
};

static int ec_get_register(struct regulator_dev *rdev, int *shift, int status)
{
	int offset = DISABLE;
	int ldo = rdev_get_id(rdev);

	if (status == DISABLE) {
		offset = DISABLE;
	} else if (status == ENABLE) {
		offset = ENABLE;
	} else {
		pr_err("[%s]int status para can only be zero or one!\n", __func__);
		return -EINVAL;
	}

	switch (ldo) {
	case EC_1V2_EN:
		*shift = EC_1V2_EN_REG + offset;
		break;
	case EC_DSI_VCCIO_ON:
		*shift = EC_DSI_VCCIO_REG + offset;
		break;
	case EC_HDMI_VDD_ON:
		*shift = EC_HDMI_VDD_ON_REG + offset;
		break;
	case EC_1V8_EN:
		*shift = EC_1V8_EN_REG + offset;
		break;
	case EC_1V1_EN:
		*shift = EC_1V1_EN_REG + offset;
		break;
	case EC_3V3_EN:
		*shift = EC_3V3_EN_REG + offset;
		break;
	case EC_CAM_PWR_EN:
		*shift = EC_CAM_PWR_REG + offset;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ec_regulator_enable(struct regulator_dev *rdev)
{
	int ret = 0;
	int shift = 0;
	int i = 0;
	int rslt = 1;
	struct echub_power *ep = rdev_get_drvdata(rdev);

	if (ep == NULL || ep->echub_dev->client == NULL) {
		pr_err("[%s]regulator get  para is err!\n", __func__);
		return -EINVAL;
	}
	ret = ec_get_register(rdev, &shift, ENABLE);
	if (ret) {
		pr_err("[%s] could not get EC enable register\n", __func__);
		return ret;
	}
	printk("<[%s]:regulator_id=%d>\n", __func__, rdev_get_id(rdev));

	for (i = 0; i < RE_COUNTS; i++) {
		rslt = ep->echub_dev->write_func(ep->echub_dev, COMMAND_TO_EC, shift);
		if (rslt == 0) {
			printk("regulator_enable_ok,re_counts= %d.\n", i);
			break;
		}
	}
	return rslt;
}

static int ec_regulator_disable(struct regulator_dev *rdev)
{
	int ret = 0;
	int shift = 0;
	int i = 0;
	int rslt = 1;
	struct echub_power *ep = rdev_get_drvdata(rdev);

	if (ep == NULL || ep->echub_dev->client == NULL) {
		pr_err("[%s]regulator get  para is err!\n", __func__);
		return -EINVAL;
	}
	ret = ec_get_register(rdev, &shift, DISABLE);
	if (ret) {
		pr_err("[%s] could not get EC disable register\n", __func__);
		return ret;
	}
	printk("<[%s]:regulator_id=%d>\n", __func__, rdev_get_id(rdev));

	for (i = 0; i < RE_COUNTS; i++) {
		rslt = ep->echub_dev->write_func(ep->echub_dev, COMMAND_TO_EC, shift);
		if (rslt == 0) {
			printk("regulator_disable_ok,re_counts= %d.\n", i);
			break;
		}
	}

	return rslt;
}

static struct regulator_ops ec_regulator_ops = {
	.enable         = ec_regulator_enable,
	.disable        = ec_regulator_disable,
};

static struct regulator_desc regulators[] = {
	{
		.name       = "LDO0",
		.id         = EC_1V2_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO1",
		.id         = EC_DSI_VCCIO_ON,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO2",
		.id         = EC_HDMI_VDD_ON,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO3",
		.id         = EC_1V8_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO4",
		.id         = EC_1V1_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO5",
		.id         = EC_3V3_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO6",
		.id         = EC_CAM_PWR_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	}
};

static int ec_regulator_dt_parse_data(struct echub_power *ep)
{
	int i, ret;
	struct device_node *regulators_np = NULL;
	struct device_node *reg_np = NULL;
	struct ec_regulator_data *rdata = NULL;

	regulators_np = of_get_child_by_name(ep->dev->of_node, "regulators");
	if (!regulators_np) {
		dev_err(ep->dev, "could not find regulators sub-node\n");
		return -EINVAL;
	}

	/* count the number of regulators to be supported in pmic */
	ep->num_regulators = of_get_child_count(regulators_np);

	rdata = devm_kzalloc(ep->dev, sizeof(struct ec_regulator_data) *
				ep->num_regulators, GFP_KERNEL);
	if (!rdata) {
		of_node_put(regulators_np);
		return -ENOMEM;
	}

	ep->regulators = rdata;
	for (i = 0; i < ARRAY_SIZE(regulators); ++i) {
		reg_np = of_get_child_by_name(regulators_np,
						regulators[i].name);
		if (!reg_np)
			continue;

		rdata->id = regulators[i].id;
		rdata->initdata = of_get_regulator_init_data(ep->dev,
								reg_np,
								&regulators[i]);
		rdata->reg_node = reg_np;
		++rdata;
	}
	ep->num_regulators = rdata - ep->regulators;
	of_node_put(reg_np);
	of_node_put(regulators_np);

	return 0;
}

static int echub_regulator_probe(struct platform_device *pdev)
{
	int i, ret;
	struct echub_i2c_dev *echub_dev = dev_get_drvdata(pdev->dev.parent);
	struct regulator_config config = { };
	struct regulator_dev *rdev = NULL;
	struct echub_power *ep = NULL;

	if (echub_dev == NULL) {
		printk("echub_dev is null, get echub_dev form tmp\n");
		echub_dev = get_echub_dev();
		if (echub_dev == NULL)
			return -ENODEV;
	}

	ep = devm_kzalloc(&pdev->dev, sizeof(struct echub_power),
				GFP_KERNEL);
	if (!ep)
		return -ENOMEM;

	ep->dev = &pdev->dev;
	ep->echub_dev = echub_dev;
	platform_set_drvdata(pdev, ep);

	if (IS_ENABLED(CONFIG_OF) && pdev->dev.of_node) {
		ret = ec_regulator_dt_parse_data(ep);
		if (ret)
			return ret;
	}

	for (i = 0; i < ep->num_regulators; i++) {
		config.dev = ep->dev;
		config.of_node = ep->regulators[i].reg_node;
		config.init_data = ep->regulators[i].initdata;
		config.driver_data = ep;
		rdev = devm_regulator_register(&pdev->dev, &regulators[i], &config);
		if (IS_ERR(rdev)) {
			ret = PTR_ERR(rdev);
			dev_err(&pdev->dev, "regulator register failed\n");
			return ret;
		}
	}

	return 0;
}

static const struct of_device_id of_hw_echub_regulator_match_tbl[] = {
	{ .compatible = "huawei,echub-power" },
	{},
};

static struct platform_driver echub_regulator_driver = {
	.driver = {
		.name = "echub-power",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(of_hw_echub_regulator_match_tbl),
	},
	.probe = echub_regulator_probe,
};

static int __init echub_regulator_init(void)
{
	return platform_driver_register(&echub_regulator_driver);
}
module_init(echub_regulator_init);

static void __exit echub_regulator_exit(void)
{
	platform_driver_unregister(&echub_regulator_driver);
}
module_exit(echub_regulator_exit);


MODULE_DESCRIPTION("EC voltage regulator driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:EC-LDO");
