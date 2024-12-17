/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 扫描模块 hmac功能
 * 作    者 :
 * 创建日期 : 2014年11月26日
 */

/* 1 头文件包含 */
#include "hmac_scan.h"
#include "mac_frame_inl.h"
#include "wlan_chip_i.h"
#include "hmac_sme_sta.h"
#include "hmac_roam_scan.h"
#include "hmac_chan_mgmt.h"
#include "hmac_p2p.h"
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
#include "hmac_hid2d.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_11ax.h"
#endif
#include "hmac_11r.h"
#include "hmac_scan_bss.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SCAN_C

/* 2 全局变量定义 */
hmac_scan_state_enum g_en_bgscan_enable_flag = HMAC_BGSCAN_ENABLE;
uint32_t g_pd_bss_expire_time = 0;
/* 等待侦听完成的额外等待时间(单位: ms)，参考wpa supplicant中侦听超时的等待时间 */
#define HMAC_P2P_LISTEN_EXTRA_WAIT_TIME 30

/*
 * 函 数 名  : hmac_scan_print_scan_params
 * 功能描述  : 打印扫描到的bss信息
 * 1.日    期  : 2015年2月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_print_scan_params(mac_scan_req_stru *scan_params, mac_vap_stru *mac_vap)
{
    if (!((scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) &&
        hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH))) {
        oam_warning_log4(scan_params->uc_vap_id, OAM_SF_SCAN,
            "hmac_scan_print_scan_params::Now Scan channel_num[%d] in [%d]ms with scan_func[0x%x], and ssid_num[%d]!",
            scan_params->uc_channel_nums, scan_params->us_scan_time,
            scan_params->uc_scan_func, scan_params->uc_ssid_num);

        oam_warning_log4(scan_params->uc_vap_id, OAM_SF_SCAN,
            "hmac_scan_print_scan_params::p2p_scan:%d,max_scan_count_per_channel:%d,need back home_channel:%d, \
            scan_channel_interval %d",
            scan_params->bit_is_p2p0_scan, scan_params->uc_max_scan_count_per_channel,
            scan_params->en_need_switch_back_home_channel, scan_params->uc_scan_channel_interval);
    }
    return;
}

/*
 * 函 数 名  : hmac_scan_clean_scan
 * 功能描述  : 清除上次扫描请求相关的扫描记录信息: 包括扫描到的bss信息，并释放内存空间、以及其他信息清零
 * 1.日    期  : 2015年2月2日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_scan_clean_scan(hmac_scan_stru *scan)
{
    hmac_scan_record_stru *scan_record = NULL;
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;

    /* 参数合法性检查 */
    if (scan == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_scan::pointer scan is null.}");
        return;
    }

    scan_record = &scan->st_scan_record_mgmt;

    /* 1.一定要先清除扫描到的bss信息，再进行清零处理 */
    bss_mgmt = &(scan_record->st_bss_mgmt);

    /* 对链表写操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 遍历链表，删除扫描到的bss信息 */
    while (oal_dlist_is_empty(&(bss_mgmt->st_bss_list_head)) == OAL_FALSE) {
        entry = oal_dlist_delete_head(&(bss_mgmt->st_bss_list_head));
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);

        bss_mgmt->bss_num--;

        /* 释放扫描队列里的内存 */
        oal_free(scanned_bss);
    }

    /* 对链表写操作前加锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    /* 2.其它信息清零 */
    memset_s(scan_record, sizeof(hmac_scan_record_stru), 0, sizeof(hmac_scan_record_stru));
    scan_record->en_scan_rsp_status = MAC_SCAN_STATUS_BUTT; /* 初始化扫描完成时状态码为无效值 */
    scan_record->en_vap_last_state = MAC_VAP_STATE_BUTT;    /* 必须置BUTT,否则aput停扫描会vap状态恢复错 */

    /* 3.重新初始化bss管理结果链表和锁 */
    bss_mgmt = &(scan_record->st_bss_mgmt);
    oal_dlist_init_head(&(bss_mgmt->st_bss_list_head));
    oal_spin_lock_init(&(bss_mgmt->st_lock));

    /* 4.删除扫描超时定时器 */
    if (scan->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(scan->st_scan_timeout));
    }

    return;
}

/*
 * 函 数 名  : hmac_scan_print_scan_record_info
 * 功能描述  : 扫描完成，打印相关维测信息，包括扫描执行时间，扫描返回状态码，扫描到的bss信息等
 * 1.日    期  : 2015年5月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_print_scan_record_info(hmac_vap_stru *hmac_vap, hmac_scan_record_stru *scan_record)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_time_t_stru timestamp_diff;

    /* 获取扫描间隔时间戳 */
    timestamp_diff = oal_ktime_sub(oal_ktime_get(), scan_record->st_scan_start_time);

    /* 调用内核接口，打印此次扫描耗时 */
    if (is_not_p2p_listen_printf(&hmac_vap->st_vap_base_info) == OAL_TRUE) {
        oam_warning_log4(scan_record->uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_print_scan_record_info::scan comp,scan_status:%d,vap ch_num:%d, \
            cookie:%x, duration time:%lums.}",
            scan_record->en_scan_rsp_status, hmac_vap->st_vap_base_info.st_channel.uc_chan_number,
            scan_record->ull_cookie, ktime_to_ms(timestamp_diff));
    }
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    // 如果是HID2D发起的扫描，不需要打印信息 */
    if (scan_record->en_scan_mode == WLAN_SCAN_MODE_HID2D_SCAN) {
        return;
    }
#endif

    if (!hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH) || hmac_vap->bit_in_p2p_listen == OAL_FALSE) {
        /* 打印扫描到的bss信息 */
        hmac_scan_print_scanned_bss_info(scan_record->uc_device_id);
    }

    return;
}

static void hmac_scan_comp_p2p_listen_proc(hmac_vap_stru *hmac_vap,
    mac_scan_rsp_stru *d2h_scan_rsp_info)
{
    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        hmac_p2p_listen_timeout(hmac_vap, &hmac_vap->st_vap_base_info);
    }
    if (hmac_vap->en_wait_roc_end == OAL_TRUE) {
        /*
         * 有此标记就通知结束,防止wal层下发扫描的时候在扫描,下发abort,
         * 但此时刚好扫描正常结束,abort未执行,这里就未complete
         */
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_comp_p2p_listen_proc::scan rsp status[%d]}", d2h_scan_rsp_info->en_scan_rsp_status);
        oal_complete(&(hmac_vap->st_roc_end_ready));
        hmac_vap->en_wait_roc_end = OAL_FALSE;
    }
}
static void hmac_scan_change_vap_state(hmac_vap_stru *hmac_vap, hmac_scan_stru *scan_mgmt,
    mac_scan_rsp_stru *d2h_scan_rsp_info)
{
    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO)) {
        if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* 改变vap状态到SCAN_COMP */
            hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }
    }
    /* BEGIN: mp12 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (scan_mgmt->st_scan_record_mgmt.en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        hmac_fsm_change_state(hmac_vap, scan_mgmt->st_scan_record_mgmt.en_vap_last_state);
        scan_mgmt->st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
}

static uint8_t hmac_scan_check_scan_comp_valid(hmac_vap_stru *hmac_vap, frw_event_hdr_stru *event_hdr,
    hmac_scan_stru *scan_mgmt, mac_scan_rsp_stru *d2h_scan_rsp_info)
{
    if ((event_hdr->uc_vap_id != scan_mgmt->st_scan_record_mgmt.uc_vap_id) ||
        ((hmac_vap->st_vap_base_info.en_p2p_mode != WLAN_LEGACY_VAP_MODE) &&
        (d2h_scan_rsp_info->ull_cookie != scan_mgmt->st_scan_record_mgmt.ull_cookie))) {
        oam_warning_log4(event_hdr->uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_check_scan_comp_valid::Report vap:%d Scan_rsp(cookie %d), another vap:%d scan(cookie %d)!}",
            event_hdr->uc_vap_id, d2h_scan_rsp_info->ull_cookie,
            scan_mgmt->st_scan_record_mgmt.uc_vap_id, scan_mgmt->st_scan_record_mgmt.ull_cookie);
            return OAL_FALSE;
    }
    return OAL_TRUE;
}

static void hmac_scan_proc_scan_timer(hmac_scan_stru *scan_mgmt, mac_scan_rsp_stru *d2h_scan_rsp_info)
{
    oal_bool_enum_uint8 temp_flag;

    /* 删除扫描超时保护定时器 */
    temp_flag = ((scan_mgmt->st_scan_timeout.en_is_registerd == OAL_TRUE) &&
                 (d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO));
    if (temp_flag) {
        /* PNO没有启动扫描定时器,考虑到取消PNO扫描,立即下发普通扫描,PNO扫描结束事件对随后的普通扫描的影响 */
        frw_timer_immediate_destroy_timer_m(&(scan_mgmt->st_scan_timeout));
    }
}

