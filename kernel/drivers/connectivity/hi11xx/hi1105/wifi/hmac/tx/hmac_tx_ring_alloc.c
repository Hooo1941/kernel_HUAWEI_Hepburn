/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac tx ring内存分配模块
 * 作    者 : wifi
 * 创建日期 : 2021年12月23日
 */

#include "hmac_tx_ring_alloc.h"
#include "hmac_tid_sche.h"
#include "hmac_tid_update.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_config.h"
#include "pcie_linux.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_RING_ALLOC_C

typedef struct {
    uint32_t tx_ring_device_base_addr; /* Device Tx Ring Table的Dev SRAM地址 */
    hmac_tx_ring_device_info_stru tx_ring_device_info[WLAN_ASSOC_USER_MAX_NUM][WLAN_TIDNO_BUTT];
} hmac_tx_ring_device_info_mgmt_stru;

hmac_tx_ring_device_info_mgmt_stru g_tx_ring_device_info_mgmt = { 0 };

static inline uint8_t hmac_get_ring_tx_mode(void)
{
    return hmac_host_ring_tx_enabled() ? HOST_RING_TX_MODE : DEVICE_RING_TX_MODE;
}

static void hmac_tid_info_init(hmac_tid_info_stru *tid_info, uint16_t user_index, uint8_t tid)
{
    memset_s(&tid_info->tx_ring, sizeof(hmac_msdu_info_ring_stru), 0, sizeof(hmac_msdu_info_ring_stru));

    tid_info->tid_no = tid;
    tid_info->user_index = user_index;
    tid_info->is_in_ring_list = OAL_FALSE;
    oal_atomic_set(&tid_info->ring_tx_mode, hmac_get_ring_tx_mode());
    oal_atomic_set(&tid_info->tid_sche_th, 1);
    oal_netbuf_head_init(&tid_info->tid_queue);
    mutex_init(&tid_info->tx_ring.tx_lock);
    mutex_init(&tid_info->tx_ring.tx_comp_lock);
    oal_spin_lock_init(&tid_info->lock);

    if (hmac_tid_schedule_list_enqueue(tid_info) != OAL_SUCC) {
        oam_error_log2(0, 0, "{hmac_tid_info_init::user[%d] tid[%d] enqueue failed!}", user_index, tid);
    }

    oal_atomic_set(&tid_info->inited, OAL_TRUE);
}

static hmac_tx_ring_device_info_stru *hmac_get_tx_ring_device_info(uint16_t user_lut_id, uint8_t tid)
{
    return &g_tx_ring_device_info_mgmt.tx_ring_device_info[user_lut_id][tid];
}

static uint32_t hmac_get_device_tx_ring_base_addr(uint8_t lut_idx, uint8_t tid)
{
    return g_tx_ring_device_info_mgmt.tx_ring_device_base_addr + TX_RING_INFO_LEN * (lut_idx * WLAN_TIDNO_BUTT + tid);
}

static uint32_t hmac_tx_ring_device_addr_init(
    hmac_tx_ring_device_info_stru *tx_ring_device_info, uint8_t user_lut_idx, uint8_t tid)
{
    uint64_t hostva = 0;
    int32_t ret;
    uint32_t word_index;
    uint32_t user_tid_ring_base_addr = hmac_get_device_tx_ring_base_addr(user_lut_idx, tid);

    for (word_index = 0; word_index < TX_RING_INFO_WORD_NUM; word_index++) {
        ret = oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, user_tid_ring_base_addr, &hostva);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_TX, "{hmac_tx_set_user_tid_ring_addr::devca to hostva failed[%d]}", ret);
            return ret;
        }

        tx_ring_device_info->word_addr[word_index] = hostva;
        user_tid_ring_base_addr += sizeof(uint32_t);
    }

    return OAL_SUCC;
}

static uint32_t hmac_tx_ring_device_info_init(
    hmac_tx_ring_device_info_stru *ring_device_info, uint8_t user_lut_index, uint8_t tid)
{
    memset_s(ring_device_info, sizeof(hmac_tx_ring_device_info_stru), 0, sizeof(hmac_tx_ring_device_info_stru));

    return hmac_tx_ring_device_addr_init(ring_device_info, user_lut_index, tid);
}

