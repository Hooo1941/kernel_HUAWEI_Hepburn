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

#ifndef BLACKBOX_SUBSYSTEM_DEF_H
#define BLACKBOX_SUBSYSTEM_DEF_H

/* module type */
#define MODULE_CP "CP"
#define MODULE_SLPI "SLPI"
#define MODULE_ADSP "ADSP"
#define MODULE_CDSP "CDSP"
#define MODULE_MSS "MODEM"
#define MODULE_WLAN "WLAN"
#define MODULE_BT "BT"
#define MODULE_VENUS "VENUS"

/* subsystem fault event type */
#define EVENT_CP_CRASH "CP_CRASH"
#define EVENT_SLPI_CRASH "SLPI_CRASH"
#define EVENT_ADSP_CRASH "ADSP_CRASH"
#define EVENT_CDSP_CRASH "CDSP_CRASH"
#define EVENT_MSS_CRASH "MODEM_CRASH"
#define EVENT_WLAN_CRASH "WLAN_CRASH"
#define EVENT_BT_CRASH "BT_CRASH"
#define EVENT_VENUS_CRASH "VENUS_CRASH"

/* fault category type */
#define CATEGORY_CP_CRASH "CP_CRASH"
#define CATEGORY_SLPI_CRASH "SLPI_CRASH"
#define CATEGORY_ADSP_CRASH "ADSP_CRASH"
#define CATEGORY_CDSP_CRASH "CDSP_CRASH"
#define CATEGORY_MSS_CRASH "MODEM_CRASH"
#define CATEGORY_WLAN_CRASH "WLAN_CRASH"
#define CATEGORY_BT_CRASH "BT_CRASH"
#define CATEGORY_VENUS_CRASH "VENUS_CRASH"

/* subsystem fault error_desc */
#define ERROR_DESC_CP_CRASH "cp_crash"
#define ERROR_DESC_SLPI_CRASH "slpi_crash"
#define ERROR_DESC_ADSP_CRASH "adsp_crash"
#define ERROR_DESC_CDSP_CRASH "cdsp_crash"
#define ERROR_DESC_MSS_CRASH "modem_crash"
#define ERROR_DESC_WLAN_CRASH "wlan_crash"
#define ERROR_DESC_BT_CRASH "bt_crash"
#define ERROR_DESC_VENUS_CRASH "venus_crash"

#endif /* BLACKBOX_SUBSYSTEM_DEF_H */
