// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_tx_pwr_src.c
 *
 * power source for wireless reverse charging
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

#include <chipset_common/hwpower/wireless_charge/wireless_tx_pwr_src.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <huawei_platform/hwpower/wireless_charge/wireless_rx_platform.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <huawei_platform/power/huawei_charger_common.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/hardware_channel/charger_channel.h>
#include <chipset_common/hwpower/accessory/wireless_lightstrap.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>

#include <linux/kernel.h>

#define HWLOG_TAG wireless_tx_pwr_src
HWLOG_REGIST();

static struct {
	enum wltx_pwr_src src;
	const char *name;
} const g_pwr_src[] = {
	{ PWR_SRC_NULL, "PWR_SRC_NULL" },
	{ PWR_SRC_VBUS, "PWR_SRC_VBUS" },
	{ PWR_SRC_OTG, "PWR_SRC_OTG" },
	{ PWR_SRC_5VBST, "PWR_SRC_5VBST" },
	{ PWR_SRC_SPBST, "PWR_SRC_SPBST" },
	{ PWR_SRC_VBUS_CP, "PWR_SRC_VBUS_CP" },
	{ PWR_SRC_OTG_CP, "PWR_SRC_OTG_CP" },
	{ PWR_SRC_BP2CP, "PWR_SRC_BP2CP" },
	{ PWR_SRC_VBUS2, "PWR_SRC_VBUS2" },
	{ PWR_SRC_5VBST_WLSIN, "PWR_SRC_5VBST_WLSIN"},
	{ PWR_SRC_RVS_SC_CP, "PWR_SRC_RVS_SC_CP"},
	{ PWR_SRC_RVS_SC4_CP, "PWR_SRC_RVS_SC4_CP"},
	{ PWR_SRC_NA, "PWR_SRC_NA" },
};

const char *wltx_get_pwr_src_name(enum wltx_pwr_src src)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(g_pwr_src); i++) {
		if (src == g_pwr_src[i].src)
			return g_pwr_src[i].name;
	}
	return "NA";
}

static enum wltx_pwr_src wltx_set_vbus_output(bool enable)
{
	charge_set_adapter_voltage(enable ? ADAPTER_5V : ADAPTER_9V,
		RESET_ADAPTER_SOURCE_WLTX, 0);
	return enable ? PWR_SRC_VBUS : PWR_SRC_NULL;
}

static void wltx_otg_output_set_wired_channel(bool enable)
{
	if (!wdcm_dev_exist())
		return;

	if (enable)
		wdcm_set_buck_channel_state(WDCM_CLIENT_TX_OTG, WDCM_DEV_ON);
	else
		wdcm_set_buck_channel_state(WDCM_CLIENT_TX_OTG, WDCM_DEV_OFF);
}

static enum wltx_pwr_src wltx_set_otg_output(bool enable)
{
	int i;
	int vout = 0;

	if (enable) {
		/*
		 * only the serial charging solution may have this problem.
		 * if the battery is poured back to usb, the otg boost does not output.
		 * delay 60*50 = 3000ms for direct charger check finish.
		 */
		for (i = 0; i < 60; i++) {
			if (!direct_charge_in_mode_check() &&
				direct_charge_get_stop_charging_complete_flag())
				break;
			if (wltx_msleep(50))
				return PWR_SRC_NULL;
		}
	}

	if (!enable) {
		(void)charge_otg_mode_enable(false, VBUS_CH_USER_WR_TX);
		charge_pump_chip_enable(CP_TYPE_MAIN, false);
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
		charger_select_channel(CHARGER_CH_USBIN);
		wltx_otg_output_set_wired_channel(enable);
		return PWR_SRC_NULL;
	}

