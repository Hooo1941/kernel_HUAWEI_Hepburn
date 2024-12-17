/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_chan_mgmt.c 的头文件
 * 作    者 :
 * 创建日期 : 2014年2月22日
 */

#ifndef HMAC_CHAN_MGMT_H
#define HMAC_CHAN_MGMT_H

/* 1 其他头文件包含 */
#include "hmac_vap.h"
#include "mac_regdomain.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 2 宏定义 */
#define HMAC_CHANNEL_SWITCH_COUNT 5

/* 3 枚举定义 */
typedef enum {
    HMAC_OP_ALLOWED = BIT0,
    HMAC_SCA_ALLOWED = BIT1,
    HMAC_SCB_ALLOWED = BIT2,
} hmac_chan_op_enum;

typedef enum {
    HMAC_NETWORK_SCA = 0,
    HMAC_NETWORK_SCB = 1,

    HMAC_NETWORK_BUTT,
} hmac_network_type_enum;

typedef enum {
    MAC_CHNL_AV_CHK_NOT_REQ = 0,  /* 不需要进行信道扫描 */
    MAC_CHNL_AV_CHK_IN_PROG = 1,  /* 正在进行信道扫描 */
    MAC_CHNL_AV_CHK_COMPLETE = 2, /* 信道扫描已完成 */

    MAC_CHNL_AV_CHK_BUTT,
} mac_chnl_av_chk_enum;

/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    uint16_t aus_num_networks[HMAC_NETWORK_BUTT];
    uint8_t en_chan_op;
    uint8_t auc_resv[3];  // 保留3字节
} hmac_eval_scan_report_stru;

typedef struct {
    uint8_t uc_idx;   /* 信道索引号 */
    uint16_t us_freq; /* 信道频点 */
    uint8_t auc_resv;
} hmac_dfs_channel_info_stru;

typedef struct {
    uint8_t channel_nums_2g; /* 2G信道个数 */
    uint8_t channel_nums_5g; /* 5G信道个数 */
    uint8_t resv[2];
} hmac_channel_nums_stat_stru;

typedef enum {
    SPECIAL_DBDC_SUBBAND_5G_LOW  = 0,
    SPECIAL_DBDC_SUBBAND_5G_HIGH = 1,
    SPECIAL_DBDC_SUBBAND_6G_LOW  = 2,
    SPECIAL_DBDC_SUBBAND_6G_HIGH = 3,
    SPECIAL_DBDC_SUBBAND_BUTT
} mac_special_dbdc_subband_enum;

typedef enum {
    SPECIAL_DBDC_TYPE_NOT_SUPPORT    = 0,
    SPECIAL_DBDC_TYPE_5G_LOW_5G_HIGH = 1,
    SPECIAL_DBDC_TYPE_5G_HIGH_5G_LOW = 2,
    SPECIAL_DBDC_TYPE_5G_HIGH_6G_LOW = 3,
    SPECIAL_DBDC_TYPE_6G_LOW_5G_HIGH = 4,
    SPECIAL_DBDC_TYPE_6G_LOW_6G_HIGH = 5,
    SPECIAL_DBDC_TYPE_6G_HIGH_6G_LOW = 6,
    SPECIAL_DBDC_TYPE_MAX_NUM,
} mac_special_dbdc_type_enum;

typedef enum {
    WLAN_5G_SUB_BAND0 = 0,
    WLAN_5G_SUB_BAND1,
    WLAN_5G_SUB_BAND2,
    WLAN_5G_SUB_BAND3,
    WLAN_5G_SUB_BAND4,
    WLAN_5G_SUB_BAND5,
    WLAN_5G_SUB_BAND6,
} wlan_5g_sub_band_enum;

typedef enum {
    MAC_MVAP_STATE_DCHN        = 0, /* 多vap 同频同信道 */
    MAC_MVAP_STATE_DBDC        = 1, /* 多vap dbdc */
    MAC_MVAP_STATE_DBAC        = 2, /* 多vap dbac */
    MAC_MVAP_STATE_DBDC_DBAC   = 3, /* 多vap dbdc + dbac */
    MAC_MVAP_STATE_SINGLE_VAP  = 4, /* 单vap状态 */
    MAC_MVAP_STATE_UNSUPPORTED = 5, /* 不支持的状态 */
    MAC_MVAP_STATE_BUTT
} mac_mvap_state_enum;

