/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM rainbow

#if !defined(_TRACE_RAINBOW_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_RAINBOW_H

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH platform/trace/hooks

#include <linux/tracepoint.h>
#include <platform_include/basicplatform/linux/hck_vendor_native_hooks.h>

DECLARE_HCK_VNH(rb_trace_task_write,
	TP_PROTO(void *next_task),
	TP_ARGS(next_task));

DECLARE_HCK_VNH(rb_trace_irq_write,
	TP_PROTO(unsigned int dir, unsigned int new_vec),
	TP_ARGS(dir, new_vec));

DECLARE_HCK_VNH(rb_sreason_set,
	TP_PROTO(char *fmt),
	TP_ARGS(fmt));

DECLARE_HCK_VNH(rb_attach_info_set,
	TP_PROTO(char *fmt),
	TP_ARGS(fmt));

DECLARE_HCK_VNH(rb_mreason_set,
	TP_PROTO(uint32_t reason),
	TP_ARGS(reason));

DECLARE_HCK_VNH(rb_kallsyms_set,
	TP_PROTO(const char *fmt),
	TP_ARGS(fmt));

DECLARE_HCK_VNH(cmd_himntn_item_switch,
	TP_PROTO(unsigned int index, bool *rtn),
	TP_ARGS(index, rtn));

#endif /* _TRACE_RAINBOW_H */

/* This part must be outside protection */
#include <trace/define_trace.h>