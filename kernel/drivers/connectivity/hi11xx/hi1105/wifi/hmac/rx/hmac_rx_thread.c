/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : UAPSD hmac层处理
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#include "hmac_rx_thread.h"
#include "oal_ext_if.h"
#include "hmac_main.h"
#include "hmac_napi.h"
#include "hmac_host_rx.h"
#if(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_THREAD_C

oal_bool_enum_uint8 hmac_get_rxthread_enable(void)
{
    hmac_rxdata_thread_stru *rxdata_thread = hmac_get_rxdata_thread_addr();

    return rxdata_thread->en_rxthread_enable;
}

void hmac_rxdata_sched(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_rxdata_thread_stru *rxdata_thread = hmac_get_rxdata_thread_addr();
    up(&rxdata_thread->st_rxdata_sema);
#endif
    return;
}

void hmac_rxdata_netbuf_enqueue(oal_netbuf_stru *pst_netbuf)
{
    oal_netdev_priv_stru *pst_netdev_priv;

    pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_netbuf->dev);
    if (pst_netdev_priv->queue_len_max < oal_netbuf_list_len(&pst_netdev_priv->st_rx_netbuf_queue)) {
        oal_netbuf_free(pst_netbuf);
        return;
    }
    oal_netbuf_list_tail(&pst_netdev_priv->st_rx_netbuf_queue, pst_netbuf);
    g_hisi_softwdt_check.napi_enq_cnt++;
    pst_netdev_priv->period_pkts++;
}

static void hmac_exec_rxdata_schedule_process(void)
{
    uint8_t uc_vap_idx, napi_sch;
    mac_device_stru *pst_mac_device = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    oal_netdev_priv_stru *pst_netdev_priv = NULL;
    pst_mac_device = mac_res_get_dev(0);

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++) {
        pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (oal_unlikely(pst_hmac_vap == NULL)) {
            continue;
        }

        if (pst_hmac_vap->pst_net_device == NULL) {
            continue;
        }

        pst_netdev_priv = (oal_netdev_priv_stru *)oal_net_dev_wireless_priv(pst_hmac_vap->pst_net_device);
        if (pst_netdev_priv == NULL) {
            continue;
        }

        if (oal_netbuf_list_len(&pst_netdev_priv->st_rx_netbuf_queue) == 0) {
            continue;
        }

        napi_sch = pst_netdev_priv->uc_napi_enable;
        if (napi_sch) {
            hmac_rxdata_update_napi_weight(pst_netdev_priv);
#ifdef _PRE_CONFIG_NAPI_DYNAMIC_SWITCH
            /* RR性能优化，当napi权重为1时，不做napi轮询处理 */
            if (pst_netdev_priv->uc_napi_weight <= 1) {
                napi_sch = 0;
            }
#endif
        }
        if (napi_sch) {
            g_hisi_softwdt_check.napi_sched_cnt++;
            oal_napi_schedule(&pst_netdev_priv->st_napi);
        } else {
            g_hisi_softwdt_check.netif_rx_cnt++;
            hmac_rxdata_netbuf_delist(pst_netdev_priv);
        }
    }
    g_hisi_softwdt_check.rxdata_cnt++;
}
int32_t hmac_rxdata_thread(void *p_data)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct sched_param param;
    hmac_rxdata_thread_stru *rxdata_thread = hmac_get_rxdata_thread_addr();

    param.sched_priority = 1;  /* 线程优先级 1，同hisi_hcc线程一致  */
    oal_set_thread_property(current, SCHED_FIFO, &param, -10); /* 线程静态优先级 nice  -10  范围-20到19  */
    allow_signal(SIGTERM);
    while (oal_likely(!down_interruptible(&rxdata_thread->st_rxdata_sema))) {
#else
    for (;;) {
#endif
#ifdef _PRE_WINDOWS_SUPPORT
        if (oal_kthread_should_stop((PRT_THREAD)p_data)) {
#else
        if (oal_kthread_should_stop()) {
#endif
            break;
        }
        hmac_exec_rxdata_schedule_process();
    }
    return OAL_SUCC;
}
