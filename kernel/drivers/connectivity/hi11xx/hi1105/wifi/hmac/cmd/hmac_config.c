/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 配置相关实现hmac接口实现源文件
 * 作    者 :
 * 创建日期 : 2013年1月8日
 */

/* 1 头文件包含 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/pm_qos.h>
#endif

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "wlan_chip_i.h"
#include "hmac_wifi_delay.h"
#include "hmac_device.h"
#include "mac_resource.h"
#include "hmac_resource.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "mac_function.h"
#include "oal_hcc_host_if.h"
#include "mac_user.h"
#include "mac_regdomain.h"
#include "host_hal_device.h"
#include "hmac_vsp_if.h"
#include "hmac_ext_if.h"
#include "hmac_fsm.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#include "hmac_tx_amsdu.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_config.h"
#include "hmac_chan_mgmt.h"
#include "hmac_rx_filter.h"
#include "hmac_psm_ap.h"
#include "hmac_protection.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_rx_filter.h"
#include "hmac_mgmt_sta.h"
#include "hmac_arp_probe.h"
#include "hmac_blacklist.h"
#include "hmac_degradation.h"
#include "hmac_scan.h"
#include "hmac_dfs.h"
#include "hmac_reset.h"
#include "hmac_blockack.h"
#include "hmac_p2p.h"
#include "hmac_mgmt_ap.h"
#include "hmac_tx_switch.h"
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#include "external/oal_pm_qos_interface.h"
#include "hmac_dyn_pcie.h"

#include "hmac_arp_offload.h"
#include "hmac_vowifi.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "mac_data.h"
#include "hmac_tcp_opt.h"
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_board.h"
#endif
#include "hmac_auto_adjust_freq.h"

#include "hmac_dfx.h"
#include "hmac_host_al_tx.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif

#include "hmac_sme_sta.h"
#include "securec.h"
#include "securectype.h"

#include "hmac_roam_connect.h"
#ifdef _PRE_WLAN_FEATURE_TWT
#include "hmac_twt.h"
#endif

#include "wlan_mib.h"
#include "hmac_ht_self_cure.h"

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#include "hmac_11ax.h"
#endif
#include "host_hal_ext_if.h"
#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
#include "hmac_rr_performance.h"
#endif
#ifdef _PRE_WLAN_FEATURE_GNSS_RSMC
#include "hmac_gnss_rsmc.h"
#endif
#include "hmac_tx_complete.h"
#ifdef _PRE_WLAN_FEATURE_HID2D
#include "hmac_hid2d.h"
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#include "hmac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_MBO
#include "hmac_mbo.h"
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#include "hmac_sae.h"
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#include "hmac_tcp_ack_buf.h"
#endif
#include "mac_mib.h"
#include "hmac_11w.h"
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif
#include "hmac_btcoex.h"

#ifdef _PRE_WLAN_FEATURE_DFR
#include "hmac_dfx.h"
#endif  /* _PRE_WLAN_FEATURE_DFR */

#include "wal_linux_bridge.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "wal_linux_atcmdsrv.h"
#include "hisi_conn_nve_interface_gf61.h"
#endif

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
#include "hmac_mcast_ampdu.h"
#endif
#include "hmac_tx_ring_alloc.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_chan_switch.h"
#include "hmac_chba_frame.h"
#include "hmac_chba_user.h"
#include "hmac_chba_coex.h"
#include "hmac_chba_ps.h"
#include "hmac_chba_sync.h"
#include "hmac_chba_mgmt.h"
#include "hmac_chba_channel_sequence.h"
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
#include "hmac_pwl.h"
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
#include "hmac_lp_miracast.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_C

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*
 * 函 数 名  : hmac_config_alloc_event
 * 功能描述  : pst_mac_vap: 指向vap
 * 1.日    期  : 2013年1月18日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_alloc_event(mac_vap_stru *pst_mac_vap, hmac_to_dmac_syn_type_enum_uint8 en_syn_type,
    hmac_to_dmac_cfg_msg_stru **ppst_syn_msg, frw_event_mem_stru **ppst_event_mem, uint16_t us_len)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = NULL;

    pst_event_mem = frw_event_alloc_m(us_len + sizeof(hmac_to_dmac_cfg_msg_stru) - 4); /* 4:事件id + payload Length */
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_alloc_event::pst_event_mem null, us_len = %d }", us_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填充事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_HOST_CRX, en_syn_type,
        (us_len + sizeof(hmac_to_dmac_cfg_msg_stru) - 4), /* 4:事件id + payload Length */
        FRW_EVENT_PIPELINE_STAGE_1, pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    /* 出参赋值 */
    *ppst_event_mem = pst_event_mem;
    *ppst_syn_msg = (hmac_to_dmac_cfg_msg_stru *)pst_event->auc_event_data;

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_send_event
 * 功能描述  : 抛事件到DMAC层, 同步DMAC数据
 * 1.日    期  : 2013年1月18日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_send_event(mac_vap_stru *pst_mac_vap, wlan_cfgid_enum_uint16 en_cfg_id,
                                uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    frw_event_mem_stru *pst_event_mem = NULL;
    hmac_to_dmac_cfg_msg_stru *pst_syn_msg = NULL;

    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (wlan_chip_h2d_cmd_need_filter(en_cfg_id) == OAL_TRUE) {
        return OAL_SUCC;
    }

    ret = hmac_config_alloc_event(pst_mac_vap, HMAC_TO_DMAC_SYN_CFG, &pst_syn_msg, &pst_event_mem, us_len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_event::hmac_config_alloc_event failed[%d].}", ret);
        return ret;
    }

    pst_syn_msg->en_syn_id = en_cfg_id;
    pst_syn_msg->us_len = us_len;
    /* 填写配置同步消息内容 */
    if ((puc_param != NULL) && (us_len)) {
        if (EOK != memcpy_s(pst_syn_msg->auc_msg_body, (uint32_t)us_len, puc_param, (uint32_t)us_len)) {
            oam_error_log0(0, OAM_SF_CFG, "hmac_config_send_event::memcpy fail!");
            frw_event_free_m(pst_event_mem);
            return OAL_FAIL;
        }
    }

    /* 抛出事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_send_event::dispatch failed[%d].}", ret);
        frw_event_free_m(pst_event_mem);
        return ret;
    }

    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_set_mode_event
 * 功能描述  : 设置模式事件，抛事件给dmac侧
 * 1.日    期  : 2015年8月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_set_mode_event(mac_vap_stru *mac_vap)
{
    uint32_t ret;
    mac_cfg_mode_param_stru prot_param = {0};

    /* 设置带宽模式，直接抛事件到DMAC配置寄存器 */
    prot_param.en_protocol = mac_vap->en_protocol;
    prot_param.en_band = mac_vap->st_channel.en_band;
    prot_param.en_bandwidth = mac_vap->st_channel.en_bandwidth;

    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MODE, sizeof(mac_cfg_mode_param_stru), (uint8_t *)&prot_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_set_mode_event::mode_set failed[%d],protocol[%d], band[%d], bandwidth[%d].}",
            ret, mac_vap->en_protocol, mac_vap->st_channel.en_band, mac_vap->st_channel.en_bandwidth);
    }
    return ret;
}

