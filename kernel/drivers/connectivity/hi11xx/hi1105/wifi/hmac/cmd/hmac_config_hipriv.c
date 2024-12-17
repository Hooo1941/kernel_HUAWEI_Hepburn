/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 配置相关实现hmac接口实现源文件
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
#include "hmac_vap.h"
#include "hmac_tx_amsdu.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_config.h"
#include "hmac_chan_mgmt.h"
#include "hmac_rx_filter.h"
#include "hmac_protection.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_rx_filter.h"
#include "hmac_mgmt_sta.h"
#include "hmac_blacklist.h"
#include "hmac_degradation.h"
#include "hmac_scan.h"
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
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_HIPRIV_C


/*
 * 函 数 名  : hmac_config_set_rate
 * 功能描述  : 发送设置non-HT速率命令到dmac
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_rate(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RATE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_rate::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_mcs
 * 功能描述  : 发送设置HT速率命令到dmac
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_mcs(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mcs::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_mcsac
 * 功能描述  : 发送设置VHT速率命令到dmac
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_mcsac(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCSAC, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mcsac::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_11AX
uint32_t hmac_config_set_mcsax(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCSAX, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mcsac::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
uint32_t hmac_config_set_mcsax_er(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_MCSAX_ER, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_mcsax_er::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif
#endif

/*
 * 函 数 名  : hmac_config_set_nss
 * 功能描述  : 发送设置空间流命令到dmac
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_nss(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_NSS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_nss::hmac_config_send_event failed[%d].}", ret);
    }
    return ret;
}

/*
 * 函 数 名  : hmac_config_set_rfch
 * 功能描述  : 发送设置通道命令到dmac
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_rfch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RFCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_rfch::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_bw
 * 功能描述  : 发送设置带宽命令到dmac
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_bw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_BW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_bw::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_band
 * 功能描述  : 发送设置band命令到dmac
 * 1.日    期  : 2020年8月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_band(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_BAND, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_band::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    if (*param == 2) { /* 0:2g 1:5g 2:6g */
        mac_vap->st_channel.ext6g_band = OAL_TRUE;
    } else {
        mac_vap->st_channel.ext6g_band = OAL_FALSE;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_always_tx
 * 功能描述  : 发送设置常发模式命令到dmac
 * 1.日    期  : 2015年1月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_always_tx(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t release_flag = OAL_FALSE;
    hal_device_always_tx_enum_uint8 al_tx_flag = ((mac_cfg_tx_comp_stru *)puc_param)->uc_param;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifndef _PRE_LINUX_TEST
    /* 设置host常发状态 */
    hal_set_altx_status(hmac_vap->hal_dev_id, (al_tx_flag == HAL_ALWAYS_TX_DISABLE) ? OAL_FALSE : OAL_TRUE);
#endif
    /* 若不支持ring tx,则直接抛到dmac */
    if (hmac_ring_tx_enabled() != OAL_TRUE) {
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_ALWAYS_TX, us_len, puc_param);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_always_tx::hmac_config_send_event failed[%d].}", ret);
        }
        return ret;
    }
    if (al_tx_flag == HAL_ALWAYS_TX_MPDU) {
        ret = hmac_always_tx_proc(puc_param, hmac_vap->hal_dev_id);
        if (ret != OAL_SUCC) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_always_tx::host tx failed[%d].}",
                ret);
            return ret;
        }
    } else if ((oal_atomic_read(&g_always_tx_ring[hmac_vap->hal_dev_id].msdu_cnt) != 0) &&
        (al_tx_flag == HAL_ALWAYS_TX_DISABLE)) {
        release_flag = OAL_TRUE;
        hmac_al_tx_ring_release(&g_always_tx_ring[hmac_vap->hal_dev_id]);
    }
    if (release_flag != OAL_TRUE) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_always_tx::al tx ring release fail, msdu_cnt[%d] size[%d] al tx mode[%d].}",
            oal_atomic_read(&g_always_tx_ring[hmac_vap->hal_dev_id].msdu_cnt),
            g_always_tx_ring[hmac_vap->hal_dev_id].host_ring_buf_size, al_tx_flag);
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_ALWAYS_TX, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_always_tx::hmac_config_send_event failed[%d].}", ret);
    }
    return ret;
}

/*
 * 功能描述  : 配置ru index
 * 1.日    期  : 2020年3月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_ru_index(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_RU_INDEX, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_ru_index::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
void hmac_config_show_fcs_info(mac_vap_stru *mac_vap)
{
#ifndef _PRE_LINUX_TEST
    dmac_atcmdsrv_atcmd_response_event atcmdsrv_get_rx_pkcg_event = { 0 };
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    if ((hmac_rx_ring_switch_enabled() == OAL_TRUE)) {
        hal_host_get_rx_pckt_info(hmac_vap, &atcmdsrv_get_rx_pkcg_event);
        hal_host_al_rx_fcs_info(hmac_vap);
        hmac_config_get_rx_fcs_info(mac_vap, sizeof(dmac_atcmdsrv_atcmd_response_event),
            (uint8_t *)&atcmdsrv_get_rx_pkcg_event);
    }
#endif
}
/*
 * 功能描述  : 打印接收帧的FCS信息
 * 1.日    期  : 2014年3月8日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_rx_fcs_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_RX_FCS_INFO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_rx_fcs_info::send event fail[%d].}", ret);
        return ret;
    }
    /* host常收功能维测输出 */
    hmac_config_show_fcs_info(pst_mac_vap);
    return OAL_SUCC;
}
/*
 * 功能描述  : 打印al tx pdet信息
 * 1.日    期  : 2023年2月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_tx_pdet_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TX_PDET_VAL, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{get_tx_pdet_info::send event fail[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#if ((defined(_PRE_WLAN_EXPORT)) && (defined(_PRE_WLAN_NARROW_BAND)))
uint32_t hmac_config_get_sw_version(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_SW_VERSION, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{get_tx_pdet_info::send event fail[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

uint32_t hmac_config_get_sw_version_info(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    oal_io_print("hmac_config_get_sw_version_info::%s\r\n", puc_param);
    return OAL_SUCC;
}

uint32_t hmac_config_nb_info_report(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *hmac_vap;
    nb_report_info *report_to_app_info = NULL;
    hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_autorate_level_report::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    report_to_app_info = (nb_report_info *)(puc_param);

    oam_warning_log4(pst_mac_vap->uc_vap_id, 0, "{hmac_config_autorate_level_report:is_connected[%d],"
        "ratelvl[%d],channel[%d],bw[%d].}", report_to_app_info->is_connected, report_to_app_info->rate_level,
        report_to_app_info->channel, report_to_app_info->bw);
    oam_warning_log2(pst_mac_vap->uc_vap_id, 0, "{hmac_config_autorate_level_report:rssi[%d],succ_rate[%d].}",
        (int32_t)report_to_app_info->rssi, report_to_app_info->succ_rate);
    return OAL_SUCC;
}
#endif
/*
 * 功能描述  : 发送设置常收模式命令到dmac
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_always_rx(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
#ifndef _PRE_LINUX_TEST
    uint8_t al_rx_flag = *(oal_bool_enum_uint8 *)puc_param;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_ALWAYS_RX, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_always_rx::send event fail[%d].}", ret);
    }

#ifndef _PRE_LINUX_TEST
    /* 设置host常收状态 */
    hal_set_alrx_status(hmac_vap->hal_dev_id, al_rx_flag);
#endif
    return ret;
}

