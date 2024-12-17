/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_datan_ct.h"

#include <securec.h>
#include <linux/mm.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/preempt.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/anon_inodes.h>

#include "hvgr_regmap.h"
#include "hvgr_kmd_defs.h"
#include "hvgr_dm_ctx.h"
#include "hvgr_pm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_asid_api.h"
#include "hvgr_cq_debug_switch.h"
#include "hvgr_ctx_sched.h"
#include "hvgr_assert.h"
#include "hvgr_dm_api.h"
#include "hvgr_datan_ct_time.h"
#include "hvgr_datan_ct_inner.h"
#include "hvgr_protect_mode.h"

struct hvgr_ct_wake_up_timer {
	struct hrtimer hrtimer;
	struct hvgr_ct_context *ct_ctx;
};

static int hvgr_ct_service_task(void *data);

static const struct file_operations hvgr_ct_client_fops = {
	.poll = hvgr_ct_data_poll,
	.release = hvgr_ct_data_release,
	.unlocked_ioctl = hvgr_ct_data_ioctl,
	.compat_ioctl = hvgr_ct_data_ioctl,
};

static int hvgr_ct_enable_go_on_without_block(struct hvgr_device *gdev)
{
	struct hvgr_ct_context *ct_ctx = gdev->ct_dev.ct_ctx;
	struct hvgr_ctx *ctx = ct_ctx->ctx;
	struct hvgr_ct_setup setup;

	hvgr_info(gdev, HVGR_DATAN, "%s enter.", __func__);

	setup.dump_buffer = ct_ctx->gpu_va;
	setup.fcm_bm = ct_ctx->bitmap[CT_FCM_INDEX];
	setup.mmu_l2_bm = ct_ctx->bitmap[CT_MMU_INDEX];
	setup.btc_bm = ct_ctx->bitmap[CT_BTC_INDEX];
	setup.bvh_bm = ct_ctx->bitmap[CT_BVH_INDEX];
	setup.gpc_bm = ct_ctx->bitmap[CT_GPC_INDEX];
	setup.set_id = ct_ctx->set_id;

	return hvgr_datan_ct_enable_inner(gdev, ctx, &setup);
}

int hvgr_api_ct_enable_and_go_on(struct hvgr_device *gdev)
{
	int ret = 0;

	if (!gdev->datan_dev.datan_ct_dev.ct_config)
		return -ENOMEM;
	hvgr_info(gdev, HVGR_DATAN, "%s enter.", __func__);

	if (hvgr_pm_get_gpu_status(gdev) == HVGR_PM_GPU_READY &&
		(gdev->datan_dev.datan_ct_dev.ct_closed)) {
		ret = hvgr_ct_enable_go_on_without_block(gdev);
		if (ret != 0)
			return ret;
		gdev->datan_dev.datan_ct_dev.ct_closed = false;
	}
	return ret;
}

static int hvgr_ct_enable_logic(struct hvgr_ct_context *ct_ctx)
{
	struct hvgr_ctx *ctx = ct_ctx->ctx;
	struct hvgr_device *gdev = ctx->gdev;
	int ret;

	hvgr_info(gdev, HVGR_DATAN, "%s enter.", __func__);

	mutex_lock(&gdev->pm_dev.pm_pwr.power_lock);
	gdev->datan_dev.datan_ct_dev.ct_config = true;
	ret = hvgr_api_ct_enable_and_go_on(gdev);
	if (ret != 0) {
		hvgr_err(ct_ctx->gdev, HVGR_DATAN, "%s error enter.", __func__);
		gdev->datan_dev.datan_ct_dev.ct_config = false;
	}
	mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);

	return ret;
}

static void hvgr_ct_disable_logic(struct hvgr_ct_context *ct_ctx)
{
	struct hvgr_ctx *ctx = ct_ctx->ctx;
	struct hvgr_device *gdev = ctx->gdev;
	int ret;

	hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s enter.", __func__);

	mutex_lock(&gdev->pm_dev.pm_pwr.power_lock);
	gdev->datan_dev.datan_ct_dev.ct_config = false;

	if (hvgr_pm_get_gpu_status(gdev) == HVGR_PM_GPU_READY &&
		(!gdev->datan_dev.datan_ct_dev.ct_closed)) {
		hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s if enter.", __func__);
		ret = hvgr_datan_ct_disable_inner(ctx);
		if (ret != 0) {
			mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);
			return;
		}
		gdev->datan_dev.datan_ct_dev.ct_closed = true;
	}
	mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);
}

static int hvgr_ct_reset_logic(struct hvgr_ct_context *ct_ctx)
{
	hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s enter.", __func__);
	hvgr_ct_disable_logic(ct_ctx);
	return hvgr_ct_enable_logic(ct_ctx);
}

static void hvgr_ct_set_bitmap(u32 dst[CT_MAX_COUNT], u32 dst_size,
	const u32 src[CT_MAX_COUNT], u32 src_size)
{
	datan_unused(dst_size);
	datan_unused(src_size);

	dst[CT_FCM_INDEX] = src[CT_FCM_INDEX];
	dst[CT_MMU_INDEX] = src[CT_MMU_INDEX];
	dst[CT_BTC_INDEX] = src[CT_BTC_INDEX];
	dst[CT_BVH_INDEX] = src[CT_BVH_INDEX];
	dst[CT_GPC_INDEX] = src[CT_GPC_INDEX];
}

static void hvgr_ct_union_bitmap(u32 dst[CT_MAX_COUNT], u32 dst_size,
	const u32 src[CT_MAX_COUNT], u32 src_size)
{
	datan_unused(dst_size);
	datan_unused(src_size);

	dst[CT_FCM_INDEX] |= src[CT_FCM_INDEX];
	dst[CT_MMU_INDEX] |= src[CT_MMU_INDEX];
	dst[CT_BTC_INDEX] |= src[CT_BTC_INDEX];
	dst[CT_BVH_INDEX] |= src[CT_BVH_INDEX];
	dst[CT_GPC_INDEX] |= src[CT_GPC_INDEX];
}

