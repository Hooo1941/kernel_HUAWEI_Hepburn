/*
 * echub_i2c.c
 *
 * I2C driver for echub
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
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mfd/core.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <securec.h>
#include "echub.h"
#include "echub_i2c.h"

static int major;
static struct class *class;
struct echub_i2c_dev *g_echub_dev = NULL;

static struct mfd_cell echubs[] = {
	{
		.name = "echub-power",
		.of_compatible = "huawei,echub-power",
	},
	{
		.name = "echub_battery",
		.of_compatible = "huawei,echub_battery",
	},
	{
		.name = "echub-muteled",
		.of_compatible = "huawei,echub-muteled",
	},
	{
		.name = "echub-lid",
		.of_compatible = "huawei,echub-lid",
	},
	{
		.name = "echub-fnled",
		.of_compatible = "huawei,echub-fnled",
	},
	{
		.name = "echub_thermal",
		.of_compatible = "huawei,echub_thermal",
	},
	{
		.name = "echub-log",
		.of_compatible = "huawei,echub-log",
	},
	{
		.name = "echub-update",
		.of_compatible = "huawei,echub-update",
	},
	{
		.name = "echub_charge_status",
		.of_compatible = "huawei,echub_charge_status",
	},
	{
		.name = "echub_wakealarm",
		.of_compatible = "huawei,echub_wakealarm",
	},
	{
		.name = "echub-rtc",
		.of_compatible = "huawei,echub-rtc",
	},
};

struct echub_i2c_dev *get_echub_dev(void)
{
	return g_echub_dev;
}

static unsigned char get_transfer_date(char devaddr, int regoffset,
					char regdata, char *value, int index)
{
	unsigned char data;

	switch (index) {
	case 0:
		data = (unsigned char)(devaddr<<1);
		break;
	case 1:
		data = (unsigned char)(regoffset >> BIT_OFFSET);
		break;
	case 2:
		data = (unsigned char)(regoffset & 0xFF);
		break;
	case 3:
		data = REG_ECHUB_LEN;
		break;
	case 4:
		data = regdata;
		break;
	case 5:
		data = (unsigned char)(devaddr<<1) + 1;
		break;
	case 6:
		data = value[0];
		break;
	case 7:
		data = value[1];
		break;
	case 8:
		data = value[2];
		break;
	default:
		echub_err("echub %s error\n", __func__);
		break;
	}

	return data;
}

/**
 * calc_bus_pec - Calculate the PEC, to verify the value returned from EC.
 * @devaddr: i2c device address
 * @regoffset: data register address offset
 * @regdata: the data flag to read
 * @value: buffer of read value
 * @type: read or write
 *
 * return value: PEC value
 */
static unsigned char calc_bus_pec(char devaddr, int regoffset,
					char regdata, char *value, int type)
{
	static const char pec_table[] = {
		0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
		0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D
	};
	unsigned char i2c_cmd_pec = 0;
	unsigned char sourcebyte = 0;
	int crc_len = 0;
	int i = 0;

	if (type == ECHUB_I2C_WRITE)
		crc_len = ECHUB_WRITE_LEN;
	else if (type == ECHUB_I2C_READ)
		crc_len = ECHUB_READ_LEN;
	else
		echub_err("%s input value err\n", __func__);

	for (i = 0; i < crc_len; i++) {
		unsigned char data = 0;
		unsigned char crc = 0;

		sourcebyte = get_transfer_date(devaddr, regoffset, regdata, value, i);
		data = i2c_cmd_pec ^ sourcebyte;
		crc = (crc  << CRC_SHIFT) ^ pec_table[((crc ^ data) >> CRC_SHIFT) & CRC_MASK];
		data <<= CRC_SHIFT;
		i2c_cmd_pec = (crc  << CRC_SHIFT) ^ pec_table[((crc ^ data) >> CRC_SHIFT) & CRC_MASK];
	}

	return i2c_cmd_pec;
}


/**
 * echub_i2c_read - read data from EC
 * @echub_dev: i2c device
 * @regoffset: data register address offset
 * @regdata: the data flag to read
 * @value: buffer of read value
 * @bytes: the byte length of @value
 *
 * return value: 0-sucess or others-fail
 */