/*
 * 函 数 名  : hmac_config_set_tx_pow_param
 * 功能描述  : 配置发送功率
 * 1.日    期  : 2016年12月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_tx_pow_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_TX_POW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_tx_pow_param::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_dscr_param
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2013年5月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_dscr_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_DSCR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_dscr_param::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_alg_send_event
 * 功能描述  : 抛配置事件到ALG层
 * 1.日    期  : 2013年10月11日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_alg_send_event(mac_vap_stru *mac_vap, wlan_cfgid_enum_uint16 cfg_id,
    uint16_t len, uint8_t *param)
{
    uint32_t ret;
    frw_event_mem_stru *event_mem = NULL;
    hmac_to_dmac_cfg_msg_stru *syn_msg = NULL;

    ret = hmac_config_alloc_event(mac_vap, HMAC_TO_DMAC_SYN_ALG, &syn_msg, &event_mem, len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_alg_send_event::hmac_config_alloc_event failed[%d].}", ret);
        return ret;
    }

    syn_msg->en_syn_id = cfg_id;
    syn_msg->us_len = len;
    /* 填写配置同步消息内容 */
    if (memcpy_s(syn_msg->auc_msg_body, len, param, len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_alg_send_event::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 抛出事件 */
    ret = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_alg_send_event::frw_event_dispatch_event failed[%d].}", ret);
        frw_event_free_m(event_mem);
        return ret;
    }

    frw_event_free_m(event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_roam_start
 * 功能描述  : 命令方式强制启动漫游
 * 1.日    期  : 2015年6月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_roam_start(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg_set_roam_start_stru *pst_roam_start = NULL;
    hmac_roam_info_stru *roam_info = NULL;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_roam_start::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_roam_start = (mac_cfg_set_roam_start_stru *)(puc_param);
    roam_info = (hmac_roam_info_stru *)pst_hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info->is_roaming_trigged_by_cmd = pst_roam_start->is_roaming_trigged_by_cmd;
    if (ether_is_all_zero(pst_roam_start->auc_bssid)) {
        /* reassociation or roaming */
        return hmac_roam_start(pst_hmac_vap, (roam_channel_org_enum)pst_roam_start->uc_scan_type,
                               pst_roam_start->en_current_bss_ignore, NULL, ROAM_TRIGGER_APP);
    } else if (!oal_memcmp(pst_mac_vap->auc_bssid, pst_roam_start->auc_bssid, OAL_MAC_ADDR_LEN)) {
        /* reassociation */
        return hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_APP);
    } else {
        /* roaming for specified BSSID */
        return hmac_roam_start(pst_hmac_vap, ROAM_SCAN_CHANNEL_ORG_DBDC,
                               OAL_TRUE, pst_roam_start->auc_bssid, ROAM_TRIGGER_BSSID);
    }
}
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
void hmac_set_user_ampdu_tx_on(hmac_vap_stru *hmac_vap)
{
    uint8_t tid_idx;
    hmac_user_stru *hmac_user;
    oal_dlist_head_stru *entry;
    oal_dlist_head_stru *next_entry;
    mac_user_stru *mac_user;

    oal_spin_lock_bh(&hmac_vap->st_ampdu_lock);
    /* 先删除聚合,切换完毕后再使能聚合 */
    mac_mib_set_CfgAmpduTxAtive(&hmac_vap->st_vap_base_info, OAL_FALSE);

    oal_dlist_search_for_each_safe(entry, next_entry, &(hmac_vap->st_vap_base_info.st_mac_user_list_head)) {
        mac_user = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
        hmac_user = mac_res_get_hmac_user(mac_user->us_assoc_id);
        if (hmac_user == NULL) {
            continue;
        }

        for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
            if (hmac_user->ast_tid_info[tid_idx].st_ba_tx_info.en_ba_status == DMAC_BA_COMPLETE) {
                hmac_tx_ba_del(hmac_vap, hmac_user, tid_idx);
            }
        }
    }
    oal_spin_unlock_bh(&hmac_vap->st_ampdu_lock);
    oal_workqueue_delay_schedule(&(hmac_vap->st_ampdu_work), oal_msecs_to_jiffies(3000)); /* 延迟3000ms */
}
void hmac_set_ampdu_tx_on(hmac_vap_stru *hmac_vap, mac_vap_stru *mac_vap)
{
    uint8_t vap_idx, dev_idx, dev_max;
    mac_chip_stru *mac_chip;
    mac_device_stru *mac_device;

    /* 遍历删除BA */
    mac_chip = hmac_res_get_mac_chip(mac_vap->uc_chip_id);
    if (oal_unlikely(mac_chip == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: mac_chip null!}");
        return;
    }
    dev_max = oal_chip_get_device_num(mac_chip->chip_ver);
    for (dev_idx = 0; dev_idx < dev_max; dev_idx++) {
        mac_device = mac_res_get_dev(mac_chip->auc_device_id[dev_idx]);
        if (mac_device == NULL) {
            continue;
        }

        for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
            hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->auc_vap_id[vap_idx]);
            if (hmac_vap == NULL) {
                continue;
            }
            hmac_set_user_ampdu_tx_on(hmac_vap);
        }
    }
}
#endif
/*
 * 函 数 名  : hmac_config_set_ampdu_tx_on
 * 功能描述  : hmac设置ampdu tx 开关
 * 1.日    期  : 2015年5月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_ampdu_tx_on(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_ampdu_tx_on_param_stru *p_ampdu_tx_on = NULL;
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    hmac_vap_stru *hmac_vap;
#endif

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    p_ampdu_tx_on = (mac_cfg_ampdu_tx_on_param_stru *)puc_param;

    oam_warning_log3(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: tx_aggr_on[0x%x], snd type[%d],aggr mode[%d]!}",
        p_ampdu_tx_on->uc_aggr_tx_on, p_ampdu_tx_on->uc_snd_type, p_ampdu_tx_on->en_aggr_switch_mode);

    /* ampdu_tx_on为0、1,删建聚合 */
    if ((uint8_t)(p_ampdu_tx_on->uc_aggr_tx_on & (~(BIT1 | BIT0))) == 0) {
        mac_mib_set_CfgAmpduTxAtive(mac_vap, p_ampdu_tx_on->uc_aggr_tx_on & BIT0);

        /* ampdu_tx_on为2、3,删建聚合,并且切换硬件聚合 */
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
        /* 切换为硬件聚合时才需要下发事件 */
        if (p_ampdu_tx_on->uc_aggr_tx_on & BIT1) {
            p_ampdu_tx_on->uc_aggr_tx_on &= BIT0; /* enable hw ampdu */
            hmac_config_send_event(mac_vap, WLAN_CFGID_AMPDU_TX_ON, us_len, puc_param);
        }
#endif
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* 1.无需删建BA切换方式 */
    if (p_ampdu_tx_on->en_aggr_switch_mode == AMPDU_SWITCH_BY_BA_LUT) {
        p_ampdu_tx_on->uc_aggr_tx_on &= BIT2; /* 4:enable hw ampdu; 8:disable */
        p_ampdu_tx_on->uc_aggr_tx_on = p_ampdu_tx_on->uc_aggr_tx_on >> NUM_2_BITS;
        hmac_config_send_event(mac_vap, WLAN_CFGID_AMPDU_TX_ON, us_len, puc_param);
        return OAL_SUCC;
    }

    /* 2.需删建BA切换方式 */
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_ampdu_tx_on:: hmac_vap null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (p_ampdu_tx_on->uc_aggr_tx_on & (~(BIT1 | BIT0))) {
        /* 提交切换硬件聚合work */
        memcpy_s(&hmac_vap->st_mode_set, sizeof(mac_cfg_ampdu_tx_on_param_stru),
                 p_ampdu_tx_on, sizeof(mac_cfg_ampdu_tx_on_param_stru));
        oal_workqueue_delay_schedule(&(hmac_vap->st_set_hw_work), oal_msecs_to_jiffies(2000)); /* 延迟2000ms */

        hmac_set_ampdu_tx_on(hmac_vap, mac_vap);
    }
#endif

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_dbdc_debug_switch
 * 功能描述  : dbdc配置命令接口
 * 1.日    期  : 2017年06月02日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_pm_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PM_DEBUG_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_DBDC,
                         "{hmac_config_pm_debug_switch::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_dbdc_debug_switch
 * 功能描述  : dbdc配置命令接口
 * 1.日    期  : 2017年06月02日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_dbdc_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    hmac_device_stru *pst_hmac_device;
    mac_dbdc_debug_switch_stru *pst_dbdc_debug_switch;
    cali_data_req_stru cali_data_req_info;

    pst_dbdc_debug_switch = (mac_dbdc_debug_switch_stru *)puc_param;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_DBDC,
                         "{hmac_config_dbdc_debug_switch::hmac device[%d] is null.", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (pst_dbdc_debug_switch->cmd_bit_map & (BIT(MAC_DBDC_CHANGE_HAL_DEV) | BIT(MAC_SINGLE_SLAVE_SCAN))) {
        cali_data_req_info.channel = pst_mac_vap->st_channel;
        cali_data_req_info.mac_vap_id = pst_mac_vap->uc_vap_id;
        cali_data_req_info.work_cali_data_sub_type = CALI_DATA_NORMAL_JOIN_TYPE;
        wlan_chip_update_cur_chn_cali_data(&cali_data_req_info);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DBDC_DEBUG_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_DBDC,
                         "{hmac_config_dbdc_debug_switch::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
#define RX_LISTEN_ALL_VOTE_ENABLE_MASK  (BIT(MAC_RX_LISTEN_CFG80211_POWER_MGMT) | BIT(MAC_RX_LISTEN_IOCTL_GAME_CTRL))
uint8_t g_rx_listen_ps_open_bitmap = RX_LISTEN_ALL_VOTE_ENABLE_MASK;
void hmac_set_rx_listen_ps_switch(mac_rx_listen_ps_switch_stru *rx_listen_ps_switch)
{
    if (rx_listen_ps_switch->rx_listen_enable == OAL_TRUE) {
        g_rx_listen_ps_open_bitmap |= BIT(rx_listen_ps_switch->rx_listen_ctrl_type);
    } else {
        g_rx_listen_ps_open_bitmap &= ~BIT(rx_listen_ps_switch->rx_listen_ctrl_type);
    }
}
uint32_t hmac_config_set_rx_listen_ps_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    mac_rx_listen_ps_switch_stru *rx_listen_ps_switch = (mac_rx_listen_ps_switch_stru *)puc_param;
    hmac_set_rx_listen_ps_switch(rx_listen_ps_switch);
    if (g_wlan_spec_cfg->rx_listen_ps_is_open) {
        /* 若使能powersaving，需检查低功耗使能跟非游戏场景都已置位 */
        if ((rx_listen_ps_switch->rx_listen_enable == OAL_TRUE) &&
            (g_rx_listen_ps_open_bitmap != RX_LISTEN_ALL_VOTE_ENABLE_MASK)) {
            return OAL_SUCC;
        }
        oam_warning_log2(0, 0, "hmac_config_set_rx_listen_ps_switch::en[%d] bitmap[%d]",
            rx_listen_ps_switch->rx_listen_enable, g_rx_listen_ps_open_bitmap);
        return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_RX_LISTEN_PS_SWITCH, us_len,
            (uint8_t *)&rx_listen_ps_switch->rx_listen_enable);
    }
    return OAL_SUCC;
}

