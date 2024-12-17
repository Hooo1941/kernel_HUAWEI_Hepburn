// SPDX-License-Identifier: GPL-2.0
/*
 * charge_mode_test.c
 *
 * charge mode test
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

#include <securec.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_test.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/adapter/adapter_detect.h>
#include <chipset_common/hwpower/hvdcp_charge/hvdcp_charge.h>
#include <chipset_common/hwpower/charger/charge_mode_test.h>

#define HWLOG_TAG charge_mode_tst
HWLOG_REGIST();

static int charge_mode_parse_protocol(struct charge_mode_dev *di, unsigned int row, const char *buf, int offset);
static int charge_mode_parse_mode(struct charge_mode_dev *di, unsigned int row, const char *buf, int offset);
static int charge_mode_parse_int(struct charge_mode_dev *di, unsigned int row, const char *buf, int offset);

typedef int (*charge_mode_parse)(struct charge_mode_dev *di, unsigned int row, const char *buf, int offset);

static charge_mode_parse g_parse_tbl[] = {
	charge_mode_parse_protocol,
	charge_mode_parse_mode,
	charge_mode_parse_int,
	charge_mode_parse_int,
	charge_mode_parse_int,
	charge_mode_parse_int,
};

static struct charge_mode_map g_protocol_tbl[] = {
	{ "ufcs", CHARGE_MODE_PROTOCOL_UFCS },
	{ "scp",  CHARGE_MODE_PROTOCOL_SCP },
	{ "fcp",  CHARGE_MODE_PROTOCOL_HVC }
};

static struct charge_mode_map g_mode_tbl[] = {
	{ "dcp",      CHARGE_MODE_TYPE_DCP },
	{ "lvc",      CHARGE_MODE_TYPE_LVC },
	{ "sc",       CHARGE_MODE_TYPE_SC },
	{ "sc_main",  CHARGE_MODE_TYPE_MAIN_SC },
	{ "sc_aux",   CHARGE_MODE_TYPE_AUX_SC },
	{ "sc4",      CHARGE_MODE_TYPE_SC4 },
	{ "sc4_main", CHARGE_MODE_TYPE_MAIN_SC4 },
	{ "sc4_aux",  CHARGE_MODE_TYPE_AUX_SC4 },
	{ "hvc",      CHARGE_MODE_TYPE_HVC },
};

static int g_adp_mode_map [] = {
	ADAPTER_SUPPORT_UNDEFINED,
	ADAPTER_SUPPORT_LVC,
	ADAPTER_SUPPORT_SC,
	ADAPTER_SUPPORT_SC,
	ADAPTER_SUPPORT_SC,
	ADAPTER_SUPPORT_SC4,
	ADAPTER_SUPPORT_SC4,
	ADAPTER_SUPPORT_SC4,
	ADAPTER_SUPPORT_HV,
};

/* The order of the table is consistent with g_mode_tbl */
static struct charge_mode_action g_action_tbl[] = {
	{ POWER_IF_OP_TYPE_DCP,     INVALID_INDEX },
	{ POWER_IF_OP_TYPE_LVC,     INVALID_INDEX },
	{ POWER_IF_OP_TYPE_SC,      INVALID_INDEX },
	{ POWER_IF_OP_TYPE_MAINSC,  POWER_IF_OP_TYPE_SC },
	{ POWER_IF_OP_TYPE_AUXSC,   POWER_IF_OP_TYPE_SC },
	{ POWER_IF_OP_TYPE_SC4,     INVALID_INDEX },
	{ POWER_IF_OP_TYPE_MAINSC4, POWER_IF_OP_TYPE_SC4 },
	{ POWER_IF_OP_TYPE_AUXSC4,  POWER_IF_OP_TYPE_SC4 },
	{ POWER_IF_OP_TYPE_HVC,     INVALID_INDEX },
};

