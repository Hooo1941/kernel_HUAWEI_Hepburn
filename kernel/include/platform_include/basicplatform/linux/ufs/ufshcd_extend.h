/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: ufs extend header file
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __UFSHCD_EXTEND_H__
#define __UFSHCD_EXTEND_H__
#include "ufshcd.h"

#include <linux/bootdevice/bootdevice.h>
#include <scsi/ufs/ufs.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_devinfo.h>
#include <trace/events/block.h>
#include <linux/sched.h>
#include <trace/events/scsi.h>
#include <linux/kthread.h>
#include <platform_include/cee/linux/lpm_thermal.h>
#include <linux/sched.h>
#include <linux/rpmb/rpmb.h>
#include <linux/bits.h>
#include <linux/workqueue.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <asm/unaligned.h>

#include "hufs_plat.h"
#include "hufs_hcd.h"
#include "ufstt.h"

#include "ufs_debugfs.h"
#include "dsm_ufs.h"
#include "ufs_trace.h"
#include "ufs_vendor_mode.h"
#include "hufs-lib.h"
#include "hufs_hci.h"
#include "hufs-dump.h"
#include "ufshcd_hufs_interface.h"
#ifdef CONFIG_BOOTDEVICE
#include "ufs_rpmb.h"
#endif

#ifdef CONFIG_HUAWEI_STORAGE_ROFA
#include <chipset_common/storage_rofa/storage_rofa.h>
#endif

#ifndef CONFIG_LIBLINUX
#include <linux/fscrypt_common.h>
#endif

#ifdef CONFIG_HUAWEI_DSM_IOMT_UFS_HOST
#include <linux/iomt_host/dsm_iomt_ufs_host.h>
#endif

#ifdef CONFIG_SCSI_UFS_SYNCCACHE_RECORD
#include <pmic_interface.h>
#define SYNCCACHE_DMD_DELAY 5
#endif

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
#include "ufs_vcmd_proc.h"
#endif

#ifdef CONFIG_SCSI_UFS_UNISTORE
#include "ufs_unistore.h"
#endif

#ifdef CONFIG_HUFS_MANUAL_BKOPS
#include "hufs-bkops.h"
#endif
#ifdef CONFIG_DFX_DEBUG_FS
#define HUFS_BUG_ON(x) 	do {BUG_ON(x);} while(0)
#define HUFS_BUG() 		do {BUG();} while(0)
#else
#define HUFS_BUG_ON(x) 	do {} while(0)
#define HUFS_BUG() 		do {} while(0)
#endif

#if (defined CONFIG_SCSI_UFS_ARIES || defined CONFIG_SCSI_UFS_LIBRA)
#define OLD_DEVICE_CONSTRAINT
#endif

/* HI1861 FSR info default size, 4k byte */
#define HI1861_FSR_INFO_SIZE 4096

#if defined(CONFIG_SCSI_UFS_HI1861_VCMD) && defined(CONFIG_PLATFORM_DIEID)
#define UFS_CONTROLLER_DIEID_SIZE 32
#define UFS_FLASH_DIE_ID_SIZE 128
#define UFS_DIEID_NUM_SIZE 4
#define UFS_DIEID_NUM_SIZE_THOR920 1
#define UFS_DCIP_CRACK_EVER_SIZE 1
#define UFS_DCIP_CRACK_NOW_SIZE 1
#define UFS_NAND_CHIP_VER_SIZE 8
#define UFS_DIEID_TOTAL_SIZE 175
#define UFS_DIEID_BUFFER_SIZE 800
#define UFS_DIEID_CHIP_VER_OFFSET 4
#define UFS_DIEID_CONTROLLER_OFFSET 12
#define UFS_DIEID_FLASH_OFFSET 44
#define UFS_DCIP_CRACK_NOW_OFFSET 173
#define UFS_DCIP_CRACK_EVER_OFFSET 174
#define UFS_FLASH_VENDOR_T 0x98
#define UFS_FLASH_VENDOR_M 0x2c
#define UFS_FLASH_VENDOR_H 0xad
#define UFS_FLASH_VENDOR_Y 0x9b
#define UFS_FLASH_TWO_DIE 2
#define UFS_FLASH_THREE_DIE 3
#define UFS_FLASH_FOUR_DIE 4
#endif

#define UFS_CONSISTEN_ACCESS_ENABLE (0x1 << 8)
#define UFS_CONSISTEN_ACCESS_AXDOMAIN (0x2 << 15)
#define UFS_CONSISTEN_ACCESS_AXCACHE (0xf << 17)

extern struct wakeup_source *ffu_lock;
extern int ufs_timeout_lock;

static inline void ufshcd_device_need_enable_autobkops(struct ufs_hba *hba)
{
	if (hba->manufacturer_id == UFS_VENDOR_SANDISK) {
		hba->auto_bkops_enabled = false;
		hba->ee_ctrl_mask |= MASK_EE_URGENT_BKOPS;
		ufshcd_enable_auto_bkops(hba);
	}
}

