/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :收发相关命令
 * 创建日期 : 2022年11月19日
 */
#include "oal_types.h"
#include "mac_vap.h"
#include "wal_config.h"
#include "hmac_vap.h"
#include "mac_mib.h"
#include "oal_util.h"
#include "plat_pm_wlan.h"
#include "mac_device.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_resource.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt.h"
#endif
#include "hmac_roam_main.h"
#include "hmac_tx_switch.h"
#include "wlan_chip_i.h"
#include "hmac_sae.h"
#include "wal_linux_ioctl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_TRX_CFG_IOCTL_C

#ifdef WIFI_DEBUG_ENABLE

/*
 * 功能描述  : 通过设置mib值, 设置AP的STBC能力
 * 1.日    期  : 2020年8月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_stbc_cap(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_bool_enum_uint8 cap_value = (oal_bool_enum_uint8)params[0];

    if (oal_unlikely(mac_vap->pst_mib_info == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_set_stbc_cap::pst_mac_vap->pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* siso能力时，配置命令不打开stbc的TX能力 */
    if (mac_vap->en_vap_rx_nss >= WLAN_DOUBLE_NSS) {
        mac_mib_set_TxSTBCOptionImplemented(mac_vap, cap_value);
        mac_mib_set_TxSTBCOptionActivated(mac_vap, cap_value);
        mac_mib_set_VHTTxSTBCOptionImplemented(mac_vap, cap_value);
        mac_mib_set_he_STBCTxBelow80M(mac_vap, cap_value);
    } else {
        mac_mib_set_TxSTBCOptionImplemented(mac_vap, OAL_FALSE);
        mac_mib_set_TxSTBCOptionActivated(mac_vap, OAL_FALSE);
        mac_mib_set_VHTTxSTBCOptionImplemented(mac_vap, OAL_FALSE);
        mac_mib_set_he_STBCTxBelow80M(mac_vap, OAL_FALSE);
    }

    mac_mib_set_RxSTBCOptionImplemented(mac_vap, cap_value);
    mac_mib_set_VHTRxSTBCOptionImplemented(mac_vap, cap_value);
    mac_mib_set_he_STBCRxBelow80M(mac_vap, cap_value);

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_set_stbc_cap::set stbc cap [%d].}", cap_value);
    return OAL_SUCC;
}
/*
 * 功能描述  : 通过设置mib值, 设置AP的LDPC能力
 * 1.日    期  : 2020年8月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_ldpc_cap(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_bool_enum_uint8 cap_value = (oal_bool_enum_uint8)params[0];

    if (oal_unlikely(mac_vap->pst_mib_info == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_set_ldpc_cap::pst_mac_vap->pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (cap_value > 1) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_set_ldpc_cap::ldpc_value is limit! value = [%d].}", cap_value);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    mac_mib_set_LDPCCodingOptionImplemented(mac_vap, cap_value);
    mac_mib_set_LDPCCodingOptionActivated(mac_vap, cap_value);
    mac_mib_set_VHTLDPCCodingOptionImplemented(mac_vap, cap_value);
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_mib_set_he_LDPCCodingInPayload(mac_vap, cap_value);
    }
#endif
    return OAL_SUCC;
}
/*
 * 功能描述  : 通过设置mib值, 设置txbf能力,bit0表示rx能力，bit1表示tx能力
 * 1.日    期  : 2020年8月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_txbf_cap(mac_vap_stru *mac_vap, uint32_t *params)
{
    /************************************************************
      TXBF设置开关的命令: sh hipriv "vap0 set_txbf_cap 0 | 1 | 2 |3"
             bit0表示RX(bfee)能力     bit1表示TX(bfer)能力
    *************************************************************/
    mac_device_stru *mac_device = NULL;
    uint8_t cap_value = (uint8_t)params[0];
    uint8_t rx_sts_num;
    oal_bool_enum_uint8 rx_switch;
    oal_bool_enum_uint8 tx_switch;

    if (oal_unlikely(mac_vap->pst_mib_info == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    rx_switch = cap_value & 0x1;
    tx_switch = (cap_value & 0x2) ? OAL_TRUE : OAL_FALSE;
    rx_sts_num = (rx_switch == OAL_TRUE) ? g_wlan_spec_cfg->vht_bfee_ntx_supports : 1;

    /* siso能力时，配置命令不打开txbf的TX能力 */
    if (mac_vap->en_vap_rx_nss >= WLAN_DOUBLE_NSS) {
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
        mac_mib_set_TransmitStaggerSoundingOptionImplemented(mac_vap, tx_switch);
        mac_vap->st_txbf_add_cap.bit_exp_comp_txbf_cap = tx_switch;
#endif
        mac_mib_set_VHTSUBeamformerOptionImplemented(mac_vap, tx_switch);
        mac_mib_set_VHTNumberSoundingDimensions(mac_vap, mac_device_get_nss_num(mac_device));
    } else {
        mac_mib_set_TransmitStaggerSoundingOptionImplemented(mac_vap, OAL_FALSE);
        mac_mib_set_VHTSUBeamformerOptionImplemented(mac_vap, OAL_FALSE);
        mac_mib_set_VHTNumberSoundingDimensions(mac_vap, WLAN_SINGLE_NSS);
    }

#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(mac_vap, rx_switch);
    if (rx_switch == OAL_FALSE) {
        mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(mac_vap, rx_sts_num);
    }
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(mac_vap, rx_switch & WLAN_MIB_HT_ECBF_DELAYED);
    mac_vap->st_txbf_add_cap.bit_channel_est_cap = rx_switch;
#endif
    mac_mib_set_VHTSUBeamformeeOptionImplemented(mac_vap, rx_switch);
    if (rx_switch == OAL_FALSE) {
        mac_mib_set_VHTBeamformeeNTxSupport(mac_vap, rx_sts_num);
    }
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        mac_mib_set_he_SUBeamformee(mac_vap, rx_switch);
    }
#endif
    oam_warning_log3(0, 0, "{wal_set_txbf_cap::rx_cap[%d], tx_cap[%d], rx_sts_nums[%d].}",
        rx_switch, tx_switch, rx_sts_num);

    return OAL_SUCC;
}

