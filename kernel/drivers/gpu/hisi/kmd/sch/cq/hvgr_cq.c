/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_cq.h"

#include <linux/kernel.h>
#include <linux/delay.h>
#include <securec.h>

#include "hvgr_regmap.h"
#include "hvgr_dm_api.h"
#include "hvgr_datan_api.h"
#include "hvgr_sch_api.h"
#include "hvgr_log_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_asid_api.h"

#include "hvgr_kmd_defs.h"
#include "hvgr_cq_driver_base.h"
#include "hvgr_msync.h"
#include "hvgr_cq_queue.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_cq_debug_switch.h"
#include "hvgr_ctx_sched.h"
#include "hvgr_softjob.h"
#include "hvgr_ksync.h"
#include "hvgr_protect_mode.h"
#include "hvgr_cq_driver_adapt.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_ksync_dma_fence.h"
#include "hvgr_softjob.h"

#define SUB_MODULE_GPUJOB  0
#define SUB_MODULE_SOFTJOB 1

void hvgr_sch_switch_on_request(struct hvgr_device *gdev)
{
	hvgr_info(gdev, HVGR_CQ, "%s submit_allow=%d,sch_switch_kref=%d.\n", __func__,
		gdev->cq_dev.submit_allow, atomic_read(&gdev->cq_dev.sch_switch_kref));

	if (atomic_inc_return(&gdev->cq_dev.sch_switch_kref) == 1)
		gdev->cq_dev.submit_allow = true;
}

void hvgr_sch_switch_off_request(struct hvgr_device *gdev)
{
	hvgr_info(gdev, HVGR_CQ, "%s submit_allow=%d,sch_switch_kref=%d.\n", __func__,
		gdev->cq_dev.submit_allow, atomic_read(&gdev->cq_dev.sch_switch_kref));
	if (atomic_dec_return(&gdev->cq_dev.sch_switch_kref) == 0)
		gdev->cq_dev.submit_allow = false;
}

long hvgr_ioctl_alloc_cq(struct hvgr_ctx * const ctx, union hvgr_ioctl_para_alloc_cq * const para)
{
	struct hvgr_cq_ctx *cq_ctx = NULL;
#if defined(HVGR_SCHTIME_SHOW) && HVGR_SCHTIME_SHOW
	struct hvgr_cq_dev *gcqctx = NULL;
	struct cq_dfx_time *cq_schtime_dfx = NULL;
#endif

	if ((ctx == NULL) || (para == NULL))
		return -1;

	if (para->in.priority >= CQ_PRIORITY_MAX) {
		hvgr_err(ctx->gdev, HVGR_CQ, "priority=%d\n", para->in.priority);
		return -EINVAL;
	}

	mutex_lock(&ctx->ctx_lock);
	cq_ctx = hvgr_cq_alloc_queue(ctx, para->in.bind_softq, para->in.all_entry_size,
		para->in.priority);
	if (cq_ctx == NULL) {
		mutex_unlock(&ctx->ctx_lock);
		return -1;
	}

#if defined(HVGR_SCHTIME_SHOW) && HVGR_SCHTIME_SHOW
	gcqctx = &cq_ctx->ctx->gdev->cq_dev;
	cq_schtime_dfx = &cq_ctx->cq_schtime_dfx;
	cq_schtime_dfx->sch_is_free = false;
	cq_schtime_dfx->submit_is_first = false;
	cq_schtime_dfx->bind_count = 0;
	cq_schtime_dfx->unbind_count = 0;

	if (gcqctx->sch_time_enable) {
		cq_schtime_dfx->alloc_time = ktime_to_ns(ktime_get());
		cq_ctx->cq_sch_state_type = HVGR_CQ_ALLOC_STATE;
	}
#endif

	para->out.queue_id = cq_ctx->queue_id;
	para->out.gpu_status_addr = ptr_to_u64(cq_ctx->cq_status_area->uva);
	para->out.sw_wr_context_addr = cq_ctx->sw_ctx_addr;
	para->out.cq_entry_addr = cq_ctx->cq_entry_base_uva;

	mutex_unlock(&ctx->ctx_lock);
	return 0;
}

