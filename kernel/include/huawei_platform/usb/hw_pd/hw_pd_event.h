/*
 * hw_pd_event.h
 *
 * Header file of platform event reporting for huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HW_PD_EVENT_H_
#define _HW_PD_EVENT_H_

#define HW_PD_EVENT_TYPE_OCP  1
#define HW_PD_EVENT_TYPE_FB   2
#define HW_PD_EVENT_TYPE_BC12 3

#define EVENT_OK 0

struct bc12_event {
	unsigned long event;
	void *data;
};

typedef void (*hw_pd_event_cb)(unsigned int type, char *event_data);

/* register pd ocp_event/bc12_event/fb_event */
int hw_pd_event_reg(unsigned int type, const char *context, hw_pd_event_cb cb);

#endif
