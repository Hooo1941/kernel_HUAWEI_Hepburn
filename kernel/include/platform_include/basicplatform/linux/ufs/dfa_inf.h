/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * DFA driver info
 */

#ifndef _DFA_INF_H
#define _DFA_INF_H

#include "ufshci.h"

#define DFA_TAG_NUM      256
#define BITS_PER_ITEM    64
#define DFA_ITEM_NUM     4

/**
 * struct dfa_utp_transfer_req_desc - DFA UTRD structure
 * @header: UTRD header DW-0 to DW-3
 * @command_desc_base_addr_lo: UCD base address low DW-4
 * @command_desc_base_addr_hi: UCD base address high DW-5
 * @response_upiu_length: response UPIU length DW-6
 * @response_upiu_offset: response UPIU offset DW-6
 * @prd_table_length: Physical region descriptor length DW-7
 * @prd_table_offset: Physical region descriptor offset DW-7
 */
struct dfa_utp_transfer_req_desc {
	/* DW 0-3 */
	struct request_desc_header header;

	/* DW 4-5 */
	__le32  command_desc_base_addr_lo;
	__le32  command_desc_base_addr_hi;

	/* DW 6 */
	__le16  response_upiu_length;
	__le16  response_upiu_offset;

	/* DW 7 */
	__le16  prd_table_length;
	__le16  prd_table_offset;

	__le32 meta_data_random_factor_0;
	__le32 meta_data_random_factor_1;
	__le32 meta_data_random_factor_2;
	__le32 meta_data_random_factor_3;

	/* DW 12-15 */
	__le32 reserved[4];
};

/**
 * @sqe_base_addr: submission queue entry base address
 * @sqe_dma_addr: submission queue dma address
 * @cqe_base_addr: completion queue base address
 * @cqe_dma_addr: completion queue dma address
 * @max_entries: max number of slots in this hardware queue
 * @id: hardware queue ID
 * @jd: DFA QUE_PROC - CPU interface queue ID
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
struct dfa_hw_queue {
	void *sqe_base_addr;
	dma_addr_t sqe_dma_addr;
	struct cq_entry *cqe_base_addr;
	dma_addr_t cqe_dma_addr;
	u32 max_entries;
	u32 id;
	u32 jd;

	void __iomem *mcq_base;
	void __iomem *mcq_base_sqd;
	void __iomem *mcq_base_sqis;
	void __iomem *mcq_base_cqd;
	void __iomem *mcq_base_cqis;
	void __iomem *que_proc_base;

	spinlock_t sq_lock;
	u32 sq_tp_slot;
	u32 sq_hp_slot;
	spinlock_t cq_lock;
	u32 cq_tp_slot;
	u32 cq_hp_slot;
};


#ifdef CONFIG_DFX_DEBUG_FS
struct dfa_debugfs_files {
	struct dentry *dfa_root;
	struct dentry *inter_core_idx;
	struct dentry *acore_dfa_commu;
	struct dentry *dfa_acore_commu;
	struct dentry *dfa_doze2_status;
};
#endif

/**
 * struct dfa_hba_info - per adapter DFA private structure
 * @dfa_sys_ctrl: dfa sys ctrl base address
 * @que_proc: dfa que proc base address
 * @dfa_utrdl_base_addr: DFA UTRD address of the command
 * @dfa_hw_queue: dfa hardware queues
 */
struct dfa_hba_info {
	struct ufs_hba *hba;
	bool dfa_enabled;
	void __iomem *dfa_sys_ctrl;
	void __iomem *que_proc;
	struct dfa_utp_transfer_req_desc *dfa_utrdl_base_addr;
	struct dfa_hw_queue *dhq;
	unsigned int nr_dhq_queues;
	unsigned long dfa_inflt_reqs[DFA_ITEM_NUM];
	atomic_t dfa_inflts;
	int dfa_acore_irq;

#ifdef CONFIG_DFX_DEBUG_FS
	struct dfa_debugfs_files debugfs_files;
#endif
};

/**
 * struct dfa_lrb_info - DFA local reference block info
 * @dfa_utr_descriptor_ptr: DFA UTRD address of the command
 * @is_dfa_utrd: use dfa UTRD structure
 * @sq: current IO SQ
 * @cpu: current IO CPU
 */
struct dfa_lrb_info {
	struct dfa_utp_transfer_req_desc *dfa_utr_descriptor_ptr;
	bool is_dfa_utrd;
	u8 sq;
	u8 cpu;
};

#endif /* End of Header */
