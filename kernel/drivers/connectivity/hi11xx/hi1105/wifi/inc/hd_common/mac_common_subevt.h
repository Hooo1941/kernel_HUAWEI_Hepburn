/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : dmac对外公共接口头文件
 * 作    者 :
 * 创建日期 : 2012年9月20日
 */

#ifndef MAC_COMMON_SUBEVT_H
#define MAC_COMMON_SUBEVT_H

/* 1 其他头文件包含 */
#include "oal_types.h"
#include "wlan_types.h"

// 此处不加extern "C" UT编译不过
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 3 枚举定义 */
/*****************************************************************************
  枚举名  : dmac_tx_host_drx_subtype_enum
  协议表格:
  枚举说明: HOST DRX事件子类型定义
*****************************************************************************/
/* WLAN_CRX子类型定义 */
typedef enum {
    DMAC_TX_HOST_DRX = 0,
    HMAC_TX_HOST_DRX = 1,
    HMAC_TX_DMAC_SCH = 2, /* 调度子事件 */
    DMAC_TX_HOST_DTX = 3, /* H2D传输netbuf, device ring tx使用 */

    DMAC_TX_HOST_DRX_BUTT
} dmac_tx_host_drx_subtype_enum;

/*****************************************************************************
  枚举名  : dmac_tx_wlan_dtx_subtype_enum
  协议表格:
  枚举说明: WLAN DTX事件子类型定义
*****************************************************************************/
typedef enum {
    DMAC_TX_WLAN_DTX = 0,

    DMAC_TX_WLAN_DTX_BUTT
} dmac_tx_wlan_dtx_subtype_enum;

/*****************************************************************************
  枚举名  : dmac_wlan_ctx_event_sub_type_enum
  协议表格:
  枚举说明: WLAN CTX事件子类型定义
*****************************************************************************/
typedef enum {
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT = 0, /* 管理帧处理 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_ADD_USER = 1,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_NOTIFY_ALG_ADD_USER = 2,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_DEL_USER = 3,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_BA_SYNC = 4,  /* 收到wlan的Delba和addba rsp用于到dmac的同步 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_PRIV_REQ = 5, /* 11N自定义的请求的事件类型 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ = 6,       /* 扫描请求 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCHED_SCAN_REQ = 7, /* PNO调度扫描请求 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_RESET_PSM = 8,      /* 收到认证请求 关联请求，复位用户的节能状态 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_SET_REG = 9,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_DTIM_TSF_REG = 10,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_CONN_RESULT = 11, /* 关联结果 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_ASOC_WRITE_REG = 12, /* AP侧处理关联时，修改SEQNUM_DUPDET_CTRL寄存器 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_EDCA_REG = 13,       /* STA收到beacon和assoc rsp时，更新EDCA寄存器 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_SWITCH_TO_NEW_CHAN = 14, /* 切换至新信道事件 */
    DMAC_WALN_CTX_EVENT_SUB_TYPR_SELECT_CHAN = 15,        /* 设置信道事件 */
    DMAC_WALN_CTX_EVENT_SUB_TYPR_DISABLE_TX = 16,         /* 禁止硬件发送 */
    DMAC_WALN_CTX_EVENT_SUB_TYPR_ENABLE_TX = 17,          /* 恢复硬件发送 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPR_RESTART_NETWORK = 18,    /* 切换信道后，恢复BSS的运行 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPR_SWITCH_TO_OFF_CHAN = 19,  /* 切换到offchan做off-chan cac检测 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPR_SWITCH_TO_HOME_CHAN = 20, /* 切回home chan */
    DMAC_WLAN_CTX_EVENT_SUB_TYPR_DFS_TEST = 21,
    DMAC_WALN_CTX_EVENT_SUB_TYPR_DFS_CAC_CTRL_TX = 22, /* DFS 1min CAC把vap状态位置为pause或者up,同时禁止或者开启硬件发送 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPR_EDCA_OPT = 23, /* edca优化中业务识别通知事件 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_HMAC2DMAC = 24,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_DSCR_OPT = 25,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_MATRIX_HMAC2DMAC = 26,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_APP_IE_H2D = 27,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_IP_FILTER = 28,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_APF_CMD = 29,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_MU_EDCA_REG = 30, /* STA收到beacon和assoc rsp时，更新MU EDCA寄存器 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_SPATIAL_REUSE_REG = 31,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT = 32,                      /* STA收到twt 时，更新寄存器 */
    /* STA收到beacon和assoc rsp时，更新NDP Feedback report寄存器 */
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_NDP_FEEDBACK_REPORT_REG = 33,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_HIEX_H2D_MSG = 34,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_CUST_HMAC2DMAC = 35,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_CHBA_PARAMS = 36,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_ADD_MULTI_USER = 37,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_CHBA_SELF_CONN_INFO_SYNC = 38,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_CHBA_TOPO_INFO_SYNC = 39,
    DMAC_WLAN_CTX_EVENT_SUB_TYPE_BUTT
} dmac_wlan_ctx_event_sub_type_enum;

