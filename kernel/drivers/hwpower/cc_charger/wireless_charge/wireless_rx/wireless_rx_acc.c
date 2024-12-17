// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_rx_acc.c
 *
 * accessory(tx,cable,adapter etc.) for wireless charging
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/wireless_charge/wireless_acc_types.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_auth.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pmode.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_common.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pctrl.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_ui_ne.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

#define HWLOG_TAG wireless_rx_acc
HWLOG_REGIST();

#define WLRX_ACC_AUTH_MIN_KID          0
#define WLRX_ACC_AUTH_MAX_KID          10
#define WLRX_ACC_AUTH_FAC_KID          0 /* factory key id */
#define WLRX_ACC_AUTH_DFLT_KID         1
#define WLRX_ACC_QVAL_ERR_TX_NUM_DEF   2
#define WLRX_ACC_TX_EFF_DEFAULT        100
#define WLRX_ACC_PREVFOD_CHK_DELAY     500
#define WLRX_ACC_PREVFOD_CHK_INTERVAL  100
#define WLRX_ACC_PREVFOD_CHK_CNT       10
#define WLRX_ACC_PREVFOD_CHK_ERR_CNT   4
#define WLRX_ACC_PREVFOD_OFFSET_VAL    0xf0

enum wlrx_acc_det_status {
	WLRX_ACC_DET_DEFAULT,
	WLRX_ACC_DET_ADAPTER_OR_CABLE_MISMATCH,
	WLRX_ACC_DET_ADAPTER_MISMATCH,
	WLRX_ACC_DET_CABLE_MISMATCH,
	WLRX_ACC_DET_ADAPTER_AND_CABLE_MISMATCH,
};

enum wlrx_acc_tx_ilim_cfg {
	WLRX_ACC_TX_ILIM_BEGIN = 0,
	WLRX_ACC_TX_ILIM_TX_TYPE = WLRX_ACC_TX_ILIM_BEGIN,
	WLRX_ACC_TX_ILIM_DEF,
	WLRX_ACC_TX_ILIM_OCP,
	WLRX_ACC_TX_ILIM_END,
};

enum wlrx_acc_ept_type {
	WLRX_ACC_EPT_OCP = 0xa,
	WLRX_ACC_EPT_DEFAULT = 0xff,
};

struct wlrx_acc_dft_prop {
	int adp_type;
	const char *name;
	int vmax;
	int imax;
};

struct wlrx_acc_tx_ilim {
	u32 tx_type;
	u32 dflt; /* default tx curr max */
	u32 ocp; /* tx curr max when tx ocp happen */
};

struct wlrx_acc_fix_tx_fop {
	int value;
	int tx_type;
};

struct wlrx_acc_scx_lrx {
	unsigned int voltage; /* unit:mv */
	unsigned int lrx_min; /* unit:0.01uH */
	unsigned int lrx_max; /* unit:0.01uH */
	unsigned int path_ratio;
};

struct wlrx_acc_dts {
	struct wlrx_acc_tx_ilim *tx_ilim;
	struct wlrx_acc_fix_tx_fop fix_tx_fop;
	int tx_ilim_len;
	int kid;
	int support_high_pwr_wltx;
	int match_det_pmax_lth;
	int eff_para[WLRX_SCN_END];
	u8 *qval_err_tx;
	int qval_err_tx_len;
	int support_prev_fodchk;
	unsigned int coil_inductance;
	int prevfod_vrect_min;
};

struct wlrx_acc_dbg {
	int kid;
};

struct wlrx_acc_dev {
	struct wlprot_acc_cap *cap;
	struct wlrx_acc_dts *dts;
	struct wlrx_acc_dbg *dbg;
	struct delayed_work prevfod_chk_work;
	bool standard_tx; /* handshake succ */
	enum wlacc_tx_type tx_type;
	u8 *fw_ver;
	int kid;
	int hs_cnt; /* hs: handshake */
	int det_cap_cnt;
	int auth_cnt;
	bool auth_succ;
	bool auth_need_rechk;
	bool send_product_id_succ;
};

static struct wlrx_acc_dev *g_rx_acc_di[WLTRX_DRV_MAX];

static struct wlrx_acc_dev *wlrx_acc_get_di(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type)) {
		hwlog_err("get_di: drv_type=%u err\n", drv_type);
		return NULL;
	}

	if (!g_rx_acc_di[drv_type] || !g_rx_acc_di[drv_type]->cap || !g_rx_acc_di[drv_type]->dts) {
		hwlog_err("get_di: drv_type=%u para null\n", drv_type);
		return NULL;
	}

	return g_rx_acc_di[drv_type];
}

static unsigned int wlrx_acc_get_ic_type(unsigned int drv_type)
{
	return wltrx_get_dflt_ic_type(drv_type);
}

