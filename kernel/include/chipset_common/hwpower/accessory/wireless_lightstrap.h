/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_lightstrap.h
 *
 * wireless lightstrap driver
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

#ifndef _WIRELESS_LIGHTSTRAP_H_
#define _WIRELESS_LIGHTSTRAP_H_

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_pwr_src.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_client.h>

#define LIGHTSTRAP_MAX_RX_SIZE           6
#define LIGHTSTRAP_INFO_LEN              16
#define LIGHTSTRAP_LIGHT_UP_MSG_LEN      1
#define LIGHTSTRAP_ENVP_OFFSET1          1
#define LIGHTSTRAP_ENVP_OFFSET2          2
#define LIGHTSTRAP_ENVP_OFFSET4          4
#define LIGHTSTRAP_PRODUCT_TYPE          7
#define COOLINGCASE_PRODUCT_TYPE         10
#define OFFLINE_PRODUCT_TYPE             0
#define LIGHTSTRAP_OFF                   0
#define LIGHTSTRAP_TIMEOUT               3600
#define COOLINGCASE_CLOSE_DELAY          150000
#define COOLINGCASE_RESET_DUTY_DELAY     30000
#define LIGHTSTRAP_DETECT_WORK_DELAY     500
#define LIGHTSTRAP_UEVENT_DELAY          500
#define LIGHTSTRAP_LIGHT_UP_DELAY        800
#define LIGHTSTRAP_INIT_IDENTIFY_DELAY   5500
#define LIGHTSTRAP_PING_FREQ_DEFAULT     0
#define LIGHTSTRAP_WORK_FREQ_DEFAULT     0
#define COOLINGCASE_PING_FREQ_DEFAULT    0
#define COOLINGCASE_WORK_FREQ_DEFAULT    0
#define COOLINGCASE_DUTY_DEFAULT         50
#define COOLINGCASE_FREQ_LEN             4
#define COOLINGCASE_DUTY_LEN             6
#define LIGHTSTRAP_RETRY_TIMES_DEFAULT   0
#define LIGHTSTRAP_RESET_TX_PING_FREQ    135
#define BOOST5V_THRESHOLD                4800
#define LV_THRESHOLD                     4700
#define HV_THRESHOLD                     4900
#define COOLINGCASE_MONITOR_VOL_DELAY    5000
#define WIRELESSCASE_CHK_MTP_TIMES       600 /* 60s */

enum wirelesscase_dmd_type {
	WIRELESSCASE_ATTACH_DMD,
	WIRELESSCASE_DETACH_DMD,
};

enum lightstrap_sysfs_type {
	LIGHTSTRAP_SYSFS_BEGIN = 0,
	LIGHTSTRAP_SYSFS_CONTROL = LIGHTSTRAP_SYSFS_BEGIN,
	LIGHTSTRAP_SYSFS_CHECK_VERSION,
};

enum lightstrap_status_type {
	LIGHTSTRAP_STATUS_DEF,  /* default */
	LIGHTSTRAP_STATUS_INIT, /* initialized */
	LIGHTSTRAP_STATUS_WWE,  /* waiting wlrx end */
	LIGHTSTRAP_STATUS_WPI,  /* waiting product info */
	LIGHTSTRAP_STATUS_DEV,  /* device on */
};

enum wirelesscase_control_type {
	WIRELESSCASE_POWER_OFF,
	WIRELESSCASE_POWER_ON,
	LIGHTSTRAP_LIGHT_UP,
};

enum wirelesscase_product_id {
	WIRELESSCASE_VER_ZERO_ID,
	WIRELESSCASE_VER_ONE_ID,
	WIRELESSCASE_VER_TWO_ID,
};

/* old type unmodifiable and new type appended */
enum wirelesscase_version_type {
	WIRELESSCASE_OFFLINE,
	LIGHTSTRAP_VERION_BASE,
	LIGHTSTRAP_VERSION_ONE = LIGHTSTRAP_VERION_BASE, /* ocean and noah */
	LIGHTSTRAP_VERSION_TWO, /* jade */
	LIGHTSTRAP_VERSION_END = 0x2F,

	COOLINGCASE_VERSION_BASE = 0x30,
	COOLINGCASE_VERSION_ONE = COOLINGCASE_VERSION_BASE,
	COOLINGCASE_VERSION_END = 0x4F,
};

struct coolingcase_freq_para {
	unsigned int hv_min;
	unsigned int hv_max;
	unsigned int lv_min;
	unsigned int lv_max;
};

struct coolingcase_duty_para {
	unsigned int hv_boot_min;
	unsigned int hv_boot_max;
	unsigned int hv_lp_min;
	unsigned int hv_lp_max;
	unsigned int lv_min;
	unsigned int lv_max;
};

enum wirelesscase_online_state {
	WLCASE_OFFLINE,
	WLCASE_ONLINE_LIGHTSTRAP,
	WLCASE_ONLINE_COOLINGCASE,
};

struct wirelesscase_di {
	struct device *dev;
	struct notifier_block lightstrap_nb;
	struct notifier_block connect_nb;
	struct work_struct event_work;
	struct delayed_work check_work;
	struct delayed_work tx_ping_work;
	struct delayed_work control_work;
	struct delayed_work identify_work;
	struct delayed_work detect_work;
	struct delayed_work delay_close_work;
	struct delayed_work duty_low_power_work;
	struct delayed_work monitor_voltage_work;
	struct mutex lock;
	enum lightstrap_status_type status;
	u8 product_type;
	u8 product_id;
	const char *model_id;
	unsigned long event_type;
	long sysfs_val;
	bool is_opened_by_hall;
	bool tx_status_ping;
	bool uevent_flag;
	unsigned int ping_freq_light;
	unsigned int work_freq_light;
	unsigned int ping_freq_cool;
	unsigned int work_freq_cool;
	struct coolingcase_freq_para coolingcase_freq;
	struct coolingcase_duty_para coolingcase_duty;
	int retry_times;
	int wltx_pwr_src;
	int online_state_new;
	enum wltx_client default_wltx_client;
	unsigned int identify_delay_time;
	bool startup_identify_timeout;
};

#ifdef CONFIG_WIRELESS_LIGHTSTRAP
bool lightstrap_online_state(void);
bool wirelesscase_hall_only(void);
bool coolingcase_online_state(void);
enum wirelesscase_online_state wirelesscase_get_online_state(void);
enum wirelesscase_online_state wirelesscase_get_online_state_new(void);
enum wltx_pwr_src lightstrap_specify_pwr_src(void);
void lightstrap_reinit_tx_chip(void);
void coolingcase_reinit_tx_chip(void);
#else
static inline bool lightstrap_online_state(void)
{
	return false;
}

static inline bool wirelesscase_hall_only(void)
{
	return false;
}

static inline bool coolingcase_online_state(void)
{
	return false;
}

static inline enum wirelesscase_online_state wirelesscase_get_online_state(void)
{
	return WLCASE_OFFLINE;
}

static inline enum wirelesscase_online_state wirelesscase_get_online_state_new(void)
{
	return WLCASE_OFFLINE;
}

static inline enum wltx_pwr_src lightstrap_specify_pwr_src(void)
{
	return PWR_SRC_NULL;
}

static inline void lightstrap_reinit_tx_chip(void)
{
}

static inline void coolingcase_reinit_tx_chip(void)
{
}
#endif /* CONFIG_WIRELESS_LIGHTSTRAP */

#endif /* _WIRELESS_LIGHTSTRAP_H_ */
