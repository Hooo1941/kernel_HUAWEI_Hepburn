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

#include "dra_hal.h"
#include "dra_def.h"
#include <bsp_print.h>
#include <securec.h>

#define THIS_MODU mod_dra
#define DRA_FIFO_RESERVE_CNT 0x80
#define DRA_BIG_POOL 1
#define DRA_RLS 0

#define dra_dbg_hal(fmt, ...)                             \
    do {                                                  \
        if (g_dra_ctx->dbg & DRA_DEBUG_MSG) {             \
            bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
        }                                                 \
    } while (0)

// 低功耗排空，送DDR，DDR满再送recycle
void __dra_release_pool_init(struct dra *dra)
{
    union dra_dbg_cnt_sel_u dbg_sel;
    struct dra_fifo *fifo = &dra->release.fifo;
    writel(fifo->phy & 0xffffffff, dra->regs + DRA_RECYCLE_POOL_BASE_ADDR_L(0));
    writel(fifo->phy >> 32, dra->regs + DRA_RECYCLE_POOL_BASE_ADDR_H(0)); // 32 high bits
    writel(fifo->depth - 1, dra->regs + DRA_RECYCLE_POOL_DEPTH(0));
    writel(fifo->uthrl, dra->regs + DRA_RECYCLE_POOL_UP_THRH(0));
    dbg_sel.val = 0;
    dbg_sel.bits.dbg_cnt_chn_sel = 0;
    dbg_sel.bits.dbg_cnt_cate_sel = DRA_DBG_CATE_SINGLE_RECYCLE;
    writel(dbg_sel.val, dra->regs + DRA_DBG_CNT_SEL(DRA_DBG_RECYCLE0));
}

void dra_rls_bigpool_fifo_init(struct dra *dra, struct dra_fifo *fifo, int level, unsigned int idx, unsigned int is_bigpool)
{
    union dra_rls_ch_ctrl_u ctrl;
    union dra_dbg_cnt_sel_u dbg_sel;

    writel(fifo->phy & 0xffffffff, dra->regs + DRA_RLS_CH_BASE_ADDR_L(idx));
    writel(fifo->phy >> 32, dra->regs + DRA_RLS_CH_BASE_ADDR_H(idx)); // 32 high bits
    writel(fifo->depth - 1, dra->regs + DRA_RLS_CH_DEPTH(idx));
    writel(fifo->update_read_addr & 0xffffffff, dra->regs + DRA_RLS_CH_PTR_ADDR_L(idx));
    writel((fifo->update_read_addr >> 32) & 0xffffffff, dra->regs + DRA_RLS_CH_PTR_ADDR_H(idx)); // 32 high bits
    writel(fifo->dthrl, dra->regs + DRA_RLS_CH_THRH(idx));

    ctrl.val = readl(dra->regs + DRA_RLS_CH_CTRL(idx));
    ctrl.bits.rls_ch_des_sel = 0;
    ctrl.bits.rls_ch_level_sel = level;
    ctrl.bits.rls_ch_recycle_pool_sel = 0;
    ctrl.bits.rls_ch_big_release_pool_sel = is_bigpool;
    ctrl.bits.rls_ch_push_ptr_en = (fifo->update_read_addr != 0) ? 1 : 0;

    writel(ctrl.val, dra->regs + DRA_RLS_CH_CTRL(idx));
    dbg_sel.val = 0;
    dbg_sel.bits.dbg_cnt_chn_sel = idx;
    dbg_sel.bits.dbg_cnt_cate_sel = DRA_DBG_CATE_SINGLE_RLS;
    writel(dbg_sel.val, dra->regs + DRA_DBG_CNT_SEL(fifo->dbg_idx));

    writel(1, dra->regs + DRA_RLS_CH_EN(idx));
}

