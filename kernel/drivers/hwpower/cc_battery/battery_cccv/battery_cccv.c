// SPDX-License-Identifier: GPL-2.0
/*
 * battery_cccv.c
 *
 * battery cccv driver
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <securec.h>
#include <chipset_common/hwpower/battery/battery_cccv.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG battery_cccv
HWLOG_REGIST();

struct bat_cccv {
	int vbat;
	int ichg;
	int time;
};

struct bat_cccv_tbat {
	int tbat_th;
	int stage_size;
	struct bat_cccv cccv[BAT_CCCV_MAX_STAGE];
};

struct bat_cccv_dev {
	struct device *dev;
	struct notifier_block plugged_nb;
	struct notifier_block chg_nb;
	struct notifier_block wlc_nb;
	struct bat_cccv_tbat tbat_cccv_tbl[BAT_CCCV_MAX_TEMP_NUM];
	int tbat_cccv_tbl_group_size;
	struct bat_cccv *cccv_with_ratio_tbl;
	int cccv_with_ratio_tbl_size;
	u32 first_start_time;
	int last_vbat;
};

static struct bat_cccv_dev *g_bat_cccv_dev;

static int bat_cccv_event_cb(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct bat_cccv_dev *di = g_bat_cccv_dev;

	if (!di)
		return -ENODEV;

	switch (action) {
	case POWER_NE_USB_DISCONNECT:
	case POWER_NE_WIRELESS_DISCONNECT:
		if (di->cccv_with_ratio_tbl)
			kfree(di->cccv_with_ratio_tbl);
		di->cccv_with_ratio_tbl = NULL;
		di->first_start_time = 0;
		break;
	case POWER_NE_DC_CHECK_SUCC:
	case POWER_NE_WLC_DC_START_CHARGING:
		if (!di->first_start_time) {
			di->first_start_time = (u32)power_get_current_kernel_time().tv_sec;
			di->last_vbat = 0;
		}
		break;
	default:
		break;
	}
	return 0;
}

static int bat_cccv_get_ichg_with_ratio(struct bat_cccv_dev *di, int vbat, u32 time_diff)
{
	int i;
	int length;
	int hysteresis = 0;

	if (!di || !di->cccv_with_ratio_tbl)
		return -ENODEV;

	if (vbat < di->last_vbat)
		hysteresis = -BAT_CCCV_TMP_HYSTERESIS;

	di->last_vbat = vbat;
	for (i = 0; i < di->cccv_with_ratio_tbl_size; i++) {
		if (vbat > (di->cccv_with_ratio_tbl[i].vbat + hysteresis))
			continue;
		if ((di->cccv_with_ratio_tbl[i].time > 0) && (time_diff > di->cccv_with_ratio_tbl[i].time))
			continue;
		return di->cccv_with_ratio_tbl[i].ichg;
	}
	return di->cccv_with_ratio_tbl[di->cccv_with_ratio_tbl_size - 1].ichg;
}

int bat_cccv_get_ichg(int vbat)
{
	struct bat_cccv_dev *di = g_bat_cccv_dev;
	u32 time_diff = 0;

	if (!di)
		return -ENODEV;

	time_diff = (u32)power_get_current_kernel_time().tv_sec - di->first_start_time;
	return bat_cccv_get_ichg_with_ratio(di, vbat, time_diff);
}

static struct bat_cccv_tbat *bat_cccv_select_tbat_cccv(struct bat_cccv_dev *di)
{
	int i;
	int tbatt = 0;

	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_TEMP, &tbatt,
		POWER_SUPPLY_DEFAULT_TEMP / POWER_SUPPLY_BAT_TEMP_UNIT,
		POWER_SUPPLY_BAT_TEMP_UNIT);
	for (i = 0; i < di->tbat_cccv_tbl_group_size; i++) {
		if (tbatt < di->tbat_cccv_tbl[i].tbat_th)
			return &di->tbat_cccv_tbl[i];
	}
	return NULL;
}

static int bat_cccv_parse_str_ichg_ratio(int *ichg_ratio_tbl, int len, char *buf)
{
	int i;
	int stage = 0;
	int ratio = 0;
	int total_ratio = BAT_CCCV_RATIO_DEFAULT;
	char *tmp1 = NULL;
	char *tmp2 = NULL;

	/* The correct string format is "0@100,1@95" */
	tmp1 = strsep(&buf, ",");
	while (tmp1) {
		tmp2 = strsep(&tmp1, "@");
		if (!tmp2 || !tmp1) {
			hwlog_info("%s tmp1 or tmp2 is null\n", __func__);
			return -EINVAL;
		}

		if (kstrtoint(tmp2, POWER_BASE_DEC, &stage) || stage > len) {
			hwlog_err("%s kstrtoint stage failed\n", __func__);
			return -EINVAL;
		}
		if (kstrtoint(tmp1, POWER_BASE_DEC, &ratio) || ratio < BAT_CCCV_RATIO_MIN) {
			hwlog_err("%s ratio kstrtoint failed\n", __func__);
			return -EINVAL;
		}
		if (stage == 0)
			total_ratio = ratio;
		else
			ichg_ratio_tbl[stage - 1] = ratio;

		if (!buf)
			break;
		tmp1 = strsep(&buf, ",");
	}
	for (i = 0; i < len; i++)
		ichg_ratio_tbl[i] = power_min_positive(total_ratio, ichg_ratio_tbl[i]);
	return 0;
}

