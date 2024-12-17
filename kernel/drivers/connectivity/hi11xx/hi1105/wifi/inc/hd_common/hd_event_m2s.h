/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递m2s事件 的头文件
 * 创建日期   : 2022年9月16日
 */

#ifndef HD_EVENT_M2S_H
#define HD_EVENT_M2S_H

#include "mac_common.h"
#include "wlan_spec.h"

/* 3.10 m2s切换枚举 */
/* mimo-siso切换tpye */
typedef enum {
    WLAN_M2S_TYPE_SW = 0,
    WLAN_M2S_TYPE_HW = 1,

    WLAN_M2S_TYPE_BUTT,
} wlan_m2s_tpye_enum;
typedef uint8_t wlan_m2s_type_enum_uint8;

/* mimo-siso切换mode */
typedef enum {
    WLAN_M2S_TRIGGER_MODE_DBDC = BIT0,
    WLAN_M2S_TRIGGER_MODE_FAST_SCAN = BIT1,
    WLAN_M2S_TRIGGER_MODE_RSSI = BIT2,
    WLAN_M2S_TRIGGER_MODE_BTCOEX = BIT3,
    WLAN_M2S_TRIGGER_MODE_COMMAND = BIT4,
    WLAN_M2S_TRIGGER_MODE_TEST = BIT5,
    WLAN_M2S_TRIGGER_MODE_CUSTOM = BIT6, /* 定制化优先级高，只能被蓝牙打断 */
    WLAN_M2S_TRIGGER_MODE_SPEC = BIT7,   /* spec优先级最高，不允许打断 */

    WLAN_M2S_TRIGGER_MODE_BUTT,
} wlan_m2s_trigger_mode_enum;
typedef uint8_t wlan_m2s_trigger_mode_enum_uint8;

/* m2s vap信息结构体 */
typedef struct {
    uint8_t uc_vap_id;                  /* 业务vap id */
    wlan_nss_enum_uint8 en_vap_rx_nss;    /* vap的接收空间流个数 */
    wlan_m2s_type_enum_uint8 en_m2s_type; /* 0:软切换 1:硬切换 */
    uint8_t en_sm_power_save; /* wlan_mib_mimo_power_save_enum */

    oal_bool_enum_uint8 en_tx_stbc;
    oal_bool_enum_uint8 en_transmit_stagger_sounding;
    oal_bool_enum_uint8 en_vht_ctrl_field_supported;
    oal_bool_enum_uint8 en_vht_number_sounding_dimensions;

    oal_bool_enum_uint8 en_vht_su_beamformer_optionimplemented;
    oal_bool_enum_uint8 en_tx_vht_stbc_optionimplemented;
    oal_bool_enum_uint8 en_support_opmode;
    uint8_t uc_vht_ntx_sts;
    oal_bool_enum_uint8 en_su_bfee;
    uint8_t auc_reserved1[NUM_3_BYTES];
#ifdef _PRE_WLAN_FEATURE_11AX
    oal_bool_enum_uint8 en_he_su_bfer;
    uint8_t uc_he_num_dim_below_80m;
    uint8_t uc_he_num_dim_over_80m;
    uint8_t uc_he_ntx_sts_below_80m;
    uint8_t uc_he_ntx_sts_over_80m;
    uint8_t auc_reserved2[NUM_3_BYTES];
#endif
} mac_vap_m2s_stru; /* hd_event */

typedef struct {
    uint8_t uc_device_id;
    wlan_nss_enum_uint8 en_m2s_nss;
    wlan_m2s_type_enum_uint8 en_m2s_type; /* 0:软切换 1:硬切换 */
    uint8_t auc_reserve[1];
} dmac_to_hmac_m2s_event_stru; /* hd_event */

/* MIMO/SISO状态机状态枚举 */
typedef enum {
    HAL_M2S_STATE_IDLE = 0, /* 初始运行状态 */
    HAL_M2S_STATE_SISO = 1, /* SISO状态,软件和硬件都是单通道 */
    HAL_M2S_STATE_MIMO = 2, /* MIMO状态,软件和硬件都是双通道 */
    HAL_M2S_STATE_MISO = 3, /* MISO探测态,软件单通道，硬件双通道 */
    HAL_M2S_STATE_SIMO = 4, /* SIMO状态,软件双通道，硬件单通道 */
    HAL_M2S_STATE_BUTT = 5, /* 最大状态 */
    HAL_M2S_ARP_DETECT = 6, /* arp探测, 05修改同bisheng一样的上报值，复用接口用到 */
} hal_m2s_state;
typedef uint8_t hal_m2s_state_uint8;

#ifdef _PRE_WLAN_FEATURE_M2S
/* MSS使用时命令形式 */
typedef enum {
    MAC_M2S_COMMAND_MODE_SET_AUTO = 0,
    MAC_M2S_COMMAND_MODE_SET_SISO_C0 = 1,
    MAC_M2S_COMMAND_MODE_SET_SISO_C1 = 2,
    MAC_M2S_COMMAND_MODE_SET_MIMO = 3,
    MAC_M2S_COMMAND_MODE_GET_STATE = 4,
    MAC_MCM_COMMAND_MODE_PROTECT = 5,

    MAC_M2S_COMMAND_MODE_BUTT,
} mac_m2s_command_mode_enum;
typedef uint8_t mac_m2s_command_mode_enum_uint8;

