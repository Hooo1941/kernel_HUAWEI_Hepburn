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

#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/igmp.h>
#include <bsp_slice.h>

#include <bsp_pfa.h>
#ifdef CONFIG_POWER_DUBAI
#include <chipset_common/dubai/dubai.h>
#endif

#include "pfa.h"
#include "pfa_dbg.h"
#include "pfa_desc.h"
#include "pfa_hal.h"
#include "pfa_ip_entry.h"
#include "pfa_port.h"

#define PORTNO_INVALID(portno) ((portno) >= PFA_PORT_NUM)

void pfa_help(void)
{
    PFA_ERR("01. pfa_dev_setting \n");
    PFA_ERR("02. pfa_dev_info \n");
    PFA_ERR("03. pfa_all_port_show \n");
    PFA_ERR("04. pfa_port_setting(port_num) \n");
    PFA_ERR("05. pfa_cpu_port_info \n");
    PFA_ERR("06. pfa_port_info_all(port_num) \n");
    PFA_ERR("07. pfa_port_td_info(port_num) \n");
    PFA_ERR("08. pfa_port_rd_info(port_num) \n");
    PFA_ERR("09. pfa_td_info_adv(port_num td_pos) \n");
    PFA_ERR("10. pfa_rd_info_adv(port_num rd_pos) \n");
    PFA_ERR("11. pfa_print_td(port_num td_pos) :print info of one td \n");
    PFA_ERR("12. pfa_print_rd(port_num rd_pos) :print info of one rd \n");
    PFA_ERR("13. pfa_ipfw_setting \n");
    PFA_ERR("14. pfa_ip_fw_show \n");
    PFA_ERR("15. pfa_add_err_show \n");
    PFA_ERR("16. pfa_port_speed \n");
    PFA_ERR("17. pfa_macfw_entry_dump \n");
    PFA_ERR("18. pfa_direct_fw_status \n");
    PFA_ERR("19. pfa_dfs_get \n");
    PFA_ERR("20. pfa_intr_set_interval(32.768khz) \n");
    PFA_ERR("21. pfa_set_cpuport_maxrate(mbps) \n");
    PFA_ERR("22. pfa_transfer_pause \n");
    PFA_ERR("23. pfa_transfer_restart \n");
    PFA_ERR("24. pfa_rd_drop_en \n");
    PFA_ERR("25. pfa_set_result_record:0:def 1:td 2:rd 3:rdsave 4:bugon \n");
    PFA_ERR("26. pfa_set_msg:0:def 1:trace 2:info 3:dbg 4:tuple \n");
    PFA_ERR("27. pfa_set_hids_ul:0:def 1:trace 2:info 3:dbg 4:tuple \n");
}

void pfa_get_port_type(unsigned int portno, enum pfa_enc_type enc_type)
{
    char *port_type_char = NULL;
    switch (enc_type) {
        case (PFA_ENC_NONE):
            port_type_char = "eth";
            break;
        case (PFA_ENC_NCM_NTB16):
            port_type_char = "ntb16";
            break;
        case (PFA_ENC_NCM_NTB32):
            port_type_char = "ntb32";
            break;
        case (PFA_ENC_RNDIS):
            port_type_char = "rndis";
            break;
        case (PFA_ENC_WIFI):
            port_type_char = "wifi";
            break;
        case (PFA_ENC_PFA_TFT):
            port_type_char = "pfa_tft";
            break;
        case (PFA_ENC_CPU):
            port_type_char = "cpu";
            break;
        case (PFA_ENC_ACK):
            port_type_char = "ack";
            break;
        default:
            port_type_char = "error";
    }
    if (g_pfa.ports[portno].ctrl.net != NULL) {
        PFA_ERR("pfa port %u type:%s, device name: %s \n", portno, port_type_char, g_pfa.ports[portno].ctrl.net->name);
    } else {
        PFA_ERR("pfa port %u type:%s device name: null \n", portno, port_type_char);
    }
}

void pfa_set_clk_div_dbg(unsigned int dbg_en, unsigned int rate)
{
    g_pfa.clk_pfa_dbg = dbg_en;
    g_pfa.clk_pfa_dbg_rate = rate;
}

void pfa_get_port_type_dbg(struct pfa_port_ctrl *ctrl)
{
    if (ctrl->net != NULL) {
        PFA_ERR("device name     %s \n", ctrl->net->name);
    }
    pfa_get_port_type(ctrl->portno, ctrl->enc_type);
}

void pfa_all_port_show(void)
{
    unsigned int i;
    unsigned int portno;
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctrl *ctrl = NULL;

    portno = find_first_zero_bit(&pfa->portmap[PFA_PORTS_NOMARL], PFA_PORT_NUM);

    for (i = 0; i < portno; i++) {
        ctrl = &pfa->ports[i].ctrl;
        if (ctrl->port_flags.enable) {
            pfa_get_port_type(ctrl->portno, ctrl->enc_type);
        }
    }
}

void pfa_cpu_port_info(void)
{
    struct pfa *pfa = &g_pfa;

    PFA_ERR("cpu_pkt_max_rate %u mbps \n", pfa->cpuport.cpu_pkt_max_rate);
    PFA_ERR("cpu_pktnum_per_interval %u \n", pfa->cpuport.cpu_pktnum_per_interval);

    PFA_ERR("cpu_updonly %u \n", pfa->cpuport.cpu_updonly);
    PFA_ERR("cpu_updonly_comp %u \n", pfa->cpuport.cpu_updonly_comp);
    PFA_ERR("cpu_rd_num %u \n", pfa->cpuport.cpu_rd_num);
    PFA_ERR("cpu_rd_udp_drop %u \n", pfa->cpuport.cpu_rd_udp_drop);
    PFA_ERR("cpu_rd_dfw_updonly_drop %u \n", pfa->cpuport.cpu_rd_dfw_updonly_drop);

    PFA_ERR("cpu_rd_to_wan %u \n", pfa->cpuport.cpu_rd_to_wan);
    PFA_ERR("cpu_rd_to_wan_fail %u \n", pfa->cpuport.cpu_rd_to_wan_fail);
    PFA_ERR("cpu_rd_to_nic %u \n", pfa->cpuport.cpu_rd_to_nic);
    PFA_ERR("cpu_rd_to_nic_fail %u \n", pfa->cpuport.cpu_rd_to_nic_fail);
    PFA_ERR("cpu_rd_to_netif_rx %u \n", pfa->cpuport.cpu_rd_to_netif_rx);
    PFA_ERR("cpu_rd_to_netif_rx_succ %u \n", pfa->cpuport.cpu_rd_to_netif_rx_succ);
    PFA_ERR("cpu_rd_to_netif_rx_fail %u \n", pfa->cpuport.cpu_rd_to_netif_rx_fail);

    PFA_ERR("checksum_succ %u \n", pfa->cpuport.checksum_succ);
    PFA_ERR("checksum_fail %u \n", pfa->cpuport.checksum_fail);
}