static int hvgr_ct_map_kernel_dump_buffer(struct hvgr_ct_context *ct_ctx)
{
	long ret;
	uint64_t page_num;
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mmu_setup_paras mmu_para = {0};
	struct hvgr_ctx *ctx = ct_ctx->ctx;

	ret = hvgr_mem_init_proc(ctx);
	if (ret != 0)
		return -ENOMEM;

	area = kzalloc(sizeof(*area), GFP_KERNEL);
	if (area == NULL) {
		hvgr_mem_term_proc(ctx);
		return -ENOMEM;
	}
	ct_ctx->dump_size = HVGR_CT_DUMP_SIZE;
	page_num = (uint32_t)hvgr_page_nums(HVGR_CT_DUMP_SIZE);

	ctx->mem_ctx.zones[0].pmem = &ctx->mem_ctx;
	area->zone = &ctx->mem_ctx.zones[0];
	/* one is global dump buffer, other is global accumulate buffer */
	area->pages = page_num + page_num;;
	area->max_pages = area->pages;
	if (!hvgr_mem_alloc_pages(area)) {
		kfree(area);
		hvgr_mem_term_proc(ctx);
		return -ENOMEM;
	}
	area->attribute = HVGR_MEM_ATTR_GPU_WR | HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_KMD_ACCESS;

	if (!hvgr_mem_map_kva(area)) {
		(void)hvgr_mem_free_pages(area);
		kfree(area);
		hvgr_mem_term_proc(ctx);
		return -ENOMEM;
	}

	area->gva = area->kva;
	hvgr_mem_attr2flag(area);

	mmu_para.gva = area->gva;
	mmu_para.page_array = area->page_array;
	mmu_para.pages = area->pages;
	mmu_para.mmu_flag = area->mmu_flag;
	mmu_para.flags = area->flags;

	ret = hvgr_mmu_set_up(ctx, &mmu_para);
	if (ret != 0) {
		hvgr_mem_unmap_kva(area);
		(void)hvgr_mem_free_pages(area);
		kfree(area);
		hvgr_mem_term_proc(ctx);
		return -ENOMEM;
	}

	ct_ctx->area = area;
	ct_ctx->cpu_va = u64_to_ptr(area->kva);
	ct_ctx->gpu_va = area->gva;
	ct_ctx->accu_cpu_va = ct_ctx->cpu_va + page_num * HVGR_MEM_PAGE_SIZE;
	return 0;
}

static void hvgr_ct_unmap_kernel_dump_buffer(struct hvgr_ct_context *ct_ctx)
{
	if (ct_ctx == NULL || ct_ctx->ctx == NULL)
		return;

	if (ct_ctx->area != NULL) {
		(void)hvgr_mmu_take_down(ct_ctx->ctx, ct_ctx->area->gva, (uint32_t)ct_ctx->area->pages);
		hvgr_mem_unmap_kva(ct_ctx->area);
		(void)hvgr_mem_free_pages(ct_ctx->area);
		kfree(ct_ctx->area);
	}

	hvgr_mem_term_proc(ct_ctx->ctx);

	ct_ctx->area = NULL;
	ct_ctx->cpu_va = NULL;
	ct_ctx->gpu_va = 0;
}

static int hvgr_ct_create_ctx(struct hvgr_ct_context *ct_ctx)
{
	struct hvgr_device *gdev = ct_ctx->gdev;
	int ret;

	hvgr_info(gdev, HVGR_DATAN, "%s enter.", __func__);
	/* create ctx */
	ct_ctx->ctx = vmalloc(sizeof(*ct_ctx->ctx));
	if (ct_ctx->ctx == NULL) {
		hvgr_err(gdev, HVGR_DATAN, "ct_ctx->ctx create fail");
		return -ENOMEM;
	}
	(void)memset_s(ct_ctx->ctx, sizeof(*ct_ctx->ctx), 0, sizeof(*ct_ctx->ctx));
	ct_ctx->ctx->asid = ct_ctx->as_id;
	ct_ctx->ctx->gdev = gdev;
	ct_ctx->accum_flag = false;

	ret = hvgr_ct_map_kernel_dump_buffer(ct_ctx);
	if (ret != 0)
		goto failed_map;

	ret = hvgr_ct_enable_logic(ct_ctx);
	if (ret != 0)
		goto failed_enable;

	ct_ctx->thread = kthread_run(hvgr_ct_service_task, ct_ctx, "hvgr_ct_service");
	if (IS_ERR(ct_ctx->thread)) {
		ret = (int)PTR_ERR(ct_ctx->thread);
		goto failed_kthread;
	}

	return 0;

failed_kthread:
	hvgr_ct_disable_logic(ct_ctx);
failed_enable:
	hvgr_ct_unmap_kernel_dump_buffer(ct_ctx);
failed_map:
	vfree(ct_ctx->ctx);
	ct_ctx->ctx = NULL;
	hvgr_err(gdev, HVGR_DATAN, "%s failed.", __func__);
	return ret;
}

static void hvgr_ct_destroy_ctx(struct hvgr_ct_context *ct_ctx)
{
	ct_ctx->thread = NULL;
	hvgr_ct_disable_logic(ct_ctx);
	hvgr_ct_unmap_kernel_dump_buffer(ct_ctx);

	vfree(ct_ctx->ctx);
	ct_ctx->ctx = NULL;
}

static struct hvgr_ct_client *hvgr_ct_create_client(struct hvgr_ct_context *ct_ctx,
	struct hvgr_ctx *ctx, const struct hvgr_ct_cfg *ct_cfg, u32 ring_num)
{
	struct hvgr_ct_client *client = NULL;

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (client == NULL)
		return NULL;

	client->ct_ctx = ct_ctx;
	client->ring_num = ring_num;
	client->event_mask =
		((u32)1 << CT_TIME_EVENT_MANUAL) | ((u32)1 << CT_TIME_EVENT_PERIODIC);
	client->pending = true;

	hvgr_ct_set_bitmap(client->bitmap, ARRAY_SIZE(client->bitmap),
		ct_cfg->bitmap, ARRAY_SIZE(ct_cfg->bitmap));
	client->set_id = ct_cfg->set_id;
	client->accum_flag = false;
	client->ctx = ctx;

	return client;
}

