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
#ifndef __DRA_DEF_H__
#define __DRA_DEF_H__

#define DRA_SET_REG_BASE 0xE000
#define DRA_VERSION (DRA_SET_REG_BASE + 0x0008)
#define DRA_BYPASS_CTRL (DRA_SET_REG_BASE + 0x0014)
#define DRA_LP_EN (DRA_SET_REG_BASE + 0x18)
#define DRA_LP_STAT (DRA_SET_REG_BASE + 0x1C)
#define DRA_AUTOCCLK (DRA_SET_REG_BASE + 0x28)
#define DRA_DIV_32K_CFG (DRA_SET_REG_BASE + 0x50)
#define DRA_TIME_SEL (DRA_SET_REG_BASE + 0x100)
#define DRA_TRANS_DATA_LEN_CFG (DRA_SET_REG_BASE + 0x104)
#define DRA_TRIGGER_NUM_CFG (DRA_SET_REG_BASE + 0x108)
#define DRA_CORE0_INT_SUPRESS_CTRL (DRA_SET_REG_BASE + 0x0120)
#define DRA_CORE1_INT_SUPRESS_CTRL (DRA_SET_REG_BASE + 0x0124)
#define DRA_DBG_INT_SUPRESS_CTRL (DRA_SET_REG_BASE + 0x0128)
#define DRA_ARUSER_CTRL (DRA_SET_REG_BASE + 0x0130)
#define DRA_AWUSER_CTRL (DRA_SET_REG_BASE + 0x0134)
#define DRA_RLS_EMPTY_0 (DRA_SET_REG_BASE + 0x0138)
#define DRA_RLS_EMPTY_1 (DRA_SET_REG_BASE + 0x013C)
#define DRA_RLS_FULL_0 (DRA_SET_REG_BASE + 0x0140)
#define DRA_RLS_FULL_1 (DRA_SET_REG_BASE + 0x0144)
#define DRA_RLS_BUSY_0 (DRA_SET_REG_BASE + 0x0148)
#define DRA_RLS_BUSY_1 (DRA_SET_REG_BASE + 0x014C)
#define DRA_ALLOC_EMPTY_0 (DRA_SET_REG_BASE + 0x0150)
#define DRA_ALLOC_EMPTY_1 (DRA_SET_REG_BASE + 0x0154)
#define DRA_ALLOC_FULL_0 (DRA_SET_REG_BASE + 0x0158)
#define DRA_ALLOC_FULL_1 (DRA_SET_REG_BASE + 0x015C)
#define DRA_ALLOC_BUSY_0 (DRA_SET_REG_BASE + 0x0160)
#define DRA_ALLOC_BUSY_1 (DRA_SET_REG_BASE + 0x0164)
#define DRA_HAC_GIF_128SPLIT (DRA_SET_REG_BASE + 0x0170)
#define DRA_HAC_GIF_ID (DRA_SET_REG_BASE + 0x0180)
#define DRA_HAC_GIF_MAX_LEN (DRA_SET_REG_BASE + 0x0184)
#define DRA_HAC_GIF_PRIOR (DRA_SET_REG_BASE + 0x0188)
#define DRA_HAC_GIF_OS_CTRL (DRA_SET_REG_BASE + 0x18C)
#define DRA_IDX_CHK_EN(level) (DRA_SET_REG_BASE + (0x200 + 0x4 * (level)))
#define DRA_IDX_CHK_MIN_CFG(level) (DRA_SET_REG_BASE + (0x240 + 0x4 * (level)))
#define DRA_IDX_CHK_MAX_CFG(level) (DRA_SET_REG_BASE + (0x280 + 0x4 * (level)))
#define DRA_LEVEL_CHK_EN (DRA_SET_REG_BASE + 0x2C0)
#define DRA_ERROR_BD_0 (DRA_SET_REG_BASE + 0x2C8)
#define DRA_ERROR_BD_1 (DRA_SET_REG_BASE + 0x2CC)
#define DRA_RLS_ERROR_IDX (DRA_SET_REG_BASE + 0x2D4)
#define DRA_DBG_INT_RAW_STATE (DRA_SET_REG_BASE + 0x2E0)
#define DRA_DBG_INT_MASK (DRA_SET_REG_BASE + 0x2E4)
#define DRA_DBG_INT_STATE (DRA_SET_REG_BASE + 0x2E8)
#define DRA_DBG_INT_CLR (DRA_SET_REG_BASE + 0x2EC)
#define DRA_HAC_DATA_DEBUG (DRA_SET_REG_BASE + 0x904)
#define DRA_HAC_PUSH_DEBUG (DRA_SET_REG_BASE + 0x908)
#define DRA_HAC_GIF_AXI_ST (DRA_SET_REG_BASE + 0x90C)

