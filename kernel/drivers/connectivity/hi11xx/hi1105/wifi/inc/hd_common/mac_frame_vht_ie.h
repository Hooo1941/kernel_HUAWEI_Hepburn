/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应的帧的结构及操作接口定义的源文件(HAL模块不可以调用)
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef MAC_FRAME_VHT_IE_H
#define MAC_FRAME_VHT_IE_H

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
/* vht opern结构体 */
struct mac_opmode_notify {
uint8_t bit_channel_width : 2, /* 当前最大允许带宽能力 */
bit_160or8080 : 1, /* 是否是160M */
              bit_noldpc : 1,
              bit_rx_nss : 3,      /* 当前最大允许空间流能力 */
              bit_rx_nss_type : 1; /* 是否为TXBF下的rx nss能力，1-是，0-不是 */
}
DECLARE_PACKED;
typedef struct mac_opmode_notify mac_opmode_notify_stru;

/* vht opern结构体 */
struct mac_vht_opern {
uint8_t uc_channel_width;
uint8_t uc_channel_center_freq_seg0;
uint8_t uc_channel_center_freq_seg1;
uint16_t us_basic_mcs_set;
} DECLARE_PACKED;
typedef struct mac_vht_opern mac_vht_opern_stru;

/* RRM ENABLED CAP信息元素结构体 */
typedef struct oal_rrm_enabled_cap_ie mac_rrm_enabled_cap_ie_stru;

/* Measurement Report Mode */
struct mac_mr_mode {
    uint8_t bit_parallel : 1,
              bit_enable : 1,
              bit_request : 1,
              bit_rpt : 1,
              bit_duration_mandatory : 1,
              bit_rsvd : 3;
} DECLARE_PACKED;
typedef struct mac_mr_mode mac_mr_mode_stru;

/* Radio Measurement Request */
struct mac_action_rm_req {
    uint8_t uc_category;
    uint8_t uc_action_code;
    uint8_t uc_dialog_token;
    uint16_t us_num_rpt;
    uint8_t auc_req_ies[1];
} DECLARE_PACKED;
typedef struct mac_action_rm_req mac_action_rm_req_stru;

/* Neighbor Report Request */
struct mac_action_neighbor_req {
    uint8_t uc_category;
    uint8_t uc_action_code;
    uint8_t uc_dialog_token;
    uint8_t auc_subelm[1];
} DECLARE_PACKED;
typedef struct mac_action_neighbor_req mac_action_neighbor_req_stru;

/* Neighbor Report Response */
struct mac_action_neighbor_rsp {
    uint8_t uc_category;
    uint8_t uc_action_code;
    uint8_t uc_dialog_token;
    uint8_t neighbor_rpt_ies[1];
} DECLARE_PACKED;
typedef struct mac_action_neighbor_rsp mac_action_neighbor_rsp_stru;

/* Radio Measurement Report */
struct mac_action_rm_rpt {
    uint8_t uc_category;
    uint8_t uc_action_code;
    uint8_t uc_dialog_token;
    uint8_t auc_rpt_ies[1];
} DECLARE_PACKED;
typedef struct mac_action_rm_rpt mac_action_rm_rpt_stru;

/* Measurement Request Elements */
struct mac_meas_req_ie {
    uint8_t uc_eid; /* IEEE80211_ELEMID_MEASREQ */
    uint8_t uc_len;
    uint8_t uc_token;
    mac_mr_mode_stru st_reqmode;
    uint8_t uc_reqtype;
    uint8_t auc_meas_req[1]; /* varialbe len measurement requet */
} DECLARE_PACKED;
typedef struct mac_meas_req_ie mac_meas_req_ie_stru;

struct mac_ftm_range_req_ie {
    uint16_t us_randomization_interval;
    uint8_t uc_minimum_ap_count;
    uint8_t auc_ftm_range_subelements[1];
} DECLARE_PACKED;
typedef struct mac_ftm_range_req_ie mac_ftm_range_req_ie_stru;

