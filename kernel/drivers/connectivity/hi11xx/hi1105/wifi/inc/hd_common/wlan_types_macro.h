/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应所有公共的信息(HAL同时可以使用的)放到该文件中
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef WLAN_TYPES_MACRO_H
#define WLAN_TYPES_MACRO_H

/* 1 其他头文件包含 */
#include "platform_spec.h"
#include "oal_ext_if.h"
#include "wlan_cfg_id.h"
#include "wlan_frame_types.h"
#include "wlan_protocol_types.h"

/* 2.1 基本宏定义 */
/* AMPDU Delimeter长度(4字节) */
#define WLAN_DELIMETER_LENGTH 4

/* 取绝对值 */
#define GET_ABS(val) ((val) > 0 ? (val) : -(val))

/* 配置命令最大长度: 从算法名称开始算起，不包括"alg" */
#define DMAC_ALG_CONFIG_MAX_ARG 7

#define WLAN_CFG_MAX_ARGS_NUM               33 // 5x:33 mpxx:7
#define WLAN_CALI_CFG_MAX_ARGS_NUM          8  /* 参数过多会事件内存申请失败115x:20 */
#define WLAN_CALI_CFG_CMD_MAX_LEN           14 // 115x:20
/* WLAN CFG */
#define IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN 500
#define WLAN_CFG_HOST_MAX_RSP_LEN         300
#define WLAN_CFG_DEVICE_MAX_RSP_LEN       600
#define WLAN_CFG_MAX_RSP_LEN              (WLAN_CFG_HOST_MAX_RSP_LEN + WLAN_CFG_DEVICE_MAX_RSP_LEN + 100)
#define WLAN_CFG_MAX_LEN_EACH_ARG           50

/* 2G/5G子频段数目 */
#define WLAN_2G_SUB_BAND_NUM      13
#define WLAN_5G_SUB_BAND_NUM      7

#define WLAN_DIEID_MAX_LEN 40

#define wlan_aid(AID)       ((AID) & ~0xc000)

