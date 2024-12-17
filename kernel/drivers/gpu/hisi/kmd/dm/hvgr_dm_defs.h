/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DM_DEFS_H
#define HVGR_DM_DEFS_H

#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/mutex.h>
#ifdef CONFIG_HVGR_DFX_DMD_REPORT
#include <linux/pm_wakeup.h>
#endif
#include "hvgr_dm_driver_adapt_defs.h"
#include "hvgr_ioctl_dev.h"
#include "hvgr_version.h"

struct hvgr_fcp_data;

struct hvgr_reg_info {
	void __iomem *reg_base;
	uint32_t gpu_id;
	uint32_t gpu_version;
	uint32_t l2_features;
	uint32_t gpc_present;
	uint32_t mmu_features;
	uint32_t thread_features;
	uint32_t thread_max_workgroup_size;
};

struct hvgr_fcp_register_base {
	uint64_t fcp_reg_base;
	uint64_t fcp_itcm_offset;
	uint64_t fcp_dtcm_offset;
	uint64_t fcp_itcm_cq_offset;
	uint64_t fcp_dtcm0_cq_offset;
	uint64_t fcp_dtcm1_cq_offset;
#if hvgr_version_ge(HVGR_V350)
	uint64_t fcp_reg_mb0_offset;
	uint64_t fcp_reg_mb1_offset;
#else
	uint64_t fcp_reg_mb_offset;
#endif
	uint64_t fcp_reg_dma_offset;
	uint64_t fcp_reg_timer_offset;
	uint64_t fcp_reg_uart_offset;
};

struct hvgr_register_module_base {
	uint64_t gpu_reg_base;
	uint64_t job_reg_base;
	uint64_t mmu_stage1_reg_base;
	uint64_t cq_reg_base;
	struct hvgr_fcp_register_base fcp_base;
};

#define HVGR_CRG_REG_PARA_NUM 6u

struct hvgr_crg_resource {
	uint32_t crg_start;
	uint32_t crg_size;
	/* reset enable */
	uint32_t rst_addr;
	uint32_t rst_val;
	/* reset disable */
	uint32_t rstdis_addr;
	uint32_t rstdis_val;
	void __iomem *reg_base;
};

#define HVGR_CTRL_REG_PARA_NUM 2u
#define HVGR_SCS_REG_PARA_NUM 2u

struct hvgr_ctrl_resource {
	uint32_t reg_start;
	uint32_t reg_size;
	void __iomem *reg_base;
};

struct hvgr_scs_resource {
	uint32_t reg_start;
	uint32_t reg_size;
	void __iomem *reg_base;
};

#define IRQ_REGISTER_DONE 1

struct hvgr_irq {
	u32 irq;  /* irq num */
	int reg_done;
	/*
	 * irq flag include: trigger mode,resource type
	 * ex: IORESOURCE_IRQ | IRQF_TRIGGER_HIGH
	 */
	unsigned long flags;
};

#define MAX_DMD_LOG_LENTH  160

#ifdef CONFIG_HVGR_DFX_DMD_REPORT
#define MAX_DMD_CHECK_CNT 4u
#define MAX_DMD_CHECK_INTERVAL 1209600UL
struct gpu_dmd_msg {
	unsigned int dmd_id;
	struct mutex dmd_lock;
	atomic_t dmd_flag;
#ifdef CONFIG_HVGR_DFX_SH
	u32 dmd_check_cnt;
	u32 dmd_sh_flag;
	u64 dmd_check_ts[MAX_DMD_CHECK_CNT];
	struct wakeup_source *sr_wakeup_lock;
	atomic_t dmd_gaf;
#endif
	char dmd_log[MAX_DMD_LOG_LENTH];
};
#endif

struct hvgr_dm_dev {
	struct hvgr_irq irqs[GPU_IRQ_CNT];
	struct hvgr_register_module_base reg_base;
	/* Register access data struct. */
	struct hvgr_reg_info dev_reg;
	struct hvgr_crg_resource crg_rst;
	struct hvgr_ctrl_resource ctrl_reg;
	struct hvgr_scs_resource scs_reg;
	/* fcp data struct */
	struct hvgr_fcp_data *fcp_data;
	struct list_head ctx_list;
	struct mutex ctx_list_mutex;
	u64 uio_reg_start;
#ifdef CONFIG_HVGR_DFX_DMD_REPORT
	struct gpu_dmd_msg *dmd_msg;
#endif
#ifdef CONFIG_HVGR_VIRTUAL_GUEST
	int report_irq;
	struct workqueue_struct *ecc_wq;
	struct work_struct ecc_work;
#endif
};

#ifdef CONFIG_HVGR_DFX_SH
#define HVGR_DMD_NV_DATA_SIZE 52

struct hvgr_dmd_nv_data {
	union {
		uint8_t value;
		struct {
			uint8_t en       : 1;
			uint8_t sh_mark  : 1;
			uint8_t gaf_mark : 1;
			uint8_t res      : 1;
			uint8_t version  : 4;
		} bit;
	} info;

	uint8_t buf[HVGR_DMD_NV_DATA_SIZE];
};
#endif

#endif
