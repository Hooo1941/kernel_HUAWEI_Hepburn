/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 芯片统计
* 作者       :
 * 创建日期   : 2020年12月24日
 */
#include "hmac_wifi_delay.h"
#include "oal_kernel_file.h"
#include "hmac_user.h"
#include "hmac_tx_data.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_config.h"
#include "mac_common.h"
#include "host_hal_device.h"
#include "oal_hcc_host_if.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WIFI_DELAY_C

#ifdef _PRE_DELAY_DEBUG
uint32_t g_wifi_delay_debug = 0;
uint32_t g_wifi_delay_log = 0;
hmac_wifi_delay_tsf_stru g_wifi_cur_tsf = {0};
oal_netbuf_stru *g_rx_delay_rpt = NULL;
/* rx skb上报工作队列 */
oal_work_stru g_rx_skb_delay_rpt_worker;
oal_netbuf_head_stru g_rx_skb_rpt_list;

oal_netbuf_stru *g_rx_e2e_delay_rpt = NULL;

#define RX_REPORT_LEN 1500
#define FILE_LEN 100

void hmac_wifi_delay_rpt_to_file_uint16(oal_netbuf_stru *netbuf, uint32_t rpt_id, uint32_t line_limit)
{
    oal_netbuf_free(netbuf);
}

void hmac_wifi_delay_rpt_to_file_uint32(oal_netbuf_stru *netbuf, uint32_t rpt_id, uint32_t line_limit)
{
    oal_netbuf_free(netbuf);
}

/* device发送统计数据统一上报接口 */
uint32_t hmac_delay_rpt_event_process(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = frw_get_event_stru(event_mem);
    dmac_tx_event_stru *ctx_event = (dmac_tx_event_stru *)event->auc_event_data;
    oal_netbuf_stru *netbuf = ctx_event->pst_netbuf;
    rpt_ctl_stru *rpt_ctl = NULL;

    if (netbuf == NULL) {
        return OAL_FAIL;
    }

    rpt_ctl = (rpt_ctl_stru*)oal_netbuf_cb(netbuf);
    switch (rpt_ctl->rpt_type) {
        case TX_SKB_RPT:
            hmac_wifi_delay_rpt_to_file_uint16(netbuf, HMAC_DELAY_TX, sizeof(hal_skb_tx_delay_stru));
            break;
        case TX_DSCR_RPT:
            hmac_wifi_delay_rpt_to_file_uint16(netbuf, HMAC_DELAY_TX_DSCR, sizeof(hal_tx_dscr_delay_stru));
            break;
        case RX_DSCR_RPT:
            hmac_wifi_delay_rpt_to_file_uint16(netbuf, HMAC_DELAY_RX_DSCR, sizeof(hal_rx_dscr_delay_stru));
            break;
        case TX_SCHED_RPT:
            hmac_wifi_delay_rpt_to_file_uint32(netbuf, HMAC_SCHED_TX, oal_netbuf_len(netbuf) / 2); // 2: device num
            break;
        default:
            break;
    };

    return OAL_SUCC;
}

/* host发送统计数据上报接口 */
void hmac_wifi_delay_tx_xmit(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    uint32_t curr_tsf;
    hmac_host_get_tsf_lo(hal_get_host_device(0), hmac_vap->hal_vap_id, &curr_tsf);
    set_tx_skb_xmit_delay_trace_time(netbuf, curr_tsf);
}

void hmac_wifi_delay_tx_hcc_device_tx(oal_netbuf_stru *netbuf, frw_event_hdr_stru *pst_event_hdr)
{
    uint32_t curr_tsf;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }
    hmac_host_get_tsf_lo(hal_get_host_device(0), hmac_vap->hal_vap_id, &curr_tsf);
    set_tx_skb_hcc_delay_trace_time(netbuf, curr_tsf);
}

void hmac_wifi_delay_rx_hcc_device_rx(oal_netbuf_stru *netbuf, frw_event_hdr_stru *pst_event_hdr)
{
    uint32_t curr_tsf;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(pst_event_hdr->uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }
    hmac_host_get_tsf_lo(hal_get_host_device(0), hmac_vap->hal_vap_id, &curr_tsf);
    set_rx_skb_hcc_delay_trace_time(netbuf, HOST_RX_SKB_HCC, curr_tsf);
}

