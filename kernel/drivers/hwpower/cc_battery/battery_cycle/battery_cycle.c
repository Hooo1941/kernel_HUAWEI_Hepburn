// SPDX-License-Identifier: GPL-2.0
/*
 * battery_cycle.c
 *
 * driver adapter for cycle count store
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

#include "battery_cycle.h"
#include <securec.h>
#include <linux/delay.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/batt_info_pub.h>
#include <chipset_common/hwpower/battery/battery_soh.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_bigdata.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include "../battery_core.h"

#define HWLOG_TAG battery_cycle
HWLOG_REGIST();

static struct bat_cycle_device *g_bat_cycle_device;

static int bat_cycle_get_nv_index_prev(int now_index)
{
	int index = (now_index - 1) % BAT_CYC_NV_REC_CNT;
	return (index >= 0) ? index : index + BAT_CYC_NV_REC_CNT;
}

static int bat_cycle_get_nv_index_next(int now_index)
{
	int index = (now_index + 1) % BAT_CYC_NV_REC_CNT;
	return (index >= 0) ? index : index + BAT_CYC_NV_REC_CNT;
}

static int bat_cycle_get_nv_index_min_cyc(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int min_cyc_index = 0;
	int i;

	if (!di) {
		hwlog_err("[%s]g_bat_cycle_device is null\n", __func__);
		return -1;
	}
	for (i = 1; i < BAT_CYC_NV_REC_CNT; i++) {
		if (di->data.records[i].cycle < di->data.records[min_cyc_index].cycle)
			min_cyc_index = i;
	}
	return min_cyc_index;
}

static int bat_cycle_send_extdmd_data(struct imonitor_eventobj *obj, void *data)
{
	struct bat_cycle_nv_data *nvdata = data;

	if (!obj || !data) {
		hwlog_err("send_bigdata: obj or data is null\n");
		return -EPERM;
	}

	(void)power_bigdata_send_integer(obj, "CycVer", nvdata->ver);
	(void)power_bigdata_send_integer(obj, "CycIndex", nvdata->index);
	(void)power_bigdata_send_string(obj, "CycSN0", nvdata->records[0].sn);
	(void)power_bigdata_send_integer(obj, "CycCnt0", nvdata->records[0].cycle);
	(void)power_bigdata_send_string(obj, "CycSN1", nvdata->records[1].sn);
	(void)power_bigdata_send_integer(obj, "CycCnt1", nvdata->records[1].cycle);
	(void)power_bigdata_send_string(obj, "CycSN2", nvdata->records[2].sn);
	(void)power_bigdata_send_integer(obj, "CycCnt2", nvdata->records[2].cycle);

	return 0;
}

static int bat_cycle_save_to_nv(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int ret;

	if (!di) {
		hwlog_err("[%s]g_bat_cycle_device is null\n", __func__);
		return -1;
	}

	mutex_lock(&(di->nv_write_lock));
	ret = power_nv_write(POWER_NV_BATCYC, &(di->data), sizeof(di->data));
	mutex_unlock(&(di->nv_write_lock));
	if (ret)
		return ret;

	hwlog_info("data save to nv:\n");
	hwlog_info("ver:%d, last_cycle:%d, index:%d\n", di->data.ver, di->data.last_cycle, di->data.index);
	hwlog_info("[0]sn:%s, cycle:%d\n", di->data.records[0].sn, di->data.records[0].cycle);
	hwlog_info("[1]sn:%s, cycle:%d\n", di->data.records[1].sn, di->data.records[1].cycle);
	hwlog_info("[2]sn:%s, cycle:%d\n", di->data.records[2].sn, di->data.records[2].cycle);

	power_bigdata_report(POWER_BIGDATA_TYPE_BAT_CYCLE, bat_cycle_send_extdmd_data, &(di->data));
	return 0;
}

/* this func only work on Qcom platform */
static int bat_cycle_get_coul_cycle_count(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int type;
	int cycle;
	if (!di) {
		hwlog_err("[%s]g_bat_cycle_device is null\n", __func__);
		return -1;
	}

	type = (di->battery_type == BAT_CYC_BATTERY_TYPE_2_BATT) ? COUL_TYPE_1S2P : COUL_TYPE_MAIN;
	cycle = coul_interface_get_battery_cycle(type);
	hwlog_info("get coul cycle, type:%d, cycle:%d", type, cycle);
	return cycle * BAT_CYC_UNIT;
}

