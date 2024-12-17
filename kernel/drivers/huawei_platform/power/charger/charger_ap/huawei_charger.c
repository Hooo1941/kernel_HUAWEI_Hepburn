/*
 * drivers/power/huawei_charger.c
 *
 * huawei charger driver
 *
 * Copyright (C) 2012-2015 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/usb/otg.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/power_supply.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/log/hwlog_kernel.h>
#include <huawei_platform/usb/switch/usbswitch_common.h>
#include <linux/delay.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_state.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_icon.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>
#include <chipset_common/hwpower/hardware_monitor/ship_mode.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#ifdef CONFIG_TCPC_CLASS
#include <huawei_platform/usb/pd/richtek/tcpm.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#include <linux/raid/pq.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/charging_core.h>
#ifdef CONFIG_SWITCH_FSA9685
#include <huawei_platform/usb/switch/switch_fsa9685.h>
#endif
#include <chipset_common/hwpower/hardware_monitor/uscp.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/wireless_charge/wireless_buck_ictrl.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#ifdef CONFIG_HUAWEI_TYPEC
#include <huawei_platform/usb/hw_typec_dev.h>
#endif
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <huawei_platform/power/series_batt_charger.h>

#ifdef CONFIG_HUAWEI_YCABLE
#include <huawei_platform/usb/hw_ycable.h>
#endif
#include <huawei_platform/usb/hw_pogopin.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/adapter/adapter_detect.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_platform_interface.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_sysfs.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/hvdcp_charge/hvdcp_charge.h>
#include <chipset_common/hwpower/charger/charger_detection.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <securec.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG huawei_charger
HWLOG_REGIST();

#define CHG_WAIT_PD_TIME                    6
#define CHGDIEID_NV_LEN                     9
#define CHGDIEID_LEN                        8
#define INCR_TERM_VOL_FORBIDDEN             0x2
#define INCR_TERM_VOL_ENABLE                0x1
#define INCR_TERM_VOL_DISABLE               0x0
#define CHARGE_INCRESE_TERM_VOLT            16
#define CHARGE_INCRESE_DETLA_VOLT           30
#define CHARGE_COUL_FAULT_VOLT              60
#define CHARGE_INCRESE_MAX_CYCLE            200
#define SYSFS_CHARGER_CVCAL_CLEAR           1
#define CHARGE_FULL_DMD_BUFF_SIZE           256

struct charge_term_volt_nvdata {
	int term_volt;
	int flag;
};

static struct wakeup_source *charge_lock;
static struct wakeup_source *stop_charge_lock;
static struct wakeup_source *uscp_plugout_lock;
static struct charge_device_info *g_di;
static struct mutex charge_wakelock_flag_lock;
static struct work_struct resume_wakelock_work;
static bool charge_done_sleep_dts;
static bool weak_source_sleep_dts;
static int pd_charge_flag;
static bool term_err_chrg_en_flag = TRUE;
static int term_err_cnt;
static int batt_ifull;
static int clear_iin_avg_flag;
static int last_vset;
static u32 g_sw_recharge_dts;
struct charge_init_data g_init_crit;

#ifdef CONFIG_TCPC_CLASS
static u32 charger_pd_support;
static u32 charger_pd_cur_trans_ratio;
#endif
static bool charger_type_ever_notify;
#ifdef CONFIG_DIRECT_CHARGER
#define PD_TO_SCP_MAX_COUNT                 5
static int try_pd_to_scp_counter;
#endif
#define PD_TO_FCP_MAX_COUNT                 5
static int try_pd_to_fcp_counter;
static time64_t boot_time;
static bool chg_wait_pd_init;
static unsigned int g_recharge_count;

static void charge_select_charging_current(struct charge_device_info *di);

#define VBUS_REPORT_NUM                     4
#define WORK_DELAY_5000MS                   5000

static int vbus_flag;
static int nonfcp_vbus_higher_count;
static int fcp_vbus_lower_count;
static int pd_vbus_abnormal_cnt;

static int ico_enable;
static int nonstand_detect_times;
static bool charger_type_update = FALSE;
static struct kobject *g_sysfs_notify_event;
struct completion emark_detect_comp;

void huawei_charge_set_init_crit(void)
{
	g_init_crit.vbus = ADAPTER_5V;
	g_init_crit.charger_type = charge_get_charger_type();
	(void)charge_init_chip(&g_init_crit);
}

void huawei_charge_vbat_ocp_handler(unsigned int *charge_fault)
{
	if (!g_di)
		return;

	if (!g_di->core_data->warm_triggered) {
		hwlog_err("vbat_ovp happend\n");
		*charge_fault = POWER_NE_CHG_FAULT_NON;
		if (g_di->core_data->vterm == g_di->core_data->vterm_bsoh)
			charge_state_dsm_report(ERROR_CHARGE_VBAT_OVP);
	}
}

bool get_term_err_chrg_en_flag(void)
{
	return term_err_chrg_en_flag;
}

void huawei_charger_input_current_handler(void)
{
	unsigned int input_current = 0;
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("input_current_handler di is null\n");
		return;
	}

	charge_select_charging_current(di);
	charge_set_input_current(di->input_current);
}

void huawei_charger_set_input_current(unsigned int input_current)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("set_input_current di is null\n");
		return;
	}

	di->input_current = input_current;
}

unsigned int huawei_charger_get_input_current(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("get_input_current di is null\n");
		return -EINVAL;
	}

	return di->input_current;
}

unsigned int huawei_charger_get_charge_current(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("get_charge_current di is null\n");
		return -EINVAL;
	}

	return di->charge_current;
}

bool huawei_charger_get_dc_enable_hiz_status(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("get_dc_enable_hiz_status di is null\n");
		return -EINVAL;
	}

	return di->is_dc_enable_hiz;
}

unsigned int huawei_charger_get_hota_iin_limit(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("get_hota_iin_limit di is null\n");
		return -EINVAL;
	}

	return di->hota_iin_limit;
}

unsigned int huawei_charger_get_startup_iin_limit(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("get_startup_iin_limit di is null\n");
		return -EINVAL;
	}

	return di->startup_iin_limit;
}

bool get_sysfs_wdt_disable_flag(void)
{
	return charge_sysfs_get_sysfs_value(CHARGE_SYSFS_WATCHDOG_DISABLE);
}

struct charge_device_info *huawei_charge_get_di(void)
{
	if (!g_di) {
		hwlog_err("g_di is null\n");
		return NULL;
	}

	return g_di;
}

#ifdef CONFIG_WIRELESS_CHARGER
void wireless_charge_wired_vbus_handler(void)
{
	static bool wired_vbus_flag = true;
	struct charge_device_info *di = huawei_charge_get_di();

	if (!di)
		return;

	if (charge_get_charger_type() == CHARGER_REMOVED) {
		if (wired_vbus_flag == true)
			wireless_charge_wired_vbus_disconnect_handler();
		wired_vbus_flag = false;
	} else if (charge_get_charger_type() != CHARGER_TYPE_WIRELESS) {
		wireless_charge_wired_vbus_connect_handler();
		wired_vbus_flag = true;
	}
}
#endif

void emark_detect_complete(void)
{
	complete(&emark_detect_comp);
}

int huawei_charger_get_iin_thermal_charge_type(void)
{
	if (g_init_crit.charger_type == CHARGER_TYPE_WIRELESS)
		return (g_init_crit.vbus == ADAPTER_5V) ?
			CHARGE_SYSFS_IIN_THERMAL_WLCURRENT_5V : CHARGE_SYSFS_IIN_THERMAL_WLCURRENT_9V;

	return (g_init_crit.vbus == ADAPTER_5V) ?
		CHARGE_SYSFS_IIN_THERMAL_WCURRENT_5V : CHARGE_SYSFS_IIN_THERMAL_WCURRENT_9V;
}

unsigned int charge_get_bsoh_vterm(void)
{
	int vterm_max;
	struct charge_device_info *di = g_di;

	if (!di || !di->core_data) {
		hwlog_err("di or core_data is null\n");
		return VTERM_MAX_DEFAULT_MV;
	}

	if (di->core_data->vterm_bsoh > 0)
		return di->core_data->vterm_bsoh;

	vterm_max = coul_drv_battery_vbat_max();
	if (vterm_max <= 0) {
		hwlog_err("get vterm_max=%d fail\n", vterm_max);
		return VTERM_MAX_DEFAULT_MV;
	}

	return vterm_max;
}

void huawei_charge_into_sleep(void)
{
	unsigned int charger_type = charge_get_charger_type();

	if (!weak_source_sleep_dts)
		return;

	if ((charger_type == CHARGER_TYPE_STANDARD) ||
		(charger_type == CHARGER_TYPE_NON_STANDARD) ||
		(charger_type == CHARGER_TYPE_FCP) ||
		(charger_type == CHARGER_TYPE_PD)) {
		power_event_bnc_notify(POWER_BNT_CHG, POWER_NE_CHG_WAKE_UNLOCK, NULL);
		mutex_lock(&charge_wakelock_flag_lock);
		charge_set_wakelock_flag(CHARGE_WAKELOCK_NO_NEED);
		power_wakeup_unlock(charge_lock, false);
		mutex_unlock(&charge_wakelock_flag_lock);
		hwlog_info("charge wake unlock while weak source happened\n");
	}
}

static void charge_start_charging(struct charge_device_info *di);
static void charge_stop_charging(struct charge_device_info *di);
static void charger_type_handler(unsigned long type, void *data);

static void charge_handle_start_sink_event(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	if (!power_cmdline_is_factory_mode())
		set_charger_disable_flags(CHARGER_CLEAR_DISABLE_FLAGS, CHARGER_SYS_NODE);
	power_event_notify_sysfs(g_sysfs_notify_event, NULL, "charger_online");
	power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_CONNECT, NULL);
	power_icon_notify(ICON_TYPE_NORMAL);
	if (wltx_get_vbus_change_type() == WLTX_VBUS_CHANGED_BY_PLUG) {
		wireless_tx_cancel_work(PWR_SW_BY_VBUS_ON);
		wireless_tx_restart_check(PWR_SW_BY_VBUS_ON);
	}
#ifdef CONFIG_TCPC_CLASS
	mutex_lock(&di->event_type_lock);
	charger_type_handler(CHARGER_TYPE_SDP, NULL);
	mutex_unlock(&di->event_type_lock);
#endif
	charge_start_charging(di);
}

static void charge_handle_start_wireless_event(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	power_event_notify_sysfs(g_sysfs_notify_event, NULL, "charger_online");
	power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_WIRELESS_CONNECT, NULL);
	power_icon_notify(ICON_TYPE_WIRELESS_NORMAL);
	mutex_lock(&di->event_type_lock);
	charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
	charge_set_charger_type(CHARGER_TYPE_WIRELESS);
	hwlog_info("%s start display wireless charge\n", __func__);
	charge_send_uevent(NO_EVENT);
	mutex_unlock(&di->event_type_lock);
	charge_start_charging(di);
}

static void charge_handle_stop_sink_event(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	power_event_notify_sysfs(g_sysfs_notify_event, NULL, "charger_online");
	power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_DISCONNECT, NULL);
	power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_WIRELESS_DISCONNECT, NULL);
	power_event_bnc_notify(POWER_BNT_CHG, POWER_NE_CHG_PRE_STOP_CHARGING, NULL);
	mutex_lock(&di->event_type_lock);
	charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
	charge_set_charger_type(CHARGER_REMOVED);
	charge_send_uevent(NO_EVENT);
	charge_stop_charging(di);
	mutex_unlock(&di->event_type_lock);

#ifdef CONFIG_WIRELESS_CHARGER
	wireless_charge_wired_vbus_disconnect_handler();
#endif
	if (wltx_get_vbus_change_type() == WLTX_VBUS_CHANGED_BY_PLUG) {
		wireless_tx_cancel_work(PWR_SW_BY_VBUS_OFF);
		wireless_tx_restart_check(PWR_SW_BY_VBUS_OFF);
	}
}

static void charge_handle_stop_wireless_event(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	power_event_notify_sysfs(g_sysfs_notify_event, NULL, "charger_online");
	power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_WIRELESS_DISCONNECT, NULL);
	mutex_lock(&di->event_type_lock);
	charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
	charge_set_charger_type(CHARGER_REMOVED);
	charge_send_uevent(NO_EVENT);
	charge_stop_charging(di);
	/* Bugfix for Hi65xx: DRP lost when a cable plugin without adapter */
#ifdef CONFIG_TCPC_CLASS
	pd_pdm_enable_drp();
#endif
	mutex_unlock(&di->event_type_lock);
}

static void charge_handle_start_source_event(void)
{
	power_wakeup_unlock(charge_lock, false);
	charge_otg_mode_enable(CHARGE_OTG_ENABLE, VBUS_CH_USER_WIRED_OTG);
}

static void charge_handle_stop_source_event(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	charge_stop_charging(di);
}

int charge_get_done_type(void)
{
	struct charge_device_info *di = g_di;

	if (!di)
		return CHARGE_DONE_NON;
	return charge_sysfs_get_sysfs_value(CHARGE_SYSFS_CHARGE_DONE_STATUS);
}

void charge_reset_hiz_state(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	mutex_lock(&di->mutex_hiz);
	di->hiz_ref = 0;
	hwlog_info("[%s] charger disconnect, clear hiz_ref\n", __func__);
	mutex_unlock(&di->mutex_hiz);
}

static void wireless_tx_limit_input_current(struct charge_device_info *di)
{
	int iin;
	int ilim;

	switch (charge_get_charger_type()) {
	case CHARGER_TYPE_FCP:
	case CHARGER_TYPE_STANDARD:
	case CHARGER_TYPE_PD:
		iin = di->core_data->iin_ac;
		break;
	case CHARGER_TYPE_NON_STANDARD:
		iin = di->core_data->iin_nonstd;
		break;
	case CHARGER_TYPE_BC_USB:
		iin = di->core_data->iin_bc_usb;
		break;
	default:
		return;
	}

	ilim = wltx_get_tx_ilimit((int)charge_get_charger_type(), iin);
	if (ilim <= 0)
		return;

	di->input_current = min(ilim, (int)di->input_current);
}

static int charge_get_next_stage_iin(struct charge_device_info *di, int iin)
{
	int i;

	for (i = 0; i < di->iin_regl_lut.total_stage; i++) {
		if (iin < di->iin_regl_lut.iin_regl_para[i])
			break;
	}

	if (i == di->iin_regl_lut.total_stage) {
		hwlog_info("input out of range\n");
		return di->iin_regl_lut.iin_regl_para[i - 1];
	}

	return di->iin_regl_lut.iin_regl_para[i];
}

static int charge_iin_regulation(struct charge_device_info *di)
{
	int ret = 0;
	int iin_next;

	mutex_lock(&di->iin_regl_lock);

	hwlog_info("iin_now=%d, iin_target=%d\n", di->iin_now, di->iin_target);

	if (di->iin_target <= di->iin_now) {
		ret = charge_set_dev_iin(di->iin_target);
		if (ret) {
			hwlog_err("iin regl %d fail\n", di->iin_target);
			goto out_func;
		}
		di->iin_now = di->iin_target;
	} else {
		iin_next = charge_get_next_stage_iin(di, di->iin_now);
		iin_next = (iin_next < di->iin_target) ?
			iin_next : di->iin_target;

		hwlog_info("iin_next=%d\n", iin_next);

		if ((iin_next <= di->iin_now) ||
			(iin_next >= di->iin_target)) {
			ret = charge_set_dev_iin(di->iin_target);
			if (ret) {
				hwlog_err("iin regl %d fail\n", di->iin_target);
				goto out_func;
			}
			di->iin_now = di->iin_target;
		}

		if ((iin_next > di->iin_now) && (iin_next < di->iin_target)) {
			ret = charge_set_dev_iin(iin_next);
			if (ret) {
				hwlog_err("iin regl %d fail\n", iin_next);
				goto out_func;
			}
			di->iin_now = iin_next;
			schedule_delayed_work(&di->iin_regl_work,
				msecs_to_jiffies(di->iin_regl_interval));
			hwlog_info("schedule iin_regl_work\n");
		}
	}

out_func:
	mutex_unlock(&di->iin_regl_lock);
	return ret;
}

static void charge_iin_regl_work(struct work_struct *work)
{
	struct charge_device_info *di = NULL;
	int ret;

	di = container_of(work, struct charge_device_info, iin_regl_work.work);
	if (!di) {
		hwlog_info("di is null\n");
		return;
	}

	hwlog_info("iin_regl_work\n");
	ret = charge_iin_regulation(di);
	if (ret)
		hwlog_err("charge_iin_regulation fail\n");
}

int charge_set_input_current(int iin)
{
	struct charge_device_info *di = g_di;
	int ret;

	iin = (iin < di->input_current) ? iin : di->input_current;

#ifdef CONFIG_WIRELESS_CHARGER
	if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS) {
		wlrx_buck_set_iin_for_charger(WLTRX_DRV_MAIN, iin);
	} else {
#endif
		hwlog_info("charge_set_input_current = %d\n", iin);

		if (di->iin_regulation_enabled &&
			(charge_detecion_get_sinksource_type() == POWER_SUPPLY_START_SINK)) {
			mutex_lock(&di->iin_regl_lock);
			di->iin_target = iin;
			mutex_unlock(&di->iin_regl_lock);
			mod_delayed_work(system_wq, &di->iin_regl_work,
				msecs_to_jiffies(di->iin_regl_interval));
		} else {
			ret = charge_set_dev_iin(iin);
			if (ret) {
				hwlog_err("set input current %d fail\n", iin);
				return ret;
			}
		}
#ifdef CONFIG_WIRELESS_CHARGER
	}
#endif

	return 0;
}

int charge_set_input_current_max(void)
{
	struct charge_device_info *di = g_di;
	int ret;

	di->input_current = di->core_data->iin_max;
	ret = charge_set_dev_iin(di->input_current);
	if (ret) {
		hwlog_err("%s:set input current %d fail\n", __func__, di->input_current);
		return -1;
	}
	hwlog_info("%s:set input current %d succ\n", __func__, di->input_current);
	return 0;
}

static int charge_rename_charger_type(unsigned long type,
	struct charge_device_info *di, bool update_flag)
{
	int ret = TRUE;
	unsigned int charger_type = charge_get_charger_type();

