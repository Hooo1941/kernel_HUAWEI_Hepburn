/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <securec.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <bsp_pcie.h>
#include "pcie_cdev.h"
#include "pcie_cdev_desc.h"
#include "pcie_cdev_dbg.h"

#define ALIGNMENT_SIZE_64K (BAR_SIZE_64K + 1)

static void pcdev_desc_init(void)
{
    union pcie_cdev_map *desc = NULL;
    int i;
    struct pcdev_ctx *pcdev_ctx = NULL;
    void *desc_start = NULL;
    struct pcdev_ports_desc *ports_desc = NULL;
    struct pcie_cdev_port_manager *pcdev_ports = get_pcdev_ports();
    struct pcie_cdev_name_type_tbl *type_table = get_pcdev_type_table();
    int ret;

    PCDEV_TRACE("in\n");
    pcdev_ctx = get_pcdev_ctx();
    desc_start = pcdev_ctx->virt_addr + sizeof(struct pcdev_ports_desc);
    ports_desc = (struct pcdev_ports_desc *)pcdev_ctx->virt_addr;

    ports_desc->allocked = PCIE_CDEV_ALLOCED;
    ports_desc->port_num = PCIE_CDEV_COUNT;

    ret = memset_s(desc_start, pcdev_ctx->buffer_size, 0, sizeof(union pcie_cdev_map) * PCIE_CDEV_COUNT);
    if (ret) {
        PCDEV_ERR("memset_s fail\n");
        return;
    }

    for (i = 0; i < PCIE_CDEV_COUNT; i++) {
        pcdev_ports[i].desc = (union pcie_cdev_map *)desc_start + i;
        desc = pcdev_ports[i].desc;
        desc->ep.port_id = type_table[i].type;
        desc->ep.rx_max_size = type_table[i].rc2ep_max_buf_sz;
        desc->ep.tx_max_size = type_table[i].ep2rc_max_buf_sz;
    }

    return;
}

int pcie_cdev_bar_init(void)
{
    int ret;
    dma_addr_t paddr;
    struct device *dev_p = NULL;
    struct pcdev_ctx *pcdev_ctx = NULL;
    unsigned long alloc_size;

    pcdev_ctx = get_pcdev_ctx();
#ifdef CONFIG_ARM64
    dev_p = &pcdev_ctx->pdev->dev;
#endif

    pcdev_ctx->buffer_size = pcie_cdev_get_bar_size();
    if (!pcdev_ctx->buffer_size) {
        PCDEV_ERR("bar size err\n");
        return -1;
    }

    alloc_size = pcdev_ctx->buffer_size + ALIGNMENT_SIZE_64K;

    pcdev_ctx->virt_addr = dma_alloc_coherent(dev_p, alloc_size, &paddr, GFP_KERNEL);
    if (pcdev_ctx->virt_addr == NULL) {
        PCDEV_ERR("virt_addr alloc fail\n");
        return -1;
    }

    if (!IS_ALIGNED(paddr, ALIGNMENT_SIZE_64K)) {
        pcdev_ctx->phys_addr = (void *)PTR_ALIGN(paddr, ALIGNMENT_SIZE_64K);
    } else {
        pcdev_ctx->phys_addr = (void *)paddr;
    }
    pcdev_ctx->virt_addr += (pcdev_ctx->phys_addr - (void *)paddr);

    pcdev_desc_init();

    ret = bsp_pcie_ep_bar_config(PCIE_BAR_CHAR_DEV, pcdev_ctx->buffer_size,
                                 (u64)(unsigned long)pcdev_ctx->phys_addr);  // map to ep msi phy addr
    if (ret) {
        PCDEV_ERR("pcie cdev bar config fail\n");
        dma_free_coherent(dev_p, alloc_size, pcdev_ctx->virt_addr, paddr);
        return -1;
    }

    return ret;
}
