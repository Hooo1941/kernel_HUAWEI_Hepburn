/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM hungtask

#if !defined(_TRACE_HUNGTASK_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HUNGTASK_H

#include <linux/tracepoint.h>

TRACE_EVENT(set_did_panic,
	TP_PROTO(int did_panic),
	TP_ARGS(did_panic),

	TP_STRUCT__entry(
		__field(int, did_panic)
	),

	TP_fast_assign(
		__entry->did_panic = did_panic;
	),

	TP_printk("did_panic=%d", __entry->did_panic)
);

TRACE_EVENT(set_timeout_secs,
	TP_PROTO(int timeout_sec),
	TP_ARGS(timeout_sec),

	TP_STRUCT__entry(
		__field(int, timeout_sec)
	),

	TP_fast_assign(
		__entry->timeout_sec = timeout_sec;
	),

	TP_printk("fectch timeout_sec=%d", __entry->timeout_sec)
);

TRACE_EVENT(check_tasks,
	TP_PROTO(unsigned long ts),
	TP_ARGS(ts),

	TP_STRUCT__entry(
		__field(unsigned long, ts)
	),

	TP_fast_assign(
		__entry->ts	= ts;
	),

	TP_printk("check timeout=%lu", __entry->ts)
);

TRACE_EVENT(mmap_sem_debug,
	TP_PROTO(const struct rw_semaphore *sem),
	TP_ARGS(sem),

	TP_STRUCT__entry(
		__field(const struct rw_semaphore *, sem)
	),

	TP_fast_assign(
		__entry->sem = sem;
	),

	TP_printk("mmap_sem_debug")
);

#endif /* _TRACE_HUNGTASK_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
