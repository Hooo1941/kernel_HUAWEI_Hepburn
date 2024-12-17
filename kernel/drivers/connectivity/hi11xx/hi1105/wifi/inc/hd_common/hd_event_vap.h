/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递vap事件 的头文件
 * 创建日期   : 2022年9月16日
 */

#ifndef HD_EVENT_VAP_H
#define HD_EVENT_VAP_H

#include "mac_common.h"
#include "wlan_spec.h"

#define HD_EVENT_RF_NUM 4
#define HD_EVENT_SLAVE_RF_NUM 2

/* p2p结构中包含此状态成员，该结构挂在mac device下，此VAP状态枚举移动到mac_device.h中 */
/* VAP状态机，AP STA共用一个状态枚举 */
typedef enum {
    /* ap sta公共状态 */
    MAC_VAP_STATE_INIT = 0,
    MAC_VAP_STATE_UP = 1,    /* VAP UP */
    MAC_VAP_STATE_PAUSE = 2, /* pause , for ap &sta */

    /* ap 独有状态 */
    MAC_VAP_STATE_AP_WAIT_START = 3,

    /* sta独有状态 */
    MAC_VAP_STATE_STA_FAKE_UP = 4,
    MAC_VAP_STATE_STA_WAIT_SCAN = 5,
    MAC_VAP_STATE_STA_SCAN_COMP = 6,
    MAC_VAP_STATE_STA_JOIN_COMP = 7,
    MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2 = 8,
    MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4 = 9,
    MAC_VAP_STATE_STA_AUTH_COMP = 10,
    MAC_VAP_STATE_STA_WAIT_ASOC = 11,
    MAC_VAP_STATE_STA_OBSS_SCAN = 12,
    MAC_VAP_STATE_STA_BG_SCAN = 13,
    MAC_VAP_STATE_STA_LISTEN = 14, /* p2p0 监听 */
    MAC_VAP_STATE_ROAMING = 15,    /* 漫游 */
    MAC_VAP_STATE_BUTT,
} mac_vap_state_enum;
typedef uint8_t mac_vap_state_enum_uint8;

#ifdef _PRE_WLAN_EXPORT
typedef enum {
    WAL_AUTORATE_CFG_ENABLE,
    WAL_AUTORATE_CFG_LEVEL,
    WAL_AUTORATE_CFG_PARAM_BUTT
} wal_autorate_cfg_enum;
#endif

/* MAC vap能力特性标识 */
typedef struct {
    uint32_t bit_uapsd : 1,
               bit_txop_ps : 1,
               bit_wpa : 1,
               bit_wpa2 : 1,
               bit_dsss_cck_mode_40mhz : 1, /* 是否允许在40M上使用DSSS/CCK, 1-允许, 0-不允许 */
               bit_rifs_tx_on : 1,
               bit_tdls_prohibited : 1,                /* tdls全局禁用开关， 0-不关闭, 1-关闭 */
               bit_tdls_channel_switch_prohibited : 1, /* tdls信道切换全局禁用开关， 0-不关闭, 1-关闭 */
               bit_hide_ssid : 1,                      /* AP开启隐藏ssid,  0-关闭, 1-开启 */
               bit_wps : 1,                            /* AP WPS功能:0-关闭, 1-开启 */
               bit_11ac2g : 1,                         /* 2.4G下的11ac:0-关闭, 1-开启 */
               bit_keepalive : 1,                      /* vap KeepAlive功能开关: 0-关闭, 1-开启 */
               bit_smps : 2,                           /* vap 当前SMPS能力(暂时不使用) */
               bit_dpd_enbale : 1,                     /* dpd是否开启 */
               bit_dpd_done : 1,                       /* dpd是否完成 */
               bit_11ntxbf : 1,                        /* 11n txbf能力 */
               bit_disable_2ght40 : 1,                 /* 2ght40禁止位，1-禁止，0-不禁止 */
               bit_peer_obss_scan : 1,                 /* 对端支持obss scan能力: 0-不支持, 1-支持 */
               bit_1024qam : 1,                        /* 对端支持1024qam能力: 0-不支持, 1-支持 */ /* mp16 不使用 */
               bit_wmm_cap : 1,                        /* 保存与STA关联的AP是否支持wmm能力信息 */
               bit_is_interworking : 1,                /* 保存与STA关联的AP是否支持interworking能力 */
               bit_ip_filter : 1,                      /* rx方向ip包过滤的功能 */
               bit_opmode : 1,                         /* 对端的probe rsp中的extended capbilities 是否支持OPMODE */
               bit_nb : 1,                             /* 硬件是否支持窄带 */
               bit_2040_autoswitch : 1,                /* 是否支持随环境自动2040带宽切换 */
               bit_2g_custom_siso : 1,                 /* 2g是否定制化单天线siso,默认等于0,初始双天线 */
               bit_5g_custom_siso : 1,                 /* 5g是否定制化单天线siso,默认等于0,初始双天线 */
               bit_bt20dbm : 1,                        /* 20dbm是否使能，用于host做sta做删聚合判断 */
               bit_icmp_filter : 1,                    /* rx方向icmp的过滤filter */
               bit_dcm : 1,                            /* 11ax dcm能力 */
               bit_rx_stbc : 1;                        /* 关联的bss 支持stbc */
} mac_cap_flag_stru; /* hd_event */

