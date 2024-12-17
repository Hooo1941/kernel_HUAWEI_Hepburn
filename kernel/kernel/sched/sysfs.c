// SPDX-License-Identifier: GPL-2.0
/*
 * sysfs.c
 *
 * eas sysfs
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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <trace/events/sched.h>
#include <securec.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/minmax.h>
#include <linux/cpumask.h>
#include <linux/percpu-defs.h>
#include <linux/cpufreq.h>

#include "sched.h"
#ifdef CONFIG_MIPS_LOAD_TRACKING
#include <platform_include/cee/linux/mips.h>
#ifdef CONFIG_CPU_FREQ_PID_MIPS
#include <pid_freq.h>
#endif
#endif
#ifdef CONFIG_SCHED_POWER_LIMIT_MAXFREQ
#include <platform_include/cee/linux/power_limit.h>
#endif
#ifdef CONFIG_SCHED_SMT_USER_HIDE
#include <linux/smp.h>
#endif
#include <linux/sched/smt.h>

struct eas_attr {
	struct attribute attr;
	ssize_t (*show)(struct kobject *kobj,
			struct kobj_attribute *kattr, char *buf);
	ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *kattr,
			 const char *buf, size_t count);
	int *value;
};

static ssize_t eas_show(struct kobject *kobj,
			struct kobj_attribute *kattr, char *buf)
{
	struct eas_attr *eas_attr =
		container_of(&kattr->attr, struct eas_attr, attr);
	int temp;

	temp = *(eas_attr->value);
	return (ssize_t)sprintf_s(buf, PAGE_SIZE, "%d\n", temp);
}

static ssize_t eas_store(struct kobject *kobj, struct kobj_attribute *kattr,
			 const char *buf, size_t count)
{
	int temp;
	ssize_t ret = count;
	struct eas_attr *eas_attr =
		container_of(&kattr->attr, struct eas_attr, attr);

	if (kstrtoint(buf, 10, &temp))
		return -EINVAL;

	if (temp < 0)
		ret = -EINVAL;
	else
		*(eas_attr->value) = temp;

	/* trace the name and value of the attribute */
	trace_perf(eas_attr_store, kattr->attr.name, temp);
	return ret;
}

unsigned int *get_tokenized_data(const char *buf, int *num_tokens)
{
	const char *cp = NULL;
	int i;
	int ntokens = 1;
	unsigned int *tokenized_data = NULL;
	int err = -EINVAL;

	cp = buf;
	while ((cp = strpbrk(cp + 1, " :")))
		ntokens++;

	tokenized_data = kmalloc(ntokens * sizeof(unsigned int), GFP_KERNEL);
	if (!tokenized_data) {
		err = -ENOMEM;
		goto err;
	}

	cp = buf;
	i = 0;
	while (i < ntokens) {
		if (sscanf_s(cp, "%u", &tokenized_data[i++]) != 1)
			goto err_kfree;

		cp = strpbrk(cp, " :");
		if (!cp)
			break;
		cp++;
	}

	if (i != ntokens)
		goto err_kfree;

	*num_tokens = ntokens;
	return tokenized_data;

err_kfree:
	kfree(tokenized_data);
err:
	return ERR_PTR(err);
}

#ifdef CONFIG_MULTI_MARGIN
static ssize_t capacity_margin_show(struct kobject *kobj,
				    struct kobj_attribute *kattr, char *buf)
{
	struct sched_cluster *cluster = NULL;
	ssize_t ret = 0;

	for_each_sched_cluster(cluster) {
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s",
				 cluster->capacity_margin, ":");
	}

	if (ret > 0)
		(void)sprintf_s(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	return ret;
}

static ssize_t sd_capacity_margin_show(struct kobject *kobj,
				       struct kobj_attribute *kattr, char *buf)
{
	struct sched_cluster *cluster = NULL;
	ssize_t ret = 0;

	for_each_sched_cluster(cluster) {
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s",
				 cluster->sd_capacity_margin, ":");
	}

	if (ret > 0)
		(void)sprintf_s(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	return ret;
}