void hmac_config_start_ap_enable_rx_listen(mac_vap_stru *mac_vap)
{
    mac_rx_listen_ps_switch_stru rx_listen_ps_switch = { 0 };
    rx_listen_ps_switch.rx_listen_ctrl_type = MAC_RX_LISTEN_CFG80211_POWER_MGMT;
    rx_listen_ps_switch.rx_listen_enable = OAL_TRUE;
    hmac_config_set_rx_listen_ps_switch(mac_vap, sizeof(mac_rx_listen_ps_switch_stru), (uint8_t *)&rx_listen_ps_switch);
}
#endif
/*
 * 函 数 名  : hmac_config_set_sta_pm_mode
 * 功能描述  : 配置staut低功耗模式
 * 1.日    期  : 2015年10月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_sta_pm_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_SET_PS_MODE, us_len, puc_param);
}
/*
 * 函 数 名  : hmac_config_set_sta_pm_on
 * 功能描述  : 打开staut低功耗
 * 1.日    期  : 2015年10月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_sta_pm_on(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_ps_mode_param_stru st_ps_mode_param;
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg_ps_open_stru *pst_sta_pm_open = (mac_cfg_ps_open_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{hmac_config_set_sta_pm_on::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 切换到手动设置为pspoll模式 */
    if (pst_sta_pm_open->uc_pm_enable == MAC_STA_PM_MANUAL_MODE_ON) {
        pst_hmac_vap->uc_cfg_sta_pm_manual = OAL_TRUE;
    } else if (pst_sta_pm_open->uc_pm_enable == MAC_STA_PM_MANUAL_MODE_OFF) {
        /* 关闭手动设置pspoll模式,回到fastps模式 */
        pst_hmac_vap->uc_cfg_sta_pm_manual = OAL_FALSE;
    }

    pst_sta_pm_open->uc_pm_enable =
        (pst_sta_pm_open->uc_pm_enable > MAC_STA_PM_SWITCH_OFF) ? MAC_STA_PM_SWITCH_ON : MAC_STA_PM_SWITCH_OFF;

    st_ps_mode_param.uc_vap_ps_mode = pst_sta_pm_open->uc_pm_enable ?
        ((pst_hmac_vap->uc_cfg_sta_pm_manual != OAL_FALSE) ? MAX_PSPOLL_PS : pst_hmac_vap->uc_ps_mode) : NO_POWERSAVE;

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "hmac_config_set_sta_pm_on,enable[%d], ps_mode[%d]",
                     pst_sta_pm_open->uc_pm_enable, st_ps_mode_param.uc_vap_ps_mode);
    /* 先下发设置低功耗模式 */
    ret = hmac_config_set_sta_pm_mode(pst_mac_vap, sizeof(st_ps_mode_param), (uint8_t *)&st_ps_mode_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "sta_pm sta_pm mode[%d]fail", ret);
        return ret;
    }

    /* 再下发打开低功耗 */
    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_SET_STA_PM_ON, us_len, puc_param);
}

