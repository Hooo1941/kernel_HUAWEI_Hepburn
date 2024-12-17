/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 接收驱动上报过来的消息，上报给内核
 * 作    者 : wifi
 * 创建日期 : 2013年8月26日
 */

#include "oal_ext_if.h"
#include "wlan_types.h"
#include "frw_ext_if.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_vap.h"
#include "mac_regdomain.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_scan.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_flowctl.h"
#include "wal_linux_cfgvendor.h"
#include "wal_linux_cfgvendor_attributes.h"
#include "oal_cfg80211.h"
#include "oal_net.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_user.h"
#include "hmac_chan_mgmt.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#include "securec.h"
#include "hmac_dfx.h"
#ifdef _PRE_WLAN_FEATURE_PWL
#include "hmac_roam_pwl.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_RX_RSP_C

/*
 * 功能描述  : 上报扫描完成事件处理
 * 1.日    期  : 2016年7月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_scan_report(hmac_scan_stru *pst_scan_mgmt, oal_bool_enum en_is_aborted)
{
    /* 通知 kernel scan 已经结束 */
    oal_cfg80211_scan_done(pst_scan_mgmt->pst_request, en_is_aborted);

    pst_scan_mgmt->pst_request = NULL;
    pst_scan_mgmt->en_complete = OAL_TRUE;

    oam_warning_log1(0, OAM_SF_SCAN, "{wal_scan_report::scan complete.abort flag[%d]!}", en_is_aborted);

    /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
    oal_smp_mb();
    oal_wait_queue_wake_up_interrupt(&pst_scan_mgmt->st_wait_queue);
}

/*
 * 功能描述  : 上报PNO扫描完成事件处理
 * 1.日    期  : 2016年7月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_schedule_scan_report(oal_wiphy_stru *pst_wiphy, hmac_scan_stru *pst_scan_mgmt)
{
    /* 上报调度扫描结果 */
    oal_cfg80211_sched_scan_result(pst_wiphy);

    pst_scan_mgmt->pst_sched_scan_req = NULL;
    pst_scan_mgmt->en_sched_scan_complete = OAL_TRUE;

    oam_warning_log0(0, OAM_SF_SCAN, "{wal_schedule_scan_report::sched scan complete.!}");
}

/*
 * 功能描述  : 非pno扫描完成事件处理
 * 1.日    期  : 2019年9月18日
 *   作    者  : wifi
 */
OAL_STATIC void wal_normal_scan_comp_handle(hmac_scan_stru *pst_scan_mgmt,
    hmac_vap_stru *pst_hmac_vap, hmac_scan_rsp_stru *pst_scan_rsp)
{
    oal_bool_enum en_is_aborted;

    /* 普通扫描结束事件 */
    if (pst_scan_mgmt->pst_request != NULL) {
        /* Begin：, 解决P2P listen被wlan扫描终止时，上报的bss个数较少的问题 */
        if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
            if (!pst_hmac_vap->bit_in_p2p_listen) {
                en_is_aborted = (pst_scan_rsp->uc_result_code == MAC_SCAN_ABORT) ? OAL_TRUE : OAL_FALSE;
                wal_scan_report(pst_scan_mgmt, en_is_aborted);
            }
        } else {
            /* 是scan abort的话告知内核 */
            en_is_aborted = (pst_scan_rsp->uc_result_code == MAC_SCAN_ABORT) ? OAL_TRUE : OAL_FALSE;
            wal_scan_report(pst_scan_mgmt, en_is_aborted);
        }
    } else {
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::scan already complete!}");
    }
}
/*
 * 功能描述  : 上层下发的普通扫描进行对应处理
 * 1.日    期  : 2019年12月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
*/
OAL_STATIC void wal_scan_upper_issued_proc(hmac_scan_rsp_stru *pst_scan_rsp, hmac_scan_stru *pst_scan_mgmt,
                                           oal_wiphy_stru *pst_wiphy, hmac_vap_stru *pst_hmac_vap)
{
    /* 上层下发的普通扫描进行对应处理 */
    if (pst_scan_rsp->uc_result_code == MAC_SCAN_PNO) {
        /* PNO扫描结束事件 */
        if (pst_scan_mgmt->pst_sched_scan_req != NULL) {
            /*
             *  存在PNO扫描结束上报但驱动PNO扫描已经结束的情况,
             * 即普通scan打断PNO scan,此时device正巧上报PNO扫描结束事件
             */
            wal_schedule_scan_report(pst_wiphy, pst_scan_mgmt);
        } else {
            oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::sched scan already complete!}");
        }
    } else {
        /* 普通扫描结束事件 */
        wal_normal_scan_comp_handle(pst_scan_mgmt, pst_hmac_vap, pst_scan_rsp);
    }
}

void wal_scan_comp_report_all_bss(frw_event_stru *event, hmac_vap_stru *hmac_vap, hmac_bss_mgmt_stru *bss_mgmt,
    oal_wiphy_stru *wiphy)
{
    /* Begin：, 解决P2P listen被wlan扫描终止时，上报的bss个数较少的问题 */
    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        if (!hmac_vap->bit_in_p2p_listen) {
            /* P2P listen在强制终止时会调用到本函数，此时不需要上报扫描结果 */
            oam_warning_log1(event->st_event_hdr.uc_vap_id, OAM_SF_SCAN,
                "{wal_scan_comp_proc_sta::P2P listen NOT inform all bss, bit_in_p2p_listen[%d].}",
                hmac_vap->bit_in_p2p_listen);
            wal_inform_all_bss(wiphy, bss_mgmt, event->st_event_hdr.uc_vap_id);
        }
    } else {
        /* 上报所有扫描到的bss, 无论扫描结果成功与否，统一上报扫描结果，有几个上报几个 */
        wal_inform_all_bss(wiphy, bss_mgmt, event->st_event_hdr.uc_vap_id);
    }
    /* End： */
}

/*
 * 功能描述  : STA上报扫描完成事件处理
 * 1.日    期  : 2013年7月5日
  *   修改内容  : 新生成函数
 * 2.日    期  : 2013年9月9日
 *   作    者  : wifi
 *   修改内容  : 增加上报内核部分函数
 */
