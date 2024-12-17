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
#include "coresight_funnel.h"
#include "coresight_errno.h"
struct coresight_funnel_info g_coresight_funnel;
static void coresight_funnel_single_enable(void *funnel_base, u32 chnls)
{
    unsigned int functl;
    if (funnel_base == NULL) {
        return;
    }

    CORESIGHT_TMC_UNLOCK((uintptr_t)funnel_base);
    chnls &= 0xFF;
    functl = CORESIGHT_READL((uintptr_t)funnel_base, CORESIGHT_FUNNEL_FUNCTL);
    functl &= ~CORESIGHT_FUNNEL_HOLDTIME_MASK;
    functl |= CORESIGHT_FUNNEL_HOLDTIME;
    functl |= chnls;
    CORESIGHT_WRITEL((uintptr_t)funnel_base, functl, CORESIGHT_FUNNEL_FUNCTL);

    CORESIGHT_TMC_LOCK((uintptr_t)funnel_base);
    return;
}
int coresight_funnel_enable(void)
{
    u32 num = g_coresight_funnel.num;
    u32 i, chnnl;
    void *base = NULL;
    if (g_coresight_funnel.status != FUNNEL_INT_OK) {
        return CORESIGHT_FUNNEL_ENABLE_FAIL;
    }
    for (i = 0; i < num; i++) {
        base = g_coresight_funnel.funnel[i].base;
        chnnl = g_coresight_funnel.funnel[i].chnnl;
        coresight_funnel_single_enable(base, chnnl);
    }
    return BSP_OK;
}
int coresight_top_funnel_init(void)
{
    u32 i = 0;
    u32 num = 0;
    u32 chnnl = 0;
    device_node_s *child = NULL;
    device_node_s *node = bsp_dt_find_node_by_path("/coresight_funnel");
    if (node == NULL) {
        return CORESIGHT_MDM_FUNNEL_NOFOUND;
    }
    (void)memset_s((void *)(uintptr_t)&g_coresight_funnel, sizeof(g_coresight_funnel), 0, sizeof(g_coresight_funnel));
    if (bsp_dt_property_read_u32(node, "num", &num) || num > CORESIGHT_MAX_FUNNEL_NUM) {
        return CORESIGHT_NO_NUM;
    }
    g_coresight_funnel.num = num;
    g_coresight_funnel.funnel =
        (struct coresight_funnel_stru *)kmalloc(num * sizeof(struct coresight_funnel_stru), GFP_KERNEL);
    if (g_coresight_funnel.funnel == NULL) {
        return CORESIGHT_MALLOC_ERR;
    }
    bsp_dt_for_each_child_of_node(node, child)
    {
        if (i >= num) {
            kfree(g_coresight_funnel.funnel);
            g_coresight_funnel.funnel = NULL;
            return CORESIGHT_CHILD_ERR;
        }
        g_coresight_funnel.funnel[i].base = (void *)(uintptr_t)bsp_dt_iomap(child, 0);
        if (g_coresight_funnel.funnel[i].base == NULL) {
            kfree(g_coresight_funnel.funnel);
            g_coresight_funnel.funnel = NULL;
            return CORESIGHT_FUNNEL_CHILD_NOFOUND;
        }
        if (bsp_dt_property_read_u32(child, "chnnl", &chnnl) || chnnl > CORESIGHT_FUNNEL_LIMIT) {
            kfree(g_coresight_funnel.funnel);
            g_coresight_funnel.funnel = NULL;
            return CORESIGHT_NO_CHNNL;
        }
        g_coresight_funnel.funnel[i].chnnl = chnnl;
        i = i + 1;
    }
    g_coresight_funnel.status = FUNNEL_INT_OK;
    return BSP_OK;
}
