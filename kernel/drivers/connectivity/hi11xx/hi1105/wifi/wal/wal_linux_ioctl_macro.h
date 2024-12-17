/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_flowctl.c 的头文件
 * 作    者 :
 * 创建日期 : 2012年12月10日
 */

#ifndef WAL_LINUX_IOCTL_MACRO_H
#define WAL_LINUX_IOCTL_MACRO_H

#include "oal_ext_if.h"
#include "wlan_types.h"
#include "wlan_spec.h"
#include "mac_vap.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "alg_cfg.h"
#include "wal_linux_vendor.h"
#ifdef CONFIG_DOZE_FILTER
#include <huawei_platform/power/wifi_filter/wifi_filter.h>
#endif /* CONFIG_DOZE_FILTER */
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
#include "hisi_customize_wifi.h"
#endif

#define WLAN1_NETDEV_NAME          "wlan1"
#define MAX_PRIV_CMD_SIZE          4096
#define NET_DEV_NAME_LEN           16
#define ENABLE 1
#define DISABLE 0
#define BUFF_SIZE 64
#define CMD_LENGTH 2

#define WAL_HIPRIV_RATE_INVALID 255 /*  无效速率配置值  */

#ifdef _PRE_WLAN_RR_PERFORMANCE
#define ASPM_ENABLE                1
#define ASPM_DISABLE               0
#define FREQ_MAX_VALUE             30
#define FREQ_MAX_FLAG              0
#define FREQ_AUTO_FLAG             1
#endif /* _PRE_WLAN_RR_PERFORMANCE */

#define WAL_HIPRIV_CMD_MAX_LEN (WLAN_MEM_LOCAL_SIZE2 - 4) /* 私有配置命令字符串最大长度，对应本地内存池一级大小 */

#define WAL_HIPRIV_CMD_NAME_MAX_LEN  80 /* 字符串中每个单词的最大长度(原20) */
#define WAL_HIPRIV_CMD_VALUE_MAX_LEN 10 /* 字符串中某个对应变量取值的最大位数 */

#define WAL_HIPRIV_PROC_ENTRY_NAME "hipriv"

#define WAL_SIOCDEVPRIVATE 0x89F0 /* SIOCDEVPRIVATE */

#define WAL_HIPRIV_HT_MCS_MIN    0
#define WAL_HIPRIV_HT_MCS_MAX    32
#define WAL_HIPRIV_VHT_MCS_MIN   0
#define WAL_HIPRIV_VHT_MCS_MAX   11
#define WAL_HIPRIV_HE_MCS_MIN    0
#define WAL_HIPRIV_HE_MCS_MAX    13  /* 增加4KQAM速度集，MCS12/13 */
#define WAL_HIPRIV_HE_ER_MCS_MIN 0
#define WAL_HIPRIV_HE_ER_MCS_MAX 2
#define WAL_HIPRIV_NSS_MIN       1
#define WAL_HIPRIV_NSS_MAX       4
#define WAL_HIPRIV_CH_NUM        4
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
#define WAL_HIPRIV_MCS_TYPE_NUM  (WAL_DSCR_PARAM_MCSAX_ER - WAL_DSCR_PARAM_MCS + 1)
#else
#define WAL_HIPRIV_MCS_TYPE_NUM  (WAL_DSCR_PARAM_MCSAX - WAL_DSCR_PARAM_MCS + 1)
#endif

#define WAL_PHY_DEBUG_TEST_WORD_CNT  5 /* trailer上报个数 */

#define WAL_HIPRIV_MS_TO_S                1000   /* ms和s之间倍数差 */
#define WAL_HIPRIV_KEEPALIVE_INTERVAL_MIN 5000   /* 受默认老化计数器出发时间所限制 */
#define WAL_HIPRIV_KEEPALIVE_INTERVAL_MAX 0xffff /* timer间隔时间限制所致(oal_uin16) */

#define WAL_LOW_LATENCY_ON 0x8 /* 1000, 提高芯片频率, 关闭napi, 关闭wifi休眠, 关闭ip休眠 */
#define WAL_LOW_LATENCY_OFF 0x7 /* 0111, 降低芯片频率, 打开napi, 打开wifi休眠, 打开ip休眠 */

/* 用户pwr ref reg的定制化保护阈值 */
#define WAL_HIPRIV_PWR_REF_DELTA_HI 40
#define WAL_HIPRIV_PWR_REF_DELTA_LO (-40)

/* IOCTL私有配置命令宏定义 */
#define WAL_IOCTL_PRIV_SETPARAM      (OAL_SIOCIWFIRSTPRIV + 0)
#define WAL_IOCTL_PRIV_GETPARAM      (OAL_SIOCIWFIRSTPRIV + 1)
#define WAL_IOCTL_PRIV_SET_WMM_PARAM (OAL_SIOCIWFIRSTPRIV + 3)
#define WAL_IOCTL_PRIV_GET_WMM_PARAM (OAL_SIOCIWFIRSTPRIV + 5)
#define WAL_IOCTL_PRIV_SET_COUNTRY   (OAL_SIOCIWFIRSTPRIV + 8)
#define WAL_IOCTL_PRIV_GET_COUNTRY   (OAL_SIOCIWFIRSTPRIV + 9)

