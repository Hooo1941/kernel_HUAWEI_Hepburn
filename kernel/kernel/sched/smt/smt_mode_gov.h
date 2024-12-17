// SPDX-License-Identifier: GPL-2.0
/*
 * smt_mode_gov.h
 *
 * smt mode governor header file
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef __SMT_MODE_GOV_H
#define __SMT_MODE_GOV_H

extern void set_smt_cpu_req(int cpu, unsigned int req);
extern unsigned int get_smt_cpu_req(int cpu);

#endif