int echub_i2c_read(struct echub_i2c_dev *echub_dev, int regoffset,
			char regdata, char *value, int bytes)
{
	int ret;
	struct i2c_client *client = echub_dev->client;
	struct i2c_msg msg[2];
	unsigned char data[4] = {0};

	/* write data buffer */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_SEND_BYTE_LEN;
	msg[0].buf = data;
	data[0] = (u8)(regoffset >> BIT_OFFSET);
	data[1] = (u8)(regoffset & 0xFF);
	data[2] = REG_DATA_LEN;
	data[3] = regdata;
	/* read data buffer */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = bytes;
	msg[1].buf = value;

	ret = i2c_transfer(client->adapter, msg, ECHUB_SEND_TRANS_LEN);
	/* check i2c transfer result */
	if (ret != ECHUB_SEND_TRANS_LEN)
		return -ENODEV;
	if (value[0]) {
		echub_err("%s failed, value[0] = %d\n", __func__, value[0]);
		return -EIO;
	}
	if (value[1] != 1 && value[1] != 2) {
		echub_err("%s failed, regoffset = 0x%04x, value[1] = %d\n", __func__, regoffset, value[1]);
		return -EINVAL;
	}
	if (value[1] == 1 && value[3] != calc_bus_pec(client->addr, regoffset, regdata, value, ECHUB_I2C_READ))
		echub_err("%s, calc_bus_pec failed, value = %d\n", __func__, value[3]);

	return 0;
}

/**
 * echub_i2c_write - write data to EC
 * @echub_dev: i2c device
 * @regoffset: data register address offset
 * @regdata: the data to write
 *
 * return value: 0-sucess or others-fail
 */
int echub_i2c_write(struct echub_i2c_dev *echub_dev, int regoffset, char regdata)
{
	int ret;
	struct i2c_client *client = echub_dev->client;
	struct i2c_msg msg[2];
	unsigned char data[4];
	unsigned char value[2];

	/* write data buffer */
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_SEND_BYTE_LEN;
	msg[0].buf = data;
	data[0] = (u8)(regoffset >> BIT_OFFSET);
	data[1] = (u8)(regoffset & 0xFF);
	data[2] = REG_DATA_LEN;
	data[3] = regdata;

	/* read data buffer */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = ECHUB_SEND_TRANS_LEN;
	msg[1].buf = value;

	ret = i2c_transfer(client->adapter, msg, ECHUB_SEND_TRANS_LEN);
	/* check i2c transfer result */
	if (ret != ECHUB_SEND_TRANS_LEN)
		return -ENODEV;
	if (value[0])
		return -EIO;
	if (value[1] != calc_bus_pec(client->addr, regoffset, regdata, value, ECHUB_I2C_WRITE))
		echub_err("%s, calc_bus_pec failed, value = %d\n", __func__, value[1]);

	return 0;
}

int echub_i2c_transfer(struct echub_i2c_dev *echub_dev, int regoffset, unsigned char *wb, int wblen,
			unsigned char *rb, int rblen)
{
	int ret = 0;
	struct i2c_msg msg[2];
	struct i2c_client *client = echub_dev->client;

	if ((wb == NULL && rb == NULL) || wblen > ECHUB_I2C_MSG_MAX_LEN || rblen > ECHUB_I2C_MSG_MAX_LEN)
		return -EINVAL;

	msg[0].buf = kmalloc(ECHUB_I2C_WRITE_DATA_HEAD + wblen, GFP_KERNEL);
	if (!(msg[0].buf))
		return -ENOMEM;
	(msg[0].buf)[0] = (u8)(regoffset >> BIT_OFFSET);
	(msg[0].buf)[1] = (u8)(regoffset & 0xFF);
	(msg[0].buf)[2] = wblen;
	if (wb != NULL && wblen != 0) {
		ret = memcpy_s(&(msg[0].buf)[ECHUB_I2C_WRITE_DATA_HEAD], wblen, wb, wblen);
		if (ret != EOK)
			goto ferr_w_mem;
	}
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_I2C_WRITE_DATA_HEAD + wblen;

	/* read data buffer */
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = (rb != NULL && rblen != 0) ? ECHUB_SEND_TRANS_LEN + 0x1 + rblen : ECHUB_SEND_TRANS_LEN;
	msg[1].buf = kmalloc(msg[1].len, GFP_KERNEL);
	if (!(msg[1].buf)) {
		ret = -ENOMEM;
		goto ferr_w_mem;
	}

	ret = i2c_transfer(client->adapter, msg, ECHUB_SEND_TRANS_LEN);
	if (ret != ECHUB_SEND_TRANS_LEN || (msg[1].buf)[0] != 0) {
		echub_err("%s i2c transfer error, ret = 0x%x, msg[1].buf[0] = 0x%x\n", __func__, ret, (msg[1].buf)[0]);
		ret = -EIO;
		goto free_r_mem;
	}

	ret = 0;
	if (rb != NULL && rblen != 0) {
		ret = memcpy_s(rb, rblen, &(msg[1].buf)[2], rblen);
		if (ret != EOK)
			goto free_r_mem;
	}

free_r_mem:
	kfree(msg[1].buf);
ferr_w_mem:
	kfree(msg[0].buf);

	return ret;
}