static int bat_cccv_update_ratio_ichg(struct bat_cccv_dev *di,
	int *ichg_ratio_tbl, struct bat_cccv_tbat *selected_tbat_cccv)
{
	int i;
	int stage_size;
	struct bat_cccv *local_cccv = NULL;
	int pre_cur = 0;

	stage_size = selected_tbat_cccv->stage_size;
	local_cccv = kzalloc(sizeof(struct bat_cccv) * stage_size, GFP_KERNEL);
	if (!local_cccv)
		return -ENOMEM;

	for (i = 0; i < stage_size; i++) {
		local_cccv[i].vbat = selected_tbat_cccv->cccv[i].vbat;
		local_cccv[i].ichg =
			selected_tbat_cccv->cccv[i].ichg * ichg_ratio_tbl[i] / BAT_CCCV_RATIO_UTIL;
		local_cccv[i].time = selected_tbat_cccv->cccv[i].time;
		local_cccv[i].ichg = power_min_positive(pre_cur, local_cccv[i].ichg);
		pre_cur = local_cccv[i].ichg;
	}
	if (di->cccv_with_ratio_tbl)
		kfree(di->cccv_with_ratio_tbl);

	di->cccv_with_ratio_tbl = local_cccv;
	di->cccv_with_ratio_tbl_size = stage_size;
	return 0;
}

static int bat_cccv_handle_ichg_ratio(struct bat_cccv_dev *di, char *buf)
{
	int ret;
	int stage_count;
	int *ichg_ratio_tbl = NULL;
	struct bat_cccv_tbat *selected_tbat_cccv = NULL;

	/* get tbat and stage count */
	selected_tbat_cccv = bat_cccv_select_tbat_cccv(di);
	if (!selected_tbat_cccv) {
		hwlog_err("%s selected_tbat_cccv is null\n", __func__);
		return -EINVAL;
	}

	stage_count = selected_tbat_cccv->stage_size;
	if ((stage_count < 0) || (stage_count >= BAT_CCCV_MAX_STAGE))
		return -EINVAL;

	ichg_ratio_tbl = kzalloc(sizeof(int) * stage_count, GFP_KERNEL);
	if (!ichg_ratio_tbl)
		return -ENOMEM;

	memset_s(ichg_ratio_tbl, sizeof(int) * stage_count, 0, sizeof(int) * stage_count);
	/* parse buf and get ichg ratio */
	ret = bat_cccv_parse_str_ichg_ratio(ichg_ratio_tbl, stage_count, buf);
	if (ret)
		goto parse_ratio_end;
	/* get cccv with ratio */
	ret = bat_cccv_update_ratio_ichg(di, ichg_ratio_tbl, selected_tbat_cccv);
parse_ratio_end:
	kfree(ichg_ratio_tbl);
	return ret;
}

#ifdef CONFIG_SYSFS
static ssize_t bat_cccv_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t bat_cccv_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info bat_cccv_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(bat_cccv, 0220,
		BAT_CCCV_SYSFS_UPDATE_ICHG_RATIO, update_ichg_ratio),
};

#define BAT_CCCV_SYSFS_ATTRS_SIZE ARRAY_SIZE(bat_cccv_sysfs_field_tbl)

static struct attribute *bat_cccv_sysfs_attrs[BAT_CCCV_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group bat_cccv_sysfs_attr_group = {
	.attrs = bat_cccv_sysfs_attrs,
};

static ssize_t bat_cccv_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		bat_cccv_sysfs_field_tbl, BAT_CCCV_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	default:
		return 0;
	}
}

