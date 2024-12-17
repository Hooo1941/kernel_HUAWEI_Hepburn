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
#include "hmac_roam_scan.h"
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
#include "wal_linux_cfg80211.h"

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
#ifdef _PRE_WLAN_FEATURE_PWL
#include "hmac_pwl.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11R
#include "hmac_11r.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_FRAME_C

/*
 * 函 数 名  : hmac_config_send_2040_coext
 * 功能描述  : 发送20/40共存管理帧
 * 1.日    期  : 2014年3月26日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_send_2040_coext(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_set_2040_coexist_stru *pst_2040_coexist = NULL;
    oal_netbuf_stru *pst_netbuf = NULL;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint32_t ret;
    uint16_t us_frame_len;

    /* 只有STA需要处理 */
    if (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_2040,
                         "hmac_config_send_2040_coext::en_vap_mode is[%d] not STAUT,return", mac_vap->en_vap_mode);
        return OAL_SUCC;
    }

    pst_2040_coexist = (mac_cfg_set_2040_coexist_stru *)puc_param;
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_2040, "hmac_config_send_2040_coext::coinfo=%d chan=%d",
                     pst_2040_coexist->coext_info, pst_2040_coexist->channel_report);

    /* 申请管理帧内存 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_2040, "{hmac_config_send_2040_coext::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_netbuf_prev(pst_netbuf) = NULL;
    oal_netbuf_next(pst_netbuf) = NULL;

    /* 封装20/40 共存管理帧 */
    us_frame_len = mac_encap_2040_coext_mgmt(mac_vap, pst_netbuf, (uint8_t)pst_2040_coexist->coext_info,
                                             pst_2040_coexist->channel_report);
    oal_netbuf_put(pst_netbuf, us_frame_len);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    memset_s(pst_tx_ctl, oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    mac_get_cb_mpdu_len(pst_tx_ctl) = us_frame_len;
    ret = mac_vap_set_cb_tx_user_idx(mac_vap, pst_tx_ctl, mac_vap->auc_bssid);
    if (ret != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_2040,
            "(hmac_config_send_2040_coext::fail to find user by %02X:xx:xx:xx:%02X:%02X.}",
            mac_vap->auc_bssid[MAC_ADDR_0], mac_vap->auc_bssid[MAC_ADDR_4], mac_vap->auc_bssid[MAC_ADDR_5]);
    }
    mac_get_cb_wme_ac_type(pst_tx_ctl) = WLAN_WME_AC_MGMT;

    /* 抛事件到DMAC发送管理帧 */
    ret = hmac_tx_mgmt_send_event(mac_vap, pst_netbuf, us_frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);

        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_2040,
                         "{hmac_config_send_2040_coext::hmac_tx_mgmt_send_event failed.}", ret);
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_set_wps_ie
 * 功能描述  : AP 设置WPS 信息元素
 * 1.日    期  : 2014年4月16日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_wps_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_app_ie_stru *pst_wps_ie;
    uint8_t *puc_ie = NULL;
    uint32_t ret;

    pst_wps_ie = (oal_app_ie_stru *)puc_param;

    /* 设置WPS 信息 */
    ret = hmac_config_set_app_ie_to_vap(pst_mac_vap, pst_wps_ie, pst_wps_ie->en_app_ie_type);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::ret=[%d].}", ret);
        return ret;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测beacon 信息中是否有WPS 信息元素 */
    if ((pst_wps_ie->en_app_ie_type == OAL_APP_BEACON_IE) && (pst_wps_ie->ie_len != 0)) {
        puc_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
                                    pst_wps_ie->auc_ie, (int32_t)(pst_wps_ie->ie_len));
        if (puc_ie != NULL) {
            /* 设置WPS 功能使能 */
            mac_mib_set_WPSActive(pst_mac_vap, OAL_TRUE);
            oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::set wps enable.}");
        }
    } else if ((pst_wps_ie->ie_len == 0) && (pst_wps_ie->en_app_ie_type == OAL_APP_BEACON_IE)) {
        mac_mib_set_WPSActive(pst_mac_vap, OAL_FALSE);
        oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_ie::set wps disable.}");
    }

    return ret;
}
#if defined(_PRE_WLAN_FEATURE_HID2D) && defined(_PRE_WLAN_FEATURE_HID2D_PRESENTATION)
/*
 * 函 数 名  : hmac_fix_country_in_hid2d_mode
 * 功能描述  : HiD2D发布会模式下固定国家码
 * 1.日    期  : 2020年04月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_fix_country_in_hid2d_mode(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_device)
{
    mac_regdomain_info_stru *pst_regdomain_info = NULL;
    mac_get_regdomain_info(&pst_regdomain_info);
    if (pst_regdomain_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* HiD2D 发布会场景拦截 */
    /* 如果发布会模式为1，且当前国家码为99，则不允许修改国家码；其他情况会下发修改国家码指令 */
    if (pst_mac_device->is_presentation_mode == OAL_TRUE &&
        (pst_regdomain_info->ac_country[0] == '9' && pst_regdomain_info->ac_country[1] == '9')) {
        oam_warning_log0(0, OAM_SF_DFS, "{Hmac: In HiD2D Presentation Mode, Can not change the country code!!!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif
/*
 * 函 数 名  : hmac_config_set_dfs_params
 * 功能描述  : 设置国家码时初始化雷达参数(函数行数超过50行整改)
 * 1.日    期  : 2020年04月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_set_dfs_params(mac_device_stru *pst_mac_device, mac_regdomain_info_stru *pst_mac_regdom)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    int8_t *pc_current_country = mac_regdomain_get_country();
    pst_mac_device->st_dfs.st_dfs_info.en_dfs_init = OAL_FALSE;
    /* 当前国家码与要设置的国家码不一致，需要重新初始化雷达信道 */
    if ((pst_mac_regdom->ac_country[0] != pc_current_country[0])
        || (pst_mac_regdom->ac_country[1] != pc_current_country[1])) {
        pst_mac_device->st_dfs.st_dfs_info.en_dfs_init = OAL_FALSE;
    }
#endif
}

/*
 * 函 数 名  : hmac_config_set_country_for_dfs
 * 功能描述  : hmac设置国家码中的dfs信息
 * 1.日    期  : 2015年1月18日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_country_for_dfs(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_COUNTRY_FOR_DFS, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_country_for_dfs::hmac_config_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_get_country
 * 功能描述  : hmac读取国际码
 * 1.日    期  : 2013年10月18日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_country(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_regdomain_info_stru *pst_regdomain_info = NULL;
    mac_cfg_get_country_stru *pst_param;

    pst_param = (mac_cfg_get_country_stru *)puc_param;

    mac_get_regdomain_info(&pst_regdomain_info);
    if (pst_regdomain_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_param->ac_country[BYTE_OFFSET_0] = pst_regdomain_info->ac_country[BYTE_OFFSET_0];
    pst_param->ac_country[BYTE_OFFSET_1] = pst_regdomain_info->ac_country[BYTE_OFFSET_1];
    pst_param->ac_country[BYTE_OFFSET_2] = pst_regdomain_info->ac_country[BYTE_OFFSET_2];
    *pus_len = WLAN_COUNTRY_STR_LEN;

    oam_info_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "hmac_config_get_country");

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_set_country
 * 功能描述  : hmac设置国家码
 * 1.日    期  : 2013年10月18日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_country(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = NULL;
    uint32_t ret;
    uint8_t uc_rc_num;
    uint32_t size;
    mac_cfg_country_stru *pst_country_param = (mac_cfg_country_stru *)puc_param;
    mac_regdomain_info_stru *pst_mac_regdom = (mac_regdomain_info_stru *)pst_country_param->p_mac_regdom;
    if (pst_mac_regdom == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::pst_mac_regdom null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (oal_unlikely(pst_mac_device == NULL)) {
        oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
        pst_mac_regdom = NULL;
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_WLAN_FEATURE_HID2D) && defined(_PRE_WLAN_FEATURE_HID2D_PRESENTATION)
    ret = hmac_fix_country_in_hid2d_mode(pst_mac_vap, pst_mac_device);
    if (ret != OAL_SUCC) {
        oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
        pst_mac_regdom = NULL;
        return ret;
    }
#endif

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::sideband_flag[%d].}",
                     pst_mac_regdom->en_regdomain);

    hmac_config_set_dfs_params(pst_mac_device, pst_mac_regdom);

    mac_regdomain_set_country(us_len, puc_param);

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 只有5G 芯片才进行雷达信道初始化 */
    if (pst_mac_device->st_dfs.st_dfs_info.en_dfs_init == OAL_FALSE) {
        hmac_dfs_channel_list_init(pst_mac_device);
        pst_mac_device->st_dfs.st_dfs_info.en_dfs_init = OAL_TRUE;
        hmac_config_ch_status_sync(pst_mac_device);
    }
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    /* 获取管制类的个数 */
    uc_rc_num = pst_mac_regdom->uc_regclass_num;

    /* 计算配置命令 */
    size = (uint32_t)(sizeof(mac_regclass_info_stru) * uc_rc_num + MAC_RD_INFO_LEN);

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_COUNTRY, (uint16_t)size, (uint8_t *)pst_mac_regdom);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_country::send event failed[%d].}", ret);
    }

    /* WAL层抛内存下来，此处释放 */
    if (pst_mac_regdom != NULL) {
        oal_mem_free_m(pst_mac_regdom, OAL_TRUE);
        pst_mac_regdom = NULL;
    }

    return ret;
}

