/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应的帧的结构及操作接口定义的源文件(HAL模块不可以调用)
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef MAC_FRAME_80211_H
#define MAC_FRAME_80211_H

/* 1 其他头文件包含 */
#include "wlan_types.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "securec.h"
#include "securectype.h"
#include "mac_element.h"
#include "mac_frame_macro.h"
#include "mac_6ghz_frame_common.h"

/* 此文件中定义的结构体与协议相关，需要1字节对齐 */
/* 用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack(1)
/* 此文件中结构体与协议一致，要求1字节对齐，统一加DECLARE_PACKED */
struct mac_ether_header {
    uint8_t auc_ether_dhost[ETHER_ADDR_LEN];
    uint8_t auc_ether_shost[ETHER_ADDR_LEN];
    uint16_t us_ether_type;
} DECLARE_PACKED;
typedef struct mac_ether_header mac_ether_header_stru;

struct mac_llc_snap {
    uint8_t uc_llc_dsap;
    uint8_t uc_llc_ssap;
    uint8_t uc_control;
    uint8_t auc_org_code[NUM_3_BYTES];
    uint16_t us_ether_type;
} DECLARE_PACKED;
typedef struct mac_llc_snap mac_llc_snap_stru;

/* eapol帧头 */
struct mac_eapol_header {
    uint8_t uc_version;
    uint8_t uc_type;
    uint16_t us_length;
} DECLARE_PACKED;
typedef struct mac_eapol_header mac_eapol_header_stru;

/* IEEE 802.11, 8.5.2 EAPOL-Key frames */
/* EAPOL KEY 结构定义 */
struct mac_eapol_key {
    uint8_t uc_type;
    /* Note: key_info, key_length, and key_data_length are unaligned */
    uint8_t auc_key_info[NUM_2_BYTES];   /* big endian */
    uint8_t auc_key_length[NUM_2_BYTES]; /* big endian */
    uint8_t auc_replay_counter[WPA_REPLAY_COUNTER_LEN];
    uint8_t auc_key_nonce[WPA_NONCE_LEN];
    uint8_t auc_key_iv[NUM_16_BYTES];
    uint8_t auc_key_rsc[WPA_KEY_RSC_LEN];
    uint8_t auc_key_id[NUM_8_BYTES]; /* Reserved in IEEE 802.11i/RSN */
    uint8_t auc_key_mic[NUM_16_BYTES];
    uint8_t auc_key_data_length[NUM_2_BYTES]; /* big endian */
    /* followed by key_data_length bytes of key_data */
} DECLARE_PACKED;
typedef struct mac_eapol_key mac_eapol_key_stru;

/* DC header for higame 2.0 */
struct mac_higame_dc_header {
        uint16_t seq;  /* 用于higame 2.0的重复帧过滤 */
        uint16_t type; /* 承载詄ther hdr中的type */
} DECLARE_PACKED;
typedef struct mac_higame_dc_header mac_higame_dc_header_stru;

/*
 * Structure of the IP frame
 */
struct mac_ip_header {
    uint8_t ip_header_len : 4,
            version : 4;
    uint8_t uc_tos;
    uint16_t us_tot_len;
    uint16_t us_id;
    uint16_t us_frag_off;
    uint8_t uc_ttl;
    uint8_t uc_protocol;
    uint16_t us_check;
    uint32_t saddr;
    uint32_t daddr;
    /* The options start here. */
} DECLARE_PACKED;
typedef struct mac_ip_header mac_ip_header_stru;

struct mac_tcp_header {
    uint16_t us_sport;
    uint16_t us_dport;
    uint32_t seqnum;
    uint32_t acknum;
    uint8_t uc_offset;
    uint8_t uc_flags;
    uint16_t us_window;
    uint16_t us_check;
    uint16_t us_urgent;
} DECLARE_PACKED;
typedef struct mac_tcp_header mac_tcp_header_stru;

/* UDP头部结构 */
typedef struct {
    uint16_t us_src_port;
    uint16_t us_des_port;
    uint16_t us_udp_len;
    uint16_t us_check_sum;
} udp_hdr_stru;

/* frame control字段结构体 */
struct mac_header_frame_control {
    uint16_t bit_protocol_version : 2, /* 协议版本 */
               bit_type : 2,                    /* 帧类型 */
               bit_sub_type : 4,                /* 子类型 */
               bit_to_ds : 1,                   /* 发送DS */
               bit_from_ds : 1,                 /* 来自DS */
               bit_more_frag : 1,               /* 分段标识 */
               bit_retry : 1,                   /* 重传帧 */
               bit_power_mgmt : 1,              /* 节能管理 */
               bit_more_data : 1,               /* 更多数据标识 */
               bit_protected_frame : 1,         /* 加密标识 */
               bit_order : 1;                   /* 次序位 */
} DECLARE_PACKED;
typedef struct mac_header_frame_control mac_header_frame_control_stru;

/* 基础802.11帧结构 */
struct mac_ieee80211_frame {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15,
               bit_duration_flag  : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4,
             bit_seq_num : 12;
} DECLARE_PACKED;
typedef struct mac_ieee80211_frame mac_ieee80211_frame_stru;

