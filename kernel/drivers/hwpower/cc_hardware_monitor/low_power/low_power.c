// SPDX-License-Identifier: GPL-2.0
/*
 * low_power.c
 *
 * low power control driver
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

#include <chipset_common/hwpower/battery/battery_ocv.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/hardware_monitor/low_power.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/hardware_channel/charger_channel.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <huawei_platform/hwpower/wireless_charge/wireless_rx_platform.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/hardware_channel/charger_channel.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_pinctrl.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG low_power
HWLOG_REGIST();

#define LPM_VBAT_TYPE_MAX            0
#define LPM_VBAT_TYPE_MIN            1
#define LTM_MON_EXIT_TBAT            0
#define LPM_UNLOCK_VAL               0x29a
#define LPM_DBG_PARA_LEN             2
#define LPM_VBUSIN_PSSW_TYPE_TXSW    0
#define LPM_VBUSIN_PSSW_TYPE_GPIO    1

static struct low_power_dev *g_low_power_dev;

static struct low_power_dev *low_power_get_dev(void)
{
	if (!g_low_power_dev) {
		hwlog_err("g_low_power_dev is null\n");
		return NULL;
	}

	return g_low_power_dev;
}

static void low_power_send_exit_ecm_event(void)
{
	struct power_event_notify_data n_data;

	n_data.event = "BATTERY_EXIT_ECM=1";
	n_data.event_len = 18; /* length of BATTERY_EXIT_ECM=1 */
	power_event_report_uevent(&n_data);
	hwlog_info("exit_ecm=1, report uevent\n");
}

static void low_power_send_under_voltage_event(void)
{
	struct power_event_notify_data n_data;

	n_data.event = "BATTERY_EXIT_ECM=2";
	n_data.event_len = 18; /* length of BATTERY_EXIT_ECM=2 */
	power_event_report_uevent(&n_data);
	hwlog_info("exit_ecm=2, report uevent\n");
}

static bool low_power_plugin_msleep_exit(void)
{
	struct low_power_dev *l_dev = low_power_get_dev();

	if (!l_dev || l_dev->plugin_status)
		return true;

	return false;
}

static bool low_power_plugout_msleep_exit(void)
{
	struct low_power_dev *l_dev = low_power_get_dev();

	if (!l_dev || !l_dev->plugin_status)
		return true;

	return false;
}

static void low_power_restore_vbusin_sw(struct low_power_dev *l_dev, bool restore_flag)
{
	if (l_dev->vbusin_pssw_type == LPM_VBUSIN_PSSW_TYPE_TXSW) {
		wlps_control(WLTRX_IC_MAIN, WLPS_TX_SW, restore_flag);
		return;
	}

	if (l_dev->vbusin_pssw_type == LPM_VBUSIN_PSSW_TYPE_GPIO) {
		gpio_set_value(l_dev->gpio_bst_chg_sw, restore_flag ? 1 : 0);
		hwlog_info("[vbusin_sw] gpio %s now\n",
			gpio_get_value(l_dev->gpio_bst_chg_sw) ? "high" : "low");
		return;
	}
}

static void low_power_cutoff_chg_vsys_by_chg_en(
	struct low_power_dev *l_dev, bool cutoff_flag)
{
	if (cutoff_flag) {
		charger_select_channel(CHARGER_CH_WLSIN);
		wdcm_set_buck_channel_state(WDCM_CLIENT_LPM, WDCM_DEV_ON);
		power_usleep(DT_USLEEP_10MS); /* delay for buck_channel off, typically 5ms */
		boost_5v_enable(true, BOOST_CTRL_LOW_POWER);
		low_power_restore_vbusin_sw(l_dev, true);
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, true);
		(void)power_msleep(DT_MSLEEP_100MS, DT_MSLEEP_25MS, low_power_plugin_msleep_exit);
		(void)wlrx_buck_set_dev_iin(100); /* 100mA for charge ic */
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_ENABLE_CHARGER, LPM_CHG_DISABLE);
		(void)power_msleep(DT_MSLEEP_500MS, DT_MSLEEP_50MS, low_power_plugin_msleep_exit);
	} else {
		wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
		low_power_restore_vbusin_sw(l_dev, false);
		boost_5v_enable(false, BOOST_CTRL_LOW_POWER);
		power_usleep(DT_USLEEP_10MS);
		charger_select_channel(CHARGER_CH_USBIN);
		wdcm_set_buck_channel_state(WDCM_CLIENT_LPM, WDCM_DEV_OFF);
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_DCP,
			POWER_IF_SYSFS_ENABLE_CHARGER, LPM_CHG_ENABLE);
		(void)wlrx_buck_set_dev_iin(2000); /* 2000mA default max icl */
	}
}

