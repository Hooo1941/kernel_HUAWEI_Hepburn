#ifndef __HCK_LPCPU_CPUIDLE__
#define __HCK_LPCPU_CPUIDLE__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct device_node;
struct cpuidle_state;
struct menu_device;
struct cpuidle_driver;
struct cpuidle_device;
DECLARE_HCK_VH(lpcpu_cpuidle_read_lp_para,
	       TP_PROTO(struct device_node *state_node,
			struct cpuidle_state *idle_state,
			int *err),
	       TP_ARGS(state_node, idle_state, err));
DECLARE_HCK_VH(lpcpu_cpuidle_last_max_interval_update,
	       TP_PROTO(struct menu_device *data,
			unsigned int max,
			unsigned int thresh),
	       TP_ARGS(data, max, thresh));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_device_cal_repeat,
	       TP_PROTO(struct menu_device *data,
			unsigned int avg),
	       TP_ARGS(data, avg));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_device_clear_repeat,
	       TP_PROTO(struct menu_device *data),
	       TP_ARGS(data));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_device_data_clear,
	       TP_PROTO(struct menu_device *data),
	       TP_ARGS(data));
DECLARE_HCK_VH(lpcpu_cpuidle_predicted_us_cal,
	       TP_PROTO(struct menu_device *data, unsigned int cpu,
			unsigned long nr_iowaiters, unsigned int *predicted_us,
			struct cpuidle_driver *drv),
	       TP_ARGS(data, cpu, nr_iowaiters, predicted_us, drv));
DECLARE_HCK_VH(lpcpu_cpuidle_predicted_ns_cal,
	       TP_PROTO(struct menu_device *data, unsigned int typical_interval,
			unsigned int predicted_us, int *hook_flag),
	       TP_ARGS(data, typical_interval, predicted_us, hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_cal_delta_next,
	       TP_PROTO(struct menu_device *data, ktime_t delta_next),
	       TP_ARGS(data, delta_next));
DECLARE_HCK_VH(lpcpu_cpuidle_cal_interactivity_req,
	       TP_PROTO(struct menu_device *data, int performance_multiplier,
			u64 *interactivity_req, int *hook_flag),
	       TP_ARGS(data, performance_multiplier, interactivity_req, hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_cal_pre_idx,
	       TP_PROTO(unsigned int cpu, int *pre_idx, int *idx, int state_count,
			int *hook_flag),
	       TP_ARGS(cpu, pre_idx, idx, state_count, hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_pre_idx_update,
	       TP_PROTO(int *pre_idx, int idx),
	       TP_ARGS(pre_idx, idx));
DECLARE_HCK_VH(lpcpu_cpuidle_low_predicted,
	       TP_PROTO(struct cpuidle_state *s, struct menu_device *data,
			int *low_predicted, int *hook_flag),
	       TP_ARGS(s, data, low_predicted, hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_select_skip_deep_cstate,
	       TP_PROTO(struct cpuidle_driver *drv, struct menu_device *data,
			int *idx, int pre_idx, unsigned int cpu),
	       TP_ARGS(drv, data, idx, pre_idx, cpu));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_select_cpu_isolation_op,
	       TP_PROTO(struct menu_device *data, unsigned int cpu, int idx,
			int *hook_flag),
	       TP_ARGS(data, cpu, idx, hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_select_op,
	       TP_PROTO(struct cpuidle_driver *drv, struct cpuidle_device *dev,
			bool *stop_tick, struct menu_device *data, int *idx,
			ktime_t delta_next, s64 latency_req, int low_predicted,
			int *hook_flag),
	       TP_ARGS(drv, dev, stop_tick, data, idx, delta_next, latency_req,
		       low_predicted, hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_reflect_op,
	       TP_PROTO(struct menu_device *data, int index),
	       TP_ARGS(data, index));
DECLARE_HCK_VH(lpcpu_cpuidle_vendor_hook,
	       TP_PROTO(int *hook_flag),
	       TP_ARGS(hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_hist_update,
	       TP_PROTO(struct cpuidle_device *dev, struct menu_device *data,
			u64 measured_ns, int *hook_flag),
	       TP_ARGS(dev, data, measured_ns, hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_update_skip_deep_cstate,
	       TP_PROTO(struct cpuidle_driver *drv, struct menu_device *data,
			int last_idx),
	       TP_ARGS(drv, data, last_idx));
DECLARE_HCK_VH(lpcpu_cpuidle_hrtimer_init,
	       TP_PROTO(struct cpuidle_device *dev),
	       TP_ARGS(dev));
DECLARE_HCK_VH(lpcpu_cpuidle_menu_init,
	       TP_PROTO(int hook_flag),
	       TP_ARGS(hook_flag));
DECLARE_HCK_VH(lpcpu_cpuidle_irqinfo_update,
	       TP_PROTO(unsigned int hwirq),
	       TP_ARGS(hwirq));
DECLARE_HCK_VH(lpcpu_cpuidle_set_deepidle_flag,
	       TP_PROTO(int hook_flag),
	       TP_ARGS(hook_flag));

#endif /* __HCK_LPCPU_CPUIDLE__ */