uint32_t wal_scan_comp_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    hmac_scan_rsp_stru *pst_scan_rsp = NULL;
    hmac_device_stru *pst_hmac_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    hmac_bss_mgmt_stru *pst_bss_mgmt = NULL;
    hmac_scan_stru *pst_scan_mgmt = NULL;
    oal_wiphy_stru *pst_wiphy = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取hmac vap结构体 */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    /* 获取hmac device 指针 */
    pst_hmac_device = hmac_res_get_mac_dev(pst_event->st_event_hdr.uc_device_id);
    if (oal_any_null_ptr2(pst_hmac_vap, pst_hmac_device)) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, 0, "{wal_scan_comp_proc_sta::hmac_vap or hmac_dev null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 删除等待扫描超时定时器 */
    if (pst_hmac_vap->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_scan_timeout));
    }

    pst_scan_mgmt = &(pst_hmac_device->st_scan_mgmt);
    pst_wiphy = pst_hmac_device->pst_device_base_info->pst_wiphy;

    /* 获取扫描结果的管理结构地址 */
    pst_bss_mgmt = &(pst_hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 获取驱动上报的扫描结果结构体指针 */
    pst_scan_rsp = (hmac_scan_rsp_stru *)pst_event->auc_event_data;

    /* 如果扫描返回结果的非成功，打印维测信息 */
    if ((pst_scan_rsp->uc_result_code != MAC_SCAN_SUCCESS) && (pst_scan_rsp->uc_result_code != MAC_SCAN_PNO)) {
        oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SCAN,
            "{wal_scan_comp_proc_sta::scan not succ, err_code[%d]!}", pst_scan_rsp->uc_result_code);
    }

    wal_scan_comp_report_all_bss(pst_event, pst_hmac_vap, pst_bss_mgmt, pst_wiphy);

    /* 对于内核下发的扫描request资源加锁 */
    oal_spin_lock(&(pst_scan_mgmt->st_scan_request_spinlock));

    /* 没有未释放的扫描资源，直接返回 */
    if ((pst_scan_mgmt->pst_request == NULL) && (pst_scan_mgmt->pst_sched_scan_req == NULL)) {
        /* 通知完内核，释放资源后解锁 */
        oal_spin_unlock(&(pst_scan_mgmt->st_scan_request_spinlock));
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::legacy scan and pno scan are complete!}");

        return OAL_SUCC;
    }
    if (!oal_any_null_ptr2(pst_scan_mgmt->pst_request, pst_scan_mgmt->pst_sched_scan_req)) {
        /* 一般情况下,2个扫描同时存在是一种异常情况,在此添加打印,暂不做异常处理 */
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta::legacy scan and pno scan are all started!!!}");
    }

    /* 上层下发的普通扫描进行对应处理 */
    wal_scan_upper_issued_proc(pst_scan_rsp, pst_scan_mgmt, pst_wiphy, pst_hmac_vap);

    /* 通知完内核，释放资源后解锁 */
    oal_spin_unlock(&(pst_scan_mgmt->st_scan_request_spinlock));
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    /* 扫描完成国家码更新 */
    wal_counrtycode_selfstudy_scan_comp(pst_hmac_vap);
#endif
    return OAL_SUCC;
}

static void wal_free_asoc_comp_proc_sta_ie_buf(hmac_asoc_rsp_stru *pst_asoc_rsp)
{
    if (pst_asoc_rsp->puc_asoc_rsp_ie_buff != NULL) {
        oal_free(pst_asoc_rsp->puc_asoc_rsp_ie_buff);
        pst_asoc_rsp->puc_asoc_rsp_ie_buff = NULL;
    }
    if (pst_asoc_rsp->puc_asoc_req_ie_buff != NULL) {
        oal_free(pst_asoc_rsp->puc_asoc_req_ie_buff);
        pst_asoc_rsp->puc_asoc_req_ie_buff = NULL;
    }
}

uint32_t wal_asoc_comp_update_bss_info(frw_event_stru *event, oal_connet_result_stru *connet_result,
    hmac_asoc_rsp_stru *asoc_rsp)
{
    hmac_device_stru *hmac_device = NULL;
    oal_wiphy_stru *wiphy = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;

    /* begin: 关联成功，上报关联结果前，先更新对应bss 信息，
     * 避免由于关联前不扫描，内核将bss 老化，4次握手完成后下发秘钥提示失败情况 */
    if (connet_result->us_status_code != MAC_SUCCESSFUL_STATUSCODE) {
        return OAL_SUCC;
    }
    hmac_device = hmac_res_get_mac_dev(event->st_event_hdr.uc_device_id);
    if (oal_any_null_ptr3(hmac_device, hmac_device->pst_device_base_info,
                          hmac_device->pst_device_base_info->pst_wiphy)) {
        oam_error_log0(event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_asoc_comp_proc_sta::get ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    wiphy = hmac_device->pst_device_base_info->pst_wiphy;
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    wal_update_bss(wiphy, bss_mgmt, connet_result->auc_bssid);

    return OAL_SUCC;
}

void wal_asoc_update_connect_result(oal_connet_result_stru *connet_result, hmac_asoc_rsp_stru *asoc_rsp)
{
    connet_result->puc_req_ie = asoc_rsp->puc_asoc_req_ie_buff;
    connet_result->req_ie_len = asoc_rsp->asoc_req_ie_len;
    connet_result->puc_rsp_ie = asoc_rsp->puc_asoc_rsp_ie_buff;
    connet_result->rsp_ie_len = asoc_rsp->asoc_rsp_ie_len;
    connet_result->us_status_code = asoc_rsp->en_status_code;
}

void wal_asoc_comp_sta_connect_report(frw_event_stru *event, oal_net_device_stru *net_device,
    oal_connet_result_stru *connet_result)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34))
    uint32_t ret;
#endif
    /* end: 关联成功，上报关联结果前，先更新对应bss 信息，
    * 避免由于关联前不扫描，内核将bss 老化，4次握手完成后下发秘钥提示失败情况 */
    /* 调用内核接口，上报关联结果 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    oal_cfg80211_connect_result(net_device, connet_result->auc_bssid, connet_result->puc_req_ie,
        connet_result->req_ie_len, connet_result->puc_rsp_ie, connet_result->rsp_ie_len,
        connet_result->us_status_code, GFP_ATOMIC);
#else
    ret = oal_cfg80211_connect_result(net_device, connet_result->auc_bssid, connet_result->puc_req_ie,
        connet_result->req_ie_len, sconnet_result->puc_rsp_ie, connet_result->rsp_ie_len,
        connet_result->us_status_code, GFP_ATOMIC);
    if (ret != OAL_SUCC) {
        oam_warning_log1(event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
            "{wal_asoc_comp_proc_sta::oal_cfg80211_connect_result fail[%d]!}", ret);
    }
#endif
}

/*
 * 功能描述  : STA上报关联完成事件处理
 * 1.日    期  : 2013年7月5日
  *   修改内容  : 新生成函数
 */
