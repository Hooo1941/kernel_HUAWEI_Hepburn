/* SPDX-License-Identifier: GPL-2.0 */
/*
 * charger_detection.h
 *
 * charger detection module
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

#ifndef _CHARGE_DETECTION_H_
#define _CHARGE_DETECTION_H_

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/power_supply.h>

struct charger_deteciton_ops {
	void (*handle_start_sink_event)(void);
	void (*handle_stop_sink_event)(void);
	void (*handle_start_wireless_event)(void);
	void (*handle_stop_wireless_event)(void);
	void (*handle_start_source_event)(void);
	void (*handle_stop_source_event)(void);
};

enum power_supply_sinksource_type charge_detecion_get_sinksource_type(void);
const char *charger_detection_get_sourcesink_type_name(void);
int charger_detecion_ops_register(struct charger_deteciton_ops *ops);
void charger_detecion_ops_unregister(void);

#endif /* _CHARGE_DETECTION_H_ */
