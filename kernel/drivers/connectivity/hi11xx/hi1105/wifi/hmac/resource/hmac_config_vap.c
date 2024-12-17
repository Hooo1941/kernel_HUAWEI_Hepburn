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
#include "hmac_resource_restriction.h"
#include "mac_vap.h"
#include "mac_ie.h"
#include "mac_function.h"
#include "oal_hcc_host_if.h"
#include "mac_user.h"
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
#include "hmac_pm.h"
#include "hmac_dfx.h"

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
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
#include "hmac_lp_miracast.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_VAP_C
uint8_t g_p2p_go_max_user_num = 0;

/*
 * 函 数 名  : hmac_cfg_vap_send_event
 * 功能描述  : 创建配置vap抛事件
 * 1.日    期  : 2013年5月29日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_cfg_vap_send_event(mac_device_stru *pst_device)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = NULL;
    uint32_t ret;

    /* 抛事件给DMAC,让DMAC完成配置VAP创建 */
    pst_event_mem = frw_event_alloc_m(0);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_cfg_vap_send_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_HOST_CRX, HMAC_TO_DMAC_SYN_CREATE_CFG_VAP,
        0, FRW_EVENT_PIPELINE_STAGE_1, pst_device->uc_chip_id, pst_device->uc_device_id, pst_device->uc_cfg_vap_id);

    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_cfg_vap_send_event::frw_event_dispatch_event failed[%d].}", ret);
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);

    return ret;
}

/*
 * 函 数 名  : hmac_config_vap_state_syn
 * 功能描述  : dmac_offload架构下同步vap的状态到device
 * 1.日    期  : 2014年12月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_vap_state_syn(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_VAP_STATE_SYN, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_vap_state_syn::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_vap_classify_en
 * 功能描述  : 配置使能基于vap的流分类功能
 * 1.日    期  : 2014年11月24日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_vap_classify_en(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t val;
    mac_device_stru *pst_mac_device = NULL;
    int8_t ac_string[OAM_PRINT_FORMAT_LENGTH];

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_warning_log1(0, OAM_SF_P2P, "hmac_config_vap_classify_en::mac_res_get_dev fail.device_id = %u",
                         pst_mac_vap->uc_device_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    val = *((uint32_t *)puc_param);

    if (val == 0xff) {
        /* 打印当前的值 */
        snprintf_s(ac_string, sizeof(ac_string), sizeof(ac_string) - 1,
                   "device classify en is %u\n",
                   pst_mac_device->en_vap_classify);

        oam_print(ac_string);

        return OAL_SUCC;
    }

    if (val == 0) {
        pst_mac_device->en_vap_classify = OAL_FALSE;
    } else {
        pst_mac_device->en_vap_classify = OAL_TRUE;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_vap_classify_tid
 * 功能描述  : 配置使能基于vap的流分类功能
 * 1.日    期  : 2014年11月24日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_vap_classify_tid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t val;
    hmac_vap_stru *pst_hmac_vap = NULL;
    int8_t ac_string[OAM_PRINT_FORMAT_LENGTH];

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "hmac_config_vap_classify_tid::mac_res_get_hmac_vap fail.vap_id = %u", pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    val = *((uint32_t *)puc_param);

    if (val == 0xff) {
        /* 打印当前的值 */
        snprintf_s(ac_string, sizeof(ac_string), sizeof(ac_string) - 1,
                   "vap classify tid is %u\n",
                   mac_mib_get_VAPClassifyTidNo(pst_mac_vap));

        oam_print(ac_string);

        return OAL_SUCC;
    }

    if (val >= WLAN_TIDNO_BUTT) {
        /* 打印当前的值 */
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "val is invalid:%d, vap classify tid is %d",
                         val, mac_mib_get_VAPClassifyTidNo(pst_mac_vap));
        return OAL_SUCC;
    }

    mac_mib_set_VAPClassifyTidNo(pst_mac_vap, (uint8_t)val);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_vap_wmm_switch
 * 功能描述  : hmac get wmm switch
 * 1.日    期  : 2017年08月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_vap_wmm_switch(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_get_vap_wmm_switch::wmm switch[%d].}", pst_mac_vap->en_vap_wmm);
    *puc_param = pst_mac_vap->en_vap_wmm;
    *pus_len = sizeof(uint32_t);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_vap_wmm_switch
 * 功能描述  : hmac set wmm switch
 * 1.日    期  : 2017年08月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_vap_wmm_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if (pst_mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* set wmm_en status */
    pst_mac_vap->en_vap_wmm = (oal_bool_enum_uint8)*puc_param;
    /* 开关WMM，修改mib信息位中的Qos位 */
    mac_mib_set_dot11QosOptionImplemented(pst_mac_vap, pst_mac_vap->en_vap_wmm);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_vap_wmm_switch::wmm switch[%d].}", pst_mac_vap->en_vap_wmm);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_sta_pm_switch_syn
 * 功能描述  : dmac_offload架构下同步sta  的低功耗开关
 * 1.日    期  : 2015年04月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_set_ipaddr_timeout(void *puc_para)
{
    uint32_t ret;
    hmac_vap_stru *pst_hmac_vap = (hmac_vap_stru *)puc_para;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if ((pst_hmac_vap->uc_ps_mode == MAX_FAST_PS) || (pst_hmac_vap->uc_ps_mode == AUTO_FAST_PS)) {
        wlan_pm_set_timeout((g_wlan_min_fast_ps_idle > 1) ? (g_wlan_min_fast_ps_idle - 1) : g_wlan_min_fast_ps_idle);
    } else {
        wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);
    }
#endif

    /* 未主动dhcp成功,超时开低功耗 */
    ret = hmac_config_set_pm_by_module(&pst_hmac_vap->st_vap_base_info,
                                       MAC_STA_PM_CTRL_TYPE_HOST, MAC_STA_PM_SWITCH_ON);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                         "{hmac_set_ipaddr_timeout::hmac_config_set_pm_by_module failed[%d].}", ret);
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_sta_vap_info_syn
 * 功能描述  : dmac_offload架构下同步sta vap信息到 dmac
 * 1.日    期  : 2014年12月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_sta_vap_info_syn(mac_vap_stru *pst_mac_vap)
{
    uint32_t ret;
    mac_h2d_vap_info_stru st_mac_h2d_vap_info;

    st_mac_h2d_vap_info.us_sta_aid = pst_mac_vap->us_sta_aid;
    st_mac_h2d_vap_info.uc_uapsd_cap = pst_mac_vap->uc_uapsd_cap;
    st_mac_h2d_vap_info.bit_ap_chip_oui = pst_mac_vap->bit_ap_chip_oui;
#ifdef _PRE_WLAN_FEATURE_DEGRADE_SWITCH
    st_mac_h2d_vap_info.degradation_nss = pst_mac_vap->en_vap_degrade_trx_nss;
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    st_mac_h2d_vap_info.en_txop_ps = mac_vap_get_txopps(pst_mac_vap);
#endif /* #ifdef _PRE_WLAN_FEATURE_TXOPPS */

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_STA_VAP_INFO_SYN,
                                 sizeof(mac_h2d_vap_info_stru), (uint8_t *)(&st_mac_h2d_vap_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sta_vap_info_syn::hmac_config_sta_vap_info_syn failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_cfg_vap_h2d
 * 功能描述  : cfg vap h2d
 * 1.日    期  : 2015年6月10日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_cfg_vap_h2d(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_device_stru *pst_dev = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_cfg_vap_h2d::pst_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_dev == NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_cfg_vap_h2d::mac_res_get_dev fail. vap_id[%u]}",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    抛事件到DMAC层, 创建dmac cfg vap
    ***************************************************************************/
    ret = hmac_cfg_vap_send_event(pst_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_cfg_vap_send_event::hmac_config_send_event fail[%d].", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_sta_update_rates
 * 功能描述  : 按照指定的协议模式更新VAP速率集
 * 1.日    期  : 2014年8月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_sta_update_rates(mac_vap_stru *pst_mac_vap,
                                      mac_cfg_mode_param_stru *pst_cfg_mode, mac_bss_dscr_stru *pst_bss_dscr)
{
    uint32_t ret;
    mac_device_stru *pst_mac_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_sta_update_rates::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_sta_update_rates::vap has been deleted.}");

        return OAL_FAIL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_sta_update_rates::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_cfg_mode->en_protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(&pst_hmac_vap->st_vap_base_info, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(pst_mac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(&pst_hmac_vap->st_vap_base_info, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(pst_mac_vap, OAL_FALSE);
    }

    mac_vap_init_by_protocol(pst_mac_vap, pst_cfg_mode->en_protocol);
    pst_mac_vap->st_channel.en_band = pst_cfg_mode->en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_cfg_mode->en_bandwidth;

    mac_sta_init_bss_rates(pst_mac_vap, pst_bss_dscr);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_start_vap_event(pst_mac_vap, OAL_FALSE);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_sta_update_rates::hmac_config_send_event failed[%d].}", ret);
        mac_vap_init_by_protocol(pst_mac_vap, pst_hmac_vap->st_preset_para.en_protocol);
        pst_mac_vap->st_channel.en_band = pst_hmac_vap->st_preset_para.en_band;
        pst_mac_vap->st_channel.en_bandwidth = pst_hmac_vap->st_preset_para.en_bandwidth;
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_calc_up_and_wait_vap
 * 功能描述  : 计算处于UP或WAIT状态的vap个数
 * 1.日    期  : 2015年7月8日
 *   修改内容  : 新生成函数
 */
uint8_t hmac_calc_up_and_wait_vap(hmac_device_stru *pst_hmac_dev)
{
    mac_vap_stru *pst_vap = NULL;
    uint8_t uc_vap_idx;
    uint8_t up_ap_num = 0;
    mac_device_stru *pst_mac_device = NULL;

    if (pst_hmac_dev->pst_device_base_info == NULL) {
        return 0;
    }

    pst_mac_device = pst_hmac_dev->pst_device_base_info;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_warning_log1(0, OAM_SF_ANY, "vap is null, vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (pst_vap->en_vap_state == MAC_VAP_STATE_UP || pst_vap->en_vap_state == MAC_VAP_STATE_AP_WAIT_START) {
            up_ap_num++;
        }
    }

    return up_ap_num;
}
/* 功能描述 : rx ddr 接收数据帧 host侧hal vap与mac vap， hal device 之间的映射关系同步 */
uint32_t hmac_config_d2h_hal_vap_syn(mac_vap_stru *mac_vap, uint8_t len, uint8_t *p_param)
{
    d2h_hal_vap_info_syn_event *p_hal_vap_info = NULL;
    hal_host_device_stru       *p_hal_dev = NULL;
    hmac_vap_stru              *hmac_vap = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, 0, "hmac_config_d2h_hal_vap_syn::hmac vap null");
        return OAL_FAIL;
    }

    p_hal_vap_info = (d2h_hal_vap_info_syn_event *)p_param;
    if ((p_hal_vap_info->hal_dev_id >= WLAN_DEVICE_MAX_NUM_PER_CHIP) ||
        (p_hal_vap_info->hal_vap_id >= HAL_MAX_VAP_NUM)) {
        return OAL_FAIL;
    }

    p_hal_dev = hal_get_host_device(p_hal_vap_info->hal_dev_id);
    if (p_hal_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, 0, "hmac_config_d2h_hal_vap_syn:: hal_dev null");
        return OAL_FAIL;
    }

    if (!p_hal_vap_info->valid) {
        p_hal_dev->hal_vap_sts_info[p_hal_vap_info->hal_vap_id].hal_vap_valid = 0;
        hmac_vap->hal_dev_id = 0xff;
        hmac_vap->hal_vap_id = 0xff;
    } else {
        p_hal_dev->hal_vap_sts_info[p_hal_vap_info->hal_vap_id].hal_vap_valid = 1;
        p_hal_dev->hal_vap_sts_info[p_hal_vap_info->hal_vap_id].mac_vap_id = p_hal_vap_info->mac_vap_id;
        hmac_vap->hal_dev_id = p_hal_vap_info->hal_dev_id;
        hmac_vap->hal_vap_id = p_hal_vap_info->hal_vap_id;
        if (p_hal_vap_info->hal_dev_id == HAL_DEVICE_ID_SLAVE) {
            hal_host_intr_regs_init(HAL_DEVICE_ID_SLAVE);
#ifdef _PRE_WLAN_FEATURE_FTM
            hal_host_ftm_reg_init(HAL_DEVICE_ID_SLAVE);
#endif
        }
    }
    oam_warning_log4(mac_vap->uc_vap_id, 0, "hmac_config_d2h_hal_vap_syn::haldev[%d],hal vap[%d],mac vap[%d],valid[%d]",
        p_hal_vap_info->hal_dev_id, p_hal_vap_info->hal_vap_id,
        p_hal_vap_info->mac_vap_id, p_hal_vap_info->valid);

    return OAL_SUCC;
}