	wltx_otg_output_set_wired_channel(enable);
	charger_select_channel(CHARGER_CH_WLSIN);
	(void)charge_otg_mode_enable(true, VBUS_CH_USER_WR_TX);
	/* delay 50*10 = 500ms for otg output, typically 250ms */
	for (i = 0; i < 10; i++) {
		if (wltx_msleep(50))
			goto fail;
		charge_get_vbus(&vout);
		hwlog_info("[set_otg_output] vout=%dmV\n", vout);
		if ((vout >= WLTX_OTG_VOUT_LTH) && (vout < WLTX_OTG_VOUT_HTH)) {
			wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
			charge_pump_chip_enable(CP_TYPE_MAIN, true);
			return PWR_SRC_OTG;
		}
	}

fail:
	(void)charge_otg_mode_enable(false, VBUS_CH_USER_WR_TX);
	charger_select_channel(CHARGER_CH_USBIN);
	wltx_otg_output_set_wired_channel(!enable);
	return PWR_SRC_NULL;
}

static void wltx_5vbst_hiz_ctrl(bool en_output)
{
	struct wltx_dts *dts = wltx_get_dts();
	static bool hiz_en_flag = false;

	if (!dts || !dts->bst5v_need_hiz_en)
		return;

	/* buck loses some 5vboost power, reduces power consumption of the coolingcase */
	if (en_output &&
		((wirelesscase_get_online_state() == WLCASE_ONLINE_COOLINGCASE) ||
		 (wirelesscase_get_online_state() == WLCASE_ONLINE_LIGHTSTRAP)) &&
		(wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF)) {
		hiz_en_flag = true;
		hwlog_info("[5vbst_hiz_ctrl] hiz enable\n");
		charge_set_hiz_enable(HIZ_MODE_ENABLE);
		return;
	}

	if (!en_output && hiz_en_flag) {
		hwlog_info("[5vbst_hiz_ctrl] hiz disable\n");
		charge_set_hiz_enable(HIZ_MODE_DISABLE);
		hiz_en_flag = false;
	}
}

void wltx_wlcase_set_lowpower(enum wltx_pwr_src src)
{
	if ((src == PWR_SRC_5VBST) || (src == PWR_SRC_5VBST_WLSIN))
		wltx_5vbst_hiz_ctrl(true);
}

static enum wltx_pwr_src wltx_set_5vbst_output(bool enable)
{
	wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, enable ? true : false);
	usleep_range(1000, 1050); /* 1ms */
	boost_5v_enable(enable, BOOST_CTRL_WLTX);
	if (!enable)
		wltx_5vbst_hiz_ctrl(enable);
	return enable ? PWR_SRC_5VBST : PWR_SRC_NULL;
}

static enum wltx_pwr_src wltx_set_5vbst_wlsin_output(bool enable)
{
	if (enable) {
		charger_select_channel(CHARGER_CH_WLSIN);
		boost_5v_enable(true, BOOST_CTRL_WLTX);
		wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, true);
		(void)wlrx_buck_set_dev_iin(100); /* 100mA, avoid vsys pumping power */
		return PWR_SRC_5VBST_WLSIN;
	}

	wltx_5vbst_hiz_ctrl(enable);
	wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, false);
	boost_5v_enable(false, BOOST_CTRL_WLTX);
	(void)wlrx_buck_set_dev_iin(2000); /* 2000mA default max icl */
	charger_select_channel(CHARGER_CH_USBIN);
	return PWR_SRC_NULL;
}

static enum wltx_pwr_src wltx_set_spbst_output(bool enable)
{
	wlps_control(WLTRX_IC_MAIN, WLPS_TX_PWR_SW, enable ? true : false);
	if (enable)
		(void)charge_pump_reverse_chip_init(CP_TYPE_MAIN);
	return enable ? PWR_SRC_SPBST : PWR_SRC_NULL;
}

static enum wltx_pwr_src wltx_set_bp2cp_output(bool enable)
{
	if (enable)
		(void)charge_pump_set_reverse_bp2cp_mode(CP_TYPE_MAIN);

	return enable ? PWR_SRC_BP2CP : PWR_SRC_NULL;
}

int wltx_cfg_adapter_output(int vol, int cur)
{
	hwlog_info("adaptor_cfg for wireless_tx: vol=%d, cur=%d\n", vol, cur);
	if (dc_set_adapter_output_capability(vol, cur, 0))
		return -EIO;

	power_platform_buck_vbus_change_handler(vol);
	return 0;
}