static struct wlrx_acc_dft_prop g_acc_dtf_prop[] = {
	{ WLACC_ADP_SDP,     "SDP",      5000, 475  },
	{ WLACC_ADP_CDP,     "CDP",      5000, 1000 },
	{ WLACC_ADP_NON_STD, "NON-STD",  5000, 1000 },
	{ WLACC_ADP_DCP,     "DCP",      5000, 1000 },
	{ WLACC_ADP_FCP,     "FCP",      9000, 2000 },
	{ WLACC_ADP_SCP,     "SCP",      5000, 1000 },
	{ WLACC_ADP_PD,      "PD",       5000, 1000 },
	{ WLACC_ADP_QC,      "QC",       5000, 1000 },
	{ WLACC_ADP_OTG_A,   "OTG_A",    5000, 475  },
	{ WLACC_ADP_OTG_B,   "OTG_B",    5000, 475  },
	{ WLACC_ADP_ERR,     "ERR",      5000, 1000 },
};

static struct wlrx_acc_scx_lrx g_acc_scx_lrx[] = {
	{ 36000,  250, 1000, 4 },
	{ 54000,  90,  120,  4 },
	{ 72000,  50,  70,   4 },
};

static int wlrx_acc_redef_adp_type(int type)
{
	if ((type >= WLACC_ADP_FAC_BASE) && (type <= WLACC_ADP_FAC_MAX))
		type %= WLACC_ADP_FAC_BASE;
	else if ((type >= WLACC_ADP_CAR_BASE) && (type <= WLACC_ADP_CAR_MAX))
		type %= WLACC_ADP_CAR_BASE;
	else if ((type >= WLACC_ADP_PWR_BANK_BASE) && (type <= WLACC_ADP_PWR_BANK_MAX))
		type %= WLACC_ADP_PWR_BANK_BASE;

	return type;
}

bool wlrx_is_std_tx(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return false;

	return di->standard_tx;
}

bool wlrx_is_err_tx(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return true;

	return wlacc_is_err_tx(di->cap->adp_type);
}

bool wlrx_is_fac_tx(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return false;

	return wlacc_is_fac_tx(di->cap->adp_type);
}

bool wlrx_is_car_tx(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return false;

	return wlacc_is_car_tx(di->cap->adp_type);
}

bool wlrx_is_rvs_tx(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return false;

	return wlacc_is_rvs_tx(di->cap->adp_type);
}

bool wlrx_is_qc_adp(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return false;

	return wlacc_is_qc_adp(wlrx_acc_redef_adp_type(di->cap->adp_type));
}

static bool wlrx_acc_is_highpwr_rvs_tx(struct wlrx_acc_dev *di)
{
	int max_pwr_mw;

	if (!wlacc_is_rvs_tx(di->cap->adp_type))
		return false;

	max_pwr_mw = di->cap->vmax / POWER_MV_PER_V * di->cap->imax;
	return max_pwr_mw >= 27000; /* high pwr threshold: 27w */
}

bool wlrx_is_highpwr_rvs_tx(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return false;

	return wlrx_acc_is_highpwr_rvs_tx(di);
}

bool wlrx_is_qval_err_tx(unsigned int drv_type)
{
	unsigned int i;
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di || !di->dts)
		return false;

	for (i = 0; i < di->dts->qval_err_tx_len; i++) {
		if (di->tx_type == di->dts->qval_err_tx[i])
			return true;
	}

	return false;
}

bool wlrx_is_low_vpa_car_tx(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di || !di->cap)
		return false;

	return (di->tx_type == WLACC_TX_CK30_LP) && di->cap->dc_pwr;
}

struct wlprot_acc_cap *wlrx_acc_get_cap(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return NULL;

	return di->cap;
}

static int wlrx_acc_get_tx_eff(struct wlrx_acc_dev *di)
{
	enum wlrx_scene scn_id = wlrx_get_scene();

	if (!di->dts || (scn_id >= WLRX_SCN_END) || (scn_id < WLRX_SCN_BEGIN))
		return POWER_PERCENT;

	return di->dts->eff_para[scn_id];
}

static int wlrx_acc_get_ept_type(struct wlrx_acc_dev *di, unsigned int ic_type)
{
	u16 ept_type = 0;

	if (!di->cap->support_get_ept || wireless_get_ept_type(ic_type,
		WIRELESS_PROTOCOL_QI, &ept_type))
		return WLRX_ACC_EPT_DEFAULT;

	hwlog_info("[get_ept_type] type=0x%x\n", ept_type);
	return ept_type;
}