	hwlog_info("%s: type = %ld, last charger_type = %u\n",
		__func__, type, charger_type);
	switch (type) {
	case CHARGER_TYPE_SDP:
		if ((charger_type == CHARGER_REMOVED) ||
			(charger_type == CHARGER_TYPE_NON_STANDARD) ||
			(charger_type == CHARGER_TYPE_USB) ||
			(charger_type == CHARGER_TYPE_WIRELESS) ||
			(charger_type == CHARGER_TYPE_POGOPIN)) {
			charge_set_charger_type(CHARGER_TYPE_USB);
			charge_set_charger_source(POWER_SUPPLY_TYPE_USB);
			ret = FALSE;
		}
		break;
	case CHARGER_TYPE_CDP:
		if ((charger_type == CHARGER_REMOVED) ||
			(charger_type == CHARGER_TYPE_NON_STANDARD) ||
			(charger_type == CHARGER_TYPE_WIRELESS) ||
			(charger_type == CHARGER_TYPE_POGOPIN) ||
			charger_type_update) {
			charge_set_charger_type(CHARGER_TYPE_BC_USB);
			charge_set_charger_source(POWER_SUPPLY_TYPE_USB);
			charger_type_update = FALSE;
			ret = FALSE;
		}
		break;
	case CHARGER_TYPE_DCP:
		if (charger_type != CHARGER_TYPE_STANDARD) {
			charge_set_charger_type(CHARGER_TYPE_STANDARD);
			charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
			if (di->support_standard_ico == 1)
				ico_enable = 1;
			else
				ico_enable = 0;
			ret = FALSE;
		}
		break;
	case CHARGER_TYPE_UNKNOWN:
		if ((charger_type == CHARGER_REMOVED) ||
			(charger_type == CHARGER_TYPE_WIRELESS) ||
			(charger_type == CHARGER_TYPE_USB) ||
			(charger_type == CHARGER_TYPE_POGOPIN) ||
			charger_type_update) {
			charge_set_charger_type(CHARGER_TYPE_NON_STANDARD);
			charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
			charger_type_update = FALSE;
			ret = FALSE;
		}
		break;
	case CHARGER_TYPE_NONE:
		if (charger_type != CHARGER_REMOVED) {
			charge_set_charger_type(CHARGER_REMOVED);
			charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
		}
		ret = FALSE;
		break;
	case PLEASE_PROVIDE_POWER:
		if (charger_type == CHARGER_REMOVED) {
			charge_set_charger_type(USB_EVENT_OTG_ID);
			charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
			ret = FALSE;
		}
		break;
	case CHARGER_TYPE_WIRELESS:
		charge_set_charger_type(CHARGER_TYPE_WIRELESS);
		charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		ret = FALSE;
		break;
	default:
		charge_set_charger_type(CHARGER_REMOVED);
		charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
		ret = FALSE;
		break;
	}
	return ret;
}

static void charge_update_charger_type(struct charge_device_info *di)
{
	int type;

	if (charge_get_charger_type() != CHARGER_TYPE_NON_STANDARD)
		return;

	nonstand_detect_times++;

	type = charge_switch_get_charger_type();
	if (type >= 0) {
#ifdef CONFIG_SWITCH
		if (usbswitch_common_dcd_timeout_status() == 1) {
			if (type == CHARGER_TYPE_DCP) {
				charge_rename_charger_type(type, di, TRUE);
				hwlog_info("[%s]charger type is update to DCP from nonstd charger! type is [%u]\n",
					__func__, charge_get_charger_type());
			}
		} else {
#endif
			if (type != CHARGER_TYPE_NONE) {
				charge_rename_charger_type(type, di, TRUE);
				hwlog_info("[%s]charger type is update to[%u] from nonstd charger\n",
					__func__, charge_get_charger_type());
			}
#ifdef CONFIG_SWITCH
		}
#endif
	}
}

static int fcp_start_charging(struct charge_device_info *di)
{
	int ret;

	hvdcp_set_charging_stage(HVDCP_STAGE_SUPPORT_DETECT);

	/* check whether support fcp detect */
	if (adapter_get_protocol_register_state(ADAPTER_PROTOCOL_FCP)) {
		hwlog_err("not support fcp\n");
		return -1;
	}
#ifdef CONFIG_POGO_PIN
	if (pogopin_is_support() && (pogopin_5pin_get_fcp_support() == false)) {
		hwlog_err("pogopin not support fcp\n");
		return -1;
	}
#endif /* CONFIG_POGO_PIN */
	/* To avoid to effect accp detect , input current need to be lower than 1A,we set 0.5A */
	di->input_current = CHARGE_CURRENT_0500_MA;
	charge_set_input_current(di->input_current);

	if (charge_get_hiz_state()) {
		hwlog_info("charge in hiz state\n");
		return -1;
	}

	/* detect fcp adapter */
	hvdcp_set_charging_stage(HVDCP_STAGE_ADAPTER_DETECT);
	ret = hvdcp_detect_adapter();
	if (ret)
		return ret;

	adapter_test_set_result(AT_TYPE_FCP, AT_DETECT_SUCC);
	hvdcp_set_charging_stage(HVDCP_STAGE_ADAPTER_ENABLE);
	g_init_crit.vbus = ADAPTER_9V;
	g_init_crit.charger_type = charge_get_charger_type();
	(void)charge_init_chip(&g_init_crit);

	ret = hvdcp_set_adapter_voltage(ADAPTER_9V * POWER_MV_PER_V);
	if (ret) {
		g_init_crit.vbus = ADAPTER_5V;
		g_init_crit.charger_type = charge_get_charger_type();
		(void)charge_init_chip(&g_init_crit);
		(void)hvdcp_reset_master();
		return 1;
	}

	(void)charge_set_vbus_vset(ADAPTER_9V);
	hvdcp_set_vboost_retry_count(0);

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP) {
		hvdcp_set_charging_stage(HVDCP_STAGE_DEFAUTL);
		return 0;
	}
	adapter_test_set_result(AT_TYPE_FCP, AT_PROTOCOL_FINISH_SUCC);
#ifdef CONFIG_DIRECT_CHARGER
	wired_connect_send_icon_uevent(ICON_TYPE_QUICK);
#endif
	power_icon_notify(ICON_TYPE_QUICK);
	charge_set_charger_type(CHARGER_TYPE_FCP);
	hvdcp_set_charging_stage(HVDCP_STAGE_SUCCESS);
	hvdcp_set_charging_flag(true);
	msleep(CHIP_RESP_TIME);
	hwlog_info("fcp charging start success\n");
	return 0;
}

static void charge_vbus_voltage_check(struct charge_device_info *di)
{
	int ret = 0;
	unsigned int vbus_vol = 0;
	unsigned int vbus_ovp_cnt = 0;
	int i;
	unsigned int state = CHAGRE_STATE_NORMAL;

	if (!di || !charge_ops_valid(get_vbus))
		return;

	if (charge_ops_valid(set_covn_start))
		ret = charge_set_covn_start(true);
	if (ret) {
		hwlog_err("[%s]set covn start fail\n", __func__);
		return;
	}
	ret = charge_get_charging_state(&state);
	if (ret < 0) {
		hwlog_err("get_charge_state fail!!ret = 0x%x\n", ret);
		return;
	}

	for (i = 0; i < VBUS_VOL_READ_CNT; ++i) {
		charge_get_vbus(&vbus_vol);
		if (vbus_vol > VBUS_VOLTAGE_13400_MV) {
			if (!(state & CHAGRE_STATE_NOT_PG))
				vbus_ovp_cnt++; /* if power ok, then count plus one */
			msleep(25); /* Wait for chargerIC to be in stable state */
		} else {
			break;
		}
	}
	if (vbus_ovp_cnt == VBUS_VOL_READ_CNT) {
		hwlog_err("[%s]vbus_vol = %u\n", __func__, vbus_vol);
		charge_state_dsm_report(ERROR_VBUS_VOL_OVER_13400MV);
	}
	if (charge_get_charger_type() == CHARGER_TYPE_PD) {
#ifdef CONFIG_TCPC_CLASS
		if (!pd_charge_flag)
			return;
		if (pd_dpm_get_high_voltage_charging_status() &&
			(vbus_vol > VBUS_VOLTAGE_7000_MV)) {
			if (!charge_get_reset_adapter_source()) {
				last_vset = ADAPTER_9V * POWER_MV_PER_V;
			} else {
				ret = adapter_set_output_voltage(
					ADAPTER_PROTOCOL_PD, ADAPTER_5V * POWER_MV_PER_V);
				if (ret)
					hwlog_err("[%s]set voltage failed\n",
						__func__);
			}
			pd_vbus_abnormal_cnt = 0;
			return;
		}
		if (!pd_dpm_get_high_voltage_charging_status() &&
			(vbus_vol < VBUS_VOLTAGE_7000_MV)) {
			if ((!pd_dpm_get_optional_max_power_status()) ||
				charge_get_reset_adapter_source()) {
				last_vset = ADAPTER_5V * POWER_MV_PER_V;
			} else {
				ret = adapter_set_output_voltage(
					ADAPTER_PROTOCOL_PD, ADAPTER_9V * POWER_MV_PER_V);
				if (ret)
					hwlog_err("[%s]set voltage failed\n",
						__func__);
			}
			pd_vbus_abnormal_cnt = 0;
			return;
		}
		if (++pd_vbus_abnormal_cnt >= VBUS_VOLTAGE_ABNORMAL_MAX_COUNT) {
			hwlog_err("%s: pd_vbus_abnormal_cnt = %d, hard_reset\n",
				__func__, VBUS_VOLTAGE_ABNORMAL_MAX_COUNT);
			adapter_hard_reset_master(ADAPTER_PROTOCOL_PD);
		}
		return;
#endif
	}
#ifdef CONFIG_WIRELESS_CHARGER
	if ((charge_get_charger_type() == CHARGER_TYPE_WIRELESS) ||
		(charge_get_reset_adapter_source() & BIT(RESET_ADAPTER_SOURCE_WLTX)))
		return;
#endif
	if (hvdcp_get_charging_stage() == HVDCP_STAGE_SUCCESS) {
		/* fcp stage : vbus must be higher than 7000 mV */
		if (vbus_vol < VBUS_VOLTAGE_7000_MV) {
			fcp_vbus_lower_count += 1;
			hwlog_err("[%s]fcp output vol =%d mV, lower 7000 mV , fcp_vbus_lower_count =%d\n",
				__func__, vbus_vol, fcp_vbus_lower_count);
		} else {
			fcp_vbus_lower_count = 0;
		}
		/* check continuous abnormal vbus cout  */
		if (fcp_vbus_lower_count >= VBUS_VOLTAGE_ABNORMAL_MAX_COUNT) {
			vbus_flag = vbus_flag + 1;
			hvdcp_check_adapter_status();
			hvdcp_set_charging_stage(HVDCP_STAGE_DEFAUTL);
			charge_set_charger_type(CHARGER_TYPE_STANDARD);
			if (hvdcp_reset_adapter())
				hwlog_err("adapter reset failed\n");
			(void)charge_set_vbus_vset(ADAPTER_5V);
			fcp_vbus_lower_count = VBUS_VOLTAGE_ABNORMAL_MAX_COUNT;
		}
		if (vbus_flag >= VBUS_REPORT_NUM) {
			vbus_flag = 0;
			charge_state_dsm_report(ERROR_FCP_VOL_OVER_HIGH);
		}
		nonfcp_vbus_higher_count = 0;
	} else {
		/* non fcp stage : vbus must be lower than 6500 mV */
		if (vbus_vol > VBUS_VOLTAGE_6500_MV) {
			nonfcp_vbus_higher_count += 1;
			hwlog_info("[%s]non standard fcp and vbus voltage is %d mv,over 6500mv ,nonfcp_vbus_higher_count =%d\n",
				__func__, vbus_vol, nonfcp_vbus_higher_count);
		} else {
			nonfcp_vbus_higher_count = 0;
		}
		/* check continuous abnormal vbus cout  */
		if (nonfcp_vbus_higher_count >= VBUS_VOLTAGE_ABNORMAL_MAX_COUNT) {
			di->charge_enable = FALSE;
			nonfcp_vbus_higher_count = VBUS_VOLTAGE_ABNORMAL_MAX_COUNT;
			charge_state_dsm_report(ERROR_FCP_VOL_OVER_HIGH);
			if (adapter_is_accp_charger_type(ADAPTER_PROTOCOL_FCP)) {
				if (hvdcp_reset_adapter())
					hwlog_err("adapter reset failed\n");
				hwlog_info("[%s]is fcp adapter\n", __func__);
			} else {
				hwlog_info("[%s] is not fcp adapter\n", __func__);
			}
		}
		fcp_vbus_lower_count = 0;
	}
}

static void pd_charge_check(struct charge_device_info *di)
{
#ifdef CONFIG_TCPC_CLASS
	if (pd_charge_flag)
		return;
	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	if ((charge_get_charger_type() != CHARGER_TYPE_PD) || !power_platform_is_battery_exit())
		return;

	if (dc_adpt_get_inherent_info(ADAP_ANTIFAKE_RESULT) == ADAPTER_ANTIFAKE_FAIL) {
		hwlog_info("[%s] adapter is fake\n", __func__);
		return;
	}

	if (pd_dpm_get_high_voltage_charging_status() == true) {
		g_init_crit.vbus = ADAPTER_9V;
		if (pd_judge_is_cover() == true)
			power_icon_notify(ICON_TYPE_WIRELESS_QUICK);
		else
			power_icon_notify(ICON_TYPE_QUICK);
	} else {
		g_init_crit.vbus = ADAPTER_5V;
	}
	g_init_crit.charger_type = charge_get_charger_type();
	(void)charge_init_chip(&g_init_crit);

	if (pd_dpm_get_high_voltage_charging_status())
		(void)charge_set_vbus_vset(ADAPTER_9V);
	else
		(void)charge_set_vbus_vset(ADAPTER_5V);

	hwlog_info("%s: ok\n", __func__);
	pd_charge_flag = true;
#endif
}

static bool fcp_charge_precheck(unsigned int type)
{
	bool cc_vbus_short = false;

	if (chg_wait_pd_init) {
		hwlog_info("wait pd init\n");
		return false;
	}

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return false;

	if (dc_adpt_get_inherent_info(ADAP_ANTIFAKE_RESULT) == ADAPTER_ANTIFAKE_FAIL) {
		hwlog_info("[%s] adapter is fake\n", __func__);
		return false;
	}

	if (!charge_sysfs_get_sysfs_value(CHARGE_SYSFS_FCP_CHARGE_ENABLE) || !charge_fcp_enable()) {
		hwlog_info("[%s] fcp charge already disable\n", __func__);
		if (type == CHARGER_TYPE_FCP) {
			hvdcp_set_charging_stage(HVDCP_STAGE_DEFAUTL);
			charge_set_charger_type(CHARGER_TYPE_STANDARD);
		}
		return false;
	}

	if (pogopin_typec_chg_ana_audio_support() &&
		(pogopin_5pin_get_pogo_status() == POGO_OTG))
		return false;

	if (charge_get_hiz_state()) {
		hwlog_info("charge in hiz state\n");
		return false;
	}

#ifdef CONFIG_TCPC_CLASS
	/* cc rp 3.0 can not do high voltage charge */
	cc_vbus_short = pd_dpm_check_cc_vbus_short();
	if (cc_vbus_short) {
		hwlog_err("cc match rp3.0, can not do fcp charge\n");
		return false;
	}
#endif
	return true;
}

static void fcp_charge_check(struct charge_device_info *di, unsigned int type)
{
	int ret;
	int i;
	bool adapter_enable = false;
	bool adapter_detect = false;

	if (!fcp_charge_precheck(type))
		return;

	if (hvdcp_get_charging_stage() == HVDCP_STAGE_SUCCESS)
		hvdcp_check_master_status();

	if (!hvdcp_check_charger_type(type) ||
		!power_platform_is_battery_exit()) {
		hwlog_info("charger type not right\n");
		return;
	}

#ifdef CONFIG_DIRECT_CHARGER
	if (direct_charge_is_failed() &&
		(hvdcp_get_charging_stage() < HVDCP_STAGE_SUCCESS))
		hvdcp_set_charging_stage(HVDCP_STAGE_DEFAUTL);
#endif
	if (power_cmdline_is_powerdown_charging_mode())
		msleep(FCP_DETECT_DELAY_IN_POWEROFF_CHARGE);

	if (((hvdcp_get_charging_stage() == HVDCP_STAGE_DEFAUTL) &&
		!(charge_get_reset_adapter_source() & (1 << RESET_ADAPTER_SOURCE_WLTX))) ||
		((hvdcp_get_charging_stage() == HVDCP_STAGE_RESET_ADAPTER) &&
		!charge_get_reset_adapter_source())) {
		ret = fcp_start_charging(di);
		for (i = 0; (i < 3) && (ret == 1); i++) {
			/* reset adapter and try again */
			if (hvdcp_reset_operate(HVDCP_RESET_ADAPTER) < 0)
				break;
			ret = fcp_start_charging(di);
		}
		if (ret == 1) {
			/* reset fsa9688 chip and try again */
			if (hvdcp_reset_operate(HVDCP_RESET_MASTER) == 0)
				ret = fcp_start_charging(di);
		}
		if ((ret == 1) && hvdcp_check_adapter_retry_count()) {
			hvdcp_set_charging_stage(HVDCP_STAGE_DEFAUTL);
			return;
		}

		if (ret == 1) {
			adapter_enable = hvdcp_check_adapter_enable_count();
			adapter_detect = hvdcp_check_adapter_detect_count();
			if (adapter_enable && adapter_detect)
				hvdcp_set_charging_stage(HVDCP_STAGE_DEFAUTL);
		}

		hwlog_info("[%s]fcp stage %s\n", __func__,
			hvdcp_get_charging_stage_string(hvdcp_get_charging_stage()));
	}

	if (charge_get_reset_adapter_source() && (hvdcp_get_charging_stage() == HVDCP_STAGE_SUCCESS)) {
		if (!hvdcp_exit_charging())
			return;

		hwlog_info("reset adapter by user\n");
		hvdcp_set_charging_stage(HVDCP_STAGE_RESET_ADAPTER);
		if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
			return;
		charge_set_charger_type(CHARGER_TYPE_STANDARD);
		msleep(CHIP_RESP_TIME);
	}
}

