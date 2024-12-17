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

#ifndef __PFA_HAL_DESC_H__
#define __PFA_HAL_DESC_H__
#include <linux/if_ether.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

enum pfa_td_ptk_drop_rsn {
    TD_DROP_RSN_TRANS_SUCC = 0,
    TD_DROP_RSN_SPORT_DISABLE,
    TD_DROP_RSN_LENGTH_ERR,
    TD_DROP_RSN_UNWRAP_ERR,
    TD_DROP_RSN_MAC_FILT,
    TD_DROP_RSN_RESERVE0,
    TD_DROP_RSN_TTL_ZERO,
    TD_DROP_RSN_IP_FILT,
    TD_DROP_RSN_UDP_RATE_LIMIT,
    TD_DROP_RSN_MAC_FW_ENTRY_ERR,
    TD_DROP_RSN_DPORT_DISABLE,
    TD_DROP_RSN_QOS_LIMIT,
    TD_DROP_RSN_DPORT_ERR,
    TD_DROP_RSN_V6_HOP_LIMIT_ZERO,
    TD_DROP_RSN_RD_PTR_NULL,
    TD_DROP_RSN_BOTTOM
};

struct pfa_td_desc {
    /* word0 packet input header, suppose to point at ip header */
    unsigned int td_inaddr_lower; // lower 32bit for input packet phy or maa addr;
    unsigned int td_inaddr_upper; // lower 32bit for input packet phy or maa addr;
    /* word2 */
    unsigned int pkt_dra_offset : 5;
    unsigned int dra_offset_positive : 1;
    unsigned int td_pkt_len : 16;      // for pfa; packet len
    unsigned int pkt_ind : 2; // 1’b0 eth;  1‘b1 IP; other
    unsigned int pkt_sec_n : 1; // no use, pkt sec use ch sec
    unsigned int td_copy_en : 1;
    unsigned int td_bypass_en : 1;
    unsigned int td_bypass_addr : 5;

    /* word3 */
    unsigned int tft_filter_ind : 1; // enable tft filter
    unsigned int td_host_ana : 1;     // analysis_ind
    unsigned int td_stick_en : 1;
    unsigned int td_with_mac_pdu_rst : 1; // bypass mod enable mac ursp
    unsigned int bypass_pkt_type : 2; // dest port pkt type
    unsigned int td_high_pri_flag : 3;
    unsigned int ack_flag : 1;
    unsigned int td_net_id : 8;
    unsigned int td_fc_head : 4; // for pfa_tft, filter chain head.
    unsigned int td_pdu_ssid : 8; // for pfa_tft; pdu ssid(0-7bit)
    unsigned int td_modem_id : 2; // for pfa_tft; modem_id(16-17bit)

    /* word4 */
    unsigned int dl_rqi : 1;        /**< dl_rqi，下行NAS反射使能标志位：0：不使能；1：使能。*/
    unsigned int dl_rdi : 1;        /**< dl_rdi，下行AS反射使能标志位：0：不使能；1：使能。*/
    unsigned int refl_fc_head : 4;   /**< 下行反射到上行的对应的过滤器链头标识。*/
    unsigned int dl_qos_flow_id : 7; /**< 下行NAS反射使用的qos_flow_id[6:0] */
    unsigned int drb_id : 6;         /**< 下行BD中DRB ID承载号用于5G反射。*/
    unsigned int ul_flow_id : 7;         /**< 上行流标识号，协议栈新增。*/
    unsigned int td_irq_en : 1;  // for pfa; pfa td finish with&without interrupt.
    unsigned int td_push_en : 1; // for both; send packet now while set 1
    unsigned int td_mode : 1; // V200 td mode is stub!
    unsigned int v2x_ind : 1; // V200 v2x_ind is stub!
    unsigned int td_int_en : 1; // V200 is stub!, for pfa_tft
    unsigned int td_reserve1 : 1;

    unsigned int td_user_field2;      /**< 维测; ap ul_packet_id; cp pdcp_count */

    /* word6-7 */
    unsigned int td_user_field0;
    unsigned int td_user_field1;
};

// td result bit 0-1
enum pfa_td_result {
    TD_RESULT_BOTTOM = 1
};

// td result bit 2-5, see pfa_hal_desc_V200.h/V300.h
// td result bit 6-16
enum pfa_td_fw {
    TD_FW_BOTTOM = 1
};

// td result bit 17-19
enum pfa_td_pkt_type {
    TD_PKT_IPV4_TCP = 0,
    TD_PKT_IPV4_UDP,
    TD_PKT_IPV4_NON_UDP_TCP,
    TD_PKT_IPV6_TCP,
    TD_PKT_IPV6_UDP,
    TD_PKT_IPV6_NON_UDP_TCP,
    TD_PKT_NON_UDP_TCP,
    TD_PKT_TRANS_BEFORE_IP_CHECK,
    TD_PKT_TYPE_BOTTOM
};

// td result bit 20-21
enum pfa_td_warp {
    TD_WARP_SUCCESS,
    TD_WARP_PART_DISCARD,
    TD_WARP_ALL_DISCARD,
    TD_WARP_BOTTOM
};

