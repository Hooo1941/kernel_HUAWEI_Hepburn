/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_IOCTL_DATAN_H
#define HVGR_IOCTL_DATAN_H

#include "linux/types.h"
#include "apicmd/hvgr_ioctl.h"

#define HVGR_DATAN_JOBTRACE_CMD_ENABLE (1u)
#define HVGR_DATAN_JOBTRACE_CMD_DISABLE (2u)

#define HVGR_CT_ALLOC_CMD (3u)

#define HVGR_CT_SINGLE_DUMP_ENABLE_CMD (4u)
#define HVGR_CT_SINGLE_DUMP_DATA_CMD (5u)
#define HVGR_CT_SINGLE_DUMP_CLEAR_CMD (6u)

#define HVGR_IOCTL_CMD_TIMELINE_SUBMIT (7u)
#define HVGR_IOCTL_CMD_TIMELINE_SET (8u)
#define HVGR_IOCTL_CMD_TIMELINE_GET (9u)
#define HVGR_IOCTL_CMD_TIMELINE_GET_CHAIN_ID_BUFFER (10u)

#define DATAN_PATH_MAX_LEN (256)
#define DATAN_JOBTRACE_MAX_LEN (1<<12)

struct hvgr_datan_jobtrace_para_enable {
	struct {
		char outpath[DATAN_PATH_MAX_LEN];
		int32_t entry_size;
		int32_t softq_id;
	} in;
	struct {
		int64_t hvgr_ctx;
	} out;
};

struct hvgr_datan_jobtrace_para_disable {
	struct {
		int32_t softq_id;
	} in;
};

// =======================================================
#define HVGR_CT_DUMP_SIZE 2048U /* 8* 64 * 4 */
struct hvgr_ct_queue_user_ctrl { // UMD can modify.
	uint32_t rd; /* Read index */
};

struct hvgr_ct_queue_data_ctrl {
	uint64_t timestamp;
	uint64_t event_id;
	char data[HVGR_CT_DUMP_SIZE];
};

struct hvgr_ct_queue_ctrl { // NOT KMD, must not modify.
	/* ct_raw_base point to hvgr_ct_queue_data_ctrl */
	uint64_t ct_raw_base;

	/* user_ctrl_addr point to hvgr_ct_queue_user_ctrl */
	uint64_t user_ctrl_addr;

	uint32_t wr; /* Write index */
};

union hvgr_ioctl_ct_alloc {
	struct {
		uint32_t ring_num; // Ring Buffer num
		uint32_t fcm_bm;
		uint32_t gpc_bm;
		uint32_t btc_bm;
		uint32_t mmu_l2_bm;
		uint32_t bvh_bm;
		uint32_t set_id;
	} in;
	struct {
		int fd;
		/* Return hvgr_ct_queue_ctrl address. */
		uint64_t ctrl_addr;
	} out;
};
// =======================================================

struct hvgr_ioctl_ct_single_dump_enable {
	uint64_t dump_buffer;
	uint32_t fcm_bm;
	uint32_t gpc_bm;
	uint32_t btc_bm;
	uint32_t mmu_l2_bm;
	uint32_t bvh_bm;
	uint32_t set_id;
};

#define HVGR_IOCTL_CT_ENABLE \
	_IOW(HVGR_IOCTL_MODULE_DATAN, HVGR_CT_SINGLE_DUMP_ENABLE_CMD, \
		struct hvgr_ioctl_ct_single_dump_enable)

#define HVGR_IOCTL_CT_DUMP \
	_IO(HVGR_IOCTL_MODULE_DATAN, HVGR_CT_SINGLE_DUMP_DATA_CMD)

#define HVGR_DATAN_JOBTRACE_ENABLE \
	_IOWR(HVGR_IOCTL_MODULE_DATAN, HVGR_DATAN_JOBTRACE_CMD_ENABLE, \
		struct hvgr_datan_jobtrace_para_enable)

#define HVGR_DATAN_JOBTRACE_DISABLE \
	_IOW(HVGR_IOCTL_MODULE_DATAN, HVGR_DATAN_JOBTRACE_CMD_DISABLE, \
		struct hvgr_datan_jobtrace_para_disable)