static void hmac_scan_comp_roam_proc(hmac_vap_stru *hmac_vap, hmac_scan_stru *scan_mgmt)
{
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    /* STA背景扫描时，需要提前识别漫游场景 */
    if (is_legacy_sta(&hmac_vap->st_vap_base_info) &&
        (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP) &&
        (scan_mgmt->st_scan_record_mgmt.en_scan_mode == WLAN_SCAN_MODE_BACKGROUND_STA)) {
        hmac_roam_check_bkscan_result(hmac_vap, &(scan_mgmt->st_scan_record_mgmt));
    }
    if (roam_info != NULL && roam_info->roam_11v_scan_reject == OAL_TRUE) {
        roam_info->roam_11v_scan_reject = OAL_FALSE;
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_comp_roam_proc::restart 11v roam}");
        hmac_roam_start(hmac_vap, ROAM_SCAN_CHANNEL_ORG_5, OAL_TRUE, NULL, ROAM_TRIGGER_11V);
    }
}

void hmac_scan_set_scan_running_false_and_wake_up_waite_queue(hmac_vap_stru *hmac_vap,
    hmac_scan_stru *scan_mgmt)
{
    if (scan_mgmt != NULL) {
        scan_mgmt->en_is_scanning = OAL_FALSE;
    }
    if (hmac_vap != NULL) {
        oal_wait_queue_wake_up_interrupt(&(hmac_vap->query_wait_q));
    }
}

/*
 * 函 数 名  : hmac_scan_proc_scan_comp_event
 * 功能描述  : DMAC扫描完成事件处理
 * 1.日    期  : 2013年6月25日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_proc_scan_comp_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;
    mac_scan_rsp_stru *d2h_scan_rsp_info = NULL;
    hmac_scan_stru *scan_mgmt = NULL;

    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(event_hdr->uc_device_id);
    if (oal_unlikely(hmac_device == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    d2h_scan_rsp_info = (mac_scan_rsp_stru *)(event->auc_event_data);
    scan_mgmt = &(hmac_device->st_scan_mgmt);

    /*  防止compete事件和正在处理的扫描不一致 */
    hmac_vap = mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_scan_check_scan_comp_valid(hmac_vap, event_hdr, scan_mgmt, d2h_scan_rsp_info) == OAL_FALSE) {
        return OAL_SUCC;
    }
    if (!hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH)) {
        oam_warning_log1(event_hdr->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event::scan status:%d!}",
            d2h_scan_rsp_info->en_scan_rsp_status);
    }
    /* 删除扫描超时保护定时器 */
    hmac_scan_proc_scan_timer(scan_mgmt, d2h_scan_rsp_info);
    /* 扫描vap状态切换 */
    hmac_scan_change_vap_state(hmac_vap, scan_mgmt, d2h_scan_rsp_info);

    /* BEGIN: mp12 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    /* 根据device上报的扫描结果，上报sme */
    /* 将扫描执行情况(扫描执行成功、还是失败等返回结果)记录到扫描运行记录结构体中 */
    scan_mgmt->st_scan_record_mgmt.en_scan_rsp_status = d2h_scan_rsp_info->en_scan_rsp_status;
    scan_mgmt->st_scan_record_mgmt.ull_cookie = d2h_scan_rsp_info->ull_cookie;

    hmac_scan_print_scan_record_info(hmac_vap, &(scan_mgmt->st_scan_record_mgmt));

    /* 如果扫描回调函数不为空，则调用回调函数 ,abort也要通知内核防止内核死等 */
    if (scan_mgmt->st_scan_record_mgmt.p_fn_cb != NULL) {
        /* 终止扫描无需调用回调,防止终止扫描结束对随后发起PNO扫描的影响 */
        scan_mgmt->st_scan_record_mgmt.p_fn_cb(&(scan_mgmt->st_scan_record_mgmt));
    }

    /* 设置当前处于非扫描状态 */
    if (d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO) {
        /* PNO扫描没有置此位为OAL_TRUE,PNO扫描结束,不能影响随后的常规扫描 */
        hmac_scan_set_scan_running_false_and_wake_up_waite_queue(hmac_vap, scan_mgmt);
    }
    hmac_scan_comp_p2p_listen_proc(hmac_vap, d2h_scan_rsp_info);
    hmac_scan_comp_roam_proc(hmac_vap, scan_mgmt);
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_scan_proc_scan_req_event_exception
 * 功能描述  : 异常扫描请求,抛事件到wal 层，执行扫描完成
 * 1.日    期  : 2013年12月26日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_proc_scan_req_event_exception(hmac_vap_stru *hmac_vap, void *p_params)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_scan_rsp_stru scan_rsp;
    hmac_scan_rsp_stru *pst_scan_rsp = NULL;

    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, p_params))) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event_exception::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 不支持发起扫描的状态发起了扫描 */
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
        "{hmac_scan_proc_scan_req_event_exception::vap state is=%x.}", hmac_vap->st_vap_base_info.en_vap_state);

    memset_s(&scan_rsp, sizeof(hmac_scan_rsp_stru), 0, sizeof(hmac_scan_rsp_stru));

    /* 抛扫描完成事件到WAL, 执行SCAN_DONE , 释放扫描请求内存 */
    event_mem = frw_event_alloc_m(sizeof(hmac_scan_rsp_stru));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_req_event_exception::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    scan_rsp.uc_result_code = MAC_SCAN_REFUSED;
    /* When STA is roaming, scan req return success instead of failure,
       in case roaming failure which will cause UI scan list null  */
    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        scan_rsp.uc_result_code = MAC_SCAN_SUCCESS;
    }
    scan_rsp.uc_num_dscr = 0;

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_SCAN_COMP_STA,
                       sizeof(hmac_scan_rsp_stru), FRW_EVENT_PIPELINE_STAGE_0, hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    pst_scan_rsp = (hmac_scan_rsp_stru *)event->auc_event_data;

    if (memcpy_s(pst_scan_rsp, sizeof(hmac_scan_rsp_stru), (void *)(&scan_rsp), sizeof(hmac_scan_rsp_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_proc_scan_req_event_exception::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_set_sour_mac_addr_in_probe_req
 * 功能描述  : 设置probe req帧中携带的源mac addr，如果随机mac addr特性开启，则携带随机mac addr
 * 1.日    期  : 2015年5月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_scan_set_sour_mac_addr_in_probe_req(hmac_vap_stru *hmac_vap, uint8_t *sour_mac_addr,
    oal_bool_enum_uint8 en_is_rand_mac_addr_scan, oal_bool_enum_uint8 en_is_p2p0_scan)
{
    mac_device_stru *mac_device = NULL;
    hmac_device_stru *hmac_device = NULL;

    if (oal_any_null_ptr2(hmac_vap, sour_mac_addr)) {
        oam_error_log0(0, OAM_SF_CFG,
            "{hmac_scan_set_sour_mac_addr_in_probe_req::param null.}");
        return;
    }

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_scan_set_sour_mac_addr_in_probe_req::mac_device is null.}");
        return;
    }

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_scan_set_sour_mac_addr_in_probe_req::hmac_device is null. device_id %d}",
                         hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    /* WLAN/P2P 特性情况下，p2p0 和p2p-p2p0 cl 扫描时候，需要使用不同设备 */
    if (en_is_p2p0_scan == OAL_TRUE) {
        oal_set_mac_addr(sour_mac_addr, mac_mib_get_p2p0_dot11StationID(&hmac_vap->st_vap_base_info));
    } else {
        /* 如果随机mac addr扫描特性开启且非P2P场景，设置随机mac addr到probe req帧中 */
        if ((en_is_rand_mac_addr_scan == OAL_TRUE) && (is_legacy_vap(&(hmac_vap->st_vap_base_info))) &&
            ((hmac_device->st_scan_mgmt.random_mac_from_kernel == OAL_TRUE) ||
            (mac_device->auc_mac_oui[BYTE_OFFSET_0] != 0) || (mac_device->auc_mac_oui[BYTE_OFFSET_1] != 0) ||
            (mac_device->auc_mac_oui[BYTE_OFFSET_2] != 0))) {
            /* 更新随机mac 地址,使用下发随机MAC OUI 生成的随机mac 地址更新到本次扫描 */
            oal_set_mac_addr(sour_mac_addr, hmac_device->st_scan_mgmt.auc_random_mac);
        } else {
            /* 设置地址为自己的 */
            oal_set_mac_addr(sour_mac_addr, mac_mib_get_StationID(&hmac_vap->st_vap_base_info));
        }
    }

    return;
}
/*
 * 函 数 名  : hmac_scan_need_skip_channel
 * 功能描述  : 更新扫描信道列表，根据芯片工作带宽及软件配置
 * 1.日    期  : 2016年7月7日
  *   修改内容  : 新生成函数
 */
