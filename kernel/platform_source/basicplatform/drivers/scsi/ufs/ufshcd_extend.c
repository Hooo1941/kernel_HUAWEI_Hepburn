/*
 * ufshcd_extend.c
 *
 * basic interface for hufs
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "ufshcd_extend.h"
#include "ufshcd.h"
#include "ufshcd_perf.h"
#include "ufshcd_debug.h"
#include "ufshcd_protocol.h"
#include "ufs-hpb.h"
#include "ufshcd_hufs_interface.h"
#include "ufs_fault_inject.h"
#include <asm/unaligned.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>

#ifdef CONFIG_MAS_BLK_HM
#include "basicplatform/linux/mas/mas_blk_hm.h"
#endif

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
#include "ufs_vcmd_proc.h"
#endif

#ifdef CONFIG_SCSI_UFS_UNISTORE
#include "ufs_unistore.h"
#endif

#ifdef CONFIG_HW_STAT_PSI_LRB
#include <linux/hw_stat.h>
#endif

#if defined (CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3)
#define UFS_INLINE_CRYPTO_VER 3
#elif defined (CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2)
#define UFS_INLINE_CRYPTO_VER 2
#elif defined (CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO)
#define UFS_INLINE_CRYPTO_VER 1
#else
#define UFS_INLINE_CRYPTO_VER 0
#endif

#define HUFS_H8_TIMEOUT_RETRY_TIMES	2000
#define HUFS_H8_TIMEOUT_RETRY_UDELAY	100
#define HUFS_H8_OP_ENTER		true
#define HUFS_H8_OP_EXIT		false

#ifdef CONFIG_SCSI_HUFS_MQ_DEFAULT
void ufshcd_hufs_mq_init(struct Scsi_Host *host)
{
#ifdef CONFIG_MAS_BLK
	host->queue_quirk_flag |= SHOST_QUIRK(SHOST_QUIRK_UNMAP_IN_SOFTIRQ);
	host->queue_quirk_flag |= SHOST_QUIRK(SHOST_QUIRK_BUSY_IDLE_ENABLE);
	host->queue_quirk_flag |= SHOST_QUIRK(SHOST_QUIRK_IO_LATENCY_WARNING);
	host->queue_quirk_flag |= SHOST_QUIRK(SHOST_QUIRK_FLUSH_REDUCING);
	host->queue_quirk_flag |= SHOST_QUIRK(SHOST_QUIRK_HUFS_MQ);
	if (host->queue_quirk_flag &
		SHOST_QUIRK(SHOST_QUIRK_HUFS_MQ)) {
		host->queue_quirk_flag |=
			 SHOST_QUIRK(SHOST_QUIRK_DRIVER_TAG_ALLOC);
		host->nr_hw_queues = 1;
		host->mq_queue_depth = 192;
#ifdef CONFIG_DFA_MCQ
		if (host->queue_quirk_flag & SHOST_QUIRK(SHOST_QUIRK_MAS_MCQ))
			host->mq_queue_depth = 256;
#endif
		host->mq_reserved_queue_depth = 64;
		host->mq_high_prio_queue_depth = 64;
		host->can_queue =
			host->mq_queue_depth * (int)host->nr_hw_queues;
	}
#endif /* CONFIG_MAS_BLK */
}
#endif

#ifdef CONFIG_MAS_MQ_USING_CP
void ufshcd_device_not_support_cp(struct scsi_device *sdev,
				struct request_queue *q)
{
	struct ufs_hba *hba = shost_priv(sdev->host);

	if ((hba->manufacturer_id == UFS_VENDOR_HI1861) &&
		(strstarts(hba->model, UFS_MODEL_SS6100)))
		blk_queue_cp_enable(q, false);
	else
		blk_queue_cp_enable(q, true);
}
#endif

#ifdef CONFIG_MAS_BLK_HM
void ufshcd_device_support_cp(struct scsi_device *sdev)
{
	struct ufs_hba *hba = shost_priv(sdev->host);

	if (!((hba->manufacturer_id == UFS_VENDOR_HI1861) &&
		(strstarts(hba->model, UFS_MODEL_SS6100))))
		mas_blk_hm_support_cp_notify();
}
#endif

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
/*
 * ufshcd_device_capbitlity_config - device need config capability.
 * @hba - Pointer to adapter instance
 */
static void ufshcd_device_capbitlity_config(struct ufs_hba *hba)
{
	int ret;
	struct ufs_query_vcmd cmd = { 0 };

	if (!(hba->quirks & UFSHCD_QUIRK_DEVICE_CONFIG))
		return;

	cmd.opcode = UPIU_QUERY_OPCODE_VENDOR_WRITE;
	cmd.idn = DEVICE_CAPABILITY_FLAG;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;

	ret = ufshcd_query_vcmd_retry(hba, &cmd);

	dev_err(hba->dev, "%s: ret=0x%x.\n", __func__, ret);
}
#endif

void ufshcd_host_force_reset_sync(struct ufs_hba *hba)
{
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);

	hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED;
	hba->force_reset = true;

	if (!kthread_queue_work(&hba->eh_worker, &hba->eh_work))
		dev_err(hba->dev, "%s: queue hba->eh_worker failed !\n",
			__func__);

	spin_unlock_irqrestore(hba->host->host_lock, flags);

	kthread_flush_work(&hba->eh_work);
}

