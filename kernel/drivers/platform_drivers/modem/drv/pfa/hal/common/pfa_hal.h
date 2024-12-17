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

#ifndef PFA_HAL_C_H
#define PFA_HAL_C_H

#include "pfa.h"
#include "pfa_tft_reg.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

void tft_config_rd_ip_head(unsigned int ext_desc_en);
void config_tft_bd_rd_ip_head(struct pfa *pfa);
void pfa_print_one_rd(struct pfa_rd_desc *cur_rd);
void pfa_print_one_td(struct pfa_td_desc *cur_td);
void queue_portx_save(struct pfa *pfa, unsigned int portno, struct pfa_queue_stash *queue_regs);
void queue_portx_restore(struct pfa *pfa_ctx, unsigned int portno, struct pfa_queue_stash *queue_regs);
void config_hash_cache_en(struct pfa *pfa, unsigned int enable);
void config_hash_value(struct pfa *pfa, unsigned int hash);
void config_tft_bd_rd_ptr_update(struct pfa *pfa, struct iport_pfa_tft_addr *pfa_tft_reg);
void config_adq_threshold_and_len(struct pfa *pfa, unsigned int size);
void config_qos_lmttime(struct pfa *pfa, unsigned int value);
void config_eth_vlan_tag_sel(struct pfa *pfa, unsigned int flag);
void config_dport_adq_core_type(struct pfa *pfa, unsigned int port_num, unsigned int type);
void set_update_only_produce_mod(struct pfa *pfa);
void enable_macfw_to_ip(struct pfa *pfa, unsigned int portno, unsigned int enable);
void pfa_clk_gate_switch(struct pfa *pfa, unsigned int enable);
void pfa_check_td_result(struct pfa *pfa, unsigned int portno, struct pfa_td_desc *td_cur, struct pfa_port_stat *stat);
void pfa_rd_record_result(struct pfa_rd_desc *cur_desc, struct pfa_port_stat *stat);
void pfa_config_td_tft_mode(struct wan_info_s *wan_info, struct pfa_td_desc *desc);
void dbgen_en(void);
void dbgen_dis(void);
int pfa_check_idle(struct pfa *pfa);
void pfa_ackpkt_identify(struct pfa *pfa, unsigned int portno);
void pfa_update_usb_max_pkt_cnt(struct pfa *pfa, unsigned int pack_max_pkt_cnt);
void pfa_update_pack_max_len(struct pfa *pfa, unsigned int pack_max_len);
void pfa_config_port_td_pri(struct pfa *pfa, unsigned int portno, unsigned int pri);
void pfa_config_port_en(struct pfa *pfa, unsigned int portno, unsigned int en);
void pfa_td_usb_result_print(struct pfa_port_ctrl *ctrl, struct pfa_port_stat *stat);
void pfa_td_result_print(struct pfa_port_ctrl *ctrl, struct pfa_port_stat *stat);
void pfa_clean_ad_buf(struct pfa *pfa);
void pfa_show_mac_entry(struct pfa *pfa, unsigned int flag);
int pfa_construct_skb(struct pfa_port_ctx *port_ctx, struct pfa_rd_desc *desc, struct sk_buff *skb,
    unsigned long long org_dra_addr, unsigned long long dra_l2_addr, unsigned int dra_addr_type);
void pfa_enable(void);
void pfa_disable(void);
void pfa_config_port_td_rd_info(unsigned int portno, unsigned int td_depth, unsigned int rd_depth);
void pfa_enc_type_init(struct pfa *pfa);
void pfa_config_port_enc_type(struct pfa *pfa, unsigned int portno, int enc_type);
void pfa_config_sc(struct pfa *pfa, unsigned int idx, unsigned int value);
int pfa_get_version(void);
int pfa_get_pdcp_cnt(struct pfa_rd_desc *desc);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* PFA_HAL_C_H */