#define WAL_IOCTL_PRIV_GET_MODE (OAL_SIOCIWFIRSTPRIV + 17) /* 读取模式 */
#define WAL_IOCTL_PRIV_SET_MODE (OAL_SIOCIWFIRSTPRIV + 13) /* 设置模式 包括协议 频段 带宽 */

#define WAL_IOCTL_PRIV_AP_GET_STA_LIST (OAL_SIOCIWFIRSTPRIV + 21)
#define WAL_IOCTL_PRIV_AP_MAC_FLTR     (OAL_SIOCIWFIRSTPRIV + 22)
/* netd将此配置命令作为GET方式下发，get方式命令用奇数，set用偶数 */
#define WAL_IOCTL_PRIV_SET_AP_CFG      (OAL_SIOCIWFIRSTPRIV + 23)
#define WAL_IOCTL_PRIV_AP_STA_DISASSOC (OAL_SIOCIWFIRSTPRIV + 24)

#define WAL_IOCTL_PRIV_SET_MGMT_FRAME_FILTERS (OAL_SIOCIWFIRSTPRIV + 28) /* 设置管理帧过滤 */

#define WAL_IOCTL_PRIV_GET_BLACKLIST (OAL_SIOCIWFIRSTPRIV + 27)

#ifdef _PRE_WLAN_FEATURE_DFS
#define WAL_IOCTL_PRIV_GET_DFS_CHN_STAT (OAL_SIOCIWFIRSTPRIV + 29)
#endif

#define WAL_IOCTL_PRIV_SUBCMD_MAX_LEN 20

#define EXT_PA_ISEXIST_5G_MASK   0x000F
#define EXT_FEM_LP_STATUS_MASK   0x00F0
#define EXT_FEM_FEM_SPEC_MASK    0xFF00
#define EXT_FEM_LP_STATUS_OFFSET 4
#define EXT_FEM_FEM_SPEC_OFFSET  8

#define WAL_IFR_DATA_OFFSET 8

#define WAL_HIPRIV_DSCR_PARAM_CMD_UCAST_DATA 0
#define WAL_HIPRIV_DSCR_PARAM_CMD_MCAST_DATA 1
#define WAL_HIPRIV_DSCR_PARAM_CMD_BCAST_DATA 2
#define WAL_HIPRIV_DSCR_PARAM_CMD_UCAST_MGMT 3
#define WAL_HIPRIV_DSCR_PARAM_CMD_MBCAST_MGMT 4

typedef uint32_t (*wal_hipriv_cmd_func)(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
typedef uint32_t (*wal_hid2d_vendor_cmd_func)(oal_net_device_stru *net_dev, int8_t *cmd);

typedef uint32_t (*pwal_get_cmd_one_arg)(int8_t *pc_cmd, int8_t *pc_arg, uint32_t arg_len, uint32_t *pul_cmd_offset);
/* wlan cfg相关 */
#define wlan_cfg_info_tbl(_wlan_cfg_id, _en_need_host_process, _en_need_w4_host_return, \
                          _en_need_dev_process, _en_need_w4_dev_return, _p_func) \
    { \
        _wlan_cfg_id, { 0, 0 }, { _en_need_host_process, \
                                  _en_need_w4_host_return, \
                                  _en_need_dev_process, \
                                  _en_need_w4_dev_return }, _p_func      \
    }
#define WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(_en_cfg_id, _p_func) \
    {                                                          \
        _en_cfg_id, { 0, 0 }, _p_func                          \
    }


#ifdef _PRE_LINUX_TEST
#define BOARD_SUCC OAL_SUCC
#endif

#ifdef _PRE_WLAN_FEATURE_NAN
/* NAN周期短或者监听时间长影响业务 */
#define WAL_NAN_MAX_DURATION 80
#define WAL_NAN_MIN_PERIOD 200
#endif

typedef enum {
    WAL_DSCR_PARAM_PA_GAIN_LEVEL = 0, /* pa增益等级 */
    WAL_DSCR_PARAM_MICRO_TX_POWER_GAIN_LEVEL,
    WAL_DSCR_PARAM_TXRTS_ANTENNA,
    WAL_DSCR_PARAM_RXCTRL_ANTENNA,
    WAL_DSCR_PARAM_CHANNAL_CODE,
    WAL_DSCR_PARAM_POWER,
    WAL_DSCR_PARAM_SHORTGI,
    WAL_DSCR_PARAM_PREAMBLE_MODE,
    WAL_DSCR_PARAM_RTSCTS,
    WAL_DSCR_PARAM_LSIGTXOP,
    WAL_DSCR_PARAM_SMOOTH,
    WAL_DSCR_PARAM_SOUNDING,
    WAL_DSCR_PARAM_TXBF,
    WAL_DSCR_PARAM_STBC,
    WAL_DSCR_PARAM_GET_ESS,
    WAL_DSCR_PARAM_DYN_BW,
    WAL_DSCR_PARAM_DYN_BW_EXIST,
    WAL_DSCR_PARAM_CH_BW_EXIST,
    WAL_DSCR_PARAM_RATE,
    WAL_DSCR_PARAM_MCS,
    WAL_DSCR_PARAM_MCSAC,
    WAL_DSCR_PARAM_MCSAX,
    WAL_DSCR_PARAM_MCSAX_ER,
    WAL_DSCR_PARAM_NSS,
    WAL_DSCR_PARAM_BW,
    WAL_DSCR_PARAM_LTF,
    WLA_DSCR_PARAM_GI,
    WLA_DSCR_PARAM_TXCHAIN,
    WLA_DSCR_PARAM_DCM,
    WLA_DSCR_PARAM_PROTOCOL_MODE,

    WAL_DSCR_PARAM_BUTT
} wal_dscr_param_enum;

