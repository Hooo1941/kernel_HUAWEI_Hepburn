/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_datan_ct_time.h"

#include <hvgr_regmap.h>
#include <hvgr_kmd_defs.h>
#include <hvgr_dm_ctx.h>
#include <securec.h>

#include "hvgr_pm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_dm_api.h"

#include "hvgr_asid_api.h"
#include "hvgr_cq_debug_switch.h"
#include "hvgr_ctx_sched.h"
#include "hvgr_datan_ct.h"
#include "hvgr_assert.h"

static int hvgr_ct_data_ready(const struct hvgr_ct_client *client)
{
	return (client->kmd_ctrl->wr != client->user_ctrl->rd);
}

static long hvgr_ct_set_interval(struct hvgr_ct_client *client, u32 interval)
{
	struct hvgr_ct_context *ct_ctx = client->ct_ctx;

	hvgr_assert(ct_ctx != NULL);
	mutex_lock(&ct_ctx->lock);
	list_del(&client->list);
	client->dump_interval = interval;

	if (client->dump_interval > 0) {
		if (client->dump_interval < CT_DUMP_MIN_INTERVAL)
			client->dump_interval = CT_DUMP_MIN_INTERVAL;

		client->dump_time = hvgr_ct_get_dump_timestamp() + client->dump_interval;
		hvgr_ct_add_client_to_wait_lists(client, &ct_ctx->waiting_clients);

		atomic_set(&ct_ctx->request_pending, 1);
		wake_up_all(&ct_ctx->waitq);
	} else {
		list_add(&client->list, &ct_ctx->idle_clients);
	}
	hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s interval=[%ld].", __func__, interval);

	mutex_unlock(&ct_ctx->lock);
	return 0;
}

long hvgr_ct_data_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long rcode;
	struct hvgr_ct_client *client = NULL;

	hvgr_assert(filp != NULL);

	client = filp->private_data;
	hvgr_assert(client != NULL);

	if (unlikely(_IOC_TYPE(cmd) != HVGR_DATAN_CT_TIME))
		return -EINVAL;

	switch (cmd) {
	case HVGR_DATAN_CT_TIME_SET_INTERVAL:
		rcode = hvgr_ct_set_interval(client, (u32)arg);
		break;
	default:
		rcode = -EINVAL;
		break;
	}

	return rcode;
}

unsigned int hvgr_ct_data_poll(struct file *filp, poll_table *wait)
{
	struct hvgr_ct_client *client = NULL;

	hvgr_assert(filp != NULL);
	hvgr_assert(wait != NULL);

	client = filp->private_data;
	hvgr_assert(client != NULL);

	poll_wait(filp, &client->waitq, wait);
	if (hvgr_ct_data_ready(client)) // (client->kmd_ctrl->wr != client->user_ctrl->rd);
		return POLLIN;
	return 0;
}

int hvgr_ct_data_release(struct inode *inode, struct file *filp)
{
	struct hvgr_ct_client *client = NULL;
	struct hvgr_ct_context *ct_ctx = NULL;
	struct hvgr_ctx *ctx = NULL;

	client = filp->private_data;
	if (client == NULL)
		return 0;

	ct_ctx = client->ct_ctx;
	if (ct_ctx == NULL)
		return 0;

	ctx = client->ctx;
	hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s enter.", __func__);
	hvgr_ct_detach_client(client);
	kref_put(&ctx->ctx_kref, hvgr_ctx_kref_release);
	return 0;
}