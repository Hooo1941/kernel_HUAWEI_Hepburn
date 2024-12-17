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
#include <scharger_v700_watchdog.h>
#include "../../../../mfd/scharger_v700/scharger_v700_adc.h"

#ifndef unused
#define unused(x) ((void)(x))
#endif
struct scharger_watchdog {
	struct device *dev;
	struct regmap *regmap;
	struct delayed_work watchdog_work;
	int work_flag;
	int count;
	int time;
};

struct scharger_watchdog *g_wdg = NULL;


static int _reset_watchdog_timer(void)
{
	return regmap_update_bits(g_wdg->regmap, WATCHDOG_SOFT_RST_REG,
				WD_RST_N_MSK, WATCHDOG_TIMER_RST);
}

static void scharger_watchdog_work(struct work_struct *work)
{
	struct scharger_watchdog *wdg = container_of(work, struct scharger_watchdog,
				 watchdog_work.work);
	scharger_dbg("%s: count:%d +\n", __func__, wdg->count);

	wdg->work_flag = 1;

	if (wdg->count >= 0) {
		_reset_watchdog_timer();
		wdg->count--;
	} else {
		wdg->work_flag = 0;
		return;
	}

	queue_delayed_work(system_power_efficient_wq, &g_wdg->watchdog_work,
			msecs_to_jiffies(WATCHDOG_WORK_TIMEOUT));
}

int scharger_reset_watchdog_timer(void)
{
	int ret;
	scharger_dbg("%s\n", __func__);
	ret = _reset_watchdog_timer();

	if (g_wdg->time > WATCHDOG_TIMER_05_S) {
		g_wdg->count = g_wdg->time / WATCHDOG_WORK_TIMEOUT_S;
		if (g_wdg->work_flag == 0)
			queue_delayed_work(system_power_efficient_wq, &g_wdg->watchdog_work,
					 msecs_to_jiffies(WATCHDOG_WORK_TIMEOUT));
	}

	return ret;
}

static int scharger_set_watchdog_enable(int enable)
{
	int ret;

	ret = regmap_update_bits(g_wdg->regmap, WATCHDOG_EN_REG,
				 WATCHDOG_EN_MSK, !!enable);

	return ret;
}

int scharger_set_watchdog_timer(int value)
{
	u8 val;
	u8 dog_time = (u8)value;
	int ret = 0;

	scharger_dbg("%s value %d\n", __func__, value);
	g_wdg->time = value;
	/*
	 * val(0,1,2,3): mask values according to
	 * chip set the concrete values could be checked in
	 * nManager or <pmic_interface.h>
	 */
	if (dog_time >= WATCHDOG_TIMER_05_S)
		val = 3;
	else if (dog_time >= WATCHDOG_TIMER_02_S)
		val = 2;
	else if (dog_time >= WATCHDOG_TIMER_01_S)
		val = 1;
	else
		val = 0;

	scharger_dbg("watch dog timer is %u,the register value is set %u\n",
		     dog_time, val);
	scharger_reset_watchdog_timer();

	ret += regmap_update_bits(g_wdg->regmap, WATCHDOG_CTRL_REG, WATCHDOG_TIMER_MSK, val);
	ret += scharger_set_watchdog_enable(val);

	return ret;
}

int scharger_set_watchdog_timer_ms(int ms)
{
	return scharger_set_watchdog_timer(ms / 1000); /* 1000: micro second to second */
}

static int scharger_dc_config_watchdog_ms(int time, void *dev_data)
{
	unused(dev_data);
	return scharger_set_watchdog_timer_ms(time);
}

static int scharger_dc_kick_watchdog_timer(void *dev_data)
{
	unused(dev_data);
	return scharger_reset_watchdog_timer();
}

static int scharger_watchdog_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct scharger_watchdog *wdg = NULL;

	scharger_dbg("[%s]+\n", __func__);

	wdg = devm_kzalloc(dev, sizeof(*wdg), GFP_KERNEL);
	if (!wdg) {
		scharger_err("[%s]wdg is null.\n", __func__);
		return -ENOMEM;
	}

	wdg->dev = dev;
	wdg->regmap = dev_get_regmap(dev->parent, NULL);
	if (!wdg->regmap) {
		scharger_err("Parent regmap unavailable.\n");
		return -ENXIO;
	}

	INIT_DELAYED_WORK(&wdg->watchdog_work, scharger_watchdog_work);

	g_wdg = wdg;
	return 0;
}

static int scharger_watchdog_remove(struct platform_device *pdev)
{
	g_wdg = NULL;

	return 0;
}

static const struct of_device_id scharger_watchdog_of_match[] = {
	{
		.compatible = "hisilicon,scharger-v700-wdg",
		.data = NULL
	},
	{},
};

MODULE_DEVICE_TABLE(of, scharger_watchdog_of_match);

static struct platform_driver scharger_watchdog_driver = {
	.probe = scharger_watchdog_probe,
	.remove = scharger_watchdog_remove,
	.driver = {
		.name = "scharger-watchdog",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(scharger_watchdog_of_match),
	},
};

static int __init scharger_watchdog_init(void)
{
	platform_driver_register(&scharger_watchdog_driver);
	return 0;
}

module_init(scharger_watchdog_init);

static void __exit scharger_watchdog_exit(void)
{
	platform_driver_unregister(&scharger_watchdog_driver);
}

module_exit(scharger_watchdog_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("scharger watchdog driver");