/*
 * 函 数 名  : hmac_config_sync_cmd_common
 * 功能描述  : 通用的从hmac同步命令到dmac函数
 * 1.日    期  : 2013年5月31日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_sync_cmd_common(mac_vap_stru *pst_mac_vap, wlan_cfgid_enum_uint16 en_cfg_id,
                                     uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, en_cfg_id, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sync_cmd_common::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_open_wmm
 * 功能描述  : 更新host侧mib信息位中的Qos位置
 * 1.日    期  : 2017年8月14日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_open_wmm(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    oal_bool_enum_uint8 en_wmm;

    ret = hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_WMM_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WMMAC, "{hmac_config_open_wmm:: failed[%d].}", ret);
        return ret;
    }

    en_wmm = *(oal_bool_enum_uint8 *)puc_param;
    /* 开关WMM，更新host侧mib信息位中的Qos位置 */
    mac_mib_set_dot11QosOptionImplemented(pst_mac_vap, en_wmm);

    return ret;
}

/*
 * 函 数 名  : hmac_config_get_wmmswitch
 * 功能描述  : hmac get wmm switch
 * 1.日    期  : 2017年03月09日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_wmmswitch(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_get_wmmswitch::wmm switch[%d].}", pst_mac_device->en_wmm);

    /* get wmm_en status */
    *puc_param = pst_mac_device->en_wmm;
    *pus_len = sizeof(int32_t);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_mib_by_bw
 * 功能描述  : 根据带宽设置mib能力
 * 1.日    期  : 2016年06月21日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_mib_by_bw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_mib_by_bw_param_stru *pst_cfg = (mac_cfg_mib_by_bw_param_stru *)puc_param;
    uint32_t ret;

    if ((pst_mac_vap == NULL) || (puc_param == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_mib_by_bw::input params null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap_change_mib_by_bandwidth(pst_mac_vap, pst_cfg->en_band, pst_cfg->en_bandwidth);

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CFG80211_SET_MIB_BY_BW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_mib_by_bw::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC void hmac_config_protocol_debug_printf_11ax_info(mac_vap_stru *pst_mac_vap)
{
    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_protocol_debug_printf_11ax_info::he_mib_enable=[%d],hal_dev_11ax_enable=[%d],11ax_switch=[%d].",
        mac_mib_get_HEOptionImplemented(pst_mac_vap), is_hal_device_support_11ax(pst_mac_vap),
        is_custom_open_11ax_switch(pst_mac_vap));
}
#endif

static uint32_t hmac_config_csa_debug_verify_channel_valid(mac_vap_stru *mac_vap,
    mac_protocol_debug_switch_stru *cfg_info)
{
    uint32_t up_vap_num;
    mac_vap_stru *up_mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    mac_channel_stru new_channel = { 0 };
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    uint32_t ret;

    ret = mac_is_channel_num_valid(mac_vap->st_channel.en_band,
        cfg_info->st_csa_debug_bit3.uc_channel, mac_vap->st_channel.ext6g_band);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_csa_debug_handle::mac_is_channel_num_valid(%d),return.",
            cfg_info->st_csa_debug_bit3.uc_channel);
        return ret;
    }

    up_vap_num = mac_device_find_up_vaps(mac_device, up_mac_vap, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    if (up_vap_num == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_csa_debug_handle::mac_device_find_up_vaps fail.}");
        return OAL_FAIL;
    }

    new_channel = mac_vap->st_channel;
    new_channel.uc_chan_number = cfg_info->st_csa_debug_bit3.uc_channel;
    if (mac_get_channel_idx_from_num(new_channel.en_band, new_channel.uc_chan_number,
        new_channel.ext6g_band, &new_channel.uc_chan_idx) != OAL_SUCC) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_csa_debug_handle::can not get channel idx.");
        return OAL_FAIL;
    }

    /* 识别原状态和目标状态，避免去dbac和非法状态 */
    if (hmac_chan_switch_get_state_trans(up_mac_vap, mac_vap, &new_channel, up_vap_num,
        MAC_MVAP_CHAN_SWITCH_GO_CSA) == MAC_MVAP_STATE_TRANS_UNSUPPORTED) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC void hmac_config_csa_debug_handle(mac_vap_stru *mac_vap,
    mac_protocol_debug_switch_stru *cfg_info, uint8_t *go_on_flag)
{
    uint8_t go_debug_csa_param[2] = {0}; // go csa参数共2字节

    *go_on_flag = OAL_FALSE;

    if (cfg_info->st_csa_debug_bit3.en_debug_flag == MAC_CSA_FLAG_GO_DEBUG) {
        go_debug_csa_param[0] = cfg_info->st_csa_debug_bit3.uc_channel;
        go_debug_csa_param[1] = cfg_info->st_csa_debug_bit3.en_bandwidth;

        hmac_p2p_change_go_channel(mac_vap, 2 * sizeof(uint8_t), go_debug_csa_param);  /* 信道号+带宽 2字节 */
        return;
    }

    if (!is_ap(mac_vap)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_csa_debug_handle::is not ap,return.");
        return;
    }

    if (hmac_config_csa_debug_verify_channel_valid(mac_vap, cfg_info) != OAL_SUCC) {
        return;
    }

    /* 根据device能力对参数进行检查 */
    if ((cfg_info->st_csa_debug_bit3.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (mac_mib_get_dot11VapMaxBandWidth(mac_vap) < WLAN_BW_CAP_80M)) {
        cfg_info->st_csa_debug_bit3.en_bandwidth =
            mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(mac_vap), cfg_info->st_csa_debug_bit3.en_bandwidth);

        /* 设置80M带宽，但device能力不支持80M， 刷新成合适带宽做业务 */
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_csa_debug_handle::not support 80MHz bandwidth, csa_new_bandwidth=%d, VAP MaxBandWidth=%d.}",
            cfg_info->st_csa_debug_bit3.en_bandwidth, mac_mib_get_dot11VapMaxBandWidth(mac_vap));
    }

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
        "hmac_config_csa_debug_handle::csa_mode=%d csa_channel=%d, csa_cnt=%d debug_flag=%d",
        cfg_info->st_csa_debug_bit3.en_mode, cfg_info->st_csa_debug_bit3.uc_channel,
        cfg_info->st_csa_debug_bit3.uc_cnt, cfg_info->st_csa_debug_bit3.en_debug_flag);
    *go_on_flag = OAL_TRUE;

    if (cfg_info->st_csa_debug_bit3.en_debug_flag == MAC_CSA_FLAG_NORMAL) {
        mac_vap->st_ch_switch_info.en_csa_mode = cfg_info->st_csa_debug_bit3.en_mode;
        mac_vap->st_ch_switch_info.uc_ch_switch_cnt = cfg_info->st_csa_debug_bit3.uc_cnt;
        hmac_chan_initiate_switch_to_new_channel(mac_vap, cfg_info->st_csa_debug_bit3.uc_channel,
            cfg_info->st_csa_debug_bit3.en_bandwidth);
        *go_on_flag = OAL_FALSE;
        return;
    }
}