static ssize_t capacity_margin_store(struct kobject *kobj, struct kobj_attribute *kattr,
				     const char *buf, size_t count)
{
	unsigned int *new_margin = NULL;
	struct sched_cluster *cluster = NULL;
	int i;
	int num = 0;
	int ret = count;

	new_margin = get_tokenized_data(buf, &num);
	if (IS_ERR(new_margin))
		return PTR_ERR(new_margin);

	if (num != num_clusters) {
		ret = -EINVAL;
		goto err;
	}

	i = 0;
	for_each_sched_cluster(cluster) {
		cluster->capacity_margin = new_margin[i++];
	}

err:
	kfree(new_margin);
	return ret;
}

static ssize_t sd_capacity_margin_store(struct kobject *kobj, struct kobj_attribute *kattr,
					const char *buf, size_t count)
{
	unsigned int *new_margin = NULL;
	struct sched_cluster *cluster = NULL;
	int i;
	int num = 0;
	int ret = count;

	new_margin = get_tokenized_data(buf, &num);
	if (IS_ERR(new_margin))
		return PTR_ERR(new_margin);

	if (num != num_clusters) {
		ret = -EINVAL;
		goto err;
	}

	i = 0;
	for_each_sched_cluster(cluster) {
		cluster->sd_capacity_margin = new_margin[i++];
	}

err:
	kfree(new_margin);
	return ret;
}
#endif /* CONFIG_MULTI_MARGIN */

#ifdef CONFIG_SCHED_SMT_USER_HIDE
enum smt_unhide_uid_id {
	RESET,
	ADD,
	DELETE,
};

static ssize_t smt_visible_uid_store(struct kobject *kobj,
				     struct kobj_attribute *kattr, const char *buf, size_t count)
{
	unsigned int *input = NULL;
	unsigned int num = 0;
	unsigned int i, uid;
	int ret = count;
	static DEFINE_MUTEX(mutex);

	input = get_tokenized_data(buf, &num);
	if (IS_ERR(input))
		return PTR_ERR(input);

	mutex_lock(&mutex);
	switch (input[0]) {
	case RESET:
		bitmap_clear(smt_visible_uid, 0, AID_USER_OFFSET);
		/* go through */
	case ADD:
		for (i = 1; i < num; i++) {
			uid = input[i];
			if (uid >= AID_USER_OFFSET)
				continue;
			bitmap_set(smt_visible_uid, uid, 1);
		}
		break;
	case DELETE:
		for (i = 1; i < num; i++) {
			uid = input[i];
			if (uid >= AID_USER_OFFSET)
				continue;
			bitmap_clear(smt_visible_uid, uid, 1);
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&mutex);

	kfree(input);
	return ret;
}

#define for_each_uid(uid, uid_bitmap)						\
	for (uid = -1;								\
		uid = find_next_bit(uid_bitmap, AID_USER_OFFSET, uid + 1),	\
		uid < AID_USER_OFFSET;)
static ssize_t smt_visible_uid_show(struct kobject *kobj,
				    struct kobj_attribute *kattr, char *buf)
{
	int uid;
	ssize_t ret = 0;

	for_each_uid(uid, smt_visible_uid)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s", uid, ":");

	if (ret > 0)
		scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	return ret;
}

DECLARE_BITMAP(cim_visible_uid, AID_USER_OFFSET);
enum cim_unhide_uid_id {
	UID_RESET,
	UID_ADD,
	UID_DELETE,
};

static ssize_t cim_visible_uid_store(struct kobject *kobj,
				     struct kobj_attribute *kattr, const char *buf, size_t count)
{
	unsigned int *input = NULL;
	unsigned int num = 0;
	unsigned int i, uid;
	int ret = count;
	static DEFINE_MUTEX(mutex);

	input = get_tokenized_data(buf, &num);
	if (IS_ERR(input))
		return PTR_ERR(input);