static ssize_t bat_cccv_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	char saved_buf[BAT_CCCV_BUF_MAX_SIZE] = { 0 };
	struct bat_cccv_dev *di = dev_get_drvdata(dev);

	if (!di)
		return -ENODEV;

	info = power_sysfs_lookup_attr(attr->attr.name,
		bat_cccv_sysfs_field_tbl, BAT_CCCV_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case BAT_CCCV_SYSFS_UPDATE_ICHG_RATIO:
		if (snprintf_s(saved_buf, BAT_CCCV_BUF_MAX_SIZE,
			BAT_CCCV_BUF_MAX_SIZE - 1, "%s", buf) < 0)
			return -EINVAL;

		hwlog_info("%s origin buf=%s, saved buf=%s\n", __func__, buf, saved_buf);
		if (bat_cccv_handle_ichg_ratio(di, saved_buf))
			return -EINVAL;
		break;
	default:
		break;
	}

	return count;
}

static void bat_cccv_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(bat_cccv_sysfs_attrs,
		bat_cccv_sysfs_field_tbl, BAT_CCCV_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "battery", "battery_cccv",
		dev, &bat_cccv_sysfs_attr_group);
}

static void bat_cccv_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "battery", "battery_cccv",
		dev, &bat_cccv_sysfs_attr_group);
}
#else
static inline void bat_cccv_sysfs_create_group(struct device *dev)
{
}

static inline void bat_cccv_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int bat_cccv_parse_cccv(struct device_node *node,
	const char *tab_name, struct bat_cccv_tbat *cccv_tbat)
{
	int i;
	int len;
	int row;
	int col;
	u32 data;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), node,
		tab_name, BAT_CCCV_MAX_STAGE, BAT_CCCV_TBL_END);
	if (len <= 0) {
		hwlog_err("%s cccv_param_dts get failed\n", __func__);
		return -EINVAL;
	}

	cccv_tbat->stage_size = len / BAT_CCCV_TBL_END;
	for (i = 0; i < len; i++) {
		if (power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
			node, tab_name, i, &data))
			return -EINVAL;

		row = i / BAT_CCCV_TBL_END;
		col = i % BAT_CCCV_TBL_END;
		switch (col) {
		case BAT_CCCV_TBL_VBAT:
			cccv_tbat->cccv[row].vbat = (int)data;
			break;
		case BAT_CCCV_TBL_ICHG:
			cccv_tbat->cccv[row].ichg = (int)data;
			break;
		case BAT_CCCV_TBL_TIME:
			cccv_tbat->cccv[row].time = (int)data;
			hwlog_info("%s %s:%s[%d]: %dmV %dmA %us\n", __func__,
				node->name, tab_name, row, cccv_tbat->cccv[row].vbat,
				cccv_tbat->cccv[row].ichg, cccv_tbat->cccv[row].time);
			break;
		default:
			break;
		}
	}
	return 0;
}

static int bat_cccv_parse_tbat_dts(struct device_node *node, struct bat_cccv_dev *di,
	const char *select_param)
{
	int i;
	int len;
	int idata = 0;
	const char *str = NULL;
	struct device_node *sub_node = NULL;

	sub_node = of_find_node_by_name(node, select_param);
	if (!sub_node) {
		hwlog_err("sub_node find failed\n");
		return -EINVAL;
	}

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), sub_node,
		"temp_tab", BAT_CCCV_MAX_TEMP_NUM, BAT_CCCV_TMP_TBL_END);
	if (len <= 0) {
		hwlog_err("temp_tab get failed\n");
		return -EINVAL;
	}

	di->tbat_cccv_tbl_group_size = len / BAT_CCCV_TMP_TBL_END;
	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), sub_node, "temp_tab", i, &str))
			return -EINVAL;

		switch (i % BAT_CCCV_TMP_TBL_END) {
		case BAT_CCCV_TMP_TBL_TBAT:
			if (kstrtoint(str, POWER_BASE_DEC, &idata))
				return -EINVAL;
			di->tbat_cccv_tbl[i / BAT_CCCV_TMP_TBL_END].tbat_th = idata;
			break;
		case BAT_CCCV_TMP_TBL_CCCV:
			if (bat_cccv_parse_cccv(sub_node, str, &(di->tbat_cccv_tbl[i / BAT_CCCV_TMP_TBL_END])))
				return -EINVAL;
			break;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

static int bat_cccv_match_brand(struct device_node *node, const char *select_param)
{
	int i;
	int len;
	const char *string = NULL;
	const char *batt_brand = "default";

	(void)power_supply_get_str_property_value(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_BRAND, &batt_brand);
	if (!batt_brand)
		return -EINVAL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), node,
		"battery_tbl", BAT_CCCV_TBL_LEN, BAT_CCCV_BAT_TBL_END);
	if (len <= 0) {
		hwlog_err("battery_tbl get failed\n");
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		switch (i % BAT_CCCV_BAT_TBL_END) {
		case BAT_CCCV_BAT_TBL_BRAND:
			if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
				node, "battery_tbl", i, &string)) {
				hwlog_err("get battery_tbl fail\n");
				return -EINVAL;
			}
			if (strcmp(string, batt_brand))
				i++;
			break;
		case BAT_CCCV_BAT_TBL_PARAM:
			return power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
				node, "battery_tbl", i, &select_param);
		default:
			break;
		}
	}
	return -EINVAL;
}

