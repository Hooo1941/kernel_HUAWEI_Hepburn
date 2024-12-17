/*
 * wp_softlockup.h
 *
 * This file is the header file for softlockup.c
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
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

#ifndef __WP_SOFTLOCKUP_H
#define __WP_SOFTLOCKUP_H

#ifdef CONFIG_DFX_ZEROHUNG_SOFTLOCKUP
void dfx_watchdog_check_hung(void);
void dfx_watchdog_lockup_init(void);
void dfx_watchdog_lockup_init_work(void);
#else
static inline void dfx_watchdog_check_hung(void)
{
}

static inline void dfx_watchdog_lockup_init(void)
{
}

static inline void dfx_watchdog_lockup_init_work(void)
{
}
#endif
#endif /* __WP_SOFTLOCKUP_H */