static struct hvgr_mem_area *hvgr_ct_mem_allocate(struct hvgr_ctx *ctx,
	uint64_t attribute, uint64_t bytes)
{
	struct hvgr_mem_alloc_para para;

	para.property = HVGR_MEM_PROPERTY_DRIVER;
	para.attribute = attribute;
	para.pages = (uint32_t)hvgr_page_nums(bytes);
	para.max_pages = para.pages;

	return hvgr_mem_allocate(ctx, &para, NULL, false);
}

static int hvgr_ct_ring_buffer_allocate(struct hvgr_ct_client *client, uint64_t *ctrl_addr)
{
	struct hvgr_mem_area *kmd_ctrl_area = NULL;
	struct hvgr_mem_area *user_ctrl_area = NULL;
	struct hvgr_mem_area *data_ctrl_area = NULL;
	struct hvgr_ctx *ctx = NULL;

	ctx = client->ctx;
	/* 1. alloc raw data ctrl mem. */
	data_ctrl_area = hvgr_ct_mem_allocate(ctx,
		HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR | HVGR_MEM_ATTR_KMD_ACCESS,
		client->ring_num * sizeof(struct hvgr_ct_queue_data_ctrl));
	if (data_ctrl_area == NULL)
		return -1;

	/* 2. alloc read index mem. */
	user_ctrl_area = hvgr_ct_mem_allocate(ctx,
		HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR | HVGR_MEM_ATTR_KMD_ACCESS,
		sizeof(struct hvgr_ct_queue_user_ctrl));
	if (user_ctrl_area == NULL) {
		(void)hvgr_mem_free(ctx, data_ctrl_area->uva);
		return -1;
	}

	/* 3. alloc kmd ctrl mem. */
	kmd_ctrl_area = hvgr_ct_mem_allocate(ctx,
		HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_KMD_ACCESS,
		sizeof(struct hvgr_ct_queue_ctrl));
	if (kmd_ctrl_area == NULL) {
		(void)hvgr_mem_free(ctx, data_ctrl_area->uva);
		(void)hvgr_mem_free(ctx, user_ctrl_area->uva);
		return -1;
	}

	client->data_ctrl_area = data_ctrl_area;
	client->data_ctrl = (struct hvgr_ct_queue_data_ctrl *)u64_to_ptr(data_ctrl_area->kva);

	client->user_ctrl_area = user_ctrl_area;
	client->user_ctrl = (struct hvgr_ct_queue_user_ctrl *)u64_to_ptr(user_ctrl_area->kva);
	client->user_ctrl->rd = 0;

	client->kmd_ctrl_area = kmd_ctrl_area;
	client->kmd_ctrl = (struct hvgr_ct_queue_ctrl *)u64_to_ptr(kmd_ctrl_area->kva);
	client->kmd_ctrl->wr = 0;
	client->kmd_ctrl->user_ctrl_addr = user_ctrl_area->uva;
	client->kmd_ctrl->ct_raw_base = data_ctrl_area->uva;

	/* 4.Transport to the up user. */
	*ctrl_addr = kmd_ctrl_area->uva;
	return 0;
}

static int hvgr_ct_init_client_buffer(struct hvgr_ct_context *ct_ctx,
	struct hvgr_ct_client *client, void *argp, uint64_t *ctrl_addr)
{
	client->dump_size = HVGR_CT_DUMP_SIZE;
	client->accum_buffer = kzalloc(client->dump_size, GFP_KERNEL);
	if (client->accum_buffer == NULL)
		return -EINVAL;

	if (client->ring_num > 0) {
		int *fd = (int *)argp;

		if (hvgr_ct_ring_buffer_allocate(client, ctrl_addr) != 0)
			return -EINVAL;

		kref_get(&client->ctx->ctx_kref);
		*fd = anon_inode_getfd("[hvgr_ct_desc]",
			&hvgr_ct_client_fops, client, O_RDONLY | O_CLOEXEC);
		if (*fd < 0) {
			kref_put(&client->ctx->ctx_kref, hvgr_ctx_kref_release);
			return -EINVAL;
		}
	} else {
		client->legacy_buffer = (void __user *)argp;
	}

	init_waitqueue_head(&client->waitq);
	return 0;
}

static struct hvgr_ct_client *hvgr_ct_attach_client(struct hvgr_ct_context *ct_ctx,
	struct hvgr_ctx *ctx, const struct hvgr_ct_cfg *ct_cfg, void *argp, uint64_t *ctrl_addr)
{
	struct task_struct *thread = NULL;
	struct hvgr_ct_client *client = NULL;
	u32 ring_num = ct_cfg->ring_num;

	if (ring_num > RING_BUFFER_MAX_SIZE) {
		hvgr_err(ct_ctx->gdev, HVGR_DATAN, "ring_num=%u", ring_num);
		return NULL;
	}

	client = hvgr_ct_create_client(ct_ctx, ctx, ct_cfg, ring_num);
	if (client == NULL) {
		hvgr_err(ct_ctx->gdev, HVGR_DATAN, "client is null");
		return NULL;
	}

	mutex_lock(&ct_ctx->lock);

	if (ctrl_addr != NULL) {
		if (atomic_read(&ct_ctx->pm_ref_cnt) == 0)
			(void)hvgr_pm_request_gpu(ctx->gdev, false);

		atomic_inc(&ct_ctx->pm_ref_cnt);
	}

	hvgr_ct_union_bitmap(ct_ctx->bitmap, ARRAY_SIZE(ct_ctx->bitmap),
		client->bitmap, ARRAY_SIZE(client->bitmap));
	ct_ctx->set_id = client->set_id;
	ct_ctx->reprogram = true;

	if (ct_ctx->client_num == 0) {
		hvgr_ct_set_bitmap(ct_ctx->bitmap, ARRAY_SIZE(ct_ctx->bitmap),
			client->bitmap, ARRAY_SIZE(client->bitmap));
		if (hvgr_ct_create_ctx(ct_ctx) < 0) {
			hvgr_err(ct_ctx->gdev, HVGR_DATAN, "hvgr_ct_create_ctx fail");
			goto error;
		}
		ct_ctx->reprogram = false;
		client->pending = false;
	}
	ct_ctx->client_num++;
	list_add(&client->list, &ct_ctx->idle_clients);