long hvgr_ioctl_free_cq(struct hvgr_ctx * const ctx, union hvgr_ioctl_para_free_cq * const para)
{
	struct hvgr_cq_ctx *cq_ctx = NULL;

	if ((ctx == NULL) || (para == NULL))
		return -EINVAL;

	mutex_lock(&ctx->ctx_lock);
	cq_ctx = hvgr_cq_find_cqctx(ctx, para->in.queue_id);
	if (cq_ctx == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ, "ctx has freed\n");
		mutex_unlock(&ctx->ctx_lock);
		return -EINVAL;
	}

	hvgr_cq_schedule_unsubmit(ctx, cq_ctx);
	hvgr_cq_free_queue(ctx, cq_ctx);
	mutex_unlock(&ctx->ctx_lock);
	return 0;
}

long hvgr_ioctl_submit_cq(struct hvgr_ctx * const ctx, union hvgr_ioctl_para_submit_cq * const para)
{
	long ret;

	if ((ctx == NULL) || (para == NULL))
		return -1;

	mutex_lock(&ctx->ctx_lock);
	ret = hvgr_cq_schedule_submit(ctx, para->in.queue_id);
	mutex_unlock(&ctx->ctx_lock);

	return ret;
}

int hvgr_cq_dev_init(struct hvgr_device * const gdev)
{
	long ret;
	struct hvgr_cq_dev *cqdev = NULL;

	if (gdev == NULL)
		return -1;

	cqdev = &gdev->cq_dev;
	gdev->sch_dev.sch_sub_dev.cq_dev = cqdev;
	(void)memset_s(cqdev, sizeof(*cqdev), 0, sizeof(*cqdev));
	cqdev->fence_timeout_enable = true;
	cqdev->semp_timeout_enable = true;
	cqdev->entry_timeout_switch = CQ_ENTRY_TIMEOUT_SWITCH;
	cqdev->cq_suspend = false;
	hvgr_info(gdev, HVGR_CQ, "%s submit_allow=%d,sch_switch_kref=%d.\n", __func__,
		gdev->cq_dev.submit_allow, atomic_read(&gdev->cq_dev.sch_switch_kref));
	atomic_set(&gdev->cq_dev.sch_switch_kref, 0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	ida_init(&cqdev->queue_ida);
#endif
	if (hvgr_fence_slab_init() != 0)
		return -1;

	if (hvgr_token_cb_data_slab_init() != 0)
		return -1;

	ret = hvgr_cq_schedule_init(gdev);
	if (ret != 0)
		return (int)ret;

	hvgr_cq_enable(gdev, HVGR_SCH_MODE_CQ, HVGR_SCH_SUBMIT_ALLOW);
	hvgr_cq_intr_switch(gdev, HVGR_SCH_INTRENABLE);
	init_waitqueue_head(&cqdev->stop_queue_done_wait);

	if (hvgr_cq_register_irq(gdev) != 0)
		return -1;

	if (hvgr_register_irq(gdev, JOB_IRQ, hvgr_job_irq_handler) != 0)
		return -1;

	gdev->cq_dev.queue_fault_wq = alloc_workqueue(
		"cq_fault_wait", WQ_UNBOUND, 1);
	if (gdev->cq_dev.queue_fault_wq == NULL)
		return -1;

	gdev->cq_dev.entry_dump_wq = alloc_workqueue("cq_entry_dump", WQ_UNBOUND, 1);
	if (gdev->cq_dev.entry_dump_wq == NULL)
		return -1;

#ifdef CONFIG_DFX_DEBUG_FS
	(void)hvgr_cq_debugfs_init(gdev);
#endif

	return 0;
}

void hvgr_cq_dev_term(struct hvgr_device * const gdev)
{
	hvgr_fence_slab_term();
	hvgr_token_cb_data_slab_term();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	ida_destroy(&gdev->cq_dev.queue_ida);
#endif
	if (gdev->cq_dev.queue_fault_wq != NULL) {
		destroy_workqueue(gdev->cq_dev.queue_fault_wq);
		gdev->cq_dev.queue_fault_wq = NULL;
	}

	if (gdev->cq_dev.entry_dump_wq != NULL) {
		destroy_workqueue(gdev->cq_dev.entry_dump_wq);
		gdev->cq_dev.entry_dump_wq = NULL;
	}
}

void hvgr_cq_enable(struct hvgr_device * const gdev, uint32_t mode, uint32_t sch_flag)
{
	struct hvgr_cq_dev *gcqctx = &gdev->cq_dev;
	unsigned long flags;

	hvgr_cq_hwcfg_init(gdev, mode);

	spin_lock_irqsave(&gcqctx->schedule_lock, flags);
	gcqctx->fault_type = CQ_FAULT_NO_FAULT;
	gcqctx->gpu_sch_sts = HVGR_GPU_IDLE;
	if (sch_flag == HVGR_SCH_SUBMIT_ALLOW) {
		hvgr_info(gdev, HVGR_CQ, "%s submit_allow=%d,sch_switch_kref=%d.\n", __func__,
			gdev->cq_dev.submit_allow, atomic_read(&gdev->cq_dev.sch_switch_kref));
		hvgr_sch_switch_on_request(gdev);
	}
	spin_unlock_irqrestore(&gcqctx->schedule_lock, flags);
}

long hvgr_cq_ctx_init(struct hvgr_ctx * const ctx)
{
	struct hvgr_cq_ctx_data *cqctl = &ctx->ctx_cq;

	if (cqctl == NULL)
		return -1;

	(void)memset_s(cqctl, sizeof(*cqctl), 0, sizeof(*cqctl));

	spin_lock_init(&cqctl->callback_lock);
	spin_lock_init(&cqctl->semp_lock);
	spin_lock_init(&cqctl->event_lock);
	mutex_init(&cqctl->event_data_lock);

	hvgr_ksync_init(ctx);

	if (hvgr_token_init(ctx) != 0)
		return -1;

	atomic_set(&ctx->ctx_cq.trigger_total_cnt, 0);
	atomic_set(&ctx->ctx_cq.wait_total_cnt, 0);
	atomic_set(&ctx->ctx_cq.trigger_uncomplte_cnt, 0);
	atomic_set(&ctx->ctx_cq.wait_uncomplte_cnt, 0);
	atomic_set(&ctx->ctx_cq.trigger_complte_cnt, 0);
	atomic_set(&ctx->ctx_cq.wait_complte_cnt, 0);

	atomic_set(&ctx->softq_closed, false);
	ctx->ctx_cq.err_disp = false;
	return 0;
}

void hvgr_cq_ctx_term(struct hvgr_ctx * const ctx)
{
	mutex_lock(&ctx->ctx_lock);
	hvgr_cq_unsubmit_all_queue(ctx);
	hvgr_token_term(ctx);
	hvgr_cq_free_all_queue(ctx);
	hvgr_msync_term(ctx);
	mutex_unlock(&ctx->ctx_lock);
	kmd_ctx_sched_remove_ctx(ctx);
}

long hvgr_ioctl_cq_clear_err(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_cq_clear_err * const para)

{
	long ret;
	struct hvgr_cq_dev *gcqctx = NULL;

	if (ctx == NULL)
		return -1;

	gcqctx = &ctx->gdev->cq_dev;

	mutex_lock(&gcqctx->err_recover_lock);
	ret = hvgr_cq_clear_err(ctx, para->in.queue_id);
	mutex_unlock(&gcqctx->err_recover_lock);

	return ret;
}

long hvgr_ioctl_cq_set_hold_en_info(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_hold_en_info * const para)
{
	uint32_t token;
	enum protect_flag flag;

	token = para->in.hold_en_token;
	flag = (enum protect_flag)para->in.protect_flag;
	if (((flag != CQ_PROTECT_MODE_IN) && (flag != CQ_PROTECT_MODE_OUT)) ||
		!hvgr_token_is_valid(ctx, token)) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"%s flag %d or token %d not right ", __func__, flag, token);
		return -1;
	}

	if (!hvgr_token_register_hold_en_info(ctx, token, flag, NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"%s hold_en with token=%d register repeatly", __func__, token);
		return -1;
	}

	return 0;
}