/* 序列控制字段 */
struct mac_seq_ctrl {
    uint16_t frag_num : 4,
             seq_num : 12;
} DECLARE_PACKED;
typedef struct mac_seq_ctrl mac_seq_ctrl_stru;

/* ps poll帧结构 */
struct mac_ieee80211_pspoll_frame {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_aid_value : 14, /* ps poll 下的AID字段 */
               bit_aid_flag1 : 1,
               bit_aid_flag2 : 1;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t auc_trans_addr[WLAN_MAC_ADDR_LEN];
} DECLARE_PACKED;
typedef struct mac_ieee80211_pspoll_frame mac_ieee80211_pspoll_frame_stru;

/* qos帧结构 */
struct mac_ieee80211_qos_frame {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15, /* duration/id */
               bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4, /* sequence control */
               bit_seq_num : 12;
    uint8_t bit_qc_tid : 4,
              bit_qc_eosp : 1,
              bit_qc_ack_polocy : 2,
              bit_qc_amsdu : 1;
    union {
        uint8_t bit_qc_txop_limit;
        uint8_t bit_qc_ps_buf_state_resv : 1,
                  bit_qc_ps_buf_state_inducated : 1,
                  bit_qc_hi_priority_buf_ac : 2,
                  bit_qc_qosap_buf_load : 4;
    } qos_control;
} DECLARE_PACKED;
typedef struct mac_ieee80211_qos_frame mac_ieee80211_qos_frame_stru;

/* qos+HTC 帧结构 */
struct mac_ieee80211_qos_htc_frame {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15, /* duration/id */
               bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4, /* sequence control */
               bit_seq_num : 12;
    uint8_t bit_qc_tid : 4,
              bit_qc_eosp : 1,
              bit_qc_ack_polocy : 2,
              bit_qc_amsdu : 1;
    union {
        uint8_t bit_qc_txop_limit;
        uint8_t bit_qc_ps_buf_state_resv : 1,
                  bit_qc_ps_buf_state_inducated : 1,
                  bit_qc_hi_priority_buf_ac : 2,
                  bit_qc_qosap_buf_load : 4;
    } qos_control;

    uint32_t htc;
} DECLARE_PACKED;
typedef struct mac_ieee80211_qos_htc_frame mac_ieee80211_qos_htc_frame_stru;

/* 四地址帧结构体 */
struct mac_ieee80211_frame_addr4 {
    mac_header_frame_control_stru st_frame_control;
    uint16_t
    bit_duration_value : 15,
                       bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4,
               bit_seq_num : 12;
    uint8_t auc_address4[WLAN_MAC_ADDR_LEN];
} DECLARE_PACKED;
typedef struct mac_ieee80211_frame_addr4 mac_ieee80211_frame_addr4_stru;

/* qos四地址帧结构 */
struct mac_ieee80211_qos_frame_addr4 {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15, /* duration/id */
               bit_duration_flag : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4, /* sequence control */
               bit_seq_num : 12;
    uint8_t auc_address4[WLAN_MAC_ADDR_LEN];
    uint8_t bit_qc_tid : 4,
              bit_qc_eosp : 1,
              bit_qc_ack_polocy : 2,
              bit_qc_amsdu : 1;
    union {
        uint8_t qc_txop_limit;                /* txop limit字段 */
        uint8_t qc_queue_size;                /* queue size字段 */
        uint8_t bit_qc_ps_buf_state_resv : 1, /* AP PS Buffer State */
                  bit_qc_ps_buf_state_inducated : 1,
                  bit_qc_hi_priority_buf_ac : 2,
                  bit_qc_qosap_buf_load : 4;
    } qos_control;
} DECLARE_PACKED;
typedef struct mac_ieee80211_qos_frame_addr4 mac_ieee80211_qos_frame_addr4_stru;

/* qos htc 四地址帧结构 */
struct mac_ieee80211_qos_htc_frame_addr4 {
    mac_header_frame_control_stru st_frame_control;
    uint16_t bit_duration_value : 15, /* duration/id */
             bit_duration_flag  : 1;
    uint8_t auc_address1[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address2[WLAN_MAC_ADDR_LEN];
    uint8_t auc_address3[WLAN_MAC_ADDR_LEN];
    uint16_t bit_frag_num : 4, /* sequence control */
             bit_seq_num  : 12;
    uint8_t auc_address4[WLAN_MAC_ADDR_LEN];
    uint8_t bit_qc_tid : 4,
            bit_qc_eosp : 1,
            bit_qc_ack_polocy : 2,
            bit_qc_amsdu : 1;
    union {
        uint8_t qc_txop_limit;                /* txop limit字段 */
        uint8_t qc_queue_size;                /* queue size字段 */
        uint8_t bit_qc_ps_buf_state_resv : 1, /* AP PS Buffer State */
                  bit_qc_ps_buf_state_inducated : 1,
                  bit_qc_hi_priority_buf_ac : 2,
                  bit_qc_qosap_buf_load : 4;
    } qos_control;

    uint32_t htc;
} DECLARE_PACKED;
typedef struct mac_ieee80211_qos_htc_frame_addr4 mac_ieee80211_qos_htc_frame_addr4_stru;

struct supported_channel {
    uint8_t fist_channel_number;
    uint8_t number_of_channels;
};
/* 用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack()

#endif