	if (hvgr_ct_init_client_buffer(ct_ctx, client, argp, ctrl_addr) != 0) {
		hvgr_err(ct_ctx->gdev, HVGR_DATAN, "hvgr_ct_init_client_buffer fail");
		goto error_init_client;
	}
	mutex_unlock(&ct_ctx->lock);

	return client;

error_init_client:
	if (client->accum_buffer != NULL)
		kfree(client->accum_buffer);
	ct_ctx->client_num--;
	list_del(&client->list);

error:
	if (ctrl_addr != NULL) {
		if (atomic_dec_and_test(&ct_ctx->pm_ref_cnt))
			hvgr_pm_release_gpu(ct_ctx->gdev);
	}
	if ((ct_ctx->client_num == 0) && (ct_ctx->ctx != NULL)) {
		thread = ct_ctx->thread;
		hvgr_ct_destroy_ctx(ct_ctx);
	}
	kfree(client);
	mutex_unlock(&ct_ctx->lock);
	if (thread != NULL)
		kthread_stop(thread);
	return NULL;
}

static void hvgr_ct_rebuild_ctx_bitmap(struct hvgr_ct_context *ct_ctx)
{
	struct hvgr_ct_client *iter = NULL;
	u32 zerobitmap[CT_MAX_COUNT] = {0};

	hvgr_ct_set_bitmap(ct_ctx->bitmap, ARRAY_SIZE(ct_ctx->bitmap),
		zerobitmap, ARRAY_SIZE(zerobitmap));
	list_for_each_entry(iter, &ct_ctx->idle_clients, list)
		hvgr_ct_union_bitmap(ct_ctx->bitmap, ARRAY_SIZE(ct_ctx->bitmap),
			iter->bitmap, ARRAY_SIZE(iter->bitmap));
	list_for_each_entry(iter, &ct_ctx->waiting_clients, list)
		hvgr_ct_union_bitmap(ct_ctx->bitmap, ARRAY_SIZE(ct_ctx->bitmap),
			iter->bitmap, ARRAY_SIZE(iter->bitmap));
}

void hvgr_ct_detach_client(struct hvgr_ct_client *client)
{
	struct hvgr_ct_context *ct_ctx = NULL;
	struct hvgr_ct_client *iter = NULL;
	struct hvgr_ct_client *tmp = NULL;
	struct task_struct *thread = NULL;
	int cli_found = 0;

	hvgr_assert(client != NULL);
	ct_ctx = client->ct_ctx;
	hvgr_assert(ct_ctx != NULL);
	mutex_lock(&ct_ctx->lock);

	hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s enter.", __func__);

	list_for_each_entry_safe(iter, tmp, &ct_ctx->idle_clients, list) {
		if (iter == client) {
			ct_ctx->reprogram = true;
			cli_found = 1;
			list_del(&iter->list);
			break;
		}
	}
	if (cli_found == 0) {
		list_for_each_entry_safe(iter, tmp, &ct_ctx->waiting_clients, list) {
			if (iter == client) {
				ct_ctx->reprogram = true;
				cli_found = 1;
				list_del(&iter->list);
				break;
			}
		}
	}
	hvgr_assert(cli_found);

	if (client->data_ctrl_area != NULL && client->ctx != NULL) {
		if (atomic_dec_and_test(&ct_ctx->pm_ref_cnt))
			hvgr_pm_release_gpu(ct_ctx->gdev);

		(void)hvgr_mem_free(client->ctx, client->data_ctrl_area->uva);
		client->data_ctrl_area = NULL;
		client->data_ctrl = NULL;
	}

	if (client->user_ctrl_area != NULL && client->ctx != NULL) {
		(void)hvgr_mem_free(client->ctx, client->user_ctrl_area->uva);
		client->user_ctrl_area = NULL;
		client->user_ctrl = NULL;
	}

	if (client->kmd_ctrl_area != NULL && client->ctx != NULL) {
		(void)hvgr_mem_free(client->ctx, client->kmd_ctrl_area->uva);
		client->kmd_ctrl_area = NULL;
		client->kmd_ctrl = NULL;
	}

	if (client->accum_buffer != NULL) {
		kfree(client->accum_buffer);
		client->accum_buffer = NULL;
	}

	kfree(client);

	ct_ctx->client_num--;
	if (ct_ctx->client_num == 0) {
		thread = ct_ctx->thread;
		hvgr_ct_destroy_ctx(ct_ctx);
	}

	hvgr_ct_rebuild_ctx_bitmap(ct_ctx);

	mutex_unlock(&ct_ctx->lock);

	if (thread != NULL)
		kthread_stop(thread);
}

static void hvgr_ct_accum_dump_buffer(void *dst, const void *src,
	size_t src_size, size_t dst_size, bool accume_flag)
{
	u32 *d = dst;
	const u32 *s = src;
	size_t i;
	size_t j;
	int ret;

	if (!accume_flag) {
		ret = memcpy_s(dst, dst_size, src, src_size);
		if (ret != EOK)
			pr_err("memcpy fail, src_size=%d", src_size);
		return;
	}
	for (i = 0; i < CT_ONCE_DUMP_GROUP_NUM; i++) {
		d += HEADER_SIZE_BY_UINT;
		s += HEADER_SIZE_BY_UINT;
		for (j = 0; j < BODY_SIZE_BY_UINT; j++) {
			if (U32_MAX - *d < *s)
				*d = U32_MAX;
			else
				*d += *s;
			d++;
			s++;
		}
	}
}

static void hvgr_ct_dump_buffer_head_update(struct hvgr_ct_context *ct_ctx,
	struct hvgr_ct_client *client, bool accume_flag)
{
	u32 i;
	u32 nr_l2;
	int ret;
	u64 core_mask = 7;
	u32 *mask = NULL;
	u8 *dst = client->accum_buffer;
	u8 *src = ct_ctx->cpu_va;

	/* copy and patch job manager header */
	if (accume_flag) {
		ret = memcpy_s(dst, HEADER_SIZE_BY_BYTES, src, HEADER_SIZE_BY_BYTES);
		if (ret != EOK)
			pr_err("memcpy jm fail");
	}
	mask = (u32 *)&dst[BITMASK_OFFSET_IN_HEADER];
	*mask &= client->bitmap[CT_FCM_INDEX];
	dst += GROUP_SIZE_BY_BYTES;
	src += GROUP_SIZE_BY_BYTES;

