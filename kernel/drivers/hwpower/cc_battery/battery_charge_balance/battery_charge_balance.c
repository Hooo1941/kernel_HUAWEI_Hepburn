// SPDX-License-Identifier: GPL-2.0
/*
 * battery_charge_balance.c
 *
 * huawei battery charge balance driver
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/battery/battery_charge_balance.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_rechg.h>

#define HWLOG_TAG battery_charge_balance
HWLOG_REGIST();

#define BAL_CCCV_TAB_COL        2
#define BAL_TEMP_TAB_COL        2

static struct bat_chg_bal_device *g_bat_chg_bal_dev;
static const char *g_battery_psy[] = {
	"battery_gauge",
	"battery_gauge_aux",
	"battery_gauge_third"
};

static int bat_chg_bal_get_cccv_node(int bat_id,
	struct bat_chg_balance_info info, struct bat_cccv_node *node)
{
	int i;
	struct bat_chg_bal_device *di = g_bat_chg_bal_dev;
	struct bat_param *param = NULL;
	struct bat_temp_cccv *temp_cccv = NULL;

	if (!di)
		return -ENODEV;

	param = &di->param_tab[bat_id];
	for (i = 0; i < param->len; i++) {
		if (info.temp < param->temp_tab[i].temp)
			break;
	}
	if (i >= param->len)
		i = param->len - 1;
	temp_cccv = &param->temp_tab[i];
	for (i = 0; i < temp_cccv->len; i++) {
		if (info.vol < temp_cccv->cccv_tab[i].vol)
			break;
	}
	if (i >= temp_cccv->len)
		i = temp_cccv->len - 1;
	*node = temp_cccv->cccv_tab[i];
	return 0;
}

static int bat_chg_balance_get_bat_info(struct bat_chg_balance_info *info, int idx)
{
	struct power_supply *psy = NULL;

	if (idx >= ARRAY_SIZE(g_battery_psy))
		return -EINVAL;

	if (!power_supply_check_psy_available(g_battery_psy[idx], &psy))
		return -EINVAL;

	power_supply_get_int_prop(g_battery_psy[idx],
		POWER_SUPPLY_PROP_CURRENT_NOW, &info->cur, 0, 1);
	power_supply_get_int_prop(g_battery_psy[idx],
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &info->vol, 0, POWER_UV_PER_MV);
	bat_temp_get_temperature(idx, &info->temp);
	return 0;
}

int bat_chg_balance_get_cur_info(struct bat_chg_balance_info *info,
	u32 info_len, struct bat_chg_balance_cur *result, int mode)
{
	struct bat_cccv_node cccv_node[BAT_BALANCE_COUNT];
	struct bat_chg_bal_device *di = g_bat_chg_bal_dev;

	if (!di)
		return -ENODEV;

	bat_chg_bal_get_cccv_node(BAT_MAIN, info[BAT_MAIN], &cccv_node[BAT_MAIN]);
	bat_chg_bal_get_cccv_node(BAT_AUX, info[BAT_AUX], &cccv_node[BAT_AUX]);

	if (mode == BAT_PARALLEL_MODE)
		return bat_chg_bal_parallel_get_cur_info(di, info, info_len,
			cccv_node, BAT_BALANCE_COUNT, result);
	else if (mode == BAT_SERIAL_MODE)
		return bat_chg_bal_serial_get_cur_info(di, info, info_len,
			cccv_node, BAT_BALANCE_COUNT, result);
	else
		hwlog_err("%s invalid mode\n", __func__);

	return -EINVAL;
}

void bat_chg_balance_init_rechg_info(void)
{
	struct bat_chg_bal_device *di = g_bat_chg_bal_dev;

	if (!di)
		return;

	di->rechg_info.ibat_th = -1;
	di->rechg_info.vol_diff = 0;
	di->rechg_info.idx = 0;
}

static void bat_chg_balance_rechg_handle(int vol_diff, struct recharge_ck_info *last_info,
	const struct recharge_ck_para *para, int len)
{
	int i;
	int ibat_th = -1;

	last_info->vol_diff = vol_diff;
	for (i = 0; i < len; i++) {
		if ((vol_diff >= para[i].vol_diff_min) && (vol_diff < para[i].vol_diff_max))
			break;
	}

	if (i >= len) {
		last_info->idx = 0;
		last_info->ibat_th = ibat_th;
		return;
	}

	if ((last_info->idx - i < 0) || (para[i].vol_diff_max - vol_diff >
		para[i].vol_diff_back) || (abs(last_info->idx - i) > 1)) {
			last_info->idx = i;
			if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS)
				last_info->ibat_th = para[i].ibat_th_wireless;
			else
				last_info->ibat_th = para[i].ibat_th;
	}
}

bool bat_chg_balance_check_recharge(int *ibat, int *pre_ibat)
{
	int ibat_th;
	struct bat_cccv_node cccv_node[BAT_BALANCE_COUNT];
	struct bat_chg_balance_info info[BAT_BALANCE_COUNT];
	struct bat_chg_bal_device *di = g_bat_chg_bal_dev;

	if (!di || (di->rechg_ck_tbl_len == 0))
		return false;

	if (bat_chg_balance_get_bat_info(&info[BAT_MAIN], BAT_MAIN) ||
		bat_chg_balance_get_bat_info(&info[BAT_AUX], BAT_AUX))
		return false;

	(void)bat_chg_bal_get_cccv_node(BAT_MAIN, info[BAT_MAIN], &cccv_node[BAT_MAIN]);
	(void)bat_chg_bal_get_cccv_node(BAT_AUX, info[BAT_AUX], &cccv_node[BAT_AUX]);

	if (cccv_node[BAT_AUX].cur <= cccv_node[BAT_MAIN].cur) {
		hwlog_info("bat need cur is %d %d, no need recharge\n",
			cccv_node[BAT_MAIN].cur, cccv_node[BAT_AUX].cur);
		return false;
	}

	bat_chg_balance_rechg_handle(info[BAT_MAIN].vol - info[BAT_AUX].vol,
		&di->rechg_info, di->rechg_ck_tbl, di->rechg_ck_tbl_len);
	ibat_th = di->rechg_info.ibat_th;
	if (ibat_th < 0)
		return false;

	cccv_node[BAT_MAIN].cur = power_min_positive(cccv_node[BAT_MAIN].cur, info[BAT_MAIN].cur);
	hwlog_info("%s current battery main max cur is %d\n", __func__, cccv_node[BAT_MAIN].cur);

	if (ibat_th == 0)
		*pre_ibat = cccv_node[BAT_MAIN].cur;

	*ibat = power_min_positive(ibat_th, cccv_node[BAT_AUX].cur - cccv_node[BAT_MAIN].cur);
	return true;
}

bool bat_chg_bal_is_control_by_single_gauge(void)
{
	struct bat_chg_bal_device *di = g_bat_chg_bal_dev;

	if (!di)
		return false;

	return !!di->batt_balance_by_single_gauge;
}

static int bat_chg_bal_get_log_head(char *buffer, int size, void *dev_data)
{
	struct bat_chg_bal_device *di = dev_data;

	if (!di || !buffer)
		return -EPERM;

	snprintf_s(buffer, size, size - 1, "bal_cur  req_cur0 req_cur1 ");

	return 0;
}

static int bat_chg_bal_dump_log_data(char *buffer, int size, void *dev_data)
{
	struct bat_chg_bal_device *di = dev_data;

	if (!di || !buffer)
		return -EPERM;

	snprintf_s(buffer, size, size - 1, "%-9d%-9d%-9d",
		di->bal_cur, di->req_cur[BAT_MAIN], di->req_cur[BAT_AUX]);

	return 0;
}

static struct power_log_ops bat_chg_bal_log_ops = {
	.dev_name = "bat_balance",
	.dump_log_head = bat_chg_bal_get_log_head,
	.dump_log_content = bat_chg_bal_dump_log_data,
};

static int bat_chg_bal_cccv_tab_dts(struct device_node *node,
	const char *tab_name,
	struct bat_temp_cccv *temp_cccv)
{
	int i, len;
	u32 data;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), node,
			tab_name, BAL_CCCV_TAB_LEN, BAL_CCCV_TAB_COL);
	if (len <= 0) {
		hwlog_err("cccv_tab_dts get failed\n");
		return -ENODEV;
	}
	temp_cccv->len = len / BAL_CCCV_TAB_COL;
	for (i = 0; i < len; i++) {
		if (power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
			node, tab_name, i, &data))
			return -ENODEV;
		if ((i % BAL_CCCV_TAB_COL) == 0)
			temp_cccv->cccv_tab[i / BAL_CCCV_TAB_COL].vol = (int)data;
		else
			temp_cccv->cccv_tab[i / BAL_CCCV_TAB_COL].cur = (int)data;
	}
	return 0;
}

static int bat_chg_bal_parse_bat_dts(int bat_id, struct bat_chg_bal_device *di,
	struct device_node *node)
{
	int ret;
	int i, len;
	int idata = 0;
	const char *string = NULL;

	ret = power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"weight", &di->param_tab[bat_id].weight, 2000); /* 2000mAH default */
	if (ret) {
		hwlog_err("batt %d weight failed\n", bat_id);
		return ret;
	}
	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), node,
		"temp_tab", BAL_TEMP_TAB_LEN, BAL_TEMP_TAB_COL);
	if (len < 0)
		return -ENODEV;
	di->param_tab[bat_id].len = len / BAL_TEMP_TAB_COL;
	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			node, "temp_tab", i, &string))
			return -ENODEV;

		if ((i % BAL_TEMP_TAB_COL) == 0) {
			if (kstrtoint(string, POWER_BASE_DEC, &idata))
				return -ENODEV;
			di->param_tab[bat_id].temp_tab[i / BAL_TEMP_TAB_COL].temp =
				idata;
			hwlog_info("bat_id %d temp_tab temp:%d\n", bat_id, idata);
		} else {
			if (bat_chg_bal_cccv_tab_dts(node, string,
				&di->param_tab[bat_id].temp_tab[i / BAL_TEMP_TAB_COL]))
				return -ENODEV;
		}
	}
	return 0;
}

