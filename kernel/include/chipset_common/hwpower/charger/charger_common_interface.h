/* SPDX-License-Identifier: GPL-2.0 */
/*
 * charger_common_interface.h
 *
 * common interface for charger module
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

#ifndef _CHARGER_COMMON_INTERFACE_H_
#define _CHARGER_COMMON_INTERFACE_H_

#include <linux/errno.h>
#include <linux/power_supply.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/charger/charger_type.h>

/* define hiz mode of charger */
#define HIZ_MODE_ENABLE                   1
#define HIZ_MODE_DISABLE                  0

/* define charging done of charger */
#define CHARGE_DONE_NON                   0
#define CHARGE_DONE                       1

/* define sleep state on charging done of charger */
#define CHARGE_DONE_SLEEP_DISABLED        0
#define CHARGE_DONE_SLEEP_ENABLED         1

/* define wakelock state of charger */
#define CHARGE_WAKELOCK_NEED              0
#define CHARGE_WAKELOCK_NO_NEED           1

/* define otg state of charger */
#define CHARGE_OTG_ENABLE                 1
#define CHARGE_OTG_DISABLE                0

/* define voltage gears of vbus */
#define VBUS_VOLTAGE_6500_MV              6500
#define VBUS_VOLTAGE_7000_MV              7000
#define VBUS_VOLTAGE_13400_MV             13400

/* define voltage gears of battery */
#define BATTERY_VOLTAGE_0000_MV           0
#define BATTERY_VOLTAGE_0200_MV           200
#define BATTERY_VOLTAGE_3200_MV           3200
#define BATTERY_VOLTAGE_3400_MV           3400
#define BATTERY_VOLTAGE_4100_MV           4100
#define BATTERY_VOLTAGE_4200_MV           4200
#define BATTERY_VOLTAGE_4350_MV           4350
#define BATTERY_VOLTAGE_4500_MV           4500
#define BATTERY_VOLTAGE_4600_MV           4600
#define BATTERY_VOLTAGE_MIN_MV            (-32767)
#define BATTERY_VOLTAGE_MAX_MV            32767

/* define voltage gears of adapter */
#define ADAPTER_0V                        0
#define ADAPTER_5V                        5
#define ADAPTER_7V                        7
#define ADAPTER_9V                        9
#define ADAPTER_12V                       12
#define ADAPTER_15V                       15

/* state with monitor work of charger */
#define CHARGE_MONITOR_WORK_NEED_START    0
#define CHARGE_MONITOR_WORK_NEED_STOP     1

enum charge_reset_adapter_mode {
	RESET_ADAPTER_DIRECT_MODE,
	RESET_ADAPTER_SET_MODE,
	RESET_ADAPTER_CLEAR_MODE,
};

enum charge_reset_adapter_source {
	RESET_ADAPTER_SOURCE_BEGIN = 0,
	RESET_ADAPTER_SOURCE_SYSFS = RESET_ADAPTER_SOURCE_BEGIN,
	RESET_ADAPTER_SOURCE_WLTX,
	RESET_ADAPTER_SOURCE_END,
};

enum fcp_disable_voter {
	FCP_DISABLE_PLATFORM_VOTER,
	FCP_DISABLE_UEM_VOTER,
	FCP_DISABLE_VOTER_END,
};

struct charge_extra_ops {
	bool (*check_ts)(void);
	bool (*check_otg_state)(void);
	unsigned int (*get_charger_type)(void);
	int (*set_state)(int state);
	int (*get_charge_current)(void);
	int (*get_batt_by_usb_id)(void);
};

struct charge_switch_ops {
	unsigned int (*get_charger_type)(void);
};

int charge_get_first_insert(void);
void charge_set_first_insert(int flag);
void charge_set_fcp_disable_flag(bool disable, unsigned int voter);
bool charge_fcp_enable(void);
unsigned int charge_get_wakelock_flag(void);
void charge_set_wakelock_flag(unsigned int flag);
unsigned int charge_get_monitor_work_flag(void);
void charge_set_monitor_work_flag(unsigned int flag);
unsigned int charge_get_quicken_work_flag(void);
void charge_reset_quicken_work_flag(void);
void charge_update_quicken_work_flag(void);
unsigned int charge_get_run_time(void);
void charge_set_run_time(unsigned int time);
void charge_set_pd_charge_flag(bool flag);
bool charge_get_pd_charge_flag(void);
void charge_set_pd_init_flag(bool flag);
bool charge_get_pd_init_flag(void);

