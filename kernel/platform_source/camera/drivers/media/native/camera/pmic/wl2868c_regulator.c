/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All Rights Reserved.
 *
 * Description: wl2868c driver
 */

#include <linux/err.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>

#include "wl2868c_regulator.h"
#include "xpmic_regulator.h"

#define WL2868C_MAX_DEFER_TIME 6 /* 6s */

struct wl2868c {
	struct device *dev;
	struct regmap *regmap;
	struct regulator_desc *rdesc;
	struct regulator_dev *rdev;
	struct regulator *parent_supply;
	struct regulator *en_supply;
	struct device_node *of_node;
	u16 base;
	int chip_cs_pin;
	int min_dropout_uv;
};

struct wl2868c_evt_sta {
	unsigned int sreg;
};

static const struct wl2868c_evt_sta wl2868c_status_reg = { WL2868C_LDO_EN };

struct enable_table {
	int ldo1_enabled;
	int ldo2_enabled;
	int ldo3_enabled;
	int ldo4_enabled;
	int ldo5_enabled;
	int ldo6_enabled;
	int ldo7_enabled;
};

static struct enable_table rgltr_enable_table = {0, 0, 0, 0, 0, 0, 0};

static struct regulator_dev *wl2868c_rdevs[WL2868C_MAX_REGULATORS];

static int g_ldo_mask = 0x80;
static struct mutex pmic_mutex_wl2868c;

struct regulator_data {
	char  *name;
	char  *supply_name;
	int    min_dropout_uv;
};

static struct regulator_data reg_data[] = {
	/* name,        parent,   headroom */
	{"wl2868c_l1", "vdd_l1_l2",  80000},
	{"wl2868c_l2", "vdd_l1_l2",  80000},
	{"wl2868c_l3", "vdd_l3_l4",  80000},
	{"wl2868c_l4", "vdd_l3_l4",  80000},
	{"wl2868c_l5", "vdd_l5",     80000},
	{"wl2868c_l6", "vdd_l6",     80000},
	{"wl2868c_l7", "vdd_l7",     80000},
};

static struct reg_default wl2868c_reg_defs[] = {
	{WL2868C_DISCHARGE_RESISTORS,        0x00},
	{WL2868C_LDO1_LDO2_SEQ,              0x00},
	{WL2868C_LDO3_LDO4_SEQ,              0x00},
	{WL2868C_LDO5_LDO6_SEQ,              0x00},
	{WL2868C_LDO7_SEQ,                   0x00},
	{WL2868C_SEQ_STATUS,                 0x00},
	{WL2868C_LDO1_OCP_CTL,               0xC0},
	{WL2868C_LDO2_OCP_CTL,               0xC0},
	{WL2868C_LDO3_OCP_CTL,               0xC0},
	{WL2868C_LDO4_OCP_CTL,               0xC0},
	{WL2868C_LDO5_OCP_CTL,               0xC0},
	{WL2868C_LDO6_OCP_CTL,               0xC0},
	{WL2868C_LDO7_OCP_CTL,               0xC0},
	{WL2868C_REPROGRAMMABLE_I2C_ADDRESS, 0x00},
	{INT_LATCHED_CLR,                    0x00},
	{INT_EN_SET,                         0x00},
	{UVLO_CTL,                           0x1E},
};

static const struct regmap_range wl2868c_writeable_ranges[] = {
	regmap_reg_range(WL2868C_DISCHARGE_RESISTORS, WL2868C_SEQ_STATUS),
	regmap_reg_range(WL2868C_LDO1_OCP_CTL, WL2868C_LDO1_OCP_CTL),
	regmap_reg_range(WL2868C_LDO2_OCP_CTL, WL2868C_LDO2_OCP_CTL),
	regmap_reg_range(WL2868C_LDO3_OCP_CTL, WL2868C_LDO3_OCP_CTL),
	regmap_reg_range(WL2868C_LDO4_OCP_CTL, WL2868C_LDO4_OCP_CTL),
	regmap_reg_range(WL2868C_LDO5_OCP_CTL, WL2868C_LDO5_OCP_CTL),
	regmap_reg_range(WL2868C_LDO6_OCP_CTL, WL2868C_LDO6_OCP_CTL),
	regmap_reg_range(WL2868C_LDO7_OCP_CTL, WL2868C_REPROGRAMMABLE_I2C_ADDRESS),
	regmap_reg_range(INT_LATCHED_CLR, INT_LATCHED_CLR),
	regmap_reg_range(INT_EN_SET, INT_EN_SET),
	regmap_reg_range(UVLO_CTL, UVLO_CTL),
};

