// SPDX-License-Identifier: GPL-2.0
/*
 * multi_bat_check.c
 *
 * multi battery check interface for power module
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
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>

#define HWLOG_TAG multi_bat_check
HWLOG_REGIST();

#define BAT_NAME_LEN_MAX            32
#define BAT_CNT_MAX                 2
#define BAT_CONN_WIRELESS_CNT_MAX   3

enum {
	MULTI_BAT_DEFAULT_MODE,
	MULTI_BAT_PARALLEL_MODE,
	MULTI_BAT_SERIES_MODE,
	MULTI_BAT_CONN_CNT_MAX,
};

enum {
	BAT_INFO_ID,
	BAT_INFO_NAME,
	BAT_INFO_MAX,
};
struct bat_para {
	int bat_id;
	char bat_name[BAT_NAME_LEN_MAX];
};

enum {
	BAT_ID,
	BAT_THRE_VOL_LTH,
	BAT_THRE_VOL_HTH,
	BAT_THRE_SOC_HTH,
	BAT_THRE_CUR_LTH,
	BAT_THRE_CUR_TH_HYSTERESIS,
	BAT_THRE_MAX,
};
struct bat_thre_para {
	int bat_id;
	int vol_lth;
	int vol_hth;
	int soc_hth;
	int cur_lth;
	int cur_th_hysteresis;
	bool en_cur_hys;
	int cur_cnt;
};

enum {
	BAT_ID_0,
	BAT_ID_1,
	BAT_EXIT_DIFF_VOL,
	BAT_ENTER_DIFF_VOL,
	BAT_EXIT_DIFF_SOC,
	BAT_ENTER_DIFF_SOC,
	BAT_DIFF_MAX,
};
struct bat_diff_para {
	int bat_id_0;
	int bat_id_1;
	int exit_diff_vol;
	int enter_diff_vol;
	int exit_diff_soc;
	int enter_diff_soc;
};

enum {
	BAT_CONN_MODE,
	BAT_CHG_MODE,
	BAT_THRE_NAME,
	BAT_DIFF_NAME,
	BAT_CONN_MAX,
};

struct bat_connect_para {
	int bat_conn_mode;
	int chg_mode;
	int thre_para_len;
	int diff_para_len;
	struct bat_thre_para thre_para[BAT_CNT_MAX];
	struct bat_diff_para diff_para[BAT_CNT_MAX];
};

struct bat_rt_para {
	int cur;
	int vol;
	int soc;
};

struct multi_bat_ck_dev {
	struct device *dev;
	struct notifier_block event_nb;
	struct notifier_block wlc_event_nb;
	int bat_info_len;
	int bat_conn_len;
	int bat_conn_wireless_len;
	struct bat_para info[BAT_CNT_MAX];
	struct bat_rt_para rt_info[BAT_CNT_MAX];
	struct bat_connect_para conn_para[MULTI_BAT_CONN_CNT_MAX];
	struct bat_connect_para conn_wireless_para[BAT_CONN_WIRELESS_CNT_MAX];
};

static struct multi_bat_ck_dev *g_bat_ck_di;
struct multi_bat_ck_dev *multi_bat_ck_get_di(void)
{
	return g_bat_ck_di;
}

static void multi_bat_ck_get_bat_info(const char *name,
	struct bat_rt_para *info)
{
	power_supply_get_int_property_value(name,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &info->vol);
	info->vol /= POWER_UV_PER_MV;
	power_supply_get_int_property_value(name,
		POWER_SUPPLY_PROP_CAPACITY, &info->soc);
	power_supply_get_int_property_value(name,
		POWER_SUPPLY_PROP_CURRENT_NOW, &info->cur);
	hwlog_info("%s bat vol=%d soc=%d cur=%d\n", name, info->vol, info->soc, info->cur);
}

static void multi_bat_ck_update_bat_info(struct bat_para *bat_info, int len, struct bat_rt_para *info)
{
	int i, id;

	for (i = 0; i < len; i++) {
		id = bat_info[i].bat_id;
		multi_bat_ck_get_bat_info(bat_info[i].bat_name, &info[id]);
	}
}

static bool multi_bat_check_thre(struct bat_thre_para *para, int len, struct bat_rt_para *info, bool cur_flag)
{
	int i, id, cur_lth;

	for (i = 0; i < len; i++) {
		id = para[i].bat_id;
		if (id >= BAT_CNT_MAX) {
			hwlog_info("[%d]bat_id=%d is greater than %d\n", i, para[i].bat_id, BAT_CNT_MAX);
			return false;
		}
		if (((para[i].vol_lth > 0) && (info[id].vol < para[i].vol_lth)) ||
			((para[i].vol_hth > 0) && (info[id].vol > para[i].vol_hth))) {
			hwlog_info("[%d]vol=%d is out of range[%d %d]\n", id, info[id].vol, para[i].vol_lth,
				para[i].vol_hth);
			return false;
		}

		if ((para[i].soc_hth > 0) && (info[id].soc > para[i].soc_hth)) {
			hwlog_info("[%d]soc=%d is greater than %d\n", id, info[id].soc, para[i].soc_hth);
			return false;
		}

		if (!cur_flag) {
			para[i].cur_cnt = 0;
			continue;
		}

		if (para[i].cur_lth <= 0)
			continue;

		cur_lth = para[i].en_cur_hys ? para[i].cur_th_hysteresis : para[i].cur_lth;
		/* current must be abnormal for 30 consecutive times(about 10s) */
		if ((info[id].cur < cur_lth) && (para[i].cur_cnt++ >= 30)) {
			hwlog_info("[%d]cur=%d is less than %d\n", id, info[id].cur, cur_lth);
			para[i].en_cur_hys = true;
			return false;
		}

		para[i].cur_cnt = 0;
		para[i].en_cur_hys = false;
	}

	return true;
}

