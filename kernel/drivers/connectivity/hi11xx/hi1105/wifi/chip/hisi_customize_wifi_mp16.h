/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi定制化头文件声明
 * 作者       : wifi
 * 创建日期   : 2015年10月08日
 */

#ifndef HISI_CUSTOMIZE_WIFI_MP16_H
#define HISI_CUSTOMIZE_WIFI_MP16_H

/* 其他头文件包含 */
#include "hisi_customize_wifi.h"
#include "wlan_customize.h"
#include "hisi_customize_wifi_cap.h"
#include "hisi_customize_wifi_pow.h"
#include "hisi_customize_wifi_dyn_pow.h"
#include "hisi_conn_nve_interface.h"
/* 宏定义 */
#define CUS_TAG_HOST                 0x21
#define CUS_TAG_RF                   0X22
#define CUS_TAG_DYN_POW              0x23
#define CUS_TAG_POW                  0X24
#define CUS_TAG_CAP                  0x25

/* 用户pwr ref reg的定制化保护阈值 */
#define CUST_PWR_REF_DELTA_HI 40
#define CUST_PWR_REF_DELTA_LO (-40)

#define CUST_SAR_SLIDE_TIME_MAX (400)
#define CUST_SAR_SLIDE_TIME_MIN (40)
#define CUST_SAR_SLIDE_POW_AMEND_MAX (50)
#define CUST_SAR_SLIDE_POW_AMEND_MIN (-50)