static int charge_mode_get_map_index(const char *buf, struct charge_mode_map *tbl, unsigned int size)
{
	unsigned int i;
	int buf_len, tmp_len;

	buf_len = strlen(buf);

	for (i = 0; i < size; i++) {
		tmp_len = strlen(tbl[i].name);
		if (strncmp(buf, tbl[i].name, max(buf_len, tmp_len)) == 0)
			return tbl[i].index;
	}

	return INVALID_INDEX;
}

static void charge_mode_set_result(struct charge_mode_para *mode_para, enum charge_mode_result result,
	enum charge_mode_sub_result sub_result)
{
	mode_para->result = result;
	mode_para->sub_result = sub_result;
}

static void charge_mode_set_all_result(struct charge_mode_dev *di, enum charge_mode_result result,
	enum charge_mode_sub_result sub_result)
{
	int i;

	for (i = 0; i < di->mode_num; i++)
		charge_mode_set_result(&di->mode_para[i], result, sub_result);
}

static int charge_mode_valid(struct charge_mode_dev *di)
{
	if (pd_dpm_get_cc_moisture_status()) {
		hwlog_err("cc moistrue\n");
		di->mode_para[di->mode_idx].sub_result = CHARGE_MODE_CC_MOISTURE;
	} else if (di->temp_err_flag) {
		hwlog_err("temp error\n");
		di->mode_para[di->mode_idx].sub_result = CHARGE_MODE_TEMP_ERR;
	} else if (di->voltage_invalid_flag) {
		hwlog_err("voltage invalid\n");
		di->mode_para[di->mode_idx].sub_result = CHARGE_MODE_VOL_INVALID;
	} else {
		return CHARGE_MODE_FAILURE;
	}
	return CHARGE_MODE_SUCCESS;
}

static int charge_mode_select_protocol(struct charge_mode_dev *di)
{
	unsigned int prot_type;
	unsigned int init_prot;
	int adp_mode;
	int index;

	for (adp_mode = 0; di->mode_idx < di->mode_num; di->mode_idx++) {
		index = charge_mode_get_map_index(di->mode_para[di->mode_idx].mode, g_mode_tbl, ARRAY_SIZE(g_mode_tbl));
		if (index == CHARGE_MODE_TYPE_HVC)
			return CHARGE_MODE_SUCCESS;

		index = charge_mode_get_map_index(di->mode_para[di->mode_idx].protocol, g_protocol_tbl, ARRAY_SIZE(g_protocol_tbl));
		if (index == INVALID_INDEX)
			continue;

		if ((di->mode_idx > 0) &&
			(strcmp(di->mode_para[di->mode_idx].protocol, di->mode_para[di->mode_idx - 1].protocol) == 0))
			return CHARGE_MODE_SUCCESS;

		prot_type = adapter_detect_get_sysfs_protocol_type(index);
		init_prot = adapter_detect_get_init_protocol_type();
		if ((prot_type & init_prot) || (init_prot == 0)) {
			power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ADAPTER_PROTOCOL, POWER_IF_SYSFS_ADAPTER_PROTOCOL, index);
			hwlog_info("select protocol %s\n", di->mode_para[di->mode_idx].protocol);

			(void)power_msleep(DT_MSLEEP_1S, 0, NULL);
			return CHARGE_MODE_SUCCESS;
		} else {
			charge_mode_set_result(&di->mode_para[di->mode_idx], CHARGE_MODE_RESULT_FAIL, CHARGE_MODE_UE_PROTOCOL_FAIL);
			hwlog_err("not support protocol %s\n", di->mode_para[di->mode_idx].protocol);
		}
	}

	hwlog_err("protocol select fail\n");
	return CHARGE_MODE_FAILURE;
}

