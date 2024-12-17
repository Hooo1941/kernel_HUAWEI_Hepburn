// SPDX-License-Identifier: GPL-2.0
/*
 * mt5806_tx.c
 *
 * mt5806 tx driver
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

#define HWLOG_TAG wireless_mt5806_tx
HWLOG_REGIST();

static const char * const g_mt5806_tx_irq_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_ss_pkt_rcvd",
	[1]  = "tx_id_pkt_rcvd",
	[2]  = "tx_cfg_pkt_rcvd",
	[8]  = "tx_pwr_trans",
	[11] = "tx_pp_pkt_rcvd",
	[14] = "tx_enable",
	[15] = "tx_ept_pkt_rcvd",
	[17] = "tx_start_ping",
	[25] = "tx_sta_attach",
	[26] = "tx_sta_remove",
	[27] = "tx_q_calibration",
};

static const char * const g_mt5806_tx_ept_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_ept_src_cmd",
	[1]  = "tx_ept_src_ss",
	[2]  = "tx_ept_src_id",
	[3]  = "tx_ept_src_xid",
	[4]  = "tx_ept_src_cfg_cnt",
	[5]  = "tx_ept_src_pch",
	[7]  = "tx_ept_timeout",
	[8]  = "tx_ept_src_cep_timeout",
	[9]  = "tx_ept_src_rpp_timeout",
	[10] = "tx_ept_src_ocp",
	[11] = "tx_ept_src_ovp",
	[12] = "tx_ept_src_lvp",
	[13] = "tx_ept_src_fod",
	[14] = "tx_ept_src_otp",
	[16] = "tx_ept_src_cfg",
	[17] = "tx_ept_src_ping_ovp",
	[18] = "tx_ept_src_ping_ocp",
	[19] = "tx_ept_src_pkterr",
};

unsigned int mt5806_tx_get_bnt_wltx_type(int ic_type)
{
	return ic_type == WLTRX_IC_AUX ? POWER_BNT_WLTX_AUX : POWER_BNT_WLTX;
}

static bool mt5806_tx_is_tx_mode(void *dev_data)
{
	int ret;
	u32 mode = 0;
	struct mt5806_dev_info *di = dev_data;

	ret = mt5806_read_block(di, MT5806_OP_MODE_ADDR, (u8 *)&mode, MT5806_OP_MODE_LEN);
	if (ret) {
		hwlog_err("is_tx_mode: get op_mode failed, ret:%d\n", ret);
		return false;
	}

	return (mode & MT5806_OP_MODE_TX);
}

static bool mt5806_tx_is_rx_mode(void *dev_data)
{
	int ret;
	u32 mode = 0;
	struct mt5806_dev_info *di = dev_data;

	ret = mt5806_read_block(di, MT5806_OP_MODE_ADDR, (u8 *)&mode, MT5806_OP_MODE_LEN);
	if (ret) {
		hwlog_err("is_rx_mode: get rx mode failed, ret:%d\n", ret);
		return false;
	}

	return (mode & MT5806_OP_MODE_RX);
}

static int mt5806_tx_set_tx_open_flag(bool enable, void *dev_data)
{
	return 0;
}

static int mt5806_tx_get_full_bridge_ith(u16 *ith, void *dev_data)
{
	return 0;
}

static int mt5806_tx_set_vset(int tx_vset, void *dev_data)
{
	return 0;
}

static int mt5806_tx_set_bridge(unsigned int v_ask, unsigned int type, void *dev_data)
{
	return 0;
}

static bool mt5806_tx_check_rx_disconnect(void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if (!di)
		return true;

	if (di->ept_type & MT5806_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~MT5806_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[check_rx_disconnect] rx disconnect\n");
		return true;
	}

	return false;
}

static int mt5806_tx_get_ping_interval(u16 *ping_interval, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct mt5806_dev_info *di = dev_data;

	if (!ping_interval) {
		hwlog_err("get_ping_interval: para null\n");
		return -EINVAL;
	}

	ret = mt5806_read_word(di, MT5806_TX_PING_INTERVAL_ADDR, &data);
	if (ret) {
		hwlog_err("get_ping_interval: read failed\n");
		return ret;
	}
	*ping_interval = data / MT5806_TX_PING_INTERVAL_STEP;

	return 0;
}

static int mt5806_tx_set_ping_interval(u16 ping_interval, void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if ((ping_interval < MT5806_TX_PING_INTERVAL_MIN) ||
		(ping_interval > MT5806_TX_PING_INTERVAL_MAX)) {
		hwlog_err("set_ping_interval: para out of range\n");
		return -EINVAL;
	}

	return mt5806_write_word(di, MT5806_TX_PING_INTERVAL_ADDR,
		ping_interval * MT5806_TX_PING_INTERVAL_STEP);
}

static int mt5806_tx_get_ping_freq(u16 *ping_freq, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct mt5806_dev_info *di = dev_data;

	if (!ping_freq) {
		hwlog_err("get_ping_frequency: para null\n");
		return -EINVAL;
	}

	ret = mt5806_read_word(di, MT5806_TX_PING_FREQ_ADDR, &data);
	if (ret) {
		hwlog_err("get_ping_frequency: read failed\n");
		return ret;
	}
	if (data != 0)
		*ping_freq = OSCCLK / data + MT5806_SHIFT_VAL;

	return 0;
}

static int mt5806_tx_set_ping_freq(u16 ping_freq, void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if ((ping_freq < MT5806_TX_PING_FREQ_MIN) || (ping_freq > MT5806_TX_PING_FREQ_MAX)) {
		hwlog_err("set_ping_frequency: para out of range\n");
		return -EINVAL;
	}

	return mt5806_write_word(di, MT5806_TX_PING_FREQ_ADDR,
		(OSCCLK / ping_freq - MT5806_SHIFT_VAL) * MT5806_TX_PING_STEP);
}

static int mt5806_tx_get_min_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct mt5806_dev_info *di = dev_data;

	if (!fop) {
		hwlog_err("get_min_fop: para null\n");
		return -EINVAL;
	}

	ret = mt5806_read_word(di, MT5806_TX_MIN_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("get_min_fop: read failed, ret:%d\n", ret);
		return ret;
	}
	if (data != 0)
		*fop = OSCCLK / data + MT5806_SHIFT_VAL;

	return 0;
}

static int mt5806_tx_set_min_fop(u16 fop, void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if ((fop < MT5806_TX_MIN_FOP) || (fop > MT5806_TX_MAX_FOP)) {
		hwlog_err("set_min_fop: para out of range\n");
		return -EINVAL;
	}

	return mt5806_write_word(di, MT5806_TX_MIN_FOP_ADDR,
		(OSCCLK / fop - MT5806_SHIFT_VAL) * MT5806_TX_FOP_STEP);
}

static int mt5806_tx_get_max_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct mt5806_dev_info *di = dev_data;

	if (!fop) {
		hwlog_err("get_max_fop: para null\n");
		return -EINVAL;
	}

	ret = mt5806_read_word(di, MT5806_TX_MAX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("get_max_fop: read failed\n");
		return ret;
	}
	if (data != 0)
		*fop = OSCCLK / data + MT5806_SHIFT_VAL;

	return 0;
}

static int mt5806_tx_set_max_fop(u16 fop, void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if ((fop < MT5806_TX_MIN_FOP) || (fop > MT5806_TX_MAX_FOP)) {
		hwlog_err("set_max_fop: para out of range\n");
		return -EINVAL;
	}

	return mt5806_write_word(di, MT5806_TX_MAX_FOP_ADDR,
		(OSCCLK / fop - MT5806_SHIFT_VAL) * MT5806_TX_FOP_STEP);
}

static int mt5806_tx_get_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;
	struct mt5806_dev_info *di = dev_data;

	if (!fop) {
		hwlog_err("get_fop: para null\n");
		return -EINVAL;
	}

	ret = mt5806_read_word(di, MT5806_TX_OP_FREQ_ADDR, &data);
	if (ret) {
		hwlog_err("get_fop: failed\n");
		return ret;
	}
	if (data != 0)
		*fop = OSCCLK / data + MT5806_SHIFT_VAL;

	return 0;
}

static int mt5806_tx_get_temp(s16 *chip_temp, void *dev_data)
{
	*chip_temp = 25;
	return 0;
}

static int mt5806_tx_get_cep(s8 *cep, void *dev_data)
{
	return 0;
}

static int mt5806_tx_get_duty(u8 *duty, void *dev_data)
{
	return mt5806_read_byte(dev_data, MT5806_TX_PWM_DUTY_ADDR, duty);
}

static int mt5806_tx_get_ptx(u32 *ptx, void *dev_data)
{
	return 0;
}

static int mt5806_tx_get_prx(u32 *prx, void *dev_data)
{
	return 0;
}

static int mt5806_tx_get_ploss(s32 *ploss, void *dev_data)
{
	return 0;
}

static int mt5806_tx_get_ploss_id(u8 *id, void *dev_data)
{
	return 0;
}

static int mt5806_tx_get_vrect(u16 *tx_vrect, void *dev_data)
{
	return mt5806_read_word(dev_data, MT5806_TX_VRECT_ADDR, tx_vrect);
}

static int mt5806_tx_get_vin(u16 *tx_vin, void *dev_data)
{
	return mt5806_read_word(dev_data, MT5806_TX_VIN_ADDR, tx_vin);
}

static int mt5806_tx_get_iin(u16 *tx_iin, void *dev_data)
{
	return mt5806_read_word(dev_data, MT5806_TX_IIN_ADDR, tx_iin);
}

static int mt5806_tx_set_ilimit(u16 tx_ilim, void *dev_data)
{
	return 0;
}

static int mt5806_tx_init_fod_coef(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_write_word(di, MT5806_TX_PLOSS_TH0_ADDR, di->tx_fod.ploss_th0);
	ret += mt5806_write_word(di, MT5806_TX_PLOSS_TH1_ADDR, di->tx_fod.ploss_th1);
	ret += mt5806_write_word(di, MT5806_TX_PLOSS_TH2_ADDR, di->tx_fod.ploss_th2);
	ret += mt5806_write_word(di, MT5806_TX_PLOSS_CNT_ADDR, di->tx_fod.ploss_cnt);
	ret += mt5806_write_word(di, MT5806_TX_PLOSS_RPP_TH0_ADDR, di->tx_fod.ploss_rpp_th0);
	if (ret) {
		hwlog_err("init_fod_coef: failed\n");
		return -EIO;
	}

	return 0;
}

static int mt5806_tx_stop_config(void *dev_data)
{
	return 0;
}

static int mt5806_tx_activate_chip(void *dev_data)
{
	return 0;
}

static int mt5806_tx_set_irq_en(u32 val, void *dev_data)
{
	return mt5806_write_block(dev_data, MT5806_TX_IRQ_EN_ADDR, (u8 *)&val,
		MT5806_TX_IRQ_EN_LEN);
}

static void mt5806_tx_select_init_para(struct mt5806_dev_info *di)
{
	di->tx_init_para.ping_freq = MT5806_TX_PING_FREQ;
	di->tx_init_para.ping_interval = MT5806_TX_PING_INTERVAL;
}

static int mt5806_tx_set_init_para(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_write_word(di, MT5806_TX_OVP_TH_ADDR, MT5806_TX_OVP_TH);
	ret += mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR, MT5806_TX_CMD_OVP,
		MT5806_TX_CMD_OVP_SHIFT, MT5806_TX_CMD_VAL);
	ret += mt5806_write_word(di, MT5806_TX_OCP_TH_ADDR, di->tx_ocp_th);
	ret += mt5806_write_word(di, MT5806_TX_PING_OCP_TH_ADDR, di->tx_pocp_th);
	ret += mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR, MT5806_TX_CMD_OCP,
		MT5806_TX_CMD_OCP_SHIFT, MT5806_TX_CMD_VAL);
	ret += mt5806_tx_init_fod_coef(di);
	ret += mt5806_tx_set_ping_freq(di->tx_init_para.ping_freq, di);
	ret += mt5806_tx_set_min_fop(di->tx_fop.tx_min_fop, di);
	ret += mt5806_tx_set_max_fop(di->tx_fop.tx_max_fop, di);
	ret += mt5806_tx_set_ping_interval(di->tx_init_para.ping_interval, di);
	ret += mt5806_tx_set_irq_en(MT5806_TX_IRQ_EN_VAL, di);
	ret += mt5806_write_word(di, MT5806_TX_CEP_TIMEOUT_ADDR, di->tx_cep_timeout);
	if (ret) {
		hwlog_err("set_init_para: write failed\n");
		return -EIO;
	}

	return 0;
}

static int mt5806_tx_chip_init(unsigned int client, void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if (!di)
		return -EINVAL;

	di->irq_cnt = 0;
	mt5806_enable_irq(di);

	mt5806_tx_select_init_para(di);

	return mt5806_tx_set_init_para(di);
}

static int mt5806_tx_enable_tx_mode(bool enable, void *dev_data)
{
	int ret;
	struct mt5806_dev_info *di = dev_data;

	if (enable) {
		ret = mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR,
			MT5806_TX_CMD_START_TX, MT5806_TX_CMD_START_TX_SHIFT, MT5806_TX_CMD_VAL);
		ret += mt5806_write_word(di, MT5806_TX_FSK_DEPTH_ADDR, MT5806_TX_FSK_DEPTH_OFFSET);
	} else {
		ret = mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR,
			MT5806_TX_CMD_STOP_TX, MT5806_TX_CMD_STOP_TX_SHIFT, MT5806_TX_CMD_VAL);
	}
	if (ret) {
		hwlog_err("%s tx_mode failed\n", enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

int mt5806_tx_lowpower_enable(bool enable, void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if (!di || !di->g_val.mtp_chk_complete)
		return -ENODEV;

	hwlog_info("[lowpower_enable] %s lowpower\n", enable ? "enter" : "exit");
	if (enable) {
		return mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR, MT5806_TX_CMD_LOW_POWER,
			MT5806_TX_CMD_LOW_POWER_SHIFT, MT5806_TX_CMD_VAL);
	} else {
		/* power off to force exit lowpower */
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
		power_msleep(DT_MSLEEP_2S, 0, NULL);
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
		power_msleep(DT_MSLEEP_100MS, 0, NULL);
	}

	return 0;
}

