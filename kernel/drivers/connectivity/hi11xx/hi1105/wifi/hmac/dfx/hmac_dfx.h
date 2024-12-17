/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : DFX文件接扩
 * 创建日期 : 2014年4月10日
 */

#ifndef HMAC_DFX_H
#define HMAC_DFX_H

#include "platform_spec.h"
#include "hmac_vap.h"

/* 1 头文件包含 */
/* 2 宏定义 */
#define HMAC_CHR_NETBUF_ALLOC_SIZE  512
/* chr周期上报数据事件的频次:30s */
#define HMAC_CHR_REPORT_INTERVAL 30000

#define HMAC_CHR_INFO_STRU_SIZE 20
/* 3 枚举定义 */
typedef enum {
    HMAC_CHR_ROAM_SUCCESS = 0,
    HMAC_CHR_ROAM_SCAN_FAIL = 1,
    HMAC_CHR_ROAM_HANDSHAKE_FAIL = 2,
    HMAC_CHR_ROAM_CONNECT_FAIL = 3,
    HMAC_CHR_ROAM_TIMEOUT_FAIL = 4,
    HMAC_CHR_ROAM_START = 5,

    HMAC_CHR_ROAM_REASON_BUTT
} hmac_chr_roam_fail_reason;

typedef enum {
    HMAC_CHR_ROAM_NORMAL = 0,
    HMAC_CHR_OVER_DS = 1,
    HMAC_CHR_OVER_THE_AIR = 2,

    HMAC_CHR_ROAM_MODE_BUTT
} hmac_chr_roam_mode;

typedef enum {
    HMAC_CHR_NORMAL_SCAN = 0,
    HMAC_CHR_11K_SCAN = 1,
    HMAC_CHR_11V_SCAN = 2,

    HMAC_CHR_SCAN_MODE_BUTT
} hmac_chr_scan_mode;

/* 4 全局变量声明 */
typedef struct {
    uint8_t uc_vap_state;
    uint8_t uc_vap_num;
    uint8_t uc_protocol;
    uint8_t uc_vap_rx_nss;
    uint8_t uc_ap_protocol_mode;
    uint8_t uc_ap_spatial_stream_num;
    uint8_t bit_ampdu_active : 1;
    uint8_t bit_amsdu_active : 1;
    uint8_t bit_is_dbac_running : 1;
    uint8_t bit_is_dbdc_running : 1;
    uint8_t bit_sta_11ntxbf : 1;
    uint8_t bit_ap_11ntxbf : 1;
    uint8_t bit_ap_qos : 1;
    uint8_t bit_ap_1024qam_cap : 1;
} hmac_chr_vap_info_stru;

#ifdef _PRE_WLAN_CHBA_MGMT
typedef struct {
    uint8_t chba_mode;
    uint8_t sync_state;
    uint8_t chba_module_state;
    uint8_t role;
    uint8_t island_device_cnt;
    uint8_t vap_bitmap_level; /* vap低功耗bitmap */
    uint8_t social_channel_number;
    uint8_t work_channel_number; /* 记录岛工作信道 */
    uint8_t second_work_channel_num; /* 记录CHBA第二个工作信道 */
    uint32_t channel_sequence_bitmap;
} hmac_chr_chba_info_stru;
#endif

typedef struct tag_hmac_chr_ba_info_stru {
    uint8_t uc_ba_num;
    uint8_t uc_del_ba_tid;
    mac_reason_code_enum_uint16 en_del_ba_reason;
} hmac_chr_del_ba_info_stru;

typedef struct tag_hmac_chr_disasoc_reason_stru {
    uint16_t us_user_id;
    dmac_disasoc_misc_reason_enum_uint16 en_disasoc_reason;
} hmac_chr_disasoc_reason_stru;
typedef struct tag_hmac_chr_rx_info_stru {
    uint32_t rx_exception_free_cnt[WLAN_DEVICE_MAX_NUM_PER_CHIP];
    uint32_t rx_mode[WLAN_DEVICE_MAX_NUM_PER_CHIP];
    uint32_t total_rx_packets;
    uint32_t rx_pps;
    uint32_t rx_tcp_checksum_fail;
    uint32_t rx_ip_checksum_fail;
} hmac_chr_rx_info_stru;