#define DRA_SECCFG_BASE 0xC000
#define DRA_ALLOC_CH_REG_SECCTRL0_N (DRA_SECCFG_BASE + 0x0)
#define DRA_ALLOC_CH_REG_SECCTRL1_N (DRA_SECCFG_BASE + 0x4)
#define DRA_ALLOC_CH_REG_SECCTRL2_N (DRA_SECCFG_BASE + 0x8)
#define DRA_ALLOC_CH_REG_SECCTRL3_N (DRA_SECCFG_BASE + 0xC)
#define DRA_ALLOC_CH_INSTR_SECCTRL0_N (DRA_SECCFG_BASE + 0x40)
#define DRA_ALLOC_CH_INSTR_SECCTRL1_N (DRA_SECCFG_BASE + 0x44)
#define DRA_ALLOC_CH_INSTR_SECCTRL2_N (DRA_SECCFG_BASE + 0x48)
#define DRA_ALLOC_CH_INSTR_SECCTRL3_N (DRA_SECCFG_BASE + 0x4C)
#define DRA_RLS_CH_REG_SECCTRL0_N (DRA_SECCFG_BASE + 0x100)
#define DRA_RLS_CH_REG_SECCTRL1_N (DRA_SECCFG_BASE + 0x104)
#define DRA_RLS_CH_REG_SECCTRL2_N (DRA_SECCFG_BASE + 0x108)
#define DRA_RLS_CH_REG_SECCTRL3_N (DRA_SECCFG_BASE + 0x10C)
#define DRA_RLS_CH_INSTR_SECCTRL0_N (DRA_SECCFG_BASE + 0x140)
#define DRA_RLS_CH_INSTR_SECCTRL1_N (DRA_SECCFG_BASE + 0x144)
#define DRA_RLS_CH_INSTR_SECCTRL2_N (DRA_SECCFG_BASE + 0x148)
#define DRA_RLS_CH_INSTR_SECCTRL3_N (DRA_SECCFG_BASE + 0x14C)
#define DRA_DDR_BUF_REG_SEC_CTRL (DRA_SECCFG_BASE + 0x200)
#define DRA_DDR_BUF_INSTR_SEC_CTRL (DRA_SECCFG_BASE + 0x204)
#define DRA_RECYCLE_POOL_REG_SEC_CTRL (DRA_SECCFG_BASE + 0x300)
#define DRA_RECYCLE_POOL_INSTR_SEC_CTRL (DRA_SECCFG_BASE + 0x304)

#define DRA_RLS_CH_BASE_ADDR_L(ch) (0x0 + (ch)*0x40)
#define DRA_RLS_CH_BASE_ADDR_H(ch) (0x4 + 0x40 * (ch))
#define DRA_RLS_CH_DEPTH(ch) (0x8 + 0x40 * (ch))
#define DRA_RLS_CH_WPTR(ch) (0xC + 0x40 * (ch))
#define DRA_RLS_CH_RPTR(ch) (0x10 + 0x40 * (ch))
#define DRA_RLS_CH_PTR_ADDR_L(ch) (0x14 + 0x40 * (ch))
#define DRA_RLS_CH_PTR_ADDR_H(ch) (0x18 + 0x40 * (ch))
#define DRA_RLS_CH_THRH(ch) (0x20 + 0x40 * (ch))
#define DRA_RLS_CH_CTRL(ch) (0x24 + 0x40 * (ch))
#define DRA_RLS_CH_TIME_OUT_CFG(ch) (0x30 + 0x40 * (ch))
#define DRA_RLS_CH_FLUSH_EN_P(ch) (0x34 + 0x40 * (ch))
#define DRA_RLS_CH_EN(ch) (0x38 + 0x40 * (ch))
#define DRA_ALLOC_CH_BASE_ADDR_L(ch) (0x2000 + 0x40 * (ch))
#define DRA_ALLOC_CH_BASE_ADDR_H(ch) (0x2004 + 0x40 * (ch))
#define DRA_ALLOC_CH_DEPTH(ch) (0x2008 + 0x40 * (ch))
#define DRA_ALLOC_CH_WPTR(ch) (0x200C + 0x40 * (ch))
#define DRA_ALLOC_CH_RPTR(ch) (0x2010 + 0x40 * (ch))
#define DRA_ALLOC_CH_PTR_ADDR_L(ch) (0x2014 + 0x40 * (ch))
#define DRA_ALLOC_CH_PTR_ADDR_H(ch) (0x2018 + 0x40 * (ch))
#define DRA_ALLOC_CH_CTRL(ch) (0x2024 + 0x40 * (ch))
#define DRA_ALLOC_CH_EN(ch) (0x2038 + 0x40 * (ch))
#define DRA_DDR_BUF_BASE_ADDR_L(ch) (0x4000 + (ch)*0x40)
#define DRA_DDR_BUF_BASE_ADDR_H(ch) (0x4004 + (ch)*0x40)
#define DRA_DDR_BUF_DEPTH(ch) (0x4008 + (ch)*0x40)
#define DRA_DDR_BUF_WPTR(ch) (0x400C + (ch)*0x40)
#define DRA_DDR_BUF_RPTR(ch) (0x4010 + (ch)*0x40)
#define DRA_DDR_BUF_DOWN_THRH(ch) (0x4018 + (ch)*0x40)
#define DRA_DDR_BUF_WPTR_INTRL(ch) (0x401C + (ch)*0x40)
#define DRA_DDR_BUF_RPTR_INTRL(ch) (0x4020 + (ch)*0x40)
#define DRA_RECYCLE_POOL_BASE_ADDR_L(ch) (0x5000 + (ch)*0x40)
#define DRA_RECYCLE_POOL_BASE_ADDR_H(ch) (0x5004 + (ch)*0x40)
#define DRA_RECYCLE_POOL_DEPTH(ch) (0x5008 + (ch)*0x40)
#define DRA_RECYCLE_POOL_WPTR(ch) (0x500C + (ch)*0x40)
#define DRA_RECYCLE_POOL_RPTR(ch) (0x5010 + (ch)*0x40)
#define DRA_RECYCLE_POOL_UP_THRH(ch) (0x5014 + (ch)*0x40)
#define DRA_RECYCLE_POOL_WPTR_INTRL(ch) (0x501C + (ch)*0x40)
#define DRA_CORE0_BIG_POOL_INT_MASK(ch) (0x6000 + (ch)*0x4)
#define DRA_CORE0_RECYCLE_POOL_INT_MASK (0x6008)
#define DRA_CORE1_BIG_POOL_INT_MASK(ch) (0x600C + (ch)*0x4)
#define DRA_CORE1_RECYCLE_POOL_INT_MASK (0x6014)
#define DRA_BIG_POOL_INT_RAW(ch) (0x6040 + (ch)*0x4)
#define DRA_RECYCLE_POOL_INT_RAW (0x6048)
#define DRA_CORE0_BIG_POOL_INT_STATE(ch) (0x604C + (ch)*0x4)
#define DRA_CORE0_RECYCLE_POOL_INT_STATE (0x6054)
#define DRA_CORE1_BIG_POOL_INT_STATE(ch) (0x6058 + (ch)*0x4)
#define DRA_CORE1_RECYCLE_POOL_INT_STATE 0x6060
#define DRA_DBG_CNT_CTRL (0x6200)
#define DRA_DBG_CNT_SEL(ch) (0x6300 + 0x4 * (ch))
#define DRA_DBG_CNT(ch) (0x6400 + 0x4 * (ch))