static void charge_mode_select_mode(char *mode)
{
	int index;

	hwlog_info("select mode %s\n", mode);

	index = charge_mode_get_map_index(mode, g_mode_tbl, ARRAY_SIZE(g_mode_tbl));

	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL, POWER_IF_SYSFS_ENABLE_CHARGER, DISABLE);
	(void)power_msleep(DT_MSLEEP_2S, 0, NULL);
	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP, POWER_IF_SYSFS_ENABLE_CHARGER, ENABLE);
	power_if_kernel_sysfs_set(g_action_tbl[index].first, POWER_IF_SYSFS_ENABLE_CHARGER, ENABLE);

	if (g_action_tbl[index].second != INVALID_INDEX) {
		(void)power_msleep(DT_MSLEEP_1S, 0, NULL);
		power_if_kernel_sysfs_set(g_action_tbl[index].second, POWER_IF_SYSFS_ENABLE_CHARGER, ENABLE);
	}
}

static bool charge_mode_caculate(struct charge_mode_dev *di)
{
	int ibat = 0;
	int ibat_th;
	int index;

	if (di->mode_idx == INVALID_INDEX)
		return false;

	ibat_th = di->mode_para[di->mode_idx].ibat_th;
	index = charge_mode_get_map_index(di->mode_para[di->mode_idx].mode, g_mode_tbl, ARRAY_SIZE(g_mode_tbl));
	if ((index == CHARGE_MODE_TYPE_HVC) && (hvdcp_get_charging_stage() == HVDCP_STAGE_SUCCESS))
		return hvdcp_check_running_current(ibat_th);

	if (direct_charge_get_stage_status() == DC_STAGE_CHARGING)
		direct_charge_get_bat_current(&ibat);

	hwlog_info("ibat = %d ibat_th = %d\n", ibat, ibat_th);

	if (ibat >= ibat_th)
		return true;

	return false;
}

static bool charge_mode_jump_mode(struct charge_mode_dev *di)
{
	if (di->mode_idx == INVALID_INDEX)
		return true;

	if (di->mode_para[di->mode_idx].result == CHARGE_MODE_RESULT_FAIL) {
		hwlog_info("this mode test fail\n");
		return true;
	}

	if (di->mode_para[di->mode_idx].result == CHARGE_MODE_RESULT_SUCC &&
		di->mode_para[di->mode_idx].sub_result != CHARGE_MODE_SUB_INIT)
		return true;

	if (di->mode_para[di->mode_idx].sub_result == CHARGE_MODE_ADP_UNSUPPORT)
		return true;

	return false;
}

static void charge_mode_update_result(struct charge_mode_dev *di)
{
	int index;

	if (di->mode_idx == INVALID_INDEX)
		return;

	index = charge_mode_get_map_index(di->mode_para[di->mode_idx].mode, g_mode_tbl, ARRAY_SIZE(g_mode_tbl));

	if (charge_mode_valid(di) == CHARGE_MODE_SUCCESS) {
		di->mode_para[di->mode_idx].result = CHARGE_MODE_RESULT_SUCC;
		di->temp_err_flag = false;
		di->voltage_invalid_flag = false;
		return;
	}

	if ((di->mode_para[di->mode_idx].result == CHARGE_MODE_RESULT_SUCC) &&
		(di->mode_para[di->mode_idx].ext == ENABLE)) {
		di->mode_para[di->mode_idx].sub_result = CHARGE_MODE_SUB_SUCC;
		hwlog_info("end the mode test in advance\n");
		return;
	}

	if (di->curr_time - di->start_time >= di->mode_para[di->mode_idx].time) {
		hwlog_info("this mode test timeout\n");
		if (di->mode_para[di->mode_idx].result == CHARGE_MODE_RESULT_INIT)
			charge_mode_set_result(&di->mode_para[di->mode_idx], CHARGE_MODE_RESULT_FAIL, CHARGE_MODE_IBAT_FAIL);
		if (di->mode_para[di->mode_idx].result == CHARGE_MODE_RESULT_SUCC)
			di->mode_para[di->mode_idx].sub_result = CHARGE_MODE_SUB_SUCC;
		return;
	}

	if (index == CHARGE_MODE_TYPE_HVC) {
		hwlog_info("hvc test\n");
		return;
	}

	if (di->ping_result == CHARGE_MODE_FAILURE) {
		charge_mode_set_result(&di->mode_para[di->mode_idx], CHARGE_MODE_RESULT_FAIL, CHARGE_MODE_ADP_PROTOCOL_FAIL);
		di->ping_result = CHARGE_MODE_SUCCESS;
		hwlog_info("adapter not support protocol %s\n", di->mode_para[di->mode_idx].protocol);
		return;
	}

	if ((di->adp_mode != 0 && (di->adp_mode & g_adp_mode_map[index]) == 0) &&
		(di->mode_para[di->mode_idx].force == DISABLE)) {
		charge_mode_set_result(&di->mode_para[di->mode_idx], CHARGE_MODE_RESULT_SUCC, CHARGE_MODE_ADP_UNSUPPORT);
		hwlog_err("adapter not support mode %s\n", di->mode_para[di->mode_idx].mode);
	}
}

