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

/*
 * pfa_port.c - pfa port management
 */
#include <linux/netdevice.h>
#include <securec.h>
#include <bsp_dra.h>
#include <bsp_syscache.h>
#include "pfa.h"
#include "pfa_port.h"
#include "pfa_desc.h"
#include "pfa_dbg.h"


int pfa_port_exist(struct pfa *pfa, int portno)
{
    int isgrp;

    if (unlikely(portno >= PFA_PORT_MAX || portno < 0)) {
        PFA_ERR("invalid portno(%d)!\n", portno);
        return 0;
    }

    isgrp = (portno >= PFA_PORT_NUM) ? 1 : 0;

    return test_bit(portno, &pfa->portmap[isgrp]);
}

int pfa_port_set_mac(unsigned int portno, const unsigned char *mac)
{
    struct pfa *pfa = &g_pfa;
    unsigned int mac_hi;
    unsigned int mac_lo;

    if (!PFA_NORM_PORT(portno)) {
        PFA_ERR("invalid port no:%d.\n", portno);
        return -EINVAL;
    }

    if (!pfa_port_exist(pfa, portno)) {
        PFA_ERR("port(%d) is not exist.\n", portno);
        return -EINVAL;
    }

    if (mac == NULL) {
        PFA_ERR("mac is null ptr.\n");
        return -EINVAL;
    }

    mac_lo = (mac[0]) | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24); /* 2,3,8,16,24 mac addr calc */
    mac_hi = (mac[4]) | (mac[5] << 8); /* 4,5,8 mac addr calc */

    pfa_writel(pfa->regs, PFA_PORTX_MAC_ADDR_L(portno), mac_lo);
    pfa_writel(pfa->regs, PFA_PORTX_MAC_ADDR_H(portno), mac_hi);

    return 0;
}

int pfa_recycle_port_queue_desc(struct pfa *pfa, unsigned int portno)
{
    struct pfa_port_ctx *port_ctx = &pfa->ports[portno];
    unsigned int rd_rptr;
    unsigned int rd_wptr;
    unsigned int port_idle;
    unsigned int timeout = 10; /* 10 * 1 ms wait */

    do {
        /* judge pfa_idle */
        port_idle = pfa_readl(pfa->regs, PFA_IDLE);

        rd_rptr = pfa_readl(pfa->regs, PFA_RDQX_RPTR(portno));
        rd_wptr = pfa_readl(pfa->regs, PFA_RDQX_WPTR(portno));
        /* make sure rd queue is empty */
        if ((port_idle & (1 << portno)) && (rd_rptr == rd_wptr)) {
            break;
        }

        // Only process the RD queue. The hardware will empty the BD queue, and make the rptr and wptr are equalr, in the case of port disabled
        pfa->smp.process_desc((void *)pfa, 0x1 << portno, 0);

        if (timeout-- <= 0) {
            if (port_idle & (1 << portno)) {
                port_ctx->stat.recycle_port_idle++;
                PFA_ERR("pfa port %u rdq wait idle timed out\n", portno);
            } else {
                port_ctx->stat.recycle_port_emtpy++;
                PFA_ERR("pfa port %u rdq rptr not eq wptr timed out, rd_rptr: %u, rd_wptr：%u \n", portno, rd_rptr, rd_wptr);
            }

            return -EBUSY;
        }
        mdelay(1);
    } while (1);

    return 0;
}

int pfa_port_check_common_attr(struct pfa_port_comm_attr *attr)
{
    int ret = 0;
    if (attr == NULL) {
        PFA_ERR("input attr NULL.\n");
        return -EINVAL;
    }

    if (attr->enc_type >= PFA_ENC_BOTTOM) {
        PFA_ERR("invalid enc_type(%d).\n", attr->enc_type);
        ret = -EINVAL;
        goto error;
    }

    /* every port but pfa_tft should have td */
    if ((!attr->rd_depth) || (!attr->td_depth) || (attr->td_depth >= PFA_MAX_TD)) {
        PFA_ERR("port desc_depth err td %u rd %u.\n", attr->td_depth, attr->rd_depth);
        ret = -EINVAL;
        goto error;
    }

    return 0;

error:
    pfa_get_port_type(0, attr->enc_type);
    pfa_bug(&g_pfa);
    return ret;
}

bool pfa_port_attr_type_check(struct pfa_port_comm_attr *attr)
{
    if ((attr->enc_type >= PFA_ENC_NCM_NTB16 && attr->enc_type <= PFA_ENC_RNDIS) || (attr->enc_type == PFA_ENC_ECM) ||
        (attr->enc_type == PFA_ENC_DIRECT_FW_NCM_NTB32) || (attr->enc_type == PFA_ENC_DIRECT_FW_NCM_NTB16) ||
        (attr->enc_type == PFA_ENC_DIRECT_FW_RNDIS) || (attr->enc_type == PFA_ENC_DIRECT_FW_ECM) ||
        (attr->td_copy_en == PFA_PORT_TD_COPY_ENABLE)) {
        return true;
    }

    return false;
}

int pfa_port_check_attr(struct pfa_port_comm_attr *attr)
{
    int ret = 0;

    if (pfa_port_check_common_attr(attr)) {
        goto error;
    }

    if (attr->enc_type == PFA_ENC_CPU && attr->ops.get_wan_info == NULL) {
        PFA_ERR("wanport set get_wan_info param failed.\n");
        goto error;
    }
    if (attr->enc_type != PFA_ENC_CPU && attr->ops.get_wan_info != NULL) {
        PFA_ERR("any port other than wanport should not set get_wan_info.\n");
        goto error;
    }

    if (attr->enc_type == PFA_ENC_PFA_TFT) {
        return 0;
    }

    if (pfa_port_attr_type_check(attr)) {
        if (!attr->ops.pfa_finish_td) {     /* only usb port needs td finish */
            PFA_ERR(" port has no pfa_finish_td.\n");
            goto error;
        }
    } else {
        if (!attr->net) {
            PFA_ERR("input net dev NULL.\n");
            goto error;
        }
    }

    /* every port but pfa_tft should have finish rd */
    if (!attr->ops.pfa_finish_rd && !attr->ops.pfa_finish_rd_dra) {
        PFA_ERR("port has no pfa_finish_rd\n");
        goto error;
    }
    /* stick mode check, stick mode only support usb and pfa_tft port */
    if (attr->stick_en && (attr->enc_type != PFA_ENC_NCM_NTB16 && attr->enc_type != PFA_ENC_NCM_NTB32)) {
            PFA_ERR("stick mode enable, but the port type is not usb or pfa_tft \n");
            goto error;
    }

    return 0;

error:
    ret = -EINVAL;
    if (attr != NULL) {
        pfa_get_port_type(0, attr->enc_type);
    }
    pfa_bug(&g_pfa);
    return ret;
}

