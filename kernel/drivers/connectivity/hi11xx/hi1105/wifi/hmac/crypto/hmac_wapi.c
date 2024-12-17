/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wpi数据加解密
 * 作    者 : wifi
 * 创建日期 : 2015年5月20日
 */
#ifdef _PRE_WLAN_FEATURE_WAPI

#include "oal_types.h"
#include "wlan_spec.h"
#include "mac_data.h"
#include "wlan_chip_i.h"
#include "oam_stat_wifi.h"
#include "hmac_wapi.h"
#include "hmac_sms4.h"
#include "hmac_wpi.h"
#include "hmac_rx_data.h"
#include "mac_frame_inl.h"
#include "hmac_wpi_mic.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WAPI_C
hmac_wapi_info_stru g_wapi_info;
const uint8_t g_auc_wapi_pn_init[WAPI_PN_LEN] = {
    0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36,
    0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c
};

#ifdef _PRE_WAPI_DEBUG
static void hmac_wapi_dump_frame(uint8_t *info, uint8_t *data, uint32_t len);
#endif

/*
 * 函 数 名  : hmac_wapi_is_keyidx_valid
 * 功能描述  : 判断keyidx是否合法
 * 1.日    期  : 2012年5月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_wapi_is_keyidx_valid(hmac_wapi_stru *hmac_wapi, uint8_t keyidx_rx)
{
    if (hmac_wapi->uc_keyidx != keyidx_rx && hmac_wapi->uc_keyupdate_flg != OAL_TRUE) {
        oam_warning_log3(0, OAM_SF_ANY, "{hmac_wapi_is_keyidx_valid::keyidx==%u, keyidx_rx==%u, update==%u.}",
                         hmac_wapi->uc_keyidx, keyidx_rx, hmac_wapi->uc_keyupdate_flg);
        wapi_rx_idx_update_err(hmac_wapi);
        return OAL_FALSE;
    }

    hmac_wapi->uc_keyupdate_flg = OAL_FALSE; /* 更新完成取消标志 */

    /* key没有启用 */
    if (hmac_wapi->ast_wapi_key[keyidx_rx].uc_key_en != OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_wapi_is_keyidx_valid::keyen==%u.}",
                         hmac_wapi->ast_wapi_key[keyidx_rx].uc_key_en);
        wapi_rx_idx_update_err(hmac_wapi);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * 函 数 名  : hmac_check_wapi_ucast_rx_pn
 * 功能描述  : 单播帧判断数据奇偶正确性
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 创建
 */
uint8_t hmac_check_wapi_ucast_rx_pn(uint8_t *user_pn, uint8_t *buf_pn)
{
    return ((*buf_pn & BIT0) == 0) ? PN_TYPE_INVALID : PN_TYPE_VALID;
}

/*
 * 函 数 名  : hmac_check_wapi_bcast_rx_pn
 * 功能描述  : 组播帧判断数据奇偶性
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 创建
 */
uint8_t hmac_check_wapi_bcast_rx_pn(uint8_t *user_pn, uint8_t *buf_pn)
{
    return PN_TYPE_VALID;
}
/*
 * 函 数 名  : hmac_wapi_netbuff_tx_handle
 * 功能描述  : 处理已经分片或者不需分片的netbuff链
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 创建
 */
 /* 待加密netbuf链表buf在调用该函数的地方释放内存，本函数里不释放内存 */