/*
 * 函 数 名  : hmac_test_encap_mgmt_action
 * 功能描述  : 组action管理帧
 * 1.日    期  : 2019年7月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_test_encap_mgmt_action(mac_vap_stru *pst_mac_vap,
                                     uint8_t *puc_buff, uint8_t *puc_param, uint32_t param_len)
{
    oal_netbuf_stru *pst_buffer = (oal_netbuf_stru *)puc_buff;
    uint8_t *puc_mac_header = oal_netbuf_header(pst_buffer);
    uint8_t *puc_payload_addr = mac_get_80211_mgmt_payload(pst_buffer);
    uint8_t *puc_payload_addr_origin = puc_payload_addr;
    mac_cfg_send_frame_param_stru *pst_param = (mac_cfg_send_frame_param_stru *)puc_param;
    uint32_t frame_length = 0;
    uint16_t us_frame_control;
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
    /*                        设置帧头                                      */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    us_frame_control = WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION;
    mac_hdr_set_frame_control(puc_mac_header, us_frame_control);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置地址1 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_param->auc_mac_ra);

    /* 设置地址2为自己的 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    /* 地址3 bssid */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    if (puc_payload_addr == NULL) {
        return frame_length;
    }

    if (memcpy_s(puc_payload_addr, WLAN_MEM_NETBUF_SIZE2,
        pst_param->uc_frame_body, pst_param->uc_frame_body_length) != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "dmac_test_encap_mgmt_action::memcpy fail!");
    }

    puc_payload_addr += pst_param->uc_frame_body_length;
    frame_length = (uint16_t)((puc_payload_addr - puc_payload_addr_origin) + MAC_80211_FRAME_LEN);

    return frame_length;
}

uint32_t hmac_send_frame(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap,
    uint16_t us_assoc_id, oal_bool_enum_uint8 en_is_bcast_addr, mac_cfg_send_frame_param_stru *pst_param)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint32_t frame_len, ret;
    uint32_t len = 0;
    /* 申请帧内存 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_send_frame_timer::cannot alloc netbuff fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    oal_mem_netbuf_trace(pst_netbuf, OAL_TRUE);
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    memset_s(pst_tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));

    /* 组帧 */
    frame_len = hmac_test_encap_mgmt_action(pst_mac_vap, (uint8_t *)(pst_netbuf), (uint8_t*)pst_param, len);
    if (frame_len == 0) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_send_frame_timer::ul_frame_len=0.}");
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    oal_netbuf_put(pst_netbuf, frame_len);

    if (en_is_bcast_addr) {
        mac_get_cb_is_mcast(pst_tx_ctl) = OAL_TRUE;
    }

    mac_get_cb_tx_user_idx(pst_tx_ctl) = us_assoc_id;
    mac_get_cb_wme_ac_type(pst_tx_ctl) = WLAN_WME_AC_MGMT;
    mac_get_cb_mpdu_num(pst_tx_ctl) = 1;
    mac_get_cb_mpdu_len(pst_tx_ctl) = frame_len; /* dmac发送需要的mpdu长度 */

    /* Buffer this frame in the Memory Queue for transmission */
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_netbuf, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_send_frame_timer::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    if (pst_param->uc_frame_cnt < pst_param->uc_pkt_num) {
        frw_timer_create_timer_m(&(pst_mac_device->st_send_frame), hmac_config_send_frame_timer, 10, /* 10ms 触发一次 */
            pst_param, OAL_FALSE, OAM_MODULE_ID_HMAC, pst_mac_device->core_id);
    } else {
        oal_mem_free_m(pst_param, OAL_TRUE);
    }
    return OAL_SUCC;
}

uint32_t hmac_config_send_frame_timer(void *p_arg)
{
    mac_vap_stru *pst_mac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    mac_user_stru *pst_mac_user = NULL;
    mac_cfg_send_frame_param_stru *pst_param = (mac_cfg_send_frame_param_stru *)p_arg;
    uint16_t us_assoc_id = 0;

    oal_bool_enum_uint8 en_is_bcast_addr;

    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_param->uc_vap_idx);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_send_frame_timer::mac_res_get_mac_vap fail}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_is_bcast_addr = oal_is_broadcast_ether_addr(pst_param->auc_mac_ra);
    if (en_is_bcast_addr) {
        us_assoc_id = pst_mac_vap->us_multi_user_idx;
    } else {
        pst_mac_user = mac_vap_get_user_by_addr(pst_mac_vap, pst_param->auc_mac_ra);
        if (pst_mac_user == NULL) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_send_frame_timer::cannot find user by addr}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        us_assoc_id = pst_mac_user->us_assoc_id;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 发送帧计数 */
    pst_param->uc_frame_cnt++;

    return hmac_send_frame(pst_mac_device, pst_mac_vap, us_assoc_id, en_is_bcast_addr, pst_param);
}