	/* copy and patch hvgr MMU/L2C headers */
	nr_l2 = 1;
	for (i = 0; i < nr_l2; i++) {
		if (accume_flag) {
			ret = memcpy_s(dst, HEADER_SIZE_BY_BYTES, src, HEADER_SIZE_BY_BYTES);
			if (ret != EOK)
				pr_err("memcpy MMU/L2C fail");
		}
		mask = (u32 *)&dst[BITMASK_OFFSET_IN_HEADER];
		*mask &= client->bitmap[CT_MMU_INDEX];
		dst += GROUP_SIZE_BY_BYTES;
		src += GROUP_SIZE_BY_BYTES;
	}

	/* copy and patch tiler header */
	if (accume_flag) {
		ret = memcpy_s(dst, HEADER_SIZE_BY_BYTES, src, HEADER_SIZE_BY_BYTES);
		if (ret != EOK)
			pr_err("memcpy tiler fail");
	}
	mask = (u32 *)&dst[BITMASK_OFFSET_IN_HEADER];
	*mask &= client->bitmap[CT_BTC_INDEX];
	dst += GROUP_SIZE_BY_BYTES;
	src += GROUP_SIZE_BY_BYTES;

	/* copy and patch bvh header */
	if (accume_flag) {
		ret = memcpy_s(dst, HEADER_SIZE_BY_BYTES, src, HEADER_SIZE_BY_BYTES);
		if (ret != EOK)
			pr_err("memcpy bvh fail");
	}
	mask = (u32 *)&dst[BITMASK_OFFSET_IN_HEADER];
	*mask &= client->bitmap[CT_BVH_INDEX];
	dst += GROUP_SIZE_BY_BYTES;
	src += GROUP_SIZE_BY_BYTES;

	/* copy and patch shader core headers */
	while (core_mask != 0ULL) {
		if (accume_flag) {
			ret = memcpy_s(dst, HEADER_SIZE_BY_BYTES, src, HEADER_SIZE_BY_BYTES);
			if (ret != EOK)
				pr_err("memcpy shader core fail");
		}
		if ((core_mask & 1ULL) != 0ULL) {
			/* if block is not reserved update header */
			mask = (u32 *)&dst[BITMASK_OFFSET_IN_HEADER];
			*mask &= client->bitmap[CT_GPC_INDEX];
		}
		dst += GROUP_SIZE_BY_BYTES;
		src += GROUP_SIZE_BY_BYTES;

		core_mask >>= 1;
	}
}

static void hvgr_ct_accum_global(struct hvgr_ct_context *ct_ctx)
{
	hvgr_ct_accum_dump_buffer(ct_ctx->accu_cpu_va, ct_ctx->cpu_va,
		ct_ctx->dump_size, ct_ctx->dump_size, ct_ctx->accum_flag);

	ct_ctx->accum_flag = true;
}

static void hvgr_ct_accum_clients(struct hvgr_ct_context *ct_ctx)
{
	struct hvgr_ct_client *iter = NULL;

	if (!ct_ctx->accum_flag)
		return;

	list_for_each_entry(iter, &ct_ctx->idle_clients, list) {
		if (iter->pending)
			continue;

		hvgr_ct_accum_dump_buffer(iter->accum_buffer,
			ct_ctx->accu_cpu_va, iter->dump_size, iter->dump_size, iter->accum_flag);
		hvgr_ct_dump_buffer_head_update(ct_ctx, iter, iter->accum_flag);

		iter->accum_flag = true;
	}
	list_for_each_entry(iter, &ct_ctx->waiting_clients, list) {
		if (iter->pending)
			continue;

		hvgr_ct_accum_dump_buffer(iter->accum_buffer,
			ct_ctx->accu_cpu_va, iter->dump_size, iter->dump_size, iter->accum_flag);
		hvgr_ct_dump_buffer_head_update(ct_ctx, iter, iter->accum_flag);

		iter->accum_flag = true;
	}

	ct_ctx->accum_flag = false;
}

u64 hvgr_ct_get_dump_timestamp(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	struct timespec ts;

	getrawmonotonic(&ts);
#else
	struct timespec64 ts;

	ktime_get_raw_ts64(&ts);
#endif
	return (u64)ts.tv_sec * CT_DUMP_BY_MS + (u64)ts.tv_nsec;
}

void hvgr_ct_add_client_to_wait_lists(
	struct hvgr_ct_client *client, struct list_head *waiting_clients)
{
	struct hvgr_ct_client *tmp = NULL;

	if (list_empty(waiting_clients)) {
		list_add(&client->list, waiting_clients);
		return;
	}
	list_for_each_entry(tmp, waiting_clients, list) {
		if (tmp->dump_time > client->dump_time) {
			list_add_tail(&client->list, &tmp->list);
			return;
		}
	}
	list_add_tail(&client->list, waiting_clients);
}

/* collect include:
 * dump counter into global rawdata buffer
 * accumalute rawdata into global accu buffer
 */
static int hvgr_ct_collect(struct hvgr_ct_context *ct_ctx, u64 *timestamp)
{
	int ret;
	unsigned long flags;
	struct hvgr_device * const gdev = ct_ctx->gdev;
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;

	if (gdev->datan_dev.datan_ct_dev.ct_closed)
		return 0;

	if (hvgr_get_prot_enhance(gdev)) {
		spin_lock_irqsave(&gcqctx->protect_mode_switch, flags);
		hvgr_info(gdev, HVGR_DATAN, "prot mode lock in.");
		if (gcqctx->protect_mode_info.protected_mode ||
			gcqctx->protect_mode_info.protected_mode_transition) {
			hvgr_info(gdev, HVGR_DATAN, "prot mode lock1 out.");
			spin_unlock_irqrestore(&gcqctx->protect_mode_switch, flags);
			return 0;
		}
	} else {
		hvgr_info(gdev, HVGR_DATAN, "disable preempt.");
		preempt_disable();
	}

	*timestamp = hvgr_ct_get_dump_timestamp();
	ret = hvgr_datan_ct_dump_inner(ct_ctx->ctx);
	hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s ret=%d enter.", __func__, ret);
	if (!hvgr_get_prot_enhance(gdev)) {
		hvgr_info(gdev, HVGR_DATAN, "enable preempt.");
		preempt_enable();
	}
	if (ret == 0)
		ret = hvgr_datan_ct_wait_for_dump_inner(ct_ctx->ctx);
	WARN_ON(ret);

	if (hvgr_get_prot_enhance(gdev)) {
		hvgr_info(gdev, HVGR_DATAN, "prot mode lock2 out.");
		spin_unlock_irqrestore(&gcqctx->protect_mode_switch, flags);
	}

	if (ret == 0)
		hvgr_ct_accum_global(ct_ctx);

	return ret;
}

