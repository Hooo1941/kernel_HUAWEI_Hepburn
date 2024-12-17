/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hl7603.h
 *
 * hl7603 header
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

#ifndef _HL7603_H_
#define _HL7603_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>

/* define ic state of bypass boost */
#define BYPASS_BST_IC_GOOD         0
#define BYPASS_BST_IC_BAD          1

#define HL7603_VOUT_SEL_DEFAULT    3450

/* register devid */
#define HL7603_DEVID_ADDR          0x00
#define HL7603_DEVID_MASK          0xf0
#define HL7603_DEVID_SHIFT         4
#define HL7603_DEVID               0xb

/* register vout_sel */
#define HL7603_VOUT_SEL_ADDR       0x02
#define HL7603_VOUT_SEL_MASK       0x3f
#define HL7603_VOUT_SEL_SHIFT      0
#define HL7603_VOUT_SEL_BASE       2850
#define HL7603_VOUT_SEL_STEP       50

/* register state */
#define HL7603_STATE_ADDR_HL       0x05
#define HL7603_STATE_ADDR_JWT      0x04

enum {
	HL7603_DEVID_HL = 0xb,
	HL7603_DEVID_JWT = 0x5,
};

struct hl7603_device_info {
	struct i2c_client *client;
	struct device *dev;
	u32 const_vout;
	u8 dev_id;
	int gpio_no;
	struct delayed_work monitor_work;
};

#endif /* _HL7603_H_ */
