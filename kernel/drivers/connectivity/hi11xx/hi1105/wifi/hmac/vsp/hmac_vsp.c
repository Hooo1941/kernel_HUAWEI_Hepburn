/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : vsp低时延丢包流程
 * 作者       : wifi
 * 创建日期   : 2020年12月24日
 */

#include "hmac_vsp.h"
#include "hmac_vsp_buf.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_tx_ring_alloc.h"
#include "hmac_config.h"
#include "hmac_scan.h"
#include "hmac_tid_ring_switch.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VSP_C

#ifdef _PRE_WLAN_FEATURE_VSP
#ifndef CONFIG_VCODEC_VSP_SUPPORT
static void wifi_tx_pkg_done(send_result *send_result);
static void wifi_rx_slice_done(rx_slice_mgmt *rx_slice_mgmt);
static rx_slice_mgmt *alloc_vdec_slice_buffer(uint32_t size);
#endif

static const hmac_vsp_vcodec_ops g_default_ops = {
    .alloc_slice_mgmt = alloc_vdec_slice_buffer,
    .rx_slice_done = wifi_rx_slice_done,
    .wifi_tx_pkg_done = wifi_tx_pkg_done,
};

const hmac_vsp_vcodec_ops *g_vsp_vcodec_ops = &g_default_ops;
hmac_vsp_info_stru *g_vsp_info = NULL;

void hmac_vsp_set_current_handle(hmac_vsp_info_stru *vsp_info)
{
    g_vsp_info = vsp_info;
}

hmac_vsp_info_stru *hmac_vsp_get_current_handle(void)
{
    return g_vsp_info;
}

static uint32_t hmac_vsp_set_status(mac_vap_stru *mac_vap, uint8_t mode, uint16_t user_index)
{
    mac_vsp_cfg_stru vsp_cfg = {
        .user_index = user_index,
        .mode = mode,
    };

    return hmac_config_send_event(mac_vap, WLAN_CFGID_VSP_STATUS_CHANGE, sizeof(mac_vsp_cfg_stru), (uint8_t *)&vsp_cfg);
}

