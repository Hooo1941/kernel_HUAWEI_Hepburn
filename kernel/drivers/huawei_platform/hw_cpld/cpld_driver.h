/*
 * cpld Driver
 *
 * Copyright (c) 2012-2022 Huawei Technologies Co., Ltd.
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
#ifndef _CPLD_DRIVER_H_
#define _CPLD_DRIVER_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/of_gpio.h>
#include <linux/pm_wakeup.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include <linux/firmware.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/file.h>
#include <linux/mm.h>
#include "huawei_platform/log/hw_log.h"

#define CHIP_NAME_LEN 31
#define FW_NAME_LEN 31

#define MAIN_CPLD_NUM 1
#define SLAVE_CPLD_NUM 2

#define NOT_SUPPORT_CPLD 0

enum cpld_bus_type {
	CPLD_BUS_I2C = 0,
	CPLD_BUS_SPI,
	CPLD_BUS_UNDEF = 255,
};

struct cpld_bus_info {
	enum cpld_bus_type btype;
	unsigned int bus_id;
	int (*bus_write)(u8 *buf, u16 length);
	int (*bus_read)(u8 *reg_addr, u16 reg_len, u8 *buf, u16 len);
};

struct cpld_device_data {
	u32 fw_version_reg_addr;
	u32 cpld_state_reg_addr;
};

struct cpld_core_data {
	u32 fw_update_type;
	u32 chip_type;
	u32 i2c_address;
	int mode1_gpio;
	int mode2_gpio;
	struct platform_device *cpld_dev;
	struct i2c_client *client;
	struct cpld_bus_info bus_info;
	u16 main_usercode;
	u16 slave_usercode;
	struct cpld_device_data device_data;
	struct timer_list watchdog_timer;
	struct work_struct watchdog_work;
	struct wakeup_source *cpld_wake_lock;
	unsigned char cpld_reg_data_arr[19];
};

void cpld_show_all_reg_data(void);
#endif