#define DOWN_VAP
/*
 * 函 数 名  : hmac_down_vap_kick_all_user
 * 功能描述  : DOWN VAP时kick所有用户
 * 1.日    期  : 2020.1.11
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_down_vap_kick_all_user(mac_device_stru *mac_device, mac_vap_stru *mac_vap)
{
    oal_dlist_head_stru *entry          = NULL;
    oal_dlist_head_stru *dlist_tmp      = NULL;
    mac_user_stru       *user_tmp       = NULL;
    hmac_user_stru      *hmac_user_tmp  = NULL;
    oal_bool_enum_uint8  is_protected;

    /* 遍历vap下所有用户, 删除用户 */
    oal_dlist_search_for_each_safe(entry, dlist_tmp, &(mac_vap->st_mac_user_list_head))
    {
        user_tmp = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
        if (user_tmp == NULL) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_user_tmp null.}");
            continue;
        }

        hmac_user_tmp = mac_res_get_hmac_user(user_tmp->us_assoc_id);
        if (hmac_user_tmp == NULL) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_hmac_user_tmp null.idx:%u}",
                user_tmp->us_assoc_id);
            continue;
        }

        /* 管理帧加密是否开启 */
        is_protected = user_tmp->st_cap_info.bit_pmf_active;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame(mac_vap, user_tmp->auc_user_mac_addr, MAC_DISAS_LV_SS, is_protected);

        /* 删除用户 */
        hmac_user_del(mac_vap, hmac_user_tmp);
    }

    /* DBDC单GO无用户场景,GO DOWN且另一路VAP正在入网，优先去关联入网状态的VAP */
    if (is_p2p_go(mac_vap)) {
        hmac_dbdc_need_kick_user(mac_vap, mac_device);
    }

    /* VAP下user链表应该为空 */
    if (OAL_FALSE == oal_dlist_is_empty(&mac_vap->st_mac_user_list_head)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_down_vap::st_mac_user_list_head is not empty.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

static void hmac_config_down_vap_ap_mode_handle(mac_vap_stru *mac_vap,
    mac_device_stru *mac_device)
{
    hmac_device_stru *hmac_dev = NULL;
    uint32_t pedding_data = 0;

#ifdef _PRE_WLAN_FEATURE_DFS
    if (mac_device->st_dfs.st_dfs_cac_timer.en_is_enabled == OAL_TRUE) {
        /* down ap，恢复硬件发送，防止影响其他vap发送 */
        hmac_cac_chan_ctrl_machw_tx(mac_vap, OAL_TRUE);
    }
    /* 取消 CAC 定时器 */
    hmac_dfs_cac_stop(mac_device, mac_vap);
    hmac_dfs_off_cac_stop(mac_device, mac_vap);
#endif
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
    if (is_legacy_vap(mac_vap)) {
        hmac_cpu_freq_upper_limit_switch(OAL_FALSE);
    }
#endif
    hmac_dev = hmac_res_get_mac_dev(mac_device->uc_device_id);
    if (hmac_dev != NULL) {
        if (hmac_dev->en_in_init_scan && hmac_dev->st_scan_mgmt.en_is_scanning) {
            hmac_config_scan_abort(mac_vap, sizeof(uint32_t), (uint8_t *)&pedding_data);
        }
        if (hmac_calc_up_and_wait_vap(hmac_dev) <= 1) {
            hmac_init_scan_cancel_timer(hmac_dev);
            hmac_dev->en_in_init_scan = OAL_FALSE;
        }
    }
}
OAL_STATIC void hmac_config_down_vap_set_vap_state(
    hmac_vap_stru *hmac_vap, mac_cfg_down_vap_param_stru *down_vap_param)
{
    mac_vap_state_enum_uint8 vap_state;

    if (g_wlan_spec_cfg->p2p_device_gc_use_one_vap && (down_vap_param->en_p2p_mode == WLAN_P2P_CL_MODE)) {
        vap_state = MAC_VAP_STATE_STA_SCAN_COMP;
    } else {
        vap_state = MAC_VAP_STATE_INIT;
    }
    hmac_fsm_change_state(hmac_vap, vap_state);
    /*
     * 上层发送GAS REQUEST帧vap进入listen，同时遇到上层netdev stop会将vap状态置为INIT，hmac_mgmt_tx_roc_comp_cb函数会
     * 恢复vap状态为en_last_vap_state，此时vap状态已为INIT，所以此处流程应正确更新last vap state,保证vap状态的正确
     */
    hmac_vap->st_vap_base_info.en_last_vap_state = vap_state;
}
OAL_STATIC void hmac_config_down_vap_in_init_state(mac_vap_stru *mac_vap, mac_cfg_down_vap_param_stru *down_vap_param)
{
    /*  判断VAP是否down，wal层通过网络设备down/up来判断,hmac层通过INIT状态判断,两种状态应该保持一致 */
    /* 设置net_device里flags标志 */
    if (oal_netdevice_flags(down_vap_param->pst_net_dev) & OAL_IFF_RUNNING) {
        oal_netdevice_flags(down_vap_param->pst_net_dev) &= (~OAL_IFF_RUNNING);
    }
    oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::vap already down.}");
}
OAL_STATIC void hmac_down_sta_clean_ssid_mib(mac_vap_stru *mac_vap)
{
    if (mac_vap->pst_mib_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::mib pointer is NULL!!}");
        return;
    }
    memset_s(mac_mib_get_DesiredSSID(mac_vap), WLAN_SSID_MAX_LEN, 0, WLAN_SSID_MAX_LEN);
}

OAL_STATIC uint32_t hmac_config_down_vap_send_event(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (hmac_config_send_event(mac_vap, WLAN_CFGID_DOWN_VAP, len, param) != OAL_SUCC) {
        oam_warning_log0(mac_vap->uc_vap_id, 0, "{hmac_config_down_vap::hmac_config_send_event fail.}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
OAL_STATIC void hmac_config_down_vap_handle(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
    mac_device_stru *mac_device)
{
     /* sta模式时 将desired ssid MIB项置空，并清空配置协议标志 */
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap->bit_sta_protocol_cfg = OAL_SWITCH_OFF;
        hmac_down_sta_clean_ssid_mib(mac_vap);
    } else if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_config_down_vap_ap_mode_handle(mac_vap, mac_device);
    }
}
/*
 * 函 数 名  : hmac_config_down_vap
 * 功能描述  : 停用vap
 * 1.日    期  : 2013年5月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_down_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_cfg_down_vap_param_stru *pst_param = (mac_cfg_down_vap_param_stru *)puc_param;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_down_vap::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (pst_param->pst_net_dev == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::pst_param->pst_net_dev is null.}");
        return OAL_SUCC;
    }

    /* 如果vap已经在down的状态，直接返回 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_INIT) {
        hmac_config_down_vap_in_init_state(pst_mac_vap, pst_param);
        return OAL_SUCC;
    }
    hmac_config_close_lp_miracast(pst_mac_vap);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_device, pst_hmac_vap))) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_down_vap::mac_device or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    oal_cancel_delayed_work_sync(&pst_hmac_vap->st_ampdu_work);
    oal_cancel_delayed_work_sync(&pst_hmac_vap->st_set_hw_work);
#endif

    /* 设置net_device里flags标志 */
    oal_netdevice_flags(pst_param->pst_net_dev) &= (~OAL_IFF_RUNNING);

    if (hmac_down_vap_kick_all_user(pst_mac_device, pst_mac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    hmac_config_down_vap_handle(pst_mac_vap, pst_hmac_vap, pst_mac_device);

    /***************************************************************************
                         抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    if (hmac_config_down_vap_send_event(pst_mac_vap, us_len, puc_param) != OAL_SUCC) {
        return OAL_FAIL;
    }
    hmac_config_down_vap_set_vap_state(pst_hmac_vap, pst_param);
    hmac_vap_down_pcie_vote(pst_mac_vap, pst_mac_device);
    if (pst_mac_vap->pst_mib_info != NULL) {
        mac_mib_set_AuthenticationMode(pst_mac_vap, WLAN_WITP_AUTH_OPEN_SYSTEM);
    }
#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_dfs_radar_wait(pst_mac_device, pst_hmac_vap);
#endif
    oam_warning_log2(pst_mac_vap->uc_vap_id, 0, "{hmac_config_down_vap:: SUCC! Now remaining %d vaps in device[%d].}",
                     pst_mac_device->uc_vap_num, pst_mac_device->uc_device_id);
    return OAL_SUCC;
}

#define DEL_VAP
/*
 * 函 数 名  : hmac_config_del_scaning_flag
 * 功能描述  : 在删除vap时将对应的扫描状态复位
 * 1.日    期  : 2019年5月14日
  */
static void hmac_config_del_scaning_flag(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device;
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
            "{hmac_config_del_scaning_flag::hmac_device is null, dev_id[%d].}",
            hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    if (hmac_device->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id == hmac_vap->st_vap_base_info.uc_vap_id) {
        /*  扫描终止时，直接清除扫描标志,无需等待devcie上报扫描结束才清除 */
        hmac_scan_set_scan_running_false_and_wake_up_waite_queue(hmac_vap, &(hmac_device->st_scan_mgmt));
    }
}

OAL_STATIC void hmac_del_vap_destroy_timer(hmac_vap_stru *pst_hmac_vap)
{
    if (pst_hmac_vap->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_mgmt_timer));
    }
    if (pst_hmac_vap->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_scan_timeout));
    }
    if (pst_hmac_vap->st_ps_sw_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_ps_sw_timer));
    }

