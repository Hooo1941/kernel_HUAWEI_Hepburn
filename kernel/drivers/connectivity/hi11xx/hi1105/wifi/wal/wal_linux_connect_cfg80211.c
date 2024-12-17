/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Linux cfg80211接口
 * 作    者 :
 * 创建日期 : 2012年11月8日
 */

#include "securec.h"
#include "securectype.h"
#include "oal_cfg80211.h"
#include "mac_ie.h"
#include "hmac_p2p.h"
#include "hmac_dfx.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#endif
#include "wal_config_base.h"
#include "wal_cfg_ioctl.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_event.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID              OAM_FILE_ID_WAL_LINUX_CONNECT_CFG80211_C

#define WAL_BCN_BSSID_LENGTH_ADDR  37

static int32_t wal_cfg80211_connect_set_bssid_param(uint8_t *bssid, oal_cfg80211_conn_stru *pst_sme)
{
    /* BEGIN  roaming failure between different encryption methods ap. */
    if (pst_sme->bssid) {
        if (memcpy_s(bssid, OAL_MAC_ADDR_LEN, (uint8_t *)pst_sme->bssid, OAL_MAC_ADDR_LEN) != EOK) {
            return -OAL_EFAIL;
        }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
    } else if (pst_sme->bssid_hint) {
        if (memcpy_s(bssid, OAL_MAC_ADDR_LEN, (uint8_t *)pst_sme->bssid_hint, OAL_MAC_ADDR_LEN) != EOK) {
            return -OAL_EFAIL;
        }
#endif
    } else {
        oam_warning_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_connect_set_bssid_param::bssid and bssid_hint is NULL.}");
        return -OAL_EFAIL;
    }
    /* END  roaming failure between different encryption methods ap. */
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_cfg80211_connect_pre_para_check(oal_wiphy_stru *wiphy,
    oal_net_device_stru *net_device, oal_cfg80211_conn_stru *sme)
{
    if (oal_any_null_ptr3(wiphy, net_device, sme)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_connect::connect failed, input params null!}\r\n");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_connect:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SNIFFER
OAL_STATIC int32_t wal_cfg80211_connect_sniffer_handle(oal_net_device_stru *net_device)
{
    hmac_device_stru *hmac_device = NULL;
    mac_vap_stru *mac_vap = oal_net_dev_priv(net_device);

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_connect_sniffer_handle::pst_mac_vap null!}");
        return -OAL_EFAIL;
    }

    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_connect_sniffer_handle::hmac_device null device_id:%d!}",
            mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }

    if (hmac_device->sniffer_mode != WLAN_SINFFER_OFF) { // sniffer抓包过程不处理关联请求
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_connect_sniffer_handle::sniffer enable, connect abort!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif

OAL_STATIC int32_t wal_cfg80211_connect_init(oal_cfg80211_conn_stru *pst_sme, hmac_conn_param_stru *mac_conn_param)
{
    uint8_t *puc_ie = NULL;
    int32_t ret;
    /* 初始化驱动连接参数 */
    memset_s(mac_conn_param, sizeof(hmac_conn_param_stru), 0, sizeof(hmac_conn_param_stru));

    /* 解析内核下发的 bssid */
    ret = wal_cfg80211_connect_set_bssid_param(mac_conn_param->auc_bssid, pst_sme);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ASSOC, "wal_cfg80211_connect_set_bssid_param:: fail!");
        return ret;
    }
    /* 解析内核下发的 ssid */
    mac_conn_param->uc_ssid_len = (uint8_t)pst_sme->ssid_len;
    ret = memcpy_s(mac_conn_param->auc_ssid, sizeof(mac_conn_param->auc_ssid),
                   (uint8_t *)pst_sme->ssid, mac_conn_param->uc_ssid_len);

    /* 解析内核下发的安全相关参数 */
    /* 设置认证类型 */
    mac_conn_param->en_auth_type = pst_sme->auth_type;

    /* 设置加密能力 */
    mac_conn_param->en_privacy = pst_sme->privacy;

    /* 获取内核下发的pmf是使能的结果 */
    mac_conn_param->en_mfp = pst_sme->mfp;

    /* 获取内核下发的flags */
    mac_conn_param->flags = pst_sme->flags;
    if (pst_sme->channel != NULL) {
        mac_conn_param->uc_channel = (uint8_t)oal_ieee80211_frequency_to_channel(pst_sme->channel->center_freq);
    }

    oam_warning_log4(0, OAM_SF_ANY,
        "{wal_cfg80211_connect::start a new connect, ssid_len[%d], auth_type[%d], privacy[%d], mfp[%d]}\r\n",
        pst_sme->ssid_len, pst_sme->auth_type, pst_sme->privacy, pst_sme->mfp);

    /* 设置加密参数 */
    wal_set_crypto_info(mac_conn_param, pst_sme);
    if ((mac_conn_param->puc_wep_key == NULL) && (pst_sme->key_len > 0) && (pst_sme->key != NULL)) {
        oam_warning_log0(0, OAM_SF_ASSOC, "{wal_cfg80211_connect::puc_wep_key is NULL.}");
        return -OAL_EFAIL;
    }

    /* 设置关联P2P/WPS ie */
    mac_conn_param->ie_len = (uint32_t)(pst_sme->ie_len);
    if ((mac_conn_param->ie_len > 0) && (pst_sme->ie != NULL)) {
        puc_ie = (uint8_t *)oal_memalloc(pst_sme->ie_len);
        if (puc_ie == NULL) {
            oam_error_log1(0, OAM_SF_CFG, "{wal_scan_work_func::puc_ie(%d) alloc mem return null ptr!}",
                           (uint32_t)(pst_sme->ie_len));
            hmac_free_connect_param_resource(mac_conn_param);
            return -OAL_EFAIL;
        }
        ret += memcpy_s(puc_ie, pst_sme->ie_len, (uint8_t *)pst_sme->ie, mac_conn_param->ie_len);
        mac_conn_param->puc_ie = puc_ie;
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ASSOC, "wal_cfg80211_connect::memcpy fail!");
        hmac_free_connect_param_resource(mac_conn_param);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_is_p2p_device
 * 功能描述  : 判断是否为P2P DEVICE .如果是P2P device，则不允许关联。
 * 1.日    期  : 2015年5月5日
  *   修改内容  : 新生成函数
 */
