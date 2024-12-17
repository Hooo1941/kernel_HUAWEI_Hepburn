// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_rx_fod.c
 *
 * rx fod for wireless charging
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

#include <chipset_common/hwpower/wireless_charge/wireless_rx_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_common.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>
#include <securec.h>

#define HWLOG_TAG wireless_rx_fod
HWLOG_REGIST();

#define WLRX_FOD_STATUS_CFG_ROW           20
#define WLRX_FOD_STATUS_FMT_OLD           0
#define WLRX_FOD_STATUS_FMT_NEW           1
#define WLRX_FOD_QR_FR_MIN                1
#define WLRX_FOD_QR_FR_MAX                255
#define WLRX_FOD_QR_FR_LEGAL_MIN          70
#define WLRX_FOD_QR_FR_LEGAL_MAX          90
#define WLRX_FOD_KM_MIN                   1
#define WLRX_FOD_KM_MAX                   65535

enum wlrx_status_cond {
	WLRX_FOD_STATUS_BEGIN = 0,
	WLRX_FOD_STATUS_ID = WLRX_FOD_STATUS_BEGIN,
	WLRX_FOD_STATUS_SCN,
	WLRX_FOD_STATUS_TX_TYPE,
	WLRX_FOD_STATUS_TH,
	WLRX_FOD_STATUS_END,
};

struct wlrx_status_cfg {
	int id;
	int scn;
	int tx_type;
	int status_th;
};

struct wlrx_fod_dts {
	int ignore_qval;
	unsigned int qval_delayed_time;
	unsigned int cfg_fmt_type;
	int support_qval_cali;
	int qval_err_bat_cycle;
	/* old format */
	unsigned int status_cfg_len;
	u32 status_cfg_old[WLRX_SCN_END];
	/* new format */
	unsigned int status_cfg_row;
	struct wlrx_status_cfg *status_cfg_new;
};

struct wlrx_fod_dev {
	int fod_status;
	struct wlrx_fod_dts *dts;
	struct delayed_work qval_delayed_work;
};

static struct wlrx_fod_dev *g_rx_fod_di[WLTRX_DRV_MAX];

static struct wlrx_fod_dev *wlrx_fod_get_di(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type)) {
		hwlog_err("get_di: drv_type=%u err\n", drv_type);
		return NULL;
	}

	if (!g_rx_fod_di[drv_type] || !g_rx_fod_di[drv_type]->dts) {
		hwlog_err("get_di: drv_type=%u para null\n", drv_type);
		return NULL;
	}

	return g_rx_fod_di[drv_type];
}

static unsigned int wlrx_fod_get_ic_type(unsigned int drv_type)
{
	return wltrx_get_dflt_ic_type(drv_type);
}

static int wlrx_fod_select_new_status_para(struct wlrx_fod_dev *di, enum wlrx_scene scn_id,
	unsigned int tx_type)
{
	int i;

	for (i = 0; i < di->dts->status_cfg_row; i++) {
		if ((di->dts->status_cfg_new[i].scn >= 0) &&
			(di->dts->status_cfg_new[i].scn != scn_id))
			continue;
		if ((di->dts->status_cfg_new[i].tx_type >= 0) &&
			(di->dts->status_cfg_new[i].tx_type != tx_type))
			continue;
		break;
	}
	if (i >= di->dts->status_cfg_row) {
		hwlog_err("select_new_status_para: status_th mismatch\n");
		return -EINVAL;
	}
	hwlog_info("[select_new_status_para] id=%d scn_id=%u tx_type=%u status_th=0x%x\n", i,
		scn_id, tx_type, di->dts->status_cfg_new[i].status_th);
	di->fod_status = di->dts->status_cfg_new[i].status_th;
	return 0;
}

