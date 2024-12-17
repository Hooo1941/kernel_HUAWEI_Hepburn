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
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_TLV_C
hmac_tx_pkts_stat_stru g_host_tx_pkts = { 0 };
/*
 * 函 数 名  : hmac_set_device_freq_mode
 * 功能描述  : 设置device调频使能
 * 1.日    期  : 2016年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_set_device_pkt_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_tx_pkts_stat_stru *pst_pkts_stat;
    hmac_vap_stru *hmac_vap;
    uint32_t durance;
    uint32_t snd_mbits;
    mac_cfg_set_tlv_stru *pst_config_para;

    pst_pkts_stat = &g_host_tx_pkts;
    pst_config_para = (mac_cfg_set_tlv_stru *)puc_param;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{hmac_config_set_device_pkt_stat::pst_hmac_vap null.}");
        return;
    }
    if (pst_config_para->uc_cmd_type == PKT_STAT_SET_START_STAT) {
        if (pst_config_para->value == OAL_TRUE) {
            oal_spin_lock_bh(&hmac_vap->st_lock_state);
            pst_pkts_stat->snd_pkts = 0;
            oal_spin_unlock_bh(&hmac_vap->st_lock_state);
            pst_pkts_stat->start_time = (uint32_t)oal_time_get_stamp_ms();
        } else {
            durance = (uint32_t)oal_time_get_stamp_ms();
            durance -= pst_pkts_stat->start_time;
            if (durance == 0) {
                oam_error_log2(0, OAM_SF_CFG, "{hmac_config_set_device_pkt_stat::START TIME[%d],NOW TINE[%d].}",
                               pst_pkts_stat->start_time, oal_time_get_stamp_ms());
                return;
            }
            snd_mbits = ((pst_pkts_stat->snd_pkts * pst_pkts_stat->pkt_len) / durance) >> NUM_7_BITS;

            oam_error_log4(0, OAM_SF_CFG,
                           "{hmac_config_set_device_pkt_stat::snd rate[%d]Mbits,snd pkts[%d],pktlen[%d],time[%d].}",
                           snd_mbits, pst_pkts_stat->snd_pkts, pst_pkts_stat->pkt_len, durance);
        }
    } else if (pst_config_para->uc_cmd_type == PKT_STAT_SET_FRAME_LEN) {
        pst_pkts_stat->pkt_len = pst_config_para->value;
    }
}

uint32_t hmac_config_set_tx_ampdu_type(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_set_device_freq_mode
 * 功能描述  : 设置device调频使能
 * 1.日    期  : 2016年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_set_device_freq_mode(uint8_t uc_device_enable)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap = NULL;
    uint8_t idx;

    /* 设置Device 调频使能控制 */
    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap == NULL) {
        return;
    }

    for (idx = 0; idx < 4; idx++) { /* 共计4个device data */
        st_device_freq_type.st_device_data[idx].speed_level = g_host_speed_freq_level[idx].speed_level;
        st_device_freq_type.st_device_data[idx].cpu_freq_level =
            g_device_speed_freq_level[idx].uc_device_type;
    }

    st_device_freq_type.uc_device_freq_enable = uc_device_enable;
    st_device_freq_type.uc_set_type = FREQ_SET_MODE;

    oam_warning_log1(0, OAM_SF_CFG, "{hmac_set_device_freq_mode: enable mode[%d][1:enable,0:disable].}",
                     st_device_freq_type.uc_device_freq_enable);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                 sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_set_device_freq_mode::hmac_set_device_freq failed[%d].}", ret);
    }
}

