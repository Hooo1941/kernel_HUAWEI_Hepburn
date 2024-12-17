/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: sysfs process func for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifdef CONFIG_CHARGE_TIME
#include "../../charge_time/hisi_charge_time.h"
#endif
#ifdef CONFIG_BATTERY_SYSFS
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#endif
#include <platform_include/basicplatform/linux/power/platform/coul/coul_drv.h>
#include <platform_include/basicplatform/linux/power/platform/bci_battery.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_merge_drv.h>
#include <securec.h>

#define coul_merge_debug(fmt, args...) pr_debug("[coul_merge_sysfs]" fmt, ## args)
#define coul_merge_info(fmt, args...) pr_info("[coul_merge_sysfs]" fmt, ## args)
#define coul_merge_warn(fmt, args...) pr_warn("[coul_merge_sysfs]" fmt, ## args)
#define coul_merge_err(fmt, args...) pr_err("[coul_merge_sysfs]" fmt, ## args)

#define STRTOL_DECIMAL_BASE     10
struct gaugelog_data {
	int temp;
	int voltage;
	int cur;
	int ufcapacity;
	int capacity;
	int rm;
	int fcc;
	int uuc;
	int cc;
	int delta_rc;
	int rbatt;
	int ocv;
	int high_pre_qmax;
	int limit_fcc;
	unsigned char last_ocv_level;
	int ocv_valid_to_refresh_fcc;
	int batt_cycle;
};

enum {
	BATT_INDEX_0 = 0,
	BATT_INDEX_1,
	BATT_INDEX_END
};

static struct coul_batt_cali_info cali_info;

#ifdef CONFIG_SYSFS
enum coul_sysfs_type {
	COUL_SYSFS_GAUGELOG_HEAD,
	COUL_SYSFS_GAUGELOG,
	COUL_SYSFS_BATT_CALI,
	COUL_SYSFS_BATT_MODE,
	COUL_SYSFS_ADC_CALI_DISABLE,
	COUL_SYSFS_MAX,
};

#define coul_sysfs_field(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, coul_sysfs_show, store), \
	.name = COUL_SYSFS_##n, \
}

#define coul_sysfs_field_rw(_name, n) \
	coul_sysfs_field(_name, n, S_IWUSR | S_IRUGO, coul_sysfs_store)

#define coul_sysfs_field_ro(_name, n) coul_sysfs_field(_name, n, S_IRUGO, NULL)

struct coul_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t coul_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
static ssize_t coul_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct coul_sysfs_field_info g_coul_sysfs_field_tbl[] = {
	coul_sysfs_field_ro(gaugelog, GAUGELOG),
	coul_sysfs_field_ro(gaugelog_head, GAUGELOG_HEAD),
	coul_sysfs_field_rw(batt_cali, BATT_CALI),
	coul_sysfs_field_rw(batt_mode, BATT_MODE),
	coul_sysfs_field_rw(adc_cali_disable, ADC_CALI_DISABLE),
};

static struct attribute *g_coul_sysfs_attrs[ARRAY_SIZE(g_coul_sysfs_field_tbl) + 1];

static const struct attribute_group g_coul_sysfs_attr_group = {
	.attrs = g_coul_sysfs_attrs,
};

