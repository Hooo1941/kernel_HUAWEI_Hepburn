/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : AP侧管理面处理
 * 作    者 :
 * 创建日期 : 2013年6月18日
 */

/* 1 头文件包含 */
#include "oal_cfg80211.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "hmac_mgmt_sta_up.h"
#include "hmac_mgmt_ap.h"
#include "hmac_encap_frame_ap.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_frame.h"
#include "hmac_rx_data.h"
#include "hmac_uapsd.h"
#include "hmac_tx_amsdu.h"
#include "mac_ie.h"
#include "mac_user.h"
#include "hmac_user.h"
#include "hmac_11i.h"
#include "hmac_protection.h"
#include "hmac_chan_mgmt.h"
#include "hmac_smps.h"
#include "hmac_fsm.h"
#include "hmac_ext_if.h"
#include "hmac_config.h"
#include "hmac_blacklist.h"
#include "hmac_dfs.h"
#include "hmac_p2p.h"
#include "hmac_blockack.h"
#include "hmac_ext_if.h"
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif
#include "hmac_dyn_pcie.h"
#include "securec.h"
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_11ax.h"
#endif
#include "hmac_11w.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_sync.h"
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
#include "hmac_pwl.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_AP_C

/* 2 全局变量定义 */
/* 3 函数实现 */
/*
 * 函 数 名  : hmac_handle_disconnect_rsp_ap
 * 功能描述  : AP(驱动)上报去关联某个STA
 * 1.日    期  : 2013年9月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_handle_disconnect_rsp_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_device_stru *mac_device = NULL;
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_handle_disconnect_rsp_ap::mac_device null.}");
        return;
    }

    if (hmac_user->en_report_kernel == OAL_FALSE) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "hmac_handle_disconnect_rsp_ap::do not report kernel");
        return;
    }

    /* 抛扫描完成事件到WAL */
    event_mem = frw_event_alloc_m(WLAN_MAC_ADDR_LEN);
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_handle_disconnect_rsp_ap::event_mem null.}");
        return;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);
    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_DISCONNECT_AP,
                       WLAN_MAC_ADDR_LEN,
                       FRW_EVENT_PIPELINE_STAGE_0,
                       hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id,
                       hmac_vap->st_vap_base_info.uc_vap_id);

    /* 去关联的STA mac地址 */
    if (memcpy_s(frw_get_event_payload(event_mem), WLAN_MAC_ADDR_LEN,
        (uint8_t *)hmac_user->st_user_base_info.auc_user_mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ASSOC, "hmac_handle_disconnect_rsp_ap::memcpy fail!");
        frw_event_free_m(event_mem);
        return;
    }

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);
}

/*
 * 函 数 名  : hmac_handle_connect_rsp_ap
 * 功能描述  : AP(驱动)上报新关联上某个STA
 * 1.日    期  : 2013年9月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_handle_connect_rsp_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_device_stru *mac_device = NULL;
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_asoc_user_req_ie_stru *asoc_user_req_info = NULL;

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_handle_connect_rsp_ap::mac_device null.}");
        return;
    }

    /* 抛关联一个新的sta完成事件到WAL */
    event_mem = frw_event_alloc_m(WLAN_MAC_ADDR_LEN);
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_handle_connect_rsp_ap::event_mem null.}");
        return;
    }

    /* 标记该user关联成功，并且上报内核 */
    hmac_user->en_report_kernel = OAL_TRUE;

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CTX,
                       HMAC_HOST_CTX_EVENT_SUB_TYPE_STA_CONNECT_AP,
                       WLAN_MAC_ADDR_LEN,
                       FRW_EVENT_PIPELINE_STAGE_0,
                       hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id,
                       hmac_vap->st_vap_base_info.uc_vap_id);

    asoc_user_req_info = (hmac_asoc_user_req_ie_stru *)(event->auc_event_data);

    /* 上报内核的关联sta发送的关联请求帧ie信息 */
    asoc_user_req_info->puc_assoc_req_ie_buff = hmac_user->puc_assoc_req_ie_buff;
    asoc_user_req_info->assoc_req_ie_len = hmac_user->assoc_req_ie_len;

    /* 关联的STA mac地址 */
    if (memcpy_s((uint8_t *)asoc_user_req_info->auc_user_mac_addr, sizeof(asoc_user_req_info->auc_user_mac_addr),
        hmac_user->st_user_base_info.auc_user_mac_addr,
        sizeof(hmac_user->st_user_base_info.auc_user_mac_addr)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_handle_connect_rsp_ap::memcpy failed!");
    }

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);
}

#ifdef _PRE_WLAN_FEATURE_SAE
OAL_STATIC void hmac_ap_set_user_auth_alg_num(uint16_t user_idx, uint16_t us_auth_alg_num)
{
    mac_user_stru *mac_user = NULL;

    mac_user = mac_res_get_mac_user(user_idx);
    if (mac_user == NULL) {
        return;
    }
    mac_user->us_auth_alg_num = us_auth_alg_num;

    oam_warning_log1(0, OAM_SF_AUTH, "{hmac_ap_set_user_auth_alg_num::\
                us_auth_alg_num = [%d]!}", mac_user->us_auth_alg_num);
}

/*
 * 功能描述  : 根据auth帧内容，过滤非法sae auth帧
 * 返回值: OAL_TRUE:过滤该auth帧；OAL_FALSE:不过滤该auth帧
 */
#define SAE_AUTH_COMMIT_LEN 104
#define SAE_AUTH_CONFIRM_LEN 40

static oal_bool_enum_uint8 hmac_ap_up_rx_filter_sae_auth_by_len(uint8_t *payload, uint16_t payload_len,
    uint16_t auth_alg, uint16_t auth_seq)
{
    uint16_t min_payload_len;
    if (auth_alg != WLAN_MIB_AUTH_ALG_SAE) {
        return OAL_TRUE;
    }
    if (auth_seq != WLAN_AUTH_SEQ_SAE_COMMIT && auth_seq != WLAN_AUTH_SEQ_SAE_CONFIRM) {
        return OAL_TRUE;
    }
    if (auth_seq == WLAN_AUTH_SEQ_SAE_COMMIT) {
        min_payload_len = SAE_AUTH_COMMIT_LEN;
    } else {
        min_payload_len = SAE_AUTH_CONFIRM_LEN;
    }
    if (payload_len < min_payload_len) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/*
 * 功能描述  : AP收到ft/sae auth帧上报hostapd的处理
 * 1.日    期  : 2017年2月21日
  *   修改内容  : 新生成函数
 */
static void hmac_ap_up_rx_auth_req_to_host(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_req, uint16_t us_auth_alg_num)
{
    uint8_t auc_addr2[ETHER_ADDR_LEN] = { 0 };
    uint8_t is_seq1;
    uint16_t auth_seq;
    uint16_t user_idx = 0xffff;
    uint8_t auth_resend = 0;
    uint32_t ret;
    hmac_user_stru *hmac_user = NULL;
    uint8_t *payload = NULL;
    mac_vap_stru *mac_vap = NULL;
    uint16_t payload_len;

    mac_vap = &hmac_vap->st_vap_base_info;
    payload = mac_get_rx_payload(auth_req, &payload_len);
    if (payload == NULL) {
        return;
    }
    /* ,判断对端mac地址是否为有效，不能为全0 */
    /* 获取STA的地址 */
    mac_get_address2(oal_netbuf_header(auth_req), auc_addr2, ETHER_ADDR_LEN);
    if (mac_addr_is_zero(auc_addr2)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_ap_rx_auth_req::user mac is all 0 and invaild!}");
        return;
    }

    /* 解析auth transaction number */
    auth_seq = mac_get_auth_seq_num(payload, payload_len);
    if (auth_seq > HMAC_AP_AUTH_SEQ3_WEP_COMPLETE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_AUTH, "{auth recieve invalid seq, auth seq [%d]}", auth_seq);
        return;
    }

    if (hmac_ap_up_rx_filter_sae_auth_by_len(payload, payload_len, us_auth_alg_num, auth_seq) == OAL_TRUE) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_AUTH,
            "{auth recieve invalid frame, auth alg %d, seq %d, status %d, payload_len %d}",
            us_auth_alg_num, auth_seq, mac_get_auth_status(payload, payload_len), payload_len);
        return;
    }
    /* 获取用户idx */
    is_seq1 = (auth_seq == WLAN_AUTH_TRASACTION_NUM_ONE);
    ret = hmac_encap_auth_rsp_get_user_idx(mac_vap, auc_addr2, sizeof(auc_addr2), is_seq1, &auth_resend, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_AUTH,
            "hmac_ap_up_rx_auth_req_to_host:get_user_idx fail, ret=%d", ret);
    }
    hmac_user = mac_res_get_hmac_user(user_idx);
    if (hmac_user == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_AUTH, "{hmac_ap_up_rx_auth_req_to_host::get user null!}");
        return;
    }
    if (hmac_ap_rx_filter_dup_auth_req(hmac_user, us_auth_alg_num, auth_seq, auth_resend) == OAL_TRUE) {
        return;
    }
    hmac_ap_set_user_auth_alg_num(user_idx, us_auth_alg_num);
    hmac_rx_mgmt_send_to_host(hmac_vap, auth_req);
}
#endif /* _PRE_WLAN_FEATURE_SAE */

OAL_STATIC uint32_t hmac_ap_rx_auth_sae_handle(hmac_vap_stru *hmac_vap,
                                               oal_netbuf_stru *pst_auth_req, uint16_t us_auth_alg_num)
{
#if defined(_PRE_WLAN_FEATURE_SAE)
    /* 如果是FT/SAE认证算法，上报hostapd */
    if (us_auth_alg_num == WLAN_MIB_AUTH_ALG_FT || us_auth_alg_num == WLAN_MIB_AUTH_ALG_SAE) {
        hmac_ap_up_rx_auth_req_to_host(hmac_vap, pst_auth_req, us_auth_alg_num);
        return OAL_SUCC;
    } else if (us_auth_alg_num == WLAN_MIB_AUTH_ALG_OPEN_SYS &&
               g_st_mac_device_custom_cfg.bit_forbit_open_auth) {
        oam_warning_log0(0, OAM_SF_CONN, "{hmac_ap_rx_auth_sae_handle::forbid open user to auth}");
        return OAL_SUCC;
    }
#endif
    return OAL_FAIL;
}