	mutex_lock(&mutex);
	switch (input[0]) {
	case UID_RESET:
		bitmap_clear(cim_visible_uid, 0, AID_USER_OFFSET);
		/* go through */
	case UID_ADD:
		for (i = 1; i < num; i++) {
			uid = input[i];
			if (uid >= AID_USER_OFFSET)
				continue;
			bitmap_set(cim_visible_uid, uid, 1);
		}
		break;
	case UID_DELETE:
		for (i = 1; i < num; i++) {
			uid = input[i];
			if (uid >= AID_USER_OFFSET)
				continue;
			bitmap_clear(cim_visible_uid, uid, 1);
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&mutex);

	kfree(input);
	return ret;
}

static ssize_t cim_visible_uid_show(struct kobject *kobj,
				    struct kobj_attribute *kattr, char *buf)
{
	int uid;
	ssize_t ret = 0;

	for_each_uid(uid, cim_visible_uid)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s", uid, ":");

	if (ret > 0)
		scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	return ret;
}

int is_whitelist_app(void) {
	unsigned int temp_uid;
	temp_uid = current_uid().val % AID_USER_OFFSET;
	if (temp_uid < AID_USER_OFFSET && test_bit(temp_uid, cim_visible_uid))
		return 1;
	return 0;
}

static unsigned int mid_cpu_max_freq(void)
{
	struct sched_cluster *cluster = NULL;
	int cpu;
	struct cpufreq_policy *policy = NULL;
	unsigned int min_freq = 0;

	for_each_sched_cluster(cluster) {
		if (cluster == min_cap_cluster())
			continue;

		cpu = cpumask_first(&cluster->cpus);
		policy = cpufreq_cpu_get_raw(cpu);
		if (policy) {
			unsigned int freq = policy->cpuinfo.max_freq;
			if (min_freq == 0 || freq < min_freq)
				min_freq = freq;
		}
	}

	return min_freq;
}

unsigned int adjust_freq_buf(unsigned int max_freq, struct cpufreq_policy *policy) {
	if (is_whitelist_app() && cpumask_test_cpu(policy->cpu, &min_cap_cluster()->cpus)) {
		max_freq = max_t(int, max_freq, mid_cpu_max_freq() - 1);
	} 
	return max_freq;
}
EXPORT_SYMBOL(adjust_freq_buf);

#endif

#ifdef CONFIG_SCHED_PRED_LOAD
static ssize_t predl_window_size_show(struct kobject *kobj,
				      struct kobj_attribute *kattr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", predl_window_size);
}

#define MIN_PREDL_WINDOW_SIZE 4000000 /* 4ms */
#define MAX_PREDL_WINDOW_SIZE 100000000 /* 100ms */
static ssize_t predl_window_size_store(struct kobject *kobj,
				       struct kobj_attribute *kattr, const char *buf, size_t count)
{
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	if (val < MIN_PREDL_WINDOW_SIZE ||
	    val > MAX_PREDL_WINDOW_SIZE)
		return -EINVAL;

	predl_window_size = val;

	return count;
}
#endif

#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
static ssize_t walt_ravg_window_nr_ticks_show(struct kobject *kobj,
					      struct kobj_attribute *kattr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", sysctl_walt_ravg_window_nr_ticks);
}

static ssize_t walt_ravg_window_nr_ticks_store(struct kobject *kobj,
					       struct kobj_attribute *kattr, const char *buf, size_t count)
{
	int ret = count;
	unsigned int val;
	static DEFINE_MUTEX(mutex);

	mutex_lock(&mutex);

	if (kstrtouint(buf, 10, &val)) {
		ret = -EINVAL;
		goto unlock;
	}

	if (val == sysctl_walt_ravg_window_nr_ticks)
		goto unlock;

	if (val != 1 && val != 2 && val != 3 && val != 4 && val != 5 && val != 8) {
		ret = -EINVAL;
		goto unlock;
	}