/*
 * 函 数 名  : hmac_config_set_device_freq
 * 功能描述  : 设置device调频level
 * 1.日    期  : 2016年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_device_freq(uint8_t uc_device_freq_type)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_device_freq_type.uc_set_type = FREQ_SET_FREQ;
    st_device_freq_type.uc_set_freq = uc_device_freq_type;

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_device_freq: set mode[%d],device freq level[%d].}",
                     st_device_freq_type.uc_set_type, uc_device_freq_type);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                 sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_device_freq::hmac_set_device_freq failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_device_freq_log(uint8_t log_val)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_device_freq_type.uc_set_type = FREQ_SET_FREQ_LOG;
    st_device_freq_type.uc_set_freq = log_val;

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_device_freq_log: set mode[%d],device freq level[%d].}",
                     st_device_freq_type.uc_set_type, st_device_freq_type.uc_set_freq);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                 sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_device_freq::hmac_set_device_freq failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_device_freq_testcase
 * 功能描述  : 设置device调频level
 * 1.日    期  : 2017年10月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
#define FREQ_TC_EN   1
#define FREQ_TC_EXIT 0
uint32_t hmac_config_set_device_freq_testcase(uint8_t uc_device_freq_type)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_device_freq_type.uc_set_freq = uc_device_freq_type;
    /* 调频类型 */
    if (uc_device_freq_type == FREQ_TC_EN) {
        st_device_freq_type.uc_set_type = FREQ_SET_FREQ_TC_EN;
    } else if (uc_device_freq_type == FREQ_TC_EXIT) {
        st_device_freq_type.uc_set_type = FREQ_SET_FREQ_TC_EXIT;
    } else {
        st_device_freq_type.uc_set_type = FREQ_SET_BUTT;
    }

    oam_warning_log2(0, OAM_SF_CFG,
                     "{hmac_config_set_device_freq_testcase: set mode[%d],device freq testcase enable[%d].}",
                     st_device_freq_type.uc_set_type, uc_device_freq_type);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                 sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_device_freq_testcase:: failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_get_device_freq
 * 功能描述  : 获取device调频level参数
 * 1.日    期  : 2016年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_device_freq(void)
{
    uint32_t ret;
    config_device_freq_h2d_stru st_device_freq_type;
    mac_vap_stru *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(0);
    if (pst_mac_vap != NULL) {
        st_device_freq_type.uc_set_type = FREQ_GET_FREQ;

        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_get_device_freq!].}");

        /***************************************************************************
            抛事件到DMAC层, 同步VAP最新状态到DMAC
        ***************************************************************************/
        ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DEVICE_FREQ,
                                     sizeof(config_device_freq_h2d_stru), (uint8_t *)(&st_device_freq_type));
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_device_freq::hmac_set_device_freq failed[%d].}", ret);
        }
    } else {
        ret = OAL_ERR_CODE_PTR_NULL;
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_auto_freq_enable
 * 功能描述  : 设置自动调频使能
 * 1.日    期  : 2015年9月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_auto_freq_enable(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t uc_value;
    mac_cfg_set_tlv_stru *pst_set_auto_freq = (mac_cfg_set_tlv_stru *)puc_param;

    uc_value = (uint8_t)pst_set_auto_freq->value;

    if (pst_set_auto_freq->uc_cmd_type == CMD_SET_AUTO_FREQ_ENDABLE) {
        // 设置device调频使能
        hmac_set_device_freq_mode(uc_value);
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_SET_DEVICE_FREQ_VALUE) {
        /* 单独设置DEVICE CPU频率 */
        hmac_config_set_device_freq(uc_value);
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_SET_AUTO_FREQ_LOG) {
        hmac_config_set_device_freq_log(uc_value);
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_GET_DEVICE_AUTO_FREQ) {
        hmac_config_get_device_freq();
    } else if (pst_set_auto_freq->uc_cmd_type == CMD_SET_DEVICE_FREQ_TC) {
        hmac_config_set_device_freq_testcase(uc_value);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_set_device_freq:parameter error!}");
    }
    oam_warning_log2(0, OAM_SF_ANY,
                     "{hmac_config_set_auto_freq_enable:set_auto_freq_enable:uc_cmd_type = %d, uc_value = %d}",
                     pst_set_auto_freq->uc_cmd_type, uc_value);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
/*
 * 函 数 名  : hmac_config_set_mcast_ampdu_retry
 * 功能描述  : 组播聚合模式下配置组播帧重传次数
 * 1.日    期  : 2020年1月18日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
void hmac_config_set_mcast_ampdu_retry(mac_vap_stru *mac_vap, uint32_t cmd)
{
    uint32_t ret;

    if (mac_get_mcast_ampdu_switch() != OAL_TRUE) {
        return;
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MCAST_AMPDU_RETRY, sizeof(uint32_t), (uint8_t *)&cmd);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_TX,
                         "{hmac_config_set_mcast_ampdu_retry:hmac_config_send_event fail[%d]", ret);
    }
}

/*
 * 函 数 名  : hmac_config_mcast_ampdu_pack_switch
 * 功能描述  : CHBA可靠组播,组播聚合PACK回馈功能的开关
 * 1.日    期  : 2021年3月20日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
void hmac_config_mcast_ampdu_pack_switch(mac_vap_stru *mac_vap, uint32_t cmd)
{
    uint32_t ret;

    if (mac_get_mcast_ampdu_switch() != OAL_TRUE) {
        return;
    }
    hmac_mcast_ampdu_set_pack_switch(mac_vap, (uint8_t)cmd);
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MCAST_AMPDU_PACK_SWITCH, sizeof(uint32_t), (uint8_t *)&cmd);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_TX,
                         "{hmac_config_mcast_ampdu_pack_switch:hmac_config_send_event fail[%d]", ret);
    }
}
#endif

OAL_STATIC void hmac_config_common_debug_part1(mac_vap_stru *pst_mac_vap, uint16_t us_set_id, uint32_t value)
{
#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_device_stru *pst_mac_device = NULL;
#endif

    if (us_set_id == RIFS_ENABLE) {
        mac_mib_set_RifsMode(pst_mac_vap, (oal_bool_enum_uint8)value);
    } else if (us_set_id == GREENFIELD_ENABLE) {
        mac_mib_set_HTGreenfieldOptionImplemented(pst_mac_vap, (oal_bool_enum_uint8)value);
    } else if (us_set_id == AUTH_RSP_TIMEOUT) {
        g_st_mac_device_custom_cfg.us_cmd_auth_rsp_timeout = (uint16_t)value;
    } else if (us_set_id == FORBIT_OPEN_AUTH) {
        g_st_mac_device_custom_cfg.bit_forbit_open_auth = (value == 0 ? OAL_FALSE : OAL_TRUE);
    } else if (us_set_id == HT_SELF_CURE_DEBUG) {
        hmac_ht_self_cure_need_check_flag_set((uint8_t)value);
    } else if (us_set_id == USERCTL_BINDCPU) {
        hmac_userctl_bindcpu_get_cmd(value);
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    } else if (us_set_id == MCAST_AMPDU_RETRY) {
        hmac_config_set_mcast_ampdu_retry(pst_mac_vap, value);
    } else if (us_set_id == MCAST_AMPDU_PACK_SWITCH) {
        hmac_config_mcast_ampdu_pack_switch(pst_mac_vap, value);
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    } else if (us_set_id == SU_PPDU_1XLTF_08US_GI_SWITCH) {
        mac_mib_set_HESUPPDUwith1xHELTFand0point8GIlmplemented(pst_mac_vap, !!value);
#endif
    }
#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
        if (oal_unlikely((oal_any_null_ptr1(pst_mac_device)))) {
            oam_warning_log0(0, OAM_SF_HIEX, "{hmac_config_common_debug_part1::hiex pst_mac_device is null.}");
            return;
        }
        if (us_set_id == HIEX_DEV_CAP) {
            mac_hiex_nego_cap(MAC_BAND_GET_HIEX_CAP(pst_mac_device), (mac_hiex_cap_stru *)(&value), NULL);
        }
        if (us_set_id == HIEX_DEBUG) {
            pst_mac_device->hiex_debug_switch = value;
        }
    }
#endif
}

/*
 * 函 数 名  : hmac_config_common_debug
 * 功能描述  : common debug命令host处理函数
 * 1.日    期  : 2019年6月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_config_common_debug(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t uc_cmd_cnt;
    uint8_t uc_cmd_index = 0;
    uint16_t us_set_id;
    uint32_t value;
    mac_cfg_set_str_stru *pst_set_cmd = NULL;

    pst_set_cmd = (mac_cfg_set_str_stru *)puc_param;
    uc_cmd_cnt = pst_set_cmd->uc_cmd_cnt;
    if (uc_cmd_cnt > MAC_STR_CMD_MAX_SIZE) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_common_debug::cnt err[%d].}", uc_cmd_cnt);
        return OAL_ERR_CODE_PTR_NULL;
    }

    while (uc_cmd_cnt > 0) {
        us_set_id = pst_set_cmd->us_set_id[uc_cmd_index];
        value = pst_set_cmd->value[uc_cmd_index];

#ifdef _PRE_WLAN_FEATURE_MBO
        if (us_set_id == MBO_SWITCH) {
            pst_mac_vap->st_mbo_para_info.uc_mbo_enable = (uint8_t)value;
        }
        if (us_set_id == MBO_CELL_CAP) {
            pst_mac_vap->st_mbo_para_info.uc_mbo_cell_capa = (uint8_t)value;
        }
        if (us_set_id == MBO_ASSOC_DISALLOWED_SWITCH) {
            pst_mac_vap->st_mbo_para_info.uc_mbo_assoc_disallowed_test_switch = (uint8_t)value;
        }
#endif
        hmac_config_common_debug_part1(pst_mac_vap, us_set_id, value);
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_common_debug:set id[%d] set data[%d].}",
                         us_set_id, value);
        uc_cmd_index++;
        uc_cmd_cnt--;
    }

    return OAL_SUCC;
}

OAL_STATIC void hmac_config_log_debug(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t uc_cmd_cnt;
    uint8_t uc_cmd_index = 0;
    uint16_t us_set_id;
    uint32_t value;
    mac_cfg_set_str_stru *pst_set_cmd = NULL;
    mac_device_stru *pst_mac_device = NULL;

    pst_set_cmd = (mac_cfg_set_str_stru *)puc_param;
    uc_cmd_cnt = pst_set_cmd->uc_cmd_cnt;
    if (uc_cmd_cnt > MAC_STR_CMD_MAX_SIZE) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_log_debug::cnt err[%d].}", uc_cmd_cnt);
        return;
    }

    while (uc_cmd_cnt > 0) {
        us_set_id = pst_set_cmd->us_set_id[uc_cmd_index];
        value = pst_set_cmd->value[uc_cmd_index];
        if (us_set_id == MAC_LOG_MONITOR_OTA_RPT) {
            pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
            if (oal_unlikely(pst_mac_device == NULL)) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_log_debug::mac_device null.}");
                return;
            }
            pst_mac_device->uc_monitor_ota_mode = (uint8_t)value;
        } else if (us_set_id == MAC_LOG_SET_HIMIT_HTC) {
#ifdef _PRE_WLAN_FEATURE_HIEX
            pst_mac_vap->uc_himit_set_htc = (uint8_t)value;
#endif
#ifdef _PRE_DELAY_DEBUG
        } else if (us_set_id == MAC_LOG_DELAY_RPT) {
            hmac_register_hcc_callback(value);
        } else if (us_set_id == MAC_LOG_TRX_LOG) {
            g_wifi_delay_log = value;
            g_wifi_trx_log = value;
#endif
        }
        uc_cmd_index++;
        uc_cmd_cnt--;
    }
}

#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
/*
 * 函 数 名  : hmac_config_set_hi2d2_presentation_mode
 * 功能描述  : 设置HiD2D发布会演示模式
 * 1.日    期  : 2020年4月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_hid2d_presentation_mode(mac_vap_stru *pst_mac_vap,
    uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_config_para = (mac_cfg_set_tlv_stru *)puc_param;
    mac_device_stru *pst_device = NULL;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if ((pst_device != NULL) && (pst_device->pst_wiphy != NULL)) {
        pst_device->is_presentation_mode = pst_config_para->value;
        oam_warning_log1(0, OAM_SF_ANY, "{HiD2D Presentation::set HiD2D Presentation mode[%d].}",
            pst_device->is_presentation_mode);
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_hid2d_switch_channel
 * 功能描述  : HiD2D 场景下GO 通过CSA 机制切换信道
 * 1.日    期  : 2020年03月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_hid2d_switch_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_csa_debug_stru *csa_cfg = NULL;
    mac_device_stru *mac_device = NULL;
    mac_vap_stru *p2p_mac_vap = NULL;
    uint8_t vap_index;
    uint8_t no_p2p_vap_flag = OAL_TRUE;

    /* 找到mac_device */
    mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return OAL_SUCC;
    }

    /* 找到挂接在该device上的p2p vap */
    for (vap_index = 0; vap_index < mac_device->uc_vap_num; vap_index++) {
        p2p_mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_index]);
        if (p2p_mac_vap == NULL) {
            continue;
        }
        if (p2p_mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE && p2p_mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
            no_p2p_vap_flag = OAL_FALSE;
            break;
        }
    }
    if (no_p2p_vap_flag == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANTI_INTF, "{HiD2D Presentation::no p2p vap error!}");
        return OAL_FAIL;
    }

    csa_cfg = (mac_csa_debug_stru *)puc_param;
    mac_device->uc_csa_vap_cnt = 1;
    p2p_mac_vap->st_ch_switch_info.en_csa_mode = WLAN_CSA_MODE_TX_DISABLE;
    p2p_mac_vap->st_ch_switch_info.uc_ch_switch_cnt = HMAC_CHANNEL_SWITCH_COUNT; /* CSA cnt 设置为5 */
    hmac_chan_initiate_switch_to_new_channel(p2p_mac_vap, csa_cfg->uc_channel, csa_cfg->en_bandwidth);
    return OAL_SUCC;
}