static void bat_chg_bal_parse_rechg_ck_para(struct device_node *np,
	struct recharge_ck_para *info, int *info_len, const char *name)
{
	int len;
	int i;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, (int *)info, BAL_RECHG_CK_PARA_COUNT, BAL_RECHG_CK_PARA_COL);
	if (len < 0)
		return;

	*info_len = len / BAL_RECHG_CK_PARA_COL;
	for (i = 0; i < *info_len; i++)
		hwlog_info("row%d: vdmin %d vdmax %d ith %d ithw %d back %d\n", i,
			info[i].vol_diff_min, info[i].vol_diff_max,
			info[i].ibat_th, info[i].ibat_th_wireless, info[i].vol_diff_back);
}

static int bat_chg_bal_parse_dts(struct bat_chg_bal_device *di)
{
	struct device_node *np = di->dev->of_node;
	int ret;
	struct device_node *bat_node = NULL;

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"unbalance_th", di->unbalance_th, BAL_UNBAL_TAB_LEN);
	if (ret)
		return ret;

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"batt_balance_by_single_gauge", &di->batt_balance_by_single_gauge, 0);
	hwlog_info("batt_balance_by_single_gauge = %d\n", di->batt_balance_by_single_gauge);

	bat_node = of_find_node_by_name(np, "battery0");
	if (!bat_node) {
		hwlog_err("batt0 find failed\n");
		return -ENODEV;
	}
	ret = bat_chg_bal_parse_bat_dts(BAT_MAIN, di, bat_node);
	if (ret) {
		hwlog_err("batt0 para read failed\n");
		return ret;
	}

	bat_node = of_find_node_by_name(np, "battery1");
	if (!bat_node) {
		hwlog_err("batt1 find failed\n");
		return -ENODEV;
	}
	ret = bat_chg_bal_parse_bat_dts(BAT_AUX, di, bat_node);
	if (ret) {
		hwlog_err("batt1 para read failed\n");
		return ret;
	}

	bat_chg_bal_parse_rechg_ck_para(np, di->rechg_ck_tbl,
		&di->rechg_ck_tbl_len, "recharge_check_para");
	return 0;
}

