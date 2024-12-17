/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <linux/kernel.h>
#include <linux/genalloc.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/lockdep.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <securec.h>

#include "hvgr_kmd_defs.h"
#include "hvgr_mem_opts.h"
#include "hvgr_mmu_api.h"
#include "mem/hvgr_mem_zone.h"
#include "hvgr_mem_gpu.h"
#include "hvgr_memory_page.h"
#include "hvgr_mem_debugfs.h"
#include "hvgr_mem_pool.h"
#include "hvgr_pm_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_asid_api.h"
#include "hvgr_cq.h"
#include "hvgr_cq_queue.h"
#include "hvgr_cq_driver_base.h"
#include "hvgr_regmap.h"
#include "hvgr_softjob.h"
#include "hvgr_debugfs_api.h"
#include "hvgr_cq_data.h"
#include "hvgr_assert.h"
#include "hvgr_cq_driver_adapt.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_platform_api.h"
#include "hvgr_datan_api.h"

#define ALIGN_64        0x40U
#define align(value, align_size) (((value) + ((align_size) - 1)) & ~((align_size) - 1))
#define OFFSET_32BIT    32U
#define OFFSET_16BIT    16U
#define ENTRY_NUM_MAX   4096U
#define OPCODE2_NOTIFY  3U
#define BASE_JCD_SIZE   8U

static bool prot_enhance = false;

struct hvgr_protect_jcd_resource {
	uint32_t protect_start;
	uint32_t protect_size;
};

struct js_info_s {
	uint64_t jc;
	uint32_t subchn;
};

bool hvgr_get_prot_enhance_from_of(struct hvgr_device * const gdev)
{
	uint32_t prot_tmp;

	if (of_property_read_u32(gdev->dev->of_node, "prot_enhance", (uint32_t *)&prot_tmp) != 0) {
		hvgr_debug(gdev, HVGR_CQ, "Can not get prot_enhance.");
		return false;
	}

	if (prot_tmp == 1)
		prot_enhance = true;

	hvgr_info(gdev, HVGR_CQ, "%s prot_enhance = %d.\n", __func__, prot_enhance);

	return true;
}

bool hvgr_get_prot_enhance(struct hvgr_device * const gdev)
{
	return prot_enhance;
}

bool hvgr_mem_protect_jcd_pool_init(struct hvgr_device * const gdev)
{
	struct hvgr_protect_jcd_resource protect_resource;

	if (hvgr_get_prot_enhance_from_of(gdev) == false || prot_enhance == false)
		return true;

	hvgr_info(gdev, HVGR_MEM, "protect jcd pool init enter");

	if (of_property_read_u32_array(gdev->dev->of_node, "protect_space",
		(uint32_t *)&protect_resource, 2) != 0) {
		hvgr_err(gdev, HVGR_MEM, "Get protect_space failed");
		return false;
	}

	hvgr_info(gdev, HVGR_MEM, "protect_start = %lx", protect_resource.protect_start);
	hvgr_info(gdev, HVGR_MEM, "protect_size = %lx", protect_resource.protect_size);

	gdev->mem_dev.protect_pool = gen_pool_create(12, 0);  /* 12 is 4K */
	if (gdev->mem_dev.protect_pool == NULL) {
		hvgr_err(gdev, HVGR_MEM, "protect_pool create failed.");
		return false;
	}

	if (gen_pool_add(gdev->mem_dev.protect_pool,
		protect_resource.protect_start, protect_resource.protect_size, 0) != 0) {
		hvgr_err(gdev, HVGR_MEM, "protect_pool add resource failed.");
		gen_pool_destroy(gdev->mem_dev.protect_pool);
		return false;
	}

#ifdef CONFIG_NEED_CHANGE_MAPPING
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	change_secpage_range(protect_resource.protect_start,
		(unsigned long)page_address(phys_to_page(protect_resource.protect_start)),
		protect_resource.protect_size, __pgprot(PROT_DEVICE_nGnRnE));
	hvgr_err(gdev, HVGR_MEM, "%s mod_to nc.\n", __func__);
#endif
#endif
#ifndef CONFIG_LIBLINUX
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	register_nosave_region_late(protect_resource.protect_start >> PAGE_SHIFT,
		(protect_resource.protect_start + protect_resource.protect_size) >> PAGE_SHIFT);
#endif
#endif
	gdev->mem_dev.protect_start = protect_resource.protect_start;
	gdev->mem_dev.protect_size = protect_resource.protect_size;
	hvgr_info(gdev, HVGR_MEM, "protect jcd pool init exit");
	return true;
}

bool hvgr_mem_protect_jcd_pool_term(struct hvgr_device * const gdev)
{
	if (prot_enhance == false || gdev->mem_dev.protect_pool == NULL)
		return false;

	gen_pool_destroy(gdev->mem_dev.protect_pool);
	gdev->mem_dev.protect_pool = NULL;
	gdev->mem_dev.protect_start = 0;
	gdev->mem_dev.protect_size = 0;

	return true;
}

