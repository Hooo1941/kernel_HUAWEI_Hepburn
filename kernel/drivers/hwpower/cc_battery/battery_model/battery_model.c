// SPDX-License-Identifier: GPL-2.0
/*
 * battery_model.c
 *
 * huawei battery model information
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "battery_model.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <securec.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/batt_info_pub.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>

#define HWLOG_TAG battery_model
HWLOG_REGIST();

static struct bat_model_device *g_bat_model_dev;
static const char * const g_bat_model_bat_type[] = {
	[BAT_MODEL_BAT_CATHODE_TYPE_INVALID] = "invalid",
	[BAT_MODEL_BAT_CATHODE_TYPE_GRAPHITE] = "graphite cathode",
	[BAT_MODEL_BAT_CATHODE_TYPE_SILICON] = "silicon cathode",
};

int bat_model_register_ops(struct bat_model_ops *ops)
{
	if (!g_bat_model_dev || !ops) {
		hwlog_err("g_bat_model_dev or ops is null\n");
		return -EPERM;
	}

	g_bat_model_dev->ops = ops;
	return 0;
}

int bat_model_get_vbat_max(void)
{
	struct bat_model_device *di = g_bat_model_dev;

	if (!di)
		return POWER_SUPPLY_DEFAULT_VOLTAGE_MAX;

	if (!di->ops || !di->ops->get_vbat_max) {
		hwlog_info("default vbat_max=%d\n", di->vbat_max);
		return di->vbat_max;
	}
	return di->ops->get_vbat_max(di->ops->dev_data);
}

int bat_model_get_design_fcc(void)
{
	struct bat_model_device *di = g_bat_model_dev;

	if (!di)
		return POWER_SUPPLY_DEFAULT_CAPACITY_FCC;

	if (!di->ops || !di->ops->get_design_fcc)
		return (di->design_fcc != BAT_MODEL_INVALID_FCC) ?
			di->design_fcc : di->fcc;

	return di->ops->get_design_fcc(di->ops->dev_data);
}

int bat_model_get_technology(void)
{
	struct bat_model_device *di = g_bat_model_dev;

	if (!di)
		return POWER_SUPPLY_DEFAULT_TECHNOLOGY;

	if (!di->ops || !di->ops->get_technology)
		return di->technology;

	return di->ops->get_technology(di->ops->dev_data);
}

int bat_model_is_removed(void)
{
	struct bat_model_device *di = g_bat_model_dev;

	if (!di || !di->ops || !di->ops->is_removed)
		return 0;

	return di->ops->is_removed(di->ops->dev_data);
}

const char *bat_model_get_brand(void)
{
	struct bat_model_device *di = g_bat_model_dev;

	if (!di)
		return POWER_SUPPLY_DEFAULT_BRAND;

	if (!di->ops || !di->ops->get_brand) {
		hwlog_info("battery brand is %s\n", di->brand);
		return di->brand;
	}
	return di->ops->get_brand(di->ops->dev_data);
}

static void bat_model_report_dsm(int id_vol)
{
	unsigned char type[BAT_MODEL_TYPE_LEN] = { 0 };
	char buf[BAT_MODEL_DSM_CONTENT_SIZE] = { 0 };

	if (power_cmdline_is_factory_mode())
		return;

	(void)get_battery_type(type, BAT_MODEL_TYPE_LEN);
	snprintf(buf, sizeof(buf) - 1, "batt type=%s, id_vol=%d", type, id_vol);
	power_dsm_report_dmd(POWER_DSM_BATTERY_DETECT,
		POWER_DSM_BATTERY_DETECT_ERROR_NO, buf);
}

static void bat_model_set_brand(struct bat_model_device *di, int index)
{
	char *token = NULL;
	char model_name[BAT_MODEL_NAME_LEN] = { 0 };
	char *name_ptr = model_name;

	di->id_index = index;
	strlcpy(model_name, di->tables[index].name, sizeof(model_name));
	token = strsep(&name_ptr, "_");
	if (!token)
		return;
	strlcpy(di->brand, token, sizeof(di->brand));

	if (!name_ptr)
		goto ret;
	token = strsep(&name_ptr, "_");
	if (!token || kstrtoint(token, 0, &di->fcc))
		goto ret;

	token = strsep(&name_ptr, "_");
	if (token)
		kstrtoint(token, 0, &di->vbat_max);

ret:
	hwlog_info("index=%d,fcc=%d,vbat_max=%d,brand=%s\n",
		index, di->fcc, di->vbat_max, di->brand);
}

static bool bat_model_identifier_match(const char *identifier_pattern)
{
	int ret;
	struct bat_model_device *di = g_bat_model_dev;
	char bat_identifier_codes[BAT_IDENTIFIER_CODES_MAX_LEN] = { 0 };
	char *token = NULL;
	char *codes_ptr = bat_identifier_codes;

	if (!di || !identifier_pattern)
		return false;

	ret = get_battery_identifier(bat_identifier_codes, BAT_IDENTIFIER_CODES_MAX_LEN,
		di->bat_identifier_index, di->bat_identifier_len);
	if (ret || (strlen(bat_identifier_codes) == 0)) {
		hwlog_err("get_battery_identifier_codes failed ret:%d\n", ret);
		return false;
	}

	while ((token = strsep(&codes_ptr, "\n"))) {
		if (strlen(token) == 0)
			continue;

		hwlog_info("identifier_pattern:%s, identifier:%s\n", identifier_pattern, token);
		if (power_regex_lite_is_matched(identifier_pattern, token))
			return true;
	}

	return false;
}

static bool bat_model_match_battery_type(const char *bat_type)
{
	int i;
	struct bat_model_device *di = g_bat_model_dev;
	const char *bat_identifier = NULL;

	if (!di)
		return false;

	for (i = 0; i < di->bat_types_num; i++) {
		if (strcmp(di->bat_types_tab[i].types, bat_type) == 0) {
			bat_identifier = di->bat_types_tab[i].identifier_pattern;
			break;
		}
	}
	if (i == di->bat_types_num)
		return false;

	if (bat_model_identifier_match(bat_identifier)) {
		hwlog_info("bat is matched with battery type %s\n", bat_type);
		return true;
	}

	return false;
}

bool bat_model_match_silicon_battery(void)
{
	return bat_model_match_battery_type("silicon");
}

bool bat_model_match_graphite_battery(void)
{
	return bat_model_match_battery_type("graphite");
}

int bat_model_get_bat_cathode_type(void)
{
	struct bat_model_device *di = g_bat_model_dev;

	if (!di)
		return BAT_MODEL_BAT_CATHODE_TYPE_INVALID;

	if (!di->bat_cathode_ext)
		return BAT_MODEL_BAT_CATHODE_TYPE_GRAPHITE;

	if (bat_model_match_silicon_battery())
		return BAT_MODEL_BAT_CATHODE_TYPE_SILICON;

	return BAT_MODEL_BAT_CATHODE_TYPE_GRAPHITE;
}
EXPORT_SYMBOL_GPL(bat_model_get_bat_cathode_type);

static int bat_model_get_index_by_type(struct bat_model_device *di)
{
	int i;
	int type_len;
	unsigned char type[BAT_MODEL_TYPE_LEN] = { 0 };

	if (get_battery_type(type, BAT_MODEL_TYPE_LEN)) {
		hwlog_err("get battery type failed\n");
		return -EINVAL;
	}

	type_len = strlen(type);
	if (type_len == 0) {
		hwlog_err("battery type len error\n");
		return -EINVAL;
	}

	hwlog_info("battery type is %s len=%d\n", type, type_len);
	if (di->bat_cathode_ext && (bat_model_get_bat_cathode_type() ==
		BAT_MODEL_BAT_CATHODE_TYPE_SILICON)) {
		strcat(type, "S"); /* S represents silicon */
		type_len += 1;
		hwlog_info("battery type is %s len=%d\n", type, type_len);
	}

	for (i = 0; i < di->table_size; i++) {
		if (!strncmp(di->tables[i].sn_type, type, type_len)) {
			bat_model_set_brand(di, i);
			return 0;
		}
	}
	return -EINVAL;
}

