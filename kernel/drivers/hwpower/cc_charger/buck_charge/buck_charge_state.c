// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge_state.c
 *
 * include fault_handler, state_check, state_update and so on.
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/buck_charge/buck_charge_ic.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_state.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/huawei_charger.h>

#define HWLOG_TAG buck_charge_state
HWLOG_REGIST();

static struct charge_state_dev *g_charge_state_di;

bool charge_state_is_ntc_fault(void)
{
	struct charge_state_dev *di = g_charge_state_di;

	if (!di)
		return false;

	return di->ntc_fault;
}

void charge_state_stop_charging_handler(void)
{
	struct charge_state_dev *di = g_charge_state_di;

	if (!di)
		return;

	di->weaksource_cnt = 0;
}

static struct power_dsm_dump g_err_count[] = {
	{ POWER_DSM_BATTERY, ERROR_FCP_VOL_OVER_HIGH, true, false,
		"fcp vbus is low ", NULL, NULL, NULL },
	{ POWER_DSM_BATTERY, ERROR_CHARGE_VBAT_OVP, true, false,
		"vbat ovp ", NULL, NULL, NULL },
	{ POWER_DSM_BATTERY, ERROR_BOOST_OCP, true, false,
		"otg ocp", NULL, NULL, NULL },
	{ POWER_DSM_BATTERY, ERROR_CHARGE_I2C_RW, true, false,
		"Scharger I2C trans error", NULL, NULL, NULL },
	{ POWER_DSM_BATTERY, ERROR_WEAKSOURCE_HAPPEN, true, true,
		"weaksource happen, do dpm regulation", NULL, NULL, NULL },
	{ POWER_DSM_BATTERY, ERROR_VBUS_VOL_OVER_13400MV, true, false,
		"vbus over 13400 mv", NULL, NULL, NULL },
	{ POWER_DSM_BATTERY, ERROR_WEAKSOURCE_STOP_CHARGE, true, false,
		"weaksource detected, stop charging", NULL, NULL, NULL },
};

void charge_state_dsm_report(int err_no)
{
	power_dsm_dump_data_with_error_no(g_err_count, ARRAY_SIZE(g_err_count),
		err_no);
}

void charge_state_check_power_good(unsigned int state, unsigned int *events)
{
	int vout = 0;
	struct charge_state_dev *di = g_charge_state_di;

	if (!di)
		return;

	if (charge_get_charger_type() == CHARGER_TYPE_PD)
		return;

	if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS) {
		if (wlrx_ic_get_vout(WLTRX_IC_MAIN, &vout) ||
			(vout >= CHARGE_STATE_RX_HIGH_VOUT)) /* 7000mV, RX_HIGH_VOUT */
			return;
	}

	if (!(state & CHAGRE_STATE_NOT_PG)) {
		di->weaksource_cnt = 0;
		return;
	}

	if (++di->weaksource_cnt > CHARGE_STATE_WEAKSOURCE_CNT) {
		di->weaksource_cnt = 0;
		hwlog_err("VCHRG_POWER_SUPPLY_WEAKSOURCE\n");
		charge_state_dsm_report(ERROR_WEAKSOURCE_STOP_CHARGE);
		*events = VCHRG_POWER_SUPPLY_WEAKSOURCE;
		charge_send_uevent(*events);
	}
}

static void charge_state_check_vbus_ovp(unsigned int state, unsigned int *events)
{
	if (state & CHAGRE_STATE_VBUS_OVP) {
		hwlog_err("VCHRG_POWER_SUPPLY_OVERVOLTAGE\n");
		*events = VCHRG_POWER_SUPPLY_OVERVOLTAGE;
		charge_send_uevent(*events);
	}
}

static void charge_state_check_watchdog_fault(unsigned int state, unsigned int *events)
{
	if (state & CHAGRE_STATE_WDT_FAULT) {
		hwlog_err("CHAGRE_STATE_WDT_TIMEOUT\n");
		/* init chip register when watchdog timeout */
		huawei_charge_set_init_crit();
		*events = VCHRG_STATE_WDT_TIMEOUT;
		charge_send_uevent(*events);
	}
}

static void charge_state_check_batt_ovp(unsigned int state)
{
	if (state & CHAGRE_STATE_BATT_OVP)
		hwlog_err("CHAGRE_STATE_BATT_OVP\n");
}

static void charge_state_check_ntc_fault(struct charge_state_dev *di, unsigned int state)
{
	if (state & CHAGRE_STATE_NTC_FAULT)
		di->ntc_fault = true;
	else
		di->ntc_fault = false;
}