#endif

/*
 * 函 数 名  : hmac_config_set_dada_collect_switch
 * 功能描述  : 设置数采接口
 * 1.日    期  : 2020年4月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_config_set_dada_collect_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    g_st_data_collect_cfg.uc_type = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->uc_cmd_type;
    g_st_data_collect_cfg.reg_num = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{set_dada_collect::type:%d,reg num %d.}",
        g_st_data_collect_cfg.uc_type, g_st_data_collect_cfg.reg_num);
}
OAL_STATIC void hmac_config_set_dyn_pcie_switch(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    g_dyn_pcie_switch = (uint8_t)((mac_cfg_set_tlv_stru *)param)->value;
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{set_dada_collect::type:%d,reg num %d.}",
        g_st_data_collect_cfg.uc_type, g_st_data_collect_cfg.reg_num);
}
/*
 * 函 数 名  : hmac_config_set_tlv_cmd
 * 功能描述  : 设置TLV格式命令统一接口
 * 1.日    期  : 2016年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_config_set_tlv_cmd_2(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_config_para = (mac_cfg_set_tlv_stru *)puc_param;

    /* HOST需要处理的事件 */
    switch (pst_config_para->us_cfg_id) {
#ifdef _PRE_WLAN_FEATURE_M2S
        case WLAN_CFGID_MIMO_BLACKLIST:
            g_en_mimo_blacklist = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
            break;
#endif
#if defined(_PRE_WLAN_FEATURE_HID2D) && defined(_PRE_WLAN_FEATURE_HID2D_PRESENTATION)
        case WLAN_CFGID_HID2D_PRESENTATION_MODE:
            hmac_config_set_hid2d_presentation_mode(pst_mac_vap, us_len, puc_param);
            break;
#endif
        case WLAN_CFGID_DATA_COLLECT:
            hmac_config_set_dada_collect_switch(pst_mac_vap, us_len, puc_param);
            break;
        case WLAN_CFGID_SET_ADC_DAC_FREQ:
        case WLAN_CFGID_SET_MAC_FREQ:
        case WLAN_CFGID_SET_WARNING_MODE:
        case WLAN_CFGID_SET_CHR_MODE:
        case WLAN_CFGID_SET_RX_ANTI_IMMU:
#ifdef _PRE_WLAN_FEATURE_DFS_ENABLE
        case WLAN_CFGID_SET_DFS_MODE:
#endif
            break;
        default:
            break;
    }
}

