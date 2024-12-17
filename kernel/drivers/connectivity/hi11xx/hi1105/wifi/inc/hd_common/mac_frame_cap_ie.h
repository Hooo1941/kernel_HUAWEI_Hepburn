/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应的帧的结构及操作接口定义的源文件(HAL模块不可以调用)
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef MAC_FRAME_CAP_IE_H
#define MAC_FRAME_CAP_IE_H

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
/* WFA TPC RPT OUI 定义 */
extern const uint8_t g_auc_wfa_oui[MAC_OUI_LEN];

/* 用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack(1)
/* Ref. 802.11-2012.pdf, 8.4.1.4 Capability information field, 中文注释参考白皮书 */
struct mac_cap_info {
    uint16_t bit_ess : 1,          /* 由BSS中的AP设置为1 */
               bit_ibss : 1,                /* 由一个IBSS中的站点设置为1，ap总是设置其为0 */
               bit_cf_pollable : 1,         /* 标识CF-POLL能力 */
               bit_cf_poll_request : 1,     /* 标识CF-POLL能力  */
               bit_privacy : 1,             /* 1=需要加密, 0=不需要加密 */
               bit_short_preamble : 1,      /* 802.11b短前导码 */
               bit_pbcc : 1,                /* 802.11g */
               bit_channel_agility : 1,     /* 802.11b */
               bit_spectrum_mgmt : 1,       /* 频谱管理: 0=不支持, 1=支持 */
               bit_qos : 1,                 /* QOS: 0=非QOS站点, 1=QOS站点 */
               bit_short_slot_time : 1,     /* 短时隙: 0=不支持, 1=支持 */
               bit_apsd : 1,                /* 自动节能: 0=不支持, 1=支持 */
               bit_radio_measurement : 1,   /* Radio检测: 0=不支持, 1=支持 */
               bit_dsss_ofdm : 1,           /* 802.11g */
               bit_delayed_block_ack : 1,   /* 延迟块确认: 0=不支持, 1=支持 */
               bit_immediate_block_ack : 1; /* 立即块确认: 0=不支持, 1=支持 */
} DECLARE_PACKED;
typedef struct mac_cap_info mac_cap_info_stru;

/* Ref. 802.11-2012.pdf, 8.4.2.58.2 HT Capabilities Info field */
struct mac_frame_ht_cap {
    uint16_t bit_ldpc_coding_cap : 1,  /* LDPC 编码 capability    */
               bit_supported_channel_width : 1, /* STA 支持的带宽          */
               bit_sm_power_save : 2,           /* SM 省电模式             */
               bit_ht_green_field : 1,          /* 绿野模式                */
               bit_short_gi_20mhz : 1,          /* 20M下短保护间隔         */
               bit_short_gi_40mhz : 1,          /* 40M下短保护间隔         */
               bit_tx_stbc : 1,                 /* Indicates support for the transmission of PPDUs using STBC */
               bit_rx_stbc : 2,                 /* 支持 Rx STBC            */
               bit_ht_delayed_block_ack : 1,    /* Indicates support for HT-delayed Block Ack opera-tion. */
               bit_max_amsdu_length : 1,        /* Indicates maximum A-MSDU length. */
               bit_dsss_cck_mode_40mhz : 1,     /* 40M下 DSSS/CCK 模式     */
               bit_resv : 1,
               /*
                * Indicates whether APs receiving this information or reports of this informa-tion are
                * required to prohibit 40 MHz transmissions
                */
               bit_forty_mhz_intolerant : 1,
               bit_lsig_txop_protection : 1; /* 支持L-SIG TXOP保护 */
} DECLARE_PACKED;
typedef struct mac_frame_ht_cap mac_frame_ht_cap_stru;

