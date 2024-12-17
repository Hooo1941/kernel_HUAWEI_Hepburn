// SPDX-License-Identifier: GPL-2.0
/*
 * multi_btb_temp.c
 *
 * multi btb temperature monitor and mixed driver
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
#include <securec.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/thermal.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_temp.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG btb_temp
HWLOG_REGIST();

#define INVALID_TEMP_HIGH                80000 /* 80 degree */
#define INVALID_TEMP_LOW                 (-40000) /* -40 degree */
#define READ_TEMPERATURE_MS              5000 /* 5 sec */
#define UPDATE_TEMP_FAST_MS              1000 /* 1 sec */
#define UPDATE_TEMP_FAST_TIMES           10

#define MIN_LOG_SHOW_LEN                 2

/*
 * define temp compensation value of different ibat
 * support up to 8 parameters list on dts
 */
#define SENSOR_MAX_NUM                   8
#define COMP_PARA_LEVEL                  16
#define COMP_PARA_NAME_LEN_MAX           32

enum comp_para_info {
	COMP_PARA_ICHG = 0,
	COMP_PARA_TEMP,
	COMP_PARA_TOTAL,
};

enum linear_comp_info {
	LINEAR_COMP_BUCK_5V_RATIO = 0,
	LINEAR_COMP_BUCK_9V_RATIO,
	LINEAR_COMP_K1_WIRED,
	LINEAR_COMP_K1_WIRELESS,
	LINEAR_COMP_K2_WIRED,
	LINEAR_COMP_K2_WIRELESS,
	LINEAR_COMP_K2_DISCHARGE,
	LINEAR_COMP_TOTAL,
};

struct linear_comp_para {
	int buck_5v_ratio;
	int buck_9v_ratio;
	int linear_k1_wired;
	int linear_k1_wireless;
	int linear_k2_wired;
	int linear_k2_wireless;
	int linear_k2_discharge;
};

struct linear_comp_data {
	int k1;
	int k2;
	int ichg;
	int ibat;
	int vbat;
};

enum comp_mode {
	COMP_MODE_BEGIN = 0,
	STEP_COMP_MODE = COMP_MODE_BEGIN,
	LINEAR_COMP_MODE,
	COMP_MODE_END,
};

enum sensor_info {
	SENSOR_INFO_SENSOR_INDEX = 0,
	SENSOR_INFO_COMP_PARA_NAME,
	SENSOR_INFO_COMP_MODE,
	SENSOR_INFO_TOTAL,
};

struct sensor_para {
	char name[THERMAL_NAME_LENGTH];
	char comp_para_name[COMP_PARA_NAME_LEN_MAX];
	struct compensation_para comp_data[COMP_PARA_LEVEL];
	struct linear_comp_para linear_comp_data;
	int comp_mode;
	int comp_data_size;
	int temp;
	int raw;
};

struct multi_btb_temp_info {
	struct device *dev;
	struct delayed_work temp_work;
	struct notifier_block connect_nb;
	struct notifier_block charge_channel_nb;
	struct sensor_para sensor_tab[SENSOR_MAX_NUM];
	struct legal_range range;
	int sensor_names_para_len;
	int ntc_compensation_is;
	int mixed_temp_en;
	int temp_mixed;
	int sensor_num;
	int max_tbat_gap;
	bool temp_already_init;
};

enum plugged_state {
	NOT_PLUGGED,
	WIRED_PLUGGED,
	WIRELESS_PLUGGED,
};

struct charge_state {
	enum plugged_state plugged;
	bool wired_dc_flag;
	bool wireless_dc_flag;
	int ratio;
};

static struct multi_btb_temp_info *g_info;
static struct charge_state g_charge_state;

static bool multi_btb_check_temp_is_legal(int temp)
{
	/*
	 * fix v600 hardware issue:
	 * temp is 125 degree when adaptor disconnect
	 * and fix driver(i2c and platform) loading sequence issue
	 */
	if ((temp >= INVALID_TEMP_HIGH) || (temp <= INVALID_TEMP_LOW))
		return false;
	return true;
}

static int multi_btb_get_temp_with_step_comp(struct sensor_para *para, int temp_without_comp)
{
	struct common_comp_data data = { 0 };
	int refer;

	refer = power_platform_get_battery_current();
	data.refer = abs(refer);
	data.para = para->comp_data;
	data.para_size = para->comp_data_size;

	return power_get_compensation_value(temp_without_comp, &data);
}