/*
 * 功能描述  : 开启或关闭ampdu发送功能
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_amsdu_tx_on(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t aggr_tx_on = (uint8_t)params[0];
    hmac_vap_stru *hmac_vap = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_set_amsdu_tx_on::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_mib_set_CfgAmsduTxAtive(&hmac_vap->st_vap_base_info, aggr_tx_on);

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_set_amsdu_tx_on::ENABLE[%d].}", aggr_tx_on);
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置be,bk,vi,vo的每次调度报文个数，lowwater_line, high_waterline
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_flowctl_param(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t idx = 0;
    uint8_t queue_type;
    uint16_t burst_limit;
    uint16_t low_waterline;
    uint16_t high_waterline;

    queue_type     = (uint8_t)params[idx++];   /* 获取队列类型参数 */
    burst_limit    = (uint16_t)params[idx++];  /* 设置队列对应的每次调度报文个数 */
    low_waterline  = (uint16_t)params[idx++];  /* 设置队列对应的流控low_waterline */
    high_waterline = (uint16_t)params[idx++];  /* 设置队列对应的流控high_waterline */

    /* 设置host flowctl 相关参数 */
    hcc_host_set_flowctl_param(queue_type, burst_limit, low_waterline, high_waterline, HCC_EP_WIFI_DEV);

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
                     "wal_set_flowctl_param, queue[%d]: burst limit = %d, low_waterline = %d, high_waterline =%d",
                     queue_type, burst_limit, low_waterline, high_waterline);
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/*
 * 功能描述  : 调整pkmode门限的debug接口
 * 1.日    期  : 2020年7月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_pk_mode_debug(mac_vap_stru *mac_vap, uint32_t *params)
{
    /* pkmode门限调整接口 hipriv "wlan0 pk_mode_debug 0/1(high/low) 0/1/2/3/4(BW) 0/1/2/3(protocol) 吞吐门限值" */
    /*
        BW:20M     40M    80M   160M   80+80M
        protocol:lagency: HT: VHT: HE:
    */
    /*
    PK模式门限基数:
    {(单位Mbps)  20M     40M    80M   160M   80+80M
    lagency:    {valid, valid, valid, valid, valid},   (基础协议模式没有pk mode )
    HT:         {72,    150,   valid, valid, valid},
    VHT:        {86,    200,   433,   866,   866},
    HE:         {valid, valid, valid, valid, valid},   (暂不支持11ax的pk mode)
    };

    PK模式二级门限:
    高档位门限: g_st_pk_mode_high_th_table = PK模式门限基数 * 70% *1024 *1024 /8  (单位字节)
    低档位门限: g_st_pk_mode_low_th_table  = PK模式门限基数 * 20% *1024 *1024 /8  (单位字节)

    */
    oam_warning_log4(0, OAM_SF_CFG, "{wal_set_pk_mode_debug::set high/low = %u, BW = %u, pro = %u, valid = %u!}",
                     params[BIT_OFFSET_0], params[BIT_OFFSET_1], params[BIT_OFFSET_2], params[BIT_OFFSET_3]);

    if (params[0] == 2) { /* 2表示pk_mode_debug high */
        g_en_pk_mode_swtich = OAL_TRUE;
        return OAL_SUCC;
    } else if (params[0] == 3) { /* 3表示pk_mode_debug low */
        g_en_pk_mode_swtich = OAL_FALSE;
        return OAL_SUCC;
    }

    if (params[0] == 0) {
        oam_warning_log2(0, OAM_SF_CFG, "{wal_set_pk_mode_debug::set high th (%u) -> (%u)!}",
                         g_st_pk_mode_high_th_table[params[BIT_OFFSET_2]][params[BIT_OFFSET_1]],
                         params[BIT_OFFSET_3]);
        g_st_pk_mode_high_th_table[params[BIT_OFFSET_2]][params[BIT_OFFSET_1]] = params[BIT_OFFSET_3];
        return OAL_SUCC;
    }

    oam_warning_log2(0, OAM_SF_CFG, "{wal_set_pk_mode_debug::set low th (%u) -> (%u)!}",
                     g_st_pk_mode_low_th_table[params[BIT_OFFSET_2]][params[BIT_OFFSET_1]],
                     params[BIT_OFFSET_3]);

    g_st_pk_mode_low_th_table[params[BIT_OFFSET_2]][params[BIT_OFFSET_1]] = params[BIT_OFFSET_3];

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
/*
 * 功能描述  : 设置所有用户帧上报的所有开关，如果是1，则上报所有类型帧的帧内容，
 *             cb字段，描述符；如果是0，则什么都不上报
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_txop_ps_machw(mac_vap_stru *mac_vap, uint32_t *params)
{
    /* sh hipriv.sh "stavap_name txopps_hw_en 0|1(txop_ps_en) 0|1(condition1) 0|1(condition2)" */
    /* 获取txop ps使能开关 */
    oal_switch_enum_uint8 machw_txopps_en = (oal_switch_enum_uint8)params[0];
    if (mac_vap->pst_mib_info == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_TXOP,
                         "{wal_set_txop_ps_machw::mib info is null, vap mode[%d].}", mac_vap->en_vap_mode);
        return OAL_FAIL;
    }
    /* mpxx txopps此测试接口通过设置mib项来打开功能，寄存器的打开在后续逻辑判断中执行 */
    mac_mib_set_txopps(mac_vap, machw_txopps_en);
    return OAL_SUCC;
}
#endif
/*
 * 功能描述  : 设置分片门限
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_frag_threshold(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint32_t frag_threshold = params[0]; /* 范围WLAN_FRAG_THRESHOLD_MIN ~ WLAN_FRAG_THRESHOLD_MAX */
    if (mac_vap->pst_mib_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_frag_threshold:pst_mib_info is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_mib_set_FragmentationThreshold(mac_vap, frag_threshold);
    return OAL_SUCC;
}