static const struct regmap_range wl2868c_readable_ranges[] = {
	regmap_reg_range(WL2868C_DEVICE_D1, RESERVED_2),
};

static const struct regmap_range wl2868c_volatile_ranges[] = {
	regmap_reg_range(WL2868C_DISCHARGE_RESISTORS, WL2868C_SEQ_STATUS),
	regmap_reg_range(WL2868C_LDO1_OCP_CTL, WL2868C_LDO1_OCP_CTL),
	regmap_reg_range(WL2868C_LDO2_OCP_CTL, WL2868C_LDO2_OCP_CTL),
	regmap_reg_range(WL2868C_LDO3_OCP_CTL, WL2868C_LDO3_OCP_CTL),
	regmap_reg_range(WL2868C_LDO4_OCP_CTL, WL2868C_LDO4_OCP_CTL),
	regmap_reg_range(WL2868C_LDO5_OCP_CTL, WL2868C_LDO5_OCP_CTL),
	regmap_reg_range(WL2868C_LDO6_OCP_CTL, WL2868C_LDO6_OCP_CTL),
	regmap_reg_range(WL2868C_LDO7_OCP_CTL, WL2868C_REPROGRAMMABLE_I2C_ADDRESS),
	regmap_reg_range(INT_LATCHED_CLR, INT_LATCHED_CLR),
	regmap_reg_range(INT_EN_SET, INT_EN_SET),
	regmap_reg_range(UVLO_CTL, UVLO_CTL),
	regmap_reg_range(RESERVED_2, RESERVED_2),
};

static const struct regmap_access_table wl2868c_writeable_table = {
	.yes_ranges   = wl2868c_writeable_ranges,
	.n_yes_ranges = ARRAY_SIZE(wl2868c_writeable_ranges),
};

static const struct regmap_access_table wl2868c_readable_table = {
	.yes_ranges   = wl2868c_readable_ranges,
	.n_yes_ranges = ARRAY_SIZE(wl2868c_readable_ranges),
};

static const struct regmap_access_table wl2868c_volatile_table = {
	.yes_ranges   = wl2868c_volatile_ranges,
	.n_yes_ranges = ARRAY_SIZE(wl2868c_volatile_ranges),
};

static const struct regmap_config wl2868c_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = RESERVED_2,
};

static int wl2868c_regulator_enable(struct regulator_dev *rdev)
{
	struct wl2868c *wl2868c_reg = rdev_get_drvdata(rdev);
	int rc = 0;
	switch (rdev->desc->id) {
	case WL2868C_REGULATOR_LDO1:
		rgltr_enable_table.ldo1_enabled = 1;
		break;
	case WL2868C_REGULATOR_LDO2:
		rgltr_enable_table.ldo2_enabled = 1;
		break;
	case WL2868C_REGULATOR_LDO3:
		rgltr_enable_table.ldo3_enabled = 1;
		break;
	case WL2868C_REGULATOR_LDO4:
		rgltr_enable_table.ldo4_enabled = 1;
		break;
	case WL2868C_REGULATOR_LDO5:
		rgltr_enable_table.ldo5_enabled = 1;
		break;
	case WL2868C_REGULATOR_LDO6:
		rgltr_enable_table.ldo6_enabled = 1;
		break;
	case WL2868C_REGULATOR_LDO7:
		rgltr_enable_table.ldo7_enabled = 1;
		break;
	default:
		rc = -1;
		break;
	}

	mutex_lock(&pmic_mutex_wl2868c);
	g_ldo_mask |= (1<<rdev->desc->id);
	rc = regmap_write(wl2868c_reg->regmap, WL2868C_LDO_EN, g_ldo_mask);
	mutex_unlock(&pmic_mutex_wl2868c);
	if (rc < 0) {
		dev_err(wl2868c_reg->dev, "Enable LDO output failed!!! \n");
		return rc;
	}
	pr_info("### wl2868c enable id %d name %s rc %d #### .... \n", rdev->desc->id, rdev->desc->name, rc);
	return rc;
}

