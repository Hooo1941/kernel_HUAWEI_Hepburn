/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Implement of igs driver
 * Create: 2019-03-26
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/ion/mm_ion.h>
#include <linux/iommu/mm_iommu.h>
#include <platform_include/camera/native/camera.h>
#include <media/v4l2-event.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <securec.h>
#include "inputhub_api/inputhub_api.h"
#include <securectype.h>
#include <platform_include/smart/linux/iomcu_status.h>
#include "shmem/shmem.h"
#include "igs_cam_dev_20.h"
#include "cam_intf.h"

#define MINIISP_IDLE 0
#define MINIISP_BUSY (-1)

static atomic_t g_ev_seq_id;
static struct swingcam_priv g_swing_priv = {0};
static struct miniisp_t g_miniisp = {
	.name = "miniisp",
};
static int miniisp_status = MINIISP_IDLE;

enum timestamp_state_t {
	TIMESTAMP_UNINITIAL = 0,
	TIMESTAMP_INITIAL,
};

static enum timestamp_state_t s_timestamp_state;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static struct timeval s_timeval;
#else
static struct timespec64 s_timeval;
#endif
static struct timespec64 s_time_spec;
static enum igs_timestamp_source  g_timestamp_source;

static void swing_cam_set_timestamp_source(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_init_timestamp *req = NULL;
	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return;
	}

	if (mini_req->api_name != COMMAND_MINIISP_INIT_TIMESTAMP) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return;
	}
	req = &(mini_req->u.req_init_timestamp);
	swing_cam_log_info("[%s] timestamp source : %d", __func__, req->source);
	g_timestamp_source = req->source;
}

static void swing_cam_init_timestamp(void)
{
	s_timestamp_state = TIMESTAMP_INITIAL;

	if (g_timestamp_source == IGS_TIMESTAMP_USE_BOOTTIME)
		return;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	get_monotonic_boottime64(&s_time_spec);
	do_gettimeofday(&s_timeval);
#else
	ktime_get_boottime_ts64(&s_time_spec);
	ktime_get_real_ts64(&s_timeval);
#endif
}

static void swing_cam_destroy_timestamp(void)
{
	s_timestamp_state = TIMESTAMP_UNINITIAL;
	(void)memset_s(&s_timeval, sizeof(s_timeval), 0x00, sizeof(s_timeval));
	(void)memset_s(&s_time_spec, sizeof(s_time_spec), 0x00,
		sizeof(s_time_spec));
}

static void swing_cam_set_timestamp(u32 *timestamp_high, u32 *timestamp_low)
{
	u64 fw_micro_second;
	u64 fw_sys_counter;
	u64 micro_second;

	if (s_timestamp_state == TIMESTAMP_UNINITIAL) {
		swing_cam_log_err("[%s] wouldn't enter this branch",
			__func__);
		swing_cam_init_timestamp();
	}

	if (!timestamp_high || !timestamp_low) {
		swing_cam_log_err("[%s] timestamp_high or timestamp_low is null",
			__func__);
		return;
	}

	if (*timestamp_high == 0 && *timestamp_low == 0)
		return;

	fw_sys_counter = ((u64)(*timestamp_high) << 32) | (u64)(*timestamp_low);
	if (g_timestamp_source == IGS_TIMESTAMP_USE_BOOTTIME) {
		fw_micro_second = fw_sys_counter / CAM_NANOSECOND_PER_MICROSECOND;
	} else {
		/* 32:offset */
		micro_second = (fw_sys_counter / CAM_NANOSECOND_PER_MICROSECOND) -
			(u64)(s_time_spec.tv_sec * CAM_MICROSECOND_PER_SECOND) -
			(unsigned long)(s_time_spec.tv_nsec / CAM_NANOSECOND_PER_MICROSECOND);

		fw_micro_second =
			(micro_second / CAM_MICROSECOND_PER_SECOND + (u64)s_timeval.tv_sec) *
			CAM_MICROSECOND_PER_SECOND +
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
			((micro_second % CAM_MICROSECOND_PER_SECOND) + (u64)s_timeval.tv_usec);
#else
			((micro_second % CAM_MICROSECOND_PER_SECOND) + (u64)s_timeval.tv_nsec /
			CAM_NANOSECOND_PER_MICROSECOND);
#endif
	}
	/* 32:offset */
	*timestamp_high = (u32)((fw_micro_second >> 32) & 0xFFFFFFFF);
	*timestamp_low = (u32)(fw_micro_second & 0xFFFFFFFF);
}