/*
 * 函 数 名  : hmac_config_set_addba_rsp_buffer
 * 功能描述  : 设置修改接收聚合个数的开关
 * 1.日    期  : 2018年6月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_addba_rsp_buffer(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_rx_buffer_size_cfg = NULL;
    hmac_vap_stru *pst_hmac_vap;
    mac_rx_buffer_size_stru *rx_buffer_size = mac_vap_get_rx_buffer_size();

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_rx_ampdu_amsdu::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_rx_buffer_size_cfg = (mac_cfg_set_tlv_stru *)puc_param;

    rx_buffer_size->en_rx_ampdu_bitmap_cmd = pst_rx_buffer_size_cfg->uc_cmd_type;
    if (rx_buffer_size->en_rx_ampdu_bitmap_cmd == OAL_TRUE) {
        rx_buffer_size->us_rx_buffer_size = pst_rx_buffer_size_cfg->value;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_addba_rsp_buffer::ENABLE MODE[%d],buffer size [%d].}",
                     rx_buffer_size->en_rx_ampdu_bitmap_cmd, pst_rx_buffer_size_cfg->value);

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_auto_ba_switch
 * 功能描述  : 设置amsdu+ampdu联合聚合的开关
 * 1.日    期  : 2013年6月20日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_amsdu_ampdu_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_amsdu_ampdu_cfg;
    uint8_t uc_amsdu_en;
    uint8_t uc_index;
    pst_amsdu_ampdu_cfg = (mac_cfg_set_tlv_stru *)puc_param;

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    for (uc_index = 0; uc_index < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_index++) {
        g_st_tx_large_amsdu.en_tx_amsdu_level[uc_index] = (uint8_t)pst_amsdu_ampdu_cfg->value;
    }
#endif

    if (pst_amsdu_ampdu_cfg->value > 0) {
        uc_amsdu_en = OAL_TRUE;
    } else {
        uc_amsdu_en = OAL_FALSE;
    }

    /* 关闭动态切换算法 */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    /* 关闭动态切换算法 */
    g_st_tx_large_amsdu.uc_host_large_amsdu_en = !uc_amsdu_en;
