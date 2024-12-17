/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递scan事件 的头文件
 * 创建日期   : 2022年9月16日
 */

#ifndef HD_EVENT_SCAN_H
#define HD_EVENT_SCAN_H

#include "mac_common.h"
#include "wlan_spec.h"

typedef void (*mac_scan_cb_fn)(void *p_scan_record);

#define MAX_PNO_SSID_COUNT      16
#define MAX_CHAN_MEAS_SCAN_NUMBERS 1 /* 信道测量最大允许扫描信道数 */

/* PNO扫描信息结构体 */
typedef struct {
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];
    oal_bool_enum_uint8 en_scan_ssid;
    uint8_t auc_resv[NUM_2_BYTES];
} pno_match_ssid_stru;

typedef struct {
    pno_match_ssid_stru ast_match_ssid_set[MAX_PNO_SSID_COUNT];
    int32_t l_ssid_count;                         /* 下发的需要匹配的ssid集的个数 */
    int32_t l_rssi_thold;                         /* 可上报的rssi门限 */
    uint32_t pno_scan_interval;                /* pno扫描间隔 */
    uint8_t auc_sour_mac_addr[WLAN_MAC_ADDR_LEN]; /* probe req帧中携带的发送端地址 */
    uint8_t uc_pno_scan_repeat;                   /* pno扫描重复次数 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否随机mac */

    mac_scan_cb_fn p_fn_cb; /* 函数指针必须放最后否则核间通信出问题 */
} mac_pno_scan_stru; /* hd_event */

typedef struct {
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];
    uint8_t auc_resv[NUM_3_BYTES];
} mac_ssid_stru;


typedef struct {
    uint8_t en_bss_type; /* wlan_mib_desired_bsstype_enum 要扫描的bss类型 */
    wlan_scan_type_enum_uint8 en_scan_type;          /* 主动/被动 */
    uint8_t uc_max_scan_count_per_channel; /* 每个信道的扫描次数 */

    mac_channel_stru ast_channel_list[WLAN_MAX_CHANNEL_NUM];

    uint8_t uc_channel_nums; /* 信道列表中信道的个数 */
    uint8_t uc_probe_delay;  /* 主动扫描发probe request帧之前的等待时间 */
    uint16_t us_scan_time;   /* 扫描在某一信道停留此时间后，扫描结束, ms，必须是10的整数倍 */

    wlan_scan_mode_enum_uint8 en_scan_mode; /* 扫描模式:前景扫描 or 背景扫描 */
    uint8_t uc_scan_func; /* DMAC SCANNER 扫描模式 */
    uint8_t uc_vap_id;    /* 下发扫描请求的vap id */
    uint8_t rsv2;
    /* 重要:回调函数指针:函数指针必须放最后否则核间通信出问题 */
    mac_scan_cb_fn p_fn_cb;
} mac_cca_meas_req_stru;

/* 扫描参数结构体 */
typedef struct {
    uint8_t en_bss_type; /* wlan_mib_desired_bsstype_enum 要扫描的bss类型 */
    wlan_scan_type_enum_uint8 en_scan_type;          /* 主动/被动 */
    uint8_t uc_bssid_num;                          /* 期望扫描的bssid个数 */
    uint8_t uc_ssid_num;                           /* 期望扫描的ssid个数 */

    uint8_t auc_sour_mac_addr[WLAN_MAC_ADDR_LEN]; /* probe req帧中携带的发送端地址 */
    uint8_t uc_p2p0_listen_channel;
    uint8_t uc_max_scan_count_per_channel; /* 每个信道的扫描次数 */

    uint8_t auc_bssid[WLAN_SCAN_REQ_MAX_BSSID][WLAN_MAC_ADDR_LEN]; /* 期望的bssid */
    mac_ssid_stru ast_mac_ssid_set[WLAN_SCAN_REQ_MAX_SSID];          /* 期望的ssid */

    uint8_t uc_max_send_probe_req_count_per_channel; /* 每次信道发送扫描请求帧的个数，默认为1 */
    uint8_t bit_is_p2p0_scan : 1;                    /* 是否为p2p0 发起扫描 */
    uint8_t bit_is_radom_mac_saved : 1;              /* 是否已经保存随机mac */
    uint8_t bit_radom_mac_saved_to_dev : 2;          /* 用于并发扫描 */
    uint8_t bit_desire_fast_scan : 1;                /* 本次扫描期望使用并发 */
    uint8_t bit_is_special_roc_type : 1;             /* 是否为特殊类型remain on channel */
    uint8_t bit_rsv : 2;                          /* 保留位 */

    oal_bool_enum_uint8 en_abort_scan_flag;         /* 终止扫描 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否是随机mac addr扫描 */

    oal_bool_enum_uint8 en_need_switch_back_home_channel; /* 背景扫描时，扫描完一个信道，判断是否需要切回工作信道工作 */
    uint8_t uc_scan_channel_interval;                   /* 间隔n个信道，切回工作信道工作一段时间 */
    uint16_t us_work_time_on_home_channel;              /* 背景扫描时，返回工作信道工作的时间 */

    mac_channel_stru ast_channel_list[WLAN_MAX_CHANNEL_NUM];

    uint8_t uc_channel_nums; /* 信道列表中信道的个数 */
    uint8_t uc_probe_delay;  /* 主动扫描发probe request帧之前的等待时间 */
    uint16_t us_scan_time;   /* 扫描在某一信道停留此时间后，扫描结束, ms，必须是10的整数倍 */

    wlan_scan_mode_enum_uint8 en_scan_mode; /* 扫描模式:前景扫描 or 背景扫描 */
    uint8_t uc_resv;
    uint8_t uc_scan_func; /* DMAC SCANNER 扫描模式 */
    uint8_t uc_vap_id;    /* 下发扫描请求的vap id */
    uint64_t ull_cookie;  /* P2P 监听下发的cookie 值 */

    oal_bool_enum_uint8 uc_neighbor_report_process_flag; /* 标记此次扫描是否是neighbor report扫描 */

    oal_bool_enum_uint8 uc_bss_transition_process_flag;  /* 标记此次扫描是否是bss transition扫描 */

    /* 重要:回调函数指针:函数指针必须放最后否则核间通信出问题 */
    mac_scan_cb_fn p_fn_cb;
} mac_scan_req_stru;