struct mac_vht_cap_info {
    uint32_t bit_max_mpdu_length : 2,
               bit_supported_channel_width : 2,
               bit_rx_ldpc : 1,
               bit_short_gi_80mhz : 1,
               bit_short_gi_160mhz : 1,
               bit_tx_stbc : 1,
               bit_rx_stbc : 3,
               bit_su_beamformer_cap : 1,
               bit_su_beamformee_cap : 1,
               bit_num_bf_ant_supported : 3,
               bit_num_sounding_dim : 3,
               bit_mu_beamformer_cap : 1,
               bit_mu_beamformee_cap : 1,
               bit_vht_txop_ps : 1,
               bit_htc_vht_capable : 1,
               bit_max_ampdu_len_exp : 3,
               bit_vht_link_adaptation : 2,
               bit_rx_ant_pattern : 1,
               bit_tx_ant_pattern : 1,
               bit_extend_nss_bw_supp : 2;
};
typedef struct mac_vht_cap_info mac_vht_cap_info_stru;

#define NOA_IE_ONE_NOA_LEN 13
#define NOA_IE_TWO_NOA_LEN 26
#define NOA_PERIOD_NOA_CNT 255

struct noa_ie_param {
    uint8_t  noacnt;
    uint32_t noaduration;
    uint32_t noainterval;
    uint32_t noastatrtime;
}DECLARE_PACKED;
typedef struct noa_ie_param noa_ie_param_stru;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
struct mac_frame_he_trig {
    /* byte 0 */
    uint16_t bit_trig_type : 4;
    uint16_t bit_ppdu_len : 12;

    uint32_t bit_cascade : 1; /* 当前skb是否为amsdu的首个skb */
    uint32_t bit_cs_required : 1;
    uint32_t bit_bandwith : 2;
    uint32_t bit_ltf_gi : 2;
    uint32_t bit_mu_mimo_ltf_mode : 1;
    uint32_t bit_num_of_ltfs : 3;
    uint32_t bit_stbc : 1;
    uint32_t bit_ldpc_extra_symbol : 1;
    uint32_t bit_ap_tx_pwr : 6;
    uint32_t bit_pe_fec_pading : 2;
    uint32_t bit_pe_disambiguity : 1;

    uint32_t bit_spatial_reuse : 16;
    uint32_t bit_dopler : 1;
    uint32_t bit_he_siga : 9;
    uint32_t bit_resv : 1;
} DECLARE_PACKED;
typedef struct mac_frame_he_trig mac_frame_he_trig_stru;

struct mac_frame_trig_depend_user_info {
    uint8_t bit_mu_spacing_factor : 2;
    uint8_t bit_multi_tid_aggr_limit : 3;
    uint8_t bit_resv : 1;
    uint8_t bit_perfer_ac : 2;
} DECLARE_PACKED;
typedef struct mac_frame_trig_depend_user_info mac_frame_trig_depend_user_info_stru;

struct mac_frame_he_trig_user_info {
    /* byte 0 */
    uint32_t bit_user_aid : 12;
    uint32_t bit_ru_location : 8;
    uint32_t bit_coding_type : 1;
    uint32_t bit_mcs : 4;
    uint32_t bit_dcm : 1;
    uint32_t bit_starting_spatial_stream : 3;
    uint32_t bit_nss : 3;

    uint8_t bit_target_rssi : 7;
    uint8_t bit_resv : 1;

    mac_frame_trig_depend_user_info_stru st_user_info;
} DECLARE_PACKED;
typedef struct mac_frame_he_trig_user_info mac_frame_he_trig_user_info_stru;

/* HE_Cap:he mac cap 字段，Len=6  */
struct mac_frame_he_mac_cap {
    /* B0-指示站点是否支持接收Qos Data、Qos Null、管理帧携带 HE 变体的HT Control field */
    uint8_t bit_htc_he_support : 1,
              bit_twt_requester_support : 1, /* B1-是否支持TWT Requester */
              bit_twt_responder_support : 1, /* B2-是否支持TWT Responder */
              bit_fragmentation_support : 2, /* B3-动态分片能力 */
              bit_msdu_max_fragment_num : 3; /* B5-msdu最大分片数 */

