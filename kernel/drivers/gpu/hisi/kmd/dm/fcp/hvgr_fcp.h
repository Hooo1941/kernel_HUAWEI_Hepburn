/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_FCP_H
#define HVGR_FCP_H

#include <linux/completion.h>

#include "hvgr_defs.h"
#include "hvgr_regmap.h"

/* used for upgrading fcp image */
struct hvg_fcp_header {
	/* magic number, here we use 0x6a6d6370 fcp */
	u32 magic;
	/*
	 * version, when we want to upgrade the fcp firmware,
	 * we must add this version
	 */
	u32 version;
	u16 crc;
	/* arc firmware itcm data size */
	u16 size_sub0;
	/* arc firmware dtcm data size */
	u32 size_sub1;
	/* point to dst vaild firmware data */
	unsigned char data[0];
};

struct fcp_dma_cfg {
	/* source addr */
	u64 src_addr;
	/* dest addr */
	u32 dst_addr;
	u32 size;
	/* as_nr */
	u32 jasid;
	u32 channel;
};

struct hvgr_fcp_data {
	bool fcp_firmware_ready; /* requested from system */
	bool load_entryed;
	/* update by dma */
	struct hvgr_ctx *ctx;
	struct hvgr_mem_area *area;
	struct completion complete;
	u64 gva;
	void *kva;
	u32 firmware_size;
	struct task_struct *thread;
};

/* the name of FCP firmware */
#define HVGR_FIRMWARE_NAME_LEN            32
#define FCP_ARC_FIRMWARE_NAME             "jmcp_arc.bin"
#define FCP_M7_FIRMWARE_NAME              "jmcp_m7.bin"
#define FCP_M7_TV110_FIRMWARE_NAME        "jmcp_m7_tv110.bin"
#define FCP_RISCV_FIRMWARE_NAME           "jmcp_riscv_cf.bin"
#define FCP_RISCV_TV210_FIRMWARE_NAME     "jmcp_riscv_cf_tv210.bin"
#define FCP_RISCV_TV350_FIRMWARE_NAME     "jmcp_riscv_cf_tv350.bin"

/* the max size of firmware bin */
#if hvgr_version_ge(HVGR_V350)
#define FCP_MAX_SIZE_FIRMWARE      (24 * 1024U)
#else
#define FCP_MAX_SIZE_FIRMWARE      (16 * 1024U)
#endif
#define FCP_SIZE_IN_PAGE (FCP_MAX_SIZE_FIRMWARE >> 12)

/* the firmware magic number */
#define HVGR_FCP_FIRMWARE_MAGIC    0x6a6d6370U

#define HVGR_MAX_TRY_TIMES          0x1000U
#define HVGR_DMA_MAX_TRY_TIMES      0x10000U

/* fcp loader task name */
#define FCP_LOADER_TASK_NAME       "fcp_loader"

/* fcp upgrade use dma channel */
#define FCP_DMA_CH_14               14U
#define FCP_DMA_CH_15               15U
#define FCP_DMA_CH_26               26U
#define FCP_DMA_CH_27               27U
#define FCP_DMA_CH_28               28U
#define FCP_DMA_CH_29               29U
#define FCP_DMA_CH_30               30U
#define FCP_DMA_CH_31               31U

/* fcp upgrade dma job type, tv100 dma_only job, tv120 const folding job */
#define FCP_DMA_ONLY_JOB            0x100U
#define FCP_CONST_FOLDING_JOB       0x0U

/* fcp upgrade dma channel config */
#define FCP_UPGRADE_DMA_CFG         0xC7733001U

/* fcp upgrade dma completed bit */
#if hvgr_version_ge(HVGR_V300)
#define FCP_UPGRADE_DMA_JASID       0U
#else
#define FCP_UPGRADE_DMA_JASID       7U
#endif

#define FCP_UPGRADE_ITCM_ADDR       0x0U

#define FCP_MB_EDMA2ARC_MASK_TV120  0x0FFFU
#define FCP_MB_EDMA2HOST_MASK_TV120 0xFFFFU
#define FCP_MB_EDMA2JM_MASK_TV120   0xF000U

#define FCP_MB_EDMA2ARC_MASK        0xFFFFU
#define FCP_MB_EDMA2HOST_MASK       0xFFFFU
#define FCP_MB_EDMA2JM_MASK         0x0U

#define FCP_JM_CONFIG2_DEAULT       0x2000U
#define FCP_JM_CONFIG3_DEAULT       0x1000U

/* fcp mcu tcm ready bit */
#define FCP_MCU_TCM_READY_BIT       1U

/* fcp mcu ahbs ready bit */
#define FCP_MCU_AHBS_READY_BIT      1U

/*
 * hvgr_fcp_loader_init - fcp load task init
 *
 * @gdev:    The hvgr device
 *
 * Return: 0 on success, -1 on fail
 */
int hvgr_fcp_loader_init(struct hvgr_device *gdev);

/*
 * hvgr_fcp_loader_entry - Trigger load fcp firmware
 *
 * @gdev:    The hvgr device
 */
void hvgr_fcp_loader_entry(struct hvgr_device *gdev);

/*
 * hvgr_fcp_loader_term - Release fcp load resource
 *
 * @gdev:    The hvgr device
 */
void hvgr_fcp_loader_term(struct hvgr_device *gdev);

bool hvgr_fcp_is_dev_ready(struct hvgr_device *gdev);

int hvgr_fcp_wait_boot_done(struct hvgr_device *gdev);

void hvgr_fcp_store_dma_cfg(struct hvgr_device *gdev, bool interupt);

void hvgr_fcp_recover_dma_cfg(struct hvgr_device *gdev);

void hvgr_fcp_upgrade_start_mcu(struct hvgr_device *gdev);

int hvgr_fcp_dma_proc(struct hvgr_device *gdev);

void hvgr_fcp_configure_dma(struct hvgr_device *gdev,
	const struct fcp_dma_cfg *dma_cfg);

bool hvgr_fcp_is_load_needed(struct hvgr_device *gdev);

int hvgr_fcp_wait_dma_finish(struct hvgr_device *gdev, u32 channel);

#endif /* HVGR_FCP_H */