static int wlrx_acc_get_tx_ilim(struct wlrx_acc_dev *di, unsigned int ic_type)
{
	int i;
	int ept_type;
	int ilim = 0;
	struct wlrx_acc_tx_ilim *tx_ilim = NULL;

	if (!di->dts || !di->dts->tx_ilim)
		return 0;

	tx_ilim = di->dts->tx_ilim;
	ept_type = wlrx_acc_get_ept_type(di, ic_type);
	for (i = 0; i < di->dts->tx_ilim_len; i++) {
		if (tx_ilim[i].tx_type == di->tx_type) {
			if ((tx_ilim[i].ocp > 0) && (ept_type == WLRX_ACC_EPT_OCP))
				ilim = tx_ilim[i].ocp;
			else
				ilim = tx_ilim[i].dflt;
			break;
		}
	}

	return ilim;
}

static int wlrx_acc_get_tx_pwr_ratio(struct wlrx_acc_dev *di)
{
	s64 tx_max_power;

	tx_max_power = (s64)di->cap->vmax * di->cap->imax;
	if (tx_max_power >= WLRX_ACC_TX_PWR_OVER_SCX_TH)
		di->cap->pwr_ratio = WLRX_ACC_TX_PWR_RATIO_OVER_SCX;
	else if (tx_max_power >= WLRX_ACC_TX_PWR_SCX_TH)
		di->cap->pwr_ratio = WLRX_ACC_TX_PWR_RATIO_SCX;
	else
		di->cap->pwr_ratio = WLRX_ACC_TX_PWR_RATIO_DFLT;

	hwlog_info("[get_tx_pwr_ratio] pwr_ratio=%d\n", di->cap->pwr_ratio);
	return di->cap->pwr_ratio;
}

int wlrx_acc_get_tx_imax(unsigned int drv_type, unsigned int path_ratio)
{
	s64 tmp;
	int tx_eff;
	int tx_ilim;
	int ilim_by_pwr;
	int tx_imax;
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di || !di->cap)
		return 0;

	tmp = (s64)(di->cap->vmax / POWER_PERCENT) * di->cap->imax * di->cap->pwr_ratio;
	tx_eff = wlrx_acc_get_tx_eff(di);
	ilim_by_pwr = (int)tmp / (WLRX_VBAT_BASE * path_ratio) * tx_eff / POWER_PERCENT;
	tx_ilim = wlrx_acc_get_tx_ilim(di, wlrx_acc_get_ic_type(drv_type));
	tx_imax = power_min_positive(ilim_by_pwr, tx_ilim);
	hwlog_info("[get_tx_imax] ilim_by_pwr=%d tx_ilim=%d tx_imax=%d pwr_ratio=%d\n",
		ilim_by_pwr, tx_ilim, tx_imax, di->cap->pwr_ratio);
	return tx_imax;
}

bool wlrx_acc_support_fix_fop(unsigned int drv_type, int fop)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di || (!di->cap->support_fop_range && (fop > WLRX_ACC_NORMAL_MAX_FOP)))
		return false;
	if (di->cap->support_fop_range && (fop > 0) &&
		!(((fop >= di->cap->fop_range.base_min) && (fop <= di->cap->fop_range.base_max)) ||
		((fop >= di->cap->fop_range.ext1_min) && (fop <= di->cap->fop_range.ext1_max)) ||
		((fop >= di->cap->fop_range.ext2_min) && (fop <= di->cap->fop_range.ext2_max))))
		return false;

	return true;
}

int wlrx_acc_get_tx_fixed_fop(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di || !di->dts || (di->dts->fix_tx_fop.value <= 0))
		return -EINVAL;

	if ((di->dts->fix_tx_fop.tx_type < 0) || (di->tx_type == di->dts->fix_tx_fop.tx_type))
		return di->dts->fix_tx_fop.value;

	return -EINVAL;
}

static struct wlrx_acc_dft_prop *wlrx_acc_get_dft_prop(int type)
{
	unsigned int i;

	type = wlrx_acc_redef_adp_type(type);
	for (i = 0; i < ARRAY_SIZE(g_acc_dtf_prop); i++) {
		if (type == g_acc_dtf_prop[i].adp_type)
			return &g_acc_dtf_prop[i];
	}

	return NULL;
}

static void wlrx_acc_set_dflt_cap(struct wlrx_acc_dev *di)
{
	di->cap->adp_type = WLACC_ADP_ERR;
	di->cap->vmax = 5000; /* default: 5V */
	di->cap->imax = 1000; /* default: 1A */
	di->cap->pwr_ratio = WLRX_ACC_TX_PWR_RATIO_DFLT; /* default: 75% */
	di->cap->can_boost = 0;
	di->cap->cable_ok = 0;
	di->cap->no_need_cert = 1;
	di->cap->support_scp = 0;
	di->cap->support_extra_cap = 0;
	/* extra cap */
	di->cap->support_fan = 0;
	di->cap->support_tec = 0;
	di->cap->support_get_ept = 0;
	di->cap->support_fod_status = 0;
	di->cap->support_fop_range = 0;
	di->cap->fop_range.base_min = 0;
	di->cap->fop_range.base_max = 0;
	di->cap->fop_range.ext1_min = 0;
	di->cap->fop_range.ext1_max = 0;
	di->cap->fop_range.ext2_min = 0;
	di->cap->fop_range.ext2_max = 0;
}