static uint32_t hmac_vsp_init_common_timer(hmac_vsp_info_stru *vsp_info)
{
    hal_mac_common_timer *timer = &vsp_info->timer;

    timer->timer_en = OAL_TRUE;
    timer->timer_mask_en = OAL_TRUE;
    timer->timer_unit = HAL_COMMON_TIMER_UNIT_1US;
    timer->timer_mode = HAL_COMMON_TIMER_MODE_ONE_SHOT;
    timer->func = NULL;

    if (hal_host_init_common_timer(timer) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_TX, "hmac_vsp_tx_info_init :: init common timer[%d] fail", timer->timer_id);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

static void hmac_vsp_deinit_common_timer(hmac_vsp_info_stru *vsp_info)
{
    hal_mac_common_timer *timer = &vsp_info->timer;

    timer->timer_en = OAL_FALSE;
    hal_host_set_mac_common_timer(timer);
}

static void hmac_vsp_clear_tx_ring(hmac_tid_info_stru *tid_info)
{
    hmac_msdu_info_ring_stru *tx_ring = &tid_info->tx_ring;

    mutex_lock(&tx_ring->tx_lock);
    mutex_lock(&tx_ring->tx_comp_lock);

    hmac_wait_tx_ring_empty(tid_info);
    hmac_tx_ring_release_all_netbuf(tx_ring);

    mutex_unlock(&tx_ring->tx_lock);
    mutex_unlock(&tx_ring->tx_comp_lock);
}

typedef struct {
    hmac_scan_state_enum scan_state;
    mac_cfg_ps_open_stru pm_state;
} hmac_vsp_config_stru;

static void hmac_vsp_config_sta(mac_vap_stru *mac_vap, hmac_vsp_config_stru *param)
{
    if (!is_legacy_sta(mac_vap)) {
        return;
    }

    hmac_config_set_sta_pm_on(mac_vap, 0, (uint8_t *)&param->pm_state);
    hmac_bgscan_enable(mac_vap, 0, (uint8_t *)&param->scan_state);
}

static void hmac_vsp_stop_config_sta(hmac_vsp_info_stru *vsp_info)
{
    hmac_vsp_config_stru param = {
        .scan_state = HMAC_BGSCAN_ENABLE,
        .pm_state = {
            .uc_pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_CMD,
            .uc_pm_enable = MAC_STA_PM_SWITCH_ON,
        }
    };

    hmac_vsp_config_sta(&vsp_info->hmac_vap->st_vap_base_info, &param);
}

/* 现阶段为保证投屏时延与稳定性, 需要关闭背景扫描与协议低功耗 */
static void hmac_vsp_start_config_sta(hmac_vsp_info_stru *vsp_info)
{
    hmac_vsp_config_stru param = {
        .scan_state = HMAC_BGSCAN_DISABLE,
        .pm_state = {
            .uc_pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_CMD,
            .uc_pm_enable = MAC_STA_PM_SWITCH_OFF,
        }
    };

    hmac_vsp_config_sta(&vsp_info->hmac_vap->st_vap_base_info, &param);
}

void hmac_vsp_context_init(hmac_vsp_info_stru *vsp_info, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_vsp_start_config_sta(vsp_info);
    hmac_vsp_clear_tx_ring(vsp_info->tid_info);
    hmac_tid_ring_switch(vsp_info->hmac_user, vsp_info->tid_info, HOST_RING_TX_MODE);
    hmac_tx_ba_setup(hmac_vap, hmac_user, WLAN_TIDNO_VSP);
    hmac_user->vsp_hdl = vsp_info;
}

void hmac_vsp_context_deinit(hmac_vsp_info_stru *vsp_info)
{
    hmac_vsp_clear_tx_ring(vsp_info->tid_info);
    hmac_vsp_stop_config_sta(vsp_info);
}

uint32_t hmac_vsp_init(
    hmac_vsp_info_stru *vsp_info, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, vsp_param *param, uint8_t mode)
{
    mac_vap_stru *mac_vap = &hmac_vap->st_vap_base_info;
    hal_host_device_stru *hal_device = hal_get_host_device(mac_vap->uc_device_id);

    if (!hal_device) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_vsp_init::hal_device NULL}");
        return OAL_FAIL;
    }

    vsp_info->hmac_user = hmac_user;
    vsp_info->hmac_vap = hmac_vap;
    vsp_info->tid_info = &hmac_user->tx_tid_info[WLAN_TIDNO_VSP];
    vsp_info->host_hal_device = hal_device;
    vsp_info->mode = mode;

    if (hmac_vsp_init_common_timer(vsp_info) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (memcpy_s(&vsp_info->param, sizeof(vsp_param), param, sizeof(vsp_param)) != EOK) {
        return OAL_FAIL;
    }

    vsp_info->timer_running = OAL_FALSE;
    hmac_vsp_set_status(mac_vap, mode, hmac_user->st_user_base_info.us_assoc_id);
    hmac_vsp_set_vcodec_ops(&g_default_ops);

    oam_warning_log3(0, 0, "{hmac_vsp_init::VSP[%d] init SUCC, vap idx[%d], user idx[%d]!}",
        mode, mac_vap->uc_vap_id, hmac_user->st_user_base_info.us_assoc_id);

    return OAL_SUCC;
}

void hmac_vsp_deinit(hmac_vsp_info_stru *vsp_info)
{
    if (oal_any_null_ptr3(vsp_info, vsp_info->hmac_vap, vsp_info->hmac_user)) {
        oam_error_log0(0, 0, "{hmac_vsp_deinit::vsp deinit failed due to null ptr!}");
        return;
    }

    vsp_info->enable = OAL_FALSE;
    hmac_vsp_deinit_common_timer(vsp_info);
    hmac_vsp_set_status(
        &vsp_info->hmac_vap->st_vap_base_info, VSP_MODE_DISABLED, vsp_info->hmac_user->st_user_base_info.us_assoc_id);
    vsp_info->hmac_vap = NULL;
    vsp_info->hmac_user = NULL;
    vsp_info->host_hal_device = NULL;
}

