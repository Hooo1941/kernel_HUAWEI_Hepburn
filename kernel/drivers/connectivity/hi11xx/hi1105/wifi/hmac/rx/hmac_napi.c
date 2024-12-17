/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : UAPSD hmac层处理
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#include "mac_vap.h"
#include "mac_frame.h"
#include "hmac_mgmt_ap.h"
#include "hmac_encap_frame_ap.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_rx_data.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_config.h"
#include "external/lpcpu_feature.h"
#include "securec.h"
#include "hmac_wifi_delay.h"
#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_NAPI_C

/*
 * 函 数 名  : hmac_rxdata_update_napi_weight
 * 功能描述  : 动态调整napi weight
 * 1.日    期  : 2019年1月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_rxdata_update_napi_weight(oal_netdev_priv_stru *pst_netdev_priv)
{
#if defined(CONFIG_ARCH_HISI)

    uint32_t now;
    uint8_t uc_new_napi_weight;

    /* 根据pps水线调整napi weight,调整周期1s */
    now = (uint32_t)oal_time_get_stamp_ms();
    if (oal_time_get_runtime(pst_netdev_priv->period_start, now) > NAPI_STAT_PERIOD) {
        pst_netdev_priv->period_start = now;
        if (pst_netdev_priv->period_pkts < NAPI_WATER_LINE_LEV1) {
            uc_new_napi_weight = NAPI_POLL_WEIGHT_LEV1;
        } else if (pst_netdev_priv->period_pkts < NAPI_WATER_LINE_LEV2) {
            uc_new_napi_weight = NAPI_POLL_WEIGHT_LEV2;
        } else if (pst_netdev_priv->period_pkts < NAPI_WATER_LINE_LEV3) {
            uc_new_napi_weight = NAPI_POLL_WEIGHT_LEV3;
        } else {
            uc_new_napi_weight = NAPI_POLL_WEIGHT_MAX;
        }
        if (uc_new_napi_weight != pst_netdev_priv->uc_napi_weight) {
            oam_warning_log3(0, OAM_SF_CFG, "{hmac_rxdata_update_napi_weight::pkts[%d], napi_weight old[%d]->new[%d]",
                pst_netdev_priv->period_pkts, pst_netdev_priv->uc_napi_weight, uc_new_napi_weight);
            pst_netdev_priv->uc_napi_weight = uc_new_napi_weight;
            pst_netdev_priv->st_napi.weight = uc_new_napi_weight;
        }
        pst_netdev_priv->period_pkts = 0;
    }
#endif
}

int32_t hmac_rxdata_polling_softirq_check(int32_t l_weight)
{
    int32_t l_rx_max = l_weight;  // 默认最大接收数为l_weight
#if defined(CONFIG_ARCH_HISI) && defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    /* 当rx_data线程切到大核，但是软中断仍在小核时会出现100%，内核bug */
    /* 因此需要检查这种场景并提前结束本次软中断，引导内核在大核上启动软中断 */
    /* 少上报一帧可以让软中断认为没有更多报文从而防止repoll */
    hmac_rxdata_thread_stru *rxdata_thread = hmac_get_rxdata_thread_addr();
    if (rxdata_thread->uc_allowed_cpus == WLAN_IRQ_AFFINITY_BUSY_CPU) {
        /* soft_irq与rxdata线程不在同一个cpu,那么需要napi complete完成触发soft_irq迁移 */
        if (((1UL << get_cpu()) & ((g_st_fastcpus.bits)[0])) == 0) {
            l_rx_max = (l_weight > 1) ? l_weight - 1 : l_weight;
        }
        put_cpu();
    }
#endif
#endif
    return l_rx_max;
}
int32_t hmac_rxdata_polling(struct napi_struct *pst_napi, int32_t l_weight)
{
    int32_t l_rx_num = 0;
    int32_t l_rx_max;
    uint32_t need_resched = OAL_FALSE;
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_netdev_priv_stru *netdev_priv = oal_container_of(pst_napi, oal_netdev_priv_stru, st_napi);
    if (netdev_priv == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_rxdata_polling: netdev_priv is NULL!");
        return 0;
    }
    g_hisi_softwdt_check.napi_poll_cnt++;
    g_hisi_softwdt_check.napi_poll_time = oal_read_once(OAL_TIME_JIFFY);

    l_rx_max = hmac_rxdata_polling_softirq_check(l_weight);
    while (l_rx_num < l_rx_max) {
        pst_netbuf = oal_netbuf_delist(&netdev_priv->st_rx_netbuf_queue);
        if (pst_netbuf == NULL) {
            /* 队列已空,不需要调度 */
            need_resched = OAL_FALSE;
            break;
        }
        need_resched = OAL_TRUE;
#ifdef _PRE_DELAY_DEBUG
        hmac_wifi_delay_rx_notify(netdev_priv->dev, pst_netbuf);
#endif
#ifdef _PRE_WLAN_PKT_TIME_STAT
        if (DELAY_STATISTIC_SWITCH_ON && IS_NEED_RECORD_DELAY(pst_netbuf, TP_SKB_HMAC_RX)) {
            skbprobe_record_time(pst_netbuf, TP_SKB_HMAC_UPLOAD);
        }
#endif
        /* 清空cb */
        memset_s(oal_netbuf_cb(pst_netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());

        if (netdev_priv->uc_gro_enable == OAL_TRUE) {
            oal_napi_gro_receive(pst_napi, pst_netbuf);
        } else {
            oal_netif_receive_skb(pst_netbuf);
        }
        l_rx_num++;
    }

    /* 队列包量比较少 */
    if (l_rx_num < l_weight) {
        oal_napi_complete(pst_napi);
        /* 没有按照napi weight阈值取帧并且队列未取空,重新出发调度出队 */
        if ((l_rx_max != l_weight) && (need_resched == OAL_TRUE)) {
            g_hisi_softwdt_check.rxshed_napi++;
            hmac_rxdata_sched();  // 可能是人为的少调度,所以需要重新调度一次
        }
    }
    return l_rx_num;
}