static int bat_model_get_index_by_iio(struct bat_model_device *di,
	int *id_volt)
{
	int retry = 30; /* wait for iio channel ready */
	int ret;

	if (!di->id_iio && di->is_iio_init)
		return -ENODEV;

	if (!di->id_iio) {
		do {
			di->id_iio = iio_channel_get(di->dev, "batt-id");
			if (IS_ERR(di->id_iio)) {
				hwlog_err("batt-id channel not ready:%d\n",
					retry);
				di->id_iio = NULL;
				/* 100: wait for iio channel ready */
				msleep(100);
			} else {
				hwlog_info("batt-id channel is ready\n");
				break;
			}
		} while (retry-- > 0);
		di->is_iio_init = true;
	}

	if (!di->id_iio) {
		hwlog_err("batt-id channel not exist\n");
		return -ENODEV;
	}

	ret = iio_read_channel_processed(di->id_iio, id_volt);
	hwlog_info("%s ret:%d, vol:%d\n", __func__, ret, *id_volt);

	return ret;
}

static void bat_model_get_index(struct bat_model_device *di)
{
	int id_volt = 0;
	int i;

	for (i = 0; i < di->table_size; i++)
		hwlog_info("low_vol=%d, high_vol=%d, type=%s\n",
			di->tables[i].id_vol_low,
			di->tables[i].id_vol_high,
			di->tables[i].sn_type);
	for (i = 0; i < di->si_item_codes_size; i++)
		hwlog_info("si_item_code=%s\n", di->si_item_codes[i].code);

	if (!bat_model_get_index_by_type(di))
		return;

	if (di->is_qcom_fg)
		if (power_platform_get_index_by_volt(&id_volt) >= 0)
			goto search_table;

	if (bat_model_get_index_by_iio(di, &id_volt) >= 0)
		goto search_table;

	if (di->id_adc_channel == BAT_MODEL_INVALID_CHANNEL) {
		hwlog_err("id adc channel invalid\n");
		goto use_default;
	}

	id_volt = power_platform_get_adc_voltage(di->id_adc_channel);
	if (id_volt <= 0) {
		hwlog_err("id vol error %d\n", id_volt);
		goto use_default;
	}

search_table:
	hwlog_info("id vol is %d\n", id_volt);
	for (i = 0; i < di->table_size; i++) {
		if ((id_volt < di->tables[i].id_vol_low) ||
			(id_volt >= di->tables[i].id_vol_high))
			continue;

		bat_model_set_brand(di, i);
		return;
	}

use_default:
	hwlog_err("get battery id failed and use default\n");
	bat_model_set_brand(di, di->default_index);
	strlcpy(di->brand, POWER_SUPPLY_DEFAULT_BRAND, sizeof(di->brand));
	bat_model_report_dsm(id_volt);
}