void pfa_set_pfa_tft_mac(struct ethhdr *mac_hdr)
{
    struct pfa *pfa = &g_pfa;
    unsigned char *mac;
    unsigned int mac_lo;
    unsigned int mac_hi;
    int ret;

    mac = mac_hdr->h_dest;
    mac_lo = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0]; /* 2,3,8,16,24 mac add calc */
    mac_hi = (mac[5] << 8) | mac[4]; /* 4,5,8 mac add calc */

    pfa_writel_relaxed(pfa->regs, PFA_GLB_DMAC_ADDR_L, mac_lo);
    pfa_writel_relaxed(pfa->regs, PFA_GLB_DMAC_ADDR_H, mac_hi);

    mac = mac_hdr->h_source;
    mac_lo = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0]; /* 2,3,8,16,24 mac add calc */
    mac_hi = (mac[5] << 8) | mac[4]; /* 4,5,8 mac add calc */

    pfa_writel_relaxed(pfa->regs, PFA_GLB_SMAC_ADDR_L, mac_lo);
    pfa_writel_relaxed(pfa->regs, PFA_GLB_SMAC_ADDR_H, mac_hi);

    ret = memcpy_s(&pfa->pfa_tftport.pfa_tft_eth_head, sizeof(pfa->pfa_tftport.pfa_tft_eth_head), mac_hdr, sizeof(*mac_hdr));
    if (ret) {
        PFA_ERR("<pfa_set_pfa_tft_mac> memcpy_s fail!\n");
    }
}

void pfa_port_clean_ctx(struct pfa *pfa, struct pfa_port_ctx *port_ctx)
{
    int ret = 0;
    struct pfa_port_ctrl *port_ctrl = &port_ctx->ctrl;

    switch (port_ctrl->enc_type) {
        case PFA_ENC_NONE:
        case PFA_ENC_WIFI:
        case PFA_ENC_ACK:
            break;

        case PFA_ENC_NCM_NTB16:
        case PFA_ENC_NCM_NTB32:
        case PFA_ENC_RNDIS:
        case PFA_ENC_ECM:
            ret += memset_s(&pfa->usbport, sizeof(pfa->usbport), 0, sizeof(struct pfa_usbport_ctx));
            break;

        case PFA_ENC_PFA_TFT:
            ret += memset_s(&pfa->pfa_tftport, sizeof(pfa->pfa_tftport), 0, sizeof(struct pfa_pfa_tftport_ctx));
            ret += memset_s(&pfa->wanport, sizeof(pfa->wanport), 0, sizeof(struct pfa_wanport_ctx));
            break;

        case PFA_ENC_CPU:
            ret += memset_s(&pfa->cpuport, sizeof(pfa->cpuport), 0, sizeof(struct pfa_cpuport_ctx));
            break;

        default:
            break;
    }

    ret += memset_s(&port_ctx->stat, sizeof(port_ctx->stat), 0, sizeof(struct pfa_port_stat));
    if (ret) {
        PFA_ERR("memset_s fail!\n");
    }
}

static void pfa_release_port_res(struct pfa *pfa, struct pfa_port_ctrl *port_ctrl)
{
    if (port_ctrl->rd_long_buf != NULL) {
        kfree(port_ctrl->rd_long_buf);
        port_ctrl->rd_long_buf = NULL;
    }

    /* free td queue */
    if (port_ctrl->td_addr != NULL) {
        bsp_sc_free_ringbuf(pfa->dev, port_ctrl->td_depth * sizeof(struct pfa_td_desc) + port_ctrl->rd_depth * sizeof(struct pfa_rd_desc),
            port_ctrl->td_addr, port_ctrl->td_dma);
        port_ctrl->td_addr = NULL;
        port_ctrl->td_dma = 0;
    }
    return;
}


int pfa_port_free(int port_num)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctx *port_ctx = NULL;
    struct pfa_port_ctrl *port_ctrl = NULL;
    int ret;
    unsigned long flags;
    unsigned int portno;

    if (port_num < 0) {
        PFA_ERR("port number %d err !\n", port_num);
        return -EINVAL;
    } else {
        portno = (unsigned int)port_num;
    }
    if (unlikely(pfa->flags & PFA_FLAG_SUSPEND)) {
        PFA_ERR("port free during pfa suspend! \n");
        return -EIO;
    }

    if ((portno >= PFA_PORT_NUM) || (!((1 << portno) & (pfa->portmap[PFA_PORTS_NOMARL])))) {
        PFA_ERR("port number %d err !\n", portno);
        return -ENOENT;
    }

    port_ctx = &pfa->ports[portno];
    port_ctrl = &port_ctx->ctrl;

    spin_lock_irqsave(&port_ctrl->lock, flags);
    if (port_ctrl->port_flags.enable) {
        spin_unlock_irqrestore(&port_ctrl->lock, flags);
        ret = bsp_pfa_disable_port(portno);
        if (ret) {
            port_ctx->stat.free_disable_fail++;
            PFA_ERR("port %u disenable fail in free!\n", portno);
        }

        return -EIO;
    }
    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    pfa_release_port_res(pfa, port_ctrl);
    pfa_port_clean_ctx(pfa, port_ctx);

    clear_bit(portno, &pfa->portmap[PFA_PORTS_NOMARL]);
    port_ctrl->port_flags.alloced = 0;

    return 0;
}

int pfa_alloc_br_port(void)
{
    unsigned int portno;
    struct pfa *pfa = &g_pfa;

    portno = find_first_zero_bit(&pfa->portmap[PFA_PORTS_BR], PFA_PORT_NUM);

    set_bit(portno, &pfa->portmap[PFA_PORTS_BR]);

    PFA_INFO("alloc a bridge chan.\n");

    return portno + PFA_PORT_NUM;
}

int pfa_free_br_port(unsigned portno)
{
    struct pfa *pfa = &g_pfa;

    if (!PFA_BR_PORT(portno)) {
        return -1;
    }

    portno -= PFA_PORT_NUM;

    if (!(BIT(portno) & (pfa->portmap[PFA_PORTS_BR]))) {
        PFA_ERR("port %d not allocated!\n", portno);
        return -ENOENT;
    }
    clear_bit(portno, &pfa->portmap[PFA_PORTS_BR]);

    return 0;
}

int pfa_get_free_portno(struct pfa_port_comm_attr *attr)
{
    unsigned int portno;
    struct pfa *pfa = &g_pfa;

    if (attr->enc_type == PFA_ENC_PFA_TFT) {
        portno = PFA_DEFAULT_PFA_TFT_PORT;
    } else if (attr->enc_type == PFA_ENC_CPU) {
        portno = PFA_DEFAULT_CPU_PORT;
    } else if (attr->rsv_port_id == PFA_ACORE_LAN_CTRL_ID) {
        portno = PFA_ACORE_LAN_CTRL_ID;
    } else {
        portno = find_first_zero_bit(&pfa->portmap[PFA_PORTS_NOMARL], PFA_CCORE_PORT_BEGIN);
    }
    set_bit(portno, &pfa->portmap[PFA_PORTS_NOMARL]);

    return portno;
}

int pfa_alloc_axi_desc_buffer(struct pfa_port_ctrl *port_ctrl)
{
    // alloc td desc
    port_ctrl->td_axi_buf_dma = 0;
    port_ctrl->axi_td = kzalloc(sizeof(struct pfa_td_desc), GFP_ATOMIC);
    if (port_ctrl->axi_td == NULL) {
        return -ENOMEM;
    }

    return 0;
}