static struct file_operations echub_i2c_fops = {
	.owner = THIS_MODULE,
	/* add fops used in user space */
};

static int echub_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	struct echub_i2c_dev *echub_dev = NULL;

	echub_info("%s enter\n", __func__);

	echub_dev = devm_kzalloc(&client->dev, sizeof(struct echub_i2c_dev), GFP_KERNEL);
	if (echub_dev == NULL) {
		echub_err("%s kzalloc fail\n", __func__);
		return -ENOMEM;
	}

	echub_dev->dev = &client->dev;

	/* init struct echub_i2c_dev echub_i2c_data */
	mutex_init(&echub_dev->lock);
	echub_dev->client = client;
	echub_dev->write_func = echub_i2c_write;
	echub_dev->read_func = echub_i2c_read;
	echub_dev->transfer_func = echub_i2c_transfer;

	i2c_set_clientdata(client, echub_dev);
	/* set i2c data  tmp */
	g_echub_dev = echub_dev;

	err = devm_mfd_add_devices(echub_dev->dev, -1, echubs,
				  ARRAY_SIZE(echubs), NULL, 0, NULL);
	if (err)
		echub_err("add mfd devices failed with err: %d\n", err);

	/*
	 * Create a char device named echub_i2c with echub_i2c_fops
	 * /dev/echub_i2c
	 */
	major = register_chrdev(0, "echub_i2c", &echub_i2c_fops);
	class = class_create(THIS_MODULE, "echub_i2c");
	device_create(class, NULL, MKDEV(major, 0), NULL, "echub_i2c");
	echub_info("%s ok\n", __func__);

	return 0;
}


static int echub_i2c_remove(struct i2c_client *client)
{
	echub_info("%s\n", __func__);
	device_destroy(class, MKDEV(major, 0));
	class_destroy(class);
	unregister_chrdev(major, "echub_i2c");

	return 0;
}

static const struct of_device_id echub_i2c_of_match[] = {
	{ .compatible = "huawei,echub_i2c" },
	{},
};

MODULE_DEVICE_TABLE(of, echub_i2c_of_match);

static const struct i2c_device_id echub_i2c_id_table[] = {
	{ "echub_i2c", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, echub_i2c_id_table);

static struct i2c_driver echub_i2c_driver = {
	.driver = {
		.name           = "echub_i2c",
		.of_match_table = of_match_ptr(echub_i2c_of_match),
	},

	.probe       = echub_i2c_probe,
	.remove      = echub_i2c_remove,
	.id_table    = echub_i2c_id_table,
};

static int __init echub_i2c_init(void)
{
	echub_info("%s\n", __func__);
	return i2c_add_driver(&echub_i2c_driver);
}
module_init(echub_i2c_init);

static void __exit echub_i2c_exit(void)
{
	echub_info("%s\n", __func__);
	i2c_del_driver(&echub_i2c_driver);
}
module_exit(echub_i2c_exit);

MODULE_ALIAS("armpc echub i2c module");
MODULE_DESCRIPTION("armpc echub i2c driver");
MODULE_LICENSE("GPL");