OAL_STATIC oal_bool_enum_uint8 hmac_scan_need_skip_channel(hmac_vap_stru *hmac_vap, uint8_t channel)
{
    wlan_channel_band_enum_uint8 en_band = mac_get_band_by_channel_num(channel);
    if (mac_chip_run_band(hmac_vap->st_vap_base_info.uc_chip_id, en_band) != OAL_TRUE) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*
 * 函 数 名  : hmac_scan_set_neighbor_report_scan_params
 * 功能描述  : 为neighbor report流程中的扫描设置参数
 * 1.日    期  : 2019年7月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_set_neighbor_report_scan_params(mac_scan_req_stru *scan_params)
{
    if (scan_params->uc_neighbor_report_process_flag == OAL_TRUE ||
        scan_params->uc_bss_transition_process_flag == OAL_TRUE) {
        scan_params->uc_scan_channel_interval = 2; /* 间隔2个信道，切回工作信道工作一段时间 */
        scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;
    }
    return;
}

OAL_STATIC oal_bool_enum_uint8 hmac_scan_get_random_mac_flag(hmac_vap_stru *hmac_vap,
                                                             oal_bool_enum_uint8 en_customize_random_mac_scan)
{
    /* 非P2P设备才配置随机mac扫描标志到device，并下发到DMAC */
    return (is_legacy_vap(&(hmac_vap->st_vap_base_info))) ? en_customize_random_mac_scan : OAL_FALSE;
}

static uint32_t hmac_scan_update_scan_mode(mac_scan_req_stru *scan_params,
    wlan_vap_mode_enum_uint8 en_vap_mode)
{
    if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (scan_params->en_scan_mode != WLAN_SCAN_MODE_ROAM_SCAN) {
            /* 修改扫描参数为sta的背景扫描 */
            scan_params->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_STA;
        }
    } else if (en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 修改扫描参数为sta的背景扫描 */
        scan_params->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_AP;
    } else {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_update_scan_params::vap mode[%d], bg scan failed.}", en_vap_mode);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/* 功能描述：sta+ap 共存场景，ap有用户时，减少切离时间，避免ap关联的用户被影响
 * 修改为每信道扫描一次，每扫描一个信道回AP工作信道
 */
void hmac_scan_set_scan_params_when_ap_up(mac_device_stru *mac_device, hmac_vap_stru *hmac_vap,
    mac_scan_req_stru *scan_params)
{
    uint32_t i;
    uint32_t up_vap_num;
    mac_vap_stru *mac_vap_tmp = NULL;
    mac_vap_stru *mac_vap_ap = NULL;
    mac_vap_stru *up_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = { NULL };

    up_vap_num = mac_device_find_up_vaps(mac_device, up_vap_array, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    /* 查找AP */
    for (i = 0; i < up_vap_num; i++) {
        mac_vap_tmp = up_vap_array[i];
        if (is_legacy_ap(mac_vap_tmp) == OAL_TRUE && mac_is_chba_mode(mac_vap_tmp) == OAL_FALSE) {
            mac_vap_ap = mac_vap_tmp;
        }
    }
    if (mac_vap_ap == NULL || mac_vap_ap->us_user_nums == 0) {
        return;
    }

    scan_params->uc_scan_channel_interval = 1;
    scan_params->uc_max_scan_count_per_channel = 1;
}

/*
 * 功能描述  : 修改扫描模式和信道扫描次数:根据是否存在up状态下的vap，如果是，则是背景扫描，如果不是，则是前景扫描
 * 日    期  : 2022年2月15日
 * 作    者  : wifi
 * 修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_scan_update_up_vap_scan_params(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params)
{
    uint32_t ret;
    wlan_vap_mode_enum_uint8 vap_mode;
    mac_vap_stru *mac_vap_temp = NULL;
    mac_device_stru *mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);

    ret = mac_device_find_up_vap(mac_device, &mac_vap_temp);
    if ((ret == OAL_SUCC) && (mac_vap_temp != NULL)) {
        /* 判断vap的类型，如果是sta则为sta的背景扫描，如果是ap，则是ap的背景扫描，其它类型的vap暂不支持背景扫描 */
        vap_mode = hmac_vap->st_vap_base_info.en_vap_mode;
        if (hmac_scan_update_scan_mode(scan_params, vap_mode) != OAL_SUCC) {
            return OAL_FAIL;
        }
        scan_params->en_need_switch_back_home_channel = OAL_TRUE;

        if (mac_device_calc_up_vap_num(mac_device) == 1 && !is_legacy_vap(mac_vap_temp) &&
            is_legacy_vap(&hmac_vap->st_vap_base_info)) {
            /*
             * 修改扫描信道间隔(2)和回工作信道工作时间(60ms):仅仅针对P2P处于关联状态，
             * wlan处于去关联状态,wlan发起的扫描
             */
            scan_params->uc_scan_channel_interval = MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE;
            scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;

            if ((scan_params->us_scan_time > WLAN_DEFAULT_ACTIVE_SCAN_TIME) &&
                (scan_params->en_scan_type == WLAN_SCAN_TYPE_ACTIVE)) {
                /* 指定SSID扫描超过3个,会修改每次扫描时间为40ms(默认是20ms) */
                /* P2P关联但wlan未关联场景,考虑到扫描时间增加对p2p wfd场景的影响,设置每信道扫描次数为1次(默认为2次) */
                scan_params->uc_max_scan_count_per_channel = 1;
            }
        } else {
            /* 携带隐藏SSID的情况下扫3个信道回一次home信道，其他情况默认扫描6个信道回home信道工作100ms */
            scan_params->uc_scan_channel_interval = (scan_params->uc_ssid_num > 1) ?
                MAC_SCAN_CHANNEL_INTERVAL_HIDDEN_SSID : MAC_SCAN_CHANNEL_INTERVAL_DEFAULT;
            scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
        }
        hmac_scan_set_neighbor_report_scan_params(scan_params);
        hmac_scan_set_scan_params_when_ap_up(mac_device, hmac_vap, scan_params);
    }
    return OAL_SUCC;
}


