// SPDX-License-Identifier: GPL-2.0
/*
 * sc8510.c
 *
 * sc8510 driver
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

#include "sc8510.h"
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG sc8510
HWLOG_REGIST();

static int sc8510_read_byte(struct sc8510_dev_info *di, u8 reg, u8 *data)
{
	if (!di || !data) {
		hwlog_err("read_byte: di is null\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte(di->client, reg, data);
}

static int sc8510_write_byte_mask(struct sc8510_dev_info *di, u8 reg, u8 mask, u8 shift, u8 value)
{
	if (!di) {
		hwlog_err("write_byte_mask: di is null\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte_mask(di->client, reg, value, mask, shift);
}

static int sc8510_fix_freq_mode_enable(struct sc8510_dev_info *di, bool enable)
{
	int ret;

	ret = sc8510_write_byte_mask(di, SC8510_FSW_CTRL_REG,
		SC8510_FIX_FREQ_EN_MASK, SC8510_FIX_FREQ_EN_SHIFT, enable);

	hwlog_info("[fix_freq_mode_enable] %s, ret=%d\n", enable ? "enable" : "disable", ret);
	return ret;
}

static int sc8510_notifier_call(struct notifier_block *nb, unsigned long event, void *data)
{
	struct sc8510_dev_info *di = container_of(nb, struct sc8510_dev_info, event_nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_USB_DISCONNECT:
		sc8510_fix_freq_mode_enable(di, false);
		break;
	case POWER_NE_USB_CONNECT:
		sc8510_fix_freq_mode_enable(di, true);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int sc8510_device_check(struct sc8510_dev_info *di)
{
	int ret;
	int i;
	int retry_times = 3; /* retry 3 times */
	u8 chip_id = 0;

	for (i = 0; i < retry_times; i++) {
		ret = sc8510_read_byte(di, SC8510_DEVICE_ID_REG, &chip_id);
		if (!ret) {
			hwlog_info("[device_check] chip_id=0x%x\n", chip_id);
			return 0;
		}

		power_usleep(DT_USLEEP_10MS);
	}

	hwlog_err("device_check: chip_id=0x%x, ret=%d\n", chip_id, ret);
	return ret;
}

static int sc8510_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct sc8510_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;

	ret = sc8510_device_check(di);
	if (ret)
		goto dev_check_fail;

	di->event_nb.notifier_call = sc8510_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &di->event_nb);
	if (ret)
		goto notifier_fail;

	i2c_set_clientdata(client, di);

	hwlog_info("probe ok\n");
	return 0;

notifier_fail:
dev_check_fail:
	i2c_set_clientdata(client, NULL);
	devm_kfree(&client->dev, di);
	return ret;
}

static int sc8510_remove(struct i2c_client *client)
{
	struct sc8510_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	(void)power_event_bnc_unregister(POWER_BNT_CONNECT, &di->event_nb);
	i2c_set_clientdata(client, NULL);

	return 0;
}

MODULE_DEVICE_TABLE(i2c, charge_pump_sc8510);
static const struct of_device_id sc8510_of_match[] = {
	{
		.compatible = "sc8510",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sc8510_i2c_id[] = {
	{ "sc8510", 0 },
	{},
};

static struct i2c_driver sc8510_driver = {
	.probe = sc8510_probe,
	.remove = sc8510_remove,
	.id_table = sc8510_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "sc8510",
		.of_match_table = of_match_ptr(sc8510_of_match),
	},
};

static int __init sc8510_init(void)
{
	return i2c_add_driver(&sc8510_driver);
}

static void __exit sc8510_exit(void)
{
	i2c_del_driver(&sc8510_driver);
}

module_init(sc8510_init);
module_exit(sc8510_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sc8510 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
