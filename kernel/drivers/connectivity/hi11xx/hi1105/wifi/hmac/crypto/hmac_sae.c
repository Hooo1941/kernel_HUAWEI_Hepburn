/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : sae特性相关
 * 作    者 : wifi3
 * 创建日期 : 2020年07月20日
 */

#ifdef _PRE_WLAN_FEATURE_SAE

#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_frame_inl.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_device.h"
#include "hmac_mgmt_ap.h"
#include "hmac_mgmt_sta.h"
#include "frw_ext_if.h"
#include "hmac_roam_main.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"

#include "hmac_roam_main.h"
#include "mac_mib.h"
#include "hmac_ht_self_cure.h"
#include "securec.h"
#include "hmac_sae.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_fsm.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SAE_C

/*
 * 功能描述  : STA第一次SAE关联,不包含pmkid,上报external auth事件到wpa_s
 * 1.日    期  : 2020年5月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_sta_sae_connect_info_wpas(hmac_vap_stru *hmac_sta, hmac_auth_req_stru *auth_req)
{
    hmac_user_stru *hmac_user_ap = NULL;
    uint16_t us_user_index = g_wlan_spec_cfg->invalid_user_id;
    uint32_t ret = OAL_SUCC;

    oam_warning_log0(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                     "{hmac_sta_sae_connect_info_wpas:: report external auth to wpa_s.}");

    /* 给STA 添加用户 */
    hmac_user_ap = (hmac_user_stru *)mac_res_get_hmac_user(hmac_sta->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user_ap == NULL) {
        ret = hmac_user_add(&(hmac_sta->st_vap_base_info), hmac_sta->st_vap_base_info.auc_bssid, &us_user_index);
        if (ret != OAL_SUCC) {
            oam_warning_log1(hmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "{hmac_sta_sae_connect_info_wpas:: add sae user failed[%d].}", ret);
            return OAL_FAIL;
        }
    }

    oal_workqueue_delay_schedule(&(hmac_sta->st_sae_report_ext_auth_worker), oal_msecs_to_jiffies(1));

    /* 切换STA 到MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2 */
    hmac_fsm_change_state(hmac_sta, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2);

    /* 启动认证超时定时器 */
    hmac_sta->st_mgmt_timetout_param.en_state = MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2;
    hmac_sta->st_mgmt_timetout_param.uc_vap_id = hmac_sta->st_vap_base_info.uc_vap_id;
    hmac_sta->st_mgmt_timetout_param.us_user_index = us_user_index;
    frw_timer_create_timer_m(&hmac_sta->st_mgmt_timer,
                             hmac_mgmt_timeout_sta,
                             auth_req->us_timeout,
                             &hmac_sta->st_mgmt_timetout_param,
                             OAL_FALSE,
                             OAM_MODULE_ID_HMAC,
                             hmac_sta->st_vap_base_info.core_id);
    return OAL_SUCC;
}

static void hmac_sta_process_auth_status_ap_full(hmac_vap_stru *hmac_vap)
{
    hmac_auth_rsp_stru auth_rsp;

    memset_s(&auth_rsp, sizeof(hmac_auth_rsp_stru), 0, sizeof(hmac_auth_rsp_stru));
    oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
        "{hmac_sta_wait_auth_seq2_rx::AP refuse STA auth reason ap full!}");
    /* 取消定时器 */
    frw_timer_immediate_destroy_timer_m(&hmac_vap->st_mgmt_timer);
    /*  AP Full need assoc reject with status code = 17 */
    auth_rsp.us_status_code = MAC_AP_FULL;
    /* 上报给SME认证结果 */
    hmac_send_rsp_to_sme_sta(hmac_vap, HMAC_SME_AUTH_RSP, (uint8_t *)&auth_rsp);
}

