/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Linux cfg80211接口
 * 作    者 :
 * 创建日期 : 2012年11月8日
 */

#include "securec.h"
#include "securectype.h"

#include "mac_ie.h"
#include "hmac_11i.h"

#include "wal_config_base.h"
#include "wal_cfg_ioctl.h"
#include "wal_linux_cfg80211.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID              OAM_FILE_ID_WAL_LINUX_WEP_KEY_CFG80211_C

/*
 * 函 数 名  : wal_set_crypto_info
 * 功能描述  : 设置STA connect 加密信息
 * 1.日    期  : 2014年1月24日
  *   修改内容  : 新生成函数
 */
void wal_set_crypto_info(hmac_conn_param_stru *pst_conn_param, oal_cfg80211_conn_stru *pst_sme)
{
    uint8_t *puc_ie_tmp = NULL;
    uint8_t *puc_wep_key = NULL;

#ifdef _PRE_WLAN_FEATURE_WAPI
    if (pst_sme->crypto.wpa_versions == WITP_WAPI_VERSION) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_cfg80211_connect::crypt ver is wapi!");
        pst_conn_param->uc_wapi = OAL_TRUE;
    } else {
        pst_conn_param->uc_wapi = OAL_FALSE;
    }
#endif

    if (pst_sme->privacy) {
        if ((pst_sme->key_len != 0) && (pst_sme->key != NULL)) {
            /* 设置wep加密信息 */
            pst_conn_param->uc_wep_key_len = pst_sme->key_len;
            pst_conn_param->uc_wep_key_index = pst_sme->key_idx;

            puc_wep_key = (uint8_t *)oal_memalloc(pst_sme->key_len);
            if (puc_wep_key == NULL) {
                oam_error_log1(0, OAM_SF_CFG, "{wal_set_crypto_info::puc_wep_key(%d) alloc mem return null ptr!}",
                               (uint32_t)(pst_sme->key_len));
                return;
            }
            if (EOK != memcpy_s(puc_wep_key, (uint32_t)pst_sme->key_len,
                                (uint8_t *)pst_sme->key, (uint32_t)(pst_sme->key_len))) {
                oam_error_log0(0, OAM_SF_CFG, "wal_set_crypto_info::memcpy fail!");
                oal_free(puc_wep_key);
                return;
            }
            pst_conn_param->puc_wep_key = puc_wep_key;

            return;
        }

        /* 优先查找 RSN 信息元素 */
        puc_ie_tmp = mac_find_ie(MAC_EID_RSN, (uint8_t *)pst_sme->ie, (int32_t)pst_sme->ie_len);
        if (puc_ie_tmp != NULL) {
            mac_ie_get_rsn_cipher(puc_ie_tmp, &pst_conn_param->st_crypto);

            oam_warning_log2(0, OAM_SF_WPA, "wal_set_crypto_info::get_rsn_cipher pair_suite[0]:0x%x pair_suite[1]:0x%x",
                             pst_conn_param->st_crypto.aul_pair_suite[0], pst_conn_param->st_crypto.aul_pair_suite[1]);
            oam_warning_log2(0, OAM_SF_WPA, "wal_set_crypto_info::aul_akm_suite[0]=0x%x, aul_akm_suite[1]=0x%x",
                pst_conn_param->st_crypto.aul_akm_suite[0], pst_conn_param->st_crypto.aul_akm_suite[1]);
        } else {
            /* WPA */
            puc_ie_tmp = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA,
                (uint8_t *)pst_sme->ie, (int32_t)pst_sme->ie_len);
            if (puc_ie_tmp != NULL) {
                mac_ie_get_wpa_cipher(puc_ie_tmp, &pst_conn_param->st_crypto);
                oam_warning_log2(0, OAM_SF_WPA,
                    "wal_set_crypto_info::get_wpa_cipher pair_suite[0]:0x%x pair_suite[1]:0x%x",
                    pst_conn_param->st_crypto.aul_pair_suite[0], pst_conn_param->st_crypto.aul_pair_suite[1]);
            }
        }
    }
}

/*
 * 函 数 名  : wal_cfg80211_add_key
 * 功能描述  : 配置ptk,gtk等密钥到物理层
 * 1.日    期  : 2013年8月19日
 *   修改内容  : 新生成函数
 * 2.日    期  : 2013年12月28日
 *   作    者  : wifi
 *   修改内容  : 使用局部变量替代malloc，以减少释放内存的复杂度
 */
