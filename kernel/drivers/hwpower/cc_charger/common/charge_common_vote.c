// SPDX-License-Identifier: GPL-2.0
/*
 * charge_common_vote.c
 *
 * vote interface of charge module
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

#include "securec.h"
#include <chipset_common/hwpower/charger/charge_common_vote.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_vote.h>

#define HWLOG_TAG chg_common_vote
HWLOG_REGIST();

#define MAX_BUFF_SIZE         256

static const char * const g_vote_client_name[] = {
	[VOTE_CLIENT_THERMAL] = "client: thermal",
	[VOTE_CLIENT_USER] = "client: user",
	[VOTE_CLIENT_JEITA] = "client: jeita",
	[VOTE_CLIENT_BASP] = "client: basp",
	[VOTE_CLIENT_WARM_WR] = "client: warm_wr",
	[VOTE_CLIENT_RT] = "client: rt",
	[VOTE_CLIENT_FCP] = "client: fcp",
	[VOTE_CLIENT_WLS] = "client: wls",
	[VOTE_CLIENT_FFC] = "client: ffc",
	[VOTE_CLIENT_SMT_BATT] = "client: smt_batt",
	[VOTE_CLIENT_IBUS_DETECT] = "client: ibus_detect",
	[VOTE_CLIENT_POWER_IF] = "client: power_if",
	[VOTE_CLIENT_RSMC] = "client: rsmc",
	[VOTE_CLIENT_DC] = "client: dc",
	[VOTE_CLIENT_CABLE] = "client: cable",
	[VOTE_CLIENT_ADAPTER] = "client: adapter",
	[VOTE_CLIENT_TIME] = "client: time",
	[VOTE_CLIENT_CABLE_TYPE] = "client: cable_type",
	[VOTE_CLIENT_CABLE_1ST_RES] = "client: cable_1st_res",
	[VOTE_CLIENT_CABLE_2ND_RES] = "client: cable_2nd_res",
	[VOTE_CLIENT_ADPT_IWATT] = "client: adpt_iwatt",
	[VOTE_CLIENT_ADPT_ANTIFAKE] = "client: adpt_antifake",
	[VOTE_CLIENT_ADPT_TIME] = "client: adpt_time",
	[VOTE_CLIENT_ADPT_MAX_CURR] = "client: adpt_max_curr",
};

static const int g_dc_ibus_table[] = {
	VOTE_CLIENT_CABLE,
	VOTE_CLIENT_ADAPTER,
	VOTE_CLIENT_TIME,
};

static const int g_dc_cable_table[] = {
	VOTE_CLIENT_CABLE_TYPE,
	VOTE_CLIENT_CABLE_1ST_RES,
	VOTE_CLIENT_CABLE_2ND_RES,
};

static const int g_dc_adpt_table[] = {
	VOTE_CLIENT_ADPT_IWATT,
	VOTE_CLIENT_ADPT_ANTIFAKE,
	VOTE_CLIENT_ADPT_TIME,
	VOTE_CLIENT_ADPT_MAX_CURR,
};

static const int g_buck_fcc_table[] = {
	VOTE_CLIENT_THERMAL,
	VOTE_CLIENT_USER,
	VOTE_CLIENT_JEITA,
	VOTE_CLIENT_SMT_BATT,
	VOTE_CLIENT_DC,
};

static const int g_buck_icl_table[] = {
	VOTE_CLIENT_THERMAL,
	VOTE_CLIENT_USER,
	VOTE_CLIENT_RT,
	VOTE_CLIENT_FCP,
	VOTE_CLIENT_WLS,
	VOTE_CLIENT_IBUS_DETECT,
	VOTE_CLIENT_POWER_IF,
	VOTE_CLIENT_RSMC,
	VOTE_CLIENT_DC,
};

static const int g_buck_vterm_table[] = {
	VOTE_CLIENT_USER,
	VOTE_CLIENT_JEITA,
	VOTE_CLIENT_BASP,
	VOTE_CLIENT_SMT_BATT,
};

static const int g_buck_iterm_table[] = {
	VOTE_CLIENT_USER,
	VOTE_CLIENT_FFC,
};

static const int g_buck_dis_table[] = {
	VOTE_CLIENT_USER,
	VOTE_CLIENT_JEITA,
	VOTE_CLIENT_WARM_WR,
};

struct chg_vote_table {
	unsigned int obj_index;
	const char *obj_name;
	unsigned int vote_type;
	const int *client_tbl;
	int client_tbl_size;
	power_vote_cb cb;
	void *data;
};

static const struct chg_vote_table g_buck_vote_tbl[] = {
	{ VOTE_OBJ_FCC, "BATT:fcc", POWER_VOTE_SET_MIN, g_buck_fcc_table,
		ARRAY_SIZE(g_buck_fcc_table), charge_vote_for_fcc, NULL },
	{ VOTE_OBJ_USB_ICL, "BATT:usb_icl", POWER_VOTE_SET_MIN, g_buck_icl_table,
		ARRAY_SIZE(g_buck_icl_table), charge_vote_for_usb_icl, NULL },
	{ VOTE_OBJ_VTERM, "BATT:vterm", POWER_VOTE_SET_MIN, g_buck_vterm_table,
		ARRAY_SIZE(g_buck_vterm_table), charge_vote_for_vterm, NULL },
	{ VOTE_OBJ_ITERM, "BATT:iterm", POWER_VOTE_SET_MAX, g_buck_iterm_table,
		ARRAY_SIZE(g_buck_iterm_table), charge_vote_for_iterm, NULL },
	{ VOTE_OBJ_DIS_CHG, "BATT:dis_chg", POWER_VOTE_SET_ANY, g_buck_dis_table,
		ARRAY_SIZE(g_buck_dis_table), charge_vote_for_dis_chg, NULL },
};

static const struct chg_vote_table g_dc_vote_tbl[] = {
	{ VOTE_OBJ_IBUS, "DC:ibus", POWER_VOTE_SET_MIN, g_dc_ibus_table,
		ARRAY_SIZE(g_dc_ibus_table), NULL, NULL},
	{ VOTE_OBJ_CABLE_CURR, "DC:cable_curr", POWER_VOTE_SET_MIN, g_dc_cable_table,
		ARRAY_SIZE(g_dc_cable_table), NULL, NULL},
	{ VOTE_OBJ_ADPT_CURR, "DC:adpt_curr", POWER_VOTE_SET_MIN, g_dc_adpt_table,
		ARRAY_SIZE(g_dc_adpt_table), NULL, NULL},
};

#define CHG_VOTE_DATA_SIZE     4
struct chg_vote_data {
	const char *obj_type_name;
	const struct chg_vote_table *tbl;
	int tbl_size;
};

struct chg_vote_data g_chg_vote_data[VOTE_OBJ_TYPE_END] = {
	[VOTE_OBJ_TYPE_BUCK] = { "OBJ_buck", g_buck_vote_tbl, ARRAY_SIZE(g_buck_vote_tbl) },
	[VOTE_OBJ_TYPE_DC] = { "OBJ_dc", g_dc_vote_tbl, ARRAY_SIZE(g_dc_vote_tbl) },
};

struct vote_info {
	unsigned int obj;
	unsigned int client;
	bool en;
	int val;
};

static const char *chg_vote_get_client_name(const int *tbl, int tbl_size, unsigned int client)
{
	int i;

	if (client >= VOTE_CLIENT_END)
		return NULL;

	for (i = 0; i < tbl_size; i++) {
		if (tbl[i] == client)
			break;
	}
	if (i >= tbl_size)
		return NULL;

	return g_vote_client_name[client];
}

static int chg_vote_get_vote_tbl_idx(const struct chg_vote_table *tbl, int tbl_size, int obj)
{
	int i;

	for (i = 0; i < tbl_size; i++) {
		if (tbl[i].obj_index == obj)
			break;
	}
	if (i >= tbl_size)
		return -EPERM;

	return i;
}

static int chg_vote_operator_set(const struct chg_vote_table *tbl, int tbl_size, struct vote_info *info)
{
	int idx;
	const char *client_name;

	idx = chg_vote_get_vote_tbl_idx(tbl, tbl_size, info->obj);
	if (idx < 0)
		return -EPERM;

	client_name = chg_vote_get_client_name(tbl[idx].client_tbl, tbl[idx].client_tbl_size, info->client);
	if (!client_name)
		return -EPERM;

	return power_vote_set(tbl[idx].obj_name, client_name, info->en, info->val);
}

static void chg_vote_operator_clear(const struct chg_vote_table *tbl, int tbl_size, int obj)
{
	int idx, i;

	idx = chg_vote_get_vote_tbl_idx(tbl, tbl_size, obj);
	if (idx < 0)
		return;

	for (i = 0; i < tbl[idx].client_tbl_size; i++) {
		if (!g_vote_client_name[tbl[idx].client_tbl[i]])
			return;
		power_vote_set(tbl[idx].obj_name, g_vote_client_name[tbl[idx].client_tbl[i]], false, 0);
	}
}

static int chg_vote_operator_get_effective_result_locked(const struct chg_vote_table *tbl,
	int tbl_size, int obj, bool lock_flag)
{
	int idx;

	idx = chg_vote_get_vote_tbl_idx(tbl, tbl_size, obj);
	if (idx < 0)
		return -EPERM;

	return power_vote_get_effective_result_locked(tbl[idx].obj_name, lock_flag);
}

static bool chg_vote_operator_is_client_enabled_locked(const struct chg_vote_table *tbl,
	int tbl_size, int obj, int voter, bool lock_flag)
{
	int idx;
	const char *client_name;

	idx = chg_vote_get_vote_tbl_idx(tbl, tbl_size, obj);
	if (idx < 0)
		return false;

	client_name = chg_vote_get_client_name(tbl[idx].client_tbl, tbl[idx].client_tbl_size, voter);
	if (!client_name)
		return false;

	return power_vote_is_client_enabled_locked(tbl[idx].obj_name, client_name, lock_flag);
}

int chg_vote_set(unsigned int obj_type, unsigned int obj, unsigned int client, bool en, int val)
{
	struct chg_vote_data *data = g_chg_vote_data;
	struct vote_info info;

	if ((obj_type < VOTE_OBJ_TYPE_BEGIN) || (obj_type >= VOTE_OBJ_TYPE_END) ||
		!data[obj_type].tbl)
		return -EPERM;

	info.obj = obj;
	info.client = client;
	info.en = en;
	info.val = val;
	return chg_vote_operator_set(data[obj_type].tbl, data[obj_type].tbl_size, &info);
}

void chg_vote_clear(unsigned int obj_type, unsigned int obj)
{
	struct chg_vote_data *data = g_chg_vote_data;

	if ((obj_type < VOTE_OBJ_TYPE_BEGIN) || (obj_type >= VOTE_OBJ_TYPE_END) ||
		!data[obj_type].tbl)
		return;

	chg_vote_operator_clear(data[obj_type].tbl, data[obj_type].tbl_size, obj);
}

int chg_vote_get_effective_result_with_lock(unsigned int obj_type, unsigned int obj)
{
	struct chg_vote_data *data = g_chg_vote_data;

	if ((obj_type < VOTE_OBJ_TYPE_BEGIN) || (obj_type >= VOTE_OBJ_TYPE_END) ||
		!data[obj_type].tbl)
		return -EPERM;

	return chg_vote_operator_get_effective_result_locked(data[obj_type].tbl, data[obj_type].tbl_size, obj, true);
}

int chg_vote_get_effective_result_without_lock(unsigned int obj_type, unsigned int obj)
{
	struct chg_vote_data *data = g_chg_vote_data;

	if ((obj_type < VOTE_OBJ_TYPE_BEGIN) || (obj_type >= VOTE_OBJ_TYPE_END) ||
		!data[obj_type].tbl)
		return -EPERM;

	return chg_vote_operator_get_effective_result_locked(data[obj_type].tbl, data[obj_type].tbl_size, obj, false);
}

bool chg_vote_is_client_enabled_locked(unsigned int obj_type, unsigned int obj, unsigned int client, bool lock_flag)
{
	struct chg_vote_data *data = g_chg_vote_data;

	if ((obj_type < VOTE_OBJ_TYPE_BEGIN) || (obj_type >= VOTE_OBJ_TYPE_END) ||
		!data[obj_type].tbl)
		return false;

	return chg_vote_operator_is_client_enabled_locked(data[obj_type].tbl, data[obj_type].tbl_size, obj, client, lock_flag);
}

static ssize_t chg_vote_dbg_object_store(void *dev_data, const char *buf, size_t size)
{
	struct vote_info info = { 0 };
	struct chg_vote_data *data = (struct chg_vote_data *)dev_data;

	if (!buf || !data)
		return -EINVAL;

	/* 4:obj, client, en, val */
	if (sscanf_s(buf, "%u %u %u %d", &info.obj, &info.client, &info.en, &info.val) != CHG_VOTE_DATA_SIZE) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	chg_vote_operator_set(data->tbl, data->tbl_size, &info);
	return size;
}

