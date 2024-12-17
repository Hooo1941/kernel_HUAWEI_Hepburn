/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : UAPSD hmac层处理
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#include "mac_vap.h"
#include "mac_frame.h"
#include "hmac_mgmt_ap.h"
#include "plat_pm_wlan.h"
#include "hmac_tid_sche.h"
#include "hmac_tx_data.h"
#include "hmac_rx_data.h"
#include "hmac_dyn_pcie.h"
#include "hmac_uapsd.h"
#include "hmac_config.h"
#include "securec.h"
#include "hmac_fsm.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_exception_rst.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_PM_C
uint8_t g_wlan_device_pm_switch = OAL_TRUE;  // device 低功耗开关
uint8_t g_wlan_ps_mode = 1;
uint8_t g_wlan_fast_ps_dyn_ctl = 0;  // app layer dynamic ctrl enable
uint8_t g_wlan_min_fast_ps_idle = 1;
uint8_t g_wlan_max_fast_ps_idle = 10;
uint8_t g_wlan_auto_ps_screen_on = 5;
uint8_t g_wlan_auto_ps_screen_off = 5;

static uint8_t hmac_is_trx_busy(void)
{
    uint8_t is_busy_flag = OAL_FALSE;
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();

    if (oal_warn_on(wlan_pm_info == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "hmac_is_trx_busy: wlan_pm is null \n");
        return is_busy_flag;
    }

    if (hmac_ring_tx_enabled() != OAL_TRUE) {
        return is_busy_flag;
    }

    if (hmac_is_tid_empty() == OAL_FALSE) {
        wlan_pm_info->tid_not_empty_cnt++;
        is_busy_flag = OAL_TRUE;
    }

    if (oal_atomic_read(&g_tid_schedule_list.ring_mpdu_num) != 0) {
        wlan_pm_info->ring_has_mpdu_cnt++;
        is_busy_flag = OAL_TRUE;
    }

    /* Host DDR接收不允许平台睡眠,低流量切device接收 */
    if (hal_master_is_in_ddr_rx() == OAL_TRUE) {
        wlan_pm_info->master_ddr_rx_cnt++;
        is_busy_flag = OAL_TRUE;
    }
    if (hal_slave_is_in_ddr_rx() == OAL_TRUE) {
        wlan_pm_info->slave_ddr_rx_cnt++;
        is_busy_flag = OAL_TRUE;
    }
    if (hal_is_host_forbid_sleep() == OAL_TRUE) {
        wlan_pm_info->host_forbid_sleep_cnt++;
        is_busy_flag = OAL_TRUE;
    }
    /* 常发常收不允许平台睡眠 */
    if (hal_device_is_al_trx() == OAL_TRUE) {
        is_busy_flag = OAL_TRUE;
    }
    return is_busy_flag;
}

void hmac_refresh_vap_pm_pause_cnt(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 *uc_is_any_cnt_exceed_limit,
                                   oal_bool_enum_uint8 *uc_is_any_timer_registerd)
{
    if (hmac_vap->st_ps_sw_timer.en_is_registerd == OAL_TRUE) {
        *uc_is_any_timer_registerd = OAL_TRUE;
        hmac_vap->us_check_timer_pause_cnt++;
        /* 低功耗pause计数 1000 次 打印输出一次 */
        if (hmac_vap->us_check_timer_pause_cnt % 1000 == 0) {
            oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                "{hmac_pm_pause_count_handle:: g_uc_check_timer_cnt[%d]",
                hmac_vap->us_check_timer_pause_cnt);
        }

        if (hmac_vap->us_check_timer_pause_cnt > HMAC_SWITCH_STA_PSM_MAX_CNT) {
            *uc_is_any_cnt_exceed_limit = OAL_TRUE;
            oam_error_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                "{hmac_pm_pause_count_handle::sw ps timer cnt too large[%d]> max[%d]}",
                hmac_vap->us_check_timer_pause_cnt, HMAC_SWITCH_STA_PSM_MAX_CNT);
        }
    } else {
        if (hmac_vap->us_check_timer_pause_cnt != 0) {
            oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                "{hmac_get_pm_pause_func::g_uc_check_timer_cnt end[%d],max[%d]",
                hmac_vap->us_check_timer_pause_cnt, HMAC_SWITCH_STA_PSM_MAX_CNT);
        }

        hmac_vap->us_check_timer_pause_cnt = 0;
    }
}

STATIC uint8_t hmac_check_dfr_status(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct st_exception_info *pst_exception_data = get_exception_info_reference();

    if (oal_work_is_busy(&pst_exception_data->wifi_excp_worker) ||
        oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker)) {
        return OAL_TRUE;
    }
#endif
    return OAL_FALSE;
}

oal_bool_enum_uint8 hmac_get_pm_pause_func(void)
{
    hmac_device_stru *pst_hmac_device = NULL;

    uint8_t uc_vap_idx;
    hmac_vap_stru *hmac_vap = NULL;
    oal_bool_enum_uint8 uc_is_any_cnt_exceed_limit = OAL_FALSE;
    oal_bool_enum_uint8 uc_is_any_timer_registerd = OAL_FALSE;

    if (hmac_check_dfr_status() == OAL_TRUE) {
        return OAL_TRUE;
    }

    /* 获取mac device结构体指针 */
    pst_hmac_device = hmac_res_get_mac_dev(0);
    if (pst_hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_get_pm_pause_func::pst_device null.}");
        return OAL_FALSE;
    }
    if (pst_hmac_device->pst_device_base_info->pm_work_disable == OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_get_pm_pause_func::pm_work_disable[%d]}",
            pst_hmac_device->pst_device_base_info->pm_work_disable);
        return OAL_TRUE;
    }
    if (hmac_is_trx_busy() == OAL_TRUE) {
        return OAL_TRUE;
    }

    if (pst_hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) {
        return OAL_TRUE;
    }

    if (hmac_get_pcie_switch_func() == HMAC_PCIE_SWITCH_UP) {
        return OAL_FALSE;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_hmac_device->pst_device_base_info->uc_vap_num; uc_vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_hmac_device->pst_device_base_info->auc_vap_id[uc_vap_idx]);
        if (hmac_vap == NULL) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_get_pm_pause_func::pst_hmac_vap null.}");
            return OAL_FALSE;
        }
        uc_is_any_cnt_exceed_limit = OAL_FALSE;
        uc_is_any_timer_registerd = OAL_FALSE;
        hmac_refresh_vap_pm_pause_cnt(hmac_vap, &uc_is_any_cnt_exceed_limit, &uc_is_any_timer_registerd);
        if (uc_is_any_timer_registerd && !uc_is_any_cnt_exceed_limit) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

void hmac_register_pm_callback(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

    struct wifi_srv_callback_handler *pst_wifi_srv_handler;

    pst_wifi_srv_handler = wlan_pm_get_wifi_srv_handler();
    if (pst_wifi_srv_handler != NULL) {
        pst_wifi_srv_handler->p_wifi_srv_get_pm_pause_func = hmac_get_pm_pause_func;
        pst_wifi_srv_handler->p_wifi_srv_open_notify = hmac_wifi_state_notify;
        pst_wifi_srv_handler->p_wifi_srv_pm_state_notify = hmac_wifi_pm_state_notify;
        pst_wifi_srv_handler->p_wifi_srv_pcie_switch_func = hmac_pcie_callback_fsm_func;
    } else {
        oal_io_print("hmac_register_pm_callback:wlan_pm_get_wifi_srv_handler is null\n");
    }

#endif
}