/*
 * 函 数 名  : hmac_config_protocol_debug_switch
 * 功能描述  : 设置带宽的开关
 * 1.日    期  : 2017年2月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_protocol_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t go_on_flag;
    mac_protocol_debug_switch_stru *pst_protocol_debug;
    oal_bool_enum_uint8 en_40_intolerant;

    pst_protocol_debug = (mac_protocol_debug_switch_stru *)puc_param;

    /* 恢复40M带宽命令 */
    if (pst_protocol_debug->cmd_bit_map & BIT0) {
        /* host无需处理，dmac处理 */
    }

    /* 不容忍40M带宽开关 */
    if (pst_protocol_debug->cmd_bit_map & BIT2) {
        /* 只有2.4G才设置该mib值 */
        if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band) {
            en_40_intolerant = (pst_protocol_debug->en_40_intolerant_bit2 == OAL_TRUE) ? OAL_TRUE : OAL_FALSE;
            mac_mib_set_FortyMHzIntolerant(pst_mac_vap, en_40_intolerant);
        } else {
            oam_warning_log1(0, OAM_SF_CFG,
                "{hmac_config_protocol_debug_switch::band is not 2G,but [%d].", pst_mac_vap->st_channel.en_band);
        }
    }
    /* csa cmd */
    if (pst_protocol_debug->cmd_bit_map & BIT3) {
        hmac_config_csa_debug_handle(pst_mac_vap, pst_protocol_debug, &go_on_flag);
        if (go_on_flag == OAL_FALSE) {
            return OAL_SUCC;
        }
    }
    /* lsigtxop使能 */
    if (pst_protocol_debug->cmd_bit_map & BIT5) {
        mac_mib_set_LsigTxopFullProtectionActivated(pst_mac_vap, pst_protocol_debug->en_lsigtxop_bit5);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (pst_protocol_debug->cmd_bit_map & BIT6) {
            hmac_config_protocol_debug_printf_11ax_info(pst_mac_vap);
        }
    }
#endif
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROTOCOL_DBG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_protocol_debug_switch::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_lte_gpio_mode
 * 功能描述  : 输入参数
 * 1.日    期  : 2015年7月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_lte_gpio_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_CHECK_LTE_GPIO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_lte_gpio_mode::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_ota_rx_dscr_switch
 * 功能描述  : rx_dscr的OTA开关
 * 1.日    期  : 2015年3月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_ota_rx_dscr_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    int32_t l_value;

    l_value = *((int32_t *)puc_param);

    /* 此处为了兼容mp13/mp15,06不使用 */
    oam_ota_set_switch(OAM_OTA_SWITCH_RX_PPDU_DSCR, (oal_switch_enum_uint8)l_value);
    oam_ota_set_switch(OAM_OTA_SWITCH_RX_MSDU_DSCR, (oal_switch_enum_uint8)l_value);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_OTA_RX_DSCR_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_ota_rx_dscr_switch::hmac_config_send_event fail[%d].", ret);
    }

    return ret;
}

 /*
 * 函 数 名  : hmac_config_ota_switch
 * 功能描述  : 设置OTA上报开关
 * 1.日    期  : 2020年7月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_ota_switch(oam_ota_switch_param_stru *ota_swicth)
{
    oam_ota_set_switch(ota_swicth->ota_type, ota_swicth->ota_switch);
}
/*
 * 函 数 名  : hmac_config_set_random_mac_addr_scan
 * 功能描述  : 设置随机mac addr扫描开关, 0关闭，1打开
 * 1.日    期  : 2015年5月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_random_mac_addr_scan(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_device_stru *pst_hmac_device = NULL;
    oal_bool_enum_uint8 en_random_mac_addr_scan_switch;

    en_random_mac_addr_scan_switch = *((oal_bool_enum_uint8 *)puc_param);

    /* 获取hmac device结构体 */
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_random_mac_addr_scan::pst_hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    g_wlan_cust.uc_random_mac_addr_scan = en_random_mac_addr_scan_switch;
#else
    pst_hmac_device->st_scan_mgmt.en_is_random_mac_addr_scan = en_random_mac_addr_scan_switch;
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_random_mac_oui
 * 功能描述  : 设置随机mac oui
 * 1.日    期  : 2016年9月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_random_mac_oui(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_device_stru *pst_mac_device = NULL;
    hmac_device_stru *pst_hmac_device = NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::pst_hmac_device is null.device_id %d}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (us_len < WLAN_RANDOM_MAC_OUI_LEN) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui::len is short:%d.}", us_len);
        return OAL_FAIL;
    }

    if (memcpy_s(pst_mac_device->auc_mac_oui, WLAN_RANDOM_MAC_OUI_LEN, puc_param, us_len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_random_mac_oui::memcpy fail!");
        return OAL_FAIL;
    }

    /* 系统会在启动wifi 后下发非零mac_oui, wps扫描或hilink连接的场景中,将mac_oui清0,
     * mac_oui 非零时生成扫描随机MAC, wifi 扫描时使用该作为源地址 */
    if ((pst_mac_device->auc_mac_oui[0] != 0) ||
        (pst_mac_device->auc_mac_oui[BYTE_OFFSET_1] != 0) || (pst_mac_device->auc_mac_oui[BYTE_OFFSET_2] != 0)) {
        oal_random_ether_addr(pst_hmac_device->st_scan_mgmt.auc_random_mac, WLAN_MAC_ADDR_LEN);
        pst_hmac_device->st_scan_mgmt.auc_random_mac[BYTE_OFFSET_0] =
            pst_mac_device->auc_mac_oui[BYTE_OFFSET_0] & 0xfe; /* 保证是单播mac */
        pst_hmac_device->st_scan_mgmt.auc_random_mac[BYTE_OFFSET_1] = pst_mac_device->auc_mac_oui[BYTE_OFFSET_1];
        pst_hmac_device->st_scan_mgmt.auc_random_mac[BYTE_OFFSET_2] = pst_mac_device->auc_mac_oui[BYTE_OFFSET_2];

        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_set_random_mac_oui::rand_mac_addr[%02X:XX:XX:XX:%02X:%02X].}",
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_0],
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_4],
                         pst_hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_5]);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RANDOM_MAC_OUI, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_random_mac_oui::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_update_ip_filter
 * 功能描述  : rx ip数据包过滤功能的相关参数配置接口
 * 1.日    期  : 2017年4月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_update_ip_filter(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    dmac_tx_event_stru *pst_tx_event = NULL;
    frw_event_mem_stru *pst_event_mem = NULL;
    oal_netbuf_stru *pst_netbuf_cmd = NULL;
    frw_event_stru *pst_hmac_to_dmac_ctx_event = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_update_ip_filter::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_netbuf_cmd = *((oal_netbuf_stru **)puc_param);
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_update_ip_filter::pst_event_mem null.}");
        oal_netbuf_free(pst_netbuf_cmd);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_to_dmac_ctx_event = (frw_event_stru *)pst_event_mem->puc_data;
    frw_event_hdr_init(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
        FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_IP_FILTER,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1,
        pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    pst_tx_event = (dmac_tx_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_tx_event->pst_netbuf = pst_netbuf_cmd;
    pst_tx_event->us_frame_len = oal_netbuf_len(pst_netbuf_cmd);

    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_update_ip_filter::frw_event_dispatch_event failed[%d].}", ret);
    }
    oal_netbuf_free(pst_netbuf_cmd);
    frw_event_free_m(pst_event_mem);

    return ret;
}

