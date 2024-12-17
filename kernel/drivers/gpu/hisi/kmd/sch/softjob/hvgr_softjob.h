/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_SOFTJOB_H
#define HVGR_SOFTJOB_H

#include <linux/dma-fence.h>

#include "hvgr_defs.h"
#include "hvgr_cq_data.h"
#include "hvgr_ioctl_sch.h"

#define CQ_TOKEN_INVALID ((uint32_t)(-1))

bool hvgr_token_register_wait_semaphore(struct hvgr_cq_ctx *cq_ctx,
	struct hvgr_msync * const msync, wq_work_entry *entry);

bool hvgr_token_register_trigger_fence(struct hvgr_ctx  * const ctx,
	struct hvgr_msync  *msync, struct dma_fence *fence, bool ai_freq_enabled);

bool hvgr_token_register_wait_fence(struct hvgr_ctx  * const ctx,
	struct hvgr_msync *msync, struct dma_fence *fence,
	dma_fence_func_t callback_func);

void hvgr_token_unregister_wait_fence(struct hvgr_ctx  * const ctx,
	uint32_t token);

bool hvgr_token_register_hold_en_info(struct hvgr_ctx * const ctx, uint32_t token,
	enum protect_flag flag, void *data);

long hvgr_token_set_dependency(struct hvgr_ctx  * const ctx,
	uint32_t token, struct hvgr_msync *dependency);

long hvgr_ioctl_cq_set_freq_hint(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_set_freq_hint * const para);

long hvgr_ioctl_cq_set_cl_profiling(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_set_cl_profiling * const para);

long hvgr_ioctl_cq_clear_cl_flags(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_clear_cl_flags * const para);
/**
 * hvgr_token_do_wq_wait - do hvgr workqueue wait from user check
 *
 * @ctx: context pointer
 * @callback: msync callback
 */
void hvgr_token_do_wq_wait(struct hvgr_ctx *ctx, struct token_callback_data *callback);

bool hvgr_token_callback_deal(struct hvgr_ctx  *ctx, uint32_t token);

bool hvgr_token_callback_deal_without_lock(struct hvgr_ctx *ctx, uint32_t token);

void hvgr_token_finish(struct hvgr_cq_ctx * const cq_ctx, uint32_t token);

long hvgr_token_init(struct hvgr_ctx  *ctx);

void hvgr_token_term(struct hvgr_ctx  *ctx);

void hvgr_token_clean_all(struct hvgr_ctx *ctx, bool need_free_token);

bool hvgr_token_is_valid(struct hvgr_ctx * const ctx, uint32_t token);

void hvgr_token_do_freq_hint(struct hvgr_ctx *ctx, struct token_callback_data *callback);

#ifdef CONFIG_DFX_DEBUG_FS
long hvgr_ioctl_cq_record_token(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_cq_record_token * const para);
#endif

long hvgr_token_register_sc_policy_callback(struct hvgr_ctx * const ctx,
	struct hvgr_msync *msync, uint64_t policy_info, uint32_t info_num);

struct token_wq_entry *hvgr_token_wq_entry_remove(struct hvgr_cq_ctx_data *ctx_cq);

void hvgr_token_wq_entry_add(struct hvgr_cq_ctx_data *ctx_cq, struct token_wq_entry *tw_entry);

bool hvgr_token_callback_deal_without_lock(struct hvgr_ctx *ctx, uint32_t token);

long hvgr_token_add_one_depend(struct hvgr_ctx * const ctx,
	struct token_callback_data *depd_callback, uint32_t dep_token);

long hvgr_ioctl_softjob(struct hvgr_ctx * const ctx, unsigned int cmd, unsigned long arg);

void hvgr_free_token_callback_cb(struct hvgr_cq_ctx_data *ctx_cq,
	struct token_callback_data *callback);

void hvgr_callback_data_alloc_and_init(struct hvgr_ctx *ctx, uint32_t token);

void hvgr_token_cb_data_slab_term(void);

int hvgr_token_cb_data_slab_init(void);

#endif
