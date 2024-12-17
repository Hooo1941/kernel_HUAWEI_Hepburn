 /*
 *
 * hck Watchdog module
 *
 * Copyright (c) 2012-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/cpumask.h>
#include <linux/watchdog.h>
#include <linux/printk.h>
#include <linux/sched/clock.h>
#include <linux/spinlock.h>
#include <platform_include/basicplatform/linux/dfx_universal_wdt.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
#include <platform_include/basicplatform/linux/hck/mntn/hck_dfx_wdt.h>

extern void set_sample_period(void);
static cpumask_t __read_mostly watchdog_cpus;
extern int __read_mostly watchdog_thresh;
static DEFINE_PER_CPU(bool, watchdog_nmi_touch);
static DEFINE_PER_CPU(unsigned long, hiwdt_touch_ts);
static bool __read_mostly hiwdt_hardlockup_panic_happen = false;
static bool __read_mostly hiwdt_running = false;
static bool __read_mostly softlockup_panic_happen = false;
extern unsigned int __read_mostly softlockup_panic;
DECLARE_PER_CPU(unsigned long, hrtimer_interrupts);
DECLARE_PER_CPU(unsigned long, hrtimer_interrupts_saved);
static DEFINE_PER_CPU(bool, hard_watchdog_warn);

void watchdog_lockup_panic_config(unsigned int enable)
{
	softlockup_panic = enable;
	pr_info("lockup_panic %u\n", softlockup_panic);
	hiwdt_running = true;
}

bool watchdog_softlockup_happen(void)
{
	return softlockup_panic_happen;
}

bool watchdog_othercpu_hardlockup_happen(void)
{
	return false;
}


bool watchdog_hiwdt_hardlockup_happen(void)
{
	return hiwdt_hardlockup_panic_happen;
}

void watchdog_set_thresh(int timeout)
{
	watchdog_thresh = (timeout + 1)/2;
	set_sample_period();
}

static int is_hardlockup_hiwdt(unsigned int cpu)
{
	unsigned long hrint = per_cpu(hrtimer_interrupts, cpu);

	if (per_cpu(hrtimer_interrupts_saved, cpu) == hrint)
		return 1;
	per_cpu(hrtimer_interrupts_saved, cpu) = hrint;
	return 0;
}

void watchdog_check_hardlockup_hiwdt(void)
{
	int cpu, local_cpu;
	unsigned long touch_ts, now;

	now = running_clock() >> 30LL;
	local_cpu = get_cpu();
	put_cpu();

	for_each_cpu_and((cpu), cpu_online_mask, &watchdog_cpus) {
		if (cpu == local_cpu) {
			continue;
		}

		if (watchdog_thresh <= 0) {
			continue;
		}

		if (per_cpu(watchdog_nmi_touch, cpu) == true) {
			per_cpu(watchdog_nmi_touch, cpu) = false;
			continue;
		}

		touch_ts = per_cpu(hiwdt_touch_ts, cpu);
		if (touch_ts == 0) {
			per_cpu(hiwdt_touch_ts, cpu) = now;
			continue;
		}

		if (!time_after(now, touch_ts + watchdog_thresh * 2)) {
			continue;
		}
		per_cpu(hiwdt_touch_ts, cpu) = now;

		/* the timeout is the same with softlockup */
		if (is_hardlockup_hiwdt(cpu)) {
			/* only warn once */
			if (per_cpu(hard_watchdog_warn, cpu) == true)
				continue;

			if (softlockup_panic) {
				hiwdt_hardlockup_panic_happen = true;
				panic("watchdog detected hard LOCKUP on cpu %u", cpu);
				break;
			} else {
				WARN(1, "watchdog detected hard LOCKUP on cpu %u", cpu);
			}

			per_cpu(hard_watchdog_warn, cpu) = true;
		} else {
			per_cpu(hard_watchdog_warn, cpu) = false;
		}
	}
}

static void watchdog_set_softlockup_panic_happen(bool state)
{
	softlockup_panic_happen = state;
}

static void watchdog_cpumask_set(unsigned int cpu)
{
	cpumask_set_cpu(cpu, &watchdog_cpus);
	__this_cpu_write(hiwdt_touch_ts, 0);
}

static void watchdog_cpumask_clear(unsigned int cpu)
{
	cpumask_clear_cpu(cpu, &watchdog_cpus);
	per_cpu(hiwdt_touch_ts, cpu) = 0;
}

static void watchdog_cond_dump_stack(bool cond)
{
	if (likely(cond))
		dump_stack();
}
static void watchdog_set_nmi_touch(bool state)
{
	/*
	 * Using __raw here because some code paths have
	 * preemption enabled.  If preemption is enabled
	 * then interrupts should be enabled too, in which
	 * case we shouldn't have to worry about the watchdog
	 * going off.
	 */

	__this_cpu_write(watchdog_nmi_touch, state);
}
static int __init hck_dfx_wdt_register(void)
{
	REGISTER_HCK_VH(hck_dfx_wdt_shutdown_oneshot, watchdog_shutdown_oneshot);
	REGISTER_HCK_VH(hck_dfx_wdt_set_nmi_touch, watchdog_set_nmi_touch);
	REGISTER_HCK_VH(hck_dfx_wdt_set_softlockup_panic_happen, watchdog_set_softlockup_panic_happen);
	REGISTER_HCK_VH(hck_dfx_wdt_cpumask_set, watchdog_cpumask_set);
	REGISTER_HCK_VH(hck_dfx_wdt_cpumask_clear, watchdog_cpumask_clear);
	REGISTER_HCK_VH(hck_dfx_wdt_cond_dump_stack, watchdog_cond_dump_stack);
	return 0;
}

early_initcall(hck_dfx_wdt_register);
#endif
