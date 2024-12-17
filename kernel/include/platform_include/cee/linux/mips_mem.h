/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * mips.h
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _MIPS_MEM_H
#define _MIPS_MEM_H
#include <securec.h>
struct pcounts {
	u64 inst;
	u64 cycle;
	u64 l2refill;
	u64 l3refill;
	int freq;
};
#ifdef CONFIG_MIPS_MEM_FREQ
void check_l3_ddr_freq_change(int cpu);
void set_mips_mem_state(bool enabled);
#else
static inline void check_l3_ddr_freq_change(int cpu) {}
static inline void set_mips_mem_state(bool enabled) {}
#endif
#endif