static int swing_cam_put_data_to_read_kfifo(unsigned char *buf,
	unsigned int len)
{
	int ret;
	size_t length = sizeof(struct swingcam_data);
	struct swingcam_data read_data = {0};

	mutex_lock(&g_swing_priv.swing_mutex);
	if (g_swing_priv.ref_cnt == 0) {
		swing_cam_log_info("[%s] ref cnt is 0", __func__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -1;
	}

	if (kfifo_avail(&g_swing_priv.read_kfifo) < length) {
		swing_cam_log_err("[%s] read_kfifo is full, drop upload data",
			__func__);
		goto ret_unlock;
	}

	read_data.recv_len = len;
	read_data.p_recv = kzalloc(read_data.recv_len, GFP_ATOMIC);
	if (!read_data.p_recv) {
		swing_cam_log_err("[%s] Failed to alloc memory to save resp", __func__);
		goto ret_unlock;
	}

	ret = memcpy_s(read_data.p_recv, read_data.recv_len,
		buf, read_data.recv_len);
	if (ret != 0) {
		swing_cam_log_err("[%s] memcpy_s failed", __func__);
		goto ret_unlock;
	}

	length = kfifo_in(&g_swing_priv.read_kfifo, (unsigned char *)&read_data,
		sizeof(struct swingcam_data));
	if (length == 0) {
		swing_cam_log_err("[%s] kfifo_in failed", __func__);
		goto ret_unlock;
	}
	mutex_unlock(&g_swing_priv.swing_mutex);

	if (!g_miniisp.subdev.devnode) {
		swing_cam_log_err("[%s] vdec is null", __func__);
		goto ret_err;
	}
	return 0;

ret_unlock:
	mutex_unlock(&g_swing_priv.swing_mutex);

ret_err:
	if (read_data.p_recv)
		kfree(read_data.p_recv);

	return -EFAULT;
}

static int swing_cam_enqueue(unsigned char *buf,
	unsigned int len, enum miniisp_event_kind cur_kind)
{
	int ret;
	struct v4l2_event ev;
	struct video_device *vdev = NULL;
	struct miniisp_event *req = NULL;
	struct miniisp_msg_t *msg = (struct miniisp_msg_t *)buf;

	ret = swing_cam_put_data_to_read_kfifo(buf, len);
	if (ret != 0)
		return ret;

	req = (struct miniisp_event *)ev.u.data;
	(void)memset_s(&ev, sizeof(struct v4l2_event), 0,
		sizeof(struct v4l2_event));

	vdev = g_miniisp.subdev.devnode;
	ev.type = MINIISP_V4L2_EVENT_TYPE;
	ev.id = MINIISP_HIGH_PRIO_EVENT;
	req->kind = cur_kind;
	req->ev_seq_id = (unsigned int)atomic_inc_return(&g_ev_seq_id);
	swing_cam_log_info("[%s] event seq id: %u, msg apiname: 0x%x, msgid: 0x%x",
			__func__, req->ev_seq_id, msg->api_name, msg->message_id);

	v4l2_event_queue(vdev, &ev);

	return 0;
}

static int swing_cam_check_miniisp_status(struct pkt_subcmd_resp *p_resp,
	struct miniisp_msg_t *p_msg_ack)
{
	struct miniisp_msg_t mini_resp = {0};

	(void)p_resp;

	if (p_msg_ack->api_name == COMMAND_MINIISP_EXTEND_SET && // extend message resp
		p_msg_ack->message_id == 0) {
		swing_cam_log_info("[%s] ignore this resp", __func__);
		return -EFAULT;
	}

	if ((p_msg_ack->api_name == MINI_EVENT_SENT) &&
		(p_msg_ack->u.event_sent.event_id == EVENT_AO_CAMERA_OPEN ||
		p_msg_ack->u.event_sent.event_id == EVENT_AO_CAMERA_CLOSE) &&
		(miniisp_status == MINIISP_BUSY)) {
		mini_resp.message_size = (unsigned int)sizeof(struct miniisp_msg_t);
		mini_resp.api_name = COMMAND_MINIISP_EXTEND_SET;
		mini_resp.message_id = 0;
		mini_resp.u.req_subcmd_swing_cam.camid = p_msg_ack->u.ack_extend_igs_set.camid;
		mini_resp.u.req_subcmd_swing_cam.extend_cmd = 9; // extend message
		mini_resp.u.req_subcmd_swing_cam.paras[0] = 0; // DEV_AO_CAM
		mini_resp.u.req_subcmd_swing_cam.paras[1] = 2; // AO_CAM_BUSY
		if (send_cmd_from_kernel(TAG_SWING_CAM, CMD_CMN_CONFIG_REQ,
			SUB_CMD_SWING_CAM_EXTEND,
			(char *)&mini_resp, sizeof(struct miniisp_msg_t)) != 0)
			swing_cam_log_err("[%s] send cmd error", __func__);
		swing_cam_log_info("[%s] send miniisp_busy success", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_cam_execute_subcmd_wakeup_lock(struct pkt_subcmd_resp *p_resp,
	struct miniisp_msg_t *p_msg_ack)
{
	(void)p_resp;

	swing_cam_log_info("[%s] enter, api_name %d", __func__, p_msg_ack->api_name);

	if ((p_msg_ack->api_name == MINI_EVENT_SENT) &&
		(p_msg_ack->u.event_sent.event_id == EVENT_AO_WAKEUP_AP_LOCK_GET)) {
		if (!g_swing_priv.igs_cam_wklock->active) {
			swing_cam_log_info("[%s]: wakeup lock get ********************\n", __func__);
			__pm_wakeup_event(g_swing_priv.igs_cam_wklock, AOCAMERA_IOCTL_WAIT_TMOUT * 2);
		}
		return -EFAULT;
	} else if ((p_msg_ack->api_name == MINI_EVENT_SENT) &&
			   (p_msg_ack->u.event_sent.event_id == EVENT_AO_WAKEUP_AP_LOCK_RELEASE)) {
		if (g_swing_priv.igs_cam_wklock->active) {
			swing_cam_log_info("[%s]: wakeup lock release ********************\n", __func__);
			__pm_relax(g_swing_priv.igs_cam_wklock);
		}
		return -EFAULT;
	} else {
		return 0;
	}
	return 0;
}

static int swing_cam_execute_subcmd(struct pkt_subcmd_resp *p_resp,
	struct miniisp_msg_t *p_msg_ack)
{
	int ret = 0;

	switch (p_resp->subcmd) {
	case SUB_CMD_SWING_CAM_CONFIG:
		swing_cam_log_info("[%s] Config Ack Received", __func__);
		break;
	case SUB_CMD_SWING_CAM_CAPTURE:
		swing_cam_log_info("[%s] Capture Ack Received, camid %d", __func__,
			p_msg_ack->u.ack_miniisp_request.camid);
		swing_cam_set_timestamp(
			&(p_msg_ack->u.ack_miniisp_request.timestamp_high),
			&(p_msg_ack->u.ack_miniisp_request.timestamp_low));
		break;
	case SUB_CMD_SWING_CAM_MATCH_ID:
		swing_cam_log_info("[%s] Received, match id, camid %d, status:%d", __func__,
			p_msg_ack->u.ack_query_igs_cam.camid,
			p_msg_ack->u.ack_query_igs_cam.status);
		break;
	case SUB_CMD_SWING_CAM_ACQUIRE:
		swing_cam_log_info("[%s] Received, acquire ,camid %d, csi_index:%u init_ret:%d", __func__,
			p_msg_ack->u.ack_acquire_igs_cam.camid,
			p_msg_ack->u.ack_acquire_igs_cam.csi_index,
			p_msg_ack->u.ack_acquire_igs_cam.init_ret);
		break;
	case SUB_CMD_SWING_CAM_EXTEND:
		swing_cam_log_info("[%s] Received, extend, camid %d, extend_cmd:%d", __func__,
			p_msg_ack->u.ack_extend_igs_set.camid,
			p_msg_ack->u.ack_extend_igs_set.extend_cmd);
		if (swing_cam_check_miniisp_status(p_resp, p_msg_ack) != 0) {
			swing_cam_log_err("[%s] miniisp is busy", __func__);
			ret = -EFAULT;
		}

		if (swing_cam_execute_subcmd_wakeup_lock(p_resp, p_msg_ack) != 0) {
			swing_cam_log_warn("[%s] miniisp wakeup ap lock", __func__);
			ret = -EFAULT;
		}
		break;
	case SUB_CMD_SWING_GET_OTP:
		swing_cam_log_info("[%s] Received, get_otp, camid %d, status:%d", __func__,
			p_msg_ack->u.ack_get_ao_otp.camid,
			p_msg_ack->u.ack_get_ao_otp.status);
		break;
	case SUB_CMD_SWING_SET_OTP:
		swing_cam_log_info("[%s] Received, set_otp", __func__);
		break;
	default:
		swing_cam_log_err("[%s] unhandled cmd: tag[%d], sub_cmd[%d]", __func__,
			p_resp->hd.tag, p_resp->subcmd);
		ret = -EFAULT;
		break;
	}
	return ret;
}

static int swing_cam_get_resp(const struct pkt_header *head)
{
	struct pkt_subcmd_resp *p_resp = NULL;
	int ret;
	struct miniisp_msg_t *p_msg_ack = NULL;

	p_resp = (struct pkt_subcmd_resp *)(head);

	if (!p_resp)
		return -EFAULT;

	if (p_resp->hd.tag != TAG_SWING_CAM)
		return -EFAULT;

	if (p_resp->hd.length < sizeof(struct miniisp_msg_t)) {
		swing_cam_log_err("[%s] invalid length[%d]", __func__, p_resp->hd.length);
		return -EFAULT;
	}

	p_msg_ack = (struct miniisp_msg_t *)(p_resp + 1);

	swing_cam_log_info("[%s] cmd[0x%x], subcmd[0x%x] length[%d]", __func__,
		p_resp->hd.cmd, p_resp->subcmd, p_resp->hd.length);

	swing_cam_log_info("[%s] tag[%d], errno[%d], tranid[%d]", __func__,
		p_resp->hd.tag, p_resp->hd.errno, p_resp->hd.tranid);

	swing_cam_log_info("[%s] size[0x%x], name[0x%x] id[0x%x]", __func__,
		p_msg_ack->message_size, p_msg_ack->api_name,
		p_msg_ack->message_id);

	if (p_resp->hd.cmd == CMD_CMN_CLOSE_RESP) {
		swing_cam_log_info("[%s] Close Ack Received, camid %d", __func__,
			p_msg_ack->u.ack_release_igs_cam.camid);
		ret = swing_cam_enqueue((unsigned char *)p_msg_ack,
			sizeof(struct miniisp_msg_t), MINIISP_MSG_CB);
		return ret;
	}

	ret = swing_cam_execute_subcmd(p_resp, p_msg_ack);
	if (ret != 0)
		return ret;

	ret = swing_cam_enqueue((unsigned char *)p_msg_ack,
		sizeof(struct miniisp_msg_t), MINIISP_MSG_CB);

	return ret;
}

static void swing_cam_clear_fifo(void)
{
	int i = 0;
	size_t length = sizeof(struct swingcam_data);
	unsigned int ret;
	struct swingcam_data read_data = {0};

	while (i < SWING_READ_CACHE_COUNT) {
		if (kfifo_len(&g_swing_priv.read_kfifo) < length) {
			swing_cam_log_err(
				"[%s]: kfifo len is less than swing data i=%d",
				__func__, i);
			break;
		}

		ret = kfifo_out(&g_swing_priv.read_kfifo,
				(unsigned char *)&read_data, length);
		if (ret == 0) {
			swing_cam_log_err("[%s]: kfifo out failed i=%d",
				__func__, i);
			break;
		}

		if (read_data.p_recv) {
			kfree(read_data.p_recv);
			read_data.p_recv = NULL;
			read_data.recv_len = 0;
		}
		i++;
	}
	kfifo_free(&g_swing_priv.read_kfifo);
}

static int swing_cam_dev_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	swing_cam_log_info("[%s] enter", __func__);

	(void)sd;
	(void)fh;
	mutex_lock(&g_swing_priv.swing_mutex);

	if (g_swing_priv.ref_cnt != 0) {
		swing_cam_log_err("[%s] duplicate open\n", __func__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -ENOMEM;
	}

	if (kfifo_alloc(&g_swing_priv.read_kfifo,
		sizeof(struct swingcam_data) * SWING_READ_CACHE_COUNT,
		GFP_KERNEL)) {
		swing_cam_log_err("[%s] kfifo alloc failed",
			__func__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -ENOMEM;
	}

	g_swing_priv.ref_cnt++;
	mutex_unlock(&g_swing_priv.swing_mutex);

	return 0;
}

static int swing_cam_dev_release(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	int rc = 0;

	swing_cam_log_info("[%s] enter", __func__);

	(void)sd;
	(void)fh;
	mutex_lock(&g_swing_priv.swing_mutex);
	if (g_swing_priv.ref_cnt == 0) {
		swing_cam_log_err("[%s] ref cnt is 0", __func__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -EFAULT;
	}

	g_swing_priv.ref_cnt--;

	if (g_swing_priv.ref_cnt == 0) {
		if (send_cmd_from_kernel(TAG_SWING_CAM, CMD_CMN_CLOSE_REQ, 0,
			NULL, (size_t)0) != 0) {
			swing_cam_log_err("[%s] send cmd error", __func__);
			rc = -EFAULT;
		} else {
			swing_cam_log_info("[%s] enter", __func__);
		}
		swing_cam_clear_fifo();
		swing_cam_log_info("[%s] finish", __func__);
	}

	mutex_unlock(&g_swing_priv.swing_mutex);

	return rc;
}

static int miniisp_reset_notify(void)
{
	int ret;
	struct miniisp_msg_t msg_ack = {0};

	msg_ack.u.event_sent.event_id = EVENT_MINI_RECOVER_CODE;
	msg_ack.api_name = MINI_EVENT_SENT;
	ret = swing_cam_enqueue((unsigned char *)(&msg_ack),
		sizeof(struct miniisp_msg_t), MINIISP_MSG_CB);
	swing_cam_log_info("[%s] enter", __func__);
	return ret;
}

static int miniisp_reset_notifier(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int ret = 0;
	(void)nb;
	(void)data;

	switch (action) {
	case IOM3_RECOVERY_START:
		miniisp_status = MINIISP_BUSY;
		swing_cam_log_info("[%s] IOM3_RECOVERY_START", __func__);
		break;
	case IOM3_RECOVERY_IDLE:
		ret = miniisp_reset_notify();
		miniisp_status = MINIISP_IDLE;
		swing_cam_log_info("[%s] IOM3_RECOVERY_IDLE", __func__);
		break;
	default:
		break;
	}

	return ret;
}

static struct notifier_block g_miniisp_reboot_notify = {
	.notifier_call = miniisp_reset_notifier,
	.priority = -1,
};

static int __init swing_cam_dev_init(struct platform_device *pdev)
{
	int ret;

	swing_cam_log_info("[%s] enter", __func__);

	if (is_sensorhub_disabled() != 0) {
		swing_cam_log_err("[%s] sensorhub disabled...", __func__);
		return -EFAULT;
	}

	mutex_init(&g_swing_priv.swing_mutex);

	ret = register_mcu_event_notifier(TAG_SWING_CAM,
		CMD_CMN_CONFIG_RESP, swing_cam_get_resp);
	if (ret != 0) {
		swing_cam_log_err("[%s] reg notifier failed. [%d]", __func__, ret);
		return -EFAULT;
	}
	ret = register_mcu_event_notifier(TAG_SWING_CAM,
		CMD_CMN_CLOSE_RESP, swing_cam_get_resp);
	if (ret != 0) {
		swing_cam_log_err("[%s] reg notifier failed. [%d]", __func__, ret);
		return -EFAULT;
	}
	ret = register_iom3_recovery_notifier(&g_miniisp_reboot_notify);
	if (ret < 0) {
		swing_cam_log_err("[%s] register_iom3_recovery_notifier fail", __func__);
		return ret;
	}
	g_swing_priv.ref_cnt = 0;
	g_swing_priv.self = &(pdev->dev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	g_swing_priv.igs_cam_wklock = wakeup_source_register(g_swing_priv.self, "igs-cam-wklock");
#else
	g_swing_priv.igs_cam_wklock = wakeup_source_register("igs-cam-wklock");
#endif
	if (!g_swing_priv.igs_cam_wklock) {
		swing_cam_log_err("[%s] wakeup source register failed\n", __func__);
		return -EFAULT;
	}

	swing_cam_log_info("[%s] exit, success", __func__);
	return 0;
}

static void __exit miniisp_dev_exit(void)
{
	swing_cam_log_info("[%s] enter", __func__);

	unregister_mcu_event_notifier(TAG_SWING_CAM,
		CMD_CMN_CLOSE_RESP, swing_cam_get_resp);
	unregister_mcu_event_notifier(TAG_SWING_CAM,
		CMD_CMN_CONFIG_RESP, swing_cam_get_resp);

	if (g_swing_priv.igs_cam_wklock->active)
		__pm_relax(g_swing_priv.igs_cam_wklock);

	wakeup_source_unregister(g_swing_priv.igs_cam_wklock);
	g_swing_priv.igs_cam_wklock = NULL;
}

static int miniisp_recv_rpmsg(void *arg)
{
	struct miniisp_msg_t *resp_msg = (struct miniisp_msg_t *)arg;

	struct swingcam_data read_data = {0};
	size_t length = sizeof(struct swingcam_data);
	int ret;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -1;
	}

	mutex_lock(&g_swing_priv.swing_mutex);
	if (g_swing_priv.ref_cnt == 0) {
		swing_cam_log_info("[%s] ref cnt is 0", __func__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -1;
	}

	if (kfifo_len(&g_swing_priv.read_kfifo) < length) {
		swing_cam_log_err("[%s] read data failed", __func__);
		mutex_unlock(&g_swing_priv.swing_mutex);
		return -1;
	}

	ret = (int)kfifo_out(&g_swing_priv.read_kfifo,
		(unsigned char *)&read_data, length);
	mutex_unlock(&g_swing_priv.swing_mutex);
	if (ret == 0) {
		swing_cam_log_err("[%s] kfifo out failed", __func__);
		return -1;
	}

	if (read_data.recv_len == sizeof(struct miniisp_msg_t)) {
		if (memcpy_s(resp_msg, sizeof(struct miniisp_msg_t),
			read_data.p_recv, sizeof(struct miniisp_msg_t)) != 0) {
			swing_cam_log_err("[%s] camera resp_msg copy fail", __func__);
			ret = -1;
		}
	} else {
		swing_cam_log_err("[%s] copy fail len[0x%x]",
			__func__, read_data.recv_len);
	}

	if (read_data.p_recv) {
		kfree(read_data.p_recv);
		read_data.p_recv = NULL;
		read_data.recv_len = 0;
	}

	return ret;
}

static int swing_cam_acquire_check(struct msg_req_acquire_igs_cam *acq)
{
	if (acq->sensor_spec.sensor_mode >= SENSOR_WORK_MODE_MAX)
		return -EFAULT;

	if (acq->sensor_spec.test_pattern >= SENSOR_TEST_PATTERN_MAX)
		return -EFAULT;

	return 0;
}

static int swing_v4l2_ioctl_cam_acquire(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_acquire_igs_cam *acp_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_ACQUIRE_IGS_CAM) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}
	swing_cam_log_info("[%s] api_name:0x%x id:0x%x size:0x%x",
		__func__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	acp_req = &(mini_req->u.req_acquire_igs_cam);

	(acp_req->sensor_name)[sizeof(acp_req->sensor_name) - 1] = '\0';
	(acp_req->i2c_bus_type)[sizeof(acp_req->i2c_bus_type) - 1] = '\0';
	swing_cam_log_info("[%s] camid:%d, i2c_index:%d, i2c_bus_type:%s",
		__func__, acp_req->camid, acp_req->i2c_index, acp_req->i2c_bus_type);

	swing_cam_log_info("[%s] sensor_mode:%d, test_patt:%d is_master:%d", __func__,
		acp_req->sensor_spec.sensor_mode,
		acp_req->sensor_spec.test_pattern,
		acp_req->sensor_spec.phy_info.is_master_sensor);

	swing_cam_log_info("[%s] phy_id:%d,phy_mode:%d, phy_work_mode:%d", __func__,
		acp_req->sensor_spec.phy_info.phy_id,
		acp_req->sensor_spec.phy_info.phy_mode,
		acp_req->sensor_spec.phy_info.phy_work_mode);

	if (swing_cam_acquire_check(acp_req)) {
		swing_cam_log_err("[%s] acquire check fail", __func__);
		return -EFAULT;
	}
	if (send_cmd_from_kernel(TAG_SWING_CAM, CMD_CMN_CONFIG_REQ,
		SUB_CMD_SWING_CAM_ACQUIRE,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s] send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_cam_config_check(struct msg_req_config_swing_cam *cfg)
{
	if (cfg->extension > FULL_SIZE)
		return -EFAULT;

	if (cfg->flow_mode > IGS_CONTINUOUS)
		return -EFAULT;

	if (cfg->process_pattern >= SENSOR_PIXEL_MAX)
		return -EFAULT;

	if (cfg->stream_cfg.format > IGS_PIXEL_FMT_YUV422I)
		return -EFAULT;

	return 0;
}

static int swing_v4l2_ioctl_cam_release(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_release_igs_cam *release_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_RELEASE_IGS_CAM) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}

	swing_cam_log_info("[%s] api_name:0x%x id:0x%x size:0x%x", __func__,
		mini_req->api_name, mini_req->message_id,
		mini_req->message_size);

	release_req = &(mini_req->u.req_release_igs_cam);

	swing_cam_log_info("[%s] camid:%d", __func__, release_req->camid);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_RELEASE,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_cam_config(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_config_swing_cam *cfg_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_USECASE_CONFIG_IGS_CAM) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}
	swing_cam_log_info("[%s] api_name:0x%x message_id:0x%x message_size:0x%x", __func__,
		mini_req->api_name, mini_req->message_id,
		mini_req->message_size);

	cfg_req = &(mini_req->u.req_usecase_cfg_swing_cam);
	swing_cam_log_info("[%s] camid:%d, extension:%d, flow_mode:%d is_secure:%d %d",
		__func__, cfg_req->camid, cfg_req->extension, cfg_req->flow_mode,
		cfg_req->is_secure[STREAM_MINIISP_OUT_PREVIEW],
		cfg_req->is_secure[STREAM_MINIISP_OUT_META]);

	swing_cam_log_info("[%s] pattern:%d w:%d, h:%d, stride:%d format:%d",
		__func__, cfg_req->process_pattern,
		cfg_req->stream_cfg.width, cfg_req->stream_cfg.height,
		cfg_req->stream_cfg.stride, cfg_req->stream_cfg.format);

	if (swing_cam_config_check(cfg_req)) {
		swing_cam_log_err("[%s] cfg check fail", __func__);
		return -EFAULT;
	}
	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_CONFIG,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s] send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_cam_match_id(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_query_igs_cam *match_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_QUERY_IGS_CAM) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}
	swing_cam_log_info("[%s] api_name:0x%x id:0x%x size:0x%x", __func__,
		mini_req->api_name, mini_req->message_id,
		mini_req->message_size);

	match_req = &(mini_req->u.req_query_igs_cam);

	(match_req->product_name)[sizeof(match_req->product_name) - 1] = '\0';
	(match_req->name)[sizeof(match_req->name) - 1] = '\0';
	swing_cam_log_info("[%s] camid:%d, pos:%d, i2c:%d, csi:%d, product_name:%s, sensor_name%s", __func__,
		match_req->camid, match_req->mount_position, match_req->i2c_index,
		match_req->csi_index, match_req->product_name, match_req->name);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_MATCH_ID,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s] send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_cam_capture(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct miniisp_req_request *cap_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_MINIISP_REQUEST) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}
	swing_cam_log_info("[%s] api_name:0x%x id:0x%x size:0x%x",
		__func__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	cap_req = &(mini_req->u.req_miniisp_request);
	swing_cam_log_info("[%s] camid:%d, map_fd:%d, frame:%d",
		__func__, cap_req->camid, cap_req->map_fd, cap_req->frame_num);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_CAPTURE,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_cam_extend(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_extend_igs_set *wxt_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_MINIISP_EXTEND_SET) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}
	swing_cam_log_info("[%s] api_name:0x%x id:0x%x size:0x%x",
		__func__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	wxt_req = &(mini_req->u.req_subcmd_swing_cam);
	swing_cam_log_info("[%s] camid=%d, extend_cmd=%d", __func__,
		wxt_req->camid, wxt_req->extend_cmd);

	swing_cam_log_info("[%s] paras[0]=%d, paras[1]=%d, paras[2]=%d, paras[3]=%d", __func__,
		wxt_req->paras[0], wxt_req->paras[1], wxt_req->paras[2], wxt_req->paras[3]);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_CAM_EXTEND,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s]send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_get_otp(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;
	struct msg_req_get_ao_otp_t *otp_req = NULL;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_GET_IGS_CAM_OTP) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}
	swing_cam_log_info("[%s] api_name:0x%x id:0x%x size:0x%x",
		__func__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	otp_req = &(mini_req->u.req_get_ao_otp);
	(otp_req->sensor_name)[sizeof(otp_req->sensor_name) - 1] = '\0';

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_GET_OTP,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s] send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_set_als(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_SET_ALS_IGS_CAM) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}

	swing_cam_log_info("[%s] api_name:0x%x id:0x%x size:0x%x", __func__,
		mini_req->api_name, mini_req->message_id,
		mini_req->message_size);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_SET_CAM_ALS,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s] send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static int swing_v4l2_ioctl_set_otp(void *arg)
{
	struct miniisp_msg_t *mini_req = (struct miniisp_msg_t *)arg;

	if (!arg) {
		swing_cam_log_err("[%s] arg NULL", __func__);
		return -EFAULT;
	}

	if (mini_req->api_name != COMMAND_SET_IGS_CAM_OTP) {
		swing_cam_log_err("[%s] api_name err", __func__);
		return -EFAULT;
	}
	swing_cam_log_info("[%s] api_name:0x%x id:0x%x size:0x%x",
		__func__, mini_req->api_name,
		mini_req->message_id, mini_req->message_size);

	if (send_cmd_from_kernel(TAG_SWING_CAM,
		CMD_CMN_CONFIG_REQ, SUB_CMD_SWING_SET_OTP,
		(char *)mini_req, sizeof(struct miniisp_msg_t)) != 0) {
		swing_cam_log_err("[%s] send cmd error", __func__);
		return -EFAULT;
	}

	return 0;
}