int32_t wal_cfg80211_add_key(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev, uint8_t uc_key_index, bool en_pairwise,
    const uint8_t *puc_mac_addr, oal_key_params_stru *pst_params)
{
    wal_msg_write_stru write_msg;
    mac_addkey_param_stru payload_para;
    wal_msg_stru *p_rsp_msg = NULL;
    int32_t ret = EOK;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr3(pst_wiphy, pst_netdev, pst_params)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::pst_wiphy or pst_netdev or pst_params is null!}");
        return -OAL_EINVAL;
    }

    /* 1.2 key长度检查，防止拷贝越界 */
    if ((pst_params->key_len > OAL_WPA_KEY_LEN) || (pst_params->key_len < 0) ||
        (pst_params->seq_len > OAL_WPA_SEQ_LEN) || (pst_params->seq_len < 0)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_cfg80211_add_key::Param Check ERROR! key_len[%x]  seq_len[%x]!}\r\n",
                       (int32_t)pst_params->key_len, (int32_t)pst_params->seq_len);
        return -OAL_EINVAL;
    }

    /* 2.1 消息参数准备 */
    memset_s(&payload_para, sizeof(payload_para), 0, sizeof(payload_para));
    payload_para.uc_key_index = uc_key_index;
    if (puc_mac_addr != NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        ret += memcpy_s(payload_para.auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
    }

    payload_para.en_pairwise = en_pairwise;

    /* 2.2 获取相关密钥值 */
    payload_para.st_key.key_len = pst_params->key_len;
    payload_para.st_key.seq_len = pst_params->seq_len;
    payload_para.st_key.cipher = pst_params->cipher;
    ret += memcpy_s(payload_para.st_key.auc_key, OAL_WPA_KEY_LEN, pst_params->key, (uint32_t)pst_params->key_len);
    oam_warning_log2(0, OAM_SF_ANY, "{wal_cfg80211_add_key::key_len:%d, ret=0X%X!}\r\n",
        pst_params->key_len, ret);
    ret += memcpy_s(payload_para.st_key.auc_seq, OAL_WPA_SEQ_LEN, pst_params->seq, (uint32_t)pst_params->seq_len);
    oam_warning_log2(0, OAM_SF_ANY, "{wal_cfg80211_add_key::seq_len:%d, ret =0X%X}\r\n",
        pst_params->seq_len, ret);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    write_msg.en_wid = WLAN_CFGID_ADD_KEY;
    write_msg.us_len = sizeof(mac_addkey_param_stru);

    /* 3.2 填写 msg 消息体 */
    ret += memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
        &payload_para, sizeof(mac_addkey_param_stru));
    if (ret != EOK) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_cfg80211_add_key::memcpy fail! ret[%X]", ret);
    }

    /* 由于消息中使用了局部变量指针，因此需要将发送该函数设置为同步，否则hmac处理时会使用野指针 */
    if (OAL_SUCC != wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_addkey_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &p_rsp_msg)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    if (OAL_SUCC != wal_check_and_release_msg_resp(p_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_add_key::wal_check_and_release_msg_resp fail!}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_get_key
 * 功能描述  : 从已保存信息里获取ptk,gtk等密钥
 * 1.日    期  : 2013年8月19日
 *   修改内容  : 新生成函数
 * 2.日    期  : 2014年1月4日
 *   作    者  : wifi
 *   修改内容  : 使用局部变量替代malloc，以减少释放内存的复杂度
 */
int32_t wal_cfg80211_get_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_pairwise, const uint8_t *puc_mac_addr,
    void *cookie, void (*callback)(void *cookie, oal_key_params_stru *))
{
    wal_msg_write_stru st_write_msg;
    hmac_getkey_param_stru st_payload_params = { 0 };
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];
    wal_msg_stru *pst_rsp_msg = NULL;
    int32_t l_ret = EOK;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr4(pst_wiphy, pst_netdev, cookie, callback)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wiphy or netdev or cookie or callback is null!}");
        return -OAL_EINVAL;
    }

    /* 2.1 消息参数准备 */
    st_payload_params.pst_netdev = pst_netdev;
    st_payload_params.uc_key_index = uc_key_index;

    if (puc_mac_addr != NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        l_ret += memcpy_s(auc_mac_addr, WLAN_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
        st_payload_params.puc_mac_addr = auc_mac_addr;
    } else {
        st_payload_params.puc_mac_addr = NULL;
    }

    st_payload_params.en_pairwise = en_pairwise;
    st_payload_params.cookie = cookie;
    st_payload_params.callback = callback;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    st_write_msg.en_wid = WLAN_CFGID_GET_KEY;
    st_write_msg.us_len = sizeof(hmac_getkey_param_stru);

    /* 3.2 填写 msg 消息体 */
    l_ret += memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                      &st_payload_params, sizeof(hmac_getkey_param_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_get_key::memcpy fail!");
        return -OAL_EINVAL;
    }

    /* 由于消息中使用了局部变量指针，因此需要将发送该函数设置为同步，否则hmac处理时会使用野指针 */
    if (OAL_SUCC != wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_getkey_param_stru),
                                       (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wal_send_cfg_event fail.}");
        return -OAL_EINVAL;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_get_key::wal_check_and_release_msg_resp fail.}");
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_remove_key
 * 功能描述  : 把ptk,gtk等密钥从物理层删除
 * 1.日    期  : 2013年8月19日
 *   修改内容  : 新生成函数
 * 2.日    期  : 2014年1月4日
 *   作    者  : wifi
 *   修改内容  : 使用局部变量替代malloc，以减少释放内存的复杂度
 */
