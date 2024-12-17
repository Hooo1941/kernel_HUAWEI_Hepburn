/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description config devive source file.
 * Create 2023-09-20

 * This program is free software; you can redistribute it andor modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __CAM_MISC_H__
#define __CAM_MISC_H__

struct cam_vib_notify_data {
	atomic_t vib_status_update;
	char vib_status;
	char reserve[3];
	unsigned long vib_shake_time;
};

struct  cam_vib_notify_dev {
	struct device *dev;
	struct miscdevice cam_vib_notify_misc;
	struct wakeup_source *cam_vib_protect_ws;
	wait_queue_head_t wait;
	struct mutex mux_lock;
	struct cam_vib_notify_data cam_vib_data;
};

static void hw_actuator_protect_work(struct cam_vib_notify_data *cam_vib_data);
#endif