static int wl2868c_regulator_disable(struct regulator_dev *rdev)
{
	struct wl2868c *wl2868c_reg = rdev_get_drvdata(rdev);
	int rc = 0;
	switch (rdev->desc->id) {
	case WL2868C_REGULATOR_LDO1:
		rgltr_enable_table.ldo1_enabled = 0;
		break;
	case WL2868C_REGULATOR_LDO2:
		rgltr_enable_table.ldo2_enabled = 0;
		break;
	case WL2868C_REGULATOR_LDO3:
		rgltr_enable_table.ldo3_enabled = 0;
		break;
	case WL2868C_REGULATOR_LDO4:
		rgltr_enable_table.ldo4_enabled = 0;
		break;
	case WL2868C_REGULATOR_LDO5:
		rgltr_enable_table.ldo5_enabled = 0;
		break;
	case WL2868C_REGULATOR_LDO6:
		rgltr_enable_table.ldo6_enabled = 0;
		break;
	case WL2868C_REGULATOR_LDO7:
		rgltr_enable_table.ldo7_enabled = 0;
		break;
	default:
		rc = -1;
		break;
	}
	mutex_lock(&pmic_mutex_wl2868c);
	g_ldo_mask &= ~((1<<rdev->desc->id));
	rc = regmap_write(wl2868c_reg->regmap, WL2868C_LDO_EN, g_ldo_mask);
	mutex_unlock(&pmic_mutex_wl2868c);
	if (rc < 0) {
		dev_err(wl2868c_reg->dev, "Disable LDO output failed!!! \n");
		return rc;
	}
	pr_info("wl2868c disable id %d %s rc %d \n", rdev->desc->id, rdev->desc->name, rc);
	return rc;
}

static int wl2868c_regulator_is_enabled(struct regulator_dev *rdev)
{
	int rc = 0;
	switch (rdev->desc->id) {
	case WL2868C_REGULATOR_LDO1:
		rc = rgltr_enable_table.ldo1_enabled;
		break;
	case WL2868C_REGULATOR_LDO2:
		rc = rgltr_enable_table.ldo2_enabled;
		break;
	case WL2868C_REGULATOR_LDO3:
		rc = rgltr_enable_table.ldo3_enabled;
		break;
	case WL2868C_REGULATOR_LDO4:
		rc = rgltr_enable_table.ldo4_enabled;
		break;
	case WL2868C_REGULATOR_LDO5:
		rc = rgltr_enable_table.ldo5_enabled;
		break;
	case WL2868C_REGULATOR_LDO6:
		rc = rgltr_enable_table.ldo6_enabled;
		break;
	case WL2868C_REGULATOR_LDO7:
		rc = rgltr_enable_table.ldo7_enabled;
		break;
	default:
		rc = -1;
		break;
	}
	pr_info("wl2868c IS_ENABLED enter id %d %s ret %d \n", rdev->desc->id, rdev->desc->name, rc);

	return rc;
}

/* common functions */
static int wl2868c_read(struct regmap *regmap,  u16 reg, u8 *val, int count)
{
	int rc;
	rc = regmap_bulk_read(regmap, reg, val, count);
	if (rc < 0)
		pr_err("failed to read 0x%04x\n", reg);

	return rc;
}

static int wl2868c_write(struct regmap *regmap, u16 reg, u8 *val, int count)
{
	int rc;

	pr_info("Writing 0x%02x to 0x%04x\n", val, reg);
	rc = regmap_bulk_write(regmap, reg, val, count);
	if (rc < 0)
		pr_err("failed to write 0x%04x\n", reg);

	return rc;
}


static int wl2868c_masked_write(struct regmap *regmap, u16 reg, u8 mask,
				u8 val)
{
	int rc;
	pr_debug("Writing 0x%02x to 0x%04x with mask 0x%02x\n", val, reg, mask);
	rc = regmap_update_bits(regmap, reg, mask, val);
	if (rc < 0)
		pr_err("failed to write 0x%02x to 0x%04x with mask 0x%02x\n",
						val, reg, mask);

	return rc;
}

