/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Linux cfg80211接口
 * 作    者 :
 * 创建日期 : 2012年11月8日
 */

#include "securec.h"
#include "securectype.h"
#include "oal_kernel_file.h"

#include "wlan_chip_i.h"

#include "mac_ie.h"
#include "hmac_p2p.h"
#include "hmac_chan_mgmt.h"
#include "hmac_dfx.h"

#include "wal_config_base.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_cfgvendor.h"
#include "wal_linux_netdev_ops.h"
#include "wal_cfg_ioctl.h"
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
#include "wal_dfx.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID              OAM_FILE_ID_WAL_LINUX_PROTOCOL_CFG80211_C

/*
 * 函 数 名  : wal_check_support_basic_rate_6M
 * 功能描述  : 判断指数速率集和扩展速率集中是否包含6M速率作为基本速率
 * 1.日    期  : 2014年4月23日
  *   修改内容  : 新生成函数
 */
OAL_STATIC oal_bool_enum wal_check_support_basic_rate_6m(uint8_t *puc_supported_rates_ie,
    uint8_t uc_supported_rates_num, uint8_t *puc_extended_supported_rates_ie, uint8_t uc_extended_supported_rates_num)
{
    uint8_t uc_loop;
    oal_bool_enum en_support = OAL_FALSE;
    for (uc_loop = 0; uc_loop < uc_supported_rates_num; uc_loop++) {
        if (puc_supported_rates_ie == NULL) {
            break;
        }
        if (puc_supported_rates_ie[BYTE_OFFSET_2 + uc_loop] == 0x8c) {
            en_support = OAL_TRUE;
        }
    }

    for (uc_loop = 0; uc_loop < uc_extended_supported_rates_num; uc_loop++) {
        if (puc_extended_supported_rates_ie == NULL) {
            break;
        }
        if (puc_extended_supported_rates_ie[BYTE_OFFSET_2 + uc_loop] == 0x8c) {
            en_support = OAL_TRUE;
        }
    }

    return en_support;
}

