/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_cq_queue.h"

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/rcupdate.h>
#include <securec.h>

#include "hvgr_kmd_defs.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_mem_api.h"
#include "hvgr_datan_api.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_wq.h"
#include "hvgr_log_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_softq.h"
#include "hvgr_softjob.h"
#include "hvgr_dm_api.h"
#include "hvgr_regmap.h"
#include "hvgr_cq_driver_base.h"
#include "hvgr_datan_api.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static int hvgr_cq_alloc_queueid(struct hvgr_ctx * const ctx)
{
	int queue_id = 0;

	queue_id = ida_alloc(&ctx->gdev->cq_dev.queue_ida, GFP_KERNEL);
	if (queue_id < 0) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u, get qid error=%d\n",
			__func__, ctx->id, queue_id);
		return -1;
	}
	return queue_id;
}
#else
static uint32_t hvgr_cq_alloc_queueid(void)
{
	static atomic_t g_queue_id = ATOMIC_INIT(0);

	return atomic_inc_return(&g_queue_id);
}
#endif

static struct hvgr_cq_ctx *hvgr_cq_alloc_cq_ctx(struct hvgr_ctx * const ctx)
{
	uint32_t idx;

	struct hvgr_cq_ctx *cq_ctx = NULL;

	for (idx = 0; idx < ARRAY_SIZE(ctx->ctx_cq.cq_ctx); idx++) {
		if (ctx->ctx_cq.cq_ctx[idx].sw_wr_ctx == NULL) {
			cq_ctx = &ctx->ctx_cq.cq_ctx[idx];
			break;
		}
	}

	return cq_ctx;
}

static void hvgr_cq_free_cq_ctx(struct hvgr_ctx * const ctx, struct hvgr_cq_ctx * const cq_ctx)
{
	(void)memset_s(cq_ctx, sizeof(*cq_ctx), 0, sizeof(*cq_ctx));
}

static void hvgr_cq_free_sw_writeable_memory(struct hvgr_ctx * const ctx,
	struct hvgr_cq_ctx * const cq_ctx)
{
	if (unlikely((ctx == NULL) || (cq_ctx == NULL)))
		return;

	hvgr_cq_free_sw_dfx_memory(ctx, cq_ctx);
	(void)hvgr_mem_free(ctx, cq_ctx->gpu_context_buffer->gva);
}

static void hvgr_cq_free_queue_memory(struct hvgr_ctx * const ctx,
	struct hvgr_cq_ctx * const cq_ctx)
{
	if (cq_ctx->gpu_context_buffer != NULL) {
		hvgr_cq_free_sw_writeable_memory(ctx, cq_ctx);
		cq_ctx->sw_wr_ctx = NULL;
		cq_ctx->gpu_context_buffer = NULL;
	}

	if (cq_ctx->queue_entry_area != NULL) {
		hvgr_mem_free(ctx, cq_ctx->queue_entry_area->gva);
		cq_ctx->cq_entry_base = NULL;
		cq_ctx->queue_entry_area = NULL;
		cq_ctx->cq_entry_base_uva = 0;
	}
}

static struct hvgr_mem_area *hvgr_cq_alloc_sw_writeable_memory(struct hvgr_ctx * const ctx)
{
	uint64_t size = 0;
	struct hvgr_mem_alloc_para para;

	size = hvgr_mem_page_align(cq_ringbuf_align(sizeof(struct cq_sw_writeable_context)));
	para.property = HVGR_MEM_PROPERTY_SAME_VA;
	para.attribute = HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR |
		HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_GPU_WR | HVGR_MEM_ATTR_KMD_ACCESS;
	para.pages = (uint32_t)hvgr_page_nums(size);
	para.max_pages = para.pages;
	return hvgr_mem_allocate(ctx, &para, NULL, false);
}

static bool hvgr_cq_alloc_queue_memory(struct hvgr_cq_ctx * const cq_ctx,
	struct hvgr_ctx * const ctx, uint32_t all_entry_size)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_alloc_para para;

	/*
	 * Alloc memory include one SW-Writeable.
	 * Every struct size must align to 64B.
	 */
	uint64_t size = 0;

	if (unlikely(all_entry_size != sizeof(struct cq_cmdq_entry) * CQ_QUEUE_ENTRY_NUMBER)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u, queue size not right(%u)\n",
			__func__, ctx->id, all_entry_size);
		return false;
	}

	size = hvgr_mem_page_align(all_entry_size);