void pfa_print_td(unsigned int vir_port_num, unsigned int td_pos)
{
    struct pfa_port_ctrl *ctrl = NULL;
    struct pfa_td_desc *cur_td = NULL;
    struct pfa_td_desc *td_base = NULL;
    unsigned int port_num = PFA_GET_PHY_PORT(vir_port_num);

    if (PORTNO_INVALID(port_num)) {
        PFA_ERR("port num %u is invalid!\n", port_num);
        return;
    }
    ctrl = &g_pfa.ports[port_num].ctrl;
    td_base = (struct pfa_td_desc *)ctrl->td_addr;

    if (td_pos >= ctrl->td_depth) {
        PFA_ERR("td_pos %u is invalid!\n", td_pos);
        return;
    }
    cur_td = &td_base[td_pos];
    pfa_print_one_td(cur_td);
    return;
}

void pfa_print_rd(unsigned int vir_port_num, unsigned int rd_pos)
{
    struct pfa_port_ctrl *ctrl = NULL;
    struct pfa_rd_desc *cur_rd = NULL;
    struct pfa_rd_desc *rd_base = NULL;
    unsigned int port_num = PFA_GET_PHY_PORT(vir_port_num);

    if (PORTNO_INVALID(port_num)) {
        PFA_ERR("port num %u is invalid!\n", port_num);
        return;
    }
    ctrl = &g_pfa.ports[port_num].ctrl;
    rd_base = (struct pfa_rd_desc *)ctrl->rd_addr;

    if (rd_pos >= ctrl->rd_depth) {
        PFA_ERR("rd_pos %u is invalid!\n", rd_pos);
        return;
    }
    cur_rd = &rd_base[rd_pos];
    pfa_print_one_rd(cur_rd);
    return;
}

void pfa_dev_setting(void)
{
    struct pfa *pfa = &g_pfa;

    PFA_ERR("msg_level                   :%u\n", pfa->msg_level);
    PFA_ERR("dbg_level                   :%u\n", pfa->dbg_level);
    PFA_ERR("phy portmap                 :0x%lx\n", pfa->portmap[PFA_PORTS_NOMARL]);
    PFA_ERR("br portmap                  :0x%lx\n", pfa->portmap[PFA_PORTS_BR]);
    PFA_ERR("ipfw_timeout                :%u\n", pfa->ipfw.ipfw_timeout);
    PFA_ERR("macfw_timeout               :%u\n", pfa->macfw.macfw_timeout);
    PFA_ERR("flags                       :%u\n", pfa->flags);
    PFA_ERR("mask_flags                       :%u\n", pfa->mask_flags);
    PFA_ERR("drop_stub                       :%u\n", pfa->drop_stub);
    PFA_ERR("macfw_dev_add               :%u\n", pfa->macfw.macfw_dev_add);
    PFA_ERR("macfw_br_add               :%u\n", pfa->macfw.macfw_br_add);
    PFA_ERR("macfw_br_del               :%u\n", pfa->macfw.macfw_br_del);
    PFA_ERR("macfw_fdb_dev_add               :%u\n", pfa->macfw.macfw_fdb_dev_add);
    PFA_ERR("macfw_fdb_dev_del               :%u\n", pfa->macfw.macfw_fdb_dev_del);

    // add each port info
}

int pfa_set_hids_ul(unsigned int enable, unsigned int portno)
{
    struct pfa *pfa = &g_pfa;
    int i;

    if (portno == PFA_PORT_NUM) {
        for (i = 0; i < PFA_PORT_NUM; i++) {
            pfa->ports[i].ctrl.port_flags.hids_upload = !!enable;
        }
    } else if ((portno >= 0) && (portno < PFA_PORT_NUM)) {
        pfa->ports[portno].ctrl.port_flags.hids_upload = !!enable;
    } else {
        PFA_ERR("input err\n");
    }
    return 0;
}

int pfa_set_result_record(unsigned int level)
{
    struct pfa *pfa = &g_pfa;
    switch (level) {
        case PFA_DBG_LEVEL0:
            pfa->dbg_level = PFA_DBG_WARN_ON;
            break;
        case PFA_DBG_LEVEL1:
            pfa->dbg_level |= PFA_DBG_TD_RESULT;
            break;
        case PFA_DBG_LEVEL2:
            pfa->dbg_level |= PFA_DBG_RD_RESULT;
            break;
        case PFA_DBG_LEVEL3:
            pfa->dbg_level |= PFA_DBG_RD_SAVE;
            break;
        case PFA_DBG_LEVEL4:
            pfa->dbg_level |= PFA_DBG_BUG_ON;
            break;
        default:
            pfa->dbg_level |= PFA_DBG_TD_RESULT;
    }

    PFA_ERR("dbg_level                :%x\n", pfa->dbg_level);

    return 0;
}

int pfa_set_msgdbg(unsigned int msg_level)
{
    struct pfa *pfa = &g_pfa;
    pfa->msg_level = msg_level;

    return 0;
}

void pfa_dev_info(void)
{
    struct pfa *pfa = &g_pfa;
    int i;

    PFA_ERR("ipfw_add                :%u\n", pfa->stat.ipfw_add);
    PFA_ERR("ipfw_add_enter          :%u\n", pfa->stat.ipfw_add_enter);
    PFA_ERR("ipfw_add_leave          :%u\n", pfa->stat.ipfw_add_leave);
    PFA_ERR("ipfw_del                :%u\n", pfa->stat.ipfw_del);
    PFA_ERR("ipfw_del_enter          :%u\n", pfa->stat.ipfw_del_enter);
    PFA_ERR("ipfw_del_leave          :%u\n", pfa->stat.ipfw_del_leave);
    PFA_ERR("ipfw_del_nothing_leave          :%u\n", pfa->stat.ipfw_del_nothing_leave);
    PFA_ERR("ip_fw_not_add          :%u\n", pfa->ipfw.ip_fw_not_add);
    PFA_ERR("ipfw_filter          :%u\n", pfa->stat.ipfw_filter);

    PFA_ERR("disable_timeout         :%u\n", pfa->stat.disable_timeout);
    PFA_ERR("wait_idle                   :%u\n", pfa->stat.wait_idle);

    PFA_ERR("pfa->suspend_count         :%u\n", pfa->suspend_count);
    PFA_ERR("pfa->resume_count                   :%u\n", pfa->resume_count);

    PFA_ERR("modem_reset_count         :%u\n", pfa->modem_reset_count);
    PFA_ERR("modem_unreset_count         :%u\n", pfa->modem_unreset_count);
    PFA_ERR("modem_noreset_count                   :%u\n", pfa->modem_noreset_count);

    for (i = 0; i < PFA_ADQ_BOTTOM; i++) {
        PFA_ERR("evt_ad_empty[%u]               :%u\n", i, pfa->stat.evt_ad_empty[i]);
    }

    PFA_ERR("evt_td_errport              :%u\n", pfa->stat.evt_td_errport);

    for (i = 0; i < PFA_PORT_NUM; i++) {
        if (test_bit(i, &pfa->portmap[PFA_PORTS_NOMARL])) {
            PFA_ERR("port[%u]:               :%u\n", i, 1);

            PFA_ERR("evt_td_complt[%u]               :%u\n", i, pfa->stat.evt_td_complt[i]);
            PFA_ERR("evt_td_full[%u]               :%u\n", i, pfa->stat.evt_td_full[i]);
            PFA_ERR("evt_rd_complt[%u]               :%u\n", i, pfa->stat.evt_rd_complt[i]);
            PFA_ERR("evt_rd_empty[%u]               :%u\n", i, pfa->stat.evt_rd_empty[i]);
            PFA_ERR("evt_rd_full[%u]               :%u\n", i, pfa->stat.evt_rd_full[i]);
        }
    }
}

