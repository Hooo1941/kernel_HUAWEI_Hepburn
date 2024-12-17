/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Device driver for schargerV700 watchdog
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/slab.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/time.h>

#include <securec.h>
#include <linux/printk.h>
#include <linux/workqueue.h>

#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/hw_pd_dev.h>
#endif

#include "../../../../mfd/scharger_v700/scharger_v700_adc.h"

#define DET_NUM_THRED 2

struct param {
	int sbu1_th;
	int sbu2_th;
	int dplus_th;
};

struct scharger_water_det {
	struct device *dev;
	struct regmap *regmap;
	struct param param_dts;
};

struct scharger_water_det *g_water_det = NULL;


#ifdef CONFIG_TCPC_CLASS
static int is_port_water_detected(void)
{
	int sbu1, sbu2, dplus;
	int res_count = 0;
	int res;
	struct scharger_water_det *water_det = g_water_det;

	if (!water_det) {
		scharger_err("%s water_det is NULL\n", __func__);
		return 0;
	}

	sbu1 = scharger_get_sbu1_res();
	sbu2 = scharger_get_sbu2_res();
	dplus = scharger_get_dp_res();

	scharger_inf("%s sbu1 %d, sbu2 %d, dpplus %d\n", __func__, sbu1, sbu2, dplus);

	if (sbu1 < water_det->param_dts.sbu1_th)
		res_count++;
	if (sbu2 < water_det->param_dts.sbu2_th)
		res_count++;
	if (dplus < water_det->param_dts.dplus_th)
		res_count++;

	if (res_count >= DET_NUM_THRED)
		res = WATER_DET_TRUE;
	else
		res = WATER_DET_FALSE;

	return res;
}

static struct water_det_ops det_ops = {
	.is_water_detected = is_port_water_detected,
};
#endif

static void parse_dts(struct device_node *np, struct scharger_water_det *water_det)
{
	int ret;

	ret = of_property_read_u32(np, "sbu1_th", (u32 *)&(water_det->param_dts.sbu1_th));
	if (ret) {
		scharger_err("%s get sbu1_th failed\n", __func__);
		return;
	}

	ret = of_property_read_u32(np, "sbu2_th", (u32 *)&(water_det->param_dts.sbu2_th));
	if (ret) {
		scharger_err( "%s get sbu2_th failed\n", __func__);
		return;
	}

	ret = of_property_read_u32(np, "dplus_th", (u32 *)&(water_det->param_dts.dplus_th));
	if (ret) {
		scharger_err( "%s get dplus_th failed\n", __func__);
		return;
	}
}

static int scharger_water_det_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct scharger_water_det *water_det = NULL;
	int ret = 0;

	water_det = devm_kzalloc(dev, sizeof(*water_det), GFP_KERNEL);
	if (!water_det) {
		scharger_err("[%s]wdg is null.\n", __func__);
		return -ENOMEM;
	}

	water_det->dev = dev;
	water_det->regmap = dev_get_regmap(dev->parent, NULL);
	if (!water_det->regmap) {
		scharger_err("Parent regmap unavailable.\n");
		return -ENXIO;
	}
	parse_dts(dev->of_node, water_det);

	g_water_det = water_det;

#ifdef CONFIG_TCPC_CLASS
	ret = water_det_ops_register(&det_ops);
	scharger_inf("[%s] ret %d\n", __func__, ret);
#endif
	return ret;
}

static int scharger_water_det_remove(struct platform_device *pdev)
{
	g_water_det = NULL;

	return 0;
}

static const struct of_device_id scharger_water_det_of_match[] = {
	{
		.compatible = "hisilicon,scharger-v700-water-detect",
		.data = NULL
	},
	{},
};

MODULE_DEVICE_TABLE(of, scharger_water_det_of_match);

static struct platform_driver scharger_water_det_driver = {
	.probe = scharger_water_det_probe,
	.remove = scharger_water_det_remove,
	.driver = {
		.name = "scharger-water-det",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(scharger_water_det_of_match),
	},
};

static int __init scharger_water_det_init(void)
{
	platform_driver_register(&scharger_water_det_driver);
	return 0;
}

module_init(scharger_water_det_init);

static void __exit scharger_water_det_exit(void)
{
	platform_driver_unregister(&scharger_water_det_driver);
}

module_exit(scharger_water_det_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("scharger water det driver");

