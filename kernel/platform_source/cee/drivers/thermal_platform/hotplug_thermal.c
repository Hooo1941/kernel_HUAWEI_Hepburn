/*
 * hotplug_thermal.c
 *
 * thermal hotplug
 *
 * Copyright (C) 2017-2022 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hotplug_thermal.h"
#include <platform_include/cee/linux/lpm_thermal.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/topology.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <uapi/linux/sched/types.h>
#include <securec.h>
#include <linux/of_platform.h>
#ifdef CONFIG_THERMAL_ISOLATE
#include <linux/delay.h>
#include <linux/core_ctl.h>
#endif

u32 g_clusters = 2;
u32 g_ipa_sensors = 3;
#define NUM_OF_ARRAY	10
#define MAX_TEMP 145000
#define MIN_TEMP (-40000)

#ifdef CONFIG_THERMAL_ISOLATE
#define BIG_CLUSTER_ISOLATE_MASK 0xF0
bool g_core_ctrl_enable = true;
EXPORT_SYMBOL(g_core_ctrl_enable);
#endif

s32 thermal_temp_check(s32 temperature)
{
	if (temperature > MAX_TEMP) {
		temperature = MAX_TEMP;
	} else if (temperature < MIN_TEMP) {
		temperature = MIN_TEMP;
	}
	return temperature;
}

#ifdef CONFIG_HOTPLUG_EMULATION
static ssize_t hotplug_emul_temp_store(struct device *dev, struct device_attribute *attr,
				       const char *buf, size_t count)
{
	int temperature = 0;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, DECIMAL, &temperature))
		return -EINVAL;

	temperature = thermal_temp_check(temperature);
	g_thermal_hotplug.hotplug.cpu_emul_temp = temperature;
	pr_err("hotplug emul temp set : %d\n", temperature);

	return (long)count;
}

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
static DEVICE_ATTR(hotplug_emul_temp, S_IWUSR, NULL, hotplug_emul_temp_store);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/
#endif

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
static ssize_t gpu_hotplug_emul_temp_store(struct device *dev, struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int temperature = 0;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, DECIMAL, &temperature))
		return -EINVAL;

	temperature = thermal_temp_check(temperature);
	g_thermal_hotplug.hotplug.gpu_emul_temp = temperature;
	pr_err("gpu hotplug emul temp set : %d\n", temperature);

	return (long)count;
}

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
static DEVICE_ATTR(gpu_hotplug_emul_temp, S_IWUSR, NULL,
		   gpu_hotplug_emul_temp_store);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/
#endif

#ifdef CONFIG_NPU_HOTPLUG_EMULATION
static ssize_t npu_hotplug_emul_temp_store(struct device *dev, struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int temperature = 0;

	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (kstrtoint(buf, DECIMAL, &temperature))
		return -EINVAL;

	temperature = thermal_temp_check(temperature);
	g_thermal_hotplug.hotplug.npu_emul_temp = temperature;
	pr_err("npu hotplug emul temp set : %d\n", temperature);

	return (long)count;
}

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
static DEVICE_ATTR(npu_hotplug_emul_temp, S_IWUSR, NULL,
		   npu_hotplug_emul_temp_store);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/
#endif

static ssize_t mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	return snprintf_s(buf, (unsigned long)MAX_MODE_LEN,
			  (unsigned long)(MAX_MODE_LEN - 1), "%s\n",
			  g_thermal_hotplug.hotplug.disabled ? "disabled" : "enabled");
}

static ssize_t mode_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	if (dev == NULL || attr == NULL || buf == NULL)
		return -EINVAL;

	if (strncmp(buf, "enabled", sizeof("enabled") - 1) == 0)
		g_thermal_hotplug.hotplug.disabled = false;
	else if (strncmp(buf, "disabled", sizeof("disabled") - 1) == 0)
		g_thermal_hotplug.hotplug.disabled = true;
	else
		return -EINVAL;

	return (ssize_t)count;
}

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
static DEVICE_ATTR(hotplug_mode, 0644, mode_show, mode_store);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/

#ifdef CONFIG_HOTPLUG_EMULATION
/* cppcheck-suppress */
#define MODE_NAME_LEN	8
#define show_threshold(mode_name)					\
static ssize_t show_##mode_name						\
(struct device *dev, struct device_attribute *attr, char *buf)		\
{									\
	if (dev == NULL || attr == NULL || buf == NULL)			\
		return 0;						\
									\
	return snprintf_s(buf, MODE_NAME_LEN,  MODE_NAME_LEN - 1, "%d\n",	\
			  (int)g_thermal_hotplug.hotplug.mode_name);	\
}

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
show_threshold(gpu_down_threshold);
show_threshold(gpu_up_threshold);
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
show_threshold(npu_down_threshold);
show_threshold(npu_up_threshold);
#endif
show_threshold(cpu_down_threshold);
show_threshold(cpu_up_threshold);
show_threshold(critical_cpu_down_threshold);
show_threshold(critical_cpu_up_threshold);
/* cppcheck-suppress */
#define store_threshold(mode_name)				\
static ssize_t store_##mode_name				\
(struct device *dev, struct device_attribute *attr,		\
	const char *buf, size_t count)				\
{								\
	int value = 0;						\
								\
	if (dev == NULL || attr == NULL || buf == NULL)		\
		return 0;					\
								\
	if (kstrtoint(buf, DECIMAL, &value))			\
		return -EINVAL;					\
								\
	g_thermal_hotplug.hotplug.mode_name = value;		\
								\
	return (ssize_t)count;					\
}

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
store_threshold(gpu_down_threshold);
store_threshold(gpu_up_threshold);
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
store_threshold(npu_down_threshold);
store_threshold(npu_up_threshold);
#endif
store_threshold(cpu_down_threshold);
store_threshold(cpu_up_threshold);
store_threshold(critical_cpu_down_threshold);
store_threshold(critical_cpu_up_threshold);