/* get the current device_attribute from charge_sysfs_field_tbl by attr's name */
static struct coul_sysfs_field_info *coul_sysfs_field_lookup(
	const char *name)
{
	size_t i;
	size_t limit = ARRAY_SIZE(g_coul_sysfs_field_tbl);

	coul_merge_debug("%s +\n", __func__);
	for (i = 0; i < limit; i++) {
		if (!strncmp(name, g_coul_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}
	if (i >= limit)
		return NULL;

	coul_merge_debug("%s -\n", __func__);
	return &g_coul_sysfs_field_tbl[i];
}

static void get_log_data(int batt, struct gaugelog_data *log_data)
{
	log_data->temp = coul_merge_drv_battery_temperature(batt);
	log_data->voltage = coul_merge_drv_battery_voltage_mv(batt);
	log_data->cur = coul_merge_drv_battery_current(batt);
	log_data->ufcapacity = coul_merge_drv_battery_unfiltered_capacity(batt);
	log_data->capacity = coul_merge_drv_battery_capacity(batt);
	log_data->rm = coul_merge_drv_battery_rm(batt);
	log_data->fcc = coul_merge_drv_battery_fcc(batt);
	log_data->uuc = coul_merge_drv_battery_uuc(batt);
	log_data->cc = coul_merge_drv_battery_cc(batt);
	log_data->delta_rc = coul_merge_drv_battery_delta_rc(batt);
	log_data->ocv = coul_merge_drv_get_ocv(batt) / PERMILLAGE;
	log_data->rbatt = coul_merge_drv_battery_resistance(batt);
	log_data->high_pre_qmax = coul_merge_drv_battery_get_qmax(batt);
	log_data->limit_fcc = coul_merge_drv_battery_get_limit_fcc(batt);
	log_data->last_ocv_level = coul_merge_drv_get_ocv_level(batt);
	log_data->ocv_valid_to_refresh_fcc = coul_merge_drv_ocv_valid_refresh_fcc(batt);
	log_data->batt_cycle = coul_merge_drv_battery_cycle_count(batt);
}

static void show_each_battery_gaugelog(char *buf, struct gaugelog_data *log_data)
{
	size_t len = strlen(buf);
	int ret;

	ret = snprintf_s(buf + len, PAGE_SIZE - len, PAGE_SIZE -len - 1,
		"%-5d  %-5d  %-7d  %-5d  %-6d  %-5d  %-6d  %-5d  "
		"%-7d  %-5d  %-6d  %-5d  %-7d  %-8d  %-7d  %-9d  "
		"%-7d  ",
		log_data->voltage, (signed short)log_data->cur,
		log_data->ufcapacity, log_data->capacity, log_data->rm,
		log_data->fcc, log_data->high_pre_qmax, log_data->uuc,
		log_data->cc, log_data->delta_rc, log_data->temp,
		log_data->ocv, log_data->rbatt, log_data->limit_fcc,
		(int)log_data->last_ocv_level, log_data->ocv_valid_to_refresh_fcc,
		log_data->batt_cycle);
	if (ret < 0) {
		coul_merge_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
		return;
	}
}
static ssize_t show_gaugelog(char *buf)
{
	int ret;
	struct gaugelog_data batt_0_data = {0};
	struct gaugelog_data batt_1_data = {0};
	struct gaugelog_data batt_2_data = {0};
	int ui_soc = bci_show_capacity();
	int merge_soc = coul_drv_battery_capacity();

	get_log_data(BATT_0, &batt_0_data);

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,	"%-5d  %-4d  ", ui_soc, merge_soc);
	if (ret < 0) {
		coul_merge_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
		return 0;
	}

	show_each_battery_gaugelog(buf, &batt_0_data);

	if (coul_merge_drv_get_batt_count() >= 2) {
		get_log_data(BATT_1, &batt_1_data);
		show_each_battery_gaugelog(buf, &batt_1_data);
	}
	if (coul_merge_drv_get_batt_count() >= 3) {
		get_log_data(BATT_2, &batt_2_data);
		show_each_battery_gaugelog(buf, &batt_2_data);
	}

#ifdef CONFIG_CHARGE_TIME
		ret = show_chg_time_log_content(buf, PAGE_SIZE);
		if (ret < 0) {
			coul_merge_err("(%s) snprintf_s fail ret = %d\n", __func__,ret);
			return 0;
		}
#endif

	return strlen(buf);
}

static void show_each_battery_log_head(char *buf, size_t dest_max)
{
	int ret;
	size_t len = strlen(buf);

	ret = snprintf_s(buf + len, dest_max - len, dest_max - len - 1, "%s",
		"VOL_0  CUR_0  ufSOC_0  SOC_0  RM_0    FCC_0  Qmax_0  UUC_0  CC_0     dRC_0  "
		"Temp_0  OCV_0  rbatt_0  limfcc_0  ocvLv_0  fccFlag_0  cycle_0  ");
	if (ret < 0) {
		coul_merge_err("(%s)batt_0, snprintf_s fail ret = %d\n", __func__, ret);
		return;
	}

	if (coul_merge_drv_get_batt_count() == 1) {
		coul_merge_info("(%s)batt cnt = 1\n", __func__);
		return;
	}

	len = strlen(buf);
	ret = snprintf_s(buf + len, dest_max - len, dest_max - len - 1, "%s",
		"VOL_1  CUR_1  ufSOC_1  SOC_1  RM_1    FCC_1  Qmax_1  UUC_1  CC_1     dRC_1  "
		"Temp_1  OCV_1  rbatt_1  limfcc_1  ocvLv_1  fccFlag_1  cycle_1  ");
	if (ret < 0) {
		coul_merge_err("(%s)batt_1, snprintf_s fail ret = %d\n", __func__, ret);
		return;
	}

	if (coul_merge_drv_get_batt_count() == 2) {
		coul_merge_info("(%s)batt cnt = 2\n", __func__);
		return;
	}

	len = strlen(buf);
	ret = snprintf_s(buf + len, dest_max - len, dest_max - len - 1, "%s",
		"VOL_2  CUR_2  ufSOC_2  SOC_2  RM_2    FCC_2  Qmax_2  UUC_2  CC_2     dRC_2  "
		"Temp_2  OCV_2  rbatt_2  limfcc_2  ocvLv_2  fccFlag_2  cycle_2  ");
	if (ret < 0) {
		coul_merge_err("(%s)batt_2, snprintf_s fail ret = %d\n", __func__, ret);
		return;
	}
}

static ssize_t get_gaugelog_head(char *buf)
{
	int ret;

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s", "uiSOC  mSOC  ");
	if (ret < 0) {
		coul_merge_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
		return 0;
	}

	show_each_battery_log_head(buf, PAGE_SIZE);

#ifdef CONFIG_CHARGE_TIME
		ret = show_chg_time_log_head(buf, PAGE_SIZE);
		if (ret < 0) {
			coul_merge_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return 0;
		}
#endif
	return strlen(buf);
}

static void coul_merge_sysfs_get_batt_data(struct coul_batt_cali_info *info)
{
	if (info->data_type == BATT_DATA_VOLT)
		info->a = coul_merge_drv_battery_voltage_uv(info->batt_index);
	else if (info->data_type == BATT_DATA_CUR)
		info->a = coul_merge_drv_battery_current(info->batt_index);
}

static ssize_t show_coul_cali_info(char *buf)
{
	int ret;
	struct coul_batt_cali_info info = { 0 };

	coul_merge_err("%s +\n", __func__);
	info.batt_index = cali_info.batt_index;
	info.charge_mode = cali_info.charge_mode;
	info.data_type = cali_info.data_type;

	switch (info.data_type) {
	case BATT_DATA_VOLT:
	case BATT_DATA_CUR:
		coul_merge_sysfs_get_batt_data(&info);
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", info.a);
		break;
	case BATT_DATA_VOLT_CALI:
	case BATT_DATA_CUR_CALI:
		if (coul_merge_drv_get_cali(&info))
			return -1;
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
			"batt_index:%d, charge_mode:%d, data_type:%d, a:%d, b:%d\n",
			info.batt_index, info.charge_mode,
			info.data_type, info.a, info.b);
		break;
	default:
		coul_merge_err("%s data type invalid\n", __func__);
		return -1;
	}

	if (ret < 0)
		coul_merge_err("(%s)snprintf_s fail ret=%d\n", __func__, ret);

	coul_merge_info("%s  %s,  %d\n", __func__, buf, ret);
	return ret;
}

