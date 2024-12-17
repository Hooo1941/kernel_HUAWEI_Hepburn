/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: add trace points for memory reclaim thread kswapd
 * Author: Mi Sasa <misasa@huawei.com>
 * Create: 2022-06-20
 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM vmscan

#if !defined(_TRACE_RECLAIM_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_RECLAIM_H

#include <linux/tracepoint.h>

DECLARE_EVENT_CLASS(mm_reclaim_general_template,

	TP_PROTO(unsigned long freed),

	TP_ARGS(freed),

	TP_STRUCT__entry(
		__field(unsigned long, freed)
	),

	TP_fast_assign(
		__entry->freed = freed;
	),

	TP_printk("freed=%lu", __entry->freed)
);

DEFINE_EVENT(mm_reclaim_general_template, kswapd_start,

	TP_PROTO(unsigned long freed),

	TP_ARGS(freed)
);

DEFINE_EVENT(mm_reclaim_general_template, kswapd_end,

	TP_PROTO(unsigned long freed),

	TP_ARGS(freed)
);

DEFINE_EVENT(mm_reclaim_general_template, shrinklists_start,

	TP_PROTO(unsigned long freed),

	TP_ARGS(freed)
);

DEFINE_EVENT(mm_reclaim_general_template, shrinklists_end,

	TP_PROTO(unsigned long freed),

	TP_ARGS(freed)
);

DEFINE_EVENT(mm_reclaim_general_template, shrinkslabs_start,

	TP_PROTO(unsigned long freed),

	TP_ARGS(freed)
);

DEFINE_EVENT(mm_reclaim_general_template, shrinkslabs_pages,

	TP_PROTO(unsigned long freed),

	TP_ARGS(freed)
);

DEFINE_EVENT(mm_reclaim_general_template, shrinkslabs_end,

	TP_PROTO(unsigned long freed),

	TP_ARGS(freed)
);

TRACE_EVENT(mm_kswapd_current_task_util,
		TP_PROTO(unsigned long current_task_util, bool fit_cpu, unsigned int cpu_id),

		TP_ARGS(current_task_util, fit_cpu, cpu_id),

		TP_STRUCT__entry(
			__field(unsigned long, current_task_util)
			__field(bool, fit_cpu)
			__field(unsigned int, cpu_id)
		),

		TP_fast_assign(
			__entry->current_task_util = current_task_util;
			__entry->fit_cpu = fit_cpu;
			__entry->cpu_id = cpu_id;
		),

		TP_printk("kswapd_current_task_util=%lu, fit_cpu=%d, cpu_id=%u",
			__entry->current_task_util, __entry->fit_cpu, __entry->cpu_id)
);

#ifdef CONFIG_PROTECT_LRU_ENHANCED
TRACE_EVENT(mm_filemap_refault_distance,
		TP_PROTO(unsigned long distance),

		TP_ARGS(distance),

		TP_STRUCT__entry(
			__field(unsigned long, distance)
		),

		TP_fast_assign(
			__entry->distance = distance;
		),

		TP_printk("distance=%lu", __entry->distance)
);
#endif

#endif /* _TRACE_RECLAIM_H */