static void mt5806_tx_show_ept_type(u32 ept)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(g_mt5806_tx_ept_name); i++) {
		if (ept & BIT(i))
			hwlog_info("[tx_ept] %s\n", g_mt5806_tx_ept_name[i]);
	}
}

static int mt5806_tx_get_ept_type(struct mt5806_dev_info *di, u32 *ept)
{
	int ret;
	u32 data = 0;

	if (!ept) {
		hwlog_err("get_ept_type: para null\n");
		return -EINVAL;
	}

	ret = mt5806_read_block(di, MT5806_TX_EPT_SRC_ADDR, (u8 *)&data, MT5806_TX_EPT_SRC_LEN);
	if (ret) {
		hwlog_err("get_ept_type: read failed\n");
		return ret;
	}
	hwlog_info("[get_ept_type] type=0x%08x", data);
	mt5806_tx_show_ept_type(data);
	*ept = data;

	ret = mt5806_write_dword(di, MT5806_TX_EPT_SRC_ADDR, 0);
	if (ret) {
		hwlog_err("get_ept_type: clr failed\n");
		return ret;
	}

	return 0;
}

static void mt5806_tx_ept_handler(struct mt5806_dev_info *di)
{
	if (mt5806_tx_get_ept_type(di, &di->ept_type))
		return;
	switch (di->ept_type) {
	case MT5806_TX_EPT_SRC_CEP_TIMEOUT:
		di->ept_type &= ~MT5806_TX_EPT_SRC_CEP_TIMEOUT;
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_EPT_CMD, NULL);
		break;
	case MT5806_TX_EPT_SRC_PCH:
	case MT5806_TX_EPT_SRC_CMD:
	case MT5806_TX_EPT_SRC_PKTERR:
	case MT5806_TX_EPT_SRC_LVP:
	case MT5806_TX_EPT_SRC_OTP:
	case MT5806_TX_EPT_SRC_PING_OVP:
		di->ept_type &= ~(MT5806_TX_EPT_SRC_PCH | MT5806_TX_EPT_SRC_CMD | MT5806_TX_EPT_SRC_PKTERR |
			MT5806_TX_EPT_SRC_LVP | MT5806_TX_EPT_SRC_OTP | MT5806_TX_EPT_SRC_PING_OVP);
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_EPT_CMD, NULL);
		break;
	case MT5806_TX_EPT_SRC_PING_OCP:
		di->ept_type &= ~MT5806_TX_EPT_SRC_PING_OCP;
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			(di->ic_type == WLTRX_IC_AUX) ? POWER_NE_WLTX_TX_PING_OCP :
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	case MT5806_TX_EPT_SRC_FOD:
		di->ept_type &= ~MT5806_TX_EPT_SRC_FOD;
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	case MT5806_TX_EPT_SRC_RPP_TIMEOUT:
		di->ept_type &= ~MT5806_TX_EPT_SRC_RPP_TIMEOUT;
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_EPT_CMD, NULL);
		break;
	case MT5806_TX_EPT_SRC_OCP:
		di->ept_type &= ~MT5806_TX_EPT_SRC_OCP;
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_OCP, NULL);
		break;
	case MT5806_TX_EPT_SRC_OVP:
		di->ept_type &= ~MT5806_TX_EPT_SRC_OVP;
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_OVP, NULL);
		break;
	default:
		break;
	}
}

