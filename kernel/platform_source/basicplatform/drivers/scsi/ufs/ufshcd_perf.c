/*
 * ufshcd_perf.c
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

#include "ufshcd_perf.h"
#include "ufshcd.h"
#include "ufshcd_extend.h"
#include "ufshcd_protocol.h"

static void ufshcd_wb_toggle_fn(struct work_struct *work)
{
	int ret;
	u32 current_buffer;
	u32 available;
	unsigned long flags;
	struct ufs_hba *hba;

	hba = container_of(work, struct ufs_hba, wb_work);

	dev_info(hba->dev, "%s: write booster toggle issued", __func__);

	pm_runtime_get_sync(hba->dev);

	if (ufshcd_wb_ctrl(hba, false) || ufshcd_wb_ctrl(hba, true))
		/*
		 * Wait error handler to complete, else caller like suspend
		 * would entering suspend while a error handler executing in
		 * another worker. After eh, wb would be reenabled.
		 */
		ufshcd_host_force_reset_sync(hba);

	/*
	 * Error handler doesn't need here. If we failed to check buffer
	 * size, following io would cause a new exception handler which
	 * let us try here again.
	 */
	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_READ_ATTR,
				      QUERY_ATTR_IDN_CURR_WB_BUFF_SIZE, 0,
				      0, &current_buffer);
	if (ret)
		goto out;

	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_READ_ATTR,
				      QUERY_ATTR_IDN_AVAIL_WB_BUFF_SIZE, 0,
				      0, &available);
	if (ret)
		goto out;
	/*
	 * Maybe, there are not enough space to alloc a new write buffer, if so,
	 * disable exception to avoid en endless notification.
	 */
	if (current_buffer < hba->wb_shared_alloc_units ||
	    available <= WB_T_BUFFER_THRESHOLD) {
		ret = ufshcd_disable_ee(hba, MASK_EE_WRITEBOOSTER_EVENT);
		if (!ret) {
			spin_lock_irqsave(hba->host->host_lock, flags);
			hba->wb_exception_enabled = false;
			spin_unlock_irqrestore(hba->host->host_lock, flags);
		}
	}

out:
	pm_runtime_put_sync(hba->dev);

	spin_lock_irqsave(hba->host->host_lock, flags);
	hba->wb_work_sched = false;
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

static int ufshcd_wb_ctrl_exception(struct ufs_hba *hba, bool enable)
{
	int ret;
	unsigned long flags;

	/*
	 * Some device can alloc a new write buffer by toggle write booster.
	 * We apply a exception handler func for and only for those devices.
	 *
	 * If new strategy was to implement, the hba->wb_work would be the good
	 * place to put new function to, ufshcd_exception_event_handler would
	 * handle them the same way.
	 */
	if (hba->manufacturer_id != UFS_VENDOR_TOSHIBA)
		return 0;

	if (enable)
		ret = ufshcd_enable_ee(hba, MASK_EE_WRITEBOOSTER_EVENT);
	else
		ret = ufshcd_disable_ee(hba, MASK_EE_WRITEBOOSTER_EVENT);

	if (ret) {
		dev_err(hba->dev,
			"%s: failed to enable wb exception event %d\n",
			__func__, ret);
	} else {
		spin_lock_irqsave(hba->host->host_lock, flags);
		hba->wb_exception_enabled = enable;
		spin_unlock_irqrestore(hba->host->host_lock, flags);

		dev_info(hba->dev, "%s: wb exception event %s\n",
			 __func__, enable ? "enable" : "disable");
	}

	return ret;
}

static inline int ufshcd_wb_enable_exception(struct ufs_hba *hba)
{
	return ufshcd_wb_ctrl_exception(hba, true);
}

static inline int ufshcd_wb_disable_exception(struct ufs_hba *hba)
{
	return ufshcd_wb_ctrl_exception(hba, false);
}

bool ufshcd_wb_allowed(struct ufs_hba *hba)
{
	return (hba->ufs_device_spec_version >= UFS_DEVICE_SPEC_3_1 ||
		       hba->ufs_device_spec_version == UFS_DEVICE_SPEC_2_2) &&
	       (hba->caps & UFSHCD_CAP_WRITE_BOOSTER) &&
	       (hba->d_ext_ufs_feature_sup & UFS_DEV_WRITE_BOOSTER_SUP) &&
	       hba->wb_shared_alloc_units;
}