/*lint -e84 -e846 -e514 -e778 -e866 -esym(84,846,514,778,866,*)*/
#define threshold_attr_rw(mode_name)				\
static DEVICE_ATTR(mode_name, S_IWUSR | S_IRUGO,		\
		   show_##mode_name, store_##mode_name)

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
threshold_attr_rw(gpu_down_threshold);
threshold_attr_rw(gpu_up_threshold);
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
threshold_attr_rw(npu_down_threshold);
threshold_attr_rw(npu_up_threshold);
#endif
threshold_attr_rw(cpu_down_threshold);
threshold_attr_rw(cpu_up_threshold);
threshold_attr_rw(critical_cpu_down_threshold);
threshold_attr_rw(critical_cpu_up_threshold);
/*lint -e84 -e846 -e514 -e778 -e866 +esym(84,846,514,778,866,*)*/
#endif

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
#ifdef CONFIG_GPU_CORE_HOTPLUG
struct gpu_hotplug_ops_t g_gpu_hotplug_ops = { .gpu_hotplug = gpu_thermal_cores_control, };
#else
struct gpu_hotplug_ops_t g_gpu_hotplug_ops = { .gpu_hotplug = NULL, };
#endif
void gpu_hotplug_callback_register(gpu_hotplug_t gpu_hotplug)
{
	if (gpu_hotplug == NULL) {
		pr_info("gpu hotplug callback register failed!\n");
		return;
	}
	g_gpu_hotplug_ops.gpu_hotplug = gpu_hotplug;
}
EXPORT_SYMBOL(gpu_hotplug_callback_register);
#endif

/* This function is used to check cpu smt mode, hotplug mask should
 * use cpu_smt_mask when function returns true which means cpu is
 * mt mode.
 */
static bool thermal_cpu_smt_check(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		if (cpumask_weight(topology_sibling_cpumask(cpu)) >= 2)
			return true;
	}
	return false;
}

static void thermal_hotplug_cpu_down(struct hotplug_t *hotplug)
{
#ifdef CONFIG_LIBLINUX
	int retry_timer;
#endif
	int cpu_num, ret;
#ifndef CONFIG_LIBLINUX
	struct device *cpu_dev = NULL;
#endif

	for (cpu_num = NR_CPUS - 1; cpu_num >= 0; cpu_num--) {
		if ((1UL << (unsigned int)cpu_num) & hotplug->control_mask) {
#ifdef CONFIG_LIBLINUX
			if (cpu_num >= 12)
				continue;
			if (liblinux_cpu_down((unsigned int)cpu_num) != 0) {
				for (retry_timer = 0; retry_timer < 10; retry_timer++) {
					if (liblinux_cpu_down((unsigned int)cpu_num) == 0)
						break;
				}
			}
#else
#ifdef CONFIG_THERMAL_ISOLATE
			ret = sched_isolate_cpu(cpu_num);
			if (ret != 0)
				pr_debug("hotplug Unable to isolate CPU%d\n", cpu_num);
#else
			cpu_dev = get_cpu_device((unsigned int)cpu_num);
			if (cpu_dev != NULL) {
				device_lock(cpu_dev);
				cpu_device_down(cpu_dev);
				kobject_uevent(&cpu_dev->kobj, KOBJ_OFFLINE);
				cpu_dev->offline = true;
				device_unlock(cpu_dev);
			}
#endif
#endif
		}
	}
	pr_err("cpu hotplug temp = %ld, mask=%x, cpu_down!!!\n", hotplug->current_temp, hotplug->control_mask);
}

