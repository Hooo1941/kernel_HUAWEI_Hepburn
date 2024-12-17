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

#include <linux/netdevice.h>
#include <linux/kthread.h>
#include <uapi/linux/sched/types.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <securec.h>
#include <bsp_print.h>
#include <bsp_diag.h>
#include <bsp_dump.h>
#include <bsp_om_enum.h>
#include <bsp_syscache.h>
#include "dra.h"
#include <bsp_reset.h>
#include <mdrv_memory_layout.h>
#include "dra_hal.h"

#define DRA_MEM_LINE_DEAULT 100
#define DRA_CURRETN_AVAIL_MEM ((unsigned int)si_mem_available() << (PAGE_SHIFT - 10))
#define DRA_TIMER_EXPIRES (msecs_to_jiffies(1000))
#define DRA_TWO_BURST_RESERVE (2 * DRA_BURST_LEN)
#define DRA_HEAD_ROM_SIZE (sizeof(struct dra_head_rom))

#define DRA_OPIPE2_LP_IPIPE 16
#define DRA_LP_IPIPE_LEN (DRA_BURST_LEN - 1)
#define DRA_IPIPE_WPTR_PHY_SIZE 500
#define DRA_LP_IPIPE_WPTR_PHY_SIZE 100
#define DRA_TIMER_AP_OPIPE_EN_VALUE 0x7
#define DRA_RECLAIM_MAX_DIFF 0x40
#define DRA_OPIPE_NAME_MAX 64
#define DRA_BYTE_ALIGN(addr) ALIGN(((u32)(addr)), DRA_SKB_RESERVE_SIZE)
#define DRA_ADDR_MASK (~(DRA_OPIPE_LEVEL_1_BUFF_SIZE - 1))
#define DRA_THREAD_PRIORITY (MAX_RT_PRIO - 1)
#define DRA_FIFO_MIN_DEPTH 0x20
#define DRA_SCHED_GAP 10

#define DRA_MAX_SKB_SIZE (max(sizeof(struct sk_buff), sizeof(struct skb_shared_info)))
#define DRA_SKB_OFFSET(X) \
    ((X) - DRA_MAX_SKB_SIZE)

struct dra *g_dra_ctx;
EXPORT_SYMBOL(g_dra_ctx);

