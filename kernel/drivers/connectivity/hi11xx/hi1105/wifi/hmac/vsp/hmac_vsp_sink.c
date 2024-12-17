/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : vsp低时延丢包流程
 * 作者       : wifi
 * 创建日期   : 2020年12月24日
 */

#include "hmac_vsp_sink.h"
#include "hmac_host_rx.h"
#include "mac_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VSP_SINK_C

#ifdef _PRE_WLAN_FEATURE_VSP
hmac_vsp_sink_stru g_vsp_sink = {
    .vsp_base.timer.timer_id = 16,
};

static void hmac_vsp_sink_init_rx_slice_priv(
    vsp_rx_slice_priv_stru *priv, hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr, rx_slice_mgmt *rx_slice)
{
    memset_s(priv, sizeof(vsp_rx_slice_priv_stru), 0, sizeof(vsp_rx_slice_priv_stru));
    oal_netbuf_head_init(&priv->rx_queue);
    priv->frm_id = vsp_hdr->frame_id;
    priv->slice_id = vsp_hdr->slice_num;
    priv->owner = sink;
    priv->alloc_ts = hmac_vsp_get_timestamp(&sink->vsp_base);
    priv->vsync_vld = vsp_hdr->ts_type == VSP_TIMESTAMP_WIFI;
    priv->vsync_ts = vsp_hdr->timestamp;

    rx_slice->priv[0] = priv;
}

static void hmac_vsp_sink_init_rx_slice(
    vsp_rx_slice_priv_stru *priv, hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr, rx_slice_mgmt *rx_slice)
{
    rx_slice->fix_len = vsp_hdr->len;
    rx_slice->payload_offset = sizeof(vsp_msdu_hdr_stru);
    hmac_vsp_sink_init_rx_slice_priv(priv, sink, vsp_hdr, rx_slice);
}

static rx_slice_mgmt *hmac_vsp_sink_alloc_rx_slice(hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr)
{
    rx_slice_mgmt *rx_slice = vsp_vcodec_alloc_slice_mgmt(sizeof(rx_slice_mgmt));
    vsp_rx_slice_priv_stru *priv = NULL;

    if (!rx_slice) {
        oam_error_log0(0, 0, "{hmac_vsp_alloc_rx_slice_mgmt::slice buffer alloc failed!}");
        return NULL;
    }

    memset_s(rx_slice, sizeof(rx_slice_mgmt), 0, sizeof(rx_slice_mgmt));

    priv = oal_memalloc(sizeof(vsp_rx_slice_priv_stru));
    if (!priv) {
        oam_error_log0(0, 0, "{hmac_vsp_alloc_rx_slice_mgmt::alloc private data failed!}");
        return rx_slice;
    }

    hmac_vsp_sink_init_rx_slice(priv, sink, vsp_hdr, rx_slice);
    hmac_vsp_sink_inc_slice_total(sink);
    hmac_vsp_log((uint8_t *)priv, VSP_LOG_LVL_0, VSP_LOG_DATATYPE_RX_SLICE_PRIV, VSP_LOG_TYPE_BUILD_RX_SLICE);

    return rx_slice;
}

static inline uint8_t hmac_vsp_sink_rx_pkt_len_invalid(vsp_msdu_hdr_stru *vsp_hdr, oal_netbuf_stru *netbuf)
{
    return oal_netbuf_len(netbuf) < sizeof(vsp_msdu_hdr_stru) + vsp_hdr->len;
}

static inline uint8_t hmac_vsp_sink_rx_pkt_hdr_invalid(vsp_msdu_hdr_stru *vsp_hdr, hmac_vsp_info_stru *vsp_info)
{
    return vsp_hdr->number >= MAX_PAKET_NUM || vsp_hdr->layer_num >= MAX_LAYER_NUM ||
           vsp_hdr->slice_num >= vsp_info->param.slice_cnt;
}