int hvgr_mem_protect_jcd_init(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mmu_setup_paras mmu_para;
	long ret;
	struct page **pages = NULL;
	uint64_t phy;
	uint32_t i;

	if (prot_enhance == false || ctx == NULL)
		return -EINVAL;

	if (ctx->jcd_area != NULL || ctx->jcd_addr != NULL || ctx->jsx_jcd != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "jcd has been initialized!");
		return -ENOMEM;
	}

	hvgr_info(ctx->gdev, HVGR_MEM, "protect jcd init enter");
	area = kzalloc(sizeof(*area), GFP_KERNEL);
	if (unlikely(area == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "protect jcd init create area fail");
		return -ENOMEM;
	}

	ctx->mem_ctx.zones[0].pmem = &ctx->mem_ctx;
	area->zone = &ctx->mem_ctx.zones[0];
	area->pages = 1;
	area->max_pages = 1;

	phy = gen_pool_alloc(ctx->gdev->mem_dev.protect_pool, PAGE_SIZE);
	if (unlikely(phy == 0)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "no buddy found!");
		goto fail_phy;
	}

	pages = kcalloc(area->max_pages, sizeof(struct page *), GFP_KERNEL);
	if (unlikely(pages == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "protect jcd init kcalloc pages fail");
		goto fail_pages;
	}

	pages[0] = phys_to_page(phy);
	area->gfp = GFP_HIGHUSER;
	area->page_array = pages;
	area->attribute = HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_GPU_WR;

	if (!hvgr_mem_map_kva(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "protect jcd init kmap page fail");
		goto fail_kmap;
	}

	area->uva = hvgr_mem_alloc_va(area, false);
	area->gva = area->uva;
	if (area->uva == 0) {
		goto fail_va;
	}
	hvgr_mem_attr2flag(area);

	mmu_para.gva = area->gva;
	mmu_para.page_array = area->page_array;
	mmu_para.pages = area->pages;
	mmu_para.mmu_flag = area->mmu_flag;
	mmu_para.flags = area->flags;

	ret = hvgr_mmu_set_up(ctx, &mmu_para);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "protect jcd init set up pgd fail");
		goto fail_pgd;
	}

	ctx->jcd_area = area;
	ctx->jcd_addr = u64_to_ptr(area->kva);
	ctx->jsx_jcd = area->gva;
	ctx->gdev->cq_dev.jcd_count++;
	hvgr_info(ctx->gdev, HVGR_MEM, "%s jcd_count=%d.", __func__, ctx->gdev->cq_dev.jcd_count);

	(void)memset_s(ctx->jcd_addr, PAGE_SIZE, 0, PAGE_SIZE);

	for (i = 0; i < BASE_JM_MAX_NR_SLOTS; i++)
		ctx->jcd_offset[i] = i * align((uint32_t)sizeof(struct base_jcd), ALIGN_64);

	hvgr_info(ctx->gdev, HVGR_MEM, "protect jcd init exit.");

	return 0;

fail_pgd:
	(void)hvgr_mem_free_va(area);
fail_va:
	hvgr_mem_unmap_kva(area);
fail_kmap:
	kfree(pages);
fail_pages:
	gen_pool_free(ctx->gdev->mem_dev.protect_pool, phy, PAGE_SIZE);
fail_phy:
	kfree(area);

	return -ENOMEM;
}

