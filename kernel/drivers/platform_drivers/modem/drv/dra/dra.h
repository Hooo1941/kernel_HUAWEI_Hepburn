/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _DRA_H_
#define _DRA_H_

#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#include <bsp_dra.h>
#include <bsp_print.h>
#include <bsp_net_om.h>
#include <linux/version.h>
#include "bsp_dt.h"

#define DRA_LEVELS_MAX_BIT 18
#define DRA_LEVELS_MAX_SIZE (1 << DRA_LEVELS_MAX_BIT)
#define DRA_FILL_MAX_IN_INT 8
#define DRA_ACPU_MID 0x41
#define DRA_NRCPU_MID 0x28
#define DRA_LRCPU_MID 0x3f
#define DRA_OPIPE_DN_BIT(X) BIT(X)
#define DRA_SKB_FROM_TCP_IP 0
#define DRA_SKB_FROM_NIC 2
#define DRA_SKB_FROM_OWN 1
#define DRA_CP_MEM 1
#define DRA_AP_MEM 0
#define THIS_MODU mod_dra
#define DRA_OWN 0x4D41414D
#define DRA_FREE 0x41414D55
#define DRA_SKB_RESERVE_SIZE (128)
#define DRA_PHY_ADDR_BITS 40
#define DRA_PHY_ADDR_MASK ((1ULL << DRA_PHY_ADDR_BITS) - 1)
#define DRA_DEBUG_MSG BIT(5)
#define DRA_DEBUG_INTR_REFILL BIT(6)
#define DRA_DEBUG_ALLOC_FREE_TIMESTAMP BIT(7)
#define DRA_DEBUG_BUGON BIT(8)
#define DRA_DEBUG_SYSTEM_ERR BIT(9)
#define DRA_DEBUG_TIMER_EN BIT(10)
#define DRA_DEBUG_RELEASE_IRQ_EN BIT(11)
#define DRA_DEBUG_PRINT_SLICE BIT(12)
#define MAX_PERF_DEBUG_CNT (1 << 10)
#define MAX_PERF_DEBUG_SIZE 2
#define MAX_PERF_DEBUG_TIME_IDX 1
#define MAX_PERF_DEBUG_CNT_IDX 0
#define DRA_DUMP_REG_NUM 2048
#define DRA_DUMP_SIZE (DRA_DUMP_REG_NUM * sizeof(unsigned int))
#define DRA_BACK_REGS_NUM 10
#define DRA_IPIPE_USING_CONT 21
#define DRA_RELEASE_THRSH_LP 128
#define DRA_FLS_SIZE 32

#define DRA_ALLOC_FIFO_SIZE 16

#define DRA_OPIPE_FREE_LEVELS 1
#define DRA_OPIPE_ALLOC_LEVELS 9
#define DRA_OPIPE_MAX (DRA_OPIPE_ALLOC_LEVELS + DRA_OPIPE_FREE_LEVELS)

#define DRA_OPIPE_STATUS_INIT 0x5a5a5a5a
#define DRA_INIT_OK (0x4D41415F)
#define DRA_ADDR_SIZE sizeof(dra_buf_t)
#define DRA_TABEL_START 0xF
#define DRA_BURST_LEN_BIT 4
#define DRA_BURST_LEN (1 << DRA_BURST_LEN_BIT)
#define DRA_OPIPE_USED_MAGIC 0xcccccccc
#define DRA_ALLOC_PARA_MAGIC 0xFFFF
#define DRA_CLKS_MAX_NUM 2
#define DRA_DBG_CNT_NUM 36

#define dra_err(fmt, ...)                             \
    do {                                              \
        bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
    } while (0)

#define DRA_EVENT_RESETTING_BIT 0
#define DRA_EVENT_THREAD_STOPING_BIT 1
#define DRA_EVENT_THREAD_STOPED_BIT 2
#define DRA_EVENT_THREAD_RUNNING_BIT 3
#define DRA_EVENT_REPORT_BIT 4

#define DRA_EVENT_RESETTING BIT(DRA_EVENT_RESETTING_BIT)
#define DRA_EVENT_THREAD_STOPING BIT(DRA_EVENT_THREAD_STOPING_BIT)
#define DRA_EVENT_THREAD_STOPED BIT(DRA_EVENT_THREAD_STOPED_BIT)
#define DRA_EVENT_THREAD_RUNNING BIT(DRA_EVENT_THREAD_RUNNING_BIT)
#define DRA_EVENT_REPORT BIT(DRA_EVENT_REPORT_BIT)

