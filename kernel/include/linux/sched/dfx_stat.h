/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LINUX_DFX_STAT_H
#define LINUX_DFX_STAT_H

#include <linux/kobject.h>

#define THREAD_TIME_RANGE_COUNT 3
#define VIP_PRIO_WIDTH 10

extern unsigned int dfx_stats_on;
extern unsigned int dfx_irq_preempt_stats_on;
static unsigned int time_distribute_range[THREAD_TIME_RANGE_COUNT] = {4000000, 10000000, 100000000};

typedef enum DFX_PRIO_TYPE {
	DFX_RT = 0,
	DFX_VIP,
	DFX_CFS,
	DFX_PRIO_TYPE_MAX,
} prio_type_t;

struct stats_info {
	unsigned int count[THREAD_TIME_RANGE_COUNT];
	unsigned long long time[THREAD_TIME_RANGE_COUNT];
};

struct dfx_info {
	struct stats_info runnable[DFX_PRIO_TYPE_MAX];
	struct stats_info block[DFX_PRIO_TYPE_MAX];
	struct stats_info iowait[DFX_PRIO_TYPE_MAX];

	unsigned long long running_time[DFX_PRIO_TYPE_MAX];
	unsigned long long nr_switches;
};

void dfx_start(void);
void dfx_stop(void);

ssize_t dfx_thread_stats_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf);
ssize_t dfx_cpu_stats_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf);
ssize_t dfx_preempt_disable_stats_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf);
ssize_t dfx_irq_disable_stats_show(struct kobject *kobj, struct kobj_attribute *kattr, char *buf);
#endif