static int bat_cycle_update_cycle_count(int cycle)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int cycle_d;

	if (!di || !di->initialized) {
		hwlog_err("[%s] cycle drv is null or not initialized\n", __func__);
		return -1;
	}

	di->coul_cycle = cycle * BAT_CYC_UNIT;
	cycle_d = di->coul_cycle - di->data.last_cycle;
	if (cycle_d != 0)
		hwlog_info("cycle start:%d, now:%d\n", di->data.last_cycle, di->coul_cycle);

	if (cycle_d >= BAT_CYC_UPDATE_INC_THR) {
		hwlog_info("cycle increment > %d, update to NV\n", BAT_CYC_UPDATE_INC_THR / BAT_CYC_UNIT);
		di->data.records[di->data.index].cycle += cycle_d;
		di->data.last_cycle = di->coul_cycle;
		bat_cycle_save_to_nv();
	} else if (cycle_d < 0) {
		hwlog_warn("cycle decreased! reset start to %d\n", di->coul_cycle);
		di->data.last_cycle = di->coul_cycle;
		bat_cycle_save_to_nv();
	}

	return 0;
}

static int bat_cycle_get_cycle_count(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	if (!di || !di->initialized) {
		hwlog_err("[%s] cycle drv is null or not initialized\n", __func__);
		return -1;
	}
	return di->data.records[di->data.index].cycle / BAT_CYC_UNIT;
}

static struct bat_core_ops bat_cycle_core_ops = {
	.type_name = "bat_cycle",
	.get_cycle_count = bat_cycle_get_cycle_count,
	.set_cycle_count = bat_cycle_update_cycle_count
};

static int bat_cycle_nv_clear(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int ret = 0;

	if (!di || !di->initialized) {
		hwlog_err("[%s] cycle drv is null or not initialized\n", __func__);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return -1;
	}

	if (di->batt_has_sn) {
		hwlog_info("batt has SN, ignore FT operation\n");
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_NO_NEED;
		return 0;
	}

	ret = strcpy_s(di->data.records[di->data.index].sn, BAT_CYC_SN_STR_LEN, BAT_CYC_SN_RM_STR);
	if (ret != EOK) {
		hwlog_err("[%s] strcpy_s failed!\n", __func__);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return ret;
	}
	di->data.index = bat_cycle_get_nv_index_next(di->data.index);
	di->data.records[di->data.index].cycle = 0;
	ret = strcpy_s(di->data.records[di->data.index].sn, BAT_CYC_SN_STR_LEN, BAT_CYC_SN_NO_CT_STR);
	if (ret != EOK) {
		hwlog_err("[%s] strcpy_s failed!\n", __func__);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return ret;
	}

	ret = bat_cycle_save_to_nv();
	if (ret) {
		hwlog_err("[%s] write cycle NV fail\n", __func__);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return ret;
	}

	if (di->ft_restore_count > 0)
		di->ft_restore_count--;
	di->ft_oper_res = BAT_CYC_FT_OPER_RES_SUCCESS;
	return 0;
}

static int bat_cycle_nv_restore(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int ret = 0;
	int8_t prev_index = -1;

	if (!di || !di->initialized) {
		hwlog_err("[%s] cycle drv is null or not initialized\n", __func__);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return -1;
	}

	if (di->ft_restore_count > 0) {
		hwlog_err("Cycle already restored.\n");
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FORBID_RST_TWICE;
		return -1;
	}

	if (di->batt_has_sn) {
		hwlog_info("batt has SN, ignore FT operation\n");
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_NO_NEED;
		return 0;
	}

	if (di->data.records[di->data.index].cycle > BAT_CYC_FT_RST_MAX_CYCLE * BAT_CYC_UNIT) {
		hwlog_err("current cycle count is large:%d, restore not allow.\n",
			di->data.records[di->data.index].cycle);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_CURR_CYC_TOO_LARGE;
		return -1;
	}

	prev_index = bat_cycle_get_nv_index_prev(di->data.index);
	if (strcmp(di->data.records[prev_index].sn, BAT_CYC_SN_RM_STR) != 0) {
		hwlog_err("No valid cycle backup, cannot restore.\n");
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_NO_VALID_BACKUP;
		return -1;
	}
	di->data.index = prev_index;
	ret = strcpy_s(di->data.records[di->data.index].sn, BAT_CYC_SN_STR_LEN, BAT_CYC_SN_NO_CT_STR);
	if (ret != EOK) {
		hwlog_err("[%s] strcpy_s failed!\n", __func__);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return ret;
	}

	ret = bat_cycle_save_to_nv();
	if (ret) {
		hwlog_err("[%s] write cycle nv fail\n", __func__);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return ret;
	}
	di->ft_restore_count++;
	di->ft_oper_res = BAT_CYC_FT_OPER_RES_SUCCESS;
	return 0;
}

