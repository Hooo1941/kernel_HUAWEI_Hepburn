/*
 * lcd_kit_power_key.h
 *
 * lcdkit function for lcdkit head file
 *
 * Copyright (c) 2022-2024 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __LCD_KIT_POWER_KEY__
#define __LCD_KIT_POWER_KEY__

#define LONG_PRESS_RST_CONFIG1 0
#define LONG_PRESS_RST_CONFIG2 2
#define LONG_PRESS_RST_CONFIG3 4
#define LONG_PRESS_RST_CONFIG4 6
#define LONG_PRESS_8S_LCD_TIMER_LEN 1700
#define LONG_PRESS_10S_LCD_TIMER_LEN 3700
#define LONG_PRESS_30S_LCD_TIMER_LEN 23700
#define LONG_PRESS_60S_LCD_TIMER_LEN 53700
#define POWERKEY_RESTART_TYPE_PROP      "ro.boot.powerkey_restart_type"

enum powerkey_restart_type {
	PRESS_POWER_ONLY,
	PRESS_POWER_VOL_DOWN,
	PRESS_POWER_VOL_UP,
};

enum {
	PWR_OFF_NO_NEED_DISABLE_ESD = 0,
	PWR_OFF_NEED_DISABLE_ESD = 1,
	PWR_OFF_DISABLED_ESD = 2,
	PWR_OFF_ESD_STATUS_BUTT,
};

void lcd_kit_register_power_key_notify(int panel_id);
#endif

