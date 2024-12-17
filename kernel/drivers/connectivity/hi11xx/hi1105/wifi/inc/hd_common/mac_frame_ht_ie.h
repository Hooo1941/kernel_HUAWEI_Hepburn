/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应的帧的结构及操作接口定义的源文件(HAL模块不可以调用)
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef MAC_FRAME_HT_IE_H
#define MAC_FRAME_HT_IE_H

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
struct mac_11ntxbf_info {
    uint8_t bit_11ntxbf : 1, /* 11n txbf  能力 */
              bit_reserve : 7;
    uint8_t auc_reserve[NUM_3_BYTES];
};
typedef struct mac_11ntxbf_info mac_11ntxbf_info_stru;

struct mac_11ntxbf_vendor_ie {
    uint8_t uc_id;  /* element ID */
    uint8_t uc_len; /* length in bytes */
    uint8_t auc_oui[NUM_3_BYTES];
    uint8_t uc_ouitype;
    mac_11ntxbf_info_stru st_11ntxbf;
};
typedef struct mac_11ntxbf_vendor_ie mac_11ntxbf_vendor_ie_stru;

#ifdef _PRE_WLAN_FEATURE_PRIV_CLOSED_LOOP_TPC
/* 私有对通功率调整数据结构 */
struct mac_hisi_adjust_pow_ie {
    uint8_t uc_id;  /* element ID */
    uint8_t uc_len; /* length in bytes */
    uint8_t auc_oui[NUM_3_BYTES];
    uint8_t uc_ouitype;
    uint8_t uc_adjust_pow;
};
typedef struct mac_hisi_adjust_pow_ie mac_hisi_adjust_pow_ie_stru;
#endif

/* MAC_HISI_CAP_IE信息元素 能力字段结构体 */
struct mac_hisi_cap_vendor_ie {
    uint32_t bit_11ax_support      : 1,  /* 1:是否支持11AX bit0 */
             bit_dcm_support       : 1,  /* 1:是否支持dcm bit1 */
             bit_p2p_csa_support   : 1,  /* 1:是否支持p2p_csa bit2 */
             bit_p2p_scenes        : 1,  /* 1:是否支持p2p_scenes bit3 */
             bit_1024qam_cap       : 1,  /* 1:是否支持1024QAM bit4 */
             bit_4096qam_cap       : 1,  /* 1:是否支持4096QAM bit5 */
             bit_sgi_400ns_cap     : 1,  /* 1:是否支持SGI_400NS bit6 */
             bit_vht_3nss_80m_mcs6 : 1,  /* 1:是否支持VHT_3NSS_80M_MCS6 bit7 */
             bit_he_ltf_repeat     : 1,  /* 1:是否支持HE LTF bit8 */
             bit_sub_wf_cap        : 1,  /* 1:是否支持sub_wf_cap bit9 */
             bit_resv              : 22; /* 22:预留扩展字段 bit10~bit31 */
} DECLARE_PACKED;
typedef struct mac_hisi_cap_vendor_ie mac_hisi_cap_vendor_ie_stru;

/* MAC_HISI_CAP_IE 数据结构，私有IE收编，包括：1024QAM、4096QAM、SGI_400NS、VHT_3NSS_80M_MCS6 */
struct mac_hisi_cap_ie {
    mac_ieee80211_vendor_ie_stru ieee80211_vendor_ie;
    mac_hisi_cap_vendor_ie_stru vendor_ie_cap;
} DECLARE_PACKED;
typedef struct mac_hisi_cap_ie mac_hisi_cap_ie_stru;

#ifdef _PRE_WLAN_FEATURE_MBO
struct mac_assoc_retry_delay_attr_mbo_ie {
    uint8_t uc_element_id;
    uint8_t uc_len;
    uint8_t auc_oui[NUM_3_BYTES];
    uint8_t uc_oui_type;
    uint8_t uc_attribute_id;
    uint8_t uc_attribute_len;
    uint32_t re_assoc_delay_time;
} DECLARE_PACKED;
typedef struct mac_assoc_retry_delay_attr_mbo_ie mac_assoc_retry_delay_attr_mbo_ie_stru;
#endif

