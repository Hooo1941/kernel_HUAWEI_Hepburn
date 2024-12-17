/*
 * ufshcd_protocol.c
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

#include "ufshcd_protocol.h"
#include "ufshcd.h"
#include "ufshcd_extend.h"
#include "ufshcd_debug.h"
#include "ufs-hpb.h"

#ifdef CONFIG_HW_STAT_PSI_LRB
#include <linux/hw_stat.h>
#endif

/**
 * ufshcd_get_dev_cmd_tag - Get device management command tag
 * @hba: per-adapter instance
 * @tag: pointer to variable with available slot value
 *
 * Get a free slot and lock it until device management command
 * completes.
 *
 * Returns false if free slot is unavailable for locking, else
 * return true with tag value in @tag.
 */
bool ufshcd_get_dev_cmd_tag(struct ufs_hba *hba, int *tag_out)
{
	int tag;
	bool ret = false;
	unsigned long tmp;

#ifdef CONFIG_DFA_MCQ
	if (is_dfa_enabled(hba))
		return dfa_get_dev_cmd_tag(hba, tag_out);
#endif
	if (!tag_out)
		goto out;

	do {
		tmp = ~hba->lrb_in_use;
		tag = find_last_bit(&tmp, hba->nutrs);
		if (tag >= hba->nutrs)
			goto out;
	} while (test_and_set_bit_lock(tag, &hba->lrb_in_use));
#ifdef CONFIG_HW_STAT_PSI_LRB
	hw_stat_lrb_in_use_changed(hba->lrb_in_use);
#endif

	*tag_out = tag;
	ret = true;
out:
	return ret;
}

int wait_for_ufs_all_complete(struct ufs_hba *hba, int timeout_ms)
{
	u64 tr_doobell;

	if (!hba)
		return -EINVAL;
	while (timeout_ms-- > 0) {
		tr_doobell = read_utr_doorbell(hba);
		if (!tr_doobell)
			return 0;
		udelay(1000);
	}
	dev_err(hba->dev, "%s: tr_doobell: 0x%llx\n", __func__, tr_doobell);
	return -ETIMEDOUT;
}

int ufshcd_read_device_desc(struct ufs_hba *hba, u8 *buf, u32 size)
{
	return ufshcd_read_desc_param(hba, QUERY_DESC_IDN_DEVICE, 0, 0, buf, size);
}