typedef struct {
    oal_bool_enum_uint8 en_11ax_cap; /* 是否支持11ax能力 */
    oal_bool_enum_uint8 en_radar_detector_cap;
    oal_bool_enum_uint8 en_11n_sounding;
    wlan_bw_cap_enum_uint8 en_wlan_bw_max;

    uint16_t us_beacon_period;
    oal_bool_enum_uint8 en_green_field;
    oal_bool_enum_uint8 en_mu_beamformee_cap;

    oal_bool_enum_uint8 en_txopps_is_supp;
    oal_bool_enum_uint8 uc_su_bfee_num;
    oal_bool_enum_uint8 en_40m_shortgi;
    oal_bool_enum_uint8 en_11n_txbf;

    oal_bool_enum_uint8 en_40m_enable;
    uint8_t uc_bfer_num_sounding_dim;
    oal_bool_enum_uint8 en_su_bfee;
#ifdef _PRE_WLAN_FEATURE_11AX
    uint8_t uc_he_ntx_sts_below_80m;
    uint8_t uc_he_ntx_sts_over_80m;
    uint8_t uc_he_num_dim_below_80m;
    uint8_t uc_he_num_dim_over_80m;
#endif
    uint8_t uc_rsv[1];
} mac_d2h_mib_update_info_stru; /* hd_event */

/* 用于同步保护相关的参数 */
typedef struct {
    uint8_t en_dot11HTProtection; /* wlan_mib_ht_protection_enum */
    oal_bool_enum_uint8 en_dot11RIFSMode;
    oal_bool_enum_uint8 en_dot11LSIGTXOPFullProtectionActivated;
    oal_bool_enum_uint8 en_dot11NonGFEntitiesPresent;

    mac_protection_stru st_protection;
} mac_h2d_protection_stru; /* hd_event */

typedef struct {
    uint32_t length;
    uint32_t aul_dieid[WLAN_DIEID_MAX_LEN];
} mac_cfg_show_dieid_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    int8_t c_rssi;
    int8_t c_free_power;
} mac_cfg_query_rssi_stru; /* hd_event */

typedef struct {
    uint16_t user_id;
    uint16_t total_mpdu_num;
} mac_cfg_query_tid_queue_stru; /* hd_event */

typedef struct {
    uint8_t uc_auth_req_st;
    uint8_t uc_asoc_req_st;
    uint8_t auc_resv[NUM_2_BYTES];
} mac_cfg_query_mngpkt_sendstat_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    uint8_t uc_ps_st;
    uint8_t auc_resv[1];
} mac_cfg_query_psst_stru; /* hd_event */

#ifdef _PRE_WLAN_DFT_STAT
typedef struct {
    uint8_t uc_device_distance;
    uint8_t uc_intf_state_cca;
    uint8_t uc_intf_state_co;
    uint8_t auc_resv[1];
} mac_cfg_query_ani_stru; /* hd_event */
#endif

