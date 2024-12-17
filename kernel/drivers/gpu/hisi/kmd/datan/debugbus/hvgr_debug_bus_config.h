/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

/* Backend specific configuration */
#ifndef HVGR_DEBUG_BUS_CONFIG_H
#define HVGR_DEBUG_BUS_CONFIG_H

#define BASE_FCM_MAX_NR_SLOTS       8
#define BASE_YEAR                   1900
#define BASE_NUM_TWO                2
#define INVALID_REGISTER_VALUE      0xdeadbeef

#ifdef CONFIG_LIBLINUX
#define HVGR_DEBUG_BUS_RECORD_LEN   320
#define HVGR_DEBUG_BUS_RECORD       "src_file[%s] dest_file[%s]\n"
#define HVGR_DEBUG_BUS_DEBUG_FS     "/sys/kernel/debug/hvgr0/datan/debugbus"
#endif

#endif /* HVGR_DEBUG_BUS_CONFIG_H */
