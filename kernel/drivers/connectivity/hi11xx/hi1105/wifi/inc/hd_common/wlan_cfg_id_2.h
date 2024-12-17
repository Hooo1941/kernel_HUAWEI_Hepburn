/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递命令枚举 的头文件
 * 作者       :
 * 创建日期   : 2020年6月19日
 */

#ifndef WLAN_CFG_ID_2_H
#define WLAN_CFG_ID_2_H
#include "oal_ext_if.h"
/*****************************************************************************
    配置命令 ID
    第一段  MIB 类配置
    第二段  非MIB类配置
*****************************************************************************/
typedef enum {
    /************************************************************************
        第三段 非MIB的内部数据同步，需要严格受控
    *************************************************************************/
    WLAN_CFGID_2_START = 2000,
    WLAN_CFGID_SET_MULTI_USER = 2000,
    WLAN_CFGID_USR_INFO_SYN = 2001,
    WLAN_CFGID_USER_ASOC_STATE_SYN = 2002,
    WLAN_CFGID_INIT_SECURTIY_PORT = 2004,
    WLAN_CFGID_USER_RATE_SYN = 2005,
#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) || defined(_PRE_WLAN_FEATURE_SMPS) || defined(_PRE_WLAN_FEATURE_M2S)
    WLAN_CFGID_USER_M2S_INFO_SYN = 2006, /* 更新opmode smps(m2s)的user相关信息 */
#endif
    WLAN_CFGID_USER_CAP_SYN = 2007, /* hmac向dmac同步mac user的cap能力信息 */

    WLAN_CFGID_SUSPEND_ACTION_SYN = 2008,
    WLAN_CFGID_SYNC_CH_STATUS = 2009,
    WLAN_CFGID_DYN_CALI_CFG = 2010,

#ifdef _PRE_WLAN_FEATURE_M2S
    WLAN_CFGID_DEVICE_M2S_INFO_SYN = 2011,
    WLAN_CFGID_VAP_M2S_INFO_SYN = 2012,
#endif

    WLAN_CFGID_PROFILING_PACKET_ADD = 2014,

    WLAN_CFGID_DBDC_DEBUG_SWITCH = 2015, /* DBDC开关 */

    WLAN_CFGID_VAP_MIB_UPDATE = 2016, /* d2h 根据hal cap更VAP mib 能力同步到host侧 */
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    WLAN_CFGID_SYNC_PK_MODE = 2017,
#endif
    WLAN_CFGID_VAP_CAP_UPDATE = 2018, /* d2h 根据hal cap同步到host侧 */

    WLAN_CFGID_DYN_EXTLNA_BYPASS_SWITCH = 2019, /* 动态外置LNA bypass开关 */

    WLAN_CFGID_VAP_CHANNEL_INFO_SYN = 2020,
    WLAN_CFGID_GET_MNGPKT_SENDSTAT = 2021, /* 获取管理帧发送状态 */

#ifdef _PRE_WLAN_DELAY_STATISTIC
    WLAN_CFGID_NOTIFY_STA_DELAY = 2022,
#endif
    WLAN_CFGID_PROBE_PESP_MODE         = 2023,

#ifdef _PRE_WLAN_FEATURE_DDR_BUGFIX
    WLAN_CFGID_SET_DDR          = 2024, /* 设置DDR status */
#endif
    WLAN_UL_OFDMA_AMSDU_SYN   = 2025,
    WLAN_CFGID_SET_SEND_TB_PPDU_FLAG = 2026, /* 配置当前发送过tb ppdu flag */
#ifdef _PRE_WLAN_FEATURE_HIEX
    WLAN_CFGID_USER_HIEX_ENABLE = 2027, /* 设置user hiex能力 */
#endif
    WLAN_CFGID_PS_RX_DELBA_TRIGGER = 2028,
    WLAN_CFGID_PS_ARP_PROBE = 2029,
    WLAN_CFGID_D2H_MAX_TX_POWER = 2031, /* d2h上报发送功率相关信息到host侧 */
#ifdef _PRE_WLAN_FEATURE_WMMAC
    WLAN_CFGID_D2H_WMMAC_PARAM = 2032,
#endif
    WLAN_CFGID_DEVICE_TCP_BUF = 2033,
    WLAN_CFGID_LOW_DELAY_STRATEGY = 2034, /* 低功耗与低时延高性能模式切换 */
    /* mp16独有,0X共用命令勿往此处放置 */
    WLAN_CFGID_MP16_PRIV_START = 4000,
    WLAN_CFGID_TX_TID_UPDATE = WLAN_CFGID_MP16_PRIV_START,
    WLAN_CFGID_TX_BA_COMP = 4001,
    WLAN_CFGID_RX_MODE_SWITCH_DEBUG = 4002,
    WLAN_CFGID_RX_MODE_SWITCH = 4003,
    WLAN_CFGID_HAL_VAP_UPDATE = 4004,
    WLAN_CFGID_LUT_USR_UPDATE = 4005,
    WLAN_CFGID_SET_AMSDU_AGGR_NUM = 4006,
    WLAN_CFGID_HW_WAPI = 4007, /* 同步是否支持硬件WAPI数据帧加解密 */
    WLAN_CFGID_EQUIPMENT_MODE = 4008,
    WLAN_CFGID_REPORT_TID_QUEUE_SIZE = 4010,

    WLAN_CFGID_HE_UORA_OCW_UPDATE = 4011,
    WLAN_CFGID_DMAC_SYNC_AMSDU_MAXNUM = 4012,
    WLAN_CFGID_PHY_EVENT_RPT = 4013,        /* PHY事件上报开关 */
    WLAN_CFGID_START_ANOTHER_DEV_CAC = 4014, /* 开启另一路cac检测 */
    WLAN_CFGID_SYNC_BT_STATE = 4015, /* D2H 同步BT状态 */
    WLAN_CFGID_TX_SWITCH_START = 4016, /* host/device ring tx切换事件 */
    WLAN_CFGID_ZERO_WAIT_DFS = 4017, /* 开启zero wait dfs */
    WLAN_CFGID_ZERO_WAIT_DFS_REPORT = 4018, /* 同步雷达信号检测结果至host侧 */
    WLAN_CFGID_GET_CALI_DATA = 4020,

    WLAN_CFGID_VSP_STATUS_CHANGE = 4021, /* device配置vsp状态 */
    WLAN_CFGID_TX_SCHE_SWITCH = 4022, /* tx调度模式切换事件 */

    WLAN_CFGID_GNSS_RSMC_STATUS_SYN = 4023, /* 同步GNSS接收短报文的状态到host */
    WLAN_CFGID_GNSS_RSMC_SHIFT_VAP_FINISH = 4024, /* 通知GNSS wifi侧已经处理完成 */
    WLAN_CFGID_GNSS_RSMC_SHIFT_VAP = 4025, /* 迁移VAP */
    WLAN_CFGID_HOST_RX_VIP_DATA_EVENT = 4026, /* host通知dev收到关键帧 */
    WLAN_CFGID_HOST_BA_INFO_RING_SYN = 4027, /* h2d同步host ba info ring信息 */
    WLAN_CFGID_HOST_RX_EAPOL_DATA_EVENT = 4028, /* h2d同步EAPOL信息 */
    WLAN_CFGID_PCIE_UP_PREPARE = 4029,
    WLAN_CFGID_PCIE_UP_PREPARE_SUCC = 4030,
    WLAN_CFGID_PCIE_UP_RECOVER = 4031,
    WLAN_CFGID_HOST_RX_PPS = 4032, /* host ring rx的pps统计 */

    WLAN_CFGID_SAR_SLIDE_DEBUG = 4033,
    WLAN_CFGID_COMPETE_ASCEND = 4035,
    WLAN_CFGID_UP_VMIN = 4036,
    WLAN_CFGID_VMIN_ABNORMAL_DEBUG = 4037,
    WLAN_CFGID_CLEAR_OLD_CHAN_CALI_DATA = 4038,
    WLAN_CFGID_MP16_PRIV_END,
    /* 61新增 */
    WLAN_CFGID_TX_POW_H2D = WLAN_CFGID_MP16_PRIV_END,
    WLAN_CFGID_GET_TX_POW,
    WLAN_CFGID_XO_DIFF_H2D,
    WLAN_CFGID_GET_XO_DIFF,
    WLAN_CFGID_SEND_XOCALI_DATA,
    WLAN_CFGID_SPMI_TEST,
    WLAN_CFGID_SSI_TEST,
    WLAN_CFGID_BUTT,
} wlan_cfgid2_enum;
#endif /* end of wlan_cfg_id.h */