#ifdef CONFIG_TCPC_CLASS
static void huawei_pd_typec_current(struct charge_device_info *di)
{
	enum pd_dpm_cc_voltage_type cc_type = pd_dpm_get_cc_voltage();

	if (di->scp_adp_normal_chg && dc_adpt_get_inherent_info(IS_ABNORMAL_ADP)) {
		hwlog_info("5V4.5A adp ignore cc voltage\n");
		return;
	}

	switch (cc_type) {
	case PD_DPM_CC_VOLT_SNK_3_0:
		di->input_current = di->input_current < TYPE_C_HIGH_MODE_CURR ?
			di->input_current : TYPE_C_HIGH_MODE_CURR;
		hwlog_info("[%s]PD_DPM_CC_VOLT_SINK_3_0\n", __func__);
		break;
	case PD_DPM_CC_VOLT_SNK_1_5:
		di->input_current = TYPE_C_MID_MODE_CURR;
		hwlog_info("[%s]PD_DPM_CC_VOLT_SINK_1_5\n", __func__);
		break;
	case PD_DPM_CC_VOLT_SNK_DFT:
		hwlog_info("[%s]PD_DPM_CC_VOLT_SINK_DFT\n", __func__);
		break;
	default:
		hwlog_info("[%s]PD_DPM_CC_VOLT_TYPE = %d\n", __func__, cc_type);
		break;
	}
}
#endif

static void charge_typec_current(struct charge_device_info *di)
{
	switch (di->typec_current_mode) {
	case TYPEC_DEV_CURRENT_HIGH:
		di->input_current = di->input_current < TYPE_C_HIGH_MODE_CURR ?
			di->input_current : TYPE_C_HIGH_MODE_CURR;
		di->charge_current = di->charge_current < TYPE_C_HIGH_MODE_CURR ?
			di->charge_current : TYPE_C_HIGH_MODE_CURR;
		break;
	case TYPEC_DEV_CURRENT_MID:
		di->input_current = TYPE_C_MID_MODE_CURR;
		di->charge_current = TYPE_C_MID_MODE_CURR;
		break;
	case TYPEC_DEV_CURRENT_DEFAULT:
	case TYPEC_DEV_CURRENT_NOT_READY:
	default:
		break;
	}
	hwlog_info("[%s]Type C type %d, %d, %d\n", __func__,
		di->typec_current_mode, di->input_current, di->charge_current);
}

static void select_ico_current(struct charge_device_info *di)
{
	static struct ico_input input;
	static struct ico_output output;

	if (di->start_attemp_ico) {
		if (!charge_ops_valid(turn_on_ico))
			return;

		input.charger_type = charge_get_charger_type();
		input.iin_max = di->core_data->iin_max;
		input.ichg_max = di->core_data->ichg_max;
		input.vterm = power_min_positive(di->core_data->vterm,
			charge_sysfs_get_sysfs_value(CHARGE_SYSFS_REGULATION_VOLTAGE));
		output.input_current = di->input_current;
		output.charge_current = di->charge_current;
		if (!charge_turn_on_ico(&input, &output)) {
			if (!di->ico_all_the_way) {
				di->start_attemp_ico = 0;
				hwlog_info("ico result: input current is %dmA, charge current is %dmA\n",
					output.input_current, output.charge_current);
			} else {
				di->start_attemp_ico = 1;
			}
		} else {
			hwlog_info("ico current detect fail\n");
		}
	}

	di->input_current = output.input_current;
	di->charge_current = output.charge_current;
}

static void charge_select_charging_current(struct charge_device_info *di)
{
	static unsigned int first_in = 1;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	switch (charge_get_charger_type()) {
	case CHARGER_TYPE_USB:
		di->input_current = di->core_data->iin_usb;
		di->charge_current = di->core_data->ichg_usb;
		break;
	case CHARGER_TYPE_NON_STANDARD:
		di->input_current = di->core_data->iin_nonstd;
		di->charge_current = di->core_data->ichg_nonstd;
		break;
	case CHARGER_TYPE_BC_USB:
		di->input_current = di->core_data->iin_bc_usb;
		di->charge_current = di->core_data->ichg_bc_usb;
		break;
	case CHARGER_TYPE_STANDARD:
		di->input_current = di->core_data->iin_ac;
		di->charge_current = di->core_data->ichg_ac;
#ifdef CONFIG_DIRECT_CHARGER
		if (di->scp_adp_normal_chg &&
			dc_adpt_get_inherent_info(IS_ABNORMAL_ADP)) {
			/* set adaptor voltage to 5500mV for normal charge */
			if (!dc_set_adapter_output_capability(5500,
				di->core_data->iin_nor_scp, 0)) {
				di->input_current = di->core_data->iin_nor_scp;
				di->charge_current = di->core_data->ichg_nor_scp;
			}
		}
#endif
		break;
	case CHARGER_TYPE_VR:
		di->input_current = di->core_data->iin_vr;
		di->charge_current = di->core_data->ichg_vr;
		break;
	case CHARGER_TYPE_FCP:
		di->input_current = di->core_data->iin_fcp;
		di->charge_current = di->core_data->ichg_fcp;
		break;
	case CHARGER_TYPE_PD:
		di->input_current = di->pd_input_current;
		di->charge_current = di->pd_charge_current;
		hwlog_info("CHARGER_TYPE_PD input_current %d  charge_current = %d",
			di->input_current, di->charge_current);
		if (di->charge_current > di->core_data->ichg_fcp)
			di->charge_current = di->core_data->ichg_fcp;
		break;
#ifdef CONFIG_WIRELESS_CHARGER
	case CHARGER_TYPE_WIRELESS:
		di->input_current = di->core_data->iin_wireless;
		di->charge_current = di->core_data->ichg_wireless;
		break;
#endif
	default:
		di->input_current = CHARGE_CURRENT_0500_MA;
		di->charge_current = CHARGE_CURRENT_0500_MA;
		break;
	}

#ifdef CONFIG_HUAWEI_YCABLE
	if (ycable_is_support() && (ycable_get_status() == YCABLE_CHARGER)) {
		di->input_current = ycable_get_input_current();
		di->charge_current = ycable_get_charge_current();
		hwlog_info("ycable input curr = %d, ichg curr = %d\n",
			di->input_current, di->charge_current);
	}
#endif
#ifdef CONFIG_POGO_PIN
	if (pogopin_is_support() && (charge_get_charger_type() == CHARGER_TYPE_POGOPIN)) {
		di->input_current = pogopin_3pin_get_input_current();
		di->charge_current = pogopin_3pin_get_charger_current();
		hwlog_info("pogopin input curr = %d, ichg curr = %d\n",
			di->input_current, di->charge_current);
	}
#endif /* CONFIG_POGO_PIN */
	/*
	 * only the typec is supported, we need read typec result and
	 * when adapter is fcp adapter, we set current by fcp adapter rule
	 */
	if (di->core_data->typec_support && (hvdcp_get_charging_stage() != HVDCP_STAGE_SUCCESS))
		charge_typec_current(di);

#ifdef CONFIG_TCPC_CLASS
	if (charger_pd_support) {
		switch (charge_get_charger_type()) {
		case CHARGER_TYPE_USB:
		case CHARGER_TYPE_NON_STANDARD:
		case CHARGER_TYPE_BC_USB:
		case CHARGER_TYPE_STANDARD:
			huawei_pd_typec_current(di);
			break;
		default:
			break;
		}
	}
#endif

	if ((charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ENABLE_CHARGER) == 1) && (ico_enable == 1))
		select_ico_current(di);

#ifndef CONFIG_HLTHERM_RUNTEST
	if (power_cmdline_is_factory_mode() && !power_platform_is_battery_exit()) {
		if (first_in) {
			hwlog_info("facory_version and battery not exist, enable charge\n");
			first_in = 0;
		}
	} else {
#endif
		if (charge_sysfs_get_sysfs_value(CHARGE_SYSFS_LIMIT_CHARGING) == TRUE) {
			di->input_current = min(di->input_current, di->core_data->iin);
			di->input_current = min(di->input_current,
				charge_sysfs_get_sysfs_value(CHARGE_SYSFS_IIN_THERMAL));
			di->input_current = min(di->input_current,
				charge_sysfs_get_sysfs_value(CHARGE_SYSFS_IIN_RUNNINGTEST));
			di->charge_current = min(di->charge_current, di->core_data->ichg);
			di->charge_current = min(di->charge_current,
				charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ICHG_THERMAL));
			di->charge_current = min(di->charge_current,
				charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ICHG_RUNNINGTEST));
		}
		/* if charger detected as weaksource, reset iuput current */
		if (((charge_get_charger_type() != CHARGER_TYPE_FCP) &&
			(charge_get_charger_type() != CHARGER_TYPE_WIRELESS)) ||
			((charge_get_charger_type() == CHARGER_TYPE_FCP) &&
			(hvdcp_get_charging_stage() == HVDCP_STAGE_CHARGE_DONE))) {
			if (charge_ops_valid(rboost_buck_limit) &&
				!power_cmdline_is_factory_mode()) {
				if ((charge_rboost_buck_limit() == WEAKSOURCE_TRUE) &&
					(di->core_data->iin_weaksource != INVALID_CURRENT_SET)) {
					hwlog_info("Weak source, reset iin_limit\n");
					di->input_current = di->input_current <
						di->core_data->iin_weaksource ?
						di->input_current : di->core_data->iin_weaksource;
				}
			}
		}
#ifndef CONFIG_HLTHERM_RUNTEST
	}
#endif

	if (charge_sysfs_get_sysfs_value(CHARGE_SYSFS_INPUTCURRENT) != 0)
		di->input_current = min(di->input_current,
			(unsigned int)charge_sysfs_get_sysfs_value(CHARGE_SYSFS_INPUTCURRENT));

	if (charge_sysfs_get_sysfs_value(CHARGE_SYSFS_BATFET_DISABLE) == 1)
		di->input_current = CHARGE_CURRENT_2000_MA;

	wireless_tx_limit_input_current(di);
}

static int charge_update_pd_vbus_voltage_check(struct charge_device_info *di)
{
	int ret;
	unsigned int vbus_vol = 0;

	if (!di) {
		hwlog_err("input para is null, just return false\n");
		return FALSE;
	}
	if (charge_get_charger_type() == CHARGER_TYPE_PD) {
		if (charge_ops_valid(get_vbus)) {
			ret = charge_get_vbus(&vbus_vol);
		} else {
			hwlog_err("Not support VBUS check\n");
			ret = -EINVAL;
		}
		if (ret) {
			hwlog_err("[%s]vbus vol read fail\n", __func__);
			ret = pd_dpm_get_high_power_charging_status() ? TRUE : FALSE;
		} else {
			ret = vbus_vol > VBUS_VOLTAGE_7000_MV ? TRUE : FALSE;
		}
		return ret;
	}
	return FALSE;
}

static void charge_update_vindpm(struct charge_device_info *di)
{
	int ret;
	int vindpm = CHARGE_VOLTAGE_4520_MV;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;
	if ((hvdcp_get_charging_stage() == HVDCP_STAGE_SUCCESS)
#ifdef CONFIG_TCPC_CLASS
		|| charge_update_pd_vbus_voltage_check(di)
#endif
	) {
		vindpm = di->fcp_vindpm;
	} else if (charge_get_charger_source() == POWER_SUPPLY_TYPE_MAINS) {
		vindpm = di->core_data->vdpm;
	} else if (charge_get_charger_source() == POWER_SUPPLY_TYPE_USB) {
		if (di->core_data->vdpm > CHARGE_VOLTAGE_4520_MV)
			vindpm = di->core_data->vdpm;
	}

	if (charge_ops_valid(set_dpm_voltage)) {
		ret = charge_set_dpm_voltage(vindpm);
		if (ret > 0) {
			hwlog_info("dpm voltage is out of range:%dmV\n", ret);
			ret = charge_set_dpm_voltage(ret);
			if (ret < 0)
				hwlog_err("set dpm voltage fail\n");
		} else if (ret < 0) {
			hwlog_err("set dpm voltage fail\n");
		}
	}
}

static void charge_update_external_setting(struct charge_device_info *di)
{
	unsigned int batfet_disable = FALSE;
	unsigned int watchdog_timer = CHAGRE_WDT_80S;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	/* update batfet setting */
	if (charge_sysfs_get_sysfs_value(CHARGE_SYSFS_BATFET_DISABLE) == TRUE)
		batfet_disable = TRUE;

	(void)charge_set_batfet_disable(batfet_disable);

	/* update watch dog timer setting */
	if (charge_sysfs_get_sysfs_value(CHARGE_SYSFS_WATCHDOG_DISABLE) == TRUE)
		watchdog_timer = CHAGRE_WDT_DISABLE;

	(void)charge_set_watchdog(watchdog_timer);
}

void huawei_charge_update_iterm(int iterm)
{
	struct charge_device_info *di = g_di;

	if (!di || !di->core_data || (iterm <= 0))
		return;

	di->core_data->iterm = (unsigned int)iterm;
	hwlog_info("%s iterm=%d\n", __func__, iterm);
}

static int calc_avg_input_current_ma(struct charge_device_info *di, int input_current_ma)
{
	int i;
	int iavg_ma;
	static int iavg_samples[IIN_AVG_SAMPLES];
	static int iavg_index;
	static int iavg_num_samples;

	if (!di) {
		hwlog_info("NULL point in [%s]\n", __func__);
		return IMPOSSIBLE_IIN;
	}
	if (clear_iin_avg_flag) {
		iavg_index = 0;
		iavg_num_samples = 0;
		clear_iin_avg_flag = 0;
	}
	if (input_current_ma == IMPOSSIBLE_IIN) {
		hwlog_info("input_current_ma is invalid [%s]\n", __func__);
		return IMPOSSIBLE_IIN;
	}
	iavg_samples[iavg_index] = input_current_ma;
	iavg_index = (iavg_index + 1) % IIN_AVG_SAMPLES;
	iavg_num_samples++;
	if (iavg_num_samples >= IIN_AVG_SAMPLES)
		iavg_num_samples = IIN_AVG_SAMPLES;

	iavg_ma = 0;
	for (i = 0; i < iavg_num_samples; i++)
		iavg_ma += iavg_samples[i];

	iavg_ma = DIV_ROUND_CLOSEST(iavg_ma, iavg_num_samples);

	if (iavg_num_samples > IIN_AVG_SAMPLES / 2)
		return  iavg_ma;
	else
		return IMPOSSIBLE_IIN;
}

static int charge_check_charging_full(struct charge_full_check_info *info,
	struct charge_device_info *di)
{
	int val = FALSE;
	int term_allow = FALSE;
	int bat_cap = 0;

	if (!di->charge_enable || !info->bat_exist)
		return val;

	if (((info->ichg > MIN_CHARGING_CURRENT_OFFSET) &&
		(info->ichg_avg > MIN_CHARGING_CURRENT_OFFSET)) ||
		di->core_data->warm_triggered)
		term_allow = TRUE;

	if (term_allow && (info->ichg < (int)di->core_data->iterm) &&
		(info->ichg_avg < (int)di->core_data->iterm)) {
		di->check_full_count++;
		if (di->check_full_count > BATTERY_FULL_CHECK_TIMIES) {
			di->check_full_count = BATTERY_FULL_CHECK_TIMIES;
			val = TRUE;
			power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
				POWER_SUPPLY_PROP_CAPACITY, &bat_cap,
				POWER_SUPPLY_DEFAULT_CAPACITY, 1);
			hwlog_info("capacity = %d, ichg = %d, ichg_avg = %d\n",
				bat_cap, info->ichg, info->ichg_avg);
		}
	} else {
		di->check_full_count = 0;
	}

	return val;
}

static int charge_get_battery_info(const char *name,
	struct charge_full_check_info *info)
{
	int ret;

	ret = power_supply_get_int_property_value(name,
		POWER_SUPPLY_PROP_CURRENT_NOW, &info->ichg);
	ret += power_supply_get_int_property_value(name,
		POWER_SUPPLY_PROP_CURRENT_AVG, &info->ichg_avg);
	ret += power_supply_get_int_property_value(name,
		POWER_SUPPLY_PROP_PRESENT, &info->bat_exist);
	hwlog_info("name=%s ichg=%d ichg_avg=%d bat_exist=%d\n", name,
		info->ichg, info->ichg_avg, info->bat_exist);
	if (ret)
		hwlog_info("%s failed\n", __func__);

	return ret;
}

static int charge_is_charging_full(struct charge_device_info *di)
{
	struct charge_full_check_info bat_main = { 0 };
	struct charge_full_check_info bat_aux = { 0 };
	int flag = FALSE;

	switch (di->charge_full_check_type) {
	case CHARGE_FULL_CHECK_DEFAULT:
		bat_main.ichg = -power_platform_get_battery_current();
		bat_main.ichg_avg = charge_get_battery_current_avg();
		bat_main.bat_exist = power_platform_is_battery_exit();
		flag = charge_check_charging_full(&bat_main, di);
		break;
	case CHARGE_FULL_CHECK_MULTI_BAT:
		(void)charge_get_battery_info("battery_gauge", &bat_main);
		(void)charge_get_battery_info("battery_gauge_aux", &bat_aux);
		if (charge_check_charging_full(&bat_main, di) &&
			charge_check_charging_full(&bat_aux, di)) {
			flag = TRUE;
			hwlog_info("%s two battery charge full\n", __func__);
		}
		break;
	default:
		break;
	}
	return flag;
}

static void charge_check_termination(struct charge_device_info *di, int full_flag)
{
	int ret;
	unsigned int state = CHAGRE_STATE_NORMAL;
	int soc = 0;

	if (!di) {
		hwlog_err("%s, di is null, return\n", __func__);
		return;
	}

	ret = charge_get_charging_state(&state);
	if (ret < 0) {
		hwlog_err("get_charge_state fail!!ret = 0x%x\n", ret);
		return;
	}
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CAPACITY, &soc,
		POWER_SUPPLY_DEFAULT_CAPACITY, 1);
	if (soc < CAPACITY_FULL) {
		term_err_chrg_en_flag = TRUE;
		di->vterm_dec_flag = FALSE;
		term_err_cnt = 0;
	} else if (full_flag && (soc >= CAPACITY_FULL) &&
		!(state & CHAGRE_STATE_CHRG_DONE) &&
		(term_err_chrg_en_flag == TRUE)) {
		term_err_cnt++;
		if (term_err_cnt >= di->term_exceed_time) {
			term_err_cnt = di->term_exceed_time;
			if (di->term_err_vterm_dec && !di->core_data->warm_triggered)
				di->vterm_dec_flag = TRUE;
			else
				term_err_chrg_en_flag = FALSE;
			hwlog_err("%s err, stop charging\n", __func__);
		}
	} else {
		term_err_cnt = 0;
	}
}