#define HMAC_TX_RING_MAX_AMSDU_NUM 7
static uint32_t hmac_host_tx_ring_init(hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info, uint8_t tid)
{
    hmac_msdu_info_ring_stru *tx_ring = &tid_info->tx_ring;

    if (!hmac_host_ring_tx_enabled()) {
        return OAL_SUCC;
    }

    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    tx_ring->tx_ring_device_info = hmac_get_tx_ring_device_info(hmac_user->lut_index, tid);
    if (hmac_tx_ring_device_info_init(tx_ring->tx_ring_device_info, hmac_user->lut_index, tid) != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        oam_error_log0(0, OAM_SF_CFG, "{hmac_host_tx_ring_init::tx ring device info init failed!}");
        return OAL_FAIL;
    }
    hal_pm_try_wakeup_allow_sleep();
    tx_ring->base_ring_info.size = hal_host_tx_tid_ring_size_get(HAL_TX_RING_SIZE_GRADE_DOWNGRADE_TWICE, tid);
    if (tx_ring->base_ring_info.size == HAL_HOST_USER_TID_TX_RING_SIZE_ERROR) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_host_tx_ring_init::tx ring size is error!}");
        return OAL_FAIL;
    }

    tx_ring->base_ring_info.read_index = 0;
    tx_ring->base_ring_info.write_index = 0;
    tx_ring->base_ring_info.max_amsdu_num = (tid != WLAN_TIDNO_BCAST) ? HMAC_TX_RING_MAX_AMSDU_NUM : 0;
    tx_ring->tx_msdu_cnt = 0;
    tx_ring->ring_sync_cnt = 0;

    oal_atomic_set(&tx_ring->msdu_cnt, 0);
    oal_atomic_set(&tx_ring->last_period_tx_msdu, 0);
    oal_atomic_set(&tx_ring->ring_sync_th, 1);
    oal_atomic_set(&tx_ring->ring_tx_opt, OAL_FALSE);

    oal_atomic_set(&tx_ring->inited, OAL_TRUE);

    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static uint32_t hmac_tx_dma_alloc_ring_buf(hmac_msdu_info_ring_stru *tx_ring, uint32_t host_ring_buf_size)
{
    dma_addr_t host_ring_dma_addr = 0;
    void *host_ring_vaddr = NULL;
    oal_pci_dev_stru *pcie_dev = oal_get_wifi_pcie_dev();

    if (pcie_dev == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(0, 0, "{hmac_tx_dma_alloc_ring_buf::size[%d]}", host_ring_buf_size);

    host_ring_vaddr = dma_alloc_coherent(&pcie_dev->dev, host_ring_buf_size,
                                         &host_ring_dma_addr, GFP_ATOMIC);
    if (host_ring_vaddr == NULL) {
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    tx_ring->host_ring_buf = (uint8_t *)host_ring_vaddr;
    tx_ring->host_ring_dma_addr = (uintptr_t)host_ring_dma_addr;
    tx_ring->host_ring_buf_size = host_ring_buf_size;

    return OAL_SUCC;
}

static uint32_t hmac_tx_dma_release_ring_buf(hmac_msdu_info_ring_stru *tx_ring)
{
    oal_pci_dev_stru *pcie_dev = oal_get_wifi_pcie_dev();

    oam_warning_log1(0, OAM_SF_TX, "{hmac_tx_dma_release_ring_buf::size[%d]}", tx_ring->host_ring_buf_size);

    if ((pcie_dev == NULL) || (tx_ring->host_ring_buf == NULL) ||
        (!tx_ring->host_ring_dma_addr) || (!tx_ring->host_ring_buf_size)) {
        return OAL_FAIL;
    }

    dma_free_coherent(&pcie_dev->dev, tx_ring->host_ring_buf_size,
                      tx_ring->host_ring_buf, (uintptr_t)tx_ring->host_ring_dma_addr);

    return OAL_SUCC;
}
#else
static uint32_t hmac_tx_dma_alloc_ring_buf(hmac_msdu_info_ring_stru *tx_ring, uint32_t host_ring_buf_size)
{
    dma_addr_t host_ring_dma_addr;
    void *host_ring_vaddr = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, host_ring_buf_size, OAL_FALSE);

    if (host_ring_vaddr == NULL) {
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    tx_ring->host_ring_buf = (uint8_t *)host_ring_vaddr;
    tx_ring->host_ring_dma_addr = (uintptr_t)host_ring_vaddr;
    tx_ring->host_ring_buf_size = host_ring_buf_size;

    return OAL_SUCC;
}

static uint32_t hmac_tx_dma_release_ring_buf(hmac_msdu_info_ring_stru *tx_ring)
{
    oal_mem_free_m(tx_ring->host_ring_buf, OAL_TRUE);

    return OAL_SUCC;
}
#endif

uint32_t hmac_alloc_tx_ring(hmac_msdu_info_ring_stru *tx_ring)
{
    uint32_t ret;
    uint16_t host_ring_size = hal_host_tx_tid_ring_depth_get(tx_ring->base_ring_info.size);
    uint32_t host_ring_buf_size = host_ring_size * HAL_TX_RING_MSDU_INFO_LEN;
    uint32_t host_netbuf_buf_size = host_ring_size * sizeof(oal_netbuf_stru *);

    ret = hmac_tx_dma_alloc_ring_buf(tx_ring, host_ring_buf_size);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_tx_host_ring_alloc::dma alloc host buf failed[%d]}", ret);
        return ret;
    }

    tx_ring->netbuf_list = (oal_netbuf_stru **)oal_memalloc(host_netbuf_buf_size);
    if (tx_ring->netbuf_list == NULL) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tx_host_ring_alloc::kmalloc netbuflist failed}");
        hmac_tx_dma_release_ring_buf(tx_ring);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    memset_s(tx_ring->host_ring_buf, host_ring_buf_size, 0, host_ring_buf_size);
    memset_s(tx_ring->netbuf_list, host_netbuf_buf_size, 0, host_netbuf_buf_size);

    return OAL_SUCC;
}

