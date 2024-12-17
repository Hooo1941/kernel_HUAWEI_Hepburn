/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 *
 * Description: This is dmd report module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <securec.h>
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_platform_api.h"

#ifdef CONFIG_HVGR_DFX_DMD_REPORT
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
#include <dfx_bbox_diaginfo.h>
#else
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>
#endif
#endif

#ifdef CONFIG_HVGR_DFX_DMD_REPORT
#define INVALID_DMD_ID     0

static unsigned int hvgr_get_higpu_dmd_id(enum hvgr_dmd_id dmd_id)
{
	switch (dmd_id) {
	case DMD_JOB_FAIL:
		return DMD_HIGPU_JOB_FAIL;
	case DMD_JOB_HANG:
		return DMD_HIGPU_JOB_HANG;
	case DMD_FCP_LOAD_FAIL:
		return DMD_HIGPU_FCP_LOAD_FAIL;
	case DMD_BUS_FAULT:
		return DMD_HIGPU_BUS_FAULT;
	case DMD_PAGE_FAULT:
		return DMD_HIGPU_PAGE_FAULT;
	case DMD_PW_ONOFF_FAIL:
		return DMD_HIGPU_PW_ONOFF_FAIL;
	case DMD_HARD_RESET:
		return DMD_HIGPU_HARD_RESET;
#ifdef CONFIG_HVGR_DFX_SH
	case DMD_SH_NV_MARK:
		return DMD_HIGPU_SH_INFO;
#ifdef CONFIG_HVGR_VIRTUAL_GUEST
	case DMD_SH_HANG_NV_MARK:
		return DMD_HIGPU_SH_HANG_INFO;
#endif
#endif
	default :
		return INVALID_DMD_ID;
	}
}
int hvgr_dmd_msg_init(struct hvgr_device *gdev)
{
	struct gpu_dmd_msg *dmd_msg = NULL;

	if (gdev == NULL)
		return -EINVAL;

	dmd_msg = kzalloc(sizeof(*dmd_msg), GFP_KERNEL);
	if (dmd_msg == NULL) {
		hvgr_err(gdev, HVGR_DM, "%s kzalloc failed.", __func__);
		return -ENOMEM;
	}

	mutex_init(&dmd_msg->dmd_lock);
	atomic_set(&dmd_msg->dmd_flag, 0);
#ifdef CONFIG_HVGR_DFX_SH
	dmd_msg->dmd_sh_flag = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	dmd_msg->sr_wakeup_lock = wakeup_source_register("sr wakelock");
#else
	dmd_msg->sr_wakeup_lock = wakeup_source_register(NULL, "sr wakelock");
#endif
	if (dmd_msg->sr_wakeup_lock == NULL)
		hvgr_err(gdev, HVGR_DM, "%s reg sr wakelock fail.", __func__);
#endif
	gdev->dm_dev.dmd_msg = dmd_msg;
	return 0;
}

void hvgr_dmd_msg_term(struct hvgr_device *gdev)
{
	if (gdev == NULL)
		return;

	if (gdev->dm_dev.dmd_msg == NULL)
		return;

#ifdef CONFIG_HVGR_DFX_SH
	if (gdev->dm_dev.dmd_msg->sr_wakeup_lock != NULL)
		wakeup_source_unregister(gdev->dm_dev.dmd_msg->sr_wakeup_lock);
#endif

	kfree(gdev->dm_dev.dmd_msg);
}

void hvgr_dmd_msg_set(struct hvgr_device *gdev, enum hvgr_dmd_id dmd_id,
	const char *dmd_log)
{
	unsigned int higpu_dmd_id = hvgr_get_higpu_dmd_id(dmd_id);
	errno_t err;
#ifdef CONFIG_HVGR_DFX_SH
	struct timespec64 cur_ts;
	u32 check_idx;
#endif

	if (gdev == NULL || dmd_log == NULL)
		return;

	if (higpu_dmd_id == INVALID_DMD_ID) {
		hvgr_err(gdev, HVGR_DM, "%s higpu_dmd_id is INVALID ID.", __func__);
		return;
	}

	if (atomic_cmpxchg(&gdev->dm_dev.dmd_msg->dmd_flag, 0, 1) != 0) {
		hvgr_err(gdev, HVGR_DM, "higpu_dmd flag error: set %u when %u is reporting",
			dmd_id, gdev->dm_dev.dmd_msg->dmd_id);
		return;
	}

	if (unlikely(gdev->dm_dev.dmd_msg->dmd_id != INVALID_DMD_ID)) {
		hvgr_err(gdev, HVGR_DM, "higpu_dmd error: set %u when %u is reporting",
			dmd_id, gdev->dm_dev.dmd_msg->dmd_id);
		return;
	}

#ifdef CONFIG_HVGR_DFX_SH
	if (higpu_dmd_id == DMD_HIGPU_PW_ONOFF_FAIL ||
		higpu_dmd_id == DMD_HIGPU_FCP_LOAD_FAIL ||
		higpu_dmd_id == DMD_HIGPU_JOB_HANG ||
		higpu_dmd_id == DMD_HIGPU_HARD_RESET) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		get_monotonic_boottime64(&cur_ts);
#else
		ktime_get_boottime_ts64(&cur_ts);
#endif
		check_idx = gdev->dm_dev.dmd_msg->dmd_check_cnt % MAX_DMD_CHECK_CNT;
		gdev->dm_dev.dmd_msg->dmd_check_ts[check_idx] = (u64)cur_ts.tv_sec;
		++gdev->dm_dev.dmd_msg->dmd_check_cnt;
	}