OAL_STATIC oal_bool_enum_uint8 wal_is_p2p_device(oal_net_device_stru *pst_net_device)
{
    mac_vap_stru *pst_mac_vap = oal_net_dev_priv(pst_net_device);
    hmac_vap_stru *pst_hmac_vap = NULL;

    if (pst_mac_vap == NULL) {
        return OAL_TRUE;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if ((pst_hmac_vap != NULL) && (pst_hmac_vap->pst_p2p0_net_device != NULL) &&
        (pst_net_device == pst_hmac_vap->pst_p2p0_net_device)) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

#define WAL_CFG80211_CONNECT_WAIT_SCAN_COMP_TIMEOUT 1 /* 关联等待扫描完成超时时间 */
static void wal_cfg80211_connect_wait_scan_complete(oal_net_device_stru *net_device)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_scan_stru *scan_mgmt = NULL;

    mac_vap = oal_net_dev_priv(net_device);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY,
            "{wal_cfg80211_connect_wait_scan_complete:: mac_vap of net_device is NULL!}");
        return;
    }

    /* 获取hmac vap */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{wal_cfg80211_connect_wait_scan_complete::hmac_vap is null, vap_id[%d]!}",
            mac_vap->uc_vap_id);
        return;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{wal_cfg80211_connect_wait_scan_complete::hmac_device[%d] is null!}",
            mac_vap->uc_device_id);
        return;
    }

    scan_mgmt = &(hmac_device->st_scan_mgmt);

    if (scan_mgmt->en_is_scanning == OAL_TRUE) {
        signed long lefttime;
        uint32_t start_time = (uint32_t)oal_time_get_stamp_ms();

        /* 关联前如果在扫描，等待扫描完成后执行关联。如超时后还未扫描完成，则直接执行关联 */
        lefttime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
            scan_mgmt->en_is_scanning == OAL_FALSE,
            WAL_CFG80211_CONNECT_WAIT_SCAN_COMP_TIMEOUT * OAL_TIME_HZ);
        if (lefttime == 0) {
            /* 超时还没有上报扫描结束 */
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_cfg80211_connect_wait_scan_complete:: wait for %ld ms timeout!}",
                WAL_CFG80211_CONNECT_WAIT_SCAN_COMP_TIMEOUT * HMAC_S_TO_MS);
        } else if (lefttime < 0) {
            /* 定时器内部错误 */
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_cfg80211_connect_wait_scan_complete:: wait for %ld ms error!}",
                WAL_CFG80211_CONNECT_WAIT_SCAN_COMP_TIMEOUT * HMAC_S_TO_MS);
        } else {
            /* 正常结束  */
            uint32_t curr_time = (uint32_t)oal_time_get_stamp_ms();
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                "wal_cfg80211_connect_wait_scan_complete::wait scan complete cost_time[%d]",
                oal_time_get_runtime(start_time, curr_time));
        }
    }
}

/*
 * 功能描述  : 解析内核下发的关联命令，sta启动关联
 */