OAL_STATIC OAL_INLINE uint32_t wal_parse_protocol_mode_rate_nums_check(uint8_t uc_rate_nums,
    uint8_t uc_supported_rates_num, uint8_t uc_extended_supported_rates_num, wlan_protocol_enum_uint8 *pen_protocol)
{
    if (uc_rate_nums < uc_supported_rates_num) {
        oam_error_log2(0, 0, "{wal_parse_protocol_mode::rate_num[%d], ext_rate_num[%d]}",
            uc_supported_rates_num, uc_extended_supported_rates_num);
        *pen_protocol = WLAN_PROTOCOL_BUTT;
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_parse_protocol_mode_by_vht_ht_ie(uint8_t *puc_vht_ie,
    uint8_t *puc_ht_ie, wlan_protocol_enum_uint8 *pen_protocol)
{
    if (puc_vht_ie != NULL) {
        /* 设置AP 为11ac 模式 */
        *pen_protocol = WLAN_VHT_MODE;
        return OAL_SUCC;
    }
    if (puc_ht_ie != NULL) {
        /* 设置AP 为11n 模式 */
        *pen_protocol = WLAN_HT_MODE;
        return OAL_SUCC;
    }
    return OAL_FAIL;
}
static uint32_t wal_get_rate_num_by_bcn_info(oal_beacon_parameters *beacon_info, uint8_t *base_rates_num,
    uint8_t *ext_rates_num)
{
    uint8_t *base_rates_ie = NULL;
    uint8_t *ext_rates_ie = NULL;
    uint16_t offset = BYTE_OFFSET_24 + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    if (beacon_info->head_len < offset) {
        oam_error_log1(0, 0, "wal_get_rate_num_by_bcn_info:head_len < 36", beacon_info->head_len);
        return OAL_FAIL;
    }
    base_rates_ie = mac_find_ie(MAC_EID_RATES, beacon_info->head + offset, beacon_info->head_len - offset);
    if (base_rates_ie != NULL) {
        *base_rates_num = base_rates_ie[1];
    }
    ext_rates_ie = mac_find_ie(MAC_EID_XRATES, beacon_info->tail, beacon_info->tail_len);
    if (ext_rates_ie != NULL) {
        *ext_rates_num = ext_rates_ie[1];
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_parse_protocol_mode
 * 功能描述  : 解析协议模式
 * 1.日    期  : 2014年6月12日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_parse_protocol_mode(wlan_channel_band_enum_uint8 en_band,
    oal_beacon_parameters *beacon_info, uint8_t *puc_ht_ie, uint8_t *puc_vht_ie,
    uint8_t *puc_he_ie, wlan_protocol_enum_uint8 *protocol, uint8_t he_flag)
{
    uint8_t *base_rates_ie = NULL;
    uint8_t *ext_rates_ie = NULL;
    uint8_t base_rates_num = 0;
    uint8_t ext_rates_num = 0;
    uint8_t rate_nums;

    *protocol = WLAN_PROTOCOL_BUTT;
    if ((puc_he_ie != NULL) || (he_flag == OAL_TRUE)) {
#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            /* 设置AP 为11ax模式 */
            *protocol = WLAN_HE_MODE;
            return OAL_SUCC;
        }
#endif
    }
    if (wal_parse_protocol_mode_by_vht_ht_ie(puc_vht_ie, puc_ht_ie, protocol) == OAL_SUCC) {
        return OAL_SUCC;
    }

    /* hostapd 先设置频段，后设置add beacon 因此在add beacon流程中，可以使用频段信息和速率信息设置AP 模式(a/b/g) */
    if (en_band == WLAN_BAND_5G) {
        *protocol = WLAN_LEGACY_11A_MODE;
        return OAL_SUCC;
    }
    if (en_band == WLAN_BAND_2G) {
        if (wal_get_rate_num_by_bcn_info(beacon_info, &base_rates_num, &ext_rates_num) != OAL_SUCC) {
            return OAL_FAIL;
        }
        rate_nums = base_rates_num + ext_rates_num;
        // 此处若直接使用base_rates_num + ext_rates_num起不到长度保护作用，仍会溢出
        if (OAL_FAIL == wal_parse_protocol_mode_rate_nums_check(rate_nums, base_rates_num, ext_rates_num, protocol)) {
            return OAL_FAIL;
        }
        if (rate_nums == 4) { /* 4表示11b的速率 */
            *protocol = WLAN_LEGACY_11B_MODE;
            return OAL_SUCC;
        } else if (rate_nums == 8) { /* 8表示11g的速率 */
            *protocol = WLAN_LEGACY_11G_MODE;
            return OAL_SUCC;
        } else if (rate_nums == 12) { /* 12表示11g混合速率 */
            /* 根据基本速率区分为 11gmix1 还是 11gmix2 */
            /* 如果基本速率集支持 6M , 则判断为 11gmix2 */
            *protocol = WLAN_MIXED_ONE_11G_MODE;
            if (OAL_TRUE == wal_check_support_basic_rate_6m(base_rates_ie, base_rates_num,
                ext_rates_ie, ext_rates_num)) {
                *protocol = WLAN_MIXED_TWO_11G_MODE;
            }
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}
/*
 * 函 数 名  : wal_parse_he_ie
 * 功能描述  : 解析he能力，ul_he_flag为定制化开关控制标识，puc_he_ie为hostapd下发ax能力
 * 1.日    期  : 2019年07月20日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint8_t* wal_parse_he_ie(mac_vap_stru *pst_mac_vap, uint8_t *puc_flag,
    oal_beacon_parameters  *pst_beacon_info)
{
    uint8_t *puc_he_ie = NULL;

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 定制化开关控制mp15 device能力起在ax模式，mp13不起在ax模式 */
        if (g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_switch == OAL_TRUE) {
            *puc_flag = OAL_TRUE;
        }
        /* 通过hostapd下发ax能力 */
        puc_he_ie = mac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_CAP, pst_beacon_info->tail,
            pst_beacon_info->tail_len);
    }
#endif
    return puc_he_ie;
}

/*
 * 功能描述  : 在STA5g，信号桥2g情况下，改变he能力位，使其不起在ax上
 * 1.日    期  : 2019年09月20日
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_he_flag_handle(oal_net_device_stru *pst_netdev, mac_vap_stru *pst_mac_vap,
    uint8_t **ppuc_he_ie, uint8_t *ul_he_flag)
{
    mac_device_stru *p_mac_dev = NULL;
    if (g_wlan_spec_cfg->feature_slave_ax_is_support == OAL_TRUE) {
        return;
    }

    if (*ppuc_he_ie == NULL && *ul_he_flag == OAL_FALSE) {
        return;
    }

    p_mac_dev = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (p_mac_dev == NULL) {
        return;
    }

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_he_flag_handle::p2p_scenes = %d!}\r\n",
        p_mac_dev->st_p2p_info.p2p_scenes);
    if (wal_should_down_protocol_bw(p_mac_dev, pst_mac_vap) == OAL_TRUE &&
        mac_vap_can_not_start_he_protocol(pst_mac_vap) == OAL_TRUE) {
        *ppuc_he_ie = NULL;
        *ul_he_flag = OAL_FALSE;
    }
}

OAL_STATIC void wal_go_protocol_handle(mac_vap_stru *pst_mac_vap, mac_beacon_param_stru *pst_beacon_param)
{
    if (!is_p2p_go(pst_mac_vap)) {
        return;
    }
    /*  如果GO已经是up状态，则维持之前的协议模式，防止被误改与已关联user协议模式不匹配 */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE) {
        pst_beacon_param->en_protocol = pst_mac_vap->en_protocol;
        return;
    }
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (pst_beacon_param->en_protocol == WLAN_HE_MODE) {
            return;
        }
    }