static enum wltx_pwr_src wltx_set_adaptor_output(int vset, int iset)
{
	int i;
	int vbus = 0;
	int adp_mode = ADAPTER_SUPPORT_UNDEFINED;

	/* delay 60*50 = 3000ms for direct charger check finish */
	for (i = 0; i < 60; i++) {
		if (!direct_charge_in_mode_check() &&
			direct_charge_get_stop_charging_complete_flag())
			break;
		if (wltx_msleep(50))
			return PWR_SRC_NULL;
	}
	if (!dc_adpt_detect_ping())
		adp_mode = dc_adpt_get_adapter_support_mode();
	if (adp_mode == ADAPTER_SUPPORT_UNDEFINED) {
		hwlog_err("set_adaptor_output: undefined adapter mode\n");
		return PWR_SRC_NULL;
	}
	charge_set_adapter_voltage(ADAPTER_5V, RESET_ADAPTER_SOURCE_WLTX, 0);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_SC, WIRED_CHANNEL_RESTORE);
	if (wltx_cfg_adapter_output(vset, iset)) {
		hwlog_err("set_adaptor_output: cfg vbus failed\n");
		goto set_output_fail;
	}

	/* delay 50*5 = 250ms for vbus output, typically 50-100ms */
	for (i = 0; i < 5; i++) {
		wltx_msleep(50);
		charge_get_vbus(&vbus);
		hwlog_info("[set_vbus_output] vbus=%dmV\n", vbus);
		if ((vbus >= WLTX_SC_VOL_LOW_TH) &&
			(vbus < WLTX_SC_VOL_HIGH_TH)) {
			wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
			charge_pump_chip_enable(CP_TYPE_MAIN, true);
			return PWR_SRC_VBUS;
		}
	}

set_output_fail:
	wired_chsw_set_wired_channel(WIRED_CHANNEL_SC, WIRED_CHANNEL_CUTOFF);
	charge_set_adapter_voltage(ADAPTER_9V, RESET_ADAPTER_SOURCE_WLTX, 0);
	return PWR_SRC_NULL;
}

static enum wltx_pwr_src wltx_reset_adaptor_output(void)
{
	charge_pump_chip_enable(CP_TYPE_MAIN, false);
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
	dc_set_adapter_default();
	wired_chsw_set_wired_channel(WIRED_CHANNEL_SC, WIRED_CHANNEL_CUTOFF);
	charge_set_adapter_voltage(ADAPTER_9V, RESET_ADAPTER_SOURCE_WLTX, 0);
	power_platform_buck_vbus_change_handler(ADAPTER_5V * POWER_MV_PER_V);

	return PWR_SRC_NULL;
}

static enum wltx_pwr_src wltx_set_vbus_cp_output(bool enable)
{
	int ret;
	enum wltx_pwr_src src;
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return PWR_SRC_NULL;

	if (!enable)
		return wltx_reset_adaptor_output();

	if (di->ps_need_ext_pwr)
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, true);

	src = wltx_set_adaptor_output(WLTX_SC_ADAP_VSET, WLTX_SC_ADAP_ISET);
	if (src != PWR_SRC_VBUS)
		goto fail;

	ret = charge_pump_reverse_cp_chip_init(CP_TYPE_MAIN, true);
	if (ret) {
		(void)charge_pump_reverse_chip_init(CP_TYPE_MAIN);
		hwlog_err("set_vbus_cp_output: set cp failed, reinit bp\n");
	} else {
		src = PWR_SRC_VBUS_CP;
	}

fail:
	if (di->ps_need_ext_pwr)
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);
	return src;
}

static enum wltx_pwr_src wltx_set_vbus2_output(bool enable)
{
	enum wltx_pwr_src src;

	if (!enable) {
		wltx_ic_chip_reset(WLTRX_IC_MAIN); /* power off */
		return wltx_reset_adaptor_output();
	}
	src = wltx_set_adaptor_output(WLTX_SC_ADAP_VSET, WLTX_SC_ADAP_ISET);
	if (src != PWR_SRC_VBUS)
		return PWR_SRC_NULL;
	return PWR_SRC_VBUS2;
}

