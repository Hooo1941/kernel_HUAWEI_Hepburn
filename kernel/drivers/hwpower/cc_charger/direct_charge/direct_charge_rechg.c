// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_rechg.c
 *
 * direct_charge_rechg module
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

#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_rechg.h>
#include <securec.h>

#define HWLOG_TAG direct_charge_rechg
HWLOG_REGIST();

#define DC_RECHG_MAX_IBAT                3000
#define DC_RECHG_DFT_MAX_IBUS            1600
#define DC_RECHG_DFT_ENFORCE_IBAT        500
#define DC_RECHG_DFT_IBAT_TH_LOW         50
#define DC_RECHG_CUR_STEP                100
#define DC_RECHG_CUR_INIT                100
#define DC_RECHARGE_CNT                  10
#define DC_RECHARGE_SYS_BUF_SIZE         64
#define DC_RECHG_VBUS_5V                 5
#define DC_RECHG_VBUS_9V                 9
#define DC_RECHG_VBUS_12V                12
#define DC_RECHG_VTERM_DFT               4656
#define DC_RECHG_IBUS_LIMIT              50
#define DC_RECHG_IBUS_MIN                1000

#define DC_RECHG_IBAT_TH                 4900
#define DC_RECHG_IBUS_OPEN_TH            2400
#define DC_RECHG_IBUS_CLOSE_TH           1000

enum dc_rechg_sysfs_type {
	DC_RECHG_SYSFS_BEGIN = 0,
	DC_RECHG_SYSFS_ENFORCE_EN = DC_RECHG_SYSFS_BEGIN,
	DC_RECHG_SYSFS_ENFORCE_IBAT,
	DC_RECHG_SYSFS_IBUS_MAX_RECORD,
	DC_RECHG_SYSFS_END,
};

struct dts_para {
	int enforce_ibat;
	int max_ibus;
	int max_ibat;
	int vterm;
	int abnomal_ibat_lth;
};

struct rechging_info {
	int ibus;
	int ibat;
	int target_cur;
	int target_ibus;
	int low_ibat_cnt;
	int low_ibus_cnt;
	bool enable;
};

struct dc_rechg_dev {
	struct device *dev;
	struct dts_para dts;
	struct rechging_info info;
	bool enforce_en;
	int ibus_max_record;
};

static struct dc_rechg_dev *g_rechg_di;

bool dc_rechg_is_support()
{
	return (g_rechg_di != NULL);
}

bool dc_rechg_is_enforce_en()
{
	if (!g_rechg_di || !power_cmdline_is_factory_mode())
		return false;

	return g_rechg_di->enforce_en;
}

bool dc_rechg_is_en()
{
	if (!g_rechg_di)
		return false;

	return g_rechg_di->info.enable;
}

bool dc_rechg_is_ibus_support(int ibus)
{
	if (!g_rechg_di)
		return false;

	if (!dc_rechg_is_en())
		return ibus >= DC_RECHG_IBUS_OPEN_TH;

	if (ibus <= DC_RECHG_IBUS_CLOSE_TH)
		return ++g_rechg_di->info.low_ibus_cnt <= DC_RECHARGE_CNT;

	g_rechg_di->info.low_ibus_cnt = 0;
	return true;
}

bool dc_rechg_is_ibat_th_support(int ibat_th)
{
	return ibat_th >= DC_RECHG_IBAT_TH;
}

int dc_rechg_get_ibat()
{
	if (!g_rechg_di || !g_rechg_di->info.enable)
		return 0;

	return g_rechg_di->info.ibat;
}

static void dc_rechg_set_vbus()
{
	charge_set_vbus_vset(DC_RECHG_VBUS_9V);
}

void dc_rechg_set_enable(int enable)
{
	int vbus = 0;

	if (!g_rechg_di || (g_rechg_di->info.enable == enable))
		return;

	g_rechg_di->info.enable = enable;
	switch (enable) {
	case DC_RECHARGE_ENABLE:
		hwlog_info("need open buck rechg\n");
		dc_rechg_set_vbus();
		charge_set_dev_iin(g_rechg_di->dts.max_ibus);
		charge_set_charge_current(DC_RECHG_CUR_INIT);
		charge_set_terminal_voltage(g_rechg_di->dts.vterm);
		charge_set_charge_enable(false);
		charger_set_hiz(DC_RECHARGE_DISABLE);
		charge_set_charge_enable(true);
		charge_set_dpm_enable(DC_RECHARGE_DISABLE);
		g_rechg_di->ibus_max_record = 0;
		g_rechg_di->info.target_ibus = g_rechg_di->dts.max_ibus;
		break;
	case DC_RECHARGE_DISABLE:
		hwlog_info("need close buck rechg\n");
		charger_set_hiz(DC_RECHARGE_ENABLE);
		charge_set_charge_enable(false);
		charge_set_dpm_enable(DC_RECHARGE_ENABLE);
		memset_s(&g_rechg_di->info, sizeof(g_rechg_di->info),
			0, sizeof(g_rechg_di->info));
		break;
	default:
		hwlog_err("invalid param!\n");
	}
}

