/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_IOCTL_DEV_H
#define HVGR_IOCTL_DEV_H

#include "linux/types.h"
#include "apicmd/hvgr_ioctl.h"

#define HVGR_IOCTL_CMD_GET_INFO        0u
#define HVGR_IOCTL_CMD_DRV_CFG         1u
#define HVGR_IOCTL_CMD_CREATE_SOFTQ    2u
#define HVGR_IOCTL_CMD_SOFTQ_ENQUEUE   3u
#define HVGR_IOCTL_CMD_GET_TIMESTAMP   4u
#define HVGR_IOCTL_CMD_DRV_DECFG       5u
#define HVGR_IOCTL_CMD_GET_MSB         6u

typedef struct {
	uint32_t gpu_id;
	uint32_t gpu_version;
	uint32_t l2_features;
	uint32_t gpc_present;
	uint32_t gpc_nums;
	uint32_t mmu_features;
	uint32_t thread_features;
	uint32_t thread_max_workgroup_size;
	uint64_t gpu_available_memory_size;
} hvgr_ioctl_para_version;

typedef struct {
	/* If mem_replay_size set to 0, means replay is not used. */
	uint64_t mem_replay_size;
	uint32_t jit_large_size;
	uint32_t jit_large_cnt;
	uint32_t jit_medium_size;
	uint32_t jit_medium_cnt;
	uint32_t jit_small_size;
	uint32_t jit_small_cnt;
	uint32_t jit_default_size;
	uint32_t wq_max_wait_cnt;
	uint32_t gaf_flag;
} hvgr_ioctl_para_drv_cfg;

#define HVGR_IOCTL_VERSION \
	_IOWR(HVGR_IOCTL_MODULE_DEV, HVGR_IOCTL_CMD_GET_INFO, hvgr_ioctl_para_version)

#define HVGR_IOCTL_CONFIGURE \
	_IOW(HVGR_IOCTL_MODULE_DEV, HVGR_IOCTL_CMD_DRV_CFG, hvgr_ioctl_para_drv_cfg)

#define HVGR_IOCTL_DECONFIGURE \
	_IOW(HVGR_IOCTL_MODULE_DEV, HVGR_IOCTL_CMD_DRV_DECFG, hvgr_ioctl_para_drv_cfg)

/* Define Soft Q direction */
#define HVGR_IOCTL_SOFTQ_K_TO_U 1
#define HVGR_IOCTL_SOFTQ_U_TO_K 2
typedef union {
	struct {
		uint32_t direct;
		uint32_t entry_size;
		uint32_t entry_nums;
	} in;
	struct {
		/* Return hvgr_softq_ctrl address. */
		uint64_t ctrl_addr;
	} out;
} hvgr_ioctl_para_create_softq;

#define HVGR_IOCTL_CREATE_SOFTQ \
	_IOWR(HVGR_IOCTL_MODULE_DEV, HVGR_IOCTL_CMD_CREATE_SOFTQ, hvgr_ioctl_para_create_softq)

typedef struct {
	uint32_t softq_id;
} hvgr_ioctl_para_softq_enqueue;

#define HVGR_IOCTL_SOFT_ENQUEUE \
	_IOW(HVGR_IOCTL_MODULE_DEV, HVGR_IOCTL_CMD_SOFTQ_ENQUEUE, hvgr_ioctl_para_softq_enqueue)

union hvgr_ioctl_para_timestamp {
	struct {
		uint64_t os_timestamp;
		uint64_t soc_timestamp;
		uint64_t gpu_timestamp;
	} out;
};

union hvgr_ioctl_para_msb {
	struct {
		uint32_t msb;
	} out;
};

#define HVGR_IOCTL_GET_MSB \
	_IOR(HVGR_IOCTL_MODULE_DEV, HVGR_IOCTL_CMD_GET_MSB, union hvgr_ioctl_para_msb)

#endif