/* DMAC模块 WLAN_DRX子类型定义 */
typedef enum {
    DMAC_WLAN_DRX_EVENT_SUB_TYPE_RX_DATA,        /* AP模式: DMAC WLAN DRX 流程 */
    DMAC_WLAN_DRX_EVENT_SUB_TYPE_TKIP_MIC_FAILE, /* DMAC tkip mic faile 上报给HMAC */
    DMAC_WLAN_DRX_EVENT_SUB_TYPE_WOW_RX_DATA,    /* WOW DMAC WLAN DRX 流程 */

    DMAC_WLAN_DRX_EVENT_SUB_TYPE_BUTT
} dmac_wlan_drx_event_sub_type_enum;
typedef uint8_t dmac_wlan_drx_event_sub_type_enum_uint8;

/* DMAC模块 WLAN_CRX子类型定义 */
typedef enum {
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_INIT,              /* DMAC 给 HMAC的初始化参数 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_RX,                /* DMAC WLAN CRX 流程 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_DELBA,             /* DMAC自身产生的DELBA帧 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_EVERY_SCAN_RESULT, /* 扫描到一个bss信息，上报结果 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_SCAN_COMP,         /* DMAC扫描完成上报给HMAC */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_OBSS_SCAN_COMP,    /* DMAC OBSS扫描完成上报给HMAC */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_CHAN_RESULT,       /* 上报一个信道的扫描结果 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_ACS_RESP,          /* DMAC ACS 回复应用层命令执行结果给HMAC */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_DISASSOC,          /* DMAC上报去关联事件到HMAC, HMAC会删除用户 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_DEAUTH,            /* DMAC上报去认证事件到HMAC */
    DMAC_WLAN_CRX_EVENT_SUB_TYPR_CH_SWITCH_COMPLETE = 10, /* 信道切换完成事件 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPR_DBAC,               /* DBAC enable/disable事件 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_HIEX_D2H_MSG,
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_RX_WOW,
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_EVERY_SCAN_RESULT_RING, /* ring方式接收的扫描结果报文上报 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_TX_RING_ADDR,
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_VSP_INFO_ADDR,
    DMAC_WLAN_DRX_EVENT_SUB_TYPE_RX_SNIFFER_INFO,
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_PSD_RPT,           /* DMAC 向HMAC上报PSD信息 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_CHAN_DET_RPT,       /* DMAC 向HMAC上报信道检测信息 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_WIFI_DELAY_RPT = 20,
#ifdef _PRE_WLAN_FEATURE_HID2D
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_HID2D_CHAN_MEAS_RESULT, /* 上报一个hid2d信道测量结果 */
#endif
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_GC_JOIN_WAIT_NOA_END, /* GC关联入网时，等待device上报NOA_END中断，表明同步上GO NOA时钟 */
    DMAC_WLAN_CRX_EVENT_SUB_TYPE_BUTT
} dmac_wlan_crx_event_sub_type_enum;
typedef uint8_t dmac_wlan_crx_event_sub_type_enum_uint8;

