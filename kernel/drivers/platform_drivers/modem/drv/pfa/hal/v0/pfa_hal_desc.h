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

#include <linux/types.h>
#include <linux/if_ether.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* td result bit 2-5 */
enum pfa_td_ptk_drop_rsn {
    TD_DROP_RSN_TRANS_SUCC = 0,
    TD_DROP_RSN_UNWRAP_ERR,
    TD_DROP_RSN_LENGTH_ERR,
    TD_DROP_RSN_SPORT_DISABLE,
    TD_DROP_RSN_MAC_FILT,
    TD_DROP_RSN_RESERVE0,
    TD_DROP_RSN_TTL_ZERO,
    TD_DROP_RSN_IP_FILT,
    TD_DROP_RSN_UDP_RATE_LIMIT,
    TD_DROP_RSN_MAC_FW_ENTRY_ERR,
    TD_DROP_RSN_DPORT_DISABLE,
    TD_DROP_RSN_QOS_LIMIT,
    TD_DROP_RSN_RESERVE2,
    TD_DROP_RSN_V6_HOP_LIMIT_ZERO,
    TD_DROP_RSN_RD_PTR_NULL,
    TD_DROP_RSN_BOTTOM
};

struct pfa_td_desc {
    /* word0 */
    unsigned int td_int_en : 1;  // for pfa_tft: pfa_tft bd finish with/without interrupt
    unsigned int td_mode : 2;    // for pfa_tft, suppose to fix at 1; 0:copy and filter, 1:filter only, 2:copy only
    unsigned int td_cd_en : 1;
    unsigned int td_reserve0 : 3;
    unsigned int td_copy_en : 1;
    unsigned int td_fc_head : 4;  // for pfa_tft, filter chain head.
    unsigned int td_irq_en : 1;   // for pfa; pfa td finish with&without interrupt.
    unsigned int td_push_en : 1;  // for both; send packet now while set 1
    unsigned int td_reserve1 : 1;
    unsigned int td_high_pri_flag : 1;  // for pfa_tft; high priority packet flag
    unsigned int td_pkt_len : 16;       // for pfa; packet len

    /* word1-2 packet input header, suppose to point at ip header */
    unsigned int td_inaddr_lower;  // lower 32bit for input packet phy or dra addr;
    unsigned int td_inaddr_upper;  // lower 32bit for input packet phy or dra addr;

    /* word3 */
    unsigned int td_pdu_ssid : 8;  // for pfa_tft; pdu ssid(0-7bit)
    unsigned int tft_filter_ind : 1; // stub
    unsigned int td_reserve2 : 6;
    unsigned int v2x_ind : 1; // stub here, only sne is active
    unsigned int td_modem_id : 2;  // for pfa_tft; modem_id(16-17bit)
    unsigned int td_stick_en : 1;
    unsigned int pkt_ind : 1; // stub here, only sne is active
    unsigned int td_reserve3 : 9;
    unsigned int td_iptype : 3;  // for pfa_tft; iptype(29-31bit)

    /* word4 */
    unsigned int td_bypass_en : 1;
    unsigned int td_bypass_addr : 4;
    unsigned int td_host_ana : 1;      // phone mode packet analysis enable,
    unsigned int td_drop_ack_ind : 1;  // ACK TD effect 0:normal,1:drop pkt,
    unsigned int td_net_id : 8;    // LAN netcard id, set by CPU TD&USB TD,
    unsigned int td_with_mac_pdu_rst : 1; // pfa new add
    unsigned int bypass_pkt_type : 1; // stub
    unsigned int td_reserve4 : 15;     // reserve

    /* word5 */
    unsigned int td_result;  // stream id pfa reseult .function unknow;

    /* word6 */
    unsigned int td_info;

    /* word7 */
    unsigned int td_user_field0;

    /* word8-9 */
    unsigned int td_user_field1;
    unsigned int td_user_field2;
};

/* rd bit 23-27 */
enum pfa_rd_send_cpu_rsn {
    RD_CPU_PKT_ERR = 0,  // DL OVER LARGE PKT, DL NON-IP PKT, DL SPECIAL
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
    RD_CPU_BOTTOM
};

