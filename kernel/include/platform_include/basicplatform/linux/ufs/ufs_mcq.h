/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * UFS Host driver for Synopsys Designware Core
 *
 * Authors: Joao Pinto <jpinto@synopsys.com>
 */

#ifndef _UFS_MCQ_H
#define _UFS_MCQ_H

#include "scsi/ufs/ufshcd.h"

#define DEFAULT_ACTIVE_CMDS 32
#define MCQ_CFG_MAC_OFFSET  8
#define MCQ_CFG_MAC_MASK  UFS_MASK(0x1FF, MCQ_CFG_MAC_OFFSET)
#define MCQ_QCFGPTR_UNIT  0x200
#define MCQ_QCFG_SIZE  0x40
#define QCFGPTR_OFFSET  16
#define QCFGPTR_MASK  0XFF0000
#define MCQ_SQ_ENTRY_SIZE_IN_DWORD	12
#define MCQ_CQ_ENTRY_SIZE_IN_DWORD	8
#define REG_MAP_SIZE 0x4
#define MQ_MAX_ENTRIES 33
#define QUEUE_EN_OFFSET 31
#define QUEUE_ID_OFFSET 16
#define REG_SQDAO_SIZE 0x14
#define REG_SQISAO_SIZE 0x8
#define REG_CQDAO_SIZE 0x8
#define REG_CQISAO_SIZE 0xC

#define MCQ_SQ_STOP_BIT 0x1
#define QUEUE_TYPE_SELECT (0x1 << 0)
#define HOST_CFG_OOO_EN (0x1 << 8)
#define MCQ_SQ_ENTRY_ABORT (0xf << 28)

#define CQTEPS_MSI_START 0x40
#define CQTEPS_MSI_END 0x5f

/* CQISy - CQ y Interrupt Status Register  */
#define UFSHCD_MCQ_CQIS_TEPS 0x1

#define UFS_PRDT_CONFIG 0x35D0
#define MSI_DEVICE_ID 0x3854

#define SQ_STOP_STATUS (0x1 << 1)
#define SQ_CLEAN_UP_STATUS_CUS (0x1 << 1)
#define SQ_CLEAN_UP_INTR_SUS (0x1 << 2)
#define INITIATE_CLEAN_UP_ICU (0x1 << 1)
#define CLEANUP_COMMAND_RETRUN_CODE 0xF0
#define UFS_TRP_DFX3 0x129C
#define TRP_DFX3_CLEAR_FINISH_REQ (0x1 << 8)
/* MCQ capability mask */
enum {
	MASK_EXT_IID_SUPPORT = 0x00000400,
	MASK_ROUND_ROBIN_PRI_SUPP = 0x00000200,
	MASK_HW_QUEUE_NUMBER = 0x000000FF
};

enum {
	REG_SQATTR		= 0x0,
	REG_SQLBA		= 0x4,
	REG_SQUBA		= 0x8,
	REG_SQDAO		= 0xC,
	REG_SQISAO		= 0x10,
	REG_SQCFG		= 0x14,

	REG_CQATTR		= 0x20,
	REG_CQLBA		= 0x24,
	REG_CQUBA		= 0x28,
	REG_CQDAO		= 0x2C,
	REG_CQISAO		= 0x30,
	REG_CQCFG		= 0x34,
};

enum {
	REG_SQHP		= 0x0,
	REG_SQTP		= 0x4,
	REG_SQRTC		= 0x8,
	REG_SQCTI		= 0xC,
	REG_SQRTS		= 0x10,
};

enum {
	REG_SQIS		= 0x0,
	REG_SQIE		= 0x4,
};

enum {
	REG_CQHP		= 0x0,
	REG_CQTP		= 0x4,
};

enum {
	REG_CQIS		= 0x0,
	REG_CQIE		= 0x4,
	REG_MCQIACR		= 0x8,
};

/**
 * @sqe_base_addr: submission queue entry base address
 * @sqe_dma_addr: submission queue dma address
 * @cqe_base_addr: completion queue base address
 * @cqe_dma_addr: completion queue dma address
 * @max_entries: max number of slots in this hardware queue
 * @id: hardware queue ID
 * @mcq_sq_hp: base address of submission queue head pointer
 * @mcq_sq_tp: base address of submission queue tail pointer
 * @mcq_cq_hp: base address of completion queue head pointer
 * @mcq_cq_tp: base address of completion queue tail pointer
 * @sq_lock: lock to protect submission queue head/tail pointer and slot update
 * @sq_tp_slot: current slot to which SQ tail pointer is pointing
 * @cq_lock: lock to protect completion queue head/tail pointer and slot update
 * @cq_tp_slot: current slot to which CQ tail pointer is pointing
 * @cq_hp_slot: current slot to which CQ head pointer is pointing
 */
struct ufs_hw_queue {
	void *sqe_base_addr;
	dma_addr_t sqe_dma_addr;
	struct cq_entry *cqe_base_addr;
	dma_addr_t cqe_dma_addr;
	u32 max_entries;
	u32 id;

	void __iomem *mcq_base;
	void __iomem *mcq_base_sqd;
	void __iomem *mcq_base_sqis;
	void __iomem *mcq_base_cqd;
	void __iomem *mcq_base_cqis;

	spinlock_t sq_lock;
	u32 sq_tp_slot;
	u32 sq_hp_slot;
	spinlock_t cq_lock;
	u32 cq_tp_slot;
	u32 cq_hp_slot;
};

static inline bool is_mcq_enabled(struct ufs_hba *hba)
{
	return hba->use_mcq;
}

static inline bool is_mcq_supported(struct ufs_hba *hba)
{
	return hba->mcq_sup;
}

static inline void ufshcd_inc_tp(struct ufs_hw_queue *q)
{
	u32 next_slot = ((q->sq_tp_slot + 1) % q->max_entries);
	u32 val = next_slot * sizeof(struct hufs_utp_transfer_req_desc);

	q->sq_tp_slot = next_slot;

	writel(val, q->mcq_base_sqd + REG_SQTP);
}

static inline struct cq_entry *ufshcd_mcq_cur_cqe(struct ufs_hw_queue *q)
{
	struct cq_entry *cqe = q->cqe_base_addr;

	return cqe + q->cq_hp_slot;
}

static inline bool ufshcd_mcq_is_sq_full(struct ufs_hw_queue *q)
{
	return ((q->sq_tp_slot + 1) % q->max_entries) == q->sq_hp_slot;
}

static inline bool ufshcd_mcq_is_cq_empty(struct ufs_hw_queue *q)
{
	return q->cq_hp_slot == q->cq_tp_slot;
}

static inline void ufshcd_mcq_inc_cq_hp_slot(struct ufs_hw_queue *q)
{
	q->cq_hp_slot++;
	if (q->cq_hp_slot == q->max_entries)
		q->cq_hp_slot = 0;
}

static inline void ufshcd_mcq_update_cq_hp(struct ufs_hw_queue *q)
{
	writel(q->cq_hp_slot * sizeof(struct cq_entry), q->mcq_base_cqd + REG_CQHP);
}

static inline void ufshcd_mcq_update_sq_hp_slot(struct ufs_hw_queue *q)
{
	u32 val = readl(q->mcq_base_sqd + REG_SQHP);

	q->sq_hp_slot = val / sizeof(struct hufs_utp_transfer_req_desc);
}

static inline void ufshcd_mcq_update_cq_tp_slot(struct ufs_hw_queue *q)
{
	u32 val = readl(q->mcq_base_cqd + REG_CQTP);

	q->cq_tp_slot = val / sizeof(struct cq_entry);
}

#endif /* End of Header */
