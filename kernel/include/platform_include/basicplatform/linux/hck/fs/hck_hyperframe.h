#ifndef __HCK_HYPERFRAME__
#define __HCK_HYPERFRAME__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>

/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct bio;
struct kiocb;
struct io_ring_ctx;
struct io_kiocb;
struct io_uring_sqe;

DECLARE_HCK_VH(hyperframe_err_record,
	TP_PROTO(const char *func, int ret, int resv1, int resv2, int resv3),
	TP_ARGS(func, ret, resv1, resv2, resv3));

DECLARE_HCK_VH(hyperframe_set_opf,
	TP_PROTO(struct bio *bio, struct kiocb *iocb),
	TP_ARGS(bio, iocb));

DECLARE_HCK_VH(hyperframe_ctx_alloc,
	TP_PROTO(struct io_ring_ctx *ctx, int *ret),
	TP_ARGS(ctx, ret));

DECLARE_HCK_VH(hyperframe_ctx_free,
	TP_PROTO(struct io_ring_ctx *ctx),
	TP_ARGS(ctx));

DECLARE_HCK_VH(hyperframe_openat_prep,
	TP_PROTO(struct io_kiocb *req, const struct io_uring_sqe *sqe),
	TP_ARGS(req, sqe));

DECLARE_HCK_VH(hyperframe_close_prep,
	TP_PROTO(struct io_kiocb *req, const struct io_uring_sqe *sqe),
	TP_ARGS(req, sqe));

DECLARE_HCK_VH(hyperframe_rw_prep,
	TP_PROTO(struct io_kiocb *req, const struct io_uring_sqe *sqe, long *ret),
	TP_ARGS(req, sqe, ret));

DECLARE_HCK_VH(hyperframe_init_req,
	TP_PROTO(struct io_kiocb *req, const struct io_uring_sqe *sqe, int *ret),
	TP_ARGS(req, sqe, ret));

DECLARE_HCK_VH(hyperframe_free_req,
	TP_PROTO(struct io_kiocb *req),
	TP_ARGS(req));

DECLARE_HCK_VH(hyperframe_issue_sqe,
	TP_PROTO(struct io_kiocb *req, int *ret),
	TP_ARGS(req, ret));

DECLARE_HCK_VH(hyperframe_complete_sqe,
	TP_PROTO(u8 opcode, int *ret),
	TP_ARGS(opcode, ret));

#endif /* __HCK_HYPERFRAME__ */