static long miniisp_vo_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	long ret = 0;
	(void)sd;

	swing_cam_log_info("[%s] cmd is (0x%x)",
		__func__, cmd);

	if ((arg) && ((sizeof(struct miniisp_msg_t) > MAX_IGS_PAYLOAD)))
		return -EFAULT;

	switch (cmd) {
	case IGS_IOCTL_CAM_OPEN:
		send_cmd_from_kernel(TAG_SWING_CAM,
			CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0);
		break;

	case IGS_IOCTL_INIT_TIMESTAMP:
		swing_cam_set_timestamp_source(arg);
		swing_cam_init_timestamp();
		break;

	case IGS_IOCTL_CAM_CLOSE:
		ret = swing_v4l2_ioctl_cam_release(arg);
		swing_cam_destroy_timestamp();
		break;

	case SWING_IOCTL_CAM_CONFIG:
		ret = swing_v4l2_ioctl_cam_config(arg);
		break;

	case IGS_IOCTL_CAM_CAPTURE:
		ret = swing_v4l2_ioctl_cam_capture(arg);
		break;

	case SWING_IOCTL_CAM_MATCH_ID:
		ret = swing_v4l2_ioctl_cam_match_id(arg);
		break;

	case SWING_IOCTL_CAM_RECV_RPMSG:
		ret = miniisp_recv_rpmsg(arg);
		break;

	case SWING_IOCTL_CAM_ACQUIRE:
		ret = swing_v4l2_ioctl_cam_acquire(arg);
		break;

	case SWING_IOCTL_CAM_SET_EXTEND:
		ret = swing_v4l2_ioctl_cam_extend(arg);
		break;

	case IGS_IOCTL_CAM_GET_OTP:
		ret = swing_v4l2_ioctl_get_otp(arg);
		break;

	case IGS_IOCTL_CAM_SET_ALS:
		ret = swing_v4l2_ioctl_set_als(arg);
		break;

	case IGS_IOCTL_CAM_SET_OTP:
		ret = swing_v4l2_ioctl_set_otp(arg);
		break;

	default:
		swing_cam_log_err("[%s] unknown cmd : 0x%x", __func__, cmd);
		return -ENOTTY;
	}

	return ret;
}