static uint8_t hmac_vsp_sink_rx_pkt_invalid(
    hmac_vsp_info_stru *vsp_info, vsp_msdu_hdr_stru *vsp_hdr, oal_netbuf_stru *netbuf)
{
    return hmac_vsp_sink_rx_pkt_len_invalid(vsp_hdr, netbuf) || hmac_vsp_sink_rx_pkt_hdr_invalid(vsp_hdr, vsp_info);
}

static inline void hmac_vsp_sink_init_timer_conext(hmac_vsp_sink_stru *sink, vsp_rx_slice_priv_stru *priv)
{
    sink->next_timeout_frm = priv->frm_id;
    sink->next_timeout_slice = priv->slice_id;
    sink->vsp_base.timer_ref_vsync = priv->vsync_ts;
    hmac_vsp_log((uint8_t *)priv, VSP_LOG_LVL_1, VSP_LOG_DATATYPE_RX_SLICE_PRIV, VSP_LOG_TYPE_INIT_RX_TIMER);
}

static inline void hmac_vsp_sink_start_rx_slice_timer(hmac_vsp_sink_stru *sink, vsp_rx_slice_priv_stru *priv)
{
    hmac_vsp_sink_init_timer_conext(sink, priv);
    if (sink->vsp_base.timer_running) {
        hmac_vsp_stop_timer(&sink->vsp_base);
    }
    hmac_vsp_set_slice_timeout(&sink->vsp_base, priv->slice_id);
}

static inline uint32_t hmac_vsp_sink_get_pkt_rx_slice_index(hmac_vsp_info_stru *vsp_info, vsp_msdu_hdr_stru *vsp_hdr)
{
    return vsp_hdr->frame_id * vsp_info->param.slice_cnt + vsp_hdr->slice_num;
}

static rx_slice_mgmt *hmac_vsp_sink_get_pkt_rx_slice(hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr)
{
    uint32_t index = hmac_vsp_sink_get_pkt_rx_slice_index(&sink->vsp_base, vsp_hdr);
    rx_slice_mgmt *rx_slice = hmac_vsp_sink_get_rx_slice_from_tbl(sink, index);

    if (oal_unlikely(!rx_slice)) {
        rx_slice = hmac_vsp_sink_alloc_rx_slice(sink, vsp_hdr);
        if (rx_slice) {
            hmac_vsp_sink_set_rx_slice_to_tbl(sink, index, rx_slice);
            hmac_vsp_sink_start_rx_slice_timer(sink, hmac_vsp_sink_get_rx_slice_priv(rx_slice));
        }
    }

    return rx_slice;
}

static inline void hmac_vsp_sink_update_rx_slice_last_pkt_info(
    rx_slice_mgmt *rx_slice, vsp_rx_slice_priv_stru *priv, vsp_msdu_hdr_stru *vsp_hdr)
{
    if (oal_likely(!vsp_hdr->last)) {
        return;
    }

    priv->total_pkts[vsp_hdr->layer_num] = vsp_hdr->number + 1;
    rx_slice->last_len[vsp_hdr->layer_num] = vsp_hdr->len;
}

static inline void hmac_vsp_sink_update_rx_slice_non_last_pkt_info(rx_slice_mgmt *rx_slice, vsp_msdu_hdr_stru *vsp_hdr)
{
    if (oal_unlikely(vsp_hdr->last || vsp_hdr->len <= rx_slice->fix_len)) {
        return;
    }

    rx_slice->fix_len = vsp_hdr->len;
}

static uint32_t hmac_vsp_sink_update_rx_slice_pkt_info(
    rx_slice_mgmt *rx_slice, vsp_rx_slice_priv_stru *priv, vsp_msdu_hdr_stru *vsp_hdr)
{
    if (oal_unlikely(hmac_vsp_sink_get_rx_slice_pkt_addr(rx_slice, vsp_hdr->layer_num, vsp_hdr->number))) {
        oam_warning_log3(0, OAM_SF_ANY, "{hmac_vsp_rx_slice_process::slice[%d] layer[%d] paket[%d] DUPLICATE!}",
            vsp_hdr->slice_num, vsp_hdr->layer_num, vsp_hdr->number);
        return OAL_FAIL;
    }

    hmac_vsp_sink_update_rx_slice_pkt_addr(rx_slice, vsp_hdr->layer_num, vsp_hdr->number, (uintptr_t)vsp_hdr);
    hmac_vsp_sink_update_rx_slice_last_pkt_info(rx_slice, priv, vsp_hdr);
    hmac_vsp_sink_update_rx_slice_non_last_pkt_info(rx_slice, vsp_hdr);

    return OAL_SUCC;
}

