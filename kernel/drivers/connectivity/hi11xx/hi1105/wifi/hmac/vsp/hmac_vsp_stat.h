/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : hmac_vsp_stat.h
 * 作者       : wifi
 * 创建日期   : 2022年3月9日
 */

#ifndef HMAC_VSP_STAT_H
#define HMAC_VSP_STAT_H

#include "wlan_types.h"
#include "hmac_stat.h"
#include "hmac_config.h"

#ifdef _PRE_WLAN_FEATURE_VSP

#define MAX_SLICE_NUM 4
typedef struct {
    uint32_t last_time;
    uint32_t slice_total;
    uint32_t slice_last;
    uint32_t slice_rate;
    uint64_t bytes_total;
    uint64_t bytes_last;
    uint32_t data_rate;
    uint64_t airtime_total[MAX_SLICE_NUM];
} hmac_vsp_stat_stru;

typedef struct {
    hmac_vsp_stat_stru rate;
    uint32_t rx_succ_layer[MAX_LAYER_NUM];
    uint32_t netbuf_recycle_cnt;
} hmac_vsp_sink_stat_stru;

typedef struct {
    hmac_vsp_stat_stru rate;
    uint32_t feedback_cnt;
    uint32_t rx_succ_feedback[MAX_LAYER_NUM];
} hmac_vsp_source_stat_stru;

static inline void hmac_vsp_sink_stat_inc_rx_succ_layer(hmac_vsp_sink_stat_stru *stat, uint8_t layer_id)
{
    stat->rx_succ_layer[layer_id]++;
}

static inline uint32_t hmac_vsp_sink_stat_get_rx_succ_layer(hmac_vsp_sink_stat_stru *stat, uint8_t layer_id)
{
    return stat->rx_succ_layer[layer_id];
}

static inline void hmac_vsp_sink_stat_update_netbuf_recycle_cnt(hmac_vsp_sink_stat_stru *stat, uint32_t cnt)
{
    stat->netbuf_recycle_cnt += cnt;
}

static inline uint32_t hmac_vsp_sink_stat_get_netbuf_recycle_cnt(hmac_vsp_sink_stat_stru *stat)
{
    return stat->netbuf_recycle_cnt;
}

static inline void hmac_vsp_source_stat_inc_feedback_cnt(hmac_vsp_source_stat_stru *stat)
{
    stat->feedback_cnt++;
}

static inline uint32_t hmac_vsp_source_stat_get_feedback_cnt(hmac_vsp_source_stat_stru *stat)
{
    return stat->feedback_cnt;
}

static inline void hmac_vsp_source_stat_inc_rx_succ_feedback(hmac_vsp_source_stat_stru *stat, uint8_t layer_id)
{
    stat->rx_succ_feedback[layer_id]++;
}

static inline uint32_t hmac_vsp_source_stat_get_rx_succ_feedback(hmac_vsp_source_stat_stru *stat, uint8_t layer_id)
{
    return stat->rx_succ_feedback[layer_id];
}

static inline uint32_t hmac_vsp_stat_get_last_time(hmac_vsp_stat_stru *stat)
{
    return stat->last_time;
}

static inline void hmac_vsp_stat_update_last_time(hmac_vsp_stat_stru *stat, uint32_t ts)
{
    stat->last_time = ts;
}

static inline uint32_t hmac_vsp_stat_get_slice_total(hmac_vsp_stat_stru *stat)
{
    return stat->slice_total;
}

static inline void hmac_vsp_stat_inc_slice_total(hmac_vsp_stat_stru *stat)
{
    stat->slice_total++;
}

static inline uint32_t hmac_vsp_stat_get_slice_last(hmac_vsp_stat_stru *stat)
{
    return stat->slice_last;
}

static inline void hmac_vsp_stat_update_slice_last(hmac_vsp_stat_stru *stat)
{
    stat->slice_last = stat->slice_total;
}

static inline uint32_t hmac_vsp_stat_get_slice_rate(hmac_vsp_stat_stru *stat)
{
    return stat->slice_rate;
}

static inline void hmac_vsp_stat_update_slice_rate(hmac_vsp_stat_stru *stat, uint32_t runtime)
{
    stat->slice_rate = runtime ? (stat->slice_total - stat->slice_last) / runtime : stat->slice_rate;
}

static inline uint64_t hmac_vsp_stat_get_bytes_total(hmac_vsp_stat_stru *stat)
{
    return stat->bytes_total;
}

static inline void hmac_vsp_stat_update_bytes_total(hmac_vsp_stat_stru *stat, uint64_t bytes)
{
    stat->bytes_total += bytes;
}

static inline uint64_t hmac_vsp_stat_get_bytes_last(hmac_vsp_stat_stru *stat)
{
    return stat->bytes_last;
}

static inline void hmac_vsp_stat_update_bytes_last(hmac_vsp_stat_stru *stat)
{
    stat->bytes_last = stat->bytes_total;
}

static inline uint32_t hmac_vsp_stat_get_data_rate(hmac_vsp_stat_stru *stat)
{
    return stat->data_rate;
}

static inline void hmac_vsp_stat_update_airtime(hmac_vsp_stat_stru *stat, uint32_t airtime, uint8_t slice_id)
{
    stat->airtime_total[slice_id] += airtime;
}

static inline uint64_t hmac_vsp_stat_get_airtime(hmac_vsp_stat_stru *stat, uint8_t slice_id)
{
    return stat->airtime_total[slice_id];
}

static inline void hmac_vsp_stat_update_data_rate(hmac_vsp_stat_stru *stat, uint32_t runtime)
{
    uint64_t value;

    value = (stat->bytes_total - stat->bytes_last) * 8; // 8: Byte to bit
    do_div(value, runtime * HMAC_S_TO_MS);
    stat->data_rate = runtime ? value : stat->data_rate;
}

static inline void hmac_vsp_stat_calc(hmac_vsp_stat_stru *stat)
{
    uint32_t now = oal_time_get_stamp_ms();
    uint32_t runtime = oal_time_get_runtime(hmac_vsp_stat_get_last_time(stat), now) / HMAC_S_TO_MS;
    /* 计算间隔2s以上 */
    if (runtime < 2) {
        return;
    }

    hmac_vsp_stat_update_slice_rate(stat, runtime);
    hmac_vsp_stat_update_data_rate(stat, runtime);
    hmac_vsp_stat_update_slice_last(stat);
    hmac_vsp_stat_update_bytes_last(stat);
    hmac_vsp_stat_update_last_time(stat, now);
}

#endif
#endif