typedef struct {
    uint16_t us_user_id;
#ifdef _PRE_WLAN_DFT_STAT
    uint8_t uc_cur_per;
    uint8_t uc_bestrate_per;
#else
    uint8_t auc_resv[NUM_2_BYTES];
#endif
    uint32_t tx_rate;     /* 当前发送速率 */
    uint32_t tx_rate_min; /* 一段时间内最小发送速率 */
    uint32_t tx_rate_max; /* 一段时间内最大发送速率 */
    uint32_t rx_rate;     /* 当前接收速率 */
    uint32_t rx_rate_min; /* 一段时间内最小接收速率 */
    uint32_t rx_rate_max; /* 一段时间内最大接收速率 */
} mac_cfg_query_rate_stru; /* hd_event */

typedef struct {
    uint16_t id;
    uint16_t resv;
    uint8_t new_rx_mode;
    uint8_t hal_device_id;
    uint8_t once_ddr_reset_smac;
    uint8_t rsv;
} mac_rx_switch_stru; /* hd_event */

typedef struct {
    uint16_t user_index;
    uint8_t mode;
} mac_vsp_cfg_stru;

/* 创建VAP参数结构体, 对应cfgid: WLAN_CFGID_ADD_VAP */
typedef struct {
    wlan_vap_mode_enum_uint8 en_vap_mode;
    uint8_t uc_cfg_vap_indx;
    uint16_t us_muti_user_id; /* 添加vap 对应的muti user index */

    uint8_t uc_vap_id;                  /* 需要添加的vap id */
    wlan_p2p_mode_enum_uint8 en_p2p_mode; /* 0:非P2P设备; 1:P2P_GO; 2:P2P_Device; 3:P2P_CL */

    uint8_t bit_11ac2g_enable : 1;
    uint8_t bit_disable_capab_2ght40 : 1;
    uint8_t bit_reserve : 6;
    uint8_t uc_dst_hal_dev_id;  // vap挂接的hal device id

    uint8_t bit_uapsd_enable : 1;
    uint8_t bit_reserve1 : 7;
    uint8_t probe_resp_enable;
    uint8_t probe_resp_status;
    uint8_t is_primary_vap;
#ifdef _PRE_WLAN_CHBA_MGMT
    uint8_t chba_mode; /* 指示该vap是否是chba_mode */
#else
    uint8_t resv2[1];
#endif
    uint8_t multi_user_lut_idx; /* 组播用户lut idx */
    oal_net_device_stru *pst_net_dev;
} mac_cfg_add_vap_param_stru; /* hd_event */
typedef mac_cfg_add_vap_param_stru mac_cfg_del_vap_param_stru; /* hd_event */

/* 启用VAP参数结构体 对应cfgid: WLAN_CFGID_START_VAP */
typedef struct {
    oal_bool_enum_uint8 en_mgmt_rate_init_flag; /* start vap时候，管理帧速率是否需要初始化 */
    uint8_t uc_protocol;
    uint8_t uc_band;
    uint8_t uc_bandwidth;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    uint8_t uc_chan_number;
    uint8_t auc_resv2[NUM_2_BYTES];
    int32_t l_ifindex;
    oal_net_device_stru *pst_net_dev; /* 此成员仅供Host(WAL&HMAC)使用，Device侧(DMAC&ALG&HAL层)不使用 */
} mac_cfg_start_vap_param_stru; /* hd_event */
typedef mac_cfg_start_vap_param_stru mac_cfg_down_vap_param_stru; /* hd_event */

typedef struct {
    uint16_t us_sta_aid;
    uint8_t uc_uapsd_cap;
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    oal_bool_enum_uint8 en_txop_ps;
#else
    uint8_t auc_resv[1];
#endif /* #ifdef _PRE_WLAN_FEATURE_TXOPPS */
    uint8_t bit_ap_chip_oui : 3,
              bit_resv : 5;
#ifdef _PRE_WLAN_FEATURE_DEGRADE_SWITCH
    wlan_nss_enum_uint8 degradation_nss;
#endif
    uint8_t auc_resv1[NUM_2_BYTES];
} mac_h2d_vap_info_stru; /* hd_event */