void ufshcd_enable_auto_hibern8(struct ufs_hba *hba)
{
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->autoh8_disable_depth > 0) {
		hba->autoh8_disable_depth--;
	} else {
		dev_err(hba->dev, "unblance auto hibern8 enabled\n");
		HUFS_BUG(); /*lint !e146*/
	}
	if (hba->autoh8_disable_depth == 0) {
		ufshcd_writel(hba, hba->ahit, REG_AUTO_HIBERNATE_IDLE_TIMER);
		if (ufshcd_is_hufs_hc(hba) && hba->vops &&
			hba->vops->ufshcd_hufs_enable_auto_hibern8)
			hba->vops->ufshcd_hufs_enable_auto_hibern8(hba);
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

void ufshcd_disable_auto_hibern8(struct ufs_hba *hba)
{
	unsigned long flags;
	ktime_t start = ktime_get();

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->autoh8_disable_depth > 0) {
		hba->autoh8_disable_depth++;
	} else {
		hba->autoh8_disable_depth++;
		ufshcd_writel(hba, 0, REG_AUTO_HIBERNATE_IDLE_TIMER);

		if (ufshcd_is_hufs_hc(hba) && hba->vops &&
			hba->vops->ufshcd_hufs_disable_auto_hibern8)
			hba->vops->ufshcd_hufs_disable_auto_hibern8(hba);
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

int ufshcd_vendor_feature_options_apply(struct ufs_hba *hba)
{
	int ret;
	uint32_t attr_val;

	if (hba->manufacturer_id != UFS_VENDOR_SAMSUNG)
		return 0;

	ret = ufshcd_query_attr(hba, UPIU_QUERY_OPCODE_READ_ATTR,
				QUERY_ATTR_IDN_VENDOR_FEATURE_OPTIONS, 0, 0,
				&attr_val);
	if (!ret && (attr_val & WB_VENDOR_FEATURE_SUPPORT))
		hba->force_selector = true;

	dev_info(hba->dev, "vendor feature options: 0x%x\n",
		 hba->force_selector);

	return ret;
}

/**
 * ufshcd_get_tm_free_slot - get a free slot for task management request
 * @hba: per adapter instance
 * @free_slot: pointer to variable with available slot value
 *
 * Get a free tag and lock it until ufshcd_put_tm_slot() is called.
 * Returns 0 if free slot is not available, else return 1 with tag value
 * in @free_slot.
 */
bool ufshcd_get_tm_free_slot(struct ufs_hba *hba, int *free_slot)
{
	int tag;
	bool ret = false;

	if (!free_slot)
		goto out;

	do {
		tag = find_first_zero_bit(&hba->tm_slots_in_use, hba->nutmrs);
		if (tag >= hba->nutmrs)
			goto out;
	} while (test_and_set_bit_lock(tag, &hba->tm_slots_in_use));

	*free_slot = tag;
	ret = true;
out:
	return ret;
}

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
static void ufshcd_count_vol(int retry, int* v_tx, int* v_rx)
{
	if (retry / 3 >= 2) {
		*v_rx = 0;
		*v_tx = 0;
	} else if (retry % 2 == 1) {
		*v_tx = (retry / 2) + 1;
		*v_rx = 0;
	} else if (retry % 2 == 0) {
		*v_tx = 0;
		*v_rx = (retry / 2) + 1;
	}
	pr_err("ufs retry: %d count v_tx:%d v_rx:%d\n", retry, *v_tx, *v_rx);
}

void ufshcd_check_init_mode(struct ufs_hba *hba, int err)
{
	if (err) {
		if (hba->init_retry > 0) {
			hba->init_retry--;
			ufshcd_count_vol(hba->init_retry, &hba->v_tx, &hba->v_rx);
		} else {
			hba->init_retry = 6;
		}
	} else {
		if (hba->use_pwm_mode) {
			if (!work_busy(&hba->recover_hs_work)) {
				pm_runtime_forbid(hba->dev);
				hba->disable_suspend = 1;
				dev_err(hba->dev, "forbid ufs pm runtime in pwm\n");
				schedule_work(&hba->recover_hs_work);
			} else {
				dev_err(hba->dev, "%s:recover_hs_work is runing \n", __func__);
			}
		}
	}
}
#endif

static inline void ufshcd_ue_clean(struct ufs_hba *hba)
{
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	ufshcd_writel(hba, UIC_ERROR, REG_INTERRUPT_STATUS);
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

static int ufshcd_ue_get(struct ufs_hba *hba)
{
	u32 value;
	int ret;
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	value = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	if (value & UIC_ERROR) {
		dev_err(hba->dev, "ufs ue happened in nop. intr = 0x%x\n", value);
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int ufshcd_trylock_hostlock(struct ufs_hba *hba,unsigned long* flags)
{
	int locked = 0;
	unsigned int trycount = 100000;
	do {
		locked = spin_trylock_irqsave(hba->host->host_lock, *flags);/*lint  !e666*/
		if(locked)
			break;
		udelay(10);
	} while(--trycount>0);
	return locked;
}

/*
 * ufshcd_set_lun_protect() - set device write protect
 * hba: per-adapter instance
 *
 * Set fPowerOnWPEn flag and poll until device toggles it.
 */
#ifdef CONFIG_SCSI_UFS_LUN_PROTECT
static int ufshcd_set_lun_protect(struct ufs_hba *hba)
{
	int err;
	bool flag_res = false;
	unsigned int enter_recovery;
	char *pstr = NULL;

	pstr = strstr(saved_command_line, "enter_recovery=");
	if (pstr && sscanf(pstr, "enter_recovery=%u", &enter_recovery) == 1) {
		if (enter_recovery == 1) {
			dev_err(hba->dev, "set LU protect enter recovery\n");
			err = 0;
			goto out;
		}
	}

	err = ufshcd_query_flag_retry(hba, UPIU_QUERY_OPCODE_SET_FLAG,
			QUERY_FLAG_IDN_PWR_ON_WPE, 0, NULL);
	if (err) {
		dev_err(hba->dev,
			"%s setting protect flag failed with error %d\n",
			__func__, err);
		goto out;
	}
	/* UFS needs 1ms to take affect for enabling lun protect. */
	msleep(1);
	err = ufshcd_query_flag_retry(hba, UPIU_QUERY_OPCODE_READ_FLAG,
			QUERY_FLAG_IDN_PWR_ON_WPE, 0, &flag_res);
	if (err)
		dev_err(hba->dev,
			"%s reading fPowerOnWPEn failed with error %d\n",
			__func__, err);
	else if (flag_res)
		dev_err(hba->dev, "set LU protect sucess\n");
	else
		dev_err(hba->dev, "set LU protect fail\n");

out:
	return err;
}
#endif

void ufshcd_set_auto_hibern8_delay(struct ufs_hba *hba, unsigned int value)
{
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	hba->ahit = value;
	if (hba->autoh8_disable_depth == 0)
		ufshcd_writel(hba, hba->ahit, REG_AUTO_HIBERNATE_IDLE_TIMER);
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}
EXPORT_SYMBOL_GPL(ufshcd_set_auto_hibern8_delay);

void ufshcd_enable_pwm_cnt(struct ufs_hba *hba)
{
	unsigned long flags;
#ifndef CONFIG_HUFS_HC
	struct hufs_host *host = (struct hufs_host *)hba->priv;
#endif

	if (!(hba->caps & UFSHCD_CAP_PWM_DAEMON_INTR))
		return;

	spin_lock_irqsave(hba->host->host_lock, flags);
#ifdef CONFIG_HUFS_HC
	ufshcd_hufs_enable_unipro_intr(hba, HSH8ENT_LR_INTR);
#else
	ufs_sys_ctrl_writel(host, 1000 * 1000, UFS_PWM_COUNTER);
	ufs_sys_ctrl_clr_bits(host, BIT_UFS_PWM_CNT_INT_MASK, UFS_DEBUG_CTRL);
	ufs_sys_ctrl_set_bits(host, BIT_UFS_PWM_CNT_EN, UFS_DEBUG_CTRL);
#endif
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

void __ufshcd_disable_pwm_cnt(struct ufs_hba *hba)
{
#ifdef CONFIG_HUFS_HC
	ufshcd_hufs_disable_unipro_intr(hba, HSH8ENT_LR_INTR);
#else
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	ufs_sys_ctrl_clr_bits(host, BIT_UFS_PWM_CNT_EN, UFS_DEBUG_CTRL);
	ufs_sys_ctrl_set_bits(host, BIT_UFS_PWM_CNT_INT_MASK, UFS_DEBUG_CTRL);
	ufs_sys_ctrl_writel(host, 1, UFS_PWM_COUNTER_CLR);
#endif
}

/**
 * ufshcd_support_inline_encrypt - Check if controller supports
 *                            UFS inline encrypt
 * @hba: per adapter instance
 */
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
bool ufshcd_support_inline_encrypt(struct ufs_hba *hba)
{
	if (hba->capabilities & MASK_INLINE_ENCRYPTO_SUPPORT)
		return true;
	else
		return false;
}

int ufshcd_hba_uie_init(struct ufs_hba *hba)
{
	int err = 0;
	pr_err("ufshcd_hba_uie_init \r\n");

	if (!ufshcd_support_inline_encrypt(hba))
		return 0;

	pr_err("ufshcd_hba_uie_init support_inline_encrypt\r\n");

	if (hba->vops && hba->vops->uie_config_init) {
		err = hba->vops->uie_config_init(hba);
		if (!err)
			hba->host->crypto_enabled = 1;
		pr_err("ufshcd_hba_uie_init support_inline_encrypt 2\r\n");
	}

	return err;
}

/**
 * ufshcd_prepare_req_desc_uie() - fills the utp_transfer_req_desc,
 * for UFS inline encrypt func
 * @hba: UFS hba
 * @lrbp: local reference block pointer
 */
void ufshcd_prepare_req_desc_uie(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	if (ufshcd_support_inline_encrypt(hba)) {
		if (hba->vops && hba->vops->uie_utrd_pre)
			hba->vops->uie_utrd_pre(hba, lrbp);
	}
}
#endif

void ufshcd_disable_pwm_cnt(struct ufs_hba *hba)
{
	unsigned long flags;

	if (!(hba->caps & UFSHCD_CAP_PWM_DAEMON_INTR))
		return;

	spin_lock_irqsave(hba->host->host_lock, flags);
	__ufshcd_disable_pwm_cnt(hba);
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

void __ufshcd_enable_dev_tmt_cnt(struct ufs_hba *hba)
{
	if (!(hba->caps & UFSHCD_CAP_DEV_TMT_INTR))
		return;

	if (hba->dev_tmt_disable_depth > 0) {
		hba->dev_tmt_disable_depth--;
	} else {
		dev_err(hba->dev, "unblance device timeout intr enabled\n");
		HUFS_BUG(); /*lint !e146*/
	}

	struct ufs_dev_info *dev_info = &hba->dev_info;
	if (hba->dev_tmt_disable_depth == 0) {
#ifdef CONFIG_HUFS_HC
		u32 reg_val = 0;

		reg_val = ufshcd_readl(hba, UFS_PROC_MODE_CFG);
		reg_val |= CFG_RX_CPORT_IDLE_CHK_EN;

		/* set dev_tmt timeout trsh, 0xC8(200) mean 200 * 10ms, HH3DV7 factory 0x64, hione 0x0 */
		reg_val &= ~CFG_RX_CPORT_IDLE_TIMEOUT_TRSH;
		if (memcmp(dev_info->model, UFS_MODEL_HHV7_256GB, UFS_MODEL_HH_PTN_LEN) == 0 ||
			memcmp(dev_info->model, UFS_MODEL_HHV7_512GB, UFS_MODEL_HH_PTN_LEN) == 0 ||
			memcmp(dev_info->model, UFS_MODEL_HHV7_1TB, UFS_MODEL_HH_PTN_LEN) == 0) {
			reg_val |= (DEV_TMT_VAL_HH_V7 << BIT_SHIFT_DEV_TMT_TRSH);
			dev_err(hba->dev, "%s: hh dev_tmt config\r\n", __func__);
		} else {
			reg_val |= (DEV_TMT_VAL << BIT_SHIFT_DEV_TMT_TRSH);
		}
		ufshcd_writel(hba, reg_val, UFS_PROC_MODE_CFG);
		/* set io timeout 5s */
		reg_val = IO_TIMEOUT_TRSH_VAL & MASK_CFG_IO_TIMEOUT_TRSH;
		reg_val |= MASK_CFG_IO_TIMEOUT_CHECK_EN;
		ufshcd_writel(hba, reg_val, UFS_IO_TIMEOUT_CHECK);
#else
		struct hufs_host *host =
			(struct hufs_host *)hba->priv;
		/* most of all, one request can not execute more than 2s */
		ufs_sys_ctrl_writel(host, 2000 * 1000, UFS_DEV_TMT_COUNTER);
		ufs_sys_ctrl_set_bits(
			host, BIT_UFS_DEV_TMT_CNT_EN, UFS_DEBUG_CTRL);
		ufs_sys_ctrl_clr_bits(
			host, BIT_UFS_DEV_TMT_CNT_MASK, UFS_DEBUG_CTRL);
#endif
	}
}

void ufshcd_enable_dev_tmt_cnt(struct ufs_hba *hba)
{
	unsigned long flags;

	if (!(hba->caps & UFSHCD_CAP_DEV_TMT_INTR))
		return;

	spin_lock_irqsave(hba->host->host_lock, flags);
	__ufshcd_enable_dev_tmt_cnt(hba);
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

static unsigned char *sync_cache_buffer = NULL;
int ufshcd_send_scsi_sync_cache_init(void)
{
	if (likely(!sync_cache_buffer)) {
		sync_cache_buffer = kzalloc((size_t)SCSI_SENSE_BUFFERSIZE, GFP_KERNEL);
		if (unlikely(!sync_cache_buffer))
			return -ENOMEM;
	}
	return 0;
}

void ufshcd_send_scsi_sync_cache_deinit(void)
{
	if (likely(sync_cache_buffer)) {
		kfree(sync_cache_buffer);
		sync_cache_buffer = NULL;
	}
}

#ifdef CONFIG_MAS_BLK
static void ufshcd_sync_cache_setup_lrbp(struct ufshcd_lrb *lrbp,
		struct scsi_cmnd *cmd, int tag)
{
	lrbp->cmd = cmd;
	lrbp->sense_bufflen = SCSI_SENSE_BUFFERSIZE;
	lrbp->sense_buffer = cmd->sense_buffer;
	lrbp->saved_sense_len = 0;
	lrbp->task_tag = tag;
	lrbp->req_send = false;
	lrbp->lun = ufshcd_scsi_to_upiu_lun((unsigned int)cmd->device->lun);
	lrbp->intr_cmd = false;
	lrbp->command_type = UTP_CMD_TYPE_SCSI;
}

static int ufshcd_sync_cache_irq_safe(struct ufs_hba *hba,
					struct scsi_cmnd *cmd,	unsigned int query_intr_timeout)
{
	int tag;
	unsigned long flags;
	struct ufshcd_lrb *lrbp = NULL;
	volatile u32 ie_value = 0;
	volatile u32 tr_doorbell = 0;

	pm_runtime_get_sync(hba->dev);

	spin_lock_irqsave(hba->host->host_lock, flags);
	scsi_block_requests(hba->host);
	if (__ufshcd_wait_for_doorbell_clr(hba)) {
		dev_err(hba->dev, "wait doorbell clear timeout\n");
		goto unlock_unblock_and_enable_intr;
	}

	ie_value = ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	if (ie_value)
		ufshcd_writel(hba, 0, REG_INTERRUPT_ENABLE);

	tag = (int)ffz(hba->lrb_in_use);
	if (tag >= hba->nutrs)
		goto unlock_unblock_and_enable_intr;

	__set_bit(tag, &hba->lrb_in_use);
#ifdef CONFIG_HW_STAT_PSI_LRB
	hw_stat_lrb_in_use_changed(hba->lrb_in_use);
#endif
	lrbp = &hba->lrb[tag];
	ufshcd_sync_cache_setup_lrbp(lrbp, cmd, tag);
	if (ufshcd_compose_upiu(hba, lrbp))
		goto unlock_unblock_and_enable_intr;
	/* Make sure descriptors are ready before ringing the doorbell */
	wmb();
	/* issue command to the controller */
	__set_bit(tag, &hba->outstanding_reqs);
#ifdef CONFIG_HUFS_HC_CORE_UTR
	ufshcd_writel(hba, 1 << ((unsigned int)tag % SLOT_NUM_EACH_CORE),
		      UFS_CORE_UTRLDBR(tag / SLOT_NUM_EACH_CORE));
#else
	ufshcd_writel(hba, 1 << (unsigned int)tag, REG_UTP_TRANSFER_REQ_DOOR_BELL);
#endif
	/* Make sure that doorbell is committed immediately */
	wmb();
	while (query_intr_timeout-- > 0) {
#ifdef CONFIG_HUFS_HC_CORE_UTR
		tr_doorbell = ufshcd_readl(hba, UFS_CORE_UTRLDBR(tag / SLOT_NUM_EACH_CORE));
		if (!(tr_doorbell & (1U << (tag % SLOT_NUM_EACH_CORE)))) {
#else
		tr_doorbell = ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL);
		if (!(tr_doorbell & (1U << (unsigned int)tag))) {
#endif
			hba->outstanding_reqs ^= (1UL << (unsigned int)tag);
			goto scsi_cmd_deinit;
		}
		udelay(50);
	}
scsi_cmd_deinit:
	lrbp->cmd = NULL;
	__clear_bit(tag, &hba->lrb_in_use);
unlock_unblock_and_enable_intr:
	if (ie_value)
		ufshcd_writel(hba, ie_value, REG_INTERRUPT_ENABLE);
#ifdef CONFIG_SCSI_UFS_SYNCCACHE_RECORD
	ufshcd_record_sc(UFS_SC_EMERGENCY, query_intr_timeout ? 0 : -ETIMEDOUT);
#endif
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	scsi_unblock_requests(hba->host);
	pm_runtime_put_autosuspend(hba->dev);
	return 0;
}

static int ufshcd_send_scsi_sync_cache_on_specified_disk(struct ufs_hba *hba, struct scsi_device *sdp)
{
	int ret;
	struct scsi_cmnd cmnd;
	unsigned char cmd[10] = {0};
	cmd[0] = SYNCHRONIZE_CACHE;
	memset(&cmnd, 0, sizeof(cmnd));
	ufshcd_compose_scsi_cmd(&cmnd, sdp, cmd, sync_cache_buffer, DMA_NONE, NULL, 0, 0);
	ret = ufshcd_sync_cache_irq_safe(hba, &cmnd, (unsigned int)100000);
	dev_err(hba->dev, "UFS:<%s> Emergency sync cache lun=%llu ret = %d \r\n", __func__, sdp->lun, ret);
	return ret;
}

int ufshcd_direct_flush(struct scsi_device *sdev)
{
	int ret;
	unsigned long flags;
	struct Scsi_Host *host = NULL;
	struct ufs_hba *hba = NULL;

	if (!sdev ) {
		pr_err("%s, sdev is null!\n",__func__);
		return -ENODEV;
	}
	host = sdev->host;
	hba = shost_priv(host);
	if (!hba ) {
		pr_err( "%s, hba is null!\n",__func__);
		return -ENODEV;
	}

	/* If ufs is suspended, it's uncessary to flush the cache */
	if (hba->is_sys_suspended || hba->pm_op_in_progress){
		dev_err(hba->dev, "%s, sys has suspended!\n",__func__);
		return 0;
	}
	/* Get hostlock timeout, the abnormal context may have the locker */
	if (!ufshcd_trylock_hostlock(hba, &flags)) {
		dev_err(hba->dev, "%s, can't get the hostlock!\n",__func__);
		return -EIO;
	}
	ret = scsi_device_get(sdev);
	if (!ret && !scsi_device_online(sdev)) {
		dev_err(hba->dev, "%s, scsi_device_get error or device not online, %d\n",__func__, ret);
		ret = -ENODEV;
	}
	if (hba->ufshcd_state != UFSHCD_STATE_OPERATIONAL) {
		dev_err(hba->dev, "%s, ufshcd_state isn't operational, ufshcd_state = %d\n",__func__, hba->ufshcd_state);
		ret = SCSI_MLQUEUE_HOST_BUSY;
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	if (ret) {
		scsi_device_put(sdev);
		return ret;
	}
	ret = ufshcd_send_scsi_sync_cache_on_specified_disk(hba, sdev);
	scsi_device_put(sdev);
	return ret;
}
#endif

#ifndef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
int ufshcd_keyregs_remap_wc(struct ufs_hba *hba,
	resource_size_t hci_reg_base)
{
	u32 ccap;
	u8 cfgptr;
	u8 cfgc;

	void __iomem *key_reg_base;

	if (ufshcd_support_inline_encrypt(hba)) {
		ccap = ufshcd_readl(hba, 0x100);
		cfgptr = (u8)(ccap >> 24);
		cfgc = (u8)((ccap & 0x0000FF00) >> 8);
		dev_info(hba->dev, "%s: ccap=0x%x, cfgptr=0x%x, cfgc=0x%x\n",
			__func__, ccap, cfgptr, cfgc);

		key_reg_base = ioremap_wc((phys_addr_t)(hci_reg_base + cfgptr * 0x100),
				(size_t)((cfgc + 1) * 0x80));
		if (!key_reg_base) {
			dev_err(hba->dev, "key regs remap error!\n");
			return -ENOMEM;
		}

		hba->key_reg_base = key_reg_base;
	}

	return 0;
}
#endif
#endif

int ufshcd_query_attr_safe(struct ufs_hba *hba, enum query_opcode opcode,
	enum attr_idn idn, u8 index, u8 selector, u32 *attr_val)
{
	int err;
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->ufshcd_state != UFSHCD_STATE_OPERATIONAL) {
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return -EBUSY;
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	/* hold error handler, reset may cause send command a noc error */
	mutex_lock(&hba->eh_mutex);

	err = ufshcd_query_attr(hba, opcode, idn, index, selector, attr_val);

	mutex_unlock(&hba->eh_mutex);
	return err;
}

static int ufshcd_read_device_health_desc(struct ufs_hba *hba,
					u8 *buf, u32 size)
{
	return ufshcd_read_desc_param(hba, QUERY_DESC_IDN_HEALTH, 0, 0, buf, size);
}

void ufs_get_device_health_info(struct ufs_hba *hba)
{
	int err;
	uint8_t desc_buf[QUERY_DESC_HEALTH_MAX_SIZE];
	u8 pre_eol_info;
	u8 life_time_est_typ_a;
	u8 life_time_est_typ_b;

	err =
	    ufshcd_read_device_health_desc(hba, desc_buf, QUERY_DESC_HEALTH_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: Failed getting device health info\n", __func__);
		goto out;
	}

	pre_eol_info = desc_buf[HEALTH_DEVICE_DESC_PARAM_PREEOL];
	life_time_est_typ_a = desc_buf[HEALTH_DEVICE_DESC_PARAM_LIFETIMEA];
	life_time_est_typ_b = desc_buf[HEALTH_DEVICE_DESC_PARAM_LIFETIMEB];

	if (strstr(saved_command_line, "androidboot.swtype=factory") &&
	    (life_time_est_typ_a > 1 || life_time_est_typ_b > 1)) {
		dsm_ufs_update_error_info(hba, DSM_UFS_LIFETIME_EXCCED_ERR);
		dev_err(hba->dev,
			"%s: life_time_est_typ_a = %d, life_time_est_typ_b = %d\n",
			__func__, life_time_est_typ_a, life_time_est_typ_b);
		dsm_ufs_update_lifetime_info(life_time_est_typ_a,
					     life_time_est_typ_b);
		if (dsm_ufs_enabled())
			schedule_work(&hba->dsm_work);
	}
#ifdef CONFIG_BOOTDEVICE
	set_bootdevice_pre_eol_info(pre_eol_info);
	set_bootdevice_life_time_est_typ_a(life_time_est_typ_a);
	set_bootdevice_life_time_est_typ_b(life_time_est_typ_b);
#endif

#if defined(CONFIG_BOOTDEVICE) && defined(CONFIG_HUAWEI_STORAGE_ROFA)
	if (get_bootdevice_type() == BOOT_DEVICE_UFS)
		storage_rochk_record_bootdevice_pre_eol_info(pre_eol_info);
#endif

out:
	return;
}

static int ufshcd_read_geometry_desc(struct ufs_hba *hba, u8 *buf, u32 size)
{
	return ufshcd_read_desc_param(hba, QUERY_DESC_IDN_GEOMETRY, 0, 0, buf, size);
}

void ufs_get_geometry_info(struct ufs_hba *hba)
{
	int err;
	uint8_t desc_buf[QUERY_DESC_GEOMETRY_MAX_SIZE];
	u64 total_raw_device_capacity;
#ifdef CONFIG_BOOTDEVICE
	u8 rpmb_read_write_size = 0;
	u64 rpmb_read_frame_support = 0;
	u64 rpmb_write_frame_support = 0;
#endif
	err =
	    ufshcd_read_geometry_desc(hba, desc_buf, QUERY_DESC_GEOMETRY_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: Failed getting geometry info\n", __func__);
		goto out;
	}
	total_raw_device_capacity =
		((u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 0] << 56) |
		((u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 1] << 48) |
		((u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 2] << 40) |
		((u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 3] << 32) |
		((u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 4] << 24) |
		((u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 5] << 16) |
		((u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 6] << 8) |
		((u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 7] << 0);

#ifdef CONFIG_BOOTDEVICE
	set_bootdevice_size(total_raw_device_capacity);
	rpmb_read_write_size = (u8)desc_buf[GEOMETRY_DESC_PARAM_RPMB_RW_SIZE];
	/* we set rpmb support frame, now max is 64 */
	if (rpmb_read_write_size > MAX_FRAME_BIT) {
		dev_err(hba->dev,
			"%s: rpmb_read_write_size is 0x%x larger than 64, we set default value is 64\n",
			__func__, rpmb_read_write_size);
		rpmb_read_write_size = MAX_FRAME_BIT;
	}
	rpmb_read_frame_support =
		(((uint64_t)1 << (rpmb_read_write_size - 1)) - 1) |
		(((uint64_t)1 << (rpmb_read_write_size - 1)));
	rpmb_write_frame_support =
		(((uint64_t)1 << (rpmb_read_write_size - 1)) - 1) |
		(((uint64_t)1 << (rpmb_read_write_size - 1)));
	set_rpmb_read_frame_support(rpmb_read_frame_support);
	set_rpmb_write_frame_support(rpmb_write_frame_support);
#endif
#ifdef CONFIG_MAS_BLK
	if (hba->host->queue_quirk_flag &
		SHOST_QUIRK(SHOST_QUIRK_IO_LATENCY_PROTECTION)) {
		if (hba->ufs_device_spec_version == UFS_DEVICE_SPEC_3_1)
			blk_mq_tagset_vl_setup_ufs_3_1(&hba->host->tag_set,
				total_raw_device_capacity);
		else if (hba->ufs_device_spec_version == UFS_DEVICE_SPEC_3_0)
			blk_mq_tagset_vl_setup(&hba->host->tag_set,
				total_raw_device_capacity);
	}
#endif
out:
	return;
}

#ifdef CONFIG_MAS_BLK
void ufshcd_dump_status(
	struct Scsi_Host *host, enum blk_dump_scene dump_type)
{
	struct ufs_hba *hba = shost_priv(host);
	struct ufshcd_lrb *lrbp = NULL;
	int tag;

	dev_err(hba->dev, "ufshcd: lrb_in_use = 0x%lx, ufshcd_state: 0x%x shost_state 0x%x host_self_blocked: 0x%x\n",
			hba->lrb_in_use, hba->ufshcd_state, host->shost_state, host->host_self_blocked);
	if (dump_type != BLK_DUMP_PANIC)
		return;

	dev_err(hba->dev, "current ktime %lld, error_handle = %u\n",
		ktime_to_us(ktime_get()), hba->error_handle_count);
#ifdef CONFIG_HUFS_HC
	dev_err(hba->dev,
		"last_intr = 0x%x, last_vs_intr = 0x%x, last_unipro_intr = 0x%x, last_fatal = 0x%x(%lld)\n",
		hba->last_intr, hba->last_vs_intr, hba->last_unipro_intr,
		hba->last_fatal_intr, hba->last_fatal_intr_time_stamp);
	dev_err(hba->dev, "last_core = 0x%x, last_core_intr = 0x%x(%lld)\n",
		hba->last_core, hba->last_core_intr,
		hba->last_core_intr_time_stamp);
#endif
#ifdef CONFIG_DFA_MCQ
	if (is_dfa_enabled(hba))
		return dfa_dump_lrbp_status(hba);
#endif
	for_each_set_bit (tag, &hba->outstanding_reqs, hba->nutrs) {
		lrbp = &hba->lrb[tag];
		dev_err(hba->dev,
			"UPIU[%d] - tag %d - cmd_type %d - opcode 0x%x - issue time %lld - complete time %lld\n",
			tag, lrbp->req_tag, lrbp->command_type, lrbp->opcode,
			ktime_to_us(lrbp->issue_time_stamp),
			ktime_to_us(lrbp->compl_time_stamp));
	}
}
#endif

static int ufshcd_device_get_rev(const struct device *dev, char *rev)
{
	struct ufs_hba *hba = NULL;
	int ret;
	const int buffer_len = 36;
	char buffer[buffer_len];
	unsigned char cmd[6] = {INQUIRY, 0, 0, 0,
				(char)buffer_len, 0};

	hba = (struct ufs_hba *)dev_get_drvdata(dev);
	if (!hba)
		return -EINVAL;

	ret = scsi_execute_req(hba->sdev_ufs_device, cmd,
				DMA_FROM_DEVICE, buffer,
				buffer_len, NULL,
				msecs_to_jiffies(30000), 3, NULL);
	if (ret)
		dev_err(hba->dev, "%s: failed with err %d\n",
			__func__, ret);
	else
		snprintf(rev, 5, "%.4s", buffer + 32);

	return ret;
}

#define MAX_REV_LEN 8
void ufshcd_rev_store(struct ufs_hba *hba)
{
	char dev_rev[MAX_REV_LEN + 1] = {0};
	int rev;
	int ret = ufshcd_device_get_rev(hba->dev, dev_rev);
	if (ret) {
		dev_err(hba->dev, "%s:get rev fail:%d\n", __func__, ret);
		return;
	}
	rev = (int)simple_strtoul(dev_rev, NULL, 10);
	hba->dev_info.fw_version = rev;
	dev_err(hba->dev, "%s:ufs fw version:%d!\n", __func__, rev);
}

#define HIGH_TEMP_SUPPORT_FW_REV 27
/**
 * ufshcd_high_temp_config - config TOO_HIGH_TEMP enable for Hi1863
 */
void ufshcd_high_temp_config(struct ufs_hba *hba)
{
#ifndef UFS_HIGH_TEMP_DISABLE
	int ret;
	if (!strstarts(hba->model, UFS_MODEL_THOR935)) {
		dev_info(hba->dev, "%s:not THOR935!\n", __func__);
		return;
	}

	if (!(hba->d_ext_ufs_feature_sup & UFS_DEV_TOO_HIGH_TEMP_SUPPORT)) {
		dev_info(hba->dev, "%s:not support too high temp feature!\n", __func__);
		return;
	}

	/* support firmware >= B027 */
	if (hba->dev_info.fw_version % 100 < HIGH_TEMP_SUPPORT_FW_REV) {
		dev_info(hba->dev, "%s:not support firmware!\n", __func__);
		return;
	}
	ret = ufshcd_enable_ee(hba, MASK_EE_TOO_HIGH_TEMP);
	if (ret)
		dev_err(hba->dev, "%s:enable high temp ee fail:%d\n", __func__, ret);
#endif
}
/**
 * ufshcd_ee_reset - reset exception event while ufshc reset
 */
void ufshcd_ee_reset(struct ufs_hba *hba)
{
	u32 val = hba->ee_ctrl_mask & MASK_EE_STATUS;
	(void)ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_WRITE_ATTR,
			QUERY_ATTR_IDN_EE_CONTROL, 0, 0, &val);
}

void ufs_dev_too_high_temp_event_handler(struct ufs_hba *hba)
{
	dev_err(hba->dev, "%s:UFS DEV HIGH-TEMPERATURE-PROTECT:RESET!\n", __func__);
	rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
}

void ufs_get_info(struct ufs_hba *hba)
{
	ufs_get_geometry_info(hba);
	if (likely(!hba->host->is_emulator))
		ufs_get_device_health_info(hba);

#ifdef CONFIG_AB_PARTITION_TABLE
	ufs_get_boot_partition_type(hba);
#endif

#if defined(CONFIG_RPMB_UFS) || defined(CONFIG_SCSI_UFS_VIRTUALIZ_SERVICE)
	if (get_bootdevice_type() == BOOT_DEVICE_UFS) {
		set_bootdevice_rev_handler(ufshcd_device_get_rev);
		ufs_get_rpmb_info(hba);
		dev_err(hba->dev, "---------ufs rpmb info----------\n");
		dev_err(hba->dev, "rpmb_region_enable:0x%x\n",get_rpmb_region_enable());
		dev_err(hba->dev, "rpmb_total_blks:%llu\n",get_rpmb_total_blks());
	}
#endif
}

void ufshcd_device_not_support_autoh8(struct ufs_hba *hba)
{
	if ((hba->manufacturer_id == UFS_VENDOR_HI1861) &&
		(strstarts(hba->model, UFS_MODEL_SS6100))) {
		if (ufshcd_is_auto_hibern8_allowed(hba) &&
			(!(hba->caps & DISABLE_UFS_PMRUNTIME))) {
			ufshcd_disable_auto_hibern8(hba);
			pm_runtime_allow(hba->dev);
			hba->caps &= (u32)(~UFSHCD_CAP_AUTO_HIBERN8);
			hba->quirks |= UFSHCD_QUIRK_BROKEN_AUTO_HIBERN8;
			dev_err(hba->dev, "not support autoH8\n");
		}
	}
}

static void ufs_idle_intr_check_timer_expire(struct timer_list *t)
{
	struct ufs_hba *hba = from_timer(hba, t, idle_intr_check_timer);

	dev_err(hba->dev, "Haven't got Idle interrupt over %d seconds\n",
		    hba->idle_intr_check_timer_threshold / 1000);
}

/**
 * ufshcd_add_idle_intr_check_timer - if Idle interrupt is never raised for UFSHCD_IDLE_INTR_CHECK_INTERVAL,
 *     we think something goes wrong.
 * @hba: per adapter instance
 */
void ufshcd_add_idle_intr_check_timer(struct ufs_hba *hba)
{
	hba->idle_intr_check_timer_threshold = UFSHCD_IDLE_INTR_CHECK_INTERVAL;
	timer_setup(&hba->idle_intr_check_timer, ufs_idle_intr_check_timer_expire, 0);
	mod_timer(&hba->idle_intr_check_timer, jiffies + msecs_to_jiffies(hba->idle_intr_check_timer_threshold));
}

/**
 * ufshcd_timeout_handle_intr - UFS timeout interrupts service routine,
 *     including Idle_intr, device_stat_timeout_intr, UFS_LA_intr, pwm_timeout_intr and auto-H8_timeout_intr.
 * @timer_irq: the interrupt line.
 * @hba: per adapter instance
 */
irqreturn_t ufshcd_timeout_handle_intr(int timer_irq, void *__hba)
{
	struct ufs_hba *hba = __hba;
	struct hufs_host *host;
	u32 debug_stat;

	host = (struct hufs_host *)hba->priv;
	spin_lock(hba->host->host_lock);
	/* PWM and Device Timeout handler */
	debug_stat = ufs_sys_ctrl_readl(host, UFS_DEBUG_STAT);
	if (debug_stat & (BIT_UFS_PWR_INTR | BIT_UFS_DEV_TMT_INTR)) {
		if (hba->ufshcd_state == UFSHCD_STATE_OPERATIONAL) {
			dev_err(hba->dev, "%s, debug_stat 0x%x\n", __func__,
				debug_stat);
			__ufshcd_disable_dev_tmt_cnt(hba);
			__ufshcd_disable_pwm_cnt(hba);

			/* block commands from scsi mid-layer */
			scsi_block_requests(hba->host);

			hba->force_reset = true;
			hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED;
			kthread_queue_work(&hba->eh_worker, &hba->eh_work);
		}
	}
	/* Idle Timeout handler */
	if (likely(hba->ufs_idle_intr_en)) {
		if (!hba->idle_intr_disabled) {
			debug_stat = ufs_sys_ctrl_readl(host, UFS_DEBUG_STAT);
			if (debug_stat & BIT_UFS_IDLE_TIMEOUT) {
				ufs_sys_ctrl_writel(host, 1, UFS_IDLE_CONUTER_CLR);
				ufshcd_idle_handler(hba);
			}
		} else {
			ufshcd_idle_handler(hba);
		}
	}
	spin_unlock(hba->host->host_lock);

	return IRQ_HANDLED;
}

/**
 * ufs_idle_intr_toggle - enable or disable UFS idle interrupt
 * @hba: per adapter instance
 * @enable: 0: disable, other: enable;
 */
void ufs_idle_intr_toggle(struct ufs_hba *hba, int enable)
{
	unsigned long flags;
#ifndef CONFIG_HUFS_HC
	struct hufs_host *host = (struct hufs_host *)hba->priv;
#endif

	if (!(hba->ufs_idle_intr_en))
		return;

	spin_lock_irqsave(hba->host->host_lock, flags);
	hba->idle_intr_disabled = !enable;
	if (enable) {
#ifdef CONFIG_HUFS_HC
		ufshcd_writel(hba, hba->idle_timeout_val / 1000,
			UFS_CFG_IDLE_TIME_THRESHOLD);
		ufshcd_hufs_enable_idle_tmt_cnt(hba);
#else
		ufs_sys_ctrl_set_bits(
			host, BIT_UFS_IDLE_CNT_TIMEOUT_MASK, UFS_DEBUG_CTRL);
		ufs_sys_ctrl_set_bits(host, BIT_UFS_IDLE_CNT_TIMEOUT_CLR,
			UFS_IDLE_CONUTER_CLR);

		/* Set idle timeout value */
		ufs_sys_ctrl_writel(
			host, hba->idle_timeout_val, UFS_IDLE_CONUTER);

		ufs_sys_ctrl_set_bits(
			host, BIT_UFS_IDLE_CNT_EN, UFS_DEBUG_CTRL);
		ufs_sys_ctrl_clr_bits(
			host, BIT_UFS_IDLE_CNT_TIMEOUT_MASK, UFS_DEBUG_CTRL);
#endif
	} else {
#ifdef CONFIG_HUFS_HC
		ufshcd_hufs_disable_idle_tmt_cnt(hba);
#else
		ufs_sys_ctrl_set_bits(host, BIT_UFS_IDLE_CNT_TIMEOUT_MASK, UFS_DEBUG_CTRL);
		ufs_sys_ctrl_clr_bits(host, BIT_UFS_IDLE_CNT_EN, UFS_DEBUG_CTRL);
		ufs_sys_ctrl_writel(host, 1, UFS_IDLE_CONUTER_CLR);
#endif
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

/**
 * ufshcd_idle_handler - UFS idle interrupt service routine
 * @hba: per adapter instance
 */
void ufshcd_idle_handler(struct ufs_hba *hba)
{
#ifdef CONFIG_MAS_BLK
	struct blk_dev_lld *lld = &(hba->host->tag_set.lld_func);
#endif
#ifdef CONFIG_DFX_DEBUG_FS
	static DEFINE_RATELIMIT_STATE(idle_print_rs, (30 * HZ), 1);
#ifdef CONFIG_HUFS_HC_CORE_UTR
	u64 utp_tx_doorbell;
#else
	u32 utp_tx_doorbell;
#endif
	u32 utp_task_doorbell;

	if (!hba->idle_intr_disabled) {
#ifdef CONFIG_HUFS_HC_CORE_UTR
		utp_tx_doorbell = read_core_utr_doorbells(hba);
#else
		utp_tx_doorbell = ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL);
#endif
		utp_task_doorbell = ufshcd_readl(hba, REG_UTP_TASK_REQ_DOOR_BELL);
		if (__ratelimit(&idle_print_rs)) {
			if (utp_tx_doorbell || utp_task_doorbell)
				dev_err(hba->dev, "%s,Got Idle interrupt while utp_tx_doorbell: 0x%llx, utp_task_doorbell: 0x%x\n",
						__func__, (u64)utp_tx_doorbell, utp_task_doorbell);
			else if (unlikely(hba->ufs_idle_intr_verify))
				dev_info(hba->dev, "%s, Idle interrupt, all the doorbell is 0\n", __func__);
		}
	}

	mod_timer(&hba->idle_intr_check_timer, jiffies + msecs_to_jiffies(hba->idle_intr_check_timer_threshold));
#endif /* CONFIG_DFX_DEBUG_FS */

#ifdef CONFIG_MAS_BLK
	blk_lld_idle_notify(lld);
#endif
	ufstt_idle_handler(hba, ktime_get());
	if (hba->is_samsung_v5)
		ufshcd_wb_idle_handler(hba, ktime_get());
}

void ufshcd_update_uic_reg_hist(struct ufs_uic_err_reg_hist *reg_hist,
		u32 reg)
{
	reg_hist->reg[reg_hist->pos] = reg;
	reg_hist->tstamp[reg_hist->pos] = ktime_get();
	reg_hist->pos = (reg_hist->pos + 1) % UIC_ERR_REG_HIST_LENGTH;
}

bool ufshcd_error_need_queue_work(struct ufs_hba *hba)
{
	bool queue_eh_work = false;

	if ((hba->errors & INT_FATAL_ERRORS) ||
		(ufshcd_is_auto_hibern8_allowed(hba) &&
			(hba->errors &
				(UIC_HIBERNATE_ENTER | UIC_HIBERNATE_EXIT)))) {
		queue_eh_work = true;
		if (ufshcd_is_auto_hibern8_allowed(hba) &&
			(hba->errors &
				(UIC_HIBERNATE_ENTER | UIC_HIBERNATE_EXIT)))
			dsm_ufs_update_error_info(
				hba, DSM_UFS_ENTER_OR_EXIT_H8_ERR);
		if (hba->errors & CONTROLLER_FATAL_ERROR)
			dsm_ufs_update_error_info(hba, DSM_UFS_CONTROLLER_ERR);
		if (hba->errors & DEVICE_FATAL_ERROR)
			dsm_ufs_update_error_info(hba, DSM_UFS_DEV_ERR);
		if (hba->errors & SYSTEM_BUS_FATAL_ERROR)
			dsm_ufs_update_error_info(hba, DSM_UFS_SYSBUS_ERR);
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
		if (hba->errors & CRYPTO_ENGINE_FATAL_ERROR)
			dsm_ufs_updata_ice_info(hba);
#endif
	}
#ifdef CONFIG_HUFS_HC
	if (ufshcd_is_auto_hibern8_allowed(hba) &&
		(hba->errors & AH8_EXIT_REQ_CNF_FAIL_INTR)) {
		queue_eh_work = true;
		dsm_ufs_update_error_info(hba, DSM_UFS_ENTER_OR_EXIT_H8_ERR);
	}
#endif

	return queue_eh_work;
}

void ufshcd_config_adapt(struct ufs_hba *hba, struct ufs_pa_layer_attr *pwr_mode)
{
	uint32_t unipro_ver;
	uint32_t unipro_rver;
	uint32_t unipro_lver;

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_REMOTEVERINFO),
			&unipro_rver);
	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_LOCALVERINFO),
			&unipro_lver);
	unipro_rver = unipro_rver & UNIPRO_VER_MASK;
	unipro_lver = unipro_lver & UNIPRO_VER_MASK;
	unipro_ver = (unipro_rver > unipro_lver) ? unipro_lver : unipro_rver;
	if (unipro_ver < UNIPRO_VER_1_8)
		return;

	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXHSADAPTTYPE), PA_HS_APT_NO);
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_ADAPTAFTERLRSTINPA_INIT), PA_HS_APT_NO);
#ifdef CONFIG_HUFS_HC
	if ((hba->manufacturer_id == UFS_VENDOR_TOSHIBA) ||
	    (hba->manufacturer_id == UFS_VENDOR_SKHYNIX) ||
	    (hba->manufacturer_id == UFS_VENDOR_SAMSUNG)) {
		if ((pwr_mode->pwr_rx == FASTAUTO_MODE ||
		     pwr_mode->pwr_rx == FAST_MODE) &&
		    (pwr_mode->gear_rx >= UFS_HS_G4)) {
			ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXHSADAPTTYPE),
				PA_HS_APT_INITIAL);
			ufshcd_dme_set(hba, UIC_ARG_MIB(PA_ADAPTAFTERLRSTINPA_INIT),
				PA_HS_APT_INITIAL);
		}
	}
