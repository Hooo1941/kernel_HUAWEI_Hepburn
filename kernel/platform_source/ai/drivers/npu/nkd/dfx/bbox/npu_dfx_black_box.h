/*
 * npu_dfx_black_box.h
 *
 * about npu dfx black box
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __NPU_BLACK_BOX_H
#define __NPU_BLACK_BOX_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/slab.h>

#include <platform_include/basicplatform/linux/rdr_pub.h>
#include "npu_common.h"
#include "npu_log.h"
#include "npu_user_common.h"

/* AI DRD */
#define NPU_BUF_LEN_MAX  256
#define NPU_BUF_TSCH_LEN 0x1000

/* SMMU EXCP BUF */
#define SMMU_EXCP_INFO_BUF_LEN  1024

/* TSCPU : 256K, include 1.Aicore: 128K, 2.LiteOS: 16K, 3.TaskScheduler: 64K */
#define NPU_BBOX_MEM_MAX     0x40000 /* 256K */

#define BBOX_ADDR_LEN  0x40000 /* 256K */

/* The address provided to a stage of translation failed */
#define SMMU_ADDR_TRANSLATION_FAULT  0x10
#define SMMU_BAD_CD_FAULT   0xa /* NPU_S_EXCEPTION:SMMU_EXCP */

#define TS_MODULE_NAME_LEN  16

/* noc target flow */
#define TARGET_FLOW_DEFAULT  0xff

typedef struct tag_excep_time {
	uint64_t tv_sec;
	uint64_t tv_usec;
} excep_time_t;

typedef struct tag_exce_info {
	excep_time_t time_stamp;
	uint32_t module_id;
	uint16_t dump_status; /* ts write & bbox frmwork read */
	uint16_t save_status; /* bbox frmwork write & ts read */
} exce_info_t;

typedef struct tag_exce_module_info {
	uint32_t magic;
	uint16_t is_valid;
	uint16_t exce_num;
	uint8_t module_name[TS_MODULE_NAME_LEN];
	exce_info_t current_info;
	uint32_t exp_message_offset;
	uint32_t exp_message_len; /* record exception message len */
	uint32_t buffer_offset;
	/* max bbox message len,
	 * equal to DEVDRV_NPU_BBOX_MEM_MAX - sizeof(exce_module_info_t)
	 */
	uint32_t buffer_len;
} exce_module_info_t;

enum rdr_npu_system_error_type {
	EXC_TYPE_NO_ERROR = 0x0,
	/* DFX_BB_MOD_NPU_START is 0xc0000000 */
	MODID_NPU_START = DFX_BB_MOD_NPU_START,

	/* OS exception code 0xc0000000-0xc00000ff */
	EXC_TYPE_OS_DATA_ABORT = MODID_NPU_START,
	EXC_TYPE_OS_INSTRUCTION_ABORT = 0xc0000001,
	EXC_TYPE_OS_PC_ALIGN_FAULT = 0xc0000002,
	EXC_TYPE_OS_SP_ALIGN_FAULT = 0xc0000003,
	EXC_TYPE_OS_INFINITE_LOOP = 0xc0000004,
	EXC_TYPE_OS_UNKNOWN_EXCEPTION = 0xc0000005,
	RDR_EXC_TYPE_OS_EXCEPTION = 0xc0000006,

	/* AICORE exception code 0xc0000200-0xc000023f */
	EXC_TYPE_TS_AICORE_EXCEPTION = 0xc0000200,
	EXC_TYPE_TS_AICORE_TIMEOUT = 0xc0000201,

	/* EFF AICORE exception code 0xc0000240-0xc000027f */
	EXC_TYPE_TS_AICORE_EFF_EXCEPTION = 0xc0000240,
	EXC_TYPE_TS_AICORE_EFF_TIMEOUT = 0xc0000241,

	/* AIV exception code 0xc0000280-0xc00002ff */
	EXC_TYPE_TS_AIV_EXCEPTION = 0xc0000280,
	EXC_TYPE_TS_AIV_TIMEOUT = 0xc0000281,

	/* SDMA exception code 0xc0000300-0xc00003ff */
	EXC_TYPE_TS_SDMA_EXCEPTION = 0xc0000300,
	EXC_TYPE_TS_SDMA_TIMEOUT = 0xc0000301,

	/* TS exception code 0xc0000400-0xc00004ff */
	RDR_EXC_TYPE_TS_RUNNING_EXCEPTION = 0xc0000400,
	RDR_EXC_TYPE_TS_RUNNING_TIMEOUT = 0xc0000401,
	RDR_EXC_TYPE_TS_INIT_EXCEPTION = 0xc0000402,

	/* driver exception code 0xc0000600-0xc00006ff, only for driver */
	RDR_EXC_TYPE_NPU_POWERUP_FAIL = 0xc0000600,
	RDR_EXC_TYPE_NPU_POWERDOWN_FAIL = 0xc0000601,
	RDR_EXC_TYPE_NPU_SMMU_EXCEPTION = 0xc0000602,