static enum wltx_pwr_src wltx_set_otg_cp_output(bool enable)
{
	int ret;
	enum wltx_pwr_src src;
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return PWR_SRC_NULL;

	if (!enable)
		return wltx_set_otg_output(false);

	if (di->ps_need_ext_pwr)
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, true);

	src = wltx_set_otg_output(true);
	if (src != PWR_SRC_OTG)
		goto fail;

	ret = charge_pump_reverse_cp_chip_init(CP_TYPE_MAIN, true);
	if (ret) {
		(void)charge_pump_reverse_chip_init(CP_TYPE_MAIN);
		hwlog_err("set_otg_cp_output: set cp failed, reinit bp\n");
	} else {
		src = PWR_SRC_OTG_CP;
	}

fail:
	if (di->ps_need_ext_pwr)
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);
	return src;
}

static enum wltx_pwr_src wltx_set_rvs_sc_output(enum wltx_pwr_src src, bool enable)
{
	int ret = 0;

	if (enable) {
		charge_set_wlsin_hiz_enable(HIZ_MODE_ENABLE);
		wdcm_set_buck_channel_state(WDCM_CLIENT_TX_RVSSC, WDCM_DEV_ON);
		if (src == PWR_SRC_RVS_SC4_CP)
			ret = charge_pump_reverse_cp4_chip_init(CP_TYPE_MAIN, true);
		else
			ret = charge_pump_reverse_cp_chip_init(CP_TYPE_MAIN, true);
		wlps_control(WLTRX_IC_MAIN, WLPS_SC_SW, true);
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
		if (!ret)
			return src;
	}
	if (!enable || ret) {
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
		wlps_control(WLTRX_IC_MAIN, WLPS_SC_SW, false);
		if (src == PWR_SRC_RVS_SC4_CP)
			(void)charge_pump_reverse_cp4_chip_init(CP_TYPE_MAIN, false);
		else
			(void)charge_pump_reverse_cp_chip_init(CP_TYPE_MAIN, false);
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_EXT_PWR, false);
		wdcm_set_buck_channel_state(WDCM_CLIENT_TX_RVSSC, WDCM_DEV_OFF);
		charge_set_wlsin_hiz_enable(HIZ_MODE_DISABLE);
	}

	return PWR_SRC_NULL;
}

enum wltx_pwr_src wltx_set_pwr_src_output(bool enable, enum wltx_pwr_src src)
{
	hwlog_info("[set_pwr_src_output] src:%s\n", wltx_get_pwr_src_name(src));
	switch (src) {
	case PWR_SRC_VBUS:
		return wltx_set_vbus_output(enable);
	case PWR_SRC_OTG:
		return wltx_set_otg_output(enable);
	case PWR_SRC_5VBST:
		return wltx_set_5vbst_output(enable);
	case PWR_SRC_SPBST:
		return wltx_set_spbst_output(enable);
	case PWR_SRC_VBUS_CP:
		return wltx_set_vbus_cp_output(enable);
	case PWR_SRC_OTG_CP:
		return wltx_set_otg_cp_output(enable);
	case PWR_SRC_BP2CP:
		return wltx_set_bp2cp_output(enable);
	case PWR_SRC_VBUS2:
		return wltx_set_vbus2_output(enable);
	case PWR_SRC_5VBST_WLSIN:
		return wltx_set_5vbst_wlsin_output(enable);
	case PWR_SRC_RVS_SC_CP:
		return wltx_set_rvs_sc_output(PWR_SRC_RVS_SC_CP, enable);
	case PWR_SRC_RVS_SC4_CP:
		return wltx_set_rvs_sc_output(PWR_SRC_RVS_SC4_CP, enable);
	case PWR_SRC_NULL:
		return PWR_SRC_NULL;
	default:
		return PWR_SRC_NA;
	}
}

enum wltx_pwr_src wltx_get_cur_pwr_src(void)
{
	struct wltx_dev_info *di = wltx_get_dev_info();

	if (!di)
		return PWR_SRC_NA;

	return di->cur_pwr_src;
}