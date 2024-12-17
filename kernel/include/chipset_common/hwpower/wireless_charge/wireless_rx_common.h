/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_rx_common.h
 *
 * common interface, variables, definition etc of wireless_rx_common.c
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

#ifndef _WIRELESS_RX_COMMON_H_
#define _WIRELESS_RX_COMMON_H_

#include <chipset_common/hwpower/wireless_charge/wireless_trx_intf.h>
#include <linux/types.h>

#define WLRX_VBAT_BASE  4500 /* mv */

#define WIRELESS_CHIP_INIT                  0
#define WILREESS_SC_CHIP_INIT               1
#define WIRELESS_FODCHK_CHIP_INIT           2

enum wlrx_scene {
	WLRX_SCN_BEGIN = 0, /* must be zero here */
	WLRX_SCN_NORMAL = WLRX_SCN_BEGIN,
	WLRX_SCN_LIGHTSTRAP,
	WLRX_SCN_UEM,
	WLRX_SCN_COOLINGCASE,
	WLRX_SCN_HALL_ONLY,
	/* new scene must be appended */
	WLRX_SCN_END,
};

#ifdef CONFIG_WIRELESS_CHARGER
bool wlrx_power_supply_online(void);
bool wlrx_support_turbo_charge(unsigned int drv_type);
void wlrx_dispatch_turbo_charge(unsigned int drv_type, bool flag);
enum wlrx_scene wlrx_get_scene(void);
enum wlrx_scene wlrx_get_hall_only_scene(void);
void wlrx_set_high_pwr_test_flag(unsigned int drv_type, bool flag);
bool wlrx_in_high_pwr_test(unsigned int drv_type);
void wireless_charge_icon_display(int crit_type);
void wlrx_cut_off_wired_channel(unsigned int drv_type);
void wlrx_dsm_report(unsigned int drv_type, int err_no,
	char *dsm_buff, size_t buff_size);
void wireless_charge_chip_init(int tx_vset);
bool wlrx_msleep_exit(void);
int wlrx_get_discon_delay(unsigned int drv_type);
void wlrx_set_discon_delay(unsigned int drv_type, int discon_delay);
#else
static inline bool wlrx_power_supply_online(void)
{
	return false;
}

static inline bool wlrx_support_turbo_charge(unsigned int drv_type)
{
	return false;
}

static inline void wlrx_dispatch_turbo_charge(unsigned int drv_type, bool flag)
{
}

static inline enum wlrx_scene wlrx_get_scene(void)
{
	return WLRX_SCN_END;
}

static inline enum wlrx_scene wlrx_get_hall_only_scene(void)
{
	return WLRX_SCN_END;
}

static inline void wlrx_set_high_pwr_test_flag(unsigned int drv_type, bool flag)
{
}

static inline bool wlrx_in_high_pwr_test(unsigned int drv_type)
{
	return false;
}

static inline void wireless_charge_icon_display(int crit_type)
{
}

static inline void wlrx_cut_off_wired_channel(unsigned int drv_type)
{
}

static inline void wlrx_dsm_report(unsigned int drv_type, int err_no,
	char *dsm_buff, size_t buff_size)
{
}

static inline void wireless_charge_chip_init(int tx_vset)
{
}

static inline bool wlrx_msleep_exit(void)
{
	return true;
}

static inline int wlrx_get_discon_delay(unsigned int drv_type)
{
	return 0;
}

static inline void wlrx_set_discon_delay(unsigned int drv_type, int discon_delay)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_COMMON_H_ */