static void thermal_hotplug_cpu_up(struct hotplug_t *hotplug)
{
#ifdef CONFIG_LIBLINUX
	int retry_timer;
#endif
	int ret;
	unsigned int control_mask, cpu_num;
#ifndef CONFIG_LIBLINUX
	struct device *cpu_dev = NULL;
#endif

	control_mask = hotplug->control_mask;
	if ((hotplug->current_temp < hotplug->cpu_up_threshold) || (control_mask == 0)) {
		control_mask = hotplug->critical_cpu_hotplug_mask == 0 ? control_mask : hotplug->critical_cpu_hotplug_mask;
		pr_err("ready hotplug temp = %ld, mask=%x, cpu_up!!!\n", hotplug->current_temp, control_mask);
	}

	for (cpu_num = 0; cpu_num < NR_CPUS; cpu_num++) {
		if ((1 << cpu_num) & control_mask) {
#ifdef CONFIG_LIBLINUX
			if (cpu_num >= 12)
				continue;
			if (liblinux_cpu_up(cpu_num) != 0) {
				for (retry_timer = 0; retry_timer < 10; retry_timer++) {
					if (liblinux_cpu_up((unsigned int)cpu_num) == 0)
						break;
				}
			}
#else
#ifdef CONFIG_THERMAL_ISOLATE
			ret = sched_unisolate_cpu(cpu_num);
			if (ret != 0)
				pr_debug("hotplug Unable to isolate CPU%d\n", cpu_num);
#else
			if (cpu_online(cpu_num)) {
				pr_err("ready hotplug temp = %ld, mask=%x, online no need cpu_up!!!\n", hotplug->current_temp, control_mask);
				continue;
			}
			cpu_dev = get_cpu_device(cpu_num);
			if (cpu_dev != NULL) {
				device_lock(cpu_dev);
				cpu_device_up(cpu_dev);
				kobject_uevent(&cpu_dev->kobj, KOBJ_ONLINE);
				cpu_dev->offline = false;
				device_unlock(cpu_dev);
			}
#endif
#endif
		}
	}
	pr_err("cpu hotplug temp = %ld, mask=%x, cpu_up!!!\n", hotplug->current_temp, control_mask);
}

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
static void thermal_hotplug_gpu_down(struct hotplug_t *hotplug)
{
	if (g_gpu_hotplug_ops.gpu_hotplug == NULL)
		return;
	pr_info("gpu limit to %u cores, current_temp:%ld !!!\n",
		hotplug->gpu_limit_cores, hotplug->gpu_current_temp);
	g_gpu_hotplug_ops.gpu_hotplug(hotplug->gpu_limit_cores);
}

static void thermal_hotplug_gpu_up(struct hotplug_t *hotplug)
{
	if (g_gpu_hotplug_ops.gpu_hotplug == NULL)
		return;
	pr_info("gpu restore to max cores:%u, current_temp:%ld !!!\n",
		hotplug->gpu_total_cores, hotplug->gpu_current_temp);
	/* max number of cores */
	g_gpu_hotplug_ops.gpu_hotplug(hotplug->gpu_total_cores);
}
#endif

#ifdef CONFIG_THERMAL_NPU_HOTPLUG
static void thermal_hotplug_npu_down(struct hotplug_t *hotplug)
{
	pr_info("npu limit to %u cores, current_temp:%ld !!!\n",
		hotplug->npu_limit_cores, hotplug->npu_current_temp);
	npu_ctrl_core(NPU_DEV_ID, hotplug->npu_limit_cores);
}

static void thermal_hotplug_npu_up(struct hotplug_t *hotplug)
{
	pr_info("npu restore to max cores:%u, current_temp:%ld !!!\n",
		hotplug->npu_total_cores, hotplug->npu_current_temp);
	/* max number of cores */
	npu_ctrl_core(NPU_DEV_ID, hotplug->npu_total_cores);
}
#endif

/*lint -e715*/
static int thermal_hotplug_task(void *data) /*lint !e715*/
{
	unsigned long flags;
	struct hotplug_t *hotplug = &g_thermal_hotplug.hotplug;

	while (1) {
		spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		if (!hotplug->need_up &&
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
		    !hotplug->gpu_need_down && !hotplug->gpu_need_up &&
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
		    !hotplug->npu_need_down && !hotplug->npu_need_up &&
#endif
		    !hotplug->need_down) {
			set_current_state(TASK_INTERRUPTIBLE); /*lint !e446 !e666*/
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);

			schedule();

			if (kthread_should_stop())
				break;

			spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		}
		if (hotplug->need_down) {
			hotplug->need_down = false;
			hotplug->cpu_downed = true;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
#ifdef CONFIG_THERMAL_ISOLATE
			if (hotplug->control_mask & BIG_CLUSTER_ISOLATE_MASK) {
				g_core_ctrl_enable = false;
				msleep(MSEC_PER_SEC / HZ);
			}
#endif
			thermal_hotplug_cpu_down(hotplug);
		} else if (hotplug->need_up) {
			hotplug->need_up = false;
			if (hotplug->hotplug_status == HOTPLUG_NONE)
				hotplug->cpu_downed = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
#ifdef CONFIG_THERMAL_ISOLATE
			if (hotplug->control_mask & BIG_CLUSTER_ISOLATE_MASK) {
				g_core_ctrl_enable = true;
				msleep(MSEC_PER_SEC / HZ);
			}
#endif
			thermal_hotplug_cpu_up(hotplug);
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
		} else if (hotplug->gpu_need_down) {
			hotplug->gpu_downed = true;
			hotplug->gpu_need_down = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_gpu_down(hotplug);
		} else if (hotplug->gpu_need_up) {
			hotplug->gpu_need_up = false;
			if (hotplug->gpu_hotplug_status == HOTPLUG_NONE)
				hotplug->gpu_downed = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_gpu_up(hotplug);
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
		} else if (hotplug->npu_need_down) {
			hotplug->npu_downed = true;
			hotplug->npu_need_down = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_npu_down(hotplug);
		} else if (hotplug->npu_need_up) {
			hotplug->npu_downed = false;
			hotplug->npu_need_up = false;
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
			thermal_hotplug_npu_up(hotplug);
#endif
		} else {
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		}
	}

	return 0;
}
/*lint +e715*/

