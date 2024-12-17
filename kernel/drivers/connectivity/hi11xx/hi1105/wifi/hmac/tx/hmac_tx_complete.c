/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Host发送完成模块
 * 作    者 : wifi
 * 创建日期 : 2020年4月23日
 */

#include "hmac_tx_complete.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_host_tx_data.h"
#include "hmac_soft_retry.h"
#include "mac_common.h"
#include "host_hal_device.h"
#include "host_hal_ring.h"
#include "host_hal_ext_if.h"
#include "pcie_host.h"
#include "wlan_spec.h"
#include "host_hal_ops.h"
#include "hmac_vsp_if.h"
#include "hmac_vsp_source.h"
#include "hmac_tid_sche.h"
#include "hmac_tx_switch.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_COMPLETE_C

static uint32_t hmac_tx_complete_netbuf_retrans(hmac_tid_info_stru *tid_info, oal_netbuf_stru *netbuf)
{
    hal_tx_msdu_dscr_info_stru tx_msdu_dscr_info = { 0 };

    if (oal_likely(!mac_get_cb_is_needretry((mac_tx_ctl_stru *)oal_netbuf_cb(netbuf)))) {
        return OAL_FAIL;
    }

    hal_tx_msdu_dscr_info_get(netbuf, &tx_msdu_dscr_info);
    if (oal_likely(tx_msdu_dscr_info.tx_count < HAL_TX_MSDU_MAX_CNT)) {
        return OAL_FAIL;
    }

    if (hmac_tx_soft_retry_process(tid_info, netbuf) != OAL_SUCC) {
        return OAL_FAIL;
    }

    hmac_tx_msdu_update_free_cnt(&tid_info->tx_ring);

    return OAL_SUCC;
}

static void hmac_tx_complete_netbuf_process(
    hmac_tid_info_stru *tid_info, hmac_msdu_info_ring_stru *tx_ring, un_rw_ptr release_ptr)
{
    oal_netbuf_stru *netbuf = hmac_tx_ring_get_netbuf(tx_ring, release_ptr);

    if (oal_unlikely(netbuf == NULL)) {
        oam_error_log3(0, 0, "{hmac_tx_complete_netbuf_process::user[%d] tid[%d] index[%d] netbuf null}",
            tid_info->user_index, tid_info->tid_no, release_ptr.rw_ptr);
        return;
    }

    /*
     * 先进行dma unmap操作, 无效msdu dscr(skb->data)的cache line, 保证软重传获取的数据包含MAC回填值
     * 此时skb->data不在cache中, 软重传访问skb->data会load cache
     */
    hmac_tx_ring_unmap_netbuf(tx_ring, netbuf, release_ptr.st_rw_ptr.bit_rw_ptr);

    /* 高流量(~2.8Gbps)不进行软重传, 避免load cache造成额外开销 */
    if (hmac_tid_ring_tx_opt(tid_info) || hmac_tx_complete_netbuf_retrans(tid_info, netbuf) != OAL_SUCC) {
        hmac_tx_ring_release_netbuf(tx_ring, netbuf, release_ptr.st_rw_ptr.bit_rw_ptr);
    }
}

/*
 * 功能描述 : 发送完成处理ring中所有msdu对应的netbuf, 并修改ring的release ptr
 * 1.日    期 : 2020年4月22日
 *   作    者 : wifi
 *   修改内容 : 新生成函数
 */
static void hmac_tx_complete_msdu_info_process(hmac_tid_info_stru *tid_info, hmac_msdu_info_ring_stru *tx_ring)
{
    un_rw_ptr release_ptr = { .rw_ptr = tx_ring->release_index };
    un_rw_ptr target_ptr = { .rw_ptr = tx_ring->base_ring_info.read_index };

    while (hmac_tx_rw_ptr_compare(target_ptr, release_ptr) == RING_PTR_GREATER) {
        hmac_tx_complete_netbuf_process(tid_info, tx_ring, release_ptr);

        host_cnt_inc_record_performance(TX_BH_PROC);

        hmac_tx_reset_msdu_info(tx_ring, release_ptr);
        hmac_tx_msdu_ring_inc_release_ptr(tx_ring);

        release_ptr.rw_ptr = tx_ring->release_index;
    }
}

/*
 * 功能描述 : 发送完成处理ring中的msdu
 * 1.日    期 : 2020年2月12日
 *   作    者 : wifi
 *   修改内容 : 新生成函数
 * 2.日    期 : 2020年4月22日
 *   作    者 : wifi
 *   修改内容 : 重构函数
 */