static void charge_mode_init_para(struct charge_mode_dev *di)
{
	di->mode_idx = INVALID_INDEX;
	di->start_time = 0;
	di->curr_time = 0;
	di->adp_mode = 0;
	di->ping_result = 0;
	di->temp_err_flag = false;
	di->voltage_invalid_flag = false;
}

static void charge_mode_state_reset(struct charge_mode_dev *di)
{
	charge_mode_init_para(di);

	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ADAPTER_PROTOCOL, POWER_IF_SYSFS_ADAPTER_PROTOCOL, SYSFS_PROTOCOL_DEFAULT);
	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL, POWER_IF_SYSFS_ENABLE_CHARGER, DISABLE);

	/* sleep 2 seconds for stop direct charge */
	(void)power_msleep(DT_MSLEEP_2S, 0, NULL);
	power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_ALL, POWER_IF_SYSFS_ENABLE_CHARGER, ENABLE);
}

static void charge_mode_monitor(struct work_struct *work)
{
	struct charge_mode_dev *di = NULL;
	di = container_of(work, struct charge_mode_dev, test_work.work);

	di->curr_time = ktime_to_ms(ktime_get_boottime());

	if (charge_mode_caculate(di))
		di->mode_para[di->mode_idx].result = CHARGE_MODE_RESULT_SUCC;
	else
		charge_mode_set_result(&di->mode_para[di->mode_idx], CHARGE_MODE_RESULT_INIT, CHARGE_MODE_SUB_INIT);

	charge_mode_update_result(di);

	if (charge_mode_jump_mode(di)) {
		di->mode_idx++;
		di->adp_mode = 0;
		di->ping_result = 0;
		if (di->mode_idx >= di->mode_num) {
			charge_mode_state_reset(di);
			return;
		}

		if (charge_mode_select_protocol(di)) {
			charge_mode_state_reset(di);
			return;
		}

		charge_mode_select_mode(di->mode_para[di->mode_idx].mode);

		di->start_time = ktime_to_ms(ktime_get_boottime());
	}

	schedule_delayed_work(&di->test_work, msecs_to_jiffies(CHARGE_MODE_WORK_TIME));
}

static void charge_mode_start(struct charge_mode_dev *di)
{
	charge_mode_set_all_result(di, CHARGE_MODE_RESULT_INIT, CHARGE_MODE_SUB_INIT);
	charge_mode_init_para(di);
	direct_charge_set_stage_status_default();

	if (di->delay_time)
		power_msleep(di->delay_time, 0, NULL);
	hwlog_info("charge mode test start\n");

	cancel_delayed_work_sync(&di->test_work);
	schedule_delayed_work(&di->test_work, 0);
}

