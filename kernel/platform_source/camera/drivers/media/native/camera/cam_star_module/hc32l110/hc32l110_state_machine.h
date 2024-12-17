/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hc32l110_state_machine.h
 *
 * hc32l110_state_machine header file
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#ifndef _HC32L110_STATE_MACHINE_H_
#define _HC32L110_STATE_MACHINE_H_

#include "../cam_star_module_adapter.h"

enum work_state_t {
	MATCH_BOOTLOADER_ID = 0,
	EXT_MATCH_BOOTLOADER_ID,
	DOWNLOAD_FW,
	TURN_TO_BOOTLOADER,
	EJECT,
	RETRACTION,
	RELOAD_EEPROM_TO_FLASH,
};

struct power_operation_t {
	int (*action)(struct cam_star_module_device_info *di);
};

struct power_state_transformer_t {
	enum power_state_t current_state;
	enum work_state_t event_id;
	enum power_state_t next_state;
	struct power_operation_t power_operation;
};

struct power_state_machine_t {
	enum power_state_t power_state;
	/* num of transformer in this state machine */
	int trans_num;
	struct power_state_transformer_t *power_state_transformer;
};

struct power_state_transformer_t *find_trans(enum work_state_t work_state);
int hc32l110_power_state_machine(struct cam_star_module_device_info *di, enum work_state_t work_state);
int hc32l110_get_power_state(void);
int hc32l110_power_state_machine_init(void);

#endif /* _HC32L110_STATE_MACHINE_H_ */
