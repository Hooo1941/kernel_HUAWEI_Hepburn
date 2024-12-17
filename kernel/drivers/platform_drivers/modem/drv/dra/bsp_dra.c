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
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/io.h>
#include "bsp_dra.h"
#include "dra.h"
#include <bsp_print.h>
#include "product_config.h"
#include <linux/netdevice.h>
#include <osl_types.h>
#include <securec.h>

#define THIS_MODU mod_dra

#define dra_err(fmt, ...)                             \
    do {                                              \
        bsp_err("<%s>" fmt, __func__, ##__VA_ARGS__); \
    } while (0)

unsigned long long bsp_dra_skb_map(struct sk_buff *skb, unsigned int reserve)
{
    struct dra *dra = g_dra_ctx;

    if (skb == NULL || skb->len == 0) {
        return 0;
    }

    return dra_map_tcpip_skb(dra, skb, reserve);
}

unsigned long long mdrv_dra_skb_map(struct sk_buff *skb, unsigned int reserve)
{
    return bsp_dra_skb_map(skb, reserve);
}

struct sk_buff *bsp_dra_skb_unmap(unsigned long long addr)
{
    struct sk_buff *skb = NULL;
    dra_buf_t buf;
    struct dra *dra = g_dra_ctx;

    buf.addr = addr;
    if (addr == 0 || buf.bits.level > DRA_OPIPE_LEVEL_3) {
        return NULL;
    }

    if (buf.bits.from == DRA_SKB_FROM_TCP_IP) {
        dra->opipe_status[buf.bits.level].unmap_test_tcp_ip++;
        skb = dra_unmap_tcpip_skb(dra, buf);
    } else {
        skb = dra_unmap_skb(dra, buf, NULL);
    }
    return skb;
}

struct sk_buff *mdrv_dra_skb_unmap(unsigned long long addr)
{
    return bsp_dra_skb_unmap(addr);
}

inline struct sk_buff *bsp_dra_unmap(unsigned long long addr, unsigned long long *orig)
{
    dra_buf_t buf;

    buf.addr = addr;
    if (unlikely((addr == 0) || (orig == NULL) || buf.bits.level > DRA_OPIPE_LEVEL_3)) {
        return NULL;
    }

    return dra_unmap_skb(g_dra_ctx, buf, orig);
}

inline void dra_reset_skb(struct sk_buff *skb)
{
    skb->data = skb->head + DRA_SKB_RESERVE_SIZE;
    skb->len = 0;
    skb_reset_tail_pointer(skb);
}

struct sk_buff *bsp_dra_skb_alloc(unsigned int size)
{
    struct sk_buff *skb = NULL;
    unsigned long long addr;
    unsigned long long orig = 0;
    struct dra *dra = g_dra_ctx;

    addr = dra_ipipe_alloc(size);
    if (!addr) {
        return 0;
    }
    skb = dra_to_skb(dra, addr, &orig);
    if (skb == NULL) {
        return 0;
    }
    dra_reset_skb(skb);
    return skb;
}

void bsp_dra_reset_skb(struct sk_buff *skb)
{
    if (skb == NULL) {
        dra_err("input skb null\n");
        return;
    }
    dra_reset_skb(skb);
}

void bsp_dra_kfree_skb(struct sk_buff *skb)
{
    struct dra_tab *tab = NULL;
    struct dra *dra = g_dra_ctx;

    if (skb == NULL) {
        dra_err("skb is null\n");
        return;
    }

    tab = (struct dra_tab *)(skb->head + DRA_SKB_RESERVE_SIZE + DRA_OPIPE_LEVEL_1_SIZE);
    if (virt_addr_valid((uintptr_t)tab) && tab->magic == DRA_OWN) {
        dra_reset_skb(skb);
        dra_free_buf(dra, (dra_buf_t)tab->dra_addr);
    } else {
        dra->status.maybe_dobulefree_or_modified++;
        dra_err("skb maybe doublefree or modified\n");
    }
}

struct sk_buff *bsp_dra_get_skb(unsigned long long addr)
{
    struct sk_buff *skb = NULL;
    unsigned long long orig = 0;
    struct dra *dra = g_dra_ctx;

    if (addr == 0) {
        return NULL;
    }

    skb = dra_to_skb(dra, addr, &orig);

    return skb;
}

unsigned long long bsp_dra_get_dra(struct sk_buff *skb)
{
    struct dra_tab *tab = NULL;

    if (skb == NULL) {
        dra_err("input skb is null\n");
        return 0;
    }

    tab = (struct dra_tab *)(skb->head + DRA_SKB_RESERVE_SIZE + DRA_OPIPE_LEVEL_1_SIZE);
    if (virt_addr_valid((uintptr_t)tab) && tab->magic == DRA_OWN) {
        return tab->dra_addr;
    } else {
        dra_err("skb maybe modified or doublefree\n");
        return 0;
    }
}

struct sk_buff *bsp_dra_to_skb(unsigned long long changed, unsigned long long *orig)
{
    struct sk_buff *skb = NULL;
    struct dra *dra = g_dra_ctx;

    if (orig == NULL) {
        return NULL;
    }

    *orig = 0;
    skb = dra_to_skb(dra, changed, orig);

    return skb;
}
unsigned long long bsp_dra_alloc(unsigned int size)
{
    return dra_ipipe_alloc(size);
}

struct sk_buff *bsp_dra_alloc_skb(unsigned int size, unsigned long long *orig)
{
    struct sk_buff *skb = NULL;
    unsigned long long addr;
    struct dra *dra = g_dra_ctx;
    if (orig == NULL) {
        return NULL;
    }
    *orig = 0;
    addr = dra_ipipe_alloc(size);
    if (!addr) {
        return 0;
    }

    skb = dra_to_skb(dra, addr, orig);
    if (skb == NULL) {
        return 0;
    }

    dra_reset_skb(skb);
    return skb;
}

void bsp_dra_free(unsigned long long changed)
{
    dra_buf_t buf;
    struct dra *dra = g_dra_ctx;

    buf.addr = changed;
    dra_free_buf(dra, buf);
}
unsigned long long bsp_dra_to_phy(unsigned long long addr)
{
    dra_buf_t buf;

    buf.addr = addr;

    return buf.bits.phyaddr;
}

unsigned int bsp_dra_get_addr_type(unsigned long long addr)
{
    dra_buf_t buf;
    buf.addr = addr;
    if (buf.bits.from) {
        return DRA_SKB_FROM_OWN;
    } else {
        return DRA_SKB_FROM_TCP_IP;
    }
}
void mdrv_dra_skb_free(struct sk_buff *skb)
{
    return bsp_dra_kfree_skb(skb);
}

unsigned int bsp_dra_set_adqbase(unsigned long long adqbase, unsigned int write, unsigned int depth, unsigned int id)
{
    struct dra *dra = g_dra_ctx;

    if (adqbase == 0 || write == 0 || depth == 0) {
        dra_err("adqbase=%llx, write=%x, depth=%d, id=%u\n", adqbase, write, depth, id);
        return 0;
    }
    return dra_set_adqbase(dra, adqbase, write, depth, id);
}

unsigned int bsp_dra_set_rlsbase(unsigned long long rlsbase, unsigned int update_ptr, unsigned int depth, unsigned int id)
{
    struct dra *dra = g_dra_ctx;

    if (rlsbase == 0 || update_ptr == 0 || depth == 0) {
        dra_err("rlsbase=%llx, update_ptr=%x, depth=%d, id=%u\n", rlsbase, update_ptr, depth, id);
        return 0;
    }
    return dra_set_rlsbase(dra, rlsbase, update_ptr, depth, id);
}

int bsp_dra_enable_rls(unsigned int idx, int enable)
{
    return dra_enable_rls(g_dra_ctx, idx, enable);
}

int bsp_dra_enable_alloc(unsigned int idx, int enable)
{
    return dra_enable_alloc(g_dra_ctx, idx, enable);
}

void mdrv_dra_reset_skb(struct sk_buff *skb)
{
    return bsp_dra_reset_skb(skb);
}

struct sk_buff *mdrv_dra_get_skb(unsigned long long addr)
{
    return bsp_dra_get_skb(addr);
}

unsigned long long mdrv_dra_alloc(unsigned int size)
{
    return bsp_dra_alloc(size);
}

void mdrv_dra_free(unsigned long long changed)
{
    return bsp_dra_free(changed);
}

EXPORT_SYMBOL(bsp_dra_skb_alloc);
EXPORT_SYMBOL(bsp_dra_skb_unmap);
EXPORT_SYMBOL(bsp_dra_kfree_skb);
EXPORT_SYMBOL(bsp_dra_skb_map);
EXPORT_SYMBOL(bsp_dra_alloc);
EXPORT_SYMBOL(bsp_dra_free);
EXPORT_SYMBOL(bsp_dra_get_addr_type);
EXPORT_SYMBOL(bsp_dra_get_dra);
EXPORT_SYMBOL(bsp_dra_get_skb);
EXPORT_SYMBOL(bsp_dra_to_phy);
EXPORT_SYMBOL(bsp_dra_unmap);
EXPORT_SYMBOL(bsp_dra_alloc_skb);
EXPORT_SYMBOL(bsp_dra_reset_skb);
EXPORT_SYMBOL(bsp_dra_to_skb);
EXPORT_SYMBOL(mdrv_dra_skb_free);
EXPORT_SYMBOL(bsp_dra_set_rlsbase);
EXPORT_SYMBOL(bsp_dra_enable_rls);
EXPORT_SYMBOL(bsp_dra_enable_alloc);
EXPORT_SYMBOL(mdrv_dra_reset_skb);
EXPORT_SYMBOL(mdrv_dra_get_skb);
EXPORT_SYMBOL(mdrv_dra_alloc);
EXPORT_SYMBOL(mdrv_dra_free);
EXPORT_SYMBOL(mdrv_dra_skb_map);
EXPORT_SYMBOL(mdrv_dra_skb_unmap);