/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_user.c
 * 作    者 :
 * 创建日期 : 2012年10月19日
 */

/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "mac_resource.h"
#include "mac_device.h"
#include "mac_user.h"
#include "securec.h"
#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_USER_C

/*
 * 函 数 名  : mac_user_get_bw_in_6ghz
 * 功能描述  : 6G时根据AP的HE operation -> 6ghz operation info获取ap的工作带宽cap
 */
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
OAL_STATIC wlan_bw_cap_enum_uint8 mac_user_get_bw_in_6ghz(mac_frame_he_6ghz_oper_info_stru *he_6ghz_oper_info)
{
    uint8_t band_width;
    uint8_t center_freq_seg0;
    uint8_t center_freq_seg1;
    wlan_bw_cap_enum_uint8 bw_cap_6ghz = WLAN_BW_CAP_20M;

    /********************* 6 GHz BSS channel width ********************************
     ---------------------------------------------------------------------------
     |6 GHz Oper Chl Width |    0    |    1    |    2    |          3          |
     ---------------------------------------------------------------------------
     | BSS Oper Chl Width  |  20MHZ  |  40MHZ  |  80MHZ  |  160MHZ / 80+80MHZ  |
     ---------------------------------------------------------------------------
    *******************************************************************************/

    band_width = (uint8_t)(he_6ghz_oper_info->bit_channel_width);
    switch (band_width) {
        case WLAN_MIB_HE_6GHZ_OP_WIDTH_20:
            bw_cap_6ghz = WLAN_BW_CAP_20M;
            break;
        case WLAN_MIB_HE_6GHZ_OP_WIDTH_40:
            bw_cap_6ghz = WLAN_BW_CAP_40M;
            break;
        case WLAN_MIB_HE_6GHZ_OP_WIDTH_80:
            bw_cap_6ghz = WLAN_BW_CAP_80M;
            break;
        case WLAN_MIB_HE_6GHZ_OP_WIDTH_80PLUS80_160:
            center_freq_seg0 = he_6ghz_oper_info->center_freq_seg0;
            center_freq_seg1 = he_6ghz_oper_info->center_freq_seg1;
            if (center_freq_seg1 > 0 && GET_ABS(center_freq_seg0 - center_freq_seg1) == CHAN_OFFSET_8) {
                bw_cap_6ghz = WLAN_BW_CAP_160M;
            } else if (center_freq_seg1 > 0 &&
                        GET_ABS(center_freq_seg0 - center_freq_seg1) > CHAN_OFFSET_16) {
                bw_cap_6ghz = WLAN_BW_CAP_80PLUS80;
            }
            break;
        default:
            break;
    }

    return bw_cap_6ghz;
}
#endif

/*
 * 函 数 名  : mac_user_update_ap_bandwidth_cap
 * 功能描述  : 根据AP的operation ie获取ap的工作带宽cap
 * 1.日    期  : 2014年4月26日
 *   修改内容  : 新生成函数
 */
uint32_t mac_user_update_ap_bandwidth_cap(mac_user_stru *pst_mac_user)
{
    mac_user_ht_hdl_stru *pst_mac_ht_hdl = NULL;
    mac_vht_hdl_stru *pst_mac_vht_hdl = NULL;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    mac_he_hdl_stru *mac_he_hdl = NULL;
    mac_frame_he_6ghz_oper_info_stru *he_6ghz_oper_info = NULL;
#endif

    if (pst_mac_user == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取HT和VHT结构体指针 */
    pst_mac_ht_hdl = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);

    pst_mac_user->en_bandwidth_cap = WLAN_BW_CAP_20M;

    if (pst_mac_ht_hdl->en_ht_capable == OAL_TRUE) {
        if (pst_mac_ht_hdl->bit_secondary_chan_offset != MAC_SCN) {
            pst_mac_user->en_bandwidth_cap = WLAN_BW_CAP_40M;
        }
    }

    if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE) {
        /* en_channel_width的取值:VHT opern ie，0 -- 20/40M, 1 -- 80/160M, 同时兼容已废弃的2和3 */
        /* bit_supported_channel_width: VHT capabilities,0--非160,1--160M, 2--160 and 80+80 */
        if (pst_mac_vht_hdl->en_channel_width == WLAN_MIB_VHT_OP_WIDTH_80) {
            if ((pst_mac_vht_hdl->bit_supported_channel_width == 0) && (pst_mac_vht_hdl->bit_extend_nss_bw_supp != 0)) {
                pst_mac_user->en_bandwidth_cap =
                    (pst_mac_ht_hdl->uc_chan_center_freq_seg2 != 0) ? WLAN_BW_CAP_160M : WLAN_BW_CAP_80M;
            } else {
                pst_mac_user->en_bandwidth_cap =
                    (pst_mac_vht_hdl->uc_channel_center_freq_seg1 != 0) ? WLAN_BW_CAP_160M : WLAN_BW_CAP_80M;
            }
        } else if (pst_mac_vht_hdl->en_channel_width == WLAN_MIB_VHT_OP_WIDTH_160) {
            pst_mac_user->en_bandwidth_cap = WLAN_BW_CAP_160M;
        } else if (pst_mac_vht_hdl->en_channel_width == WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
            pst_mac_user->en_bandwidth_cap = WLAN_BW_CAP_80M;
        }
    }

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    /* 6G时从he opern IE的6ghz opern info字段解析ap带宽能力 */
    mac_he_hdl        = &(pst_mac_user->st_he_hdl);
    he_6ghz_oper_info = &(pst_mac_user->he_6ghz_operation_info);
    if (mac_he_hdl->en_he_capable == OAL_TRUE &&
        mac_he_hdl->st_he_oper_ie.st_he_oper_param.bit_6g_oper_info_present == OAL_TRUE) {
        pst_mac_user->en_bandwidth_cap = mac_user_get_bw_in_6ghz(he_6ghz_oper_info);
    }
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : mac_user_set_num_spatial_stream_160M
 * 功能描述  : 设置user公共区域en_avail_num_spatial_stream Tx和Rx支持Nss的交集,供算法调用的值
 * 1.日    期  : 2019年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_num_spatial_stream_160M(mac_user_stru *pst_mac_user, uint8_t uc_value)
{
    pst_mac_user->st_vht_hdl.bit_user_num_spatial_stream_160m = uc_value;
}