static void low_power_boost_vsys(struct low_power_dev *l_dev, bool bst_flag)
{
	if (bst_flag) {
		buck_boost_set_enable(LPM_ENABLE, BBST_USER_LPM);
		buck_boost_set_vout(l_dev->lpm_bbst_vout[LPM_BBST_ENTER_VOL],
			BBST_USER_LPM);
		(void)power_msleep(l_dev->ecm.delay_time, 0, NULL);
		gpio_set_value(l_dev->gpio_bst_vsys_sw, 1); /* 1: enable */
	} else {
		gpio_set_value(l_dev->gpio_bst_vsys_sw, 0);
		(void)power_msleep(l_dev->ecm.delay_time, 0, NULL);
		buck_boost_set_vout(l_dev->lpm_bbst_vout[LPM_BBST_EXIT_VOL],
			BBST_USER_LPM);
		buck_boost_set_enable(LPM_DISABLE, BBST_USER_LPM);
	}
}

static void low_power_boost_vsys_chg_en(struct low_power_dev *l_dev, bool enable)
{
	if (enable) {
		low_power_cutoff_chg_vsys_by_chg_en(l_dev, true);
		low_power_boost_vsys(l_dev, true);
	} else {
		low_power_boost_vsys(l_dev, false);
		low_power_cutoff_chg_vsys_by_chg_en(l_dev, false);
	}
}

static void low_power_set_idischrg_bst_th(struct low_power_dev *l_dev)
{
	l_dev->idischrg_en_bst_th = LPM_IBAT_EN_BST_TH;
	l_dev->idischrg_dis_bst_th = LPM_IBAT_DIS_BST_TH;

	if (l_dev->boost_vsys_status) {
		l_dev->idischrg_en_bst_th += l_dev->icost_bst;
		l_dev->idischrg_dis_bst_th += l_dev->icost_bst;
	}
	hwlog_info("[set_idischrg_bst_th] en:>%dmA, dis:<%dmA\n",
		l_dev->idischrg_en_bst_th, l_dev->idischrg_dis_bst_th);
}

static void low_power_boost_vsys_q4_en(struct low_power_dev *l_dev, bool enable)
{
	if (enable) {
		charge_set_low_power_mode_enable(true);
		low_power_boost_vsys(l_dev, true);
	} else {
		low_power_boost_vsys(l_dev, false);
		charge_set_low_power_mode_enable(false);
	}
}

static void low_power_boost_vsys_enable(struct low_power_dev *l_dev, bool enable)
{
	if (enable == l_dev->boost_vsys_status)
		return;

	switch (l_dev->boost_type) {
	case LPM_BST_TYPE_CHG_EN:
		low_power_boost_vsys_chg_en(l_dev, enable);
		break;
	case LPM_BST_TYPE_Q4:
		low_power_boost_vsys_q4_en(l_dev, enable);
		break;
	default:
		break;
	}

	hwlog_info("[boost_vsys_enable] enable:%d, gpio_vsys_sw:%d\n",
		enable, gpio_get_value(l_dev->gpio_bst_vsys_sw));
	l_dev->boost_vsys_status = enable;
	low_power_set_idischrg_bst_th(l_dev);
}

