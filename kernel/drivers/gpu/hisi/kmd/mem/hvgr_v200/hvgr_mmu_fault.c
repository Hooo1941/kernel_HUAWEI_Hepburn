/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mmu_api.h"
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <securec.h>

#include "hvgr_regmap.h"
#include "hvgr_mem_zone.h"
#include "hvgr_mem_opts.h"
#include "hvgr_mem_pool.h"
#include "hvgr_memory_data.h"
#include "hvgr_mem_api.h"
#include "hvgr_memory_page.h"
#include "hvgr_dm_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_asid_api.h"
#include "hvgr_log_api.h"
#include "hvgr_mmu_driver.h"
#include "hvgr_platform_api.h"
#include "hvgr_datan_api.h"

#define MMU_BUS_FAULT 0x1
#define MMU_PAGE_FAULT 0x2
#define mmu_page_fault_bit(as) (1U << (as))
#define MMU_DECODE_BIT_OFFSET 10

static void hvgr_mmu_clear_fault(struct hvgr_device * const gdev, struct hvgr_mmu_as *mmu_as)
{
	uint32_t fault_mask;
	unsigned long flags;

	spin_lock_irqsave(&gdev->mem_dev.mmu_hw_lock, flags);
	fault_mask = (uint32_t)mmu_page_fault_bit(mmu_as->no);
	if (mmu_as->fault_type == MMU_BUS_FAULT)
		fault_mask |= mmu_bus_fault_bit(mmu_as->no);

	hvgr_write_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_CLEAR), fault_mask);
	spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);
}

static void hvgr_mmu_unmask_fault(struct hvgr_device * const gdev, struct hvgr_mmu_as *mmu_as)
{
	uint32_t fault_mask;
	unsigned long flags;

	spin_lock_irqsave(&gdev->mem_dev.mmu_hw_lock, flags);
	fault_mask = hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_MASK));
	fault_mask |= (uint32_t)mmu_page_fault_bit(mmu_as->no);
	if (mmu_as->fault_type == MMU_BUS_FAULT)
		fault_mask |= mmu_bus_fault_bit(mmu_as->no);

	hvgr_write_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_MASK), fault_mask);
	spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);
}

static const char *hvgr_mmu_get_access_type(uint32_t fault_stat)
{
	switch (fault_stat & AS_ACCESS_TYPE_MASK) {
	case AS_ACCESS_TYPE_ATOMIC:
		return "UNKNOWN";
	case AS_ACCESS_TYPE_READ:
		return "READ";
	case AS_ACCESS_TYPE_WRITE:
		return "WRITE";
	case AS_ACCESS_TYPE_EX:
		return "EXECUTE";
	default:
		WARN_ON(1);
		return NULL;
	}
}

static void hvgr_mmu_show_fault_info(struct hvgr_device * const gdev, struct hvgr_ctx *ctx,
	struct hvgr_mmu_as *mmu_as, char *reason)
{
	uint32_t source_id = (mmu_as->fault_stat >> AS_SOURCE_ID_OFFSET) & AS_SOURCE_ID_MASK;
	uint32_t exception_type = (mmu_as->fault_stat >> AS_EXCEPTION_CODE_OFFSET) & 0xFF;
	uint32_t access_type = (mmu_as->fault_stat >> AS_ACCESS_TYPE_OFFSET) & AS_ACCESS_TYPE_MASK;
	uint32_t stream_id = (mmu_as->fault_stat >> MMU_STREAM_ID_OFFSET) & MMU_STREAM_ID_MASK;
	uint32_t utlb_id = (mmu_as->fault_stat >> MMU_UTLB_ID_OFFSET) & MMU_UTLB_ID_MASK;
	uint32_t fault_adrmode = hvgr_mmu_get_fault_adrmode(mmu_as->fault_stat);
	uint32_t mem_info = hvgr_mmu_get_mem_info(mmu_as->fault_stat);

	if (mmu_as->fault_type == MMU_BUS_FAULT) {
		hvgr_err(gdev, HVGR_MEM, "Bus fault is AS%u at VA 0x%016llx",
			mmu_as->no, mmu_as->fault_addr);
		return;
	}

