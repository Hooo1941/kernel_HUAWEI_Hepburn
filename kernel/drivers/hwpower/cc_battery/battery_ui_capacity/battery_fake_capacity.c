// SPDX-License-Identifier: GPL-2.0
/*
 * battery_fake_capacity.c
 *
 * huawei battery fake capacity
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

#include "battery_ui_capacity.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/battery/battery_model_public.h>
#include <chipset_common/hwpower/battery/battery_capacity_public.h>
#include <chipset_common/hwpower/hardware_monitor/low_power.h>

#define HWLOG_TAG bat_fake_capacity
HWLOG_REGIST();

#define BAT_FAKE_CAP_MAX_SUPPORT_NUM      32

struct bat_fake_cap_range {
	int low;
	int high;
};

struct bat_fake_cap_policy {
	int index;
	struct bat_fake_cap_range range;
	int target_cap;
	int policy_enable;
	int func_type;
	bool (*func)(void);
};

struct bat_fake_cap_device {
	struct device *dev;
	unsigned long support_map;
	struct notifier_block lpm_nb;
	struct delayed_work start_work;
	u32 start_work_wait_time;
};

enum bat_fake_cap_src {
	BAT_FAKE_CAP_SRC_BEGIN = 0,
	BAT_FAKE_CAP_SRC_BAT_MODEL = BAT_FAKE_CAP_SRC_BEGIN,
	BAT_FAKE_CAP_SRC_ECM_MODE,
	BAT_FAKE_CAP_SRC_END,
};

enum bat_fake_cap_switch {
	BAT_FAKE_CAP_SWITCH_BEGIN = 0,
	BAT_FAKE_CAP_SWITCH_OFF = BAT_FAKE_CAP_SWITCH_BEGIN,
	BAT_FAKE_CAP_SWITCH_ON,
	BAT_FAKE_CAP_SWITCH_END,
};

enum bat_fake_cap_func_type {
	BAT_FAKE_CAP_FUNC_TYPE_BEGIN = 0,
	BAT_FAKE_CAP_FUNC_TYPE_INVALID = BAT_FAKE_CAP_FUNC_TYPE_BEGIN,
	BAT_FAKE_CAP_FUNC_TYPE_INIT,
	BAT_FAKE_CAP_FUNC_TYPE_NOTIFY,
	BAT_FAKE_CAP_FUNC_TYPE_END,
};

static bool bat_fake_cap_src_bat_model(void);
static struct bat_fake_cap_device *g_bat_fake_cap_dev;
static struct bat_fake_cap_policy g_bat_fake_cap_array[BAT_FAKE_CAP_SRC_END] = {
	{ BAT_FAKE_CAP_SRC_BAT_MODEL, { 0, 100 }, 0, BAT_FAKE_CAP_SWITCH_OFF,
		BAT_FAKE_CAP_FUNC_TYPE_INIT, bat_fake_cap_src_bat_model }, /* using for checking silicon battery */
	{ BAT_FAKE_CAP_SRC_ECM_MODE, { 0, 1 }, 1, BAT_FAKE_CAP_SWITCH_OFF,
		BAT_FAKE_CAP_FUNC_TYPE_NOTIFY, NULL }, /* using for extending the ECM mode duration */
};

static bool bat_fake_cap_src_bat_model(void)
{
	return bat_model_match_graphite_battery(); /* if it's definite graphite battery, return true */
}

static bool bat_fake_cap_check_cap_range(struct bat_fake_cap_policy *plc_ptr, int cap)
{
	struct bat_fake_cap_range *range_ptr = &plc_ptr->range;

	hwlog_info("[bat_fake_cap_check_cap_range] cap:%d,range:[%d,%d],target:%d\n",
		cap, range_ptr->low, range_ptr->high, plc_ptr->target_cap);

	return (range_ptr->low <= cap) && (cap <= range_ptr->high);
}

int bat_fake_cap_filter(int cap)
{
	struct bat_fake_cap_device *di = g_bat_fake_cap_dev;
	int min_target = -1; /* -1 means invalid capacity */
	int i;

	if (!di)
		return cap;

	for (i = 0; i < BAT_FAKE_CAP_SRC_END; ++i) {
		if (g_bat_fake_cap_array[i].policy_enable == BAT_FAKE_CAP_SWITCH_OFF)
			continue;

		if (bat_fake_cap_check_cap_range(&g_bat_fake_cap_array[i], cap)) {
			if (min_target < 0)
				min_target = g_bat_fake_cap_array[i].target_cap;
			else
				min_target = min(min_target, g_bat_fake_cap_array[i].target_cap);
		}
	}

	return min_target < 0 ? cap : min_target;
}

static bool bat_fake_cap_check_support(struct bat_fake_cap_device *di, int src_idx)
{
	if ((src_idx < 0) || (src_idx >= BAT_FAKE_CAP_SRC_END))
		return false;

	hwlog_info("[bat_fake_cap_check_support] src_idx:%d\n", src_idx);
	if (test_bit(src_idx, &di->support_map))
		return true;

	return false;
}

/**
 * bat_fake_cap_on_start: called when os boots
 * NOTICE: this function should be called after init done of others module
 *         that this function depends on.
 *         when it checks SI battery type, it need to know the type of current battery,
 *         so it should be called after init done of battery info
 */