#ifdef _PRE_WLAN_FEATURE_160M
#define wlan_bandwidth_to_bw_cap(_bw) \
    ((WLAN_BAND_WIDTH_20M == (_bw)) ? \
    WLAN_BW_CAP_20M : ((WLAN_BAND_WIDTH_40PLUS == (_bw)) || (WLAN_BAND_WIDTH_40MINUS == (_bw))) ? \
    WLAN_BW_CAP_40M : (((_bw) >= WLAN_BAND_WIDTH_80PLUSPLUS) && ((_bw) <= WLAN_BAND_WIDTH_80MINUSMINUS)) ? \
    WLAN_BW_CAP_80M : (((_bw) >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) && ((_bw) <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) ? \
    WLAN_BW_CAP_160M : WLAN_BW_CAP_BUTT)
#else
#define wlan_bandwidth_to_bw_cap(_bw) \
    ((WLAN_BAND_WIDTH_20M == (_bw)) ? WLAN_BW_CAP_20M : \
    ((WLAN_BAND_WIDTH_40PLUS == (_bw)) || (WLAN_BAND_WIDTH_40MINUS == (_bw))) ?  WLAN_BW_CAP_40M : \
    (((_bw) >= WLAN_BAND_WIDTH_80PLUSPLUS) && ((_bw) <= WLAN_BAND_WIDTH_80MINUSMINUS)) ?  WLAN_BW_CAP_80M : \
    WLAN_BW_CAP_BUTT)
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
#define TWO_UP_VAP_NUMS 2   /* 两个up的vap个数 */
#endif
/* kernel oal_nl80211_chan_width convertion */
#define wlan_bandwidth_to_ieee_chan_width(_bw, _ht) \
    ((((_bw) == WLAN_BAND_WIDTH_20M) && (!(_ht))) ? NL80211_CHAN_WIDTH_20_NOHT : \
    (((_bw) == WLAN_BAND_WIDTH_20M) && (_ht)) ? NL80211_CHAN_WIDTH_20 : \
    (((_bw) == WLAN_BAND_WIDTH_40PLUS) || ((_bw) == WLAN_BAND_WIDTH_40MINUS)) ?  NL80211_CHAN_WIDTH_40 : \
    (((_bw) >= WLAN_BAND_WIDTH_80PLUSPLUS) && ((_bw) <= WLAN_BAND_WIDTH_80MINUSMINUS)) ?  NL80211_CHAN_WIDTH_80 : \
    (((_bw) >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) && ((_bw) <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) ?  \
    NL80211_CHAN_WIDTH_160 : WLAN_BW_CAP_BUTT)

#define WLAN_INVALD_VHT_MCS                           0xff
#define wlan_get_vht_max_support_mcs(_us_vht_mcs_map) \
    ((3 == (_us_vht_mcs_map)) ? \
    WLAN_INVALD_VHT_MCS : (2 == (_us_vht_mcs_map)) ? \
    WLAN_VHT_MCS9 : (1 == (_us_vht_mcs_map)) ? \
    WLAN_VHT_MCS8 : WLAN_VHT_MCS7)

/* 2.2 WME宏定义 */
#define WLAN_WME_AC_TO_TID(_ac) ( \
    ((_ac) == WLAN_WME_AC_VO) ? 6 : ((_ac) == WLAN_WME_AC_VI) ? 5 : ((_ac) == WLAN_WME_AC_BK) ? 1 : 0)

/* 标记bitmap */
#define wlan_wme_ac_to_all_tid(_bitmap, _ac)   \
    do { \
        ((_ac) == WLAN_WME_AC_VO) ? ((_bitmap) |= (uint8_t)BIT(WLAN_TIDNO_VOICE)) :   \
        ((_ac) == WLAN_WME_AC_VI) ? ((_bitmap) |= (uint8_t)BIT(WLAN_TIDNO_VIDEO)) :   \
        ((_ac) == WLAN_WME_AC_BE) ? ((_bitmap) |= (uint8_t)BIT(WLAN_TIDNO_BEST_EFFORT)) : (_bitmap); \
    } while (0)

/* 清除bitmap */
#define wlan_wme_ac_clear_all_tid(_bitmap, _ac)   \
    do { \
        ((_ac) == WLAN_WME_AC_VO) ? ((_bitmap) &= (uint8_t)(~(BIT(WLAN_TIDNO_VOICE)))) :   \
        ((_ac) == WLAN_WME_AC_VI) ? ((_bitmap) &= (uint8_t)(~(BIT(WLAN_TIDNO_VIDEO)))) :   \
        ((_ac) == WLAN_WME_AC_BE) ? ((_bitmap) &= (uint8_t)(~(BIT(WLAN_TIDNO_BEST_EFFORT)))) :  \
        ((_bitmap) &= (uint8_t)(~(BIT(WLAN_TIDNO_BACKGROUND)))); \
    } while (0)

#define WLAN_WME_TID_TO_AC(_tid) ( \
    (((_tid) == 0) || ((_tid) == 3)) ? WLAN_WME_AC_BE : (((_tid) == 1) || ((_tid) == 2)) ? \
    WLAN_WME_AC_BK : (((_tid) == 4) || ((_tid) == 5)) ? WLAN_WME_AC_VI : WLAN_WME_AC_VO)

#define wlan_invalid_frame_type(_uc_frame_type) \
    ((WLAN_FC0_TYPE_DATA != (_uc_frame_type)) && (WLAN_FC0_TYPE_MGT != (_uc_frame_type)) \
        && (WLAN_FC0_TYPE_CTL != (_uc_frame_type)))

#define WLAN_AMSDU_FRAME_MAX_LEN_LONG  7935

/* 2.5 过滤命令宏定义 */
#define WLAN_BIP_REPLAY_FAIL_FLT     BIT0 /* BIP重放攻击过滤 */
#define WLAN_CCMP_REPLAY_FAIL_FLT    BIT1 /* CCMP重放攻击过滤 */
#define WLAN_OTHER_CTRL_FRAME_FLT    BIT2 /* direct控制帧过滤 */
#define WLAN_BCMC_MGMT_OTHER_BSS_FLT BIT3 /* 其他BSS网络的组播管理帧过滤 */
/* 只有1151V100需要处理 */
#define WLAN_UCAST_MGMT_OTHER_BSS_FLT BIT4 /* 其他BSS网络的单播管理帧过滤 */

