/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : hmac_vsp.c 的头文件
 * 作者       :
 */

#ifndef HMAC_VSP_SINK_H
#define HMAC_VSP_SINK_H

#include "hmac_vsp.h"

#ifdef _PRE_WLAN_FEATURE_VSP
typedef struct {
    hmac_vsp_info_stru vsp_base;
    rx_slice_mgmt **rx_slice_tbl;
    uint8_t next_timeout_frm;
    uint8_t next_timeout_slice;
    hmac_vsp_sink_stat_stru sink_stat;
} hmac_vsp_sink_stru;

typedef struct {
    uint8_t frm_id;
    uint8_t slice_id;
    uint32_t tbl_idx;
    uint32_t total_pkts[MAX_LAYER_NUM];
    oal_netbuf_head_stru rx_queue;
    uint32_t alloc_ts; /* 结构体申请的时间点，即第一个数据包收到的时间 */
    uint32_t vsync_ts; /* 发送端同步的vsync时间 */
    uint8_t vsync_vld; /* vsync_ts是否有效 */
    hmac_vsp_sink_stru *owner;
} vsp_rx_slice_priv_stru;

void hmac_vsp_sink_rx_amsdu_proc(oal_netbuf_stru *netbuf);
void hmac_vsp_sink_rx_slice_timeout(hmac_vsp_sink_stru *sink);
uint32_t hmac_vsp_sink_cfg(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, vsp_param *param);
void hmac_vsp_sink_stop(void);

static inline uint8_t hmac_is_vsp_sink(hmac_user_stru *hmac_user)
{
    hmac_vsp_info_stru *vsp_info = hmac_user->vsp_hdl;

    return vsp_info && vsp_info->enable && vsp_info->mode == VSP_MODE_SINK;
}

static inline uint8_t hmac_is_vsp_sink_tid(hmac_tid_info_stru *tid_info)
{
    hmac_user_stru *hmac_user = mac_res_get_hmac_user(tid_info->user_index);

    return hmac_user && hmac_is_vsp_sink(hmac_user) && tid_info->tid_no == WLAN_TIDNO_VSP;
}

static inline uint8_t hmac_is_vsp_sink_cfg_pkt(vsp_msdu_hdr_stru *vsp_hdr)
{
    return vsp_hdr->cfg_pkt;
}

static inline void hmac_vsp_sink_inc_layer_rx_succ_num(rx_slice_mgmt *rx_slice, uint8_t layer_id)
{
    rx_slice->packet_num[layer_id]++;
}

static inline void hmac_vsp_sink_reset_layer_rx_succ_num(rx_slice_mgmt *rx_slice, uint8_t layer_id)
{
    rx_slice->packet_num[layer_id] = 0;
}

static inline uint32_t hmac_vsp_sink_get_layer_rx_succ_num(rx_slice_mgmt *rx_slice, uint8_t layer_id)
{
    return rx_slice->packet_num[layer_id];
}

static inline vsp_rx_slice_priv_stru *hmac_vsp_sink_get_rx_slice_priv(rx_slice_mgmt *rx_slice)
{
    return (vsp_rx_slice_priv_stru *)rx_slice->priv[0];
}

static inline uint32_t hmac_vsp_sink_get_rx_slice_tbl_idx(rx_slice_mgmt *rx_slice)
{
    return hmac_vsp_sink_get_rx_slice_priv(rx_slice)->tbl_idx;
}

static inline void hmac_vsp_sink_update_rx_slice_tbl_idx(rx_slice_mgmt *rx_slice, uint32_t idx)
{
    hmac_vsp_sink_get_rx_slice_priv(rx_slice)->tbl_idx = idx;
}

static inline hmac_vsp_sink_stru *hmac_vsp_sink_get_rx_slice_vsp_info(rx_slice_mgmt *rx_slice)
{
    return hmac_vsp_sink_get_rx_slice_priv(rx_slice)->owner;
}

static inline oal_netbuf_head_stru *hmac_vsp_sink_rx_slice_netbuf_queue(rx_slice_mgmt *rx_slice)
{
    return &hmac_vsp_sink_get_rx_slice_priv(rx_slice)->rx_queue;
}

static inline void hmac_vsp_sink_rx_slice_netbuf_enqueue(rx_slice_mgmt *rx_slice, oal_netbuf_stru *netbuf)
{
    oal_netbuf_addlist(hmac_vsp_sink_rx_slice_netbuf_queue(rx_slice), netbuf);
}

static inline void hmac_vsp_sink_rx_slice_netbuf_queue_purge(rx_slice_mgmt *rx_slice)
{
    oal_netbuf_queue_purge(hmac_vsp_sink_rx_slice_netbuf_queue(rx_slice));
}