#else
	if ((hba->manufacturer_id == UFS_VENDOR_TOSHIBA) ||
	    (hba->manufacturer_id == UFS_VENDOR_MICRON) ||
	    (hba->manufacturer_id == UFS_VENDOR_SAMSUNG)) {
		if ((pwr_mode->pwr_rx == FASTAUTO_MODE ||
		     pwr_mode->pwr_rx == FAST_MODE) &&
		    (pwr_mode->gear_rx >= UFS_HS_G4)) {
			ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXHSADAPTTYPE),
				PA_HS_APT_INITIAL);
			ufshcd_dme_set(hba, UIC_ARG_MIB(PA_ADAPTAFTERLRSTINPA_INIT),
				PA_HS_APT_INITIAL);
		}
	}
#endif
}

int __ufshcd_wait_for_doorbell_clr(struct ufs_hba *hba)
{
#ifdef CONFIG_HUFS_HC_CORE_UTR
	u32 tm_doorbell, wait_timeout_us;
	u64 tr_doorbell;
#else
	u32 tr_doorbell, tm_doorbell, wait_timeout_us;
#endif
	int ret = 0;
	ktime_t start = ktime_get();

	wait_timeout_us = 2000000;
	tm_doorbell = ufshcd_readl(hba, REG_UTP_TASK_REQ_DOOR_BELL);
#ifdef CONFIG_HUFS_HC_CORE_UTR
	tr_doorbell = read_core_utr_doorbells(hba);
#else
	tr_doorbell = ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL);
