/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_cq_driver_adapt.h"

#include <linux/kernel.h>
#include <linux/delay.h>

#include "hvgr_cq_driver_base.h"
#include "hvgr_regmap.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_datan_api.h"

#include "hvgr_cq_debug_switch.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_msync.h"
#include "hvgr_softjob.h"
#include "hvgr_pm_api.h"
#include "hvgr_cq.h"
#include "hvgr_kmd_defs.h"

static struct hvgr_cq_prioriy_cfg cq_pri_tbl[] = {
	{CQ_PRIORITY_LOW, 1, 0},
	{CQ_PRIORITY_HIGH, 3, 0},
};
#ifdef HVGR_DATAN_JOB_TIMELINE
static uint8_t hvgr_cq_get_chain_type(union cq_bind_svd_status token)
{
	uint8_t chain_type = SUB_CHANNEL_TYPE_WHOLE;

	switch(token.st.bind_svd_jsid) {
	case RENDER_LOW_IDX:
	case RENDER_HIGH_IDX:
		chain_type = SUB_CHANNEL_TYPE_RENDER;
		break;
	case BIN_LOW_IDX:
	case BIN_HIGH_IDX:
		chain_type = SUB_CHANNEL_TYPE_BIN;
		break;
	case COMPUTE_LOW_IDX:
	case COMPUTE_HIGH_IDX:
		chain_type = SUB_CHANNEL_TYPE_COMPUTE;
		break;
	case BVHG_IDX:
		chain_type = SUB_CHANNEL_TYPE_RT_TREE_BUILD;
		break;
	case RT_IDX:
		chain_type = SUB_CHANNEL_TYPE_RT_TRAVERSE;
		break;
	default :
		break;
	}
	return chain_type;
}
#endif
void hvgr_get_priority_cfg(uint32_t pri, uint32_t *weight, uint32_t *real)
{
	if (unlikely(pri >= ARRAY_SIZE(cq_pri_tbl)))
		return;
	*weight = cq_pri_tbl[pri].weight;
	*real = cq_pri_tbl[pri].real;
}

static void hvgr_cq_config_bind_cfg(struct hvgr_device * const gdev, uint32_t channel_id)
{
#ifdef HVGR_BIND_CFG_OPEN
	uint32_t value;

	value = hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_config(channel_id, BIN_LOW_IDX)))
	value = bind_end_flush_set_bin_en_and_l2_no_flush(value)
	hvgr_write_reg(gdev, cq_control_reg(gdev, cq_bind_config(channel_id, BIN_LOW_IDX)), value)
	hvgr_write_reg(gdev, cq_control_reg(gdev, cq_bind_config(channel_id, BIN_HIGH_IDX)), value)

	value = hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_config(channel_id, BVHG_IDX)));
	value = bind_end_flush_l2_set_no_flush(value);
	hvgr_write_reg(gdev, cq_control_reg(gdev, cq_bind_config(channel_id, BVHG_IDX)), value);
#endif
}

void hvgr_cq_multi_chain_enable_init(struct hvgr_device * const gdev,
	struct hvgr_cq_dev * const gcqctx, uint32_t channel_id)
{
	uint32_t value;

	value = hvgr_read_reg(gdev,
		cq_control_reg(gdev, cq_bind_config(channel_id, RENDER_LOW_IDX)));
	cq_set_u32_bit4(value, gcqctx->hw_cfg.cq_multi_chain_enable);
	hvgr_write_reg(gdev,
		cq_control_reg(gdev, cq_bind_config(channel_id, RENDER_LOW_IDX)), value);

	value = hvgr_read_reg(gdev,
		cq_control_reg(gdev, cq_bind_config(channel_id, RENDER_HIGH_IDX)));
	cq_set_u32_bit4(value, gcqctx->hw_cfg.cq_multi_chain_enable);
	hvgr_write_reg(gdev,
		cq_control_reg(gdev, cq_bind_config(channel_id, RENDER_HIGH_IDX)), value);
}