uint32_t wal_asoc_comp_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_connet_result_stru st_connet_result;
    oal_net_device_stru *net_device = NULL;
    hmac_asoc_rsp_stru *pst_asoc_rsp = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_asoc_comp_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_asoc_rsp = (hmac_asoc_rsp_stru *)pst_event->auc_event_data;

    /* 获取net_device */
    net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_asoc_comp_proc_sta::get net device ptr is null!}");
        wal_free_asoc_comp_proc_sta_ie_buf(pst_asoc_rsp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&st_connet_result, sizeof(oal_connet_result_stru), 0, sizeof(oal_connet_result_stru));

    /* 准备上报内核的关联结果结构体 */
    memcpy_s(st_connet_result.auc_bssid, WLAN_MAC_ADDR_LEN, pst_asoc_rsp->auc_addr_ap, WLAN_MAC_ADDR_LEN);

    wal_asoc_update_connect_result(&st_connet_result, pst_asoc_rsp);

    if (wal_asoc_comp_update_bss_info(pst_event, &st_connet_result, pst_asoc_rsp) != OAL_SUCC) {
        wal_free_asoc_comp_proc_sta_ie_buf(pst_asoc_rsp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    wal_asoc_comp_sta_connect_report(pst_event, net_device, &st_connet_result);

    wal_free_asoc_comp_proc_sta_ie_buf(pst_asoc_rsp);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 如果关联成功，sta根据AP的国家码设置自己的管制域 */
    if (pst_asoc_rsp->en_result_code == HMAC_MGMT_SUCCESS) {
        wal_regdomain_update_sta(pst_event->st_event_hdr.uc_vap_id);
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
        wal_selfstudy_regdomain_update_by_ap(pst_event);
#endif
    }
#endif

    /* 启动发送队列，防止发送队列被漫游关闭后无法恢复 */
    oal_net_tx_wake_all_queues(net_device);

    oal_wifi_report_sta_action(net_device->ifindex, OAL_WIFI_STA_JOIN, st_connet_result.auc_bssid, OAL_MAC_ADDR_LEN);

    oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
        "{wal_asoc_comp_proc_sta status_code[%d] OK!}", st_connet_result.us_status_code);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PMF
/*
 * 功能描述  : 重关联PMF使能上报port authed处理
 * 1.日    期  : 2023年7月25日
 *   修改内容  : 新生成函数
 */
uint32_t wal_report_sta_pmf_port_auth(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *net_device = NULL;
    uint8_t *bss_addr = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_report_sta_pmf_port_auth::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    bss_addr = (uint8_t *)pst_event->auc_event_data;

    /* 获取net_device */
    net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_report_sta_pmf_port_auth::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oam_warning_log2(0, OAM_SF_ASSOC, "{wal_report_sta_pmf_port_auth::bssid[xx:xx:xx:xx:%02x:%02x]!}",
        bss_addr[BYTE_OFFSET_4], bss_addr[BYTE_OFFSET_5]);
    oal_cfg80211_port_authorized(net_device, bss_addr, GFP_KERNEL);
    return OAL_SUCC;
}
#endif

static void wal_roam_comp_proc_sta_free_ie_buf(hmac_roam_rsp_stru *pst_roam_rsp)
{
    if (pst_roam_rsp->puc_asoc_rsp_ie_buff != NULL) {
        oal_free(pst_roam_rsp->puc_asoc_rsp_ie_buff);
        pst_roam_rsp->puc_asoc_rsp_ie_buff = NULL;
    }
    if (pst_roam_rsp->puc_asoc_req_ie_buff != NULL) {
        oal_free(pst_roam_rsp->puc_asoc_req_ie_buff);
        pst_roam_rsp->puc_asoc_req_ie_buff = NULL;
    }
}

/*
 * 功能描述  : STA上报漫游完成事件处理
 * 1.日    期  : 2015年6月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_roam_comp_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    mac_device_stru *pst_mac_device = NULL;
    hmac_roam_rsp_stru *pst_roam_rsp = NULL;
    struct ieee80211_channel *pst_channel = NULL;
    uint8_t band;
    int32_t l_freq;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_roam_comp_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_roam_rsp = (hmac_roam_rsp_stru *)pst_event->auc_event_data;

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ROAM,
                       "{wal_asoc_comp_proc_sta::get net device ptr is null!}");
        wal_roam_comp_proc_sta_free_ie_buf(pst_roam_rsp);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取device id 指针 */
    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (pst_mac_device == NULL) {
        oam_warning_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SCAN,
                         "{wal_asoc_comp_proc_sta::pst_mac_device is null ptr!}");
        wal_roam_comp_proc_sta_free_ie_buf(pst_roam_rsp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_roam_rsp->st_channel.en_band >= WLAN_BAND_BUTT) {
        oam_error_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ROAM,
                       "{wal_asoc_comp_proc_sta::unexpected band[%d]!}",
                       pst_roam_rsp->st_channel.en_band);
        wal_roam_comp_proc_sta_free_ie_buf(pst_roam_rsp);
        return OAL_FAIL;
    }

    band = hmac_get_80211_band_type(&pst_roam_rsp->st_channel);

    l_freq = oal_ieee80211_channel_to_frequency(pst_roam_rsp->st_channel.uc_chan_number, band);

    pst_channel = (struct ieee80211_channel *)oal_ieee80211_get_channel(pst_mac_device->pst_wiphy, l_freq);

    /* 调用内核接口，上报关联结果 */
    oal_cfg80211_roamed(pst_net_device, pst_channel, pst_roam_rsp->auc_bssid, pst_roam_rsp->puc_asoc_req_ie_buff,
        pst_roam_rsp->asoc_req_ie_len, pst_roam_rsp->puc_asoc_rsp_ie_buff, pst_roam_rsp->asoc_rsp_ie_len, GFP_ATOMIC);

    oam_warning_log2(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
        "{wal_roam_comp_proc_sta::oal_cfg80211_roamed OK asoc_req_ie len[%d] asoc_rsp_ie len[%d]!}",
        pst_roam_rsp->asoc_req_ie_len, pst_roam_rsp->asoc_rsp_ie_len);
    wal_roam_comp_proc_sta_free_ie_buf(pst_roam_rsp);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_11R
/*
 * 功能描述  : STA上报FT事件处理
 * 1.日    期  : 2015年6月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_ft_event_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_roam_ft_stru *pst_ft_event = NULL;
    oal_cfg80211_ft_event_stru st_cfg_ft_event;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_ft_event_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_ft_event = (hmac_roam_ft_stru *)pst_event->auc_event_data;

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ROAM,
                       "{wal_ft_event_proc_sta::get net device ptr is null!}");
        oal_free(pst_ft_event->puc_ft_ie_buff);
        pst_ft_event->puc_ft_ie_buff = NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_cfg_ft_event.ies = pst_ft_event->puc_ft_ie_buff;
    st_cfg_ft_event.ies_len = pst_ft_event->us_ft_ie_len;
    st_cfg_ft_event.target_ap = pst_ft_event->auc_bssid;
    st_cfg_ft_event.ric_ies = NULL;
    st_cfg_ft_event.ric_ies_len = pst_ft_event->us_ft_ie_len;

    /* 调用内核接口，上报关联结果 */
    oal_cfg80211_ft_event(pst_net_device, &st_cfg_ft_event);

    oal_free(pst_ft_event->puc_ft_ie_buff);
    pst_ft_event->puc_ft_ie_buff = NULL;
    return OAL_SUCC;
}