int32_t wal_cfg80211_connect(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device,
    oal_cfg80211_conn_stru *pst_sme)
{
    hmac_conn_param_stru st_mac_conn_param;
    int32_t ret;

    ret = wal_cfg80211_connect_pre_para_check(pst_wiphy, pst_net_device, pst_sme);
    if (ret != OAL_SUCC) {
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    if (wal_cfg80211_connect_sniffer_handle(pst_net_device) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
#endif

    /* 禁止采用p2p device设备启动关联 */
    if (wal_is_p2p_device(pst_net_device) == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_cfg80211_connect:connect stop, p2p device should not connect.");
        return -OAL_EINVAL;
    }

    ret = wal_cfg80211_connect_init(pst_sme, &st_mac_conn_param);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_set_timeout(WLAN_SLEEP_LONG_CHECK_CNT);
#endif

    /* BEGIN:, 关联前需要取消正在进行的扫描，
     * 避免关联命令下发时vap 处在扫描状态，驱动不执行关联命令，导致关联失败 */
    wal_force_scan_abort_then_scan_comp(pst_net_device);
    /* END:, 关联前需要取消正在进行的扫描，
     * 避免关联命令下发时vap 处在扫描状态，驱动不执行关联命令，导致关联失败 */

    /* 为避免驱动漫游触发的扫描和上层漫游关联冲突，引起关联失败，导致WiFi断联接。
     * 修改为:关联前，等待扫描完成。最长等待1s */
    wal_cfg80211_connect_wait_scan_complete(pst_net_device);

    /* 抛事件给驱动，启动关联 */
    ret = wal_cfg80211_start_connect(pst_net_device, &st_mac_conn_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_connect::wal_cfg80211_start_connect fail %d!}\r\n", ret);
        /* 事件下发失败在WAL释放，下发成功，无论succ fail HMAC会释放 */
        if (oal_value_ne_all2(ret, -OAL_EFAIL, -OAL_ETIMEDOUT)) {
            hmac_free_connect_param_resource(&st_mac_conn_param);
        }
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_disconnect
 * 1.日    期  : 2013年8月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_disconnect(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_net_device, uint16_t us_reason_code)
{
    mac_cfg_kick_user_param_stru st_mac_cfg_kick_user_param;
    int32_t l_ret;
    mac_user_stru *pst_mac_user = NULL;
    mac_vap_stru *pst_mac_vap = NULL;

    if (oal_any_null_ptr2(pst_wiphy, pst_net_device)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::pst_wiphy or pst_netdev is null!}");
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 对于CHBA VAP的特殊启动 */
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) &&
        (oal_strcmp("chba0", pst_net_device->name) == 0)) {
        oam_warning_log0(0, 0, "CHBA: Not suppport such disconnect cmd, please use NL80211_CMD_DEL_STATION.");
        return OAL_SUCC;
    }
#endif

    oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_disconnect:: deauth us_reason_code=%d}", us_reason_code);

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_disconnect:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_net_device);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_disconnect::mac vap has already been del!}");
        oal_io_print("wal_cfg80211_disconnect:: mac vap has already been del!ifname %s\r\n", pst_net_device->name);
        return OAL_SUCC;
    }

    /* 填写和sta关联的ap mac 地址 */
    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id);
    if (pst_mac_user == NULL) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_cfg80211_disconnect::mac_res_get_mac_user pst_mac_user is null, user idx[%d]!}\r\n",
            pst_mac_vap->us_assoc_vap_id);
        return OAL_SUCC;
    }

    /* 解析内核下发的connect参数 */
    memset_s(&st_mac_cfg_kick_user_param, sizeof(mac_cfg_kick_user_param_stru),
             0, sizeof(mac_cfg_kick_user_param_stru));

    /* 解析内核下发的去关联原因  */
    st_mac_cfg_kick_user_param.us_reason_code = us_reason_code;
    st_mac_cfg_kick_user_param.send_disassoc_immediately = OAL_FALSE;
    memcpy_s(st_mac_cfg_kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN,
             pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    /* locally generated置1表明是本地发起去关联，0表明是被对端踢 */
    if (is_legacy_sta(pst_mac_vap)) {
        wal_set_locally_generated(pst_mac_vap->uc_vap_id, OAL_TRUE);
    }

    l_ret = wal_cfg80211_start_disconnect(pst_net_device, &st_mac_cfg_kick_user_param);
    if (l_ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_parse_wpa_wpa2_ie
 * 功能描述  : 解析内核传递过来的beacon信息中的WPA/WPA2 信息元素
 * 1.日    期  : 2013年12月11日
  *   修改内容  : 新生成函数
 */
void wal_parse_wpa_wpa2_ie(oal_beacon_parameters *pst_beacon_info, mac_beacon_param_stru *pst_beacon_param)
{
    oal_ieee80211_mgmt *pst_mgmt = (oal_ieee80211_mgmt *)pst_beacon_info->head;
    uint16_t us_capability_info = pst_mgmt->u.beacon.capab_info;
    uint8_t *puc_ie_tmp = NULL;
    int32_t l_ret = EOK;

    pst_beacon_param->en_privacy = OAL_FALSE;

    if (us_capability_info & WLAN_WITP_CAPABILITY_PRIVACY) {
        pst_beacon_param->en_privacy = OAL_TRUE;

        /* 查找 RSN 信息元素 */
        puc_ie_tmp = mac_find_ie(MAC_EID_RSN, pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (puc_ie_tmp != NULL) {
            l_ret += memcpy_s(pst_beacon_param->auc_rsn_ie, MAC_MAX_RSN_LEN,
                puc_ie_tmp, MAC_IE_HDR_LEN + puc_ie_tmp[1]);
        }

        /* 查找 WPA 信息元素，并解析出认证类型 */
        puc_ie_tmp = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA,
            pst_beacon_info->tail, pst_beacon_info->tail_len);
        if (puc_ie_tmp != NULL) {
            l_ret += memcpy_s(pst_beacon_param->auc_wpa_ie, MAC_MAX_RSN_LEN,
                puc_ie_tmp, MAC_IE_HDR_LEN + puc_ie_tmp[1]);
        }
    }
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_parse_wpa_wpa2_ie::memcpy fail!");
    }
}
/*
 * 功能描述  : 查找内核下发的beacon_info中的wmm ie中wmm uapsd是否使能
 * 1.日    期  : 2015年8月24日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_find_wmm_uapsd(uint8_t *puc_wmm_ie)
{
    /* 判断 WMM UAPSD 是否使能 */
    if (puc_wmm_ie[1] < MAC_WMM_QOS_INFO_POS) {
        return OAL_FALSE;
    }

    if (puc_wmm_ie[MAC_WMM_QOS_INFO_POS] & BIT7) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
/*
 * 功能描述  : 在内核下发的信息字段中解析到是否含有wmm字段，即打开或者关闭wmm
 * 1.日    期  : 2015年7月16日
  *   修改内容  : 新生成函数
 */
uint32_t wal_cfg80211_open_wmm(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_open_wmm::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 针对配置vap做保护 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_cfg80211_open_wmm::this is config vap! can't get info.}");
        return OAL_FAIL;
    }

    /* host侧,开关WMM，更新mib信息位中的Qos位置 */
    return hmac_config_open_wmm(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_parse_wmm_ie
 * 功能描述  : 解析内核传递过来beacon信息中的Wmm信息元素
 * 1.日    期  : 2015年7月16日
  *   修改内容  : 新生成函数
 */
uint32_t wal_parse_wmm_ie(oal_net_device_stru *pst_dev,
                          mac_vap_stru *pst_mac_vap, oal_beacon_parameters *pst_beacon_info)
{
    uint8_t *puc_wmm_ie;
    uint16_t us_len = sizeof(uint8_t);
    uint8_t uc_wmm = OAL_TRUE;
    uint32_t ret;
    uint32_t uapsd = OAL_TRUE;
    uint32_t cmd_id = 0;
    uint32_t offset = 0;
    /*  查找wmm_ie  */
    puc_wmm_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM,
                                    pst_beacon_info->tail, pst_beacon_info->tail_len);
    if (puc_wmm_ie == NULL) {
        /* wmm ie未找到，则说明wmm 关 */
        uc_wmm = OAL_FALSE;
    /*  找到wmm ie，顺便判断下uapsd是否使能 */
    } else {
        /* 判断WMM信息元素后偏移8字节 的bit7位是否为1,1表示uapsd使能 */
        if (OAL_FALSE == wal_find_wmm_uapsd(puc_wmm_ie)) {
            /* 对应UAPSD 关 */
            uapsd = OAL_FALSE;
            oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_parse_wmm_ie::uapsd is disabled!!}");
        }

        /*  解决Go不支持UAPSD的问题 */
        /* 该功能目前按照驱动设置来实现，后续需要结合上层设置 */
        if (is_p2p_go(pst_mac_vap)) {
            uapsd = WLAN_FEATURE_UAPSD_ENABLE_CAP;
        }
        ret = wal_get_cmd_id("uapsd_en_cap", &cmd_id, &offset);
        if (ret != OAL_SUCC) {
            oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "wal_parse_wmm_ie:find uapsd en cmd is fail");
            return ret;
        }
        wal_process_cmd_params(pst_dev, cmd_id, &uapsd);
    }
    /*  wmm 开启/关闭 标记  */
    ret = wal_cfg80211_open_wmm(pst_mac_vap, us_len, &uc_wmm);
    if (ret != OAL_SUCC) {
        ret = OAL_FAIL;
        oam_warning_log0(0, OAM_SF_TX, "{wal_parse_wmm_ie::can not open wmm!}\r\n");
    }

    return ret;
}
#define BEACON_HEAD_BUF_SIZE 256
#define BEACON_TAIL_BUF_SIZE 512
OAL_STATIC int32_t wal_cfg80211_fill_beacon_param_prepare(oal_net_device_stru *pst_netdev, mac_vap_stru *pst_mac_vap,
    oal_beacon_data_stru *pst_beacon_info, mac_beacon_param_stru *pst_beacon_param)
{
    oal_beacon_parameters st_beacon_info_tmp;
    oal_beacon_parameters *beacon_info_tmp = &st_beacon_info_tmp;
    uint8_t *puc_beacon_info_tmp = NULL;
    uint32_t beacon_head_len, beacon_tail_len;
    int32_t l_ret = EOK;

    beacon_head_len = (uint32_t)pst_beacon_info->head_len;
    beacon_tail_len = (uint32_t)pst_beacon_info->tail_len;

    if (beacon_head_len > BEACON_HEAD_BUF_SIZE || beacon_tail_len > BEACON_TAIL_BUF_SIZE) {
        oam_error_log2(0, OAM_SF_ANY, "beacon_head_len[%u],beacon_tail_len[%u]", beacon_head_len, beacon_tail_len);
        return -OAL_EINVAL;
    }

    puc_beacon_info_tmp = (uint8_t *)(oal_memalloc(beacon_head_len + beacon_tail_len));
    if (puc_beacon_info_tmp == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{puc_beacon_info_tmp memalloc failed.}");
        return -OAL_EINVAL;
    }
    l_ret += memcpy_s(puc_beacon_info_tmp, beacon_head_len + beacon_tail_len, pst_beacon_info->head, beacon_head_len);
    l_ret += memcpy_s(puc_beacon_info_tmp + beacon_head_len, beacon_tail_len, pst_beacon_info->tail, beacon_tail_len);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "wal_cfg80211_fill_beacon_param::memcpy fail!");
        oal_free(puc_beacon_info_tmp);
        return -OAL_EINVAL;
    }

    /* 为了复用51的解析接口，将新内核结构中的内容赋值给51接口识别的结构体，进而获取信息元素 */
    memset_s(beacon_info_tmp, sizeof(oal_beacon_parameters), 0, sizeof(oal_beacon_parameters));
    beacon_info_tmp->head = puc_beacon_info_tmp;
    beacon_info_tmp->head_len = (int32_t)beacon_head_len;
    beacon_info_tmp->tail = puc_beacon_info_tmp + beacon_head_len;
    beacon_info_tmp->tail_len = (int32_t)beacon_tail_len;

    /* 获取 WPA/WPA2 信息元素 */
    wal_parse_wpa_wpa2_ie(beacon_info_tmp, pst_beacon_param);

    /* 此接口需要修改，linux上没问题，但是win32有错 */
    if (wal_parse_ht_vht_ie(pst_netdev, pst_mac_vap, beacon_info_tmp, pst_beacon_param) != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{failed parse HT/VHT ie!}");
        oal_free(puc_beacon_info_tmp);
        return -OAL_EINVAL;
    }

    if (beacon_head_len + beacon_tail_len < (WAL_BCN_BSSID_LENGTH_ADDR + 1)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{total len is[%u]}", beacon_head_len + beacon_tail_len);
        return -OAL_EINVAL;
    }
    /* 解析是否隐藏SSID */
    if (puc_beacon_info_tmp[WAL_BCN_BSSID_LENGTH_ADDR] == 0) {
        pst_beacon_param->uc_hidden_ssid = 1;
    }

    /* 配置Wmm信息元素 */
    if (wal_parse_wmm_ie(pst_netdev, pst_mac_vap, beacon_info_tmp) != OAL_SUCC) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::Failed parse wmm ie!}");
        oal_free(puc_beacon_info_tmp);
        return -OAL_EINVAL;
    }
    /* 释放临时申请的内存 */
    oal_free(puc_beacon_info_tmp);
    return OAL_SUCC;
}
/*
 * 功能描述  : 将要下发的修改的beacon帧参数填入到入参结构体中
 */