    uint16_t bit_min_fragment_size : 2,        /* B8-最小分片的长度 */
               bit_trigger_mac_padding_duration : 2,    /* B10-trigger mac padding 时长 */
               bit_mtid_aggregation_rx_support : 3,     /* B12-接收多tid聚合支持 */
               bit_he_link_adaptation : 2,              /* B15-使用HE 变体的HT Control field 调整 */
               bit_all_ack_support : 1,                 /* B17-支持接收M-BA */
               bit_trs_support : 1,                    /* B18-A-Control 支持 UL MU Response Scheduling */
               bit_bsr_support : 1,                     /* B19-A-Control 支持BSR */
               bit_broadcast_twt_support : 1,           /* B20-支持广播twt */
               bit_32bit_ba_bitmap_support : 1,         /* B21-支持32位bitmap ba */
               bit_mu_cascading_support : 1,            /* B22-mu csacade */
               bit_ack_enabled_aggregation_support : 1; /* B23-A-MPDU ack使能 */

    uint8_t bit_reserved_b24 : 1,           /* B24- */
              bit_om_control_support : 1,           /* B25-A-Control 支持OMI  */
              bit_ofdma_ra_support : 1,             /* B26-OFDMA 随机接入 */
              bit_max_ampdu_length_exponent : 2,    /* B27-A-MPDU最大长度 */
              bit_amsdu_fragment_support : 1,       /* B29-A-MSDU Fragmentation Support */
              bit_flex_twt_schedule_support : 1,    /* B30- */
              bit_rx_control_frame_to_multibss : 1; /* B31- */

    uint16_t bit_bsrp_bqrp_ampdu_addregation : 1,       /* B32- */
               bit_qtp_support : 1,                              /* B33- */
               bit_bqr_support : 1,                              /* B34- HTC BQR */
               bit_srp_responder : 1,                             /* B35- */
               bit_ndp_feedback_report_support : 1,              /* B36- */
               bit_ops_support : 1,                              /* B37- */
               bit_amsdu_no_ba_ack_enabled_support : 1,          /* B38-ack enabled related */
               bit_mtid_aggregation_tx_support : 3,              /* B39-41 */
               bit_he_subchannel_select_transmission : 1,        /* B42 */
               bit_ul_2x996_ru_support : 1,                      /* B43 */
               bit_om_control_ul_mu_data_disable_rx_support : 1, /* B44 */
               bit_he_dynamic_sm_power_save : 1,                 /* B45 */
               bit_punctured_sounding_support : 1,               /* B46 */
               bit_he_and_vht_trigger_frame_rx_support : 1;      /* B47 */
} DECLARE_PACKED;
typedef struct mac_frame_he_mac_cap mac_frame_he_mac_cap_stru;

/* HE_CAP: PHY Cap 字段 Len=11   */
struct mac_frame_he_phy_cap {
    uint8_t bit_reserved : 1,    /* B0  */
              bit_channel_width_set : 7; /* B1-7: */

    uint16_t bit_punctured_preamble_rx : 4, /* B8-11: */
               bit_device_class : 1,                 /* B12- */
               bit_ldpc_coding_in_paylod : 1,        /* B13- */
               bit_he_su_ppdu_1x_he_ltf_08us_gi : 1,     /* B14-HE SU PPDU 1x HE-LTF + 0.8usGI  */
               bit_midamble_trx_max_nsts : 2,         /* B15 */
               bit_ndp_4x_he_ltf_32us_gi : 1,               /* B17-ndp 4x HE-LTF + 3.2usGI */
               bit_stbc_tx_below_80mhz : 1,          /* B18 */
               bit_stbc_rx_below_80mhz : 1,          /* B19 */
               bit_doppler_tx : 1,                   /* B20 */
               bit_doppler_rx : 1,                   /* B21 */
               bit_full_bandwidth_ul_mu_mimo : 1,    /* B22 */
               bit_partial_bandwidth_ul_mu_mimo : 1; /* B23 */

    uint8_t bit_dcm_max_constellation_tx : 2, /* B24- */
              bit_dcm_max_nss_tx : 1,                 /* B26 */
              bit_dcm_max_constellation_rx : 2,       /* B27- */
              bit_dcm_max_nss_rx : 1,                 /* B29- */
              bit_rx_partial_bw_su_in_20mhz_mu_ppdu : 1, /* B30- */
              bit_su_beamformer : 1;                  /* B31- */