#define WLAN_UCAST_DATA_OTHER_BSS_FLT BIT5 /* 其他BSS网络的单播数据帧过滤 */

/* 2.7  TX & RX Chain Macro */
/* RF通道数规格 */
/* RF0对应bit0 */
#define WLAN_RF_CHANNEL_ZERO 0
/* RF1对应bit1 */
#define WLAN_RF_CHANNEL_ONE 1
/* RF2对应bit2 */
#define WLAN_RF_CHANNEL_TWO 2
/* RF3对应bit3 */
#define WLAN_RF_CHANNEL_THREE 3
/* 板级天线数定义 */
#define WLAN_ANT_NUM_FOUR 4
#define WLAN_ANT_NUM_THREE 3
#define WLAN_ANT_NUM_TWO 2
#define WLAN_ANT_NUM_ONE 1
/* PHY通道选择 */
/* 通道0对应bit0 */
#define WLAN_PHY_CHAIN_ZERO_IDX 0
/* 通道1对应bit1 */
#define WLAN_PHY_CHAIN_ONE_IDX 1
/* 通道2对应bit2 */
#define WLAN_PHY_CHAIN_TWO_IDX 2
/* 通道3对应bit3 */
#define WLAN_PHY_CHAIN_THREE_IDX 3

#define WLAN_RF_CHAIN_QUAD (BIT3 | BIT2 | BIT1 | BIT0)
#define WLAN_RF_CHAIN_C2C3 (BIT3 | BIT2)
#define WLAN_RF_CHAIN_C3  BIT3
/* 双道掩码,对应mp13 mp15,06不使用 */
#define WLAN_RF_CHAIN_DOUBLE 3
#define WLAN_RF_CHAIN_ONE    2
#define WLAN_RF_CHAIN_ZERO   1

/* 四通道掩码 */
#define WLAN_PHY_CHAIN_QUAD (BIT3 | BIT2 | BIT1 | BIT0)

/* 二通道掩码 */
/* C0C1 */
#define WLAN_PHY_CHAIN_C0C1 (BIT1 | BIT0)

/* 双道掩码,对应mp13 mp15,06不使用 */
#define WLAN_PHY_CHAIN_DOUBLE 3
#define WLAN_TX_CHAIN_DOUBLE  WLAN_PHY_CHAIN_DOUBLE
#define WLAN_RX_CHAIN_DOUBLE  WLAN_PHY_CHAIN_DOUBLE

/*  通道0 掩码0001 */
#define WLAN_PHY_CHAIN_ZERO BIT0
#define WLAN_TX_CHAIN_ZERO  WLAN_PHY_CHAIN_ZERO
/*  通道1 掩码0010 */
#define WLAN_PHY_CHAIN_ONE BIT1
#define WLAN_TX_CHAIN_ONE  WLAN_PHY_CHAIN_ONE

/*  通道2 掩码0100 */
#define WLAN_PHY_CHAIN_TWO BIT2
#define WLAN_TX_CHAIN_TWO  WLAN_PHY_CHAIN_TWO

/*  通道3 掩码1000 */
#define WLAN_PHY_CHAIN_THREE BIT3

#define WLAN_2G_CHANNEL_NUM 14
#define WLAN_5G_CHANNEL_NUM 29
#define WLAN_6G_CHANNEL_NUM 59

#define WLAN_MAX_CHANNEL_NUM (WLAN_2G_CHANNEL_NUM + WLAN_5G_CHANNEL_NUM)

/* PLL0 */
#define WLAN_RF_PLL_ID_0 0
/* PLL1 */
#define WLAN_RF_PLL_ID_1 1
/* 2.8 linkloss only device */
#define WLAN_LINKLOSS_REPORT        10  /* linkloss每10次打印一次维测 */
#define WLAN_LINKLOSS_MIN_THRESHOLD 10  /* linkloss门限最小最低值 */
#define WLAN_LINKLOSS_MAX_THRESHOLD 250 /* linkloss门限最大最高值 */

#define WLAN_MIB_TOKEN_STRING_MAX_LENGTH 32 /* 支持与网管兼容的token的字符串最长长度，此定义后续可能与版本相关 */
#endif
