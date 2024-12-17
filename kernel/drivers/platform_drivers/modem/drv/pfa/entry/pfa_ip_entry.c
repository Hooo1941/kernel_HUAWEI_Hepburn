/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/jhash.h>
#include <linux/kernel.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/netdevice.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_tuple.h>

#include <securec.h>
#include <bsp_pfa.h>

#include "pfa_direct_fw.h"
#include "pfa_ip_entry.h"
#include "pfa_dbg.h"
#include "pfa_port.h"


/*
 * debug code for pfa_ip_fw_add, the IP_FW_ADD_ERR_REC_MAX is the max code line
 * of function: pfa_ip_fw_add
 */
#define IP_FW_ADD_ERR_REC_MAX 300
#define IP_FW_ADD_REC_MASK (IP_FW_ADD_ERR_REC_MAX - 1)
#define IP_FW_ADD_ERR_REC_INIT() do { \
    /* idx 0 is record the base line */                                              \
    if (unlikely(NULL == g_sg_ip_fw_err_line)) {                                     \
        g_sg_ip_fw_err_line = kzalloc(g_sg_ip_fw_dft_max * sizeof(int), GFP_ATOMIC); \
        if (g_sg_ip_fw_err_line != NULL)                                             \
            g_sg_ip_fw_err_line[0] = __LINE__;                                       \
    }                                                                                \
} while (0)

#define IP_FW_ADD_ERR_REC_START() do { \
    int idx;                                                                   \
    /* the room of "err line rec" is ok */                                     \
    if (likely(g_sg_ip_fw_err_line != NULL)) {                                 \
        idx = __LINE__ - g_sg_ip_fw_err_line[0];                               \
        /* the room of "err line rec" is not enough */                         \
        if (unlikely(idx >= g_sg_ip_fw_dft_max)) {                             \
            /* expand the max and remalloc */                                  \
            int *rec_alloc;                                                    \
            g_sg_ip_fw_dft_max = idx + 100;                                    \
            rec_alloc = kzalloc(g_sg_ip_fw_dft_max * sizeof(int), GFP_ATOMIC); \
            if (rec_alloc != NULL) {                                           \
                /* kfree the old one and use the new room */                   \
                rec_alloc[0] = g_sg_ip_fw_err_line[0];                         \
                kfree((void *)g_sg_ip_fw_err_line);                            \
                g_sg_ip_fw_err_line = rec_alloc;                               \
                g_sg_ip_fw_err_line[idx]++;                                    \
            }                                                                  \
        } /* ok, record it */                                                  \
        else {                                                                 \
            g_sg_ip_fw_err_line[idx]++;                                        \
        }                                                                      \
    }                                                                          \
} while (0)


int bsp_pfa_update_ipfw_entry(struct wan_dev_info_s *wan_dev_info)
{
    struct pfa *pfa = &g_pfa;
    int ret = 0;

    if (pfa->direct_fw.dfw_flags.enable) {
        ret = pfa_direct_fw_record_wandev(wan_dev_info);
    }
    return ret;
}


