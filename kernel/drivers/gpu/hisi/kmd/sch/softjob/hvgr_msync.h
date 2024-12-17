/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_SYNC_H
#define HVGR_SYNC_H

#include "hvgr_defs.h"
#include "apicmd/hvgr_ioctl_sch.h"
#include "hvgr_cq_data.h"
#include "hvgr_msync_defs.h"

uint32_t hvgr_msync_wait(struct hvgr_ctx * const ctx, struct hvgr_msync * const msync_list,
	uint32_t msync_nums, uint64_t timeout_ns, bool wait_all);

long hvgr_ioctl_alloc_semp(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_alloc_semp * const para);

long hvgr_ioctl_free_semp(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_free_semp * const para);

/*
 * hvgr_semaphore_is_set() - check semaphore set or not.
 * @ctx: kmd context struct.
 * @sema_addr: kva of semaphore.
 * @sema_value: the value check for sema_addr.
 *  Return: true for set and false for not set.
 */
bool hvgr_semaphore_is_set(struct hvgr_ctx * const ctx, uint64_t sema_addr, uint64_t sema_value);

/*
 * hvgr_semaphore_set() - check semaphore set or not.
 * @sema_addr: kva of semaphore.
 * @sema_value: the value will be set to sema_addr.
 */
void hvgr_semaphore_set(struct hvgr_ctx * const ctx, uint64_t sema_addr, uint64_t sema_value);

/*
 * hvgr_semaphore_get_addr() - gva change to kva.
 * @sema_addr: gva of semaphore.
 *  Return: kva of semaphore.
 */
uint64_t hvgr_semaphore_get_addr(struct hvgr_ctx * const ctx, uint64_t sema_addr);

void hvgr_msync_event_wakeup(struct hvgr_ctx * const ctx);

void hvgr_msync_event_enqueue(struct hvgr_ctx * const ctx, struct msync_event *event);

uint32_t hvgr_token_dequeue(struct hvgr_ctx * const ctx, char __user *buf, uint32_t num);

uint32_t hvgr_msync_get_event_read_num(struct hvgr_ctx * const ctx, uint32_t count);

bool hvgr_msync_event_checkdata(struct hvgr_ctx * const ctx);

void hvgr_semaphore_print_value(struct hvgr_ctx * const ctx, uint64_t sema_kaddr,
	uint64_t sema_uaddr, uint64_t sema_value);

#endif
