/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_PREEMPT_H
#define __ASM_PREEMPT_H

#include <linux/thread_info.h>

#define PREEMPT_NEED_RESCHED	BIT(32)
#define PREEMPT_ENABLED	(PREEMPT_NEED_RESCHED)

#ifdef CONFIG_DFX_STATS
extern unsigned int dfx_stats_on;
extern void dfx_preempt_disable_stats(u32 pc, int val);
extern void dfx_preempt_enable_stats(u32 pc, int val);

static inline void preempt_disable_stats(u32 pc, int val)
{
	if (dfx_stats_on)
		dfx_preempt_disable_stats(pc, val);
}

static inline void preempt_enable_stats(u32 pc, int val)
{
	if (dfx_stats_on)
		dfx_preempt_enable_stats(pc, val);
}
#else /* !CONFIG_DFX_STATS */
# define preempt_disable_stats(pc, val)			do { } while (0)
# define preempt_enable_stats(pc, val)			do { } while (0)
#endif /* CONFIG_DFX_STATS */

static inline int preempt_count(void)
{
	return READ_ONCE(current_thread_info()->preempt.count);
}

static inline void preempt_count_set(u64 pc)
{
	/* Preserve existing value of PREEMPT_NEED_RESCHED */
	WRITE_ONCE(current_thread_info()->preempt.count, pc);
}

#define init_task_preempt_count(p) do { \
	task_thread_info(p)->preempt_count = FORK_PREEMPT_COUNT; \
} while (0)

#define init_idle_preempt_count(p, cpu) do { \
	task_thread_info(p)->preempt_count = PREEMPT_ENABLED; \
} while (0)

static inline void set_preempt_need_resched(void)
{
	current_thread_info()->preempt.need_resched = 0;
}

static inline void clear_preempt_need_resched(void)
{
	current_thread_info()->preempt.need_resched = 1;
}

static inline bool test_preempt_need_resched(void)
{
	return !current_thread_info()->preempt.need_resched;
}

static inline void __preempt_count_add(int val)
{
	u32 pc = READ_ONCE(current_thread_info()->preempt.count);
	pc += val;
	WRITE_ONCE(current_thread_info()->preempt.count, pc);
	preempt_disable_stats(pc, val);
}

static inline void __preempt_count_sub(int val)
{
	u32 pc = READ_ONCE(current_thread_info()->preempt.count);
	preempt_enable_stats(pc, val);
	pc -= val;
	WRITE_ONCE(current_thread_info()->preempt.count, pc);
}

static inline bool __preempt_count_dec_and_test(void)
{
	struct thread_info *ti = current_thread_info();
	u64 pc = READ_ONCE(ti->preempt_count);
	preempt_enable_stats(pc, 1);

	/* Update only the count field, leaving need_resched unchanged */
	WRITE_ONCE(ti->preempt.count, --pc);

	/*
	 * If we wrote back all zeroes, then we're preemptible and in
	 * need of a reschedule. Otherwise, we need to reload the
	 * preempt_count in case the need_resched flag was cleared by an
	 * interrupt occurring between the non-atomic READ_ONCE/WRITE_ONCE
	 * pair.
	 */
	return !pc || !READ_ONCE(ti->preempt_count);
}

static inline bool should_resched(int preempt_offset)
{
	u64 pc = READ_ONCE(current_thread_info()->preempt_count);
	return pc == preempt_offset;
}

#ifdef CONFIG_PREEMPTION
void preempt_schedule(void);
#define __preempt_schedule() preempt_schedule()
void preempt_schedule_notrace(void);
#define __preempt_schedule_notrace() preempt_schedule_notrace()
#endif /* CONFIG_PREEMPTION */

#endif /* __ASM_PREEMPT_H */