/*
 * 函 数 名  : hmac_config_log_level
 * 功能描述  : 日志级别设置，hmac 层抛向dmac 层
 * 1.日    期  : 2015年1月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_log_level(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_LOG_LEVEL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_log_level::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_TPC_OPT
/*
 * 函 数 名  : hmac_config_reduce_sar
 * 功能描述  : 降SAR
 * 1.日    期  : 2014年8月27日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_reduce_sar(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REDUCE_SAR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_reduce_sar::hmac_config_send_event failed, error no[%d]!", ret);
        return ret;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
/*
 * 函 数 名  : hmac_config_tas_pwr_ctrl
 * 功能描述  : tas功率控制
 * 1.日    期  : 2018年7月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_tas_pwr_ctrl(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TAS_PWR_CTRL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_tas_pwr_ctrl::hmac_config_send_event failed, error no[%d]!", ret);
        return ret;
    }
    return OAL_SUCC;
}
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
/*
 * 函 数 名  : hmac_config_tas_rssi_access
 * 功能描述  : tas功率控制
 * 1.日    期  : 2018年7月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_tas_rssi_access(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TAS_RSSI_ACCESS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_tas_rssi_access::hmac_config_send_event failed, error no[%d]!", ret);
        return ret;
    }
    return OAL_SUCC;
}
#endif

uint32_t hmac_config_memory_monitor_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MEMORY_MONITOR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_memory_monitor_info::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_sdio_flowctrl
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2015年3月30日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_sdio_flowctrl(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SDIO_FLOWCTRL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sdio_flowctrl::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_DELAY_STATISTIC
/*
 * 函 数 名  : hmac_config_pkt_time_switch
 * 功能描述  : dmac空口时延统计控制开关
 * 1.日    期  : 2018年9月12日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_pkt_time_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数
    **************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PKT_TIME_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_pkt_time_switch::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif /* _PRE_WLAN_DELAY_STATISTIC */
/*
 * 函 数 名  : hmac_config_send_bar
 * 功能描述  : 指定用户的指定tid发送bar
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_send_bar(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SEND_BAR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_send_bar::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#ifdef PLATFORM_DEBUG_ENABLE
/*
 * 函 数 名  : hmac_config_reg_info
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2013年5月31日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_reg_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REG_INFO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reg_info::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_reg_write
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2013年9月6日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_reg_write(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_REG_WRITE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reg_write::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif

#ifdef _PRE_WLAN_EXPORT
/*
 * 函 数 名  : hmac_config_mac_addr_write
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2023年08月08日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_mac_addr_write(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MAC_ADDR_WRITE, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_mac_addr_write::send_event failed[%d].}",
                         ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_mac_addr_read
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2023年08月08日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_mac_addr_read(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MAC_ADDR_READ, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_mac_addr_read::send_event failed[%d].}",
                         ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_report_mac_addr
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2023年08月08日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_report_mac_addr(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap;
    uint8_t i;
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_report_mac_addr::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (i = 0; i < NUM_3_BYTES; i++) {
        g_wlan_device_mac_addr[NUM_5_BYTES - i] = *(param++);
    }

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    hmac_vap->uc_efuse_mac_addr_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(hmac_vap->query_wait_q));

    return OAL_SUCC;
}
#endif


/*
 * 函 数 名  : hmac_config_reg_write
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2013年9月6日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_dpd_cfg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DPD, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reg_write::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_sar_slide_cfg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SAR_SLIDE_DEBUG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_sar_slide_cfg::hmac_config_send_event failed, error no[%d]!", ret);
        return ret;
    }
    return OAL_SUCC;
}

uint32_t hmac_config_compete_pow_cfg(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_COMPETE_ASCEND, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_config_compete_pow_cfg::hmac_config_send_event failed, error no[%d]!", ret);
    }
    return ret;
}

/*
 * 函 数 名   : hmac_config_cali_debug
 * 功能描述   : hmac校准配置命令;
 * 1.日    期   : 2020年06月22日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t hmac_config_cali_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CALI_CFG, len, param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_cali_debug::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/*
 * 函 数 名  : hmac_11v_sta_tx_query
 * 功能描述  : 11V 触发接口，触发STA发送bss transition management query 帧
 * 1.日    期  : 2017年2月28日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_11v_sta_tx_query(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    /* 仅STA模式下支持触发发送query帧 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_11v_sta_tx_query::vap mode:[%d] not support this.}", pst_mac_vap->en_vap_mode);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (mac_mib_get_MgmtOptionBSSTransitionActivated(pst_mac_vap) == OAL_FALSE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_11v_sta_tx_query::en_dot11MgmtOptionBSSTransitionActivated is FALSE.}");
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_11V_TX_QUERY, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_11v_sta_tx_query::hmac_config_send_event failed[%d].}", ret);
    }
    return OAL_SUCC;
}
#endif

/*
 * 函 数 名  : hmac_config_set_2040_coext_support
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2014年6月5日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_2040_coext_support(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if ((*puc_param != 0) && (*puc_param != 1)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_2040_coext_support::invalid param[%d].", *puc_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 如果是配置VAP, 直接返回 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_2040_coext_support::this is config vap! can't set.}");
        return OAL_FAIL;
    }

    mac_mib_set_2040BSSCoexistenceManagementSupport(pst_mac_vap, (oal_bool_enum_uint8)(*puc_param));
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                  "{hmac_config_set_2040_coext_support::end func,puc_param=%d.}", *puc_param);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_pm_by_module
 * 功能描述  : 低功耗控制接口
 * 1.日    期  : 2015年9月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_pm_by_module(mac_vap_stru *pst_mac_vap,
                                      mac_pm_ctrl_type_enum pm_ctrl_type, mac_pm_switch_enum pm_enable)
{
    uint32_t ret;
    mac_cfg_ps_open_stru st_ps_open = { 0 };

    if (pm_enable >= MAC_STA_PM_SWITCH_BUTT ||
        pm_ctrl_type >= MAC_STA_PM_CTRL_TYPE_BUTT ||
        pst_mac_vap == NULL) {
        oam_error_log2(0, OAM_SF_ANY,
                       "hmac_config_set_pm_by_module, PARAM ERROR! pm_ctrl_type=%d, pm_enable=%d ",
                       pm_ctrl_type, pm_enable);
        return OAL_FAIL;
    }

    st_ps_open.uc_pm_enable = pm_enable;
    st_ps_open.uc_pm_ctrl_type = pm_ctrl_type;

    ret = hmac_config_set_sta_pm_on(pst_mac_vap, sizeof(mac_cfg_ps_open_stru), (uint8_t *)&st_ps_open);

    oam_warning_log3(0, OAM_SF_PWR, "hmac_config_set_pm_by_module, pm_module=%d, pm_enable=%d, cfg ret=%d ",
                     pm_ctrl_type, pm_enable, ret);

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_probe_resp_mode
 * 功能描述  : 同步hmac 的probe resp mode能力
 * 1.日    期  : 2019年1月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_probe_resp_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_probe_resp_mode_enum *pen_probe_resp_status = NULL;

    if (!is_p2p_dev(pst_mac_vap)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_probe_resp_mode:: [vap_mode:%d] \
            [p2p_mode:%d] is not P2P_dev. return INVALID_CONFIG.}", pst_mac_vap->en_vap_mode,pst_mac_vap->en_p2p_mode);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pen_probe_resp_status = (mac_probe_resp_mode_enum *)puc_param;

    /* probe_resp_flag未开启，不允许改动probe_resp_mode的默认值 */
    if (pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_enable == OAL_FALSE) {
        oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                      "{hmac_config_set_probe_resp_mode::Host set probe_resp_status[%d],buf Drv_enable == FALSE.}",
                      *pen_probe_resp_status);
        return OAL_SUCC;
    }

    if (*pen_probe_resp_status >= MAC_PROBE_RESP_MODE_BUTT) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_TX,
                       "{hmac_config_set_probe_resp_mode::config probe_resp_status error %d.}", *pen_probe_resp_status);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_probe_resp_mode::Host set probe_resp_status from [%d] to [%d].}",
                     pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_status, *pen_probe_resp_status);
    pst_mac_vap->st_probe_resp_ctrl.en_probe_resp_status = *pen_probe_resp_status;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROBE_PESP_MODE,
                                 sizeof(mac_probe_resp_mode_enum), puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_probe_resp_mode::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


