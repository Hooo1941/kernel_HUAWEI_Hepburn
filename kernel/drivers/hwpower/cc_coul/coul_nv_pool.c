// SPDX-License-Identifier: GPL-2.0
/*
 * coul_nv_pool.c
 *
 * nv pool for coul
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/coul/coul_nv_pool.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include <huawei_platform/power/batt_info_pub.h>
#include <linux/workqueue.h>

#define HWLOG_TAG coul_nv_pool
HWLOG_REGIST();

static int g_coul_nv_pool[COUL_NV_POOL_END];
static bool g_nv_need_flush;
static struct coul_nv_pool_dev *g_coul_nv_pool_dev;

static int __init coul_nv_pool_parse_cmd(char *p)
{
	int ret;

	if (!p) {
		return 0;
	}

	ret = sscanf(p, "0x%x,0x%x,0x%x,0x%x",
		&g_coul_nv_pool[COUL_NV_POOL_BK_BATTERY_CURR_CALI], &g_coul_nv_pool[COUL_NV_POOL_OCV_INDEX],
		&g_coul_nv_pool[COUL_NV_POOL_TEMP_CYCLE], &g_coul_nv_pool[COUL_NV_POOL_TOTAL_CYCLE]);
	hwlog_info("parse_cmd ret=%d, cali=%d, ocv_idx=%d, temp_cycle=%d, total_cycle=%d\n", ret,
		g_coul_nv_pool[COUL_NV_POOL_BK_BATTERY_CURR_CALI], g_coul_nv_pool[COUL_NV_POOL_OCV_INDEX],
		g_coul_nv_pool[COUL_NV_POOL_TEMP_CYCLE], g_coul_nv_pool[COUL_NV_POOL_TOTAL_CYCLE]);

	return 0;
}
early_param("fg_para", coul_nv_pool_parse_cmd);

int coul_nv_pool_get_para(int coul_type, int mode, int *value)
{
	struct coul_nv_pool_dev *di = g_coul_nv_pool_dev;

	if (!di || !value)
		return -EPERM;

	if ((mode < COUL_NV_POOL_BEGIN) || (mode >= COUL_NV_POOL_END))
		return -EPERM;

	*value = g_coul_nv_pool[mode];

	return 0;
}

static void coul_nv_pool_update_ocv_index(int ic_role, int idx)
{
	if ((ic_role != COUL_NV_POOL_IC_TYPE_MAIN) ||
		(g_coul_nv_pool[COUL_NV_POOL_OCV_INDEX] == idx))
		return;

	g_coul_nv_pool[COUL_NV_POOL_OCV_INDEX] = idx;
	g_nv_need_flush = true;
	hwlog_info("ocv_index=%d\n", g_coul_nv_pool[COUL_NV_POOL_OCV_INDEX]);
}

static void coul_nv_pool_update_temp_cycle(int ic_role, int ic_cycle)
{
	if ((ic_role != COUL_NV_POOL_IC_TYPE_MAIN) ||
		(g_coul_nv_pool[COUL_NV_POOL_TEMP_CYCLE] == ic_cycle))
		return;

	/* if temp_cycle > ic_cycle, the fuel gauge is powered off abnormally. real cycle = temp_cycle + total_cycle */
	if (g_coul_nv_pool[COUL_NV_POOL_TEMP_CYCLE] > ic_cycle) {
		g_coul_nv_pool[COUL_NV_POOL_TOTAL_CYCLE] += g_coul_nv_pool[COUL_NV_POOL_TEMP_CYCLE];
		g_coul_nv_pool[COUL_NV_POOL_TEMP_CYCLE] = ic_cycle;
	} else {
		g_coul_nv_pool[COUL_NV_POOL_TEMP_CYCLE] = ic_cycle;
	}
	g_nv_need_flush = true;
	hwlog_info("temp_cycle=%d, total_cycle=%d\n", g_coul_nv_pool[COUL_NV_POOL_TEMP_CYCLE],
		g_coul_nv_pool[COUL_NV_POOL_TOTAL_CYCLE]);
}

static void coul_nv_pool_update_total_cycle(int ic_role, int total_cycle)
{
	if ((ic_role != COUL_NV_POOL_IC_TYPE_MAIN) ||
		(g_coul_nv_pool[COUL_NV_POOL_TOTAL_CYCLE] == total_cycle))
		return;

	g_coul_nv_pool[COUL_NV_POOL_TOTAL_CYCLE] = total_cycle;
	g_nv_need_flush = true;
}

int coul_nv_pool_set_para(int coul_type, int mode, int value)
{
	struct coul_nv_pool_dev *di = g_coul_nv_pool_dev;
	int ret = 0;

	if (!di)
		return ret;

	switch (mode) {
	case COUL_NV_POOL_BK_BATTERY_CURR_CALI:
		break;
	case COUL_NV_POOL_OCV_INDEX:
		coul_nv_pool_update_ocv_index(coul_type, value);
		break;
	case COUL_NV_POOL_TEMP_CYCLE:
		coul_nv_pool_update_temp_cycle(coul_type, value);
		break;
	case COUL_NV_POOL_TOTAL_CYCLE:
		coul_nv_pool_update_total_cycle(coul_type, value);
		break;
	default:
		break;
	}

	if (g_nv_need_flush) {
		ret = power_nv_write(POWER_NV_QCOUL, g_coul_nv_pool, sizeof(g_coul_nv_pool));
		g_nv_need_flush = false;
		if (ret) {
			hwlog_err("power_nv_write fail\n");
			g_nv_need_flush = true;
		}
	}

	return ret;
}

static void coul_nv_pool_once_work(struct work_struct *work)
{
	struct coul_nv_pool_dev *di = container_of(work,
		struct coul_nv_pool_dev, once_work.work);

	if (!di)
		return;

	if (check_battery_sn_changed()) {
		(void)coul_nv_pool_set_para(COUL_NV_POOL_IC_TYPE_MAIN, COUL_NV_POOL_TEMP_CYCLE, 0);
		(void)coul_nv_pool_set_para(COUL_NV_POOL_IC_TYPE_MAIN, COUL_NV_POOL_TOTAL_CYCLE, 0);
		hwlog_info("new battery! cycle in nv pool is cleared\n");
	}
}

static int coul_nv_pool_probe(struct platform_device *pdev)
{
	struct coul_nv_pool_dev *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	INIT_DELAYED_WORK(&di->once_work, coul_nv_pool_once_work);
	platform_set_drvdata(pdev, di);
	g_coul_nv_pool_dev = di;
	queue_delayed_work(system_power_efficient_wq, &di->once_work, 3000); /* check sn changed after 3s */

	return 0;
}

static int coul_nv_pool_remove(struct platform_device *pdev)
{
	struct coul_nv_pool_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	kfree(di);
	g_coul_nv_pool_dev = NULL;

	return 0;
}

static const struct of_device_id coul_nv_pool_match_table[] = {
	{
		.compatible = "huawei,coul_nv_pool",
		.data = NULL,
	},
	{},
};

static struct platform_driver coul_nv_pool_driver = {
	.probe = coul_nv_pool_probe,
	.remove = coul_nv_pool_remove,
	.driver = {
		.name = "huawei,coul_nv_pool",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(coul_nv_pool_match_table),
	},
};

static int __init coul_nv_pool_init(void)
{
	return platform_driver_register(&coul_nv_pool_driver);
}

static void __exit coul_nv_pool_exit(void)
{
	platform_driver_unregister(&coul_nv_pool_driver);
}

subsys_initcall_sync(coul_nv_pool_init);
module_exit(coul_nv_pool_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("coul nv pool driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