void pfa_ip_fw_htab_set(void)
{
    struct pfa *pfa = &g_pfa;
    unsigned int size;

    size = sizeof(struct pfa_ip_fw_entry);
    size /= 0x4;

    pfa_writel(pfa->regs, PFA_HASH_BADDR_L, lower_32_bits(pfa->ipfw.hbucket_dma));
    pfa_writel(pfa->regs, PFA_HASH_BADDR_H, upper_32_bits(pfa->ipfw.hbucket_dma));
    pfa_writel(pfa->regs, PFA_HASH_WIDTH, size);
    pfa_writel(pfa->regs, PFA_HASH_DEPTH, pfa->ipfw.hlist_size);
    pfa_writel(pfa->regs, PFA_HASH_ZONE, pfa->ipfw.hzone);
    pfa_writel(pfa->regs, PFA_HASH_RAND, pfa->ipfw.hrand);
    // we are going to use default reg setting on PFA_HASH_L3_PROTO_OFFSET;
}
static void pfa_ip_empty_entry_free(struct pfa *pfa, unsigned int size_ipfw)
{
    if (pfa->ipfw.hbucket_empty) {
        dma_free_coherent(pfa->dev, size_ipfw * PFA_IPFW_HTABLE_EMPTY_SIZE, pfa->ipfw.hbucket_empty,
                          pfa->ipfw.hbucket_dma_empty);
        pfa->ipfw.hbucket_empty = NULL;
    }
    return;
}
static void pfa_ip_entry_free(struct pfa *pfa, unsigned int size_ipfw)
{
    int i = 0;
    struct pfa_ip_fw_entry *entry = NULL;

    while (i < pfa->ipfw.hlist_size) {
        entry = (struct pfa_ip_fw_entry *)(pfa->ipfw.hbucket + i * size_ipfw);
        if (entry->priv != NULL) {
            kfree(entry->priv);
        }
        i++;
    }

    if (pfa->ipfw.hbucket) {
        dma_free_coherent(pfa->dev, size_ipfw * pfa->ipfw.hlist_size, pfa->ipfw.hbucket, pfa->ipfw.hbucket_dma);

        pfa->ipfw.hbucket = NULL;
    }
    return;
}
int pfa_ip_empty_entry_init(struct pfa *pfa, unsigned int size_ipfw)
{
    struct pfa_ip_fw_entry *empty_entry = NULL;
    int ret;

    /* alloc pfa hash bucket empty for pfa switch of func */
    empty_entry = dma_alloc_coherent(pfa->dev, size_ipfw * PFA_IPFW_HTABLE_EMPTY_SIZE, &pfa->ipfw.hbucket_dma_empty,
                                     GFP_KERNEL);
    if (empty_entry == NULL) {
        PFA_ERR("alloc ip fw hbucket empty 1 entry failed\n");
        return -ENOMEM;
    }
    pfa->ipfw.hbucket_empty = empty_entry;

    ret = memset_s(empty_entry, sizeof(*empty_entry) * PFA_IPFW_HTABLE_EMPTY_SIZE, 0,
                   size_ipfw * PFA_IPFW_HTABLE_EMPTY_SIZE);
    if (ret) {
        PFA_ERR("memset_s ip fw hbucket empty  entry failed\n");
        pfa_ip_empty_entry_free(pfa, size_ipfw);
    }
    return ret;
}

static inline void pfa_ip_entry_defcfg(struct pfa *pfa, unsigned int size_ipfw)
{
    if (!pfa->ipfw.hlist_size) {
        pfa->ipfw.hlist_size = PFA_IPFW_HTABLE_SIZE;
    }

    pfa->ipfw.hrand = PFA_IPFW_HTABLE_RAND;
    pfa->ipfw.hzone = PFA_IPFW_HTABLE_ZONE;
    pfa->ipfw.hitem_width = size_ipfw; /* jiffies */
    return;
}

int pfa_ip_entry_table_init(struct pfa *pfa)
{
    int ret, i;
    unsigned int size_ipfw = sizeof(struct pfa_ip_fw_entry);

    pfa_ip_entry_defcfg(pfa, size_ipfw);
    ret = pfa_ip_empty_entry_init(pfa, size_ipfw);
    if (ret) {
        return ret;
    }

    /* alloc pfa hash bucket */
    pfa->ipfw.hbucket = dma_alloc_coherent(pfa->dev, size_ipfw * pfa->ipfw.hlist_size, &pfa->ipfw.hbucket_dma,
                                           GFP_KERNEL);
    if (!pfa->ipfw.hbucket) {
        PFA_ERR("alloc ip fw hbucket of %d entrys failed\n", pfa->ipfw.hlist_size);
        goto err_alloc_hbucket;
    }

    ret = memset_s(pfa->ipfw.hbucket, sizeof(struct pfa_ip_fw_entry) * pfa->ipfw.hlist_size, 0,
                   size_ipfw * pfa->ipfw.hlist_size);
    if (ret) {
        goto err_alloc_hbucket;
    }

    for (i = 0; i < pfa->ipfw.hlist_size; i++) {
        struct pfa_ip_fw_entry *entry = (struct pfa_ip_fw_entry *)(pfa->ipfw.hbucket + i * size_ipfw);

        entry->dead_timestamp = jiffies;
        entry->valid = 0;
        entry->priv = kzalloc(sizeof(struct pfa_ip_fw_entry_priv), GFP_KERNEL);
        if (entry->priv == NULL) {
            PFA_ERR("alloc ip fw hbucket priv failed\n");
            goto err_alloc_hslab;
        }
        entry->priv->entry = entry;
        entry->priv->dma = pfa->ipfw.hbucket_dma + i * size_ipfw;
        INIT_LIST_HEAD(&entry->priv->list);
    }

    /* alloc dma pool for ip fw entry */
    pfa->ipfw.hslab = (struct dma_pool *)dma_pool_create(dev_name(pfa->dev), pfa->dev,
                                                         sizeof(struct pfa_ip_fw_entry_dma), 4, PAGE_SIZE); /* 4 byte alignment */
    if (pfa->ipfw.hslab == NULL) {
        goto err_alloc_hslab;
    }

    INIT_LIST_HEAD(&pfa->ipfw.free_list);
    INIT_LIST_HEAD(&pfa->ipfw.wan_entry_list);

    spin_lock_init(&pfa->ipfw.lock);
    spin_lock_init(&pfa->ipfw.free_lock);

    pfa->ipfw.deadtime = 2; /* jiffies, 2 deadtime */
    return 0;

err_alloc_hslab:
    pfa_ip_entry_free(pfa, size_ipfw);
err_alloc_hbucket:
    pfa_ip_empty_entry_free(pfa, size_ipfw);
    return -1;
}