static int mt5806_tx_get_sample_para(struct mt5806_dev_info *di)
{
	int ret;

	ret = mt5806_read_word(di, MT5806_TX_NO_RX_CNT_ADDR, &di->tx_q_factor.tx_q_cnt);
	ret += mt5806_read_word(di, MT5806_TX_NO_RX_CNT_VAR_ADDR, &di->tx_q_factor.tx_q_cnt_var);
	ret += mt5806_read_word(di, MT5806_TX_NO_RX_WIDTH_ADDR, &di->tx_q_factor.tx_q_width);
	ret += mt5806_read_word(di, MT5806_TX_NO_RX_WIDTH_VAR_ADDR, &di->tx_q_factor.tx_q_width_var);
	ret += mt5806_read_word(di, MT5806_TX_CALI_FAC_WIDTH_ADDR, &di->tx_q_factor.tx_q_fac_width);
	ret += mt5806_read_word(di, MT5806_TX_CALI_DYN_WIDTH_ADDR, &di->tx_q_factor.tx_q_dyn_width);
	hwlog_info("[get_sample_para] q sample cnt=%u, cnt_var=%u, width=%u, width_var=%u,"
		" fac_width=%u, dyn_width=%u\n", di->tx_q_factor.tx_q_cnt, di->tx_q_factor.tx_q_cnt_var,
		di->tx_q_factor.tx_q_width, di->tx_q_factor.tx_q_width_var, di->tx_q_factor.tx_q_fac_width,
		di->tx_q_factor.tx_q_dyn_width);

	return ret;
}