static ssize_t show_adc_cali_disable_state(char *buf)
{
	int state = -1;
	int ret = -1;

	state = coul_merge_drv_get_adc_cali_state();
	if((state == 0) || (state == 1))
	{
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d", state);
		if (ret < 0)
			coul_merge_err("(%s)snprintf_s fail ret=%d\n", __func__, ret);
	}

	coul_merge_info("%s  %s, state:%d, %d\n", __func__, buf, state, ret);
	return ret;
}


/* show the value for all coul device's node,
   return value:  0-sucess or others-fail */
static ssize_t coul_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct coul_sysfs_field_info *info = NULL;
	struct coul_merge_device *di = dev_get_drvdata(dev);

	info = coul_sysfs_field_lookup(attr->attr.name);
	if ((di == NULL) || (info == NULL)) {
		coul_merge_err("%s di is null\n", __func__);
		return -EINVAL;
	}

	switch (info->name) {
	case COUL_SYSFS_GAUGELOG_HEAD:
		return get_gaugelog_head(buf);
	case COUL_SYSFS_GAUGELOG:
		return show_gaugelog(buf);
	case COUL_SYSFS_BATT_CALI:
		return show_coul_cali_info(buf);
	case COUL_SYSFS_ADC_CALI_DISABLE:
		return show_adc_cali_disable_state(buf);
	default:
		coul_merge_err("(%s)NODE ERR!!HAVE NO THIS NODE: %u\n",
			__func__, info->name);
		return 0;
	}
}