void hvgr_cq_compute_affinity_init(struct hvgr_device * const gdev,
	uint32_t channel_id, uint32_t core_mask)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	uint32_t cq_cl_affinity = gcqctx->hw_cfg.cq_cl_affinity;

	/* cq_cl_affinity must be part of core_mask */
	cq_cl_affinity &= core_mask;
	if (cq_cl_affinity == 0) {
		hvgr_err(gdev, HVGR_CQ, "[cq]: %s: core_mask=%d, cq_cl_affinity=%d, not right",
			__func__, core_mask, gcqctx->hw_cfg.cq_cl_affinity);
		/* when config fail, report and recover config. */
		cq_cl_affinity = core_mask;
	}

	hvgr_write_reg(gdev,
		cq_control_reg(gdev, cq_bind_affinity_sub_channel(channel_id, COMPUTE_LOW_IDX)),
		cq_cl_affinity);
	hvgr_write_reg(gdev,
		cq_control_reg(gdev, cq_bind_affinity_sub_channel(channel_id, COMPUTE_HIGH_IDX)),
		cq_cl_affinity);
}

void hvgr_cq_set_affinity(struct hvgr_device * const gdev,
	uint32_t channel_id, uint32_t core_mask)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	uint32_t cq_cl_affinity = gcqctx->hw_cfg.cq_cl_affinity;
	uint32_t idx;

	/* cq_cl_affinity must be part of core_mask */
	cq_cl_affinity &= core_mask;
	if (cq_cl_affinity == 0) {
		hvgr_err(gdev, HVGR_CQ, "[cq]: %s: core_mask=%d, cq_cl_affinity=%d, not right",
			__func__, core_mask, gcqctx->hw_cfg.cq_cl_affinity);
		/* when config fail, report and recover config. */
		cq_cl_affinity = core_mask;
	}

	for (idx = RENDER_LOW_IDX; idx < SUBCHN_MAX_IDX; ++idx) {
		if ((idx == COMPUTE_LOW_IDX) || (idx == COMPUTE_HIGH_IDX))
			hvgr_write_reg(gdev, cq_control_reg(gdev,
				cq_bind_affinity_sub_channel(channel_id, idx)), cq_cl_affinity);
		else
			hvgr_write_reg(gdev, cq_control_reg(gdev,
				cq_bind_affinity_sub_channel(channel_id, idx)), core_mask);
	}
}

void hvgr_cq_affinity_config(struct hvgr_device * const gdev, uint32_t channel_id)
{
	int sub_chn;
	uint32_t core_mask = hvgr_pm_get_availiable_cores(gdev);

	hvgr_cq_compute_affinity_init(gdev, channel_id, core_mask);

	for (sub_chn = RENDER_LOW_IDX; sub_chn < SUBCHN_MAX_IDX; sub_chn++)
		if ((sub_chn != COMPUTE_LOW_IDX) && (sub_chn != COMPUTE_HIGH_IDX))
			hvgr_write_reg(gdev,
				cq_control_reg(gdev,
				cq_bind_affinity_sub_channel(channel_id, sub_chn)), core_mask);
}