#ifdef _PRE_WLAN_CHBA_MGMT
uint32_t hmac_chba_ap_check_whitelist(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_req)
{
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    mac_rx_ctl_stru *rx_info = NULL;
    mac_ieee80211_frame_stru *ieee80211_hdr = NULL;

    if (!(hmac_chba_vap_start_check(hmac_vap))) {
        return OAL_SUCC;
    }
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(auth_req);
    rx_info = (mac_rx_ctl_stru *)(&(rx_ctrl->st_rx_info));
    ieee80211_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(rx_info);
    if (hmac_chba_whitelist_check(hmac_vap, ieee80211_hdr->auc_address2, WLAN_MAC_ADDR_LEN) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif

uint32_t hmac_ap_rx_reset_psm(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_rsp)
{
    mac_tx_ctl_stru *tx_ctl = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint8_t vap_id;

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;
    /* 获取cb字段 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(auth_rsp);
    /* 发送认证响应帧之前，将用户的节能状态复位 */
    /* hmac_encap_auth_rsp中user id的字段值非法，表明组帧失败, 直接回复失败的认证响应帧 */
    if (mac_get_cb_tx_user_idx(tx_ctl) == g_wlan_spec_cfg->invalid_user_id) {
        oam_warning_log1(vap_id, OAM_SF_AUTH, "{hmac_ap_rx_reset_psm::user[%d] auth fail.}",
                         mac_get_cb_tx_user_idx(tx_ctl));
        return OAL_SUCC;
    }
    hmac_user = mac_res_get_hmac_user(mac_get_cb_tx_user_idx(tx_ctl));
    if (hmac_user == NULL) {
        oam_error_log0(vap_id, OAM_SF_AUTH, "{hmac_ap_rx_reset_psm::hmac_user null.}");
        return OAL_FAIL;
    }

    hmac_mgmt_reset_psm(&hmac_vap->st_vap_base_info, mac_get_cb_tx_user_idx(tx_ctl));
    return OAL_SUCC;
}

uint32_t hmac_ap_auth_rsp_handle(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_req, oal_netbuf_stru *auth_rsp,
    uint8_t *chtxt, uint16_t *auth_rsp_len)
{
    uint8_t vap_id;

    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;
    oal_mem_netbuf_trace(auth_rsp, OAL_TRUE);

    memset_s(oal_netbuf_cb(auth_rsp), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    *auth_rsp_len = hmac_encap_auth_rsp(&hmac_vap->st_vap_base_info, auth_rsp, auth_req, chtxt, WLAN_CHTXT_SIZE);
    if (*auth_rsp_len == 0) {
        oam_warning_log0(vap_id, OAM_SF_AUTH, "{hmac_ap_auth_rsp_handle::auth_rsp_len is 0.}");
        return OAL_FAIL;
    }

    oal_netbuf_put(auth_rsp, *auth_rsp_len);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ap_rx_auth_req
 * 功能描述  : 处理认证请求帧
 * 1.日    期  : 2013年7月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_ap_rx_auth_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_auth_req)
{
    oal_netbuf_stru *auth_rsp = NULL;
    uint8_t *auth_req_payload = NULL;
    uint8_t auc_chtxt[WLAN_CHTXT_SIZE] = { 0 };
    uint16_t uc_auth_rsp_len;
    uint16_t auth_alg_num;
    uint16_t auth_req_payload_len;
    uint32_t pedding_data = 0;
    uint8_t uc_vap_id;

    if (oal_any_null_ptr2(hmac_vap, pst_auth_req)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_ap_rx_auth_req::hmac_vap or auth_req is null.}");
        return;
    }
    auth_req_payload = mac_get_rx_payload(pst_auth_req, &auth_req_payload_len);
    if (auth_req_payload == NULL) {
        return;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    if (hmac_chba_ap_check_whitelist(hmac_vap, pst_auth_req) != OAL_SUCC) {
        return;
    }
#endif
    uc_vap_id = hmac_vap->st_vap_base_info.uc_vap_id;
    auth_alg_num = mac_get_auth_alg(auth_req_payload, auth_req_payload_len);

    oam_warning_log1(uc_vap_id, OAM_SF_CONN, "{hmac_ap_rx_auth_req::rx_auth_alg_num=[%d]}", auth_alg_num);
    if (hmac_ap_rx_auth_sae_handle(hmac_vap, pst_auth_req, auth_alg_num) == OAL_SUCC) {
        return;
    }

    if (auth_alg_num == WLAN_WITP_AUTH_SHARED_KEY) {
        /* 获取challenge text */
        /* 硬件寄存器获取随即byte,用于WEP SHARED加密 */
        oal_get_random_bytes((int8_t *)&auc_chtxt, WLAN_CHTXT_SIZE);
    }

    /* AP接收到STA发来的认证请求帧组相应的认证响应帧 */
    auth_rsp = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (auth_rsp == NULL) {
        oam_error_log0(uc_vap_id, OAM_SF_AUTH, "{hmac_ap_rx_auth_req::auth_rsp null.}");
        return;
    }

    if (hmac_ap_auth_rsp_handle(hmac_vap, pst_auth_req, auth_rsp, auc_chtxt, &uc_auth_rsp_len) != OAL_SUCC) {
        oal_netbuf_free(auth_rsp);
        return;
    }
    if (hmac_ap_rx_reset_psm(hmac_vap, auth_rsp) != OAL_SUCC) {
        oal_netbuf_free(auth_rsp);
        return;
    }

    /* 收到auth req帧时，暂停扫描，提高入网成功率 */
    hmac_config_scan_abort(&hmac_vap->st_vap_base_info, sizeof(uint32_t), (uint8_t *)&pedding_data);

    /* 无论认证成功或者失败，都抛事件给dmac发送认证帧 */
    if (hmac_tx_mgmt_send_event(&hmac_vap->st_vap_base_info, auth_rsp, uc_auth_rsp_len) != OAL_SUCC) {
        oal_netbuf_free(auth_rsp);
        oam_warning_log0(uc_vap_id, OAM_SF_AUTH, "hmac_ap_rx_auth_req::send event fail");
    }
}

/*
 * 函 数 名  : hmac_ap_rx_deauth_req
 * 功能描述  : 处理接收去认证帧
 * 1.日    期  : 2013年7月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_ap_rx_deauth_req(hmac_vap_stru *hmac_vap,
                                          uint8_t *mac_hdr, oal_bool_enum_uint8 en_is_protected)
{
    uint8_t *puc_sa = NULL;
    hmac_user_stru *hmac_user = NULL;
    uint16_t us_err_code;
    uint32_t ret;

    if (oal_any_null_ptr2(hmac_vap, mac_hdr)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_ap_rx_deauth_req::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &puc_sa);

    us_err_code = *((uint16_t *)(mac_hdr + MAC_80211_FRAME_LEN));

    /* 增加接收到去认证帧时的维测信息 */
    oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CONN,
                     "{hmac_ap_rx_deauth_req:: DEAUTH rx, reason code = %d, sa[%02X:XX:XX:XX:%02X:%02X]}",
                     us_err_code, puc_sa[MAC_ADDR_0], puc_sa[MAC_ADDR_4], puc_sa[MAC_ADDR_5]);

    hmac_user = mac_vap_get_hmac_user_by_addr(&hmac_vap->st_vap_base_info, puc_sa, WLAN_MAC_ADDR_LEN);
    if (hmac_user == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{aput rx deauth frame, hmac_user null.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    /* 检查是否需要发送SA query request */
    if ((hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (hmac_pmf_check_err_code(&hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info,
            en_is_protected, mac_hdr) == OAL_SUCC)) {
        hmac_user->attack_mgmt.pmf_rx_deauth_cnt++;
        /* 在关联状态下收到未加密的ReasonCode 6/7需要开启SA Query流程 */
        ret = hmac_start_sa_query(&hmac_vap->st_vap_base_info, hmac_user,
                                  hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
        if (ret != OAL_SUCC) {
            return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
        }
        return OAL_SUCC;
    }

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    if (hmac_mgmt_pmf_frame_filter(&hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info,
        (mac_ieee80211_frame_stru *)mac_hdr, en_is_protected) != OAL_SUCC) {
        return OAL_FAIL;
    }
#endif
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_PASSIVE_DISCONNECT, us_err_code);
    /* 抛事件上报内核，已经去关联某个STA */
    hmac_handle_disconnect_rsp_ap(hmac_vap, hmac_user);

    hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_user_sort_op_rates
 * 功能描述  : 按一定顺序将user中速率重新排序
 * 1.日    期  : 2013年7月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_user_sort_op_rates(hmac_user_stru *hmac_user)
{
    uint8_t uc_loop;
    uint8_t uc_num_rates;
    uint8_t uc_min_rate;
    uint8_t uc_temp_rate; /* 临时速率，用于数据交换 */
    uint8_t uc_index;
    uint8_t uc_temp_index; /* 临时索引，用于数据交换 */

    uc_num_rates = hmac_user->st_op_rates.uc_rs_nrates;

    for (uc_loop = 0; uc_loop < uc_num_rates; uc_loop++) {
        /* 记录当前速率为最小速率 */
        uc_min_rate = (hmac_user->st_op_rates.auc_rs_rates[uc_loop] & 0x7F);
        uc_temp_index = uc_loop;

        /* 依次查找最小速率 */
        for (uc_index = uc_loop + 1; uc_index < uc_num_rates; uc_index++) {
            /* 记录的最小速率大于如果当前速率 */
            if (uc_min_rate > (hmac_user->st_op_rates.auc_rs_rates[uc_index] & 0x7F)) {
                /* 更新最小速率 */
                uc_min_rate = (hmac_user->st_op_rates.auc_rs_rates[uc_index] & 0x7F);
                uc_temp_index = uc_index;
            }
        }

        uc_temp_rate = hmac_user->st_op_rates.auc_rs_rates[uc_loop];
        hmac_user->st_op_rates.auc_rs_rates[uc_loop] = hmac_user->st_op_rates.auc_rs_rates[uc_temp_index];
        hmac_user->st_op_rates.auc_rs_rates[uc_temp_index] = uc_temp_rate;
    }

    /*******************************************************************
      重排11g模式的可操作速率，使11b速率都聚集在11a之前
      802.11a 速率:6、9、12、18、24、36、48、54Mbps
      802.11b 速率:1、2、5.5、11Mbps
      由于按由小到大排序后802.11b中的速率11Mbps在802.11a中，下标为5
      所以从第五位进行检验并排序。
    *******************************************************************/
    if (hmac_user->st_op_rates.uc_rs_nrates == MAC_DATARATES_PHY_80211G_NUM) { /* 11g_compatibility mode */
        if ((hmac_user->st_op_rates.auc_rs_rates[BYTE_OFFSET_5] & 0x7F) == 0x16) { /* 11Mbps */
            uc_temp_rate = hmac_user->st_op_rates.auc_rs_rates[BYTE_OFFSET_5];
            hmac_user->st_op_rates.auc_rs_rates[BYTE_OFFSET_5] =
                                                   hmac_user->st_op_rates.auc_rs_rates[BYTE_OFFSET_4];
            hmac_user->st_op_rates.auc_rs_rates[BYTE_OFFSET_4] =
                                                    hmac_user->st_op_rates.auc_rs_rates[BYTE_OFFSET_3];
            hmac_user->st_op_rates.auc_rs_rates[BYTE_OFFSET_3] = uc_temp_rate;
        }
    }
}

/*
 * 函 数 名  : hmac_ap_up_update_sta_cap_info
 * 功能描述  : AP侧设置STA的能力信息
 * 1.日    期  : 2013年7月3日
  *   修改内容  : 新生成函数
 */
oal_bool_enum_uint8 hmac_ap_up_update_sta_cap_info(hmac_vap_stru *hmac_vap,
    uint16_t us_cap_info, hmac_user_stru *hmac_user, mac_status_code_enum_uint16 *pstatus_code)
{
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t ret;
    mac_cap_info_stru *pst_cap_info = (mac_cap_info_stru *)(&us_cap_info);
    if (oal_any_null_ptr2(hmac_vap, hmac_user)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ap_up_update_sta_cap_info::hmac_vap or hmac_user is null.}");
        *pstatus_code = MAC_UNSPEC_FAIL;
        return OAL_FALSE;
    }
    pst_mac_vap = &(hmac_vap->st_vap_base_info);
    /* check bss capability info MAC,忽略MAC能力不匹配的STA */
    ret = hmac_check_bss_cap_info(us_cap_info, pst_mac_vap);
    if (ret != OAL_TRUE) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                       "{hmac_ap_up_update_sta_cap_info::hmac_check_bss_cap_info failed[%d].}", ret);
        *pstatus_code = MAC_UNSUP_CAP;
        return OAL_FALSE;
    }
    /* 如果以上各能力信息均满足关联要求，则继续处理其他能力信息 */
    mac_vap_check_bss_cap_info_phy_ap(us_cap_info, pst_mac_vap);
    if ((pst_cap_info->bit_privacy == 0) &&
        (hmac_user->st_user_base_info.st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_NO_ENCRYP)) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{hmac_ap_up_update_sta_cap_info::\
            The key encryption mode does not match, status_code=%d.}", *pstatus_code);
        *pstatus_code = MAC_UNSPEC_FAIL;
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

OAL_STATIC uint16_t hmac_check_tkip_user_ht_vht_he_capable(mac_user_stru *mac_user)
{
    uint16_t status = MAC_SUCCESSFUL_STATUSCODE;

    if (mac_user->st_ht_hdl.en_ht_capable == OAL_TRUE) {
        status = MAC_MISMATCH_HTCAP;
    }
    if (mac_user->st_vht_hdl.en_vht_capable == OAL_TRUE) {
        status = MAC_MISMATCH_VHTCAP;
    }
    if (mac_user->st_he_hdl.en_he_capable == OAL_TRUE) {
        status = MAC_MISMATCH_HECAP;
    }
    return status;
}

/*
 * 函 数 名  : hmac_check_wpa_cipher_ap
 * 功能描述  : 检查关联请求中的wpaie的加密套件是否符合本地要求
 * 1.日    期  : 2016年11月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint16_t hmac_check_wpa_cipher_ap(mac_vap_stru *mac_vap, mac_user_stru *mac_user, uint8_t *cipher_ie)
{
    mac_crypto_settings_stru crypto;
    uint16_t status = MAC_SUCCESSFUL_STATUSCODE;

    if (mac_ie_get_wpa_cipher(cipher_ie, &crypto) != OAL_SUCC) {
        return MAC_INVALID_INFO_ELMNT;
    }

    if (mac_mib_get_wpa_group_suite(mac_vap) != crypto.group_suite) {
        return MAC_INVALID_GRP_CIPHER;
    }

    if (mac_mib_wpa_pair_match_suites_s(mac_vap, crypto.aul_pair_suite, sizeof(crypto.aul_pair_suite)) == 0) {
        return MAC_INVALID_PW_CIPHER;
    }

    if (mac_mib_wpa_akm_match_suites_s(mac_vap, crypto.aul_akm_suite, sizeof(crypto.aul_akm_suite)) == 0) {
        return MAC_INVALID_AKMP_CIPHER;
    }

    /* 如果sta仅支持TKIP加密，且携带了非leagcy协议的IE，则拒绝关联 */
    if (crypto.wpa_pair_suite_count == 1 && crypto.aul_pair_suite[0] == MAC_WPA_CHIPER_TKIP) {
        status = hmac_check_tkip_user_ht_vht_he_capable(mac_user);
        mac_user->st_ht_hdl.en_ht_capable = OAL_FALSE;
        mac_user->st_vht_hdl.en_vht_capable = OAL_FALSE;
        mac_user->st_he_hdl.en_he_capable = OAL_FALSE;
    }
    return status;
}
/*
 * 函 数 名  : hmac_check_rsn_cipher_ap
 * 功能描述  : 检查关联请求中的rsnie的加密套件是否符合本地要求
 * 1.日    期  : 2016年11月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint16_t hmac_check_rsn_cipher_ap(mac_vap_stru *pst_mac_vap,
                                             mac_user_stru *mac_user, uint8_t *cipher_ie)
{
    mac_crypto_settings_stru crypto;
    uint16_t us_rsn_cap;
    uint16_t status = MAC_SUCCESSFUL_STATUSCODE;

    if (mac_ie_get_rsn_cipher(cipher_ie, &crypto) != OAL_SUCC) {
        return MAC_INVALID_INFO_ELMNT;
    }

    if (mac_mib_get_rsn_group_suite(pst_mac_vap) != crypto.group_suite) {
        return MAC_INVALID_GRP_CIPHER;
    }

    if (mac_mib_rsn_pair_match_suites_s(pst_mac_vap, crypto.aul_pair_suite, sizeof(crypto.aul_pair_suite)) == 0) {
        return MAC_INVALID_PW_CIPHER;
    }

    if (mac_mib_rsn_akm_match_suites_s(pst_mac_vap, crypto.aul_akm_suite, sizeof(crypto.aul_akm_suite)) == 0) {
        return MAC_INVALID_AKMP_CIPHER;
    }

    us_rsn_cap = mac_get_rsn_capability(cipher_ie);
    /* 预认证能力检查 */
    if ((us_rsn_cap & BIT0) != 0) {
        return MAC_INVALID_RSN_INFO_CAP;
    }

    /* sae auth 成功但是user 不支持pmf,不允许入网,待认证结束删除维测 */
    oam_warning_log2(0, OAM_SF_ASSOC, "{hmac_check_rsn_cipher_ap::us_auth_alg_num=[%d],us_rsn_cap=[%X].}",
                     mac_user->us_auth_alg_num, us_rsn_cap);
    if (mac_user->us_auth_alg_num == WLAN_MIB_AUTH_ALG_SAE && (!(us_rsn_cap & BIT7))) {
        oam_warning_log0(0, OAM_SF_ASSOC, "{hmac_check_rsn_cipher_ap::WLAN_MIB_AUTH_ALG_SAE MFP disable.}");
        return MAC_R0KH_UNREACHABLE;
    }

    /* 本地强制，对端没有MFP能力 */
    if ((mac_mib_get_dot11RSNAMFPR(pst_mac_vap) == OAL_TRUE) && (!(us_rsn_cap & BIT7))) {
        return MAC_MFP_VIOLATION;
    }
    /* 对端强制，本地没有MFP能力 */
    if ((mac_mib_get_dot11RSNAMFPC(pst_mac_vap) == OAL_FALSE) && (us_rsn_cap & BIT6)) {
        return MAC_MFP_VIOLATION;
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    if ((mac_mib_get_dot11RSNAMFPC(pst_mac_vap) == OAL_TRUE) && (us_rsn_cap & BIT7)) {
        mac_user_set_pmf_active(mac_user, OAL_TRUE);
    }
#endif

    /* 如果sta仅支持TKIP加密，且携带了非leagcy协议的IE，则拒绝关联 */
    if (crypto.wpa_pair_suite_count == 1 && crypto.aul_pair_suite[0] == MAC_RSN_CHIPER_TKIP) {
        status = hmac_check_tkip_user_ht_vht_he_capable(mac_user);
        mac_user->st_ht_hdl.en_ht_capable = OAL_FALSE;
        mac_user->st_vht_hdl.en_vht_capable = OAL_FALSE;
        mac_user->st_he_hdl.en_he_capable = OAL_FALSE;
    }

    return status;
}

/*
 * 函 数 名  : hmac_check_rsn_ap
 * 功能描述  : AP检查(重)关联请求中的rsn/wpa 信息元素
 * 1.日    期  : 2016年11月08日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint16_t hmac_check_rsn_ap(mac_vap_stru *pst_mac_vap, mac_user_stru *mac_user,
    uint8_t *payload, uint32_t msg_len)
{
    uint8_t *puc_rsn_ie = NULL;
    uint8_t *puc_wpa_ie = NULL;

    /* 若本地没有rsn能力,忽略检查ie。以增加兼容性 */
    if (mac_mib_get_rsnaactivated(pst_mac_vap) == OAL_FALSE) {
        return MAC_SUCCESSFUL_STATUSCODE;
    }

    /* 获取RSN和WPA IE信息 */
    puc_rsn_ie = mac_find_ie(MAC_EID_RSN, payload, (int32_t)msg_len);
    puc_wpa_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA, payload, (int32_t)msg_len);
    if ((puc_rsn_ie == NULL) && (puc_wpa_ie == NULL)) {
        /* 非wps关联过程仍需要检查ie */
        if (mac_mib_get_WPSActive(pst_mac_vap) == OAL_TRUE) {
            return MAC_SUCCESSFUL_STATUSCODE;
        } else {
            return MAC_INVALID_INFO_ELMNT;
        }
    }

    if ((pst_mac_vap->st_cap_flag.bit_wpa2 == OAL_TRUE) && (puc_rsn_ie != NULL)) {
        return hmac_check_rsn_cipher_ap(pst_mac_vap, mac_user, puc_rsn_ie);
    }

    if ((pst_mac_vap->st_cap_flag.bit_wpa == OAL_TRUE) && (puc_wpa_ie != NULL)) {
        return hmac_check_wpa_cipher_ap(pst_mac_vap, mac_user, puc_wpa_ie);
    }

    return MAC_CIPHER_REJ;
}

