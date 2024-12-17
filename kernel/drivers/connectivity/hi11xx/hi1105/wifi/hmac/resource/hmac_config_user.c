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
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_USER_C
typedef struct {
    wlan_protocol_enum_uint8 en_protocol_mode; /* wid枚举 */
    uint8_t auc_resv[NUM_3_BYTES];
    int8_t *puc_protocol_desc;
} hmac_protocol_stru;

OAL_STATIC hmac_protocol_stru g_st_protocol_mode_list[WLAN_PROTOCOL_BUTT] = {
    { WLAN_LEGACY_11A_MODE,    { 0 }, "11a" },
    { WLAN_LEGACY_11B_MODE,    { 0 }, "11b" },
    { WLAN_LEGACY_11G_MODE,    { 0 }, "abandon_mode" },
    { WLAN_MIXED_ONE_11G_MODE, { 0 }, "11bg" },
    { WLAN_MIXED_TWO_11G_MODE, { 0 }, "11g" },
    { WLAN_HT_MODE,            { 0 }, "11n" },
    { WLAN_VHT_MODE,           { 0 }, "11ac" },
    { WLAN_HT_ONLY_MODE,       { 0 }, "11n_only" },
    { WLAN_VHT_ONLY_MODE,      { 0 }, "11ac_only" },
    { WLAN_HT_11G_MODE,        { 0 }, "11ng" },
#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    { WLAN_HE_MODE, { 0 }, "11ax" },
#endif
};
OAL_STATIC uint32_t hmac_fill_user_info(mac_vap_stru *mac_vap, int8_t *print_buff, int32_t remainder_len)
{
    oal_dlist_head_stru *head = NULL;
    mac_user_stru *mac_user = NULL;
    uint16_t user_idx;
    wlan_protocol_enum_uint8 protocol_mode;
    int32_t ret;
    int8_t tmp_buff[256] = { 0 }; /* 临时存放user信息的buffer长度 256 */
    for (user_idx = 0; user_idx < MAC_VAP_USER_HASH_MAX_VALUE; user_idx++) {
        oal_dlist_search_for_each(head, &(mac_vap->ast_user_hash[user_idx]))
        {
            /* 找到相应用户 */
            mac_user = (mac_user_stru *)oal_dlist_get_entry(head, mac_user_stru, st_user_hash_dlist);
            if (mac_user == NULL) {
                oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_fill_user_info::mac_user null.}");
                continue;
            }
            /* user结构体下的协议模式不区分a和g，需要根据频段区分 */
            protocol_mode = mac_user->en_protocol_mode;
            if (protocol_mode >= WLAN_PROTOCOL_BUTT) {
                oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                    "{hmac_fill_user_info:: protocol_mode wrong.}", protocol_mode);
                continue;
            }
            if ((protocol_mode == WLAN_LEGACY_11G_MODE) && (mac_vap->st_channel.en_band == WLAN_BAND_5G)) {
                protocol_mode = WLAN_LEGACY_11A_MODE;
            }
            ret = snprintf_s(tmp_buff, sizeof(tmp_buff), sizeof(tmp_buff) - 1,
                "     %u       %02X:XX:XX:XX:%02X:%02X       %s \n", mac_user->us_assoc_id,
                mac_user->auc_user_mac_addr[MAC_ADDR_0], mac_user->auc_user_mac_addr[MAC_ADDR_4],
                mac_user->auc_user_mac_addr[MAC_ADDR_5], g_st_protocol_mode_list[protocol_mode].puc_protocol_desc);
            if (ret < 0) {
                oam_error_log0(0, OAM_SF_CFG, "hmac_fill_user_info::snprintf_s error!");
                return OAL_FAIL;
            }
            if (strncat_s(print_buff, OAM_REPORT_MAX_STRING_LEN, tmp_buff, remainder_len - 1) != EOK) {
                oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_fill_user_info::strncat_s fail.}");
                return OAL_FAIL;
            }
            memset_s(tmp_buff, sizeof(tmp_buff), 0, sizeof(tmp_buff));
            remainder_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(print_buff));
        }
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_list_sta
 * 功能描述  : 显示AP侧关联的用户
 * 1.日    期  : 2013年7月22日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_list_sta(mac_vap_stru *pst_mac_vap)
{
    int32_t l_remainder_len;
    int8_t *pc_print_tmp = NULL;
    int8_t *pc_print_buff = NULL;
    /* AP侧的信息才能打印相关信息 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_list_sta::invalid en_vap_mode[%d].}",
            pst_mac_vap->en_vap_mode);
        return OAL_FAIL;
    }
    pc_print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    if (snprintf_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN - 1,
        "Total user num is %u \nUser assoc id ADDR Protocol Type \n", pst_mac_vap->us_user_nums) < 0) {
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return OAL_FAIL;
    };
    pc_print_tmp = pc_print_buff;
    l_remainder_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - OAL_STRLEN(pc_print_tmp));
    /* 同步02  */
    oal_spin_lock_bh(&pst_mac_vap->st_cache_user_lock);
    if (hmac_fill_user_info(pst_mac_vap, pc_print_tmp, l_remainder_len) != EOK) {
        oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return OAL_FAIL;
    }
    /* 同步02  */
    oal_spin_unlock_bh(&pst_mac_vap->st_cache_user_lock);
    oam_print(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_protection_update_sta_user
 * 功能描述  : 用户改变同步保护机制
 * 1.日    期  : 2017年1月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_protection_update_from_user(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    mac_dump_protection(pst_mac_vap, puc_param);

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_PROTECTION_UPDATE_STA_USER, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_protection::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) || defined(_PRE_WLAN_FEATURE_SMPS)
/*
 * 函 数 名  : hmac_show_m2s_sync_cap
 * 功能描述  : 打印m2s user同步前后信息
 * 1.日    期  : 2018年9月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_show_m2s_sync_cap(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, mac_user_m2s_stru *pst_syn_info)
{
    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_show_m2s_sync_cap:: \
                     original user cap user_nss[%d] avail_nss[%d] bf_nss[%d] smps_mode[%d]",
                     pst_mac_user->en_user_max_cap_nss, pst_mac_user->en_avail_num_spatial_stream,
                     pst_mac_user->en_avail_bf_num_spatial_stream, pst_mac_user->st_ht_hdl.bit_sm_power_save);

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_show_m2s_sync_cap::avail_bw[%d] cur_bw[%d]",
                     pst_mac_user->en_avail_bandwidth, pst_mac_user->en_cur_bandwidth);

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_show_m2s_sync_cap:: \
                     new user cap user_nss[%d] avail_nss[%d] bf_nss[%d] smps_mode[%d]",
                     pst_syn_info->en_user_max_cap_nss, pst_syn_info->en_avail_num_spatial_stream,
                     pst_syn_info->en_avail_bf_num_spatial_stream, pst_syn_info->en_cur_smps_mode);

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_show_m2s_sync_cap::avail_bw[%d] cur_bw[%d]",
                     pst_syn_info->en_avail_bandwidth, pst_syn_info->en_cur_bandwidth);
}

/*
 * 函 数 名  : hmac_config_user_m2s_info_syn
 * 功能描述  : m2s user 信息同步
 * 1.日    期  : 2016年9月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_user_m2s_info_syn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_user_m2s_stru *pst_syn_info = NULL;
    mac_user_stru *pst_mac_user = NULL;

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || oal_any_null_ptr2(pst_mac_vap->pst_mib_info, puc_param)) {
        oam_warning_log1(0, OAM_SF_M2S, "{hmac_config_user_m2s_info_syn::pst_mac_vap->uc_init_flag[%d], \
                         pst_mac_vap->pst_mib_info or puc_param null!}",
                         pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_user_m2s_stru *)puc_param;

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_syn_info->us_user_idx);
    if (oal_unlikely(pst_mac_user == NULL)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{hmac_config_user_m2s_info_syn:: \
                         pst_mac_user null.user idx [%d]}", pst_syn_info->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 打印同步前后user的带宽空间流信息 */
    hmac_show_m2s_sync_cap(pst_mac_vap, pst_mac_user, pst_syn_info);

    /* 同步USR带宽 */
    mac_user_set_bandwidth_info(pst_mac_user, pst_syn_info->en_avail_bandwidth, pst_syn_info->en_cur_bandwidth);

    /* 同步用户空间流 */
    mac_user_set_num_spatial_stream(pst_mac_user, pst_syn_info->en_user_max_cap_nss);
    mac_user_set_avail_num_spatial_stream(pst_mac_user, pst_syn_info->en_avail_num_spatial_stream);
    mac_user_avail_bf_num_spatial_stream(pst_mac_user, pst_syn_info->en_avail_bf_num_spatial_stream);
#ifdef _PRE_WLAN_FEATURE_SMPS
    mac_user_set_sm_power_save(pst_mac_user, pst_syn_info->en_cur_smps_mode);
#endif
    mac_user_set_smps_opmode_notify_nss(pst_mac_user, pst_syn_info->en_smps_opmode_nodify_nss);

    return OAL_SUCC;
}
#endif

/*
 * 函 数 名  : hmac_config_user_asoc_state_syn
 * 功能描述  : dmac_offload架构下同步user关联状态到device侧
 * 1.日    期  : 2014年12月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_user_asoc_state_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    uint32_t ret;
    mac_h2d_user_asoc_state_stru st_h2d_user_asoc_state_stru;

    st_h2d_user_asoc_state_stru.us_user_idx = pst_mac_user->us_assoc_id;
    st_h2d_user_asoc_state_stru.en_asoc_state = pst_mac_user->en_user_asoc_state;

    /***************************************************************************
        抛事件到DMAC层, 同步user关联状态到device侧
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_ASOC_STATE_SYN,
                                 sizeof(mac_h2d_user_asoc_state_stru), (uint8_t *)(&st_h2d_user_asoc_state_stru));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_asoc_state_syn::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_user_htc_cap_update
 * 功能描述  : 查询wifi6 black list 如果user在htc blacklist 里面关闭 htc
 * 1.日    期  : 2020年1月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_config_user_htc_cap_update(mac_vap_stru *p_mac_vap, mac_user_stru *p_mac_user)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_user_arp_probe_close_htc(p_mac_user) = OAL_FALSE;
    if (is_legacy_sta(p_mac_vap) && mac_user_is_he_user(p_mac_user)) {
        if (hmac_wifi6_self_cure_mac_is_htc_blacklist_type(p_mac_user->auc_user_mac_addr)) {
            mac_user_arp_probe_close_htc(p_mac_user) = OAL_TRUE;
            oam_warning_log3(0, OAM_SF_SCAN, "hmac_config_user_htc_cap_update::\
                mac:%02X:XX:XX:XX:%02X:%02X in htc balcklist, close HTC",
                p_mac_user->auc_user_mac_addr[0],  /* 0为mac地址字节位置 */
                p_mac_user->auc_user_mac_addr[4],  /* 4为mac地址字节位置 */
                p_mac_user->auc_user_mac_addr[5]); /* 5为mac地址字节位置 */
        }
    }
    mac_vap_tx_data_set_user_htc_cap(p_mac_vap, p_mac_user);