oal_netbuf_stru *hmac_wapi_encrypt_netbuf(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    oal_netbuf_stru *netbuf_encrypt = NULL; /* 临时变量，获取已加密netbuf链表节点 */
    oal_netbuf_stru *netbuf_temp = netbuf; /* 临时变量，用于遍历待加密netbuf链表 */
    oal_netbuf_stru *netbuf_next = NULL;    /* 临时变量，用于保存已加密netbuf链表节点，始终处于链表尾部 */
    oal_netbuf_stru *netbuf_head = NULL;    /* 返回结果，指向第一个已加密netbuf链表节点 */
    hmac_wapi_info_stru *wap_info = hmac_get_wapi_info();

    if (hmac_user->st_wapi.uc_port_valid == OAL_FALSE) {
        return netbuf;
    }

    /* buf的初始位置在snap头的llc处 */
    if (MAC_DATA_WAPI == mac_get_data_type_from_80211(netbuf, 0, oal_netbuf_get_len(netbuf))) {
        oam_warning_log0(0, OAM_SF_WAPI, "{hmac_wapi_encrypt_netbuf::wapi, don't encrypt!.}");
        return netbuf;
    }

    if (wap_info->wapi_encrypt == NULL) {
        oam_error_log0(0, OAM_SF_WAPI, "hmac_wapi_encrypt_netbuf:wap_info->wapi_encrypt is NULL");
        return NULL;
    }
    while (netbuf_temp != NULL) {
        netbuf_encrypt = wap_info->wapi_encrypt(hmac_user, netbuf_temp);
        if (netbuf_encrypt == NULL) {
            hmac_free_netbuf_list(netbuf_head);
            return NULL;
        }
        if (netbuf_head == NULL) {
            netbuf_head = netbuf_encrypt;
            netbuf_next = netbuf_head;
        } else {
            oal_netbuf_next(netbuf_next) = netbuf_encrypt;
            netbuf_next = netbuf_encrypt;
        }
        oal_netbuf_next(netbuf_next) = NULL;
        netbuf_temp = oal_netbuf_next(netbuf_temp);
    }
    return netbuf_head;
}

/*
 * 函 数 名  : hmac_wapi_netbuf_tx_encrypt_mp16
 * 功能描述  ： mp16 host_tx场景，加密已经封装为802.11格式的数据帧
 */
oal_netbuf_stru *hmac_wapi_netbuf_tx_encrypt_mp16(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf)
{
    hmac_wapi_stru *hmac_user_wapi = NULL;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint32_t mac_hdr_len;
    int32_t ret;

    if (!is_legacy_sta(&hmac_vap->st_vap_base_info) ||
        (wlan_chip_is_support_hw_wapi() == OAL_TRUE)) {
        return netbuf;
    }

    hmac_user_wapi = &hmac_user->st_wapi;
    if (wapi_port_flag(hmac_user_wapi) != OAL_TRUE) {
        return netbuf;
    }

    mac_hdr_len = mac_get_cb_frame_header_length(tx_ctl);
    oal_netbuf_pull(netbuf, mac_hdr_len); /* WAPI加密host_tx,需将netbuff->data指向SNAP头 */
    netbuf = hmac_wpi_encrypt_tx_data(hmac_user, netbuf);
    if (netbuf == NULL) {
        oam_stat_vap_incr(hmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{hmac_wapi_netbuf_tx_encrypt_mp16 fail!}");
        return NULL;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf); /* WAPI加密后返回新netbuf，更新tx_ctl指针 */

    /* MAC header和snap头部分存在空洞，需要调整mac_header内容位置 */
    ret = memmove_s(oal_netbuf_data(netbuf) - mac_hdr_len, mac_hdr_len,
        mac_get_cb_frame_header_addr(tx_ctl), mac_hdr_len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WAPI,
            "hmac_wapi_netbuf_tx_encrypt_mp16:memmove fail");
    }
    oal_netbuf_push(netbuf, mac_hdr_len); /* WAPI加密后，将netbuf->data重新指向mac_header */

    /* 记录新的MAC头的位置 */
    mac_get_cb_frame_header_addr(tx_ctl) = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);

    return netbuf;
}

/*
 * 函 数 名  : hmac_wapi_netbuff_rx_handle
 * 功能描述  : 接收处理比发送要简单，因为每次只会有一个netbuff需要处理
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 创建
 */
static oal_netbuf_stru *hmac_wapi_netbuff_rx_handle(hmac_user_stru *hmac_user, oal_netbuf_stru *buf)
{
    uint32_t ret;
    hmac_wapi_info_stru *wap_info = hmac_get_wapi_info();
    oal_netbuf_stru *netbuf_tmp = NULL; /* 指向需要释放的netbuff */

    /* 非加密帧，不进行解密 */
    if (!((oal_netbuf_data(buf))[1] & 0x40)) {
        return buf;
    }

    if (wap_info->wapi_decrypt == NULL) {
        oal_netbuf_free(buf);
        oam_error_log0(0, OAM_SF_WAPI, "hmac_wapi_netbuff_rx_handle:wap_info->wapi_decrypt is NULL");
        return NULL;
    }
    ret = wap_info->wapi_decrypt(hmac_user, buf);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(buf);
        return NULL;
    }

    netbuf_tmp = buf;
    buf = oal_netbuf_next(buf);
    oal_netbuf_free(netbuf_tmp);

    return buf;
}