long hvgr_ioctl_init_entry_addr(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_init_entry_addr * const para)
{
	if ((ctx == NULL) || (para == NULL))
		return -EINVAL;

	mutex_lock(&ctx->ctx_lock);
	if (ctx->jcd_area == NULL || ctx->jcd_addr == NULL || ctx->jsx_jcd == 0)
		if (hvgr_mem_protect_jcd_init(ctx) != 0) {
			hvgr_err(ctx->gdev, HVGR_CQ, "%s JCD init failed. jcd_count=%d",
				__func__, ctx->gdev->cq_dev.jcd_count);
			mutex_unlock(&ctx->ctx_lock);
			return -EINVAL;
		}

	mutex_unlock(&ctx->ctx_lock);
	return 0;
}

#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
long hvgr_ioctl_mod_queue_priv(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_mod_queue_priv * const para)
{
	struct hvgr_cq_ctx *cq_ctx = NULL;
	struct hvgr_cq_dev *gcqctx = NULL;
	unsigned long flags;
	bool exist = false;

	if ((ctx == NULL) || (para == NULL))
		return -EINVAL;

	gcqctx = &ctx->gdev->cq_dev;

	mutex_lock(&ctx->ctx_lock);
	cq_ctx = hvgr_cq_find_cqctx(ctx, para->in.queue_id);
	if (cq_ctx == NULL) {
		hvgr_err(ctx->gdev, HVGR_SPEC_LOW, "%s can not find cqctx.\n", __func__);
		goto exit;
	}

	if (cq_ctx->mod_priv_his) {
		mutex_unlock(&ctx->ctx_lock);
		return 0;
	}

	if (cq_ctx->priority != CQ_PRIORITY_LOW) {
		hvgr_err(ctx->gdev, HVGR_SPEC_LOW, "%s priority=%d, mod_priv_his=%d.\n",
			__func__, cq_ctx->priority, cq_ctx->mod_priv_his);
		goto exit;
	}

	spin_lock_irqsave(&gcqctx->schedule_lock, flags);
	exist = hvgr_cq_schedule_find_queue(gcqctx, cq_ctx);

	cq_ctx->mod_priv_his = true;
	cq_ctx->priority = CQ_PRIORITY_SPECIAL_LOW;

	if (exist) {
		list_del(&cq_ctx->pending_item);
		(void)hvgr_cq_schedule_enqueue(gcqctx, cq_ctx);
	}
	spin_unlock_irqrestore(&gcqctx->schedule_lock, flags);

	hvgr_info(ctx->gdev, HVGR_SPEC_LOW, "%s ctx_%u qid=%d mod to CQ_PRIORITY_SPECIAL_LOW.\n",
		__func__, ctx->id, cq_ctx->queue_id);
	mutex_unlock(&ctx->ctx_lock);
	return 0;
exit:
	mutex_unlock(&ctx->ctx_lock);
	return -EINVAL;
}
#endif