static void wlrx_acc_revise_cap(struct wlrx_acc_dev *di)
{
	struct wlrx_acc_dft_prop *dft_prop = NULL;

	if (!di->dts->support_high_pwr_wltx && wlrx_acc_is_highpwr_rvs_tx(di))
		di->cap->no_need_cert = true;

	dft_prop = wlrx_acc_get_dft_prop(di->cap->adp_type);
	if (!dft_prop)
		return;

	if (di->cap->vmax <= 0) {
		di->cap->vmax = dft_prop->vmax;
		hwlog_info("[revise_cap] vmax=%d\n", di->cap->vmax);
	}
	if (di->cap->imax <= 0) {
		di->cap->imax = dft_prop->imax;
		hwlog_info("[revise_cap] imax=%d\n", di->cap->imax);
	}
}

static void wlrx_acc_send_fan_status(struct wlrx_acc_dev *di)
{
	int fan_status = 1; /* 1: fan exist */

	if (di->cap->support_fan)
		power_ui_event_notify(POWER_UI_NE_WL_FAN_STATUS, &fan_status);
}

static void wlrx_acc_get_fop_range(struct wlrx_acc_dev *di, unsigned int ic_type)
{
	if (!di->cap->support_fop_range)
		return;

	(void)wireless_get_tx_fop_range(ic_type, WIRELESS_PROTOCOL_QI, &di->cap->fop_range);
}

unsigned int wlrx_acc_get_tx_type(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return WLACC_TX_UNKNOWN;

	return di->tx_type;
}

static void wlrx_acc_get_info(struct wlrx_acc_dev *di, unsigned int ic_type)
{
	if (!di->standard_tx || wlacc_is_fac_tx(di->cap->adp_type))
		return;

	if (!di->fw_ver)
		di->fw_ver = wireless_get_tx_fw_version(ic_type, WIRELESS_PROTOCOL_QI);
	if (di->tx_type == WLACC_TX_UNKNOWN)
		di->tx_type = wireless_get_tx_type(ic_type, WIRELESS_PROTOCOL_QI);
	if (wlacc_is_rvs_tx(di->cap->adp_type))
		di->tx_type = WLACC_TX_PHONE;
	if ((di->tx_type == WLACC_TX_W081) || (di->tx_type == WLACC_TX_K080)) {
		if (di->cap->vpa_max < WLTRX_VOLT_36V)
			di->cap->vpa_max = WLTRX_VOLT_36V;
		di->cap->support_fod_km = true;
	}
	hwlog_info("[get_info] tx_fw_ver: %s tx_type=%d\n", di->fw_ver, di->tx_type);
	wireless_get_tx_bigdata_info(ic_type, WIRELESS_PROTOCOL_QI);
}

static void wlrx_acc_send_fod_status(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di || !di->cap->support_fod_status)
		return;

	wlrx_send_fod_status(drv_type, di->tx_type);
}

static int wlrx_acc_get_det_min_pwr(struct wlrx_acc_dev *di)
{
	return di->dts->match_det_pmax_lth * POWER_PERCENT / di->cap->pwr_ratio;
}

static bool wlrx_acc_need_det_match_status(unsigned int drv_type)
{
	int det_min_pwr;
	int acc_max_pwr; /* cmd: 0x41 */
	int tx_max_pwr; /* cmd: 0x61 */
	int product_max_pwr;
	struct wlrx_dts *dts = wlrx_get_dts();
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!dts || !di)
		return false;

	det_min_pwr = wlrx_acc_get_det_min_pwr(di);
	if (!di->standard_tx || wlacc_is_rvs_tx(di->cap->adp_type) ||
		wlacc_is_fac_tx(di->cap->adp_type))
		return false;

	product_max_pwr = wlrx_pctrl_get_product_pmax(drv_type);
	if (product_max_pwr < di->dts->match_det_pmax_lth)
		return false;

	acc_max_pwr = di->cap->vmax * di->cap->imax / POWER_MV_PER_V;
	hwlog_info("acc_det_pwr=%d tx_pwr=%d standard_tx=%d\n",
		di->dts->match_det_pmax_lth, acc_max_pwr, di->standard_tx);
	if (acc_max_pwr >= det_min_pwr)
		return false;

	if (wireless_get_tx_max_power(wlrx_acc_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI, &tx_max_pwr))
		return false;

	tx_max_pwr = tx_max_pwr * di->cap->pwr_ratio / POWER_PERCENT;
	if (tx_max_pwr < di->dts->match_det_pmax_lth)
		return false;

	return true;
}