/*
 * 函 数 名  : hmac_sta_process_sae_commit
 * 功能描述  : 处理收到seq = 1 的SAE 认证帧
 * 1.日    期  : 2019年10月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_sta_process_sae_commit(hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf)
{
    hmac_user_stru *pst_hmac_user_ap = NULL;
    mac_rx_ctl_stru *rx_ctl = NULL; /* 每一个MPDU的控制信息 */
    uint8_t *payload = NULL;
    uint16_t payload_len;
    uint16_t auth_status;
    uint16_t auth_seq;

    pst_hmac_user_ap = mac_res_get_hmac_user(pst_sta->st_vap_base_info.us_assoc_vap_id);
    if (pst_hmac_user_ap == NULL) {
        oam_error_log1(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_AUTH,
            "{hmac_sta_wait_auth_seq2_rx::pst_hmac_user[%d] null.}", pst_sta->st_vap_base_info.us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf); /* 每一个MPDU的控制信息 */
    if (rx_ctl == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    payload = mac_get_rx_payload(pst_netbuf, &payload_len);
    if (payload == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    auth_seq = mac_get_auth_seq_num(payload, payload_len);
    auth_status = mac_get_auth_status(payload, payload_len);
    oam_warning_log2(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
        "{hmac_sta_wait_auth_seq2_rx::rx sae auth frame, status_code %d, seq_num %d.}",
        auth_status, auth_seq);
    if (auth_status == MAC_AP_FULL) {
        hmac_sta_process_auth_status_ap_full(pst_sta);
        return OAL_SUCC;
    }

    /* SAE commit帧的seq number是1，confirm帧的seq number是2 */
    if (auth_seq != WLAN_AUTH_TRASACTION_NUM_ONE) {
        oam_warning_log0(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
            "{hmac_sta_wait_auth_seq2_rx::drop sae auth frame for wrong seq num}");
        return OAL_SUCC;
    }

    pst_sta->duplicate_auth_seq4_flag = OAL_FALSE; // seq4置位

    /* 取消定时器 */
    frw_timer_immediate_destroy_timer_m(&pst_sta->st_mgmt_timer);

    /* SAE 判断seq number以后，上传给wpas 处理 */
    hmac_rx_mgmt_send_to_host(pst_sta, pst_netbuf);

    /* 启动认证超时定时器 */
    if ((auth_status != MAC_ANTI_CLOGGING) && (auth_status != MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED)) {
        /* 切换STA 到MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4 */
        hmac_fsm_change_state(pst_sta, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4);
        pst_sta->st_mgmt_timetout_param.en_state = MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4;
    } else {
        /* WPA3 anti-clogging 验证，会重新发送seq_num = 1 的commit auth， 不能切换vap state 到WAIT_AUTH_SEQ4 */
        /* WPA3 Rejected Groups element 验证，会重新发送seq_num = 1 的commit auth， 不能切换vap state 到WAIT_AUTH_SEQ4 */
        pst_sta->st_mgmt_timetout_param.en_state = MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2;
    }
    pst_sta->st_mgmt_timetout_param.uc_vap_id = pst_sta->st_vap_base_info.uc_vap_id;
    pst_sta->st_mgmt_timetout_param.us_user_index = pst_hmac_user_ap->st_user_base_info.us_assoc_id;
    frw_timer_create_timer_m(&pst_sta->st_mgmt_timer, hmac_mgmt_timeout_sta, pst_sta->st_mgmt_timer.timeout,
        &pst_sta->st_mgmt_timetout_param, OAL_FALSE, OAM_MODULE_ID_HMAC, pst_sta->st_vap_base_info.core_id);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_sta_process_sae_confirm
 * 功能描述  : 处理收到seq = 2 的SAE 认证帧
 * 1.日    期  : 2019年10月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_sta_process_sae_confirm(hmac_vap_stru *pst_sta, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru *rx_ctl = NULL; /* 每一个MPDU的控制信息 */
    uint8_t *payload = NULL;
    uint16_t payload_len;
    uint16_t auth_seq;
    uint16_t auth_status;

    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf); /* 每一个MPDU的控制信息 */
    if (rx_ctl == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    payload = mac_get_rx_payload(pst_netbuf, &payload_len);
    if (payload == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    auth_seq = mac_get_auth_seq_num(payload, payload_len);
    auth_status = mac_get_auth_status(payload, payload_len);
    /* SAE commit帧的seq number是1，confirm帧的seq number是2 */
    if (auth_seq != WLAN_AUTH_TRASACTION_NUM_TWO) {
        oam_warning_log2(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
            "{hmac_sta_wait_auth_seq4_rx::drop sae auth frame, status_code %d, seq_num %d.}",
            auth_status, auth_seq);
        return OAL_SUCC;
    }

    if (pst_sta->duplicate_auth_seq4_flag == OAL_TRUE) { // wpa3 auth seq4重复帧过滤，禁止上报waps
        oam_warning_log2(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
            "{hmac_sta_wait_auth_seq4_rx::drop sae auth frame, status_code %d, seq_num %d.}",
            auth_status, auth_seq);
        return OAL_SUCC;
    }

    /* 取消定时器 */
    frw_timer_immediate_destroy_timer_m(&pst_sta->st_mgmt_timer);

    /* SAE 判断seq number以后，上传给wpas 处理 */
    hmac_rx_mgmt_send_to_host(pst_sta, pst_netbuf);

    oam_warning_log2(pst_sta->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
        "{hmac_sta_wait_auth_seq4_rx::rx sae auth frame, status_code %d, seq_num %d.}",
        auth_status, auth_seq);
    pst_sta->duplicate_auth_seq4_flag = OAL_TRUE;
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_report_ext_auth_worker
 * 功能描述  : 输入参数  : oal_work_stru *pst_sae_report_ext_auth_worker
 * 1.日    期  : 2019年3月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_report_ext_auth_worker(oal_work_stru *pst_sae_report_ext_auth_worker)
{
    hmac_vap_stru *pst_hmac_vap;

    pst_hmac_vap = oal_container_of((oal_delayed_work *)pst_sae_report_ext_auth_worker,
        hmac_vap_stru, st_sae_report_ext_auth_worker);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_report_ext_auth_worker:: hmac_vap is null}");
        return;
    }

    oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                     "{hmac_report_ext_auth_worker:: hmac_report_external_auth_req auth start}");

    hmac_report_external_auth_req(pst_hmac_vap, NL80211_EXTERNAL_AUTH_START, OAL_FALSE);
}
#ifdef _PRE_WLAN_CHBA_MGMT
static uint32_t hmac_chba_check_external_auth_ssid(mac_vap_stru *mac_vap, hmac_external_auth_req_stru *external_auth)
{
    uint32_t ret;
    uint16_t user_id;
    hmac_user_stru *hmac_user = NULL;
    ret = mac_vap_find_user_by_macaddr(mac_vap, external_auth->auc_bssid, &user_id);
    if (ret != OAL_SUCC) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "hmac_config_external_auth_param_check:can not find user [%02X:XX:XX:XX:%02X:%02X]",
            external_auth->auc_bssid[0],
            external_auth->auc_bssid[4], /* auc_bssid第0、4、5byte为参数输出打印 */
            external_auth->auc_bssid[5]);
        return OAL_FAIL;
    }
    hmac_user = mac_res_get_hmac_user(user_id);
    if (hmac_user == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "hmac_config_external_auth_param_check:invalid user_id %d", user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 非关联的SSID，直接返回 */
    if ((external_auth->st_ssid.uc_ssid_len != hmac_user->chba_user.ssid_len) ||
        oal_memcmp(hmac_user->chba_user.ssid, external_auth->st_ssid.auc_ssid, hmac_user->chba_user.ssid_len) != 0) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "{hmac_config_external_auth_param_check::chba user SSID len [%d], external_auth ssid len [%d]}",
            hmac_user->chba_user.ssid_len, external_auth->st_ssid.uc_ssid_len);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif
static uint32_t hmac_sta_check_external_auth_ssid(mac_vap_stru *mac_vap, hmac_external_auth_req_stru *external_auth)
{
    /* 非关联的bssid， 直接返回 */
    if (oal_memcmp(mac_vap->auc_bssid, external_auth->auc_bssid, WLAN_MAC_ADDR_LEN) != 0) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_SAE,
            "{hmac_config_external_auth_param_check::wrong bssid %02X:XX:XX:XX:%02X:%02X}",
            external_auth->auc_bssid[0],
            external_auth->auc_bssid[4], /* auc_bssid第0、4、5byte为参数输出打印 */
            external_auth->auc_bssid[5]);
        return OAL_FAIL;
    }

    /* 非关联的SSID，直接返回 */
    if ((external_auth->st_ssid.uc_ssid_len != OAL_STRLEN(mac_mib_get_DesiredSSID(mac_vap))) ||
        oal_memcmp(mac_mib_get_DesiredSSID(mac_vap),
            external_auth->st_ssid.auc_ssid, external_auth->st_ssid.uc_ssid_len) != 0) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_SAE,
            "{hmac_config_external_auth_param_check::DesiredSSID len [%d], external_auth ssid len [%d]}",
            OAL_STRLEN(mac_mib_get_DesiredSSID(mac_vap)),
            external_auth->st_ssid.uc_ssid_len);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函数名称: hmac_config_external_auth_param_check
 * 功能描述: 参数检查
 * 1.  日期: 2019年10月21日
 *      作者: wifi
 * 修改内容: 新生成函数
 */