static int miniisp_subdev_subscribe_event(struct v4l2_subdev *sd,
	struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
	(void)sd;
	swing_cam_log_info("[%s] enter", __func__);
	return v4l2_event_subscribe(fh, sub, 128, NULL); /* 128:length */
}

static int miniisp_subdev_unsubscribe_event(struct v4l2_subdev *sd,
	struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
	(void)sd;
	swing_cam_log_info("[%s] enter", __func__);
	return v4l2_event_unsubscribe(fh, sub);
}

static struct v4l2_subdev_core_ops s_subdev_core_ops_miniisp = {
	.ioctl = miniisp_vo_subdev_ioctl,
	.subscribe_event = miniisp_subdev_subscribe_event,
	.unsubscribe_event = miniisp_subdev_unsubscribe_event,
};

static struct v4l2_subdev_internal_ops s_subdev_internal_ops_miniisp = {
	.open = swing_cam_dev_open,
	.close = swing_cam_dev_release,
};

static struct v4l2_subdev_ops s_subdev_ops_miniisp = {
	.core = &s_subdev_core_ops_miniisp,
};

static int miniisp_probe(struct platform_device *pdev)
{
	int rc;
	struct v4l2_subdev *subdev = NULL;

	subdev = &g_miniisp.subdev;

	if (!pdev) {
		swing_cam_log_err("[%s] pdev is NULL", __func__);
		return -EFAULT;
	}

	swing_cam_log_info("[%s] enter", __func__);

	g_miniisp.pdev = pdev;
	mutex_init(&g_miniisp.lock);
	v4l2_subdev_init(subdev, &s_subdev_ops_miniisp);
	subdev->internal_ops = &s_subdev_internal_ops_miniisp;
	(void)snprintf_s(subdev->name, sizeof(subdev->name),
		sizeof(subdev->name), "hwcam-cfg-miniisp");
	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;

	(void)init_subdev_media_entity(subdev, CAM_SUBDEV_IGS);
	(void)cam_cfgdev_register_subdev(subdev, CAM_SUBDEV_IGS);
	subdev->devnode->lock = &g_miniisp.lock;
	platform_set_drvdata(pdev, subdev);

	rc = swing_cam_dev_init(pdev);
	swing_cam_log_info("[%s] exit", __func__);

	return rc;
}

