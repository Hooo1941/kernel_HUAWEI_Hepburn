/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : hmac_vsp.c 的头文件
 * 作者       :
 */

#ifndef HMAC_VSP_SOURCE_H
#define HMAC_VSP_SOURCE_H

#include "hmac_vsp.h"
#include "oal_dma_buf.h"

#ifdef _PRE_WLAN_FEATURE_VSP
#define VSP_RECIEVER_FEEDBACK 0
#define VSP_TRANSMITTER_FEEDBACK 1

enum vsp_source_event {
    VSP_SRC_EVT_TIMEOUT,
    VSP_SRC_EVT_TX_COMP,
    VSP_SRC_EVT_NEW_SLICE,
    VSP_SRC_EVT_RX_FEEDBACK,
    VSP_SRC_EVT_BUTT,
};

typedef struct {
    oal_dlist_head_stru entry;
    tx_layer_ctrl *head; /* layer链表头 */
    tx_layer_ctrl *curr; /* 当前发送的layer */
    uint16_t tx_pos; /* 下一个要发送的packet idx */
    uint8_t slice_id;
    uint8_t frame_id;
    uint32_t arrive_ts; /* 本slice达到驱动的时间戳 */
    uint32_t vsync_ts; /* 同last_vsync 记录与本video frame对应的vsync时间，用于计算超时时间 */
    uint32_t pkts_in_ring;
    uint8_t feedback;
    uint8_t is_rpt;
    uint8_t retrans_cnt;
    send_result tx_result;
} hmac_vsp_tx_slice_stru;

typedef struct {
    oal_dlist_head_stru head;
    oal_spin_lock_stru lock;
    uint32_t len;
} hmac_vsp_slice_queue_stru;

#define HMAC_VSP_LAYER_MAX_FACTOR_NUM 4
typedef struct {
    uint32_t layer0_max;
    uint32_t layerall_max;
    uint16_t ratio_th[HMAC_VSP_LAYER_MAX_FACTOR_NUM];
    uint16_t factor[HMAC_VSP_LAYER_MAX_FACTOR_NUM];
} hmac_vsp_layer_max_mgmt_stru;

typedef struct {
    hmac_vsp_info_stru vsp_base;
    hmac_vsp_slice_queue_stru free_queue; /* 存放新到达的slice的队列 */
    hmac_vsp_slice_queue_stru comp_queue; /* 存放发送完成的slice的队列 */
    uint32_t last_vsync; // 本次slice视频帧vsync同步时间戳
    uint32_t pkt_ts_vsync; // 本次slice视频帧pkt级别的时间戳，同last_vsync
    struct task_struct *evt_loop;
    unsigned long evt_flags;
    oal_wait_queue_head_stru evt_wq;
    oal_netbuf_head_stru feedback_q; /* 存放接收端发送的反馈帧的队列 */
    hmac_vsp_layer_max_mgmt_stru layer_max_mgmt;
    hmac_vsp_source_stat_stru source_stat;
} hmac_vsp_source_stru;

void hmac_vsp_source_rx_amsdu_proc(oal_netbuf_stru *netbuf);
int hmac_vsp_source_evt_loop(void *data);
uint32_t hmac_vsp_source_cfg(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, vsp_param *param);
void hmac_vsp_source_stop(void);

static inline uint8_t hmac_is_vsp_source(hmac_user_stru *hmac_user)
{
    hmac_vsp_info_stru *vsp_info = hmac_user->vsp_hdl;

    return vsp_info && vsp_info->enable && vsp_info->mode == VSP_MODE_SOURCE;
}

static inline uint8_t hmac_is_vsp_source_tid(hmac_tid_info_stru *tid_info)
{
    hmac_user_stru *hmac_user = mac_res_get_hmac_user(tid_info->user_index);

    return hmac_user && hmac_is_vsp_source(hmac_user) && tid_info->tid_no == WLAN_TIDNO_VSP;
}

static inline uint8_t hmac_is_vsp_source_cfg_pkt(tx_layer_ctrl *layer)
{
    return (layer->qos_type & BIT14) != 0;
}

static inline uint8_t hmac_vsp_source_get_tx_slice_id(tx_layer_ctrl *layer)
{
    return (layer->qos_type >> BIT_OFFSET_4) & 0xf;
}

static inline uint8_t hmac_vsp_source_get_tx_frame_id(tx_layer_ctrl *layer)
{
    return (layer->qos_type >> BIT_OFFSET_8) & VSP_FRAME_ID_MASK;
}

