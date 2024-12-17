// SPDX-License-Identifier: GPL-2.0
/*
 * battery_fake.c
 *
 * huawei faulty coul recovery drive
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/battery/battery_capacity_public.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/coul/coul_interface.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>

#define HWLOG_TAG battery_fake
HWLOG_REGIST();

#define BAT_MAIN               0
#define BAT_AUX                1
#define BASE                   10000
#define BATT_CUR_OFFSET_500    500
#define BATT_CUR_OFFSET_200    200
#define BATT_VOL_OFFSET_100    100
#define BATT_TEMP_OFFSET_8     8

struct bat_fake_device *g_bat_fake_dev;

struct bat_fake_device {
	struct device *dev;
	struct notifier_block event_nb;
	struct wakeup_source *wakelock;
	struct mutex data_lock;
	int fake_source;
	int main_batt_capacity;
	int aux_batt_capacity;
};

static int bat_fake_main_aux_coul_replace(int source)
{
	struct bat_fake_device *di = g_bat_fake_dev;
	int main_batt_capacity;
	int aux_batt_capacity;

	if (!di)
		return 0;

	main_batt_capacity = di->main_batt_capacity;
	aux_batt_capacity = di->aux_batt_capacity;
	if ((aux_batt_capacity == 0) || (main_batt_capacity == 0)) {
		hwlog_err("batt capacity is invalid.\n");
		return 0;
	}

	if (source == BAT_MAIN)
		return BASE * aux_batt_capacity / main_batt_capacity;
	else
		return BASE * main_batt_capacity / aux_batt_capacity;
}

static int bat_fake_is_ready(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;

	if (!di)
		return 0;

	return coul_interface_is_coul_ready(di->fake_source);
}

static int bat_fake_is_battery_exist(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;

	if (!di)
		return 0;

	return coul_interface_is_battery_exist(di->fake_source);
}

static int bat_fake_read_battery_soc(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;

	if (!di)
		return 0;

	return coul_interface_get_battery_capacity(di->fake_source);
}

static int bat_fake_read_battery_vol(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;
	int ret;

	if (!di)
		return 0;

	ret = coul_interface_get_battery_voltage(di->fake_source);
	if (di->fake_source == BAT_AUX)
		ret += BATT_VOL_OFFSET_100;
	return ret;
}

static int bat_fake_read_battery_current(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;
	int ret;

	if (!di)
		return 0;

	ret = coul_interface_get_battery_current(di->fake_source) *
		bat_fake_main_aux_coul_replace(di->fake_source) / BASE;
	if (di->fake_source == BAT_AUX)
		ret += BATT_CUR_OFFSET_500;
	else
		ret += BATT_CUR_OFFSET_200;
	return ret;
}

static int bat_fake_read_battery_avg_current(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;
	int ret;

	if (!di)
		return 0;

	ret = coul_interface_get_battery_avg_current(di->fake_source) *
	bat_fake_main_aux_coul_replace(di->fake_source) / BASE;
	if (di->fake_source == BAT_AUX)
		ret += BATT_CUR_OFFSET_500;
	else
		ret += BATT_CUR_OFFSET_200;
	return ret;
}

static int bat_fake_read_battery_temperature(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;
	int ret;

	if (!di)
		return 0;

	ret = coul_interface_get_battery_temperature(di->fake_source);
	if (di->fake_source == BAT_AUX)
		ret += BATT_TEMP_OFFSET_8; /* 8℃ */
	return ret;
}

static int bat_fake_read_battery_fcc(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;

	if (!di)
		return 0;

	return coul_interface_get_battery_fcc(di->fake_source) *
		bat_fake_main_aux_coul_replace(di->fake_source) / BASE;
}

static int bat_fake_read_battery_cycle(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;

	if (!di)
		return 0;

	return coul_interface_get_battery_cycle(di->fake_source);
}

static int bat_fake_read_battery_rm(void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;

	if (!di)
		return 0;

	return coul_interface_get_battery_rm(di->fake_source) *
		bat_fake_main_aux_coul_replace(di->fake_source) / BASE;
}

static int bat_fake_set_vterm_dec(int vterm_dec, void *dev_data)
{
	struct bat_fake_device *di = g_bat_fake_dev;

	if (!di)
		return 0;

	return coul_interface_set_vterm_dec(di->fake_source, vterm_dec);
}

static int bat_fake_parse_dts(struct bat_fake_device *di)
{
	struct device_node *np = di->dev->of_node;
	int ret;

	ret = power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fake_source", &di->fake_source, 0);
	ret |= power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"main_batt_capacity", &di->main_batt_capacity, 1);
	ret |= power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"aux_batt_capacity", &di->aux_batt_capacity, 1);
	return ret;
}

static struct coul_interface_ops g_bat_fake_main_ops = {
	.type_name = "main",
	.is_coul_ready = bat_fake_is_ready,
	.is_battery_exist = bat_fake_is_battery_exist,
	.get_battery_capacity = bat_fake_read_battery_soc,
	.get_battery_voltage = bat_fake_read_battery_vol,
	.get_battery_current = bat_fake_read_battery_current,
	.get_battery_avg_current = bat_fake_read_battery_avg_current,
	.get_battery_temperature = bat_fake_read_battery_temperature,
	.get_battery_fcc = bat_fake_read_battery_fcc,
	.get_battery_cycle = bat_fake_read_battery_cycle,
	.get_battery_rm = bat_fake_read_battery_rm,
	.set_vterm_dec = bat_fake_set_vterm_dec,
};