static unsigned int wlrx_acc_det_match_status(unsigned int drv_type)
{
	int ret;
	int vadp = 0;
	int iadp = 0;
	int cable_type = 0;
	int cable_iout = 0;
	int det_min_pwr;
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return WLRX_ACC_DET_DEFAULT;

	det_min_pwr = wlrx_acc_get_det_min_pwr(di);
	ret = wireless_get_tx_adapter_capability(wlrx_acc_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI, &vadp, &iadp);
	ret += wireless_get_tx_cable_type(wlrx_acc_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI, &cable_type, &cable_iout);
	if (ret)
		return WLRX_ACC_DET_ADAPTER_OR_CABLE_MISMATCH;
	else if (vadp * iadp / POWER_UW_PER_MW < det_min_pwr)
		return WLRX_ACC_DET_ADAPTER_MISMATCH;
	else if ((vadp > 0) && (cable_iout < det_min_pwr * POWER_UW_PER_MW / vadp))
		return WLRX_ACC_DET_CABLE_MISMATCH;
	else
		return WLRX_ACC_DET_DEFAULT;
}

static void wlrx_acc_notify_match_status(int status)
{
	power_ui_event_notify(POWER_UI_NE_WL_ACC_STATUS, &status);
}

static void wlrx_acc_send_match_status(unsigned int drv_type)
{
	if (!wlrx_acc_need_det_match_status(drv_type))
		return;

	wlrx_acc_notify_match_status(wlrx_acc_det_match_status(drv_type));
}

static void wlrx_acc_update_icon_display(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return;

	if (di->cap->no_need_cert)
		wireless_charge_icon_display(WLRX_PMODE_NORMAL_JUDGE);
	else
		wireless_charge_icon_display(WLRX_PMODE_QUICK_JUDGE);
}

bool wlrx_acc_auth_need_recheck(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return false;

	return di->auth_need_rechk && wlrx_auth_srv_ready();
}

bool wlrx_acc_auth_succ(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return false;

	return di->auth_succ;
}

static void wlrx_acc_send_auth_confirm_msg(unsigned int drv_type, bool flag)
{
	if (!wlrx_is_fac_tx(drv_type))
		return;

	if (!wireless_send_cert_confirm(wlrx_acc_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI, flag))
		hwlog_info("[send_confirm_msg] succ\n");
}

int wlrx_acc_auth(unsigned int drv_type)
{
	int ret;
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return WLRX_ACC_AUTH_DEV_ERR;

	if (di->auth_cnt >= WLRX_ACC_AUTH_CNT) {
		di->auth_succ = false;
		hwlog_err("auth: exceed %d times\n", WLRX_ACC_AUTH_CNT);
		wlrx_acc_send_auth_confirm_msg(drv_type, false);
		return WLRX_ACC_AUTH_CNT_ERR;
	}

	di->auth_need_rechk = false;

	hwlog_info("[auth] kid=%d\n", di->kid);
	ret = wlrx_auth_handler(wlrx_acc_get_ic_type(drv_type), WIRELESS_PROTOCOL_QI, di->kid);
	if (ret == WLRX_ACC_AUTH_SRV_NOT_READY) {
		di->auth_need_rechk = true;
		return ret;
	}
	if (ret != WLRX_ACC_AUTH_SUCC) {
		di->auth_cnt++;
		return ret;
	}

	di->auth_succ = true;
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_AUTH_SUCC, NULL);
	wlrx_acc_send_auth_confirm_msg(drv_type, true);
	hwlog_info("[auth] succ\n");
	return WLRX_ACC_AUTH_SUCC;
}

static void wlrx_acc_prevfod_chk_work(struct work_struct *work)
{
	int i;
	int err_cnt = 0;
	int vrect = 0;
	struct wlrx_acc_dev *di = container_of(work,
		struct wlrx_acc_dev, prevfod_chk_work.work);

	if (!di || (di->dts->prevfod_vrect_min <= 0))
		return;

	for (i = 0; i < WLRX_ACC_PREVFOD_CHK_CNT; i++) {
		wlrx_ic_get_vrect(WLTRX_IC_MAIN, &vrect);
		if (vrect <= di->dts->prevfod_vrect_min)
			err_cnt++;

		if (err_cnt > WLRX_ACC_PREVFOD_CHK_ERR_CNT) {
			hwlog_info("[prevfod_chk] vrect is error, count=%d\n", err_cnt);
			wlrx_ic_set_fod_offset(WLTRX_IC_MAIN, WLRX_ACC_PREVFOD_OFFSET_VAL);
			break;
		}
		msleep(WLRX_ACC_PREVFOD_CHK_INTERVAL);
	}
}