static int hvgr_ct_collect_and_accumulate(
	struct hvgr_ct_context *ct_ctx, u64 *timestamp)
{
	int ret;

	hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s enter.", __func__);
	ret = hvgr_ct_collect(ct_ctx, timestamp);
	if (ret == 0)
		hvgr_ct_accum_clients(ct_ctx);

	return ret;
}

static void hvgr_ct_collect_and_disable(struct hvgr_device *gdev, bool dump_ct)
{
	int ret;
	struct hvgr_ct_context *ct_ctx = gdev->ct_dev.ct_ctx;
	u64 unused;

	if (dump_ct) {
		ret = hvgr_ct_collect(ct_ctx, &unused);
		if (ret != 0) {
			hvgr_err(gdev, HVGR_DATAN, "%s Dump Failed.", __func__);
			return;
		}
	}

	ret = hvgr_datan_ct_disable_inner(ct_ctx->ctx);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_DATAN, "%s Disable Failed.", __func__);
		return;
	}
}

/* the caller should hold power_lock */
void hvgr_api_ct_collect_and_disable(struct hvgr_device *gdev, bool dump_ct)
{
	hvgr_info(gdev, HVGR_DATAN, "%s config=%d, closed=%d enter.", __func__,
		gdev->datan_dev.datan_ct_dev.ct_config,
		gdev->datan_dev.datan_ct_dev.ct_closed);

	if (!gdev->datan_dev.datan_ct_dev.ct_config)
		return;

	if (gdev->datan_dev.datan_ct_dev.ct_closed)
		return;

	if (hvgr_pm_get_gpu_status(gdev) == HVGR_PM_GPU_READY)
		hvgr_ct_collect_and_disable(gdev, dump_ct);
	gdev->datan_dev.datan_ct_dev.ct_closed = true;
}

static int hvgr_ct_update_fixed_time_buffer(struct hvgr_ct_client *client,
	u64 timestamp, enum hvgr_ct_time_event event_id)
{
	struct hvgr_ct_queue_ctrl *kmd_ctrl = NULL;
	struct hvgr_ct_queue_data_ctrl *buffer = NULL;
	uint32_t rd = 0;
	uint32_t wr = 0;
	int ret;
	struct hvgr_device *gdev = hvgr_get_device();

	if (client == NULL || client->kmd_ctrl == NULL || gdev == NULL
		|| client->user_ctrl == NULL || client->data_ctrl == NULL)
		return -EINVAL;

	kmd_ctrl = client->kmd_ctrl;
	rd = client->user_ctrl->rd;
	wr = kmd_ctrl->wr;

	if (((wr + 1u) & (client->ring_num - 1u)) == rd) {
		hvgr_err(gdev, HVGR_DATAN, "ring full: [wr:%u] [rd:%u].", wr, rd);
		return -EINVAL;
	}
	hvgr_info(gdev, HVGR_DATAN, "current [wr:%u] [rd:%u].", wr, rd);
	buffer = &client->data_ctrl[wr];
	buffer->timestamp = timestamp;
	buffer->event_id = event_id;
	ret = memcpy_s(buffer->data, client->dump_size,
		client->accum_buffer, client->dump_size);
	kmd_ctrl->wr = ((wr + 1u) & (client->ring_num - 1u));
	if (ret != EOK)
		return -EINVAL;
	return ret;
}

static int hvgr_ct_fill_dump_buffer_legacy(struct hvgr_ct_client *client)
{
	void __user  *buffer = client->legacy_buffer;

	if (client->dump_size != HVGR_CT_DUMP_SIZE)
		return -EFAULT;
	if (copy_to_user(buffer, client->accum_buffer, client->dump_size) != 0) {
		pr_warn("error while copying buffer to user\n");
		return -EFAULT;
	}
	return 0;
}

static void hvgr_ct_reprogram(struct hvgr_ct_context *ct_ctx)
{
	struct hvgr_ct_client *iter = NULL;

	if (!ct_ctx->reprogram)
		return;

	if (hvgr_ct_reset_logic(ct_ctx) == 0) {
		ct_ctx->reprogram = false;
		list_for_each_entry(iter, &ct_ctx->idle_clients, list)
			iter->pending = false;
		list_for_each_entry(iter, &ct_ctx->waiting_clients, list)
			iter->pending = false;
	}
}

static int hvgr_ct_update_client(struct hvgr_ct_client *client,
	u64 timestamp, enum hvgr_ct_time_event event_id)
{
	int ret;

	if (client->ring_num > 0)
		ret = hvgr_ct_update_fixed_time_buffer(client, timestamp, event_id);
	else
		ret = hvgr_ct_fill_dump_buffer_legacy(client);

	if (ret != 0)
		goto exit;

	/* Notify client. Make sure all changes to memory are visible. */
	wmb();
	wake_up_interruptible(&client->waitq);

	(void)memset_s(client->accum_buffer, client->dump_size, 0, client->dump_size);
	client->accum_flag = false;
exit:
	return ret;
}

static enum hrtimer_restart hvgr_ct_wake_up_callback(struct hrtimer *hrtimer)
{
	struct hvgr_ct_wake_up_timer *timer =
		container_of(hrtimer, struct hvgr_ct_wake_up_timer, hrtimer);

	hvgr_assert(timer != NULL);

	atomic_set(&timer->ct_ctx->request_pending, 1);
	wake_up_all(&timer->ct_ctx->waitq);

	return HRTIMER_NORESTART;
}