/*
 * 函 数 名  : hmac_scan_update_scan_params
 * 功能描述  : 根据device下所有的vap状态以及其它信息，更新扫描参数
 * 1.日    期  : 2015年2月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_scan_update_scan_params(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params,
                                                 oal_bool_enum_uint8 en_is_random_mac_addr_scan)
{
    mac_device_stru *mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    uint8_t loop;
    uint8_t chan_cnt = 0;

    if (mac_device == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_device_id, OAM_SF_SCAN,
                         "{hmac_scan_update_scan_params::mac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_HID2D
    /* HiD2D触发的扫描仅仅为信道测量扫描，不发送probe request，无需更新扫描参数 */
    if (scan_params->en_scan_mode == WLAN_SCAN_MODE_HID2D_SCAN) {
        return OAL_SUCC;
    }
#endif
    /* 1.记录发起扫描的vap id到扫描参数 */
    scan_params->uc_vap_id = hmac_vap->st_vap_base_info.uc_vap_id;
    scan_params->en_need_switch_back_home_channel = OAL_FALSE;

    if (scan_params->en_scan_mode != WLAN_SCAN_MODE_ROAM_SCAN) {
        scan_params->en_scan_mode = WLAN_SCAN_MODE_FOREGROUND;
    }

    /* 2.修改扫描模式和信道扫描次数: 根据是否存在up状态下的vap，如果是，则是背景扫描，如果不是，则是前景扫描 */
    if (hmac_scan_update_up_vap_scan_params(hmac_vap, scan_params) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 3.设置发送的probe req帧中源mac addr */
    scan_params->en_is_random_mac_addr_scan = hmac_scan_get_random_mac_flag(hmac_vap, en_is_random_mac_addr_scan);
    hmac_scan_set_sour_mac_addr_in_probe_req(hmac_vap, scan_params->auc_sour_mac_addr,
                                             en_is_random_mac_addr_scan, scan_params->bit_is_p2p0_scan);

    for (loop = 0; loop < scan_params->uc_channel_nums; loop++) {
        if (!hmac_scan_need_skip_channel(hmac_vap, scan_params->ast_channel_list[loop].uc_chan_number)) {
            if (chan_cnt != loop) {
                scan_params->ast_channel_list[chan_cnt] = scan_params->ast_channel_list[loop];
            }
            chan_cnt++;
        }
    }

    if (!chan_cnt) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_update_scan_params::channels trimed to none!, ori cnt=%d}",
                         scan_params->uc_channel_nums);

        return OAL_FAIL;
    }

    scan_params->uc_channel_nums = chan_cnt;
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_check_is_dispatch_scan_req
 * 功能描述  : 检测是否能够发起扫描，如果可以，则记录扫描请求者的信息，并清空上一次扫描结果
 * 1.日    期  : 2015年5月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_check_can_enter_scan_state(mac_vap_stru *mac_vap, oal_bool_enum_uint8 is_high_prio_scan)
{
    /* p2p有可能进行监听动作，但是和scan req的优先级一样，因此当上层发起的是扫描请求时，统一可使用下面的接口判断 */
    return hmac_check_can_enter_state(mac_vap, HMAC_FSM_INPUT_SCAN_REQ, is_high_prio_scan);
}

/*
 * 函 数 名  : hmac_scan_check_is_dispatch_scan_req
 * 功能描述  : 检测是否能够发起扫描，如果可以，则记录扫描请求者的信息，并清空上一次扫描结果
 * 1.日    期  : 2015年5月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_check_is_dispatch_scan_req(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    oal_bool_enum_uint8 is_high_prio_scan)
{
    uint32_t ret;

    if (hmac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1.先检测其它vap的状态从而判断是否可进入扫描状态，使得扫描尽量不打断其它的入网流程 */
    ret = hmac_scan_check_can_enter_scan_state(&(hmac_vap->st_vap_base_info), is_high_prio_scan);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_check_is_dispatch_scan_req::Because of err_code[%d], can't enter into scan state.}", ret);
        return ret;
    }

    /* 2.判断当前扫描是否正在执行 */
    if (hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_check_is_dispatch_scan_req::the scan request is rejected.}");
        return OAL_FAIL;
    }

    /* 3.判断当前是否正在执行漫游 */
    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_check_is_dispatch_scan_req:: roam reject new scan.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 4.判断当前是否正在执行CAC */
    if (hmac_cac_abort_scan_check(hmac_device) == OAL_SUCC) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_check_is_dispatch_scan_req:: cac abort scan.}");
        return OAL_FAIL;
    }
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_proc_last_scan_record
 * 功能描述  : 处理上一次的扫描记录，接口封装，从而便于可扩展(未来可能使用老化机制判断是否清除)
 * 1.日    期  : 2015年5月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_proc_last_scan_record(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    /* 本次扫描请求发起时，清除上一次扫描结果中过期的bss信息 */
    hmac_scan_clean_expire_scanned_bss(hmac_vap, &(hmac_device->st_scan_mgmt.st_scan_record_mgmt));

    return;
}