unsigned int dra_hal_set_rls(struct dra *dra, unsigned long long rlsbase, unsigned int update_ptr,
    unsigned int depth, struct dra_rls_para *rls_para)
{
    struct dra_fifo fifo = {0};

    fifo.phy = rlsbase;
    fifo.depth = depth;
    fifo.update_read_addr = update_ptr;
    fifo.dthrl = depth - DRA_BURST_LEN;
    fifo.dbg_idx = rls_para->dbg_idx;

    dra_rls_bigpool_fifo_init(dra, &fifo, rls_para->level, rls_para->idx, DRA_RLS);

    return (unsigned int)(uintptr_t)dra->phy_regs + DRA_RLS_CH_WPTR(rls_para->idx);
}

void dra_rls_fifo_init(struct dra *dra, struct dra_rls_ch *rls, unsigned int level)
{
    dra_rls_bigpool_fifo_init(dra, &rls->fifo, level, rls->idx, DRA_RLS);
}

void dra_rls_input(struct dra *dra, unsigned long long buf, struct dra_rls_ch *rls, unsigned int level)
{
    struct dra_fifo *fifo = &rls->fifo;
    unsigned long long *base = (unsigned long long *)(uintptr_t)fifo->base;
    unsigned long long *local_fifo = fifo->local_buf;
    int ret;
    unsigned long flags;

    spin_lock_irqsave(&dra->rls_opipes.lock, flags);
    local_fifo[fifo->idx++] = buf;
    if (fifo->idx == DRA_BURST_LEN) {
        fifo->idx = 0;
        fifo->read = *(unsigned int *)(uintptr_t)fifo->update_read_virt;
        fifo->free = (fifo->write >= fifo->read) ? (fifo->depth - 1 - fifo->write + fifo->read)
            : (fifo->read - fifo->write - 1);
        if (fifo->free < DRA_BURST_LEN) {
            spin_unlock_irqrestore(&dra->rls_opipes.lock, flags);
            dra->opipes[level].rls_full_cnt++;
            bsp_info("rls input full wptr:%u, rptr:%u, depth:%u\n", fifo->write, fifo->read, fifo->depth);
            return;
        }
        ret = memcpy_s(base + fifo->write, DRA_BURST_LEN * sizeof(unsigned long long), local_fifo, DRA_BURST_LEN * sizeof(unsigned long long));
        if (ret != EOK) {
            spin_unlock_irqrestore(&dra->rls_opipes.lock, flags);
            bsp_info("rls input cpy fail:%d\n", ret);
            return;
        }

        fifo->write += DRA_BURST_LEN;
        if (fifo->write == fifo->depth) {
            fifo->write = 0;
        }
        writel(fifo->write, dra->regs + DRA_RLS_CH_WPTR(rls->idx));
    }

    dra->opipes[level].delay_free_cnt++;
    spin_unlock_irqrestore(&dra->rls_opipes.lock, flags);
}

void dra_ddr_buf_fifo_init(struct dra *dra, struct dra_fifo *fifo, unsigned int level)
{
    union dra_dbg_cnt_sel_u dbg_sel;

    writel(fifo->phy & 0xffffffff, dra->regs + DRA_DDR_BUF_BASE_ADDR_L(level));
    writel(fifo->phy >> 32, dra->regs + DRA_DDR_BUF_BASE_ADDR_H(level)); // 32 high bits
    writel(fifo->depth - 1, dra->regs + DRA_DDR_BUF_DEPTH(level));
    writel(fifo->dthrl, dra->regs + DRA_DDR_BUF_DOWN_THRH(level));

    dbg_sel.val = 0;
    dbg_sel.bits.dbg_cnt_chn_sel = level;
    dbg_sel.bits.dbg_cnt_cate_sel = DRA_DBG_CATE_SINGLE_DDR_BUF;
    writel(dbg_sel.val, dra->regs + DRA_DBG_CNT_SEL(fifo->dbg_idx));
}