typedef enum {
    MAC_MVAP_STATE_TRANS_DCHN_TO_DCHN = 0, /* 同频同信道状态不变 */
    MAC_MVAP_STATE_TRANS_DCHN_TO_DBDC = 1, /* 同频同信道到dbdc */
    MAC_MVAP_STATE_TRANS_DCHN_TO_DBAC = 2, /* 同频同信道到dbac */
    MAC_MVAP_STATE_TRANS_DCHN_TO_DBDC_AC = 3, /* 同频同信道到dbdc + dbac */
    MAC_MVAP_STATE_TRANS_DCHN_TO_SINGLE_VAP = 4, /* 同频同信道状态到单vap */

    MAC_MVAP_STATE_TRANS_DBDC_TO_DCHN = 5, /* dbdc到同频同信道切换 */
    MAC_MVAP_STATE_TRANS_DBDC_TO_DBDC = 6, /* dbdc状态不变 */
    MAC_MVAP_STATE_TRANS_DBDC_TO_DBAC = 7, /* dbdc到dbac状态切换 */
    MAC_MVAP_STATE_TRANS_DBDC_TO_DBDC_AC = 8, /* dbdc到dbdc + dbac */
    MAC_MVAP_STATE_TRANS_DBDC_TO_SINGLE_VAP = 9, /* dbac状态到单vap */

    MAC_MVAP_STATE_TRANS_DBAC_TO_DCHN = 10, /* dbac状态到同信道状态 */
    MAC_MVAP_STATE_TRANS_DBAC_TO_DBDC = 11, /* dbac状态到dbdc状态切换 */
    MAC_MVAP_STATE_TRANS_DBAC_TO_DBAC = 12, /* 切换信道后仍然停留在dbac状态 */
    MAC_MVAP_STATE_TRANS_DBAC_TO_DBDC_AC = 13, /* dbac状态到dbdc + dbac */
    MAC_MVAP_STATE_TRANS_DBAC_TO_SINGLE_VAP = 14, /* dbac状态到单vap */

    MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DCHN = 15, /* dbdc + dbac到同频同信道 */
    MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBDC = 16, /* dbdc + dbac到dbdc */
    MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBAC = 17, /* dbdc + dbac到dbac */
    MAC_MVAP_STATE_TRANS_DBDC_AC_TO_DBDC_AC = 18, /* dbdc + dbac到dbdc + dbac */
    MAC_MVAP_STATE_TRANS_DBDC_AC_TO_SINGLE_VAP = 19, /* dbac状态到单vap */

    MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_DCHN = 20, /* 单vap到同频同信道 */
    MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_DBDC = 21, /* 单vap到dbdc */
    MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_DBAC = 22, /* 单vap到dbac */
    MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_DBDC_AC = 23, /* 单vap到dbdc+dbac */
    MAC_MVAP_STATE_TRANS_SINGLE_VAP_TO_SINGLE_VAP = 24, /* 单vap切换 */

    MAC_MVAP_STATE_TRANS_UNSUPPORTED = 25, /* CSA切换不支持 */

    MAC_MVAP_STATE_TRANS_BUTT
} mac_mvap_state_trans_enum;

typedef enum {
    MAC_MVAP_CHAN_SWITCH_STA_ROAM = 0, /* 漫游切信道 */
    MAC_MVAP_CHAN_SWITCH_GO_CSA = 1, /* go csa切信道 */
    MAC_MVAP_CHAN_SWITCH_CHBA_CSA = 2, /* chba csa切信道 */

    MAC_MVAP_CHAN_SWITCH_BUTT
} mac_mvap_chan_switch_type_enum;