#endif

    /* 设置amsdu状态 */
    mac_mib_set_AmsduPlusAmpduActive(pst_mac_vap, uc_amsdu_en);
    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_amsdu_ampdu_switch::ENABLE LEVEL:%d,en %d.}",
        pst_amsdu_ampdu_cfg->value, !uc_amsdu_en);

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_rx_ampdu_amsdu
 * 功能描述  : 设置rx amsdu+ampdu联合聚合的开关
 * 1.日    期  : 2017年12月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_rx_ampdu_amsdu(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_tlv_stru *pst_rx_ampdu_amsdu_cfg = NULL;
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_rx_ampdu_amsdu::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_rx_ampdu_amsdu_cfg = (mac_cfg_set_tlv_stru *)puc_param;

    pst_hmac_vap->bit_rx_ampduplusamsdu_active = (uint8_t)pst_rx_ampdu_amsdu_cfg->value;

    g_uc_host_rx_ampdu_amsdu = pst_hmac_vap->bit_rx_ampduplusamsdu_active;
    pst_mac_vap->en_ps_rx_amsdu = g_uc_host_rx_ampdu_amsdu;
    pst_hmac_vap->en_ps_rx_amsdu = g_uc_host_rx_ampdu_amsdu;
    pst_mac_vap->uc_ps_type |= MAC_PS_TYPE_CMD;
    hmac_arp_probe_type_set(pst_mac_vap, OAL_FALSE, HMAC_VAP_ARP_PROBE_TYPE_DBAC);
    hmac_arp_probe_type_set(pst_mac_vap, OAL_FALSE, HMAC_VAP_ARP_PROBE_TYPE_BTCOEX);
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_rx_ampdu_amsdu::ENABLE MODE[%d][0:disable,1:enable].}",
                     pst_rx_ampdu_amsdu_cfg->value);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_tlv_cmd
 * 功能描述  : 设置TLV格式命令统一接口
 * 1.日    期  : 2016年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_tlv_cmd(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_set_tlv_stru *pst_config_para = (mac_cfg_set_tlv_stru *)puc_param;

    /* HOST需要处理的事件 */
    switch (pst_config_para->us_cfg_id) {
        case WLAN_CFGID_SET_DYN_PCIE:
            hmac_config_set_dyn_pcie_switch(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;
        case WLAN_CFGID_SET_DEVICE_PKT_STAT:
            hmac_config_set_device_pkt_stat(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;

        case WLAN_CFGID_SET_TX_AMPDU_TYPE:
            hmac_config_set_tx_ampdu_type(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;

        case WLAN_CFGID_SET_DEVICE_FREQ:
            hmac_config_set_auto_freq_enable(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;

        case WLAN_CFGID_AMSDU_AMPDU_SWITCH:
            hmac_config_amsdu_ampdu_switch(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;

        case WLAN_CFGID_SET_RX_AMPDU_AMSDU:
            hmac_config_rx_ampdu_amsdu(pst_mac_vap, us_len, puc_param);
            /* mp13 mpw2 由于存在译码错误导致去使能amsdu默认打开可能导致死机 默认是关闭,不再开启 */
            return OAL_SUCC;
        case WLAN_CFGID_SET_SK_PACING_SHIFT:
            g_sk_pacing_shift = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
            return OAL_SUCC;
        case WLAN_CFGID_SET_TRX_STAT_LOG:
            g_freq_lock_control.uc_trx_stat_log_en = (uint8_t)((mac_cfg_set_tlv_stru *)puc_param)->value;
            return OAL_SUCC;
        case WLAN_CFGID_SET_ADDBA_RSP_BUFFER:
            hmac_config_set_addba_rsp_buffer(pst_mac_vap, us_len, puc_param);
            return OAL_SUCC;
        default:
            break;
    }
    hmac_config_set_tlv_cmd_2(pst_mac_vap, us_len, puc_param);
    /* DEVICE需要处理的事件 */
    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, pst_config_para->us_cfg_id, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_tlv_cmd::send msg failed[%d].}", ret);
    }

    return ret;
}

OAL_STATIC void hmac_config_set_trx_ring_mode(mac_cfg_set_str_stru *pst_set_cmd)
{
    uint8_t uc_cmd_index = 0;
    uint8_t uc_cmd_cnt = pst_set_cmd->uc_cmd_cnt;
    uint16_t us_set_id;
    uint32_t value;

    while (uc_cmd_cnt > 0) {
        us_set_id = pst_set_cmd->us_set_id[uc_cmd_index];
        value = pst_set_cmd->value[uc_cmd_index];
        if (us_set_id == MAC_TRX_TX_RING_SWITCH) {
            g_trx_switch.tx_ring_switch = value;
        } else if (us_set_id == MAC_TRX_RING_SWITCH_INDEPENDENT) {
            g_trx_switch.ring_switch_independent = value;
        } else if (us_set_id == MAC_TRX_HOST_UPDATE_WPTR) {
            g_trx_switch.host_update_wptr = value;
        } else if (us_set_id == MAC_TRX_SOFT_IRQ_SCHED) {
            g_trx_switch.soft_irq_sched = value;
        } else if (us_set_id == MAC_TRX_DEVICE_LOOP_SCHED) {
            g_trx_switch.device_loop_sched = value;
        }
        uc_cmd_index++;
        uc_cmd_cnt--;
    }

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_trx_ring_mode::ini[0x%x], ring switch[%d] independent[%d]}",
        g_trx_switch.tx_ring_switch, g_trx_switch.ring_switch_independent);
    oam_warning_log3(0, OAM_SF_CFG, "{hmac_config_set_trx_ring_mode::wptr[%d] softirq[%d] loop[%d]}",
        g_trx_switch.host_update_wptr, g_trx_switch.soft_irq_sched, g_trx_switch.device_loop_sched);
}

OAL_STATIC void hmac_config_set_trx_debug(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_str_stru *pst_set_cmd = (mac_cfg_set_str_stru *)puc_param;
    if (pst_set_cmd->uc_cmd_cnt > MAC_STR_CMD_MAX_SIZE) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_set_trx_debug::cnt err[%d].}",
            pst_set_cmd->uc_cmd_cnt);
        return;
    }

    hmac_config_set_trx_ring_mode(pst_set_cmd);
}

