/*
 *
 * rdr dfx poweroff header file.
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_POWEROFF_H__
#define __DFX_POWEROFF_H__
#include <linux/reboot.h>

typedef void (*arm_pm_restart_func)(enum reboot_mode reboot_mode, const char *cmd);

arm_pm_restart_func get_arm_pm_restart_func(void);

typedef void (*shipmode_cb_t)(void);
#ifdef CONFIG_SHIPMODE_CB
void register_shipmode_cb(shipmode_cb_t func);
#else
static inline void register_shipmode_cb(shipmode_cb_t func) {return;}
#endif
#endif