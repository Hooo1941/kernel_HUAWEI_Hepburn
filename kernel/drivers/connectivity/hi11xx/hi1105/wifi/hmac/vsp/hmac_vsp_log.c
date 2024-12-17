/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : vsp维测
 * 作    者 : wifi
 * 创建日期 : 2022年8月24日
 */

#include "hmac_vsp_if.h"
#include "hmac_vsp_source.h"
#include "hmac_vsp_sink.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VSP_LOG_C

#ifdef _PRE_WLAN_FEATURE_VSP
static void hmac_vsp_tx_result_log(uint8_t *data)
{
    uint8_t layer_idx;
    uint16_t layer_succ_num;
    send_result *tx_result = (send_result *)data;

    oam_warning_log3(0, 0, "{hmac_vsp_tx_result_log::slice_info[0x%x] layer0_max[%d] layerall_max[%d]}",
        tx_result->slice_info, tx_result->slice_layer0_max, tx_result->slice_layerall_max);

    for (layer_idx = 0; layer_idx < MAX_LAYER_NUM; layer_idx++) {
        layer_succ_num = tx_result->layer_suc_num[layer_idx] & 0x7FFF;
        if (!layer_succ_num) {
            continue;
        }
        oam_warning_log2(0, 0, "{hmac_vsp_tx_result_log::layer_idx[%d] layer_suc_num[%d]}", layer_idx, layer_succ_num);
    }
}

static void hmac_vsp_slice_log(uint8_t *data)
{
    hmac_vsp_tx_slice_stru *slice = (hmac_vsp_tx_slice_stru *)data;

    oam_warning_log4(0, 0, "{hmac_vsp_slice_log::frame[%d] slice[%d] arrive_ts[%d] vsync_ts[%d]}",
        slice->frame_id, slice->slice_id, slice->arrive_ts, slice->vsync_ts);
    hmac_vsp_tx_result_log((uint8_t *)&slice->tx_result);
}

static void hmac_vsp_layer_log(uint8_t *data)
{
    tx_layer_ctrl *layer = (tx_layer_ctrl *)data;

    oam_warning_log4(0, 0, "{hmac_vsp_layer_log::layer[%d] pkt[%d] qos_type[0x%x] version[%d]}",
        layer->layer_number, layer->paket_number, layer->qos_type, layer->version);
    oam_warning_log2(0, 0, "{hmac_vsp_layer_log::normal_pack_length[%d] last_paket_len[%d]}",
        layer->layer_number, layer->paket_number);
    oam_warning_log3(0, 0, "{hmac_vsp_layer_log::RA[%02X:XX:XX:XX:%02X:%02X]}",
        layer->mac_ra_address[MAC_ADDR_0], layer->mac_ra_address[MAC_ADDR_4], layer->mac_ra_address[MAC_ADDR_5]);
    oam_warning_log3(0, 0, "{hmac_vsp_layer_log::SA[%02X:XX:XX:XX:%02X:%02X]}",
        layer->mac_sa_address[MAC_ADDR_0], layer->mac_sa_address[MAC_ADDR_4], layer->mac_sa_address[MAC_ADDR_5]);
}

static void hmac_vsp_pkt_log(uint8_t *data)
{
    vsp_msdu_hdr_stru *msdu = (vsp_msdu_hdr_stru *)data;

    oam_warning_log4(0, 0, "{hmac_vsp_pkt_log::frame[%d] slice[%d] layer[0x%x] pkt[%d]}",
        msdu->frame_id, msdu->slice_num, msdu->layer_num, msdu->number);
    oam_warning_log4(0, 0, "{hmac_vsp_pkt_log::last[%d] len[%d] cfg_pkt[0x%x] timestamp[%d]}",
        msdu->last, msdu->len, msdu->cfg_pkt, msdu->timestamp);

    oam_warning_log3(0, 0, "{hmac_vsp_pkt_log::RA[%02X:XX:XX:XX:%02X:%02X]}",
        msdu->ra[MAC_ADDR_0], msdu->ra[MAC_ADDR_4], msdu->ra[MAC_ADDR_5]);
    oam_warning_log3(0, 0, "{hmac_vsp_pkt_log::SA[%02X:XX:XX:XX:%02X:%02X]}",
        msdu->sa[MAC_ADDR_0], msdu->sa[MAC_ADDR_4], msdu->sa[MAC_ADDR_5]);
}

