/* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
*/

#ifndef HDCP_COMMON_H
#define HDCP_COMMON_H

#include "dp_ctrl.h"

struct hdcp_ctrl {
	struct dp_ctrl *dptx;
	struct workqueue_struct *hdcp_notify_wq;
	struct work_struct hdcp_notify_work;
	uint32_t notification;
	int32_t counter;
};

void hdcp_dp_on(struct dp_ctrl *dptx, bool en);
void hdcp_handle_cp_irq(struct dp_ctrl *dptx);
void hdcp_notification(struct switch_dev *sdev, uint32_t state);
void wait_hdcp_quit(void);
void hdcp_increase_counter(void);
void hdcp_decrease_counter(void);

#endif

