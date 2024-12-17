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

#ifndef __PFA_MAC_ENTRY_H__
#define __PFA_MAC_ENTRY_H__

#include <linux/types.h>
#include <linux/slab.h>

#include "pfa.h"

#define IS_MAC_FW_ENTRY (BIT(17))
#define IS_MAC_FL_ENTRY (BIT(16))
#define IS_QOS_MAC_ENTRY (BIT(26))

struct pfa_mac_fw_entry_ext {
    struct list_head list;
    struct pfa_mac_fw_entry ent;
};

struct pfa_mac_filt_entry {
    unsigned int mac_hi;        /* mac in big endian */
    unsigned int mac_lo;        /* mac in big endian */
    unsigned int redir_act : 1; /* 0:no redir 1:redir */
    unsigned int reserve1 : 7;
    unsigned int redir_port : 5; /* redir port */
    unsigned int reserve2 : 19;
};

struct pfa_mac_pdu_entry {
    unsigned int index : 5;
    unsigned int rule_type : 1; /* 0 – Ethernet 1 – Match All */
    unsigned int mask : 6; /* mask[5:0]的屏蔽字段依次对应：c_pcp_dei、s_pcp_dei、c_vid、s_vid、eth_type、dmac */
    unsigned int modem_id : 2;
    unsigned int reserve : 1;
    unsigned int pdu_session_status : 1; /* PDU Session Status, 0 – Inactive，1 – Active */
    unsigned int eth_type : 16; /* ethernet type */
    unsigned int s_tag_vid_start : 12;
    unsigned int s_tag_pcp_dei : 4;
    unsigned int s_tag_vid_end : 12;
    unsigned int s_tag_pcp_dei_mask : 4;
    unsigned int c_tag_vid_start : 12;
    unsigned int c_tag_pcp_dei : 4;
    unsigned int c_tag_vid_end : 12;
    unsigned int c_tag_pcp_dei_mask : 4;
    unsigned int dmac_hi; /* mac in big endian */
    unsigned int dmac_lo : 16; /* mac in big endian */
    unsigned int pdu_session_id : 8;
    unsigned int fc_head : 4;
    unsigned int reserve1 : 3;
};

struct pfa_mac_pdu_entry_ram {
    unsigned int index : 5;
    unsigned int rule_type : 1; /* 0 – Ethernet 1 – Match All */
    unsigned int mask : 6; /* mask[5:0]的屏蔽字段依次对应：c_pcp_dei、s_pcp_dei、c_vid、s_vid、eth_type、dmac */
    unsigned int modem_id : 2;
    unsigned int reserve : 1;
    unsigned int pdu_session_status : 1; /* PDU Session Status, 0 – Inactive，1 – Active */
    unsigned int eth_type : 16; /* ethernet type */
    unsigned int s_tag_vid_start : 12;
    unsigned int s_tag_pcp_dei : 4;
    unsigned int s_tag_vid_end : 12;
    unsigned int s_tag_pcp_dei_mask : 4;
    unsigned int c_tag_vid_start : 12;
    unsigned int c_tag_pcp_dei : 4;
    unsigned int c_tag_vid_end : 12;
    unsigned int c_tag_pcp_dei_mask : 4;
    unsigned int dmac_hi; /* mac in big endian */
    unsigned int dmac_lo : 16; /* mac in big endian */
    unsigned int pdu_session_id : 8;
    unsigned int pdu_session_vld : 1; /* PDU Session的有效指示。0 – 无效，1 – 有效 */
    unsigned int fc_head : 4;
    unsigned int reserve1 : 3;
};

struct pfa_ip_qos_entry {
    unsigned int sip[4]; /* 4 IP SIZE;源IP地址。IPV4：使用32Bit   IPV6：使用128Bit */
    unsigned int sip_mask : 8; /* 源IP的mask中1的个数 */
    unsigned int rsv0 : 24;
    unsigned int dip[4]; /* 4 IP SIZE;目的IP地址。IPV4：使用32Bit   IPV6：使用128Bit */
    unsigned int dip_mask : 8; /* 目的IP的mask中1的个数 */
    unsigned int rsv1 : 24;
    unsigned int sport_min : 16;
    unsigned int sport_max : 16;
    unsigned int dport_min : 16;
    unsigned int dport_max : 16;
    /* 13th word */
    unsigned int filter_mask : 8; /* 0 Bit:sip 1:dip 2:sport 3:dport 4:protocol/next_header 5:dscp 6:flow_lable */
    unsigned int dscp_min : 6;
    unsigned int dscp_max : 6; /* IPV4为type of service域，前6bit是dscp IPV6为traffic class域，前6bit是dscp */
    unsigned int rsv2 : 12;
    /* 14th word */
    unsigned int flow_label : 20; /* IPV4：保留 IPV6：flow_lable */
    unsigned int iptype : 1; /* 0: ipv4 1: ipv6 */
    unsigned int rsv3 : 11;
    /* 15th word */
    unsigned int protocol_or_next_hdr : 8; /* IPV4：protocol IPV6：next header */
    unsigned int priority : 3;
    unsigned int discard_time : 8;
    unsigned int rsv4 : 13;
};