static struct coul_interface_ops g_bat_fake_aux_ops = {
	.type_name = "aux",
	.is_coul_ready = bat_fake_is_ready,
	.is_battery_exist = bat_fake_is_battery_exist,
	.get_battery_capacity = bat_fake_read_battery_soc,
	.get_battery_voltage = bat_fake_read_battery_vol,
	.get_battery_current = bat_fake_read_battery_current,
	.get_battery_avg_current = bat_fake_read_battery_avg_current,
	.get_battery_temperature = bat_fake_read_battery_temperature,
	.get_battery_fcc = bat_fake_read_battery_fcc,
	.get_battery_cycle = bat_fake_read_battery_cycle,
	.get_battery_rm = bat_fake_read_battery_rm,
	.set_vterm_dec = bat_fake_set_vterm_dec,
};

static void bat_fake_update_status(struct bat_fake_device *di)
{
	/* all coul is OK! */
	if (coul_interface_is_coul_ready(BAT_MAIN) && coul_interface_is_coul_ready(BAT_AUX)) {
		hwlog_info("All coul are normal.\n");
		return;
	}

	/* main is ok, aux is error.paras from fake_source(main 0; aux 1).Virtually register aux. */
	if (coul_interface_is_coul_ready(BAT_MAIN)) {
		di->fake_source = BAT_MAIN; /* main Correction aux */
		hwlog_info("aux_ops register.\n");
		coul_interface_ops_register(&g_bat_fake_aux_ops);
		return;
	}

	/* aux is ok, main is error.paras from fake_source(main 0; aux 1).Virtually register max. */
	if (coul_interface_is_coul_ready(BAT_AUX)) {
		di->fake_source = BAT_AUX; /* aux Correction main */
		hwlog_info("main_ops register.\n");
		coul_interface_ops_register(&g_bat_fake_main_ops);
		return;
	}
}

static bool bat_fake_coul_wait_ready()
{
	return ((coul_interface_is_coul_ready(BAT_MAIN) &&
		coul_interface_is_battery_exist(BAT_MAIN)) ||
		(coul_interface_is_coul_ready(BAT_AUX) &&
		coul_interface_is_battery_exist(BAT_AUX)));
}

static int bat_fake_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct bat_fake_device *di = g_bat_fake_dev;
	bool coul_ready = bat_fake_coul_wait_ready();

	if (!di)
		return NOTIFY_OK;

	if (event != POWER_NE_COUL_READY_STATUS)
		return NOTIFY_OK;
	hwlog_info("receive event=%lu,coul capability recovery started.\n", event);

	if (!coul_ready) {
		hwlog_err("all coul are abnormal.\n");
		return NOTIFY_OK;
	}
	hwlog_info("fake_source =%d, main_batt_capacity = %d, aux_batt_capacity=%d.\n",
		di->fake_source, di->main_batt_capacity, di->aux_batt_capacity);

	mutex_lock(&di->data_lock);
	bat_fake_update_status(di);
	mutex_unlock(&di->data_lock);

	return NOTIFY_OK;
}

static int bat_fake_probe(struct platform_device *pdev)
{
	struct bat_fake_device *di = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	ret = bat_fake_parse_dts(di);
	if (ret) {
		hwlog_err("get parse dts err!\n");
		goto fail_free_mem;
	}

	di->wakelock = power_wakeup_source_register(di->dev, "battery_fake");
	if (!di->wakelock) {
		hwlog_err("enable register error!\n");
		ret = -ENODEV;
		goto fail_free_mem;
	}

	mutex_init(&di->data_lock);
	di->event_nb.notifier_call = bat_fake_event_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_COUL, &di->event_nb);
	if (ret) {
		power_wakeup_source_unregister(di->wakelock);
		goto fail_free_mem;
	}
	g_bat_fake_dev = di;
	hwlog_info("probe successs.\n");
	return 0;
fail_free_mem:
	kfree(di);
	g_bat_fake_dev = NULL;
	return ret;
}

static int bat_fake_remove(struct platform_device *pdev)
{
	struct bat_fake_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;
	mutex_destroy(&di->data_lock);
	power_event_bnc_unregister(POWER_BNT_COUL, &di->event_nb);
	power_wakeup_source_unregister(di->wakelock);
	kfree(di);
	g_bat_fake_dev = NULL;
	return 0;
}

static const struct of_device_id bat_fake_match_table[] = {
	{
		.compatible = "huawei,battery_fake",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_fake_driver = {
	.probe = bat_fake_probe,
	.remove = bat_fake_remove,
	.driver = {
		.name = "huawei,battery_fake",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_fake_match_table),
	},
};

static int __init bat_fake_init(void)
{
	return platform_driver_register(&bat_fake_driver);
}

static void __exit bat_fake_exit(void)
{
	platform_driver_unregister(&bat_fake_driver);
}

device_initcall_sync(bat_fake_init);
module_exit(bat_fake_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("faulty coul recovery drive");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
