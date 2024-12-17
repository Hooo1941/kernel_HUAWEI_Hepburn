/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DATAN_API_H
#define HVGR_DATAN_API_H

#include <linux/types.h>
#include <linux/errno.h>

#include "hvgr_defs.h"
#include "jobtrace_type.h"
#ifdef HVGR_DATAN_JOB_TIMELINE
#include "datan/jobtimeline/hvgr_job_timeline.h"
#endif

#ifdef CONFIG_HVGR_DFX_DEBUG_BUS_DUMP
#include "datan/debugbus/hvgr_debug_bus_dump.h"
#endif
#ifdef CONFIG_HVGR_DFX_MEM_DUMP
#include "datan/memdump/hvgr_mem_dump.h"
#endif
#include "datan/counter/hvgr_datan_ct.h"

#include "hvgr_dm_api.h"

long hvgr_ioctl_datan(struct hvgr_ctx * const ctx, unsigned int cmd, unsigned long arg);

int hvgr_datan_init_proc(struct hvgr_ctx *ctx);
void hvgr_datan_term_proc(struct hvgr_ctx *ctx);

int hvgr_datan_init(struct hvgr_device * const gdev);
void hvgr_datan_term(struct hvgr_device *gdev);

void hvgr_datan_dump_debug_bus(struct hvgr_device *gdev);

void hvgr_datan_dump_mem(struct hvgr_ctx * const ctx);

void hvgr_datan_dump_chain_state(struct hvgr_device * const gdev);

void hvgr_datan_cbit_config(struct hvgr_device * const gdev);

void hvgr_datan_gpu_fault_type_set(struct hvgr_device *gdev, hvgr_gpu_fault_id gpu_fault_id,
	const char *gpu_fault_log);

void hvgr_datan_gpu_fault_type_reset(struct hvgr_device *gdev);

#ifdef CONFIG_HVGR_DFX_DATAN
void hvgr_datan_jobtrace(struct hvgr_ctx *ctx, enum jobtrace_type trace_type,
	uint32_t trace_id, ...);
#else
static inline void hvgr_datan_jobtrace(struct hvgr_ctx *ctx, enum jobtrace_type trace_type,
	uint32_t trace_id, ...) {}
#endif
#if (CONFIG_HVGR_DFX_DATAN && hvgr_version_ge(HVGR_V350))
void datan_jobtrace_upgrade_fw(struct hvgr_ctx * const ctx);
void datan_jobtrace_global_reg_dump(struct hvgr_ctx * const ctx);
#else
static inline void datan_jobtrace_upgrade_fw(struct hvgr_ctx * const ctx) {}
static inline void datan_jobtrace_global_reg_dump(struct hvgr_ctx * const ctx) {}
#endif

void hvgr_datan_ct_enable_by_power(struct hvgr_device *gdev);
void hvgr_datan_ct_disable_by_power(struct hvgr_device *gdev, bool dump_ct);

#ifdef HVGR_DATAN_JOB_TIMELINE
void hvgr_timeline_record_phychn_status(struct hvgr_ctx *ctx, uint16_t channel_id,
	uint16_t bind_state, uint32_t queue_id);

void hvgr_timeline_record_entry_boundary(struct hvgr_ctx *ctx, uint16_t type, uint16_t chain_type,
	uint32_t token, uint32_t queue_id);

void hvgr_timeline_record_fence(struct hvgr_ctx *ctx, uint32_t token);

void hvgr_job_timeline_alloc_chain_buf(struct hvgr_ctx *ctx);
void hvgr_job_timeline_start_timestamp_buf(struct hvgr_ctx *ctx);
void hvgr_job_timeline_end_timestamp_buf(struct hvgr_ctx *ctx);
#endif
#endif