static inline void hmac_vsp_source_set_tx_result_cfg_pkt(hmac_vsp_tx_slice_stru *slice)
{
    slice->tx_result.slice_info |= BIT7;
}

static inline void hmac_vsp_source_set_tx_result_rpt_type(hmac_vsp_tx_slice_stru *slice)
{
    slice->tx_result.slice_info |= BIT6;
}

static inline uint8_t hmac_vsp_source_get_tx_result_rpt_type(hmac_vsp_tx_slice_stru *slice, uint8_t type)
{
    return slice->tx_result.slice_info & BIT6;
}

static inline void hmac_vsp_source_set_tx_result_layer_tx_comp(hmac_vsp_tx_slice_stru *slice, uint8_t layer_id)
{
    slice->tx_result.layer_suc_num[layer_id] |= BIT15;
}

static inline void hmac_vsp_source_inc_tx_result_layer_succ_num(hmac_vsp_tx_slice_stru *slice, uint8_t layer_id)
{
    slice->tx_result.layer_suc_num[layer_id]++;
}

static inline uint8_t hmac_vsp_source_get_tx_result_layer_succ_num(hmac_vsp_tx_slice_stru *slice, uint8_t layer_id)
{
    return slice->tx_result.layer_suc_num[layer_id];
}

static inline void hmac_vsp_source_reset_tx_result_layer_succ_num(hmac_vsp_tx_slice_stru *slice, uint8_t layer_id)
{
    slice->tx_result.layer_suc_num[layer_id] = 0;
}

static inline void hmac_vsp_source_update_tx_result_layer0_max(hmac_vsp_tx_slice_stru *slice, uint16_t layer0_max)
{
    slice->tx_result.slice_layer0_max = layer0_max;
}

static inline void hmac_vsp_source_update_tx_result_layerall_max(hmac_vsp_tx_slice_stru *slice, uint16_t layerall_max)
{
    slice->tx_result.slice_layerall_max = layerall_max;
}

static inline uint8_t hmac_vsp_source_tx_layer_comp(
    hmac_vsp_tx_slice_stru *slice, tx_layer_ctrl *layer, uint8_t layer_id)
{
    return layer->paket_number && layer->paket_number == slice->tx_result.layer_suc_num[layer_id];
}

static inline uintptr_t hmac_vsp_source_get_pkt_vsp_hdr(uintptr_t va)
{
    return va + VSP_MSDU_CB_LEN;
}

static inline uintptr_t hmac_vsp_source_get_pkt_msdu_dscr(uintptr_t va)
{
    return hmac_vsp_source_get_pkt_vsp_hdr(va) - HAL_TX_MSDU_DSCR_LEN;
}

static inline void hmac_vsp_source_init_slice_tx_result_frame_id(uint16_t *slice_info, uint8_t frame_id)
{
    *slice_info |= frame_id & VSP_FRAME_ID_MASK;
}

static inline void hmac_vsp_source_init_slice_tx_result_slice_id(uint16_t *slice_info, uint8_t slice_id)
{
    *slice_info |= (slice_id & 0xF) << BIT_OFFSET_12;
}

static inline void hmac_vsp_source_init_slice_tx_result_layer_num(uint16_t *slice_info, uint8_t layer_num)
{
    *slice_info |= (layer_num & 0xF) << BIT_OFFSET_8;
}

static inline void hmac_vsp_source_init_slice_tx_result(
    hmac_vsp_info_stru *vsp_info, hmac_vsp_tx_slice_stru *slice, tx_layer_ctrl *layer)
{
    memset_s(&slice->tx_result, sizeof(send_result), 0, sizeof(send_result));
    slice->tx_result.slice_layer = layer;
    hmac_vsp_source_init_slice_tx_result_frame_id(&slice->tx_result.slice_info, slice->frame_id);
    hmac_vsp_source_init_slice_tx_result_slice_id(&slice->tx_result.slice_info, slice->slice_id);
    hmac_vsp_source_init_slice_tx_result_layer_num(&slice->tx_result.slice_info, hmac_vsp_info_get_layer_num(vsp_info));
}

static inline void hmac_vsp_source_set_vsp_hdr_cfg_pkt(hmac_vsp_tx_slice_stru *slice, uint8_t cfg_pkt)
{
    vsp_msdu_hdr_stru *vsp_hdr = (vsp_msdu_hdr_stru *)hmac_vsp_source_get_pkt_vsp_hdr(slice->head->data_addr);

    vsp_hdr->cfg_pkt = cfg_pkt;
}