static int charge_mode_result(struct charge_mode_dev *di, char *buf)
{
	int i, k, ret;

	k = 0;
	for (i = 0; i < di->mode_num; i++) {
		ret = sprintf_s(buf + k, PAGE_SIZE, "%s,%s,%d,%d;",
			di->mode_para[i].protocol, di->mode_para[i].mode, di->mode_para[i].result, di->mode_para[i].sub_result);
		if (ret == INVALID_RESULT)
			return CHARGE_MODE_FAILURE;

		k += ret;
	}
	return k;
}

static int charge_mode_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct charge_mode_dev *di = NULL;
	di = container_of(nb, struct charge_mode_dev, charge_mode_nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_DC_PING_FAIL:
		di->ping_result = CHARGE_MODE_FAILURE;
		break;
	case POWER_NE_DC_ADAPTER_MODE:
		di->adp_mode = *((int *)data);
		break;
	case POWER_NE_DC_TEMP_ERR:
		di->temp_err_flag = true;
		break;
	case POWER_NE_DC_VOLTAGE_INVALID:
		di->voltage_invalid_flag = true;
		break;
	case POWER_NE_DC_CHECK_SUCC:
		di->temp_err_flag = false;
		di->voltage_invalid_flag = false;
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

#if CONFIG_SYSFS
static ssize_t charge_mode_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t charge_mode_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t conut);

static struct power_sysfs_attr_info charge_mode_sysfs_field_tbl[] = {
	power_sysfs_attr_wo(charge_mode, 0200, CHARGE_MODE_SYSFS_START, start),
	power_sysfs_attr_ro(charge_mode, 0440, CHARGE_MODE_SYSFS_RESULT, result),
};

#define CHARGE_MODE_SYSFS_ATTRS_SIZE ARRAY_SIZE(charge_mode_sysfs_field_tbl)

static struct attribute *charge_mode_sysfs_attrs[CHARGE_MODE_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group charge_mode_sysfs_attr_group = {
	.attrs = charge_mode_sysfs_attrs,
};

static ssize_t charge_mode_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct charge_mode_dev *di = dev_get_drvdata(dev);
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		charge_mode_sysfs_field_tbl, CHARGE_MODE_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case CHARGE_MODE_SYSFS_RESULT:
		return charge_mode_result(di, buf);
	default:
		break;
	}

	return 0;
}

static ssize_t charge_mode_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct charge_mode_dev *di = dev_get_drvdata(dev);
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		charge_mode_sysfs_field_tbl, CHARGE_MODE_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case CHARGE_MODE_SYSFS_START:
		charge_mode_start(di);
		break;
	default:
		break;
	}

	return count;
}

static void charge_mode_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(charge_mode_sysfs_attrs,
		charge_mode_sysfs_field_tbl, CHARGE_MODE_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "charge_mode_tst",
		dev, &charge_mode_sysfs_attr_group);
}

static void charge_mode_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "charge_mode_tst",
		dev, &charge_mode_sysfs_attr_group);
}
#else
static inline void charge_mode_sysfs_create_group(struct device *dev)
{
	return 0;
}

static inline void charge_mode_sysfs_remove_group(struct device *dev)
{
}
#endif

static ssize_t charge_mode_set_delay_time_show(void *dev_data,
	char *buf, size_t size)
{
	struct charge_mode_dev *l_dev = dev_data;

	if (!buf || !l_dev) {
		hwlog_err("buf or l_dev is null\n");
		return scnprintf(buf, size, "buf or l_dev is null\n");
	}

	return scnprintf(buf, size, "delay_time=%d\n", l_dev->delay_time);
}


static ssize_t charge_mode_set_delay_time_store(void *dev_data,
	const char *buf, size_t size)
{
	struct charge_mode_dev *l_dev = dev_data;
	int delay_time = 0;

	if (!buf || !l_dev) {
		hwlog_err("buf or l_dev is null\n");
		return -EINVAL;
	}

	if (sscanf_s(buf, "%d ", &delay_time) != DELAY_TIME_READY_OK) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	l_dev->delay_time = delay_time;
	hwlog_info("delay_time=%u\n", l_dev->delay_time);

	return size;
}

