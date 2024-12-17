/*
 * alloc_acct.h
 *
 * Memory alloc delay accounting
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _ALLOC_ACCT_H
#define _ALLOC_ACCT_H

#include <linux/sched.h>

#define MAX_ALLOC_ORDER 10
/* RA is the abbreviation of alloc accouting */
enum alloc_acct_stubs {
	AA_PGFAULT = 0,
	AA_SPECULATIVE_PGFAULT,
};
#define NR_AA_STUBS (AA_SPECULATIVE_PGFAULT + 1)
#define NR_ALL_STUBS (NR_AA_STUBS + MAX_ALLOC_ORDER + 1)

#define PGFAULT_STR "pgfault"
#define SPECULATIVE_PGFAULT_STR "speculative_pgfault"

#define DELAY_LV0 1000000 /* 1ms */
#define DELAY_LV1 5000000 /* 5ms */
#define DELAY_LV2 10000000 /* 10ms */
#define DELAY_LV3 50000000 /* 50ms */
#define DELAY_LV4 100000000 /* 100ms */
#define DELAY_LV5 1000000000 /* 1000ms */
#define DELAY_LV6 20000000000 /* 20000ms */
#define NR_DELAY_LV 7

void alloc_acct_pgfault_start(void);
void alloc_acct_pgfault_end(void);
void alloc_acct_spf_start(void);
void alloc_acct_spf_end(void);
void alloc_acct_allocpage_start(void);
void alloc_acct_allocpage_end(unsigned int order);

enum aa_show_type {
	AA_DELAY,
	AA_COUNT,
};

/*
 * When type is AA_DELAY or AA_COUNT, the caller should make sure
 * 0 <= level < NR_DELAY_LV and 0 <= stub < NR_AA_STUBS.
 */
u64 alloc_acct_get_data(enum aa_show_type type, int level, int stub);

#endif /* _ALLOC_ACCT_H */
