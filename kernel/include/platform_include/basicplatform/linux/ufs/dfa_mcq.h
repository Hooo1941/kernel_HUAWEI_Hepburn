/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * DFA driver
 */

#ifndef _DFA_MCQ_H
#define _DFA_MCQ_H

#include <linux/platform_device.h>
#include "ufshcd.h"
#include "ufs_mcq.h"
#include "dfa_inf.h"
#include "dfa_aquila.h"

#define DFA_MAX_ENTRIES            64
#define DFA_SQ_ENTRY_SIZE_IN_DWORD 16
#define DFA_CQ_ENTRY_SIZE_IN_DWORD 8
#define DFA_CQTEPS_MSI_START       160
#define DFA_CQTEPS_MSI_END         239
#define DFA_IRQ_NUMS               325
#define DFA_QUEUE_EN_OFFSET        31
#define DFA_QUEUE_ID_OFFSET        16
#define DFA_HW_QUEUES              5
#define DFA_VCMD_TAG               (DFA_TAG_NUM - 1)

/* corresponds to DFA_META_INPUT_LEN in ufs_plat.c */
#define DFA_META_INPUT_LEN         11

/* CQISy - CQ y Interrupt Status Register  */
#define DFA_CQIS_TEPS         0x1
#define DFA_SQ_STOP_BIT       0x1
#define DFA_SQ_CQ_I_SIZE      0x1C
#define DFA_SQ_CQ_Y_SIZE      0x8
#define DFA_CQ_INTERRUPT_SIZE 0xC
#define DFA_SQ_RUN_TIME_SIZE  0x14
#define DFA_SQ_STOP_STATUS    0x1
#define DFA_SQ_ENTRY_ABORT    (0xF << 28)
#define MASK_LOWER_ADDR       0xFFFFFF80

#define dfa_tag_idx(tag) ((tag) / BITS_PER_ITEM)
#define dfa_tag_off(tag) ((tag) % BITS_PER_ITEM)

enum {
	QUE_PROC_DISABLE = 1,
	QUE_PROC_ENABLE,
};

enum boot_mode {
	MODE_SYNC_BUF,
	MODE_DDR,
	MODE_RESET,
	MODE_OTHER,
	BOOT_MODE_MAX,
};

enum chip_mode {
	MODE_ASIC,
	MODE_FPGA,
	MODE_EMU,
	CHIP_MODE_MAX,
};

/* ACORE-DFA communication register index */
enum {
	DFA_WORK_MODE_IND = 0,
	DFA_BOOT_MODE_IND = 1,
	DFA_LOAD_ADDR_IND = 2,
	DFA_SWITCH_ADDR_IND = 3,
	DFA_DISABLE_QUEUE_IND = 4,
	DFA_UFS_LSU_IND = 5,
	DFA_UFS_PMC_IND = 6,
	DFA_SCEFUSEDATA2_IND = 7,
	DFA_SCEFUSEDATA8_IND = 8,

	/* add new ind above */
	DFA_COMMU_BANK_IND = 31,
};

/* DFA-ACORE communication register index */
enum {
	DFA_INIT_DONE_REP = 0,
	DFA_WFI_READY_REP = 1,
	DFA_SUSPEND_INF_REP = 2,
	DFA_PREPARE_RESET_REP = 3,

	/* add new rep above */
	DFA_COMMU_BANK_REP = 31,
};

enum {
	/* query, rpmb read, boot read/write, TM UPIU, read/write buffer */
	SQ0 = 0,
	/* CP+ */
	SQ1,
	/* CP */
	SQ2,
	/* sync read */
	SQ3,
	/* write, rpmb write, UNMAP, barrier, DataMove */
	SQ4,
};