	hvgr_err(gdev, HVGR_MEM,
		"Unhandled Page fault in AS%u at VA %pK\n"
		"Reason: %s\n"
		"raw fault status: 0x%x\n"
		"decoded fault status: %s\n"
		"exception type 0x%x\n"
		"access type 0x%x: %s\n"
		"source id 0x%x\n"
		"utlb id 0x%x\n"
		"stream id 0x%x\n"
		"fault_adrmode 0x%x\n"
		"mem_info 0x%x\n"
		"pid: %d\n"
		"tgid: %d\n"
		"tsk_name: %s\n",
		mmu_as->no, (void *)mmu_as->fault_addr, reason, mmu_as->fault_stat,
		(mmu_as->fault_stat & ((uint32_t)1 << MMU_DECODE_BIT_OFFSET) ?
		"DECODER FAULT" : "SLAVE FAULT"),
		exception_type,
		access_type, hvgr_mmu_get_access_type(mmu_as->fault_stat),
		source_id, utlb_id, stream_id, fault_adrmode, mem_info,
		(ctx == NULL ? 0 : ctx->pid), (ctx == NULL ? 0 : ctx->tgid),
		(ctx == NULL ? "unknown" : ctx->process_name));
}

static void hvgr_mmu_reset_gpu(struct hvgr_ctx *ctx, struct hvgr_mmu_as *mmu_as, char *reason)
{
	uint32_t source_id;
	uint32_t stream_id;
	uint32_t exception_type;
	enum hvgr_dmd_id dmd_id;
	int ret;

	char dmd_msg[MAX_DMD_LOG_LENTH] = {0};

	source_id = (mmu_as->fault_stat >> AS_SOURCE_ID_OFFSET) & AS_SOURCE_ID_MASK;
	stream_id = (mmu_as->fault_stat >> MMU_STREAM_ID_OFFSET) & MMU_STREAM_ID_MASK;
	exception_type = (mmu_as->fault_stat >> AS_EXCEPTION_CODE_OFFSET) & (uint32_t)0xFF;
	if (mmu_as->fault_type == MMU_BUS_FAULT) {
		dmd_id = DMD_BUS_FAULT;
		ret = snprintf_s(dmd_msg, MAX_DMD_LOG_LENTH, MAX_DMD_LOG_LENTH - 1,
			"higpu bus fault");
	} else {
		dmd_id = DMD_PAGE_FAULT;
		ret = snprintf_s(dmd_msg, MAX_DMD_LOG_LENTH, MAX_DMD_LOG_LENTH - 1,
			"%s,%s,exception_type:0x%x,source_id:0x%x,stream_id:0x%x,task:%s,%d",
			"higpu page fault", hvgr_mmu_get_access_type(mmu_as->fault_stat),
			exception_type, source_id, stream_id, hvgr_get_main_process_name(ctx),
			hvgr_get_ctx_compat(ctx));
	}

	if (ret > 0)
		hvgr_datan_gpu_fault_type_set(ctx->gdev, dmd_id, dmd_msg);

#ifdef CONFIG_HVGR_DFX_SH
	hvgr_dmd_set_sid(ctx, source_id);
#endif
	hvgr_mmu_show_fault_info(ctx->gdev, ctx, mmu_as, reason);
	hvgr_mmu_clear_fault(ctx->gdev, mmu_as);

	hvgr_pm_gpu_reset(ctx->gdev, GPU_RESET_TYPE_NORMAL);
}

static void hvgr_mmu_proc_no_ctx_fault(struct hvgr_device * const gdev, struct hvgr_mmu_as *mmu_as)
{
	hvgr_mmu_show_fault_info(gdev, NULL, mmu_as, "No context");

	hvgr_mmu_clear_fault(gdev, mmu_as);
	hvgr_mmu_unmask_fault(gdev, mmu_as);
}

static bool hvgr_mmu_fault_area_illegal(struct hvgr_mem_area *fault_area, char **reset_reason)
{
	if (fault_area == NULL) {
		*reset_reason = "Memory is not mapped on the GPU";
		return true;
	}

	if ((fault_area->attribute & HVGR_MEM_ATTR_GROWABLE) == 0) {
		*reset_reason = "Memory is not growable";
		return true;
	}

	return false;
}

static bool hvgr_mmu_fault_no_need_proc(struct hvgr_mmu_as *mmu_as,
	struct hvgr_mem_area *fault_area, uint64_t *add_pages)
{
	uint64_t offset;
	uint64_t offset_page;
	uint64_t new_pages;
	uint64_t temp_pages;