/* Measurement Report Mode */
struct mac_meas_rpt_mode {
    uint8_t bit_late : 1,
              bit_incapable : 1,
              bit_refused : 1,
              bit_rsvd : 5;
} DECLARE_PACKED;
typedef struct mac_meas_rpt_mode mac_meas_rpt_mode_stru;
/* Measurement Report Elements */
struct mac_meas_rpt_ie {
    uint8_t uc_eid; /* IEEE80211_ELEMID_MEASRPT */
    uint8_t uc_len;
    uint8_t uc_token;
    mac_meas_rpt_mode_stru st_rptmode;
    uint8_t uc_rpttype;
    uint8_t auc_meas_rpt[100]; /* varialbe len measurement report -- 1 ,数组长度100byte */
} DECLARE_PACKED;
typedef struct mac_meas_rpt_ie mac_meas_rpt_ie_stru;

/* Neighbor Report Elements */
struct mac_bssid_info {
    uint32_t bit_ap_reachability : 2,
               bit_security : 1,
               bit_key_scope : 1,
               bit_spec_management : 1,
               bit_qos : 1,
               bit_apsd : 1,
               bit_radio_meas : 1,
               bit_delayed_ba : 1,
               bit_immediate_ba : 1,
               bit_rsv : 22;
} DECLARE_PACKED;
typedef struct mac_bssid_info mac_bssid_info_stru;

struct mac_neighbor_rpt_ie {
    uint8_t uc_eid; /* IEEE80211_ELEMID_MEASRPT */
    uint8_t uc_len;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    mac_bssid_info_stru st_bssid_info;
    uint8_t uc_optclass;
    uint8_t uc_channum;
    uint8_t uc_phy_type;
    uint8_t auc_subelm[1]; /* varialbe len sub element fileds */
} DECLARE_PACKED;
typedef struct mac_neighbor_rpt_ie mac_neighbor_rpt_ie_stru;

struct mac_meas_sub_ie {
    uint8_t sub_eid; /* IEEE80211_ELEMID_MEASREQ */
    uint8_t len;
    uint8_t meas_sub_data[1]; /* varialbe len measurement requet */
} DECLARE_PACKED;
typedef struct mac_meas_sub_ie mac_meas_sub_ie_stru;

/* Beacon Report SubElement: Reported Frame Body Fgrament ID */
struct mac_reported_frame_body_fragment_id {
    uint8_t sub_eid;
    uint8_t len;
    uint8_t bit_beacon_report_id; /* beacon report编号 */
    uint8_t bit_fragment_id_number : 7, /* beacon report分段编号 */
            bit_more_frame_body_fragments : 1; /* 当前beacon report后续是否还有更多分段，[1:有|0:没有] */
} DECLARE_PACKED;
typedef struct mac_reported_frame_body_fragment_id mac_reported_frame_body_fragment_id_stru;

/* Beacon Report SubElement: Last Beacon Report Indication */
struct mac_last_beacon_report_indication {
    uint8_t sub_eid;
    uint8_t len;
    uint8_t last_beacon_report_indication; /* 是否是最后一个beacon report，[1:是|0:不是] */
} DECLARE_PACKED;
typedef struct mac_last_beacon_report_indication mac_last_beacon_report_indication_stru;

/* Beacon report request */
struct mac_bcn_req {
    uint8_t uc_optclass;
    uint8_t uc_channum; /*  请求测量的信道号 */
    uint16_t us_random_ivl;
    uint16_t us_duration;
    rm_bcn_req_meas_mode_enum_uint8 en_mode;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t auc_subelm[]; /* varialbe len sub element fileds */
} DECLARE_PACKED;
typedef struct mac_bcn_req mac_bcn_req_stru;

