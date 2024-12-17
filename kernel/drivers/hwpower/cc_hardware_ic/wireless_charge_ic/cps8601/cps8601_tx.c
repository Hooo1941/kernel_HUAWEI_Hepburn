// SPDX-License-Identifier: GPL-2.0
/*
 * cps8601_tx.c
 *
 * cps8601 tx driver
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

#define HWLOG_TAG wireless_cps8601_tx
HWLOG_REGIST();

static const char * const g_cps8601_tx_irq_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_init",
	[1]  = "tx_start_ping",
	[2]  = "tx_ss_pkt_rcvd",
	[3]  = "tx_id_pkt_rcvd",
	[4]  = "tx_cfg_pkt_rcvd",
	[5]  = "tx_ask_pkt_rcvd",
	[6]  = "tx_ept",
	[7]  = "tx_rpp_timeout",
	[8]  = "tx_cep_timeout",
	[9]  = "tx_rx_attach",
	[10] = "tx_rx_removed",
	[11] = "tx_ept_other",
	[12] = "tx_q_calibration",
};

static const char * const g_cps8601_tx_ept_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_ept_src_wrong_pkt",
	[2]  = "tx_ept_src_ss",
	[3]  = "tx_ept_src_rx_ept",
	[4]  = "tx_ept_src_cep_timeout",
	[6]  = "tx_ept_src_ocp",
	[7]  = "tx_ept_src_ovp",
	[8]  = "tx_ept_src_uvp",
	[9]  = "tx_ept_src_fod",
	[10] = "tx_ept_src_otp",
	[11] = "tx_ept_src_ping_ocp",
};

unsigned int cps8601_tx_get_bnt_wltx_type(int ic_type)
{
	return ic_type == WLTRX_IC_AUX ? POWER_BNT_WLTX_AUX : POWER_BNT_WLTX;
}

static bool cps8601_tx_is_tx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;
	struct cps8601_dev_info *di = dev_data;

	ret = cps8601_read_byte(di, CPS8601_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("is_tx_mode: get op_mode failed\n");
		return false;
	}

	return mode == CPS8601_OP_MODE_TX;
}

static int cps8601_tx_set_tx_open_flag(bool enable, void *dev_data)
{
	return 0;
}

static void cps8601_tx_chip_reset(void *dev_data)
{
	int ret;
	struct cps8601_dev_info *di = dev_data;

	ret = cps8601_write_word_mask(di, CPS8601_TX_CMD_ADDR, CPS8601_TX_CMD_SYS_RST,
		CPS8601_TX_CMD_SYS_RST_SHIFT, CPS8601_TX_CMD_VAL);
	if (ret) {
		hwlog_err("chip_reset: set cmd failed\n");
		return;
	}

	hwlog_info("[chip_reset] succ\n");
}

static bool cps8601_tx_check_rx_disconnect(void *dev_data)
{
	struct cps8601_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("check_rx_disconnect: di null\n");
		return true;
	}

	if (di->ept_type & CPS8601_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~CPS8601_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[check_rx_disconnect] rx disconnect\n");
		return true;
	}

	return false;
}

static int cps8601_tx_get_ping_interval(u16 *ping_interval, void *dev_data)
{
	return cps8601_read_word(dev_data, CPS8601_TX_PING_INTERVAL_ADDR, ping_interval);
}

static int cps8601_tx_set_ping_interval(u16 ping_interval, void *dev_data)
{
	if ((ping_interval < CPS8601_TX_PING_INTERVAL_MIN) ||
		(ping_interval > CPS8601_TX_PING_INTERVAL_MAX)) {
		hwlog_err("set_ping_interval: para out of range\n");
		return -EINVAL;
	}

	return cps8601_write_word(dev_data, CPS8601_TX_PING_INTERVAL_ADDR, ping_interval);
}

static int cps8601_tx_get_ping_frequency(u16 *ping_freq, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!ping_freq)
		return -EINVAL;

	ret = cps8601_read_word(dev_data, CPS8601_TX_PING_FREQ_ADDR, &data);
	if (ret)
		return ret;

	*ping_freq = data / CPS8601_TX_PING_FREQ_UNIT;
	return 0;
}

static int cps8601_tx_set_ping_frequency(u16 ping_freq, void *dev_data)
{
	if ((ping_freq < CPS8601_TX_PING_FREQ_MIN) ||
		(ping_freq > CPS8601_TX_PING_FREQ_MAX)) {
		hwlog_err("set_ping_frequency: para out of range\n");
		return -EINVAL;
	}

	return cps8601_write_word(dev_data, CPS8601_TX_PING_FREQ_ADDR,
		CPS8601_TX_PING_FREQ_UNIT * ping_freq);
}

static int cps8601_tx_get_min_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!fop)
		return -EINVAL;

	ret = cps8601_read_word(dev_data, CPS8601_TX_MIN_FOP_ADDR, &data);
	if (ret)
		return ret;

	*fop = data / CPS8601_TX_OP_FREQ_UNIT;
	return 0;
}

static int cps8601_tx_set_min_fop(u16 fop, void *dev_data)
{
	if ((fop < CPS8601_TX_MIN_FOP) || (fop > CPS8601_TX_MAX_FOP)) {
		hwlog_err("set_min_fop: para out of range\n");
		return -EINVAL;
	}

	return cps8601_write_word(dev_data, CPS8601_TX_MIN_FOP_ADDR, CPS8601_TX_OP_FREQ_UNIT * fop);
}

static int cps8601_tx_get_max_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!fop)
		return -EINVAL;

	ret = cps8601_read_word(dev_data, CPS8601_TX_MAX_FOP_ADDR, &data);
	if (ret)
		return ret;

	*fop = data / CPS8601_TX_OP_FREQ_UNIT;
	return 0;
}

static int cps8601_tx_set_max_fop(u16 fop, void *dev_data)
{
	if ((fop < CPS8601_TX_MIN_FOP) || (fop > CPS8601_TX_MAX_FOP)) {
		hwlog_err("set_max_fop: para out of range\n");
		return -EINVAL;
	}

	return cps8601_write_word(dev_data, CPS8601_TX_MAX_FOP_ADDR, CPS8601_TX_OP_FREQ_UNIT * fop);
}

static int cps8601_tx_get_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!fop)
		return -EINVAL;

	ret = cps8601_read_word(dev_data, CPS8601_TX_OP_FREQ_ADDR, &data);
	if (ret)
		return ret;

	*fop = data / CPS8601_TX_OP_FREQ_UNIT;
	return 0;
}

static int cps8601_tx_get_cep(s8 *cep, void *dev_data)
{
	return 0;
}

static int cps8601_tx_get_duty(u8 *duty, void *dev_data)
{
	int ret;
	u16 pwm_duty = 0;

	if (!duty) {
		hwlog_err("get_duty: para null\n");
		return -EINVAL;
	}

	ret = cps8601_read_word(dev_data, CPS8601_TX_PWM_DUTY_ADDR, &pwm_duty);
	if (ret)
		return ret;

	*duty = pwm_duty / CPS8601_TX_PWM_DUTY_UNIT;
	return 0;
}

static int cps8601_tx_get_ptx(u32 *ptx, void *dev_data)
{
	return 0;
}

static int cps8601_tx_get_prx(u32 *prx, void *dev_data)
{
	return 0;
}

static int cps8601_tx_get_ploss(s32 *ploss, void *dev_data)
{
	return 0;
}

static int cps8601_tx_get_ploss_id(u8 *id, void *dev_data)
{
	return 0;
}

static int cps8601_tx_get_temp(s16 *chip_temp, void *dev_data)
{
	return cps8601_read_byte(dev_data, CPS8601_TX_CHIP_TEMP_ADDR, (u8 *)chip_temp);
}

static int cps8601_tx_get_vin(u16 *tx_vin, void *dev_data)
{
	return cps8601_read_word(dev_data, CPS8601_TX_VIN_ADDR, tx_vin);
}

static int cps8601_tx_get_vrect(u16 *tx_vrect, void *dev_data)
{
	return cps8601_read_word(dev_data, CPS8601_TX_VRECT_ADDR, tx_vrect);
}

static int cps8601_tx_get_iin(u16 *tx_iin, void *dev_data)
{
	return cps8601_read_word(dev_data, CPS8601_TX_IIN_ADDR, tx_iin);
}

static int cps8601_tx_set_fod_coef(u16 pl_th, u8 pl_cnt, void *dev_data)
{
	return 0;
}

static int cps8601_tx_init_fod_coef(struct cps8601_dev_info *di)
{
	int ret;

	ret = cps8601_write_word(di, CPS8601_TX_PLOSS_TH0_ADDR, di->tx_fod.ploss_th0);
	ret += cps8601_write_word(di, CPS8601_TX_PLOSS_TH1_ADDR, di->tx_fod.ploss_th1);
	ret += cps8601_write_word(di, CPS8601_TX_PLOSS_TH2_ADDR, di->tx_fod.ploss_th2);
	ret += cps8601_write_byte(di, CPS8601_TX_PLOSS_CNT_ADDR, di->tx_fod.ploss_cnt);
	ret += cps8601_write_word(di, CPS8601_TX_PLOSS_RP_TH0_ADDR, di->tx_fod.ploss_rp_th0);
	if (ret) {
		hwlog_err("init_fod_coef: failed\n");
		return ret;
	}

	return 0;
}

static int cps8601_tx_set_rp_dm_timeout_val(u8 val, void *dev_data)
{
	return 0;
}

static int cps8601_tx_stop_config(void *dev_data)
{
	struct cps8601_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("stop_charging: para null\n");
		return -EINVAL;
	}

	di->g_val.tx_stop_chrg_flag = true;
	return 0;
}

static void cps8601_tx_select_init_para(struct cps8601_dev_info *di)
{
	di->tx_init_para.ping_freq = CPS8601_TX_PING_FREQ;
	di->tx_init_para.ping_interval = CPS8601_TX_PING_INTERVAL;
}

static int cps8601_tx_set_init_para(struct cps8601_dev_info *di)
{
	int ret;

	ret = cps8601_write_word(di, CPS8601_TX_OCP_TH_ADDR, di->tx_ocp_th);
	ret += cps8601_write_word(di, CPS8601_TX_OVP_TH_ADDR, CPS8601_TX_OVP_TH);
	ret += cps8601_write_word(di, CPS8601_TX_IRQ_EN_ADDR, CPS8601_TX_IRQ_VAL);
	ret += cps8601_write_word(di, CPS8601_TX_PING_OCP_ADDR, di->tx_pocp_th);
	ret += cps8601_tx_init_fod_coef(di);
	ret += cps8601_tx_set_ping_frequency(di->tx_init_para.ping_freq, di);
	ret += cps8601_tx_set_min_fop(di->tx_fop.tx_min_fop, di);
	ret += cps8601_tx_set_max_fop(di->tx_fop.tx_max_fop, di);
	ret += cps8601_tx_set_ping_interval(di->tx_init_para.ping_interval, di);
	ret += cps8601_write_byte(di, CPS8601_TX_PING_TIME_ADDR, CPS8601_TX_PING_TIME);
	ret += cps8601_write_word(di, CPS8601_TX_CEP_TIMEOUT_ADDR, di->tx_cep_timeout);
	if (ret) {
		hwlog_err("set_init_para: write failed\n");
		return -EIO;
	}

	return 0;
}

static int cps8601_tx_chip_init(unsigned int client, void *dev_data)
{
	struct cps8601_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("chip_init: di null\n");
		return -EINVAL;
	}

	di->irq_cnt = 0;
	di->g_val.tx_stop_chrg_flag = false;
	cps8601_enable_irq(di);

	cps8601_tx_select_init_para(di);

	return cps8601_tx_set_init_para(di);
}

static int cps8601_tx_enable_tx_mode(bool enable, void *dev_data)
{
	int ret;
	struct cps8601_dev_info *di = dev_data;

	if (enable)
		ret = cps8601_write_word_mask(di, CPS8601_TX_CMD_ADDR,
			CPS8601_TX_CMD_EN_TX, CPS8601_TX_CMD_EN_TX_SHIFT, CPS8601_TX_CMD_VAL);
	else
		ret = cps8601_write_word_mask(di, CPS8601_TX_CMD_ADDR,
			CPS8601_TX_CMD_DIS_TX, CPS8601_TX_CMD_DIS_TX_SHIFT, CPS8601_TX_CMD_VAL);

	if (ret) {
		hwlog_err("%s tx_mode failed\n", enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static void cps8601_tx_show_ept_type(u16 ept)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_cps8601_tx_ept_name); i++) {
		if (ept & BIT(i))
			hwlog_info("[tx_ept] %s\n", g_cps8601_tx_ept_name[i]);
	}
}

static int cps8601_tx_get_ept_type(struct cps8601_dev_info *di, u16 *ept)
{
	int ret;
	u16 ept_value = 0;

	if (!ept) {
		hwlog_err("get_ept_type: para null\n");
		return -EINVAL;
	}

	ret = cps8601_read_word(di, CPS8601_TX_EPT_SRC_ADDR, &ept_value);
	if (ret) {
		hwlog_err("get_ept_type: read failed\n");
		return ret;
	}
	*ept = ept_value;
	hwlog_info("[get_ept_type] type=0x%02x", *ept);
	cps8601_tx_show_ept_type(*ept);

	return 0;
}

static int cps8601_tx_clear_ept_src(struct cps8601_dev_info *di)
{
	int ret;

	ret = cps8601_write_word(di, CPS8601_TX_EPT_SRC_ADDR, CPS8601_TX_EPT_SRC_CLEAR);
	if (ret) {
		hwlog_err("clear_ept_src: failed\n");
		return ret;
	}

	hwlog_info("[clear_ept_src] success\n");
	return 0;
}

static void cps8601_tx_ept_handler(struct cps8601_dev_info *di)
{
	int ret;
	u8 rx_ept_value = 0;

	ret = cps8601_tx_get_ept_type(di, &di->ept_type);
	ret += cps8601_tx_clear_ept_src(di);
	if (ret)
		return;

	switch (di->ept_type) {
	case CPS8601_TX_EPT_SRC_RX_EPT:
		di->ept_type &= ~CPS8601_TX_EPT_SRC_RX_EPT;
		ret = cps8601_read_byte(di, CPS8601_TX_RCVD_RX_EPT_ADDR, &rx_ept_value);
		ret += cps8601_write_byte(di, CPS8601_TX_RCVD_RX_EPT_ADDR, CPS8601_TX_RCVD_RX_EPT_CLEAR);
		hwlog_info("[ept_handler] type=0x%02x, ret=%d\n", rx_ept_value, ret);
		/* fall-through */
	case CPS8601_TX_EPT_SRC_CEP_TIMEOUT:
		di->ept_type &= ~CPS8601_TX_EPT_SRC_CEP_TIMEOUT;
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_EPT_CMD, NULL);
		break;
	case CPS8601_TX_EPT_SRC_WRONG_PKT:
	case CPS8601_TX_EPT_SRC_SSP:
	case CPS8601_TX_EPT_SRC_OCP:
	case CPS8601_TX_EPT_SRC_OVP:
	case CPS8601_TX_EPT_SRC_UVP:
	case CPS8601_TX_EPT_SRC_OTP:
		di->ept_type &= ~(CPS8601_TX_EPT_SRC_WRONG_PKT | CPS8601_TX_EPT_SRC_SSP |
			CPS8601_TX_EPT_SRC_OCP | CPS8601_TX_EPT_SRC_OVP | CPS8601_TX_EPT_SRC_UVP |
			CPS8601_TX_EPT_SRC_OTP);
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_EPT_CMD, NULL);
		break;
	case CPS8601_TX_EPT_SRC_FOD:
		di->ept_type &= ~CPS8601_TX_EPT_SRC_FOD;
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	case CPS8601_TX_EPT_SRC_POCP:
		di->ept_type &= ~CPS8601_TX_EPT_SRC_POCP;
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			(di->ic_type == WLTRX_IC_AUX) ? POWER_NE_WLTX_TX_PING_OCP :
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	default:
		break;
	}
}