/* 建立BA会话时，BA参数域结构定义 */
struct mac_ba_parameterset {
#if (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN) /* BIG_ENDIAN */
    uint16_t bit_buffersize : 10,          /* B6-15  buffer size */
               bit_tid : 4,                         /* B2-5   TID */
               bit_bapolicy : 1,                    /* B1   block ack policy */
               bit_amsdusupported : 1;              /* B0   amsdu supported */
#else
    uint16_t bit_amsdusupported : 1, /* B0   amsdu supported */
               bit_bapolicy : 1,              /* B1   block ack policy */
               bit_tid : 4,                   /* B2-5   TID */
               bit_buffersize : 10;           /* B6-15  buffer size */
#endif
} DECLARE_PACKED;
typedef struct mac_ba_parameterset mac_ba_parameterset_stru;

/* BA会话过程中的序列号参数域定义 */
struct mac_ba_seqctrl {
#if (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN) /* BIG_ENDIAN */
    uint16_t bit_startseqnum : 12,         /* B4-15  starting sequence number */
               bit_fragnum : 4;                     /* B0-3  fragment number */
#else
    uint16_t bit_fragnum : 4,        /* B0-3  fragment number */
               bit_startseqnum : 12;          /* B4-15  starting sequence number */
#endif
} DECLARE_PACKED;
typedef struct mac_ba_seqctrl mac_ba_seqctrl_stru;

/* Quiet信息元素结构体 */
struct mac_quiet_ie {
    uint8_t quiet_count;
    uint8_t quiet_period;
    uint16_t quiet_duration;
    uint16_t quiet_offset;
} DECLARE_PACKED;
typedef struct mac_quiet_ie mac_quiet_ie_stru;

/* erp 信息元素结构体 */
struct mac_erp_params {
    uint8_t bit_non_erp : 1,
              bit_use_protection : 1,
              bit_preamble_mode : 1,
              bit_resv : 5;
} DECLARE_PACKED;
typedef struct mac_erp_params mac_erp_params_stru;

/* rsn信息元素 rsn能力字段结构体 */
struct mac_rsn_cap {
    uint16_t bit_pre_auth : 1,
               bit_no_pairwise : 1,
               bit_ptska_relay_counter : 2,
               bit_gtska_relay_counter : 2,
               bit_mfpr : 1,
               bit_mfpc : 1,
               bit_rsv0 : 1,
               bit_peer_key : 1,
               bit_spp_amsdu_capable : 1,
               bit_spp_amsdu_required : 1,
               bit_pbac : 1,
               bit_ext_key_id : 1,
               bit_rsv1 : 2;
} DECLARE_PACKED;
typedef struct mac_rsn_cap mac_rsn_cap_stru;

/* obss扫描ie obss扫描参数结构体 */
struct mac_obss_scan_params {
    uint16_t us_passive_dwell;
    uint16_t us_active_dwell;
    uint16_t us_scan_interval;
    uint16_t us_passive_total_per_chan;
    uint16_t us_active_total_per_chan;
    uint16_t us_transition_delay_factor;
    uint16_t us_scan_activity_thresh;
} DECLARE_PACKED;
typedef struct mac_obss_scan_params mac_obss_scan_params_stru;

/* 扩展能力信息元素结构体定义 */
struct mac_ext_cap_ie {
    uint8_t bit_2040_coexistence_mgmt : 1,
              bit_resv1 : 1,
              bit_ext_chan_switch : 1,
              bit_resv2 : 1,
              bit_psmp : 1,
              bit_resv3 : 1,
              bit_s_psmp : 1,
              bit_event : 1;
    uint8_t bit_resv4 : 4,
              bit_proxyarp : 1,
              bit_resv13 : 3; /* bit13~bit15 */
    uint8_t bit_resv5 : 3,
              bit_bss_transition : 1, /* bit19 */
              bit_resv14 : 2,         /* bit20~bit21 */
              bit_multi_bssid : 1,    /* bit22 Multi bssid */
              bit_resv15 : 1;         /* bit23 */
    uint8_t bit_resv6 : 7,
              bit_interworking : 1;
    uint8_t bit_resv7 : 5,
              bit_tdls_prhibited : 1,
              bit_tdls_channel_switch_prhibited : 1,
              bit_resv8 : 1;