uint32_t wal_roam_11v_succ_report(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    oal_net_device_stru *net_dev;
    oal_wireless_dev_stru *wdev;
    oal_netbuf_stru *skb;
    int32_t mem_length;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
    uint16_t roam_trigger;

    event = frw_get_event_stru(event_mem);
    roam_trigger = *(uint16_t *)(event->auc_event_data);

    net_dev = hmac_vap_get_net_device(event->st_event_hdr.uc_vap_id);
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{wal_roam_11v_succ_report::cannot find netdev}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL || wdev->wiphy == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "wal_roam_11v_succ_report::wdev or wdev->wiphy is null");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mem_length = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(sizeof(uint16_t));
    skb = oal_cfg80211_vendor_event_alloc(wdev->wiphy, wdev, mem_length, VENDOR_11V_ROAM_SUCC_EVENT, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_ROAM, "wal_roam_11v_succ_report::skb alloc failed len:%d flags:%d",
            mem_length, kflags);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u16(skb, ROAM_ATTRIBUTE_TRIGGER_TYPE, roam_trigger);
    oal_cfg80211_vendor_event(skb, kflags);

    oam_warning_log1(0, OAM_SF_ROAM, "wal_roam_11v_succ_report::roam_trigger:%d", roam_trigger);

    return OAL_SUCC;
}

uint32_t wal_ft_connect_fail_report(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    oal_net_device_stru *net_dev;
    oal_wireless_dev_stru *wdev;
    oal_netbuf_stru *skb;
    int32_t mem_needed;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
    uint16_t status_code;

    event = frw_get_event_stru(event_mem);
    status_code = *(uint16_t *)(event->auc_event_data);

    net_dev = hmac_vap_get_net_device(event->st_event_hdr.uc_vap_id);
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ROAM, "{wal_ft_connect_fail_report::cannot find netdev}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL || wdev->wiphy == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "wal_ft_connect_fail_report::wdev or wdev->wiphy is null");
        oal_dev_put(net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }
    mem_needed = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(sizeof(uint16_t));
    skb = oal_cfg80211_vendor_event_alloc(wdev->wiphy, wdev, mem_needed, VENDOR_FT_CONNECT_FAIL_EVENT, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_ROAM, "wal_ft_connect_fail_report::skb alloc failed len:%d flags:%d",
            mem_needed, kflags);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u16(skb, FT_ATTRIBUTE_STATUS_CODE, status_code);
    oal_cfg80211_vendor_event(skb, kflags);

    oam_warning_log1(0, OAM_SF_ROAM, "wal_ft_connect_fail_report::status_code:%d", status_code);

    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_11R
/*
 * 功能描述  : STA上报去关联完成事件处理
 * 1.日    期  : 2013年7月5日
  *   修改内容  : 新生成函数
 */
uint32_t wal_disasoc_comp_proc_sta(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_disconnect_result_stru st_disconnect_result;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_disconnect *disconnect = NULL;
    uint8_t vap_id;
    uint8_t locally_generated;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34))
    uint32_t ret;
#endif

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_disasoc_comp_proc_sta::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    vap_id = pst_event->st_event_hdr.uc_vap_id;

    /* 获取locally_generated, 是否本地触发的去关联 */
    locally_generated = wal_get_locally_generated(vap_id);
    wal_set_locally_generated(vap_id, OAL_FALSE);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(vap_id, OAM_SF_ASSOC, "{wal_disasoc_comp_proc_sta::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取去关联原因码指针 */
    disconnect = (hmac_disconnect *)(pst_event->auc_event_data);

    memset_s(&st_disconnect_result, sizeof(oal_disconnect_result_stru), 0, sizeof(oal_disconnect_result_stru));

    /* 准备上报内核的关联结果结构体 */
    st_disconnect_result.us_reason_code = disconnect->reason_code;

    /* 调用内核接口，上报去关联结果 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    oal_cfg80211_disconnected(pst_net_device, st_disconnect_result.us_reason_code, st_disconnect_result.pus_disconn_ie,
        st_disconnect_result.us_disconn_ie_len, GFP_ATOMIC, locally_generated);
#else
    ret = oal_cfg80211_disconnected(pst_net_device, st_disconnect_result.us_reason_code,
        st_disconnect_result.pus_disconn_ie, st_disconnect_result.us_disconn_ie_len,
        GFP_ATOMIC, locally_generated);
    if (ret != OAL_SUCC) {
        oam_warning_log1(vap_id, OAM_SF_ASSOC,
                         "{wal_disasoc_comp_proc_sta::oal_cfg80211_disconnected fail[%d]!}", ret);
        return ret;
    }
#endif

    // sta模式下，不需要传mac地址
    oal_wifi_report_sta_action(pst_net_device->ifindex, OAL_WIFI_STA_LEAVE, 0, 0);

    oam_warning_log1(vap_id, OAM_SF_ASSOC,
        "{wal_disasoc_comp_proc_sta reason_code[%d] OK!}", st_disconnect_result.us_reason_code);
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    g_country_code_self_study_flag = OAL_TRUE;
#endif
    return OAL_SUCC;
}

uint32_t wal_ap_report_new_sta_connect(oal_net_device_stru *net_device, uint8_t *connect_user_addr,
    uint8_t user_addr_len, oal_station_info_stru *station_info)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34))
    uint32_t ret;
#endif
    /* 调用内核接口，上报STA关联结果 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
    oal_cfg80211_new_sta(net_device, connect_user_addr, station_info, GFP_ATOMIC);
#else
    ret = oal_cfg80211_new_sta(net_device, connect_user_addr, station_info, GFP_ATOMIC);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                         "{wal_connect_new_sta_proc_ap::oal_cfg80211_new_sta fail[%d]!}", ret);
        return ret;
    }
#endif
    return OAL_SUCC;
}

uint32_t wal_get_assoc_user_info(frw_event_stru *event, oal_station_info_stru *station_info,
    uint8_t *connect_user_addr, uint8_t user_addr_len)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    hmac_asoc_user_req_ie_stru *asoc_user_req_info;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    /* 向内核标记填充了关联请求帧的ie信息 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    /* Linux 4.0 版本不需要STATION_INFO_ASSOC_REQ_IES 标识 */
#else
    station_info->filled |= STATION_INFO_ASSOC_REQ_IES;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    asoc_user_req_info = (hmac_asoc_user_req_ie_stru *)(event->auc_event_data);
    station_info->assoc_req_ies = asoc_user_req_info->puc_assoc_req_ie_buff;
    if (station_info->assoc_req_ies == NULL) {
        oam_error_log0(event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_connect_new_sta_proc_ap::asoc ie is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    station_info->assoc_req_ies_len = asoc_user_req_info->assoc_req_ie_len;

    /* 获取关联user mac addr */
    memcpy_s(connect_user_addr, user_addr_len,
             (uint8_t *)asoc_user_req_info->auc_user_mac_addr, user_addr_len);
#else
    /* 获取关联user mac addr */
    memcpy_s(connect_user_addr, user_addr_len,
             (uint8_t *)event->auc_event_data, user_addr_len);
#endif
    return OAL_SUCC;
}

/*
 * 功能描述  : 驱动上报内核bss网络中新加入了一个STA
 * 1.日    期  : 2013年9月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_connect_new_sta_proc_ap(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    uint8_t auc_connect_user_addr[WLAN_MAC_ADDR_LEN];
    oal_station_info_stru st_station_info;
    uint32_t ret;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_connect_new_sta_proc_ap::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_connect_new_sta_proc_ap::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&st_station_info, sizeof(oal_station_info_stru), 0, sizeof(oal_station_info_stru));

    ret = wal_get_assoc_user_info(pst_event, &st_station_info, auc_connect_user_addr, WLAN_MAC_ADDR_LEN);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = wal_ap_report_new_sta_connect(pst_net_device, auc_connect_user_addr, WLAN_MAC_ADDR_LEN, &st_station_info);
    if (ret != OAL_SUCC) {
        return ret;
    }

    oal_wifi_report_sta_action(pst_net_device->ifindex, OAL_WIFI_STA_JOIN, auc_connect_user_addr, WLAN_MAC_ADDR_LEN);

    oam_warning_log3(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                     "{wal_connect_new_sta_proc_ap mac[%02X:XX:XX:XX:%02X:%02X] OK!}",
                     auc_connect_user_addr[MAC_ADDR_0],
                     auc_connect_user_addr[MAC_ADDR_4],
                     auc_connect_user_addr[MAC_ADDR_5]);

    return OAL_SUCC;
}

/*
 * 功能描述  : 驱动上报内核bss网络中删除了一个STA
 * 1.日    期  : 2013年9月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_disconnect_sta_proc_ap(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    uint8_t user_addr[WLAN_MAC_ADDR_LEN];
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 44))
    int32_t l_ret;
#endif

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{wal_disconnect_sta_proc_ap::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                       "{wal_disconnect_sta_proc_ap::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取去关联user mac addr */
    if (EOK != memcpy_s(user_addr, WLAN_MAC_ADDR_LEN,
                        (uint8_t *)pst_event->auc_event_data, WLAN_MAC_ADDR_LEN)) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY, "{wal_disconnect_sta_proc_ap:memcopy fail!}");
        return OAL_FAIL;
    }

    /* 调用内核接口，上报STA去关联结果 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    oal_cfg80211_del_sta(pst_net_device, user_addr, WLAN_MAC_ADDR_LEN, GFP_ATOMIC);
#else
    l_ret = oal_cfg80211_del_sta(pst_net_device, &user_addr[0], WLAN_MAC_ADDR_LEN, GFP_ATOMIC);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
                         "{wal_disconnect_sta_proc_ap::oal_cfg80211_del_sta return fail[%d]!}", l_ret);
        return OAL_FAIL;
    }
#endif
    oam_warning_log3(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ASSOC,
        "{wal_disconnect_sta_proc_ap mac[%02X:XX:XX:XX:%02X:%02X] OK!}",
        user_addr[MAC_ADDR_0], user_addr[MAC_ADDR_4], user_addr[MAC_ADDR_5]);

    oal_wifi_report_sta_action(pst_net_device->ifindex, OAL_WIFI_STA_LEAVE, user_addr, WLAN_MAC_ADDR_LEN);

    return OAL_SUCC;
}

/*
 * 功能描述  : 驱动上报内核mic错误
 * 1.日    期  : 2013年12月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_mic_failure_proc(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_mic_event_stru *pst_mic_event = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_CRYPTO, "{wal_mic_failure_proc::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_mic_event = (hmac_mic_event_stru *)(pst_event->auc_event_data);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CRYPTO,
                       "{wal_mic_failure_proc::get net device ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用内核接口，上报mic攻击 */
    oal_cfg80211_mic_failure(pst_net_device, pst_mic_event->auc_user_mac,
                             pst_mic_event->en_key_type, pst_mic_event->l_key_id,
                             NULL, GFP_ATOMIC);

    oam_warning_log3(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CRYPTO,
        "{wal_mic_failure_proc::mac[%02X:XX:XX:XX:%02X:%02X] OK!}", pst_mic_event->auc_user_mac[MAC_ADDR_0],
        pst_mic_event->auc_user_mac[MAC_ADDR_4], pst_mic_event->auc_user_mac[MAC_ADDR_5]);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_NAN
