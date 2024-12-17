/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: cam pmic dev driver
 */
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regmap.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/regulator/of_regulator.h>
#include <securec.h>

#include "et5907_regulator.h"
#include "xpmic_regulator.h"

#define DEFAULT_IRQ_EVENT_DELAY 20 /* 100mS */
#define ET5907_MAX_DEFER_TIME 6 /* 6s */

static int dbg_enable = 1;
module_param_named(dbg_level, dbg_enable, int, 0644);

static struct mutex pmic_mutex_et5907;
static struct regulator_dev *et5907_rdevs[ET5907_MAX_REGULATORS];

static const struct regmap_config et5907_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = ET5907_REG_CONTROL,
};

static int et5907_regulator_enable(struct regulator_dev *rdev)
{
	pr_info("xpmic et5907 enable enter");
	struct et5907_regulator *info = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	int ret;
	pr_info("%s  id= %d\n", __func__, id);
	if (info == NULL) {
		pr_err("regulator info null pointer\n");
		return -EINVAL;
	}
	mutex_lock(&pmic_mutex_et5907);
	ret = regmap_update_bits(rdev->regmap, ET5907_REG_LDO_EN,
		rdev->desc->vsel_mask, rdev->desc->vsel_mask);
	pr_info("et5907 enable ret = %d", ret);
	mutex_unlock(&pmic_mutex_et5907);
	return ret;
}

static int et5907_regulator_disable(struct regulator_dev *rdev)
{
	struct et5907_regulator *info = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	int ret;
	pr_info("%s id= %d\n", __func__, id);
	if (info == NULL) {
		pr_err("regulator info null pointer\n");
		return -EINVAL;
	}
	mutex_lock(&pmic_mutex_et5907);
	ret = regmap_update_bits(rdev->regmap, ET5907_REG_LDO_EN,
		rdev->desc->vsel_mask, 0);
	pr_info("et5907 disable ret = %d", ret);
	mutex_unlock(&pmic_mutex_et5907);
	return ret;
}

static int et5907_regulator_is_enabled(struct regulator_dev *rdev)
{
	pr_info("xpmic et5907 isenable enter");
	struct et5907_regulator *info = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	int val;

	if (info == NULL) {
		pr_err("regulator info null pointer\n");
		return -EINVAL;
	}
	regmap_read(rdev->regmap, ET5907_REG_LDO_EN, &val);
	pr_info("%s, id=%d, val= %d, vsel_mask=%d",
			__func__, id, val, rdev->desc->vsel_mask);
	if (val & rdev->desc->vsel_mask)
		return 1;

	return 0;
}

static int et5907_regulator_set_voltage(struct regulator_dev *rdev, int min_uv,
	int max_uv, unsigned *selector)
{
	pr_info("Xpmic et5907 set voltage enter\n");
	struct et5907_regulator *info = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	u8 val = 0;
	int ret = 0;
	int mask = rdev->desc->vsel_mask;

	pr_info("et5907_set_voltage id= %d min_uv=%d max_uv=%d mask=%d", id,
		min_uv, max_uv, mask);
	if (info == NULL) {
		pr_err("regulator info null pointer\n");
		return -EINVAL;
	}

	if (id == ET5907_ID_REGULATOR1 || id == ET5907_ID_REGULATOR2) {
	/* check voltage range */
		if (min_uv > 1800000 || min_uv < 600000) {
			pr_err("min_uv or max_uv out of range\n");
			return -EINVAL;
		}
		val = (min_uv - 600000) / 6000;
	} else {
	/* check voltage range */
		if (min_uv > 3750000 || min_uv < 1200000) {
			pr_err("min_uv or max_uv out of range\n");
			return -EINVAL;
		}
		val = (min_uv - 1200000) / 10000;
	}
	ret = regmap_write(rdev->regmap, et5907_ldo_reg_index(id), val);
	if (ret < 0) {
		pr_err("Failed to et5907_set_voltage: %d\n", ret);
		return ret;
	}

	return ret;
}

