/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应的帧的结构及操作接口定义的源文件(HAL模块不可以调用)
 * 创建日期 : 2012年12月3日
 */

#ifndef MAC_FRAME_COMMON_H
#define MAC_FRAME_COMMON_H

/* 1 其他头文件包含 */
#include "wlan_types.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "securec.h"
#include "securectype.h"
#include "mac_element.h"
#include "mac_frame_macro.h"
#include "mac_6ghz_frame_common.h"
#include "mac_frame_80211.h"
#include "mac_frame_ht_ie.h"
#include "mac_frame_vht_ie.h"
#include "mac_frame_cap_ie.h"

#if defined(_PRE_WLAN_FEATURE_WMMAC) || (defined(_PRE_WLAN_FEATURE_11K))
struct mac_ts_info {
    uint16_t bit_traffic_type : 1,
               bit_tsid : 4,
               bit_direction : 2,
               bit_acc_policy : 2,
               bit_aggr : 1,
               bit_apsd : 1,
               bit_user_prio : 3,
               bit_ack_policy : 2;
    uint8_t bit_schedule : 1,
              bit_rsvd : 7;
} DECLARE_PACKED;
typedef struct mac_ts_info mac_ts_info_stru;

struct mac_wmm_tspec {
    mac_ts_info_stru ts_info;
    uint16_t us_norminal_msdu_size;
    uint16_t us_max_msdu_size;
    uint32_t min_srv_interval;
    uint32_t max_srv_interval;
    uint32_t inactivity_interval;
    uint32_t suspension_interval;
    uint32_t srv_start_time;
    uint32_t min_data_rate;
    uint32_t mean_data_rate;
    uint32_t peak_data_rate;
    uint32_t max_burst_size;
    uint32_t delay_bound;
    uint32_t min_phy_rate;
    uint16_t us_surplus_bw;
    uint16_t us_medium_time;
} DECLARE_PACKED;
typedef struct mac_wmm_tspec mac_wmm_tspec_stru;
#endif

/* ACTION帧的参数格式，注:不同的action帧下对应的参数不同 */
typedef struct {
    uint8_t uc_category; /* ACTION的类别 */
    uint8_t uc_action;   /* 不同ACTION类别下的分类 */
    uint8_t uc_resv[NUM_2_BYTES];
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
    uint8_t *puc_arg5;
} mac_action_mgmt_args_stru;

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
/* CHBA可靠组播私有BA帧体 */
typedef struct {
    uint8_t action_type;
    uint8_t vendor_oui[3];
    uint8_t vendor_type;
    uint8_t ba_subtype; /* 组播私有BA帧类型 */
    uint8_t category; /* 私有BA帧的下分类别 */
    uint8_t tid_no;
    uint32_t rx_all_pkts;
    uint32_t rx_non_retry_pkts;
    int16_t rx_data_rssi;
    uint8_t resv[2];
} priv_pack_action_stru;
#endif

/* 私有管理帧通用的设置参数信息的结构体 */
typedef struct {
    uint8_t uc_type;
    uint8_t uc_arg1;      /* 对应的tid序号 */
    uint8_t uc_arg2;      /* 接收端可接收的最大的mpdu的个数(针对AMPDU_START命令) */
    uint8_t uc_arg3;      /* 确认策略 */
    uint16_t us_user_idx; /* 对应的用户 */
    uint8_t auc_resv[NUM_2_BYTES];
} mac_priv_req_args_stru;

/* SMPS节能控制action帧体 */
typedef struct {
    uint8_t category;
    uint8_t action;
    uint8_t sm_ctl;
    uint8_t arg;
} mac_smps_action_mgt_stru;

/* operating mode notify控制action帧体 */
typedef struct {
    uint8_t category;
    uint8_t action;
    uint8_t opmode_ctl;
    uint8_t arg;
} mac_opmode_notify_action_mgt_stru;
typedef struct {
    uint8_t uc_chan_number;                       /* 主20MHz信道号 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
    wlan_channel_band_enum_uint8 en_band;           /* 频段 */
#else
    uint8_t en_band : 4,
            ext6g_band : 1, /* 指示是否为6G频段标志 */
            resv : 3;
#endif
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽模式 */
    uint8_t uc_chan_idx;                          /* 信道索引号 */
} mac_channel_stru;

#ifdef _PRE_WLAN_CHBA_MGMT
/* kernel的信道表示结构体 */
typedef struct {
    uint32_t center_freq_20m; /* 主20M的频点 */
    uint32_t center_freq1; /* 整个带宽的频点 */
    uint32_t center_freq2; /* 80+80下使用 */
    uint8_t bandwidth; /* 带宽 */
} mac_kernel_channel_stru;
#endif

#endif /* end of mac_frame.h */
