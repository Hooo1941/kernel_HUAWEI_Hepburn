/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递vap事件 的头文件
 * 创建日期   : 2022年9月16日
 */

#ifndef HD_EVENT_USER_H
#define HD_EVENT_USER_H

#include "mac_common.h"
#include "wlan_spec.h"

/* 用户信息相关的配置命令参数 */
typedef struct {
    uint16_t us_user_idx; /* 用户索引 */
    uint8_t auc_reserve[NUM_2_BYTES];
} mac_cfg_user_info_param_stru; /* hd_event */

/* m2s user信息结构体 */
typedef struct {
    uint16_t              us_user_idx;
    wlan_nss_enum_uint8     en_user_max_cap_nss;                    /* 用户自身空间流能力 */
    wlan_nss_enum_uint8     en_avail_num_spatial_stream;            /* Tx和Rx支持Nss的交集,供算法调用 */

    wlan_nss_enum_uint8     en_avail_bf_num_spatial_stream;         /* 用户支持的Beamforming空间流个数 */
    wlan_bw_cap_enum_uint8  en_avail_bandwidth;                     /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8  en_cur_bandwidth;                       /* 默认值与en_avail_bandwidth相同,供算法调用修改 */
    uint8_t en_cur_smps_mode;     /* wlan_mib_mimo_power_save_enum */
    wlan_nss_enum_uint8     en_smps_opmode_nodify_nss;              /* opmode 或者smps 通知修改的nss值 */
    uint8_t auc_rsv[3];                                             // 3代表保留字节， 结构体4字节对齐
} mac_user_m2s_stru; /* hd_event */

typedef struct {
    uint8_t uc_qos_info;   /* 关联请求中的WMM IE的QOS info field */
    uint8_t uc_max_sp_len; /* 从qos info字段中提取翻译的最大服务长度 */
    uint8_t auc_resv[2]; // 2代表保留字节， 4字节对齐
    uint8_t uc_ac_trigger_ena[WLAN_WME_AC_BUTT]; /* 4个AC的trigger能力 */
    uint8_t uc_ac_delievy_ena[WLAN_WME_AC_BUTT]; /* 4个AC的delivery能力 */
} mac_user_uapsd_status_stru; /* hd_event */

#pragma pack(push, 1)
typedef struct {
    uint16_t user_id;
    uint8_t uapsd_flag;
    mac_user_uapsd_status_stru mac_user_uapsd_status;
} h2d_event_user_uapsd_info; /* AP同步uapsd参数，mp13/mp15 rom化，需要按字节对齐 */
#pragma pack(pop)

typedef struct {
    uint16_t us_user_idx;
    uint8_t uc_arg1;
    uint8_t uc_arg2;

    /* 协议模式信息 */
    wlan_protocol_enum_uint8 en_cur_protocol_mode;
    wlan_protocol_enum_uint8 en_protocol_mode;
    uint8_t en_avail_protocol_mode; /* 用户和VAP协议模式交集, 供算法调用 */

    wlan_bw_cap_enum_uint8 en_bandwidth_cap;   /* 用户带宽能力信息 */
    wlan_bw_cap_enum_uint8 en_avail_bandwidth; /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8 en_cur_bandwidth;   /* 默认值与en_avail_bandwidth相同,供算法调用修改 */

    oal_bool_enum_uint8 en_user_pmf;
    mac_user_asoc_state_enum_uint8 en_user_asoc_state; /* 用户关联状态 */
#ifdef _PRE_WLAN_FEATURE_11AX
    uint8_t   bit_in_htc_blacklist   : 1, /* 当前user是否在htc blacklist表里 */
              bit_rev                : 7;
    uint8_t   rsv[3]; /* 3为数组大小 */
#endif
} mac_h2d_usr_info_stru; /* hd_event */

typedef struct {
    mac_user_cap_info_stru st_user_cap_info; /* 用户能力信息 */
    mac_hisi_cap_vendor_ie_stru hisi_priv_cap;  /* 用户私有能力 */
    uint16_t us_user_idx;
    uint16_t amsdu_maxsize; /* amsdu的最大字节长度 */
    uint32_t ampdu_maxsize; /* ampdu的最大字节长度 */
#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_hiex_cap_stru st_hiex_cap;
#endif
} mac_h2d_usr_cap_stru; /* hd_event */