static int wlrx_fod_select_old_status_para(struct wlrx_fod_dev *di, enum wlrx_scene scn_id)
{
	if (di->dts->status_cfg_old[scn_id] <= 0)
		return -EINVAL;

	di->fod_status = di->dts->status_cfg_old[scn_id];
	return 0;
}

static int wlrx_fod_select_status_para(struct wlrx_fod_dev *di, unsigned int tx_type)
{
	enum wlrx_scene scn_id = wlrx_get_scene();

	if ((scn_id < 0) || (scn_id >= WLRX_SCN_END))
		return -EINVAL;

	if (di->dts->cfg_fmt_type == WLRX_FOD_STATUS_FMT_NEW)
		return wlrx_fod_select_new_status_para(di, scn_id, tx_type);
	else if (di->dts->cfg_fmt_type == WLRX_FOD_STATUS_FMT_OLD)
		return wlrx_fod_select_old_status_para(di, scn_id);
	else
		return -EINVAL;
}

static bool wlrx_ignore_fod_status(unsigned int drv_type)
{
	struct wlrx_fod_dev *di = wlrx_fod_get_di(drv_type);

	if (!di || (di->dts->ignore_qval <= 0))
		return false;

	return wlrx_is_qval_err_tx(drv_type);
}

static bool wlrx_need_delayed_fod_status(unsigned int drv_type)
{
	int i;
	struct wlrx_fod_dev *di = wlrx_fod_get_di(drv_type);

	if (!di || (di->dts->qval_delayed_time <= 0))
		return false;

	for (i = 0; i < di->dts->qval_delayed_time / DT_MSLEEP_100MS; i++) {
		power_msleep(DT_MSLEEP_100MS, 0, NULL);
		if (di->dts->qval_delayed_time <= 0)
			return false;
		if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
			di->dts->qval_delayed_time = 0;
			return true;
		}
	}

	return false;
}

static void wlrx_send_noncali_qval_para(unsigned int drv_type, 
	struct wlrx_fod_dev *di, unsigned int tx_type)
{
	int ret;

	if ((di->dts->cfg_fmt_type == WLRX_FOD_STATUS_FMT_NEW) &&
		(di->dts->status_cfg_row <= 0))
		return;

	if ((di->dts->cfg_fmt_type == WLRX_FOD_STATUS_FMT_OLD) &&
		(di->dts->status_cfg_len <= 0))
		return;

	ret = wlrx_fod_select_status_para(di, tx_type);
	if (ret)
		return;

	ret = wireless_send_fod_status(wlrx_fod_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI, di->fod_status);
	if (ret) {
		hwlog_err("send_fod_status: val=0x%x, failed\n", di->fod_status);
		return;
	}

	hwlog_info("[send_fod_status] val=0x%x, succ\n", di->fod_status);
}

static bool wlrx_fod_use_calibrated_qval(unsigned int drv_type, struct wlrx_fod_dev *di)
{
	int i;
	enum wlrx_scene scn_id;
	struct wlprot_acc_cap *acc_cap;

	if (!di->dts->support_qval_cali)
		return false;

	if (wlrx_is_fac_tx(drv_type))
		return true;

	scn_id = wlrx_get_scene();
	if (scn_id != WLRX_SCN_NORMAL) {
		hwlog_err("wlrx_need_use_nv_qval: scn_id=%d\n", scn_id);
		return false;
	}

	acc_cap = wlrx_acc_get_cap(drv_type);
	if (!acc_cap || !acc_cap->support_fod_km) {
		hwlog_err("wlrx_need_use_nv_qval: support_fod_km=%d\n",
			acc_cap->support_fod_km);
		return false;
	}

	return true;
}

static bool wlrx_fod_recheck_cali_val(struct wlrx_fod_dev *di, struct power_nv_fod_qval qval)
{
	int batt_cycle = 0;

	if (di->dts->qval_err_bat_cycle == 0)
		return true;
	(void)power_supply_get_int_property_value(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CYCLE_COUNT, &batt_cycle);
	if ((batt_cycle >= di->dts->qval_err_bat_cycle) &&
		((qval.qr < WLRX_FOD_QR_FR_LEGAL_MIN) || (qval.qr > WLRX_FOD_QR_FR_LEGAL_MAX)))
		return false;

	return true;
}

