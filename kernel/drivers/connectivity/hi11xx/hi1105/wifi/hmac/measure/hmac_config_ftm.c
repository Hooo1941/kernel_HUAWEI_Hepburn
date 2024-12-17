/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 配置相关实现hmac接口实现源文件
 * 作    者 :
 * 创建日期 : 2013年1月8日
 */

/* 1 头文件包含 */
#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "mac_resource.h"
#include "mac_frame_inl.h"
#include "hmac_resource.h"
#include "hmac_vap.h"
#include "hmac_config.h"
#include "hmac_mgmt_join.h"
#include "hmac_roam_main.h"
#include "hmac_scan.h"
#include "hmac_sme_sta.h"
#include "external/oal_pm_qos_interface.h"

#include "securec.h"
#include "securectype.h"

#include "wlan_mib.h"
#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#include "hmac_ftm.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_FTM_C

/* FTM认证，由于需要访问扫描结果列表，因此该用例在host侧实现 */
#ifdef _PRE_WLAN_FEATURE_FTM
typedef enum {
    ANQP_INFO_ID_RESERVED        = 255,
    ANQP_INFO_ID_QUERY_LIST      = 256,
    ANQP_INFO_ID_AP_GEO_LOC      = 265,
    ANQP_INFO_ID_AP_CIVIC_LOC    = 266,
    ANQP_INFO_ID_BUTT,
} anqp_info_id_enum;

typedef struct {
    void *pst_hmac_vap;
} ftm_timer_arg_stru;

typedef struct {
    uint8_t  uc_dialog_token;
    uint16_t status_code;
    uint8_t  fragment_id;
    uint16_t comeback_delay;
    uint8_t  adv_protocol_ele;
    uint16_t query_resp_len;
} gas_init_resp_stru;

/*
 * 函 数 名  : hmac_encap_query_list
 * 功能描述  : 组装query list
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint16_t hmac_encap_query_list(uint8_t *puc_buffer, mac_send_gas_init_req_stru *pst_gas_req)
{
    uint16_t idx = 0;
    uint16_t *ptemp = NULL;

    /* AP支持AP_GEO_LOC， EC_IE_bit15 */
    if (pst_gas_req->en_geo_enable) {
        ptemp = (uint16_t *)&puc_buffer[idx];    /* AP Geospatial Location */
       *ptemp = ANQP_INFO_ID_AP_GEO_LOC;
        idx += BYTE_OFFSET_2;
    }

    /* AP支持AP_CIVIC_LOC， EC_IE_BIT14 */
    if (pst_gas_req->en_civic_enable) {
        ptemp = (uint16_t *)&puc_buffer[idx];    /* AP Civic Location */
       *ptemp = ANQP_INFO_ID_AP_CIVIC_LOC;
        idx += BYTE_OFFSET_2;
    }

    return idx;
}

/*
 * 函 数 名  : hmac_encap_anqp_query
 * 功能描述  : 组装anqp Query Request帧
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint16_t hmac_encap_anqp_query(uint8_t *puc_buffer, mac_send_gas_init_req_stru *pst_gas_req)
{
    uint16_t  idx   = 0;
    uint16_t *ptemp = NULL;

    /* Query Request - ANQP elements */
    /* **************************** */
    /* |Info ID|Length|Informatino| */
    /* ---------------------------- */
    /* |2      |2     |variable   | */
    /* **************************** */
    ptemp = (uint16_t *)&puc_buffer[idx];    /* Info ID - Query List, Table 9-271 */
    *ptemp = ANQP_INFO_ID_QUERY_LIST;
    idx += BYTE_OFFSET_2;

    ptemp = (uint16_t *)&puc_buffer[idx];    /* Length, 先跳过 */
    idx += BYTE_OFFSET_2;

    *ptemp = hmac_encap_query_list(&puc_buffer[idx], pst_gas_req);
    return 4 + *ptemp; /* Info ID 和 Length 的长度 4 */
}