void hvgr_mem_protect_jcd_term(struct hvgr_ctx * const ctx)
{
	int i;

	if (prot_enhance == false || ctx == NULL)
		return;

	hvgr_info(ctx->gdev, HVGR_MEM, "protect jcd term enter");
	if (ctx->jcd_area == NULL || ctx->jcd_addr == NULL || ctx->jsx_jcd == 0) {
		hvgr_info(ctx->gdev, HVGR_MEM, "jcd has not been initialized!");
		return;
	}

	(void)hvgr_mmu_take_down(ctx, ctx->jcd_area->gva, (uint32_t)ctx->jcd_area->pages);
	hvgr_mem_unmap_kva(ctx->jcd_area);
	gen_pool_free(ctx->gdev->mem_dev.protect_pool,
		page_to_phys(ctx->jcd_area->page_array[0]), PAGE_SIZE);
	(void)hvgr_mem_free_va(ctx->jcd_area);
	kfree(ctx->jcd_area->page_array);
	kfree(ctx->jcd_area);
	ctx->gdev->cq_dev.jcd_count--;
	hvgr_info(ctx->gdev, HVGR_MEM, "%s jcd_count=%d.", __func__, ctx->gdev->cq_dev.jcd_count);

	ctx->jcd_area = NULL;
	ctx->jcd_addr = NULL;
	ctx->jsx_jcd = 0;
	for (i = 0; i < BASE_JM_MAX_NR_SLOTS; i++)
		ctx->jcd_offset[i] = 0;
	hvgr_info(ctx->gdev, HVGR_MEM, "protect jcd term sucess.");
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static void backup_timer_callback_protect_mode(struct timer_list *timer)
{
#else
static void backup_timer_callback_protect_mode(unsigned long addr)
{
	struct timer_list *timer = (struct timer_list *)(uintptr_t)addr;
#endif
	struct hvgr_cq_ctx *cq_ctx =
		container_of(timer, struct hvgr_cq_ctx, backup_timer_protect_mode);
	unsigned long flags;
	struct hvgr_ctx *ctx = cq_ctx->ctx;
	struct hvgr_device *gdev = NULL;
	struct hvgr_cq_dev *gcqctx = NULL;
	int ret;
	char dmd_msg[MAX_DMD_LOG_LENTH] = {0};

	if (ctx == NULL) {
		pr_err("%s ctx may be has freed.\n", __func__);
		return;
	}

	gdev = ctx->gdev;
	gcqctx = &gdev->cq_dev;

	spin_lock_irqsave(&gcqctx->schedule_lock, flags);

	hvgr_err(gdev, HVGR_CQ, "gpu hang, tgid: %d tsk_name: %s\n",
		ctx->tgid, ctx->process_name);
	hvgr_err(gdev, HVGR_CQ, "%s schedule timeout qid=%u ctx_%u\n",
		__func__, cq_ctx->queue_id, ctx->id);

	hvgr_cq_append_flag_all_queue(gdev, CQ_KMD_CTX_TIME_OUT_HANG, false);

	ret = snprintf_s(dmd_msg, MAX_DMD_LOG_LENTH, MAX_DMD_LOG_LENTH - 1,
		"higpu protect mode job hang,task:%s,%d", hvgr_get_main_process_name(ctx),
		hvgr_get_ctx_compat(ctx));
	if (ret > 0)
		hvgr_datan_gpu_fault_type_set(gdev, DMD_JOB_HANG, dmd_msg);
	else
		hvgr_datan_gpu_fault_type_set(gdev, DMD_JOB_HANG, "higpu protect mode job hang");

	hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_IMMEDIATELY);
	spin_unlock_irqrestore(&gcqctx->schedule_lock, flags);
}

static uint32_t hvgr_cq_get_slot_id(struct hvgr_cq_ctx * const cqctx, uint32_t subchn)
{
	if (subchn == SUB_CHANNEL_TYPE_RENDER) {
		return BASE_JM_SLOT0;
	} else if (subchn == SUB_CHANNEL_TYPE_BIN) {
		return BASE_JM_SLOT2;
	} else if (subchn == SUB_CHANNEL_TYPE_COMPUTE) {
		return BASE_JM_SLOT4;
	} else if (subchn == BASE_JD_REQ_BVHG) {
		return BASE_JM_SLOT6;
	} else if (subchn == BASE_JD_REQ_RTTC) {
		return BASE_JM_SLOT7;
	} else {
		hvgr_err(cqctx->ctx->gdev, HVGR_CQ, "%s subchn=%u not right.", __func__, subchn);
		return BASE_JM_SLOT2;
	}
}

static uint32_t hvgr_cq_get_entry_info(struct hvgr_cq_ctx * const cqctx, struct js_info_s *js_info)
{
	struct cq_cmdq_protect_info *cmd_entry_base =
		(struct cq_cmdq_protect_info *)cqctx->cq_entry_base;
	struct cq_cmdq_protect_info *cmd_entry = NULL;
	uint32_t get_index_hold = cqctx->sw_wr_ctx->queue_get_index;

	if (unlikely(get_index_hold >= ENTRY_NUM_MAX))
		return -1;

	cmd_entry = &cmd_entry_base[get_index_hold];
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s cqctx->sw_wr_ctx->queue_get_index=%u.\n", __func__,
		cqctx->sw_wr_ctx->queue_get_index);
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s get_index_hold=%u.\n", __func__, get_index_hold);

	if ((cmd_entry->head.in_line != 1) || (cmd_entry->opcode1 != 0) ||
		(cmd_entry->opcode2 != OPCODE2_SET_JC_FOR_PROTECT_MODE)) {
		hvgr_err(cqctx->ctx->gdev, HVGR_CQ, "%s inline=%u,opcode1=%u,opcode2=%u, wrong.",
			__func__, cmd_entry->head.in_line, cmd_entry->opcode1, cmd_entry->opcode2);
		return -1;
	}

	js_info->subchn = hvgr_cq_get_slot_id(cqctx, cmd_entry->sub_channel_type);
	js_info->jc = (((uint64_t)cmd_entry->sem_addr_low) |
		((uint64_t)cmd_entry->sem_addr_high << 32));

	return 0;
}

static uint32_t hvgr_cq_get_flush_type_by_js(int js)
{
	uint32_t type = JS_CONFIG_END_FLUSH_ALL;

	if ((js == BIN_LOW_IDX) || (js == BIN_HIGH_IDX))
		type = JS_CONFIG_END_FLUSH_BINNING;
	else if ((js == RENDER_LOW_IDX) || (js == RENDER_HIGH_IDX))
		type = JS_CONFIG_END_FLUSH_RENDERING;
	else if ((js == COMPUTE_LOW_IDX) || (js == COMPUTE_HIGH_IDX))
		type = JS_CONFIG_END_FLUSH_COMPUTE;
	else if (js == RT_IDX)
		type = JS_CONFIG_END_FLUSH_RAYTRACING;
	else if (js == BVHG_IDX)
		type = JS_CONFIG_END_FLUSH_NO_OPERATION;

	return type;
}

static uint32_t hvgr_cq_hw_submit_configure_mmu(struct hvgr_device *gdev,
	struct hvgr_ctx *ctx, int js, u32 jc_index)
{
	uint32_t cfg;

	if (unlikely(js >= BASE_JM_MAX_NR_SLOTS)) {
		hvgr_err(gdev, HVGR_CQ, "%s js is overstep( > 8)\n", __func__);
		return -ENOMEM;
	}

