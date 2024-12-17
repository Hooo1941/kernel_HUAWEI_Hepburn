/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sc8510.h
 *
 * sc8510 macro, addr etc.
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

#ifndef _SC8510_H_
#define _SC8510_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_device.h>

/* device id register */
#define SC8510_DEVICE_ID_REG                0x14
#define SC8510_DEVICE_ID                    0x10

/* fsw ctrl register */
#define SC8510_FSW_CTRL_REG                 0x07
#define SC8510_FIX_FREQ_EN_MASK             BIT(6)
#define SC8510_FIX_FREQ_EN_SHIFT            6

struct sc8510_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct notifier_block event_nb;
};

#endif /* _SC8510_H_ */