/* 多线程可能导致tx ring deinit后仍然alloc tx ring，检查inited为false时要释放tx ring */
static uint32_t hmac_tx_ring_deinit_check(hmac_msdu_info_ring_stru *tx_ring)
{
    if (oal_atomic_read(&tx_ring->inited) == OAL_TRUE) {
        return OAL_SUCC;
    }

    hmac_tx_host_ring_release_nolock(tx_ring);
    return OAL_FAIL;
}

uint32_t hmac_alloc_tx_ring_by_tid(hmac_msdu_info_ring_stru *tx_ring, uint8_t tid_no)
{
    uint8_t ring_size_grade;

    // tx ring内存申请梯度&申请失败异常处理
    for (ring_size_grade = HAL_TX_RING_SIZE_GRADE_DEFAULT; ring_size_grade < HAL_TX_RING_SIZE_GRADE_BUTT;
         ring_size_grade++) {
        tx_ring->base_ring_info.size = hal_host_tx_tid_ring_size_get(ring_size_grade, tid_no);
        if (tx_ring->base_ring_info.size == HAL_HOST_USER_TID_TX_RING_SIZE_ERROR) {
            return OAL_FAIL;
        }
        if (hmac_alloc_tx_ring(tx_ring) == OAL_SUCC) {
            return hmac_tx_ring_deinit_check(tx_ring);
        }
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_RING_TX, CHR_WIFI_DRV_ERROR_TX_RING_ALLOC);
    oam_error_log1(0, OAM_SF_TX, "{hmac_alloc_tx_ring::tx ring alloc fail([%d]times)!}", HAL_TX_RING_SIZE_GRADE_BUTT);
    return OAL_FAIL;
}