/* 枚举定义 */
/* 定制化 HOST CONFIG ID */
typedef enum {
    /* ROAM */
    WLAN_CFG_HOST_ROAM_SWITCH = 0,
    WLAN_CFG_HOST_SCAN_ORTHOGONAL,
    WLAN_CFG_HOST_TRIGGER_B,
    WLAN_CFG_HOST_TRIGGER_A,
    WLAN_CFG_HOST_DELTA_B,
    WLAN_CFG_HOST_DELTA_A,
    WLAN_CFG_HOST_DENSE_ENV_TRIGGER_B,
    WLAN_CFG_HOST_DENSE_ENV_TRIGGER_A,
    WLAN_CFG_HOST_SCENARIO_ENABLE,
    WLAN_CFG_HOST_CANDIDATE_GOOD_RSSI,
    WLAN_CFG_HOST_CANDIDATE_GOOD_NUM,
    WLAN_CFG_HOST_CANDIDATE_WEAK_NUM,
    WLAN_CFG_HOST_INTERVAL_VARIABLE,

    /* 性能 */
    WLAN_CFG_HOST_AMPDU_TX_MAX_NUM,  // 7

    WLAN_CFG_HOST_DMA_LATENCY,
    WLAN_CFG_HOST_LOCK_CPU_TH_HIGH,
    WLAN_CFG_HOST_LOCK_CPU_TH_LOW,
    WLAN_CFG_HOST_IRQ_AFFINITY,
    WLAN_CFG_HOST_IRQ_TH_LOW,
    WLAN_CFG_HOST_IRQ_TH_HIGH,
    WLAN_CFG_HOST_IRQ_PPS_TH_LOW,
    WLAN_CFG_HOST_IRQ_PPS_TH_HIGH,
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    WLAN_CFG_HOST_HW_AMPDU,
    WLAN_CFG_HOST_HW_AMPDU_TH_LOW,
    WLAN_CFG_HOST_HW_AMPDU_TH_HIGH,
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    WLAN_CFG_HOST_AMPDU_AMSDU_SKB,
    WLAN_CFG_HOST_AMSDU_AMPDU_TH_LOW,
    WLAN_CFG_HOST_AMSDU_AMPDU_TH_MIDDLE,
    WLAN_CFG_HOST_AMSDU_AMPDU_TH_HIGH,
#endif
#ifdef _PRE_WLAN_TCP_OPT
    WLAN_CFG_HOST_TCP_ACK_FILTER,
    WLAN_CFG_HOST_TCP_ACK_FILTER_TH_LOW,
    WLAN_CFG_HOST_TCP_ACK_FILTER_TH_HIGH,
#endif
    WLAN_CFG_HOST_TX_SMALL_AMSDU,
    WLAN_CFG_HOST_SMALL_AMSDU_HIGH,
    WLAN_CFG_HOST_SMALL_AMSDU_LOW,
    WLAN_CFG_HOST_SMALL_AMSDU_PPS_HIGH,
    WLAN_CFG_HOST_SMALL_AMSDU_PPS_LOW,

    WLAN_CFG_HOST_TX_TCP_ACK_BUF,
    WLAN_CFG_DEVICE_TX_TCP_ACK_BUF,
    WLAN_CFG_HOST_TCP_ACK_BUF_HIGH,
    WLAN_CFG_HOST_TCP_ACK_BUF_LOW,
    WLAN_CFG_HOST_TCP_ACK_BUF_HIGH_40M,
    WLAN_CFG_HOST_TCP_ACK_BUF_LOW_40M,
    WLAN_CFG_HOST_TCP_ACK_BUF_HIGH_80M,
    WLAN_CFG_HOST_TCP_ACK_BUF_LOW_80M,
    WLAN_CFG_HOST_TCP_ACK_BUF_HIGH_160M,
    WLAN_CFG_HOST_TCP_ACK_BUF_LOW_160M,
    WLAN_CFG_HOST_TX_TCP_ACK_BUF_USERCTL,
    WLAN_CFG_HOST_TCP_ACK_BUF_USERCTL_HIGH,
    WLAN_CFG_HOST_TCP_ACK_BUF_USERCTL_LOW,

    WLAN_CFG_HOST_RX_DYN_BYPASS_EXTLNA,
    WLAN_CFG_HOST_RX_DYN_BYPASS_EXTLNA_HIGH,
    WLAN_CFG_HOST_RX_DYN_BYPASS_EXTLNA_LOW,

    /* 接收ampdu+amsdu */
    WLAN_CFG_HOST_RX_AMPDU_AMSDU_SKB,
    WLAN_CFG_HOST_RX_AMPDU_BITMAP,

    /* 低功耗 */
    WLAN_CFG_HOST_PS_MODE,
    /* SCAN */
    WLAN_CFG_HOST_RANDOM_MAC_ADDR_SCAN,
    /* 11AC2G */
    WLAN_CFG_HOST_11AC2G_ENABLE,
    /* 11ac2g开关 */                    // 56
    WLAN_CFG_HOST_DISABLE_CAPAB_2GHT40, /* 2ght40禁止开关 */

    /* probe req应答模式功能配置 */
    WLAN_CFG_HOST_PROBE_RESP_MODE, /* BIT7~BIT4:动态功能开关 ;BIT3~BIT0:功能模式 , 目前此配置仅针对p2p dev设备有效 */
    WLAN_CFG_HOST_MIRACAST_SINK_ENABLE, /* miracast_sink特性开关 */
    WLAN_CFG_HOST_REDUCE_MIRACAST_LOG, /* miracast log 降级开关 */
    WLAN_CFG_HOST_CORE_BIND_CAP, /* 是否支持cpu绑定 */

    /* sta keepalive */
    WLAN_CFG_HOST_LTE_GPIO_CHECK_SWITCH, /* lte?????? */
    WLAN_HOST_ATCMDSRV_ISM_PRIORITY,
    WLAN_HOST_ATCMDSRV_LTE_RX,
    WLAN_HOST_ATCMDSRV_LTE_TX,
    WLAN_HOST_ATCMDSRV_LTE_INACT,
    WLAN_HOST_ATCMDSRV_ISM_RX_ACT,
    WLAN_HOST_ATCMDSRV_BANT_PRI,
    WLAN_HOST_ATCMDSRV_BANT_STATUS,
    WLAN_HOST_ATCMDSRV_WANT_PRI,
    WLAN_HOST_ATCMDSRV_WANT_STATUS,
    /* 注册WiFi动态锁频麒麟接口 */
    WLAN_CFG_HOST_LOCK_CPU_FREQ,
    /* 优化特性开关 */
    WLAN_CFG_HOST_OPTIMIZED_FEATURE_SWITCH,
    WLAN_CFG_HOST_CHBA_INIT_SOCIAL_CHAN,
    WLAN_CFG_HOST_CHBA_SOCIAL_FOLLOW_WORK,
    WLAN_CFG_HOST_CHBA_DBAC_CHAN_FOLLOW_SWITCH,
    WLAN_CFG_HOST_EXTPAND_FEATURE_SWITCH_BITMAP,
    WLAN_CFG_HOST_REDUCE_PWR_2G_40MHZ_CHANNEL_BITMAP, /* 2G 40MHz降功率中心信道BITMAP */
    WLAN_CFG_HOST_BUTT,
} wlan_host_cfg_enum;

