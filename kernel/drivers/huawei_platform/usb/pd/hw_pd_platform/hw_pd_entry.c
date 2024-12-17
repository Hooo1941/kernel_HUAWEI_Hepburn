/*
 * hw_pd_entry.c
 *
 * Source file of platform entry for huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/module.h>

#include <chipset_common/hwusb/hw_pd/hw_pd_core.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_pd_entry
HWLOG_REGIST();
#endif /* HWLOG_TAG */

int support_dp = 1;

static void pd_cc_protection_dts_parser(struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnormal_cc_detection", &pd_state(abnormal_cc_detection), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_dynamic_protect", &pd_state(cc_dynamic_protect), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnormal_cc_interval", &pd_state(abnormal_cc_interval), PD_DPM_CC_CHANGE_INTERVAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"moisture_detection_by_cc_enable", &pd_state(cc_detect_moisture), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_moisture_status_report", &pd_state(cc_moisture_report), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_abnormal_dmd_report_enable", &pd_state(cc_abnormal_dmd), 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"external_pd_flag", &pd_state(external_pd_flag), 0);
}

static void pd_misc_dts_parser(struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_manual_enable", &pd_state(switch_manual_enable), 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_dp", &pd_state(support_dp), 1);
	support_dp = pd_state(support_dp);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_channel", &pd_state(otg_channel), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibus_check", &pd_state(ibus_check), 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vbus_only_min_vlotage", &pd_state(vbus_only_min_voltage), 0);
}

static void pd_accessory_dts_parser(struct device_node *np)
{
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_smart_holder", &pd_state(support_smart_holder), 0);
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_analog_audio", &pd_state(support_analog_audio), 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"emark_detect_enable", &pd_state(emark_detect_enable), 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"emark_detect_finish_not_support", &pd_state(emark_finish_disable), 0);
}

/* Read dts config for 'pd reinit' work */
static void pd_reinit_dts_parser(struct pd_dpm_info *di, struct device_node *np)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pd_reinit_enable", &di->pd_reinit_enable, 0))
		return;

	if (!di->pd_reinit_enable) {
		hwlog_err("pd reinit not enable\n");
		return;
	}

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"ldo_name", &di->ldo_name))
		return;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_ocp_count", &di->max_ocp_count, 1000); /* default 1000 times */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ocp_delay_time", &di->ocp_delay_time, 5000); /* default 5000 ms */
}

static void pd_state_parse_dts(struct pd_dpm_info *di, struct device_node *np)
{
	pd_cc_protection_dts_parser(np);
	pd_misc_dts_parser(np);
	pd_accessory_dts_parser(np);
	pd_reinit_dts_parser(di, np);
	hwlog_info("parse_dts success\n");
}

static int hw_pd_probe(struct platform_device *pdev)
{
	struct pd_dpm_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("%s begin\n", __func__);
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	pd_state_parse_dts(di, np);

	platform_set_drvdata(pdev, di);
	hw_pd_register_instance(di);

	return 0;
}

static const struct of_device_id hw_pd_callback_match_table[] = {
	{
		.compatible = "huawei,pd_dpm",
		.data = NULL,
	},
	{},
};

static struct platform_driver hw_pd_callback_driver = {
	.probe = hw_pd_probe,
	.remove = NULL,
	.driver = {
		.name = "huawei,pd_dpm",
		.owner = THIS_MODULE,
		.of_match_table = hw_pd_callback_match_table,
	}
};

static int __init hw_pd_init(void)
{
	return platform_driver_register(&hw_pd_callback_driver);
}

static void __exit hw_pd_exit(void)
{
	platform_driver_unregister(&hw_pd_callback_driver);
}

module_init(hw_pd_init);
module_exit(hw_pd_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("hw pf logic driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