void hmac_wifi_srv_tx_skb_notify(oal_netbuf_stru *netbuf)
{
    uint32_t curr_tsf;
    hmac_vap_stru *hmac_vap = NULL;
    struct hcc_header *hdr = (struct hcc_header*)oal_netbuf_data(netbuf);
    struct frw_hcc_extend_hdr *frw_hdr = (struct frw_hcc_extend_hdr *)((uint8_t*)hdr + HCC_HDR_LEN + 1);
    if ((hdr->main_type == HCC_ACTION_TYPE_WIFI && hdr->sub_type == 0) &&
        (frw_hdr->en_nest_type == FRW_EVENT_TYPE_HOST_DRX) && (frw_hdr->uc_nest_sub_type == DMAC_TX_HOST_DTX)) {
        /* CB中的字段未被覆盖 */
        set_tx_skb_ete_delay_trace_time(netbuf, HOST_TX_SKB_XMIT, get_tx_skb_xmit_delay_trace_time(netbuf));
        set_tx_skb_ete_delay_trace_time(netbuf, HOST_TX_SKB_HCC, get_tx_skb_hcc_delay_trace_time(netbuf));

        hmac_vap = mac_res_get_hmac_vap(mac_get_cb_tx_vap_index((mac_tx_ctl_stru*)oal_netbuf_cb(netbuf)));
        if (oal_unlikely(hmac_vap == NULL)) {
            return;
        }

        hmac_host_get_tsf_lo(hal_get_host_device(0), hmac_vap->hal_vap_id, &curr_tsf);
        set_tx_skb_ete_delay_trace_time(netbuf, HOST_TX_SKB_ETE, curr_tsf);
    }
}

void hmac_wifi_srv_rx_skb_notify(oal_netbuf_stru *netbuf)
{
    uint32_t curr_tsf;
    hmac_vap_stru *hmac_vap = NULL;
    struct hcc_header *hdr = (struct hcc_header*)oal_netbuf_data(netbuf);
    struct frw_hcc_extend_hdr *pst_extend_hdr = (struct frw_hcc_extend_hdr *)((uint8_t*)hdr + HCC_HDR_LEN + 1);
    if ((pst_extend_hdr->en_nest_type == FRW_EVENT_TYPE_WLAN_DRX) &&
        (pst_extend_hdr->uc_nest_sub_type == DMAC_WLAN_DRX_EVENT_SUB_TYPE_WOW_RX_DATA)) {
        hmac_vap = mac_res_get_hmac_vap(mac_get_rx_cb_mac_vap_id((mac_rx_ctl_stru*)oal_netbuf_cb(netbuf)));
        if (oal_unlikely(hmac_vap == NULL)) {
            return;
        }
        hmac_host_get_tsf_lo(hal_get_host_device(0), hmac_vap->hal_vap_id, &curr_tsf);
        set_rx_skb_ete_delay_trace_time(netbuf, HOST_RX_SKB_ETE, curr_tsf);
    }
}

/* 端到端时延统计source接口 */
void hmac_wifi_e2e_delay_set_time(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    priv_e2e_trans_stru *tail = NULL;
    uint32_t curr_tsf = 0xffee55aa;
    uint32_t total_len;
    uint32_t tailroom = sizeof(priv_e2e_trans_stru);

    total_len = oal_netbuf_end(netbuf) - oal_netbuf_data(netbuf);
    if (total_len - oal_netbuf_len(netbuf) <= tailroom) {
        return;
    }
    hmac_host_get_tsf_lo(hal_get_host_device(0), hmac_vap->hal_vap_id, &curr_tsf);

    tail = (priv_e2e_trans_stru*)((uint8_t*)oal_netbuf_data(netbuf) + oal_netbuf_len(netbuf));
    tail->magic_head = OAL_DOG_TAG;
    tail->timestamp = curr_tsf;
    oal_netbuf_put(netbuf, tailroom);
}