static void wlrx_acc_start_prevfod_check(struct wlrx_acc_dev *di)
{
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!di->dts->support_prev_fodchk || !wlacc_is_rvs_tx(di->cap->adp_type))
		return;
	if (dts && !di->send_product_id_succ)
		di->send_product_id_succ = !wireless_send_rx_product_id(WLTRX_IC_MAIN,
			WIRELESS_PROTOCOL_QI, dts->product_id);
	if (wireless_send_start_prevfod_check(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI))
		return;
	charge_set_hiz_enable(HIZ_MODE_ENABLE);
	wireless_charge_chip_init(WIRELESS_FODCHK_CHIP_INIT);
	cancel_delayed_work_sync(&di->prevfod_chk_work);
	schedule_delayed_work(&di->prevfod_chk_work,
		msecs_to_jiffies(WLRX_ACC_PREVFOD_CHK_DELAY));
	(void)power_msleep(DT_MSLEEP_5S, DT_MSLEEP_50MS, wlrx_msleep_exit);
	charge_set_hiz_enable(HIZ_MODE_DISABLE);
	wireless_charge_chip_init(WIRELESS_CHIP_INIT);
	(void)power_msleep(DT_MSLEEP_5S, DT_MSLEEP_50MS, wlrx_msleep_exit);
}

static void wlrx_acc_cap_post_handler(unsigned int drv_type)
{
	unsigned int ic_type;
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return;

	ic_type = wlrx_acc_get_ic_type(drv_type);
	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_TX_CAP_SUCC, NULL);
	wlrx_acc_update_icon_display(drv_type);
	wlrx_acc_start_prevfod_check(di);
	wlrx_acc_send_fan_status(di);
	wlrx_acc_get_fop_range(di, ic_type);
	wlrx_acc_get_ept_type(di, ic_type);
	wlrx_acc_get_info(di, ic_type);
	wlrx_acc_send_match_status(drv_type);
	wlrx_acc_send_fod_status(drv_type);
}

static int wlrx_acc_cap_prev_handler(unsigned int drv_type)
{
	unsigned int ic_type;
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return WLRX_ACC_DET_CAP_DEV_ERR;

	ic_type = wlrx_acc_get_ic_type(drv_type);
	if (di->det_cap_cnt >= WLRX_ACC_DET_CAP_CNT) {
		hwlog_err("cap_prev_handler: detect exceed %d times\n", WLRX_ACC_DET_CAP_CNT);
		wlrx_acc_set_dflt_cap(di);
		wlrx_acc_get_info(di, ic_type);
		wlrx_acc_send_fod_status(drv_type);
		return WLRX_ACC_DET_CAP_CNT_ERR;
	}

	(void)wireless_get_tx_capability(ic_type, WIRELESS_PROTOCOL_QI, di->cap);
	(void)wlrx_acc_get_tx_pwr_ratio(di);
	if (wlacc_is_err_tx(di->cap->adp_type)) {
		++di->det_cap_cnt;
		return WLRX_ACC_DET_CAP_CM_ERR;
	}

	wlrx_acc_revise_cap(di);
	return WLRX_ACC_DET_CAP_SUCC;
}

int wlrx_acc_detect_cap(unsigned int drv_type)
{
	int ret = wlrx_acc_cap_prev_handler(drv_type);

	if (ret != WLRX_ACC_DET_CAP_SUCC)
		return ret;

	wlrx_acc_cap_post_handler(drv_type);
	hwlog_info("[detect_cap] succ\n");
	return WLRX_ACC_DET_CAP_SUCC;
}

int wlrx_acc_handshake(unsigned int drv_type)
{
	int tx_id;
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!di || !dts)
		return WLRX_ACC_HS_DEV_ERR;

	if (di->hs_cnt >= WLRX_ACC_HS_CNT) {
		hwlog_err("handshake: exceed %d times\n", WLRX_ACC_HS_CNT);
		wlrx_acc_set_dflt_cap(di);
		return WLRX_ACC_HS_CNT_ERR;
	}

	di->send_product_id_succ = !wireless_send_rx_product_id(wlrx_acc_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI, dts->product_id);
	tx_id = wireless_get_tx_id(wlrx_acc_get_ic_type(drv_type),
		WIRELESS_PROTOCOL_QI);
	if (tx_id < 0) {
		++di->hs_cnt;
		return WLRX_ACC_HS_CM_ERR;
	} else if (tx_id != WLRX_ACC_HW_ID) {
		hwlog_err("handshake: err_id=0x%x\n", tx_id);
		wlrx_acc_set_dflt_cap(di);
		return WLRX_ACC_HS_ID_ERR;
	}

	power_event_bnc_notify(POWER_BNT_WLC, POWER_NE_WLC_HS_SUCC, NULL);
	di->standard_tx = true;
	hwlog_info("[handshake] succ\n");
	return WLRX_ACC_HS_SUCC;
}