#endif
}

/*
 * 函 数 名  : hmac_config_user_cap_syn
 * 功能描述  : dmac offload模式下hmac向dmac同步user cap info的所有内容
 * 1.日    期  : 2015年5月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_user_cap_syn(mac_vap_stru *mac_vap, mac_user_stru *mac_user)
{
    uint32_t ret;
    mac_h2d_usr_cap_stru mac_h2d_usr_cap;
    hmac_user_stru *hmac_user = mac_res_get_hmac_user(mac_user->us_assoc_id);
    if (hmac_user == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_config_user_cap_syn:hmac_user null");
        return OAL_FAIL;
    }
    mac_h2d_usr_cap.us_user_idx = mac_user->us_assoc_id;
    mac_h2d_usr_cap.amsdu_maxsize = hmac_user->amsdu_maxsize;
    mac_h2d_usr_cap.ampdu_maxsize = mac_user->ampdu_maxsize;
    memcpy_s((uint8_t *)(&mac_h2d_usr_cap.st_user_cap_info), sizeof(mac_user_cap_info_stru),
             (uint8_t *)(&mac_user->st_cap_info), sizeof(mac_user_cap_info_stru));
    memcpy_s((uint8_t *)(&mac_h2d_usr_cap.hisi_priv_cap), sizeof(mac_hisi_cap_vendor_ie_stru),
             (uint8_t *)(&mac_user->hisi_priv_cap), sizeof(mac_hisi_cap_vendor_ie_stru));
#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        /* hmac向dmac同步user HIEX能力 */
        memcpy_s((uint8_t *)(&mac_h2d_usr_cap.st_hiex_cap), sizeof(mac_hiex_cap_stru),
                 (uint8_t *)mac_user_get_hiex_cap(mac_user), sizeof(mac_hiex_cap_stru));
    }