/*
 * 函 数 名  : hmac_scan_proc_scan_timeout_fn
 * 功能描述  : host侧抛扫描请求时间到device侧，防止因核间通信、抛事件等异常情况，host侧接收不到
 * 1.日    期  : 2015年5月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_scan_proc_scan_timeout_fn(void *arg)
{
    hmac_device_stru *hmac_device = (hmac_device_stru *)arg;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_scan_record_stru *scan_record = NULL;
    uint32_t pedding_data = 0;

    /* 获取扫描记录信息 */
    scan_record = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt);

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(scan_record->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(scan_record->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_timeout_fn::hmac_vap null.}");

        /* 扫描状态恢复为未在执行的状态 */
        hmac_scan_set_scan_running_false_and_wake_up_waite_queue(hmac_vap, &(hmac_device->st_scan_mgmt));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* 改变vap状态到SCAN_COMP */
            hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }
    }

    /* BEGIN: mp12 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (scan_record->en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        hmac_fsm_change_state(hmac_vap, scan_record->en_vap_last_state);
        scan_record->en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* END: mp12 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    /* 设置扫描响应状态为超时 */
    scan_record->en_scan_rsp_status = MAC_SCAN_TIMEOUT;
    oam_warning_log1(scan_record->uc_vap_id, OAM_SF_SCAN,
        "{hmac_scan_proc_scan_timeout_fn::scan time out cookie [%x].}", scan_record->ull_cookie);

    /* 如果扫描回调函数不为空，则调用回调函数 */
    if (scan_record->p_fn_cb != NULL) {
        oam_warning_log0(scan_record->uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_timeout_fn::scan callback func proc.}");
        scan_record->p_fn_cb(scan_record);
    }

    /* DMAC 超时未上报扫描完成，HMAC 下发扫描结束命令，停止DMAC 扫描 */
    hmac_config_scan_abort(&hmac_vap->st_vap_base_info, sizeof(uint32_t), (uint8_t *)&pedding_data);

    /* 扫描状态恢复为未在执行的状态 */
    hmac_scan_set_scan_running_false_and_wake_up_waite_queue(hmac_vap, &(hmac_device->st_scan_mgmt));

    chr_exception(chr_wifi_drv(CHR_WIFI_DRV_EVENT_SCAN, CHR_WIFI_DRV_ERROR_SCAN_TIMEOUT));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_proc_clear_last_scan_record
 * 功能描述  : 解决P2P listen被wlan扫描终止时，上报的bss个数较少的问题
 * 1.日    期  : 2019年10月31日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_scan_proc_clear_last_scan_record(mac_scan_req_stru *scan_params,
    hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    /* Begin：, 解决P2P listen被wlan扫描终止时，上报的bss个数较少的问题 */
    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        if (scan_params->uc_scan_func & MAC_SCAN_FUNC_P2P_LISTEN) {
            hmac_vap->bit_in_p2p_listen = OAL_TRUE;
        } else {
            hmac_vap->bit_in_p2p_listen = OAL_FALSE;
            /* 处理上一次扫描记录，目前直接清楚上一次结果，后续可能需要老化时间处理 */
            hmac_scan_proc_last_scan_record(hmac_vap, hmac_device);
        }
    } else {
        /* 处理上一次扫描记录，目前直接清楚上一次结果，后续可能需要老化时间处理 */
        hmac_scan_proc_last_scan_record(hmac_vap, hmac_device);
    }
    /* End： */
}
/*
 * 函 数 名  : hmac_scan_preproc_scan_req_event
 * 功能描述  : 扫描请求预处理(减少圈复杂度)
 * 1.日    期  : 2020年04月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_preproc_scan_req_event(hmac_vap_stru *hmac_vap, void *params,
    mac_scan_req_h2d_stru **ph2d_scan_req_params, mac_scan_req_stru **pscan_params, hmac_device_stru **phmac_device)
{
    oal_bool_enum_uint8 temp_flag;
    mac_scan_req_h2d_stru *h2d_scan_req_params = NULL; /* hmac发送到dmac的扫描请求参数 */
    mac_scan_req_stru *scan_params = NULL;
    hmac_device_stru *hmac_device = NULL;

    /* 参数合法性检查 */
    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, params))) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 扫描停止模块测试 */
    temp_flag = (((g_en_bgscan_enable_flag == HMAC_BGSCAN_DISABLE) &&
                  (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP)) ||
                  (g_en_bgscan_enable_flag == HMAC_SCAN_DISABLE));
    if (temp_flag) {
        oam_warning_log1(0, 0, "hmac_scan_proc_scan_req_event:bgscan_en_flag:%d", g_en_bgscan_enable_flag);
        return OAL_FAIL;
    }

    h2d_scan_req_params = (mac_scan_req_h2d_stru *)params;
    scan_params = &(h2d_scan_req_params->st_scan_params);

    /* 异常判断: 扫描的信道个数为0 */
    if (scan_params->uc_channel_nums == 0) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event::channel_nums=0.}");
        return OAL_FAIL;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_req_event::hmac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    *ph2d_scan_req_params = h2d_scan_req_params;
    *pscan_params = scan_params;
    *phmac_device = hmac_device;

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_pc_update_passive_scan_channel_param
 * 功能描述  : PC被动扫描信道设置
 * 修改历史  :
 * 1.日    期  : 2019年9月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