    uint8_t bit_su_beamformee : 1,        /* B32- */
              bit_mu_beamformer : 1,              /* B33- */
              bit_beamformee_sts_below_80mhz : 3, /* B34- */
              bit_beamformee_sts_over_80mhz : 3;  /* B37- */

    uint8_t bit_below_80mhz_sounding_dimensions_num : 3, /* B40- */
              bit_over_80mhz_sounding_dimensions_num : 3,        /* B43- */
              bit_ng16_su_feedback : 1,                          /* B46- */
              bit_ng16_mu_feedback : 1;                          /* B47- */

    uint8_t bit_codebook_42_su_feedback : 1,             /* B48- */
              bit_codebook_75_mu_feedback : 1,                   /* B49- */
              bit_trigger_su_beamforming_feedback : 1,           /* B50- */
              bit_trigger_mu_beamforming_partialbw_feedback : 1, /* B51- */
              bit_trigger_cqi_feedback : 1,                      /* B52- */
              bit_partial_bandwidth_extended_range : 1,          /* B53- */
              bit_partial_bandwidth_dl_mu_mimo : 1,              /* B54- */
              bit_ppe_threshold_present : 1;                     /* B55- */

    uint8_t bit_srp_based_sr_support : 1,  /* B56- */
              bit_power_boost_factor_support : 1,  /* B57- */
              bit_he_su_ppdu_he_mu_ppdu_4x_he_ltf_08us_gi : 1, /* B58-he su ppdu and he mu ppdu 4x HE-LTF + 0.8usGI */
              bit_max_nc : 3,                      /* B59- */
              bit_stbc_tx_over_80mhz : 1,          /* B62- */
              bit_stbc_rx_over_80mhz : 1;          /* B63- */

    uint8_t bit_he_er_su_ppdu_4x_he_ltf_08us_gi : 1, /* B64- */
              bit_20mhz_in_40mhz_he_ppdu_2g : 1,         /* B65- */
              bit_20mhz_in_160mhz_he_ppdu : 1,           /* B66- */
              bit_80mhz_in_160mhz_he_ppdu : 1,           /* B67- */
              bit_he_er_su_ppdu_1x_he_ltf_08us_gi : 1,       /* B68- */
              bit_midamble_trx_2x_1x_he_ltf : 1,          /* B69- */
              bit_dcm_max_ru : 2;                        /* B70-71 */

    uint16_t bit_longer_than_16_he_sigb_support : 1,       /* B72- */
               bit_non_triggered_cqi_feedback : 1,                  /* B73 */
               bit_tx_1024qam_below_242ru_support : 1,                    /* B74 */
               bit_rx_1024qam_below_242ru_support : 1,                    /* B75 */
               bit_rx_full_bw_su_ppdu_with_compressed_sigb : 1,     /* B76 */
               bit_rx_full_bw_su_ppdu_with_non_compressed_sigb : 1, /* B77 */
               bit_nominal_packet_padding : 2,                      /* B78_79: */
               bit_reserv : 8;                                      /* B80_87: */
} DECLARE_PACKED;
typedef struct mac_frame_he_phy_cap mac_frame_he_phy_cap_stru; /* 协议规定的 phy cap 格式 */

/* HE CAP:Tx Rx MCS NSS Support */
struct mac_frame_he_mcs_nss_bit_map {
    uint16_t bit_max_he_mcs_1ss : 2,
             bit_max_he_mcs_2ss : 2,
             bit_max_he_mcs_3ss : 2,
             bit_max_he_mcs_4ss : 2,
             bit_max_he_mcs_5ss : 2,
             bit_max_he_mcs_6ss : 2,
             bit_max_he_mcs_7ss : 2,
             bit_max_he_mcs_8ss : 2;
} DECLARE_PACKED;
typedef struct mac_frame_he_mcs_nss_bit_map mac_frame_he_basic_mcs_nss_stru;