typedef enum {
    /* 动态校准 */
    WLAN_CFG_DYN_CALI_DSCR_ITERVL,
    WLAN_CFG_FAR_DIST_POW_GAIN_SWITCH,

    /* 2G RF前端 */
    WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND_START,  // 33
    WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND1 = WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND_START,
    WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND2,
    WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND3,
    WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND_END = WLAN_CFG_RF_RX_INSERTION_LOSS_2G_BAND3,
    /* 5G RF前端 */
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND_START,  // 37
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND1 = WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND_START,
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND2,
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND3,
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND4,
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND5,
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND6,
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND7,
    WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND_END = WLAN_CFG_RF_RX_INSERTION_LOSS_5G_BAND7,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND_START,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND0 = WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND_START,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND1,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND2,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND3,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND4,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND5,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND6,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND7,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND8,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND9,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND10,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND11,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND12,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND13,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND14,
    WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND_END = WLAN_CFG_RF_RX_INSERTION_LOSS_6G_BAND14,
#endif
    /* 用于定制化计算PWR RF值的偏差 */
    WLAN_CFG_RF_PWR_REF_RSSI_2G_20M_MULT4,
    WLAN_CFG_RF_PWR_REF_RSSI_2G_40M_MULT4,
    WLAN_CFG_RF_PWR_REF_RSSI_5G_20M_MULT4,
    WLAN_CFG_RF_PWR_REF_RSSI_5G_40M_MULT4,
    WLAN_CFG_RF_PWR_REF_RSSI_5G_80M_MULT4,
    WLAN_CFG_RF_PWR_REF_RSSI_5G_160M_MULT4,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    WLAN_CFG_RF_PWR_REF_RSSI_6G_20M_MULT4,
    WLAN_CFG_RF_PWR_REF_RSSI_6G_40M_MULT4,
    WLAN_CFG_RF_PWR_REF_RSSI_6G_80M_MULT4,
    WLAN_CFG_RF_PWR_REF_RSSI_6G_160M_MULT4,
#endif
    /* 用于定制化计算RSSI的偏差 */
    WLAN_CFG_RF_AMEND_RSSI_2G_20M,
    WLAN_CFG_RF_AMEND_RSSI_2G_40M,
    WLAN_CFG_RF_AMEND_RSSI_2G_11B,

    WLAN_CFG_RF_AMEND_RSSI_5G_20M,
    WLAN_CFG_RF_AMEND_RSSI_5G_40M,
    WLAN_CFG_RF_AMEND_RSSI_5G_80M,
    WLAN_CFG_RF_AMEND_RSSI_5G_160M,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    WLAN_CFG_RF_AMEND_RSSI_6G_20M,
    WLAN_CFG_RF_AMEND_RSSI_6G_40M,
    WLAN_CFG_RF_AMEND_RSSI_6G_80M,
    WLAN_CFG_RF_AMEND_RSSI_6G_160M,
#endif
    /* FEM mp13考虑2g */
    WLAN_CFG_RF_LNA_BYPASS_GAIN_DB_2G,
    WLAN_CFG_RF_LNA_GAIN_DB_2G,
    WLAN_CFG_RF_PA_GAIN_DB_B0_2G,
    WLAN_CFG_RF_PA_GAIN_DB_B1_2G,
    WLAN_CFG_RF_PA_GAIN_LVL_2G,
    WLAN_CFG_EXT_SWITCH_ISEXIST_2G,
    WLAN_CFG_EXT_PA_ISEXIST_2G,
    WLAN_CFG_EXT_LNA_ISEXIST_2G,
    WLAN_CFG_LNA_ON2OFF_TIME_NS_2G,
    WLAN_CFG_LNA_OFF2ON_TIME_NS_2G,
    WLAN_CFG_RF_LNA_BYPASS_GAIN_DB_5G,
    WLAN_CFG_RF_LNA_GAIN_DB_5G,
    WLAN_CFG_RF_PA_GAIN_DB_B0_5G,
    WLAN_CFG_RF_PA_GAIN_DB_B1_5G,
    WLAN_CFG_RF_PA_GAIN_LVL_5G,
    WLAN_CFG_EXT_SWITCH_ISEXIST_5G,
    WLAN_CFG_EXT_PA_ISEXIST_5G,
    WLAN_CFG_EXT_LNA_ISEXIST_5G,
    WLAN_CFG_LNA_ON2OFF_TIME_NS_5G,
    WLAN_CFG_LNA_OFF2ON_TIME_NS_5G,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    WLAN_CFG_RF_LNA_BYPASS_GAIN_DB_6G,
    WLAN_CFG_RF_LNA_GAIN_DB_6G,
    WLAN_CFG_RF_PA_GAIN_DB_B0_6G,
    WLAN_CFG_RF_PA_GAIN_DB_B1_6G,
    WLAN_CFG_RF_PA_GAIN_LVL_6G,
    WLAN_CFG_EXT_SWITCH_ISEXIST_6G,
    WLAN_CFG_EXT_PA_ISEXIST_6G,
    WLAN_CFG_EXT_LNA_ISEXIST_6G,
    WLAN_CFG_LNA_ON2OFF_TIME_NS_6G,
    WLAN_CFG_LNA_OFF2ON_TIME_NS_6G,
#endif
    WLAN_CFG_FEM_MODE_2G,
    WLAN_CFG_FEM_MODE_5G,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    WLAN_CFG_FEM_MODE_6G,
#endif
    WLAN_CFG_DELTA_CCA_ED_HIGH_20TH_2G,
    WLAN_CFG_DELTA_CCA_ED_HIGH_40TH_2G,
    WLAN_CFG_DELTA_CCA_ED_HIGH_20TH_5G,
    WLAN_CFG_DELTA_CCA_ED_HIGH_40TH_5G,
    WLAN_CFG_DELTA_CCA_ED_HIGH_80TH_5G,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    WLAN_CFG_DELTA_CCA_ED_HIGH_20TH_6G,
    WLAN_CFG_DELTA_CCA_ED_HIGH_40TH_6G,
    WLAN_CFG_DELTA_CCA_ED_HIGH_80TH_6G,
#endif
    WLAN_CFG_RF_CALI_BUTT,
} wlan_rf_cfg_enum;