static inline void hmac_vsp_source_inc_slice_total(hmac_vsp_source_stru *source)
{
    hmac_vsp_stat_inc_slice_total(&source->source_stat.rate);
}

static inline void hmac_vsp_source_stat_tx_msdu(hmac_vsp_source_stru *source, uint64_t bytes)
{
    hmac_vsp_stat_update_bytes_total(&source->source_stat.rate, bytes);
    hmac_stat_device_tx_msdu(bytes);
}

static inline void hmac_vsp_slice_queue_init(hmac_vsp_slice_queue_stru *queue)
{
    queue->len = 0;
    oal_dlist_init_head(&queue->head);
    oal_spin_lock_init(&queue->lock);
}

typedef uint8_t (* hmac_vsp_slice_func)(hmac_vsp_tx_slice_stru *, void *);
static inline void hmac_vsp_slice_queue_foreach_safe(
    hmac_vsp_slice_queue_stru *queue, hmac_vsp_slice_func func, void *param)
{
    oal_dlist_head_stru *entry = NULL;
    oal_dlist_head_stru *entry_tmp = NULL;

    oal_dlist_search_for_each_safe(entry, entry_tmp, &queue->head) {
        if (func(oal_dlist_get_entry(entry, hmac_vsp_tx_slice_stru, entry), param) == OAL_RETURN) {
            break;
        }
    }
}

static inline uint8_t hmac_vsp_slice_cnt(hmac_vsp_tx_slice_stru *slice, void *param)
{
    (*(uint8_t *)param)++;

    return OAL_CONTINUE;
}

static inline uint32_t hmac_vsp_slice_queue_len_unlock(hmac_vsp_slice_queue_stru *queue)
{
    uint32_t len = 0;

    hmac_vsp_slice_queue_foreach_safe(queue, hmac_vsp_slice_cnt, &len);

    return len;
}

static inline uint32_t hmac_vsp_slice_queue_len(hmac_vsp_slice_queue_stru *queue)
{
    uint32_t len;

    oal_spin_lock(&queue->lock);
    len = hmac_vsp_slice_queue_len_unlock(queue);
    oal_spin_unlock(&queue->lock);

    return len;
}

static inline uint8_t hmac_vsp_slice_queue_empty(hmac_vsp_slice_queue_stru *queue)
{
    return oal_dlist_is_empty(&queue->head);
}

static inline void hmac_vsp_slice_enqueue_unlock(hmac_vsp_slice_queue_stru *queue, hmac_vsp_tx_slice_stru *slice)
{
    oal_dlist_add_tail(&slice->entry, &queue->head);
}

static inline void hmac_vsp_slice_enqueue(hmac_vsp_slice_queue_stru *queue, hmac_vsp_tx_slice_stru *slice)
{
    oal_spin_lock(&queue->lock);
    hmac_vsp_slice_enqueue_unlock(queue, slice);
    oal_spin_unlock(&queue->lock);
}

static inline void hmac_vsp_slice_enqueue_head_unlock(hmac_vsp_slice_queue_stru *queue, hmac_vsp_tx_slice_stru *slice)
{
    oal_dlist_add_head(&slice->entry, &queue->head);
}

static inline void hmac_vsp_slice_enqueue_head(hmac_vsp_slice_queue_stru *queue, hmac_vsp_tx_slice_stru *slice)
{
    oal_spin_lock(&queue->lock);
    hmac_vsp_slice_enqueue_head_unlock(queue, slice);
    oal_spin_unlock(&queue->lock);
}

static inline hmac_vsp_tx_slice_stru *hmac_vsp_slice_dequeue_unlock(hmac_vsp_slice_queue_stru *queue)
{
    return oal_container_of(oal_dlist_delete_head(&queue->head), hmac_vsp_tx_slice_stru, entry);
}

static inline hmac_vsp_tx_slice_stru *hmac_vsp_slice_dequeue(hmac_vsp_slice_queue_stru *queue)
{
    hmac_vsp_tx_slice_stru *slice = NULL;

    oal_spin_lock(&queue->lock);
    slice = hmac_vsp_slice_dequeue_unlock(queue);
    oal_spin_unlock(&queue->lock);

    return slice;
}

static inline hmac_vsp_tx_slice_stru *hmac_vsp_slice_queue_peek(hmac_vsp_slice_queue_stru *queue)
{
    return oal_container_of(queue->head.pst_next, hmac_vsp_tx_slice_stru, entry);
}

#endif
#endif