static void bat_cycle_send_ft_uevent(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int cycle = bat_cycle_get_cycle_count();
	char data[BAT_CYC_EVENT_NOTIFY_SIZE];
	int len;

	if (!di) {
		hwlog_err("[%s]g_bat_cycle_device is null\n", __func__);
		return;
	}

	len = snprintf_s(data, BAT_CYC_EVENT_NOTIFY_SIZE, BAT_CYC_EVENT_NOTIFY_SIZE,
		"cycle:%d,ft_restored:%d", cycle, (di->ft_restore_count > 0) ? 1 : 0);
	if (len <= 0) {
		hwlog_err("[%s] snprintf_s failed: %d\n", __func__, len);
		return;
	}
	bsoh_uevent_rcv(BSOH_EVT_BAT_CYC_FT_ACT, data);
}

#ifdef CONFIG_SYSFS
static ssize_t bcyc_battery_removed_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int len;
	int flag = power_platform_is_battery_removed();
	if (flag)
		goto flag_result_show;

	if (!di || !di->initialized) {
		hwlog_err("[%s] cycle drv is null or not initialized\n", __func__);
		return -1;
	}
	flag = di->battery_removed;

flag_result_show:
	len = snprintf_s(buff, BAT_CYC_SYSFS_BUFF_SIZE, BAT_CYC_SYSFS_BUFF_SIZE, "%d", flag);
	return len;
}

static ssize_t bcyc_ft_remind_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int flag;

	if (!di || !di->initialized) {
		hwlog_err("[%s] cycle drv is null or not initialized\n", __func__);
		return -1;
	}
	flag = (bat_cycle_get_coul_cycle_count() < BAT_CYC_FT_REMIND_CYC_THR) ? 1 : 0;

	hwlog_info("ft remind mode is %d", di->ft_remind_mode);
	if ((di->ft_remind_mode == 0) && di->batt_has_sn)
		flag = 0;

	bat_cycle_send_ft_uevent();
	return snprintf_s(buff, BAT_CYC_SYSFS_BUFF_SIZE, BAT_CYC_SYSFS_BUFF_SIZE, "%d", flag);
}

static const struct bat_cycle_ft_operate bat_cycle_ft_operate_table[BAT_CYC_FT_OPER_END] = {
	[BAT_CYC_FT_OPER_CLR]	= {
		.type = "battery_cycle_clear",
		.operate = bat_cycle_nv_clear,
	},
	[BAT_CYC_FT_OPER_RST]	= {
		.type = "battery_cycle_restore",
		.operate = bat_cycle_nv_restore,
	},
};

static ssize_t bcyc_ft_operate_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int ret = 0;
	int i;

	hwlog_info("Receive FT oper: %s\n", buf);
	if (!di || !di->initialized) {
		hwlog_err("[%s] cycle drv is null or not initialized\n", __func__);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return -1;
	}

	di->ft_oper_res = 0;
	for (i = 0; i < BAT_CYC_FT_OPER_END; i++) {
		if (sysfs_streq(buf, bat_cycle_ft_operate_table[i].type))
			break;
	}

	if (i >= BAT_CYC_FT_OPER_END) {
		hwlog_err("Unknown FT operation:%s\n", buf);
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return -1;
	}

	if (!bat_cycle_ft_operate_table[i].operate) {
		di->ft_oper_res = BAT_CYC_FT_OPER_RES_FUNC_ERR;
		return -1;
	}

	ret = bat_cycle_ft_operate_table[i].operate();
	if (ret) {
		hwlog_err("FT operate fail: %s, ret=%d\n", buf, ret);
		return ret;
	}

	hwlog_info("FT operate success: %s", buf);
	bat_cycle_send_ft_uevent();
	return count;
}