/* 发送流程统计 */
void hmac_wifi_delay_tx_notify(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    if (g_wifi_delay_debug & HMAC_DELAY_TX) {
        hmac_wifi_delay_tx_xmit(hmac_vap, netbuf);
    } else if (g_wifi_delay_debug &  HMAC_E2E_DELAY_TX) {
        hmac_wifi_e2e_delay_set_time(hmac_vap, netbuf);
    }
}

/* 统计控制接口 */
uint32_t g_rx_e2e_delay_max = 0;
void hmac_register_hcc_callback(uint32_t delay_mask)
{
    oal_netbuf_stru *netbuf = NULL;
    struct hcc_handler *hcc = hcc_get_handler(HCC_EP_WIFI_DEV);
    if (hcc == NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_register_hcc_callback::hcc null.}");
        return;
    }

    g_wifi_delay_debug = delay_mask;
    hcc->p_wifi_srv_tx_skb_notify = NULL;
    hcc->p_wifi_srv_rx_skb_notify = NULL;
    if (delay_mask & HMAC_DELAY_TX) {
        hcc->p_wifi_srv_tx_skb_notify = hmac_wifi_srv_tx_skb_notify;
    }
    if (delay_mask & HMAC_DELAY_RX) {
        hcc->p_wifi_srv_rx_skb_notify = hmac_wifi_srv_rx_skb_notify;
    }
    if (delay_mask == 0) {
        oal_cancel_work_sync(&g_rx_skb_delay_rpt_worker);
        if (g_rx_delay_rpt != NULL) {
            oal_netbuf_list_tail(&g_rx_skb_rpt_list, g_rx_delay_rpt);
            g_rx_delay_rpt = NULL;
        }
        if (g_rx_e2e_delay_rpt != NULL) {
            oal_netbuf_list_tail(&g_rx_skb_rpt_list, g_rx_e2e_delay_rpt);
            g_rx_e2e_delay_rpt = NULL;
        }
        while ((netbuf = oal_netbuf_delist(&g_rx_skb_rpt_list)) != NULL) {
            oal_netbuf_free(netbuf);
        }
    } else {
        g_rx_e2e_delay_max = 0;
        oal_init_work(&g_rx_skb_delay_rpt_worker, hmac_delay_rx_skb_rpt_worker);
        oal_netbuf_list_head_init(&g_rx_skb_rpt_list);
    }
    oam_warning_log1(0, OAM_SF_TX, "{hmac_register_hcc_callback:: set mask 0x%x.}", delay_mask);
}

