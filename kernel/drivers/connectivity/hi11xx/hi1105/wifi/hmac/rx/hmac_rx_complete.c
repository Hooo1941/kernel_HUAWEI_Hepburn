/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Host发送完成模块
 * 作    者 : wifi
 * 创建日期 : 2020年4月23日
 */

#include "hmac_rx_complete.h"
#include "mac_common.h"
#include "host_hal_device.h"
#include "host_hal_ring.h"
#include "host_hal_ext_if.h"
#include "pcie_host.h"
#include "wlan_spec.h"
#include "host_hal_ops.h"
#include "pcie_linux.h"
#include "hmac_multi_netbuf_amsdu.h"


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_COMPLETE_C

hmac_rx_comp_stru g_rx_comp_mgmt = { .rx_comp_thread = NULL};

void hmac_rx_comp_init(void)
{
    uint8_t hal_dev_id;

    for (hal_dev_id = 0; hal_dev_id < WLAN_DEVICE_MAX_NUM_PER_CHIP; hal_dev_id++) {
        hmac_clear_rx_comp_triggered(hal_dev_id);
    }
}

OAL_STATIC void hmac_rx_comp_thread_process(void)
{
    uint8_t hal_dev_id;

    host_start_record_performance(RX_THREAD_PROC);

    for (hal_dev_id = 0; hal_dev_id < WLAN_DEVICE_MAX_NUM_PER_CHIP; hal_dev_id++) {
        if (!hmac_get_rx_comp_triggered(hal_dev_id)) {
            continue;
        }

        hmac_clear_rx_comp_triggered(hal_dev_id);

        hal_host_rx_add_buff(hal_get_host_device(hal_dev_id), HAL_RX_DSCR_NORMAL_PRI_QUEUE);
    }

    host_end_record_performance(host_cnt_get_record_performance(RX_THREAD_PROC), RX_THREAD_PROC);
}

int32_t hmac_rx_comp_thread(void *p_data)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct sched_param param;

    param.sched_priority = 99; /* set priority: 99 */
    oal_set_thread_property(current, SCHED_FIFO, &param, -10);  /* set nice -10 */

    allow_signal(SIGTERM);

    while (oal_likely(!down_interruptible(&g_rx_comp_mgmt.rx_comp_sema))) {
        if (oal_kthread_should_stop()) {
            break;
        }

        hmac_rx_comp_thread_process();
    }
#endif

    return OAL_SUCC;
}
void hmac_rx_add_buff_process(hal_host_device_stru *hal_device)
{
    if (hmac_get_rx_comp_thread_enable()) {
        hmac_rx_comp_trigger_scheduled(hal_device);
    } else {
        hal_host_rx_add_buff(hal_device, HAL_RX_DSCR_NORMAL_PRI_QUEUE);
    }
    return;
}