static bool multi_bat_check_diff(struct bat_diff_para *para, int len,
	struct bat_rt_para *info, bool is_same_mode)
{
	int i, id0, id1, diff_vol, diff_vol_th, diff_soc, diff_soc_th;

	for (i = 0; i < len; i++) {
		id0 = para[i].bat_id_0;
		id1 = para[i].bat_id_1;
		if ((id0 >= BAT_CNT_MAX) || (id1 >= BAT_CNT_MAX)) {
			hwlog_info("[%d]bat_id_0=%d or bat_id_1=%d is greater than %d\n", i,
				para[i].bat_id_0, para[i].bat_id_0, BAT_CNT_MAX);
			return false;
		}

		diff_vol = abs(info[id0].vol - info[id1].vol);
		diff_soc = abs(info[id0].soc - info[id1].soc);
		diff_vol_th = is_same_mode ? para[i].exit_diff_vol : para[i].enter_diff_vol;
		if (diff_vol > diff_vol_th) {
			hwlog_info("%d %d diff_vol of %d %d is greater than %d\n", id0, id1, info[id0].vol,
				info[id1].vol, diff_vol_th);
			return false;
		}

		diff_soc_th = is_same_mode ? para[i].exit_diff_soc : para[i].enter_diff_soc;
		if (diff_soc > diff_soc_th) {
			hwlog_info("is_same_mode %u %d %d diff_soc of %d %d is greater than %d\n",
				is_same_mode, id0, id1, info[id0].soc, info[id1].soc, diff_soc_th);
			return false;
		}
	}
	return true;
}

static bool __multi_bat_check_support(struct bat_connect_para *conn_para, int i,
	bool cur_flag, bool is_same_mode)
{
	struct multi_bat_ck_dev *di = multi_bat_ck_get_di();

	if (!multi_bat_check_thre(conn_para[i].thre_para, conn_para[i].thre_para_len, di->rt_info,
		cur_flag)) {
		hwlog_err("[%d]thre check fail\n", i);
		return false;
	}

	if (!multi_bat_check_diff(conn_para[i].diff_para, conn_para[i].diff_para_len, di->rt_info,
		is_same_mode)) {
		hwlog_err("[%d]diff check fail\n", i);
		return false;
	}

	hwlog_info("[%d]connect_mode=%d ,chg_mode=%d is support\n", i, conn_para[i].bat_conn_mode,
		conn_para[i].chg_mode);
	return true;
}

