/*
 * Copyright (C) huawei company
 *
 * This	program	is free	software; you can redistribute it and/or modify
 * it under	the	terms of the GNU General Public	License	version	2 as
 * published by	the	Free Software Foundation.
 */
#ifndef __FINGERPRINT_INTERFACE_H
#define __FINGERPRINT_INTERFACE_H

struct ud_fp_display_ops {
	void (*de_open_update_te)(void);
	uint32_t (*fp_get_local_hbm_mode)(void);
	int (*fp_set_brightness_level)(int brightness_level);
};

void ud_fp_display_register(struct ud_fp_display_ops *fp_display_ops);

void ud_fp_dispaly_unregister(void);

void ud_fp_on_hbm_completed(void);

void fp_on_lcd_backlight_change(int panel_id, int value);

void fp_set_lcd_charge_time(int time);

void fp_set_lcd_light_on_time(int time);

void fp_set_cpu_wake_up_time(int time);

#endif