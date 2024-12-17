// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_test_wp.c
 *
 * wireless charge wp test
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

#include <linux/module.h>
#include <linux/device.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_test.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_plim.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_test_wp.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>

#define HWLOG_TAG wireless_wp
HWLOG_REGIST();

#define WP_DTS_PARA_LEN             1
#define WP_DC_CHK_INTERVAL          100 /* ms */
#define RX_SAMPLE_WORK_DELAY        500

#define WP_ASK_DC_MODE              0x4

struct wp_para {
	int cp_iout_th;
	u32 sample_delay_time;
	u32 new_test_process;
};

struct wp_dev_info {
	struct notifier_block wp_nb;
	struct notifier_block connect_nb;
	struct delayed_work dc_chk_work;
	struct delayed_work rx_sample_work;
	struct wp_para dts_para;
	int dc_type;
	u8 start_sample_type;
};

static struct wp_dev_info *g_wp_test_di;

int wp_get_sample_type(void)
{
	if (!g_wp_test_di)
		return 0;

	return g_wp_test_di->start_sample_type;
}

void wp_start_sample_iout(void)
{
	struct wp_dev_info *di = g_wp_test_di;

	if (!di)
		return;

	di->start_sample_type = WLRX_SAMPLE_WORK_PENDING;
	mod_delayed_work(system_wq, &di->rx_sample_work,
		msecs_to_jiffies(di->dts_para.sample_delay_time));
}

static void wp_send_bst_succ_msg(void)
{
	if (!wlrx_is_fac_tx(WLTRX_DRV_MAIN) ||
		(wlrx_get_charge_stage() != WLRX_STAGE_CHARGING))
		return;

	if (!wireless_send_rx_boost_succ(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI))
		hwlog_info("[%s] send cmd boost_succ ok\n", __func__);
}

static void wp_rx_sample_work(struct work_struct *work)
{
	int rx_vout = 0;
	int rx_iout = 0;
	struct wp_dev_info *di = container_of(work,
		struct wp_dev_info, rx_sample_work.work);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	di->start_sample_type = WLRX_SAMPLE_WORK_RUNNING;

	/* send confirm message to TX */
	(void)wlrx_ic_get_vout(WLTRX_IC_MAIN, &rx_vout);
	(void)wlrx_ic_get_iout(WLTRX_IC_MAIN, &rx_iout);
	wireless_send_rx_vout(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, rx_vout);
	wireless_send_rx_iout(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, rx_iout);
	hwlog_info("[%s] rx_vout = %d, rx_iout = %d\n", __func__, rx_vout, rx_iout);
	schedule_delayed_work(&di->rx_sample_work, msecs_to_jiffies(RX_SAMPLE_WORK_DELAY));
}

static void wp_send_charge_mode(struct wp_dev_info *di)
{
	int ret;

	hwlog_info("[send_charge_mode] mode=%d\n", di->dc_type);
	if (di->dc_type > WP_ASK_DC_MODE)
		di->dc_type = WP_ASK_DC_MODE;
	ret = wireless_send_charge_mode(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, (u8)di->dc_type);
	if (ret)
		hwlog_err("send_charge_mode: failed\n");
}

static void wp_dc_chk_work(struct work_struct *work)
{
	int cp_avg_iout;
	struct wp_dev_info *di = container_of(work, struct wp_dev_info, dc_chk_work.work);

	if (!di)
		return;

	if (wldc_is_stop_charging_complete())
		return;

	cp_avg_iout = wldc_cp_get_iavg();
	if (cp_avg_iout <= di->dts_para.cp_iout_th) {
		schedule_delayed_work(&di->dc_chk_work, msecs_to_jiffies(WP_DC_CHK_INTERVAL));
		return;
	}

	wp_send_charge_mode(di);

	if (di->dts_para.new_test_process)
		wlrx_plim_set_src(WLTRX_DRV_MAIN, WLRX_PLIM_SRC_WP_BUCK);

	return;
}

