/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux与驱动的接口函数，封装消息，抛事件给驱动；接收从驱动抛上来的事件，解析消息
 * 作    者 : wifi
 * 创建日期 : 2013年8月26日
 */

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "wal_linux_ioctl.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_scan.h"
#include "wal_linux_event.h"
#include "hmac_resource.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_EVENT_C

typedef oal_bool_enum_uint8 (*wal_wait_condition_complete_func)(uint8_t *condition);

/*
 * 功能秒速:等待事件完成
 * timeout:单位 ms
 */
void wal_wait_event_completed(oal_net_device_stru *net_device,
    wal_wait_condition_complete_func wal_wait_condition_complete_cb, uint8_t *condition,
    uint32_t timeout)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    if (net_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wait_event_completed:: net_device is NULL!}");
        return;
    }

    mac_vap = oal_net_dev_priv(net_device);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wait_event_completed:: mac_vap is NULL!}");
        return;
    }

    /* 获取hmac vap */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_SCAN,
            "{wal_wait_event_completed::hmac_vap is null, vap_id[%d]!}",
            mac_vap->uc_vap_id);
        return;
    }

    if (wal_wait_condition_complete_cb(condition) == OAL_FALSE) {
        signed long lefttime;
        uint32_t start_time = (uint32_t)oal_time_get_stamp_ms();

        /* 关联前如果在扫描，等待扫描完成后执行关联。如超时后还未扫描完成，则直接执行关联 */
        lefttime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
            wal_wait_condition_complete_cb(condition) == OAL_TRUE,
            timeout * OAL_TIME_HZ / HMAC_S_TO_MS);
        if (lefttime == 0) {
            /* 超时还没有上报扫描结束 */
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                "{wal_wait_event_completed:: wait for %u ms timeout!}", timeout);
        } else if (lefttime < 0) {
            /* 定时器内部错误 */
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                "{wal_wait_event_completed:: wait for %u ms error!}", timeout);
        } else {
            /* 正常结束  */
            uint32_t curr_time = (uint32_t)oal_time_get_stamp_ms();
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                "wal_wait_event_completed::wait event cost_time[%d]",
                oal_time_get_runtime(start_time, curr_time));
        }
    } else {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_wait_event_completed:: not need wait! %u}", timeout);
    }
}

/*
 * 函 数 名  : wal_cfg80211_start_req
 * 功能描述  : 向wal抛事件
 * 1.日    期  : 2014年1月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_start_req(oal_net_device_stru *pst_net_dev, const void *ps_param,
    uint16_t us_len, wlan_cfgid_enum_uint16 en_wid, oal_bool_enum_uint8 en_need_rsp)
{
    wal_msg_write_stru st_write_msg;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint32_t err_code;
    int32_t l_ret;

    if (ps_param == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{wal_cfg80211_start_req::param is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写 msg 消息头 */
    st_write_msg.en_wid = en_wid;
    st_write_msg.us_len = us_len;

    /* 填写 msg 消息体 */
    if (us_len > WAL_MSG_WRITE_MAX_LEN) {
        oam_error_log2(0, OAM_SF_SCAN,
                       "{wal_cfg80211_start_req::us_len %d > WAL_MSG_WRITE_MAX_LEN %d err!}\r\n",
                       us_len, WAL_MSG_WRITE_MAX_LEN);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), ps_param, us_len)) {
        oam_error_log0(0, OAM_SF_SCAN, "wal_cfg80211_start_req::memcpy fail!");
        return -OAL_EFAIL;
    }

    /***************************************************************************
           抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_send_cfg_event(pst_net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
        (uint8_t *)&st_write_msg,
        en_need_rsp,
        en_need_rsp ? &pst_rsp_msg : NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SCAN,
                         "{wal_cfg80211_start_req::wal_send_cfg_event return err code %d!}\r\n", l_ret);
        return l_ret;
    }

    if (en_need_rsp && (pst_rsp_msg != NULL)) {
        /* 读取返回的错误码 */
        err_code = wal_check_and_release_msg_resp(pst_rsp_msg);
        if (err_code != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_SCAN,
                             "{wal_cfg80211_start_req::wal_send_cfg_event return err code:[%u]}",
                             err_code);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_cfg80211_start_scan
 * 1.日    期  : 2013年8月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_cfg80211_start_scan(oal_net_device_stru *pst_net_dev, mac_cfg80211_scan_param_stru *pst_scan_param)
{
    if (pst_scan_param == NULL) {
        oam_error_log0(0, OAM_SF_SCAN,
                       "{wal_cfg80211_start_scan::scan failed, null ptr, pst_scan_param = null.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*  异步事件抛结构体，不需要考虑上下层释放内存问题 */
    return (uint32_t)wal_cfg80211_start_req(pst_net_dev, pst_scan_param,
                                            sizeof(mac_cfg80211_scan_param_stru),
                                            WLAN_CFGID_CFG80211_START_SCAN, OAL_FALSE);
}

/*
 * 函 数 名  : wal_pno_scan_with_assigned_ssid
 * 功能描述  : 检查pst_ssid是否在pst_ssid_list列表里面
 * 1.日    期  : 2016年8月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC oal_bool_enum_uint8 wal_pno_scan_with_assigned_ssid(oal_cfg80211_ssid_stru *pst_ssid,
                                                               oal_cfg80211_ssid_stru *pst_ssid_list,
                                                               int32_t l_count)
{
    int32_t l_loop;

    if (oal_any_null_ptr2(pst_ssid, pst_ssid_list)) {
        return OAL_FALSE;
    }

    for (l_loop = 0; l_loop < l_count; l_loop++) {
        if ((pst_ssid->ssid_len == pst_ssid_list[l_loop].ssid_len) &&
            (oal_memcmp(pst_ssid->ssid, pst_ssid_list[l_loop].ssid, pst_ssid->ssid_len) == 0)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}
/*
 * 函 数 名  : wal_cfg80211_sched_scan_get_scan_interval
 * 功能描述  : PNO扫描间隔时间获取
 * 1.日    期  : 2023年1月30日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_cfg80211_sched_scan_get_scan_interval(oal_cfg80211_sched_scan_request_stru *request)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    /* 上层下发interval合法性检查 */
    if (request->scan_plans == NULL) {
        return PNO_SCHED_SCAN_INTERVAL_LEVEL2;
    }
    /* 必须是约定好的interval否则继续采用最大值 */
    if (request->scan_plans[0].interval == PNO_SCHED_SCAN_INTERVAL_LEVEL1 ||
        request->scan_plans[0].interval == PNO_SCHED_SCAN_INTERVAL_LEVEL2) {
        return request->scan_plans[0].interval;
    }
    return PNO_SCHED_SCAN_INTERVAL_LEVEL2;
#else
    return PNO_SCHED_SCAN_INTERVAL_LEVEL2;
#endif
}

OAL_STATIC void wal_cfg80211_sched_scan_start_pno_init(oal_cfg80211_sched_scan_request_stru *pst_request,
    mac_pno_scan_stru *pno_scan_info)
{
    oal_cfg80211_ssid_stru *pst_ssid_tmp = NULL;
    oal_cfg80211_ssid_stru *pst_scan_ssid_list = NULL;
    int32_t l_loop;
    int32_t l_ret;
    /* 初始化pno扫描的结构体信息 */
    memset_s(pno_scan_info, sizeof(mac_pno_scan_stru), 0, sizeof(mac_pno_scan_stru));

    /* 将内核下发的匹配的ssid集合复制到本地 */
    if (pst_request->n_ssids > 0) {
        pst_scan_ssid_list = pst_request->ssids;
    }
    for (l_loop = 0; l_loop < oal_min(pst_request->n_match_sets, MAX_PNO_SSID_COUNT); l_loop++) {
        pst_ssid_tmp = &(pst_request->match_sets[l_loop].ssid);
        if (pst_ssid_tmp->ssid_len >= WLAN_SSID_MAX_LEN) {
            oam_warning_log1(0, 0, "{wal_cfg80211_sched_scan_start:: wrong ssid_len[%d]!}", pst_ssid_tmp->ssid_len);
            continue;
        }
        l_ret = memcpy_s(pno_scan_info->ast_match_ssid_set[l_loop].auc_ssid, WLAN_SSID_MAX_LEN,
                         pst_ssid_tmp->ssid, pst_ssid_tmp->ssid_len);
        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "wal_cfg80211_sched_scan_start::memcpy fail!");
        }

        pno_scan_info->ast_match_ssid_set[l_loop].auc_ssid[pst_ssid_tmp->ssid_len] = '\0';
        pno_scan_info->ast_match_ssid_set[l_loop].en_scan_ssid =
            wal_pno_scan_with_assigned_ssid(pst_ssid_tmp, pst_scan_ssid_list, pst_request->n_ssids);
        pno_scan_info->l_ssid_count++;
    }

    /* 其它参数赋值 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pno_scan_info->l_rssi_thold = pst_request->min_rssi_thold;
#else
    pno_scan_info->l_rssi_thold = pst_request->rssi_thold;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    pno_scan_info->pno_scan_interval = wal_cfg80211_sched_scan_get_scan_interval(pst_request) * HMAC_S_TO_MS;
    pno_scan_info->uc_pno_scan_repeat = MAX_PNO_REPEAT_TIMES;
}

/*
 * 函 数 名  : wal_cfg80211_start_sched_scan
 * 1.日    期  : 2015年6月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_cfg80211_start_sched_scan(oal_net_device_stru *pst_net_dev,
    oal_cfg80211_sched_scan_request_stru *pst_request)
{
    mac_pno_scan_stru *pst_pno_scan_params;
    uint32_t ret;
    /* 申请pno调度扫描参数，此处申请hmac层释放 */
    pst_pno_scan_params = (mac_pno_scan_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
                                                               sizeof(mac_pno_scan_stru),
                                                               OAL_FALSE);
    if (pst_pno_scan_params == NULL) {
        oam_warning_log0(0, OAM_SF_SCAN,
                         "{wal_cfg80211_start_sched_scan::alloc pno scan param memory failed!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(pst_pno_scan_params, sizeof(mac_pno_scan_stru), 0, sizeof(mac_pno_scan_stru));
    wal_cfg80211_sched_scan_start_pno_init(pst_request, pst_pno_scan_params);

    ret = (uint32_t)wal_cfg80211_start_req(pst_net_dev, &pst_pno_scan_params,
                                           sizeof(mac_pno_scan_stru *),
                                           WLAN_CFGID_CFG80211_START_SCHED_SCAN, OAL_TRUE);

    /* 无论wal_cfg80211_start_req内部执行成功与否，统一在外部释放 */
    oal_mem_free_m(pst_pno_scan_params, OAL_FALSE);

    return ret;
}

/*
 * 函 数 名  : wal_cfg80211_start_connect
 * 1.日    期  : 2013年8月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_cfg80211_start_connect(oal_net_device_stru *pst_net_dev, hmac_conn_param_stru *pst_mac_conn_param)
{
    return wal_cfg80211_start_req(pst_net_dev,
                                  pst_mac_conn_param,
                                  sizeof(hmac_conn_param_stru),
                                  WLAN_CFGID_CFG80211_START_CONNECT,
                                  OAL_TRUE);
}

oal_bool_enum_uint8 wal_wait_disassoc_send_complete(uint8_t *disassoc_tx_completed_flag)
{
    return (*(oal_bool_enum_uint8 *)disassoc_tx_completed_flag == OAL_TRUE);
}

/*
 * 功能描述  : 断开连接，下发kick_user命令
 */
#define WAL_CFG80211_DISCONNECT_WAIT_DISASSOC_COMP_TIMEOUT 100 /* 去关联等待disassoc发送完成超时时间100 ms */
int32_t wal_cfg80211_start_disconnect(oal_net_device_stru *pst_net_dev,
                                      mac_cfg_kick_user_param_stru *pst_disconnect_param)
{
    mac_vap_stru *pst_mac_vap;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_scan_record_stru *scan_record = NULL;

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    // 终止可能存在的扫描
    if (pst_mac_vap != NULL && (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
        if (hmac_device == NULL) {
            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                             "{wal_cfg80211_start_disconnect::hmac_device[%d] is null!}",
                             pst_mac_vap->uc_device_id);
            return -OAL_EINVAL;
        }
        /* abort漫游扫描，需要先把扫描回调函数置成null。否则可能先执行漫游入网，后删user，此时找不到user */
        scan_record = &hmac_device->st_scan_mgmt.st_scan_record_mgmt;
        if (scan_record->en_scan_mode == WLAN_SCAN_MODE_ROAM_SCAN) {
            scan_record->p_fn_cb = NULL;
        }
        /* 如果是来自内部的扫描, 且当前为listen状态则把扫描回调函数置成null */
        if (hmac_device->st_scan_mgmt.pst_request == NULL) {
            /* 判断是否存在内部扫描，如果存在，也需要停止 */
            if ((hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) &&
                (pst_mac_vap->uc_vap_id == scan_record->uc_vap_id) &&
                (pst_mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN)) {
                scan_record->p_fn_cb = NULL;
            }
        }
        wal_force_scan_abort_then_scan_comp(pst_net_dev);
    }

    if (pst_mac_vap && pst_disconnect_param->send_disassoc_immediately == OAL_FALSE) {
        /* wal层kick_user，先下发disassoc事件到hmac，等待disassoc发送完成后，再下发kick_user事件到hmac */
        wal_cfg80211_start_req(pst_net_dev, pst_disconnect_param, sizeof(mac_cfg_kick_user_param_stru),
            WLAN_CFGID_SEND_DISASSOC, OAL_TRUE);
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
        if (hmac_vap) {
            hmac_vap->disassoc_tx_completed_flag = OAL_FALSE;
            wal_wait_event_completed(pst_net_dev,
                wal_wait_disassoc_send_complete, &hmac_vap->disassoc_tx_completed_flag,
                WAL_CFG80211_DISCONNECT_WAIT_DISASSOC_COMP_TIMEOUT);
        }
    }

    /* 注意 由于消息未真正处理就直接返回，导致WPA_SUPPLICANT继续下发消息，在驱动侧等到处理时被异常唤醒，导致后续下发的消息误以为操作失败，
       目前将去关联事件修改为等待消息处理结束后再上报，最后一个入参由OAL_FALSE改为OAL_TRUE */
    return wal_cfg80211_start_req(pst_net_dev, pst_disconnect_param,
                                  sizeof(mac_cfg_kick_user_param_stru), WLAN_CFGID_KICK_USER, OAL_TRUE);
}

#ifdef _PRE_WLAN_FEATURE_SAE
/*
 * 函 数 名  : wal_cfg80211_do_external_auth
 * 功能描述  : 执行内核下发 ext_auth 命令到 hmac
 * 1.日    期  : 2019年11月2日
  *   修改内容  : 新生成函数
 */
uint32_t wal_cfg80211_do_external_auth(oal_net_device_stru *pst_netdev, hmac_external_auth_req_stru *pst_ext_auth)
{
    return (uint32_t)wal_cfg80211_start_req(pst_netdev, pst_ext_auth,
                                            sizeof(*pst_ext_auth),
                                            WLAN_CFGID_CFG80211_EXTERNAL_AUTH, OAL_TRUE);
}
#endif /* _PRE_WLAN_FEATURE_SAE */
