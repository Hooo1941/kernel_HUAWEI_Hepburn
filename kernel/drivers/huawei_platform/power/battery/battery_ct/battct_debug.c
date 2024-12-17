// SPDX-License-Identifier: GPL-2.0
/*
 * battct_debug.c
 *
 * Debug system for battery ct
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

#include "batt_aut_checker.h"
#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/of.h>
#include <securec.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG battct_debug
HWLOG_REGIST();

#define BATTCT_DBG_SN_ASC_LEN             16
#define BATTCT_DBG_UID_LEN                7
#define BATTCT_DBG_ACT_LEN                60
#define BATTCT_DBG_SWI_STR_LEN            200

struct battct_dbg_ic_info {
	bool uid_set;
	unsigned char uid[BATTCT_DBG_UID_LEN];
	bool sn_set;
	unsigned char sn[BATTCT_DBG_SN_ASC_LEN];
	bool source_set;
	enum batt_source source;
	bool group_sn_set;
	unsigned char group_sn[BATTCT_DBG_SN_ASC_LEN];
	unsigned char act_sign[BATTCT_DBG_ACT_LEN];
};

struct battct_dbg_dev {
	struct battct_dbg_ic_info ic_info;
	bool debug_on;
	char swi_val[BATTCT_DBG_SWI_STR_LEN];
	struct batt_ct_ops debug_ops;
};

static struct battct_dbg_dev *g_battct_dbg_dev;

static int battct_dbg_get_ic_uuid(struct platform_device *pdev,
	const unsigned char **uuid, unsigned int *uuid_len)
{
	struct battct_dbg_dev *di = g_battct_dbg_dev;

	(void)pdev;
	if (!di || !uuid || !uuid_len) {
		hwlog_err("%s pointer NULL\n", __func__);
		return -EINVAL;
	}

	if (!di->debug_on || !di->ic_info.uid_set)
		return -EINVAL;

	*uuid = di->ic_info.uid;
	*uuid_len = BATTCT_DBG_UID_LEN;
	return 0;
}

/*
 * Replace origin func: get_batt_sn
 * Use to get sn from debug_info
 */
static int battct_dbg_get_batt_sn(struct platform_device *pdev,
	struct power_genl_attr *res, const unsigned char **sn, unsigned int *sn_len)
{
	struct battct_dbg_dev *di = g_battct_dbg_dev;

	(void)res;
	(void)pdev;
	if (!di || !sn || !sn_len) {
		hwlog_err("%s pointer NULL\n", __func__);
		return -EINVAL;
	}

	if (!di->debug_on || !di->ic_info.sn_set)
		return -EINVAL;

	*sn = di->ic_info.sn;
	*sn_len = BATTCT_DBG_SN_ASC_LEN;
	return 0;
}

/*
 * Replace origin func get_batt_safe_info
 * Use to get source and group_sn from debug_info
 */
