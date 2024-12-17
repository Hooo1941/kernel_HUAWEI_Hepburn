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
#include "pfa_desc.h"
#include "pfa_tft_reg.h"
#include "pfa_hal_reg.h"
#include "pfa_tft.h"


struct pfa_om_reg_set g_pfa_om_reg[PFA_REG_DUMP_GROUP_NUM] = {
    {
        .start = 0,
        .end = 0x34
    },
    {
        .start = 0x40,
        .end = 0xBC
    },
    {
        .start = 0x100,
        .end = 0x114
    },
    {
        .start = 0x200,
        .end = 0x37C
    },
    {
        .start = 0x500,
        .end = 0x5C0
    },
    {
        .start = 0x600,
        .end = 0x620
    },
    {
        .start = 0x700,
        .end = 0x7A8
    },
    {
        .start = 0x2800,
        .end = 0x2C0C
    },
    {
        .start = 0x3000,
        .end = 0x307c
    },
};

unsigned int g_pfa_reset_bak_reg[PFA_RESET_BAK_REG_NUM][PFA_STASH_REG_BUTTON] = {
    { PFA_PFA_TFT_BD_RD_IP_HEAD, 0 },
    { PFA_PFA_TFT_RDQ_WPTR_UPDATE_ADDR_L, 0 },
    { PFA_PFA_TFT_RDQ_WPTR_UPDATE_ADDR_H, 0 },
    { PFA_TFT_PFA_BDQ_RPTR_UPDATE_ADDR_L, 0 },
    { PFA_TFT_PFA_BDQ_RPTR_UPDATE_ADDR_H, 0 },
    { PFA_HASH_CACHE_EN, 0 },
    { PFA_ADQ_THRESHOLD, 0 },
    { PFA_ADQ0_LEN, 0 },
    { PFA_ADQ1_LEN, 0 },
    { PFA_RDQ_FULL_LEVEL, 0 },
    { PFA_ADQ_EMPTY_LEVEL, 0 },
    { PFA_ACK_IDEN_PORT, 0 },
    { PFA_ACK_IDEN_EN, 0 },
    { PFA_PACK_CTRL, 0 },
};

void config_tft_bd_rd_ip_head(struct pfa *pfa)
{
    pfa_writel(pfa->regs, PFA_PFA_TFT_BD_RD_IP_HEAD, 1);
}

void tft_config_rd_ip_head(unsigned int ext_desc_en)
{
    unsigned int reg;

    if (ext_desc_en) {
        UPDATE2(reg, PFA_TFT_CTRL, dl_rdq_with_head, 1, ul_bdq_with_head, 1);
    } else {
        UPDATE2(reg, PFA_TFT_CTRL, dl_rdq_with_head, 0, ul_bdq_with_head, 0);
    }
}

void config_tft_bd_rd_ptr_update(struct pfa *pfa, struct iport_pfa_tft_addr *pfa_tft_reg)
{
    pfa_writel(pfa->regs, PFA_PFA_TFT_RDQ_WPTR_UPDATE_ADDR_L, lower_32_bits(pfa_tft_reg->ulrd_wptr_addr));
    pfa_writel(pfa->regs, PFA_PFA_TFT_RDQ_WPTR_UPDATE_ADDR_H, upper_32_bits(pfa_tft_reg->ulrd_wptr_addr));

    pfa->pfa_tftport.pfa_tft_rd_rptr_addr = (void *)(uintptr_t)(pfa_tft_reg->dltd_rptr_phy_addr);
    pfa_writel(pfa->regs, PFA_TFT_PFA_BDQ_RPTR_UPDATE_ADDR_L, (unsigned int)(uintptr_t)pfa->pfa_tftport.pfa_tft_rd_rptr_addr);
}