static void dc_rechg_check_ibat_lth(struct dc_rechg_dev *di)
{
	if (di->info.ibat < di->dts.abnomal_ibat_lth) {
		di->info.low_ibat_cnt++;
		if (di->info.low_ibat_cnt >= DC_RECHARGE_CNT)
			hwlog_err("rechg ibat too low!\n");
	} else {
		di->info.low_ibat_cnt = 0;
	}
}

static int dc_rechg_get_rechg_cur()
{
	int ibath = 0;
	int ibatl = 0;

	power_supply_get_int_prop("battery_gauge",
			POWER_SUPPLY_PROP_CURRENT_NOW, &ibath, 0, 1);
	power_supply_get_int_prop("battery_gauge_aux",
			POWER_SUPPLY_PROP_CURRENT_NOW, &ibatl, 0, 1);
	if (ibatl - ibath < 0)
		return 0;

	return ibatl - ibath;
}

static void dc_rechg_update_recharging_info(struct dc_rechg_dev *di)
{
	di->info.ibat = dc_rechg_get_rechg_cur();
	di->info.ibus = charge_get_ibus();
	di->ibus_max_record = power_max_positive(di->info.ibus, di->ibus_max_record);
	hwlog_info("rechg ibus=%d ibat=%d\n", di->info.ibus, di->info.ibat);
	dc_rechg_check_ibat_lth(di);
}

static void dc_rechg_ibus_regulation(struct dc_rechg_dev *di, int delta, int lim_ibus)
{
	int max_ibus;

	if (delta < 0 && (di->info.target_ibus <= DC_RECHG_IBUS_MIN)) {
		hwlog_info("target_ibus already min\n");
		return;
	}

	max_ibus = power_min_positive(lim_ibus, di->dts.max_ibus);
	if (delta > 0 && (di->info.target_ibus >= max_ibus)) {
		hwlog_info("target_ibus already max\n");
		charge_set_charge_current(g_rechg_di->info.target_cur);
		return;
	}

	g_rechg_di->info.target_ibus += delta;
	charge_set_dev_iin(g_rechg_di->info.target_ibus);
	hwlog_info("target_ibus=%d\n", g_rechg_di->info.target_ibus);
}

void dc_rechg_regulation(int lim_ibus, int target_ibat)
{
	int delta;

	if (!g_rechg_di)
		return;

	if (!g_rechg_di->info.enable)
		dc_rechg_set_enable(DC_RECHARGE_ENABLE);

	target_ibat = power_min_positive(target_ibat, g_rechg_di->dts.max_ibat);
	delta = target_ibat - g_rechg_di->info.target_cur;
	delta = delta > 0 ? DC_RECHG_CUR_STEP : -DC_RECHG_CUR_STEP;

	if (abs(delta) < DC_RECHG_CUR_STEP)
		goto regulation_exit;

	g_rechg_di->info.target_cur += delta;
	/* if ibat < target, do nothing */
	if (delta < 0 && (g_rechg_di->info.ibat <= target_ibat))
		goto regulation_exit;

	dc_rechg_ibus_regulation(g_rechg_di, delta, lim_ibus);
regulation_exit:
	dc_rechg_update_recharging_info(g_rechg_di);
}

void dc_rechg_enforce_en()
{
	if (!g_rechg_di || !power_cmdline_is_factory_mode()) {
		hwlog_err("direct recharge is null or not factory mode\n");
		return;
	}

	dc_rechg_set_enable(DC_RECHARGE_ENABLE);
	dc_rechg_regulation(g_rechg_di->dts.enforce_ibat - DC_RECHG_IBUS_LIMIT,
		g_rechg_di->dts.enforce_ibat);
}

#ifdef CONFIG_SYSFS
static ssize_t dc_rechg_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t dc_rechg_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info dc_rechg_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(dc_rechg, 0644,
		DC_RECHG_SYSFS_ENFORCE_EN, enforce_en),
	power_sysfs_attr_rw(dc_rechg, 0644,
		DC_RECHG_SYSFS_ENFORCE_IBAT, enforce_ibat),
	power_sysfs_attr_ro(dc_rechg, 0444,
		DC_RECHG_SYSFS_IBUS_MAX_RECORD, ibus_max_record),
};

