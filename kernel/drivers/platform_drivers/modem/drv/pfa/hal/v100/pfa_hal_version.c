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

#include "pfa.h"
#include "pfa_dbg.h"
#include "pfa_tft_reg.h"
#include "pfa_hal_reg.h"
#include "pfa_tft.h"
#include "pfa_desc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PFA_CLK_GATE_VALUE 0x13

struct pfa_om_reg_set g_pfa_om_reg[] = {
    {
        .start = 0x0,
        .end = 0x61c
    },
    {
        .start = 0x700,
        .end = 0x71c
    },
    {
        .start = 0x800,
        .end = 0x9fc
    },
    {
        .start = 0xa00,
        .end = 0xafc
    },
    {
        .start = 0xc00,
        .end = 0xcbc
    },
    {
        .start = 0xc000,
        .end = 0xc12c
    },
};

unsigned int g_pfa_reset_bak_reg[PFA_RESET_BAK_REG_NUM][PFA_STASH_REG_BUTTON] = {
    { PFA_RDQ_FULL_LEVEL, 0 },
    { PFA_ADQ_EMPTY_LEVEL, 0 },
    { PFA_ACK_IDEN_PORT, 0 },
    { PFA_ACK_IDEN_EN, 0 },
    { PFA_PACK_CTRL, 0 },
};

void config_tft_svlan_tag_sel(unsigned int flag)
{
    unsigned int reg;

    UPDATE1(reg, PFA_TFT_CTRL, svlan_tag_sel, flag);
}

void config_tft_bd_rd_ip_head(struct pfa *pfa)
{
    return;
}

void tft_config_rd_ip_head(unsigned int ext_desc_en)
{
    return;
}

void config_tft_bd_rd_ptr_update(struct pfa *pfa, struct iport_pfa_tft_addr *pfa_tft_reg)
{
    return;
}

void queue_portx_save(struct pfa *pfa_ctx, unsigned int portno, struct pfa_queue_stash *queue_regs)
{
    queue_regs->pfa_portx_udp_lmtnum[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_UDP_LMTNUM(portno));
    queue_regs->pfa_portx_lmtbyte[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_LMTBYTE(portno));
    queue_regs->pfa_portx_property[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_PROPERTY(portno));
    queue_regs->pfa_portx_mac_addr_l[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_MAC_ADDR_L(portno));
    queue_regs->pfa_protx_mac_addr_h[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_MAC_ADDR_H(portno));
    queue_regs->pfa_tdqx_pri[portno] = pfa_readl(pfa_ctx->regs, PFA_TDQX_PRI(portno));
}

