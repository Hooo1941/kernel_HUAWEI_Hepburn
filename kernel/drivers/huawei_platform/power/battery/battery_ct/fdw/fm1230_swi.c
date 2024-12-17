// SPDX-License-Identifier: GPL-2.0+
/*
 * fm1230_swi.c
 *
 * fm1230_swi driver
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "fm1230_swi.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/time.h>
#include <huawei_platform/log/hw_log.h>
#include "fm1230_api.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG fm1230_rdwr_ops
HWLOG_REGIST();

#ifndef us2cycles
#define us2cycles(x)  (((((x) * 0x10c7UL) * loops_per_jiffy * HZ) + \
	(1UL << 31)) >> 32)
#endif

void fm1230_delay(unsigned long cnt_delay)
{
	cycles_t start = get_cycles();
	cycles_t cnt_gap = 0;

	while (cnt_gap < cnt_delay)
		cnt_gap = (get_cycles() - start);
}

void fm1230_gpio_dir_input(int onewire_gpio)
{
	gpio_direction_input(onewire_gpio);
}

void fm1230_gpio_dir_output(int onewire_gpio, uint8_t value)
{
	gpio_direction_output(onewire_gpio, value);
}

void fm1230_gpio_set_value(int onewire_gpio, uint8_t value)
{
	gpio_set_value(onewire_gpio, value);;
}

int fm1230_gpio_get_value(int onewire_gpio)
{
	return gpio_get_value(onewire_gpio);
}

int fm1230_ow_reset(struct fm1230_dev *di)
{
	uint8_t bdat = HIGH;
	uint16_t i;
	int gpio_id;

	if (!di)
		return -ENODEV;

	gpio_id = di->onewire_gpio;
	fm1230_gpio_dir_output(gpio_id, HIGH);
	fm1230_gpio_set_value(gpio_id, LOW);
	udelay(di->fm1230_swi.ow_reset_start_delay);
	fm1230_gpio_set_value(gpio_id, HIGH);
	fm1230_gpio_dir_input(gpio_id);
	udelay(di->fm1230_swi.ow_reset_sample_delay);
	for (i = 0; (i < OW_RESET_READ_CNT) && bdat; i++)
		bdat = fm1230_gpio_get_value(gpio_id);
	udelay(di->fm1230_swi.ow_reset_end_delay);
	fm1230_gpio_dir_output(gpio_id, HIGH);
	udelay(OW_RESET_DELAY);

	return bdat;
}

uint8_t ow_dev_reset(struct fm1230_dev *di)
{
	uint8_t bdat = HIGH;
	uint16_t i;
	int gpio_id;

	if (!di)
		return -ENODEV;

	gpio_id = di->onewire_gpio;

	fm1230_gpio_dir_output(gpio_id, HIGH);
	fm1230_gpio_set_value(gpio_id, LOW);
	udelay(di->fm1230_swi.ow_reset_start_delay);
	fm1230_gpio_set_value(gpio_id, HIGH);
	fm1230_gpio_dir_input(gpio_id);
	udelay(di->fm1230_swi.ow_reset_sample_delay);
	for (i = 0; i < OW_RESET_READ_CNT && bdat; i++)
		bdat = fm1230_gpio_get_value(gpio_id);
	udelay(di->fm1230_swi.ow_reset_end_delay);
	fm1230_gpio_dir_output(gpio_id, HIGH);
	udelay(OW_DEV_RESET_DELAY);

	return bdat;
}

static void write_bit_8m(struct fm1230_dev *di, uint8_t bit_data)
{
	unsigned long flags;
	int gpio_id;

	if (!di)
		return;

	gpio_id = di->onewire_gpio;

	fm1230_gpio_dir_output(gpio_id, LOW);
	local_irq_save(flags);
	fm1230_delay(us2cycles(di->fm1230_swi.ow_write_start_delay));
	if (bit_data) {
		local_irq_restore(flags);
		fm1230_gpio_set_value(gpio_id, HIGH);
		local_irq_save(flags);
		fm1230_delay(us2cycles(di->fm1230_swi.ow_write_high_delay));
	} else {
		fm1230_delay(us2cycles(di->fm1230_swi.ow_write_low_delay));
	}
	local_irq_restore(flags);
	fm1230_gpio_set_value(gpio_id, HIGH);
	fm1230_delay(us2cycles(di->fm1230_swi.ow_write_end_delay));
}

static uint8_t read_bit_8m(struct fm1230_dev *di)
{
	uint8_t bit_data = 1;
	unsigned long flags;
	int gpio_id;

	if (!di)
		return -1;

	gpio_id = di->onewire_gpio;

	fm1230_gpio_dir_output(gpio_id, LOW);
	local_irq_save(flags);
	fm1230_delay(us2cycles(di->fm1230_swi.ow_read_start_delay));
	local_irq_restore(flags);
	fm1230_gpio_dir_input(gpio_id);
	local_irq_save(flags);
	fm1230_delay(us2cycles(di->fm1230_swi.ow_read_sample_delay));
	local_irq_restore(flags);
	bit_data = fm1230_gpio_get_value(gpio_id);
	fm1230_delay(us2cycles(di->fm1230_swi.ow_read_end_delay));
	local_irq_restore(flags);
	fm1230_gpio_dir_output(gpio_id, HIGH_VOLTAGE);
	return bit_data;
}

static void write_byte_8m(struct fm1230_dev *di, uint8_t val)
{
	uint8_t i;

	/* writes byte, one bit at a time */
	for (i = 0; i < 8; i++)
		write_bit_8m(di, (val >> i) & 0x01); /* write bit in temp into */
	udelay(OW_WRITE_BYTE_INV);
}