unsigned int g_dra_once_flag = 0;
#define dra_err_once(fmt, ...)           \
    do {                                 \
        if (!g_dra_once_flag) {          \
            bsp_err("Warning once \n");  \
            bsp_err(fmt, ##__VA_ARGS__); \
            g_dra_once_flag = 1;         \
        }                                \
    } while (0)

#define dra_dbg(fmt, ...)                                 \
    do {                                                  \
        if (dra->dbg & DRA_DEBUG_MSG) {                   \
            bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
        }                                                 \
    } while (0)

#define dra_print_slice(fmt, ...)                          \
    do {                                                   \
        if (dra->dbg & DRA_DEBUG_PRINT_SLICE) {            \
            bsp_info("<%s>" fmt, __func__, ##__VA_ARGS__); \
        }                                                  \
    } while (0)

#define dra_bug_on()                                                 \
    do {                                                             \
        dump_stack();                                                \
        if (dra->dbg & DRA_DEBUG_SYSTEM_ERR) {                       \
            system_error(DRV_ERRNO_DRA_ADDR_CHECK_FAIL, 0, 0, 0, 0); \
        }                                                            \
        if (dra->dbg & DRA_DEBUG_BUGON) {                            \
            BUG_ON(1);                                               \
        }                                                            \
    } while (0)

/* dra alloc buff size */
static const int g_level_size[DRA_OPIPE_MAX] = {
    DRA_OPIPE_LEVEL_0_BUFF_SIZE, DRA_OPIPE_LEVEL_1_BUFF_SIZE, DRA_OPIPE_LEVEL_2_SIZE,
    DRA_OPIPE_LEVEL_3_BUFF_SIZE, DRA_OPIPE_LEVEL_INVALID
};

/* data size in buff */
static const int g_level_data_size[DRA_OPIPE_MAX] = {
    DRA_OPIPE_LEVEL_0_SIZE, DRA_OPIPE_LEVEL_1_SIZE, DRA_OPIPE_LEVEL_2_SIZE, DRA_OPIPE_LEVEL_3_SIZE,
    DRA_OPIPE_LEVEL_INVALID
};

static const int g_dra_report_level[] = {
    DRA_OPIPE_LEVEL_0, DRA_OPIPE_LEVEL_1, DRA_OPIPE_LEVEL_2, DRA_OPIPE_LEVEL_3, DRA_OPIPE_LEVEL_7
};

int dra_mdmreset_cb(drv_reset_cb_moment_e eparam, int userdata);

unsigned long long dra_map_tcpip_skb(struct dra *dra, struct sk_buff *skb, unsigned int reserve)
{
    unsigned long long reversed_addr;
    unsigned long long phy;
    unsigned int level;

    if (dra->event & DRA_EVENT_RESETTING) {
        return 0;
    }

    level = DRA_OPIPE_LEVEL_0; // tcpip包独立通道
    if (dra_tab_is_empty(dra, level)) {
        dra->opipe_status[level].map_tcp_ip_fail++;
        return 0;
    }
    /* draaddr point to iphead */
    phy = bsp_sc_dma_map_single(dra->dev, skb->data, skb->len, DMA_TO_DEVICE);

    reversed_addr = dra_map_single(dra, phy + reserve, skb, DRA_SKB_FROM_TCP_IP, level);
    if (!reversed_addr) {
        dra->opipe_status[level].map_tcp_ip_fail++;
        return 0;
    }
    dra->opipe_status[level].map_tcp_ip_ok++;

    return reversed_addr;
}

struct sk_buff *dra_unmap_tcpip_skb(struct dra *dra, dra_buf_t buf)
{
    struct dra_opipe *opipe = NULL;
    struct dra_map *map = NULL;
    struct dra_tab *tab = NULL;
    struct sk_buff *skb = NULL;
    unsigned int level = buf.bits.level;
    unsigned int idx = buf.bits.idx;
    unsigned long flags = 0;

    opipe = &dra->opipes[level];
    map = &opipe->map;

    if (unlikely(buf.bits.idx < DRA_TABEL_START || buf.bits.idx > (map->tab_max_size - 1))) {
        return NULL;
    }

    spin_lock_irqsave(&opipe->lock, flags);
    tab = &map->tab[idx];

    if (unlikely(tab->magic != DRA_OWN)) {
        spin_unlock_irqrestore(&opipe->lock, flags);
        bsp_err("buf already free:0x%llx", buf.addr);
        return NULL;
    }

    skb = (struct sk_buff *)tab->manager;
    tab->manager = (void *)(uintptr_t)(buf.addr + 1);
    tab->magic = DRA_FREE;
    tab->dra_addr = (unsigned long long)((uintptr_t)skb) + 1;

    list_move_tail(&tab->list, &map->free_list);
    dra->opipes[level].busy_depth--;
    map->busy--;
    map->free++;
    map->curr = tab->idx;
    dra->opipe_status[level].unmap_tcp_ip++;
    spin_unlock_irqrestore(&opipe->lock, flags);

    if (skb->len) {
        dma_unmap_single(dra->dev, (dma_addr_t)virt_to_phys(skb->data), skb->len, DMA_TO_DEVICE);
    } else {
        dra_err("skb->len is zero\n");
    }

    return skb;
}

void dra_clear_tcpip(struct dra *dra, int level)
{
    struct dra_map *map = NULL;
    struct dra_tab *tab = NULL;
    struct dra_tab *tmp = NULL;
    struct dra_opipe *opipe = NULL;
    struct sk_buff *skb = NULL;
    unsigned long flags;
    LIST_HEAD(local_list);

    opipe = &dra->opipes[level];
    map = &opipe->map;

    spin_lock_irqsave(&opipe->lock, flags);
    list_splice_init(&map->busy_list, &local_list);
    spin_unlock_irqrestore(&opipe->lock, flags);

    list_for_each_entry_safe(tab, tmp, &local_list, list) {
        skb = dra_unmap_tcpip_skb(dra, (dra_buf_t)tab->dra_addr);
        if (skb != NULL) {
            dev_kfree_skb_any(skb);
        }
    }
}

static inline char *dra_check_own_buf(struct dra *dra, dra_buf_t buf)
{
    char *buf_virt = NULL;

    if (unlikely(buf.bits.idx != DRA_SKB_OFFSET(g_level_size[buf.bits.level]))) {
        dra_err("invalid  idx dra 0x%llx\n", buf.addr);
        dra->status.unmap_invalid_idx++;
        return NULL;
    }

    buf_virt = (char *)phys_to_virt((buf.bits.phyaddr & DRA_ADDR_MASK));
    return buf_virt;
}

static struct dra_tab *dra_get_tab(struct dra *dra, dra_buf_t buf)
{
    struct dra_map *map = NULL;
    unsigned int level = buf.bits.level;
    struct dra_tab *tab = NULL;
    char *buf_virt = NULL;

    if (unlikely(buf.bits.level != DRA_OPIPE_LEVEL_0 && buf.bits.level != DRA_OPIPE_LEVEL_1
        && buf.bits.level != DRA_OPIPE_LEVEL_3)) {
        dra_err("invalid level dra 0x%llx\n", buf.addr);
        dra->status.unmap_invalid_level++;
        dra_bug_on();
        return NULL;
    }

    if (dra->opipes[level].use_tab) {
        map = &dra->opipes[level].map;
        if (unlikely(buf.bits.idx < DRA_TABEL_START || buf.bits.idx > (map->tab_max_size - 1))) {
            goto fail;
        }
        tab =  &map->tab[buf.bits.idx];
    } else {
        buf_virt = dra_check_own_buf(dra, buf);
        if (unlikely(buf_virt == NULL)) {
            goto fail;
        }

        tab = (struct dra_tab *)(buf_virt + DRA_SKB_RESERVE_SIZE + g_level_data_size[level]);
    }

    if (unlikely(tab->magic != DRA_OWN)) {
        dra_err("magic %x, dra 0x%llx, olddra 0x%llx\n", tab->magic, buf.addr, tab->dra_addr);
        dra->status.maybe_dobulefree_or_modified++;
        dra_bug_on();
        return NULL;
    }

    return tab;

fail:
    dra->status.unmap_invalid_idx++;
    dra_err("invalid idx dra 0x%llx\n", buf.addr);
    dra_bug_on();
    return NULL;
}

void dra_hds_transreport(void)
{
    int i, level;
    struct dra *dra = g_dra_ctx;
    struct dra_debug_info *dbg_info = &dra->hds_debug_info;

    set_bit(DRA_EVENT_REPORT_BIT, (void *)&dra->event);
    if (!test_bit(DRA_EVENT_RESETTING_BIT, (void *)&dra->event)) {
        dra_hds_transreport_hal(dra, dbg_info);
    }
    clear_bit(DRA_EVENT_REPORT_BIT, (void *)&dra->event);
    wake_up(&dra->wqueue);

    dbg_info->work_resetting = dra->status.work_resetting;
    dbg_info->work_start = dra->status.work_start;
    dbg_info->work_end = dra->status.work_end;
    dbg_info->refill_succ = dra->status.refill_succ;
    dbg_info->sche_cnt = dra->status.sche_cnt;
    dbg_info->mem_protect = dra->status.mem_protect;
    dbg_info->map_invalid_level = dra->status.map_invalid_level;
    dbg_info->unmap_invalid_level = dra->status.unmap_invalid_level;
    dbg_info->fifo_timer_en = dra->fifo_timer_en;

    for (i = 0; i < sizeof(g_dra_report_level) / sizeof(int); i++) {
        level = g_dra_report_level[i];
        dbg_info->opipe_dbg[level].skb_alloc_success = dra->opipe_status[level].alloc_success;
        dbg_info->opipe_dbg[level].skb_own_free = dra->opipe_status[level].skb_own_free;
        dbg_info->opipe_dbg[level].map_own_success = dra->opipe_status[level].map_own_success;
        dbg_info->opipe_dbg[level].map_own_fail = dra->opipe_status[level].map_own_fail;
        dbg_info->opipe_dbg[level].alloc_map_own_fail = dra->opipe_status[level].alloc_map_own_fail;
        dbg_info->opipe_dbg[level].free_list_empty = dra->opipe_status[level].free_list_empty;
        dbg_info->opipe_dbg[level].skb_alloc_fail = dra->opipe_status[level].alloc_fail;
        dbg_info->opipe_dbg[level].unmap_own = dra->opipe_status[level].unmap_own;
        dbg_info->opipe_dbg[level].map_fail_skb_free = dra->opipe_status[level].map_fail_skb_free;
        if (level == DRA_OPIPE_LEVEL_1) {
            dbg_info->opipe_dbg[level].map_tcp_ip_ok = dra->opipes[level].busy_depth;
        } else {
            dbg_info->opipe_dbg[level].map_tcp_ip_ok = dra->opipe_status[level].map_tcp_ip_ok;
        }
        dbg_info->opipe_dbg[level].map_tcp_ip_fail = dra->opipe_status[level].map_tcp_ip_fail;
        dbg_info->opipe_dbg[level].unmap_tcp_ip = dra->opipe_status[level].unmap_tcp_ip;
        dbg_info->opipe_dbg[level].unmap_test_tcp_ip = dra->opipe_status[level].unmap_test_tcp_ip;
        dbg_info->opipe_dbg[level].delay_free_cnt = dra->opipes[level].delay_free_cnt;
        dbg_info->opipe_dbg[level].reclaim_cnt = dra->opipes[level].reclaim_cnt;
        dbg_info->opipe_dbg[level].max_alloc_cnt = dra->opipes[level].max_alloc_cnt;
        dbg_info->opipe_dbg[level].min_alloc_cnt = dra->opipes[level].min_alloc_cnt;
        dbg_info->opipe_dbg[level].free = dra->opipe_status[level].free;
        dbg_info->opipe_dbg[level].skb_reclaim_free = dra->opipe_status[level].skb_reclaim_free;
        dbg_info->opipe_dbg[level].reclaim_depth = dra->opipes[level].reclaim_depth;
    }
}

void dra_trans_report_register(void)
{
    struct dra *dra = g_dra_ctx;
    struct net_om_info *info = &dra->report_info;
    int ret;

    info->ul_msg_id = NET_OM_MSGID_DRA;
    info->data = (void *)&dra->hds_debug_info;
    info->size = sizeof(dra->hds_debug_info);
    info->func_cb = dra_hds_transreport;
    INIT_LIST_HEAD(&info->list);

    ret = bsp_net_report_register(info);
    if (ret) {
        dra_err("trans report register fail\n");
    }
}

int dra_sys_mem_protect(struct dra *dra)
{
    long avail;
    if (dra->protect_thresh) {
        avail = DRA_CURRETN_AVAIL_MEM;
        if (avail < dra->protect_thresh) {
            dra->status.mem_protect++;
            return 0;
        }
    }
    return 1;
}

static inline struct dra_allc_para *dra_get_alloc_para(struct dra *dra, enum dra_ipipe_idx ipipe_id)
{
    unsigned int i;
    for (i = 0; i < dra->allc_para_size; i++) {
        if (dra->allc_para[i].ipipe_id == ipipe_id && dra->allc_para[i].idx != DRA_ALLOC_PARA_MAGIC) {
            return &dra->allc_para[i];
        }
    }
    bsp_err("dra_get_ipipe_level failed:%d\n", ipipe_id);
    return NULL;
}

static inline struct dra_rls_para *dra_get_rls_para(struct dra *dra, enum dra_rls_idx rls_id)
{
    unsigned int i;
    for (i = 0; i < dra->rls_para_size; i++) {
        if (dra->rls_para[i].rls_id == rls_id && dra->rls_para[i].idx != DRA_ALLOC_PARA_MAGIC) {
            return &dra->rls_para[i];
        }
    }
    bsp_err("dra_get_rls_level failed:%d\n", rls_id);
    return NULL;
}

unsigned int dra_set_adqbase(struct dra *dra, unsigned long long adqbase, unsigned int update_ptr, unsigned int depth,
    unsigned int id)
{
    struct dra_allc_para *alloc_para = NULL;

    alloc_para = dra_get_alloc_para(dra, id);
    if (alloc_para == NULL) {
        return 0;
    }

    if (!dra_opipe_is_enable(dra, alloc_para->level)) {
        return 0;
    }

    return dra_hal_set_adq(dra, alloc_para->level, adqbase, update_ptr, depth, alloc_para->idx);
}

unsigned int dra_set_rlsbase(struct dra *dra, unsigned long long rlsbase, unsigned int update_ptr, unsigned int depth,
    unsigned int id)
{
    struct dra_rls_para *rls_para = NULL;

    rls_para = dra_get_rls_para(dra, id);
    if (rls_para == NULL) {
        return 0;
    }

    if (!dra_opipe_is_enable(dra, rls_para->level)) {
        return 0;
    }

    return dra_hal_set_rls(dra, rlsbase, update_ptr, depth, rls_para);
}

int dra_enable_alloc(struct dra *dra, unsigned int idx, int enable)
{
    struct dra_allc_para *alloc_para = NULL;

    alloc_para = dra_get_alloc_para(dra, idx);
    if (alloc_para == NULL) {
        return -1;
    }

    return dra_enable_alloc_ch(dra, alloc_para->idx, enable);
}

int dra_enable_rls(struct dra *dra, unsigned int idx, int enable)
{
    struct dra_rls_para *rls_para = NULL;

    rls_para = dra_get_rls_para(dra, idx);
    if (rls_para == NULL) {
        return -1;
    }

    return dra_enable_rls_ch(dra, rls_para->idx, enable);
}

int dra_rls_chn_init(struct dra *dra, unsigned int level)
{
    struct dra_fifo *fifo = NULL;

    spin_lock_init(&dra->rls_opipes.lock);
    fifo = &dra->rls_opipes.fifo;
    fifo->depth = dra->para->rls_chn_depth;
    dra->rls_opipes.idx = dra->para->rls_chn_idx;

    fifo->base = dra->virt_base + dra->offset;
    fifo->phy = dra->dma_base + dra->offset;
    dra->offset += fifo->depth * sizeof(dra_buf_t);

    fifo->update_read_virt = dra->virt_base + dra->offset;
    fifo->update_read_addr = dra->dma_base + dra->offset;
    dra->offset += sizeof(unsigned long long);

    fifo->dbg_idx = dra->para->rls_chn_dbg_idx;
    *(unsigned int *)(uintptr_t)fifo->update_read_virt = 0;
    dra_rls_fifo_init(dra, &dra->rls_opipes, level);

    return 0;
}

static int dra_cpu_ipipe_config(struct dra *dra, unsigned int i)
{
    unsigned int idx;
    struct dra_fifo *fifo = NULL;
    idx = dra->para->iparamter.ipipeidx[i];

    if (!dra->para->iparamter.depth[i]) {
        dra_err("ipipe %u depth is zero\n", idx);
        return -1;
    }
    spin_lock_init(&dra->cpu_ipipes[i].lock);
    dra->cpu_ipipes[i].idx = idx;
    dra->cpu_ipipes[i].level = DRA_OPIPE_LEVEL_1;
    if (DRA_OPIPE_LEVEL_INVALID != g_level_size[dra->cpu_ipipes[i].level]) {
        dra_err("ipipe %u,level %u,using\n", idx, dra->cpu_ipipes[i].level);
    }
    if (dra->cpu_using_ipipe[dra->cpu_ipipes[i].level] == dra->max_cpu_ipipes) {
        dra->cpu_using_ipipe[dra->cpu_ipipes[i].level] = i;
    }

    fifo = &dra->cpu_ipipes[i].fifo;
    fifo->depth = dra->para->iparamter.depth[i];

    fifo->base = dra->virt_base + dra->offset;
    fifo->phy = dra->dma_base + dra->offset;
    dra->offset += fifo->depth * sizeof(dra_buf_t);

    fifo->update_write_virt = dra->virt_base + dra->offset;
    fifo->update_write_addr = dra->dma_base + dra->offset;
    dra->offset += sizeof(unsigned long long);

    fifo->empty = 1;
    fifo->size = g_level_size[dra->cpu_ipipes[i].level];
    fifo->uthrl = fifo->depth * 3 / 4; // set uthrl as 3 / 4 depth
    fifo->dthrl = fifo->depth / 2; // set dthrl as 1 / 2 depth
    fifo->dbg_idx = dra->para->iparamter.dbg_idx[i];
    dra_ipipe_init(dra, &dra->cpu_ipipes[i], idx);
    return 0;
}

static int dra_cpu_ipipe_init(struct dra *dra)
{
    unsigned int i;

    if (!dra->para->iparamter.cnt) {
        dra_err("dra->para->iparamter.cnt = 0\n");
        return 0;
    }
    dra->max_cpu_ipipes = dra->para->iparamter.cnt;
    dra->cpu_ipipes = (struct dra_ipipe *)kzalloc(dra->max_cpu_ipipes * sizeof(struct dra_ipipe), GFP_KERNEL);
    if (dra->cpu_ipipes == NULL) {
        dra_err("cpu_ipipes alloc\n ");
        return 0;
    }

    for (i = 0; i < DRA_OPIPE_ALLOC_LEVELS; i++) {
        dra->cpu_using_ipipe[i] = dra->max_cpu_ipipes;
    }
    for (i = 0; i < dra->max_cpu_ipipes; i++) {
        dra_cpu_ipipe_config(dra, i);
    }

    return 0;
}

static unsigned int dra_local_fifo_input(struct dra *dra, struct dra_ipipe *ipipe)
{
    struct dra_fifo *fifo = &ipipe->fifo;
    unsigned int busy_num;
    int ret;

    fifo->write = *(unsigned int *)(uintptr_t)fifo->update_write_virt;
    busy_num = dra_get_busy_num(fifo->write, fifo->read, fifo->depth);
    if (busy_num < DRA_BURST_LEN) {
        ipipe->debug.ipipe_empty++;
        return 0;
    }
    ret = memcpy_s((void *)fifo->local_buf, sizeof(fifo->local_buf),
        (dra_buf_t *)(uintptr_t)fifo->base + fifo->read, DRA_BURST_LEN * sizeof(dra_buf_t));
    if (ret) {
        bsp_err("memcpy_s failed\n");
        return 0;
    }

    fifo->read += DRA_BURST_LEN;
    if (fifo->read >= fifo->depth) {
        fifo->read = 0;
    }
    dra_ipipe_read_update(dra, fifo->read, ipipe->idx);

    return DRA_BURST_LEN;
}

static unsigned long long dra_ipipe_get_buf(struct dra *dra, struct dra_ipipe *ipipe)
{
    unsigned long flags;
    unsigned long long buf = 0;
    struct dra_fifo *fifo = &ipipe->fifo;
    int ret;

    spin_lock_irqsave(&ipipe->lock, flags);
    if (fifo->idx == DRA_BURST_LEN || fifo->empty) {
        ret = dra_local_fifo_input(dra, ipipe);
        if (ret == DRA_BURST_LEN) {
            fifo->empty = 0;
        } else {
            fifo->empty = 1;
        }
        fifo->idx = 0;
    }

    if (!fifo->empty) {
        buf = fifo->local_buf[fifo->idx];
        fifo->idx++;
        ipipe->debug.alloc++;
    }

    spin_unlock_irqrestore(&ipipe->lock, flags);
    return buf;
}

unsigned long long dra_ipipe_alloc(unsigned int size)
{
    unsigned int level;
    unsigned int cpu_ipipe_idx;
    dra_buf_t buf;
    struct dra_ipipe *ipipe = NULL;
    struct dra *dra = g_dra_ctx;

    level = dra_len_to_levels(size);
    if (level == DRA_OPIPE_LEVEL_INVALID) {
        return 0;
    }

    cpu_ipipe_idx = dra->cpu_using_ipipe[level];
    if (cpu_ipipe_idx >= dra->max_cpu_ipipes) {
        return 0;
    }

    ipipe = dra->cpu_ipipes + cpu_ipipe_idx;
    if (!ipipe->fifo.update_write_virt || !ipipe->fifo.base) {
        return 0;
    }

    buf.addr = dra_ipipe_get_buf(dra, ipipe);

    return buf.addr;
}

int dra_tab_is_empty(struct dra *dra, unsigned int level)
{
    struct dra_map *map = NULL;

    if (unlikely(!dra_opipe_is_enable(dra, level))) {
        dra_err("invalid level\n");
        dra->status.map_invalid_level++;
        return -1;
    }

    map = &dra->opipes[level].map;

    if (list_empty(&map->free_list)) {
        dra->opipe_status[level].free_list_empty++;
        return 1;
    }

    return 0;
}

static unsigned long long dra_create_desc(struct dra *dra, unsigned long long phyaddr, unsigned int idx,
    unsigned int level, unsigned int from)
{
    dra_buf_t buf;
    unsigned int bitcnt;

    buf.addr = 0;
    buf.bits.phyaddr = phyaddr;
    buf.bits.sc_mode = dra->para->sc_mode;
    buf.bits.idx = idx;
    buf.bits.from = from;
    buf.bits.level = level;
    buf.bits.crc = 0;
    bitcnt = __sw_hweight64(buf.addr);
    if (bitcnt & 1) {
        buf.bits.crc = 1;
    }

    return buf.addr;
}

unsigned long long dra_map_single(struct dra *dra, unsigned long long phy, void *manager, int from, unsigned int level)
{
    struct dra_map *map = NULL;
    struct dra_tab *tab = NULL;
    dra_buf_t buf;
    unsigned long flags;

    if (unlikely(!dra_opipe_is_enable(dra, level))) {
        dra_err("invalid level\n");
        dra->status.map_invalid_level++;
        return 0;
    }
    map = &dra->opipes[level].map;
    spin_lock_irqsave(&dra->opipes[level].lock, flags);
    if (unlikely(list_empty(&map->free_list))) {
        dra->opipe_status[level].free_list_empty++;
        spin_unlock_irqrestore(&dra->opipes[level].lock, flags);
        return 0;
    }

    tab = list_first_entry(&map->free_list, struct dra_tab, list);
    map->free--;
    tab->manager = manager;
    tab->magic = DRA_OWN;
    buf.addr = dra_create_desc(dra, phy, tab->idx, level, from);
    tab->dra_addr = buf.addr;
    map->busy++;
    list_move_tail(&tab->list, &map->busy_list);
    dra->opipes[level].busy_depth++;
    spin_unlock_irqrestore(&dra->opipes[level].lock, flags);

    return buf.addr;
}

static inline struct sk_buff *dra_build_skb(struct sk_buff *skb)
{
    struct sk_buff *skb_new = NULL;

    skb_new = build_skb((void *)skb->head, 0);
    if (skb_new == NULL) {
        return NULL;
    }
    skb_reserve(skb_new, DRA_SKB_RESERVE_SIZE);

    return skb_new;
}

struct sk_buff *dra_list_del(struct dra *dra, dra_buf_t buf, dra_buf_t *p)
{
    struct dra_opipe *opipe = NULL;
    struct sk_buff *skb = NULL;
    unsigned int level = buf.bits.level;
    unsigned long flags;
    struct dra_tab *tab = NULL;

    tab = dra_get_tab(dra, buf);
    if (unlikely(tab == NULL)) {
        dra->opipe_status[level].unmap_own_fail++;
        return NULL;
    }

    opipe = &dra->opipes[level];
    spin_lock_irqsave(&opipe->lock, flags);
    list_del(&tab->list);
    opipe->busy_depth--;
    tab->magic = DRA_FREE;
    spin_unlock_irqrestore(&opipe->lock, flags);

    p->addr = tab->dra_addr;
    skb = (struct sk_buff *)tab->manager;

    return skb;
}

static inline void dra_buf_cache_flush(struct dra *dra, dra_buf_t buf, unsigned int size)
{
    buf.addr = buf.addr & DRA_ADDR_MASK;
    bsp_sc_dma_unmap_single(dra->dev, buf.bits.phyaddr, size, DMA_FROM_DEVICE);
    dra->opipe_status[buf.bits.level].unmap_cache_flush++;
}

static inline void dra_recycle_mem(struct dra *dra, dra_buf_t buf)
{
    struct dra_opipe *opipe = NULL;
    struct dra_tab *tab = NULL;

    tab = dra_get_tab(dra, buf);
    if (tab == NULL) {
        return;
    }

    opipe = &dra->opipes[buf.bits.level];
    dra->opipe_status[buf.bits.level].free++;
    dra_input(dra, tab->dra_addr, opipe);
}

static struct sk_buff *__dra_unmap_skb(struct dra *dra, dra_buf_t buf, unsigned long long *orig_addr)
{
    struct sk_buff *skb = NULL;
    dra_buf_t true_addr;

    skb = dra_list_del(dra, buf, &true_addr);
    if (skb == NULL) {
        return NULL;
    }

    dra->opipe_status[true_addr.bits.level].unmap_own++;
    if (orig_addr != NULL) {
        *orig_addr = true_addr.addr;
    }
    return skb;
}

/* use for cpu buf */
struct sk_buff *dra_unmap_skb(struct dra *dra, dra_buf_t buf, unsigned long long *orig)
{
    struct sk_buff *skb = NULL;
    struct sk_buff *skb_new = NULL;
    unsigned int level = buf.bits.level;

    if (unlikely(level == DRA_OPIPE_LEVEL_3)) {
        dra_recycle_mem(dra, buf);
        bsp_info("used ap 9k\n");
        return NULL;
    }

    skb = __dra_unmap_skb(dra, buf, orig);
    if (unlikely(skb == NULL)) {
        return NULL;
    }

    skb_new = dra_build_skb(skb);
    if (unlikely(skb_new == NULL)) {
        bsp_info("build skb fail\n");
        kfree(skb->head);
        return NULL;
    }

    return skb_new;
}

unsigned long dra_alloc_page(struct dra *dra, struct dra_opipe *opipe, dra_buf_t *p_addr, unsigned int level)
{
    unsigned long page;
    u64 phy;
    u64 addr;

    if (dra_tab_is_empty(dra, level)) {
        dra->opipe_status[level].map_own_fail++;
        dra->int_mask &= ~(BIT(level));
        return 0;
    }

    page = __get_free_page(GFP_KERNEL);
    if (unlikely(!page)) {
        dra->opipe_status[level].alloc_fail++;
        dra->need_schedule |= BIT(level);
        return 0;
    }
    dra->opipe_status[level].alloc_success++;

    phy = virt_to_phys((void *)(uintptr_t)page);
    addr = dra_map_single(dra, phy, (void *)(uintptr_t)page, DRA_SKB_FROM_OWN, level);
    p_addr->addr = addr;
    if (unlikely(!addr)) {
        free_page(page);
        dra->opipe_status[level].map_own_fail++;
        dra->opipe_status[level].map_fail_skb_free++;
        return 0;
    }

    (void)bsp_sc_dma_map_single(dra->dev, (void *)(uintptr_t)page, PAGE_SIZE, DMA_FROM_DEVICE);
    dra->opipe_status[level].map_own_success++;
    return page;
}

static struct sk_buff* __dra_alloc_skb(struct dra *dra, struct dra_opipe *opipe, gfp_t gfp)
{
    struct sk_buff *skb = NULL;
    char *buff = NULL;
    unsigned int len = g_level_size[opipe->level];
    unsigned int data_size = g_level_data_size[opipe->level];
    int ret;

    buff = (char *)kmalloc(len, gfp);
    if (unlikely(buff == NULL)) {
        return NULL;
    }

    skb = (struct sk_buff *)(buff + DRA_SKB_OFFSET(len));
    ret = memset_s((void *)skb, sizeof(*skb), 0, sizeof(struct sk_buff));
    if (ret != EOK) {
        dra_err("mem set fail:%d\n", ret);
        kfree(buff);
        return NULL;
    }
    skb->head = buff;
    skb->data = buff;
    skb_reset_tail_pointer(skb);
    skb->end = DRA_SKB_RESERVE_SIZE + data_size;

    dra->opipe_status[opipe->level].alloc_success++;
    if (gfp == GFP_ATOMIC) {
        dra->opipe_status[opipe->level].alloc_atomic++;
    } else if (gfp == GFP_KERNEL) {
        dra->opipe_status[opipe->level].alloc_kernel++;
    }

    return skb;
}

struct sk_buff *dra_alloc_skb(struct dra *dra, struct dra_opipe *opipe, dra_buf_t *p_addr, gfp_t gfp)
{
    struct sk_buff *skb = NULL;
    struct dra_tab *tab = NULL;
    unsigned long long addr;
    unsigned int level = opipe->level;
    unsigned int head_rom = DRA_SKB_RESERVE_SIZE;
    unsigned int data_size = g_level_data_size[level];
    unsigned int offset;
    unsigned long flags;

    if (dra_list_is_full(dra, level)) {
        dra->opipe_status[level].map_own_fail++;
        return 0;
    }

    skb = __dra_alloc_skb(dra, opipe, gfp);
    if (unlikely(skb == NULL)) {
        dra->opipe_status[level].alloc_fail++;
        dra_err("skb == NULL\n");
        return NULL;
    }

    offset = DRA_SKB_OFFSET(g_level_size[opipe->level]);
    addr = dra_create_desc(dra, virt_to_phys(skb->data) + head_rom, offset, level, DRA_SKB_FROM_OWN);
    p_addr->addr = addr;

    (void)bsp_sc_dma_map_single(dra->dev, skb->head, head_rom + data_size, DMA_FROM_DEVICE);
    skb_reserve(skb, head_rom);
    dra->opipe_status[level].map_own_success++;

    tab = (struct dra_tab *)(skb->head + DRA_SKB_RESERVE_SIZE + data_size);
    tab->dra_addr = addr;
    tab->magic = DRA_OWN;
    tab->manager = skb;

    spin_lock_irqsave(&opipe->lock, flags);
    list_add_tail(&tab->list, &opipe->map.busy_list);
    opipe->busy_depth++;
    spin_unlock_irqrestore(&opipe->lock, flags);
    return skb;
}

void dra_free_buf(struct dra *dra, dra_buf_t buf)
{
    struct sk_buff *skb = NULL;

    if (unlikely(buf.bits.level == DRA_OPIPE_LEVEL_3)) {
        dra_recycle_mem(dra, buf);
        bsp_info("used ap 9k\n");
        return;
    }

    if (unlikely(buf.bits.sc_mode != dra->para->sc_mode)) {
        dra_err("dra check sc mode error, 0x%llx\n", buf.addr);
        return;
    }

    if (buf.bits.from == DRA_SKB_FROM_OWN) {
        dra_rls_input(dra, (buf.addr & DRA_ADDR_MASK) + DRA_SKB_RESERVE_SIZE, &dra->rls_opipes, DRA_OPIPE_LEVEL_1);
    } else if (buf.bits.from == DRA_SKB_FROM_TCP_IP) {
        skb = dra_unmap_tcpip_skb(dra, buf);
        if (skb != NULL) {
            dev_kfree_skb_any(skb);
        }
    } else {
        dra_err("dra check dra error, 0x%llx\n", buf.addr);
    }
    return;
}

static int dra_fill_from_alloc(struct dra *dra, struct dra_opipe *opipe, struct dra_fifo *fifo)
{
    dra_buf_t mbuf;
    unsigned long page;
    struct sk_buff *skb = NULL;

    if (!dra_sys_mem_protect(dra)) {
        return -1;
    }

    if (DRA_OPIPE_LEVEL_2 != opipe->level && DRA_OPIPE_LEVEL_7 != opipe->level) {
        skb = dra_alloc_skb(dra, opipe, &mbuf, GFP_KERNEL);
        if (unlikely(skb == NULL)) {
            return -1;
        }
        dra_bigpool_input(dra, mbuf.addr, opipe);
    } else {
        page = dra_alloc_page(dra, opipe, &mbuf, opipe->level);
        if (unlikely(!page)) {
            return -1;
        }
        dra_input(dra, mbuf.addr, opipe);
    }

    return 0;
}

unsigned int dra_fill_fifo(struct dra *dra, struct dra_fifo *fifo, unsigned int cnt)
{
    unsigned long flag;
    struct dra_tab *tab = NULL;
    unsigned int i = 0;
    struct dra_opipe *opipe = container_of(fifo, struct dra_opipe, fifo);

    spin_lock_irqsave(&opipe->lock, flag);
    while (i < cnt && !list_empty(&opipe->map.reclaim_list)) {
        tab = list_first_entry(&opipe->map.reclaim_list, struct dra_tab, list);
        list_move_tail(&tab->list, &opipe->map.busy_list);
        opipe->busy_depth++;
        opipe->reclaim_depth--;
        if (DRA_OPIPE_LEVEL_2 != opipe->level) {
            dra_bigpool_input(dra, tab->dra_addr, opipe);
        } else {
            dra_input(dra, tab->dra_addr, opipe);
        }
        opipe->reclaim_cnt++;
        i++;
    }
    spin_unlock_irqrestore(&opipe->lock, flag);

    for (; i < cnt; i++) {
        if (dra_fill_from_alloc(dra, opipe, fifo) != 0) { /* just fill a buff, retren err goto break */
            break;
        }
    }

    return !(i == cnt); /* when num of fill is cnt return 0 else return 1 */
}

void dra_free_one_buf(struct dra *dra, dra_buf_t buf)
{
    struct sk_buff *skb = NULL;

    skb = __dra_unmap_skb(dra, buf, NULL);
    if (skb != NULL && skb->head != NULL) {
        dra_buf_cache_flush(dra, buf, DRA_SKB_RESERVE_SIZE + g_level_data_size[buf.bits.level]);
        kfree(skb->head);
        dra->opipe_status[buf.bits.level].skb_reclaim_free++;
    } else {
        dra_err("skb null\n");
    }
}

void dra_recycle_to_slab(struct dra *dra, unsigned int level)
{
    struct dra_opipe *opipe = NULL;
    struct dra_ipipe *ipipe = NULL;
    dra_buf_t buf;
    int i, cnt;

    ipipe = dra->cpu_ipipes + dra->cpu_using_ipipe[level];
    opipe = &dra->opipes[level];
    cnt = dra_get_busy_cnt(dra, opipe);

    for (i = 0; i < cnt; i++) {
        buf.addr = dra_ipipe_get_buf(dra, ipipe);
        if (buf.addr == 0) {
            break;
        }
        dra_free_one_buf(dra, buf);
    }
}

void dra_clear_reclaim(struct dra *dra, unsigned int level)
{
    struct dra_opipe *opipe = NULL;
    unsigned long flag;
    struct dra_tab *tab = NULL;
    struct dra_tab *tmp = NULL;
    struct sk_buff *skb = NULL;
    dra_buf_t buf;
    LIST_HEAD(local_list);

    opipe = &dra->opipes[level];
    if (list_empty(&opipe->map.reclaim_list)) {
        return;
    }

    spin_lock_irqsave(&opipe->lock, flag);
    list_splice_init(&opipe->map.reclaim_list, &local_list);
    opipe->reclaim_depth = 0;
    spin_unlock_irqrestore(&opipe->lock, flag);

    list_for_each_entry_safe(tab, tmp, &local_list, list) {
        if (unlikely(tab->magic != DRA_OWN)) {
            dra_err("reclaim free magic %x, olddra 0x%llx\n", tab->magic, tab->dra_addr);
            continue;
        }
        tab->magic = DRA_FREE;
        buf.addr = tab->dra_addr;
        dra_buf_cache_flush(dra, buf, DRA_SKB_RESERVE_SIZE + g_level_data_size[buf.bits.level]);

        skb = (struct sk_buff *)tab->manager;
        if (skb != NULL && skb->head != NULL) {
            kfree(skb->head);
            dra->opipe_status[level].skb_reclaim_free++;
        } else {
            dra_err("skb info err dra addr:0x%llx\n", buf.addr);
        }
        dra->opipe_status[level].unmap_own++;
    }
}


void dra_bigpool_timer(struct timer_list *t)
{
    struct dra *dra = from_timer(dra, t, timer);
    int i;
    unsigned int wptr, rptr;

    if (dra->event & DRA_EVENT_RESETTING) {
        mod_timer(&dra->timer, jiffies + DRA_TIMER_EXPIRES);
        dra->timer_cnt++;
        dra_err("dra is resetting\n");
        return;
    }

    for (i = 1; i < DRA_OPIPE_LEVEL_2; i++) {
        if (dra->opipes[i].used != DRA_OPIPE_USED_MAGIC) {
            continue;
        }
        dra_get_big_pool_ptr(dra, &dra->opipes[i], &wptr, &rptr);
        if ((dra->opipes[i].last_wptr == wptr) && (dra->opipes[i].last_rptr == rptr)) {
            dra->opipes[i].equal_times++;
        } else {
            dra->opipes[i].equal_times = 0;
        }

        dra->opipes[i].last_wptr = wptr;
        dra->opipes[i].last_rptr = rptr;
        if (dra->opipes[i].equal_times > 3) { // try 3 times to make sure idle
            dra_recycle_to_slab(dra, i);
            dra_clear_reclaim(dra, i);
            dra->opipes[i].equal_times = 0;
            dra->opipes[i].timer_free_cnt++;
        }
    }

    mod_timer(&dra->timer, jiffies + DRA_TIMER_EXPIRES);
    dra->timer_cnt++;

    return;
}

struct sk_buff *dra_to_skb(struct dra *dra, unsigned long long addr, unsigned long long *orig)
{
    struct sk_buff *skb = NULL;
    dra_buf_t buf;
    struct dra_tab *tab = NULL;

    buf.addr = addr;
    tab = dra_get_tab(dra, buf);
    if (tab == NULL) {
        return NULL;
    }

    skb = (struct sk_buff *)tab->manager;
    if (orig != NULL) {
        *orig = tab->dra_addr;
    }

    return skb;
}

void __dra_release_func(struct dra *dra, dra_buf_t *local_buf, unsigned int cnt)
{
    dra_buf_t buf;
    unsigned int i;
    struct sk_buff *skb = NULL;

    for (i = 0; i < cnt; i++) {
        buf = local_buf[i];
        if (buf.bits.from != DRA_SKB_FROM_TCP_IP) {
            dra_err("dra release dra error, 0x%llx\n", buf.addr);
            continue;
        }

        skb = dra_unmap_tcpip_skb(dra, buf);
        if (skb != NULL) {
            dev_kfree_skb_any(skb);
        }
    }
}

void dra_release_func(struct dra *dra)
{
    struct dra_release_poll *release = &dra->release;
    dra_buf_t *base = (dra_buf_t *)(uintptr_t)release->fifo.base;
    unsigned int max_free_cnt = 0;
    int ret;
    unsigned int rsl_busy_cnt;
    unsigned int cnt = 0;
    unsigned int tail_num;

    release->fifo.write = dra_get_rls(dra);

    while (release->fifo.write != release->fifo.read) {
        rsl_busy_cnt = dra_get_busy_num(release->fifo.write, release->fifo.read, release->fifo.depth);
        if (rsl_busy_cnt > DRA_BURST_LEN) {
            cnt = DRA_BURST_LEN;
        } else {
            cnt = rsl_busy_cnt;
        }
        if (release->fifo.read + cnt <= release->fifo.depth) {
            ret = memcpy_s(release->local_release, sizeof(release->local_release), base + release->fifo.read,
                cnt * sizeof(dra_buf_t));
        } else {
            /* copy two space */
            tail_num = release->fifo.depth - release->fifo.read;
            ret = memcpy_s(release->local_release, sizeof(release->local_release), base + release->fifo.read,
                tail_num * sizeof(dra_buf_t));
            ret = memcpy_s(release->local_release + tail_num,
                sizeof(release->local_release) - tail_num * sizeof(dra_buf_t), base,
                (cnt - tail_num) * sizeof(dra_buf_t));
        }
        if (ret) {
            release->mcp_fail++;
        }
        max_free_cnt += cnt;
        __dra_release_func(dra, release->local_release, cnt);
        release->fifo.read += cnt;
        if (release->fifo.read >= release->fifo.depth) {
            release->fifo.read -= release->fifo.depth;
        }
    }
    if (release->max_free_cnt < max_free_cnt) {
        release->max_free_cnt = max_free_cnt;
    }
    if (release->min_free_cnt > max_free_cnt) {
        release->min_free_cnt = max_free_cnt;
    }

    dra_set_rls(dra, release->fifo.read);
}

int dra_refill_bigpool_work(struct dra *dra)
{
    unsigned int i;
    unsigned int total;
    int ret = 0;

    dra->status.work_start = bsp_get_slice_value();
    dra->irq_status = dra_get_intr_status(dra);
    dra_release_func(dra);
    for (i = 1; i <= dra->max_opipes; i++) {
        if ((dra->int_mask & BIT(i)) && (dra->irq_status & BIT(i))) {
            total = dra_get_space(dra, &dra->opipes[i]);
            if (total) {
                ret |= dra_fill_fifo(dra, &dra->opipes[i].fifo, total - dra->opipes[i].fifo.idx);
            }
        }
    }

    if (dra->status.work_start - dra->irq_time > DRA_SCHED_GAP) {
        bsp_diag_drv_log_report(BSP_P_INFO, "dra sche time:%u, irq:%u, work:%u\n", dra->status.work_start - dra->irq_time,
            dra->irq_time, dra->status.work_start);
    } else {
        dra_print_slice("dra sche time:%u, irq:%u, work:%u\n", dra->status.work_start - dra->irq_time,
            dra->irq_time, dra->status.work_start);
    }

    dra->status.work_end = bsp_get_slice_value();
    return ret;
}

static int dra_thread(void *data)
{
    struct dra *dra = g_dra_ctx;
    int ret;

    while (1) {
        wait_event(dra->wqueue, test_bit(DRA_EVENT_THREAD_RUNNING_BIT, (void *)&dra->event) || test_bit(DRA_EVENT_THREAD_STOPING_BIT, (void *)&dra->event));
        if (unlikely(dra->event & DRA_EVENT_THREAD_STOPING)) {
            __pm_relax(dra->wake_lock);
            clear_bit(DRA_EVENT_THREAD_RUNNING_BIT, (void *)&dra->event);
            clear_bit(DRA_EVENT_THREAD_STOPING_BIT, (void *)&dra->event);
            set_bit(DRA_EVENT_THREAD_STOPED_BIT, (void *)&dra->event);
            wake_up(&dra->wqueue);
            continue;
        }

        ret = dra_refill_bigpool_work(dra);
        if (ret) {
            dra->status.sche_cnt++;
            msleep(1);
            continue;
        }

        __pm_relax(dra->wake_lock);
        clear_bit(DRA_EVENT_THREAD_RUNNING_BIT, (void *)&dra->event); // before enable intr
        dra_set_intr(dra); // enable intr
        dra->status.refill_succ++;
    }

    return 0;
}

irqreturn_t dra_interrupt(int irq, void *dev_id)
{
    struct dra *dra = (struct dra *)dev_id;

    __pm_stay_awake(dra->wake_lock);
    dra_top_intr(dra);

    dra->irq_time = bsp_get_slice_value();

    set_bit(DRA_EVENT_THREAD_RUNNING_BIT, (void *)&dra->event);
    wake_up(&dra->wqueue);

    return IRQ_HANDLED;
}

static unsigned int dra_set_dthrl(struct dra *dra, struct dra_fifo *fifo, int level)
{
    unsigned int depth;

    switch (level) {
        case DRA_OPIPE_LEVEL_0:
        case DRA_OPIPE_LEVEL_1:
        case DRA_OPIPE_LEVEL_2:
        case DRA_OPIPE_LEVEL_7:
            depth = fifo->depth >> 4;
            break;
        default:
            depth = fifo->depth - DRA_TWO_BURST_RESERVE - 1;
    }

    return (depth > DRA_FIFO_MIN_DEPTH) ? depth : DRA_FIFO_MIN_DEPTH;
}

int dra_ddr_buf_init(struct dra *dra, unsigned int level)
{
    struct dra_fifo *fifo = &dra->ddr_buf[level].fifo;

    fifo->depth = dra->para->ddr_buf_depth[level];
    if (fifo->depth == 0) {
        return 0;
    }

    fifo->base = dra->virt_base + dra->offset;
    fifo->phy = dra->dma_base + dra->offset;
    dra->offset += fifo->depth * sizeof(dra_buf_t);

    fifo->dthrl = dra_set_dthrl(dra, fifo, level);
    fifo->dbg_idx = dra->para->ddr_buf_dbg_idx[level];

    dra_ddr_buf_fifo_init(dra, fifo, level);

    return 0;
}

int dra_create_tab(struct dra *dra, struct dra_map *map, unsigned int level)
{
    unsigned int i;
    struct dra_tab *tmp = NULL;

    INIT_LIST_HEAD(&map->free_list);
    INIT_LIST_HEAD(&map->busy_list);
    INIT_LIST_HEAD(&map->reclaim_list);

    map->tab_max_size = dra->para->oparamter.tab_size[level];
    if (!dra->opipes[level].use_tab) {
        dra_err("level:%u no use tab\n", level);
        return 0;
    }

    map->tab = (struct dra_tab *)kzalloc(sizeof(struct dra_tab) * map->tab_max_size, GFP_KERNEL);
    if (map->tab == NULL) {
        dra_err("no map tab alloc\n");
        return -ENOMEM;
    }

    for (i = DRA_TABEL_START; i < map->tab_max_size; i++) {
        tmp = map->tab + i;
        tmp->idx = i;
        map->total++;
        map->free++;
        list_add_tail(&tmp->list, &map->free_list);
    }

    return 0;
}

int dra_opipe_init(struct dra *dra, int level)
{
    int ret;
    struct dra_map *map = &dra->opipes[level].map;
    struct dra_fifo *fifo = &dra->opipes[level].fifo;
    spin_lock_init(&dra->opipes[level].lock);

    fifo->depth = dra->para->oparamter.depth[level];
    dra->opipes[level].from = dra->para->oparamter.from[level];
    dra->opipes[level].level = level;

    ret = dra_create_tab(dra, map, level);
    if (ret) {
        return ret;
    }

    if (fifo->depth == 0) {
        dra_err("level:%u no use opipe\n", level);
        return 0;
    }

    fifo->base = dra->virt_base + dra->offset;
    fifo->phy = dra->dma_base + dra->offset;
    dra->offset += fifo->depth * sizeof(dra_buf_t);

    fifo->update_read_virt = dra->virt_base + dra->offset;
    fifo->update_read_addr = dra->dma_base + dra->offset;
    dra->offset += sizeof(unsigned long long);

    fifo->size = g_level_size[level];
    fifo->uthrl = fifo->depth - DRA_BURST_LEN - 1;
    fifo->dthrl = dra_set_dthrl(dra, fifo, level);
    fifo->free = fifo->depth - fifo->dthrl;
    fifo->dbg_idx = dra->para->oparamter.dbg_idx[level];
    dra_fifo_init(dra, fifo, level);

    if (!dra->para->oparamter.disable_irq[level]) {
        dra->int_mask |= 1 << (unsigned int)level;
    }

    dra_fill_fifo(dra, fifo, fifo->dthrl);
    dra->opipes[level].status = DRA_OPIPE_STATUS_INIT;
    return 0;
}

void dra_release_pool_init(struct dra *dra)
{
    struct dra_fifo *fifo = &dra->release.fifo;

    fifo->base = dra->virt_base + dra->offset;
    fifo->phy = dra->dma_base + dra->offset;
    dra->offset += fifo->depth * sizeof(dra_buf_t);

    __dra_release_pool_init(dra); // hal
}

void dra_global_init(struct dra *dra)
{
    dra->int_mask = 0;
}

static void dra_dump_hook(void)
{
    unsigned int *reg = NULL;
    unsigned int i, j;
    unsigned int num = 0;
    struct dra *dra = g_dra_ctx;

    for (i = 0; i < DRA_BACK_REGS_NUM && (dra != NULL); i++) {
        for (j = 0; j < dra->back[i].num; j++) {
            reg = dra->dump_base + num + j;
            *reg = readl((void *)(dra->back[i].start + j * sizeof(unsigned int)));
        }
        num += dra->back[i].num;
    }
}

static void dra_dump_init(struct dra *dra)
{
    int ret;

    dra_set_backup_regs(dra);
    dra->dump_base = (unsigned int *)bsp_dump_register_field(DUMP_MODEMAP_DRA, "DRA", DRA_DUMP_SIZE, 0);
    if (dra->dump_base == NULL) {
        dra_err("dump mem alloc fail\n");
        return;
    }
    ret = (int)memset_s(dra->dump_base, DRA_DUMP_SIZE, 0, DRA_DUMP_SIZE);
    if (ret) {
        dra_err("dump memset_s fail\n");
    }

    ret = bsp_dump_register_hook("dra", dra_dump_hook);
    if (ret == BSP_ERROR) {
        dra_err("register om fail\n");
    }
}

static int dra_creat_thread(struct dra *dra)
{
    struct task_struct *thread = NULL;
    struct sched_param sch_para = {
        .sched_priority = DRA_THREAD_PRIORITY
    };

    thread = kthread_run(dra_thread, NULL, "dra_thread");
    if (thread == NULL) {
        dra_err("creat thread fail\n");
        return -1;
    }

    if (sched_setscheduler(thread, SCHED_FIFO, &sch_para)) {
        dra_err("change priority fail\n");
        return -1;
    }

    dra->thread = thread;
    return 0;
}

int dra_alloc_dma_mem(struct dra *dra)
{
    unsigned int level, i;
    unsigned int size = 0;

    size += (dra->release.fifo.depth) * sizeof(dra_buf_t);
    for (level = 0; level <= dra->max_opipes; level++) {
        if (!dra_opipe_is_enable(dra, level)) {
            continue;
        }
        size += dra->para->ddr_buf_depth[level] * sizeof(dra_buf_t);
        size += dra->para->oparamter.depth[level] * sizeof(dra_buf_t) + sizeof(unsigned long long);
    }
    for (i = 0; i < dra->para->iparamter.cnt; i++) {
        size += dra->para->iparamter.depth[i] * sizeof(dra_buf_t) + sizeof(unsigned long long);
    }

    size += dra->para->rls_chn_depth * sizeof(dra_buf_t) + sizeof(unsigned long long);

    dra->dma_size = size;
    dra->virt_base = (unsigned long long)(uintptr_t)bsp_sc_alloc_ringbuf(dra->dev, size, &dra->dma_base, GFP_KERNEL);
    if (dra->virt_base == 0) {
        dra_err("dma mem alloc fail\n");
        return -ENOMEM;
    }

    return 0;
}

void dra_set_interrupt(unsigned int val)
{
    struct dra *dra = g_dra_ctx;

    dra->int_mask = val;
    dra_interupt_init(dra);
}

int dra_mem_input(unsigned int cnt)
{
    struct dra *dra = g_dra_ctx;
    dra_buf_t mbuf;
    struct sk_buff *skb = NULL;
    struct dra_opipe *opipe = &dra->opipes[DRA_OPIPE_LEVEL_1];
    unsigned int i = 0;

    for (i = 0; i < cnt; i++) {
        skb = dra_alloc_skb(dra, opipe, &mbuf, GFP_KERNEL);
        if (unlikely(skb == NULL)) {
            break;
        }
        dra_rls_input(dra, mbuf.addr, &dra->rls_opipes, DRA_OPIPE_LEVEL_1);
    }

    return i;
}

int dra_core_probe(struct dra *dra)
{
    int ret;
    unsigned int level;
    g_dra_ctx = dra;

    BUILD_BUG_ON((SKB_DATA_ALIGN(DRA_HEAD_ROM_SIZE) > NET_SKB_PAD));
    BUILD_BUG_ON((DRA_SKB_RESERVE_SIZE + DRA_OPIPE_LEVEL_1_SIZE + sizeof(struct dra_tab) + DRA_MAX_SKB_SIZE)
        > DRA_OPIPE_LEVEL_1_BUFF_SIZE);

    dra_global_init(dra);
    dra_basic_cfg(dra);

    ret = dra_alloc_dma_mem(dra);
    if (ret) {
        return -1;
    }
    dra_release_pool_init(dra);

    /* ddr buf init must before opipe init */
    for (level = 0; level <= dra->max_opipes; level++) {
        if (!dra_opipe_is_enable(dra, level)) {
            continue;
        }
        ret = dra_ddr_buf_init(dra, level);
        if (ret) {
            dra_err("dra_ddr_buf_init failed\n");
            return -1;
        }
        ret = dra_opipe_init(dra, level);
        if (ret) {
            return -1;
        }
    }

    dra_cpu_ipipe_init(dra);
    dra_rls_chn_init(dra, DRA_OPIPE_LEVEL_1);

    dra->event = 0;
    init_waitqueue_head(&dra->wqueue);
    if (dra_creat_thread(dra)) {
        return -1;
    }

    dra_interupt_init(dra);

    if (bsp_reset_cb_func_register("DRA", dra_mdmreset_cb, 0, DRV_RESET_CB_PIOR_DRA)) {
        bsp_err("set modem reset call back func failed\n");
    }

    dra_dump_init(dra);
    dra_trans_report_register();
    dra_err("[init] ok\n");

    return 0;
}

int dra_suspend(struct device *dev)
{
    return dra_hal_suspend(g_dra_ctx);
}
EXPORT_SYMBOL(dra_suspend);
int dra_resume(struct device *dev)
{
    return dra_hal_resume(g_dra_ctx);
}
EXPORT_SYMBOL(dra_resume);

int dra_deinit(void)
{
    struct dra *dra = g_dra_ctx;

    dra_detach(dra);
    disable_irq(dra->irq);

    set_bit(DRA_EVENT_RESETTING_BIT, (void *)&dra->event);
    set_bit(DRA_EVENT_THREAD_STOPING_BIT, (void *)&dra->event);
    wake_up(&dra->wqueue);
    wait_event(dra->wqueue, test_bit(DRA_EVENT_THREAD_STOPED_BIT, (void *)&dra->event) && !test_bit(DRA_EVENT_REPORT_BIT, (void *)&dra->event));

    mod_timer(&dra->timer, jiffies + DRA_TIMER_EXPIRES);

    dra->deinit_cnt++;
    return 0;
}

int dra_interrupt_reinit(void)
{
    struct dra *dra = g_dra_ctx;
    dra->event = 0;
    dra_interupt_init(dra);
    enable_irq(dra->irq);
    return 0;
}

static void dra_fifo_clear(struct dra_fifo *fifo)
{
    int ret;

    fifo->idx = 0;
    fifo->write = 0;
    fifo->read = 0;

    ret = memset_s(fifo->local_buf, DRA_BURST_LEN * sizeof(unsigned long long), 0, DRA_BURST_LEN * sizeof(unsigned long long));
    if (ret) {
        dra_err("memset_s fail\n");
    }

    if (fifo->update_write_virt != 0) {
        *(unsigned int *)(uintptr_t)fifo->update_write_virt = 0;
    }

    if (fifo->update_read_virt != 0) {
        *(unsigned int *)(uintptr_t)fifo->update_read_virt = 0;
    }
}

int bsp_dra_reinit(void)
{
    int i;
    struct dra *dra = g_dra_ctx;
    unsigned long flags;
    struct dra_fifo *fifo = NULL;

    __dra_release_pool_init(dra);

    dra_fifo_clear(&dra->rls_opipes.fifo);
    dra_rls_fifo_init(dra, &dra->rls_opipes, DRA_OPIPE_LEVEL_1);

    for (i = DRA_OPIPE_LEVEL_1; i <= dra->max_opipes; i++) {
        if (!dra_opipe_is_enable(dra, i) || (dra->ddr_buf[i].fifo.depth == 0 || dra->opipes[i].fifo.depth == 0)) {
            continue;
        }

        dra_ddr_buf_fifo_init(dra, &dra->ddr_buf[i].fifo, i);
        spin_lock_irqsave(&dra->opipes[i].lock, flags);
        list_splice_init(&dra->opipes[i].map.busy_list, &dra->opipes[i].map.reclaim_list);
        dra->opipes[i].reclaim_depth += dra->opipes[i].busy_depth;
        dra->opipes[i].delay_free_cnt += dra->opipes[i].busy_depth;
        dra->opipes[i].busy_depth = 0;
        spin_unlock_irqrestore(&dra->opipes[i].lock, flags);
        dra_fifo_clear(&dra->opipes[i].fifo);
        dra_fifo_reinit(dra, &dra->opipes[i].fifo, i);
        dra_fill_fifo(dra, &dra->opipes[i].fifo, dra->opipes[i].fifo.dthrl);
    }

    dra_clear_tcpip(dra, DRA_OPIPE_LEVEL_0);
    for (i = 0; i < dra->max_cpu_ipipes; i++) {
        if (!dra->para->iparamter.depth[i]) {
            continue;
        }
        fifo = &dra->cpu_ipipes[i].fifo;
        fifo->empty = 1;
        dra_fifo_clear(fifo);
        dra_ipipe_init(dra, &dra->cpu_ipipes[i], dra->para->iparamter.ipipeidx[i]);
    }

    dra_fifo_clear(&dra->release.fifo);

    dra_interrupt_reinit();
    dra->reinit_cnt++;
    return 0;
}

int dra_mdmreset_cb(drv_reset_cb_moment_e eparam, int userdata)
{
    if (eparam == MDRV_RESET_CB_BEFORE) {
        dra_deinit();
    }
    return 0;
}