OAL_STATIC uint32_t hmac_config_external_auth_param_check(mac_vap_stru *mac_vap,
    hmac_external_auth_req_stru *ext_auth)
{
    uint32_t ret;
    if (oal_any_null_ptr2(mac_vap, ext_auth)) {
        oam_error_log0(0, OAM_SF_SAE, "{hmac_config_external_auth_param_check::null ptr}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* LEGACY_AP模式，不检查external_auth参数 */
    if (is_legacy_ap(mac_vap) && !mac_is_chba_mode(mac_vap)) {
        return OAL_SUCC;
    }

    /* 非legacy sta、非chba 和 非P2P GC设备，直接返回 */
    /* 6G P2P适配，GC支持WPA3 SAE参数检查 */
    if (!is_legacy_sta(mac_vap) && !is_p2p_cl(mac_vap) && !mac_is_chba_mode(mac_vap)) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_SAE,
                         "{hmac_config_external_auth_param_check::wrong vap. vap_mode %d, p2p_mode %d}",
                         mac_vap->en_vap_mode,
                         mac_vap->en_p2p_mode);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_CHBA_MGMT
    if (mac_vap->chba_mode == CHBA_MODE) {
        ret = hmac_chba_check_external_auth_ssid(mac_vap, ext_auth);
    } else {
#endif
        ret = hmac_sta_check_external_auth_ssid(mac_vap, ext_auth);
#ifdef _PRE_WLAN_CHBA_MGMT
    }
#endif
    return ret;
}
static void hmac_sae_process_external_auth_succ(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_auth_rsp_stru st_auth_rsp = {
        { HMAC_MGMT_SUCCESS, },
    };
    /* ext_auth 状态为成功，执行SAE关联 */
    /* 取消定时器 */
    frw_timer_immediate_destroy_timer_m(&hmac_vap->st_mgmt_timer);
#ifdef _PRE_WLAN_CHBA_MGMT
    if (hmac_vap->st_vap_base_info.chba_mode == CHBA_MODE) {
        /* CHBA 设备WPA3入网，aut成功，修改chba 用户状态为AUTH_COMP */
        hmac_chba_connect_initiator_fsm_change_state(hmac_user, CHBA_USER_STATE_AUTH_COMPLETE);
        hmac_user_set_asoc_state(&(hmac_vap->st_vap_base_info), &hmac_user->st_user_base_info,
            MAC_USER_STATE_AUTH_COMPLETE);

        /* 上报给SME认证成功，执行关联 */
        hmac_chba_handle_auth_rsp(hmac_vap, hmac_user, &st_auth_rsp);
    } else {
#endif
        /* 将状态更改为AUTH_COMP */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_AUTH_COMP);

        /* 上报SME认证成功，执行关联 */
        hmac_send_rsp_to_sme_sta(hmac_vap, HMAC_SME_AUTH_RSP, (uint8_t *)&st_auth_rsp);
#ifdef _PRE_WLAN_CHBA_MGMT
    }
#endif
}