static void multi_btb_get_linear_comp_data(struct linear_comp_para *para, struct linear_comp_data *data)
{
	int ratio = 0;
	int ibus = 0;
	int vbus = 0;

	switch (g_charge_state.plugged) {
	case WIRED_PLUGGED:
		data->k1 = para->linear_k1_wired;
		data->k2 = para->linear_k2_wired;
		break;
	case WIRELESS_PLUGGED:
		data->k1 = para->linear_k1_wireless;
		data->k2 = para->linear_k2_wireless;
		break;
	default:
		data->k2 = para->linear_k2_discharge;
		break;
	}

	if (g_charge_state.wired_dc_flag) {
		ratio = g_charge_state.ratio * POWER_TENTH;
		dc_get_device_ibus(&ibus);
	} else if (g_charge_state.wireless_dc_flag) {
		ratio = g_charge_state.ratio * POWER_TENTH;
		(void)wlrx_ic_get_iout(WLTRX_IC_MAIN, &ibus);
	} else {
		/* If vbus is greater than 6V, the charging mode is considered to be 9V BUCK */
		charge_get_vbus(&vbus);
		if (vbus > 6000)
			ratio = para->buck_9v_ratio;
		else
			ratio = para->buck_5v_ratio;
		ibus = charge_get_ibus();
	}

	data->ichg = ibus * ratio / POWER_TENTH;
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CURRENT_NOW, &data->ibat,
		POWER_SUPPLY_DEFAULT_CURRENT_NOW, 1);
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &data->vbat,
		POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);

	hwlog_info("k1=%d,ibus=%d,ratio=%d,ibat=%d,vbat=%d,k2=%d\n", data->k1, ibus, ratio, data->ibat, data->vbat, data->k2);
}

static int multi_btb_get_temp_with_linear_comp(struct linear_comp_para *para, int temp_without_comp)
{
	struct linear_comp_data data = { 0 };
	int psys;
	int comp;

	multi_btb_get_linear_comp_data(para, &data);
	psys = abs(data.ichg - data.ibat) * data.vbat;
	comp = data.k1 * data.ichg / POWER_TENTH + data.k2 * psys / POWER_TENTH / POWER_UW_PER_MW;

	return temp_without_comp - comp;
}

static int multi_btb_get_temp_with_comp(int index, int temp_without_comp)
{
	struct multi_btb_temp_info *di = g_info;
	struct smooth_comp_data smooth_data = { 0 };
	int temp_with_comp;

	if (!di) {
		hwlog_err("di is null\n");
		return temp_without_comp;
	}

	if (!di->ntc_compensation_is || !multi_btb_check_temp_is_legal(temp_without_comp))
		return temp_without_comp;

	switch (di->sensor_tab[index].comp_mode) {
	case STEP_COMP_MODE:
		temp_with_comp = multi_btb_get_temp_with_step_comp(&di->sensor_tab[index], temp_without_comp);
		break;
	case LINEAR_COMP_MODE:
		temp_with_comp = multi_btb_get_temp_with_linear_comp(&di->sensor_tab[index].linear_comp_data, temp_without_comp);
		break;
	default:
		temp_with_comp = temp_without_comp;
		break;
	}

	if (multi_btb_check_temp_is_legal(di->sensor_tab[index].raw) && di->temp_already_init) {
		smooth_data.current_raw = temp_without_comp;
		smooth_data.current_comp = temp_with_comp;
		smooth_data.last_raw = di->sensor_tab[index].raw;
		smooth_data.last_comp = di->sensor_tab[index].temp;
		smooth_data.max_delta = di->max_tbat_gap;
		temp_with_comp = power_get_smooth_compensation_value(&smooth_data);
	}

	hwlog_info("sensor:%d,raw:%d,temp:%d\n", index, temp_without_comp, temp_with_comp);
	return temp_with_comp;
}

static int multi_btb_calculate_mixed_temp(struct multi_btb_temp_info *di)
{
	int i;
	int mixed;

	mixed = di->sensor_tab[0].temp;
	/* i start with 1, mixed tab[0], tab[1] ...... */
	for (i = 1; i < di->sensor_num; i++)
		mixed = power_get_mixed_value(mixed, di->sensor_tab[i].temp, &di->range);

	hwlog_info("mixed temp:%d\n", mixed);
	return mixed;
}