#define PARAMS_NUM 5
static int get_batt_cali_info(char *buf, struct coul_batt_cali_info *info)
{
	int ret = 0;
	char *token = NULL;
	char *next_token = NULL;
	int i = 0;
	int val[PARAMS_NUM] = {0};

	token = strtok_s(buf, " ", &next_token);
	while(token != NULL) {
		if(kstrtol(token, DEC, (long *)(&(val[i]))))
			return -1;
		i++;
		if(i > PARAMS_NUM)
			return -1;
		token = strtok_s(NULL, " ", &next_token);
	}

	ret = memcpy_s(info, sizeof(*info), &val, PARAMS_NUM * sizeof(int));
	if (ret != EOK) {
		coul_merge_err("[%s] memcpy_s failed\n",__func__);
		return -EINVAL;
	}

	coul_merge_info("[%s] batt_index %d, charge_mode %d, data_type %d, a %d, b %d\n",
		__func__, info->batt_index, info->charge_mode, info->data_type,
		info->a, info->b);
	return 0;
}

/* echo batt_index:0 charge_mode:0 volt_curr:0 a:1000000 b:5  */
static void coul_batt_cali_store(char *buf)
{
	struct coul_batt_cali_info *info = &cali_info;

	coul_merge_info("%s %s \n", __func__, buf);

	(void)memset_s(info, sizeof(*info), 0, sizeof(*info));
	if(get_batt_cali_info(buf, info)) {
		coul_merge_info("%s get_batt_cali_info error\n", __func__);
		return;
	}

	if (((info->data_type != BATT_DATA_VOLT_CALI) &&
		(info->data_type != BATT_DATA_CUR_CALI)) || (info->a == 0)) {
		coul_merge_info("%s a is 0\n", __func__);
		return;
	}

	coul_merge_drv_set_cali(info);
	coul_merge_info("%s -\n", __func__);
}

static void coul_batt_mode_store(char *buf)
{
	long mode = 0;

	coul_merge_info("%s %s \n", __func__, buf);
	if ((kstrtol(buf, STRTOL_DECIMAL_BASE, &mode) < 0) || (mode < 0) || (mode > 1))
			return;

	(void)coul_merge_drv_set_batt_mode(0, (int)mode);
}

static void set_adc_cali_disable_state(char *buf)
{
	long state = -1;

	coul_merge_info("%s %s \n", __func__, buf);

	if ((kstrtol(buf, STRTOL_DECIMAL_BASE, &state) < 0) || (state < 0) || (state > 1))
		return;

	coul_merge_drv_set_adc_cali_disable(0, (int)state);
	coul_merge_drv_set_adc_cali_disable(1, (int)state);
}