    uint8_t bit_resv9 : 8;
    uint8_t bit_resv10 : 8;

    uint8_t bit_resv11 : 6,
              bit_operating_mode_notification : 1, /* 11ac Operating Mode Notification特性标志 */
              bit_resv12 : 1;
} DECLARE_PACKED;
typedef struct mac_ext_cap_ie mac_ext_cap_ie_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
struct mac_ext_cap_ftm_ie {
    uint8_t bit_2040_coexistence_mgmt : 1,
              bit_resv1 : 1,
              bit_ext_chan_switch : 1,
              bit_resv2 : 1,
              bit_psmp : 1,
              bit_resv3 : 1,
              bit_s_psmp : 1,
              bit_event : 1;
    uint8_t bit_resv4 : 4,
              bit_proxyarp : 1,
              bit_resv13 : 3; /* bit13~bit15 */
    uint8_t bit_resv5 : 3,
              bit_bss_transition : 1, /* bit19 */
              bit_resv14 : 4;         /* bit20~bit23 */
    uint8_t bit_resv6 : 7,
              bit_interworking : 1;
    uint8_t bit_resv7 : 5,
              bit_tdls_prhibited : 1,
              bit_tdls_channel_switch_prhibited : 1,
              bit_resv8 : 1;

    uint8_t bit_resv9 : 8;
    uint8_t bit_resv10 : 8;

    uint8_t bit_resv11 : 6,
              bit_operating_mode_notification : 1, /* 11ac Operating Mode Notification特性标志 */
              bit_resv12 : 1;

    /* 商用网卡（Tplink6200）关联不上APUT */
    uint8_t bit_resv15 : 6,
              bit_ftm_responder : 1, /* bit70 Fine Timing Measurement Responder */
              bit_ftm_initiator : 1; /* bit71 Fine Timing Measurement Initiator */
} DECLARE_PACKED;
typedef struct mac_ext_cap_ftm_ie mac_ext_cap_ftm_ie_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_TWT
struct mac_ext_cap_twt_ie {
    uint8_t bit_2040_coexistence_mgmt : 1,
              bit_resv1 : 1,
              bit_ext_chan_switch : 1,
              bit_resv2 : 1,
              bit_psmp : 1,
              bit_resv3 : 1,
              bit_s_psmp : 1,
              bit_event : 1;
    uint8_t bit_resv4 : 4,
              bit_proxyarp : 1,
              bit_resv13 : 3; /* bit13~bit15 */
    uint8_t bit_resv5 : 3,
              bit_bss_transition : 1, /* bit19 */
              bit_resv14 : 4;         /* bit20~bit23 */
    uint8_t bit_resv6 : 7,
              bit_interworking : 1;
    uint8_t bit_resv7 : 5,
              bit_tdls_prhibited : 1,
              bit_tdls_channel_switch_prhibited : 1,
              bit_resv8 : 1;

    uint8_t bit_resv9 : 8;
    uint8_t bit_resv10 : 8;

    uint8_t bit_resv11 : 6,
              bit_operating_mode_notification : 1, /* 11ac Operating Mode Notification特性标志 */
              bit_resv12 : 1;

    /* 商用网卡（Tplink6200）关联不上APUT */
    uint8_t bit_resv15 : 6,
              bit_ftm_responder : 1, /* bit70 Fine Timing Measurement Responder */
              bit_ftm_initiator : 1; /* bit71 Fine Timing Measurement Initiator */