OAL_STATIC void hmac_scan_update_passive_scan_channel_param(mac_scan_req_ex_stru *scan_params,
                                                            mac_scan_channel_stru *mac_channel)
{
    mac_regdomain_info_stru *regdomain_info = NULL;

    /* 获取管制域全局变量 */
    mac_get_regdomain_info(&regdomain_info);
    if (regdomain_info == NULL) {
        return;
    }
    /* 特殊管制域 99 */
    if (((regdomain_info->ac_country[0] == '9') && (regdomain_info->ac_country[1] == '9')) ||
        ((regdomain_info->ac_country[0] == 'Z') && (regdomain_info->ac_country[1] == 'Z'))) {
        /* 2g特殊管制域信道执行主动扫描 */
        if (mac_channel->en_band == WLAN_BAND_2G) {
            if (mac_channel->uc_chan_number >= DMAC_CHANNEL12) { /* 12、13、14 信道被动扫描 */
                mac_channel->scan_policy = WLAN_SCAN_TYPE_PASSIVE;
                mac_channel->scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
            } else {
                mac_channel->scan_policy = WLAN_SCAN_TYPE_ACTIVE;
                mac_channel->scan_time = scan_params->us_scan_time;
            }
        } else {
            /* 5g特殊管制域信道执行被动扫描 */
            mac_channel->scan_policy = WLAN_SCAN_TYPE_PASSIVE;
            mac_channel->scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
        }
    }
}
#endif
OAL_STATIC void hmac_scan_update_dfs_channel_scan_param(mac_scan_req_ex_stru *scan_params,
                                                        mac_scan_channel_stru *mac_channel)
{
    mac_vap_stru *mac_vap = mac_res_get_mac_vap(scan_params->uc_vap_id);
    if (scan_params->uc_neighbor_report_process_flag == OAL_TRUE) {
        mac_channel->scan_policy = WLAN_SCAN_TYPE_ACTIVE;
        mac_channel->scan_time = scan_params->us_scan_time;
        return;
    }
    if (mac_vap != NULL && mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_channel->scan_policy = WLAN_SCAN_TYPE_PASSIVE;
        mac_channel->scan_time = scan_params->us_scan_time;
        return;
    }
    mac_channel->scan_policy = WLAN_SCAN_TYPE_HALF;
    mac_channel->scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
}
OAL_STATIC void hmac_scan_channel_convert_ex(mac_scan_req_ex_stru *scan_req_ex, mac_scan_req_stru *scan_params)
{
    uint8_t idx;
    if (scan_params->uc_channel_nums > WLAN_MAX_CHANNEL_NUM) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_channel_convert_ex::channel_nums[%d] invalid.}",
            scan_params->uc_channel_nums);
        return;
    }
    for (idx = 0; idx < scan_params->uc_channel_nums; idx++) {
        if (memcpy_s(&(scan_req_ex->ast_channel_list[idx]), sizeof(mac_scan_channel_stru),
            &(scan_params->ast_channel_list[idx]), sizeof(mac_channel_stru)) != EOK) {
            oam_error_log0(0, 0, "hmac_scan_channel_convert_ex::memcpy_s fail!");
        }
        scan_req_ex->ast_channel_list[idx].scan_policy = WLAN_SCAN_TYPE_ACTIVE;
        scan_req_ex->ast_channel_list[idx].scan_time = scan_params->us_scan_time;
        if (mac_is_dfs_channel(scan_req_ex->ast_channel_list[idx].en_band,
            scan_req_ex->ast_channel_list[idx].uc_chan_number) == OAL_TRUE) {
            hmac_scan_update_dfs_channel_scan_param(scan_req_ex, &(scan_req_ex->ast_channel_list[idx]));
        }
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
        /*
         * 国家码自学习定制化开关 国家码自学习功能仅当ini country code = 99,
         * self study = 1(99国家码自学习，所有逻辑在驱动)/2（ZZ国家码自学习，学习逻辑在上层，信道处理在驱动
         * 打开
         */
        if (wlan_chip_get_selfstudy_country_flag() != 0) {
            /* PC5G被动扫描信道设置 */
            hmac_scan_update_passive_scan_channel_param(scan_req_ex, &(scan_req_ex->ast_channel_list[idx]));
        }
#endif
    }
}
void hmac_scan_param_convert_ex(mac_scan_req_ex_stru *scan_req_ex, mac_scan_req_stru *scan_params)
{
        scan_req_ex->en_bss_type = scan_params->en_bss_type;
        scan_req_ex->en_scan_type = scan_params->en_scan_type;
        scan_req_ex->uc_bssid_num = scan_params->uc_bssid_num;
        scan_req_ex->uc_ssid_num = scan_params->uc_ssid_num;
        memcpy_s(scan_req_ex->auc_sour_mac_addr, WLAN_MAC_ADDR_LEN, scan_params->auc_sour_mac_addr, WLAN_MAC_ADDR_LEN);
        scan_req_ex->uc_p2p0_listen_channel = scan_params->uc_p2p0_listen_channel;
        scan_req_ex->uc_max_scan_count_per_channel = scan_params->uc_max_scan_count_per_channel;
        memcpy_s(scan_req_ex->auc_bssid, sizeof(scan_req_ex->auc_bssid),
                 scan_params->auc_bssid, sizeof(scan_params->auc_bssid));
        memcpy_s(scan_req_ex->ast_mac_ssid_set, sizeof(scan_req_ex->ast_mac_ssid_set),
                 scan_params->ast_mac_ssid_set, sizeof(scan_params->ast_mac_ssid_set));
        scan_req_ex->uc_max_send_probe_req_count_per_channel = scan_params->uc_max_send_probe_req_count_per_channel;
        scan_req_ex->bit_is_p2p0_scan = scan_params->bit_is_p2p0_scan;
        scan_req_ex->bit_is_radom_mac_saved = scan_params->bit_is_radom_mac_saved;
        scan_req_ex->bit_radom_mac_saved_to_dev = scan_params->bit_radom_mac_saved_to_dev;
        scan_req_ex->bit_desire_fast_scan = scan_params->bit_desire_fast_scan;
        scan_req_ex->bit_is_special_roc_type = scan_params->bit_is_special_roc_type;
        scan_req_ex->bit_rsv = scan_params->bit_rsv;
        scan_req_ex->en_abort_scan_flag = scan_params->en_abort_scan_flag;
        scan_req_ex->en_is_random_mac_addr_scan = scan_params->en_is_random_mac_addr_scan;
        scan_req_ex->en_need_switch_back_home_channel = scan_params->en_need_switch_back_home_channel;
        scan_req_ex->uc_scan_channel_interval = scan_params->uc_scan_channel_interval;
        scan_req_ex->us_work_time_on_home_channel = scan_params->us_work_time_on_home_channel;
        scan_req_ex->uc_channel_nums = scan_params->uc_channel_nums;
        scan_req_ex->uc_probe_delay = scan_params->uc_probe_delay;
        scan_req_ex->us_scan_time = scan_params->us_scan_time;
        scan_req_ex->en_scan_mode = scan_params->en_scan_mode;
        scan_req_ex->uc_resv = scan_params->uc_resv;
        scan_req_ex->uc_scan_func = scan_params->uc_scan_func;
        scan_req_ex->uc_vap_id = scan_params->uc_vap_id;
        scan_req_ex->ull_cookie = scan_params->ull_cookie;
        scan_req_ex->uc_neighbor_report_process_flag = scan_params->uc_neighbor_report_process_flag;
        scan_req_ex->uc_bss_transition_process_flag = scan_params->uc_bss_transition_process_flag;
        scan_req_ex->p_fn_cb = scan_params->p_fn_cb;
        hmac_scan_channel_convert_ex(scan_req_ex, scan_params);
}

static int32_t hmac_scan_param_update(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params)
{
    int32_t ret;
    /* 更新此次扫描请求的扫描参数 */
    if (scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
        /* 监听状态下不设置随机扫描，避免wlan0 监听状态下发送管理帧失败 */
        ret = (int32_t)hmac_scan_update_scan_params(hmac_vap, scan_params, OAL_FALSE);
    } else {
        /* 更新此次扫描请求的扫描参数 */
        ret = (int32_t)hmac_scan_update_scan_params(hmac_vap, scan_params, g_wlan_cust.uc_random_mac_addr_scan);
    }

    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_param_get_and_update::update scan mode failed, error[%d].}", ret);
        return ret;
    }

    return ret;
}

static void hmac_scan_update_scan_record(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params,
                                         hmac_device_stru *hmac_device)
{
    hmac_scan_record_stru *scan_record = NULL;
   /* 设置扫描模块处于扫描状态，其它扫描请求将丢弃 */
    hmac_device->st_scan_mgmt.en_is_scanning = OAL_TRUE;

    hmac_scan_proc_clear_last_scan_record(scan_params, hmac_vap, hmac_device);

    /* 记录扫描发起者的信息，某些模块回调函数使用 */
    scan_record = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt);
    scan_record->uc_chip_id = hmac_device->pst_device_base_info->uc_chip_id;
    scan_record->uc_device_id = hmac_device->pst_device_base_info->uc_device_id;
    scan_record->uc_vap_id = scan_params->uc_vap_id;
    scan_record->uc_chan_numbers = scan_params->uc_channel_nums;
    scan_record->p_fn_cb = scan_params->p_fn_cb;
    scan_record->en_scan_mode = scan_params->en_scan_mode;

    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_update_scan_record::save last vap_state:%d}", hmac_vap->st_vap_base_info.en_vap_state);

        scan_record->en_vap_last_state = hmac_vap->st_vap_base_info.en_vap_state;
    }

    scan_record->ull_cookie = scan_params->ull_cookie;

    /* 记录扫描开始时间 */
    scan_record->st_scan_start_time = oal_ktime_get();
}