int multi_batchk_get_support_mode(int cur_mode, bool cur_flag)
{
	int i;
	bool is_same_mode;
	unsigned int chg_mode = 0;
	struct multi_bat_ck_dev *di = multi_bat_ck_get_di();

	if (!di) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	multi_bat_ck_update_bat_info(di->info, di->bat_info_len, di->rt_info);
	for (i = 0; i < di->bat_conn_len; i++) {
		is_same_mode = (unsigned int)cur_mode & (unsigned int)di->conn_para[i].chg_mode;
		if (!__multi_bat_check_support(di->conn_para, i, cur_flag, is_same_mode))
			continue;

		chg_mode |= (unsigned int)di->conn_para[i].chg_mode;
	}

	return (int)chg_mode;
}

bool multi_batchk_match_curr_dc_type(int dc_type, bool cur_flag)
{
	int i;
	struct multi_bat_ck_dev *di = multi_bat_ck_get_di();

	if (!di) {
		hwlog_err("di is null\n");
		return true;
	}

	multi_bat_ck_update_bat_info(di->info, di->bat_info_len, di->rt_info);
	for (i = 0; i < di->bat_conn_wireless_len; i++) {
		if (dc_type != di->conn_wireless_para[i].chg_mode)
			continue;

		return __multi_bat_check_support(di->conn_wireless_para, i, cur_flag, cur_flag);
	}

	return true;
}

static int multi_bat_ck_parse_bat_para(struct device_node *np, struct multi_bat_ck_dev *di)
{
	int i, row, col, len, idata, ret;
	const char *tmp_string = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"bat_para", BAT_CNT_MAX, BAT_INFO_MAX);
	if (len < 0)
		return -EPERM;

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "bat_para", i, &tmp_string))
			return -EPERM;

		row = i / BAT_INFO_MAX;
		col = i % BAT_INFO_MAX;
		switch (col) {
		case BAT_INFO_ID:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return -EPERM;
			if ((idata >= BAT_CNT_MAX) || (idata < 0))
				return -EPERM;
			di->info[row].bat_id = idata;
			break;
		case BAT_INFO_NAME:
			ret = strncpy_s(di->info[row].bat_name, BAT_NAME_LEN_MAX - 1,
				tmp_string, strlen(tmp_string));
			if (ret != EOK)
				return -EPERM;
			break;
		default:
			break;
		}
	}

	di->bat_info_len = len / BAT_INFO_MAX;
	for (i = 0; i < di->bat_info_len; i++)
		hwlog_info("bat_para[%d]=%d %s\n", i, di->info[i].bat_id, di->info[i].bat_name);

	return 0;
}

static void multi_bat_ck_parse_thre_para(struct device_node *np,
	struct bat_thre_para *info, int *info_len, const char *name)
{
	int row, col, len;
	int data[BAT_CNT_MAX * BAT_THRE_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, data, BAT_CNT_MAX, BAT_THRE_MAX);
	if (len < 0) {
		hwlog_err("%s is null\n", name);
		return;
	}

	*info_len = len / BAT_THRE_MAX;
	for (row = 0; row < *info_len; row++) {
		col = row * BAT_THRE_MAX + BAT_ID;
		info[row].bat_id = data[col];
		col = row * BAT_THRE_MAX + BAT_THRE_VOL_LTH;
		info[row].vol_lth = data[col];
		col = row * BAT_THRE_MAX + BAT_THRE_VOL_HTH;
		info[row].vol_hth = data[col];
		col = row * BAT_THRE_MAX + BAT_THRE_SOC_HTH;
		info[row].soc_hth = data[col];
		col = row * BAT_THRE_MAX + BAT_THRE_CUR_LTH;
		info[row].cur_lth = data[col];
		col = row * BAT_THRE_MAX + BAT_THRE_CUR_TH_HYSTERESIS;
		info[row].cur_th_hysteresis = data[col];
		hwlog_info("%s[%d]=%d %d %d %d %d %d\n", name, row,
			info[row].bat_id, info[row].vol_lth,
			info[row].vol_hth, info[row].soc_hth,
			info[row].cur_lth, info[row].cur_th_hysteresis);
	}
}