#ifdef _PRE_WLAN_FEATURE_M2S_MODEM
/*
 * 函 数 名  : hmac_config_set_m2s_switch_modem
 * 功能描述  : 上层下发MODEM模式切换
 * 1.日    期  : 2019.10.15
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_m2s_switch_modem(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_M2S_MODEM, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_config_set_m2s_switch_modem::failed[%d].}", ret);
    }

    return ret;
}
#endif

/*
 * 函 数 名  : hmac_atcmdsrv_get_rx_pkcg
 * 功能描述  : 查询FCS校验正确的包数
 * 1.日    期  : 2015年10月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_rx_fcs_info(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_get_rx_pkcg_event = NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_get_rx_fcs_info::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_atcmdsrv_get_rx_pkcg_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_get_rx_pkcg_event->uc_event_id != OAL_ATCMDSRV_GET_RX_PKCG) {
        /* only pkct_succ_num valid, atcmd need to wait rssi from dmac */
        pst_hmac_vap->st_atcmdsrv_get_status.rx_pkct_succ_num = pst_atcmdsrv_get_rx_pkcg_event->event_para;
        return OAL_SUCC;
    }
    if (pst_atcmdsrv_get_rx_pkcg_event->uc_event_id == OAL_ATCMDSRV_GET_RX_PKCG) {
        if (pst_atcmdsrv_get_rx_pkcg_event->uc_reserved != OAL_ATCMDSRV_GET_RX_PKCG) {
            /* since mp16, only rssi is valid */
            pst_hmac_vap->st_atcmdsrv_get_status.rx_pkct_succ_num = pst_atcmdsrv_get_rx_pkcg_event->event_para;
        }
        pst_hmac_vap->st_atcmdsrv_get_status.s_rx_rssi = pst_atcmdsrv_get_rx_pkcg_event->s_always_rx_rssi;
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_config_get_rx_fcs_info:: rx_pkct_succ_num=[%d], rx_rssi=[%d]}",
            pst_hmac_vap->st_atcmdsrv_get_status.rx_pkct_succ_num,
            pst_hmac_vap->st_atcmdsrv_get_status.s_rx_rssi);
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_rx_pkct_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_tx_pdet_info
 * 功能描述  : 查询altx pdet
 * 1.日    期  : 2015年10月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_tx_pdet_info(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_get_rx_fcs_info::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_vap->st_atcmdsrv_get_status.dbb_num = *(uint32_t *)puc_param;
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_mode
 * 功能描述  : 获取模式 包括协议 频段 带宽
 * 1.日    期  : 2012年12月24日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_cfg_mode_param_stru *pst_prot_param;

    pst_prot_param = (mac_cfg_mode_param_stru *)puc_param;

    pst_prot_param->en_protocol = pst_mac_vap->en_protocol;
    pst_prot_param->en_band = pst_mac_vap->st_channel.en_band;
    pst_prot_param->en_bandwidth = pst_mac_vap->st_channel.en_bandwidth;

    *pus_len = sizeof(mac_cfg_mode_param_stru);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_mode_check_freq
 * 功能描述  : 设置mode时，检查device当前的带宽模式的限制
 * 1.日    期  : 2013年7月29日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_config_set_mode_check_freq(wlan_channel_band_enum_uint8 en_band_config,
                                                    wlan_channel_band_enum_uint8 en_band_device)
{
    if ((WLAN_BAND_5G == en_band_config) && (WLAN_BAND_CAP_2G == en_band_device)) {
        /* 设置5G频带，但device不支持5G */
        oam_warning_log2(0, OAM_SF_CFG,
                         "{hmac_config_check_mode_param::not support 5GHz band,en_protocol=%d en_protocol_cap=%d.}",
                         en_band_config, en_band_device);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    } else if ((WLAN_BAND_2G == en_band_config) && (WLAN_BAND_CAP_5G == en_band_device)) {
        /* 设置2G频带，但device不支持2G */
        oam_warning_log2(0, OAM_SF_CFG,
                         "{hmac_config_check_mode_param::not support 2GHz band,en_protocol=%d en_protocol_cap=%d.}",
                         en_band_config, en_band_device);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_config_check_dev_protocol_cap_para(mac_device_stru *pst_mac_device,
    wlan_protocol_enum_uint8 protocol)
{
    switch (protocol) {
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11B_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            break;

        case WLAN_HT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
            if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_HT) {
                /* 设置11n协议，但device不支持HT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                    "{hmac_config_check_dev_protocol_cap_para::not support HT mode,protocol=%d protocol_cap=%d.}",
                    protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_VHT) {
                /* 设置11ac协议，但device不支持VHT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                    "{hmac_config_check_dev_protocol_cap_para::not support VHT mode,protocol=%d protocol_cap=%d.}",
                    protocol, pst_mac_device->en_protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
            if (g_wlan_spec_cfg->feature_11ax_is_open) {
                if (pst_mac_device->en_protocol_cap < WLAN_PROTOCOL_CAP_HE) {
                    /* 设置11aX协议，但device不支持HE模式 */
                    oam_error_log2(0, OAM_SF_CFG,
                        "{hmac_config_check_dev_protocol_cap_para::not support HE mode,protocol=%d protocol_cap=%d.}",
                        protocol, pst_mac_device->en_protocol_cap);
                    return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
                }
                break;
            }
            oam_error_log0(0, OAM_SF_CFG, "{hmac_config_check_dev_protocol_cap_para::protocol does not in the list.}");
            break;
#endif

        default:
            oam_error_log0(0, OAM_SF_CFG, "{hmac_config_check_dev_protocol_cap_para::protocol does not in the list.}");
            break;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_set_mode_check_bandwith
 * 功能描述  : 非首次设置带宽时根据已配置带宽检查新配置带宽参数
 * 1.日    期  : 2013年11月18日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_config_set_mode_check_bandwith(wlan_channel_bandwidth_enum_uint8 en_bw_device,
                                                        wlan_channel_bandwidth_enum_uint8 en_bw_config)
{
    /* 要配置带宽是20M */
    if (WLAN_BAND_WIDTH_20M == en_bw_config) {
        return OAL_SUCC;
    }

    /* 要配置带宽与首次配置带宽相同 */
    if (en_bw_device == en_bw_config) {
        return OAL_SUCC;
    }

    switch (en_bw_device) {
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            if (WLAN_BAND_WIDTH_40PLUS == en_bw_config) {
                return OAL_SUCC;
            }
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            if (WLAN_BAND_WIDTH_40MINUS == en_bw_config) {
                return OAL_SUCC;
            }
            break;

        case WLAN_BAND_WIDTH_40PLUS:
            if ((WLAN_BAND_WIDTH_80PLUSPLUS == en_bw_config) || (WLAN_BAND_WIDTH_80PLUSMINUS == en_bw_config)) {
                return OAL_SUCC;
            }
            break;
        case WLAN_BAND_WIDTH_40MINUS:
            if ((WLAN_BAND_WIDTH_80MINUSPLUS == en_bw_config) || (WLAN_BAND_WIDTH_80MINUSMINUS == en_bw_config)) {
                return OAL_SUCC;
            }
            break;
        case WLAN_BAND_WIDTH_20M:
            return OAL_SUCC;

        default:
            break;
    }

    return OAL_FAIL;
}
/*
 * 函 数 名  : hmac_config_check_mode_param
 * 功能描述  : 设置mode时，协议 频段 带宽参数检查
 * 1.日    期  : 2013年7月29日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_config_check_mode_param(
    mac_device_stru *mac_device, mac_vap_stru *mac_vap, mac_cfg_mode_param_stru *mode_param)
{
    uint32_t ret;

    /* 根据device能力对参数进行检查 */
    if (hmac_config_check_dev_protocol_cap_para(mac_device, mode_param->en_protocol) != OAL_SUCC) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if ((mac_vap_bw_mode_to_bw(mode_param->en_bandwidth) >= WLAN_BW_CAP_80M)
        && (mac_mib_get_dot11VapMaxBandWidth(mac_vap) < WLAN_BW_CAP_80M)) {
        /* 设置80M带宽，但device能力不支持80M，返回错误码 */
        oam_warning_log2(0, OAM_SF_CFG,
            "{hmac_config_check_mode_param::not support 80MHz bandwidth,en_protocol=%d en_protocol_cap=%d.}",
            mode_param->en_bandwidth, mac_mib_get_dot11VapMaxBandWidth(mac_vap));
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    ret = hmac_config_set_mode_check_freq(mode_param->en_band, mac_device->en_band_cap);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* device已经配置时，需要校验下频段、带宽是否一致 */
    if ((mac_device->en_max_bandwidth != WLAN_BAND_WIDTH_BUTT) && (!mac_dbac_enable(mac_device)) &&
        (mac_device->uc_vap_num > 1)) {
        if (mac_device->en_max_band != mode_param->en_band) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_check_mode_param::previous vap band[%d] \
                mismatch with [%d].}", mac_device->en_max_band, mode_param->en_band);
            return OAL_FAIL;
        }

        ret = hmac_config_set_mode_check_bandwith(mac_device->en_max_bandwidth, mode_param->en_bandwidth);
        if (ret != OAL_SUCC) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_check_mode_param::check_bandwith failed[%d],previous vap bandwidth[%d, current[%d].}",
                ret, mac_device->en_max_bandwidth, mode_param->en_bandwidth);
            return ret;
        }
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_set_protocol_and_bw
 * 功能描述  : 设置实际能支持的协议模式和带宽
 * 1.日    期  : 2020年11月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_set_protocol_and_bw(mac_vap_stru *mac_vap, mac_cfg_mode_param_stru *prot_param)
{
    mac_vap->st_cap_flag.bit_11ac2g = wlan_chip_get_11ac2g_enable();

    if ((mac_vap->st_cap_flag.bit_11ac2g == OAL_FALSE) && (mac_vap->en_protocol == WLAN_VHT_MODE) &&
        (mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_protocol_and_bw::11ac2g not supported.");
        oal_io_print("{hmac_config_set_protocol_and_bw::11ac2g not supported.\n");
        prot_param->en_protocol = WLAN_HT_MODE;
    }

    if ((mac_vap->st_cap_flag.bit_disable_2ght40 == OAL_TRUE) && (prot_param->en_bandwidth != WLAN_BAND_WIDTH_20M) &&
        (mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_protocol_and_bw::2ght40 not supported.");
        oal_io_print("{hmac_config_set_protocol_and_bw::2ght40 not supported.\n");
        prot_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
}
OAL_STATIC void hmac_update_device_max_band_and_bandwidth(
    mac_device_stru *mac_device, mac_cfg_mode_param_stru *mode_param)
{
    if ((mac_device->en_max_bandwidth != WLAN_BAND_WIDTH_BUTT) && (hmac_calc_up_ap_num(mac_device) != 0)) {
        return;
    }
    mac_device->en_max_bandwidth = mode_param->en_bandwidth;
    mac_device->en_max_band = mode_param->en_band;
}

OAL_STATIC void hmac_set_mode_update_mib_by_protocol(mac_vap_stru *mac_vap, wlan_protocol_enum_uint8 protocol)
{
    if (protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(mac_vap, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(mac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(mac_vap, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(mac_vap, OAL_FALSE);
    }
}
OAL_STATIC void hmac_set_sta_mode(hmac_vap_stru *hmac_vap, mac_cfg_mode_param_stru *mode_param)
{
    if (hmac_vap->st_vap_base_info.en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return;
    }
    hmac_vap->bit_sta_protocol_cfg = OAL_SWITCH_ON;
    hmac_vap->st_preset_para.en_protocol = mode_param->en_protocol;
    hmac_vap->st_preset_para.en_bandwidth = mode_param->en_bandwidth;
    hmac_vap->st_preset_para.en_band = mode_param->en_band;
}

/*
 * 函 数 名  : hmac_config_set_mode
 * 功能描述  : 设置模式 包括协议 频段 带宽
 * 1.日    期  : 2012年12月24日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_mode(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *param)
{
    mac_cfg_mode_param_stru *prot_param = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);

    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::mac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
    /* 设置模式时，device下必须至少有一个vap */
    if (mac_device->uc_vap_num == 0) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::no vap in device.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    prot_param = (mac_cfg_mode_param_stru *)param;

    /* 检查配置参数是否在device能力内 */
    ret = hmac_config_check_mode_param(mac_device, mac_vap, prot_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::check_mode_param failed[%d].}", ret);
        return ret;
    }
    hmac_set_mode_update_mib_by_protocol(mac_vap, prot_param->en_protocol);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hmac_config_set_protocol_and_bw(mac_vap, prot_param);
#endif
    /* 更新STA协议配置标志位 */
    hmac_set_sta_mode(hmac_vap, prot_param);
    /* 记录协议模式, band, bandwidth到mac_vap下 */
    mac_vap->st_channel.en_band = prot_param->en_band;
    mac_vap->st_channel.en_bandwidth = prot_param->en_bandwidth;
    mac_vap->st_ch_switch_info.en_user_pref_bandwidth = prot_param->en_bandwidth;

#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    if ((prot_param->en_protocol < WLAN_HT_MODE) || (OAL_TRUE != mac_device_get_cap_subfee(mac_device))) {
        mac_vap->st_cap_flag.bit_11ntxbf = OAL_FALSE;
    }
#endif
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::protocol=%d, band=%d, bandwidth=%d.}",
        prot_param->en_protocol, mac_vap->st_channel.en_band, mac_vap->st_channel.en_bandwidth);

    /* 根据协议更新vap能力 */
    mac_vap_init_by_protocol(mac_vap, prot_param->en_protocol);
    /* 更新device的频段及最大带宽信息 */
    hmac_update_device_max_band_and_bandwidth(mac_device, prot_param);
    /***************************************************************************
     抛事件到DMAC层, 配置寄存器
    ***************************************************************************/
    ret = hmac_set_mode_event(mac_vap);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mode::send_event failed[%d].}", ret);
        return ret;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_set_channel_160m_invalid
 * 功能描述  : 检测是否在不支持160M时，下发160M带宽
 * 1.日    期  : 2019.4.8
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint8_t hmac_set_channel_160m_invalid(mac_vap_stru *pst_mac_vap, mac_cfg_channel_param_stru *pst_prot_param)
{
    hmac_device_stru *pst_hmac_device;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_set_channel_160m_invalid::pst_hmac_device null,divice_id=%d.}", pst_mac_vap->uc_device_id);
        return OAL_TRUE;
    }

    if ((pst_prot_param->en_band == WLAN_BAND_5G) &&
        (pst_prot_param->en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
        (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) && (pst_hmac_device->en_ap_support_160m == OAL_FALSE)) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_set_channel_160m_invalid::aput not support 160M");
        return OAL_TRUE;
    }

    if ((mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap) <= WLAN_BW_CAP_80M) &&
        (pst_prot_param->en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS)) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_set_channel_160m_invalid::NOT support 160M!");
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*
 * 函 数 名  : hmac_config_set_channel_check_param
 * 功能描述  : 设置Channnel时，协议 频段 带宽参数检查
 * 1.日    期  : 2014年8月15日
 *   修改内容  : 新生成函数
 */