int bat_model_id_index(void)
{
	struct bat_model_device *di = g_bat_model_dev;

	if (!di)
		return 0;

	if (di->id_index == BAT_MODEL_INVALID_INDEX)
		bat_model_get_index(di);

	if ((di->id_index < 0) || (di->id_index >= di->table_size))
		return 0;

	hwlog_info("bat id index=%d name=%s\n", di->id_index,
		di->tables[di->id_index].name);
	return di->id_index;
}

const char *bat_model_name(void)
{
	int index;
	struct bat_model_device *di = g_bat_model_dev;

	if (!di)
		return NULL;

	index = bat_model_id_index();
	return di->tables[index].name;
}
EXPORT_SYMBOL_GPL(bat_model_name);

static ssize_t bat_model_dbg_id_show(void *dev_data,
	char *buf, size_t size)
{
	return scnprintf(buf, size, "%d\n", bat_model_id_index());
}

static ssize_t bat_model_dbg_name_show(void *dev_data,
	char *buf, size_t size)
{
	const char *name = bat_model_name();

	return scnprintf(buf, size, "%s\n", name ? name : "null");
}

static ssize_t bat_model_dbg_brand_show(void *dev_data,
	char *buf, size_t size)
{
	return scnprintf(buf, size, "%s\n", bat_model_get_brand());
}

static ssize_t bat_model_dbg_vbat_max_show(void *dev_data,
	char *buf, size_t size)
{
	return scnprintf(buf, size, "%d\n", bat_model_get_vbat_max());
}

static ssize_t bat_model_dbg_rated_fcc_show(void *dev_data,
	char *buf, size_t size)
{
	return scnprintf(buf, size, "%d\n", bat_model_get_design_fcc());
}

static void bat_model_debug_register(struct bat_model_device *di)
{
	power_dbg_ops_register("bat_model", "id", (void *)di,
		bat_model_dbg_id_show, NULL);
	power_dbg_ops_register("bat_model", "name", (void *)di,
		bat_model_dbg_name_show, NULL);
	power_dbg_ops_register("bat_model", "brand", (void *)di,
		bat_model_dbg_brand_show, NULL);
	power_dbg_ops_register("bat_model", "vbat_max", (void *)di,
		bat_model_dbg_vbat_max_show, NULL);
	power_dbg_ops_register("bat_model", "rated_fcc", (void *)di,
		bat_model_dbg_rated_fcc_show, NULL);
}

static void bat_model_init_data(struct bat_model_device *di)
{
	di->id_adc_channel = BAT_MODEL_INVALID_CHANNEL;
	di->id_index = BAT_MODEL_INVALID_INDEX;
	di->vbat_max = POWER_SUPPLY_DEFAULT_VOLTAGE_MAX;
	di->fcc = POWER_SUPPLY_DEFAULT_CAPACITY_FCC;
	strlcpy(di->brand, POWER_SUPPLY_DEFAULT_BRAND, sizeof(di->brand));
}