/* Beacon report */
struct mac_bcn_rpt {
    uint8_t uc_optclass;
    uint8_t uc_channum;
    uint32_t aul_act_meas_start_time[2]; /* 数组长为2个uint32_t */
    uint16_t us_duration;
    uint8_t bit_condensed_phy_type : 7,
              bit_rpt_frm_type : 1;
    uint8_t uc_rcpi;
    uint8_t uc_rsni;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t uc_antenna_id;
    uint32_t parent_tsf;
    uint8_t auc_subelm[]; /* varialbe len sub element fileds */
} DECLARE_PACKED;
typedef struct mac_bcn_rpt mac_bcn_rpt_stru;

/* Channel Load request */
struct mac_chn_load_req {
    uint8_t uc_optclass;
    uint8_t uc_channum; /*  请求测量的信道号 */
    uint16_t us_random_ivl;
    uint16_t us_duration;
    uint8_t uc_chn_load;
    uint8_t auc_subelm[1]; /* varialbe len sub element fileds */
} DECLARE_PACKED;
typedef struct mac_chn_load_req mac_chn_load_req_stru;

/* Channel Load Report */
struct mac_chn_load_rpt {
    uint8_t uc_optclass;
    uint8_t uc_channum;
    uint32_t aul_act_meas_start_time[2]; /* 数组长为2个uint32_t */
    uint16_t us_duration;
    uint8_t uc_channel_load;
} DECLARE_PACKED;
typedef struct mac_chn_load_rpt mac_chn_load_rpt_stru;

/* Frame request */
struct mac_frm_req {
    uint8_t uc_optclass;
    uint8_t uc_channum; /*  请求测量的信道号 */
    uint16_t us_random_ivl;
    uint16_t us_duration;
    uint8_t uc_frm_req_type;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];
    uint8_t auc_subelm[1]; /* varialbe len sub element fileds */
} DECLARE_PACKED;
typedef struct mac_frm_req mac_frm_req_stru;

/* Frame Report */
struct mac_frm_cnt_rpt {
    uint8_t auc_tx_addr[WLAN_MAC_ADDR_LEN];
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t uc_phy_type;
    uint8_t uc_avrg_rcpi;
    uint8_t uc_last_rsni;
    uint8_t uc_last_rcpi;
    uint8_t uc_ant_id;
    uint16_t us_frm_cnt;
} DECLARE_PACKED;
typedef struct mac_frm_cnt_rpt mac_frm_cnt_rpt_stru;

/* Fine Timing Measurement Parameters */
struct mac_ftm_parameters_ie {
    uint8_t uc_eid; /* IEEE80211_ELEMID_MEASRPT */
    uint8_t uc_len;
    uint8_t bit_status_indication : 2,
              bit_value : 5,
              bit_reserved0 : 1;
    uint8_t bit_number_of_bursts_exponent : 4,
              bit_burst_duration : 4;
    uint8_t uc_min_delta_ftm;
    uint16_t us_partial_tsf_timer;
    uint8_t bit_partial_tsf_timer_no_preference : 1,
              bit_asap_capable : 1,
              bit_asap : 1,
              bit_ftms_per_burst : 5;
    uint8_t bit_reserved : 2,
              bit_format_and_bandwidth : 6;
    uint16_t us_burst_period;
} DECLARE_PACKED;
typedef struct mac_ftm_parameters_ie mac_ftm_parameters_ie_stru;

struct mac_frm_rpt {
    uint8_t uc_optclass;
    uint8_t uc_channum;
    uint8_t auc_act_meas_start_time[MAC_RADIO_MEAS_START_TIME_LEN];
    uint16_t us_duration;
    uint8_t auc_subelm[1]; /* varialbe len sub element fileds */
} DECLARE_PACKED;
typedef struct mac_frm_rpt mac_frm_rpt_stru;