	cfg = ctx->asid;

	cfg |= JS_N_CONFIG_END_FLUSH_L1_CLEAN_INVALIDATE;
	cfg |= JS_N_CONFIG_END_FLUSH_L2_CLEAN_INVALIDATE;

	cfg |= hvgr_cq_get_flush_type_by_js(js);

	/* Set barrier on any jobs to get round of FPGA problems */
	cfg |= JS_N_CONFIG_BARRIER;

	hvgr_write_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_CONFIG), cfg);

	return cfg;
}

static void hvgr_cq_js_hw_submit(struct hvgr_device *gdev, struct hvgr_cq_ctx * const cqctx,
	struct js_info_s *js_info)
{
	struct hvgr_ctx *ctx = NULL;
	uint32_t cfg;
	uint32_t jc_index;
	uint64_t jc_head;
	uint32_t js = js_info->subchn;
	uint64_t affinity;
	size_t size;

	jc_head = js_info->jc;
	ctx = cqctx->ctx;
	/* 4 job chain per slot, we use 1th chain in slot. */
	jc_index = js << 2;

	affinity = hvgr_pm_get_availiable_cores(gdev);
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s affinity=0x%x.\n", __func__, affinity);
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s subchn=%u. js_info->jc=%llx, jc_y_addr=0x%x\n",
		__func__, js_info->subchn, js_info->jc,
		job_jc_y_reg(gdev, jc_index, JSX_JCY_HEAD_LO));

	hvgr_write_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_HEAD_LO),
		jc_head & (u64)0xFFFFFFFFU);

	hvgr_write_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_HEAD_HI),
		jc_head >> OFFSET_32BIT);

	hvgr_write_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_AFFINITY),
		affinity & (u64)0xFFFFFFFFU);

	hvgr_write_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_JCD_LO),
		(ctx->jsx_jcd + ctx->jcd_offset[js]) & (u64)0xFFFFFFFFU);

	hvgr_write_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_JCD_HI),
		(ctx->jsx_jcd + ctx->jcd_offset[js]) >> OFFSET_32BIT);

	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s JCD_LO=0x%x.\n", __func__,
		hvgr_read_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_JCD_LO)));
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s JCD_HI=0x%x.\n", __func__,
		hvgr_read_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_JCD_HI)));
	/* 32bit or 64bit */
	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		hvgr_write_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_JD_MSB), 0);
	} else {
		if (ctx->same_va_4g_base_page != 0) {
			hvgr_write_reg(gdev,
				job_jc_y_reg(gdev, jc_index, JSX_JCY_JD_MSB),
				ctx->same_va_4g_base_page);
			hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s same_va_4g_base_page=0x%x.\n",
				__func__, ctx->same_va_4g_base_page);
		}
	}
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s JSX_JCY_JD_MSB addr =0x%x.\n", __func__,
		job_jc_y_reg(gdev, jc_index, JSX_JCY_JD_MSB));

	cfg = hvgr_cq_hw_submit_configure_mmu(gdev, ctx, (int)js, jc_index);
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s cfg=0x%x.\n", __func__, cfg);
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ,
		"%s subchn=%u. js_info->jc=%llx, command=0x%x has submit.\n", __func__,
		js_info->subchn, js_info->jc, job_jc_y_reg(gdev, jc_index, JSX_JCY_COMMAND));

	/* memset jcd 0 */
	size = align(sizeof(struct base_jcd), ALIGN_64);
	memset_s(ctx->jcd_addr + ctx->jcd_offset[js], size, 0, size);
	hvgr_info(gdev, HVGR_CQ, "%s memset jcd end\n", __func__);

	/* sumbit js ot GPU */
	hvgr_write_reg(gdev, job_jc_y_reg(gdev, jc_index, JSX_JCY_COMMAND), JS_COMMAND_START);

	/* 5s timer to reset gpu and out TZPC */
	kmd_timer_setup(&cqctx->backup_timer_protect_mode, backup_timer_callback_protect_mode);
	(void)mod_timer(&cqctx->backup_timer_protect_mode,
		jiffies + (uint32_t)gdev->cq_dev.reset_timeout_normal * HZ);
}

static void hvgr_cq_schedule_js_in(struct hvgr_cq_ctx * const cqctx)
{
	struct hvgr_ctx *ctx = cqctx->ctx;
	struct js_info_s js_info;
	struct hvgr_cq_dev * const gcqctx = &cqctx->ctx->gdev->cq_dev;

	/* backup cqctx to channel_info */
	gcqctx->channel_info[0].running_queue = cqctx;
	gcqctx->channel_info[0].running_queue->channel_id = 0;
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s change GPU status\n", __func__);
	gcqctx->gpu_sch_sts = HVGR_GPU_RUNNING;
	cqctx->status = CQ_STATUS_RUNNING;

	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s start get entry info\n", __func__);
	if (ctx->jcd_area == NULL || hvgr_cq_get_entry_info(cqctx, &js_info) != 0 ||
		!hvgr_ctx_sched_in(ctx)) {
		hvgr_err(cqctx->ctx->gdev, HVGR_CQ,
			"%s get entry info failure and will reset gpu immediately,as=%d\n",
			__func__, ctx->asid);
		hvgr_pm_gpu_reset(cqctx->ctx->gdev, GPU_RESET_TYPE_EXIT_PROTECT_MODE);
		return;
	}

	ctx->gdev->cq_dev.jcd_ctx[js_info.subchn] = ctx;

	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s start cq js hw submit\n", __func__);
	hvgr_cq_js_hw_submit(ctx->gdev, cqctx, &js_info);
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s submit job chain to GPU sucessfully.\n", __func__);
}