static int charge_mode_parse_protocol(struct charge_mode_dev *di, unsigned int row, const char *buf, int offset)
{
	if (strncpy_s(di->mode_para[row].protocol, PROTOCOL_LEN_MAX, buf, strlen(buf)))
		return CHARGE_MODE_FAILURE;

	return CHARGE_MODE_SUCCESS;
}

static int charge_mode_parse_mode(struct charge_mode_dev *di, unsigned int row, const char *buf, int offset)
{
	if (strncpy_s(di->mode_para[row].mode, CHARGE_MODE_LEN_MAX, buf, strlen(buf)))
		return CHARGE_MODE_FAILURE;

	di->mode_num++;
	return CHARGE_MODE_SUCCESS;
}

static int charge_mode_parse_int(struct charge_mode_dev *di, unsigned int row, const char *buf, int offset)
{
	int data = 0;

	if (kstrtoint(buf, POWER_BASE_DEC, &data))
		return CHARGE_MODE_FAILURE;

	*(&di->mode_para[row].ibat_th + offset) = data;

	return CHARGE_MODE_SUCCESS;
}

static void charge_mode_parse_dts(struct device_node *np, struct charge_mode_dev *di)
{
	int i, row, col, array_len, ret;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"test_para", CHARGE_MODE_NUM_MAX, CHARGE_MODE_PARA_TOTAL);
	if (array_len < 0)
		return;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "test_para", i, &tmp_string))
			continue;

		row = i / CHARGE_MODE_PARA_TOTAL;
		col = i % CHARGE_MODE_PARA_TOTAL;

		ret = g_parse_tbl[col](di, row, tmp_string, (col - CHARGE_MODE_IBAT_TH));
		if (ret)
			return;

		if (col == CHARGE_MODE_TIME)
			di->mode_para[row].time *= MSEC_PER_SEC;
	}
}

static int charge_mode_probe(struct platform_device *pdev)
{
	struct charge_mode_dev *di = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	charge_mode_parse_dts(np, di);

	di->charge_mode_nb.notifier_call = charge_mode_notifier_call;
	power_dbg_ops_register("charge_mode_tst", "delay", (void *)di,
		charge_mode_set_delay_time_show, charge_mode_set_delay_time_store);
	ret = power_event_bnc_register(POWER_BNT_DC, &di->charge_mode_nb);
	if (ret) {
		hwlog_err("charge mode register notify failed\n");
		goto register_notify_fail;
	}

	INIT_DELAYED_WORK(&di->test_work, charge_mode_monitor);
	platform_set_drvdata(pdev, di);
	charge_mode_sysfs_create_group(di->dev);

	return 0;

register_notify_fail:
	kfree(di);
	return ret;
}

static int charge_mode_remove(struct platform_device *pdev)
{
	struct charge_mode_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	cancel_delayed_work(&di->test_work);
	charge_mode_sysfs_remove_group(di->dev);
	power_event_bnc_unregister(POWER_BNT_DC, &di->charge_mode_nb);
	kfree(di);

	return 0;
}

static const struct of_device_id charge_mode_match_table[] = {
	{
		.compatible = "huawei,charge_mode_test",
		.data = NULL,
	},
	{},
};

static struct platform_driver charge_mode_driver = {
	.probe = charge_mode_probe,
	.remove = charge_mode_remove,
	.driver = {
		.name = "huawei,charge_mode_test",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(charge_mode_match_table),
	},
};

static int __init charge_mode_init(void)
{
	return platform_driver_register(&charge_mode_driver);
}

static void __exit charge_mode_exit(void)
{
	return platform_driver_unregister(&charge_mode_driver);
}

device_initcall_sync(charge_mode_init);
module_exit(charge_mode_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("rt test monitor driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
