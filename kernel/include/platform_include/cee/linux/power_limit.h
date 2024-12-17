// SPDX-License-Identifier: GPL-2.0
/*
 * power_limit.h
 *
 * header file of drg module
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_LIMIT_H
#define _POWER_LIMIT_H

#ifdef CONFIG_SCHED_POWER_LIMIT_MAXFREQ
void power_limit_check(bool sched_avg_updated);
extern unsigned int g_power_limit_enable;
#else
static inline void power_limit_check(bool sched_avg_updated) {}
#endif
#endif