/* 配置命令使用，mimo-siso切换mode */
typedef enum {
    MAC_M2S_MODE_QUERY          = 0,    /* 参数查询模式 */
    MAC_M2S_MODE_MSS            = 1,    /* MSS下发模式 */
    MAC_M2S_MODE_DELAY_SWITCH   = 2,    /* 延迟切换测试模式 */
    MAC_M2S_MODE_SW_TEST        = 3,    /* 软切换测试模式,测试siso和mimo */
    MAC_M2S_MODE_HW_TEST        = 4,    /* 硬切换测试模式,测试siso和mimo */
    MAC_M2S_MODE_RSSI           = 5,    /* rssi切换 */
    MAC_M2S_MODE_MODEM          = 6,    /* MODEM切换模式 */
    MAC_M2S_MODE_BTCOEX         = 7,    /* btcoex切换模式 */
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
    MAC_M2S_MODE_LP_ANT_SWITCH    = 8,    /* 天线切换模式 */
    MAC_M2S_MODE_REPORT         = 9,    /* host 查询device是否切换可以切mimo/siso */
#endif

    MAC_M2S_MODE_BUTT,
} mac_m2s_mode_enum;
typedef uint8_t mac_m2s_mode_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
/* 配置命令使用，mimo-siso切换mode */
typedef enum {
    MAC_LP_MIRACAST_DISABLE          = 0,    /* 投屏低功耗禁止 */
    MAC_LP_MIRACAST_ENABLE           = 1,    /* 投屏低功耗使能 */
    MAC_LP_MIRACAST_DEBUG_ON         = 2,    /* 投屏低功耗开启debug模式 */
    MAC_LP_MIRACAST_DEBUG_OFF        = 3,    /* 投屏低功耗关闭debug模式 */

    MAC_LP_MIRACAST_BUTT,
} mac_lp_miracast_cmd_enum;
#endif

/* MODEM命令使用，期望切换状态 */
typedef enum {
    MAC_MCM_MODEM_STATE_SISO_C0     = 0,
    MAC_MCM_MODEM_STATE_MIMO        = 2,

    MAC_MCM_MODEM_STATE_BUTT,
} mac_mcm_modem_state_enum;
typedef uint8_t mac_mcm_modem_state_enum_uint8;

typedef struct {
    /* 0:参数查询模式; 1:参数配置模式;2.切换模式;3.软切换测试模式;4.硬切换测试模式 5.RSSI配置命令 */
    mac_m2s_mode_enum_uint8         en_cfg_m2s_mode;
    union {
        struct {
            oal_bool_enum_uint8      en_m2s_type;    /* 切换类型 */
            uint8_t                uc_master_id;   /* 主辅路id */
            uint8_t uc_m2s_state;   /* 期望切换到状态 */
            wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode; /* 切换触发业务模式 */
        } test_mode;

        struct {
            oal_bool_enum_uint8 en_mss_on;
        } mss_mode;

        struct {
            uint8_t uc_opt;
            int8_t c_value;
        } rssi_mode;

        struct {
            mac_mcm_modem_state_enum_uint8    en_m2s_state;   /* 期望切换到状态 */
        } modem_mode;
        struct {
            uint8_t chain_apply;
            uint8_t ant_occu_prio;
        } btcoex_mode;
    } pri_data;
} mac_m2s_mgr_stru; /* hd_event */

typedef struct {
    wlan_m2s_mgr_vap_stru ast_m2s_comp_vap[WLAN_SERVICE_STA_MAX_NUM_PER_DEVICE];
    uint8_t uc_m2s_state; /* 当前m2s状态 */
    union {
        struct {
            uint8_t en_m2s_result;
            uint8_t uc_m2s_mode; /* 当前切换业务 */
            uint8_t uc_vap_num;
        } mss_result;

        struct {
            oal_bool_enum_uint8 en_arp_detect_on;
        } arp_detect_result;
    } pri_data;
} dmac_m2s_complete_syn_stru; /* hd_event */
#endif

typedef struct {
    wlan_m2s_mgr_vap_stru ast_m2s_blacklist[WLAN_M2S_BLACKLIST_MAX_NUM];
    uint8_t uc_blacklist_cnt;
} mac_m2s_ie_stru; /* hd_event */

/* m2s device信息结构体 */
typedef struct {
    uint8_t uc_device_id;                           /* 业务vap id */
    wlan_nss_enum_uint8 en_nss_num;                   /* device的接收空间流个数 */
    /* mac device的smps能力，用于切换后vap能力初始化 */
    uint8_t en_smps_mode;   /* wlan_mib_mimo_power_save_enum */
    uint8_t auc_reserved[1];
} mac_device_m2s_stru; /* hd_event */

/* 存在MIMO兼容性AP，需抛事件通知hmac重关联，事件结构体 */
typedef struct {
    uint16_t us_user_id;
    mac_ap_type_enum_uint16 en_ap_type;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];
    uint8_t auc_resv[NUM_2_BYTES];
} dmac_to_hmac_mimo_compatibility_event_stru; /* hd_event */
#endif /* end of hd_event_m2s.h */
