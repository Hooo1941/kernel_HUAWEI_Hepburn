/*
 * ana_hs_pop_window.c
 *
 * ana_hs_pop_window driver
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
 */

#include "ana_hs_pop_window.h"

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#ifdef CONFIG_LOG_EXCEPTION
#include "huawei_platform/log/imonitor.h"
#include "huawei_platform/log/imonitor_keys.h"
#endif

#define ANA_HS_DMD_ID     916019302
#define ANA_HS_EVENT_SIZE 64

static struct ana_hs_pop_window_priv *g_ana_hs_pop_window_priv = NULL;

struct ana_hs_dev ana_hs_pop_window_ops = {
	.tag = TAG_ANA_HS_POP_WINDOW,
};

#ifdef CONFIG_LOG_EXCEPTION
static void ana_hs_dmd_report(void)
{
	struct imonitor_eventobj *obj = NULL;
	int ret;

	obj = imonitor_create_eventobj(ANA_HS_DMD_ID);
	if (!obj) {
		pr_err("%s: create eventobj is NULL\n", __func__);
		return;
	}

	ret = imonitor_send_event(obj);
	if (ret <= 0)
		pr_err("%s: ana hs dmd sent fail\n", __func__);

	imonitor_destroy_eventobj(obj);
}
#endif

static int ana_hs_status_report(struct notifier_block *nb, unsigned long event, void *data)
{
	char kdata[ANA_HS_EVENT_SIZE] = { 0 };
	char *envp[] = { kdata, NULL };
	int ret;

	if (!g_ana_hs_pop_window_priv)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_HW_USB_HEADPHONE:
		snprintf(kdata, sizeof(kdata), "ANAHS=INSERT");
#ifdef CONFIG_LOG_EXCEPTION
		ana_hs_dmd_report();
#endif
		break;
	case POWER_NE_HW_USB_HEADPHONE_OUT:
		snprintf(kdata, sizeof(kdata), "ANAHS=REMOVE");
		break;
	default:
		return NOTIFY_OK;
	}

	pr_info("%s: report uevent: %s\n", __func__, kdata);
	ret = kobject_uevent_env(&g_ana_hs_pop_window_priv->dev->kobj, KOBJ_CHANGE, envp);
	if (ret < 0)
		pr_err("%s: uevent failed ret: %d, kdata: %s\n", __func__, ret, kdata);

	return NOTIFY_OK;
}

static int ana_hs_pop_window_probe(struct platform_device *pdev)
{
	struct ana_hs_pop_window_priv *ana_hs_priv = NULL;
	int ret;

	pr_info("%s: enter\n", __func__);

	if (!pdev) {
		pr_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ana_hs_priv = kzalloc(sizeof(*ana_hs_priv), GFP_KERNEL);
	if (!ana_hs_priv)
		return -ENOMEM;

	ana_hs_priv->dev = &pdev->dev;

	ana_hs_priv->event_nb.notifier_call = ana_hs_status_report;

	ret = power_event_bnc_register(POWER_BNT_HW_USB, &ana_hs_priv->event_nb);
	if (ret) {
		pr_err("%s: ana hs event register fail\n", __func__);
		goto err_out;
	}

	g_ana_hs_pop_window_priv = ana_hs_priv;

	ana_hs_dev_register(&ana_hs_pop_window_ops, ANA_PRIORITY_H);

	pr_info("%s: end success\n", __func__);
	return 0;

err_out:
	kfree(ana_hs_priv);
	g_ana_hs_pop_window_priv = NULL;
	return ret;
}

static int ana_hs_pop_window_remove(struct platform_device *pdev)
{
	if (!g_ana_hs_pop_window_priv) {
		pr_err("%s: g_ana_hs_pop_window_priv is NULL\n", __func__);
		return -EINVAL;
	}

	ana_hs_dev_deregister(TAG_ANA_HS_POP_WINDOW);

	kfree(g_ana_hs_pop_window_priv);
	g_ana_hs_pop_window_priv = NULL;

	return 0;
}

static const struct of_device_id ana_hs_pop_window_match[] = {
	{ .compatible = "huawei,ana_hs_pop_window", },
	{},
};
MODULE_DEVICE_TABLE(of, ana_hs_pop_window_match);

static struct platform_driver ana_hs_pop_window_driver = {
	.driver = {
		.name           = "ana_hs_pop_window",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(ana_hs_pop_window_match),
	},
	.probe    = ana_hs_pop_window_probe,
	.remove   = ana_hs_pop_window_remove,
};

static int __init ana_hs_pop_window_init(void)
{
	int ret;

	ret = platform_driver_register(&ana_hs_pop_window_driver);
	if (ret != 0)
		pr_err("%s: platform_driver_register failed, %d\n",
			__func__, ret);

	return ret;
}

static void __exit ana_hs_pop_window_exit(void)
{
	platform_driver_unregister(&ana_hs_pop_window_driver);
}

late_initcall(ana_hs_pop_window_init);
module_exit(ana_hs_pop_window_exit);

MODULE_DESCRIPTION("analog headset pop window driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

