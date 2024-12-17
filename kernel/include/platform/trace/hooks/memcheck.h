/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM memcheck

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH platform/trace/hooks

#if !defined(_TRACE_MEMCHECK_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MEMCHECK_H

#include <linux/tracepoint.h>
#include <platform_include/basicplatform/linux/hck_vendor_native_hooks.h>
#include <platform_include/basicplatform/linux/hck_vendor_restricted_hooks.h>

DECLARE_HCK_VRH(mm_mem_stats_show,
	TP_PROTO(int unused),
	TP_ARGS(unused), 1);

DECLARE_HCK_VNH(gpu_get_total_used,
	TP_PROTO(unsigned long *gpu_total),
	TP_ARGS(gpu_total));

DECLARE_HCK_VRH(mm_memcheck_gpumem_info_show,
	TP_PROTO(int unused),
	TP_ARGS(unused), 1);

DECLARE_HCK_VRH(mm_memcheck_gpumem_get_usage,
	TP_PROTO(pid_t pid, u64 *mapped, u64 *total),
	TP_ARGS(pid, mapped, total), 1);

DECLARE_HCK_VRH(cma_report,
	TP_PROTO(char *name, unsigned long total, unsigned long req),
	TP_ARGS(name, total, req), 1);

DECLARE_HCK_VRH(slub_obj_report,
	TP_PROTO(struct kmem_cache *s),
	TP_ARGS(s), 1);

DECLARE_HCK_VRH(lowmem_report,
	TP_PROTO(struct task_struct *p, unsigned long points),
	TP_ARGS(p, points), 1);

#endif /* _TRACE_MEMCHECK_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