#ifdef _PRE_WLAN_FEATURE_FTM
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        if (pst_hmac_vap->st_ftm_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_ftm_timer));
        }
    }
#endif
}
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static void hmac_del_vap_disable_hal_vap(hmac_vap_stru *hmac_vap)
{
    hal_host_device_stru *haldev = NULL;
    uint8_t loop;
    hmac_vap->hal_vap_id = 0xff;
    haldev = hal_get_host_device(hmac_vap->hal_dev_id);
    if (haldev == NULL) {
        return;
    }

    for (loop = 0; loop < HAL_MAX_VAP_NUM; loop++) {
        if (haldev->hal_vap_sts_info[loop].mac_vap_id ==
            hmac_vap->st_vap_base_info.uc_vap_id) {
            haldev->hal_vap_sts_info[loop].hal_vap_valid = 0;
            oam_warning_log2(0, OAM_SF_ANY, "{hmac_del_vap_disable_hal_vap::mac id[%d],hal id[%d].}",
                hmac_vap->st_vap_base_info.uc_vap_id, loop);
            return;
        }
    }

    oam_warning_log1(0, OAM_SF_ANY, "{hmac_del_vap_disable_hal_vap::mac id[%d] not find hal}",
        hmac_vap->st_vap_base_info.uc_vap_id);

    return;
}
#endif

uint32_t hmac_del_vap_post_process(mac_vap_stru *pst_vap)
{
    mac_chip_stru *pst_chip = NULL;
    uint8_t uc_vap_num;
    hmac_device_stru *pst_hmac_device = NULL;

    // 如果WIFI sta，wlan0 stop，下电
    pst_chip = hmac_res_get_mac_chip(pst_vap->uc_chip_id);
    if (pst_chip == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_del_vap_post_process::hmac_res_get_mac_chip id[%d] NULL",
            pst_vap->uc_chip_id);
        return OAL_FAIL;
    }

    uc_vap_num = hmac_get_chip_vap_num(pst_chip);
    if (uc_vap_num != 0) {
        return OAL_SUCC;
    }

    /*  关闭wifi,清空扫描结构体 -- start */
    pst_hmac_device = hmac_res_get_mac_dev(pst_vap->uc_device_id);
    if (oal_likely(pst_hmac_device != NULL)) {
        hmac_scan_clean_scan(&(pst_hmac_device->st_scan_mgmt));
        // wlan0下电清除sniffer所置标记
        if (pst_hmac_device->en_monitor_mode == OAL_TRUE) {
            pst_hmac_device->en_monitor_mode = OAL_FALSE;
        }
#ifdef _PRE_WLAN_FEATURE_SNIFFER
        if (pst_hmac_device->sniffer_mode != WLAN_SINFFER_OFF) {
            pst_hmac_device->sniffer_mode = WLAN_SINFFER_OFF;
        }
#endif
    } else {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_del_vap_post_process::pst_hmac_device[%d] null!}", pst_vap->uc_device_id);
    }
    /* 不论AP or STA,没有vap 后都释放host device 资源 */
    hmac_config_host_dev_exit(pst_hmac_device);
    return OAL_SUCC;
}

static void hmac_config_del_vap_deinit_features(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_stop_chba(mac_vap);
#endif
    if (mac_is_secondary_sta(&hmac_vap->st_vap_base_info) == OAL_TRUE) {
        mac_set_dual_sta_mode(OAL_FALSE);
    }

    /* can't return when dfr process! */
    if ((mac_vap->en_vap_state != MAC_VAP_STATE_INIT) && (g_st_dfr_info.bit_device_reset_process_flag != OAL_TRUE)) {
        oam_error_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::deleting vap failed. vap state is not "
                         "INIT, en_vap_state=%d,en_vap_mode=%d}", mac_vap->en_vap_state, mac_vap->en_vap_mode);
        oam_report_backtrace();
    }

    /*  */
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_vap->uc_edca_opt_flag_ap = 0;
        frw_timer_immediate_destroy_timer_m(&(hmac_vap->st_edca_opt_timer));
    } else if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap->uc_edca_opt_flag_sta = 0;
    }
#endif
}

static void hmac_config_del_cfg_vap(hmac_vap_stru *hmac_vap)
{
    /* 在注销netdevice之前先将指针赋为空 */
    oal_net_device_stru *net_device = hmac_vap->pst_net_device;
    hmac_vap->pst_net_device = NULL;
    oal_smp_mb();
    oal_net_unregister_netdev(net_device);

    mac_res_free_mac_vap(hmac_vap->st_vap_base_info.uc_vap_id);
}

static void hmac_config_del_vap_release_resource(
    mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap, uint16_t len, uint8_t *param)
{
    /* 清理所有的timer */
    hmac_del_vap_destroy_timer(hmac_vap);

    /* 删除vap时删除TCP ACK的队列 */
#ifdef _PRE_WLAN_TCP_OPT
    hmac_tcp_opt_deinit_list(hmac_vap);
#endif

    hmac_user_del_multi_user(mac_vap);

    /* 释放pmksa */
    hmac_config_flush_pmksa(mac_vap, len, param);

    hmac_config_del_scaning_flag(hmac_vap);

    mac_vap_exit(&(hmac_vap->st_vap_base_info));

    mac_res_free_mac_vap(hmac_vap->st_vap_base_info.uc_vap_id);
}
OAL_STATIC void hmac_config_del_vap_send_event(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (oal_unlikely(hmac_config_send_event(mac_vap, WLAN_CFGID_DEL_VAP, len, param) != OAL_SUCC)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::hmac_config_send_event failed}");
        // 不退出，保证Devce挂掉的情况下可以下电。
    }
}

/*
 * 函 数 名  : hmac_config_del_vap
 * 功能描述  : 删除vap
 * 1.日    期  : 2013年5月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_del_vap(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_device_stru *mac_dev = NULL;
    mac_cfg_del_vap_param_stru *del_vap_param = (mac_cfg_del_vap_param_stru *)param;

    if (oal_any_null_ptr2(mac_vap, param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_vap::mac_vap or param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::mac_res_get_hmac_vap failed.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_SAE
    oal_cancel_delayed_work_sync(&(hmac_vap->st_sae_report_ext_auth_worker));
#endif

    if (g_wlan_spec_cfg->p2p_device_gc_use_one_vap && mac_vap->en_p2p_mode == WLAN_P2P_CL_MODE) {
        return hmac_del_p2p_cl_vap(mac_vap, len, param);
    }

    hmac_config_del_vap_deinit_features(mac_vap, hmac_vap);

    /* 如果是配置VAP, 去注册配置vap对应的net_device, 释放，返回 */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        hmac_config_del_cfg_vap(hmac_vap);
        return OAL_SUCC;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_del_vap_disable_hal_vap(hmac_vap);