static int wl2868c_write_voltage(struct wl2868c *wl2868c_reg, int min_uv,
				int max_uv)
{
	int rc = 0;
	int mv = 0;
	u8 vset_raw[2];

	mv = DIV_ROUND_UP(min_uv, 1000); // uv to mv
	if (mv * 1000 > max_uv) {
		wl2868c_err(wl2868c_reg,
			"requested voltage above maximum limit\n");
		return -EINVAL;
	}
	/*
	 * Each LSB of regulator is 1mV and the voltage setpoint
	 * should be multiple of 8mV(step).
	 */
	if (wl2868c_reg->base == WL2868C_LDO1_VOUT || wl2868c_reg->base == WL2868C_LDO2_VOUT)
		vset_raw[0] = (min_uv-496000)/8000;
	else
		vset_raw[0] = (min_uv-1504000)/8000;

	rc = wl2868c_write(wl2868c_reg->regmap, wl2868c_reg->base,
					vset_raw, 1);
	if (rc < 0) {
		wl2868c_err(wl2868c_reg, "failed to write voltage rc=%d\n", rc);
		return rc;
	}

	wl2868c_debug(wl2868c_reg, "VSET=[%x][%x]\n", vset_raw[1], vset_raw[0]);
	return 0;
}

static int wl2868c_regulator_set_voltage(struct regulator_dev *rdev,
				int min_uv, int max_uv, unsigned int *selector)
{
	struct wl2868c *wl2868c_reg = rdev_get_drvdata(rdev);
	int rc = 0;

	if (wl2868c_reg->parent_supply) {
		/* request on parent regulator with headroom */
		rc = regulator_set_voltage(wl2868c_reg->parent_supply,
						wl2868c_reg->min_dropout_uv + min_uv,
						INT_MAX);
		if (rc < 0) {
			wl2868c_err(wl2868c_reg,
					"failed to request parent supply voltage rc=%d\n",
				rc);
			return rc;
		}
	}

	rc = wl2868c_write_voltage(wl2868c_reg, min_uv, max_uv);
	if (rc < 0) {
		/* remove parent's voltage vote */
		if (wl2868c_reg->parent_supply)
			regulator_set_voltage(wl2868c_reg->parent_supply,
							0, INT_MAX);
	}

	wl2868c_debug(wl2868c_reg, "voltage set to %d\n", min_uv);
	return rc;
}


/* WL2868C LDO Regulator callbacks */
static int wl2868c_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct wl2868c *wl2868c_reg = rdev_get_drvdata(rdev);
	u8 vset_raw[2];
	int rc;

	rc = wl2868c_read(wl2868c_reg->regmap,
					wl2868c_reg->base,
					vset_raw, 2);
	if (rc < 0) {
		wl2868c_err(wl2868c_reg,
			"failed to read regulator voltage rc=%d\n", rc);
		return rc;
	}

	wl2868c_debug(wl2868c_reg, "VSET read [%x][%x]\n",
			vset_raw[1], vset_raw[0]);

	if (wl2868c_reg->base == WL2868C_LDO1_VOUT || wl2868c_reg->base == WL2868C_LDO2_VOUT)
		return (((vset_raw[0])) * 8 + 496) * 1000;
	else
		return (((vset_raw[0])) * 8000 + 1504000);
}

static const struct regulator_ops wl2868c_regl_ops = {
	.enable         = wl2868c_regulator_enable,
	.disable        = wl2868c_regulator_disable,
	.is_enabled     = wl2868c_regulator_is_enabled,
	.set_voltage    = wl2868c_regulator_set_voltage,
	.get_voltage    = wl2868c_regulator_get_voltage,
};


int wl2868c_regulator_enable_warpper(struct regulator_dev *rdev)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return wl2868c_regulator_enable(xpmic->real_rdev);
}

int wl2868c_regulator_disable_warpper(struct regulator_dev *rdev)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return wl2868c_regulator_disable(xpmic->real_rdev);
}