static int multi_btb_get_temp_sync(const int index, const char *sensor_name)
{
	int raw;

	raw = power_temp_get_average_value(sensor_name);
	return multi_btb_get_temp_with_comp(index, raw);
}

static int multi_btb_get_mixed_temp_sync(struct multi_btb_temp_info *di)
{
	int i;
	int mixed, temp;

	mixed = multi_btb_get_temp_sync(0, di->sensor_tab[0].name);
	/* i start with 1, mixed tab[0], tab[1] ...... */
	for (i = 1; i < di->sensor_num; i++) {
		temp = multi_btb_get_temp_sync(i, di->sensor_tab[i].name);
		mixed = power_get_mixed_value(mixed, temp, &di->range);
	}
	return mixed;
}

static void multi_btb_update_temp(struct multi_btb_temp_info *di)
{
	int i, raw, temp;

	for (i = 0; i < di->sensor_num; i++) {
		raw = power_temp_get_average_value(di->sensor_tab[i].name);
		temp = multi_btb_get_temp_with_comp(i, raw);
		hwlog_info("update temp%d:%s,raw:%d,temp:%d\n", i,
			di->sensor_tab[i].name, raw, temp);
		di->sensor_tab[i].raw = raw;
		di->sensor_tab[i].temp = temp;
	}

	if (di->mixed_temp_en)
		di->temp_mixed = multi_btb_calculate_mixed_temp(di);

	if (!di->temp_already_init)
		di->temp_already_init = true;
}

static void multi_btb_temp_work(struct work_struct *work)
{
	struct multi_btb_temp_info *di = container_of(work,
		struct multi_btb_temp_info, temp_work.work);
	static int cnt = 0;

	multi_btb_update_temp(di);

	if (cnt <= UPDATE_TEMP_FAST_TIMES) {
		cnt++;
		schedule_delayed_work(&di->temp_work,
			msecs_to_jiffies(UPDATE_TEMP_FAST_MS));
	} else {
		schedule_delayed_work(&di->temp_work,
			msecs_to_jiffies(READ_TEMPERATURE_MS));
	}
}

static int multi_btb_temp(enum bat_temp_id id, int *temp)
{
	int batt_temp;
	struct multi_btb_temp_info *di = g_info;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -EPERM;
	}

	if (!di || (di->sensor_num <= 0)) {
		*temp = POWER_TEMP_INVALID_TEMP / POWER_MC_PER_C;
		hwlog_err("g_di is null or sensor_num is 0\n");
		return -EINVAL;
	}

	if (!di->temp_already_init)
		multi_btb_update_temp(di);

	if ((id == BAT_TEMP_MIXED) && di->mixed_temp_en) {
		batt_temp = di->temp_mixed;
		*temp = batt_temp / POWER_MC_PER_C;
		hwlog_info("sensor%d:temp:%d\n", id, batt_temp);
		return 0;
	}

	if ((int)id >= di->sensor_num)
		id = BTB_TEMP_0;

	batt_temp = di->sensor_tab[id].temp;
	hwlog_info("sensor%d temp:%d\n", id, batt_temp);
	*temp = batt_temp / POWER_MC_PER_C;
	return 0;
}

static int multi_btb_temp_sync(enum bat_temp_id id, int *temp)
{
	struct multi_btb_temp_info *di = g_info;
	int batt_temp;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -EPERM;
	}

	if (!di || (di->sensor_num <= 0)) {
		*temp = POWER_TEMP_INVALID_TEMP / POWER_MC_PER_C;
		hwlog_err("g_di is null or sensor_num is 0\n");
		return -EINVAL;
	}

	if ((id == BAT_TEMP_MIXED) && di->mixed_temp_en) {
		batt_temp = multi_btb_get_mixed_temp_sync(di);
		*temp = batt_temp / POWER_MC_PER_C;
		hwlog_info("temp_sync%d:temp:%d\n", id, batt_temp);
		return 0;
	}

	if ((int)id >= di->sensor_num)
		id = BTB_TEMP_0;

	batt_temp = multi_btb_get_temp_sync(id, di->sensor_tab[id].name);

	hwlog_info("temp_sync%d temp:%d\n", id, batt_temp);
	*temp = batt_temp / POWER_MC_PER_C;
	return 0;
}