void wlrx_acc_reset_para(unsigned int drv_type)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);

	if (!di)
		return;

	wlrx_acc_set_dflt_cap(di);
	di->standard_tx = false;
	di->tx_type = WLACC_TX_UNKNOWN;
	di->fw_ver = NULL;
	di->hs_cnt = 0;
	di->det_cap_cnt = 0;
	di->auth_cnt = 0;
	di->auth_succ = false;
	di->auth_need_rechk = false;
	di->send_product_id_succ = false;
	wlrx_acc_notify_match_status(WLRX_ACC_DET_DEFAULT);
}

bool wlrx_can_enter_scx(unsigned int drv_type, unsigned int path_ratio)
{
	struct wlrx_acc_dev *di = wlrx_acc_get_di(drv_type);
	unsigned int i;
	unsigned int lrx;

	if (!di)
		return false;

	lrx = di->dts->coil_inductance;
	if (lrx == 0) {
		hwlog_info("coil_inductance is not configured, can't enter scx\n");
		return false;
	}
	for (i = 0; i < ARRAY_SIZE(g_acc_scx_lrx); i++) {
		if ((di->cap->vpa_max >= g_acc_scx_lrx[i].voltage) &&
			(path_ratio == g_acc_scx_lrx[i].path_ratio) &&
			(lrx <= g_acc_scx_lrx[i].lrx_max) &&
			(lrx >= g_acc_scx_lrx[i].lrx_min))
			return true;
	}

	return false;
}

static ssize_t wlrx_acc_dbg_kid_store(void *dev_data, const char *buf, size_t size)
{
	int val = 0;
	struct wlrx_acc_dev *di = dev_data;

	if (!di || !di->dbg)
		return -ENODEV;

	if (kstrtoint(buf, 0, &val) ||
		((val != WLRX_ACC_AUTH_FAC_KID) && (val != di->dts->kid)))
		return -EINVAL;

	di->dbg->kid = val;
	di->kid = di->dbg->kid;
	hwlog_info("[dbg_kid_store] kid=%d\n", di->dbg->kid);
	return size;
}

static ssize_t wlrx_acc_dbg_kid_show(void *dev_data, char *buf, size_t size)
{
	struct wlrx_acc_dev *di = dev_data;

	if (!di || !di->dbg)
		return -ENODEV;

	return scnprintf(buf, size, "%d\n", di->dbg->kid);
}

static int wlrx_acc_dbg_register(struct wlrx_acc_dev *di)
{
	di->dbg = kzalloc(sizeof(*di->dbg), GFP_KERNEL);
	if (!di->dbg)
		return -ENOMEM;

	di->dbg->kid = WLRX_ACC_AUTH_MIN_KID - 1; /* invalid kid */
	power_dbg_ops_register("wlrx", "kid", di,
		wlrx_acc_dbg_kid_show, wlrx_acc_dbg_kid_store);

	return 0;
}

static void wlrx_acc_parse_kid(const struct device_node *np, struct wlrx_acc_dev *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"antifake_key_index", (u32 *)&di->dts->kid, WLRX_ACC_AUTH_DFLT_KID);
	if ((di->dts->kid > WLRX_ACC_AUTH_MAX_KID) || (di->dts->kid < WLRX_ACC_AUTH_MIN_KID))
		di->dts->kid = WLRX_ACC_AUTH_DFLT_KID;

	if (!power_cmdline_is_factory_mode())
		di->kid = di->dts->kid;
	else
		di->kid = WLRX_ACC_AUTH_FAC_KID;

	hwlog_info("[parse_kid] kid=%d\n", di->kid);
}

static void wlrx_acc_parse_tx_ilim(const struct device_node *np,
	struct wlrx_acc_dts *dts)
{
	int len, i;
	int ret;
	int row;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np,
		"tx_ilim", WLACC_TX_ERROR, WLRX_ACC_TX_ILIM_END);
	if (len <= 0)
		return;

	row = len / WLRX_ACC_TX_ILIM_END;
	dts->tx_ilim = kcalloc(row, sizeof(struct wlrx_acc_tx_ilim), GFP_KERNEL);
	if (!dts->tx_ilim)
		goto kcalloc_fail;

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"tx_ilim", (u32 *)dts->tx_ilim, len);
	if (ret)
		goto parse_tx_ilim_fail;

	for (i = 0; i < row; i++)
		hwlog_info("[parse_tx_ilim] tx_ilim[%d]: %u %u %u\n",
			i, dts->tx_ilim[i].tx_type, dts->tx_ilim[i].dflt,
			dts->tx_ilim[i].ocp);
	dts->tx_ilim_len = row;
	return;

parse_tx_ilim_fail:
	kfree(dts->tx_ilim);
	dts->tx_ilim = NULL;
kcalloc_fail:
	hwlog_err("parse_tx_ilim: failed\n");
}

static void wlrx_acc_parse_eff(const struct device_node *np, struct wlrx_acc_dev *di)
{
	int len;
	int idx = 0;

	len = of_property_count_u32_elems(np, "eff_para");
	if ((len > WLRX_SCN_END) || (len <= 0))
		goto set_dflt;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"eff_para", (u32 *)di->dts->eff_para, len))
		goto set_dflt;

	idx = len;

