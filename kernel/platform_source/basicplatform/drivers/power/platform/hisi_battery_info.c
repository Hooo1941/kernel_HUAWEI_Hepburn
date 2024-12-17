/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description:  battery info for power supply
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <securec.h>

#include <platform_include/basicplatform/linux/power/platform/coul/coul_merge_drv.h>

#define bci_debug(fmt, args...) pr_debug("[batt_info]" fmt, ## args)
#define bci_info(fmt, args...) pr_info("[batt_info]" fmt, ## args)
#define bci_warn(fmt, args...) pr_warn("[batt_info]" fmt, ## args)
#define bci_err(fmt, args...) pr_err("[batt_info]" fmt, ## args)

static enum power_supply_property battery_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_BRAND,
	POWER_SUPPLY_PROP_BAT_QMAX,
};

static int get_property_extra(enum power_supply_property psp,
	union power_supply_propval *val, int batt)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = coul_merge_drv_battery_fcc_design(batt);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		val->intval = coul_merge_drv_battery_fcc(batt);
		break;
	case POWER_SUPPLY_PROP_BRAND:
		val->strval = coul_merge_drv_battery_brand(batt);
		break;
	case POWER_SUPPLY_PROP_BAT_QMAX:
		val->intval = coul_merge_drv_battery_get_qmax(batt);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int batt_info_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	int batt;
	int ret = 0;

	if(coul_merge_drv_get_batt_index(psy->desc->name, &batt)) {
		bci_err("[%s] battery name is error\n", __func__);
		return -EINVAL;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = coul_merge_drv_battery_voltage_uv(batt);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		/* Charging is Positive value, discharge is negative */
		val->intval = coul_merge_drv_battery_current(batt);
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		/* Charging is Positive value, discharge is negative */
		val->intval = coul_merge_drv_battery_current_avg(batt);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = coul_merge_drv_battery_temperature(batt) * TENTH;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = coul_merge_drv_is_battery_exist(batt);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = coul_merge_drv_battery_capacity(batt);
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = coul_merge_drv_battery_cycle_count(batt);
		break;
	default:
		ret = get_property_extra(psp, val, batt);
	}

	bci_info("[%s] batt[%d] name %s, val %d\n", __func__, batt, psy->desc->name, val->intval);
	return ret;
}


static char *batt_supplied_to[] = {
	"battery_info",
};

static const struct power_supply_config battery_info_cfg = {
	.supplied_to = batt_supplied_to,
	.num_supplicants = ARRAY_SIZE(batt_supplied_to),
};

static const struct power_supply_desc _battery_desc = {
	/* BCI has registered POWER_SUPPLY_TYPE_BATTERY
	  Healthd cannot register multiple POWER_SUPPLY_TYPE_BATTERY. */
	.type = POWER_SUPPLY_TYPE_UNKNOWN,
	.properties = battery_props,
	.num_properties = ARRAY_SIZE(battery_props),
	.get_property = batt_info_get_property,
};

static int batt_info_probe(struct platform_device *pdev)
{
	struct power_supply_desc *battery_desc = NULL;
	char *name = NULL;
	struct power_supply *psy = NULL;
	int i;

	for (i = 0; i < BATT_MAX; i++) {
		name = kzalloc(BATT_NAME_SIZE_MAX, GFP_KERNEL);
		if (!name)
			return -ENOMEM;

		if (coul_merge_drv_get_batt_name(i, name, BATT_NAME_SIZE_MAX))
			continue;

		battery_desc = kzalloc(sizeof(*battery_desc), GFP_KERNEL);
		if (!battery_desc)
			return -ENOMEM;
		(void)memcpy_s(battery_desc, sizeof(*battery_desc), &_battery_desc, sizeof(_battery_desc));
		battery_desc->name = name;
		psy = power_supply_register(&pdev->dev,	battery_desc, &battery_info_cfg);
		if (IS_ERR(psy)) {
			bci_err("[%s]power_supply_register failed, batt %d, name %s\n",
				__func__, i, name);
			return -EINVAL;
		} else {
			bci_info("[%s]power_supply_register succ, batt %d, name %s\n",
				__func__, i, name);
		}
	}

	bci_info("%d succ\n", __func__);
	return 0;
}

static const struct of_device_id batt_info_match_table[] = {
	{
		.compatible = "hisilicon,batt_info",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver batt_info_driver = {
	.probe = batt_info_probe,
	.driver = {
		.name = "batt_info",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(batt_info_match_table),
	},
};

static int __init batt_info_init(void)
{
	return platform_driver_register(&batt_info_driver);
}

module_init(batt_info_init);

static void __exit batt_info_exit(void)
{
	platform_driver_unregister(&batt_info_driver);
}

module_exit(batt_info_exit);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("battery info");