struct pfa_rd_desc {
    /* word0 */
    unsigned int rd_int_en : 1;  // for pfa_tft pfa: pfa_tft bd finish with/without interrupt
    unsigned int rd_mode : 2;    // for pfa_tft, suppose to fix at 1; 0:copy and filter, 1:filter only, 2:copy only
    unsigned int pkt_dra_offset : 1; // this is stub
    unsigned int rd_data_from_unsec : 1;
    unsigned int rd_reserve : 1;
    unsigned int rd_pfa_tft_dl_err_flag : 1;
    unsigned int rd_ack_flag : 1;
    unsigned int rd_fc_head : 4;  // for pfa_tft, filter chain head.
    unsigned int rd_v2x_ind : 1; // this is stub
    unsigned int rd_reserve1 : 2;
    unsigned int rd_high_pri_flag : 1;  // for pfa_tft; high priority packet flag
    unsigned int rd_pkt_len : 16;       // for pfa; packet len

    /* word1-2 */
    unsigned int rd_outaddr_lower;
    unsigned int rd_outaddr_upper;

    /* word3 */
    unsigned int rd_pdu_ssid : 8;  // for pfa_tft; pdu ssid(0-7bit)
    unsigned int rd_net_id : 8;    // for pfa_tft; mutli-NCM ips id; function know
    unsigned int rd_modem_id : 2;  // for pfa_tft; modem_id(16-17bit)
    unsigned int rd_offset_bd : 5;
    unsigned int rd_offset_bd_drct : 1;
    unsigned int rd_l2_hdr_offset : 5;  // offset of mac header to ip header
    unsigned int rd_iptype : 3;         // for pfa_tft; iptype(29-31bit)

    /* word4 pfa transfer result */
    unsigned int rd_trans_result : 2;
    unsigned int rd_drop_rsn : 4;
    unsigned int rd_trans_path : 10;
    unsigned int rd_trans_path_finish : 1;
    unsigned int rd_pkt_type : 3;
    unsigned int rd_finish_warp_res : 3;
    unsigned int rd_tocpu_res : 5;
    unsigned int rd_bd_cd_len_not_same : 1;
    unsigned int rd_reserve4 : 2;
    unsigned int rd_updata_only : 1;

    /* word5 */
    unsigned int rd_sport : 5;
    unsigned int rd_dport : 5;
    unsigned int rd_pktnum : 6;
    unsigned int rd_ethtype : 16;

    /* word6 */
    unsigned int rd_pfa_tftres_stmid;  // pfa_tft result, stream id

    /* word7 */
    unsigned int rd_user_field0;

    /* word8-9 */
    unsigned int rd_user_field1;
    unsigned int rd_user_field2;
};

#define PFA_DESC_IP_PKT_WORD_NUM 16
#define PFA_DESC_CD_NUM_MAX 1024

// td result bit 0-1
enum pfa_td_result {
    TD_RESULT_UPDATA_ONLY = 0,
    TD_RESULT_DISCARD,
    TD_RESULT_NORMAL,
    TD_RESULT_WRAP_OR_LENTH_WRONG,
    TD_RESULT_BOTTOM
};