static void multi_bat_ck_parse_diff_para(struct device_node *np,
	struct bat_diff_para *info, int *info_len, const char *name)
{
	int row, col, len;
	int data[BAT_CNT_MAX * BAT_DIFF_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, data, BAT_CNT_MAX, BAT_DIFF_MAX);
	if (len < 0) {
		hwlog_err("%s is null\n", name);
		return;
	}

	*info_len = len / BAT_DIFF_MAX;
	for (row = 0; row < *info_len; row++) {
		col = row * BAT_DIFF_MAX + BAT_ID_0;
		info[row].bat_id_0 = data[col];
		col = row * BAT_DIFF_MAX + BAT_ID_1;
		info[row].bat_id_1 = data[col];
		col = row * BAT_DIFF_MAX + BAT_EXIT_DIFF_VOL;
		info[row].exit_diff_vol = data[col];
		col = row * BAT_DIFF_MAX + BAT_ENTER_DIFF_VOL;
		info[row].enter_diff_vol = data[col];
		col = row * BAT_DIFF_MAX + BAT_EXIT_DIFF_SOC;
		info[row].exit_diff_soc = data[col];
		col = row * BAT_DIFF_MAX + BAT_ENTER_DIFF_SOC;
		info[row].enter_diff_soc = data[col];
		hwlog_info("%s[%d]=%d %d %d %d %d %d\n", name, row,
			info[row].bat_id_0, info[row].bat_id_1,
			info[row].exit_diff_vol, info[row].enter_diff_vol,
			info[row].exit_diff_soc, info[row].enter_diff_soc);
	}
}

static int __multi_bat_ck_parse_conn_para(struct device_node *np, const char *prop,
	int conn_cnt_max, struct bat_connect_para *conn_para, int *bat_conn_len)
{
	int i, row, col, len, idata;
	const char *tmp_string = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		prop, conn_cnt_max, BAT_CONN_MAX);
	if (len < 0)
		return -EPERM;

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, prop, i, &tmp_string))
			return -EPERM;

		row = i / BAT_CONN_MAX;
		col = i % BAT_CONN_MAX;
		switch (col) {
		case BAT_CONN_MODE:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return -EPERM;
			if ((idata >= conn_cnt_max) || (idata < 0))
				return -EPERM;
			conn_para[row].bat_conn_mode = idata;
			break;
		case BAT_CHG_MODE:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return -EPERM;
			if (idata < 0)
				return -EPERM;
			conn_para[row].chg_mode = idata;
			break;
		case BAT_THRE_NAME:
			multi_bat_ck_parse_thre_para(np, conn_para[row].thre_para,
				&conn_para[row].thre_para_len, tmp_string);
			break;
		case BAT_DIFF_NAME:
			multi_bat_ck_parse_diff_para(np, conn_para[row].diff_para,
				&conn_para[row].diff_para_len, tmp_string);
			break;
		default:
			break;
		}
	}

	*bat_conn_len = len / BAT_CONN_MAX;
	for (i = 0; i < *bat_conn_len; i++)
		hwlog_info("%s[%d]=%d %d\n", prop, i, conn_para[i].bat_conn_mode, conn_para[i].chg_mode);

	return 0;
}

static int multi_bat_ck_parse_conn_para(struct device_node *np, struct multi_bat_ck_dev *di)
{
	if (__multi_bat_ck_parse_conn_para(np, "bat_connect_para", MULTI_BAT_CONN_CNT_MAX,
		di->conn_para, &di->bat_conn_len))
		return -EPERM;

	__multi_bat_ck_parse_conn_para(np, "bat_connect_wireless_para", BAT_CONN_WIRELESS_CNT_MAX,
		di->conn_wireless_para, &di->bat_conn_wireless_len);

	return 0;
}