    uint8_t bit_resv16 : 5,
              bit_twt_requester_support : 1, /* 11ax TWT requester 特性标志 */
              bit_resv17 : 2;
} DECLARE_PACKED;
typedef struct mac_ext_cap_twt_ie mac_ext_cap_twt_ie_stru;

struct mac_twt_request_type {
    uint16_t bit_request : 1;
    uint16_t bit_setup_command : 3;
    uint16_t bit_trigger : 1;
    uint16_t bit_implicit : 1;
    uint16_t bit_flow_type : 1;
    uint16_t bit_flow_id : 3;
    uint16_t bit_exponent : 5;
    uint16_t bit_protection : 1;
} DECLARE_PACKED;
typedef struct mac_twt_request_type mac_twt_request_type_stru;
struct mac_btwt_request_type {
    uint16_t request : 1;
    uint16_t setup_command : 3;
    uint16_t trigger : 1;
    uint16_t implicit : 1;
    uint16_t flow_type : 1;
    uint16_t flow_id : 3;
    uint16_t exponent : 5;
    uint16_t protection : 1;
} DECLARE_PACKED;
typedef struct mac_btwt_request_type mac_btwt_request_type_stru;

struct mac_twt_control_field {
    uint8_t bit_ndp_paging_indicator : 1;
    uint8_t bit_responder_pm_mode : 1;
    uint8_t bit_negotiation : 2;
    uint8_t bit_twt_information_frame_disabled : 1;
    uint8_t bit_wake_duration_unit : 1;
    uint8_t reserve : 2;
} DECLARE_PACKED;
typedef struct mac_twt_control_field mac_twt_control_field_stru;

struct mac_twt_element_ie_individual {
    uint8_t category;
    uint8_t action;
    uint8_t dialog_token;
    uint8_t element_id;
    uint8_t len;
    mac_twt_control_field_stru st_control;
    mac_twt_request_type_stru st_request_type;
    uint64_t twt;
    uint8_t min_duration;
    uint16_t mantissa;  // 可能对不齐
    uint8_t channel;
} DECLARE_PACKED;
typedef struct mac_twt_element_ie_individual mac_itwt_ie_stru;

struct mac_btwt_info {
    uint8_t bit_resv : 3;
    uint8_t bit_btwt_id : 5;
    uint8_t btwt_persistence;
} DECLARE_PACKED;
typedef struct mac_btwt_info mac_btwt_info_stru;

struct mac_twt_element_ie_broadcast {
    uint8_t element_id;
    uint8_t ie_len;
    mac_twt_control_field_stru control; /* 1字节 */
    mac_btwt_request_type_stru request_type; /* 2字节 */
    uint16_t twt;
    uint8_t min_duration;
    uint16_t mantissa;
    mac_btwt_info_stru btwt_info;
} DECLARE_PACKED;
typedef struct mac_twt_element_ie_broadcast mac_btwt_ie_stru;

struct mac_btwt_action {
    uint8_t category; /* ACTION的类别 */
    uint8_t action;   /* 不同ACTION类别下的分类 */
    uint8_t dialog_token;
    mac_btwt_ie_stru btwt_ie;
} DECLARE_PACKED;
typedef struct mac_btwt_action mac_btwt_action_stru;

struct mac_twt_teardown {
    uint8_t uc_category;
    uint8_t uc_action;
    uint8_t bit_twt_flow_id : 5,
              bit_nego_type : 2,
              bit_resv2 : 1;
} DECLARE_PACKED;
typedef struct mac_twt_teardown mac_twt_teardown_stru;

#ifdef _PRE_WLAN_FEATURE_TWT
struct mac_twt_information_field {
    uint8_t bit_twt_flow_id : 3;
    uint8_t bit_response_requested : 1;
    uint8_t bit_next_twt_request : 1;
    uint8_t bit_next_twt_subfield_size : 2;
    uint8_t bit_all_twt : 1;
} DECLARE_PACKED;
typedef struct mac_twt_information_field mac_twt_information_field_stru;

