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

#include <linux/kernel.h>
#include <securec.h>
#include "pfa_mac_entry.h"
#include "pfa_dbg.h"



int pfa_mac_entry_table_init(struct pfa *pfa)
{
    /* alloc pool for mac fw entry */
    pfa->macfw.slab = (struct kmem_cache *)kmem_cache_create(dev_name(pfa->dev), sizeof(struct pfa_mac_fw_entry_ext),
                                                             0, SLAB_HWCACHE_ALIGN, NULL);

    if (pfa->macfw.slab == NULL) {
        PFA_ERR("alloc mac fw slab failed\n");
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&pfa->macfw.pending);
    INIT_LIST_HEAD(&pfa->macfw.backups);
    spin_lock_init(&pfa->macfw.lock);
    PFA_INFO("[init] pfa mac entry table init success\n");
    return 0;
}

void pfa_mac_entry_table_exit(struct pfa *pfa)
{
    unsigned long flags;
    struct pfa_mac_fw_entry_ext *pos = NULL;
    struct pfa_mac_fw_entry_ext *n = NULL;

    /* free node in pending queue */
    spin_lock_irqsave(&pfa->macfw.lock, flags);
    list_for_each_entry_safe(pos, n, &pfa->macfw.pending, list)
    {
        list_del_init(&pos->list);
        kmem_cache_free(pfa->macfw.slab, (void *)pos);
    }

    list_for_each_entry_safe(pos, n, &pfa->macfw.backups, list)
    {
        list_del_init(&pos->list);
        kmem_cache_free(pfa->macfw.slab, (void *)pos);
    }
    spin_unlock_irqrestore(&pfa->macfw.lock, flags);

    if (pfa->macfw.slab != NULL) {
        kmem_cache_destroy(pfa->macfw.slab);
        pfa->macfw.slab = NULL;
    }
}

void pfa_config_eth_vlan_tag_sel(unsigned int flag)
{
    struct pfa *pfa = &g_pfa;

    pfa->hal->config_eth_vlan_tag_sel(pfa, flag);
}

bool mdrv_pfa_macfw_find_mac(const unsigned char *mac)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_mac_fw_entry_ext *pos = NULL;
    struct pfa_mac_fw_entry_ext *n = NULL;
    unsigned int mac_hi, mac_lo;

    if (mac == NULL) {
        return false;
    }
    mac_hi = (mac[0]) | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24); /* 2,3,8,16,24 mac addr shift */
    mac_lo = (mac[4]) | (mac[5] << 8); /* 4,5,8 mac addr shift */

    list_for_each_entry_safe(pos, n, &pfa->macfw.backups, list)
    {
        if (pos->ent.mac_hi == mac_hi && pos->ent.mac_lo == mac_lo) {
            return true;
        }
    }

    return false;
}


MODULE_LICENSE("GPL");


