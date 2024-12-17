/*
 * dfx_universal_wdt.h
 *
 * Watchdog driver for ARM SP805 watchdog module.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_UNIVERSAL_WDT_H
#define __DFX_UNIVERSAL_WDT_H

#include <linux/types.h>
#include <linux/notifier.h>
#include <linux/version.h>

struct rdr_arctimer_s {
	u32 cntv_ctl_el0;
	u32 cntv_tval_el0;
	u32 cntp_ctl_el0;
	u32 cntp_tval_el0;
	u32 cntfrq_el0;

	u64 cntv_cval_el0;
	u64 cntp_cval_el0;
	u64 cntvct_el0;
	u64 cntpct_el0;
};

#if defined(CONFIG_DFX_SP805_WATCHDOG) || defined(CONFIG_WATCHDOG_V500)|| defined(CONFIG_MNTN_VWATCHDOG)
extern unsigned int get_wdt_kick_time(void);
extern unsigned long get_wdt_expires_time(void);
extern void dfx_wdt_dump(void);
extern void rdr_arctimer_register_read(struct rdr_arctimer_s *arctimer);
extern void rdr_archtime_register_print(struct rdr_arctimer_s *arctimer, bool after);

extern struct rdr_arctimer_s g_rdr_arctimer_record;
static inline struct rdr_arctimer_s *rdr_get_arctimer_record(void)
{
	return &g_rdr_arctimer_record;
}

extern void wdt_register_notify(struct notifier_block *nb);
extern void wdt_unregister_notify(struct notifier_block *nb);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
extern void watchdog_lockup_panic_config(unsigned int enable);
extern bool watchdog_softlockup_happen(void);
extern bool watchdog_othercpu_hardlockup_happen(void);
extern bool watchdog_hiwdt_hardlockup_happen(void);
extern void watchdog_set_thresh(int timeout);
extern void watchdog_check_hardlockup_hiwdt(void);
extern void watchdog_shutdown_oneshot(unsigned int timeout);
#endif
#else
static inline unsigned int get_wdt_kick_time(void) { return 0; }
static inline unsigned long get_wdt_expires_time(void) { return 0; }
static inline void dfx_wdt_dump(void) { return; }
static inline void rdr_arctimer_register_read(struct rdr_arctimer_s *arctimer) { return; }
static inline void rdr_archtime_register_print(struct rdr_arctimer_s *arctimer, bool after) { return; }

static inline struct rdr_arctimer_s *rdr_get_arctimer_record(void){ return NULL; }

static inline void wdt_register_notify(struct notifier_block *nb) { return; }
static inline void wdt_unregister_notify(struct notifier_block *nb) { return; }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static inline void watchdog_lockup_panic_config(unsigned int enable) { return; }
static inline bool watchdog_softlockup_happen(void) { return false; }
static inline bool watchdog_othercpu_hardlockup_happen(void) { return false; }
static inline bool watchdog_hiwdt_hardlockup_happen(void) { return false; }
static inline void watchdog_set_thresh(int timeout) { return; }
static inline void watchdog_check_hardlockup_hiwdt(void) { return; }
static inline void watchdog_shutdown_oneshot(unsigned int timeout) { return; }
#endif
#endif

#if defined(CONFIG_DFX_SP805_WATCHDOG) || defined(CONFIG_WATCHDOG_V500)
extern void wdt_fast_reset(void);
#endif

#if defined(CONFIG_WATCHDOG_V500) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)) \
	&& defined(CONFIG_SR_HIBERNATION_WATCHDOG)
extern int watchdog_get_default_timeout(void);
extern void watchdog_s4_timeout_set(unsigned int timeout);
#endif
#endif /* __DFX_UBIVERSAL_WDT_H */