static int cps8601_tx_clear_irq(struct cps8601_dev_info *di, u16 itr)
{
	int ret;

	ret = cps8601_write_word(di, CPS8601_TX_IRQ_CLR_ADDR, itr);
	if (ret) {
		hwlog_err("clear_irq: write failed\n");
		return ret;
	}

	return 0;
}

static void cps8601_tx_ask_pkt_handler(struct cps8601_dev_info *di)
{
	if (di->irq_val & CPS8601_TX_IRQ_SS_PKG_RCVD) {
		di->irq_val &= ~CPS8601_TX_IRQ_SS_PKG_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & CPS8601_TX_IRQ_ID_PKT_RCVD) {
		di->irq_val &= ~CPS8601_TX_IRQ_ID_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & CPS8601_TX_IRQ_CFG_PKT_RCVD) {
		di->irq_val &= ~CPS8601_TX_IRQ_CFG_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_GET_CFG, NULL);
	}

	if (di->irq_val & CPS8601_TX_IRQ_ASK_PKT_RCVD) {
		di->irq_val &= ~CPS8601_TX_IRQ_ASK_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_non_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_non_qi_ask_pkt(di);
	}
}

static int cps8601_tx_get_sample_para(struct cps8601_dev_info *di)
{
	int ret;

	ret = cps8601_read_byte(di, CPS8601_TX_NO_RX_CNT_ADDR, (u8 *)&di->tx_q_factor.tx_q_cnt);
	ret += cps8601_read_word(di, CPS8601_TX_NO_RX_WIDTH_ADDR, &di->tx_q_factor.tx_q_width);
	ret += cps8601_read_byte(di, CPS8601_TX_NO_RX_CNT_VAR_ADDR, &di->tx_q_factor.tx_q_cnt_var);
	ret += cps8601_read_byte(di, CPS8601_TX_NO_RX_WIDTH_VAR_ADDR,
		&di->tx_q_factor.tx_q_width_var);
	ret += cps8601_read_word(di, CPS8601_TX_CALI_FAC_WIDTH_ADDR, &di->tx_q_factor.tx_q_fac_width);
	ret += cps8601_read_word(di, CPS8601_TX_CALI_DYN_WIDTH_ADDR, &di->tx_q_factor.tx_q_dyn_width);
	hwlog_info("[get_sample_para] q sample cnt=%u, cnt_var=%u, width=%u, width_var=%u,"
		" fac_width=%u, dyn_width=%u\n", di->tx_q_factor.tx_q_cnt, di->tx_q_factor.tx_q_cnt_var,
		di->tx_q_factor.tx_q_width, di->tx_q_factor.tx_q_width_var, di->tx_q_factor.tx_q_fac_width,
		di->tx_q_factor.tx_q_dyn_width);

	return ret;
}

