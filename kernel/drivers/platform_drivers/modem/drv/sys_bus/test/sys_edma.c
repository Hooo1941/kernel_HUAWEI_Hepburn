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
#include "sys_edma.h"
#include "sys_bus_api.h"

#include "securec.h"
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include "bsp_edma.h"
#include "bsp_slice.h"
#include <osl_malloc.h>

#define SYS_EDMA_CNT_TO_TIME_BIT (15)
#define SYS_EDMA_TEST_SIZE (1024 * 1024)
#define SYS_EDMA_ADDR_BIT_MASK (64)

u32 g_edma_lli_loop_src = 0;
u32 g_edma_lli_loop_des = 0;
struct sys_edma g_sys_edma[1];
struct device g_sys_edma_dev;

/* update time and totalsize and restart */
void sys_edma_ddr_irq_handler(unsigned long channel_arg, unsigned int int_status)
{
    if (bsp_slice_getcurtime(&g_sys_edma[0].end_t) != 0) {
        sys_bus_trace("get 64bit timestamp error\n");
    }

    g_sys_edma[0].totaltime = g_sys_edma[0].end_t - g_sys_edma[0].start_t;
    g_sys_edma[0].totalsize += 1;

    if (g_sys_edma[0].flag) {
        sys_edma_lli_start(&g_sys_edma[0]);
    }
}

void sys_edma_lli_config(struct sys_edma* sys_edma)
{
    edma_addr_t phy_edma_addr = 0;
    edma_addr_t phy_edma_addr_tmp;
    struct edma_cb* psttemp = NULL;
    u32 lli_num = sys_edma->lli_num;
    u32 i = 0;

    sys_edma->edma_cb = (struct edma_cb*)dma_alloc_coherent(&g_sys_edma_dev, (lli_num * sizeof(struct edma_cb)),
                                                            &phy_edma_addr, GFP_DMA | __GFP_WAIT);
    if (sys_edma->edma_cb == NULL) {
        sys_bus_trace("[sysbus]edma, sys_edma_lli_config alloc failed!\n");
        return;
    }

    psttemp = sys_edma->edma_cb;
    phy_edma_addr_tmp = phy_edma_addr;

    for (i = 0; i < lli_num; i++) {
        phy_edma_addr_tmp += sizeof(struct edma_cb);
        if (i == (lli_num - 1)) {
            psttemp->lli = EDMA_SET_LLI(phy_edma_addr_tmp, 1);
        } else {
            psttemp->lli = EDMA_SET_LLI(phy_edma_addr_tmp, 0);
        }
        psttemp->config = EDMA_SET_CONFIG(sys_edma->req, EDMA_M2M, EDMA_TRANS_WIDTH_64, EDMA_BUR_LEN_16);
        psttemp->src_addr = sys_edma->src;
        psttemp->des_addr = sys_edma->dst;
        psttemp->cnt0 = sys_edma->size;
        psttemp->bindx = 0;
        psttemp->cindx = 0;
        psttemp->cnt1 = 0;
        psttemp++;
    }

    sys_edma->channel_id = bsp_edma_channel_init(sys_edma->req, sys_edma->handler, 0, EDMA_INT_DONE);
    if (sys_edma->channel_id < 0) {
        sys_bus_trace("[sysbus]edma, error ret_id = 0x%X\n\n", sys_edma->channel_id);
        return;
    }
}

void sys_edma_lli_start(struct sys_edma* sys_edma)
{
    struct edma_cb* psttemp = NULL;

    psttemp = bsp_edma_channel_get_lli_addr(sys_edma->channel_id);
    if (psttemp == NULL) {
        sys_bus_trace("[sysbus]edma, failed!\n\n");
        return;
    }

    psttemp->lli = sys_edma->edma_cb->lli;
    psttemp->config = sys_edma->edma_cb->config & 0xFFFFFFFE;
    psttemp->src_addr = sys_edma->edma_cb->src_addr;
    psttemp->des_addr = sys_edma->edma_cb->des_addr;
    psttemp->cnt0 = sys_edma->edma_cb->cnt0;
    psttemp->bindx = 0;
    psttemp->cindx = 0;
    psttemp->cnt1 = 0;
    if (bsp_edma_channel_lli_async_start(sys_edma->channel_id)) {
        sys_bus_trace("[sysbus]edma, FAILED!\n\n");
    }
}