static uint8_t hmac_vsp_sink_rx_slice_complete(hmac_vsp_info_stru *vsp_info, rx_slice_mgmt *rx_slice)
{
    uint8_t layer_id;

    for (layer_id = 0; layer_id < vsp_info->param.layer_cnt; layer_id++) {
        if (!hmac_vsp_sink_rx_layer_complete(rx_slice, layer_id)) {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}

static void hmac_vsp_fill_rx_slice_info(rx_slice_mgmt *rx_slice, hmac_vsp_sink_stru *sink)
{
    uint32_t layer_id;
    vsp_rx_slice_priv_stru *priv = hmac_vsp_sink_get_rx_slice_priv(rx_slice);

    for (layer_id = 0; layer_id < MAX_LAYER_NUM; layer_id++) {
        if (!hmac_vsp_sink_rx_layer_complete(rx_slice, layer_id)) {
            break;
        }

        hmac_vsp_sink_stat_inc_rx_succ_layer(&sink->sink_stat, layer_id);
    }

    /* 某个layer没收全，则后续的layer都不需要 */
    for (; layer_id < MAX_LAYER_NUM; layer_id++) {
        hmac_vsp_sink_reset_layer_rx_succ_num(rx_slice, layer_id);
    }

    hmac_vsp_log((uint8_t *)priv, VSP_LOG_LVL_0, VSP_LOG_DATATYPE_RX_SLICE_PRIV, VSP_LOG_TYPE_FILL_RX_SLICE);
}

static void hmac_vsp_sink_encap_feedback(oal_netbuf_stru *netbuf, rx_slice_mgmt *rx_slice)
{
    vsp_rx_slice_priv_stru *priv = hmac_vsp_sink_get_rx_slice_priv(rx_slice);
    hmac_vsp_sink_stru *vsp_sink = hmac_vsp_sink_get_rx_slice_vsp_info(rx_slice);
    vsp_feedback_frame *feedback = (vsp_feedback_frame *)oal_netbuf_data(netbuf);
    uint32_t layer_id;

    oal_set_mac_addr(feedback->ra, hmac_vsp_info_get_mac_ra(&vsp_sink->vsp_base));
    oal_set_mac_addr(feedback->sa, hmac_vsp_info_get_mac_sa(&vsp_sink->vsp_base));

    feedback->type = oal_host2net_short(0x0800); // 0x88A7 <--- what's this?
    feedback->frm_id = priv->frm_id;
    feedback->slice_id = priv->slice_id;
    feedback->layer_num = vsp_sink->vsp_base.param.layer_cnt;

    for (layer_id = 0; layer_id < feedback->layer_num; layer_id++) {
        feedback->layer_succ_num[layer_id] = hmac_vsp_sink_get_layer_rx_succ_num(rx_slice, layer_id);
        feedback->layer_succ_num[layer_id] |= (uint16_t)hmac_vsp_sink_rx_layer_complete(rx_slice, layer_id) <<
                                                        BIT_OFFSET_15; /* bit15标记layer是不是全部收到了 */
    }

    hmac_vsp_log((uint8_t *)feedback, VSP_LOG_LVL_2, VSP_LOG_DATATYPE_FEEDBACK, VSP_LOG_TYPE_SEND_FEEDBACK);
}

static void hmac_vsp_sink_set_feedback_cb(oal_netbuf_stru *netbuf, hmac_vsp_info_stru *vsp_info)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

    memset_s(tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));
    mac_get_cb_mpdu_num(tx_ctl) = 1;
    mac_get_cb_netbuf_num(tx_ctl) = 1;
    mac_get_cb_wlan_frame_type(tx_ctl) = WLAN_DATA_BASICTYPE;
    mac_get_cb_ack_policy(tx_ctl) = WLAN_TX_NORMAL_ACK;
    mac_get_cb_tx_vap_index(tx_ctl) = hmac_vsp_info_get_vap_id(vsp_info);
    mac_get_cb_tx_user_idx(tx_ctl) = hmac_vsp_info_get_user_id(vsp_info);
    mac_get_cb_wme_tid_type(tx_ctl) = WLAN_TIDNO_VSP;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_TID_TO_AC(WLAN_TIDNO_VSP);
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_NUM;
}