/* 发向HOST侧的配置事件 */
typedef enum {
    DMAC_TO_HMAC_SYN_UP_REG_VAL = 1,
    DMAC_TO_HMAC_CREATE_BA = 2,
    DMAC_TO_HMAC_DEL_BA = 3,
    DMAC_TO_HMAC_SYN_CFG = 4,

    DMAC_TO_HMAC_ALG_INFO_SYN = 6,
    DMAC_TO_HMAC_VOICE_AGGR = 7,
    DMAC_TO_HMAC_SYN_UP_SAMPLE_DATA = 8,
    DMAC_TO_HMAC_M2S_DATA = 10,
    DMAC_TO_HMAC_BANDWIDTH_INFO_SYN = 11,  /* dmac向hmac同步带宽的信息 */
    DMAC_TO_HMAC_PROTECTION_INFO_SYN = 12, /* dmac向hmac同步保护mib信息 */
    DMAC_TO_HMAC_CH_STATUS_INFO_SYN = 13,  /* dmac向hmac同步可用信道列表 */
    DMAC_TO_HMAC_FTM_CALI = 14,
    /* 事件注册时候需要枚举值列出来，防止出现device侧和host特性宏打开不一致，
       造成出现同步事件未注册问题，后续各子特性人注意宏打开的一致性
    */
    DMAC_TO_HMAC_SYN_BUTT
} dmac_to_hmac_syn_type_enum;

/* hmac to dmac定制化配置同步消息结构 */
typedef enum {
    CUSTOM_CFGID_NV_ID = 0,
    CUSTOM_CFGID_INI_ID,
    CUSTOM_CFGID_DTS_ID,
    CUSTOM_CFGID_PRIV_INI_ID,
    CUSTOM_CFGID_CAP_ID,

    CUSTOM_CFGID_BUTT,
} custom_cfgid_enum;
typedef unsigned int custom_cfgid_enum_uint32;