static uint8_t read_byte_8m(struct fm1230_dev *di)
{
	uint8_t i;
	uint8_t value = 0;

	for (i = 0; i < 8; i++)
		if (read_bit_8m(di))
			value |= 0x01 << i; /* reads byte in, one byte at a time and then shifts it left */
	udelay(OW_READ_BYTE_INV);
	return value;
}

static int fm1230_swi_ops_register(struct fm1230_swi_rdwr_ops *rwo)
{
	rwo->wbyte = write_byte_8m;
	rwo->rbyte = read_byte_8m;
	rwo->reset = fm1230_ow_reset;
	rwo->dev_reset = ow_dev_reset;
	fm1230_ops_register(rwo);
	hwlog_info("[%s] ops registed\n", __func__);
	return 0;
}

static int fm1230_swi_probe(struct platform_device *pdev)
{
	int ret;
	struct fm1230_swi_rdwr_ops *pow_se_ops = NULL;

	hwlog_info("[%s] probe begin\n", __func__);

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	pow_se_ops = kzalloc(sizeof(struct fm1230_swi_rdwr_ops), GFP_KERNEL);
	if (!pow_se_ops)
		return -ENOMEM;

	ret = fm1230_swi_ops_register(pow_se_ops);
	if (ret) {
		hwlog_err("[%s] fail:%u\n", __func__);
		goto se_ops_fail;
	}

	return 0;
se_ops_fail:
	kfree(pow_se_ops);
	return ret;
}

static int fm1230_swi_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id ow_swi_dt_match[] = {
	{
		.compatible = "fdw,fm1230_swi",
		.data = NULL,
	},
	{},
};

static struct platform_driver fm1230_swi_driver = {
	.probe			= fm1230_swi_probe,
	.remove			= fm1230_swi_remove,
	.driver			= {
		.owner		= THIS_MODULE,
		.name		= "fm1230_swi",
		.of_match_table = ow_swi_dt_match,
	},
};

static int __init fm1230_swi_init(void)
{
	return platform_driver_register(&fm1230_swi_driver);
}

static void __exit fm1230_swi_exit(void)
{
	platform_driver_unregister(&fm1230_swi_driver);
}

subsys_initcall_sync(fm1230_swi_init);
module_exit(fm1230_swi_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("fm1230 swi");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