static int et5907_regulator_get_voltage(struct regulator_dev *rdev)
{
	pr_info("xpmic et5907_get_voltage enter");
	int id = rdev_get_id(rdev);
	int err;
	int val = 0;

	err = regmap_read(rdev->regmap, et5907_ldo_reg_index(id), &val);
	if (err < 0) {
		pr_err("Failed to et5907_get_voltage: %d\n", err);
		return -EINVAL;
	}
	if (id == ET5907_ID_REGULATOR1 || id == ET5907_ID_REGULATOR2) {
	/* calculate real voltage */
		pr_info("et5907_get_voltage %d", (val * 6000 + 600000));
		return val * 6000 + 600000;
	} else {
	/* calculate real voltage */
		pr_info("et5907_get_voltage %d", (val * 10000 + 1200000));
		return val * 10000 + 1200000;
	}
}

static const struct regulator_ops et5907_regulator_ops = {
	.enable = et5907_regulator_enable,
	.disable = et5907_regulator_disable,
	.is_enabled = et5907_regulator_is_enabled,
	.set_voltage = et5907_regulator_set_voltage,
	.get_voltage = et5907_regulator_get_voltage,
};

int et5907_regulator_enable_warpper(struct regulator_dev *rdev)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return et5907_regulator_enable(xpmic->real_rdev);
}

int et5907_regulator_disable_warpper(struct regulator_dev *rdev)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return et5907_regulator_disable(xpmic->real_rdev);
}

int et5907_regulator_is_enabled_warpper(struct regulator_dev *rdev)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return et5907_regulator_is_enabled(xpmic->real_rdev);
}

int et5907_regulator_set_voltage_warpper(struct regulator_dev *rdev, int min_uv, int max_uv, unsigned *selector)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return et5907_regulator_set_voltage(xpmic->real_rdev, min_uv, max_uv, selector);
}

int et5907_regulator_get_voltage_warpper(struct regulator_dev *rdev)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return et5907_regulator_get_voltage(xpmic->real_rdev);
}

struct regulator_dev *et5907_get_rdevs(int index)
{
	if (index < ET5907_ID_REGULATOR1 || index > ET5907_ID_REGULATOR7)
		return ERR_PTR(-EINVAL);
	return et5907_rdevs[index];
}

static struct regulator_desc et5907_regulator_desc[ET5907_MAX_REGULATORS] = {
	{
		.name = "et07_ldo1",
		.id = ET5907_ID_REGULATOR1,
		.ops = &et5907_regulator_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_mask = BIT(0),
		.n_voltages = 1,
	},
	{
		.name = "et07_ldo2",
		.id = ET5907_ID_REGULATOR2,
		.ops = &et5907_regulator_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_mask = BIT(1),
		.n_voltages = 1,
	},
	{
		.name = "et07_ldo3",
		.id = ET5907_ID_REGULATOR3,
		.ops = &et5907_regulator_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_mask = BIT(2),
		.n_voltages = 1,
	},
	{
		.name = "et07_ldo4",
		.id = ET5907_ID_REGULATOR4,
		.ops = &et5907_regulator_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_mask = BIT(3),
		.n_voltages = 1,
	},
	{
		.name = "et07_ldo5",
		.id = ET5907_ID_REGULATOR5,
		.ops = &et5907_regulator_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_mask = BIT(4),
		.n_voltages = 1,
	},
	{
		.name = "et07_ldo6",
		.id = ET5907_ID_REGULATOR6,
		.ops = &et5907_regulator_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_mask = BIT(5),
		.n_voltages = 1,
	},
	{
		.name = "et07_ldo7",
		.id = ET5907_ID_REGULATOR7,
		.ops = &et5907_regulator_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.vsel_mask = BIT(6),
		.n_voltages = 1,
	}
};

static int et5907_regulator_init(struct et5907_regulator *chip)
{
	struct regulator_config config = {};
	int i, ret;

	/* init disable ldos */
	ret = regmap_write(chip->regmap, ET5907_REG_LDO_EN, 0x00);
	if (ret < 0) {
		pr_err("Failed to set ET5907_REG_LDO_EN reg: %d\n",
			ret);
		return ret;
	}
	/* ET5907 SOFTRESET */
	ret = regmap_write(chip->regmap, ET5907_REG_RESET, 0x0B);
	if (ret < 0) {
		pr_err("Failed to set ET5907_REG_RESET reg: %d\n",
			ret);
		return ret;
	}
	/* Set up regulators */
	/* Register the regulators */
	for (i = 0; i < ET5907_MAX_REGULATORS; i++) {
		config.init_data = chip->pdata->init_data[i];
		config.dev = chip->dev;
		config.driver_data = chip;
		config.regmap = chip->regmap;
		config.of_node = chip->pdata->reg_node[i];
		chip->rdev[i] = devm_regulator_register(
			chip->dev, &et5907_regulator_desc[i], &config);
		et5907_rdevs[i] = chip->rdev[i];
		if (IS_ERR(chip->rdev[i])) {
			ret = PTR_ERR(chip->rdev[i]);
			pr_err(
				"Failed to register ET5907 regulator[ %s: %d]\n",
				et5907_regulator_desc[i].name, ret);
			return ret;
		}
	}

	return 0;
}