void pfa_print_mac_addr(const unsigned char *mac)
{
    int i;

    if (mac == NULL || strlen(mac) < ETH_ALEN) {
        return;
    }

    for (i = 0; i < ETH_ALEN; i++) {
        if (i < (ETH_ALEN >> 1)) {
            PFA_ERR(" *");
        } else {
            PFA_ERR("  %2x  ", mac[i]);
        }
    }
    PFA_ERR(" \n");
}


void pfa_port_td_info(unsigned int vir_portno)
{
    unsigned int td_wptr;
    unsigned int td_rptr;
    struct pfa *pfa = &g_pfa;
    struct pfa_port_stat *stat = NULL;
    struct pfa_port_ctrl *ctrl = NULL;
    unsigned int portno = PFA_GET_PHY_PORT(vir_portno);

    if (PORTNO_INVALID(portno)) {
        PFA_ERR("port num is invalid!\n");
        return;
    }

    stat = &pfa->ports[portno].stat;
    ctrl = &pfa->ports[portno].ctrl;

    pfa_get_port_type_dbg(ctrl);
    PFA_ERR("td_busy 0x%x \n", ctrl->td_busy);
    PFA_ERR("td_free 0x%x \n", ctrl->td_free);

    td_wptr = pfa_readl(pfa->regs, PFA_TDQX_WPTR(portno));
    td_rptr = pfa_readl(pfa->regs, PFA_TDQX_RPTR(portno));

    PFA_ERR("td_hard_ptr wptr 0x%x \n", td_wptr);
    PFA_ERR("td_hard_ptr rptr  0x%x \n", td_rptr);

    PFA_ERR("td config %u \n", stat->td_config);

    PFA_ERR("td config(port disabled) %u \n", stat->td_port_disabled);
    PFA_ERR("td full times %u \n", stat->td_full);
    PFA_ERR("td finish event %u \n", stat->td_finsh_intr_complete);
    PFA_ERR("td desc complete %u \n", stat->td_desc_complete);
    PFA_ERR("td pkt complete %u \n", stat->td_pkt_complete);
    PFA_ERR("td desc fail drop %u \n", stat->td_desc_fail_drop);

    PFA_ERR("td push %u \n", stat->td_kick);
    PFA_ERR("td_dma_null %u \n", stat->td_dma_null);
}

void pfa_td_info_adv(unsigned int vir_portno)
{
    struct pfa_port_stat *stat = NULL;
    struct pfa_port_ctrl *ctrl = NULL;
    unsigned int portno = PFA_GET_PHY_PORT(vir_portno);

    if (PORTNO_INVALID(portno)) {
        PFA_ERR("port num is invalid!\n");
        return;
    }

    stat = &g_pfa.ports[portno].stat;
    ctrl = &g_pfa.ports[portno].ctrl;

    pfa_port_td_info(portno);

    PFA_ERR("\n");

    g_pfa.hal->td_result_print(ctrl, stat);
    g_pfa.hal->td_usb_result_print(ctrl, stat);

    return;
}

void pfa_port_setting(unsigned int vir_portno)
{
    unsigned int i;
    struct pfa_port_ctrl *ctrl = NULL;
    pfa_port_prop_t ch_propert;
    unsigned int portno = PFA_GET_PHY_PORT(vir_portno);

    if (PORTNO_INVALID(portno)) {
        PFA_ERR("port num is invalid!\n");
        return;
    }

    ctrl = &g_pfa.ports[portno].ctrl;
    ch_propert.u32 = ctrl->property.u32;
    for (i = 0; i < PFA_PORT_NUM; i++) {
        if (ctrl->net_map[i].net != NULL) {
            PFA_ERR("net id:%u, net device name    %s \n", i, ctrl->net_map[i].net->name);
        }
    }

    pfa_get_port_type_dbg(ctrl);

    PFA_ERR("is_enable       0x%x \n", ctrl->port_flags.enable);
    PFA_ERR("port_pad_en   0x%x \n", ch_propert.bits.pfa_port_pad_en);
    PFA_ERR("port_attach     0x%x \n", ch_propert.bits.pfa_port_attach_brg);
    PFA_ERR("enc_type        0x%x \n", ctrl->enc_type);

    PFA_ERR("port mac addr :");
    if (ctrl->net != NULL) {
        pfa_print_mac_addr(ctrl->net->dev_addr);
    } else {
        PFA_ERR("no dev\n");
    }

    PFA_ERR("rd_addr         0x%lx \n", (uintptr_t)ctrl->rd_addr);
    PFA_ERR("rd_depth        %u \n", ctrl->rd_depth);
    PFA_ERR("rd_evt_gap      0x%x \n", ctrl->rd_evt_gap);
    PFA_ERR("rd_dma          %lx \n", (uintptr_t)ctrl->rd_dma);

    PFA_ERR("td_addr         0x%lx \n", (uintptr_t)ctrl->td_addr);
    PFA_ERR("td_depth        %u \n", ctrl->td_depth);
    PFA_ERR("td_evt_gap      0x%x \n", ctrl->td_evt_gap);
    PFA_ERR("td_dma          %lx \n", (uintptr_t)ctrl->td_dma);

    PFA_ERR("udp_limit_time  0x%x \n", ctrl->udp_limit_time);
    PFA_ERR("udp_limit_cnt   0x%x \n", ctrl->udp_limit_cnt);
    PFA_ERR("rate_limit_time 0x%x \n", ctrl->rate_limit_time);
    PFA_ERR("rate_limit_byte 0x%x \n", ctrl->rate_limit_byte);
}