struct mac_fram_he_mac_nss_set {
    mac_frame_he_basic_mcs_nss_stru st_rx_he_mcs_map_below_80mhz;
    mac_frame_he_basic_mcs_nss_stru st_tx_he_mcs_map_below_80mhz;
    mac_frame_he_basic_mcs_nss_stru st_rx_he_mcs_map_160mhz;
    mac_frame_he_basic_mcs_nss_stru st_tx_he_mcs_map_160mhz;
    mac_frame_he_basic_mcs_nss_stru st_rx_he_mcs_map_8080mhz;
    mac_frame_he_basic_mcs_nss_stru st_tx_he_mcs_map_8080mhz;
} DECLARE_PACKED;
typedef struct mac_fram_he_mac_nss_set mac_frame_he_supported_he_mcs_nss_set_stru;

/* HE CAP:PPE Thresholds */
/* 该IE用于声明设备接收SU PPDU时,支持的MAC PADDING类型, 当前受限160M接收PAD 16us */
typedef struct {
    uint32_t          bit_nss                                                 :3,
                        bit_ru_index_mask                                       :4,
                        bit_ppet16_nss0_ru0                                     :3,
                        bit_ppet8_nss0_ru0                                      :3,
                        bit_ppet16_nss0_ru1                                     :3,
                        bit_ppet8_nss0_ru1                                      :3,
                        bit_ppet16_nss1_ru0                                     :3,
                        bit_ppet8_nss1_ru0                                      :3,
                        bit_ppet16_nss1_ru1                                     :3,
                        bit_ppet8_nss1_ru1                                      :3,
                        bit_ppe_pad                                             :1;

}DECLARE_PACKED mac_frame_ppe_thresholds_pre_field_stru;

typedef struct {
    mac_frame_he_mac_cap_stru st_he_mac_cap;
    mac_frame_he_phy_cap_stru st_he_phy_cap;
    mac_frame_he_supported_he_mcs_nss_set_stru st_he_mcs_nss;
} DECLARE_PACKED mac_frame_he_cap_ie_stru;

struct mac_frame_he_mcs_nss {
    uint8_t bit_mcs : 4,
              bit_nss : 3,
              bit_last_mcs_nss : 1;
} DECLARE_PACKED;
typedef struct mac_frame_he_mcs_nss mac_frame_he_mcs_nss_stru;

/* 协议规定帧格式 */
struct mac_frame_he_operation_param {
    uint16_t bit_default_pe_duration : 3, /* B0_2: */
               bit_twt_required : 1,               /* B3: */
               bit_he_duration_rts_threshold : 10, /* B4_13: */
               bit_vht_operation_info_present : 1, /* B14: */
               bit_co_located_bss : 1;             /* B15 */

    uint8_t bit_er_su_disable : 1,  /* B16； */
              bit_6g_oper_info_present : 1, /* B17: */
              bit_reserved : 6;             /* B18_23: */
} DECLARE_PACKED;
typedef struct mac_frame_he_operation_param mac_frame_he_operation_param_stru;

struct mac_frame_he_bss_color_info {
    uint8_t bit_bss_color : 6,
              bit_partial_bss_color : 1,
              bit_bss_color_disable : 1;
} DECLARE_PACKED;
typedef struct mac_frame_he_bss_color_info mac_frame_he_bss_color_info_stru;

struct mac_frame_vht_operation_info {
    uint8_t uc_channel_width;
    uint8_t uc_center_freq_seg0;
    uint8_t uc_center_freq_seg1;
} DECLARE_PACKED;
typedef struct mac_frame_vht_operation_info mac_frame_vht_operation_info_stru;

typedef struct
{
    mac_frame_he_operation_param_stru st_he_oper_param;
    mac_frame_he_bss_color_info_stru st_bss_color;
    mac_frame_he_basic_mcs_nss_stru st_he_basic_mcs_nss;
    mac_frame_vht_operation_info_stru st_vht_operation_info;
} DECLARE_PACKED mac_frame_he_oper_ie_stru;

typedef struct {
    uint8_t bit_uora_eocw_min : 3,
            bit_uora_eocw_max : 3,
            bit_uora_capable  : 1,
            bit_resv : 1;
} DECLARE_PACKED mac_he_uora_para_stru;