	offset = mmu_as->fault_addr - fault_area->gva;
	offset_page = offset >> PAGE_SHIFT;

	if (offset_page < fault_area->pages)
		return true;

	new_pages = offset_page - fault_area->pages + 1;
	if (fault_area->extent != 0) {
		temp_pages = new_pages % fault_area->extent;
		if (temp_pages != 0)
			new_pages = new_pages + fault_area->extent - temp_pages;
	}

	if (new_pages + fault_area->pages > fault_area->max_pages)
		new_pages = fault_area->max_pages - fault_area->pages;

	if (new_pages == 0)
		return true;

	*add_pages = new_pages;
	return false;
}

static void hvgr_mmu_trans_fault_no_proc(struct hvgr_ctx *ctx, struct hvgr_mmu_as *mmu_as,
	struct hvgr_mem_area *fault_area)
{
	dev_dbg(ctx->gdev->dev,
		"Ignoring page fault 0x%llx in growable area 0x%llx-0x%llx",
		mmu_as->fault_addr, fault_area->gva,
		(fault_area->gva + (fault_area->pages << PAGE_SHIFT)));

	hvgr_mmu_clear_fault(ctx->gdev, mmu_as);
	(void)hvgr_mmu_hal_unlock(ctx, mmu_as->no);
	hvgr_mmu_unmask_fault(ctx->gdev, mmu_as);
}

static long hvgr_mmu_fault_add_pages(struct hvgr_ctx *ctx, struct hvgr_mmu_as *mmu_as,
	struct hvgr_mem_area *fault_area, uint64_t add_pages)
{
	long ret;
	uint64_t add_gva;
	struct page **pages = NULL;
	struct hvgr_mmu_setup_paras mmu_para = {0};

	mutex_lock(&fault_area->mutex);
	pages = &fault_area->page_array[fault_area->pages];

	ret = hvgr_mem_pool_ctx_alloc(ctx, add_pages, (void **)pages, fault_area->attribute);
	if (ret != 0) {
		mutex_unlock(&fault_area->mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mmu fault alloc 0x%lx pages fail %d",
			ctx->id, add_pages, ret);
		return ret;
	}

	add_gva = fault_area->gva + (fault_area->pages << PAGE_SHIFT);
	hvgr_mem_attr2flag(fault_area);

	mmu_para.gva = add_gva;
	mmu_para.page_array = pages;
	mmu_para.pages = add_pages;
	mmu_para.mmu_flag = fault_area->mmu_flag;
	mmu_para.flags = fault_area->flags;

	ret = hvgr_mmu_set_up(ctx, &mmu_para);
	if (ret != 0) {
		hvgr_mem_pool_ctx_free(ctx, add_pages, (void **)pages, 0,
			fault_area->attribute);
		mutex_unlock(&fault_area->mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mmu fault setup pt 0x%lx pages fail %d",
			ctx->id, add_pages, ret);
		return ret;
	}

	fault_area->pages += add_pages;
	atomic_add((int)add_pages, &ctx->mem_ctx.used_growable_pages);
	atomic_add((int)add_pages, &fault_area->growable_pages);
	mutex_unlock(&fault_area->mutex);

	hvgr_mmu_clear_fault(ctx->gdev, mmu_as);
	hvgr_mmu_hal_flush_pt(ctx, mmu_as->fault_addr, (uint32_t)add_pages);
	hvgr_mmu_unmask_fault(ctx->gdev, mmu_as);
	return 0;
}

void hvgr_mmu_trans_fault_process(struct hvgr_ctx *ctx, struct hvgr_mmu_as *mmu_as)
{
	struct hvgr_mem_area *fault_area = NULL;
	char *reset_reason = NULL;
	uint64_t add_pages = 0;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	fault_area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx,
		mmu_as->fault_addr);
	if (hvgr_mmu_fault_area_illegal(fault_area, &reset_reason))
		goto fault_reset;

	if (hvgr_mmu_fault_no_need_proc(mmu_as, fault_area, &add_pages)) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		hvgr_mmu_trans_fault_no_proc(ctx, mmu_as, fault_area);
		return;
	}

	if (hvgr_mmu_fault_add_pages(ctx, mmu_as, fault_area, add_pages) == 0) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return;
	}

	reset_reason = "Add pages fail";
fault_reset:
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	hvgr_mmu_reset_gpu(ctx, mmu_as, reset_reason);
}