void dra_fifo_init(struct dra *dra, struct dra_fifo *fifo, int level)
{
    dra_rls_bigpool_fifo_init(dra, fifo, level, level, DRA_BIG_POOL);
}

int dra_ipipe_init(struct dra *dra, struct dra_ipipe *ipipe, unsigned int idx)
{
    union dra_alloc_ch_ctrl_u ctrl;
    union dra_dbg_cnt_sel_u dbg_sel;
    struct dra_fifo *fifo = &ipipe->fifo;

    dbg_sel.val = 0;
    dbg_sel.bits.dbg_cnt_chn_sel = idx;
    dbg_sel.bits.dbg_cnt_cate_sel = DRA_DBG_CATE_SINGLE_ALLOC;
    writel(dbg_sel.val, dra->regs + DRA_DBG_CNT_SEL(fifo->dbg_idx));

    writel(fifo->phy & 0xffffffff, dra->regs + DRA_ALLOC_CH_BASE_ADDR_L(idx));
    writel(fifo->phy >> 32, dra->regs + DRA_ALLOC_CH_BASE_ADDR_H(idx)); // 32 high bits
    writel(fifo->depth - 1, (void *)(dra->regs + DRA_ALLOC_CH_DEPTH(idx)));
    writel(fifo->update_write_addr & 0xffffffff, dra->regs + DRA_ALLOC_CH_PTR_ADDR_L(idx));
    writel((fifo->update_write_addr >> 32) & 0xffffffff, dra->regs + DRA_ALLOC_CH_PTR_ADDR_H(idx)); // 32 high bits
    ctrl.val = readl(dra->regs + DRA_ALLOC_CH_CTRL(idx));
    ctrl.bits.alloc_ch_level_sel = ipipe->level;
    ctrl.bits.alloc_ch_push_ptr_en = 1;
    writel(ctrl.val, dra->regs + DRA_ALLOC_CH_CTRL(idx));
    writel(1, (void *)(dra->regs + DRA_ALLOC_CH_EN(idx)));
    return 0;
}

void dra_interupt_init(struct dra *dra)
{
    bsp_err("dra->int_mask:%x", dra->int_mask);

    writel(0x1020, dra->regs + DRA_CORE0_INT_SUPRESS_CTRL);
    writel(0x1020, dra->regs + DRA_DBG_INT_SUPRESS_CTRL);
    writel(dra->int_mask, (dra->regs + DRA_CORE0_BIG_POOL_INT_MASK(0)));
    writel(1, (dra->regs + DRA_CORE0_RECYCLE_POOL_INT_MASK));
    return;
}

void dra_ipipe_read_update(struct dra *dra, unsigned int rptr, unsigned int idx)
{
    writel(rptr, dra->regs + DRA_ALLOC_CH_RPTR(idx));
}

void dra_input(struct dra *dra, unsigned long long buf, struct dra_opipe *opipe)
{
    struct dra_fifo *fifo = &opipe->fifo;
    unsigned long long *base = (unsigned long long *)(uintptr_t)fifo->base;
    dra_dbg_hal("dra_input: %llx\n", buf);
    base[fifo->write] = buf;
    fifo->write = (fifo->write >= fifo->depth - 1) ? 0 : fifo->write + 1;
    writel(fifo->write, dra->regs + DRA_RLS_CH_WPTR(opipe->level));
    dra_dbg_hal("wptr %x\n", readl(dra->regs + DRA_RLS_CH_WPTR(opipe->level)));
    dra_dbg_hal("rptr %x\n", readl(dra->regs + DRA_RLS_CH_RPTR(opipe->level)));
}

