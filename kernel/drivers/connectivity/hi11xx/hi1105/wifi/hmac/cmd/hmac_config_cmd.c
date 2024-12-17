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
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_CMD_C

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_FEATURE_DDR_BUGFIX
int32_t g_ddr_qos_req_count = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) && defined(CONFIG_PLATFORM_QOS)
struct platform_qos_request *g_pst_auto_ddr_freq = NULL; /* DDR频率申请结构体 */
#else
struct pm_qos_request *g_pst_auto_ddr_freq = NULL; /* DDR频率申请结构体 */
#endif
/*
 * 函 数 名  : hmac_remove_ddr_freq_req
 * 功能描述  : 移除内核ddr调频请求
 * 1.日    期  : 2019年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_remove_ddr_freq_req(void)
{
    if (g_pst_auto_ddr_freq == NULL || g_ddr_qos_req_count <= 0) {
        return;
    }

    oal_pm_qos_remove_request(g_pst_auto_ddr_freq);
    g_ddr_qos_req_count--;
}

/*
 * 函 数 名  : hmac_add_ddr_freq_req
 * 功能描述  : 向内核发起ddr调频请求
 * 1.日    期  : 2019年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_add_ddr_freq_req(void)
{
    if (g_pst_auto_ddr_freq == NULL) {
        g_pst_auto_ddr_freq = kzalloc(sizeof(*g_pst_auto_ddr_freq), GFP_KERNEL);
        return;
    }

    if (g_pst_auto_ddr_freq != NULL && g_ddr_qos_req_count == 0) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#if defined(CONFIG_PLATFORM_QOS) && defined(CONFIG_DEVFREQ_GOV_PLATFORM_QOS)
        oal_pm_qos_add_request(g_pst_auto_ddr_freq, PLATFORM_QOS_MEMORY_LATENCY, g_ddr_freq);
#else
        oal_pm_qos_add_request(g_pst_auto_ddr_freq, 0, g_ddr_freq);
#endif
#else
#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
        oal_pm_qos_add_request(g_pst_auto_ddr_freq, PM_QOS_MEMORY_LATENCY, g_ddr_freq);
#endif
#endif
        g_ddr_qos_req_count++;
    }
}

/*
 * 函 数 名  : hmac_set_ddr
 * 功能描述  : 向内核发起ddr调频请求
 * 1.日    期  : 2019年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_set_ddr(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    if (*puc_param) {
        hmac_add_ddr_freq_req();
    } else {
        hmac_remove_ddr_freq_req();
    }
    return OAL_SUCC;
}
#endif
#endif

/*
 * 函 数 名  : hmac_config_eth_switch
 * 功能描述  : 设置以太网帧收发开关
 * 1.日    期  : 2014年5月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_eth_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_eth_switch_param_stru *pst_eth_switch_param;
    uint16_t us_user_idx = 0;
    uint32_t ret;

    pst_eth_switch_param = (mac_cfg_eth_switch_param_stru *)puc_param;

    ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_eth_switch_param->auc_user_macaddr, &us_user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_eth_switch::mac_vap_find_user_by_macaddr failed[%d].}", ret);
        return ret;
    }

    ret = oam_report_eth_frame_set_switch(us_user_idx, pst_eth_switch_param->en_switch,
        pst_eth_switch_param->en_frame_direction);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_eth_switch::oam_report_eth_frame_set_switch failed[%d].}", ret);
        return ret;
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_ETH_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_eth_switch::hmac_config_send_event fail[%d].", ret);
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_NRCOEX
/*
 * 函 数 名  : hmac_config_query_nrcoex_stat
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2019年3月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_query_nrcoex_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_QUERY_NRCOEX_STAT, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_COEX,
                         "{hmac_config_query_nrcoex_stat::send event return err code [%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_report_nrcoex_stat
 * 功能描述  : NR共存信息上报HOST
 * 1.日    期  : 2019年3月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_report_nrcoex_stat(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_device_stru *pst_hmac_device;
    mac_nrcoex_stat_stru *pst_mac_nrcoex_stat;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_COEX,
                       "hmac_config_report_nrcoex_stat: pst_hmac_device is null ptr. device id:%d",
                       pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_nrcoex_stat = (mac_nrcoex_stat_stru *)puc_param;

    if (memcpy_s(&pst_hmac_device->st_nrcoex_stat_query.st_nrcoex_stat, sizeof(mac_nrcoex_stat_stru),
        pst_mac_nrcoex_stat, uc_len) != EOK) {
        oam_error_log0(0, OAM_SF_COEX, "hmac_config_report_nrcoex_stat::memcpy fail!");
        return OAL_FAIL;
    }
    pst_hmac_device->st_nrcoex_stat_query.en_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_device->st_nrcoex_stat_query.st_wait_queue));
    return OAL_SUCC;
}
uint32_t hmac_config_nrcoex_test(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NRCOEX_TEST, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_COEX,
                         "{hmac_config_nrcoex::send event return err code [%d].}", ret);
    }

    return ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_TXBF
/*
 * 函 数 名  : hmac_config_vap_update_txbf_cap
 * 功能描述  : 更新vap中的txbf能力并同步到device
 * 1.日    期  : 2019.4.9
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_vap_close_txbf_cap(mac_vap_stru *pst_mac_vap)
{
    uint32_t ret;
    uint16_t us_len;
    uint8_t uc_rx_sts_num = 1;

    oal_bool_enum_uint8 en_txbf_rx_cap = OAL_FALSE;
    /* 当前mac device只支持bfee 不支持bfer 当前只处理bfee能力变化，bfer待开发 */
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, uc_rx_sts_num);
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, OAL_FALSE);
    pst_mac_vap->st_txbf_add_cap.bit_channel_est_cap = OAL_FALSE;
#endif
    mac_mib_set_VHTSUBeamformeeOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_VHTBeamformeeNTxSupport(pst_mac_vap, uc_rx_sts_num);

    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_TXBF, "hmac_config_vap_close_txbf_cap::close txbf.");

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    us_len = sizeof(en_txbf_rx_cap);

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_TXBF_MIB_UPDATE, us_len, &en_txbf_rx_cap);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_TXBF,
                         "{hmac_config_vap_update_txbf_cap::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif

/*
 * 函 数 名  : hmac_config_set_blacklist_mode
 * 功能描述  : 设置黑名单模式
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_blacklist_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t *pul_cfg_mode;

    pul_cfg_mode = (uint32_t *)puc_param;
    ret = hmac_blacklist_set_mode(pst_mac_vap, (uint8_t)*pul_cfg_mode);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_blacklist_set_mode fail: ret=%d; mode=%d}", ret, *pul_cfg_mode);
        return ret;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_blacklist_mode
 * 功能描述  : 读取黑名单模式
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_blacklist_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    uint32_t ret;

    ret = hmac_blacklist_get_mode(pst_mac_vap, puc_param);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_blacklist_get_mode fail: ret=%d; mode=%d}", ret, *puc_param);
        return ret;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_blacklist_add
 * 功能描述  : 增加黑名单
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_blacklist_add(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_blacklist_stru *pst_blklst;
    mac_vap_stru *pst_assoc_vap = NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    pst_blklst = (mac_blacklist_stru *)puc_param;
    en_assoc_vap = hmac_blacklist_get_assoc_ap(pst_mac_vap, pst_blklst->auc_mac_addr, &pst_assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_add(pst_assoc_vap, pst_blklst->auc_mac_addr, pst_blklst->aging_time);
    } else {
        ret = hmac_blacklist_add(pst_mac_vap, pst_blklst->auc_mac_addr, pst_blklst->aging_time);
    }
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_blacklist_add fail: ret=%d;}", ret);
        return ret;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_blacklist_add_only
 * 功能描述  : 增加黑名单
 * 1.日    期  : 2015年8月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_blacklist_add_only(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_blacklist_stru *pst_blklst;
    mac_vap_stru *pst_assoc_vap = NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    pst_blklst = (mac_blacklist_stru *)puc_param;
    en_assoc_vap = hmac_blacklist_get_assoc_ap(pst_mac_vap, pst_blklst->auc_mac_addr, &pst_assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_add_only(pst_assoc_vap, pst_blklst->auc_mac_addr, 0);
    } else {
        ret = hmac_blacklist_add_only(pst_mac_vap, pst_blklst->auc_mac_addr, 0);
    }
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_blacklist_add_only fail: ret=%d;}", ret);
        return ret;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_blacklist_del
 * 功能描述  : 删除黑名单
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_blacklist_del(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_vap_stru *pst_assoc_vap = NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    en_assoc_vap = hmac_blacklist_get_assoc_ap(pst_mac_vap, puc_param, &pst_assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_del(pst_assoc_vap, puc_param);
    } else {
        ret = hmac_blacklist_del(pst_mac_vap, puc_param);
    }
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_blacklist_del::blacklist_del fail: ret=%d;}", ret);

        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_autoblacklist_enable
 * 功能描述  : 使能自动黑名单
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_autoblacklist_enable(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint8_t uc_enable;

    uc_enable = *puc_param;

    ret = hmac_autoblacklist_enable(pst_mac_vap, uc_enable);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_autoblacklist_enable fail: ret=%d; cfg=%d}", ret, *puc_param);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_autoblacklist_aging
 * 功能描述  : 配置自动黑名单的老化时间
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_autoblacklist_aging(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t *pul_cfg_aging_time;

    pul_cfg_aging_time = (uint32_t *)puc_param;
    ret = hmac_autoblacklist_set_aging(pst_mac_vap, *pul_cfg_aging_time);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_autoblacklist_set_aging fail: ret=%d; cfg=%d}", ret, *pul_cfg_aging_time);
        return ret;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_autoblacklist_threshold
 * 功能描述  : 配置自动黑名单的门限
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_autoblacklist_threshold(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t *pul_cfg_threshold;

    pul_cfg_threshold = (uint32_t *)puc_param;
    ret = hmac_autoblacklist_set_threshold(pst_mac_vap, *pul_cfg_threshold);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_autoblacklist_set_threshold fail: ret=%d; cfg=%d}", ret, *pul_cfg_threshold);
        return ret;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_set_autoblacklist_reset_time
 * 功能描述  : 设置自动黑名单的重置时间
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_autoblacklist_reset_time(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    uint32_t *pul_cfg_reset_time;

    pul_cfg_reset_time = (uint32_t *)puc_param;
    ret = hmac_autoblacklist_set_reset_time(pst_mac_vap, *pul_cfg_reset_time);
    if (ret != OAL_SUCC) {
        oam_error_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_autoblacklist_set_reset_time fail: ret=%d; cfg=%d}", ret, *pul_cfg_reset_time);
        return ret;
    }
    return OAL_SUCC;
}
OAL_STATIC uint32_t hmac_add_pmksa(hmac_vap_stru *hmac_vap, mac_cfg_pmksa_param_stru *cfg_pmksa)
{
    hmac_pmksa_cache_stru *pmksa_cache = NULL;
    pmksa_cache = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_pmksa_cache_stru), OAL_TRUE);
    if (pmksa_cache == NULL) {
        oam_error_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
            "{hmac_add_pmksa:: oal_mem_alloc_m fail [%02X:XX:XX:XX:%02X:%02X]}",
            cfg_pmksa->auc_bssid[MAC_ADDR_0], cfg_pmksa->auc_bssid[MAC_ADDR_4], cfg_pmksa->auc_bssid[MAC_ADDR_5]);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memcpy_s(pmksa_cache->auc_bssid, OAL_MAC_ADDR_LEN, cfg_pmksa->auc_bssid, OAL_MAC_ADDR_LEN);
    memcpy_s(pmksa_cache->auc_pmkid, WLAN_PMKID_LEN, cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN);
    oal_dlist_add_head(&(pmksa_cache->st_entry), &(hmac_vap->st_pmksa_list_head));
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_set_pmksa
 * 功能描述  : 增加一条pmk缓存记录
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_pmksa(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_pmksa_param_stru *cfg_pmksa = NULL;
    hmac_pmksa_cache_stru *p_pmksa_cache = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    oal_dlist_head_stru *p_pmksa_entry = NULL;
    oal_dlist_head_stru *p_pmksa_entry_tmp = NULL;
    uint32_t pmksa_cnt = 0;

    if (oal_any_null_ptr2(mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_pmksa param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_pmksa = (mac_cfg_pmksa_param_stru *)puc_param;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_pmksa::hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_dlist_is_empty(&(hmac_vap->st_pmksa_list_head))) {
        oal_dlist_init_head(&(hmac_vap->st_pmksa_list_head));
    }

    oal_dlist_search_for_each_safe(p_pmksa_entry, p_pmksa_entry_tmp, &(hmac_vap->st_pmksa_list_head))
    {
        p_pmksa_cache = oal_dlist_get_entry(p_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        /* 已存在时，先删除，保证最新的pmk在dlist头部 */
        if (oal_compare_mac_addr(cfg_pmksa->auc_bssid, p_pmksa_cache->auc_bssid) == 0) {
            oal_dlist_delete_entry(p_pmksa_entry);
            oal_mem_free_m(p_pmksa_cache, OAL_TRUE);
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_set_pmksa:: DEL first [%02X:XX:XX:XX:%02X:%02X]}",
                cfg_pmksa->auc_bssid[MAC_ADDR_0], cfg_pmksa->auc_bssid[MAC_ADDR_4], cfg_pmksa->auc_bssid[MAC_ADDR_5]);
        }
        pmksa_cnt++;
    }

    if (pmksa_cnt > WLAN_PMKID_CACHE_SIZE) {
        /* 超过最大个数时，先队列尾，保证最新的pmk在dlist头部 */
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_pmksa:: can't store more pmksa for [%02X:XX:XX:XX:%02X:%02X]}",
            cfg_pmksa->auc_bssid[MAC_ADDR_0], cfg_pmksa->auc_bssid[MAC_ADDR_4], cfg_pmksa->auc_bssid[MAC_ADDR_5]);
        p_pmksa_entry = oal_dlist_delete_tail(&(hmac_vap->st_pmksa_list_head));
        p_pmksa_cache = oal_dlist_get_entry(p_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        oal_mem_free_m(p_pmksa_cache, OAL_TRUE);
    }
    if (hmac_add_pmksa(hmac_vap, cfg_pmksa) != OAL_SUCC) {
        return OAL_FAIL;
    }
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_set_pmksa:: SET pmksa for [%02X:XX:XX:XX:%02X:%02X] OK!}",
        cfg_pmksa->auc_bssid[MAC_ADDR_0], cfg_pmksa->auc_bssid[MAC_ADDR_4], cfg_pmksa->auc_bssid[MAC_ADDR_5]);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_del_pmksa
 * 功能描述  : 删除一条pmk缓存记录
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_del_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa = NULL;
    hmac_pmksa_cache_stru *pst_pmksa_cache = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_dlist_head_stru *pst_pmksa_entry = NULL;
    oal_dlist_head_stru *pst_pmksa_entry_tmp = NULL;

    if (oal_any_null_ptr2(pst_mac_vap, puc_param)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_pmksa param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)puc_param;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_del_pmksa::pst_hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_dlist_is_empty(&(pst_hmac_vap->st_pmksa_list_head))) {
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_del_pmksa:: pmksa dlist is null [%02X:XX:XX:XX:%02X:%02X]}",
                         pst_cfg_pmksa->auc_bssid[MAC_ADDR_0],
                         pst_cfg_pmksa->auc_bssid[MAC_ADDR_4],
                         pst_cfg_pmksa->auc_bssid[MAC_ADDR_5]);
    }

    oal_dlist_search_for_each_safe(pst_pmksa_entry, pst_pmksa_entry_tmp, &(pst_hmac_vap->st_pmksa_list_head))
    {
        pst_pmksa_cache = oal_dlist_get_entry(pst_pmksa_entry, hmac_pmksa_cache_stru, st_entry);
        if (oal_compare_mac_addr(pst_cfg_pmksa->auc_bssid, pst_pmksa_cache->auc_bssid) == 0) {
            oal_dlist_delete_entry(pst_pmksa_entry);
            oal_mem_free_m(pst_pmksa_cache, OAL_TRUE);
            oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_del_pmksa:: DEL pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                             pst_cfg_pmksa->auc_bssid[MAC_ADDR_0],
                             pst_cfg_pmksa->auc_bssid[MAC_ADDR_4],
                             pst_cfg_pmksa->auc_bssid[MAC_ADDR_5]);
            return OAL_SUCC;
        }
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_del_pmksa:: NO pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                     pst_cfg_pmksa->auc_bssid[MAC_ADDR_0],
                     pst_cfg_pmksa->auc_bssid[MAC_ADDR_4],
                     pst_cfg_pmksa->auc_bssid[MAC_ADDR_5]);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_flush_pmksa
 * 功能描述  : 清除pmk缓存记录
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_flush_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_pmksa_cache_stru *pst_pmksa_cache = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_dlist_head_stru *pst_pmksa_entry = NULL;
    oal_dlist_head_stru *pst_pmksa_entry_tmp = NULL;

    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_flush_pmksa param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_flush_pmksa::pst_hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_dlist_is_empty(&(pst_hmac_vap->st_pmksa_list_head))) {
        return OAL_SUCC;
    }

    oal_dlist_search_for_each_safe(pst_pmksa_entry, pst_pmksa_entry_tmp, &(pst_hmac_vap->st_pmksa_list_head))
    {
        pst_pmksa_cache = oal_dlist_get_entry(pst_pmksa_entry, hmac_pmksa_cache_stru, st_entry);

        oal_dlist_delete_entry(pst_pmksa_entry);
        oal_mem_free_m(pst_pmksa_cache, OAL_TRUE);
        oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_flush_pmksa:: DEL pmksa of [%02X:XX:XX:XX:%02X:%02X]}",
                         pst_pmksa_cache->auc_bssid[MAC_ADDR_0],
                         pst_pmksa_cache->auc_bssid[MAC_ADDR_4],
                         pst_pmksa_cache->auc_bssid[MAC_ADDR_5]);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_scan_abort
 * 功能描述  : 扫描终止
 * 1.日    期  : 2015年6月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_scan_abort(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_device_stru *pst_hmac_device;
    uint32_t ret;

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::pst_hmac_device is null, dev_id[%d].}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* BEGIN: mp12 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::pst_hmac_vap is null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_config_scan_abort::scan abort,curr_scan_vap_id:%d vap state: %d.}",
                     pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.uc_vap_id,
                     pst_hmac_vap->st_vap_base_info.en_vap_state);

    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if (pst_hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* 改变vap状态到SCAN_COMP */
            hmac_fsm_change_state(pst_hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }

        /*  如果处于监听状态，也需要提前置为恢复到scan comp状态，依赖dmac abort重新上报恢复vap状态会太晚，这里提前置状态保证入网 */
        if (pst_hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
            hmac_p2p_listen_timeout(pst_hmac_vap, &pst_hmac_vap->st_vap_base_info);
        }
    }

    if ((pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP)
        && (pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::en_vap_last_state:%d}",
                         pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state);
        hmac_fsm_change_state(pst_hmac_vap, pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state);
        pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* END: mp12 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    /* 删除HMAC 扫描超时定时器 */
    if (pst_hmac_device->st_scan_mgmt.st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_device->st_scan_mgmt.st_scan_timeout));
    }

    /***************************************************************************
                         抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SCAN_ABORT, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_scan_abort::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#ifdef _PRE_CONFIG_HISIFASTPOWER_DOWN_STATE
/*
 * 功能描述  : PC S5关机，通知wifi业务侧扫描终止
 * 1.日    期  : 2020年6月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_force_scan_complete_for_power_down(oal_net_device_stru *net_dev, hmac_vap_stru *hmac_vap,
                                                    hmac_device_stru *hmac_device, mac_vap_stru *mac_vap)
{
    hmac_scan_stru *scan_mgmt = NULL;
    uint32_t pedding_data = 0;

    scan_mgmt = &(hmac_device->st_scan_mgmt);

    /* 如果是来自内部的扫描 */
    if (scan_mgmt->pst_request == NULL) {
        /* 判断是否存在内部扫描，如果存在，也需要停止 */
        if ((scan_mgmt->en_is_scanning == OAL_TRUE) &&
            (mac_vap->uc_vap_id == scan_mgmt->st_scan_record_mgmt.uc_vap_id)) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_force_complete_for_power_down:stop scan!}");
            /* 终止扫描 */
            hmac_config_scan_abort(&hmac_vap->st_vap_base_info, sizeof(uint32_t), (uint8_t *)&pedding_data);
        }

        return;
    }

    /* 如果是上层下发的扫描请求，停止本netdev 发起的扫描 */
    if ((scan_mgmt->pst_request != NULL) && oal_wdev_match(net_dev, scan_mgmt->pst_request)) {
        /* 删除等待扫描超时定时器 */
        if (hmac_vap->st_scan_timeout.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(hmac_vap->st_scan_timeout));
        }

        scan_mgmt->pst_request = NULL;
        scan_mgmt->en_complete = OAL_TRUE;

        /* 下发device终止扫描 */
        hmac_config_scan_abort(&hmac_vap->st_vap_base_info, sizeof(uint32_t), (uint8_t *)&pedding_data);

        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_force_complete_for_power_down:scan abort!}");
    }

    return;
}