static int multi_btb_get_register_head(char *buffer, int size, void *dev_data)
{
	struct multi_btb_temp_info *di = g_info;
	int i;
	int btb_num;
	int len;
 
	if (!buffer || !di)
		return -EPERM;

	btb_num = di->sensor_num > MIN_LOG_SHOW_LEN ? di->sensor_num : MIN_LOG_SHOW_LEN;
	for (i = 0; i < btb_num; i++) {
		len = strlen(buffer);
		(void)snprintf_s(buffer + len, size - len, size - len - 1, "btb_temp%-3d", i);
	}
	return 0;
}

static int multi_btb_value_dump(char *buffer, int size, void *dev_data)
{
	struct multi_btb_temp_info *di = g_info;
	int btb_temp = 0;
	int i;
	int btb_num;
	int len;
 
	if (!buffer || !di)
		return -EPERM;

	btb_num = di->sensor_num > MIN_LOG_SHOW_LEN ? di->sensor_num : MIN_LOG_SHOW_LEN;
	for (i = 0; i < btb_num; i++) {
		multi_btb_temp_sync(i, &btb_temp);
		len = strlen(buffer);
		(void)snprintf_s(buffer + len, size - len, size - len - 1, "%-11d", btb_temp);
	}
 
	return 0;
}

static struct bat_temp_ops multi_temp_ops = {
	.get_rt_temp = multi_btb_temp_sync,
	.get_temp = multi_btb_temp,
};

static struct power_log_ops multi_btb_log_ops = {
	.dev_name = "multi_btb",
	.dump_log_head = multi_btb_get_register_head,
	.dump_log_content = multi_btb_value_dump,
};

static int multi_btb_temp_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	switch (event) {
	case POWER_NE_USB_DISCONNECT:
	case POWER_NE_WIRELESS_DISCONNECT:
		g_charge_state.plugged = NOT_PLUGGED;
		break;
	case POWER_NE_WIRELESS_CONNECT:
		g_charge_state.plugged = WIRELESS_PLUGGED;
		break;
	case POWER_NE_USB_CONNECT:
		g_charge_state.plugged = WIRED_PLUGGED;
		break;
	case POWER_NE_BUCK_CHANNEL:
		g_charge_state.wired_dc_flag = false;
		g_charge_state.wireless_dc_flag = false;
		break;
	case POWER_NE_WIRED_DC_CHANNEL:
		g_charge_state.wired_dc_flag = true;
		g_charge_state.ratio = *(int *)data;
		break;
	case POWER_NE_WIRELESS_DC_CHANNEL:
		g_charge_state.wireless_dc_flag = true;
		g_charge_state.ratio = *(int *)data;
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static ssize_t multi_btb_dbg_para_store(void *dev_data,
	const char *buf, size_t size)
{
	int max_tbat_gap = 0;
	int ret;
	struct multi_btb_temp_info *di = dev_data;

	if (!di)
		return -EINVAL;

	ret = kstrtoint(buf, 0, &max_tbat_gap);
	if (ret) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->max_tbat_gap = max_tbat_gap;
	hwlog_info("set max_tbat_gap:%d\n", max_tbat_gap);

	return size;
}

static ssize_t multi_btb_dbg_para_show(void *dev_data,
	char *buf, size_t size)
{
	struct multi_btb_temp_info *di = dev_data;

	if (!di)
		return scnprintf(buf, size, "not support\n");

	return snprintf(buf, PAGE_SIZE, "%d\n", di->max_tbat_gap);
}

static int multi_btb_temp_parse_sensor_tab(struct device_node *np,
	struct multi_btb_temp_info *di)
{
	int array_len, i, row, col;
	int comp_mode = 0;
	const char *str = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"sensor-names", SENSOR_MAX_NUM, di->sensor_names_para_len);
	if (array_len <= 0) {
		hwlog_err("sensor_table read fail\n");
		goto err_out;
	}

	di->sensor_num = array_len / di->sensor_names_para_len;
	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "sensor-names", i, &str))
			goto err_out;

		row = i / di->sensor_names_para_len;
		col = i % di->sensor_names_para_len;
		switch (col) {
		case SENSOR_INFO_SENSOR_INDEX:
			if (strlen(str) >= THERMAL_NAME_LENGTH) {
				hwlog_err("invalid sensor name\n");
				goto err_out;
			}
			strncpy(di->sensor_tab[row].name,
				str, THERMAL_NAME_LENGTH - 1);
			break;
		case SENSOR_INFO_COMP_PARA_NAME:
			strncpy(di->sensor_tab[row].comp_para_name,
				str, COMP_PARA_NAME_LEN_MAX - 1);
			break;
		case SENSOR_INFO_COMP_MODE:
			if (kstrtoint(str, POWER_BASE_DEC, &comp_mode))
				goto err_out;

			if ((comp_mode < COMP_MODE_BEGIN) || (comp_mode >= COMP_MODE_END))
				goto err_out;

			di->sensor_tab[row].comp_mode = comp_mode;
			break;
		default:
			break;
		}
	}

	for (i = 0; i < di->sensor_num; i++)
		hwlog_info("sensor_tab[%d] %s %s %d\n", i, di->sensor_tab[i].name,
			di->sensor_tab[i].comp_para_name, di->sensor_tab[i].comp_mode);

	return 0;