enum pfa_td_ncm_unwrap {
    TD_NCM_WRAP_SUCCESS,
    TD_NCM_PORT_DISABLE,
    TD_NCM_TD_LENGTH_TOO_LONG,
    TD_NCM_NTH_FLAG_WRONG,
    TD_NCM_NTH_LENGTH_WRONG,
    TD_NCM_BLOCK_LENGTH_WRONG,
    TD_NCM_NDP_FLAG_WRONG,
    TD_NCM_NDP_LENGTH_WRONG,
    TD_NCM_DATAGRAM_WRONG,
    TD_NCM_ETH_LENGTH_WRONG,
    TD_NCM_TOTAL_DATA_LEN_TOO_LONG,
    TD_NCM_ETH_PKT_TO_LAGE,
    TD_NCM_UNWRAP_BOTTOM
};

// rd bit 1-5
enum pfa_rd_send_cpu_rsn {
    RD_CPU_PKT_ERR = 0, // DL OVER LARGE PKT, DL NON-IP PKT, DL SPECIAL
    RD_CPU_MAC_NON_STICK_V4_V6,
    RD_CPU_1STMAC_IPVER_ERR,
    RD_CPU_1STMAC_DMAC_SMAC_DIFF,
    RD_CPU_1STMAC_MAC_TRANS_FAIL,
    RD_CPU_2SNDMAC_BRG_DMAC_FAIL,
    RD_CPU_L3_CHECK_L3_PROTOCOL_FAIL,
    RD_CPU_L3_CHECK_IPV4_HDR_FAIL,
    RD_CPU_IPV6_HOP_LIMIT_1,
    RD_CPU_IPV4_HDR_LEN_ERR,
    RD_CPU_IPV4_ONE_TTL_LEFT,
    RD_CPU_IPV4_SLICE_PKT,
    RD_CPU_L4_PORT_MATCH,
    RD_CPU_IP_TABLE_MISMATCH,
    RD_CPU_STICK_ARP_PKT,
    RD_CPU_FWSUCC_OR_DISCARD,
    RD_CPU_ECM2USB_BYCPU,
    RD_CPU_IP_TYPE_CHECK_DOUBLE_VLAN,
    RD_CPU_IP_TYPE_CHECK_NOT_DOUBLE_VLAN_NOT_IP,
    RD_CPU_MAC_PDU_FAIL,
    RD_CPU_MAC_PDU_TBALE_NOT_DONE,
    RD_CPU_RESERVE0,
    RD_CPU_RESERVE1,
    RD_CPU_RESERVE2,
    RD_CPU_USB_TO_USB,
    RD_CPU_BOTTOM = 32
};
    //unsigned int pkt_version_err : 1;
    //unsigned int pkt_head_len_err : 1;
    //unsigned int pkt_bd_noreq : 1;
    //unsigned int pkt_nh_err : 1;
    //unsigned int pkt_no_tcp_udp : 1;
    //unsigned int pkt_ip_len_err : 1;
struct pfa_rd_desc {
    /* word0 packet input header, suppose to point at ip header */
    unsigned int rd_outaddr_lower;
    unsigned int rd_outaddr_upper;
    /* word2 */
    unsigned int pkt_dra_offset : 5;
    unsigned int dra_offset_positive : 1;
    unsigned int rd_pkt_len : 16;      // for pfa; packet len
    unsigned int rd_packet_ind : 2; // 1’b0 eth;  1‘b1 ip; other
    unsigned int rd_tocpu_res : 1; // no use, pkt sec use ch sec
    unsigned int rd_mode : 2;
    unsigned int rd_sport : 5;
    /* word3 */
    unsigned int rd_trans_result : 1;
    unsigned int rd_drop_rsn : 5;
    unsigned int rd_trans_path : 14;
    unsigned int rd_l2_hdr_offset : 1; //临时打桩！！！！！！！！！！
    unsigned int rd_pfa_tftres_stmid : 1; //临时打桩！！！！！！！！！！
    unsigned int rd_v2x_ind : 1; // this is stub
    unsigned int rd_pktnum : 6; // 复用下面的tft result 推动芯片修改！！！！！！！！！！！！！！！
    unsigned int rd_finish_warp_res : 3;

    /* word4 */
    unsigned int rd_high_pri_flag : 3; // for pfa_tft; high priority packet flag
    unsigned int rd_ack_flag : 1;
    unsigned int rd_net_id : 8;   // net id
    unsigned int rd_fc_head : 4; // for pfa_tft, filter chain head.
    unsigned int rd_pdu_ssid : 8; // for pfa_tft; pdu ssid(0-7bit)
    unsigned int rd_modem_id : 2; // for pfa_tft; modem_id(16-17bit)
    unsigned int drb_id : 6; /**< drb_id[7:0]上行RD上报的DRB ID承载号。8'hFF代表过滤器不匹配。*/
    /* word5 */
    unsigned int qfi : 7;
    unsigned int special_flag : 1;
    unsigned int rd_ptp_syn_ind : 1;
    unsigned int ul_flow_id : 7;         /**< 上行流标识号，协议栈新增。*/
    unsigned int rd_pkt_type : 3;
    unsigned int rd_pfa_tft_dl_err_flag : 1; // V200 reserve
    unsigned int version : 4;
    unsigned int protocol : 8;
    /* word6 */
    unsigned int eth_type : 16;
    unsigned int rd_vlan_flag : 1;
    unsigned int rd_vlan_pcp : 3;
    unsigned int rd_vlan_vid : 12;

