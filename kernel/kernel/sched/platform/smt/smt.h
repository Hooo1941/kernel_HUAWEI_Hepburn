// SPDX-License-Identifier: GPL-2.0
/*
 * smt.h
 *
 * smt header file
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
 *
 */

#ifndef __SMT_H
#define __SMT_H

#include <linux/sched/smt.h>

extern unsigned int sysctl_task_cap_discount_in_mt;
extern unsigned int sysctl_top_task_cap_discount_in_mt;
extern unsigned int sysctl_cpu_cap_discount_in_mt;

#ifdef CONFIG_SCHED_SMT
enum smt_mode get_smt_mode(int cpu, bool is_current);
#else
static inline enum smt_mode get_smt_mode(int cpu, bool is_current) { return ST_MODE; }
#endif

#endif
