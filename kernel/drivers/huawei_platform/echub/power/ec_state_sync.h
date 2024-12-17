/*
 * ec_state_sync.h
 *
 * ec_state_sync driver for suspend/resume
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef _ARMPC_EC_STATE_SYNC
#define _ARMPC_EC_STATE_SYNC

#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
extern int get_sleep_count(void);
#else
static inline int get_sleep_count(void) { return 0; }
#endif

#endif
