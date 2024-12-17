// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_ic_iout.c
 *
 * ic iout for wireless charging
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/hardware_ic/wireless_ic_iout.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include "securec.h"

#define HWLOG_TAG wireless_ic_iout
HWLOG_REGIST();

#define RX_TEMP_PARA_ROW        3
#define RX_TEMP_PARA_COL        4
#define RX_TIME_PARA_ROW        6
#define RX_TIME_PARA_COL        2
#define RX_IMAX_START_MON_TIME  10
#define OUT_BUF_SIZE            256
#define TMP_BUF_SIZE            32
#define IRX_SAMPLE_LEN          10
#define RX_MODE_NAME_LEN        16
#define RX_FACTOR_FALG_BIT      7    /* Bit(7): 1-AND 0-OR */

enum wlic_mon_irx_type {
	RX_IMAX_NO_NEED_MON,
	RX_IMAX_NEED_MON,
	RX_IMAX_FIRST_MON,
	RX_IMAX_NON_FIRST_MON,
};

struct wlirx_time_para {
	int th;
	int rx_imax;
};

struct wlirx_temp_para {
	int lth;
	int hth;
	int rx_imax;
	int back;
};

enum wlirx_time_para_group_info {
	WLIRX_TIMEG_MODE_NAME = 0,
	WLIRX_TIMEG_FACTOR_MODE,
	WLIRX_TIMEG_GROUP_NAME,
	WLIRX_TIMEG_TOTAL,
};

struct wlirx_time_para_group {
	char pmode_name[RX_MODE_NAME_LEN];
	unsigned int factor_mode;
	bool parse_ok;
	struct wlirx_time_para time_para[RX_TIME_PARA_ROW];
};

struct wlic_iout_dev {
	int *rx_ic_imax;
	int cur_rx_imax;
	int rx_imax_flag;
	int rx_iavg;
	int irx_samples[IRX_SAMPLE_LEN];
	unsigned int ic_type;
	unsigned long factor;
	bool time_para_parse_ok;
	struct delayed_work calc_work;
	struct wlirx_time_para dflt_time_para[RX_TIME_PARA_ROW];
	struct wlirx_temp_para temp_para[RX_TEMP_PARA_ROW];
	bool rx_stop_chrg_flag;
	char cur_pmode_name[RX_MODE_NAME_LEN];
	int time_para_group_size;
	struct wlirx_time_para_group *time_para_group;
};

static struct wlic_iout_dev *g_ic_iout_di[WLTRX_IC_MAX];

static struct wlic_iout_dev *wlic_iout_get_di(unsigned int ic_type)
{
	if (!wltrx_ic_is_type_valid(ic_type)) {
		hwlog_err("get_di: ic_type=%u err\n", ic_type);
		return NULL;
	}

	return g_ic_iout_di[ic_type];
}

static bool __wlic_iout_is_match_factor(struct wlic_iout_dev *di, unsigned long factor_mode)
{
	if (test_bit(RX_FACTOR_FALG_BIT, &factor_mode)) {
		factor_mode &= ~BIT(RX_FACTOR_FALG_BIT);
		return (di->factor & factor_mode) == factor_mode;
	}

	return (di->factor & factor_mode) > 0;
}

static struct wlirx_time_para *wlic_get_time_para(struct wlic_iout_dev *di)
{
	int i;
	int vfc_reg = 0;

	for (i = 0; i < di->time_para_group_size; i++) {
		if (!power_strsame(di->cur_pmode_name, di->time_para_group[i].pmode_name) ||
			!di->time_para_group[i].parse_ok)
			continue;
		if (di->time_para_group[i].factor_mode > 0 &&
			!__wlic_iout_is_match_factor(di, di->time_para_group[i].factor_mode))
			continue;
		return di->time_para_group[i].time_para;
	}

	(void)wlrx_ic_get_vfc_reg(WLTRX_IC_MAIN, &vfc_reg);
	if ((vfc_reg < WLRX_IC_HIGH_VOUT2) && !wlrx_is_rvs_tx(WLTRX_DRV_MAIN))
		return NULL;

	return di->dflt_time_para;
}