static ssize_t bcyc_ft_operate_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int len;

	if (!di || !di->initialized) {
		hwlog_err("[%s] cycle drv is null or not initialized\n", __func__);
		return snprintf_s(buff, BAT_CYC_SYSFS_BUFF_SIZE, BAT_CYC_SYSFS_BUFF_SIZE, "%d",
			BAT_CYC_FT_OPER_RES_FUNC_ERR);
	}

	len = snprintf_s(buff, BAT_CYC_SYSFS_BUFF_SIZE, BAT_CYC_SYSFS_BUFF_SIZE, "%d", di->ft_oper_res);
	return len;
}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
/*
 * input format:<index> <ver> <last_cycle> <SN0> <cyc0> <SN1> <cyc1> <SN2> <cyc2>
 * e.g.:0 1 500 A_BATTERY_SN 600 B_BATTERY_SN 200 C_BATTERY_SN 100
 */
static ssize_t bcyc_set_nv_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int index = 0;
	char sn_0[BAT_CYC_SN_STR_LEN] = { 0 };
	char sn_1[BAT_CYC_SN_STR_LEN] = { 0 };
	char sn_2[BAT_CYC_SN_STR_LEN] = { 0 };
	int ret = 0;

	if (!di) {
		hwlog_err("[%s] cycle drv is null\n", __func__);
		return -1;
	}
	ret = sscanf_s(buf, "%d %d %d %16s %d %16s %d %16s %d",
		&index, &(di->data.ver), &(di->data.last_cycle),
		sn_0, BAT_CYC_SN_STR_LEN, &(di->data.records[0].cycle),
		sn_1, BAT_CYC_SN_STR_LEN, &(di->data.records[1].cycle),
		sn_2, BAT_CYC_SN_STR_LEN, &(di->data.records[2].cycle));
	if (ret != 9) {
		hwlog_err("unable to parse input:%s\n, ret=%d", buf, ret);
		return -EINVAL;
	}
	di->data.index = index;
	ret = strncpy_s(di->data.records[0].sn, BAT_CYC_SN_BUFF_LEN, sn_0, BAT_CYC_SN_LEN);
	ret += strncpy_s(di->data.records[1].sn, BAT_CYC_SN_BUFF_LEN, sn_1, BAT_CYC_SN_LEN);
	ret += strncpy_s(di->data.records[2].sn, BAT_CYC_SN_BUFF_LEN, sn_2, BAT_CYC_SN_LEN);
	if (ret != EOK) {
		hwlog_err("[%s] strncpy_s failed!\n", __func__);
		return ret;
	}
	bat_cycle_save_to_nv();
	return count;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static DEVICE_ATTR_RO(bcyc_battery_removed);
static DEVICE_ATTR_RO(bcyc_ft_remind);
static DEVICE_ATTR_RW(bcyc_ft_operate);
#ifdef CONFIG_HUAWEI_POWER_DEBUG
static DEVICE_ATTR_WO(bcyc_set_nv);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static const struct attribute *g_bat_cycle_attrs[] = {
	&dev_attr_bcyc_battery_removed.attr,
	&dev_attr_bcyc_ft_remind.attr,
	&dev_attr_bcyc_ft_operate.attr,
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	&dev_attr_bcyc_set_nv.attr,
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
	NULL,
};
static int bat_cycle_sysfs_create_files(struct bat_cycle_device *di)
{
	return sysfs_create_files(&di->dev->kobj, g_bat_cycle_attrs);
}

static inline void bat_cycle_sysfs_remove_files(struct bat_cycle_device *di)
{
	sysfs_remove_files(&di->dev->kobj, g_bat_cycle_attrs);
}
#else
static inline int bat_cycle_sysfs_create_files(struct bat_cycle_device *di)
{
	return 0;
}

static inline void bat_cycle_sysfs_remove_files(struct bat_cycle_device *di)
{}
#endif /* CONFIG_SYSFS */