/*
 * 函 数 名  : hmac_is_erp_sta
 * 功能描述  : 查看sta是否属于erp站点
 * 1.日    期  : 2014年3月26日
  *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_is_erp_sta(hmac_user_stru *hmac_user)
{
    uint32_t loop;
    oal_bool_enum_uint8 en_is_erp_sta;

    /* 确认是否是erp 站点 */
    if (hmac_user->st_op_rates.uc_rs_nrates <= MAC_NUM_DR_802_11B) {
        en_is_erp_sta = OAL_FALSE;
        for (loop = 0; loop < hmac_user->st_op_rates.uc_rs_nrates; loop++) {
            /* 如果支持速率不在11b的1M, 2M, 5.5M, 11M范围内，则说明站点为支持ERP的站点 */
            if (((hmac_user->st_op_rates.auc_rs_rates[loop] & 0x7F) != 0x2) &&
                ((hmac_user->st_op_rates.auc_rs_rates[loop] & 0x7F) != 0x4) &&
                ((hmac_user->st_op_rates.auc_rs_rates[loop] & 0x7F) != 0xb) &&
                ((hmac_user->st_op_rates.auc_rs_rates[loop] & 0x7F) != 0x16)) {
                en_is_erp_sta = OAL_TRUE;
                break;
            }
        }
    } else {
        en_is_erp_sta = OAL_TRUE;
    }

    return en_is_erp_sta;
}

static void hmac_ap_up_update_short_slot_capability(uint16_t cap_info, hmac_user_stru *hmac_user,
                                                    mac_user_stru *mac_user, mac_protection_stru *protection)
{    /* 如果STA不支持short slot */
    if ((cap_info & MAC_CAP_SHORT_SLOT) != MAC_CAP_SHORT_SLOT) {
        /* 如果STA之前没有关联， 或者之前以支持short slot站点身份关联，需要update处理 */
        if ((mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC)
            || (hmac_user->st_hmac_cap_info.bit_short_slot_time == OAL_TRUE)) {
            protection->uc_sta_no_short_slot_num++;
        }

        hmac_user->st_hmac_cap_info.bit_short_slot_time = OAL_FALSE;
    } else { /* 如果STA支持short slot */
        /* 如果STA以不支持short slot站点身份关联，需要update处理 */
        if ((mac_user->en_user_asoc_state == MAC_USER_STATE_ASSOC)
            && (hmac_user->st_hmac_cap_info.bit_short_slot_time == OAL_FALSE)
            && (protection->uc_sta_no_short_slot_num != 0)) {
            protection->uc_sta_no_short_slot_num--;
        }

        hmac_user->st_hmac_cap_info.bit_short_slot_time = OAL_TRUE;
    }
}

/*
 * 函 数 名  : hmac_ap_up_update_legacy_capability
 * 功能描述  : AP处理assoc req 中的capability info
 * 1.日    期  : 2013年7月9日
  *   修改内容  : 新生成函数
 */
void hmac_ap_up_update_legacy_capability(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, uint16_t us_cap_info)
{
    mac_protection_stru *pst_protection = &(hmac_vap->st_vap_base_info.st_protection);
    mac_user_stru *mac_user = &(hmac_user->st_user_base_info);
    oal_bool_enum_uint8 en_is_erp_sta;

    hmac_ap_up_update_short_slot_capability(us_cap_info, hmac_user, mac_user, pst_protection);

    hmac_user->st_user_stats_flag.bit_no_short_slot_stats_flag = OAL_TRUE;

    /* 如果STA不支持short preamble */
    if ((us_cap_info & MAC_CAP_SHORT_PREAMBLE) != MAC_CAP_SHORT_PREAMBLE) {
        /* 如果STA之前没有关联， 或者之前以支持short preamble站点身份关联，需要update处理 */
        if ((mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC)
            || (hmac_user->st_hmac_cap_info.bit_short_preamble == OAL_TRUE)) {
            pst_protection->uc_sta_no_short_preamble_num++;
        }

        hmac_user->st_hmac_cap_info.bit_short_preamble = OAL_FALSE;
    } else { /* 如果STA支持short preamble */
        /* 如果STA之前以不支持short preamble站点身份关联，需要update处理 */
        if ((mac_user->en_user_asoc_state == MAC_USER_STATE_ASSOC)
            && (hmac_user->st_hmac_cap_info.bit_short_preamble == OAL_FALSE)
            && (pst_protection->uc_sta_no_short_preamble_num != 0)) {
            pst_protection->uc_sta_no_short_preamble_num--;
        }

        hmac_user->st_hmac_cap_info.bit_short_preamble = OAL_TRUE;
    }

    hmac_user->st_user_stats_flag.bit_no_short_preamble_stats_flag = OAL_TRUE;

    /* 确定user是否是erp站点 */
    en_is_erp_sta = hmac_is_erp_sta(hmac_user);
    /* 如果STA不支持ERP */
    if (en_is_erp_sta == OAL_FALSE) {
        /* 如果STA之前没有关联， 或者之前以支持ERP站点身份关联，需要update处理 */
        if ((mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC)
            || (hmac_user->st_hmac_cap_info.bit_erp == OAL_TRUE)) {
            pst_protection->uc_sta_non_erp_num++;
        }

        hmac_user->st_hmac_cap_info.bit_erp = OAL_FALSE;
    } else { /* 如果STA支持ERP */
        /* 如果STA之前以不支持ERP身份站点关联，需要update处理 */
        if ((mac_user->en_user_asoc_state == MAC_USER_STATE_ASSOC)
            && (hmac_user->st_hmac_cap_info.bit_erp == OAL_FALSE)
            && (pst_protection->uc_sta_non_erp_num != 0)) {
            pst_protection->uc_sta_non_erp_num--;
        }

        hmac_user->st_hmac_cap_info.bit_erp = OAL_TRUE;
    }

    hmac_user->st_user_stats_flag.bit_no_erp_stats_flag = OAL_TRUE;

    if ((us_cap_info & MAC_CAP_SPECTRUM_MGMT) != MAC_CAP_SPECTRUM_MGMT) {
        mac_user_set_spectrum_mgmt(&hmac_user->st_user_base_info, OAL_FALSE);
    } else {
        mac_user_set_spectrum_mgmt(&hmac_user->st_user_base_info, OAL_TRUE);
    }
}

/*
 * 函 数 名  : hmac_ap_up_update_asoc_entry_prot
 * 功能描述  : 更新ASOC关联实体中的UAPSD信息
 * 1.日    期  : 2013年7月9日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_ap_up_update_asoc_entry_prot(hmac_user_stru *hmac_user, uint8_t *payload, uint32_t msg_len)
{
    /* WMM */
    hmac_uapsd_update_user_para(hmac_user, payload, msg_len);

    return;
}


/*
 * 函 数 名  : hmac_user_is_connecting
 * 功能描述  : 判断是否有user处于关联状态
 * 1.日    期  : 2018年5月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
oal_bool_enum hmac_user_is_connecting(mac_vap_stru *pst_mac_vap)
{
    oal_dlist_head_stru *pst_entry = NULL;
    oal_dlist_head_stru *pst_dlist_tmp = NULL;
    mac_user_stru *pst_user_tmp = NULL;

    if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OAL_FALSE;
    }

    oal_dlist_search_for_each_safe(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        if (pst_user_tmp == NULL) {
            oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_user_is_connecting::pst_user_tmp null.}");
            continue;
        }
        if (pst_user_tmp->en_user_asoc_state != MAC_USER_STATE_ASSOC) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{hmac_user_is_connecting::user[%d] is connecting, state[%d].}", pst_user_tmp->us_assoc_id,
                pst_user_tmp->en_user_asoc_state);
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/*
 * 函 数 名  : hmac_ap_check_owe_capa
 * 功能描述  : 检查assoc req帧的OWE携带情况
 * 1.日    期  : 2019年1月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_ap_check_owe_capa(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
                            uint8_t *payload, uint32_t payload_len,
                            mac_status_code_enum_uint16 *status_code)
{
    uint32_t ret;
    uint8_t *puc_rsn_ie = NULL;
    uint8_t *puc_owe_dh = NULL;
    mac_crypto_settings_stru st_crypto = { 0 };
    uint16_t us_suite_idx;
    uint16_t owe_group;

    hmac_user->st_hmac_cap_info.bit_owe = OAL_FALSE;

    /* 优先查找 RSN 信息元素 */
    puc_rsn_ie = mac_find_ie(MAC_EID_RSN, (uint8_t *)payload, (int32_t)payload_len);
    if (puc_rsn_ie == NULL) {
        return;
    }

    /* 1. check OWE AKM */
    ret = mac_ie_get_rsn_cipher(puc_rsn_ie, &st_crypto);
    if (ret != OAL_SUCC) {
        return;
    }

    /* AKM Suite */
    for (us_suite_idx = 0; us_suite_idx < WLAN_AUTHENTICATION_SUITES; us_suite_idx++) {
        if (st_crypto.aul_akm_suite[us_suite_idx] == MAC_RSN_AKM_OWE) {
            hmac_user->st_hmac_cap_info.bit_owe = OAL_TRUE;
            break;
        }
    }
    if (hmac_user->st_hmac_cap_info.bit_owe == OAL_FALSE) {
        return;
    }

    oam_warning_log3(0, OAM_SF_WPA,
                     "hmac_ap_check_owe_capa::get_rsn_cipher owe:%d, akm_suite[0]:0x%x akm_suite[1]:0x%x",
                     hmac_user->st_hmac_cap_info.bit_owe,
                     st_crypto.aul_akm_suite[0], st_crypto.aul_akm_suite[1]);

    /* 2. check OWE Diffie-Hellman Parameter Element */
    /*************************************************************************/
    /*             OWE Diffie-Hellman Parameter element                      */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Element ID |  element-specific data            */
    /* |           |        | Extension  |  group        | public key        */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    1       |      2        |                   */
    /* --------------------------------------------------------------------- */
    /* |    255    |variable|    32      |   19/20/21    |                   */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    puc_owe_dh = mac_find_ie_ext_ie(MAC_EID_EXTENSION, MAC_EID_EXT_OWE_DH_PARAM, payload, (int32_t)payload_len);
    /* owe dh ie len不足3字节不处理 */
    if ((puc_owe_dh == NULL) || (puc_owe_dh[1] < MAC_EXT_OWE_DH_PARAM_LEN)) {
        hmac_user->st_hmac_cap_info.bit_owe = OAL_FALSE;
        return;
    }

    puc_owe_dh += MAC_IE_HDR_LEN; /* skip EID and length */
    owe_group = *(puc_owe_dh + 1);
    owe_group += (*(puc_owe_dh + 2)) << 8; /*  字节2左移8位 */
    if ((owe_group == 0) || ((hmac_vap->owe_group[0] != owe_group) && (hmac_vap->owe_group[1] != owe_group) &&
        (hmac_vap->owe_group[2] != owe_group))) { /* group 1 2 */
        *status_code = MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED; /* OWE group not matched */
    }

    oam_warning_log4(0, OAM_SF_WPA,
                     "hmac_ap_check_owe_capa::vap owe_group_cap=%d, associated STA bit_owe=%d owe_group:%d, status:%d",
                     hmac_vap->owe_group_cap, hmac_user->st_hmac_cap_info.bit_owe, owe_group, *status_code);

    return;
}

