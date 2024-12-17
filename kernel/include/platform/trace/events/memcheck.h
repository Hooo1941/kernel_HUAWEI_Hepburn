/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM memcheck

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH platform/trace/events

#if !defined(_TRACE_MEMCHECK_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MEMCHECK_H

#include <linux/tracepoint.h>

struct kmem_cache;

TRACE_EVENT(mm_mem_stats_show,

	TP_PROTO(int unused),

	TP_ARGS(unused),

	TP_STRUCT__entry(
		__field(int, unused)
	),

	TP_fast_assign(
		__entry->unused = unused;
	),

	TP_printk("mm_mem_stats_show %d", __entry->unused)
);

TRACE_EVENT(mm_memcheck_gpumem_info_show,

	TP_PROTO(int unused),

	TP_ARGS(unused),

	TP_STRUCT__entry(
		__field(int, unused)
	),

	TP_fast_assign(
		__entry->unused = unused;
	),

	TP_printk("mm_memcheck_gpumem_info_show %d", __entry->unused)
);

TRACE_EVENT(android_gpu_get_total_used,

	TP_PROTO(unsigned long *total),

	TP_ARGS(total),

	TP_STRUCT__entry(
		__field(unsigned long *, total)
	),

	TP_fast_assign(
		__entry->total = total;
	),

	TP_printk("gpu_get_total_used %d", __entry->total)
);

TRACE_EVENT(cma_report,

	TP_PROTO(char *name, unsigned long total, unsigned long req),

	TP_ARGS(name, total, req),

	TP_STRUCT__entry(
		__field(char *, name)
		__field(unsigned long, total)
		__field(unsigned long, req)
	),

	TP_fast_assign(
		__entry->name = name;
		__entry->total = total;
		__entry->req = req;
	),

	TP_printk("cma_report name=%s,total=%ld,req=%ld", __entry->name, __entry->total, __entry->req)
);

TRACE_EVENT(slub_obj_report,

	TP_PROTO(struct kmem_cache *s),

	TP_ARGS(s),

	TP_STRUCT__entry(
		__field(struct kmem_cache *, s)
		__field(int, unused)
	),

	TP_fast_assign(
		__entry->s = s;
		__entry->unused = 0;
	),

	TP_printk("slub_obj_report %d", __entry->unused)
);

TRACE_EVENT(lowmem_report,

	TP_PROTO(struct task_struct *p, unsigned long points),

	TP_ARGS(p, points),

	TP_STRUCT__entry(
		__field(struct task_struct *, p)
		__field(unsigned long, points)
	),

	TP_fast_assign(
		__entry->p = p;
		__entry->points = points;
	),

	TP_printk("lowmem_report, points=%ld", __entry->points)
);

#endif /* _TRACE_MEMCHECK_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