/* set the value for coul_data's node which is can be written,
   return value:  0-sucess or others-fail */
static ssize_t coul_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct coul_sysfs_field_info *info = NULL;
	struct smartstar_coul_device *di = dev_get_drvdata(dev);
	char *buffer = NULL;
	int ret = 0;

#ifndef CONFIG_DFX_DEBUG_FS
	return count;
#endif

	buffer = kzalloc(count, GFP_KERNEL);
	if(buffer == NULL)
		return count;

	ret = memcpy_s(buffer, count, buf, count);
	if (ret != EOK) {
		kfree(buffer);
		coul_merge_err("[%s] memcpy_s failed\n",__func__);
		return -EINVAL;
	}

	info = coul_sysfs_field_lookup(attr->attr.name);
	if ((di == NULL) || (info == NULL)) {
		ret = -1;
		goto out;
	}

	switch (info->name) {
	case COUL_SYSFS_BATT_CALI:
		coul_batt_cali_store(buffer);
		break;
	case COUL_SYSFS_BATT_MODE:
		coul_batt_mode_store(buffer);
		break;
	case COUL_SYSFS_ADC_CALI_DISABLE:
		set_adc_cali_disable_state(buffer);
		break;
	default:
		coul_merge_err("(%s)NODE ERR! HAVE NO THIS NODE: %u\n", __func__, info->name);
		break;
	}

out:
	kfree(buffer);
	return count;
}

/* initialize coul_sysfs_attrs[] for coul attribute. */
static void coul_sysfs_init_attrs(void)
{
	size_t i;
	size_t limit = ARRAY_SIZE(g_coul_sysfs_field_tbl);

	coul_merge_debug("%s\n", __func__);
	for (i = 0; i < limit; i++)
		g_coul_sysfs_attrs[i] = &g_coul_sysfs_field_tbl[i].attr.attr;
	g_coul_sysfs_attrs[limit] = NULL;
}

/* create the coul device sysfs group
   return value:   0-sucess or others-fail  */
static int coul_sysfs_create_group(struct coul_merge_device *di)
{
	coul_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &g_coul_sysfs_attr_group);
}

static inline void coul_sysfs_remove_group(struct coul_merge_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &g_coul_sysfs_attr_group);
}
#else
static int coul_sysfs_create_group(const struct coul_merge_device *di)
{
	return 0;
}

static inline void coul_sysfs_remove_group(struct coul_merge_device *di)
{
}

#endif

#ifdef CONFIG_BATTERY_SYSFS
#define DEV_NAME_MAX 10
static void create_coul_dev(struct coul_merge_device *di)
{
	int retval;

	if (di->sysfs_device != NULL) {
		retval = sysfs_create_link(&di->sysfs_device->kobj, &di->dev->kobj, "coul_data");
		if (retval)
			coul_merge_err("%s failed to create sysfs link\n", __func__);
	} else {
		coul_merge_err("%s failed to create new_dev\n", __func__);
	}
}

#endif

int coul_merge_create_sysfs(struct coul_merge_device *di)
{
	int retval;
#ifdef CONFIG_BATTERY_SYSFS
	struct class *power_class = NULL;
#endif

	if (di == NULL) {
		coul_merge_err("%s di is null", __func__);
		return -1;
	}

	retval = coul_sysfs_create_group(di);
	if (retval) {
		coul_merge_err("%s create sysfs group fail\n", __func__);
		return -1;
	}
#ifdef CONFIG_BATTERY_SYSFS
	power_class = power_sysfs_get_class("hw_power");
	if (power_class) {
		if (di->sysfs_device == NULL) {
			di->sysfs_device = power_sysfs_get_device("hw_power", "coul");
			if (IS_ERR(di->sysfs_device))
				di->sysfs_device = NULL;
		}
		create_coul_dev(di);
	}
#endif
	return retval;
}

struct device *coul_merge_dev_sysfs(struct coul_merge_device *di)
{
	return di->sysfs_device;
}
