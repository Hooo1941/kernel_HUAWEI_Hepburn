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

#ifndef __PFA_IP_ENTRY_H__
#define __PFA_IP_ENTRY_H__

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>

#include <mdrv_pfa.h>

#include "pfa.h"

#define PFA_IPFW_HTABLE_ZONE (0)
#define PFA_IPFW_HTABLE_RAND (0)
#define IPV4_ADDR_LEN 4

#define IS_IP_FL_ENTRY (BIT(31))
#define PFA_IPFW_HTABLE_EMPTY_SIZE 1

struct pfa_ip_fw_entry_priv {
    struct list_head list;
    struct list_head wan_list;
    struct net_device *wan_dev;
    struct pfa_ip_fw_entry *entry;
    dma_addr_t dma;
    unsigned int dst_net;
    int dir;
    unsigned int add_cnt;
    unsigned int remove_cnt;
    unsigned int hash;
    unsigned int hkey;
    struct nf_conn *contrack;
    struct net_device *in;
    struct net_device *out;
};

struct pfa_ip_fw_entry {
    unsigned int next_lower : 32;  /* next ip fw entry address for hardware, */
    unsigned int next_higher : 32; /* next ip fw entry address for hardware, higher 24bit is reserved */

    struct nf_conntrack_tuple tuple; /* tuple to track the stream */

    // word 12
    unsigned int timestamp; /* timestamp used to expire the entry */

    // word 13
    unsigned int vid : 16;       /* vid in big endian */
    unsigned int portno : 8;     /* pfa port no */
    unsigned int net_id : 8; /* mutile ncm port id */

    // word 14
    unsigned int nat_src_ip; /* manip data for SNAT */
    // word 15
    unsigned int nat_src_port : 16; /* manip data for SPORT */
    unsigned int action : 8;        /* nat action.00:nocation 01:dst nat 10:src nat 11:src and dst nat */
    unsigned int iptype : 1;        /* 0-ipv4 1-ipv6 */
    unsigned int is_static : 1;     /* 0-dynamic 1-static */
    unsigned int ttl_ind : 1;       /* 0-router mode, ttl minus 1, 1-stick mode ttl unchange */
    unsigned int reserve1 : 4;
    unsigned int valid : 1; /* 0-invalid 1-valid */
    // word 16
    unsigned char dhost[ETH_ALEN]; /* dest mac in big endian */
    unsigned char shost[ETH_ALEN]; /* src mac in big endian */
    // word 19
    unsigned int dead_timestamp; /* timestamp when entry set to invalid */
    unsigned int stream_id;      /* calc using quintuple(ip*2 port*2 protocal) by software */
    // word 21
    unsigned int pdu_ssid : 8;
    unsigned int fc_head : 4;
    unsigned int modem_id : 2;
    unsigned int pfa_tft_reserve : 2;
    unsigned int htab_usrfield_lower16 : 16; /* usrfield for ads */
    // word 22
    unsigned int nat_dst_ip;                  /* manip data for DNAT */
    unsigned int nat_dst_port : 16;           /* manip data for DPORT */
    unsigned int htab_usrfield_higher16 : 16; /* usrfield for ads */
    // word 25
    unsigned int prev_lower : 32;  /* prev ip fw entry address for hardware, */
    unsigned int prev_higher : 32; /* prev ip fw entry address for hardware, higher 24bit is reserved */

    /* the following is in ddr memory but will not be read by pfa hardware */
    struct pfa_ip_fw_entry_priv *priv;
} __attribute__((aligned(4)));

#define PFA_IP_FW_ENTRY_RSV_NUM 4
struct pfa_ip_fw_entry_dma {
    unsigned int rsv[PFA_IP_FW_ENTRY_RSV_NUM];
    struct pfa_ip_fw_entry entry;
};
#define PFA_IP_FW_TO_DMA_OFFSET(x) ((unsigned long long)(x) - PFA_IP_FW_ENTRY_RSV_NUM * 4)
#define PFA_IP_FW_TO_ENTRY_OFFSET(x) ((unsigned long long)(x) + PFA_IP_FW_ENTRY_RSV_NUM * 4)

struct pfa_ipv4_filt_entry {
    unsigned int sip; /* source ip in big endian */
    unsigned int sip_mask;
    unsigned int dip; /* dest ip in big endian */
    unsigned int dip_mask;
    unsigned int sport_min : 16; /* source min port in big endian */
    unsigned int sport_max : 16; /* source max port in big endian */
    unsigned int dport_min : 16; /* dest min port in big endian */
    unsigned int dport_max : 16; /* dest max port in big endian */
    unsigned int protocol : 8;
    unsigned int tos : 8;
    unsigned int filt_mask : 8;
    unsigned int reserve : 8;
};

struct pfa_ipv6_filt_entry {
    unsigned int sip[4]; /* source ip in big endian, 4 is ip addr */
    unsigned int sip_mask;
    unsigned int dip[4]; /* dest ip in big endian, 4 is ip addr */
    unsigned int dip_mask;