#ifdef CONFIG_DFX_DEBUG_FS
#define dfa_hex_dump(prefix_str, buf, len) do {                       \
	size_t __len = (len);                                            \
	print_hex_dump(KERN_ERR, prefix_str,                             \
		       __len > 4 ? DUMP_PREFIX_OFFSET : DUMP_PREFIX_NONE,\
		       16, 4, buf, __len, false);                        \
} while (0)
#endif

/* dfa_mcq.c */
int dfa_get_base_addr(struct ufs_hba *hba, struct device_node *np);
int dfa_link_startup(struct ufs_hba *hba);
bool dfa_get_dev_cmd_tag(struct ufs_hba *hba, int *tag_out);
void dfa_put_dev_cmd_tag(struct ufs_hba *hba, int tag);
void dfa_mcq_transfer_req_compl_all(struct ufs_hba *hba, bool force_compl);
void dfa_enable_ooo_mode(struct ufs_hba *hba, bool enable);
void dfa_mcq_make_queues_operational(struct ufs_hba *hba);
void dfa_get_intr_core_irq(struct platform_device *pdev, struct ufs_hba *hba);
int dfa_config_mcq(struct ufs_hba *hba, bool async);
void dfa_mcq_send_command(struct ufs_hba *hba,
	unsigned int task_tag, struct ufshcd_lrb *lrbp);