#if defined(HVGR_CMDQ_PROPERTY) && HVGR_CMDQ_PROPERTY != 0
	para.property = HVGR_MEM_PROPERTY_4G_VA;
#else
	para.property = HVGR_MEM_PROPERTY_SAME_VA;
#endif
	para.attribute = HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR | HVGR_MEM_ATTR_GPU_RD |
		HVGR_MEM_ATTR_GPU_WR | HVGR_MEM_ATTR_KMD_ACCESS;
	para.pages = (uint32_t)hvgr_page_nums(size);
	para.max_pages = para.pages;
	area = hvgr_mem_allocate(ctx, &para, NULL, false);
	if (!area)
		return false;
	cq_ctx->queue_entry_area = area;
	cq_ctx->cq_entry_base = u64_to_ptr(area->kva);
	cq_ctx->cq_entry_base_uva = area->uva;

	area = hvgr_cq_alloc_sw_writeable_memory(ctx);
	if (!area) {
		hvgr_mem_free(ctx, cq_ctx->queue_entry_area->gva);
		cq_ctx->queue_entry_area = NULL;
		cq_ctx->cq_entry_base = NULL;
		cq_ctx->cq_entry_base_uva = 0;
		return false;
	}

	cq_ctx->gpu_context_buffer = area;
	cq_ctx->sw_wr_ctx = u64_to_ptr(area->kva);
	cq_ctx->sw_ctx_addr = area->uva;

	hvgr_cq_alloc_sw_dfx_memory(ctx, cq_ctx->sw_wr_ctx);
	return true;
}

static void hvgr_cq_show_hang_ctx_info(struct hvgr_device *gdev, struct hvgr_ctx *ctx,
	struct hvgr_cq_ctx *cq_ctx, char *dmd_msg, int *len)
{
	int size = *len;

	hvgr_debug(gdev, HVGR_CQ, "gpu hang, tgid: %d tsk_name: %s\n",
		ctx->tgid, ctx->process_name);
	hvgr_debug(gdev, HVGR_CQ, "%s schedule timeout qid=%u ctx_%u\n",
		__func__, cq_ctx->queue_id, ctx->id);
	if (size == 0)
		size += snprintf_s(dmd_msg + size, MAX_DMD_LOG_LENTH - size,
			MAX_DMD_LOG_LENTH - size - 1, "higpu job hang,task:%s,%d ",
			hvgr_get_main_process_name(ctx), hvgr_get_ctx_compat(ctx));
	else
		size += snprintf_s(dmd_msg + size, MAX_DMD_LOG_LENTH - size,
			MAX_DMD_LOG_LENTH - size - 1, "task:%s,%d ",
			hvgr_get_main_process_name(ctx), hvgr_get_ctx_compat(ctx));
	*len = size;
}

static void hvgr_cq_show_all_hang_ctx_info(struct hvgr_device *gdev, struct hvgr_ctx *ctx,
	struct hvgr_cq_ctx *cq_ctx)
{
	uint32_t idx;
	int len = 0;
	struct hvgr_cq_ctx *cq_ctx_other = NULL;
	char dmd_msg[MAX_DMD_LOG_LENTH] = {0};

	hvgr_info(gdev, HVGR_CQ, "%s in.\n", __func__);
	/* show hang process info */
	hvgr_cq_show_hang_ctx_info(gdev, ctx, cq_ctx, dmd_msg, &len);
	/* show other process info */
	for (idx = 0; idx < ARRAY_SIZE(gdev->cq_dev.channel_info); idx++) {
		cq_ctx_other = gdev->cq_dev.channel_info[idx].running_queue;
		if (cq_ctx_other == NULL || cq_ctx_other->ctx == ctx)
			continue;
		hvgr_cq_show_hang_ctx_info(gdev, cq_ctx_other->ctx, cq_ctx_other, dmd_msg, &len);
	}

