/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * mips.c
 *
 * for hisilicon efficinecy control algorithm mips load tracking.
 *
 * Copyright (c) 2023-2024 Huawei Technologies Co., Ltd.
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
#include <platform_include/cee/linux/mips_mem.h>
#include <platform_include/cee/linux/mips.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/topology.h>
#include <linux/cpu.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <trace/events/power.h>
#include <linux/sched/cpufreq.h>
#include <linux/cpu_pm.h>
#include <../../../kernel/sched/sched.h>

#define CREATE_TRACE_POINTS
#include <trace/events/mips_mem.h>

static struct kobject *g_mips_mem_global_kobject;

#define KHZ_PER_MHZ 1000
#define HZ_PER_MHZ 1000000

static DEFINE_SPINLOCK(state_lock);
struct core_dev_map {
	unsigned int core_mhz;
	unsigned long target_freq;
};

struct vote_reg {
	void __iomem *reg; /* dev frequency vote register */
	u32 mask;
	u8 bit;
};

struct dev_info {
	u32 *ratios;
	int nratios;
	unsigned int *ipc_min_sample_time_ns;
	int nipc_min_sample_time;
	u64 min_sample_time;
	int next_freq;
	u64 update_time;
	unsigned int mem_load_window_size_ns;
	struct vote_reg *hw_vote;
	struct core_dev_map *freq_map;
	unsigned int map_len;
	bool enable;
	struct kobject kobj;
};

struct mipsmem_cluster_policy {
	cpumask_t cpu_mask;
	struct dev_info *ddr_dev;
	struct dev_info *l3c_dev;
	struct kobject kobj;
};

struct mipsmem_cpu_info {
	int cpu;
	bool enable;
	struct mipsmem_cluster_policy *policy;
	struct notifier_block mips_cpu_idle_nb;
	u32 next_l3_freq;
	u32 next_ddr_freq;
	u64 l3_min_sample_time;
	u64 ddr_min_sample_time;
};

static DEFINE_PER_CPU(struct mipsmem_cpu_info, cpu_info);

#define DEFAULT_TARGET_RATIO 600
#define DEFAULT_IPC_MIN_SAMPLE_TIME 4000000
#define DEFAULT_MEM_WINDOW_SIZE	4000000
static unsigned int default_target_ratios[] = {DEFAULT_TARGET_RATIO};
static unsigned int default_ipc_min_sample_time[] = {DEFAULT_IPC_MIN_SAMPLE_TIME};

static u32 core_to_dev_freq(struct dev_info *info, u16 coref)
{
	struct core_dev_map *map = info->freq_map;
	unsigned long freq = 0;

	if (unlikely(map == NULL))
		goto out;

	while (map->core_mhz && map->core_mhz < coref)
		map++;

	if (map->core_mhz == 0)
		map--;

	freq = map->target_freq / HZ_PER_MHZ;

out:
	pr_debug("freq: %lu -> dev: %lu\n", coref, freq);
	return freq;
}

static bool mem_enable(struct mipsmem_cpu_info *mem_cpu)
{
	struct mipsmem_cluster_policy *mem_policy = mem_cpu->policy;

	if (!mem_cpu->enable)
		return false;
	if (!mips_enable)
		return false;
	if (!mem_policy)
		return false;
	if (!mem_policy->l3c_dev || !mem_policy->ddr_dev)
		return false;

	return mem_policy->l3c_dev->enable && mem_policy->ddr_dev->enable;
}
static void print_mem_freq(void)
{
	int cpu;
	struct mipsmem_cpu_info *mem_cpu = NULL;
	int max_ddrfreq = 0;
	int max_l3cfreq = 0;
	int l3_id = 0;
	int ddr_id = 0;

	for_each_online_cpu(cpu) {
		mem_cpu = &per_cpu(cpu_info, cpu);
		if (mem_cpu->next_ddr_freq > max_ddrfreq) {
			max_ddrfreq = mem_cpu->next_ddr_freq;
			ddr_id = cpu;
		}
		if (mem_cpu->next_l3_freq > max_l3cfreq) {
			max_l3cfreq = mem_cpu->next_l3_freq;
			l3_id = cpu;
		}
	}
	trace_perf(clock_set_rate, "mipsddrfreq", max_ddrfreq, ddr_id);
	trace_perf(clock_set_rate, "mipsl3cfreq", max_l3cfreq, l3_id);
}