/* 设置mac地址参数 对应cfgid: WLAN_CFGID_STATION_ID */
typedef struct {
    uint8_t auc_station_id[WLAN_MAC_ADDR_LEN];
    wlan_p2p_mode_enum_uint8 en_p2p_mode;
    uint8_t auc_resv[1];
} mac_cfg_staion_id_param_stru; /* hd_event */

/* SSID参数 对应cfgid: WLAN_CFGID_SSID */
typedef struct {
    uint8_t uc_ssid_len;
    uint8_t auc_resv[NUM_2_BYTES];
    int8_t ac_ssid[WLAN_SSID_MAX_LEN];
} mac_cfg_ssid_param_stru; /* hd_event */

/* HOSTAPD 设置 Beacon 信息 */
typedef struct {
    int32_t l_interval;    /* beacon interval */
    int32_t l_dtim_period; /* DTIM period     */
    oal_bool_enum_uint8 en_privacy;
    uint8_t auc_rsn_ie[MAC_MAX_RSN_LEN];
    uint8_t auc_wpa_ie[MAC_MAX_RSN_LEN];
    oal_bool_enum_uint8 uc_hidden_ssid;
    oal_bool_enum_uint8 en_shortgi_20;
    oal_bool_enum_uint8 en_shortgi_40;
    oal_bool_enum_uint8 en_shortgi_80;
    wlan_protocol_enum_uint8 en_protocol;

    uint8_t uc_smps_mode;
    mac_beacon_operation_type_uint8 en_operation_type;
    uint8_t auc_resv1[NUM_2_BYTES];
} mac_beacon_param_stru; /* hd_event */

#ifdef _PRE_WLAN_CHBA_MGMT
/* HOSTAPD 设置 chba vap 信息 */
typedef struct {
    uint8_t mac_addr[WLAN_MAC_ADDR_LEN]; /*  */
    mac_channel_stru init_channel;
} mac_chba_param_stru; /* hd_event */
#endif

/* 用户相关的配置命令参数 */
typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /*  */
    oal_bool_enum_uint8 en_ht_cap;             /* ht能力 */
    uint8_t auc_resv[NUM_3_BYTES];
    uint16_t us_user_idx; /* 用户索引 */
} mac_cfg_add_user_param_stru; /* hd_event */
typedef mac_cfg_add_user_param_stru mac_cfg_del_user_param_stru; /* hd_event */

/* sniffer配置命令参数 */
typedef struct {
    uint8_t uc_sniffer_mode;                 /* sniffer模式 */
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* 需要过滤的 */
    uint8_t auc_resv[1];
} mac_cfg_sniffer_param_stru; /* hd_event */

/* 暂停tid配置命令参数 */
typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /*  */
    uint8_t uc_tid;
    uint8_t uc_is_paused;
} mac_cfg_pause_tid_param_stru; /* hd_event */

/* 暂停tid配置命令参数 */
typedef struct {
    uint8_t uc_aggr_tx_on;
    uint8_t uc_snd_type;
    mac_ampdu_switch_mode_enum_uint8 en_aggr_switch_mode;
    uint8_t uc_rsv;
} mac_cfg_ampdu_tx_on_param_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
typedef struct {
    mac_tcp_ack_buf_cfg_cmd_enum_uint8 en_cmd;
    oal_bool_enum_uint8 en_enable;
    uint8_t uc_timeout_ms;
    uint8_t uc_count_limit;
} mac_cfg_tcp_ack_buf_stru; /* hd_event */
#endif

#ifdef _PRE_WLAN_FEATURE_CSI
#define WLAN_CSI_BLACK_LIST_NUM 8
typedef struct {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* csi对应的MAC ADDR */
    oal_bool_enum_uint8 en_cfg_csi_on;         /* csi使能 */
    uint8_t uc_csi_bw;                       /* csi采集带宽 */
    uint8_t uc_csi_frame_type;               /* csi采集的帧类型 */
    uint8_t uc_csi_sample_period;            /* csi采样周期 */
    uint8_t uc_csi_phy_report_mode;          /* 配置CSI PHY的上报模式  */
    uint8_t auc_reserve[1];
} mac_cfg_csi_param_stru; /* hd_event */