OAL_STATIC OAL_INLINE void hmac_ap_2g_11ac_user_asoc(hmac_vap_stru *hmac_vap,
                                                     uint8_t *payload, uint32_t msg_len,
                                                     hmac_user_stru *hmac_user)
{
    uint8_t *puc_vendor_vht_ie;
    uint32_t vendor_vht_ie_offset = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;
    uint8_t *puc_ie_tmp = NULL;

    puc_vendor_vht_ie = mac_find_vendor_ie(MAC_WLAN_OUI_BROADCOM_EPIGRAM,
                                           MAC_WLAN_OUI_VENDOR_VHT_TYPE,
                                           payload, msg_len);
    if ((puc_vendor_vht_ie != NULL) && (puc_vendor_vht_ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::find broadcom/epigram vendor ie, enable hidden bit_11ac2g}");
        /* 进入此函数代表user支持2G 11ac */
        puc_ie_tmp = mac_find_ie(MAC_EID_VHT_CAP,
                                 puc_vendor_vht_ie + vendor_vht_ie_offset,
                                 (int32_t)(puc_vendor_vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
        if (puc_ie_tmp != NULL) {
            hmac_user->en_user_vendor_vht_capable = OAL_TRUE;
            hmac_proc_vht_cap_ie(&hmac_vap->st_vap_base_info, hmac_user, puc_ie_tmp);
        } else {
            hmac_user->en_user_vendor_novht_capable = OAL_TRUE;  // 表示支持5G 20M mcs9
        }
    }
}

OAL_STATIC oal_bool_enum hmac_ap_frame_len_check(uint8_t *mac_hdr, uint32_t msg_len)
{
    uint32_t len_min;

    len_min = ((WLAN_FC0_SUBTYPE_REASSOC_REQ | WLAN_FC0_TYPE_MGT) == mac_get_frame_type_and_subtype(mac_hdr)) ?
                 (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN + WLAN_MAC_ADDR_LEN) : (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN);

    return ((msg_len > len_min) ? OAL_TRUE : OAL_FALSE);
}

#ifdef _PRE_WLAN_FEATURE_11AC2G
OAL_STATIC void hmac_ap_update_avail_protocol_mode(mac_vap_stru *pst_mac_vap,
                                                   mac_user_stru *mac_user, uint8_t *puc_avail_mode)
{
    if ((pst_mac_vap->en_protocol == WLAN_HT_MODE) && (mac_user->en_protocol_mode == WLAN_VHT_MODE) &&
        (pst_mac_vap->st_cap_flag.bit_11ac2g == OAL_TRUE) && (pst_mac_vap->st_channel.en_band == WLAN_BAND_2G)) {
        *puc_avail_mode = WLAN_VHT_MODE;
    }
}
#endif

OAL_STATIC uint32_t hmac_ap_up_update_sta_user_cmp_ssid(hmac_vap_stru *hmac_vap,
    uint8_t *puc_ie, uint32_t remain_len, mac_status_code_enum_uint16 *pstatus_code)
{
    mac_vap_stru *mac_vap = &(hmac_vap->st_vap_base_info);

#ifdef _PRE_WLAN_CHBA_MGMT
    /* chba极速连接时, ssid校验由上层进行，极速连接响应方不在驱动校验 */
    if (hmac_chba_is_fast_connect_response(hmac_vap) == OAL_TRUE) {
        return OAL_SUCC;
    }
#endif
    if ((puc_ie != NULL) && (remain_len > 2)) { // 消息buffer剩余长度超过2
        if ((puc_ie[1] != (uint8_t)OAL_STRLEN((int8_t *)mac_mib_get_DesiredSSID(mac_vap))) ||
            (oal_memcmp(&puc_ie[BYTE_OFFSET_2], mac_mib_get_DesiredSSID(mac_vap), puc_ie[1]) != 0)) {
            *pstatus_code = MAC_UNSPEC_FAIL;
            oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_ap_up_update_sta_user::ssid mismatch.}");
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

uint32_t hmac_mgmt_update_vht_cap(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap,
    uint8_t *payload, uint32_t msg_len)
{
    uint8_t *ie_tmp;
    ie_tmp = mac_find_ie(MAC_EID_VHT_CAP, payload, (int32_t)msg_len);
    if (ie_tmp != NULL) {
        hmac_proc_vht_cap_ie(mac_vap, hmac_user, ie_tmp);
#ifdef _PRE_WLAN_FEATURE_1024QAM
        ie_tmp = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_HISI_1024QAM_IE, payload, msg_len);
        if (ie_tmp != NULL) {
            hmac_user->st_user_base_info.st_cap_info.bit_1024qam_cap = OAL_TRUE;
            hmac_user->st_user_base_info.st_cap_info.bit_20mmcs11_compatible_mp13 = OAL_TRUE;
        }
#endif
    } else if (mac_vap->en_protocol == WLAN_VHT_ONLY_MODE) {
        /* 不允许不支持11ac STA关联11aconly 模式的AP */
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user:AP 11ac only, but STA not support 11ac}");
        return OAL_FAIL;
    }
    /* *  APUT根据BRCM VENDOR OUI 适配2G 11AC */
    if (hmac_user->st_user_base_info.st_vht_hdl.en_vht_capable == OAL_FALSE) {
        hmac_ap_2g_11ac_user_asoc(hmac_vap, payload, msg_len, hmac_user);
    }

    return OAL_SUCC;
}

void hmac_mgmt_update_bandwidth_cap(mac_user_stru *mac_user, hmac_vap_stru *hmac_vap, mac_vap_stru *mac_vap)
{
    wlan_bw_cap_enum_uint8 bandwidth_cap = WLAN_BW_CAP_BUTT;
    wlan_bw_cap_enum_uint8 bwcap_ap; /* ap自身带宽能力 */
    wlan_bw_cap_enum_uint8 bwcap_vap;
    uint32_t ret;
    /* 获取用户的带宽能力 */
    mac_user_get_sta_cap_bandwidth(mac_user, &bandwidth_cap);
    /* 获取vap带宽能力与用户带宽能力的交集 */
    mac_vap_get_bandwidth_cap(&hmac_vap->st_vap_base_info, &bwcap_ap);
    bwcap_vap = oal_min(bwcap_ap, bandwidth_cap);
    mac_user_set_bandwidth_info(mac_user, bwcap_vap, bwcap_vap);
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_ap_up_update_sta_user::mac_vap->bandwidth:%d, \
        mac_user->bandwidth:%d, cur_bandwidth:%d.}", bwcap_ap, bandwidth_cap, mac_user->en_cur_bandwidth);
    /* 获取用户160M带宽下的空间流 */
    hmac_user_set_num_spatial_stream_160M(mac_user);
    /* 根据用户支持带宽能力，协商出当前带宽，dmac offload架构下，同步带宽信息到device */
    ret = hmac_config_user_info_syn(mac_vap, mac_user);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_user->uc_vap_id, 0, "{hmac_ap_up_update_sta_user::usr_info_syn failed[%d].}", ret);
    }
}

uint32_t hmac_mgmt_updata_protocol_cap(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, mac_user_stru *mac_user)
{
    uint8_t avail_mode;
    /* 获取用户的协议模式 */
    hmac_set_user_protocol_mode(mac_vap, hmac_user);
    avail_mode = g_auc_avail_protocol_mode[mac_vap->en_protocol][mac_user->en_protocol_mode];
    if (avail_mode == WLAN_PROTOCOL_BUTT) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::user not allowed:no valid protocol: \
                         vap mode=%d, user mode=%d, user avail mode=%d.}",
                         mac_vap->en_protocol, mac_user->en_protocol_mode, mac_user->en_avail_protocol_mode);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_11AC2G
    hmac_ap_update_avail_protocol_mode(mac_vap, mac_user, &avail_mode);
#endif
    /* 获取用户与VAP协议模式交集 */
    mac_user_set_avail_protocol_mode(mac_user, avail_mode);
    mac_user_set_cur_protocol_mode(mac_user, mac_user->en_avail_protocol_mode);

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_ap_up_update_sta_user::vap protocol:%d,mac user protocol mode:%d,avail protocol mode:%d}",
                     mac_vap->en_protocol, mac_user->en_protocol_mode, mac_user->en_avail_protocol_mode);
    return OAL_SUCC;
}


uint32_t hmac_mgmt_update_spatial_stream_cap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint8_t *mac_hdr, uint8_t *payload, uint32_t msg_len)
{
    uint32_t ret;
    uint32_t result = OAL_SUCC;
    mac_vap_stru *mac_vap;
    mac_user_stru *mac_user;

    mac_user = &(hmac_user->st_user_base_info);
    mac_vap = &(hmac_vap->st_vap_base_info);
    /* 获取用户与VAP空间流交集 */
    if (hmac_user_set_avail_num_space_stream(mac_user, mac_vap->en_vap_rx_nss) != OAL_SUCC) {
        result = OAL_FAIL;
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_ap_up_update_sta_user::set nss fail}");
    }
    /* 根据asoc req中的smps能力位更新user信息 */
#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 根据smps更新空间流能力 */
    if (!is_vap_single_nss(mac_vap) && !is_user_single_nss(mac_user)) {
        hmac_smps_update_user_status(mac_vap, mac_user);
    }
#endif
    /*  */
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 处理Operating Mode Notification 信息元素 */
    ret = hmac_check_opmode_notify(hmac_vap, mac_hdr, payload, msg_len, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::hmac_check_opmode_notify failed[%d].}", ret);
    }
#endif
    /* 同步空间流信息 */
    ret = hmac_config_user_num_spatial_stream_cap_syn(mac_vap, mac_user);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_ap_up_update_sta_user::hmac_config_user_num_spatial_stream_cap_syn failed[%d].}", ret);
    }
    return result;
}
#ifdef _PRE_WLAN_FEATURE_11AX
void hmac_ap_up_update_he_cap(hmac_user_stru *hmac_user, mac_vap_stru *mac_vap,
    uint8_t *ie_tmp, uint8_t *payload, uint32_t msg_len)
{
    ie_tmp = mac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_CAP, payload, (int32_t)msg_len);
    hmac_proc_he_cap_ie(mac_vap, hmac_user, ie_tmp);

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    /* 适配HE 6 GHz cap IE解析 */
    if (mac_vap->st_channel.ext6g_band == OAL_TRUE) {
        ie_tmp = mac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_6GHZ_BAND_CAP, payload, (int32_t)msg_len);
        if (ie_tmp != NULL) {
            hmac_proc_he_6ghz_band_cap_ie(mac_vap, hmac_user, ie_tmp);
        }
    }
#endif
}
#endif
OAL_STATIC uint16_t hmac_ap_up_update_protocol_cap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
                                                   uint16_t cap_info, uint8_t *mac_hdr, uint8_t *payload,
                                                   uint32_t msg_len)
{
    uint16_t ret;
    uint8_t *ie_tmp = NULL;
    mac_vap_stru *mac_vap = &(hmac_vap->st_vap_base_info);
    mac_user_stru *mac_user = &(hmac_user->st_user_base_info);
    /* 更新对应STA的legacy协议能力 */
    hmac_ap_up_update_legacy_capability(hmac_vap, hmac_user, cap_info);
    /* 检查HT capability以及Extend capability是否匹配，并进行处理  */
    ret = hmac_vap_check_ht_capabilities_ap(hmac_vap, payload, msg_len, hmac_user);
    if (ret != MAC_SUCCESSFUL_STATUSCODE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::Reject STA because of ht_capability[%d].}", ret);
        return ret;
    }
    /* 更新AP中保护相关mib量 */
    ret = hmac_user_protection_sync_data(mac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_update_sta_user::protection update mib failed, ret=%d.}", ret);
    }

    /* 更新对应STA的协议能力 */
    hmac_ap_up_update_asoc_entry_prot(hmac_user, payload, msg_len);
    /* 更新QoS能力 */
    hmac_mgmt_update_assoc_user_qos_table(payload, (uint16_t)msg_len, hmac_user);

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        hmac_hiex_rx_assoc_req(hmac_vap, hmac_user, payload, msg_len);
    }
