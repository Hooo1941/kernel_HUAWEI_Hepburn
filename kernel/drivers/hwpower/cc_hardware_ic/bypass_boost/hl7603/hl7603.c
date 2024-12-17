// SPDX-License-Identifier: GPL-2.0
/*
 * hl7603.c
 *
 * hl7603 driver
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

#include "hl7603.h"
#include <linux/workqueue.h>
#include <securec.h>
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG hl7603
HWLOG_REGIST();

#define HL7603_DSM_BUFF_SIZE                         64
#define HL7603_ABNORMAL_STATE_MASK                   0x86
#define HL7603_MONITOR_INTERVAL_DEFAULT              10000

static int hl7603_read_byte(struct hl7603_device_info *di, u8 reg, u8 *val)
{
	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, val);
}

static int hl7603_write_mask(struct hl7603_device_info *di,
	u8 reg, u8 mask, u8 shift, u8 value)
{
	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte_mask(di->client, reg, value, mask, shift);
}

static int hl7603_set_vout(struct hl7603_device_info *di, unsigned int vset)
{
	u8 vset_reg;
	char buf[HL7603_DSM_BUFF_SIZE] = { 0 };

	vset_reg = (vset - HL7603_VOUT_SEL_BASE) / HL7603_VOUT_SEL_STEP;
	if (hl7603_write_mask(di, HL7603_VOUT_SEL_ADDR,
		HL7603_VOUT_SEL_MASK, HL7603_VOUT_SEL_SHIFT, vset_reg)) {
		hwlog_err("fail to set vout\n");
		snprintf_s(buf, sizeof(buf), sizeof(buf) - 1,
			"deviceID=0x%x, fail to set vout\n", di->dev_id);
		hwlog_info("dmd content: %s\n", buf);
		power_dsm_report_dmd(POWER_DSM_BATTERY, POWER_DSM_BYPASS_BOOST_I2C_TRANS_FAIL, buf);
		return -EIO;
	}

	return 0;
}

static int hl7603_get_state(struct hl7603_device_info *di, u8 *state)
{
	u8 addr;
	char buf[HL7603_DSM_BUFF_SIZE] = { 0 };

	switch (di->dev_id) {
	case HL7603_DEVID_HL:
		addr = HL7603_STATE_ADDR_HL;
		break;
	case HL7603_DEVID_JWT:
		addr = HL7603_STATE_ADDR_JWT;
		break;
	default:
		hwlog_err("no chip match\n");
		return -EINVAL;
	}

	if (hl7603_read_byte(di, addr, state)) {
		hwlog_err("fail to get device state\n");
		snprintf_s(buf, sizeof(buf), sizeof(buf) - 1,
			"deviceID=0x%x, fail to get state\n", di->dev_id);
		hwlog_info("dmd content: %s\n", buf);
		power_dsm_report_dmd(POWER_DSM_BATTERY, POWER_DSM_BYPASS_BOOST_I2C_TRANS_FAIL, buf);
		return -EIO;
	}

	return 0;
}

static void hl7603_init_vout(struct hl7603_device_info *di)
{
	hwlog_info("init const_vout is %d\n", di->const_vout);
	hl7603_set_vout(di, di->const_vout);
}

static int hl7603_get_device_id(struct hl7603_device_info *di)
{
	int ret;
	u8 reg_val;

	ret = hl7603_read_byte(di, HL7603_DEVID_ADDR, &reg_val);
	if (ret) {
		hwlog_err("chek chip fail\n");
		return BYPASS_BST_IC_BAD;
	}
	di->dev_id = (reg_val & HL7603_DEVID_MASK) >> HL7603_DEVID_SHIFT;
	hwlog_info("device_id is 0x%x\n", di->dev_id);
	switch (di->dev_id) {
	case HL7603_DEVID_HL:
	case HL7603_DEVID_JWT:
		hwlog_info("hl7603 is good\n");
		return BYPASS_BST_IC_GOOD;
	default:
		hwlog_err("no chip match, return ic bad\n");
		return BYPASS_BST_IC_BAD;
	}
}

static void hl7603_parse_dts(struct device_node *np,
	struct hl7603_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"const_vout", &di->const_vout, HL7603_VOUT_SEL_DEFAULT);
	(void)power_gpio_config_output(np,
		"gpio_enable", "gpio_enable", &di->gpio_no, 1);
}

static bool hl7603_is_power_good(u8 state)
{
	return state & BIT(0);
}

static bool hl7603_is_fault(u8 state)
{
	return state & HL7603_ABNORMAL_STATE_MASK;
}

static void hl7603_monitor_work(struct work_struct *work)
{
	u8 state = 0;
	char buf[HL7603_DSM_BUFF_SIZE] = { 0 };
	struct hl7603_device_info *di = container_of(work,
		struct hl7603_device_info, monitor_work.work);

	if (!di)
		return;

	if (hl7603_get_state(di, &state))
		return;

	hwlog_info("hl7603 state = 0x%x\n", state);
	if (!hl7603_is_power_good(state) || hl7603_is_fault(state)) {
		snprintf_s(buf, sizeof(buf), sizeof(buf) - 1,
			"deviceID=0x%x, state=0x%x\n", di->dev_id, state);
		hwlog_info("dmd content: %s\n", buf);
		power_dsm_report_dmd(POWER_DSM_BATTERY, POWER_DSM_BYASS_BOOST_FAULT, buf);
	}

	schedule_delayed_work(&di->monitor_work,
		msecs_to_jiffies(HL7603_MONITOR_INTERVAL_DEFAULT));
}

static int hl7603_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct device_node *np = NULL;
	struct hl7603_device_info *di = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;

	hl7603_parse_dts(np, di);
	ret = hl7603_get_device_id(di);
	if (ret != BYPASS_BST_IC_GOOD)
		goto get_devid_fail;
	INIT_DELAYED_WORK(&di->monitor_work, hl7603_monitor_work);
	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = di->dev_id;
		power_dev_info->ver_id = 0;
	}

	hl7603_init_vout(di);
	i2c_set_clientdata(client, di);
	schedule_delayed_work(&di->monitor_work, msecs_to_jiffies(0));
	return 0;

get_devid_fail:
	devm_kfree(&client->dev, di);
	return -EINVAL;
}

static int hl7603_remove(struct i2c_client *client)
{
	struct hl7603_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	i2c_set_clientdata(client, NULL);
	cancel_delayed_work_sync(&di->monitor_work);
	return 0;
}

static void hl7603_shutdown(struct i2c_client *client)
{
	struct hl7603_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	cancel_delayed_work_sync(&di->monitor_work);
}

MODULE_DEVICE_TABLE(i2c, bypass_bst_hl7603);
static const struct of_device_id hl7603_of_match[] = {
	{
		.compatible = "huawei,bypass_bst_hl7603",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id hl7603_i2c_id[] = {
	{ "bypass_bst_hl7603", 0 },
	{},
};

static struct i2c_driver hl7603_driver = {
	.probe = hl7603_probe,
	.remove = hl7603_remove,
	.id_table = hl7603_i2c_id,
	.shutdown = hl7603_shutdown,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bypass_bst_hl7603",
		.of_match_table = of_match_ptr(hl7603_of_match),
	},
};

static int __init hl7603_init(void)
{
	return i2c_add_driver(&hl7603_driver);
}

static void __exit hl7603_exit(void)
{
	i2c_del_driver(&hl7603_driver);
}

fs_initcall_sync(hl7603_init);
module_exit(hl7603_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hl7603 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
