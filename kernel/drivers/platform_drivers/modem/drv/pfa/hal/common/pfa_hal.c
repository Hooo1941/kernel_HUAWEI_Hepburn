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
 *
 */

#include "pfa_hal_reg.h"
#include "pfa.h"
#include "pfa_hal.h"
#include "pfa_core.h"


void transfer_ctx_init(struct pfa *pfa)
{
    int i;
    /* set timeout */
    pfa_writel(pfa->regs, PFA_PACK_MAX_TIME, pfa->usbport.attr.gether_timeout);

    pfa_writel(pfa->regs, PFA_ETH_MINLEN, pfa->min_pkt_len);

    config_tft_bd_rd_ip_head(pfa);

    for (i = 0; i < PFA_PORT_NUM; i++) {
        /* set pfa limit rate to the max */
        pfa_writel(pfa->regs, PFA_PORTX_UDP_LMTNUM(i), PFA_PORT_UDP_LIM_MAX);
        pfa_writel(pfa->regs, PFA_PORTX_LMTBYTE(i), PFA_PORT_BYTE_LIM_MAX);
    }
}

void config_hash_value(struct pfa *pfa, unsigned int hash)
{
    pfa_writel_relaxed(pfa->regs, PFA_HASH_VALUE, hash);
}

static inline unsigned int get_tdq_rptr(struct pfa *pfa, unsigned int portno)
{
    return pfa_readl(pfa->regs, PFA_TDQX_RPTR(portno));
}
static inline unsigned int get_tdq_wptr(struct pfa *pfa, unsigned int portno)
{
    return pfa_readl(pfa->regs, PFA_TDQX_WPTR(portno));
}
static inline unsigned int get_rdq_rptr(struct pfa *pfa, unsigned int portno)
{
    return pfa_readl(pfa->regs, PFA_RDQX_RPTR(portno));
}
static inline unsigned int get_rdq_wptr(struct pfa *pfa, unsigned int portno)
{
    return pfa_readl(pfa->regs, PFA_RDQX_WPTR(portno));
}

static inline void set_tdq_wptr(struct pfa *pfa, unsigned int portno, unsigned int ptr)
{
    pfa_writel(pfa->regs, PFA_TDQX_WPTR(portno), ptr);
}

static inline void set_rdq_rptr(struct pfa *pfa, unsigned int portno, unsigned int ptr)
{
    pfa_writel(pfa->regs, PFA_RDQX_RPTR(portno), ptr);
}

static inline void set_rdq_wptr(struct pfa *pfa, unsigned int portno, unsigned int ptr)
{
    pfa_writel(pfa->regs, PFA_RDQX_WPTR(portno), ptr);
}

struct pfa_hal g_pfa_hal = {
    .transfer_ctx_init = transfer_ctx_init,
    .config_tft_bd_rd_ip_head = config_tft_bd_rd_ip_head,
    .tft_config_rd_ip_head = tft_config_rd_ip_head,
    .config_tft_bd_rd_ptr_update = config_tft_bd_rd_ptr_update,
    .queue_portx_save = queue_portx_save,
    .queue_portx_restore = queue_portx_restore,
    .config_hash_cache_en = config_hash_cache_en,
    .config_hash_value = config_hash_value,
    .config_adq_threshold_and_len = config_adq_threshold_and_len,
    .config_qos_lmttime = config_qos_lmttime,
    .config_eth_vlan_tag_sel = config_eth_vlan_tag_sel,
    .config_dport_adq_core_type = config_dport_adq_core_type,
    .set_update_only_produce_mod = set_update_only_produce_mod,
    .enable_macfw_to_ip = enable_macfw_to_ip,
    .clk_gate_switch = pfa_clk_gate_switch,
    .check_td_result = pfa_check_td_result,
    .rd_record_result = pfa_rd_record_result,
    .config_td_tft_mode = pfa_config_td_tft_mode,
    .dbgen_en = dbgen_en,
    .dbgen_dis = dbgen_dis,
    .check_idle = pfa_check_idle,
    .ackpkt_identify = pfa_ackpkt_identify,
    .update_pack_max_len = pfa_update_pack_max_len,
    .update_usb_max_pkt_cnt = pfa_update_usb_max_pkt_cnt,
    .config_port_td_pri = pfa_config_port_td_pri,
    .enable_port = pfa_config_port_en,
    .td_usb_result_print = pfa_td_usb_result_print,
    .td_result_print = pfa_td_result_print,
    .clean_ad_buf = pfa_clean_ad_buf,
    .show_mac_entry = pfa_show_mac_entry,
    .construct_skb = pfa_construct_skb,
    .config_enable = pfa_enable,
    .config_disable = pfa_disable,
    .config_port_td_rd_info = pfa_config_port_td_rd_info,
    .enc_type_init = pfa_enc_type_init,
    .config_port_enc_type = pfa_config_port_enc_type,
    .config_sc = pfa_config_sc,
    .get_version = pfa_get_version,
    .get_tdq_rptr = get_tdq_rptr,
    .get_tdq_wptr = get_tdq_wptr,
    .get_rdq_rptr = get_rdq_rptr,
    .get_rdq_wptr = get_rdq_wptr,
    .set_tdq_wptr = set_tdq_wptr,
    .set_rdq_rptr = set_rdq_rptr,
    .set_rdq_wptr = set_rdq_wptr,
    .get_pdcp_cnt = pfa_get_pdcp_cnt,
};

