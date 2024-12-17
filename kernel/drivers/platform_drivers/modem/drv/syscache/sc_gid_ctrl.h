/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description: DRA driver source code
 */
#ifndef __SC_GID_CTRL_H_
#define __SC_GID_CTRL_H_

#include <linux/types.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <bsp_syscache.h>

#define SC_GID_CTRL_MAX 32
#define SC_GID_CTRL_ALIGN 0x1000

struct gid_ctrl_cfg_dt {
    unsigned int arhint;
    unsigned int awhint;
    unsigned int arcache;
    unsigned int awcache;
    unsigned int domain;
};

typedef union {
    struct {
        unsigned int awcache : 4;
        unsigned int arcache : 4;
        unsigned int domain : 2;
        unsigned int awhint : 4;
        unsigned int arhint : 4;
        unsigned int gid : 6;
        unsigned int partid : 2;
        unsigned int override_en : 6;
    } bits;
    unsigned int val;
} gid_cfg;

struct gid_ctrl_cfg_om {
    unsigned long long start_addr;
    unsigned long long end_addr;
    unsigned int size;
    unsigned int cfg;
    unsigned int enable;
};

struct gid_ctrl {
    char *regs_acpu;
    char *regs_peri;
    unsigned int init;
    spinlock_t lock;
    gid_cfg common_cfg;
    unsigned int gid_only;
    unsigned int entry_max_num;
    unsigned int entry_bit_map;
    unsigned int sc_mode_idx;
    struct gid_ctrl_cfg_om cfg_om[SC_GID_CTRL_MAX];
};

void sc_gid_ctrl_hal_addr_cfg(struct gid_ctrl *gid_ctx, unsigned int start_addr, unsigned int end_addr, unsigned int cfg, int idx);
void sc_gid_ctrl_hal_enabel(struct gid_ctrl *gid_ctx, unsigned int idx, unsigned int enable);

int sc_addr_config(unsigned int start_addr, unsigned int end_addr, int rpolicy, int wpolicy, int gid, int idx);
void sc_addr_enable(unsigned int idx, unsigned int enable);
struct gid_ctrl *sc_get_gid_ctx(void);
#endif
