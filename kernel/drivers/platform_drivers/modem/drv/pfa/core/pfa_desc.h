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

#ifndef __PFA_DESC_H__
#define __PFA_DESC_H__

#include <linux/types.h>
#include "pfa.h"

#define RNDIS_HDR_LEN 11
#define PFA_SPORT_FREE_ORI_ADDR_FLAG BIT(31) // SNEV200, td userfield1 bit31

struct pfa_nth32 {
    unsigned int sig;
    unsigned int nth_len : 16;
    unsigned int seq : 16;
    unsigned int total_len;
    unsigned int index;
};

struct pfa_ndp32_hdr {
    unsigned int sig;
    unsigned int nth_len : 16;
    unsigned int reserve : 16;
    unsigned int next_ndp;
    unsigned int reserve1;
};

struct pfa_ndp32_data {
    unsigned int index;
    unsigned int len;
};

struct pfa_ncm_data {
    unsigned int addr_low;
    unsigned int addr_high;
    unsigned int len;
};

struct pfa_rndis_hdr {
    unsigned int rndis_hdr[RNDIS_HDR_LEN];
};

struct pfa_rndis_data {
    struct pfa_rndis_hdr hdr;
    unsigned int addr_low;
    unsigned int addr_high;
};

struct pfa_td_user_info {
    unsigned long long dra_l2_addr;
    unsigned int len;
    void *param;
    unsigned int flags;
    unsigned int net_id;
};

#define PFA_ADQ_SIZE_DEFAULT 256

#define PFA_ADQ_LONG 0x1
#define PFA_ADQ_SHORT 0x0
#define PFA_MIN_PKT_SIZE 1

#define PFA_AD0_PKT_LEN DRA_OPIPE_LEVEL_0_SIZE /* short adq */
#define PFA_AD1_PKT_LEN DRA_OPIPE_LEVEL_1_SIZE /* long adq */
#define PFA_AD_PKT_LEN_THR PFA_AD0_PKT_LEN     /* short adq */

#define PFA_AD_INTR_EN 0x80000000
#define LATENCY_MSGID_PFA 0x9f38102d
#define LATENCY_PDCP_CNT 0x9f381030

#define PFA_STOMS (1000 * 1000)
#define TD_MODE_INDEX(bit) ((bit) >> 0x8)

#define PFA_MACHDR_TO_DRA_OFFSET_MAX 32 /* double vlan is 22 */

int pfa_ad_init(struct pfa *pfa);
void pfa_ad_exit(struct pfa *pfa);
void pfa_update_adq_wptr(struct pfa *pfa);
void pfa_process_pfa_tft_td_desc(struct pfa *pfa);
unsigned int pfa_process_rd_desc(unsigned int portno);
unsigned int pfa_process_td_desc(unsigned int portno);
void pfa_adq_timer_init(struct pfa *pfa);
void pfa_rd_per_interval_cnt(struct pfa_port_ctx *port_ctx, unsigned int rd_total_num);
int pfa_construct_skb(struct pfa_port_ctx *port_ctx, struct pfa_rd_desc *desc, struct sk_buff *skb,
    unsigned long long org_dra_addr, unsigned long long dra_l2_addr, unsigned int dra_addr_type);
void pfa_enable_port_bypass(unsigned int enable, unsigned int sport, unsigned int dport,
    unsigned int mac_ursp_with);
void pfa_rd_process_cpu_wan(struct pfa *pfa, struct pfa_port_ctx *port_ctx, struct pfa_rd_desc *desc,
    unsigned long long dra_l2_addr);
void pfa_rd_process_cpu_lan(struct pfa *pfa, struct pfa_port_ctx *port_ctx, struct pfa_rd_desc *desc,
    unsigned long long dra_l2_addr);
void pfa_rd_process_cpuport_desc(struct pfa *pfa, struct pfa_port_ctx *port_ctx, struct pfa_rd_desc *desc,
    unsigned long long dra_l2_addr);
unsigned int pfa_get_skb_update_only_value(struct sk_buff *skb);
void pfa_set_skb_update_only_value(struct sk_buff *skb, unsigned int value);
void pfa_set_latency_rpt(unsigned int enable);
void pfa_free_dra_mem(struct pfa *pfa, unsigned int portno, unsigned long long dra_addr);
void pfa_pts_rpt(unsigned int pkt_id, enum fw_pos pos);
void pfa_ad_dts_init(struct pfa *pfa);

#endif /* __PFA_DESC_H__ */
