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
#include "hvgr_cq.h"
#include "hvgr_msync.h"
#include "hvgr_ksync.h"
#include "hvgr_softjob.h"
#include "hvgr_schedule.h"

int hvgr_sch_init(struct hvgr_device * const gdev)
{
	struct hvgr_sch_dev *sch = &gdev->sch_dev;

	if (sch->ops == NULL)
		sch->ops = hvgr_sched_get_ops();
	return sch->ops->init(gdev);
}

void hvgr_sch_term(struct hvgr_device * const gdev)
{
	struct hvgr_sch_dev *sch = &gdev->sch_dev;

	if (sch->ops != NULL)
		sch->ops->term(gdev);
}

long hvgr_sch_ctx_create(struct hvgr_ctx * const ctx)
{
	struct hvgr_sch_backend_ops *backend_ops = ctx->gdev->sch_dev.ops;

	if (unlikely(backend_ops == NULL))
		return -EINVAL;

	return backend_ops->ctx_create(ctx);
}

void hvgr_sch_ctx_destroy(struct hvgr_ctx * const ctx)
{
	struct hvgr_sch_backend_ops *backend_ops = ctx->gdev->sch_dev.ops;

	if (unlikely(backend_ops == NULL))
		return;

	backend_ops->ctx_destroy(ctx);
}

long hvgr_sch_cmd_dispatch(struct hvgr_ctx * const ctx, unsigned int cmd, unsigned long arg)
{
	struct hvgr_sch_backend_ops *backend_ops = ctx->gdev->sch_dev.ops;

	if (unlikely(backend_ops == NULL))
		return -EINVAL;

	return backend_ops->cmd_dispatch(ctx, cmd, arg);
}

void hvgr_sch_start(struct hvgr_device * const gdev)
{
	if (unlikely(gdev->sch_dev.ops == NULL))
		return;

	gdev->sch_dev.ops->start(gdev);
}

void hvgr_sch_stop(struct hvgr_device * const gdev)
{
	if (unlikely(gdev->sch_dev.ops == NULL))
		return;

	gdev->sch_dev.ops->stop(gdev);
}

void hvgr_ctx_stop(struct hvgr_ctx * const ctx)
{
	if (unlikely(ctx->gdev->sch_dev.ops == NULL))
		return;

	ctx->gdev->sch_dev.ops->ctx_stop(ctx);
}

void hvgr_sch_reset(struct hvgr_device * const gdev, enum sch_rst_status reset_stage)
{
	if (unlikely(gdev->sch_dev.ops == NULL))
		return;

	gdev->sch_dev.ops->reset(gdev, reset_stage);
}

int hvgr_sch_suspend(struct hvgr_device * const gdev)
{
	if (unlikely(gdev->sch_dev.ops == NULL))
		return -EINVAL;

	return gdev->sch_dev.ops->suspend(gdev);
}

void hvgr_sch_resume(struct hvgr_device * const gdev)
{
	if (unlikely(gdev->sch_dev.ops == NULL))
		return;

	gdev->sch_dev.ops->resume(gdev);
}

void hvgr_sch_set_mode(struct hvgr_device * const gdev, uint32_t mode, uint32_t sch_flag)
{
	if (unlikely(gdev->sch_dev.ops == NULL))
		return;

	gdev->sch_dev.ops->set_mode(gdev, mode, sch_flag);
}

void hvgr_sch_intr_switch(struct hvgr_device * const gdev, uint32_t flag)
{
	if (unlikely(gdev->sch_dev.ops == NULL))
		return;

	gdev->sch_dev.ops->intr_switch(gdev, flag);
}