#endif
    hmac_config_user_htc_cap_update(mac_vap, mac_user);
    mac_vap_csa_support_set(mac_vap, mac_h2d_usr_cap.st_user_cap_info.bit_p2p_support_csa);

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_USER_CAP_SYN,
        sizeof(mac_h2d_usr_cap_stru), (uint8_t *)(&mac_h2d_usr_cap));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_user_cap_syn::hmac_config_sta_vap_info_syn failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_user_rate_info_syn
 * 功能描述  : dmac_offload架构下同步user速率信息到device侧
 * 1.日    期  : 2015年3月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_user_rate_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    int32_t ret;
    mac_h2d_usr_rate_info_stru st_mac_h2d_usr_rate_info;
    st_mac_h2d_usr_rate_info.us_user_idx = pst_mac_user->us_assoc_id;
    st_mac_h2d_usr_rate_info.en_protocol_mode = pst_mac_user->en_protocol_mode;

    /* legacy速率集信息，同步到dmac */
    st_mac_h2d_usr_rate_info.uc_avail_rs_nrates = pst_mac_user->st_avail_op_rates.uc_rs_nrates;
    ret = memcpy_s(st_mac_h2d_usr_rate_info.auc_avail_rs_rates, WLAN_MAX_SUPP_RATES,
        pst_mac_user->st_avail_op_rates.auc_rs_rates, pst_mac_user->st_avail_op_rates.uc_rs_nrates);
    if (ret != EOK) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_user_rate_info_syn::memcpy_s failed[%d].}", ret);
    }
    /* ht速率集信息，同步到dmac */
    mac_user_get_ht_hdl(pst_mac_user, &st_mac_h2d_usr_rate_info.st_ht_hdl);

    /* vht速率集信息，同步到dmac */
    mac_user_get_vht_hdl(pst_mac_user, &st_mac_h2d_usr_rate_info.st_vht_hdl);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* he速率集信息，同步到dmac */
        mac_user_get_he_hdl(pst_mac_user, &(st_mac_h2d_usr_rate_info.st_he_hdl));
    }
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步user关联状态到device侧
    ***************************************************************************/
    ret = (int32_t)hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USER_RATE_SYN,
                                          sizeof(mac_h2d_usr_rate_info_stru), (uint8_t *)(&st_mac_h2d_usr_rate_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_rate_info_syn::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_config_usr_info_syn
 * 功能描述  : dmac_offload架构下同步sta usr的状态到dmac
 * 1.日    期  : 2014年12月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_user_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    uint32_t ret;
    mac_h2d_usr_info_stru st_mac_h2d_usr_info;
    uint8_t min_mpdu_start_spacing = 0;

    /* 关联6G信道时, min_mpdu_start_spacing能力在he 6ghz band cap IE中 */
    if (pst_mac_vap->st_channel.ext6g_band != OAL_TRUE) {
        min_mpdu_start_spacing = pst_mac_user->st_ht_hdl.uc_min_mpdu_start_spacing;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    } else {
        min_mpdu_start_spacing = pst_mac_user->he_6ghz_band_cap_ie.min_mpdu_start_spacing;
#endif
    }

    st_mac_h2d_usr_info.en_avail_bandwidth = pst_mac_user->en_avail_bandwidth;
    st_mac_h2d_usr_info.en_cur_bandwidth = pst_mac_user->en_cur_bandwidth;
    st_mac_h2d_usr_info.us_user_idx = pst_mac_user->us_assoc_id;
    st_mac_h2d_usr_info.en_user_pmf = pst_mac_user->st_cap_info.bit_pmf_active;
    st_mac_h2d_usr_info.uc_arg1 = pst_mac_user->st_ht_hdl.uc_max_rx_ampdu_factor;
    st_mac_h2d_usr_info.uc_arg2 = min_mpdu_start_spacing;
    st_mac_h2d_usr_info.en_user_asoc_state = pst_mac_user->en_user_asoc_state;

    /* 协议模式信息同步到dmac */
    st_mac_h2d_usr_info.en_avail_protocol_mode = pst_mac_user->en_avail_protocol_mode;

    st_mac_h2d_usr_info.en_cur_protocol_mode = pst_mac_user->en_cur_protocol_mode;
    st_mac_h2d_usr_info.en_protocol_mode = pst_mac_user->en_protocol_mode;
    st_mac_h2d_usr_info.en_bandwidth_cap = pst_mac_user->en_bandwidth_cap;
#ifdef _PRE_WLAN_FEATURE_11AX
    st_mac_h2d_usr_info.bit_in_htc_blacklist = mac_user_arp_probe_close_htc(pst_mac_user);
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_USR_INFO_SYN,
                                 sizeof(st_mac_h2d_usr_info), (uint8_t *)(&st_mac_h2d_usr_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_info_syn::hmac_config_send_event failed[%d],user_id[%d].}",
                         ret, pst_mac_user->us_assoc_id);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_init_user_security_port
 * 功能描述  : 初始化用户的加密端口标志
 * 1.日    期  : 2015年5月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_init_user_security_port(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    uint32_t ret;
    mac_cfg80211_init_port_stru st_init_port;

    /* 初始化认证端口信息 */
    mac_vap_init_user_security_port(pst_mac_vap, pst_mac_user);

    memcpy_s(st_init_port.auc_mac_addr, OAL_MAC_ADDR_LEN, pst_mac_user->auc_user_mac_addr, OAL_MAC_ADDR_LEN);
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_INIT_SECURTIY_PORT,
                                 sizeof(st_init_port), (uint8_t *)&st_init_port);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{hmac_config_user_security_port::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}

