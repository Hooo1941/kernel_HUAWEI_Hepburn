/*
 * echub_log.c
 *
 * echub log interface
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

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "../echub.h"
#include "../echub_i2c.h"

#define ECLOG_DUMP_SIZE            256
#define ECLOG_DUMP_CMD             0xdb01
#define ECLOG_DUMP_START_ADDR      0x2000
#define ECLOG_DUMP_TIMES           64
#define ECLOG_OUTPUT_SIZE          (128 * 1024)

#define upper_byte(n) ((u8)(((u16)(n)) >> 8))
#define lower_byte(n) ((u8)(u16)(n))

static int query_echub_log(struct i2c_client *client, u8 *buf, int size, int index)
{
	u8 write_buf[4] = {0}; /* echub log query cmd, 4 byte */
	u16 query_addr = ECLOG_DUMP_START_ADDR + index * ECLOG_DUMP_SIZE;

	if (!buf || size != ECLOG_DUMP_SIZE) {
		echub_err("%s, query echub log failed\n", __func__);
		return -EFAULT;
	}

	write_buf[0] = upper_byte(ECLOG_DUMP_CMD);
	write_buf[1] = lower_byte(ECLOG_DUMP_CMD);
	write_buf[2] = upper_byte(query_addr);
	write_buf[3] = lower_byte(query_addr);

	struct i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.buf = write_buf,
			.len = sizeof(write_buf),
			.flags = 0,
		}, {
			.addr = client->addr,
			.buf = buf,
			.len = size,
			.flags = I2C_M_RD,
		}
	};

	if (i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs)) != ARRAY_SIZE(msgs)) {
		echub_err("%s, transfer data failed\n", __func__);
		return -EIO;
	}
	return 0;
}

static int echub_log_show(struct seq_file *m, void *v)
{
	struct echub_i2c_dev *echub_dev = (struct echub_i2c_dev *)m->private;
	u8 log_buf[ECLOG_DUMP_SIZE] = {0};
	int ret = 0;
	char *buffer = NULL;
	size_t size;

	if (!echub_dev) {
		echub_err("%s, invalid value\n", __func__);
		return -EINVAL;
	}

	size = seq_get_buf(m, &buffer);
	if (size < ECLOG_OUTPUT_SIZE) {
		seq_commit(m, -1);
		return 0;
	}

	for (int i = 0; i < ECLOG_DUMP_TIMES; i++) {
		ret = query_echub_log(echub_dev->client, log_buf, ECLOG_DUMP_SIZE, i);
		if (ret) {
			echub_err("%s, query_echub_log failed: %d\n", __func__, ret);
			break;
		}
		seq_hex_dump(m, "", DUMP_PREFIX_NONE, 32, 1, log_buf, ECLOG_DUMP_SIZE, 0);
	}
	return ret;
}

static int echub_log_probe(struct platform_device *pdev)
{
	struct echub_i2c_dev *echub_dev = dev_get_drvdata(pdev->dev.parent);
	if (echub_dev == NULL) {
		echub_err("echub_dev is NULL\n");
		return -ENODEV;
	}

	if (!proc_create_single_data("echub_log", 0440, NULL, echub_log_show, echub_dev)) {
		echub_err("create echub log proc node failed\n");
		return -ENOMEM;
	}
	return 0;
}

static const struct of_device_id of_echub_log_match_tbl[] = {
	{ .compatible = "huawei,echub-log" },
	{},
};

static struct platform_driver echub_log_driver = {
	.driver = {
		.name = "echub-log",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(of_echub_log_match_tbl),
	},
	.probe = echub_log_probe,
};


static int __init echub_log_init(void)
{
	return platform_driver_register(&echub_log_driver);
}

static void __exit echub_log_exit(void)
{
	platform_driver_unregister(&echub_log_driver);
}

module_init(echub_log_init);
module_exit(echub_log_exit);

MODULE_DESCRIPTION("echub log driver");
MODULE_LICENSE("GPL");