	/* RDR_EXC_TYPE_NOC_NPU need reset process range 0xc0000700-0xc000077f */
	RDR_EXC_TYPE_NOC_NPU0 = 0xc0000700,
	RDR_EXC_TYPE_NOC_NPU1 = 0xc0000701,
	RDR_EXC_TYPE_NOC_NPU2 = 0xc0000702,
	RDR_EXC_TYPE_NOC_NPU3 = 0xc0000703,
	RDR_EXC_TYPE_NOC_NPU4 = 0xc0000704,
	RDR_EXC_TYPE_NOC_NPU5 = 0xc0000705,
	RDR_EXC_TYPE_NOC_NPU6 = 0xc0000706,
	RDR_EXC_TYPE_NOC_NPU7 = 0xc0000707,

	/* RDR_EXC_TYPE_NOC_NORESET_NPU no need reset process range 0xc0000780-0xc00007ff */
	RDR_EXC_TYPE_NOC_NORESET_NPU0 = 0xc0000780,
	RDR_EXC_TYPE_NOC_NORESET_NPU1 = 0xc0000781,
	RDR_EXC_TYPE_NOC_NORESET_NPU2 = 0xc0000782,

	/* HWTS exception code 0xc0000800-0xc00008ff */
	EXC_TYPE_HWTS_ERROR_START = 0xc0000800,
	RDR_TYPE_HWTS_BUS_ERROR = EXC_TYPE_HWTS_ERROR_START,
	RDR_TYPE_HWTS_EXCEPTION_ERROR = 0xc0000801,
	EXC_TYPE_HWTS_ERROR_END = 0xc00008ff,

	/* DFX_BB_MOD_NPU_END is 0xc0000fff */
	MODID_NPU_EXC_END = DFX_BB_MOD_NPU_END,

	DMD_EXC_TYPE_EXCEPTION_START = 0xc0001000, /* for lite DMD */
	DMD_EXC_TYPE_EXCEPTION_END = 0xc000101f /* for lite DMD */
};

typedef enum {
	NPU_CORE_0,
	NPU_CORE_1,
	MAX_SUPPORT_CORE_NUM
} npu_core_id_e;

struct list_head_rdr {
	struct list_head_rdr *next, *prev;
};

struct aicpu_excep_time_t {
	u64 tv_sec;
	u64 tv_usec;
};

struct exc_info_s {
	struct aicpu_excep_time_t e_clock;
	u32 e_excepid;
	u16 e_dump_status;
	u16 e_save_status;
};

struct exc_description_s {
	u32 e_modid;
	u8 e_process_level;
	u8 e_reboot_priority;
	u8 e_exce_type;
	u8 e_reentrant;
	u64 e_notify_core_mask;
	u8 e_desc[48];
};

struct exc_module_info_s {
	u32 magic;
	u16 e_mod_valid;
	u16 e_mod_num;
	u8 e_from_module[16];
	struct exc_info_s cur_info;
	u32 e_mini_offset;
	u32 e_mini_len;
	u32 e_info_offset;
	u32 e_info_len;
	struct exc_description_s e_description[0];
};

struct npu_dump_info_s {
	u32 modid;
	u64 coreid;
	pfn_cb_dump_done cb;
	char *pathname;
};

struct npu_mntn_info_s {
	unsigned int bbox_addr_offset;
	struct rdr_register_module_result npu_ret_info;
	struct npu_dump_info_s dump_info;
	void *rdr_addr;
};

struct npu_mntn_private_s {
	unsigned int core_id;
	struct npu_mntn_info_s mntn_info;
	struct work_struct dump_work;
	struct work_struct reset_work;
	struct workqueue_struct *rdr_wq;
};

struct rdr_exception_info {
	u32 error_code;
	u16 limitation;
	u16 count;
};

struct npu_dump_offset {
	unsigned int offset;
	const char *desc;
};

struct npu_dump_reg {
	unsigned long base;
	unsigned int range;
	struct npu_dump_offset *regs;
	unsigned int reg_num;
};

struct npu_qic_register_info_s {
	u32 mid_start;
	u32 mid_end;
	u32 qic_bus;
	u32 modid;
};

int npu_mntn_copy_reg_to_bbox(const char *src_addr, unsigned int len);

int npu_black_box_init(void);

int npu_black_box_exit(void);

void npu_rdr_exception_reset(void);

int npu_rdr_resource_init(void);

int npu_rdr_register_exception(void);

int npu_rdr_register_core(void);

void npu_rdr_exception_report(uint32_t error_code);

int npu_rdr_addr_map(void);

int npu_bus_errprobe_register(void);

int npu_blackbox_addr_release(void);

void npu_bbox_record_err_task(struct npu_task_dump_info *error_task);

#endif /* __NPU_BLACK_BOX_H */