void queue_portx_save(struct pfa *pfa_ctx, unsigned int portno, struct pfa_queue_stash *queue_regs)
{
    queue_regs->pfa_portx_udp_lmtnum[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_UDP_LMTNUM(portno));
    queue_regs->pfa_portx_lmtbyte[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_LMTBYTE(portno));
    queue_regs->pfa_portx_property[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_PROPERTY(portno));
    queue_regs->pfa_portx_mac_addr_l[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_MAC_ADDR_L(portno));
    queue_regs->pfa_protx_mac_addr_h[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_MAC_ADDR_H(portno));
    queue_regs->pfa_portx_ipv6_addr_1s[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_IPV6_ADDR_1ST(portno));
    queue_regs->pfa_portx_ipv6_addr_2n[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_IPV6_ADDR_2ND(portno));
    queue_regs->pfa_portx_ipv6_addr_3t[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_IPV6_ADDR_3TH(portno));
    queue_regs->pfa_portx_ipv6_addr_4t[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_IPV6_ADDR_4TH(portno));
    queue_regs->pfa_portx_ipv6_mask[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_IPV6_MASK(portno));
    queue_regs->pfa_portx_ipv4_addr[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_IPV4_ADDR(portno));
    queue_regs->pfa_portx_ipv4_mask[portno] = pfa_readl(pfa_ctx->regs, PFA_PORTX_IPV4_MASK(portno));
    queue_regs->pfa_tdqx_pri[portno] = pfa_readl(pfa_ctx->regs, PFA_TDQX_PRI(portno));
}

