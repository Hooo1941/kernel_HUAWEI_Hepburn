// SPDX-License-Identifier: GPL-2.0
/*
 * bq40z50.c
 *
 * bq40z50 driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "bq40z50.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <chipset_common/hwmanufac/dev_detect/dev_detect.h>
#endif
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

#include "../cbm8580/cbm8580_interface.h"

#define HWLOG_TAG bq40z50
HWLOG_REGIST();

int bq40z50_read_byte(struct bq40z50_device_info *di, u8 reg, u8 *value)
{
	if (!di) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

int bq40z50_write_byte(struct bq40z50_device_info *di, u8 reg, u8 value)
{
	if (!di) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

int bq40z50_read_word(struct bq40z50_device_info *di, u8 reg, u16 *data)
{
	int ret;

	if (!di)
		return -ENODEV;

	mutex_lock(&di->rw_lock);
	ret = power_i2c_u8_read_word(di->client, reg, data, false);
	mutex_unlock(&di->rw_lock);
	if (ret < 0)
		hwlog_err("reg=0x%x read fail\n", reg);

	return ret;
}

int bq40z50_write_word(struct bq40z50_device_info *di, u8 reg, u16 data)
{
	int ret;

	if (!di)
		return -ENODEV;

	mutex_lock(&di->rw_lock);
	ret = power_i2c_u8_write_word(di->client, reg, data);
	mutex_unlock(&di->rw_lock);
	if (ret < 0)
		hwlog_err("reg=0x%x write fail\n", reg);

	return ret;
}

int bq40z50_read_block(struct bq40z50_device_info *di, u8 reg, u8 *buf, u8 buf_len)
{
	int ret;
	u8 cmd[BQ40Z50_CMD_LEN] = { 0 };

	if (!di)
		return -ENODEV;

	cmd[0] = reg;
	mutex_lock(&di->rw_lock);
	ret = power_i2c_read_block(di->client, cmd, sizeof(cmd), buf, buf_len);
	mutex_unlock(&di->rw_lock);
	if (ret < 0)
		hwlog_err("reg=0x%x read block fail\n", reg);

	return ret;
}

int bq40z50_write_block(struct bq40z50_device_info *di, u8 reg, u8 *buf, u8 buf_len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !buf || (buf_len > BQ40Z50_BLOCK_LEN_MAX))
		return -EINVAL;

	cmd = kcalloc(BQ40Z50_CMD_LEN + buf_len, sizeof(u8), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	cmd[0] = reg;
	memcpy(&cmd[BQ40Z50_CMD_LEN], buf, buf_len);

	mutex_lock(&di->rw_lock);
	ret = power_i2c_write_block(di->client, cmd, BQ40Z50_CMD_LEN + buf_len);
	mutex_unlock(&di->rw_lock);
	if (ret < 0)
		hwlog_err("reg=0x%x write block fail\n", reg);
	kfree(cmd);

	return ret;
}

static int bq40z50_get_batt_para(struct bq40z50_device_info *di)
{
	if (!di)
		return -ENODEV;

	/* vbat_max */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), di->dev->of_node,
		"vbat_max", (u32 *)&di->vbat_max, BQ40Z50_BATT_MAX_VOLTAGE_DEFAULT);

	/* cell count */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), di->dev->of_node,
		"cell_count", (u32 *)&di->cell_count, BQ40Z50_BATT_CELL_COUNT_DEFAULT);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), di->dev->of_node,
		"power_saving_enable", (u32 *)&di->power_saving_enable, 0);

#if defined(CONFIG_COUL_DRV)
	/* smart charge support */
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"bq40z50_battery", "is_smart_battery", &di->is_smart_battery, true);

	/* prevent low cap */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), di->dev->of_node,
		"prevent_low_cap", (u32 *)&di->prevent_low_cap, 0);

	/* cutoff vol */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), di->dev->of_node,
		"cutoff_vol", (u32 *)&di->cutoff_vol, BQ40Z50_CUTOFF_VOL_DEFAULT);

	/* is using 2:1 converter */
	di->use_converter = of_property_read_bool(di->dev->of_node, "use_converter");

	/* soc jump calibrate */
	di->soc_cali_enable = of_property_read_bool(di->dev->of_node, "soc_cali_enable");

	return bq40z50_get_para_data(di);
#elif defined(CONFIG_HUAWEI_COUL)
	/* smart charge support */
	di->is_smart_battery = of_property_read_bool(di->dev->of_node, "is_smart_battery");

	return 0;
#endif
}

struct battery_dev_name_table {
	char *name;
	int battery_fuel_id;
};

static struct battery_dev_name_table battery_dev_tab[] = {
	{"T", BATTERY_FUEL_BQ40Z50},
	{"S", BATTERY_FUEL_SH366006},
	{"D", BATTERY_FUEL_CBM8580},
	{"A", BATTERY_FUEL_BQ40Z50},
	{"C", BATTERY_FUEL_SH366006},
};

