/* SPDX-License-Identifier: GPL-2.0 */
/*
 * power_extra_event.h
 *
 * extra event for power module
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_EXTRA_EVENT_H_
#define _POWER_EXTRA_EVENT_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>

enum {
	PANEL_CLIENT_FB,
	PANEL_CLIENT_DRM,
	PANEL_CLIENT_DRM_8425,
};

struct power_extra_event_dev {
	struct device *dev;
	struct notifier_block nb;
	int panel_client;
	void *notifier_cookie;
};

#if IS_ENABLED(CONFIG_HUAWEI_POWER_EXTRA_EVENT)
void power_panel_event_register(struct power_extra_event_dev *di);
void power_panel_event_unregister(struct power_extra_event_dev *di);
int power_panel_event_parse_active_panel(void);
#else
static inline void power_panel_event_register(struct power_extra_event_dev *di)
{
}

static inline void power_panel_event_unregister(struct power_extra_event_dev *di)
{
}

static inline int power_panel_event_parse_active_panel(void)
{
	return -EINVAL;
}
#endif /* IS_ENABLED(CONFIG_HUAWEI_POWER_EXTRA_EVENT) */

#endif /* _POWER_EXTRA_EVENT_H_ */