/*
 * 函 数 名  : hmac_user_set_asoc_state
 * 功能描述  : 配置用户关联状态，offload模式下同步信息到dmac
 * 1.日    期  : 2015年5月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_user_set_asoc_state(mac_vap_stru *pst_mac_vap,
                                  mac_user_stru *pst_mac_user,
                                  mac_user_asoc_state_enum_uint8 en_value)
{
    uint32_t ret;

    mac_user_set_asoc_state(pst_mac_user, en_value);

    /* dmac offload架构下，同步user关联状态信息到dmac */
    ret = hmac_config_user_asoc_state_syn(pst_mac_vap, pst_mac_user);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_user_set_asoc_state::hmac_config_user_asoc_state_syn failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_user_num_spatial_stream_cap_syn
 * 功能描述  : 同步空间流能力
 * 1.日    期  : 2017年9月9日
 *   作    者  : wifi
 *   修改内容  : 从hmac_user_set_avail_num_space_stream函数中剥离
 */
uint32_t hmac_config_user_num_spatial_stream_cap_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    mac_user_nss_stru st_user_nss;
    uint32_t ret = OAL_FALSE;
    if (pst_mac_vap == NULL) {
        oam_error_log0(pst_mac_user->uc_vap_id, OAM_SF_CFG,
                       "hmac_config_user_num_spatial_stream_cap_syn::mac vap(idx=%d) is null!");
        return ret;
    }

    st_user_nss.en_avail_num_spatial_stream = pst_mac_user->en_avail_num_spatial_stream;
    st_user_nss.en_user_max_cap_nss = pst_mac_user->en_user_max_cap_nss;
    st_user_nss.us_user_idx = pst_mac_user->us_assoc_id;
    st_user_nss.en_user_num_spatial_stream_160m = pst_mac_user->st_vht_hdl.bit_user_num_spatial_stream_160m;
    st_user_nss.en_smps_opmode_notify_nss       = pst_mac_user->en_smps_opmode_notify_nss;

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_NSS,
                                 sizeof(mac_user_nss_stru), (uint8_t *)(&st_user_nss));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_user->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_user_num_spatial_stream_cap_syn:: \
                         hmac_config_user_num_spatial_stream_cap_syn send event failed[%d].}", ret);
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_d2h_user_info_syn
 * 功能描述  : user 信息同步
 * 1.日    期  : 2016年4月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_d2h_user_info_syn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    mac_d2h_syn_info_stru *pst_syn_info = NULL;
    mac_user_stru *pst_mac_user = NULL;
    uint8_t uc_idx;
    uint32_t ret;
    uint8_t band;
