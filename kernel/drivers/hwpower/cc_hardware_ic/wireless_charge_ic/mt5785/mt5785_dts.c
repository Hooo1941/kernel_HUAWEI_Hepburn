// SPDX-License-Identifier: GPL-2.0
/*
 * mt5785_dts.c
 *
 * mt5785 dts driver
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

#include "mt5785.h"

#define HWLOG_TAG wireless_mt5785_dts
HWLOG_REGIST();

#define MT5785_CM_CFG_LEN                   3
#define MT5785_POLAR_CFG_LEN                3
#define MT5785_LDO_OPP_VBST_LEN             3

/* for cm_intfr_para */
#define WLRX_CM_INTFR_CFG_ROW               3
#define WLRX_CM_INTFR_CFG_COL               3

static int mt5785_parse_tx_fod_ploss(struct device_node *np, struct mt5785_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_th0",
		&di->tx_fod.ploss_th0, MT5785_TX_PLOSS_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_th1",
		&di->tx_fod.ploss_th1, MT5785_TX_PLOSS_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_th2",
		&di->tx_fod.ploss_th2, MT5785_TX_PLOSS_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_th3",
		&di->tx_fod.ploss_th3, MT5785_TX_PLOSS_TH3_VAL);

	return 0;
}

static int mt5785_parse_tx_fod(struct device_node *np, struct mt5785_dev_info *di)
{
	int ret, len;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_cond", WLTRX_FOD_CFG_ROW, WLTX_FOD_COND_END);
	if (len > 0) {
		di->tx_fod_fmt = WLTX_FOD_STATUS_FMT_NEW;
		ret = wltx_parse_fod_cfg(di->ic_type, np, MT5785_TX_FOD_LEN);
	} else {
		di->tx_fod_fmt = WLTX_FOD_STATUS_FMT_OLD;
		ret = mt5785_parse_tx_fod_ploss(np, di);
	}
	if (ret)
		return ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_cnt",
		&di->tx_fod.ploss_cnt, MT5785_TX_PLOSS_CNT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_q_en",
		&di->tx_fod.q_en, MT5785_TX_FUNC_DIS);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_q_coil_th",
		&di->tx_fod.q_coil_th, MT5785_TX_Q_ONLY_COIL_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_q_th",
		&di->tx_fod.q_th, MT5785_TX_Q_TH);

	return 0;
}

static int mt5785_parse_ldo_cfg(struct device_node *np, struct mt5785_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg.l_volt, MT5785_RX_LDO_CFG_LEN))
		return -EINVAL;
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg.m_volt, MT5785_RX_LDO_CFG_LEN))
		return -EINVAL;
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg.sc, MT5785_RX_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

static void mt5785_parse_ldo_opp_vbst(struct device_node *np,
	struct mt5785_dev_info *di)
{
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"ldo_opp_vbst", (u32 *)&di->ldo_opp_vbst, MT5785_LDO_OPP_VBST_LEN)) {
		di->ldo_opp_vbst.vbst = MT5785_RX_LDO_OPP_VBST_DFLT;
		di->ldo_opp_vbst.iout_lth = MT5785_RX_LDO_OPP_ITHL_DFLT;
		di->ldo_opp_vbst.iout_hth = MT5785_RX_LDO_OPP_ITHH_DFLT;
	}

	hwlog_info("[parse_ldo_opp_vbst] vbst=%u iout_lth=%u iout_hth=%u\n",
		di->ldo_opp_vbst.vbst, di->ldo_opp_vbst.iout_lth, di->ldo_opp_vbst.iout_hth);
}

static void mt5785_parse_cm_intfr_cfg(struct device_node *np, struct mt5785_dev_info *di)
{
	int i, len, size;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np,
		"rx_cm_intfr_cfg", WLRX_CM_INTFR_CFG_ROW, WLRX_CM_INTFR_CFG_COL);
	if (len <= 0)
		return;

	size = sizeof(*di->cm_intfr_cfg) * (len / WLRX_CM_INTFR_CFG_COL);
	di->cm_intfr_cfg = kzalloc(size, GFP_KERNEL);
	if (!di->cm_intfr_cfg)
		return;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_cm_intfr_cfg", (u8 *)di->cm_intfr_cfg, len))
		return;

	di->cm_intfr_cfg_level = len / WLRX_CM_INTFR_CFG_COL;
	for (i = 0; i < di->cm_intfr_cfg_level; i++)
		hwlog_info("rx_cm_intfr[%d], type: %u cm: %u polar: %u\n",
			i, di->cm_intfr_cfg[i].type, di->cm_intfr_cfg[i].cm, di->cm_intfr_cfg[i].polar);
}