#endif
    /* 业务vap net_device已在WAL释放，此处置为null */
    if (del_vap_param->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* 针对p2p0,需要删除hmac 中对应的p2p0 netdevice 指针 */
        hmac_vap->pst_p2p0_net_device = NULL;
    }
    hmac_vap->pst_net_device = NULL;

    /* 业务vap已删除，从device上去掉 */
    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_vap::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    hmac_config_del_vap_release_resource(mac_vap, hmac_vap, len, param);

    /***************************************************************************
                          抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    hmac_config_del_vap_send_event(mac_vap, len, param);
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ANY,
        "{hmac_config_del_vap::Del succ.vap_mode[%d], p2p_mode[%d], multi user idx[%d], device_id[%d]}",
        mac_vap->en_vap_mode, del_vap_param->en_p2p_mode, mac_vap->us_multi_user_idx, mac_vap->uc_device_id);
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        return OAL_SUCC;
    }
#endif  // _PRE_WLAN_FEATURE_DFR
    return hmac_del_vap_post_process(mac_vap);
}

#define ADD_VAP
/*
 * 函 数 名  : hmac_normal_check_legacy_vap_num
 * 功能描述  : 添加vap时检查现有传统模式（非proxy sta）下vap num是否符合要求
 * 1.日    期  : 2014年11月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_normal_check_legacy_vap_num(mac_device_stru *pst_mac_device,
    wlan_vap_mode_enum_uint8 en_vap_mode)
{
    /* VAP个数判断 */
    if (en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        if ((pst_mac_device->uc_sta_num == g_wlan_spec_cfg->max_sta_num) &&
            (pst_mac_device->uc_vap_num == WLAN_AP_STA_COEXIST_VAP_NUM)) {
            /* AP STA共存场景，只能创建4个AP + 1个STA */
            oam_warning_log0(0, OAM_SF_CFG,
                             "{hmac_normal_check_legacy_vap_num::have created 4AP + 1STA, cannot create another AP.}");
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }

        if ((pst_mac_device->uc_vap_num - pst_mac_device->uc_sta_num) >= WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE) {
            /* 已创建的AP个数达到最大值4 */
            oam_warning_log2(0, OAM_SF_CFG,
                "{hmac_normal_check_legacy_vap_num::ap num exceeds the supported spec,vap_num[%u],sta_num[%u].}",
                pst_mac_device->uc_vap_num, pst_mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    } else if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_mac_device->uc_sta_num >= g_wlan_spec_cfg->max_sta_num) {
            /* 已创建的STA个数达到最大值 */
            oam_warning_log1(0, OAM_SF_CFG,
                             "{hmac_normal_check_legacy_vap_num::have created 2+ AP.can not create STA any more[%d].}",
                             pst_mac_device->uc_sta_num);
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_normal_check_vap_num
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2014年5月13日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_config_normal_check_vap_num(mac_device_stru *pst_mac_device,
    mac_cfg_add_vap_param_stru *pst_param)
{
    wlan_vap_mode_enum_uint8 en_vap_mode;
    wlan_p2p_mode_enum_uint8 en_p2p_mode;

    en_p2p_mode = pst_param->en_p2p_mode;
    if (en_p2p_mode != WLAN_LEGACY_VAP_MODE) {
        return hmac_check_p2p_vap_num(pst_mac_device, en_p2p_mode);
    }

    en_vap_mode = pst_param->en_vap_mode;
    return hmac_normal_check_legacy_vap_num(pst_mac_device, en_vap_mode);
}

/*
 * 函 数 名  : hmac_config_check_vap_num
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2014年5月13日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_config_check_vap_num(mac_device_stru *pst_mac_device, mac_cfg_add_vap_param_stru *pst_param)
{
    return hmac_config_normal_check_vap_num(pst_mac_device, pst_param);
}

OAL_STATIC uint32_t hmac_config_add_vap_uapsd_is_enable(mac_cfg_add_vap_param_stru *pst_param,
    hmac_vap_stru *pst_hmac_vap)
{
    switch (pst_param->en_vap_mode) {
        case WLAN_VAP_MODE_BSS_AP:
            pst_param->bit_uapsd_enable = pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_uapsd;
            break;

        case WLAN_VAP_MODE_BSS_STA:
        case WLAN_VAP_MODE_WDS:
            break;

        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_get_p2p_max_user(void)
{
    if (g_p2p_go_max_user_num != 0) {
        return g_p2p_go_max_user_num;
    }
    return g_wlan_spec_cfg->p2p_go_max_user_num;
}
/*
 * 功能描述  : 设置最大用户数
 * 1.日    期  : 2014年4月30日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_max_user(mac_vap_stru *mac_vap, uint32_t max_user_num)
{
    uint32_t p2p_max_user_num = hmac_get_p2p_max_user();
    uint32_t max_assoc_num = mac_chip_get_max_asoc_user(mac_vap->uc_chip_id);
    /* P2P GO最大用户数不能超过p2p限制，普通模式不能超过芯片最大用户数约束 */
    if ((is_p2p_go(mac_vap) && (max_user_num > p2p_max_user_num)) || (max_user_num > max_assoc_num) ||
        (max_user_num == 0)) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_max_user::the max_user_value[%d] out of range [1 - %d], set to default max_user!}",
            max_user_num, (is_p2p_go(mac_vap) ? p2p_max_user_num : max_assoc_num));
        max_user_num = is_p2p_go(mac_vap) ? p2p_max_user_num : max_assoc_num;
    }

    mac_mib_set_MaxAssocUserNums(mac_vap, (uint16_t)max_user_num);

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_max_user::chip us_user_num_max[%d], us_user_nums_max[%d].}",
                     max_assoc_num, mac_mib_get_MaxAssocUserNums(mac_vap));

    return OAL_SUCC;
}

/*
 * 功能描述  : 创建HMAC 业务VAP
 * 1.日    期  : 2012年11月21日
 *   修改内容  : 新生成函数
 */
void hmac_config_add_vap_cfg(hmac_vap_stru *pst_hmac_vap, mac_cfg_add_vap_param_stru *pst_param)
{
    pst_hmac_vap->pst_net_device = pst_param->pst_net_dev;

    /* 包括'\0' */
    memcpy_s(pst_hmac_vap->auc_name, OAL_IF_NAME_SIZE, pst_param->pst_net_dev->name, OAL_IF_NAME_SIZE);

    /* 将申请到的mac_vap空间挂到net_device ml_priv指针上去 */
    oal_net_dev_priv(pst_param->pst_net_dev) = &pst_hmac_vap->st_vap_base_info;
    /* 申请hmac组播用户 */
    hmac_user_add_multi_user(&(pst_hmac_vap->st_vap_base_info), &pst_param->us_muti_user_id,
        &pst_param->multi_user_lut_idx);
    mac_vap_set_multi_user_idx(&(pst_hmac_vap->st_vap_base_info), pst_param->us_muti_user_id);
    /* 业务vap都默认初始化在主路上，这里dfs能力先根据spec能力初始化，否则mib能力事件可能未同步导致不启动CAC */
    pst_hmac_vap->st_vap_base_info.en_radar_detector_is_supp = OAL_TRUE;
}

/*
 * 功能描述  : 检查vap组合，wlan1和cl/go不能共存。
 * 1.日    期  : 2020年6月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_config_check_vap_comb(mac_device_stru *mac_device, mac_cfg_add_vap_param_stru *add_vap_param)
{
    /* 双sta模式下，不能创建gc/go */
    if ((mac_is_dual_sta_mode() == OAL_TRUE) &&
        (add_vap_param->en_p2p_mode == WLAN_P2P_CL_MODE || add_vap_param->en_p2p_mode == WLAN_P2P_GO_MODE)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_check_vap_comb::cannot add gc/go[%d] in dual sta mode!}",
            add_vap_param->en_p2p_mode);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* 已经sta/gc/go/chba + ap场景，不能创建新vap */
    if (hmac_res_restriction_is_ap_with_other_vap() == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_check_vap_comb::cannot add vap while ap work with other vap!}");
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    if (wlan_chip_ap_p2p_device_coex_check_valid(mac_device, add_vap_param) == OAL_FALSE) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    /* 如果P2P dev已添加到最大个数，新添加的P2P DEV不允许添加 */
    if (mac_device->st_p2p_info.uc_p2p_device_num == WLAN_MAX_SERVICE_P2P_DEV_NUM &&
        add_vap_param->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_check_vap_comb::cannot add p2p dev while already add p2p dev!}");
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    return OAL_SUCC;
}

OAL_STATIC void hmac_config_send_add_vap_event_fail(mac_device_stru *mac_dev, mac_vap_stru *new_mac_vap,
    uint8_t vap_id, mac_cfg_add_vap_param_stru *param)
{
    /* 此处回退有误，需要对应mac_device_set_vap_id，做回退操作 */
    mac_device_set_vap_id(mac_dev, new_mac_vap, vap_id, param->en_vap_mode, param->en_p2p_mode, OAL_FALSE);
    hmac_user_del_multi_user(new_mac_vap);
    if (new_mac_vap->en_p2p_mode == WLAN_LEGACY_VAP_MODE) {
        mac_vap_vowifi_exit(new_mac_vap);
    }

    mac_res_free_mac_vap(vap_id);
    /* add vap 失败，需要将net_device ml_priv 设置为NULL,避免下次开wifi 不添加vap，导致打开wifi 失败 */
    oal_net_dev_priv(param->pst_net_dev) = NULL;
}

/*
 * 函 数 名  : hmac_config_add_vap
 * 功能描述  : 创建HMAC 业务VAP
 * 1.日    期  : 2012年11月21日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_add_vap(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_add_vap_param_stru *param = (mac_cfg_add_vap_param_stru *)puc_param;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    mac_device_stru *mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    mac_vap_stru *new_mac_vap = NULL;
    uint8_t vap_id;
    if (mac_dev == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = hmac_config_check_vap_comb(mac_dev, param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param->uc_dst_hal_dev_id = 0;  // 默认创建在主路

    if (g_wlan_spec_cfg->p2p_device_gc_use_one_vap && param->en_p2p_mode == WLAN_P2P_CL_MODE) {
        return hmac_add_p2p_cl_vap(mac_vap, us_len, puc_param);
    }

    /* VAP个数判断 */
    ret = hmac_config_check_vap_num(mac_dev, param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::check_vap_num failed[%d].}", ret);
        return ret;
    }

    /* 避免打开wifi的同时，dfr恢复vap，同时在netdev_priv下挂vap */
    if (oal_net_dev_priv(param->pst_net_dev) != NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::net_dev priv is not NULL, RETURN.}");
        return OAL_SUCC;
    }
    /*  */
    /* 从资源池申请hmac vap */
    ret = mac_res_alloc_hmac_vap(&vap_id, OAL_OFFSET_OF(hmac_vap_stru, st_vap_base_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::mac_res_alloc_hmac_vap failed[%d]}", ret);
        return ret;
    }
    /* 从资源池获取新申请到的hmac vap */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    new_mac_vap = &hmac_vap->st_vap_base_info;
    param->uc_vap_id = vap_id;

    /* 初始清0 */
    memset_s(hmac_vap, sizeof(hmac_vap_stru), 0, sizeof(hmac_vap_stru));

    /* 初始化HMAC VAP */
    ret = hmac_vap_init(hmac_vap, mac_dev->uc_chip_id, mac_dev->uc_device_id, vap_id, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::hmac_vap_init failed[%d].}", ret);
        if (new_mac_vap->en_p2p_mode == WLAN_LEGACY_VAP_MODE) {
            mac_vap_vowifi_exit(new_mac_vap);
        }
        /* 异常处理，释放内存 */
        mac_res_free_mac_vap(vap_id);
        return ret;
    }

    if (is_legacy_sta(new_mac_vap)) {
        /* 记录TxBASessionNumber mib值到chr全局变量中 */
        hmac_chr_set_ba_session_num(mac_mib_get_TxBASessionNumber(new_mac_vap));
    }

    /* 设置反挂的net_device指针 */
    if (param->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* p2p0 DEV 模式vap，采用pst_p2p0_net_device 成员指向对应的net_device */
        hmac_vap->pst_p2p0_net_device = param->pst_net_dev;
        mac_dev->st_p2p_info.uc_p2p0_vap_idx = new_mac_vap->uc_vap_id;
    }

    hmac_config_add_vap_cfg(hmac_vap, param);

    mac_device_set_vap_id(mac_dev, new_mac_vap, vap_id, param->en_vap_mode, param->en_p2p_mode, OAL_TRUE);

    if (hmac_config_add_vap_uapsd_is_enable(param, hmac_vap) != OAL_SUCC) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(new_mac_vap, WLAN_CFGID_ADD_VAP, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        hmac_config_send_add_vap_event_fail(mac_dev, new_mac_vap, vap_id, param);
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_add_vap::hmac_config_alloc_event fail[%d]}", ret);
        return ret;
    }
    if (is_p2p_go(new_mac_vap)) {
        hmac_config_set_max_user(new_mac_vap, hmac_get_p2p_max_user());
    }
    oam_warning_log4(vap_id, OAM_SF_ANY,
        "{hmac_config_add_vap::SUCCESS!!vap_mode[%d], p2p_mode[%d]}, multi user idx[%d], device id[%d]",
        param->en_vap_mode, param->en_p2p_mode, new_mac_vap->us_multi_user_idx, new_mac_vap->uc_device_id);