/*
 * 函 数 名  : hmac_wapi_netbuf_rx_decrypt_mp16
 * 功能描述  : mp16 接收，解密802.11格式的数据帧
 */
uint32_t hmac_wapi_netbuf_rx_decrypt_mp16(hmac_host_rx_context_stru *rx_context)
{
    hmac_vap_stru *hmac_vap = rx_context->hmac_vap;
    hmac_user_stru *hmac_user = rx_context->hmac_user;
    oal_netbuf_stru *netbuf = rx_context->netbuf;
    mac_rx_ctl_stru *rx_ctrl = rx_context->cb;
    hmac_wapi_info_stru *wap_info = hmac_get_wapi_info();

    if (wlan_chip_is_support_hw_wapi() == OAL_TRUE ||
        rx_ctrl->bit_frame_format != MAC_FRAME_TYPE_80211 ||
        rx_ctrl->bit_amsdu_enable == OAL_TRUE ||
        !is_legacy_sta(&hmac_vap->st_vap_base_info)) {
        return OAL_SUCC;
    }

    if ((wapi_port_flag(&hmac_user->st_wapi) != OAL_TRUE) || (wap_info->wapi_netbuff_rxhandle == NULL)) {
        return OAL_SUCC;
    }

    netbuf = wap_info->wapi_netbuff_rxhandle(hmac_user, netbuf);
    if (netbuf == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{hmac_wapi_netbuf_rx_decrypt_mp16::wapi decrypt FAIL!}");
        HMAC_USER_STATS_PKT_INCR(hmac_user->rx_pkt_drop, 1);
        return OAL_ERR_CODE_WAPI_DECRYPT_FAIL;
    }

    /* WAPI解密完成后，返回新的netbuff，需要重新更新rx_context的netbuf/cb指针 */
    rx_context->netbuf = netbuf;
    rx_context->cb = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_user_get_wapi_key
 * 功能描述  : 通用流程，获取key
 */
OAL_STATIC uint8_t *hmac_user_get_wapi_key(hmac_user_stru *hmac_user, uint8_t key_index, uint8_t key_type)
{
    if ((key_index >= HMAC_WAPI_MAX_KEYID) || (key_type >= KEY_TYPE_BUTT)) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_wapi_get_key:invalid key_index[%d], key_type[%d]", key_index, key_type);
        return NULL;
    }
    return (key_type == KEY_TYPE_MIC) ? hmac_user->st_wapi.ast_wapi_key[key_index].auc_wpi_ck :\
        hmac_user->st_wapi.ast_wapi_key[key_index].auc_wpi_ek;
}

/*
 * 函 数 名  : hmac_user_get_wapi_key_index
 * 功能描述  : 加密流程，获取keyIndex(keyIndex为add key时候下发)
 */
OAL_STATIC uint8_t hmac_user_get_wapi_tx_key_index(hmac_user_stru *hmac_user)
{
    return hmac_user->st_wapi.uc_keyidx;
}

/*
 * 函 数 名  : hmac_user_get_wapi_rx_key_index
 * 功能描述  : 解密流程，获取keyIndex
 */
OAL_STATIC uint8_t hmac_user_get_wapi_rx_key_index(uint8_t *buf)
{
    return ((wapi_head_stru *)buf)->key_index;
}
/*
 * 函 数 名  : hmac_user_get_wapi_rx_pc
 * 功能描述  : 解密流程，获取keyIndex 计算MIC时使用
 */
OAL_STATIC uint16_t hmac_user_get_wapi_rx_pc(uint8_t *buf)
{
    return (uint16_t)(((wapi_head_stru *)buf)->key_index);
}
/*
 * 函 数 名  : hmac_user_get_wapi_rx_pn
 * 功能描述  : 解密流程，获取pn
 */
OAL_STATIC uint8_t* hmac_user_get_wapi_rx_pn(uint8_t *buf)
{
    return ((wapi_head_stru *)buf)->pn;
}