static void bat_cycle_nv_init_ct(struct bat_cycle_nv_data *nvdata)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int min_cyc_index = 0;
	int ret;
	int i;

	if (!di) {
		hwlog_err("[%s]g_bat_cycle_device is null\n", __func__);
		return;
	}

	for (i = 0; i < BAT_CYC_NV_REC_CNT; i++) {
		if (strcmp(di->batt_sn, nvdata->records[i].sn) == 0) {
			hwlog_info("found same SN record at index %d\n", i);
			nvdata->index = i;
			return;
		}
	}
	min_cyc_index = bat_cycle_get_nv_index_min_cyc();
	if (min_cyc_index < 0) {
		hwlog_err("failed to get NV min cycle index");
		return;
	}
	hwlog_info("no SN record found, create at index %d\n", min_cyc_index);
	nvdata->index = min_cyc_index;
	ret = strcpy_s(nvdata->records[min_cyc_index].sn, BAT_CYC_SN_STR_LEN, di->batt_sn);
	if (ret != EOK)
		hwlog_err("[%s] strcpy_s failed!\n", __func__);
	nvdata->records[min_cyc_index].cycle = 0;
}

static void bat_cycle_nv_init_noct(struct bat_cycle_nv_data *nvdata)
{
	int ret = 0;
	int cycle = 0;
	if (strcmp(nvdata->records[nvdata->index].sn, BAT_CYC_SN_NO_CT_STR) != 0) {
		cycle = nvdata->records[nvdata->index].cycle;
		nvdata->index = bat_cycle_get_nv_index_next(nvdata->index);
		nvdata->records[nvdata->index].cycle = cycle;
		ret = strcpy_s(nvdata->records[nvdata->index].sn, BAT_CYC_SN_STR_LEN, BAT_CYC_SN_NO_CT_STR);
		if (ret != EOK)
			hwlog_err("[%s] strcpy_s failed!\n", __func__);
		hwlog_info("current record has SN, but battery has no SN, copy the record\n");
	}
}

static void bat_cycle_nv_init_ver(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;

	if (!di) {
		hwlog_err("[%s]g_bat_cycle_device is null\n", __func__);
		return;
	}

	if (di->data.ver <= 0) {
		hwlog_info("ver is 0, first time installed\n");
		di->data.ver = 1;
		di->data.index = 0;
		di->data.records[0].cycle = di->coul_cycle;
		di->data.last_cycle = di->coul_cycle;
		if (strcpy_s(di->data.records[0].sn, BAT_CYC_SN_STR_LEN, di->batt_sn) != EOK)
			hwlog_err("[%s] strcpy_s failed!\n", __func__);
	} else {
		if (di->batt_has_sn)
			bat_cycle_nv_init_ct(&(di->data));
		else
			bat_cycle_nv_init_noct(&(di->data));
	}
}

static int bat_cycle_nv_init(void)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	int ret = 0;

	if (!di) {
		hwlog_err("[%s]g_bat_cycle_device is null\n", __func__);
		return -1;
	}

	if (di->data.last_cycle > di->coul_cycle)
		di->data.last_cycle = di->coul_cycle;

	bat_cycle_nv_init_ver();
	ret = bat_cycle_save_to_nv();
	hwlog_info("battery_cycle NV init result: %d\n", ret);
	return ret;
}

static int bat_cycle_get_battery_sn(char *buff)
{
	struct bat_cycle_device *di = g_bat_cycle_device;
	char res[BAT_CYC_SN_BUFF_LEN] = { 0 };
	int ret;

	if (!di) {
		hwlog_err("[%s]g_bat_cycle_device is null\n", __func__);
		return -1;
	}

	switch (di->battery_type) {
	case BAT_CYC_BATTERY_TYPE_1_BATT:
		ret = get_battery_identifier(res, BAT_CYC_SN_BUFF_LEN, 0, BAT_CYC_SN_LEN);
		if (ret)
			hwlog_err("[%s] get battery identifier fail\n", __func__);
		break;
	case BAT_CYC_BATTERY_TYPE_2_BATT:
		ret = get_battery_group_sn(res, BAT_CYC_SN_BUFF_LEN);
		if (ret)
			hwlog_err("[%s] get battery group sn fail\n", __func__);
		break;
	default:
		hwlog_err("invalid battery type!\n");
		break;
	}
	/* batt_info driver setup earlier, get SN fail means no SN, set to empty string */
	return strncpy_s(buff, BAT_CYC_SN_BUFF_LEN, res, BAT_CYC_SN_LEN);
}