static int mt5806_tx_send_bigdata(struct imonitor_eventobj *obj, void *dev_data)
{
	struct mt5806_dev_info *di = dev_data;

	if (!obj || !di) {
		hwlog_err("send_bigdata: obj or data is null\n");
		return -EPERM;
	}

	(void)power_bigdata_send_integer(obj, "FACTORYWIDTH", di->tx_q_factor.tx_q_fac_width);
	(void)power_bigdata_send_integer(obj, "DYNAMICWIDTH", di->tx_q_factor.tx_q_dyn_width);
	(void)power_bigdata_send_integer(obj, "SAMPLEWIDTH", di->tx_q_factor.tx_q_spl_width);
	(void)power_bigdata_send_integer(obj, "CALIBRATEFLAG", di->tx_q_factor.tx_q_flag);
	return 0;
}

static void mt5806_tx_q_cali_handler(struct mt5806_dev_info *di)
{
	int ret;
	s16 delta_fac;
	s16 delta_dyn;

	if (!di->g_val.tx_calibrate_flag)
		return;

	ret = mt5806_tx_get_sample_para(di);
	if (ret) {
		hwlog_err("q_cali_handler: get sample failed, stop calibrate\n");
		mt5806_tx_lowpower_enable(true, di);
		return;
	}

	di->tx_q_factor.tx_q_spl_width = di->tx_q_factor.tx_q_width;
	delta_fac = (s16)(di->tx_q_factor.tx_q_width - di->tx_q_factor.tx_q_fac_width);
	delta_dyn = (s16)(di->tx_q_factor.tx_q_width - di->tx_q_factor.tx_q_dyn_width);
	if (((di->tx_q_factor.tx_q_fac_width < MT5806_Q_FACTORY_WIDTH_LTH) ||
		(di->tx_q_factor.tx_q_fac_width > MT5806_Q_FACTORY_WIDTH_HTH))) {
		if ((di->tx_q_factor.tx_q_width >= MT5806_Q_SAMPLE_WIDTH_LTH) &&
			(di->tx_q_factor.tx_q_width <= MT5806_Q_SAMPLE_WIDTH_HTH)) {
			di->tx_q_factor.tx_q_flag = MT5806_Q_CALI_FACTORY;
			mt5806_fw_program_q_data(di);
		} else {
			mt5806_tx_lowpower_enable(true, di);
		}
	} else {
		if (di->q_cali_dynamic_en &&
			(di->tx_q_factor.tx_q_width >= MT5806_Q_SAMPLE_WIDTH_LTH) &&
			(di->tx_q_factor.tx_q_width <= MT5806_Q_SAMPLE_WIDTH_HTH) &&
			(di->tx_q_factor.tx_q_width_var <= MT5806_Q_SAMPLE_WIDTH_VAR_TH) &&
			(delta_fac >= MT5806_Q_FAC_WIDTH_DELTA_LTH) &&
			(delta_fac <= MT5806_Q_FAC_WIDTH_DELTA_HTH) &&
			(((delta_dyn >= MT5806_Q_DYN_WIDTH_DELTA_LTH0) &&
			(delta_dyn < MT5806_Q_DYN_WIDTH_DELTA_HTH0)) ||
			((delta_dyn > MT5806_Q_DYN_WIDTH_DELTA_LTH1) &&
			(delta_dyn <= MT5806_Q_DYN_WIDTH_DELTA_HTH1)))) {
			di->tx_q_factor.tx_q_flag = MT5806_Q_CALI_DYNAMIC;
			di->tx_q_factor.tx_q_width = di->tx_q_factor.tx_q_dyn_width + delta_dyn / 3;
			hwlog_info("[q_cali_handler] calc_cali_cnt=%u, calc_cali_wid=%u\n",
				di->tx_q_factor.tx_q_cnt, di->tx_q_factor.tx_q_width);
			mt5806_fw_program_q_data(di);
		} else {
			mt5806_tx_lowpower_enable(true, di);
		}
	}

	di->g_val.tx_calibrate_flag = false;
	power_bigdata_report(POWER_BIGDATA_TYPE_WLTX_AUX_CALIBRATE, mt5806_tx_send_bigdata, di);
}