#ifdef _PRE_WLAN_FEATURE_TWT
    if (g_wlan_spec_cfg->feature_twt_is_open) {
        hmac_dbac_teardown_twt_session(mac_dev);
    }
#endif
    if (mac_is_secondary_sta(new_mac_vap) == OAL_TRUE) {
        mac_set_dual_sta_mode(OAL_TRUE);
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 启动chba supplicant时驱动就会add vap，但是chba mode是在hml start时才会置位，这里提前置位便于识别chba vap */
    if (param->chba_mode == CHBA_MODE) {
        new_mac_vap->chba_mode = CHBA_MODE;
    }
#endif
    return OAL_SUCC;
}

#define START_VAP
/*
 * 函 数 名  : hmac_config_def_chan
 * 功能描述  : 配置默认频带，信道，带宽
 * 1.日    期  : 2015年3月37日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_def_chan(mac_vap_stru *pst_mac_vap)
{
    uint8_t uc_channel;
    mac_cfg_mode_param_stru st_param;

    if (((pst_mac_vap->st_channel.en_band == WLAN_BAND_BUTT) ||
        (pst_mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_BUTT) ||
        (pst_mac_vap->en_protocol == WLAN_PROTOCOL_BUTT))
        && (!is_p2p_go(pst_mac_vap))) {
        st_param.en_band = WLAN_BAND_2G;
        st_param.en_bandwidth = WLAN_BAND_WIDTH_20M;
        st_param.en_protocol = WLAN_HT_MODE;
        hmac_config_set_mode(pst_mac_vap, sizeof(st_param), (uint8_t *)&st_param);
    }

    if ((pst_mac_vap->st_channel.uc_chan_number == 0) && (!is_p2p_go(pst_mac_vap))) {
        pst_mac_vap->st_channel.uc_chan_number = 6; /* 主20MHz信道号 6信道 */
        uc_channel = pst_mac_vap->st_channel.uc_chan_number;
        hmac_config_set_freq(pst_mac_vap, sizeof(uint32_t), &uc_channel);
    }

    return OAL_SUCC;
}

OAL_INLINE void hmac_config_set_ap_bssid_and_spectrum_mgmt(mac_vap_stru *mac_vap)
{
    /* 设置bssid */
    mac_vap_set_bssid(mac_vap, mac_mib_get_StationID(mac_vap), WLAN_MAC_ADDR_LEN);

    /* 入网优化，不同频段下的能力不一样 */
    if (mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_mib_set_SpectrumManagementRequired(mac_vap, OAL_FALSE);
    } else {
        mac_mib_set_SpectrumManagementRequired(mac_vap, OAL_TRUE);
    }
}

OAL_INLINE void hmac_config_start_vap_set_sta_state(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
                                                    mac_cfg_start_vap_param_stru *start_vap_param)
{
    if (mac_vap->en_p2p_mode == WLAN_P2P_DEV_MODE) {
        /* 启动p2p device时，vap_param的p2p模式和mac_vap的p2p模式不同 */
        /* hishare从p2p device转化为gc，若p2p device启动时置fake up状态，
           会导致不配置vap_ctrl寄存器，gc mac地址无效化 */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
    } else if (start_vap_param->en_p2p_mode != WLAN_P2P_CL_MODE ||
        (start_vap_param->en_p2p_mode == WLAN_P2P_CL_MODE && mac_vap->en_vap_state == MAC_VAP_STATE_INIT)) {
        /* p2p0和p2p-p2p0 共VAP 结构，对于p2p cl不用修改vap 状态 */
        /* ,上层hishare下，优化了不执行扫描的话，直接创建p2p cl会在init状态，此时需要置fake up */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
    }
}
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
OAL_STATIC oal_bool_enum_uint8 hmac_need_limit_cpu_freq(mac_vap_stru *mac_vap)
{
    return is_legacy_vap(mac_vap) && (mac_vap->st_channel.en_band == WLAN_BAND_5G) &&
        (mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS &&
        mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS);
}
#endif
uint32_t hmac_config_set_vap_param_by_vap_mode(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
                                               mac_cfg_start_vap_param_stru *start_vap_param)
{
    uint8_t *ssid = NULL;

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        ssid = mac_mib_get_DesiredSSID(mac_vap);
        /* P2P GO 创建后，未设置ssid 信息，设置为up 状态不需要检查ssid 参数 */
        if (OAL_STRLEN((int8_t *)ssid) == 0 && (!is_p2p_go(mac_vap))) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_vap_param_by_vap_mode::ssid length=0.}");
            return OAL_FAIL; /* 没设置SSID，则不启动VAP */
        }

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
        hmac_config_start_ap_enable_rx_listen(mac_vap);
#endif
        /* 设置AP侧状态机为 WAIT_START */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_AP_WAIT_START);

        if (is_legacy_vap(&(hmac_vap->st_vap_base_info))) {
            /* wifi: 02使用hostapd进行初始扫描，51使用驱动初始扫描 */
            hmac_config_def_chan(mac_vap);
        }

        /* 这里 en_status 等于 MAC_CHNL_AV_CHK_NOT_REQ(无需检测) 或者 MAC_CHNL_AV_CHK_COMPLETE(检测完成) */
        /* 检查协议 频段 带宽是否设置 */
        if ((mac_vap->st_channel.en_band == WLAN_BAND_BUTT) || (mac_vap->en_protocol == WLAN_PROTOCOL_BUTT) ||
            (mac_vap->st_channel.en_bandwidth == WLAN_BAND_WIDTH_BUTT)) {
            if (is_p2p_go(mac_vap)) {
                /* wpa_supplicant 会先设置vap up， 此时并未给vap 配置信道、带宽和协议模式信息，
                   wpa_supplicant 在cfg80211_start_ap 接口配置GO 信道、带宽和协议模式信息，
                   故此处如果没有设置信道、带宽和协议模式，直接返回成功，不返回失败。 */
                hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_INIT);
                oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_vap_param_by_vap_mode::set band \
                    bandwidth protocol first band[%d], bw[%d], protocol[%d]}", mac_vap->st_channel.en_band,
                    mac_vap->st_channel.en_bandwidth, mac_vap->en_protocol);
                return OAL_SUCC;
            } else {
                hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_INIT);
                oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                               "{hmac_config_set_vap_param_by_vap_mode::set band bandwidth protocol first.}");
                return OAL_FAIL;
            }
        }

        /* 检查信道号是否设置 */
        if ((mac_vap->st_channel.uc_chan_number == 0) && (!is_p2p_go(mac_vap))) {
            hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_INIT);
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_set_vap_param_by_vap_mode::set channel number first.}");
            return OAL_FAIL;
        }

        /* 设置AP的bssid和频谱管理能力 */
        hmac_config_set_ap_bssid_and_spectrum_mgmt(mac_vap);
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
        hmac_cpu_freq_upper_limit_switch(hmac_need_limit_cpu_freq(mac_vap));
#endif
        /* 设置AP侧状态机为 UP */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_UP);
    } else if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_config_start_vap_set_sta_state(mac_vap, hmac_vap, start_vap_param);
    } else {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_vap_param_by_vap_mode::Do not surport other mode[%d].}", mac_vap->en_vap_mode);
    }

    return OAL_SUCC_GO_ON;
}
/*
 * 函 数 名  : hmac_config_start_vap_event
 * 功能描述  : 抛start vap事件
 * 1.日    期  : 2015年8月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_start_vap_event(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_mgmt_rate_init_flag)
{
    uint32_t ret;
    mac_cfg_start_vap_param_stru st_start_vap_param;

    /* DMAC不使用netdev成员 */
    st_start_vap_param.pst_net_dev = NULL;
    st_start_vap_param.en_mgmt_rate_init_flag = en_mgmt_rate_init_flag;
    st_start_vap_param.uc_protocol = pst_mac_vap->en_protocol;
    st_start_vap_param.uc_band = pst_mac_vap->st_channel.en_band;
    st_start_vap_param.uc_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
    st_start_vap_param.uc_chan_number = pst_mac_vap->st_channel.uc_chan_number;
    st_start_vap_param.en_p2p_mode = pst_mac_vap->en_p2p_mode;
    ret = hmac_config_send_event(pst_mac_vap,
        WLAN_CFGID_START_VAP, sizeof(mac_cfg_start_vap_param_stru), (uint8_t *)&st_start_vap_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_start_vap_event::Start_vap failed[%d].}", ret);
    }
    return ret;
}