/* 功能描述  : ap 模式处理external auth 事件 */
static void hmac_sae_external_auth_process_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_external_auth_req_stru *ext_auth)
{
    if (ext_auth->us_status != MAC_SUCCESSFUL_STATUSCODE) {
        /* 处理external_auth 失败事件：
         * (1)非anti-clogging错误，需要上报内核去关联事件;
         * (2)anti-clogging异常，需要hostapd保存用户信息，不能上报内核去关联事件 */
        if (ext_auth->us_status != MAC_ANTI_CLOGGING) {
            hmac_handle_disconnect_rsp_ap(hmac_vap, hmac_user);
        }
        /* 删除用户 */
        hmac_user_del(&(hmac_vap->st_vap_base_info), hmac_user);

        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
            "{hmac_sae_external_auth_process_ap::ext_auth->us_status %d, delete user %d}",
            ext_auth->us_status, hmac_user->st_user_base_info.us_assoc_id);
    }
}

static uint32_t hmac_config_external_auth_get_res(mac_vap_stru *mac_vap, hmac_external_auth_req_stru *ext_auth,
    hmac_vap_stru **hmac_vap, hmac_user_stru **hmac_user)
{
    uint16_t user_idx;
    uint32_t ret;

    *hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (*hmac_vap == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SAE, "{hmac_config_external_auth::hmac_vap null, vap_idx:%d}",
            mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 不论ap/sta/chba，上层下发对端mac 地址。根据用户mac地址，查找用户 */
    ret = mac_vap_find_user_by_macaddr(mac_vap, ext_auth->auc_bssid, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_SAE,
            "{hmac_config_external_auth::mac_vap_find_user_by_macaddr failed[%d] bssid[%02X:xx:xx:xx:%02X:%02X]}",
            ret, ext_auth->auc_bssid[MAC_ADDR_0], ext_auth->auc_bssid[MAC_ADDR_4], ext_auth->auc_bssid[MAC_ADDR_5]);
        return OAL_FAIL;
    }
    *hmac_user = mac_res_get_hmac_user(user_idx);
    if (*hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SAE,
            "hmac_config_external_auth::pst_hmac_user[%d] NULL.", user_idx);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_external_auth
 * 功能描述  : 执行SAE external auth命令
 * 1.日    期  : 2019年1月2日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_config_external_auth(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    hmac_external_auth_req_stru *ext_auth;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *pst_hmac_user = NULL;

    ext_auth = (hmac_external_auth_req_stru *)puc_param;
    ret = hmac_config_external_auth_param_check(mac_vap, ext_auth);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = hmac_config_external_auth_get_res(mac_vap, ext_auth, &hmac_vap, &pst_hmac_user);
    if (ret != OAL_SUCC) {
        return ret;
    }

    if (is_legacy_ap(mac_vap) && !mac_is_chba_mode(mac_vap)) {
        /* LEGACY_AP模式，处理external_auth */
        hmac_sae_external_auth_process_ap(hmac_vap, pst_hmac_user, ext_auth);
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_CHBA_MGMT
    if ((mac_is_chba_mode(mac_vap) == OAL_TRUE) &&
        (pst_hmac_user->chba_user.connect_info.connect_role != CHBA_CONN_INITIATOR)) {
        /*  chba建链响应方不需要处理external auth事件。 */
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SAE, "hmac_config_external_auth::chba role[%d] return.",
            pst_hmac_user->chba_user.connect_info.connect_role);
        return OAL_SUCC;
    }
#endif
    /* chba 建链发起方和sta 处理external auth事件 */
    /* ext_auth 状态为失败情况下，断开连接 */
    if (ext_auth->us_status == MAC_UNSPEC_FAIL) {
        /*  SAE Wrong password processing, UI need popup wrong password dialog,
         * FWK need assoc reject event, and supplicant need connect event with status!=0 */
        hmac_handle_connect_failed_result(hmac_vap, ext_auth->us_status,
            pst_hmac_user->st_user_base_info.auc_user_mac_addr);
        return OAL_SUCC;
    } else if (ext_auth->us_status != MAC_SUCCESSFUL_STATUSCODE) {
        mac_cfg_kick_user_param_stru st_kick_user_param;
        st_kick_user_param.us_reason_code = ext_auth->us_status;
        st_kick_user_param.send_disassoc_immediately = OAL_TRUE;
        memcpy_s(st_kick_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN, pst_hmac_user->st_user_base_info.auc_user_mac_addr,
            WLAN_MAC_ADDR_LEN);
        hmac_config_kick_user(mac_vap, sizeof(st_kick_user_param), (uint8_t *)(&st_kick_user_param));
        return OAL_SUCC;
    }

    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        hmac_roam_sae_config_reassoc_req(hmac_vap);
        return OAL_SUCC;
    }

    /* ext_auth 成功处理 */
    hmac_sae_process_external_auth_succ(hmac_vap, pst_hmac_user);
    return OAL_SUCC;
}