/*
 * 函 数 名  : hmac_config_ch_status_sync
 * 功能描述  : 同步信道信息
 * 1.日    期  : 2016年3月26日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_ch_status_sync(mac_device_stru *pst_mac_dev)
{
    uint32_t ret;
    mac_ap_ch_info_stru ast_ap_channel_list[MAC_MAX_SUPP_CHANNEL] = { { 0 } };
    mac_vap_stru *pst_mac_vap = NULL;

    if (pst_mac_dev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_ch_status_sync::pst_mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = mac_res_get_mac_vap(pst_mac_dev->auc_vap_id[0]);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_ch_status_sync::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memcpy_s((uint8_t *)ast_ap_channel_list, sizeof(ast_ap_channel_list),
             (uint8_t *)(pst_mac_dev->st_ap_channel_list), sizeof(pst_mac_dev->st_ap_channel_list));

    /***************************************************************************
        抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SYNC_CH_STATUS,
                                 sizeof(ast_ap_channel_list), (uint8_t *)ast_ap_channel_list);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_ch_status_sync::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}


/*
 * 函 数 名  : hmac_config_get_version
 * 功能描述  : 获取版本
 * 1.日    期  : 2014年3月28日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_version(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_VERSION, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_version::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_extlna_bypass_rssi(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_EXTLNA_BYPASS_RSSI, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_extlna_bypass_rssi::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

OAL_STATIC uint32_t hmac_config_get_cali_data(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    cali_data_req_stru *cali_data_req_info = (cali_data_req_stru *)puc_param;

    /* device从host 获取CSA 校准数据 */
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CSA,
        "{hmac_config_get_cali_data::get cali data info: channel[%d] bw[%d], cali_data_sub_type %d}",
        cali_data_req_info->channel.uc_chan_number,
        cali_data_req_info->channel.en_bandwidth,
        cali_data_req_info->work_cali_data_sub_type);

    wlan_chip_update_cur_chn_cali_data(cali_data_req_info);
    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*
 * 函 数 名  : hmac_config_set_all_log_level
 * 功能描述  : 输入参数
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_all_log_level(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t vap_idx;
    uint8_t level;

    if (oal_any_null_ptr2(pst_mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_all_log_level::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    level = (uint8_t)(*puc_param);

    for (vap_idx = 0; vap_idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_idx++) {
        oam_log_set_vap_level(vap_idx, level);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_ALL_LOG_LEVEL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_all_log_level::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_fem_cfg_param
 * 功能描述  : 输入参数
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_cus_rf(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_RF, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_rf::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_set_cus_dts_cali
 * 功能描述  : 输入参数
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_cus_dts_cali(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_DTS_CALI, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_dts_cali::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_set_cus_nvram_params
 * 功能描述  : 输入参数
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_cus_nvram_params(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_NVRAM_PARAM, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_nvram_params::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_cus_dyn_cali
 * 功能描述  : 输入参数
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_cus_dyn_cali(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_CUS_DYN_CALI_PARAM, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_cus_dyn_cali::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

/*
 * 函 数 名  : hmac_config_dyn_cali_param
 * 功能描述  : hmac层动态校准参数配置函数
 * 1.日    期  : 2016年09月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_dyn_cali_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DYN_CALI_CFG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_dyn_cali_param::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_send_custom_data(mac_vap_stru *pst_mac_vap, uint16_t len, uint8_t *param, uint8_t syn_id)
{
    oal_netbuf_stru *netbuf = NULL;
    uint32_t ret;
    frw_event_mem_stru       *event_mem = NULL;
    frw_event_stru           *event = NULL;
    dmac_tx_event_stru       *h2d_custom_event = NULL;
    int32_t sec_ret;

    /*  申请netbuf */
    netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_send_custom_data::netbuf alloc failed");
        return OAL_FAIL;
    }
    /* 填充netbuf sys id(1字节) + 定制化数据 */
    sec_ret = memcpy_s(oal_netbuf_data(netbuf), WLAN_LARGE_NETBUF_SIZE, &syn_id, sizeof(syn_id));
    sec_ret += memcpy_s(oal_netbuf_data(netbuf) + sizeof(syn_id),
        (WLAN_LARGE_NETBUF_SIZE - sizeof(syn_id)), param, len);
    if (sec_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_send_custom_data::memcpy fail!");
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }
    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_send_custom_data::pst_event_mem null.}");
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_CUST_HMAC2DMAC,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id,
        pst_mac_vap->uc_vap_id);
    h2d_custom_event               = (dmac_tx_event_stru *)event->auc_event_data;
    h2d_custom_event->pst_netbuf   = netbuf;
    h2d_custom_event->us_frame_len = len + sizeof(syn_id);

    ret = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_send_cali_data::dispatch event fail ret[%d], len[%d] syn_id[%d].}", ret, len, syn_id);
        oal_netbuf_free(netbuf);
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    oal_netbuf_free(netbuf);
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}
#ifdef HISI_CONN_NVE_SUPPORT
uint32_t hmac_config_report_xo_diff(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap;
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_report_xo_diff::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (memcpy_s(g_xo_pwr_diff, sizeof(g_xo_pwr_diff), param, len) != EOK) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_report_xo_diff::memcpy fail.}");
        return OAL_FAIL;
    }

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    hmac_vap->uc_efuse_cali_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(hmac_vap->query_wait_q));

    return OAL_SUCC;
}
#ifdef PLATFORM_DEBUG_ENABLE
uint32_t hmac_config_report_write_pow_diff(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap;
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_report_xo_diff::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_write_pow_flag = *(int32_t *)(param);

    if (g_write_pow_flag == 0) {
        g_write_pow_flag = OAL_SUCC;
    } else {
        g_write_pow_flag = -OAL_EINVAL;
    }

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    hmac_vap->uc_write_pow_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(hmac_vap->query_wait_q));

    return OAL_SUCC;
}