/*lint +e702*/
static bool hotplug_cpu_downed_check(struct hotplug_t *hotplug, int temp)
{
	if (temp > hotplug->critical_cpu_down_threshold) {
		if (hotplug->hotplug_status == HOTPLUG_NORMAL) {
			hotplug->need_down = true;
			hotplug->control_mask =
				hotplug->critical_cpu_hotplug_mask &
				(~hotplug->cpu_hotplug_mask);
			hotplug->hotplug_status = HOTPLUG_CRITICAL;
			return true;
		}
	} else if (temp < hotplug->critical_cpu_up_threshold &&
		   temp > hotplug->cpu_up_threshold) {
		if (hotplug->hotplug_status == HOTPLUG_CRITICAL) {
			hotplug->need_up = true;
			hotplug->control_mask =
				hotplug->critical_cpu_hotplug_mask &
				(~hotplug->cpu_hotplug_mask);
			hotplug->hotplug_status = HOTPLUG_NORMAL;
			return true;
		}
	} else if (temp < hotplug->cpu_up_threshold) {
		hotplug->need_up = true;
		if (hotplug->hotplug_status == HOTPLUG_CRITICAL)
			hotplug->control_mask =
				hotplug->critical_cpu_hotplug_mask;
		else if (hotplug->hotplug_status == HOTPLUG_NORMAL)
			hotplug->control_mask = hotplug->cpu_hotplug_mask;
		else
			hotplug->control_mask = 0;

		hotplug->hotplug_status = HOTPLUG_NONE;
		return true;
	}

	return false;
}

static int hotplug_cpu_undowned_check(struct hotplug_t *hotplug,
				int temp)
{
	if (temp > hotplug->critical_cpu_down_threshold) {
		hotplug->need_down = true;
		hotplug->hotplug_status = HOTPLUG_CRITICAL;
		hotplug->control_mask =
			hotplug->critical_cpu_hotplug_mask;
		return true;
	} else if (temp > hotplug->cpu_down_threshold) {
		hotplug->need_down = true;
		hotplug->hotplug_status = HOTPLUG_NORMAL;
		hotplug->control_mask =
			hotplug->cpu_hotplug_mask;
		return true;
	}

	return false;
}

static void thermal_hotplug_check(int *temp)
{
	unsigned long flags;
	struct hotplug_t *hotplug = &g_thermal_hotplug.hotplug;
	bool need_wakeup = false;

#ifdef CONFIG_HOTPLUG_EMULATION
	if (hotplug->cpu_emul_temp != 0)
		*temp = hotplug->cpu_emul_temp;
#endif
	if (hotplug->initialized && !hotplug->disabled) {
		spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		hotplug->current_temp = *temp;

		if (hotplug->cpu_downed)
			need_wakeup = hotplug_cpu_downed_check(hotplug, *temp);
		else
			need_wakeup = hotplug_cpu_undowned_check(hotplug, *temp);
		spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		if (need_wakeup)
			wake_up_process(hotplug->hotplug_task);
	}
}

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
static bool hotplug_gpu_downed_check(struct hotplug_t *hotplug, int temp)
{
	if (temp > hotplug->critical_gpu_down_threshold) {
		if (hotplug->gpu_hotplug_status == HOTPLUG_NORMAL) {
			hotplug->gpu_need_down = true;
			hotplug->gpu_limit_cores = hotplug->critical_gpu_limit_cores;
			hotplug->gpu_hotplug_status = HOTPLUG_CRITICAL;
			return true;
		}
	} else if (temp < hotplug->critical_gpu_up_threshold &&
		   temp > hotplug->gpu_up_threshold) {
		if (hotplug->gpu_hotplug_status == HOTPLUG_CRITICAL) {
			hotplug->gpu_need_up = true;
			hotplug->gpu_total_cores = hotplug->ori_gpu_total_cores -
				hotplug->ori_gpu_limit_cores;
			hotplug->gpu_hotplug_status = HOTPLUG_NORMAL;
			return true;
		}
	} else if (temp < hotplug->gpu_up_threshold) {
		hotplug->gpu_need_up = true;
		hotplug->gpu_total_cores = hotplug->ori_gpu_total_cores;
		hotplug->gpu_hotplug_status = HOTPLUG_NONE;
		return true;
	}

	return false;
}