void dra_bigpool_input(struct dra *dra, unsigned long long buf, struct dra_opipe *opipe)
{
    struct dra_fifo *fifo = &opipe->fifo;
    unsigned long long *local_fifo = fifo->local_buf;
    int ret;

    local_fifo[fifo->idx++] = buf;
    if (fifo->idx == DRA_BURST_LEN) {
        fifo->idx = 0;

        ret = memcpy_s((unsigned long long *)(uintptr_t)fifo->base + fifo->write, DRA_BURST_LEN * sizeof(unsigned long long),
            local_fifo, DRA_BURST_LEN * sizeof(unsigned long long));
        if (ret != EOK) {
            bsp_info("rls input cpy fail:%d\n", ret);
            return;
        }

        fifo->write += DRA_BURST_LEN;
        if (fifo->write == fifo->depth) {
            fifo->write = 0;
        }
        writel(fifo->write, dra->regs + DRA_RLS_CH_WPTR(opipe->level));
    }
}

unsigned int dra_hal_set_adq(struct dra *dra, unsigned int level, unsigned long long adqbase, unsigned int update_ptr,
    unsigned int depth, unsigned int idx)
{
    union dra_alloc_ch_ctrl_u ctrl;
    writel_relaxed((unsigned int)adqbase & 0xffffffffU, (void *)(dra->regs + DRA_ALLOC_CH_BASE_ADDR_L(idx)));
    writel_relaxed((unsigned int)(adqbase >> 32) & 0xff, (void *)(dra->regs + DRA_ALLOC_CH_BASE_ADDR_H(idx))); // 32 high bits
    writel_relaxed(depth - 1, (void *)(dra->regs + DRA_ALLOC_CH_DEPTH(idx)));
    writel(update_ptr & 0xffffffff, dra->regs + DRA_ALLOC_CH_PTR_ADDR_L(idx));
    writel(0, dra->regs + DRA_ALLOC_CH_PTR_ADDR_H(idx));
    dra_err("hw_idx:0x%x, base:0x%llx write:0x%x  depth:0x%x\n", idx, adqbase, update_ptr, depth);
    ctrl.val = readl(dra->regs + DRA_ALLOC_CH_CTRL(idx));
    ctrl.bits.alloc_ch_level_sel = level;
    ctrl.bits.alloc_ch_push_ptr_en = 1;
    writel(ctrl.val, dra->regs + DRA_ALLOC_CH_CTRL(idx));
    writel(1, (void *)(dra->regs + DRA_ALLOC_CH_EN(idx)));
    return dra->phy_regs + DRA_ALLOC_CH_RPTR(idx);
}

int dra_enable_rls_ch(struct dra *dra, unsigned int idx, int enable)
{
    unsigned int time = 1000;
    union dra_rls_ch_ctrl_u ctrl;
    unsigned int val;

    if (enable == readl(dra->regs + DRA_RLS_CH_EN(idx))) {
        return 0;
    }
    if (enable == 0) {
        writel(1, (void *)(dra->regs + DRA_RLS_CH_FLUSH_EN_P(idx)));
        do {
            val = readl(dra->regs + DRA_RLS_CH_FLUSH_EN_P(idx));
            time--;
        } while (val != 0 && time > 0);
        if (val != 0) {
            return -1;
        }
        writel(0, (void *)(dra->regs + DRA_RLS_CH_EN(idx)));

        time = 1000;
        do {
            ctrl.val = readl(dra->regs + DRA_RLS_CH_CTRL(idx));
            time--;
        } while (ctrl.bits.rls_ch_busy != 0 && time > 0);
        if (ctrl.bits.rls_ch_busy) {
            return -1;
        }
    }else {
        writel(1, (void *)(dra->regs + DRA_RLS_CH_EN(idx)));
    }

    return 0;
}

int dra_enable_alloc_ch(struct dra *dra, unsigned int idx, int enable)
{
    unsigned int time = 1000;
    unsigned int cnt = 0;
    union dra_alloc_ch_ctrl_u ctrl;
    int ret = 0;

    writel(!!enable, (void *)(dra->regs + DRA_ALLOC_CH_EN(idx)));
    if (enable == 0) {
        do {
            ctrl.val = readl(dra->regs + DRA_ALLOC_CH_CTRL(idx));
            cnt++;
        } while (ctrl.bits.alloc_ch_busy != 0 && cnt <= time);
        if (ctrl.bits.alloc_ch_busy) {
            ret = -1;
        }
    }

    return ret;
}

