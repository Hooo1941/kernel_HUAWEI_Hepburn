/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: gear ctrl implement
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

#include <linux/blkdev.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_device.h>
#include <platform_include/basicplatform/linux/ufs/scsi_gear_ctrl.h>

static int scsi_read_lrb_use_rate(struct request_queue *q, unsigned long *lrb_use_rate)
{
	struct scsi_device *sdev;

	if (unlikely(!q))
		return -EINVAL;

	sdev = q->queuedata;
	if (unlikely(!sdev || (sdev->type != TYPE_DISK && sdev->type != TYPE_WLUN)))
		return -EINVAL;
	if (unlikely(sdev->sdev_state != SDEV_RUNNING))
		return -EINVAL;
	if (unlikely(!sdev->host || !sdev->host->hostt || !sdev->host->hostt->gear_ctrl_ops ||
		     !sdev->host->hostt->gear_ctrl_ops->read_lrb_use_rate))
		return -EINVAL;

	return sdev->host->hostt->gear_ctrl_ops->read_lrb_use_rate(sdev, lrb_use_rate);
}

static int scsi_gear_ctrl(struct request_queue *q, u32 set_gear)
{
	struct scsi_device *sdev;

	if (unlikely(!q))
		return -EINVAL;

	sdev = q->queuedata;
	if (unlikely(!sdev || (sdev->type != TYPE_DISK && sdev->type != TYPE_WLUN)))
		return -EINVAL;
	if (unlikely(sdev->sdev_state != SDEV_RUNNING))
		return -EINVAL;
	if (unlikely(!sdev->host || !sdev->host->hostt || !sdev->host->hostt->gear_ctrl_ops ||
				!sdev->host->hostt->gear_ctrl_ops->gear_ctrl))
		return -EINVAL;

	return sdev->host->hostt->gear_ctrl_ops->gear_ctrl(sdev, set_gear);
}

static int scsi_gear_level_get(struct request_queue *q, u32 *get_gear)
{
	struct scsi_device *sdev;

	if (unlikely(!q))
		return -EINVAL;

	sdev = q->queuedata;
	if (unlikely(!sdev || (sdev->type != TYPE_DISK && sdev->type != TYPE_WLUN)))
		return -EINVAL;
	if (unlikely(sdev->sdev_state != SDEV_RUNNING))
		return -EINVAL;
	if (unlikely(!sdev->host || !sdev->host->hostt || !sdev->host->hostt->gear_ctrl_ops ||
				!sdev->host->hostt->gear_ctrl_ops->gear_level_get))
		return -EINVAL;

	return sdev->host->hostt->gear_ctrl_ops->gear_level_get(sdev, get_gear);
}

static int scsi_gear_level_cap_get(struct request_queue *q, u32 *min_gear, u32 *max_gear)
{
	struct scsi_device *sdev;

	if (unlikely(!q))
		return -EINVAL;

	sdev = q->queuedata;
	if (unlikely(!sdev || (sdev->type != TYPE_DISK && sdev->type != TYPE_WLUN)))
		return -EINVAL;
	if (unlikely(sdev->sdev_state != SDEV_RUNNING))
		return -EINVAL;
	if (unlikely(!sdev->host || !sdev->host->hostt || !sdev->host->hostt->gear_ctrl_ops ||
				!sdev->host->hostt->gear_ctrl_ops->gear_level_cap_get))
		return -EINVAL;

	return sdev->host->hostt->gear_ctrl_ops->gear_level_cap_get(sdev, min_gear, max_gear);
}

static int scsi_switch_latency_check(struct request_queue *q, enum ufs_dss_latency_enum scenario, int set_gear,
				     int switch_reason)
{
	struct scsi_device *sdev;

	if (unlikely(!q))
		return -EINVAL;

	sdev = q->queuedata;
	if (unlikely(!sdev || (sdev->type != TYPE_DISK && sdev->type != TYPE_WLUN)))
		return -EINVAL;
	if (unlikely(sdev->sdev_state != SDEV_RUNNING))
		return -EINVAL;
	if (unlikely(!sdev->host || !sdev->host->hostt || !sdev->host->hostt->gear_ctrl_ops ||
				!sdev->host->hostt->gear_ctrl_ops->switch_latency_check))
		return -EINVAL;

	return sdev->host->hostt->gear_ctrl_ops->switch_latency_check(sdev, scenario, set_gear, switch_reason);
}

struct lld_gear_ctrl_ops scsi_gear_ctrl_ops = {
	.read_gear_use_rate = scsi_read_lrb_use_rate,
	.gear_ctrl = scsi_gear_ctrl,
	.gear_level_get = scsi_gear_level_get,
	.gear_level_cap_get = scsi_gear_level_cap_get,
	.switch_latency_check = scsi_switch_latency_check,
};

void scsi_dev_gear_ctrl_register(struct Scsi_Host *shost)
{
	static bool flag = false;

	if (flag)
		return;

	if (!shost || !shost->hostt)
		return;

	mas_blk_mq_tagset_gear_ctrl_register(&shost->tag_set, shost->hostt->gear_ctrl_ops ? &scsi_gear_ctrl_ops : NULL);
	flag = true;
}

void hck_scsi_dev_gear_ctrl_register(struct Scsi_Host *shost)
{
	return scsi_dev_gear_ctrl_register(shost);
}

void track_mas_gear_ctrl_init(void *data, struct request_queue *q)
{
	return mas_gear_ctrl_init(q);
}

