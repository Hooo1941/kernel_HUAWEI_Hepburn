// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_uevent.c
 *
 * uevent handle for direct charge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_pmode.h>

#define HWLOG_TAG direct_charge_uevent
HWLOG_REGIST();

void dc_send_icon_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();
	int max_cur;

	if (!l_di)
		return;

	if (direct_charge_is_priority_inversion()) {
		hwlog_info("icon type already send\n");
		return;
	}

	max_cur = dc_pmode_get_optimal_mode_max_cur(QUICK_CHECK);
	l_di->max_pwr = dc_pmode_get_optimal_mode_max_pwr(QUICK_CHECK);
	if (max_cur >= l_di->super_ico_current)
		wired_connect_send_icon_uevent(ICON_TYPE_SUPER);
	else
		wired_connect_send_icon_uevent(ICON_TYPE_QUICK);
}

void dc_send_max_power_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || l_di->ui_max_pwr <= 0)
		return;

	if (l_di->max_pwr >= l_di->ui_max_pwr)
		power_ui_event_notify(POWER_UI_NE_MAX_POWER, &l_di->max_pwr);
}

void dc_send_soc_decimal_uevent(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	power_event_bnc_notify(POWER_BNT_SOC_DECIMAL, POWER_NE_SOC_DECIMAL_DC, &l_di->max_pwr);
}