void hvgr_cq_hwcfg_init(struct hvgr_device * const gdev, uint32_t mode)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	int chnid;

	/* Enable cq mode. */
	hvgr_write_reg(gdev, cq_control_reg(gdev, CQ_ENABLE), mode);

	for (chnid = 0; chnid < CQ_CHANNEL_NUM; chnid++) {
		uint32_t value;

		hvgr_cq_multi_chain_enable_init(gdev, gcqctx, (uint32_t)chnid);

		/* Affinity must precede bc_send_mode. */
		hvgr_cq_affinity_config(gdev, (uint32_t)chnid);

		hvgr_cq_config_bind_cfg(gdev, (uint32_t)chnid);

		hvgr_write_reg(gdev,
			cq_control_reg(gdev, cq_bind_waitcycle(chnid)),
				gcqctx->hw_cfg.cq_bind_waitcycle);
		hvgr_write_reg(gdev,
			cq_control_reg(gdev, cq_bind_timeout(chnid)),
				gcqctx->hw_cfg.cq_queue_empty_timeout);
		hvgr_write_reg(gdev,
			cq_control_reg(gdev, cq_bind_timeout_1(chnid)),
				gcqctx->hw_cfg.cq_semp_wait_timeout);

		value = hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_misccfg(chnid)));
		cq_set_u32_bit4(value, gcqctx->hw_cfg.cq_semp_wait_timeout_switch);
		hvgr_write_reg(gdev, cq_control_reg(gdev, cq_bind_misccfg(chnid)), value);

		/* testchip 300mhz, fpga:30Mhz. */
		hvgr_write_reg(gdev, cq_control_reg(gdev, cq_bind_timeslice(chnid)),
			gcqctx->hw_cfg.cq_serial_timeslice);
	}

	hvgr_write_reg(gdev,
		cq_control_reg(gdev, CQ_SCH_MODE), gcqctx->hw_cfg.cq_sch_mode);
	hvgr_cq_config_bc_send_mode(gdev);
	hvgr_write_reg(gdev, cq_control_reg(gdev, CQ_DBG_EN), gcqctx->hw_cfg.work_mode);
	hvgr_info(gdev, HVGR_CQ, "[cq]: %s: cq_sch_mode=%u, bc_send_mode=%u, work_mode=%u",
		__func__, gcqctx->hw_cfg.cq_sch_mode, gcqctx->hw_cfg.bc_send_mode,
		gcqctx->hw_cfg.work_mode);
}

#define HVGR_QUEUE_STATUS_EMPTY 1
#define HVGR_QUEUE_STATUS_WAIT 3

bool hvgr_cq_get_empty_flag(struct hvgr_device *gdev, uint32_t channel_id)
{
	uint32_t queue_status =
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_0(channel_id)));

	return (queue_status & 0x7U) == HVGR_QUEUE_STATUS_EMPTY;
}

bool hvgr_cq_get_wait_flag(struct hvgr_device *gdev, uint32_t channel_id)
{
	uint32_t queue_status =
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_0(channel_id)));

	/* wait. */
	return (queue_status & 0x7U) == HVGR_QUEUE_STATUS_WAIT;
}

static void hvgr_cq_irq_do_svd(struct hvgr_cq_ctx * const cq_ctx, struct hvgr_device *gdev,
	uint32_t channel_id)
{
	union cq_bind_svd_status token;
#ifdef HVGR_DATAN_JOB_TIMELINE
	uint8_t chain_type;
	uint32_t svd_flag;
#endif
	token.value = hvgr_read_reg(gdev,
		cq_control_reg(gdev, cq_bind_status_1(channel_id)));

	while ((token.st.bind_svd_token_vld == 1) && (token.st.bind_svd_token != 0)) {
#ifdef HVGR_DATAN_JOB_TIMELINE
		svd_flag = 0xff;
		if (token.st.bind_svd_token & 0x1000)
			svd_flag = HVGR_TIMELINE_JOB_CHAIN_START;
		if (token.st.bind_svd_token & 0x2000)
			svd_flag = HVGR_TIMELINE_JOB_CHAIN_END;
		token.st.bind_svd_token &= ~0x3000;
#endif
		hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_TL_NEW_WAIT_IRQ,
			cq_ctx->ctx, cq_ctx->queue_id, channel_id,
			KDATAN_JOBTRACE_IRQ_SVD_STATUS, token.value);

		/* If we found error token, break to avoid deadloop. */
		if (unlikely(!hvgr_token_is_valid(cq_ctx->ctx, token.st.bind_svd_token))) {
			hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "%s token=%d, channel=%d\n",
				__func__, token.st.bind_svd_token, channel_id);
			break;
		}
#ifdef HVGR_DATAN_JOB_TIMELINE
		chain_type = hvgr_cq_get_chain_type(token);
		hvgr_timeline_record_entry_boundary(cq_ctx->ctx, svd_flag,
			chain_type, token.st.bind_svd_token, cq_ctx->queue_id);
#endif
		hvgr_token_finish(cq_ctx, token.st.bind_svd_token);

		token.value = hvgr_read_reg(gdev,
			cq_control_reg(gdev, cq_bind_status_1(channel_id)));
	}
}

static void hvgr_cq_get_profiling_timestamp(struct hvgr_device *gdev,
	struct token_callback_data *callback)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	struct timespec ts;

	getrawmonotonic(&ts);
#else
	struct timespec64 ts;

	ktime_get_raw_ts64(&ts);
#endif

	callback->profiling_data.soc_timestamp = hvgr_dm_get_soc_timestamp(gdev);
	callback->profiling_data.os_timestamp =
		(uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
	hvgr_info(gdev, HVGR_CQ, "soc timestamp is %lu, os timestamp is %lu.",
		callback->profiling_data.soc_timestamp, callback->profiling_data.os_timestamp);
}

static void hvgr_cq_irq_do_notify(struct hvgr_cq_ctx * const cq_ctx, struct hvgr_device *gdev,
	uint32_t channel_id, enum protect_flag *flag)
{
	union cq_bind_notify_status token;
	struct token_callback_data *callback = NULL;
	unsigned long flags;

	token.value = hvgr_read_reg(gdev,
		cq_control_reg(gdev, cq_bind_status_0(channel_id)));

	if (unlikely(!hvgr_token_is_valid(cq_ctx->ctx, token.st.bind_token)))
		return;

	hvgr_info(cq_ctx->ctx->gdev, HVGR_SCH, "%s ctx_%u token=%d, put:get(%d:%d)\n",
		__func__, cq_ctx->ctx->id, token.st.bind_token,
		cq_ctx->sw_wr_ctx->queue_put_index, cq_ctx->sw_wr_ctx->queue_get_index);

	hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_TL_NEW_WAIT_IRQ, cq_ctx->ctx,
		cq_ctx->queue_id, channel_id, KDATAN_JOBTRACE_IRQ_NOTIFY_STATUS, token.value);

	callback = cq_ctx->ctx->ctx_cq.token_callback[token.st.bind_token];
	/* We do not need to check hold_en when the callback->ctx is null. */
	if (callback == NULL || callback->ctx == NULL)
		goto exit;
	spin_lock_irqsave(&callback->lock, flags);
	if ((callback->flags & HVGR_TOKEN_CL_PROFILING_EXEC) != 0 ||
		(callback->flags & HVGR_TOKEN_CL_PROFILING_COMPLETE) != 0)
		hvgr_cq_get_profiling_timestamp(cq_ctx->ctx->gdev, callback);
	callback->flags |= HVGR_TOKEN_NOTIFY_TOP;

	if ((callback->flags & HVGR_TOKEN_HOLD_EN_IN) != 0)
		*flag = CQ_PROTECT_MODE_IN;
	else if ((callback->flags & HVGR_TOKEN_HOLD_EN_OUT) != 0)
		*flag = CQ_PROTECT_MODE_OUT;
	else if ((callback->flags & HVGR_TOKEN_FREQ_HINT) != 0)
		callback->cq_ctx = cq_ctx;
	else
		*flag = CQ_PROTECT_MODE_INVALID;

	spin_unlock_irqrestore(&callback->lock, flags);

#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
	if ((*flag != CQ_PROTECT_MODE_INVALID) && (cq_ctx->priority == CQ_PRIORITY_SPECIAL_LOW)) {
		*flag = CQ_PROTECT_MODE_INVALID;
		hvgr_err(cq_ctx->ctx->gdev, HVGR_SPEC_LOW,
			"%s ctx_%u qid=%u specail low not support protect mode.", __func__,
			cq_ctx->ctx->id, cq_ctx->queue_id);
	}
#endif
exit:
	hvgr_info(cq_ctx->ctx->gdev, HVGR_SCH, "%s hold_en =%d, channel_id=%d, token=%d",
		__func__, *flag, channel_id, token.st.bind_token);
	hvgr_token_finish(cq_ctx, token.st.bind_token);
}

static void hvgr_cq_irq_check_callback(struct hvgr_cq_ctx * const cq_ctx,
	union cq_bind_signal_status status, struct hvgr_ctx *ctx)
{
	struct token_callback_data *callback = NULL;
	unsigned long flags;

