/*
 * hw_pd_misc.h
 *
 * Header file of misc device interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HW_PD_MISC_H_
#define _HW_PD_MISC_H_

enum ANALOG_STATE {
	ANALOG_DETTACH,
	ANALOG_ATTACH,
	ANALOG_DISABLE,
};

/* hand audio plug out */
void hw_pd_misc_power_off(void);
/* response audio attach/detach event */
void hw_pd_misc_typec_state(int detach);
/* check typec event validity while audio mode */
bool pogopin_ignore_typec_event(unsigned long event, void *data, int cur_usb_event);
/* response analog plug in/out while audio mode */
void hw_pd_misc_notify_analog(int state);
/* check and send analog attach/detach event */
int hw_pd_misc_pre_handle_analog(enum ANALOG_STATE state);
/* response analog state change */
int hw_pd_misc_handle_analog(enum ANALOG_STATE state);
/* send cover event to wireless charger */
void hw_pd_misc_handle_wireless_cover_event(
	unsigned int vid, unsigned int pid, unsigned int bcd);

#endif