/*
 * 函 数 名  : mac_user_get_sta_cap_bandwidth_11ac
 * 功能描述  : 11ac模式下，获取用户的带宽
 * 1.日    期  : 2019年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t mac_user_get_sta_cap_bandwidth_11ac(wlan_channel_band_enum_uint8 en_band,
    mac_user_ht_hdl_stru *pst_mac_ht_hdl, mac_vht_hdl_stru *pst_mac_vht_hdl,
    mac_user_stru *pst_mac_user)
{
    wlan_bw_cap_enum_uint8 en_bandwidth_cap;

    /* 2.4g band不应根据vht cap获取带宽信息 */
    if ((en_band == WLAN_BAND_2G) && (pst_mac_ht_hdl->en_ht_capable == OAL_TRUE)) {
        en_bandwidth_cap =
            (pst_mac_ht_hdl->bit_supported_channel_width == WLAN_BW_CAP_40M) ? WLAN_BW_CAP_40M : WLAN_BW_CAP_20M;
    } else {
        if (pst_mac_vht_hdl->bit_supported_channel_width == 0) {
            if ((pst_mac_vht_hdl->bit_extend_nss_bw_supp == WLAN_EXTEND_NSS_BW_SUPP0) ||
                (pst_mac_user->en_user_max_cap_nss == WLAN_SINGLE_NSS)) {
                en_bandwidth_cap = WLAN_BW_CAP_80M;
            } else {
                en_bandwidth_cap = WLAN_BW_CAP_160M;
            }
        } else {
            en_bandwidth_cap = WLAN_BW_CAP_160M;
        }
    }
    return en_bandwidth_cap;
}
/*
 * 函 数 名  : mac_user_init_rom
 * 功能描述  : 初始化mac user rom 扩展部分
 * 1.日    期  : 2019年3月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mac_user_init_rom(mac_user_stru *pst_mac_user, uint16_t us_user_idx)
{
    mac_user_set_num_spatial_stream_160M(pst_mac_user, WLAN_NSS_LIMIT);

    /* 组播用户port默认打开,否则aput不加密场景会拦截组播帧 */
    if (pst_mac_user->en_is_multi_user == OAL_TRUE) {
        mac_user_set_port(pst_mac_user, OAL_TRUE);
    }
    return OAL_SUCC;
}

