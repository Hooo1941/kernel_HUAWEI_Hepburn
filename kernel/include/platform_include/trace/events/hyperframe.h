/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM hyperframe

#if !defined(_TRACE_HYPEFRAME_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HYPEFRAME_H

#include <linux/tracepoint.h>

/**
 *
 * @ctx:		pointer to a ring context structure
 * @hf_ctx:		pointer to hyperframe context with the ring context
 * @opcode:		opcode of request
 * @user_data:	user data associated with the request
 * @ret:		hyperframe request return ret
 * @ion:		whether ion buffer
 * @fd:			hyperframe file fd
 *
 * Allows to track SQE from submitting to completing.
 */

TRACE_EVENT(hyperframe_req_init,

	TP_PROTO(void *ctx, void *hf_ctx, u8 opcode, u64 ud, int fd),

	TP_ARGS(ctx, hf_ctx, opcode, ud, fd),

	TP_STRUCT__entry (
		__field(  void *,	ctx		)
		__field(  void *,	hf_ctx	)
		__field(  u8,		opcode	)
		__field(  u64,		ud		)
		__field(  int,		fd		)
	),

	TP_fast_assign(
		__entry->ctx	= ctx;
		__entry->hf_ctx	= hf_ctx;
		__entry->opcode	= opcode;
		__entry->ud		= ud;
		__entry->fd		= fd;
	),

	TP_printk("ring:%pK - %pK, op:%d, user_data:0x%llx, fd:%d",
			  __entry->ctx, __entry->hf_ctx, __entry->opcode,
			  (unsigned long long) __entry->ud, __entry->fd)
);

TRACE_EVENT(hyperframe_add_list,

	TP_PROTO(void *hf_ctx, u8 opcode, bool ion, int fd),

	TP_ARGS(hf_ctx, opcode, ion, fd),

	TP_STRUCT__entry (
		__field(  void *,	hf_ctx	)
		__field(  u8,		opcode	)
		__field(  bool,		ion		)
		__field(  int,		fd		)
	),

	TP_fast_assign(
		__entry->hf_ctx		= hf_ctx;
		__entry->opcode		= opcode;
		__entry->ion		= ion;
		__entry->fd			= fd;
	),

	TP_printk("ring:%pK, op:%d, ion:%d, fd:%d",
			  __entry->hf_ctx, __entry->opcode,
			  __entry->ion, __entry->fd)
);

TRACE_EVENT(hyperframe_work_start,

	TP_PROTO(void *hf_ctx, u8 opcode, int io_cnt, int fd),

	TP_ARGS(hf_ctx, opcode, io_cnt, fd),

	TP_STRUCT__entry (
		__field(  void *,	hf_ctx	)
		__field(  u8,		opcode	)
		__field(  int,		io_cnt	)
		__field(  int,		fd		)
	),

	TP_fast_assign(
		__entry->hf_ctx	= hf_ctx;
		__entry->opcode	= opcode;
		__entry->io_cnt	= io_cnt;
		__entry->fd		= fd;
	),

	TP_printk("ring:%pK, op:%d, io_count:%d, fd:%d",
			  __entry->hf_ctx, __entry->opcode,
			  __entry->io_cnt, __entry->fd)
);

TRACE_EVENT(hyperframe_req_dispatch,

	TP_PROTO(u8 opcode, int ret, int fd),

	TP_ARGS(opcode, ret, fd),

	TP_STRUCT__entry (
		__field(  u8,	opcode	)
		__field(  int,	ret		)
		__field(  int,	fd		)
	),

	TP_fast_assign(
		__entry->opcode	= opcode;
		__entry->ret	= ret;
		__entry->fd		= fd;
	),

	TP_printk("op:%d, ret:%d, fd:%d",
			  __entry->opcode, __entry->ret, __entry->fd)
);

TRACE_EVENT(hyperframe_irq_complete,

	TP_PROTO(void *hf_ctx, u8 opcode, int comp_cnt, int fd),

	TP_ARGS(hf_ctx, opcode, comp_cnt, fd),

	TP_STRUCT__entry (
		__field(  void *,	hf_ctx	)
		__field(  u8,		opcode	)
		__field(  int,		comp_cnt)
		__field(  int,		fd		)
	),

	TP_fast_assign(
		__entry->hf_ctx		= hf_ctx;
		__entry->opcode		= opcode;
		__entry->comp_cnt	= comp_cnt;
		__entry->fd			= fd;
	),

	TP_printk("ring:%pK, op:%d, comp_count:%d, fd:%d",
			  __entry->hf_ctx, __entry->opcode,
			  __entry->comp_cnt, __entry->fd)
);

TRACE_EVENT(hyperframe_fill_cqe,

	TP_PROTO(void *hf_ctx, u8 opcode, int disp_count,
			 int comp_count, int fd),

	TP_ARGS(hf_ctx, opcode, disp_count, comp_count, fd),

	TP_STRUCT__entry (
		__field(  void *,hf_ctx	)
		__field(  u8,	opcode	)
		__field(  int,	disp_count)
		__field(  int,	comp_count)
		__field(  int,	fd		)
	),

	TP_fast_assign(
		__entry->hf_ctx		= hf_ctx;
		__entry->opcode		= opcode;
		__entry->disp_count	= disp_count;
		__entry->comp_count	= comp_count;
		__entry->fd			= fd;
	),

	TP_printk("ring:%pK, op:%d, disp:%d, comp:%d, fd:%d",
			  __entry->hf_ctx, __entry->opcode, __entry->disp_count,
			  __entry->comp_count, __entry->fd)
);

TRACE_EVENT(hyperframe_req_completed,

	TP_PROTO(u8 opcode, u64 ud, int ret, int ref, int fd),

	TP_ARGS(opcode, ud, ret, ref, fd),

	TP_STRUCT__entry (
		__field(  u8,	opcode)
		__field(  u64,	ud	)
		__field(  int,	ret	)
		__field(  int,	ref	)
		__field(  int,	fd	)
	),

	TP_fast_assign(
		__entry->opcode	= opcode;
		__entry->ud		= ud;
		__entry->ret	= ret;
		__entry->ref	= ref;
		__entry->fd		= fd;
	),

	TP_printk("op:%d, user_data:0x%llx, ret:%d, ref:%d, fd:%d",
			  __entry->opcode, (unsigned long long) __entry->ud,
			  __entry->ret, __entry->ref, __entry->fd)
);

#endif /* _TRACE_HYPERFRAME_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