#endif

    /* 更新11ac VHT capabilities ie */
    ret = hmac_mgmt_update_vht_cap(mac_vap, hmac_user, hmac_vap, payload, msg_len);
    if (ret != OAL_SUCC) {
        return MAC_MISMATCH_VHTCAP;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 更新11ax HE capabilities ie */
        hmac_ap_up_update_he_cap(hmac_user, mac_vap, ie_tmp, payload, msg_len);
    }
#endif

    /* 检查接收到的ASOC REQ消息中的SECURITY参数.如出错,则返回对应的错误码 */
    mac_user_init_key(&hmac_user->st_user_base_info);
    ret = hmac_check_rsn_ap(mac_vap, &hmac_user->st_user_base_info, payload, msg_len);
    if (ret != MAC_SUCCESSFUL_STATUSCODE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_check_rsn_ap fail[%d].}", ret);
        return ret;
    }

    hmac_user_cap_update_by_hisi_cap_ie(hmac_user, payload, msg_len);
    /* 处理协议能力 */
    if (hmac_mgmt_updata_protocol_cap(mac_vap, hmac_user, mac_user) != OAL_SUCC) {
        return MAC_UNSUP_CAP;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_get_ie_offset(uint8_t *mac_hdr)
{
    uint32_t ie_offset;

    ie_offset = (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN);
    if ((WLAN_FC0_SUBTYPE_REASSOC_REQ | WLAN_FC0_TYPE_MGT) == mac_get_frame_type_and_subtype(mac_hdr)) {
        ie_offset += WLAN_MAC_ADDR_LEN;
    }
    return ie_offset;
}

/* 功能描述: 将assoc req中信道列表信息保存到用户 */
void hmac_mgmt_update_user_supported_channel(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint8_t *payload, uint32_t msg_len)
{
    uint8_t *ie = NULL;
    struct supported_channel *sta_supported_channel = NULL;
    uint32_t loop;

    memset_s(&(hmac_user->supported_channel_param), sizeof(supproted_channel_stru),
        0, sizeof(supproted_channel_stru));
    ie = mac_find_ie(MAC_EID_SUPPCHAN, payload, msg_len);
    if (ie == NULL) {
        return;
    }
    sta_supported_channel = (struct supported_channel *)(ie + MAC_IE_HDR_LEN);
    hmac_user->supported_channel_param.num = ie[1] / sizeof(struct supported_channel);

    if (hmac_user->supported_channel_param.num > SUPPORTED_CHANNEL_NUM_MAX_CNT) {
        oam_error_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
            "hmac_mgmt_update_user_supported_channel::set sta supported channel %d to %d",
            hmac_user->supported_channel_param.num, SUPPORTED_CHANNEL_NUM_MAX_CNT);
        hmac_user->supported_channel_param.num = SUPPORTED_CHANNEL_NUM_MAX_CNT;
    }

    oam_info_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
        "hmac_mgmt_update_user_supported_channel::sta supported channel_num %d",
        hmac_user->supported_channel_param.num);
    for (loop = 0; loop < hmac_user->supported_channel_param.num; loop++)  {
        hmac_user->supported_channel_param.supported_channels[loop] = sta_supported_channel[loop];
        oam_info_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
            "hmac_mgmt_update_user_supported_channel::first channel %d, num_of_channel %d",
            hmac_user->supported_channel_param.supported_channels[loop].fist_channel_number,
            hmac_user->supported_channel_param.supported_channels[loop].number_of_channels);
    }
}

/*
 * 函 数 名  : hmac_ap_up_update_sta_user
 * 功能描述  : AP侧更新STA信息
 * 1.日    期  : 2013年7月3日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_ap_up_update_sta_user(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr, uint8_t *payload,
                                               uint32_t msg_len, hmac_user_stru *hmac_user,
                                               mac_status_code_enum_uint16 *pstatus_code)
{
    uint16_t us_cap_info;
    uint8_t *puc_ie_tmp = NULL;
    uint32_t ret;
    mac_vap_stru *mac_vap = NULL;
    mac_user_stru *mac_user = &(hmac_user->st_user_base_info);
    uint32_t remain_len = 0;

    *pstatus_code = MAC_SUCCESSFUL_STATUSCODE;
    mac_vap = &(hmac_vap->st_vap_base_info);
    if (hmac_ap_frame_len_check(mac_hdr, msg_len) != OAL_TRUE) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_ap_up_update_sta_user::len[%d] err}", msg_len);
        return OAL_FAIL;
    }
    /***************************************************************************
        检查AP是否支持当前正在关联的STA的所有能力
        |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|Reserved|
    ***************************************************************************/
    us_cap_info = oal_make_word16(payload[0], payload[1]);
    if (hmac_ap_up_update_sta_cap_info(hmac_vap, us_cap_info, hmac_user, pstatus_code) != OAL_TRUE) {
        return OAL_FAIL;
    }

    payload += hmac_get_ie_offset(mac_hdr);
    msg_len -= hmac_get_ie_offset(mac_hdr);

    /* 判断SSID,长度或内容不一致时,认为是SSID不一致，考虑兼容性找不到ie时不处理 */
    puc_ie_tmp = mac_find_ie_sec(MAC_EID_SSID, payload, (int32_t)msg_len, &remain_len);
    if (hmac_ap_up_update_sta_user_cmp_ssid(hmac_vap, puc_ie_tmp, remain_len, pstatus_code) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 根据wmm ie是否存在获取sta的wmm开关 */
    puc_ie_tmp = mac_find_ie(MAC_EID_WMM, payload, (int32_t)msg_len);
    hmac_user->en_wmm_switch = (puc_ie_tmp != NULL) ? OAL_FALSE : OAL_TRUE;
    /* 当前用户已关联 */
    ret = hmac_ie_proc_assoc_user_legacy_rate(payload, msg_len, hmac_user);
    if (ret != OAL_SUCC) {
        *pstatus_code = MAC_UNSUP_RATE;
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC, "{hmac_ap_up_update_sta_user::rates mismatch ret %d.}", ret);
        return ret;
    }
    /* 按一定顺序重新排列速率 */
    hmac_user_sort_op_rates(hmac_user);
    ret = hmac_ap_up_update_protocol_cap(hmac_vap, hmac_user, us_cap_info, mac_hdr, payload, msg_len);
    if (ret != OAL_SUCC) {
        *pstatus_code = ret;
        return ret;
    }
    /* 获取用户和VAP 可支持的11a/b/g 速率交集 */
    hmac_vap_set_user_avail_rates(mac_vap, hmac_user);
    if (hmac_config_user_cap_syn(mac_vap, mac_user) != OAL_SUCC) {
        oam_error_log0(mac_user->uc_vap_id, 0, "{hmac_ap_up_update_sta_user::usr_cap_syn fail}");
    }
    /* 处理带宽能力 */
    hmac_mgmt_update_bandwidth_cap(mac_user, hmac_vap, mac_vap);
    /* 处理空间流 */
    ret = hmac_mgmt_update_spatial_stream_cap(hmac_vap, hmac_user, mac_hdr, payload, msg_len);
    if (ret != OAL_SUCC) {
        *pstatus_code = MAC_UNSPEC_FAIL;
    }
    /* 将assoc req中信道列表信息保存到用户 */
    hmac_mgmt_update_user_supported_channel(hmac_vap, hmac_user, payload, msg_len);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ap_save_user_assoc_req
 * 功能描述  : AP 保存STA 的关联请求帧信息，以备上报内核
 * 1.日    期  : 2013年12月2日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_ap_save_user_assoc_req(hmac_user_stru *hmac_user, uint8_t *payload,
                                     uint32_t payload_len, uint8_t mgmt_frm_type)
{
    uint32_t ret;

    /* AP 保存STA 的关联请求帧信息，以备上报内核 */
    ret = hmac_user_free_asoc_req_ie(hmac_user->st_user_base_info.us_assoc_id);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_ap_save_user_assoc_req :: hmac_user_free_asoc_req_ie fail.}");
        return OAL_FAIL;
    }

    /*  &  */
    /* 目前11r没有实现，所以处理重关联帧的流程和关联帧一样，11r实现后此处需要修改 */
    return hmac_user_set_asoc_req_ie(hmac_user,
                                     payload + MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN,
                                     payload_len - MAC_CAP_INFO_LEN - MAC_LIS_INTERVAL_IE_LEN,
                                     (uint8_t)(mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ));
}

void hmac_ap_user_assoc_succ_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_status_code_enum_uint16 status_code)
{
    uint32_t ret;

    if (status_code != MAC_SUCCESSFUL_STATUSCODE) {
        return;
    }
    /* AP检测STA成功，允许其关联成功 */
    ret = hmac_config_user_rate_info_syn(&(hmac_vap->st_vap_base_info), &hmac_user->st_user_base_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_ap_user_assoc_succ_notify::hmac_config_user_rate_info_syn failed[%d].}", ret);
    }

    /*  user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg((&hmac_vap->st_vap_base_info), hmac_user->st_user_base_info.us_assoc_id);
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
void hmac_ap_report_sta_leave(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, const unsigned char *sta_addr)
{
    oal_net_device_stru *net_device = NULL;

    /* 判断当前状态，如果用户已经关联成功则向上报用户离开信息 */
    if (hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        return;
    }
    net_device = hmac_vap_get_net_device(hmac_vap->st_vap_base_info.uc_vap_id);
    if (net_device == NULL) {
        return;
    }
    oal_kobject_uevent_env_sta_leave(net_device, sta_addr);
}
#endif

/*
 * 函 数 名  : hmac_ap_send_assoc_rsp
 * 功能描述  : AP侧发送对应STA的关联响应消息，从hmac_ap_up_rx_assoc_req提取hmac_ap_send_assoc_rsp
 * 1.日    期  : 2019年1月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ap_send_assoc_rsp(hmac_vap_stru *hmac_vap,
                                hmac_user_stru *hmac_user,
                                const unsigned char *puc_sta_addr,
                                uint8_t mgmt_frm_type,
                                mac_status_code_enum_uint16 *pstatus_code)
{
    oal_netbuf_stru *pst_asoc_rsp;
    uint32_t asoc_rsp_len, rslt;
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint8_t uc_tx_mgmt_frm_type;

    pst_asoc_rsp = (oal_netbuf_stru *)oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
                                                           WLAN_MEM_NETBUF_SIZE2,
                                                           OAL_NETBUF_PRIORITY_MID);
    if (pst_asoc_rsp == NULL) {
        oam_error_log0(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_ap_up_rx_asoc_req::pst_asoc_rsp null.}");
        /* 异常返回之前删除user */
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_asoc_rsp);
    memset_s(pst_tx_ctl, oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

    oal_mem_netbuf_trace(pst_asoc_rsp, OAL_TRUE);

    /* update tx mgmt frame type according to assoc req type */
    uc_tx_mgmt_frm_type = (mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ ? WLAN_FC0_SUBTYPE_REASSOC_RSP :
        WLAN_FC0_SUBTYPE_ASSOC_RSP);

    asoc_rsp_len = hmac_mgmt_encap_asoc_rsp_ap(&(hmac_vap->st_vap_base_info), puc_sta_addr,
        hmac_user->st_user_base_info.us_assoc_id, *pstatus_code, oal_netbuf_header(pst_asoc_rsp), uc_tx_mgmt_frm_type);
    if (asoc_rsp_len == 0) {
        oam_warning_log0(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::hmac_mgmt_encap_asoc_rsp_ap encap msg fail.}");
        oal_netbuf_free(pst_asoc_rsp);

        /* 异常返回之前删除user */
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);

        return OAL_FAIL;
    }

    oal_netbuf_put(pst_asoc_rsp, asoc_rsp_len);

    mac_get_cb_tx_user_idx(pst_tx_ctl) = hmac_user->st_user_base_info.us_assoc_id;
    mac_get_cb_mpdu_len(pst_tx_ctl) = (uint16_t)asoc_rsp_len;

    /* 发送关联响应帧之前，将用户的节能状态复位 */
    hmac_mgmt_reset_psm(&hmac_vap->st_vap_base_info, mac_get_cb_tx_user_idx(pst_tx_ctl));

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_ap_report_sta_leave(hmac_vap, hmac_user, puc_sta_addr);
#endif

    if ((*pstatus_code != MAC_SUCCESSFUL_STATUSCODE) && (*pstatus_code != MAC_REJECT_TEMP)) {
        hmac_user_set_asoc_state(&(hmac_vap->st_vap_base_info), &hmac_user->st_user_base_info,
            MAC_USER_STATE_AUTH_COMPLETE);
    }

    rslt = hmac_tx_mgmt_send_event(&(hmac_vap->st_vap_base_info), pst_asoc_rsp, (uint16_t)asoc_rsp_len);
    if (rslt != OAL_SUCC) {
        oam_warning_log1(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::hmac_tx_mgmt_send_event failed[%d].}", rslt);
        oal_netbuf_free(pst_asoc_rsp);

        /* 异常返回之前删除user */
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);

        return rslt;
    }

    hmac_ap_user_assoc_succ_notify(hmac_vap, hmac_user, *pstatus_code);

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_ap_up_rx_asoc_req_payload_chk(hmac_vap_stru *hmac_vap,
    uint8_t mgmt_frm_type, uint32_t payload_len)
{
    uint8_t frm_least_len;

    frm_least_len = MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN;
    frm_least_len += (mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ? WLAN_MAC_ADDR_LEN : 0;

    /* 网络安全检测，异常关联请求帧不处理 */
    if (payload_len <= frm_least_len) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "hmac_ap_up_rx_asoc_req:assoc req len[%d] too short!",
                         payload_len);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC hmac_user_stru *hmac_ap_up_rx_asoc_req_user_proc(hmac_vap_stru *hmac_vap, uint8_t *sta_addr,
                                                            uint16_t *user_idx)
{
    uint32_t rslt;
    hmac_user_stru *hmac_user = NULL;

    rslt = mac_vap_find_user_by_macaddr(&(hmac_vap->st_vap_base_info), sta_addr, user_idx);
    if (rslt != OAL_SUCC) {
        oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::find user by macaddr %02X:XX:XX:XX:%02X:%02X result is %d }",
                         sta_addr[MAC_ADDR_0], sta_addr[MAC_ADDR_4], sta_addr[MAC_ADDR_5], rslt);
        hmac_mgmt_send_deauth_frame(&(hmac_vap->st_vap_base_info), sta_addr, MAC_ASOC_NOT_AUTH, OAL_FALSE);
        return NULL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(*user_idx);
    if (hmac_user == NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "{hmac_ap_up_rx_asoc_req::hmac_user[%d] null.}", *user_idx);
        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame(&(hmac_vap->st_vap_base_info), sta_addr, MAC_ASOC_NOT_AUTH, OAL_FALSE);
        return NULL;
    }

    if (hmac_user->st_mgmt_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&hmac_user->st_mgmt_timer);
    }
    return hmac_user;
}