static void hmac_vsp_feedback_log(uint8_t *data)
{
    vsp_feedback_frame *feedback = (vsp_feedback_frame *)data;

    oam_warning_log3(0, 0, "{hmac_vsp_feedback_log::frame[%d] slice[%d] total_layer[0x%x]}",
        feedback->frm_id, feedback->slice_id, feedback->layer_num);

    oam_warning_log3(0, 0, "{hmac_vsp_feedback_log::RA[%02X:XX:XX:XX:%02X:%02X]}",
        feedback->ra[MAC_ADDR_0], feedback->ra[MAC_ADDR_4], feedback->ra[MAC_ADDR_5]);
    oam_warning_log3(0, 0, "{hmac_vsp_feedback_log::SA[%02X:XX:XX:XX:%02X:%02X]}",
        feedback->sa[MAC_ADDR_0], feedback->sa[MAC_ADDR_4], feedback->sa[MAC_ADDR_5]);
}

static void hmac_vsp_rx_slice_priv_log(uint8_t *data)
{
    vsp_rx_slice_priv_stru *priv = (vsp_rx_slice_priv_stru *)data;

    oam_warning_log4(0, 0, "{hmac_vsp_rx_slice_log::frame[%d] slice[%d] alloc_ts[%d] vsync_ts[%d]}",
        priv->frm_id, priv->slice_id, priv->alloc_ts, priv->vsync_ts);
}

static void hmac_vsp_rx_slice_log(uint8_t *data)
{
    uint8_t layer_idx;
    rx_slice_mgmt *slice = (rx_slice_mgmt *)data;

    hmac_vsp_rx_slice_priv_log(slice->priv[0]);
    oam_warning_log2(0, 0, "{hmac_vsp_rx_slice_log::fix_len[%d] offset[%d]}", slice->fix_len, slice->payload_offset);

    for (layer_idx = 0; layer_idx < MAX_LAYER_NUM; layer_idx++) {
        oam_warning_log4(0, 0, "{hmac_vsp_rx_slice_log::layer_idx[%d] last_len[%d], packet_num[%d], rx_comp[%d]}",
            layer_idx, slice->last_len[layer_idx], slice->packet_num[layer_idx],
            hmac_vsp_sink_rx_layer_complete(slice, layer_idx));
    }
}

static void hmac_vsp_timer_log(uint8_t *data)
{
    hal_mac_common_timer *timer = (hal_mac_common_timer *)data;

    oam_warning_log4(0, 0, "{hmac_vsp_timer_log::timer_id[%d] enabled[%d] start_time[%d] timeout[%d]}",
        timer->timer_id, timer->timer_en, timer->start_time, timer->common_timer_val);
}

static void hmac_vsp_param_log(uint8_t *data)
{
    vsp_param *param = (vsp_param *)data;

    oam_warning_log3(0, 0, "{hmac_vsp_param_log::slice_cnt[%d] layer_cnt[%d] feedback_pkt_len[%d]}",
        param->slice_cnt, param->layer_cnt, param->feedback_pkt_len);
    oam_warning_log3(0, 0, "{hmac_vsp_param_log::tmax[%d] tslice[%d] tfeedback[%d]}",
        param->tmax, param->tslice, param->tfeedback);
}

static void hmac_vsp_cmd_log(uint8_t *data)
{
    hmac_vsp_cmd *vsp_cmd = (hmac_vsp_cmd *)data;

    oam_warning_log3(0, 0, "{hmac_vsp_debug_link_cmd_parse::mode[%d] slice_cnt[%d] layer_cnt[%d]}",
        vsp_cmd->mode, vsp_cmd->slice_cnt, vsp_cmd->layer_cnt);
    oam_warning_log3(0, 0, "{hmac_vsp_debug_link_cmd_parse::tmax[%d] tslice[%d] tfeedback[%d]}",
        vsp_cmd->tmax, vsp_cmd->tslice, vsp_cmd->tfeedback);
}