static bool wlrx_fod_check_qval(struct wlrx_fod_dev *di, struct power_nv_fod_qval qval)
{
	if ((qval.km < WLRX_FOD_KM_MIN) || (qval.km > WLRX_FOD_KM_MAX))
		return false;
	if ((qval.qr < WLRX_FOD_QR_FR_MIN) || (qval.qr > WLRX_FOD_QR_FR_MAX))
		return false;
	if ((qval.fr < WLRX_FOD_QR_FR_MIN) || (qval.fr > WLRX_FOD_QR_FR_MAX))
		return false;
	if (!wlrx_fod_recheck_cali_val(di, qval))
		return false;

	return true;
}

static int wlrx_send_cali_qval_para(unsigned int drv_type, struct wlrx_fod_dev *di)
{
	int ret;
	u32 fod_status;
	struct power_nv_fod_qval qval = { 0 };

	if (!wlrx_fod_use_calibrated_qval(drv_type, di))
		return -ENOTSUPP;

	ret = wlrx_fod_get_cali_para(drv_type, &qval);
	hwlog_info("[get_fod_qval] km=%u, qr=%u, fr=%u, ret=%d\n",
		qval.km, qval.qr, qval.fr, ret);
	if (ret || !wlrx_fod_check_qval(di, qval))
		return -EINVAL;

	ret = wireless_send_fod_km(wlrx_fod_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI, (int)qval.km);
	if (ret) {
		hwlog_err("send_fod_km: failed\n");
		return ret;
	}
	hwlog_info("[send_fod_km] succ\n");

	/* fod_status: qr[8bits] fr[8bits] */
	fod_status = (qval.qr << 8) + qval.fr;
	ret = wireless_send_fod_status(wlrx_fod_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI, (int)fod_status);
	hwlog_info("[send_fod_status] val=0x%x, ret=%d\n", fod_status, ret);

	return ret;
}

int wlrx_fod_get_cali_para(unsigned int drv_type, struct power_nv_fod_qval *qval)
{
	struct wlrx_fod_dev *di = wlrx_fod_get_di(drv_type);

	if (!di)
		return -EINVAL;

	return power_common_nv_read(POWER_NV_COMMON_QVAL, (void *)qval);
}

int wlrx_fod_set_cali_para(unsigned int drv_type, struct power_nv_fod_qval qval)
{
	struct wlrx_fod_dev *di = wlrx_fod_get_di(drv_type);

	if (!di)
		return -EINVAL;

	return power_common_nv_write(POWER_NV_COMMON_QVAL, (void *)&qval);
}

void wlrx_send_fod_status(unsigned int drv_type, unsigned int tx_type)
{
	struct wlrx_fod_dev *di = wlrx_fod_get_di(drv_type);

	if (!di || !di->dts)
		return;

	if (wlrx_ignore_fod_status(drv_type)) {
		wlrx_preproccess_fod_status();
		return;
	}

	if (wlrx_need_delayed_fod_status(drv_type))
		return;

	if (wlrx_send_cali_qval_para(drv_type, di))
		wlrx_send_noncali_qval_para(drv_type, di, tx_type);
}

static void wlrx_qval_delayed_timeout_work(struct work_struct *work)
{
	struct wlrx_fod_dev *di = container_of(work,
		struct wlrx_fod_dev, qval_delayed_work.work);

	if (!di) {
		hwlog_err("qval_delayed_timeout_work: para null\n");
		return;
	}

	di->dts->qval_delayed_time = 0;
	hwlog_info("[qval_delayed_timeout_work] qval_delayed_time cleared\n");
}