uint32_t hmac_config_report_write_xo_diff(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap;
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_report_xo_diff::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_write_xo_flag = *(int32_t *)(param);

    if (g_write_xo_flag == 0) {
        g_write_xo_flag = OAL_SUCC;
    } else {
        g_write_xo_flag = -OAL_EINVAL;
    }

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    hmac_vap->uc_write_xo_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(hmac_vap->query_wait_q));

    return OAL_SUCC;
}
#endif
#endif

/*
 * 功能描述  : disassoc/deauth发送完成，删除用户
 */
uint32_t hmac_tx_complete_deauth_disassoc(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    uint16_t user_idx;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;

    if (oal_any_null_ptr2(mac_vap, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取hmac_user 若user已被删除则无需处理 */
    user_idx = *(uint16_t *)param;
    hmac_user = mac_res_get_hmac_user(user_idx);
    if (hmac_user == NULL) {
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_UM,
            "{hmac_tx_complete_deauth_disassoc:hmac_vap is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (is_legacy_ap(mac_vap) == OAL_TRUE || mac_is_chba_mode(mac_vap) == OAL_TRUE) {
        /*  kick_user_timer定时器启动，即这是上层发起的去关联。需在发送完成时再进行del user处理 */
        if (hmac_user->kick_user_timer.en_is_registerd == OAL_TRUE) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_UM,
                "{hmac_tx_complete_deauth_disassoc:user[%d] tx disassoc complete!}", user_idx);

            /* 发送完成后,先停止kick user定时器 */
            frw_timer_immediate_destroy_timer_m(&(hmac_user->kick_user_timer));

            /* 抛事件上报内核，已经去关联某个user */
            hmac_handle_disconnect_rsp(hmac_vap, hmac_user, hmac_user->assoc_err_code);

            /* 删除用户 */
            hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
        }
    } else if (is_sta(mac_vap)) {
        /* STA 发送完成，上报wal 继续执行kick_user */
        hmac_vap->disassoc_tx_completed_flag = OAL_TRUE;
        oal_wait_queue_wake_up(&(hmac_vap->query_wait_q));
    }
    return  OAL_SUCC;
}