/*
 * 功能描述  : PC S5关机,终止wifi侧正在执行的业务
 * 1.日    期  : 2020年6月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_change_wifi_state_for_power_down(void)
{
    oal_net_device_stru *net_dev = NULL;
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;

    net_dev = oal_dev_get_by_name("wlan0");
    if (net_dev == NULL) {
        return;
    }
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        return;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_change_wifi_state_for_power_down:hmac_device[%d] is null!}",
                         mac_vap->uc_device_id);
        return;
    }

    /* 获取hmac vap */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_config_change_wifi_state_for_power_down:hmac_vap is null, vap_id[%d]!}",
                         mac_vap->uc_vap_id);
        return;
    }

    /* 如果wifi业务侧正在进行扫描，则扫描终止 */
    hmac_config_force_scan_complete_for_power_down(net_dev, hmac_vap, hmac_device, mac_vap);

    /* 如果wifi业务侧正在进行关联，删除关联定时器 */
    if (hmac_vap->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(hmac_vap->st_mgmt_timer));
    }

    return;
}
#endif
/*
 * 函 数 名  : hmac_config_get_dbdc_info
 * 功能描述  : 同步dmac 的dbdc信息
 * 1.日    期  : 2018年8月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_dbdc_info(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_get_dbdc_info::device id [%d] is null", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device->en_dbdc_running = (oal_bool_enum_uint8)*puc_param;
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "hmac_config_get_dbdc_info::dbdc state[%d]", pst_mac_device->en_dbdc_running);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
/*
 * 函 数 名  : hmac_ant_tas_switch_rssi_notify_event_status
 * 功能描述  : TAS ANT0 RSSI测量通知host
 * 1.日    期  : 2018年7月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ant_tas_switch_rssi_notify_event_status(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;
    uint32_t ret;

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (puc_param == NULL)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_ant_tas_switch_rssi_notify_event_status::pst_mac_vap->uc_init_flag[%d], or puc_param null!}",
            pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛RSSI测量完成事件到WAL */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_tas_rssi_notify_stru));
    if (pst_event_mem == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_ant_tas_switch_rssi_notify_event_status::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX,
        HMAC_HOST_CTX_EVENT_SUB_TYPE_TAS_NOTIFY_RSSI, sizeof(dmac_tas_rssi_notify_stru),
        FRW_EVENT_PIPELINE_STAGE_0, pst_mac_vap->uc_chip_id, pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    if (memcpy_s((uint8_t *)frw_get_event_payload(pst_event_mem), sizeof(dmac_tas_rssi_notify_stru),
        puc_param, uc_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_ant_tas_switch_rssi_notify_event_status::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{hmac_ant_tas_switch_rssi_notify_event_status::frw_event_dispatch_event fail.}");
    }

    frw_event_free_m(pst_event_mem);

    return ret;
}
#endif


/*
 * 函 数 名  : hmac_config_stop_altx
 * 功能描述  : 停止 常发
 * 1.日    期  : 2016年4月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_stop_altx(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    pst_mac_vap->bit_al_tx_flag = OAL_SWITCH_OFF;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_NAN
/*
 * 功能描述  : NAN response上报
 * 1.日    期  : 2020年4月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_nan_response(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;

    event_mem = frw_event_alloc_m(sizeof(mac_nan_rsp_msg_stru));
    if (event_mem == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_nan_response::frw_event_alloc_m fail!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_NAN_RSP,
                       sizeof(mac_nan_rsp_msg_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       mac_vap->uc_chip_id,
                       mac_vap->uc_device_id,
                       mac_vap->uc_vap_id);

    if (memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(mac_nan_rsp_msg_stru),
        param, len) != EOK) {
        oam_error_log0(0, OAM_SF_CHAN, "hmac_config_nan_response::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }
    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
/*
 * 函 数 名  : hmac_config_set_qos_map
 * 功能描述  : 设置QoSMap参数
 * 1.日    期  : 2015年9月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_qos_map(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint8_t uc_idx;
    hmac_cfg_qos_map_param_stru *pst_qos_map;
    hmac_vap_stru *pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    if (oal_any_null_ptr2(pst_hmac_vap, puc_param)) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_config_set_qos_map:: pointer is null: pst_hmac_vap[%d],puc_param[%d]",
                       pst_hmac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_qos_map = (hmac_cfg_qos_map_param_stru *)puc_param;
    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_qos_map:uc_num_dscp_except=%d, uc_valid=%d}",
                  pst_qos_map->uc_num_dscp_except,
                  pst_qos_map->uc_valid);

    /* 判断QOS MAP SET的使能开关是否打开 */
    if (!pst_qos_map->uc_valid) {
        return OAL_FAIL;
    }

    /* 检查下发的QoS Map Set参数中的DSCP Exception fields 是否超过最大数目21 */
    if (pst_qos_map->uc_num_dscp_except > MAX_DSCP_EXCEPT) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_config_set_qos_map:: input exceeds maximum : pst_qos_map->num_dscp_except[%d]",
                       pst_qos_map->uc_num_dscp_except);
        return OAL_FAIL;
    }
    /* 判断DSCP Exception fields是否为空 */
    if ((pst_qos_map->uc_num_dscp_except != 0)) {
        pst_hmac_vap->st_cfg_qos_map_param.uc_num_dscp_except = pst_qos_map->uc_num_dscp_except;
        for (uc_idx = 0; uc_idx < pst_qos_map->uc_num_dscp_except; uc_idx++) {
            pst_hmac_vap->st_cfg_qos_map_param.auc_dscp_exception[uc_idx] = pst_qos_map->auc_dscp_exception[uc_idx];
            pst_hmac_vap->st_cfg_qos_map_param.auc_dscp_exception_up[uc_idx] =
                pst_qos_map->auc_dscp_exception_up[uc_idx];
        }
    }

    /* 配置DSCP Exception format中的User Priority的HIGHT和LOW VALUE值 */
    for (uc_idx = 0; uc_idx < MAX_QOS_UP_RANGE; uc_idx++) {
        pst_hmac_vap->st_cfg_qos_map_param.auc_up_high[uc_idx] = pst_qos_map->auc_up_high[uc_idx];
        pst_hmac_vap->st_cfg_qos_map_param.auc_up_low[uc_idx] = pst_qos_map->auc_up_low[uc_idx];
    }
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_HS20
/*
 * 函 数 名  : hmac_config_enable_2040bss
 * 功能描述  : 20/40 bss检测开关
 * 1.日    期  : 2015年8月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_enable_2040bss(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = NULL;
    oal_bool_enum_uint8 en_2040bss_switch;
    uint8_t uc_vap_idx;
    mac_vap_stru *pst_vap = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_enable_2040bss::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_enable_2040bss:: pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_2040bss_switch = (*puc_param == 0) ? OAL_FALSE : OAL_TRUE;
    // 同步device下所有vap的mib 2040特性的配置开关
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (pst_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "{dmac_config_enable_2040bss::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
        mac_mib_set_2040SwitchProhibited(pst_vap, ((en_2040bss_switch == OAL_TRUE) ? OAL_FALSE : OAL_TRUE));
    }
    mac_set_2040bss_switch(pst_mac_device, en_2040bss_switch);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_enable_2040bss:: set 2040bss switch[%d]}", en_2040bss_switch);

    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_2040BSS_ENABLE, us_len, puc_param);
}

/*
 * 函 数 名  : hmac_config_get_2040bss_sw
 * 功能描述  : 获取20/40 bss检测开关状态
 * 1.日    期  : 2017年6月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_2040bss_sw(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_get_2040bss_sw::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_2040bss_sw:: pst_mac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pus_len = sizeof(uint32_t);
    *((uint32_t *)puc_param) = (uint32_t)mac_get_2040bss_switch(pst_mac_device);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_dieid_rsp
 * 功能描述  : 获取dieid
 * 1.日    期  : 2017年08月04日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_dieid_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    mac_cfg_show_dieid_stru *pst_dieid = NULL;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_get_polynomial_params::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dieid = (mac_cfg_show_dieid_stru *)(puc_param);

    if (memcpy_s(&pst_hmac_vap->st_dieid, sizeof(mac_cfg_show_dieid_stru), pst_dieid, uc_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_config_get_dieid_rsp::memcpy fail!");
        return OAL_FAIL;
    }

    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_dieid
 * 功能描述  : 获取dieid
 * 1.日    期  : 2017年08月04日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_dieid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_DIEID, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_cali_power:hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_set_vendor_ie
 * 功能描述  : AP 设置okc 信息元素
 * 1.日    期  : 2016年1月19日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_vendor_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    oal_app_ie_stru *pst_okc_ie;
    uint32_t ret;

    pst_okc_ie = (oal_app_ie_stru *)puc_param;

    /* 设置WPS 信息 */
    ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, pst_okc_ie, pst_okc_ie->en_app_ie_type);
    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "hmac_config_set_vendor_ie::vap_id=%d, ie_type=%d, ie_length=%d",
                     pst_mac_vap->uc_vap_id, pst_okc_ie->en_app_ie_type, pst_okc_ie->ie_len);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_vendor_ie::ret=[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_sta_11h_abillty
 * 功能描述  : 获取STA 11v能力信息
 * 1.日    期  : 2017年6月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_sta_11h_abillty(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    oal_hilink_get_sta_11h_ability *pst_sta_11h_abillty = NULL;
    hmac_vap_stru *pst_hmac_vap;
    oal_dlist_head_stru *pst_entry = NULL;
    oal_dlist_head_stru *pst_dlist_tmp = NULL;
    mac_user_stru *pst_user_tmp = NULL;
    uintptr_t temp;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_device_id, OAM_SF_CFG, "{hmac_config_get_sta_11h_abillty::hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (puc_param == NULL) {
        oam_warning_log0(pst_mac_vap->uc_device_id, OAM_SF_CFG, "{hmac_config_get_sta_11h_abillty::puc_param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (us_len == sizeof(oal_hilink_get_sta_11h_ability) + sizeof(temp)) {
        temp = *((uintptr_t *)(puc_param + sizeof(oal_hilink_get_sta_11h_ability)));
        pst_sta_11h_abillty = (oal_hilink_get_sta_11h_ability *)(uintptr_t)(temp);
    } else {
        pst_sta_11h_abillty = (oal_hilink_get_sta_11h_ability *)puc_param;
    }

    oal_dlist_search_for_each_safe(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        if (pst_user_tmp == NULL) {
            oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_get_sta_11h_abillty:: pst_user_tmp NULL !!!}");
            continue;
        }

        if (oal_memcmp(pst_user_tmp->auc_user_mac_addr, pst_sta_11h_abillty->auc_sta_mac, WLAN_MAC_ADDR_LEN) == 0) {
            if (pst_user_tmp->st_cap_info.bit_spectrum_mgmt == 1) {
                pst_sta_11h_abillty->en_support_11h = OAL_TRUE;
            }
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}

static uint32_t hmac_mlme_set_app_ie_to_vap(mac_vap_stru *mac_vap, oal_w2h_mlme_ie_stru *mlme_ie)
{
    uint32_t ret;
    oal_app_ie_stru app_ie;

    app_ie.en_app_ie_type = OAL_APP_ASSOC_RSP_IE;
    app_ie.ie_len = mlme_ie->us_ie_len;
    if (memcpy_s(app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, mlme_ie->puc_data_ie, app_ie.ie_len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_mlme_set_app_ie_to_vap::memcpy fail!");
        return OAL_FAIL;
    }

    ret = hmac_config_set_app_ie_to_vap(mac_vap, &app_ie, app_ie.en_app_ie_type);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_mlme_set_app_ie_to_vap:: hmac_config_set_app_ie_to_vap fail, err_code=%d.}", ret);
    }
    return ret;
}

static uint32_t hmac_mlme_set_frm_type(oal_w2h_mlme_ie_stru *mlme_ie)
{
    uint8_t mgmt_frm_type = WLAN_FC0_SUBTYPE_ASSOC_REQ;

    if (mlme_ie->en_mlme_type == OAL_IEEE80211_MLME_ASSOC_RSP) {
        mgmt_frm_type = WLAN_FC0_SUBTYPE_ASSOC_REQ;
    } else if (mlme_ie->en_mlme_type == OAL_IEEE80211_MLME_REASSOC_RSP) {
        mgmt_frm_type = WLAN_FC0_SUBTYPE_REASSOC_REQ;
    }
    return mgmt_frm_type;
}

/*
 * 函 数 名  : hmac_config_set_mlme
 * 功能描述  : hostapd下发的ft md rsn ie
 * 1.日    期  : 2017年2月18日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_mlme(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint16_t user_idx = 0;
    uint8_t mgmt_frm_type;
    mac_status_code_enum_uint16 en_status_code;
    oal_w2h_mlme_ie_stru *mlme_ie = (oal_w2h_mlme_ie_stru *)puc_param;
    if ((mlme_ie->en_mlme_type != OAL_IEEE80211_MLME_ASSOC_RSP) &&
        (mlme_ie->en_mlme_type != OAL_IEEE80211_MLME_REASSOC_RSP)) {
        return ret;
    }

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mlme::hmac assoc rsp ie type=%d, status=%d, \
        ie length[%d].}", mlme_ie->en_mlme_type, mlme_ie->us_status, mlme_ie->us_ie_len);
    if (mlme_ie->us_ie_len > WLAN_WPS_IE_MAX_SIZE) {
        return OAL_SUCC;
    }

    ret = mac_vap_find_user_by_macaddr(mac_vap, mlme_ie->auc_macaddr, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ASSOC,
            "{hmac_config_set_mlme::failed find user:%02X:XX:XX:XX:%02X:%02X.}", mlme_ie->auc_macaddr[MAC_ADDR_0],
            mlme_ie->auc_macaddr[MAC_ADDR_4], mlme_ie->auc_macaddr[MAC_ADDR_5]);
        hmac_mgmt_send_deauth_frame(mac_vap, mlme_ie->auc_macaddr, MAC_ASOC_NOT_AUTH, OAL_FALSE);
        return ret;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(user_idx);
    if (hmac_user == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_config_set_mlme::hmac_user[%d] null.}", user_idx);
        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame(mac_vap, mlme_ie->auc_macaddr, MAC_ASOC_NOT_AUTH, OAL_FALSE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mgmt_frm_type = hmac_mlme_set_frm_type(mlme_ie);
    if (hmac_mlme_set_app_ie_to_vap(mac_vap, mlme_ie) != OAL_SUCC) {
        return OAL_FAIL;
    }
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_ASSOC,
        "{hmac_config_set_mlme::find user:%02X:XX:XX:XX:%02X:%02X, user_idx=%d.}", mlme_ie->auc_macaddr[MAC_ADDR_0],
        mlme_ie->auc_macaddr[MAC_ADDR_4], mlme_ie->auc_macaddr[MAC_ADDR_5], user_idx);

    en_status_code = mlme_ie->us_status;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_mlme::hmac_vap is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = hmac_ap_send_assoc_rsp(hmac_vap, hmac_user, mlme_ie->auc_macaddr, mgmt_frm_type, &en_status_code);
    if ((ret != OAL_SUCC) || (en_status_code != MAC_SUCCESSFUL_STATUSCODE)) {
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_vendor_cmd_get_channel_list
 * 功能描述  : hmac读全部信道列表
 * 1.日    期  : 2016年5月31日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_vendor_cmd_get_channel_list(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    uint8_t uc_chan_idx;
    uint8_t uc_chan_num;
    uint8_t uc_chan_number;
    uint8_t *puc_channel_list = NULL;
    mac_vendor_cmd_channel_list_stru *pst_channel_list = NULL;
    uint32_t ret;

    if (oal_any_null_ptr2(pus_len, puc_param)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_config_vendor_cmd_get_channel_list::len or param is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_channel_list = (mac_vendor_cmd_channel_list_stru *)puc_param;
    *pus_len = sizeof(mac_vendor_cmd_channel_list_stru);

    /* 获取2G 信道列表 */
    uc_chan_num = 0;
    puc_channel_list = pst_channel_list->auc_channel_list_2g;

    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_chan_idx++) {
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, uc_chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, uc_chan_idx, OAL_FALSE, &uc_chan_number);
            puc_channel_list[uc_chan_num++] = uc_chan_number;
        }
    }
    pst_channel_list->uc_channel_num_2g = uc_chan_num;

    /* 检查定制化5g开关是否使能 */
    if (mac_device_band_is_support(pst_mac_vap->uc_device_id, MAC_DEVICE_CAP_5G) == OAL_FALSE) {
        pst_channel_list->uc_channel_num_5g = 0;
        return OAL_SUCC;
    }

    /* 获取5G 信道列表 */
    uc_chan_num = 0;
    puc_channel_list = pst_channel_list->auc_channel_list_5g;

    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_chan_idx++) {
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, uc_chan_idx, OAL_FALSE, &uc_chan_number);
            puc_channel_list[uc_chan_num++] = uc_chan_number;
        }
    }
    pst_channel_list->uc_channel_num_5g = uc_chan_num;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_APF