static int ufshcd_clear_ua_wlun(struct ufs_hba *hba, u8 wlun)
{
	struct scsi_device *sdp = NULL;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (wlun == UFS_UPIU_UFS_DEVICE_WLUN)
		sdp = hba->sdev_ufs_device;
	else if (wlun == UFS_UPIU_RPMB_WLUN)
		sdp = hba->sdev_rpmb;
	else
		BUG();
	if (sdp) {
		ret = scsi_device_get(sdp);
		if (!ret && !scsi_device_online(sdp)) {
			scsi_device_put(sdp);
			ret = -ENODEV;
		}
	} else {
		ret = -ENODEV;
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	if (ret)
		goto out_err;
	if (hba->caps & UFSHCD_CAP_SSU_BY_SELF)
		ret = ufshcd_send_scsi_request_sense(hba, sdp, 1500, true);
	else
		ret = ufshcd_send_request_sense(hba, sdp);
	scsi_device_put(sdp);
out_err:
	if (ret)
		dev_err(hba->dev, "%s: UAC clear LU=%x ret = %d\n",
				__func__, wlun, ret);
	return ret;
}

int ufshcd_clear_ua_wluns(struct ufs_hba *hba)
{
	int ret = 0;

	if (!hba->wlun_dev_clr_ua)
		goto out;

	ret = ufshcd_clear_ua_wlun(hba, UFS_UPIU_UFS_DEVICE_WLUN);
	if (!ret)
		ret = ufshcd_clear_ua_wlun(hba, UFS_UPIU_RPMB_WLUN);
	if (!ret)
		hba->wlun_dev_clr_ua = false;
out:
	if (ret)
		dev_err(hba->dev, "%s: Failed to clear UAC WLUNS ret = %d\n",
				__func__, ret);
	return ret;
}

int ufshcd_send_request_sense(struct ufs_hba *hba, struct scsi_device *sdp)
{
	unsigned char cmd[6] = {REQUEST_SENSE,
				0,
				0,
				0,
				UFS_SENSE_SIZE,
				0};
	char *buffer;
	int ret;

	buffer = kzalloc(UFS_SENSE_SIZE, GFP_KERNEL);
	if (!buffer) {
		ret = -ENOMEM;
		goto out;
	}

	ret = scsi_execute(sdp, cmd, DMA_FROM_DEVICE, buffer,
			UFS_SENSE_SIZE, NULL, NULL,
			msecs_to_jiffies(1000), 3, 0, RQF_PM, NULL);
	if (ret)
		dev_err(hba->dev, "%s: failed with err %d\n", __func__, ret);

	kfree(buffer);
out:
	return ret;
}

bool ufshcd_compl_tm(struct request *req, void *priv, bool reserved)
{
	struct ctm_info *const ci = priv;
	struct completion *c = NULL;

	WARN_ON_ONCE(reserved);
	if (test_bit(req->tag, &ci->pending))
		return true;
	ci->ncpl++;
	c = req->end_io_data;
	if (c)
		complete(c);
	return true;
}

unsigned int ufshcd_get_tag_from_cmd(struct Scsi_Host *host, struct ufs_hba *hba, struct scsi_cmnd *cmd)
{
	unsigned int tag;

#ifdef CONFIG_HUFS_HC_CORE_UTR
	tag = (unsigned int)cmd->tag;
#else
#ifdef CONFIG_MAS_BLK
	if ((host->queue_quirk_flag & SHOST_QUIRK(SHOST_QUIRK_DRIVER_TAG_ALLOC)))
		tag = (unsigned int)cmd->tag;
	else
#endif
		tag = (unsigned int)cmd->request->tag;
#endif /* CONFIG_HUFS_HC_CORE_UTR */

	if (!ufshcd_valid_tag(hba, tag)) {
		dev_err(hba->dev,
			"%s: invalid command tag %d: cmd=0x%pK, cmd->request=0x%pK",
			__func__, tag, cmd, cmd->request);
		BUG();  /*lint !e146*/
	}

	return tag;
}

void ufshcd_update_dev_cmd_lun(struct ufs_hba *hba,
				      struct ufshcd_lrb *lrbp)
{
	struct ufs_query_req *request = &hba->dev_cmd.query.request;

	if (request->upiu_req.opcode == UPIU_QUERY_OPCODE_VENDOR_READ &&
	    request->upiu_req.idn == QUERY_TZ_IDN_BLK_INFO)
		lrbp->lun = request->lun;
}

/**
 * ufshcd_compose_scsi_cmd - filling scsi_cmnd with already info
 * @cmd: command from SCSI Midlayer
 * @done: call back function
 *
 * Until now, we do not filling all the infomation for scsi_cmnd, we just use
 * this for direct send cmd to ufs driver, skip block layer and scsi layer
 *
 * Returns 0 for success, non-zero in case of failure
 */
void ufshcd_compose_scsi_cmd(struct scsi_cmnd *cmd,
				    struct scsi_device *device,
				    unsigned char *cdb,
				    unsigned char *sense_buffer,
				    enum dma_data_direction sc_data_direction,
				    struct scatterlist *sglist,
				    unsigned int nseg,
				    unsigned int sg_len)
{
	cmd->device = device;
	cmd->cmnd = cdb;
	cmd->cmd_len = COMMAND_SIZE(cdb[0]);
#ifdef CONFIG_HUAWEI_UFS_VENDOR_MODE
	if(((cdb[0] >> 5) & 7) >= UFS_IOCTL_VENDOR_CMD)
		cmd->cmd_len = UFS_IOCTL_VENDOR_CDB_LEN;
#endif
	cmd->sense_buffer = sense_buffer;
	cmd->sc_data_direction = sc_data_direction;

	cmd->sdb.table.sgl = sglist;
	cmd->sdb.table.nents = nseg;
	cmd->sdb.length = sg_len;
}

/**
 * ufshcd_compose_upiu - form UFS Protocol Information Unit(UPIU)
 * @hba - per adapter instance
 * @lrb - pointer to local reference block
 */
int ufshcd_compose_upiu(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	u8 upiu_flags = 0;
	int ret = 0;

	switch (lrbp->command_type) {
	case UTP_CMD_TYPE_SCSI:
		if (likely(lrbp->cmd)) {
			ufshcd_prepare_req_desc_hdr(lrbp, &upiu_flags,
					    lrbp->cmd->sc_data_direction);
			ufshcd_prepare_utp_scsi_cmd_upiu(lrbp, upiu_flags);
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
			ufshcd_prepare_req_desc_uie(hba, lrbp);
#endif
			if (hba->ufs_hpb &&
		    		(hba->ufs_hpb->ufshpb_state == UFSHPB_PRESENT)) {
				if (hba->manufacturer_id == UFS_VENDOR_SAMSUNG)
					ufshpb_prep_for_dev_mode(hba, lrbp);
				else
					ufshpb_prep_fn_condense(hba, lrbp);
			}
		} else {
			ret = -EINVAL;
		}
		break;
	case UTP_CMD_TYPE_DEV_MANAGE:
		ufshcd_prepare_req_desc_hdr(lrbp, &upiu_flags, DMA_NONE);
		if (hba->dev_cmd.type == DEV_CMD_TYPE_QUERY)
			ufshcd_prepare_utp_query_req_upiu(
					hba, lrbp, upiu_flags);
		else if (hba->dev_cmd.type == DEV_CMD_TYPE_NOP)
			ufshcd_prepare_utp_nop_upiu(lrbp);
		else
			ret = -EINVAL;
		break;
	case UTP_CMD_TYPE_UFS:
		/* For UFS native command implementation */
		ret = -ENOTSUPP;
		dev_err(hba->dev, "%s: UFS native command are not supported\n",
			__func__);
		break;
	default:
		ret = -ENOTSUPP;
		dev_err(hba->dev, "%s: unknown command type: 0x%x\n",
				__func__, lrbp->command_type);
		break;
	} /* end of switch */

	return ret;
}
EXPORT_SYMBOL(ufshcd_compose_upiu);

/**
 * ufshcd_queuecommand_directly - API for sending scsi cmd directly, of course
 * skip error handler of scsi
 * @hba - UFS hba
 * @cmd - scsi_cmnd
 * @timeout - time in jiffies
 *
 * NOTE: We use device management tag and mutext lock, without this, we must
 * define a new wait, and complete it in scsi_done
 * Since there is only one available tag for device management commands,
 * it is expected you hold the hba->dev_cmd.lock mutex.
 * This function may sleep.
 */
static int __ufshcd_queuecommand_directly(struct ufs_hba *hba,
					  struct scsi_cmnd *cmd,
					  unsigned int timeout)
{
	int err;
	int tag;
	unsigned long flags;
	unsigned long time_left;
	struct ufshcd_lrb *lrbp = NULL;
	struct completion wait;

	init_completion(&wait);
	mutex_lock(&hba->dev_cmd.lock);
	hba->dev_cmd.complete = &wait;

	err = pm_runtime_get_sync(hba->dev);
	if (err < 0) {
		spin_lock_irqsave(hba->host->host_lock, flags);
		hba->dev_cmd.complete = NULL;
		spin_unlock_irqrestore(hba->host->host_lock,
				       flags);
		goto unlock;
	}
	wait_event(hba->dev_cmd.tag_wq,
		   ufshcd_get_dev_cmd_tag(hba, &tag));
	lrbp = &hba->lrb[tag];
	WARN_ON(lrbp->cmd); /*lint !e146 !e665*/
	lrbp->cmd = cmd;
	lrbp->sense_buffer = cmd->sense_buffer;
	lrbp->sense_bufflen = SCSI_SENSE_BUFFERSIZE;
	lrbp->saved_sense_len = 0;
	lrbp->task_tag = tag;
	lrbp->req_send = false;
	lrbp->lun = ufshcd_scsi_to_upiu_lun((unsigned int)cmd->device->lun);
	lrbp->intr_cmd = !ufshcd_is_intr_aggr_allowed(hba) ? true : false;
	lrbp->command_type = UTP_CMD_TYPE_SCSI;

	/* form UPIU before issuing the command */
	err = ufshcd_compose_upiu(hba, lrbp);
	if (err)
		goto out;

	/* Black Magic, dont touch unless you want a BUG */
	lrbp->command_type = UTP_CMD_TYPE_DEV_MANAGE;
	err = ufshcd_map_sg(hba, lrbp);
	if (err)
		goto out;

	/* Make sure descriptors are ready before ringing the doorbell */
	wmb();
	spin_lock_irqsave(hba->host->host_lock, flags);
	/* issue command to the controller */
	ufshcd_send_command(hba, (unsigned int)tag);
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	time_left = wait_for_completion_timeout(hba->dev_cmd.complete,
						msecs_to_jiffies(timeout));

	spin_lock_irqsave(hba->host->host_lock, flags);
	hba->dev_cmd.complete = NULL;
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	if (likely(time_left)) {
		err = ufshcd_transfer_rsp_status(hba, lrbp, false);
		if (err)
			dev_err(hba->dev, "SCSI command result err = 0x%x\n", err);
	} else {
		err = -ETIMEDOUT;
		dev_err(hba->dev, "%s: scsi request timedout, tag %d\n",
			__func__, lrbp->task_tag);
		if (!ufshcd_clear_cmd(hba, lrbp->task_tag)) {
			err = -EAGAIN;
#ifdef CONFIG_DFA_MCQ
			if (is_dfa_enabled(hba)) {
				dfa_inflt_req_clear(hba, lrbp->task_tag);
				goto out;
			}
#endif
			clear_bit(lrbp->task_tag, &hba->outstanding_reqs);
		} else {
			hba->ufs_dm_timeout++;
			spin_lock_irqsave(hba->host->host_lock, flags);
			lrbp->command_type = UTP_CMD_TYPE_DM_TIMEOUT;
			spin_unlock_irqrestore(hba->host->host_lock, flags);
		}
	}

out:
	lrbp->cmd = NULL;
	if (likely(err != -ETIMEDOUT))
		ufshcd_put_dev_cmd_tag(hba, tag);
	wake_up(&hba->dev_cmd.tag_wq);
unlock:
	pm_runtime_put_sync(hba->dev);
	mutex_unlock(&hba->dev_cmd.lock);
	return err;
}

static int ufshcd_queuecommand_directly(struct ufs_hba *hba,
					struct scsi_cmnd *cmd,
					unsigned int timeout, bool eh_handle)
{
	int err;
	int retry = 1;
	unsigned long flags;
	bool needs_flush = false;

start:
	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->force_reset || hba->ufshcd_state == UFSHCD_STATE_RESET ||
		hba->ufshcd_state == UFSHCD_STATE_EH_SCHEDULED) {
			needs_flush = true;
			dev_err(hba->dev, "%s: ufshcd_state = 0x%x \r\n", __func__, hba->ufshcd_state);
		}

	spin_unlock_irqrestore(hba->host->host_lock, flags);

	if (needs_flush) {
		if (hba->ufs_in_err_handle)
			return -1;
#ifdef CONFIG_SCSI_UFS_FAULT_INJECT
		if (hba->pm_op_in_progress)
			return -1;
#endif
		kthread_flush_work(&hba->eh_work);
	}

	/* Assume flush work makes ufshcd works well, or return error */
	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->ufshcd_state != UFSHCD_STATE_OPERATIONAL) {
		err = SCSI_MLQUEUE_HOST_BUSY;
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return err;
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	err = __ufshcd_queuecommand_directly(hba, cmd, timeout);
	if (err) {
		err = __ufshcd_queuecommand_directly(hba, cmd, timeout);
		dev_err(hba->dev, "%s: __ufshcd_queuecommand_directly retry! \r\n", __func__);
	}
	if (err && eh_handle) {
		spin_lock_irqsave(hba->host->host_lock, flags);
		hba->ufs_in_err_handle = TRUE;
		hba->force_reset = true;
		hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED;

		if (!kthread_queue_work(&hba->eh_worker, &hba->eh_work))
			dev_err(hba->dev, "%s: queue hba->eh_worker fail! \r\n", __func__);

		spin_unlock_irqrestore(hba->host->host_lock, flags);
		if (retry-- > 0)
			goto start;
	}

	return err;
}