static void bat_cycle_init_work(struct work_struct *work)
{
	struct bat_cycle_device *di = container_of(work, struct bat_cycle_device, init_work.work);
	if (di->initialized)
		return;

	if (power_nv_read(POWER_NV_BATCYC, &(di->data), sizeof(di->data))) {
		hwlog_err("read NV fail\n", __func__);
		goto delay_retry;
	}

	di->coul_cycle = bat_cycle_get_coul_cycle_count();
	if (di->coul_cycle < 0) {
		hwlog_err("get coul cycle fail\n", __func__);
		goto delay_retry;
	}

	bat_cycle_get_battery_sn(di->batt_sn);
	hwlog_info("battery SN:%s\n", di->batt_sn);
	di->batt_has_sn = (strlen(di->batt_sn) > 0) ? 1 : 0;
	if (!di->batt_has_sn) {
		if (strcpy_s(di->batt_sn, BAT_CYC_SN_STR_LEN, BAT_CYC_SN_NO_CT_STR))
			hwlog_err("[%s] strcpy_s failed!\n", __func__);
		else
			hwlog_info("battery has no SN, use default:%s", BAT_CYC_SN_NO_CT_STR);
	}

	di->battery_removed = (di->coul_cycle <= (1 * BAT_CYC_UNIT)) ? 1 : 0;
	hwlog_info("cycle start = %d, battery_removed = %d\n", di->data.last_cycle, di->battery_removed);

	if (bat_cycle_nv_init()) {
		hwlog_err("[%s] NV init fail\n", __func__);
		goto delay_retry;
	}

	if (bat_core_ops_register(&bat_cycle_core_ops)) {
		hwlog_err("[%s] bat_core_ops register fail\n", __func__);
		goto delay_retry;
	}

	di->initialized = 1;
	hwlog_info("battery_cycle init succ\n");
	return;

delay_retry:
	di->init_retry_times++;
	if (di->init_retry_times > BAT_CYC_INIT_MAX_RETRY) {
		hwlog_err("bat cycle init failed after %d retries!", BAT_CYC_INIT_MAX_RETRY);
		bat_core_ops_unregister(&bat_cycle_core_ops);
		return;
	}
	hwlog_info("bat cycle init fail, retry: %d\n", di->init_retry_times);
	queue_delayed_work(system_freezable_power_efficient_wq, &di->init_work,
		msecs_to_jiffies(BAT_CYC_INIT_RETRY_DELAY));
}

static int bat_cycle_dts_parse(struct device_node *np, struct bat_cycle_device *di)
{
	int ret = power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ft_remind_mode", &(di->ft_remind_mode), 0);
	ret += power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "battery_type", &(di->battery_type), 1);
	return ret;
}

static int bat_cycle_probe(struct platform_device *pdev)
{
	struct bat_cycle_device *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	if (bat_cycle_dts_parse(pdev->dev.of_node, di)) {
		hwlog_err("[%s] parse dts error\n", __func__);
		goto free_mem;
	}
	if (bat_cycle_sysfs_create_files(di)) {
		hwlog_err("[%s] create device node error\n", __func__);
		goto free_mem;
	}
	mutex_init(&di->nv_write_lock);
	INIT_DELAYED_WORK(&di->init_work, bat_cycle_init_work);
	queue_delayed_work(system_freezable_power_efficient_wq, &di->init_work, 0);

	g_bat_cycle_device = di;
	platform_set_drvdata(pdev, di);
	return 0;

free_mem:
	kfree(di);
	return -EPERM;
}

static int bat_cycle_remove(struct platform_device *pdev)
{
	struct bat_cycle_device *di = platform_get_drvdata(pdev);
	if (!di)
		return -ENODEV;

	hwlog_info("drv remove, di->initialized is %d", di->initialized);
	if (di->initialized) {
		hwlog_info("drv remove, save data to nv");
		bat_cycle_save_to_nv();
	}

	platform_set_drvdata(pdev, NULL);
	cancel_delayed_work(&di->init_work);
	kfree(di);
	g_bat_cycle_device = NULL;
	return 0;
}

static const struct of_device_id bat_cycle_match_table[] = {
	{
		.compatible = "huawei,battery_cycle",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_cycle_driver = {
	.probe = bat_cycle_probe,
	.remove = bat_cycle_remove,
	.driver = {
		.name = "huawei,battery_cycle",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_cycle_match_table),
	},
};

static int __init bat_cycle_init(void)
{
	return platform_driver_register(&bat_cycle_driver);
}

static void __exit bat_cycle_exit(void)
{
	platform_driver_unregister(&bat_cycle_driver);
}

device_initcall_sync(bat_cycle_init);
module_exit(bat_cycle_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei battery cycle driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
