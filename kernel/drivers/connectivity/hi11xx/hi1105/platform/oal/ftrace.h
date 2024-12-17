/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:ftrace.c header file
 * Author: @CompanyNameTag
 */

#ifndef FTRACE_HISI_H
#define FTRACE_HISI_H

#ifdef FTRACE_ENABLE

#include <linux/types.h>
#include <linux/sort.h>
#include <linux/ktime.h>

#undef FTRACE_DEBUG

typedef struct _ftrace_event_ {
    struct hlist_node hlist;
    unsigned long total_count;
    unsigned int refcount;
    ktime_t time_stamp;      // enter time
    ktime_t total_cost;      // function total time cost
    unsigned long func_addr; // function address
} ftrace_event;

struct ftrace_hash {
    unsigned long size_bits;
    struct hlist_head *buckets;
    unsigned long count;
};

void notrace ftrace_hisi_init(void);
void notrace ftrace_hisi_exit(void);
#endif
#endif