void queue_portx_restore(struct pfa *pfa_ctx, unsigned int portno, struct pfa_queue_stash *queue_regs)
{
    pfa_writel(pfa_ctx->regs, PFA_PORTX_UDP_LMTNUM(portno), queue_regs->pfa_portx_udp_lmtnum[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_LMTBYTE(portno), queue_regs->pfa_portx_lmtbyte[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_PROPERTY(portno), queue_regs->pfa_portx_property[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_MAC_ADDR_L(portno), queue_regs->pfa_portx_mac_addr_l[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_MAC_ADDR_H(portno), queue_regs->pfa_protx_mac_addr_h[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_IPV6_ADDR_1ST(portno), queue_regs->pfa_portx_ipv6_addr_1s[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_IPV6_ADDR_2ND(portno), queue_regs->pfa_portx_ipv6_addr_2n[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_IPV6_ADDR_3TH(portno), queue_regs->pfa_portx_ipv6_addr_3t[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_IPV6_ADDR_4TH(portno), queue_regs->pfa_portx_ipv6_addr_4t[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_IPV6_MASK(portno), queue_regs->pfa_portx_ipv6_mask[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_IPV4_ADDR(portno), queue_regs->pfa_portx_ipv4_addr[portno]);
    pfa_writel(pfa_ctx->regs, PFA_PORTX_IPV4_MASK(portno), queue_regs->pfa_portx_ipv4_mask[portno]);
    pfa_writel(pfa_ctx->regs, PFA_TDQX_PRI(portno), queue_regs->pfa_tdqx_pri[portno]);
}

void config_hash_cache_en(struct pfa *pfa, unsigned int enable)
{
    pfa_writel(pfa->regs, PFA_HASH_CACHE_EN, enable);
}

void config_adq_threshold_and_len(struct pfa *pfa, unsigned int size)
{
    pfa_writel_relaxed(pfa->regs, PFA_ADQ_THRESHOLD, size);
    pfa_writel_relaxed(pfa->regs, PFA_ADQ0_LEN, PFA_ADQ_LONG);
    pfa_writel_relaxed(pfa->regs, PFA_ADQ1_LEN, PFA_ADQ_SHORT);
}

void config_qos_lmttime(struct pfa *pfa, unsigned int value)
{
    pfa_writel(pfa->regs, PFA_QOS_LMTTIME, value);
}

void config_eth_vlan_tag_sel(struct pfa *pfa, unsigned int flag)
{
    return;
}

void config_dport_adq_core_type(struct pfa *pfa, unsigned int portno, unsigned int type)
{
    return;
}

static void pfa_td_result_record(struct pfa_td_result_s *td_result, struct pfa_port_stat *stat)
{
    struct desc_result_s *result = &stat->result;

    result->td_result[td_result->td_trans_result]++;            // td result bit 0-1
    result->td_ptk_drop_rsn[td_result->td_drop_rsn]++;          // td result bit 2-5
    result->td_pkt_fw_path[__fls(td_result->td_trans_path)]++;  // td result bit 6-16
    result->td_pkt_type[td_result->td_pkt_type]++;              // td result bit 17-19
    result->td_warp[td_result->td_warp]++;                      // td result bit 20-21
    result->td_unwrap[td_result->td_unwrap]++;                  // td result bit 22-25

    return;
}

static inline void pfa_td_free_mem_v300(struct pfa *pfa, unsigned int portno, unsigned long long dra_addr)
{
    if (portno != pfa->usbport.portno && portno != pfa->pfa_tftport.portno) {
        pfa_free_dra_mem(pfa, portno, dra_addr);
    }

    return;
}

void pfa_check_td_result(struct pfa *pfa, unsigned int portno, struct pfa_td_desc *td_cur, struct pfa_port_stat *stat)
{
    struct pfa_td_result_s *td_result = NULL;
    unsigned long long dra_addr;
    unsigned int td_result_u32;

    td_result_u32 = td_cur->td_result;
    td_result = (struct pfa_td_result_s *)&td_result_u32;

    if (pfa->dbg_level & PFA_DBG_TD_RESULT) {
        pfa_td_result_record(td_result, stat);
    }
    if (unlikely(td_result->td_trans_result == TD_RESULT_DISCARD ||
                    td_result->td_trans_result == TD_RESULT_WRAP_OR_LENTH_WRONG)) {
        dra_addr = (unsigned long long)td_cur->td_inaddr_lower +
                    (((unsigned long long)td_cur->td_inaddr_upper) << 32);  /* left shit 32 bit */
        pfa_td_free_mem_v300(pfa, portno, dra_addr);
        stat->td_desc_fail_drop++;
    }
    stat->td_pkt_complete += td_result->td_pkt_ext_cnt;
}

void pfa_print_one_rd(struct pfa_rd_desc *cur_rd)
{
    PFA_ERR("rd_int_en %u \n", cur_rd->rd_int_en);
    PFA_ERR("rd_mode %u \n", cur_rd->rd_mode);
    PFA_ERR("rd_data_from_unsec %u \n", cur_rd->rd_data_from_unsec);
    PFA_ERR("rd_pfa_tft_dl_err_flag %u \n", cur_rd->rd_pfa_tft_dl_err_flag);
    PFA_ERR("rd_ack_flag %u \n", cur_rd->rd_ack_flag);
    PFA_ERR("rd_fc_head %u \n", cur_rd->rd_fc_head);
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
    PFA_ERR("rd_iptype %u \n", cur_rd->rd_iptype);

    PFA_ERR("\n RD word 4 \n");
    PFA_ERR("rd_trans_result %u \n", cur_rd->rd_trans_result);
    PFA_ERR("rd_drop_rsn %u \n", cur_rd->rd_drop_rsn);
    PFA_ERR("rd_trans_path %u \n", cur_rd->rd_trans_path);
    PFA_ERR("rd_trans_path_finish %u \n", cur_rd->rd_trans_path_finish);
    PFA_ERR("rd_pkt_type %u \n", cur_rd->rd_pkt_type);
    PFA_ERR("rd_finish_warp_res %u \n", cur_rd->rd_finish_warp_res);
    PFA_ERR("rd_tocpu_res %u \n", cur_rd->rd_tocpu_res);
    PFA_ERR("rd_bd_cd_len_not_same %u \n", cur_rd->rd_bd_cd_len_not_same);
    PFA_ERR("rd_updata_only %u \n", cur_rd->rd_updata_only);

    PFA_ERR("\n RD word 5 \n");
    PFA_ERR("rd_sport %x \n", cur_rd->rd_sport);
    PFA_ERR("rd_dport %x \n", cur_rd->rd_dport);
    PFA_ERR("rd_pktnum %x \n", cur_rd->rd_pktnum);
    PFA_ERR("rd_ethtype %x \n", cur_rd->rd_ethtype);

    PFA_ERR("\n RD word 6-9 \n");
    PFA_ERR("rd_pfa_tftres_stmid %x \n", cur_rd->rd_pfa_tftres_stmid);
    PFA_ERR("rd_user_field0 %x \n", cur_rd->rd_user_field0);
    PFA_ERR("rd_user_field1 %x \n", cur_rd->rd_user_field1);
    PFA_ERR("rd_user_field2 %x \n", cur_rd->rd_user_field2);
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
    PFA_ERR("td_modem_id %u \n", cur_td->td_modem_id);
    PFA_ERR("td_stick_en %u \n", cur_td->td_stick_en);
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
    unsigned int mod;

    mod = pfa_readl(pfa->regs, PFA_DEBUG_WORD);
    mod |= 0x1;

    pfa_writel(pfa->regs, PFA_DEBUG_WORD, mod);
}

void enable_macfw_to_ip(struct pfa *pfa, unsigned int portno, unsigned int enable)
{
    unsigned int value;

    value = pfa_readl_relaxed(pfa->regs, PFA_MAC_IP_FW_FLAG);
    if (enable) {
        pfa_writel(pfa->regs, PFA_MAC_IP_FW_FLAG, value |= BIT(portno));
    } else {
        pfa_writel(pfa->regs, PFA_MAC_IP_FW_FLAG, value &= (~BIT(portno)));
    }
}

void pfa_clk_gate_switch(struct pfa *pfa, unsigned int enable)
{
    unsigned int clken_sel = 0;

    if (enable) {
        clken_sel = 0x3f;
    }
    pfa_writel(pfa->regs, CLKEN_SEL, clken_sel);
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
    // td result bit 0-1
    PFA_ERR("[td_result]:updata_only %u \n", stat->result.td_result[TD_RESULT_UPDATA_ONLY]);
    PFA_ERR("[td_result]:discard %u \n", stat->result.td_result[TD_RESULT_DISCARD]);
    PFA_ERR("[td_result]:normal %u \n", stat->result.td_result[TD_RESULT_NORMAL]);
    PFA_ERR("[td_result]:wrap_or_lenth_wrong %u \n", stat->result.td_result[TD_RESULT_WRAP_OR_LENTH_WRONG]);

    // td result bit 2-5
    PFA_ERR("[td_ptk_drop_rsn]:trans_succ %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_TRANS_SUCC]);
    PFA_ERR("[td_ptk_drop_rsn]:unwrap_err %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_UNWRAP_ERR]);
    PFA_ERR("[td_ptk_drop_rsn]:length_err %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_LENGTH_ERR]);
    PFA_ERR("[td_ptk_drop_rsn]:sport_disable %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_SPORT_DISABLE]);
    PFA_ERR("[td_ptk_drop_rsn]:mac_filt %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_MAC_FILT]);
    PFA_ERR("[td_ptk_drop_rsn]:ttl_zero %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_TTL_ZERO]);
    PFA_ERR("[td_ptk_drop_rsn]:ip_filt %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_IP_FILT]);
    PFA_ERR("[td_ptk_drop_rsn]:udp_rate_limit %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_UDP_RATE_LIMIT]);
    PFA_ERR("[td_ptk_drop_rsn]:mac_fw_entry_err %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_MAC_FW_ENTRY_ERR]);
    PFA_ERR("[td_ptk_drop_rsn]:dport_disable %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_DPORT_DISABLE]);
    PFA_ERR("[td_ptk_drop_rsn]:qos_limit %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_QOS_LIMIT]);
    PFA_ERR("[td_ptk_drop_rsn]:v6_hop_limit_zero %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_V6_HOP_LIMIT_ZERO]);
    PFA_ERR("[td_ptk_drop_rsn]:rd_ptr_null %u \n", stat->result.td_ptk_drop_rsn[TD_DROP_RSN_RD_PTR_NULL]);

    // td result bit 6-16
    PFA_ERR("[td_pkt_fw_path]:mac_filter %u \n", stat->result.td_pkt_fw_path[TD_FW_MAC_FILTER]);
    PFA_ERR("[td_pkt_fw_path]:eth_type %u \n", stat->result.td_pkt_fw_path[TD_FW_ETH_TYPE]);
    PFA_ERR("[td_pkt_fw_path]:1st_macfw %u \n", stat->result.td_pkt_fw_path[TD_FW_1ST_MACFW]);
    PFA_ERR("[td_pkt_fw_path]:ip_hdr %u \n", stat->result.td_pkt_fw_path[TD_FW_IP_HDR_CHECK]);
    PFA_ERR("[td_pkt_fw_path]:ip_filter %u \n", stat->result.td_pkt_fw_path[TD_FW_IP_FILTER]);
    PFA_ERR("[td_pkt_fw_path]:tcp_or_udp %u \n", stat->result.td_pkt_fw_path[TD_FW_TCP_UDP_CHECK]);
    PFA_ERR("[td_pkt_fw_path]:hash_calculate %u \n", stat->result.td_pkt_fw_path[TD_FW_HASH_CALC]);
    PFA_ERR("[td_pkt_fw_path]:hash_cache %u \n", stat->result.td_pkt_fw_path[TD_FW_HASH_CACHE]);
    PFA_ERR("[td_pkt_fw_path]:hash_ddr %u \n", stat->result.td_pkt_fw_path[TD_FW_HASH_DDR]);
    PFA_ERR("[td_pkt_fw_path]:2nd_macfw %u \n", stat->result.td_pkt_fw_path[TD_FW_2ND_MACFW]);
    PFA_ERR("[td_pkt_fw_path]:complete %u \n", stat->result.td_pkt_fw_path[TD_FW_COMPLETE]);

    // td result bit 17-19
    PFA_ERR("[td_pkt_type]:ipv4_tcp %u \n", stat->result.td_pkt_type[TD_PKT_IPV4_TCP]);
    PFA_ERR("[td_pkt_type]:ipv4_udp %u \n", stat->result.td_pkt_type[TD_PKT_IPV4_UDP]);
    PFA_ERR("[td_pkt_type]:ipv4_non_udp_tcp %u \n", stat->result.td_pkt_type[TD_PKT_IPV4_NON_UDP_TCP]);
    PFA_ERR("[td_pkt_type]:ipv6_tcp %u \n", stat->result.td_pkt_type[TD_PKT_IPV6_TCP]);
    PFA_ERR("[td_pkt_type]:ipv6_udp %u \n", stat->result.td_pkt_type[TD_PKT_IPV6_UDP]);
    PFA_ERR("[td_pkt_type]:ipv6_non_udp_tcp %u \n", stat->result.td_pkt_type[TD_PKT_IPV6_NON_UDP_TCP]);
    PFA_ERR("[td_pkt_type]:non_udp_tcp %u \n", stat->result.td_pkt_type[TD_PKT_NON_UDP_TCP]);
    PFA_ERR("[td_pkt_type]:trans_before_ip_check %u \n", stat->result.td_pkt_type[TD_PKT_TRANS_BEFORE_IP_CHECK]);

    // td result bit 20-21
    PFA_ERR("[td_warp]:success %u \n", stat->result.td_warp[TD_WARP_SUCCESS]);
    PFA_ERR("[td_warp]:discard %u \n", stat->result.td_warp[TD_WARP_PART_DISCARD]);
    PFA_ERR("[td_warp]:all_discard %u \n", stat->result.td_warp[TD_WARP_ALL_DISCARD]);
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
            PFA_ERR("mac entry (%d) : %pM bid:%u vid:%u port:%u timestamp:0x%x static:%u action:%u redir_port:%u\n", i,
                mac_entry->mac, ((unsigned)mac_entry->bid) & 0xf, ((unsigned)mac_entry->vid) & 0xfff,
                ((unsigned)mac_entry->port) & 0x1f, ((unsigned)mac_entry->timestamp) & 0xffff,
                ((unsigned)mac_entry->is_static) & 0x1, ((unsigned)mac_entry->action) & 0x1,
                ((unsigned)mac_entry->rd_port) & 0xf);
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
    return PFA_VERSION_V0;
}

void pfa_config_sc(struct pfa *pfa, unsigned int idx, unsigned int value)
{
    pfa_writel_relaxed(pfa->regs, PFA_SC_REG(idx), value);
}

int pfa_get_pdcp_cnt(struct pfa_rd_desc *desc)
{
    return 0;
}

EXPORT_SYMBOL(pfa_construct_skb);
