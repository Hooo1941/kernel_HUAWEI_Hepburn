// SPDX-License-Identifier: GPL-2.0
/*
 * cps8601_dts.c
 *
 * cps8601 dts driver
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

#include "cps8601.h"

#define HWLOG_TAG wireless_cps8601_dts
HWLOG_REGIST();

static void cps8601_parse_tx_fod(struct device_node *np, struct cps8601_dev_info *di)
{
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_rp_th0", &di->tx_fod.ploss_rp_th0, CPS8601_TX_PLOSS_RP_TH0_VAL);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th0", &di->tx_fod.ploss_th0, CPS8601_TX_PLOSS_TH0_VAL);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th1", &di->tx_fod.ploss_th1, CPS8601_TX_PLOSS_TH1_VAL);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th2", &di->tx_fod.ploss_th2, CPS8601_TX_PLOSS_TH2_VAL);
	(void)power_dts_read_u8(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_cnt", &di->tx_fod.ploss_cnt, CPS8601_TX_PLOSS_CNT_VAL);
}

int cps8601_parse_dts(struct device_node *np, struct cps8601_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"default_psy_type", &di->default_psy_type, CPS8601_DEFAULT_LOWPOWER);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_max_fop", &di->tx_fop.tx_max_fop, CPS8601_TX_MAX_FOP);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_min_fop", &di->tx_fop.tx_min_fop, CPS8601_TX_MIN_FOP);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ocp_th", &di->tx_ocp_th, CPS8601_TX_OCP_TH);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_ocp_th", &di->tx_pocp_th, CPS8601_TX_PING_OCP_TH);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_cep_timeout", &di->tx_cep_timeout, CPS8601_TX_CEP_TIMEOUT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"q_cali_dynamic_en", &di->q_cali_dynamic_en, 0);

	cps8601_parse_tx_fod(np, di);

	return 0;
}