static void mt5806_tx_q_detect_handler(struct mt5806_dev_info *di)
{
	if (di->default_psy_type != MT5806_DEFAULT_LOWPOWER)
		return;

	if (di->irq_val & MT5806_TX_IRQ_RX_ATTACH) {
		di->irq_val &= ~MT5806_TX_IRQ_RX_ATTACH;
		gpio_set_value(di->gpio_rx_online, true);
		hwlog_info("[q_detect_handler] rx_online=%d\n", gpio_get_value(di->gpio_rx_online));
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_HALL_APPROACH, NULL);
	}
	if (di->irq_val & MT5806_TX_IRQ_RX_REMOVED) {
		di->irq_val &= ~MT5806_TX_IRQ_RX_REMOVED;
		gpio_set_value(di->gpio_rx_online, false);
		hwlog_info("[q_detect_handler] rx_online=%d\n", gpio_get_value(di->gpio_rx_online));
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_HALL_AWAY_FROM, NULL);
	}
	if (di->irq_val & MT5806_TX_IRQ_Q_CAIL) {
		di->irq_val &= ~MT5806_TX_IRQ_Q_CAIL;
		mt5806_tx_q_cali_handler(di);
	}
}

static int mt5806_tx_clear_irq(struct mt5806_dev_info *di, u32 itr)
{
	int ret;

	ret = mt5806_write_block(di, MT5806_TX_IRQ_CLR_ADDR, (u8 *)&itr, MT5806_TX_IRQ_CLR_LEN);
	ret += mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR, MT5806_TX_CMD_CLEAR_INT,
		MT5806_TX_CMD_CLEAR_INT_SHIFT, MT5806_TX_CMD_VAL);
	if (ret) {
		hwlog_err("clear_irq: write failed\n");
		return ret;
	}

	return 0;
}