#ifdef _PRE_WLAN_FEATURE_160M
    uint8_t fem_enable = 0;
#endif

    if ((pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) || oal_any_null_ptr2(pst_mac_vap->pst_mib_info, puc_param)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_d2h_user_info_syn::pst_mac_vap->uc_init_flag[%d], \
                         pst_mac_vap->pst_mib_info or puc_param null!}", pst_mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_syn_info = (mac_d2h_syn_info_stru *)puc_param;

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_syn_info->us_user_idx);
    if (oal_unlikely(pst_mac_user == NULL)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_user_info_syn::pst_mac_user null.user idx [%d]}", pst_syn_info->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 同步USR带宽 */
    mac_user_set_bandwidth_cap(pst_mac_user, pst_syn_info->en_bandwidth_cap);
    mac_user_set_bandwidth_info(pst_mac_user, pst_syn_info->en_avail_bandwidth, pst_syn_info->en_cur_bandwidth);

#ifdef _PRE_WLAN_FEATURE_160M
    /* STA从160M切换到非160M带宽时，关闭fem低功耗 */
    if (pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS &&
        pst_syn_info->st_channel.en_bandwidth < WLAN_BAND_WIDTH_160PLUSPLUSPLUS) {
        hmac_config_fem_lp_flag(pst_mac_vap, sizeof(uint8_t), &fem_enable);
    }