/*
 * 函 数 名  : hmac_print_apf_program
 * 功能描述  : 打印apf program到sdt
 * 1.日    期  : 2018年3月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_print_apf_program(uint8_t *puc_program, uint32_t program_len)
{
    uint32_t idx, string_len;
    int32_t l_string_tmp_len;
    uint8_t *pc_print_buff;

    pc_print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_print_apf_program::pc_print_buff null.}");
        return;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
    string_len = 0;
    l_string_tmp_len = snprintf_s(pc_print_buff + string_len, (OAM_REPORT_MAX_STRING_LEN - string_len) - 1,
                                  ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1,
                                  "Id           :200\n"
                                  "Program len  :%u\n", program_len);
    if (l_string_tmp_len < 0) {
        oam_error_log1(0, OAM_SF_CFG, "hmac_print_apf_program:sprintf return error[%d]", l_string_tmp_len);
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return;
    }
    string_len += (uint32_t)l_string_tmp_len;

    for (idx = 0; idx < program_len; idx++) {
        l_string_tmp_len = snprintf_s(pc_print_buff + string_len, ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1),
                                      ((OAM_REPORT_MAX_STRING_LEN - string_len) - 1) - 1, "%02x", puc_program[idx]);
        if (l_string_tmp_len < 0) {
            break;
        }
        string_len += (uint32_t)l_string_tmp_len;
    }

    pc_print_buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
}
OAL_STATIC uint32_t hmac_send_apf_filter_cmd_event(mac_vap_stru *mac_vap, oal_netbuf_stru *cmd_netbuf)
{
    uint32_t ret;
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *hmac_to_dmac_ctx_event = NULL;
    dmac_tx_event_stru *tx_event = NULL;
    /***************************************************************************
      抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_send_apf_filter_cmd_event::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_to_dmac_ctx_event = (frw_event_stru *)event_mem->puc_data;
    frw_event_hdr_init(&(hmac_to_dmac_ctx_event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
        DMAC_WLAN_CTX_EVENT_SUB_TYPE_APF_CMD, sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1,
        mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);

    tx_event = (dmac_tx_event_stru *)(hmac_to_dmac_ctx_event->auc_event_data);
    tx_event->pst_netbuf = cmd_netbuf;
    tx_event->us_frame_len = oal_netbuf_len(cmd_netbuf);

    ret = frw_event_dispatch_event(event_mem);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_send_apf_filter_cmd_event:dispatch_event fail[%d]!", ret);
    }
    frw_event_free_m(event_mem);
    return ret;
}
/*
 * 函 数 名  : hmac_config_apf_filter_cmd
 * 功能描述  : 处理apf下发或上报program的命令
 * 1.日    期  : 2018年3月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_apf_filter_cmd(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    oal_netbuf_stru *cmd_netbuf = NULL;
    mac_apf_filter_cmd_stru *apf_filter_cmd = NULL;
    int32_t l_ret;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_apf_program::mac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    apf_filter_cmd = (mac_apf_filter_cmd_stru *)param;

    /* 申请netbuf */
    cmd_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
        sizeof(mac_apf_cmd_type_uint8) + apf_filter_cmd->us_program_len, OAL_NETBUF_PRIORITY_MID);
    if (cmd_netbuf == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_set_apf_program::netbuf alloc null!}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 拷贝命令结构体到netbuf */
    l_ret = memcpy_s(oal_netbuf_data(cmd_netbuf), sizeof(mac_apf_cmd_type_uint8) + apf_filter_cmd->us_program_len,
                     &apf_filter_cmd->en_cmd_type, sizeof(mac_apf_cmd_type_uint8));
    oal_netbuf_put(cmd_netbuf, sizeof(mac_apf_cmd_type_uint8));

    if (apf_filter_cmd->en_cmd_type == APF_SET_FILTER_CMD) {
        /* program内容拷贝到netbuf */
        l_ret += memcpy_s(oal_netbuf_data(cmd_netbuf) + sizeof(mac_apf_cmd_type_uint8),
            apf_filter_cmd->us_program_len, apf_filter_cmd->puc_program, apf_filter_cmd->us_program_len);
        oal_netbuf_put(cmd_netbuf, apf_filter_cmd->us_program_len);
        /* 打印到sdt */
        hmac_print_apf_program(apf_filter_cmd->puc_program, apf_filter_cmd->us_program_len);
    }
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_apf_filter_cmd::memcpy fail!");
        oal_netbuf_free(cmd_netbuf);
        return OAL_FAIL;
    }
    ret = hmac_send_apf_filter_cmd_event(mac_vap, cmd_netbuf);
    oal_netbuf_free(cmd_netbuf);
    return ret;
}