/*
 * 功能描述  : 设置所有用户的以太网帧上报开关，此开关设置以后，所有用户上报策略
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_all_ether_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    /* sh hipriv.sh "Hisilicon0 ether_all 0|1(帧方向tx|rx) 0|1(开关)" */
    uint8_t frame_direction = (uint8_t)params[0];
    uint8_t ether_switch = (uint8_t)params[1];
    uint16_t user_num;

    /* 设置开关 */
    for (user_num = 0; user_num < WLAN_USER_MAX_USER_LIMIT; user_num++) {
        oam_report_eth_frame_set_switch(user_num, ether_switch, frame_direction);
    }
    /* 同时设置广播arp dhcp帧的上报开关 */
    oam_report_dhcp_arp_set_switch(ether_switch);
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置发送广播dhcp和arp开关
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_dhcp_arp_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_switch_enum_uint8 dhcp_arp_switch = (oal_switch_enum_uint8)params[0];

    oam_report_dhcp_arp_set_switch(dhcp_arp_switch);
    return OAL_SUCC;
}

uint32_t wal_set_ota_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    oam_ota_switch_param_stru ota_swicth;
    ota_swicth.ota_type = (uint8_t)params[0];
    ota_swicth.ota_switch = (uint8_t)params[1];
    hmac_config_ota_switch(&ota_swicth);

    return OAL_SUCC;
}