static int wlrx_parse_new_fod_status(const struct device_node *np, struct wlrx_fod_dts *dts)
{
	int i, len;

	dts->status_cfg_new = kcalloc(dts->status_cfg_row, sizeof(*(dts->status_cfg_new)), GFP_KERNEL);
	if (!dts->status_cfg_new)
		goto parse_fod_status_fail;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"fod_status_new", (int *)dts->status_cfg_new, dts->status_cfg_row, WLRX_FOD_STATUS_END);
	if (len <= 0)
		goto parse_status_cfg_fail;

	for (i = 0; i < dts->status_cfg_row; i++)
		hwlog_info("rx_status_cond[%d] id=%d scn=%d tx_type=%d status_th=0x%x\n", i,
			dts->status_cfg_new[i].id, dts->status_cfg_new[i].scn,
			dts->status_cfg_new[i].tx_type, dts->status_cfg_new[i].status_th);

	return 0;

parse_status_cfg_fail:
	kfree(dts->status_cfg_new);
parse_fod_status_fail:
	dts->status_cfg_row = 0;
	hwlog_err("parse_new_fod_status: failed\n");
	return -EINVAL;
}

static int wlrx_parse_old_fod_status(const struct device_node *np, struct wlrx_fod_dts *dts)
{
	int i;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"fod_status", (u32 *)dts->status_cfg_old, dts->status_cfg_len)) {
		dts->status_cfg_len = 0;
		return -EINVAL;
	}
	for (i = 0; i < dts->status_cfg_len; i++)
		hwlog_info("fod_status[%d]=0x%x\n", i, dts->status_cfg_old[i]);

	return 0;
}

static int wlrx_parse_fod_status(const struct device_node *np, struct wlrx_fod_dts *dts)
{
	int len;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"fod_status_new", WLRX_FOD_STATUS_CFG_ROW, WLRX_FOD_STATUS_END);
	if (len > 0) {
		dts->status_cfg_row = len / WLRX_FOD_STATUS_END;
		dts->cfg_fmt_type = WLRX_FOD_STATUS_FMT_NEW;
		return wlrx_parse_new_fod_status(np, dts);
	}

	len = of_property_count_u32_elems(np, "fod_status");
	if ((len <= 0) || (len > WLRX_SCN_END)) {
		dts->status_cfg_len = 0;
		return 0;
	}
	dts->status_cfg_len = len;
	dts->cfg_fmt_type = WLRX_FOD_STATUS_FMT_OLD;
	return wlrx_parse_old_fod_status(np, dts);
}

static int wlrx_fod_parse_dts(const struct device_node *np, struct wlrx_fod_dts **dts)
{
	int ret;

	*dts = kzalloc(sizeof(**dts), GFP_KERNEL);
	if (!*dts)
		return -ENOMEM;

	ret = wlrx_parse_fod_status(np, *dts);
	if (ret)
		return ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ignore_qval", &(*dts)->ignore_qval, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"qval_delayed_time", &(*dts)->qval_delayed_time, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_qval_cali", &(*dts)->support_qval_cali, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"qval_err_bat_cycle", &(*dts)->qval_err_bat_cycle, 0);

	return 0;
}

static void wlrx_fod_kfree_dev(struct wlrx_fod_dev *di)
{
	if (!di)
		return;

	kfree(di->dts);
	kfree(di);
}

static void wlrx_fod_free_resources(struct wlrx_fod_dev *di)
{
	if (!di)
		return;

	if (di->dts->qval_delayed_time)
		cancel_delayed_work_sync(&di->qval_delayed_work);
}

