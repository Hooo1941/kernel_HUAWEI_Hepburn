// SPDX-License-Identifier: GPL-2.0
/*
 * cps4067_dts.c
 *
 * cps4067 dts driver
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

#include "cps4067.h"

#define HWLOG_TAG wireless_cps4067_dts
HWLOG_REGIST();

#define CPS4067_CM_CFG_LEN                   3
#define CPS4067_POLAR_CFG_LEN                3
#define CPS4067_LDO_OPP_VBST_LEN             3

/* for cm_intfr_para */
#define WLRX_CM_INTFR_CFG_ROW                3
#define WLRX_CM_INTFR_CFG_COL                3

static int cps4067_parse_tx_fod_ploss(struct device_node *np, struct cps4067_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_th0",
		&di->tx_fod.ploss_th0, CPS4067_TX_PLOSS_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_th1",
		&di->tx_fod.ploss_th1, CPS4067_TX_PLOSS_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_th2",
		&di->tx_fod.ploss_th2, CPS4067_TX_PLOSS_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_th3",
		&di->tx_fod.ploss_th3, CPS4067_TX_PLOSS_TH3_VAL);

	return 0;
}

static int cps4067_parse_tx_fod(struct device_node *np, struct cps4067_dev_info *di)
{
	int ret, len;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_cond", WLTRX_FOD_CFG_ROW, WLTX_FOD_COND_END);
	if (len > 0) {
		di->tx_fod_fmt = WLTX_FOD_STATUS_FMT_NEW;
		ret = wltx_parse_fod_cfg(di->ic_type, np, CPS4067_TX_FOD_LEN);
	} else {
		di->tx_fod_fmt = WLTX_FOD_STATUS_FMT_OLD;
		ret = cps4067_parse_tx_fod_ploss(np, di);
	}
	if (ret)
		return ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_ploss_cnt",
		&di->tx_fod.ploss_cnt, CPS4067_TX_PLOSS_CNT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_q_en",
		&di->tx_fod.q_en, CPS4067_TX_FUNC_DIS);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_q_coil_th",
		&di->tx_fod.q_coil_th, CPS4067_TX_Q_ONLY_COIL_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "tx_q_th",
		&di->tx_fod.q_th, CPS4067_TX_Q_TH);

	return 0;
}

static int cps4067_parse_ldo_cfg(struct device_node *np, struct cps4067_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg.l_volt, CPS4067_RX_LDO_CFG_LEN))
		return -EINVAL;
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg.m_volt, CPS4067_RX_LDO_CFG_LEN))
		return -EINVAL;
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg.sc, CPS4067_RX_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

static void cps4067_parse_cm_intfr_cfg(struct device_node *np, struct cps4067_dev_info *di)
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

static void cps4067_parse_rx_ask_mod_cfg(struct device_node *np,
	struct cps4067_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_mod_cm_cfg", (u8 *)&di->rx_mod_cfg.cm, CPS4067_CM_CFG_LEN)) {
		di->rx_mod_cfg.cm.l_volt = CPS4067_RX_CMALL_EN_VAL;
		di->rx_mod_cfg.cm.h_volt = CPS4067_RX_CMC_EN_VAL;
		di->rx_mod_cfg.cm.fac_h_volt = CPS4067_RX_CMBC_EN_VAL;
	}
	hwlog_info("[parse_rx_ask_mod_cfg] [cm] l_volt=0x%x h_volt=0x%x fac_h_volt=0x%x\n",
		di->rx_mod_cfg.cm.l_volt, di->rx_mod_cfg.cm.h_volt,
		di->rx_mod_cfg.cm.fac_h_volt);
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_mod_polar_cfg", (u8 *)&di->rx_mod_cfg.polar, CPS4067_POLAR_CFG_LEN)) {
		di->rx_mod_cfg.polar.l_volt = CPS4067_RX_CM_POSITIVE;
		di->rx_mod_cfg.polar.h_volt = CPS4067_RX_CM_NEGTIVE;
		di->rx_mod_cfg.polar.fac_h_volt = CPS4067_RX_CM_NEGTIVE;
	}
	hwlog_info("[parse_rx_ask_mod_cfg] [polar] l_volt=0x%x h_volt=0x%x fac_h_volt=0x%x\n",
		di->rx_mod_cfg.polar.l_volt, di->rx_mod_cfg.polar.h_volt,
		di->rx_mod_cfg.polar.fac_h_volt);
}

static void cps4067_parse_ldo_opp_vbst(struct device_node *np,
	struct cps4067_dev_info *di)
{
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"ldo_opp_vbst", (u32 *)&di->ldo_opp_vbst, CPS4067_LDO_OPP_VBST_LEN)) {
		di->ldo_opp_vbst.vbst = CPS4067_RX_LDO_OPP_VBST_DFLT;
		di->ldo_opp_vbst.iout_lth = CPS4067_RX_LDO_OPP_ITHL_DFLT;
		di->ldo_opp_vbst.iout_hth = CPS4067_RX_LDO_OPP_ITHH_DFLT;
	}

	hwlog_info("[parse_ldo_opp_vbst] vbst=%u iout_lth=%u iout_hth=%u\n",
		di->ldo_opp_vbst.vbst, di->ldo_opp_vbst.iout_lth, di->ldo_opp_vbst.iout_hth);
}

