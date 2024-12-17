/*
 * lcd_kit_ext_power.h
 *
 * lcdkit ext power function for lcd driver head file
 *
 * Copyright (c) 2023-2024 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_EXT_POWER__
#define __LCD_KIT_EXT_POWER__

struct lcd_pwr_off_optimize_config {
	int working_flag; /* 0 means no work, 1 means primary, 2 means secondary */
	unsigned int delay_time;
	unsigned int panel_power_off_seq_cnt;
	struct lcd_kit_array_data *panel_power_off_event;
	struct lcd_kit_array_data *lcd_vci;
	struct lcd_kit_array_data *lcd_iovcc;
	struct lcd_kit_array_data *lcd_rst;
	struct lcd_kit_array_data *lcd_vdd;
	struct delayed_work pwr_off_optimize_work;
};

void lcd_kit_power_off_optimize_handle(int panel_id);
void lcd_kit_register_power_off_optimize(void);
int lcd_kit_gpio_tx_plugin(uint32_t type, uint32_t op);
void wait_power_off_done(int panel_id);
#endif