static void mt5806_tx_ask_pkt_handler(struct mt5806_dev_info *di)
{
	if (di->irq_val & MT5806_TX_IRQ_SS_PKG_RCVD) {
		di->irq_val &= ~MT5806_TX_IRQ_SS_PKG_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & MT5806_TX_IRQ_ID_PKT_RCVD) {
		di->irq_val &= ~MT5806_TX_IRQ_ID_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & MT5806_TX_IRQ_CFG_PKT_RCVD) {
		di->irq_val &= ~MT5806_TX_IRQ_CFG_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_GET_CFG, NULL);
	}

	if (di->irq_val & MT5806_TX_IRQ_PP_PKT_RCVD) {
		di->irq_val &= ~MT5806_TX_IRQ_PP_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_non_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_non_qi_ask_pkt(di);
	}
}

static void mt5806_tx_show_irq(u32 intr)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(g_mt5806_tx_irq_name); i++) {
		if (intr & BIT(i))
			hwlog_info("[tx_irq] %s\n", g_mt5806_tx_irq_name[i]);
	}
}

static int mt5806_tx_get_interrupt(struct mt5806_dev_info *di, u32 *intr)
{
	int ret;

	ret = mt5806_read_block(di, MT5806_TX_IRQ_ADDR, (u8 *)intr, MT5806_TX_IRQ_LEN);
	if (ret)
		return ret;

	hwlog_info("[get_interrupt] irq=0x%08x\n", *intr);
	mt5806_tx_show_irq(*intr);

	return 0;
}