/*
 * 函 数 名  : hmac_user_get_wapi_tx_pn
 * 功能描述  : 加密流程，获取pn
 */
OAL_STATIC uint8_t* hmac_user_get_wapi_tx_pn(hmac_user_stru *hmac_user)
{
    if (hmac_user->st_wapi.uc_keyidx >= HMAC_WAPI_MAX_KEYID) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_user_get_wapi_tx_pn:key_index invalid [%d]", hmac_user->st_wapi.uc_keyidx);
        return NULL;
    }
    return hmac_user->st_wapi.ast_wapi_key[hmac_user->st_wapi.uc_keyidx].auc_pn_tx;
}
/*
 * 函 数 名  : hmac_user_get_user_wapi_rx_pn
 * 功能描述  : 解密流程，获取pn
 */
OAL_STATIC uint8_t* hmac_user_get_user_wapi_rx_pn(hmac_user_stru *hmac_user, mac_rx_ctl_stru *rx_ctl)
{
    if (hmac_user->st_wapi.uc_keyidx >= HMAC_WAPI_MAX_KEYID) {
        oam_error_log1(0, 0, "hmac_user_get_user_wapi_rx_pn:key_index invalid [%d]", hmac_user->st_wapi.uc_keyidx);
        return NULL;
    }
    return hmac_user->st_wapi.ast_wapi_key[hmac_user->st_wapi.uc_keyidx].auc_pn_rx;
}

/*
 * 函 数 名  : hmac_user_get_wapi_pn_inc
 * 功能描述  : 加密流程，获取步长 pn_inc
 */
OAL_STATIC uint16_t hmac_user_get_wapi_pn_inc(hmac_user_stru *hmac_user)
{
    return (uint16_t)(hmac_user->st_wapi.uc_pn_inc);
}
/*
 * 函 数 名  : hmac_wapi_update_pn
 * 功能描述  : 更新pn
 */
void hmac_wapi_update_pn(uint8_t *pn, uint8_t inc)
{
    uint32_t loop;
    uint32_t loop_num;
    uint32_t overlow; /* 进位 */
    uint32_t *pn_temp;

    pn_temp = (uint32_t *)pn;
    loop_num = WAPI_PN_LEN / sizeof(uint32_t);
    overlow = inc;

    for (loop = 0; loop < loop_num; loop++) {
        if (*pn_temp > (*pn_temp + overlow)) {
            *pn_temp += overlow;
            overlow = 1; /* 溢出高位加1 */
        } else {
            *pn_temp += overlow;
            break;
        }
        pn_temp++;
    }
}

/*
 * 函 数 名  : hmac_user_set_wapi_head
 * 功能描述  : 加密流程，设置WAPI协议头
 */