	sysctl_walt_ravg_window_nr_ticks = val;
	sched_window_nr_ticks_change();

unlock:
	mutex_unlock(&mutex);
	return ret;
}
#endif

#ifdef CONFIG_MIPS_LOAD_TRACKING
static ssize_t mips_enable_show(struct kobject *kobj,
				      struct kobj_attribute *kattr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", mips_enable);
}

static ssize_t mips_enable_store(struct kobject *kobj,
				 struct kobj_attribute *kattr, const char *buf, size_t count)
{
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;
	int ret;
	if (val)
		ret = mips_start();
	else
		ret = mips_stop();

	return ret ?: count;;
}

#ifdef CONFIG_CPU_FREQ_PID_MIPS
static ssize_t freq_pid_show(struct kobject *kobj,
				struct kobj_attribute *kattr, char *buf)
{
	int i = 0;
	ssize_t ret = 0;

	while (i < PID_PARAM_NUM)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s",
				freq_pid[i++], ":");

	if (ret > 0)
		(void)sprintf_s(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	return ret;
}

static ssize_t freq_pid_store(struct kobject *kobj,
				struct kobj_attribute *kattr, const char *buf, size_t count)
{
	unsigned int *pid = NULL;
	int num = 0;
	int ret = count;

	pid = get_tokenized_data(buf, &num);
	if (IS_ERR(pid))
		return PTR_ERR(pid);

	if (num != PID_PARAM_NUM) {
		ret = -EINVAL;
		goto err;
	}

	int i = 0;

	while (i < PID_PARAM_NUM) {
		freq_pid[i] = pid[i];
		i++;
	}

err:
	kfree(pid);
	return ret;
}

static ssize_t speed_pid_show(struct kobject *kobj,
				struct kobj_attribute *kattr, char *buf)
{
	int i = 0;
	ssize_t ret = 0;

	while (i < PID_PARAM_NUM)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s",
				speed_pid[i++], ":");

	if (ret > 0)
		(void)sprintf_s(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	return ret;
}

static ssize_t speed_pid_store(struct kobject *kobj,
				struct kobj_attribute *kattr, const char *buf, size_t count)
{
	unsigned int *pid = NULL;
	int num = 0;
	int ret = count;

	pid = get_tokenized_data(buf, &num);
	if (IS_ERR(pid))
		return PTR_ERR(pid);

	if (num != PID_PARAM_NUM) {
		ret = -EINVAL;
		goto err;
	}

	int i = 0;

	while (i < PID_PARAM_NUM) {
		speed_pid[i] = pid[i];
		i++;
	}

err:
	kfree(pid);
	return ret;
}
#endif /* CONFIG_CPU_FREQ_PID_MIPS */
#endif

#ifdef CONFIG_DFX_STATS
static ssize_t thread_stats_show(struct kobject *kobj,
						struct kobj_attribute *kattr, char *buf)
{
	return dfx_thread_stats_show(kobj, kattr, buf);
}

static ssize_t cpu_stats_show(struct kobject *kobj,
						struct kobj_attribute *kattr, char *buf)
{
	return dfx_cpu_stats_show(kobj, kattr, buf);
}

static ssize_t preempt_disable_stats_show(struct kobject *kobj,
						struct kobj_attribute *kattr, char *buf)
{
	return dfx_preempt_disable_stats_show(kobj, kattr, buf);
}

static ssize_t irq_disable_stats_show(struct kobject *kobj,
						struct kobj_attribute *kattr, char *buf)
{
	return dfx_irq_disable_stats_show(kobj, kattr, buf);
}

static ssize_t dfx_enable_show(struct kobject *kobj,
						struct kobj_attribute *kattr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", dfx_stats_on);
}

static ssize_t dfx_enable_store(struct kobject *kobj,
						struct kobj_attribute *kattr, const char *buf, size_t count)
{
	u32 val = 0;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == dfx_stats_on)
		return -EINVAL;