void charge_state_check(unsigned int state, unsigned int *events)
{
	struct charge_state_dev *di = g_charge_state_di;

	if (!di)
		return;

	/* check status charger ovp err */
	charge_state_check_vbus_ovp(state, events);
	/* check status watchdog timer expiration */
	charge_state_check_watchdog_fault(state, events);
	/* check status battery ovp */
	charge_state_check_batt_ovp(state);
	/* check charger ntc fault */
	charge_state_check_ntc_fault(di, state);
}

static void charge_state_boost_ocp_handler(struct charge_state_dev *di)
{
	if (charge_check_charger_otg_state()) {
		hwlog_err("vbus overloaded in boost mode, close otg mode\n");
		vbus_ch_close(VBUS_CH_USER_WIRED_OTG,
			VBUS_CH_TYPE_CHARGER, false, true);
		di->charge_fault = POWER_NE_CHG_FAULT_NON;
		charge_state_dsm_report(ERROR_BOOST_OCP);
	}
}

static void charge_state_vbat_ocp_handler(struct charge_state_dev *di)
{
	huawei_charge_vbat_ocp_handler(&di->charge_fault);
}

static void charge_state_scharger_handler(struct charge_state_dev *di)
{
	hwlog_err("scharger fault\n");
	di->charge_fault = POWER_NE_CHG_FAULT_NON;
}

static void charge_state_i2c_err_handler(struct charge_state_dev *di)
{
	hwlog_err("scharger i2c trans error\n");
	di->charge_fault = POWER_NE_CHG_FAULT_NON;
	charge_state_dsm_report(ERROR_CHARGE_I2C_RW);
}

static void charge_state_weaksource_handler(struct charge_state_dev *di)
{
	hwlog_err("weaksource happened\n");
	di->charge_fault = POWER_NE_CHG_FAULT_NON;
	charge_state_dsm_report(ERROR_WEAKSOURCE_HAPPEN);
	/* charge will into sleep to reduce power loss while weak source happened */
	huawei_charge_into_sleep();
}

static void charge_state_charge_done_handler(struct charge_state_dev *di)
{
	hwlog_info("charge done happened\n");
	huawei_charge_recharge_get_charge_time();
	huawei_charge_update_volt_term_data();
}
static void charge_state_work(struct work_struct *work)
{
	struct charge_state_dev *di = container_of(work, struct charge_state_dev, fault_work);

	if (!di)
		return;

	switch (di->charge_fault) {
	case POWER_NE_CHG_FAULT_BOOST_OCP:
		charge_state_boost_ocp_handler(di);
		break;
	case POWER_NE_CHG_FAULT_VBAT_OVP:
		charge_state_vbat_ocp_handler(di);
		break;
	case POWER_NE_CHG_FAULT_SCHARGER:
		charge_state_scharger_handler(di);
		break;
	case POWER_NE_CHG_FAULT_I2C_ERR:
		charge_state_i2c_err_handler(di);
		break;
	case POWER_NE_CHG_FAULT_WEAKSOURCE:
		charge_state_weaksource_handler(di);
		break;
	case POWER_NE_CHG_FAULT_CHARGE_DONE:
		charge_state_charge_done_handler(di);
		break;
	default:
		break;
	}
}

static int charge_state_notifier_call(struct notifier_block *fault_nb,
	unsigned long event, void *data)
{
	struct charge_state_dev *di = container_of(fault_nb, struct charge_state_dev, fault_nb);

	if (!di)
		return NOTIFY_OK;

	di->charge_fault = event;
	schedule_work(&di->fault_work);
	return NOTIFY_OK;
}

int charge_state_init(struct device *dev)
{
	int ret;
	struct charge_state_dev *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di || !dev)
		return -ENODEV;

	INIT_WORK(&di->fault_work, charge_state_work);
	di->fault_nb.notifier_call = charge_state_notifier_call;
	ret = power_event_anc_register(POWER_ANT_CHARGE_FAULT, &di->fault_nb);
	if (ret < 0)
		return ret;

	di->charge_fault = POWER_NE_CHG_FAULT_NON;
	di->weaksource_cnt = 0;
	g_charge_state_di = di;
	return 0;
}

void charge_state_remove(struct device *dev)
{
	struct charge_state_dev *di = g_charge_state_di;

	if (!di)
		return;

	power_event_anc_unregister(POWER_ANT_CHARGE_FAULT, &di->fault_nb);
	kfree(di);
	g_charge_state_di = NULL;
}