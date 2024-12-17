/*
 *
 * lpcpu cpuidle lp mode
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
 */

#include <linux/cpuidle.h>
#include <linux/cpumask.h>
#include <linux/spinlock.h>

#include "cpuidle.h"

#if defined(CONFIG_CPUIDLE_LP_MODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static int lp_mode_enabled = 0;

int get_lp_mode(void)
{
	return lp_mode_enabled;
}

extern struct cpuidle_driver * cpuidle_drivers;

void cpuidle_switch_to_lp_mode(int enabled)
{
	int cpu, i;
	struct cpuidle_driver *drv = NULL;

	cpuidle_pause();

	spin_lock(&cpuidle_driver_lock);

	for_each_possible_cpu(cpu) {
		drv = per_cpu(cpuidle_drivers, cpu);
		if (drv == NULL)
			continue;
		if (cpumask_first(drv->cpumask) != cpu)
			continue;

		/* state0 will be ignored */
		for (i = 1; i < drv->state_count; i++) {
			if (enabled) {
				drv->states[i].exit_latency_ns = drv->states[i].lp_exit_latency * NSEC_PER_USEC;
				drv->states[i].target_residency_ns = drv->states[i].lp_target_residency * NSEC_PER_USEC;
			} else {
				drv->states[i].exit_latency_ns = drv->states[i].exit_latency * NSEC_PER_USEC;
				drv->states[i].target_residency_ns = drv->states[i].target_residency * NSEC_PER_USEC;
			}
		}
	}
	lp_mode_enabled = enabled;
	spin_unlock(&cpuidle_driver_lock);

	cpuidle_resume();
}
#endif