unsigned int dra_get_space(struct dra *dra, struct dra_opipe *opipe)
{
    unsigned int wptr, rptr;
    struct dra_fifo *fifo = &opipe->fifo;
    wptr = readl(dra->regs + DRA_RLS_CH_WPTR(opipe->level));
    rptr = readl(dra->regs + DRA_RLS_CH_RPTR(opipe->level));
    return (wptr >= rptr) ? fifo->depth - 1 + rptr - wptr : rptr - wptr - 1;
}

void dra_get_big_pool_ptr(struct dra *dra, struct dra_opipe *opipe, unsigned int *wptr, unsigned int *rptr)
{
    *wptr = readl(dra->regs + DRA_DDR_BUF_WPTR(opipe->level));
    *rptr = readl(dra->regs + DRA_DDR_BUF_RPTR(opipe->level));
}

unsigned int dra_get_busy_cnt(struct dra *dra, struct dra_opipe *opipe)
{
    unsigned int wptr, rptr;
    struct dra_fifo *fifo = &dra->ddr_buf[opipe->level].fifo;
    unsigned int space, cnt;

    wptr = readl(dra->regs + DRA_DDR_BUF_WPTR(opipe->level));
    rptr = readl(dra->regs + DRA_DDR_BUF_RPTR(opipe->level));
    space = (wptr >= rptr) ? wptr -rptr : fifo->depth - rptr + wptr - 1;

    if (space > fifo->dthrl + DRA_FIFO_RESERVE_CNT) {
        cnt = space - fifo->dthrl - DRA_FIFO_RESERVE_CNT;
    } else {
        cnt = 0;
    }

    return cnt;
}

unsigned int dra_get_rls(struct dra *dra)
{
    return readl(dra->regs + DRA_RECYCLE_POOL_WPTR(0));
}

void dra_set_rls(struct dra *dra, unsigned int rptr)
{
    writel(rptr, dra->regs + DRA_RECYCLE_POOL_RPTR(0));
}

void dra_set_intr(struct dra *dra)
{
    writel(dra->int_mask, dra->regs + DRA_CORE0_BIG_POOL_INT_MASK(0));
    writel(1, dra->regs + DRA_CORE0_RECYCLE_POOL_INT_MASK);
}

unsigned int dra_get_intr_status(struct dra *dra)
{
    return readl(dra->regs + DRA_BIG_POOL_INT_RAW(0));
}

void dra_top_intr(struct dra *dra)
{
    dra->irq_status = readl(dra->regs + DRA_CORE0_BIG_POOL_INT_STATE(0));

    writel(0, dra->regs + DRA_CORE0_BIG_POOL_INT_MASK(0));
    writel(0, dra->regs + DRA_CORE0_RECYCLE_POOL_INT_MASK);
}

void dra_basic_cfg(struct dra *dra)
{
    writel(0xfff, dra->regs + DRA_AUTOCCLK);
    writel(1, dra->regs + DRA_DBG_CNT_CTRL);
}

int dra_hal_suspend(struct dra *dra)
{
    unsigned int i;

    /* acore rls dont use timeout */
    for (i = 1; i < DRA_OPIPE_ALLOC_LEVELS; i++) {
        writel(dra->opipes[i].fifo.dthrl / 4, dra->regs + DRA_RLS_CH_THRH(i)); // turn down dthrl to 1 / 4
    }
    writel(DRA_RELEASE_THRSH_LP, dra->regs + DRA_RECYCLE_POOL_UP_THRH(0));

    dra_set_intr(dra);

    return 0;
}