static void hmac_scan_req_fsm_change_state(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params)
{
    oal_bool_enum_uint8 temp_flag;
    /* 如果发起扫描的vap的模式为sta，并且，其关联状态为非up状态，且非p2p监听状态，则切换其扫描状态 */
    temp_flag = ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
                 (scan_params->uc_scan_func != MAC_SCAN_FUNC_P2P_LISTEN) &&
                 (hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP));
    if (temp_flag) {
        /* 切换vap的状态为WAIT_SCAN状态 */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_WAIT_SCAN);
    }

    /* AP的启动扫描做特殊处理，当hostapd下发扫描请求时，VAP还处于INIT状态 */
    temp_flag = ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
                 (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_INIT));
    if (temp_flag) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_req_fsm_change_state::ap startup scan.}");
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_AP_WAIT_START);
    }
}

static void hmac_scan_req_event_scan_timeout_mechanism_set(mac_scan_req_stru *scan_params,
    hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    uint32_t scan_timeout;
    /******************************************************************/
    /* 打印扫描参数，测试使用 */
    /* 如果是P2P 发起监听，则设置HMAC 扫描超时时间为P2P 监听时间 */
    scan_timeout = (scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) ?
        (scan_params->us_scan_time * 2) : WLAN_DEFAULT_MAX_TIME_PER_SCAN; // 超时时间是扫描时间的2倍
    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        scan_timeout = (scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) ?
            /* 因为大屏板测试发现dmac切换信道需要约20ms时间，当wpa supplicant将p2p listen设置成10ms时 */
            /* 如果ul_scan_timeout只设置2 * 10ms，将导致dmac上报扫描结果前该定时器就超时，故设为4 * 10ms */
            (scan_params->us_scan_time * 4 + HMAC_P2P_LISTEN_EXTRA_WAIT_TIME) : WLAN_DEFAULT_MAX_TIME_PER_SCAN;
    }

    hmac_scan_print_scan_params(scan_params, &hmac_vap->st_vap_base_info);

    /* 启动扫描保护定时器，防止因拋事件、核间通信失败等情况下的异常保护，定时器初步的超时时间为4.5秒 */
    frw_timer_create_timer_m(&(hmac_device->st_scan_mgmt.st_scan_timeout), hmac_scan_proc_scan_timeout_fn,
        scan_timeout, hmac_device, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_device->pst_device_base_info->core_id);
}
static void hmac_scan_proc_scan_req_11v_scan_reject_process(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params)
{
    hmac_roam_info_stru *roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;

    if (roam_info != NULL && scan_params->uc_bss_transition_process_flag == OAL_TRUE) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_req_11v_scan_reject_process::11v scan reject}");
        roam_info->roam_11v_scan_reject = OAL_TRUE;
        hmac_roam_main_del_timer(roam_info);
        hmac_roam_main_clear(roam_info);
    }
}
/*
 * 函 数 名  : hmac_scan_proc_scan_req_event
 * 功能描述  : 处理扫描请求的总入口
 * 1.日    期  : 2015年2月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_proc_scan_req_event(hmac_vap_stru *hmac_vap, void *params)
{
    frw_event_mem_stru *event_mem = NULL;
    mac_scan_req_h2d_stru *h2d_scan_req_params = NULL; /* hmac发送到dmac的扫描请求参数 */
    mac_scan_req_stru *scan_params = NULL;
    hmac_device_stru *hmac_device = NULL;
    oal_netbuf_stru *netbuf_scan_req = NULL;
    uint32_t ret;

    ret = hmac_scan_preproc_scan_req_event(hmac_vap, params, &h2d_scan_req_params, &scan_params, &hmac_device);
    if (ret != OAL_SUCC) {
        return ret;
    }

    if (hmac_device->en_monitor_mode == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_scan_proc_scan_req_event_etc: in sniffer monitor mode, scan abort!}");
        return -OAL_EINVAL;
    }

    ret = (uint32_t)hmac_scan_param_update(hmac_vap, scan_params);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 检测是否符合发起扫描请求的条件，如果不符合，直接返回 */
    ret = hmac_scan_check_is_dispatch_scan_req(hmac_vap, hmac_device, scan_params->bit_is_special_roc_type);
    if (ret != OAL_SUCC) {
        /*  扫描被拒绝必须恢复到原来的状态 */
        if (scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
            hmac_vap_state_restore(hmac_vap);
        }
        hmac_scan_proc_scan_req_11v_scan_reject_process(hmac_vap, scan_params);
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event::Because of error[%d], can't dispatch scan req.}", ret);
        return ret;
    }

    hmac_scan_update_scan_record(hmac_vap, scan_params, hmac_device);

    /* 抛扫描请求事件到DMAC, 申请event事件内存 */
    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_req_event::alloc memory failed.}");
        /* 恢复扫描状态为非运行状态 */
        hmac_scan_set_scan_running_false_and_wake_up_waite_queue(hmac_vap, &(hmac_device->st_scan_mgmt));
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_scan_req_fsm_change_state(hmac_vap, scan_params);

    ret = wlan_chip_scan_req_alloc_and_fill_netbuf(event_mem, hmac_vap, &netbuf_scan_req, params);
    if (ret != OAL_SUCC) {
        frw_event_free_m(event_mem);
        return ret;
    }

    hmac_scan_req_event_scan_timeout_mechanism_set(scan_params, hmac_device, hmac_vap);

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);

    oal_netbuf_free(netbuf_scan_req);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_proc_sched_scan_req_event
 * 功能描述  : 处理pno调度扫描请求的入口
 * 1.日    期  : 2015年6月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_proc_sched_scan_req_event(hmac_vap_stru *hmac_vap, void *params)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_scan_record_stru *scan_record = NULL;
    mac_pno_scan_stru *pno_scan_params = NULL;
    dmac_tx_memaddr_stru *event_addr = NULL;
    uint32_t ret;

    /* 参数合法性检查 */
    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, params))) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_sched_scan_req_event::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pno_scan_params = (mac_pno_scan_stru *)params;

    /* 判断PNO调度扫描下发的过滤的ssid个数小于等于0 */
    if (pno_scan_params->l_ssid_count <= 0) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_sched_scan_req_event::ssid_count <=0.}");
        return OAL_FAIL;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_sched_scan_req_event::hmac_device[%d] null.}", hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 检测是否符合发起扫描请求的条件，如果不符合，直接返回 */
    ret = hmac_scan_check_is_dispatch_scan_req(hmac_vap, hmac_device, OAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_sched_scan_req_event::Because of error[%d], can't dispatch scan req.}", ret);
        return ret;
    }

    /* 清空上一次的扫描结果 */
    hmac_scan_proc_last_scan_record(hmac_vap, hmac_device);

    /* 记录扫描发起者的信息，某些模块回调函数使用 */
    scan_record = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt);
    scan_record->uc_chip_id = hmac_device->pst_device_base_info->uc_chip_id;
    scan_record->uc_device_id = hmac_device->pst_device_base_info->uc_device_id;
    scan_record->uc_vap_id = hmac_vap->st_vap_base_info.uc_vap_id;
    scan_record->p_fn_cb = pno_scan_params->p_fn_cb;
    scan_record->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_PNO;

    /* 抛扫描请求事件到DMAC, 申请事件内存 */
    event_mem = frw_event_alloc_m(sizeof(mac_pno_scan_stru *));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_sched_scan_req_event::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCHED_SCAN_REQ,
        sizeof(mac_pno_scan_stru *), FRW_EVENT_PIPELINE_STAGE_1, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    /* 事件data域内携带PNO扫描请求参数 */
    event_addr = (dmac_tx_memaddr_stru *)frw_get_event_payload(event_mem);
    event_addr->memaddr = (unsigned long *)pno_scan_params;
    event_addr->us_mem_len = sizeof(mac_pno_scan_stru);

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_process_chan_result_event
 * 功能描述  : 处理dmac上报的信道扫描结果处理DMAC上报的信道测量结果
 * 1.日    期  : 2014年11月27日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_process_chan_result_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    hmac_device_stru *hmac_device = NULL;
    dmac_crx_chan_result_stru *chan_result_param = NULL;
    hmac_scan_record_stru *scan_record = NULL;
    uint8_t scan_idx;

    /* 获取事件信息 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);
    chan_result_param = (dmac_crx_chan_result_stru *)(event->auc_event_data);
    scan_idx = chan_result_param->uc_scan_idx;

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(event_hdr->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_process_chan_result_event::hmac_device is null.}");
        return OAL_FAIL;
    }

    scan_record = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt);

    /* 检查上报的索引是否合法 */
    if (scan_idx >= scan_record->uc_chan_numbers) {
        /* dmac上报的扫描结果超出了要扫描的信道个数 */
        oam_warning_log2(0, OAM_SF_SCAN,
            "{hmac_scan_process_chan_result_event::result from dmac error! scan_idx[%d], chan_numbers[%d].}",
            scan_idx, scan_record->uc_chan_numbers);

        return OAL_FAIL;
    }
    scan_record->ast_chan_results[scan_idx] = chan_result_param->st_chan_result;
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_scan_init
 * 功能描述  : 扫描模块初始化
 * 1.日    期  : 2015年5月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_scan_init(hmac_device_stru *hmac_device)
{
    hmac_scan_stru *scan_mgmt = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;

    /* 初始化扫描管理结构体信息 */
    scan_mgmt = &(hmac_device->st_scan_mgmt);
    memset_s(scan_mgmt, sizeof(hmac_scan_stru), 0, sizeof(hmac_scan_stru));
    scan_mgmt->en_is_scanning = OAL_FALSE;
    scan_mgmt->st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;

    /* 初始化bss管理结果链表和锁 */
    bss_mgmt = &(scan_mgmt->st_scan_record_mgmt.st_bss_mgmt);
    oal_dlist_init_head(&(bss_mgmt->st_bss_list_head));
    oal_spin_lock_init(&(bss_mgmt->st_lock));

    /* 初始化内核下发扫描request资源锁 */
    oal_spin_lock_init(&(scan_mgmt->st_scan_request_spinlock));

    /* 初始化 st_wiphy_mgmt 结构 */
    oal_wait_queue_init_head(&(scan_mgmt->st_wait_queue));

    /* 初始化扫描生成随机MAC 地址 */
    oal_random_ether_addr(hmac_device->st_scan_mgmt.auc_random_mac, WLAN_MAC_ADDR_LEN);
    return;
}