static void wlic_iout_get_time_imax(struct wlic_iout_dev *di, int *imax_time)
{
	int i;
	unsigned int delta_time;
	static unsigned int start_time;
	struct wlirx_time_para *time_para = NULL;

	if ((di->rx_imax_flag == RX_IMAX_FIRST_MON) || (start_time < RX_IMAX_START_MON_TIME))
		start_time = power_get_current_kernel_time().tv_sec;

	time_para = wlic_get_time_para(di);
	if (!time_para) {
		*imax_time = WLIC_DEFAULT_RX_IMAX;
		return;
	}
	delta_time = power_get_current_kernel_time().tv_sec - start_time;
	for (i = 0; i < RX_TIME_PARA_ROW; i++) {
		if (delta_time >= time_para[i].th)
			break;
	}
	if ((i >= RX_TIME_PARA_ROW) || (time_para[i].rx_imax <= 0)) {
		*imax_time = WLIC_DEFAULT_RX_IMAX;
		return;
	}
	*imax_time = time_para[i].rx_imax;
}

static void wlic_iout_get_temp_imax(struct wlic_iout_dev *di, int *imax_temp)
{
	int i;
	int temp = 0;
	static int last_i;
	int cur_i = last_i;

	(void)wlrx_ic_get_temp(WLTRX_IC_MAIN, &temp);
	for (i = 0; i < RX_TEMP_PARA_ROW; i++) {
		if ((temp > di->temp_para[i].lth) && (temp <= di->temp_para[i].hth)) {
			if ((di->rx_imax_flag == RX_IMAX_FIRST_MON) || (last_i < i) ||
				(di->temp_para[i].hth - temp > di->temp_para[i].back))
				cur_i = i;
			else
				cur_i = last_i;
			break;
		}
	}

	if ((i >= RX_TEMP_PARA_ROW) || (di->temp_para[cur_i].rx_imax <= 0)) {
		*imax_temp = WLIC_DEFAULT_RX_IMAX;
		return;
	}

	last_i = cur_i;
	*imax_temp = di->temp_para[cur_i].rx_imax;
}

static void wlic_calc_rx_imax(struct wlic_iout_dev *di)
{
	int imax_temp = 0;
	int imax_time = 0;
	int rx_imax = WLIC_DEFAULT_RX_IMAX_IC;

	if (di->rx_imax_flag == RX_IMAX_NO_NEED_MON)
		return;

	if (di->rx_ic_imax && (*di->rx_ic_imax > 0))
		rx_imax = *di->rx_ic_imax;

	wlic_iout_get_temp_imax(di, &imax_temp);
	wlic_iout_get_time_imax(di, &imax_time);

	rx_imax = min(rx_imax, imax_temp);
	rx_imax = min(rx_imax, imax_time);

	di->cur_rx_imax = rx_imax;
	if (di->rx_imax_flag == RX_IMAX_FIRST_MON)
		di->rx_imax_flag = RX_IMAX_NON_FIRST_MON;
}

static void wlic_calc_rx_iavg(struct wlic_iout_dev *di)
{
	int i;
	int irx = 0;
	int irx_sum = 0;
	static int index;

	(void)wlrx_ic_get_iout(di->ic_type, &irx);
	di->irx_samples[index] = irx;
	index = (index + 1) % IRX_SAMPLE_LEN; /* 1: index++ */
	for (i = 0; i < IRX_SAMPLE_LEN; i++)
		irx_sum += di->irx_samples[i];

	di->rx_iavg = irx_sum / IRX_SAMPLE_LEN;
}

static void wlic_reset_rx_iavg(struct wlic_iout_dev *di)
{
	int i;

	di->rx_iavg = 100; /* default avg irx:100mA */
	for (i = 0; i < IRX_SAMPLE_LEN; i++)
		di->irx_samples[i] = 100;
}

static void wlic_iout_calc_work(struct work_struct *work)
{
	struct wlic_iout_dev *di = container_of(work, struct wlic_iout_dev, calc_work.work);

	if (!di)
		return;

	if (di->rx_stop_chrg_flag) {
		wlic_reset_rx_iavg(di);
		return;
	}

	wlic_calc_rx_imax(di);
	wlic_calc_rx_iavg(di);

	/* calc_work interval: 100ms */
	schedule_delayed_work(&di->calc_work, msecs_to_jiffies(100));
}