typedef enum {
    CUSTOM_CFGID_INI_ENDING_ID = 0,
    CUSTOM_CFGID_INI_FREQ_ID,
    CUSTOM_CFGID_INI_PERF_ID,
    CUSTOM_CFGID_INI_LINKLOSS_ID,
    CUSTOM_CFGID_INI_PM_SWITCH_ID,
    CUSTOM_CFGID_INI_PS_FAST_CHECK_CNT_ID,
    CUSTOM_CFGID_INI_FAST_MODE,
    CUSTOM_CFGID_INI_LP_AGENT_SWITCH_ID,

    /* 私有定制 */
    CUSTOM_CFGID_PRIV_INI_RADIO_CAP_ID,
    CUSTOM_CFGID_PRIV_FASTSCAN_SWITCH_ID,
    CUSTOM_CFGID_PRIV_ANT_SWITCH_ID,
    CUSTOM_CFGID_PRIV_LINKLOSS_THRESHOLD_FIXED_ID,
    CUSTOM_CFGID_PRIV_RADAR_ISR_FORBID_ID,
    CUSTOM_CFGID_PRIV_INI_BW_MAX_WITH_ID,
    CUSTOM_CFGID_PRIV_INI_LDPC_CODING_ID,
    CUSTOM_CFGID_PRIV_INI_RX_STBC_ID,
    CUSTOM_CFGID_PRIV_INI_TX_STBC_ID,
    CUSTOM_CFGID_PRIV_INI_SU_BFER_ID,
    CUSTOM_CFGID_PRIV_INI_SU_BFEE_ID,
    CUSTOM_CFGID_PRIV_INI_MU_BFER_ID,
    CUSTOM_CFGID_PRIV_INI_MU_BFEE_ID,
    CUSTOM_CFGID_PRIV_INI_11N_TXBF_ID,
    CUSTOM_CFGID_PRIV_INI_1024_QAM_ID,

    CUSTOM_CFGID_PRIV_INI_CALI_MASK_ID,
    CUSTOM_CFGID_PRIV_CALI_DATA_MASK_ID,
    CUSTOM_CFGID_PRIV_INI_AUTOCALI_MASK_ID,
    CUSTOM_CFGID_PRIV_INI_DOWNLOAD_RATELIMIT_PPS,
    CUSTOM_CFGID_PRIV_INI_TXOPPS_SWITCH_ID,

    CUSTOM_CFGID_PRIV_INI_OVER_TEMPER_PROTECT_THRESHOLD_ID,
    CUSTOM_CFGID_PRIV_INI_TEMP_PRO_ENABLE_ID,
    CUSTOM_CFGID_PRIV_INI_TEMP_PRO_REDUCE_PWR_ENABLE_ID,
    CUSTOM_CFGID_PRIV_INI_TEMP_PRO_SAFE_TH_ID,
    CUSTOM_CFGID_PRIV_INI_TEMP_PRO_OVER_TH_ID,
    CUSTOM_CFGID_PRIV_INI_TEMP_PRO_PA_OFF_TH_ID,

    CUSTOM_CFGID_PRIV_INI_DSSS2OFDM_DBB_PWR_BO_VAL_ID,
    CUSTOM_CFGID_PRIV_INI_EVM_PLL_REG_FIX_ID,
    CUSTOM_CFGID_PRIV_INI_VOE_SWITCH_ID,
    CUSTOM_CFGID_PRIV_COUNTRYCODE_SELFSTUDY_CFG_ID,
    CUSTOM_CFGID_PRIV_M2S_FUNCTION_EXT_MASK_ID,
    CUSTOM_CFGID_PRIV_M2S_FUNCTION_MASK_ID,
    CUSTOM_CFGID_PRIV_MCM_CUSTOM_FUNCTION_MASK_ID,
    CUSTOM_CFGID_PRIV_MCM_FUNCTION_MASK_ID,
    CUSTOM_CFGID_PRIV_INI_11AX_SWITCH_ID,
    CUSTOM_CFGID_PRIV_INI_HTC_SWITCH_ID,
    CUSTOM_CFGID_PRIV_INI_AC_SUSPEND_ID,
    CUSTOM_CFGID_PRIV_INI_MBSSID_SWITCH_ID,
    CUSTOM_CFGID_PRIV_DYN_BYPASS_EXTLNA_ID,
    CUSTOM_CFGID_PRIV_CTRL_FRAME_TX_CHAIN_ID,

    CUSTOM_CFGID_PRIV_CTRL_UPC_FOR_18DBM_C0_ID,
    CUSTOM_CFGID_PRIV_CTRL_UPC_FOR_18DBM_C1_ID,
    CUSTOM_CFGID_PRIV_CTRL_11B_DOUBLE_CHAIN_BO_POW_ID,
    CUSTOM_CFGID_PRIV_INI_LDAC_M2S_TH_ID,
    CUSTOM_CFGID_PRIV_INI_BTCOEX_MCM_TH_ID,
    CUSTOM_CFGID_PRIV_INI_NRCOEX_ID,
    CUSTOM_CFGID_PRIV_INI_HCC_FLOWCTRL_TYPE_ID,
    CUSTOM_CFGID_PRIV_INI_MBO_SWITCH_ID,

    CUSTOM_CFGID_PRIV_INI_DYNAMIC_DBAC_ID,
    CUSTOM_CFGID_PRIV_INI_PHY_CAP_SWITCH_ID,
    CUSTOM_CFGID_PRIV_DC_FLOWCTRL_ID,
    CUSTOM_CFGID_PRIV_INI_HAL_PS_RSSI_ID,
    CUSTOM_CFGID_PRIV_INI_HAL_PS_PPS_ID,
    CUSTOM_CFGID_PRIV_INI_OPTIMIZED_FEATURE_SWITCH_ID,
    CUSTOM_CFGID_PRIV_DDR_SWITCH_ID,
    CUSTOM_CFGID_PRIV_FEM_POW_CTL_ID,
    CUSTOM_CFGID_PRIV_INI_HIEX_CAP_ID,
    CUSTOM_CFGID_PRIV_INI_TX_SWITCH_ID,
    CUSTOM_CFGID_PRIV_INI_FTM_CAP_ID,
    CUSTOM_CFGID_PRIV_INI_MIRACAST_SINK,
    CUSTOM_CFGID_PRIV_INI_MCAST_AMPDU_ENABLE_ID,
    CUSTOM_CFGID_PRIV_INI_PT_MCAST_ENABLE_ID,
    CUSTOM_CFGID_PRIV_CLOSE_FILTER_SWITCH_ID,
    CUSTOM_CFGID_PRIV_CHBA_DBAC_CHAN_FOLLOW_SWITCH_ID,
    CUSTOM_CFGID_PRIV_EXPAND_FEATURE_SWITCH_BITMAP_SWITCH_ID,
    CUSTOM_CFGID_PRIV_IS_V100_NEW_PRODUCT_ID,
    CUSTOM_CFGID_INI_BUTT,
} custom_cfgid_h2d_ini_enum;
typedef unsigned int custom_cfgid_h2d_ini_enum_uint32;