void ufshcd_wb_idle_handler(struct ufs_hba *hba, ktime_t now_time)
{
	static ktime_t last_time;

	if (!ufshcd_wb_allowed(hba))
		return ;

	if (ktime_ms_delta(now_time, last_time) > WB_FLUSH_WORKER_INTEVAL) {
		queue_delayed_work(system_freezable_wq, &hba->wb_flush_work,
				msecs_to_jiffies(WB_FLUSH_WORKER_INTEVAL));
		last_time = now_time;
	}
}

static bool ufshcd_wb_map_selector_desc(enum query_opcode opcode, u8 idn)
{
	if (opcode == UPIU_QUERY_OPCODE_READ_DESC ||
	    opcode == UPIU_QUERY_OPCODE_WRITE_DESC)
		if (idn == QUERY_DESC_IDN_DEVICE ||
		    idn == QUERY_DESC_IDN_CONFIGURATION ||
		    idn == QUERY_DESC_IDN_GEOMETRY)
		    	return true;
	return false;
}

static bool ufshcd_wb_map_selector_flag(enum query_opcode opcode, u8 idn)
{
	if (opcode == UPIU_QUERY_OPCODE_SET_FLAG ||
	    opcode == UPIU_QUERY_OPCODE_CLEAR_FLAG ||
	    opcode == UPIU_QUERY_OPCODE_TOGGLE_FLAG ||
	    opcode == UPIU_QUERY_OPCODE_READ_FLAG)
		if (idn == QUERY_FLAG_IDN_WB_EN ||
		    idn == QUERY_FLAG_IDN_WB_BUFF_FLUSH_EN ||
		    idn == QUERY_FLAG_IDN_WB_BUFF_FLUSH_DURING_HIBERN8)
		    	return true;
	return false;
}

static bool ufshcd_wb_map_selector_attr(enum query_opcode opcode, u8 idn)
{
	if (opcode == UPIU_QUERY_OPCODE_READ_ATTR ||
	    opcode == UPIU_QUERY_OPCODE_WRITE_ATTR)
		if (idn == QUERY_ATTR_IDN_EE_CONTROL ||
		    idn == QUERY_ATTR_IDN_EE_STATUS ||
		    idn == QUERY_ATTR_IDN_WB_FLUSH_STATUS ||
		    idn == QUERY_ATTR_IDN_AVAIL_WB_BUFF_SIZE ||
		    idn == QUERY_ATTR_IDN_WB_BUFF_LIFE_TIME_EST ||
		    idn == QUERY_ATTR_IDN_CURR_WB_BUFF_SIZE)
		    	return true;
	return false;
}

u8 ufshcd_wb_map_selector(struct ufs_hba *hba, enum query_opcode opcode,
				 u8 idn, u8 index, u8 selector)
{
	if (!hba->force_selector)
		return selector;

	if (ufshcd_wb_map_selector_desc(opcode, idn) ||
	    ufshcd_wb_map_selector_flag(opcode, idn) ||
	    ufshcd_wb_map_selector_attr(opcode, idn))
		return 1;

	return selector;
}

static int ufshcd_wb_type_detect(struct ufs_hba *hba)
{
	if (hba->wb_type != WB_SINGLE_SHARED) {
		dev_err(hba->dev, "dedicated write booster mode not support\n");
		/* Dedicated mode not implemented */
		return -EINVAL;
	}

	dev_info(hba->dev, "single shared write booster found\n");
	return 0;
}

static int ufshcd_wb_auto_flush_ctrl(struct ufs_hba *hba, bool enable)
{
	int ret;
	enum query_opcode opcode;

	if (enable)
		opcode = UPIU_QUERY_OPCODE_SET_FLAG;
	else
		opcode = UPIU_QUERY_OPCODE_CLEAR_FLAG;

	ret = ufshcd_query_flag_retry(
		hba, opcode, QUERY_FLAG_IDN_WB_BUFF_FLUSH_DURING_HIBERN8, 0, NULL);
	if (ret) {
		dev_err(hba->dev, "%s %s failed %d\n", __func__,
			enable ? "enable" : "disable", ret);
		return ret;
	}

	hba->flush_is_open = enable;
	dev_dbg(hba->dev, "%s auto flush %s %d\n", __func__,
		enable ? "enable" : "disable", ret);
	return ret;
}

static bool ufshcd_wb_permanent_disable(struct ufs_hba *hba)
{
	unsigned long flags;
	bool wb_permanent_disabled = false;

	spin_lock_irqsave(hba->host->host_lock, flags);
	wb_permanent_disabled = hba->wb_permanent_disabled;
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	return wb_permanent_disabled;
}