int wl2868c_regulator_is_enabled_warpper(struct regulator_dev *rdev)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return wl2868c_regulator_is_enabled(xpmic->real_rdev);
}

int wl2868c_regulator_set_voltage_warpper(struct regulator_dev *rdev, int min_uv, int max_uv, unsigned int *selector)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return wl2868c_regulator_set_voltage(xpmic->real_rdev, min_uv, max_uv, selector);
}

int wl2868c_regulator_get_voltage_warpper(struct regulator_dev *rdev)
{
	struct xpmic_info *xpmic = rdev_get_drvdata(rdev);
	return wl2868c_regulator_get_voltage(xpmic->real_rdev);
}

struct regulator_dev *wl2868c_get_rdev(int index)
{
	if (index < WL2868C_REGULATOR_LDO1 || index > WL2868C_REGULATOR_LDO7)
		return ERR_PTR(-EINVAL);
	return wl2868c_rdevs[index];
}

static int wl2868c_of_parse_cb(struct device_node *np, const struct regulator_desc *desc,
				struct regulator_config *config)
{
	int ena_gpio;

	ena_gpio = of_get_named_gpio(np, "enable-gpios", 0);
	if (gpio_is_valid(ena_gpio))
		config->ena_gpiod = gpio_to_desc(ena_gpio);

	return 0;
}