struct dra_opipe_para {
    unsigned int size[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int base[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int depth[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int use_tab[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int disable_irq[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int from[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int tab_size[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int opipeidx[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int alloc[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int cpu[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int dbg_idx[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int cnt;
    unsigned int dthr_config;
};

struct dra_ipipe_para {
    unsigned int size[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int phy[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int depth[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int ipipeidx[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int alloc[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int cpu[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int dbg_idx[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int cnt;
};

struct dra_para_s {
    struct dra_opipe_para oparamter;
    struct dra_ipipe_para iparamter;
    unsigned int ddr_buf_depth[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int ddr_buf_dbg_idx[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int opipes_used[DRA_OPIPE_ALLOC_LEVELS];
    unsigned int rls_chn_idx;
    unsigned int rls_chn_depth;
    unsigned int rls_chn_dbg_idx;
    unsigned int opipe_cnt;
    unsigned int max_opipes;
    unsigned int opipe_use_aximem;
    unsigned int release_depth;
    unsigned int protect_thresh;
    unsigned int sc_mode;
};

struct dra_fifo {
    unsigned long long local_buf[DRA_BURST_LEN];
    unsigned int write;
    unsigned int read;
    unsigned int depth;

    unsigned int empty;
    unsigned int idx;
    unsigned int dbg_idx;
    unsigned int uthrl;
    unsigned int dthrl;
    unsigned int free;
    unsigned int size;
    unsigned long long base;
    unsigned long long phy;
    unsigned long long update_write_addr;
    unsigned long long update_write_virt;
    unsigned long long update_read_addr;
    unsigned long long update_read_virt;
};

struct dra_ipipe_alloc_debug {
    unsigned int alloc;
    unsigned int size_level_err;
    unsigned int ipipe_empty;
};

struct dra_free_debug {
    unsigned int free[DRA_OPIPE_MAX];
    unsigned int free_fail;
};

struct dra_ipipe {
    struct dra_fifo fifo;
    struct dra_ipipe_alloc_debug debug;
    unsigned long long wptr_update_addr;
    unsigned int level;
    unsigned int idx;
    unsigned int user;
    spinlock_t lock;
};

struct dra_perf_log {
    unsigned int fifo[MAX_PERF_DEBUG_SIZE][MAX_PERF_DEBUG_CNT];
    unsigned int idx;
};

struct dra_perf_debug {
    struct dra_perf_log intr_perf_debug[DRA_OPIPE_ALLOC_LEVELS];
    struct dra_perf_log hf_perf_debug[DRA_OPIPE_ALLOC_LEVELS];
    struct dra_perf_log release_perf_debug;
};

struct dra_head_rom {
    unsigned long long check;
    unsigned long long dra_addr;
};

struct dra_tab {
    struct list_head list;
    unsigned int magic;
    unsigned int idx;
    unsigned long long dra_addr;
    void *manager;
};

struct dra_map {
    struct list_head free_list;
    struct list_head busy_list;
    struct list_head reclaim_list;

    struct dra_tab *tab;
    struct dra_tab *ext_tab;
    unsigned int tab_max_size;
    unsigned int free;
    unsigned int busy;
    unsigned int reclaim;
    unsigned int curr;
    unsigned int total;
    unsigned int empty;
};
struct dra_debug {
    unsigned int free_fail;
    unsigned int unmap_invalid_level;
    unsigned int map_invalid_level;
    unsigned int unmap_invalid_idx;
    unsigned int map_sec_levels;
    unsigned int safe_check_err;
    unsigned int rls_full;
    unsigned int maybe_dobulefree_or_modified;
    unsigned int sche_cnt;
    unsigned int refill_succ;
    unsigned int reclaim_succ;
    unsigned int work_resetting;
    unsigned int work_start;
    unsigned int work_end;
    unsigned int mem_protect;
};

struct dra_opipe_debug {
    unsigned int alloc_success;
    unsigned int skb_build_fail;
    unsigned int alloc_fail;
    unsigned int alloc_atomic;
    unsigned int alloc_kernel;
    unsigned int skb_own_free;
    unsigned int skb_tcp_ip_free;
    unsigned int skb_reclaim_free;
    unsigned int map_fail_skb_free;
    unsigned int free;
    unsigned int max_fill_cnt;
    unsigned int to_skb_fail;

    unsigned int free_list_empty;
    unsigned int map_own_success;
    unsigned int map_own_fail;
    unsigned int alloc_map_own_fail;
    unsigned int unmap_own;
    unsigned int unmap_cache_flush;
    unsigned int unmap_own_fail;
    unsigned int map_tcp_ip_ok;
    unsigned int unmap_test_tcp_ip;
    unsigned int map_tcp_ip_fail;
    unsigned int unmap_tcp_ip;
    unsigned int opipe_full;
    unsigned int opipe_dn;
    unsigned int opipe_up;
    unsigned int ipipe_up;
};

struct dra_opipe {
    spinlock_t lock;
    struct dra_map map;
    struct dra_fifo fifo;
    unsigned int rls_full_cnt;
    unsigned int delay_free_cnt;
    unsigned int reclaim_cnt;
    unsigned int reclaim_depth;
    unsigned int last_wptr;
    unsigned int last_rptr;
    unsigned int last_reclaim_depth;
    unsigned int timer_free_cnt;
    unsigned int equal_times;
    unsigned int busy_depth;
    unsigned int max_alloc_cnt;
    unsigned int min_alloc_cnt;
    u32 status;
    unsigned int level;
    unsigned int from;
    unsigned int used;
    unsigned int use_tab;
};

struct dra_rls_ch {
    struct dra_fifo fifo;
    spinlock_t lock;
    unsigned int level;
    unsigned int idx;
};

struct dra_ddr_buf {
    struct dra_fifo fifo;
    unsigned int level;
    unsigned int from;
};

struct dra_release_poll {
    struct dra_fifo fifo;
    dra_buf_t local_release[2 * DRA_BURST_LEN]; // buffer len is 2 DRA_BURST_LEN
    unsigned int max_free_cnt;
    unsigned int min_free_cnt;
    unsigned int mcp_fail;
};

struct dra_para {
    unsigned int size;
    unsigned int base;
    unsigned int depth;
    unsigned int ipipeidx;
    unsigned int alloc;
    unsigned int cpu;
};

struct dra_back_regs_cfg {
    char *start;
    char *end;
    unsigned int num;
    unsigned int back_addr;
};

struct dra_allc_para {
    char *name;
    unsigned int ipipe_id;
    unsigned int idx;
    unsigned int level;
};

struct dra_rls_para {
    char *name;
    unsigned int rls_id;
    unsigned int idx;
    unsigned int level;
    unsigned int dbg_idx;
};

#define DRA_MAX_LEVEL 7

struct dra_opipe_debug_info {
    unsigned int skb_alloc_success;
    unsigned int skb_own_free;
    unsigned int map_own_success;
    unsigned int map_own_fail;
    unsigned int alloc_map_own_fail;
    unsigned int free_list_empty;
    unsigned int skb_alloc_fail;
    unsigned int unmap_own;
    unsigned int map_fail_skb_free;
    unsigned int map_tcp_ip_ok;
    unsigned int map_tcp_ip_fail;
    unsigned int unmap_tcp_ip;
    unsigned int unmap_test_tcp_ip;
    unsigned int delay_free_cnt;
    unsigned int reclaim_cnt;
    unsigned int max_alloc_cnt;
    unsigned int min_alloc_cnt;
    unsigned int free;
    unsigned int wptr;
    unsigned int rptr;
    unsigned int skb_reclaim_free;
    unsigned int reclaim_depth;
};

struct dra_ipipe_debug_info {
    unsigned int ch_en;
};

#define DRA_PER_IPIPE_POINT 6

struct dra_debug_info {
    unsigned int rsl_full;
    unsigned int cur_ts;
    unsigned int int_ts;
    unsigned int max_sche_time;
    unsigned int min_sche_time;
    unsigned int fill_cnt[DRA_MAX_LEVEL];
    unsigned int irq_mask;
    unsigned int irq_raw;
    unsigned int work_resetting;
    unsigned int work_start;
    unsigned int work_end;
    unsigned int refill_succ;
    unsigned int sche_cnt;
    unsigned int mem_protect;
    unsigned int map_invalid_level;
    unsigned int unmap_invalid_level;
    unsigned int timer_en;
    unsigned int fifo_timer_en;
    unsigned int rsl_rptr;
    unsigned int rsl_wptr;
    struct dra_opipe_debug_info opipe_dbg[DRA_OPIPE_LEVEL_INVALID];
    unsigned int dbg_cnt[DRA_DBG_CNT_NUM];
};

struct dra {
    struct dra_para_s *para;
    device_node_s *np;
    device_node_s *cust_np;
    spinlock_t lock;
    spinlock_t irq_lock;
    void __iomem *regs;
    unsigned int *dump_base;
    unsigned int back_regs_num;
    resource_size_t phy_regs;
    unsigned int version;
    struct device *dev;
    unsigned int int_mask;
    unsigned long irq_flags;
    struct task_struct *thread;
    wait_queue_head_t wqueue;
    unsigned int irq_time;
    int irq;
    unsigned int wake_irq;
    struct dra_debug status;
    struct dra_opipe_debug opipe_status[DRA_OPIPE_MAX];
    struct dra_back_regs_cfg back[DRA_BACK_REGS_NUM];
    struct wakeup_source *wake_lock;
    unsigned int sche_cnt;
    unsigned int fifo_empty;
    unsigned int need_schedule;
    struct timer_list timer;
    unsigned long timeout_jiffies;
    unsigned int irq_status;
    struct dra_opipe opipes[DRA_OPIPE_MAX];
    struct dra_rls_ch rls_opipes;
    struct dra_ddr_buf ddr_buf[DRA_OPIPE_MAX];
    struct dra_ipipe *cpu_ipipes;
    struct dra_ipipe dra_lp_ipipe;
    unsigned int max_cpu_ipipes;
    unsigned int max_opipes;
    unsigned long long use;
    unsigned int cpu_using_ipipe[DRA_OPIPE_MAX];
    struct dra_allc_para *allc_para;
    unsigned int allc_para_size;
    struct dra_rls_para *rls_para;
    unsigned int rls_para_size;
    struct dra_release_poll release;
    struct net_om_info report_info;
    struct dra_debug_info hds_debug_info;
    unsigned int dbg;
    unsigned long long ipipe_write_ptr_phy;
    unsigned int fifo_timer_en;
    unsigned int opipe_use_aximem;
    unsigned int event;
    unsigned int timer_cnt;
    unsigned int deinit_cnt;
    unsigned int reinit_cnt;
    unsigned int protect_thresh;
    unsigned long long virt_base;
    unsigned long long dma_base;
    unsigned int dma_size;
    unsigned int offset;
};
extern struct dra *g_dra_ctx;

unsigned long long dra_map_single(struct dra *dra, unsigned long long phy, void *manger, int from, unsigned int level);

struct sk_buff *dra_unmap_skb(struct dra *dra, dra_buf_t buf, unsigned long long *orig);
unsigned long long dra_map_tcpip_skb(struct dra *dra, struct sk_buff *skb, unsigned int reserve);
struct sk_buff *dra_unmap_tcpip_skb(struct dra *dra, dra_buf_t buf);

void dra_ipipe_free(struct dra *dra, unsigned long long ptr);

unsigned long long dra_ipipe_alloc(unsigned int size);

int dra_tab_is_empty(struct dra *dra, unsigned int level);

struct sk_buff *dra_to_skb(struct dra *dra, unsigned long long addr, unsigned long long *orig);
unsigned int dra_set_adqbase(struct dra *dra, unsigned long long adqbase, unsigned int update_ptr, unsigned int depth,
    unsigned int id);
unsigned int dra_set_rlsbase(struct dra *dra, unsigned long long rlsbase, unsigned int update_ptr, unsigned int depth,
    unsigned int id);
static inline int dra_len_to_levels(unsigned int size)
{
    if (!size || size > DRA_OPIPE_LEVEL_1_SIZE) {
        return DRA_OPIPE_LEVEL_INVALID;
    }
    return DRA_OPIPE_LEVEL_1;
}

static inline int dra_list_is_full(struct dra *dra, unsigned int level)
{
    return (dra->opipes[level].busy_depth + dra->opipes[level].reclaim_depth) >= dra->para->oparamter.tab_size[level];
}

int dra_suspend(struct device *dev);
int dra_resume(struct device *dev);
int dra_core_probe(struct dra *dra);
void dra_bigpool_timer(struct timer_list *t);
irqreturn_t dra_interrupt(int irq, void *dev_id);
void dra_free_buf(struct dra *dra, dra_buf_t buf);
int dra_enable_alloc(struct dra *dra, unsigned int idx, int enable);
int dra_enable_rls(struct dra *dra, unsigned int idx, int enable);

static inline int dra_opipe_is_enable(struct dra *dra, unsigned int level)
{
    return (level < DRA_OPIPE_MAX) && (dra->opipes[level].used == DRA_OPIPE_USED_MAGIC);
}

static inline unsigned int dra_get_busy_num(unsigned int w, unsigned int r, unsigned int depth)
{
    return (w >= r) ? (w - r) : (depth - (r - w));
}

static inline int dra_check_addr_crc(unsigned long long addr)
{
    unsigned int bitcnt;

    bitcnt = __sw_hweight64(addr);
    if (bitcnt & 1) {
        dra_err("crc check fail,dra 0x%llx\n", addr);
        return -1;
    }
    return 0;
}

#endif