int pfa_alloc_desc_queue(struct pfa_port_ctrl *port_ctrl, struct pfa_port_comm_attr *attr)
{
    struct pfa *pfa = &g_pfa;
    unsigned int td_desc_size;
    unsigned int rd_desc_size;
    int ret;

    td_desc_size = sizeof(struct pfa_td_desc) * port_ctrl->td_depth;
    rd_desc_size = sizeof(struct pfa_rd_desc) * port_ctrl->rd_depth;

    port_ctrl->td_addr = bsp_sc_alloc_ringbuf(pfa->dev, td_desc_size + rd_desc_size, &port_ctrl->td_dma, GFP_KERNEL);
    if (port_ctrl->td_addr == NULL) {
        PFA_ERR("port %d alloc td pool failed.\n", port_ctrl->portno);
        goto alloc_desc_fail;
    }

    port_ctrl->rd_addr = port_ctrl->td_addr + td_desc_size;
    port_ctrl->rd_dma = port_ctrl->td_dma + td_desc_size;

    port_ctrl->rd_long_buf = kzalloc(rd_desc_size + port_ctrl->td_depth * sizeof(struct sk_buff *), GFP_KERNEL);
    if (port_ctrl->rd_long_buf == NULL) {
        PFA_ERR("port %d alloc rd_long_buf failed.\n", port_ctrl->portno);
        goto alloc_desc_fail;
    }
    port_ctrl->td_param = (struct sk_buff **)(port_ctrl->rd_long_buf + rd_desc_size);

    ret = pfa_alloc_axi_desc_buffer(port_ctrl);
    if (ret) {
        goto alloc_desc_fail;
    }
    return 0;

alloc_desc_fail:
    if (port_ctrl->td_addr != NULL) {
        bsp_sc_free_ringbuf(pfa->dev, td_desc_size + rd_desc_size, port_ctrl->td_addr, port_ctrl->td_dma);
        port_ctrl->td_addr = NULL;
        port_ctrl->td_dma = 0;
    }

    if (port_ctrl->rd_long_buf != NULL) {
        kfree(port_ctrl->rd_long_buf);
        port_ctrl->rd_long_buf = NULL;
    }

    return -ENOMEM;
}

void pfa_direct_fw_enc_type_property_table_init(void)
{
    struct pfa *pfa = &g_pfa;

    pfa->type_property_table[PFA_ENC_NONE].enc_type = PFA_ENC_NONE;
    pfa->type_property_table[PFA_ENC_NONE].port_weight = DIRECT_FW_PE_PORT_WEIGHT;

    pfa->type_property_table[PFA_ENC_DIRECT_FW_NCM_NTB32].enc_type = PFA_ENC_NCM_NTB32;
    pfa->type_property_table[PFA_ENC_DIRECT_FW_NCM_NTB32].port_weight = DIRECT_FW_HP_PORT_WEIGHT;

    pfa->type_property_table[PFA_ENC_DIRECT_FW_NCM_NTB16].enc_type = PFA_ENC_NCM_NTB16;
    pfa->type_property_table[PFA_ENC_DIRECT_FW_NCM_NTB16].port_weight = DIRECT_FW_PE_PORT_WEIGHT;

    pfa->type_property_table[PFA_ENC_DIRECT_FW_RNDIS].enc_type = PFA_ENC_RNDIS;
    pfa->type_property_table[PFA_ENC_DIRECT_FW_RNDIS].port_weight = DIRECT_FW_HP_PORT_WEIGHT;

    pfa->type_property_table[PFA_ENC_DIRECT_FW_ECM].enc_type = PFA_ENC_ECM;
    pfa->type_property_table[PFA_ENC_DIRECT_FW_ECM].port_weight = DIRECT_FW_HP_PORT_WEIGHT;
}


static void pfa_port_property_init(struct pfa_port_ctrl *port_ctrl, struct pfa_port_comm_attr *attr)
{
    struct pfa *pfa = &g_pfa;
    /* copy param from attr */
    port_ctrl->property.bits.pfa_port_attach_brg = PFA_BRG_NORMAL;
    if (attr->enc_type == PFA_ENC_ECM) {
        port_ctrl->property.bits.pfa_port_enc_type = PFA_ENC_NONE;
    } else {
        port_ctrl->property.bits.pfa_port_enc_type = attr->enc_type;
    }
    port_ctrl->property.bits.pfa_port_pad_en = attr->padding_enable;
    port_ctrl->property.bits.pfa_dport_adq_core_type = PFA_DPORT_ADQ_AP_SEL; // use ap ad

    // direct fw
    if ((attr->enc_type >= PFA_ENC_DIRECT_FW_NCM_NTB16 && attr->enc_type <= PFA_ENC_DIRECT_FW_ECM) ||
        attr->direct_fw_en) {
        port_ctrl->direct_fw_ctx.port_weight = pfa->type_property_table[attr->enc_type].port_weight;
        port_ctrl->property.bits.pfa_port_enc_type = pfa->type_property_table[attr->enc_type].enc_type;
    }
    if (attr->enc_type > PFA_ENC_ECM && attr->enc_type < PFA_ENC_BOTTOM) {
        port_ctrl->property.bits.pfa_port_enc_type = PFA_ENC_NONE;
    }
    port_ctrl->enc_type = port_ctrl->property.bits.pfa_port_enc_type;

    return;
}

void pfa_config_vir_port_net(struct pfa_port_ctrl *port_ctrl, struct pfa_port_comm_attr *attr, unsigned int vir_portno, unsigned int id)
{
    struct pfa *pfa = &g_pfa;

    port_ctrl->net_map[id].net = attr->net;
    port_ctrl->net_map[id].portno = PFA_GET_PHY_PORT(vir_portno);
    port_ctrl->net_map[id].port_priv = attr->priv;
    if (memcpy_s(&port_ctrl->net_map[id].ops, sizeof(struct pfa_ops), &attr->ops, sizeof(struct pfa_ops))) {
        return;
    }
    pfa->enc_type_map[attr->enc_type].alloced = 1;
    pfa->enc_type_map[attr->enc_type].vir_portno = vir_portno;

    set_bit(id, &port_ctrl->port_alloc_map);
}

void pfa_portctrl_init(struct pfa *pfa, struct pfa_port_ctrl *port_ctrl, struct pfa_port_comm_attr *attr, unsigned int portno)
{
    struct pfa_port_ctx *port_ctx = &pfa->ports[portno];
    unsigned int td_depth = attr->td_depth > port_ctrl->td_depth ? attr->td_depth : port_ctrl->td_depth;
    unsigned int rd_depth = attr->rd_depth > port_ctrl->rd_depth ? attr->rd_depth : port_ctrl->rd_depth;

    if (memset_s(port_ctx, sizeof(*port_ctx), 0, sizeof(struct pfa_port_ctx))) {
        PFA_ERR("memset_s fail!\n");
    }
    port_ctrl->portno = portno;
    pfa_port_property_init(port_ctrl, attr);
    port_ctrl->td_depth = td_depth;
    port_ctrl->rd_depth = rd_depth;
    port_ctrl->ops.pfa_finish_rd = attr->ops.pfa_finish_rd;
    port_ctrl->ops.pfa_finish_rd_dra = attr->ops.pfa_finish_rd_dra;
    port_ctrl->ops.pfa_complete_rd = attr->ops.pfa_complete_rd;
    port_ctrl->ops.pfa_finish_td = attr->ops.pfa_finish_td;
    port_ctrl->ops.pfa_netif_rx = attr->ops.pfa_netif_rx;
    port_ctrl->port_priv = attr->priv;

    if (attr->net) {
        dev_hold(attr->net);
        port_ctrl->net = attr->net;
        dev_put(attr->net);
    } else {
        port_ctrl->net = NULL;
    }

    /* set some default value   */
    port_ctrl->priority = PFA_DEF_PORT_PRI;

    /* rate limit */
    port_ctrl->udp_limit_time = PORT_UDP_LIMIT_TIME_DEF;
    port_ctrl->udp_limit_cnt = PORT_UDP_LIMIT_CNT_DEF;
    port_ctrl->rate_limit_time = PORT_RATE_LIMIT_TIME_DEF;
    port_ctrl->rate_limit_byte = PORT_RATE_LIMIT_BYTE_DEF;

    port_ctrl->td_evt_gap = PORT_TD_EVT_GAP_DEF;
    port_ctrl->rd_evt_gap = PORT_RD_EVT_GAP_DEF;

    port_ctrl->dfs_speed_multiple_en = attr->dfs_speed_multiple_en;
    port_ctrl->dfs_speed_multiple = attr->dfs_speed_multiple;
    port_ctrl->dpa_en = attr->dpa_en;

    // bypass
    port_ctrl->bypassport = attr->bypassport;
    port_ctrl->bypassport_en = attr->bypassport_en;

    port_ctrl->td_copy_en = attr->td_copy_en;
    port_ctrl->stick_en = attr->stick_en;

    return;
}

int bsp_pfa_update_net(unsigned int vir_portno, struct net_device *ndev, unsigned int alloced)
{
    return 0;
}
static int pfa_alloc_port_desc(struct pfa *pfa, struct pfa_port_ctrl *port_ctrl, unsigned int portno,
    struct pfa_port_comm_attr *attr)
{
    switch (attr->enc_type) {
        case PFA_ENC_PFA_TFT:
            pfa->pfa_tftport.portno = portno;
            pfa->pfa_tftport.alloced = 1;
            if (pfa->cpuport.alloced) {
                pfa->ports[pfa->cpuport.portno].ctrl.bypassport = pfa->pfa_tftport.portno;
                pfa->ports[pfa->cpuport.portno].ctrl.bypassport_en = 1;
            }
            break;
        case PFA_ENC_NONE:
        case PFA_ENC_WIFI_MULTIPLE:
        case PFA_ENC_RELAY:
            break;
        case PFA_ENC_CPU:
            pfa->wanport.portno = portno;
            pfa->cpuport.portno = portno;
            pfa->cpuport.alloced = 1;
            pfa->wanport.get_wan_info = attr->ops.get_wan_info;
            if (pfa->pfa_tftport.alloced) {
                port_ctrl->bypassport = pfa->pfa_tftport.portno;
                port_ctrl->bypassport_en = 1;
            }
            break;
        case PFA_ENC_NCM_NTB16:
        case PFA_ENC_NCM_NTB32:
        case PFA_ENC_RNDIS:
        case PFA_ENC_ECM:
        case PFA_ENC_DIRECT_FW_NCM_NTB16:
        case PFA_ENC_DIRECT_FW_NCM_NTB32:
        case PFA_ENC_DIRECT_FW_RNDIS:
        case PFA_ENC_DIRECT_FW_ECM:
            if (pfa->usbport.alloced) {
                return -EINVAL;
            }
            port_ctrl->port_flags.copy_port = 1;
            pfa->usbport.portno = portno;
            pfa->usbport.alloced = 1;
            break;
        default:
            return -EINVAL;
    }
    return pfa_alloc_desc_queue(port_ctrl, attr);
}
static int pfa_portdesc_init(struct pfa *pfa, unsigned int portno, struct pfa_port_comm_attr *attr)
{
    struct pfa_port_ctrl *port_ctrl = &pfa->ports[portno].ctrl;
    int ret;

    port_ctrl->port_flags.copy_port = 0;
    port_ctrl->ext_desc = 0;

    ret = pfa_alloc_port_desc(pfa, port_ctrl, portno, attr);
    if (ret) {
        PFA_ERR("port %d alloc desc failed.\n", portno);
        return ret;
    }
    // td queue
    pfa->hal->config_port_td_pri(pfa, portno, PFA_DEF_PORT_PRI);

    pfa_writel_relaxed(pfa->regs, PFA_TDQX_BADDR_L(portno), lower_32_bits(port_ctrl->td_dma));
    pfa_writel_relaxed(pfa->regs, PFA_TDQX_BADDR_H(portno), upper_32_bits(port_ctrl->td_dma));

    // rd queue
    pfa_writel_relaxed(pfa->regs, PFA_RDQX_BADDR_L(portno), lower_32_bits(port_ctrl->rd_dma));
    pfa_writel_relaxed(pfa->regs, PFA_RDQX_BADDR_H(portno), upper_32_bits(port_ctrl->rd_dma));

    pfa->hal->config_port_td_rd_info(portno, port_ctrl->td_depth, port_ctrl->rd_depth);

    return 0;
}

void pfa_intr_set_interval(unsigned irq_interval)
{
    struct pfa *pfa = &g_pfa;

    pfa->irq_interval = irq_interval;
    pfa_writel(pfa->regs, PFA_INTA_INTERVAL, pfa->irq_interval);
    pfa->dfs_div_threshold_pktnum = PFA_PER_INTERVAL_CALC(PFA_CLK_DIV_THR_RATE);
}

void pfa_set_cpuport_maxrate(unsigned rate)
{
    struct pfa *pfa = &g_pfa;

    pfa->cpuport.cpu_pkt_max_rate = rate;
}

void pfa_en_port_stick_mode(struct pfa *pfa, unsigned int bypassport)
{
    pfa_stick_mode_t pfa_mode;

    pfa_mode.bits.stick_en = PFA_PORT_STICK_ENABLE;
    pfa_mode.bits.bypass_en = PFA_PORT_STICK_ENABLE;
    pfa_mode.bits.bypass_dport = bypassport;
    pfa_mode.bits.reserved = 0;

    pfa_writel(pfa->regs, PFA_STICK_MODE, pfa_mode.u32);
    return;
}

void pfa_port_set_stick_mode(struct pfa *pfa, struct pfa_port_ctrl *port_ctrl, struct pfa_port_comm_attr *attr,
    unsigned int portno)
{
    if ((attr->direct_fw_en) || (attr->enc_type == PFA_ENC_DIRECT_FW_NCM_NTB16) ||
        (attr->enc_type == PFA_ENC_DIRECT_FW_NCM_NTB32) || (attr->enc_type == PFA_ENC_DIRECT_FW_RNDIS) ||
        (attr->enc_type == PFA_ENC_DIRECT_FW_ECM)) {
        port_ctrl->direct_fw_ctx.stream_cnt = 1;
        port_ctrl->direct_fw_ctx.enable = 1;
        port_ctrl->direct_fw_ctx.alloced = 1;
        port_ctrl->direct_fw_ctx.port_no = portno;
        if (pfa->direct_fw.dfw_flags.enable == 0 && pfa->direct_fw.direct_fw_timer_add == 0) {
            add_timer(&pfa->direct_fw.direct_fw_timer);
            pfa->direct_fw.direct_fw_timer_add = 1;
        }
        pfa->direct_fw.dfw_flags.enable = 1;
        INIT_LIST_HEAD(&port_ctrl->direct_fw_ctx.stream_list);

        pfa->direct_fw.port_nums[pfa->direct_fw.port_cnt] = portno;
        pfa->direct_fw.port_cnt++;
    } else {
        port_ctrl->direct_fw_ctx.stream_cnt = 0;
        port_ctrl->direct_fw_ctx.enable = 0;
        port_ctrl->direct_fw_ctx.alloced = 0;
    }

    if (port_ctrl->portno == PFA_DEFAULT_PFA_TFT_PORT && port_ctrl->stick_en) {
        pfa_en_port_stick_mode(pfa, port_ctrl->bypassport);
    }

    return;
}
void pfa_port_set_reg(struct pfa *pfa, struct pfa_port_ctrl *port_ctrl, struct pfa_port_comm_attr *attr,
    unsigned int portno)
{
    unsigned int evt_rd_full_mask, evt_rd_done_mask, evt_td_done_mask;

    // when disable / enable port, rd may be not in start pos.
    // we update the free / busy slot from the hardware.
    // rd_busy must restore to rptr
    port_ctrl->td_free = pfa_readl(pfa->regs, PFA_TDQX_WPTR(portno));
    port_ctrl->td_busy = pfa_readl(pfa->regs, PFA_TDQX_RPTR(portno));
    // be care, rd_busy must restore to rptr
    port_ctrl->rd_busy = pfa_readl(pfa->regs, PFA_RDQX_RPTR(portno));
    port_ctrl->rd_free = port_ctrl->rd_busy;


    // set port property
    pfa_writel_relaxed(pfa->regs, PFA_PORTX_PROPERTY(portno), port_ctrl->property.u32);
    if (pfa->hal->config_port_enc_type) {
        pfa->hal->config_port_enc_type(pfa, portno, port_ctrl->enc_type);
    }
    pfa->hal->config_port_td_pri(pfa, portno, port_ctrl->priority);

    if (attr->enc_type != PFA_ENC_PFA_TFT) {
        // set mac addr
        if (!((attr->enc_type >= PFA_ENC_NCM_NTB16 && attr->enc_type <= PFA_ENC_RNDIS) || attr->enc_type == PFA_ENC_ECM ||
               attr->enc_type == PFA_ENC_DIRECT_FW_NCM_NTB16 || attr->enc_type == PFA_ENC_DIRECT_FW_NCM_NTB32 ||
               attr->enc_type == PFA_ENC_DIRECT_FW_RNDIS || attr->enc_type == PFA_ENC_DIRECT_FW_ECM)) {
            pfa_port_set_mac(portno, port_ctrl->net->dev_addr);
        }
        if (attr->dpa_en == 0) {
            evt_rd_done_mask = pfa->rd_done_mask & ~(1 << portno);
            pfa_writel(pfa->regs, PFA_RD_DONE_INTA_MASK, evt_rd_done_mask);
            pfa->rd_done_mask = evt_rd_done_mask;
        }
    }
    if (attr->dpa_en == 0) {
        if (pfa->td_fail_generate_rd == 0) {
            evt_rd_full_mask = pfa_readl(pfa->regs, PFA_RDQ_FULL_INTA_MASK) & ~(1 << portno);
            pfa_writel(pfa->regs, PFA_RDQ_FULL_INTA_MASK, evt_rd_full_mask);
        }
        evt_td_done_mask = pfa_readl(pfa->regs, PFA_TD_DONE_INTA_MASK) & ~(1 << portno);
        pfa_writel(pfa->regs, PFA_TD_DONE_INTA_MASK, evt_td_done_mask);
    }

    spin_lock_init(&port_ctrl->td_lock);
    spin_lock_init(&port_ctrl->rd_lock);
    spin_lock_init(&port_ctrl->lock);
    port_ctrl->port_flags.alloced = 1;
    return;
}

void pfa_config_dport_adq_core_type(unsigned int portno, unsigned int type)
{
    struct pfa *pfa = &g_pfa;

    pfa->hal->config_dport_adq_core_type(pfa, portno, type);
}

int pfa_config_vir_port(struct pfa *pfa, struct pfa_port_comm_attr *attr)
{
    pfa_port_t vir_portno;
    unsigned int portno;
    struct pfa_port_ctrl *port_ctrl;

    portno = PFA_GET_PHY_PORT(pfa->enc_type_map[attr->enc_type].vir_portno);
    port_ctrl = &pfa->ports[portno].ctrl;
    vir_portno.u32 = portno;

    vir_portno.bits.net_id = find_first_zero_bit(&port_ctrl->port_alloc_map, PFA_PORT_NUM);

    pfa_config_vir_port_net(port_ctrl, attr, vir_portno.u32, vir_portno.bits.net_id);

    port_ctrl->port_multiple_en = 1;

    pfa->enc_type_map[attr->enc_type].vir_portno = vir_portno.u32;

    return vir_portno.u32;
}
int bsp_pfa_alloc_port(struct pfa_port_comm_attr *attr)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int portno;
    unsigned long flag;
    spinlock_t *port_alloc_lock = &pfa->port_alloc_lock;
    int ret;

    if (g_pfa.init == 0) {
        PFA_ERR("bsp_pfa_alloc_port fail, pfa no init\n");
        return -1;
    }

    if (unlikely(pfa->flags & PFA_FLAG_SUSPEND)) {
        PFA_ERR("port alloc during pfa suspend! \n");
        return -EIO;
    }

    // check validity of attr param
    ret = pfa_port_check_attr(attr);
    if (ret) {
        return ret;
    }

    spin_lock_irqsave(port_alloc_lock, flag);
    if (pfa->enc_type_map[attr->enc_type].port_multiple_en && pfa->enc_type_map[attr->enc_type].alloced) {
        portno = pfa_config_vir_port(pfa, attr);
        spin_unlock_irqrestore(port_alloc_lock, flag);
        return portno;
    }

    portno = pfa_get_free_portno(attr);
    if ((portno < 0) || (portno >= PFA_CCORE_PORT_BEGIN)) {
        spin_unlock_irqrestore(port_alloc_lock, flag);
        return -1;
    }

    spin_unlock_irqrestore(port_alloc_lock, flag);

    port_ctrl = &pfa->ports[portno].ctrl;
    pfa_portctrl_init(pfa, port_ctrl, attr, portno);
    pfa_config_vir_port_net(port_ctrl, attr, portno, 0);

    ret = pfa_portdesc_init(pfa, portno, attr);
    if (ret) {
        goto init_desc_fail;
    }

    pfa_port_set_reg(pfa, port_ctrl, attr, portno);
    pfa_port_set_stick_mode(pfa, port_ctrl, attr, portno);
    PFA_INFO("port alloc succ ! \n");

    return portno;

init_desc_fail:
    clear_bit(portno, &pfa->portmap[PFA_PORTS_NOMARL]);
    PFA_ERR("port alloc fail! \n");
    BUG_ON(1);
    return ret;
}

int bsp_pfa_free_port(int vir_portno)
{
    struct pfa *pfa = &g_pfa;
    int portno = PFA_GET_PHY_PORT(vir_portno);
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int net_id = PFA_NET_ID_BY_VIR_PORT(vir_portno);

    if (!pfa_port_exist(pfa, portno)) {
        return -1;
    }

    port_ctrl = &pfa->ports[portno].ctrl;
    clear_bit(net_id, &port_ctrl->port_alloc_map);
    if (port_ctrl->port_alloc_map) {
        return 0;
    }
    return pfa_port_free(portno);
}

int bsp_pfa_set_portmac(int portno, const char *mac_addr)
{
    const char *mac = mac_addr;
    unsigned int port_num;
    int ret;

    port_num = (unsigned int)PFA_GET_PHY_PORT(portno);

    ret = pfa_port_set_mac(port_num, mac);

    return ret;
}

int mdrv_pfa_open_port(struct mdrv_pfa_port_info_s* info)
{
    struct pfa_port_comm_attr relay_port_attr = {0};
    struct pfa *pfa = &g_pfa;
    int portno;

    if (g_pfa.init == 0) {
        PFA_ERR("mdrv_pfa_open_port fail, pfa no init\n");
        return -1;
    }

    relay_port_attr.rd_depth = pfa->relayport.rd_depth;
    relay_port_attr.td_depth = pfa->relayport.td_depth;
    relay_port_attr.ops.pfa_finish_rd = info->ops.rx;
    relay_port_attr.ops.pfa_netif_rx = info->ops.tx;
    relay_port_attr.ops.pfa_complete_rd = info->ops.rx_complete;
    relay_port_attr.priv = info->ops.para;
    relay_port_attr.net = info->dev;
    relay_port_attr.enc_type = PFA_ENC_RELAY;
    portno = bsp_pfa_alloc_port(&relay_port_attr);
    if (portno < 0) {
        PFA_ERR("pfa relay port alloc failed \n");
        return -1;
    }
    pfa->relayport.portno = portno;

    bsp_pfa_enable_port(portno);
    return portno;
}

int mdrv_pfa_close_port(int port)
{
    struct pfa *pfa = &g_pfa;
    int portno = (unsigned int)PFA_GET_PHY_PORT(port);
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int net_id = PFA_NET_ID_BY_VIR_PORT(port);

    if (!pfa_port_exist(pfa, portno)) {
        return -1;
    }
    port_ctrl = &pfa->ports[portno].ctrl;
    clear_bit(net_id, &port_ctrl->port_alloc_map);

    bsp_pfa_disable_port(port);

    return 0;
}

static void pfa_set_usb_pack_attr(struct pfa *pfa, struct pfa_usbport_attr *usb_attr)
{
    pfa_unpack_max_len_t usb_unpack_len;

    // set default settings
    usb_unpack_len.bits.pfa_ncm_max_len = (unsigned int)PFA_DEF_USB_MAX_SIZE;
    usb_unpack_len.bits.pfa_rndis_max_len = (unsigned int)PFA_DEF_USB_MAX_SIZE;

    pfa->usbport.attr.gether_max_pkt_cnt = PFA_DEF_USB_ENC_PKT_CNT;

    pfa_writel_relaxed(pfa->regs, PFA_UNPACK_MAX_LEN, usb_unpack_len.u32);
    pfa->hal->update_usb_max_pkt_cnt(pfa, PFA_DEF_USB_ENC_PKT_CNT);
    pfa->hal->update_pack_max_len(pfa, PFA_DEF_USB_MAX_SIZE);
    pfa_writel_relaxed(pfa->regs, PFA_PACK_REGION_DIV, PFA_GETHER_RD_DIV);
    pfa_writel_relaxed(pfa->regs, PFA_PACK_ADDR_CTRL, PFA_DEF_PACK_ALIGN);
    pfa_writel_relaxed(pfa->regs, PFA_UNPACK_CTRL, PFA_DEF_USB_UNPACK_MAX_SIZE);

    if (usb_attr->gether_align_parameter != 0) {
        pfa->usbport.attr.gether_align_parameter = usb_attr->gether_align_parameter;
        pfa_writel_relaxed(pfa->regs, PFA_PACK_ADDR_CTRL, usb_attr->gether_align_parameter);
    }

    if (usb_attr->gether_max_pkt_cnt != 0) {
        pfa->usbport.attr.gether_max_pkt_cnt = usb_attr->gether_max_pkt_cnt;

        pfa->hal->update_usb_max_pkt_cnt(pfa, usb_attr->gether_max_pkt_cnt);
    }

    if (usb_attr->gether_max_size != 0) {
        pfa->usbport.attr.gether_max_size = usb_attr->gether_max_size;
        usb_unpack_len.bits.pfa_ncm_max_len = usb_attr->gether_max_size;
        usb_unpack_len.bits.pfa_rndis_max_len = usb_attr->gether_max_size;

        pfa->hal->update_pack_max_len(pfa, usb_attr->gether_max_size);
        pfa_writel_relaxed(pfa->regs, PFA_UNPACK_MAX_LEN, usb_unpack_len.u32);
    }

    if (usb_attr->gether_timeout != 0) {
        pfa->usbport.attr.gether_timeout = usb_attr->gether_timeout;
        pfa_writel_relaxed(pfa->regs, PFA_PACK_MAX_TIME, usb_attr->gether_timeout);
    }
    return;
}
static int pfa_check_usbproperty_para(struct pfa *pfa, struct pfa_usbport_attr *usb_attr)
{
    if (usb_attr == NULL) {
        PFA_ERR("in put usb param err \n");
        return -EINVAL;
    }

    if (!pfa->usbport.alloced) {
        PFA_ERR("no usb port err \n");
        return -EINVAL;
    }

    if (!usb_attr->net) {
        PFA_ERR("input net dev NULL.\n");
        return -EINVAL;
    }

    if ((usb_attr->enc_type < PFA_ENC_NCM_NTB16 || usb_attr->enc_type > PFA_ENC_RNDIS) &&
        (usb_attr->enc_type != PFA_ENC_ECM)) {
            PFA_ERR("port enc type not usb! \n");
            BUG_ON(1);
            return -EINVAL;
    }

    return 0;
}

int bsp_pfa_update_bypass_portno(unsigned int src_portno_vir, unsigned int dst_portno_vir)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int src_portno = PFA_GET_PHY_PORT(src_portno_vir);
    unsigned int dst_portno = PFA_GET_PHY_PORT(dst_portno_vir);

    if (!pfa_port_exist(pfa, src_portno)) {
        PFA_ERR("src port(%d) is not exist.\n", src_portno);
        return -EINVAL;
    }

    if (!pfa_port_exist(pfa, dst_portno)) {
        PFA_ERR("dst port(%d) is not exist.\n", dst_portno);
        return -EINVAL;
    }

    port_ctrl = &pfa->ports[src_portno].ctrl;
    port_ctrl->bypassport_en = 1;
    port_ctrl->bypassport = dst_portno;
    return 0;
}

