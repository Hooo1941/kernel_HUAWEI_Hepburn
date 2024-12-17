/* SPDX-License-Identifier: GPL-2.0 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM ion_system_heap

#if !defined(_ION_SYSTEM_HEAP_H) || defined(TRACE_HEADER_MULTI_READ)
#define _ION_SYSTEM_HEAP_H

#include <linux/tracepoint.h>

TRACE_EVENT(sys_heap_fill_stat,
		TP_PROTO(unsigned long try_to_fill_num,
			 unsigned long filled_num),
		TP_ARGS(try_to_fill_num, filled_num),
		TP_STRUCT__entry(
			__field(unsigned long, try_to_fill_num)
			__field(unsigned long, filled_num)
		),
		TP_fast_assign(
			__entry->try_to_fill_num = try_to_fill_num ;
			__entry->filled_num = filled_num;
		),
		TP_printk("try to fill %lu pages, %lu pages filled",
			__entry->try_to_fill_num,
			__entry->filled_num)
);
#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE sys_heap_trace
#include <trace/define_trace.h>