static void hmac_sync_base_info(hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap, uint8_t mgmt_frm_type,
                                uint8_t *payload, uint32_t payload_len, uint16_t *status_code)
{
    uint32_t rslt;
    int32_t len;

    if (*status_code != MAC_SUCCESSFUL_STATUSCODE) {
        oam_warning_log1(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::hmac_ap_up_update_sta_user failed[%d].}", *status_code);
        hmac_user_set_asoc_state(&(hmac_vap->st_vap_base_info),
                                 &hmac_user->st_user_base_info,
                                 MAC_USER_STATE_AUTH_COMPLETE);
    }

    /* check OWE capability in assoc req frame */
    len = ((mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ?
                (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN + WLAN_MAC_ADDR_LEN) : (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN));
    hmac_ap_check_owe_capa(hmac_vap, hmac_user, (payload + len), (payload_len - len), status_code);

    /* 同步ap带宽，能力等信息到dmac */
    hmac_chan_sync(&hmac_vap->st_vap_base_info, hmac_vap->st_vap_base_info.st_channel.uc_chan_number,
                   hmac_vap->st_vap_base_info.st_channel.en_bandwidth, OAL_FALSE);

    /* 根据用户支持带宽能力，协商出当前带宽，dmac offload架构下，同步带宽信息到device */
    rslt = hmac_config_user_info_syn(&(hmac_vap->st_vap_base_info), &hmac_user->st_user_base_info);
    if (rslt != OAL_SUCC) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "{hmac_ap_up_rx_asoc_req::usr_info_syn failed[%d].}", rslt);
    }
}

static void hmac_check_p2p_ie(uint8_t mgmt_frm_type, hmac_vap_stru *hmac_vap,
                              hmac_user_stru *hmac_user, uint8_t *payload, uint32_t payload_len)
{
    int32_t len;
    len = ((mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ?
           (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN + WLAN_MAC_ADDR_LEN) : (MAC_CAP_INFO_LEN + MAC_LISTEN_INT_LEN));
    if (is_p2p_go(&hmac_vap->st_vap_base_info) &&
        (mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P,
                            payload + len, (int32_t)payload_len - len) == NULL)) {
        oam_info_log1(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                      "{hmac_ap_up_rx_asoc_req::GO got assoc request from legacy device, length = [%d]}",
                      payload_len);
        hmac_disable_p2p_pm(hmac_vap);
    }
}

static void hmac_ap_check_sta_fail(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint16_t status_code)
{
    if (status_code != MAC_REJECT_TEMP) {
        if (hmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC == OAL_FALSE) {
            if (status_code == MAC_INVALID_AKMP_CIPHER) {
                hmac_mgmt_send_deauth_frame(&hmac_vap->st_vap_base_info, hmac_user->st_user_base_info.auc_user_mac_addr,
                                            MAC_AUTH_NOT_VALID, OAL_FALSE);
            }
        }
        hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    }
}
OAL_STATIC void hmac_ap_up_rx_asoc_req_change_user_state_to_auth(mac_vap_stru *mac_vap, hmac_user_stru *hmac_user)
{
#ifdef _PRE_WLAN_FEATURE_PWL
    // PWL模式下不需要走pmf的探测机制
    if (mac_vap->pwl_enable == OAL_TRUE && (hmac_user->assoc_ap_up_tx_auth_req)) {
#else
    // 只有非PMF场景才需要改变user状态，否则保持user assoc态
    if ((mac_mib_get_dot11RSNAMFPC(mac_vap) != OAL_TRUE) && (hmac_user->assoc_ap_up_tx_auth_req)) {
#endif
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_asoc_req::RX (re)assoc req, change user to auth.}");
        hmac_user->assoc_ap_up_tx_auth_req = OAL_FALSE;
        hmac_user_set_asoc_state(mac_vap, &hmac_user->st_user_base_info,
            MAC_USER_STATE_AUTH_COMPLETE);
    }
}
#ifdef _PRE_WLAN_FEATURE_PMF
OAL_STATIC uint32_t hmac_ap_up_rx_asoc_req_pmf_process(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, mac_status_code_enum_uint16 *status_code)
{
    uint32_t rslt;
#ifdef _PRE_WLAN_FEATURE_PWL
    /* PWL模式下不进行PMF流程 */
    if (hmac_vap->st_vap_base_info.pwl_enable == OAL_TRUE) {
        return OAL_SUCC;
    }
#endif

    if ((hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (hmac_user->st_user_base_info.st_cap_info.bit_pmf_active == OAL_TRUE)) {
        hmac_user->attack_mgmt.pmf_rx_asoc_cnt++;
        rslt = hmac_start_sa_query(&hmac_vap->st_vap_base_info, hmac_user,
                                   hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
        if (rslt != OAL_SUCC) {
            oam_warning_log1(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_ap_up_rx_asoc_req_pmf_process::hmac_start_sa_query failed[%d].}", rslt);
            return rslt;
        }
        oam_info_log0(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC,
                      "{hmac_ap_up_rx_asoc_req_pmf_process::set status_code is MAC_REJECT_TEMP.}");
        *status_code = MAC_REJECT_TEMP;
    }
    return OAL_SUCC;
}
#endif

OAL_STATIC uint32_t hmac_ap_up_rx_asoc_status_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_rx_ctl_stru *rx_info, mac_status_code_enum_uint16 *status_code)
{
    uint8_t *mac_hdr = NULL;
    uint8_t *payload = NULL;
    mac_vap_stru *mac_vap = NULL;
    uint8_t sta_addr[WLAN_MAC_ADDR_LEN];
    uint32_t rslt, payload_len;
    mac_status_code_enum_uint16 cur_status = *status_code;
    uint8_t mgmt_frm_type;

    if (cur_status == MAC_REJECT_TEMP) {
        return OAL_SUCC;
    }
    payload_len = rx_info->us_frame_len - rx_info->uc_mac_header_len;

    mac_hdr = (uint8_t *)mac_get_rx_cb_mac_header_addr(rx_info);
    payload = (uint8_t *)(mac_hdr) + rx_info->uc_mac_header_len;
    mac_vap = &(hmac_vap->st_vap_base_info);
    mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);
    mac_get_address2(mac_hdr, sta_addr, WLAN_MAC_ADDR_LEN);

    /* 当可以查找到用户时,说明当前USER的状态为已关联或已认证完成 处理用户相关信息以及能力交互 */
    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CONN,
        "{hmac_ap_up_rx_asoc_req:: ASSOC_REQ rx : user mac:%02X:XX:XX:XX:%02X:%02X.}",
        sta_addr[0], sta_addr[MAC_ADDR_4], sta_addr[MAC_ADDR_5]);

    if (hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) {
        oam_warning_log0(0, 0, "{hmac_ap_up_rx_asoc_req::user associated, unexpected (re)assoc req no handle!}");
        return OAL_FAIL;
    }

    rslt = hmac_ap_up_update_sta_user(hmac_vap, mac_hdr, payload, payload_len, hmac_user, &cur_status);
    hmac_sync_base_info(hmac_user, hmac_vap, mgmt_frm_type, payload, payload_len, &cur_status);
    if (cur_status == MAC_SUCCESSFUL_STATUSCODE) {
        /* add 2013-10-17 wifi */
        rslt = hmac_init_security(mac_vap, sta_addr, sizeof(sta_addr));
        if (rslt != OAL_SUCC) {
            oam_error_log2(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_ASSOC, "{hmac_ap_up_rx_asoc_req:: \
                hmac_init_security failed[%d] status_code[%d].}", rslt, MAC_UNSPEC_FAIL);
            cur_status = MAC_UNSPEC_FAIL;
        }
        rslt = hmac_init_user_security_port(mac_vap, &(hmac_user->st_user_base_info));
        if (rslt != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ASSOC,
                "{hmac_ap_up_rx_asoc_req::hmac_init_user_security_port failed[%d].}", rslt);
        }
    }
    if ((rslt != OAL_SUCC) || (cur_status != MAC_SUCCESSFUL_STATUSCODE)) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_ap_up_rx_asoc_req:: \
            hmac_ap_up_update_sta_user fail rslt[%d] status_code[%d].", rslt, cur_status);
        hmac_user_set_asoc_state(mac_vap, &hmac_user->st_user_base_info, MAC_USER_STATE_AUTH_COMPLETE);
    }
    *status_code = cur_status;
    hmac_check_p2p_ie(mgmt_frm_type, hmac_vap, hmac_user, payload, payload_len);
    return OAL_SUCC;
}

/* 关联成功处理 */
OAL_STATIC void hmac_ap_up_process_asoc_succ(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, uint8_t *payload,
    uint32_t payload_len, uint8_t mgmt_frm_type)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    uint8_t frm_least_len;
    frm_least_len = MAC_CAP_INFO_LEN + MAC_LIS_INTERVAL_IE_LEN;
    frm_least_len += (mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ? WLAN_MAC_ADDR_LEN : 0;
#endif
    /* AP 保存STA 的关联请求帧信息，以备上报内核 */
    hmac_ap_save_user_assoc_req(hmac_user, payload, payload_len, mgmt_frm_type);

    /* 上报WAL层(WAL上报内核) AP关联上了一个新的STA */
    hmac_handle_connect_rsp_ap(hmac_vap, hmac_user);
#ifdef _PRE_WLAN_CHBA_MGMT
    /* CHBA关联成功处理 */
    hmac_chba_connect_succ_handler(hmac_vap, hmac_user, payload + frm_least_len, payload_len - frm_least_len);
#endif
    hmac_process_assoc_dyn_pcie_vote(hmac_vap, hmac_user);
}