/*
 * 函 数 名  : hmac_config_set_str_cmd
 * 功能描述  : 设置TLV格式命令统一接口
 * 1.日    期  : 2016年4月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_str_cmd(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_set_str_stru *pst_config_para;

    pst_config_para = (mac_cfg_set_str_stru *)puc_param;

    /* HOST需要处理的事件 */
    switch (pst_config_para->us_cfg_id) {
        case WLAN_CFGID_COMMON_DEBUG:
            hmac_config_common_debug(pst_mac_vap, us_len, puc_param);
            break;
        case WLAN_CFGID_11AX_DEBUG:
            if (g_wlan_spec_cfg->feature_11ax_is_open) {
                hmac_config_ax_debug(pst_mac_vap, us_len, puc_param);
                break;
            }
            hmac_config_log_debug(pst_mac_vap, us_len, puc_param);
            break;
        case WLAN_CFGID_LOG_DEBUG:
            hmac_config_log_debug(pst_mac_vap, us_len, puc_param);
            break;
        case WLAN_CFGID_SET_TRX_DEBUG:
            hmac_config_set_trx_debug(pst_mac_vap, us_len, puc_param);
            break;
        default:
            break;
    }

    /* DEVICE需要处理的事件 */
    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, pst_config_para->us_cfg_id, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_str_cmd::send msg failed[%d].}", ret);
    }

    return ret;
}