/* 问题单 hmac向dmac同步速率集合信息时，
   使用的结构体大小超出了事件内存的大小，导致事件同步有可能失败。
   单独定义结构体mac_h2d_user_rate_info_stru,保证mac_h2d_usr_info_stru不超过限制 */
typedef struct {
    uint16_t us_user_idx;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    mac_he_hdl_stru st_he_hdl;
#endif
    /* vht速率集信息 */
    mac_vht_hdl_stru st_vht_hdl;

    /* ht速率集信息 */
    mac_user_ht_hdl_stru st_ht_hdl;

    /* legacy速率集信息 */
    uint8_t uc_avail_rs_nrates;
    uint8_t auc_avail_rs_rates[WLAN_MAX_SUPP_RATES];

    wlan_protocol_enum_uint8 en_protocol_mode; /* 用户协议模式 */
} mac_h2d_usr_rate_info_stru; /* hd_event */

typedef struct {
    uint16_t us_user_idx;
    mac_user_asoc_state_enum_uint8 en_asoc_state;
    uint8_t uc_rsv[1];
} mac_h2d_user_asoc_state_stru; /* hd_event */

/* 空间流信息结构体 */
typedef struct {
    uint16_t            us_user_idx;
    wlan_nss_enum_uint8   en_avail_num_spatial_stream;            /* Tx和Rx支持Nss的交集,供算法调用 */
    wlan_nss_enum_uint8   en_user_max_cap_nss;                    /* 用户支持的空间流个数 */
    wlan_nss_enum_uint8   en_user_num_spatial_stream_160m;        /* 用户带宽为160M时支持的空间流个数 */
    wlan_nss_enum_uint8   en_smps_opmode_notify_nss;              /* opmode 或者smps 通知修改的nss值  */
    uint8_t             auc_reserv[2];                            // 2代表保留字节， 4字节对齐
} mac_user_nss_stru; /* hd_event */

#ifdef _PRE_WLAN_DELAY_STATISTIC
#define DL_TIME_ARRAY_LEN 10
#define TID_DELAY_STAT    0
#define AIR_DELAY_STAT    1
typedef struct {
    uint16_t dl_time_array[DL_TIME_ARRAY_LEN];
    uint8_t dl_measure_type;
} user_delay_info_stru; /* hd_event */

typedef struct {
    uint32_t dmac_packet_count_num;
    uint32_t dmac_report_interval;
    uint8_t dmac_stat_enable;
    uint8_t reserved[NUM_3_BYTES];
} user_delay_switch_stru;
#endif

/* 添加用户事件payload结构体 */
typedef struct {
    uint16_t us_user_idx; /* 用户index */
    uint8_t auc_user_mac_addr[WLAN_MAC_ADDR_LEN];
    uint16_t us_sta_aid;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];

    mac_vht_hdl_stru st_vht_hdl;
    mac_user_ht_hdl_stru st_ht_hdl;
    mac_ap_type_enum_uint16 en_ap_type;
    int8_t c_rssi; /* 用户bss的信号强度 */
    uint8_t lut_index;
} dmac_ctx_add_user_stru; /* hd_event */

/* 删除用户事件结构体 */
typedef struct {
    uint16_t us_user_idx; /* 用户index */
    uint8_t auc_user_mac_addr[WLAN_MAC_ADDR_LEN];
    mac_ap_type_enum_uint16 en_ap_type;
    uint8_t auc_resv[NUM_2_BYTES]; /* , 修复删除del新增成员，结构大小尽量不要超过event第一级内存72字节 */
} dmac_ctx_del_user_stru; /* hd_event */

/* d2h lut信息同步 */
typedef struct {
    uint16_t usr_idx;
    uint8_t  lut_idx;
    uint8_t  hal_dev_id;
    uint8_t  valid;
} d2h_usr_lut_info_syn_event; /* hd_event */

#endif /* end of hd_event_user.h */
