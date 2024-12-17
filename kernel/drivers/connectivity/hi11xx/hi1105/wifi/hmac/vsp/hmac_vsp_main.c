/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : vsp低时延丢包流程
 * 作者       : wifi
 * 创建日期   : 2020年12月24日
 */

#include "oal_schedule.h"
#include "hmac_vsp_if.h"
#include "hmac_vsp_source.h"
#include "hmac_vsp_sink.h"
#include "hmac_vsp_test.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VSP_MAIN_C

#ifdef _PRE_WLAN_FEATURE_VSP
static void hmac_vsp_cfg_param(hmac_vsp_cmd *cmd, vsp_param *param)
{
    param->slice_cnt = cmd->slice_cnt;
    param->layer_cnt = cmd->layer_cnt;
    param->max_payload = cmd->max_payload;
    param->header_len = cmd->header_len;
    oal_set_mac_addr(param->sa, cmd->sa);
    oal_set_mac_addr(param->ra, cmd->ra);
    param->feedback_pkt_len = param->layer_cnt * 2 + sizeof(vsp_feedback_frame); // 2: 每个layer填写2字节succ_num信息

    /* msec to usec */
    param->tmax = cmd->tmax * HMAC_MS_TO_US;
    param->tslice = cmd->tslice * HMAC_MS_TO_US;
    param->tfeedback = cmd->tfeedback * HMAC_MS_TO_US;

    hmac_vsp_log((uint8_t *)param, VSP_LOG_LVL_0, VSP_LOG_DATATYPE_PARAM, VSP_LOG_TYPE_CFG_PARAM);
}

uint32_t hmac_vsp_cfg(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, hmac_vsp_cmd *cmd)
{
    vsp_param param = { 0 };

    switch (cmd->mode) {
        case VSP_MODE_SINK:
            hmac_vsp_cfg_param(cmd, &param);
            return hmac_vsp_sink_cfg(hmac_vap, hmac_user, &param);
        case VSP_MODE_SOURCE:
            hmac_vsp_cfg_param(cmd, &param);
            return hmac_vsp_source_cfg(hmac_vap, hmac_user, &param);
        default:
            oam_error_log1(0, 0, "{hmac_vsp_cfg::mode[%d] invalid}", cmd->mode);
    }

    return OAL_FAIL;
}

uint32_t hmac_vsp_stop(hmac_user_stru *hmac_user)
{
    hmac_vsp_info_stru *vsp_info = hmac_user->vsp_hdl;

    if (!vsp_info || !vsp_info->enable) {
        return OAL_FAIL;
    }

    switch (vsp_info->mode) {
        case VSP_MODE_SINK:
            hmac_vsp_sink_stop();
            break;
        case VSP_MODE_SOURCE:
            hmac_vsp_source_stop();
            break;
        default:
            oam_error_log1(0, 0, "{hmac_vsp_stop::vsp mode[%d] error!}", vsp_info->mode);
    }

    hmac_user->vsp_hdl = NULL;

    return OAL_SUCC;
}

uint32_t hmac_vsp_start(hmac_user_stru *hmac_user)
{
    hmac_vsp_info_stru *vsp_info = hmac_user->vsp_hdl;

    if (!vsp_info || vsp_info->enable) {
        return OAL_FAIL;
    }

    vsp_info->enable = OAL_TRUE;

    return OAL_SUCC;
}

/*
 * 功能描述  : vsp tx通用定时器超时
 * 1.日    期  : 2021年1月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_vsp_common_timeout(frw_event_mem_stru *event_mem)
{
    hmac_vsp_info_stru *vsp_info = NULL;
    hal_host_common_timerout_stru *timeout_event;
    timeout_event = (hal_host_common_timerout_stru*)frw_get_event_stru(event_mem)->auc_event_data;
    if (timeout_event->timer == NULL) {
        oam_error_log0(0, 0, "{hmac_vsp_common_timeout::timer ptr is null.}");
        return OAL_FAIL;
    }
    vsp_info = oal_container_of(timeout_event->timer, hmac_vsp_info_stru, timer);
    if (vsp_info->mode != VSP_MODE_SINK) {
        return OAL_SUCC;
    }

    /* 若下个slice已开始rx, 此处stop timer, 可能会有问题? */
    vsp_info->timer_running = OAL_FALSE;
    hmac_vsp_sink_rx_slice_timeout((hmac_vsp_sink_stru *)vsp_info);
    /* 处理report帧,实际接口要在rx done时上报 */
    unref_param(event_mem);
    return OAL_SUCC;
}

static void hmac_vsp_rx_amsdu_proc(hmac_vsp_info_stru *vsp_info, oal_netbuf_stru *netbuf)
{
    switch (vsp_info->mode) {
        case VSP_MODE_SINK:
            hmac_vsp_sink_rx_amsdu_proc(netbuf);
            break;
        case VSP_MODE_SOURCE:
            hmac_vsp_source_rx_amsdu_proc(netbuf);
            break;
        default:
            hmac_rx_netbuf_list_free(netbuf);
    }
}