/*
 * 函 数 名  : hmac_config_send_frame
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2019年7月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_send_frame(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_send_frame_param_stru *pst_param = NULL;
    mac_device_stru *pst_mac_device = NULL;
    int32_t ret;

    pst_param = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, sizeof(mac_cfg_send_frame_param_stru), OAL_TRUE);
    if (pst_param == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_config_send_frame::melloc err!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = memcpy_s(pst_param, sizeof(mac_cfg_send_frame_param_stru),
                   puc_param, us_len);
    if (ret != EOK) {
        oal_mem_free_m(pst_param, OAL_TRUE);
        oam_error_log1(0, OAM_SF_SCAN, "hmac_config_send_frame::memcpy fail! ret[%d]", ret);
        return OAL_FAIL;
    }

    pst_param->uc_vap_idx = pst_mac_vap->uc_vap_id;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oal_mem_free_m(pst_param, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                     "{hmac_config_send_frame::frame_types = %d send_times = %d body_len=%d.}",
                     pst_param->en_frame_type, pst_param->uc_pkt_num, pst_param->uc_frame_body_length);

    if (pst_param->uc_pkt_num > 0) {
        frw_timer_create_timer_m(&(pst_mac_device->st_send_frame), hmac_config_send_frame_timer, 10, /* 10ms 触发一次 */
                                 pst_param, OAL_FALSE, OAM_MODULE_ID_HMAC, pst_mac_device->core_id);
    } else {
        oal_mem_free_m(pst_param, OAL_TRUE);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_free_connect_param_resource
 * 功能描述  : 释放申请的内核IE资源和wep_key资源
 * 1.日    期  : 2018年9月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_free_connect_param_resource(hmac_conn_param_stru *pst_conn_param)
{
    if (pst_conn_param->puc_wep_key != NULL) {
        oal_free(pst_conn_param->puc_wep_key);
        pst_conn_param->puc_wep_key = NULL;
    }
    if (pst_conn_param->puc_ie != NULL) {
        oal_free(pst_conn_param->puc_ie);
        pst_conn_param->puc_ie = NULL;
    }
}

/*
 * 功能描述  : 限制wlan1在wlan0关联后才能关联，并且不能关联在相同频段。
 * 1.日    期  : 2020年6月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_connect_dual_sta_check(mac_vap_stru *mac_vap, hmac_conn_param_stru *connect_param)
{
    mac_device_stru *mac_dev = NULL;
    mac_vap_stru *another_up_vap = NULL;

    /* 只在双sta模式下，检查wlan1 */
    if ((mac_is_dual_sta_mode() != OAL_TRUE) || (mac_is_secondary_sta(mac_vap) != OAL_TRUE)) {
        return OAL_SUCC;
    }

    mac_dev = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_dev == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_connect_dual_sta_check::dev[%d] ptr is null}", mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    another_up_vap = mac_device_find_another_up_vap(mac_dev, mac_vap->uc_vap_id);
    if (another_up_vap == NULL) {
        /* wlan1先于wlan0入网 */
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_dual_sta_connect_check::wlan1 trying to connect before wlan0!!!}");
        return OAL_FAIL;
    }

    /* wlan0关联了wapi网络，不允许wlan1连接 */
    if (hmac_user_is_wapi_connected(mac_vap->uc_device_id)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_dual_sta_connect_check::wlan1 cannot connect while wlan0 is connected using wapi!!!}");
        return OAL_FAIL;
    }

    if (!mac_is_primary_legacy_vap(another_up_vap)) {
        oam_warning_log2(another_up_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_dual_sta_connect_check::Not primary vap, vap mode[%d] p2p mode[%d]}", another_up_vap->en_vap_mode,
            another_up_vap->en_p2p_mode);
        return OAL_FAIL;
    }

    /* wlan1关联频段和wlan0相同 */
    if (another_up_vap->st_channel.en_band == mac_get_band_by_channel_num(connect_param->uc_channel)) {
        oam_warning_log0(another_up_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_dual_sta_connect_check::wlan1 trying to connect the same band of wlan0!!!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 连接参数判断
 */
uint32_t hmac_config_connect_param_check(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_conn_param_stru *connect_param = NULL;

    if (oal_unlikely(puc_param == NULL || mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_connect:: connect failed, puc_param OR mac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    connect_param = (hmac_conn_param_stru *)puc_param;
    if (us_len != sizeof(hmac_conn_param_stru)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_connect:: connect failed, unexpected param len ! [%d]!}", us_len);
        hmac_free_connect_param_resource(connect_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (!mac_vap_is_up(mac_vap) &&
        (hmac_check_up_vap_num_is_valid(mac_vap) == OAL_FALSE ||
        hmac_res_restriction_is_ap_with_other_vap() == OAL_TRUE)) {
        hmac_free_connect_param_resource(connect_param);
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CHAN, "{hmac_check_up_vap_num_is_valid::vap num is invalid.}");
        return OAL_FAIL;
    }

    if (connect_param->ie_len > WLAN_WPS_IE_MAX_SIZE) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_connect::connect failed, connect_param ie_len[%d] error!}",
            connect_param->ie_len);
        hmac_free_connect_param_resource(connect_param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (hmac_connect_dual_sta_check(mac_vap, connect_param) != OAL_SUCC) {
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC void hmac_roam_check_conflict(hmac_vap_stru *hmac_vap,
    hmac_conn_param_stru *connect_param)
{
    hmac_roam_info_stru *roam_info = NULL;
    uint32_t cur_timestamp = 0;

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info == NULL) {
        return;
    }

    /* 指定bssid不是5G 或者 前一次漫游不是11v触发，不属于频繁漫游问题，记录清零 */
    if (hmac_vap->is_hwrouter != OAL_TRUE ||
        connect_param->uc_channel < WAL_MIN_CHANNEL_5G ||
        roam_info->roam_conflict_info.pre_roam_trigger_type != ROAM_TRIGGER_11V) {
        hmac_roam_alg_clear_repeat_roam_stat(roam_info);
        return;
    }

    /* 上一次漫游是上层指定bssid触发，本次是11v触发，连续三次后认为属于11v频繁漫游 */
    if (roam_info->roam_conflict_info.repeat_11v_roam_cnt >= ROAM_CONFLICT_ALLOWED_NUM) {
        roam_info->roam_conflict_info.repeat_11v_roam_cnt = 0;
        /* roam_info记录拒绝11v漫游的2g bssid */
        memcpy_s(roam_info->roam_conflict_info.reject_bssid, OAL_MAC_ADDR_LEN,
            roam_info->roam_conflict_info.pre_llv_roam_2g_bssid, OAL_MAC_ADDR_LEN);
        return;
    }

    cur_timestamp = (uint32_t)oal_time_get_stamp_ms();
    if (oal_time_get_runtime(roam_info->roam_conflict_info.pre_11v_roam_timestamp, cur_timestamp) <
        ROAM_CONFLICT_INTERVAL) {
        /* 小于10s，认为是一次频繁漫游 */
        roam_info->roam_conflict_info.repeat_11v_roam_cnt++;
    } else {
        /* 不属于频繁漫游问题，记录清零 */
        hmac_roam_alg_clear_repeat_roam_stat(roam_info);
    }
}

static void hmac_process_connect_while_connecting(mac_vap_stru *mac_vap)
{
    hmac_user_stru *hmac_user = NULL;
    oal_bool_enum_uint8 en_is_protected;

    if ((mac_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) &&
        (mac_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC)) {
        hmac_user = mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);
        if (hmac_user != NULL) {
            oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_process_connect_while_connecting:: \
                deauth the connecting AP, vap id=%d, state=%d}", mac_vap->us_assoc_vap_id, mac_vap->en_vap_state);
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_process_connect_while_connecting:: \
                deauth the connecting AP,  %02X:xx:xx:xx:%02X:%02X}",
                hmac_user->st_user_base_info.auc_user_mac_addr[MAC_ADDR_0],
                hmac_user->st_user_base_info.auc_user_mac_addr[MAC_ADDR_4],
                hmac_user->st_user_base_info.auc_user_mac_addr[MAC_ADDR_5]);

            en_is_protected = hmac_user->st_user_base_info.st_cap_info.bit_pmf_active;
            /* 发去认证帧 */
            hmac_mgmt_send_disassoc_frame(mac_vap, hmac_user->st_user_base_info.auc_user_mac_addr,
                                          MAC_DISAS_LV_SS, en_is_protected);
            /* 删除用户 */
            hmac_user_del(mac_vap, hmac_user);
        }
    }
}