struct mac_twt_information_frame {
    uint8_t uc_category;
    uint8_t uc_action;
    mac_twt_information_field_stru st_twt_information_filed;
} DECLARE_PACKED;
typedef struct mac_twt_information_frame mac_twt_information_frame_stru;
#endif
#endif

/* qos info字段结构体定义 */
struct mac_qos_info {
    uint8_t bit_params_count : 4,
              bit_resv : 3,
              bit_uapsd : 1;
} DECLARE_PACKED;
typedef struct mac_qos_info mac_qos_info_stru;

/* wmm信息元素 ac参数结构体 */
typedef struct {
    uint8_t bit_aifsn : 4,
              bit_acm : 1,
              bit_aci : 2,
              bit_resv : 1;
    uint8_t bit_ecwmin : 4,
              bit_ecwmax : 4;
    uint16_t us_txop;
} mac_wmm_ac_params_stru;

/* BSS load信息元素结构体 */
struct mac_bss_load {
    uint16_t us_sta_count;       /* 关联的sta个数 */
    uint8_t uc_chan_utilization; /* 信道利用率 */
    uint16_t us_aac;
}DECLARE_PACKED;
typedef struct mac_bss_load mac_bss_load_stru;

/* country信息元素 管制域字段 */
struct mac_country_reg_field {
    uint8_t uc_first_channel; /* 第一个信道号 */
    uint8_t uc_channel_num;   /* 信道个数 */
    uint16_t us_max_tx_pwr;   /* 最大传输功率，dBm */
} DECLARE_PACKED;
typedef struct mac_country_reg_field mac_country_reg_field_stru;

/* ht capabilities信息元素支持的ampdu parameters字段结构体定义 */
struct mac_ampdu_params {
    uint8_t bit_max_ampdu_len_exponent : 2,
              bit_min_mpdu_start_spacing : 3,
              bit_resv : 3;
} DECLARE_PACKED;
typedef struct mac_ampdu_params mac_ampdu_params_stru;

/* ht cap信息元素 支持的mcs集字段 结构体定义 */
struct mac_sup_mcs_set {
    uint8_t auc_rx_mcs[NUM_10_BYTES];
    uint16_t bit_rx_highest_rate : 10,
               bit_resv1 : 6;
    uint32_t bit_tx_mcs_set_def : 1,
               bit_tx_rx_not_equal : 1,
               bit_tx_max_stream : 2,
               bit_tx_unequal_modu : 1,
               bit_resv2 : 27;
} DECLARE_PACKED;
typedef struct mac_sup_mcs_set mac_sup_mcs_set_stru;

/* vht信息元素，支持的mcs集字段,16版11ac协议新增bit30，涉及rom,原始结构体不再使用 */
typedef struct {
    uint32_t  bit_rx_mcs_rom_map            : 16,
                bit_rx_highest_rom_rate       : 13,
                bit_max_nsts                  : 3;
    uint32_t  bit_tx_mcs_rom_map            : 16,
                bit_tx_highest_rom_rate       : 13,
                bit_resv2                     : 3;
}mac_vht_sup_mcs_set_rom_stru;
/* vht信息元素，支持的mcs集字段 */
typedef struct {
    uint32_t  bit_rx_mcs_map                : 16,
                bit_rx_highest_rate           : 13,
                bit_max_nsts                  : 3;
    uint32_t  bit_tx_mcs_map                : 16,
                bit_tx_highest_rate           : 13,
                bit_vht_extend_nss_bw_capable : 1,
                bit_resv2                     : 2;
}mac_vht_sup_mcs_set_stru;

/* ht capabilities信息元素支持的extended cap.字段结构体定义 */
struct mac_ext_cap {
    uint16_t bit_pco : 1, /* */
               bit_pco_trans_time : 2,
               bit_resv1 : 5,
               bit_mcs_fdbk : 2,
               bit_htc_sup : 1,
               bit_rd_resp : 1,
               bit_resv2 : 4;
} DECLARE_PACKED;
typedef struct mac_ext_cap mac_ext_cap_stru;