void hvgr_protect_mode_disable_interrupts(struct hvgr_device *gdev)
{
	int i;
	for (i = 0; i < BASE_JM_MAX_NR_SLOTS; i++) {
		hvgr_write_reg(gdev, job_slot_x_reg(gdev, (uint32_t)i, JSX_IRQ_CLEAR), 0xFFFFFFFFU);
		hvgr_write_reg(gdev, job_slot_x_reg(gdev, (uint32_t)i, JSX_IRQ_MASK), 0x0);
	}
	hvgr_info(gdev, HVGR_CQ, "%s disable job interrupts sucessfully.\n", __func__);
}

static void hvgr_protect_mode_enable_interrupts(struct hvgr_device *gdev)
{
	int i;
	for (i = 0; i < BASE_JM_MAX_NR_SLOTS; i++) {
		hvgr_write_reg(gdev, job_slot_x_reg(gdev, (uint32_t)i, JSX_IRQ_CLEAR), 0xFFFFFFFFU);
		hvgr_write_reg(gdev, job_slot_x_reg(gdev, (uint32_t)i, JSX_IRQ_MASK), 0xFFFFFFFFU);
	}
	hvgr_info(gdev, HVGR_CQ, "%s enable job interrupts sucessfully.\n", __func__);
}

void hvgr_cq_protect_mode_enter(struct hvgr_cq_ctx * const cq_ctx, struct hvgr_device *gdev,
	uint32_t channel_id)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	uint32_t value;
	uint32_t try_times;
	bool enqueue_flag = true;
	unsigned long flags;
	uint32_t i;

	switch (gcqctx->protect_mode_info.protected_state.enter) {
	case HVGR_CQ_ENTER_PROTECTED_CHECK:
		/* close submit new job. */
		hvgr_info(gdev, HVGR_CQ, "%s close submit.", __func__);
		hvgr_sch_switch_off_request(gdev);
		/* record running queue. */
		gcqctx->protect_mode_info.running_queue = cq_ctx;
		hvgr_cq_unbind_all_channel(gdev);
		gcqctx->protect_mode_info.protected_state.enter = HVGR_CQ_ENTER_PROTECTED_WAIT_IDLE;
		break;

	case HVGR_CQ_ENTER_PROTECTED_WAIT_IDLE:
		/* fall-through */
		enqueue_flag = (cq_ctx != gcqctx->protect_mode_info.running_queue);
		for (i = 0; i < ARRAY_SIZE(gcqctx->protect_mode_info.bak_running_queue); i++) {
			if (gcqctx->protect_mode_info.bak_running_queue[i] != NULL) {
				enqueue_flag = (enqueue_flag &&
					(cq_ctx != gcqctx->protect_mode_info.bak_running_queue[i]));
				hvgr_info(gdev, HVGR_CQ, "%s bakrq_id=%u\n", __func__,
					gcqctx->protect_mode_info.bak_running_queue[i]->queue_id);
			}
		}
		hvgr_cq_schedule_cq_out(cq_ctx, enqueue_flag);

		hvgr_info(gdev, HVGR_CQ, "%s qid=%u,rqid=%u. ef=%d. gs=%d\n",
			__func__, cq_ctx->queue_id,
			gcqctx->protect_mode_info.running_queue->queue_id, enqueue_flag,
			gcqctx->gpu_sch_sts);

		if (gcqctx->gpu_sch_sts == HVGR_GPU_IDLE)
			gcqctx->protect_mode_info.protected_state.enter =
				HVGR_CQ_ENTER_PROTECTED_CONFIG;
		else
			break;
	/* fall through */
	case HVGR_CQ_ENTER_PROTECTED_CONFIG:
		/* 1. flush cache */
		hvgr_mmu_flush_l2_caches(gdev);
		if (prot_enhance == true) {
			spin_lock_irqsave(&gcqctx->protect_mode_switch, flags);
			/* 2. switch to job slot mode */
			hvgr_info(gdev, HVGR_CQ, "%s switch to job slot mode.\n", __func__);
			hvgr_write_reg(gdev, cq_control_reg(gdev, CQ_ENABLE), 0);
			hvgr_protect_mode_enable_interrupts(gdev);
			/* 3. start TZPC */
			(void)hvgr_platform_protected_cfg(gdev, 0);
		}
		/* 4. Switch GPU to protected mode */
		hvgr_write_reg(gdev, GPU_PROT_MODE, GPU_COMMAND_SET_PROTECTED_MODE);

		hvgr_info(gdev, HVGR_CQ, "%s start wait protect mode.\n", __func__);
		try_times = HVGR_CQ_PROTECT_TRY_MAX_TIMES;
		while (try_times > 0) {
			value = hvgr_read_reg(gdev, GPU_STATUS);
			if (value & HVGR_CQ_PROTECT_MODE_ACTIVE)
				break;
			udelay(1);
			try_times--;
		}
		hvgr_info(gdev, HVGR_CQ, "%s protect mode =%d.\n", __func__, value);
		gcqctx->protect_mode_info.protected_mode = true;
		gcqctx->protect_mode_info.protected_mode_transition = false;
		if (prot_enhance == true) {
			spin_unlock_irqrestore(&gcqctx->protect_mode_switch, flags);
			hvgr_cq_schedule_js_in(gcqctx->protect_mode_info.running_queue);
		} else {
			hvgr_cq_schedule_cq_in(gcqctx->protect_mode_info.running_queue, channel_id);
		}
		gcqctx->protect_mode_info.protected_state.enter = HVGR_CQ_ENTER_PROTECTED_FINISHED;
	/* fall through */
	case HVGR_CQ_ENTER_PROTECTED_FINISHED:
		break;
	}
}