void hmac_tx_complete_tid_ring_process(hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info)
{
    hmac_msdu_info_ring_stru *tx_ring = &tid_info->tx_ring;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(hmac_user->st_user_base_info.uc_vap_id);
    uint16_t last_release_ptr = tx_ring->release_index;

    if (oal_unlikely(hmac_vap == NULL)) {
        return;
    }

    if (oal_unlikely(hmac_tx_ring_rw_ptr_check(tx_ring) != OAL_TRUE)) {
        oam_warning_log3(0, OAM_SF_TX, "{hmac_tx_complete_tid_ring_process::tid[%d], rptr[%d], wptr[%d]}",
            tid_info->tid_no, tx_ring->base_ring_info.read_index, tx_ring->base_ring_info.write_index);
        return;
    }

    hmac_tx_complete_msdu_info_process(tid_info, tx_ring);
    hmac_host_ring_tx_resume(hmac_vap, hmac_user, tid_info);

    /*
     * 本次发送完成处理释放过帧才判断是否切换ring, 否则可能存在如下时序(假设ring中有10个帧):
     * 1. 第一次发送完成中断上半部, 此时发帧数5/10
     * 2. 第二次发送完成中断上半部, 此时发帧数10/10
     * 3. 第一次发送完成中断下半部, 读到的read指针为10, netbuf全部释放, ring空, 下发ring切换事件
     * 4. 第二次发送完成中断下半部, 读到的read指针为10, 无netbuf释放, ring空, 下发ring切换事件
     * 导致ring切换事件重复下发
     */
    if (last_release_ptr != tx_ring->release_index) {
        hmac_tx_mode_h2d_switch_process(tid_info);
    }
}

static uint8_t hmac_tx_complete_ring_rptr_valid(hmac_msdu_info_ring_stru *tx_ring, uint32_t rptr_val)
{
    un_rw_ptr rptr = { .rw_ptr = (uint16_t)rptr_val };
    return (rptr_val != HAL_HOST_READL_INVALID_VAL) &&
           (rptr.st_rw_ptr.bit_rw_ptr < hal_host_tx_tid_ring_depth_get(tx_ring->base_ring_info.size));
}

uint32_t hmac_tx_complete_update_tid_ring_rptr(hmac_tid_info_stru *tid_info)
{
    uintptr_t rptr_addr = tid_info->tx_ring.tx_ring_device_info->word_addr[BYTE_OFFSET_3];
    uint32_t rptr_val;

    if (oal_unlikely(!rptr_addr)) {
        return OAL_FAIL;
    }

    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    rptr_val = hal_host_readl(rptr_addr);
    hal_pm_try_wakeup_allow_sleep();
    if (oal_unlikely(!hmac_tx_complete_ring_rptr_valid(&tid_info->tx_ring, rptr_val))) {
        oam_warning_log3(0, 0, "{hmac_tx_complete_update_tid_ring_rptr::user[%d] tid[%d] invalid read_index[%d]}",
            tid_info->user_index, tid_info->tid_no, rptr_val);
        return OAL_FAIL;
    }
    tid_info->tx_ring.base_ring_info.read_index = (uint16_t)rptr_val;

    return OAL_SUCC;
}

OAL_STATIC void hmac_tx_complete_process(hal_host_device_stru *hal_device)
{
    hal_host_tx_complete_process(hal_device);
}

static inline hmac_tid_info_stru *hmac_tx_comp_tid_info_getter(void *entry)
{
    return oal_container_of(entry, hmac_tid_info_stru, tid_tx_comp_entry);
}

static inline void *hmac_tx_comp_entry_getter(hmac_tid_info_stru *tid_info)
{
    return &tid_info->tid_tx_comp_entry;
}
const hmac_tid_list_ops g_tid_tx_comp_list_ops = {
    .tid_info_getter = hmac_tx_comp_tid_info_getter,
    .entry_getter = hmac_tx_comp_entry_getter,
};

hmac_tx_comp_stru g_tx_comp_mgmt = { .tx_comp_thread = NULL};

void hmac_tx_comp_init(void)
{
    uint8_t hal_dev_id;

    for (hal_dev_id = 0; hal_dev_id < WLAN_DEVICE_MAX_NUM_PER_CHIP; hal_dev_id++) {
        hmac_clear_tx_comp_triggered(hal_dev_id);
    }

    hmac_tid_list_init(&g_tx_comp_mgmt.tid_tx_list, &g_tid_tx_comp_list_ops);
}

OAL_STATIC void hmac_tx_comp_thread_process(void)
{
    uint8_t hal_dev_id;

    host_start_record_performance(TX_BH_PROC);

    for (hal_dev_id = 0; hal_dev_id < WLAN_DEVICE_MAX_NUM_PER_CHIP; hal_dev_id++) {
        if (!hmac_get_tx_comp_triggered(hal_dev_id)) {
            continue;
        }

        hmac_clear_tx_comp_triggered(hal_dev_id);

        if (!oal_pcie_link_state_up(HCC_EP_WIFI_DEV)) {
            oam_warning_log0(0, OAM_SF_TX, "{hmac_hal_dev_tx_complete_process::pcie link down!}");
            continue;
        }

        hmac_tx_complete_process(hal_get_host_device(hal_dev_id));
    }

    host_end_record_performance(host_cnt_get_record_performance(TX_BH_PROC), TX_BH_PROC);
}

int32_t hmac_tx_comp_thread(void *p_data)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct sched_param param;

    param.sched_priority = 97; /* set priority: 97 */
    oal_set_thread_property(current, SCHED_FIFO, &param, -10); /* set nice -10 */

    allow_signal(SIGTERM);

    while (oal_likely(!down_interruptible(&g_tx_comp_mgmt.tx_comp_sema))) {
        if (oal_kthread_should_stop()) {
            break;
        }

        hmac_tx_comp_thread_process();
    }
#endif

    return OAL_SUCC;
}
