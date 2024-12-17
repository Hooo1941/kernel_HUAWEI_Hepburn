/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac tid调度模块 - 文件待重构
 * 作    者 : wifi
 * 创建日期 : 2020年7月28日
 */

#include "hmac_tid_sche.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_host_tx_data.h"
#include "oam_stat_wifi.h"
#include "hmac_tid_update.h"
#include "hmac_soft_retry.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TID_SCHE_C

/* tid调度管理结构体, 主要用于保存待调度的user tid链表, 在调度线程中处理 */
hmac_tid_schedule_stru g_tid_schedule_list;
oal_atomic g_tid_sche_forbid;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static uint8_t hmac_tid_sche_list_for_each_rcu(hmac_tid_list_func func, void *param)
{
    hmac_tid_info_stru *tid_info = NULL;
    struct list_head *tid_list = &g_tid_schedule_list.tid_list.list;

    if (oal_unlikely(list_empty(tid_list))) {
        return OAL_FALSE;
    }

    list_for_each_entry_rcu(tid_info, tid_list, tid_sche_entry) {
        if (func(tid_info, param) == OAL_RETURN) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}
#endif

static inline hmac_tid_info_stru *hmac_sche_tid_info_getter(void *entry)
{
    return oal_container_of(entry, hmac_tid_info_stru, tid_sche_entry);
}

static inline void *hmac_sche_entry_getter(hmac_tid_info_stru *tid_info)
{
    return &tid_info->tid_sche_entry;
}

const hmac_tid_list_ops g_tid_sche_list_ops = {
    .tid_info_getter = hmac_sche_tid_info_getter,
    .entry_getter = hmac_sche_entry_getter,
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    .for_each_rcu = hmac_tid_sche_list_for_each_rcu,
#endif
};

static inline hmac_tid_info_stru *hmac_tx_tid_info_getter(void *entry)
{
    return oal_container_of(entry, hmac_tid_info_stru, tid_tx_entry);
}

static inline void *hmac_tx_entry_getter(hmac_tid_info_stru *tid_info)
{
    return &tid_info->tid_tx_entry;
}

const hmac_tid_list_ops g_tid_tx_list_ops = {
    .tid_info_getter = hmac_tx_tid_info_getter,
    .entry_getter = hmac_tx_entry_getter,
};

void hmac_tid_schedule_init(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_tid_list_init_rcu(&g_tid_schedule_list.tid_list, &g_tid_sche_list_ops);
#else
    hmac_tid_list_init(&g_tid_schedule_list.tid_list, &g_tid_sche_list_ops);
#endif
    hmac_tid_list_init(&g_tid_schedule_list.tid_tx_list, &g_tid_tx_list_ops);
    oal_atomic_set(&g_tid_schedule_list.ring_mpdu_num, 0);
    oal_atomic_set(&g_tid_sche_forbid, OAL_FALSE);
}

static inline oal_bool_enum_uint8 hmac_tid_push_ring_allowed(hmac_tid_info_stru *tid_info)
{
    return oal_atomic_read(&tid_info->tx_ring.inited);
}

static inline void hmac_tid_get_netbuf_list(
    oal_netbuf_head_stru *tid_queue, oal_netbuf_head_stru *netbuf_list, uint32_t delist_limit)
{
    oal_netbuf_head_init(netbuf_list);

    oal_spin_lock_head_bh(tid_queue);

    while (!oal_netbuf_list_empty(tid_queue) && oal_netbuf_list_len(netbuf_list) < delist_limit) {
        oal_netbuf_list_tail_nolock(netbuf_list, oal_netbuf_delist_nolock(tid_queue));
    }

    oal_spin_unlock_head_bh(tid_queue);
}

static void hmac_device_ring_tx_update_netbuf(oal_netbuf_stru *netbuf)
{
    /* 设置8023帧长, 用于设置device ring msdu info的msdu_len字段 */
    mac_get_cb_mpdu_len((mac_tx_ctl_stru *)oal_netbuf_cb(netbuf)) = oal_netbuf_len(netbuf);

#ifndef _PRE_WINDOWS_SUPPORT
    /* device ring tx某些情况下会切换为tid7 device tx发送, 此处统一计算csum */
    if (netbuf->ip_summed == CHECKSUM_PARTIAL) {
        oal_skb_checksum_help(netbuf);
    }
#endif
}

static uint32_t hmac_device_ring_tx_send_netbuf(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf)
{
    frw_event_stru *frw_event = NULL;
    frw_event_mem_stru *frw_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));

    if (oal_unlikely(!frw_event_mem)) {
        oam_error_log0(0, 0, "{hmac_device_ring_tx_send_netbuf::frw_event_alloc_m failed}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    frw_event = frw_get_event_stru(frw_event_mem);
    frw_event_hdr_init(&frw_event->st_event_hdr, FRW_EVENT_TYPE_HOST_DRX, DMAC_TX_HOST_DTX, sizeof(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1, mac_vap->uc_chip_id, mac_vap->uc_device_id, mac_vap->uc_vap_id);
    ((dmac_tx_event_stru *)frw_event->auc_event_data)->pst_netbuf = netbuf;

    /* h2d传输netbuf时, 传输失败的netbuf由hcc统一释放, hmac无需释放 */
    if (oal_unlikely(frw_event_dispatch_event(frw_event_mem) != OAL_SUCC)) {
        oam_warning_log0(0, 0, "{hmac_device_ring_tx_send_netbuf::frw_event_dispatch_event failed}");
        oam_stat_vap_incr(mac_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
    }

    frw_event_free_m(frw_event_mem);

    return OAL_SUCC;
}

static inline void hmac_tid_push_ring_fail_proc(hmac_tid_info_stru *tid_info, oal_netbuf_stru *netbuf)
{
    if (hmac_tx_soft_retry_process(tid_info, netbuf) == OAL_SUCC) {
        return;
    }

    hmac_free_netbuf_list(netbuf);
}

static uint32_t hmac_device_ring_tx(
    hmac_vap_stru *hmac_vap, hmac_tid_info_stru *tid_info, oal_netbuf_stru *netbuf)
{
    uint32_t ret;

    mutex_lock(&tid_info->tx_ring.tx_lock);

    if (oal_atomic_read(&tid_info->ring_tx_mode) != DEVICE_RING_TX_MODE) {
        mutex_unlock(&tid_info->tx_ring.tx_lock);
        oam_warning_log2(0, 0, "hmac_device_ring_tx::usr[%d] tid[%d] mode err", tid_info->user_index, tid_info->tid_no);
        return OAL_FAIL;
    }

    hmac_device_ring_tx_update_netbuf(netbuf);
    ret = hmac_device_ring_tx_send_netbuf(&hmac_vap->st_vap_base_info, netbuf);

    mutex_unlock(&tid_info->tx_ring.tx_lock);

    return ret;
}

static void hmac_tid_push_netbuf_list_to_device_ring(
    hmac_vap_stru *hmac_vap, hmac_tid_info_stru *tid_info, oal_netbuf_head_stru *netbuf_list)
{
    oal_netbuf_stru *netbuf = NULL;

    while ((netbuf = oal_netbuf_delist_nolock(netbuf_list)) != NULL) {
        if (hmac_device_ring_tx(hmac_vap, tid_info, netbuf) != OAL_SUCC) {
            /* 此处FAIL为hmac层异常, 需要处理对应netbuf */
            hmac_tid_push_ring_fail_proc(tid_info, netbuf);
        }
    }
}

uint32_t hmac_tid_push_netbuf_to_device_ring(
    hmac_tid_info_stru *tid_info, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    oal_netbuf_head_stru netbuf_list;

    hmac_tid_get_netbuf_list(&tid_info->tid_queue, &netbuf_list, oal_netbuf_list_len(&tid_info->tid_queue));
    if (oal_unlikely(oal_netbuf_list_empty(&netbuf_list))) {
        return OAL_SUCC;
    }

    hmac_tid_push_netbuf_list_to_device_ring(hmac_vap, tid_info, &netbuf_list);

    return OAL_SUCC;
}

static inline uint32_t hmac_tid_get_ring_avail_size(hmac_msdu_info_ring_stru *tx_ring)
{
    return hal_host_tx_tid_ring_depth_get(tx_ring->base_ring_info.size) - (uint32_t)oal_atomic_read(&tx_ring->msdu_cnt);
}

static inline void hmac_tid_set_chr_report_param(hmac_tid_info_stru *tid_info)
{
    tid_info->tx_ring.tx_ring_stuck_check.last_push_ring_tsf = oal_time_get_stamp_ms(); /* 最后一次入ring的tsf */
    tid_info->tx_ring.tx_ring_stuck_check.chr_report_flag = OAL_FALSE;
}

static uint32_t hmac_tid_push_netbuf_list_to_host_ring(
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info, oal_netbuf_head_stru *netbuf_list)
{
    oal_netbuf_stru *netbuf = NULL;
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    while ((netbuf = oal_netbuf_delist_nolock(netbuf_list)) != NULL) {
        if (hmac_host_ring_tx(hmac_vap, hmac_user, netbuf) != OAL_SUCC) {
            hmac_tid_push_ring_fail_proc(tid_info, netbuf);
            continue;
        }

        if (++tid_info->tx_ring.ring_sync_cnt >= oal_atomic_read(&tid_info->tx_ring.ring_sync_th)) {
            tid_info->tx_ring.ring_sync_cnt = 0;
            hmac_tx_reg_write_ring_info(tid_info, TID_CMDID_ENQUE);
            hmac_tx_sched_trigger(hmac_vap, tid_info);
        }
    }

    hmac_tx_reg_write_ring_info(tid_info, TID_CMDID_ENQUE);
    hal_pm_try_wakeup_allow_sleep();
    return OAL_SUCC;
}

static inline void hmac_tid_recycle_netbuf_list(oal_netbuf_head_stru *tid_queue, oal_netbuf_head_stru *netbuf_list)
{
    oal_spin_lock_head_bh(tid_queue);
    oal_netbuf_queue_splice_tail_init(netbuf_list, tid_queue);
    oal_spin_unlock_head_bh(tid_queue);
}

uint32_t hmac_tid_push_netbuf_to_host_ring(
    hmac_tid_info_stru *tid_info, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    oal_netbuf_head_stru netbuf_list;

    host_cnt_init_record_performance(TX_RING_PROC);
    host_start_record_performance(TX_RING_PROC);

    if (oal_unlikely(!hmac_tid_push_ring_allowed(tid_info))) {
        return OAL_FAIL;
    }

    hmac_tid_get_netbuf_list(&tid_info->tid_queue, &netbuf_list, hmac_tid_get_ring_avail_size(&tid_info->tx_ring));
    if (oal_unlikely(oal_netbuf_list_empty(&netbuf_list))) {
        return OAL_SUCC;
    }

    if (hmac_tid_push_netbuf_list_to_host_ring(hmac_vap, hmac_user, tid_info, &netbuf_list) != OAL_SUCC) {
        hmac_tid_recycle_netbuf_list(&tid_info->tid_queue, &netbuf_list);
        return OAL_FAIL;
    }

    hmac_tid_set_chr_report_param(tid_info);
    host_end_record_performance(host_cnt_get_record_performance(TX_RING_PROC), TX_RING_PROC);

    return OAL_SUCC;
}

typedef uint32_t (* hmac_tid_push_ring)(hmac_tid_info_stru *, hmac_vap_stru *, hmac_user_stru *);
hmac_tid_push_ring hmac_get_tid_push_ring_func(hmac_tid_info_stru *tid_info)
{
    uint8_t ring_tx_mode = (uint8_t)oal_atomic_read(&tid_info->ring_tx_mode);
    hmac_tid_push_ring func = NULL;

    if (oal_likely(ring_tx_mode == HOST_RING_TX_MODE)) {
        func = hmac_tid_push_netbuf_to_host_ring;
    } else if (ring_tx_mode == DEVICE_RING_TX_MODE) {
        func = hmac_tid_push_netbuf_to_device_ring;
    }

    return func;
}

#define HMAC_TID_PUSH_RING_FAIL_DUMP_CNT 30
static inline void hmac_tid_push_ring_fail_handle(hmac_tid_info_stru *tid_info)
{
    if (++tid_info->push_fail_cnt % HMAC_TID_PUSH_RING_FAIL_DUMP_CNT == 0) {
        oam_warning_log4(0, 0, "{hmac_tid_push_ring_fail_handle::user[%d] tid[%d] push ring[%d] fail cnt[%d]!}",
            tid_info->user_index, tid_info->tid_no, oal_atomic_read(&tid_info->ring_tx_mode), tid_info->push_fail_cnt);
    }
}

static inline void hmac_tid_push_ring_fail_reset(hmac_tid_info_stru *tid_info)
{
    tid_info->push_fail_cnt = 0;
}

static uint8_t hmac_tid_push_ring_process(hmac_tid_info_stru *tid_info, void *param)
{
    hmac_user_stru *hmac_user = mac_res_get_hmac_user(tid_info->user_index);
    hmac_vap_stru *hmac_vap = NULL;
    hmac_tid_push_ring func;

    if (oal_unlikely(hmac_user == NULL ||
        hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC)) {
        return OAL_CONTINUE;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->st_user_base_info.uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL || hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP)) {
        return OAL_CONTINUE;
    }

    func = hmac_get_tid_push_ring_func(tid_info);
    if (oal_unlikely(func == NULL)) {
        hmac_tid_push_ring_fail_handle(tid_info);
        return OAL_CONTINUE;
    }

    hmac_tid_push_ring_fail_reset(tid_info);
    func(tid_info, hmac_vap, hmac_user);

    g_pm_wlan_pkt_statis.ring_tx_pkt += (uint32_t)oal_atomic_read(&g_tid_schedule_list.ring_mpdu_num);

    return OAL_CONTINUE;
}

static uint8_t hmac_get_tx_tid_info(hmac_tid_info_stru *tid_info, void *param)
{
    if (oal_unlikely(tid_info == NULL)) {
        oam_error_log0(0, 0, "{hmac_get_tx_tid_info::tid_info NULL}");
        return OAL_RETURN;
    }

    if (!oal_netbuf_list_len(&tid_info->tid_queue)) {
        return OAL_CONTINUE;
    }

    hmac_tid_tx_list_enqueue(tid_info);

    return OAL_CONTINUE;
}

int32_t hmac_tid_schedule_thread(void *p_data)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct sched_param param;

    param.sched_priority = 97; /* 调度优先级97 */
    oal_set_thread_property(current, SCHED_FIFO, &param, -10); /* set nice -10 */

    allow_signal(SIGTERM);

    while (oal_likely(!down_interruptible(&g_tid_schedule_list.tid_sche_sema))) {
#ifdef _PRE_WINDOWS_SUPPORT
        if (oal_kthread_should_stop((PRT_THREAD)p_data)) {
#else
        if (oal_kthread_should_stop()) {
#endif
            break;
        }
        /* rcu protected */
        hmac_tid_schedule_list_for_each(hmac_get_tx_tid_info, NULL);

        /* local */
        hmac_tid_tx_list_for_each(hmac_tid_push_ring_process, NULL);
        hmac_tid_tx_list_clear();
    }
#endif

    return OAL_SUCC;
}

static inline uint8_t hmac_tid_queue_empty(hmac_tid_info_stru *tid_info, void *param)
{
    if (oal_unlikely(tid_info == NULL)) {
        oam_error_log0(0, 0, "{hmac_tid_queue_empty::tid_info NULL}");
        return OAL_RETURN;
    }

    if (oal_netbuf_list_len(&tid_info->tid_queue)) {
        *((uint8_t *)param) = OAL_FALSE;
        return OAL_RETURN;
    }

    return OAL_CONTINUE;
}

uint8_t hmac_is_tid_empty(void)
{
    uint8_t tid_empty = OAL_TRUE;

    if (oal_unlikely(oal_atomic_read(&g_tid_sche_forbid) == OAL_TRUE)) {
        return OAL_TRUE;
    }

    hmac_tid_schedule_list_for_each(hmac_tid_queue_empty, &tid_empty);

    return tid_empty;
}

#define TX_RING_STUCK_CHECK_TIMEOUT_MS 1000
static uint8_t hmac_tx_ring_stuck_check_for_each(hmac_tid_info_stru *tid_info, void *param)
{
    uint32_t cur_time;
    uint32_t delta_time;

    if (oal_unlikely(tid_info == NULL)) {
        oam_error_log0(0, 0, "{hmac_tx_ring_stuck_check_for_each::tid_info NULL}");
        return OAL_RETURN;
    }

    if (tid_info->tx_ring.tx_ring_stuck_check.chr_report_flag == OAL_TRUE) {
        return OAL_CONTINUE;
    }

    cur_time = (uint32_t)oal_time_get_stamp_ms();
    delta_time = (uint32_t)oal_time_get_runtime(tid_info->tx_ring.tx_ring_stuck_check.last_push_ring_tsf, cur_time);
    /* 堵塞触发条件:上次入ring时间超过1s，且ring中的msdu个数不为0 */
    if (delta_time > TX_RING_STUCK_CHECK_TIMEOUT_MS && (oal_atomic_read(&tid_info->tx_ring.msdu_cnt) != 0)) {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_RING_TX, CHR_WIFI_DRV_ERROR_TX_RING_STUCK);
        tid_info->tx_ring.tx_ring_stuck_check.chr_report_flag = OAL_TRUE;
    }

    return OAL_CONTINUE;
}

void hmac_tx_ring_stuck_check(void)
{
    hmac_tid_schedule_list_for_each(hmac_tx_ring_stuck_check_for_each, NULL);
}

static uint8_t hmac_tid_sche_list_tid_info_dump(hmac_tid_info_stru *tid_info, void *param)
{
    if (oal_unlikely(tid_info == NULL)) {
        oam_error_log0(0, 0, "{hmac_tid_sche_list_tid_info_dump::tid_info NULL}");
        return OAL_RETURN;
    }

    oam_warning_log2(0, 0, "{hmac_tid_sche_list_dump::user[%d] tid[%d]}", tid_info->user_index, tid_info->tid_no);

    return OAL_CONTINUE;
}

void hmac_tid_sche_list_dump(uint8_t type)
{
    oam_warning_log1(0, 0, "{hmac_tid_sche_list_dump::type[%d]}", type);
    hmac_tid_schedule_list_for_each(hmac_tid_sche_list_tid_info_dump, NULL);
}