typedef struct {
    uint16_t min_mpdu_start_spacing : 3,
             max_ampdu_len_exponent : 3,
             max_mpdu_len : 2,
             resv : 1,
             sm_power_save : 2,
             rd_responder : 1,
             rx_ant_pattern : 1,
             tx_ant_pattern : 1,
             resv_1 : 2;
} DECLARE_PACKED mac_he_6ghz_band_cap_info_stru;

/* he mu edca param */
typedef struct
{
    uint8_t bit_edca_update_count : 4,
              bit_q_ack : 1,
              bit_queue_request : 1,
              bit_txop_request : 1,
              bit_more_data_ack : 1;
} DECLARE_PACKED mac_frame_he_mu_qos_info;

typedef struct
{
    uint8_t bit_aifsn : 4,
              bit_acm : 1,
              bit_ac_index : 2,
              bit_reserv : 1;
    uint8_t bit_ecw_min : 4,
              bit_ecw_max : 4;
    uint8_t uc_mu_edca_timer; /* 单位 8TU */
} DECLARE_PACKED mac_frame_he_mu_ac_parameter;

typedef struct
{
    mac_frame_he_mu_qos_info st_qos_info;
    mac_frame_he_mu_ac_parameter ast_mu_ac_parameter[WLAN_WME_AC_BUTT];
} DECLARE_PACKED mac_frame_he_mu_edca_parameter_ie_stru;
/* he mu edca param */
typedef struct
{
    uint8_t bit_srp_disallowed : 1;
    uint8_t bit_non_srg_obss_pd_sr_disallowed : 1;
    uint8_t bit_non_srg_offset_present : 1;
    uint8_t bit_srg_information_present : 1;
    uint8_t bit_hesiga_sr_value15_allowed : 1;
    uint8_t bit_resvered : 3;
} DECLARE_PACKED mac_frame_he_sr_control_stru;

#define MAC_HE_SRG_BSS_COLOR_BITMAP_LEN     8
#define MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN 8
typedef struct
{
    mac_frame_he_sr_control_stru st_sr_control;
    uint8_t uc_non_srg_obss_pd_max_offset;
    uint8_t uc_srg_obss_pd_min_offset;
    uint8_t uc_srg_obss_pd_max_offset;
    uint8_t auc_srg_bss_color_bitmap[MAC_HE_SRG_BSS_COLOR_BITMAP_LEN];
    uint8_t auc_srg_partial_bssid_bitmap[MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN];
} DECLARE_PACKED mac_frame_he_spatial_reuse_parameter_set_ie_stru;

typedef struct
{
    uint8_t uc_color_switch_countdown;
    uint8_t bit_new_bss_color : 6,
              bit_reserved : 2;
} DECLARE_PACKED mac_frame_bss_color_change_annoncement_ie_stru;

typedef union {
    uint32_t htc_value;
    union {
        struct
        {
            uint32_t bit_vht_flag : 1,
                       bit_he_flag : 1,
                       bit_om_id : 4,
                       bit_rx_nss : 3,
                       bit_channel_width : 2,
                       bit_ul_mu_disable : 1,
                       bit_tx_nsts : 3,
                       bit_er_su_disable : 1,
                       bit_dl_mu_mimo_resound_recommendation : 1,
                       bit_ul_mu_data_disable : 1,
                       bit_rsv : 14;
        } st_om_info;
        /* 后续数据帧中携带om 使用 */
        struct
        {
            uint32_t bit_vht_flag : 1,
                       bit_he_flag : 1,
                       bit_uph_id : 4,
                       bit_ul_power_headroom : 5,
                       bit_min_transmit_power_flag : 1,
                       bit_uph_rsv : 2,
                       bit_om_id : 4,
                       bit_rx_nss : 3,
                       bit_channel_width : 2,
                       bit_ul_mu_disable : 1,
                       bit_tx_nsts : 3,
                       bit_er_su_disable : 1,
                       bit_dl_mu_mimo_resound_recommendation : 1,
                       bit_ul_mu_data_disable : 1,
                       bit_rsv : 2;
        } st_uph_om_info;

    } un_a_control_info;
} DECLARE_PACKED mac_htc_a_control_field_union;
#endif /* end _PRE_WLAN_FEATURE_11AX */
/* 用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack()

#endif