	callback = ctx->ctx_cq.token_callback[status.st.token];
	if (callback != NULL) {
		spin_lock_irqsave(&callback->lock, flags);
		if ((callback->flags & HVGR_TOKEN_FREQ_HINT) != 0)
			callback->cq_ctx = cq_ctx;
		if ((callback->flags & HVGR_TOKEN_CL_PROFILING_EXEC) != 0 ||
			(callback->flags & HVGR_TOKEN_CL_PROFILING_COMPLETE) != 0)
			hvgr_cq_get_profiling_timestamp(ctx->gdev, callback);
		spin_unlock_irqrestore(&callback->lock, flags);
	}
	hvgr_token_finish(cq_ctx, status.st.token);
}

static void hvgr_cq_irq_check_status(struct hvgr_cq_ctx * const cq_ctx,
	struct hvgr_device *gdev, uint32_t chnid, uint32_t idx)
{
	union cq_bind_signal_status status;
	struct hvgr_ctx *ctx = cq_ctx->ctx;

	status.value = hvgr_read_reg(gdev,
		cq_control_reg(gdev, cq_bind_status_sub_channel(chnid, idx)));
	while ((status.st.token_vld == 1) && (status.st.token != 0)) {
		hvgr_datan_jobtrace(ctx, JOBTRACE_TYPE_OBJ, KDATAN_TL_NEW_WAIT_IRQ,
			ctx, cq_ctx->queue_id, chnid,
			(uint32_t)KDATAN_JOBTRACE_IRQ_SIGNAL_RENDER_STATUS + idx, status.value);
		hvgr_info(ctx->gdev, HVGR_SCH, "%s ctx_%u token=%d,put:get(%d:%d)\n",
			__func__, ctx->id, status.st.token,
			cq_ctx->sw_wr_ctx->queue_put_index,
			cq_ctx->sw_wr_ctx->queue_get_index);

		/* If we found error token, break to avoid deadloop. */
		if (unlikely(!hvgr_token_is_valid(ctx, status.st.token))) {
			hvgr_info(ctx->gdev, HVGR_SCH,
				"%s token=%d, idx=%d, channel=%d\n",
				__func__, status.st.token, idx, chnid);
			break;
		}

		hvgr_cq_irq_check_callback(cq_ctx, status, ctx);

		status.value = hvgr_read_reg(gdev,
			cq_control_reg(gdev, cq_bind_status_sub_channel(chnid, idx)));
	}
}

static void hvgr_cq_irq_do_signal(struct hvgr_cq_ctx * const cq_ctx,
	struct hvgr_device *gdev, uint32_t chnid, uint32_t sub_chn_mask)
{
	uint32_t idx;

	for (idx = 0; idx < CQ_IRQ_STATUS_SIGNAL_BIT_NUMS; idx++) {
		if ((sub_chn_mask & (1U << idx)) == 0)
			continue;
		hvgr_cq_irq_check_status(cq_ctx, gdev, chnid, idx);
	}
}

static inline struct hvgr_cq_ctx *hvgr_cq_get_channel_ctx(struct hvgr_device *gdev,
	uint32_t channel_id)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;

	return gcqctx->channel_info[channel_id].running_queue;
}