static int bat_cccv_parse_dts(struct device_node *node, struct bat_cccv_dev *di)
{
	const char *select_param = "cccv_para0";

	if (bat_cccv_match_brand(node, select_param))
		hwlog_err("%s batt_cccv_match_brand fail\n", __func__);

	return bat_cccv_parse_tbat_dts(node, di, select_param);
}

static int bat_cccv_probe(struct platform_device *pdev)
{
	int ret;
	struct bat_cccv_dev *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;

	ret = bat_cccv_parse_dts(np, di);
	if (ret) {
		hwlog_err("%s bat_cccv_parse_dts failed\n", __func__);
		goto fail_free_mem;
	}

	di->plugged_nb.notifier_call = bat_cccv_event_cb;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &di->plugged_nb);
	if (ret < 0) {
		hwlog_err("%s register POWER_BNT_CONNECT failed\n", __func__);
		goto fail_free_mem;
	}

	di->chg_nb.notifier_call = bat_cccv_event_cb;
	ret = power_event_bnc_register(POWER_BNT_DC, &di->chg_nb);
	if (ret < 0) {
		hwlog_err("%s register POWER_BNT_DC failed\n", __func__);
		goto fail_regist_chg_nb;
	}

	di->wlc_nb.notifier_call = bat_cccv_event_cb;
	ret = power_event_bnc_register(POWER_BNT_WLC, &di->wlc_nb);
	if (ret < 0) {
		hwlog_err("%s register POWER_BNT_WLC failed\n", __func__);
		goto fail_regist_wlc_nb;
	}
	bat_cccv_sysfs_create_group(di->dev);
	platform_set_drvdata(pdev, di);
	g_bat_cccv_dev = di;
	return 0;

fail_regist_wlc_nb:
	(void)power_event_bnc_unregister(POWER_BNT_DC, &di->chg_nb);
fail_regist_chg_nb:
	(void)power_event_bnc_unregister(POWER_BNT_CONNECT, &di->plugged_nb);
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	platform_set_drvdata(pdev, NULL);
	g_bat_cccv_dev = NULL;
	return ret;
}

static int bat_cccv_remove(struct platform_device *pdev)
{
	struct bat_cccv_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	(void)power_event_bnc_unregister(POWER_BNT_CONNECT, &di->plugged_nb);
	(void)power_event_bnc_unregister(POWER_BNT_DC, &di->chg_nb);
	(void)power_event_bnc_unregister(POWER_BNT_WLC, &di->wlc_nb);
	bat_cccv_sysfs_remove_group(di->dev);
	if (di->cccv_with_ratio_tbl)
		kfree(di->cccv_with_ratio_tbl);

	di->cccv_with_ratio_tbl = NULL;
	devm_kfree(&pdev->dev, di);
	platform_set_drvdata(pdev, NULL);
	g_bat_cccv_dev = NULL;
	return 0;
}

static const struct of_device_id bat_cccv_match_table[] = {
	{
		.compatible = "huawei,battery_cccv",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_cccv_driver = {
	.probe = bat_cccv_probe,
	.remove = bat_cccv_remove,
	.driver = {
		.name = "huawei,battery_cccv",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_cccv_match_table),
	},
};

static int __init battery_cccv_init(void)
{
	return platform_driver_register(&bat_cccv_driver);
}

static void __exit battery_cccv_exit(void)
{
	platform_driver_unregister(&bat_cccv_driver);
}

late_initcall_sync(battery_cccv_init);
module_exit(battery_cccv_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery cccv driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");