static void low_power_vbus_plugged_handler(struct low_power_dev *l_dev, bool status)
{
	l_dev->plugin_status = status;
	l_dev->wltx_dping_status = false;
	if ((l_dev->ecm.trigger_status == ECM_TRIGGER_IDLE) &&
		(l_dev->ltm.monitor_type == LTM_MONITOR_IDLE))
		return;

	hwlog_info("[vbus_plugged_handler] plugin_status:%d\n", l_dev->plugin_status);
	cancel_delayed_work_sync(&l_dev->lpm_work);
	if (l_dev->plugin_status)
		low_power_boost_vsys_enable(l_dev, false);

	power_wakeup_lock(l_dev->wakelock, false);
	/* delay 50ms for plug status debounce, typically 20-30ms */
	mod_delayed_work(system_wq, &l_dev->lpm_work, msecs_to_jiffies(50));
}

static int low_power_get_vbat(unsigned int vbat_type)
{
	int cnt = 0;
	int vbat_min = 0;
	int vbat_max = 0;
	int vbat = 0;

	do {
		power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, &vbat,
			POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
		power_usleep(DT_USLEEP_20MS);
		if ((vbat_max <= 0) || (vbat_max < vbat))
			vbat_max = vbat;
		if ((vbat_min <= 0) || (vbat_min > vbat))
			vbat_min = vbat;
	} while (++cnt <= 3); /* 3 times for vbatt debouncing */

	switch (vbat_type) {
	case LPM_VBAT_TYPE_MAX:
		return vbat_max;
	case LPM_VBAT_TYPE_MIN:
		return vbat_min;
	default:
		return 0;
	}
}

static int low_power_check_idischarge(struct low_power_dev *l_dev, bool *bst_vsys)
{
	int idischrg;

	idischrg = power_platform_get_battery_current();
	hwlog_info("[check_idischarge] idischrg=%dmA\n", idischrg);
	if (idischrg > l_dev->idischrg_en_bst_th) {
		*bst_vsys = true;
		return 0;
	} else if (idischrg < l_dev->idischrg_dis_bst_th) {
		*bst_vsys = false;
		return 0;
	}

	return -EINVAL;
}

static int low_power_get_shutdown_th(struct low_power_dev *l_dev)
{
	int ret;
	int voltage = 0;

	ret = battery_ocv_get_ocv_by_cap(BATTERY_OCV_SHUTDOWN_CAPACITY, &voltage);
	if (ret)
		return l_dev->ecm.vbat_shutdown_th;

	return voltage;
}

static void low_power_ecm_status_check(struct low_power_dev *l_dev)
{
	int vbat;
	int shutdown_th;

	if (!l_dev->support_ecm || (l_dev->ecm.trigger_status == ECM_TRIGGER_IDLE))
		return;

	shutdown_th = low_power_get_shutdown_th(l_dev);
	vbat = low_power_get_vbat(LPM_VBAT_TYPE_MAX);
	hwlog_info("[status check]: shutdown_vol=%d vbat=%d gsm_vol=%u gsm_status=%u\n",
		shutdown_th, vbat, l_dev->ecm.vbat_gsm, l_dev->ecm.gsm_status);

	if ((vbat <= shutdown_th) || ((vbat <= l_dev->ecm.vbat_gsm)
		&& (l_dev->ecm.gsm_status != ECM_GSM_IDLE))) {
		l_dev->ecm.bst_vsys = false;
		l_dev->ecm.event_type = LPM_EVENT_UNDER_VOLT;
		return;
	}

	if (l_dev->plugin_status || l_dev->wltx_dping_status) {
		l_dev->ecm.bst_vsys = false;
		l_dev->ecm.event_type = LPM_EVENT_EXIT_ECM;
		return;
	}

	vbat = low_power_get_vbat(LPM_VBAT_TYPE_MIN);
	if (vbat > l_dev->ecm.vbat_bst_th) {
		l_dev->ecm.bst_vsys = false;
		return;
	}

	if (!low_power_check_idischarge(l_dev, &l_dev->ecm.bst_vsys))
		return;

	l_dev->ecm.bst_vsys = true;
}

static void low_power_ltm_status_check(struct low_power_dev *l_dev)
{
	int tbatt = 0;

	if (!l_dev->support_ltm || (l_dev->ltm.monitor_type == LTM_MONITOR_IDLE))
		return;

	/* if support ltm only, under voltage should also be checked */
	if (l_dev->ui_capacity > l_dev->ltm.soc_th) {
		l_dev->ltm.bst_vsys = false;
		l_dev->ltm.monitor_type = LTM_MONITOR_EXIT;
		return;
	}

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	if (l_dev->plugin_status || l_dev->wltx_dping_status ||
		(tbatt > l_dev->ltm.temp_th)) {
		l_dev->ltm.bst_vsys = false;
		return;
	}

	if (!low_power_check_idischarge(l_dev, &l_dev->ltm.bst_vsys))
		return;

	l_dev->ltm.bst_vsys = true;
}

static void low_power_lpm_status_process(struct low_power_dev *l_dev)
{
	if (l_dev->ecm.bst_vsys || l_dev->ltm.bst_vsys)
		low_power_boost_vsys_enable(l_dev, true);
	else
		low_power_boost_vsys_enable(l_dev, false);

	if (l_dev->ecm.event_type == LPM_EVENT_UNDER_VOLT) {
		l_dev->ecm.trigger_status = ECM_TRIGGER_IDLE;
		low_power_send_under_voltage_event();
		power_event_bnc_notify(POWER_BNT_COUL, POWER_NE_UI_LOW_POWER_FORCE_ZERO,
			&l_dev->ecm.trigger_status);
	}

	if (l_dev->ecm.event_type == LPM_EVENT_EXIT_ECM) {
		l_dev->ecm.trigger_status = ECM_TRIGGER_IDLE;
		low_power_send_exit_ecm_event();
	}

	power_event_bnc_notify(POWER_BNT_LOW_POWER,
		POWER_NE_BAT_ECM_TRIGGER_STATUS, &l_dev->ecm.trigger_status);
}

static bool low_power_lpm_work_check_exit(struct low_power_dev *l_dev)
{
	if (l_dev->ltm.monitor_type == LTM_MONITOR_EXIT) {
		l_dev->ltm.bst_vsys = false;
		l_dev->ltm.monitor_type = LTM_MONITOR_IDLE;
	}

	if (l_dev->ecm.trigger_status == ECM_TRIGGER_IDLE) {
		l_dev->ecm.bst_vsys = false;
		l_dev->ecm.event_type = LPM_EVENT_DEFAULT;
	}

	if (l_dev->ltm.monitor_type != LTM_MONITOR_IDLE)
		return false;
	if (l_dev->ecm.trigger_status != ECM_TRIGGER_IDLE)
		return false;
	return true;
}

static void low_power_set_work_interval(struct low_power_dev *l_dev)
{
	if (l_dev->support_ecm && (l_dev->ecm.trigger_status != ECM_TRIGGER_IDLE))
		l_dev->work_interval = LPM_ECM_WORK_INTERVAL;
	else
		l_dev->work_interval = LPM_DFLT_WORK_INTERVAL;
}

static void low_power_lpm_work(struct work_struct *work)
{
	struct low_power_dev *l_dev = container_of(work, struct low_power_dev, lpm_work.work);

	if (!l_dev)
		return;

	low_power_ecm_status_check(l_dev);
	low_power_ltm_status_check(l_dev);

	hwlog_info("[lpm_work] ecm: trigger_status=%u bst=%d, ltm: monitor_type=%d bst=%d\n",
		l_dev->ecm.trigger_status, l_dev->ecm.bst_vsys, l_dev->ltm.monitor_type, l_dev->ltm.bst_vsys);

	low_power_lpm_status_process(l_dev);

	if (low_power_lpm_work_check_exit(l_dev))
		goto exit;

	low_power_set_work_interval(l_dev);
	schedule_delayed_work(&l_dev->lpm_work, msecs_to_jiffies(l_dev->work_interval));

exit:
	power_wakeup_unlock(l_dev->wakelock, false);
}

static void low_power_wltx_dping_handler(struct low_power_dev *l_dev)
{
	if ((l_dev->ecm.trigger_status == ECM_TRIGGER_IDLE) &&
		(l_dev->ltm.monitor_type == LTM_MONITOR_IDLE))
		return;
	if (l_dev->boost_type != LPM_BST_TYPE_CHG_EN)
		return;

	hwlog_info("wltx_dping_handler\n");
	cancel_delayed_work_sync(&l_dev->lpm_work);
	l_dev->wltx_dping_status = true;
	low_power_boost_vsys_enable(l_dev, false);
	power_wakeup_lock(l_dev->wakelock, false);
	mod_delayed_work(system_wq, &l_dev->lpm_work, msecs_to_jiffies(0));
}

static int low_power_ui_cap_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	int tbatt = 0;
	struct low_power_dev *l_dev = low_power_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_BAT_UI_CAP_CHAGNED:
		l_dev->ui_capacity = *(int *)data;
		l_dev->wltx_dping_status = false;
		bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
		if (l_dev->support_ltm && (tbatt < LTM_MON_EXIT_TBAT) &&
			(l_dev->ui_capacity <= l_dev->ltm.soc_th) &&
			(l_dev->ltm.monitor_type == LTM_MONITOR_IDLE)) {
			l_dev->ltm.monitor_type = LTM_MONITOR_WORKING;
			power_wakeup_lock(l_dev->wakelock, false);
			mod_delayed_work(system_wq, &l_dev->lpm_work, msecs_to_jiffies(0));
			hwlog_info("[ui_cap_notifier_call] triggered ltm monitor\n");
		}
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int low_power_plugged_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct low_power_dev *l_dev = low_power_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_USB_DISCONNECT:
		low_power_vbus_plugged_handler(l_dev, false);
		break;
	case POWER_NE_USB_CONNECT:
		low_power_vbus_plugged_handler(l_dev, true);
		break;
	case POWER_NE_WIRELESS_DISCONNECT:
		low_power_vbus_plugged_handler(l_dev, false);
		break;
	case POWER_NE_WIRELESS_CONNECT:
		low_power_vbus_plugged_handler(l_dev, true);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int low_power_wltx_dping_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct low_power_dev *l_dev = low_power_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_WLTX_RCV_DPING:
		low_power_wltx_dping_handler(l_dev);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void low_power_fb_handler(struct low_power_dev *l_dev)
{
	if ((l_dev->ecm.trigger_status == ECM_TRIGGER_IDLE) &&
		(l_dev->ltm.monitor_type == LTM_MONITOR_IDLE))
		return;

	cancel_delayed_work_sync(&l_dev->lpm_work);
	power_wakeup_lock(l_dev->wakelock, false);
	mod_delayed_work(system_wq, &l_dev->lpm_work, msecs_to_jiffies(LPM_FB_WORK_DELAY));
}

static int low_power_fb_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct low_power_dev *l_dev = low_power_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_PANEL_BLANK:
		l_dev->screen_state = LPM_SCREEN_OFF;
		hwlog_info("[fb_notifier_call] fb screen off\n");
		low_power_fb_handler(l_dev);
		break;
	case POWER_NE_PANEL_UNBLANK:
		l_dev->screen_state = LPM_SCREEN_ON;
		hwlog_info("[fb_notifier_call] fb screen on\n");
		low_power_fb_handler(l_dev);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
static ssize_t low_power_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t low_power_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info low_power_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(low_power, 0440, LPM_SYSFS_SUPPORT_ECM, support_ecm),
	power_sysfs_attr_wo(low_power, 0220, LPM_SYSFS_TRIGGER_ECM, trigger_ecm),
	power_sysfs_attr_wo(low_power, 0220, LPM_SYSFS_GSM_ECM, gsm_ecm),
	power_sysfs_attr_wo(low_power, 0220, LPM_SYSFS_BOOST_ECM, boost_ecm),
};

#define LPM_SYSFS_ATTRS_SIZE  ARRAY_SIZE(low_power_sysfs_field_tbl)

static struct attribute *low_power_sysfs_attrs[LPM_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group low_power_sysfs_attr_group = {
	.attrs = low_power_sysfs_attrs,
};

static ssize_t low_power_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct low_power_dev *l_dev = low_power_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		low_power_sysfs_field_tbl, LPM_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case LPM_SYSFS_SUPPORT_ECM:
		return scnprintf(buf, PAGE_SIZE, "%u\n", l_dev->support_ecm);
	default:
		break;
	}

	return 0;
}

static ssize_t low_power_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	struct power_sysfs_attr_info *info = NULL;
	struct low_power_dev *l_dev = low_power_get_dev();

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		low_power_sysfs_field_tbl, LPM_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case LPM_SYSFS_TRIGGER_ECM:
		if (!l_dev->support_ecm || (kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < ECM_TRIGGER_BEGIN) || (val >= ECM_TRIGGER_END))
			return -EINVAL;
		l_dev->ecm.trigger_status = val;
		hwlog_info("set ecm trigger_status:%u\n", l_dev->ecm.trigger_status);
		cancel_delayed_work_sync(&l_dev->lpm_work);
		power_wakeup_lock(l_dev->wakelock, false);
		mod_delayed_work(system_wq, &l_dev->lpm_work, msecs_to_jiffies(0));
		break;
	case LPM_SYSFS_GSM_ECM:
		if (!l_dev->support_ecm || (kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < ECM_GSM_BEGIN) || (val >= ECM_GSM_END))
			return -EINVAL;
		l_dev->ecm.gsm_status = val;
		hwlog_info("set ecm gsm_status:%u\n", l_dev->ecm.gsm_status);
		break;
	case LPM_SYSFS_BOOST_ECM:
		if (!l_dev->support_ecm || (kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < ECM_BOOST_BEGIN) || (val >= ECM_BOOST_END))
			return -EINVAL;

		if (val == ECM_BOOST_WORKING)
			low_power_boost_vsys_chg_en(l_dev, true);
		break;
	default:
		break;
	}

	return count;
}

static struct device *low_power_sysfs_create_group(void)
{
	power_sysfs_init_attrs(low_power_sysfs_attrs,
		low_power_sysfs_field_tbl, LPM_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "low_power",
		&low_power_sysfs_attr_group);
}

static void low_power_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &low_power_sysfs_attr_group);
}
#else
static inline struct device *low_power_sysfs_create_group(void)
{
	return NULL;
}

static inline void low_power_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void low_power_ecm_para_init(struct device_node *np, struct low_power_dev *l_dev)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_ecm", &l_dev->support_ecm, 0))
		return;

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ecm_vbat_bst", &l_dev->ecm.vbat_bst_th, 3100); /* 3100: default vbat bst th */
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ecm_vbat_shutdown", &l_dev->ecm.vbat_shutdown_th, 3050); /* 3050: default vbat_shutdown_th */
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ecm_vbat_gsm", &l_dev->ecm.vbat_gsm, 3200); /* 3200: default vbat_gsm */
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ecm_delay_time", &l_dev->ecm.delay_time, 0); /* 0ms: default delay_time */
}

static void low_power_ltm_para_init(struct device_node *np, struct low_power_dev *l_dev)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_ltm", &l_dev->support_ltm, 0))
		return;

	(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
		"ltm_temp", &l_dev->ltm.temp_th, -15); /* -15: default ltm temp_th */
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ltm_soc", &l_dev->ltm.soc_th, 10); /* 10: default ltm soc_th */
}

static void low_power_para_init(struct device_node *np,
	struct device *dev, struct low_power_dev *l_dev)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"boost_type", &l_dev->boost_type, LPM_BST_TYPE_CHG_EN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vbusin_pssw_type", &l_dev->vbusin_pssw_type, LPM_VBUSIN_PSSW_TYPE_TXSW);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"icost_bst", &l_dev->icost_bst, 150); /* 150mA cost by boost mode */
	low_power_set_idischrg_bst_th(l_dev);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pinctrl_len", &l_dev->pinctrl_len, 1); /* 1: default length */
	(void)power_pinctrl_config(dev, "pinctrl-names", l_dev->pinctrl_len);
	(void)power_gpio_config_output(np,
		"gpio_bst_vsys_sw", "gpio_bst_vsys_sw", &l_dev->gpio_bst_vsys_sw, 0);
	if (l_dev->vbusin_pssw_type == LPM_VBUSIN_PSSW_TYPE_GPIO)
		(void)power_gpio_config_output(np, "gpio_bst_chg_sw",
			"gpio_bst_chg_sw", &l_dev->gpio_bst_chg_sw, 0);

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"lpm_bbst_vout", l_dev->lpm_bbst_vout, LPM_BBST_VOL_END))
		hwlog_err("lpm_bbst_vout parse failed\n");
	hwlog_info("lpm_bbst_vout %u %u\n",
		l_dev->lpm_bbst_vout[LPM_BBST_ENTER_VOL],
		l_dev->lpm_bbst_vout[LPM_BBST_EXIT_VOL]);

	low_power_ecm_para_init(np, l_dev);
	low_power_ltm_para_init(np, l_dev);
}

static ssize_t low_power_dbg_boost_vsys_show(void *dev_data, char *buf, size_t size)
{
	struct low_power_dev *l_dev = dev_data;

	if (!l_dev)
		return 0;

	return scnprintf(buf, size, "boost_vsys_status:%d\n", l_dev->boost_vsys_status);
}

static ssize_t low_power_dbg_boost_vsys_store(void *dev_data, const char *buf, size_t size)
{
	int val = 0;
	int unlock_val = 0;
	struct low_power_dev *l_dev = dev_data;

	if (!l_dev)
		return 0;

	if ((sscanf(buf, "%d %d", &unlock_val, &val) != LPM_DBG_PARA_LEN) ||
		(unlock_val != LPM_UNLOCK_VAL))
		return -EINVAL;

	low_power_boost_vsys_enable(l_dev, val);
	return size;
}

static int low_power_nb_register(struct low_power_dev *l_dev)
{
	int ret;

	if (!l_dev->support_ecm && !l_dev->support_ltm) {
		hwlog_err("nb_register: not support ecm and ltm\n");
		return -EINVAL;
	}

	l_dev->ui_cap_nb.notifier_call = low_power_ui_cap_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_BAT_UI_CAPACITY, &l_dev->ui_cap_nb);
	if (ret < 0) {
		hwlog_err("nb_register: register bat_ui_capa notifier failed\n");
		return -EINVAL;
	}
	l_dev->plugged_nb.notifier_call = low_power_plugged_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &l_dev->plugged_nb);
	if (ret < 0) {
		hwlog_err("nb_register: register connect notifier failed\n");
		goto unregister_bat_ui_cap;
	}

	l_dev->wltx_dping_nb.notifier_call = low_power_wltx_dping_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLTX, &l_dev->wltx_dping_nb);
	if (ret < 0) {
		hwlog_err("nb_register: register wltx_dping notifier failed\n");
		goto unregister_connect;
	}
	l_dev->fb_nb.notifier_call = low_power_fb_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_PANEL_EVENT, &l_dev->fb_nb);
	if (ret < 0) {
		hwlog_err("nb_register: register wltx_dping notifier failed\n");
		goto unregister_wltx_dping;
	}

	INIT_DELAYED_WORK(&l_dev->lpm_work, low_power_lpm_work);
	power_dbg_ops_register("low_power", "boost_vsys", (void *)l_dev,
		low_power_dbg_boost_vsys_show, low_power_dbg_boost_vsys_store);

	return 0;