	if (len > 0)
		hvgr_datan_gpu_fault_type_set(gdev, DMD_JOB_HANG, dmd_msg);
	else
		hvgr_datan_gpu_fault_type_set(gdev, DMD_JOB_HANG, "higpu job hang");
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static void backup_timer_callback(struct timer_list *timer)
{
#else
static void backup_timer_callback(unsigned long addr)
{
	struct timer_list *timer = (struct timer_list *)(uintptr_t)addr;
#endif
	struct hvgr_cq_ctx *cq_ctx =
		container_of(timer, struct hvgr_cq_ctx, backup_timer);
	unsigned long flags;
	struct hvgr_ctx *ctx = cq_ctx->ctx;
	struct hvgr_device *gdev = NULL;
	struct hvgr_cq_dev *gcqctx = NULL;
	bool running = false;
	bool pm_ready = false;

	if (ctx == NULL) {
		pr_err("%s ctx may be has freed.\n", __func__);
		return;
	}

	gdev = ctx->gdev;
	gcqctx = &gdev->cq_dev;

	spin_lock_irqsave(&gcqctx->schedule_lock, flags);

	running = hvgr_cq_is_queue_running(gcqctx, cq_ctx);
	pm_ready = hvgr_pm_get_gpu_status(ctx->gdev) == HVGR_PM_GPU_READY;
	if (!running || !pm_ready) {
		hvgr_err(gdev, HVGR_CQ, "%s Not right, running: %d, pm_ready=%d\n",
			__func__, running, pm_ready);
		spin_unlock_irqrestore(&gcqctx->schedule_lock, flags);
		return;
	}

	if (hvgr_is_cl_task(ctx->gdev, cq_ctx->channel_id) && !cq_ctx->cl_tried) {
		long timeout;

		if (gcqctx->reset_timeout_cl > gcqctx->reset_timeout_normal) {
			timeout = gcqctx->reset_timeout_cl - gcqctx->reset_timeout_normal;
			cq_ctx->cl_tried = true;
			spin_unlock_irqrestore(&gcqctx->schedule_lock, flags);
			hvgr_info(gdev, HVGR_CQ, "%s cl mode timer, delay %ds, qid=%u ctx_%u\n",
				__func__, timeout, cq_ctx->queue_id, ctx->id);
			(void)mod_timer(&cq_ctx->backup_timer,
				jiffies + (uint32_t)timeout * HZ);
			return;
		}
	}
	/* The dmd is set only when the first hang process comes in. */
	if (hvgr_dmd_id_check(gdev)) {
		hvgr_cq_show_all_hang_ctx_info(gdev, ctx, cq_ctx);
		hvgr_cq_append_flag_all_queue(gdev, CQ_KMD_CTX_TIME_OUT_HANG, false);
	}
#if defined(CONFIG_HVGR_DFX_SH) && defined(CONFIG_HVGR_VIRTUAL_GUEST)
	if (gdev->pm_dev.pm_pwr.present_cores[HVGR_PM_CORE_GPC] !=
		(uint32_t)(atomic_read(&gdev->pm_dev.pm_pwr.core_mask_info)))
		(void)queue_work(gdev->pm_dev.pm_pwr.sh_hang_wq, &gdev->pm_dev.pm_pwr.sh_hang_work);
#endif
	if (!gcqctx->cq_suspend)
		hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_NORMAL);
	else
		hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_IMMEDIATELY);

	spin_unlock_irqrestore(&gcqctx->schedule_lock, flags);
}

#ifdef CONFIG_HVGR_DFX_MEM_DUMP
static void hvgr_gpu_fault_mem_dump(struct work_struct *data)
{
	struct hvgr_cq_ctx *cq_ctx = NULL;

	cq_ctx = container_of(data, struct hvgr_cq_ctx, queue_fault_work);
	hvgr_datan_dump_mem(cq_ctx->ctx);

	hvgr_token_finish(cq_ctx, CQ_ERR_FLOW_TOKEN_VALUE);

	mutex_lock(&cq_ctx->dump_mem_lock);
	if (cq_ctx->dump_mem_flag) {
		hvgr_debug(cq_ctx->ctx->gdev, HVGR_CQ, "%s wake_up queue_done_wait\n", __func__);
		cq_ctx->dump_mem_flag = false;
		wake_up(&cq_ctx->queue_done_wait);
	}
	mutex_unlock(&cq_ctx->dump_mem_lock);
}
#endif