int32_t wal_cfg80211_fill_beacon_param(oal_net_device_stru *pst_netdev,
    oal_beacon_data_stru *pst_beacon_info, mac_beacon_param_stru *pst_beacon_param)
{
    uint32_t offset, beacon_head_len, beacon_tail_len;
    uint8_t uc_vap_id;
    mac_vap_stru *pst_mac_vap = NULL;
    int32_t l_ret;

    if (oal_any_null_ptr3(pst_netdev, pst_beacon_info, pst_beacon_param)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::netdev or beacon_info, beacon_param is null");
        return -OAL_EINVAL;
    }
    /* 获取vap id */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::pst_mac_vap is null}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    /*****************************************************************************
        1.安全配置ie消息等
    *****************************************************************************/
    if (oal_any_null_ptr2(pst_beacon_info->tail, pst_beacon_info->head)) {
        oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::tail or head is null!}");
        return -OAL_EINVAL;
    }

    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    /* oal_ieee80211_mgmt 前面公共部分size为 MAC_80211_FRAME_LEN(24)  */
    if (pst_beacon_info->head_len < (offset + MAC_80211_FRAME_LEN)) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_fill_beacon_param::pst_beacon_info head_len[%d] error.}",
            pst_beacon_info->head_len);
        return -OAL_EINVAL;
    }

    /* 运算溢出保护 */
    beacon_head_len = (uint32_t)pst_beacon_info->head_len;
    beacon_tail_len = (uint32_t)pst_beacon_info->tail_len;
    if ((beacon_head_len + beacon_tail_len) < beacon_head_len) {
        oam_error_log2(0, 0, "{wal_cfg80211_fill_beacon_param::beacon_head_len:%d, beacon_tail_len:%d. len abnormal.}",
            beacon_head_len, beacon_tail_len);
        return -OAL_EINVAL;
    }

    l_ret = wal_cfg80211_fill_beacon_param_prepare(pst_netdev, pst_mac_vap, pst_beacon_info, pst_beacon_param);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 对日本14信道作特殊判断，只在11b模式下才能启用14，非11b模式 降为11b */
    if ((pst_mac_vap->st_channel.uc_chan_number == 14) && (pst_beacon_param->en_protocol != WLAN_LEGACY_11B_MODE)) {
        oam_error_log1(uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_fill_beacon_param::ch 14 should in 11b, but is %d!}", pst_beacon_param->en_protocol);
        oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_fill_beacon_param::change protocol to 11b!}");
        pst_beacon_param->en_protocol = WLAN_LEGACY_11B_MODE;
    }
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_change_beacon
 * 功能描述  : 修改ap beacon帧配置参数
 * 1.日    期  : 2014年12月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_change_beacon(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    oal_beacon_data_stru *pst_beacon_info)
{
    mac_beacon_param_stru st_beacon_param = {0}; /* beacon info struct */
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    /* 参数合法性检查 */
    if (oal_any_null_ptr3(pst_wiphy, pst_netdev, pst_beacon_info)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon:: \
            pst_wiphy or pst_netdev or pst_beacon_info is null!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    /* 初始化beacon interval 和DTIM_PERIOD 参数 */
    l_ret = wal_cfg80211_fill_beacon_param(pst_netdev, pst_beacon_info, &st_beacon_param);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::failed to fill beacon param, error[%d]}", l_ret);
        return -OAL_EINVAL;
    }

    /* 设置操作类型 */
    st_beacon_param.en_operation_type = MAC_SET_BEACON;

    /* 填写 msg 消息头 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_CONFIG_BEACON, sizeof(mac_beacon_param_stru));

    /* 填写 msg 消息体 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(mac_beacon_param_stru),
                        &st_beacon_param, sizeof(mac_beacon_param_stru))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_change_beacon::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_beacon_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_change_beacon::Fail to start addset beacon, error[%d]}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_cfg80211_change_bss
 * 功能描述  : 修改bss参数信息
 * 1.日    期  : 2014年12月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_change_bss(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev,
    oal_bss_parameters *pst_bss_params)
{
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_print_sched_scan_req_info
 * 功能描述  : 打印上层下发的调度扫描请求信息
 * 1.日    期  : 2015年6月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_cfg80211_print_sched_scan_req_info(oal_cfg80211_sched_scan_request_stru *pst_request)
{
    int8_t ac_tmp_buff[200]; /* 200字节的buff */
    int32_t l_loop;
    int32_t l_ret;

    /* 打印基本参数 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]s,flags[%d],rssi_thold[%d]",
        pst_request->n_channels, pst_request->scan_plans[0].interval, pst_request->flags,
        pst_request->min_rssi_thold);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]s,flags[%d],rssi_thold[%d]",
        pst_request->n_channels, pst_request->interval,
        pst_request->flags,
        pst_request->min_rssi_thold);
#else
    oam_warning_log4(0, OAM_SF_SCAN,
        "wal_cfg80211_print_sched_scan_req_info::channels[%d],interval[%d]s,flags[%d],rssi_thold[%d]",
        pst_request->n_channels, pst_request->interval,
        pst_request->flags,
        pst_request->rssi_thold);
#endif

    /* 打印ssid集合的信息 */
    for (l_loop = 0; l_loop < pst_request->n_match_sets; l_loop++) {
        memset_s(ac_tmp_buff, sizeof(ac_tmp_buff), 0, sizeof(ac_tmp_buff));
        l_ret = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff) - 1,
                           "mactch_sets[%d] info, ssid_len[%u], ssid: %.32s.\n",
                           l_loop, pst_request->match_sets[l_loop].ssid.ssid_len,
                           pst_request->match_sets[l_loop].ssid.ssid);
        if (l_ret < 0) {
            oam_error_log0(0, OAM_SF_SCAN, "wal_cfg80211_print_sched_scan_req_info::snprintf_s error!");
            return;
        }

        oam_print(ac_tmp_buff);
    }

    for (l_loop = 0; l_loop < pst_request->n_ssids; l_loop++) {
        memset_s(ac_tmp_buff, sizeof(ac_tmp_buff), 0, sizeof(ac_tmp_buff));
        l_ret = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff) - 1,
                           "ssids[%d] info, ssid_len[%u], ssid: %.32s.\n",
                           l_loop, pst_request->ssids[l_loop].ssid_len, pst_request->ssids[l_loop].ssid);
        if (l_ret < 0) {
            oam_error_log0(0, OAM_SF_SCAN, "wal_cfg80211_print_sched_scan_req_info::snprintf_s error!");
            return;
        }

        oam_print(ac_tmp_buff);
    }

    return;
}
int32_t wal_cfg80211_sched_scan_start(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev, oal_cfg80211_sched_scan_request_stru *pst_request)
{
    hmac_device_stru *pst_hmac_device = NULL;
    hmac_scan_stru *pst_scan_mgmt = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t ret;

    /* 参数合法性检查 */
    if (oal_any_null_ptr3(pst_wiphy, pst_netdev, pst_request)) {
        oam_error_log0(0, 0, "{wal_cfg80211_sched_scan_start::input param null!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_sched_scan_start:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: pst_mac_vap is null!}");
        return -OAL_EINVAL;
    }

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: pst_mac_device is null!}");
        return -OAL_EINVAL;
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    /* 如果当前设备处于扫描状态，abort当前扫描 */
    if (pst_scan_mgmt->pst_request != NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_start:: device is busy, stop current scan!}");

        wal_force_scan_complete(pst_netdev, OAL_TRUE);
    }

    /* 检测内核下发的需要匹配的ssid集合的个数是否合法 */
    if (pst_request->n_match_sets <= 0) {
        oam_warning_log1(0, OAM_SF_SCAN, "{wal_cfg80211_sched_scan_start::match_sets = %d}", pst_request->n_match_sets);
        return -OAL_EINVAL;
    }

    /* 保存当前的PNO调度扫描请求指针 */
    pst_scan_mgmt->pst_sched_scan_req = pst_request;
    pst_scan_mgmt->en_sched_scan_complete = OAL_FALSE;

    /* 维测打印上层下发的调度扫描请求参数信息 */
    wal_cfg80211_print_sched_scan_req_info(pst_request);
    oam_warning_log1(0, 0, "wal_cfg80211_sched_scan_start::flags[%d]", pst_request->flags);
    wal_cfg80211_set_scan_random_mac(pst_hmac_device, pst_request->flags,
        (uint8_t *)(pst_request->mac_addr), (uint8_t *)(pst_request->mac_addr_mask));
    /* 下发pno扫描请求到hmac */
    ret = wal_cfg80211_start_sched_scan(pst_netdev, pst_request);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "{wal_cfg80211_sched_scan_start::wal_cfg80211_start_sched_scan fail[%d]!}", ret);
        return -OAL_EBUSY;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_add_station
 * 功能描述  : 增加用户
 * 1.日    期  : 2013年11月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_add_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev,
    const uint8_t *puc_mac, oal_station_parameters_stru *pst_sta_parms)
{
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_del_station
 * 功能描述  : 删除用户
 * 1.日    期  : 2013年11月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_del_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    struct station_del_parameters *params
#else
    uint8_t *puc_mac
#endif
    )
{
    mac_vap_stru *pst_mac_vap = NULL;
    mac_cfg_kick_user_param_stru st_kick_user_param;
    int32_t ret = OAL_FAIL;
    uint8_t auc_mac_boardcast[OAL_MAC_ADDR_LEN];
    uint16_t us_reason_code = MAC_INACTIVITY;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    uint8_t *puc_mac;
    if (params == NULL) {
        return -OAL_EFAUL;
    }
    puc_mac = (uint8_t *)params->mac;
    us_reason_code = params->reason_code;
#endif

    if (oal_any_null_ptr2(pst_wiphy, pst_dev)) {
        return -OAL_EFAUL;
    }
    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_station:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    pst_mac_vap = oal_net_dev_priv(pst_dev);
    if (pst_mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_del_station::can't get mac vap from netdevice priv data!}\r\n");
        return -OAL_EFAUL;
    }

    /* 判断是否是AP模式 */
    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_error_log1(pst_mac_vap->uc_vap_id, 0, "{wal_cfg80211_del_station::vap_mode:%d!}", pst_mac_vap->en_vap_mode);
        return -OAL_EINVAL;
    }

    if (puc_mac == NULL) {
        memset_s(auc_mac_boardcast, OAL_MAC_ADDR_LEN, 0xff, OAL_MAC_ADDR_LEN);
        puc_mac = auc_mac_boardcast;
    }
    st_kick_user_param.us_reason_code = us_reason_code;
    st_kick_user_param.send_disassoc_immediately = OAL_TRUE;
    if (EOK != memcpy_s(st_kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac, OAL_MAC_ADDR_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_del_station::memcpy fail!");
    }
    ret = wal_cfg80211_start_disconnect(pst_dev, &st_kick_user_param);
    if (ret != OAL_SUCC) {
        /* 由于删除的时候可能用户已经删除，此时再进行用户查找，会返回错误，输出ERROR打印，修改为warning */
        oam_warning_log1(pst_mac_vap->uc_vap_id, 0, "{wal_cfg80211_del_station::config_kick_user fail[%d]!}", ret);
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_change_station
 * 功能描述  : 删除用户
 * 1.日    期  : 2013年11月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_change_station(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_dev,
    const uint8_t *puc_mac,
    oal_station_parameters_stru *pst_sta_parms)
{
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_fill_station_rate_info
 * 功能描述  : 填写上报sta速率相关信息
 * 1.日    期  : 2020.2.12
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_fill_station_rate_info(oal_station_info_stru *sta_info, oal_station_info_stru *stats)
{
    /* 11ac模式下 由于私有速率的存在内核无法识别 修改为通过legacy上报 更改相应的标志位 */
    if ((stats->txrate.flags & RATE_INFO_FLAGS_VHT_MCS) || (stats->txrate.flags & MAC_RATE_INFO_FLAGS_HE_MCS)) {
        sta_info->txrate.legacy = stats->txrate.legacy;      /* dmac已将单位修改为100kbps */
        /* 清除标志位RATE_INFO_FLAGS_VHT_MCS RATE_INFO_FLAGS_MCS */
        sta_info->txrate.flags = stats->txrate.flags & 0x7C;
    } else {
        sta_info->txrate.legacy = (uint16_t)(stats->txrate.legacy * 10); /* 内核中单位为100kbps */
        sta_info->txrate.flags  = stats->txrate.flags;
    }

    sta_info->txrate.mcs = stats->txrate.mcs;
    sta_info->txrate.nss = stats->txrate.nss;
    sta_info->txrate.bw  = stats->txrate.bw;

    /* 上报接收速率 */
    sta_info->filled |= BIT(NL80211_STA_INFO_RX_BITRATE);
    sta_info->rxrate.legacy = stats->rxrate.legacy;
}

#define QUERY_STATION_INFO_TIME (5 * OAL_TIME_HZ)
/*
 * 函 数 名  : wal_cfg80211_fill_station_info
 * 功能描述  : station_info结构赋值
 * 1.日    期  : 2014年12月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_cfg80211_fill_station_info(oal_station_info_stru *pst_sta_info,
                                               oal_station_info_stru *pst_stats)
{
    /* 适配linux 4.0.修改为BIT(NL80211_STA_INFO_SIGNAL)等. */
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_SIGNAL);
    pst_sta_info->signal = pst_stats->signal;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_PACKETS);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_PACKETS);
    pst_sta_info->rx_packets = pst_stats->rx_packets;
    pst_sta_info->tx_packets = pst_stats->tx_packets;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_BYTES);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_BYTES);
    pst_sta_info->rx_bytes = pst_stats->rx_bytes;
    pst_sta_info->tx_bytes = pst_stats->tx_bytes;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_RETRIES);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_FAILED);
    pst_sta_info->filled |= BIT(NL80211_STA_INFO_RX_DROP_MISC);
    pst_sta_info->tx_retries = pst_stats->tx_retries;
    pst_sta_info->tx_failed = pst_stats->tx_failed;
    pst_sta_info->rx_dropped_misc = pst_stats->rx_dropped_misc;

    pst_sta_info->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);

    wal_fill_station_rate_info(pst_sta_info, pst_stats);
}