static uint32_t hmac_config_set_channel_check_param(mac_vap_stru *pst_mac_vap,
    mac_cfg_channel_param_stru *pst_prot_param)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_channel_check_param::pst_mac_device null,divice_id=%d.}",
            pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据device能力对参数进行检查 */
    if ((pst_prot_param->en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap) < WLAN_BW_CAP_80M)) {
        /* 设置80M带宽，但device能力不支持80M，返回错误码 */
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_channel_check_param::"
                         "not support 80MHz bandwidth,en_protocol=%d en_dot11VapMaxBandWidth=%d.}",
                         pst_prot_param->en_bandwidth, mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap));
        return OAL_ERR_CODE_CONFIG_BW_EXCEED;
    }

    if ((WLAN_BAND_5G == pst_prot_param->en_band) && (WLAN_BAND_CAP_2G == pst_mac_device->en_band_cap)) {
        /* 设置5G频带，但device不支持5G */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_channel_check_param::"
                         "not support 5GHz band,en_protocol=%d en_protocol_cap=%d.}",
                         pst_prot_param->en_band, pst_mac_device->en_band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    } else if ((WLAN_BAND_2G == pst_prot_param->en_band) && (WLAN_BAND_CAP_5G == pst_mac_device->en_band_cap)) {
        /* 设置2G频带，但device不支持2G */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_channel_check_param::"
                         "not support 2GHz band,en_protocol=%d en_protocol_cap=%d.}",
                         pst_prot_param->en_band, pst_mac_device->en_band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* 若vap不支持2g 40M则，返回不支持该带宽的错误码 */
    if ((WLAN_BAND_2G == pst_prot_param->en_band) && (WLAN_BAND_WIDTH_20M < pst_prot_param->en_bandwidth) &&
        (OAL_FALSE == mac_mib_get_2GFortyMHzOperationImplemented(pst_mac_vap))) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 不支持160M，但是上层下发160M带宽 */
    if (OAL_TRUE == hmac_set_channel_160m_invalid(pst_mac_vap, pst_prot_param)) {
        return OAL_ERR_CODE_CONFIG_BW_EXCEED;
    }

    return OAL_SUCC;
}

oal_bool_enum_uint8 hmac_check_up_vap_num_is_valid(mac_vap_stru *mac_vap)
{
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        return OAL_FALSE;
    }
    /* 下面这句用于检查up的vap个数是否达到该芯片规格最大值 */
    /* max_work_vap_num在不同的芯片中有不同的定义，支持3vap时3，否则为2. */
    if (mac_device_calc_up_vap_num(mac_device) >= g_wlan_spec_cfg->max_work_vap_num) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}
/*
 * 函 数 名  : hmac_config_chan_bw_exceed
 * 功能描述  : 上层配置带宽超出硬件能力处理函数
 * 1.日    期  : 2019.4.8
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_chan_bw_exceed(mac_vap_stru *pst_mac_vap, mac_cfg_channel_param_stru *pst_channel_param)
{
    if (WLAN_BAND_WIDTH_160PLUSPLUSPLUS <= pst_channel_param->en_bandwidth) {
        pst_channel_param->en_bandwidth = mac_vap_get_bandwith(WLAN_BW_CAP_80M,
                                                               pst_channel_param->en_bandwidth);
    } else {
        pst_channel_param->en_bandwidth = mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap),
                                                               pst_channel_param->en_bandwidth);
    }
}

static void hmac_set_channel_changed_report(mac_vap_stru *mac_vap, mac_channel_stru *dest_channel)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CHAN, "hmac_chan_switch_to_new_chan_check::get hmac vap NULL");
        return;
    }
    ret = hmac_report_channel_switch(hmac_vap, dest_channel);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHAN,
            "hmac_chan_switch_to_new_chan_check::return fail is[%d]", ret);
    }
}

/* 功能描述：启动ap/go/chba，配置信道参数检查 */
static uint32_t hmac_config_set_channel_check(mac_vap_stru *mac_vap, mac_cfg_channel_param_stru *channel_param)
{
    mac_channel_stru set_mac_channel = {0};
    oal_bool_enum_uint8 channel_changed = OAL_FALSE;
    uint32_t ret;

    set_mac_channel.en_band = channel_param->en_band;
    set_mac_channel.en_bandwidth = channel_param->en_bandwidth;
    set_mac_channel.uc_chan_number = channel_param->uc_channel;
    set_mac_channel.ext6g_band = channel_param->ext6g_band; /* P2P 6G适配：置位信道的6G标志位 */

    /* 信道跟随检查
     * 创建CHBA VAP,不在创建时信道跟随，而是通过上层切换CHBA信道;
     * 创建AP/GO，在创建时直接信道跟随
     */
    if (is_ap(mac_vap) == OAL_TRUE && mac_is_chba_mode(mac_vap) == OAL_FALSE) {
        if (hmac_check_ap_channel_follow_other_vap(mac_vap, channel_param, &set_mac_channel) == OAL_SUCC) {
            channel_changed = OAL_TRUE;
        }
    }
    /* 检查配置参数是否在device能力内 */
    ret = hmac_config_set_channel_check_param(mac_vap, channel_param);
    if (ret == OAL_ERR_CODE_CONFIG_BW_EXCEED) {
        hmac_config_chan_bw_exceed(mac_vap, channel_param);
    }

    if (ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::channel check failed[%d].}", ret);
        return ret;
    }

    /* 该错误码表示不支持2g 40M,故更改带宽为20M */
    if (ret == OAL_ERR_CODE_CONFIG_UNSUPPORT) {
        channel_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
    if (hmac_check_up_vap_num_is_valid(mac_vap) != OAL_TRUE) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_check_up_vap_num_is_valid::vap num is invalid.}");
        return OAL_FAIL;
    }

    if (mac_get_channel_idx_from_num(set_mac_channel.en_band, set_mac_channel.uc_chan_number,
        set_mac_channel.ext6g_band, &(set_mac_channel.uc_chan_idx)) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 检查aput、chba、go的入网信道组合是否支持，不支持2p2p组dbac、3vap纯dbac、3vap同频异信道 */
    if (hmac_check_coex_channel_is_valid(mac_vap, &set_mac_channel) != OAL_TRUE) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::channel invalid!}");
        hmac_chr_mvap_report(mac_vap, MAC_MVAP_ERROR_SCENE_START);
        return OAL_FAIL;
    }

    if (channel_changed == OAL_TRUE) {
        /* 若驱动修改过ap/go工作信道，则上报新的信道给上层应用 */
        hmac_set_channel_changed_report(mac_vap, &set_mac_channel);
    }
    return OAL_SUCC;
}
uint32_t hmac_config_set_channel_check_band_and_bandwidth(mac_device_stru *mac_device, mac_vap_stru *mac_vap,
    mac_cfg_channel_param_stru *channel_param, uint32_t up_vap_cnt, oal_bool_enum_uint8 *en_set_reg)
{
    uint32_t ret;

    if (up_vap_cnt <= 1) {
        /* 记录首次配置的带宽值 */
        mac_device_set_channel(mac_device, channel_param);

        /***************************************************************************
         抛事件到DMAC层, 配置寄存器  置标志位
        ***************************************************************************/
        *en_set_reg = OAL_TRUE;
    } else if (mac_is_dbac_enabled(mac_device) == OAL_TRUE) {
        /* 开启DBAC不进行信道判断 */
        /* 信道设置只针对AP模式，非AP模式则跳出 */
    } else {
        /* 信道不是当前信道 */
        if (mac_device->uc_max_channel != channel_param->uc_channel) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::previous vap channel"
                "number=%d mismatch [%d].}", mac_device->uc_max_channel, channel_param->uc_channel);

            return OAL_FAIL;
        }

        /* 带宽不能超出已配置的带宽 */
        ret = hmac_config_set_mode_check_bandwith(mac_device->en_max_bandwidth, channel_param->en_bandwidth);
        if (ret != OAL_SUCC) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel:: \
                hmac_config_set_mode_check_bandwith failed[%d], previous vap bandwidth[%d, current[%d].}",
                ret, mac_device->en_max_bandwidth, channel_param->en_bandwidth);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

