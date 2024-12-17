// SPDX-License-Identifier: GPL-2.0
/*
 * bat_par_ctrl.c
 *
 * battery parallel control driver
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <securec.h>
#include "scharger_v700_buck.h"
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_vote.h>

#define HWLOG_TAG bat_par_ctrl
HWLOG_REGIST();

#define BAT_PAR_CTRL_VOTE_OBJECT        "BAT_PAR_CTRL"
#define SERIAL_CHARGE_VOTER             "SERIAL_CHARGE"
#define BUCK_CV_CHARGE_VOTER            "BUCK_CV_CHARGE"
#define BAT_TEMP_VOTER                  "BAT_TEMP_CHARGE"
#define BAT_PSY_NAME_LEN                64
#define BAT_PAR_CTRL_INFO_SIZE          4
#define BAT_PAR_CTRL_WORK_DEBOUNCE_TIME 500
#define BAT_PAR_CTRL_WORK_DELAY         10000
#define DUAL_BAT_TEMP_WORK_DELAY        5000
#define BAT_TEMP_TH                     55
#define FAC_BAT_TEMP_TH                 60

#define BATFET_OFF_VTERM                4656
#define BATFET_ON_VTERM                 4515

enum {
	BAT_PAR_CTRL_BAT_NAME = 0,
	BAT_PAR_CTRL_BAT_VOLT_HIGH_TH,
	BAT_PAR_CTRL_BAT_VOLT_LOW_TH,
	BAT_PAR_CTRL_INFO_TOTAL,
};

enum {
	BATFET_CTRL_VBAT_LTH = 0,
	BATFET_CTRL_VBAT_HTH,
	BATFET_CTRL_SOC_TH,
	BATFET_CTRL_TOTAL,
};

struct bat_par_ctrl_info {
	char bat_name[BAT_PSY_NAME_LEN];
	u32 bat_volt_high_th;
	u32 bat_volt_low_th;
	int batfet_vol_lth;
	int batfet_vol_hth;
	int soc_th;
};

struct bat_par_ctrl {
	struct device *dev;
	struct notifier_block nb;
	struct notifier_block dc_nb;
	struct delayed_work monitor_work;
	struct delayed_work bat_temp_work;
	int parallel_switch;
	int switch_off_value;
	struct bat_par_ctrl_info *ctrl_info;
	int info_num;
	int bat_temp_th;
	int batfet_switch;
	int batfet_on_vterm;
	int batfet_off_vterm;
};

static struct bat_par_ctrl *g_bat_par_ctrl_dev;

int bat_par_ctrl_set_switch_off(bool enable)
{
	int ret;
	struct bat_par_ctrl *di = g_bat_par_ctrl_dev;

	if (!di)
		return 0;

	if (enable)
		ret = power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT,
			SERIAL_CHARGE_VOTER, true, true);
	else
		ret = power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT,
			SERIAL_CHARGE_VOTER, false, false);

	hwlog_info("switch_status=%d\n", gpio_get_value(di->parallel_switch));
	return ret;
}

static void bat_par_ctrl_batfet(void)
{
	int i;
	bool enable = true;
	int vols[BAT_PAR_CTRL_INFO_SIZE] = { 0 };
	int socs[BAT_PAR_CTRL_INFO_SIZE] = { 0 };
	struct bat_par_ctrl *di = g_bat_par_ctrl_dev;

	if (!di || !di->batfet_switch)
		return;

	if (direct_charge_in_charging_stage() != DC_NOT_IN_CHARGING_STAGE)
		return;

	for (i = 0; i < di->info_num; i++) {
		power_supply_get_int_property_value(di->ctrl_info[i].bat_name,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, &vols[i]);
		vols[i] /= POWER_UV_PER_MV;
		power_supply_get_int_property_value(di->ctrl_info[i].bat_name,
			POWER_SUPPLY_PROP_CAPACITY, &socs[i]);
		enable = enable && (vols[i] >= di->ctrl_info[i].batfet_vol_lth) &&
			(vols[i] <= di->ctrl_info[i].batfet_vol_hth) &&
			(socs[i] >= di->ctrl_info[i].soc_th);
	}

	if (enable && (socs[0] > socs[1])) {
		power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT, BUCK_CV_CHARGE_VOTER,
			true, true);
		scharger_set_terminal_voltage(di->batfet_off_vterm);
		set_batfet_single_ctrl(1, DBFET_BAT_H_OFF);
	} else {
		scharger_set_terminal_voltage(di->batfet_on_vterm);
		set_batfet_single_ctrl(0, DBFET_BAT_H_OFF);
	}
}

static void bat_par_ctrl_work(struct work_struct *work)
{
	int i;
	int value[BAT_PAR_CTRL_INFO_SIZE] = { 0 };
	int switch_off_cnt = 0;
	int switch_on_cnt = 0;
	struct bat_par_ctrl *di = container_of(work, struct bat_par_ctrl,
		monitor_work.work);
	static bool switch_off_flag = false;

	if (!di || !di->ctrl_info)
		return;

	for (i = 0; i < di->info_num; i++) {
		power_supply_get_int_property_value(di->ctrl_info[i].bat_name,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, &value[i]);
		value[i] /= POWER_UV_PER_MV;
		if (value[i] > (int)di->ctrl_info[i].bat_volt_high_th)
			switch_off_cnt++;
		if (value[i] < (int)di->ctrl_info[i].bat_volt_low_th)
			switch_on_cnt++;
	}

	if ((switch_off_cnt == di->info_num) &&
		(direct_charge_in_charging_stage() == DC_NOT_IN_CHARGING_STAGE)) {
		power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT, BUCK_CV_CHARGE_VOTER,
			true, true);
		switch_off_flag = true;
	}

	if (switch_off_flag && switch_on_cnt) {
		power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT, BUCK_CV_CHARGE_VOTER,
			false, false);
		switch_off_flag = false;
	}

	bat_par_ctrl_batfet();
	schedule_delayed_work(&di->monitor_work,
		msecs_to_jiffies(BAT_PAR_CTRL_WORK_DELAY));
}

static void bat_par_ctrl_bat_temp_work(struct work_struct *work)
{
	struct bat_par_ctrl *di = container_of(work, struct bat_par_ctrl,
		bat_temp_work.work);
	int bat_main_temp = 0;
	int bat_aux_temp = 0;

	if (!di)
		return;

	bat_temp_get_temperature(BTB_TEMP_0, &bat_main_temp);
	bat_temp_get_temperature(BTB_TEMP_1, &bat_aux_temp);
	hwlog_info("bat_main_temp=%d, bat_aux_temp=%d, temp_th=%d\n",
		bat_main_temp, bat_aux_temp, di->bat_temp_th);
	if ((bat_main_temp >= di->bat_temp_th) ||
		(bat_aux_temp >= di->bat_temp_th)) {
		hwlog_info("bat temp over th, need switch off bat parralle\n");
		power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT, BAT_TEMP_VOTER,
			true, true);
	}

	if ((bat_main_temp < di->bat_temp_th) &&
		(bat_aux_temp < di->bat_temp_th))
		power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT, BAT_TEMP_VOTER,
			false, false);

	queue_delayed_work(system_freezable_power_efficient_wq,
		&di->bat_temp_work, msecs_to_jiffies(DUAL_BAT_TEMP_WORK_DELAY));
}

static int bat_par_ctrl_event_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct bat_par_ctrl *di = g_bat_par_ctrl_dev;

	if (!di)
		return NOTIFY_OK;

	hwlog_info("%s receive event %lu\n", __func__, event);
	switch (event) {
	case POWER_NE_CHARGING_START:
		schedule_delayed_work(&di->monitor_work,
			msecs_to_jiffies(BAT_PAR_CTRL_WORK_DEBOUNCE_TIME));
		break;
	case POWER_NE_CHARGING_STOP:
		if (di->batfet_switch)
			set_batfet_single_ctrl(0, DBFET_BAT_H_OFF);
		cancel_delayed_work_sync(&di->monitor_work);
		power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT, BUCK_CV_CHARGE_VOTER,
			false, false);
		break;
	case POWER_NE_DC_LVC_CHARGING:
		power_vote_set(BAT_PAR_CTRL_VOTE_OBJECT, BUCK_CV_CHARGE_VOTER,
			false, false);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int bat_par_ctrl_cb(struct power_vote_object *obj,
	void *data, int result, const char *client)
{
	struct bat_par_ctrl *di = (struct bat_par_ctrl *)data;

	if (!di)
		return -EINVAL;

	if (result)
		gpio_set_value(di->parallel_switch, di->switch_off_value);
	else
		gpio_set_value(di->parallel_switch, !di->switch_off_value);
	hwlog_info("%s result=%d, switch status=%d\n", __func__, result,
		gpio_get_value(di->parallel_switch));
	return 0;
}

static int bat_par_ctrl_parse_bat_info(struct device_node *np,
	struct bat_par_ctrl *di)
{
	int i, row, col, array_len, num;
	const char *tmp_string = NULL;
	struct bat_par_ctrl_info *info = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"ctrl_info", BAT_PAR_CTRL_INFO_SIZE, BAT_PAR_CTRL_INFO_TOTAL);
	if (array_len < 0)
		return -EINVAL;

	num = array_len / BAT_PAR_CTRL_INFO_TOTAL;
	info = kzalloc(sizeof(*info) * num, GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "ctrl_info", i, &tmp_string))
			goto err_out;

		row = i / BAT_PAR_CTRL_INFO_TOTAL;
		col = i % BAT_PAR_CTRL_INFO_TOTAL;

		switch (col) {
		case BAT_PAR_CTRL_BAT_NAME:
			(void)strncpy_s(info[row].bat_name, BAT_PSY_NAME_LEN,
				tmp_string, BAT_PSY_NAME_LEN - 1);
			break;
		case BAT_PAR_CTRL_BAT_VOLT_HIGH_TH:
			if (kstrtoint(tmp_string, POWER_BASE_DEC,
				&info[row].bat_volt_high_th))
				goto err_out;
			break;
		case BAT_PAR_CTRL_BAT_VOLT_LOW_TH:
			if (kstrtoint(tmp_string, POWER_BASE_DEC,
				&info[row].bat_volt_low_th))
				goto err_out;
			break;
		default:
			break;
		}
	}
	di->ctrl_info = info;
	di->info_num = num;
	for (i = 0; i < num; i++)
		hwlog_info("ctrl_info[%d]=%s %d %d\n", i,
			di->ctrl_info[i].bat_name,
			di->ctrl_info[i].bat_volt_high_th,
			di->ctrl_info[i].bat_volt_low_th);
	return 0;

err_out:
	kfree(info);
	return -EINVAL;
}

static int bat_par_ctrl_parse_batfet_info(struct device_node *np,
	struct bat_par_ctrl *di)
{
	int i, row, col, array_len;
	const char *tmp_string = NULL;
	struct bat_par_ctrl_info *info = di->ctrl_info;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"ctrl_batfet_info", BAT_PAR_CTRL_INFO_SIZE, BATFET_CTRL_TOTAL);
	if (array_len < 0 || !info)
		return -EINVAL;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "ctrl_batfet_info", i, &tmp_string))
			return -EINVAL;

		row = i / BATFET_CTRL_TOTAL;
		col = i % BATFET_CTRL_TOTAL;
		switch (col) {
		case BATFET_CTRL_VBAT_LTH:
			if (kstrtoint(tmp_string, POWER_BASE_DEC,
				&info[row].batfet_vol_lth))
				return -EINVAL;
			break;
		case BATFET_CTRL_VBAT_HTH:
			if (kstrtoint(tmp_string, POWER_BASE_DEC,
				&info[row].batfet_vol_hth))
				return -EINVAL;
			break;
		case BATFET_CTRL_SOC_TH:
			if (kstrtoint(tmp_string, POWER_BASE_DEC,
				&info[row].soc_th))
				return -EINVAL;
			break;
		default:
			break;
		}
	}
	return 0;
}

static int bat_par_ctrl_parse_dts(struct device_node *np,
	struct bat_par_ctrl *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_off_value", (u32 *)&di->switch_off_value, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"batfet_switch", (u32 *)&di->batfet_switch, 0);
	if (power_cmdline_is_factory_mode())
		di->bat_temp_th = FAC_BAT_TEMP_TH;
	else
		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			"bat_temp_th", (u32 *)&di->bat_temp_th, BAT_TEMP_TH);
	ret = power_gpio_config_output(np, "parallel_switch", "parallel_switch",
		&di->parallel_switch, !di->switch_off_value);
	ret += bat_par_ctrl_parse_bat_info(np, di);
	if (di->batfet_switch) {
		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			"batfet_on_vterm", (u32 *)&di->batfet_on_vterm, BATFET_ON_VTERM);
		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			"batfet_off_vterm", (u32 *)&di->batfet_off_vterm, BATFET_OFF_VTERM);
		ret += bat_par_ctrl_parse_batfet_info(np, di);
	}

	return ret;
}

static int bat_par_ctrl_probe(struct platform_device *pdev)
{
	int ret;
	struct bat_par_ctrl *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;

	ret = bat_par_ctrl_parse_dts(np, di);
	if (ret) {
		hwlog_err("bat_par_ctrl_parse_dts failed\n");
		goto fail_free_mem;
	}

	INIT_DELAYED_WORK(&di->monitor_work, bat_par_ctrl_work);
	INIT_DELAYED_WORK(&di->bat_temp_work, bat_par_ctrl_bat_temp_work);

	if (power_vote_create_object(BAT_PAR_CTRL_VOTE_OBJECT,
		POWER_VOTE_SET_ANY, bat_par_ctrl_cb, di))
		hwlog_err("create BAT_PAR_CTRL object failed\n");

	di->nb.notifier_call = bat_par_ctrl_event_call;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &di->nb);
	if (ret)
		goto fail_free_mem1;

	di->dc_nb.notifier_call = bat_par_ctrl_event_call;
	(void)power_event_bnc_register(POWER_BNT_DC, &di->dc_nb);

	queue_delayed_work(system_freezable_power_efficient_wq,
		&di->bat_temp_work, 0);
	platform_set_drvdata(pdev, di);
	g_bat_par_ctrl_dev = di;
	return 0;

fail_free_mem1:
	power_vote_destroy_object(BAT_PAR_CTRL_VOTE_OBJECT);
	if (di->parallel_switch > 0) {
		gpio_set_value(di->parallel_switch, di->switch_off_value);
		gpio_free(di->parallel_switch);
	}
fail_free_mem:
	if (di->ctrl_info)
		kfree(di->ctrl_info);
	devm_kfree(&pdev->dev, di);
	g_bat_par_ctrl_dev = NULL;
	return ret;
}

static int bat_par_ctrl_remove(struct platform_device *pdev)
{
	struct bat_par_ctrl *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_bnc_unregister(POWER_BNT_DC, &di->dc_nb);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &di->nb);
	power_vote_destroy_object(BAT_PAR_CTRL_VOTE_OBJECT);
	if (di->parallel_switch > 0) {
		gpio_set_value(di->parallel_switch, di->switch_off_value);
		gpio_free(di->parallel_switch);
	}
	kfree(di->ctrl_info);
	devm_kfree(&pdev->dev, di);
	g_bat_par_ctrl_dev = NULL;
	platform_set_drvdata(pdev, NULL);
	return 0;
}

#ifdef CONFIG_PM
static int bat_par_ctrl_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bat_par_ctrl *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	hwlog_info("%s +++\n", __func__);
	cancel_delayed_work(&di->bat_temp_work);
	hwlog_info("%s ---\n", __func__);
	return 0;
}

static int bat_par_ctrl_resume(struct platform_device *pdev)
{
	struct bat_par_ctrl *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	hwlog_info("%s +++\n", __func__);
	queue_delayed_work(system_freezable_power_efficient_wq,
		&di->bat_temp_work, 0);
	hwlog_info("%s ---\n", __func__);
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id bat_par_ctrl_match_table[] = {
	{
		.compatible = "huawei,bat_par_ctrl",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_par_ctrl_driver = {
	.probe = bat_par_ctrl_probe,
	.remove = bat_par_ctrl_remove,
#ifdef CONFIG_PM
	.suspend = bat_par_ctrl_suspend,
	.resume = bat_par_ctrl_resume,
#endif /* CONFIG_PM */
	.driver = {
		.name = "huawei,bat_par_ctrl",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_par_ctrl_match_table),
	},
};
module_platform_driver(bat_par_ctrl_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery parallel control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