#endif

	hvgr_debug(gdev, HVGR_DM, "higpu_dmd set dmd %u", dmd_id);

	err = strncpy_s(gdev->dm_dev.dmd_msg->dmd_log,
		MAX_DMD_LOG_LENTH, dmd_log, strlen(dmd_log));
	if (err != EOK)
		hvgr_err(gdev, HVGR_DM, "higpu_dmd set dmd log failed!");

	gdev->dm_dev.dmd_msg->dmd_id = higpu_dmd_id;
}

void hvgr_dmd_msg_report(struct hvgr_device *gdev)
{
	if (gdev == NULL)
		return;

	if (gdev->dm_dev.dmd_msg == NULL) {
		hvgr_err(gdev, HVGR_DM, "dmd_msg is NULL.");
		return;
	}

	mutex_lock(&gdev->dm_dev.dmd_msg->dmd_lock);
	if (gdev->dm_dev.dmd_msg->dmd_id == INVALID_DMD_ID) {
		hvgr_debug(gdev, HVGR_DM, "higpu_dmd error: report invalid dmd msg");
		mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
		return;
	}

	hvgr_debug(gdev, HVGR_DM, "higpu_dmd: report dmd %u", gdev->dm_dev.dmd_msg->dmd_id);

	bbox_diaginfo_record(gdev->dm_dev.dmd_msg->dmd_id, NULL,
		gdev->dm_dev.dmd_msg->dmd_log);
	/* init dmd msg after report */
	gdev->dm_dev.dmd_msg->dmd_id = INVALID_DMD_ID;
	atomic_set(&gdev->dm_dev.dmd_msg->dmd_flag, 0);
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);

#ifdef CONFIG_HVGR_DFX_SH
	hvgr_dmd_msg_check(gdev);
#endif
}

bool hvgr_dmd_id_check(struct hvgr_device *gdev)
{
	return gdev->dm_dev.dmd_msg->dmd_id == INVALID_DMD_ID;
}

void hvgr_dmd_msg_clear(struct hvgr_device *gdev)
{
	if (hvgr_dmd_id_check(gdev))
		return;
	/* clear dmd msg after gpu reset done if dmdmsg is not null. */
	mutex_lock(&gdev->dm_dev.dmd_msg->dmd_lock);
	gdev->dm_dev.dmd_msg->dmd_id = INVALID_DMD_ID;
	atomic_set(&gdev->dm_dev.dmd_msg->dmd_flag, 0);
	mutex_unlock(&gdev->dm_dev.dmd_msg->dmd_lock);
}

#else
int hvgr_dmd_msg_init(struct hvgr_device *gdev)
{
	return 0;
}

void hvgr_dmd_msg_term(struct hvgr_device *gdev)
{
}

void hvgr_dmd_msg_set(struct hvgr_device *gdev, enum hvgr_dmd_id dmd_id,
	const char *dmd_log)
{
}

void hvgr_dmd_msg_report(struct hvgr_device *gdev)
{
}

bool hvgr_dmd_id_check(struct hvgr_device *gdev)
{
	return true;
}

void hvgr_dmd_msg_clear(struct hvgr_device *gdev)
{
}
#endif

#define BIT32    32
#define BIT64    64
int hvgr_get_ctx_compat(struct hvgr_ctx *ctx)
{
	if (ctx == NULL)
		return 0;

	return hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT) ? BIT32 : BIT64;
}

char *hvgr_get_main_process_name(struct hvgr_ctx *ctx)
{
	if (ctx == NULL)
		return "unknown";

	if (likely(ctx->leader != NULL))
		return ctx->group_name;

	return ctx->process_name;
}