static void hmac_reset_tx_ring(hmac_msdu_info_ring_stru *tx_ring)
{
    memset_s(&tx_ring->base_ring_info, sizeof(msdu_info_ring_stru), 0, sizeof(msdu_info_ring_stru));
    tx_ring->host_ring_buf = NULL;
    tx_ring->host_ring_dma_addr = 0;
    tx_ring->host_ring_buf_size = 0;
    tx_ring->release_index = 0;
    oal_atomic_set(&tx_ring->msdu_cnt, 0);
    if (tx_ring->netbuf_list != NULL) {
        oal_free(tx_ring->netbuf_list);
        tx_ring->netbuf_list = NULL;
    }
}

uint32_t hmac_ring_tx_init(hmac_user_stru *hmac_user, uint8_t tid)
{
    hmac_tid_info_stru *tid_info = &hmac_user->tx_tid_info[tid];

    if ((hmac_ring_tx_enabled() != OAL_TRUE) || tid == WLAN_TIDNO_BCAST) {
        return OAL_SUCC;
    }

    hmac_tid_info_init(tid_info, hmac_user->st_user_base_info.us_assoc_id, tid);

    if (hmac_host_tx_ring_init(hmac_user, tid_info, tid) != OAL_SUCC) {
        hmac_tid_info_deinit(tid_info);
        oam_error_log2(0, 0, "{hmac_ring_tx_init::user[%d] tid[%d] host ring init failed}", tid_info->user_index, tid);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_RING_TX, CHR_WIFI_DRV_ERROR_TX_RING_INIT);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

uint32_t hmac_alloc_tx_ring_after_inited(hmac_tid_info_stru *tid_info, hmac_msdu_info_ring_stru *tx_ring)
{
    if (oal_atomic_read(&tid_info->tx_ring.inited) != OAL_TRUE) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_alloc_tx_ring::tx ring not inited!}");
        return OAL_FAIL;
    }

    mutex_lock(&tid_info->tx_ring.tx_comp_lock);
    if (hmac_alloc_tx_ring_by_tid(tx_ring, tid_info->tid_no) != OAL_SUCC) {
        hmac_reset_tx_ring(tx_ring);
        mutex_unlock(&tid_info->tx_ring.tx_comp_lock);
        return OAL_FAIL;
    }

    hmac_tx_reg_write_ring_info(tid_info, TID_CMDID_CREATE);
    mutex_unlock(&tid_info->tx_ring.tx_comp_lock);

    hmac_tid_update_list_enqueue(tid_info);

    return OAL_SUCC;
}

void hmac_tx_ring_release_all_netbuf(hmac_msdu_info_ring_stru *tx_ring)
{
    un_rw_ptr release_ptr = { .rw_ptr = tx_ring->release_index };
    un_rw_ptr target_ptr = { .rw_ptr = tx_ring->base_ring_info.write_index };
    oal_netbuf_stru *netbuf = NULL;

    while (hmac_tx_rw_ptr_compare(target_ptr, release_ptr) == RING_PTR_GREATER) {
        netbuf = hmac_tx_ring_get_netbuf(tx_ring, release_ptr);
        if (netbuf != NULL) {
            hmac_tx_ring_unmap_netbuf(tx_ring, netbuf, release_ptr.st_rw_ptr.bit_rw_ptr);
            hmac_tx_ring_release_netbuf(tx_ring, netbuf, release_ptr.st_rw_ptr.bit_rw_ptr);
        }

        hmac_tx_reset_msdu_info(tx_ring, release_ptr);
        hmac_tx_msdu_ring_inc_release_ptr(tx_ring);

        release_ptr.rw_ptr = tx_ring->release_index;
    }
}

static inline void hmac_tx_ring_release_netbuf_record(hmac_msdu_info_ring_stru *tx_ring)
{
    if (tx_ring->netbuf_list == NULL) {
        return;
    }

    oal_free(tx_ring->netbuf_list);
    tx_ring->netbuf_list = NULL;
}