void pfa_ip_entry_res_free(struct pfa *pfa)
{
    if (pfa->ipfw.hslab != NULL) {
        dma_pool_destroy(pfa->ipfw.hslab);
        pfa->ipfw.hslab = NULL;
    }

    if (pfa->ipfw.hbucket_empty != NULL) {
        dma_free_coherent(pfa->dev, sizeof(struct pfa_ip_fw_entry) * pfa->ipfw.hlist_size, pfa->ipfw.hbucket,
                          pfa->ipfw.hbucket_dma);
        pfa->ipfw.hbucket_empty = NULL;
    }

    if (pfa->ipfw.hbucket != NULL) {
        dma_free_coherent(pfa->dev, sizeof(struct pfa_ip_fw_entry) * PFA_IPFW_HTABLE_EMPTY_SIZE,
                          pfa->ipfw.hbucket_empty, pfa->ipfw.hbucket_dma_empty);
        pfa->ipfw.hbucket = NULL;
    }
    return;
}

void pfa_ip_entry_table_exit(struct pfa *pfa)
{
    int i;
    unsigned long flags;
    dma_addr_t dma;
    struct pfa_ip_fw_entry *head = NULL;
    struct pfa_ip_fw_entry *pos = NULL;
    struct pfa_ip_fw_entry_priv *n = NULL;
    struct pfa_ip_fw_entry_priv *priv_pos = NULL;
    struct pfa_ip_fw_entry_priv *priv_head = NULL;
    spin_lock_irqsave(&pfa->ipfw.lock, flags);

    /* free node in free queue */
    for (i = 0; i < pfa->ipfw.hlist_size; i++) {
        head = (struct pfa_ip_fw_entry *)(pfa->ipfw.hbucket + i * sizeof(struct pfa_ip_fw_entry));
        priv_head = head->priv;
        list_for_each_entry_safe(priv_pos, n, &priv_head->list, list)
        {
            pos = priv_pos->entry;
            list_del_init(&pos->priv->list);
            list_del_init(&pos->priv->wan_list);
            dma = pos->priv->dma;
            kfree(pos->priv);
            dma_pool_free(pfa->ipfw.hslab, container_of(pos, struct pfa_ip_fw_entry_dma, entry), PFA_IP_FW_TO_DMA_OFFSET(dma));
        }
    }

    spin_unlock_irqrestore(&pfa->ipfw.lock, flags);

    spin_lock_irqsave(&pfa->ipfw.free_lock, flags);

    list_for_each_entry_safe(priv_pos, n, &pfa->ipfw.free_list, list)
    {
        pos = priv_pos->entry;
        dma = pos->priv->dma;
        kfree(pos->priv);
        dma_pool_free(pfa->ipfw.hslab, container_of(pos, struct pfa_ip_fw_entry_dma, entry), PFA_IP_FW_TO_DMA_OFFSET(dma));
    }

    spin_unlock_irqrestore(&pfa->ipfw.free_lock, flags);

    pfa_ip_entry_res_free(pfa);
    return;
}

MODULE_LICENSE("GPL");