__attribute__((unused)) static struct of_regulator_match
	et5907_regulator_matches[ET5907_MAX_REGULATORS] = {
		[ET5907_ID_REGULATOR1] = { .name = "et07_ldo1" },
		[ET5907_ID_REGULATOR2] = { .name = "et07_ldo2" },
		[ET5907_ID_REGULATOR3] = { .name = "et07_ldo3" },
		[ET5907_ID_REGULATOR4] = { .name = "et07_ldo4" },
		[ET5907_ID_REGULATOR5] = { .name = "et07_ldo5" },
		[ET5907_ID_REGULATOR6] = { .name = "et07_ldo6" },
		[ET5907_ID_REGULATOR7] = { .name = "et07_ldo7" },
	};

static struct et5907_pdata *et5907_regulator_parse_dt(struct device *dev)
{
	pr_info("Xpmic et5907 parse enter\n");
	struct et5907_pdata *pdata;
	struct device_node *node;
	int i, ret, n;
	struct device_node *np = dev->of_node;

	if (np == NULL) {
		pr_err("Error: et-changer np = NULL\n");
		return ERR_PTR(-ENODEV);
	}
	node = of_get_child_by_name(dev->of_node, "regulators");
	if (!node) {
		pr_err("regulator node not found\n");
		return ERR_PTR(-ENODEV);
	}
	ret = of_regulator_match(dev, node, et5907_regulator_matches,
		ARRAY_SIZE(et5907_regulator_matches));
	of_node_put(node);
	if (ret < 0) {
		pr_err("Error parsing regulator init data: %d\n", ret);
		return ERR_PTR(-EINVAL);
	}
	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);
	n = 0;
	for (i = 0; i < ARRAY_SIZE(et5907_regulator_matches); i++) {
		if (!et5907_regulator_matches[i].init_data)
			continue;
		pdata->init_data[n] = et5907_regulator_matches[i].init_data;
		pdata->init_data[n]->constraints.valid_ops_mask |= REGULATOR_CHANGE_STATUS
				| REGULATOR_CHANGE_VOLTAGE
				| REGULATOR_CHANGE_MODE
				| REGULATOR_CHANGE_DRMS;
		pdata->reg_node[n] = et5907_regulator_matches[i].of_node;
		n++;
	}

	return pdata;
}

static int et5907_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	pr_info("Xpmic et5907 probe enter\n");
	int ret;
	unsigned int data;
	struct et5907_regulator *chip;
	unsigned long long time_from_boot;

	chip = devm_kzalloc(&i2c->dev, sizeof(struct et5907_regulator),
		GFP_KERNEL);
	if (!chip) {
		ret = -ENOMEM;
		goto error;
	}

	chip->dev = &i2c->dev;
	chip->regmap = devm_regmap_init_i2c(i2c, &et5907_regmap_config);
	if (IS_ERR(chip->regmap)) {
		ret = PTR_ERR(chip->regmap);
		pr_err("Failed to allocate register map: %d\n", ret);
		goto error;
	}

	chip->reset_gpio = of_get_named_gpio_flags(chip->dev->of_node, "et5907,reset", 0, NULL);
	ret = xpmic_gpio_ops(chip->reset_gpio, XPMIC_GPIO_PULL_HIGH);
	if (ret) {
		pr_err("GPIO[%d] request failed[%d] \n", chip->reset_gpio, ret);
		goto error;
	}

	i2c_set_clientdata(i2c, chip);

	chip->pdata = i2c->dev.platform_data;

	ret = regmap_read(chip->regmap, ET5907_REG_CHIPID, &data);
	if (ret < 0)
		pr_err("Failed to read DEVICE_ID reg: %d\n", ret);

	switch (data) {
	case ET5907_DEVICE_ID:
		chip->chip_id = ET5907;
		break;
	default:
		pr_err("Unsupported device id = 0x%x.\n", data);
		ret = -ENODEV;
		goto error;
	}

	if (!chip->pdata)
		chip->pdata = et5907_regulator_parse_dt(chip->dev);

	if (IS_ERR(chip->pdata)) {
		pr_err("No regulators defined for the platform\n");
		ret = PTR_ERR(chip->pdata);
		goto error;
	}