#define WL2868C_REGL_DESC(_id, _name, _min, _max, _step)    \
	[WL2868C_REGULATOR_##_id] = {                           \
		.name = #_name,                                     \
		.id  = WL2868C_REGULATOR_##_id,                     \
		.of_parse_cb = wl2868c_of_parse_cb,                 \
		.ops = &wl2868c_regl_ops,                           \
		.n_voltages = ((_max)-(_min))/(_step),              \
		.min_uV = (_min),                                   \
		.uV_step = (_step),                                 \
		.linear_min_sel = 0,                                \
		.vsel_mask = WL2868C_VSEL_MASK,                     \
		.vsel_reg = WL2868C_##_id##_VOUT,                   \
		.enable_reg = WL2868C_LDO_EN,                       \
		.enable_mask = BIT(WL2868C_REGULATOR_##_id),        \
		.type = REGULATOR_VOLTAGE,                          \
		.owner = THIS_MODULE,                               \
		.of_match = of_match_ptr(#_name),                   \
		.regulators_node = of_match_ptr("regulators"),      \
	}


static struct regulator_desc wl2868c_regls_desc[WL2868C_MAX_REGULATORS] = {
	WL2868C_REGL_DESC(LDO1, ldo1, 496000,  2536000,  8000),
	WL2868C_REGL_DESC(LDO2, ldo2, 496000,  2536000,  8000),
	WL2868C_REGL_DESC(LDO3, ldo3, 1476000, 3478000,  7850),
	WL2868C_REGL_DESC(LDO4, ldo4, 1476000, 3478000,  7850),
	WL2868C_REGL_DESC(LDO5, ldo5, 1476000, 3478000,  7850),
	WL2868C_REGL_DESC(LDO6, ldo6, 1476000, 3478000,  7850),
	WL2868C_REGL_DESC(LDO7, ldo7, 1476000, 3478000,  7850),
};

static int wl2868c_regulator_init(struct wl2868c *chip,	const char *name)
{
	struct regulator_config config = {};
	struct regulator_init_data *init_data;
	struct device *dev = chip->dev;
	struct device_node *reg_node = chip->of_node;
	struct regulator_desc *rdesc;
	int ret = 0;
	int init_voltage, i;

/* Disable all ldo output by default */
	ret = regmap_write(chip->regmap, WL2868C_LDO_EN, 0x80);
	if (ret < 0) {
		dev_err(chip->dev, "Disable all LDO output failed!!! \n");
		return ret;
	}
/* get regulator data */
	for (i = 0; i < WL2868C_MAX_REGULATORS; i++) {
		if (!strcmp(reg_data[i].name, name)) {
			break;
		}
	}
	if (i == WL2868C_MAX_REGULATORS) {
		pr_err("Invalid regulator name %s\n", name);
		return -EINVAL;
	}
	ret = of_property_read_u16(reg_node, "reg", &chip->base);
	if (ret < 0) {
		pr_err("%s: failed to get regulator base ret=%d\n", name, ret);
		return ret;
	}
	chip->min_dropout_uv = reg_data[i].min_dropout_uv;
	of_property_read_u32(reg_node, "will,min-dropout-voltage",
					&chip->min_dropout_uv);

	init_voltage = -EINVAL;
	of_property_read_u32(reg_node, "will,init-voltage", &init_voltage);

	init_data = of_get_regulator_init_data(dev, reg_node,
								chip->rdesc);
	if (init_data == NULL) {
		pr_err("%s: failed to get regulator data\n", name);
		return -ENODATA;
	}
	if (!init_data->constraints.name) {
		pr_err("%s: regulator name missing\n", name);
		return -EINVAL;
	}

	/* configure the initial voltage for the regulator */
	if (init_voltage > 0) {
		ret = wl2868c_write_voltage(chip, init_voltage,
						init_data->constraints.max_uV);
		if (ret < 0)
			pr_err("%s: failed to set initial voltage ret=%d\n",
							name, ret);
	}

	chip->rdesc = &wl2868c_regls_desc[i];
	chip->rdesc->name = init_data->constraints.name;
	rdesc = chip->rdesc;
	config.regmap = chip->regmap;
	config.dev = dev;
	config.driver_data = chip;
	config.of_node = reg_node;
	init_data->constraints.input_uV = init_data->constraints.max_uV;
	init_data->constraints.valid_ops_mask |= REGULATOR_CHANGE_STATUS
						| REGULATOR_CHANGE_VOLTAGE
						| REGULATOR_CHANGE_MODE
						| REGULATOR_CHANGE_DRMS;

	config.init_data = init_data;

	chip->rdev = devm_regulator_register(chip->dev, rdesc, &config);
	wl2868c_rdevs[i] = chip->rdev;
	if (IS_ERR(chip->rdev)) {
		ret = PTR_ERR(chip->rdev);
		dev_err(chip->dev, "Failed to register regulator(%s):%d \n", chip->rdesc->name, ret);

		return ret;
	}
	pr_info("wl2868_regulator_init: LDO%d, successful name %s", (i + 1), chip->rdesc->name);
	return 0;
}

/* PMIC probe and helper function */
static int wl2868c_parse_regulator(struct regmap *regmap, struct device *dev)
{
	int rc = 0;
	const char *name;
	struct device_node *child;
	struct wl2868c *wl2868c_reg;

	pr_info("%s: enter", __func__);
	/* parse each subnode and register regulator for regulator child */
	for_each_available_child_of_node(dev->of_node, child) {
		pr_info("%s-> for_each_available_child_of_node", __func__);
		wl2868c_reg = devm_kzalloc(dev, sizeof(*wl2868c_reg), GFP_KERNEL);
		if (!wl2868c_reg)
			return -ENOMEM;

		wl2868c_reg->regmap = regmap;
		wl2868c_reg->of_node = child;
		wl2868c_reg->dev = dev;

		rc = of_property_read_string(child, "regulator-name", &name);
		if (rc)
			continue;
		pr_info("%s: regulator-name = %s", __func__, name);
		rc = wl2868c_regulator_init(wl2868c_reg, name);
		if (rc < 0) {
			pr_err("failed to register regulator %s rc=%d\n",
							name, rc);
			return rc;
		}
	}

	return 0;
}

static int wl2868c_i2c_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct wl2868c *chip = NULL;
	unsigned int data;
	int ret;
	int cs_gpio = 0;
	unsigned long long time_from_boot;
	/*
		Set all register to initial value when probe driver to avoid register value was modified.
	 */
	pr_info("### wl2868c probe enter #### .... \n");
	chip = devm_kzalloc(dev, sizeof(struct wl2868c), GFP_KERNEL);
	if (!chip) {
		dev_err(chip->dev, "wl2868c_i2c_probe Memory error .... \n");
		ret = -ENOMEM;
		goto error;
	}
	/* probe step 1 init gpio WL2868_ENr */
	cs_gpio = of_get_named_gpio(dev->of_node, "will,cs-gpios", 0);

	ret = xpmic_gpio_ops(cs_gpio, XPMIC_GPIO_PULL_HIGH);
	if (ret) {
		pr_err("GPIO%d request failed:%d \n", cs_gpio, ret);
		goto error;
	}
	chip->chip_cs_pin = cs_gpio;
	mdelay(10);  /* wait for pmic init  */
	pr_info("#### wl2868c_i2c_probe cs_gpio: %d...\n", cs_gpio);
	/* probe step2 init i2c regmap */
	chip->dev = dev;
	chip->regmap = devm_regmap_init_i2c(client, &wl2868c_regmap_config);
	if (IS_ERR(chip->regmap)) {
		ret = PTR_ERR(chip->regmap);
		dev_err(dev, "Failed to allocate register map: %d \n", ret);
		goto error;
	}
	i2c_set_clientdata(client, chip);

	ret = regmap_read(chip->regmap, WL2868C_REG_CHIPID, &data);
	if (ret < 0) {
		pr_err("Failed to read DEVICE_ID reg: %d\n", ret);
		goto error;
	}
	pr_info("Chip id wl2868 = 0x%x.\n", data);

	/* set UVLO to 2.45 */
	ret = regmap_write(chip->regmap, UVLO_CTL, 0x13);
	if (ret < 0) {
		dev_err(chip->dev, "Set UVLO failed!!! \n");
		goto error;
	}

	/* set ldo4 ocp */
	ret = regmap_write(chip->regmap, WL2868C_LDO4_OCP_CTL, 0xF0);
	if (ret < 0) {
		dev_err(chip->dev, "Set ldo4 ocp failed!!!\n");
		goto error;
	}

	ret = wl2868c_parse_regulator(chip->regmap, dev);
	if (ret < 0) {
		pr_err("failed to parse device tree ret=%d\n", ret);
		goto error;
	}

	mutex_init(&pmic_mutex_wl2868c);
	pr_info("initialize wl2868c success\n");
	return ret;

error:
	pr_err("initialize wl2868c failed\n");
	if (chip->chip_cs_pin) {
		ret = xpmic_gpio_ops(chip->chip_cs_pin, XPMIC_GPIO_PULL_LOW);
		if (ret)
			pr_err("Error: release RESET GPIO %d\n", chip->chip_cs_pin);
	}

	time_from_boot = ktime_to_timespec64(ktime_get_boottime()).tv_sec;
	if (time_from_boot < (unsigned long long)WL2868C_MAX_DEFER_TIME) {
		pr_err("wl2868c defer probe\n");
		return -EPROBE_DEFER;
	}

	return ret;
}

static int wl2868c_i2c_remove(struct i2c_client *client)
{
	struct wl2868c *chip = i2c_get_clientdata(client);
	int ret = 0;

	if (chip->chip_cs_pin)
		ret = xpmic_gpio_ops(chip->chip_cs_pin, XPMIC_GPIO_PULL_LOW);
	return ret;
}

static const struct i2c_device_id wl2868c_i2c_id[] = {
	{"wl2868c-regulator", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, wl2868c_i2c_id);

static const struct of_device_id wl2868c_dt_ids[] = {
	{ .compatible = "qcom,wl2868c-regulator",
	  .data = &wl2868c_i2c_id[0]
	},
	{},
};

MODULE_DEVICE_TABLE(of, wl2868c_dt_ids);

static struct i2c_driver wl2868c_regulator_driver = {
	.driver = {
		.name = "wl2868c-regulator",
		.of_match_table = of_match_ptr(wl2868c_dt_ids),
	},
	.probe    = wl2868c_i2c_probe,
	.remove   = wl2868c_i2c_remove,
	.id_table = wl2868c_i2c_id,
};

int wl2868c_regulator_i2c_init(void)
{
	pr_debug("%s", __func__);
	return i2c_add_driver(&wl2868c_regulator_driver);
}

void wl2868c_regulator_i2c_exit(void)
{
	pr_debug("%s", __func__);
	i2c_del_driver(&wl2868c_regulator_driver);
}

MODULE_AUTHOR("WILL");
MODULE_DESCRIPTION("WL2868C regulator driver");
MODULE_LICENSE("GPL");
