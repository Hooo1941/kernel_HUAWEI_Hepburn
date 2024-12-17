// SPDX-License-Identifier: GPL-2.0
/*
 * cps4057_dts.c
 *
 * cps4057 dts driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "cps4057.h"

#define HWLOG_TAG wireless_cps4057_dts
HWLOG_REGIST();

#define CPS4057_CM_CFG_LEN                   3
#define CPS4057_POLAR_CFG_LEN                3
#define CPS4057_DUMMY_ILOAD_LEN              4
#define CPS4057_LDO_OPP_VBST_LEN             3

static int cps4057_parse_tx_fod_ploss(struct device_node *np,
	struct cps4057_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th0", (u32 *)&di->tx_fod.ploss_th0,
		CPS4057_TX_PLOSS_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th1", (u32 *)&di->tx_fod.ploss_th1,
		CPS4057_TX_PLOSS_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th2", (u32 *)&di->tx_fod.ploss_th2,
		CPS4057_TX_PLOSS_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th3", (u32 *)&di->tx_fod.ploss_th3,
		CPS4057_TX_PLOSS_TH3_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th0", (u32 *)&di->tx_fod.hp_ploss_th0,
		CPS4057_TX_HP_PLOSS_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th1", (u32 *)&di->tx_fod.hp_ploss_th1,
		CPS4057_TX_HP_PLOSS_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th2", (u32 *)&di->tx_fod.hp_ploss_th2,
		CPS4057_TX_HP_PLOSS_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_fod_cur_th0", (u32 *)&di->tx_fod.hp_cur_th0,
		CPS4057_TX_HP_FOD_CUR_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_fod_cur_th1", (u32 *)&di->tx_fod.hp_cur_th1,
		CPS4057_TX_HP_FOD_CUR_TH1_VAL);

	return 0;
}

static int cps4057_parse_tx_fod(struct device_node *np,
	struct cps4057_dev_info *di)
{
	int ret, len;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_cond", WLTRX_FOD_CFG_ROW, WLTX_FOD_COND_END);
	if (len > 0) {
		di->tx_fod_fmt = WLTX_FOD_STATUS_FMT_NEW;
		ret = wltx_parse_fod_cfg(di->ic_type, np, CPS4057_TX_FOD_LEN);
	} else {
		di->tx_fod_fmt = WLTX_FOD_STATUS_FMT_OLD;
		ret = cps4057_parse_tx_fod_ploss(np, di);
	}
	if (ret)
		return ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_cnt", (u32 *)&di->tx_fod.ploss_cnt,
		CPS4057_TX_PLOSS_CNT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_q_en", (u32 *)&di->tx_fod.q_en,
		CPS4057_TX_FUNC_DIS);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_q_coil_th", (u32 *)&di->tx_fod.q_coil_th,
		CPS4057_TX_Q_ONLY_COIL_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_q_th", (u32 *)&di->tx_fod.q_th,
		CPS4057_TX_Q_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_fod_cur_th2", (u32 *)&di->tx_fod.hp_cur_th2,
		CPS4057_TX_HP_FOD_CUR_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th3", (u32 *)&di->tx_fod.hp_ploss_th3,
		CPS4057_TX_HP_PLOSS_TH3_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_min_duty", &di->tx_min_duty,
		CPS4057_TX_PT_DUTY_MIN);

	return 0;
}

static int cps4057_parse_ldo_cfg(struct device_node *np,
	struct cps4057_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg_5v, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg_9v, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg_sc, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

static void cps4057_parse_dummy_iload(const struct device_node *np,
	struct cps4057_dev_info *di)
{
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"dummy_load", (u32 *)&di->dummy_load, CPS4057_DUMMY_ILOAD_LEN)) {
		di->dummy_load.sc4_outside_ask = CPS4057_RX_HV_DUMMY_ILOAD_DEFAULT;
		di->dummy_load.sc4_within_ask = CPS4057_RX_HV_DUMMY_ILOAD1_DEFAULT;
		di->dummy_load.sc2_outside_ask = CPS4057_RX_LV_DUMMY_ILOAD_DEFAULT;
		di->dummy_load.sc2_within_ask = CPS4057_RX_LV_DUMMY_ILOAD1_DEFAULT;
	}
	hwlog_info("[parse_dummy_iload] sc4_outside_ask=%u sc4_within_ask=%u\t"
		"sc2_outside_ask=%u sc2_within_ask=%u\n", di->dummy_load.sc4_outside_ask,
		di->dummy_load.sc4_within_ask, di->dummy_load.sc2_outside_ask,
		di->dummy_load.sc2_within_ask);
}

static void cps4057_parse_ldo_opp_vbst(struct device_node *np,
	struct cps4057_dev_info *di)
{
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"ldo_opp_vbst", (u32 *)&di->ldo_opp_vbst, CPS4057_LDO_OPP_VBST_LEN)) {
		di->ldo_opp_vbst.vbst = CPS4057_RX_LDO_OPP_VBST_DFLT;
		di->ldo_opp_vbst.iout_lth = CPS4057_RX_LDO_OPP_ITHL_DFLT;
		di->ldo_opp_vbst.iout_hth = CPS4057_RX_LDO_OPP_ITHH_DFLT;
	}

	hwlog_info("[parse_ldo_opp_vbst] vbst=%u iout_lth=%u iout_hth=%u\n",
		di->ldo_opp_vbst.vbst, di->ldo_opp_vbst.iout_lth, di->ldo_opp_vbst.iout_hth);
}

static void cps4057_parse_rx_ask_mod_cfg(struct device_node *np,
	struct cps4057_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_mod_cm_cfg", (u8 *)&di->mod_cfg.cm, CPS4057_CM_CFG_LEN)) {
		di->mod_cfg.cm.l_volt = CPS4057_RX_CMALL_EN_VAL;
		di->mod_cfg.cm.h_volt = CPS4057_RX_CMC_EN_VAL;
		di->mod_cfg.cm.fac_h_volt = CPS4057_RX_CMBC_EN_VAL;
	}
	hwlog_info("[parse_rx_ask_mod_cfg] [cm] l_volt=0x%x h_volt=0x%x fac_h_volt=0x%x\n",
		di->mod_cfg.cm.l_volt, di->mod_cfg.cm.h_volt, di->mod_cfg.cm.fac_h_volt);
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_mod_polar_cfg", (u8 *)&di->mod_cfg.polar, CPS4057_POLAR_CFG_LEN)) {
		di->mod_cfg.polar.l_volt = CPS4057_RX_CM_POSITIVE;
		di->mod_cfg.polar.h_volt = CPS4057_RX_CM_NEGTIVE;
		di->mod_cfg.polar.fac_h_volt = CPS4057_RX_CM_NEGTIVE;
	}
	hwlog_info("[parse_rx_ask_mod_cfg] [polar] l_volt=0x%x h_volt=0x%x fac_h_volt=0x%x\n",
		di->mod_cfg.polar.l_volt, di->mod_cfg.polar.h_volt, di->mod_cfg.polar.fac_h_volt);
}

int cps4057_parse_dts(struct device_node *np, struct cps4057_dev_info *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth,
		CPS4057_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val,
		WLTRX_IC_EN_ENABLE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"full_bridge_ith", (u32 *)&di->full_bridge_ith,
		CPS4057_FULL_BRIDGE_ITH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"mtp_check_delay", &di->mtp_check_delay.user,
		WIRELESS_FW_WORK_DELAYED_TIME);
	di->mtp_check_delay.fac = WIRELESS_FW_WORK_DELAYED_TIME;
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_freq", &di->tx_ping_freq,
		CPS4057_TX_PING_FREQ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"coil_test_ping_freq", (u32 *)&di->coil_test_ping_freq,
		CPS4057_COIL_TEST_PING_FREQ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_bridge_ctrl", (u32 *)&di->tx_ping_bridge_ctrl,
		CPS4057_PING_BRIDGE_CTRL_DIS);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"min_fop", &di->min_fop, CPS4057_TX_MIN_FOP_DFLT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fodchk_dummy_iload", (u32 *)&di->fodchk_dummy_iload, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_ocp", &di->tx_ping_ocp, CPS4057_TX_PING_OCP_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fodchk_ask_cap", &di->fodchk_ask_cap, CPS4057_RX_CMA_EN_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_timer_cc_ctrl_val", &di->timer_cc_ctrl_val,
		CPS4057_TX_TIMER_CC_CTRL_DFLT);

	ret = cps4057_parse_tx_fod(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse tx_fod para failed\n");
		return ret;
	}

	ret = wlrx_parse_fod_cfg(di->ic_type, np, CPS4057_RX_FOD_LEN);
	if (ret) {
		hwlog_err("parse_dts: parse rx_fod para failed\n");
		return ret;
	}
	ret = cps4057_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse ldo cfg failed\n");
		return ret;
	}

	cps4057_parse_ldo_opp_vbst(np, di);
	cps4057_parse_dummy_iload(np, di);
	cps4057_parse_rx_ask_mod_cfg(np, di);

	return 0;
}
