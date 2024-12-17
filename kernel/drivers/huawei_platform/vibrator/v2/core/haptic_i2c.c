/*
 * haptic_i2c.c
 *
 * code for vibrator
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

#include <linux/regmap.h>
#include <linux/module.h>

#include "haptic_core.h"
#include "haptic_regmap.h"

#define DEVICE_NAME "hwvibrator"

static const struct regmap_config haptic_i2c_regmap_config = {
	.name = DEVICE_NAME,
	.reg_bits = 8, /* 8 bit addr */
	.val_bits = 8, /* 8 bit value */
	.max_register = 0xFF, /* max 255 regs */
	.cache_type = REGCACHE_NONE, /* disable cache */
};

static int haptic_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	int ret;
	struct haptic *haptic = NULL;

	vib_info("enter");

	haptic = devm_kzalloc(&i2c->dev, sizeof(*haptic), GFP_KERNEL);
	if (!haptic)
		return -ENOMEM;

	haptic->dev = &i2c->dev;
	haptic->i2c = i2c;
	i2c_set_clientdata(i2c, haptic);

	haptic->regmap =
		haptic_regmap_init(haptic->i2c, &haptic_i2c_regmap_config);
	if (!haptic->regmap) {
		vib_err("haptic regmap init failed");
		return -EFAULT;
	}

	ret = hwvibrator_init(haptic);
	if (ret) {
		vib_err("hwvibrator init failed");
		return ret;
	}

	vib_info("probe completed successfully!");
	return 0;
}

static int haptic_i2c_remove(struct i2c_client *i2c)
{
	struct haptic *haptic = i2c_get_clientdata(i2c);

	if (!haptic)
		return -EFAULT;

	hwvibrator_deinit(haptic);

	vib_info("haptic i2c driver removed");

	return 0;
}

static int haptic_i2c_suspend(struct device *dev)
{
	struct haptic *haptic = dev_get_drvdata(dev);

	if (!haptic)
		return -EFAULT;

	vib_info("enter");

	mutex_lock(&haptic->lock);
	haptic->hops->play_stop(haptic);
	mutex_unlock(&haptic->lock);

	return 0;
}

static int haptic_i2c_resume(struct device *dev __maybe_unused)
{
	return 0;
}

static SIMPLE_DEV_PM_OPS(haptic_pm_ops, haptic_i2c_suspend, haptic_i2c_resume);

static const struct i2c_device_id haptic_i2c_id[] = { { DEVICE_NAME, 0 }, {} };
MODULE_DEVICE_TABLE(i2c, haptic_i2c_id);

static const struct of_device_id haptic_dt_match[] = {
	{ .compatible = "hwvibrator,haptic" },
	{},
};

static struct i2c_driver haptic_i2c_driver = {
	.driver = {
		.name = DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(haptic_dt_match),
#ifdef CONFIG_PM_SLEEP
		.pm = &haptic_pm_ops,
#endif
	},
	.probe = haptic_i2c_probe,
	.remove = haptic_i2c_remove,
	.id_table = haptic_i2c_id,
};

static int __init haptic_i2c_init(void)
{
	int ret = i2c_add_driver(&haptic_i2c_driver);
	if (ret) {
		vib_err("add haptic i2c driver failed");
		return ret;
	}
	return 0;
}
module_init(haptic_i2c_init);

static void __exit haptic_i2c_exit(void)
{
	i2c_del_driver(&haptic_i2c_driver);
}
module_exit(haptic_i2c_exit);

MODULE_DESCRIPTION("Huawei Haptic Driver");
MODULE_LICENSE("GPL v2");