uint32_t hmac_vsp_rx_proc(hmac_host_rx_context_stru *rx_context, oal_netbuf_stru *netbuf)
{
    hmac_vsp_info_stru *vsp_info = NULL;

    if (oal_unlikely(rx_context->hmac_user == NULL)) {
        return VSP_RX_REFUSE;
    }

    vsp_info = rx_context->hmac_user->vsp_hdl;
    if (vsp_info == NULL || !vsp_info->enable) {
        return VSP_RX_REFUSE;
    }

    if (mac_get_rx_cb_tid((mac_rx_ctl_stru *)oal_netbuf_cb(netbuf)) != WLAN_TIDNO_VSP) {
        return VSP_RX_REFUSE;
    }

    hmac_vsp_rx_amsdu_proc(vsp_info, netbuf);

    return VSP_RX_ACCEPT;
}

static uint32_t hmac_vsp_debug_link_cmd_parse(hmac_vsp_debug_cmd *debug_cmd, hmac_vsp_cmd *vsp_cmd)
{
    vsp_cmd->slice_cnt = debug_cmd->param[VSP_PARAM_SLICES_PER_FRAME];
    if (!vsp_cmd->slice_cnt) {
        oam_error_log0(0, 0, "{hmac_vsp_start_cmd_parse::error slice_cnt[0]!}");
        return OAL_FAIL;
    }

    vsp_cmd->layer_cnt = debug_cmd->param[VSP_PARAM_LAYER_NUM];
    vsp_cmd->tmax = debug_cmd->param[VSP_PARAM_MAX_TRANSMIT_DLY];
    vsp_cmd->tslice = debug_cmd->param[VSP_PARAM_VSYNC_INTERVAL] / vsp_cmd->slice_cnt;
    vsp_cmd->tfeedback = debug_cmd->param[VSP_PARAM_MAX_FEEDBACK_DLY];
    vsp_cmd->mode = debug_cmd->mode;

    hmac_vsp_log((uint8_t *)vsp_cmd, VSP_LOG_LVL_0, VSP_LOG_DATATYPE_CMD, VSP_LOG_TYPE_PARSE_CMD);

    return OAL_SUCC;
}

static uint32_t hmac_vsp_debug_link_enable_cmd_proc(
    mac_vap_stru *mac_vap, hmac_user_stru *hmac_user, hmac_vsp_debug_cmd *debug_cmd)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    hmac_vsp_cmd vsp_cmd = { 0 };

    if (!hmac_vap) {
        return OAL_FAIL;
    }

    if (hmac_vsp_debug_link_cmd_parse(debug_cmd, &vsp_cmd) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (hmac_vsp_cfg(hmac_vap, hmac_user, &vsp_cmd) != OAL_SUCC) {
        return OAL_FAIL;
    }

    return hmac_vsp_start(hmac_user);
}

static uint32_t hmac_vsp_debug_link_dsiable_cmd_proc(hmac_user_stru *hmac_user)
{
    return hmac_vsp_stop(hmac_user);
}

uint32_t hmac_vsp_debug_link_cmd_proc(mac_vap_stru *mac_vap, hmac_vsp_debug_cmd *debug_cmd)
{
    uint16_t user_idx;
    hmac_user_stru *hmac_user = NULL;

    if (mac_vap_find_user_by_macaddr(mac_vap, debug_cmd->user_mac, &user_idx) != OAL_SUCC) {
        oam_error_log0(0, 0, "{hmac_vsp_start_cmd_proc::find user by mac failed!}");
        return OAL_FAIL;
    }

    hmac_user = mac_res_get_hmac_user(user_idx);
    if (!hmac_user) {
        oam_error_log1(0, 0, "{hmac_vsp_start_cmd_proc::get user[%d] failed!}", user_idx);
        return OAL_FAIL;
    }

    if (debug_cmd->link_en) {
        return hmac_vsp_debug_link_enable_cmd_proc(mac_vap, hmac_user, debug_cmd);
    } else {
        return hmac_vsp_debug_link_dsiable_cmd_proc(hmac_user);
    }
}

uint32_t hmac_vsp_debug_test_cmd_proc(hmac_vsp_debug_cmd *cmd)
{
    if (cmd->test_en) {
        return hmac_vsp_test_start(cmd);
    } else {
        return hmac_vsp_test_stop();
    }
}

uint32_t hmac_vsp_debug_cmd_proc(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    hmac_vsp_debug_cmd *debug_cmd = (hmac_vsp_debug_cmd *)param;

    if (debug_cmd->type == VSP_CMD_LINK_EN) {
        return hmac_vsp_debug_link_cmd_proc(mac_vap, debug_cmd);
    } else if (debug_cmd->type == VSP_CMD_TEST_EN) {
        return hmac_vsp_debug_test_cmd_proc(debug_cmd);
    }

    return OAL_FAIL;
}
#endif