static int bq40z50_check_chip(struct bq40z50_device_info *di)
{
	char device_name[BQ40Z50_BLOCK_LEN_MAX] = {0};
	int ret;
	int i = 0;
	int retry = BQ40Z50_RETRY_COUNT;

	while (retry--) {
		ret = bq40z50_read_block(di, BQ40Z50_REG_DEVICE_NAME, device_name, BQ40Z50_BLOCK_LEN_MAX);
		if (!ret)
			break;
		power_msleep(DT_MSLEEP_10MS, 0, NULL);
	}
	if (ret != 0) {
		hwlog_err("read device name 3 times fail\n");
		return ret;
	}
	hwlog_info("device name : %s\n", device_name);
	for (i = 0; i < ARRAY_SIZE(battery_dev_tab); i++) {
		if (battery_dev_tab[i].name[0] == 
			device_name[BQ40Z50_FW_DEVICE_NAME_FW_VERSION_INDEX]) {
			di->device_id = battery_dev_tab[i].battery_fuel_id;
			hwlog_info("device id is %d\n", di->device_id);
			return 0;
		}
	}
	di->device_id = BATTERY_FUEL_INVALID;
	hwlog_err("device name not match\n");

	return ret;
}

static int bq40z50_ops_register(struct bq40z50_device_info *di)
{
#if defined(CONFIG_COUL_DRV)
	return bq40z50_ops_register_hisi(di);
#elif defined(CONFIG_HUAWEI_COUL)
	return bq40z50_ops_register_base(di);
#endif
}

static int bq40z50_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = -ENOMEM;
	struct bq40z50_device_info *di = NULL;
	struct device_node *np = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return ret;

	di->cache.vol = BQ40Z50_BATT_VOLTAGE_DEFAULT;
	di->cache.temp = BQ40Z50_BATT_TEMP_ABNORMAL_LOW - 1;
	di->cache.soc = BQ40Z50_BATT_CAPACITY_DEFAULT;
	di->charge_status = BQ40Z50_CHARGE_STATE_NOT_CHARGING;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	mutex_init(&di->rw_lock);
#if defined(CONFIG_COUL_DRV)
	bq40z50_set_di(di);
#endif
	ret = bq40z50_check_chip(di);
	if (ret)
		goto dev_check_fail;

	ret = bq40z50_get_batt_para(di);
	if (ret)
		goto get_para_fail;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_GAUGE_IC);
#endif

	ret = bq40z50_ops_register(di);
	if (ret)
		goto register_coul_ops_fail;
	
	if (di->device_id == BATTERY_FUEL_CBM8580)
		cbm8580_power_saving_restore(di);

	return 0;

register_coul_ops_fail:
#ifdef CONFIG_COUL_DRV
	kfree(di->para_batt_data);
	di->para_batt_data = NULL;
#endif
get_para_fail:
#if defined(CONFIG_COUL_DRV)
	bq40z50_set_di(NULL);
#endif
dev_check_fail:
	mutex_destroy(&di->rw_lock);
	kfree(di);
	return ret;
}

static int bq40z50_remove(struct i2c_client *client)
{
	struct bq40z50_device_info *di = i2c_get_clientdata(client);

#if defined(CONFIG_COUL_DRV)
	if (!di || !di->para_batt_data)
		return -ENODEV;

	kfree(di->para_batt_data);
	di->para_batt_data = NULL;
	bq40z50_set_di(NULL);
#elif defined(CONFIG_HUAWEI_COUL)
	if (!di)
		return -ENODEV;
#endif
	if (di->device_id == BATTERY_FUEL_CBM8580)
		cbm8580_power_saving_free_resource(di);

	mutex_destroy(&di->rw_lock);
	kfree(di);

	return 0;
}

static void bq40z50_shutdown(struct i2c_client *client)
{
	struct bq40z50_device_info *di = i2c_get_clientdata(client);
 
	if (!di)
		return;
	
	if (di->device_id == BATTERY_FUEL_CBM8580)
		cbm8580_power_saving_shutdown(di);
}

MODULE_DEVICE_TABLE(i2c, bq40z50);
static const struct of_device_id bq40z50_of_match[] = {
	{
		.compatible = "huawei,bq40z50",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq40z50_i2c_id[] = {
	{ "bq40z50", 0 },
	{},
};

static struct i2c_driver bq40z50_driver = {
	.probe = bq40z50_probe,
	.remove = bq40z50_remove,
	.shutdown = bq40z50_shutdown,
	.id_table = bq40z50_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq40z50_fg",
		.of_match_table = of_match_ptr(bq40z50_of_match),
	},
};

static int __init bq40z50_init(void)
{
	return i2c_add_driver(&bq40z50_driver);
}

static void __exit bq40z50_exit(void)
{
	i2c_del_driver(&bq40z50_driver);
}

rootfs_initcall(bq40z50_init);
module_exit(bq40z50_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq40z50 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
