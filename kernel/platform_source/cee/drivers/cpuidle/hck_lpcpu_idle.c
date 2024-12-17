/*
 *
 * hck cpuidle module
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

#include <linux/init.h>
#include <linux/cpuidle.h>
#include <linux/smp.h>
#include <linux/of.h>
#include <linux/errno.h>
#include <linux/math64.h>
#include <linux/compiler_attributes.h>
#include <platform_include/cee/linux/lpcpu_idle_drv.h>
#include <platform_include/cee/linux/hck/lpcpu/cpuidle/hck_lpcpu_cpuidle.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include "lpcpu_cpuidle_menu.h"

static void lpcpu_cpuidle_read_lp_para(struct device_node *state_node,
				       struct cpuidle_state *idle_state,
				       int *err)
{
#ifdef CONFIG_CPUIDLE_LP_MODE
	*err = of_property_read_u32(state_node, "lp-exit-latency-us",
				    &idle_state->lp_exit_latency);
	if (*err)
		pr_warn(" * %pOF missing lp-residency-us property\n",
			state_node);

	*err = of_property_read_u32(state_node, "lp-residency-us",
				   &idle_state->lp_target_residency);
	if (*err)
		pr_warn(" * %pOF missing lp-residency-us property\n",
			state_node);
#endif
}

static void lpcpu_cpuidle_last_max_interval_update(struct menu_device *data __maybe_unused,
						   unsigned int max __maybe_unused,
						   unsigned int thresh __maybe_unused)
{
#ifdef CONFIG_CPUIDLE_SKIP_DEEP_CSTATE
	if (thresh == INT_MAX)
		data->last_max_interval_us = max;
#endif
}

static void lpcpu_cpuidle_menu_device_cal_repeat(struct menu_device *data,
						 unsigned int avg)
{
#ifdef CONFIG_ARCH_PLATFORM
	/* if the avg is beyond the known next tick, it's worthless */
	if ((u64)avg * NSEC_PER_USEC > data->next_timer_ns)
		data->repeat = 0;
	else
		data->repeat = 1;
#endif
}

static void lpcpu_cpuidle_menu_device_clear_repeat(struct menu_device *data)
{
#ifdef CONFIG_ARCH_PLATFORM
	data->repeat = 0;
#endif
}

static void lpcpu_cpuidle_menu_device_data_clear(struct menu_device *data)
{
#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	data->org_state_idx  = 0;
	data->last_state_idx = 0;
	data->hist_inv_predict = 0;
	data->hist_inv_repeat = 0;
#endif
}

static void lpcpu_cpuidle_predicted_us_cal(struct menu_device *data, unsigned int cpu,
					   unsigned long nr_iowaiters,
					   unsigned int *predicted_us,
					   struct cpuidle_driver *drv)
{
#ifdef CONFIG_CPUIDLE_LITTLE_SKIP_CORRECTION
	if (test_slow_cpu(cpu) && !nr_iowaiters &&
	    *predicted_us * NSEC_PER_USEC > drv->states[1].target_residency_ns)
		*predicted_us = data->next_timer_ns / NSEC_PER_USEC;
#endif
}

static void lpcpu_cpuidle_predicted_ns_cal(struct menu_device *data,
					   unsigned int typical_interval,
					   unsigned int predicted_us,
					   int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	*hook_flag = 1;
	data->repeat = 0;

	/* Use the lowest expected idle interval to pick the idle state. */
	data->predicted_ns = (u64)min(predicted_us, typical_interval) *
				NSEC_PER_USEC;

	/*
	 * We disable the predict when the next timer is too long,
	 * so that it'll not stay in a light C state for a long time after
	 * a wrong predict.
	 */
	if (data->next_timer_ns > PREDICT_THRESHOLD_NS)
		data->predicted_ns = data->next_timer_ns;
#endif
}

static void lpcpu_cpuidle_cal_delta_next(struct menu_device *data, ktime_t delta_next)
{
#ifdef CONFIG_ARCH_PLATFORM
	if (data->predicted_ns < TICK_NSEC)
		data->predicted_ns = delta_next;
#endif
}

static void lpcpu_cpuidle_cal_interactivity_req(struct menu_device *data,
						int performance_multiplier,
						u64 *interactivity_req,
						int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	*hook_flag = 1;
	*interactivity_req = div64_u64(data->predicted_ns, performance_multiplier);
#endif
}

static void lpcpu_cpuidle_cal_pre_idx(unsigned int cpu, int *pre_idx __maybe_unused, int *idx,
				      int state_count, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_CPU_ISOLATION_OPT
	if (cpu_isolated(cpu)) {
#ifdef CONFIG_CPUIDLE_SKIP_DEEP_CSTATE
		*pre_idx = *idx;
#endif
		*idx = state_count;
		*hook_flag = 1;
	}
#endif
}

static void lpcpu_cpuidle_pre_idx_update(int *pre_idx __maybe_unused, int idx __maybe_unused)
{
#ifdef CONFIG_CPUIDLE_SKIP_DEEP_CSTATE
	*pre_idx = idx;
#endif
}

static void lpcpu_cpuidle_low_predicted(struct cpuidle_state *s,
					struct menu_device *data, int *low_predicted,
					int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	*hook_flag = 1;
	if (s->target_residency_ns > data->predicted_ns)
		*low_predicted = 1;
#endif
}

static void lpcpu_cpuidle_menu_select_skip_deep_cstate(struct cpuidle_driver *drv __maybe_unused,
						       struct menu_device *data __maybe_unused,
						       int *idx __maybe_unused, int pre_idx __maybe_unused,
						       unsigned int cpu __maybe_unused)
{
#ifdef CONFIG_CPUIDLE_SKIP_DEEP_CSTATE
	if ((*idx) + 1 == drv->state_count &&
		!test_slow_cpu(cpu) &&
		data->deep_state_num > SKIP_DEEP_CSTATE_THD_NUM &&
		data->last_max_interval_us < SKIP_DEEP_CSTATE_MAX_INTERVAL_US) {
		data->resel_deep_state = 1;
		*idx = pre_idx;
	} else {
		data->resel_deep_state = 0;
	}
#endif
}

static void lpcpu_cpuidle_menu_select_cpu_isolation_op(struct menu_device *data,
						       unsigned int cpu,
						       int idx,
						       int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_CPU_ISOLATION_OPT
	if (cpu_isolated(cpu)) {
		data->last_state_idx = idx;
		*hook_flag = 1;
	}
#endif
}

static void lpcpu_cpuidle_menu_select_op(struct cpuidle_driver *drv,
					 struct cpuidle_device *dev,
					 bool *stop_tick, struct menu_device *data,
					 int *idx, ktime_t delta_next, s64 latency_req,
					 int low_predicted, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	*hook_flag = 1;
	cpuidle_menu_select_op(drv, dev, stop_tick, data, idx, delta_next, latency_req,
			       low_predicted);
#endif
}

static void lpcpu_cpuidle_menu_reflect_op(struct menu_device *data, int index)
{
#ifdef CONFIG_ARCH_PLATFORM
	data->last_state_idx = index;
#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	data->hist_data[data->hist_ptr].idle_state_idx = index;
	data->hist_data[data->hist_ptr].org_state_idx = data->org_state_idx;
	data->hist_data[data->hist_ptr].exit_time_ns = ktime_get();
#endif
#endif
}

static void lpcpu_cpuidle_vendor_hook(int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	*hook_flag = 1;
#endif
}

static void lpcpu_cpuidle_menu_hist_update(struct cpuidle_device *dev,
					   struct menu_device *data,
					   u64 measured_ns, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST
	cpuidle_menu_update(dev, data, measured_ns, hook_flag);
#endif
}

static void lpcpu_cpuidle_menu_update_skip_deep_cstate(struct cpuidle_driver *drv __maybe_unused,
						       struct menu_device *data __maybe_unused,
						       int last_idx __maybe_unused)
{
#ifdef CONFIG_CPUIDLE_SKIP_DEEP_CSTATE
	if (data->deep_state_num > SKIP_DEEP_CSTATE_TOP_NUM)
		data->deep_state_num = 0;
	else if (last_idx + 1 == drv->state_count)
		data->deep_state_num++;
	else if (data->resel_deep_state == 1)
		data->deep_state_num++;
	else
		data->deep_state_num = 0;
#endif
}

static void lpcpu_cpuidle_hrtimer_init(struct cpuidle_device *dev)
{
#ifdef CONFIG_ARCH_PLATFORM
	cpuidle_hrtimer_init(dev);
#endif
}

static void lpcpu_cpuidle_menu_init(int hook_flag __always_unused)
{
#ifdef CONFIG_ARCH_PLATFORM
	int ret;

	ret = get_menu_switch_profile();
	if (!ret)
		register_menu_cpufreq_notifier();
#endif
}

static void lpcpu_cpuidle_irqinfo_update(unsigned int hwirq)
{
	idle_irq_info_update(hwirq);
}

/* set directly enter deepest idle */
static void lpcpu_cpuidle_set_deepidle_flag(int hook_flag __always_unused)
{
	set_deepest_idle_flag();
}

static int __init hck_lpcpu_cpuidle_register(void)
{
	REGISTER_HCK_VH(lpcpu_cpuidle_read_lp_para, lpcpu_cpuidle_read_lp_para);
	REGISTER_HCK_VH(lpcpu_cpuidle_last_max_interval_update, lpcpu_cpuidle_last_max_interval_update);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_device_cal_repeat, lpcpu_cpuidle_menu_device_cal_repeat);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_device_clear_repeat, lpcpu_cpuidle_menu_device_clear_repeat);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_device_data_clear, lpcpu_cpuidle_menu_device_data_clear);
	REGISTER_HCK_VH(lpcpu_cpuidle_predicted_us_cal, lpcpu_cpuidle_predicted_us_cal);
	REGISTER_HCK_VH(lpcpu_cpuidle_predicted_ns_cal, lpcpu_cpuidle_predicted_ns_cal);
	REGISTER_HCK_VH(lpcpu_cpuidle_cal_delta_next, lpcpu_cpuidle_cal_delta_next);
	REGISTER_HCK_VH(lpcpu_cpuidle_cal_interactivity_req, lpcpu_cpuidle_cal_interactivity_req);
	REGISTER_HCK_VH(lpcpu_cpuidle_cal_pre_idx, lpcpu_cpuidle_cal_pre_idx);
	REGISTER_HCK_VH(lpcpu_cpuidle_pre_idx_update, lpcpu_cpuidle_pre_idx_update);
	REGISTER_HCK_VH(lpcpu_cpuidle_low_predicted, lpcpu_cpuidle_low_predicted);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_select_skip_deep_cstate, lpcpu_cpuidle_menu_select_skip_deep_cstate);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_select_cpu_isolation_op, lpcpu_cpuidle_menu_select_cpu_isolation_op);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_select_op, lpcpu_cpuidle_menu_select_op);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_reflect_op, lpcpu_cpuidle_menu_reflect_op);
	REGISTER_HCK_VH(lpcpu_cpuidle_vendor_hook, lpcpu_cpuidle_vendor_hook);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_hist_update, lpcpu_cpuidle_menu_hist_update);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_update_skip_deep_cstate, lpcpu_cpuidle_menu_update_skip_deep_cstate);
	REGISTER_HCK_VH(lpcpu_cpuidle_hrtimer_init, lpcpu_cpuidle_hrtimer_init);
	REGISTER_HCK_VH(lpcpu_cpuidle_menu_init, lpcpu_cpuidle_menu_init);
	REGISTER_HCK_VH(lpcpu_cpuidle_irqinfo_update, lpcpu_cpuidle_irqinfo_update);
	REGISTER_HCK_VH(lpcpu_cpuidle_set_deepidle_flag, lpcpu_cpuidle_set_deepidle_flag);
	return 0;
}

early_initcall(hck_lpcpu_cpuidle_register);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