	if (val)
		dfx_start();
	else
		dfx_stop();
	return count;
}

static ssize_t dfx_irq_preempt_stats_enable_show(struct kobject *kobj,
						struct kobj_attribute *kattr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", dfx_irq_preempt_stats_on);
}

static ssize_t dfx_irq_preempt_stats_enable_store(struct kobject *kobj,
						struct kobj_attribute *kattr, const char *buf, size_t count)
{
	u32 val = 0;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	dfx_irq_preempt_stats_on = val;
	return count;
}
#endif /* CONFIG_DFX_STATS */

/*
 * Note:
 * The field 'value' of eas_attr point to int, don't use type shorter than int.
 */
static struct eas_attr attrs[] = {
	{
		.attr = { .name = "boost", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &global_boost_enable,
	},
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0) || LINUX_VERSION_CODE >= KERNEL_VERSION(5,5,0))
	{
		.attr = { .name = "boot_boost", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &boot_boost,
	},
	{
		.attr = { .name = "task_boost_limit", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &task_boost_limit,
	},
	{
		.attr = { .name = "up_migration_util_filter", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &up_migration_util_filter,
	},
#endif
#ifdef CONFIG_MULTI_MARGIN
	{
		.attr = { .name = "capacity_margin", .mode = 0640, },
		.show = capacity_margin_show,
		.store = capacity_margin_store,
		.value = NULL,
	},
	{
		.attr = { .name = "sd_capacity_margin", .mode = 0640, },
		.show = sd_capacity_margin_show,
		.store = sd_capacity_margin_store,
		.value = NULL,
	},
#else
	{
		.attr = { .name = "capacity_margin", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &capacity_margin,
	},
	{
		.attr = { .name = "sd_capacity_margin", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &sd_capacity_margin,
	},
#endif
#ifdef CONFIG_SCHED_PRED_LOAD
	{
		.attr = { .name = "predl_enable", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &predl_enable,
	},
	{
		.attr = { .name = "predl_jump_load", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &predl_jump_load,
	},
	{
		.attr = { .name = "predl_jump_boost", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &predl_jump_boost,
	},
	{
		.attr = { .name = "predl_do_predict", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &predl_do_predict_ctrl,
	},
	{
		.attr = { .name = "predl_window_size", .mode = 0640, },
		.show = predl_window_size_show,
		.store = predl_window_size_store,
		.value = NULL,
	},
#endif
#ifdef CONFIG_MIPS_LOAD_TRACKING
	{
		.attr = { .name = "mips_enabled", .mode = 0640, },
		.show = mips_enable_show,
		.store = mips_enable_store,
		.value = NULL,
	},
	{
		.attr = { .name = "mips_task_enable", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &mips_task_enable,
	},
	{
		.attr = { .name = "mips_window_size", .mode = 0640, },
		.show = eas_show,
		.store = NULL,
		.value = &mips_window_size,
	},
	{
		.attr = { .name = "mips_task_report_policy", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &mips_task_report_policy,
	},
	{
		.attr = { .name = "mips_cpu_report_policy", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &mips_cpu_report_policy,
	},
#ifdef CONFIG_CPU_FREQ_PID_MIPS
	{
		.attr = { .name = "mips_pid_control_update", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &pid_control_update,
	},
	{
		.attr = { .name = "mips_pid_control_target_load", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &pid_control_target_load,
	},
	{
		.attr = { .name = "mips_freq_pid", .mode = 0640, },
		.show = freq_pid_show,
		.store = freq_pid_store,
		.value = NULL,
	},
	{
		.attr = { .name = "mips_speed_pid", .mode = 0640, },
		.show = speed_pid_show,
		.store = speed_pid_store,
		.value = NULL,
	},
#endif /* CONFIG_CPU_FREQ_PID_MIPS */
#endif
#ifdef CONFIG_SCHED_POWER_LIMIT_MAXFREQ
	{
		.attr = { .name = "power_limit_enabled", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &g_power_limit_enable,
	},
#endif
#ifdef CONFIG_SCHED_WALT
	{
		.attr = { .name = "walt_init_task_load_pct", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &sysctl_sched_walt_init_task_load_pct,
	},
#ifdef CONFIG_SCHED_WALT_DYNAMIC_WINDOW_SIZE
	{
		.attr = { .name = "walt_ravg_window_nr_ticks", .mode = 0640, },
		.show = walt_ravg_window_nr_ticks_show,
		.store = walt_ravg_window_nr_ticks_store,
		.value = NULL,
	},
#endif
#endif
#ifdef CONFIG_PRODUCT_ARMPC
	{
		.attr = { .name = "sync_hint", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &sysctl_sched_sync_hint_enable,
	},
#endif
#ifdef CONFIG_SMT_MODE_GOV
	{
		.attr = { .name = "task_cap_mt", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &sysctl_task_cap_discount_in_mt,
	},
	{
		.attr = { .name = "top_task_cap_mt", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &sysctl_top_task_cap_discount_in_mt,
	},
	{
		.attr = { .name = "cpu_cap_mt", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &sysctl_cpu_cap_discount_in_mt,
	},
#endif
#ifdef CONFIG_SCHED_SMT_USER_HIDE
	{
		.attr = { .name = "smt_hide", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &smt_hide_enable,
	},
	{
		.attr = { .name = "smt_visible_uid", .mode = 0640, },
		.show = smt_visible_uid_show,
		.store = smt_visible_uid_store,
		.value = NULL,
	},
	{
		.attr = { .name = "cim_visible_uid", .mode = 0640, },
		.show = cim_visible_uid_show,
		.store = cim_visible_uid_store,
		.value = NULL,
	},
#endif
#ifdef CONFIG_SCHED_CORE
	{
		.attr = { .name = "set_cookie_by_uid", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &sched_core_set_cookie_by_uid,
	},
#endif
#ifdef CONFIG_FUTEX_OPTIMISTIC_SPIN
	{
		.attr = { .name = "futex_opt_spin_enable", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &sysctl_futex_opt_spin_enable,
	},
#endif
#ifdef CONFIG_SCHED_VIP_PI
	{
		.attr = { .name = "specific_threads_vip_prio", .mode = 0640, },
		.show = eas_show,
		.store = eas_store,
		.value = &specific_threads_vip_prio,
	},
#endif
#ifdef CONFIG_DFX_STATS
	{
		.attr = { .name = "dfx_enable", .mode = 0640, },
		.show = dfx_enable_show,
		.store = dfx_enable_store,
		.value = &dfx_stats_on,
	},
	{
		.attr = { .name = "dfx_irq_preempt_stats_enable", .mode = 0640, },
		.show = dfx_irq_preempt_stats_enable_show,
		.store = dfx_irq_preempt_stats_enable_store,
		.value = &dfx_irq_preempt_stats_on,
	},
	{
		.attr = { .name = "thread_stats", .mode = 0440, },
		.show = thread_stats_show,
		.store = NULL,
		.value = NULL,
	},
	{
		.attr = { .name = "cpu_stats", .mode = 0440, },
		.show = cpu_stats_show,
		.store = NULL,
		.value = NULL,
	},
	{
		.attr = { .name = "preempt_disable_stats", .mode = 0440, },
		.show = preempt_disable_stats_show,
		.store = NULL,
		.value = NULL,
	},
	{
		.attr = { .name = "irq_disable_stats", .mode = 0440, },
		.show = irq_disable_stats_show,
		.store = NULL,
		.value = NULL,
	},
#endif /* CONFIG_DFX_STATS */
};

struct eas_data_struct {
	struct attribute_group attr_group;
	struct attribute *attributes[ARRAY_SIZE(attrs) + 1];
} eas_data;

static int eas_attr_init(void)
{
	int i, nr_attr;

	nr_attr = ARRAY_SIZE(attrs);
	for (i = 0; i < nr_attr; i++)
		eas_data.attributes[i] = &attrs[i].attr;
	eas_data.attributes[i] = NULL;

	eas_data.attr_group.name = "eas";
	eas_data.attr_group.attrs = eas_data.attributes;
	
	return sysfs_create_group(kernel_kobj, &eas_data.attr_group);
}
late_initcall(eas_attr_init);