#endif

    if (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        pst_beacon_param->en_protocol =
            ((pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) ? WLAN_VHT_MODE : WLAN_HT_MODE);
    }
    if (mac_vap_avoid_dbac_close_vht_protocol(pst_mac_vap) == OAL_TRUE) {
        pst_beacon_param->en_protocol = WLAN_HT_MODE;
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_go_protocol_handle::close vht = %d!}\r\n",
            pst_beacon_param->en_protocol);
    }
}

/*
 * 函 数 名  : wal_parse_ht_vht_ie
 * 功能描述  : 解析内核传递过来的beacon信息中的ht_vht 信息元素
 * 1.日    期  : 2014年4月4日
  *   修改内容  : 新生成函数
 */
uint32_t wal_parse_ht_vht_ie(oal_net_device_stru *pst_netdev,
    mac_vap_stru *pst_mac_vap, oal_beacon_parameters *pst_beacon_info, mac_beacon_param_stru *pst_beacon_param)
{
    uint8_t *puc_ht_ie, *puc_vht_ie;
    uint8_t *puc_he_ie = NULL;
    uint32_t ret;
    mac_frame_ht_cap_stru *pst_ht_cap = NULL;
    mac_vht_cap_info_stru *pst_vht_cap = NULL;
    uint8_t he_flag = OAL_FALSE;

    puc_ht_ie = mac_find_ie(MAC_EID_HT_CAP, pst_beacon_info->tail, pst_beacon_info->tail_len);
    puc_vht_ie = mac_find_ie(MAC_EID_VHT_CAP, pst_beacon_info->tail, pst_beacon_info->tail_len);
    puc_he_ie = wal_parse_he_ie(pst_mac_vap, &he_flag, pst_beacon_info);
    wal_he_flag_handle(pst_netdev, pst_mac_vap, &puc_he_ie, &he_flag);

    /* 解析协议模式 */
    ret = wal_parse_protocol_mode(pst_mac_vap->st_channel.en_band, pst_beacon_info,
        puc_ht_ie, puc_vht_ie, puc_he_ie, &pst_beacon_param->en_protocol, he_flag);
    if (ret != OAL_SUCC) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_parse_ht_vht_ie::return err code!}\r\n", ret);
        return ret;
    }

    wal_go_protocol_handle(pst_mac_vap, pst_beacon_param);

    /* 如果ht vht均为空，返回 */
    if ((puc_ht_ie == NULL) && (puc_vht_ie == NULL)) {
        return OAL_SUCC;
    }
    /* 解析short gi能力 */
    if (puc_ht_ie != NULL) {
        /* 使用ht cap ie中数据域的2个字节 */
        if (puc_ht_ie[1] < sizeof(mac_frame_ht_cap_stru)) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                "{wal_parse_ht_vht_ie::invalid ht cap ie len[%d]!}\r\n", puc_ht_ie[1]);
            return OAL_SUCC;
        }

        pst_ht_cap = (mac_frame_ht_cap_stru *)(puc_ht_ie + MAC_IE_HDR_LEN);

        pst_beacon_param->en_shortgi_20 = (uint8_t)pst_ht_cap->bit_short_gi_20mhz;
        pst_beacon_param->en_shortgi_40 = 0;

        if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M) &&
            (pst_mac_vap->st_channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
            pst_beacon_param->en_shortgi_40 = (uint8_t)pst_ht_cap->bit_short_gi_40mhz;
        }
    }

    if (puc_vht_ie == NULL) {
        return OAL_SUCC;
    }

    /* 使用vht cap ie中数据域的4个字节 */
    if (puc_vht_ie[1] < sizeof(mac_vht_cap_info_stru)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, 0, "{wal_parse_ht_vht_ie::invalid ht cap ie len[%d]!}", puc_vht_ie[1]);
        return OAL_SUCC;
    }

    pst_vht_cap = (mac_vht_cap_info_stru *)(puc_vht_ie + MAC_IE_HDR_LEN);

    pst_beacon_param->en_shortgi_80 = 0;

    if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_40MINUS) &&
        (pst_mac_vap->st_channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
        pst_beacon_param->en_shortgi_80 = pst_vht_cap->bit_short_gi_80mhz;
    }

    return OAL_SUCC;
}