void hvgr_cq_intr_switch(struct hvgr_device * const gdev, uint32_t flag)
{
	if (flag == HVGR_SCH_INTRDISABLE)
		hvgr_cq_int_operate(gdev, 0);
	else
		hvgr_cq_int_operate(gdev, CQ_IRQ_MASK_VALUE);
}

long hvgr_ioctl_sched(struct hvgr_ctx * const ctx,
	unsigned int cmd, unsigned long arg)
{
	long ret = -1;

	switch (cmd) {
	case HVGR_IOCTL_ALLOC_CQ:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_ALLOC_CQ,
			hvgr_ioctl_alloc_cq, arg, union hvgr_ioctl_para_alloc_cq);
		break;
	case HVGR_IOCTL_FREE_CQ:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_FREE_CQ,
			hvgr_ioctl_free_cq, arg, union hvgr_ioctl_para_free_cq);
		break;
	case HVGR_IOCTL_SUBMIT_CQ:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_SUBMIT_CQ,
			hvgr_ioctl_submit_cq, arg, union hvgr_ioctl_para_submit_cq);
		break;
	case HVGR_IOCTL_ALLOC_SEMP:
		hvgr_ioctl_handle_wr(ctx, HVGR_IOCTL_CMD_ALLOC_SEMP,
			hvgr_ioctl_alloc_semp, arg, union hvgr_ioctl_para_alloc_semp);
		break;
	case HVGR_IOCTL_FREE_SEMP:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CMD_FREE_SEMP,
			hvgr_ioctl_free_semp, arg, union hvgr_ioctl_para_free_semp);
	case HVGR_IOCTL_CQ_CLEAR_ERR:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CQ_CLEAR_ERR,
			hvgr_ioctl_cq_clear_err, arg, union hvgr_ioctl_cq_clear_err);
		break;