void sys_edma_report(void)
{
    u64 s_time;
    u32 M_size;
    sys_bus_trace("[sysbus]edma, analysis :\n");

    s_time = g_sys_edma[0].totaltime >> SYS_EDMA_CNT_TO_TIME_BIT; // change to time in second.
    M_size = g_sys_edma[0].totalsize;
    sys_bus_trace("[sysbus]edma, ddr total time :%d s\n", (u32)s_time);
    sys_bus_trace("[sysbus]edma, ddr total size :%d MB\n", M_size);
    if (s_time) {
        sys_bus_trace("[sysbus]edma, ddr rate :%d MB/s\n", M_size / (u32)s_time);
    }
}

void sys_edma_start(void)
{
    dma_addr_t dma_handle;
    struct edma_cb* edma_cb = NULL;

    edma_cb = dma_alloc_coherent(&g_sys_edma_dev, edma_ddr_size, &dma_handle,
                                 GFP_DMA | __GFP_WAIT);
    if (edma_cb == NULL) {
        sys_bus_trace("[sysbus]edma, alloc g_edma_lli_loop_src failed!\n");
        return;
    }
    g_edma_lli_loop_src = (u32)dma_handle;

    edma_cb = dma_alloc_coherent(&g_sys_edma_dev, edma_ddr_size, &dma_handle,
                                 GFP_DMA | __GFP_WAIT);
    if (edma_cb == NULL) {
        sys_bus_trace("[sysbus]edma, alloc g_edma_lli_loop_des failed!\n");
        return;
    }
    g_edma_lli_loop_des = (u32)dma_handle;

    /* 0 is ddr , 1 is sram */
    g_sys_edma[0].handler = sys_edma_ddr_irq_handler;
    g_sys_edma[0].src = (u32)g_edma_lli_loop_src;
    g_sys_edma[0].dst = (u32)g_edma_lli_loop_des;

    g_sys_edma[0].size = edma_ddr_size;
    g_sys_edma[0].lli_num = SYS_EDMA_TEST_SIZE / edma_ddr_size; // 1M
    g_sys_edma[0].flag = 1;
    g_sys_edma[0].totalsize = 0;
    g_sys_edma[0].totaltime = 0;

    sys_edma_lli_config(&g_sys_edma[0]);

    if (bsp_slice_getcurtime(&g_sys_edma[0].start_t) != 0) {
        sys_bus_trace("[sysbus]edma, get 64bit timestamp error\n");
    }

    sys_edma_lli_start(&g_sys_edma[0]);
}

void sys_edma_stop(void)
{
    g_sys_edma[0].flag = 0;
}

// 当前IP的总线压测事件入口�?
int sys_edma_entry(sys_bus_evt_e event)
{
    sys_bus_err("[sys_bus]edma, rcv evt: %d\n", event);

    if (event == E_SYS_BUS_EVT_START) {
        sys_edma_start();
    } else if (event == E_SYS_BUS_EVT_STOP) {
        sys_edma_stop();
    } else if (event == E_SYS_BUS_EVT_REPORT) {
        sys_edma_report();
    } else {
        sys_bus_err("[sysbus]edma, receive incorrect event: %d\n", event);
        return E_SYS_BUS_RET_ERR;
    }

    return E_SYS_BUS_RET_OK;
}

void sys_edma_init(struct device_node* np)
{
    int ret;

    ret = of_property_read_u32_index(np, "edma_req_id_ddr", 0, &g_sys_edma[0].req);
    if (ret != 0) {
        sys_bus_trace("[sysbus]edma, read edma_req_id_ddr err\n");
        return;
    }

    if (memset_s(&g_sys_edma_dev, sizeof(struct device), 0, sizeof(struct device)) != 0) {
        sys_bus_trace("[sysbus]edma, memset device fail.\n ");
        return;
    }
    of_dma_configure(&g_sys_edma_dev, NULL);
    arch_setup_dma_ops(&g_sys_edma_dev, 0, 0, NULL, 0);
    dma_set_mask_and_coherent(&g_sys_edma_dev, DMA_BIT_MASK(SYS_EDMA_ADDR_BIT_MASK)); // dma mask

    sys_bus_register_observer(SYS_EDMA_NAME, sys_edma_entry);

    sys_bus_trace("[sysbus]edma, init ok.\n");
    return;
}