set_dflt:
	for (; idx < WLRX_SCN_END; idx++)
		di->dts->eff_para[idx] = WLRX_ACC_TX_EFF_DEFAULT;

	for (idx = 0; idx < WLRX_SCN_END; idx++)
		hwlog_info("eff_para[%d]=%d\n", idx, di->dts->eff_para[idx]);
}

static void wlrx_fod_parse_qval_err_tx_dts(const struct device_node *np,
	struct wlrx_acc_dts *dts)
{
	int i, len;

	len = of_property_count_u8_elems(np, "qval_err_tx");
	if ((len <= 0) || (len >= WLACC_TX_ERROR))
		goto default_value;

	dts->qval_err_tx = kzalloc(len * sizeof(*(dts->qval_err_tx)), GFP_KERNEL);
	if (!dts->qval_err_tx)
		return;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"qval_err_tx", dts->qval_err_tx, len)) {
		kfree(dts->qval_err_tx);
		goto default_value;
	}

	dts->qval_err_tx_len = len;
	goto print;

default_value:
	dts->qval_err_tx = kzalloc(WLRX_ACC_QVAL_ERR_TX_NUM_DEF *
		sizeof(*(dts->qval_err_tx)), GFP_KERNEL);
	if (!dts->qval_err_tx)
		return;
	dts->qval_err_tx[0] = WLACC_TX_CP39S; /* 0 : array subscript(0~1) */
	dts->qval_err_tx[1] = WLACC_TX_CP39S_HK; /* 1 : array subscript(0~1) */
	dts->qval_err_tx_len = WLRX_ACC_QVAL_ERR_TX_NUM_DEF;
print:
	for (i = 0; i < dts->qval_err_tx_len; i++)
		hwlog_info("qval_err_tx[%d]=0x%x\n", i, dts->qval_err_tx[i]);
}

static void wlrx_acc_parse_fix_tx_fop(const struct device_node *np, struct wlrx_acc_dts *dts)
{
	(void)power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"fix_tx_fop", (int *)&dts->fix_tx_fop, 1, sizeof(dts->fix_tx_fop) / sizeof (int)); /* 1: one row */
	hwlog_info("[parse_fix_tx_fop] tx_type:%d value:%d\n", dts->fix_tx_fop.tx_type,
		dts->fix_tx_fop.value);
}

static int wlrx_acc_parse_dts(const struct device_node *np, struct wlrx_acc_dev *di)
{
	di->dts = kzalloc(sizeof(*di->dts), GFP_KERNEL);
	if (!di->dts)
		return -ENOMEM;

	wlrx_acc_parse_kid(np, di);
	wlrx_acc_parse_eff(np, di);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_high_pwr_wltx", &di->dts->support_high_pwr_wltx, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"acc_det_pwr", (u32 *)&di->dts->match_det_pmax_lth, 27000); /* default_lth:27w */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_prev_fodchk", (u32 *)&di->dts->support_prev_fodchk, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"coil_inductance", &di->dts->coil_inductance, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"prevfod_vrect_min", (u32 *)&di->dts->prevfod_vrect_min, 0);
	wlrx_fod_parse_qval_err_tx_dts(np, di->dts);
	wlrx_acc_parse_tx_ilim(np, di->dts);
	wlrx_acc_parse_fix_tx_fop(np, di->dts);
	return 0;
}

static void wlrx_acc_kfree_dev(struct wlrx_acc_dev *di)
{
	if (!di)
		return;

	kfree(di->cap);
	if (di->dts) {
		kfree(di->dts->qval_err_tx);
		kfree(di->dts->tx_ilim);
	}
	kfree(di->dts);
	kfree(di->dbg);
	kfree(di);
}

int wlrx_acc_init(unsigned int drv_type, struct device *dev)
{
	int ret;
	struct wlrx_acc_dev *di = NULL;

	if (!dev || !wltrx_is_drv_type_valid(drv_type))
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	di->cap = kzalloc(sizeof(*(di->cap)), GFP_KERNEL);
	if (!di->cap) {
		kfree(di->cap);
		return -ENOMEM;
	}

	ret = wlrx_acc_parse_dts(dev->of_node, di);
	if (ret)
		goto exit;
	ret = wlrx_acc_dbg_register(di);
	if (ret)
		goto exit;

	INIT_DELAYED_WORK(&di->prevfod_chk_work, wlrx_acc_prevfod_chk_work);

	g_rx_acc_di[drv_type] = di;
	return 0;

exit:
	wlrx_acc_kfree_dev(di);
	return ret;
}

void wlrx_acc_deinit(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type))
		return;

	wlrx_acc_kfree_dev(g_rx_acc_di[drv_type]);
	g_rx_acc_di[drv_type] = NULL;
}