typedef struct {
    uint8_t csi_black_list_num;
    uint8_t csi_black_list_mac_map[WLAN_CSI_BLACK_LIST_NUM][WLAN_MAC_ADDR_LEN];
} mac_csi_black_list_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
typedef struct {
    oal_switch_enum_uint8 en_wmm_ac_switch;
    oal_switch_enum_uint8 en_auth_flag; /* WMM AC认证开关 */
    uint16_t us_timeout_period;       /* 定时器超时时间 */
    uint8_t uc_factor;                /* 除数因子，medium_time/2^factor */
    uint8_t auc_rsv[NUM_3_BYTES];
} mac_cfg_wmm_ac_param_stru; /* hd_event */
#endif

/* 发送功率配置参数 */
typedef struct {
    mac_set_pow_type_enum_uint8 en_type;
    uint8_t uc_reserve;
    uint8_t auc_value[80];
} mac_cfg_set_tx_pow_param_stru; /* hd_event */

typedef struct {
    uint8_t uc_function_index;
    uint8_t auc_reserve[NUM_2_BYTES];
    mac_vap_config_dscr_frame_type_uint8 en_type; /* 配置的帧类型 */
    int32_t l_value;
} mac_cfg_set_dscr_param_stru; /* hd_event */

/* non-HT协议模式下速率配置结构体 */
typedef struct {
    wlan_legacy_rate_value_enum_uint8 en_rate;     /* 速率值 */
    wlan_phy_protocol_enum_uint8 en_protocol_mode; /* 对应的协议 */
    uint8_t auc_reserve[NUM_2_BYTES];                      /* 保留 */
} mac_cfg_non_ht_rate_stru; /* hd_event */

typedef struct {
    uint8_t uc_show_ip_addr : 4;          // show ip addr
    uint8_t uc_show_arpoffload_info : 4;  // show arpoffload 维测
} mac_cfg_arpoffload_info_stru; /* hd_event */

typedef struct {
    uint8_t uc_in_suspend;         // 亮暗屏
    uint8_t uc_arpoffload_switch;  // arpoffload开关
} mac_cfg_suspend_stru; /* hd_event */

typedef struct {
    uint16_t user_id;
    uint8_t ao_switch;
    uint8_t resv;
} mac_cfg_arpoffload_stru; /* hd_event */