/*
 * 功能描述  : 配置全局日志开关
 * 1.日    期  : 2020年7月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_global_log_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    oam_log_set_global_switch((oal_switch_enum_uint8)params[0]);
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置80211组播帧上报开关
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_80211_mcast_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    mac_cfg_80211_mcast_switch_stru mcast_switch = { 0 };
    oam_80211_frame_ctx_union oam_switch_param = { 0 };
    uint8_t idx = 0;
    uint32_t ret;
    /*
     * sh hipriv.sh "Hisilicon0 80211_mc_switch 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
     * 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
     */
    mcast_switch.en_frame_direction = (uint8_t)params[idx++]; /* 获取80211帧方向 */
    mcast_switch.en_frame_type      = (uint8_t)params[idx++]; /* 获取帧类型 */
    mcast_switch.en_frame_switch    = (uint8_t)params[idx++]; /* 获取帧内容打印开关 */
    mcast_switch.en_cb_switch       = (uint8_t)params[idx++]; /* 获取帧CB字段打印开关 */
    mcast_switch.en_dscr_switch     = (uint8_t)params[idx++]; /* 获取描述符打印开关 */

    /* 将配置命令结构转换为OAM结构 */
    oam_switch_param.frame_ctx.bit_content    = mcast_switch.en_frame_switch;
    oam_switch_param.frame_ctx.bit_cb         = mcast_switch.en_cb_switch;
    oam_switch_param.frame_ctx.bit_dscr       = mcast_switch.en_dscr_switch;

    ret = oam_report_80211_mcast_set_switch(mcast_switch.en_frame_direction,
                                            mcast_switch.en_frame_type,
                                            &oam_switch_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{wal_set_80211_mcast_switch::oam_report_80211_mcast_set_switch failed[%d].}", ret);
        return ret;
    }
    return OAL_SUCC;
}


