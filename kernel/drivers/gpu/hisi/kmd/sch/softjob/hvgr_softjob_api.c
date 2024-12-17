/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_sch_api.h"

#include "hvgr_ioctl_sch.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_cq.h"
#include "hvgr_cq_queue.h"
#include "hvgr_msync.h"
#include "hvgr_ksync.h"
#include "hvgr_softjob.h"

static long hvgr_ioctl_bind_softq(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_bind_softq * const para)
{
	uint32_t softq_id;
	struct hvgr_soft_q *qctx = NULL;

	softq_id = para->in.softq_id;
	/* Check soft queue direction and input entry size */
	if (softq_id >= HVGR_MAX_NR_SOFTQ)
		return -EINVAL;

	mutex_lock(&ctx->ctx_lock);
	mutex_lock(&ctx->softq_lock);
	qctx = ctx->id_to_qctx[softq_id];
	if (qctx == NULL) {
		mutex_unlock(&ctx->softq_lock);
		mutex_unlock(&ctx->ctx_lock);
		return -EINVAL;
	}

	if (ctx->sq_ctx != NULL || qctx->ctrl->flags != HVGR_SOFTQ_DIR_K_TO_U) {
		hvgr_err(ctx->gdev, HVGR_CQ, "ctx_%u sq not null or sq_%u flag=0x%llx not prop\n",
			ctx->id, softq_id, qctx->ctrl->flags);
		mutex_unlock(&ctx->softq_lock);
		mutex_unlock(&ctx->ctx_lock);
		return -EINVAL;
	}

	ctx->sq_ctx = ctx->id_to_qctx[softq_id];
	mutex_unlock(&ctx->softq_lock);
	mutex_unlock(&ctx->ctx_lock);
	return 0;
}

static long hvgr_ioctl_term_softq(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_term_softq * const para)
{
	uint32_t stage;

	mutex_lock(&ctx->ctx_lock);
	stage = para->in.stage;
	if (stage == HVGR_TERM_STAGE_MESSAGE) {
		atomic_set(&ctx->softq_closed, true);
		hvgr_msync_event_wakeup(ctx);
#ifdef CONFIG_LIBLINUX_HVGR_POLL_OPT
		liblinux_pal_sync_wakeup(ctx->kfile->liblinux_private, POLLIN | POLLRDNORM);
#endif
	} else if (stage == HVGR_TERM_STAGE_CLOSED) {
		if (ctx->sq_ctx != NULL) {
			hvgr_cq_unsubmit_all_queue(ctx);
			(void)hvgr_softq_term(ctx->sq_ctx);
			ctx->sq_ctx = NULL;
		}
	} else {
		mutex_unlock(&ctx->ctx_lock);
		return -1;
	}
	mutex_unlock(&ctx->ctx_lock);
	return 0;
}

long hvgr_ioctl_softjob(struct hvgr_ctx * const ctx,
	unsigned int cmd, unsigned long arg)
{
	long ret = -1;

	switch (cmd) {
	case HVGR_IOCTL_BIND_SOFTQ:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_BIND_SOFTQ,
			hvgr_ioctl_bind_softq, arg, union hvgr_ioctl_para_bind_softq);
		break;
	case HVGR_IOCTL_TERM_SOFTQ:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_TERM_SOFTQ,
			hvgr_ioctl_term_softq, arg, union hvgr_ioctl_para_term_softq);
		break;
	case HVGR_IOCTL_MSYNC_GET_FD:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_MSYNC_GET_FD,
			hvgr_ioctl_cq_get_fd, arg, union hvgr_ioctl_para_msync_fd);
		break;
	case HVGR_IOCTL_MSYNC_SET_FD:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_MSYNC_SET_FD,
			hvgr_ioctl_cq_set_fd, arg, union hvgr_ioctl_para_msync_fd);
		break;
	case HVGR_IOCTL_TRIGGER_FENCE_FD:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_TRIGGER_FENCE_FD,
			hvgr_ioctl_trigger_fence, arg, union hvgr_ioctl_para_trigger_fence_fd);
		break;
	case HVGR_IOCTL_WAIT_FENCE_FD:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_WAIT_FENCE_FD,
			hvgr_ioctl_wait_fence, arg, union hvgr_ioctl_para_wait_fence_fd);
		break;
	case HVGR_IOCTL_FENCE_VALIDATE:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_FENCE_VALIDATE,
			hvgr_ksync_fence_validate, arg, struct hvgr_ioctl_fence_validate);
		break;
	case HVGR_IOCTL_CQ_SC_UPDATE_POLICY:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CQ_SC_UPDATE_POLICY,
			hvgr_ioctl_sc_update_policy, arg, union hvgr_ioctl_sc_update_policy);
		break;
	case HVGR_IOCTL_SET_HOLD_EN_INFO:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_SET_HOLD_EN_INFO,
			hvgr_ioctl_cq_set_hold_en_info, arg, union hvgr_ioctl_para_hold_en_info);
		break;
	case HVGR_IOCTL_CQ_SET_FREQ_HINT:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CQ_SET_FREQ_HINT,
			hvgr_ioctl_cq_set_freq_hint, arg, union hvgr_ioctl_para_set_freq_hint);
		break;
	case HVGR_IOCTL_CQ_SET_CL_PROFILING:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CQ_SET_CL_PROFILING,
			hvgr_ioctl_cq_set_cl_profiling, arg, union hvgr_ioctl_para_set_cl_profiling);
		break;
	case HVGR_IOCTL_CQ_CLEAR_CL_FLAGS:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CQ_CLEAR_CL_FLAGS,
			hvgr_ioctl_cq_clear_cl_flags, arg, union hvgr_ioctl_para_clear_cl_flags);
	case HVGR_IOCTL_SET_TOP_NOTIFY:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_SET_TOP_NOTIFY,
			hvgr_ioctl_cq_set_top_notify, arg, union hvgr_ioctl_para_top_notify);
		break;
	default:
		dev_emerg(ctx->gdev->dev, "Softjob ioctl cmd %x is INVALID.", cmd);
		break;
	}

exit:
	return ret;
}

