/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include "coresight_trace_mgr.h"
#include "coresight_replicator.h"
#include "coresight_errno.h"
struct coresight_replicator_info g_coresight_replicator;

static int coresight_replicator_sel_single(void *replicator_base, u32 chnl0, u32 chnl1)
{
    if (replicator_base == NULL) {
        return CORESIGHT_REP_BASE_ERR;
    }
    CORESIGHT_TMC_UNLOCK((uintptr_t)replicator_base);
    chnl0 &= 0xFF;
    chnl1 &= 0xFF;

    CORESIGHT_WRITEL((uintptr_t)replicator_base, chnl0, CORESIGHT_REPLICATOR_FLT0);
    CORESIGHT_WRITEL((uintptr_t)replicator_base, chnl1, CORESIGHT_REPLICATOR_FLT1);

    CORESIGHT_TMC_LOCK((uintptr_t)replicator_base);

    return 0;
}
int coresight_replicator_sel(void)
{
    u32 i;
    if (g_coresight_replicator.status != REPLICATOR_INT_OK) {
        return CORESIGHT_REP_STATE_ERR;
    }
    for (i = 0; i < g_coresight_replicator.num; i++) {
        if (coresight_replicator_sel_single(g_coresight_replicator.replicator[i].base,
            g_coresight_replicator.replicator[i].chnnl0, g_coresight_replicator.replicator[i].chnnl1)) {
            return CORESIGHT_REP_SEL_ERR;
        }
    }
    return BSP_OK;
}
int coresight_replicator_init(void)
{
    u32 i = 0;
    u32 num = 0;
    u32 chnnl = 0;
    device_node_s *child = NULL;
    device_node_s *node = bsp_dt_find_node_by_path("/coresight_replicator");
    if (node == NULL) {
        return BSP_OK;
    }
    (void)memset_s((void *)(uintptr_t)&g_coresight_replicator, sizeof(g_coresight_replicator), 0,
        sizeof(g_coresight_replicator));
    if (bsp_dt_property_read_u32(node, "num", &num) || num > CORESIGHT_MAX_REPLICATOR_NUM) {
        return CORESIGHT_REP_NO_NUM;
    }
    g_coresight_replicator.num = num;
    g_coresight_replicator.replicator =
        (struct coresight_replicator_stru *)kmalloc(num * sizeof(struct coresight_replicator_stru), GFP_KERNEL);
    if (g_coresight_replicator.replicator == NULL) {
        return CORESIGHT_REP_MALLOC_ERR;
    }
    bsp_dt_for_each_child_of_node(node, child)
    {
        if (i >= num) {
            kfree(g_coresight_replicator.replicator);
            g_coresight_replicator.replicator = NULL;
            return CORESIGHT_REP_OVERFLOW;
        }
        g_coresight_replicator.replicator[i].base = (void *)(uintptr_t)bsp_dt_iomap(child, 0);
        if (g_coresight_replicator.replicator[i].base == NULL) {
            kfree(g_coresight_replicator.replicator);
            g_coresight_replicator.replicator = NULL;
            return CORESIGHT_REP_MAP_ERR;
        }
        if (bsp_dt_property_read_u32(child, "chnnl0", &chnnl) || chnnl > 0xFFF) {
            kfree(g_coresight_replicator.replicator);
            g_coresight_replicator.replicator = NULL;
            return CORESIGHT_REP_CHNNL0_ERR;
        }
        g_coresight_replicator.replicator[i].chnnl0 = chnnl;
        if (bsp_dt_property_read_u32(child, "chnnl1", &chnnl) || chnnl > 0xFFF) {
            kfree(g_coresight_replicator.replicator);
            g_coresight_replicator.replicator = NULL;
            return CORESIGHT_REP_CHNNL1_ERR;
        }
        g_coresight_replicator.replicator[i].chnnl1 = chnnl;
        i = i + 1;
    }
    g_coresight_replicator.status = REPLICATOR_INT_OK;
    return BSP_OK;
}
