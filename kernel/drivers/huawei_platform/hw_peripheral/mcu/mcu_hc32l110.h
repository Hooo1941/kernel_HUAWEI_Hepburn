/*
 * mcu.h
 *
 * mcu driver header
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
#ifndef _MCU_HC32L110_H_
#define _MCU_HC32L110_H_


#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/rtc.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/semaphore.h>

struct mcu_hc32l110_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct notifier_block event_nb;
	struct notifier_block usb_event_nb;
	struct notifier_block std_event_nb;
	struct rtc_device *rtc_dev;
	int gpio_boot0;
	int gpio_reset;
	int chip_already_init;
	int fw_dev_id;
	int fw_ver_id;
	int fw_boot_ver_id;
	int fw_size;
	u8 *fw_data;
	bool is_low_power_mode;
	bool is_usb_connected;
	bool enter_std_proc;
};

struct mcu_upgrade {
	struct mcu_hc32l110_device_info *di;
	struct delayed_work mcu_upgrade_work;
};
#endif // MCU_HC32L110_H