oal_bool_enum_uint8 hmac_connect_check_stop_roam(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;

    return (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING ||
        (roam_info != NULL && roam_info->en_main_state == ROAM_MAIN_STATE_SCANING));
}

uint32_t hmac_process_connect_by_vap_state(mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap,
                                           hmac_conn_param_stru *connect_param)
{
    hmac_user_stru *hmac_user = NULL;
    uint32_t ret;
    roam_channel_org_enum scan_type;

    if (hmac_connect_check_stop_roam(hmac_vap) == OAL_TRUE) {
        /* 通知ROAM主状态机, ABORT Roaming FSM */
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_process_connect_by_vap_state:: force roam complete}");
        hmac_roam_connect_complete(hmac_vap, OAL_FAIL);
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
        /* 相同ssid时，走漫游流程 */
        if (!oal_memcmp(mac_mib_get_DesiredSSID(mac_vap), connect_param->auc_ssid, connect_param->uc_ssid_len) &&
            (connect_param->uc_ssid_len == OAL_STRLEN(mac_mib_get_DesiredSSID(mac_vap)))) {
            /*  support wpa_cli/wpa_supplicant reassociate function */
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_process_connect_by_vap_state:: \
                roaming AP with ressoc frame, %02X:XX:XX:XX:%02X:%02X}", connect_param->auc_bssid[MAC_ADDR_0],
                connect_param->auc_bssid[MAC_ADDR_4], connect_param->auc_bssid[MAC_ADDR_5]);

            if (!oal_memcmp(mac_vap->auc_bssid, connect_param->auc_bssid, OAL_MAC_ADDR_LEN)) {
                /* reassociation */
                ret = hmac_roam_start(hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, NULL, ROAM_TRIGGER_APP);
            } else {
                /* 指定bssid尚未老化不用扫描 直接关联。防止down其他vap打断扫描，导致漫游失败。 */
                scan_type = hmac_roam_choose_scan_type_by_bssid(mac_vap, connect_param);

                /* 指定bssid漫游时判断当前场景是否为11v频繁漫游场景 */
                hmac_roam_check_conflict(hmac_vap, connect_param);
                /* roaming */
                ret = hmac_roam_start(hmac_vap, scan_type, OAL_TRUE, connect_param->auc_bssid, ROAM_TRIGGER_BSSID);
            }

            /* reassociation failure need delete user first, and then connect again (MaxDepth) */
            if (ret == OAL_SUCC) {
                hmac_free_connect_param_resource(connect_param);
                return OAL_SUCC;
            } else if (ret == OAL_ERR_CODE_ROAM_INVALID_VAP_STATUS) {
                hmac_free_connect_param_resource(connect_param);
                return OAL_FAIL;
            }
        }

        /* 先删除用户，再connect */
        hmac_user = mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);
        if (hmac_user != NULL) {
            hmac_user_del(mac_vap, hmac_user);
        }
    }

    //  abort connecting
    hmac_process_connect_while_connecting(mac_vap);

    return OAL_SUCC_GO_ON;
}

#ifdef _PRE_WLAN_FEATURE_HS20
/*
 * 1.日    期  : 2016年4月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static uint32_t hmac_interworking_check(hmac_vap_stru *pst_hmac_vap, uint8_t *puc_param)
{
    uint8_t *puc_extend_cap_ie;
    mac_bss_dscr_stru *pst_bss_dscr;

    if (oal_unlikely(oal_any_null_ptr2(pst_hmac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_interworking_check:: check failed, null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_bss_dscr = (mac_bss_dscr_stru *)puc_param;
    if (pst_bss_dscr->mgmt_len < (MAC_80211_FRAME_LEN + MAC_SSID_OFFSET)) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{hmac_interworking_check:: mgmt_len(%d) < (80211_FRAME_LEN+SSID_OFFSET).}",
                         pst_bss_dscr->mgmt_len);
        return OAL_FAIL;
    }

    /* 查找interworking ie */
    puc_extend_cap_ie = mac_find_ie(MAC_EID_EXT_CAPS,
                                    pst_bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN + MAC_SSID_OFFSET,
                                    (int32_t)(pst_bss_dscr->mgmt_len - MAC_80211_FRAME_LEN - MAC_SSID_OFFSET));
    if (puc_extend_cap_ie == NULL) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{hmac_interworking_check:: \
            puc_extend_cap_ie is NULL, the mgmt_len is %d.}", pst_bss_dscr->mgmt_len);
        return OAL_FAIL;
    }

    /*  未检测到interworking能力位，返回fail */
    if (puc_extend_cap_ie[1] < NUM_4_BITS || !(puc_extend_cap_ie[BYTE_OFFSET_5] & 0x80)) {
        pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_is_interworking = OAL_FALSE;
        return OAL_FAIL;
    }

    pst_hmac_vap->st_vap_base_info.st_cap_flag.bit_is_interworking = OAL_TRUE;
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_HS20

#ifdef _PRE_WLAN_FEATURE_WAPI
static uint32_t hmac_config_connect_wapi_proc(mac_vap_stru *mac_vap, hmac_conn_param_stru *connect_param,
    mac_bss_dscr_stru *bss_dscr, mac_conn_security_stru *conn_sec)
{
    mac_device_stru *mac_device;
    bss_dscr->uc_wapi = connect_param->uc_wapi;
    if (bss_dscr->uc_wapi) {
        conn_sec->is_wapi_connect = OAL_TRUE;
        mac_device = mac_res_get_dev(mac_vap->uc_device_id);
        if (mac_device == NULL) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_connect::connect failed, mac_device null! uc_device_id[%d]}", mac_vap->uc_device_id);
            hmac_free_connect_param_resource(connect_param);
            return OAL_ERR_CODE_MAC_DEVICE_NULL;
        }

        if (mac_device_is_p2p_connected(mac_device) == OAL_SUCC && wlan_chip_is_support_hw_wapi() == OAL_FALSE) {
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_connect:: wapi connect failed for p2p having been connected!.}");
            hmac_free_connect_param_resource(connect_param);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}