OAL_STATIC void hmac_ap_up_process_asoc_result(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_rx_ctl_stru *rx_info, mac_status_code_enum_uint16 status_code, uint16_t user_idx)
{
    mac_vap_stru *mac_vap = NULL;
    uint8_t sta_addr[WLAN_MAC_ADDR_LEN];
    uint8_t *payload = NULL;
    uint8_t mgmt_frm_type;
    uint8_t *mac_hdr = NULL;
    uint32_t payload_len;

    mac_hdr = (uint8_t *)mac_get_rx_cb_mac_header_addr(rx_info);
    mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);
    payload_len = rx_info->us_frame_len - rx_info->uc_mac_header_len;
    payload = (uint8_t *)(mac_hdr) + rx_info->uc_mac_header_len;
    mac_get_address2(mac_hdr, sta_addr, WLAN_MAC_ADDR_LEN);
    mac_vap = &(hmac_vap->st_vap_base_info);

    if (status_code == MAC_SUCCESSFUL_STATUSCODE) {
        hmac_user_set_asoc_state(mac_vap, &hmac_user->st_user_base_info, MAC_USER_STATE_ASSOC);
    }

    if (hmac_user->st_hmac_cap_info.bit_owe == OAL_TRUE) {
        /* APUT received OWE assoc req && check valid OWE DH Parameter Element, report assoc req to hostapd;
         * otherwise, APUT send assoc rsp frame with reject status code */
        if (status_code == MAC_SUCCESSFUL_STATUSCODE) {
            hmac_ap_up_process_asoc_succ(hmac_vap, hmac_user, payload, payload_len, mgmt_frm_type);

            oam_warning_log4(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_CONN,
                             "{new OWE STA assoc AP! STA_indx=%d. user mac:%02X:XX:XX:XX:%02X:%02X.}",
                             user_idx, sta_addr[MAC_ADDR_0], sta_addr[MAC_ADDR_4], sta_addr[MAC_ADDR_5]);
        } else {
            hmac_ap_send_assoc_rsp(hmac_vap, hmac_user, sta_addr, mgmt_frm_type, &status_code);
            hmac_user_del(mac_vap, hmac_user);
        }
    } else {
        hmac_ap_send_assoc_rsp(hmac_vap, hmac_user, sta_addr, mgmt_frm_type, &status_code);

        if (status_code == MAC_SUCCESSFUL_STATUSCODE) {
            hmac_ap_up_process_asoc_succ(hmac_vap, hmac_user, payload, payload_len, mgmt_frm_type);
            oam_warning_log4(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_CONN,
                             "{ASSOC_RSP tx,STA assoc AP SUCC! STA_idx=%d.user mac:%02X:XX:XX:XX:%02X:%02X}",
                             user_idx, sta_addr[MAC_ADDR_0], sta_addr[MAC_ADDR_4], sta_addr[MAC_ADDR_5]);
        } else {
            /* AP检测STA失败，将其删除 */
            hmac_ap_check_sta_fail(hmac_vap, hmac_user, status_code);
        }
    }
}
/*
 * 功能描述  : AP侧接收到对应STA的关联请求消息
 * 1.日    期  : 2013年7月2日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_ap_up_rx_asoc_req(hmac_vap_stru *hmac_vap, mac_rx_ctl_stru *rx_info)
{
    uint32_t rslt, payload_len;
    hmac_user_stru *hmac_user = NULL;
    uint16_t user_idx = 0;
    mac_status_code_enum_uint16 status_code = MAC_SUCCESSFUL_STATUSCODE;
    uint8_t sta_addr[WLAN_MAC_ADDR_LEN];
    mac_cfg_user_info_param_stru hmac_user_info_event;
    mac_vap_stru *mac_vap = NULL;
    uint8_t mgmt_frm_type;
    uint8_t *mac_hdr = NULL;

    mac_hdr = (uint8_t *)mac_get_rx_cb_mac_header_addr(rx_info);
    mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);
    payload_len = rx_info->us_frame_len - rx_info->uc_mac_header_len;

    if (hmac_ap_up_rx_asoc_req_payload_chk(hmac_vap, mgmt_frm_type, payload_len) != OAL_SUCC) {
        return OAL_FAIL;
    }

    mac_get_address2(mac_hdr, sta_addr, WLAN_MAC_ADDR_LEN);
#ifdef _PRE_WLAN_CHBA_MGMT
    rslt = hmac_chba_fast_conn_response_rx_assoc_proc(hmac_vap, rx_info, mac_hdr);
    if (rslt != OAL_SUCC) {
        return rslt;
    }
#endif
    hmac_user = hmac_ap_up_rx_asoc_req_user_proc(hmac_vap, sta_addr, &user_idx);
    if (hmac_user == NULL) {
        return OAL_FAIL;
    }
    hmac_ap_up_rx_asoc_req_change_user_state_to_auth(&(hmac_vap->st_vap_base_info), hmac_user);
#ifdef _PRE_WLAN_FEATURE_PWL
    /* 为抵御auth攻击，不再清除秘钥 */
    hmac_pwl_aput_init_pwl_pre_cb(hmac_user); /* 此时鉴权数据帧，需要用预制秘钥加解密 */
#endif
    /* 是否符合触发SA query流程的条件 */
#ifdef _PRE_WLAN_FEATURE_PMF
    rslt = hmac_ap_up_rx_asoc_req_pmf_process(hmac_vap, hmac_user, &status_code);
    if (rslt != OAL_SUCC) {
        return rslt;
    }
#endif
    mac_vap = &(hmac_vap->st_vap_base_info);

    /* 不同的关联状态处理 */
    rslt = hmac_ap_up_rx_asoc_status_process(hmac_vap, hmac_user, rx_info, &status_code);
    if (rslt != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 不同的关联结果处理 */
    hmac_ap_up_process_asoc_result(hmac_vap, hmac_user, rx_info, status_code, user_idx);
    /* STA 入网后，上报VAP 信息和用户信息 */
    hmac_user_info_event.us_user_idx = user_idx;
    hmac_config_vap_info(mac_vap, sizeof(uint32_t), (uint8_t *)&rslt);
    hmac_config_user_info(mac_vap, sizeof(mac_cfg_user_info_param_stru), (uint8_t *)&hmac_user_info_event);

    return OAL_SUCC;
}

static uint32_t hmac_ap_up_rx_disasoc_in_assoc_state(hmac_user_stru *hmac_user, oal_bool_enum_uint8 is_protected,
                                                     uint8_t *mac_hdr, hmac_vap_stru *hmac_vap)
{
    uint32_t ret;
#ifdef _PRE_WLAN_FEATURE_PMF
        /* 检查是否需要发送SA query request */
        if (hmac_pmf_check_err_code(&hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info,
            is_protected, mac_hdr) == OAL_SUCC) {
            hmac_user->attack_mgmt.pmf_rx_disasoc_cnt++;
            /* 在关联状态下收到未加密的ReasonCode 6/7需要启动SA Query流程 */
            ret = hmac_start_sa_query(&hmac_vap->st_vap_base_info, hmac_user,
                                      hmac_user->st_user_base_info.st_cap_info.bit_pmf_active);
            if (ret != OAL_SUCC) {
                oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                    "{hmac_ap_up_rx_disasoc_in_assoc_state::hmac_start_sa_query failed[%d].}", ret);
                return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
            }
            return OAL_SUCC;
        }
        /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
        if (hmac_mgmt_pmf_frame_filter(&hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info,
            (mac_ieee80211_frame_stru *)mac_hdr, is_protected) != OAL_SUCC) {
            return OAL_FAIL;
        }
#endif
        mac_user_set_asoc_state(&hmac_user->st_user_base_info, MAC_USER_STATE_AUTH_COMPLETE);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                             CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_PASSIVE_DISCONNECT,
                             *((uint16_t *)(mac_hdr + MAC_80211_FRAME_LEN)));

        /* 抛事件上报内核，已经去关联某个STA */
        hmac_handle_disconnect_rsp_ap(hmac_vap, hmac_user);
        /* 有些网卡去关联时只发送DISASOC,也将删除其在AP内部的数据结构 */
        ret = hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
        if (ret != OAL_SUCC) {
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_ap_up_rx_disasoc_in_assoc_state::hmac_user_del failed[%d].}", ret);
        }

        return OAL_SUCC;
}

/*
 * 功能描述  : AP侧接收到对应STA的去关联请求消息
 * 1.日    期  : 2013年7月2日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_ap_up_rx_disasoc(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr, uint32_t mac_hdr_len,
                                          uint32_t payload_len, oal_bool_enum_uint8 is_protected)
{
    hmac_user_stru *hmac_user = NULL;
    uint8_t *sa = NULL;
    uint8_t auc_sta_addr[WLAN_MAC_ADDR_LEN];

    if (payload_len < MAC_80211_REASON_CODE_LEN) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CONN,
                         "{hmac_ap_up_rx_disasoc::invalid payload_len len [%d]}", payload_len);
        return OAL_FAIL;
    }

    mac_get_address2(mac_hdr, auc_sta_addr, WLAN_MAC_ADDR_LEN);

    /* 增加接收到去关联帧时的维测信息 */
    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);

    oam_warning_log4(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CONN,
                     "{hmac_ap_up_rx_disasoc::DISASSOC rx, Because of err_code[%d], \
                     received disassoc frame from source addr %02X:XX:XX:XX:%02X:%02X.}",
                     *((uint16_t *)(mac_hdr + MAC_80211_FRAME_LEN)), sa[0], sa[MAC_ADDR_4], sa[MAC_ADDR_5]);

    hmac_user = mac_vap_get_hmac_user_by_addr(&(hmac_vap->st_vap_base_info), &auc_sta_addr[0], WLAN_MAC_ADDR_LEN);
    if (hmac_user == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "{hmac_ap_up_rx_disasoc::hmac_user null.}");
        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame(&(hmac_vap->st_vap_base_info), auc_sta_addr, MAC_NOT_ASSOCED, OAL_FALSE);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_user->st_user_base_info.en_user_asoc_state == MAC_USER_STATE_ASSOC) {
        return hmac_ap_up_rx_disasoc_in_assoc_state(hmac_user, is_protected, mac_hdr, hmac_vap);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ap_up_rx_action_nonuser
 * 功能描述  : AP在UP状态下的接收ACTION帧处理
 * 1.日    期  : 2014年2月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_ap_up_rx_action_nonuser(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    uint8_t *payload = NULL;
    uint16_t payload_len;

    payload = mac_get_rx_payload(netbuf, &payload_len);
    if (payload == NULL) {
        return;
    }
    if (payload_len < MAC_ACTION_CATEGORY_AND_CODE_LEN) {
        return;
    }
    if ((payload[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_PUBLIC) &&
        (payload[MAC_ACTION_OFFSET_ACTION] == MAC_PUB_VENDOR_SPECIFIC)) {
#ifdef _PRE_WLAN_FEATURE_LOCATION
        hmac_huawei_action_process(hmac_vap, netbuf);
#endif
    }
}

void hmac_action_category_ba(uint8_t *data, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    uint32_t frame_body_len)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_BA_ACTION_ADDBA_REQ:
            hmac_mgmt_rx_addba_req(hmac_vap, hmac_user, data, frame_body_len);
            break;

        case MAC_BA_ACTION_ADDBA_RSP:
            hmac_mgmt_rx_addba_rsp(hmac_vap, hmac_user, data, frame_body_len);
            break;

        case MAC_BA_ACTION_DELBA:
            hmac_mgmt_rx_delba(hmac_vap, hmac_user, data, frame_body_len);
            break;

        default:
            break;
    }
}

static void hmac_action_category_public(
    hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, uint8_t *payload, uint16_t payload_len)
{    /* Action */
    switch (payload[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_PUB_VENDOR_SPECIFIC: {
            if (payload_len <= MAC_ACTION_CATEGORY_AND_CODE_LEN + MAC_OUI_LEN) {
                oam_warning_log1(0, OAM_SF_RX, "{hmac_action_category_public::payload_len %d.}", payload_len);
                return;
            }
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0)  */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (mac_ie_check_p2p_action(payload + MAC_ACTION_CATEGORY_AND_CODE_LEN,
                payload_len - MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host(hmac_vap, netbuf);
            }
#if defined(_PRE_WLAN_FEATURE_LOCATION)
            hmac_huawei_action_process(hmac_vap, netbuf);
#endif
            break;
        }
        default:
            break;
    }
}

static void hmac_action_category_ht(uint8_t *data)
{    /* Action */
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH:
            break;

        case MAC_HT_ACTION_BUTT:
        default:
            break;
    }
}

static void hmac_action_category_sa_query(uint8_t *data, hmac_vap_stru *hmac_vap,
                                          oal_netbuf_stru *netbuf, oal_bool_enum_uint8 is_protected)
{    /* Action */
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_SA_QUERY_ACTION_REQUEST:
            hmac_rx_sa_query_req(hmac_vap, netbuf, is_protected);
            break;
        case MAC_SA_QUERY_ACTION_RESPONSE:
            hmac_rx_sa_query_rsp(hmac_vap, netbuf, is_protected);
            break;
        default:
            break;
    }
}

static void hmac_action_category_wmmac_qos(uint8_t *data, hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    if (g_en_wmmac_switch == OAL_TRUE) {
        switch (data[MAC_ACTION_OFFSET_ACTION]) {
            case MAC_WMMAC_ACTION_ADDTS_REQ:
                hmac_mgmt_rx_addts_req_frame(hmac_vap, netbuf);
                break;

            default:
                break;
        }
    }
}

static void hmac_ap_up_rx_action_category(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, oal_bool_enum_uint8 is_protected)
{
    uint8_t *payload = NULL;
    mac_rx_ctl_stru *rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint16_t payload_len = hmac_get_frame_body_len(netbuf);

    if (rx_ctl == NULL) {
        return;
    }
    payload = mac_get_rx_payload_addr(rx_ctl, netbuf);
    if (payload == NULL) {
        return;
    }
    /* Category */
    switch (payload[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_BA: {
            hmac_action_category_ba(payload, hmac_vap, hmac_user, payload_len);
            break;
        }

        case MAC_ACTION_CATEGORY_PUBLIC: {
            hmac_action_category_public(hmac_vap, netbuf, payload, payload_len);
            break;
        }

        case MAC_ACTION_CATEGORY_HT: {
            hmac_action_category_ht(payload);
            break;
        }
#ifdef _PRE_WLAN_FEATURE_PMF
        case MAC_ACTION_CATEGORY_SA_QUERY: {
            hmac_action_category_sa_query(payload, hmac_vap, netbuf, is_protected);
            break;
        }
#endif
        case MAC_ACTION_CATEGORY_VENDOR: {
            if (payload_len < MAC_ACTION_CATEGORY_AND_CODE_LEN) {
                return;
            }
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0)  */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (mac_ie_check_p2p_action(payload + MAC_ACTION_CATEGORY_AND_CODE_LEN,
                payload_len - MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host(hmac_vap, netbuf);
            }
            break;
        }

#ifdef _PRE_WLAN_FEATURE_WMMAC
        case MAC_ACTION_CATEGORY_WMMAC_QOS: {
            hmac_action_category_wmmac_qos(payload, hmac_vap, netbuf);
            break;
        }
#endif  // _PRE_WLAN_FEATURE_WMMAC

        default:
            break;
    }
}

/*
 * 函 数 名  : hmac_ap_up_rx_action
 * 功能描述  : AP在UP状态下的接收ACTION帧处理
 * 1.日    期  : 2014年2月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_ap_up_rx_action(hmac_vap_stru *hmac_vap,
                                     oal_netbuf_stru *netbuf,
                                     oal_bool_enum_uint8 en_is_protected)
{
    dmac_rx_ctl_stru *rx_ctrl = NULL;
    mac_ieee80211_frame_stru *pst_frame_hdr = NULL; /* 保存mac帧的指针 */
    hmac_user_stru *hmac_user = NULL;

    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    if (hmac_get_frame_body_len(netbuf) < MAC_ACTION_CATEGORY_AND_CODE_LEN) {
        oam_warning_log1(0, OAM_SF_RX, "{ap_up::frame len too short[%d].}", rx_ctrl->st_rx_info.us_frame_len);
        return;
    }
    /* 获取帧头信息 */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(&rx_ctrl->st_rx_info);

    /* 获取发送端的用户指针 */
    hmac_user = mac_vap_get_hmac_user_by_addr(&hmac_vap->st_vap_base_info, pst_frame_hdr->auc_address2,
        WLAN_MAC_ADDR_LEN);
    if (hmac_user == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                         "{hmac_ap_up_rx_action::mac_vap_find_user_by_macaddr failed.}");
        hmac_ap_up_rx_action_nonuser(hmac_vap, netbuf);
        return;
    }
    hmac_ap_up_rx_action_category(hmac_vap, hmac_user, netbuf, en_is_protected);
    return;
}

