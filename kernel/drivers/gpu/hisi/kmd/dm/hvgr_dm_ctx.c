/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_dm_ctx.h"

#include <linux/thread_info.h>
#include <securec.h>
#include <linux/kref.h>
#ifdef CONFIG_DFX_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include <linux/sched/task.h>
#include "hvgr_version.h"
#include "hvgr_mem_api.h"
#include "hvgr_mmu_api.h"

#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_sch_api.h"
#include "hvgr_datan_api.h"
#include "hvgr_protect_mode.h"
#include "hvgr_platform_api.h"
#include "hvgr_defs.h"
#include "hvgr_regmap.h"

#if hvgr_version_ge(HVGR_V350)
#define FCP_LOADER_CTX_ID		1
#else
#define FCP_LOADER_CTX_ID		0
#endif

#ifdef CONFIG_DFX_DEBUG_FS
#define CTX_NAME_MAX_LEN 64
static void hvgr_context_debugfs_init(struct hvgr_device * const gdev, struct hvgr_ctx *ctx)
{
	char ctx_name[CTX_NAME_MAX_LEN];

	(void)snprintf_s(ctx_name, CTX_NAME_MAX_LEN, CTX_NAME_MAX_LEN - 1,
		"%u_%d_%d", ctx->id, ctx->tgid, ctx->pid);
	ctx->debugfs_dir = debugfs_create_dir(ctx_name, gdev->ctx_debugfs_dir);
	if (IS_ERR_OR_NULL(ctx->debugfs_dir))
		hvgr_info(gdev, HVGR_DM, "ctx %d_%d init debugfs fail", ctx->tgid, ctx->pid);
}

static void hvgr_context_debugfs_term(struct hvgr_ctx *ctx)
{
	if (!IS_ERR_OR_NULL(ctx->debugfs_dir))
		debugfs_remove_recursive(ctx->debugfs_dir);
}
#endif

#if defined(CONFIG_HVGR_DFX_SH) && defined(CONFIG_HVGR_VIRTUAL_GUEST)
static void hvgr_context_update_core_mask(struct hvgr_device *gdev)
{
	uint32_t core_mask_nv;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	if (atomic_read(&pm_pwr->core_mask_rd_flag) != 0)
		return;

	atomic_add(1, &pm_pwr->core_mask_rd_flag);
	core_mask_nv = hvgr_dmd_get_core_mask_info(gdev);
	atomic_set(&pm_pwr->core_mask_info, core_mask_nv);
	hvgr_debug(gdev, HVGR_DM, "core_mask_nv = 0x%x.", core_mask_nv);
	if ((pm_pwr->present_cores[HVGR_PM_CORE_GPC] != core_mask_nv) &&
		(core_mask_nv != 0))
		pm_pwr->core_mask_cur = pm_pwr->core_mask_desire =
			(pm_pwr->present_cores[HVGR_PM_CORE_GPC] & ~core_mask_nv);
}
#endif

struct hvgr_ctx *hvgr_open_context(struct hvgr_device * const gdev, struct file *kfile)
{
	struct hvgr_ctx *ctx = kvzalloc(sizeof(*ctx), GFP_KERNEL);

	if (ctx == NULL) {
		hvgr_err(gdev, HVGR_DM, "%s: alloc ctx failed", __func__);
		return NULL;
	}
	ctx->kfile = kfile;
	ctx->gdev = gdev;

	ctx->tgid = current->tgid;
	ctx->pid = current->pid;
	get_task_comm(ctx->process_name, current);
	ctx->leader = current->group_leader;
	if (likely(ctx->leader != NULL))
		get_task_comm(ctx->group_name, ctx->leader);
	get_task_struct(ctx->leader);
	ctx->asid = HVGR_AS_NR_INVALID;

	mutex_init(&ctx->softq_lock);
	mutex_init(&ctx->ctx_lock);
	init_waitqueue_head(&ctx->softq_wait);

	ctx->softq_unused = HVGR_MAX_SOFTQ_BITMAP;
	kref_init(&ctx->ctx_kref);

	ctx->id = (u32)atomic_inc_return(&(gdev->ctx_num)) - 1;
	hvgr_info(gdev, HVGR_DM, "ctx_%u pid(%d) ctx_num(%d) %s.",
		ctx->id, ctx->pid, atomic_read(&(gdev->ctx_num)), ctx->process_name);

	if (is_compat_task())
		hvgr_ctx_flag_set(ctx, HVGR_CTX_FLAG_COMPAT);

	if (hvgr_mem_init_proc(ctx) != 0) {
		hvgr_err(gdev, HVGR_DM, "%s: mem init failed", __func__);
		goto mem_init_fail;
	}

	if (hvgr_sch_ctx_create(ctx) != 0) {
		hvgr_err(gdev, HVGR_DM, "%s: cq ctx init failed", __func__);
		goto cq_ctx_init_fail;
	}