OAL_STATIC void wal_cfg80211_fill_station_info_ext(oal_station_info_stru *sta_info,
    hmac_vap_stru *hmac_vap)
{
#ifdef CONFIG_HW_GET_EXT_2_SIG
    uint8_t idx;
    uint8_t i;
#endif
#ifdef CONFIG_HW_GET_EXT_SIG
    sta_info->filled |= BIT(NL80211_STA_INFO_NOISE);
    sta_info->noise = hmac_vap->station_info.noise;

    sta_info->filled |= BIT(NL80211_STA_INFO_SNR);
    sta_info->snr = oal_max(hmac_vap->st_station_info_extend.c_snr_ant0,
        hmac_vap->st_station_info_extend.c_snr_ant1);

    sta_info->filled |= BIT(NL80211_STA_INFO_CNAHLOAD);
    sta_info->chload = hmac_vap->station_info.chload;
#endif

#ifdef CONFIG_HW_GET_EXT_SIG_ULDELAY
    sta_info->filled |= BIT(NL80211_STA_INFO_UL_DELAY);
    sta_info->ul_delay = hmac_vap->station_info.ul_delay;
#endif
    sta_info->filled |= BIT(NL80211_STA_INFO_BEACON_SIGNAL_AVG);
    sta_info->rx_beacon_signal_avg = hmac_vap->beacon_rssi;
#ifdef CONFIG_HW_GET_EXT_2_SIG
    sta_info->filled |= BIT(NL80211_STA_INFO_UL_DELAY_ARRAY);
    if (memcpy_s(sta_info->ul_delay_array, sizeof(sta_info->ul_delay_array),
        hmac_vap->station_info_extend2.uplink_delay,
        sizeof(hmac_vap->station_info_extend2.uplink_delay)) != EOK) {
        return;
    }
    sta_info->filled |= BIT(NL80211_STA_INFO_TX_TIME);
    if (memcpy_s(sta_info->tx_time, sizeof(sta_info->tx_time),
        hmac_vap->station_info_extend2.tx_time,
        sizeof(hmac_vap->station_info_extend2.tx_time)) != EOK) {
        return;
    }
    sta_info->filled |= BIT(NL80211_STA_INFO_BEACON_RSSI);
    if (memset_s(sta_info->beacon_rssi, sizeof(sta_info->beacon_rssi), 0, sizeof(sta_info->beacon_rssi)) != EOK) {
        return;
    }
    /* 若beacon rssi统计个数少于10，需要对index特别处理避免上报顺序出错 */
    idx = hmac_vap->station_info_extend2.last_beacon_idx > NUM_10_BYTES ?
        (hmac_vap->station_info_extend2.last_beacon_idx % NUM_10_BYTES) : 0;
    /* 10: 最近1s收到的beacon rssi，按接收顺序上报 */
    for (i = 0; i < NUM_10_BYTES; i++) {
        sta_info->beacon_rssi[i] = hmac_vap->station_info_extend2.beacon_rssi[idx];
        idx = (idx + 1) % NUM_10_BYTES;
    }
    sta_info->filled |= BIT(NL80211_STA_INFO_CHLOAD_SELF);
    sta_info->chload_self = hmac_vap->station_info_extend2.chload_self;
#endif
}