#ifdef _PRE_WLAN_TCP_OPT
/*
 * 功能描述  : 查询TCP ACK过滤统计值
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_get_tcp_ack_stream_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_get_tcp_ack_stream_info fail: hmac_vap is null}");
        return OAL_FAIL;
    }

    hmac_tcp_opt_ack_show_count(hmac_vap);
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置发送TCP ACK优化使能
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_tcp_tx_ack_opt_enable(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint32_t tcp_ack_opt_val;
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_set_tcp_tx_ack_opt_enable fail: hmac_device is null}");
        return OAL_FAIL;
    }

    tcp_ack_opt_val = params[0];
    if (tcp_ack_opt_val == 0) {
        hmac_device->sys_tcp_tx_ack_opt_enable = OAL_FALSE;
    } else {
        hmac_device->sys_tcp_tx_ack_opt_enable = OAL_TRUE;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_tcp_tx_ack_opt_enable:sys_tcp_tx_ack_opt_enable = %d}",
                     hmac_device->sys_tcp_tx_ack_opt_enable);
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置接收TCP ACK优化使能
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_tcp_rx_ack_opt_enable(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint32_t tcp_rx_ack_opt_val;
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_set_tcp_rx_ack_opt_enable fail: hmac_device is null}");
        return OAL_FAIL;
    }

    tcp_rx_ack_opt_val = params[0];
    if (tcp_rx_ack_opt_val == 0) {
        hmac_device->sys_tcp_rx_ack_opt_enable = OAL_FALSE;
    } else {
        hmac_device->sys_tcp_rx_ack_opt_enable = OAL_TRUE;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_tcp_rx_ack_opt_enable:sys_tcp_tx_ack_opt_enable = %d}",
                     hmac_device->sys_tcp_rx_ack_opt_enable);
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置发送ACK门限值
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_tcp_tx_ack_limit(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint32_t tcp_tx_ack_limit_val;
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_set_tcp_tx_ack_limit fail: hmac_vap is null}");
        return OAL_FAIL;
    }

    tcp_tx_ack_limit_val = params[0];
    if (tcp_tx_ack_limit_val >= DEFAULT_TX_TCP_ACK_THRESHOLD) {
        hmac_vap->st_hmac_tcp_ack[HCC_TX].filter_info.ack_limit = DEFAULT_TX_TCP_ACK_THRESHOLD;
    } else {
        hmac_vap->st_hmac_tcp_ack[HCC_TX].filter_info.ack_limit = tcp_tx_ack_limit_val;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_tcp_tx_ack_limit:ul_ack_limit = %ld}",
                     hmac_vap->st_hmac_tcp_ack[HCC_TX].filter_info.ack_limit);
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置接收ACK门限值
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_tcp_rx_ack_limit(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint32_t tcp_rx_ack_limit_val;
    hmac_vap_stru *hmac_vap;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_set_tcp_rx_ack_limit fail: hmac_vap is null}");
        return OAL_FAIL;
    }

    tcp_rx_ack_limit_val = params[0];

    if (tcp_rx_ack_limit_val >= DEFAULT_RX_TCP_ACK_THRESHOLD) {
        hmac_vap->st_hmac_tcp_ack[HCC_RX].filter_info.ack_limit = DEFAULT_RX_TCP_ACK_THRESHOLD;
    } else {
        hmac_vap->st_hmac_tcp_ack[HCC_RX].filter_info.ack_limit = tcp_rx_ack_limit_val;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_tcp_rx_ack_limit:ul_ack_limit = %ld}",
                     hmac_vap->st_hmac_tcp_ack[HCC_RX].filter_info.ack_limit);
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#define WAL_TCP_ACK_BUF_TIMEOUT_MS_INVALID_START 0
#define WAL_TCP_ACK_BUF_TIMEOUT_MS_INVALID_END (10 * 1000)

typedef struct {
    int8_t *puc_string;
    mac_tcp_ack_buf_cfg_cmd_enum_uint8 en_tcp_ack_buf_cfg_id;
    uint8_t auc_resv[NUM_3_BYTES];
} mac_tcp_ack_buf_cfg_table_stru;

static mac_tcp_ack_buf_cfg_table_stru g_ast_hmac_tcp_ack_buf_cfg_table[] = {
    { "enable",  MAC_TCP_ACK_BUF_ENABLE },
    { "timeout", MAC_TCP_ACK_BUF_TIMEOUT },
    { "count",   MAC_TCP_ACK_BUF_MAX },
};

OAL_STATIC uint32_t wal_hipriv_tcp_ack_buf_analyze(oal_net_device_stru *net_dev, uint8_t uc_idx,
    wal_msg_write_stru *write_msg, uint32_t result, int32_t l_tmp)
{
    mac_cfg_tcp_ack_buf_stru *pst_tcp_ack_param = (mac_cfg_tcp_ack_buf_stru *)(write_msg->auc_value);
    int32_t ret;
    if (pst_tcp_ack_param->en_cmd == MAC_TCP_ACK_BUF_ENABLE) {
        if (((uint8_t)l_tmp == OAL_FALSE) || ((uint8_t)l_tmp == OAL_TRUE)) {
            pst_tcp_ack_param->en_enable = (uint8_t)l_tmp;
        } else {
            oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::fast_aging_flag[%d]:0/1}", (uint8_t)l_tmp);
            return OAL_FAIL;
        }
    }
    if (pst_tcp_ack_param->en_cmd == MAC_TCP_ACK_BUF_TIMEOUT) {
        if ((uint8_t)l_tmp == 0) {
            oam_warning_log0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: timer_ms shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_tcp_ack_param->uc_timeout_ms = (uint8_t)l_tmp;
    }
    if (pst_tcp_ack_param->en_cmd == MAC_TCP_ACK_BUF_MAX) {
        if ((uint8_t)l_tmp == 0) {
            oam_warning_log0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: uc_count_limit shoule not be 0.}");
            return OAL_FAIL;
        }

        pst_tcp_ack_param->uc_count_limit = (uint8_t)l_tmp;
    }

    wal_write_msg_hdr_init(write_msg, WLAN_CFGID_TCP_ACK_BUF, sizeof(mac_cfg_tcp_ack_buf_stru));

    oam_warning_log4(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::cmd[%d],enable[%d],timeout_ms[%d] count_limit[%d]!}",
        pst_tcp_ack_param->en_cmd, pst_tcp_ack_param->en_enable,
        pst_tcp_ack_param->uc_timeout_ms, pst_tcp_ack_param->uc_count_limit);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tcp_ack_buf_stru), (uint8_t *)write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::return err code [%d]!}", result);
        return (uint32_t)result;
    }

    return OAL_SUCC;
}
uint32_t wal_hipriv_tcp_ack_buf_cfg(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    int32_t l_tmp;
    uint8_t uc_idx;
    mac_cfg_tcp_ack_buf_stru *pst_tcp_ack_param = (mac_cfg_tcp_ack_buf_stru *)(write_msg.auc_value);
    memset_s((void *)pst_tcp_ack_param, sizeof(mac_cfg_tcp_ack_buf_stru),
             0, sizeof(mac_cfg_tcp_ack_buf_stru));

    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::get cmd one arg err[%d]!}", result);
        return result;
    }
    pc_param = pc_param + off_set;

    for (uc_idx = 0; uc_idx < MAC_TCP_ACK_BUF_TYPE_BUTT; uc_idx++) {
        if (0 == oal_strcmp(ac_name, g_ast_hmac_tcp_ack_buf_cfg_table[uc_idx].puc_string)) {
            break;
        }
    }
    if (uc_idx == MAC_TCP_ACK_BUF_TYPE_BUTT) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg:: parameter error !");
        return OAL_FAIL;
    }

    pst_tcp_ack_param->en_cmd = g_ast_hmac_tcp_ack_buf_cfg_table[uc_idx].en_tcp_ack_buf_cfg_id;

    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::get cmd one arg err[%d]!}", result);
        return result;
    }

    l_tmp = (int32_t)oal_atoi(ac_name);
    if ((l_tmp < WAL_TCP_ACK_BUF_TIMEOUT_MS_INVALID_START) || (l_tmp > WAL_TCP_ACK_BUF_TIMEOUT_MS_INVALID_END)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_tcp_ack_buf_cfg::  car param[%d] invalid! }", oal_atoi(ac_name));
        return OAL_FAIL;
    }

    return wal_hipriv_tcp_ack_buf_analyze(net_dev, uc_idx, &write_msg, result, l_tmp);
}
#endif
OAL_STATIC void wal_hipriv_always_tx_write_msg(wal_msg_write_stru *write_msg, uint8_t en_tx_flag,
    mac_rf_payload_enum_uint8 en_payload_flag, uint32_t len)
{
    mac_cfg_tx_comp_stru *al_tx_param = (mac_cfg_tx_comp_stru *)(write_msg->auc_value);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(write_msg, WLAN_CFGID_SET_ALWAYS_TX, sizeof(mac_cfg_tx_comp_stru));

    al_tx_param->en_payload_flag = en_payload_flag;
    al_tx_param->payload_len = len;
    al_tx_param->uc_param = en_tx_flag;
    al_tx_param->write_index = 0;

    oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_always_tx_write_msg::tx_flag[%d],len[%d]!}", en_tx_flag, len);
}
/*
 * 函 数 名  : wal_hipriv_always_tx
 * 功能描述  : 数据常发
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_always_tx(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t en_tx_flag;
    mac_rf_payload_enum_uint8 en_payload_flag = RF_PAYLOAD_RAND;
    uint32_t len = 2000;
    wal_msg_stru *rsp_msg = NULL;

    /* 获取常发模式开关标志 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::get cmd one arg err[%d]!}", ret);
        return ret;
    }
    en_tx_flag = (uint8_t)oal_atoi(ac_name);
    if (en_tx_flag > HAL_ALWAYS_TX_MPDU) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_always_tx::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* 关闭的情况下不需要解析后面的参数 */
    if (en_tx_flag != HAL_ALWAYS_TX_DISABLE) {
        /* ack_policy参数后续扩充 */
        /* 获取payload_flag参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        /* 若后面payload_flag和len参数没有设置，采用默认RF_PAYLOAD_RAND 2000 */
        if (ret == OAL_SUCC) {
            en_payload_flag = (uint8_t)oal_atoi(ac_name);
            if (en_payload_flag >= RF_PAYLOAD_BUTT) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::payload flag err[%d]!}", en_payload_flag);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }

            /* 获取len参数 */
            pc_param = pc_param + off_set;
            ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::get cmd one arg err[%d]!}", ret);
                return ret;
            }
            len = (uint32_t)oal_atoi(ac_name);
        }
    }

    wal_hipriv_always_tx_write_msg(&write_msg, en_tx_flag, en_payload_flag, len);

    ret = (uint32_t)wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_tx::send event err [%d]!}", ret);
        return ret;
    }

    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(rsp_msg);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}