void pfa_rd_result_print_bit0_19(struct pfa_port_stat *stat)
{
    // bit0-1
    PFA_ERR("[rd_result]:rd_result_updata_only %u \n", stat->result.rd_result[RD_RESULT_UPDATA_ONLY]);
    PFA_ERR("[rd_result]:rd_result_discard %u \n", stat->result.rd_result[RD_RESULT_DISCARD]);
    PFA_ERR("[rd_result]:rd_result_success %u \n", stat->result.rd_result[RD_RESULT_SUCCESS]);
    PFA_ERR("[rd_result]:rd_result_wrap_or_length_wrong %u \n", stat->result.rd_result[RD_RESULT_WRAP_OR_LENGTH_WRONG]);

    // bit2-5
    PFA_ERR("[rd_pkt_drop_rsn]:rd_pkt_drop_rsn_undiscard %u \n", stat->result.rd_pkt_drop_rsn[RD_DROP_RSN_UNDISCARD]);
    PFA_ERR("[rd_pkt_drop_rsn]:rd_pkt_drop_rsn_rd_point_null %u \n",
            stat->result.rd_pkt_drop_rsn[RD_DROP_RSN_RD_POINT_NULL]);

    // rd bit 6-16
    PFA_ERR("[rd_pkt_fw_path]:rd_mac_filter %u \n", stat->result.rd_pkt_fw_path[RD_MAC_FILTER]);
    PFA_ERR("[rd_pkt_fw_path]:rd_eth_type %u \n", stat->result.rd_pkt_fw_path[RD_ETH_TYPE]);
    PFA_ERR("[rd_pkt_fw_path]:rd_1st_mac_fw %u \n", stat->result.rd_pkt_fw_path[RD_1ST_MAC_FW]);
    PFA_ERR("[rd_pkt_fw_path]:rd_ip_hdr_check %u \n", stat->result.rd_pkt_fw_path[RD_IP_HDR_CHECK]);
    PFA_ERR("[rd_pkt_fw_path]:rd_ip_filter %u \n", stat->result.rd_pkt_fw_path[RD_IP_FILTER]);
    PFA_ERR("[rd_pkt_fw_path]:rd_tcp_udp_check %u \n", stat->result.rd_pkt_fw_path[RD_TCP_UDP_CHECK]);
    PFA_ERR("[rd_pkt_fw_path]:rd_hash_check %u \n", stat->result.rd_pkt_fw_path[RD_HASH_CHECK]);
    PFA_ERR("[rd_pkt_fw_path]:rd_hash_cache_check %u \n", stat->result.rd_pkt_fw_path[RD_HASH_CACHE_CHECK]);
    PFA_ERR("[rd_pkt_fw_path]:rd_hash_ddr_check %u \n", stat->result.rd_pkt_fw_path[RD_HASH_DDR_CHECK]);
    PFA_ERR("[rd_pkt_fw_path]:rd_2nd_mac_check %u \n", stat->result.rd_pkt_fw_path[RD_2ND_MAC_CHECK]);
    PFA_ERR("[rd_pkt_fw_path]:rd_trans_complete %u \n", stat->result.rd_pkt_fw_path[RD_TRANS_COMPLETE]);

    // rd bit 17-19
    PFA_ERR("[rd_pkt_type]:rd_pkt_ipv4_tcp %u \n", stat->result.rd_pkt_type[RD_PKT_IPV4_TCP]);
    PFA_ERR("[rd_pkt_type]:rd_pkt_ipv4_udp %u \n", stat->result.rd_pkt_type[RD_PKT_IPV4_UDP]);
    PFA_ERR("[rd_pkt_type]:rd_pkt_ipv4_non_udp_tcp %u \n", stat->result.rd_pkt_type[RD_PKT_IPV4_NON_UDP_TCP]);
    PFA_ERR("[rd_pkt_type]:rd_pkt_ipv6_tcp %u \n", stat->result.rd_pkt_type[RD_PKT_IPV6_TCP]);
    PFA_ERR("[rd_pkt_type]:rd_pkt_ipv6_udp %u \n", stat->result.rd_pkt_type[RD_PKT_IPV6_UDP]);
    PFA_ERR("[rd_pkt_type]:rd_pkt_ipv6_non_udp_tcp %u \n", stat->result.rd_pkt_type[RD_PKT_IPV6_NON_UDP_TCP]);
    PFA_ERR("[rd_pkt_type]:rd_pkt_non_udp_tcp %u \n", stat->result.rd_pkt_type[RD_PKT_NON_UDP_TCP]);
    PFA_ERR("[rd_pkt_type]:rd_pkt_trans_before_ip_check %u \n", stat->result.rd_pkt_type[RD_PKT_TRANS_BEFORE_IP_CHECK]);
}