/* ht cap信息元素的Transmit Beamforming Capabilities字段结构体定义 */
typedef struct {
    uint32_t bit_implicit_txbf_rx : 1,
               bit_rx_stagg_sounding : 1,
               bit_tx_stagg_sounding : 1,
               bit_rx_ndp : 1,
               bit_tx_ndp : 1,
               bit_implicit_txbf : 1,
               bit_calibration : 2,
               bit_explicit_csi_txbf : 1,
               bit_explicit_noncompr_steering : 1,
               bit_explicit_compr_steering : 1,
               bit_explicit_txbf_csi_fdbk : 2,
               bit_explicit_noncompr_bf_fdbk : 2,
               bit_explicit_compr_bf_fdbk : 2,
               bit_minimal_grouping : 2,
               bit_csi_num_bf_antssup : 2,
               bit_noncompr_steering_num_bf_antssup : 2,
               bit_compr_steering_num_bf_antssup : 2,
               bit_csi_maxnum_rows_bf_sup : 2,
               bit_chan_estimation : 2,
               bit_resv : 3;
} mac_txbf_cap_stru;

/* ht cap信息元素的Asel(antenna selection) Capabilities字段结构体定义 */
struct mac_asel_cap {
    uint8_t bit_asel : 1,
              bit_explicit_sci_fdbk_tx_asel : 1,
              bit_antenna_indices_fdbk_tx_asel : 1,
              bit_explicit_csi_fdbk : 1,
              bit_antenna_indices_fdbk : 1,
              bit_rx_asel : 1,
              bit_trans_sounding_ppdu : 1,
              bit_resv : 1;
} DECLARE_PACKED;
typedef struct mac_asel_cap mac_asel_cap_stru;

/* ht opern元素, ref 802.11-2012 8.4.2.59 */
struct mac_ht_opern {
    uint8_t uc_primary_channel;

    uint8_t bit_secondary_chan_offset : 2,
              bit_sta_chan_width : 1,
              bit_rifs_mode : 1,
              bit_resv1 : 4;
    uint8_t bit_ht_protection : 2,
              bit_nongf_sta_present : 1,
              bit_resv2 : 1,
              bit_obss_nonht_sta_present : 1,
              bit_resv3 : 3;
    uint8_t bit_resv4 : 8;
    uint8_t bit_resv5 : 6,
              bit_dual_beacon : 1,
              bit_dual_cts_protection : 1;
    uint8_t bit_secondary_beacon : 1,
              bit_lsig_txop_protection_full_support : 1,
              bit_pco_active : 1,
              bit_pco_phase : 1,
              bit_resv6 : 4;

    uint8_t auc_basic_mcs_set[MAC_HT_BASIC_MCS_SET_LEN];
} DECLARE_PACKED;
typedef struct mac_ht_opern mac_ht_opern_stru;

/* ht opern元素, 802.11-2016 11ac */
struct mac_ht_opern_ac {
    uint8_t uc_primary_channel;

    uint8_t bit_secondary_chan_offset : 2,
              bit_sta_chan_width : 1,
              bit_rifs_mode : 1,
              bit_resv1 : 4;
    uint16_t bit_ht_protection : 2,
               bit_nongf_sta_present : 1,
               bit_resv2 : 1,
               bit_obss_nonht_sta_present : 1,
               bit_chan_center_freq_seg2 : 8,
               bit_resv3 : 3;
    uint8_t bit_resv4 : 6,
              bit_dual_beacon : 1,
              bit_dual_cts_protection : 1;
    uint8_t bit_secondary_beacon : 1,
              bit_lsig_txop_protection_full_support : 1,
              bit_pco_active : 1,
              bit_pco_phase : 1,
              bit_resv5 : 4;

    uint8_t auc_basic_mcs_set[MAC_HT_BASIC_MCS_SET_LEN];
} DECLARE_PACKED;
typedef struct mac_ht_opern_ac mac_ht_opern_ac_stru;
/* 用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack()

#endif