/* Statistic Report basic */
struct mac_stats_rpt_basic {
    uint16_t us_duration;
    uint8_t uc_group_id;
    uint8_t auc_group[1];
} DECLARE_PACKED;
typedef struct mac_stats_rpt_basic mac_stats_rpt_basic_stru;

/* Statistic Report Group0, dot11Counters Group */
struct mac_stats_cnt_rpt {
    uint32_t tx_frag_cnt;
    uint32_t multi_tx_cnt;
    uint32_t fail_cnt;
    uint32_t rx_frag_cnt;
    uint32_t multi_rx_cnt;
    uint32_t fcs_err_cnt;
    uint32_t tx_cnt;
} DECLARE_PACKED;
typedef struct mac_stats_cnt_rpt mac_stats_cnt_rpt_stru;

/* Statistic Report Group1, dot11MACStatistics Group */
struct mac_stats_mac_rpt {
    uint32_t retry_cnt;
    uint32_t multi_retry_cnt;
    uint32_t dup_cnt;
    uint32_t rts_succ_cnt;
    uint32_t trs_fail_cnt;
    uint32_t ack_fail_cnt;
} DECLARE_PACKED;
typedef struct mac_stats_mac_rpt mac_stats_mac_rpt_stru;

/* Statistic Report Group2~9 */
struct mac_stats_up_cnt_rpt {
    uint32_t tx_frag_cnt;
    uint32_t fail_cnt;
    uint32_t retry_cnt;
    uint32_t multi_retry_cnt;
    uint32_t dup_cnt;
    uint32_t rts_succ_cnt;
    uint32_t trs_fail_cnt;
    uint32_t ack_fail_cnt;
    uint32_t rx_frag_cnt;
    uint32_t tx_cnt;
    uint32_t discard_cnt;
    uint32_t rx_cnt;
    uint32_t rx_retry_cnt;
} DECLARE_PACKED;
typedef struct mac_stats_up_cnt_rpt mac_stats_up_cnt_rpt_stru;

/* Statistic Report Group11 */
struct mac_stats_access_delay_rpt {
    uint32_t avrg_access_delay;
    uint32_t aul_avrg_access_delay[4]; /* 数组长为4个uint32_t */
    uint32_t sta_cnt;
    uint32_t channel_utilz;
} DECLARE_PACKED;
typedef struct mac_stats_access_delay_rpt mac_stats_access_delay_rpt_stru;

/* TSC Report */
struct mac_tsc_rpt {
    uint8_t auc_act_meas_start_time[MAC_RADIO_MEAS_START_TIME_LEN];
    uint16_t us_duration;
    uint8_t auc_peer_sta_addr[WLAN_MAC_ADDR_LEN];
    uint8_t uc_tid;
    uint8_t uc_rpt_reason;

    uint32_t tx_frag_cnt;
    uint32_t fail_cnt;
    uint32_t retry_cnt;
    uint32_t multi_retry_cnt;
} DECLARE_PACKED;
typedef struct mac_tsc_rpt mac_tsc_rpt_stru;

/* AP Channel Report */
struct mac_ap_chn_rpt {
    uint8_t uc_eid;
    uint8_t uc_length;
    uint8_t uc_oper_class;
    uint8_t auc_chan[1];
} DECLARE_PACKED;
typedef struct mac_ap_chn_rpt mac_ap_chn_rpt_stru;

struct mac_pwr_constraint_frm {
    uint8_t uc_eid;
    uint8_t uc_len;
    uint8_t uc_local_pwr_constraint;
} DECLARE_PACKED;
typedef struct mac_pwr_constraint_frm mac_pwr_constraint_frm_stru;