static uint32_t hvgr_cq_protect_exit_multi_user(struct hvgr_device *gdev, uint32_t channel_id)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	int i;

	if (gcqctx->protect_mode_info.protected_mode_refcount == 0) {
		hvgr_info(gdev, HVGR_CQ, "%s refcount=%d. bak_rq not possible.\n", __func__,
			gcqctx->protect_mode_info.protected_mode_refcount);
		return 1;
	}

	gcqctx->protect_mode_info.protected_mode_transition = false;
	/* find bak running queue from back to front */
	for (i = (int)ARRAY_SIZE(gcqctx->protect_mode_info.bak_running_queue) - 1; i >= 0; i--)
		if (gcqctx->protect_mode_info.bak_running_queue[i] != NULL) {
			gcqctx->protect_mode_info.running_queue =
				gcqctx->protect_mode_info.bak_running_queue[i];
			gcqctx->protect_mode_info.bak_running_queue[i] = NULL;
			break;
		}

	if (prot_enhance)
		hvgr_cq_schedule_js_in(gcqctx->protect_mode_info.running_queue);
	else
		hvgr_cq_schedule_cq_in(gcqctx->protect_mode_info.running_queue, channel_id);

	hvgr_info(gdev, HVGR_CQ, "%s gs=%d, submit another queue.\n",
		__func__, gcqctx->gpu_sch_sts);
	return 0;
}

static void hvgr_cq_protect_mode_exit(struct hvgr_cq_ctx * const cq_ctx, struct hvgr_device *gdev,
	uint32_t channel_id)
{
	uint32_t i;
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;

	hvgr_info(gdev, HVGR_CQ, "%s enter protect mode exit \n", __func__);

	switch (gcqctx->protect_mode_info.protected_state.exit) {
	case HVGR_CQ_EXIT_PROTECTED_CHECK:
		if (prot_enhance == false)
			hvgr_cq_unbind_all_channel(gdev);

		gcqctx->protect_mode_info.protected_state.exit = HVGR_CQ_EXIT_PROTECTED_WAIT_IDLE;
		if (prot_enhance == false)
			break;
	/* fall-through */
	case HVGR_CQ_EXIT_PROTECTED_WAIT_IDLE:
		if (prot_enhance == false) {
			hvgr_cq_schedule_cq_out(cq_ctx, true);
			hvgr_info(gdev, HVGR_CQ, "%s cq_id=%u, gs=%d\n",
				__func__, cq_ctx->queue_id, gcqctx->gpu_sch_sts);
		} else {
			if (hvgr_cq_is_need_into_pending(true, cq_ctx))
				(void)hvgr_cq_schedule_enqueue(gcqctx, cq_ctx);

			gcqctx->gpu_sch_sts = HVGR_GPU_IDLE;
		}

		gcqctx->protect_mode_info.running_queue = NULL;

		for (i = 0; i < ARRAY_SIZE(gcqctx->protect_mode_info.bak_running_queue); i++) {
			if (gcqctx->protect_mode_info.bak_running_queue[i] != NULL)
				hvgr_info(gdev, HVGR_CQ, "%s bakrq_id=%u\n", __func__,
					gcqctx->protect_mode_info.bak_running_queue[i]->queue_id);
		}

		if (gcqctx->gpu_sch_sts != HVGR_GPU_IDLE)
			break;

		if (hvgr_cq_protect_exit_multi_user(gdev, channel_id) == 0)
			break;

		gcqctx->protect_mode_info.protected_state.exit = HVGR_CQ_EXIT_PROTECTED_RESET;
	/* fall-through */
	case HVGR_CQ_EXIT_PROTECTED_RESET:
		hvgr_info(gdev, HVGR_CQ, "%s  start reset gpu for exit protect mode.\n",
			__func__, gcqctx->gpu_sch_sts);

		/* reset GPU */
		hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_EXIT_PROTECT_MODE);
		break;
	case HVGR_CQ_EXIT_PROTECTED_RESET_WAIT:
		break;
	}
}