#endif
	if ((!tm_doorbell) && (!tr_doorbell))
		return 0;

	__ufshcd_print_doorbell(hba, tm_doorbell, tr_doorbell, "begin");
	/*
	 * Wait for all the outstanding tasks/transfer requests.
	 * Verify by checking the doorbell registers are clear.
	 */
	do {
		tm_doorbell = ufshcd_readl(hba, REG_UTP_TASK_REQ_DOOR_BELL);
#ifdef CONFIG_HUFS_HC_CORE_UTR
		tr_doorbell = read_core_utr_doorbells(hba);
#else
		tr_doorbell = ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL);
#endif
		/*lint -save -e446*/
		if (ktime_to_us(ktime_sub(ktime_get(), start)) >
			wait_timeout_us) {
			ret = -EIO;
			dev_err(hba->dev, "wait doorbell clean timeout\n");
			hufs_key_reg_dump(hba);
			break;
		}
		/*lint -restore*/
	} while (tm_doorbell || tr_doorbell);
	__ufshcd_print_doorbell(hba, tm_doorbell, tr_doorbell, "end");
	return ret;
}

#ifdef CONFIG_MAS_HEALTH
int ufshcd_get_health_info(struct scsi_device *sdev,
	u8 *pre_eol_info, u8 *life_time_est_a, u8 *life_time_est_b)
{
	int ret;
	struct ufs_hba *hba = NULL;
	u8 buff[QUERY_DESC_HEALTH_MAX_SIZE];

	if ((!sdev) || (!pre_eol_info) || (!life_time_est_a) ||
		(!life_time_est_b))
		return -EFAULT;

	hba = shost_priv(sdev->host);
	if (!hba)
		return -EFAULT;

	ret = ufshcd_read_device_health_desc(hba, buff,
		QUERY_DESC_HEALTH_MAX_SIZE);
	if (ret) {
		dev_err(hba->dev, "%s: Failed getting device health info\n",
			__func__);
		return ret;
	}

	*pre_eol_info = buff[HEALTH_DEVICE_DESC_PARAM_PREEOL];
	*life_time_est_a = buff[HEALTH_DEVICE_DESC_PARAM_LIFETIMEA];
	*life_time_est_b = buff[HEALTH_DEVICE_DESC_PARAM_LIFETIMEB];

	return 0;
}
#endif