#endif

    /* 同步信道信息 */
    band = mac_get_band_by_channel_num(pst_syn_info->st_channel.uc_chan_number);
    ret = mac_get_channel_idx_from_num(band, pst_syn_info->st_channel.uc_chan_number,
                                       pst_syn_info->st_channel.ext6g_band, &uc_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_d2h_user_info_syn::mac_get_channel_idx_from_num failed[%d].}", ret);

        return ret;
    }

    pst_mac_vap->st_channel.uc_chan_number = pst_syn_info->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.en_band = pst_syn_info->st_channel.en_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_syn_info->st_channel.en_bandwidth;
    pst_mac_vap->st_channel.uc_chan_idx = uc_idx;

    oam_warning_log4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_d2h_user_info_syn::channe[%d],bandwidth[%d]avail bw_cap[%d],cur_bw_cap[%d]!}",
                     pst_mac_vap->st_channel.uc_chan_number, pst_mac_vap->st_channel.en_bandwidth,
                     pst_mac_user->en_avail_bandwidth, pst_mac_user->en_cur_bandwidth);

    return OAL_SUCC;
}

static uint32_t hmac_get_kicking_user_idx(mac_vap_stru *mac_vap, uint8_t *user_mac, uint16_t *user_idx)
{
    uint32_t ret;
    ret = mac_vap_find_user_by_macaddr(mac_vap, user_mac, user_idx);
    if (ret == OAL_SUCC) {
        return ret;
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        // 漫游状态下，user的mac已经被修改，返回vap记录的idx
        *user_idx = mac_vap->us_assoc_vap_id;
        return OAL_SUCC;
    }

    return ret;
}

OAL_STATIC uint32_t hmac_config_kick_user_check_valid(mac_vap_stru *mac_vap, uint8_t *param)
{
    hmac_vap_stru *hmac_vap = NULL;
    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_kick_user_check_valid::mac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_UM,
            "{hmac_config_kick_user_check_valid::en_vap_mode is WLAN_VAP_MODE_CONFIG.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_kick_user::null param,hmac_vap[%d].}", mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_kick_all_user
 * 功能描述  : 配置命令去关联全部用户
 * 1.日    期  : 2022年3月22日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
static void hmac_config_kick_all_user(hmac_vap_stru *hmac_vap, mac_cfg_kick_user_param_stru *ku_param)
{
    oal_dlist_head_stru *entry = NULL;
    oal_dlist_head_stru *dlist_tmp = NULL;
    mac_user_stru *mac_user_tmp = NULL;
    hmac_user_stru *hmac_user_tmp = NULL;
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    oal_bool_enum_uint8 en_pmf;

    /* aput dfr恢复后，上层下发广播的去关联命令，但此时aput无用户，不会发deauth帧，辅助机仍是关联状态，
       待辅助机给aput发包时，aput检查无此user，会发deauth去关联。 */
    /* 遍历vap下所有用户, 删除用户 */
    oal_dlist_search_for_each_safe(entry, dlist_tmp, &(mac_vap->st_mac_user_list_head))
    {
        mac_user_tmp = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
        if (mac_user_tmp == NULL) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::mac_user_tmp null.}");
            continue;
        }

        hmac_user_tmp = mac_res_get_hmac_user(mac_user_tmp->us_assoc_id);
        if (oal_unlikely(hmac_user_tmp == NULL)) {
            oam_error_log1(0, OAM_SF_UM, "{hmac_config_kick_user::null user id[%d].}", mac_user_tmp->us_assoc_id);
            continue;
        }

        /* 管理帧加密是否开启 */
        en_pmf = mac_user_tmp->st_cap_info.bit_pmf_active;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame(mac_vap, mac_user_tmp->auc_user_mac_addr, ku_param->us_reason_code, en_pmf);

        /* 修改 state & 删除 user */
        hmac_handle_disconnect_rsp(hmac_vap, hmac_user_tmp, ku_param->us_reason_code);

        /* 删除用户 */
        hmac_user_del(mac_vap, hmac_user_tmp);
    }

    /* VAP下user头指针不应该为空 */
    if (oal_dlist_is_empty(&mac_vap->st_mac_user_list_head) == OAL_FALSE) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::mac_user_list_head is not empty.}");
    }
}