/*
 * 函 数 名  : hmac_encap_gas_init_req
 * 功能描述  : 组装GAS Initial Request帧
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint16_t hmac_encap_gas_init_req(mac_vap_stru *pst_mac_vap,
                                            oal_netbuf_stru *pst_buffer,
                                            mac_send_gas_init_req_stru *pst_gas_req)
{
    uint8_t      *puc_mac_header   = oal_netbuf_header(pst_buffer);
    uint8_t      *puc_payload_addr = mac_get_80211_mgmt_payload(pst_buffer);
    uint16_t     *ptemp;
    uint16_t      us_index         = 0;
    hmac_vap_stru  *pst_hmac_vap     = NULL;

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_encap_gas_init_req::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
    mac_hdr_set_fragment_number(puc_mac_header, 0);
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_gas_req->auc_bssid);
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_gas_req->auc_bssid);

    /*************************************************************************************/
    /*         GAS Initial Request  frame - Frame Body                                   */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token  | Advertisement Protocol element |        */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1             |Variable                        |        */
    /* --------------------------------------------------------------------------------- */
    /* |Query Request length|Query Request|Multi-band (optional)                         */
    /* --------------------------------------------------------------------------------- */
    /* |2                   |variable     |                                              */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/
    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;      /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_GAS_INIT_REQ;            /* Public Action */
    puc_payload_addr[us_index++] = pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token;   /* Dialog Token */

    /*****************************************************************************************************/
    /*                   Advertisement Protocol element                                */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Advertisement Protocol Tuple #1| ... |Advertisement Protocol Tuple #n(optional) | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | variable                      |     |variable                              | */
    /* ------------------------------------------------------------------------------------------------- */
    /*                                                                                                   */
    /*****************************************************************************************************/
    puc_payload_addr[us_index++] = MAC_EID_ADVERTISEMENT_PROTOCOL;    /* Element ID */
    puc_payload_addr[us_index++] = 2;                                 /* Length ：2 */
    puc_payload_addr[us_index++] = 0;                                 /* Query Response Info */
    puc_payload_addr[us_index++] = 0;                                 /* Advertisement Protocol ID, 0: ANQP */

    /* Query Request length */
    ptemp = (uint16_t *)(&puc_payload_addr[us_index]);
    us_index += BYTE_OFFSET_2;

   *ptemp = hmac_encap_anqp_query(&puc_payload_addr[us_index], pst_gas_req);
    return (uint16_t)(us_index + MAC_80211_FRAME_LEN + *ptemp);
}