#ifdef CONFIG_DFX_DEBUG_FS
static void hvgr_cq_order_dump_handle(struct work_struct *data)
{
	struct hvgr_cq_ctx *cq_ctx = container_of(data, struct hvgr_cq_ctx, order_dump_work);
	if (cq_ctx->ctx == NULL)
		return;

	hvgr_cq_ordersync_entry_dump(cq_ctx->ctx->gdev, cq_ctx, ORDER_DUMP, true);
}

static void hvgr_cq_sync_dump_handle(struct work_struct *data)
{
	struct hvgr_cq_ctx *cq_ctx = container_of(data, struct hvgr_cq_ctx, sync_dump_work);
	if (cq_ctx->ctx == NULL)
		return;

	hvgr_cq_ordersync_entry_dump(cq_ctx->ctx->gdev, cq_ctx, SYNC_DUMP, true);
}
#endif

static bool hvgr_cq_alloc_status_mem(struct hvgr_cq_ctx * const cq_ctx, struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_alloc_para para;
	struct hvgr_mem_area *area = NULL;

	para.property = HVGR_MEM_PROPERTY_DRIVER;
	para.attribute = HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_KMD_ACCESS;
	para.pages = (uint32_t)hvgr_page_nums(sizeof(struct hvgr_cq_status));
	para.max_pages = para.pages;

	area = hvgr_mem_allocate(ctx, &para, NULL, false);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ, "alloc cq status fail\n");
		return false;
	}

	cq_ctx->cq_status_area = area;
	cq_ctx->cq_status = u64_to_ptr(area->kva);

	hvgr_info(ctx->gdev, HVGR_CQ, "%s ctx_%u alloc status mem, uva=0x%lx\n",
		__func__, ctx->id, area->uva);
	return true;
}

static void hvgr_cq_free_status_mem(struct hvgr_cq_ctx * const cq_ctx, struct hvgr_ctx * const ctx)
{
	if (cq_ctx->cq_status != NULL) {
		hvgr_info(ctx->gdev, HVGR_CQ, "%s ctx_%u free status mem, uva=0x%lx\n",
			__func__, ctx->id, cq_ctx->cq_status_area->uva);
		(void)hvgr_mem_free(ctx, cq_ctx->cq_status_area->uva);
		cq_ctx->cq_status_area = NULL;
		cq_ctx->cq_status = NULL;
	}
}

struct hvgr_cq_ctx *hvgr_cq_alloc_queue(struct hvgr_ctx * const ctx, uint32_t softq_id,
	uint32_t all_entry_size, uint32_t priority)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	int queue_id;
#endif
	struct hvgr_cq_ctx *cq_ctx = NULL;

	if (ctx == NULL)
		return NULL;

	cq_ctx = hvgr_cq_alloc_cq_ctx(ctx);
	if (cq_ctx == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u alloc cqctx fail\n", __func__, ctx->id);
		return NULL;
	}

	init_waitqueue_head(&cq_ctx->queue_done_wait);

	/* Alloc hardware memory */
	if (!hvgr_cq_alloc_queue_memory(cq_ctx, ctx, all_entry_size)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u alloc queue fail\n", __func__, ctx->id);
		hvgr_cq_free_cq_ctx(ctx, cq_ctx);
		return NULL;
	}

	/* Init work queue context */
	if (hvgr_wq_ctx_init(ctx, cq_ctx, softq_id) != 0) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u wq init fail\n", __func__, ctx->id);
		hvgr_cq_free_queue_memory(ctx, cq_ctx);
		hvgr_cq_free_cq_ctx(ctx, cq_ctx);
		return NULL;
	}

	/* Init cq status */
	if (!hvgr_cq_alloc_status_mem(cq_ctx, ctx)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u status mem fail\n", __func__, ctx->id);
		hvgr_cq_free_queue_memory(ctx, cq_ctx);
		hvgr_wq_ctx_term(cq_ctx);
		hvgr_cq_free_cq_ctx(ctx, cq_ctx);
		return NULL;
	}

	cq_ctx->priority = priority;