/*
 * 功能描述  : 删用户超时定时器超时处理
 */
static uint32_t hmac_kick_user_timeout(void *arg)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)arg;
    if (hmac_user == NULL) {
        oam_error_log0(0, OAM_SF_UM, "{hmac_kick_user_timeout:hmac_user is NULL!}");
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->st_user_base_info.uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_UM, "{hmac_kick_user_timeout:hmac_vap is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_UM,
        "{hmac_kick_user_timeout:del user[%d]}", hmac_user->st_user_base_info.us_assoc_id);

    /* 抛事件上报内核，已经去关联某个user */
    hmac_handle_disconnect_rsp(hmac_vap, hmac_user, hmac_user->assoc_err_code);

    /* 删除用户 */
    hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    return OAL_SUCC;
}

/*
 * 功能描述  : 发起去关联一个用户,启动定时器。
 */
static void hmac_start_kick_user_timer(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    uint32_t kick_user_timeout;

    /* 去关联定时器不重复启动, 以第一次定时为准 */
    if (hmac_user->kick_user_timer.en_is_registerd == OAL_TRUE) {
        return;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    if (mac_is_chba_mode(&hmac_vap->st_vap_base_info) == OAL_TRUE) {
        kick_user_timeout = MAC_CHBA_DEFALUT_PERIODS_MS;
    } else
#endif
    if (is_ap(&hmac_vap->st_vap_base_info) == OAL_TRUE) {
        kick_user_timeout = 500;    /* AP模式最多等待500ms */
    } else {
        kick_user_timeout = 100;    /* STA模式最多等待100ms */
    }

    /* 启动一个去关联定时器, 等待去关联完成 */
    frw_timer_create_timer_m(&(hmac_user->kick_user_timer), hmac_kick_user_timeout,
        kick_user_timeout, (void *)hmac_user, OAL_FALSE, OAM_MODULE_ID_HMAC,
        hmac_vap->st_vap_base_info.core_id);
}

static void hmac_kick_user_send_disassoc_or_deauth(mac_vap_stru *mac_vap, uint8_t *da,
    uint16_t reason_code, oal_bool_enum_uint8 is_protected)
{
    /* WPA3认证用例4.8.2 检查aput模式删用户，需要发送去认证帧 */
    if (is_legacy_ap(mac_vap) && !mac_is_chba_mode(mac_vap) &&
        mac_vap->sae_pwe != SAE_PWE_UNSPECIFIED) {
        hmac_mgmt_send_deauth_frame(mac_vap, da, reason_code, is_protected);
    } else {
        hmac_mgmt_send_disassoc_frame(mac_vap, da, reason_code, is_protected);
    }
}

static uint32_t hmac_config_kick_process(hmac_vap_stru *hmac_vap, mac_vap_stru *mac_vap, uint16_t user_idx,
    mac_cfg_kick_user_param_stru *kick_user)
{
    hmac_user_stru *hmac_user = NULL;
    mac_user_stru *mac_user = NULL;
    oal_bool_enum_uint8 en_pmf;
    uint16_t us_reason_code = kick_user->us_reason_code;

    hmac_user = mac_res_get_hmac_user(user_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::user[%d] is null.}", user_idx);
        hmac_fsm_change_state_check(hmac_vap, MAC_VAP_STATE_BUTT, MAC_VAP_STATE_STA_FAKE_UP);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_user->kick_user_timer.en_is_registerd == OAL_TRUE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_UM,
            "{hmac_config_kick_user::user[%d] kick user timer have registerd.}", user_idx);
        return OAL_SUCC;
    }

    mac_user = &hmac_user->st_user_base_info;
    if (mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_UM, "{hmac_config_kick_user::user[%d] is unassociated.}", user_idx);
    }

    en_pmf = mac_user->st_cap_info.bit_pmf_active;

    if (kick_user->send_disassoc_immediately == OAL_TRUE) {
        hmac_kick_user_send_disassoc_or_deauth(mac_vap, mac_user->auc_user_mac_addr, us_reason_code, en_pmf);
    }

    /* 设置错误码信息，EAP首次鉴权失败 */
    hmac_user->assoc_err_code = us_reason_code;

    /* aput和chba 启动删除用户定时器，延迟删除用户 */
    if (is_legacy_ap(&hmac_vap->st_vap_base_info) == OAL_TRUE ||
        mac_is_chba_mode(&hmac_vap->st_vap_base_info) == OAL_TRUE) {
        hmac_start_kick_user_timer(hmac_vap, hmac_user);
        return OAL_SUCC;
    }

    /* 修改 state & 删除 user */
    hmac_handle_disconnect_rsp(hmac_vap, hmac_user, us_reason_code);
    /* 删除用户 */
    hmac_user_del(mac_vap, hmac_user);
    return OAL_SUCC;
}
/*
 * 功能描述  : 配置命令去关联1个用户
 */