int32_t wal_cfg80211_remove_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_pairwise, const uint8_t *puc_mac_addr)
{
    mac_removekey_param_stru st_payload_params = { 0 };
    wal_msg_write_stru st_write_msg = { 0 };
    wal_msg_stru *pst_rsp_msg = NULL;
    int32_t l_ret = EOK;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr2(pst_wiphy, pst_netdev)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::pst_wiphy or pst_netdev is null!}");
        return -OAL_EINVAL;
    }

    if (wal_cfg80211_dfr_and_s3s4_param_check() == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key:: dfr or s3s4 is processing!}");
        return OAL_SUCC;
    }

    /* 2.1 消息参数准备 */
    st_payload_params.uc_key_index = uc_key_index;
    memset_s(st_payload_params.auc_mac_addr, OAL_MAC_ADDR_LEN, 0, OAL_MAC_ADDR_LEN);
    if (puc_mac_addr != NULL) {
        /* 不能使用内核下发的mac指针，可能被释放，需要拷贝到本地再使用 */
        l_ret += memcpy_s(st_payload_params.auc_mac_addr, OAL_MAC_ADDR_LEN, puc_mac_addr, WLAN_MAC_ADDR_LEN);
    }
    st_payload_params.en_pairwise = en_pairwise;
    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 3.1 填写 msg 消息头 */
    st_write_msg.en_wid = WLAN_CFGID_REMOVE_KEY;
    st_write_msg.us_len = sizeof(mac_removekey_param_stru);

    /* 3.2 填写 msg 消息体 */
    l_ret += memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                      &st_payload_params, sizeof(mac_removekey_param_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_remove_key::memcpy fail!");
        return -OAL_EFAIL;
    }

    if (OAL_SUCC != wal_send_cfg_event(pst_netdev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_removekey_param_stru),
                                       (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::wal_send_cfg_event fail.}");
        return -OAL_EFAIL;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfg80211_remove_key::wal_check_and_release_msg_resp fail.}");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_set_default_key
 * 功能描述  : 使配置的密钥生效
 * 1.日    期  : 2013年8月15日
 *   修改内容  : 新生成函数
 * 2.日    期  : 2014年07月31日
 *   修改内容  : 合并设置数据帧默认密钥和设置管理帧默认密钥函数
 */
int32_t wal_cfg80211_set_default_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_unicast, bool en_multicast)
{
    uint8_t  idx = 0;
    uint32_t cmd_id;
    uint32_t params[CMD_PARAMS_MAX_CNT] = { 0 };
    uint32_t offset;
    uint32_t ret;

    /* 1.1 入参检查 */
    if (oal_unlikely(oal_any_null_ptr2(pst_wiphy, pst_netdev))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_set_default_key::wiphy or netdev ptr is null!}");
        return -OAL_EINVAL;
    }

    /* 2.1 参数准备 */
    /* set_default_key x(key_index) 0|1(en_unicast) 0|1(multicast) */
    params[idx++] = uc_key_index;
    params[idx++] = en_unicast;
    params[idx++] = en_multicast;

    ret = wal_get_cmd_id("set_default_key", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_set_default_key:find set_default_key cmd is fail");
        return -OAL_EINVAL;
    }
    if (wal_process_cmd_params(pst_netdev, cmd_id, params) != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_set_default_mgmt_key
 * 功能描述  : 使配置的密钥生效.PMF 特性使用，配置管理密钥
 * 1.日    期  : 2014年11月21日
  *   修改内容  : 新生成函数
 * 2.日    期  : 2016年8月29日
  *   修改内容  : pmf适配
 */
int32_t wal_cfg80211_set_default_mgmt_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index)
{
    uint8_t  idx = 0;
    uint32_t cmd_id;
    uint32_t params[CMD_PARAMS_MAX_CNT] = { 0 };
    uint32_t offset;
    uint32_t ret;

    /* 1.1 入参检查 */
    if (oal_unlikely(oal_any_null_ptr2(pst_wiphy, pst_netdev))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfg80211_set_default_mgmt_key::wiphy or netdev is null!}");
        return -OAL_EINVAL;
    }
    /* 2.1 参数准备 */
    /* set_default_key x(key_index) 0|1(en_unicast) 0|1(multicast) */
    params[idx++] = uc_key_index;
    params[idx++] = OAL_FALSE;
    params[idx++] = OAL_TRUE;

    ret = wal_get_cmd_id("set_default_key", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_cfg80211_set_default_mgmt_key:find set_default_key cmd is fail");
        return -OAL_EINVAL;
    }

    if (wal_process_cmd_params(pst_netdev, cmd_id, params) != OAL_SUCC) {
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_cfg80211_set_pmksa
 * 功能描述  : 增加一个pmk缓存
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_set_pmksa(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_net_device, oal_cfg80211_pmksa_stru *pmksa)
{
    wal_msg_write_stru st_write_msg = { 0 };
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa = NULL;
    int32_t l_ret;
    int32_t l_memcpy_ret;

    if (oal_any_null_ptr3(pst_wiphy, pst_net_device, pmksa)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::wiphy or netdev or pmksa is null!}");
        return -OAL_EINVAL;
    }

    if (oal_any_null_ptr2(pmksa->bssid, pmksa->pmkid)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::bssid or pmkid is null!}");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_SET_PMKSA, sizeof(mac_cfg_pmksa_param_stru));
    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)st_write_msg.auc_value;
    l_memcpy_ret = memcpy_s(pst_cfg_pmksa->auc_bssid, WLAN_MAC_ADDR_LEN, pmksa->bssid, WLAN_MAC_ADDR_LEN);
    l_memcpy_ret += memcpy_s(pst_cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN, pmksa->pmkid, WLAN_PMKID_LEN);
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "wal_cfg80211_set_pmksa::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_pmksa_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_set_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_del_pmksa
 * 功能描述  : 删除一个pmk缓存
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_del_pmksa(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_net_device, oal_cfg80211_pmksa_stru *pmksa)
{
    wal_msg_write_stru st_write_msg = { 0 };
    mac_cfg_pmksa_param_stru *pst_cfg_pmksa = NULL;
    int32_t l_ret;
    int32_t l_memcpy_ret;

    if (oal_any_null_ptr3(pst_wiphy, pst_net_device, pmksa)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::param null!}\r\n");
        return -OAL_EINVAL;
    }

    if (oal_any_null_ptr2(pmksa->bssid, pmksa->pmkid)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::param null!}");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_DEL_PMKSA, sizeof(mac_cfg_pmksa_param_stru));
    pst_cfg_pmksa = (mac_cfg_pmksa_param_stru *)st_write_msg.auc_value;
    l_memcpy_ret = memcpy_s(pst_cfg_pmksa->auc_bssid, WLAN_MAC_ADDR_LEN, pmksa->bssid, WLAN_MAC_ADDR_LEN);
    l_memcpy_ret += memcpy_s(pst_cfg_pmksa->auc_pmkid, WLAN_PMKID_LEN, pmksa->pmkid, WLAN_PMKID_LEN);
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "wal_cfg80211_del_pmksa::memcpy fail!");
        return -OAL_EFAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_pmksa_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_cfg80211_del_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_flush_pmksa
 * 功能描述  : 清除所有pmk缓存
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_flush_pmksa(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device)
{
    wal_msg_write_stru st_write_msg = { 0 };
    int32_t l_ret;

    if (oal_any_null_ptr2(pst_wiphy, pst_net_device)) {
        oam_error_log0(0, OAM_SF_CFG,
                       "{wal_cfg80211_flush_pmksa::param null!}");
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CFG80211_FLUSH_PMKSA, 0);

    l_ret = wal_send_cfg_event(pst_net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH, (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_cfg80211_flush_pmksa::wal_send_cfg_event fail[%d]!}\r\n", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