static int is_charge_current_full(struct charge_device_info *di)
{
	int ichg = -power_platform_get_battery_current();
	int ichg_avg = charge_get_battery_current_avg();
	int bat_vol = hw_battery_voltage(BAT_ID_MAX);
	int bat_cap = 0;
	int val = FALSE;
	int current_full_allow = FALSE;

	if (!di)
		return val;

	if (!di->charge_enable || !power_platform_is_battery_exit())
		return val;

	if ((ichg > MIN_CHARGING_CURRENT_OFFSET) && (ichg_avg > MIN_CHARGING_CURRENT_OFFSET))
		current_full_allow = TRUE;

	if (current_full_allow && charge_ops_valid(check_input_vdpm_state) &&
		charge_ops_valid(check_input_idpm_state)) {
		if (charge_check_input_vdpm_state() || charge_check_input_idpm_state())
			current_full_allow = FALSE;
	}
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CAPACITY, &bat_cap,
		POWER_SUPPLY_DEFAULT_CAPACITY, 1);
	if (current_full_allow && !di->core_data->warm_triggered &&
		(ichg <= batt_ifull + DELA_ICHG_FOR_CURRENT_FULL) &&
		(ichg_avg >= batt_ifull - DELA_ICHG_FOR_CURRENT_FULL) &&
		(ichg_avg <= batt_ifull + DELA_ICHG_FOR_CURRENT_FULL) &&
		(di->charge_current >= batt_ifull) && (di->input_current >= batt_ifull) &&
		(charge_get_ibus() < di->max_iin_ma * CURRENT_FULL_VALID_PERCENT / 100) &&
		(di->max_iin_ma != IMPOSSIBLE_IIN) &&
		(bat_vol > di->core_data->vterm - CURRENT_FULL_VOL_OFFSET) &&
		(bat_cap >= CAP_TH_FOR_CURRENT_FULL)) {
		di->check_current_full_count++;
		if (di->check_current_full_count >= CURRENT_FULL_CHECK_TIMES) {
			di->check_current_full_count = CURRENT_FULL_CHECK_TIMES;
			val = TRUE;
		}
	} else {
		di->check_current_full_count = 0;
	}
	hwlog_info("[%s] ichg %d,ichg_avg %d,bat_vol %d,ibus %d,max_iin_ma %d,warm_triggered %d,is_current_full %d\n",
		__func__, ichg, ichg_avg, bat_vol, charge_get_ibus(), di->max_iin_ma,
		di->core_data->warm_triggered, val);
	return val;
}

static void charge_full_dmd_report(struct charge_device_info *di, int is_battery_full)
{
	char tmp_buff[CHARGE_FULL_DMD_BUFF_SIZE] = { 0 };
	int vbat_main = 0;
	int vbat_aux = 0;
	static int last_state = 0;
	int ret;

	if (!di)
		return;

	if (is_battery_full == 0) {
		last_state = is_battery_full;
		return;
	}
	if (last_state == 1)
		return;

	switch (di->charge_full_check_type) {
	case CHARGE_FULL_CHECK_DEFAULT:
		power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, &vbat_main,
			POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
		hwlog_info("[%s] vbat_main= %d\n", __func__, vbat_main);
		ret = snprintf_s(tmp_buff, sizeof(tmp_buff), sizeof(tmp_buff) - 1, "[charge done]vbat = %d\n", vbat_main);
		if (ret < 0) {
			hwlog_err("%s, offset %d, snprintf_s error\n", __func__, ret);
			return;
		}
		break;
	case CHARGE_FULL_CHECK_MULTI_BAT:
		(void)power_supply_get_int_property_value("battery_gauge", POWER_SUPPLY_PROP_VOLTAGE_NOW, &vbat_main);
		(void)power_supply_get_int_property_value("battery_gauge_aux", POWER_SUPPLY_PROP_VOLTAGE_NOW, &vbat_aux);
		hwlog_info("[%s] vbat_main= %d uv , vbat_aux = %d uv\n", __func__, vbat_main, vbat_aux);
		ret = snprintf_s(tmp_buff, sizeof(tmp_buff), sizeof(tmp_buff) - 1, "[charge done]vbat1 = %d uv, vbat2 = %d uv\n",
			 vbat_main, vbat_aux);
		if (ret < 0) {
			hwlog_err("%s, offset %d, snprintf_s error\n", __func__, ret);
			return;
		}
		break;
	default:
		break;
	}
	hwlog_info("%s %s\n", __func__, tmp_buff);
	power_dsm_report_dmd(POWER_DSM_BATTERY, POWER_DSM_CHARGE_DYNAMIC_CV_CHARGER_DONE_VBAT_INFO, tmp_buff);
	last_state = is_battery_full;
}

static void charge_full_handle(struct charge_device_info *di)
{
	int ret;
	int is_battery_full = charge_is_charging_full(di);
	int bat_cap = 0;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	if (!di || !charge_ops_valid(set_term_enable)) {
		hwlog_err("di or ops is null\n");
		return;
	}

	if (!di->charge_term_disable) {
		charge_full_dmd_report(di, is_battery_full);
		ret = charge_set_term_enable(is_battery_full);
	} else {
		ret = charge_set_term_enable(0);
	}
	if (ret)
		hwlog_err("set term enable fail\n");

	/* set terminal current */
	ret = charge_set_terminal_current(di->core_data->iterm);
	if (ret > 0)
		charge_set_terminal_current(ret);
	else if (ret < 0)
		hwlog_err("set terminal current fail\n");

	/*
	 * reset adapter to 5v after fcp charge done(soc is 100),
	 * avoid long-term at high voltage
	 */
	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CAPACITY, &bat_cap,
		POWER_SUPPLY_DEFAULT_CAPACITY, 1);
	if (is_battery_full && (bat_cap == 100)) {
		if (hvdcp_get_charging_stage() == HVDCP_STAGE_RESET_ADAPTER) {
			hvdcp_set_charging_stage(HVDCP_STAGE_CHARGE_DONE);
			hwlog_info("reset adapter to 5v already\n");
		} else if (hvdcp_get_charging_stage() == HVDCP_STAGE_SUCCESS) {
			if ((di->charge_done_maintain_fcp == 1) &&
				!power_cmdline_is_powerdown_charging_mode()) {
				hwlog_info("fcp charge done, no reset adapter to 5v\n");
			} else {
				(void)hvdcp_decrease_adapter_voltage_to_5v();
				hvdcp_set_charging_stage(HVDCP_STAGE_CHARGE_DONE);
				(void)charge_set_vbus_vset(ADAPTER_5V);
				hwlog_info("fcp charge done, reset adapter to 5v\n");
			}
		}
	}
	charge_check_termination(di, is_battery_full);
}

static int set_charge_state(int state)
{
	int old_state;
	int chg_en;
	struct charge_device_info *di = g_di;

	if (!di || ((state != 0) && (state != 1)) || !charge_ops_valid(get_charge_enable_status))
		return -1;

	old_state = charge_get_charge_enable_status();
	chg_en = state;
	charge_set_charge_enable(chg_en);

	return old_state;
}

bool charge_get_hiz_state(void)
{
	bool ret = false;
	struct charge_device_info *di = g_di;

	if (!di)
		return false;
	mutex_lock(&di->mutex_hiz);
	if (di->hiz_ref)
		ret = true;
	else
		ret = false;
	mutex_unlock(&di->mutex_hiz);
	return ret;
}

int charge_set_hiz_enable(int enable)
{
	struct charge_device_info *di = g_di;
	int ret = 0;

	if (!di || !charge_ops_valid(set_charger_hiz))
		return -1;

	mutex_lock(&di->mutex_hiz);
	hwlog_info("set hiz enable = %d refcnt = %u\n", enable, di->hiz_ref);
	if (enable) {
		ret = charger_set_hiz(enable);
		di->hiz_ref++;
	} else {
		if (di->hiz_ref == 0) {
			ret = charger_set_hiz(enable);
			hwlog_err("%s: Unbalanced hiz mode\n", __func__);
		} else {
			if (di->hiz_ref == 1)
				ret = charger_set_hiz(enable);
			di->hiz_ref--;
		}
	}
	mutex_unlock(&di->mutex_hiz);
	return ret;
}

int charge_set_wlsin_hiz_enable(int hz_enable)
{
	return 0;
}

int charge_set_hiz_enable_by_direct_charge(int enable)
{
	struct charge_device_info *di = g_di;

	if (!di)
		return -1;

	if (enable)
		di->is_dc_enable_hiz = true;
	else
		di->is_dc_enable_hiz = false;
	return charge_set_hiz_enable(enable);
}

int set_charger_disable_flags(int val, int type)
{
	struct charge_device_info *di = g_di;
	int i;
	int disable = 0;
	unsigned int charge_disable;

	if (!di) {
		hwlog_err("NULL pointer(di) found in %s\n", __func__);
		return -1;
	}

	if ((type < 0) || (type >= MAX_DISABLE_CHAGER)) {
		hwlog_err("invalid disable_type=%d\n", type);
		return -1;
	}

	di->disable_charger[type] = val;
	for (i = 0; i < MAX_DISABLE_CHAGER; i++)
		disable |= di->disable_charger[i];
	charge_disable = charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ENABLE_CHARGER);
	if (charge_disable == disable) {
		charge_disable = !disable;
		charge_sysfs_set_sysfs_value(CHARGE_SYSFS_ENABLE_CHARGER, charge_disable);
		if (!charge_disable)
			charge_set_charge_enable(charge_disable);
	}
	return 0;
}

void charge_request_charge_monitor(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("[%s]di is null\n", __func__);
		return;
	}

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	if ((charge_get_charger_type() == CHARGER_REMOVED) ||
		(charge_get_charger_source() == POWER_SUPPLY_TYPE_BATTERY)) {
		hwlog_info("charger already plugged out\n");
		return;
	}

	mod_delayed_work(system_power_efficient_wq, &di->charge_work, 0);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static void charge_recharge_get_charge_time(unsigned int enable, struct timespec64 *time)
{
	if (!enable || !time)
		return;

	power_get_timeofday(time);
}
#else
static void charge_recharge_get_charge_time(unsigned int enable, struct timeval *time)
{
	if (!enable || !time)
		return;

	power_get_timeofday(time);
}
#endif

void huawei_charge_recharge_get_charge_time(void)
{
	if (!g_di)
		return;

	charge_recharge_get_charge_time(g_di->recharge_para[RECHARGE_DMD_SWITCH],
		&g_di->charge_done_time);
}

static void charge_start_charging(struct charge_device_info *di)
{
	hwlog_info("---->START CHARGING\n");
	power_wakeup_lock(charge_lock, false);
	if (!power_cmdline_is_factory_mode())
		set_charger_disable_flags(CHARGER_CLEAR_DISABLE_FLAGS, CHARGER_SYS_NODE);

	term_err_chrg_en_flag = TRUE;
	term_err_cnt = 0;
	di->check_full_count = 0;
	di->start_attemp_ico = 1;
	clear_iin_avg_flag = 1;
	di->current_full_status = 0;
	di->avg_iin_ma = IMPOSSIBLE_IIN;
	di->max_iin_ma = IMPOSSIBLE_IIN;
	di->smart_battery_chargedone = false;
	/* chip init */
	g_init_crit.vbus = ADAPTER_5V;
	g_init_crit.charger_type = charge_get_charger_type();
	(void)charge_init_chip(&g_init_crit);
	power_platform_charge_enable_sys_wdt();
	charge_reset_quicken_work_flag();
	mod_delayed_work(system_power_efficient_wq, &di->charge_work, 0);

	/* notify start charging event */
	power_event_bnc_notify(POWER_BNT_CHARGING, POWER_NE_CHARGING_START, NULL);
	(void)direct_charge_set_stage_status_default();
	power_ui_int_event_notify(POWER_UI_NE_MAX_POWER, 0);
	charge_recharge_get_charge_time(di->recharge_para[RECHARGE_DMD_SWITCH],
		&di->charge_start_time);
}

static void charge_stop_charging(struct charge_device_info *di)
{
	int ret;

	hwlog_info("---->STOP CHARGING\n");
	di->vterm_dec_flag = FALSE;
	pd_charge_flag = false;
	charge_set_monitor_work_flag(CHARGE_MONITOR_WORK_NEED_STOP);
	power_wakeup_lock(charge_lock, false);
	nonstand_detect_times = 0;
	ico_enable = 0;
	g_recharge_count = 0;
	if (!power_cmdline_is_factory_mode())
		set_charger_disable_flags(CHARGER_SET_DISABLE_FLAGS, CHARGER_SYS_NODE);

	charge_sysfs_set_sysfs_value(CHARGE_SYSFS_ADC_CONV_RATE, 0);
	charge_sysfs_set_sysfs_value(CHARGE_SYSFS_CHARGE_DONE_STATUS, CHARGE_DONE_NON);
	charge_state_stop_charging_handler();
#ifdef CONFIG_TCPC_CLASS
	di->pd_input_current = 0;
	di->pd_charge_current = 0;
#endif

	power_icon_notify(ICON_TYPE_INVALID);
	/* notify stop charging event */
	power_event_bnc_notify(POWER_BNT_CHARGING, POWER_NE_CHARGING_STOP, NULL);
	/* notify event to power ui */
	power_ui_event_notify(POWER_UI_NE_DEFAULT, NULL);

	if (adapter_set_default_param(ADAPTER_PROTOCOL_FCP))
		hwlog_err("fcp set default param failed\n");

#ifdef CONFIG_DIRECT_CHARGER
	if (di->force_disable_dc_path && (direct_charge_in_charging_stage() ==
		DC_IN_CHARGING_STAGE))
		direct_charge_force_disable_dc_path();

	if (!direct_charge_get_cutoff_normal_flag())
		direct_charge_set_stage_status_default();
#endif

	uvdm_charge_cutoff_reset_param();
	charger_type_update = FALSE;
	vbus_flag = 0;
	nonfcp_vbus_higher_count = 0;
	fcp_vbus_lower_count = 0;
	pd_vbus_abnormal_cnt = 0;
	hvdcp_reset_flags();
#ifdef CONFIG_DIRECT_CHARGER
	try_pd_to_scp_counter = 0;
#endif
	try_pd_to_fcp_counter = 0;
	if (charge_ops_valid(set_adc_conv_rate))
		charge_set_adc_conv_rate(charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ADC_CONV_RATE));
	di->check_full_count = 0;
	ret = charge_set_charge_enable(FALSE);
	if (ret)
		hwlog_err("[%s]set charge enable fail\n", __func__);
	charge_otg_mode_enable(CHARGE_OTG_DISABLE, VBUS_CH_USER_WIRED_OTG);
	cancel_delayed_work_sync(&di->iin_regl_work);
	mutex_lock(&di->iin_regl_lock);
	di->iin_now = 0;
	mutex_unlock(&di->iin_regl_lock);
	cancel_delayed_work_sync(&di->charge_work);
	charge_set_monitor_work_flag(CHARGE_MONITOR_WORK_NEED_START);
	/* when charger stop, disable watch dog, only for hiz */
	if (charge_sysfs_get_sysfs_value(CHARGE_SYSFS_WATCHDOG_DISABLE) == TRUE) {
		if (charge_set_watchdog(CHAGRE_WDT_DISABLE))
			hwlog_err("set watchdog timer fail for hiz\n");
	}
	if (charge_ops_valid(stop_charge_config)) {
		if (charge_stop_charge_config())
			hwlog_err("stop charge config failed\n");
	}

	if (adapter_stop_charging_config(ADAPTER_PROTOCOL_FCP))
		hwlog_err("fcp stop charge config failed\n");

	stop_charging_core_config();
	/* flag must be clear after charge_work has been canceled */
	hvdcp_set_charging_stage(HVDCP_STAGE_DEFAUTL);
	hvdcp_set_charging_flag(false);
#ifdef CONFIG_DIRECT_CHARGER
	if (!direct_charge_get_cutoff_normal_flag())
		direct_charge_exit();

	direct_charge_update_cutoff_normal_flag();
	dc_set_adapter_default_param();
	(void)dcm_reset_and_init_ic_reg(SC_MODE, CHARGE_IC_MAIN);
#endif

	__pm_wakeup_event(stop_charge_lock, jiffies_to_msecs(HZ));
	mutex_lock(&charge_wakelock_flag_lock);
	charge_set_wakelock_flag(CHARGE_WAKELOCK_NO_NEED);
	power_wakeup_unlock(charge_lock, false);
	mutex_unlock(&charge_wakelock_flag_lock);
	power_platform_charge_stop_sys_wdt();
	hw_pd_set_hv_en(true);
}

extern void chip_usb_otg_bc_again(void);

void charge_type_dcp_detected_notify(void)
{
	int pmic_vbus_irq_enabled = 1;
#ifdef CONFIG_TCPC_CLASS
	pmic_vbus_irq_enabled = pmic_vbus_irq_is_enabled();
#endif

	if (g_di && (charge_get_charger_type() == CHARGER_TYPE_NON_STANDARD)) {
		chip_usb_otg_bc_again();
		hwlog_info("stop phy enter\n");
		if (!pmic_vbus_irq_enabled) {
			mutex_lock(&g_di->event_type_lock);
			if (charge_detecion_get_sinksource_type() == POWER_SUPPLY_START_SINK) {
				charge_reset_quicken_work_flag();
				mod_delayed_work(system_power_efficient_wq, &g_di->charge_work, 0);
			}
			mutex_unlock(&g_di->event_type_lock);
		} else {
			charge_reset_quicken_work_flag();
			mod_delayed_work(system_power_efficient_wq, &g_di->charge_work, 0);
		}
	}
}

static void charge_pd_voltage_change_work(struct work_struct *work)
{
#ifdef CONFIG_TCPC_CLASS
	int vset;
	int ret;

	if (charge_get_reset_adapter_source())
		vset = ADAPTER_5V * POWER_MV_PER_V;
	else
		vset = ADAPTER_9V * POWER_MV_PER_V;

	if (vset != last_vset) {
		ret = adapter_set_output_voltage(ADAPTER_PROTOCOL_PD, vset);
		if (ret)
			hwlog_err("[%s]set voltage failed\n", __func__);
	}
#endif
}

void charge_set_adapter_voltage(int val, unsigned int type,
	unsigned int delay_time)
{
	struct charge_device_info *di = g_di;
	int pmic_vbus_irq_enabled = 1;
#ifdef CONFIG_TCPC_CLASS
	pmic_vbus_irq_enabled = pmic_vbus_irq_is_enabled();
#endif

	if (!di) {
		hwlog_err("[%s] di is NULL\n", __func__);
		return;
	}

	if (val == ADAPTER_5V)
		charge_set_reset_adapter_source(RESET_ADAPTER_SET_MODE, type);
	else
		charge_set_reset_adapter_source(RESET_ADAPTER_CLEAR_MODE, type);

	/* fcp adapter reset */
	if ((charge_get_charger_type() == CHARGER_TYPE_FCP) ||
		(charge_get_charger_type() == CHARGER_TYPE_STANDARD)) {
		if (!pmic_vbus_irq_enabled) {
			if (charge_detecion_get_sinksource_type() == POWER_SUPPLY_START_SINK)
				mod_delayed_work(system_power_efficient_wq, &di->charge_work, 0);
		} else {
			mod_delayed_work(system_power_efficient_wq, &di->charge_work, 0);
		}
	}
	/* pd adapter reset */
	if (charge_get_charger_type() == CHARGER_TYPE_PD) {
		cancel_delayed_work_sync(&di->pd_voltage_change_work);
		schedule_delayed_work(&di->pd_voltage_change_work, msecs_to_jiffies(delay_time));
	}
}

static int charge_get_incr_term_volt(struct charge_device_info *di)
{
	int ret;
	struct charge_term_volt_nvdata nv_data = { 0 };

	if (!di->increase_term_volt_en)
		return ffc_ctrl_get_incr_vterm();

	ret = power_nv_read(POWER_NV_TERMVOL, &nv_data, sizeof(nv_data));
	if (ret)
		return 0;

	if ((nv_data.flag & INCR_TERM_VOL_ENABLE) &&
		!(nv_data.flag & INCR_TERM_VOL_FORBIDDEN))
		return CHARGE_INCRESE_TERM_VOLT;

	return 0;
}

void huawei_charge_update_volt_term_data(void)
{
	struct charge_device_info *di = g_di;
	struct charge_term_volt_nvdata nv_data = { 0 };
	int vbat = coul_drv_battery_voltage();
	int charge_cycle = coul_drv_battery_cycle_count();
	int ret;

	if (!di)
		return;

	if (power_cmdline_is_factory_mode())
		return;

	if (!di->increase_term_volt_en)
		return;

	ret = power_nv_read(POWER_NV_TERMVOL,
		&nv_data, sizeof(nv_data));
	if (ret)
		return;

	if (nv_data.flag & INCR_TERM_VOL_FORBIDDEN) {
		hwlog_info("%s increase term volt is forbidden\n", __func__);
		return;
	}

	nv_data.term_volt = vbat;
	if ((vbat > di->core_data->vterm) ||
		(vbat < di->core_data->vterm - CHARGE_COUL_FAULT_VOLT) ||
		(charge_cycle > CHARGE_INCRESE_MAX_CYCLE)) {
		nv_data.flag = INCR_TERM_VOL_FORBIDDEN;
		goto write_nve;
	}

	if (vbat > di->core_data->vterm - CHARGE_INCRESE_DETLA_VOLT)
		return;

	nv_data.flag = INCR_TERM_VOL_ENABLE;

write_nve:
	hwlog_info("update term volt nv, flag %d\n", nv_data.flag);
	(void)power_nv_write(POWER_NV_TERMVOL,
		&nv_data, sizeof(nv_data));

	return;
}

#define IBIAS_RETRY_TIMES                   3

static void check_ibias_current_safe(struct charge_device_info *di)
{
	int ichg_coul;
	int ichg_set_before;
	int ichg_set_after;
	int warm_current;
	int temp = 0;
	int retry_times = 0;
	int err_flag = 0;

	if (!di || (charge_get_charger_type() == CHARGER_TYPE_PD) ||
		!charge_ops_valid(get_charge_current))
		return;
	do {
		ichg_set_before = charge_get_charge_current();
		bat_temp_get_rt_temperature(BAT_TEMP_MIXED, &temp);
		msleep(di->check_ibias_sleep_time);
		if (coul_drv_get_calibration_status()) {
			hwlog_info("In hisi_coul_calibration_status\n");
			return;
		}
		ichg_coul = -power_platform_get_battery_current();
		ichg_set_after = charge_get_charge_current();
		warm_current = (WARM_CUR_RATIO * coul_drv_battery_fcc_design()) / RATIO_BASE;
		retry_times++;

		if ((ichg_set_before == ichg_set_after) && (ichg_set_after > warm_current) &&
			(temp > COOL_LIMIT) && (temp < WARM_LIMIT) &&
			(ichg_coul > CHARGE_CURRENT_0000_MA)) {
			if (ichg_coul >= (IBIS_RATIO * ichg_set_after) / RATIO_BASE)
				err_flag++;
			if (err_flag == IBIAS_RETRY_TIMES)
				hwlog_err("check_ibas_current_safe ichg_coul = %d ichg_set_after = %d warm_current = %d retry_times =%d\n",
					ichg_coul, ichg_set_after, warm_current, retry_times);
		}
	} while ((err_flag == retry_times) && (retry_times < IBIAS_RETRY_TIMES));
}

#define SW_RECHARGE_TH                      96
#define DISCHARGE_CC_TH                     4
#define RECHARGE_VOL                        60
#define RECHARGE_TIMES                      10
#define OCV_REFRESH_MIN                     (-5)
#define OCV_REFRESH_MAX                     5
#define RECHARGE_DMD_BUFF_SIZE              256

static bool charge_is_night_time(struct charge_device_info *di)
{
	if (!di || !di->recharge_para[RECHARGE_NT_SWITCH])
		return false;

	/* night time: 2:00-5:30 */
	return power_is_within_time_interval(2, 0, 5, 30);
}

static void charge_recharge_dmd_report(struct charge_device_info *di)
{
	struct rtc_time tm0 = { 0 };
	struct rtc_time tm1 = { 0 };
	char tmp_buff[RECHARGE_DMD_BUFF_SIZE] = { 0 };

	if (!di || !di->recharge_para[RECHARGE_DMD_SWITCH])
		return;

	power_get_local_time(&di->charge_start_time, &tm0);
	power_get_local_time(&di->charge_done_time, &tm1);
	snprintf(tmp_buff, sizeof(tmp_buff),
		"charge start time is %04d-%02d-%02d %02d:%02d:%02d, charge done time is %04d-%02d-%02d %02d:%02d:%02d",
		tm0.tm_year + 1900, tm0.tm_mon + 1, tm0.tm_mday,
		tm0.tm_hour, tm0.tm_min, tm0.tm_sec,
		tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday,
		tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	hwlog_info("%s %s\n", __func__, tmp_buff);
	power_dsm_report_dmd(POWER_DSM_BATTERY, POWER_DSM_SOFT_RECHARGE_NO, tmp_buff);
}

static int discharge_cc_calc(int reset, int cc)
{
	static int total;
	static int cc_last;

	if (reset) {
		cc_last = 0;
		total = 0;
		return 0;
	}

	if (cc > cc_last) {
		total += (cc - cc_last);
		cc_last = cc;
		return total;
	}

	if ((cc < cc_last) && (cc < OCV_REFRESH_MAX) &&
		(cc > OCV_REFRESH_MIN)) {
		cc_last = cc;
		return total;
	}
	/* cc invalid */
	return total;
}

static void charger_software_recharge(struct charge_device_info *di)
{
	int ret;

	g_recharge_count++;

	ret = charge_set_charge_enable(FALSE);
	/* sleep 100ms for chip diable and enable */
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
	ret += charge_set_charge_enable(TRUE & charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ENABLE_CHARGER));
	if (ret)
		hwlog_err("[%s]set charge enable fail\n", __func__);

	charge_recharge_dmd_report(di);
	hwlog_info("software recharge %d\n", g_recharge_count);
}

static void charger_smart_battery_recharge_check(struct charge_device_info *di, unsigned int state)
{
	bool last_charge_done_state = di->smart_battery_chargedone;
	bool cur_charge_done_state = !!(state & CHAGRE_STATE_CHRG_DONE);

	if (last_charge_done_state && !cur_charge_done_state)
		charger_software_recharge(di);

	di->smart_battery_chargedone = cur_charge_done_state;
}

static void charge_recharge_check(struct charge_device_info *di)
{
	int ret;
	int unfilter_soc;
	unsigned int state = CHAGRE_STATE_NORMAL;
	static bool first_done;
	static int cv_set, cv_real; /* mv */
	int vbat, cv, discharge_cc; /* mv, mv, mah */
	int cc, fcc;

	if (!g_sw_recharge_dts || !di->charge_enable ||
		di->core_data->warm_triggered ||
		!charge_ops_valid(get_terminal_voltage) || !power_platform_is_battery_exit())
		return;

	if (g_recharge_count >= di->recharge_para[RECHARGE_TIMES_TH])
		return;

	if (charge_is_night_time(di)) {
		hwlog_info("now is night time, not do soft recharge\n");
		return;
	}

	ret = charge_get_charging_state(&state);
	if (ret < 0) {
		hwlog_err("get_charge_state fail, ret = 0x%x\n", ret);
		return;
	}

	if (coul_drv_is_smart_battery())
		return charger_smart_battery_recharge_check(di, state);

	if (state & CHAGRE_STATE_CHRG_DONE) {
		vbat = coul_drv_battery_voltage();
		cv = charge_get_terminal_voltage();
		unfilter_soc = coul_drv_battery_unfiltered_capacity();
		cc = coul_drv_battery_cc();
		fcc = coul_drv_battery_fcc();

		hwlog_info("[%s] vbat %d, cv %d, uf_soc %d, cc %d, fcc %d\n",
			__func__, vbat, cv, unfilter_soc, cc, fcc);

		if (!first_done) {
			first_done = true;
			cv_real = vbat;
			cv_set = cv;
			discharge_cc_calc(1, 0);
			hwlog_info("[%s] first done\n", __func__);
			return;
		}

		if (cv_set != cv) {
			first_done = false;
			return;
		}

		if ((unfilter_soc > (int)di->recharge_para[RECHARGE_SOC_TH]) ||
			(cv_set - vbat < (int)di->recharge_para[RECHARGE_VOL_TH]) ||
			(cv_real - vbat < (int)di->recharge_para[RECHARGE_VOL_TH]))
			return;

		discharge_cc = discharge_cc_calc(0, cc);
		/* 100 is full soc */
		if (fcc && ((discharge_cc * 100 / fcc) <
			(int)di->recharge_para[RECHARGE_CC_TH]))
			return;

		charger_software_recharge(di);
		hwlog_info("[%s]cv %d,set %d,dis_cc %d,software recharge %d\n",
			__func__, cv_real, cv_set, discharge_cc, g_recharge_count);
	} else {
		first_done = false;
	}
}

static int charge_check_warm_status(struct charge_device_info *di,
	unsigned int state, unsigned int *events)
{
	static bool last_warm_triggered;
	int ret;

	charge_sysfs_set_sysfs_value(CHARGE_SYSFS_CHARGE_DONE_STATUS, CHARGE_DONE_NON);
	if (last_warm_triggered ^ di->core_data->warm_triggered) {
		last_warm_triggered = di->core_data->warm_triggered;
		ret = charge_set_charge_enable(FALSE);
		(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
		ret |= charge_set_charge_enable(TRUE &
			charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ENABLE_CHARGER));
		if (ret)
			hwlog_err("[%s]set charge enable fail\n", __func__);
		hwlog_info("warm status changed, resume charging\n");
		return -1;
	}

	if (di->enable_current_full) {
		if (!state && is_charge_current_full(di) && !di->current_full_status) {
			*events =  VCHRG_CURRENT_FULL_EVENT;
			charge_send_uevent(*events);
			di->current_full_status = 1;
		}
	}

	return 0;
}

static int charge_check_charge_done(struct charge_device_info *di,
	unsigned int state, unsigned int *events)
{
	int ret;

	/* check status charge done, ac charge and usb charge */
	if (di->charge_enable && power_platform_is_battery_exit()) {
		ret = charge_check_warm_status(di, state, events);
		if (ret < 0)
			return ret;
		if ((state & CHAGRE_STATE_CHRG_DONE) && !di->core_data->warm_triggered) {
			if (charge_get_charger_type() == CHARGER_REMOVED)
				return -1;

			*events = VCHRG_CHARGE_DONE_EVENT;
			hwlog_info("VCHRG_CHARGE_DONE_EVENT\n");
			charge_sysfs_set_sysfs_value(CHARGE_SYSFS_CHARGE_DONE_STATUS, CHARGE_DONE);
			power_event_bnc_notify(POWER_BNT_CHG, POWER_NE_CHG_CHARGING_DONE, NULL);
			di->vterm_dec_flag = FALSE;
			/*
			 * charge done sleep has been configured as enable and battery is full
			 * then allow phone to sleep
			 */
			if (((charge_sysfs_get_sysfs_value(CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS) ==
				CHARGE_DONE_SLEEP_ENABLED) || charge_done_sleep_dts) &&
				(bci_show_capacity() == CAPACITY_FULL)) {
				if ((charge_get_charger_type() == CHARGER_TYPE_STANDARD) ||
					(charge_get_charger_type() == CHARGER_TYPE_NON_STANDARD) ||
					(charge_get_charger_type() == CHARGER_TYPE_FCP) ||
					(charge_get_charger_type() == CHARGER_TYPE_PD)) {
					power_event_bnc_notify(POWER_BNT_CHG, POWER_NE_CHG_WAKE_UNLOCK, NULL);
					mutex_lock(&charge_wakelock_flag_lock);
					charge_set_wakelock_flag(CHARGE_WAKELOCK_NEED);
					power_wakeup_unlock(charge_lock, false);
					mutex_unlock(&charge_wakelock_flag_lock);
					hwlog_info("charge wake unlock while charging done\n");
				}
			}
		} else if (charge_get_charger_source() == POWER_SUPPLY_TYPE_MAINS) {
			*events = VCHRG_START_AC_CHARGING_EVENT;
		} else if (charge_get_charger_source() == POWER_SUPPLY_TYPE_BATTERY) {
			*events = VCHRG_NOT_CHARGING_EVENT;
			hwlog_info("VCHRG_NOT_CHARGING_EVENT, power_supply: BATTERY\n");
		} else {
			*events = VCHRG_START_USB_CHARGING_EVENT;
		}
	} else {
		*events = VCHRG_NOT_CHARGING_EVENT;
		hwlog_info("VCHRG_NOT_CHARGING_EVENT\n");
	}
	return 0;
}

static void charge_update_status(struct charge_device_info *di)
{
	enum charge_status_event events = VCHRG_POWER_NONE_EVENT;
	unsigned int state = CHAGRE_STATE_NORMAL;
	int ret;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;
#ifdef CONFIG_DIRECT_CHARGER
	if (charge_need_ignore_plug_event())
		return;
#endif
	ret = charge_get_charging_state(&state);
	if (ret < 0) {
		hwlog_err("get_charge_state fail ret = 0x%x\n", ret);
		return;
	}

	if (charge_ops_valid(get_ibus)) {
		di->avg_iin_ma = calc_avg_input_current_ma(di, charge_get_ibus());
		if (((di->max_iin_ma < di->avg_iin_ma) || (di->max_iin_ma == IMPOSSIBLE_IIN)) &&
			(di->avg_iin_ma != IMPOSSIBLE_IIN))
			di->max_iin_ma = di->avg_iin_ma;
	}

	/* check status charger not power good state */
	charge_state_check_power_good(state, &events);
	charge_state_check(state, &events);
	ret = charge_check_charge_done(di, state, &events);
	if (ret < 0)
		return;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	if (charge_get_charger_type() == CHARGER_REMOVED)
		return;

	charge_send_uevent(events);
}

static void charge_select_terminal_voltage(struct charge_device_info *di)
{
	unsigned int vterm = 0;
	int increase_volt;
	int ret;

	/* set CV terminal voltage */
	if (power_cmdline_is_factory_mode() && !power_platform_is_battery_exit()) {
		vterm = coul_drv_battery_vbat_max();
		hwlog_info("facory_version and battery not exist, vterm is set to %d\n", vterm);
	} else {
		vterm = power_min_positive(di->core_data->vterm,
			charge_sysfs_get_sysfs_value(CHARGE_SYSFS_REGULATION_VOLTAGE));
	}

	increase_volt = charge_get_incr_term_volt(di);
	vterm += increase_volt;
	if (di->vterm_dec_flag) {
		vterm -= di->term_err_vterm_dec;
		hwlog_info("vterm_dec_flag is set, vterm is set to %d\n", vterm);
	}

	hwlog_info("set vterm %d, increase volt=%d\n", vterm, increase_volt);
	ret = charge_set_terminal_voltage(vterm);
	if (ret > 0) {
		hwlog_info("terminal voltage is out of range:%dmV\n", ret);
		charge_set_terminal_voltage(ret);
	} else if (ret < 0) {
		hwlog_err("set terminal voltage fail\n");
	}
}

static void charge_turn_on_charging(struct charge_device_info *di)
{
	int ret;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	di->charge_enable = TRUE;
	/* check vbus voltage ,if vbus is abnormal disable charge or abort from fcp */
	charge_vbus_voltage_check(di);
	/* set input current */
#ifdef CONFIG_HISI_ASW
	if (asw_get_iin_limit() == ASW_PROTECT_IIN_LIMIT)
		charge_set_input_current(ASW_PROTECT_IIN_LIMIT);
	else
#endif /* CONFIG_HISI_ASW */
		charge_set_input_current(di->input_current);

	if (di->smart_charge_support && coul_drv_is_smart_battery()) {
		/* smart battery charge: get cc and cv from battery */
		hwlog_info("in smart battery charge state\n");
	} else if (di->charge_current == CHARGE_CURRENT_0000_MA) {
		di->charge_enable = FALSE;
		hwlog_info("charge current is set 0mA, turn off charging\n");
	} else {
		/* set CC charge current */
		ret = charge_set_charge_current(di->charge_current);
		if (ret > 0) {
			hwlog_info("charge current is out of range:%dmA\n", ret);
			charge_set_charge_current(ret);
		} else if (ret < 0) {
			hwlog_err("set charge current fail\n");
		}
		charge_select_terminal_voltage(di);
	}
	/* enable/disable charge */
	di->charge_enable &= charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ENABLE_CHARGER);
	di->charge_enable &= term_err_chrg_en_flag;
	ret = charge_set_charge_enable(di->charge_enable);
	if (!charge_sysfs_get_sysfs_value(CHARGE_SYSFS_ENABLE_CHARGER))
		hwlog_info("Disable flags: sysnode = %d, isc = %d",
			di->disable_charger[CHARGER_SYS_NODE],
			di->disable_charger[CHARGER_FATAL_ISC_TYPE]);
	if (ret)
		hwlog_err("set charge enable fail\n");
	hwlog_debug("input_current is %d,charge_current is %d,charge_enable is %d\n",
		di->input_current, di->charge_current, di->charge_enable);
}

static void charge_safe_protect(struct charge_device_info *di)
{
	if (!di) {
		hwlog_err("%s:NULL pointer\n", __func__);
		return;
	}
	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;
	/* do soft ovp protect for 5V/9V charge */
	if (charge_ops_valid(soft_vbatt_ovp_protect))
		charge_soft_vbatt_ovp_protect();
	check_ibias_current_safe(di);
}

#ifdef CONFIG_DIRECT_CHARGER
static int charger_disable_usbpd(bool disable)
{
	bool dfg_online = power_platform_check_online_status();
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("%s:NULL pointer\n", __func__);
		return -EPERM;
	}

	(void)charge_set_vbus_vset(ADAPTER_5V);

	adapter_set_usbpd_enable(ADAPTER_PROTOCOL_SCP, !disable, dfg_online ? true : false);
	pd_dpm_disable_pd(disable);
	hwlog_info("%s\n", __func__);
	return 0;
}

static void charger_switch_type_to_standard(struct charge_device_info *di)
{
	if (!di) {
		hwlog_err("%s:NULL pointer\n", __func__);
		return;
	}

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	pd_charge_flag = false;
	pd_vbus_abnormal_cnt = 0;
	charge_set_charger_type(CHARGER_TYPE_STANDARD);
	charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
	if (di->support_standard_ico == 1)
		ico_enable = 1;
	else
		ico_enable = 0;
	hwlog_info("%s CHARGER_TYPE_STANDARD\n", __func__);
}

static void charger_try_pd2scp(struct charge_device_info *di)
{
	int time = EAMRK_WAIT_TIME;

	if (pd_dpm_support_cable_auth())
		time = EMARK_WITH_CABLE_AUTH;

	if (!pd_dpm_get_ctc_cable_flag())
		return;

	if (try_pd_to_scp_counter <= 0)
		return;

	chg_wait_pd_init = false;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP) {
		try_pd_to_scp_counter = 0;
		return;
	}

	hwlog_info("%s try_pd_to_scp\n", __func__);
	if (direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE) {
		hwlog_info("ignore pre_check\n");
		charger_switch_type_to_standard(di);
		try_pd_to_scp_counter = 0;
	} else if (!direct_charge_pre_check()) {
		if (adapter_detect_get_runtime_protocol_type() == BIT(ADAPTER_PROTOCOL_UFCS)) {
				hw_pd_set_hv_en(false);
				time = (time > EMARK_WAIT_TIME_UFCS) ? time : EMARK_WAIT_TIME_UFCS;
		}
		if (pd_dpm_get_emark_detect_enable()) {
			pd_dpm_detect_emark_cable();
			/* wait time to get cable vdo */
			if (!wait_for_completion_timeout(
				&emark_detect_comp,
				msecs_to_jiffies(time)))
				hwlog_info("emark timeout\n");

			reinit_completion(&emark_detect_comp);
			pd_dpm_detect_emark_cable_finish();
		}

		if (direct_charge_in_charging_stage() == DC_NOT_IN_CHARGE_DONE_STAGE &&
			adapter_detect_get_runtime_protocol_type() == BIT(ADAPTER_PROTOCOL_SCP))
			charger_disable_usbpd(true);
		charger_switch_type_to_standard(di);
		try_pd_to_scp_counter = 0;
	} else {
		try_pd_to_scp_counter--;
	}

	if (try_pd_to_scp_counter < 0)
		try_pd_to_scp_counter = 0;
}

static void charger_direct_charge_check(struct charge_device_info *di,
	unsigned int type)
{
	if (!di)
		return;

#ifdef CONFIG_POGO_PIN
	if (pogopin_typec_chg_ana_audio_support() && !pogopin_5pin_get_fcp_support()) {
		hwlog_err("pogopin not support fcp\n");
		return;
	}
#endif /* CONFIG_POGO_PIN */

#ifndef CONFIG_HLTHERM_RUNTEST
	if (!di->charge_enable) {
		hwlog_err("enable false,no in direct check\n");
		return;
	}
#endif

	if (type != CHARGER_TYPE_STANDARD)
		return;

	if (chg_wait_pd_init)
		return;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	if (di->need_filter_pd_event &&
		(pd_dpm_get_pd_event_num() < PD_EVENT_NUM_TH1)) {
		hwlog_info("pd_event_num=%d\n", pd_dpm_get_pd_event_num());
		direct_charge_check();
	}

	if (!di->need_filter_pd_event)
		direct_charge_check();
}
#else
static inline void charger_try_pd2scp(struct charge_device_info *di)
{
}

static inline void charger_direct_charge_check(struct charge_device_info *di,
	unsigned int type)
{
}
#endif

static void charger_switch_type_to_fcp(struct charge_device_info *di)
{
	if (!di) {
		hwlog_err("%s:NULL pointer\n", __func__);
		return;
	}

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
		return;

	pd_charge_flag = false;
	pd_vbus_abnormal_cnt = 0;
	charge_set_charger_type(CHARGER_TYPE_FCP);
	charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
	if (di->support_standard_ico == 1)
		ico_enable = 1;
	else
		ico_enable = 0;
	hwlog_info("%s CHARGER_TYPE_FCP\n", __func__);
}

static void charger_try_pd2fcp(struct charge_device_info *di)
{
	if (!pd_dpm_get_ctc_cable_flag())
		return;

	if ((try_pd_to_fcp_counter <= 0) || !power_platform_check_online_status() ||
		(charge_get_charger_type() == CHARGER_TYPE_STANDARD))
		return;

	chg_wait_pd_init = false;

	if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP) {
		try_pd_to_fcp_counter = 0;
		return;
	}

	hwlog_info("%s try_pd_to_fcp\n", __func__);
	if (hvdcp_get_charging_stage() == HVDCP_STAGE_SUCCESS) {
		hwlog_info("ignore check fcp\n");
		charger_switch_type_to_fcp(di);
		try_pd_to_fcp_counter = 0;
	} else if (hvdcp_detect_adapter()== ADAPTER_DETECT_SUCC) {
		if (pd_dpm_get_emark_detect_enable()) {
			pd_dpm_detect_emark_cable();
			/* wait 200ms to get cable vdo */
			if (!wait_for_completion_timeout(
				&emark_detect_comp,
				msecs_to_jiffies(200)))
				hwlog_info("emark timeout\n");

			reinit_completion(&emark_detect_comp);
			pd_dpm_detect_emark_cable_finish();
		}

		charger_disable_usbpd(true);
		msleep(800); /* Wait 800ms for BC1.2 complete */
		charger_switch_type_to_fcp(di);
		try_pd_to_fcp_counter = 0;
	} else {
		try_pd_to_fcp_counter--;
	}

	if (try_pd_to_fcp_counter < 0)
		try_pd_to_fcp_counter = 0;
}

static void set_charge_work_interval(struct charge_device_info *di)
{
	unsigned int fast_time = charge_get_quicken_work_flag();

#ifndef CONFIG_DIRECT_CHARGER
	di->monitor_interval = CHARGING_WORK_TIMEOUT;
	return;
#endif

	if (try_pd_to_scp_counter > 0)
		di->monitor_interval = CHARGING_WORK_PDTOSCP_TIMEOUT;
	else if (chg_wait_pd_init)
		di->monitor_interval = CHARGING_WORK_WAITPD_TIMEOUT;
	else if (di->need_filter_pd_event &&
		(pd_dpm_get_pd_event_num() >= PD_EVENT_NUM_TH1) &&
		(pd_dpm_get_pd_event_num() < PD_EVENT_NUM_TH2))
		di->monitor_interval = CHARGING_WORK_TIMEOUT1;
	else if (fast_time && (fast_time <= CHARGING_WORK_FAST_TIMES))
		di->monitor_interval = fast_time * SECONDS_TO_MILLISECONDS;
	else
		di->monitor_interval = CHARGING_WORK_TIMEOUT;

	hwlog_info("monitor_interval=%d\n", di->monitor_interval);
}

static void charge_monitor_handler(struct charge_device_info *di)
{
	if (!di)
		return;

	di->core_data = charge_core_get_params();
	if (!di->core_data) {
		hwlog_err("charge core data is null\n");
		return;
	}

	charge_select_charging_current(di);
	charge_turn_on_charging(di);
	charge_safe_protect(di);
	charge_full_handle(di);
	charge_update_vindpm(di);
	charge_update_external_setting(di);
	charge_recharge_check(di);
	charge_update_status(di);
	charge_kick_watchdog();
}

static void charge_monitor_work(struct work_struct *work)
{
	struct charge_device_info *di = container_of(work, struct charge_device_info,
		charge_work.work);
	unsigned int type;

	if (chg_wait_pd_init) {
		/* wait a moment for PD init and detect PD adaptor */
		if (power_get_monotonic_boottime() - boot_time > CHG_WAIT_PD_TIME)
			chg_wait_pd_init = false;
	}
	charge_kick_watchdog();

	/* if support both SCP and PD, try to control adapter by SCP */
	charger_try_pd2scp(di);

	/* For uem:if support both FCP and PD, try to control adapter by FCP */
	charger_try_pd2fcp(di);

	type = charge_get_charger_type();

	if (direct_charge_get_stop_charging_complete_flag()) {
		/* update type before get params */
		charge_update_charger_type(di);
		type = charge_get_charger_type();
#ifdef CONFIG_HUAWEI_TYPEC
		/* only the typec is supported, we need read typec result */
		if (di->core_data->typec_support)
			di->typec_current_mode = typec_detect_current_mode();
#endif
		charger_direct_charge_check(di, type);
	}

	if (!direct_charge_get_stop_charging_complete_flag())
		goto reschedule_work;

	if (uvdm_charge_in_charging_stage() == UVDM_NOT_IN_CHARGING_STAGE)
		uvdm_pre_check();

	if (uvdm_get_sc_enable())
		goto reschedule_work;

	uvdm_handle_cover_report_event();
	pd_charge_check(di);

	if (di->need_filter_pd_event) {
		if (pd_dpm_get_pd_event_num() < PD_EVENT_NUM_TH1) {
			hwlog_info("pd_event_num=%d\n",
				pd_dpm_get_pd_event_num());
			fcp_charge_check(di, type);
		}
	} else {
		fcp_charge_check(di, type);
	}

	charge_monitor_handler(di);

reschedule_work:
	set_charge_work_interval(di);
	queue_delayed_work(system_power_efficient_wq, &di->charge_work,
		msecs_to_jiffies(di->monitor_interval));
}

void water_detection_entrance(void)
{
#ifdef CONFIG_HUAWEI_WATER_CHECK
	power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_DETECT_BY_USB_GPIO, NULL);
#endif /* CONFIG_HUAWEI_WATER_CHECK */
	power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_DETECT_BY_USB_ID, NULL);
	power_event_bnc_notify(POWER_BNT_WD, POWER_NE_WD_DETECT_BY_AUDIO_DP_DN, NULL);
}

static void charge_usb_work(struct work_struct *work)
{
	struct charge_device_info *di = container_of(work, struct charge_device_info, usb_work);

#ifdef CONFIG_TCPC_CLASS
	if (charger_pd_support)
		mutex_lock(&di->tcpc_otg_lock);
#endif

	water_detection_entrance();

	switch (charge_get_charger_type()) {
	case CHARGER_TYPE_USB:
		hwlog_info("case = CHARGER_TYPE_USB->\n");
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_CONNECT, NULL);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_NON_STANDARD:
		hwlog_info("case = CHARGER_TYPE_NON_STANDARD ->\n");
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_CONNECT, NULL);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_BC_USB:
		hwlog_info("case = CHARGER_TYPE_BC_USB ->\n");
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_CONNECT, NULL);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_STANDARD:
		hwlog_info("case = CHARGER_TYPE_STANDARD->\n");
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_CONNECT, NULL);
		charge_start_charging(di);
		break;
	case CHARGER_REMOVED:
		hwlog_info("case = USB_EVENT_NONE->\n");
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_DISCONNECT, NULL);
		charge_stop_charging(di);
		break;
	case USB_EVENT_OTG_ID:
		hwlog_info("case = USB_EVENT_OTG_ID->\n");
		charge_otg_mode_enable(CHARGE_OTG_ENABLE, VBUS_CH_USER_WIRED_OTG);
		break;
	case CHARGER_TYPE_PD:
		hwlog_info("case = CHARGER_TYPE_PD->\n");
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_CONNECT, NULL);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_WIRELESS:
		hwlog_info("case = CHARGER_TYPE_WIRELESS->\n");
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_WIRELESS_CONNECT, NULL);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_POGOPIN:
		hwlog_info("case = CHARGER_TYPE_POGOPIN->\n");
		power_event_bnc_notify(POWER_BNT_CONNECT, POWER_NE_USB_CONNECT, NULL);
		charge_start_charging(di);
		break;
	default:
		break;
	}
#ifdef CONFIG_TCPC_CLASS
	if (charger_pd_support)
		mutex_unlock(&di->tcpc_otg_lock);
#endif
}

void huawei_charger_process_vr_charge_event(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return;
	}

	charge_stop_charging(di);
	power_wakeup_lock(charge_lock, false);

	switch (charge_sysfs_get_sysfs_value(CHARGE_SYSFS_VR_CHARGER_TYPE)) {
	case CHARGER_TYPE_SDP:
		charge_set_charger_type(CHARGER_TYPE_USB);
		charge_set_charger_source(POWER_SUPPLY_TYPE_USB);
		charge_send_uevent(NO_EVENT);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_CDP:
		charge_set_charger_type(CHARGER_TYPE_BC_USB);
		charge_set_charger_source(POWER_SUPPLY_TYPE_USB);
		charge_send_uevent(NO_EVENT);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_DCP:
		charge_set_charger_type(CHARGER_TYPE_VR);
		charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		charge_send_uevent(NO_EVENT);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_UNKNOWN:
		charge_set_charger_type(CHARGER_TYPE_NON_STANDARD);
		charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		charge_send_uevent(NO_EVENT);
		charge_start_charging(di);
		break;
	case CHARGER_TYPE_NONE:
		charge_set_charger_type(USB_EVENT_OTG_ID);
		charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
		charge_send_uevent(NO_EVENT);
		power_wakeup_unlock(charge_lock, false);
		charge_otg_mode_enable(CHARGE_OTG_ENABLE, VBUS_CH_USER_WIRED_OTG);
		break;
	default:
		hwlog_info("Invalid vr charger type! vr_charge_type = %d\n",
			charge_sysfs_get_sysfs_value(CHARGE_SYSFS_VR_CHARGER_TYPE));
		break;
	}
}

static void charge_resume_wakelock_work(struct work_struct *work)
{
	mutex_lock(&charge_wakelock_flag_lock);
	if (charge_get_wakelock_flag() == CHARGE_WAKELOCK_NEED) {
		power_wakeup_lock(charge_lock, false);
		hwlog_info("charge wake lock when resume during charging\n");
	}
	mutex_unlock(&charge_wakelock_flag_lock);
}

static void uscp_plugout_send_uevent(struct work_struct *work)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("[%s]di is NULL\n", __func__);
		return;
	}
	charge_send_uevent(NO_EVENT);
	power_wakeup_unlock(uscp_plugout_lock, false);
}

#ifdef CONFIG_TCPC_CLASS
int is_pd_supported(void)
{
	return charger_pd_support;
}
#endif

static void charge_uevent_process(struct charge_device_info *di, unsigned long event)
{
#ifdef CONFIG_DIRECT_CHARGER
	if (!charge_need_ignore_plug_event() && !direct_charge_get_cutoff_normal_flag()) {
#endif

	if (uscp_is_in_protect_mode() && (charge_get_charger_type() == CHARGER_REMOVED)) {
		power_wakeup_lock(uscp_plugout_lock, false);
		schedule_delayed_work(&di->plugout_uscp_work, msecs_to_jiffies(WORK_DELAY_5000MS));
	} else {
		charge_send_uevent(NO_EVENT);
	}

#ifdef CONFIG_DIRECT_CHARGER
	} else {
		hwlog_info("%s ignore evnet : %ld\n", __func__, event);
	}
#endif
}

#ifdef CONFIG_POGO_PIN
static int pogopin_charger_vbus_notifier_call(struct notifier_block *pogopin_nb,
	unsigned long event, void *data)
{
	struct charge_device_info *di = NULL;
	int ret;

	if (!pogopin_nb) {
		hwlog_err("pogopin_nb null\n");
		return NOTIFY_OK;
	}

	di = container_of(pogopin_nb, struct charge_device_info, pogopin_nb);
	if (!di) {
		hwlog_err("pogo charge di null\n");
		return NOTIFY_OK;
	}

	hwlog_info("pogopin_charger_vbus_notifier_call event =%ld\n", event);

	if (event == POGOPIN_CHARGER_INSERT) {
		if ((charge_get_charger_type() == CHARGER_REMOVED) ||
			(charge_get_charger_type() == USB_EVENT_OTG_ID)) {
			charge_set_charger_type(CHARGER_TYPE_POGOPIN);
			charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		} else {
			if (pogopin_get_interface_status() ==
				POGOPIN_INTERFACE) {
				charge_set_charger_type(CHARGER_TYPE_POGOPIN);
				charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
			} else {
				/* now is through typec charging return */
				hwlog_info("typec charging,ignore pogopin\n");
				return NOTIFY_OK;
			}
		}
	} else if (event == POGOPIN_CHARGER_REMOVE) {
		/* must fron pogopin connect to disconnect */
		if (charge_get_charger_type() == CHARGER_TYPE_POGOPIN) {
			charge_set_charger_type(CHARGER_REMOVED);
			charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
		} else {
			hwlog_info("now not is pogopin charger,return\n");
			return NOTIFY_OK;
		}
	}

	charge_uevent_process(di, event);

	ret = schedule_work(&di->usb_work);
	if (!ret)
		hwlog_err("usb work state ret = %d\n", ret);

	return NOTIFY_OK;
}