int bsp_pfa_set_usbproperty(struct pfa_usbport_attr *usb_attr)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int portno;
    int ret;

    ret = pfa_check_usbproperty_para(pfa, usb_attr);
    if (ret) {
        return ret;
    }

    portno = pfa->usbport.portno;
    port_ctrl = &pfa->ports[portno].ctrl;

    dev_hold(usb_attr->net);
    port_ctrl->net = usb_attr->net;
    pfa->usbport.attr.net = usb_attr->net;
    dev_put(usb_attr->net);

    ret = pfa_port_set_mac(portno, port_ctrl->net->dev_addr);
    if (ret) {
        PFA_ERR("pfa_port_set_mac fail \n");
    }

    pfa->usbport.attr.enc_type = usb_attr->enc_type;
    if (usb_attr->enc_type == PFA_ENC_ECM) {
        port_ctrl->property.bits.pfa_port_enc_type = PFA_ENC_NONE;
    } else {
        port_ctrl->property.bits.pfa_port_enc_type = usb_attr->enc_type;
    }
    port_ctrl->enc_type = port_ctrl->property.bits.pfa_port_enc_type;
    pfa_writel(pfa->regs, PFA_PORTX_PROPERTY(portno), port_ctrl->property.u32);
    if (pfa->hal->config_port_enc_type) {
        pfa->hal->config_port_enc_type(pfa, portno, port_ctrl->enc_type);
    }

    pfa_set_usb_pack_attr(pfa, usb_attr);

    return 0;
}

void pfa_set_port_property_brg(unsigned int portno, unsigned int brg_flag)
{
    pfa_port_prop_t value;
    struct pfa *pfa = &g_pfa;

    value.u32 = pfa_readl(pfa->regs, PFA_PORTX_PROPERTY(portno));

    value.bits.pfa_port_attach_brg = brg_flag;

    pfa_writel(pfa->regs, PFA_PORTX_PROPERTY(portno), value.u32);
}

int bsp_pfa_set_pfa_tftproperty(struct pfa_tft_port_attr *attr)
{
    struct pfa *pfa = &g_pfa;
    struct iport_pfa_addr *pfa_reg = NULL;
    struct iport_pfa_tft_addr *pfa_tft_reg = NULL;
    unsigned int portno;

    if (attr == NULL) {
        PFA_ERR("in put pfa_tft param err \n");
        return -EINVAL;
    }

    pfa->pfa_tftport.check_pfa_tft_bd = attr->check_pfa_tft_bd;
    pfa_reg = &attr->pfa_addr;
    pfa_tft_reg = &attr->pfa_tft_addr;

    portno = pfa->pfa_tftport.portno;
    if (!pfa->pfa_tftport.alloced) {
        PFA_ERR("pfa_tft ctx port NULL. \n");
        return -1;
    }

    if ((pfa_reg == NULL) && (pfa_tft_reg == NULL)) {
        PFA_ERR("pfa_tft ctx input error. \n");
        return -EINVAL;
    }

    pfa->pfa_tftport.pfa_push_addr = (char *)dma_alloc_coherent(pfa->dev, sizeof(unsigned int),
        &pfa->pfa_tftport.pfa_push_dma, GFP_KERNEL);

    if (pfa->pfa_tftport.pfa_push_addr == NULL) {
        PFA_ERR("pfa_tft pfa_push_addr alloc err. \n");
        return -1;
    }

    pfa_reg->dltd_base_addr = pfa->ports[portno].ctrl.td_dma;
    pfa_reg->ulrd_base_addr = pfa->ports[portno].ctrl.rd_dma;
    pfa_reg->dltd_base_addr_v = pfa->ports[portno].ctrl.td_addr;
    pfa_reg->ulrd_base_addr_v = pfa->ports[portno].ctrl.rd_addr;
    pfa_reg->ulrd_rptr_addr = pfa->res + PFA_RDQX_RPTR(portno);
    pfa_reg->dltd_wptr_addr = pfa->res + PFA_TDQX_WPTR(portno);
    pfa_reg->ulrd_rptr_addr_v = (unsigned char *)pfa->regs + PFA_RDQX_RPTR(portno);
    pfa_reg->dltd_wptr_addr_v = (unsigned char *)pfa->regs + PFA_TDQX_WPTR(portno);

    pfa->hal->config_tft_bd_rd_ptr_update(pfa, pfa_tft_reg);

    PFA_INFO("pfa_tft port property set complete \n");

    return 0;
}

void bsp_pfa_set_pfa_tftmac(struct ethhdr *mac_addr)
{
    if (mac_addr == NULL) {
        return;
    }

    pfa_set_pfa_tft_mac(mac_addr);

    return;
}

int pfa_enable_port(int portno)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned long flags = 0;

    if (!pfa_port_exist(pfa, portno)) {
        return -ENOENT;
    }

    port_ctrl = &pfa->ports[portno].ctrl;

    spin_lock_irqsave(&port_ctrl->lock, flags);

    if (port_ctrl->port_flags.enable) {
        spin_unlock_irqrestore(&port_ctrl->lock, flags);
        PFA_DBG("port %d is enabled\n", portno);
        return 0;
    }

    pfa->hal->enable_port(pfa, portno, 1);
    pfa->hal->config_port_td_pri(pfa, portno, PFA_DEF_PORT_PRI);

    port_ctrl->port_flags.enable = 1;

    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    return 0;
}

int bsp_pfa_enable_port(int vir_portno)
{
    struct pfa *pfa = &g_pfa;
    int portno = PFA_GET_PHY_PORT(vir_portno);
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int net_id = PFA_NET_ID_BY_VIR_PORT(vir_portno);

    if (!pfa_port_exist(pfa, portno)) {
        return -ENOENT;
    }

    port_ctrl = &pfa->ports[portno].ctrl;
    set_bit(net_id, &port_ctrl->port_enable_map);

    PFA_INFO("port %d enable called\n", portno);
    return pfa_enable_port(portno);
}

int mdrv_pfa_enable_port(int portno)
{
    return bsp_pfa_enable_port(portno);
}

int pfa_disable_port(struct pfa *pfa, int port_num)
{
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned long flags = 0;
    unsigned int portno;

    PFA_TRACE("enter\n");

    if (port_num < 0) {
        PFA_ERR("pfa port %d err\n", port_num);
        return -EINVAL;
    } else {
        portno = (unsigned int)port_num;
    }

    if (!pfa_port_exist(pfa, portno)) {
        PFA_ERR("pfa port %d not exist\n", port_num);
        return -ENOENT;
    }

    port_ctrl = &pfa->ports[portno].ctrl;

    spin_lock_irqsave(&port_ctrl->lock, flags);
    if (!port_ctrl->port_flags.enable) {
        spin_unlock_irqrestore(&port_ctrl->lock, flags);
        PFA_DBG("port %u is disabled\n", portno);
        return 0;
    }
    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    pfa->hal->enable_port(pfa, portno, 0);
    port_ctrl->port_flags.enable = 0;

    PFA_TRACE("leave\n");

    return 0;
}

int bsp_pfa_disable_port(int vir_port_num)
{
    struct pfa *pfa = &g_pfa;
    int ret;
    int port_num = PFA_GET_PHY_PORT(vir_port_num);
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int net_id = PFA_NET_ID_BY_VIR_PORT(vir_port_num);

    if (!pfa_port_exist(pfa, port_num)) {
        return -ENOENT;
    }

    port_ctrl = &pfa->ports[port_num].ctrl;
    clear_bit(net_id, &port_ctrl->port_enable_map);
    if (port_ctrl->port_enable_map) {
        return 0;
    }

    PFA_INFO("port %d disable called\n", port_num);
    ret = pfa_disable_port(pfa, port_num);
    if (ret) {
        PFA_ERR("pfa disable port %d fail.\n", port_num);
        return ret;
    }
    if (unlikely(pfa->modem_resetting == 1)) {
        return 0;
    }

    ret = pfa_recycle_port_queue_desc(pfa, port_num);
    if (ret) {
        PFA_ERR("pfa recycle port:%d queue desc fail.\n", port_num);
        return ret;
    }

    return 0;
}