unregister_wltx_dping:
	power_event_bnc_unregister(POWER_BNT_WLTX, &l_dev->wltx_dping_nb);
unregister_connect:
	power_event_bnc_unregister(POWER_BNT_CONNECT, &l_dev->plugged_nb);
unregister_bat_ui_cap:
	power_event_bnc_unregister(POWER_BNT_BAT_UI_CAPACITY, &l_dev->ui_cap_nb);
	return -EINVAL;
}

static int low_power_probe(struct platform_device *pdev)
{
	struct low_power_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	np = pdev->dev.of_node;
	low_power_para_init(np, &pdev->dev, l_dev);
	if (low_power_nb_register(l_dev)) {
		kfree(l_dev);
		return -EINVAL;
	}
	l_dev->dev = low_power_sysfs_create_group();
	l_dev->wakelock = power_wakeup_source_register(&pdev->dev, "lpm_wakelock");
	g_low_power_dev = l_dev;
	platform_set_drvdata(pdev, l_dev);

	return 0;
}

static int low_power_remove(struct platform_device *pdev)
{
	struct low_power_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	if (l_dev->support_ecm || l_dev->support_ltm) {
		cancel_delayed_work_sync(&l_dev->lpm_work);
		power_event_bnc_unregister(POWER_BNT_BAT_UI_CAPACITY, &l_dev->ui_cap_nb);
		power_event_bnc_unregister(POWER_BNT_CONNECT, &l_dev->plugged_nb);
		power_event_bnc_unregister(POWER_BNT_WLTX, &l_dev->wltx_dping_nb);
		power_event_bnc_unregister(POWER_BNT_PANEL_EVENT, &l_dev->fb_nb);
	}
	low_power_sysfs_remove_group(l_dev->dev);
	power_wakeup_source_unregister(l_dev->wakelock);
	l_dev->wakelock = NULL;
	kfree(l_dev);
	platform_set_drvdata(pdev, NULL);
	g_low_power_dev = NULL;

	return 0;
}

static void low_power_shutdown(struct platform_device *pdev)
{
	struct low_power_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return;

	if (l_dev->support_ecm || l_dev->support_ltm) {
		power_event_bnc_unregister(POWER_BNT_BAT_UI_CAPACITY, &l_dev->ui_cap_nb);
		power_event_bnc_unregister(POWER_BNT_CONNECT, &l_dev->plugged_nb);
		power_event_bnc_unregister(POWER_BNT_WLTX, &l_dev->wltx_dping_nb);
		power_event_bnc_unregister(POWER_BNT_PANEL_EVENT, &l_dev->fb_nb);
	}
	cancel_delayed_work_sync(&l_dev->lpm_work);
	low_power_boost_vsys_enable(l_dev, false);
	kfree(l_dev);
	g_low_power_dev = NULL;
}

static const struct of_device_id low_power_match_table[] = {
	{
		.compatible = "huawei,low_power",
		.data = NULL,
	},
	{},
};

static struct platform_driver low_power_driver = {
	.probe = low_power_probe,
	.remove = low_power_remove,
	.shutdown = low_power_shutdown,
	.driver = {
		.name = "huawei,low_power",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(low_power_match_table),
	},
};

static int __init low_power_init(void)
{
	return platform_driver_register(&low_power_driver);
}

static void __exit low_power_exit(void)
{
	platform_driver_unregister(&low_power_driver);
}

late_initcall_sync(low_power_init);
module_exit(low_power_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("low power control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