int dra_hal_resume(struct dra *dra)
{
    unsigned int i;
    unsigned int bigpool_irq_stat;
    unsigned int recyclepool_irq_stat;

    bigpool_irq_stat = readl(dra->regs + DRA_BIG_POOL_INT_RAW(0));
    recyclepool_irq_stat = readl(dra->regs + DRA_RECYCLE_POOL_INT_RAW);
    bsp_info("dra resume irq stat:%u-%u", bigpool_irq_stat, recyclepool_irq_stat);

    /* restore bigpool down waterline */
    for (i = 1; i < DRA_OPIPE_ALLOC_LEVELS; i++) {
        writel(dra->opipes[i].fifo.dthrl, dra->regs + DRA_RLS_CH_THRH(i));
    }
    writel(dra->release.fifo.uthrl, dra->regs + DRA_RECYCLE_POOL_UP_THRH(0));

    return 0;
}

void dra_detach(struct dra *dra)
{
    int i, idx;

    for (i = 0; i < dra->allc_para_size; i++) {
        idx = dra->allc_para[i].idx;
        if (idx != DRA_ALLOC_PARA_MAGIC) {
            writel_relaxed(0, dra->regs + DRA_ALLOC_CH_EN(idx));
        }
    }

    for (i = 0; i < dra->max_cpu_ipipes; i++) {
        idx = dra->cpu_ipipes[i].idx;
        writel(0, dra->regs + DRA_ALLOC_CH_EN(idx));
    }
}

void dra_fifo_reinit(struct dra *dra, struct dra_fifo *fifo, int level)
{
    dra_fifo_init(dra, fifo, level);
    fifo->read = 0;
    fifo->write = 0;
}

void dra_hds_transreport_hal(struct dra *dra, struct dra_debug_info *dbg_info)
{
    int level;

    dbg_info->irq_mask = readl(dra->regs + DRA_CORE0_BIG_POOL_INT_MASK(0));
    dbg_info->irq_raw = readl(dra->regs + DRA_BIG_POOL_INT_RAW(0));

    for (level = 0; level <= DRA_OPIPE_LEVEL_3; level++) {
        dbg_info->opipe_dbg[level].wptr = readl(dra->regs + DRA_RLS_CH_WPTR(level));
        dbg_info->opipe_dbg[level].rptr = readl(dra->regs + DRA_RLS_CH_RPTR(level));
    }
    for (level = 0; level < DRA_DBG_CNT_NUM; level++) {
        dbg_info->dbg_cnt[level] = readl_relaxed(dra->regs + DRA_DBG_CNT(level));
    }
}

void __dra_set_backup_regs(struct dra *dra, unsigned int idx, unsigned int start, unsigned int end)
{
    if (idx < DRA_BACK_REGS_NUM && end > start) {
        dra->back[idx].num = (end - start) / sizeof(unsigned int) + 1;
        if (dra->back[idx].num + dra->back_regs_num < DRA_DUMP_REG_NUM) {
            dra->back[idx].start = (char *)dra->regs + start;
            dra->back_regs_num += dra->back[idx].num;
        }
    }
}

void dra_set_backup_regs(struct dra *dra)
{
    __dra_set_backup_regs(dra, 0, DRA_CORE0_BIG_POOL_INT_MASK(0), DRA_CORE1_BIG_POOL_INT_STATE(1)); // 0x6000-0x605c
    __dra_set_backup_regs(dra, 1, DRA_DBG_CNT_SEL(0), DRA_DBG_CNT_SEL(35)); // dbg cnt:0x6300-0x638c
    __dra_set_backup_regs(dra, 2, DRA_DBG_CNT(0), DRA_DBG_CNT(35)); // dbg cnt:0x6400-0x648c
    __dra_set_backup_regs(dra, 3, DRA_SET_REG_BASE, DRA_SET_REG_BASE + 0x2fc); // glb_reg 0xe000-0xe2fc
}