static int bat_model_parse_table_int_item(struct device_node *np,
	int index, int *value)
{
	const char *tmp_string = NULL;

	if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
		"bat_id_table", index, &tmp_string))
		return -EINVAL;

	return kstrtoint(tmp_string, 0, value);
}

static int bat_model_parse_table_string_item(struct device_node *np,
	int index, char *value, int size)
{
	const char *tmp_string = NULL;

	if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
		"bat_id_table", index, &tmp_string))
		return -EINVAL;

	if ((strlen(tmp_string) + 1) > size)
		return -EINVAL;

	strlcpy(value, tmp_string, size);
	return 0;
}

static int bat_model_parse_table_row(struct bat_model_device *di, int index)
{
	int ret = 0;
	int row_index = index / BAT_MODEL_TABLE_COLS;
	struct device_node *np = di->dev->of_node;
	struct bat_model_table *table = &di->tables[row_index];

	ret += bat_model_parse_table_string_item(np, index++, table->name,
		sizeof(table->name));
	ret += bat_model_parse_table_int_item(np, index++, &table->id_vol_low);
	ret += bat_model_parse_table_int_item(np, index++, &table->id_vol_high);
	ret += bat_model_parse_table_string_item(np, index, table->sn_type,
		sizeof(table->sn_type));

	return ret;
}

static void bat_model_parse_table(struct bat_model_device *di)
{
	int i;
	int len;
	struct device_node *np = di->dev->of_node;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"bat_id_table", BAT_MODEL_TABLE_ROWS, BAT_MODEL_TABLE_COLS);
	if (len <= 0)
		return;
	di->table_size = len / BAT_MODEL_TABLE_COLS;

	for (i = 0; i < len; i += BAT_MODEL_TABLE_COLS) {
		if (bat_model_parse_table_row(di, i))
			break;
	}
}

static void bat_model_parse_si_item_code(struct bat_model_device *di)
{
	int i;
	int len;
	struct device_node *np = di->dev->of_node;
	const char *tmp_string = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np, "si_item_code",
		BAT_MODEL_SI_ITEM_CODE_ROWS, BAT_MODEL_SI_ITEM_CODE_COLS);
	if (len <= 0)
		return;
	di->si_item_codes_size = len / BAT_MODEL_SI_ITEM_CODE_COLS;

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"si_item_code", i, &tmp_string))
			return;
		if ((strlen(tmp_string) + 1) > sizeof(di->si_item_codes[i / BAT_MODEL_SI_ITEM_CODE_COLS])) {
			hwlog_err("si_item_code is too long\n");
			return;
		}
		if ((i % BAT_MODEL_SI_ITEM_CODE_COLS) == 0)
			strlcpy(di->si_item_codes[i / BAT_MODEL_SI_ITEM_CODE_COLS].code,
				tmp_string, sizeof(di->si_item_codes[i / BAT_MODEL_SI_ITEM_CODE_COLS]));
	}
}

static void bat_model_parse_bat_structure_types_tab(struct bat_model_device *di)
{
	int array_len, i, row, col, ret;
	const char *str = NULL;
	struct device_node *np = di->dev->of_node;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"bat_types_pattern", BAT_TYPES_MAX_NUM, BAT_TYPES_INFO_TOTAL);
	if (array_len <= 0)
		return;

	di->bat_types_num = array_len / BAT_TYPES_INFO_TOTAL;
	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"bat_types_pattern", i, &str)) {
			di->bat_types_num = 0;
			return;
		}

		row = i / BAT_TYPES_INFO_TOTAL;
		col = i % BAT_TYPES_INFO_TOTAL;
		switch (col) {
		case BAT_TYPES_INFO_TYPES:
			if (strlen(str) >= BAT_TYPES_LEN) {
				hwlog_err("invalid bat types\n");
				di->bat_types_num = 0;
				return;
			}
			ret = strncpy_s(di->bat_types_tab[row].types, BAT_TYPES_LEN,
				str, strlen(str));
			if (ret != EOK) {
				di->bat_types_num = 0;
				return;
			}
			break;
		case BAT_TYPES_INFO_IDENTIFIER_PATTERN:
			if (strlen(str) >= BAT_IDENTIFIER_PATTERN_LEN) {
				hwlog_err("invalid bat identifier pattern\n");
				di->bat_types_num = 0;
				return;
			}
			ret = strncpy_s(di->bat_types_tab[row].identifier_pattern, BAT_IDENTIFIER_PATTERN_LEN,
				str, strlen(str));
			if (ret != EOK) {
				di->bat_types_num = 0;
				return;
			}
			break;
		default:
			break;
		}
	}
}

