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

#include <bsp_pcie.h>
#include "pcie_cdev.h"
#include "pcie_cdev_desc.h"

unsigned long pcie_cdev_get_bar_size(void)
{
    unsigned long desc_size;

    desc_size = sizeof(union pcie_cdev_map) * PCIE_CDEV_COUNT + sizeof(struct pcdev_ports_desc);

    if (desc_size < BAR_SIZE_256B) {
        return BAR_SIZE_256B;
    } else if (desc_size < BAR_SIZE_1K) {
        return BAR_SIZE_1K;
    } else if (desc_size < BAR_SIZE_4K) {
        return BAR_SIZE_4K;
    } else if (desc_size < BAR_SIZE_16K) {
        return BAR_SIZE_16K;
    } else if (desc_size < BAR_SIZE_64K) {
        return BAR_SIZE_64K;
    } else if (desc_size < BAR_SIZE_256K) {
        return BAR_SIZE_256K;
    } else if (desc_size < BAR_SIZE_1M) {
        return BAR_SIZE_1M;
    } else if (desc_size < BAR_SIZE_4M) {
        return BAR_SIZE_4M;
    } else if (desc_size < BAR_SIZE_16M) {
        return BAR_SIZE_16M;
    } else if (desc_size < BAR_SIZE_64M) {
        return BAR_SIZE_64M;
    } else if (desc_size < BAR_SIZE_256M) {
        return BAR_SIZE_256M;
    } else {
        return 0;
    }
}

static void port_desc_map_ep(struct pcie_cdev_port *port, union pcie_cdev_map *desc)
{
    int i;
    port->port_id = &desc->ep.port_id;
    port->local_stat = &desc->ep.self_status;
    port->remote_stat = &desc->ep.peer_status;
    for (i = 0; i < IRQ_NUM; i++) {
        port->pcie_cdev_send_irq[i] = &(desc->ep.snd_irq_status[i]);
        port->pcie_cdev_rcv_irq[i] = &(desc->ep.rcv_irq_status[i]);
    }
    port->event_type = &desc->ep.event_type;
    port->read_sig = (pcdev_mdm_msc_stru *)desc->ep.read_sig;
    port->rel_ind = &desc->ep.rel_ind;
    port->msc_stru = (pcdev_mdm_msc_stru *)desc->ep.msc_stru;
    port->msc_busy = &desc->ep.msc_busy;
    port->rx_rp = &desc->ep.rx_rp;
    port->rx_wp = &desc->ep.rx_wp;
    port->tx_rp = &desc->ep.tx_rp;
    port->tx_rp_todo = &desc->ep.tx_rp_todo_ep;

#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    port->tx_rp_todo_rc = &desc->ep.tx_rp_todo_rc;
    port->tx_rp_dma_cmp_rc = &desc->ep.tx_rp_dma_cmp_rc;
    port->tx_rp_dma_cfg_rc = desc->ep.rx_rp;
#endif
    port->tx_wp = &desc->ep.tx_wp;
    port->rx_desc = desc->ep.rx;
    port->tx_desc = desc->ep.tx;
    port->rx_buf_max_sz = &desc->ep.rx_max_size;
    port->rx_buf_max_size = desc->ep.rx_max_size;
    port->tx_buf_max_sz = &desc->ep.tx_max_size;
}

static void port_desc_map_rc(struct pcie_cdev_port *port, union pcie_cdev_map *desc)
{
    int i;
    port->port_id = &desc->rc.port_id;
    port->local_stat = &desc->rc.self_status;
    port->remote_stat = &desc->rc.peer_status;
    for (i = 0; i < IRQ_NUM; i++) {
        port->pcie_cdev_send_irq[i] = &(desc->rc.snd_irq_status[i]);
        port->pcie_cdev_rcv_irq[i] = &(desc->rc.rcv_irq_status[i]);
    }
    port->event_type = &desc->rc.event_type;
    port->read_sig = (pcdev_mdm_msc_stru *)desc->rc.read_sig;
    port->rel_ind = &desc->rc.rel_ind;
    port->msc_stru = (pcdev_mdm_msc_stru *)desc->rc.msc_stru;
    port->msc_busy = &desc->rc.msc_busy;
    port->rx_rp = &desc->rc.rx_rp;
    port->rx_wp = &desc->rc.rx_wp;
    port->tx_rp = &desc->rc.tx_rp;
    port->tx_rp_todo = &desc->rc.tx_rp_todo_rc;
#ifdef CONFIG_PCIE_CDEV_DMA_SINGLE
    port->tx_rp_dma_cmp_rc = &desc->rc.tx_rp_dma_cmp_rc;
    port->tx_rp_cmp = desc->rc.tx_rp;
#endif
    port->tx_wp = &desc->rc.tx_wp;
    port->rx_desc = desc->rc.rx;
    port->tx_desc = desc->rc.tx;
    port->rx_buf_max_sz = &desc->rc.rx_max_size;
    port->rx_buf_max_size = desc->rc.rx_max_size;
    port->tx_buf_max_sz = &desc->rc.tx_max_size;
}

void port_desc_map(unsigned int port_num, unsigned int mode)
{
    struct pcie_cdev_port_manager *pcdev_ports = get_pcdev_ports();
    struct pcie_cdev_port *port = pcdev_ports[port_num].port;
    union pcie_cdev_map *desc = pcdev_ports[port_num].desc;

    if (mode == PCIE_EP) {
        port_desc_map_ep(port, desc);
    } else if (mode == PCIE_RC) {
        port_desc_map_rc(port, desc);
    }
}