    unsigned int sport_min : 16; /* source min port in big endian, 16 bit width */
    unsigned int sport_max : 16; /* source max port in big endian, 16 bit width */
    unsigned int dport_min : 16; /* dest min port in big endian, 16 bit width */
    unsigned int dport_max : 16; /* dest max port in big endian, 16 bit width */

    unsigned int next_hdr : 8;  /* 8 bit width */
    unsigned int tc : 8;        /* 8 bit width */
    unsigned int reserve1 : 16; /* 16 bit width */

    unsigned int flow_label : 20; /* flow label in big endian, 20 bit width */
    unsigned int reserve2 : 12; /* 12 bit width */

    unsigned int filt_mask : 8; /* 8 bit width */
    unsigned int valid : 1; /* 1 bit width */
    unsigned int reserve3 : 23; /* 23 bit width */
};

struct pfa_ipv4_filter_entry_in_sram {
    unsigned int sip_mask : 8; /* 8 bit width */
    unsigned int dip_mask : 8; /* 8 bit width */
    unsigned int protocol : 8; /* 8 bit width */
    unsigned int tos : 8; /* 8 bit width */

    unsigned int filter_mask : 8; /* 8 bit width */
    unsigned int reserve : 23; /* 23 bit width */
    unsigned int valid : 1; /* 1 bit width */

    unsigned int sip;
    unsigned int dip;
    unsigned int sport_min : 16; /* 16 bit width */
    unsigned int sport_max : 16; /* 16 bit width */
    unsigned int dport_min : 16; /* 16 bit width */
    unsigned int dport_max : 16; /* 16 bit width */
};

struct pfa_ipv6_filter_entry_in_sram {
    unsigned int sip_mask : 8; /* 8 bit width */
    unsigned int dip_mask : 8; /* 8 bit width */
    unsigned int next_hdr : 8; /* 8 bit width */
    unsigned int tc : 8; /* 8 bit width */

    unsigned int filter_mask : 8; /* 8 bit width */
    unsigned int flow_label : 20; /* 20 bit width */
    unsigned int reserve : 3; /* 3 bit width */
    unsigned int valid : 1; /* 1 bit width */

    short sip[8]; /* 8 is ip addr len */
    short dip[8]; /* 8 is ip addr len */
    unsigned int sport_min : 16; /* 16 bit width */
    unsigned int sport_max : 16; /* 16 bit width */
    unsigned int dport_min : 16; /* 16 bit width */
    unsigned int dport_max : 16; /* 16 bit width */
};

enum pfa_manip_type {
    PFA_NF_NAT_MANIP_NONE,  // 00
    PFA_NF_NAT_MANIP_DST,   // 01
    PFA_NF_NAT_MANIP_SRC,   // 10
    PFA_NF_NAT_MANIP_BOTH   // 11
};

int pfa_ip_entry_table_init(struct pfa *pfa);
void pfa_ip_entry_table_exit(struct pfa *pfa);
void pfa_transfer_pause(void);
void pfa_transfer_restart(void);
void pfa_ip_fw_htab_set(void);
int pfa_skb_is_updateonly(void *skb);

#ifdef CONFIG_PFA_FW

void pfa_ip_fw_add(void *n, void *skb);
void pfa_ip_fw_del(void *ct);
void pfa_aging_timer_set(void);
void pfa_ipfw_list_del_entry(struct pfa *pfa, struct pfa_ip_fw_entry *pos, uint32_t hkey);
int pfa_direct_fw_record_add_waninfo(struct pfa_ip_fw_entry *entry);;
int pfa_wan_ip_fw_show(void);
void pfa_add_err_show(void);
void pfa_ip_fw_del_one(struct pfa *pfa, struct nf_conntrack_tuple *tuple);
void pfa_ip_fw_del_all_entry(void);
void pfa_add_ipfw_entry_cb(struct pfa *pfa, struct sk_buff *skb, struct pfa_ip_fw_entry *entry, int dir, unsigned int hkey);
#else

static inline void pfa_ip_fw_add(void *n, void *skb)
{
    return;
}
static inline void pfa_ip_fw_del(void *ct)
{
    return;
}
static inline void pfa_aging_timer_set(void)
{
    return;
}
static inline void pfa_ip_fw_del_all_entry(void)
{
    return;
}
#endif

#ifdef CONFIG_PFA_DIRECT_FW
void ip_fw_list_add(struct pfa *pfa, struct pfa_ip_fw_entry *entry, uint32_t hkey);
void ip_fw_node_put(struct pfa *pfa, struct pfa_ip_fw_entry *ent, bool to_pool);
void pfa_ip_fw_write_wan_info(struct pfa *pfa, struct wan_dev_info_s *wan_dev_info, struct pfa_ip_fw_entry *entry);

unsigned int pfa_ipfw_get_hash(struct nf_conntrack_tuple *tuple, u16 zone);
struct pfa_ip_fw_entry *ip_fw_entry_exist(struct pfa *pfa, struct nf_conntrack_tuple *tuple, uint32_t hkey);
struct pfa_ip_fw_entry *ip_fw_node_get(struct pfa *pfa);
void ip_fw_node_put(struct pfa *pfa, struct pfa_ip_fw_entry *ent, bool to_pool);

#endif

#endif /* __PFA_IP_ENTRY_H__ */