int dfa_map_sg(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
void dfa_prepare_req_desc_hdr(struct ufshcd_lrb *lrbp,
	u8 *upiu_flags, enum dma_data_direction cmd_dir);
int dfa_memory_alloc(struct ufs_hba *hba);
void dfa_host_memory_configure(struct ufs_hba *hba);
void dfa_send_command(struct ufs_hba *hba, unsigned int tag);
int dfa_queuecommand(struct Scsi_Host *host, struct scsi_cmnd *cmd);
void dfa_dhq_slot_clear(struct ufs_hba *hba);
int dfa_suspend(struct ufs_hba *hba, enum ufs_pm_op pm_op);
int dfa_resume(struct ufs_hba *hba, enum ufs_pm_op pm_op);
void dfa_mcq_params_init(struct ufs_hba *hba);
void dfa_dump_lrbp_status(struct ufs_hba *hba);

/* dfa_error.c */
enum blk_eh_timer_return dfa_eh_timed_out(struct scsi_cmnd *scmd);
void dfa_print_host_state(struct ufs_hba *hba);

#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
void dfa_uie_utrd_prepare(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
#endif
#ifdef CONFIG_DFX_DEBUG_FS
void dfa_print_trs(struct ufs_hba *hba, bool pr_prdt);
#endif

/* ufs-plat.c */
void hufs_crypto_set_keyindex(struct ufs_hba *hba, struct ufshcd_lrb *lrbp,
	u32 *hash_res, u32 *crypto_cci, unsigned long *flags);
void hufs_get_meta_data_factor(struct ufshcd_lrb *lrbp, struct ufs_hba *hba,
	u32 *dword_8, u32 *dword_9, u32 *dword_10, u32 *dword_11);
#if defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO) && defined(CONFIG_DFX_DEBUG_FS)
void hufs_inline_crypto_debug(struct ufs_hba *hba, u32 *hash_res, u32 *crypto_enable, u64 dun);
#endif

/* ufshcd.c */
void ufshcd_start_delay_work(struct ufs_hba *hba, struct scsi_cmnd *cmd);
int ufshcd_comp_scsi_upiu(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
void ufshcd_check_disable_dev_tmt_cnt(struct ufs_hba *hba, struct scsi_cmnd *cmd);
int ufshcd_custom_upiu(struct utp_upiu_req *ucd_req_ptr,
	struct request *req, struct scsi_cmnd *scmd, struct ufs_hba *hba);
int ufshcd_set_dev_pwr_mode(struct ufs_hba *hba, enum ufs_dev_pwr_mode pwr_mode);

static inline struct cq_entry *dfa_mcq_cur_cqe(struct dfa_hw_queue *q)
{
	struct cq_entry *cqe = q->cqe_base_addr;

	return cqe + q->cq_hp_slot;
}

static inline bool dfa_mcq_is_sq_full(struct dfa_hw_queue *q)
{
	return ((q->sq_tp_slot + 1) % q->max_entries) == q->sq_hp_slot;
}

static inline bool dfa_mcq_is_cq_empty(struct dfa_hw_queue *q)
{
	return q->cq_hp_slot == q->cq_tp_slot;
}

static inline void dfa_mcq_inc_cq_hp_slot(struct dfa_hw_queue *q)
{
	q->cq_hp_slot++;
	if (q->cq_hp_slot == q->max_entries)
		q->cq_hp_slot = 0;
}

static inline void dfa_mcq_update_sq_hp(struct dfa_hw_queue *q)
{
	u32 val;

	val = readl(que_proc_sq_i_sqh(q->que_proc_base, q->id));
	q->sq_hp_slot = val / sizeof(struct dfa_utp_transfer_req_desc);
}

static inline void dfa_mcq_update_sq_tp(struct dfa_hw_queue *q)
{
	q->sq_tp_slot = (q->sq_tp_slot + 1) % q->max_entries;
	writel(q->sq_tp_slot * sizeof(struct dfa_utp_transfer_req_desc),
		que_proc_sq_i_sqt(q->que_proc_base, q->id));
}

static inline void dfa_mcq_update_cq_hp(struct dfa_hw_queue *q)
{
	u32 val;

	val = q->cq_hp_slot * sizeof(struct cq_entry);
	writel(val, que_proc_cq_i_cqh(q->que_proc_base, q->id));
}

static inline void dfa_mcq_update_cq_tp(struct dfa_hw_queue *q)
{
	u32 val;

	val = readl(que_proc_cq_i_cqt(q->que_proc_base, q->id));
	q->cq_tp_slot = val / sizeof(struct cq_entry);
}

static inline u32 dfa_mcq_read_cqis(struct ufs_hba *hba, int i)
{
	return readl(que_proc_cq_i_cqis(hba->dhba.que_proc, i));
}

static inline void dfa_mcq_write_cqis(struct ufs_hba *hba, u32 val, int i)
{
	writel(val, que_proc_cq_i_cqis(hba->dhba.que_proc, i));
}

static inline void dfa_mcq_enable_esi(struct ufs_hba *hba)
{
	writel(0x20000, que_proc_msi_device_id(hba->dhba.que_proc));
}

static inline bool is_dfa_lrbp(struct ufshcd_lrb *lrbp)
{
	return lrbp->dlrb.is_dfa_utrd;
}

/**
 * dfa_get_tr_ocs - Get the DFA UTRD Overall Command Status
 * @lrbp: pointer to local command reference block
 *
 * This function is used to get the OCS field from UTRD
 * Returns the OCS field in the UTRD
 */
static inline int dfa_get_tr_ocs(struct ufshcd_lrb *lrbp)
{
	return le32_to_cpu(lrbp->dlrb.dfa_utr_descriptor_ptr->header.dword_2) & MASK_OCS;
}

static inline struct dfa_hw_queue *dfa_mcq_find_hwq(
	struct ufs_hba *hba, unsigned int task_tag, struct ufshcd_lrb *lrbp)
{
	return &hba->dhba.dhq[lrbp->dlrb.cpu * DFA_HW_QUEUES + lrbp->dlrb.sq];
}

static inline unsigned long *dfa_tag_bitmap(struct ufs_hba *hba, int tag)
{
	return &hba->dhba.dfa_inflt_reqs[dfa_tag_idx(tag)];
}

static inline void dfa_inflt_req_clear(struct ufs_hba *hba, int tag)
{
	clear_bit(dfa_tag_off(tag), dfa_tag_bitmap(hba, tag));
	atomic_dec(&hba->dhba.dfa_inflts);
}
#endif /* End of Header */