#ifdef ET5907_IRQ_EN
	chip->chip_irq = i2c->irq;

	if (chip->chip_irq != 0) {
		ret = regmap_write(chip->regmap, ET5907_REG_UVP_INTMA, 0x00);
		if (ret < 0) {
			pr_err(
				"Failed to mask ET5907_REG_UVP_INTMA reg: %d\n", ret);
			goto error;
		}

		ret = regmap_write(chip->regmap, ET5907_REG_OCP_INTMA, 0x00);
		if (ret < 0) {
			pr_err(
				"Failed to mask ET5907_REG_OCP_INTMA reg: %d\n", ret);
			goto error;
		}

		ret = regmap_write(chip->regmap, ET5907_REG_TSD_UVLO_INTMA, 0x00);
		if (ret < 0) {
			pr_err(
				"Failed to mask ET5907_REG_TSD_UVLO_INTMA reg: %d\n", ret);
			goto error;
		}

		ret = devm_request_threaded_irq(chip->dev, chip->chip_irq, NULL,
			et5907_irq_handler,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT,
			"et5907", chip);
		if (ret != 0) {
			pr_err("Failed to request IRQ: %d\n", chip->chip_irq);
			goto error;
		}
		INIT_DELAYED_WORK(&chip->irq_event_work, et5907_irq_event_work);
		chip->irq_num = 0;
	} else {
		dev_warn(chip->dev, "No IRQ configured\n");
	}
#endif

	ret = et5907_regulator_init(chip);
	if (ret < 0) {
		pr_err("Failed to initialize regulator: %d\n", ret);
		goto error;
	}

	mutex_init(&pmic_mutex_et5907);

	pr_info("initialize et5907 success");
	return ret;

error:
	pr_err("initialize et5907 failed");
	if (chip->reset_gpio) {
		ret = xpmic_gpio_ops(chip->reset_gpio, XPMIC_GPIO_PULL_LOW);
		mdelay(1); // Delay enough time to ensure valid gpio
		if (ret)
			pr_err("Error: release RESET GPIO %d\n", chip->reset_gpio);
	}

	time_from_boot = ktime_to_timespec64(ktime_get_boottime()).tv_sec;
	if (time_from_boot < (unsigned long long)ET5907_MAX_DEFER_TIME) {
		pr_err("et5907 defer probe\n");
		return -EPROBE_DEFER;
	}
	return ret;
}

static int et5907_i2c_remove(struct i2c_client *client)
{
	int ret = 0;
	struct et5907_regulator *chip = i2c_get_clientdata(client);
	if (chip->reset_gpio > 0)
		ret = xpmic_gpio_ops(chip->reset_gpio, XPMIC_GPIO_PULL_LOW);
	return ret;
}

static const struct i2c_device_id et5907_i2c_id[] = {
	{ "et5907", ET5907 },
	{},
};

MODULE_DEVICE_TABLE(i2c, et5907_i2c_id);

static const struct of_device_id et5907_dt_ids[] = {
	{ .compatible = "etek,et5907",
	  .data = &et5907_i2c_id[0]
	},
	{},
};

MODULE_DEVICE_TABLE(of, et5907_dt_ids);

static struct i2c_driver et5907_regulator_driver = {
	.driver = {
		.name = "et5907",
		.of_match_table = of_match_ptr(et5907_dt_ids),
	},
	.probe    = et5907_i2c_probe,
	.id_table = et5907_i2c_id,
	.remove   = et5907_i2c_remove,
};

int et5907_regulator_i2c_init(void)
{
	pr_debug("%s", __func__);
	pr_err("Xpmic et5907 init yuelin\n");
	pr_info("Xpmic et5907 init\n");
	return i2c_add_driver(&et5907_regulator_driver);
}

void et5907_regulator_i2c_exit(void)
{
	pr_debug("%s", __func__);
	i2c_del_driver(&et5907_regulator_driver);
}

MODULE_AUTHOR("HUAWEI");
MODULE_DESCRIPTION("et5907 regulator driver");
MODULE_LICENSE("GPL");