#endif
/* 获取关联时的加密相关信息 */
static void hmac_connect_set_conn_security(hmac_conn_param_stru *connect_param, mac_conn_security_stru *conn_sec)
{
    int32_t l_ret;
    memset_s(conn_sec, sizeof(mac_conn_security_stru), 0, sizeof(mac_conn_security_stru));
    conn_sec->uc_wep_key_len = connect_param->uc_wep_key_len;
    conn_sec->en_auth_type = connect_param->en_auth_type;
    conn_sec->en_privacy = connect_param->en_privacy;
    conn_sec->st_crypto = connect_param->st_crypto;
    conn_sec->uc_wep_key_index = connect_param->uc_wep_key_index;
    conn_sec->en_mgmt_proteced = connect_param->en_mfp;
    if (conn_sec->uc_wep_key_len > WLAN_WEP104_KEY_LEN) {
        oam_error_log1(0, OAM_SF_ANY,
            "{hmac_connect_set_conn_security:: wep_key_len[%d] > WLAN_WEP104_KEY_LEN!}", conn_sec->uc_wep_key_len);
        conn_sec->uc_wep_key_len = WLAN_WEP104_KEY_LEN;
    }
    if (connect_param->uc_wep_key_len != 0) {
        l_ret = memcpy_s(conn_sec->auc_wep_key, sizeof(conn_sec->auc_wep_key),
            connect_param->puc_wep_key, connect_param->uc_wep_key_len);
        if (l_ret != EOK) {
            oam_warning_log2(0, OAM_SF_WPA, "hmac_connect_set_conn_security::memcpy fail! l_ret[%d], wep_key_len %u",
                l_ret, connect_param->uc_wep_key_len);
        }
    }
#ifdef _PRE_WLAN_FEATURE_PMF
    conn_sec->en_pmf_cap = mac_get_pmf_cap(connect_param->puc_ie, connect_param->ie_len);
    oam_warning_log2(0, OAM_SF_ANY, "{hmac_config_connect:: connect param en_mfp[%d] pmf_cap[%d]!}",
        conn_sec->en_mgmt_proteced, conn_sec->en_pmf_cap);
#endif
    conn_sec->en_wps_enable = OAL_FALSE;
    if (mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
        connect_param->puc_ie, (int32_t)(connect_param->ie_len)) != NULL) {
        conn_sec->en_wps_enable = OAL_TRUE;
    }
}
static void hmac_connect_set_auth_mode(hmac_vap_stru *hmac_vap, mac_conn_security_stru *conn_sec)
{
#ifdef _PRE_WLAN_FEATURE_11R
    uint8_t akm_type;
#endif
    mac_mib_set_AuthenticationMode(&hmac_vap->st_vap_base_info, conn_sec->en_auth_type);

#ifdef _PRE_WLAN_FEATURE_11R
    if (hmac_vap->bit_11r_enable == OAL_TRUE) {
        akm_type = oal_ntoh_32(conn_sec->st_crypto.aul_akm_suite[0]) & 0xFF;
        if (oal_value_eq_any3(akm_type, WLAN_AUTH_SUITE_FT_1X, WLAN_AUTH_SUITE_FT_PSK, WLAN_AUTH_SUITE_FT_SHA256)) {
            mac_mib_set_AuthenticationMode(&hmac_vap->st_vap_base_info, WLAN_WITP_AUTH_FT);
        }
    }
#endif
}
/* 关联流程黑名单处理 */
static uint32_t hmac_connect_blacklist_process(mac_vap_stru *mac_vap, hmac_conn_param_stru *connect_param,
    mac_conn_security_stru *conn_sec, mac_bss_dscr_stru *bss_dscr)
{
    uint32_t ret;
#ifdef _PRE_WLAN_FEATURE_MBO
    /* MBO STA获取AP的MBO IE中表示不允许关联或re-assoc delay的子元素，STA不与(或re-assoc delay time内不与)AP关联 */
    ret = hmac_mbo_check_is_assoc_or_re_assoc_allowed(mac_vap, connect_param, bss_dscr);
    if (ret != OAL_TRUE) {
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_PMF
    hmac_pmf_update_pmf_black_list(mac_vap, bss_dscr, conn_sec->en_mgmt_proteced);
#endif

    /* check bssid blacklist from Framework/WIFI HAL Configuration */
    ret = hmac_blacklist_filter(mac_vap, bss_dscr->auc_bssid, WLAN_MAC_ADDR_LEN);
    if (ret == OAL_TRUE) {
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/* 关联时ssid 信道参数检查 */
static uint32_t hmac_connect_ssid_channel_check_proc(mac_vap_stru *mac_vap, hmac_conn_param_stru *connect_param,
    mac_bss_dscr_stru *bss_dscr)
{
#ifdef _PRE_WLAN_FEATURE_PWL
    /* PWL模式下，beacon携带的ssid是被加密的，为了后面正常使用，拷贝到bss_dscr中 */
    hmac_pwl_add_ssid_ie(mac_vap, connect_param->auc_ssid, connect_param->uc_ssid_len, bss_dscr);
#endif
    if (oal_memcmp(connect_param->auc_ssid, bss_dscr->ac_ssid, (uint32_t)connect_param->uc_ssid_len)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::find the bss failed by ssid.}");
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }

    /*  Magic Link scenario:GO will change channel random,so we have to check channel in connect req */
    if ((connect_param->uc_channel != bss_dscr->st_channel.uc_chan_number) && (connect_param->uc_channel != 0)) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_connect::p2p_mode=%d, connect req channel=%u, bss_dscr chan_number=%u in scan results.}",
            mac_vap->en_p2p_mode, connect_param->uc_channel, bss_dscr->st_channel.uc_chan_number);
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
static void hmac_connect_set_reassoc_flag(hmac_vap_stru *hmac_vap, hmac_conn_param_stru *connect_param,
    mac_bss_dscr_stru *bss_dscr)
{
#ifdef _PRE_WLAN_FEATURE_HS20
    if (oal_memcmp(hmac_vap->st_vap_base_info.auc_bssid, connect_param->auc_bssid, OAL_MAC_ADDR_LEN) ||
        (hmac_interworking_check(hmac_vap, (uint8_t *)bss_dscr))) {
        hmac_vap->bit_reassoc_flag = OAL_FALSE;
    } else {
        hmac_vap->bit_reassoc_flag = OAL_TRUE;
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect:: assoc ap with ressoc frame.}");
    }
    /* passpoint网络中暂不考虑重关联 */
#endif  // _PRE_WLAN_FEATURE_HS20
    hmac_vap->bit_reassoc_flag = OAL_FALSE;
}
/* 设置P2P/WPS IE 信息到 vap 结构体中 */
static uint32_t hmac_connect_app_ie_proc(hmac_vap_stru *hmac_vap, mac_vap_stru *mac_vap,
    hmac_conn_param_stru *connect_param)
{
    oal_app_ie_stru app_ie;
    uint32_t ret;
    int32_t l_ret;
    /* BEGIN: WLAN发送的 assoc request 不携带P2P IE */
    if (is_legacy_vap(mac_vap)) {
        hmac_config_del_p2p_ie(connect_param->puc_ie, &(connect_param->ie_len));
    }
    /* END: WLAN发送的 assoc request 不携带P2P IE */
    app_ie.ie_len = connect_param->ie_len;
    l_ret = memcpy_s(app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, connect_param->puc_ie, app_ie.ie_len);
    if (l_ret != EOK) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::memcpy app_ie fail,err[%d].}", l_ret);
    }
    app_ie.en_app_ie_type = OAL_APP_ASSOC_REQ_IE;
    hmac_free_connect_param_resource(connect_param);
    ret = hmac_config_set_app_ie_to_vap(mac_vap, &app_ie, app_ie.en_app_ie_type);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::set_app_ie_to_vap fail,err[%d].}", ret);
        return ret;
    }
#ifdef _PRE_WLAN_FEATURE_SAE
    hmac_update_sae_connect_param(hmac_vap, app_ie.auc_ie, app_ie.ie_len);
#endif /* _PRE_WLAN_FEATURE_SAE */
    return OAL_SUCC;
}
/* 根据关联信息设置vap的相关能力 */
static uint32_t hmac_connect_set_vap_cap(hmac_vap_stru *hmac_vap, mac_vap_stru *mac_vap, mac_bss_dscr_stru *bss_dscr,
    mac_conn_security_stru *conn_sec)
{
    uint32_t ret;
    uint8_t chain_index;

    mac_mib_set_dot11dtimperiod(mac_vap, bss_dscr->uc_dtim_period);
    /* 设置关联用户的能力信息 */
    mac_vap->us_assoc_user_cap_info = bss_dscr->us_cap_info;
    mac_vap->bit_ap_11ntxbf = (bss_dscr->en_11ntxbf == OAL_TRUE);

    /* 入网选择的热点rssi，同步到dmac传给tpc算法做管理报文tpc */
    conn_sec->c_rssi = bss_dscr->c_rssi;
    for (chain_index = 0; chain_index < HD_EVENT_RF_NUM; chain_index++) {
        conn_sec->rssi[chain_index] = bss_dscr->rssi[chain_index];
    }

    ret = mac_vap_init_privacy(mac_vap, conn_sec);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::mac_vap_init_privacy fail[%d]!}", ret);
        return ret;
    }