static int __exit miniisp_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev = NULL;

	swing_cam_log_info("[%s]...", __func__);

	(void)pdev;
	if (g_miniisp.pdev == NULL)
		return -EFAULT;
	subdev = platform_get_drvdata(g_miniisp.pdev);
	media_entity_cleanup(&subdev->entity);
	cam_cfgdev_unregister_subdev(subdev);
	mutex_destroy(&g_miniisp.lock);

	miniisp_dev_exit();

	return 0;
}

static const struct of_device_id g_swing_cam_match_table[] = {
	{ .compatible = SWING_CAM_DRV_NAME, },
	{},
};

static struct platform_driver g_swing_cam_platdev = {
	.driver = {
		.name = "huawei,swing_cam_dev",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_swing_cam_match_table),
	},
	.probe  = miniisp_probe,
	.remove = miniisp_remove,
};

static int __init swing_cam_main_init(void)
{
	swing_cam_log_info("[%s] enter", __func__);
	return platform_driver_register(&g_swing_cam_platdev);
}

static void __exit swing_cam_main_exit(void)
{
	swing_cam_log_info("[%s] enter", __func__);
	platform_driver_unregister(&g_swing_cam_platdev);
}

late_initcall_sync(swing_cam_main_init);
module_exit(swing_cam_main_exit);
MODULE_DESCRIPTION("swingcamdev");
MODULE_LICENSE("GPL v2");