oal_bool_enum_uint8 wal_cfg80211_dfr_and_s3s4_param_check(void)
{
#ifdef _PRE_WLAN_FEATURE_DFR
    if (g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_dfr_and_s3s4_param_check::dfr_process_status[%d]!}",
                         g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_FALSE;
    }
#endif  // #ifdef _PRE_WLAN_FEATURE_DFR

#ifdef  _PRE_CONFIG_HISI_S3S4_POWER_STATE
    if (g_st_recovery_info.device_s3s4_process_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfg80211_dfr_and_s3s4_param_check::s3s4_process_status[%d]!}",
                         g_st_recovery_info.device_s3s4_process_flag);
        return OAL_FALSE;
    }
#endif

    return OAL_TRUE;
}
/*
 * 函 数 名  : wal_cfg80211_sched_scan_stop
 * 功能描述  : 调度扫描关闭
 * 1.日    期  : 2015年6月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
int32_t wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev,
    uint64_t reqid)

#else
int32_t wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev)
#endif
{
    /* 参数合法性检查 */
    if (oal_any_null_ptr2(pst_wiphy, pst_netdev)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_sched_scan_stop::input param pointer is null!}");
        return -OAL_EINVAL;
    }

    return wal_stop_sched_scan(pst_netdev);
}
OAL_STATIC OAL_INLINE void wal_cfg80211_scan_sched_scan_stop(hmac_scan_stru *pst_scan_mgmt,
    oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev, mac_vap_stru *pst_mac_vap)
{
    /* 如果当前调度扫描在运行，先暂停调度扫描 */
    if (pst_scan_mgmt->pst_sched_scan_req != NULL) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_cfg80211_scan_sched_scan_stop::stop sched scan,before normal scan}");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
        wal_cfg80211_sched_scan_stop(pst_wiphy, pst_netdev, 0);
#else
        wal_cfg80211_sched_scan_stop(pst_wiphy, pst_netdev);
#endif
    }
}

/*
 * 函 数 名  : wal_cfg80211_scan_param_check
 * 功能描述  : 校验参数
 * 1.日    期  : 2019年10月16日
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 wal_cfg80211_scan_param_check(oal_wiphy_stru *pst_wiphy,
    oal_cfg80211_scan_request_stru *pst_request)
{
    if (oal_any_null_ptr2(pst_wiphy, pst_request)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan_param_check::scan failed, null ptr!}");
        return OAL_FALSE;
    }

    /* 判断扫描传入内存长度不能大于后续缓存空间大小，避免拷贝内存越界 */
    if (pst_request->ie_len > WLAN_WPS_IE_MAX_SIZE || pst_request->wdev == NULL) {
        oam_error_log2(0, OAM_SF_CFG, "{wal_cfg80211_scan_param_check:: scan ie is too large to save. [%d][%d]!}",
            pst_request->ie_len, WLAN_WPS_IE_MAX_SIZE);
        return OAL_FALSE;
    }

    if (pst_request->wdev->netdev == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan_param_check::scan failed! pst_netdev null}");
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

OAL_STATIC int32_t wal_cfg80211_scan_pre_para_check(oal_wiphy_stru *wiphy, oal_cfg80211_scan_request_stru *request)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_device_stru *pst_hmac_device = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    oal_net_device_stru *pst_netdev = NULL;

    if (wal_cfg80211_scan_param_check(wiphy, request) == OAL_FALSE) {
        return -OAL_EFAIL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan:: dfr or s3s4 is processing!}");
        return -OAL_EFAIL;
    }

    /* 通过net_device 找到对应的mac_device_stru 结构 */
    mac_vap = oal_net_dev_priv(request->wdev->netdev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan::scan failed! pst_mac_vap null!}");
        return -OAL_EFAIL;
    }

    pst_netdev = request->wdev->netdev;
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
#ifdef _PRE_WLAN_FEATURE_WAPI
    if (wlan_chip_is_support_hw_wapi() == OAL_FALSE && is_p2p_scan_req(request) &&
        (hmac_user_is_wapi_connected(pst_mac_vap->uc_device_id) == OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_CFG, "{stop p2p scan under wapi!}");
        return -OAL_EFAIL;
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_scan::hmac_device null device_id:%d!}", pst_mac_vap->uc_device_id);
        return -OAL_EFAIL;
    }
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    if (pst_hmac_device->sniffer_mode != WLAN_SINFFER_OFF) { // sniffer抓包过程不处理扫描请求
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan::sniffer enable, scan abort!}");
        return -OAL_EFAIL;
    }
#endif

    return OAL_SUCC;
}

void wal_cfg80211_set_scan_random_mac(hmac_device_stru *hmac_device,
    uint32_t flags, uint8_t *mac_addr, uint8_t *mac_addr_mask)
{
    uint8_t random_mac[WLAN_MAC_ADDR_LEN];
    uint8_t *random_mac_addr = random_mac;
    uint8_t idx;
    /*
     *  flags的bit3为1表示上层下发随机mac扫描。
     * mac_addr_mask中bit为1位用mac_addr中的值；
     * mac_addr_mask中bit为0位用随机值。
     */
    if (flags & NL80211_SCAN_FLAG_RANDOM_ADDR) {
        hmac_device->st_scan_mgmt.random_mac_from_kernel = OAL_TRUE;
        oal_random_ether_addr(random_mac_addr, WLAN_MAC_ADDR_LEN);
        for (idx = 0; idx < WLAN_MAC_ADDR_LEN; idx++) {
            random_mac[idx] &= ~(mac_addr_mask[idx]);
            random_mac[idx] |= (mac_addr[idx] & mac_addr_mask[idx]);
        }
        random_mac[MAC_ADDR_0] &= 0xfe;
        memcpy_s(hmac_device->st_scan_mgmt.auc_random_mac, WLAN_MAC_ADDR_LEN, random_mac, WLAN_MAC_ADDR_LEN);
        oam_warning_log3(0, OAM_SF_SCAN,
                         "{wal_cfg80211_set_scan_random_mac::rand_mac_addr[%02X:XX:XX:XX:%02X:%02X].}",
                         hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_0],
                         hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_4],
                         hmac_device->st_scan_mgmt.auc_random_mac[MAC_ADDR_5]);
    } else {
        hmac_device->st_scan_mgmt.random_mac_from_kernel = OAL_FALSE;
    }
}
/*
 * 函 数 名  : wal_cfg80211_scan
 * 功能描述  : 内核调用启动扫描的接口函数
 * 1.日    期  : 2013年8月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_scan(oal_wiphy_stru *pst_wiphy, oal_cfg80211_scan_request_stru *pst_request)
{
    hmac_device_stru *pst_hmac_device = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    hmac_scan_stru *scan_mgmt = NULL;
    oal_net_device_stru *pst_netdev = NULL;

    if (wal_cfg80211_scan_pre_para_check(pst_wiphy, pst_request) != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    pst_netdev = pst_request->wdev->netdev;
    pst_mac_vap = oal_net_dev_priv(pst_netdev);
    pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    scan_mgmt = &(pst_hmac_device->st_scan_mgmt);

    oam_warning_log3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
        "{wal_cfg80211_scan::start a new normal scan. n_channels[%d], ie_len[%d], n_ssid[%d]}",
        pst_request->n_channels, pst_request->ie_len, pst_request->n_ssids);
#ifdef _PRE_EMU
    pst_request->n_channels = 1;
#endif
    /* gong 不可以重叠扫描, 若上次未结束，需要返回busy  */
    /* 等待上次的扫描请求完成 */
    if (oal_wait_event_interruptible_m(scan_mgmt->st_wait_queue, (scan_mgmt->pst_request == NULL)) < 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cfg80211_scan::start a new scan failed, wait return error.}");
        return -OAL_EFAIL;
    }

    /* p2p normal scan自己可以打断自己上一次的roc */
    if (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop roc scan, before normal scan.}");
        if (wal_p2p_stop_roc(pst_mac_vap, pst_netdev) < 0) {
            return -OAL_EFAIL;
        }
    }

    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        /* 如果有其他VAP处于P2P listen状态，则先取消listen */
        wal_another_vap_stop_p2p_listen(pst_hmac_device->pst_device_base_info, pst_mac_vap->uc_vap_id);
    }

    /* 关联阶段不下发，规避关联成功瞬间WLAN界面空白的不良体验 */
    if (hmac_sta_is_connecting(pst_mac_vap->en_vap_state) == OAL_TRUE) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_cfg80211_scan::stop normal scan while connecting.}");
        return -OAL_EFAIL;
    }

    /* 保存当前内核下发的扫描请求到本地 */
    scan_mgmt->pst_request = pst_request;
    wal_cfg80211_scan_sched_scan_stop(scan_mgmt, pst_wiphy, pst_netdev, pst_mac_vap);
    oam_warning_log1(0, 0, "wal_cfg80211_scan::flags[%d]", pst_request->flags);
    wal_cfg80211_set_scan_random_mac(pst_hmac_device, pst_request->flags,
        (uint8_t *)(pst_request->mac_addr), (uint8_t *)(pst_request->mac_addr_mask));
    /* 进入扫描 */
    if (OAL_SUCC != wal_scan_work_func(scan_mgmt, pst_netdev, pst_request)) {
        scan_mgmt->pst_request = NULL;
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_cfg80211_convert_value_to_vht_width
 * 1.日    期  : 2016年6月25日
  *   修改内容  : 新生成函数
 */
uint8_t wal_cfg80211_convert_value_to_vht_width(int32_t l_channel_value)
{
    uint8_t uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;

    switch (l_channel_value) {
        case 20:  /* 20M带宽 */
        case 40:  /* 40M带宽 */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
            break;
        case 80:  /* 80M带宽 */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_80;
            break;
        case 160: /* 160M带宽 */
            uc_channel_vht_width = WLAN_MIB_VHT_OP_WIDTH_160;
            break;
        default:
            break;
    }

    return uc_channel_vht_width;
}

/*
 * 函 数 名  : wal_cfg80211_set_wiphy_params
 * 功能描述  : 设置wiphy设备的 参数，RTS 门限阈值，分片门限阈值
 * 1.日    期  : 2013年10月28日
  *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_set_wiphy_params(oal_wiphy_stru *pst_wiphy, uint32_t changed)
{
    /* 通过HOSTAPD 设置RTS 门限，分片门限 采用接口wal_ioctl_set_frag， wal_ioctl_set_rts */
    oam_warning_log0(0, OAM_SF_CFG,
        "{wal_cfg80211_set_wiphy_params::should not call this function.call wal_ioctl_set_frag/wal_ioctl_set_rts!}");
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_convert_width_to_value
 * 功能描述  : 将内核下发的带宽枚举转换成真实的带宽宽度值
 * 1.日    期  : 2015年1月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_cfg80211_convert_width_to_value(int32_t l_channel_width)
{
    int32_t l_channel_width_value = 0;

    switch (l_channel_width) {
        case WIFI_CHAN_WIDTH_20:
        case WIFI_CHAN_WIDTH_40:
            l_channel_width_value = 20; /* 20M */
            break;
        case WIFI_CHAN_WIDTH_80:
            l_channel_width_value = 40; /* 40M */
            break;
        case WIFI_CHAN_WIDTH_160:
        case WIFI_CHAN_WIDTH_80P80:
            l_channel_width_value = 80; /* 80M */
            break;
        case WIFI_CHAN_WIDTH_5:
            l_channel_width_value = 160; /* 160M */
            break;
        default:
            break;
    }

    return l_channel_width_value;
}
#define WLAN_CHAN_OFFSET_PLUS_2 2
#define WLAN_CHAN_OFFSET_MINUS_2 (-2)
OAL_STATIC wlan_channel_bandwidth_enum_uint8 wal_cfg80211_set_channel_cal_bw(oal_net_device_stru *netdev)
{
    int32_t channel_center_freq0;
    int32_t channel_center_freq1;
    int32_t bandwidth_value;
    uint8_t vht_width;
    int32_t channel;
    wlan_channel_bandwidth_enum_uint8 channl_band_width;
    oal_ieee80211_channel *chan_stru = NULL;
    mac_vap_stru          *mac_vap   = NULL;
    mac_device_stru       *mac_dev   = NULL;

    if (oal_unlikely(netdev == NULL || netdev->ieee80211_ptr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_set_channel_cal_bw::netdev or ieee80211_ptr null!}");
        return WLAN_BAND_WIDTH_BUTT;
    }

    mac_vap = oal_net_dev_priv(netdev);

    chan_stru = netdev->ieee80211_ptr->preset_chandef.chan;
    channel   = chan_stru->hw_value;

    /* 进行内核带宽值和WITP 带宽值转换 */
    channel_center_freq0 = oal_ieee80211_frequency_to_channel(netdev->ieee80211_ptr->preset_chandef.center_freq1);
    channel_center_freq1 = oal_ieee80211_frequency_to_channel(netdev->ieee80211_ptr->preset_chandef.center_freq2);
    bandwidth_value = wal_cfg80211_convert_width_to_value(netdev->ieee80211_ptr->preset_chandef.width);
    vht_width = wal_cfg80211_convert_value_to_vht_width(bandwidth_value);

    if (bandwidth_value == 0) {
        oam_error_log1(0, 0, "{wal_cfg80211_cal_bw::chan width invalid, bandwidth=%d",
            netdev->ieee80211_ptr->preset_chandef.width);
        return WLAN_BAND_WIDTH_BUTT;
    }

    if (bandwidth_value == 80 || bandwidth_value == 160) { /* 80 160  为上层下发带宽参数 */
        channl_band_width = mac_get_bandwith_from_center_freq_seg0_seg1(vht_width, (uint8_t)channel,
            (uint8_t)channel_center_freq0, (uint8_t)channel_center_freq1);
    } else if (bandwidth_value == 40) { /* 40为上层下发带宽参数 */
        switch (channel_center_freq0 - channel) {
            case WLAN_CHAN_OFFSET_MINUS_2:
                channl_band_width = WLAN_BAND_WIDTH_40MINUS;
                break;
            case WLAN_CHAN_OFFSET_PLUS_2:
                channl_band_width = WLAN_BAND_WIDTH_40PLUS;
                break;
            default:
                channl_band_width = WLAN_BAND_WIDTH_20M;
                break;
        }
    } else {
        channl_band_width = WLAN_BAND_WIDTH_20M;
    }

    mac_dev = (mac_device_stru *)mac_res_get_dev(mac_vap->uc_device_id);
    if (wal_should_down_protocol_bw(mac_dev, mac_vap) == OAL_TRUE) {
        mac_vap_p2p_bw_back_to_40m(mac_vap, (uint8_t)channel, &channl_band_width);
    }
    return channl_band_width;
}

/* GO起在6G频段时，内核下发6G信道类型，值为3 */
/* 驱动当前只适配了2G=0 5G=1两种信道类型，6G信道在驱动的信道类型也为1，用ext6g_band标志位与5G区别 */
/* 因此起6G GO需将信道类型强制写为1，并置位6ghz标志位 */
static uint8_t wal_cfg80211_reset_6g_band_type(oal_ieee80211_channel *channel)
{
    uint8_t is_6ghz = OAL_FALSE;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 43))
    if (channel->band == NL80211_BAND_6GHZ) {
        is_6ghz = OAL_TRUE;
        channel->band = NL80211_BAND_5GHZ;
        oam_warning_log0(0, OAM_SF_P2P, "{wal_cfg80211_reset_6G_band_type::reset 6G band type}");
    }