#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
	cq_ctx->mod_priv_his = false;
#endif
	cq_ctx->status = CQ_STATUS_IDLE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	queue_id = hvgr_cq_alloc_queueid(ctx);
	if (queue_id == -1) {
		hvgr_cq_free_status_mem(cq_ctx, ctx);
		hvgr_cq_free_queue_memory(ctx, cq_ctx);
		hvgr_wq_ctx_term(cq_ctx);
		hvgr_cq_free_cq_ctx(ctx, cq_ctx);
		return NULL;
	}
	cq_ctx->queue_id = (uint32_t)queue_id;
#else
	cq_ctx->queue_id = hvgr_cq_alloc_queueid();
#endif
	cq_ctx->ctx = ctx;
	cq_ctx->channel_id = CQ_CHANNEL_ID_INVALID;
	cq_ctx->flags |= CQ_KMD_CTX_START_SUBMIT;
	hvgr_cq_sched_update_dvfs_status(cq_ctx, HVGR_THRO_IDLE, HVGR_THRO_ACTION_RESTORE_INITAL);

	kmd_timer_setup(&cq_ctx->backup_timer, backup_timer_callback);

	cq_ctx->cl_tried = false;

	mutex_init(&cq_ctx->dump_mem_lock);
#ifdef CONFIG_HVGR_DFX_MEM_DUMP
	INIT_WORK(&cq_ctx->queue_fault_work, hvgr_gpu_fault_mem_dump);
#endif

	mutex_init(&cq_ctx->dump_order_lock);
	mutex_init(&cq_ctx->dump_sync_lock);
#ifdef CONFIG_DFX_DEBUG_FS
	INIT_WORK(&cq_ctx->order_dump_work, hvgr_cq_order_dump_handle);
	INIT_WORK(&cq_ctx->sync_dump_work, hvgr_cq_sync_dump_handle);
#endif
	hvgr_info(ctx->gdev, HVGR_CQ, "[cq]: ctx_%u alloc_cq, qid=%u",
		ctx->id, cq_ctx->queue_id);
	return cq_ctx;
}

void hvgr_cq_free_queue(struct hvgr_ctx * const ctx, struct hvgr_cq_ctx *cq_ctx)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	uint32_t queue_id = cq_ctx->queue_id;
#endif
#if defined(HVGR_SCHTIME_SHOW) && HVGR_SCHTIME_SHOW
	struct cq_dfx_time *cq_schtime_dfx = NULL;
	struct hvgr_cq_dev *gcqctx = NULL;

	gcqctx = &cq_ctx->ctx->gdev->cq_dev;
	cq_schtime_dfx = &cq_ctx->cq_schtime_dfx;
#endif
	del_timer_sync(&cq_ctx->backup_timer);

	if (ctx->ctx_cq.token_callback_wq != NULL)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		flush_workqueue(ctx->ctx_cq.token_callback_wq);
#else
		kthread_flush_worker(ctx->ctx_cq.token_callback_wq);
#endif

	hvgr_wq_ctx_term(cq_ctx);
	hvgr_cq_free_status_mem(cq_ctx, ctx);
	hvgr_cq_free_queue_memory(ctx, cq_ctx);
	hvgr_cq_free_cq_ctx(ctx, cq_ctx);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	ida_free(&ctx->gdev->cq_dev.queue_ida, queue_id);
	hvgr_info(ctx->gdev, HVGR_CQ, "[cq]: ctx_%u free mgr_qid=%d",
		ctx->id, queue_id);
#endif

#if defined(HVGR_SCHTIME_SHOW) && HVGR_SCHTIME_SHOW
	if (gcqctx->sch_time_enable) {
		if (cq_ctx->cq_sch_state_type == HVGR_CQ_UNBIND_STATE ||
			cq_ctx->cq_sch_state_type == HVGR_CQ_SUBMIT_STATE) {
			cq_schtime_dfx->sch_is_free = true;
			cq_schtime_dfx->free_time = ktime_to_ns(ktime_get());
			cq_ctx->cq_sch_state_type = HVGR_CQ_FREE_STATE;
		}
	}