#ifdef _PRE_WINDOWS_SUPPORT
    /* FIPS模式 OS加解密 关闭硬件加解密 */
    hmac_vap->fips = (connect_param->fips == TRUE) ? TRUE : FALSE;
    if (connect_param->fips == TRUE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_connect::Fips is [%d].}", connect_param->fips);
        ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DISABLE_DECR_ENCR, 0, NULL);
        if (ret != OAL_SUCC) {
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_connect::WLAN_CFGID_DISABLE_DECR_ENCR send fail[%d].}", ret);
            return ret;
        }
    }
#endif
    hmac_roam_check_psk(hmac_vap, conn_sec);
    mac_mib_set_WPSActive(mac_vap, conn_sec->en_wps_enable);

#ifdef _PRE_WLAN_FEATURE_11R
    hmac_config_11r_cap(hmac_vap, conn_sec, bss_dscr);
#endif  // _PRE_WLAN_FEATURE_11R
    return OAL_SUCC;
}
static uint32_t hmac_config_connect_other_cap_proc(hmac_vap_stru *hmac_vap, hmac_conn_param_stru *connect_param,
    mac_bss_dscr_stru *bss_dscr, mac_conn_security_stru *conn_sec)
{
    uint32_t ret;
#ifdef _PRE_WLAN_FEATURE_WAPI
    ret = hmac_config_connect_wapi_proc(&hmac_vap->st_vap_base_info, connect_param, bss_dscr, conn_sec);
    if (ret != OAL_SUCC) {
        return ret;
    }
#endif
    ret = hmac_connect_app_ie_proc(hmac_vap, &hmac_vap->st_vap_base_info, connect_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = hmac_connect_set_vap_cap(hmac_vap, &hmac_vap->st_vap_base_info, bss_dscr, conn_sec);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = hmac_check_capability_mac_phy_supplicant(&hmac_vap->st_vap_base_info, bss_dscr);
    if (ret != OAL_SUCC) {
        return ret;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_connect
 * 功能描述  : hmac连接
 * 1.日    期  : 2015年5月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_connect(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    hmac_vap_stru *hmac_vap = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    hmac_conn_param_stru *connect_param = NULL;
    mac_conn_security_stru conn_sec;

    ret = hmac_config_connect_param_check(mac_vap, us_len, puc_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    connect_param = (hmac_conn_param_stru *)puc_param;
    /* 先判断其他VAP 的状态是否允许本VAP 入网连接 */
    /* 如果允许本VAP 入网，则返回设备忙状态           */
    /* 根据内核下发的关联能力，赋值加密相关的mib 值 */
    /* 根据下发的join,提取出安全相关的内容 */
    hmac_connect_set_conn_security(connect_param, &conn_sec);
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_connect::connect failed, hmac_vap null.uc_vap_id[%d]}", mac_vap->uc_vap_id);
        hmac_free_connect_param_resource(connect_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_connect_set_auth_mode(hmac_vap, &conn_sec);

    /* 获取扫描的bss信息 */
    bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(mac_vap, connect_param->auc_bssid);
    if (bss_dscr == NULL) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_connect::find bss failed by bssid:%x:XX:XX:XX:%x:%x}", connect_param->auc_bssid[MAC_ADDR_0],
            connect_param->auc_bssid[MAC_ADDR_4], connect_param->auc_bssid[MAC_ADDR_5]);
        hmac_free_connect_param_resource(connect_param);
        return OAL_FAIL;
    }
    if (hmac_connect_blacklist_process(mac_vap, connect_param, &conn_sec, bss_dscr) != OAL_SUCC) {
        return OAL_FAIL;
    }
    if (hmac_connect_ssid_channel_check_proc(mac_vap, connect_param, bss_dscr) != OAL_SUCC) {
        return OAL_FAIL;
    }
    hmac_connect_set_reassoc_flag(hmac_vap, connect_param, bss_dscr);

    /* 不同的vap state走不同的流程 */
    ret = hmac_process_connect_by_vap_state(mac_vap, hmac_vap, connect_param);
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }
    ret = hmac_config_connect_other_cap_proc(hmac_vap, connect_param, bss_dscr, &conn_sec);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = wlan_chip_send_connect_security_params(mac_vap, &conn_sec);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    return hmac_sta_initiate_join(mac_vap, bss_dscr);
}

uint32_t hmac_set_start_join_bssid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_scanned_bss_info *scanned_bss = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN];
    hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_set_start_join_bssid::hmac_device is null.}");
        return OAL_FAIL;
    }
    if (memcpy_s(auc_bssid, WLAN_MAC_ADDR_LEN, puc_param, us_len) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_set_start_join_bssid::bssid memcpy fail.}");
        return OAL_FAIL;
    }
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    /* 根据bss index查找对应的bss dscr结构信息 */
    scanned_bss = hmac_scan_find_scanned_bss_by_bssid(bss_mgmt, auc_bssid, WLAN_MAC_ADDR_LEN);
    if (scanned_bss == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_set_start_join_bssid::find bss failed by index!}");
        return OAL_FAIL;
    }
    bss_dscr = &(scanned_bss->st_bss_dscr_info);
    /* 增加cap info能力 */
    pst_mac_vap->us_assoc_user_cap_info = bss_dscr->us_cap_info;
    return hmac_sta_initiate_join(pst_mac_vap, bss_dscr);
}

/*
 * 函 数 名  : hmac_config_get_tid
 * 功能描述  : 获取最新数据帧的tid
 * 1.日    期  : 2013年10月18日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_get_tid(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_dev;
    mac_cfg_get_tid_stru *pst_tid;

    pst_tid = (mac_cfg_get_tid_stru *)puc_param;
    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_tid->en_tid = pst_mac_dev->en_tid;
    *pus_len = sizeof(pst_tid->en_tid);

    oam_info_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_get_tid::en_tid=%d.}", pst_tid->en_tid);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_list_channel
 * 功能描述  : 输出设备支持的信道列表
 * 1.日    期  : 2013年12月27日,星期五
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_list_channel(mac_vap_stru *pst_mac_vap)
{
    uint8_t chan_num;
    uint8_t chan_idx;
    uint32_t ret;

    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_2_BUTT; chan_idx++) {
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, chan_idx, OAL_FALSE, &chan_num);

            /* 输出2G信道号 */
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_list_channel::2gCHA.NO=%d}", chan_num);
        }
    }
    if (mac_device_band_is_support(pst_mac_vap->uc_device_id, MAC_DEVICE_CAP_5G) == OAL_FALSE) {
        return OAL_SUCC;
    }

    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_5_BUTT; chan_idx++) { /* 36-124 128-196信道 */
        ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, chan_idx, OAL_FALSE);
        if (ret == OAL_SUCC) {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, chan_idx, OAL_FALSE, &chan_num);

#ifdef _PRE_WLAN_FEATURE_DFS
            /* 检测5G 36~120 124~196信道上的DFS雷达标记 */
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_config_list_channel::5gCHA.NO=%d,DFS_REQUIRED[%c]}",
                chan_num, ((mac_is_ch_in_radar_band(MAC_RC_START_FREQ_5, chan_idx) == OAL_TRUE) ? 'Y' : 'N'));
#endif
        }
    }

    return OAL_SUCC;
}