/*
 * 函 数 名  : hmac_apf_program_report_event
 * 功能描述  : apf program上报事件处理
 * 1.日    期  : 2018年3月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_apf_program_report_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    frw_event_hdr_stru *pst_event_hdr;
    dmac_apf_report_event_stru *pst_report_event;
    oal_netbuf_stru *pst_netbuf;
    uint8_t *puc_program = NULL;

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);
    pst_report_event = (dmac_apf_report_event_stru *)pst_event->auc_event_data;

    pst_netbuf = (oal_netbuf_stru *)pst_report_event->p_program;
    if (!pst_netbuf) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_apf_program_report_event:netbuf is null");
        return OAL_FAIL;
    }
    puc_program = oal_netbuf_data(pst_netbuf);
    hmac_print_apf_program(oal_netbuf_data(pst_netbuf), oal_netbuf_len(pst_netbuf));

    oal_netbuf_free(pst_netbuf);
    return OAL_SUCC;
}
#endif
uint32_t hmac_config_force_stop_filter(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_rx_filter_force_switch::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_FORCE_STOP_FILTER, us_len, puc_param);
}

/*
 * 函 数 名  : hmac_config_set_owe
 * 功能描述  : 设置APUT模式下支持的OWE group
 * 1.日    期  : 2019年1月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_owe(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    uint8_t uc_owe_group;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_owe::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_owe::not AP mode!}");
        return OAL_SUCC;
    }

    uc_owe_group = *puc_param;
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_owe::AP mode support owe group=0x%02x}", uc_owe_group);

    pst_hmac_vap->owe_group_cap = uc_owe_group;

    /* MAC_OWE_GROUP_19 */
    pst_hmac_vap->owe_group[0] = (uc_owe_group & WAL_HIPRIV_OWE_19) ? MAC_OWE_GROUP_19 : 0;
    /* MAC_OWE_GROUP_20 */
    pst_hmac_vap->owe_group[1] = (uc_owe_group & WAL_HIPRIV_OWE_20) ? MAC_OWE_GROUP_20 : 0;
    /* MAC_OWE_GROUP_21 */
    pst_hmac_vap->owe_group[2] = (uc_owe_group & WAL_HIPRIV_OWE_21) ? MAC_OWE_GROUP_21 : 0;

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_fem_lp_flag
 * 功能描述  : 上层fem低功耗开关
 * 1.日    期  : 2019年1月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_fem_lp_flag(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
       ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SPEC_FEMLP_EN, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_fem_lp_flag::hmac_config_fem_lp_flag fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_softap_mimo_mode
 * 功能描述  : 上层配置softap启动是否以mimo模式
 * 1.日    期  : 2019年1月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_softap_mimo_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
       ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SOFTAP_MIMO_MODE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_softap_mimo_mode::config_softap_mimo_mode fail[%d].", ret);
        return ret;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_NAN
/*
 * 功能描述  : 下发nan管理帧到device
 * 1.日    期  : 2020年4月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_nan_tx_mgmt(mac_vap_stru *pst_mac_vap, mac_nan_mgmt_info_stru *mgmt_info, uint16_t transaction)
{
#define NAN_TAIL_INFO_LEN 3
    oal_netbuf_stru *netbuf_mgmt_tx = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint8_t *tail;
    uint32_t ret;
    if (mgmt_info->data == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_nan_tx_mgmt::tx data ptr is null}");
        return OAL_FAIL;
    }
    if (mgmt_info->len + NAN_TAIL_INFO_LEN + sizeof(dmac_ctx_action_event_stru) > WLAN_MEM_NETBUF_SIZE2) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_nan_tx_mgmt::len[%d] invalid}", mgmt_info->len);
        return OAL_FAIL;
    }

    /*  申请netbuf */
    netbuf_mgmt_tx = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (netbuf_mgmt_tx == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_nan_tx_mgmt::pst_mgmt_tx null.}");
        return OAL_FAIL;
    }

    /* 填充netbuf */
    if (memcpy_s((uint8_t *)oal_netbuf_header(netbuf_mgmt_tx), WLAN_MEM_NETBUF_SIZE2,
        mgmt_info->data, mgmt_info->len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_nan_tx_mgmt::memcpy fail!");
        oal_netbuf_free(netbuf_mgmt_tx);
        return OAL_FAIL;
    }

    /* 帧体尾部增加3字节信息字段用来标记是否周期性发送和transaction id */
    oal_netbuf_put(netbuf_mgmt_tx,  mgmt_info->len + NAN_TAIL_INFO_LEN + sizeof(dmac_ctx_action_event_stru));
    tail = oal_netbuf_header(netbuf_mgmt_tx) + mgmt_info->len;
    *tail = (mgmt_info->periodic == 0); /* 帧体尾部字节0存放是否周期性发送 */
    tail[1] = transaction & 0xff; /* 帧体尾部字节1和字节2存放transaction id */
    tail[2] = (transaction >> 8) & 0xff; /* 帧体尾部字节1和字节2存放transaction id, 右移8位获取高位 */

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf_mgmt_tx);
    memset_s(tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));
    /* mpdu长度加上尾部3字节信息 */
    mac_get_cb_mpdu_len(tx_ctl) = mgmt_info->len + NAN_TAIL_INFO_LEN + sizeof(dmac_ctx_action_event_stru);
    mac_get_cb_tx_user_idx(tx_ctl) = g_wlan_spec_cfg->invalid_user_id; /* NAN发送给未关联的设备 */
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_ACTION;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_AC_MGMT;
    mac_get_cb_frame_subtype(tx_ctl) = mgmt_info->action;
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, netbuf_mgmt_tx, mgmt_info->len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf_mgmt_tx);
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_nan_tx_mgmt::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

