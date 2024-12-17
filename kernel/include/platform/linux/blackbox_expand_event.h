/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef BLACKBOX_EXPAND_EVENT_H
#define BLACKBOX_EXPAND_EVENT_H

#include <platform/linux/blackbox_subsystem_def.h>

/* extend ap fault event type */
#define EVENT_UNDEFINE_CMD "undefine_cmd"

	{
		MODULE_SYSTEM,
		{EVENT_UNDEFINE_CMD, CATEGORY_SYSTEM_PANIC, TOP_CATEGORY_SYSTEM_RESET}
	},
	{
		MODULE_CP,
		{EVENT_CP_CRASH, CATEGORY_CP_CRASH, TOP_CATEGORY_SUBSYSTEM_CRASH}
	},
	{
		MODULE_SLPI,
		{EVENT_SLPI_CRASH, CATEGORY_SLPI_CRASH, TOP_CATEGORY_SUBSYSTEM_CRASH}
	},
	{
		MODULE_ADSP,
		{EVENT_ADSP_CRASH, CATEGORY_ADSP_CRASH, TOP_CATEGORY_SUBSYSTEM_CRASH}
	},
	{
		MODULE_CDSP,
		{EVENT_CDSP_CRASH, CATEGORY_CDSP_CRASH, TOP_CATEGORY_SUBSYSTEM_CRASH}
	},
	{
		MODULE_MSS,
		{EVENT_MSS_CRASH, CATEGORY_MSS_CRASH, TOP_CATEGORY_SUBSYSTEM_CRASH}
	},
	{
		MODULE_WLAN,
		{EVENT_WLAN_CRASH, CATEGORY_WLAN_CRASH, TOP_CATEGORY_SUBSYSTEM_CRASH}
	},
	{
		MODULE_BT,
		{EVENT_BT_CRASH, CATEGORY_BT_CRASH, TOP_CATEGORY_SUBSYSTEM_CRASH}
	},
	{
		MODULE_VENUS,
		{EVENT_VENUS_CRASH, CATEGORY_VENUS_CRASH, TOP_CATEGORY_SUBSYSTEM_CRASH}
	},

#endif /* BLACKBOX_EXPAND_EVENT_H */