static uint32_t hmac_app_ie_remove(oal_app_ie_stru *app_ie, uint8_t eid)
{
    uint32_t remain_len;
    uint8_t *ie = NULL;
    ie = mac_find_ie(eid, app_ie->auc_ie, (int32_t)app_ie->ie_len);
    if (ie == NULL) {
        return OAL_SUCC;
    }
    if (app_ie->ie_len < ie[1] + MAC_IE_HDR_LEN) {
        return OAL_FAIL;
    }
    app_ie->ie_len -= (uint32_t)(ie[1] + MAC_IE_HDR_LEN);
    if (app_ie->ie_len < (ie - app_ie->auc_ie)) {
        return OAL_FAIL;
    }
    remain_len = app_ie->ie_len - (uint32_t)(ie - app_ie->auc_ie);
    if (memmove_s(ie, remain_len + (ie[1] + MAC_IE_HDR_LEN), ie + (ie[1] + MAC_IE_HDR_LEN), remain_len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_app_ie_remove:memmove fail");
    }
    return OAL_SUCC;
}
static uint32_t hmac_app_ie_remove_ext_caps(oal_app_ie_stru *app_ie)
{
    return hmac_app_ie_remove(app_ie, MAC_EID_EXT_CAPS);
}
static uint32_t hmac_app_ie_remove_operating_class(mac_vap_stru *mac_vap, oal_app_ie_stru *app_ie)
{
    if (mac_mib_get_dot11ExtendedChannelSwitchActivated(mac_vap) == OAL_TRUE) {
        return OAL_SUCC;
    }
    return hmac_app_ie_remove(app_ie, MAC_EID_OPERATING_CLASS);
}
static uint32_t hmac_app_ie_remove_specified_ie(mac_vap_stru *mac_vap, oal_app_ie_stru *app_ie)
{
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_app_ie_to_vap::hmac_vap null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* remove type 为1表示移除该IE，0为恢复IE，不处理即可，支持处理多个IE需在此扩展 */
    if (hmac_vap->st_remove_ie.uc_type == OAL_FALSE) {
        return OAL_SUCC;
    }
    return hmac_app_ie_remove(app_ie, hmac_vap->st_remove_ie.uc_eid);
}

OAL_STATIC uint32_t hmac_fill_app_ie_event(
    frw_event_stru *event, oal_netbuf_stru *netbuf_app_ie, oal_app_ie_stru *app_ie, uint16_t app_ie_len)
{
    uint8_t *param = NULL;
    dmac_tx_event_stru *app_ie_event = NULL;
    memset_s(oal_netbuf_cb(netbuf_app_ie), OAL_TX_CB_LEN, 0, OAL_TX_CB_LEN);
    param = (uint8_t *)(oal_netbuf_data(netbuf_app_ie));
    if (memcpy_s(param, app_ie_len, (uint8_t *)app_ie, app_ie_len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_h2d_send_app_ie::memcpy fail!");
        return OAL_FAIL;
    }

    app_ie_event = (dmac_tx_event_stru *)event->auc_event_data;
    app_ie_event->pst_netbuf = netbuf_app_ie;
    app_ie_event->us_frame_len = app_ie_len;
    app_ie_event->us_remain = 0;
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_config_h2d_send_app_ie
 * 功能描述  : 发送app ie到dmac
 * 1.日    期  : 2017年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_h2d_send_app_ie(mac_vap_stru *mac_vap, oal_app_ie_stru *app_ie)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    oal_netbuf_stru *netbuf_app_ie = NULL;
    uint16_t frame_len;
    uint32_t ret;
    uint8_t app_ie_header_len;

    if (oal_any_null_ptr2(mac_vap, app_ie)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::param is NULL.}");
        return;
    }
    /* 帧长校验 */
    app_ie_header_len = sizeof(oal_app_ie_stru) - sizeof(app_ie->auc_ie) / sizeof(app_ie->auc_ie[0]);
    frame_len = app_ie_header_len + app_ie->ie_len;
    if (frame_len >= WLAN_LARGE_NETBUF_SIZE || app_ie->ie_len > WLAN_WPS_IE_MAX_SIZE) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_h2d_send_app_ie::frame_len =[%d] ie_len=[%d] invalid.}", frame_len, app_ie->ie_len);
        return;
    }
    /* 申请netbuf内存  */
    netbuf_app_ie = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, frame_len, OAL_NETBUF_PRIORITY_MID);
    if (netbuf_app_ie == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::netbuf_app_ie alloc fail}");
        return;
    }
    /* 申请event 事件内存    */
    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oal_netbuf_free(netbuf_app_ie);
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie::event_mem alloc fail}");
        return;
    }

    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX,
        DMAC_WLAN_CTX_EVENT_SUB_TYPE_APP_IE_H2D, sizeof(dmac_tx_event_stru),
        FRW_EVENT_PIPELINE_STAGE_1, mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);
    if (hmac_fill_app_ie_event(event, netbuf_app_ie, app_ie, frame_len) != OAL_SUCC) {
        oal_netbuf_free(netbuf_app_ie);
        frw_event_free_m(event_mem);
        return;
    }
    ret = frw_event_dispatch_event(event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_h2d_send_app_ie:: dispatch failed, \
            app_ie_type=[%d], app_ie_len=[%d].}", app_ie->en_app_ie_type, app_ie->ie_len);
    }
    oal_netbuf_free(netbuf_app_ie);
    frw_event_free_m(event_mem);
}