OAL_STATIC uint32_t hmac_user_set_wapi_head(hmac_encrypt_params_stru *encrypt_params, oal_netbuf_stru *netbuf)
{
    /* netbuffer->data指向802.11帧头，偏移MAC_80211_QOS_HTC_4ADDR_FRAME_LEN字节填充WPI 头 */
    wapi_head_stru *wapi_hdr = (wapi_head_stru *)(oal_netbuf_data(netbuf) + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    memset_s((uint8_t *)wapi_hdr, sizeof(wapi_head_stru), 0, sizeof(wapi_head_stru));
    /* 填写key_index */
    wapi_hdr->key_index = (uint8_t)(encrypt_params->pc & 0x00ff);
    /* 填写PN */
    if (memcpy_s(wapi_hdr->pn, WAPI_PN_LEN, encrypt_params->tx_pn, WAPI_PN_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_user_get_wapi_tx_pn:memcpy tx pn fail");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_user_reset_wapi_cb
 * 功能描述  : 去使能user下PWL钩子
 */
OAL_STATIC void hmac_user_reset_wapi_cb(hmac_user_stru *hmac_user)
{
    /* 通用流程 */
    hmac_user->wpi_cb.hmac_wpi_get_key_cb = NULL;

    /* 加密流程 */
    hmac_user->wpi_cb.hmac_wpi_get_tx_key_index_cb = NULL;
    hmac_user->wpi_cb.hmac_wpi_get_tx_pn_cb = NULL;
    hmac_user->wpi_cb.hmac_wpi_get_pn_inc = NULL;

    hmac_user->wpi_cb.hmac_wpi_set_head_cb = NULL;

    /* 解密流程 */
    hmac_user->wpi_cb.hmac_wpi_get_rx_key_index_cb = NULL;
    hmac_user->wpi_cb.hmac_wpi_get_rx_pn_cb = NULL;
    hmac_user->wpi_cb.hmac_wpi_get_rx_pc_cb = NULL;
    hmac_user->wpi_cb.hmac_wpi_get_user_rx_pn_cb = NULL;
    hmac_user->wpi_cb.hmac_wpi_check_rx_pn = NULL;
}

/*
 * 函 数 名  : hmac_user_init_pwl_cb
 * 功能描述  : 设置user下PWL正式秘钥钩子
 */
OAL_STATIC void hmac_user_init_wapi_cb(hmac_user_stru *hmac_user, oal_bool_enum_uint8 pairwise)
{
    /* 通用流程 */
    hmac_user->wpi_cb.hmac_wpi_get_key_cb = hmac_user_get_wapi_key;

    /* 加密流程 */
    hmac_user->wpi_cb.hmac_wpi_get_tx_key_index_cb = hmac_user_get_wapi_tx_key_index;
    hmac_user->wpi_cb.hmac_wpi_get_tx_pn_cb = hmac_user_get_wapi_tx_pn;
    hmac_user->wpi_cb.hmac_wpi_get_pn_inc = hmac_user_get_wapi_pn_inc;
    hmac_user->wpi_cb.hmac_wpi_set_head_cb = hmac_user_set_wapi_head;
    hmac_user->wpi_cb.encrypt_data = hmac_wpi_encrypt_data;

    /* 解密流程 */
    hmac_user->wpi_cb.hmac_wpi_get_rx_key_index_cb = hmac_user_get_wapi_rx_key_index;
    hmac_user->wpi_cb.hmac_wpi_get_rx_pn_cb = hmac_user_get_wapi_rx_pn;
    hmac_user->wpi_cb.hmac_wpi_get_rx_pc_cb = hmac_user_get_wapi_rx_pc;
    hmac_user->wpi_cb.hmac_wpi_get_user_rx_pn_cb = hmac_user_get_user_wapi_rx_pn;
    hmac_user->wpi_cb.decrypt_data = hmac_wpi_decrypt_data;
    hmac_user->wpi_cb.hmac_wpi_check_rx_pn = (pairwise ? hmac_check_wapi_ucast_rx_pn : hmac_check_wapi_bcast_rx_pn);
}

/*
 * 函 数 名  : hmac_wapi_add_key
 * 功能描述  : 增加/更新 key
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 创建
 */
uint32_t hmac_wapi_add_key(hmac_user_stru *hmac_user, oal_bool_enum_uint8 pairwise, uint8_t key_index, uint8_t *key)
{
    hmac_wapi_key_stru *wapi_key = NULL;
    hmac_wapi_stru *hmac_wapi = &(hmac_user->st_wapi);
    int32_t l_ret;

    hmac_wapi_set_port(hmac_wapi, OAL_TRUE);
    hmac_wapi->uc_keyidx = key_index;
    hmac_wapi->uc_keyupdate_flg = OAL_TRUE;
    wapi_key = &(hmac_wapi->ast_wapi_key[key_index]);

    l_ret = memcpy_s(wapi_key->auc_wpi_ek, WAPI_KEY_LEN, key, WAPI_KEY_LEN);
    l_ret += memcpy_s(wapi_key->auc_wpi_ck, WAPI_KEY_LEN, key + WAPI_KEY_LEN, WAPI_KEY_LEN);
    wapi_key->uc_key_en = OAL_TRUE;

    /* 重置PN */
    l_ret += memcpy_s(wapi_key->auc_pn_rx, WAPI_PN_LEN, g_auc_wapi_pn_init, WAPI_PN_LEN);
    l_ret += memcpy_s(wapi_key->auc_pn_tx, WAPI_PN_LEN, g_auc_wapi_pn_init, WAPI_PN_LEN);
    if (l_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_wapi_add_key::memcpy fail!");
        return OAL_FAIL;
    }
    hmac_user_init_wapi_cb(hmac_user, pairwise);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_user_wapi_deinit
 * 功能描述  : 去初始化wapi对象
 * 1.日    期  : 2015年5月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_user_wapi_deinit(hmac_user_stru *hmac_user)
{
    memset_s(&(hmac_user->st_wapi), sizeof(hmac_wapi_stru), 0, sizeof(hmac_wapi_stru));
    hmac_user_reset_wapi_cb(hmac_user);
}

/*
 * 函 数 名  : hmac_wapi_set_port
 * 功能描述  : 将port置位
 * 1.日    期  : 2015年5月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_wapi_set_port(hmac_wapi_stru *hmac_wapi, oal_bool_enum_uint8 value)
{
    hmac_wapi->uc_port_valid = value;
}

/*
 * 函 数 名  : hmac_user_wapi_init
 * 功能描述  : 初始化wapi对象
 * 1.日    期  : 2015年5月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_user_wapi_init(hmac_user_stru *hmac_user, uint8_t uc_pairwise)
{
    uint32_t loop;
    hmac_wapi_stru *hmac_wapi = &(hmac_user->st_wapi);
    hmac_user_wapi_deinit(hmac_user);

    if (uc_pairwise == OAL_TRUE) {
        hmac_wapi->uc_pn_inc = WAPI_UCAST_INC;
    } else {
        hmac_wapi->uc_pn_inc = WAPI_BCAST_INC;
    }

    for (loop = 0; loop < HMAC_WAPI_MAX_KEYID; loop++) {
        hmac_wapi->ast_wapi_key[loop].uc_key_en = OAL_FALSE;
    }

    hmac_wapi_set_port(hmac_wapi, OAL_FALSE);
    return OAL_SUCC;
}

void hmac_wapi_init(void)
{
    g_wapi_info.wapi_decrypt = hmac_wpi_decrypt_rx_data;
    g_wapi_info.wapi_encrypt = hmac_wpi_encrypt_tx_data;
    g_wapi_info.wapi_netbuff_rxhandle = hmac_wapi_netbuff_rx_handle;
}

hmac_wapi_info_stru *hmac_get_wapi_info(void)
{
    return &g_wapi_info;
}
#ifdef _PRE_WAPI_DEBUG
/*
 * 函 数 名  : hmac_wapi_dump_frame
 * 功能描述  : 打印帧内容
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 创建
 */
static void hmac_wapi_dump_frame(uint8_t *info, uint8_t *data, uint32_t len)
{
    uint32_t loop;
    oal_io_print("%s: \r\n", info);
    for (loop = 0; loop < len; loop += 4) { // 按4字节轮询
        oal_io_print("%2x ", loop / 4); // 按4字节轮询
        oal_io_print("%2x %2x %2x %2x \r\n", data[loop], data[loop + 1],
                     data[loop + BYTE_OFFSET_2], data[loop + BYTE_OFFSET_3]);
    }
}

/*
 * 函 数 名  : hmac_wapi_display_usr_info
 * 功能描述  : 打印用户wapi内容
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 创建
 */
void hmac_wapi_display_usr_info(hmac_user_stru *hmac_user)
{
    uint32_t loop;
    hmac_wapi_stru *hmac_wapi = NULL;
    hmac_wapi_key_stru *wapi_key = NULL;
    hmac_wapi_debug *wapi_debug = NULL;

    oam_warning_log1(0, OAM_SF_ANY, "wapi port is %u!", wapi_port_flag(&hmac_user->st_wapi));
    if (wapi_port_flag(&hmac_user->st_wapi) != OAL_TRUE) {
        oal_io_print("Err! wapi port is not valid!\n");

        return;
    }

    hmac_wapi = &(hmac_user->st_wapi);
    oam_warning_log0(0, OAM_SF_ANY, "keyidx\tupdate\t\tpn_inc\t\n");
    oam_warning_log3(0, OAM_SF_ANY, "%u\t%08x%04x\t\n",
                     hmac_wapi->uc_keyidx,
                     hmac_wapi->uc_keyupdate_flg,
                     hmac_wapi->uc_pn_inc);

    for (loop = 0; loop < HMAC_WAPI_MAX_KEYID; loop++) {
        wapi_key = &hmac_wapi->ast_wapi_key[loop];
        hmac_wapi_dump_frame("ek :", wapi_key->auc_wpi_ek, WAPI_KEY_LEN);
        hmac_wapi_dump_frame("ck :", wapi_key->auc_wpi_ck, WAPI_KEY_LEN);
        hmac_wapi_dump_frame("pn_local_rx :", wapi_key->auc_pn_rx, WAPI_PN_LEN);
        hmac_wapi_dump_frame("pn_local_tx :", wapi_key->auc_pn_tx, WAPI_PN_LEN);
        oam_warning_log1(0, OAM_SF_ANY, "key_en: %u\n", wapi_key->uc_key_en);
    }

    wapi_debug = &hmac_wapi->st_debug;
    oam_warning_log0(0, OAM_SF_ANY, "TX DEBUG INFO:");
    hmac_wapi_dump_frame("pn_rx :", wapi_debug->aucrx_pn, WAPI_PN_LEN);
    oam_warning_log4(0, OAM_SF_ANY, "tx_drop==%u, tx_wai==%u, tx_port_valid==%u, tx_memalloc_fail==%u",
                     wapi_debug->ultx_ucast_drop,
                     wapi_debug->ultx_wai,
                     wapi_debug->ultx_port_valid,
                     wapi_debug->ultx_memalloc_err);
    oam_warning_log3(0, OAM_SF_ANY, "tx_mic_calc_fail==%u, tx_encrypt_ok==%u, tx_memalloc_err==%u",
                     wapi_debug->ultx_mic_calc_fail,
                     // wapi_debug->ultx_drop_wai,
                     wapi_debug->ultx_encrypt_ok,
                     wapi_debug->ultx_memalloc_err);

    oam_warning_log0(0, OAM_SF_ANY, "RX DEBUG INFO:");
    oam_warning_log4(0, OAM_SF_ANY, "rx_port_valid==%u, rx_idx_err==%u, rx_netbuff_len_err==%u, rx_idx_update_err==%u",
                     wapi_debug->ulrx_port_valid,
                     wapi_debug->ulrx_idx_err,
                     wapi_debug->ulrx_netbuff_len_err,
                     wapi_debug->ulrx_idx_update_err);

    oam_warning_log4(0, OAM_SF_ANY, "rx_key_en_err==%u, rx_pn_odd_err==%u, rx_pn_replay_err==%u, rx_decrypt_ok==%u",
                     wapi_debug->ulrx_key_en_err,
                     wapi_debug->ulrx_pn_odd_err,
                     wapi_debug->ulrx_pn_replay_err,
                     wapi_debug->ulrx_decrypt_ok);
    return;
}

/*
 * 函 数 名  : hmac_wapi_display_info
 * 功能描述  : 打印wapi内容
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 创建
 */
uint32_t hmac_wapi_display_info(mac_vap_stru *pst_mac_vap, uint16_t us_usr_idx)
{
    hmac_user_stru *hmac_user = NULL;
    hmac_user_stru *hmac_multi_user;

    hmac_multi_user = (hmac_user_stru *)mac_res_get_hmac_user(pst_mac_vap->us_multi_user_idx);
    if (hmac_multi_user == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "Err! multi usr %u does not exist!",
                         pst_mac_vap->us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "*****************multi usr info start****************");
    hmac_wapi_display_usr_info(hmac_multi_user);
    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "*****************multi usr info end****************");

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_usr_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "Err! ucast usr %u does not exist!", us_usr_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "*****************ucast usr info start****************");
    hmac_wapi_display_usr_info(hmac_user);
    oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "*****************ucast usr info end****************");

    return OAL_SUCC;
}

/* hipriv命令配置WAPI硬件加密调试开关。0：关闭WAPI硬件加密;1:打开WAPI硬件加密 */
uint32_t hmac_wapi_set_debug_hw_wapi(mac_vap_stru *mac_vap, uint32_t *params)
{
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    mac_device->debug_hw_wapi = !!(*params);
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_WAPI, "hmac_wapi_set_debug_hw_wapi::set hw_wapi %u",
        mac_device->debug_hw_wapi);
    return OAL_SUCC;
}

#endif /* _PRE_WAPI_DEBUG */

#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */
