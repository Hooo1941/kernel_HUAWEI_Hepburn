// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge_ic.c
 *
 * buck charge ic module
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic.h>

#define HWLOG_TAG buck_charge_ic
HWLOG_REGIST();

struct charge_device_ops *g_ops[BUCK_IC_TYPE_END];
static struct bc_ic_dev *g_bc_ic_di;

int charge_ops_register(struct charge_device_ops *ops, int ic_type)
{
	if (!ops) {
		hwlog_err("charge ops is null\n");
		return -EPERM;
	}

	if ((ic_type < BUCK_IC_TYPE_BEGIN) || (ic_type >= BUCK_IC_TYPE_END)) {
		hwlog_err("ic_type is invalid\n");
		return -EPERM;
	}

	if (g_ops[ic_type]) {
		hwlog_err("ops[%d] exist, register failed\n", ic_type);
		return -EPERM;
	}

	g_ops[ic_type] = ops;
	hwlog_info("charge ops register ok\n");
	return 0;
}

struct charge_device_ops *bc_ic_get_ic_ops(void)
{
	struct bc_ic_dev *di = g_bc_ic_di;

	if (!di || !di->probe_finish)
		return NULL;

	if ((di->ic_type < BUCK_IC_TYPE_BEGIN) || (di->ic_type >= BUCK_IC_TYPE_END)) {
		hwlog_err("prase type is invalid\n");
		return NULL;
	}

	return g_ops[di->ic_type];
}

int bc_ic_get_convert_ratio(void)
{
	if (!g_bc_ic_di || !g_bc_ic_di->convert_ratio)
		return 1;

	return g_bc_ic_di->convert_ratio;
}

static void bc_ic_parse_dts(struct device_node *np, struct bc_ic_dev *di)
{
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ic_type", &di->ic_type, BUCK_IC_TYPE_PLATFORM);
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"convert_ratio", &di->convert_ratio, 1);
}

static int bc_ic_probe(struct platform_device *pdev)
{
	struct bc_ic_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_bc_ic_di = l_dev;
	np = pdev->dev.of_node;

	bc_ic_parse_dts(np, l_dev);
	platform_set_drvdata(pdev, l_dev);
	g_bc_ic_di->probe_finish = true;

	return 0;
}

static int bc_ic_remove(struct platform_device *pdev)
{
	struct dc_ic_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_bc_ic_di = NULL;

	return 0;
}

static const struct of_device_id bc_ic_match_table[] = {
	{
		.compatible = "huawei,buck_charge_ic",
		.data = NULL,
	},
	{},
};

static struct platform_driver bc_ic_driver = {
	.probe = bc_ic_probe,
	.remove = bc_ic_remove,
	.driver = {
		.name = "huawei,buck_charge_ic",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bc_ic_match_table),
	},
};

static int __init bc_ic_init(void)
{
	return platform_driver_register(&bc_ic_driver);
}

static void __exit bc_ic_exit(void)
{
	platform_driver_unregister(&bc_ic_driver);
}

device_initcall_sync(bc_ic_init);
module_exit(bc_ic_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("buck charge ic driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