typedef struct {
    uint8_t en_app_ie_type;
    uint8_t ie_len;
    uint8_t huawei_oui[NUM_3_BYTES];
    uint8_t feature_id;
    uint8_t cap_tlv_type;  // 0
    uint8_t cap_len;       // 2
    uint8_t support_adaptive_11r : 1;
    uint8_t support_conflict_stat : 1;
    uint8_t support_5g : 1;
    uint8_t support_dual_5g : 1;
    uint8_t support_11ad : 1;
    uint8_t support_24g_channel_mode : 2;
    uint8_t support_neighbor_report : 1;
    uint8_t akm_suite_val : 4;
    uint8_t mdid_flag : 1;
    uint8_t ft_over_ds : 1;
    uint8_t support_res_req_proto : 1;
    uint8_t support_nallow_band : 1;
    uint8_t dc_roaming_type;  // 7
    uint8_t dc_roaming_len;   // 8
    uint8_t other_radio_mac[NUM_6_BYTES];
    uint8_t other_radio_channel;
    uint8_t other_radio_bandwidth;
    uint8_t other_radio_erip;
} oal_sta_ap_cowork_ie;

typedef struct {
    uint8_t real_akm_val;
    uint32_t akm_suite;
} oal_sta_ap_cowork_akm_para_stru;

typedef struct {
    uint8_t tlv_type;  // cap_tlv_type
    uint8_t tlv_len;   // cap_len
    uint8_t support_adaptive_11r : 1;
    uint8_t support_conflict_stat : 1;
    uint8_t support_5g : 1;
    uint8_t support_dual_5g : 1;
    uint8_t support_11ad : 1;
    uint8_t support_24g_channel_mode : 2;
    uint8_t support_neighbor_report : 1;
    uint8_t akm_suite_val : 4;
    uint8_t mdid_flag : 1;
    uint8_t ft_over_ds : 1;
    uint8_t support_res_req_proto : 1;
    uint8_t resv : 1;
    uint16_t mdid;
} oal_cowork_ie_capa_bitmap_stru;

typedef struct {
    uint8_t tlv_type;  // dc_roaming_type
    uint8_t tlv_len;   // dc_roaming_len
    uint8_t other_radio_mac[NUM_6_BYTES];
    uint8_t other_radio_channel;
    uint8_t other_radio_bandwidth;
    uint8_t other_radio_erip;
} oal_cowork_ie_dc_roaming_stru;

typedef struct {
    uint8_t en_app_ie_type;
    uint8_t ie_len;
    uint8_t huawei_oui[NUM_3_BYTES];
    uint8_t feature_id;
    oal_cowork_ie_capa_bitmap_stru capa_bitmap;
    oal_cowork_ie_dc_roaming_stru dc_roaming; /* 暂时不携带此部分信息，ie帧长度需要减去11 */
} oal_sta_ap_cowork_ie_beacon;

typedef struct {
    uint8_t tag_num;
    uint8_t tag_len;
    uint16_t mdid;
    uint8_t ft_over_ds : 1;
    uint8_t support_res_req_proto : 1;
    uint8_t resv : 6;
} oal_cowork_md_ie;

typedef struct {
    uint8_t en_app_ie_type;
    uint8_t ie_len;
    uint8_t huawei_oui[3]; // 3字节oui
    uint8_t feature_id;
    uint8_t cap_tlv_type;  // 0
    uint8_t cap_len;
    uint8_t tx_pwr_detail_2g[2]; /* EIRP、52-ToneEIRP */
    uint8_t tx_pwr_detail_5g[4][2]; /* EIRP、52-ToneEIRP，分4个band */
} oal_sta_max_power_ie;

typedef struct {
    uint8_t en_app_ie_type;
    uint8_t ie_len;
    uint8_t huawei_oui[3]; // 3字节oui
    uint8_t feature_id;
    uint8_t cap_tlv_type;  // 0
    uint8_t cap_len;
    uint8_t tb_frame_gain_gain_2g;
    uint8_t tb_frame_gain_gain_5g[4]; /* 分4个band */
} oal_ap_tb_frame_gain_ie;
/* 用#pragma pack(1)/#pragma pack()方式达到一字节对齐 */
#pragma pack()

#endif