static void wp_start_dc_test(struct wp_dev_info *di)
{
	if (!wlrx_is_fac_tx(WLTRX_DRV_MAIN))
		return;

	hwlog_info("start_dc_test\n");
	schedule_delayed_work(&di->dc_chk_work, msecs_to_jiffies(0));
}

static int wp_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct wp_dev_info *di = container_of(nb, struct wp_dev_info, wp_nb);

	if (!di)
		return NOTIFY_OK;

	hwlog_info("notifier_call: event=%ld\n", event);
	switch (event) {
	case POWER_NE_WLC_DC_START_CHARGING:
		if (data)
			di->dc_type = *((int *)data);
		wp_start_dc_test(di);
		break;
	case POWER_NE_WLC_TX_VSET:
		wp_send_bst_succ_msg();
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int wp_connect_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct wp_dev_info *di = container_of(nb, struct wp_dev_info, connect_nb);

	if (!di)
		return NOTIFY_OK;

	hwlog_info("notifier_call: event=%ld\n", event);
	switch (event) {
	case POWER_NE_WIRELESS_CONNECT:
		di->start_sample_type = 0;
		wlrx_plim_clear_src(WLTRX_DRV_MAIN, WLRX_PLIM_SRC_WP_BUCK);
		break;
	case POWER_NE_WIRELESS_DISCONNECT:
		cancel_delayed_work_sync(&di->rx_sample_work);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int wp_parse_dts(struct device_node *np, struct wp_dev_info *di)
{
	int len;

	len = of_property_count_u32_elems(np, "wlc_wp_para");
	hwlog_info("len = %d\n", len);
	if (len <= 0)
		goto print_para;
	(void)power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"wlc_wp_para", (u32 *)&di->dts_para, len);

print_para:
	hwlog_info("[wp_para] cp_iout_th:%dma sample_delay_time:%dms new_test_process:%d\n",
		di->dts_para.cp_iout_th, di->dts_para.sample_delay_time,
		di->dts_para.new_test_process);
	return 0;
}

static void wp_init(struct device *dev)
{
	int ret;
	struct wp_dev_info *di = NULL;

	if (!dev || !dev->of_node)
		return;

	di = devm_kzalloc(dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	dev_set_drvdata(dev, di);
	ret = wp_parse_dts(dev->of_node, di);
	if (ret)
		goto parse_dts_fail;

	INIT_DELAYED_WORK(&di->dc_chk_work, wp_dc_chk_work);
	INIT_DELAYED_WORK(&di->rx_sample_work, wp_rx_sample_work);
	di->wp_nb.notifier_call = wp_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLC, &di->wp_nb);
	if (ret)
		goto nb_register_fail;

	di->connect_nb.notifier_call = wp_connect_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &di->connect_nb);
	if (ret)
		goto connect_nb_register_fail;

	g_wp_test_di = di;
	hwlog_info("init succ\n");
	return;

connect_nb_register_fail:
	(void)power_event_bnc_unregister(POWER_BNT_WLC, &di->wp_nb);
nb_register_fail:
parse_dts_fail:
	devm_kfree(dev, di);
}

static void wp_exit(struct device *dev)
{
	struct wp_dev_info *di = NULL;

	if (!dev)
		return;

	di = dev_get_drvdata(dev);
	if (!di)
		return;

	cancel_delayed_work_sync(&di->dc_chk_work);
	cancel_delayed_work_sync(&di->rx_sample_work);
	(void)power_event_bnc_unregister(POWER_BNT_WLC, &di->wp_nb);
	(void)power_event_bnc_unregister(POWER_BNT_CONNECT, &di->connect_nb);
	devm_kfree(dev, di);
}

static struct power_test_ops g_wp_ops = {
	.name = "wlc_wp",
	.init = wp_init,
	.exit = wp_exit,
};

static int __init wp_module_init(void)
{
	return power_test_ops_register(&g_wp_ops);
}

module_init(wp_module_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless wp test module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