#define DC_RECHG_SYSFS_ATTRS_SIZE   ARRAY_SIZE(dc_rechg_sysfs_field_tbl)

static struct attribute *dc_rechg_sysfs_attrs[DC_RECHG_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group dc_rechg_sysfs_attr_group = {
	.attrs = dc_rechg_sysfs_attrs,
};

static void dc_rechg_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(dc_rechg_sysfs_attrs, dc_rechg_sysfs_field_tbl,
		DC_RECHG_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "direct_charge_rechg",
		dev, &dc_rechg_sysfs_attr_group);
}

static void dc_rechg_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "direct_charge_rechg",
		dev, &dc_rechg_sysfs_attr_group);
}

static ssize_t dc_rechg_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct dc_rechg_dev *di = dev_get_drvdata(dev);

	if (!di)
		return -ENODEV;

	info = power_sysfs_lookup_attr(attr->attr.name,
		dc_rechg_sysfs_field_tbl, DC_RECHG_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case DC_RECHG_SYSFS_ENFORCE_EN:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1,
			"enforce_en=%d\n", di->enforce_en);
	case DC_RECHG_SYSFS_ENFORCE_IBAT:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1,
			"enforce_ibat=%d\n", di->dts.enforce_ibat);
	case DC_RECHG_SYSFS_IBUS_MAX_RECORD:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1,
			"ibat_max=%d\n", di->ibus_max_record);
	default:
		return 0;
	}
}

static ssize_t dc_rechg_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct dc_rechg_dev *di = dev_get_drvdata(dev);
	int value = 0;

	if (!di)
		return -ENODEV;

	info = power_sysfs_lookup_attr(attr->attr.name,
		dc_rechg_sysfs_field_tbl, DC_RECHG_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	if (kstrtouint(buf, POWER_BASE_DEC, &value) < 0)
		return -EINVAL;

	switch (info->name) {
	case DC_RECHG_SYSFS_ENFORCE_EN:
		if (value < 0 || value > 1)
			return -EINVAL;
		di->enforce_en = value;
		break;
	case DC_RECHG_SYSFS_ENFORCE_IBAT:
		value = power_min_positive(value, DC_RECHG_MAX_IBAT);
		di->dts.enforce_ibat = value;
		break;
	default:
		break;
	}
	return count;
}

#else
static inline void dc_rechg_sysfs_create_group(struct device *dev)
{
}

static inline void dc_rechg_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void dc_rechg_parse_dts(struct device_node *np, struct dc_rechg_dev *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_ibus", &di->dts.max_ibus, DC_RECHG_DFT_MAX_IBUS);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_ibat", &di->dts.max_ibat, DC_RECHG_MAX_IBAT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vterm", &di->dts.vterm, DC_RECHG_VTERM_DFT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"enforce_ibat", &di->dts.enforce_ibat, DC_RECHG_DFT_ENFORCE_IBAT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"abnomal_ibat_lth", &di->dts.abnomal_ibat_lth,
		DC_RECHG_DFT_IBAT_TH_LOW);
}

static int dc_rechg_probe(struct platform_device *pdev)
{
	struct dc_rechg_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	l_dev->dev = &pdev->dev;
	np = l_dev->dev->of_node;
	dc_rechg_parse_dts(np, l_dev);
	platform_set_drvdata(pdev, l_dev);
	dc_rechg_sysfs_create_group(l_dev->dev);
	g_rechg_di = l_dev;
	return 0;
}

static int dc_rechg_remove(struct platform_device *pdev)
{
	struct dc_rechg_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;
	dc_rechg_sysfs_remove_group(l_dev->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(l_dev);
	g_rechg_di = NULL;
	return 0;
}

static const struct of_device_id dc_rechg_match_table[] = {
	{
		.compatible = "huawei,direct_charge_rechg",
		.data = NULL,
	},
	{},
};

static struct platform_driver dc_rechg_driver = {
	.probe = dc_rechg_probe,
	.remove = dc_rechg_remove,
	.driver = {
		.name = "huawei,direct_charge_rechg",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dc_rechg_match_table),
	},
};

static int __init dc_rechg_init(void)
{
	return platform_driver_register(&dc_rechg_driver);
}

static void __exit dc_rechg_exit(void)
{
	platform_driver_unregister(&dc_rechg_driver);
}

device_initcall_sync(dc_rechg_init);
module_exit(dc_rechg_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct_charge_rechg driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