/*
 * 函 数 名  : hmac_scan_exit
 * 功能描述  : hmac扫描模块退出
 * 1.日    期  : 2015年5月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_scan_exit(hmac_device_stru *hmac_device)
{
    hmac_scan_stru *scan_mgmt = NULL;

    /* 清空扫描记录信息 */
    hmac_scan_clean_scan(&(hmac_device->st_scan_mgmt));

    if (hmac_device->st_scan_mgmt.st_init_scan_timeout.en_is_registerd) {
        frw_timer_immediate_destroy_timer_m(&hmac_device->st_scan_mgmt.st_init_scan_timeout);
    }

    /* 清除扫描管理结构体信息 */
    scan_mgmt = &(hmac_device->st_scan_mgmt);
    memset_s(scan_mgmt, sizeof(hmac_scan_stru), 0, sizeof(hmac_scan_stru));
    scan_mgmt->en_is_scanning = OAL_FALSE;
    return;
}
/*
 * 函 数 名  : hmac_bgscan_enable
 * 功能描述  : 终止扫描模块测试
 * 1.日    期  : 2015年10月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_bgscan_enable(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *param)
{
    hmac_scan_state_enum pen_bgscan_enable_flag;

    pen_bgscan_enable_flag = *param; /* 背景扫描停止使能位 */

    /* 背景扫描停止命令 */
    switch (pen_bgscan_enable_flag) {
        case HMAC_BGSCAN_DISABLE:
            g_en_bgscan_enable_flag = HMAC_BGSCAN_DISABLE;
            break;
        case HMAC_BGSCAN_ENABLE:
            g_en_bgscan_enable_flag = HMAC_BGSCAN_ENABLE;
            break;
        case HMAC_SCAN_DISABLE:
            g_en_bgscan_enable_flag = HMAC_SCAN_DISABLE;
            break;
        default:
            g_en_bgscan_enable_flag = HMAC_BGSCAN_ENABLE;
            break;
    }

    oam_warning_log1(0, OAM_SF_SCAN, "hmac_bgscan_enable: g_en_bgscan_enable_flag=%d.", g_en_bgscan_enable_flag);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_scan_start_dbac
 * 功能描述  : 在HMAC启动DBAC
 * 1.日    期  : 2015年5月29日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_scan_start_dbac(mac_device_stru *dev)
{
    uint8_t auc_cmd[NUM_32_BYTES];
    uint16_t us_len;
    uint32_t ret = OAL_FAIL;
    uint8_t uc_idx;
#define DBAC_START_STR     " dbac start"
#define DBAC_START_STR_LEN 11
    mac_vap_stru *mac_vap = NULL;

    mac_ioctl_alg_config_stru *alg_config = (mac_ioctl_alg_config_stru *)auc_cmd;

    if (memcpy_s(auc_cmd + sizeof(mac_ioctl_alg_config_stru), sizeof(auc_cmd) - sizeof(mac_ioctl_alg_config_stru),
        (const int8_t *)DBAC_START_STR, 11) != EOK) { // " dbac start" 长度11
        oam_error_log0(0, OAM_SF_CFG, "hmac_scan_start_dbac::memcpy fail!");
        return OAL_FAIL;
    }
    auc_cmd[sizeof(mac_ioctl_alg_config_stru) + DBAC_START_STR_LEN] = 0;

    alg_config->uc_argc = 2; // 2: dbac start 参数数目
    alg_config->auc_argv_offset[0] = 1;
    alg_config->auc_argv_offset[1] = 6; // 6: 配置参数
    for (uc_idx = 0; uc_idx < dev->uc_vap_num; uc_idx++) {
        mac_vap = mac_res_get_mac_vap(dev->auc_vap_id[uc_idx]);
        if (mac_vap != NULL && mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            break;
        }
    }
    if (mac_vap != NULL) {
        us_len = sizeof(mac_ioctl_alg_config_stru) + DBAC_START_STR_LEN + 1;
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_ALG, us_len, auc_cmd);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_alg::send_event failed[%d].}", ret);
        }
        oal_io_print("start dbac\n");
    } else {
        oal_io_print("no vap found to start dbac\n");
    }

    return ret;
}
/*
 * 函 数 名  : hmac_init_scan_cancel_timer
 * 功能描述  : 清除扫描超时定时器
 * 1.日    期  : 2015年5月25日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_init_scan_cancel_timer(hmac_device_stru *hmac_dev)
{
    if (hmac_dev != NULL && hmac_dev->st_scan_mgmt.st_init_scan_timeout.en_is_registerd) {
        /* 关闭超时定时器 */
        frw_timer_immediate_destroy_timer_m(&hmac_dev->st_scan_mgmt.st_init_scan_timeout);
    }

    return OAL_SUCC;
}