/**
 * ufs_ffu_pm_runtime_delay_enable - when ffu_pm_work in workqueue is scheduled, after 30 allow pm_runtime
 * and unlock the wake lock
* @work: pointer to work structure
 *
 */
void ufs_ffu_pm_runtime_delay_enable(struct work_struct *work)
{
	struct ufs_hba *hba = NULL;
	int err;
	unsigned long flags;

	hba = container_of(work, struct ufs_hba, ffu_pm_work);
	/* follow scsi command timeout value 2s */
	msleep(2000);
	mutex_lock(&hba->eh_mutex);
	spin_lock_irqsave(hba->host->host_lock, flags);
	hba->ufshcd_state = UFSHCD_STATE_RESET;
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_KM
	gufs_sk_state = UFSHCD_SK_STATE_INVAL;
#endif
	ufshcd_set_eh_in_progress(hba);
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	err = ufshcd_reset_and_restore(hba);

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (!err)
		hba->ufshcd_state = UFSHCD_STATE_OPERATIONAL;
	else
		hba->ufshcd_state = UFSHCD_STATE_ERROR;
	ufshcd_clear_eh_in_progress(hba);
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	pm_runtime_set_active(hba->dev);
	mutex_unlock(&hba->eh_mutex);
	/* wait 27s, this msleep does not effect ffu reset time */
	msleep(27000);

	if (ffu_lock->active) {
		/*lint -save -e455*/
		__pm_relax(ffu_lock);
		/*lint -restore*/
		dev_err(hba->dev, "ffu unlock wake lock.\n");
	}
	if (ufshcd_is_auto_hibern8_allowed(hba))
		ufshcd_enable_auto_hibern8(hba);
	else
		pm_runtime_allow(hba->dev);
}