OAL_STATIC oal_netbuf_stru *wal_nan_alloc_rx_event(int32_t mem_needed, oal_gfp_enum_uint8 kflags)
{
    oal_netbuf_stru *skb = NULL;
    oal_net_device_stru *nan0_dev = NULL;
    oal_wireless_dev_stru *wdev = NULL;

    /* 使用nan0上报vendor event */
    nan0_dev = oal_dev_get_by_name("nan0");
    if (!nan0_dev) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_nan_rx_send_event::cannot find nan netdev}");
        return NULL;
    }

    wdev = oal_netdevice_wdev(nan0_dev);
    if (wdev == NULL || wdev->wiphy == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "wal_nan_rx_mgmt::wdev or wdev->wiphy is null");
        oal_dev_put(nan0_dev);
        return NULL;
    }

    skb = oal_cfg80211_vendor_event_alloc(wdev->wiphy, wdev, mem_needed, NAN_EVENT_RX, kflags);
    oal_dev_put(nan0_dev); /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    return skb;
}

uint32_t wal_nan_send_response(int32_t attr, uint16_t transaction)
{
    oal_netbuf_stru *skb;
    int32_t mem_needed;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

    mem_needed = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(sizeof(transaction));
    skb = wal_nan_alloc_rx_event(mem_needed, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_ANY, "wal_nan_rx_mgmt::skb alloc failed, len %d, flags %d", mem_needed, kflags);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u16(skb, attr, transaction);
    oal_cfg80211_vendor_event(skb, kflags);
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_nan_tx_mgmt_get_rsp_attr(uint8_t action)
{
    int32_t attr;
    switch (action) {
        case WLAN_ACTION_NAN_PUBLISH:
            attr = NAN_ATTRIBUTE_PUBLISH_SERVICE_RSP;
            break;
        case WLAN_ACTION_NAN_SUBSCRIBE:
            attr = NAN_ATTRIBUTE_SUBSCRIBE_SERVICE_RSP;
            break;
        case WLAN_ACTION_NAN_FLLOWUP:
            attr = NAN_ATTRIBUTE_TRANSMIT_FOLLOWUP_RSP;
            break;
        default:
            attr = NAN_ATTRIBUTE_ERROR_RSP;
    }
    return attr;
}

uint32_t wal_nan_response_event_process(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    mac_nan_rsp_msg_stru *nan_rsp;
    int32_t attr = NAN_ATTRIBUTE_ERROR_RSP;

    event = frw_get_event_stru(event_mem);
    nan_rsp = (mac_nan_rsp_msg_stru*)(event->auc_event_data);

    if (nan_rsp->status) {
        if (nan_rsp->type == NAN_RSP_TYPE_SET_PARAM) {
            attr = NAN_ATTRIBUTE_CAPABILITIES_RSP;
        } else if (nan_rsp->type == NAN_RSP_TYPE_SET_TX_MGMT) {
            attr = wal_nan_tx_mgmt_get_rsp_attr(nan_rsp->action);
        } else {
            attr = NAN_ATTRIBUTE_ERROR_RSP;
        }
    } else {
        attr = NAN_ATTRIBUTE_ERROR_RSP;
    }
    return wal_nan_send_response(attr, nan_rsp->transaction);
}
#endif


/*
 * 功能描述  : NAN管理帧上报
 * 1.日    期  : 2020年4月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_nan_rx_mgmt(oal_net_device_stru *net_device, uint8_t *buf, uint16_t len)
{
#ifdef _PRE_WLAN_FEATURE_NAN
    int32_t mem_needed;
    oal_netbuf_stru *skb = NULL;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

    mem_needed = (int32_t)VENDOR_DATA_OVERHEAD + (int32_t)oal_nlmsg_length(len);
    skb = wal_nan_alloc_rx_event(mem_needed, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_ANY, "wal_nan_rx_mgmt::skb alloc failed, len %d, flags %d", mem_needed, kflags);
        return;
    }

    oal_nla_put(skb, NAN_ATTRIBUTE_RECEIVE_DATA, len, buf);

    oal_cfg80211_vendor_event(skb, kflags);
    return;
#endif
}

/*
 * 功能描述  : 驱动上报内核接收到管理帧
 * 1.日    期  : 2014年5月17日
  *   修改内容  : 新生成函数
 */
uint32_t wal_send_mgmt_to_host(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    int32_t l_freq;
    uint8_t uc_rssi;
    uint8_t *puc_buf = NULL;
    uint16_t us_len;
    uint32_t ret;
    hmac_rx_mgmt_event_stru *pst_mgmt_frame = NULL;
    oal_ieee80211_mgmt *pst_ieee80211_mgmt = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_send_mgmt_to_host::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_mgmt_frame = (hmac_rx_mgmt_event_stru *)(pst_event->auc_event_data);

    /* 获取net_device */
    pst_net_device = wal_config_get_netdev(pst_mgmt_frame->ac_name, OAL_STRLEN(pst_mgmt_frame->ac_name));
    if (pst_net_device == NULL || pst_net_device->ieee80211_ptr == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY,
                       "{wal_send_mgmt_to_host::get net device or ieee80211_ptr ptr is null!}");
        oal_free(pst_mgmt_frame->puc_buf);
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_dev_put(pst_net_device);

    puc_buf = pst_mgmt_frame->puc_buf;
    us_len = pst_mgmt_frame->us_len;
    l_freq = pst_mgmt_frame->l_freq;
    uc_rssi = pst_mgmt_frame->uc_rssi;

    pst_ieee80211_mgmt = (oal_ieee80211_mgmt *)puc_buf;
    if (pst_mgmt_frame->event_type == HMAC_RX_MGMT_EVENT_TYPE_NAN) {
        wal_nan_rx_mgmt(pst_net_device, puc_buf, us_len);
    } else {
        /**********************************************************************************************
            NOTICE:  AUTH ASSOC DEAUTH DEASSOC 这几个帧上报给host的时候 可能给ioctl调用死锁
                     需要添加这些帧上报的时候 请使用workqueue来上报
        ***********************************************************************************************/
        /* 调用内核接口，上报接收到管理帧 */
        ret = oal_cfg80211_rx_mgmt(pst_net_device, l_freq, uc_rssi, puc_buf, us_len, GFP_ATOMIC);
        if (ret != OAL_SUCC) {
            oam_warning_log2(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY,
                             "{wal_send_mgmt_to_host::fc[0x%02x], if_type[%d]!}\r\n",
                             pst_ieee80211_mgmt->frame_control, pst_net_device->ieee80211_ptr->iftype);
            oam_warning_log3(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY,
                             "{wal_send_mgmt_to_host::oal_cfg80211_rx_mgmt fail[%d]!len[%d], freq[%d]}",
                             ret, us_len, l_freq);
            oal_free(puc_buf);
            return OAL_FAIL;
        }
    }
    oal_free(puc_buf);
    return OAL_SUCC;
}

/*
 * 功能描述  : HMAC上报监听超时
 * 1.日    期  : 2014年11月25日
  *   修改内容  : 新生成函数
 */
uint32_t wal_p2p_listen_timeout(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_wireless_dev_stru *pst_wdev = NULL;
    uint64_t ull_cookie;
    oal_ieee80211_channel_stru st_listen_channel;
    hmac_p2p_listen_expired_stru *pst_p2p_listen_expired = NULL;
    mac_device_stru *pst_mac_device = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_P2P, "{wal_p2p_listen_timeout::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    /* 获取mac_device_stru */
    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_PROXYSTA,
                       "{wal_p2p_listen_timeout::get mac_device ptr is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_p2p_listen_expired = (hmac_p2p_listen_expired_stru *)(pst_event->auc_event_data);

    pst_wdev = pst_p2p_listen_expired->pst_wdev;
    ull_cookie = pst_mac_device->st_p2p_info.ull_last_roc_id;
    st_listen_channel = pst_p2p_listen_expired->st_listen_channel;
    oal_cfg80211_remain_on_channel_expired(pst_wdev, ull_cookie, &st_listen_channel, GFP_ATOMIC);
    if (!hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH)) {
        oam_warning_log1(0, OAM_SF_P2P, "{wal_p2p_listen_timeout::END!cookie [%x]}\r\n", ull_cookie);
    }
    return OAL_SUCC;
}

/* 功能描述  : 漫游场景通过vendor 事件， 上报wpa_supplicant external_auth */
uint32_t wal_roam_exteranl_auth_report(oal_net_device_stru *net_device,
    oal_cfg80211_external_auth_stru *data, uint32_t len)
{
    oal_wireless_dev_stru *wdev;
    oal_netbuf_stru *skb;
    int32_t mem_needed;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

    wdev = oal_netdevice_wdev(net_device);
    if (wdev == NULL || wdev->wiphy == NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "wal_roam_exteranl_auth_report::wdev or wdev->wiphy is null");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mem_needed = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(len);
    skb = oal_cfg80211_vendor_event_alloc(wdev->wiphy, wdev, mem_needed, VENDOR_ROAM_EXTERNAL_AUTH, kflags);
    if (oal_unlikely(skb == NULL)) {
        oam_warning_log2(0, OAM_SF_ROAM, "wal_roam_exteranl_auth_report::skb alloc failed len:%d flags:%d",
            mem_needed, kflags);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_nla_put_u32(skb, ROAM_ATTRIBUTE_EXTERNAL_AUTH_AKM, data->key_mgmt_suite) ||
        oal_nla_put_u32(skb, ROAM_ATTRIBUTE_EXTERNAL_AUTH_ACTION, data->action) ||
        oal_nla_put(skb, ROAM_ATTRIBUTE_EXTERNAL_AUTH_BSSID, OAL_ETH_ALEN_SIZE, data->bssid) ||
        oal_nla_put(skb, ROAM_ATTRIBUTE_EXTERNAL_AUTH_SSID, data->ssid.ssid_len, data->ssid.ssid)) {
        oal_netbuf_free(skb);
        oam_warning_log0(0, OAM_SF_ROAM,
            "wal_roam_exteranl_auth_report:::encap netlink msg fail");
        return OAL_FAIL;
    }
    oal_cfg80211_vendor_event(skb, kflags);

    oam_warning_log0(0, OAM_SF_ROAM, "wal_roam_exteranl_auth_report::succ");

    return OAL_SUCC;
}

/*
 * 功能描述  : HMAC上报external auth request处理
 * 1.日    期  : 2018年12月26日
  *   修改内容  : 新生成函数
 */
uint32_t wal_report_external_auth_req(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_external_auth_req_stru *pst_ext_auth_req = NULL;
    oal_cfg80211_external_auth_stru st_external_auth_req;
    int l_ret;
    int32_t l_memcpy_ret;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_SAE, "{wal_report_external_auth_req::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (pst_net_device == NULL) {
        oam_error_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SAE,
                       "{wal_report_external_auth_req::get net device ptr is null! vap_id %d}",
                       pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ext_auth_req = (hmac_external_auth_req_stru *)(pst_event->auc_event_data);

    st_external_auth_req.action = pst_ext_auth_req->en_action;
    st_external_auth_req.key_mgmt_suite = pst_ext_auth_req->key_mgmt_suite;
    st_external_auth_req.status = pst_ext_auth_req->us_status;
    st_external_auth_req.ssid.ssid_len = oal_min(pst_ext_auth_req->st_ssid.uc_ssid_len, OAL_IEEE80211_MAX_SSID_LEN);
    l_memcpy_ret = memcpy_s(st_external_auth_req.ssid.ssid, sizeof(st_external_auth_req.ssid.ssid),
                            pst_ext_auth_req->st_ssid.auc_ssid, st_external_auth_req.ssid.ssid_len);
    l_memcpy_ret += memcpy_s(st_external_auth_req.bssid, OAL_ETH_ALEN_SIZE, pst_ext_auth_req->auc_bssid, ETH_ALEN);
    if (l_memcpy_ret != EOK) {
        oam_error_log0(0, OAM_SF_SAE, "wal_report_external_auth_req::memcpy fail!");
        return OAL_FAIL;
    }

    if (pst_ext_auth_req->roam_report == OAL_TRUE) {
        l_ret = wal_roam_exteranl_auth_report(pst_net_device,
            &st_external_auth_req, sizeof(st_external_auth_req));
    } else {
        l_ret = oal_cfg80211_external_auth_request(pst_net_device,
            &st_external_auth_req, GFP_ATOMIC);
    }

    oam_warning_log4(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SAE,
        "{wal_report_external_auth_req::action %x, status %d, key_mgmt 0x%X, ssid_len %d}",
        st_external_auth_req.action, st_external_auth_req.status,
        st_external_auth_req.key_mgmt_suite, st_external_auth_req.ssid.ssid_len);

    oam_warning_log4(pst_event->st_event_hdr.uc_vap_id, OAM_SF_SAE,
        "{wal_report_external_auth_req::mac[%02X:XX:XX:XX:%02X:%02X], ret[%d]}",
        st_external_auth_req.bssid[MAC_ADDR_0],
        st_external_auth_req.bssid[MAC_ADDR_4],
        st_external_auth_req.bssid[MAC_ADDR_5],
        l_ret);

    return l_ret;
}
#ifdef _PRE_WLAN_FEATURE_M2S
/*
 * 函 数 名  : wal_cfg80211_m2s_status_report
 * 功能描述  : 触发上报m2s事件
 * 1.日    期  : 2018年2月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_cfg80211_m2s_status_report(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    dmac_m2s_complete_syn_stru *pst_m2s_switch_comp_status;
    wlan_m2s_mgr_vap_stru *pst_m2s_vap_mgr;
    hmac_vap_stru *pst_hmac_vap;
    uint8_t uc_vap_idx;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{wal_cfg80211_m2s_status_report::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = (frw_event_stru *)pst_event_mem->puc_data;

    /* 获取配置vap */
    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_m2s_switch_comp_status = (dmac_m2s_complete_syn_stru *)(pst_event->auc_event_data);

    /* 需要上层提供oal接口 */
    oam_warning_log4(0, OAM_SF_M2S,
        "{wal_cfg80211_m2s_status_report::vap num[%d]device result[%d]state[%d]m2s_mode[%d].}",
        pst_m2s_switch_comp_status->pri_data.mss_result.uc_vap_num,
        pst_m2s_switch_comp_status->pri_data.mss_result.en_m2s_result,
        pst_m2s_switch_comp_status->uc_m2s_state, pst_m2s_switch_comp_status->pri_data.mss_result.uc_m2s_mode);

    for (uc_vap_idx = 0; uc_vap_idx < pst_m2s_switch_comp_status->pri_data.mss_result.uc_vap_num; uc_vap_idx++) {
        pst_m2s_vap_mgr = &(pst_m2s_switch_comp_status->ast_m2s_comp_vap[uc_vap_idx]);

        oam_warning_log3(0, OAM_SF_M2S,
                         "{wal_cfg80211_m2s_status_report::Notify to host, ap index[%d]action type[%d]state[%d].}",
                         uc_vap_idx, pst_m2s_vap_mgr->en_action_type, pst_m2s_vap_mgr->en_m2s_result);

        oam_warning_log3(0, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::Notify to host, ap addr->%02X:XX:XX:XX:%02X:%02X.}",
            pst_m2s_vap_mgr->auc_user_mac_addr[MAC_ADDR_0],
            pst_m2s_vap_mgr->auc_user_mac_addr[MAC_ADDR_4], pst_m2s_vap_mgr->auc_user_mac_addr[MAC_ADDR_5]);
    }

    if (!pst_hmac_vap->pst_net_device) {
        oam_warning_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_M2S,
            "{wal_cfg80211_m2s_status_report::net dev is null, vap maybe deleted!}");
        return OAL_SUCC;
    }

    /* 上报内核 */
    oal_cfg80211_m2s_status_report(pst_hmac_vap->pst_net_device, GFP_KERNEL,
                                   (uint8_t *)pst_m2s_switch_comp_status, sizeof(dmac_m2s_complete_syn_stru));

    return OAL_SUCC;
}
#endif
/*
 * 功能描述  : HMAC上报channel switch处理
 * 1.日    期  : 2019年09月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_report_channel_switch(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    oal_net_device_stru *pst_net_device = NULL;
    hmac_channel_switch_stru *pst_channel_switch = NULL;
    oal_cfg80211_chan_def st_chandef = {0};
    hmac_device_stru *pst_hmac_device = NULL;
    oal_wiphy_stru *pst_wiphy = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_CHAN, "{wal_report_channel_switch::pst_event_mem is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 获取net_device */
    pst_net_device = hmac_vap_get_net_device(pst_event->st_event_hdr.uc_vap_id);
    if (oal_unlikely(pst_net_device == NULL)) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::get net device ptr is null! vap_id %d}",
                       pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取hmac_device */
    pst_hmac_device = hmac_res_get_mac_dev(pst_event->st_event_hdr.uc_device_id);
    if (oal_unlikely(pst_hmac_device == NULL)) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::get pst_hmac_device is null! device_id %d}",
                       pst_event->st_event_hdr.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取wiphy */
    pst_wiphy = pst_hmac_device->pst_device_base_info->pst_wiphy;
    if (oal_unlikely(pst_wiphy == NULL)) {
        oam_error_log0(0, OAM_SF_CHAN, "{wal_report_channel_switch:: pst_wiphy is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_channel_switch = (hmac_channel_switch_stru *)(pst_event->auc_event_data);

    st_chandef.chan = oal_ieee80211_get_channel(pst_wiphy, pst_channel_switch->center_freq);
    if (oal_unlikely(st_chandef.chan == NULL)) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::can't get chan pointer from wiphy(freq=%d)!}",
            pst_channel_switch->center_freq);
        return OAL_FAIL;
    }
    st_chandef.chan->center_freq = pst_channel_switch->center_freq;
    st_chandef.width = pst_channel_switch->width;
    st_chandef.center_freq1 = pst_channel_switch->center_freq1;
    st_chandef.center_freq2 = pst_channel_switch->center_freq2;

    oal_cfg80211_ch_switch_notify(pst_net_device, &st_chandef);

    oam_warning_log4(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CHAN,
                     "{wal_report_channel_switch::center_freq=%d, width=%d, center_freq1=%d, center_freq2=%d}",
                     st_chandef.chan->center_freq, st_chandef.width,
                     st_chandef.center_freq1, st_chandef.center_freq2);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_FTM
/*
 * 功能描述  : HMAC上报wifi rtt resulr report处理
 * 1.日    期  : 2020年09月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_report_rtt_result(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    mac_ftm_wifi_rtt_result* rtt_result;
    oal_net_device_stru *net_dev;
    oal_wireless_dev_stru *wdev;
    oal_netbuf_stru *skb;
    int32_t mem_needed;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
    oal_nlattr_stru *rtt_nl_hdr = NULL;

    event = frw_get_event_stru(event_mem);
    rtt_result = (mac_ftm_wifi_rtt_result*)(event->auc_event_data);
    if (rtt_result == NULL) {
        oam_warning_log0(0, OAM_SF_FTM, "{wal_report_rtt_result::rtt_result NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    net_dev = hmac_vap_get_net_device(event->st_event_hdr.uc_vap_id);
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_FTM, "{wal_report_rtt_result::cannot find netdev}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL || wdev->wiphy == NULL) {
        oam_error_log0(0, OAM_SF_FTM, "wal_report_rtt_result::wdev or wdev->wiphy is null");
        oal_dev_put(net_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }
    mem_needed = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(sizeof(mac_ftm_wifi_rtt_result));
    skb = oal_cfg80211_vendor_event_alloc(wdev->wiphy, wdev, mem_needed, VENDOR_RTT_COMPLETE_EVENT, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_FTM, "wal_report_rtt_result::skb alloc failed len:%d flags:%d", mem_needed, kflags);
        return -OAL_ENOMEM;
    }
    oal_nla_put_u32(skb, RTT_ATTRIBUTE_RESULTS_COMPLETE, 1);
    rtt_nl_hdr = oal_nla_nest_start(skb, RTT_ATTRIBUTE_RESULTS_PER_TARGET);
    if (!rtt_nl_hdr) {
        oam_warning_log0(0, 0, "wal_report_rtt_result::rtt_nl_hdr is NULL");
        oal_netbuf_free(skb);
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_nla_put(skb, RTT_ATTRIBUTE_TARGET_MAC, ETHER_ADDR_LEN, &rtt_result->addr);
    oal_nla_put_u32(skb, RTT_ATTRIBUTE_RESULT_CNT, 1);
    oal_nla_put(skb, RTT_ATTRIBUTE_RESULT, sizeof(mac_ftm_wifi_rtt_result), (uint8_t*)rtt_result);
    oal_nla_nest_end(skb, rtt_nl_hdr);
    oam_warning_log3(0, OAM_SF_FTM, "wal_report_rtt_result::status:%d negotiated_num:%d succ_number:%d",
        rtt_result->status, rtt_result->negotiated_burst_num, rtt_result->success_number);
    oam_warning_log4(0, OAM_SF_FTM, "wal_report_rtt_result::rssi:%d distance_mm:%d distance_sd_mm:%d distance_sp_mm:%d",
        rtt_result->rssi, rtt_result->distance_mm, rtt_result->distance_sd_mm, rtt_result->distance_spread_mm);
    oal_cfg80211_vendor_event(skb, kflags);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_PWL
uint32_t wal_event_req_pwl_pre_key(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    pwl_req_key_info *req_key_info = NULL;
    oal_net_device_stru *net_dev = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    oal_netbuf_stru *skb = NULL;
    int32_t mem_needed;
    oal_gfp_enum_uint8 kflags = oal_in_atomic() ? GFP_ATOMIC : GFP_KERNEL;

    /* 获取hmac上报内容 */
    event = frw_get_event_stru(event_mem);
    req_key_info = (pwl_req_key_info*)(event->auc_event_data);
    if (req_key_info == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_event_req_pwl_pre_key::req_key_info null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取net_device */
    net_dev = hmac_vap_get_net_device(event->st_event_hdr.uc_vap_id);
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_event_req_pwl_pre_key::cannot find netdev}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取wiphy */
    wdev = oal_netdevice_wdev(net_dev);
    if (wdev == NULL || wdev->wiphy == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_event_req_pwl_pre_key::wdev or wdev->wiphy is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 计算skb需要申请内存的大小 */
    mem_needed = VENDOR_DATA_OVERHEAD + oal_nlmsg_length(sizeof(pwl_req_key_info));
    /* skb申请内存 */
    skb = oal_cfg80211_vendor_event_alloc(wdev->wiphy, wdev, mem_needed, VENDOR_REQ_PWL_PRE_KEY_EVENT, kflags);
    if (oal_unlikely(!skb)) {
        oam_warning_log2(0, OAM_SF_ANY, "{wal_event_req_pwl_pre_key::skb alloc failed len[%d] flags[%d]}", mem_needed,
            kflags);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 填充数据 */
    oal_nla_put(skb, PWL_ATTRIBUTE_PRE_KEY_REQ, sizeof(pwl_req_key_info), (uint8_t*)req_key_info);
    oam_warning_log0(0, OAM_SF_ANY, "wal_event_req_pwl_pre_key::report pre_key_req success");
    /* 发送 */
    oal_cfg80211_vendor_event(skb, kflags);
    return OAL_SUCC;
}
#endif