static void bat_model_parse_dts(struct bat_model_device *di)
{
	struct device_node *np = di->dev->of_node;

	if (!np)
		return;

	bat_model_parse_si_item_code(di);
	bat_model_parse_table(di);
	bat_model_parse_bat_structure_types_tab(di);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bat_cathode_ext", &di->bat_cathode_ext, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bat_num", &di->bat_num, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"design_fcc", &di->design_fcc,
		BAT_MODEL_INVALID_FCC);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"id_adc_channel", &di->id_adc_channel,
		BAT_MODEL_INVALID_CHANNEL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"default_id_index", &di->default_index,
		BAT_MODEL_DEFAULT_INDEX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"technology", &di->technology,
		POWER_SUPPLY_DEFAULT_TECHNOLOGY);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bat_identifier_index", &di->bat_identifier_index,
		BAT_IDENTIFIER_INDEX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bat_identifier_len", &di->bat_identifier_len,
		BAT_IDENTIFIER_LEN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_qcom_fg", &di->is_qcom_fg, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_iio", &di->support_iio, 0);
	if (!di->support_iio) {
		di->id_iio = NULL;
		di->is_iio_init = true;
		hwlog_info("not use iio\n");
	} else {
		di->id_iio = iio_channel_get(di->dev, "batt-id");
		if (IS_ERR(di->id_iio)) {
			hwlog_err("batt-id channel not ready in probe\n");
			di->id_iio = NULL;
			di->is_iio_init = false;
		} else {
			di->is_iio_init = true;
		}
	}
	if ((di->default_index < 0) || (di->default_index >= di->table_size))
		di->default_index = BAT_MODEL_DEFAULT_INDEX;
}

#ifdef CONFIG_SYSFS
static ssize_t bat_model_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct power_sysfs_attr_info bat_model_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(bat_model, 0440, BAT_MODEL_SYSFS_BAT_CATHODE_TYPE_DETECT,
		bat_cathode_type),
};

#define BAT_MODEL_SYSFS_ATTRS_SIZE  ARRAY_SIZE(bat_model_sysfs_field_tbl)

static struct attribute *bat_model_sysfs_attrs[BAT_MODEL_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group bat_model_sysfs_attr_group = {
	.attrs = bat_model_sysfs_attrs,
};

static ssize_t bat_model_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct bat_model_device *di = dev_get_drvdata(dev);
	int len;
	int idx = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		bat_model_sysfs_field_tbl, BAT_MODEL_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case BAT_MODEL_SYSFS_BAT_CATHODE_TYPE_DETECT:
		idx = bat_model_get_bat_cathode_type();
		len = snprintf(buf, PAGE_SIZE, "%d\n", idx);
		break;
	default:
		len = 0;
		break;
	}

	return len;
}

static void bat_model_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(bat_model_sysfs_attrs,
		bat_model_sysfs_field_tbl, BAT_MODEL_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "battery", "battery_model",
		dev, &bat_model_sysfs_attr_group);
}

static void bat_model_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "battery", "battery_model",
		dev, &bat_model_sysfs_attr_group);
}
#else
static inline void bat_model_sysfs_create_group(struct device *dev)
{
}

static inline void bat_model_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int bat_model_probe(struct platform_device *pdev)
{
	struct bat_model_device *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	bat_model_sysfs_create_group(di->dev);
	g_bat_model_dev = di;
	mutex_init(&di->update_lock);

	bat_model_init_data(di);
	bat_model_parse_dts(di);
	bat_model_debug_register(di);
	platform_set_drvdata(pdev, di);
	return 0;
}

static int bat_model_remove(struct platform_device *pdev)
{
	struct bat_model_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	mutex_destroy(&di->update_lock);
	bat_model_sysfs_remove_group(di->dev);
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	g_bat_model_dev = NULL;
	return 0;
}

static const struct of_device_id bat_model_match_table[] = {
	{
		.compatible = "huawei,battery_model",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_model_driver = {
	.probe = bat_model_probe,
	.remove = bat_model_remove,
	.driver = {
		.name = "huawei,battery_model",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_model_match_table),
	},
};

static int __init bat_model_init(void)
{
	return platform_driver_register(&bat_model_driver);
}

static void __exit bat_model_exit(void)
{
	platform_driver_unregister(&bat_model_driver);
}

subsys_initcall_sync(bat_model_init);
module_exit(bat_model_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery model driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