#define VSP_NETBUF_RESERVE (MAC_80211_QOS_HTC_4ADDR_FRAME_LEN + SNAP_LLC_FRAME_LEN)
static oal_netbuf_stru *hmac_vsp_sink_alloc_feedback_netbuf(hmac_vsp_info_stru *vsp_info)
{
    uint32_t len = hmac_vsp_info_get_feedback_pkt_len(vsp_info);
    oal_netbuf_stru *netbuf = oal_netbuf_alloc(len + VSP_NETBUF_RESERVE, VSP_NETBUF_RESERVE, 4); // 4字节对齐

    if (!netbuf) {
        return NULL;
    }

    oal_netbuf_prev(netbuf) = NULL;
    oal_netbuf_next(netbuf) = NULL;
    oal_netbuf_put(netbuf, len);

    return netbuf;
}

static uint32_t hmac_vsp_sink_send_feedback(rx_slice_mgmt *rx_slice, hmac_vsp_info_stru *vsp_info)
{
    oal_netbuf_stru *netbuf = hmac_vsp_sink_alloc_feedback_netbuf(vsp_info);

    if (!netbuf) {
        oam_error_log0(0, 0, "{hmac_vsp_rx_feedback::netbuf alloc failed!}");
        return OAL_FAIL;
    }

    hmac_vsp_sink_encap_feedback(netbuf, rx_slice);
    hmac_vsp_sink_set_feedback_cb(netbuf, vsp_info);

    if (oal_unlikely(hmac_tx_lan_to_wlan_no_tcp_opt(hmac_vsp_info_get_mac_vap(vsp_info), &netbuf) != OAL_SUCC)) {
        oam_error_log0(0, 0, "{hmac_vsp_rx_feedback::send feedback failed!");
        hmac_free_netbuf_list(netbuf);
        return OAL_FAIL;
    }

    hmac_vsp_log((uint8_t *)rx_slice, VSP_LOG_LVL_2, VSP_LOG_DATATYPE_RX_SLICE, VSP_LOG_TYPE_SEND_FEEDBACK);

    return OAL_SUCC;
}

static void hmac_vsp_sink_rx_slice_complete_proc(hmac_vsp_sink_stru *sink, rx_slice_mgmt *rx_slice)
{
    if (sink->vsp_base.timer_running) {
        hmac_vsp_stop_timer(&sink->vsp_base);
    }

    hmac_vsp_fill_rx_slice_info(rx_slice, sink);
    hmac_vsp_sink_send_feedback(rx_slice, &sink->vsp_base);

    hmac_vsp_sink_update_rx_comp_stat(sink);
    hmac_vsp_inc_next_timeout_slice(sink);
    hmac_vsp_sink_reset_rx_slice_in_tbl(sink, hmac_vsp_sink_get_rx_slice_tbl_idx(rx_slice));

    vsp_vcodec_rx_slice_done(rx_slice);
}