static void set_hw_vote(struct dev_info *info, int max_freq, u64 min_sample_time)
{
	u32 value;
	u64 now;

	now = sched_ktime_clock();
	if (max_freq != info->next_freq && now - info->update_time > min_sample_time) {
		info->next_freq = max_freq;
		value = (max_freq << info->hw_vote->bit) | info->hw_vote->mask;
		info->update_time = now;
		writel(value, info->hw_vote->reg);
		print_mem_freq();
	}
}

static void change_mem_freq(struct mipsmem_cpu_info *cpum)
{
	int max_l3_freq = 0;
	int max_ddr_freq = 0;
	int cpu;
	struct mipsmem_cluster_policy *mem_policy = cpum->policy;
	struct mipsmem_cpu_info *mem_cpu = NULL;
	unsigned long flags;

	for_each_cpu(cpu, &mem_policy->cpu_mask) {
		mem_cpu = &per_cpu(cpu_info, cpu);
		max_l3_freq = max(max_l3_freq, mem_cpu->next_l3_freq);
		max_ddr_freq = max(max_ddr_freq, mem_cpu->next_ddr_freq);
	}
	set_hw_vote(mem_policy->l3c_dev, max_l3_freq, cpum->l3_min_sample_time);
	set_hw_vote(mem_policy->ddr_dev, max_ddr_freq, cpum->ddr_min_sample_time);
}

static u32 freq_to_targetratio(u32 freq, u32 *ratios, int len)
{
	int i;

	for (i = 0; i < len - 1 && freq >= ratios[i + 1]; i += 2)
		;
	return ratios[i];
}

static u32 get_max_possible_freq(struct dev_info *dev, u32 ratio, u32 freq)
{
	int i;
	u32 next_freq;
	u32 max_possible_freq = 0;
	int index = -1;
	struct core_dev_map *map = dev->freq_map;

	for (i = dev->nratios - 1; i >= 0; i -= 2) {
		if (ratio <= dev->ratios[i]) {
			index = i;
			break;
		}
	}

	if (index != -1) {
		index++;
		if (index >= dev->nratios)
			return 0;
		next_freq = dev->ratios[index];
		if (next_freq <= freq) {
			while (map->core_mhz < next_freq) {
				max_possible_freq = map->core_mhz;
				map++;
			}
			return max_possible_freq;
		}
	}
	return 0;
}

static u64 ipc_to_min_sample_time(struct dev_info *info, unsigned int ipc)
{
	unsigned int ret;
	int i;

	for (i = 0; i < info->nipc_min_sample_time - 1 &&
	     ipc >= info->ipc_min_sample_time_ns[i + 1]; i += 2)
		;

	ret = info->ipc_min_sample_time_ns[i];

	return ret;
}

static u32 mips_mem_get_freq(struct dev_info *info, u32 ratio, u32 core_freq)
{
	u32 target_ratio_ceil;
	u32 freq = 0;

	if (info == NULL || !info->enable)
		return 0;

	target_ratio_ceil = freq_to_targetratio(core_freq, info->ratios, info->nratios);
	if (ratio <= target_ratio_ceil)
		freq = core_freq;
	else
		freq = get_max_possible_freq(info, ratio, core_freq);
	return core_to_dev_freq(info, freq);
}