/* NAPI SOFTIRQ无法打开文件,需提交work延迟处理 */
void hmac_delay_rx_skb_rpt_worker(oal_work_stru *work)
{
    oal_netbuf_stru *netbuf = NULL;
    rpt_ctl_stru *rpt_ctl = NULL;
    while ((netbuf = oal_netbuf_delist(&g_rx_skb_rpt_list)) != NULL) {
        rpt_ctl = (rpt_ctl_stru*)oal_netbuf_cb(netbuf);
        switch (rpt_ctl->rpt_type) {
            case RX_SKB_RPT:
                hmac_wifi_delay_rpt_to_file_uint16(netbuf, HMAC_DELAY_RX, sizeof(hal_skb_rx_delay_stru));
                break;
            case RX_E2E_SKB_RPT:
                hmac_wifi_delay_rpt_to_file_uint16(netbuf, HMAC_E2E_DELAY_RX, sizeof(uint16_t));
                break;
            default:
                break;
        };
    }
}
/* host 单接收流程统计数据上报接口 */
void hmac_delay_rx_skb_rpt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    uint32_t curr_tsf;
    hal_skb_rx_delay_stru *skb_delay = NULL;
    rpt_ctl_stru *rpt_ctl = NULL;
    uint32_t rxskb = get_rx_skb_host_delay_trace_time(netbuf, HAL_RX_SKB_START);
    uint32_t dhcc = get_rx_skb_host_delay_trace_time(netbuf, HAL_RX_SKB_REPORT);
    uint32_t etes = get_rx_skb_host_delay_trace_time(netbuf, HAL_RX_SKB_ETE);
    uint32_t etem = get_rx_skb_host_delay_trace_time(netbuf, HOST_RX_SKB_ETE);
    uint32_t hhcc = get_rx_skb_host_delay_trace_time(netbuf, HOST_RX_SKB_HCC);
    hmac_host_get_tsf_lo(hal_get_host_device(0), hmac_vap->hal_vap_id, &curr_tsf);

    if (g_rx_delay_rpt == NULL) {
        g_rx_delay_rpt = oal_netbuf_alloc(RX_REPORT_LEN + 128, 8, 4); // 128/8: 预留字节, 4: 4字节对齐
    }
    if (g_rx_delay_rpt == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_rx_delay_log::alloc null");
        return;
    }
    if (oal_netbuf_tailroom(g_rx_delay_rpt) < sizeof(hal_skb_rx_delay_stru)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_rx_delay_log::g_rx_delay_rpt rest mem not enough");
        oal_netbuf_free(g_rx_delay_rpt);
        g_rx_delay_rpt = NULL;
        return;
    }
    /* restore */
    skb_delay = (hal_skb_rx_delay_stru*)((uint8_t*)oal_netbuf_data(g_rx_delay_rpt) + oal_netbuf_len(g_rx_delay_rpt));
    skb_delay->skb_len = oal_netbuf_len(netbuf) >> BIT_OFFSET_3;
    skb_delay->rxskb_hcc = dhcc - rxskb;
    skb_delay->hcc_eteslave = etes - dhcc;
    skb_delay->ete_slave_master = etem - etes;
    skb_delay->etemaster_hcc = hhcc - etem;
    skb_delay->hcc_netifrx = curr_tsf - hhcc;

    if (g_wifi_delay_log & HAL_LOG_RX_DELAY_LOG) {
        oam_warning_log4(0, OAM_SF_CFG, "{hmac_delay_rx_skb_rpt::dmac 0x%x hcc 0x%x bus 0x%x hcc 0x%x",
            skb_delay->rxskb_hcc, skb_delay->hcc_eteslave, skb_delay->ete_slave_master, skb_delay->etemaster_hcc);
        oam_warning_log4(0, OAM_SF_CFG, "{hmac_delay_rx_skb_rpt::dmac 0x%x hcc_dev 0x%x ete_dev 0x%x ete_host 0x%x",
            rxskb, dhcc, etes, etem);
        oam_warning_log3(0, OAM_SF_CFG, "{hmac_delay_rx_skb_rpt::hcc_host 0x%x netif_rx 0x%x netif 0x%x",
            hhcc, curr_tsf, skb_delay->hcc_netifrx);
    }
    oal_netbuf_put(g_rx_delay_rpt, sizeof(hal_skb_rx_delay_stru));
    /* report when no space for next store */
    if (oal_netbuf_len(g_rx_delay_rpt) + sizeof(hal_skb_rx_delay_stru) > RX_REPORT_LEN) {
        rpt_ctl = (rpt_ctl_stru*)oal_netbuf_cb(g_rx_delay_rpt);
        rpt_ctl->rpt_type = RX_SKB_RPT;
        oal_netbuf_list_tail(&g_rx_skb_rpt_list, g_rx_delay_rpt);
        g_rx_delay_rpt = NULL;
        oal_workqueue_schedule(&g_rx_skb_delay_rpt_worker);
    }
}