uint32_t hmac_vsp_get_timestamp(hmac_vsp_info_stru *vsp_info)
{
    uint32_t tsf = 0;

    if (oal_any_null_ptr3(vsp_info, vsp_info->host_hal_device, vsp_info->hmac_vap)) {
        return 0;
    }

    return hal_host_get_tsf_lo(vsp_info->host_hal_device, vsp_info->hmac_vap->hal_vap_id, &tsf) == OAL_SUCC ? tsf : 0;
}

void hmac_vsp_start_timer(hmac_vsp_info_stru *vsp_info, uint32_t start_time, uint32_t timeout_us)
{
    if (!vsp_info) {
        oam_error_log0(0, 0, "{hmac_vsp_start_timer::vsp timer start failed!}");
        return;
    }

    oal_warn_on(vsp_info->timer_running);
    vsp_info->timer.start_time = start_time;
    vsp_info->timer.common_timer_val = timeout_us;
    vsp_info->timer.timer_en = OAL_TRUE;
    hal_host_set_mac_common_timer(&vsp_info->timer);
    vsp_info->timer_running = OAL_TRUE;
}

void hmac_vsp_stop_timer(hmac_vsp_info_stru *vsp_info)
{
    if (!vsp_info) {
        oam_error_log0(0, 0, "{hmac_vsp_stop_timer::vsp timer stop failed!}");
        return;
    }

    vsp_info->timer.timer_en = OAL_FALSE;
    hal_host_set_mac_common_timer(&vsp_info->timer);
    vsp_info->timer_running = OAL_FALSE;
}

static uint32_t hmac_vsp_calc_slice_timeout(hmac_vsp_info_stru *vsp_info, uint8_t slice_id, uint32_t cur)
{
    uint32_t consume = hmac_vsp_calc_runtime(vsp_info->timer_ref_vsync, cur); // 当前slice已经run的时间
    uint32_t target = vsp_info->param.tmax + slice_id * vsp_info->param.tslice; // 计算当前slice最大允许时间(相对vsync而言)
    uint32_t timeout;

    if (consume < target) {
        timeout = target - consume; // 当前slice的剩余时间作为超时时间
        /* min timeout for a slice 100us */
        if (oal_warn_on(timeout < 100)) {
            timeout = vsp_info->param.tslice;
        }
    } else {
        oam_error_log2(0, 0, "{hmac_vsp_calc_slice_timeout::slice[%d] consume[%d] is lager than target transmit dly}",
            slice_id, consume);
        timeout = vsp_info->param.tslice;
    }

    return timeout;
}

void hmac_vsp_set_slice_timeout(hmac_vsp_info_stru *vsp_info, uint8_t slice_id)
{
    uint32_t cur;

    if (!vsp_info) {
        oam_error_log0(0, 0, "{hmac_vsp_stop_timer::vsp timer stop failed!}");
        return;
    }

    cur = hmac_vsp_get_timestamp(vsp_info);
    hmac_vsp_start_timer(vsp_info, cur, hmac_vsp_calc_slice_timeout(vsp_info, slice_id, cur));

    hmac_vsp_log((uint8_t *)&vsp_info->timer, VSP_LOG_LVL_1, VSP_LOG_DATATYPE_TIMER, VSP_LOG_TYPE_START_TIMER);
}

void hmac_vsp_set_vcodec_ops(const hmac_vsp_vcodec_ops *ops)
{
    g_vsp_vcodec_ops = ops;
}

#ifndef CONFIG_VCODEC_VSP_SUPPORT
/* wifi call venc to send the result */
static void wifi_tx_pkg_done(send_result *send_result)
{
}

/* wifi call vdec to send slice buffer */
static void wifi_rx_slice_done(rx_slice_mgmt *rx_slice_mgmt)
{
}

/* wifi call vdec alloc buffer for wifi */
static rx_slice_mgmt *alloc_vdec_slice_buffer(uint32_t size)
{
    return NULL;
}
#endif
#endif
