/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM rainbow

#if !defined(_TRACE_RAINBOW_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_RAINBOW_H

#include <linux/tracepoint.h>

TRACE_EVENT(rb_trace_task_write,

	TP_PROTO(void *next_task),

	TP_ARGS(next_task),

	TP_STRUCT__entry(
		__field(void *, next_task)
	),

	TP_fast_assign(
		__entry->next_task = next_task;
	),

	TP_printk("rb_trace_task_write next_task = %p", __entry->next_task)
);

TRACE_EVENT(rb_trace_irq_write,

	TP_PROTO(unsigned int dir, unsigned int new_vec),

	TP_ARGS(dir, new_vec),

	TP_STRUCT__entry(
		__field(unsigned int, dir)
		__field(unsigned int, new_vec)
	),

	TP_fast_assign(
		__entry->dir = dir;
		__entry->new_vec = new_vec;
	),

	TP_printk("rb_trace_irq_write dir = %u, hwirq = %u", __entry->dir, __entry->new_vec)
);

TRACE_EVENT(rb_sreason_set,

	TP_PROTO(char *fmt),

	TP_ARGS(fmt),

	TP_STRUCT__entry(
		__field(char *, fmt)
	),

	TP_fast_assign(
		__entry->fmt = fmt;
	),

	TP_printk("rb_sreason_set fmt = %s", __entry->fmt)
);

TRACE_EVENT(rb_attach_info_set,

	TP_PROTO(char *fmt),

	TP_ARGS(fmt),

	TP_STRUCT__entry(
		__field(char *, fmt)
	),

	TP_fast_assign(
		__entry->fmt = fmt;
	),

	TP_printk("rb_attach_info_set fmt = %s", __entry->fmt)
);

TRACE_EVENT(rb_mreason_set,

	TP_PROTO(uint32_t reason),

	TP_ARGS(reason),

	TP_STRUCT__entry(
		__field(uint32_t, reason)
	),

	TP_fast_assign(
		__entry->reason = reason;
	),

	TP_printk("rb_mreason_set reason = %d", __entry->reason)
);

TRACE_EVENT(rb_kallsyms_set,

	TP_PROTO(const char *fmt),

	TP_ARGS(fmt),

	TP_STRUCT__entry(
		__field(const char *, fmt)
	),

	TP_fast_assign(
		__entry->fmt = fmt;
	),

	TP_printk("rb_kallsyms_set fmt = %s", __entry->fmt)
);

TRACE_EVENT(cmd_himntn_item_switch,

	TP_PROTO(unsigned int index, bool *rtn),

	TP_ARGS(index, rtn),

	TP_STRUCT__entry(
		__field(unsigned int, index)
		__field(bool *, rtn)
	),

	TP_fast_assign(
		__entry->index = index;
		__entry->rtn = rtn;
	),

	TP_printk("cmd_himntn_item_switch index = %u rtn = %d", __entry->index, *__entry->rtn)
);

#endif /* _TRACE_RAINBOW_H */

/* This part must be outside protection */
#include <trace/define_trace.h>