static void hvgr_ct_process_expired_requests(struct hvgr_ct_context *ct_ctx,
	u64 timestamp, int ret)
{
	struct hvgr_ct_client *client = NULL;
	struct hvgr_ct_client *tmp = NULL;
	struct list_head expired_requests;

	INIT_LIST_HEAD(&expired_requests);

	list_for_each_entry_safe(client, tmp, &ct_ctx->waiting_clients, list) {
		s64 tdiff = (s64)(timestamp + CT_DUMP_MIN_INTERVAL) - (s64)client->dump_time;

		if (tdiff >= 0LL) {
			list_del(&client->list);
			list_add(&client->list, &expired_requests);
		} else {
			break;
		}
	}

	list_for_each_entry_safe(client, tmp, &expired_requests, list) {
		hvgr_assert(client->ring_num != 0);
		hvgr_assert(client->dump_interval != 0);

		if (ret == 0)
			hvgr_ct_update_client(client, timestamp, CT_TIME_EVENT_PERIODIC);

		do
			client->dump_time += client->dump_interval;
		while (client->dump_time < timestamp);

		list_del(&client->list);
		hvgr_ct_add_client_to_wait_lists(client, &ct_ctx->waiting_clients);
	}
}

static int hvgr_ct_service_task(void *data)
{
	struct hvgr_ct_context *ct_ctx = data;
	struct hvgr_ct_wake_up_timer *timer = NULL;
	struct hvgr_device *gdev = ct_ctx->gdev;

	hvgr_assert(ct_ctx != NULL);

	timer = kzalloc(sizeof(*timer), GFP_KERNEL);
	if (timer == NULL)
		return -ENOMEM;

	hrtimer_init(&timer->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	timer->hrtimer.function = hvgr_ct_wake_up_callback;
	timer->ct_ctx = ct_ctx;

	while (!kthread_should_stop()) {
		struct hvgr_ct_client *client = NULL;
		int ret;
		u64 dump_time = 0;
		u64 timestamp = hvgr_ct_get_dump_timestamp();

		mutex_lock(&ct_ctx->lock);

		if (current == ct_ctx->thread) {
			atomic_set(&ct_ctx->request_pending, 0);
			if (list_empty(&ct_ctx->waiting_clients) == 0) {
				client = list_first_entry(&ct_ctx->waiting_clients,
					struct hvgr_ct_client, list);
				dump_time = client->dump_time;
			}
		}

		/* If client == NULL, we need wait, until next request by user;
		 * If client != NULL, timestamp is less than dump_time, we need wait;
		 * If client != NULL, timestamp is greater than dump_time, we can dump directly. */
		if (client == NULL || ((s64)timestamp - (s64)dump_time < 0LL)) {
			mutex_unlock(&ct_ctx->lock);
			if (client != NULL) {
				u64 diff = dump_time - timestamp;
				hrtimer_start(&timer->hrtimer, ns_to_ktime(diff), HRTIMER_MODE_REL);
			}
			wait_event(ct_ctx->waitq,
				atomic_read(&ct_ctx->request_pending) || kthread_should_stop());
			(void)hrtimer_cancel(&timer->hrtimer);
			continue;
		}
		mutex_lock(&gdev->pm_dev.pm_pwr.power_lock);
		ret = hvgr_ct_collect_and_accumulate(ct_ctx, &timestamp);
		mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);

		hvgr_ct_process_expired_requests(ct_ctx, timestamp, ret);
		hvgr_ct_reprogram(ct_ctx);
		mutex_unlock(&ct_ctx->lock);
	}

	kfree(timer);

	return 0;
}

static int hvgr_ct_single_dump_setup(struct hvgr_ct_context *ct_ctx, struct hvgr_ctx *ctx,
	struct hvgr_ct_client  **client, struct hvgr_ct_setup *setup)
{
	hvgr_assert(ct_ctx != NULL);
	hvgr_assert(setup != NULL);
	hvgr_assert(client != NULL);

	hvgr_info(ct_ctx->gdev, HVGR_DATAN, "%s enter.", __func__);
	if (setup->dump_buffer != 0) {
		struct hvgr_ct_cfg ct_cfg;

		ct_cfg.bitmap[CT_FCM_INDEX] = setup->fcm_bm;
		ct_cfg.bitmap[CT_MMU_INDEX] = setup->mmu_l2_bm;
		ct_cfg.bitmap[CT_BTC_INDEX] = setup->btc_bm;
		ct_cfg.bitmap[CT_BVH_INDEX] = setup->bvh_bm;
		ct_cfg.bitmap[CT_GPC_INDEX] = setup->gpc_bm;
		ct_cfg.set_id = setup->set_id;
		ct_cfg.ring_num = 0;

		if ((*client) != NULL)
			return -EBUSY;
		*client = hvgr_ct_attach_client(ct_ctx, ctx, &ct_cfg,
			(void *)((uintptr_t)(setup->dump_buffer)), NULL);

		if ((*client) == NULL)
			return -ENOMEM;
	} else {
		if ((*client) == NULL)
			return -EINVAL;

		hvgr_ct_detach_client(*client);
		*client = NULL;
	}

	return 0;
}

void hvgr_api_ct_init_by_dev(struct hvgr_device * const gdev)
{
	struct hvgr_ct_context *ct_ctx = NULL;

	hvgr_datan_ct_init_status_inner(gdev);
	ct_ctx = kzalloc(sizeof(*ct_ctx), GFP_KERNEL);
	if (ct_ctx == NULL)
		return;
	INIT_LIST_HEAD(&ct_ctx->idle_clients);
	INIT_LIST_HEAD(&ct_ctx->waiting_clients);
	mutex_init(&ct_ctx->lock);
	ct_ctx->gdev = gdev;
	ct_ctx->thread = NULL;
	ct_ctx->as_id = DATAN_CT_DUMP_ASID;

	atomic_set(&ct_ctx->request_pending, 0);
	atomic_set(&ct_ctx->pm_ref_cnt, 0);
	init_waitqueue_head(&ct_ctx->waitq);

	gdev->ct_dev.ct_ctx = ct_ctx;
	spin_lock_init(&gdev->datan_dev.datan_ct_dev.ct_lock);
}