typedef struct tag_hmac_chr_tx_ring_info_stru {
    uint8_t tx_ring_used;        /* tx ring使能标记 */
    uint8_t ring_tx_mode;
    uint16_t push_ring_full_cnt; /* 入ring满次数统计 */
    uint32_t tx_msdu_cnt;        /* 已发送msdu数 */
    uint16_t msdu_cnt;           /* 当前ring中msdu数 */
    uint16_t tx_pps;
} hmac_chr_tx_ring_info_stru;

typedef struct tag_hmac_chr_info_new_arch {
    uint8_t chr_version_flag;
    uint8_t cur_pcie_state; /* 对应hmac_pcie_state */
    hmac_chr_tx_ring_info_stru tx_ring_info[WLAN_TID_MAX_NUM - 1];
    hmac_chr_rx_info_stru rx_info;
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_chr_chba_info_stru chba_info;
#endif
} hmac_chr_info_new_arch;

typedef struct tag_hmac_chr_info {
    hmac_chr_disasoc_reason_stru st_disasoc_reason;
    hmac_chr_del_ba_info_stru st_del_ba_info;
    hmac_chr_vap_info_stru st_vap_info;
    uint16_t us_connect_code;
    uint8_t _resv[2];
} hmac_chr_info;

typedef struct tag_hmac_chr_connect_fail_report_stru {
    int32_t snr;
    int32_t noise;  /* 底噪 */
    int32_t chload; /* 信道繁忙程度 */
    int8_t c_signal;
    uint8_t uc_distance; /* 算法的tpc距离，对应dmac_alg_tpc_user_distance_enum */
    uint8_t uc_cca_intr; /* 算法的cca_intr干扰，对应alg_cca_opt_intf_enum */
    uint16_t us_err_code;
    uint8_t _resv[2]; // 保留2字节
} mac_chr_connect_fail_report_stru;

typedef struct {
    uint8_t uc_trigger;
    uint8_t uc_roam_result;
    uint8_t uc_scan_mode;
    uint8_t uc_roam_mode;
    uint32_t uc_scan_time;
    uint32_t uc_connect_time;
    uint8_t _resv[4]; // 保留4字节
} hmac_chr_roam_info_stru;

typedef struct {
    uint8_t auc_src_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t uc_src_channel;
    int8_t uc_src_rssi;
    uint8_t auc_target_bssid[WLAN_MAC_ADDR_LEN];
    uint8_t uc_target_channel;
    int8_t uc_target_rssi;
    uint8_t uc_roam_mode;
    uint8_t uc_roam_stage;
    uint8_t uc_roam_result;
    uint8_t uc_roam_type;
    uint32_t uc_roam_time;
} hmac_chr_roam_connect_info_stru;

#ifdef _PRE_WLAN_FEATURE_DFR
extern hmac_dfr_info_stru g_st_dfr_info; /* DFR异常复位开关 */
#endif

uint32_t hmac_dfx_init(void);
uint32_t hmac_dfx_exit(void);

hmac_chr_disasoc_reason_stru *hmac_chr_disasoc_reason_get_pointer(void);
uint16_t *hmac_chr_connect_code_get_pointer(void);
hmac_chr_del_ba_info_stru *hmac_chr_ba_info_get_pointer(void);
void hmac_chr_info_clean(void);
void hmac_chr_set_disasoc_reason(uint16_t user_id, uint16_t reason_id);
void hmac_chr_get_disasoc_reason(hmac_chr_disasoc_reason_stru *pst_disasoc_reason);
void hmac_chr_set_del_ba_info(uint8_t uc_tid, uint16_t reason_id);
void hmac_chr_get_del_ba_info(mac_vap_stru *pst_mac_vap, hmac_chr_del_ba_info_stru *pst_del_ba_reason);
void hmac_chr_set_ba_session_num(uint8_t uc_ba_num);
void hmac_chr_set_connect_code(uint16_t connect_code);
void hmac_chr_get_connect_code(uint16_t *pus_connect_code);
void hmac_chr_get_vap_info(mac_vap_stru *pst_mac_vap, hmac_chr_vap_info_stru *pst_vap_info);
uint32_t hmac_chr_get_chip_info(uint32_t chr_event_id);
uint32_t hmac_get_chr_info_event_hander(uint32_t chr_event_id);
void hmac_chr_connect_fail_query_and_report(hmac_vap_stru *pst_hmac_vap,
    mac_status_code_enum_uint16 connet_code);
void hmac_chr_mvap_report(mac_vap_stru *mac_vap, uint8_t scene);

#endif /* end of hmac_dfx.h */
