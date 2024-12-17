/*
 * drivers/huawei/drivers/isl98608.c
 *
 * lcdkit backlight function for lcd driver
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/param.h>
#include <linux/delay.h>
#include <linux/idr.h>
#include <linux/i2c.h>
#include <asm/unaligned.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/of.h>

#include "isl98608.h"

#if defined(CONFIG_LCD_KIT_DRIVER)
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#endif

static struct isl98608_device_info *isl98608_client = NULL;
static bool is_isl98608_device = false;

static struct isl98608_voltage vol_table[] = {
	{5500000, ISL98608_VOL_55},
	{5600000, ISL98608_VOL_56},
	{5700000, ISL98608_VOL_57},
	{5800000, ISL98608_VOL_58},
	{5900000, ISL98608_VOL_59},
	{6000000, ISL98608_VOL_60},
};

static void isl98608_enable()
{
	if (isl98608_client != NULL) {
		gpiod_set_value_cansleep(gpio_to_desc(isl98608_client->config.vsp_enable), VSP_ENABLE);
		mdelay(2);
		gpiod_set_value_cansleep(gpio_to_desc(isl98608_client->config.vsn_enable), VSN_ENABLE);
	}
}

static int isl98608_reg_init(struct i2c_client *client, u8 vbst, u8 vpos, u8 vneg)
{
	int ret = 0;
	unsigned int reg_enable;

	if (client == NULL) {
		pr_err("[%s,%d]: NULL point for client\n", __FUNCTION__, __LINE__);
		goto exit;
	}

	ret = i2c_smbus_read_byte_data(client, ISL98608_REG_ENABLE);
	if (ret < 0) {
		pr_err("%s read reg_enable failed\n", __func__);
		goto exit;
	}
	reg_enable = ret;

	reg_enable = reg_enable | ISL98608_SHUTDOWN_BIT | ISL98608_VP_BIT | ISL98608_VN_BIT | ISL98608_VBST_BIT;

	ret = i2c_smbus_write_byte_data(client, ISL98608_REG_VBST, vbst);
	if (ret < 0) {
		pr_err("%s write vbst failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, ISL98608_REG_VPOS, vpos);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, ISL98608_REG_VNEG, vneg);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, ISL98608_REG_ENABLE, (u8)reg_enable);
	if (ret < 0) {
		pr_err("%s write reg_enable failed\n", __func__);
		goto exit;
	}

exit:
	return ret;
}

#if defined(CONFIG_LCD_KIT_DRIVER)
int isl98608_get_bias_voltage(int *vpos_target, int *vneg_target)
{
	int i = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct isl98608_voltage); i++) {
		if (vol_table[i].voltage == *vpos_target) {
			pr_err("isl98608 vsp voltage:0x%x\n", vol_table[i].value);
			*vpos_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct isl98608_voltage)) {
		pr_err("not found vsp voltage, use default voltage:ISL98608_VOL_55\n");
		*vpos_target = ISL98608_VOL_55;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct isl98608_voltage); i++) {
		if (vol_table[i].voltage == *vneg_target) {
			pr_err("isl98608 vsn voltage:0x%x\n", vol_table[i].value);
			*vneg_target = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct isl98608_voltage)) {
		pr_err("not found vsn voltage, use default voltage:ISL98608_VOL_55\n");
		*vpos_target = ISL98608_VOL_55;
	}
	return 0;
}
#endif


static bool isl98608_device_verify(struct i2c_client *client)
{
	int ret;
	ret = i2c_smbus_read_byte_data(client, ISL98608_REG_ENABLE);
	if (ret < 0 && ret != ISL98608_REG_ENABLE_VAL) {
		pr_err("%s read reg_enable failed\n", __func__);
		return false;
	}
	pr_info("ISL98608 verify ok, reg_enable = 0x%x\n", ret);
	return true;
}

static int isl98608_start_setting(struct i2c_client *client, int vsp, int vsn)
{
	int ret;

	/* Request and Enable VSP gpio */
	ret = devm_gpio_request_one(&client->dev, vsp, GPIOF_OUT_INIT_HIGH, "rt4801-vsp");
	if (ret) {
		pr_err("devm_gpio_request_one gpio_vsp_enable %d faild\n", vsp);
		return ret;
	}
	/* delay 2ms */
	mdelay(2);

	/* Request And Enable VSN gpio */
	ret = devm_gpio_request_one(&client->dev, vsn, GPIOF_OUT_INIT_HIGH, "rt4801-vsn");
	if (ret) {
		pr_err("devm_gpio_request_one gpio_vsn_enable %d faild\n", vsn);
		return ret;
	}

	return ret;
}