#endif
}

struct hvgr_cq_ctx *hvgr_cq_find_cqctx(struct hvgr_ctx * const ctx, uint32_t queue_id)
{
	uint32_t idx;

	if (ctx == NULL)
		return NULL;

	for (idx = 0; idx < ARRAY_SIZE(ctx->ctx_cq.cq_ctx); idx++) {
		if (ctx->ctx_cq.cq_ctx[idx].sw_wr_ctx == NULL)
			continue;

		if (ctx->ctx_cq.cq_ctx[idx].queue_id == queue_id)
			return (struct hvgr_cq_ctx * const)&ctx->ctx_cq.cq_ctx[idx];
	}

	return (struct hvgr_cq_ctx * const)NULL;
}
void hvgr_cq_append_all_queue_with_ctx(struct hvgr_device *gdev, struct hvgr_ctx *ctx,
	uint32_t flag)
{
	uint32_t idx;
	struct hvgr_cq_ctx *cq_ctx = NULL;

	hvgr_info(gdev, HVGR_CQ, "%s in, flag=0x%x\n", __func__, flag);
	for (idx = 0; idx < CQ_QUEUE_NUM_PER_CTX; idx++) {
		cq_ctx = &ctx->ctx_cq.cq_ctx[idx];
		if ((cq_ctx == NULL) || (cq_ctx->cq_status == NULL))
			continue;

		cq_ctx->flags |= flag;
		if (flag == CQ_KMD_CTX_ERROR_FLAG) {
			cq_ctx->cq_status->err_flags = CQ_ERROR;
			cq_ctx->err_flags = CQ_ERROR;
		}
		hvgr_info(gdev, HVGR_CQ, "%s in, qid=%u, ctx_%u, flag=0x%x\n",
			__func__, cq_ctx->queue_id, cq_ctx->ctx->id, cq_ctx->flags);
	}
}

void hvgr_cq_free_all_queue(struct hvgr_ctx * const ctx)
{
	uint32_t idx;

	if (ctx == NULL)
		return;

	for (idx = 0; idx < ARRAY_SIZE(ctx->ctx_cq.cq_ctx); idx++) {
		if (ctx->ctx_cq.cq_ctx[idx].sw_wr_ctx == NULL)
			continue;
		hvgr_cq_free_queue(ctx, &ctx->ctx_cq.cq_ctx[idx]);
	}

	for (idx = 0; idx < ARRAY_SIZE(ctx->id_to_qctx); ++idx) {
		if (ctx->id_to_qctx[idx] == NULL)
			continue;
		hvgr_softq_term(ctx->id_to_qctx[idx]);
	}
}

void hvgr_cq_unsubmit_all_queue(struct hvgr_ctx * const ctx)
{
	uint32_t idx;

	if (unlikely(ctx == NULL))
		return;

	for (idx = 0; idx < ARRAY_SIZE(ctx->ctx_cq.cq_ctx); idx++) {
		if (ctx->ctx_cq.cq_ctx[idx].sw_wr_ctx == NULL)
			continue;
		hvgr_cq_schedule_unsubmit(ctx, &ctx->ctx_cq.cq_ctx[idx]);
	}
}

void hvgr_cq_append_flag_all_queue(struct hvgr_device *gdev, uint32_t flag, bool expend_all_err)
{
	uint32_t idx;
	struct hvgr_cq_ctx *cq_ctx = NULL;

	hvgr_info(gdev, HVGR_CQ, "%s in, flag=0x%x\n", __func__, flag);
	for (idx = 0; idx < ARRAY_SIZE(gdev->cq_dev.channel_info); idx++) {
		cq_ctx = gdev->cq_dev.channel_info[idx].running_queue;
		if (cq_ctx == NULL)
			continue;
		if (expend_all_err)
			hvgr_cq_append_all_queue_with_ctx(gdev, cq_ctx->ctx, flag);
		else
			cq_ctx->flags |= flag;
		hvgr_info(gdev, HVGR_CQ, "%s in, qid=%u, ctx_%u, flag=0x%x\n",
			__func__, cq_ctx->queue_id, cq_ctx->ctx->id, cq_ctx->flags);
	}
}