typedef uint8_t wal_dscr_param_enum_uint8;

/* rx ip数据包过滤功能和上层协定(格式)结构体 */
#ifdef CONFIG_DOZE_FILTER
typedef hw_wifi_filter_item wal_hw_wifi_filter_item;
typedef struct hw_wlan_filter_ops wal_hw_wlan_filter_ops;

#else
typedef struct {
    unsigned short protocol;  // 协议类型
    unsigned short port;      // 目的端口号
    unsigned int filter_cnt;  // 过滤报文数
} wal_hw_wifi_filter_item;

typedef struct {
    int (*set_filter_enable)(int);
    int (*set_filter_enable_ex)(int, int);
    int (*add_filter_items)(wal_hw_wifi_filter_item *, int);
    int (*clear_filters)(void);
    int (*get_filter_pkg_stat)(wal_hw_wifi_filter_item *, int, int *);
} wal_hw_wlan_filter_ops;
#endif

typedef enum {
    WAL_TX_POW_PARAM_SET_RF_REG_CTL = 0,
    WAL_TX_POW_PARAM_SET_FIX_LEVEL,
    WAL_TX_POW_PARAM_SET_MAG_LEVEL,
    WAL_TX_POW_PARAM_SET_CTL_LEVEL,
    WAL_TX_POW_PARAM_SET_AMEND,
    WAL_TX_POW_PARAM_SET_NO_MARGIN,
    WAL_TX_POW_PARAM_SET_SHOW_LOG,
    WAL_TX_POW_PARAM_SET_SAR_LEVEL,
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    WAL_TX_POW_PARAM_TAS_POW_CTRL,
    WAL_TX_POW_PARAM_TAS_RSSI_MEASURE,
    WAL_TX_POW_PARAM_TAS_ANT_SWITCH,
#endif
    WAL_TX_POW_PARAM_SHOW_TPC_TABLE_GAIN,
    WAL_TX_POW_POW_SAVE,
    WAL_TX_POW_GET_PD_INFO,
    WAL_TX_POW_SET_TPC_IDX,

    WAL_TX_POW_PARAM_BUTT
} wal_tx_pow_param_enum;

typedef enum {
    WAL_AMPDU_DISABLE,
    WAL_AMPDU_ENABLE,

    WAL_AMPDU_CFG_BUTT
} wal_ampdu_idx_enum;
#ifdef _PRE_WLAN_RR_PERFORMANCE
/* pc命令枚举类型 */
typedef enum {
    WAL_EXT_PRI_CMD_SET_DEVICE_FREQ_MAX,     // 0 将Device频率锁定到最高
    WAL_EXT_PRI_CMD_SET_DEVICE_FREQ_AUTO,    // 1 将Device频率设置成动态调频
    WAL_EXT_PRI_CMD_ENBLE_PCIE_ASPM,         // 2 打开PCIE低功耗
    WAL_EXT_PRI_CMD_DISABLE_PCIE_ASPM,       // 3 关闭PCIE低功耗

    WAL_EXT_PRI_CMD_BUFF
} wal_ext_pri_cmd_neum;
#endif /* _PRE_WLAN_RR_PERFORMANCE */

/* HiD2D APK命令枚举值，上层下发下来 */
typedef enum {
    WAL_HID2D_INIT_CMD = 0,
    WAL_HID2D_SCAN_START_CMD = 1,
    WAL_HID2D_CHAN_SWITCH_CMD = 2,
    WAL_HID2D_HIGH_BW_MCS_CMD = 3,
    WAL_HID2D_RTS_CMD = 4,
    WAL_HID2D_TPC_CMD = 5,
    WAL_HID2D_CCA_CMD = 6,
    WAL_HID2D_EDCA_CMD = 7,
    WAL_HID2D_ACS_CMD = 8,
    WAL_HID2D_ACS_STATE_CMD = 9,
    WAL_HID2D_LINK_MEAS_CMD = 10,

    WAL_HID2D_CMD_BUTT,
} hmac_hid2d_cmd_enum;
#endif /* end of wal_linux_ioctl.h */
