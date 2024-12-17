/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递chan_mgmt事件 的头文件
 * 创建日期   : 2022年9月16日
 */

#ifndef HD_EVENT_CHAN_MGMT_H
#define HD_EVENT_CHAN_MGMT_H

#include "mac_common.h"
#include "wlan_spec.h"

#define MAC_RD_INFO_LEN 12 /* mac_regdomain_info_stru去掉mac_regclass_info_stru的长度 */

/* 一个管制类的起始频率枚举 */
typedef enum {
    MAC_RC_START_FREQ_2 = WLAN_BAND_2G, /* 2.407 */
    MAC_RC_START_FREQ_5 = WLAN_BAND_5G, /* 5 */

    MAC_RC_START_FREQ_BUTT,
} mac_rc_start_freq_enum;
typedef uint8_t mac_rc_start_freq_enum_uint8;

/* 管制类信道间距 */
typedef enum {
    MAC_CH_SPACING_5MHZ = 0,
    MAC_CH_SPACING_10MHZ,
    MAC_CH_SPACING_20MHZ,
    MAC_CH_SPACING_25MHZ,
    MAC_CH_SPACING_40MHZ,
    MAC_CH_SPACING_80MHZ,

    MAC_CH_SPACING_BUTT
} mac_ch_spacing_enum;
typedef uint8_t mac_ch_spacing_enum_uint8;

/* 雷达认证标准枚举 */
typedef enum {
    MAC_DFS_DOMAIN_NULL = 0,
    MAC_DFS_DOMAIN_FCC = 1,
    MAC_DFS_DOMAIN_ETSI = 2,
    MAC_DFS_DOMAIN_MKK = 3,
    MAC_DFS_DOMAIN_KOREA = 4,
    MAC_DFS_DOMAIN_CN = 5,

    MAC_DFS_DOMAIN_BUTT
} mac_dfs_domain_enum;
typedef uint8_t mac_dfs_domain_enum_uint8;

typedef struct {
    uint16_t us_num_networks;
    mac_ch_type_enum_uint8 en_ch_type;
#ifdef _PRE_WLAN_FEATURE_DFS
    mac_chan_status_enum_uint8 en_ch_status;
#else
    uint8_t auc_resv[1];
#endif
} mac_ap_ch_info_stru; /* hd_event */

/* host/device管制域同步结构体 */
typedef struct {
    mac_rc_start_freq_enum_uint8 en_start_freq; /* 起始频率 */
    mac_ch_spacing_enum_uint8 en_ch_spacing;    /* 信道间距 */
    uint8_t uc_behaviour_bmap;                /* 允许的行为位图 位图定义见mac_behaviour_bmap_enum */
    uint8_t uc_coverage_class;                /* 覆盖类 */
    uint8_t uc_max_reg_tx_pwr;                /* 管制类规定的最大发送功率, 单位dBm */
    /* 实际使用的最大发送功率,扩大了10倍用于计算, 单位dBm，可以比管制域规定功率大 */
    uint16_t us_max_tx_pwr;
    uint8_t auc_resv[1];
    uint32_t channel_bmap; /* 支持信道位图，例 0011表示支持的信道的index为0 1 */
} mac_regclass_info_stru;

/* 管制域信息结构体 */
/* 管制类值、管制类位图与管制类信息 数组下表的关系
    管制类取值        : .... 7  6  5  4  3  2  1  0
    管制类位图        : .... 1  1  0  1  1  1  0  1
    管制类信息数组下标: .... 5  4  x  3  2  1  x  0
*/
typedef struct {
    int8_t ac_country[WLAN_COUNTRY_STR_LEN]; /* 国家字符串 */
    mac_dfs_domain_enum_uint8 en_dfs_domain;   /* DFS 雷达标准 */
    uint8_t uc_regclass_num;                 /* 管制类个数 */
    regdomain_enum_uint8 en_regdomain;
    uint8_t auc_resv[2]; /* 预留2字节 */
    mac_regclass_info_stru ast_regclass[WLAN_MAX_RC_NUM]; /* 管制域包含的管制类信息，注意 此成员只能放在最后一项! */
} mac_regdomain_info_stru; /* hd_event */

typedef struct {
    wlan_ch_switch_status_enum_uint8 en_ch_switch_status;     /* 信道切换状态 */
    uint8_t uc_announced_channel;                           /* 新信道号 */
    wlan_channel_bandwidth_enum_uint8 en_announced_bandwidth; /* 新带宽模式 */
    uint8_t uc_ch_switch_cnt;                               /* 信道切换计数 */
    oal_bool_enum_uint8 en_csa_present_in_bcn;                /* Beacon帧中是否包含CSA IE */
    uint8_t uc_csa_vap_cnt;                                 /* 需要发送csa的vap个数 */
    wlan_csa_mode_tx_enum_uint8 en_csa_mode;
    oal_bool_enum_uint8 en_go_csa_is_running;
    uint64_t go_csa_start_time;
} dmac_set_ch_switch_info_stru; /* hd_event */

typedef struct {
    mac_channel_stru st_channel;
    mac_ch_switch_info_stru st_ch_switch_info;

    oal_bool_enum_uint8 en_switch_immediately; /* 1 - 马上切换  0 - 暂不切换, 推迟到tbtt中切换 */
    oal_bool_enum_uint8 en_check_cac;
    oal_bool_enum_uint8 en_dot11FortyMHzIntolerant;
    uint8_t auc_resv[1];
} dmac_set_chan_stru; /* hd_event */