static int ufs_timeout_count = 0;
#define UFS_TIMEOUT_SERIOUS_THRESHOLD  3
enum blk_eh_timer_return ufshcd_eh_timed_out(struct scsi_cmnd *scmd)
{
	unsigned long flags;
	struct Scsi_Host *host = NULL;
	struct ufs_hba *hba = NULL;
	int index;
	bool found = false;
	int tag = -1;

	if (!scmd || !scmd->device || !scmd->device->host)
		return BLK_EH_DONE;

	host = scmd->device->host;
	hba = shost_priv(host);
	if (!hba)
		return BLK_EH_DONE;

#ifdef CONFIG_DFA_MCQ
	if (is_dfa_enabled(hba))
		return dfa_eh_timed_out(scmd);
#endif

	spin_lock_irqsave(host->host_lock, flags);

	for_each_set_bit(index, &hba->outstanding_reqs, hba->nutrs) {
		if (hba->lrb[index].cmd == scmd) {
			found = true;
			tag = index;
			break;
		}
	}
	spin_unlock_irqrestore(host->host_lock, flags);

	if (true == found) {
		dsm_ufs_update_upiu_info(hba, tag, DSM_UFS_TIMEOUT_ERR);
		/*
		* Report DSM_UFS_HARDWARE_RESET when doorbell timeout has
		* been recovered by ufshcd_host_reset_and_restore over
		* UFS_TIMEOUT_SERIOUS_THRESHOLD times. ufs_timeout_count
		* will not increase if ufshcd_host_reset_and_restore not
		* called.
		*/
		if (ufs_timeout_lock != 1) {
			ufs_timeout_lock = 1;
			ufs_timeout_count++;
		}

		schedule_ufs_dsm_work(hba);
	} else {
		dev_warn( hba->dev,\
			"scsi cmd[%x] with tag[%x] is timeout which can't be found.",
			scmd->cmnd[0], scmd->request->tag );
	}
	/*
	 * Bypass SCSI error handling and reset the block layer timer if this
	 * SCSI command was not actually dispatched to UFS driver, otherwise
	 * let SCSI layer handle the error as usual.
	 */
	return (enum blk_eh_timer_return)(found ? BLK_EH_DONE : BLK_EH_RESET_TIMER);
}