struct pfa_ip_qos_entry_ram {
    unsigned int sip_mask : 8; /* 源IP的mask中1的个数 */
    unsigned int dip_mask : 8; /* 目的IP的mask中1的个数 */
    unsigned int fillter_mask : 8; /* 0 Bit:sip 1:dip 2:sport 3:dport 4:protocol/next_header 5:dscp 6:flow_lable */
    unsigned int dscp_min : 6;
    unsigned int reserve : 2;
    unsigned int dscp_max : 6; /* IPV4为type of service域，前6bit是dscp IPV6为traffic class域，前6bit是dscp */
    unsigned int rsv : 2;
    unsigned int flow_label : 20; /* IPV4：保留 IPV6：flow_lable */
    unsigned int rsv1 : 2;
    unsigned int iptype : 1; /* 0: ipv4 1: ipv6 */
    unsigned int valid : 1; /* 表项有效标志 */
    unsigned int sip[4]; /* 4 IP SIZE;源IP地址。IPV4：使用32Bit   IPV6：使用128Bit */
    unsigned int dip[4]; /* 4 IP SIZE;目的IP地址。IPV4：使用32Bit   IPV6：使用128Bit */
    unsigned int sport_min : 16;
    unsigned int sport_max : 16;
    unsigned int dport_min : 16;
    unsigned int dport_max : 16;
    unsigned int protocol_or_next_hdr : 8; /* IPV4：protocol IPV6：next header */
    unsigned int rsv2 : 5;
    unsigned int priority : 3;
    unsigned int discard_time : 8;
    unsigned int rsv3 : 8;
    unsigned int rsv4;
};

struct pfa_qos_mac_entry {
    unsigned int mac_hi; /* mac in big endian */
    unsigned int mac_lo; /* mac in big endian */
    unsigned int limit_byte_lo;
    unsigned int limit_byte_hi : 8;
    unsigned int limit_en : 2;
    unsigned int reserver : 22;
};

/* pfa_mac_qos_entry read from sram */
struct pfa_qos_mac_entry_low_in_sram {
    unsigned long long lmt_en : 2;
    unsigned long long lmt_byte : 40;
    unsigned long long mac_low : 22;
};

struct pfa_qos_mac_entry_high_in_sram {
    unsigned long long mac_high : 26;
    unsigned long long vid : 1;
    unsigned long long reserve0 : 37;
};

struct pfa_qos_mac_entry_in_sram {
    struct pfa_qos_mac_entry_low_in_sram ent_low;
    struct pfa_qos_mac_entry_high_in_sram ent_high;
};

int pfa_mac_entry_table_init(struct pfa *pfa);
void pfa_mac_entry_table_exit(struct pfa *pfa);

#ifdef CONFIG_PFA_FW
void pfa_mac_fw_add(const unsigned char *mac, unsigned short vid, unsigned int br_pfano, unsigned int src_pfano,
    unsigned int is_static);
void pfa_mac_fw_del(const unsigned char *mac, unsigned short vid, unsigned int br_pfano, unsigned int src_pfano);
int pfa_mac_entry_config(enum pfa_tab_ctrl_type type, const unsigned int *buf, unsigned int size);
int pfa_mac_filt_add(struct pfa_mac_filt_entry *ent);
int pfa_mac_filt_del(struct pfa_mac_filt_entry *ent);
void pfa_macfw_entry_dump(void);
void pfa_macfl_entry_dump(void);

void tab_clr(unsigned int is_write);
void pfa_print_macfw_backup(void);
void pfa_macfw_entry_clear(void);

int pfa_qos_mac_lmt_add(struct pfa_qos_mac_entry *ent);
int pfa_qos_mac_lmt_del(struct pfa_qos_mac_entry *ent);
unsigned int pfa_mac_to_ip_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);
#else
static inline void pfa_mac_fw_add(const unsigned char *mac, unsigned short vid, unsigned int br_pfano,
    unsigned int src_pfano, unsigned int is_static)
{
    return;
};
static inline void pfa_mac_fw_del(const unsigned char *mac, unsigned short vid, unsigned int br_pfano,
    unsigned int src_pfano)
{
    return;
}
static inline int pfa_mac_entry_config(enum pfa_tab_ctrl_type type, const unsigned int *buf, unsigned int size)
{
    return 0;
}
static inline int pfa_mac_filt_add(struct pfa_mac_filt_entry *ent)
{
    return 0;
}
static inline int pfa_mac_filt_del(struct pfa_mac_filt_entry *ent)
{
    return 0;
}
static inline void pfa_macfw_entry_dump(void)
{
    return;
}
static inline void pfa_macfl_entry_dump(void)
{
    return;
}
#endif
#endif /* __PFA_MAC_ENTRY_H__ */
