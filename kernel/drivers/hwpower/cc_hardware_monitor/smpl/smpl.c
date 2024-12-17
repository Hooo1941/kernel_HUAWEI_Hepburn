// SPDX-License-Identifier: GPL-2.0
/*
 * smpl.c
 *
 * smpl(sudden momentary power loss) error monitor driver
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

#include "smpl.h"
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG smpl
HWLOG_REGIST();

static struct smpl_dev *g_smpl_dev;

static bool smpl_check_reboot_reason(void)
{
	if (strstr(saved_command_line, "normal_reset_type=SMPL")) {
		hwlog_info("smpl happened: normal_reset_type=smpl\n");
		return true;
	} else if (strstr(saved_command_line, "reboot_reason=power_loss")) {
		hwlog_info("smpl happened: reboot_reason=power_loss\n");
		return true;
	} else if (strstr(saved_command_line, "reboot_reason=2sec_reboot")) {
		hwlog_info("smpl happened: reboot_reason=2sec_reboot\n");
		return true;
	} else if (strstr(saved_command_line, "normal_reset_type=BR_POWERON_BY_SMPL")) {
		hwlog_info("smpl happened: normal_reset_type=BR_POWERON_BY_SMPL\n");
		return true;
	}

	return false;
}

static void smpl_error_monitor_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0128] = { 0 };
	const char *batt_brand = POWER_SUPPLY_DEFAULT_BRAND;
	int batt_volt = 0;
	int batt_temp = 0;
	int bat_cap = 0;

	hwlog_info("monitor_work begin\n");

	if (!smpl_check_reboot_reason())
		return;

	(void)power_supply_get_str_property_value(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_BRAND, &batt_brand);
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &batt_volt,
		POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_TEMP, &batt_temp,
		POWER_SUPPLY_DEFAULT_TEMP / POWER_SUPPLY_BAT_TEMP_UNIT,
		POWER_SUPPLY_BAT_TEMP_UNIT);
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CAPACITY, &bat_cap,
		POWER_SUPPLY_DEFAULT_CAPACITY, 1);
	snprintf(buf, POWER_DSM_BUF_SIZE_0128 - 1,
		"smpl happened : brand=%s t_bat=%d, volt=%d, soc=%d\n",
		batt_brand, batt_temp, batt_volt, bat_cap);
	power_dsm_report_dmd(POWER_DSM_SMPL, POWER_DSM_ERROR_NO_SMPL, buf);
	hwlog_info("smpl happened: %s\n", buf);
}

static int __init smpl_init(void)
{
	struct smpl_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_smpl_dev = l_dev;

	INIT_DELAYED_WORK(&l_dev->monitor_work, smpl_error_monitor_work);
	schedule_delayed_work(&l_dev->monitor_work,
		msecs_to_jiffies(DELAY_TIME_FOR_MONITOR_WORK));
	return 0;
}

static void __exit smpl_exit(void)
{
	if (!g_smpl_dev)
		return;

	cancel_delayed_work(&g_smpl_dev->monitor_work);
	kfree(g_smpl_dev);
	g_smpl_dev = NULL;
}

module_init(smpl_init);
module_exit(smpl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("smpl error monitor module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
