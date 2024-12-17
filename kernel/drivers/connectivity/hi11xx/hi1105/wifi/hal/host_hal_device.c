/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : host hal device相关功能
 * 作    者 : wifi
 * 创建日期 : 2012年9月18日
 */

#if (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

#include "board.h"
#endif

#include "oal_util.h"
#include "oal_net.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "plat_pm_wlan.h"
#include "host_hal_ring.h"
#include "host_hal_device.h"
#include "host_hal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_HAL_DEVICE_C

hal_host_chip_stru  g_hal_chip;
hal_host_device_stru *hal_get_host_device(uint8_t dev_id)
{
    if (dev_id >= WLAN_DEVICE_MAX_NUM_PER_CHIP) {
        return NULL;
    }

    return &g_hal_chip.hal_device[dev_id];
}

hal_host_device_stru *hal_pcie_down_get_hal_dev(mac_device_stru *mac_device)
{
    mac_vap_stru *mac_vap = NULL;
    mac_vap_stru *up_mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t ret;
    uint8_t vap_num;

    vap_num = mac_device_calc_up_vap_num(mac_device);
    if (vap_num == 1) {
        ret = mac_device_find_up_vap(mac_device, &mac_vap);
        if (ret != OAL_SUCC) {
            return NULL;
        }
    } else if (vap_num ==  2) { // 2: 2vap场景
        ret = mac_device_find_up_vaps(mac_device, up_mac_vap, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
        if (ret != OAL_SUCC || up_mac_vap[0] == NULL || up_mac_vap[1] == NULL) {
            return NULL;
        }

        /* 找到之前的VAP,有可能是STA也有可能是GC，需要根据sta_limit/gc_limit的情况判断 */
        if (mac_device->sta_pcie_probe == WLAN_PCIE_PROBE_LOW) {
            mac_vap = is_legacy_sta(up_mac_vap[0]) ? up_mac_vap[0] : up_mac_vap[1];
        } else if (mac_device->gc_pcie_probe == WLAN_PCIE_PROBE_LOW) {
            mac_vap = is_p2p_cl(up_mac_vap[0]) ? up_mac_vap[0] : up_mac_vap[1];
        }
    }
    if (mac_vap == NULL) {
        return NULL;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return NULL;
    }
    oam_warning_log2(0, OAM_SF_FTM, "{hal_pcie_down_get_hal_dev: vap[%d].hal_dev[%d]}", mac_vap->uc_vap_id,
        hmac_vap->hal_dev_id);
    return hal_get_host_device(hmac_vap->hal_dev_id);
}

uint8_t hal_is_host_forbid_sleep(void)
{
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();
    if ((wlan_pm_info != NULL) && (oal_atomic_read(&wlan_pm_info->forbid_sleep) != 0)) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

uint8_t hal_device_is_in_ddr_rx(uint8_t device_id)
{
    hal_host_device_stru *hal_dev = hal_get_host_device(device_id);
    if ((hal_dev == NULL) || (hal_dev->inited == OAL_FALSE)) {
        return OAL_FALSE;
    }
    if (oal_atomic_read(&hal_dev->rx_mode) == HAL_DDR_RX) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

uint8_t hal_master_is_in_ddr_rx(void)
{
    return hal_device_is_in_ddr_rx(HAL_DEVICE_ID_MASTER);
}

uint8_t hal_slave_is_in_ddr_rx(void)
{
    return hal_device_is_in_ddr_rx(HAL_DEVICE_ID_SLAVE);
}

uint8_t hal_device_is_al_trx(void)
{
    hal_host_device_stru *hal_dev = NULL;
    uint8_t idx;
    for (idx = 0; idx < WLAN_DEVICE_MAX_NUM_PER_CHIP; idx++) {
        hal_dev = hal_get_host_device(idx);
        if (hal_dev->is_al_tx_flag || (hal_dev->st_alrx.en_al_rx_flag != 0)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/*
 * 功能描述  : pingpang队列出队
 * 1.日    期  : 2020年2月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hal_host_rx_mpdu_que_pop(hal_host_device_stru *hal_device,
    oal_netbuf_head_stru *netbuf_head)
{
    hal_rx_mpdu_que         *rx_mpdu = &(hal_device->st_rx_mpdu);
    unsigned long            lock_flag;

    oal_spin_lock_irq_save(&rx_mpdu->st_spin_lock, &lock_flag);
    oal_netbuf_queue_splice_tail_init(&rx_mpdu->ast_rx_mpdu_list[rx_mpdu->cur_idx], netbuf_head);
    rx_mpdu->cur_idx = (rx_mpdu->cur_idx + 1) % RXQ_NUM;
    oal_spin_unlock_irq_restore(&rx_mpdu->st_spin_lock, &lock_flag);
}

/*
 * 功能描述  : 用于ring中的netbuf管理
 * 1.日    期  : 2020年2月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static hal_rx_nodes *hal_init_rx_nodes(uint32_t req_entries, enum dma_data_direction dma_dir)
{
    hal_rx_node  *node = NULL;
    hal_rx_nodes *nodes = NULL;

    node = oal_memalloc(req_entries * sizeof(*node));
    if (node == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_init_rx_nodes:: node alloc fail.}");
        return NULL;
    }

    nodes = oal_memalloc(sizeof(*nodes));
    if (nodes == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_init_rx_nodes:: nodes alloc fail.}");
        kfree(node);
        return NULL;
    }

    nodes->tbl      = node;
    nodes->tbl_size = req_entries;
    nodes->dma_dir  = dma_dir;
    oam_warning_log0(0, OAM_SF_ANY, "{hal_init_rx_nodes:: nodes alloc SUCC.}");

    return nodes;
}

void hal_rx_mpdu_que_init(hal_rx_mpdu_que *rx_mpdu)
{
    uint32_t que_idx;

    rx_mpdu->cur_idx = 0;
    rx_mpdu->process_num_per_round = HMAC_MSDU_RX_MAX_PROCESS_NUM;

    oal_spin_lock_init(&rx_mpdu->st_spin_lock);
    for (que_idx = 0; que_idx < RXQ_NUM; que_idx++) {
        oal_netbuf_list_head_init(&rx_mpdu->ast_rx_mpdu_list[que_idx]);
    }
}

/* 功能描述 : 释放host rx相关资源 */
void hal_device_reset_rx_res(void)
{
    uint8_t idx;
    hal_host_device_stru *hal_dev = NULL;
    for (idx = 0; idx < WLAN_DEVICE_MAX_NUM_PER_CHIP; idx++) {
        hal_dev = hal_get_host_device(idx);
        if (!hal_dev->inited) {
            continue;
        }
        /* 释放资源前,先关host rx中断,避免关wifi下电释放完资源后仍然有中断上报出现异常 */
        hal_pm_try_wakeup_forbid_sleep();
        if (hal_pm_try_wakeup_dev_lock() == OAL_SUCC) {
            hal_host_mac_mask_rx_irq(hal_dev);
            hal_host_mac_clear_rx_irq(hal_dev);
        }
        hal_pm_try_wakeup_allow_sleep();
        hal_rx_alloc_list_free(hal_dev, &hal_dev->host_rx_normal_alloc_list);
        hal_rx_alloc_list_free(hal_dev, &hal_dev->host_rx_small_alloc_list);
        hal_dev->rx_q_inited = OAL_FALSE;
        oal_atomic_set(&(hal_dev->rx_mode), HAL_RAM_RX);
    }
}

void hal_set_alrx_status(uint8_t hal_dev_id, uint8_t status)
{
    hal_host_device_stru *hal_dev = hal_get_host_device(hal_dev_id);

    if (hal_dev == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hal_set_alrx_status: hal device[%d] null.");
        return;
    }
    hal_dev->st_alrx.en_al_rx_flag = status;
    oam_warning_log1(hal_dev_id, OAM_SF_ANY, "{hal_host_device_init::al rx enable [%d].}", status);
    return;
}

void hal_set_altx_status(uint8_t hal_dev_id, uint8_t status)
{
    hal_host_device_stru *hal_dev = hal_get_host_device(hal_dev_id);

    if (hal_dev == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hal_set_altx_status: hal device[%d] null.");
        return;
    }
    hal_dev->is_al_tx_flag = status;
    oam_warning_log1(hal_dev_id, OAM_SF_ANY, "{hal_set_altx_status::al rx enable [%d].}", status);
    return;
}

/*
 * 功能描述  : host hal device初始化函数
 * 1.日    期  : 2020年2月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hal_host_device_init(hal_host_chip_stru *hal_chip, uint8_t dev_id)
{
    hal_host_device_stru *hal_device = NULL;

    hal_device            = &hal_chip->hal_device[dev_id];
    hal_device->chip_id   = hal_chip->chip_id;
    hal_device->device_id = dev_id;
    hal_device->inited    = 0;
    hal_device->host_ba_ring_regs_inited = 0;
    hal_device->intr_inited = 0;
    oal_atomic_set(&(hal_device->rx_mode), HAL_RAM_RX);
    oal_atomic_set(&hal_device->ba_ring_type, INIT_BA_INFO_RING);

    hal_device->rx_nodes = hal_init_rx_nodes(RX_NODES, DMA_FROM_DEVICE);
    if (hal_device->rx_nodes == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_host_device_init::rx nodes fail.}");
        return OAL_FAIL;
    }

    hal_rx_mpdu_que_init(&hal_device->st_rx_mpdu);
#ifdef _PRE_WLAN_FEATURE_FTM
    hal_device->ftm_regs.inited = 0;
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    hal_host_ftm_csi_init(hal_device);
#endif
#ifdef _PRE_WLAN_FEATURE_VSP
    oal_netbuf_head_init(&hal_device->vsp_netbuf_recycle_list);
#endif
    return OAL_SUCC;
}

uint32_t hal_host_device_exit(uint8_t hal_dev_id)
{
    hal_host_device_stru *hal_device = hal_get_host_device(hal_dev_id);

    if (hal_device == NULL) {
        oam_error_log0(0, 0, "{hal_host_device_exit::hal device null!}");
        return OAL_FAIL;
    }

    hal_host_ring_tx_deinit(hal_device);
#ifdef _PRE_WLAN_FEATURE_VSP
    oal_netbuf_list_purge(&hal_device->vsp_netbuf_recycle_list);
#endif
    return OAL_SUCC;
}

/*
 * 功能描述  : host hal层初始化入口函数
 * 1.日    期  : 2020年2月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hal_host_board_init(void)
{
    hal_host_chip_stru *hal_chip = &g_hal_chip;
    uint8_t dev_id;
    uint32_t ret;

    /* tdo 非ddr收帧，不走此接口 */
    memset_s(&g_hal_chip, sizeof(hal_host_chip_stru), 0, sizeof(hal_host_chip_stru));

    hal_chip->chip_id = 0;
    hal_chip->device_nums = WLAN_DEVICE_MAX_NUM_PER_CHIP;
    for (dev_id = 0; dev_id < hal_chip->device_nums; dev_id++) {
        ret = hal_host_device_init(hal_chip, dev_id);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ANY, "hal_device_init failed: dev_id:%d, l_ret:%d.", dev_id, ret);
            return;
        }
    }
    hal_host_chip_irq_init();
    hal_host_access_device_init();
    oam_warning_log0(0, OAM_SF_ANY, "hal_device_init::hal chip init succ.");
    oal_io_print("hal_device_init::hal chip init succ.\r\n");
}