/* 内核flag保持一致 */
typedef struct {
    uint32_t scan_flag; /* 内核下发的扫描模式,每个bit意义见wlan_scan_flag_enum，暂时只解析是否为并发扫描 */
    mac_scan_req_stru st_scan_params;
} mac_scan_req_h2d_stru;

typedef struct {
    uint8_t uc_chan_number;                       /* 主20MHz信道号 */
    uint8_t en_band : 4,
            ext6g_band : 1, /* 指示是否为6G频段标志 */
            resv : 3;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽模式 */
    uint8_t uc_chan_idx;                          /* 信道索引号 */
    wlan_scan_type_enum_uint8 scan_policy; /* 扫描策略 */
    uint16_t scan_time;
} mac_scan_channel_stru;

typedef struct {
    /* 内核下发的扫描模式,每个bit意义见wlan_scan_flag_enum，暂时只解析是否为并发扫描 */
    uint32_t scan_flag;

    uint8_t en_bss_type; /* wlan_mib_desired_bsstype_enum 要扫描的bss类型 */
    wlan_scan_type_enum_uint8 en_scan_type;          /* 主动/被动 */
    uint8_t uc_bssid_num;                          /* 期望扫描的bssid个数 */
    uint8_t uc_ssid_num;                           /* 期望扫描的ssid个数 */

    uint8_t auc_sour_mac_addr[WLAN_MAC_ADDR_LEN]; /* probe req帧中携带的发送端地址 */
    uint8_t uc_p2p0_listen_channel;
    uint8_t uc_max_scan_count_per_channel; /* 每个信道的扫描次数 */

    uint8_t auc_bssid[WLAN_SCAN_REQ_MAX_BSSID][WLAN_MAC_ADDR_LEN]; /* 期望的bssid */
    mac_ssid_stru ast_mac_ssid_set[WLAN_SCAN_REQ_MAX_SSID];          /* 期望的ssid */

    uint8_t uc_max_send_probe_req_count_per_channel; /* 每次信道发送扫描请求帧的个数，默认为1 */
    uint8_t bit_is_p2p0_scan : 1;                    /* 是否为p2p0 发起扫描 */
    uint8_t bit_is_radom_mac_saved : 1;              /* 是否已经保存随机mac */
    uint8_t bit_radom_mac_saved_to_dev : 2;          /* 用于并发扫描 */
    uint8_t bit_desire_fast_scan : 1;                /* 本次扫描期望使用并发 */
    uint8_t bit_is_special_roc_type : 1;             /* 是否为特殊类型remain on channel */
    uint8_t bit_rsv : 2;                          /* 保留位 */

    oal_bool_enum_uint8 en_abort_scan_flag;         /* 终止扫描 */
    oal_bool_enum_uint8 en_is_random_mac_addr_scan; /* 是否是随机mac addr扫描 */

    oal_bool_enum_uint8 en_need_switch_back_home_channel; /* 背景扫描时，扫描完一个信道，判断是否需要切回工作信道工作 */
    uint8_t uc_scan_channel_interval;                   /* 间隔n个信道，切回工作信道工作一段时间 */
    uint16_t us_work_time_on_home_channel;              /* 背景扫描时，返回工作信道工作的时间 */

    mac_scan_channel_stru ast_channel_list[WLAN_MAX_CHANNEL_NUM];

    uint8_t uc_channel_nums; /* 信道列表中信道的个数 */
    uint8_t uc_probe_delay;  /* 主动扫描发probe request帧之前的等待时间 */
    uint16_t us_scan_time;   /* 扫描在某一信道停留此时间后，扫描结束, ms，必须是10的整数倍 */

    wlan_scan_mode_enum_uint8 en_scan_mode; /* 扫描模式:前景扫描 or 背景扫描 */
    uint8_t uc_resv;
    uint8_t uc_scan_func; /* DMAC SCANNER 扫描模式 */
    uint8_t uc_vap_id;    /* 下发扫描请求的vap id */
    uint64_t ull_cookie;  /* P2P 监听下发的cookie 值 */

    oal_bool_enum_uint8 uc_neighbor_report_process_flag; /* 标记此次扫描是否是neighbor report扫描 */

    oal_bool_enum_uint8 uc_bss_transition_process_flag;  /* 标记此次扫描是否是bss transition扫描 */

    /* 重要:回调函数指针:函数指针必须放最后否则核间通信出问题 */
    mac_scan_cb_fn p_fn_cb;
} mac_scan_req_ex_stru; /* hd_event mp16 only */