uint32_t hmac_config_set_channel_set_band_and_bandwidth(mac_device_stru *mac_device, mac_vap_stru *mac_vap,
    mac_cfg_channel_param_stru *channel_param, oal_bool_enum_uint8 *en_set_reg)
{
    mac_vap_stru *mac_vap_tmp = NULL;
    uint32_t ret;
    uint8_t vap_idx;

    if (mac_is_dbac_enabled(mac_device) == OAL_TRUE) {
        mac_vap->st_channel.uc_chan_number = channel_param->uc_channel;
        mac_vap->st_channel.en_band = channel_param->en_band;
        mac_vap->st_channel.en_bandwidth = channel_param->en_bandwidth;
        mac_vap->st_channel.ext6g_band = channel_param->ext6g_band; /* P2P 6G适配：置位信道的6G标志位 */

        ret = mac_get_channel_idx_from_num(channel_param->en_band, channel_param->uc_channel,
                                           channel_param->ext6g_band, &(mac_vap->st_channel.uc_chan_idx));
        if (ret != OAL_SUCC) {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHAN,
                "{hmac_config_set_channel::mac_get_channel_idx_from_num failed[%d], band[%d], channel[%d].}",
                ret, channel_param->en_band, channel_param->uc_channel);
            return OAL_FAIL;
        }

        *en_set_reg = OAL_TRUE;
    } else {
        for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
            mac_vap_tmp = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
            if (mac_vap_tmp == NULL) {
                continue;
            }
            mac_vap_tmp->st_channel.uc_chan_number = channel_param->uc_channel;
            mac_vap_tmp->st_channel.en_band = channel_param->en_band;
            mac_vap_tmp->st_channel.en_bandwidth = channel_param->en_bandwidth;
            mac_vap_tmp->st_channel.ext6g_band = channel_param->ext6g_band; /* P2P 6G适配：置位信道的6G标志位 */
            ret = mac_get_channel_idx_from_num(channel_param->en_band, channel_param->uc_channel,
                channel_param->ext6g_band, &(mac_vap_tmp->st_channel.uc_chan_idx));
            if (ret != OAL_SUCC) {
                oam_warning_log3(mac_vap_tmp->uc_vap_id, OAM_SF_CHAN,
                    "{hmac_config_set_channel::mac_get_channel_idx_from_num failed[%d], band[%d], channel[%d].}",
                    ret, channel_param->en_band, channel_param->uc_channel);
                continue;
            }
        }
    }

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHAN, "hmac_config_set_channel: channel_num:%d, bw:%d, band:%d",
                     channel_param->uc_channel, channel_param->en_bandwidth, channel_param->en_band);

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_go_not_start_160m_handle
 * 功能描述  : go 准备启动160M时，检查另外一路sta是否存在，如果存在返回TRUE
               1)sta与新addGO信道一致且在36-48之间仍返回false
 * 1.日    期  : 2019.2.9
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_go_not_start_160m_handle(mac_vap_stru *p_go_mac_vap,
    uint8_t go_channel, wlan_channel_bandwidth_enum_uint8 *p_bandwidth)
{
#ifdef _PRE_WLAN_FEATURE_160M
    wlan_channel_bandwidth_enum_uint8 new_80bw;
    wlan_bw_cap_enum_uint8 bw_cap;
    mac_cfg_mib_by_bw_param_stru st_cfg;

    bw_cap = mac_vap_bw_mode_to_bw(*p_bandwidth);
    if (!is_p2p_go(p_go_mac_vap) || bw_cap != WLAN_BW_CAP_160M) {
        return;
    }

    /* 只有GO 且 160M 带宽时才处理 */
    if (OAL_TRUE == mac_vap_go_can_not_in_160m_check(p_go_mac_vap, go_channel)) {
        new_80bw = mac_regdomain_get_support_bw_mode(WLAN_BW_CAP_80M, go_channel);
        bw_cap = mac_vap_bw_mode_to_bw(new_80bw);
        if (bw_cap == WLAN_BW_CAP_80M) {
            oam_warning_log2(0, OAM_SF_QOS, "{hmac_config_go_not_start_160m_handle::\
                bw from [%d] change to [%d]}", *p_bandwidth, new_80bw);
            *p_bandwidth = new_80bw;

            st_cfg.en_band = p_go_mac_vap->st_channel.en_band;
            st_cfg.en_bandwidth = *p_bandwidth;
            hmac_config_set_mib_by_bw(p_go_mac_vap, (uint16_t)sizeof(st_cfg), (uint8_t *)&st_cfg);
        }
    }
#endif
}

/*
 * 函 数 名  : hmac_config_set_channel
 * 功能描述  : HMAC 层设置信道信息
 * 1.日    期  : 2013年10月26日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_channel(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret, up_vap_cnt;
    oal_bool_enum_uint8 en_set_reg = OAL_FALSE;
    oal_bool_enum_uint8 en_override = OAL_FALSE;

    mac_cfg_channel_param_stru *channel_param = (mac_cfg_channel_param_stru *)puc_param;

    /* 获取device */
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log1(0, OAM_SF_CHAN, "{hmac_config_set_channel::mac_device null,dev_id=%d.}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* p2p go场景下启动160m时，切换至80m */
    hmac_config_go_not_start_160m_handle(mac_vap, channel_param->uc_channel, &channel_param->en_bandwidth);

    ret = hmac_config_set_channel_check(mac_vap, channel_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    up_vap_cnt = hmac_calc_up_ap_num(mac_device);

#ifdef _PRE_WLAN_FEATURE_DFS
    en_override |= ((mac_vap_get_dfs_enable(mac_vap) && mac_dfs_get_cac_enable(mac_device))) ? OAL_TRUE : OAL_FALSE;
#endif

    en_override |= (mac_get_2040bss_switch(mac_device) && (!mac_vap->bit_bw_fixed)) ? OAL_TRUE : OAL_FALSE;

    en_override &= !mac_is_dbac_enabled(mac_device) ? OAL_TRUE : OAL_FALSE;

    en_override &= (up_vap_cnt > 1) ? OAL_TRUE : OAL_FALSE;

    /* 初始扫描使能时，运行时临时设置不同的信道 */
    if (en_override) {
        mac_device_get_channel(mac_device, channel_param);
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::force chan band=%d ch=%d bw=%d}",
                         channel_param->en_band, channel_param->uc_channel, channel_param->en_bandwidth);
    }

    /* 仅在没有VAP up的情况下，配置硬件频带、带宽寄存器 */
    ret = hmac_config_set_channel_check_band_and_bandwidth(mac_device, mac_vap, channel_param, up_vap_cnt, &en_set_reg);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* zero wait dfs(aput)场景下启动160m时，切换至80m,目前:1106支持，mp13/mp15不支持 */
    wlan_chip_start_zero_wait_dfs(mac_vap, channel_param);

    ret = hmac_config_set_channel_set_band_and_bandwidth(mac_device, mac_vap, channel_param, &en_set_reg);
    if (ret != OAL_SUCC) {
        return ret;
    }

    hmac_cali_send_work_channel_cali_data(mac_vap, &(mac_vap->st_channel), CALI_DATA_NORMAL_JOIN_TYPE);
    /***************************************************************************
     抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    if (en_set_reg == OAL_TRUE) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CHAN, "hmac_config_set_channel::send event to dmac, set register");
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CFG80211_SET_CHANNEL, us_len, puc_param);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_config_set_channel::send event failed[%d].}", ret);
            return ret;
        }
    }

    /* hostapd配置带宽记录信息，同步dmac操作40m恢复定时器 */
    hmac_40m_intol_sync_data(mac_vap, mac_vap->st_channel.en_bandwidth, OAL_FALSE);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_init_vap_and_rate_by_protocol
 * 功能描述  : 根据协议模式初始化vap能力及速率集
 * 1.日    期  : 2019.11.16
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_init_vap_and_rate_by_protocol(mac_vap_stru *pst_mac_vap,
    wlan_protocol_enum_uint8 en_protocol)
{
    wlan_nss_enum_uint8 en_ori_rx_nss = pst_mac_vap->en_vap_rx_nss;

    mac_vap_init_by_protocol(pst_mac_vap, en_protocol);

    /* GO已经UP，则此时空间流能力已经更新，取交集处理 */
    if ((pst_mac_vap->en_p2p_mode == WLAN_P2P_GO_MODE) &&
        (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP)) {
        pst_mac_vap->en_vap_rx_nss = oal_min(pst_mac_vap->en_vap_rx_nss, en_ori_rx_nss);
    }

    mac_vap_init_rates(pst_mac_vap);
}

static void hmac_config_set_beacon_update_mib(mac_vap_stru *mac_vap, mac_beacon_param_stru *beacon_param)
{
    if (beacon_param->en_protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(mac_vap, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(mac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(mac_vap, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(mac_vap, OAL_FALSE);
    }

    mac_vap_set_hide_ssid(mac_vap, beacon_param->uc_hidden_ssid);

    /* mp12适配新内核start ap和change beacon接口复用此接口，不同的是change beacon时，不再设置beacon周期
       和dtim周期，因此，change beacon时，interval和dtim period参数为全零，此时不应该被设置到mib中 */
    /* 设置VAP beacon interval， dtim_period */
    if ((beacon_param->l_dtim_period != 0) || (beacon_param->l_interval != 0)) {
        mac_mib_set_dot11dtimperiod(mac_vap, (uint32_t)beacon_param->l_dtim_period);
        mac_mib_set_BeaconPeriod(mac_vap, (uint32_t)beacon_param->l_interval);
    }

    /* 设置short gi */
    mac_mib_set_ShortGIOptionInTwentyImplemented(mac_vap, beacon_param->en_shortgi_20);
    mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap, beacon_param->en_shortgi_40);

    if ((mac_vap->st_channel.en_band == WLAN_BAND_2G) && (mac_vap->st_cap_flag.bit_disable_2ght40 == OAL_TRUE)) {
        mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap, OAL_FALSE);
    }

    mac_mib_set_VHTShortGIOptionIn80Implemented(mac_vap, beacon_param->en_shortgi_80);
}

static void hmac_config_set_beacon_update_akm_suite(hmac_vap_stru *hmac_vap)
{
    uint32_t akm_suite[WLAN_AUTHENTICATION_SUITES] = { 0 };
    uint16_t suite_idx;

    mac_mib_get_rsn_akm_suites_s(&hmac_vap->st_vap_base_info, (uint32_t *)akm_suite, sizeof(akm_suite));

    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_beacon::mac_mib_get_rsn_akm_suites_s 0x[%x], 0x[%x].}", akm_suite[0], akm_suite[1]);

    for (suite_idx = 0; suite_idx < WLAN_AUTHENTICATION_SUITES; suite_idx++) {
        if (akm_suite[suite_idx] == MAC_RSN_AKM_OWE) {
            hmac_vap->owe_group[BYTE_OFFSET_0] = MAC_OWE_GROUP_19; /* APUT default supported OWE group */
            hmac_vap->owe_group[BYTE_OFFSET_1] = MAC_OWE_GROUP_20;
            hmac_vap->owe_group[BYTE_OFFSET_2] = MAC_OWE_GROUP_21;
            break;
        }
    }
}