/*
 * 函 数 名  : wal_cfg80211_get_station_filter
 * 功能描述  : update rssi once a second
 * 1.日    期  : 2015年8月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t wal_cfg80211_get_station_filter(mac_vap_stru *pst_mac_vap, uint8_t *puc_mac)
{
    hmac_user_stru *pst_hmac_user = NULL;
    uint32_t current_time = (uint32_t)oal_time_get_stamp_ms();
    uint32_t runtime;
    uint32_t get_station_threshold;
    mac_device_stru *pst_mac_dev = NULL;

    pst_hmac_user = mac_vap_get_hmac_user_by_addr(pst_mac_vap, puc_mac, WLAN_MAC_ADDR_LEN);
    if (pst_hmac_user == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station_filter::user is null.}");
        return OAL_FALSE;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_dev == NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_get_station_filter::dev is null.}");
        return OAL_FALSE;
    }
    /*  从VOWIFI到VOLTE切换存在语音2-3s断续的问题，
      修改方法:Vowif VOWIFI_DISABLE_REPORT模式下更新事件变为200ms */
    if (is_legacy_sta(pst_mac_vap) &&
        (pst_mac_vap->pst_vowifi_cfg_param != NULL &&
         pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode == VOWIFI_DISABLE_REPORT)) { /* 亮屏且vowifi正在使用时 */
        get_station_threshold = WAL_VOWIFI_GET_STATION_THRESHOLD;
    } else {
        if (!is_legacy_vap(pst_mac_vap)) {
            get_station_threshold = WAL_CAST_SCREEN_GET_STATION_THRESHOLD;
        } else {
            get_station_threshold = WAL_GET_STATION_THRESHOLD;
        }
    }

    runtime = (uint32_t)oal_time_get_runtime(pst_hmac_user->rssi_last_timestamp, current_time);
    if (get_station_threshold > runtime) {
        return OAL_FALSE;
    }

    pst_hmac_user->rssi_last_timestamp = current_time;
    return OAL_TRUE;
}
uint32_t g_get_station_running_time = 0;
OAL_STATIC int32_t wal_cfg80211_get_station_check_left_time(oal_station_info_stru *sta_info,
    hmac_vap_stru *hmac_vap)
{
    signed long i_leftime;
    uint32_t curr_time = 0;

    i_leftime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
    (OAL_TRUE == hmac_vap->station_info_query_completed_flag), QUERY_STATION_INFO_TIME);
    if (i_leftime == 0) {
        /* 超时还没有上报扫描结束 */
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_get_station::query info wait for %ld ms timeout!}",
                         ((QUERY_STATION_INFO_TIME * HMAC_S_TO_MS) / OAL_TIME_HZ));
        return -OAL_EINVAL;
    } else if (i_leftime < 0) {
        /* 定时器内部错误 */
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_get_station::query info wait for %ld ms error!}",
                         ((QUERY_STATION_INFO_TIME * HMAC_S_TO_MS) / OAL_TIME_HZ));
        return -OAL_EINVAL;
    } else {
        /* 正常结束  */
        wal_cfg80211_fill_station_info(sta_info, &hmac_vap->station_info);
        wal_cfg80211_fill_station_info_ext(sta_info, hmac_vap);
        curr_time = (uint32_t)oal_time_get_stamp_ms();
#ifdef CONFIG_HW_GET_EXT_SIG
        oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "wal_cfg80211_get_station::rssi[%d] chload[%d] noise[%d] snr[%d]",
            hmac_vap->station_info.signal, hmac_vap->station_info.chload, hmac_vap->station_info.noise,
            hmac_vap->st_station_info_extend.c_snr_ant0);
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "wal_cfg80211_get_station::cost_time[%d]",
            oal_time_get_runtime(g_get_station_running_time, curr_time));
