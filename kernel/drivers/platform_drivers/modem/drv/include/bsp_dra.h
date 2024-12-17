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
 */

#ifndef _BSP_DRA_H_
#define _BSP_DRA_H_
#include <product_config.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#ifdef __cplusplus
extern "C" {
#endif

#define DRA_SKB_FROM_TCP_IP 0
#define DRA_SKB_FROM_OWN 1

enum dra_ipipe_idx {
    DRA_IPIPE_FOR_PFA_2K = 0,
    DRA_IPIPE_FOR_PFA_TFT_2K = 1,
    DRA_IPIPE_FOR_HIMAC0_2K = 2,
    DRA_IPIPE_FOR_HIMAC1_2K = 3,
    DRA_IPIPE_FOR_PFA_TFT_512 = 4,
    DRA_IPIPE_FOR_ACPU_2K = 5,
};

enum dra_rls_idx {
    DRA_RLS_FOR_HIMAC0_2K = 0,
    DRA_RLS_FOR_HIMAC1_2K,
};

enum dra_opipe_level_size {
    DRA_OPIPE_LEVEL_0_SIZE = 128U,  /* 128B, reverse 128B */
    DRA_OPIPE_LEVEL_1_SIZE = 1536U, /* 1536B reverse 128B */
    DRA_OPIPE_LEVEL_2_SIZE = 4096U, /* 4KB no reverse */
    DRA_OPIPE_LEVEL_3_SIZE = 9216U, /* 9KB reverse 128B */
    DRA_OPIPE_LEVEL_4_SIZE = 128U, /* C mem */
    DRA_OPIPE_LEVEL_5_SIZE = 1568U,  /* C mem */
    DRA_OPIPE_LEVEL_6_SIZE = 9216U,  /* C mem */
    DRA_OPIPE_LEVEL_7_SIZE = 4096U,  /* A mem */
    DRA_OPIPE_LEVEL_8_SIZE = 4096U,  /* A mem */
    DRA_OPIPE_LEVEL_INVALID_SIZE = 0xdeadbeefU
};

#define DRA_OPIPE_LEVEL_0_BUFF_SIZE 1024U
#define DRA_OPIPE_LEVEL_1_BUFF_SIZE 2048UL
#define DRA_OPIPE_LEVEL_3_BUFF_SIZE 9728UL

enum dra_opipe_level_idx {
    DRA_OPIPE_LEVEL_0 = 0U,
    DRA_OPIPE_LEVEL_1 = 1U,
    DRA_OPIPE_LEVEL_2 = 2U,
    DRA_OPIPE_LEVEL_3 = 3U,
    DRA_OPIPE_LEVEL_4 = 4U,
    DRA_OPIPE_LEVEL_5 = 5U,
    DRA_OPIPE_LEVEL_6 = 6U,
    DRA_OPIPE_LEVEL_7 = 7U,
    DRA_OPIPE_LEVEL_8 = 8U,
    DRA_OPIPE_LEVEL_INVALID
};

typedef union {
    struct {
        unsigned long long phyaddr : 36;
        unsigned long long sc_mode : 4;
        unsigned long long offset : 6;
        unsigned long long idx : 11;
        unsigned long long crc : 1;
        unsigned long long from : 2;
        unsigned long long level : 4;
    } bits_1;
    struct {
        unsigned long long phyaddr : 36;
        unsigned long long sc_mode : 4;
        unsigned long long idx : 17;
        unsigned long long crc : 1;
        unsigned long long from : 2;
        unsigned long long level : 4;
    } bits;
    unsigned long long addr;
} dra_buf_t;

#ifdef CONFIG_DRA
unsigned int bsp_dra_set_adqbase(unsigned long long adqbase, unsigned int write, unsigned int depth,
    unsigned int ipipe);
unsigned int bsp_dra_set_rlsbase(unsigned long long rlsbase, unsigned int write, unsigned int depth,
    unsigned int id);
unsigned long long bsp_dra_skb_map(struct sk_buff *skb, unsigned int reserve);
struct sk_buff *bsp_dra_skb_unmap(unsigned long long dradr);
struct sk_buff *bsp_dra_skb_alloc(unsigned int size);
void bsp_dra_kfree_skb(struct sk_buff *skb);
struct sk_buff *bsp_dra_get_skb(unsigned long long addr);
unsigned long long bsp_dra_get_dra(struct sk_buff *skb);
void bsp_dra_reset_skb(struct sk_buff *skb);
struct sk_buff *bsp_dra_to_skb(unsigned long long changed, unsigned long long *orig);
struct sk_buff *bsp_dra_alloc_skb(unsigned int size, unsigned long long *orig);
void bsp_dra_free(unsigned long long orig);
unsigned long long bsp_dra_alloc(unsigned int size);
unsigned long long bsp_dra_to_phy(unsigned long long addr);
unsigned int bsp_dra_get_addr_type(unsigned long long addr);
int bsp_dra_reinit(void);
unsigned int bsp_dra_get_cur_mem(void);
inline struct sk_buff *bsp_dra_unmap(unsigned long long addr, unsigned long long *orig);
int bsp_dra_enable_rls(unsigned int idx, int enable);
int bsp_dra_enable_alloc(unsigned int idx, int enable);
#else
static inline unsigned int bsp_dra_set_adqbase(unsigned long long adqbase, unsigned int write, unsigned int depth,
    unsigned int ipipe)
{
    return 0;
}

static inline unsigned int bsp_dra_set_rlsbase(unsigned long long rlsbase, unsigned int write, unsigned int depth,
    unsigned int id)
{
    return 0;
}

static inline void bsp_dra_enable_ipipe(unsigned int ipipe, int enable)
{
    return;
}

static inline unsigned long long bsp_dra_skb_map(struct sk_buff *skb, unsigned int reserve)
{
    return 0;
}

static inline struct sk_buff *bsp_dra_skb_unmap(unsigned long long dradr)
{
    return NULL;
}

static inline struct sk_buff *bsp_dra_skb_alloc(unsigned int size)
{
    return NULL;
}

static inline void bsp_dra_kfree_skb(struct sk_buff *skb)
{
    return;
}

static inline struct sk_buff *bsp_dra_get_skb(unsigned long long addr)
{
    return NULL;
}

static inline unsigned long long bsp_dra_get_dra(struct sk_buff *skb)
{
    return 0;
}

static inline void bsp_dra_reset_skb(struct sk_buff *skb)
{
    return;
}

static inline struct sk_buff *bsp_dra_to_skb(unsigned long long changed, unsigned long long *orig)
{
    return NULL;
}

static inline struct sk_buff *bsp_dra_alloc_skb(unsigned int size, unsigned long long *orig)
{
    return NULL;
}

static inline void bsp_dra_free(unsigned long long orig)
{
    return;
}

static inline unsigned long long bsp_dra_alloc(unsigned int size)
{
    return 0;
}

static inline unsigned long long bsp_dra_to_phy(unsigned long long addr)
{
    return 0;
}

static inline unsigned int bsp_dra_get_addr_type(unsigned long long addr)
{
    return 0;
}

static inline int bsp_dra_reinit(void)
{
    return 0;
}

static inline struct sk_buff *bsp_dra_unmap(unsigned long long addr, unsigned long long *orig)
{
    return NULL;
}

static inline unsigned int bsp_dra_get_cur_mem(void)
{
    return 0;
}
static inline int bsp_dra_enable_rls(unsigned int idx, int enable)
{
    return 0;
}

static inline int bsp_dra_enable_alloc(unsigned int idx, int enable)
{
    return 0;
}

#endif

#ifdef __cplusplus
}
#endif
#endif