void pfa_rd_result_print_bit20_26(struct pfa_port_stat *stat)
{
    // bit20-22
    PFA_ERR("[pfa_wraped_rsn]:rd_wrap_wrong_format %u \n", stat->result.rd_finsh_wrap_rsn[RD_WRAP_WRONG_FORMAT]);
    PFA_ERR("[pfa_wraped_rsn]:rd_wrap_push_en %u \n", stat->result.rd_finsh_wrap_rsn[RD_WRAP_PUSH_EN]);
    PFA_ERR("[pfa_wraped_rsn]:rd_wrap_num_oversize %u \n", stat->result.rd_finsh_wrap_rsn[RD_WRAP_NUM_OVERSIZE]);
    PFA_ERR("[pfa_wraped_rsn]:rd_wrap_length_oversize %u \n", stat->result.rd_finsh_wrap_rsn[RD_WRAP_LENGTH_OVERSIZE]);
    PFA_ERR("[pfa_wraped_rsn]:rd_wrap_timeout %u \n", stat->result.rd_finsh_wrap_rsn[RD_WRAP_TIMEOUT]);
    PFA_ERR("[pfa_wraped_rsn]:rd_wrap_port_dis %u \n", stat->result.rd_finsh_wrap_rsn[RD_WRAP_PORT_DIS]);
    PFA_ERR("[pfa_wraped_rsn]:rd_wrap_mutli_ncm %u \n", stat->result.rd_finsh_wrap_rsn[RD_WRAP_MUTLI_NCM]);

    // rd bit 23-26
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_pkt_err %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_PKT_ERR]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_mac_non_stick_v4_v6 %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_MAC_NON_STICK_V4_V6]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_1stmac_ipver_err %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_1STMAC_IPVER_ERR]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_1stmac_dmac_smac_diff %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_1STMAC_DMAC_SMAC_DIFF]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_1stmac_mac_trans_fail %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_1STMAC_MAC_TRANS_FAIL]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_2sndmac_brg_dmac_fail %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_2SNDMAC_BRG_DMAC_FAIL]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_l3_check_l3_protocol_fail %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_L3_CHECK_L3_PROTOCOL_FAIL]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_l3_check_ipv4_hdr_fail %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_L3_CHECK_IPV4_HDR_FAIL]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_ipv6_hop_limit_1 %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_IPV6_HOP_LIMIT_1]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_ipv4_hdr_len_err %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_IPV4_HDR_LEN_ERR]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_ipv4_one_llt_left %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_IPV4_ONE_TTL_LEFT]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_ipv4_slice_pkt %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_IPV4_SLICE_PKT]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_l4_port_match %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_L4_PORT_MATCH]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_ip_table_mismatch %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_IP_TABLE_MISMATCH]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_stick_arp_pkt %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_STICK_ARP_PKT]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_fwsucc_or_discard %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_FWSUCC_OR_DISCARD]);
    PFA_ERR("[pfa_to_cpu_rsn]:rd_cpu_ecm2usb_bycpu %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_ECM2USB_BYCPU]);
    PFA_ERR("[pfa_to_cpu_rsn]:RD_CPU_IP_TYPE_CHECK_DOUBLE_VLAN %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_IP_TYPE_CHECK_DOUBLE_VLAN]);
    PFA_ERR("[pfa_to_cpu_rsn]:RD_CPU_IP_TYPE_CHECK_NOT_DOUBLE_VLAN_NOT_IP %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_IP_TYPE_CHECK_NOT_DOUBLE_VLAN_NOT_IP]);
    PFA_ERR("[pfa_to_cpu_rsn]:RD_CPU_MAC_PDU_FAIL %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_MAC_PDU_FAIL]);
    PFA_ERR("[pfa_to_cpu_rsn]:RD_CPU_MAC_PDU_TBALE_NOT_DONE %u \n",
            stat->result.rd_send_cpu_rsn[RD_CPU_MAC_PDU_TBALE_NOT_DONE]);
    PFA_ERR("[pfa_to_cpu_rsn]:RD_CPU_USB_TO_USB %u \n", stat->result.rd_send_cpu_rsn[RD_CPU_USB_TO_USB]);

    PFA_ERR("\n");
}

void pfa_port_rd_info(unsigned int vir_portno)
{
    unsigned int rd_hard_wptr;
    unsigned int rd_hard_rptr;
    struct pfa_port_stat *stat = NULL;
    struct pfa_port_ctrl *ctrl = NULL;
    unsigned int i;
    unsigned int portno = PFA_GET_PHY_PORT(vir_portno);

    if (PORTNO_INVALID(portno)) {
        PFA_ERR("port num is invalid!\n");
        return;
    }

    stat = &g_pfa.ports[portno].stat;
    ctrl = &g_pfa.ports[portno].ctrl;

    pfa_get_port_type_dbg(ctrl);

    rd_hard_wptr = pfa_readl(g_pfa.regs, PFA_RDQX_WPTR(portno));
    rd_hard_rptr = pfa_readl(g_pfa.regs, PFA_RDQX_RPTR(portno));

    PFA_ERR("rd_busy         0x%x \n", ctrl->rd_busy);
    PFA_ERR("rd_free         0x%x \n", ctrl->rd_free);

    PFA_ERR("rd_hard_ptr wptr 0x%x \n", rd_hard_wptr);
    PFA_ERR("rd_hard_ptr rptr 0x%x \n\n", rd_hard_rptr);

    PFA_ERR("rd configed         %u \n", stat->rd_config);
    PFA_ERR("rd finish           %u \n", stat->rd_finished);
    PFA_ERR("rd finish event     %u \n", stat->rd_finsh_intr_complete);
    PFA_ERR("rd finish event cb  %u \n", stat->rd_finsh_intr_complete_called);
    PFA_ERR("rd finish pkt num     %u \n", stat->rd_finsh_pkt_num);

    PFA_ERR("rd drop             %u \n", stat->rd_droped);
    PFA_ERR("rd rd_sended             %u \n", stat->rd_sended);
    PFA_ERR("rd rd_dra_zero             %u \n", stat->rd_dra_zero);
    PFA_ERR("rd dra_to_skb_fail          %u \n", stat->dra_to_skb_fail);
    PFA_ERR("rd skb_err             %u \n", stat->skb_err);
    PFA_ERR("rd filter drop             %u \n", stat->rd_filter);

    PFA_ERR("rd result success   %u \n\n", stat->result.rd_result[RD_RESULT_SUCCESS]);
    for (i = 0; i < PFA_PORT_NUM; i++) {
        if (ctrl->net_map[i].net != NULL) {
            PFA_ERR("net id:%u, pkt cnt             %u \n", i, ctrl->net_map[i].pkt_cnt);
        }
    }

}

void pfa_rd_info_adv(unsigned int vir_portno, unsigned int password)
{
    unsigned int i;
    struct pfa_port_stat *stat = NULL;
    struct pfa_port_ctrl *ctrl = NULL;
    unsigned int portno = PFA_GET_PHY_PORT(vir_portno);

    if (PORTNO_INVALID(portno)) {
        PFA_ERR("port num is invalid!\n");
        return;
    }

    stat = &g_pfa.ports[portno].stat;
    ctrl = &g_pfa.ports[portno].ctrl;

    pfa_get_port_type_dbg(ctrl);

    pfa_port_rd_info(portno);
    pfa_rd_result_print_bit0_19(stat);
    pfa_rd_result_print_bit20_26(stat);

    for (i = 0; i < PFA_PORT_NUM; i++) {
        PFA_ERR("rd from port %u count: %u \n", i, stat->result.rd_sport_cnt[i]);
    }
}

void pfa_ipfw_setting(void)
{
    struct pfa_ip_fw_ctx *ipfw_ctx = &g_pfa.ipfw;
    struct pfa *pfa = &g_pfa;

    unsigned int reg_hash_base_l;
    unsigned int reg_hash_base_h;
    unsigned int reg_size;
    unsigned int reg_depth;
    unsigned int reg_zone;
    unsigned int reg_rand;

    reg_hash_base_l = pfa_readl(pfa->regs, PFA_HASH_BADDR_L);
    reg_hash_base_h = pfa_readl(pfa->regs, PFA_HASH_BADDR_H);
    reg_size = pfa_readl(pfa->regs, PFA_HASH_WIDTH);
    reg_depth = pfa_readl(pfa->regs, PFA_HASH_DEPTH);
    reg_zone = pfa_readl(pfa->regs, PFA_HASH_ZONE);
    reg_rand = pfa_readl(pfa->regs, PFA_HASH_RAND);

    PFA_ERR("free_cnt %u \n", ipfw_ctx->free_cnt);
    PFA_ERR("free_threhold %u \n", ipfw_ctx->free_threhold);
    PFA_ERR("deadtime %u \n", ipfw_ctx->deadtime);
    PFA_ERR("hash list base %lx \n", (uintptr_t)ipfw_ctx->hbucket);
    PFA_ERR("hash list base dma_h sw:%u, hw:%u \n", upper_32_bits(ipfw_ctx->hbucket_dma), reg_hash_base_h);
    PFA_ERR("hash list base dma_l sw:%u, hw:%u\n", lower_32_bits(ipfw_ctx->hbucket_dma), reg_hash_base_l);
    PFA_ERR("hash list size sw:%u, hw:%u \n", ipfw_ctx->hlist_size, reg_depth);
    PFA_ERR("hash list width sw:%u, hw:%u \n", ipfw_ctx->hitem_width, reg_size);
    PFA_ERR("hash list zone sw:%u, hw:%u \n", ipfw_ctx->hzone, reg_zone);
    PFA_ERR("hash list rand sw:%u, hw:%u \n", ipfw_ctx->hrand, reg_rand);
}

void pfa_port_info_all(unsigned int vir_port_num)
{
    unsigned int port_num = PFA_GET_PHY_PORT(vir_port_num);

    if (PORTNO_INVALID(port_num)) {
        PFA_ERR("port num is invalid!\n");
        return;
    }
    pfa_port_setting(port_num);
    pfa_port_rd_info(port_num);
    pfa_port_td_info(port_num);
}

void pfa_ad_info(void)
{
    struct pfa_adq_ctrl *cur_ctrl = NULL;
    struct pfa *pfa = &g_pfa;
    enum pfa_adq_num adq_num = 0;
    unsigned int dra_stub_wptr = 0;
    unsigned int pfa_ad_wptr;
    unsigned int pfa_ad_rptr;

    cur_ctrl = &pfa->adqs_ctx.ctrl;

    if (pfa->soft_push) {  // evide dra hard error
        dra_stub_wptr = readl(cur_ctrl->dra_wptr_stub_addr);
    }
    pfa_ad_wptr = pfa_readl(pfa->regs, PFA_ADQ0_WPTR);
    pfa_ad_rptr = pfa_readl(pfa->regs, PFA_ADQ0_RPTR);

    PFA_ERR("AD[%u] cur_ctrl->dra_wptr_stub_addr %u \n", adq_num, (u32)(uintptr_t)cur_ctrl->dra_wptr_stub_addr);
    PFA_ERR("AD[%u] dra_stub_rptr %u \n", adq_num, dra_stub_wptr);
    PFA_ERR("AD[%u] pfa_ad_wptr %u \n", adq_num, pfa_ad_wptr);
    PFA_ERR("AD[%u] pfa_ad_rptr %u \n", adq_num, pfa_ad_rptr);
}

static void pfa_pr_pkg_pm(const unsigned char *buf, int len)
{
    int j, count;

    count = len > 64 ? 64 : len; /* get pkt hrd 64 Byte */
    for (j = 0; j < count; j += 16) { /* 16 Byte/line */
        PFA_ERR("%03x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", j, buf[j],
                buf[j + 1], buf[j + 2], buf[j + 3], buf[j + 4], buf[j + 5], // 1,2,3,4,5 pkt shift
                buf[j + 6], buf[j + 7], buf[j + 8], buf[j + 9], buf[j + 0xa],  // 6,7,8,9 pkt shift
                buf[j + 0xb], buf[j + 0xc], buf[j + 0xd], buf[j + 0xe], buf[j + 0xf]);
    }
    PFA_ERR("\n");
}
void pfa_print_pkt_ipv4(unsigned char *data)
{
    struct udphdr *udph = NULL;
    struct tcphdr *tcph = NULL;
    struct icmphdr *icmph = NULL;
    struct iphdr *iph = (struct iphdr *)data;

    PFA_ERR("[PFA]src ip:%d.%d.%d.x, dst ip:%d.%d.%d.x\n", iph->saddr & 0xff, (iph->saddr >> 8) & 0xff, // 8 bit shift
            (iph->saddr >> 16) & 0xff, iph->daddr & 0xff, // 16 bit shift
            (iph->daddr >> 8) & 0xff, (iph->daddr >> 16) & 0xff); // 8,16 bit shift
    if (iph->protocol == IPPROTO_UDP) {
        udph = (struct udphdr *)(data + sizeof(struct iphdr));
        PFA_ERR("[PFA]UDP packet, src port:%d, dst port:%d.\n", ntohs(udph->source), ntohs(udph->dest));
#ifdef CONFIG_POWER_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_MODEM_PACKET_WAKEUP_UDP_V4", "port", ntohs(udph->dest));
#endif
    } else if (iph->protocol == IPPROTO_TCP) {
        tcph = (struct tcphdr *)(data + sizeof(struct iphdr));
        PFA_ERR("[PFA]TCP packet, src port:%d, dst port:%d\n", ntohs(tcph->source), ntohs(tcph->dest));
#ifdef CONFIG_POWER_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_MODEM_PACKET_WAKEUP_TCP_V4", "port", ntohs(tcph->dest));
#endif
    } else if (iph->protocol == IPPROTO_ICMP) {
        icmph = (struct icmphdr *)(data + sizeof(struct iphdr));
        PFA_ERR("[PFA]ICMP packet, type(%d):%s, code:%d.\n", icmph->type,
                ((icmph->type == 0) ? "ping reply" : ((icmph->type == 8) ? // 8 icmph type
                "ping request" : "other icmp pkt")), icmph->code);
#ifdef CONFIG_POWER_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_MODEM_PACKET_WAKEUP", "protocol", (int)iph->protocol);
#endif
    } else if (iph->protocol == IPPROTO_IGMP) {
        PFA_ERR("[PFA]ICMP packet\n");
#ifdef CONFIG_POWER_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_MODEM_PACKET_WAKEUP", "protocol", (int)iph->protocol);
#endif
    } else {
        PFA_ERR("[PFA]Other IPV4 packet\n");
#ifdef CONFIG_POWER_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_MODEM_PACKET_WAKEUP", "protocol", (int)iph->protocol);
#endif
    }
}
void pfa_print_pkt_ipv6(struct iphdr *iph)
{
    struct ipv6hdr *ip6h = NULL;

    ip6h = (struct ipv6hdr *)iph;
    PFA_ERR("[PFA]version: %d, payload length: %d, nh->nexthdr: %d. \n", ip6h->version, ntohs(ip6h->payload_len),
            ip6h->nexthdr);
    PFA_ERR("[PFA]ipv6 src addr:%04x:%x:%xx:x:x:x:x:x  \n", ntohs(ip6h->saddr.in6_u.u6_addr16[7]), // 7 is addr shift
            ntohs(ip6h->saddr.in6_u.u6_addr16[6]), (ip6h->saddr.in6_u.u6_addr8[11])); // 6,11 is addr shift
    PFA_ERR("[PFA]ipv6 dst addr:%04x:%x:%xx:x:x:x:x:x \n", ntohs(ip6h->saddr.in6_u.u6_addr16[7]), // 7 is addr shift
            ntohs(ip6h->saddr.in6_u.u6_addr16[6]), (ip6h->saddr.in6_u.u6_addr8[11])); // 6,11 is addr shift
#ifdef CONFIG_POWER_DUBAI
    dubai_log_packet_wakeup_stats("DUBAI_TAG_MODEM_PACKET_WAKEUP", "protocol", IPPROTO_IPV6);
#endif
}

void pfa_print_pkt_info(unsigned char *data)
{
    struct iphdr *iph = (struct iphdr *)data;

    PFA_ERR("[PFA] wakeup, ip version=%d\n", iph->version);

    if (iph->version == 4) { /* 4 is ipv4 */
        pfa_print_pkt_ipv4(data);
    } else if (iph->version == 6) { /* 6 is ipv6 */
        pfa_print_pkt_ipv6(iph);
    } else {
        pfa_pr_pkg_pm(data, 40);  /* 40 is pkt len */
    }
}

void pfa_hds_report_cpu_rd_info(struct pfa *pfa, struct pfa_om_info *info)
{
    info->cpu.updonly_comp = pfa->cpuport.cpu_updonly_comp;
    info->cpu.rd_num = pfa->cpuport.cpu_rd_num;
    info->cpu.rd_udp_drop = pfa->cpuport.cpu_rd_udp_drop;
    info->cpu.rd_wan = pfa->cpuport.cpu_rd_to_wan;
    info->cpu.rd_wan_fail = pfa->cpuport.cpu_rd_to_wan_fail;
    info->cpu.rd_nic = pfa->cpuport.cpu_rd_to_nic;
    info->cpu.rd_nic_fail = pfa->cpuport.cpu_rd_to_nic_fail;
    info->cpu.rd_netif = pfa->cpuport.cpu_rd_to_netif_rx;
    info->cpu.rd_netif_succ = pfa->cpuport.cpu_rd_to_netif_rx_succ;
    info->cpu.rd_netif_fail = pfa->cpuport.cpu_rd_to_netif_rx_fail;

    return;
}

void pfa_hds_report_port_rd_info(struct pfa *pfa, struct pfa_om_info *info)
{
    int portno;

    for (portno = 0; portno < PFA_PORT_NUM; portno++) {
        if (test_bit(portno, &pfa->portmap[PFA_PORTS_NOMARL])) {
            info->port[portno].rd_finished = pfa->ports[portno].stat.rd_finished;
            info->port[portno].rd_finished_bytes = pfa->ports[portno].stat.rd_finished_bytes;
            info->port[portno].rd_finsh_pkt = pfa->ports[portno].stat.rd_finsh_pkt_num;
            info->port[portno].rd_droped = pfa->ports[portno].stat.rd_droped;
            info->port[portno].rd_sended = pfa->ports[portno].stat.rd_sended;
            info->port[portno].rd_full = pfa->stat.evt_rd_full[portno];
            info->port[portno].rd_larger = 0;
            info->port[portno].td_config = pfa->ports[portno].stat.td_config;
            info->port[portno].td_full = pfa->ports[portno].stat.td_full;
            info->port[portno].td_complete = pfa->ports[portno].stat.td_desc_complete;
            info->port[portno].td_fail_drop = pfa->ports[portno].stat.td_desc_fail_drop;
            info->port[portno].td_comp = pfa->ports[portno].stat.td_pkt_complete;
        }
    }
    return;
}

void pfa_hds_report_tocpursn(struct pfa *pfa, struct pfa_om_info *info)
{
    unsigned int cpu_portno = PFA_DEFAULT_CPU_PORT;

    info->to_cpu_rsn.pkt_err = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_PKT_ERR];
    info->to_cpu_rsn.mac_non_stick_v4_v6 =
        pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_MAC_NON_STICK_V4_V6];
    info->to_cpu_rsn.first_mac_ipver_err = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_1STMAC_IPVER_ERR];
    info->to_cpu_rsn.first_mac_dmac_smac_diff =
        pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_1STMAC_DMAC_SMAC_DIFF];
    info->to_cpu_rsn.first_mac_mac_trans_fail =
        pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_1STMAC_MAC_TRANS_FAIL];
    info->to_cpu_rsn.snd_mac_brg_dmac_fail =
        pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_2SNDMAC_BRG_DMAC_FAIL];
    info->to_cpu_rsn.l3_check_l3_protocol_fail =
        pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_L3_CHECK_L3_PROTOCOL_FAIL];
    info->to_cpu_rsn.l3_check_ipv4_hdr_fail =
        pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_L3_CHECK_IPV4_HDR_FAIL];
    info->to_cpu_rsn.ipv6_hop_limit_1 = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_IPV6_HOP_LIMIT_1];
    info->to_cpu_rsn.ipv4_hdr_len_err = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_IPV4_HDR_LEN_ERR];
    info->to_cpu_rsn.ipv4_one_llt_left = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_IPV4_ONE_TTL_LEFT];
    info->to_cpu_rsn.ipv4_slice_pkt = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_IPV4_SLICE_PKT];
    info->to_cpu_rsn.l4_port_match = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_L4_PORT_MATCH];
    info->to_cpu_rsn.ip_table_mismatch = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_IP_TABLE_MISMATCH];
    info->to_cpu_rsn.fwsucc_or_discard = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_FWSUCC_OR_DISCARD];
    info->to_cpu_rsn.stick_arp_pkt = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_STICK_ARP_PKT];
    info->to_cpu_rsn.ecm2usb_bycpu = pfa->ports[cpu_portno].stat.result.rd_send_cpu_rsn[RD_CPU_ECM2USB_BYCPU];

    return;
}
void pfa_hds_report_task_time(struct pfa *pfa, struct pfa_om_info *info)
{
    info->task_time.int_time = pfa->stat.int_time;
    info->task_time.task_begin_time = pfa->stat.wq_task_begin_time;
    info->task_time.task_end_time = pfa->stat.wq_task_end_time;

    return;
}

void pfa_hds_transreport(void)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_om_info *info = &pfa->hds_info;

    pfa_hds_report_cpu_rd_info(pfa, info);
    pfa_hds_report_port_rd_info(pfa, info);
    pfa_hds_report_tocpursn(pfa, info);
    pfa_hds_report_task_time(pfa, info);

    return;
}

void pfa_trans_report_register(void)
{
    struct pfa *pfa = &g_pfa;
    struct net_om_info *info = &pfa->report_info;
    int ret;

    info->ul_msg_id = NET_OM_MSGID_PFA;
    info->data = (void *)&pfa->hds_info;
    info->size = sizeof(pfa->hds_info);
    info->func_cb = pfa_hds_transreport;
    INIT_LIST_HEAD(&info->list);

    ret = bsp_net_report_register(info);
    if (ret) {
        PFA_ERR("trans report register fail\n");
    }
    return;
}

extern struct pfa_om_reg_set g_pfa_om_reg[PFA_REG_DUMP_GROUP_NUM];
int pfa_get_om_reg_size(void)
{
    unsigned int i;
    unsigned int totalsize = 0;

    for (i = 0; i < sizeof(g_pfa_om_reg) / sizeof(g_pfa_om_reg[0]); i++) {
        totalsize += g_pfa_om_reg[i].end - g_pfa_om_reg[i].start + sizeof(unsigned int);
    }

    return totalsize;
}

void pfa_dump_hook(void)
{
    unsigned int *ptr = NULL;
    unsigned int i;
    unsigned int j;
    struct pfa *pfa = &g_pfa;
    unsigned int om_reg_size;

    om_reg_size = pfa_get_om_reg_size();
    if (om_reg_size > PFA_DUMP_SIZE) {
        PFA_ERR("pfa om dump hook size err, om_reg_size 0x%x.\n", om_reg_size);
        return;
    }

    if (pfa->dump_base != NULL) {
        ptr = (unsigned int *)pfa->dump_base;
        for (i = 0; i < sizeof(g_pfa_om_reg) / sizeof(g_pfa_om_reg[0]); i++) {
            for (j = g_pfa_om_reg[i].start; j <= g_pfa_om_reg[i].end;) {
                *ptr++ = pfa_readl(pfa->regs, j);
                j += sizeof(unsigned int);
            }
        }
    } else {
        PFA_ERR("pfa dump base err.\n");
        return;
    }
}

void pfa_om_dump_init(struct pfa *pfa)
{
    int ret;

    pfa_trans_report_register();
    pfa->dump_base = (unsigned int *)bsp_dump_register_field(DUMP_MODEMAP_PFA, "PFA", PFA_DUMP_SIZE, 0);
    if (pfa->dump_base == NULL) {
        PFA_ERR("pfa om dump mem alloc fail\n");
        return;
    }
    ret = (int)memset_s(pfa->dump_base, PFA_DUMP_SIZE, 0, PFA_DUMP_SIZE);
    if (ret) {
        PFA_ERR("pfa om dump memset_s fail\n");
        return;
    }

    ret = bsp_dump_register_hook("pfa", pfa_dump_hook);
    if (ret == BSP_ERROR) {
        PFA_ERR("pfa om dump register fail\n");
        return;
    }

    return;
}

int pfa_rd_drop_en(int drop_stub_en)
{
    struct pfa *pfa = &g_pfa;

    pfa->drop_stub = drop_stub_en;

    return 0;
}