/*
 * 函 数 名  : hmac_config_start_vap
 * 功能描述  : hmac启用VAP
 * 1.日    期  : 2012年12月11日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_start_vap(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *param)
{
    uint32_t ret;
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_cfg_start_vap_param_stru *start_vap_param = (mac_cfg_start_vap_param_stru *)param;
#if defined(_PRE_WLAN_FEATURE_DFS)
    hmac_device_stru *hmac_device;
#endif
    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_start_vap::mac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::the vap has been deleted.}");
        return OAL_FAIL;
    }

    /* 如果已经在up状态，则返回成功 */
    if ((mac_vap->en_vap_state == MAC_VAP_STATE_UP) || (mac_vap->en_vap_state == MAC_VAP_STATE_AP_WAIT_START) ||
        (mac_vap->en_vap_state == MAC_VAP_STATE_STA_FAKE_UP)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_start_vap::state=%d, duplicate start again}", mac_vap->en_vap_state);
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_config_close_lp_miracast(mac_vap);

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    oal_init_delayed_work(&hmac_vap->st_ampdu_work, (void *)hmac_set_ampdu_worker);
    oal_spin_lock_init(&hmac_vap->st_ampdu_lock);
    oal_init_delayed_work(&hmac_vap->st_set_hw_work, (void *)hmac_set_ampdu_hw_worker);
#endif

    /* 根据vap mode设置vap 参数 */
    ret = hmac_config_set_vap_param_by_vap_mode(mac_vap, hmac_vap, start_vap_param);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }

    mac_vap_init_rates(mac_vap);
    ret = hmac_config_start_vap_event(mac_vap, start_vap_param->en_mgmt_rate_init_flag);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::send event failed[%d].}", ret);
        return ret;
    }

#if defined(_PRE_WLAN_FEATURE_DFS)
    if (is_ap(mac_vap) && !mac_device->zero_wait_flag) {
        hmac_device = hmac_res_get_mac_dev(mac_device->uc_device_id);
        hmac_dfs_try_cac(hmac_device, mac_vap);
    }
#endif

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_start_vap::host start vap ok. event to dmac. vap\
        mode[%d], p2p mode[%d] bw[%d]}", mac_vap->en_vap_mode, mac_vap->en_p2p_mode, mac_vap->st_channel.en_bandwidth);
    return OAL_SUCC;
}

#define VAP_INFO
int8_t *hmac_config_index2string(uint32_t index, int8_t *pst_string[], uint32_t max_str_nums)
{
    if (index >= max_str_nums) {
        return (int8_t *)"unkown";
    }
    return pst_string[index];
}

int8_t *hmac_config_protocol2string(uint32_t protocol)
{
    int8_t *pac_protocol2string[] = {
        "11a", "11b", "error", "11bg", "11g", "11n",
        "11ac", "11nonly", "11aconly", "11ng", "11ax", "error"
    };
    return hmac_config_index2string(protocol, pac_protocol2string, sizeof(pac_protocol2string) / sizeof(int8_t *));
}

int8_t *hmac_config_band2string(uint32_t band)
{
    int8_t *pac_band2string[] = { "2.4G", "5G", "error" };
    return hmac_config_index2string(band, pac_band2string, sizeof(pac_band2string) / sizeof(int8_t *));
}

int8_t *hmac_config_bw2string(uint32_t bw)
{
    int8_t *pac_bw2string[] = {
        "20M", "40+", "40-", "80++", "80+-", "80-+", "80--",
#ifdef _PRE_WLAN_FEATURE_160M
        "160+++", "160++-", "160+-+", "160+--", "160-++", "160-+-", "160--+", "160---",
#endif
        "error"
    };
    return hmac_config_index2string(bw, pac_bw2string, sizeof(pac_bw2string) / sizeof(int8_t *));
}

int8_t *hmac_config_ciper2string(uint32_t ciper2)
{
    int8_t *pac_ciper2string[] = { "GROUP", "WEP40", "TKIP", "RSV", "CCMP", "WEP104", "BIP", "NONE" };
    return hmac_config_index2string(ciper2, pac_ciper2string, sizeof(pac_ciper2string) / sizeof(int8_t *));
}

int8_t *hmac_config_akm2string(uint32_t akm2)
{
    int8_t *pac_akm2string[] = { "RSV", "1X", "PSK", "FT_1X", "FT_PSK", "1X_SHA256", "PSK_SHA256", "NONE" };
    return hmac_config_index2string(akm2, pac_akm2string, sizeof(pac_akm2string) / sizeof(int8_t *));
}

int8_t *hmac_config_keytype2string(uint32_t keytype)
{
    int8_t *pac_keytype2string[] = { "GTK", "PTK", "RX_GTK", "ERR" };
    return hmac_config_index2string(keytype, pac_keytype2string,
                                    sizeof(pac_keytype2string) / sizeof(int8_t *));
}

int8_t *hmac_config_cipher2string(uint32_t cipher)
{
    int8_t *pac_cipher2string[] = { "GROUP", "WEP40", "TKIP", "NO_ENCRYP", "CCMP", "WEP104", "BIP", "GROUP_DENYD" };
    return hmac_config_index2string(cipher, pac_cipher2string, sizeof(pac_cipher2string) / sizeof(int8_t *));
}

int8_t *hmac_config_smps2string(uint32_t smps)
{
    int8_t *pac_smps2string[] = { "Static", "Dynamic", "MIMO", "error" };
    return hmac_config_index2string(smps, pac_smps2string, sizeof(pac_smps2string) / sizeof(int8_t *));
}

int8_t *hmac_config_dev2string(uint32_t dev)
{
    int8_t *pac_dev2string[] = { "Close", "Open", "error" };
    return hmac_config_index2string(dev, pac_dev2string, sizeof(pac_dev2string) / sizeof(int8_t *));
}

int8_t *hmac_config_nss2string(uint32_t nss)
{
    int8_t *pac_nss2string[] = { "Single Nss", "Double Nss", "error" };
    return hmac_config_index2string(nss, pac_nss2string, sizeof(pac_nss2string) / sizeof(int8_t *));
}

int8_t *hmac_config_b_w2string(uint32_t b_w)
{
    int8_t *pac_bw2string[] = { "20M", "40M", "80M", "error" };
    return hmac_config_index2string(b_w, pac_bw2string, sizeof(pac_bw2string) / sizeof(int8_t *));
}