/*
 * 函 数 名  : is_sae_connect_with_PMKID
 * 功能描述  : 判断wpa_s下发关联rsn ie 是否包含PMKID.
 * 1.日    期  : 2019年1月24日
  *   修改内容  : 新生成函数
 */
OAL_STATIC oal_bool_enum_uint8 is_sae_connect_with_pmkid(uint8_t *puc_rsn_ie, uint8_t uc_rsn_ie_len)
{
    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Suite  */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |      0 or 4        |     0 or 2      */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /*  Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*       |       0 or 4*m             |     0 or 2      |   0 or 4*n     */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |      0 or 2    |   0 or 2  | 0 or 16 *s  |          0 or 4        | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /* 对于wpa_s下发关联的rsn_ie包含组播加密套件、单播加密套件、秘钥管理套件、RSN capability。
     * 可以不用考虑没有以上套件的场景
     */
    struct hmac_rsn_connect_pmkid_stru {
        uint8_t uc_eid;
        uint8_t uc_ie_len;
        uint16_t us_rsn_ver;
        uint32_t group_cipher;
        uint16_t us_pairwise_cipher_cnt;
        uint32_t pairwise_cipher;
        uint16_t us_akm_cnt;
        uint32_t akm;
        uint16_t us_rsn_cap;
        uint16_t us_pmkid_cnt;
        uint8_t auc_pmkid[WLAN_PMKID_LEN];
    };

    /* RSN长度过短，不包含PMKID */
    if (uc_rsn_ie_len < sizeof(struct hmac_rsn_connect_pmkid_stru)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * 函 数 名  : hmac_update_sae_connect_param
 * 功能描述  : 关联时先判断是否为SAE关联
 * 1.日    期  : 2019年1月24日
  *   修改内容  : 新生成函数
 */
void hmac_update_sae_connect_param(hmac_vap_stru *pst_hmac_vap, uint8_t *puc_ie, uint32_t ie_len)
{
    uint8_t *puc_rsn_ie = NULL;
    mac_crypto_settings_stru st_crypto = {0};
    oal_bool_enum_uint8 en_sae_connect_with_pmkid;

    pst_hmac_vap->bit_sae_connect_with_pmkid = OAL_FALSE;
    pst_hmac_vap->en_sae_connect = OAL_FALSE;

    if (puc_ie == NULL) {
        return;
    }

    /* wpa_s下发SAE关联必然携带RSN IE */
    puc_rsn_ie = mac_find_ie(MAC_EID_RSN, puc_ie, ie_len);
    if (puc_rsn_ie == NULL) {
        return;
    }

    mac_ie_get_rsn_cipher(puc_rsn_ie, &st_crypto);
    if (st_crypto.aul_akm_suite[0] != MAC_RSN_AKM_SAE && st_crypto.aul_akm_suite[0] != MAC_RSN_AKM_TBPEKE) {
        oam_warning_log1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
            "aul_akm_suite=0x%x", st_crypto.aul_akm_suite[0]);
        return ;
    }

    /* 单独的sae_connect 只表明第一次入网是WPA3（cfg80211_connect 接口） */
    /* en_sae_connect需要在roam 的时候也更新 */
    pst_hmac_vap->en_sae_connect = OAL_TRUE;

    en_sae_connect_with_pmkid = is_sae_connect_with_pmkid(puc_rsn_ie, puc_rsn_ie[1]);
    if ((mac_mib_get_AuthenticationMode(&(pst_hmac_vap->st_vap_base_info)) == WLAN_WITP_AUTH_SAE ||
        mac_mib_get_AuthenticationMode(&(pst_hmac_vap->st_vap_base_info)) == WLAN_WITP_AUTH_TBPEKE) &&
        (en_sae_connect_with_pmkid == OAL_TRUE)) {
        oam_error_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                       "hmac_update_sae_connect_param:: auth_type=SAE with PMKID=1");
        return ;
    }

    /* 在RSN IE中查找携带PMKID内容，则设置sae_connect_with_pmkid = true;
     * 未携带PMKID内容，设置sae_connect_with_pmkid = false
     */
    pst_hmac_vap->bit_sae_connect_with_pmkid = en_sae_connect_with_pmkid;

    if (st_crypto.aul_akm_suite[0] == MAC_RSN_AKM_SAE) {
        oam_warning_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                         "hmac_update_sae_connect_param::use sae connect[%d]. with PMKID [%d]",
                         pst_hmac_vap->en_sae_connect, pst_hmac_vap->bit_sae_connect_with_pmkid);
    } else if (st_crypto.aul_akm_suite[0] == MAC_RSN_AKM_TBPEKE) {
        oam_warning_log2(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SAE,
                         "hmac_update_sae_connect_param::use tbpeke connect[%d]. with PMKID [%d]",
                         pst_hmac_vap->en_sae_connect, pst_hmac_vap->bit_sae_connect_with_pmkid);
    }
}

/*
 * 函数名:hmac_vap_set_sae_pwe
 * 功能:设置SAE_PWE 参数
 */
uint32_t hmac_vap_set_sae_pwe(mac_vap_stru *mac_vap, uint32_t *params)
{
    mac_vap->sae_pwe = *params;
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_vap_set_sae_pwe:: sae_pwe [%d]}",
        mac_vap->sae_pwe);

    return OAL_SUCC;
}

/*
 * 函数名:hmac_config_set_sae_pwe
 * 功能:设置SAE_PWE 参数
 */
uint32_t hmac_config_set_sae_pwe(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    hmac_vap_set_sae_pwe(mac_vap, (uint32_t *)param);

    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_SAE_PWE, len, param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_set_sae_pwe::hmac_config_send_event failed[%d].}", ret);
    }
    return ret;
}

#endif