#ifdef _PRE_WLAN_NARROW_BAND
typedef struct {
    oal_bool_enum_uint8 en_open;    /* 打开关闭此特性 */
    mac_narrow_bw_enum_uint8 en_bw; /* 1M,5M,10M */
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_cfg_narrow_bw_stru; /* hd_event */
#ifdef _PRE_WLAN_EXPORT
typedef struct {
    uint32_t value;
    uint8_t type; /* UPC, LPF, DBB */
    uint8_t uc_rsv[NUM_2_BYTES];
} mac_cfg_power_param_stru;
typedef struct {
    wal_autorate_cfg_enum en_type;
    uint8_t val;
} mac_cfg_set_autorate_cfg_stru;
typedef struct {
    uint8_t send_success_rate;
    int8_t up_level_rssi;
    mac_narrow_bw_enum_uint8 bandwith;
    uint32_t upc;
    uint8_t lpf;
    uint32_t dbb;
    uint8_t resv[2];
    uint32_t rate_level;
} mac_autorate_set;
typedef struct {
    uint8_t autorate_en;
    uint8_t max_rate_level;
    uint8_t current_rate_level;
    uint8_t change_freq_num;
    mac_autorate_set autorate_set[12];
} mac_cfg_set_autorate_param;
typedef struct {
    uint8_t is_connected;
    uint8_t rate_level;
    uint8_t channel;
    uint8_t bw;
    int8_t rssi;
    uint8_t succ_rate;
    uint8_t resv[2];
} nb_report_info;
#endif
#endif

typedef enum mac_vowifi_mkeep_alive_type {
    VOWIFI_MKEEP_ALIVE_TYPE_STOP = 0,
    VOWIFI_MKEEP_ALIVE_TYPE_START = 1,
    VOWIFI_MKEEP_ALIVE_TYPE_BUTT
} mac_vowifi_nat_keep_alive_type_enum;
typedef uint8_t mac_vowifi_nat_keep_alive_type_enum_uint8;

typedef struct {
    uint8_t uc_keep_alive_id;
    mac_vowifi_nat_keep_alive_type_enum_uint8 en_type;
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_vowifi_nat_keep_alive_basic_info_stru; /* hd_event */

typedef struct {
    mac_vowifi_nat_keep_alive_basic_info_stru st_basic_info;
    uint8_t auc_src_mac[WLAN_MAC_ADDR_LEN];
    uint8_t auc_dst_mac[WLAN_MAC_ADDR_LEN];
    uint32_t period_msec;
    uint16_t us_ip_pkt_len;
    uint8_t auc_rsv[NUM_2_BYTES];
    uint8_t auc_ip_pkt_data[NUM_4_BYTES];
} mac_vowifi_nat_keep_alive_start_info_stru; /* hd_event */

/* vowifi质量评估参数配置命令集合 */
typedef enum {
    VOWIFI_SET_MODE = 0,
    VOWIFI_GET_MODE,
    VOWIFI_SET_PERIOD,
    VOWIFI_GET_PERIOD,
    VOWIFI_SET_LOW_THRESHOLD,
    VOWIFI_GET_LOW_THRESHOLD,
    VOWIFI_SET_HIGH_THRESHOLD,
    VOWIFI_GET_HIGH_THRESHOLD,
    VOWIFI_SET_TRIGGER_COUNT,
    VOWIFI_GET_TRIGGER_COUNT,
    VOWIFI_SET_IS_SUPPORT,

    VOWIFI_CMD_BUTT
} mac_vowifi_cmd_enum;
typedef uint8_t mac_vowifi_cmd_enum_uint8;

/* vowifi质量评估参数配置命令结构体 */
typedef struct {
    mac_vowifi_cmd_enum_uint8 en_vowifi_cfg_cmd; /* 配置命令 */
    uint8_t uc_value;                          /* 配置值 */
    uint8_t auc_resv[NUM_2_BYTES];
} mac_cfg_vowifi_stru; /* hd_event */

/* Number of Cipher Suites Implemented */
#define WLAN_PAIRWISE_CIPHER_SUITES 5   /* 单播加密套件为5 */
#define WLAN_AUTHENTICATION_SUITES  5   /* 密钥管理套件为5 */

/* 解析内核配置的connect参数后，下发给驱动的connect参数 */
typedef struct {
    uint32_t wpa_versions;
    uint32_t group_suite;
    uint32_t aul_pair_suite[WLAN_PAIRWISE_CIPHER_SUITES];
    uint32_t aul_akm_suite[WLAN_AUTHENTICATION_SUITES];
    uint32_t group_mgmt_suite;
    uint16_t wpa_pair_suite_count;
    uint16_t resv;
} mac_crypto_settings_stru; /* hd_event */

typedef struct {
    oal_bool_enum_uint8 en_privacy;                /* 是否加密标志 */
    oal_nl80211_auth_type_enum_uint8 en_auth_type; /* 认证类型，OPEN or SHARE-KEY */
    uint8_t uc_wep_key_len;                      /* WEP KEY长度 */
    uint8_t uc_wep_key_index;                    /* WEP KEY索引 */
    uint8_t auc_wep_key[WLAN_WEP104_KEY_LEN];    /* WEP KEY密钥 */
    mac_nl80211_mfp_enum_uint8 en_mgmt_proteced;   /* 此条链接pmf是否使能 */
    wlan_pmf_cap_status_uint8 en_pmf_cap;          /* 设备pmf能力 */
    oal_bool_enum_uint8 en_wps_enable;
    mac_crypto_settings_stru st_crypto; /* 密钥套件信息 */
#ifdef _PRE_WLAN_FEATURE_11R
    uint8_t auc_mde[NUM_8_BYTES]; /* MD IE信息 */
#endif                    // _PRE_WLAN_FEATURE_11R
    int8_t c_rssi;      /* 关联AP的RSSI信息 */
    int8_t rssi[HD_EVENT_RF_NUM];     /* 4天线的rssi */
    oal_bool_enum_uint8 is_wapi_connect;
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_conn_security_stru; /* hd_event */

/* begin:mp13/mp15 rom使用 */
#define WLAN_PAIRWISE_CIPHER_SUITES_MP13 2
#define WLAN_AUTHENTICATION_SUITES_MP13  2
typedef struct {
    uint32_t wpa_versions;
    uint32_t group_suite;
    uint32_t aul_pair_suite[WLAN_PAIRWISE_CIPHER_SUITES_MP13];
    uint32_t aul_akm_suite[WLAN_AUTHENTICATION_SUITES_MP13];
    uint32_t group_mgmt_suite;
} mac_crypto_settings_stru_mp13; /* hd_event */

typedef struct {
    oal_bool_enum_uint8 en_privacy;                /* 是否加密标志 */
    oal_nl80211_auth_type_enum_uint8 en_auth_type; /* 认证类型，OPEN or SHARE-KEY */
    uint8_t uc_wep_key_len;                      /* WEP KEY长度 */
    uint8_t uc_wep_key_index;                    /* WEP KEY索引 */
    uint8_t auc_wep_key[WLAN_WEP104_KEY_LEN];    /* WEP KEY密钥 */
    mac_nl80211_mfp_enum_uint8 en_mgmt_proteced;   /* 此条链接pmf是否使能 */
    wlan_pmf_cap_status_uint8 en_pmf_cap;          /* 设备pmf能力 */
    oal_bool_enum_uint8 en_wps_enable;
    mac_crypto_settings_stru_mp13 st_crypto; /* 密钥套件信息 */ /* mp13/mp15 rom使用 */
#ifdef _PRE_WLAN_FEATURE_11R
    uint8_t auc_mde[NUM_8_BYTES]; /* MD IE信息 */
#endif                    // _PRE_WLAN_FEATURE_11R
    int8_t c_rssi;      /* 关联AP的RSSI信息 */
    int8_t rssi[HD_EVENT_RF_NUM];     /* 4天线的rssi */
    oal_bool_enum_uint8 is_wapi_connect;
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_conn_security_stru_mp13; /* hd_event */
/* end:mp13/mp15 rom使用 */
typedef struct {
    uint8_t uc_key_index;
    oal_bool_enum_uint8 en_pairwise;
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN];
    mac_key_params_stru st_key;
} mac_addkey_param_stru; /* hd_event */

typedef struct {
    uint8_t uc_key_index;
    oal_bool_enum_uint8 en_pairwise;
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN];
} mac_removekey_param_stru; /* hd_event */

/****************************************************************************************/
/* dot11QAPEDCATable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11QAPEDCAEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Conceptual table for EDCA default parameter values at the AP. This table       */
/*      contains the four entries of the EDCA parameters corresponding to four          */
/*      possible ACs. Index 1 corresponds to AC_BK, index 2 to AC_BE, index 3 to        */
/*      AC_VI, and index 4 to AC_VO."                                                   */
/*      REFERENCE                                                                       */
/*      "IEEE 802.11-<year>, 9.19.2"                                                    */
/* ::= { dot11mac 5 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11QAPEDCATableIndex;              /* dot11QAPEDCATableIndex    Unsigned32 */
    uint32_t dot11QAPEDCATableCWmin;              /* dot11QAPEDCATableCWmin Unsigned32 */
    uint32_t dot11QAPEDCATableCWmax;              /* dot11QAPEDCATableCWmax Unsigned32 */
    uint32_t dot11QAPEDCATableAIFSN;              /* dot11QAPEDCATableAIFSN Unsigned32 */
    uint32_t dot11QAPEDCATableTXOPLimit;          /* dot11QAPEDCATableTXOPLimit Unsigned32 */
    uint32_t dot11QAPEDCATableMSDULifetime;       /* dot11QAPEDCATableMSDULifetime Unsigned32 */
    oal_bool_enum_uint8 en_dot11QAPEDCATableMandatory; /* dot11QAPEDCATableMandatory TruthValue */
} wlan_mib_Dot11QAPEDCAEntry_stru; /* 注意：该结构为host/device抛事件使用,host/device需要采用相同结构体 */

#endif /* end of hd_event_vap.h */
