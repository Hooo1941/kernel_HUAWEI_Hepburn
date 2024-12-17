/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_SMT_H
#define _LINUX_SCHED_SMT_H

#include <linux/static_key.h>

enum smt_mode {
	ST_MODE = 0,
	ST_IDLE_MODE = 0,
	ST_HOTPULG_MODE = 0,
	SMT_MODE = 1,
	NR_SMT_MODE,
	NO_SMT_MODE,
};

#ifdef CONFIG_SCHED_SMT

#ifdef CONFIG_SCHED_SMT_EXPELLING
typedef enum SMT_EXPEL_TYPE {
	SMT_EXPELLEE = 0,
	SMT_EXPELLER_UTIL,
	SMT_EXPELLER,
	SMT_EXPELLER_FORCE,
	SMT_EXPELLER_FORCE_LONG,
	SMT_EXPELLER_MAX,
} smt_expeller_t;

void set_task_expeller(struct task_struct *task, smt_expeller_t type);
void force_smt_expeller_prepare(void);
#endif

#ifdef CONFIG_SMT_MODE_GOV
unsigned int smt_get_mode(int cpu);
#endif

extern struct static_key_false sched_smt_present;

static __always_inline bool sched_smt_active(void)
{
	return static_branch_likely(&sched_smt_present);
}

bool cpu_should_thread_off(int cpu);
#else
static inline bool sched_smt_active(void) { return false; }
static inline bool cpu_should_thread_off(int cpu) { return false; };
#endif

extern unsigned int sysctl_task_cap_discount_in_mt;

void arch_smt_update(void);

#endif