void wlic_iout_select_para(unsigned int ic_type, unsigned int factor, bool flag)
{
	struct wlic_iout_dev *di = wlic_iout_get_di(ic_type);

	if (!di || (factor < WLICI_FACTOR_BEGIN) || (factor >= WLICI_FACTOR_END))
		return;

	if (flag)
		di->factor |= BIT(factor);
	else
		di->factor &= ~BIT(factor);
	hwlog_info("[select_para] factor=0x%x\n", di->factor);
}

void wlic_iout_set_cur_pmode_name(unsigned int ic_type, const char *cur_pmode_name)
{
	struct wlic_iout_dev *di = wlic_iout_get_di(ic_type);

	if (!di || !cur_pmode_name)
		return;

	(void)strncpy_s(di->cur_pmode_name, RX_MODE_NAME_LEN - 1,
		cur_pmode_name, strlen(cur_pmode_name));
	hwlog_info("[set_cur_pmode_name] cur_pmode_name=%s\n", di->cur_pmode_name);
}

void wlic_iout_start_sample(unsigned int ic_type)
{
	struct wlic_iout_dev *di = wlic_iout_get_di(ic_type);

	if (!di)
		return;

	di->rx_stop_chrg_flag = false;
	if (di->rx_imax_flag != RX_IMAX_NO_NEED_MON)
		di->rx_imax_flag = RX_IMAX_FIRST_MON;

	/* calc_work interval: 100ms */
	mod_delayed_work(system_wq, &di->calc_work, msecs_to_jiffies(100));
}

void wlic_iout_stop_sample(unsigned int ic_type)
{
	struct wlic_iout_dev *di = wlic_iout_get_di(ic_type);

	if (!di)
		return;

	di->rx_stop_chrg_flag = true;
}

void wlic_get_rx_imax(unsigned int ic_type, int *rx_imax)
{
	struct wlic_iout_dev *di = wlic_iout_get_di(ic_type);

	if (!rx_imax)
		return;

	if (!di || (di->cur_rx_imax <= 0))
		*rx_imax = WLIC_DEFAULT_RX_IMAX;
	else
		*rx_imax = di->cur_rx_imax;
}

void wlic_get_rx_iavg(unsigned int ic_type, int *rx_iavg)
{
	struct wlic_iout_dev *di = wlic_iout_get_di(ic_type);

	if (!di || !rx_iavg)
		return;

	*rx_iavg = di->rx_iavg;
}

static ssize_t wlic_iout_time_para_store(void *dev_data, const char *buf, size_t size)
{
	int i;
	int unlock_val = 0;
	char out_buf[OUT_BUF_SIZE] = {0};
	char tmp_buf[TMP_BUF_SIZE] = {0};
	struct wlic_iout_dev *di = dev_data;
	struct wlirx_time_para temp[RX_TIME_PARA_ROW] = {0};

	if (!di)
		return -EINVAL;

	/* 0~5: six group parameters, 1: unlock_val length */
	if ((sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d", &unlock_val, &temp[0].th,
		&temp[0].rx_imax, &temp[1].th, &temp[1].rx_imax, &temp[2].th, &temp[2].rx_imax,
		&temp[3].th, &temp[3].rx_imax, &temp[4].th, &temp[4].rx_imax, &temp[5].th,
		&temp[5].rx_imax) != (RX_TIME_PARA_ROW * RX_TIME_PARA_COL + 1)) ||
		(unlock_val != WLTRX_UNLOCK_VAL))
		return -EINVAL;

	for (i = 0; i < RX_TIME_PARA_ROW; i++) {
		di->dflt_time_para[i].th = temp[i].th;
		di->dflt_time_para[i].rx_imax = temp[i].rx_imax;
		memset(tmp_buf, 0, TMP_BUF_SIZE);
		scnprintf(tmp_buf, TMP_BUF_SIZE, "%-4d  %-4d\n",
			di->dflt_time_para[i].th, di->dflt_time_para[i].rx_imax);
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	hwlog_info("time_para:\n%s\n", out_buf);
	return size;
}

static ssize_t wlic_iout_time_para_show(void *dev_data, char *buf, size_t size)
{
	int i;
	char out_buf[OUT_BUF_SIZE] = {0};
	char tmp_buf[TMP_BUF_SIZE] = {0};
	struct wlic_iout_dev *di = dev_data;

	if (!di)
		return 0;

	for (i = 0; i < RX_TIME_PARA_ROW; i++) {
		memset(tmp_buf, 0, TMP_BUF_SIZE);
		scnprintf(tmp_buf, TMP_BUF_SIZE, "%-4d  %-4d\n",
			di->dflt_time_para[i].th, di->dflt_time_para[i].rx_imax);
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	return scnprintf(buf, size, "time_para:\n%s\n", out_buf);
}

static ssize_t wlic_iout_temp_para_store(void *dev_data, const char *buf, size_t size)
{
	int i;
	int unlock_val = 0;
	char out_buf[OUT_BUF_SIZE] = {0};
	char tmp_buf[TMP_BUF_SIZE] = {0};
	struct wlic_iout_dev *di = dev_data;
	struct wlirx_temp_para temp[RX_TEMP_PARA_ROW] = {0};

	if (!di)
		return -EINVAL;

	/* 0~2: three group parameters, 1: unlock_val length */
	if ((sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d", &unlock_val, &temp[0].lth,
		&temp[0].hth, &temp[0].rx_imax, &temp[0].back, &temp[1].lth, &temp[1].hth,
		&temp[1].rx_imax, &temp[1].back, &temp[2].lth, &temp[2].hth, &temp[2].rx_imax,
		&temp[2].back) != (RX_TEMP_PARA_ROW * RX_TEMP_PARA_COL + 1)) ||
		(unlock_val != WLTRX_UNLOCK_VAL))
		return -EINVAL;

	for (i = 0; i < RX_TEMP_PARA_ROW; i++) {
		di->temp_para[i].lth = temp[i].lth;
		di->temp_para[i].hth = temp[i].hth;
		di->temp_para[i].rx_imax = temp[i].rx_imax;
		di->temp_para[i].back = temp[i].back;
		memset(tmp_buf, 0, TMP_BUF_SIZE);
		scnprintf(tmp_buf, TMP_BUF_SIZE, "%-3d %-3d %-4d %-3d\n", di->temp_para[i].lth,
			di->temp_para[i].hth, di->temp_para[i].rx_imax, di->temp_para[i].back);
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	hwlog_info("temp_para:\n%s\n", out_buf);
	return size;
}

static ssize_t wlic_iout_temp_para_show(void *dev_data, char *buf, size_t size)
{
	int i;
	char out_buf[OUT_BUF_SIZE] = {0};
	char tmp_buf[TMP_BUF_SIZE] = {0};
	struct wlic_iout_dev *di = dev_data;

	if (!di)
		return 0;

	for (i = 0; i < RX_TEMP_PARA_ROW; i++) {
		memset(tmp_buf, 0, TMP_BUF_SIZE);
		scnprintf(tmp_buf, TMP_BUF_SIZE, "%-3d %-3d %-4d %-3d\n", di->temp_para[i].lth,
			di->temp_para[i].hth, di->temp_para[i].rx_imax, di->temp_para[i].back);
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	return scnprintf(buf, size, "temp_para:\n%s\n", out_buf);
}

static void wlic_iout_dbg_register(struct wlic_iout_dev *di)
{
	if (di->rx_imax_flag == RX_IMAX_NO_NEED_MON)
		return;

	power_dbg_ops_register("wlic", "time_para", di,
		wlic_iout_time_para_show, wlic_iout_time_para_store);
	power_dbg_ops_register("wlic", "temp_para", di,
		wlic_iout_temp_para_show, wlic_iout_temp_para_store);
}

static bool wlic_parse_rx_time_para_by_name(const struct device_node *np,
	const char *prop, struct wlirx_time_para *time_para)
{
	int len;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np, prop,
		RX_TIME_PARA_ROW, RX_TIME_PARA_COL);
	if (len <= 0)
		return false;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np, prop,
		(u32 *)time_para, len))
		return false;

	return true;
}