const int8_t *g_ru_size_tbl[WLAN_HE_RU_SIZE_BUTT] = {
    "26tone",
    "52tone",
    "106tone",
    "242tone",
    "484tone",
    "996tone",
    "1992tone"
};
/*
 * 功能描述  : 设置RU index
 * 1.日    期  : 2020年3月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_ru_index(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t ret;
    mac_cfg_tx_comp_stru *set_param;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_RU_INDEX, sizeof(mac_cfg_tx_comp_stru));
    /* 解析并设置配置命令参数 */
    set_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ru_index::get cmd one arg err[%d]!}", ret);
        return ret;
    }

    for (set_param->uc_param = 0; set_param->uc_param < WLAN_HE_RU_SIZE_BUTT; set_param->uc_param++) {
        if (!oal_strcmp(g_ru_size_tbl[set_param->uc_param], name)) {
            break;
        }
    }
    /* 检查命令是否打错 */
    if (set_param->uc_param >= WLAN_HE_RU_SIZE_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ru_index::not support this ru size!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    param = param + off_set;
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ru_index::get cmd one arg err[%d]!}", ret);
        return ret;
    }
    set_param->en_payload_flag = (uint8_t)oal_atoi(name);

    param = param + off_set;
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    set_param->en_protocol_mode = (uint8_t)oal_atoi(name);

    ret = (uint32_t)wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(mac_cfg_tx_comp_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ru_index::err [%d]!}", ret);
        return ret;
    }

    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_ru_index fail, err code[%u]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_always_rx
 * 功能描述  : 数据常收
 * 1.日    期  : 2014年3月28日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_always_rx(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_al_rx(net_dev, pc_param);
}
#ifdef _PRE_WLAN_FEATURE_SNIFFER
/*
 * 函 数 名  : wal_hipriv_set_sniffer
 * 功能描述  : 设置sniffer开关
 * 1.日    期  : 2019年5月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_sniffer(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_sniffer(net_dev, pc_param);
}
#endif
/*
 * 函 数 名  : wal_hipriv_enable_monitor_mode
 * 功能描述  : 设置monitor开关
 * 1.日    期  : 2019年5月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_enable_monitor_mode(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_monitor_mode(net_dev, pc_param);
}

#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
/*
 * 函 数 名  : wal_hipriv_set_mcsax_er
 * 功能描述  : 设置HE ER模式下的速率
 * 1.日    期  : 2019年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_mcsax_er(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_hipriv_set_mcs_by_cfgid(net_dev, pc_param, WLAN_CFGID_SET_MCSAX_ER,
                                       WAL_HIPRIV_HE_ER_MCS_MIN, WAL_HIPRIV_HE_ER_MCS_MAX);
}

uint32_t _wal_hipriv_set_mcsax_er(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_11ax_er_su_dcm_is_open) {
        return wal_hipriv_set_mcsax_er(net_dev, pc_param);
    }

    return OAL_SUCC;
}
#endif


/*
 * 函 数 名  : wal_hipriv_rx_fcs_info
 * 功能描述  : 打印对应vap的接收FCS的信息
 * 1.日    期  : 2014年3月8日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_rx_fcs_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t ret;
    uint32_t result;
    mac_cfg_rx_fcs_info_stru *pst_rx_fcs_info = NULL;
    mac_cfg_rx_fcs_info_stru st_rx_fcs_info; /* 临时保存获取的use的信息 */

    /* 打印接收帧的FCS正确与错误信息:sh hipriv.sh "vap0 rx_fcs_info 0/1 1-4" 0/1  0代表不清除，1代表清除 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::wal_get_cmd_one_arg  err_code %d!}", result);
        return result;
    }

    st_rx_fcs_info.data_op = (uint32_t)oal_atoi(ac_name);

    if (st_rx_fcs_info.data_op > 1) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info:: data_op cmd is error!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::get cmd one arg err%d!}", result);
        return result;
    }

    st_rx_fcs_info.print_info = (uint32_t)oal_atoi(ac_name);

    if (st_rx_fcs_info.print_info > 4) { // 4代表参数数值上限
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info:: print_info cmd is error!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_RX_FCS_INFO, sizeof(mac_cfg_rx_fcs_info_stru));

    /* 设置配置命令参数 */
    pst_rx_fcs_info = (mac_cfg_rx_fcs_info_stru *)(write_msg.auc_value);
    pst_rx_fcs_info->data_op = st_rx_fcs_info.data_op;
    pst_rx_fcs_info->print_info = st_rx_fcs_info.print_info;

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_rx_fcs_info_stru), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_rx_fcs_info::err %d!}", ret);
        return (uint32_t)ret;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_hipriv_sdio_flowctrl
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2015年3月30日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_sdio_flowctrl(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;

    if (oal_unlikely(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param))) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_sdio_flowctrl:: pc_param overlength:%d}", OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    /* 拷贝的时候目的地址要预留1字节为后面填充结束符使用,否则可能导致内存越界写风险 */
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value) - 1, pc_param, OAL_STRLEN(pc_param));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_sdio_flowctrl::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SDIO_FLOWCTRL, len);

    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_sdio_flowctrl::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_alg(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;  // st_write_msg can only carry bytes less than 48
    int32_t l_ret;
    uint32_t off_set;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    int8_t *pc_tmp = (int8_t *)pc_param;
    uint16_t us_config_len;
    uint16_t us_param_len;
    uint32_t size = sizeof(mac_ioctl_alg_config_stru);
    mac_ioctl_alg_config_stru st_alg_config;

    st_alg_config.uc_argc = 0;
    while (wal_get_cmd_one_arg(pc_tmp, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set) == OAL_SUCC) {
        st_alg_config.auc_argv_offset[st_alg_config.uc_argc] = (uint8_t)((uint8_t)(pc_tmp - pc_param) +
            (uint8_t)off_set - (uint8_t)OAL_STRLEN(ac_arg));
        pc_tmp += off_set;
        st_alg_config.uc_argc++;
        if (st_alg_config.uc_argc > DMAC_ALG_CONFIG_MAX_ARG) {
            oam_error_log1(0, OAM_SF_ANY,
                "{wal_hipriv_alg::wal_hipriv_alg error, argc too big [%d]!}\r\n", st_alg_config.uc_argc);
            return OAL_FAIL;
        }
    }

    if (st_alg_config.uc_argc == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_alg::argc=0!}\r\n");
        return OAL_FAIL;
    }

    us_param_len = (uint16_t)OAL_STRLEN(pc_param);
    if (us_param_len > WAL_MSG_WRITE_MAX_LEN - 1 - size) {
        return OAL_FAIL;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    us_config_len = size + us_param_len + 1;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ALG, us_config_len);
    l_ret = memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), &st_alg_config, size);
    l_ret += memcpy_s(st_write_msg.auc_value + size,
                      sizeof(st_write_msg.auc_value) - size, pc_param, us_param_len + 1);
    if (l_ret != EOK) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_alg::memcpy fail! [%d]}", l_ret);
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_config_len,
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg::wal_send_cfg_event return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_alg_cfg_entry
 * 功能描述  : 算法模块参数配置
 *       配置算法的配置命令: sh hipriv.sh "wlan0 alg txmode_mode_sw <value>"
 *       该命令针对某一个VAP
 * 1.日    期  : 2020/08/25
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_alg_cfg_entry(oal_net_device_stru *net_dev, int8_t *param)
{
    uint32_t result, off_set;
    char args_str[IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN] = { 0 };
    int8_t cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /* 获取配置参数名称 */
    result = wal_get_cmd_one_arg(param, cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_entry::get_cmd_one_arg failed! [%d]!}\r\n", result);
        return result;
    }

    if (oal_strcmp(cmd_name, "dbac") == 0) {
        return wal_hipriv_alg(net_dev, param);
    }

    result = (uint32_t)sprintf_s(args_str, IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN, "%s", param);
    if (result < 0) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_entry ::snprintf_s return ERR!!}");
        return OAL_FAIL;
    }

    result = wal_algcmd_char_extra_adapt(net_dev, (char *)args_str, IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN,
        NULL, 0);
    if (result != OAL_SUCC) {
        oal_io_print("wal_hipriv_alg_cfg_entry:: return err_code [%d]", result);
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_entry::algcmd fail.}");
    }

    return result;
}
#ifdef _PRE_WLAN_DELAY_STATISTIC
/*
 * 函 数 名  : wal_hipriv_pkt_time_switch
 * 功能描述  : hipriv命令控制dmac空口时延统计的开关
 * 1.日    期  : 2019年09月14日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_pkt_time_switch(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t off_set = 0;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_msg_write_stru write_msg = {0};
    user_delay_switch_stru st_switch_cmd;
    int32_t ret;

    /* 获取命令字符串 */
    uint32_t result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg err[%d]!}", result);
        return result;
    }

    memset_s(&st_switch_cmd, sizeof(user_delay_switch_stru), 0, sizeof(user_delay_switch_stru));
    if (0 == (oal_strcmp("on", ac_arg))) {
        st_switch_cmd.dmac_stat_enable = 1;

        /* 第二个参数，统计数据帧的数量 */
        pc_param += off_set;
        result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg 2nd err:%d}", result);
            return result;
        }
        st_switch_cmd.dmac_packet_count_num = (uint32_t)oal_atoi(ac_arg);

        /* 第三个参数，上报间隔 */
        pc_param += off_set;
        result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_pkt_time_switch::wal_get_cmd_one_arg 2nd err:%d}", result);
            return result;
        }
        st_switch_cmd.dmac_report_interval = (uint32_t)oal_atoi(ac_arg);
    } else if (0 == (oal_strcmp("off", ac_arg))) {
        st_switch_cmd.dmac_stat_enable = 0;
        st_switch_cmd.dmac_packet_count_num = 0;
        st_switch_cmd.dmac_report_interval = 0;
    } else {
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_pkt_time_switch::invalid parameter!}");
        return OAL_FAIL;
    }
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), &st_switch_cmd, sizeof(st_switch_cmd));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "wal_hipriv_pkt_time_switch::memcpy fail!");
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_PKT_TIME_SWITCH, sizeof(st_switch_cmd));
    result = (uint32_t)wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_switch_cmd), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_pkt_time_switch::return err code %d!}", result);
    }
    return result;
}
#endif
/*
 * 功能描述  : 设置是否上报rx描述符的开关
 * 1.日    期  : 2015年3月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_ota_rx_dscr_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_param;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    int32_t l_ret;
    uint32_t ret;

    /* OAM ota模块的开关的命令: hipriv "Hisilicon0 ota_rx_dscr_switch 0 | 1"
    */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ota_rx_dscr_switch::wal_get_cmd_one_arg fails!}\r\n");
        return ret;
    }

    /* 解析参数 */
    ret = wal_get_cmd_one_arg(pc_param + off_set, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ota_rx_dscr_switch::wal_get_cmd_one_arg fails!}\r\n");
        return ret;
    }
    l_param = oal_atoi((const int8_t *)ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_OTA_RX_DSCR_SWITCH, sizeof(uint32_t));
    *((int32_t *)(st_write_msg.auc_value)) = l_param;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_ota_rx_dscr_switch::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

#endif