static int hotplug_gpu_undowned_check(struct hotplug_t *hotplug,
				int temp)
{
	if (temp > hotplug->critical_gpu_down_threshold) {
		hotplug->gpu_need_down = true;
		hotplug->gpu_hotplug_status = HOTPLUG_CRITICAL;
		hotplug->gpu_limit_cores = hotplug->critical_gpu_limit_cores;
		return true;
	} else if (temp > hotplug->gpu_down_threshold) {
		hotplug->gpu_need_down = true;
		hotplug->gpu_hotplug_status = HOTPLUG_NORMAL;
		hotplug->gpu_limit_cores = hotplug->ori_gpu_total_cores -
			hotplug->ori_gpu_limit_cores;
		return true;
	}

	return false;
}

static void thermal_gpu_hotplug_check(int *temp)
{
	unsigned long flags;
	struct hotplug_t *hotplug = &g_thermal_hotplug.hotplug;
	bool need_wakeup = false;

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
	if (hotplug->gpu_emul_temp != 0)
		*temp = hotplug->gpu_emul_temp;
#endif
	if (hotplug->initialized && !hotplug->disabled) {
		spin_lock_irqsave(&hotplug->hotplug_lock, flags);
		hotplug->gpu_current_temp = *temp;

		if (hotplug->gpu_downed)
			need_wakeup = hotplug_gpu_downed_check(hotplug, *temp);
		else
			need_wakeup = hotplug_gpu_undowned_check(hotplug, *temp);

		spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		if (need_wakeup)
			wake_up_process(hotplug->hotplug_task);
	}
}

#endif

#ifdef CONFIG_THERMAL_NPU_HOTPLUG
static int ipa_get_npu_temp(int *val)
{
	int id, ret;

	id = ipa_get_tsensor_id("npu");
	if (id < 0) {
		pr_err("%s:%d:tsensor npu is not exist.\n", __func__, __LINE__);
		return -ENODEV;
	}

	ret = ipa_get_sensor_value((u32)id, val);
	return ret;
}

static void thermal_npu_hotplug_check(void)
{
	unsigned long flags;
	int temp = 0;
	int ret;
	struct hotplug_t *hotplug = &g_thermal_hotplug.hotplug;

	ret = ipa_get_npu_temp(&temp);
	if (ret != 0)
		return;

#ifdef CONFIG_NPU_HOTPLUG_EMULATION
	if (hotplug->npu_emul_temp != 0)
		temp = hotplug->npu_emul_temp;
#endif
	if (hotplug->initialized && !hotplug->disabled) {
		spin_lock_irqsave(&hotplug->hotplug_lock, flags); /*lint !e550*/
		hotplug->npu_current_temp = temp;

		if (hotplug->npu_downed) {
			if (temp < hotplug->npu_up_threshold) {
				hotplug->npu_need_up = true;
				wake_up_process(hotplug->hotplug_task);
			}
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		} else {
			if (temp > hotplug->npu_down_threshold) {
				hotplug->npu_need_down = true;
				wake_up_process(hotplug->hotplug_task);
			}
			spin_unlock_irqrestore(&hotplug->hotplug_lock, flags);
		}
	}
}
#endif
#define DEFAULT_POLL_DELAY	200 /* unit is ms */

static int thermal_cpu_hotplug_dts_parse(struct device_node *hotplug_np,
					      struct hotplug_t *hotplug)
{
	int ret;
	u32 cpu_smt_mask;