/*
 * 函 数 名  : hmac_ap_up_rx_probe_req
 * 功能描述  : AP在UP状态下的接收WPS probe req帧处理
 * 1.日    期  : 2014年5月17日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_ap_up_rx_probe_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = NULL;
    mac_rx_ctl_stru *pst_rx_info = NULL;
    int32_t l_freq;
    uint8_t band;

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));

    band = hmac_get_80211_band_type(&hmac_vap->st_vap_base_info.st_channel);
    l_freq = oal_ieee80211_channel_to_frequency(hmac_vap->st_vap_base_info.st_channel.uc_chan_number, band);

    /* 上报接收到的probe req 管理帧 */
    hmac_send_mgmt_to_host(hmac_vap, pst_netbuf, pst_rx_info->us_frame_len, l_freq);
}

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
void hmac_ap_wkup_src_parse(hmac_vap_stru *hmac_vap, uint8_t mgmt_frm_type)
{
    if (wlan_pm_wkup_src_debug_get() != OAL_TRUE) {
        return;
    }
    wlan_pm_wkup_src_debug_set(OAL_FALSE);
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
        "{wifi_wake_src:hmac_ap_wkup_src_parse::wakeup mgmt type[0x%x]}", mgmt_frm_type);
}
#endif

void hmac_ap_up_rx_deauth_req(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr, uint32_t mac_hdr_len, uint32_t msg_len,
    oal_bool_enum_uint8 is_protected)
{
    if (msg_len < mac_hdr_len + MAC_80211_REASON_CODE_LEN) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
            "{hmac_ap_up_rx_deauth_req::invalid deauth_req length[%d]}", msg_len);
        return;
    }
    hmac_ap_rx_deauth_req(hmac_vap, mac_hdr, is_protected);
}

uint32_t hmac_ap_check_blacklist_result(hmac_vap_stru *hmac_vap, uint8_t *mac_hdr, uint8_t mgmt_frm_type)
{
    oal_bool_enum_uint8 blacklist_result;
    uint8_t *sa = NULL;

    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);
    /* 自动加入黑名单检查 */
    if (oal_value_eq_any2(mgmt_frm_type, WLAN_FC0_SUBTYPE_ASSOC_REQ, WLAN_FC0_SUBTYPE_REASSOC_REQ)) {
        hmac_autoblacklist_filter(&hmac_vap->st_vap_base_info, sa);
    }

    /* 黑名单过滤检查 */
    blacklist_result = hmac_blacklist_filter(&hmac_vap->st_vap_base_info, sa, WLAN_MAC_ADDR_LEN);
    if ((blacklist_result == OAL_TRUE) && (mgmt_frm_type != WLAN_FC0_SUBTYPE_AUTH)) {
        return OAL_SUCC;
    }
    return OAL_SUCC_GO_ON;
}

void hmac_ap_handle_mgmt_frm_type(hmac_vap_stru *hmac_vap, mac_rx_ctl_stru *rx_info,
    dmac_wlan_crx_event_stru *mgmt_rx_event)
{
    oal_bool_enum_uint8 protected_status;
    uint8_t mgmt_frm_type;
    uint8_t *mac_hdr = NULL;
    uint32_t msg_len;     /* 消息总长度,不包括FCS */
    uint32_t mac_hdr_len; /* MAC头长度 */

    mac_hdr = (uint8_t *)mac_get_rx_cb_mac_header_addr(rx_info);
    mac_hdr_len = rx_info->uc_mac_header_len; /* MAC头长度 */
    msg_len = rx_info->us_frame_len; /* 消息总长度,不包括FCS */
    protected_status = (uint8_t)mac_is_protectedframe(mac_hdr);
    mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);

    switch (mgmt_frm_type) {
        case WLAN_FC0_SUBTYPE_AUTH:
            hmac_ap_rx_auth_req(hmac_vap, mgmt_rx_event->pst_netbuf);
            break;

        case WLAN_FC0_SUBTYPE_DEAUTH:
            hmac_ap_up_rx_deauth_req(hmac_vap, mac_hdr, mac_hdr_len, msg_len, protected_status);
            break;

        case WLAN_FC0_SUBTYPE_ASSOC_REQ:
        case WLAN_FC0_SUBTYPE_REASSOC_REQ:
            hmac_ap_up_rx_asoc_req(hmac_vap, rx_info);
            break;

        case WLAN_FC0_SUBTYPE_DISASSOC:
            hmac_ap_up_rx_disasoc(hmac_vap, mac_hdr,
                                  mac_hdr_len, (msg_len - mac_hdr_len), protected_status);
            break;

        case WLAN_FC0_SUBTYPE_ACTION:
            hmac_ap_up_rx_action(hmac_vap, mgmt_rx_event->pst_netbuf, protected_status);
            break;

        case WLAN_FC0_SUBTYPE_PROBE_REQ:
            hmac_ap_up_rx_probe_req(hmac_vap, mgmt_rx_event->pst_netbuf);
            break;

        default:
            break;
    }
}

/*
 * 函 数 名  : hmac_ap_up_rx_mgmt
 * 功能描述  : AP在UP状态下的接收管理帧处理
 * 1.日    期  : 2013年6月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_ap_up_rx_mgmt(hmac_vap_stru *hmac_vap, void *p_param)
{
    dmac_wlan_crx_event_stru *pst_mgmt_rx_event = NULL;
    dmac_rx_ctl_stru *pst_rx_ctrl = NULL;
    mac_rx_ctl_stru *pst_rx_info = NULL;
    uint8_t *mac_hdr = NULL;
    uint32_t mac_hdr_len; /* MAC头长度 */
    uint8_t mgmt_frm_type;

    if (oal_any_null_ptr2(hmac_vap, p_param)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_up_rx_mgmt::hmac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mgmt_rx_event = (dmac_wlan_crx_event_stru *)p_param;
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_mgmt_rx_event->pst_netbuf);
    pst_rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));
    mac_hdr = (uint8_t *)mac_get_rx_cb_mac_header_addr(pst_rx_info);
    mac_hdr_len = pst_rx_info->uc_mac_header_len; /* MAC头长度 */

    /* AP在UP状态下 接收到的各种管理帧处理 */
    mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    hmac_ap_wkup_src_parse(hmac_vap, mgmt_frm_type);
#endif
    /* Bar frame proc here */
    if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_CTL) {
        mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);
        if (mgmt_frm_type == WLAN_FC0_SUBTYPE_BAR) {
            hmac_up_rx_bar(hmac_vap, pst_rx_ctrl, pst_mgmt_rx_event->pst_netbuf);
        }
    } else if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_MGT) {
        if (hmac_ap_check_blacklist_result(hmac_vap, mac_hdr, mgmt_frm_type) == OAL_SUCC) {
            return OAL_SUCC;
        }
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
        proc_sniffer_write_file(NULL, 0, mac_hdr, pst_rx_info->us_frame_len, 0);
#endif
#endif
        hmac_ap_handle_mgmt_frm_type(hmac_vap, pst_rx_info, pst_mgmt_rx_event);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_mgmt_timeout_fn
 * 功能描述  : 管理帧超时处理函数
 * 1.日    期  : 2013年7月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_mgmt_timeout_ap(void *p_param)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user;
    uint32_t ret;

    hmac_user = (hmac_user_stru *)p_param;
    if (hmac_user == NULL) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_mgmt_timeout_ap::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->st_user_base_info.uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_AUTH,
                       "{hmac_mgmt_timeout_ap::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_AUTH,
                     "{hmac_mgmt_timeout_ap::Wait AUTH timeout!! After %d ms.}",
                     WLAN_AUTH_AP_TIMEOUT);
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI,
                         CHR_LAYER_DRV, CHR_WIFI_DRV_EVENT_SOFTAP_CONNECT,
                         MAC_AP_AUTH_RSP_TIMEOUT);
    /* 发送去关联帧消息给STA */
    hmac_mgmt_send_deauth_frame(&hmac_vap->st_vap_base_info,
                                hmac_user->st_user_base_info.auc_user_mac_addr,
                                MAC_AUTH_NOT_VALID, OAL_FALSE);

    /* 抛事件上报内核，已经去关联某个STA */
    /*   */
    hmac_handle_disconnect_rsp_ap(hmac_vap, hmac_user);

    ret = hmac_user_del(&hmac_vap->st_vap_base_info, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_user->st_user_base_info.uc_vap_id, OAM_SF_AUTH,
                         "{hmac_mgmt_timeout_ap::hmac_user_del failed[%d].}", ret);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ap_wait_start_misc
 * 功能描述  : AP在WAIT START状态下的misc事件处理
 * 1.日    期  : 2014年10月16日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_ap_wait_start_misc(hmac_vap_stru *hmac_vap, void *p_param)
{
    hmac_misc_input_stru *pst_misc_input = NULL;

    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, p_param))) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_wait_start_misc::hmac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_misc_input = (hmac_misc_input_stru *)p_param;

    switch (pst_misc_input->en_type) {
        case HMAC_MISC_RADAR:
#ifdef _PRE_WLAN_FEATURE_DFS
            hmac_dfs_ap_wait_start_radar_handler(hmac_vap);
#endif
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_ap_up_misc
 * 功能描述  : AP在UP状态下的misc事件处理
 * 1.日    期  : 2014年10月20日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_ap_up_misc(hmac_vap_stru *hmac_vap, void *p_param)
{
    hmac_misc_input_stru *pst_misc_input = NULL;

    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, p_param))) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_up_misc::hmac_vap or param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_misc_input = (hmac_misc_input_stru *)p_param;

    switch (pst_misc_input->en_type) {
        case HMAC_MISC_RADAR:
#ifdef _PRE_WLAN_FEATURE_DFS
            hmac_dfs_ap_up_radar_handler(hmac_vap);
#endif
            break;

        default:
            break;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_ap_clean_bss
 * 功能描述  : 清除BSS下所有关联用户
 * 1.日    期  : 2016年5月24日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_ap_clean_bss(hmac_vap_stru *hmac_vap)
{
    oal_dlist_head_stru *pst_entry = NULL;
    oal_dlist_head_stru *pst_next_entry = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_user_stru *pst_user_tmp = NULL;
    hmac_user_stru *hmac_user_tmp = NULL;
    oal_bool_enum_uint8 en_is_protected;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_ap_clean_bss::hmac vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 删除vap下所有已关联用户，并通知内核 */
    pst_mac_vap = &hmac_vap->st_vap_base_info;
    oal_dlist_search_for_each_safe(pst_entry, pst_next_entry, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
        if (pst_user_tmp == NULL) {
            continue;
        }

        hmac_user_tmp = mac_res_get_hmac_user(pst_user_tmp->us_assoc_id);
        if (hmac_user_tmp == NULL) {
            continue;
        }

        /* 管理帧加密是否开启 */
        en_is_protected = pst_user_tmp->st_cap_info.bit_pmf_active;

        /* 发去关联帧 */
        hmac_mgmt_send_disassoc_frame(pst_mac_vap, pst_user_tmp->auc_user_mac_addr,
                                      MAC_DISAS_LV_SS, en_is_protected);

        /* 通知内核 */
        hmac_handle_disconnect_rsp_ap(hmac_vap, hmac_user_tmp);

        /* 删除用户 */
        hmac_user_del(pst_mac_vap, hmac_user_tmp);
    }

    return OAL_SUCC;
}

/*
 * 1.日    期  : 2014年5月17日
  *   修改内容  : 新生成函数
 */
void hmac_mgmt_reset_psm(mac_vap_stru *pst_vap, uint16_t us_user_id)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    uint16_t *pus_user_id = NULL;
    hmac_user_stru *hmac_user = NULL;

    if (pst_vap == NULL) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_mgmt_reset_psm::pst_vap null.}");
        return;
    }
    /* 在这里直接做重置的一些操作，不需要再次同步 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_id);
    if (oal_unlikely(hmac_user == NULL)) {
        oam_error_log1(pst_vap->uc_vap_id, OAM_SF_PWR,
                       "{hmac_mgmt_reset_psm::hmac_user[%d] null.}",
                       us_user_id);
        return;
    }

    event_mem = frw_event_alloc_m(sizeof(uint16_t));
    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(pst_vap->uc_vap_id, OAM_SF_PWR, "{hmac_mgmt_reset_psm::event_mem null.}");
        return;
    }

    event = frw_get_event_stru(event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CTX,
                       DMAC_WLAN_CTX_EVENT_SUB_TYPE_RESET_PSM,
                       sizeof(uint16_t),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_vap->uc_chip_id,
                       pst_vap->uc_device_id,
                       pst_vap->uc_vap_id);

    pus_user_id = (uint16_t *)event->auc_event_data;

    *pus_user_id = us_user_id;

    frw_event_dispatch_event(event_mem);

    frw_event_free_m(event_mem);
}