err_out:
	di->sensor_num = 0;
	return -EPERM;
}

static int multi_btb_temp_parse_step_comp_para(struct device_node *np,
	struct sensor_para *para)
{
	int array_len, col, row;
	int idata[COMP_PARA_LEVEL * COMP_PARA_TOTAL] = { 0 };
	const char *str = NULL;

	para->comp_data_size = 0;
	str = para->comp_para_name;
	array_len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		str, idata, COMP_PARA_LEVEL, COMP_PARA_TOTAL);
	if (array_len < 0)
		return -EPERM;

	para->comp_data_size = array_len / COMP_PARA_TOTAL;
	for (row = 0; row < array_len / COMP_PARA_TOTAL; row++) {
		col = row * COMP_PARA_TOTAL + COMP_PARA_ICHG;
		para->comp_data[row].refer = idata[col];
		col = row * COMP_PARA_TOTAL + COMP_PARA_TEMP;
		para->comp_data[row].comp_value = idata[col];
		hwlog_info("temp_comp_para[%d]=%d %d\n", row,
			para->comp_data[row].refer, para->comp_data[row].comp_value);
	}

	return 0;
}

static int multi_btb_temp_parse_linear_comp_para(struct device_node *np,
	struct sensor_para *para)
{
	int len;
	int tmp_para[LINEAR_COMP_TOTAL] = { 0 };
	const char *str = NULL;

	str = para->comp_para_name;
	/* 1:only one line parameters */
	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np, str,
		tmp_para, 1, LINEAR_COMP_TOTAL);
	if (len < 0) {
		hwlog_err("%s parse error\n", __func__);
		return -EINVAL;
	}

	para->linear_comp_data.buck_5v_ratio = tmp_para[LINEAR_COMP_BUCK_5V_RATIO];
	para->linear_comp_data.buck_9v_ratio = tmp_para[LINEAR_COMP_BUCK_9V_RATIO];
	para->linear_comp_data.linear_k1_wired = tmp_para[LINEAR_COMP_K1_WIRED];
	para->linear_comp_data.linear_k1_wireless = tmp_para[LINEAR_COMP_K1_WIRELESS];
	para->linear_comp_data.linear_k2_wired = tmp_para[LINEAR_COMP_K2_WIRED];
	para->linear_comp_data.linear_k2_wireless = tmp_para[LINEAR_COMP_K2_WIRELESS];
	para->linear_comp_data.linear_k2_discharge = tmp_para[LINEAR_COMP_K2_DISCHARGE];
	hwlog_info("%s %d %d %d %d %d %d\n", str, tmp_para[LINEAR_COMP_BUCK_5V_RATIO],
		tmp_para[LINEAR_COMP_BUCK_9V_RATIO], tmp_para[LINEAR_COMP_K1_WIRED],
		tmp_para[LINEAR_COMP_K1_WIRELESS], tmp_para[LINEAR_COMP_K2_WIRED],
		tmp_para[LINEAR_COMP_K2_WIRELESS], tmp_para[LINEAR_COMP_K2_DISCHARGE]);

	return 0;
}

static void multi_btb_temp_parse_group_comp_para(
	struct device_node *np, struct multi_btb_temp_info *di)
{
	int i;

	if (!di || !di->sensor_num || !di->ntc_compensation_is)
		return;

	for (i = 0; i < di->sensor_num; i++) {
		switch (di->sensor_tab[i].comp_mode) {
		case STEP_COMP_MODE:
			if (multi_btb_temp_parse_step_comp_para(np, &di->sensor_tab[i]))
				goto err_out;
			break;
		case LINEAR_COMP_MODE:
			if (multi_btb_temp_parse_linear_comp_para(np, &di->sensor_tab[i]))
				goto err_out;
			break;
		default:
			break;
		}
	}

	return;

err_out:
	di->ntc_compensation_is = 0;
}

static int multi_btb_temp_parse_dts(struct device_node *np,
	struct multi_btb_temp_info *di)
{
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ntc_compensation_is", &di->ntc_compensation_is, 0);

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tbat_gap", &di->max_tbat_gap, 5000); /* 5 degrees */

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"mixed_temp_en", &di->mixed_temp_en, 0);

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"battery_temp_high", &di->range.high, BAT_TEMP_HIGH);

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"battery_temp_low", &di->range.low, BAT_TEMP_LOW);

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sensor_names_para_len", &di->sensor_names_para_len, 2);

	if (multi_btb_temp_parse_sensor_tab(np, di))
		return -EPERM;

	multi_btb_temp_parse_group_comp_para(np, di);
	return 0;
}

static int multi_btb_temp_probe(struct platform_device *pdev)
{
	struct multi_btb_temp_info *di = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	ret = multi_btb_temp_parse_dts(np, di);
	if (ret)
		goto err_out;

	platform_set_drvdata(pdev, di);
	g_info = di;
	di->temp_already_init = false;

	ret = bat_temp_ops_register("multi_btb_temp", &multi_temp_ops);
	if (ret)
		goto err_out;

	di->connect_nb.notifier_call = multi_btb_temp_notifier_call;
	(void)power_event_bnc_register(POWER_BNT_CONNECT, &di->connect_nb);
	di->charge_channel_nb.notifier_call = multi_btb_temp_notifier_call;
	(void)power_event_bnc_register(POWER_BNT_CHANNEL, &di->charge_channel_nb);

	multi_btb_log_ops.dev_data = (void *)di;
	power_log_ops_register(&multi_btb_log_ops);

	power_dbg_ops_register("multi_btb", "para", (void *)di,
		multi_btb_dbg_para_show, multi_btb_dbg_para_store);

	INIT_DELAYED_WORK(&di->temp_work, multi_btb_temp_work);
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));

	return 0;

err_out:
	devm_kfree(&pdev->dev, di);
	g_info = NULL;
	return ret;
}

static int multi_btb_temp_remove(struct platform_device *pdev)
{
	struct multi_btb_temp_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_bnc_unregister(POWER_BNT_CONNECT, &di->connect_nb);
	power_event_bnc_unregister(POWER_BNT_DC, &di->charge_channel_nb);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_info = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int multi_btb_temp_resume(struct platform_device *pdev)
{
	struct multi_btb_temp_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	di->temp_already_init = false;
	schedule_delayed_work(&di->temp_work, 0);
	return 0;
}

static int multi_btb_temp_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	struct multi_btb_temp_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	cancel_delayed_work_sync(&di->temp_work);
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id btb_temp_match_table[] = {
	{
		.compatible = "huawei,multi_btb_temp",
		.data = NULL,
	},
	{},
};

static struct platform_driver multi_btb_temp_driver = {
	.probe = multi_btb_temp_probe,
	.remove = multi_btb_temp_remove,
#ifdef CONFIG_PM
	.resume = multi_btb_temp_resume,
	.suspend = multi_btb_temp_suspend,
#endif /* CONFIG_PM */
	.driver = {
		.name = "huawei,multi_btb_temp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(btb_temp_match_table),
	},
};

static int __init multi_btb_temp_init(void)
{
	return platform_driver_register(&multi_btb_temp_driver);
}

static void __exit multi_btb_temp_exit(void)
{
	platform_driver_unregister(&multi_btb_temp_driver);
}

device_initcall_sync(multi_btb_temp_init);
module_exit(multi_btb_temp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei multi btb temp module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
