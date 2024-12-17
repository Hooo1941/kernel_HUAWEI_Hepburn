/* SPDX-License-Identifier: GPL-2.0 */
/*
 * power_sw.h
 *
 * interface for power switch
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

#ifndef _POWER_SW_H_
#define _POWER_SW_H_

#include <linux/types.h>

#define POWER_SW_ON     1
#define POWER_SW_OFF    0
#define POWER_SW_CHARGER_OVP "charger_ovp" /* cannot be modifed */

enum {
	POWER_SW_ATTR_LABEL = 0,
	POWER_SW_ATTR_INDEX,
	POWER_SW_ATTR_DFLT,
	POWER_SW_ATTR_EN,
	POWER_SW_ATTR_TOTAL,
};

struct power_sw_attr {
	void *dev;
	int num;
	int dflt;
	int en;
	const char *label;
	int status_set;
};

struct power_sw {
	bool ready;
	struct power_sw_attr attr;
	int (*set)(struct power_sw_attr *attr, int status);
	int (*get)(struct power_sw_attr *attr);
	void (*free)(struct power_sw_attr *attr);
};

int power_sw_register(int idx, struct power_sw *sw);
bool power_sw_ready(int idx);
int power_sw_set_output(int idx, int status);
int power_sw_get_output(int idx);
void power_sw_free(int idx);

int power_sw_set_output_by_label(const char *label, int status, bool force);
int power_sw_get_output_by_label(const char *label);
bool power_sw_exist_by_label(const char *label);
#endif /* _POWER_SW_H_ */