/*
 * 函 数 名  : check_interworking_support
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC mac_bss_dscr_stru *check_interworking_support(mac_vap_stru *pst_mac_vap,
    mac_send_gas_init_req_stru *pst_gas_req)
{
    hmac_bss_mgmt_stru      *pst_bss_mgmt;
    oal_dlist_head_stru     *pst_entry;
    hmac_scanned_bss_info   *pst_scanned_bss;
    mac_bss_dscr_stru       *pst_bss_dscr = NULL;
    mac_bss_dscr_stru       *pst_sel_dscr = NULL;

    /* 获取hmac device */
    hmac_device_stru *pst_hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_roam_scan_complete::device null!}");
        return NULL;
    }

    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 获取锁 */
    oal_spin_lock(&(pst_bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息，查找可以漫游的bss */
    oal_dlist_search_for_each(pst_entry, &(pst_bss_mgmt->st_bss_list_head)) {
        pst_scanned_bss = oal_dlist_get_entry(pst_entry, hmac_scanned_bss_info, st_dlist_head);
        pst_bss_dscr    = &(pst_scanned_bss->st_bss_dscr_info);

        if (!oal_compare_mac_addr(pst_gas_req->auc_bssid, pst_bss_dscr->auc_bssid)) {
            pst_sel_dscr = pst_bss_dscr;
            oam_warning_log0(0, OAM_SF_CFG, "{FTM Certificate:: Find AP device}");
        }

        pst_bss_dscr = NULL;
    }

    /* 解除锁 */
    oal_spin_unlock(&(pst_bss_mgmt->st_lock));

    return pst_sel_dscr;
}

static oal_netbuf_stru *hmac_ftm_encap_gas_init_req_frame(mac_vap_stru *mac_vap, mac_send_gas_init_req_stru *gas_req)
{
    oal_netbuf_stru *netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    mac_tx_ctl_stru *tx_ctl = NULL;
    uint16_t len;

    if (netbuf == NULL) {
        return NULL;
    }

    oal_mem_netbuf_trace(netbuf, OAL_TRUE);

    memset_s(oal_netbuf_cb(netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
    memset_s((uint8_t *)oal_netbuf_header(netbuf), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    len = hmac_encap_gas_init_req(mac_vap, netbuf, gas_req);

    oal_netbuf_put(netbuf, len);

    /* 为填写发送描述符准备参数 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_get_cb_mpdu_len(tx_ctl) = len;

    return netbuf;
}

static uint32_t hmac_ftm_update_gas_init_req(mac_bss_dscr_stru *dscr, mac_send_gas_init_req_stru *gas_req)
{
    uint8_t *ie = NULL;

    /* 寻找EC_IE信元 */
    ie = mac_find_ie(MAC_EID_EXT_CAPS, dscr->auc_mgmt_frame_body, dscr->mgmt_len - MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    if (ie == NULL || ie[1] < MAC_EXT_CAP_IE_INTERWORKING_OFFSET) {
        oam_warning_log0(0, OAM_SF_CFG, "{FTM Certificate:: Can't find EC_IE}");
        return OAL_FAIL;
    }

    /* 获取相关能力，BIT14， BIT15，BIT31， 先跳过ID与len字段 */
    gas_req->en_interworking_enable = (ie[BYTE_OFFSET_5] & 0x80) >> NUM_7_BITS;
    gas_req->en_civic_enable = (ie[BYTE_OFFSET_3] & 0x40) >> NUM_6_BITS;
    gas_req->en_geo_enable = (ie[BYTE_OFFSET_3] & 0x80) >> NUM_7_BITS;

    oam_warning_log3(0, OAM_SF_CFG, "{FTM Certificate:: interworking = %d, civic_enable = %d, geo_enable = %d}",
        gas_req->en_interworking_enable, gas_req->en_civic_enable, gas_req->en_geo_enable);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ftm_send_gas_init_req
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ftm_send_gas_init_req(mac_vap_stru *mac_vap, mac_send_gas_init_req_stru *gas_req)
{
    oal_netbuf_stru *gas_init_req_frame = NULL;
    mac_bss_dscr_stru *dscr = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_join_req_stru join_req = { { 0 } };

    if (oal_any_null_ptr2(mac_vap, gas_req)) {
        return OAL_FAIL;
    }

    dscr = check_interworking_support(mac_vap, gas_req);
    if (dscr == NULL) {
        return OAL_SUCC;
    }

    if (hmac_ftm_update_gas_init_req(dscr, gas_req) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (!gas_req->en_interworking_enable) {
        oam_warning_log0(0, OAM_SF_CFG, "{FTM Certificate:: AP does not support interworking!}");
        return OAL_SUCC;
    }

    /* 配置join参数 */
    hmac_prepare_join_req(&join_req, dscr);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_sta_update_join_req_params(hmac_vap, &join_req) != OAL_SUCC) {
        oam_error_log0(0, 0, "{hmac_ftm_send_gas_init_req::hmac_sta_update_join_req_params fail}");
        return OAL_FAIL;
    }

    /* 切换STA状态到JOIN_COMP */
    hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);

    /* 组装gas init req帧 */
    gas_init_req_frame = hmac_ftm_encap_gas_init_req_frame(mac_vap, gas_req);
    if (gas_init_req_frame == NULL) {
        return OAL_FAIL;
    }

    /* 抛事件让dmac将该帧发送 */
    if (hmac_tx_mgmt_send_event(mac_vap, gas_init_req_frame, oal_netbuf_len(gas_init_req_frame)) != OAL_SUCC) {
        oal_netbuf_free(gas_init_req_frame);
        oam_error_log0(0, 0, "{hmac_ftm_send_gas_init_req::hmac_tx_mgmt_send_event failed}");
        return OAL_FAIL;
    }

    /* 设置gas request frame sent标志位 */
    hmac_vap->st_gas_mgmt.uc_is_gas_request_sent = OAL_TRUE;

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_encap_gas_comeback_req
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint16_t hmac_encap_gas_comeback_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer)
{
    uint8_t      *p_mac_header   = oal_netbuf_header(buffer);
    uint8_t      *p_payload_addr = mac_get_80211_mgmt_payload(buffer);
    uint16_t      idx         = 0;
    mac_vap_stru   *pst_mac_vap      = &hmac_vap->st_vap_base_info;

    mac_hdr_set_frame_control(p_mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    mac_hdr_set_fragment_number(p_mac_header, 0);

    oal_set_mac_addr(p_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_mac_vap->auc_bssid);

    oal_set_mac_addr(p_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    oal_set_mac_addr(p_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /*************************************************************************************/
    /*        GAS Comeback Request  frame - Frame Body                                   */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token  | Multi-band (optional)  |        */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1             |Variable                        |        */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/
    p_payload_addr[idx++] = MAC_ACTION_CATEGORY_PUBLIC;                     /* Category */
    p_payload_addr[idx++] = MAC_PUB_GAS_COMBAK_REQ;                         /* Public Action */
    p_payload_addr[idx++] = hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token; /* 要填入AP gas resp帧中的Dialog Token */

    oam_warning_log1(0, OAM_SF_FTM,
        "{hmac_encap_gas_comeback_req::dialog token = %d", hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token);

    return (uint16_t)(idx + MAC_80211_FRAME_LEN);
}

/*
 * 函 数 名  : hmac_ftm_send_gas_comeback_req
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_ftm_send_gas_comeback_req(hmac_vap_stru *pst_hmac_vap)
{
    oal_netbuf_stru                    *pst_netbuf;
    mac_tx_ctl_stru                    *pst_tx_ctl;
    uint32_t                           ret;
    uint32_t                           len;
    mac_vap_stru                       *pst_mac_vap      = &pst_hmac_vap->st_vap_base_info;
    int32_t                            l_ret;

    /* 组装gas comeback req帧 */
    pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (pst_netbuf == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_send_gas_comeback_req::pst_gas_comeback_req_frame is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_mem_netbuf_trace(pst_netbuf, OAL_TRUE);

    l_ret = memset_s(oal_netbuf_cb(pst_netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    /* 将mac header清零 */
    l_ret += memset_s((uint8_t *)oal_netbuf_header(pst_netbuf), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);
    if (l_ret != EOK) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_send_gas_comeback_req::memcpy fail![%d]", l_ret);
    }

    len = hmac_encap_gas_comeback_req(pst_hmac_vap, pst_netbuf);

    oal_netbuf_put(pst_netbuf, len);

    /* 为填写发送描述符准备参数 */
    pst_tx_ctl                   = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    mac_get_cb_mpdu_len(pst_tx_ctl)      = (uint16_t)len;

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event(pst_mac_vap, pst_netbuf, (uint16_t)len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(pst_netbuf);
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{hmac_ftm_send_gas_comeback_req::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ftm_gas_comeback_timeout
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t  hmac_ftm_gas_comeback_timeout(void *p_arg)
{
    hmac_vap_stru *pst_hmac_vap;
    uint32_t ret;

    pst_hmac_vap = (hmac_vap_stru *)p_arg;
    if (pst_hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_ftm_send_gas_comeback_req(pst_hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_gas_comeback_timeout::hmac_ftm_send_gas_comeback_req fail![%d]", ret);
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ftm_start_gas_comeback_timer
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void  hmac_ftm_start_gas_comeback_timer(hmac_vap_stru *pst_hmac_vap, gas_init_resp_stru *pst_gas_init_resp)
{
    frw_timer_create_timer_m(&(pst_hmac_vap->st_ftm_timer),
        hmac_ftm_gas_comeback_timeout, (uint32_t)pst_gas_init_resp->comeback_delay, (void *)pst_hmac_vap,
        OAL_FALSE, OAM_MODULE_ID_HMAC, pst_hmac_vap->st_vap_base_info.core_id);
}

/*
 * 函 数 名  : hmac_ftm_rx_gas_initial_response_frame
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t  hmac_ftm_rx_gas_initial_response_frame(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *pst_rx_info;
    uint8_t *puc_data;
    gas_init_resp_stru st_gas_resp = { 0, 0, 0, 0, 0, 0 };
    uint16_t idx = 0;

    if (oal_unlikely((pst_hmac_vap == NULL) || (pst_netbuf == NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_ftm_rx_gas_initial_response_frame::it isn't for hmac, forward to wpa.}");
        return OAL_FAIL;
    }
    pst_rx_info = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    puc_data = (uint8_t *)mac_get_rx_cb_mac_header_addr(pst_rx_info) + pst_rx_info->uc_mac_header_len;

    idx += BYTE_OFFSET_2; // 跳过category、public action字段
    st_gas_resp.uc_dialog_token = *(uint8_t *)(&puc_data[idx]);
    if (st_gas_resp.uc_dialog_token != pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token) {
        oam_warning_log2(0, OAM_SF_RX, "{hmac_ftm_rx_gas_initial_response_frame::gas response frame's \
            dialog token [%d] != hmac gas dialog token [%d].}",
            st_gas_resp.uc_dialog_token, pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token);
        return OAL_FAIL;
    }

    idx += BYTE_OFFSET_1;

    st_gas_resp.status_code = *(uint16_t *)(&puc_data[idx]);
    idx += BYTE_OFFSET_2;

    st_gas_resp.comeback_delay = *(uint16_t *)(&puc_data[idx]);

    oam_warning_log4(0, OAM_SF_FTM,
        "{hmac_ftm_rx_gas_initial_response_frame::dialog token=%d, status code=%d, fragment_id=%d, comebakc_delay=%d",
        st_gas_resp.uc_dialog_token, st_gas_resp.status_code, st_gas_resp.fragment_id, st_gas_resp.comeback_delay);

    pst_hmac_vap->st_gas_mgmt.uc_gas_response_dialog_token = st_gas_resp.uc_dialog_token;

    // 启动comeback定时器
    if (st_gas_resp.comeback_delay > 0) {
        hmac_ftm_start_gas_comeback_timer(pst_hmac_vap, &st_gas_resp);
    }

    pst_hmac_vap->st_gas_mgmt.uc_gas_dialog_token++;
    pst_hmac_vap->st_gas_mgmt.uc_is_gas_request_sent = OAL_FALSE;

    return OAL_SUCC;
}

/*
 * 功能描述  : 根据ftm hipriv命令下发的format bw(9~13)设置对应wifi rtt的bw和preamle参数
 * 1.日    期  : 2020年09月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_config_ftm_dbg_set_bw_and_preamble(mac_ftm_debug_switch_stru* ftm_debug)
{
    switch (ftm_debug->st_send_iftmr_bit1.format_bw) {
        case FTM_FORMAT_BANDWIDTH_HTMIXED_20:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_20;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_HT;
            break;
        case FTM_FORMAT_BANDWIDTH_VHT20:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_20;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_VHT;
            break;
        case FTM_FORMAT_BANDWIDTH_HTMIXED_40:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_40;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_HT;
            break;
        case FTM_FORMAT_BANDWIDTH_VHT_40:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_40;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_VHT;
            break;
        case FTM_FORMAT_BANDWIDTH_VHT_80:
            ftm_debug->st_send_iftmr_bit1.bw = WIFI_RTT_BW_80;
            ftm_debug->st_send_iftmr_bit1.preamble = WIFI_RTT_PREAMBLE_VHT;
            break;
        default:
            oam_warning_log1(0, OAM_SF_FTM, "hmac_config_ftm_dbg_set_bw_and_preamble::invalid format_bw:%d",
                ftm_debug->st_send_iftmr_bit1.format_bw);
            break;
    }
}

/*
 * 功能描述  : 检查ftm命令下发参数的合法性
 * 1.日    期  : 2020年09月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ftm_check_dbg_params(mac_send_iftmr_stru* send_iftmr)
{
    if (mac_addr_is_zero(send_iftmr->mac_addr)) {
        oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_check_dbg_params::bssid should not be all zero!}");
        return OAL_FAIL;
    }
    if (mac_is_channel_num_valid(send_iftmr->st_channel.en_band, send_iftmr->channel_num,
        send_iftmr->st_channel.ext6g_band) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_check_dbg_params::channel num[%u] invalid!", send_iftmr->channel_num);
        return OAL_FAIL;
    }
    if (send_iftmr->burst_num > MAC_FTM_MAX_NUM_OF_BURSTS_EXPONENT) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_check_dbg_params::burst num[%u] over 16", send_iftmr->burst_num);
        return OAL_FAIL;
    }
    if (send_iftmr->ftms_per_burst > MAC_FTM_MAX_FTMS_PER_BURST) {
        oam_warning_log1(0, OAM_SF_FTM, "{hmac_ftm_check_dbg_params::ftms_per_burst[%u] over 10!}",
            send_iftmr->ftms_per_burst);
        return OAL_FAIL;
    }
    if ((send_iftmr->lci_request != 0 && send_iftmr->lci_request != 1) ||
        (send_iftmr->lcr_request != 0 && send_iftmr->lcr_request != 1)) {
        oam_warning_log2(0, OAM_SF_FTM, "hmac_ftm_check_dbg_params::lci_request[%u] lcr_request[%u] not valid,\
            should be 1 or 0", send_iftmr->lci_request, send_iftmr->lcr_request);
        return OAL_FAIL;
    }
    if ((send_iftmr->format_bw < FTM_FORMAT_BANDWIDTH_HTMIXED_20) ||
        (send_iftmr->format_bw > FTM_FORMAT_BANDWIDTH_VHT_80)) {
        oam_warning_log1(0, OAM_SF_FTM, "hmac_ftm_check_dbg_params::format_bw[%u] valid!", send_iftmr->format_bw);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_ftm_dbg
 * 功能描述  : 处理ftm配置命令
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_device_ftm_config(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t                      ret;
    mac_ftm_debug_switch_stru      *pst_ftm_debug;
    mac_bss_dscr_stru              *pst_bss_dscr = NULL;
    pst_ftm_debug = (mac_ftm_debug_switch_stru *)puc_param;

    /* 发送iftmr命令 */
    if (pst_ftm_debug->cmd_bit_map & BIT1) {
        pst_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(pst_mac_vap,
            pst_ftm_debug->st_send_iftmr_bit1.mac_addr);
        if (pst_bss_dscr != NULL) {
            if (pst_ftm_debug->st_send_iftmr_bit1.channel_num != pst_bss_dscr->st_channel.uc_chan_number) {
                oam_warning_log3(0, OAM_SF_FTM, "{hmac_config_ftm_dbg::send iftmr:AP [%02X:XX:XX:XX:%02X:%02X]",
                    pst_ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_0],
                    pst_ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_4],
                    pst_ftm_debug->st_send_iftmr_bit1.mac_addr[MAC_ADDR_5]);
                oam_warning_log2(0, OAM_SF_FTM, "{hmac_config_ftm_dbg::send iftmr: channel_num %d, channel %d.}",
                    pst_ftm_debug->st_send_iftmr_bit1.channel_num, pst_bss_dscr->st_channel.uc_chan_number);
                pst_ftm_debug->st_send_iftmr_bit1.channel_num = pst_bss_dscr->st_channel.uc_chan_number;
            }
            if (memcpy_s(&pst_ftm_debug->st_send_iftmr_bit1.st_channel, sizeof(mac_channel_stru),
                &pst_bss_dscr->st_channel, sizeof(mac_channel_stru)) != EOK) {
                oam_error_log0(0, OAM_SF_FTM, "hmac_config_ftm_dbg::st_channel memcpy fail!");
            }
            if (hmac_ftm_check_dbg_params(&pst_ftm_debug->st_send_iftmr_bit1) != OAL_SUCC) {
                return OAL_FAIL;
            }
            hmac_config_ftm_dbg_set_bw_and_preamble(pst_ftm_debug);
        }
    }

    /* gas init req,需要访问扫描列表，因此在host侧实现，消息不用抛到dmac */
    if (pst_ftm_debug->cmd_bit_map & BIT16) {
        return hmac_ftm_send_gas_init_req(pst_mac_vap, &pst_ftm_debug->st_gas_init_req_bit16);
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_FTM_DBG, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_ftm_dbg::hmac_config_send_event fail[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 获取ftm channel_bandwidth
 * 1.日    期  : 2020年09月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
wlan_channel_bandwidth_enum_uint8 hmac_ftm_get_channel_bandwidth(int32_t center_freq0, uint8_t channel_num,
    uint32_t channel_width)
{
    int32_t channel_center_freq0;
    wlan_channel_bandwidth_enum_uint8 channel_bandwidth;

    channel_center_freq0 = oal_ieee80211_frequency_to_channel(center_freq0);

    if (channel_width == MAC_FTM_WIFI_CHAN_WIDTH_80) {
        switch (channel_center_freq0 - (int8_t)channel_num) {
            case CHAN_OFFSET_PLUS_6: /* 中心频率相对于主20偏6个信道 */
                channel_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
                break;
            case CHAN_OFFSET_MINUS_2: /* 中心频率相对于主20偏-2个信道 */
                channel_bandwidth = WLAN_BAND_WIDTH_80PLUSMINUS;
                break;
            case CHAN_OFFSET_PLUS_2: /* 中心频率相对于主20偏2个信道 */
                channel_bandwidth = WLAN_BAND_WIDTH_80MINUSPLUS;
                break;
            case CHAN_OFFSET_MINUS_6: /* 中心频率相对于主20偏-6个信道 */
                channel_bandwidth = WLAN_BAND_WIDTH_80MINUSMINUS;
                break;
            default:
                channel_bandwidth = WLAN_BAND_WIDTH_20M;
                break;
        }
    } else if (channel_width == MAC_FTM_WIFI_CHAN_WIDTH_40) {
        switch (channel_center_freq0 - (int8_t)channel_num) {
            case CHAN_OFFSET_MINUS_2:
                channel_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                break;
            case CHAN_OFFSET_PLUS_2:
                channel_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                break;
            default:
                channel_bandwidth = WLAN_BAND_WIDTH_20M;
                break;
        }
    } else {
        channel_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    return channel_bandwidth;
}

/*
 * 功能描述  : 处理ftm测量命令
 * 1.日    期  : 2020年8月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_device_wifi_rtt_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t result;
    mac_ftm_wifi_rtt_config *wifi_rtt_config = NULL;
    mac_bss_dscr_stru *mac_bss_dscr = NULL;
    mac_send_iftmr_stru mac_send_iftmr;

    wifi_rtt_config = (mac_ftm_wifi_rtt_config *)param;
    memset_s(&mac_send_iftmr, sizeof(mac_send_iftmr_stru), 0, sizeof(mac_send_iftmr_stru));
    oal_set_mac_addr(mac_send_iftmr.mac_addr, wifi_rtt_config->addr);
    mac_send_iftmr.is_asap_on = OAL_TRUE;
    mac_send_iftmr.lci_request = wifi_rtt_config->lci_request;
    mac_send_iftmr.lcr_request = wifi_rtt_config->lcr_request;
    mac_send_iftmr.burst_num = wifi_rtt_config->num_burst;
    mac_send_iftmr.ftms_per_burst = wifi_rtt_config->num_frames_per_burst;
    mac_send_iftmr.burst_duration = wifi_rtt_config->burst_duration;
    mac_send_iftmr.burst_period = wifi_rtt_config->burst_period;
    mac_send_iftmr.channel_num = (uint8_t)(oal_ieee80211_frequency_to_channel(wifi_rtt_config->channel.center_freq));
    mac_send_iftmr.preamble = wifi_rtt_config->preamble;
    mac_send_iftmr.bw = wifi_rtt_config->bw;

    /* 发送iftmr命令 */
    mac_bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(mac_vap, wifi_rtt_config->addr);
    if (mac_bss_dscr != NULL) {
        mac_send_iftmr.channel_num = (uint8_t)oal_ieee80211_frequency_to_channel(wifi_rtt_config->channel.center_freq);
        if (mac_send_iftmr.channel_num != mac_bss_dscr->st_channel.uc_chan_number) {
            oam_warning_log2(0, OAM_SF_FTM, "{hmac_config_wifi_rtt_config::send iftmr: channel_num %d, chan_number %d}",
                mac_send_iftmr.channel_num, mac_bss_dscr->st_channel.uc_chan_number);
            oam_warning_log3(0, OAM_SF_FTM, "{hmac_config_wifi_rtt_config::send iftmr:AP [%02X:XX:XX:XX:%02X:%02X]",
                wifi_rtt_config->addr[MAC_ADDR_0], wifi_rtt_config->addr[MAC_ADDR_4],
                wifi_rtt_config->addr[MAC_ADDR_5]);
            mac_send_iftmr.channel_num = mac_bss_dscr->st_channel.uc_chan_number;
        }
        mac_send_iftmr.st_channel.uc_chan_number = mac_send_iftmr.channel_num;
        mac_send_iftmr.st_channel.en_band = mac_get_band_by_channel_num(mac_send_iftmr.channel_num);
        mac_send_iftmr.st_channel.en_bandwidth = hmac_ftm_get_channel_bandwidth(wifi_rtt_config->channel.center_freq0,
            mac_send_iftmr.channel_num, wifi_rtt_config->channel.width);
        mac_get_channel_idx_from_num(mac_send_iftmr.st_channel.en_band, mac_send_iftmr.st_channel.uc_chan_number,
            mac_send_iftmr.st_channel.ext6g_band, &(mac_send_iftmr.st_channel.uc_chan_idx));
    }

    /* 抛事件到DMAC层, 同步DMAC数据 */
    result = hmac_config_send_event(mac_vap, WLAN_CFGID_RTT_CONFIG, sizeof(mac_send_iftmr_stru),
        (uint8_t *)&mac_send_iftmr);
    if (oal_unlikely(result != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_wifi_rtt_config::hmac_config_send_event fail[%d]}", result);
        return result;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  :  wifi RTT上报测量结果
 * 1.日    期  : 2020年7月22日
 *   作    者 : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_rtt_result_report(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    frw_event_mem_stru *event_mem;
    frw_event_stru *event;
    uint32_t ul_ret;

    if ((mac_vap->uc_init_flag == MAC_VAP_INVAILD) || (param == NULL)) {
        oam_warning_log1(0, OAM_SF_FTM, "{hmac_wifi_rtt_result_report::pst_mac_vap->uc_init_flag[%d]!}",
            mac_vap->uc_init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛RTT wifi result上报事件到WAL */
    event_mem = frw_event_alloc_m(sizeof(mac_ftm_wifi_rtt_result));
    if (event_mem == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_wifi_rtt_result_report::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_RTT_RESULT_RPT,
        sizeof(mac_ftm_wifi_rtt_result), FRW_EVENT_PIPELINE_STAGE_0, mac_vap->uc_chip_id, mac_vap->uc_device_id,
        mac_vap->uc_vap_id);

    if (memcpy_s((uint8_t *)frw_get_event_payload(event_mem), sizeof(mac_ftm_wifi_rtt_result), param,
        len) != EOK) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_wifi_rtt_result_report::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(event_mem);
    if (ul_ret != OAL_SUCC) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_FTM, "{hmac_wifi_rtt_result_report::dispatch_event fail}");
    }
    frw_event_free_m(event_mem);

    return ul_ret;
}
#endif
