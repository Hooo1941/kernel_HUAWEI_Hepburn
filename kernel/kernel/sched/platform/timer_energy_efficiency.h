/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Optimize cpu fcm idle chance by doing some hacks on timer.
 */
#ifndef __TIMER_ENERGY_EFFICIENCY_H
#define __TIMER_ENERGY_EFFICIENCY_H

#ifdef CONFIG_OPT_TIMER_ENERGY_EFFICIENCY
#include "uclamp_group_def.h"
void init_timer_slack_sched_group(uclamp_group_t *tg);
u64 timer_slack_pct_read_u64(struct cgroup_subsys_state *css,
				struct cftype *cft);
int timer_slack_pct_write_u64(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 value);
u64 timer_slack_max_delay_ms_read_u64(struct cgroup_subsys_state *css,
				struct cftype *cft);
int timer_slack_max_delay_ms_write_u64(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 value);
u64 timer_slack_exempt_ms_read_u64(struct cgroup_subsys_state *css,
				struct cftype *cft);
int timer_slack_exempt_ms_write_u64(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 value);
#endif

#ifdef CONFIG_HRTIMER_SOFT_EXPIRE_SYNC
void init_hrtimer_interrupt_irqwork(int cpu);
#endif

#endif
