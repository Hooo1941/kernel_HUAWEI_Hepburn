/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Based on arch/arm/include/asm/thread_info.h
 *
 * Copyright (C) 2002 Russell King.
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_THREAD_INFO_H
#define __ASM_THREAD_INFO_H

#include <linux/compiler.h>

#ifndef __ASSEMBLY__

struct task_struct;

#include <asm/memory.h>
#include <asm/stack_pointer.h>
#include <asm/types.h>

typedef unsigned long mm_segment_t;

/*
 * low level task data that entry.S needs immediate access to.
 */
struct thread_info {
	unsigned long		flags;		/* low level flags */
	mm_segment_t		addr_limit;	/* address limit */
#ifdef CONFIG_ARM64_SW_TTBR0_PAN
	u64			ttbr0;		/* saved TTBR0_EL1 */
#endif
	union {
		u64		preempt_count;	/* 0 => preemptible, <0 => bug */
		struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
			u32	need_resched;
			u32	count;
#else
			u32	count;
			u32	need_resched;
#endif
		} preempt;
	};
#ifdef CONFIG_SHADOW_CALL_STACK
	void			*scs_base;
	void			*scs_sp;
#endif
};

#define thread_saved_pc(tsk)	\
	((unsigned long)(tsk->thread.cpu_context.pc))
#define thread_saved_sp(tsk)	\
	((unsigned long)(tsk->thread.cpu_context.sp))
#define thread_saved_fp(tsk)	\
	((unsigned long)(tsk->thread.cpu_context.fp))

void arch_setup_new_exec(void);
#define arch_setup_new_exec     arch_setup_new_exec

void arch_release_task_struct(struct task_struct *tsk);

#endif

#define TIF_SIGPENDING		0	/* signal pending */
#define TIF_NEED_RESCHED	1	/* rescheduling necessary */
#define TIF_NOTIFY_RESUME	2	/* callback before returning to user */
#define TIF_FOREIGN_FPSTATE	3	/* CPU's FP state is not current's */
#define TIF_UPROBE		4	/* uprobe breakpoint or singlestep */
#define TIF_FSCHECK		5	/* Check FS is USER_DS on return */
#define TIF_MTE_ASYNC_FAULT	6	/* MTE Asynchronous Tag Check Fault */
#define TIF_SYSCALL_TRACE	8	/* syscall trace active */
#define TIF_SYSCALL_AUDIT	9	/* syscall auditing */
#define TIF_SYSCALL_TRACEPOINT	10	/* syscall tracepoint for ftrace */
#define TIF_SECCOMP		11	/* syscall secure computing */
#define TIF_SYSCALL_EMU		12	/* syscall emulation active */
#define TIF_MEMDIE		18	/* is terminating due to OOM killer */
#define TIF_FREEZE		19
#define TIF_RESTORE_SIGMASK	20
#define TIF_SINGLESTEP		21
#define TIF_32BIT		22	/* 32bit process */
#define TIF_SVE			23	/* Scalable Vector Extension in use */
#define TIF_SVE_VL_INHERIT	24	/* Inherit sve_vl_onexec across exec */
#define TIF_SSBD		25	/* Wants SSB mitigation */
#define TIF_TAGGED_ADDR		26	/* Allow tagged user addresses */
#ifdef CONFIG_HISI_EAS_SCHED
#define TIF_WAKE_SYNC		27	/* Wakes someone up with WF_SYNC */
#define TIF_FAVOR_SMALL_CAP	28	/* Prefer to be scheduled on a smaller cpu */
#endif
#ifdef CONFIG_RT_ENERGY_EFFICIENT_SUPPORT
#define TIF_ENERGY_EFFICIENT	29	/* RT task that want energy efficient more than latency */
#define TIF_EXPECTED_HEAVY	30	/* RT task that is expected to be a heavy one */
#endif
#ifdef CONFIG_RT_PRIO_EXTEND_VIP
#define TIF_NO_EXTEND		31	/* To let spare_vip_width() using actual priority */
#endif
#ifdef CONFIG_SCHED_DYNAMIC_PRIO
#define TIF_DYN_PRIO		32	/* To distinguish dyn prio callers in __sched_setscheduler() */
#endif
#ifdef CONFIG_OPT_TIMER_ENERGY_EFFICIENCY
#define TIF_TIMER_DEFERRABLE	33	/* Want deferrable when setting up timers. */
#endif
#ifdef CONFIG_SCHED_COMMON_QOS_CTRL
#define TIF_IGN_INITIAL_LOAD		34
#define TIF_LATENCY_SENSITIVE		35
#endif
#ifdef CONFIG_RT_LONG_PREEMPT_OPTIMIZATION
#define TIF_LONG_NONPREEMPTIBLE		36
#endif
#ifdef CONFIG_HW_USERSPACE_THROTTLE
#define TIF_NEED_THROTTLED	37
#define TIF_USERSPACE_UNTHROTTLED  38
#endif
#define TIF_NO_USER_SETPRIO	39
#define TIF_NO_SETAFFINITY	40

#define _TIF_SIGPENDING		(1 << TIF_SIGPENDING)
#define _TIF_NEED_RESCHED	(1 << TIF_NEED_RESCHED)
#define _TIF_NOTIFY_RESUME	(1 << TIF_NOTIFY_RESUME)
#define _TIF_FOREIGN_FPSTATE	(1 << TIF_FOREIGN_FPSTATE)
#define _TIF_SYSCALL_TRACE	(1 << TIF_SYSCALL_TRACE)
#define _TIF_SYSCALL_AUDIT	(1 << TIF_SYSCALL_AUDIT)
#define _TIF_SYSCALL_TRACEPOINT	(1 << TIF_SYSCALL_TRACEPOINT)
#define _TIF_SECCOMP		(1 << TIF_SECCOMP)
#define _TIF_SYSCALL_EMU	(1 << TIF_SYSCALL_EMU)
#define _TIF_UPROBE		(1 << TIF_UPROBE)
#define _TIF_FSCHECK		(1 << TIF_FSCHECK)
#define _TIF_SINGLESTEP		(1 << TIF_SINGLESTEP)
#define _TIF_32BIT		(1 << TIF_32BIT)
#define _TIF_SVE		(1 << TIF_SVE)
#define _TIF_MTE_ASYNC_FAULT	(1 << TIF_MTE_ASYNC_FAULT)
#ifdef CONFIG_HW_USERSPACE_THROTTLE
#define _TIF_NEED_THROTTLED	(1 << TIF_NEED_THROTTLED)
#define _TIF_USERSPACE_UNTHROTTLED	(1 << TIF_USERSPACE_UNTHROTTLED)
#endif

#define _TIF_WORK_MASK		(_TIF_NEED_RESCHED | _TIF_SIGPENDING | \
				 _TIF_NOTIFY_RESUME | _TIF_FOREIGN_FPSTATE | \
				 _TIF_UPROBE | _TIF_FSCHECK | _TIF_MTE_ASYNC_FAULT)

#define _TIF_SYSCALL_WORK	(_TIF_SYSCALL_TRACE | _TIF_SYSCALL_AUDIT | \
				 _TIF_SYSCALL_TRACEPOINT | _TIF_SECCOMP | \
				 _TIF_SYSCALL_EMU)

#define _TIF_RESET_ON_FORK_MASK		(BIT(TIF_NO_USER_SETPRIO) | \
				 BIT(TIF_NO_SETAFFINITY))

#ifdef CONFIG_SHADOW_CALL_STACK
#define INIT_SCS							\
	.scs_base	= init_shadow_call_stack,			\
	.scs_sp		= init_shadow_call_stack,
#else
#define INIT_SCS
#endif

#define INIT_THREAD_INFO(tsk)						\
{									\
	.flags		= _TIF_FOREIGN_FPSTATE,				\
	.preempt_count	= INIT_PREEMPT_COUNT,				\
	.addr_limit	= KERNEL_DS,					\
	INIT_SCS							\
}

#endif /* __ASM_THREAD_INFO_H */