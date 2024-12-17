/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_IOCTL_SCH_H
#define HVGR_IOCTL_SCH_H

#include "linux/types.h"
#include "apicmd/hvgr_ioctl.h"
#include "apicmd/hvgr_ioctl_softjob.h"

#define HVGR_IOCTL_CMD_ALLOC_CQ           (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 0)
#define HVGR_IOCTL_CMD_FREE_CQ            (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 1)
#define HVGR_IOCTL_CMD_SUBMIT_CQ          (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 2)
#define HVGR_IOCTL_CMD_ALLOC_SEMP         (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 3)
#define HVGR_IOCTL_CMD_FREE_SEMP          (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 4)
#define HVGR_IOCTL_CMD_CLEAR_ERR          (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 5)
#define HVGR_IOCTL_CMD_SUBMIT_CQ_TO_KMD   (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 6)
#define HVGR_IOCTL_CMD_RECORD_TOKEN       (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 7)
#define HVGR_IOCTL_CMD_INIT_ENTRY_ADDR    (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 8)
#define HVGR_IOCTL_CMD_MOD_QUEUE_PRIV     (HVGR_IOCTL_SCH_SUB_MODULE_SCHED + 9)

union hvgr_ioctl_para_alloc_cq {
	struct {
		uint32_t priority;
		uint32_t freq_level;
		uint32_t bind_softq;
		uint32_t all_entry_size;
	} in;
	struct {
		uint64_t sw_wr_context_addr;
		uint64_t gpu_status_addr;
		uint32_t queue_id;
		uint64_t cq_entry_addr;
	} out;
};

#define HVGR_IOCTL_ALLOC_CQ \
	_IOWR(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_ALLOC_CQ, union hvgr_ioctl_para_alloc_cq)

union hvgr_ioctl_para_free_cq {
	struct {
		uint32_t queue_id;
	} in;
};

#define HVGR_IOCTL_FREE_CQ \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_FREE_CQ, union hvgr_ioctl_para_free_cq)

union hvgr_ioctl_para_submit_cq {
	struct {
		uint32_t queue_id;
	} in;
};

#define HVGR_IOCTL_SUBMIT_CQ \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_SUBMIT_CQ, union hvgr_ioctl_para_submit_cq)

union hvgr_ioctl_para_alloc_semp {
	struct {
		uint32_t size;
	} in;
	struct {
		uint64_t semaphore_addr;
	} out;
};

#define HVGR_IOCTL_ALLOC_SEMP \
	_IOWR(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_ALLOC_SEMP, union hvgr_ioctl_para_alloc_semp)

union hvgr_ioctl_para_free_semp {
	struct {
		uint32_t size;
	} in;
};

#define HVGR_IOCTL_FREE_SEMP \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_FREE_SEMP, union hvgr_ioctl_para_free_semp)

union hvgr_ioctl_cq_clear_err {
	struct {
		uint32_t queue_id;
	} in;
};

#define HVGR_IOCTL_CQ_CLEAR_ERR \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_CLEAR_ERR, union hvgr_ioctl_cq_clear_err)

union hvgr_ioctl_cq_record_token {
	struct {
		uint32_t token;
	} in;
};

#define HVGR_IOCTL_CQ_RECORD_TOKEN \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_RECORD_TOKEN, union hvgr_ioctl_cq_record_token)

union hvgr_ioctl_para_submit_cq_kmd_info {
	struct {
		uint32_t queue_id;
		uint32_t own_token;
		uint32_t dep_token_cnt;
		uint32_t entry_cnt;
		uint32_t sub_chn;
		uint64_t own_semp_addr;
		uint64_t own_semp_value;
		uint64_t dep_token_addr;
		uint64_t entry_addr;
	} in;
};

#define HVGR_IOCTL_SUBMIT_CQ_TO_KMD \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_SUBMIT_CQ_TO_KMD, \
	union hvgr_ioctl_para_submit_cq_kmd_info)

union hvgr_ioctl_para_init_entry_addr {
	struct {
		uint32_t queue_id;
	} in;
};

#define HVGR_IOCTL_INIT_ENTRY_ADDR \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_INIT_ENTRY_ADDR, \
	union hvgr_ioctl_para_init_entry_addr)

union hvgr_ioctl_para_mod_queue_priv {
	struct {
		uint32_t queue_id;
	} in;
};

#define HVGR_IOCTL_MOD_QUEUE_PRIV \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_MOD_QUEUE_PRIV, union hvgr_ioctl_para_mod_queue_priv)
#endif