typedef enum {
    CUST_CFG_UINT8,
    CUST_CFG_INT8,
    CUST_CFG_UINT16,
    CUST_CFG_INT16,
    CUST_CFG_UINT32,
    CUST_CFG_INT32,
} cust_data_type;

typedef enum {
    CUST_CHECK_MAX,
    CUST_CHECK_MIN,
    CUST_CHECK_VAL,
} cust_data_check_type;

typedef struct {
    char *name;          /* 定制化项名称 */
    int case_entry;      /* 定制化项枚举变量 */
    void *data_addr;     /* 定制化数据存储地址 */
    uint8_t data_type;   /* 定制化数据存储类型(占用地址) */
    uint8_t data_num;    /* 定制化项中定义数据个数 */
    oal_bool_enum_uint8 en_value_state;  /* 定制化项读取成功状态 */
} wlan_cust_cfg_cmd;

typedef struct {
    void *data_addr;        /* 数据存放地址 */
    uint8_t data_type;      /* 数据类型 */
    uint8_t check_type;     /* 检查类型 */
    int32_t max_val;        /* 最大值 */
    int32_t min_val;        /* 最小值 */
    int32_t default_val;    /* 默认值 */
} wlan_cust_range_stru;

#ifdef HISI_CONN_NVE_SUPPORT
typedef struct {
    int8_t *ini_data_addr;                  /* addr in ini */
    wlan_cali_pow_para_stru *nv_data_addr;  /* addr in nv */
    uint8_t ini_data_num;                   /* data num */
} wlan_cust_dpn_cfg_cmd;
#endif
extern cust_host_cfg_stru g_cust_host;
extern mac_cust_feature_cap_stru g_cust_cap;
extern mac_cust_dyn_pow_sru g_cust_dyn_pow;
extern wlan_cust_pow_stru g_cust_pow;

extern mac_cus_rf_cali_stru g_cust_rf_cali;
extern mac_customize_rf_front_sru g_cust_rf_front;
extern wlan_cust_cfg_cmd g_wifi_cfg_host[WLAN_CFG_HOST_BUTT];
extern wlan_cust_cfg_cmd g_wifi_cfg_rf_cali[WLAN_CFG_RF_CALI_BUTT];
extern wlan_cust_cfg_cmd g_wifi_cfg_dyn_pow[WLAN_CFG_DYN_POW_PARAMS_BUTT];
extern wlan_cust_cfg_cmd g_wifi_cfg_pow[NVRAM_PARA_PWR_INDEX_BUTT];
extern wlan_cust_cfg_cmd g_wifi_cfg_cap[WLAN_CFG_CAP_BUTT];

oal_bool_enum_uint8 hwifi_get_cust_read_status(int32_t cus_tag, int32_t cfg_id);
uint32_t hwifi_custom_host_read_cfg_init_mp16(void);
uint32_t hwifi_hcc_customize_h2d_data_cfg_mp16(void);
void hwifi_show_customize_info_mp16(void);
void hwifi_rf_cali_data_host_addr_init_mp16(void);
uint32_t hwifi_config_init_fcc_ce_params_mp16(oal_net_device_stru *pst_cfg_net_dev);
void hwifi_config_cpu_freq_ini_param_mp16(void);
void hwifi_config_host_global_ini_param_mp16(void);
uint32_t wal_custom_cali_mp16(void);
void wal_send_cali_data_mp16(oal_net_device_stru *cfg_net_dev);
int32_t hwifi_config_init_mp16(int32_t cus_tag);
uint32_t hwifi_get_sar_ctrl_params_mp16(uint8_t lvl_num, uint8_t *data_addr, uint16_t *data_len, uint16_t dest_len);
void wal_get_nv_dpd_data_mp16(void);
uint32_t hwifi_config_set_cali_chan(uint32_t);
uint32_t hwifi_config_get_cali_chan(uint32_t, uint32_t);
int32_t hwifi_config_get_cust_val(wlan_cust_cfg_cmd *cfg_cmd);
#endif  // hisi_customize_wifi_mp16.h