void queue_portx_restore(struct pfa *pfa_ctx, unsigned int portno, struct pfa_queue_stash *queue_regs)
{
    pfa_writel(pfa_ctx->regs, PFA_PORTX_UDP_LMTNUM(portno), queue_regs->pfa_portx_udp_lmtnum[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_LMTBYTE(portno), queue_regs->pfa_portx_lmtbyte[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_PROPERTY(portno), queue_regs->pfa_portx_property[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_MAC_ADDR_L(portno), queue_regs->pfa_portx_mac_addr_l[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_MAC_ADDR_H(portno), queue_regs->pfa_protx_mac_addr_h[portno]);
    pfa_writel(pfa_ctx->regs, PFA_TDQX_PRI(portno), queue_regs->pfa_tdqx_pri[portno]);
}

void config_hash_cache_en(struct pfa *pfa, unsigned int enable)
{
    return;
}

void config_adq_threshold_and_len(struct pfa *pfa, unsigned int size)
{
    return;
}

void config_qos_lmttime(struct pfa *pfa, unsigned int value)
{
    return;
}

void config_eth_vlan_tag_sel(struct pfa *pfa, unsigned int flag)
{
    eth_vlan_tag_sel value;

    value.u32 = pfa_readl(pfa->regs, PFA_TOP_TAG_SEL_REG);
    value.bits.one_vlan_tag_sel = flag;
    pfa_writel(pfa->regs, PFA_TOP_TAG_SEL_REG, value.u32);
    if (flag) {
        pfa_writel(pfa->regs, PFA_TOP_SVLAN_TPID0_REG, 0x88a8);
        pfa_writel(pfa->regs, PFA_TOP_SVLAN_TPID1_REG, 0x9100);
        pfa_writel(pfa->regs, PFA_TOP_SVLAN_TPID2_REG, 0x9200);
        pfa_writel(pfa->regs, PFA_TOP_SVLAN_TPID3_REG, 0x8300);
    } else {
        pfa_writel(pfa->regs, PFA_TOP_SVLAN_TPID0_REG, 0x8100);
    }
}

void config_dport_adq_core_type(struct pfa *pfa, unsigned int portno, unsigned int type)
{
    pfa_port_prop_t prop;

    prop.u32 = pfa_readl(pfa->regs, PFA_PORTX_PROPERTY(portno));
    prop.bits.pfa_dport_adq_core_type = type;
    pfa_writel(pfa->regs, PFA_PORTX_PROPERTY(portno), prop.u32);
}

void pfa_check_td_result(struct pfa *pfa, unsigned int portno, struct pfa_td_desc *td_cur, struct pfa_port_stat *stat)
{
}

void pfa_print_one_rd_word_five_to_end(struct pfa_rd_desc *cur_rd)
{
    PFA_ERR("\n RD word 5 \n");
    PFA_ERR("rd_sport %x \n", cur_rd->rd_sport);
    PFA_ERR("rd_dport %x \n", cur_rd->rd_dport);
    PFA_ERR("rd_pktnum %x \n", cur_rd->rd_pktnum);
    PFA_ERR("rd_ethtype %x \n", cur_rd->rd_ethtype);

    PFA_ERR("\n RD word 6 \n");
    PFA_ERR("rd_pfa_tftres_stmid %x \n", cur_rd->rd_pfa_tftres_stmid);

    PFA_ERR("\n RD word 7 \n");
    PFA_ERR("rd_reserve5 %u \n", cur_rd->rd_reserve5);
    PFA_ERR("rd_vlan_flag %u \n", cur_rd->rd_vlan_flag);
    PFA_ERR("rd_vlan_pcp %u \n", cur_rd->rd_vlan_pcp);
    PFA_ERR("rd_vlan_vid %u \n", cur_rd->rd_vlan_vid);
    PFA_ERR("rd_qos_priority %u \n", cur_rd->rd_qos_priority);
    PFA_ERR("rd_qos_discard_time %u \n", cur_rd->rd_qos_discard_time);

    PFA_ERR("\n RD word 8 \n");
    PFA_ERR("tft_pdcp_cnt %x \n", cur_rd->tft_pdcp_cnt);

    PFA_ERR("\n RD word 9-11 \n");
    PFA_ERR("rd_user_field0 %x \n", cur_rd->rd_user_field0);
    PFA_ERR("rd_user_field1 %x \n", cur_rd->rd_user_field1);
    PFA_ERR("rd_user_field2 %x \n", cur_rd->rd_user_field2);
}

void pfa_print_one_rd(struct pfa_rd_desc *cur_rd)
{
    PFA_ERR("rd_int_en %u \n", cur_rd->rd_int_en);
    PFA_ERR("rd_mode %u \n", cur_rd->rd_mode);
    PFA_ERR("rd_cd_en %u \n", cur_rd->rd_cd_en);
    PFA_ERR("rd_data_from_unsec %u \n", cur_rd->rd_data_from_unsec);
    PFA_ERR("rd_ptp_syn_ind %u \n", cur_rd->rd_ptp_syn_ind);
    PFA_ERR("rd_pfa_tft_dl_err_flag %u \n", cur_rd->rd_pfa_tft_dl_err_flag);
    PFA_ERR("rd_ack_flag %u \n", cur_rd->rd_ack_flag);
    PFA_ERR("rd_fc_head %u \n", cur_rd->rd_fc_head);
    PFA_ERR("rd_packet_ind %u \n", cur_rd->rd_packet_ind);
    PFA_ERR("rd_v2x_ind %u \n", cur_rd->rd_v2x_ind);
    PFA_ERR("rd_tft_mac_err %u \n", cur_rd->rd_tft_mac_err);
    PFA_ERR("rd_high_pri_flag %u \n", cur_rd->rd_high_pri_flag);
    PFA_ERR("rd_pkt_len %u \n", cur_rd->rd_pkt_len);

    PFA_ERR("\n RD word 1~2 \n");
    PFA_ERR("rd_out_lower %x \n", cur_rd->rd_outaddr_lower);
    PFA_ERR("rd_out_upper %x \n", cur_rd->rd_outaddr_upper);

    PFA_ERR("\n RD word 3 \n");
    PFA_ERR("rd_pdu_ssid %u \n", cur_rd->rd_pdu_ssid);
    PFA_ERR("rd_net_id %u \n", cur_rd->rd_net_id);
    PFA_ERR("rd_modem_id %u \n", cur_rd->rd_modem_id);
    PFA_ERR("rd_offset_bd %u \n", cur_rd->rd_offset_bd);
    PFA_ERR("rd_offset_bd_drct %u \n", cur_rd->rd_offset_bd_drct);
    PFA_ERR("rd_l2_hdr_offset %u \n", cur_rd->rd_l2_hdr_offset);
    PFA_ERR("rd_iptype %u \n", cur_rd->rd_pkt_type);

    PFA_ERR("\n RD word 4 \n");
    PFA_ERR("rd_trans_result %u \n", cur_rd->rd_trans_result);
    PFA_ERR("rd_drop_rsn %u \n", cur_rd->rd_drop_rsn);
    PFA_ERR("rd_trans_path %u \n", cur_rd->rd_trans_path);
    PFA_ERR("rd_trans_path_finish %u \n", cur_rd->rd_trans_path_finish);
    PFA_ERR("rd_pkt_err %u \n", cur_rd->rd_pkt_err);
    PFA_ERR("rd_finish_warp_res %u \n", cur_rd->rd_finish_warp_res);
    PFA_ERR("rd_tocpu_res %u \n", cur_rd->rd_tocpu_res);

    pfa_print_one_rd_word_five_to_end(cur_rd);
}

void pfa_print_one_td(struct pfa_td_desc *cur_td)
{
    PFA_ERR("TD word 0 \n");
    PFA_ERR("td_int_en %u \n", cur_td->td_int_en);
    PFA_ERR("td_mode %u \n", cur_td->td_mode);
    PFA_ERR("td_cd_en %u \n", cur_td->td_cd_en);
    PFA_ERR("td_copy_en %u \n", cur_td->td_copy_en);
    PFA_ERR("td_fc_head %u \n", cur_td->td_fc_head);
    PFA_ERR("td_push_en %u \n", cur_td->td_push_en);
    PFA_ERR("td_high_pri_flag %u \n", cur_td->td_high_pri_flag);
    PFA_ERR("td_pkt_len %u \n", cur_td->td_pkt_len);

    PFA_ERR("TD word 1~2 \n");
    PFA_ERR("td_in_lower %x \n", cur_td->td_inaddr_lower);
    PFA_ERR("td_in_upper %x \n", cur_td->td_inaddr_upper);

    PFA_ERR("TD word 3 \n");
    PFA_ERR("td_pdu_ssid %u \n", cur_td->td_pdu_ssid);
    PFA_ERR("td_pdu_ssid %u \n", cur_td->v2x_ind);
    PFA_ERR("td_modem_id %u \n", cur_td->td_modem_id);
    PFA_ERR("td_stick_en %u \n", cur_td->td_stick_en);
    PFA_ERR("td_pkt_ind %u \n", cur_td->pkt_ind);
    PFA_ERR("td_iptype %u \n", cur_td->td_iptype);

    PFA_ERR("TD word 4 \n");
    PFA_ERR("td_bypass_en %u \n", cur_td->td_bypass_en);
    PFA_ERR("td_bypass_addr %u \n", cur_td->td_bypass_addr);
    PFA_ERR("td_host_ana %u \n", cur_td->td_host_ana);
    PFA_ERR("td_drop_ack_ind %u \n", cur_td->td_drop_ack_ind);
    PFA_ERR("td_net_id %u \n", cur_td->td_net_id);

    PFA_ERR("TD word 5-9 \n");
    PFA_ERR("td_result %x \n", cur_td->td_result);

    PFA_ERR("td_info %x \n", cur_td->td_info);

    PFA_ERR("td_user_field0 %x \n", cur_td->td_user_field0);
    PFA_ERR("td_user_field1 %x \n", cur_td->td_user_field1);
    PFA_ERR("td_user_field2 %x \n", cur_td->td_user_field2);
}
void set_update_only_produce_mod(struct pfa *pfa)
{
}

void enable_macfw_to_ip(struct pfa *pfa, unsigned int portno, unsigned int enable)
{
    pfa_port_prop_t property;

    property.u32 = pfa_readl_relaxed(pfa->regs, PFA_PORTX_PROPERTY(portno));
    if (enable) {
        property.bits.mac_ip_fw_flag = 1;
    } else {
        property.bits.mac_ip_fw_flag = 0;
    }
    pfa_writel_relaxed(pfa->regs, PFA_PORTX_PROPERTY(portno), property.u32);
}

void pfa_clk_gate_switch(struct pfa *pfa, unsigned int enable)
{
    unsigned int clken_sel = 0;
    unsigned int clk_gate = 0;

    if (enable) {
        clken_sel = 0x3f;
        clk_gate = PFA_CLK_GATE_VALUE;
    }
    pfa_writel(pfa->regs, CLKEN_SEL, clken_sel);
    pfa_writel(pfa->regs, PFA_CLK_GATE, clk_gate);
}

void pfa_rd_record_result(struct pfa_rd_desc *cur_desc, struct pfa_port_stat *stat)
{
    struct desc_result_s *result = &stat->result;

    result->rd_result[cur_desc->rd_trans_result]++;             // rd desc bit0-1
    result->rd_pkt_drop_rsn[cur_desc->rd_drop_rsn]++;           // rd desc bit2-5
    result->rd_pkt_fw_path[__fls(cur_desc->rd_trans_path)]++;   // rd desc bit6-15
    result->rd_finsh_wrap_rsn[cur_desc->rd_finish_warp_res]++;  // rd desc bit20-22
    result->rd_send_cpu_rsn[cur_desc->rd_tocpu_res]++;          // rd desc bit23-26

    stat->result.rd_sport_cnt[cur_desc->rd_sport]++;
    stat->rd_finished_bytes += cur_desc->rd_pkt_len;

    return;
}

void pfa_config_td_tft_mode(struct wan_info_s *wan_info, struct pfa_td_desc *desc)
{
    if (wan_info->info.td_mode_en) {
        desc->td_mode = wan_info->info.td_mode;
    }
    desc->td_user_field2 = wan_info->userfield2;
}

void dbgen_en(void)
{
    struct pfa *pfa = &g_pfa;
    pfa_dbgen_t pfa_dbg;

    pfa_dbg.u32 = pfa_readl(pfa->regs, PFA_DBGEN);
    pfa_dbg.bits.pfa_dbgen = 1;
    pfa_writel(pfa->regs, PFA_DBGEN, pfa_dbg.u32);
}

void dbgen_dis(void)
{
    struct pfa *pfa = &g_pfa;
    pfa_dbgen_t pfa_dbg;

    pfa_dbg.u32 = pfa_readl(pfa->regs, PFA_DBGEN);
    pfa_dbg.bits.pfa_dbgen = 0;
    pfa_writel(pfa->regs, PFA_DBGEN, pfa_dbg.u32);
}

int pfa_check_idle(struct pfa *pfa)
{
    unsigned int pfa_idle;
    unsigned int pfa_rd_status;

    pfa_idle = pfa_readl_relaxed(pfa->regs, PFA_IDLE);
    pfa_rd_status = pfa_readl(pfa->regs, PFA_RDQ_STATUS);
    if ((pfa_idle != PFA_IDLE_DEF_VAL) || (pfa_rd_status != PFA_RDQ_STATUS_EMPYT)) {
        PFA_ERR("pfa busy; pfa_idle=%x pfa_rd_status=%x  \n", pfa_idle, pfa_rd_status);
        return -1;
    }
    return 0;
}

void pfa_ackpkt_identify(struct pfa *pfa, unsigned int portno)
{
    pfa_writel_relaxed(pfa->regs, PFA_ACK_IDEN_PORT, portno);

    return;
}

void pfa_update_usb_max_pkt_cnt(struct pfa *pfa, unsigned int pack_max_pkt_cnt)
{
    pfa_pack_ctrl_t usb_pack_ctrl;

    usb_pack_ctrl.u32 = pfa_readl(pfa->regs, PFA_PACK_CTRL);
    usb_pack_ctrl.bits.pfa_pack_max_pkt_cnt = pack_max_pkt_cnt;
    pfa_writel_relaxed(pfa->regs, PFA_PACK_CTRL, usb_pack_ctrl.u32);
}

void pfa_update_pack_max_len(struct pfa *pfa, unsigned int pack_max_len)
{
    pfa_pack_ctrl_t usb_pack_ctrl;

    usb_pack_ctrl.u32 = pfa_readl(pfa->regs, PFA_PACK_CTRL);
    usb_pack_ctrl.bits.pfa_pack_max_len = pack_max_len;

    pfa_writel_relaxed(pfa->regs, PFA_PACK_CTRL, usb_pack_ctrl.u32);
}

void pfa_config_port_td_pri(struct pfa *pfa, unsigned int portno, unsigned int pri)
{
    pfa_writel_relaxed(pfa->regs, PFA_TDQX_PRI(portno), pri);
}

void pfa_config_port_en(struct pfa *pfa, unsigned int portno, unsigned int en)
{
    pfa_port_prop_t property;

    property.u32 = pfa_readl(pfa->regs, PFA_PORTX_PROPERTY(portno));
    property.bits.pfa_port_en = en;
    pfa->ports[portno].ctrl.property.u32 = property.u32;
    pfa_writel(pfa->regs, PFA_PORTX_PROPERTY(portno), property.u32);
}

void pfa_td_usb_result_print(struct pfa_port_ctrl *ctrl, struct pfa_port_stat *stat)
{
    // td result bit 22-25
#ifdef CONFIG_RNDIS
    if (ctrl->enc_type == PFA_ENC_RNDIS) {
        PFA_ERR("[td_rndis_wrap]:wrap_success %u \n", stat->result.td_unwrap[TD_RNDIS_WRAP_SUCCESS]);
        PFA_ERR("[td_rndis_wrap]:port_disable %u \n", stat->result.td_unwrap[TD_RNDIS_PORT_DISABLE]);
        PFA_ERR("[td_rndis_wrap]:td_length_too_long, %u \n", stat->result.td_unwrap[TD_RNDIS_TD_LENGTH_TOO_LONG]);
        PFA_ERR("[td_rndis_wrap]:first_msg_type %u \n", stat->result.td_unwrap[TD_RNDIS_FIRST_MSG_TYPE]);
        PFA_ERR("[td_rndis_wrap]:msg_length_oversize %u \n", stat->result.td_unwrap[TD_RNDIS_MSG_LENGTH_OVERSIZE]);
        PFA_ERR("[td_rndis_wrap]:eth_over_pkt_len %u \n", stat->result.td_unwrap[TD_RNDIS_ETH_OVER_PKT_LEN]);
        PFA_ERR("[td_rndis_wrap]:eth_length_wrong %u \n", stat->result.td_unwrap[TD_RNDIS_ETH_LENGTH_WRONG]);
    }
#endif

#ifdef CONFIG_NCM_UDC
    if ((ctrl->enc_type == PFA_ENC_NCM_NTB16) ||
        (ctrl->enc_type == PFA_ENC_NCM_NTB32)) {
        PFA_ERR("[td_ncm_wrap]:wrap_success %u \n", stat->result.td_unwrap[TD_NCM_WRAP_SUCCESS]);
        PFA_ERR("[td_ncm_wrap]:port_disable %u \n", stat->result.td_unwrap[TD_NCM_PORT_DISABLE]);
        PFA_ERR("[td_ncm_wrap]:td_length_too_long %u \n", stat->result.td_unwrap[TD_NCM_TD_LENGTH_TOO_LONG]);
        PFA_ERR("[td_ncm_wrap]:nth_length_wrong %u \n", stat->result.td_unwrap[TD_NCM_NTH_LENGTH_WRONG]);
        PFA_ERR("[td_ncm_wrap]:block_length_wrong %u \n", stat->result.td_unwrap[TD_NCM_BLOCK_LENGTH_WRONG]);
        PFA_ERR("[td_ncm_wrap]:ndp_flag_wrong %u \n", stat->result.td_unwrap[TD_NCM_NDP_FLAG_WRONG]);
        PFA_ERR("[td_ncm_wrap]:ndp_length_wrong %u \n", stat->result.td_unwrap[TD_NCM_NDP_LENGTH_WRONG]);
        PFA_ERR("[td_ncm_wrap]:datagram_wrong %u \n", stat->result.td_unwrap[TD_NCM_DATAGRAM_WRONG]);
        PFA_ERR("[td_ncm_wrap]:eth_length_wrong %u \n", stat->result.td_unwrap[TD_NCM_ETH_LENGTH_WRONG]);
        PFA_ERR("[td_ncm_wrap]:total_data_len_too_long %u \n", stat->result.td_unwrap[TD_NCM_TOTAL_DATA_LEN_TOO_LONG]);
        PFA_ERR("[td_ncm_wrap]:eth_pkt_to_lage %u \n", stat->result.td_unwrap[TD_NCM_ETH_PKT_TO_LAGE]);
    }
#endif

    if (!(ctrl->enc_type == PFA_ENC_NCM_NTB16 ||
          ctrl->enc_type == PFA_ENC_NCM_NTB32 ||
          ctrl->enc_type == PFA_ENC_RNDIS)) {
        PFA_ERR("[td_no_wrap]:finish_success %u \n", stat->result.td_unwrap[TD_FINISH_SUCCESS]);
        PFA_ERR("[td_no_wrap]:port_disable %u \n", stat->result.td_unwrap[TD_PORT_DISABLE]);
        PFA_ERR("[td_no_wrap]:pkt_len_abnormity %u \n", stat->result.td_unwrap[TD_PKT_LEN_ABNORMITY]);
    }
}

void pfa_td_result_print(struct pfa_port_ctrl *ctrl, struct pfa_port_stat *stat)
{
}

void pfa_clean_ad_buf(struct pfa *pfa)
{
    pfa_writel(pfa->regs, PFA_ADQ0_BUF_FLUSH, 0x1);
}

void pfa_show_mac_entry(struct pfa *pfa, unsigned int flag)
{
    unsigned int pfa_tab[PFA_MAC_TAB_WORD_NO];
    unsigned int i, j;
    struct pfa_mac_entry_in_sram *mac_entry = NULL;

    for (i = 0; i < PFA_MAC_TAB_NUMBER; i++) {
        for (j = 0; j < PFA_MAC_TAB_WORD_NO; j++) {
            pfa_tab[j] = pfa_readl(pfa->regs, PFA_MAC_TAB(i * PFA_MAC_TAB_WORD_NO + j));
        }
        if (pfa_tab[3] & flag) { // 3 is mac entry valid flag
            mac_entry = (struct pfa_mac_entry_in_sram *)pfa_tab;

            PFA_ERR("mac entry (%d) : %pM    bid:%u    vid:%u    port:%u  timestamp:0x%x  static:%u\n", i,
                    mac_entry->mac, ((unsigned)mac_entry->bid) & 0xf, ((unsigned)mac_entry->vid) & 0xfff,
                    ((unsigned)mac_entry->port) & 0x1f, ((unsigned)mac_entry->timestamp) & 0xffff,
                    ((unsigned)mac_entry->is_static) & 0x1);
        }
    }
}

int pfa_construct_skb(struct pfa_port_ctx *port_ctx, struct pfa_rd_desc *desc, struct sk_buff *skb,
    unsigned long long org_dra_addr, unsigned long long dra_l2_addr, unsigned int dra_addr_type)
{
    unsigned long long org_phy_addr, pfa_phy_addr, machdr_to_dra_offset;

    switch (dra_addr_type) {
        case (DRA_SKB_FROM_TCP_IP):
            org_phy_addr = virt_to_phys(skb->data);
            pfa_phy_addr = bsp_dra_to_phy(dra_l2_addr);
            skb->len = 0;
            skb_reset_tail_pointer(skb);
            break;
        case (DRA_SKB_FROM_OWN):
            org_phy_addr = bsp_dra_to_phy(org_dra_addr);
            pfa_phy_addr = bsp_dra_to_phy(dra_l2_addr);
            bsp_dra_reset_skb(skb);
            break;
        default:
            PFA_ERR_ONCE("sport %u dra 0x%llx skb %lx is neither dra_owned nor dra_maped\n", desc->rd_sport,
                         dra_l2_addr, (uintptr_t)skb);
            return -1;
    }

    if (unlikely(skb->data_len)) {
        port_ctx->stat.skb_err++;
        PFA_ERR_ONCE("%u time skb err, sport %u org_dra %llx l2_dra %llx hdr_offset %u skb %lx \n",
            port_ctx->stat.skb_err, desc->rd_sport, org_dra_addr, dra_l2_addr, desc->rd_l2_hdr_offset, (uintptr_t)skb);
        return -1;
    }

    if (org_phy_addr >= pfa_phy_addr) {
        // mac hdr move to lower addr than dra hdr, main sence is lower addr for pfa_tft to pcie
        machdr_to_dra_offset = org_phy_addr - pfa_phy_addr;
        if (desc->rd_sport == PFA_DEFAULT_PFA_TFT_PORT && port_ctx->ctrl.portno == PFA_DEFAULT_CPU_PORT) {
            skb_put(skb, desc->rd_pkt_len - machdr_to_dra_offset);
            return 0;
        }
        if (machdr_to_dra_offset <= PFA_MACHDR_TO_DRA_OFFSET_MAX) {
            skb_push(skb, machdr_to_dra_offset);  // add pkt to start of skb header
            skb_put(skb, desc->rd_pkt_len - machdr_to_dra_offset);
        } else {
            PFA_ERR_ONCE("sport %u org dra 0x%llx  pfa dra 0x%llx \n", desc->rd_sport, org_dra_addr, dra_l2_addr);
            return -1;
        }
    } else {  // mac hdr move to higher addr than dra hdr
        machdr_to_dra_offset = pfa_phy_addr - org_phy_addr;
        if (machdr_to_dra_offset <= PFA_MACHDR_TO_DRA_OFFSET_MAX) {
            skb_reserve(skb, machdr_to_dra_offset);
            skb_put(skb, desc->rd_pkt_len);
        } else {
            PFA_ERR_ONCE("sport %u org dra 0x%llx  pfa dra 0x%llx \n", desc->rd_sport, org_dra_addr, dra_l2_addr);
            return -1;
        }
    }
    return 0;
}

void pfa_enable(void)
{
    pfa_en_t value;
    struct pfa *pfa = &g_pfa;
    PFA_TRACE("enter\n");

    value.u32 = pfa_readl(pfa->regs, PFA_EN);
    value.bits.pfa_en = 1;
    pfa_writel(pfa->regs, PFA_EN, value.u32);
    PFA_TRACE("exit\n");

    pfa->flags |= PFA_FLAG_ENABLE;

    return;
}

void pfa_disable(void)
{
    struct pfa *pfa = &g_pfa;
    pfa_en_t value;

    /* disable pfa hw */
    value.u32 = pfa_readl(pfa->regs, PFA_EN);
    value.bits.pfa_en = 0;
    pfa_writel(pfa->regs, PFA_EN, value.u32);

    return;
}

void pfa_config_port_td_rd_info(unsigned int portno, unsigned int td_depth, unsigned int rd_depth)
{
    struct pfa *pfa = &g_pfa;

    pfa_writel_relaxed(pfa->regs, PFA_TDQX_LEN(portno), td_depth);
    pfa_writel_relaxed(pfa->regs, PFA_RDQX_LEN(portno), rd_depth);
}

void pfa_enc_type_init(struct pfa *pfa)
{
}
void pfa_config_port_enc_type(struct pfa *pfa, unsigned int portno, int enc_type)
{
}

int pfa_get_version(void)
{
    return PFA_VERSION_V100;
}
void pfa_config_sc(struct pfa *pfa, unsigned int idx, unsigned int value)
{
}

int pfa_get_pdcp_cnt(struct pfa_rd_desc *desc)
{
    return desc->tft_pdcp_cnt;
}

EXPORT_SYMBOL(pfa_construct_skb);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