void check_l3_ddr_freq_change(int cpu)
{
	int i, freq;
	struct mipsmem_cpu_info *mem_cpu = &per_cpu(cpu_info, cpu);
	struct mipsmem_cluster_policy *mem_policy = mem_cpu->policy;
	u32 l3_ratio = UINT_MAX;
	u32 ddr_ratio = UINT_MAX;
	unsigned int ipc;
	struct pcounts pcount;
	u64 now = sched_ktime_clock();
	int new_window;
	unsigned long flags;

	spin_lock_irqsave(&state_lock, flags);
	if (!mem_enable(mem_cpu))
		goto unlock;

	u64 mem_window_size = mem_policy->ddr_dev->mem_load_window_size_ns;
	int nr_window = update_mips_window_start(&cpu_rq(cpu)->mips_mem_window_start,
						 mem_window_size, now);
	if (!nr_window)
		goto unlock;

	int nr_mem_load_window = mem_window_size / mips_window_size;

	get_mips_mem_info(cpu, &pcount, nr_mem_load_window);
	if (pcount.l2refill != 0)
		l3_ratio = pcount.inst / pcount.l2refill;
	if (pcount.l3refill != 0)
		ddr_ratio = pcount.inst / pcount.l3refill;
	freq = pcount.freq / KHZ_PER_MHZ;

	ipc = div_u64_protected(pcount.inst * 100, pcount.cycle);
	mem_cpu->next_l3_freq = mips_mem_get_freq(mem_policy->l3c_dev, l3_ratio, freq);
	mem_cpu->next_ddr_freq = mips_mem_get_freq(mem_policy->ddr_dev, ddr_ratio, freq);
	mem_cpu->l3_min_sample_time = ipc_to_min_sample_time(mem_policy->l3c_dev, ipc);
	mem_cpu->ddr_min_sample_time = ipc_to_min_sample_time(mem_policy->ddr_dev, ipc);
	trace_check_l3_ddr_freq_change(cpu, &pcount, l3_ratio, ddr_ratio,
				       mem_cpu->next_l3_freq, mem_cpu->next_ddr_freq);
	change_mem_freq(mem_cpu);
unlock:
	spin_unlock_irqrestore(&state_lock, flags);
}

static int get_mask_from_dev_handle(struct platform_device *pdev,
				    cpumask_t *mask)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cpu_np = NULL;
	struct device_node *mon_np = NULL;
	int i, cpu;

	cpumask_clear(mask);

	for_each_possible_cpu(cpu) {
		cpu_np = of_get_cpu_node(cpu, NULL);
		if (!cpu_np) {
			pr_err("%s: failed to get cpu%d node\n",
			       __func__, cpu);
			return -ENOENT;
		}

		/* Get mipsmem monitor descriptor node */
		for (i = 0; ; i++) {
			mon_np = of_parse_phandle(cpu_np, "mips-mem-monitors", i);
			if (!mon_np)
				break;

			/* CPUs are sharing mipsmem monitor node */
			if (np == mon_np) {
				cpumask_set_cpu(cpu, mask);
				of_node_put(mon_np);
				break;
			}
			of_node_put(mon_np);
		}

		of_node_put(cpu_np);
	}

	if (cpumask_empty(mask))
		return -ENOENT;

	return 0;
}

static int cmd_parse(char *para_cmd, char *argv[], int max_args)
{
	int para_id = 0;

	while (*para_cmd != '\0') {
		if (para_id >= max_args)
			break;

		while (*para_cmd == ' ')
			para_cmd++;

		if (*para_cmd == '\0')
			break;

		argv[para_id] = para_cmd;
		para_id++;

		while ((*para_cmd != ' ') && (*para_cmd != '\0'))
			para_cmd++;

		if (*para_cmd == '\0')
			break;

		*para_cmd = '\0';
		para_cmd++;
	}

	return para_id;
}

static ssize_t store_freq_map(struct dev_info *info, const char *buf, size_t count)
{
	char local_buf[PAGE_SIZE] = {0};
	char *argv[3] = {0};
	int argc, ret;
	u32 core_freq, idx;
	u64 target_freq;
	struct core_dev_map *map = info->freq_map;

	ret = strncpy_s(local_buf, PAGE_SIZE, buf,
			min_t(size_t, sizeof(local_buf) - 1, count));
	if (ret != EOK)
		return -EINVAL;
	argc = cmd_parse(local_buf, argv, ARRAY_SIZE(argv));
	if (argc != 3) {
		pr_err("error, only surport three para\n");
		return -EINVAL;
	}

	ret = sscanf_s(argv[0], "%u", &idx);
	if (ret != 1 || idx >= info->map_len)
		return -EINVAL;
	ret = sscanf_s(argv[1], "%u", &core_freq);
	if (ret != 1)
		return -EINVAL;
	ret = sscanf_s(argv[2], "%llu", &target_freq);
	if (ret != 1)
		return -EINVAL;

	map = map + idx;
	map->core_mhz = core_freq;
	map->target_freq = target_freq;
	return count;
}