static void hvgr_cq_signal_hold_en_out_token(struct hvgr_cq_ctx * const cqctx)
{
	struct cq_cmdq_inline_notify_protect *cmd_entry_base =
		(struct cq_cmdq_inline_notify_protect *)cqctx->cq_entry_base;
	struct cq_cmdq_inline_notify_protect *cmd_entry = NULL;
	uint32_t get_index_hold = (cqctx->sw_wr_ctx->queue_get_index + 2) % ENTRY_NUM_MAX;

	if (unlikely(cmd_entry_base == NULL))
		return;

	cmd_entry = &cmd_entry_base[get_index_hold];
	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s get_index_hold=%u.\n", __func__, get_index_hold);
	if ((cmd_entry->head.in_line != 1) || (cmd_entry->opcode1 != 0) ||
		(cmd_entry->opcode2 != OPCODE2_NOTIFY) || (cmd_entry->hold_en != 1)) {
		hvgr_err(cqctx->ctx->gdev, HVGR_CQ,
			"%s inline=%u, op1=%u, op2=%u,hold=%u,not right,need hold_en.\n",
			__func__, cmd_entry->head.in_line, cmd_entry->opcode1,
			cmd_entry->opcode2, cmd_entry->hold_en);
		return;
	}

	hvgr_info(cqctx->ctx->gdev, HVGR_CQ, "%s token=%d\n", __func__, cmd_entry->token);
	hvgr_token_finish(cqctx, cmd_entry->token);
}

static void kmd_gpu_job_done(struct hvgr_device *gdev, int js)
{
	u32 jc_done;
	u32 jc_stop;
	u32 js_irq_status;
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	struct hvgr_ctx *ctx = NULL;
	unsigned long flags;
	struct hvgr_cq_ctx *cqctx = NULL;

	spin_lock_irqsave(&gcqctx->schedule_lock, flags);

	js_irq_status = hvgr_read_reg(gdev, job_slot_x_reg(gdev, (unsigned int)js, JSX_IRQ_STATUS));
	jc_done = js_irq_status & 0xF;
	jc_stop = (js_irq_status >> OFFSET_16BIT) & 0xF;

	hvgr_write_reg(gdev, job_slot_x_reg(gdev, (unsigned int)js, JSX_IRQ_CLEAR), js_irq_status);
	hvgr_info(gdev, HVGR_CQ, "jc_stop:0x%x, jc_done=0x%x\n", jc_stop, jc_done);

	if (gcqctx->protect_mode_info.running_queue == NULL || jc_stop != 0) {
		spin_unlock_irqrestore(&gcqctx->schedule_lock, flags);
		return;
	}

	/* del timer for 5s GPU hang */
	del_timer(&gcqctx->protect_mode_info.running_queue->backup_timer_protect_mode);

	if (jc_done != 0) {
		hvgr_info(gdev, HVGR_CQ, "%s will exit protect mode.\n", __func__,
			gcqctx->protect_mode_info.protected_mode_refcount);

		gcqctx->protect_mode_info.protected_mode_refcount--;

		hvgr_info(gdev, HVGR_CQ, "%s out protected_mode_refcount=%d.\n",
			__func__, gcqctx->protect_mode_info.protected_mode_refcount);

		ctx = gdev->cq_dev.jcd_ctx[js];
		gcqctx->protect_mode_info.protected_mode_transition = true;
		gcqctx->protect_mode_info.protected_state.exit = HVGR_CQ_EXIT_PROTECTED_CHECK;
		cqctx = gcqctx->protect_mode_info.running_queue;

		hvgr_cq_signal_hold_en_out_token(cqctx);
		hvgr_info(gdev, HVGR_CQ, "%s update ctx_%u, qid=%d. getindex=%d\n",
			__func__, ctx->id, cqctx->queue_id, cqctx->sw_wr_ctx->queue_get_index + 3);

		cqctx->sw_wr_ctx->queue_get_index = (cqctx->sw_wr_ctx->queue_get_index + 3) % ENTRY_NUM_MAX;

		hvgr_ctx_sched_out(ctx);
		/* normal exit protect mode should set channel_info NULL */
		gcqctx->channel_info[0].running_queue->channel_id = CQ_CHANNEL_ID_INVALID;
		gcqctx->channel_info[0].running_queue = NULL;
		cqctx->status = CQ_STATUS_IDLE;
		hvgr_info(gdev, HVGR_CQ, "%s will call hvgr_cq_protect_mode_exit.\n", __func__);
		hvgr_cq_protect_mode_exit(cqctx, gdev, 0);
	}

	spin_unlock_irqrestore(&gcqctx->schedule_lock, flags);
	return;
}

irqreturn_t hvgr_job_irq_handler(int irq, void *data)
{
	u32 done;
	int i;
	int count = 0;
	struct hvgr_device * const gdev = (struct hvgr_device * const)data;

	done = hvgr_read_reg(gdev, job_control_reg(gdev, JOB_IRQ_STATUS));

	/* In protect mode, only has one job done. */
	while (done) {
		u32 finished = done & KMD_JOB_SLOT_MASK;
		i = ffs((int)finished) - 1;
		hvgr_info(gdev, HVGR_CQ, "done:0x%x, i=0x%x\n", done, i);
		/* job hang detected */
		if (unlikely(i < 0)) {
			hvgr_err(gdev, HVGR_CQ, "Job hang with status:%u", done);
			break;
		}

		do {
			kmd_gpu_job_done(gdev, i);
			count++;
			hvgr_info(gdev, HVGR_CQ, "%s here can only do 1 times. cnt=%d, i=%d",
				__func__, count, i);
			done = hvgr_read_reg(gdev, job_control_reg(gdev, JOB_IRQ_STATUS));
			finished = done & KMD_JOB_SLOT_MASK;
		} while (finished & ((u32)1 << i));
	}
	return IRQ_HANDLED;
}