/* 扫描结果 */
typedef struct {
    mac_scan_status_enum_uint8 en_scan_rsp_status;
    uint8_t auc_resv[NUM_3_BYTES];
    uint64_t ull_cookie;
} mac_scan_rsp_stru; /* hd_event */

#pragma pack(push, 1)
/* 上报的扫描结果的扩展信息，放在上报host侧的管理帧netbuf的后面 */
typedef struct {
    int32_t l_rssi;                                /* 信号强度 */
    uint8_t en_bss_type; /* wlan_mib_desired_bsstype_enum 扫描到的bss类型 */
    uint8_t auc_resv[BYTE_OFFSET_2]; /* 预留字段 */

    int8_t rssi[HD_EVENT_RF_NUM]; /* 4天线的rssi */
    int8_t snr[HD_EVENT_RF_NUM];  /* 4天线的snr */

    wlan_nss_enum_uint8 en_support_max_nss; /* 该AP支持的最大空间流数 */
#ifdef _PRE_WLAN_FEATURE_M2S
    oal_bool_enum_uint8 en_support_opmode;  /* 该AP是否支持OPMODE */
    uint8_t uc_num_sounding_dim;          /* 该AP发送txbf的天线数 */
#endif
} mac_scanned_result_extend_info_stru; /* hd_event */
#pragma pack(pop)

/* DMAC SCAN 信道统计测量结果结构体 */
typedef struct {
    uint8_t uc_channel_number; /* 信道号 */
    uint8_t uc_stats_valid;
    uint8_t uc_stats_cnt;      /* 信道繁忙度统计次数 */
    uint8_t uc_free_power_cnt; /* 信道空闲功率测量次数 */

    /* PHY信道测量统计 */
    uint8_t uc_bandwidth_mode;
    uint8_t auc_resv[1];
    int16_t free_power_stats_20m;
    int16_t free_power_stats_40m;
    int16_t free_power_stats_80m;

    /* MAC信道测量统计 */
    uint32_t total_stats_time_us;
    uint32_t total_free_time_20m_us;
    uint32_t total_free_time_40m_us;
    uint32_t total_free_time_80m_us;
    uint32_t total_send_time_us;
    uint32_t total_recv_time_us;

    uint8_t uc_radar_detected;  //  feed value
    uint8_t uc_radar_bw;
    uint8_t uc_radar_type;
    uint8_t uc_radar_freq_offset;

    int16_t free_power_stats_160m;
    uint16_t us_phy_total_stats_time_ms;
} wlan_scan_chan_stats_stru;

typedef struct {
    uint8_t uc_scan_idx;
    uint8_t auc_resv[NUM_3_BYTES];
    wlan_scan_chan_stats_stru st_chan_result;
} dmac_crx_chan_result_stru; /* hd_event */

typedef struct {
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];
    uint8_t uc_action_dialog_token;
    uint8_t uc_meas_token;
    uint8_t uc_oper_class;
    uint16_t us_ssid_len;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    uint16_t us_duration;
    uint8_t auc_resv[NUM_2_BYTES];
} mac_vap_rrm_trans_req_info_stru; /* hd_event */

typedef struct {
    uint16_t scan_time; /* 扫描时长 */
    uint8_t resv[2];
    mac_channel_stru scan_channel; /* 待扫描信道 */
} mac_chan_meas_h2d_info_stru; /* host向dmac同步的信道测量相关结构体 */

#endif /* end of hd_event_scan.h */