static ssize_t show_freq_map(const struct dev_info *info, char *buf)
{
	struct core_dev_map *map = info->freq_map;
	unsigned int cnt = 0;

	cnt += scnprintf(buf, PAGE_SIZE, "Core freq (MHz)\tDevice BW\n");

	while (map->core_mhz && cnt < PAGE_SIZE) {
		cnt += scnprintf(buf + cnt, PAGE_SIZE - cnt, "%15u\t%9lu\n",
				 map->core_mhz, map->target_freq);
		map++;
	}
	if (cnt < PAGE_SIZE)
		cnt += scnprintf(buf + cnt, PAGE_SIZE - cnt, "\n");

	return cnt;
}

static unsigned int *get_tokenized_data(const char *buf, int *num_tokens)
{
	const char *cp;
	unsigned int i;
	unsigned int ntokens = 1;
	unsigned int *tokenized_data = NULL;
	int err = -EINVAL;

	cp = buf;
	while ((cp = strpbrk(cp + 1, " :")))
		ntokens++;

	if (!(ntokens & 0x1))
		goto err;

	tokenized_data = kmalloc(ntokens * sizeof(unsigned int), GFP_KERNEL);
	if (tokenized_data == NULL) {
		err = -ENOMEM;
		goto err;
	}

	cp = buf;
	i = 0;
	while (i < ntokens) {
		if (sscanf_s(cp, "%u", &tokenized_data[i++]) != 1)
			goto err_kfree;

		cp = strpbrk(cp, " :");
		if (cp == NULL)
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

static ssize_t store_target_ratio(struct dev_info *info, const char *buf, size_t count)
{
	int ntokens = 0;
	unsigned int *new_target_ratios = NULL;
	int i;

	new_target_ratios = get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_target_ratios))
		return PTR_ERR_OR_ZERO(new_target_ratios);

	for (i = 0; i < ntokens; i++)
		if (i & 0x1)
			new_target_ratios[i] /= KHZ_PER_MHZ;

	if (info->ratios != default_target_ratios)
		kfree(info->ratios);

	info->ratios = new_target_ratios;
	info->nratios = ntokens;
	return count;
}

static ssize_t show_target_ratio(const struct dev_info *info, char *buf)
{
	unsigned int i;
	ssize_t ret = 0;

	for (i = 0; i < (unsigned int)(info->nratios); i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s",
				(i & 0x1) ? (info->ratios[i] * KHZ_PER_MHZ)
					: info->ratios[i],
				(i & 0x1) ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	return ret;
}

static ssize_t show_mipsmem_enable(const struct dev_info *info, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n", info->enable);
}

#define TRANS_BASE	10
static ssize_t store_mipsmem_enable(struct dev_info *info, const char *buf, size_t count)
{
	int  val;
	unsigned long flags;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;
	spin_lock_irqsave(&state_lock, flags);
	info->enable = !!val;
	if (!info->enable)
		set_hw_vote(info, 0, 0);
	spin_unlock_irqrestore(&state_lock, flags);
	return count;
}

static ssize_t show_ipc_min_sample_time(const struct dev_info *info, char *buf)
{
	ssize_t ret = 0;
	int i;

	for (i = 0; i < info->nipc_min_sample_time; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s", info->ipc_min_sample_time_ns[i],
				 i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	return ret;
}

static ssize_t store_ipc_min_sample_time(struct dev_info *info, const char *buf, size_t count)
{
	unsigned int *new_ipc_min_sample_time;
	int ntokens;
	int i;

	new_ipc_min_sample_time = cpufreq_get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_ipc_min_sample_time))
		return PTR_ERR(new_ipc_min_sample_time);

	for (i = 0; i < ntokens; i++) {
		if (new_ipc_min_sample_time[i] == 0) {
			kfree(new_ipc_min_sample_time);
			return -EINVAL;
		}
	}

	if (info->ipc_min_sample_time_ns != default_ipc_min_sample_time)
		kfree(info->ipc_min_sample_time_ns);
	info->ipc_min_sample_time_ns = new_ipc_min_sample_time;
	info->nipc_min_sample_time = ntokens;

	return count;
}

