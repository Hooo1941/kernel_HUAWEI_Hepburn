/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * pmic_gpio.c
 *
 * Device driver for PMU GPIO driver
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "pmic_ramp.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/irqnr.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>
#include <linux/regmap.h>
#include <linux/platform_device.h>
#include <platform_include/basicplatform/linux/pr_log.h>

#define PR_LOG_TAG "pmic-gpio"

struct config_regs {
	unsigned int data_reg;
	unsigned int dir_reg;
};

struct pmic_gpio_info {
	struct device *dev;
	struct regmap *regmap;
	unsigned int nr;
	struct config_regs regs;
	raw_spinlock_t lock;
	struct gpio_chip gc;
};

/* return: 0=out, 1=in */
static int pmic_gpio_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	struct pmic_gpio_info *pmic_gpio = gpiochip_get_data(gc);
	unsigned int val = 0;

	regmap_read(pmic_gpio->regmap, pmic_gpio->regs.dir_reg, &val);
	pr_debug("%s, gpio %d, 0x%x = 0x%x\n", __func__,
		pmic_gpio->nr, pmic_gpio->regs.dir_reg, val);
	return !(val & (BIT(0)));
}

static int pmic_gpio_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	struct pmic_gpio_info *pmic_gpio = gpiochip_get_data(gc);
	unsigned long flags;

	if (offset >= gc->ngpio)
		return -EINVAL;

	pr_debug("%s,gpio %d\n", __func__, pmic_gpio->nr);
	raw_spin_lock_irqsave(&pmic_gpio->lock, flags);
	regmap_update_bits(pmic_gpio->regmap, pmic_gpio->regs.dir_reg, BIT(0), 0);
	raw_spin_unlock_irqrestore(&pmic_gpio->lock, flags);
	return 0;
}

static int pmic_gpio_direction_output(struct gpio_chip *gc, unsigned int offset,
		int value)
{
	struct pmic_gpio_info *pmic_gpio = gpiochip_get_data(gc);
	unsigned long flags;

	if (offset >= gc->ngpio)
		return -EINVAL;

	raw_spin_lock_irqsave(&pmic_gpio->lock, flags);
	regmap_update_bits(pmic_gpio->regmap, pmic_gpio->regs.dir_reg, BIT(0), 1);
	regmap_update_bits(pmic_gpio->regmap, pmic_gpio->regs.data_reg, BIT(0), value);
	pr_debug("%s, gpio %d, dir 0x%x = 0x%x, data 0x%x = 0x%x\n", __func__,
		pmic_gpio->nr, pmic_gpio->regs.dir_reg, 1, pmic_gpio->regs.data_reg, value);
	raw_spin_unlock_irqrestore(&pmic_gpio->lock, flags);
	return 0;
}

static int pmic_gpio_get_value(struct gpio_chip *gc, unsigned int offset)
{
	struct pmic_gpio_info *pmic_gpio = gpiochip_get_data(gc);
	unsigned int val = 0;

	regmap_read(pmic_gpio->regmap, pmic_gpio->regs.data_reg, &val);
	pr_debug("%s, gpio %d, 0x%x = 0x%x\n", __func__,
		pmic_gpio->nr, pmic_gpio->regs.data_reg, val);
	return val & (BIT(0));
}

static void pmic_gpio_set_value(struct gpio_chip *gc, unsigned int offset,
		int value)
{
	struct pmic_gpio_info *pmic_gpio = gpiochip_get_data(gc);

	pr_debug("%s, gpio %d, 0x%x = 0x%x\n", __func__,
		pmic_gpio->nr, pmic_gpio->regs.data_reg, value);
	regmap_update_bits(pmic_gpio->regmap, pmic_gpio->regs.data_reg, BIT(0), value);
}

static int parse_dts(struct pmic_gpio_info *pmic_gpio,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "gpio-nr", &pmic_gpio->nr);
	if (ret) {
		pr_err("fatal error: can not get gpio-nr\n");
		return -1;
	}

	ret = of_property_read_u32(np, "data-reg", &pmic_gpio->regs.data_reg);
	if (ret) {
		pr_err("fatal error: can not get data-reg\n");
		return -1;
	}

	ret = of_property_read_u32(np, "dir-reg", &pmic_gpio->regs.dir_reg);
	if (ret) {
		pr_err("fatal error: can not get dir-reg\n");
		return -1;
	}

	pr_info("pmic gpio nr %d, 0x%x, 0x%x\n", pmic_gpio->nr,
		pmic_gpio->regs.data_reg, pmic_gpio->regs.dir_reg);
	return 0;
}

static int pmic_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pmic_gpio_info *pmic_gpio = NULL;
	int ret;

	pmic_gpio = devm_kzalloc(dev, sizeof(*pmic_gpio), GFP_KERNEL);
	if (!pmic_gpio)
		return -ENOMEM;

	pmic_gpio->dev = dev;
	pmic_gpio->regmap = dev_get_regmap(dev->parent, NULL);
	if (!pmic_gpio->regmap)
		return -ENODEV;

	raw_spin_lock_init(&pmic_gpio->lock);

	ret = parse_dts(pmic_gpio, dev->of_node);
	if (ret) {
		dev_err(&pdev->dev, "parse_dts error\n");
		return ret;
	}

	pmic_gpio->gc.request = gpiochip_generic_request;
	pmic_gpio->gc.free = gpiochip_generic_free;
	pmic_gpio->gc.base = pmic_gpio->nr;
	pmic_gpio->gc.ngpio = 1;

	pmic_gpio->gc.get_direction = pmic_gpio_get_direction;
	pmic_gpio->gc.direction_input = pmic_gpio_direction_input;
	pmic_gpio->gc.direction_output = pmic_gpio_direction_output;
	pmic_gpio->gc.get = pmic_gpio_get_value;
	pmic_gpio->gc.set = pmic_gpio_set_value;
	pmic_gpio->gc.label = dev_name(dev);
	pmic_gpio->gc.parent = dev;
	pmic_gpio->gc.owner = THIS_MODULE;

	ret = devm_gpiochip_add_data(dev, &pmic_gpio->gc, pmic_gpio);
	dev_info(&pdev->dev, "PMIC GPIO chip registered ret %d\n", ret);
	return ret;
}

const static struct of_device_id pmic_gpio_match_tbl[] = {
	{
		.compatible = "pmic-gpio",
	},
	{ }    /* end */
};

static struct platform_driver pmic_gpio_driver = {
	.driver = {
			.name = "pmic-gpio",
			.owner = THIS_MODULE,
			.of_match_table = pmic_gpio_match_tbl,
		},
	.probe = pmic_gpio_probe,
};

static int __init pmic_gpio_init(void)
{
	return platform_driver_register(&pmic_gpio_driver);
}

static void __exit pmic_gpio_exit(void)
{
	platform_driver_unregister(&pmic_gpio_driver);
}

module_init(pmic_gpio_init);
module_exit(pmic_gpio_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PMU GPIO Driver");
