// SPDX-License-Identifier: GPL-2.0
/*
 * power_extra_event.c
 *
 * extra event for power module
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

#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_extra_event.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_extra_event
HWLOG_REGIST();

static struct power_extra_event_dev *g_power_extra_event_dev;

static int power_extra_event_parse_dts(struct device_node *np, struct power_extra_event_dev *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"panel_client", (u32 *)&di->panel_client, PANEL_CLIENT_FB);

	if (di->panel_client == PANEL_CLIENT_DRM_8425)
		return power_panel_event_parse_active_panel();

	return 0;
}

static int power_extra_event_probe(struct platform_device *pdev)
{
	int ret;
	struct power_extra_event_dev *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;

	ret = power_extra_event_parse_dts(np, di);
	if (ret < 0) {
		hwlog_err("power_extra_event_parse_dts failed\n");
		kfree(di);
		return ret;
	}

	power_panel_event_register(di);

	g_power_extra_event_dev = di;
	platform_set_drvdata(pdev, di);
	return 0;
}

static int power_extra_event_remove(struct platform_device *pdev)
{
	struct power_extra_event_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_panel_event_unregister(di);
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	g_power_extra_event_dev = NULL;
	return 0;
}

static const struct of_device_id power_extra_event_match_table[] = {
	{
		.compatible = "huawei,power_extra_event",
		.data = NULL,
	},
	{},
};

static struct platform_driver power_extra_event_driver = {
	.probe = power_extra_event_probe,
	.remove = power_extra_event_remove,
	.driver = {
		.name = "huawei,power_extra_event",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(power_extra_event_match_table),
	},
};
module_platform_driver(power_extra_event_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power extra event driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