oal_bool_enum_uint8 mac_user_get_port(mac_user_stru *mac_user)
{
    if (mac_user != NULL) {
        return mac_user->en_port_valid;
    }

    return OAL_FALSE;
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*
 * 函 数 名  : mac_user_set_he_capable
 * 功能描述  : 设置he cap信息
 * 1.日    期  : 2017年5月7日
 *   作    者  : 无
 *   修改内容  : 新生成函数
 */
void mac_user_set_he_capable(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_he_capable)
{
    mac_he_hdl_stru *pst_he_hdl = NULL;

    if (pst_mac_user != NULL) {
        pst_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
        pst_he_hdl->en_he_capable = en_he_capable;
    }
}

/*
 * 函 数 名  : mac_user_get_he_capable
 * 功能描述  : 获取he cap能力
 * 1.日    期  : 2017年5月7日
 *   作    者  : 无
 *   修改内容  : 新生成函数
 */
oal_bool_enum_uint8 mac_user_get_he_capable(mac_user_stru *pst_mac_user)
{
    mac_he_hdl_stru *pst_he_hdl = NULL;

    if (pst_mac_user != NULL) {
        pst_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
        return pst_he_hdl->en_he_capable;
    }

    return OAL_FALSE;
}

/*
 * 函 数 名  : mac_user_get_he_hdl
 * 功能描述  : 获取用户的he信息
 * 1.日    期  : 2017年5月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_get_he_hdl(mac_user_stru *pst_mac_user, mac_he_hdl_stru *pst_he_hdl)
{
    if (memcpy_s((uint8_t *)pst_he_hdl, sizeof(mac_he_hdl_stru),
        (uint8_t *)(MAC_USER_HE_HDL_STRU(pst_mac_user)), sizeof(mac_he_hdl_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_get_he_hdl::memcpy fail!");
    }
}

/*
 * 函 数 名  : mac_user_set_he_hdl
 * 功能描述  : 对用户的vht信息进行设置
 * 1.日    期  : 2015年5月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_he_hdl(mac_user_stru *pst_mac_user, mac_he_hdl_stru *pst_he_hdl)
{
    if (memcpy_s((uint8_t *)(MAC_USER_HE_HDL_STRU(pst_mac_user)),
        sizeof(mac_he_hdl_stru), (uint8_t *)pst_he_hdl, sizeof(mac_he_hdl_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_set_he_hdl::memcpy fail!");
    }
}

#endif

/*
 * 函 数 名  : mac_user_add_wep_key
 * 功能描述  : 添加wep密钥到指定的密钥槽
 * 1.日    期  : 2015年5月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mac_user_add_wep_key(mac_user_stru *pst_mac_user, uint8_t uc_key_index, mac_key_params_stru *pst_key)
{
    int32_t l_ret;

    if (uc_key_index >= WLAN_NUM_TK) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    if ((uint32_t)pst_key->key_len > WLAN_WEP104_KEY_LEN) {
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    if ((uint32_t)pst_key->seq_len > WLAN_WPA_SEQ_LEN) {
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    pst_mac_user->st_key_info.ast_key[uc_key_index].cipher = pst_key->cipher;
    pst_mac_user->st_key_info.ast_key[uc_key_index].key_len = (uint32_t)pst_key->key_len;
    pst_mac_user->st_key_info.ast_key[uc_key_index].seq_len = (uint32_t)pst_key->seq_len;

    l_ret = memcpy_s(&pst_mac_user->st_key_info.ast_key[uc_key_index].auc_key, WLAN_WPA_KEY_LEN,
                     pst_key->auc_key, (uint32_t)pst_key->key_len);
    l_ret += memcpy_s(&pst_mac_user->st_key_info.ast_key[uc_key_index].auc_seq, WLAN_WPA_SEQ_LEN,
                      pst_key->auc_seq, (uint32_t)pst_key->seq_len);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_add_wep_key::memcpy fail!");
        return OAL_FAIL;
    }

    pst_mac_user->st_user_tx_info.st_security.en_cipher_key_type = WLAN_KEY_TYPE_TX_GTK;

    return OAL_SUCC;
}

/*
 * 函 数 名  : mac_user_add_rsn_key
 * 功能描述  : 更新rsn单播信息
 * 1.日    期  : 2014年11月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mac_user_add_rsn_key(mac_user_stru *pst_mac_user, uint8_t uc_key_index, mac_key_params_stru *pst_key)
{
    int32_t l_ret;

    if (uc_key_index >= WLAN_NUM_TK) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }
    if ((uint32_t)pst_key->key_len > WLAN_WPA_KEY_LEN) {
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    if ((uint32_t)pst_key->seq_len > WLAN_WPA_SEQ_LEN) {
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    pst_mac_user->st_key_info.ast_key[uc_key_index].cipher = pst_key->cipher;
    pst_mac_user->st_key_info.ast_key[uc_key_index].key_len = (uint32_t)pst_key->key_len;
    pst_mac_user->st_key_info.ast_key[uc_key_index].seq_len = (uint32_t)pst_key->seq_len;

    l_ret = memcpy_s(&pst_mac_user->st_key_info.ast_key[uc_key_index].auc_key, WLAN_WPA_KEY_LEN,
                     pst_key->auc_key, (uint32_t)pst_key->key_len);
    l_ret += memcpy_s(&pst_mac_user->st_key_info.ast_key[uc_key_index].auc_seq, WLAN_WPA_SEQ_LEN,
                      pst_key->auc_seq, (uint32_t)pst_key->seq_len);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_add_rsn_key::memcpy fail!");
        return OAL_FAIL;
    }

    pst_mac_user->st_key_info.en_cipher_type = (uint8_t)pst_key->cipher;
    pst_mac_user->st_key_info.uc_default_index = uc_key_index;

    return OAL_SUCC;
}

/*
 * 函 数 名  : mac_user_add_bip_key
 * 功能描述  : 更新bip信息
 * 1.日    期  : 2014年11月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mac_user_add_bip_key(mac_user_stru *pst_mac_user, uint8_t uc_key_index, mac_key_params_stru *pst_key)
{
    int32_t l_ret;

    /* keyid校验 */
    if (uc_key_index < WLAN_NUM_TK || uc_key_index > WLAN_MAX_IGTK_KEY_INDEX) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    pst_mac_user->st_key_info.ast_key[uc_key_index].cipher = pst_key->cipher;
    pst_mac_user->st_key_info.ast_key[uc_key_index].key_len = (uint32_t)pst_key->key_len;
    pst_mac_user->st_key_info.ast_key[uc_key_index].seq_len = (uint32_t)pst_key->seq_len;

    l_ret = memcpy_s(&pst_mac_user->st_key_info.ast_key[uc_key_index].auc_key, WLAN_WPA_KEY_LEN,
                     pst_key->auc_key, (uint32_t)pst_key->key_len);
    l_ret += memcpy_s(&pst_mac_user->st_key_info.ast_key[uc_key_index].auc_seq, WLAN_WPA_SEQ_LEN,
                      pst_key->auc_seq, (uint32_t)pst_key->seq_len);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_add_bip_key::memcpy fail!");
        return OAL_FAIL;
    }

    pst_mac_user->st_key_info.uc_igtk_key_index = uc_key_index;

    return OAL_SUCC;
}