#ifdef CONFIG_HUAWEI_UFS_VENDOR_MODE
int ufshcd_send_vendor_scsi_cmd(struct ufs_hba *hba,
		struct scsi_device *sdp, unsigned char* cdb, void* buf)
{
	int ret;
	unsigned char *dma_buf;
	struct scatterlist sglist;
	struct scsi_cmnd cmnd;

	dma_buf = kzalloc((size_t)PAGE_SIZE, GFP_ATOMIC);
	if (!dma_buf) {
		ret = -ENOMEM;
		goto out;
	}
	sg_init_one(&sglist, dma_buf, (unsigned int)PAGE_SIZE);

	memset(&cmnd, 0, sizeof(cmnd));

	ufshcd_compose_scsi_cmd(&cmnd, sdp, cdb, NULL, DMA_FROM_DEVICE,
				&sglist, 1, (unsigned int)PAGE_SIZE);

	ret = ufshcd_queuecommand_directly(hba, &cmnd, (unsigned int)1000, false);
	if(buf)
		memcpy(buf, dma_buf, PAGE_SIZE);
	if (ret)
		pr_err("%s: failed with err %d\n", __func__, ret);

	kfree(dma_buf);
out:
	return ret;
}
#endif

/*
 * Sync cache for already knowed fixed lun (0-4)
 * If failed, then failed, skip SCSI layer means skip exception handler
 */
int ufshcd_send_scsi_sync_cache(struct ufs_hba *hba,
				       struct scsi_device *sdp, enum ufs_dev_pwr_mode pwr_mode)
{
	int ret = 0;
	int j;
	unsigned int i;
	unsigned char *buffer = NULL;
	struct scsi_cmnd cmnd;
	struct scsi_device* psdev = NULL;

	unsigned char cmd[10] = {0};
	cmd[0] = SYNCHRONIZE_CACHE;

	if (!sdp)
		return -ENODEV;
	psdev = (struct scsi_device*)kzalloc(sizeof(struct scsi_device),GFP_KERNEL);
	if (!psdev){
		buffer = NULL;
		ret = -ENOMEM;
		goto out;
	}
	buffer = kzalloc((size_t)SCSI_SENSE_BUFFERSIZE, GFP_KERNEL);
	if (!buffer) {
		ret = -ENOMEM;
		goto out;
	}

	memset(&cmnd, 0, sizeof(cmnd));

	/* find scsi_host by device well know's host member */
	psdev->host = sdp->host;

	for (i = 3,j=0; j < 4; j++,i--) {
#ifdef CONFIG_SCSI_UFS_LUN_PROTECT
		/*
		 * LU0/LU1/LU2 are write-protected, and sync-cache cannot be
		 * executed.
		 */
		if ((i == 0) || (i == 1) || (i == 2))
			continue;
#endif
		psdev->lun = i;
		ufshcd_compose_scsi_cmd(&cmnd, psdev, cmd, buffer, DMA_NONE,
					NULL, 0, 0);

		ret = ufshcd_queuecommand_directly(hba, &cmnd, (unsigned int)1500, true);
#ifdef CONFIG_SCSI_UFS_SYNCCACHE_RECORD
		if (pwr_mode == UFS_POWERDOWN_PWR_MODE)
			ufshcd_record_sc(UFS_SC_POWERDOWN, ret);
#endif
		if (ret) {
			dev_err(hba->dev, "%s: failed for lun %llu, ret = %d\n", __func__,
			       psdev->lun, ret);
			goto out;
		}
	}

out:
	if(psdev)
		kfree(psdev);
	if(buffer)
		kfree(buffer);
	return ret;
}

int ufshcd_send_scsi_request_sense(struct ufs_hba *hba,
		struct scsi_device *sdp, unsigned int timeout, bool eh_handle)
{
	int ret;
	unsigned char *buffer = NULL;
	unsigned char *dma_buf = NULL;
	struct scatterlist sglist;
	struct scsi_cmnd cmnd;

	unsigned char cmd[6] = {REQUEST_SENSE,	 0, 0, 0,
				SCSI_SENSE_BUFFERSIZE, 0};

	buffer = kzalloc((size_t)SCSI_SENSE_BUFFERSIZE, GFP_KERNEL);
	if (!buffer) {
		ret = -ENOMEM;
		goto out;
	}

	dma_buf = kzalloc((size_t)PAGE_SIZE, GFP_KERNEL);
	if (!dma_buf) {
		ret = -ENOMEM;
		goto free_buffer;
	}

	sg_init_one(&sglist, dma_buf, (unsigned int)PAGE_SIZE);

	memset(&cmnd, 0, sizeof(cmnd));

	ufshcd_compose_scsi_cmd(&cmnd, sdp, cmd, buffer, DMA_FROM_DEVICE,
				&sglist, 1, (unsigned int)PAGE_SIZE);

	ret = ufshcd_queuecommand_directly(hba, &cmnd, timeout, eh_handle);
	if (ret)
		dev_err(hba->dev, "%s: failed with err %d\n", __func__, ret);

	kfree(dma_buf);
free_buffer:
	kfree(buffer);
out:
	return ret;
}

#if defined(CONFIG_MAS_ORDER_PRESERVE) || defined(CONFIG_MAS_UNISTORE_PRESERVE)
int ufshcd_send_request_sense_directly(struct scsi_device *sdp,
				unsigned int timeout, bool eh_handle)
{
	int ret;
	struct ufs_hba *hba;
	struct Scsi_Host *shost;

	shost = sdp->host;
	hba = shost_priv(shost);

	ret = ufshcd_send_scsi_request_sense(hba, sdp, timeout, eh_handle);
	if (ret)
		dev_err(hba->dev, "%s: failed with err %d\n", __func__, ret);

	return ret;
}
#endif

int ufshcd_send_scsi_ssu(struct ufs_hba *hba,
				struct scsi_device *sdp,
				unsigned char *cmd,
				unsigned int timeout,
				struct scsi_sense_hdr *sshdr)
{
	int ret;
	struct scsi_cmnd cmnd;
	unsigned char *buffer;

	buffer = kzalloc((size_t)SCSI_SENSE_BUFFERSIZE, GFP_KERNEL);
	if (!buffer) {
		ret = -ENOMEM;
		goto out;
	}

	memset(&cmnd, 0, sizeof(cmnd));

	ufshcd_compose_scsi_cmd(&cmnd, sdp, cmd, buffer, DMA_NONE, NULL, 0, 0);

	ret = ufshcd_queuecommand_directly(hba, &cmnd, timeout, true);
	if (ret)
		dev_err(hba->dev, "%s: failed with err %d\n", __func__, ret);

	kfree(buffer);
out:
	return ret;
}

/*
 * ufshcd_rsp_sense_data - check rsp sense_data
 * @hba: per adapter instance
 * @lrb: pointer to local reference block of completed command
 * @scsi_status: the result based on SCSI status response
 */
void ufshcd_rsp_sense_data(struct ufs_hba *hba, struct ufshcd_lrb *lrbp, int scsi_status)
{
	if ((lrbp->ucd_rsp_ptr->sr.sense_data[2] & 0xf) == HARDWARE_ERROR) {
		dsm_ufs_update_scsi_info(lrbp, scsi_status, DSM_UFS_HARDWARE_ERR);
		schedule_ufs_dsm_work(hba);
		ufs_rdr_hardware_err(hba, lrbp);
	} else if ((lrbp->ucd_rsp_ptr->sr.sense_data[2] & 0xf) == MEDIUM_ERROR) {
		dsm_ufs_update_scsi_info(lrbp, scsi_status, DSM_UFS_MEDIUM_ERR);
		schedule_ufs_dsm_work(hba);
		ufs_rdr_hardware_err(hba, lrbp);
	}
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	if (UFS_VENDOR_HI1861 == hba->manufacturer_id) {
		if ((lrbp->ucd_rsp_ptr->sr.sense_data[2] & 0xf) == MEDIUM_ERROR &&
			(lrbp->ucd_rsp_ptr->sr.sense_data[12] & 0xff) == 0x03 &&
			(lrbp->ucd_rsp_ptr->sr.sense_data[13] & 0xff) == 0) {
			dev_err(hba->dev,"1861 write fault\n");
			BUG();
		}
		if ((lrbp->ucd_rsp_ptr->sr.sense_data[2] & 0xf) == HI1861_INTERNEL &&
			(lrbp->ucd_rsp_ptr->sr.sense_data[12] & 0xff) == 0x80 &&
			(lrbp->ucd_rsp_ptr->sr.sense_data[13] & 0xff) == 0) {
			dsm_ufs_update_scsi_info(lrbp, scsi_status, DSM_UFS_HI1861_INTERNEL_ERR);
			schedule_ufs_dsm_work(hba);
			schedule_work(&hba->fsr_work);
		}
	}
#endif
}