void hvgr_api_ct_term_by_dev(struct hvgr_ct_context *ct_ctx)
{
	struct hvgr_ct_client *client = NULL;

	if (ct_ctx == NULL)
		return;

	if (ct_ctx->thread != NULL)
		kthread_stop(ct_ctx->thread);

	while (1) {
		struct list_head *list = &ct_ctx->idle_clients;

		if (list_empty(list)) {
			list = &ct_ctx->waiting_clients;
			if (list_empty(list))
				break;
		}

		client = list_first_entry(list, struct hvgr_ct_client, list);
		list_del(&client->list);

		if (client->accum_buffer != NULL)
			kfree(client->accum_buffer);

		kfree(client);
		ct_ctx->client_num--;
	}
	hvgr_assert(!ct_ctx->client_num);
	if (ct_ctx->ctx != NULL)
		hvgr_ct_destroy_ctx(ct_ctx);
	kfree(ct_ctx);
}

long hvgr_api_ct_alloc(struct hvgr_ctx *ctx,
	union hvgr_ioctl_ct_alloc *const para)
{
	struct hvgr_ct_client *client = NULL;
	struct hvgr_ct_cfg ct_cfg;

	if (ctx == NULL || para == NULL) {
		hvgr_err(hvgr_get_device(), HVGR_DATAN, "%s invalid para.", __func__);
		return -1;
	}

	ct_cfg.bitmap[CT_FCM_INDEX] = para->in.fcm_bm;
	ct_cfg.bitmap[CT_MMU_INDEX] = para->in.mmu_l2_bm;
	ct_cfg.bitmap[CT_BTC_INDEX] = para->in.btc_bm;
	ct_cfg.bitmap[CT_BVH_INDEX] = para->in.bvh_bm;
	ct_cfg.bitmap[CT_GPC_INDEX] = para->in.gpc_bm;
	ct_cfg.set_id = para->in.set_id;
	ct_cfg.ring_num = para->in.ring_num;

	client = hvgr_ct_attach_client(ctx->gdev->ct_dev.ct_ctx, ctx, &ct_cfg, &para->out.fd,
		&para->out.ctrl_addr);
	if (client == NULL)
		return -ENOMEM;
	hvgr_info(ctx->gdev, HVGR_DATAN, "%s success.", __func__);
	return 0;
}

int hvgr_api_ct_enable(struct hvgr_ctx *ctx,
	struct hvgr_ioctl_ct_single_dump_enable *enable)
{
	int ret;
	struct hvgr_ct_setup setup = {
		.dump_buffer = enable->dump_buffer,
		.fcm_bm = enable->fcm_bm,
		.mmu_l2_bm = enable->mmu_l2_bm,
		.btc_bm = enable->btc_bm,
		.bvh_bm = enable->bvh_bm,
		.gpc_bm = enable->gpc_bm,
		.set_id = enable->set_id
	};

	hvgr_info(ctx->gdev, HVGR_DATAN, "%s enter.", __func__);

	mutex_lock(&ctx->counter_ctx.ct_client_lock);
	ret = hvgr_ct_single_dump_setup(ctx->gdev->ct_dev.ct_ctx, ctx,
		&ctx->counter_ctx.single_dump_client, &setup);
	mutex_unlock(&ctx->counter_ctx.ct_client_lock);

	hvgr_info(ctx->gdev, HVGR_DATAN, "%s exit.", __func__);

	return ret;
}

int hvgr_api_ct_close_ctx(struct hvgr_ctx *ctx)
{
	int ret = 0;

	mutex_lock(&ctx->counter_ctx.ct_client_lock);

	hvgr_info(ctx->gdev, HVGR_DATAN, "%s enter.", __func__);

	if (ctx->gdev->ct_dev.ct_ctx != NULL) {
		struct hvgr_ct_setup setup;

		setup.dump_buffer = 0;
		ret = hvgr_ct_single_dump_setup(ctx->gdev->ct_dev.ct_ctx, ctx,
			&ctx->counter_ctx.single_dump_client, &setup);
	}
	mutex_unlock(&ctx->counter_ctx.ct_client_lock);
	return ret;
}

void hvgr_api_ct_open_ctx(struct hvgr_ctx *ctx)
{
	mutex_init(&ctx->counter_ctx.ct_client_lock);
}

static int hvgr_ct_dump(struct hvgr_ct_client *client,
	enum hvgr_ct_time_event event_id)
{
	int ret = 0;

	struct hvgr_ct_context *ct_ctx = NULL;
	u64 timestamp;
	u32 event_mask;
	struct hvgr_device *gdev = NULL;

	if (client == NULL)
		return -EINVAL;

	ct_ctx = client->ct_ctx;
	gdev = ct_ctx->gdev;

	event_mask = (u32)1 << event_id;

	mutex_lock(&ct_ctx->lock);

	if (!(event_mask & client->event_mask))
		goto exit;

	mutex_lock(&gdev->pm_dev.pm_pwr.power_lock);

	hvgr_info(gdev, HVGR_DATAN, "%s enter.", __func__);

	if ((hvgr_pm_get_gpu_status(gdev) == HVGR_PM_GPU_READY) &&
		!gdev->datan_dev.datan_ct_dev.ct_closed) {
		ret = hvgr_ct_collect(ct_ctx, &timestamp);
		if (ret != 0) {
			mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);
			hvgr_err(ct_ctx->gdev, HVGR_DATAN, "%s, Dump Failed", __func__);
			goto exit;
		}
	} else {
		timestamp = hvgr_ct_get_dump_timestamp();
	}
	hvgr_ct_accum_clients(ct_ctx);
	mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);
	ret = hvgr_ct_update_client(client, timestamp, event_id);
	if (ret != 0)
		goto exit;

	hvgr_ct_reprogram(ct_ctx);

exit:
	mutex_unlock(&ct_ctx->lock);

	return ret;
}

int hvgr_api_ct_dump(struct hvgr_ctx *ctx)
{
	int ret;

	mutex_lock(&ctx->counter_ctx.ct_client_lock);
	ret = hvgr_ct_dump(ctx->counter_ctx.single_dump_client, CT_TIME_EVENT_MANUAL);
	mutex_unlock(&ctx->counter_ctx.ct_client_lock);

	return ret;
}