static void wlic_parse_rx_time_para_group(const struct device_node *np, struct wlic_iout_dev *di)
{
	int i, row, col, len;
	const char *tmp_str = NULL;

	len = of_property_count_strings(np, "time_para_group");
	if ((len <= 0) || ((unsigned int)len % WLIRX_TIMEG_TOTAL != 0))
		return;

	di->time_para_group_size = len / WLIRX_TIMEG_TOTAL;
	di->time_para_group = kzalloc(di->time_para_group_size * sizeof(*(di->time_para_group)), GFP_KERNEL);
	if (!di->time_para_group)
		goto error;

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "time_para_group", i, &tmp_str)) {
			goto error;
		}

		row = i / WLIRX_TIMEG_TOTAL;
		col = i % WLIRX_TIMEG_TOTAL;

		switch (col) {
		case WLIRX_TIMEG_MODE_NAME:
			(void)strncpy_s(di->time_para_group[row].pmode_name, RX_MODE_NAME_LEN - 1,
				tmp_str, strlen(tmp_str));
			break;
		case WLIRX_TIMEG_FACTOR_MODE:
			if (kstrtouint(tmp_str, POWER_BASE_DEC, &di->time_para_group[row].factor_mode))
				goto error;
			break;
		case WLIRX_TIMEG_GROUP_NAME:
			di->time_para_group[row].parse_ok = wlic_parse_rx_time_para_by_name(np,
				tmp_str, di->time_para_group[row].time_para);
			break;
		default:
			break;
		}
	}

	return;
error:
	di->time_para_group_size = 0;
	if (di->time_para_group) {
		kfree(di->time_para_group);
		di->time_para_group = NULL;
	}
}

static void wlic_parse_rx_time_para(const struct device_node *np, struct wlic_iout_dev *di)
{
	if (!wlic_parse_rx_time_para_by_name(np, "time_para", di->dflt_time_para))
		return;
	di->time_para_parse_ok = true;

	wlic_parse_rx_time_para_group(np, di);

	di->rx_imax_flag = RX_IMAX_NEED_MON;
}

static void wlic_parse_rx_temp_para(const struct device_node *np, struct wlic_iout_dev *di)
{
	int len;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np, "temp_para",
		RX_TEMP_PARA_ROW, RX_TEMP_PARA_COL);
	if (len <= 0)
		return;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"temp_para", (u32 *)di->temp_para, len))
		return;

	di->rx_imax_flag = RX_IMAX_NEED_MON;
}

static int wlic_rx_imax_init(struct wlic_iout_dev *di, const struct device_node *np, int *rx_ic_imax)
{
	di->rx_ic_imax = rx_ic_imax;
	di->cur_rx_imax = WLIC_DEFAULT_RX_IMAX;

	wlic_parse_rx_time_para(np, di);
	wlic_parse_rx_temp_para(np, di);
	wlic_iout_dbg_register(di);

	return 0;
}

static void wlic_kfree_dev(unsigned int ic_type)
{
	if (g_ic_iout_di[ic_type]->time_para_group) {
		kfree(g_ic_iout_di[ic_type]->time_para_group);
		g_ic_iout_di[ic_type]->time_para_group = NULL;
	}
	kfree(g_ic_iout_di[ic_type]);
	g_ic_iout_di[ic_type] = NULL;
}

void wlic_iout_init(unsigned int ic_type, const struct device_node *np, int *rx_ic_imax)
{
	int ret;
	struct wlic_iout_dev *di = NULL;

	if (!np || !wltrx_ic_is_type_valid(ic_type))
		return;

	if (g_ic_iout_di[ic_type])
		return;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	ret = wlic_rx_imax_init(di, np, rx_ic_imax);
	if (ret)
		goto exit;

	INIT_DELAYED_WORK(&di->calc_work, wlic_iout_calc_work);

	di->ic_type = ic_type;
	g_ic_iout_di[ic_type] = di;
	return;

exit:
	wlic_kfree_dev(ic_type);
}

void wlic_iout_deinit(unsigned int ic_type)
{
	if (!wltrx_ic_is_type_valid(ic_type) || !g_ic_iout_di[ic_type])
		return;

	cancel_delayed_work(&g_ic_iout_di[ic_type]->calc_work);
	wlic_kfree_dev(ic_type);
}
