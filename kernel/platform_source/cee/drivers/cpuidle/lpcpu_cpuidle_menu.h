/*
 *
 * lpcpu cpuidle menu gov head file
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

#ifndef __LPCPU_CPUIDLE_MENU_H__
#define __LPCPU_CPUIDLE_MENU_H__

#include <linux/cpuidle.h>
#include <linux/cpumask.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/hrtimer.h>
#include <platform_include/cee/linux/lpcpu_idle_drv.h>

#ifdef CONFIG_CPUIDLE_SKIP_DEEP_CSTATE
#define SKIP_DEEP_CSTATE_THD_NUM (8)
#define SKIP_DEEP_CSTATE_TOP_NUM (8 + 4)
#define SKIP_DEEP_CSTATE_MAX_INTERVAL_US (2000)
#endif

#define PREDICT_THRESHOLD_NS	(5000000ULL * NSEC_PER_USEC) /* in ns */

/* The following code must be consistent with the source code */
#define BUCKETS 12
#define INTERVAL_SHIFT 3
#define INTERVALS (1UL << INTERVAL_SHIFT)

struct menu_device {
	int             last_state_idx;
	int             needs_update;
	int             tick_wakeup;

	u64		next_timer_ns;
	u64		predicted_ns;
	unsigned int	bucket;
	unsigned int	correction_factor[BUCKETS];
	unsigned int	intervals[INTERVALS];
	int		interval_ptr;

	unsigned int	repeat;

#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	int		org_state_idx;
	int		hist_inv_flag;
	int		hist_inv_repeat;
	int		hist_inv_repeat_old;
	int		hist_inv_predict;
	int		hist_ptr;
	struct menu_hist_cstate_data hist_data[HIST_SIZE];

	u64		hrtime_ns;
	unsigned int	hrtime_out;
	unsigned int	hrtime_addtime;
#endif

#ifdef CONFIG_CPUIDLE_SKIP_DEEP_CSTATE
	unsigned int	deep_state_num;
	unsigned int	last_max_interval_us;
	unsigned int	resel_deep_state;
#endif
};

void cpuidle_menu_select_op(struct cpuidle_driver *drv, struct cpuidle_device *dev,
			    bool *stop_tick, struct menu_device *data, int *idx,
			    ktime_t delta_next, s64 latency_req, int low_predicted);
int get_menu_switch_profile(void);
void cpuidle_menu_update(struct cpuidle_device *dev, struct menu_device *data,
			 u64 measured_ns, int *hook_flag);
void cpuidle_hrtimer_init(struct cpuidle_device *dev);
int __init register_menu_cpufreq_notifier(void);
#endif /* __LPCPU_CPUIDLE_MENU_H__ */