#if (defined(CONFIG_HUAWEI_CHARGER_AP) || defined(CONFIG_HUAWEI_CHARGER) || defined(CONFIG_FCP_CHARGER))
int charge_extra_ops_register(struct charge_extra_ops *ops);
int charge_switch_ops_register(struct charge_switch_ops *ops);
int charge_set_hiz_enable(int enable);
int charge_set_wlsin_hiz_enable(int hz_enable);
bool charge_get_hiz_state(void);
int charge_get_done_type(void);
unsigned int charge_get_charger_type(void);
void charge_set_charger_type(unsigned int type);
unsigned int charge_get_charger_source(void);
void charge_set_charger_source(unsigned int source);
int charge_get_charger_online(void);
void charge_set_charger_online(int online);
int charge_switch_get_charger_type(void);
unsigned int charge_convert_charger_type(unsigned int type);
int charge_set_buck_fv_delta(unsigned int value);
int charge_get_battery_current_avg(void);
unsigned int charge_get_reset_adapter_source(void);
void charge_set_reset_adapter_source(unsigned int mode, unsigned int value);
bool charge_need_ignore_plug_event(void);
void charge_ignore_plug_event(bool state);
void charge_update_charger_remove_type(void);
void charge_update_buck_iin_thermal(void);
int charge_get_vterm_dec(unsigned int *value);
void charge_set_usbpd_disable(bool flag);
void charge_update_iterm(int iterm);
int charger_dev_get_ibus(u32 *ibus);
#else
static inline int charge_extra_ops_register(struct charge_extra_ops *ops)
{
	return -EINVAL;
}

static inline int charge_switch_ops_register(struct charge_switch_ops *ops)
{
	return -EINVAL;
}

static inline int charge_set_hiz_enable(int enable)
{
	return 0;
}

static inline bool charge_get_hiz_state(void)
{
	return false;
}

static inline int charge_get_done_type(void)
{
	return CHARGE_DONE_NON;
}

static inline unsigned int charge_get_charger_type(void)
{
	return CHARGER_REMOVED;
}

static inline void charge_set_charger_type(unsigned int type)
{
}

static inline unsigned int charge_get_charger_source(void)
{
	return POWER_SUPPLY_TYPE_BATTERY;
}

static inline void charge_set_charger_source(unsigned int source)
{
}

static inline int charge_get_charger_online(void)
{
	return -EINVAL;
}

static inline void charge_set_charger_online(int online)
{
}

static inline int charge_switch_get_charger_type(void)
{
	return -EINVAL;
}

static inline unsigned int charge_convert_charger_type(unsigned int type)
{
	return CHARGER_REMOVED;
}

static inline int charge_set_buck_fv_delta(unsigned int value)
{
	return 0;
}

static inline int charge_get_battery_current_avg(void)
{
	return 0;
}

static inline unsigned int charge_get_reset_adapter_source(void)
{
	return 0;
}

static inline void charge_set_reset_adapter_source(unsigned int mode, unsigned int value)
{
}

static inline bool charge_need_ignore_plug_event(void)
{
	return false;
}

static inline void charge_ignore_plug_event(bool state)
{
}

static inline void charge_update_charger_remove_type(void)
{
}

static inline void charge_update_buck_iin_thermal(void)
{
}

static inline int charge_get_vterm_dec(unsigned int *value)
{
	return 0;
}

static inline void charge_set_usbpd_disable(bool flag)
{
}

static inline void charge_update_iterm(int iterm)
{
}

static inline int charger_dev_get_ibus(u32 *ibus)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_CHARGER_AP || CONFIG_HUAWEI_CHARGER || CONFIG_FCP_CHARGER */

#endif /* _CHARGER_COMMON_INTERFACE_H_ */
