/* SPDX-License-Identifier: GPL-2.0 */
/*
 * drivers/staging/android/ion/ion-trace.h
 *
 * Copyright (C) 2020 Google, Inc.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM ion

#if !defined(_ION_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _ION_TRACE_H

#include <linux/tracepoint.h>

#ifndef __ION_PTR_TO_HASHVAL
static unsigned int __maybe_unused __ion_ptr_to_hash(const void *ptr)
{
	unsigned long hashval;

	if (ptr_to_hashval(ptr, &hashval))
		return 0;

	/* The hashed value is only 32-bit */
	return (unsigned int)hashval;
}

#define __ION_PTR_TO_HASHVAL
#endif

TRACE_EVENT(ion_stat,
	    TP_PROTO(const void *addr, long len,
		     unsigned long total_allocated),
	    TP_ARGS(addr, len, total_allocated),
	    TP_STRUCT__entry(__field(unsigned int, buffer_id)
		__field(long, len)
		__field(unsigned long, total_allocated)
	    ),
	    TP_fast_assign(__entry->buffer_id = __ion_ptr_to_hash(addr);
		__entry->len = len;
		__entry->total_allocated = total_allocated;
	    ),
	    TP_printk("buffer_id=%u len=%ldB total_allocated=%ldB",
		      __entry->buffer_id,
		      __entry->len,
		      __entry->total_allocated)
	    );

TRACE_EVENT(ion_alloc,
	    TP_PROTO(unsigned int heap_id_mask, size_t len, long delay_us),
	    TP_ARGS(heap_id_mask, len, delay_us),
	    TP_STRUCT__entry(
		__field(unsigned int, heap_id_mask)
		__field(size_t, len)
		__field(long, delay_us)
	    ),
	    TP_fast_assign(
		__entry->heap_id_mask = heap_id_mask;
		__entry->len = len;
		__entry->delay_us = delay_us;
	    ),
	    TP_printk("heap_id_mask=0x%x len=%lu delay_us=%ld",
		      __entry->heap_id_mask,
		      __entry->len,
		      __entry->delay_us)
	    );

#endif /* _ION_TRACE_H */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE ion_trace
#include <trace/define_trace.h>
