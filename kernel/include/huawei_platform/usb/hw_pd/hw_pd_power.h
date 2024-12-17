/*
 * hw_pd_power.h
 *
 * Header file of power operation interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HW_PD_POWER_H_
#define _HW_PD_POWER_H_

enum PD_POWER_TYPE {
	PD_POWER_BOOST,
	PD_POWER_POGOPIN,
};

#define P_OK 1
#define P_ER (-1)

/* set vbus mode of a port */
void hw_pd_power_set_state(enum PD_POWER_TYPE pwr_type, int on);
/* set vconn pin power supply */
void hw_pd_power_set_vconn(int enable);
/* depending on the power setting icon_type */
void hw_pd_power_set_max_power(int max_power);
/* send plugin to charger */
void hw_pd_power_set_source_sink_state(int type);
/* depending on the vbus and power setting high_voltage/power */
void hw_pd_power_set_charge_state(int mv, int ma);
/* open the external power supply */
void hw_pd_power_vbus_ch_open(void);
/* close the external power supply */
void hw_pd_power_vbus_ch_close(void);
void hw_pd_power_init(void);
/* get bc1.2 charger type */
unsigned long hw_pd_power_get_chgtype(void);
/* send icon_type event to ui */
void pd_send_wireless_cover_uevent(void);
/* send cc_short action to charger */
void hw_pd_cc_short_action(void);

#ifdef CONFIG_UVDM_CHARGER
void uvdm_init_work(void);
void uvdm_cancel_work(void);
#else
static inline void uvdm_init_work(void) {}
static inline void uvdm_cancel_work(void) {}
#endif /* CONFIG_UVDM_CHARGER */

#endif