#else
        oam_warning_log3(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "wal_cfg80211_get_station::rssi[%d] snr[%d] cost_time[%d]",
            hmac_vap->station_info.signal, hmac_vap->st_station_info_extend.c_snr_ant0,
            oal_time_get_runtime(g_get_station_running_time, curr_time));
#endif
        return OAL_SUCC;
    }
}

/*
 * 函 数 名  : wal_cfg80211_get_station
 * 功能描述  : 获取station信息
 * 1.日    期  : 2014年12月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_get_station(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_dev, const uint8_t *puc_mac, oal_station_info_stru *pst_sta_info)
{
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    dmac_query_station_info_request_event *pst_query_station_info = NULL;
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    int32_t l_ret;
    uint8_t uc_vap_id;
    g_get_station_running_time = (uint32_t)oal_time_get_stamp_ms();

    if (oal_any_null_ptr4(pst_wiphy, pst_dev, puc_mac, pst_sta_info)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_station::input params null!}");
        return -OAL_EINVAL;
    }

    pst_mac_vap = oal_net_dev_priv(pst_dev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_station::oal_net_dev_priv, return null!}");
        return -OAL_EINVAL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_get_station::mac_res_get_hmac_vap fail.}");
        return -OAL_EINVAL;
    }

    /* 固定时间内最多更新一次RSSI */
    if (OAL_FALSE == wal_cfg80211_get_station_filter(&pst_hmac_vap->st_vap_base_info, (uint8_t *)puc_mac)) {
        wal_cfg80211_fill_station_info(pst_sta_info, &pst_hmac_vap->station_info);
        wal_cfg80211_fill_station_info_ext(pst_sta_info, pst_hmac_vap);
        return OAL_SUCC;
    }

    pst_query_station_info = (dmac_query_station_info_request_event *)write_msg.auc_value;
    pst_query_station_info->query_event = OAL_QUERY_STATION_INFO_EVENT;
    oal_set_mac_addr(pst_query_station_info->auc_query_sta_addr, (uint8_t *)puc_mac);

    pst_hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    /********************************************************************************
        抛事件到wal层处理 ，对于低功耗需要做额外处理，不能像下层抛事件，直接起定时器
        低功耗会在接收beacon帧的时候主动上报信息。
    ********************************************************************************/
    /* 3.1 填写 msg 消息头 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_QUERY_STATION_STATS, sizeof(dmac_query_station_info_request_event));
    l_ret = wal_send_cfg_event(pst_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(dmac_query_station_info_request_event), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(uc_vap_id, 0, "{wal_cfg80211_get_station::wal_send_cfg_event return err code %d!}", l_ret);
        return -OAL_EFAIL;
    }
    /* 读取返回的错误码 */
    l_ret = wal_check_and_release_msg_resp(rsp_msg);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_cfg80211_get_station fail, err code[%u]!}", l_ret);
        return -OAL_EFAIL;
    }
    return wal_cfg80211_get_station_check_left_time(pst_sta_info, pst_hmac_vap);
}

/*
 * 函 数 名  : wal_cfg80211_dump_station
 * 功能描述  : 删除用户
 * 1.日    期  : 2013年11月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_dump_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev,
    int32_t int_index, uint8_t *puc_mac, oal_station_info_stru *pst_sta_info)
{
    return OAL_SUCC;
}