/*****************************************************************************
    g_ast_hmac_config_syn: dmac向hmac同步控制面数据处理函数表
*****************************************************************************/
OAL_STATIC const hmac_config_syn_stru g_ast_hmac_config_syn[] = {
    /* 同步ID                    保留2个字节            函数操作 */
    { WLAN_CFGID_QUERY_STATION_STATS, { 0, 0 }, hmac_proc_query_response_event },
    { WLAN_CFGID_QUERY_STATION_STATS_EXT, { 0, 0 }, hmac_proc_query_additional_response_event },
    { WLAN_CFGID_RESET_HW_OPERATE, { 0, 0 }, hmac_reset_sys_event },
    { WLAN_CFGID_BTCOEX_RX_DELBA_TRIGGER, { 0, 0 }, hmac_btcoex_rx_delba_trigger },
    { WLAN_CFGID_QUERY_RSSI, { 0, 0 }, hmac_config_query_rssi_rsp },
    { WLAN_CFGID_QUERY_PSST, { 0, 0 }, hmac_config_query_psst_rsp },
    {WLAN_CFGID_QUERY_PSM_STAT,         {0, 0}, hmac_config_query_psm_rsp},
#ifdef _PRE_WLAN_DELAY_STATISTIC
    { WLAN_CFGID_NOTIFY_STA_DELAY, { 0, 0 }, hmac_config_receive_sta_delay },
#endif
    { WLAN_CFGID_QUERY_RATE, { 0, 0 }, hmac_config_query_rate_rsp },
#ifdef _PRE_WLAN_DFT_STAT
    { WLAN_CFGID_QUERY_ANI, { 0, 0 }, hmac_config_query_ani_rsp },
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
    { WLAN_CFGID_PWL_SYNC_PN, { 0, 0 }, hmac_pwl_sync_pn_event },
#endif
#ifdef PLATFORM_DEBUG_ENABLE
    { WLAN_CFGID_GET_VERSION,  { 0, 0 }, hmac_atcmdsrv_dbb_num_response },
    { WLAN_CFGID_GET_ANT,      { 0, 0 }, hmac_atcmdsrv_get_ant_response },
    { WLAN_CFGID_CHECK_LTE_GPIO, { 0, 0 }, hmac_atcmdsrv_lte_gpio_check },
#endif
    { WLAN_CFGID_RX_FCS_INFO, { 0, 0 }, hmac_config_get_rx_fcs_info },
    { WLAN_CFGID_TX_PDET_VAL, { 0, 0 }, hmac_config_get_tx_pdet_info },
#if ((defined(_PRE_WLAN_EXPORT)) && (defined(_PRE_WLAN_NARROW_BAND)))
    { WLAN_CFGID_GET_SW_VERSION, { 0, 0 }, hmac_config_get_sw_version_info },
    { WLAN_CFGID_NB_INFO_REPORT, { 0, 0 }, hmac_config_nb_info_report },
#endif
    { WLAN_CFGID_GET_DIEID, { 0, 0 }, hmac_config_get_dieid_rsp },
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef PLATFORM_DEBUG_ENABLE
    { WLAN_CFGID_REG_INFO, { 0, 0 }, hmac_atcmdsrv_report_efuse_reg },
#endif
#endif
    { WLAN_CFGID_CFG80211_MGMT_TX_STATUS, { 0, 0 }, hmac_mgmt_tx_event_status },
    { WLAN_CFGID_USR_INFO_SYN,            { 0, 0 }, hmac_config_d2h_user_info_syn },
    { WLAN_CFGID_VAP_MIB_UPDATE,          { 0, 0 }, hmac_config_d2h_vap_mib_update },
    { WLAN_CFGID_VAP_CAP_UPDATE,          { 0, 0 }, hmac_config_d2h_vap_cap_update },
    { WLAN_CFGID_VAP_CHANNEL_INFO_SYN,    { 0, 0 }, hmac_config_d2h_vap_channel_info },
    { WLAN_CFGID_GET_MNGPKT_SENDSTAT,     { 0, 0 }, hmac_config_query_sta_mngpkt_sendstat_rsp },

#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) || defined(_PRE_WLAN_FEATURE_SMPS)
    { WLAN_CFGID_USER_M2S_INFO_SYN, { 0, 0 }, hmac_config_user_m2s_info_syn },
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
    { WLAN_CFGID_DEVICE_M2S_INFO_SYN, { 0, 0 }, hmac_config_d2h_device_m2s_info_syn },
    { WLAN_CFGID_VAP_M2S_INFO_SYN,    { 0, 0 }, hmac_config_d2h_vap_m2s_info_syn },
    { WLAN_CFGID_M2S_SWITCH_COMP,     { 0, 0 }, hmac_m2s_switch_protect_comp_event_status },
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
    { WLAN_CFGID_REPORT_STATE,     { 0, 0 }, hmac_m2s_lp_miracast_report_state },
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    { WLAN_CFGID_ANT_TAS_SWITCH_RSSI_NOTIFY, { 0, 0 }, hmac_ant_tas_switch_rssi_notify_event_status },
#endif
    { WLAN_CFGID_AMSDU_SWITCH_REPORT, { 0, 0 }, hmac_d2h_amsdu_switch_report },
#ifdef _PRE_WLAN_FEATURE_11K
    { WLAN_CFGID_REQ_SAVE_BSS_INFO, { 0, 0 }, hmac_scan_rrm_proc_save_bss },
#endif

    { WLAN_CFGID_VOWIFI_REPORT, { 0, 0 }, hmac_config_vowifi_report },

    { HAL_TX_COMP_SUB_TYPE_AL_TX, { 0, 0 }, hmac_config_stop_altx },

    { WLAN_CFGID_GET_DBDC_INFO, { 0, 0 }, hmac_config_get_dbdc_info },

#ifdef _PRE_WLAN_FEATURE_M2S
    { WLAN_CFGID_MIMO_COMPATIBILITY, { 0, 0 }, hmac_config_mimo_compatibility },
#endif

#ifdef _PRE_WLAN_FEATURE_NRCOEX
    { WLAN_CFGID_QUERY_NRCOEX_STAT, { 0, 0 }, hmac_config_report_nrcoex_stat },
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_FEATURE_DDR_BUGFIX
    { WLAN_CFGID_SET_DDR, { 0, 0 }, hmac_set_ddr },
#endif
#endif
    { WLAN_CFGID_HAL_VAP_UPDATE, { 0, 0 }, hmac_config_d2h_hal_vap_syn},
    { WLAN_CFGID_LUT_USR_UPDATE, { 0, 0 }, hmac_config_d2h_usr_lut_syn},
    { WLAN_CFGID_RX_MODE_SWITCH, { 0, 0 }, hmac_config_d2h_rx_mode_syn},
    { WLAN_CFGID_PCIE_UP_PREPARE_SUCC, { 0, 0 }, hmac_pcie_up_prepare_succ_handle },

#ifdef _PRE_WLAN_FEATURE_11AX
    { WLAN_UL_OFDMA_AMSDU_SYN, { 0, 0 }, _hmac_config_ul_ofdma_amsdu },
    { WLAN_CFGID_SET_SEND_TB_PPDU_FLAG, { 0, 0 }, _hmac_config_set_send_tb_ppdu_flag },
#endif
    { WLAN_CFGID_TRIGGER_REASSOC, { 0, 0 }, hmac_config_d2h_trigger_reassoc },
    { WLAN_CFGID_PS_RX_DELBA_TRIGGER, { 0, 0 }, hmac_ps_rx_delba },
    { WLAN_CFGID_PS_ARP_PROBE, { 0, 0 }, hmac_ps_rx_amsdu_arp_probe_process },
#ifdef _PRE_WLAN_FEATURE_NAN
    { WLAN_CFGID_NAN, { 0, 0 }, hmac_config_nan_response },
#endif

#ifdef _PRE_WLAN_FEATURE_HID2D
    { WLAN_CFGID_HID2D_ACS_REPORT, { 0, 0 }, hmac_hid2d_cur_chan_stat_handler },
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
    { WLAN_CFGID_D2H_MAX_TX_POWER, { 0, 0 }, hmac_save_max_tx_power_info },
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    { WLAN_CFGID_D2H_WMMAC_PARAM, { 0, 0 }, hmac_config_d2h_wmmac_update_info },
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    { WLAN_CFGID_D2H_FTM_REPORT_CALI, { 0, 0 }, hmac_event_ftm_cali},
    { WLAN_CFGID_RTT_RESULT_REPORT, { 0, 0 }, hmac_config_rtt_result_report},
    { WLAN_CFGID_D2H_FTM_ISR, { 0, 0 }, hmac_handle_ftm_isr},
#endif
    { WLAN_CFGID_SYNC_BT_STATE, { 0, 0 }, hmac_btcoex_d2h_bt_state_syn },
#ifdef _PRE_WLAN_FEATURE_DFS
    { WLAN_CFGID_ZERO_WAIT_DFS_REPORT, { 0, 0 }, hmac_dfs_zero_wait_report_event },
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    { WLAN_CFGID_SET_CSI_BLACK_LIST, { 0, 0 }, hmac_config_csi_set_black_list_rsp },
#endif
#ifdef _PRE_WLAN_FEATURE_GNSS_RSMC
    { WLAN_CFGID_GNSS_RSMC_STATUS_SYN, { 0, 0 }, hmac_process_gnss_rsmc_status_sync },
#endif
    { WLAN_CFGID_GET_CALI_DATA, { 0, 0 }, hmac_config_get_cali_data},
    { WLAN_CFGID_QUERY_TSF, { 0, 0 }, hmac_config_tsf_event_process },
    { WLAN_CFGID_TX_DEAUTH_DISASSOC_COMPLETE, { 0, 0 }, hmac_tx_complete_deauth_disassoc },

#ifdef _PRE_WLAN_CHBA_MGMT
    { WLAN_CFGID_CHBA_SYNC_EVENT, { 0, 0 }, hmac_chba_d2h_sync_event },
    { WLAN_CFGID_CHBA_NOTIFY_MULTI_DOMAIN, { 0, 0 }, hmac_chba_sync_multiple_domain_info },
    { WLAN_CFGID_CHBA_NOTIFY_NON_ALIVE_DEVICE, { 0, 0 }, hmac_chba_del_non_alive_device },
    { WLAN_CFGID_CHBA_SET_USER_BITMAP_LEVEL, { 0, 0 }, hmac_chba_update_user_bitmap_level },
    { WLAN_CFGID_CHBA_SET_VAP_BITMAP_LEVEL, { 0, 0 }, hmac_chba_update_vap_bitmap_level },
    { WLAN_CFGID_CHBA_CSA_LOST_DEVICE_NOTIFY, { 0, 0 }, hmac_chba_csa_lost_device_proc },
    { WLAN_CFGID_CHBA_SET_COEX_CHAN_INFO, { 0, 0 }, hmac_chba_coex_info_changed_report },
    { WLAN_CFGID_CHBA_UPDATE_TOPO_BITMAP_INFO, { 0, 0 }, hmac_chba_update_topo_bitmap_info_proc },
    { WLAN_CFGID_CHBA_DBAC_UPDATE_CHAN_SEQ_CONFIG, { 0, 0 }, hmac_chba_dbac_update_fcs_config },
    { WLAN_CFGID_CHBA_DOMAIN_MERGE_EVENT, { 0, 0 }, hmac_chba_domain_merge_send_notify_immediately },
#endif
    { WLAN_CFGID_QUERY_TID_QUEUE, { 0, 0 }, hmac_config_query_tid_queue_rsp },
    { WLAN_CFGID_STA_CSA_STATUS_NOTIFY, { 0, 0 }, hmac_config_d2h_sta_csa_status_notify },
    { WLAN_CFGID_UP_VMIN, { 0, 0 }, hmac_config_d2h_up_vmin },
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef PLATFORM_DEBUG_ENABLE
    { WLAN_CFGID_GET_TEMPERATURE, { 0, 0 }, hmac_config_report_temperature },
    { WLAN_CFGID_GET_HW_WORD, { 0, 0 }, hmac_config_report_hw_word },
    { WLAN_CFGID_GET_BOARD_SN, { 0, 0 }, hmac_config_report_board_sn },
    { WLAN_CFGID_SET_BOARD_SN, { 0, 0 }, hmac_config_set_board_sn },
    { WLAN_CFGID_FLASH_TEST, { 0, 0 }, hmac_config_report_flash_test },
    { WLAN_CFGID_SPMI_TEST, { 0, 0 }, hmac_config_report_spmi_test },
#endif
#endif
#ifdef HISI_CONN_NVE_SUPPORT
    { WLAN_CFGID_GET_XO_DIFF, { 0, 0 }, hmac_config_report_xo_diff },
#ifdef PLATFORM_DEBUG_ENABLE
    { WLAN_CFGID_TX_POW_H2D, { 0, 0 }, hmac_config_report_write_pow_diff },
    { WLAN_CFGID_XO_DIFF_H2D, { 0, 0 }, hmac_config_report_write_xo_diff },
#endif
#endif
#ifdef _PRE_WLAN_EXPORT
    { WLAN_CFGID_MAC_ADDR_READ, { 0, 0 }, hmac_config_report_mac_addr },
#endif
    { WLAN_CFGID_BUTT, { 0, 0 }, NULL },
};

