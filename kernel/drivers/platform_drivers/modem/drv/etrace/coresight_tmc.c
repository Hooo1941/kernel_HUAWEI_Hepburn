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
#include <bsp_slice.h>
#include <bsp_print.h>
#include "coresight_trace_mgr.h"
#include "coresight_errno.h"
#include "coresight_tmc.h"
struct coresight_etf_info g_coresight_etf;
struct coresight_etf_info g_coresight_etr;
#undef THIS_MODU
#define THIS_MODU mod_coresight
int coresight_tmc_etr_init(void)
{
    u32 mode = 0;
    device_node_s *node = bsp_dt_find_node_by_path("/coresight_etr");
    if (node == NULL) {
        return -CORESIGHT_ETF_NOFOUND;
    }
    (void)memset_s((void *)(uintptr_t)&g_coresight_etr, sizeof(g_coresight_etr), 0, sizeof(g_coresight_etr));
    g_coresight_etr.etf = (struct coresight_etf_stru *)kmalloc(sizeof(struct coresight_etf_stru), GFP_KERNEL);
    if (g_coresight_etr.etf == NULL) {
        return -CORESIGHT_ETF_MALLOC_ERR;
    }
    g_coresight_etr.etf->base = (void *)(uintptr_t)bsp_dt_iomap(node, 0);
    if (g_coresight_etr.etf->base == NULL) {
        kfree(g_coresight_etr.etf);
        g_coresight_etr.etf = NULL;
        return -CORESIGHT_ETF_CHILD_NOFOUND;
    }
    if (bsp_dt_property_read_u32(node, "mode", &mode) || mode > 0xF) {
        kfree(g_coresight_etr.etf);
        g_coresight_etr.etf = NULL;
        return -CORESIGHT_NO_ETF_MODE;
    }
    g_coresight_etr.etf->mode = mode;
    g_coresight_etr.status = ETF_INT_OK;
    return CORESIGHT_ETF_OK;
}
int coresight_tmc_etf_init(void)
{
    u32 i = 0;
    u32 num = 0;
    u32 mode = 0;
    device_node_s *child = NULL;
    device_node_s *node = bsp_dt_find_node_by_path("/coresight_etf");
    if (node == NULL) {
        return -CORESIGHT_ETF_NOFOUND;
    }
    (void)memset_s((void *)(uintptr_t)&g_coresight_etf, sizeof(g_coresight_etf), 0, sizeof(g_coresight_etf));
    if (bsp_dt_property_read_u32(node, "num", &num) || num > CORESIGHT_MAX_ETF_NUM) {
        return -CORESIGHT_NO_ETF_NUM;
    }
    g_coresight_etf.num = num;
    g_coresight_etf.etf = (struct coresight_etf_stru *)kmalloc(num * sizeof(struct coresight_etf_stru), GFP_KERNEL);
    if (g_coresight_etf.etf == NULL) {
        return -CORESIGHT_ETF_MALLOC_ERR;
    }
    bsp_dt_for_each_child_of_node(node, child)
    {
        if (i >= num) {
            kfree(g_coresight_etf.etf);
            g_coresight_etf.etf = NULL;
            return -CORESIGHT_ETF_CHILD_ERR;
        }
        g_coresight_etf.etf[i].base = (void *)bsp_dt_iomap(child, 0);
        if (g_coresight_etf.etf[i].base == NULL) {
            kfree(g_coresight_etf.etf);
            g_coresight_etf.etf = NULL;
            return -CORESIGHT_ETF_CHILD_NOFOUND;
        }
        if (bsp_dt_property_read_u32(child, "mode", &mode) || mode > 0xF) {
            kfree(g_coresight_etf.etf);
            g_coresight_etf.etf = NULL;
            return -CORESIGHT_NO_ETF_MODE;
        }
        g_coresight_etf.etf[i].mode = mode;
        i = i + 1;
    }
    g_coresight_etf.status = ETF_INT_OK;
    return CORESIGHT_ETF_OK;
}
static int coresight_tmc_stop_and_flush(void *base)
{
    u32 ffcr;
    ffcr = CORESIGHT_READL(base, CORESIGHT_TMC_FFCR);
    ffcr |= TMC_FFCR_STOPONFLUSH;
    CORESIGHT_WRITEL(base, ffcr, CORESIGHT_TMC_FFCR);
    ffcr |= TMC_FFCR_FLUSHMAN;
    CORESIGHT_WRITEL(base, ffcr, CORESIGHT_TMC_FFCR);
    return BSP_OK;
}
static void coresight_disable_etf(void *etf_base)
{
    if (etf_base == NULL) {
        return;
    }
    CORESIGHT_TMC_UNLOCK((uintptr_t)etf_base);

    coresight_tmc_stop_and_flush(etf_base);
    CORESIGHT_WRITEL(etf_base, TMC_CTL_TRACECAPDIS, CORESIGHT_TMC_CTL);

    CORESIGHT_TMC_LOCK(etf_base);
}
void coresight_tmc_enable_etr(unsigned long ddr_buffer_addr, u32 ddr_buffer_len)
{
    void *etr_base = g_coresight_etr.etf->base;
    if (g_coresight_etr.etf->base == NULL) {
        return;
    }
    CORESIGHT_TMC_UNLOCK((uintptr_t)etr_base);

    CORESIGHT_WRITEL((uintptr_t)etr_base, TMC_CTL_TRACECAPDIS, CORESIGHT_TMC_CTL);
    CORESIGHT_WRITEL((uintptr_t)etr_base, ddr_buffer_len / 0x4, CORESIGHT_TMC_RSZ);

    CORESIGHT_WRITEL((uintptr_t)etr_base, 0, CORESIGHT_TMC_RRP);
    CORESIGHT_WRITEL((uintptr_t)etr_base, 0, CORESIGHT_TMC_RWP);

    CORESIGHT_WRITEL((uintptr_t)etr_base, CORESIGHT_TMC_MODE_CIRCULAR_BUFFER, CORESIGHT_TMC_MODE);
    CORESIGHT_WRITEL((uintptr_t)etr_base, 0x1123, CORESIGHT_TMC_FFCR);
    CORESIGHT_WRITEL((uintptr_t)etr_base, 0xF03, CORESIGHT_TMC_AXICTL);
    CORESIGHT_WRITEL((uintptr_t)etr_base, ddr_buffer_addr, CORESIGHT_TMC_DBALO);
    CORESIGHT_WRITEL((uintptr_t)etr_base, 0, CORESIGHT_TMC_DBAHI);
    bsp_err("cs addr = %x %x\n", readl(etr_base + CORESIGHT_TMC_RSZ), readl(etr_base + CORESIGHT_TMC_DBALO));
    /* enable tmc */
    CORESIGHT_WRITEL((uintptr_t)etr_base, TMC_CTL_TRACECAPEN, CORESIGHT_TMC_CTL);
    CORESIGHT_TMC_LOCK((uintptr_t)etr_base);
}
void coresight_disable_etr(void)
{
    coresight_disable_etf(g_coresight_etr.etf->base);
}
/* 清除ETR的目的地址配置，需要先disable，再调用该接口，否则不生效。hlt中会使用该接口，删除前需要确认是否影响hlt用例 */
void coresight_clear_etr(void)
{
    void *etr_base = g_coresight_etr.etf->base;
    if (g_coresight_etr.etf->base == NULL) {
        return;
    }
    CORESIGHT_TMC_UNLOCK((uintptr_t)etr_base);
    CORESIGHT_WRITEL((uintptr_t)etr_base, 0, CORESIGHT_TMC_RSZ);
    CORESIGHT_WRITEL((uintptr_t)etr_base, 0, CORESIGHT_TMC_DBALO);
    CORESIGHT_WRITEL((uintptr_t)etr_base, 0, CORESIGHT_TMC_DBAHI);
    CORESIGHT_TMC_LOCK((uintptr_t)etr_base);
}
unsigned long coresight_get_etr_rwp(void)
{
    void *etr_base = NULL;
    if (g_coresight_etr.etf == NULL) {
        return ETR_ERROR_ADDR;
    }
    etr_base = g_coresight_etr.etf->base;
    if (etr_base == NULL) {
        return ETR_ERROR_ADDR;
    }
    return readl(etr_base + CORESIGHT_TMC_RWP);
}
static void coresight_tmc_etf_enable(void *base, u32 mode)
{
    if (base == NULL) {
        return;
    }
    CORESIGHT_TMC_UNLOCK(base);
    if (mode == CORESIGHT_TMC_MODE_CIRCULAR_BUFFER) {
        CORESIGHT_WRITEL(base, TMC_CTL_TRACECAPDIS, CORESIGHT_TMC_CTL);
        CORESIGHT_WRITEL(base, CORESIGHT_TMC_MODE_CIRCULAR_BUFFER, CORESIGHT_TMC_MODE);
        CORESIGHT_WRITEL(base, TMC_FFCR_CIRCULAR_CFG, CORESIGHT_TMC_FFCR);
        CORESIGHT_WRITEL(base, 0, CORESIGHT_TMC_TRG);
        CORESIGHT_WRITEL(base, TMC_CTL_TRACECAPEN, CORESIGHT_TMC_CTL);
    } else if (mode == CORESIGHT_TMC_MODE_HARDWARE_FIFO) {
        CORESIGHT_WRITEL(base, TMC_CTL_TRACECAPDIS, CORESIGHT_TMC_CTL);
        CORESIGHT_WRITEL(base, 0, CORESIGHT_TMC_RRP);
        CORESIGHT_WRITEL(base, 0, CORESIGHT_TMC_RWP);
        CORESIGHT_WRITEL(base, CORESIGHT_TMC_MODE_HARDWARE_FIFO, CORESIGHT_TMC_MODE);
        CORESIGHT_WRITEL(base, TMC_CTL_TRACECAPEN, CORESIGHT_TMC_CTL);
    }
    /* enable tmc */
    CORESIGHT_TMC_LOCK(base);
}
int coresight_tmc_etf_opt(u32 opt)
{
    u32 num = g_coresight_etf.num;
    u32 i, mode;
    void *base = NULL;
    if (g_coresight_etf.status != ETF_INT_OK) {
        return -CORESIGHT_ETF_ENABLE_FAIL;
    }
    for (i = 0; i < num; i++) {
        base = g_coresight_etf.etf[i].base;
        mode = g_coresight_etf.etf[i].mode;
        if (opt == ETF_ENABLE) {
            coresight_tmc_etf_enable(base, mode);
        } else {
            coresight_disable_etf(base);
        }
    }
    return BSP_OK;
}