static inline void hmac_vsp_sink_update_netbuf_recycle_cnt(rx_slice_mgmt *rx_slice)
{
    hmac_vsp_sink_stru *sink = hmac_vsp_sink_get_rx_slice_vsp_info(rx_slice);
    oal_netbuf_head_stru *queue = hmac_vsp_sink_rx_slice_netbuf_queue(rx_slice);
    uint32_t recycle_cnt = hal_host_rx_buff_recycle(sink->vsp_base.host_hal_device, queue);

    hmac_vsp_sink_stat_update_netbuf_recycle_cnt(&sink->sink_stat, recycle_cnt);
}

static inline rx_slice_mgmt *hmac_vsp_sink_get_rx_slice_from_tbl(hmac_vsp_sink_stru *sink, uint32_t idx)
{
    return sink->rx_slice_tbl[idx];
}

static inline void hmac_vsp_sink_set_rx_slice_to_tbl(hmac_vsp_sink_stru *sink, uint32_t idx, rx_slice_mgmt *slice)
{
    sink->rx_slice_tbl[idx] = slice;
    hmac_vsp_sink_update_rx_slice_tbl_idx(slice, idx);
}

static inline void hmac_vsp_sink_reset_rx_slice_in_tbl(hmac_vsp_sink_stru *sink, uint32_t idx)
{
    sink->rx_slice_tbl[idx] = NULL;
}

static inline void hmac_vsp_sink_deinit_rx_slice_priv(rx_slice_mgmt *rx_slice)
{
    oal_free(hmac_vsp_sink_get_rx_slice_priv(rx_slice));
}

static inline void hmac_vsp_sink_update_rx_slice_pkt_addr(
    rx_slice_mgmt *rx_slice, uint8_t layer_id, uint32_t pkt_id, uintptr_t addr)
{
    rx_slice->base_addr[layer_id][pkt_id] = addr;
    hmac_vsp_sink_inc_layer_rx_succ_num(rx_slice, layer_id);
}

static inline uintptr_t hmac_vsp_sink_get_rx_slice_pkt_addr(rx_slice_mgmt *rx_slice, uint8_t layer_id, uint32_t pkt_id)
{
    return rx_slice->base_addr[layer_id][pkt_id];
}

static inline uint32_t hmac_vsp_sink_get_next_timeout_rx_slice_index(hmac_vsp_sink_stru *sink)
{
    return sink->next_timeout_frm * sink->vsp_base.param.slice_cnt + sink->next_timeout_slice;
}

static inline rx_slice_mgmt *hmac_vsp_sink_get_next_timeout_rx_slice(hmac_vsp_sink_stru *sink)
{
    return hmac_vsp_sink_get_rx_slice_from_tbl(sink, hmac_vsp_sink_get_next_timeout_rx_slice_index(sink));
}

static inline void hmac_vsp_sink_stat_rx_msdu(hmac_vsp_sink_stru *sink, uint32_t bytes)
{
    hmac_vsp_stat_update_bytes_total(&sink->sink_stat.rate, bytes);
    hmac_stat_device_rx_msdu(bytes);
}

static inline void hmac_vsp_sink_inc_slice_total(hmac_vsp_sink_stru *sink)
{
    hmac_vsp_stat_inc_slice_total(&sink->sink_stat.rate);
}

static inline void hmac_vsp_sink_update_rx_comp_stat(hmac_vsp_sink_stru *sink)
{
    uint32_t airtime = hmac_vsp_get_timestamp(&sink->vsp_base) - sink->vsp_base.timer_ref_vsync;

    hmac_vsp_stat_update_airtime(&sink->sink_stat.rate, airtime, sink->next_timeout_slice);
    hmac_vsp_stat_calc(&sink->sink_stat.rate);
}

static inline void hmac_vsp_inc_next_timeout_slice(hmac_vsp_sink_stru *sink)
{
    if (++sink->next_timeout_slice >= sink->vsp_base.param.slice_cnt) {
        sink->next_timeout_slice = 0;
        sink->next_timeout_frm = (sink->next_timeout_frm + 1) & VSP_FRAME_ID_MASK;
    }
}

static inline uint8_t hmac_vsp_info_next_timeout_info_invalid(hmac_vsp_sink_stru *sink)
{
    return sink->next_timeout_frm == VSP_INVALID_FRAME_ID;
}

static inline uint8_t hmac_vsp_sink_rx_layer_complete(rx_slice_mgmt *rx_slice, uint8_t layer_id)
{
    vsp_rx_slice_priv_stru *priv = hmac_vsp_sink_get_rx_slice_priv(rx_slice);

    return priv->total_pkts[layer_id] && (priv->total_pkts[layer_id] == rx_slice->packet_num[layer_id]);
}

#endif
#endif