static void mt5806_tx_mode_irq_recheck(struct mt5806_dev_info *di)
{
	int ret;
	u32 irq_val = 0;

	if (gpio_get_value(di->gpio_int))
		return;

	hwlog_info("[tx_mode_irq_recheck] gpio_int low, re-check irq\n");
	ret = mt5806_tx_get_interrupt(di, &irq_val);
	if (ret)
		return;

	mt5806_tx_clear_irq(di, MT5806_TX_IRQ_CLR_ALL);
}

static int mt5806_tx_q_calibration(void *dev_data)
{
	int ret;
	struct mt5806_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->q_cali_result = MT5806_Q_CALIBRATING;
	di->g_val.tx_calibrate_flag = true;
	di->tx_q_factor.tx_q_flag = MT5806_Q_CALI_UNVALID;
	power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
		POWER_NE_WLTX_TX_Q_CALIBRATION, &di->q_cali_result);

	hwlog_info("[q_calibration] send q_calibration cmd\n");
	ret = mt5806_write_dword_mask(di, MT5806_TX_CMD_ADDR, MT5806_TX_CMD_Q_SCAN,
		MT5806_TX_CMD_Q_SCAN_SHIFT, MT5806_TX_CMD_VAL);

	return 0;
}

void mt5806_tx_mode_irq_handler(struct mt5806_dev_info *di)
{
	int ret;

	if (!di)
		return;

	ret = mt5806_tx_get_interrupt(di, &di->irq_val);
	if (ret) {
		hwlog_err("irq_handler: get irq failed, clear\n");
		mt5806_tx_clear_irq(di, MT5806_TX_IRQ_CLR_ALL);
		goto recheck_irq;
	}

	mt5806_tx_clear_irq(di, di->irq_val);

	mt5806_tx_ask_pkt_handler(di);
	mt5806_tx_q_detect_handler(di);

	if (di->irq_val & MT5806_TX_IRQ_START_PING) {
		di->irq_val &= ~MT5806_TX_IRQ_START_PING;
		power_event_bnc_notify(mt5806_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_PING_RX, NULL);
	}
	if (di->irq_val & MT5806_TX_IRQ_EPT_PKT_RCVD) {
		di->irq_val &= ~MT5806_TX_IRQ_EPT_PKT_RCVD;
		mt5806_tx_ept_handler(di);
	}

recheck_irq:
	mt5806_tx_mode_irq_recheck(di);
}

static struct wltx_ic_ops g_mt5806_tx_ic_ops = {
	.get_dev_node           = mt5806_dts_dev_node,
	.fw_update              = mt5806_fw_sram_update,
	.prev_psy               = NULL,
	.chip_init              = mt5806_tx_chip_init,
	.chip_reset             = mt5806_chip_reset,
	.chip_enable            = mt5806_chip_enable,
	.mode_enable            = mt5806_tx_enable_tx_mode,
	.activate_chip          = mt5806_tx_activate_chip,
	.set_open_flag          = mt5806_tx_set_tx_open_flag,
	.set_stop_cfg           = mt5806_tx_stop_config,
	.is_rx_discon           = mt5806_tx_check_rx_disconnect,
	.is_in_tx_mode          = mt5806_tx_is_tx_mode,
	.is_in_rx_mode          = mt5806_tx_is_rx_mode,
	.get_vrect              = mt5806_tx_get_vrect,
	.get_vin                = mt5806_tx_get_vin,
	.get_iin                = mt5806_tx_get_iin,
	.get_temp               = mt5806_tx_get_temp,
	.get_fop                = mt5806_tx_get_fop,
	.get_cep                = mt5806_tx_get_cep,
	.get_duty               = mt5806_tx_get_duty,
	.get_ptx                = mt5806_tx_get_ptx,
	.get_prx                = mt5806_tx_get_prx,
	.get_ploss              = mt5806_tx_get_ploss,
	.get_ploss_id           = mt5806_tx_get_ploss_id,
	.get_ping_freq          = mt5806_tx_get_ping_freq,
	.get_ping_interval      = mt5806_tx_get_ping_interval,
	.get_min_fop            = mt5806_tx_get_min_fop,
	.get_max_fop            = mt5806_tx_get_max_fop,
	.get_full_bridge_ith    = mt5806_tx_get_full_bridge_ith,
	.set_ping_freq          = mt5806_tx_set_ping_freq,
	.set_ping_interval      = mt5806_tx_set_ping_interval,
	.set_min_fop            = mt5806_tx_set_min_fop,
	.set_max_fop            = mt5806_tx_set_max_fop,
	.set_ilim               = mt5806_tx_set_ilimit,
	.set_vset               = mt5806_tx_set_vset,
	.set_fod_coef           = NULL,
	.set_rp_dm_to           = NULL,
	.set_bridge             = mt5806_tx_set_bridge,
	.lowpower_enable        = mt5806_tx_lowpower_enable,
	.q_calibration          = mt5806_tx_q_calibration,
};

int mt5806_tx_ops_register(struct wltrx_ic_ops *ops, struct mt5806_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->tx_ops = kzalloc(sizeof(*(ops->tx_ops)), GFP_KERNEL);
	if (!ops->tx_ops)
		return -ENOMEM;

	memcpy(ops->tx_ops, &g_mt5806_tx_ic_ops, sizeof(g_mt5806_tx_ic_ops));
	ops->tx_ops->dev_data = (void *)di;

	return wltx_ic_ops_register(ops->tx_ops, di->ic_type);
}