/*
 * 函 数 名  : hmac_config_set_beacon
 * 功能描述  : HMAC 层设置AP 信息
 * 1.日    期  : 2015年6月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_beacon(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_beacon_param_stru *beacon_param = (mac_beacon_param_stru *)param;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;

    if (oal_any_null_ptr2(mac_vap, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device */
    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon:: mac_device or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检查协议配置参数是否在device能力内 */
    ret = hmac_config_check_dev_protocol_cap_para(mac_device, beacon_param->en_protocol);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::check param failed[%d].}", ret);
        return ret;
    }

    hmac_config_set_beacon_update_mib(mac_vap, beacon_param);

    ret = mac_vap_set_security(mac_vap, beacon_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::set_security failed[%d].}", ret);
        return ret;
    }

    hmac_config_set_beacon_update_akm_suite(hmac_vap);
    hmac_init_vap_and_rate_by_protocol(mac_vap, beacon_param->en_protocol);

    /***************************************************************************
     抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CFG80211_CONFIG_BEACON, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_beacon::send event failed[%d].}", ret);
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 同步vap修改信息到device侧 */
    hmac_config_vap_m2s_info_syn(mac_vap);
#endif

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_SNIFFER
/*
 * 功能描述  : 设置sniffer功能开或关
 * 1.日    期  : 2019年5月12日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_sniffer(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return g_wlan_chip_ops->set_sniffer_config(mac_vap, len, param);
}

uint32_t hmac_config_set_sniffer_mp13(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    mac_cfg_sniffer_param_stru *cfg_sniffer_param = NULL;

    cfg_sniffer_param = (mac_cfg_sniffer_param_stru *)param;
    if (oal_value_eq_any2(cfg_sniffer_param->uc_sniffer_mode, WLAN_SNIFFER_TRAVEL_CAP_ON, WLAN_SINFFER_ON)) {
        uint32_t pedding_data = 0;
        hmac_config_scan_abort(mac_vap, sizeof(uint32_t), (uint8_t *)&pedding_data);
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_sniffer: in sniffer mode, scan abort!}");
    }
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_sniffer::set sniffer %d",
        cfg_sniffer_param->uc_sniffer_mode);

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SNIFFER, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_sniffer::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
OAL_STATIC void hmac_set_sniffer_init_device(
    hmac_device_stru *hmac_device, mac_cfg_sniffer_param_stru *cfg_sniffer_param)
{
    hmac_device->sniffer_mode = (cfg_sniffer_param->uc_sniffer_mode != 0) ? WLAN_SINFFER_ON : WLAN_SINFFER_OFF;
    if (hmac_device->sniffer_mode != WLAN_SINFFER_OFF) {
        hmac_device->rssi0 = 0;
        hmac_device->rssi1 = 0;
        hmac_device->mgmt_frames_cnt = 0;
        hmac_device->control_frames_cnt = 0;
        hmac_device->data_frames_cnt = 0;
        hmac_device->others_frames_cnt = 0;
    }
    oal_set_mac_addr(hmac_device->monitor_mac_addr, cfg_sniffer_param->auc_mac_addr);
    if (!ether_is_broadcast(cfg_sniffer_param->auc_mac_addr)) {
        hmac_device->addr_filter_on = OAL_TRUE;
    } else {
        hmac_device->addr_filter_on = OAL_FALSE;
    }
}
uint32_t hmac_config_set_sniffer_mp16(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    mac_cfg_sniffer_param_stru *cfg_sniffer_param = NULL;
    mac_rx_switch_stru  rx_switch = {0};
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    hal_host_device_stru *hal_device = hal_get_host_device(mac_vap->uc_device_id);

    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_sniffer::pst_hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    cfg_sniffer_param = (mac_cfg_sniffer_param_stru *)param;
    if (oal_value_eq_any2(cfg_sniffer_param->uc_sniffer_mode, WLAN_SNIFFER_TRAVEL_CAP_ON, WLAN_SINFFER_ON)) {
        uint32_t pedding_data = 0;
        hmac_config_scan_abort(mac_vap, sizeof(uint32_t), (uint8_t *)&pedding_data);
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_sniffer: in sniffer mode, scan abort!}");
    }
    hmac_set_sniffer_init_device(hmac_device, cfg_sniffer_param);
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_sniffer::set sniffer %d.",
        cfg_sniffer_param->uc_sniffer_mode);
    /* 切换rx mode */
    rx_switch.hal_device_id = 0;
    if (hmac_device->sniffer_mode == WLAN_SINFFER_ON) {
        /* 初始化 rx ppdu dscr free ring */
        if (hal_host_sniffer_rx_ppdu_free_ring_init(hal_device) != OAL_SUCC) {
            oam_error_log0(0, 0, "hmac_config_set_sniffer::init_rx_ppdu_free_ring fail.");
            return OAL_FAIL;
        }
    } else {
        hal_host_sniffer_rx_ppdu_free_ring_deinit(hal_device);
    }
    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SNIFFER, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_sniffer::hmac_config_send_event failed[%d].}", ret);
    }
    return ret;
}
#endif
/*
 * 功能描述  : 设置地址过滤能力位
 * 1.日    期  : 2019年5月12日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_monitor_mode(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_device_stru *hmac_device;
    uint32_t ret;

    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{hmac_config_set_monitor_mode::hmac_res_get_mac_dev fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device->en_monitor_mode = *param;
    if (hmac_device->en_monitor_mode == OAL_TRUE) {
        uint32_t pedding_data = 0;
        hmac_config_scan_abort(mac_vap, sizeof(uint32_t), (uint8_t *)&pedding_data);
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_monitor_mode: in sniffer monitor mode, scan abort!}");
    }

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_monitor_mode::set monitor mode %d", hmac_device->en_monitor_mode);

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_monitor_mode::mgmt[%d],control[%d],data[%d],others[%d]",
        hmac_device->mgmt_frames_cnt, hmac_device->control_frames_cnt, hmac_device->data_frames_cnt,
        hmac_device->others_frames_cnt);

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_MONITOR_MODE, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_monitor_mode::send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_get_monitor
 * 功能描述  : 读前地址过滤能力位
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_monitor(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_monitor::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((int32_t *)puc_param) = pst_hmac_vap->en_monitor_mode;
    *pus_len = sizeof(int32_t);
    return OAL_SUCC;
}
uint32_t hmac_config_set_pt_mcast(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_pt_mcast::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap->en_pt_mcast_switch = *puc_param;
    return OAL_SUCC;
}


/*
 * 函 数 名  : hmac_config_set_prot_mode
 * 功能描述  : 设置保护模式
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_prot_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    int32_t l_value;
    uint32_t ret;

    l_value = *((int32_t *)puc_param);

    if (oal_unlikely(l_value >= WLAN_PROT_BUTT)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_prot_mode::invalid l_value[%d].}", l_value);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_mac_vap->st_protection.en_protection_mode = (uint8_t)l_value;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROT_MODE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_prot_mode::send_event failed[%d]}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_get_prot_mode
 * 功能描述  : 读取保护模式
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_prot_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = pst_mac_vap->st_protection.en_protection_mode;
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_auth_mode
 * 功能描述  : 设置认证模式
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_auth_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_mib_set_AuthenticationMode(pst_mac_vap, *puc_param);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_auth_mode::set auth mode[%d] succ.}", mac_mib_get_AuthenticationMode(pst_mac_vap));
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_auth_mode
 * 功能描述  : 读取认证模式
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_auth_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = mac_mib_get_AuthenticationMode(pst_mac_vap);
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_nobeacon
 * 功能描述  : 设置no beacon
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_nobeacon(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    uint32_t ret;
    int32_t l_value;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_nobeacon::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    l_value = *((int32_t *)puc_param);
    pst_hmac_vap->en_no_beacon = (uint8_t)l_value;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NO_BEACON, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_nobeacon::send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_get_nobeacon
 * 功能描述  : 读取no beacon
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_nobeacon(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_nobeacon::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *((int32_t *)puc_param) = pst_hmac_vap->en_no_beacon;
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_txpower
 * 功能描述  : 设置发送功率
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_txpower(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    int32_t l_value;
    uint8_t uc_value;
    uint32_t ret;

    l_value = (*((int32_t *)puc_param) < 0) ? 0 : (*((int32_t *)puc_param));

    uc_value = (uint8_t)((l_value + 5) / 10); /* 加5除以10 四舍五入 */
    mac_vap_set_tx_power(pst_mac_vap, uc_value);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TX_POWER, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_txpower::send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_get_txpower
 * 功能描述  : 读取发送功率
 * 1.日    期  : 2013年1月15日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_txpower(mac_vap_stru *mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = mac_vap->uc_tx_power;
    *pus_len = sizeof(int32_t);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_reset_hw
 * 功能描述  : 设置UAPSD使能
 * 1.日    期  : 2013年9月18日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_reset_hw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_RESET_HW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_reset_hw::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

uint32_t hmac_config_set_reset_state(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
    mac_reset_sys_stru *pst_reset_sys = NULL;
    hmac_device_stru *pst_hmac_device;

    pst_reset_sys = (mac_reset_sys_stru *)puc_param;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG,
                       "{hmac_config_set_reset_state::pst_hmac_device[%d] is null.}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_device_set_dfr_reset(pst_hmac_device->pst_device_base_info, pst_reset_sys->uc_value);

    /* 待整改，当前存在DMAC TO HMAC SYNC，待处理。处理后做抛事件处理 */
    return ret;
}

/* 将配置命令结构转换为OAM结构 */
OAL_STATIC void hmac_config_init_oam_switch(mac_cfg_80211_ucast_switch_stru *mac_80211_switch_param,
    oam_80211_frame_ctx_union *oam_switch_param)
{
    oam_switch_param->frame_ctx.bit_content = mac_80211_switch_param->en_frame_switch;
    oam_switch_param->frame_ctx.bit_cb = mac_80211_switch_param->en_cb_switch;
    oam_switch_param->frame_ctx.bit_dscr = mac_80211_switch_param->en_dscr_switch;
    oam_switch_param->frame_ctx.bit_msdu_dscr = mac_80211_switch_param->sub_switch.stru.bit_msdu_dscr;
    oam_switch_param->frame_ctx.bit_ba_info = mac_80211_switch_param->sub_switch.stru.bit_ba_info;
    oam_switch_param->frame_ctx.bit_himit_dscr = mac_80211_switch_param->sub_switch.stru.bit_himit_dscr;
    oam_switch_param->frame_ctx.bit_mu_dscr = mac_80211_switch_param->sub_switch.stru.bit_mu_dscr;
}

/*
 * 函 数 名  : hmac_config_80211_ucast_switch
 * 功能描述  : 设置80211单播帧收发开关
 * 1.日    期  : 2014年5月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_80211_ucast_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_80211_ucast_switch_stru *mac_80211_switch_param;
    oam_80211_frame_ctx_union oam_switch_param = {0};
    uint16_t us_user_idx = 0;
    uint16_t us_max_user_idx;
    uint32_t ret;

    mac_80211_switch_param = (mac_cfg_80211_ucast_switch_stru *)puc_param;

    us_max_user_idx = mac_board_get_max_user();

    /* 将配置命令结构转换为OAM结构 */
    hmac_config_init_oam_switch(mac_80211_switch_param, &oam_switch_param);

    /* 广播地址，操作所有用户的单播帧开关 */
    if (ether_is_broadcast(mac_80211_switch_param->auc_user_macaddr)) {
        for (us_user_idx = 0; us_user_idx < us_max_user_idx; us_user_idx++) {
            oam_report_80211_ucast_set_switch(mac_80211_switch_param->en_frame_direction,
                                              mac_80211_switch_param->en_frame_type,
                                              &oam_switch_param, us_user_idx);
        }
    } else {
        ret = mac_vap_find_user_by_macaddr(pst_mac_vap, mac_80211_switch_param->auc_user_macaddr, &us_user_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_80211_ucast_switch::mac_vap_find_user_by_macaddr[%02X:XX:XX:XX:%02X:%02X]failed!}",
                mac_80211_switch_param->auc_user_macaddr[MAC_ADDR_0],
                mac_80211_switch_param->auc_user_macaddr[MAC_ADDR_4],
                mac_80211_switch_param->auc_user_macaddr[MAC_ADDR_5]);
            return ret;
        }

        ret = oam_report_80211_ucast_set_switch(mac_80211_switch_param->en_frame_direction,
                                                mac_80211_switch_param->en_frame_type,
                                                &oam_switch_param, us_user_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_80211_ucast_switch::Set switch of report_ucast failed[%d]!"
                "frame_switch[%d], cb_switch[%d], dscr_switch[%d].}",
                ret, mac_80211_switch_param->en_frame_switch,
                mac_80211_switch_param->en_cb_switch,
                mac_80211_switch_param->en_dscr_switch);
            return ret;
        }
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_80211_UCAST_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_80211_ucast_switch::hmac_config_send_event fail[%d].", ret);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_mgmt_log
 * 功能描述  : 打开或关闭用户管理帧维测
 * 1.日    期  : 2016年4月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_mgmt_log(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_start)
{
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch = {0};

    if (oal_any_null_ptr2(pst_mac_vap, pst_mac_user)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_start != OAL_TRUE) {
        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_OFF;
        oal_set_mac_addr(st_80211_ucast_switch.auc_user_macaddr, pst_mac_user->auc_user_mac_addr);
        hmac_config_80211_ucast_switch(pst_mac_vap, sizeof(st_80211_ucast_switch),
                                       (uint8_t *)&st_80211_ucast_switch);

        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_OFF;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_OFF;
        hmac_config_80211_ucast_switch(pst_mac_vap, sizeof(st_80211_ucast_switch),
                                       (uint8_t *)&st_80211_ucast_switch);
    } else {
        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
        oal_set_mac_addr(st_80211_ucast_switch.auc_user_macaddr, pst_mac_user->auc_user_mac_addr);

        hmac_config_80211_ucast_switch(pst_mac_vap, sizeof(st_80211_ucast_switch),
                                       (uint8_t *)&st_80211_ucast_switch);

        st_80211_ucast_switch.en_frame_direction = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
        st_80211_ucast_switch.en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
        st_80211_ucast_switch.en_frame_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_cb_switch = OAL_SWITCH_ON;
        st_80211_ucast_switch.en_dscr_switch = OAL_SWITCH_ON;
        hmac_config_80211_ucast_switch(pst_mac_vap, sizeof(st_80211_ucast_switch),
                                       (uint8_t *)&st_80211_ucast_switch);
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_phy_debug_switch
 * 功能描述  : 设置打印phy_debug信息的开关
 * 1.日    期  : 2015年3月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_phy_debug_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
#ifdef _PRE_WLAN_FEATURE_DFS
    mac_phy_debug_switch_stru *pst_phy_debug_switch;

    pst_phy_debug_switch = (mac_phy_debug_switch_stru *)puc_param;
    if (pst_phy_debug_switch->uc_report_radar_switch == OAL_TRUE) {
        ret = hmac_dfs_radar_detect_event_test(pst_mac_vap->uc_vap_id);
        if (ret != OAL_SUCC) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_phy_debug_switch::hmac_dfs_radar_detect_event_test failed[%d].}", ret);
            return ret;
        }
    }