/*
 * 函 数 名  : mac_user_set_port
 * 功能描述  : 复位11X端口
 * 1.日    期  : 2014年11月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_port(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_port_valid)
{
    oam_warning_log3(0, OAM_SF_ANY, "mac_user_set_port::user[%d] old %d, new %d",
        pst_mac_user->us_assoc_id, pst_mac_user->en_port_valid, en_port_valid);
    pst_mac_user->en_port_valid = en_port_valid;
}

/*
 * 函 数 名  : mac_user_init_key
 * 功能描述  : 初始化用户的密钥信息
 * 1.日    期  : 2015年5月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_init_key(mac_user_stru *pst_mac_user)
{
    memset_s(&pst_mac_user->st_key_info, sizeof(mac_key_mgmt_stru), 0, sizeof(mac_key_mgmt_stru));
    pst_mac_user->st_key_info.en_cipher_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    pst_mac_user->st_key_info.uc_last_gtk_key_idx = 0xFF;
    pst_mac_user->st_cap_info.bit_pmf_active = OAL_FALSE;
}

/*
 * 函 数 名  : mac_user_set_key
 * 功能描述  : 设置用户的安全信息
 * 其    他  : 截止此函数编写时，只有keyid得到使用，multiuser下的安全信息可以优化一下
 * 1.日    期  : 2015年03月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_key(mac_user_stru *pst_multiuser,
    wlan_cipher_key_type_enum_uint8 en_keytype, wlan_ciper_protocol_type_enum_uint8 en_ciphertype,
    uint8_t uc_keyid)
{
    pst_multiuser->st_user_tx_info.st_security.en_cipher_key_type = en_keytype;
    pst_multiuser->st_user_tx_info.st_security.en_cipher_protocol_type = en_ciphertype;
    pst_multiuser->st_user_tx_info.st_security.uc_cipher_key_id = uc_keyid;
    oam_warning_log4(0, OAM_SF_WPA,
                     "{mac_user_set_key::keytpe==%u, ciphertype==%u, keyid==%u, usridx==%u}",
                     en_keytype, en_ciphertype, uc_keyid, pst_multiuser->us_assoc_id);
}

/*
 * 函 数 名  : mac_user_init
 * 功能描述  : 初始化mac user公共部分
 * 1.日    期  : 2013年8月21日
 *   修改内容  : 新生成函数
 */