static int bat_chg_bal_probe(struct platform_device *pdev)
{
	struct bat_chg_bal_device *di = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	di->dev = &pdev->dev;
	ret = bat_chg_bal_parse_dts(di);
	if (ret) {
		hwlog_err("parse dts err\n");
		return ret;
	}
	g_bat_chg_bal_dev = di;

	bat_chg_bal_log_ops.dev_data = (void *)di;
	power_log_ops_register(&bat_chg_bal_log_ops);

	hwlog_info("%s ok\n", __func__);
	return 0;
}

static int bat_chg_bal_remove(struct platform_device *pdev)
{
	struct bat_chg_bal_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;
	g_bat_chg_bal_dev = NULL;
	return 0;
}

static const struct of_device_id bat_chg_bal_match_table[] = {
	{
		.compatible = "huawei,battery_charge_balance",
		.data = NULL,
	},
	{},
};

static struct platform_driver bat_chg_bal_driver = {
	.probe = bat_chg_bal_probe,
	.remove = bat_chg_bal_remove,
	.driver = {
		.name = "huawei,battery_charge_balance",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bat_chg_bal_match_table),
	},
};

static int __init bat_chg_bal_init(void)
{
	return platform_driver_register(&bat_chg_bal_driver);
}

static void __exit bat_chg_bal_exit(void)
{
	platform_driver_unregister(&bat_chg_bal_driver);
}

late_initcall_sync(bat_chg_bal_init);
module_exit(bat_chg_bal_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery charge balance driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