/**
 * ufshcd_get_tmr_ocs - Get the UTMRD Overall Command Status
 * @task_req_descp: pointer to utp_task_req_desc structure
 *
 * This function is used to get the OCS field from UTMRD
 * Returns the OCS field in the UTMRD
 */
static inline int ufshcd_get_tmr_ocs(struct utp_task_req_desc *task_req_descp)
{
	return le32_to_cpu(task_req_descp->header.dword_2) & MASK_OCS;
}

#ifdef CONFIG_SCSI_HUFS
/**
 * ufshcd_task_req_compl - handle task management request completion
 * @hba: per adapter instance
 * @index: index of the completed request
 * @resp: task management service response
 *
 * Returns non-zero value on error, zero on success
 */
int ufshcd_task_req_compl(struct ufs_hba *hba, u32 index, u8 *resp)
{
	struct utp_task_req_desc *treq;
	unsigned long flags;
	int ocs_value;
	int task_result;

	spin_lock_irqsave(hba->host->host_lock, flags);

	/* Clear completed tasks from outstanding_tasks */
	__clear_bit(index, &hba->outstanding_tasks);

	treq = hba->utmrdl_base_addr;
	ocs_value = ufshcd_get_tmr_ocs(&treq[index]);
	ufsdbg_error_inject_dispatcher(hba,
		ERR_INJECT_TM_OCS,
		TM_OCS_SUCCESS,
		&ocs_value);
	if (ocs_value == OCS_SUCCESS) {
		task_result = be32_to_cpu(treq->output_param1) &
				MASK_TM_SERVICE_RESP;
		ufsdbg_error_inject_dispatcher(hba,
			ERR_INJECT_TM_RSP,
			UPIU_TASK_MANAGEMENT_FUNC_SUCCEEDED,
			&task_result);
		if (resp)
			*resp = (u8)task_result;
	} else {
		dev_err(hba->dev, "%s: failed, ocs = 0x%x\n",
				__func__, ocs_value);
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	return ocs_value;
}
#endif

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
void ufs_check_pwm_after_h8(struct ufs_hba *hba, bool h8_op)
{
	if (h8_op || !hba->check_pwm_after_h8)
		return;

	if (!hba->vops->get_pwr_by_debug_register) {
		dev_err(hba->dev, "no check pwm op\n");
		hba->check_pwm_after_h8 = 0;
	} else {
		value = hba->vops->get_pwr_by_debug_register(hba);
		if (value == SLOW) {
			dev_err(hba->dev, "ufs pwr = 0x%x after H8\n", value);
			hba->check_pwm_after_h8 = 0;
			ufshcd_init_pwr_info(hba);
			if (!work_busy(&hba->recover_hs_work))
				schedule_work(&hba->recover_hs_work);
			else
				dev_err(hba->dev,
					"%s:recover_hs_work is runing \n",
					__func__);
		} else {
			hba->check_pwm_after_h8--;
		}
		dev_err(hba->dev, "check pwr after H8, %d times remain\n",
			hba->check_pwm_after_h8);
	}
}
#endif

int __ufshcd_uic_hibern8_op_irq_safe(struct ufs_hba *hba, bool h8_op)
{
	u32 value;
	u32 ie_value;
	int ret;
	u32 mask;
	u8 status;

	/* emulator have no real MPHY */
	if (unlikely(hba->host->is_emulator))
		return 0;
	if (ufshcd_is_hufs_hc(hba) && !ufshcd_hufs_use_standard_uic(hba) && hba->vops &&
		hba->vops->ufshcd_hibern8_op_irq_safe)
		return hba->vops->ufshcd_hibern8_op_irq_safe(hba, h8_op);

	/* step 1: close interrupt and save interrupt value */
	ie_value = ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	ufshcd_writel(hba, 0, REG_INTERRUPT_ENABLE);

	/* step 2: send uic cmd */
	ret = ufshcd_wait_for_register_poll(hba, REG_CONTROLLER_STATUS,
					    UIC_COMMAND_READY,
					    HUFS_H8_TIMEOUT_RETRY_TIMES,
					    HUFS_H8_TIMEOUT_RETRY_UDELAY);
	if (ret) {
		dev_err(hba->dev, "%s wait for uic ready timeout\n", __func__);
		goto out;
	}

	ufshcd_writel(hba, 0, REG_UIC_COMMAND_ARG_1);
	ufshcd_writel(hba, 0, REG_UIC_COMMAND_ARG_2);
	ufshcd_writel(hba, 0, REG_UIC_COMMAND_ARG_3);
	if (h8_op) /* HUFS_H8_OP_ENTER */
		ufshcd_writel(hba, UIC_CMD_DME_HIBER_ENTER, REG_UIC_COMMAND);
	else /* HUFS_H8_OP_EXIT */
		ufshcd_writel(hba, UIC_CMD_DME_HIBER_EXIT, REG_UIC_COMMAND);

	ret = ufshcd_wait_for_register_poll(hba, REG_INTERRUPT_STATUS,
					    UIC_COMMAND_COMPL,
					    HUFS_H8_TIMEOUT_RETRY_TIMES,
					    HUFS_H8_TIMEOUT_RETRY_UDELAY);
	if (ret) {
		dev_err(hba->dev, "[%s]wait UCCS time out\n",
			h8_op ? "ENTER_H8" : "EXIT_H8");
		ret = -EIO;
		goto out;
	}
	ufshcd_writel(hba, UIC_COMMAND_COMPL, REG_INTERRUPT_STATUS);
	value = ufshcd_readl(hba, REG_UIC_COMMAND_ARG_2);
	if ((value & 0xFF) != 0) {
		dev_err(hba->dev, "[%s]uic err, arg2 is 0x%x\n",
			h8_op ? "ENTER_H8" : "EXIT_H8", value);
		ret = -EIO;
		goto out;
	}

	/* step 3: check UHES/UHXS */
	if (h8_op) /* HUFS_H8_OP_ENTER */
		mask = UIC_HIBERNATE_ENTER;
	else /* HUFS_H8_OP_EXIT */
		mask = UIC_HIBERNATE_EXIT;
	ret = ufshcd_wait_for_register_poll(hba, REG_INTERRUPT_STATUS, mask,
					    HUFS_H8_TIMEOUT_RETRY_TIMES,
					    HUFS_H8_TIMEOUT_RETRY_UDELAY);
	if (ret) {
		dev_err(hba->dev, "[%s]wait UHES time out\n",
			h8_op ? "ENTER_H8" : "EXIT_H8");
		ret = -EIO;
		goto out;
	}
	ufshcd_writel(hba, mask, REG_INTERRUPT_STATUS);

	/* step 4: check UPMCRS */
	status = (ufshcd_readl(hba, REG_CONTROLLER_STATUS) >> 8) & 0x7;
	if (status != PWR_LOCAL) {
		dev_err(hba->dev,
			"[%s]pwr ctrl cmd failed, host umpcrs:0x%x\n",
			h8_op ? "ENTER_H8" : "EXIT_H8", status);
		ret = (status != PWR_OK) ? -EIO : -1;
		goto out;
	}

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
	ufs_check_pwm_after_h8(hba, h8_op);
#endif
out:

	ufsdbg_error_inject_dispatcher(hba,
		ERR_INJECT_PWR_MODE_CHANGE_ERR,
		0,
		&ret);

	if (ret) {
		ufshcd_writel(hba, UIC_ERROR, REG_INTERRUPT_STATUS);
		dsm_ufs_update_error_info(hba, DSM_UFS_ENTER_OR_EXIT_H8_ERR);
		schedule_ufs_dsm_work(hba);
	}
	ufshcd_writel(hba, ie_value, REG_INTERRUPT_ENABLE);
	return ((ret <= 0) ? ret : -ret);
}

void ufs_update_linkup_err(struct ufs_hba *hba)
{
#ifdef CONFIG_HUAWEI_UFS_DSM
	unsigned long flags;
	u32 avail_lane_rx, avail_lane_tx;
	struct hufs_host *host = hba->priv;

	ufshcd_dme_peer_get(hba, UIC_ARG_MIB(PA_AVAILRXDATALANES),
			&avail_lane_rx);
	ufshcd_dme_peer_get(hba, UIC_ARG_MIB(PA_AVAILTXDATALANES),
			&avail_lane_tx);

	/* no need to do this when use one lane */
	if (((hba->max_pwr_info.info.lane_rx < avail_lane_rx) ||
		(hba->max_pwr_info.info.lane_tx < avail_lane_tx)) &&
		!(host->caps & USE_ONE_LANE)) {
		dev_err(hba->dev, "ufs line number is less than avail "
			"rx=%d, tx=%d, avail_rx=%d, avail_tx=%d\n",
			hba->max_pwr_info.info.lane_rx,
			hba->max_pwr_info.info.lane_tx,
			avail_lane_rx, avail_lane_tx);
		spin_lock_irqsave(hba->host->host_lock, flags);
		dsm_ufs_update_error_info(hba, DSM_UFS_LINKUP_ERR);
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		schedule_ufs_dsm_work(hba);
	}
#endif
#ifdef CONFIG_SCSI_HUFS_LINERESET_CHECK
	if (hba->bg_task_enable && hba->vops && hba->vops->background_thread) {
		if (!hba->background_task) {
			hba->background_task = kthread_run(hba->vops->background_thread,
				hba, "ufs_bg_thread");
			if (IS_ERR(hba->background_task))
				dev_err(hba->dev, "background_thread create fail! \r\n", __func__);
		}
	}
#endif
}

int ufs_switch_hs_check(struct ufs_hba *hba)
{
	int ret = 0;

	/* clean ufs ue intr */
	ufshcd_ue_clean(hba);

	/* confirm ufs works well after switch to hs mode */
	ret = ufshcd_verify_dev_init(hba);
	if (ret)
		return ret;
#ifdef CONFIG_HUFS_HC
	ufshcd_ue_clean(hba);
#endif
#ifdef CONFIG_SCSI_UFS_LUN_PROTECT
	ret = ufshcd_set_lun_protect(hba);
	if (ret)
		return ret;
#endif
#if defined(CONFIG_SCSI_UFS_HI1861_VCMD) && defined(CONFIG_PLATFORM_DIEID)
	ufshcd_ufs_set_dieid(hba, &hba->dev_info);
#endif

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	ufshcd_device_capbitlity_config(hba);
#endif

	hba->force_reset = false;
	/* set the state as operational after switching to desired gear */
	hba->ufshcd_state = UFSHCD_STATE_OPERATIONAL;

	return ret;
}

int ufs_extend_config_init(struct ufs_hba *hba)
{
	int ret = 0;

	/* get ufs ue intr status */
	ret = ufshcd_ue_get(hba);
	if (ret)
		return ret;
#ifdef CONFIG_SCSI_HUFS
	if (!ufshcd_eh_in_progress(hba) && !hba->pm_op_in_progress) {
		if (ufshcd_is_auto_hibern8_allowed(hba))
			ufshcd_enable_auto_hibern8(hba);
		if (hba->caps & UFSHCD_CAP_PWM_DAEMON_INTR)
			ufshcd_enable_pwm_cnt(hba);
		if (hba->caps & UFSHCD_CAP_DEV_TMT_INTR)
			ufshcd_enable_dev_tmt_cnt(hba);
	}
#endif

#if (defined(CONFIG_SCSI_UFS_SCORPIO) || defined(CONFIG_SCSI_UFS_SGR)) &&                        \
	defined(CONFIG_HUAWEI_UFS_DSM)
	dsm_ufs_update_error_info(hba, DSM_UFS_MPHY_CALIBRATION_INFO);
	schedule_ufs_dsm_work(hba);
#endif
	dsm_ufs_enable_uic_err(hba);
	ufs_idle_intr_toggle(hba, 1);

	return ret;
}

int ufs_blk_config_init(struct ufs_hba *hba)
{
	int ret = 0;

	hba->inline_encryto_ver = UFS_INLINE_CRYPTO_VER;

#ifdef CONFIG_SCSI_UFS_UNISTORE
	ret = ufshcd_unistore_init(hba);
	if (ret)
		return ret;
#endif

	/*
	 * Background operations need to be enabled for Sandisk ufs device.
	 * Hynix use manual bkops. And others are enabled by default.
	 */
	ufshcd_device_need_enable_autobkops(hba);

#ifdef CONFIG_HUFS_HC
	if (ufshcd_is_hufs_hc(hba))
		ufshcd_enable_clock_gating_autodiv(hba);
#endif

	hba->wlun_dev_clr_ua = true;
	return ret;
}

int ufshcd_hufs_extend_trc_handler(struct ufs_hba *hba, unsigned long completed_reqs)
{
	u32 tr_doorbell;
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	tr_doorbell = ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL);
	completed_reqs = tr_doorbell ^ hba->outstanding_reqs;

	if (completed_reqs) {
		__ufshcd_transfer_req_compl(hba, completed_reqs);
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return IRQ_HANDLED;
	} else {
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return IRQ_NONE;
	}
}
