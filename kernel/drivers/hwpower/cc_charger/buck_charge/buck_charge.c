// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge.c
 *
 * buck charge driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/buck_charge/buck_charge.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/charger/charge_common_vote.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/coul/coul_interface.h>
#include <chipset_common/hwpower/hardware_monitor/ffc_control.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG buck_charge
HWLOG_REGIST();

#define BATTERY_DEFAULT_VTERM           4450
#define BATTERY_DEFAULT_ITERM           160
#define BUCK_CHARGE_FULL_CHECK_TIMIES   3
#define BATTERY_FULL_DELTA_VOTAGE       20
#define BATTERY_MAX_ITERM               750
#define CHARGING_CURRENT_OFFSET         (-10)

static struct buck_charge_dev *g_buck_charge_dev;

static bool buck_charge_need_check_charging_full(struct buck_charge_dev *di)
{
	int vbat = 0;
	int charge_enable;
	unsigned int vterm_dec = 0;

	charge_get_vterm_dec(&vterm_dec);
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &vbat,
		POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
	hwlog_info("%s vterm_dec=%u, vbat=%d\n", __func__, vterm_dec, vbat);
	if (vbat < (di->vterm - vterm_dec - BATTERY_FULL_DELTA_VOTAGE))
		return false;

	charge_enable = charge_get_charge_enable_status();
	hwlog_info("%s charge_enable=%d\n", __func__, charge_enable);
	if (!charge_enable)
		return false;

	return true;
}

static bool buck_charge_is_charging_full(struct buck_charge_dev *di)
{
	int ichg, ichg_avg, iterm_th;
	bool term_allow = false;
	bool charge_full = false;
	int bat_cap = 0;

	if (!power_platform_is_battery_exit())
		return false;

	if (!buck_charge_need_check_charging_full(di))
		return false;

	ichg = -power_platform_get_battery_current();
	ichg_avg = charge_get_battery_current_avg();
	if ((ichg > CHARGING_CURRENT_OFFSET) && (ichg_avg > CHARGING_CURRENT_OFFSET))
		term_allow = true;

	iterm_th = chg_vote_get_effective_result_with_lock(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_ITERM);
	if (iterm_th < 0)
		iterm_th = di->iterm;

	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CAPACITY, &bat_cap,
		POWER_SUPPLY_DEFAULT_CAPACITY, 1);
	hwlog_info("%s ichg=%d, ichg_avg=%d, iterm_th=%d, capacity=%d\n",
		__func__, ichg, ichg_avg, iterm_th, bat_cap);
	if (term_allow && (ichg < iterm_th) && (ichg_avg < iterm_th)) {
		di->check_full_count++;
		if (di->check_full_count >= BUCK_CHARGE_FULL_CHECK_TIMIES) {
			di->check_full_count = BUCK_CHARGE_FULL_CHECK_TIMIES;
			charge_full = true;
		}
	} else {
		di->check_full_count = 0;
	}

	return charge_full;
}

static void buck_charge_force_termination(struct buck_charge_dev *di)
{
	bool flag;

	if (!di || !di->force_term_support)
		return;

	flag = buck_charge_is_charging_full(di);
	if (flag)
		chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_ITERM, VOTE_CLIENT_USER, true, BATTERY_MAX_ITERM);
	else
		chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_ITERM, VOTE_CLIENT_USER, true, 0);
}

static void buck_charge_smart_battery_config_cc_cv(struct buck_charge_dev *di)
{
	int charge_current = coul_interface_get_desired_charging_current(COUL_TYPE_MAIN);
	int charge_voltage = coul_interface_get_desired_charging_voltage(COUL_TYPE_MAIN);

	/* set CC charge current */
	chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_FCC, VOTE_CLIENT_SMT_BATT, true, charge_current);
	hwlog_info("set desired charge current : %dmA\n", charge_current);

	/* set CV terminal voltage */
	if (charge_voltage <= 0)
		charge_voltage = (int)di->vterm;

	hwlog_info("set desired vterm %d\n", charge_voltage);
	chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_VTERM, VOTE_CLIENT_SMT_BATT, true, charge_voltage);
}

static void buck_charge_monitor_work(struct work_struct *work)
{
	int increase_volt;
	int tbat = 0;
	struct buck_charge_dev *l_dev = container_of(work, struct buck_charge_dev, buck_charge_work.work);

	if (!l_dev)
		return;

	hwlog_info("%s enter\n", __func__);

	if (!l_dev->charging_on)
		return;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbat);

	increase_volt = ffc_ctrl_get_incr_vterm();
	charge_set_buck_fv_delta(increase_volt);
	hwlog_info("%s increase_volt=%d\n", __func__, increase_volt);

	if (coul_interface_is_smart_battery(COUL_TYPE_MAIN))
		buck_charge_smart_battery_config_cc_cv(l_dev);

	if (l_dev->jeita_support) {
		buck_charge_jeita_tbatt_handler(tbat, l_dev->jeita_table, &l_dev->jeita_result);
		chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_FCC, VOTE_CLIENT_JEITA, true, l_dev->jeita_result.ichg);
		chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_VTERM, VOTE_CLIENT_JEITA, true, l_dev->jeita_result.vterm);
	}
	charge_update_buck_iin_thermal();

	ffc_ctrl_notify_ffc_info();
	buck_charge_force_termination(l_dev);

	schedule_delayed_work(&l_dev->buck_charge_work, msecs_to_jiffies(BUCK_CHARGE_WORK_TIMEOUT));
	hwlog_info("%s end\n", __func__);
}

static void buck_charge_stop_monitor_work(struct work_struct *work)
{
	struct buck_charge_dev *l_dev = container_of(work, struct buck_charge_dev, stop_charge_work);

	if (!l_dev)
		return;

	charge_set_buck_fv_delta(0);
	chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_ITERM, VOTE_CLIENT_FFC, true, l_dev->iterm);
	chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_USB_ICL, VOTE_CLIENT_FCP, false, 0);
	chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_USB_ICL, VOTE_CLIENT_RT, false, 0);
	chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_USB_ICL, VOTE_CLIENT_USER, false, 0);
}

static int buck_charge_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct buck_charge_dev *l_dev = g_buck_charge_dev;

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_CHARGING_START:
		l_dev->charging_on = true;
		schedule_delayed_work(&l_dev->buck_charge_work, msecs_to_jiffies(0));
		break;
	case POWER_NE_CHARGING_STOP:
		l_dev->charging_on = false;
		schedule_work(&l_dev->stop_charge_work);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int buck_charge_chg_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct buck_charge_dev *l_dev = g_buck_charge_dev;

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_CHG_CHARGING_DONE:
		if (l_dev->ibus_limit_after_chg_done)
			chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_USB_ICL, VOTE_CLIENT_USER, true,
				l_dev->ibus_limit_after_chg_done);
		break;
	case POWER_NE_CHG_CHARGING_RECHARGE:
		if (l_dev->ibus_limit_after_chg_done)
			chg_vote_set(VOTE_OBJ_TYPE_BUCK, VOTE_OBJ_USB_ICL, VOTE_CLIENT_USER, false, 0);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int buck_charge_parse_dts(struct device_node *np, struct buck_charge_dev *di)
{
	if (!np || !di)
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "vterm", &di->vterm, BATTERY_DEFAULT_VTERM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "iterm", &di->iterm, BATTERY_DEFAULT_ITERM);
	ffc_ctrl_set_default_info(di->vterm, di->iterm);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "jeita_support", &di->jeita_support, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "force_term_support", &di->force_term_support, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ibus_limit_after_chg_done",
		&di->ibus_limit_after_chg_done, 0);
	if (di->jeita_support)
		buck_charge_jeita_parse_jeita_table(np, (void *)di);

	return 0;
}

static int buck_charge_probe(struct platform_device *pdev)
{
	int ret;
	struct buck_charge_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	l_dev->dev = &pdev->dev;
	np = l_dev->dev->of_node;

	ret = buck_charge_parse_dts(np, l_dev);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&l_dev->buck_charge_work, buck_charge_monitor_work);
	INIT_WORK(&l_dev->stop_charge_work, buck_charge_stop_monitor_work);

	l_dev->event_nb.notifier_call = buck_charge_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &l_dev->event_nb);
	if (ret)
		goto fail_free_mem;

	l_dev->chg_event_nb.notifier_call = buck_charge_chg_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CHG, &l_dev->chg_event_nb);
	if (ret)
		goto fail_bnc_register;

	g_buck_charge_dev = l_dev;
	platform_set_drvdata(pdev, l_dev);
	return 0;

fail_bnc_register:
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->event_nb);
fail_free_mem:
	kfree(l_dev);
	g_buck_charge_dev = NULL;
	return ret;
}

static int buck_charge_remove(struct platform_device *pdev)
{
	struct buck_charge_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	cancel_delayed_work(&l_dev->buck_charge_work);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->event_nb);
	power_event_bnc_unregister(POWER_BNT_CHG, &l_dev->chg_event_nb);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_buck_charge_dev = NULL;
	return 0;
}

static const struct of_device_id buck_charge_match_table[] = {
	{
		.compatible = "huawei,buck_charge",
		.data = NULL,
	},
	{},
};

static struct platform_driver buck_charge_driver = {
	.probe = buck_charge_probe,
	.remove = buck_charge_remove,
	.driver = {
		.name = "huawei,buck_charge",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(buck_charge_match_table),
	},
};

static int __init buck_charge_init(void)
{
	return platform_driver_register(&buck_charge_driver);
}

static void __exit buck_charge_exit(void)
{
	platform_driver_unregister(&buck_charge_driver);
}

device_initcall_sync(buck_charge_init);
module_exit(buck_charge_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("buck charge driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