static uint32_t hmac_vsp_sink_update_rx_slice(
    hmac_vsp_sink_stru *sink, rx_slice_mgmt *rx_slice, vsp_msdu_hdr_stru *vsp_hdr, oal_netbuf_stru *netbuf)
{
    vsp_rx_slice_priv_stru *priv = hmac_vsp_sink_get_rx_slice_priv(rx_slice);

    if (hmac_vsp_sink_update_rx_slice_pkt_info(rx_slice, priv, vsp_hdr) != OAL_SUCC) {
        return OAL_FAIL;
    }

    hmac_vsp_sink_rx_slice_netbuf_enqueue(rx_slice, netbuf);
    hmac_vsp_sink_stat_rx_msdu(sink, vsp_hdr->len);

    return OAL_SUCC;
}

static inline uint8_t hmac_vsp_sink_rx_current_slice_pkt(hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr)
{
    if (oal_unlikely(hmac_vsp_info_next_timeout_info_invalid(sink))) {
        /* first rx */
        return OAL_TRUE;
    }

    return (vsp_hdr->frame_id == sink->next_timeout_frm) && (vsp_hdr->slice_num == sink->next_timeout_slice);
}

static uint32_t hmac_vsp_sink_rx_current_slice_pkt_proc(
    hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr, oal_netbuf_stru *netbuf)
{
    rx_slice_mgmt *rx_slice = hmac_vsp_sink_get_pkt_rx_slice(sink, vsp_hdr);

    if (oal_unlikely(!rx_slice)) {
        return OAL_FAIL;
    }

    if (oal_unlikely(hmac_vsp_sink_update_rx_slice(sink, rx_slice, vsp_hdr, netbuf) != OAL_SUCC)) {
        return OAL_FAIL;
    }

    if (hmac_vsp_sink_rx_slice_complete(&sink->vsp_base, rx_slice)) {
        hmac_vsp_sink_rx_slice_complete_proc(sink, rx_slice);
    }

    return OAL_SUCC;
}

static void hmac_vsp_sink_rx_slice_partial_complete_proc(hmac_vsp_sink_stru *sink)
{
    rx_slice_mgmt *rx_slice = hmac_vsp_sink_get_next_timeout_rx_slice(sink);

    if (!rx_slice) {
        oam_error_log2(0, 0, "{hmac_vsp_sink_partial_comp::rx_slice NULL! next_timeout frame[%d] slice[%d]}",
            sink->next_timeout_frm, sink->next_timeout_slice);
        return;
    }

    hmac_vsp_sink_rx_slice_complete_proc(sink, rx_slice);
}

/* 根据VSP的特性, 收到非当前slice的帧时, 认为之前的slice已接收完成, 将其上报后再处理当前slice */
static uint32_t hmac_vsp_sink_rx_non_current_slice_pkt_proc(
    hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr, oal_netbuf_stru *netbuf)
{
    hmac_vsp_sink_rx_slice_partial_complete_proc(sink);

    return hmac_vsp_sink_rx_current_slice_pkt_proc(sink, vsp_hdr, netbuf);
}

static void hmac_vsp_sink_rx_cfg_pkt_complete_proc(hmac_vsp_sink_stru *sink, rx_slice_mgmt *rx_slice)
{
    hmac_vsp_fill_rx_slice_info(rx_slice, sink);
    hmac_vsp_sink_send_feedback(rx_slice, &sink->vsp_base);

    hmac_vsp_sink_reset_rx_slice_in_tbl(sink, hmac_vsp_sink_get_rx_slice_tbl_idx(rx_slice));
    vsp_vcodec_rx_slice_done(rx_slice);
}

static uint32_t hmac_vsp_sink_rx_cfg_pkt_proc(
    hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr, oal_netbuf_stru *netbuf)
{
    rx_slice_mgmt *rx_slice = hmac_vsp_sink_get_pkt_rx_slice(sink, vsp_hdr);

    if (oal_unlikely(!rx_slice)) {
        return OAL_FAIL;
    }

    if (oal_unlikely(hmac_vsp_sink_update_rx_slice(sink, rx_slice, vsp_hdr, netbuf) != OAL_SUCC)) {
        return OAL_FAIL;
    }

    hmac_vsp_sink_rx_cfg_pkt_complete_proc(sink, rx_slice);

    return OAL_SUCC;
}