void hvgr_cq_schedule_do_channel_irq(struct hvgr_device * const gdev, uint32_t chn)
{
	union cq_irq irq_state;
	bool sche_en = false;
	struct hvgr_cq_ctx *cq_ctx = hvgr_cq_get_channel_ctx(gdev, chn);
	enum protect_flag flag = CQ_PROTECT_MODE_INVALID;
	bool need_stop = false;

	irq_state.value = hvgr_read_reg(gdev, cq_control_reg(gdev, cq_irq_status(chn)));

	if (unlikely(cq_ctx == NULL)) {
		hvgr_err(gdev, HVGR_CQ, "%s chn(%d) reg value 0x%x not right\n",
			__func__, chn, irq_state.value);
		hvgr_write_reg(gdev, cq_control_reg(gdev, cq_irq_clear(chn)), irq_state.value);
		return;
	}

	if (irq_state.st.svd != 0) {
		hvgr_cq_irq_do_svd(cq_ctx, gdev, chn);
		sche_en = true;
	}

	if ((irq_state.value & CQ_IRQ_STATUS_SIGNAL_BIG_MASK) != 0) {
		hvgr_cq_irq_do_signal(cq_ctx, gdev, chn,
			irq_state.value >> CQ_IRQ_STATUS_SIGNAL_BIT_START);
		sche_en = true;
	}

	if (irq_state.st.notify != 0) {
		hvgr_cq_irq_do_notify(cq_ctx, gdev, chn, &flag);
		sche_en = true;
	}

	if ((irq_state.st.stop == 0) && (irq_state.st.timeout_empty != 0)) {
		hvgr_info(gdev, HVGR_SCH, "%s chn(%d) timeout,qid=%u,ctx_%u, cq_ctx->status=%d\n",
			__func__, chn, cq_ctx->queue_id, cq_ctx->ctx->id, cq_ctx->status);
		need_stop = true;
		sche_en = true;
	}

	/* we resolve stop irq at last, to avoid irq left after stop.
	 * when we receive hold_en, we ignore current stop to enter or exit protect mode.
	 */
	if (irq_state.st.stop != 0) {
		hvgr_info(gdev, HVGR_SCH, "%s chn(%d) stop,qid=%u,ctx_%u\n",
			__func__, chn, cq_ctx->queue_id, cq_ctx->ctx->id);
		hvgr_cq_irq_do_stop(cq_ctx, gdev, chn);
		sche_en = true;
	}

	hvgr_write_reg(gdev, cq_control_reg(gdev, cq_irq_clear(chn)), irq_state.value);
	if (sche_en)
		hvgr_cq_schedule_channel(gdev, chn, flag, need_stop);
}

void hvgr_cq_dump_debug_register(struct hvgr_device * const gdev)
{
	hvgr_debug(gdev, HVGR_CQ, "  CQ_SCH_MODE=0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, CQ_SCH_MODE)));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BC_SEND_MODE=0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, CQ_BC_SEND_MODE)));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_IRQ_STATUS=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_irq_status(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_irq_status(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_IRQ_MASK=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_irq_mask(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_irq_mask(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_ASID=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_asid(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_asid(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_STATUS0=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_0(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_0(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_STATUS1=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_1(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_1(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_STATUS2=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_2(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_2(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_STATUS3=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_3(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_3(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_STATUS4=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_4(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_4(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_STATUS5=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_5(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_5(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_STATUS6=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_6(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_6(1))));

	hvgr_debug(gdev, HVGR_CQ, "  CQ_BIND_SUBCHN_OS=0x%08x    0x%08x",
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_subchn_os(0))),
		hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_subchn_os(1))));

	hvgr_datan_dump_chain_state(gdev);
}

int hvgr_cq_register_irq(struct hvgr_device * const gdev)
{
	if (hvgr_register_irq(gdev, CQ_IRQ, hvgr_cq_irq_handler) != 0) {
		hvgr_err(gdev, HVGR_CQ, "Register cq irq handler failed.");
		return -1;
	}
	return 0;
}

void hvgr_cq_alloc_sw_dfx_memory(struct hvgr_ctx * const ctx,
	struct cq_sw_writeable_context *sw_wr_ctx)
{
}

void hvgr_cq_free_sw_dfx_memory(struct hvgr_ctx * const ctx,
	struct hvgr_cq_ctx * const cq_ctx)
{
}

#ifdef CONFIG_DFX_DEBUG_FS
void hvgr_cq_ordersync_entry_dump(struct hvgr_device * const gdev,
	struct hvgr_cq_ctx * const cq_ctx, bool is_order, bool is_full)
{
}
#endif

void hvgr_cq_config_bc_send_mode(struct hvgr_device *gdev)
{
	uint32_t core_mask = hvgr_pm_get_availiable_cores(gdev);
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	uint32_t bc_send_mode = gcqctx->hw_cfg.bc_send_mode;

	/* We must set bc_send_mode as interleave when single core. */
	if (hweight32(core_mask) == 1)
		bc_send_mode = CQ_BC_SEND_INTERLEAVE;
	hvgr_write_reg(gdev,
		cq_control_reg(gdev, CQ_BC_SEND_MODE), bc_send_mode);
}