/* 带宽调试开关相关的结构体 */
typedef enum {
    MAC_CSA_FLAG_NORMAL = 0,
    MAC_CSA_FLAG_START_DEBUG, /* 固定csa ie 在beacon帧中 */
    MAC_CSA_FLAG_CANCLE_DEBUG,
    MAC_CSA_FLAG_GO_DEBUG,

    MAC_CSA_FLAG_BUTT
} mac_csa_flag_enum;
typedef uint8_t mac_csa_flag_enum_uint8;

typedef struct {
    wlan_csa_mode_tx_enum_uint8 en_mode;
    uint8_t uc_channel;
    uint8_t uc_cnt;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    mac_csa_flag_enum_uint8 en_debug_flag; /* 0:正常切信道; 1:仅beacon帧中含有csa,信道不切换;2:取消beacon帧中含有csa */
    uint8_t auc_reserv[NUM_3_BYTES];
} mac_csa_debug_stru;

typedef struct {
    uint32_t cmd_bit_map;
    oal_bool_enum_uint8 en_band_force_switch_bit0; /* 恢复40M带宽命令 */
    oal_bool_enum_uint8 en_2040_ch_swt_prohi_bit1; /* 不允许20/40带宽切换开关 */
    oal_bool_enum_uint8 en_40_intolerant_bit2;     /* 不容忍40M带宽开关 */
    uint8_t uc_resv;
    mac_csa_debug_stru st_csa_debug_bit3;
    oal_bool_enum_uint8 en_lsigtxop_bit5; /* lsigtxop使能 */
    uint8_t auc_resv0[NUM_3_BYTES];
} mac_protocol_debug_switch_stru; /* hd_event */

typedef struct {
    mac_channel_stru st_channel;
    uint16_t us_user_idx;
    wlan_bw_cap_enum_uint8 en_bandwidth_cap;   /* 用户带宽能力信息 */
    wlan_bw_cap_enum_uint8 en_avail_bandwidth; /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8 en_cur_bandwidth;   /* 默认值与en_avail_bandwidth相同,供算法调用修改 */
    uint8_t auc_rsv[3];                        // 3代表保留字节， 4字节对齐
} mac_d2h_syn_info_stru; /* hd_event */

/* HOSTAPD 设置工作频段，信道和带宽参数 */
typedef struct {
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
    wlan_channel_band_enum_uint8 en_band;           /* 频带 */
#else
    uint8_t en_band : 4,
            ext6g_band : 1, /* 指示是否为6G频段标志 */
            resv : 3;
#endif
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    uint8_t uc_channel;                           /* 信道编号 */
    uint8_t auc_resv[1];                          /* 保留位 */
} mac_cfg_channel_param_stru; /* hd_event */

/* 用于同步带宽切换的参数 */
typedef struct {
    wlan_channel_bandwidth_enum_uint8 en_40m_bandwidth;
    oal_bool_enum_uint8 en_40m_intol_user;
    uint8_t auc_resv[NUM_2_BYTES];
} mac_bandwidth_stru; /* hd_event */

/* 协议参数 对应cfgid: WLAN_CFGID_MODE */
typedef struct {
    wlan_protocol_enum_uint8 en_protocol;           /* 协议 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
    wlan_channel_band_enum_uint8 en_band;           /* 频带 */
#else
    uint8_t en_band : 4,                            /* 频带 */
            ext6g_band : 1,                         /* 指示是否为6G频段标志 */
            resv : 3;
#endif
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    uint8_t en_channel_idx;                       /* 主20M信道号 */
} mac_cfg_mode_param_stru; /* hd_event */

typedef struct {
    wlan_channel_band_enum_uint8 en_band;           /* 频带 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
} mac_cfg_mib_by_bw_param_stru; /* hd_event */

/* zero wait dfs 设置工作频段，信道和带宽参数,检测时间 */
typedef struct {
    mac_channel_stru channel_param;
    uint32_t radar_detect_time;
} zero_wait_dfs_param_stru; /* hd_event */

typedef struct {
    /* 时间统计类 */
    uint32_t rx_direct_time;
    uint32_t rx_nondir_time;
    uint32_t tx_time;
    uint32_t free_time;
    uint32_t abort_time_us;

    /* 干扰繁忙度 */
    uint16_t duty_cyc_ratio_20; /* 20M干扰繁忙度 */
    uint16_t duty_cyc_ratio_40; /* 40M干扰繁忙度 */
    uint16_t duty_cyc_ratio_80; /* 80M干扰繁忙度 */
    uint16_t duty_cyc_ratio_160; /* 160M干扰繁忙度 */

    /* rssi统计值 */
    int8_t intf_det_avg_rssi_20;           /* pri20 RSSI平均值 */
    int8_t intf_det_avg_rssi_40;           /* pri40 RSSI平均值 */
    int8_t intf_det_avg_rssi_80;           /* 80M RSSI平均值 */
    int8_t intf_det_avg_rssi_160;          /* 160M RSSI平均值 */

    /* per20M的空闲时间和空闲功率统计 */
    int8_t per20m_idle_pwr[8]; /* 一共160m 每个20M空闲功率统计上报 8 */
    uint32_t per20m_idle_time[8]; /* 一共160m 每个20M空闲功率时间统计上报) 8 */

    /* 信道繁忙度 */
    uint16_t chan_ratio[7]; /* 索引的枚举定义:wlan_channel_enum */
    int8_t free_power[7];
    uint8_t resv[3];   /* 对齐  3 */
} d2h_chan_det_rpt_sttu;

typedef struct {
    d2h_chan_det_rpt_sttu chan_rpt;
    uint32_t mac_free_time[7];
    uint32_t phy_free_time[7];
} chan_det_rpt_stru;

#endif /* end of hd_event_chan_mgmt.h */