static int wlrx_fod_debug_set(struct wlrx_fod_dev *di, int fod_status)
{
	int i;

	if ((di->dts->cfg_fmt_type == WLRX_FOD_STATUS_FMT_NEW) &&
		(di->dts->status_cfg_row > 0)) {
		di->dts->status_cfg_new[0].scn = -1;
		di->dts->status_cfg_new[0].tx_type = -1;
		di->dts->status_cfg_new[0].status_th = fod_status;
		return 0;
	}

	di->dts->cfg_fmt_type == WLRX_FOD_STATUS_FMT_OLD;
	for (i = WLRX_SCN_BEGIN; i < WLRX_SCN_END; i++)
		di->dts->status_cfg_old[i] = fod_status;

	hwlog_info("[fod_debug_set] fod_status=0x%x\n", fod_status);
	return 0;
}

static int wlrx_fod_debug_get(struct wlrx_fod_dev *di, char *buf, size_t size)
{
	int i;
	int len = 0;

	if (di->dts->cfg_fmt_type == WLRX_FOD_STATUS_FMT_NEW) {
		len += snprintf_s(buf + len, size - len, size - len - 1, "id scn tx_type status_th\n");
		for (i = 0; i < di->dts->status_cfg_row; i++)
			len += snprintf_s(buf + len, size - len, size - len - 1, "%d %d %d 0x%x\n",
				di->dts->status_cfg_new[i].id, di->dts->status_cfg_new[i].scn,
				di->dts->status_cfg_new[i].tx_type, di->dts->status_cfg_new[i].status_th);
		return len;
	}

	len += snprintf_s(buf + len, size - len, size - len - 1, "scn val\n");
	for (i = WLRX_SCN_BEGIN; i < WLRX_SCN_END; i++)
		len += snprintf_s(buf + len, size - len, size - len - 1, "%d 0x%x\n",
			i, di->dts->status_cfg_old[i]);

	return len;
}

static ssize_t wlrx_fod_debug_store(void *dev_data, const char *buf, size_t size)
{
	int ret;
	int fod_status;
	struct wlrx_fod_dev *di = NULL;

	hwlog_info("debug_store\n");
	di = (struct wlrx_fod_dev *)dev_data;
	if (!di || !di->dts)
		return -EINVAL;

	/* 1: input para num */
	if (sscanf_s(buf, "%x", &fod_status) != 1) {
		hwlog_err("fod_status_store: unable to parse input %s\n", buf);
		return -EINVAL;
	}

	ret = wlrx_fod_debug_set(di, fod_status);
	if (ret)
		hwlog_err("fod_status_store: set fail\n");
	return (ssize_t)size;
}

static ssize_t wlrx_fod_debug_show(void *dev_data, char *buf, size_t size)
{
	struct wlrx_fod_dev *di = NULL;

	hwlog_info("debug_show\n");
	di = (struct wlrx_fod_dev *)dev_data;
	if (!di || !di->dts)
		return -EINVAL;

	return wlrx_fod_debug_get(di, buf, size);
}

int wlrx_fod_init(unsigned int drv_type, struct device *dev)
{
	int ret;
	struct wlrx_fod_dev *di = NULL;

	if (!dev || !wltrx_is_drv_type_valid(drv_type))
		return -EINVAL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	ret = wlrx_fod_parse_dts(dev->of_node, &di->dts);
	if (ret)
		goto exit;

	if (di->dts->qval_delayed_time) {
		INIT_DELAYED_WORK(&di->qval_delayed_work, wlrx_qval_delayed_timeout_work);
		schedule_delayed_work(&di->qval_delayed_work,
			msecs_to_jiffies(di->dts->qval_delayed_time));
	}

	power_dbg_ops_register("wlrx", "fod_status", di, wlrx_fod_debug_show, wlrx_fod_debug_store);
	g_rx_fod_di[drv_type] = di;
	return 0;

exit:
	wlrx_fod_kfree_dev(di);
	return ret;
}

void wlrx_fod_deinit(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type))
		return;

	wlrx_fod_free_resources(g_rx_fod_di[drv_type]);
	wlrx_fod_kfree_dev(g_rx_fod_di[drv_type]);
	g_rx_fod_di[drv_type] = NULL;
}