    /* word7 */
    unsigned int tcp_udp_field0;

    /* word8 */
    unsigned int stream_id; // tcp_udp_field1
    /* word9 */
    unsigned int rd_user_field2; // dl:tft_pdcp_cnt, ul:ul_packet_id

    /* word10-11 */
    unsigned int rd_user_field0;
    unsigned int rd_user_field1;
};
enum pfa_rd_result {
    RD_RESULT_SUCCESS,
    RD_RESULT_UPDATA_ONLY,
    RD_RESULT_DISCARD,
    RD_RESULT_WRAP_OR_LENGTH_WRONG,
    RD_RESULT_BOTTOM
};


// rd bit 17-19
enum pfa_rd_pkt_ind {
    RD_PKT_IP = 0,
    RD_PKT_ETH,
    RD_PKT_OHTER
};

// rd bit 2-5
enum pfa_rd_pkt_drop_rsn {
    RD_DROP_RSN_UNDISCARD = 0,
    RD_DROP_RSN_TTL_ZERO = 22,
    RD_DROP_RSN_RD_POINT_NULL = 31,
    RD_DROP_RSN_BOTTOM = 32,
};

// rd bit 6-16
enum pfa_rd_fw {
    RD_MAC_FILTER,       // BIT6
    RD_ETH_TYPE,         // BIT7
    RD_1ST_MAC_FW,       // BIT8
    RD_IP_HDR_CHECK,     // BIT9
    RD_IP_FILTER,        // BIT10
    RD_TCP_UDP_CHECK,    // BIT11
    RD_HASH_CHECK,       // BIT12
    RD_HASH_CACHE_CHECK, // BIT13
    RD_HASH_DDR_CHECK,   // BIT14
    RD_2ND_MAC_CHECK,    // BIT15
    RD_TRANS_COMPLETE,   // BIT16
    RD_INDICATE_BOTTOM
};

// rd bit 17-19
enum pfa_rd_pkt_type {
    RD_PKT_IPV4_TCP = 0,
    RD_PKT_IPV4_UDP,
    RD_PKT_IPV4_NON_UDP_TCP,
    RD_PKT_IPV6_TCP,
    RD_PKT_IPV6_UDP,
    RD_PKT_IPV6_NON_UDP_TCP,
    RD_PKT_NON_UDP_TCP,
    RD_PKT_TRANS_BEFORE_IP_CHECK,
    RD_PKT_TYPE_BOTTOM
};

// rd bit 20-22
enum pfa_rd_finsh_wrap_rsn {
    RD_WRAP_WRONG_FORMAT,
    RD_WRAP_PUSH_EN,
    RD_WRAP_NUM_OVERSIZE,
    RD_WRAP_LENGTH_OVERSIZE,
    RD_WRAP_TIMEOUT,
    RD_WRAP_PORT_DIS,
    RD_WRAP_MUTLI_NCM,
    RD_WRAP_BOTTOM
};

/* struct of pfa_td_desc->stream_id_result as pfa result  */
struct pfa_td_result_s {
    unsigned int rsv;
};

struct pfa_ad_desc {
    unsigned long long ad_addr;  // lower 32bit for input packet phy or dra addr;
};

/* pfa_mac_entry read from sram */
struct pfa_mac_entry_in_sram {
    unsigned long long action : 1;
    unsigned long long rd_port : 5;
    unsigned long long bid : 5;
    unsigned long long vid : 12;
    unsigned long long vlan_pcp : 3;
    unsigned long long vlan_valid : 1;
    unsigned long long port : 5;
    unsigned long long timestamp : 16;
    unsigned long long userfield : 32;
    unsigned long long is_static : 1;
    unsigned long long reserve : 15;
    unsigned long long reserve1 : 32;
};

struct pfa_mac_entry_in_reg {
    char mac[ETH_ALEN];
    char mac_fil_vld : 1;
    char mac_fw_vld : 1;
    char reserve : 6;
    char reserve1;
};

struct pfa_mac_fw_entry {
    unsigned int mac_hi; /* mac in big endian */
    unsigned int mac_lo; /* mac in big endian */

    unsigned int vid : 12; /* vlan id in big endian */
    unsigned int reserve0 : 4;
    unsigned int port_br_id : 5; /* br id this entry belongs to */
    unsigned int reserve1 : 3;
    unsigned int port_no : 5; /* port no */
    unsigned int reserve2 : 3;

    unsigned int pcp : 3;
    unsigned int vlan_valid : 1;
    unsigned int timestamp : 12; /* mac fw timestamp,software no care. */
    unsigned int is_static : 1;  /* is static entry */
    unsigned int reserve3 : 15;
    unsigned int userfiled : 32;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __PFA_HAL_DESC_H__ */