OAL_STATIC int32_t hmac_print_vap_basic_info(mac_vap_stru *mac_vap, mac_device_stru *mac_device,
                                             int8_t *print_buff, uint32_t string_len)
{
    int32_t string_tmp_len;

    string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
        ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
        "vap id: %u  device id: %u  chip id: %u\n"
        "vap state: %u\n"
        "vap mode: %u   P2P mode:%u\n"
        "ssid: %.32s\n"
        "hide_ssid :%u\n"
        "vap nss[%u] devicve nss[%u]\n",
        mac_vap->uc_vap_id, mac_vap->uc_device_id, mac_vap->uc_chip_id, mac_vap->en_vap_state, mac_vap->en_vap_mode,
        mac_vap->en_p2p_mode, mac_mib_get_DesiredSSID(mac_vap), mac_vap->st_cap_flag.bit_hide_ssid,
        mac_vap->en_vap_rx_nss, mac_device_get_nss_num(mac_device));
    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_protocol_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t string_len)
{
    int32_t string_tmp_len;
    mac_user_stru *mac_user = NULL;

    mac_user = mac_res_get_mac_user(mac_vap->us_assoc_vap_id);
    if ((mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) && (mac_user != NULL)) {
        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                    "protocol: sta|ap[%s|%s],\n"
                                    "user bandwidth_cap:[%u] avail_bandwidth[%u] cur_bandwidth[%u],\n"
                                    "user id[%u] user nss:[%u] user_avail_nss[%u] smpd_opmode_nss[%u]\n",
                                    hmac_config_protocol2string(mac_vap->en_protocol),
                                    hmac_config_protocol2string(mac_user->en_cur_protocol_mode),
                                    mac_user->en_bandwidth_cap, mac_user->en_avail_bandwidth,
                                    mac_user->en_cur_bandwidth, mac_vap->us_assoc_vap_id,
                                    mac_user->en_user_max_cap_nss,
                                    mac_user->en_avail_num_spatial_stream,
                                    mac_user->en_smps_opmode_notify_nss);
    } else {
        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                    "protocol: %s\n",
                                    hmac_config_protocol2string(mac_vap->en_protocol));
    }

    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_detail_info(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
                                              int8_t *print_buff, uint32_t string_len)
{
    int32_t string_tmp_len;

    string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                "band: %s  bandwidth: %s\nchannel number:%d \n"
                                "associated user number:%d/%d \nBeacon interval:%d \nvap feature info:\n"
                                "amsdu     uapsd   wpa   wpa2   wps  keepalive\n"
                                "%d         %d       %d    %d     %d     %d\n"
                                "vap cap info:\n"
                                "shpreamble  shslottime  nobeacon  shortgi   2g11ac \n"
                                "%d           %d          %d         %d         %d\n"
                                "tx power: %d \nprotect mode: %d, auth mode: %d\n"
                                "erp aging cnt: %d, ht aging cnt: %d\n"
                                "auto_protection: %d\nobss_non_erp_present: %d\nobss_non_ht_present: %d\n"
                                "rts_cts_protect_mode: %d\ntxop_protect_mode: %d\n"
                                "no_short_slot_num: %d\nno_short_preamble_num: %d\nnon_erp_num: %d\n"
                                "non_ht_num: %d\nnon_gf_num: %d\n20M_only_num: %d\n"
                                "no_40dsss_cck_num: %d\nno_lsig_txop_num: %d\n",
                                hmac_config_band2string(mac_vap->st_channel.en_band),
                                /* ; */
                                hmac_config_bw2string((mac_vap->en_protocol <= WLAN_MIXED_TWO_11G_MODE) ?
                                WLAN_BAND_WIDTH_20M : mac_vap->st_channel.en_bandwidth),
                                mac_vap->st_channel.uc_chan_number, mac_vap->us_user_nums,
                                mac_mib_get_MaxAssocUserNums(mac_vap), mac_mib_get_BeaconPeriod(mac_vap),
                                mac_mib_get_CfgAmsduTxAtive(mac_vap), mac_vap->st_cap_flag.bit_uapsd,
                                mac_vap->st_cap_flag.bit_wpa, mac_vap->st_cap_flag.bit_wpa2,
                                mac_mib_get_WPSActive(mac_vap), mac_vap->st_cap_flag.bit_keepalive,
                                mac_mib_get_ShortPreambleOptionImplemented(mac_vap),
                                mac_mib_get_ShortSlotTimeOptionImplemented(mac_vap),
                                hmac_vap->en_no_beacon, mac_mib_get_ShortGIOptionInTwentyImplemented(mac_vap),
                                mac_vap->st_cap_flag.bit_11ac2g, mac_vap->uc_tx_power,
                                mac_vap->st_protection.en_protection_mode,
                                mac_mib_get_AuthenticationMode(mac_vap),
                                mac_vap->st_protection.uc_obss_non_erp_aging_cnt,
                                mac_vap->st_protection.uc_obss_non_ht_aging_cnt,
                                mac_vap->st_protection.bit_auto_protection,
                                mac_vap->st_protection.bit_obss_non_erp_present,
                                mac_vap->st_protection.bit_obss_non_ht_present,
                                mac_vap->st_protection.bit_rts_cts_protect_mode,
                                mac_vap->st_protection.bit_lsig_txop_protect_mode,
                                mac_vap->st_protection.uc_sta_no_short_slot_num,
                                mac_vap->st_protection.uc_sta_no_short_preamble_num,
                                mac_vap->st_protection.uc_sta_non_erp_num,
                                mac_vap->st_protection.uc_sta_non_ht_num,
                                mac_vap->st_protection.uc_sta_non_gf_num,
                                mac_vap->st_protection.uc_sta_20m_only_num,
                                mac_vap->st_protection.uc_sta_no_40dsss_cck_num,
                                mac_vap->st_protection.uc_sta_no_lsig_txop_num);
    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_wpa_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t string_len)
{
    uint32_t pair_suites[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
    uint32_t akm_suites[WLAN_PAIRWISE_CIPHER_SUITES]  = { 0 };
    uint32_t group_suit;
    int32_t string_tmp_len;

    group_suit = mac_mib_get_wpa_group_suite(mac_vap);

    mac_mib_get_wpa_pair_suites_s(mac_vap, pair_suites, sizeof(pair_suites));
    mac_mib_get_wpa_akm_suites_s(mac_vap, akm_suites, sizeof(akm_suites));
    string_tmp_len = snprintf_s(print_buff + string_len,
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                "Privacy Invoked: \nRSNA-WPA \n "
                                "GRUOP     WPA PAIREWISE0[Actived]    WPA PAIRWISE1[Actived]     "
                                "AUTH1[Active]     AUTH2[Active]\n "
                                "%s        %s[%s]          %s[%s]            %s[%s]             %s[%s]\n",
                                hmac_config_ciper2string((uint8_t)group_suit),
                                hmac_config_ciper2string((uint8_t)pair_suites[0]),
                                (pair_suites[0] == 0) ? "Actived" : "Inactived",
                                hmac_config_ciper2string((uint8_t)pair_suites[1]),
                                (pair_suites[1] == 0) ? "Actived" : "Inactived",
                                hmac_config_akm2string((uint8_t)akm_suites[0]),
                                (akm_suites[0] == 0) ? "Actived" : "Inactived",
                                hmac_config_akm2string((uint8_t)akm_suites[1]),
                                (akm_suites[1] == 0) ? "Actived" : "Inactived");
    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_wpa2_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t string_len)
{
    uint32_t pair_suites[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };
    uint32_t akm_suites[WLAN_PAIRWISE_CIPHER_SUITES]  = { 0 };
    uint32_t group_suit;
    int32_t string_tmp_len;

    group_suit = mac_mib_get_rsn_group_suite(mac_vap);

    mac_mib_get_rsn_pair_suites_s(mac_vap, pair_suites, sizeof(pair_suites));
    mac_mib_get_rsn_akm_suites_s(mac_vap, akm_suites, sizeof(akm_suites));
    string_tmp_len = snprintf_s(print_buff + string_len,
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                "Privacy Invoked: \nRSNA-WPA2 \n"
                                "GRUOP     WPA2 PAIREWISE0[Actived]    WPA2 PAIRWISE1[Actived]     "
                                "AUTH1[Active]     AUTH2[Active]\n"
                                "%s        %s[%s]          %s[%s]            %s[%s]             %s[%s]\n",
                                hmac_config_ciper2string((uint8_t)group_suit),
                                hmac_config_ciper2string((uint8_t)pair_suites[0]),
                                (pair_suites[0] == 0) ? "Actived" : "Inactived",
                                hmac_config_ciper2string((uint8_t)pair_suites[1]),
                                (pair_suites[1] == 0) ? "Actived" : "Inactived",
                                hmac_config_akm2string((uint8_t)akm_suites[0]),
                                (akm_suites[0] == 0) ? "Actived" : "Inactived",
                                hmac_config_akm2string((uint8_t)akm_suites[1]),
                                (akm_suites[1] == 0) ? "Actived" : "Inactived");
    return string_tmp_len;
}

OAL_STATIC int32_t hmac_print_vap_app_ie_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t *string_len)
{
    int32_t string_tmp_len;
    uint8_t loop;
    /* APP IE 信息 */
    for (loop = 0; loop < OAL_APP_IE_NUM; loop++) {
        string_tmp_len = snprintf_s(print_buff + *string_len,
                                    ((OAM_REPORT_MAX_STRING_LEN - *string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - *string_len) - 1) - 1,
                                    "APP IE:type= %u, len = %u, max_len = %u\n",
                                    loop,
                                    mac_vap->ast_app_ie[loop].ie_len,
                                    mac_vap->ast_app_ie[loop].ie_max_len);
        if (string_tmp_len < 0) {
            return string_tmp_len;
        }
        *string_len += (uint32_t)string_tmp_len;
    }
    return string_tmp_len;
}
static uint32_t hmac_print_vap_encryption_info(mac_vap_stru *mac_vap, int8_t *print_buff, uint32_t *string_len)
{
    int32_t string_tmp_len;
    mac_user_stru *multi_user = mac_res_get_mac_user(mac_vap->us_multi_user_idx);
    if (multi_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_print_vap_encryption_info::multi_user[%d] null.}",
            mac_vap->us_multi_user_idx);
        return OAL_FAIL;
    }

    if (mac_mib_get_rsnaactivated(mac_vap) == OAL_TRUE) {
        if (mac_vap->st_cap_flag.bit_wpa == 1) {
            string_tmp_len = hmac_print_vap_wpa_info(mac_vap, print_buff, *string_len);
            if (string_tmp_len < 0) {
                return OAL_FAIL;
            }
            *string_len += (uint32_t)string_tmp_len;
        }

        if (mac_vap->st_cap_flag.bit_wpa2 == 1) {
            string_tmp_len = hmac_print_vap_wpa2_info(mac_vap, print_buff, *string_len);
            if (string_tmp_len < 0) {
                return OAL_FAIL;
            }
            *string_len += (uint32_t)string_tmp_len;
        }
    }

    string_tmp_len = snprintf_s(print_buff + *string_len, ((OAM_REPORT_MAX_STRING_LEN - *string_len) - 1),
        ((OAM_REPORT_MAX_STRING_LEN - *string_len) - 1) - 1, "MULTI_USER: cipher_type:%s, key_type:%u, sae_pwe %d \n",
        hmac_config_ciper2string(multi_user->st_key_info.en_cipher_type),
        multi_user->st_user_tx_info.st_security.en_cipher_key_type,
        mac_vap->sae_pwe);
    if (string_tmp_len < 0) {
        return OAL_FAIL;
    }
    *string_len += (uint32_t)string_tmp_len;
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_vap_info
 * 功能描述  : 打印vap参数信息
 * 1.日    期  : 2013年5月28日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_vap_info(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *mac_device = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    int8_t *print_buff = NULL;
    uint32_t string_len = 0;
    int32_t string_tmp_len;
    uint32_t ret;

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        return OAL_FAIL;
    }
    if (mac_vap->pst_mib_info == NULL) {
        return OAL_FAIL;
    }

    mac_device = (mac_device_stru *)mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (print_buff == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::print_buff null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    /* vap的基本信息 */
    string_tmp_len = hmac_print_vap_basic_info(mac_vap, mac_device, print_buff, string_len);
    if (string_tmp_len < 0) {
        goto sprint_fail;
    }
    string_len += (uint32_t)string_tmp_len;

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    if ((g_tas_switch_en[WLAN_RF_CHANNEL_ZERO] == OAL_TRUE) || (OAL_TRUE == g_tas_switch_en[WLAN_RF_CHANNEL_ONE])) {
        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
            ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1, "tas_gpio[%d]\n", board_get_wifi_tas_gpio_state());
        if (string_tmp_len < 0) {
            goto sprint_fail;
        }
        string_len += (uint32_t)string_tmp_len;
    }
#endif

#ifdef _PRE_WLAN_CHBA_MGMT
    if (mac_is_chba_mode(mac_vap) == OAL_TRUE) {
        hmac_chba_print_self_conn_info();
    }
#endif

    /* AP/STA协议模式显示 */
    string_tmp_len = hmac_print_vap_protocol_info(mac_vap, print_buff, string_len);
    if (string_tmp_len < 0) {
        goto sprint_fail;
    }
    string_len += (uint32_t)string_tmp_len;

    string_tmp_len = hmac_print_vap_detail_info(mac_vap, hmac_vap, print_buff, string_len);
    if (string_tmp_len < 0) {
        goto sprint_fail;
    }

    print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(print_buff);

    /* 上述日志量超过OAM_REPORT_MAX_STRING_LEN，分多次oam_print */
    memset_s(print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    string_len = 0;

    /* WPA/WPA2 加密参数 */
    if (mac_mib_get_privacyinvoked(mac_vap) == OAL_TRUE) {
        if (hmac_print_vap_encryption_info(mac_vap, print_buff, &string_len) != OAL_SUCC) {
            goto sprint_fail;
        }
    } else {
        string_tmp_len = snprintf_s(print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                    ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1, "Privacy NOT Invoked\n");
        if (string_tmp_len < 0) {
            goto sprint_fail;
        }
        string_len += (uint32_t)string_tmp_len;
    }

    /* APP IE 信息 */
    string_tmp_len = hmac_print_vap_app_ie_info(mac_vap, print_buff, &string_len);
    if (string_tmp_len < 0) {
        goto sprint_fail;
    }

    print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(print_buff);
    oal_mem_free_m(print_buff, OAL_TRUE);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_VAP_INFO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_vap_info::send event failed[%d].}", ret);
    }

    return ret;