void hmac_tx_host_ring_release_nolock(hmac_msdu_info_ring_stru *tx_ring)
{
    if (!hmac_tx_ring_alloced(tx_ring)) {
        return;
    }

    hmac_tx_ring_release_all_netbuf(tx_ring);
    hmac_tx_ring_release_netbuf_record(tx_ring);

    if (hmac_tx_dma_release_ring_buf(tx_ring) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_host_ring_release::host buf release failed}");
    }

    hmac_reset_tx_ring(tx_ring);
}

void hmac_tx_host_ring_release(hmac_msdu_info_ring_stru *tx_ring)
{
    mutex_lock(&tx_ring->tx_lock);
    mutex_lock(&tx_ring->tx_comp_lock);

    hmac_tx_host_ring_release_nolock(tx_ring);

    mutex_unlock(&tx_ring->tx_comp_lock);
    mutex_unlock(&tx_ring->tx_lock);
}

static void hmac_tx_tid_info_dump(hmac_tid_info_stru *tid_info)
{
    oam_warning_log4(0, 0, "hmac_tx_tid_info_dump::user[%d] tid[%d] ring_tx_mode[%d] msdu_cnt[%d]",
        tid_info->user_index, tid_info->tid_no,
        oal_atomic_read(&tid_info->ring_tx_mode), oal_atomic_read(&tid_info->tx_ring.msdu_cnt));

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oam_warning_log3(0, 0, "hmac_tx_tid_info_dump::release[%d] rptr[%d] wptr[%d]",
        READ_ONCE(tid_info->tx_ring.release_index), READ_ONCE(tid_info->tx_ring.base_ring_info.read_index),
        READ_ONCE(tid_info->tx_ring.base_ring_info.write_index));
#else
    oam_warning_log3(0, 0, "hmac_tx_tid_info_dump::release[%d] rptr[%d] wptr[%d]",
        tid_info->tx_ring.release_index, tid_info->tx_ring.base_ring_info.read_index,
        tid_info->tx_ring.base_ring_info.write_index);
#endif
}

#define HMAC_WAIT_TX_RING_EMPTY_MAX_CNT 25
void hmac_wait_tx_ring_empty(hmac_tid_info_stru *tid_info)
{
    uint8_t wait_cnt = 0;

    while (oal_atomic_read(&tid_info->tx_ring.msdu_cnt) && wait_cnt < HMAC_WAIT_TX_RING_EMPTY_MAX_CNT) {
        if (++wait_cnt == HMAC_WAIT_TX_RING_EMPTY_MAX_CNT) {
            hmac_tx_tid_info_dump(tid_info);
        }
        oal_msleep(2); // 2: sleep 2ms
    }
}

static inline void hmac_tid_info_queue_release(oal_netbuf_head_stru *tid_queue)
{
    oal_netbuf_stru *netbuf = NULL;

    while ((netbuf = hmac_tid_netbuf_dequeue(tid_queue)) != NULL) {
        oal_netbuf_free(netbuf);
    }
}

void hmac_tid_info_deinit(hmac_tid_info_stru *tid_info)
{
    oal_spin_lock_bh(&tid_info->lock);

    if (!hmac_tid_info_inited(tid_info)) {
        oal_spin_unlock_bh(&tid_info->lock);
        return;
    }

    oal_atomic_set(&tid_info->inited, OAL_FALSE);
    hmac_tid_info_queue_release(&tid_info->tid_queue);

    oal_spin_unlock_bh(&tid_info->lock);

    hmac_tid_schedule_list_delete(tid_info);
}

static uint32_t hmac_wait_ring_tx_done(hmac_tid_info_stru *tid_info)
{
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    hmac_tx_reg_write_ring_info(tid_info, TID_CMDID_STOP_TX);
    hal_pm_try_wakeup_allow_sleep();

    oal_msleep(20); /* wait 20ms for mac to complete tx */

    return OAL_SUCC;
}