/*
 * 函 数 名  : hmac_config_set_app_ie_to_vap
 * 功能描述  : 将用户态 IE 信息拷贝到内核态中
 * 1.日    期  : 2014年4月15日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_app_ie_to_vap(mac_vap_stru *mac_vap, oal_app_ie_stru *app_ie, en_app_ie_type_uint8 en_type)
{
    uint32_t ret;

    if (oal_any_null_ptr2(mac_vap, app_ie)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_app_ie_to_vap::scan failed, mac_vap or app_ie null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 移除驱动侧重复MAC_EID_EXT_CAPS */
    if (hmac_app_ie_remove_ext_caps(app_ie) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /*  该问题通过修复 hipriv命令保留后续使用 */
    if (hmac_app_ie_remove_specified_ie(mac_vap, app_ie) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /*   对比标杆默认不携带该IE 修改通过判断MIB值过滤掉EID 59 */
    if (hmac_app_ie_remove_operating_class(mac_vap, app_ie) != OAL_SUCC) {
        return OAL_FAIL;
    }
    ret = mac_vap_save_app_ie(mac_vap, app_ie, en_type);
    if (ret != OAL_SUCC) {
        oam_error_log3(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_app_ie_to_vap::save app ie failed[%d], \
                       en_type[%d], len[%d].}", ret, en_type, app_ie->ie_len);
        return ret;
    }

    if (app_ie->en_app_ie_type >= OAL_APP_ASSOC_REQ_IE) {
        /* 只有OAL_APP_BEACON_IE、OAL_APP_PROBE_REQ_IE、OAL_APP_PROBE_RSP_IE 才需要保存到device */
        return OAL_SUCC;
    }
    hmac_config_h2d_send_app_ie(mac_vap, app_ie);
    return ret;
}

/*
 * 函 数 名  : hmac_config_set_wps_p2p_ie_param_check
 * 功能描述  : 校验参数，为降低代码行数
 * 1.日    期  : 2020年1月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC oal_bool_enum_uint8 hmac_config_set_wps_p2p_ie_param_check(mac_vap_stru *pst_mac_vap,
                                                                      oal_w2h_app_ie_stru *pst_w2h_wps_p2p_ie)
{
    if ((pst_w2h_wps_p2p_ie->en_app_ie_type >= OAL_APP_IE_NUM) ||
        (pst_w2h_wps_p2p_ie->ie_len >= WLAN_WPS_IE_MAX_SIZE)) {
        oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wps_p2p_ie::app_ie_type=[%d] app_ie_len=[%d],param invalid.}",
                         pst_w2h_wps_p2p_ie->en_app_ie_type, pst_w2h_wps_p2p_ie->ie_len);
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

/*
 * 函 数 名  : hmac_config_set_wps_p2p_ie
 * 功能描述  : mp12 设置WPS/P2P 信息元素
 * 1.日    期  : 2014年11月26日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_wps_p2p_ie(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    oal_app_ie_stru app_ie;
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    uint32_t delta_time;
    oal_w2h_app_ie_stru *w2h_wps_p2p_ie = (oal_w2h_app_ie_stru *)puc_param;

    if (hmac_config_set_wps_p2p_ie_param_check(mac_vap, w2h_wps_p2p_ie) == OAL_FALSE) {
        return OAL_FAIL;
    }

    /* 如果wal to hmac的事件超时，则pst_w2h_wps_p2p_ie->puc_data_ie指向的内存已经释放，不能再处理该事件 */
    delta_time = oal_time_get_runtime(w2h_wps_p2p_ie->delivery_time, oal_time_get_stamp_ms());
    if (delta_time >= (HMAC_WAIT_EVENT_RSP_TIME * HMAC_S_TO_MS)) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_wps_p2p_ie::schedule event timeout[%d]ms, \
            delta_time[%d]ms, app_ie_type[%d] app_ie_len[%d].}", HMAC_WAIT_EVENT_RSP_TIME * HMAC_S_TO_MS, delta_time,
            w2h_wps_p2p_ie->en_app_ie_type, w2h_wps_p2p_ie->ie_len);
        return OAL_FAIL;
    }

    if (!hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH)) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_set_wps_p2p_ie::p2p_ie_type=[%d],p2p_ie_len=[%d].}",
                         w2h_wps_p2p_ie->en_app_ie_type, w2h_wps_p2p_ie->ie_len);
    }

    memset_s(&app_ie, sizeof(app_ie), 0, sizeof(app_ie));
    app_ie.en_app_ie_type = w2h_wps_p2p_ie->en_app_ie_type;
    app_ie.ie_len = w2h_wps_p2p_ie->ie_len;
    if (memcpy_s(app_ie.auc_ie, WLAN_WPS_IE_MAX_SIZE, w2h_wps_p2p_ie->puc_data_ie, app_ie.ie_len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_wps_p2p_ie::memcpy fail!");
        return OAL_FAIL;
    }

    /* 设置WPS/P2P 信息 */
    ret = hmac_config_set_app_ie_to_vap(mac_vap, &app_ie, app_ie.en_app_ie_type);
    if (ret != OAL_SUCC) {
        return ret;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_p2p_ie::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测beacon 信息中是否有WPS 信息元素 */
    if (app_ie.en_app_ie_type == OAL_APP_BEACON_IE) {
        if (app_ie.ie_len != 0 && mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
            app_ie.auc_ie, (int32_t)(app_ie.ie_len)) != NULL) {
            /* 设置WPS 功能使能 */
            mac_mib_set_WPSActive(mac_vap, OAL_TRUE);
            oam_info_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_wps_p2p_ie::set wps enable.}");
        } else {
            mac_mib_set_WPSActive(mac_vap, OAL_FALSE);
        }
    }

    return ret;
}
/*
 * 函 数 名  : hmac_config_addba_req
 * 功能描述  : 建立BA会话的配置命令
 * 1.日    期  : 2013年6月7日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_addba_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_addba_req_param_stru *pst_addba_req = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap;
    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */
    oal_bool_enum_uint8 en_ampdu_support;

    pst_addba_req = (mac_cfg_addba_req_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_addba_req::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户对应的索引 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_addba_req->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_addba_req::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 建立BA会话，是否需要判断VAP的AMPDU的支持情况，因为需要实现建立BA会话时，一定发AMPDU */
    en_ampdu_support = hmac_user_xht_support(pst_hmac_user);
    /* 手动创建ba会话，不受其他额外限制 */
    if (en_ampdu_support) {
        /*
            建立BA会话时，st_action_args(ADDBA_REQ)结构各个成员意义如下
            (1)uc_category:action的类别
            (2)uc_action:BA action下的类别
            (3)arg1:BA会话对应的TID
            (4)arg2:BUFFER SIZE大小
            (5)arg3:BA会话的确认策略
            (6)arg4:TIMEOUT时间
        */
        st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
        st_action_args.uc_action = MAC_BA_ACTION_ADDBA_REQ;
        st_action_args.arg1 = pst_addba_req->uc_tidno;     /* 该数据帧对应的TID号 */
        st_action_args.arg2 = pst_addba_req->us_buff_size; /* ADDBA_REQ中，buffer_size的默认大小 */
        st_action_args.arg3 = pst_addba_req->en_ba_policy; /* BA会话的确认策略 */
        st_action_args.arg4 = pst_addba_req->us_timeout;   /* BA会话的超时时间设置为0 */

        /* 建立BA会话 */
        hmac_mgmt_tx_action(pst_hmac_vap, pst_hmac_user, &st_action_args);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_delba_req
 * 功能描述  : 删除BA会话的配置命令(相当于接收到DELBA帧)
 * 1.日    期  : 2013年6月7日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_delba_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_delba_req_param_stru *pst_delba_req = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */
    hmac_tid_stru *pst_hmac_tid = NULL;

    pst_delba_req = (mac_cfg_delba_req_param_stru *)puc_param;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    /* 获取用户对应的索引 */
    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, pst_delba_req->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_delba_req::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_tid = &(pst_hmac_user->ast_tid_info[pst_delba_req->uc_tidno]);

    /* 查看会话是否存在 */
    if (pst_delba_req->en_direction == MAC_RECIPIENT_DELBA) {
        if (pst_delba_req->en_trigger == MAC_DELBA_TRIGGER_COMM) {
            pst_hmac_tid->en_ba_handle_rx_enable = OAL_FALSE;
        }

        if (pst_hmac_tid->pst_ba_rx_info == NULL) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_delba_req::the rx hdl is not exist.}");
            return OAL_SUCC;
        }
    } else {
        if (pst_delba_req->en_trigger == MAC_DELBA_TRIGGER_COMM) {
            pst_hmac_tid->en_ba_handle_tx_enable = OAL_FALSE;
        }

        if (pst_hmac_tid->st_ba_tx_info.en_ba_status == DMAC_BA_INIT) {
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_delba_req::the tx hdl is not exist.}");
            return OAL_SUCC;
        }
    }

    /*
        建立BA会话时，st_action_args(DELBA_REQ)结构各个成员意义如下
        (1)uc_category:action的类别
        (2)uc_action:BA action下的类别
        (3)arg1:BA会话对应的TID
        (4)arg2:删除ba会话的发起端
        (5)arg3:删除ba会话的原因
        (6)arg5:ba会话对应的用户
    */
    st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    st_action_args.uc_action = MAC_BA_ACTION_DELBA;
    st_action_args.arg1 = pst_delba_req->uc_tidno;      /* 该数据帧对应的TID号 */
    st_action_args.arg2 = pst_delba_req->en_direction;  /* ADDBA_REQ中，buffer_size的默认大小 */
    st_action_args.arg3 = MAC_QSTA_TIMEOUT;             /* BA会话的确认策略 */
    st_action_args.puc_arg5 = pst_delba_req->auc_mac_addr; /* ba会话对应的user */

    /* 建立BA会话 */
    hmac_mgmt_tx_action(pst_hmac_vap, pst_hmac_user, &st_action_args);

    return OAL_SUCC;
}