static int cps8601_tx_send_bigdata(struct imonitor_eventobj *obj, void *dev_data)
{
	struct cps8601_dev_info *di = dev_data;

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

static void cps8601_tx_q_cali_handler(struct cps8601_dev_info *di)
{
	int ret;
	s16 delta_fac;
	s16 delta_dyn;

	if (!di->g_val.tx_calibrate_flag)
		return;

	ret = cps8601_tx_get_sample_para(di);
	if (ret) {
		hwlog_err("q_cali_handler: get sample failed, stop calibrate\n");
		cps8601_tx_lowpower_enable(true, di);
		return;
	}

	di->tx_q_factor.tx_q_spl_width = di->tx_q_factor.tx_q_width;
	delta_fac = (s16)(di->tx_q_factor.tx_q_width - di->tx_q_factor.tx_q_fac_width);
	delta_dyn = (s16)(di->tx_q_factor.tx_q_width - di->tx_q_factor.tx_q_dyn_width);
	if (((di->tx_q_factor.tx_q_fac_width < CPS8601_Q_FACTORY_WIDTH_LTH) ||
		(di->tx_q_factor.tx_q_fac_width > CPS8601_Q_FACTORY_WIDTH_HTH))) {
		if ((di->tx_q_factor.tx_q_width >= CPS8601_Q_SAMPLE_WIDTH_LTH) &&
			(di->tx_q_factor.tx_q_width <= CPS8601_Q_SAMPLE_WIDTH_HTH)) {
			di->tx_q_factor.tx_q_flag = CPS8601_Q_CALI_FACTORY;
			cps8601_fw_program_q_data(di);
		} else {
			cps8601_tx_lowpower_enable(true, di);
		}
	} else {
		if (di->q_cali_dynamic_en &&
			(di->tx_q_factor.tx_q_width >= CPS8601_Q_SAMPLE_WIDTH_LTH) &&
			(di->tx_q_factor.tx_q_width <= CPS8601_Q_SAMPLE_WIDTH_HTH) &&
			(di->tx_q_factor.tx_q_width_var <= CPS8601_Q_SAMPLE_WIDTH_VAR_TH) &&
			(delta_fac >= CPS8601_Q_FAC_WIDTH_DELTA_LTH) &&
			(delta_fac <= CPS8601_Q_FAC_WIDTH_DELTA_HTH) &&
			(((delta_dyn >= CPS8601_Q_DYN_WIDTH_DELTA_LTH0) &&
			(delta_dyn < CPS8601_Q_DYN_WIDTH_DELTA_HTH0)) ||
			((delta_dyn > CPS8601_Q_DYN_WIDTH_DELTA_LTH1) &&
			(delta_dyn <= CPS8601_Q_DYN_WIDTH_DELTA_HTH1)))) {
			di->tx_q_factor.tx_q_flag = CPS8601_Q_CALI_DYNAMIC;
			di->tx_q_factor.tx_q_width = di->tx_q_factor.tx_q_dyn_width + delta_dyn / 3;
			hwlog_info("[q_cali_handler] calc_cali_cnt=%u, calc_cali_wid=%u\n",
				di->tx_q_factor.tx_q_cnt, di->tx_q_factor.tx_q_width);
			cps8601_fw_program_q_data(di);
		} else {
			cps8601_tx_lowpower_enable(true, di);
		}
	}

	di->g_val.tx_calibrate_flag = false;
	power_bigdata_report(POWER_BIGDATA_TYPE_WLTX_AUX_CALIBRATE, cps8601_tx_send_bigdata, di);
}

static void cps8601_tx_q_detect_handler(struct cps8601_dev_info *di)
{
	if (di->default_psy_type != CPS8601_DEFAULT_LOWPOWER)
		return;

	if (di->irq_val & CPS8601_TX_IRQ_RX_ATTACH) {
		di->irq_val &= ~CPS8601_TX_IRQ_RX_ATTACH;
		gpio_set_value(di->gpio_rx_online, true);
		hwlog_info("[q_detect_handler] rx_online=%d\n", gpio_get_value(di->gpio_rx_online));
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_HALL_APPROACH, NULL);
	}
	if (di->irq_val & CPS8601_TX_IRQ_RX_REMOVED) {
		di->irq_val &= ~CPS8601_TX_IRQ_RX_REMOVED;
		gpio_set_value(di->gpio_rx_online, false);
		hwlog_info("[q_detect_handler] rx_online=%d\n", gpio_get_value(di->gpio_rx_online));
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_HALL_AWAY_FROM, NULL);
	}
	if (di->irq_val & CPS8601_TX_IRQ_Q_CAIL) {
		di->irq_val &= ~CPS8601_TX_IRQ_Q_CAIL;
		cps8601_tx_q_cali_handler(di);
	}
}

static void cps8601_tx_show_irq(u32 intr)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_cps8601_tx_irq_name); i++) {
		if (intr & BIT(i))
			hwlog_info("[tx_irq] %s\n", g_cps8601_tx_irq_name[i]);
	}
}

static int cps8601_tx_get_interrupt(struct cps8601_dev_info *di, u16 *intr)
{
	int ret;

	ret = cps8601_read_word(di, CPS8601_TX_IRQ_ADDR, intr);
	if (ret)
		return ret;

	hwlog_info("[get_interrupt] irq=0x%04x\n", *intr);
	cps8601_tx_show_irq(*intr);

	return 0;
}

static void cps8601_tx_mode_irq_recheck(struct cps8601_dev_info *di)
{
	int ret;
	u16 irq_val = 0;

	if (gpio_get_value(di->gpio_int))
		return;

	hwlog_info("[tx_mode_irq_recheck] gpio_int low, re-check irq\n");
	ret = cps8601_tx_get_interrupt(di, &irq_val);
	if (ret)
		return;

	cps8601_tx_clear_irq(di, CPS8601_TX_IRQ_CLR_ALL);
}

void cps8601_tx_mode_irq_handler(struct cps8601_dev_info *di)
{
	int ret;

	if (!di)
		return;

	ret = cps8601_tx_get_interrupt(di, &di->irq_val);
	if (ret) {
		hwlog_err("irq_handler: get irq failed, clear\n");
		cps8601_tx_clear_irq(di, CPS8601_TX_IRQ_CLR_ALL);
		goto recheck_irq;
	}

	cps8601_tx_clear_irq(di, di->irq_val);

	cps8601_tx_ask_pkt_handler(di);
	cps8601_tx_q_detect_handler(di);

	if (di->irq_val & CPS8601_TX_IRQ_START_PING) {
		di->irq_val &= ~CPS8601_TX_IRQ_START_PING;
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_PING_RX, NULL);
	}
	if (di->irq_val & CPS8601_TX_IRQ_EPT_PKT_RCVD) {
		di->irq_val &= ~CPS8601_TX_IRQ_EPT_PKT_RCVD;
		cps8601_tx_ept_handler(di);
	}
	if (di->irq_val & CPS8601_TX_IRQ_RPP_TIMEOUT) {
		di->irq_val &= ~CPS8601_TX_IRQ_RPP_TIMEOUT;
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_EPT_CMD, NULL);
	}
	if (di->irq_val & CPS8601_TX_IRQ_OTHER_ERROR) {
		di->irq_val &= ~CPS8601_TX_IRQ_OTHER_ERROR;
		power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_EPT_CMD, NULL);
	}

recheck_irq:
	cps8601_tx_mode_irq_recheck(di);
}

static int cps8601_tx_activate_chip(void *dev_data)
{
	return 0;
}

static int cps8601_tx_set_vset(int tx_vset, void *dev_data)
{
	return 0;
}

int cps8601_tx_lowpower_enable(bool enable, void *dev_data)
{
	struct cps8601_dev_info *di = dev_data;

	if (!di || !di->g_val.mtp_chk_complete)
		return -ENODEV;

	hwlog_info("[lowpower_enable] %s lowpower\n", enable ? "enter" : "exit");
	if (enable) {
		return cps8601_write_word_mask(di, CPS8601_TX_CMD_ADDR, CPS8601_TX_CMD_EN_DLP_MODE,
			CPS8601_TX_CMD_EN_DLP_MODE_SHIFT, CPS8601_TX_CMD_VAL);
	} else {
		/* power off to force exit lowpower */
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
		power_msleep(DT_MSLEEP_2S, 0, NULL);
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
		power_msleep(DT_MSLEEP_100MS, 0, NULL);
	}

	return 0;
}

static int cps8601_tx_q_calibration(void *dev_data)
{
	struct cps8601_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->q_cali_result = CPS8601_Q_CALIBRATING;
	di->g_val.tx_calibrate_flag = true;
	di->tx_q_factor.tx_q_flag = CPS8601_Q_CALI_UNVALID;
	power_event_bnc_notify(cps8601_tx_get_bnt_wltx_type(di->ic_type),
		POWER_NE_WLTX_TX_Q_CALIBRATION, &di->q_cali_result);

	hwlog_info("[q_calibration] send q_calibration cmd\n");
	return cps8601_write_word_mask(di, CPS8601_TX_CMD_ADDR, CPS8601_TX_CMD_Q_CAIL,
		CPS8601_TX_CMD_Q_CAIL_SHIFT, CPS8601_TX_CMD_VAL);
}

static struct wltx_ic_ops g_cps8601_tx_ops = {
	.get_dev_node           = cps8601_dts_dev_node,
	.fw_update              = cps8601_fw_sram_update,
	.chip_init              = cps8601_tx_chip_init,
	.chip_reset             = cps8601_tx_chip_reset,
	.chip_enable            = cps8601_chip_enable,
	.mode_enable            = cps8601_tx_enable_tx_mode,
	.activate_chip          = cps8601_tx_activate_chip,
	.set_open_flag          = cps8601_tx_set_tx_open_flag,
	.set_stop_cfg           = cps8601_tx_stop_config,
	.is_rx_discon           = cps8601_tx_check_rx_disconnect,
	.is_in_tx_mode          = cps8601_tx_is_tx_mode,
	.get_vrect              = cps8601_tx_get_vrect,
	.get_vin                = cps8601_tx_get_vin,
	.get_iin                = cps8601_tx_get_iin,
	.get_temp               = cps8601_tx_get_temp,
	.get_fop                = cps8601_tx_get_fop,
	.get_cep                = cps8601_tx_get_cep,
	.get_duty               = cps8601_tx_get_duty,
	.get_ptx                = cps8601_tx_get_ptx,
	.get_prx                = cps8601_tx_get_prx,
	.get_ploss              = cps8601_tx_get_ploss,
	.get_ploss_id           = cps8601_tx_get_ploss_id,
	.get_ping_freq          = cps8601_tx_get_ping_frequency,
	.get_ping_interval      = cps8601_tx_get_ping_interval,
	.get_min_fop            = cps8601_tx_get_min_fop,
	.get_max_fop            = cps8601_tx_get_max_fop,
	.set_ping_freq          = cps8601_tx_set_ping_frequency,
	.set_ping_interval      = cps8601_tx_set_ping_interval,
	.set_min_fop            = cps8601_tx_set_min_fop,
	.set_max_fop            = cps8601_tx_set_max_fop,
	.set_vset               = cps8601_tx_set_vset,
	.set_fod_coef           = cps8601_tx_set_fod_coef,
	.set_rp_dm_to           = cps8601_tx_set_rp_dm_timeout_val,
	.lowpower_enable        = cps8601_tx_lowpower_enable,
	.q_calibration          = cps8601_tx_q_calibration,
};

int cps8601_tx_ops_register(struct wltrx_ic_ops *ops, struct cps8601_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->tx_ops = kzalloc(sizeof(*(ops->tx_ops)), GFP_KERNEL);
	if (!ops->tx_ops)
		return -ENOMEM;

	memcpy(ops->tx_ops, &g_cps8601_tx_ops, sizeof(g_cps8601_tx_ops));
	ops->tx_ops->dev_data = (void *)di;

	return wltx_ic_ops_register(ops->tx_ops, di->ic_type);
}