/*
 * 函 数 名  : hmac_event_config_syn
 * 功能描述  : hmac配置同步事件处理入口
 * 1.日    期  : 2013年4月28日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_event_config_syn(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    frw_event_hdr_stru *pst_event_hdr = NULL;
    dmac_to_hmac_cfg_msg_stru *pst_dmac2hmac_msg = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    uint32_t ret;
    uint16_t us_cfgid;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_event_config_syn::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_dmac2hmac_msg = (dmac_to_hmac_cfg_msg_stru *)pst_event->auc_event_data;

    oam_info_log1(pst_event_hdr->uc_vap_id, OAM_SF_CFG,
                  "{hmac_event_config_syn::a dmac config syn event occur, cfg_id=%d.}", pst_dmac2hmac_msg->en_syn_id);

    /* 获取dmac vap */
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_event_hdr->uc_vap_id);
    if (pst_mac_vap == NULL) {
        oam_error_log0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::pst_mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device */
    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得cfg id对应的操作函数 */
    for (us_cfgid = 0; WLAN_CFGID_BUTT != g_ast_hmac_config_syn[us_cfgid].en_cfgid; us_cfgid++) {
        if (g_ast_hmac_config_syn[us_cfgid].en_cfgid == pst_dmac2hmac_msg->en_syn_id) {
            break;
        }
    }

    /* 异常情况，cfgid在g_ast_dmac_config_syn中不存在 */
    if (g_ast_hmac_config_syn[us_cfgid].en_cfgid == WLAN_CFGID_BUTT) {
        oam_warning_log1(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{hmac_event_config_syn::invalid en_cfgid[%d].",
                         pst_dmac2hmac_msg->en_syn_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 执行操作函数 */
    ret = g_ast_hmac_config_syn[us_cfgid].p_set_func(pst_mac_vap, (uint8_t)(pst_dmac2hmac_msg->us_len),
        (uint8_t *)pst_dmac2hmac_msg->auc_msg_body);
    if (ret != OAL_SUCC) {
        oam_warning_log2(pst_event_hdr->uc_vap_id, OAM_SF_CFG,
                         "{hmac_event_config_syn::p_set_func failed, ret=%d en_syn_id=%d.",
                         ret, pst_dmac2hmac_msg->en_syn_id);
        return ret;
    }

    return OAL_SUCC;
}