static void bat_fake_cap_on_start(struct bat_fake_cap_device *di)
{
	int i;

	for (i = 0; i < BAT_FAKE_CAP_SRC_END; ++i) {
		if ((g_bat_fake_cap_array[i].func_type != BAT_FAKE_CAP_FUNC_TYPE_INIT) ||
			!bat_fake_cap_check_support(di, i))
			continue;

		if (g_bat_fake_cap_array[i].func())
			g_bat_fake_cap_array[i].policy_enable = BAT_FAKE_CAP_SWITCH_ON;

		hwlog_info("[bat_fake_cap_on_start] policy_enable[%d]=%d\n",
			i, g_bat_fake_cap_array[i].policy_enable);
	}
}

static void bat_fake_cap_start_work(struct work_struct *work)
{
	struct bat_fake_cap_device *di = container_of(work, struct bat_fake_cap_device,
		start_work.work);
	if (!di) {
		hwlog_info("[bat_fake_cap_start_work] di null\n");
		return;
	}

	bat_fake_cap_on_start(di);
}

static int bat_fake_cap_parse_dts(struct bat_fake_cap_device *di)
{
	int support_num, i, ret;
	int support_arr[BAT_FAKE_CAP_MAX_SUPPORT_NUM];
	struct device_node *np = di->dev->of_node;

	/* wait for init done of battery info, see `bat_fake_cap_on_start` */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"start_work_wait_time", &di->start_work_wait_time, 0);

	support_num = of_property_count_u32_elems(np, "fake_src_support");
	if (support_num > BAT_FAKE_CAP_MAX_SUPPORT_NUM) {
		hwlog_err("[bat_fake_cap_parse_dts] fake_src_support out of range\n");
		return -EINVAL;
	}

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"fake_src_support", support_arr, support_num);
	if (ret < 0) {
		hwlog_err("[bat_fake_cap_parse_dts] support NA\n");
		memset32(support_arr, -1, BAT_FAKE_CAP_MAX_SUPPORT_NUM); /* -1 means not support */
	}

	for (i = 0; i < support_num; ++i) {
		if ((support_arr[i] < 0) || (support_arr[i] >= BAT_FAKE_CAP_MAX_SUPPORT_NUM))
			continue;

		set_bit(support_arr[i], &di->support_map);
	}
	hwlog_info("[bat_fake_cap_parse_dts] support_map=0x%x\n", di->support_map);

	return 0;
}

static void bat_fake_cap_try_switch(struct bat_fake_cap_device *di, int mode, bool enable)
{
	if (!bat_fake_cap_check_support(di, mode))
		return;

	g_bat_fake_cap_array[mode].policy_enable =
		enable ? BAT_FAKE_CAP_SWITCH_ON : BAT_FAKE_CAP_SWITCH_OFF;

	hwlog_info("[bat_fake_cap_try_switch] policy_enable[%d]=%d\n",
		mode, g_bat_fake_cap_array[mode].policy_enable);
}

static int bat_fake_cap_lpm_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	u32 ecm_status;
	struct bat_fake_cap_device *di = g_bat_fake_cap_dev;
	bool enable = false;

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_BAT_ECM_TRIGGER_STATUS:
		ecm_status = *(u32 *)data;
		enable = (ecm_status == ECM_TRIGGER_CN) ||
			(ecm_status == ECM_TRIGGER_OVERSEA);
		bat_fake_cap_try_switch(di, BAT_FAKE_CAP_SRC_ECM_MODE, enable);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int bat_fake_cap_probe(struct platform_device *pdev)
{
	struct bat_fake_cap_device *di = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	ret = bat_fake_cap_parse_dts(di);
	if (ret) {
		devm_kfree(&pdev->dev, di);
		return ret;
	}

	platform_set_drvdata(pdev, di);
	INIT_DELAYED_WORK(&di->start_work, bat_fake_cap_start_work);
	schedule_delayed_work(&di->start_work, msecs_to_jiffies(di->start_work_wait_time));
	di->lpm_nb.notifier_call = bat_fake_cap_lpm_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_LOW_POWER, &di->lpm_nb);
	if (ret < 0) {
		hwlog_err("nb_register: register lpm notifier failed\n");
		goto fail_free_mem;
	}
	g_bat_fake_cap_dev = di;
	hwlog_info("bat_fake_cap_probe ok\n");

	return 0;

fail_free_mem:
	kfree(di);
	g_bat_fake_cap_dev = NULL;
	return ret;
}

static int bat_fake_cap_remove(struct platform_device *pdev)
{
	struct bat_fake_cap_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_bnc_unregister(POWER_BNT_LOW_POWER, &di->lpm_nb);
	devm_kfree(&pdev->dev, di);
	g_bat_fake_cap_dev = NULL;

	return 0;
}

static const struct of_device_id bat_fake_cap_match_table[] = {
	{
		.compatible = "huawei,battery_fake_cap",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_fake_cap_driver = {
	.probe = bat_fake_cap_probe,
	.remove = bat_fake_cap_remove,
	.driver = {
		.name = "huawei,battery_fake_cap",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_fake_cap_match_table),
	},
};

static int __init bat_fake_cap_init(void)
{
	return platform_driver_register(&bat_fake_cap_driver);
}

static void __exit bat_fake_cap_exit(void)
{
	platform_driver_unregister(&bat_fake_cap_driver);
}

device_initcall_sync(bat_fake_cap_init);
module_exit(bat_fake_cap_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery fake capacity driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");