	if (ctx->id == FCP_LOADER_CTX_ID) {
		hvgr_info(gdev, HVGR_DM, "%s hvgr_fcp_loader_entry", __func__);
		hvgr_fcp_loader_entry(gdev);
#ifdef CONFIG_HVGR_DFX_SH
		atomic_set(&gdev->dm_dev.dmd_msg->dmd_gaf, (int)hvgr_dmd_read_gaf(gdev));
		hvgr_debug(gdev, HVGR_DM, "%s nvram gaf = 0x%x",
			__func__, atomic_read(&gdev->dm_dev.dmd_msg->dmd_gaf));
#if defined(CONFIG_HVGR_VIRTUAL_GUEST)
		hvgr_context_update_core_mask(gdev);
#endif
#endif
	}

#ifdef CONFIG_DFX_DEBUG_FS
	hvgr_context_debugfs_init(gdev, ctx);
#endif
	mutex_lock(&gdev->dm_dev.ctx_list_mutex);
	list_add(&ctx->dev_link, &gdev->dm_dev.ctx_list);
	mutex_unlock(&gdev->dm_dev.ctx_list_mutex);

	(void)hvgr_datan_init_proc(ctx);
#ifdef CONFIG_HVGR_SAMEVA_CACHE
	hvgr_mem_enable_cache(ctx);
#endif
	return ctx;

cq_ctx_init_fail:
	hvgr_mem_term_proc(ctx);
mem_init_fail:
	atomic_dec(&(gdev->ctx_num));
	put_task_struct(ctx->leader);
	kvfree(ctx);
	return NULL;
}

void hvgr_close_context(struct hvgr_ctx * const ctx)
{
	struct hvgr_ctx *tmp = NULL;
	struct hvgr_ctx *ctx_tmp = NULL;

	mutex_lock(&ctx->gdev->dm_dev.ctx_list_mutex);
	list_for_each_entry_safe(ctx_tmp, tmp, &ctx->gdev->dm_dev.ctx_list, dev_link) {
		if (ctx_tmp == ctx) {
			list_del(&ctx_tmp->dev_link);
			break;
		}
	}
	mutex_unlock(&ctx->gdev->dm_dev.ctx_list_mutex);
#ifdef CONFIG_HVGR_SAMEVA_CACHE
	hvgr_mem_disable_cache(ctx);
#endif
	hvgr_datan_term_proc(ctx);
#ifdef CONFIG_DFX_DEBUG_FS
	hvgr_context_debugfs_term(ctx);
#endif
	hvgr_sch_ctx_destroy(ctx);

	hvgr_mem_protect_jcd_term(ctx);

	hvgr_mem_term_proc(ctx);
	put_task_struct(ctx->leader);
	kvfree(ctx);
}

#if hvgr_version_ge(HVGR_V350)
int hvgr_ctx_common_ctx_init(struct hvgr_device * const gdev)
{
	struct hvgr_ctx *ctx = NULL;
	int ret;

	hvgr_info(gdev, HVGR_DM, "%s enter", __func__);
	ctx = vmalloc(sizeof(*ctx));
	if (ctx == NULL) {
		hvgr_err(gdev, HVGR_DM, "common ctx vmalloc fail");
		return -ENOMEM;
	}

	(void)memset_s(ctx, sizeof(*ctx), 0, sizeof(*ctx));
	ctx->asid = COMMON_USE_SSID_VAL;
	ctx->ssid = COMMON_USE_SSID_VAL;
	ctx->gdev = gdev;
	ret = hvgr_mem_init_proc(ctx);
	if (ret != 0) {
		vfree(ctx);
		hvgr_err(gdev, HVGR_DM, "common ctx init fail");
		return -ENOMEM;
	}

	atomic_inc(&(gdev->ctx_num));
	gdev->com_ctx.ctx = ctx;
	ctx->mem_ctx.zones[0].pmem = &ctx->mem_ctx;

	return 0;
}

void hvgr_ctx_common_ctx_term(struct hvgr_device * const gdev)
{
	struct hvgr_ctx *ctx = gdev->com_ctx.ctx;

	if (ctx == NULL)
		return;

	hvgr_info(gdev, HVGR_DM, "%s enter", __func__);
	hvgr_mem_term_proc(ctx);
	vfree(ctx);
	gdev->com_ctx.ctx = NULL;

	return;
}

static int hvgr_ctx_config_common_ssid(struct hvgr_device *gdev)
{
	u32 value;
	u32 time = 1000;

	hvgr_write_reg(gdev, cq_control_reg(gdev, COMMON_USE_SSID), COMMON_USE_SSID_VAL);

	do {
		value = hvgr_read_reg(gdev, cq_control_reg(gdev, COMMON_USE_SSID_STATUS));
		time--;
	} while ((value != COMMON_USE_SSID_EN) && (time != 0));

	if (time == 0) {
		hvgr_err(gdev, HVGR_DM, "common ssid set timeout");
		return -1;
	}

	return 0;
}

int hvgr_ctx_common_ctx_enable(struct hvgr_device * const gdev)
{
	struct hvgr_ctx *ctx = gdev->com_ctx.ctx;

	if (ctx == NULL) {
		hvgr_err(gdev, HVGR_DM, "no common ctx");
		return -1;
	}

	hvgr_mmu_hal_enable(ctx);

	if (hvgr_ctx_config_common_ssid(gdev) != 0) {
		hvgr_err(gdev, HVGR_DM, "config common ssid failed");
		return -1;
	}

	return 0;
}

#endif