static void ufshcd_wb_flush_fn(struct work_struct *work)
{
	int ret;
	u32 available;
	struct ufs_hba *hba;

	hba = container_of(work, struct ufs_hba, wb_flush_work.work);

	if (!ufshcd_wb_allowed(hba))
		return;

	if (ufshcd_wb_permanent_disable(hba))
		return;

	if (!hba->wb_enabled)
		return;

	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_READ_ATTR,
				      QUERY_ATTR_IDN_AVAIL_WB_BUFF_SIZE, 0,
				      0, &available);
	if (ret) {
		dev_info(hba->dev, "%s, read wb available buffer size failed!\n", __func__);
		return;
	}

	if (available <= SAMSUNG_FLUSH_LOWER_LIMIT && !hba->flush_is_open)
		ufshcd_wb_auto_flush_ctrl(hba, true);
	if (available >= SAMSUNG_FLUSH_UPPER_LIMIT && hba->flush_is_open)
		ufshcd_wb_auto_flush_ctrl(hba, false);
}

static bool is_samsung_v5_workaround(struct ufs_hba *hba)
{
	char *product_name = hba->model;
	struct list_head *node = NULL;
	struct samsung_pnm *samsung_v5 = NULL;

	list_for_each(node, &hba->wb_flush_v5_whitelist) {
		samsung_v5 = list_entry(node, struct samsung_pnm, p);
		if (memcmp(product_name, samsung_v5->ufs_pnm, SAMSUNG_PNM_LEN) == 0)
			return true;
	}
	return false;
}

static int ufshcd_wb_init(struct ufs_hba *hba)
{
	int ret;
	unsigned long flags;

	/*
	 * Put init once code here, as we need send query command to get device
	 * info which was not able to do in ufshcd_init. To avoid needless
	 * reinit of that, eh in progress was used.
	 */
	spin_lock_irqsave(hba->host->host_lock, flags);
	if (ufshcd_eh_in_progress(hba) || hba->pm_op_in_progress) {
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	INIT_WORK(&hba->wb_work, ufshcd_wb_toggle_fn);
	if (is_samsung_v5_workaround(hba)) {
		hba->is_samsung_v5 = true;
		INIT_DELAYED_WORK(&hba->wb_flush_work, ufshcd_wb_flush_fn);
	} else {
		hba->is_samsung_v5 = false;
	}

	ret = ufshcd_vendor_feature_options_apply(hba);
	if (ret)
		return ret;

	return ufshcd_wb_type_detect(hba);
}

void hufs_wb_config(struct ufs_hba *hba)
{
	int ret;

	if (!ufshcd_wb_allowed(hba))
		return;

	if (ufshcd_wb_permanent_disable(hba))
		return;

	ret = ufshcd_wb_init(hba);
	if (ret)
		return;

	ret = ufshcd_wb_ctrl(hba, true);
	if (ret)
		return;

	if (!hba->is_samsung_v5) {
		ret = ufshcd_wb_auto_flush_ctrl(hba, true);
		if (ret)
			return;
	}

	ret = ufshcd_wb_enable_exception(hba);
	return;
}

void ufshcd_set_wb_permanent_disable(struct ufs_hba *hba)
{
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	hba->wb_permanent_disabled = true;
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

static inline struct ufs_hba *ufshcd_dev_to_hba(struct device *dev)
{
	if (!dev)
		return NULL;

	return shost_priv(class_to_shost(dev));
}

ssize_t wb_permanent_disable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	bool flag = false;
	struct ufs_hba *hba = ufshcd_dev_to_hba(dev);

	if (!hba)
		return snprintf(buf, PAGE_SIZE,
			"wb permanent disable hba is null\n");

	flag = ufshcd_wb_permanent_disable(hba);

	return snprintf(buf, PAGE_SIZE, "wb permanent disable : %s\n",
		flag ? "true" : "false");
}

bool ufshcd_get_wb_work_sched(struct ufs_hba *hba)
{
	unsigned long flags;
	bool wb_work_sched = false;

	spin_lock_irqsave(hba->host->host_lock, flags);
	wb_work_sched = hba->wb_work_sched;
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	return wb_work_sched;
}

void ufshcd_wb_exception_event_handler(struct ufs_hba *hba)
{
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);

	if (!hba->wb_exception_enabled)
		goto out;

	if (hba->wb_work_sched)
		goto out;
	hba->wb_work_sched = true;

	schedule_work(&hba->wb_work);

out:
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

ssize_t wb_permanent_disable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t len)
{
	int ret = 0;
	struct ufs_hba *hba = ufshcd_dev_to_hba(dev);

	if (!hba)
		return -EFAULT;

	pm_runtime_get_sync(hba->dev);
	ufshcd_set_wb_permanent_disable(hba);
	if (!ufshcd_wb_allowed(hba)) {
		dev_info(hba->dev, "%s wb not allowed\n", __func__);

		goto out;
	}

	ret = ufshcd_wb_disable_exception(hba);
	if (ret) {
		dev_err(hba->dev, "%s wb disable exception fail\n", __func__);

		goto out;
	}

	/* when WRITEBOOSTER_EVENT is procing, caller will retry. */
	if (ufshcd_get_wb_work_sched(hba)) {
		ret = -EAGAIN;
		goto out;
	}

	ret = ufshcd_wb_ctrl(hba, false);
	if (ret) {
		dev_err(hba->dev, "%s wb disable fail\n", __func__);

		goto out;
	}
	dev_info(hba->dev, "%s wb permanent disable succ\n", __func__);
out:
	pm_runtime_put_sync(hba->dev);

	return !ret ? len : ret;
}