void pfa_port_speed(void)
{
    struct pfa *pfa = &g_pfa;
    static unsigned int port_td_cnt[PFA_PORT_NUM] = {0};
    static unsigned int port_rd_cnt[PFA_PORT_NUM] = {0};
    static unsigned int pfa_last_timestamp = 0;
    unsigned int cur_time;
    unsigned int portno;
    unsigned int td_inc_cnt;
    unsigned int rd_inc_cnt;
    unsigned int time_slice;

    cur_time = bsp_get_slice_value();

    for (portno = 0; portno < PFA_PORT_NUM; portno++) {
        if (test_bit(portno, &pfa->portmap[PFA_PORTS_NOMARL])) {
            if (pfa->ports[portno].stat.td_pkt_complete >= port_td_cnt[portno]) {
                td_inc_cnt = pfa->ports[portno].stat.td_pkt_complete - port_td_cnt[portno];
            } else {
                td_inc_cnt = 0xFFFFFFFF + pfa->ports[portno].stat.td_pkt_complete - port_td_cnt[portno];
            }
            if (pfa->ports[portno].stat.rd_finsh_pkt_num >= port_rd_cnt[portno]) {
                rd_inc_cnt = pfa->ports[portno].stat.rd_finsh_pkt_num - port_rd_cnt[portno];
            } else {
                rd_inc_cnt = 0xFFFFFFFF + pfa->ports[portno].stat.rd_finsh_pkt_num - port_rd_cnt[portno];
            }
            if (cur_time > pfa_last_timestamp) {
                time_slice = cur_time - pfa_last_timestamp;
            } else {
                time_slice = 0xFFFFFFFF + cur_time - pfa_last_timestamp;
            }
            PFA_ERR("port[%d] td speed during last sample:%lld Mb/s\n", portno,
                    (long long)td_inc_cnt * UDP_PKT_LEN * BIT_NUM_PER_BYTE * NUM_PER_SLICE / time_slice /
                        PFA_SPEED_BTOMB);

            PFA_ERR("port[%d] rd speed during last sample:%lld Mb/s\n", portno,
                    (long long)rd_inc_cnt * UDP_PKT_LEN * BIT_NUM_PER_BYTE * NUM_PER_SLICE / time_slice /
                        PFA_SPEED_BTOMB);

            port_td_cnt[portno] = pfa->ports[portno].stat.td_pkt_complete;
            port_rd_cnt[portno] = pfa->ports[portno].stat.rd_finsh_pkt_num;
        }
    }
    pfa_last_timestamp = cur_time;
    return;
}

void pfa_set_rd_loop_cnt(unsigned long long cnt)
{
    struct pfa *pfa = &g_pfa;
    if (cnt == 0) {
        PFA_ERR("para err\n");
        return;
    }
    pfa->rd_loop_cnt = cnt;
    return;
}

void pfa_get_rd_time(void)
{
    struct pfa *pfa = &g_pfa;

    PFA_ERR("int_time                :%u\n", pfa->stat.int_time);
    PFA_ERR("wq_task_begin_time          :%u\n", pfa->stat.wq_task_begin_time);
    PFA_ERR("wq_task_end_time          :%u\n", pfa->stat.wq_task_end_time);

    return;
}

int pfa_enable_dfs(int en_dfs)
{
    struct pfa *pfa = &g_pfa;

    pfa->pfa_dfs_enable = en_dfs;

    return 0;
}

void pfa_dfs_time_set(int time)
{
    struct pfa *pfa = &g_pfa;
    pfa->pfa_dfs_time_interval = time;
    PFA_ERR("time interval : %d \n", pfa->pfa_dfs_time_interval);
}

void pfa_dfs_time_get(void)
{
    struct pfa *pfa = &g_pfa;
    PFA_ERR("time interval : %d \n", pfa->pfa_dfs_time_interval);
}

void pfa_dfs_start_limit_set(int value_kbps)
{
    struct pfa *pfa = &g_pfa;
    pfa->start_byte_limit = value_kbps;
}

void pfa_dfs_start_limit_get(void)
{
    struct pfa *pfa = &g_pfa;
    PFA_ERR("start_byte_limit : %d (kbps)\n", pfa->start_byte_limit);
}

void pfa_dfs_grade_set(int x, int val1, int val2, int val3)
{
    struct pfa *pfa = &g_pfa;
    pfa->freq_grade_array[x].freq_min = val2;
    pfa->freq_grade_array[x].freq_max = val3;
}

void pfa_dfs_grade_get(void)
{
    struct pfa *pfa = &g_pfa;
    int i = 0;
    for (i = 0; i < DFS_MAX_LEVEL; i++) {
        PFA_ERR("%d  %d \n ", pfa->freq_grade_array[i].freq_min, pfa->freq_grade_array[i].freq_max);
    }
}

void pfa_port_speed_multiple(unsigned int portno, unsigned int enable, unsigned int multiple)
{
    if (portno >= PFA_PORT_NUM) {
        return;
    }
    g_pfa.ports[portno].ctrl.dfs_speed_multiple_en = enable;
    g_pfa.ports[portno].ctrl.dfs_speed_multiple = multiple;
    return;
}

void pfa_port_copy_en(unsigned int portno, unsigned int enable)
{
    g_pfa.ports[portno].ctrl.td_copy_en = enable;
    return;
}

void pfa_set_bypass(unsigned int port_num, unsigned int bypass_dir, unsigned int en)
{
    struct pfa *pfa = &g_pfa;
    if (port_num >= PFA_PORT_NUM) {
        PFA_ERR("wrong pfa port \n");
        return;
    }
    pfa->ports[port_num].ctrl.bypassport_en = en;
    pfa->ports[port_num].ctrl.bypassport = bypass_dir;
}

void pfa_set_task_delay_time(unsigned int value)
{
    struct pfa *pfa = &g_pfa;

    pfa->stat.task_delay_max = value;
}

EXPORT_SYMBOL(pfa_rd_drop_en);
EXPORT_SYMBOL(pfa_help);
EXPORT_SYMBOL(pfa_port_td_info);
EXPORT_SYMBOL(pfa_port_rd_info);
EXPORT_SYMBOL(pfa_print_rd);
EXPORT_SYMBOL(pfa_print_td);
EXPORT_SYMBOL(pfa_all_port_show);
EXPORT_SYMBOL(pfa_cpu_port_info);
EXPORT_SYMBOL(pfa_dev_setting);
EXPORT_SYMBOL(pfa_dev_info);
EXPORT_SYMBOL(pfa_td_info_adv);
EXPORT_SYMBOL(pfa_port_setting);
EXPORT_SYMBOL(pfa_rd_info_adv);
EXPORT_SYMBOL(pfa_ipfw_setting);
EXPORT_SYMBOL(pfa_port_info_all);
EXPORT_SYMBOL(pfa_ad_info);
EXPORT_SYMBOL(pfa_print_pkt_ipv4);
EXPORT_SYMBOL(pfa_print_pkt_ipv6);
EXPORT_SYMBOL(pfa_print_pkt_info);
EXPORT_SYMBOL(pfa_set_hids_ul);
EXPORT_SYMBOL(pfa_set_result_record);
EXPORT_SYMBOL(pfa_set_msgdbg);
EXPORT_SYMBOL(pfa_enable_dfs);
EXPORT_SYMBOL(pfa_dfs_time_set);
EXPORT_SYMBOL(pfa_dfs_time_get);
EXPORT_SYMBOL(pfa_dfs_start_limit_set);
EXPORT_SYMBOL(pfa_dfs_start_limit_get);
EXPORT_SYMBOL(pfa_dfs_grade_set);
EXPORT_SYMBOL(pfa_dfs_grade_get);
EXPORT_SYMBOL(pfa_hds_transreport);
EXPORT_SYMBOL(pfa_get_om_reg_size);
EXPORT_SYMBOL(pfa_dump_hook);
EXPORT_SYMBOL(pfa_port_speed);
EXPORT_SYMBOL(pfa_set_rd_loop_cnt);
EXPORT_SYMBOL(pfa_get_rd_time);
EXPORT_SYMBOL(pfa_port_copy_en);
EXPORT_SYMBOL(pfa_set_bypass);

MODULE_ALIAS("network hardware accelerator driver");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("smart packet engine(pfa) driver");