static void mt5785_parse_rx_ask_mod_cfg(struct device_node *np,
	struct mt5785_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_mod_cm_cfg", (u8 *)&di->rx_mod_cfg.cm, MT5785_CM_CFG_LEN)) {
		di->rx_mod_cfg.cm.l_volt = MT5785_RX_CMALL_EN_VAL;
		di->rx_mod_cfg.cm.h_volt = MT5785_RX_CMALL_EN_VAL;
		di->rx_mod_cfg.cm.fac_h_volt = MT5785_RX_CMALL_EN_VAL;
	}
	hwlog_info("[parse_rx_ask_mod_cfg] [cm] l_volt=0x%x h_volt=0x%x fac_h_volt=0x%x\n",
		di->rx_mod_cfg.cm.l_volt, di->rx_mod_cfg.cm.h_volt,
		di->rx_mod_cfg.cm.fac_h_volt);
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_mod_polar_cfg", (u8 *)&di->rx_mod_cfg.polar, MT5785_POLAR_CFG_LEN)) {
		di->rx_mod_cfg.polar.l_volt = MT5785_RX_CM_POSITIVE;
		di->rx_mod_cfg.polar.h_volt = MT5785_RX_CM_POSITIVE;
		di->rx_mod_cfg.polar.fac_h_volt = MT5785_RX_CM_POSITIVE;
	}
	hwlog_info("[parse_rx_ask_mod_cfg] [polar] l_volt=0x%x h_volt=0x%x fac_h_volt=0x%x\n",
		di->rx_mod_cfg.polar.l_volt, di->rx_mod_cfg.polar.h_volt,
		di->rx_mod_cfg.polar.fac_h_volt);
}

static void mt5785_parse_dummy_iload(const struct device_node *np,
	struct mt5785_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dummy_iload", &di->dummy_iload, MT5785_RX_DUMMYOLAD_TH);

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"dummy_iload_new", (u32 *)&di->dummy_iload_new, MT5785_DUMMY_ILOAD_END)) {
		di->dummy_iload_new[MT5785_DUMMY_ILOAD_5V_TH] = di->dummy_iload;
		di->dummy_iload_new[MT5785_DUMMY_ILOAD_9V_TH] = di->dummy_iload;
		di->dummy_iload_new[MT5785_DUMMY_ILOAD_SC_TH] = di->dummy_iload;
	}

	hwlog_info("[parse_dummy_iload] dummy_iload_old=%u\n", di->dummy_iload);
	hwlog_info("[parse_dummy_iload] dummy_iload_new=<%u %u %u %u>\n",
		di->dummy_iload_new[MT5785_DUMMY_ILOAD_5V_TH],
		di->dummy_iload_new[MT5785_DUMMY_ILOAD_9V_TH],
		di->dummy_iload_new[MT5785_DUMMY_ILOAD_SC_TH],
		di->dummy_iload_new[MT5785_DUMMY_ILOAD_FODCHK_TH]);
}

static int mt5785_parse_basic_para(struct device_node *np, struct mt5785_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otp_th", (u32 *)&di->otp_th, MT5785_RX_OTP_TH_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"duty_min", (u32 *)&di->duty_min, MT5785_TX_PT_DUTY_MIN_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"duty_max", (u32 *)&di->duty_max, MT5785_TX_PT_DUTY_MAX_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ping_ocp_th", (u32 *)&di->ping_ocp_th,
		MT5785_TX_PING_OCP_TH_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth, MT5785_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val,
		WLTRX_IC_EN_ENABLE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"mtp_check_delay_user", &di->mtp_check_delay.user,
		WIRELESS_FW_WORK_DELAYED_TIME);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"mtp_check_delay_fac", &di->mtp_check_delay.fac,
		WIRELESS_FW_WORK_DELAYED_TIME);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_freq", &di->tx_ping_freq, MT5785_TX_PING_FREQ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_duty", &di->tx_ping_duty, MT5785_TX_PING_DUTY);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"coil_test_ping_freq", &di->coil_test_ping_freq,
		MT5785_TX_PING_FREQ_COIL_TEST);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_pt_ocp_th", &di->tx_pt_ocp_th, MT5785_TX_OCP1_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_vfc_diff", &di->rx_vfc_diff, MT5785_RX_VFC_DIFF_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_offset", &di->rx_fod_offset, MT5785_RX_FOD_OFFSET_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fodchk_ask_cap", &di->fodchk_ask_cap, MT5785_RX_CAP0_POSITIVE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ilim_proc_cycle", &di->tx_ilim_proc_cycle, MT5785_TX_ILIM_PROC_CYCLE_DFLT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"prevfod_gain_unit", &di->prevfod_gain_unit, MT5785_RX_RDSON_OF_COIL_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_opp_en", &di->rx_ldo_opp_en, MT5785_LDO_OPP_EN_DEFAULT);

	return 0;
}

int mt5785_parse_dts(struct device_node *np, struct mt5785_dev_info *di)
{
	int ret;

	ret = mt5785_parse_basic_para(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse basic para failed\n");
		return ret;
	}

	ret = mt5785_parse_tx_fod(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse tx_fod para failed\n");
		return ret;
	}

	ret = wlrx_parse_fod_cfg(di->ic_type, np, MT5785_RX_FOD_LEN);
	if (ret) {
		hwlog_err("parse_dts: parse rx_fod para failed\n");
		return ret;
	}

	ret = mt5785_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse ldo cfg failed\n");
		return ret;
	}

	mt5785_parse_ldo_opp_vbst(np, di);
	mt5785_parse_cm_intfr_cfg(np, di);
	mt5785_parse_rx_ask_mod_cfg(np, di);
	mt5785_parse_dummy_iload(np, di);
	return 0;
}