int mdrv_pfa_disable_port(int portno)
{
    return bsp_pfa_disable_port(portno);
}

int bsp_pfa_get_reserve_port_id(enum pfa_reserve_port_vir_id id)
{
    struct pfa *pfa = &g_pfa;

    if (id >= PFA_RSV_PORT_VIR_ID_MAX) {
        return -1;
    }
    return pfa->rsv_port_map[id];
}


void pfa_print_packet_stream(void)
{
    struct pfa_port_ctrl *ctrl = NULL;
    struct pfa_port_stat *stat = NULL;
    int i;

    for (i = 0; i < PFA_PORT_NUM; i++) {
        ctrl = &g_pfa.ports[i].ctrl;
        stat = &g_pfa.ports[i].stat;
        PFA_ERR("port[i]:%d ", ctrl->portno);
    }
    PFA_ERR("\ntd: \n");

    for (i = 0; i < PFA_PORT_NUM; i++) {
        stat = &g_pfa.ports[i].stat;
        PFA_ERR("%d ", stat->rd_config);
    }
    PFA_ERR("\ntd complete: \n");

    for (i = 0; i < PFA_PORT_NUM; i++) {
        stat = &g_pfa.ports[i].stat;
        PFA_ERR("%d ", stat->td_finsh_intr_complete);
    }
    PFA_ERR("\nrd complete: \n");

    for (i = 0; i < PFA_PORT_NUM; i++) {
        stat = &g_pfa.ports[i].stat;
        PFA_ERR("%d ", stat->td_config);
    }
    PFA_ERR("\nrd: \n");

    for (i = 0; i < PFA_PORT_NUM; i++) {
        stat = &g_pfa.ports[i].stat;
        PFA_ERR("%d ", stat->rd_finished);
    }
    PFA_ERR("\n");

    return;
}

void pfa_rwptr_show(void)
{
    struct pfa *pfa = &g_pfa;
    unsigned int rd_rptr;
    unsigned int rd_wptr;
    unsigned int td_rptr;
    unsigned int td_wptr;
    unsigned int portno;

    for (portno = 0; portno < PFA_PORT_NUM; portno++) {
        if (test_bit(portno, &pfa->portmap[PFA_PORTS_NOMARL])) {
            rd_rptr = pfa_readl_relaxed(pfa->regs, PFA_RDQX_RPTR(portno));
            rd_wptr = pfa_readl_relaxed(pfa->regs, PFA_RDQX_WPTR(portno));
            td_rptr = pfa_readl_relaxed(pfa->regs, PFA_TDQX_RPTR(portno));
            td_wptr = pfa_readl_relaxed(pfa->regs, PFA_TDQX_WPTR(portno));

            PFA_ERR("portno:%d, rd_rptr = 0x%x rd_wptr = 0x%x td_rptr = 0x%x td_wptr = 0x%x \n", portno, rd_rptr,
                    rd_wptr, td_rptr, td_wptr);
        }
    }

    if (pfa->pfa_tftport.check_pfa_tft_bd != NULL) {
        pfa->pfa_tftport.check_pfa_tft_bd();
    }
    return;
}

void pfa_rsv_port_map_init(struct pfa *pfa)
{
    pfa->rsv_port_map[PFA_ACORE_LAN_CTRL_VIR_ID] = PFA_ACORE_LAN_CTRL_ID;
}

void pfa_enc_type_map_init(struct pfa *pfa)
{
    pfa->enc_type_map[PFA_ENC_WIFI_MULTIPLE].port_multiple_en = 1;
    pfa->enc_type_map[PFA_ENC_RELAY].port_multiple_en = 1;
    if (pfa->hal->enc_type_init != NULL) {
        pfa->hal->enc_type_init(pfa);
    }
}

void pfa_relay_port_config_init(struct pfa *pfa)
{
    int ret;

    ret = bsp_dt_property_read_u32_array(pfa->np, "relay_td_depth", &pfa->relayport.td_depth, 1);
    if (ret || pfa->relayport.td_depth > PFA_RD_NUM_MAX) {
        pfa->relayport.td_depth = PFA_RELAY_NUM_DEFAULT;
    }

    ret = bsp_dt_property_read_u32_array(pfa->np, "relay_rd_depth", &pfa->relayport.rd_depth, 1);
    if (ret || pfa->relayport.rd_depth > PFA_RD_NUM_MAX) {
        pfa->relayport.rd_depth = PFA_RELAY_NUM_DEFAULT;
    }
}

void pfa_port_init(struct pfa *pfa)
{
    /* Rsv Port for cpu/pfa_tft/ndis ctrl */
    pfa_rsv_port_map_init(pfa);
    pfa->portmap[PFA_PORTS_NOMARL] = PFA_PORT_MAP_DEFAULT; /* portmap default value for reserve port */

    pfa_enc_type_map_init(pfa);

    pfa->rd_loop_cnt = PFA_RD_MAX_GET_TIMES;

    pfa_direct_fw_enc_type_property_table_init();

    spin_lock_init(&pfa->port_alloc_lock);

    return;
}

int mdrv_pfa_port_alloc(struct pfa_port_comm_attr *attr)
{
    return bsp_pfa_alloc_port(attr);
}

int mdrv_pfa_port_free(int portno)
{
    return bsp_pfa_free_port(portno);
}

int mdrv_pfa_set_macaddr(int portno, const char *mac_addr)
{
    return bsp_pfa_set_portmac(portno, mac_addr);
}

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(bsp_pfa_alloc_port);
EXPORT_SYMBOL(bsp_pfa_disable_port);
EXPORT_SYMBOL(bsp_pfa_enable_port);
EXPORT_SYMBOL(bsp_pfa_set_usbproperty);
EXPORT_SYMBOL(pfa_en_port_stick_mode);
EXPORT_SYMBOL(pfa_rwptr_show);
EXPORT_SYMBOL(pfa_print_packet_stream);
EXPORT_SYMBOL(bsp_pfa_set_portmac);
EXPORT_SYMBOL(pfa_set_port_property_brg);
EXPORT_SYMBOL(bsp_pfa_free_port);
EXPORT_SYMBOL(bsp_pfa_set_pfa_tftproperty);
EXPORT_SYMBOL(bsp_pfa_update_bypass_portno);
EXPORT_SYMBOL(bsp_pfa_update_net);
EXPORT_SYMBOL(bsp_pfa_set_pfa_tftmac);
EXPORT_SYMBOL(pfa_set_cpuport_maxrate);
EXPORT_SYMBOL(mdrv_pfa_port_alloc);
EXPORT_SYMBOL(mdrv_pfa_port_free);
EXPORT_SYMBOL(mdrv_pfa_set_macaddr);
EXPORT_SYMBOL(mdrv_pfa_open_port);
EXPORT_SYMBOL(mdrv_pfa_enable_port);
EXPORT_SYMBOL(mdrv_pfa_disable_port);