uint32_t hmac_config_nan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_nan_cfg_msg_stru *nan_msg = (mac_nan_cfg_msg_stru*)param;
    uint32_t ret;
    /* 目前限制sta模式下支持NAN */
    if (!is_legacy_sta(mac_vap)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_nan::Not STA mode}");
        return OAL_SUCC;
    }

    switch (nan_msg->type) {
        case NAN_CFG_TYPE_SET_PARAM:
            ret = hmac_config_sync_cmd_common(mac_vap, WLAN_CFGID_NAN, len, param);
            break;
        case NAN_CFG_TYPE_SET_TX_MGMT:
            ret = hmac_nan_tx_mgmt(mac_vap, &nan_msg->mgmt_info, nan_msg->transaction_id);
            break;
        default:
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_nan::invalid nan cfg type[%d]}", nan_msg->type);
            ret = OAL_SUCC;
            break;
    }

    return ret;
}
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
uint32_t hmac_config_set_hid2d_acs_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    uint8_t uc_acs_mode;
    uc_acs_mode = *puc_param;

    /* 设置HiD2D acs mode */
    hmac_hid2d_set_acs_mode(pst_mac_vap, uc_acs_mode);
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_LOW_LATENCY_SWITCHING
OAL_STATIC void hmac_config_freq_boost(bool enable)
{
    mac_cfg_set_tlv_stru tlv_cmd;
    if (enable) {
        /* disable auto-switching first */
        tlv_cmd.uc_cmd_type = CMD_SET_AUTO_FREQ_ENDABLE;
        tlv_cmd.value    = FREQ_LOCK_DISABLE;
        hmac_config_set_auto_freq_enable(NULL, 0, (uint8_t *)&tlv_cmd);

        /* then set the highest chip frequency */
        tlv_cmd.uc_cmd_type = CMD_SET_DEVICE_FREQ_VALUE;
        tlv_cmd.value    = FREQ_HIGHEST;
        hmac_config_set_auto_freq_enable(NULL, 0, (uint8_t *)&tlv_cmd);
    } else {
        /* enable auto-switching */
        tlv_cmd.uc_cmd_type = CMD_SET_AUTO_FREQ_ENDABLE;
        tlv_cmd.value    = FREQ_LOCK_ENABLE;
        hmac_config_set_auto_freq_enable(NULL, 0, (uint8_t *)&tlv_cmd);
    }
}

void hmac_low_latency_freq_high(void)
{
    hmac_config_freq_boost(OAL_TRUE);
}

void hmac_low_latency_freq_default(void)
{
    hmac_config_freq_boost(OAL_FALSE);
}
#endif
/*
 * 功能描述  : 请求device tsf信息
 * 日    期  : 2021年03月21日
 * 作    者  : wifi
 */
uint32_t hmac_config_get_sync_tsf(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_QUERY_TSF, len, param);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_get_sync_tsf: fail to send event to dmac}");
    }

    return ret;
}

/*
 * 功能描述  : 处理device上报的tsf信息
 * 日    期  : 2021年03月21日
 * 作    者  : wifi
 */
