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

#ifndef BLACKBOX_SUBSYSTEM_H
#define BLACKBOX_SUBSYSTEM_H

#define SUBSYSTEM_TYPE_NUM  7

#define SLPI_SUBSYS_NUMBER  1
#define ADSP_SUBSYS_NUMBER  2
#define CDSP_SUBSYS_NUMBER  3
#define MODEM_SUBSYS_NUMBER 4
#define WLAN_SUBSYS_NUMBER  5
#define BT_SUBSYS_NUMBER    6
#define VENUS_SUBSYS_NUMBER 7

#define SUBSYS_CRASH_RAMDUMP_PATH  ", /data/vendor/log/ramdump"

int bbox_subsystem_crash_notify(const char *name);
void subsystem_register_module_ops(void);

#endif /* BLACKBOX_SUBSYSTEM_H */