static int compose_lrbp(struct ufs_hba *hba, struct scsi_cmnd *cmd, int tag, struct ufshcd_lrb *lrbp)
{
	int err;

	lrbp->cmd = cmd;
	lrbp->sense_bufflen = SCSI_SENSE_BUFFERSIZE;
	lrbp->sense_buffer = cmd->sense_buffer;
	lrbp->saved_sense_len = 0;
	lrbp->task_tag = tag;
	lrbp->lun = ufshcd_scsi_to_upiu_lun((unsigned int)cmd->device->lun);
	lrbp->intr_cmd = !ufshcd_is_intr_aggr_allowed(hba) ? true : false;
	lrbp->command_type = UTP_CMD_TYPE_SCSI;
	err = ufshcd_compose_upiu(hba, lrbp);
	if (err)
		return err;
	lrbp->command_type = UTP_CMD_TYPE_DEV_MANAGE;

	return 0;
}

int ufstt_do_scsi_cmd(
	struct ufs_hba *hba, struct scsi_cmnd *cmd, unsigned int timeout, ktime_t start_time)
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
		dev_err(hba->dev, "%s, failed to exit from hibernate, %d\n", __func__, err);
		spin_lock_irqsave(hba->host->host_lock, flags);
		hba->dev_cmd.complete = NULL;
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		goto unlock;
	}
	wait_event(hba->dev_cmd.tag_wq, ufshcd_get_dev_cmd_tag(hba, &tag));
	lrbp = &hba->lrb[tag];
	WARN_ON(lrbp->cmd);
	err = compose_lrbp(hba, cmd, tag, lrbp);
	if (err)
		goto out;
	err = ufshcd_map_sg(hba, lrbp);
	if (err)
		goto out;
	wmb();
	spin_lock_irqsave(hba->host->host_lock, flags);
	ufshcd_send_command(hba, (unsigned int)tag);
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	time_left = wait_for_completion_timeout(
		hba->dev_cmd.complete, msecs_to_jiffies(timeout));
	spin_lock_irqsave(hba->host->host_lock, flags);
	hba->dev_cmd.complete = NULL;
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	if (likely(time_left)) {
		err = ufshcd_transfer_rsp_status(hba, lrbp, false);
		if (err)
			hba->ufshpb_check_rsp_err++;
	}
	if (!time_left) {
		err = -ETIMEDOUT;
		hba->ufshpb_scsi_timeout++;
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
			spin_lock_irqsave(hba->host->host_lock, flags);
			lrbp->command_type = UTP_CMD_TYPE_DM_TIMEOUT;
			spin_unlock_irqrestore(hba->host->host_lock, flags);
			hba->ufs_dm_timeout++;
		}
	}
out:
	lrbp->cmd = NULL;
	if (err != -ETIMEDOUT)
		ufshcd_put_dev_cmd_tag(hba, tag);
	wake_up(&hba->dev_cmd.tag_wq);
unlock:
	pm_runtime_put_sync(hba->dev);
	mutex_unlock(&hba->dev_cmd.lock);
	return err;
}