uint32_t hmac_config_tsf_event_process(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = NULL;
    query_device_tsf_stru *device_tsf = NULL;

    if (oal_any_null_ptr2(mac_vap, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    device_tsf = (query_device_tsf_stru *)param;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_tsf_event_process: cannot get hmac vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap->sync_tsf.device_tsf = (((uint64_t)device_tsf->tsf_hi) << BIT_OFFSET_32) | ((uint64_t)device_tsf->tsf_lo);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_vap->sync_tsf.host_end_tsf = ktime_get_real_ns();
#else
    hmac_vap->sync_tsf.host_end_tsf = oal_time_get_high_precision_ms();
#endif

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    hmac_vap->tsf_info_query_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&hmac_vap->query_wait_q);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_11K
static void hmac_config_encap_action_header(mac_vap_stru *mac_vap, mac_user_stru *mac_user,
    oal_netbuf_stru *action_neighbor_req)
{
    uint8_t *data = (uint8_t *)oal_netbuf_header(action_neighbor_req);

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    data[BYTE_OFFSET_2] = 0;
    data[BYTE_OFFSET_3] = 0;

    /* DA is address of STA requesting association */
    oal_set_mac_addr(data + BYTE_OFFSET_4, mac_user->auc_user_mac_addr);

    /* SA is the dot11MACAddress */
    oal_set_mac_addr(data + BYTE_OFFSET_10, mac_mib_get_StationID(mac_vap));

    oal_set_mac_addr(data + BYTE_OFFSET_16, mac_vap->auc_bssid);

    /* seq control */
    data[BYTE_OFFSET_22] = 0;
    data[BYTE_OFFSET_23] = 0;
}

/*
 * 函 数 名  : hmac_config_send_neighbor_req
 * 功能描述  : 发送neighbor req配置命令
 * 1.日    期  : 2016年6月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_send_neighbor_req(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
    oal_netbuf_stru *action_neighbor_req;
    uint16_t neighbor_req_frm_len;
    mac_tx_ctl_stru *tx_ctl;
    uint16_t idx;
    uint8_t *data = NULL;
    mac_cfg_ssid_param_stru *ssid = (mac_cfg_ssid_param_stru *)puc_param;
    mac_user_stru *mac_user = mac_res_get_mac_user(mac_vap->us_assoc_vap_id);
    if (mac_user == NULL) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_config_send_neighbor_req::mac_user[%d] null.", mac_vap->us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    action_neighbor_req =
        (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (action_neighbor_req == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_config_send_neighbor_req::action_neighbor_req null.}");
        return ret;
    }

    memset_s(oal_netbuf_cb(action_neighbor_req), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    /* 组帧头 */
    hmac_config_encap_action_header(mac_vap, mac_user, action_neighbor_req);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*             Neighbor report request Frame - Frame Body                */
    /*        -------------------------------------------------              */
    /*        | Category | Action |  Dialog Token | Opt SubEle |             */
    /*        -------------------------------------------------              */
    /*        | 1        | 1      |       1       | Var        |             */
    /*        -------------------------------------------------              */
    /*************************************************************************/
    data = (uint8_t *)oal_netbuf_header(action_neighbor_req);

    /* Initialize index and the frame data pointer */
    idx = MAC_80211_FRAME_LEN;

    /* Category */
    data[idx++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;

    /* Action */
    data[idx++] = MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST;

    /* Dialog Token */
    data[idx++] = 1;

    if (ssid->uc_ssid_len != 0) {
        /* Subelement ID */
        data[idx++] = 0;

        /* length */
        data[idx++] = ssid->uc_ssid_len;

        /* SSID */
        if (memcpy_s(data + idx, WLAN_MEM_NETBUF_SIZE2 - idx, ssid->ac_ssid, ssid->uc_ssid_len) != EOK) {
            oam_error_log0(0, OAM_SF_TX, "hmac_config_send_neighbor_req::memcpy fail!");
            oal_netbuf_free(action_neighbor_req);
            return OAL_FAIL;
        }
        idx += ssid->uc_ssid_len;
    }

    neighbor_req_frm_len = idx;

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(action_neighbor_req);
    mac_get_cb_mpdu_len(tx_ctl) = neighbor_req_frm_len;
    /* 发送完成需要获取user结构体 */
    ret = mac_vap_set_cb_tx_user_idx(mac_vap, tx_ctl, mac_user->auc_user_mac_addr);
    if (ret != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_TX, "(hmac_config_send_neighbor_req::fail to find user by \
            %02X:XX:XX:XX:%02X:%02X.}", mac_user->auc_user_mac_addr[MAC_ADDR_0],
            mac_user->auc_user_mac_addr[MAC_ADDR_4], mac_user->auc_user_mac_addr[MAC_ADDR_5]);
    }

    oal_netbuf_put(action_neighbor_req, neighbor_req_frm_len);

    ret = hmac_tx_mgmt_send_event(mac_vap, action_neighbor_req, neighbor_req_frm_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(action_neighbor_req);
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_config_send_neighbor_req::send_event failed[%d]}", ret);
    }

    return ret;
}
#endif  // _PRE_WLAN_FEATURE_11K
uint32_t hmac_config_set_dc_status(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    mac_device_stru *mac_device = mac_res_get_mac_dev();
    mac_device->dc_status = *puc_param;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DC_STATUS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_dc_status::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_p2p_miracast_status
 * 功能描述  : 设置miracast状态
 * 1.日    期  : 2018年12月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_p2p_miracast_status(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_MIRACAST_STATUS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_miracast_status::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_p2p_ps_ops
 * 功能描述  : 设置P2P OPS 节能
 * 1.日    期  : 2015年1月14日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_p2p_ps_ops(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_p2p_ops_param_stru *pst_p2p_ops;
    pst_p2p_ops = (mac_cfg_p2p_ops_param_stru *)puc_param;
    oam_info_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_p2p_ps_ops:ctrl:%d, ct_window:%d}",
                  pst_p2p_ops->en_ops_ctrl,
                  pst_p2p_ops->uc_ct_window);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_PS_OPS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_ps_ops::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_p2p_ps_noa
 * 功能描述  : 设置P2P NOA 节能
 * 1.日    期  : 2015年1月14日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_p2p_ps_noa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_p2p_noa_param_stru *pst_p2p_noa;
    pst_p2p_noa = (mac_cfg_p2p_noa_param_stru *)puc_param;
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_p2p_ps_noa:start_time:%d, duration:%d, interval:%d, count:%d}",
                     pst_p2p_noa->start_time, pst_p2p_noa->duration, pst_p2p_noa->interval, pst_p2p_noa->uc_count);
    /* ms to us */
    pst_p2p_noa->start_time *= HMAC_MS_TO_US;
    pst_p2p_noa->duration *= HMAC_MS_TO_US;
    pst_p2p_noa->interval *= HMAC_MS_TO_US;

    if (pst_p2p_noa->uc_count != 0 && pst_p2p_noa->duration >= pst_p2p_noa->interval) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_p2p_ps_noa:noa_params invalid!}");
        return OAL_FAIL;
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_PS_NOA, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_ps_noa::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_p2p_ps_stat
 * 功能描述  : 设置P2P 节能统计
 * 1.日    期  : 2015年1月14日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_p2p_ps_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_p2p_stat_param_stru *pst_p2p_stat;
    pst_p2p_stat = (mac_cfg_p2p_stat_param_stru *)puc_param;
    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "(hmac_config_set_p2p_ps_stat::ctrl %d}",
                  pst_p2p_stat->uc_p2p_statistics_ctrl);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_P2P_PS_STAT, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_p2p_ps_stat::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_set_ip_addr
 * 功能描述  : 配置信息
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 配置DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_IP_ADDR, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_ip_addr::hmac_config_send_event fail[%d].", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_del_p2p_ie
 * 功能描述  : 删除wpa_supplicant 下发的IE 中的P2P IE
 * 1.日    期  : 2015年8月11日
 *   修改内容  : 新生成函数
 */