/* 频率信道索引映射关系 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
void hmac_chan_reval_bandwidth_sta(mac_vap_stru *pst_mac_vap, uint32_t change);


void hmac_chan_multi_select_channel_mac(mac_vap_stru *pst_mac_vap,
                                        uint8_t uc_channel,
                                        wlan_channel_bandwidth_enum_uint8 en_bandwidth);
uint32_t hmac_start_bss_in_available_channel(hmac_vap_stru *pst_hmac_vap);
void hmac_chan_restart_network_after_switch(mac_vap_stru *pst_mac_vap);
uint32_t hmac_chan_switch_to_new_chan_complete(frw_event_mem_stru *pst_event_mem);
void hmac_chan_sync(mac_vap_stru *pst_mac_vap,
                    uint8_t uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth,
                    oal_bool_enum_uint8 en_switch_immediately);
void hmac_chan_update_40m_intol_user(mac_vap_stru *pst_mac_vap);

void hmac_40m_intol_sync_data(mac_vap_stru *pst_mac_vap,
                              wlan_channel_bandwidth_enum_uint8 en_40m_bandwidth,
                              oal_bool_enum_uint8 en_40m_intol_user);

uint8_t hmac_get_80211_band_type(mac_channel_stru *channel);

void hmac_chan_initiate_switch_to_new_channel(mac_vap_stru *pst_mac_vap,
                                              uint8_t uc_channel,
                                              wlan_channel_bandwidth_enum_uint8 en_bandwidth);
uint32_t hmac_check_ap_channel_follow_other_vap(mac_vap_stru *check_mac_vap,
    mac_cfg_channel_param_stru *channel_param, mac_channel_stru *set_mac_channel);
uint16_t mac_get_center_freq1_from_freq_and_bandwidth(uint16_t freq,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth);
uint32_t hmac_report_channel_switch(hmac_vap_stru *hmac_vap, mac_channel_stru *channel_info);
oal_bool_enum_uint8 hmac_jugde_dual_5g_dbdc(const mac_channel_stru *chan_info,
    const mac_channel_stru *other_vap_chan_info);
oal_bool_enum_uint8 hmac_check_is_support_single_dbdc(void);
oal_bool_enum_uint8 hmac_check_is_support_dual_5g_dbdc(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num);
oal_bool_enum_uint8 hmac_check_coex_channel_is_valid(mac_vap_stru *mac_vap,
    mac_channel_stru *set_mac_channel);
mac_mvap_state_trans_enum hmac_chan_switch_get_state_trans(mac_vap_stru **up_mac_vap, mac_vap_stru *mac_vap,
    mac_channel_stru *new_channel, uint32_t up_vap_num, mac_mvap_chan_switch_type_enum chan_switch_type);
uint32_t hmac_ch_status_info_syn_event(frw_event_mem_stru *pst_event_mem);
/* 11 inline函数定义 */
/*
 * 函 数 名  : hmac_chan_initiate_switch_to_20mhz_ap
 * 功能描述  : 设置VAP信道参数，准备切换至20MHz运行
 * 备    注  : 频宽切换，不会改变频带(2.4G or 5G)、以及主信道号，因此不用设置
 *             vap.st_channel.uc_chan_number和vap.st_channel.en_band，只需改变
 *             带宽模式vap.st_channel.en_bandwidth
 * 1.日    期  : 2014年2月21日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void hmac_chan_initiate_switch_to_20mhz_ap(mac_vap_stru *pst_mac_vap)
{
    /* 设置VAP带宽模式为20MHz */
    pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;

    /* 设置带宽切换状态变量，表明在下一个DTIM时刻切换至20MHz运行 */
    pst_mac_vap->st_ch_switch_info.en_bw_switch_status = WLAN_BW_SWITCH_40_TO_20;
}

/*
 * 函 数 名  : hmac_chan_scan_availability
 * 1.日    期  : 2014年10月16日
 *   修改内容  : 新生成函数
 *   备    注  :该函数仅mp12使用，mp13使用时需要将mac_dfs_get_dfs_enable替换为mac_vap_get_dfs_enable
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_chan_scan_availability(mac_device_stru *pst_mac_device,
                                                                      mac_ap_ch_info_stru *pst_channel_info)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    if (OAL_FALSE == mac_dfs_get_dfs_enable(pst_mac_device)) {
        return OAL_TRUE;
    }

    return ((pst_channel_info->en_ch_status != MAC_CHAN_NOT_SUPPORT) &&
            (pst_channel_info->en_ch_status != MAC_CHAN_BLOCK_DUE_TO_RADAR));
#else
    return OAL_TRUE;
#endif
}

/*
 * 函 数 名  : hmac_chan_is_ch_op_allowed
 * 1.日    期  : 2014年11月4日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_chan_is_ch_op_allowed(
    hmac_eval_scan_report_stru *pst_chan_scan_report, uint8_t uc_chan_idx)
{
    return (pst_chan_scan_report[uc_chan_idx].en_chan_op & HMAC_OP_ALLOWED);
}

/*
 * 功能描述 : 判断两vap信道配置是否组成dbac
 */
oal_bool_enum_uint8 hmac_chan_is_dbac_cfg(const mac_channel_stru *channel1,
    const mac_channel_stru *channel2);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_chan_mgmt.h */
