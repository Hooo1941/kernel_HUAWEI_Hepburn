// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description:
 * Miscellaneous little independent features.
 */
#include "sched.h"
#include <linux/cpu_pm.h>
#include <linux/notifier.h>
#include <linux/smp.h>

#ifdef CONFIG_GICD_MISS_PPI_WAKE_REQ_WORKAROUND
struct cpumask __cpu_powerdown_mask;
#define cpu_powerdown_mask (&__cpu_powerdown_mask)

static DEFINE_SPINLOCK(cpu_pm_lock);
static DEFINE_PER_CPU(ktime_t, last_ipi_pending);
static DEFINE_PER_CPU(ktime_t, last_ipi_sent);

static int cpu_pm_notifier(struct notifier_block *nb, unsigned long action,
			void *data)
{
	int cpu = smp_processor_id();
	unsigned long flags;

	switch (action) {
	case CPU_PM_ENTER:
		spin_lock_irqsave(&cpu_pm_lock, flags);
		cpumask_set_cpu(cpu, cpu_powerdown_mask);
		per_cpu(last_ipi_pending, cpu) = 0;
		per_cpu(last_ipi_sent, cpu) = 0;
		spin_unlock_irqrestore(&cpu_pm_lock, flags);
		break;
	case CPU_PM_ENTER_FAILED:
	case CPU_PM_EXIT:
		spin_lock_irqsave(&cpu_pm_lock, flags);
		cpumask_clear_cpu(cpu, cpu_powerdown_mask);
		per_cpu(last_ipi_pending, cpu) = 0;
		per_cpu(last_ipi_sent, cpu) = 0;
		spin_unlock_irqrestore(&cpu_pm_lock, flags);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block cpu_pm_notifier_block = {
	.notifier_call = cpu_pm_notifier,
};

static int __init cpu_pm_notifier_init(void)
{
	if (cpu_pm_register_notifier(&cpu_pm_notifier_block))
		pr_err("%s/%s: error\n", __FILE__, __func__);
	return 0;
}
late_initcall(cpu_pm_notifier_init);

static bool ipi_resched_pending(int cpu)
{
	return test_tsk_need_resched(cpu_rq(cpu)->curr);
}

/*
 * To be robust to unknow bugs, just send ipi once in 1s when cpu
 * keeps powered down.
 */
#define SEND_IPI_INTERVAL ns_to_ktime(1 * NSEC_PER_SEC) /* 1s */
static void send_ipi_wakeup(int cpu, ktime_t now)
{
	ktime_t last_sent = per_cpu(last_ipi_sent, cpu);

	if (last_sent == 0 || now > last_sent + SEND_IPI_INTERVAL) {
		pr_err("cpu %d seems unresponsitive to ipi\n",	cpu);
		smp_send_reschedule(cpu);
		per_cpu(last_ipi_sent, cpu) = now;
	}
}

#define IPI_UNUSUAL_LATENCY ns_to_ktime(3500 * NSEC_PER_USEC) /* 3.5ms */
static void suspect_ipi_miss(int cpu)
{
	ktime_t now = ktime_get();
	ktime_t last = per_cpu(last_ipi_pending, cpu);

	if (last > 0 && now > last + IPI_UNUSUAL_LATENCY)
		send_ipi_wakeup(cpu, now);

	per_cpu(last_ipi_pending, cpu) = now;
}

/*
 * Arm errata 1717652: Wake_request may not be delivered if multiple
 * cores are woken by PPIs at the same time. (WFI is not affected)
 * Since IPI_FUNCTION_CALL and IPI_RESCHEDULE only send ipi for the
 * first time and omit future ipis until cpu response and clear some
 * software flags. We must detect ipi pending for unusual time on
 * powered down cpu and send a new ipi to wakeup it as a workaround.
 */
void check_gic_missed_wakeup(void)
{
	int cpu;
	unsigned long flags;

	spin_lock_irqsave(&cpu_pm_lock, flags);
	for_each_cpu_and(cpu, cpu_powerdown_mask, cpu_online_mask) {
		if (ipi_resched_pending(cpu) ||
		    ipi_function_call_pending(cpu))
			suspect_ipi_miss(cpu);
	}
	spin_unlock_irqrestore(&cpu_pm_lock, flags);
}
#endif /* CONFIG_GICD_MISS_PPI_WAKE_REQ_WORKAROUND */