typedef void (*hmac_vsp_log_func)(uint8_t *);
hmac_vsp_log_func g_vsp_log_func[VSP_LOG_DATATYPE_BUTT] = {
    hmac_vsp_slice_log,          /* VSP_LOG_DATATYPE_SLICE */
    hmac_vsp_layer_log,          /* VSP_LOG_DATATYPE_LAYER */
    hmac_vsp_pkt_log,            /* VSP_LOG_DATATYPE_PKT */
    hmac_vsp_feedback_log,       /* VSP_LOG_DATATYPE_FEEDBACK */
    hmac_vsp_rx_slice_log,       /* VSP_LOG_DATATYPE_RX_SLICE */
    hmac_vsp_rx_slice_priv_log,  /* VSP_LOG_DATATYPE_RX_SLICE_PRIV */
    hmac_vsp_timer_log,          /* VSP_LOG_DATATYPE_TIMER */
    hmac_vsp_param_log,          /* VSP_LOG_DATATYPE_PARAM */
    hmac_vsp_cmd_log,            /* VSP_LOG_DATATYPE_CMD */
};

uint8_t g_vsp_log_lvl = VSP_LOG_LVL_DISABLE;

static void hmac_vsp_log_context_print(uint8_t type)
{
    switch (type) {
        case VSP_LOG_TYPE_PARSE_CMD:
            oam_warning_log0(0, 0, "VSP::parse cmd");
            break;
        case VSP_LOG_TYPE_CFG_PARAM:
            oam_warning_log0(0, 0, "VSP::cfg param");
            break;
        case VSP_LOG_TYPE_START_TIMER:
            oam_warning_log0(0, 0, "VSP::timer start");
            break;
        case VSP_LOG_TYPE_BUILD_SLICE:
            oam_warning_log0(0, 0, "VSP::build new slice");
            break;
        case VSP_LOG_TYPE_SEND_PKT:
            oam_warning_log0(0, 0, "VSP::sending pkt");
            break;
        case VSP_LOG_TYPE_BUILD_RX_SLICE:
            oam_warning_log0(0, 0, "VSP::alloc rx slice");
            break;
        case VSP_LOG_TYPE_INIT_RX_TIMER:
            oam_warning_log0(0, 0, "VSP::rx timer init");
            break;
        case VSP_LOG_TYPE_FILL_RX_SLICE:
            oam_warning_log0(0, 0, "VSP::fill rx slice");
            break;
        case VSP_LOG_TYPE_RX_EXPIRED_PKT:
            oam_warning_log0(0, 0, "VSP::rx expired pkt");
            break;
        case VSP_LOG_TYPE_RX_NONCURRENT_PKT:
            oam_warning_log0(0, 0, "VSP::rx noncurrent pkt");
            break;
        case VSP_LOG_TYPE_RX_FEEDBACK:
            oam_warning_log0(0, 0, "VSP::rx feedback");
            break;
        case VSP_LOG_TYPE_SEND_FEEDBACK:
            oam_warning_log0(0, 0, "VSP::send feedback");
            break;
        case VSP_LOG_TYPE_RPT_SLICE:
            oam_warning_log0(0, 0, "VSP::rpt slice");
            break;
        default:
            break;
    }
}

void hmac_vsp_log(uint8_t *data, uint8_t lvl, uint8_t datatype, uint8_t type)
{
    if (g_vsp_log_lvl < lvl || !data || datatype >= VSP_LOG_DATATYPE_BUTT || type >= VSP_LOG_TYPE_BUTT) {
        return;
    }

    hmac_vsp_log_context_print(type);

    if (g_vsp_log_func[datatype]) {
        g_vsp_log_func[datatype](data);
    }
}

uint32_t hmac_vsp_set_log_lvl(mac_vap_stru *mac_vap, uint32_t *params)
{
    g_vsp_log_lvl = (uint8_t)params[0];

    oam_warning_log1(0, 0, "{hmac_vsp_set_debug_lvl::log lvl set to [%d]}", g_vsp_log_lvl);

    return OAL_SUCC;
}

#endif