void hmac_config_del_p2p_ie(uint8_t *puc_ie, uint32_t *pul_ie_len)
{
    uint8_t *puc_p2p_ie = NULL;
    uint32_t p2p_ie_len;
    uint8_t *puc_ie_end = NULL;
    uint8_t *puc_p2p_ie_end = NULL;

    if ((oal_any_null_ptr2(puc_ie, pul_ie_len)) || (*pul_ie_len == 0)) {
        return;
    }

    puc_p2p_ie = mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, puc_ie, (int32_t)(*pul_ie_len));
    if ((puc_p2p_ie == NULL) || (puc_p2p_ie[1] < MAC_P2P_MIN_IE_LEN)) {
        return;
    }

    p2p_ie_len = puc_p2p_ie[1] + MAC_IE_HDR_LEN;

    /* 将p2p ie 后面的内容拷贝到p2p ie 所在位置 */
    puc_ie_end = (puc_ie + *pul_ie_len);
    puc_p2p_ie_end = (puc_p2p_ie + p2p_ie_len);

    if (puc_ie_end >= puc_p2p_ie_end) {
        if (memmove_s(puc_p2p_ie, *pul_ie_len, puc_p2p_ie_end, (uint32_t)(puc_ie_end - puc_p2p_ie_end)) != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "hmac_config_del_p2p_ie::memmove fail!");
            return;
        }
        *pul_ie_len -= p2p_ie_len;
    }
    return;
}
#ifdef _PRE_WLAN_FEATURE_CSI
/*
 * 功能描述  :  上层驻留bssid需求，需要关闭漫游最多两分钟，两分钟后，驱动自动恢复漫游使能，也可以由上层下发命令提前恢复。
 * 修改历史  : mp13 mp15 需要
 * 1.日    期  : 2020年11月20日
 *   作    者 : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_roam_enable_timeout_handle(void *p_arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)p_arg;
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;

    roam_info->stay_on_current_bss = OAL_FALSE; // 恢复接受ap发的漫游请求
    return hmac_roam_enable(hmac_vap, OAL_TRUE);
}
#define ROAM_REENABLE_TIMEOUT_MS  120000 /* bssid驻留需求下发漫游关闭, 2分钟后使能漫游 */
uint32_t hmac_csi_enable_roam(mac_vap_stru *mac_vap, uint8_t *param)
{
    uint8_t roam_enable_value;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_roam_info_stru *roam_info = NULL;
    mac_cfg_csi_param_stru *cfg_csi_param = NULL;

    cfg_csi_param = (mac_cfg_csi_param_stru *)param;
    roam_enable_value = cfg_csi_param->en_cfg_csi_on ? 0 : 1; // csi打开，漫游关闭；csi关闭，漫游打开

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_roam_enable::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_roam_enable::roam_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_enable_value == 0) {
        /* 当前不在漫游，可以关闭漫游 */
        if (mac_vap->en_vap_state != MAC_VAP_STATE_ROAMING) {
            if (hmac_roam_enable(hmac_vap, roam_enable_value) != OAL_SUCC) {
                oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_roam_enable::roam was disable}");
            } else {
                /* 起定时器，2分钟后驱动侧自动恢复漫游使能 */
                frw_timer_create_timer_m(&roam_info->st_timer, hmac_roam_enable_timeout_handle,
                    ROAM_REENABLE_TIMEOUT_MS, hmac_vap, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_vap->core_id);

                roam_info->stay_on_current_bss = OAL_TRUE; // 拒绝ap发的漫游请求，驻留当前bss
            }
        } else {
            /* 当前正在漫游，不能关闭漫游，此次关闭漫游命令不执行 */
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_set_roam_enable::roam is running! can't disable roam.}");
        }
    } else {
        /* 上层bssid驻留不到2分钟提前使能漫游 */
        if (hmac_roam_enable(hmac_vap, roam_enable_value) != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_roam_enable::roam was enable}");
        } else {
            /* 删除定时器 */
            frw_timer_immediate_destroy_timer_m(&roam_info->st_timer);
            roam_info->stay_on_current_bss = OAL_FALSE; // 恢复接受ap发的漫游请求
        }
    }

    return OAL_SUCC;
}

mac_csi_black_list_stru g_csi_black_list = {0};
OAL_STATIC mac_csi_black_list_stru *hmac_get_csi_black_list_handler(void)
{
    return &g_csi_black_list;
}

uint32_t hmac_config_csi_set_black_list_rsp(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    int32_t ret;
    mac_csi_black_list_stru *csi_black_list = hmac_get_csi_black_list_handler();

    csi_black_list->csi_black_list_num++;
    if (csi_black_list->csi_black_list_num >= WLAN_CSI_BLACK_LIST_NUM) {
        ret = memcpy_s(&csi_black_list->csi_black_list_mac_map[WLAN_CSI_BLACK_LIST_NUM - 1][MAC_ADDR_0],
            WLAN_MAC_ADDR_LEN, puc_param, uc_len);
    } else {
        ret = memcpy_s(&csi_black_list->csi_black_list_mac_map[csi_black_list->csi_black_list_num - 1][MAC_ADDR_0],
            WLAN_MAC_ADDR_LEN, puc_param, uc_len);
    }
    if (ret != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_csi_set_black_list_rsp::memcpy fail.}");
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : mp13 mp15 csi参数抛到device
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_device_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    mac_cfg_csi_param_stru *cfg_csi_param = (mac_cfg_csi_param_stru *)param;
    mac_csi_black_list_stru *csi_black_list = hmac_get_csi_black_list_handler();

    if (mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
        ret = hmac_csi_enable_roam(mac_vap, param);
        if (ret != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_csi::hmac_csi_enable_roam fail.}");
        }
    }
    if (cfg_csi_param->en_cfg_csi_on) { // csi开启后同步黑名单信息到dmac
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_CSI_BLACK_LIST,
            sizeof(mac_csi_black_list_stru), (uint8_t *)csi_black_list);
        if (ret != OAL_SUCC) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_csi::set black list fail.}");
        }
    }
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_CSI, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_device_csi_config::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_csi_config
 * 功能描述  : 通过设置CSI
 * 1.日    期  : 2020年11月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint8_t val;
    val = wlan_chip_get_d2h_access_ddr();
    if (val) {
        return wlan_chip_host_csi_config(mac_vap, len, param);
    } else {
        return wlan_chip_device_csi_config(mac_vap, len, param);
    }
}
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
/*
 * 函 数 名   : hmac_config_set_user_hiex_enable
 * 功能描述   : 设置用户hiex_cap
 * 1.日    期   : 2019年12月07日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t hmac_config_set_user_hiex_enable(mac_vap_stru *pst_mac_vap,
    uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_user_hiex_param_stru *pst_cfg       = NULL;
    hmac_user_stru               *pst_hmac_user = NULL;
    mac_hiex_cap_stru            *pst_cap       = NULL;

    pst_cfg = (mac_cfg_user_hiex_param_stru *)puc_param;
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_cfg->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_HIEX, "{hmac_config_set_user_hiex_enable::pst_hmac_user null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cap = mac_user_get_hiex_cap(&pst_hmac_user->st_user_base_info);
    mac_hiex_nego_cap(pst_cap, &pst_cfg->st_hiex_cap, NULL);

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_user_hiex_enable::user id %d, enable %d}",
                     pst_hmac_user->st_user_base_info.us_assoc_id, pst_cap->bit_himit_enable);

    /* 抛事件到DMAC层, 同步DMAC数据 */
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_HIEX_ENABLE, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_HIEX,
                         "{hmac_config_set_user_hiex_enable::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
/*
 * 函 数 名  : hmac_config_set_m2s_switch
 * 功能描述  : 设置mimo或者siso工作模式
 * 1.日    期  : 2016年9月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_m2s_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_config_set_m2s_switch::\
                en_vap_state is MAC_VAP_STATE_ROAMING m2s_switch is forbid.}");
        return OAL_SUCC;
    }
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_M2S_SWITCH, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                       "{hmac_config_set_m2s_switch::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

/*
 * 函 数 名  : hmac_config_ru_set
 * 功能描述  : 设置11ax ru模式
 * 1.日    期  : 2019年7月29日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_ru_set(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_RU_TEST, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{hmac_config_radar_set::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*
 * 函 数 名  : hmac_config_get_user_rssbw
 * 功能描述  : hmac获取用户带宽和空间流信息
 * 1.日    期  : 2014年6月12日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_user_rssbw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_add_user_param_stru *pst_user;
    hmac_vap_stru *pst_hmac_vap;
    hmac_user_stru *pst_hmac_user;
    uint32_t ret;

    pst_user = (mac_cfg_add_user_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_OPMODE, "{hmac_config_get_user_rssbw::pst_hmac_vap null.}");
        return OAL_FAIL;
    }

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_user->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{hmac_config_get_user_rssbw::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE,
                     "{hmac_config_get_user_rssbw:: user macaddr %02X:XX:XX:XX:%02X:%02X.}",
                     pst_user->auc_mac_addr[MAC_ADDR_0],
                     pst_user->auc_mac_addr[MAC_ADDR_4], pst_user->auc_mac_addr[MAC_ADDR_5]);
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE,
                     "{hmac_config_get_user_rssbw::nss_cap[%d]avail_nss[%d]user bw_cap[%d]avail_bw[%d].}",
                     pst_hmac_user->st_user_base_info.en_user_max_cap_nss,
                     pst_hmac_user->st_user_base_info.en_avail_num_spatial_stream,
                     pst_hmac_user->st_user_base_info.en_bandwidth_cap,
                     pst_hmac_user->st_user_base_info.en_avail_bandwidth);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_USER_RSSBW, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE,
                       "{hmac_config_get_user_rssbw::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif
