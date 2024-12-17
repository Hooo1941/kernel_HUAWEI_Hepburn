// SPDX-License-Identifier: GPL-2.0
/*
 * sched_limit.c
 *
 * sched running task rotation
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
#include <linux/cpumask.h>
#include <linux/topology.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/cpufreq.h>
#include <linux/devfreq.h>
#include <linux/platform_device.h>
#include <linux/init.h>

#include <platform_include/cee/linux/power_limit.h>

#include "sched.h"

struct limit_device {
	struct device_node *np;
	unsigned int divider;
};

struct task_struct *g_power_limit_update_kthread = NULL;
struct limit_device *g_limit_devices;
int g_limit_devices_num = 0;
bool g_power_limited = false;
bool g_power_limit_inited = false;
unsigned int g_power_limit_enable = 1;

static int limit_max_freq_qos_init(struct sched_cluster *cluster,
			struct cpufreq_policy *policy)
{
	char name[30];
	int  i;
	struct device_node *cn = NULL;
	int ret = 0;
	bool find = false;

	ret = sprintf_s(name, sizeof(name), "/cpus/cpu-map/cluster%d", cluster->id);
	if (ret < 0) {
		pr_err("sprintf_s error.\n");
		return ret;
	}
	cn = of_find_node_by_path(name);
	if (!cn) {
		pr_err("No Cluster%d information found\n", cluster);
		return -ENOENT;
	}
	for (i = 0; i < g_limit_devices_num; i++) {
		if (g_limit_devices[i].np == cn) {
			find = true;
			break;
		}
	}

	if (!find)
		return 0;

	cluster->power_limit_maxfreq = g_limit_devices[i].divider;
	ret = freq_qos_add_request(&policy->constraints, &(cluster->power_limit_maxfreq_req),
			FREQ_QOS_MAX, FREQ_QOS_MAX_DEFAULT_VALUE);
	if (ret < 0) {
		pr_err("%s: Failed to add freq constraint %d\n", __func__, ret);
		return ret;
	}

	cluster->power_limit_dev = true;

	return 0;
}

static void power_limit_dev_freq_qos_init(void)
{
	struct sched_cluster *cluster = NULL;

	for_each_sched_cluster(cluster) {
		unsigned int cpu = cpumask_first(&cluster->cpus);
		struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);

		if (!policy)
			return;

		if (limit_max_freq_qos_init(cluster, policy))
			pr_err("%s: Failed to init dev qos\n", __func__);

		cpufreq_cpu_put(policy);
	}
}

static int limit_checkpoint(void *data)
{
	int ret = 0;
	struct sched_cluster *cluster = NULL;
	unsigned int target_freq = FREQ_QOS_MAX_DEFAULT_VALUE;

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if (kthread_should_stop())
			break;

		for_each_sched_cluster(cluster) {
			if (!cluster->power_limit_dev)
				continue;

			if (g_power_limited)
				target_freq = cluster->power_limit_maxfreq;
			else
				target_freq = FREQ_QOS_MAX_DEFAULT_VALUE;

			ret = freq_qos_update_request(&(cluster->power_limit_maxfreq_req), target_freq);
			if (ret < 0)
				pr_err("%s: update qos fail, ret=%d\n", __func__, ret);
		}
	}

	return ret;
}

static int power_limit_kthread_create(void)
{
	struct sched_param param = { .sched_priority = MAX_RT_PRIO / 2 - 1 };

	g_power_limit_update_kthread = kthread_create(limit_checkpoint, NULL, "power-limit");
	if (IS_ERR(g_power_limit_update_kthread)) {
		pr_err("failed to create power-limit thread: %ld\n", PTR_ERR(g_power_limit_update_kthread));
		return PTR_ERR(g_power_limit_update_kthread);
	}
	sched_setscheduler_nocheck(g_power_limit_update_kthread, SCHED_FIFO, &param);

	return 0;
}

void power_limit_check(bool sched_avg_updated)
{
	unsigned int nr_big_task = 0;
	unsigned int nr_cpus = 0;
	struct sched_cluster *cluster = NULL;
	bool should_limit = false;

	if (!sched_avg_updated || !g_power_limit_inited)
		return;

	for_each_sched_cluster(cluster) {
		if (!cluster->power_limit_dev)
			continue;

		nr_cpus += cpumask_weight(&cluster->cpus);
		nr_big_task += sched_get_cpus_real_big_tasks(&cluster->cpus);
	}

	if (nr_big_task >= nr_cpus && g_power_limit_enable)
		should_limit = true;

	if (should_limit != g_power_limited) {
		g_power_limited = should_limit;
		wake_up_process(g_power_limit_update_kthread);
	}
}

static int power_limit_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct of_phandle_args dev_spec;
	int i, count, ret = 0;
	const char *prop = "limit-ruler";

	count = of_property_count_u32_elems(np, prop);
	count >>= 1;
	if (count <= 0) {
		dev_err(dev, "count %s err:%d\n", prop, count);
		return -ENOENT;
	}

	g_limit_devices = (struct limit_device *)devm_kzalloc(dev, sizeof(*g_limit_devices) * count, GFP_KERNEL);
	if (IS_ERR_OR_NULL(g_limit_devices)) {
		dev_err(dev, "alloc %s fail\n", prop);
		return -ENOMEM;
	}
	g_limit_devices_num = count;

	for (i = 0; i < count; i++) {
		if (of_parse_phandle_with_fixed_args(np, prop, 1, i, &dev_spec)) {
			dev_err(dev, "parse %s%d err\n", prop, i);
			return -ENOENT;
		}
		g_limit_devices[i].np = dev_spec.np;
		g_limit_devices[i].divider = dev_spec.args[0];
	}

	power_limit_dev_freq_qos_init();

	ret = power_limit_kthread_create();
	if (ret)
		pr_err("kthread create fail\n");

	g_power_limit_inited = true;

	return ret;
}

static const struct of_device_id limit_of_match[] = {
	{
		.compatible = "power-limit-maxfreq",
	},
	{} /* end */
};

static struct platform_driver limit_driver = {
	.probe = power_limit_probe,
	.driver = {
		.name = "power_limit",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(limit_of_match),
	},
};

static int __init power_limit_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&limit_driver);
	if (ret != 0)
		pr_err("%s: failed to register: %d.\n", __func__, ret);

	return ret;
}

late_initcall(power_limit_init);