static ssize_t chg_vote_dbg_object_show(void *dev_data, char *buf, size_t size)
{
	int i, val;
	char out_buf[MAX_BUFF_SIZE] = { 0 };
	char tmp_buf[MAX_BUFF_SIZE] = { 0 };
	struct chg_vote_data *data = (struct chg_vote_data *)dev_data;

	if (!buf || !data)
		return -EINVAL;

	for (i = 0; i < data->tbl_size; i++) {
		val = chg_vote_operator_get_effective_result_locked(data->tbl, data->tbl_size, data->tbl[i].obj_index, true);
		scnprintf(tmp_buf, MAX_BUFF_SIZE, "%s eff_result=%d\n", data->tbl[i].obj_name, val);
		if (strncat_s(out_buf, MAX_BUFF_SIZE - 1, tmp_buf, strlen(tmp_buf)) != EOK)
			return -EINVAL;
	}

	return scnprintf(buf, MAX_BUFF_SIZE, "%s", out_buf);
}

static int chg_vote_create_object(const struct chg_vote_table *tbl, int tbl_size)
{
	int i;

	for (i = 0; i < tbl_size; i++) {
		if (!tbl[i].obj_name)
			continue;

		if (power_vote_create_object(tbl[i].obj_name, tbl[i].vote_type, tbl[i].cb, tbl[i].data)) {
			hwlog_err("failed to create limit obj %s\n", tbl[i].obj_name);
			return -EPERM;
		}

		hwlog_info("succeed to create vote obj %s\n", tbl[i].obj_name);
	}
	return 0;
}