#ifdef CONFIG_DFX_DEBUG_FS
	case HVGR_IOCTL_CQ_RECORD_TOKEN:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_CQ_RECORD_TOKEN,
			hvgr_ioctl_cq_record_token, arg, union hvgr_ioctl_cq_record_token);
		break;
#endif
	case HVGR_IOCTL_INIT_ENTRY_ADDR:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_INIT_ENTRY_ADDR,
			hvgr_ioctl_init_entry_addr, arg, union hvgr_ioctl_para_init_entry_addr);
		break;
#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
	case HVGR_IOCTL_MOD_QUEUE_PRIV:
		hvgr_ioctl_handle_w(ctx, HVGR_IOCTL_MOD_QUEUE_PRIV,
			hvgr_ioctl_mod_queue_priv, arg, union hvgr_ioctl_para_mod_queue_priv);
		break;
#endif
	default:
		dev_emerg(ctx->gdev->dev, "Sch ioctl cmd %x is INVALID.", cmd);
		break;
	}

exit:
	return ret;
}

long hvgr_ioctl_sch(struct hvgr_ctx * const ctx,
	unsigned int cmd, unsigned long arg)
{
	uint32_t cmd_submod;

	cmd_submod = _IOC_NR(cmd) >> HVGR_IOCTL_SCH_SUB_MODULE_SPLIT;

	switch (cmd_submod) {
	case SUB_MODULE_GPUJOB:
		return hvgr_ioctl_sched(ctx, cmd, arg);
	case SUB_MODULE_SOFTJOB:
		return hvgr_ioctl_softjob(ctx,cmd, arg);
	default:
		dev_emerg(ctx->gdev->dev, "Sch ioctl cmd %x is INVALID.", cmd);
		break;
	};

	return -EINVAL;
}

static struct hvgr_sch_backend_ops g_sch_backend_ops = {
	.init = hvgr_cq_dev_init,
	.term = hvgr_cq_dev_term,
	.ctx_create = hvgr_cq_ctx_init,
	.ctx_destroy = hvgr_cq_ctx_term,
	.cmd_dispatch = hvgr_ioctl_sch,
	.suspend = hvgr_cq_suspend,
	.resume = hvgr_cq_resume,
	.start = hvgr_cq_schedule_channel_all,
	.stop = hvgr_cq_schedule_stop_submit,
	.ctx_stop = hvgr_cq_unsubmit_all_queue,
	.reset = hvgr_cq_schedule_reset,
	.set_mode = hvgr_cq_enable,
	.intr_switch = hvgr_cq_intr_switch,
};

struct hvgr_sch_backend_ops *hvgr_sched_get_ops(void)
{
	return &g_sch_backend_ops;
}