static int battct_dbg_get_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	struct battct_dbg_dev *di = g_battct_dbg_dev;

	(void)pdev;
	if (!di || !value) {
		hwlog_err("%s pointer NULL\n", __func__);
		return -EINVAL;
	}

	if (!di->debug_on)
		return -EINVAL;

	switch (type) {
	case BATT_CHARGE_CYCLES:
		*(int *)value = BATT_INVALID_CYCLES;
		break;
	case BATT_SPARE_PART:
		*(enum batt_source *)value = di->ic_info.source;
		break;
	case BATT_CERT_READY:
		*(enum batt_cert_state *)value = CERT_READY;
		break;
	case BATT_MATCH_INFO:
		*(unsigned char **)value = di->ic_info.group_sn;
		break;
	default:
		hwlog_err("%s type error\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/*
 * Replace origin func: prepare
 * Use to get a illegal act_sign
 */
static int battct_dbg_ic_prepare(struct platform_device *pdev,
	enum res_type type, struct power_genl_attr *res)
{
	struct battct_dbg_dev *di = g_battct_dbg_dev;

	(void)pdev;
	if (!di || !di->debug_on || !res) {
		hwlog_err("%s debug not on\n", __func__);
		return -EINVAL;
	}

	switch (type) {
	case RES_CT:
		res->data = di->ic_info.uid;
		res->len = BATTCT_DBG_UID_LEN;
		break;
	case RES_ACT:
		res->data = di->ic_info.act_sign;
		res->len = BATTCT_DBG_ACT_LEN;
		break;
	default:
		hwlog_err("%s invalid option\n");
		res->data = NULL;
		res->len = 0;
		break;
	}
	return 0;
}

/*
 * Replace origin func: certification
 * Use to get batt certification
 */
static int battct_dbg_ic_certification(struct platform_device *pdev,
	struct power_genl_attr *res, enum key_cr *result)
{
	struct battct_dbg_dev *di = g_battct_dbg_dev;

	(void)pdev;
	(void)res;
	if (!di || !di->debug_on || !result) {
		hwlog_err("%s debug not on\n", __func__);
		return -EINVAL;
	}

	*result = KEY_PASS;
	return 0;
}

static void battct_dbg_ic_ops_register(struct battct_dbg_dev *di)
{
	di->debug_ops.get_ic_uuid = battct_dbg_get_ic_uuid;
	di->debug_ops.get_batt_sn = battct_dbg_get_batt_sn;
	di->debug_ops.prepare = battct_dbg_ic_prepare;
	di->debug_ops.certification = battct_dbg_ic_certification;
	di->debug_ops.get_batt_safe_info = battct_dbg_get_batt_safe_info;
}

static ssize_t battct_dbg_switch_show(void *dev, char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;

	if (!di || !buf) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	return scnprintf(buf, size, "debug_on staus: %s\n", di->debug_on ? "ON" : "OFF");
}

static ssize_t battct_dbg_switch_store(void *dev, const char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;

	if (!di || !buf) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	if (sysfs_streq(buf, "on"))
		di->debug_on = true;
	else if (sysfs_streq(buf, "off"))
		di->debug_on = false;
	else
		hwlog_err("%s illegal\n", __func__);
	return size;
}

static ssize_t battct_dbg_uid_show(void *dev, char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;
	char uid_str[BATTCT_DBG_UID_LEN + 1] = { 0 };

	if (!di || !buf) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	if (!di->ic_info.uid_set)
		return scnprintf(buf, size, "Debug uid: NOT SET\n");

	(void)memcpy_s(uid_str, BATTCT_DBG_UID_LEN, di->ic_info.uid, BATTCT_DBG_UID_LEN);
	return scnprintf(buf, size, "Debug uid: %s\n", uid_str);
}

static ssize_t battct_dbg_uid_store(void *dev, const char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;

	if (!di || !buf || (strlen(buf) > BATTCT_DBG_UID_LEN)) {
		hwlog_err("%s: di null\n", __func__);
		return size;
	}

	memset(di->ic_info.uid, 0, BATTCT_DBG_UID_LEN);
	if (!memcpy_s(di->ic_info.uid, BATTCT_DBG_UID_LEN, buf, strlen(buf)))
		di->ic_info.uid_set = true;

	return size;
}

static ssize_t battct_dbg_sn_show(void *dev, char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;
	char sn_str[BATTCT_DBG_SN_ASC_LEN + 1] = { 0 };

	if (!di || !buf) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	if (!di->ic_info.sn_set)
		return scnprintf(buf, size, "Debug sn: NOT SET\n");

	(void)memcpy_s(sn_str, BATTCT_DBG_SN_ASC_LEN + 1, di->ic_info.sn, BATTCT_DBG_SN_ASC_LEN);
	return scnprintf(buf, size, "Debug sn: %s\n", sn_str);
}

static ssize_t battct_dbg_sn_store(void *dev, const char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;

	if (!di || !buf || (strlen(buf) > BATTCT_DBG_SN_ASC_LEN)) {
		hwlog_err("%s: di null\n", __func__);
		return size;
	}

	memset(di->ic_info.sn, 0, BATTCT_DBG_SN_ASC_LEN);
	if (!memcpy_s(di->ic_info.sn, BATTCT_DBG_SN_ASC_LEN, buf, strlen(buf)))
		di->ic_info.sn_set = true;

	return size;
}

static ssize_t battct_dbg_group_sn_show(void *dev, char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;
	char group_sn_str[BATTCT_DBG_SN_ASC_LEN + 1] = { 0 };

	if (!di || !buf) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	if (!di->ic_info.group_sn_set)
		return scnprintf(buf, size, "Debug group_sn: NOT SET\n");

	(void)memcpy_s(group_sn_str, BATTCT_DBG_SN_ASC_LEN, di->ic_info.group_sn, BATTCT_DBG_SN_ASC_LEN);
	return scnprintf(buf, size, "Debug group_sn: %s\n", group_sn_str);
}

/*
 * Write group_sn for DEBUG
 * Format: "echo xxxxx>debug_group_sn"
 */
static ssize_t battct_dbg_group_sn_store(void *dev, const char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;

	if (!di || !buf || (strlen(buf) > BATTCT_DBG_SN_ASC_LEN)) {
		hwlog_err("%s: di null\n", __func__);
		return size;
	}

	memset(di->ic_info.group_sn, 0, BATTCT_DBG_SN_ASC_LEN);
	if (!memcpy_s(di->ic_info.group_sn, BATTCT_DBG_SN_ASC_LEN, buf, strlen(buf)))
		di->ic_info.group_sn_set = true;

	return size;
}

/*
 * Display debug org_spar
 * Format: "cat debug_org_spar"
 */
static ssize_t battct_dbg_org_spar_show(void *dev, char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;

	if (!di || !buf) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	if (!di->ic_info.source_set)
		return scnprintf(buf, size, "debug batt source: NOT SET\n");

	return scnprintf(buf, size, "debug batt source: %d\n", di->ic_info.source);
}

/*
 * Write org_spar for DEBUG
 * Options: org - BATTERY_ORIGINAL, spar - BATTERY_SPARE_PART
 * Format: "echo org>debug_org_spar"
 */
static ssize_t battct_dbg_org_spar_store(void *dev, const char *buf, size_t size)
{
	enum batt_source source;
	struct battct_dbg_dev *di = dev;

	if (!di || !buf) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	if (!sysfs_streq(buf, "org")) {
		source = BATTERY_ORIGINAL;
	} else if (!sysfs_streq(buf, "spar")) {
		source = BATTERY_SPARE_PART;
	} else {
		hwlog_err("org_spar input illegal\n");
		return size;
	}

	di->ic_info.source = source;
	di->ic_info.source_set = true;
	return size;
}

static ssize_t battct_dbg_swi_show(void *dev, char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;

	if (!di || !buf) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	return scnprintf(buf, size, "Debug swi: %s\n", di->swi_val);
}

static ssize_t battct_dbg_swi_store(void *dev, const char *buf, size_t size)
{
	struct battct_dbg_dev *di = dev;

	if (!di || !buf || (strlen(buf) >= BATTCT_DBG_SWI_STR_LEN)) {
		hwlog_err("%s: di null\n", __func__);
		return size;
	}

	memset(di->swi_val, 0, BATTCT_DBG_SWI_STR_LEN);
	memcpy_s(di->swi_val, BATTCT_DBG_SWI_STR_LEN, buf, strlen(buf));
	return size;
}

static void battct_register_power_debug(struct battct_dbg_dev *di)
{
	power_dbg_ops_register("battct", "debug_on", di,
		battct_dbg_switch_show, battct_dbg_switch_store);
	power_dbg_ops_register("battct", "uid", di,
		battct_dbg_uid_show, battct_dbg_uid_store);
	power_dbg_ops_register("battct", "sn", di,
		battct_dbg_sn_show, battct_dbg_sn_store);
	power_dbg_ops_register("battct", "group_sn", di,
		battct_dbg_group_sn_show, battct_dbg_group_sn_store);
	power_dbg_ops_register("battct", "batt_source", di,
		battct_dbg_org_spar_show, battct_dbg_org_spar_store);
	power_dbg_ops_register("battct", "swi_val", di,
		battct_dbg_swi_show, battct_dbg_swi_store);
}

static int hw_battct_debug_probe(struct platform_device *pdev)
{
	struct battct_dbg_dev *di = NULL;

	di = kzalloc(sizeof(struct battct_dbg_dev), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_battct_dbg_dev = di;
	platform_set_drvdata(pdev, di);
	battct_dbg_ic_ops_register(di);
	battct_register_power_debug(di);
	return 0;
}

static int hw_battct_debug_remove(struct platform_device *pdev)
{
	struct battct_dbg_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	kfree(di);
	g_battct_dbg_dev = NULL;
	return 0;
}

static const struct of_device_id hw_battct_debug_match_table[] = {
	{ .compatible = "huawei,battery_ct_debug", },
	{},
};

static struct platform_driver hw_battct_debug_driver = {
	.probe = hw_battct_debug_probe,
	.remove = hw_battct_debug_remove,
	.driver = {
		.name = "huawei,battery_ct_debug",
		.owner = THIS_MODULE,
		.of_match_table = hw_battct_debug_match_table,
	},
};

int __init hw_battct_debug_init(void)
{
	return platform_driver_register(&hw_battct_debug_driver);
}

void __exit hw_battct_debug_exit(void)
{
	platform_driver_unregister(&hw_battct_debug_driver);
}

subsys_initcall_sync(hw_battct_debug_init);
module_exit(hw_battct_debug_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei battery certification debug driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");