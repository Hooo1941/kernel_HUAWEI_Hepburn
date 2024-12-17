// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_dc_control.c
 *
 * kick the watchdog for wireless direct charge
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

#include <chipset_common/hwpower/wireless_charge/wireless_dc_control.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>

#define HWLOG_TAG wireless_dc_control
HWLOG_REGIST();

bool wldc_need_stop_work(struct wldc_dev_info *di)
{
	if (!di) {
		hwlog_err("need_stop_work: di null\n");
		return true;
	}

	if (di->stop_flag_error || di->stop_flag_info ||
		di->stop_flag_warning || !di->sysfs_data.enable_charger ||
		di->force_disable || (wldc_get_charge_stage() < WLDC_STAGE_CHARGING)) {
		hwlog_err("need_stop_work: direct_charge stop charging, force_disable=%u\n",
			di->force_disable);
		return true;
	}
	return false;
}

void wldc_kick_wtd_work(struct work_struct *work)
{
	struct wldc_dev_info *di = container_of(work,
		struct wldc_dev_info, wldc_kick_wtd_work.work);

	if (!di) {
		hwlog_err("kick_wtd_work: di null\n");
		return;
	}

	if (wldc_need_stop_work(di)) {
		hwlog_err("kick_wtd_work: stop kick wtd work\n");
		return;
	}

	dcm_kick_ic_watchdog(di->dc_ratio, di->ic_data.cur_type);
	mod_delayed_work(system_wq, &di->wldc_kick_wtd_work,
		msecs_to_jiffies(WLDC_WDT_TIME));
}
