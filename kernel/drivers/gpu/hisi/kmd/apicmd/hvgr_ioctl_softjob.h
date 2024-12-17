/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_IOCTL_SOFTJOB_H
#define HVGR_IOCTL_SOFTJOB_H

#include "linux/types.h"
#include "apicmd/hvgr_ioctl.h"

#define HVGR_IOCTL_CMD_WAIT_MSYNC         (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 0)
#define HVGR_IOCTL_CMD_SET_MSYNC_NOTIFY   (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 1)
#define HVGR_IOCTL_CMD_CREATE_FENCE_FD    (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 2)
#define HVGR_IOCTL_CMD_WAIT_FENCE_FD      (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 3)
#define HVGR_IOCTL_CMD_TRIGGER_FENCE_FD   (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 4)
#define HVGR_IOCTL_CMD_BIND_SOFTQ         (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 5)
#define HVGR_IOCTL_CMD_TERM_SOFTQ         (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 6)
#define HVGR_IOCTL_CMD_SET_FD             (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 7)
#define HVGR_IOCTL_CMD_GET_FD             (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 8)
#define HVGR_IOCTL_CMD_FENCE_VALIDATE     (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 9)
#define HVGR_IOCTL_CMD_SC_UPDATE_POLICY   (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 10)
#define HVGR_IOCTL_CMD_SET_HOLD_EN_INFO   (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 11)
#define HVGR_IOCTL_CMD_SET_FREQ_HINT      (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 12)
#define HVGR_IOCTL_CMD_SET_CL_PROFILING   (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 13)
#define HVGR_IOCTL_CMD_SET_TOP_NOTIFY     (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 14)
#define HVGR_IOCTL_CMD_CLEAR_CL_FLAGS     (HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB + 15)

#define HVGR_TERM_STAGE_MESSAGE 0
#define HVGR_TERM_STAGE_CLOSED  1

enum msync_result {
	MSYNC_RESULT_SUCCESSFUL = 0,
	MSYNC_RESULT_PARA_INVALID,
	MSYNC_RESULT_TIMEOUT,
};

struct hvgr_ioctl_msync_data {
	uint64_t sem_addr;
	uint64_t sem_value;
	uint32_t token;
	int fd;
};

union hvgr_ioctl_para_msync_fd {
	struct {
		uint64_t sem_addr;
		uint64_t sem_value;
		uint32_t token;
		int fd;
		bool ai_freq_enabled;
		char res[3];
	} in;
	struct {
		int fd;
	} out;
};

#define HVGR_IOCTL_MSYNC_SET_FD \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_SET_FD, union hvgr_ioctl_para_msync_fd)

#define HVGR_IOCTL_MSYNC_GET_FD \
	_IOWR(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_GET_FD, union hvgr_ioctl_para_msync_fd)

union hvgr_ioctl_para_bind_softq {
	struct {
		uint32_t softq_id;
	} in;
};

#define HVGR_IOCTL_BIND_SOFTQ \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_BIND_SOFTQ, union hvgr_ioctl_para_bind_softq)

union hvgr_ioctl_para_term_softq {
	struct {
		uint32_t stage;
	} in;
};

#define HVGR_IOCTL_TERM_SOFTQ \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_TERM_SOFTQ, union hvgr_ioctl_para_term_softq)

union hvgr_ioctl_para_create_fence_fd {
	struct {
		int fd;
	} out;
};

#define HVGR_IOCTL_CREATE_FENCE_FD \
	_IOR(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_CREATE_FENCE_FD, \
	union hvgr_ioctl_para_create_fence_fd)

union hvgr_ioctl_para_wait_fence_fd {
	struct {
		uint64_t semaphore_addr;
		uint64_t semaphore_value;
		uint32_t token;
		int fd;
	} in;
};

#define HVGR_IOCTL_WAIT_FENCE_FD \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_WAIT_FENCE_FD, \
	union hvgr_ioctl_para_wait_fence_fd)

union hvgr_ioctl_para_trigger_fence_fd {
	struct {
		uint64_t wait_semp_addr;
		uint64_t wait_semp_value;
		uint32_t wait_token;
		uint32_t token;
		bool ai_freq_enabled;
		char res[3];
	} in;
	struct {
		int fd;
	} out;
};

#define HVGR_IOCTL_TRIGGER_FENCE_FD \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_TRIGGER_FENCE_FD, \
	union hvgr_ioctl_para_trigger_fence_fd)

/**
 * struct kmd_ioctl_fence_validate - Validate a fd refers to a fence
 * @fd: The file descriptor to validate
 */
struct hvgr_ioctl_fence_validate {
	int fd;
};

#define HVGR_IOCTL_FENCE_VALIDATE \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_FENCE_VALIDATE, struct hvgr_ioctl_fence_validate)

union hvgr_ioctl_sc_update_policy {
	struct {
		uint64_t wait_semp_addr;
		uint64_t wait_semp_value;
		uint64_t policy_info;
		uint64_t semaphore_addr;
		uint64_t semaphore_value;
		uint32_t info_num;
		uint32_t wait_token;
		uint32_t token;
	} in;
};

#define HVGR_IOCTL_CQ_SC_UPDATE_POLICY \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_SC_UPDATE_POLICY, \
	union hvgr_ioctl_sc_update_policy)
union hvgr_ioctl_para_hold_en_info {
	struct {
		uint32_t hold_en_token;
		uint32_t protect_flag;
	} in;
};

#define HVGR_IOCTL_SET_HOLD_EN_INFO \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_SET_HOLD_EN_INFO, \
	union hvgr_ioctl_para_hold_en_info)

union hvgr_ioctl_para_set_freq_hint {
	struct {
		uint32_t freq_hint;  /* HIGH, MED,LOW */
		uint32_t token_start;
		uint32_t token_end;
	} in;
};

#define HVGR_IOCTL_CQ_SET_FREQ_HINT \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_SET_FREQ_HINT, \
	union hvgr_ioctl_para_set_freq_hint)

union hvgr_ioctl_para_set_cl_profiling {
	struct {
		uint64_t exec_timestamp_addr;
		uint64_t complete_timestamp_addr;
		uint64_t wait_semp_addr;
		uint64_t wait_semp_value;
		uint32_t exec_token;
		uint32_t complete_token;
	} in;
};

#define HVGR_IOCTL_CQ_SET_CL_PROFILING \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_SET_CL_PROFILING, \
	union hvgr_ioctl_para_set_cl_profiling)

union hvgr_ioctl_para_clear_cl_flags {
	struct {
		uint32_t token;
	} in;
};

#define HVGR_IOCTL_CQ_CLEAR_CL_FLAGS \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_CLEAR_CL_FLAGS, \
	union hvgr_ioctl_para_clear_cl_flags)

union hvgr_ioctl_para_top_notify {
	struct {
		uint32_t notify_token;
	} in;
};

#define HVGR_IOCTL_SET_TOP_NOTIFY \
	_IOW(HVGR_IOCTL_MODULE_SCH, HVGR_IOCTL_CMD_SET_TOP_NOTIFY, \
	union hvgr_ioctl_para_top_notify)

#endif