static void chg_vote_destory_object(const struct chg_vote_table *tbl, int tbl_size)
{
	int i;

	for (i = 0; i < tbl_size; i++) {
		if (!tbl[i].obj_name)
			continue;

		power_vote_destroy_object(tbl[i].obj_name);
	}
}

static int __init charge_common_vote_init(void)
{
	int i, ret;
	struct chg_vote_data *data = g_chg_vote_data;

	for (i = 0; i < VOTE_OBJ_TYPE_END; i++) {
		if (!data[i].tbl)
			continue;

		ret = chg_vote_create_object(data[i].tbl, data[i].tbl_size);
		if (ret) {
			hwlog_err("failed to create vote obj type %s\n", data[i].obj_type_name);
			continue;
		}
		power_dbg_ops_register("chg_common_vote", data[i].obj_type_name, &data[i],
			chg_vote_dbg_object_show, chg_vote_dbg_object_store);
	}

	return 0;
}

static void __exit charge_common_vote_exit(void)
{
	int i;
	struct chg_vote_data *data = g_chg_vote_data;

	for (i = 0; i < VOTE_OBJ_TYPE_END; i++) {
		if (!data[i].tbl)
			continue;

		chg_vote_destory_object(data[i].tbl, data[i].tbl_size);
	}
}

fs_initcall(charge_common_vote_init);
module_exit(charge_common_vote_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("charge common vote module");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