static void hmac_host_tx_ring_deinit(hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info)
{
    hmac_vap_stru *hmac_vap = NULL;

    if (!hmac_host_ring_tx_enabled() || !oal_atomic_read(&tid_info->tx_ring.inited)) {
        return;
    }

    oal_atomic_set(&tid_info->tx_ring.inited, OAL_FALSE);

    if (hmac_wait_ring_tx_done(tid_info) != OAL_SUCC) {
        hmac_wait_tx_ring_empty(tid_info);
    }

    hmac_tid_update_list_delete(tid_info);
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() == OAL_SUCC) {
        hmac_tx_reg_write_ring_info(tid_info, TID_CMDID_DEL);
    }
    hal_pm_try_wakeup_allow_sleep();
    hmac_tx_host_ring_release(&tid_info->tx_ring);

    hmac_vap = mac_res_get_hmac_vap(hmac_user->st_user_base_info.uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    hmac_host_ring_tx_resume(hmac_vap, hmac_user, tid_info);
}

void hmac_ring_tx_deinit(hmac_user_stru *hmac_user, uint8_t tid)
{
    hmac_tid_info_stru *tid_info = &hmac_user->tx_tid_info[tid];

    if ((hmac_ring_tx_enabled() != OAL_TRUE) || tid == WLAN_TIDNO_BCAST) {
        return;
    }

    hmac_tid_info_deinit(tid_info);
    hmac_host_tx_ring_deinit(hmac_user, tid_info);
}

void hmac_user_tx_ring_deinit(hmac_user_stru *hmac_user)
{
    uint32_t tid;

    if (hmac_ring_tx_enabled() != OAL_TRUE) {
        return;
    }

    hmac_tid_sche_list_dump(HMAC_TID_SCHE_LIST_BEFORE_DEL_USER);
    for (tid = 0; tid < WLAN_TID_MAX_NUM; tid++) {
        hmac_ring_tx_deinit(hmac_user, tid);
    }
    hmac_tid_sche_list_dump(HMAC_TID_SCHE_LIST_AFTER_DEL_USER);
}

#define get_low_32bits(val)  ((uint32_t)(((uint64_t)(val)) & 0x00000000FFFFFFFFUL))
#define get_high_32bits(val) ((uint32_t)((((uint64_t)(val)) & 0xFFFFFFFF00000000UL) >> 32UL))
static void hmac_sync_host_ba_ring_addr(uint8_t hal_dev_id)
{
    hal_host_ring_ctl_stru *ba_ring_ctl = &hal_get_host_device(hal_dev_id)->host_ba_info_ring;
    host_ba_ring_info_sync_stru host_ba_ring_info = {
        .hal_dev_id = hal_dev_id,
        .size = ba_ring_ctl->entries,
        .lsb = get_low_32bits(ba_ring_ctl->devva),
        .msb = get_high_32bits(ba_ring_ctl->devva),
    };

    (void)hmac_config_send_event(mac_res_get_mac_vap(0), WLAN_CFGID_HOST_BA_INFO_RING_SYN,
                                 sizeof(host_ba_ring_info_sync_stru), (uint8_t *)&host_ba_ring_info);
}

uint32_t hmac_set_tx_ring_device_base_addr(frw_event_mem_stru *frw_mem)
{
    frw_event_stru *frw_event = NULL;
    mac_d2h_tx_ring_base_addr_stru *d2h_tx_ring_base_addr = NULL;

    if (frw_mem == NULL) {
        return OAL_FAIL;
    }

    if (!hmac_host_ring_tx_enabled()) {
        return OAL_SUCC;
    }

    frw_event = frw_get_event_stru(frw_mem);
    d2h_tx_ring_base_addr = (mac_d2h_tx_ring_base_addr_stru *)frw_event->auc_event_data;

    g_tx_ring_device_info_mgmt.tx_ring_device_base_addr = d2h_tx_ring_base_addr->tx_ring_base_addr;

    hal_host_ba_ring_regs_init(HAL_DEVICE_ID_MASTER);
    hal_host_ba_ring_regs_init(HAL_DEVICE_ID_SLAVE);
    hmac_sync_host_ba_ring_addr(HAL_DEVICE_ID_SLAVE);

    return OAL_SUCC;
}