#define HVGR_IOCTL_CT_ALLOC \
	_IOW(HVGR_IOCTL_MODULE_DATAN, HVGR_CT_ALLOC_CMD, union hvgr_ioctl_ct_alloc)

typedef enum {
	JOB_TIMELINE_DEP_INFO = 0,
	JOB_TIMELINE_SPECIAL_NOTIFY,
	JOB_TIMELINE_PHYCHN_STATE,
	JOB_TIMELINE_SVD,
	JOB_TIMELINE_FENCE,
	JOB_TIMELINE_VK_SUBMIT,
	JOB_TIMELINE_CMD_BUF_BUILD,
	JOB_TIMELINE_END,
} job_timeline_msg;

#define HVGR_DATAN_MAX_JOB_TYPE              3
#define HVGR_DATAN_MAX_DEPS_COUNT            50

#define HVGR_DATAN_TIMLINE_ENABLE            1
#define HVGR_DATAN_TIMLINE_DISABLE           0

struct hvgr_ioctl_timeline_dep_info {
	uint32_t token;
	uint32_t token_id;
	uint32_t queue_id;
	uint32_t chain_type;
	uint16_t job_cnt[HVGR_DATAN_MAX_JOB_TYPE];
	uint64_t begin_chain_build_timestamp;
	uint64_t end_chain_build_timestamp;
	uint64_t end_cmar_sched_timestamp;
	uint16_t *deps_list;
	uint16_t deps_num;
};

struct hvgr_ioctl_timeline_special_notify {
	uint32_t token_id;
	uint32_t queue_id;
	uint16_t deps_num;
	uint16_t *deps_list;
	uint64_t end_cmar_sched_timestamp;
};

struct hvgr_ioctl_timeline_vk_submit {
	uint64_t submit_begin_time;
	uint64_t submit_end_time;
};

struct hvgr_ioctl_timeline_cmd_buf_build {
	uint64_t cmd_buf_begin_time;
	uint64_t cmd_buf_end_time;
};

struct hvgr_ioctl_timelime_submit {
	job_timeline_msg msg_type;
	union {
		struct hvgr_ioctl_timeline_dep_info dep_info;
		struct hvgr_ioctl_timeline_special_notify speical_notify;
		struct hvgr_ioctl_timeline_vk_submit vk_submit;
		struct hvgr_ioctl_timeline_cmd_buf_build cmd_buf_build;
	} timeline_msg;
};

union kmd_ioctl_timeline_set {
	struct {
		uint8_t set_flag;
	} in;
};

union kmd_ioctl_timeline_get {
	struct {
		uint32_t size;
		uint64_t addr;
	} in;
	struct {
		uint32_t msg_num;
	} out;
};

struct kmd_ioctl_timeline_buf {
	uint32_t chain_buf_size;
	uint64_t chain_buf_addr;
	uint32_t start_buf_size;
	uint64_t start_buf_addr;
	uint32_t end_buf_size;
	uint64_t end_buf_addr;
};

#define HVGR_IOCTL_TIMELINE_SUBMIT \
	_IOW(HVGR_IOCTL_MODULE_DATAN, HVGR_IOCTL_CMD_TIMELINE_SUBMIT, \
		struct hvgr_ioctl_timelime_submit)

#define HVGR_IOCTL_TIMELINE_SET \
	_IOW(HVGR_IOCTL_MODULE_DATAN, HVGR_IOCTL_CMD_TIMELINE_SET, union kmd_ioctl_timeline_set)

#define HVGR_IOCTL_TIMELINE_GET \
	_IOWR(HVGR_IOCTL_MODULE_DATAN, HVGR_IOCTL_CMD_TIMELINE_GET, union kmd_ioctl_timeline_get)

#define HVGR_IOCTL_TIMELINE_GET_CHAIN_ID_BUF \
	_IOR(HVGR_IOCTL_MODULE_DATAN, HVGR_IOCTL_CMD_TIMELINE_GET_CHAIN_ID_BUFFER, \
		struct kmd_ioctl_timeline_buf)

#endif