union dra_alloc_ch_ctrl_u {
    unsigned int val;
    struct {
        unsigned int rsv0 : 1;
        unsigned int alloc_ch_push_ptr_en : 1;
        unsigned int alloc_ch_busy : 1;
        unsigned int rsv1 : 1;
        unsigned int alloc_ch_prio : 4;
        unsigned int alloc_ch_level_sel : 4;
        unsigned int rsv2 : 4;
        unsigned int ch_full : 1;
        unsigned int ch_empty : 1;
        unsigned int rsv3 : 14;
    } bits;
};

union dra_rls_ch_ctrl_u {
    unsigned int val;
    struct {
        unsigned int rls_ch_des_sel : 1;
        unsigned int rls_ch_push_ptr_en : 1;
        unsigned int rls_ch_busy : 1;
        unsigned int rls_ch_recycle_pool_sel : 1;
        unsigned int rls_ch_prio : 4;
        unsigned int rls_ch_level_sel : 4;
        unsigned int rls_ch_big_release_pool_sel : 1;
        unsigned int rsv : 3;
        unsigned int ch_full : 1;
        unsigned int ch_empty : 1;
        unsigned int rsv3 : 14;
    } bits;
};

union dra_dbg_cnt_sel_u {
    unsigned int val;
    struct {
        unsigned int dbg_cnt_chn_sel : 6;
        unsigned int rsv0 : 2;
        unsigned int dbg_cnt_cate_sel : 3;
        unsigned int rsv1 : 21;
    } bits;
};

#define DRA_DBG_CATE_SINGLE_RLS (0)
#define DRA_DBG_CATE_SINGLE_ALLOC (1)
#define DRA_DBG_CATE_SINGLE_RECYCLE (2)
#define DRA_DBG_CATE_SINGLE_DDR_BUF (3)
#define DRA_DBG_CATE_ALL_RLS (4)
#define DRA_DBG_CATE_ALL_ALLOC (5)

enum dra_dbg_cnt {
    DRA_DBG_RECYCLE0,

    DRA_DBG_DDRBUF1, // 位置不能动
    DRA_DBG_DDRBUF2,
    DRA_DBG_DDRBUF3,

    DRA_DBG_BIGPOOL1,
    DRA_DBG_BIGPOOL2,
    DRA_DBG_BIGPOOL3,

    DRA_DBG_RLS_BASE = 7,
    DRA_DBG_ADQ_BASE = 11,
    DRA_DBG_CNT_TSP_BASE = 15,

    DRA_DBG_MAX = 35,
};

#endif