sprint_fail:

    oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_vap_info:: OAL_SPRINTF return error!}");
    print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(print_buff);
    oal_mem_free_m(print_buff, OAL_TRUE);

    return OAL_FAIL;
}

static uint32_t hmac_config_get_each_user_info(mac_vap_stru *mac_vap, int8_t *sta_list_buff)
{
    oal_dlist_head_stru *dlist_head = NULL;
    mac_user_stru *mac_user = NULL;
    int8_t tmp_buff[256] = { 0 }; /* 临时存放user信息的buffer长度 256 */
    uint16_t user_idx;
    int32_t remainder_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(sta_list_buff));

    /* 同步02  */
    oal_spin_lock_bh(&mac_vap->st_cache_user_lock);

    /* AP侧的USER信息 */
    for (user_idx = 0; user_idx < MAC_VAP_USER_HASH_MAX_VALUE; user_idx++) {
        oal_dlist_search_for_each(dlist_head, &(mac_vap->ast_user_hash[user_idx]))
        {
            /* 找到相应用户 */
            mac_user = (mac_user_stru *)oal_dlist_get_entry(dlist_head, mac_user_stru, st_user_hash_dlist);
            if (mac_user == NULL) {
                oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_sta_list::mac_user null.}");
                continue;
            }
            /* 检查用户关联状态 */
            if (mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC) {
                continue;
            }
            if (snprintf_s(tmp_buff, sizeof(tmp_buff), sizeof(tmp_buff) - 1, "%02X:XX:XX:XX:%02X:%02X\n",
                mac_user->auc_user_mac_addr[MAC_ADDR_0], mac_user->auc_user_mac_addr[MAC_ADDR_4],
                mac_user->auc_user_mac_addr[MAC_ADDR_5]) < 0) {
                continue;
            };

            oal_io_print("hmac_config_get_sta_list,STA:%02X:XX:XX:XX:%02X:%02X\n",
                         mac_user->auc_user_mac_addr[MAC_ADDR_0],
                         mac_user->auc_user_mac_addr[MAC_ADDR_4], mac_user->auc_user_mac_addr[MAC_ADDR_5]);

            if (strncat_s(sta_list_buff, OAM_REPORT_MAX_STRING_LEN, tmp_buff, remainder_len - 1) != EOK) {
                oam_error_log0(0, OAM_SF_CFG, "hmac_config_get_sta_list::strncat_s error!");
                oal_mem_free_m(sta_list_buff, OAL_TRUE);
                oal_spin_unlock_bh(&mac_vap->st_cache_user_lock);
                return OAL_FAIL;
            }
            memset_s(tmp_buff, sizeof(tmp_buff), 0, sizeof(tmp_buff));
            remainder_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(sta_list_buff));
        }
    }

    oal_spin_unlock_bh(&mac_vap->st_cache_user_lock);

    return OAL_SUCC;
}

static oal_netbuf_stru *hmac_config_report_sta_list(mac_vap_stru *mac_vap, int8_t *sta_list_buff)
{
    uint32_t netbuf_len = OAL_STRLEN(sta_list_buff);
    oal_netbuf_stru *netbuf = NULL;

    if (netbuf_len == 0) {
        /* 若在获取用户列表时 去关联用户，概率性会导致获取不到user，从而导致 netbuf_len为0的情况 */
        oam_warning_log0(0, OAM_SF_CFG, "hmac_config_get_sta_list::not find suit user");
        return NULL;
    }

    netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, 0,
                         "hmac_config_get_sta_list::Alloc netbuf(size %d) NULL in normal_netbuf_pool!", netbuf_len);
        return NULL;
    }

    oal_netbuf_put(netbuf, netbuf_len);
    if (memcpy_s(oal_netbuf_data(netbuf), netbuf_len, sta_list_buff, netbuf_len) != EOK) {
        oam_error_log1(0, OAM_SF_CFG, "hmac_config_get_sta_list::memcpy fail! len[%d]", netbuf_len);
        oal_netbuf_free(netbuf);
        return NULL;
    }

    return netbuf;
}

/*
 * 函 数 名  : hmac_config_get_sta_list
 * 功能描述  : 取得关联的STA
 * 1.日    期  : 2015年5月22日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_sta_list(mac_vap_stru *mac_vap, uint16_t *len, uint8_t *param)
{
    int8_t *sta_list_buff = NULL;
    oal_netbuf_stru *netbuf = NULL;

    /* 事件传递指针值，此处异常返回前将其置为NULL */
    *(uintptr_t *)param = (uintptr_t)NULL;

    /* AP侧的信息才能打印相关信息 */
    if (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_sta_list::invalid en_vap_mode[%d].}", mac_vap->en_vap_mode);
        return OAL_FAIL;
    }

    sta_list_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (sta_list_buff == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_get_sta_list, oal_mem_alloc_m failed.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(sta_list_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    if (hmac_config_get_each_user_info(mac_vap, sta_list_buff) != OAL_SUCC) {
        return OAL_FAIL;
    }

    netbuf = hmac_config_report_sta_list(mac_vap, sta_list_buff);
    if (!netbuf) {
        oal_mem_free_m(sta_list_buff, OAL_TRUE);
        return OAL_FAIL;
    }

    *(uintptr_t *)param = (uintptr_t)netbuf;
    /* 事件传递指针，此处记录指针长度 */
    *len = (uint16_t)sizeof(oal_netbuf_stru *);

    oal_mem_free_m(sta_list_buff, OAL_TRUE);
    return OAL_SUCC;
}
OAL_STATIC void hmac_config_io_print_user_info(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user)
{
    uint8_t tid_idx;
    oal_io_print("en_user_asoc_state :  %d \n", hmac_user->st_user_base_info.en_user_asoc_state);
    oal_io_print("uc_is_wds :           %d \n", hmac_user->uc_is_wds);
    oal_io_print("us_amsdu_maxsize :    %d \n", hmac_user->amsdu_maxsize);
    oal_io_print("11ac2g :              %d \n", hmac_user->st_hmac_cap_info.bit_11ac2g);
    oal_io_print("\n");

    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        oal_io_print("tid               %d \n", tid_idx);
        oal_io_print("uc_amsdu_maxnum : %d \n", hmac_user->ast_hmac_amsdu[tid_idx].amsdu_maxnum);
        oal_io_print("us_amsdu_maxsize :%d \n", hmac_user->ast_hmac_amsdu[tid_idx].amsdu_maxsize);
        oal_io_print("us_amsdu_size :   %d \n", hmac_user->ast_hmac_amsdu[tid_idx].amsdu_size);
        oal_io_print("uc_msdu_num :     %d \n", hmac_user->ast_hmac_amsdu[tid_idx].msdu_num);
        oal_io_print("\n");
    }

    oal_io_print("us_user_hash_idx :    %d \n", hmac_user->st_user_base_info.us_user_hash_idx);
    oal_io_print("us_assoc_id :         %d \n", hmac_user->st_user_base_info.us_assoc_id);
    oal_io_print("uc_vap_id :           %d \n", hmac_user->st_user_base_info.uc_vap_id);
    oal_io_print("uc_device_id :        %d \n", hmac_user->st_user_base_info.uc_device_id);
    oal_io_print("uc_chip_id :          %d \n", hmac_user->st_user_base_info.uc_chip_id);
    oal_io_print("uc_amsdu_supported :  %d \n", hmac_user->uc_amsdu_supported);
    oal_io_print("uc_htc_support :      %d \n", hmac_user->st_user_base_info.st_ht_hdl.uc_htc_support);
    oal_io_print("en_ht_support :       %d \n", hmac_user->st_user_base_info.st_ht_hdl.en_ht_capable);
    oal_io_print("short gi 20 40 80:    %d %d %d \n", hmac_user->st_user_base_info.st_ht_hdl.bit_short_gi_20mhz,
        hmac_user->st_user_base_info.st_ht_hdl.bit_short_gi_40mhz,
        hmac_user->st_user_base_info.st_vht_hdl.bit_short_gi_80mhz);
    oal_io_print("\n");

    oal_io_print("Privacy info : \r\n");
    oal_io_print("    port_valid   :                     %d \r\n",
        hmac_user->st_user_base_info.en_port_valid);
    oal_io_print("    user_tx_info.security.cipher_key_type:      %s \r\n"
        "    user_tx_info.security.cipher_protocol_type: %s \r\n",
        hmac_config_keytype2string(hmac_user->st_user_base_info.st_user_tx_info.st_security.en_cipher_key_type),
        hmac_config_cipher2string(hmac_user->st_user_base_info.st_key_info.en_cipher_type));
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        oal_io_print("    STA:cipher_type :                           %s \r\n",
            hmac_config_cipher2string(hmac_user->st_user_base_info.st_key_info.en_cipher_type));
    }
    oal_io_print("\n");
}
/*
 * 函 数 名  : hmac_config_user_info
 * 功能描述  : 打印user信息
 * 1.日    期  : 2013年5月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_user_info(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_user_stru *hmac_user;
    uint32_t ret;
    uint8_t en_output_type = OAM_OUTPUT_TYPE_BUTT; // oam_output_type_enum
    mac_cfg_user_info_param_stru *hmac_event;

    hmac_event = (mac_cfg_user_info_param_stru *)puc_param;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(hmac_event->us_user_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_user_info::hmac_user[%d] null.}", hmac_event->us_user_idx);
        return OAL_FAIL;
    }

    oam_get_output_type(&en_output_type);
    if (en_output_type != OAM_OUTPUT_TYPE_SDT) {
        hmac_config_io_print_user_info(mac_vap, hmac_user);
    } else {
        oam_ota_report((uint8_t *)hmac_user, (uint16_t)(sizeof(hmac_user_stru) - sizeof(mac_user_stru)),
                       0, 0, OAM_OTA_TYPE_HMAC_USER);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_USER_INFO, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_user_info::send event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
