/*
 * buck_charge_sysfs.h
 *
 * buck charge sysfs driver
 *
 * Copyright (C) 2023-2023 Huawei Technologies Co., Ltd.
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

#ifndef _BUCK_CHARGE_SYSFS_
#define _BUCK_CHARGE_SYSFS_

#define CHARGE_SYSFS_THERMAL_REASON_SIZE                16
#define CHARGE_SYSFS_SDP_IIN_USB                        475
#define CHARGE_SYSFS_MAX_IIN_CURRENT                    3000
#define CHARGE_SYSFS_MIN_IIN_CURRENT                    100
#define CHARGE_SYSFS_MIN_ICHG_CURRENT                   500
#define CHARGE_SYSFS_MIN_VTERM_RT                       3200
#define CHARGE_SYSFS_MAX_VTERM_RT                       4400
#define CHARGE_SYSFS_MIN_ICHG_RT                        205
#define CHARGE_SYSFS_MIN_INPUTCURRENT                   100
#define CHARGE_SYSFS_MAX_INPUTCURRENT                   2000
#define CHARGE_SYSFS_DEFAULT_IIN_CURRENT                1000
/* 0: dbc close charger, 1: dbc open charger, 2: dbc not control charger */
#define CHARGE_SYSFS_CHARGER_NOT_DBC_CONTROL            2

enum charge_sysfs_iin_thermal_charge_type {
	CHARGE_SYSFS_IIN_THERMAL_CHARGE_TYPE_BEGIN = 0,
	CHARGE_SYSFS_IIN_THERMAL_WCURRENT_5V = CHARGE_SYSFS_IIN_THERMAL_CHARGE_TYPE_BEGIN,
	CHARGE_SYSFS_IIN_THERMAL_WCURRENT_9V,
	CHARGE_SYSFS_IIN_THERMAL_WLCURRENT_5V,
	CHARGE_SYSFS_IIN_THERMAL_WLCURRENT_9V,
	CHARGE_SYSFS_IIN_THERMAL_CHARGE_TYPE_END,
};

enum charge_sysfs_type {
	CHARGE_SYSFS_IIN_THERMAL = 0,
	CHARGE_SYSFS_IIN_RUNNINGTEST,
	CHARGE_SYSFS_WATCHDOG_DISABLE,
	CHARGE_SYSFS_HIZ,
	CHARGE_SYSFS_ADAPTOR_VOLTAGE,
	CHARGE_SYSFS_CHARGE_TYPE,
	CHARGE_SYSFS_VOLTAGE_SYS,
	CHARGE_SYSFS_CHARGE_DONE_STATUS,
	CHARGE_SYSFS_IIN_RT_CURRENT,
	CHARGE_SYSFS_ENABLE_CHARGER,
	CHARGE_SYSFS_FACTORY_DIAG_CHARGER,
	CHARGE_SYSFS_UPDATE_VOLT_NOW,
	CHARGE_SYSFS_IBUS,
	CHARGE_SYSFS_IBUS_QCOM,
	CHARGE_SYSFS_VBUS,
	CHARGE_SYSFS_MAINS_ONLINE,
	CHARGE_SYSFS_CHARGE_TERM_VOLT_DESIGN,
	CHARGE_SYSFS_CHARGE_TERM_CURR_DESIGN,
	CHARGE_SYSFS_CHARGE_TERM_VOLT_SETTING,
	CHARGE_SYSFS_CHARGE_TERM_CURR_SETTING,
	CHARGE_SYSFS_DBC_CHARGE_CONTROL,
	CHARGE_SYSFS_DBC_CHARGE_DONE,
	CHARGE_SYSFS_ADAPTOR_TEST,
	CHARGE_SYSFS_REGULATION_VOLTAGE,
	CHARGE_SYSFS_PLUGUSB,
	CHARGE_SYSFS_THERMAL_REASON,
	CHARGE_SYSFS_VTERM_DEC,
	CHARGE_SYSFS_ICHG_RATIO,
	CHARGE_SYSFS_CHARGER_ONLINE,
	CHARGE_SYSFS_ADC_CONV_RATE,
	CHARGE_SYSFS_ICHG_THERMAL,
	CHARGE_SYSFS_ICHG_RUNNINGTEST,
	CHARGE_SYSFS_LIMIT_CHARGING,
	CHARGE_SYSFS_BATFET_DISABLE,
	CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS,
	CHARGE_SYSFS_INPUTCURRENT,
	CHARGE_SYSFS_VR_CHARGER_TYPE,
	CHARGE_SYSFS_CHARGER_CVCAL,
	CHARGE_SYSFS_CHARGER_CVCAL_CLEAR,
	CHARGE_SYSFS_CHARGER_GET_CVSET,
	CHARGE_SYSFS_DISABLE_CHARGER,
	CHARGE_SYSFS_FCP_CHARGE_ENABLE,
	CHARGE_SYSFS_IS_TRIM_CHARGER,
};

/* just sysfs variables and local global variables should be added to charge_sysfs_dev */
struct charge_sysfs_dev {
	/* sysfs variables */
	int iin_rt_curr;
	int hiz_mode;
	int vbus;
	int voltage_sys;
	int charge_done_status;
	int charge_done_sleep_status;
	int vterm;
	int iterm;
	int charger_cvcal_value;
	int charger_cvcal_clear;
	int charger_get_cvset;
	int chrg_config;
	int factory_diag;
	int iin_rt;
	unsigned int inputcurrent;
	unsigned int batfet_disable;
	unsigned int charge_limit;
	unsigned int iin_thl;
	unsigned int iin_thl_array[CHARGE_SYSFS_IIN_THERMAL_CHARGE_TYPE_END];
	unsigned int adc_conv_rate;
	unsigned int ichg_thl;
	unsigned int ichg_rt;
	unsigned int vterm_rt;
	unsigned int wdt_disable;;
	unsigned int fcp_charge_enable;
	unsigned int vr_charger_type;
	unsigned int dbc_charge_control;
	unsigned int water_intrused;
	unsigned int charge_enable;
	char thermal_reason[CHARGE_SYSFS_THERMAL_REASON_SIZE];
	/* local global variables */
	unsigned int iin_max;
	unsigned int ichg_max;
};

struct charge_sysfs_show_struct {
	int (*show_process)(struct charge_sysfs_dev *di, char *buf, size_t offset);
	size_t offset;
};

struct charge_sysfs_store_struct {
	int (*store_process)(struct charge_sysfs_dev *di, const char *buf);
};

#if (defined(CONFIG_HUAWEI_CHARGER_AP) || defined(CONFIG_HUAWEI_CHARGER))
int charge_sysfs_get_sysfs_value(unsigned int info_name);
void charge_sysfs_set_sysfs_value(unsigned int info_name, unsigned int value);
void charge_sysfs_init(struct device *dev);
void charge_sysfs_remove(struct device *dev);
#else
int charge_sysfs_get_sysfs_value(unsigned int info_name)
{
	return 0;
}

void charge_sysfs_set_sysfs_value(unsigned int info_name, unsigned int value)
{
}

void charge_sysfs_init(struct device *dev)
{
}

void charge_sysfs_remove(struct device *dev)
{
}
#endif /* CONFIG_HUAWEI_CHARGER_AP || CONFIG_HUAWEI_CHARGER */

#endif /* _BUCK_CHARGE_SYSFS_ */