static int multi_bat_ck_parse_dts(struct device_node *np,
	struct multi_bat_ck_dev *di)
{
	if (multi_bat_ck_parse_bat_para(np, di))
		return -EPERM;

	if (multi_bat_ck_parse_conn_para(np, di))
		return -EPERM;
	return 0;
}

static void multi_bat_ck_clear_info(struct bat_connect_para *conn_para, int bat_conn_len)
{
	int i, j;

	for (i = 0; i < bat_conn_len; i++) {
		for (j = 0; j < conn_para[i].thre_para_len; j++) {
			conn_para[i].thre_para[j].en_cur_hys = false;
			conn_para[i].thre_para[j].cur_cnt = 0;
		}
	}
}

static void multi_bat_ck_notifier_handler(struct multi_bat_ck_dev *di, unsigned long event)
{
	switch (event) {
	case POWER_NE_USB_DISCONNECT:
		multi_bat_ck_clear_info(di->conn_para, di->bat_conn_len);
		break;
	case POWER_NE_WLC_VBUS_DISCONNECT:
		multi_bat_ck_clear_info(di->conn_wireless_para, di->bat_conn_wireless_len);
		break;
	default:
		break;
	}
}

static int multi_bat_ck_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct multi_bat_ck_dev *di = container_of(nb, struct multi_bat_ck_dev, event_nb);

	if (!di)
		return NOTIFY_OK;

	multi_bat_ck_notifier_handler(di, event);

	return NOTIFY_OK;
}

static int multi_bat_ck_wlc_notifier_call(struct notifier_block *wlc_event_nb,
	unsigned long event, void *data)
{
	struct multi_bat_ck_dev *di = container_of(wlc_event_nb,
		struct multi_bat_ck_dev, wlc_event_nb);

	if (!di)
		return NOTIFY_OK;

	multi_bat_ck_notifier_handler(di, event);

	return NOTIFY_OK;
}

static int multi_bat_ck_probe(struct platform_device *pdev)
{
	struct multi_bat_ck_dev *di = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	ret = multi_bat_ck_parse_dts(np, di);
	if (ret)
		goto err_out;

	di->event_nb.notifier_call = multi_bat_ck_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &di->event_nb);
	if (ret)
		goto err_out;
	di->wlc_event_nb.notifier_call = multi_bat_ck_wlc_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLC, &di->wlc_event_nb);
	if (ret)
		goto err_out;

	platform_set_drvdata(pdev, di);
	g_bat_ck_di = di;
	return 0;

err_out:
	devm_kfree(&pdev->dev, di);
	g_bat_ck_di = NULL;
	return ret;
}

static int multi_bat_ck_remove(struct platform_device *pdev)
{
	struct multi_bat_ck_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	power_event_bnc_unregister(POWER_BNT_CONNECT, &di->event_nb);
	power_event_bnc_unregister(POWER_BNT_WLC, &di->wlc_event_nb);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_bat_ck_di = NULL;

	return 0;
}

static const struct of_device_id bat_ck_match_table[] = {
	{
		.compatible = "huawei,multi_bat_check",
		.data = NULL,
	},
	{},
};

static struct platform_driver multi_bat_ck_driver = {
	.probe = multi_bat_ck_probe,
	.remove = multi_bat_ck_remove,
	.driver = {
		.name = "huawei,multi_bat_check",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_ck_match_table),
	},
};

static int __init multi_bat_ck_init(void)
{
	return platform_driver_register(&multi_bat_ck_driver);
}

static void __exit multi_bat_ck_exit(void)
{
	platform_driver_unregister(&multi_bat_ck_driver);
}

device_initcall_sync(multi_bat_ck_init);
module_exit(multi_bat_ck_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei multi bat check module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