void hvgr_mmu_pf_worker(struct work_struct *work)
{
	struct hvgr_mmu_as *mmu_as = NULL;
	struct hvgr_mem_dev *mem_dev = NULL;
	struct hvgr_device *gdev = NULL;
	struct hvgr_ctx *ctx = NULL;
	char *reset_reason = NULL;

	mmu_as = container_of(work, struct hvgr_mmu_as, pf_work);
	mem_dev = container_of(mmu_as, struct hvgr_mem_dev, mmu_as[mmu_as->no]);
	gdev = container_of(mem_dev, struct hvgr_device, mem_dev);

	ctx = mmu_as->ctx;
	if (ctx == NULL) {
		hvgr_mmu_proc_no_ctx_fault(gdev, mmu_as);
		goto out;
	}

	if (unlikely(mmu_as->protected_mode)) {
		hvgr_mmu_reset_gpu(ctx, mmu_as, "Permission failure");
		hvgr_cq_sched_release_ctx(gdev, ctx);
		goto out;
	}

	if (hvgr_mmu_is_trans_fault(mmu_as->fault_stat)) {
		hvgr_mmu_trans_fault_process(ctx, mmu_as);
	} else {
		reset_reason = hvgr_mmu_other_fault_process(ctx, mmu_as);
		if (reset_reason != NULL)
			hvgr_mmu_reset_gpu(ctx, mmu_as, reset_reason);
	}

	hvgr_cq_sched_release_ctx(gdev, ctx);
out:
	atomic_dec(&gdev->mem_dev.pending_faults);
}

void hvgr_mmu_bf_worker(struct work_struct *work)
{
	struct hvgr_mmu_as *mmu_as = NULL;
	struct hvgr_mem_dev *mem_dev = NULL;
	struct hvgr_device *gdev = NULL;
	struct hvgr_ctx *ctx = NULL;

	mmu_as = container_of(work, struct hvgr_mmu_as, bf_work);
	mem_dev = container_of(mmu_as, struct hvgr_mem_dev, mmu_as[mmu_as->no]);
	gdev = container_of(mem_dev, struct hvgr_device, mem_dev);

	ctx = mmu_as->ctx;
	if (ctx == NULL) {
		hvgr_mmu_proc_no_ctx_fault(gdev, mmu_as);
		goto out;
	}

	if (unlikely(mmu_as->protected_mode)) {
		hvgr_mmu_reset_gpu(ctx, mmu_as, "Permission failure");
		hvgr_cq_sched_release_ctx(gdev, ctx);
		goto out;
	}

	if (hvgr_pm_get_gpu_status(gdev) != HVGR_PM_POWER_OFF) {
		(void)hvgr_mmu_hal_disable(ctx);
		hvgr_mmu_clear_fault(ctx->gdev, mmu_as);
	}

	hvgr_mmu_reset_gpu(ctx, mmu_as, "MMU bus fault");
	hvgr_cq_sched_release_ctx(gdev, ctx);
out:
	atomic_dec(&gdev->mem_dev.pending_faults);
}

void hvgr_mmu_process_fault(struct hvgr_device * const gdev, struct hvgr_mmu_as *mmu_as)
{
	if (mmu_as->ctx == NULL) {
		hvgr_mmu_proc_no_ctx_fault(gdev, mmu_as);
		return;
	}

	if (mmu_as->fault_type == MMU_PAGE_FAULT) {
		WARN_ON(work_pending(&mmu_as->pf_work));
		atomic_inc(&gdev->mem_dev.pending_faults);
		(void)queue_work(mmu_as->fault_wq, &mmu_as->pf_work);
	} else {
		WARN_ON(work_pending(&mmu_as->bf_work));
		atomic_inc(&gdev->mem_dev.pending_faults);
		(void)queue_work(mmu_as->fault_wq, &mmu_as->bf_work);
	}
}