static inline uint16_t hmac_vsp_sink_get_frame_slice_id(hmac_vsp_info_stru *vsp_info, uint8_t frm_id, uint8_t slice_id)
{
    return frm_id * vsp_info->param.slice_cnt + slice_id;
}

/*
 * 当前接收到的vsp帧frame&slice id是否小于等于已接收完成frame&slice id
 * 考虑翻转情况, 差值小于最大frame&slice id一半为有效值
 */
static inline uint8_t hmac_vsp_sink_frame_slice_id_cmp(hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr)
{
    uint16_t cur = hmac_vsp_sink_get_frame_slice_id(&sink->vsp_base, sink->next_timeout_frm, sink->next_timeout_slice);
    uint16_t rx = hmac_vsp_sink_get_frame_slice_id(&sink->vsp_base, vsp_hdr->frame_id, vsp_hdr->slice_num);
    uint16_t half = VSP_FRAME_ID_TOTAL * sink->vsp_base.param.slice_cnt / 2;

    return (rx < cur && cur - rx < half) || (rx > cur && rx - cur > half);
}

static inline uint8_t hmac_vsp_sink_rx_expired_pkt(hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr)
{
    if (oal_unlikely(hmac_vsp_info_next_timeout_info_invalid(sink))) {
        return OAL_FALSE;
    }

    return hmac_vsp_sink_frame_slice_id_cmp(sink, vsp_hdr);
}

static uint32_t hmac_vsp_sink_rx_data_pkt_proc(
    hmac_vsp_sink_stru *sink, vsp_msdu_hdr_stru *vsp_hdr, oal_netbuf_stru *netbuf)
{
    if (oal_unlikely(hmac_vsp_sink_rx_expired_pkt(sink, vsp_hdr))) {
        hmac_vsp_log((uint8_t *)vsp_hdr, VSP_LOG_LVL_2, VSP_LOG_DATATYPE_PKT, VSP_LOG_TYPE_RX_EXPIRED_PKT);
        return OAL_FAIL;
    }

    if (oal_likely(hmac_vsp_sink_rx_current_slice_pkt(sink, vsp_hdr))) {
        return hmac_vsp_sink_rx_current_slice_pkt_proc(sink, vsp_hdr, netbuf);
    } else {
        hmac_vsp_log((uint8_t *)vsp_hdr, VSP_LOG_LVL_2, VSP_LOG_DATATYPE_PKT, VSP_LOG_TYPE_RX_NONCURRENT_PKT);
        return hmac_vsp_sink_rx_non_current_slice_pkt_proc(sink, vsp_hdr, netbuf);
    }
}

static uint32_t hmac_vsp_sink_rx_msdu_proc(hmac_vsp_sink_stru *sink, oal_netbuf_stru *netbuf)
{
    vsp_msdu_hdr_stru *vsp_hdr = (vsp_msdu_hdr_stru *)oal_netbuf_data(netbuf);

    if (oal_unlikely(hmac_vsp_sink_rx_pkt_invalid(&sink->vsp_base, vsp_hdr, netbuf))) {
        return OAL_FAIL;
    }

    if (oal_likely(!hmac_is_vsp_sink_cfg_pkt(vsp_hdr))) {
        return hmac_vsp_sink_rx_data_pkt_proc(sink, vsp_hdr, netbuf);
    } else {
        return hmac_vsp_sink_rx_cfg_pkt_proc(sink, vsp_hdr, netbuf);
    }
}

/* 接收端入口, 解析并处理每个VSP MSDU */
void hmac_vsp_sink_rx_amsdu_proc(oal_netbuf_stru *netbuf)
{
    hmac_vsp_sink_stru *sink = &g_vsp_sink;
    oal_netbuf_stru *current_msdu = netbuf;
    oal_netbuf_stru *next_msdu = NULL;

    if (oal_unlikely(!netbuf)) {
        return;
    }

    while (current_msdu) {
        next_msdu = oal_netbuf_next(current_msdu);
        if (hmac_vsp_sink_rx_msdu_proc(sink, current_msdu) != OAL_SUCC) {
            oal_netbuf_next(current_msdu) = NULL;
            oal_netbuf_free(current_msdu);
        }
        current_msdu = next_msdu;
    }
}