/* td result bit 6-16 */
enum pfa_td_fw {
    TD_FW_MAC_FILTER,     // BIT6
    TD_FW_ETH_TYPE,       // BIT7
    TD_FW_1ST_MACFW,      // BIT8
    TD_FW_IP_HDR_CHECK,   // BIT9
    TD_FW_IP_FILTER,      // BIT10
    TD_FW_TCP_UDP_CHECK,  // BIT11
    TD_FW_HASH_CALC,      // BIT12
    TD_FW_HASH_CACHE,     // BIT13
    TD_FW_HASH_DDR,       // BIT14
    TD_FW_2ND_MACFW,      // BIT15
    TD_FW_COMPLETE,       // BIT16
    TD_FW_BOTTOM
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

// td result bit 22-25
enum pfa_td_rndis_unwrap {
    TD_RNDIS_WRAP_SUCCESS,
    TD_RNDIS_PORT_DISABLE,
    TD_RNDIS_TD_LENGTH_TOO_LONG,
    TD_RNDIS_FIRST_MSG_TYPE,
    TD_RNDIS_MSG_LENGTH_OVERSIZE,
    TD_RNDIS_ETH_OVER_PKT_LEN,
    TD_RNDIS_ETH_LENGTH_WRONG,
    TD_RNDIS_WRAP_BOTTOM
};

// td result bit 22-25
enum pfa_td_normal_result {
    TD_FINISH_SUCCESS,
    TD_PORT_DISABLE,
    TD_PKT_LEN_ABNORMITY,
    TD_NORMAL_BOTTOM
};

/* struct of pfa_td_desc->stream_id_result as pfa result  */
struct pfa_td_result_s {
    unsigned int td_trans_result : 2;
    unsigned int td_drop_rsn : 4;
    unsigned int td_trans_path : 11;
    unsigned int td_pkt_type : 3;     // message type
    unsigned int td_warp : 2;         // drop ncm like packet reason
    unsigned int td_unwrap : 4;       // packet analytic error type
    unsigned int td_pkt_ext_cnt : 6;  // ncm and other packet extract count
};

/* struct of pfa_td_desc->stream_id_result as pfa result  */
struct pfa_td_ipsec_info_s {
    unsigned int dport_num : 4;
    unsigned int update_pkt : 1;
    unsigned int reserve : 27;
};

struct pfa_ext_td_desc {
    struct pfa_td_desc std_td_desc;
    unsigned int td_ip_pkt_hdr[PFA_DESC_IP_PKT_WORD_NUM];
};

// rd bit 0-1
enum pfa_rd_result {
    RD_RESULT_UPDATA_ONLY = 0,
    RD_RESULT_DISCARD,
    RD_RESULT_SUCCESS,
    RD_RESULT_WRAP_OR_LENGTH_WRONG,
    RD_RESULT_BOTTOM
};

// rd bit 2-5
enum pfa_rd_pkt_drop_rsn {
    RD_DROP_RSN_UNDISCARD = 0,
    RD_DROP_RSN_TTL_ZERO = 6,
    RD_DROP_RSN_RD_POINT_NULL = 15,
    RD_DROP_RSN_BOTTOM
};

// rd bit 6-16
enum pfa_rd_fw {
    RD_MAC_FILTER,        // BIT6
    RD_ETH_TYPE,          // BIT7
    RD_1ST_MAC_FW,        // BIT8
    RD_IP_HDR_CHECK,      // BIT9
    RD_IP_FILTER,         // BIT10
    RD_TCP_UDP_CHECK,     // BIT11
    RD_HASH_CHECK,        // BIT12
    RD_HASH_CACHE_CHECK,  // BIT13
    RD_HASH_DDR_CHECK,    // BIT14
    RD_2ND_MAC_CHECK,     // BIT15
    RD_TRANS_COMPLETE,    // BIT16
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

/* struct of pfa_td_desc->rd_stemid_info as rd info  */
struct pfa_rd_ipsec_info_s {
    unsigned int sport_num : 4;
    unsigned int dport_num : 4;
    unsigned int usb_sg_cnt : 8;
    unsigned int eth_pkt_type : 16;
};

/* struct of pfa_rd_desc->rd_result as pfa transfer result  */
struct pfa_rd_result_s {
    unsigned int rd_trans_result : 2;
    unsigned int rd_drop_rsn : 4;
    unsigned int rd_trans_path : 11;
    unsigned int rd_mess_indicat : 3;  // message type
    unsigned int rd_sg_comp_rsn : 3;
    unsigned int rd_tocpu_rsn : 4;
    unsigned int rd_reserve : 4;      // packet analytic error type
    unsigned int rd_updata_only : 1;  // updata only packet flag
};

struct pfa_ext_rd_desc {
    struct pfa_rd_desc std_rd_desc;
    unsigned int rd_ip_pkt_hdr[PFA_DESC_IP_PKT_WORD_NUM];
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
    unsigned long long port : 5;
    unsigned long long timestamp : 16;
    unsigned long long userfiled : 8;
    unsigned long long is_static : 1;
    unsigned long long reserve : 11;
    char mac[ETH_ALEN];
    char mac_fil_vld : 1;
    char mac_fw_vld : 1;
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

    unsigned int pcp : 3; // stub
    unsigned int vlan_valid : 1; // stub
    unsigned int timestamp : 12; /* mac fw timestamp,software no care. */
    unsigned int is_static : 1;  /* is static entry */
    unsigned int reserve3 : 7;
    unsigned int userfiled : 8;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __PFA_HAL_DESC_H__ */