static int pogopin_otg_typec_charge_notifier_call(struct notifier_block *usb_nb,
	unsigned long event_typec, void *data)
{
	if (!g_di) {
		hwlog_err("g_di is null\n");
		return NOTIFY_OK;
	}

	hwlog_info("%s event_typec:%d chg_type:%u\n",
		__func__, event_typec, charge_get_charger_type());
	if (event_typec == POGOPIN_OTG_AND_CHG_START) {
		/* set charger insert flow */
		if (charge_get_monitor_work_flag() == CHARGE_MONITOR_WORK_NEED_STOP)
			cancel_delayed_work_sync(&g_di->charge_work);
		/* set chg type to pd when pogo insert otg then typec insert pd */
		if (charge_get_charger_type() == CHARGER_TYPE_PD)
			hwlog_info("current charger type:CHARGER_TYPE_PD");
		/* set chg type to standard in other cases */
		else
			charge_set_charger_type(CHARGER_TYPE_STANDARD);
		charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		charge_send_uevent(charge_get_charger_type());
		charge_start_charging(g_di);
	} else if (event_typec == POGOPIN_OTG_AND_CHG_STOP) {
		/* set charger removed flow */
		charge_set_charger_type(CHARGER_REMOVED);
		charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
		charge_stop_charging(g_di);
		charge_send_uevent(NO_EVENT);
	}

	return NOTIFY_OK;
}
#endif /* CONFIG_POGO_PIN */

void charger_vbus_init_handler(int vbus)
{
	struct charge_device_info *di = g_di;

	if (!di)
		return;

	if (vbus >= ADAPTER_12V * POWER_MV_PER_V)
		g_init_crit.vbus = ADAPTER_12V;
	else if (vbus >= ADAPTER_9V * POWER_MV_PER_V)
		g_init_crit.vbus = ADAPTER_9V;
	else
		g_init_crit.vbus = ADAPTER_5V;
	g_init_crit.charger_type = charge_get_charger_type();
	hwlog_info("[vbus_init_handler] charger_type:%u, vbus:%dmV\n",
		charge_get_charger_type(), vbus);
	(void)charge_init_chip(&g_init_crit);
	(void)charge_set_vbus_vset(g_init_crit.vbus);

	mod_delayed_work(system_power_efficient_wq, &di->charge_work, 0);
}

#ifdef CONFIG_WIRELESS_CHARGER
static int wireless_charger_vbus_notifier_call(struct notifier_block *wireless_nb,
	unsigned long event, void *data)
{
	struct charge_device_info *di = container_of(wireless_nb,
		struct charge_device_info, wireless_nb);

	if (!di)
		return NOTIFY_OK;

	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_OFF) {
		hwlog_info("%s not in wireless charging\n", __func__);
		return NOTIFY_OK;
	}

	switch (event) {
	case POWER_NE_WLC_CHARGER_VBUS:
		if (data) {
			di->wireless_vbus = *(int *)data;
			charger_vbus_init_handler(di->wireless_vbus);
		}
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}
#endif

static int charge_usb_notifier_call(struct notifier_block *usb_nb,
	unsigned long event, void *data)
{
	struct charge_device_info *di = container_of(usb_nb, struct charge_device_info, usb_nb);
	int filter_flag;
	int ret;

	charger_type_ever_notify = true;
	power_wakeup_lock(charge_lock, false);

#ifdef CONFIG_HUAWEI_YCABLE
	if (ycable_is_support() && ycable_is_with_charger() && (event == PLEASE_PROVIDE_POWER)) {
		hwlog_info("ycable mode with charger,should not start otg\n");
		return NOTIFY_OK;
	}
#endif

	filter_flag = charge_rename_charger_type(event, di, FALSE);
	if (filter_flag) {
		hwlog_info("not use work,filter_flag=%d\n", filter_flag);
		return NOTIFY_OK;
	}

	charge_uevent_process(di, event);

	ret = schedule_work(&di->usb_work);
	if (!ret)
		hwlog_err("usb work state ret = %d\n", ret);
	return NOTIFY_OK;
}

#ifdef CONFIG_HUAWEI_YCABLE
static int charge_ycable_notifier_call(struct notifier_block *ycable_nb,
	unsigned long event, void *data)
{
	struct charge_device_info *di = NULL;
	int filter_flag;
	int ret;

	if (!ycable_nb) {
		hwlog_err("error:ycable_nb is null\n");
		return NOTIFY_OK;
	}

	di = container_of(ycable_nb, struct charge_device_info, ycable_nb);
	if (!di) {
		hwlog_err("error:ycable charger info is null\n");
		return NOTIFY_OK;
	}

	charger_type_ever_notify = true;
	power_wakeup_lock(charge_lock, false);

	hwlog_info("charge_ycable_notifier_call event = %ld\n", event);

	filter_flag = charge_rename_charger_type(event, di, FALSE);
	if (filter_flag) {
		hwlog_info("not use work,filter_flag=%d\n", filter_flag);
		return NOTIFY_OK;
	}

	charge_uevent_process(di, event);

	ret = schedule_work(&di->usb_work);
	if (!ret)
		hwlog_err("error:usb work state ret = %d\n", ret);

	return NOTIFY_OK;
}
#endif

static int huawei_get_charge_current_max(void)
{
	struct charge_device_info *di = g_di;

	if (!di) {
		hwlog_err("[%s]di is NULL\n", __func__);
		return 0;
	}
	return di->charge_current;
}

int set_otg_switch_mode_enable(void)
{
	return vbus_ch_set_switch_mode(VBUS_CH_USER_WIRED_OTG,
		VBUS_CH_TYPE_CHARGER, 1);
}
EXPORT_SYMBOL(set_otg_switch_mode_enable);

int huawei_charger_get_dieid(char *dieid, unsigned int len)
{
	if (charge_ops_valid(get_dieid))
		return charge_get_dieid(dieid, len);

	return -1;
}
EXPORT_SYMBOL(huawei_charger_get_dieid);

#ifdef CONFIG_TCPC_CLASS

static void charge_rename_pd_charger_type(unsigned long type,
	struct charge_device_info *di, bool ext_power)
{
	switch (type) {
	case PD_DPM_VBUS_TYPE_PD:
		charge_set_charger_type(CHARGER_TYPE_PD);
		hwlog_info("%s is ext_power %d", __func__, ext_power);
		if (ext_power)
			charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		else
			charge_set_charger_source(POWER_SUPPLY_TYPE_USB);
		break;
	case PD_DPM_VBUS_TYPE_TYPEC:
		charge_set_charger_type(CHARGER_TYPE_TYPEC);
		charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		break;
	default:
		charge_set_charger_type(CHARGER_REMOVED);
		charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);
		hwlog_info("%s default type %ld", __func__, type);
		break;
	}
}

static void pd_typec_current_handler(void)
{
	struct charge_device_info *di = g_di;

	if (!di)
		return;
	charge_select_charging_current(di);
	hwlog_info("%s input_current %u", __func__, di->input_current);
	charge_set_input_current(di->input_current);
}

static void charger_type_handler(unsigned long type, void *data)
{
	struct charge_device_info *di = g_di;
	struct pd_dpm_vbus_state *vbus_state = NULL;
	int need_resched_work = 0;
	unsigned int last_type;
	unsigned int charger_type = charge_get_charger_type();

	if (!di)
		return;
	switch (type) {
	case CHARGER_TYPE_SDP:
		if ((charger_type == CHARGER_TYPE_USB) ||
			(charger_type == CHARGER_TYPE_WIRELESS) ||
			(charger_type == CHARGER_REMOVED)) {
			charge_set_charger_type(CHARGER_TYPE_USB);
			charge_set_charger_source(POWER_SUPPLY_TYPE_USB);
			hwlog_info("%s case = CHARGER_TYPE_SDP\n", __func__);
			need_resched_work = 1;
		}
		break;
	case CHARGER_TYPE_CDP:
		if ((charger_type == CHARGER_TYPE_NON_STANDARD) ||
			(charger_type == CHARGER_TYPE_WIRELESS) ||
			(charger_type == CHARGER_REMOVED) ||
			(charger_type == CHARGER_TYPE_USB)) {
			/*
			 * factory equipment has usb reset issues
			 * when bc1.2 type is CDP
			 */
			if (power_cmdline_is_factory_mode())
				charge_set_charger_type(CHARGER_TYPE_USB);
			else
				charge_set_charger_type(CHARGER_TYPE_BC_USB);

			charge_set_charger_source(POWER_SUPPLY_TYPE_USB);
			hwlog_info("%s case = CHARGER_TYPE_CDP\n", __func__);
			need_resched_work = 1;
		}
		break;
	case CHARGER_TYPE_DCP:
		if ((charger_type != CHARGER_TYPE_STANDARD) &&
			(charger_type != CHARGER_TYPE_PD) &&
			(charger_type != CHARGER_TYPE_FCP)) {
			charge_set_charger_type(CHARGER_TYPE_STANDARD);
			charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
			if (di->support_standard_ico == 1)
				ico_enable = 1;
			else
				ico_enable = 0;
			need_resched_work = 1;
			hwlog_info("%s case = CHARGER_TYPE_DCP\n", __func__);
		}
		break;
	case CHARGER_TYPE_UNKNOWN:
		if ((charger_type == CHARGER_TYPE_WIRELESS) ||
			(charger_type == CHARGER_TYPE_USB) ||
			(charger_type == CHARGER_REMOVED)) {
			charge_set_charger_type(CHARGER_TYPE_NON_STANDARD);
			charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
			charger_type_update = FALSE;
			need_resched_work = 1;
			hwlog_info("%s case = CHARGER_TYPE_NONSTANDARD\n", __func__);
		}
		break;
	case PD_DPM_VBUS_TYPE_TYPEC:
		hwlog_debug("%s case = CHARGER_TYPE_TYPEC\n", __func__);
		pd_typec_current_handler();
		return;
	case PD_DPM_VBUS_TYPE_PD:
		last_type = charger_type;
		vbus_state = (struct pd_dpm_vbus_state *) data;
		charge_set_charger_type(CHARGER_TYPE_PD);
		di->pd_input_current = vbus_state->ma;
		di->pd_charge_current = (vbus_state->mv * vbus_state->ma *
			charger_pd_cur_trans_ratio) / (100 * di->core_data->vterm);
		if (vbus_state->ext_power)
			charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		else
			charge_set_charger_source(POWER_SUPPLY_TYPE_USB);

		if (last_type == CHARGER_TYPE_STANDARD) {
			try_pd_to_fcp_counter = PD_TO_FCP_MAX_COUNT;
#ifdef CONFIG_DIRECT_CHARGER
			try_pd_to_scp_counter = PD_TO_SCP_MAX_COUNT;
#endif
		}
		need_resched_work = 1;
		hwlog_info("%s case = CHARGER_TYPE_PD\n", __func__);
		break;
	case CHARGER_TYPE_WIRELESS:
		charge_set_charger_type(CHARGER_TYPE_WIRELESS);
		charge_set_charger_source(POWER_SUPPLY_TYPE_MAINS);
		need_resched_work = 1;
		hwlog_info("%s case = CHARGER_TYPE_WIRELESS\n", __func__);
		break;
	default:
		hwlog_info("%s ignore type = %lu\n", __func__, type);
		return;
	}

	charge_send_uevent(NO_EVENT);
	if (need_resched_work)
		mod_delayed_work(system_power_efficient_wq, &di->charge_work, 0);
}

static int pd_dpm_notifier_call(struct notifier_block *tcpc_nb, unsigned long event, void *data)
{
	bool ret = false;
	struct pd_dpm_vbus_state *vbus_state = NULL;
	struct charge_device_info *di = container_of(tcpc_nb, struct charge_device_info, tcpc_nb);

	hwlog_info("%s ++ , event = %ld\n", __func__, event);
	charger_type_ever_notify = true;

	if ((charge_detecion_get_sinksource_type() == POWER_SUPPLY_START_SINK) &&
		(event == CHARGER_TYPE_DCP) &&
		(charge_get_charger_type() == CHARGER_TYPE_PD)) {
		try_pd_to_fcp_counter = PD_TO_SCP_MAX_COUNT;
#ifdef CONFIG_DIRECT_CHARGER
		try_pd_to_scp_counter = PD_TO_SCP_MAX_COUNT;
		hwlog_info("%s try_pd_to_scp\n", __func__);
		mod_delayed_work(system_power_efficient_wq, &di->charge_work, 0);
#endif
		return NOTIFY_OK;
	}

	if (event == PD_DPM_VBUS_TYPE_PD)
		pd_charge_flag = false;
	if (!pmic_vbus_irq_is_enabled()) {
		mutex_lock(&di->event_type_lock);
		if (charge_get_charger_online())
			charger_type_handler(event, data);
		mutex_unlock(&di->event_type_lock);
		return NOTIFY_OK;
	}
	if (event == PD_DPM_VBUS_TYPE_TYPEC) {
		hwlog_info("%s typec_notify\n", __func__);
		pd_typec_current_handler();
		return NOTIFY_OK;
	}

	if (event == PD_DPM_VBUS_TYPE_PD) {
		vbus_state = (struct pd_dpm_vbus_state *) data;
		di->pd_input_current = vbus_state->ma;
		di->pd_charge_current = (vbus_state->mv * vbus_state->ma *
			charger_pd_cur_trans_ratio) / (100 * di->core_data->vterm);
		charge_rename_pd_charger_type(event, di, vbus_state->ext_power);
	} else {
		ret = charge_rename_charger_type(event, di, FALSE);
		if (ret) {
			hwlog_info("unbalanced charger type notify\n");
			return NOTIFY_OK;
		}
	}
	power_wakeup_lock(charge_lock, false);

	charge_uevent_process(di, event);

	if (event == PLEASE_PROVIDE_POWER) {
		hwlog_info("case = USB_EVENT_OTG_ID-> (IM)\n");
		power_wakeup_unlock(charge_lock, false);
		mutex_lock(&di->tcpc_otg_lock);
		charge_otg_mode_enable(CHARGE_OTG_ENABLE, VBUS_CH_USER_WIRED_OTG);
		mutex_unlock(&di->tcpc_otg_lock);
	} else if ((charge_get_charger_type() == CHARGER_REMOVED) &&
		charge_check_charger_otg_state()) {
		hwlog_info("case = USB_EVENT_NONE-> (IM)\n");
		mutex_lock(&di->tcpc_otg_lock);
		charge_stop_charging(di);
		mutex_unlock(&di->tcpc_otg_lock);
	} else {
		schedule_work(&di->usb_work);
	}

	return NOTIFY_OK;
}

#endif

#ifdef CONFIG_HUAWEI_TYPEC
static int typec_current_notifier_call(struct notifier_block *typec_nb,
	unsigned long event, void *data)
{
	struct charge_device_info *di = container_of(typec_nb,
		struct charge_device_info, typec_nb);

	if (!di)
		return NOTIFY_OK;

	if (event == POWER_NE_TYPEC_CURRENT_CHANGE) {
		di->typec_current_mode = typec_detect_current_mode();
		charge_select_charging_current(di);
		hwlog_info("%s input_current %u\n", __func__, di->input_current);
		charge_set_input_current(di->input_current);
	}

	return NOTIFY_OK;
}
#endif

static bool charge_check_otg_state(void)
{
	int mode = VBUS_CH_NOT_IN_OTG_MODE;

	vbus_ch_get_mode(VBUS_CH_USER_WIRED_OTG, VBUS_CH_TYPE_CHARGER, &mode);
	return (bool)mode;
}

static void charge_parse_iin_regl_para(struct charge_device_info *di)
{
	int ret;
	int i;
	int array_len;

	ret = of_property_read_u32(of_find_compatible_node(NULL, NULL,
		"huawei,charger"), "iin_regulation_enabled",
		&di->iin_regulation_enabled);
	if (ret) {
		hwlog_err("get iin_regulation_enabled fail\n");
		di->iin_regulation_enabled = 0; /* default is disabled */
	}

	hwlog_info("iin_regulation_enabled = %d\n", di->iin_regulation_enabled);

	if (!di->iin_regulation_enabled)
		return;

	array_len = of_property_count_u32_elems(of_find_compatible_node(NULL,
		NULL, "huawei,charger"), "iin_regl_para");
	if ((array_len <= 0) || (array_len > IIN_REGL_STAGE_MAX)) {
		hwlog_err("iin_regl_para length invalid\n ");
		di->iin_regulation_enabled = 0;
		return;
	}

	di->iin_regl_lut.total_stage = array_len;
	hwlog_info("iin_regl_stage = %d\n", di->iin_regl_lut.total_stage);

	di->iin_regl_lut.iin_regl_para = kzalloc(sizeof(
		*(di->iin_regl_lut.iin_regl_para)) *
		di->iin_regl_lut.total_stage, GFP_KERNEL);
	if (!di->iin_regl_lut.iin_regl_para) {
		hwlog_err("alloc iin_regl_para fail\n");
		di->iin_regulation_enabled = 0;
		di->iin_regl_lut.total_stage = 0;
		return;
	}

	for (i = 0; i < di->iin_regl_lut.total_stage; i++) {
		ret = of_property_read_u32_index(of_find_compatible_node(NULL,
			NULL, "huawei,charger"), "iin_regl_para", i,
			(unsigned int *)(&(di->iin_regl_lut.iin_regl_para[i])));
		if (ret) {
			hwlog_err("iin_regl_para[%d] dts read failed\n", i);
			di->iin_regulation_enabled = 0;
			return;
		}
		hwlog_info("iin_regl_para[%d]=%d\n", i,
			di->iin_regl_lut.iin_regl_para[i]);
	}

	ret = of_property_read_u32(of_find_compatible_node(NULL, NULL,
		"huawei,charger"), "iin_regl_interval", &di->iin_regl_interval);
	if (ret) {
		hwlog_err("get iin_regl_interval fail\n");
		di->iin_regl_interval = IIN_REGL_INTERVAL_DEFAULT;
	}
	hwlog_info("iin_regl_interval = %dms\n", di->iin_regl_interval);
}