/* vdec处理完数据后通知Wi-Fi */
void vdec_rx_slice_done(rx_slice_mgmt *rx_slice)
{
    if (oal_unlikely(!rx_slice || !hmac_vsp_sink_get_rx_slice_priv(rx_slice))) {
        oam_error_log0(0, 0, "{vdec_rx_slice_done::rx_slice_mgmt/priv NULL}");
        return;
    }

    if (oal_unlikely(!hmac_vsp_sink_get_rx_slice_vsp_info(rx_slice))) {
        oam_error_log0(0, 0, "{vdec_rx_slice_done::vap_info NULL}");
        hmac_vsp_sink_rx_slice_netbuf_queue_purge(rx_slice);
        return;
    }

    hmac_vsp_sink_update_netbuf_recycle_cnt(rx_slice);
    hmac_vsp_sink_deinit_rx_slice_priv(rx_slice);
}

void hmac_vsp_sink_rx_slice_timeout(hmac_vsp_sink_stru *sink)
{
    if (oal_unlikely(!sink)) {
        return;
    }

    hmac_vsp_sink_rx_slice_partial_complete_proc(sink);
}

static void hmac_vsp_sink_rx_slice_tbl_deinit(hmac_vsp_sink_stru *sink)
{
    if (!sink->rx_slice_tbl) {
        return;
    }

    oal_free(sink->rx_slice_tbl);
    sink->rx_slice_tbl = NULL;
}

static void hmac_vsp_sink_deinit(void)
{
    hmac_vsp_sink_stru *sink = &g_vsp_sink;

    hmac_vsp_deinit(&sink->vsp_base);
    hmac_vsp_sink_rx_slice_tbl_deinit(sink);
}

static uint32_t hmac_vsp_sink_rx_slice_tbl_init(hmac_vsp_sink_stru *sink)
{
    uint32_t tbl_size = VSP_FRAME_ID_TOTAL * sizeof(rx_slice_mgmt*) * sink->vsp_base.param.slice_cnt;

    sink->rx_slice_tbl = (rx_slice_mgmt**)oal_memalloc(tbl_size);
    if (sink->rx_slice_tbl == NULL) {
        oam_error_log1(0, 0, "{hmac_vsp_start::slice table alloc failed, size %d!}", tbl_size);
        return OAL_FAIL;
    }
    memset_s(sink->rx_slice_tbl, tbl_size, 0, tbl_size);

    return OAL_SUCC;
}

static uint32_t hmac_vsp_sink_init(hmac_vsp_sink_stru *sink)
{
    sink->next_timeout_frm = VSP_INVALID_FRAME_ID;
    sink->next_timeout_slice = 0;
    memset_s(&sink->sink_stat, sizeof(hmac_vsp_sink_stat_stru), 0, sizeof(hmac_vsp_sink_stat_stru));

    return hmac_vsp_sink_rx_slice_tbl_init(sink);
}

uint32_t hmac_vsp_sink_cfg(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, vsp_param *param)
{
    if (oal_any_null_ptr3(hmac_vap, hmac_user, param)) {
        return OAL_FAIL;
    }

    if (hmac_vsp_init(&g_vsp_sink.vsp_base, hmac_vap, hmac_user, param, VSP_MODE_SINK) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (hmac_vsp_sink_init(&g_vsp_sink) != OAL_SUCC) {
        hmac_vsp_sink_deinit();
        return OAL_FAIL;
    }

    hmac_vsp_context_init(&g_vsp_sink.vsp_base, hmac_vap, hmac_user);
    hmac_vsp_set_current_handle(&g_vsp_sink.vsp_base);

    return OAL_SUCC;
}

void hmac_vsp_sink_stop(void)
{
    hmac_vsp_context_deinit(&g_vsp_sink.vsp_base);
    hmac_vsp_sink_deinit();
}

#endif