static void cps4067_parse_dummy_iload(const struct device_node *np,
	struct cps4067_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dummy_iload", &di->dummy_iload,
		CPS4067_RX_DUMMY_LOAD_NO_MOD_VAL);

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"dummy_iload_new", (u32 *)&di->dummy_iload_new, CPS4067_DUMMY_ILOAD_END)) {
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_5V_NO_MOD] = CPS4067_RX_DUMMY_LOAD_NO_MOD_VAL;
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_5V_MOD] = CPS4067_RX_DUMMY_LOAD_MOD_VAL;
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_9V_NO_MOD] = CPS4067_RX_DUMMY_LOAD_NO_MOD_VAL;
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_9V_MOD] = CPS4067_RX_DUMMY_LOAD_MOD_VAL;
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_SC_NO_MOD] = di->dummy_iload;
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_SC_MOD] = CPS4067_RX_DUMMY_LOAD_MOD_VAL;
	}

	hwlog_info("[parse_dummy_iload] dummy_iload_old=%u\n", di->dummy_iload);
	hwlog_info("[parse_dummy_iload] dummy_iload_new=<%u %u %u %u %u %u %u %u>\n",
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_5V_NO_MOD],
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_5V_MOD],
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_9V_NO_MOD],
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_9V_MOD],
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_SC_NO_MOD],
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_SC_MOD],
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_FODCHK_NO_MOD],
		di->dummy_iload_new[CPS4067_DUMMY_ILOAD_FODCHK_MOD]);
}

static int cps4067_parse_basic_para(struct device_node *np, struct cps4067_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"duty_min", (u32 *)&di->duty_min, CPS4067_TX_PT_DUTY_MIN_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"duty_max", (u32 *)&di->duty_max, CPS4067_TX_PT_DUTY_MAX_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ping_ocp_th", (u32 *)&di->ping_ocp_th,
		CPS4067_TX_PING_OCP_TH_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth, CPS4067_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val,
		WLTRX_IC_EN_ENABLE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"full_bridge_ith", (u32 *)&di->tx_full_bri_ith,
		CPS4067_TX_FULL_BRI_ITH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"mtp_check_delay_user", &di->mtp_check_delay.user,
		WIRELESS_FW_WORK_DELAYED_TIME);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"mtp_check_delay_fac", &di->mtp_check_delay.fac,
		WIRELESS_FW_WORK_DELAYED_TIME);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_freq", &di->tx_ping_freq, CPS4067_TX_PING_FREQ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_duty", &di->tx_ping_duty, CPS4067_TX_PING_DUTY);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"coil_test_ping_freq", &di->coil_test_ping_freq,
		CPS4067_COIL_TEST_PING_FREQ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_vfc_diff", &di->rx_vfc_diff,
		CPS4067_RX_FC_DELTA_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_offset", &di->rx_fod_offset,
		CPS4067_RX_FOD_OFFSET_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_pt_ocp_th", &di->tx_pt_ocp_th, CPS4067_TX_OCP_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fodchk_ask_cap", &di->fodchk_ask_cap, CPS4067_RX_CMA_EN_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_wdt_timeout", &di->rx_wdt_timeout, CPS4067_RX_WDT_TO);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_timer_cc_ctrl_val", &di->timer_cc_ctrl_val,
		CPS4067_TX_TIMER_CC_CTRL_DFLT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"prevfod_gain_unit", &di->prevfod_gain_unit,
		CPS4067_RX_GAIN_UNIT_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_opp_en", &di->rx_ldo_opp_en, 0);
	return 0;
}

int cps4067_parse_dts(struct device_node *np, struct cps4067_dev_info *di)
{
	int ret;

	ret = cps4067_parse_basic_para(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse basic para failed\n");
		return ret;
	}

	ret = cps4067_parse_tx_fod(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse tx_fod para failed\n");
		return ret;
	}

	ret = wlrx_parse_fod_cfg(di->ic_type, np, CPS4067_RX_FOD_LEN);
	if (ret) {
		hwlog_err("parse_dts: parse rx_fod para failed\n");
		return ret;
	}

	ret = cps4067_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse ldo cfg failed\n");
		return ret;
	}

	cps4067_parse_ldo_opp_vbst(np, di);
	cps4067_parse_cm_intfr_cfg(np, di);
	cps4067_parse_rx_ask_mod_cfg(np, di);
	cps4067_parse_dummy_iload(np, di);
	return 0;
}