#ifdef CONFIG_LCD_KIT_DRIVER
static void isl98608_get_bias_config(int vpos, int vneg, int *outvsp, int *outvsn)
{
	unsigned int i;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct isl98608_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			*outvsp = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct isl98608_voltage)) {
		pr_err("not found vpos voltage, use default voltage:ISL98608_VOL_55\n");
		*outvsp = ISL98608_VOL_55;
	}

	for (i = 0; i < sizeof(vol_table) / sizeof(struct isl98608_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			*outvsn = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct isl98608_voltage)) {
		pr_err("not found vneg voltage, use default voltage:ISL98608_VOL_55\n");
		*outvsn = ISL98608_VOL_55;
	}
	pr_info("isl98608_get_bias_config: %d(vpos)= 0x%x, %d(vneg) = 0x%x\n",
		vpos, *outvsp, vneg, *outvsn);
}

static int isl98608_set_bias_power_down(int vpos, int vneg)
{
	int vsp = 0;
	int vsn = 0;
	int ret;

	isl98608_get_bias_config(vpos, vneg, &vsp, &vsn);
	ret = i2c_smbus_write_byte_data(isl98608_client->client, ISL98608_REG_VPOS, vsp);
	if (ret < 0) {
		pr_err("%s write vpos failed\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_byte_data(isl98608_client->client, ISL98608_REG_VNEG, vsn);
	if (ret < 0) {
		pr_err("%s write vneg failed\n", __func__);
		return ret;
	}
	return ret;
}

static int isl98608_set_bias(int vpos, int vneg)
{
	int i = 0;
	int vbst = 0;

	for (i = 0; i < sizeof(vol_table) / sizeof(struct isl98608_voltage); i++) {
		if (vol_table[i].voltage == vpos) {
			pr_err("isl98608 vsp voltage:0x%x\n", vol_table[i].value);
			vpos = vol_table[i].value;
			break;
		}
	}
	if (i >= sizeof(vol_table) / sizeof(struct isl98608_voltage)) {
		pr_err("not found vsp voltage, use default voltage:ISL98608_VOL_59\n");
		vpos = ISL98608_VOL_59;
	}
	for (i = 0; i < sizeof(vol_table) / sizeof(struct isl98608_voltage); i++) {
		if (vol_table[i].voltage == vneg) {
			pr_err("isl98608 vsn voltage:0x%x\n", vol_table[i].value);
			vneg = vol_table[i].value;
			break;
		}
	}

	if (i >= sizeof(vol_table) / sizeof(struct isl98608_voltage)) {
		pr_err("not found vsn voltage, use default voltage:ISL98608_VOL_59\n");
		vneg = ISL98608_VOL_59;
	}

	if (vpos == ISL98608_VOL_59)
		vbst = ISL98608_VBST_VOL_64;
	else
		vbst = vpos;

	isl98608_reg_init(isl98608_client->client, (u8)vbst, (u8)vpos, (u8)vneg);
	return 0;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = isl98608_set_bias,
	.set_bias_power_down = isl98608_set_bias_power_down,
};
#endif

static int isl98608_get_config(struct i2c_client *client, int* vsp_pin, int* vsn_pin)
{
	int retval = 0;
	struct device_node *np = NULL;

        if (client == NULL) {
                pr_err("[%s,%d]: NULL point for client\n", __FUNCTION__, __LINE__);
                return -ENODEV;
        }

        np = client->dev.of_node;
        *vsp_pin = of_get_named_gpio_flags(np, "gpio_vsp_enable", 0, NULL);
        if (!gpio_is_valid(*vsp_pin)) {
                pr_err("get vsp_enable gpio faild\n");
                return -ENODEV;
        }
        *vsn_pin = of_get_named_gpio_flags(np, "gpio_vsn_enable", 0, NULL);
        if (!gpio_is_valid(*vsn_pin)) {
                pr_err("get vsn_enable gpio faild\n");
                return -ENODEV;
        }

        retval = isl98608_start_setting(client, *vsp_pin, *vsn_pin);
        if (retval) {
                pr_err("isl98608_start_setting faild\n");
                return -ENODEV;
        }

        if (isl98608_device_verify(client)) {
                is_isl98608_device = true;
        } else {
                pr_info("isl98608_reg_verify failed\n");
                is_isl98608_device = false;
                return -ENODEV;
        }

        if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
                pr_err("[%s, %d]: need I2C_FUNC_I2C\n", __FUNCTION__, __LINE__);
                return -ENODEV;
        }
        return retval;
}

static int isl98608_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int retval = 0;
	int vsp_pin = 0;
	int vsn_pin = 0;

	retval = isl98608_get_config(client, &vsp_pin, &vsn_pin);
	if (retval) {
		goto failed_1;
	}
	isl98608_client = kzalloc(sizeof(*isl98608_client), GFP_KERNEL);
	if (!isl98608_client) {
		dev_err(&client->dev, "failed to allocate device info data\n");
		retval = -ENOMEM;
		goto failed_1;
	}

	i2c_set_clientdata(client, isl98608_client);
	isl98608_client->dev = &client->dev;
	isl98608_client->client = client;
	isl98608_client->config.vsp_enable = vsp_pin;
	isl98608_client->config.vsn_enable = vsn_pin;
	isl98608_client->config.vbst_cmd = ISL98608_VBST_VOL_64;
	isl98608_client->config.vpos_cmd = ISL98608_VOL_59;
	isl98608_client->config.vneg_cmd = ISL98608_VOL_59;
	isl98608_client->config.enable_cmd = ISL98608_REG_ENABLE_VAL;

	isl98608_enable();
	retval = isl98608_reg_init(isl98608_client->client, (u8)isl98608_client->config.vbst_cmd,
				(u8)isl98608_client->config.vpos_cmd, (u8)isl98608_client->config.vneg_cmd);
	if (retval) {
		retval = -ENODEV;
		pr_err("isl98608_reg_init failed\n");
		goto failed;
	}
	pr_info("isl98608 inited succeed\n");

#ifdef CONFIG_LCD_KIT_DRIVER
	lcd_kit_bias_register(&bias_ops);
#endif
failed_1:
	return retval;
failed:
	if (isl98608_client) {
		kfree(isl98608_client);
		isl98608_client = NULL;
	}
	return retval;
}

static const struct of_device_id isl98608_match_table[] = {
	{
		.compatible = DTS_COMP_ISL98608,
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id isl98608_i2c_id[] = {
	{ "isl98608", 0 },
	{ }
};

MODULE_DEVICE_TABLE(of, isl98608_match_table);

static struct i2c_driver isl98608_driver = {
	.id_table = isl98608_i2c_id,
	.probe = isl98608_probe,
	.driver = {
		.name = "isl98608",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(isl98608_match_table),
	},
};

int __init isl98608_module_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&isl98608_driver);
	if (ret)
		pr_err("Unable to register isl98608 driver\n");

	return ret;
}

void __exit isl98608_exit(void)
{
	i2c_del_driver(&isl98608_driver);
}

MODULE_DESCRIPTION("ISL98608 driver");
MODULE_LICENSE("GPL");