#endif
    return is_6ghz;
}

/*
 * 函 数 名  : wal_cfg80211_set_channel_info
 * 功能描述  : 设置信道
 * 1.日    期  : 2014年12月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_set_channel_info(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev)
{
    int32_t l_ret;
    uint32_t err_code;
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    mac_cfg_channel_param_stru *pst_channel_param = NULL;
    mac_vap_stru *pst_mac_vap = oal_net_dev_priv(pst_netdev);
    uint8_t uc_vap_id = pst_mac_vap->uc_vap_id;
    int32_t l_bandwidth = pst_netdev->ieee80211_ptr->preset_chandef.width;
    int32_t l_center_freq1 = (int32_t)pst_netdev->ieee80211_ptr->preset_chandef.center_freq1;
    int32_t l_center_freq2 = (int32_t)pst_netdev->ieee80211_ptr->preset_chandef.center_freq2;
    oal_ieee80211_channel *pst_channel = pst_netdev->ieee80211_ptr->preset_chandef.chan;
    int32_t l_channel = pst_channel->hw_value;
    uint8_t is_6ghz = OAL_FALSE;

    oam_warning_log4(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel_info::l_bandwidth = %d, l_center_freq1 = %d,\
        l_center_freq2 = %d, l_channel = %d.}", l_bandwidth, l_center_freq1, l_center_freq2, l_channel);

    /* 内核下发6G信道类型为3时，驱动侧将信道类型写为1（5G），并置位6g标志位 */
    is_6ghz = wal_cfg80211_reset_6g_band_type(pst_channel);

    l_ret = (int32_t)mac_is_channel_num_valid(pst_channel->band, (uint8_t)l_channel,
        (uint8_t)(pst_channel->center_freq >= 5955)); /* 5955是6G的起始中心频点 */
    if (l_ret != OAL_SUCC) {
        oam_warning_log2(uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_set_channel_info::chan num invalid. band, ch num [%d] [%d]!}", pst_channel->band, l_channel);
        return -OAL_EINVAL;
    }

    en_bandwidth = wal_cfg80211_set_channel_cal_bw(pst_netdev);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    pst_channel_param = (mac_cfg_channel_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_channel_param, sizeof(mac_cfg_channel_param_stru), 0, sizeof(mac_cfg_channel_param_stru));
    pst_channel_param->uc_channel = (uint8_t)pst_channel->hw_value;
    pst_channel_param->en_band = pst_channel->band;
    pst_channel_param->ext6g_band = (is_6ghz == OAL_TRUE) ? 1 : 0; /* 信道为6G时 标志位置1 */
    pst_channel_param->en_bandwidth = en_bandwidth;

    oam_warning_log3(uc_vap_id, OAM_SF_ANY, "{wal_cfg80211_set_channel::uc_channel=%d, en_band=%d, en_bandwidth=%d.}",
                     pst_channel_param->uc_channel, pst_channel_param->en_band, pst_channel_param->en_bandwidth);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_SET_CHANNEL, sizeof(mac_cfg_channel_param_stru));

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_channel_param_stru),
                               (uint8_t *)&st_write_msg,  OAL_TRUE, &pst_rsp_msg);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_cfg80211_set_channel_info::wal_send_cfg_event return err:%d}", l_ret);
        return -OAL_EFAIL;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(uc_vap_id, OAM_SF_ANY,
            "{wal_cfg80211_set_channel_info::wal_check_and_release_msg_resp fail return err code:%u}", err_code);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
