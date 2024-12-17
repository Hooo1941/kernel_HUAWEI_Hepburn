// SPDX-License-Identifier: GPL-2.0
/*
 * mt5806_dts.c
 *
 * mt5806 dts driver
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

#include "mt5806.h"

#define HWLOG_TAG wireless_mt5806_dts
HWLOG_REGIST();

static void mt5806_parse_tx_fod(struct device_node *np, struct mt5806_dev_info *di)
{
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_rpp_th0", &di->tx_fod.ploss_rpp_th0, MT5806_TX_PLOSS_RP_TH0_VAL);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th0", &di->tx_fod.ploss_th0, MT5806_TX_PLOSS_TH0_VAL);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th1", &di->tx_fod.ploss_th1, MT5806_TX_PLOSS_TH1_VAL);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th2", &di->tx_fod.ploss_th2, MT5806_TX_PLOSS_TH2_VAL);
	(void)power_dts_read_u8(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_cnt", &di->tx_fod.ploss_cnt, MT5806_TX_PLOSS_CNT_VAL);
}

int mt5806_parse_dts(struct device_node *np, struct mt5806_dev_info *di)
{
	if (!np || !di)
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"default_psy_type", &di->default_psy_type, MT5806_DEFAULT_LOWPOWER);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_ocp_th", (u32 *)&di->tx_pocp_th, MT5806_TX_PING_OCP_TH);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_ocp_th", &di->tx_ocp_th, MT5806_TX_OCP_TH);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_max_fop", &di->tx_fop.tx_max_fop, MT5806_TX_MAX_FOP);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_min_fop", &di->tx_fop.tx_min_fop, MT5806_TX_MIN_FOP);
	(void)power_dts_read_u16(power_dts_tag(HWLOG_TAG), np,
		"tx_cep_timeout", &di->tx_cep_timeout, MT5806_TX_CEP_TIMEOUT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"q_cali_dynamic_en", &di->q_cali_dynamic_en, 0);

	mt5806_parse_tx_fod(np, di);

	return 0;
}