void *hvgr_cq_vmap_user_addr(struct hvgr_ctx * const ctx, uint64_t gpu_addr)
{
	struct hvgr_mem_area *area = NULL;
	pgprot_t prot = PAGE_KERNEL;

	area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, gpu_addr);
	if (area == NULL)
		return NULL;

	/* cpu no cacheable */
	prot = pgprot_writecombine(prot);
	return vmap(area->page_array, (uint32_t)area->pages, VM_MAP, prot);
}

void hvgr_cq_protect_mode_check_running(struct hvgr_cq_ctx * const cq_ctx,
	struct hvgr_device *gdev, uint32_t channel_id, enum protect_flag flag)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	uint32_t index;

	if ((flag != CQ_PROTECT_MODE_IN) && (flag != CQ_PROTECT_MODE_OUT))
		return;

	if (unlikely(!gcqctx->protect_mode_info.protected_mode && (flag == CQ_PROTECT_MODE_OUT))) {
		hvgr_err(gdev, HVGR_CQ, "%s invalid hold_en out.\n", __func__);
		return;
	}

	gcqctx->protect_mode_info.protected_mode_transition = true;
	/* will enter or exit protect mode. */
	if (flag == CQ_PROTECT_MODE_IN) {
		gcqctx->protect_mode_info.protected_mode_refcount++;
		hvgr_info(gdev, HVGR_CQ, "%s in protected_mode_refcount=%d.\n",
			__func__, gcqctx->protect_mode_info.protected_mode_refcount);
		if (gcqctx->protect_mode_info.protected_mode_refcount == 1) {
			hvgr_assert(!gcqctx->protect_mode_info.protected_mode);
			gcqctx->protect_mode_info.protected_state.enter =
				HVGR_CQ_ENTER_PROTECTED_CHECK;
			hvgr_cq_protect_mode_enter(cq_ctx, gdev, channel_id);
		} else if (gcqctx->protect_mode_info.protected_mode_refcount >= 2 &&
			gcqctx->protect_mode_info.protected_mode_refcount <= CQ_CHANNEL_NUM) {
			/* intercept invaild hold_en. */
			if (unlikely(cq_ctx == gcqctx->protect_mode_info.running_queue)) {
				hvgr_err(gdev, HVGR_CQ, "%s invalid hold_en in.\n", __func__);
				hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_EXIT_PROTECT_MODE);
				return;
			}
			/* 2 to 4 queues receive hold_en at the same time, we need record them. */
			index = gcqctx->protect_mode_info.protected_mode_refcount - 2;
			gcqctx->protect_mode_info.bak_running_queue[index] = cq_ctx;
			hvgr_info(gdev, HVGR_CQ, "%s bakq index=%d stored.", __func__, index);
		} else {
			hvgr_err(gdev, HVGR_CQ, "%s in protected_mode_refcount=%d, not right.\n",
				__func__, gcqctx->protect_mode_info.protected_mode_refcount);
			hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_EXIT_PROTECT_MODE);
			return;
		}
	} else {
		gcqctx->protect_mode_info.protected_mode_refcount--;
		hvgr_info(gdev, HVGR_CQ, "%s out protected_mode_refcount=%d.\n",
			__func__, gcqctx->protect_mode_info.protected_mode_refcount);
		gcqctx->protect_mode_info.protected_state.exit = HVGR_CQ_EXIT_PROTECTED_CHECK;
		hvgr_cq_protect_mode_exit(cq_ctx, gdev, channel_id);
	}
}

void hvgr_cq_protect_mode_check_stopped(struct hvgr_cq_ctx * const cq_ctx,
	struct hvgr_device *gdev, uint32_t channel_id)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;

	if (!gcqctx->protect_mode_info.protected_mode)
		hvgr_cq_protect_mode_enter(cq_ctx, gdev, channel_id);
	else
		hvgr_cq_protect_mode_exit(cq_ctx, gdev, channel_id);
}

void hvgr_cq_schedule_channel_running_proc(struct hvgr_device * const gdev,
	struct hvgr_cq_ctx * const cq_ctx, uint32_t channel_id, enum protect_flag flag,
	bool need_stop)
{
	bool empty_flag = hvgr_cq_get_empty_flag(gdev, channel_id);
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;

	if ((flag == CQ_PROTECT_MODE_IN) || (flag == CQ_PROTECT_MODE_OUT)) {
		hvgr_cq_protect_mode_check_running(cq_ctx, gdev, channel_id, flag);
		return;
	}

	if (gcqctx->protect_mode_info.protected_mode ||
		gcqctx->protect_mode_info.protected_mode_transition)
		return;
	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ,
		"%s ctx_%u chn=%d, stop=%d getidx=%d,put=%d,empty=%d.\n", __func__,
		cq_ctx->ctx->id, channel_id, need_stop, cq_ctx->sw_wr_ctx->queue_get_index,
		cq_ctx->sw_wr_ctx->queue_put_index, empty_flag);
	if (hvgr_cq_is_need_unbind(empty_flag, cq_ctx) || need_stop)
		hvgr_cq_unbind_channel(cq_ctx, channel_id, CQ_NEED_UNBIND_TIMER_FLAG);
}