/* host e2e接收流程统计数据上报接口 */
void hmac_e2e_delay_rpt(uint32_t src_tsf, uint32_t cur_tsf)
{
    uint16_t *skb_delay = NULL;
    rpt_ctl_stru *rpt_ctl = NULL;

    if (g_rx_e2e_delay_max > 10000) { // 10000 次统计
        return;
    }
    g_rx_e2e_delay_max++;
    if (g_rx_e2e_delay_rpt == NULL) {
        g_rx_e2e_delay_rpt = oal_netbuf_alloc(RX_REPORT_LEN + 128, 8, 4); // 128/8: 预留字节, 4: 4字节对齐
    }
    if (g_rx_e2e_delay_rpt == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_rx_delay_log::alloc null");
        return;
    }
    if (oal_netbuf_tailroom(g_rx_e2e_delay_rpt) < sizeof(uint16_t)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_rx_delay_log::g_rx_e2e_delay_rpt rest mem not enough");
        oal_netbuf_free(g_rx_e2e_delay_rpt);
        g_rx_e2e_delay_rpt = NULL;
        return;
    }
    /* restore */
    skb_delay = (uint16_t*)((uint8_t*)oal_netbuf_data(g_rx_e2e_delay_rpt) + oal_netbuf_len(g_rx_e2e_delay_rpt));
    *skb_delay = cur_tsf - src_tsf;
    oal_netbuf_put(g_rx_e2e_delay_rpt, sizeof(uint16_t));

    /* report when no space for next store */
    if (oal_netbuf_len(g_rx_e2e_delay_rpt) + sizeof(uint16_t) > RX_REPORT_LEN) {
        rpt_ctl = (rpt_ctl_stru*)oal_netbuf_cb(g_rx_e2e_delay_rpt);
        rpt_ctl->rpt_type = RX_E2E_SKB_RPT;
        oal_netbuf_list_tail(&g_rx_skb_rpt_list, g_rx_e2e_delay_rpt);
        g_rx_e2e_delay_rpt = NULL;
        oal_workqueue_schedule(&g_rx_skb_delay_rpt_worker);
    }
}

void hmac_e2e_delay_rx_skb_rpt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    uint32_t curr_tsf = 0;
    uint32_t tailroom = sizeof(priv_e2e_trans_stru);
    priv_e2e_trans_stru *tail = NULL;
    mac_rx_ctl_stru *cb = (mac_rx_ctl_stru*)oal_netbuf_cb(netbuf);

    if (cb->rx_priv_trans == 0 || oal_netbuf_len(netbuf) <= tailroom) {
        return;
    }

    tail = (priv_e2e_trans_stru*)((uint8_t*)oal_netbuf_data(netbuf) + (oal_netbuf_len(netbuf) - tailroom));
    if (tail->magic_head != OAL_DOG_TAG) {
        return;
    }
    hmac_host_get_tsf_lo(hal_get_host_device(0), hmac_vap->hal_vap_id, &curr_tsf);

    hmac_e2e_delay_rpt(tail->timestamp, curr_tsf);
    oal_netbuf_trim(netbuf, tailroom);
}

/* 接收流程统计 */
void hmac_wifi_delay_rx_notify(oal_net_device_stru *net_dev, oal_netbuf_stru *netbuf)
{
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *vap = (mac_vap_stru *)oal_net_dev_priv(net_dev);
    if (oal_unlikely(vap == NULL)) {
        return;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        return;
    }

    if (g_wifi_delay_debug & HMAC_DELAY_RX) {
        hmac_delay_rx_skb_rpt(hmac_vap, netbuf);
    } else if (g_wifi_delay_debug & HMAC_E2E_DELAY_RX) {
        hmac_e2e_delay_rx_skb_rpt(hmac_vap, netbuf);
    }
}
/* 唤醒dev，获取tsf */
void hmac_host_record_tsf_lo(void)
{
    uint8_t hal_vap_id;
    uint32_t curr_tsf;
    for (hal_vap_id = 0; hal_vap_id < HAL_MAX_VAP_NUM; hal_vap_id++) {
        hal_host_get_tsf_lo(hal_get_host_device(0), hal_vap_id, &curr_tsf);
        g_wifi_cur_tsf.cur_tsf[hal_vap_id] = curr_tsf;
    }
    g_wifi_cur_tsf.host_tsf = (uint32_t)oal_jiffies_to_usecs(OAL_TIME_JIFFY);
}
void hmac_host_get_tsf_lo(hal_host_device_stru *hal_device, uint8_t hal_vap_id, uint32_t *tsf)
{
    uint32_t cur_host_tsf = (uint32_t)oal_jiffies_to_usecs(OAL_TIME_JIFFY);
    *tsf = g_wifi_cur_tsf.cur_tsf[hal_vap_id] +
        (uint32_t)oal_time_get_runtime_us(g_wifi_cur_tsf.host_tsf, cur_host_tsf);
}
#endif