#endif
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PHY_DEBUG_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_phy_debug_switch::hmac_config_send_event fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_set_ampdu_aggr_num
 * 功能描述  : 设置聚合最大个数
 * 1.日    期  : 2014年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_ampdu_aggr_num(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_AGGR_NUM, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_ampdu_aggr_num::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_alg_param
 * 功能描述  : hmac, 算法配置命令示例
 * 1.日    期  : 2013年10月11日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_alg_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    /***************************************************************************
        抛事件到ALG层, 同步ALG数据
    ***************************************************************************/
    return hmac_config_alg_send_event(pst_mac_vap, WLAN_CFGID_ALG_PARAM, us_len, puc_param);
}

/*
 * 函 数 名  : hmac_config_alg
 * 功能描述  : 算法配置接口
 * 1.日    期  : 2014年1月22日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_alg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t uc_idx;

    for (uc_idx = sizeof(mac_ioctl_alg_config_stru); uc_idx < us_len; uc_idx++) {
        if (puc_param[uc_idx] == ' ') {
            puc_param[uc_idx] = 0;
        }
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_ALG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_alg::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

static oal_bool_enum_uint8 is_country_code_reduce_2g_pwr(int8_t *country_code, uint32_t len)
{
    uint32_t i;
    char data[][COUNTRY_CODE_LEN] = {
        "CN",
    };
    for (i = 0; i < sizeof(data) / COUNTRY_CODE_LEN; i++) {
        if (oal_strncasecmp(country_code, data[i], OAL_STRLEN(data[i])) == 0) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/*
 * 在2G 40MHz场景，中心信道是定制化指定信道，则降低为20MHz 带宽
 */
wlan_channel_bandwidth_enum_uint8 hmac_config_update_2g_40mhz_bandwidth(mac_vap_stru *mac_vap,
    const mac_channel_stru *channel)
{
    uint8_t channel_num;
    uint32_t reduce_pwr_2g_40mhz_channel_bitmap;
    wlan_channel_bandwidth_enum_uint8 new_bandwidth;

    if (oal_any_null_ptr2(mac_vap, channel) == OAL_TRUE) {
        return WLAN_BAND_WIDTH_20M;
    }

    new_bandwidth = channel->en_bandwidth;
    if (is_country_code_reduce_2g_pwr(mac_regdomain_get_country(), COUNTRY_CODE_LEN) != OAL_TRUE) {
        return channel->en_bandwidth;
    }

    if (channel->en_band != WLAN_BAND_2G) {
        return channel->en_bandwidth;
    }

    if (wlan_bandwidth_to_bw_cap(channel->en_bandwidth) == WLAN_BW_CAP_20M) {
        return channel->en_bandwidth;
    }

    if (mac_is_channel_num_valid(channel->en_band, channel->uc_chan_number, OAL_FALSE) != OAL_SUCC) {
        return channel->en_bandwidth;
    }

    /* 根据信道和带宽，计算中心信道 */
    channel_num = wlan_get_center_freq_idx(channel);

    reduce_pwr_2g_40mhz_channel_bitmap = wlan_chip_get_reduce_pwr_channel_2g_bitmap();
    if (reduce_pwr_2g_40mhz_channel_bitmap & BIT(channel_num)) {
        /* 需要降带宽为20MHz */
        new_bandwidth = WLAN_BAND_WIDTH_20M;
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
            "hmac_config_update_2g_40mhz_bandwidth: channel %d, change bw from %d to %d, customize_bitmap 0x%X",
            channel->uc_chan_number, channel->en_bandwidth, new_bandwidth, reduce_pwr_2g_40mhz_channel_bitmap);
    }

    return new_bandwidth;
}
