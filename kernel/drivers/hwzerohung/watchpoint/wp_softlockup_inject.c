/*
 * wp_softlockup_inject.c
 *
 * This file is use to cause softlockup
 *
 * Copyright (c) 2017-2021 Huawei Technologies Co., Ltd.
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
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/smpboot.h>

static spinlock_t softlockup_spinlock;
static DEFINE_PER_CPU(struct task_struct *, softlockup_inject_task);
static unsigned int softlockup_cpu = 0xff;

static int softlockup_inject_should_run(unsigned int cpu)
{
	return true;
}

static void softlockup_inject(unsigned int cpu)
{
	if (softlockup_cpu != cpu)
		return;

	spin_lock(&softlockup_spinlock);
	pr_err("%s lockup cpu = %d\n", __func__, cpu);
	do {} while (1);
}

static struct smp_hotplug_thread softlockup_inject_threads = {
	.store = &softlockup_inject_task,
	.thread_should_run = softlockup_inject_should_run,
	.thread_comm = "softlockup_inject/%u",
};

static void _softlockup_inject(void *data)
{
	pr_err("cpu = %d", smp_processor_id());
	wake_up_process(__this_cpu_read(softlockup_inject_task));
}

int hung_softlockup_inject(int cpu)
{
	struct cpumask cpumask;
	int err;

	spin_lock_init(&softlockup_spinlock);
	pr_err("current cpu = %d, lockup cpu = %d\n", smp_processor_id(), cpu);
	softlockup_inject_threads.thread_fn = softlockup_inject;

	softlockup_cpu = cpu;
	cpumask_clear(&cpumask);
	cpumask_set_cpu(cpu, &cpumask);

	err = smpboot_register_percpu_thread(&softlockup_inject_threads);
	if (err) {
		pr_err("Failed to create lockup threads\n");
		return 1;
	}

	smp_call_function_single(cpu, _softlockup_inject, NULL, 0);

	return 0;
}