enum custom_optimize_feature {
    CUSTOM_OPTIMIZE_FEATURE_RA_FLT = 0,
    CUSTOM_OPTIMIZE_FEATURE_NAN = 1,
    CUSTOM_OPTIMIZE_FEATURE_CERTIFY_MODE = 2,
    CUSTOM_OPTIMIZE_TXOP_COT = 3,
    CUSTOM_OPTIMIZE_11AX20M_NON_LDPC = 4, /* 认证时ap 20M 不支持LDPC可以关联在ax */
    CUSTOM_OPTIMIZE_CHBA = 5, /* CHBA组网开关 */
    CUSTOM_OPTIMIZE_CE_IDLE = 6, /* CE认证用例场景关闭pk mode，不调整竞争参数 */
    CUSTOM_OPTIMIZE_CSA_EXT = 7, /* CSA扩展 跨频段功能 开关 */
    CUSTOM_OPTIMIZE_FEATURE_BUTT,
};

typedef enum {
    CUSTOM_CHBA_FAST_CONNECT_ENABLE = BIT0, /* chba极速连接使能 */
    CUSTOM_ACTION_NEGOTIATION_OPTIMIZATION = BIT1, /* action协商优化 */
    CUSTOM_PM_FILTER_IPV6 = BIT3,  /* ipv6过滤开关 */
} expand_feature_switch_enum; /* 扩展特性枚举 */

enum custom_close_filter_switch {
    CUSTOM_APF_CLOSE_SWITCH,
    CUSTOM_ARP_CLOSE_MULITCAST_FILTER,
    CUSTOM_CLOSE_FILTER_SWITCH_BUTT
};

/* MISC杂散事件 */
typedef enum {
    DMAC_MISC_SUB_TYPE_RADAR_DETECT,
    DMAC_MISC_SUB_TYPE_DISASOC,
    DMAC_MISC_SUB_TYPE_CALI_TO_HMAC,
    DMAC_MISC_SUB_TYPE_HMAC_TO_CALI,

    DMAC_MISC_SUB_TYPE_ROAM_TRIGGER,
    DMAC_TO_HMAC_DPD,

#ifdef _PRE_WLAN_FEATURE_APF
    DMAC_MISC_SUB_TYPE_APF_REPORT,
#endif
    DMAC_MISC_SUB_TYPE_TX_SWITCH_STATE,
    DMAC_MISC_PSM_GET_HOST_RING_STATE,

    DMAC_TO_HMAC_POW_DIFF,
    DMAC_MISC_SUB_TYPE_BUTT
} dmac_misc_sub_type_enum;

typedef enum {
    DMAC_DISASOC_MISC_LINKLOSS = 0,
    DMAC_DISASOC_MISC_KEEPALIVE = 1,
    DMAC_DISASOC_MISC_CHANNEL_MISMATCH = 2,
    DMAC_DISASOC_MISC_LOW_RSSI = 3,
    DMAC_DISASOC_MISC_GET_CHANNEL_IDX_FAIL = 5,
    DMAC_DISASOC_MISC_BUTT
} dmac_disasoc_misc_reason_enum;
typedef uint16_t dmac_disasoc_misc_reason_enum_uint16;

/* HMAC to DMAC同步类型 */
typedef enum {
    HMAC_TO_DMAC_SYN_INIT,
    HMAC_TO_DMAC_SYN_CREATE_CFG_VAP,
    HMAC_TO_DMAC_SYN_CFG,
    HMAC_TO_DMAC_SYN_ALG,
    HMAC_TO_DMAC_SYN_REG,
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    HMAC_TO_DMAC_SYN_SAMPLE,
#endif

    HMAC_TO_DMAC_SYN_BUTT
} hmac_to_dmac_syn_type_enum;
typedef uint8_t hmac_to_dmac_syn_type_enum_uint8;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_common.h */