static inline void ufshcd_put_tm_slot(struct ufs_hba *hba, int slot)
{
	clear_bit_unlock(slot, &hba->tm_slots_in_use);
}

static inline void ufshcd_put_dev_cmd_tag(struct ufs_hba *hba, int tag)
{
#ifdef CONFIG_DFA_MCQ
	if (is_dfa_enabled(hba))
		return dfa_put_dev_cmd_tag(hba, tag);
#endif
	clear_bit_unlock(tag, &hba->lrb_in_use);
}

static inline void __ufshcd_print_doorbell(struct ufs_hba *hba, u32 tm_doorbell,
#ifdef CONFIG_HUFS_HC_CORE_UTR
		u64 tr_doorbell, char *s)
#else
		u32 tr_doorbell, char *s)
#endif
{
	dev_err(hba->dev, "wait door bell clean %s:tm_doorbell:0x%x, "
#ifdef CONFIG_HUFS_HC_CORE_UTR
			  "tr_doorbell:0x%lx\n", s, tm_doorbell, tr_doorbell);
#else
			  "tr_doorbell:0x%x\n", s, tm_doorbell, tr_doorbell);
#endif
}

void ufshcd_hufs_mq_init(struct Scsi_Host *host);
void ufshcd_device_not_support_cp(struct scsi_device *sdev,
				struct request_queue *q);
#ifdef CONFIG_MAS_BLK_HM
void ufshcd_device_support_cp(struct scsi_device *sdev);
#endif
void ufshcd_host_force_reset_sync(struct ufs_hba *hba);

bool ufshcd_get_tm_free_slot(struct ufs_hba *hba, int *free_slot);

void ufshcd_enable_auto_hibern8(struct ufs_hba *hba);
void ufshcd_disable_auto_hibern8(struct ufs_hba *hba);
int ufshcd_vendor_feature_options_apply(struct ufs_hba *hba);

void ufshcd_check_init_mode(struct ufs_hba *hba, int err);
void ufshcd_enable_pwm_cnt(struct ufs_hba *hba);
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
int ufshcd_hba_uie_init(struct ufs_hba *hba);
void ufshcd_prepare_req_desc_uie(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
#endif
#ifdef CONFIG_MAS_BLK
int ufshcd_direct_flush(struct scsi_device *sdev);
#endif
void ufshcd_disable_pwm_cnt(struct ufs_hba *hba);
void __ufshcd_enable_dev_tmt_cnt(struct ufs_hba *hba);
void ufshcd_enable_dev_tmt_cnt(struct ufs_hba *hba);
int ufshcd_send_scsi_sync_cache_init(void);
void ufshcd_send_scsi_sync_cache_deinit(void);

void ufs_get_info(struct ufs_hba *hba);
#ifdef CONFIG_MAS_BLK
void ufshcd_dump_status(struct Scsi_Host *host, enum blk_dump_scene dump_type);
#endif
void ufshcd_device_not_support_autoh8(struct ufs_hba *hba);
void ufshcd_add_idle_intr_check_timer(struct ufs_hba *hba);
irqreturn_t ufshcd_timeout_handle_intr(int timer_irq, void *__hba);
void ufs_idle_intr_toggle(struct ufs_hba *hba, int enable);
void ufshcd_update_uic_reg_hist(struct ufs_uic_err_reg_hist *reg_hist, u32 reg);
bool ufshcd_error_need_queue_work(struct ufs_hba *hba);
void ufshcd_config_adapt(struct ufs_hba *hba, struct ufs_pa_layer_attr *pwr_mode);
#ifdef CONFIG_MAS_HEALTH
int ufshcd_get_health_info(struct scsi_device *sdev,
	u8 *pre_eol_info, u8 *life_time_est_a, u8 *life_time_est_b);
#endif
void ufs_ffu_pm_runtime_delay_enable(struct work_struct *work);
enum blk_eh_timer_return ufshcd_eh_timed_out(struct scsi_cmnd *scmd);
#ifdef CONFIG_SCSI_HUFS
int ufshcd_task_req_compl(struct ufs_hba *hba, u32 index, u8 *resp);
#endif
int __ufshcd_uic_hibern8_op_irq_safe(struct ufs_hba *hba, bool h8_op);
void ufs_update_linkup_err(struct ufs_hba *hba);
int ufs_switch_hs_check(struct ufs_hba *hba);
int ufs_extend_config_init(struct ufs_hba *hba);
int ufs_blk_config_init(struct ufs_hba *hba);
int ufshcd_hufs_extend_trc_handler(struct ufs_hba *hba, unsigned long completed_reqs);
void ufs_dev_too_high_temp_event_handler(struct ufs_hba *hba);
void ufshcd_high_temp_config(struct ufs_hba *hba);
void ufshcd_rev_store(struct ufs_hba *hba);
void ufshcd_ee_reset(struct ufs_hba *hba);
#endif