uint32_t mac_user_init(mac_user_stru *pst_mac_user,
    uint16_t us_user_idx, uint8_t *puc_mac_addr, uint8_t uc_chip_id,
    uint8_t uc_device_id, uint8_t uc_vap_id)
{
#ifdef _PRE_WLAN_FEATURE_WMMAC
    uint8_t uc_ac_loop;
#endif

    if (oal_unlikely(pst_mac_user == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_user_init::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化chip id, device ip, vap id */
    pst_mac_user->uc_chip_id = uc_chip_id;
    pst_mac_user->uc_device_id = uc_device_id;
    pst_mac_user->uc_vap_id = uc_vap_id;
    pst_mac_user->us_assoc_id = us_user_idx;
    pst_mac_user->ampdu_maxsize = 65535; // 65535最大聚合字节数
    /* 初始化密钥 */
    pst_mac_user->st_user_tx_info.st_security.en_cipher_key_type = WLAN_KEY_TYPE_PTK;
    pst_mac_user->st_user_tx_info.st_security.en_cipher_protocol_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;

    /* 初始化安全加密信息 */
    mac_user_init_key(pst_mac_user);
    mac_user_set_key(pst_mac_user, WLAN_KEY_TYPE_PTK, WLAN_80211_CIPHER_SUITE_NO_ENCRYP, 0);
    mac_user_set_port(pst_mac_user, OAL_FALSE);
    pst_mac_user->en_user_asoc_state = MAC_USER_STATE_BUTT;

    if (puc_mac_addr == NULL) {
        pst_mac_user->en_is_multi_user = OAL_TRUE;
        pst_mac_user->en_user_asoc_state = MAC_USER_STATE_ASSOC;
    } else {
        /* 初始化一个用户是否是组播用户的标志，组播用户初始化时不会调用本函数 */
        pst_mac_user->en_is_multi_user = OAL_FALSE;

        /* 设置mac地址 */
        oal_set_mac_addr(pst_mac_user->auc_user_mac_addr, puc_mac_addr);
    }

    /* 初始化能力 */
    mac_user_set_pmf_active(pst_mac_user, OAL_FALSE);
    pst_mac_user->st_cap_info.bit_proxy_arp = OAL_FALSE;

    mac_user_set_avail_num_spatial_stream(pst_mac_user, WLAN_SINGLE_NSS);

#ifdef _PRE_WLAN_FEATURE_WMMAC
    /* TS信息初始化 */
    for (uc_ac_loop = 0; uc_ac_loop < WLAN_WME_AC_BUTT; uc_ac_loop++) {
        pst_mac_user->st_ts_info[uc_ac_loop].uc_up = WLAN_WME_AC_TO_TID(uc_ac_loop);
        pst_mac_user->st_ts_info[uc_ac_loop].en_ts_status = MAC_TS_NONE;
        pst_mac_user->st_ts_info[uc_ac_loop].uc_vap_id = pst_mac_user->uc_vap_id;
        pst_mac_user->st_ts_info[uc_ac_loop].us_mac_user_idx = pst_mac_user->us_assoc_id;
        pst_mac_user->st_ts_info[uc_ac_loop].uc_tsid = 0xFF;
        pst_mac_user->need_degrade[uc_ac_loop] = 0;
    }
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : mac_user_set_avail_bf_num_spatial_stream
 * 功能描述  : 设置user公共区域en_avail_num_spatial_stream Tx和Rx支持Nss的交集,供算法调用的值
 * 1.日    期  : 2015年4月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_avail_bf_num_spatial_stream(mac_user_stru *pst_mac_user, uint8_t uc_value)
{
    pst_mac_user->en_avail_bf_num_spatial_stream = uc_value;
}

/*
 * 函 数 名  : mac_user_set_avail_num_spatial_stream
 * 功能描述  : 设置user公共区域en_avail_num_spatial_stream Tx和Rx支持Nss的交集,供算法调用的值
 * 1.日    期  : 2015年4月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_avail_num_spatial_stream(mac_user_stru *pst_mac_user, uint8_t uc_value)
{
    pst_mac_user->en_avail_num_spatial_stream = uc_value;
}
/*
 * 函 数 名  : mac_user_set_num_spatial_stream
 * 功能描述  : 设置user公共区域en_user_num_spatial_stream用户支持空间流个数的值
 * 1.日    期  : 2015年4月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_num_spatial_stream(mac_user_stru *pst_mac_user, uint8_t uc_value)
{
    pst_mac_user->en_user_max_cap_nss = uc_value;
}
/*
 * 函 数 名  : mac_user_set_bandwidth_cap
 * 功能描述  : 设置用户的bandwidth能力
 * 1.日    期  : 2015年4月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_bandwidth_cap(mac_user_stru *pst_mac_user, wlan_bw_cap_enum_uint8 en_bandwidth_value)
{
    pst_mac_user->en_bandwidth_cap = en_bandwidth_value;
}
/*
 * 函 数 名  : mac_user_set_bandwidth_info
 * 功能描述  : 设置可用带宽的信息
 * 1.日    期  : 2015年4月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 * 2.日    期  : 2015年5月27日
 *   修改内容  : 针对协议切换, 对带宽更新进行特殊判断
 */
void mac_user_set_bandwidth_info(mac_user_stru *pst_mac_user,
    wlan_bw_cap_enum_uint8 en_avail_bandwidth, wlan_bw_cap_enum_uint8 en_cur_bandwidth)
{
    pst_mac_user->en_avail_bandwidth = en_avail_bandwidth;
    pst_mac_user->en_cur_bandwidth = en_cur_bandwidth;

    /* Autorate将协议从11n切换成11b后, cur_bandwidth会变为20M
       此时如果软件将带宽改为40M, cur_bandwidth仍需要保持20M */
    if ((pst_mac_user->en_cur_protocol_mode == WLAN_LEGACY_11B_MODE) &&
        (pst_mac_user->en_cur_bandwidth != WLAN_BW_CAP_20M)) {
        pst_mac_user->en_cur_bandwidth = WLAN_BW_CAP_20M;
    }
}
#ifdef _PRE_WLAN_FEATURE_11AX
/*
 * 函 数 名  : mac_user_get_sta_cap_bandwidth_11ax
 * 功能描述  : 获取用户的带宽
 * 1.日    期  : 2020年4月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t mac_user_get_sta_cap_bandwidth_11ax(wlan_channel_band_enum_uint8 en_band,
    mac_he_hdl_stru *pst_mac_he_hdl)
{
    wlan_bw_cap_enum_uint8 en_bandwidth_cap;
    if (en_band == WLAN_BAND_2G) {
        if (pst_mac_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_channel_width_set & 0x1) { /* Bit0 2G 是否支持40MHz */
            en_bandwidth_cap = WLAN_BW_CAP_40M;
        } else {
            en_bandwidth_cap = WLAN_BW_CAP_20M;
        }
    } else {
        if (pst_mac_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_channel_width_set & 0x4) {
            /* Bit3 指示 160MHz mp15支持 */
            en_bandwidth_cap = WLAN_BW_CAP_160M;
        } else if (pst_mac_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_channel_width_set & 0x2) {
            /* Bit2 指示5G 80MHz */
            en_bandwidth_cap = WLAN_BW_CAP_80M;
        } else if (pst_mac_he_hdl->st_he_cap_ie.st_he_phy_cap.bit_channel_width_set & 0x1) {
            en_bandwidth_cap = WLAN_BW_CAP_40M;
        } else {
            en_bandwidth_cap = WLAN_BW_CAP_20M;
        }
    }
    return en_bandwidth_cap;
}
#endif
/*
 * 函 数 名  : mac_user_get_sta_cap_bandwidth
 * 功能描述  : 获取用户的带宽
 * 1.日    期  : 2013年10月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_get_sta_cap_bandwidth(mac_user_stru *pst_mac_user,
    wlan_bw_cap_enum_uint8 *pen_bandwidth_cap)
{
    mac_user_ht_hdl_stru    *pst_mac_ht_hdl = NULL;
    mac_vht_hdl_stru        *pst_mac_vht_hdl = NULL;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru         *pst_mac_he_hdl = MAC_USER_HE_HDL_STRU(pst_mac_user);
#endif
    wlan_bw_cap_enum_uint8   en_bandwidth_cap;
    mac_vap_stru            *pst_mac_vap;

    pst_mac_vap = mac_res_get_mac_vap(pst_mac_user->uc_vap_id);
    if (pst_mac_vap == NULL) {
        return;
    }
    /* 获取HT和VHT结构体指针 */
    pst_mac_ht_hdl = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open && (OAL_TRUE == mac_user_is_he_user(pst_mac_user))) {
        en_bandwidth_cap = mac_user_get_sta_cap_bandwidth_11ax(pst_mac_vap->st_channel.en_band, pst_mac_he_hdl);
    } else if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE) {
#else // ut 11AX宏未开启，待后续修改
    if (pst_mac_vht_hdl->en_vht_capable == OAL_TRUE) {
#endif
        /*  2.4g band不应根据vht cap获取带宽信息 */
        en_bandwidth_cap = mac_user_get_sta_cap_bandwidth_11ac(pst_mac_vap->st_channel.en_band,
            pst_mac_ht_hdl, pst_mac_vht_hdl, pst_mac_user);
    } else if (pst_mac_ht_hdl->en_ht_capable == OAL_TRUE) {
        en_bandwidth_cap = (pst_mac_ht_hdl->bit_supported_channel_width == 1) ? WLAN_BW_CAP_40M : WLAN_BW_CAP_20M;
    } else {
        en_bandwidth_cap = WLAN_BW_CAP_20M;
    }

    mac_user_set_bandwidth_cap(pst_mac_user, en_bandwidth_cap);

    /* 将带宽值由出参带出 */
    *pen_bandwidth_cap = en_bandwidth_cap;
}

/*
 * 函 数 名  : mac_user_set_assoc_id
 * 功能描述  : 设置user的assoc id
 * 1.日    期  : 2015年4月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_assoc_id(mac_user_stru *pst_mac_user, uint16_t us_assoc_id)
{
    pst_mac_user->us_assoc_id = us_assoc_id;
}
/*
 * 函 数 名  : mac_user_set_avail_protocol_mode
 * 功能描述  : 这只mac user中的user与vap协议模式的交集模式
 * 1.日    期  : 2015年4月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_avail_protocol_mode(mac_user_stru *pst_mac_user,
    wlan_protocol_enum_uint8 en_avail_protocol_mode)
{
    pst_mac_user->en_avail_protocol_mode = en_avail_protocol_mode;
}
/*
 * 函 数 名  : mac_user_set_cur_protocol_mode
 * 功能描述  : 设置mac user中的cur_protocol_mode
 * 1.日    期  : 2015年4月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_cur_protocol_mode(mac_user_stru *pst_mac_user, wlan_protocol_enum_uint8 en_cur_protocol_mode)
{
    pst_mac_user->en_cur_protocol_mode = en_cur_protocol_mode;
}

/*
 * 函 数 名  : mac_user_set_avail_protocol_mode
 * 功能描述  : 这只mac user中的user与vap协议模式的交集模式
 * 1.日    期  : 2015年4月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_protocol_mode(mac_user_stru *pst_mac_user, wlan_protocol_enum_uint8 en_protocol_mode)
{
    pst_mac_user->en_protocol_mode = en_protocol_mode;
}
/*
 * 函 数 名  : mac_user_set_asoc_state
 * 功能描述  : 设置en_user_asoc_state 的统一接口
 * 1.日    期  : 2015年5月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_asoc_state(mac_user_stru *pst_mac_user, mac_user_asoc_state_enum_uint8 en_value)
{
    pst_mac_user->en_user_asoc_state = en_value;
}
/*
 * 函 数 名  : mac_user_set_avail_op_rates
 * 功能描述  : 设置user下可用速率集
 * 1.日    期  : 2015年5月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_avail_op_rates(mac_user_stru *pst_mac_user, uint8_t uc_rs_nrates, uint8_t *puc_rs_rates)
{
    int32_t l_ret;

    pst_mac_user->st_avail_op_rates.uc_rs_nrates = uc_rs_nrates;
    l_ret = memcpy_s(pst_mac_user->st_avail_op_rates.auc_rs_rates,
                     WLAN_MAX_SUPP_RATES, puc_rs_rates, uc_rs_nrates);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_set_avail_op_rates::memcpy fail!");
    }
}
/*
 * 函 数 名  : mac_user_set_vht_hdl
 * 功能描述  : 对用户的vht信息进行设置
 * 1.日    期  : 2015年5月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_vht_hdl(mac_user_stru *pst_mac_user, mac_vht_hdl_stru *pst_vht_hdl)
{
    int32_t l_ret;

    l_ret = memcpy_s((uint8_t *)(&pst_mac_user->st_vht_hdl), sizeof(mac_vht_hdl_stru),
                     (uint8_t *)pst_vht_hdl, sizeof(mac_vht_hdl_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_set_vht_hdl::memcpy fail!");
    }
}
/*
 * 函 数 名  : mac_user_get_vht_hdl
 * 功能描述  : 获取用户的vht信息
 * 1.日    期  : 2015年5月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_get_vht_hdl(mac_user_stru *pst_mac_user, mac_vht_hdl_stru *pst_vht_hdl)
{
    int32_t l_ret;

    l_ret = memcpy_s((uint8_t *)pst_vht_hdl, sizeof(mac_vht_hdl_stru),
                     (uint8_t *)(&pst_mac_user->st_vht_hdl), sizeof(mac_vht_hdl_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_get_vht_hdl::memcpy fail!");
    }
}

/*
 * 函 数 名  : mac_user_set_vht_hdl
 * 功能描述  : 对用户的vht信息进行设置
 * 1.日    期  : 2015年5月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_ht_hdl(mac_user_stru *pst_mac_user, mac_user_ht_hdl_stru *pst_ht_hdl)
{
    int32_t l_ret;

    l_ret = memcpy_s((uint8_t *)(&pst_mac_user->st_ht_hdl), sizeof(mac_user_ht_hdl_stru),
                     (uint8_t *)pst_ht_hdl, sizeof(mac_user_ht_hdl_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_set_ht_hdl::memcpy fail!");
    }
}
/*
 * 函 数 名  : mac_user_get_ht_hdl
 * 功能描述  : 获取用户的ht信息
 * 1.日    期  : 2015年5月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_get_ht_hdl(mac_user_stru *pst_mac_user, mac_user_ht_hdl_stru *pst_ht_hdl)
{
    int32_t l_ret;

    l_ret = memcpy_s((uint8_t *)pst_ht_hdl, sizeof(mac_user_ht_hdl_stru),
                     (uint8_t *)(&pst_mac_user->st_ht_hdl), sizeof(mac_user_ht_hdl_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "mac_user_get_ht_hdl::memcpy fail!");
    }
}

#ifdef _PRE_WLAN_FEATURE_SMPS
/*
 * 函 数 名  : mac_user_set_sm_power_save
 * 功能描述  : 设置用户的ht能力中sm_power_save的信息
 * 1.日    期  : 2015年5月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_sm_power_save(mac_user_stru *pst_mac_user, uint8_t uc_sm_power_save)
{
    pst_mac_user->st_ht_hdl.bit_sm_power_save = uc_sm_power_save;
}
#endif

/*
 * 函 数 名  : mac_user_set_pmf_active
 * 功能描述  : 设置与用户pmf能力协商的结果
 * 1.日    期  : 2015年5月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_pmf_active(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_pmf_active)
{
    pst_mac_user->st_cap_info.bit_pmf_active = en_pmf_active;
}
/*
 * 函 数 名  : mac_user_set_barker_preamble_mode
 * 功能描述  : 设置用户barker_preamble的模式
 * 1.日    期  : 2015年5月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_barker_preamble_mode(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_barker_preamble_mode)
{
    pst_mac_user->st_cap_info.bit_barker_preamble_mode = en_barker_preamble_mode;
}

/*
 * 函 数 名  : mac_user_set_qos
 * 功能描述  : 设置与用户的qos使能结果‘
 * 1.日    期  : 2015年5月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_qos(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_qos_mode)
{
    pst_mac_user->st_cap_info.bit_qos = en_qos_mode;
}

/*
 * 函 数 名  : mac_user_get_key
 * 功能描述  : 根据en_key_type，调用相应的函数，更新vap信息
 * 1.日    期  : 2014年11月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
wlan_priv_key_param_stru *mac_user_get_key(mac_user_stru *pst_mac_user, uint8_t uc_key_id)
{
    if (uc_key_id >= WLAN_NUM_TK + WLAN_NUM_IGTK) {
        return NULL;
    }
    return &pst_mac_user->st_key_info.ast_key[uc_key_id];
}

/*
 * 函 数 名  : mac_user_set_cur_bandwidth
 * 功能描述  : 设置mac user中的带宽信息
 * 1.日    期  : 2015年8月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_cur_bandwidth(mac_user_stru *pst_mac_user, wlan_bw_cap_enum_uint8 en_cur_bandwidth)
{
    pst_mac_user->en_cur_bandwidth = en_cur_bandwidth;
}

/*
 * 函 数 名  : mac_user_set_ht_capable
 * 功能描述  : 设置ht cap信息
 * 1.日    期  : 2015年5月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_ht_capable(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_ht_capable)
{
    pst_mac_user->st_ht_hdl.en_ht_capable = en_ht_capable;
}

/*
 * 函 数 名  : mac_user_set_spectrum_mgmt
 * 功能描述  : 设置更新用户bit_spectrum_mgmt能力的信息
 * 1.日    期  : 2015年5月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_spectrum_mgmt(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_spectrum_mgmt)
{
    pst_mac_user->st_cap_info.bit_spectrum_mgmt = en_spectrum_mgmt;
}
/*
 * 函 数 名  : mac_user_set_apsd
 * 功能描述  : 设置用户公共区域的bit_apsd能力
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mac_user_set_apsd(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_apsd)
{
    pst_mac_user->st_cap_info.bit_apsd = en_apsd;
}

/*
 * 函 数 名  : mac_user_update_wep_key
 * 功能描述  : 设置用户wep加密密钥信息
 * 1.日    期  : 2014年1月26日
 *   作    者  :
 *   修改内容  : 新生成函数
 */
uint32_t mac_user_update_wep_key(mac_user_stru *pst_mac_usr, uint16_t us_multi_user_idx)
{
    mac_user_stru *pst_multi_user = NULL;
    int32_t      l_ret;

    if (pst_mac_usr == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_multi_user = (mac_user_stru *)mac_res_get_mac_user(us_multi_user_idx);
    if (pst_multi_user == NULL) {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }

    if (pst_multi_user->st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_WEP_104 &&
        pst_multi_user->st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_WEP_40) {
        oam_error_log1(0, OAM_SF_WPA, "{mac_wep_add_usr_key::en_cipher_type==%d}",
                       pst_multi_user->st_key_info.en_cipher_type);
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    if (pst_multi_user->st_key_info.uc_default_index >= WLAN_MAX_WEP_KEY_COUNT) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    /* wep加密下，拷贝组播用户的密钥信息到单播用户 */
    l_ret = memcpy_s(&pst_mac_usr->st_key_info, sizeof(mac_key_mgmt_stru),
                     &pst_multi_user->st_key_info, sizeof(mac_key_mgmt_stru));
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_WPA, "mac_user_update_wep_key::memcpy fail!");
        return OAL_FAIL;
    }

    pst_mac_usr->st_user_tx_info.st_security.en_cipher_key_type =
        pst_mac_usr->st_key_info.uc_default_index + HAL_KEY_TYPE_PTK;  // 获取WEP default key id

    return OAL_SUCC;
}
/*
 * 函 数 名  : mac_addr_is_zero
 * 功能描述  : 判断mac地址是否全0
 * 1.日    期  : 2014年11月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
oal_bool_enum_uint8 mac_addr_is_zero(const unsigned char *puc_mac)
{
    uint8_t auc_mac_zero[OAL_MAC_ADDR_LEN] = {0};

    if (puc_mac == NULL) {
        return OAL_TRUE;
    }
    return (oal_memcmp(auc_mac_zero, puc_mac, OAL_MAC_ADDR_LEN) == 0);
}

/*
 * 函 数 名  : mac_res_get_mac_user
 * 功能描述  : 获取对应MAC USER索引的内存
 * 1.日    期  : 2015年11月12日
 *   作    者  :
 *   修改内容  : 新生成函数
 */
void *mac_res_get_mac_user(uint16_t us_idx)
{
    mac_user_stru *pst_mac_user = NULL;

    pst_mac_user = (mac_user_stru *)_mac_res_get_mac_user(us_idx);
    if (pst_mac_user == NULL) {
        return NULL;
    }

    /* 异常: 用户资源已被释放 */
    if (pst_mac_user->uc_is_user_alloced != MAC_USER_ALLOCED) {
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
        oam_warning_log2(0, OAM_SF_UM, "{mac_res_get_mac_user::[E]user has been freed,user_idx=%d.}", us_idx);
#endif
        /* device侧获取用户时用户已经释放属于正常，返回空指针，
         * 后续调用者查找用户失败，请打印WARNING并直接释放buf，走其他分支等等
         */
        return NULL;
    }

    return (void *)pst_mac_user;
}

