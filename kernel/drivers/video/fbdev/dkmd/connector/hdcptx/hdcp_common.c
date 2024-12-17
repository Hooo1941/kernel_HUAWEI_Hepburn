/* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 and only version 2
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
*/

#include "hdcp_common.h"

#include <securec.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "dkmd_log.h"

#define HDCP_STR_BUFFER_LENGTH 120

#define HDCP_STOP_WAIT_TIME 100 /* 1000ms */
#define HDCP_STOP_CHECK_INTERVAL 10

static struct hdcp_ctrl g_hdcp_control;

// Report the uevent event to the HAL layer
void hdcp_notification(struct switch_dev *sdev, uint32_t state)
{
	char *envp[3];
	char name_buffer[HDCP_STR_BUFFER_LENGTH] = {0};
	char state_buffer[HDCP_STR_BUFFER_LENGTH] = {0};
	int length;

	if (sdev == NULL) {
		dpu_pr_err("[HDCP] sdev is NULL!\n");
		return;
	}

	length = snprintf_s(name_buffer, sizeof(name_buffer), sizeof(name_buffer) - 1, "SWITCH_NAME=%s", sdev->name);
	if (length < 0 || length >= HDCP_STR_BUFFER_LENGTH) {
		dpu_pr_err("[HDCP] fatal: snprintf_s name_buffer fail, length=%d\n", length);
		return;
	}

	length = snprintf_s(state_buffer, sizeof(state_buffer), sizeof(state_buffer) - 1, "SWITCH_STATE=%u", state);
	if (length < 0 || length >= HDCP_STR_BUFFER_LENGTH) {
		dpu_pr_err("[HDCP] fatal: snprintf_s state_buffer fail, length=%d\n", length);
		return;
	}

	envp[0] = name_buffer;
	envp[1] = state_buffer;
	envp[2] = NULL;
	kobject_uevent_env(&sdev->dev->kobj, KOBJ_CHANGE, envp);

	dpu_pr_info("[HDCP] Notification: %s, %s.\n", name_buffer, state_buffer);
}

// workqueue scheduling response function
static void hdcp_notify_wq_handler(struct work_struct *work)
{
	struct hdcp_ctrl *hdcp = NULL;

	hdcp = container_of(work, struct hdcp_ctrl, hdcp_notify_work);

	if (hdcp == NULL) {
		dpu_pr_err("[HDCP] hdcp is null!\n");
		return;
	}
	if (hdcp->dptx == NULL) {
		dpu_pr_err("[HDCP] hdcp->dptx is null!\n");
		return;
	}

	hdcp_notification(&hdcp->dptx->sdev, hdcp->notification);
}

static void hdcp_init(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		dpu_pr_err("[HDCP]dptx is null!\n");
		return;
	}

	if (g_hdcp_control.hdcp_notify_wq == NULL) {
		g_hdcp_control.counter = 0;
		g_hdcp_control.dptx = dptx;
		g_hdcp_control.notification = 0;
		g_hdcp_control.hdcp_notify_wq = create_singlethread_workqueue("hdcp_notify");
		if (g_hdcp_control.hdcp_notify_wq == NULL) {
			dpu_pr_err("[HDCP]create hdcp_wq failed!\n");
			return;
		}
		INIT_WORK(&g_hdcp_control.hdcp_notify_work, hdcp_notify_wq_handler);
		dpu_pr_info("[HDCP]hdcp kernel is initialized\n");
	} else {
		dpu_pr_info("[HDCP]hdcp kernel is already initialized!\n");
	}
}

static void hdcp_deinit(struct dp_ctrl *dptx)
{
	if (g_hdcp_control.dptx != dptx) {
		dpu_pr_err("[HDCP]dptx unmatch\n");
		return;
	}

	g_hdcp_control.dptx = NULL;
	g_hdcp_control.notification = 0;
	if (g_hdcp_control.hdcp_notify_wq != NULL) {
		destroy_workqueue(g_hdcp_control.hdcp_notify_wq);
		g_hdcp_control.hdcp_notify_wq = NULL;
	}

	hdcp_notification(&dptx->sdev, HOT_PLUG_HDCP_OUT);

	dpu_pr_info("[HDCP]hdcp kernel is de-init\n");
}

// DP enable/disable, according to input param en, initialize or deinitialize HDCP
void hdcp_dp_on(struct dp_ctrl *dptx, bool en)
{
	if (dptx == NULL) {
		dpu_pr_err("[HDCP]dptx is null!\n");
		return;
	}

	if (en)
		hdcp_init(dptx);
	else
		hdcp_deinit(dptx);
}

/* The role of cp_irq: HDCP interacts with the peer device, and the peer needs to return
   the status of the peer device through cp_irq (data is sent to the display, the display
   returns whether it is ready, or whether re-authentication is required) */
void hdcp_handle_cp_irq(struct dp_ctrl *dptx)
{
	g_hdcp_control.notification = HOT_PLUG_HDCP_CP_IRQ;
	if (g_hdcp_control.hdcp_notify_wq != NULL)
		queue_work(g_hdcp_control.hdcp_notify_wq, &(g_hdcp_control.hdcp_notify_work));
}

void wait_hdcp_quit(void)
{
	int i = 0;
	while (g_hdcp_control.counter > 0) {
		if (i < HDCP_STOP_WAIT_TIME) {
			msleep(HDCP_STOP_CHECK_INTERVAL); // wait for hdcp ta exit
			i++;
		} else {
			break;
		}
	}
	dpu_pr_info("[HDCP] wait hdcp quit time %d ms", i * HDCP_STOP_CHECK_INTERVAL);
}

void hdcp_increase_counter(void)
{
	g_hdcp_control.counter++;
	dpu_pr_info("[HDCP] receive increase counter = %d", g_hdcp_control.counter);
}

void hdcp_decrease_counter(void)
{
	g_hdcp_control.counter--;
	dpu_pr_info("[HDCP] receive decrease counter = %d", g_hdcp_control.counter);
	if (g_hdcp_control.counter < 0) {
		dpu_pr_warn("[HDCP]hdcp counter < 0");
		g_hdcp_control.counter = 0;
	}
}

