/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_dc_control.h
 *
 * common interface, variables, definition etc of wireless_dc_control.c
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

#ifndef _WIRELESS_DC_CONTROL_H_
#define _WIRELESS_DC_CONTROL_H_

#include <linux/workqueue.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>

#define WLDC_WDT_TIME                     500 /* ms */

#ifdef CONFIG_WIRELESS_CHARGER
void wldc_kick_wtd_work(struct work_struct *work);
bool wldc_need_stop_work(struct wldc_dev_info *di);
#else
static inline void wldc_kick_wtd_work(struct work_struct *work)
{
}

static inline bool wldc_need_stop_work(struct wldc_dev_info *di)
{
	return true;
}

#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_DC_CONTROL_H_ */