static void parse_extra_module_dts(struct charge_device_info *di)
{
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"hisilicon,coul_core", "current_full_enable",
		&di->enable_current_full, 0);
}

static void charger_dts_read_u32(struct charge_device_info *di,
	struct device_node *np)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cust_term_exceed_time", &di->term_exceed_time, TERM_ERR_CNT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"boost_5v_enable", &di->boost_5v_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"charge_done_maintain_fcp", &di->charge_done_maintain_fcp, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_standard_ico", &di->support_standard_ico, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ico_all_the_way", &di->ico_all_the_way, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"fcp_vindpm", &di->fcp_vindpm, CHARGE_VOLTAGE_4600_MV);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"check_ibias_sleep_time", &di->check_ibias_sleep_time, DT_MSLEEP_110MS);
#ifdef CONFIG_TCPC_CLASS
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pd_cur_trans_ratio", &charger_pd_cur_trans_ratio, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pd_support", &charger_pd_support, 0);
#endif
	/* water intrused flag need clear, 1 is default value */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"clear_water_intrused_flag_after_read",
		&di->clear_water_intrused_flag_after_read, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"need_filter_pd_event", &di->need_filter_pd_event, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"force_disable_dc_path", &di->force_disable_dc_path, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"scp_adp_normal_chg", &di->scp_adp_normal_chg, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"startup_iin_limit", &di->startup_iin_limit, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hota_iin_limit", &di->hota_iin_limit, di->startup_iin_limit);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sw_recharge_flag", &g_sw_recharge_dts, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"increase_term_volt_en", &di->increase_term_volt_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"smart_charge_support", &di->smart_charge_support, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"charge_full_check_type", &di->charge_full_check_type,
		CHARGE_FULL_CHECK_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"term_err_vterm_dec", &di->term_err_vterm_dec, 0);
}

static inline void charger_dts_read_bool(struct charge_device_info *di, struct device_node *np)
{
	charge_done_sleep_dts = of_property_read_bool(np, "charge_done_sleep_enabled");
	weak_source_sleep_dts = of_property_read_bool(np, "weak_source_sleep_enabled");
	di->charge_term_disable = of_property_read_bool(np, "charge_term_disable");
}

static void charge_parse_recharge_para(struct charge_device_info *di,
	struct device_node *np)
{
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"recharge_para", di->recharge_para, RECHARGE_PARA_TOTAL)) {
		di->recharge_para[RECHARGE_SOC_TH] = SW_RECHARGE_TH;
		di->recharge_para[RECHARGE_VOL_TH] = RECHARGE_VOL;
		di->recharge_para[RECHARGE_CC_TH] = DISCHARGE_CC_TH;
		di->recharge_para[RECHARGE_TIMES_TH] = RECHARGE_TIMES;
		di->recharge_para[RECHARGE_DMD_SWITCH] = 0;
		di->recharge_para[RECHARGE_NT_SWITCH] = 0;
	}
}

static void parse_charger_module_dts(struct charge_device_info *di,
	struct device_node *np)
{
	charger_dts_read_u32(di, np);
	charger_dts_read_bool(di, np);
	charge_parse_iin_regl_para(di);
	charge_parse_recharge_para(di, np);
}

static inline void charge_parse_dts(struct charge_device_info *di,
	struct device_node *np)
{
	parse_extra_module_dts(di);
	parse_charger_module_dts(di, np);
}

static struct charge_extra_ops huawei_charge_extra_ops = {
	.check_ts = charge_state_is_ntc_fault,
	.check_otg_state = charge_check_otg_state,
	.set_state = set_charge_state,
	.get_charge_current = huawei_get_charge_current_max,
};

int charge_otg_mode_enable(int enable, unsigned int user)
{
	if (enable)
		vbus_ch_open(user, VBUS_CH_TYPE_CHARGER, false);
	else
		vbus_ch_close(user, VBUS_CH_TYPE_CHARGER, false, false);

	hwlog_info("charge_otg_mode_enable = %d,user = %d\n", enable, user);
	return 0;
}

static void huawei_charger_set_entry_time(unsigned int time, void *dev_data)
{
	if (!dev_data)
		return;

	charge_set_batfet_disable_delay(time);
	hwlog_info("set_entry_time: value=%u\n", time);
}

static void huawei_charger_set_work_mode(unsigned int mode, void *dev_data)
{
	if (!dev_data)
		return;

	if ((mode != SHIP_MODE_IN_SHIP) && (mode != SHIP_MODE_IN_SHUTDOWN_SHIP))
		return;

	charge_set_ship_mode_enable(true);
	charge_disable_watchdog();
}

static struct ship_mode_ops huawei_charger_ship_mode_ops = {
	.ops_name = "huawei_charger",
	.set_entry_time = huawei_charger_set_entry_time,
	.set_work_mode = huawei_charger_set_work_mode,
};
static struct charger_deteciton_ops huawei_charger_detection_ops = {
	.handle_start_sink_event = charge_handle_start_sink_event,
	.handle_stop_sink_event = charge_handle_stop_sink_event,
	.handle_start_wireless_event = charge_handle_start_wireless_event,
	.handle_stop_wireless_event = charge_handle_stop_wireless_event,
	.handle_start_source_event = charge_handle_start_source_event,
	.handle_stop_source_event = charge_handle_stop_source_event,
};

static void huawei_charger_set_ffc_info(struct charge_device_info *di)
{
	int vterm, iterm;

	vterm = coul_drv_battery_vbat_max();
	if (vterm < 0) {
		hwlog_err("get vterm=%d fail\n", vterm);
		vterm = VTERM_MAX_DEFAULT_MV;
	}

	iterm = (int)di->core_data->iterm;
	ffc_ctrl_set_default_info(vterm, iterm);
	hwlog_info("%s vterm=%d, iterm=%d\n", __func__, vterm, iterm);
}

static int charge_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct charge_device_info *di = NULL;
	struct device_node *np = NULL;
	enum chip_charger_type type;
#ifdef CONFIG_TCPC_CLASS
	unsigned long local_event;
	struct pd_dpm_vbus_state local_state;
#endif
	int pmic_vbus_irq_enabled = 1;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->core_data = charge_core_get_params();
	if (!di->core_data) {
		hwlog_err("di->core_data is NULL\n");
		ret = -EINVAL;
		goto charge_fail_0;
	}
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	if (!charge_ops_valid(chip_init) || !charge_ops_valid(set_charge_current) ||
		!charge_ops_valid(set_charge_enable) ||
		!charge_ops_valid(set_terminal_current) ||
		!charge_ops_valid(set_terminal_voltage) ||
		!charge_ops_valid(get_charge_state) ||
		!charge_ops_valid(reset_watchdog_timer)) {
		hwlog_err("charge ops is NULL\n");
		ret = -EINVAL;
		goto charge_fail_0;
	}
	charge_parse_dts(di, np);

	if (di->boost_5v_enable == 1)
		boost_5v_enable(1, BOOST_CTRL_DC);

	platform_set_drvdata(pdev, di);

	charge_lock = power_wakeup_source_register(di->dev, "charge_wakelock");
	uscp_plugout_lock = power_wakeup_source_register(di->dev, "uscp_plugout_lock");
	stop_charge_lock = power_wakeup_source_register(di->dev, "stop_charge_wakelock");

	init_completion(&emark_detect_comp);

	mutex_init(&di->event_type_lock);
	mutex_init(&di->iin_regl_lock);
	INIT_DELAYED_WORK(&di->iin_regl_work, charge_iin_regl_work);
	INIT_DELAYED_WORK(&di->charge_work, charge_monitor_work);
	INIT_DELAYED_WORK(&di->pd_voltage_change_work, charge_pd_voltage_change_work);
	INIT_WORK(&di->usb_work, charge_usb_work);
	INIT_WORK(&resume_wakelock_work, charge_resume_wakelock_work);
	INIT_DELAYED_WORK(&di->plugout_uscp_work, uscp_plugout_send_uevent);

#ifdef CONFIG_TCPC_CLASS
	if (charger_pd_support) {
		mutex_init(&di->tcpc_otg_lock);
		hwlog_info("Register pd_dpm notifier\n");

		di->tcpc_nb.notifier_call = pd_dpm_notifier_call;
		ret = register_pd_dpm_notifier(&di->tcpc_nb);
		if (ret < 0)
			hwlog_err("register_pd_dpm_notifier failed\n");
		else
			hwlog_info("register_pd_dpm_notifier OK\n");
	} else {
		di->usb_nb.notifier_call = charge_usb_notifier_call;
		ret = chip_charger_type_notifier_register(&di->usb_nb);
		if (ret < 0) {
			hwlog_err("chip_charger_type_notifier_register failed\n");
			goto charge_fail_1;
		}
#ifdef CONFIG_HUAWEI_TYPEC
		di->typec_nb.notifier_call = typec_current_notifier_call;
		ret = power_event_bnc_register(POWER_BNT_TYPEC, &di->typec_nb);
		if (ret < 0)
			hwlog_err("chip_charger_type_notifier_register failed\n");
#endif
	}

#else
	hwlog_info("Register usb notifier\n");
	di->usb_nb.notifier_call = charge_usb_notifier_call;
	ret = chip_charger_type_notifier_register(&di->usb_nb);
	if (ret < 0) {
		hwlog_err("chip_charger_type_notifier_register failed\n");
		goto charge_fail_1;
	}
#ifdef CONFIG_HUAWEI_TYPEC
	di->typec_nb.notifier_call = typec_current_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_TYPEC, &di->typec_nb);
	if (ret < 0)
		hwlog_err("chip_charger_type_notifier_register failed\n");
#endif
#endif

	ret = charge_state_init(di->dev);
	if (ret < 0)
		goto charge_fail_1;
	ret = charger_detecion_ops_register(&huawei_charger_detection_ops);
	if (ret) {
		hwlog_err("register charge detecion ops failed\n");
		goto charge_fail_1;
	}

	charge_set_charger_type(CHARGER_REMOVED);
	charge_set_charger_source(POWER_SUPPLY_TYPE_BATTERY);

	mutex_init(&charge_wakelock_flag_lock);
	mutex_init(&di->mutex_hiz);
	batt_ifull = coul_drv_battery_ifull();
	hwlog_info("batt_ifull %d from coul", batt_ifull);
	di->check_full_count = 0;
	di->vterm_dec_flag = FALSE;
	boot_time = power_get_monotonic_boottime();
	g_di = di;
#ifdef CONFIG_TCPC_CLASS
	pmic_vbus_irq_enabled = pmic_vbus_irq_is_enabled();
#endif
	type = chip_get_charger_type();
	if (!charger_type_ever_notify && pmic_vbus_irq_enabled) {
#ifdef CONFIG_TCPC_CLASS
		if (charger_pd_support) {
			pd_dpm_get_charge_event(&local_event, &local_state);
			pd_dpm_notifier_call(&(di->tcpc_nb), local_event, &local_state);
		} else {
			charge_rename_charger_type(type, di, FALSE);
			charge_send_uevent(NO_EVENT);
			schedule_work(&di->usb_work);
		}
		if (charge_get_charger_type() == CHARGER_TYPE_USB) {
			charger_type_update = TRUE;
			hwlog_info("get charger type: usb\n");
		}
#else
		charge_rename_charger_type(type, di, FALSE);
		charge_send_uevent(NO_EVENT);
		schedule_work(&di->usb_work);
#endif
	}
	if (!pmic_vbus_irq_enabled) {
		chg_wait_pd_init = true;
		if (charge_detecion_get_sinksource_type() == POWER_SUPPLY_MAX_CHARGER_EVENT) {
#ifdef CONFIG_WIRELESS_CHARGER
			if (wlrx_ic_is_tx_exist(WLTRX_IC_MAIN) &&
				wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) {
				power_supply_set_int_property_value("charger_detection",
					POWER_SUPPLY_PROP_CHG_PLUGIN, POWER_SUPPLY_START_SINK_WIRELESS);
			} else {
#endif
#ifdef CONFIG_TCPC_CLASS
				if (charger_pd_support) {
					power_supply_set_int_property_value("charger_detection",
						POWER_SUPPLY_PROP_CHG_PLUGIN,
						pd_dpm_get_source_sink_state());
					pd_dpm_get_charge_event(&local_event, &local_state);
					pd_dpm_notifier_call(&(di->tcpc_nb),
						local_event, &local_state);
				}
#endif
#ifdef CONFIG_WIRELESS_CHARGER
			}
#endif
		}
	}
#ifdef CONFIG_WIRELESS_CHARGER
	di->wireless_nb.notifier_call = wireless_charger_vbus_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLC, &di->wireless_nb);
	if (ret < 0)
		hwlog_err("register_wireless_charger_notifier failed\n");
#endif

#ifdef CONFIG_HUAWEI_YCABLE
	di->ycable_nb.notifier_call = charge_ycable_notifier_call;
	ret = ycable_register_event_notifier(&di->ycable_nb);
	if (ret < 0)
		hwlog_err("error:register_ycable_event_notifier failed\n");
#endif

#ifdef CONFIG_POGO_PIN
	di->pogopin_nb.notifier_call = pogopin_charger_vbus_notifier_call;
	ret = pogopin_3pin_register_pogo_vbus_notifier(&di->pogopin_nb);
	if (ret < 0)
		hwlog_err("pogopin_register_charger_vbus_notifier failed\n");

	di->pogopin_otg_typec_chg_nb.notifier_call =
		pogopin_otg_typec_charge_notifier_call;
	ret = pogopin_event_notifier_register(&di->pogopin_otg_typec_chg_nb);
	if (ret < 0)
		hwlog_err("pogo_pin_chargein_notifier_register failed\n");
#endif /* CONFIG_POGO_PIN */

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	if (charge_ops_valid(dev_check)) {
		if (charge_dev_check() == CHARGE_IC_GOOD) {
			hwlog_info("charger ic is good\n");
			set_hw_dev_flag(DEV_I2C_CHARGER);
		} else {
			hwlog_err("charger ic is bad\n");
		}
	} else {
		hwlog_err("ops dev_check is null\n");
	}
#endif

	charge_sysfs_init(&pdev->dev);
	g_sysfs_notify_event = &di->dev->kobj;
	ret = charge_extra_ops_register(&huawei_charge_extra_ops);
	if (ret)
		hwlog_err("register extra charge ops failed\n");

	huawei_charger_set_ffc_info(di);
	huawei_charger_ship_mode_ops.dev_data = di;
	ship_mode_ops_register(&huawei_charger_ship_mode_ops, SHIP_MODE_IC_TYPE_PLATFORM);
	hwlog_info("huawei charger probe ok\n");
	return 0;

charge_fail_1:
	power_wakeup_source_unregister(charge_lock);
	power_wakeup_source_unregister(stop_charge_lock);
	power_wakeup_source_unregister(uscp_plugout_lock);
charge_fail_0:
	platform_set_drvdata(pdev, NULL);
	if (di->iin_regl_lut.iin_regl_para) {
		kfree(di->iin_regl_lut.iin_regl_para);
		di->iin_regl_lut.iin_regl_para = NULL;
	}
	kfree(di);
	di = NULL;
	g_di = NULL;
	return ret;
}

static int charge_remove(struct platform_device *pdev)
{
	struct charge_device_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

#ifdef CONFIG_WIRELESS_CHARGER
	(void)power_event_bnc_unregister(POWER_BNT_WLC, &di->wireless_nb);
#endif
	chip_charger_type_notifier_unregister(&di->usb_nb);
	charge_state_remove(di->dev);
	charge_sysfs_remove(&pdev->dev);
	power_wakeup_source_unregister(charge_lock);
	power_wakeup_source_unregister(uscp_plugout_lock);
	power_wakeup_source_unregister(stop_charge_lock);
	charger_detecion_ops_unregister();
	cancel_delayed_work(&di->charge_work);
	cancel_delayed_work(&di->plugout_uscp_work);

	if (di->iin_regl_lut.iin_regl_para) {
		kfree(di->iin_regl_lut.iin_regl_para);
		di->iin_regl_lut.iin_regl_para = NULL;
	}
	kfree(di);
	di = NULL;
	g_di = NULL;
	return 0;
}

static void charge_shutdown(struct platform_device *pdev)
{
	struct charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	if (!di) {
		hwlog_err("[%s]di is NULL\n", __func__);
		return;
	}

	if (hvdcp_get_charging_stage() == HVDCP_STAGE_SUCCESS)
		(void)charge_set_vbus_vset(ADAPTER_5V);

	cancel_delayed_work(&di->charge_work);

	hwlog_info("%s --\n", __func__);

	return;
}

#ifdef CONFIG_PM
static int charge_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);

	if (charge_get_charger_source() == POWER_SUPPLY_TYPE_MAINS) {
		if (charge_is_charging_full(di)) {
			if (!charge_lock->active)
				cancel_delayed_work(&di->charge_work);
		}
	}

	if ((charge_sysfs_get_sysfs_value(CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS) == CHARGE_DONE_SLEEP_ENABLED) ||
		charge_done_sleep_dts) {
		charge_disable_watchdog();
		charge_set_input_current(di->input_current);
	}

	hwlog_info("%s --\n", __func__);

	return 0;
}

static int charge_resume(struct platform_device *pdev)
{
	struct charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	schedule_work(&resume_wakelock_work);

	if (charge_get_charger_source() == POWER_SUPPLY_TYPE_MAINS) {
		power_platform_charge_enable_sys_wdt();
		queue_delayed_work(system_power_efficient_wq,
			&di->charge_work, 0);
	}

	hwlog_info("%s --\n", __func__);

	return 0;
}
#endif /* CONFIG_PM */

static struct of_device_id charge_match_table[] = {
	{
		.compatible = "huawei,charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver charge_driver = {
	.probe = charge_probe,
	.remove = charge_remove,
#ifdef CONFIG_PM
	.suspend = charge_suspend,
	.resume = charge_resume,
#endif
	.shutdown = charge_shutdown,
	.driver = {
		.name = "huawei,charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(charge_match_table),
	},
};

static int __init charge_init(void)
{
	return platform_driver_register(&charge_driver);
}

static void __exit charge_exit(void)
{
	platform_driver_unregister(&charge_driver);
}

late_initcall(charge_init);
module_exit(charge_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("huawei charger module driver");
MODULE_AUTHOR("HUAWEI Inc");