uint32_t hmac_config_kick_user(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *param)
{
    mac_cfg_kick_user_param_stru *ku_param = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint16_t user_idx = 0;
    uint32_t ret;

    /* 检查参数是否合法 */
    ret = hmac_config_kick_user_check_valid(mac_vap, param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ku_param = (mac_cfg_kick_user_param_stru *)param;
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_UM,
        "{hmac_config_kick_user::kick user mac[%02X:XX:XX:XX:%02X:%02X] reason code[%d]}",
        ku_param->auc_mac_addr[0], ku_param->auc_mac_addr[BYTE_OFFSET_4],
        ku_param->auc_mac_addr[BYTE_OFFSET_5], ku_param->us_reason_code);
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_kick_user::null param,hmac_vap[%d].}", mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 踢掉全部user */
    if (oal_is_broadcast_ether_addr(ku_param->auc_mac_addr)) {
        hmac_config_kick_all_user(hmac_vap, ku_param);
        return OAL_SUCC;
    }

    ret = hmac_get_kicking_user_idx(mac_vap, ku_param->auc_mac_addr, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_UM,
            "{hmac_config_kick_user::find_user_by_addr failed[%d], vap state[%d].}", ret, mac_vap->en_vap_state);
        if (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
            user_idx = mac_vap->us_assoc_vap_id;
        } else {
            hmac_fsm_change_state_check(hmac_vap, MAC_VAP_STATE_BUTT, MAC_VAP_STATE_STA_FAKE_UP);
            return ret;
        }
    }

    return hmac_config_kick_process(hmac_vap, mac_vap, user_idx, ku_param);
}

/*
 * 功能描述  : STA断联前，发送disassoc帧
 */
uint32_t hmac_config_send_disassoc(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_cfg_kick_user_param_stru *kick_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint16_t user_idx = 0;
    uint32_t ret;

    if (oal_any_null_ptr2(mac_vap, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    kick_user = (mac_cfg_kick_user_param_stru *)param;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_send_disassoc::hmac_vap[%d] is null.}",
            mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_get_kicking_user_idx(mac_vap, kick_user->auc_mac_addr, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_UM,
            "{hmac_config_send_disassoc::find_user_by_addr failed[%d], vap state[%d], mac_addr xx:xx:xx:xx:%02x:%02x.}",
            ret, mac_vap->en_vap_state,
            kick_user->auc_mac_addr[MAC_ADDR_4], kick_user->auc_mac_addr[MAC_ADDR_5]);
        return ret;
    }

    hmac_user = mac_res_get_hmac_user(user_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_UM,
            "{hmac_config_send_disassoc::user[%d] is null.}", user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_kick_user_send_disassoc_or_deauth(mac_vap, kick_user->auc_mac_addr,
        kick_user->us_reason_code,
        hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_max_user
 * 功能描述  : 获取最大用户数
 * 1.日    期  : 2014年4月30日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_max_user(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    *((int32_t *)puc_param) = mac_mib_get_MaxAssocUserNums(pst_mac_vap);
    *pus_len = sizeof(int32_t);

    oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_get_max_user::chip us_user_num_max[%d], us_user_nums_max[%d].}",
                     mac_chip_get_max_asoc_user(pst_mac_vap->uc_chip_id), mac_mib_get_MaxAssocUserNums(pst_mac_vap));

    return OAL_SUCC;
}