	ret = of_property_read_u32(hotplug_np, "ithermal,up_threshold",
				   (u32 *)&hotplug->cpu_up_threshold);
	if (ret != 0) {
		pr_err("%s hotplug up threshold read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,down_threshold",
				   (u32 *)&hotplug->cpu_down_threshold);
	if (ret != 0) {
		pr_err("%s hotplug down threshold read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,cpu_hotplug_mask",
				   (u32 *)&hotplug->cpu_hotplug_mask);
	if (ret != 0) {
		pr_err("%s hotplug maskd read err\n", __func__);
		return ret;
	}

	if (thermal_cpu_smt_check()) {
		ret = of_property_read_u32(hotplug_np, "ithermal,cpu_smt_hotplug_mask",
					   &cpu_smt_mask);
		if (ret == 0) {
			pr_err("%s cpu is mt mode, use cpu_smt_hotplug_mask\n",
			       __func__);
			hotplug->cpu_hotplug_mask = cpu_smt_mask;
		}
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,polling_delay",
				   (u32 *)&hotplug->polling_delay);
	if (ret != 0) {
		pr_err("%s hotplug polling_delay use  default value\n",
		       __func__);
		hotplug->polling_delay = DEFAULT_POLL_DELAY;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,polling_delay_passive",
				   (u32 *)&hotplug->polling_delay_passive);
	if (ret != 0) {
		pr_err("%s hotplug polling_delay_passive use  default value\n",
		       __func__);
		hotplug->polling_delay_passive = DEFAULT_POLL_DELAY;
	}

	return 0;
}

static void thermal_critical_cpu_hotplug_dts_parse(struct device_node *hotplug_np,
						   struct hotplug_t *hotplug)
{
	int ret;
	u32 cpu_smt_mask;

	ret = of_property_read_u32(hotplug_np, "ithermal,critical_down_threshold",
				   (u32 *)&hotplug->critical_cpu_down_threshold);
	if (ret != 0) {
		pr_err("%s critical_down_threshold use down_threshold value\n",
		       __func__);
		hotplug->critical_cpu_down_threshold =
			hotplug->cpu_down_threshold;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,critical_up_threshold",
				   (u32 *)&hotplug->critical_cpu_up_threshold);
	if (ret != 0) {
		pr_err("%s critical_up_threshold use up_threshold value\n",
		       __func__);
		hotplug->critical_cpu_up_threshold =
			hotplug->cpu_up_threshold;
	}

	ret = of_property_read_u32(hotplug_np,
				   "ithermal,critical_cpu_hotplug_mask",
				   (u32 *)&hotplug->critical_cpu_hotplug_mask);
	if (ret != 0) {
		pr_err("%s critical_cpu_hotplug_mask use "
		       "cpu_hotplug_mask value:%x\n", __func__,
			hotplug->cpu_hotplug_mask);
		hotplug->critical_cpu_hotplug_mask =
			hotplug->cpu_hotplug_mask;
	}

	if (thermal_cpu_smt_check()) {
		ret = of_property_read_u32(hotplug_np,
					   "ithermal,critical_cpu_smt_hotplug_mask",
					   &cpu_smt_mask);
		if (ret == 0) {
			pr_err("%s cpu is mt mode, use critical_cpu_smt_hotplug_mask\n",
			       __func__);
			hotplug->critical_cpu_hotplug_mask = cpu_smt_mask;
		}
	}
	pr_err("%s hotplug mask:%x, critical mask:%x\n", __func__,
		hotplug->cpu_hotplug_mask, hotplug->critical_cpu_hotplug_mask);
}

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
static int thermal_gpu_hotplug_dts_parse(struct device_node *hotplug_np,
					 struct hotplug_t *hotplug)
{
	int ret;

	ret = of_property_read_u32(hotplug_np, "ithermal,gpu_limit_cores",
				   (u32 *)&hotplug->gpu_limit_cores);
	if (ret != 0) {
		pr_err("%s gpu_limit_cores read err\n", __func__);
		return ret;
	}
	hotplug->ori_gpu_limit_cores = hotplug->gpu_limit_cores;
	ret = of_property_read_u32(hotplug_np, "ithermal,gpu_total_cores",
				   (u32 *)&hotplug->gpu_total_cores);
	if (ret != 0) {
		pr_err("%s gpu_total_cores read err\n", __func__);
		return ret;
	}
	hotplug->ori_gpu_total_cores = hotplug->gpu_total_cores;
	ret = of_property_read_u32(hotplug_np, "ithermal,gpu_down_threshold",
				   (u32 *)&hotplug->gpu_down_threshold);
	if (ret != 0) {
		pr_err("%s gpu_down_threshold read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,gpu_up_threshold",
				   (u32 *)&hotplug->gpu_up_threshold);
	if (ret != 0) {
		pr_err("%s gpu_up_threshold read err\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,critical_gpu_down_threshold",
				   (u32 *)&hotplug->critical_gpu_down_threshold);
	if (ret != 0) {
		pr_err("%s critical_gpu_down_threshold parse err\n", __func__);
		hotplug->critical_gpu_down_threshold = hotplug->gpu_down_threshold;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,critical_gpu_up_threshold",
				   (u32 *)&hotplug->critical_gpu_up_threshold);
	if (ret != 0) {
		pr_err("%s critical_gpu_up_threshold parse err\n", __func__);
		hotplug->critical_gpu_up_threshold = hotplug->gpu_up_threshold;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,critical_gpu_limit_cores",
				   (u32 *)&hotplug->critical_gpu_limit_cores);
	if (ret != 0) {
		pr_err("%s critical_gpu_limit_cores read err\n", __func__);
		hotplug->critical_gpu_limit_cores = hotplug->gpu_limit_cores;
	}

	return 0;
}
#endif

#ifdef CONFIG_THERMAL_NPU_HOTPLUG
static int thermal_npu_hotplug_dts_parse(struct device_node *hotplug_np,
					 struct hotplug_t *hotplug)
{
	int ret;

	ret = of_property_read_u32(hotplug_np, "ithermal,npu_limit_cores",
				   (u32 *)&hotplug->npu_limit_cores);
	if (ret != 0) {
		pr_err("%s npu_limit_cores read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,npu_total_cores",
				   (u32 *)&hotplug->npu_total_cores);
	if (ret != 0) {
		pr_err("%s npu_total_cores read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,npu_down_threshold",
				   (u32 *)&hotplug->npu_down_threshold);
	if (ret != 0) {
		pr_err("%s gpu_down_threshold read err\n", __func__);
		goto node_put;
	}

	ret = of_property_read_u32(hotplug_np, "ithermal,npu_up_threshold",
				   (u32 *)&hotplug->npu_up_threshold);
	if (ret != 0) {
		pr_err("%s npu_up_threshold read err\n", __func__);
		goto node_put;
	}

	return 0;
node_put:
	return ret;
}
#endif

static int thermal_hotplug_task_create(struct hotplug_t *hotplug)
{
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	hotplug->hotplug_task
		= kthread_create(thermal_hotplug_task, NULL, "thermal_hotplug");
	if (IS_ERR(hotplug->hotplug_task)) {
		pr_err("%s: thermal hotplug task create fail\n", __func__);
		hotplug->hotplug_task = NULL;
		return -EINVAL;
	}

	sched_setscheduler_nocheck(hotplug->hotplug_task, SCHED_FIFO, &param);
	get_task_struct(hotplug->hotplug_task);
	wake_up_process(hotplug->hotplug_task);

	return 0;
}

static int thermal_hotplug_init(void)
{
	struct device_node *hotplug_np = NULL;
	int ret;
	struct hotplug_t *hotplug = &g_thermal_hotplug.hotplug;

	if (hotplug->initialized)
		return 0;

	hotplug_np = of_find_node_by_name(NULL, "cpu_temp_threshold");
	if (hotplug_np == NULL) {
		pr_err("cpu_temp_threshold node not found\n");
		return -ENODEV;
	}

	ret = thermal_cpu_hotplug_dts_parse(hotplug_np, hotplug);
	if (ret != 0) {
		pr_err("%s thermal_cpu_hotplug_dts_parse err\n", __func__);
		goto node_put;
	}
	thermal_critical_cpu_hotplug_dts_parse(hotplug_np, hotplug);

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
	ret = thermal_gpu_hotplug_dts_parse(hotplug_np, hotplug);
	if (ret != 0) {
		pr_err("%s thermal_gpu_hotplug_dts_parse err\n", __func__);
		goto node_put;
	}
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
	ret = thermal_npu_hotplug_dts_parse(hotplug_np, hotplug);
	if (ret != 0) {
		pr_err("%s thermal_npu_hotplug_dts_parse err\n", __func__);
		goto node_put;
	}
#endif
	spin_lock_init(&hotplug->hotplug_lock);
	ret = thermal_hotplug_task_create(hotplug);
	if (ret != 0) {
		pr_err("%s thermal_hotplug_task_create err\n", __func__);
		goto node_put;
	}

	hotplug->initialized = true;
	of_node_put(hotplug_np);

	return 0;

node_put:
	of_node_put(hotplug_np);

	return ret;
}

static void hotplug_check_work(struct work_struct *work)
{
	int sensor_val[NUM_OF_ARRAY] = {0};
	int val = 0;
	int val_max, ret, id;
	u32 polling_delay = DEFAULT_POLL_DELAY;
	struct delayed_work *delayed_work = to_delayed_work(work);

	/* read all sensor */
	for (id = 0; id < (int)g_ipa_sensors; id++) {
		ret = ipa_get_sensor_value((u32)id, &val);
		sensor_val[id] = val;
		if (ret != 0)
			return;
	}

	val_max = sensor_val[0];
	for (id = 1; id < (int)g_clusters; id++) {
		if (sensor_val[id] > val_max)
			val_max = sensor_val[id];
	}

#if (!defined CONFIG_PRODUCT_CDC_ACE) || (defined CONFIG_LIBLINUX) || (defined CONFIG_THERMAL_ISOLATE)
	thermal_hotplug_check(&val_max); /* max cluster temp */
#endif

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
	/* GPU */
	thermal_gpu_hotplug_check(&sensor_val[g_ipa_sensors - 1]);
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
	thermal_npu_hotplug_check(); /* NPU */
#endif
	if (val_max > g_thermal_hotplug.hotplug.critical_cpu_up_threshold ||
	    sensor_val[g_ipa_sensors - 1] >
	    g_thermal_hotplug.hotplug.critical_cpu_up_threshold)
		polling_delay = g_thermal_hotplug.hotplug.polling_delay_passive;
	else
		polling_delay = g_thermal_hotplug.hotplug.polling_delay;

	mod_delayed_work(system_freezable_power_efficient_wq, delayed_work,
			 msecs_to_jiffies(polling_delay));
}

#ifdef CONFIG_HOTPLUG_EMULATION
static int thermal_hotplug_emulation_node_create(void)
{
	int ret;

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_hotplug_emul_temp);
	if (ret != 0) {
		pr_err("Hotplug emulation temp create error\n");
		return ret;
	}

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_cpu_down_threshold);
	if (ret != 0) {
		pr_err("Hotplug down_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_cpu_up_threshold);
	if (ret != 0) {
		pr_err("Hotplug up_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_critical_cpu_down_threshold);
	if (ret != 0) {
		pr_err("Hotplug critical_down_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_critical_cpu_up_threshold);
	if (ret != 0) {
		pr_err("Hotplug critical_up_threshold create error\n");
		return ret;
	}
	return 0;
}
#endif

#ifdef CONFIG_GPU_HOTPLUG_EMULATION
static int thermal_hotplug_gpu_emulation_node_create(void)
{
	int ret;

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_gpu_hotplug_emul_temp);
	if (ret != 0) {
		pr_err("GPU hotplug emulation temp create error\n");
		return ret;
	}
	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_gpu_down_threshold);
	if (ret != 0) {
		pr_err("Hotplug gpu_down_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_gpu_up_threshold);
	if (ret != 0) {
		pr_err("Hotplug gpu_up_threshold create error\n");
		return ret;
	}

	return 0;
}
#endif

#ifdef CONFIG_NPU_HOTPLUG_EMULATION
static int thermal_hotplug_npu_emulation_node_create(void)
{
	int ret;

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_npu_hotplug_emul_temp);
	if (ret != 0) {
		pr_err("NPU hotplug emulation temp create error\n");
		return ret;
	}
	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_npu_down_threshold);
	if (ret != 0) {
		pr_err("Hotplug npu_down_threshold create error\n");
		return ret;
	}

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_npu_up_threshold);
	if (ret != 0) {
		pr_err("Hotplug gpu_up_threshold create error\n");
		return ret;
	}

	return 0;
}
#endif

int thermal_hotplugs_init(void)
{
	int ret;
	struct device_node *np = NULL;

	ret = thermal_hotplug_init();
	if (ret != 0) {
		pr_err("thermal hotplug init error\n");
		return ret;
	}

	g_thermal_hotplug.hotplug.device =
		device_create(g_thermal_hotplug.thermal_class, NULL, 0, NULL,
			      "hotplug");
	if (IS_ERR(g_thermal_hotplug.hotplug.device)) {
		pr_err("Hotplug device create error\n");
		ret = (int)PTR_ERR(g_thermal_hotplug.hotplug.device);
		goto device_destroy;
	}

	ret = device_create_file(g_thermal_hotplug.hotplug.device,
				 &dev_attr_hotplug_mode);
	if (ret != 0) {
		pr_err("Hotplug mode create error\n");
		goto device_destroy;
	}

	np = of_find_node_by_name(NULL, "ipa_sensors_info");
	if (np == NULL) {
		pr_err("ipa_sensors_info node not found\n");
		ret = -EINVAL;
		goto device_destroy;
	}
	ret = of_property_read_u32(np, "ithermal,cluster_num", &g_clusters);
	if (ret != 0) {
		pr_err("%s cluster_num read err\n", __func__);
		goto device_destroy;
	}
	ret = of_property_read_u32(np, "ithermal,ipa_sensor_num",
				   &g_ipa_sensors);
	if (ret != 0) {
		pr_err("%s ipa_sensor_num read err\n", __func__);
		goto device_destroy;
	}

#ifdef CONFIG_HOTPLUG_EMULATION
	ret = thermal_hotplug_emulation_node_create();
	if (ret != 0) {
		pr_err("thermal_hotplug_emulation_node_create error\n");
		goto device_destroy;
	}
#endif
#ifdef CONFIG_GPU_HOTPLUG_EMULATION
	ret = thermal_hotplug_gpu_emulation_node_create();
	if (ret != 0) {
		pr_err("thermal_hotplug_gpu_emulation_node_create error\n");
		goto device_destroy;
	}
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
	ret = thermal_hotplug_npu_emulation_node_create();
	if (ret != 0) {
		pr_err("thermal_hotplug_npu_emulation_node_create error\n");
		goto device_destroy;
	}
#endif
	INIT_DEFERRABLE_WORK(&g_thermal_hotplug.hotplug.poll_work, hotplug_check_work);
	mod_delayed_work(system_freezable_power_efficient_wq,
			 &g_thermal_hotplug.hotplug.poll_work,
			 msecs_to_jiffies(DEFAULT_POLL_DELAY));

	return 0;

device_destroy:
	device_destroy(g_thermal_hotplug.thermal_class, 0);
	class_destroy(g_thermal_hotplug.thermal_class);
	g_thermal_hotplug.thermal_class = NULL;

	return ret;
}
EXPORT_SYMBOL(thermal_hotplugs_init);

void thermal_hotplugs_exit(void)
{
	if (g_thermal_hotplug.thermal_class) {
		device_destroy(g_thermal_hotplug.thermal_class, 0);
		class_destroy(g_thermal_hotplug.thermal_class);
	}

	if (g_thermal_hotplug.hotplug.hotplug_task) {
		kthread_stop(g_thermal_hotplug.hotplug.hotplug_task);
		put_task_struct(g_thermal_hotplug.hotplug.hotplug_task);
		g_thermal_hotplug.hotplug.hotplug_task = NULL;
	}
	cancel_delayed_work(&g_thermal_hotplug.hotplug.poll_work);
}
EXPORT_SYMBOL(thermal_hotplugs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("thermal hotplug module driver");