static void hvgr_mmu_int_process(struct hvgr_device * const gdev, uint32_t stat)
{
	uint32_t bus_fault;
	uint32_t page_fault;
	uint32_t irq_mask;
	uint32_t as_no;
	uint32_t val;
	unsigned long flags;
	struct hvgr_mmu_as *mmu_as = NULL;

	hvgr_mmu_get_fault(stat, &page_fault, &bus_fault);

	spin_lock_irqsave(&gdev->mem_dev.mmu_hw_lock, flags);
	irq_mask = hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_MASK));
	hvgr_write_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_MASK), 0);
	spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);

	while (bus_fault | page_fault) {
		as_no = (uint32_t)ffs(bus_fault | page_fault) - 1;
		if (unlikely(as_no >= MMU_ADDR_SPACE_MAX)) {
			hvgr_err(gdev, HVGR_MEM, "As no is invalid, as_no = %u", as_no);
			continue;
		}
		mmu_as = &gdev->mem_dev.mmu_as[as_no];

		mmu_as->ctx = hvgr_cq_lookup_ctx(gdev, (int)as_no);
		mmu_as->protected_mode = false;
		mmu_as->no = as_no;
		mmu_as->fault_type = (bus_fault & (1U << as_no)) ? MMU_BUS_FAULT : MMU_PAGE_FAULT;
		mmu_as->fault_addr = hvgr_read_reg(gdev,
			mmu_control_reg(gdev, mmu_as_reg(as_no, AS_FAULTADDRESS_HI)));
		mmu_as->fault_addr <<= 32;
		mmu_as->fault_addr |= hvgr_read_reg(gdev,
			mmu_control_reg(gdev, mmu_as_reg(as_no, AS_FAULTADDRESS_LO)));
		mmu_as->fault_stat = hvgr_read_reg(gdev,
			mmu_control_reg(gdev, mmu_as_reg(as_no, AS_FAULTSTATUS)));

		if (mmu_as->fault_type == MMU_BUS_FAULT) {
			bus_fault &= ~(1U << as_no);
			irq_mask &= ~(mmu_bus_fault_bit(as_no) | mmu_page_fault_bit(as_no));
		} else {
			page_fault &= ~(1U << as_no);
			irq_mask &= ~(mmu_page_fault_bit(as_no));
		}

		hvgr_mmu_process_fault(gdev, mmu_as);
	}

	spin_lock_irqsave(&gdev->mem_dev.mmu_hw_lock, flags);
	val = hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_MASK));
	val |= irq_mask;
	hvgr_write_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_MASK), val);
	spin_unlock_irqrestore(&gdev->mem_dev.mmu_hw_lock, flags);
}

long hvgr_mmu_fault_wq_init(struct hvgr_device * const gdev)
{
	struct hvgr_mmu_as *mmu_as = NULL;
	uint32_t i;
	char wq_name[32] = {0};

	for (i = 0; i < MMU_ADDR_SPACE_MAX; i++) {
		mmu_as = &gdev->mem_dev.mmu_as[i];
		(void)snprintf_s(wq_name, sizeof(wq_name), sizeof(wq_name) - 1, "hvgr_as%u", i);

		mmu_as->fault_wq = alloc_workqueue(wq_name, WQ_HIGHPRI | WQ_UNBOUND, 1);
		if (mmu_as->fault_wq == NULL) {
			hvgr_err(gdev, HVGR_MEM, "mmu fault alloc wq for as%u fail", i);
			return -EINVAL;
		}

		INIT_WORK(&mmu_as->pf_work, hvgr_mmu_pf_worker);
		INIT_WORK(&mmu_as->bf_work, hvgr_mmu_bf_worker);
	}

	return 0;
}

irqreturn_t hvgr_mmu_irq_handler(int irq, void *data)
{
	struct hvgr_device * const gdev = (struct hvgr_device * const)data;
	uint32_t stat;

	if (hvgr_pm_get_gpu_status(gdev) == HVGR_PM_POWER_OFF)
		return IRQ_NONE;

	stat = hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_STATUS));
	if (stat == 0)
		return IRQ_NONE;

	hvgr_mmu_int_process(gdev, stat);

	return IRQ_HANDLED;
}

long hvgr_mmu_irq_init(struct hvgr_device * const gdev)
{
	if (hvgr_mmu_fault_wq_init(gdev) != 0)
		return -EINVAL;

	return hvgr_register_irq(gdev, MMU_IRQ, hvgr_mmu_irq_handler);
}

void hvgr_flush_mmu_faults(struct hvgr_device * const gdev)
{
	int i;
	struct hvgr_mmu_as *mmu_as = NULL;

	if (atomic_read(&gdev->mem_dev.pending_faults) == 0)
		return;

	for (i = 0; i < MMU_ADDR_SPACE_MAX; i++) {
		mmu_as = &gdev->mem_dev.mmu_as[i];
		flush_workqueue(mmu_as->fault_wq);
	}
}