static ssize_t show_mem_load_window_size_ns(const struct dev_info *info, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n", info->mem_load_window_size_ns);
}

static ssize_t store_mem_load_window_size_ns(struct dev_info *info, const char *buf, size_t count)
{
	unsigned int val;

	if (kstrtouint(buf, TRANS_BASE, &val) != 0)
		return -EINVAL;
	if (val < MIN_MIPS_WINDOW_SIZE || val > MAX_MIPS_WINDOW_SIZE)
		return -EINVAL;
	info->mem_load_window_size_ns = val;
	return count;
}

#define to_cluster_data(k) container_of(k, struct dev_info, kobj)
#define to_attr(a) container_of(a, struct mipsmem_attr, attr)
struct mipsmem_attr {
	struct attribute attr;
	ssize_t (*show)(const struct dev_info *, char *);
	ssize_t (*store)(struct dev_info *, const char *, size_t count);
};

#define mipsmem_attr_ro(_name)		\
	static struct mipsmem_attr _name = 	\
	__ATTR(_name, 0440, show_##_name, NULL)

#define mipsmem_attr_rw(_name)		\
	static struct mipsmem_attr _name = 		\
	__ATTR(_name, 0640, show_##_name, store_##_name)

mipsmem_attr_rw(freq_map);
mipsmem_attr_rw(target_ratio);
mipsmem_attr_rw(mipsmem_enable);
mipsmem_attr_rw(ipc_min_sample_time);
mipsmem_attr_rw(mem_load_window_size_ns);

static ssize_t show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct dev_info *data = to_cluster_data(kobj);
	struct mipsmem_attr *mattr = to_attr(attr);
	ssize_t ret = -EIO;

	if (mattr->show != NULL)
		ret = mattr->show(data, buf);

	return ret;
}

static ssize_t store(struct kobject *kobj, struct attribute *attr,
		     const char *buf, size_t count)
{
	struct dev_info *data = to_cluster_data(kobj);
	struct mipsmem_attr *mattr = to_attr(attr);
	ssize_t ret = -EIO;

	if (mattr->store != NULL)
		ret = mattr->store(data, buf, count);

	return ret;
}

static struct attribute *mips_attrs[] = {
	&freq_map.attr,
	&target_ratio.attr,
	&mipsmem_enable.attr,
	&ipc_min_sample_time.attr,
	&mem_load_window_size_ns.attr,
	NULL,
};

static const struct sysfs_ops sysfs_ops = {
	.show   = show,
	.store  = store,
};

static struct kobj_type ktype_mips = {
	.sysfs_ops	  = &sysfs_ops,
	.default_attrs  = mips_attrs,
};

static struct attribute *default_attrs[] = {
	NULL,
};

static struct kobj_type ktype_default = {
	.default_attrs  = default_attrs,
};

static void clear_mem_freq(struct mipsmem_cpu_info *mem_cpu)
{
	mem_cpu->next_ddr_freq = 0;
	mem_cpu->next_l3_freq = 0;
	mem_cpu->l3_min_sample_time = 0;
	mem_cpu->ddr_min_sample_time = 0;
}

static void mips_mem_idle_clear_freq(struct mipsmem_cpu_info *mem_cpu)
{
	unsigned long flags;

	spin_lock_irqsave(&state_lock, flags);
	if (!mem_enable(mem_cpu))
		goto unlock;

	clear_mem_freq(mem_cpu);
	change_mem_freq(mem_cpu);
unlock:
	spin_unlock_irqrestore(&state_lock, flags);
}
void set_mips_mem_state(bool enabled)
{
	int cpu;
	unsigned long flags;
	struct dev_info *ddr_info;
	struct dev_info *l3_info;

	for_each_online_cpu(cpu) {
		struct mipsmem_cpu_info *mem_cpu = &per_cpu(cpu_info, cpu);

		if (!mem_cpu->policy)
			continue;

		if (cpu == cpumask_first(&mem_cpu->policy->cpu_mask)) {
			spin_lock_irqsave(&state_lock, flags);
			ddr_info = mem_cpu->policy->ddr_dev;
			l3_info = mem_cpu->policy->l3c_dev;
			ddr_info->enable = enabled;
			l3_info->enable = enabled;
			if (!enabled) {
				set_hw_vote(l3_info, 0, 0);
				set_hw_vote(ddr_info, 0, 0);
			}
			spin_unlock_irqrestore(&state_lock, flags);
		}
	}
}

static int mips_mem_idle_notif(struct notifier_block *nb, unsigned long action,
			       void *data __maybe_unused)
{
	int cpu = smp_processor_id();
	struct mipsmem_cpu_info *mem_cpu = &per_cpu(cpu_info, cpu);

	switch (action) {
	case CPU_PM_ENTER:
		mips_mem_idle_clear_freq(mem_cpu);
		break;

	case CPU_PM_ENTER_FAILED:
		break;
	case CPU_PM_EXIT:
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static struct mipsmem_cluster_policy *mipsmem_cluster_alloc_and_init(struct platform_device *pdev,
								     cpumask_t *cpus)
{
	struct mipsmem_cluster_policy *policy = NULL;
	int ret = 0;
	int cpu;
	struct device *dev = &pdev->dev;
	struct device *cpu_dev;
	struct mipsmem_cpu_info *mem_cpu = &per_cpu(cpu_info, cpumask_first(cpus));

	policy = mem_cpu->policy;
	if (policy)
		return policy;
	policy = devm_kzalloc(dev, sizeof(*policy), GFP_KERNEL);
	if (IS_ERR_OR_NULL(policy)) {
		dev_err(dev, "mips mem alloc cluster err\n");
		return NULL;
	}
	cpumask_copy(&policy->cpu_mask, cpus);

	ret = kobject_init_and_add(&policy->kobj, &ktype_default,
				   g_mips_mem_global_kobject, "cluster%u",
				   topology_physical_package_id(cpumask_first(cpus)));
	if (ret) {
		dev_err(dev, "mips: failed to init cluster->kobj: %d\n", ret);
		kobject_put(&policy->kobj);
		goto error;
	}

	for_each_cpu(cpu, cpus) {
		cpu_dev = get_cpu_device(cpu);
		ret = sysfs_create_link(&cpu_dev->kobj, &policy->kobj, "mips_mem");
		mem_cpu = &per_cpu(cpu_info, cpu);
			if (ret) {
				dev_err(dev, "mips: symlink creation cpu%d failed\n", cpu);
				kobject_put(&policy->kobj);
				goto error;
			}
			mem_cpu->policy = policy;
			mem_cpu->cpu = cpu;
			mem_cpu->enable = true;
		mem_cpu->mips_cpu_idle_nb.notifier_call = mips_mem_idle_notif;
		cpu_pm_register_notifier(&mem_cpu->mips_cpu_idle_nb);
	}
	return policy;
error:
	devm_kfree(dev, policy);
	return NULL;
}

#define NUM_COLS	2

static struct core_dev_map *init_core_dev_map(struct device *dev,
					      const char *prop_name, int *map_len)
{
	int len, nf, i, j;
	u32 data;
	struct core_dev_map *tbl = NULL;
	int ret;

	if (of_find_property(dev->of_node, prop_name, &len) == NULL)
		return NULL;

	len /= sizeof(data);

	if ((len % NUM_COLS) != 0 || len == 0)
		return NULL;

	nf = len / NUM_COLS;

	tbl = devm_kzalloc(dev, (nf + 1) * sizeof(struct core_dev_map),
			   GFP_KERNEL);
	if (tbl == NULL)
		return NULL;

	for (i = 0, j = 0; i < nf; i++, j += 2) {
		ret = of_property_read_u32_index(dev->of_node, prop_name, j,
						 &data);
		if (ret != 0)
			return NULL;

		tbl[i].core_mhz = data / KHZ_PER_MHZ;

		ret = of_property_read_u32_index(dev->of_node, prop_name, j + 1,
						 &data);
		if (ret != 0)
			return NULL;

		tbl[i].target_freq = data * HZ_PER_MHZ;
		pr_debug("Entry%d CPU:%u, Dev:%lu\n", i, tbl[i].core_mhz,
			 tbl[i].target_freq);
	}
	tbl[i].core_mhz = 0;
	*map_len = nf;

	return tbl;
}

#define VOTE_REG_NUM	2
#define MBITS_LEFT_SHIFT	16
static struct vote_reg *init_hw_vote(struct device *dev)
{
	void __iomem *reg_base = NULL;
	struct vote_reg *hw_vote = NULL;
	u32 temp[VOTE_REG_NUM] = {0};
	u32 reg_mask = 0;
	u32 reg_bit;
	int ret;

	reg_base = of_iomap(dev->of_node, 0);
	if (reg_base == NULL) {
		dev_err(dev, "fail to map io!\n");
		return NULL;
	}
	hw_vote = devm_kzalloc(dev, sizeof(*hw_vote), GFP_KERNEL);
	if (hw_vote == NULL) {
		dev_err(dev, "[%s] node %s doesn't have hw_vote alloc fail!\n",
			__func__, dev->of_node->name);
		goto numap_base;
	}
	if (of_property_read_u32_array(dev->of_node, "vote-reg",
				       &temp[0], VOTE_REG_NUM) != 0) {
		dev_err(dev, "[%s] node %s doesn't have vote-reg property!\n",
			__func__, dev->of_node->name);
		goto free_hw;
	}
	hw_vote->reg = reg_base + temp[0];

	if (of_property_read_u32(dev->of_node, "vote-reg-mask",
				 &reg_mask) != 0) {
		dev_err(dev, "[%s] node %s doesn't have vote-reg mask property!\n",
			__func__, dev->of_node->name);
		goto free_hw;
	}
	hw_vote->mask = reg_mask;

	if (of_property_read_u32(dev->of_node, "vote-reg-bit", &reg_bit) != 0) {
		dev_err(dev, "[%s] node %s doesn't have vote-reg bit property!\n",
			__func__, dev->of_node->name);
		goto free_hw;
	}
	hw_vote->bit = reg_bit;
	return hw_vote;
free_hw:
	devm_kfree(dev, hw_vote);
numap_base:
	iounmap(reg_base);
	return NULL;
}

static void init_vote_max_freq(struct dev_info *dev_info)
{
	unsigned long init_freq = dev_info->freq_map[dev_info->map_len - 1].target_freq / HZ_PER_MHZ;
	set_hw_vote(dev_info, (int)init_freq, 0);
}

static int mipsmem_dev_alloc_and_init(struct platform_device *pdev, struct mipsmem_cluster_policy *policy)
{
	struct dev_info *dev_info = NULL;
	struct device *dev = &pdev->dev;
	int ret;

	dev_info = devm_kzalloc(dev, sizeof(*dev_info), GFP_KERNEL);
	if (dev_info == NULL) {
		dev_err(dev, "[%s] node %s doesn't have dev_info alloc fail!\n",
			__func__, dev->of_node->name);
		return -ENOMEM;
	}
	if (strstr(pdev->name, "l3")) {
		policy->l3c_dev = dev_info;
		ret = kobject_init_and_add(&dev_info->kobj, &ktype_mips,
					   &policy->kobj, "mips_l3");

		dev_info(dev, "mips: l3 name: %s\n", pdev->name);
	} else {
		policy->ddr_dev = dev_info;
		ret = kobject_init_and_add(&dev_info->kobj, &ktype_mips,
					   &policy->kobj, "mips_ddr");
		dev_info(dev, "mips: ddr name: %s\n", pdev->name);
	}
	if (ret) {
		dev_err(dev, "mips: failed to init dev_info->kobj: %d\n", ret);
		goto error;
	}

	dev_info->freq_map = init_core_dev_map(dev, "core-dev-table", &dev_info->map_len);
	if (dev_info->freq_map == NULL) {
		dev_err(dev, "Couldn't find the freq map table!\n");
		ret = -EINVAL;
		goto error;
	}

	dev_info->hw_vote = init_hw_vote(dev);
	if (dev_info->hw_vote == NULL) {
		dev_err(dev, "Couldn't find the hw_vote!\n");
		ret = -EINVAL;
		goto error;
	}

	dev_info->nratios = ARRAY_SIZE(default_target_ratios);
	dev_info->ratios = default_target_ratios;
	dev_info->ipc_min_sample_time_ns = default_ipc_min_sample_time;
	dev_info->nipc_min_sample_time = ARRAY_SIZE(default_ipc_min_sample_time);
	dev_info->mem_load_window_size_ns = DEFAULT_MEM_WINDOW_SIZE;
	init_vote_max_freq(dev_info);
	return 0;
error:
	kobject_put(&policy->kobj);
	devm_kfree(dev, dev_info);
	return ret;
}

static int mips_driver_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	cpumask_t cpus;
	int ret;
	struct mipsmem_cluster_policy *policy = NULL;

	if (get_mask_from_dev_handle(pdev, &cpus) != 0) {
		dev_err(dev, "No CPU use this mon\n");
		return -ENODEV;
	}

	policy = mipsmem_cluster_alloc_and_init(pdev, &cpus);
	if (!policy) {
		dev_err(dev, "init cluster fail\n");
		return -ENOMEM;
	}

	ret = mipsmem_dev_alloc_and_init(pdev, policy);
	if (ret) {
		dev_err(dev, "init dev fail\n");
		devm_kfree(dev, policy);
	}
	return ret;
}

static int __ref mips_mem_online(unsigned int cpu)
{
	unsigned long flags;
	struct mipsmem_cpu_info *mem_cpu = &per_cpu(cpu_info, cpu);

	spin_lock_irqsave(&state_lock, flags);
	mem_cpu->enable = true;
	spin_unlock_irqrestore(&state_lock, flags);
	return 0;
}

static int __ref mips_mem_offline(unsigned int cpu)
{
	unsigned long flags;
	struct dev_info *ddr_info;
	struct dev_info *l3_info;
	struct mipsmem_cpu_info *mem_cpu = &per_cpu(cpu_info, cpu);

	spin_lock_irqsave(&state_lock, flags);
	if (!mem_enable(mem_cpu))
		goto unlock;
	clear_mem_freq(mem_cpu);
	change_mem_freq(mem_cpu);
unlock:
	mem_cpu->enable = false;
	spin_unlock_irqrestore(&state_lock, flags);
	return 0;
}

static const struct of_device_id match_table[] = {
	{ .compatible = "mips-mem" },
	{}
};

static struct platform_driver mips_mem_mon_driver = {
	.probe = mips_driver_probe,
	.driver = {
		.name = "mips-mem",
		.of_match_table = match_table,
		.owner = THIS_MODULE,
	},
};

static int __init mips_mem_init(void)
{
	int ret;
	g_mips_mem_global_kobject = kobject_create_and_add("mips_mem", &cpu_subsys.dev_root->kobj);
	if (!g_mips_mem_global_kobject)
		return -ENOMEM;
	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN, "mips-mem:online",
					mips_mem_online, mips_mem_offline);
	if (ret < 0) {
		pr_err("mips mem set hotplug state failed\n");
		return ret;
	}
	return platform_driver_register(&mips_mem_mon_driver);
}
module_init(mips_mem_init);
