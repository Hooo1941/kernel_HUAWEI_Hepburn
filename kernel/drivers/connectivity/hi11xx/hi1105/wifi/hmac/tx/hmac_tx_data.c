/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : HMAC BSS AP TX 主文件
 * 作    者 : wifi
 * 创建日期 : 2012年11月12日
 * 1.日    期  : 2012年11月12日
 *   作    者  : wifi
 *   修改内容  : 创建文件
 * 2.日    期  : 2013年1月8日
 *   修改内容  : 文件名修改为hmac_tx_data.c
 */
#include "hmac_vap.h"
#include "oal_net.h"
#include "frw_ext_if.h"
#include "hmac_tx_data.h"
#include "oam_stat_wifi.h"
#include "hmac_tx_amsdu.h"
#include "mac_frame.h"
#include "mac_data.h"
#include "hmac_frag.h"
#include "hmac_11i.h"
#include "wlan_chip.h"
#include "hmac_hid2d.h"
#include "hmac_wapi.h"
#include "hmac_edca_opt.h"
#include "hmac_traffic_classify.h"
#include "hmac_user.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_stat.h"
#include "hmac_tcp_opt.h"
// 此头文件UT需要编译使用，不作删除
#include "hisi_customize_wifi.h"
#include "hmac_wifi_delay.h"
#include "hmac_auto_adjust_freq.h"  // 为发包统计数据准备
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_ps.h"
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif

#include "hmac_tx_msdu_dscr.h"

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
#include "hmac_rr_performance.h"
#endif

#include "hmac_host_tx_data.h"
#include "hmac_ht_self_cure.h"

#include "securec.h"
#if defined(_PRE_WLAN_FEATURE_WMMAC)
#include "hmac_wmmac.h"
#endif
#ifdef _PRE_WLAN_FEATURE_VSP
#include "hmac_vsp.h"
#endif
#include "hmac_tcp_ack_buf.h"
#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))
#include "hmac_soft_encrypt.h"
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
#include "hmac_pwl.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_DATA_C
/*
 * definitions of king of games feature
 */
#ifdef CONFIG_NF_CONNTRACK_MARK
#define VIP_APP_VIQUE_TID WLAN_TIDNO_VIDEO
#define VIP_APP_MARK      0x5a
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#define pktmark(p)       (((struct sk_buff *)(p))->mark)
#define pktsetmark(p, m) ((struct sk_buff *)(p))->mark = (m)
#else /* !2.6.0 */
#define pktmark(p)       (((struct sk_buff *)(p))->nfmark)
#define pktsetmark(p, m) ((struct sk_buff *)(p))->nfmark = (m)
#endif /* 2.6.0 */
#else  /* CONFIG_NF_CONNTRACK_MARK */
#define pktmark(p) 0
#define pktsetmark(p, m)
#endif /* CONFIG_NF_CONNTRACK_MARK */

/* 2 全局变量定义 */
OAL_STATIC uint8_t g_uc_ac_new = 0;

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
hmac_tx_ampdu_hw_stru g_st_ampdu_hw = { 0 };
#endif

wlan_trx_switch_stru g_trx_switch = { 0 };

mac_small_amsdu_switch_stru g_st_small_amsdu_switch = { 0 };
mac_rx_dyn_bypass_extlna_stru g_st_rx_dyn_bypass_extlna_switch = { 0 };

mac_small_amsdu_switch_stru *mac_vap_get_small_amsdu_switch(void)
{
    return &g_st_small_amsdu_switch;
}

mac_rx_dyn_bypass_extlna_stru *mac_vap_get_rx_dyn_bypass_extlna_switch(void)
{
    return &g_st_rx_dyn_bypass_extlna_switch;
}
/* 3 函数实现 */
/*
 * 函 数 名  : hmac_tx_is_dhcp
 * 功能描述  : 判断是否是dhcp帧
 * 1.日    期  : 2014年6月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tx_is_dhcp(mac_ether_header_stru *pst_ether_hdr)
{
    mac_ip_header_stru *puc_ip_hdr;

    puc_ip_hdr = (mac_ip_header_stru *)(pst_ether_hdr + 1);

    return mac_is_dhcp_port(puc_ip_hdr, oal_net2host_short(puc_ip_hdr->us_tot_len));
}

/*
 * 函 数 名  : hmac_tx_report_dhcp_and_arp
 * 功能描述  : 上报dhcp和arp request
 * 1.日    期  : 2014年6月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_tx_report_dhcp_and_arp(mac_vap_stru *pst_mac_vap,
    mac_ether_header_stru *pst_ether_hdr, uint16_t us_ether_len)
{
    oal_bool_enum_uint8 en_flg;

    switch (oal_host2net_short(pst_ether_hdr->us_ether_type)) {
        case ETHER_TYPE_ARP:
            en_flg = OAL_TRUE;
            break;

        case ETHER_TYPE_IP:
            en_flg = hmac_tx_is_dhcp(pst_ether_hdr);
            break;

        default:
            en_flg = OAL_FALSE;
            break;
    }

    if (en_flg && oam_report_dhcp_arp_get_switch()) {
        if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            oam_report_eth_frame(pst_ether_hdr->auc_ether_dhost, (uint8_t *)pst_ether_hdr, us_ether_len,
                OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        } else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            oam_report_eth_frame(pst_mac_vap->auc_bssid, (uint8_t *)pst_ether_hdr, us_ether_len,
                OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        } else {
        }
    }
}

/*
 * 函 数 名  : hmac_tx_report_eth_frame
 * 功能描述  : 将以太网过来的帧上报SDT
 * 1.日    期  : 2014年5月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_tx_report_eth_frame(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    uint16_t us_user_idx = 0;
    mac_ether_header_stru *pst_ether_hdr = NULL;
    uint32_t ret;
    uint8_t auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oal_switch_enum_uint8 en_eth_switch = oam_ota_get_switch(OAM_OTA_SWITCH_TX_ETH_FRAME);
    mac_vap_stru *pst_mac_vap = &pst_hmac_vap->st_vap_base_info;

    /* 统计以太网下来的数据包统计 */
    hmac_stat_vap_tx_netbuf(pst_hmac_vap, pst_netbuf);

    if (oal_likely(en_eth_switch != OAL_SWITCH_ON)) {
        return;
    }
    /* 获取目的用户资源池id和用户，用于过滤 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);
        if (oal_unlikely(pst_ether_hdr == NULL)) {
            oam_error_log0(0, OAM_SF_TX, "{hmac_tx_report_eth_frame::ether_hdr is null!\r\n");
            return;
        }

        ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_ether_hdr->auc_ether_dhost, &us_user_idx);
        if (ret == OAL_ERR_CODE_PTR_NULL) {
            oam_error_log1(0, OAM_SF_TX, "{hmac_tx_report_eth_frame::find user return ptr null!!\r\n", ret);
            return;
        }

        if (ret == OAL_FAIL) {
            /* 如果找不到用户，该帧可能是dhcp或者arp request，需要上报 */
            hmac_tx_report_dhcp_and_arp(pst_mac_vap, pst_ether_hdr, (uint16_t)oal_netbuf_len(pst_netbuf));
            return;
        }

        oal_set_mac_addr(&auc_user_macaddr[0], pst_ether_hdr->auc_ether_dhost);
    } else if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (pst_mac_vap->us_user_nums == 0) {
            return;
        }
        pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);
        if (oal_unlikely(pst_ether_hdr == NULL)) {
            oam_error_log0(0, OAM_SF_TX, "{hmac_tx_report_eth_frame::ether_hdr is null!\r\n");
            return;
        }
        /* 如果找不到用户，该帧可能是dhcp或者arp request，需要上报 */
        hmac_tx_report_dhcp_and_arp(pst_mac_vap, pst_ether_hdr, (uint16_t)oal_netbuf_len(pst_netbuf));
        us_user_idx = pst_mac_vap->us_assoc_vap_id;
        oal_set_mac_addr(&auc_user_macaddr[0], pst_mac_vap->auc_bssid);
    }

    /* 将以太网下来的帧上报 */
    ret = oam_report_eth_frame(&auc_user_macaddr[0], oal_netbuf_data(pst_netbuf), (uint16_t)oal_netbuf_len(pst_netbuf),
        OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_tx_report_eth_frame::oam_report_eth_frame return err: 0x%x.}\r\n", ret);
    }
}

/*
 * 函 数 名  : oal_free_netbuf_list
 * 功能描述  : 释放一个netbuf链，此链中的skb或者都来自lan，或者都来自wlan
 * 1.日    期  : 2012年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_free_netbuf_list(oal_netbuf_stru *netbuf)
{
    oal_netbuf_stru *next = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;

    if (oal_unlikely(!netbuf)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_free_netbuf_list::netbuf is null}");
        return;
    }

    while (netbuf) {
        next = oal_netbuf_list_next(netbuf);
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        /* 如果frame_header部分是内存池申请的才从内存池释放，采用skb内部自身的，就不需要再释放 */
        if (mac_get_cb_frame_header_addr(tx_ctl) != NULL && mac_get_cb_80211_mac_head_type(tx_ctl) == 0) {
            oal_mem_free_m(mac_get_cb_frame_header_addr(tx_ctl), OAL_TRUE);
        }

        mac_get_cb_frame_header_addr(tx_ctl) = NULL;
        oal_netbuf_free(netbuf);

        netbuf = next;
    }
}

#ifdef _PRE_WLAN_FEATURE_HS20
/*
 * 函 数 名  : hmac_tx_set_qos_map
 * 功能描述  : 根据对端发送的QoS_Map项更新TID
 * 1.日    期  : 2015年9月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static void hmac_tx_set_qos_map(
    hmac_vap_stru *hmac_vap, mac_ip_header_stru *ip_header, mac_tx_ctl_stru *pst_tx_ctl, uint8_t *tid)
{
    uint8_t uc_dscp;
    uint8_t uc_idx;

    /* 从IP TOS字段寻找DSCP优先级 */
    /* ---------------------------------
      tos位定义
      ---------------------------------
    |    bit7~bit2      | bit1~bit0 |
    |    DSCP优先级     | 保留      |
    --------------------------------- */
    uc_dscp = ip_header->uc_tos >> WLAN_DSCP_PRI_SHIFT;
    if ((hmac_vap->st_cfg_qos_map_param.uc_num_dscp_except > 0) &&
        (hmac_vap->st_cfg_qos_map_param.uc_num_dscp_except <= MAX_DSCP_EXCEPT) &&
        (hmac_vap->st_cfg_qos_map_param.uc_valid)) {
        for (uc_idx = 0; uc_idx < hmac_vap->st_cfg_qos_map_param.uc_num_dscp_except; uc_idx++) {
            if (uc_dscp == hmac_vap->st_cfg_qos_map_param.auc_dscp_exception[uc_idx]) {
                *tid = hmac_vap->st_cfg_qos_map_param.auc_dscp_exception_up[uc_idx];
                mac_get_cb_frame_type(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
                mac_get_cb_frame_subtype(pst_tx_ctl) = MAC_DATA_HS20;
                mac_get_cb_is_needretry(pst_tx_ctl) = OAL_TRUE;
                hmac_vap->st_cfg_qos_map_param.uc_valid = 0;
                return;
            }
        }
    }

    for (uc_idx = 0; uc_idx < MAX_QOS_UP_RANGE; uc_idx++) {
        if ((uc_dscp < hmac_vap->st_cfg_qos_map_param.auc_up_high[uc_idx]) &&
            (uc_dscp > hmac_vap->st_cfg_qos_map_param.auc_up_low[uc_idx])) {
            *tid = uc_idx;
            mac_get_cb_frame_type(pst_tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
            mac_get_cb_frame_subtype(pst_tx_ctl) = MAC_DATA_HS20;
            mac_get_cb_is_needretry(pst_tx_ctl) = OAL_TRUE;
            hmac_vap->st_cfg_qos_map_param.uc_valid = 0;
            return;
        } else {
            *tid = 0;
        }
    }
    pst_hmac_vap->st_cfg_qos_map_param.uc_valid = 0;
}
#endif  // _PRE_WLAN_FEATURE_HS20

/*
 * 函 数 名  : hmac_set_tid_based_pktmark
 * 功能描述  : 从lan过来报文的业务识别
 * 1.日    期  : 2020年4月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_set_tid_based_pktmark(oal_netbuf_stru *pst_buf, uint8_t *puc_tid,
    mac_tx_ctl_stru *pst_tx_ctl)
{
#ifdef CONFIG_NF_CONNTRACK_MARK
    /*
     * the king of game feature will mark packets
     * and we will use VI queue to send these packets.
     */
    if (pktmark(pst_buf) == VIP_APP_MARK) {
        *puc_tid = VIP_APP_VIQUE_TID;
        pst_tx_ctl->bit_is_needretry = OAL_TRUE;
        return OAL_TRUE;
    }
#endif
    return OAL_FALSE;
}
uint8_t hmac_tx_classify_tcp(hmac_vap_stru *hmac_vap, mac_ip_header_stru *ip_header, uint16_t ip_len,
    mac_tx_ctl_stru *tx_ctl, uint8_t tid_num)
{
    uint8_t curr_tid = tid_num;
    uint16_t ip_header_len = ip_header_len(ip_header->ip_header_len);
    mac_tcp_header_stru *tcp_hdr = NULL;
    if (ip_len < ip_header_len + sizeof(mac_tcp_header_stru)) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_tx_classify_tcp:ip_len=%d", ip_len);
        return curr_tid;
    }
    tcp_hdr = (mac_tcp_header_stru *)(((uint8_t *)ip_header) + ip_header_len);
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
    if (oal_value_eq_any2(MAC_CHARIOT_NETIF_PORT, tcp_hdr->us_dport, tcp_hdr->us_sport)) {
        curr_tid = WLAN_TIDNO_BCAST;
        mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
        mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_CHARIOT_SIG;
        mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
        return curr_tid;
    }
#endif
    if (oal_netbuf_is_tcp_ack((oal_ip_header_stru *)ip_header) == OAL_TRUE) {
        /* option3:SYN FIN RST URG有为1的时候不缓存 */
        if ((tcp_hdr->uc_flags) & (FIN_FLAG_BIT | RESET_FLAG_BIT | URGENT_FLAG_BIT)) {
            mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_URGENT_TCP_ACK;
        } else if ((tcp_hdr->uc_flags) & SYN_FLAG_BIT) {
            mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
            mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_TCP_SYN;
        } else {
            mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_NORMAL_TCP_ACK;
        }
    } else if ((!is_legacy_vap(&(hmac_vap->st_vap_base_info))) &&
       (oal_ntoh_16(tcp_hdr->us_sport) == MAC_WFD_RTSP_PORT)) {
        mac_get_cb_frame_type(tx_ctl) = WLAN_DATA_BASICTYPE;
        mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
        mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_RTSP;
        curr_tid = WLAN_TIDNO_BCAST;
    } else {
        mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_BUTT;
    }

    return curr_tid;
}
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
void hmac_adjust_edca_opt_tx_ptks(hmac_user_stru *hmac_user,
    uint8_t curr_tid, mac_ip_header_stru *ip_header, uint16_t ip_len)
{
    if (((ip_header->uc_protocol == MAC_UDP_PROTOCAL) &&
        (hmac_user->aaul_txrx_data_stat[WLAN_WME_TID_TO_AC(curr_tid)][WLAN_TX_UDP_DATA] <
        (HMAC_EDCA_OPT_PKT_NUM + BYTE_OFFSET_10))) ||
        ((ip_header->uc_protocol == MAC_TCP_PROTOCAL) &&
        (hmac_user->aaul_txrx_data_stat[WLAN_WME_TID_TO_AC(curr_tid)][WLAN_TX_TCP_DATA] <
        (HMAC_EDCA_OPT_PKT_NUM + BYTE_OFFSET_10)))) {
        hmac_edca_opt_tx_pkts_stat(hmac_user, curr_tid, ip_header, ip_len);
    }
}
#endif
OAL_STATIC void hmac_tx_data_set_dhcp_cb(mac_tx_ctl_stru *tx_ctl)
{
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_DHCP;
    mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
}
OAL_STATIC void hmac_tx_data_set_dns_cb(mac_tx_ctl_stru *tx_ctl)
{
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_DNS;
    mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
}
OAL_STATIC uint8_t hmac_tx_classify_lan_to_wlan_ip_handle(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_ether_header_stru *ether_header, mac_tx_ctl_stru *tx_ctl, uint16_t netbuf_len)
{
    /* 从IP TOS字段寻找优先级 */
    /* ----------------------------------------------------------------------
                tos位定义
       ----------------------------------------------------------------------
            | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
            | 包优先级  | 时延 | 吞吐量 | 可靠性 | 传输成本 | 保留 |
       ---------------------------------------------------------------------- */
    mac_ip_header_stru *ip_header = NULL;
    uint8_t curr_tid = 0;
    uint16_t remain_len = netbuf_len - sizeof(mac_ether_header_stru); /* 上层函数保证减法不会翻转 */
    uint16_t ip_len;
    if (remain_len < sizeof(mac_ip_header_stru)) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_tx_classify_lan_to_wlan_ip_handle:netbuf_len=%d", netbuf_len);
        return curr_tid;
    }
    ip_header = (mac_ip_header_stru *)(ether_header + 1); /* 偏移一个以太网头，取ip头 */
    ip_len = oal_net2host_short(ip_header->us_tot_len);
    if (remain_len < ip_len) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_tx_classify_lan_to_wlan_ip_handle:remain_len=%d, ip_len=%d",
            remain_len, ip_len);
        return curr_tid;
    }
#ifdef _PRE_WLAN_FEATURE_HS20
    if (hmac_vap->st_cfg_qos_map_param.uc_valid) {
        hmac_tx_set_qos_map(hmac_vap, ip_header, tx_ctl, &curr_tid);
        return curr_tid;
    }
#endif  // _PRE_WLAN_FEATURE_HS20
    if ((ip_header->uc_tos >> WLAN_DSCP_PRI_SHIFT) >= HMAC_MAX_DSCP_VALUE_NUM) {
        return curr_tid;
    }
    curr_tid = ip_header->uc_tos >> WLAN_IP_PRI_SHIFT;
    if (OAL_SWITCH_ON == mac_mib_get_TxTrafficClassifyFlag(&hmac_vap->st_vap_base_info)) {
        if (curr_tid != 0) {
            return curr_tid;
        }
        /* RTP RTSP 限制只在 P2P上才开启识别功能 */
        if (!is_legacy_vap(&(hmac_vap->st_vap_base_info))) {
            hmac_tx_traffic_classify(hmac_user, tx_ctl, ip_header, ip_len, &curr_tid);
        }
    }
    /* 如果是DHCP帧，则进入VO队列发送 */
    if (mac_is_dhcp_port(ip_header, ip_len) == OAL_TRUE) {
        curr_tid = WLAN_TIDNO_BCAST;
        hmac_tx_data_set_dhcp_cb(tx_ctl);
    } else if (mac_is_dns(ip_header, ip_len) == OAL_TRUE) {
        hmac_tx_data_set_dns_cb(tx_ctl);
    } else if (ip_header->uc_protocol == MAC_TCP_PROTOCAL) { /* 对于chariot信令报文进行特殊处理，防止断流 */
        curr_tid = hmac_tx_classify_tcp(hmac_vap, ip_header, ip_len, tx_ctl, curr_tid);
    }
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    if ((hmac_vap->uc_edca_opt_flag_ap == OAL_TRUE) &&
        (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP)) {
        /* mips优化:解决开启业务统计性能差10M问题 */
        hmac_adjust_edca_opt_tx_ptks(hmac_user, curr_tid, ip_header, ip_len);
    }
#endif
    return curr_tid;
}

OAL_STATIC uint8_t hmac_tx_classify_lan_to_wlan_ipv6_handle(mac_ether_header_stru *ether_header,
    mac_tx_ctl_stru *tx_ctl)
{
    /* 从IPv6 traffic class字段获取优先级 */
    /* ----------------------------------------------------------------------
        IPv6包头 前32为定义
     -----------------------------------------------------------------------
    | 版本号 | traffic class   | 流量标识 |
    | 4bit   | 8bit(同ipv4 tos)|  20bit   |
    ----------------------------------------------------------------------- */
    uint32_t ipv6_hdr = *((uint32_t *)(ether_header + 1)); /* 偏移一个以太网头，取ip头 */
    uint32_t ipv6_pri = (oal_net2host_long(ipv6_hdr) & WLAN_IPV6_PRIORITY_MASK) >> WLAN_IPV6_PRIORITY_SHIFT;
    uint8_t curr_tid = (uint8_t)(ipv6_pri >> WLAN_IP_PRI_SHIFT);
    /* 如果是DHCPV6帧，则进入VO队列发送 */
    oal_ipv6hdr_stru *ipv6_header = (oal_ipv6hdr_stru *)(ether_header + 1); /* 偏移一个以太网头，取ip头 */
    uint32_t payload_len = oal_net2host_short(ipv6_header->payload_len);
    if (mac_is_dhcp6((oal_ipv6hdr_stru *)(ether_header + 1), payload_len) == OAL_TRUE) {
        curr_tid = WLAN_TIDNO_BCAST;
        mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
        mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_DHCPV6;
        mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
    }
    return curr_tid;
}

OAL_STATIC uint8_t hmac_tx_classify_lan_to_wlan_pae_handle(hmac_vap_stru *hmac_vap,
    mac_ether_header_stru *ether_header, mac_tx_ctl_stru *tx_ctl)
{
    /* 如果是EAPOL帧，则进入VO队列发送 */
    uint8_t curr_tid = WLAN_TIDNO_BCAST;
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_EAPOL;
    mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
    /* 如果是4 次握手设置单播密钥，则设置tx cb 中bit_is_eapol_key_ptk 置一，dmac 发送不加密 */
    if (OAL_TRUE == mac_is_eapol_key_ptk((mac_eapol_header_stru *)(ether_header + 1))) {
        mac_get_cb_is_eapol_key_ptk(tx_ctl) = OAL_TRUE;
    }
    oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CONN,
                     "{hmac_tx_classify_lan_to_wlan:: EAPOL tx : uc_tid=%d,IS_EAPOL_KEY_PTK=%d.}",
                     curr_tid, mac_get_cb_is_eapol_key_ptk(tx_ctl));
    return curr_tid;
}

static uint8_t hmac_tx_classify_lan_to_wlan_pppoe_handle(mac_tx_ctl_stru *tx_ctl)
{
    uint8_t curr_tid = WLAN_TIDNO_BCAST;
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_PPPOE;
    mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
    return curr_tid;
}

static uint8_t hmac_tx_classify_lan_to_wlan_wapi_handle(mac_tx_ctl_stru *tx_ctl)
{
    uint8_t curr_tid = WLAN_TIDNO_BCAST;
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_WAPI;
    mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
    return curr_tid;
}

OAL_STATIC uint8_t hmac_tx_classify_lan_to_wlan_eth_type_handle(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl)
{
    mac_ether_header_stru *ether_header = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    oal_vlan_ethhdr_stru *vlan_ethhdr = NULL;
    uint16_t netbuf_len = oal_netbuf_get_len(netbuf);
    uint16_t vlan_tci;
    uint8_t curr_tid = 0;

    if (netbuf_len < sizeof(mac_ether_header_stru)) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_tx_classify_lan_to_wlan_eth_type_handle:netbuf_len=%d", netbuf_len);
        return curr_tid;
    }
    /* 获取tid 并对data type进行赋值处理 */
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_NUM;
    switch (ether_header->us_ether_type) {
        case oal_host2net_short(ETHER_TYPE_IP):
            curr_tid = hmac_tx_classify_lan_to_wlan_ip_handle(hmac_vap, hmac_user, ether_header, tx_ctl, netbuf_len);
            break;
        case oal_host2net_short(ETHER_TYPE_IPV6):
            curr_tid = hmac_tx_classify_lan_to_wlan_ipv6_handle(ether_header, tx_ctl);
            break;
        case oal_host2net_short(ETHER_TYPE_PAE):
            curr_tid = hmac_tx_classify_lan_to_wlan_pae_handle(hmac_vap, ether_header, tx_ctl);
            break;
        case oal_host2net_short(ETHER_TYPE_ARP):
        /* 如果是ARP帧，则进入VO队列发送 */
            wlan_chip_update_arp_tid(&curr_tid);
            mac_get_cb_frame_subtype(tx_ctl) = mac_get_arp_type_by_arphdr((oal_eth_arphdr_stru *)(ether_header + 1),
                netbuf_len - sizeof(mac_ether_header_stru));
            mac_get_cb_is_needretry(tx_ctl) = OAL_TRUE;
            break;
        /* TDLS帧处理，建链保护，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_TDLS):
            curr_tid = WLAN_TIDNO_BCAST;
            mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_TDLS;
            break;
        /* PPPOE帧处理，建链保护(发现阶段, 会话阶段)，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_PPP_DISC):
        case oal_host2net_short(ETHER_TYPE_PPP_SES):
            curr_tid = hmac_tx_classify_lan_to_wlan_pppoe_handle(tx_ctl);
            break;
#ifdef _PRE_WLAN_FEATURE_WAPI
        case oal_host2net_short(ETHER_TYPE_WAI):
            curr_tid = hmac_tx_classify_lan_to_wlan_wapi_handle(tx_ctl);
            break;
#endif
        case oal_host2net_short(ETHER_TYPE_VLAN):
            /* 获取vlan tag的优先级 */
            vlan_ethhdr = (oal_vlan_ethhdr_stru *)oal_netbuf_data(netbuf);
            /* ------------------------------------------------------------------
                802.1Q(VLAN) TCI(tag control information)位定义
             -------------------------------------------------------------------
            |Priority | DEI  | Vlan Identifier |
            | 3bit    | 1bit |      12bit      |
             ------------------------------------------------------------------ */
            vlan_tci = oal_net2host_short(vlan_ethhdr->h_vlan_TCI);
            curr_tid = vlan_tci >> OAL_VLAN_PRIO_SHIFT; /* 右移13位，提取高3位优先级 */
            break;
        default:
            break;
    }
    return curr_tid;
}
/*
 * 函 数 名  : hmac_tx_classify_lan_to_wlan
 * 功能描述  : 从lan过来报文的业务识别
 * 1.日    期  : 2013年10月10日
 *   修改内容  : 新生成函数
 */
void hmac_tx_classify_lan_to_wlan(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, uint8_t *tid_num)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint8_t curr_tid;

    if (hmac_set_tid_based_pktmark(netbuf, tid_num, tx_ctl) == OAL_TRUE) {
        return;
    }
    curr_tid = hmac_tx_classify_lan_to_wlan_eth_type_handle(hmac_vap, hmac_user, netbuf, tx_ctl);

    /* 出参赋值 */
    *tid_num = curr_tid;
}

/*
 * 函 数 名  : hmac_tx_update_tid
 * 功能描述  : 根据算法需求，tid = 1, 3, 5, 7的，分别更新为0, 2, 4, 6
 *             如果WMM功能关闭，直接填为DMAC_WMM_SWITCH_TID
 * 1.日    期  : 2014年3月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE void hmac_tx_update_tid(oal_bool_enum_uint8 en_wmm, uint8_t *puc_tid)
{
    if (oal_likely(en_wmm == OAL_TRUE)) { /* wmm使能 */
        *puc_tid = (*puc_tid < WLAN_TIDNO_BUTT) ? wlan_tos_to_tid(*puc_tid) : WLAN_TIDNO_BCAST;
    } else { /* wmm不使能 */
        *puc_tid = MAC_WMM_SWITCH_TID;
    }
}
/*
 * 函 数 名  : hmac_tx_wmm_acm
 * 功能描述  : 根据热点配置ACM，重新选择队列
 * 1.日    期  : 2015年11月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_tx_wmm_acm(oal_bool_enum_uint8 en_wmm, hmac_vap_stru *pst_hmac_vap, uint8_t *puc_tid)
{
    uint8_t uc_ac;

    if (oal_any_null_ptr2(pst_hmac_vap, puc_tid)) {
        return OAL_FALSE;
    }

    if (en_wmm == OAL_FALSE) {
        return OAL_FALSE;
    }

    uc_ac = WLAN_WME_TID_TO_AC(*puc_tid);
    g_uc_ac_new = uc_ac;
    while ((g_uc_ac_new != WLAN_WME_AC_BK) &&
           (OAL_TRUE == mac_mib_get_QAPEDCATableMandatory(&pst_hmac_vap->st_vap_base_info, g_uc_ac_new))) {
        switch (g_uc_ac_new) {
            case WLAN_WME_AC_VO:
                g_uc_ac_new = WLAN_WME_AC_VI;
                break;

            case WLAN_WME_AC_VI:
                g_uc_ac_new = WLAN_WME_AC_BE;
                break;

            default:
                g_uc_ac_new = WLAN_WME_AC_BK;
                break;
        }
    }

    if (g_uc_ac_new != uc_ac) {
        *puc_tid = WLAN_WME_AC_TO_TID(g_uc_ac_new);
    }

    return OAL_TRUE;
}
#ifdef _PRE_WLAN_FEATURE_VSP
static inline void hmac_tx_classify_vsp_change_tidno(hmac_user_stru *hmac_user, uint8_t *tidno)
{
    if (hmac_is_vsp_tid(hmac_user, *tidno)) {
        *tidno = WLAN_TIDNO_VIDEO;
    }
}
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
/* 如果组播聚合打开，则组播数据需要走tid3, 为了reorder处理，其余be帧走tid0 */
OAL_INLINE void hmac_tx_classify_mcast_ampdu_remove_tidno(hmac_vap_stru *hmac_vap, uint8_t *tidno)
{
    if (mac_get_mcast_ampdu_switch() == OAL_TRUE && (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (*tidno == WLAN_TIDNO_ANT_TRAINING_LOW_PRIO)) {
        *tidno = WLAN_TIDNO_BEST_EFFORT;
    }
}
#endif
OAL_STATIC OAL_INLINE void hmac_tx_classify_get_tid(mac_device_stru *mac_dev,
    hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *tx_ctl, uint8_t *tidno)
{
    /* 如果使能了vap流等级，则采用设置的vap流等级 */
    if (mac_dev->en_vap_classify == OAL_TRUE) {
        *tidno = mac_mib_get_VAPClassifyTidNo(&hmac_vap->st_vap_base_info);
    }

    /* 如果ap关闭了WMM，则所有报文入BE 队列 */
    if (!hmac_vap->st_vap_base_info.en_vap_wmm) {
        *tidno = WLAN_TIDNO_BEST_EFFORT;
    }

    /* tid7供VIP帧使用，不建立VO聚合，普通数据帧tid改为tid6 */
    if ((*tidno == WLAN_TIDNO_BCAST) && (!mac_get_cb_is_vipframe(tx_ctl))) {
        *tidno = WLAN_TIDNO_VOICE;
    }
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    hmac_tx_classify_mcast_ampdu_remove_tidno(hmac_vap, tidno);
#endif
}

OAL_STATIC void hmac_tx_wmmac_classify(hmac_vap_stru *pst_hmac_vap, mac_tx_ctl_stru *tx_ctl,
    mac_device_stru *pst_mac_dev, mac_user_stru *pst_user, uint8_t *tid_num)
{
#if defined(_PRE_WLAN_FEATURE_WMMAC)
    if (g_en_wmmac_switch) {
        uint8_t uc_ac_num;
        uc_ac_num = WLAN_WME_TID_TO_AC(*tid_num);
        /* 如果ACM位为1，且对应AC的TS没有建立成功，则将该AC的数据全部放到BE队列发送 */
        /* 非vip在此处处理，vip在dmac处理 */
        if (hmac_need_degrade_for_wmmac(pst_hmac_vap, tx_ctl, pst_user, uc_ac_num, *tid_num) == OAL_TRUE) {
            *tid_num = WLAN_TIDNO_BEST_EFFORT;
        }
    } else {
#endif  // defined(_PRE_WLAN_FEATURE_WMMAC)
        hmac_tx_wmm_acm(pst_mac_dev->en_wmm, pst_hmac_vap, tid_num);

        /*  由于1151支持VO聚合, VIP报文和VO报文都放在tid 6聚合传输, 导致VIP报文无法以基本速率发送。
           因此, 将VIP报文放在tid 7, 与实际VO业务区分, 保证VIP报文的传输可靠性。 */
        /* mp12正常数据只使用了4个tid:0 1 5 6 */
        if ((!mac_get_cb_is_vipframe(tx_ctl)) || (pst_mac_dev->en_wmm == OAL_FALSE)) {
            hmac_tx_update_tid(pst_mac_dev->en_wmm, tid_num);
        }
#if defined(_PRE_WLAN_FEATURE_WMMAC)
    }
#endif
}
void hmac_update_arp_tid_mp16(uint8_t *tid)
{
    *tid = WLAN_TIDNO_BCAST;
}
/*
 * 函 数 名  : hmac_tx_classify
 * 功能描述  : 以太网包 业务识别
 * 1.日    期  : 2013年5月2日
 *   修改内容  : 新生成函数
 */
void hmac_tx_classify(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *pst_buf, mac_tx_ctl_stru *tx_ctl)
{
    uint8_t uc_tid = 0;
    mac_device_stru *pst_mac_dev = NULL;
    uint16_t ether_type = mac_get_cb_ether_type(tx_ctl);

    /* 非qos下同样需要对EAPOL进行业务识别 */
    hmac_tx_classify_lan_to_wlan(pst_hmac_vap, hmac_user, pst_buf, &uc_tid);
    /* 非QoS站点，直接返回，默认走TID6 */
    if (oal_unlikely(hmac_user->st_user_base_info.st_cap_info.bit_qos != OAL_TRUE)) {
        /* 非qos的WAPI协商帧需要入VIP队列走device tx */
        /* 非qos的EAPOL帧需要走device tx */
        if ((ether_type == oal_host2net_short(ETHER_TYPE_WAI)) ||
            (ether_type == oal_host2net_short(ETHER_TYPE_PAE))) {
            /* 设置ac和tid到cb字段 */
            mac_get_cb_wme_tid_type(tx_ctl) = uc_tid;
            mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_TID_TO_AC(uc_tid);
        }
        oam_info_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_tx_classify::user isn't a QoS sta.}");
        return;
    }

    pst_mac_dev = mac_res_get_dev(hmac_user->st_user_base_info.uc_device_id);
    if (oal_unlikely(pst_mac_dev == NULL)) {
        oam_warning_log0(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_tx_classify::pst_mac_dev null.}");
        return;
    }

    hmac_tx_wmmac_classify(pst_hmac_vap, tx_ctl, pst_mac_dev, &(hmac_user->st_user_base_info), &uc_tid);

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        /* 如果是游戏标记数据：若支持网络切片队列，进NS队列；此外，进VO队列 */
        hmac_hiex_judge_is_game_marked_enter_to_vo(pst_hmac_vap, hmac_user, pst_buf, &uc_tid);
    }
#endif
    hmac_tx_classify_get_tid(pst_mac_dev, pst_hmac_vap, tx_ctl, &uc_tid);
#ifdef _PRE_WLAN_FEATURE_VSP
    hmac_tx_classify_vsp_change_tidno(hmac_user, &uc_tid);
#endif

    /* 设置ac和tid到cb字段 */
    mac_get_cb_wme_tid_type(tx_ctl) = uc_tid;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_TID_TO_AC(uc_tid);
}

/*
 * 函 数 名  : hmac_rx_dyn_bypass_extlna_switch
 * 功能描述  : 动态bypas外置LNA开关
 * 1.日    期  : 2018年6月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_rx_dyn_bypass_extlna_switch(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    mac_device_stru *pst_mac_device = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t limit_throughput_high, limit_throughput_low, ret;
    uint32_t throughput_mbps = oal_max(tx_throughput_mbps, rx_throughput_mbps);
    oal_bool_enum_uint8 en_is_pm_test;
    mac_rx_dyn_bypass_extlna_stru *rx_extlna = mac_vap_get_rx_dyn_bypass_extlna_switch();

    /* 如果定制化不支持根据吞吐bypass外置LNA */
    if (rx_extlna->uc_ini_en == OAL_FALSE) {
        return;
    }

    /* 每秒吞吐量门限 */
    if ((rx_extlna->us_throughput_high != 0) && (rx_extlna->us_throughput_low != 0)) {
        limit_throughput_high = rx_extlna->us_throughput_high;
        limit_throughput_low = rx_extlna->us_throughput_low;
    } else {
        limit_throughput_high = WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_HIGH;
        limit_throughput_low = WLAN_DYN_BYPASS_EXTLNA_THROUGHPUT_THRESHOLD_LOW;
    }
    if (throughput_mbps > limit_throughput_high) {
        /* 高于100M,非低功耗测试场景 */
        en_is_pm_test = OAL_FALSE;
    } else if (throughput_mbps < limit_throughput_low) {
        /* 低于50M,低功耗测试场景 */
        en_is_pm_test = OAL_TRUE;
    } else {
        /* 介于50M-100M之间,不作切换 */
        return;
    }

    /* 需要切换时，满足条件后通知device操作 */
    pst_mac_device = mac_res_get_dev(0);
    ret = mac_device_find_up_vap(pst_mac_device, &pst_mac_vap);
    if ((ret != OAL_SUCC) || (pst_mac_vap == NULL)) {
        return;
    }

    /* 当前方式相同,不处理 */
    if (rx_extlna->uc_cur_status == en_is_pm_test) {
        return;
    }

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_DYN_EXTLNA_BYPASS_SWITCH,
        sizeof(uint8_t), (uint8_t *)(&en_is_pm_test));
    if (ret == OAL_SUCC) {
        rx_extlna->uc_cur_status = en_is_pm_test;
    }

    oam_warning_log4(0, OAM_SF_ANY, "{hmac_rx_dyn_bypass_extlna_switch: limit_high:%d,limit_low:%d,throughput:%d,\
        cur_status:%d(0:not pm, 1:pm))!}", limit_throughput_high, limit_throughput_low, throughput_mbps, en_is_pm_test);
}
void hmac_tx_small_amsdu_get_limit_throughput(uint32_t *limit_throughput_high,
    uint32_t *limit_throughput_low)
{
    mac_small_amsdu_switch_stru *small_amsdu_switch = mac_vap_get_small_amsdu_switch();

    if ((small_amsdu_switch->us_small_amsdu_throughput_high != 0) &&
        (small_amsdu_switch->us_small_amsdu_throughput_low != 0)) {
        *limit_throughput_high = small_amsdu_switch->us_small_amsdu_throughput_high;
        *limit_throughput_low = small_amsdu_switch->us_small_amsdu_throughput_low;
    } else {
        *limit_throughput_high = WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_HIGH;
        *limit_throughput_low = WLAN_SMALL_AMSDU_THROUGHPUT_THRESHOLD_LOW;
    }
}

void hmac_tx_small_amsdu_get_limit_pps(uint32_t *limit_pps_high, uint32_t *limit_pps_low)
{
    mac_small_amsdu_switch_stru *small_amsdu_switch = mac_vap_get_small_amsdu_switch();

    if ((small_amsdu_switch->us_small_amsdu_pps_high != 0) &&
        (small_amsdu_switch->us_small_amsdu_pps_low != 0)) {
        *limit_pps_high = small_amsdu_switch->us_small_amsdu_pps_high;
        *limit_pps_low = small_amsdu_switch->us_small_amsdu_pps_low;
    } else {
        *limit_pps_high = WLAN_SMALL_AMSDU_PPS_THRESHOLD_HIGH;
        *limit_pps_low = WLAN_SMALL_AMSDU_PPS_THRESHOLD_LOW;
    }
}

/*
 * 函 数 名  : hmac_config_set_amsdu_tx_on
 * 功能描述  : hmac设置amsdu tx 开关
 * 1.日    期  : 2015年5月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_set_amsdu_tx_on(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_ampdu_tx_on_param_stru *pst_ampdu_tx_on_param = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_amsdu_tx_on:: parma null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_config_set_amsdu_tx_on::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_ampdu_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)puc_param;

    mac_mib_set_CfgAmsduTxAtive(&pst_hmac_vap->st_vap_base_info, pst_ampdu_tx_on_param->uc_aggr_tx_on);

    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_config_set_amsdu_tx_on::ENABLE[%d].}", pst_ampdu_tx_on_param->uc_aggr_tx_on);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_tx_small_amsdu_switch
 * 功能描述  : 1、下行性能大于200M时切换为允许小包amsdu聚合
 *             2、tx pps大于25000时允许小包amsdu聚合
 * 1.日    期  : 2018年05月08日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_tx_small_amsdu_switch(uint32_t rx_throughput_mbps, uint32_t tx_pps)
{
    mac_device_stru *pst_mac_device = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_cfg_ampdu_tx_on_param_stru st_ampdu_tx_on = { 0 };
    uint32_t limit_throughput_high, limit_throughput_low;
    uint32_t limit_pps_high;
    uint32_t limit_pps_low;
    uint32_t ret;
    oal_bool_enum_uint8 en_small_amsdu;
    mac_small_amsdu_switch_stru *small_amsdu_switch = mac_vap_get_small_amsdu_switch();

    /* 如果定制化不支持硬件聚合 */
    if (small_amsdu_switch->uc_ini_small_amsdu_en == OAL_FALSE) {
        return;
    }

    /* 每秒吞吐量门限 */
    hmac_tx_small_amsdu_get_limit_throughput(&limit_throughput_high, &limit_throughput_low);
    /* 每秒PPS门限 */
    hmac_tx_small_amsdu_get_limit_pps(&limit_pps_high, &limit_pps_low);

    if ((rx_throughput_mbps > limit_throughput_high) || (tx_pps > limit_pps_high)) {
        /* rx吞吐量高于300M或者tx pps大于25000,打开小包amsdu聚合 */
        en_small_amsdu = OAL_TRUE;
    } else if ((rx_throughput_mbps < limit_throughput_low) && (tx_pps < limit_pps_low)) {
        /* rx吞吐量低于200M且tx pps小于15000,关闭小包amsdu聚合，避免来回切换 */
        en_small_amsdu = OAL_FALSE;
    } else {
        /* 介于200M-300M之间,不作切换 */
        return;
    }

    /* 当前聚合方式相同,不处理 */
    if (small_amsdu_switch->uc_cur_small_amsdu_en == en_small_amsdu) {
        return;
    }

    pst_mac_device = mac_res_get_dev(0);
    /* 如果非单VAP,则不切换 */
    if (mac_device_calc_up_vap_num(pst_mac_device) != 1) {
        return;
    }

    ret = mac_device_find_up_vap(pst_mac_device, &pst_mac_vap);
    if ((ret != OAL_SUCC) || (pst_mac_vap == NULL)) {
        return;
    }

    oam_warning_log3(0, OAM_SF_ANY, "{hmac_tx_small_amsdu_switch: limit_high = [%d],limit_low = [%d],\
        rx_throught= [%d]!}", limit_throughput_high, limit_throughput_low, rx_throughput_mbps);
    oam_warning_log3(0, OAM_SF_ANY, "{hmac_tx_small_amsdu_switch: PPS limit_high = [%d],PPS limit_low = [%d],\
        tx_pps = %d!}", limit_pps_high, limit_pps_low, tx_pps);

    st_ampdu_tx_on.uc_aggr_tx_on = en_small_amsdu;

    small_amsdu_switch->uc_cur_small_amsdu_en = en_small_amsdu;

    hmac_set_amsdu_tx_on(pst_mac_vap, sizeof(mac_cfg_ampdu_tx_on_param_stru), (uint8_t *)&st_ampdu_tx_on);
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
OAL_STATIC uint32_t hmac_tx_ampdu_hw_switch(mac_vap_stru *pst_mac_vap, uint32_t tx_throughput_mbps,
    uint32_t limit_throughput_high, uint32_t limit_throughput_low, oal_bool_enum_uint8 *pen_ampdu_hw)
{
    if ((tx_throughput_mbps > limit_throughput_high)
#ifdef _PRE_WLAN_FEATURE_11AX
        || (g_wlan_spec_cfg->feature_11ax_is_open &&
        (pst_mac_vap->bit_ofdma_aggr_num == MAC_VAP_AMPDU_HW) &&
        (tx_throughput_mbps > (limit_throughput_high >> 2))) /* 2代表门限除以4 */
#endif
    ) {
        /* 高于350M或者UL OFDMA流程中需要聚合字节数特别长并且有一定的性能,切换硬件聚合 */
        *pen_ampdu_hw = OAL_TRUE;
    } else if (tx_throughput_mbps < limit_throughput_low) {
        /* 低于200M,切换硬件聚合 */
        *pen_ampdu_hw = OAL_FALSE;
    } else {
        /* 介于200M-300M之间,不作切换 */
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t hmac_tx_ampdu_close_hw_switch(mac_vap_stru *pst_mac_vap)
{
    if (oal_value_ne_all2(pst_mac_vap->en_protocol, WLAN_VHT_MODE, WLAN_VHT_ONLY_MODE)) {
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
#ifdef _PRE_WLAN_FEATURE_11AX
            if (pst_mac_vap->en_protocol != WLAN_HE_MODE) {
                return OAL_FAIL;
            }
#endif
        } else {
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_tx_ampdu_hw_cfg_send
 * 功能描述  : 硬件聚合开启下发
 * 1.日    期  : 2017年9月25日
 *   作    者  :
 *   修改内容  : 新生成函数
 */
#define HMAC_TX_AMPDU_AGGR_HW_ENABLE 4
#define HMAC_TX_AMPDU_AGGR_HW_DISABLE 8
OAL_STATIC void hmac_tx_ampdu_hw_cfg_send(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_ampdu_hw)
{
    mac_cfg_ampdu_tx_on_param_stru st_ampdu_tx_on = { 0 };

    if (en_ampdu_hw == OAL_TRUE) {
        st_ampdu_tx_on.uc_aggr_tx_on = HMAC_TX_AMPDU_AGGR_HW_ENABLE;
    } else {
        st_ampdu_tx_on.uc_aggr_tx_on = HMAC_TX_AMPDU_AGGR_HW_DISABLE;
    }

    st_ampdu_tx_on.uc_snd_type = OAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_11AX
    if (pst_mac_vap->bit_ofdma_aggr_num == MAC_VAP_AMPDU_HW) {
        st_ampdu_tx_on.uc_snd_type = OAL_FALSE;
    }
#endif
    st_ampdu_tx_on.en_aggr_switch_mode = AMPDU_SWITCH_BY_BA_LUT;
    hmac_config_set_ampdu_tx_on(pst_mac_vap, sizeof(mac_cfg_ampdu_tx_on_param_stru), (uint8_t *)&st_ampdu_tx_on);
}
static uint32_t hmac_tx_ampdu_hw_ampdu_proc(mac_vap_stru *mac_vap)
{
    hmac_user_stru *hmac_user = NULL;

    g_st_ampdu_hw.us_remain_hw_cnt = 0;
    /* VHT/HE才支持切换 */ /* 嵌套深度优化封装 */
    if (hmac_tx_ampdu_close_hw_switch(mac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 限制:建议工作频率低于160Mhz(80M以下带宽)不支持切换 */
    if (WLAN_BAND_WIDTH_80PLUSPLUS > mac_vap->st_channel.en_bandwidth) {
        return OAL_FAIL;
    }
    /*  规避与linksys ea8500软硬件聚合切换兼容性问题 */
    /* 黑名单内AP不进行硬件聚合切换 */
    if (is_legacy_sta(mac_vap)) {
        hmac_user = mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);
        if (hmac_user == NULL) {
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_tx_ampdu_hw_ampdu_proc: hmac_user is null ptr.");
            return OAL_FAIL;
        }
        if (hmac_user->en_user_ap_type & MAC_AP_TYPE_AGGR_BLACKLIST) {
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}
static void hmac_get_tx_ampdu_limit(uint32_t *limit_throughput_high, uint32_t *limit_throughput_low)
{
    /* 每秒吞吐量门限 */
    if ((g_st_ampdu_hw.us_throughput_high != 0) && (g_st_ampdu_hw.us_throughput_low != 0)) {
        *limit_throughput_high = g_st_ampdu_hw.us_throughput_high;
        *limit_throughput_low = g_st_ampdu_hw.us_throughput_low;
    }
}
/*
 * 函 数 名  : hmac_tx_ampdu_switch
 * 功能描述  : VHT 80M,并且上行性能大于300M时切换为硬件聚合
 * 1.日    期  : 2017年9月25日
 *   修改内容  : 新生成函数
 */
void hmac_tx_ampdu_switch(uint32_t tx_throughput_mbps)
{
    mac_device_stru *pst_mac_device = mac_res_get_dev(0);
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t limit_throughput_high = WLAN_AMPDU_THROUGHPUT_THRESHOLD_HIGH;
    uint32_t limit_throughput_low = WLAN_AMPDU_THROUGHPUT_THRESHOLD_LOW;
    uint32_t ret;
    oal_bool_enum_uint8 en_ampdu_hw = OAL_TRUE;

    /* 如果定制化不支持硬件聚合 */
    if (g_st_ampdu_hw.uc_ampdu_hw_en == OAL_FALSE) {
        return;
    }
    /* 每秒吞吐量门限 */
    hmac_get_tx_ampdu_limit(&limit_throughput_high, &limit_throughput_low);
    /* 如果非单VAP,则不开启硬件聚合 */
    if (mac_device_calc_up_vap_num(pst_mac_device) != 1) {
        en_ampdu_hw = OAL_FALSE;
    } else {
        ret = mac_device_find_up_vap(pst_mac_device, &pst_mac_vap);
        if ((ret != OAL_SUCC) || (pst_mac_vap == NULL)) {
            return;
        }
        /* 嵌套深度优化封装 */
        if (hmac_tx_ampdu_hw_switch(pst_mac_vap, tx_throughput_mbps, limit_throughput_high,
            limit_throughput_low, &en_ampdu_hw) != OAL_SUCC) {
            return;
        }
    }

    /* 当前聚合方式相同,不处理 */
    if (g_st_ampdu_hw.uc_ampdu_hw_enable == en_ampdu_hw) {
        return;
    }

    /* 切换至硬件聚合时,需要判断是否符合切换条件 */
    if (en_ampdu_hw == OAL_TRUE) {
        if (hmac_tx_ampdu_hw_ampdu_proc(pst_mac_vap) != OAL_SUCC) {
            return;
        }
    } else {
        /* 避免来回频繁切换,从硬件聚合切回软件聚合,除了性能降低至切换阈值,
           还需要在硬件聚合下连续保持一定时间 */
        /* 非单vap,直接切换 */
        if (mac_device_calc_up_vap_num(pst_mac_device) == 1) {
            g_st_ampdu_hw.us_remain_hw_cnt++;
            if (g_st_ampdu_hw.us_remain_hw_cnt < WLAN_AMPDU_HW_SWITCH_PERIOD) {
                return;
            }
            g_st_ampdu_hw.us_remain_hw_cnt = 0;
        }
        pst_mac_vap = mac_res_get_mac_vap(0);
        if (pst_mac_vap == NULL) {
            return;
        }
    }

    g_st_ampdu_hw.uc_ampdu_hw_enable = en_ampdu_hw;

    oam_warning_log3(0, OAM_SF_ANY, "{hmac_tx_ampdu_switch: limit_high = [%d],limit_low = [%d],tx_throught= [%d]!}",
                     limit_throughput_high, limit_throughput_low, tx_throughput_mbps);

    hmac_tx_ampdu_hw_cfg_send(pst_mac_vap, en_ampdu_hw);
}
#endif

/*
 * 函 数 名  : hmac_tx_filter_security
 * 功能描述  : 针对来自以太网的数据帧做安全过滤
 * 1.日    期  : 2013年9月12日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tx_filter_security(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_buf, hmac_user_stru *pst_hmac_user)
{
    mac_user_stru *pst_mac_user = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t ret = OAL_SUCC;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    uint16_t ether_type = mac_get_cb_ether_type(tx_ctl);

    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_mac_user = &(pst_hmac_user->st_user_base_info);

    if (OAL_TRUE == mac_mib_get_rsnaactivated(pst_mac_vap)) { /* 判断是否使能WPA/WPA2 */
        if (mac_user_get_port(pst_mac_user) != OAL_TRUE) { /* 判断端口是否打开 */
            /* 发送数据时，针对非EAPOL 的数据帧做过滤 */
#ifdef _PRE_WLAN_FEATURE_PWL
            if ((oal_byteorder_host_to_net_uint16(ETHER_TYPE_PAE) != ether_type) &&
                (oal_byteorder_host_to_net_uint16(ETHER_TYPE_WAI) != ether_type)) {
#else
            if (oal_byteorder_host_to_net_uint16(ETHER_TYPE_PAE) != ether_type) {
#endif
                ether_type = oal_byteorder_host_to_net_uint16(ether_type);
                oam_warning_log3(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                    "{hmac_tx_filter_security::TYPE 0x%04x, 0x%04x, 0x%04x.}",
                    ether_type, ETHER_TYPE_PAE, ETHER_TYPE_WAI);
                ret = OAL_FAIL;
            }
        }
    }

    return ret;
}

/*
 * 函 数 名  : hmac_tx_ba_setup
 * 功能描述  : 自动触发BA会话的建立
 * 1.日    期  : 2013年4月25日
 *   作    者  :
 *   修改内容  : 新生成函数
 */
void hmac_tx_ba_setup(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_user, uint8_t uc_tidno)
{
    mac_action_mgmt_args_stru st_action_args; /* 用于填写ACTION帧的参数 */

    /*
    建立BA会话时，st_action_args结构各个成员意义如下
    (1)uc_category:action的类别
    (2)uc_action:BA action下的类别
    (3)arg1:BA会话对应的TID
    (4)arg2:BUFFER SIZE大小
    (5)arg3:BA会话的确认策略
    (6)arg4:TIMEOUT时间
    */
    st_action_args.uc_category = MAC_ACTION_CATEGORY_BA;
    st_action_args.uc_action = MAC_BA_ACTION_ADDBA_REQ;
    st_action_args.arg1 = uc_tidno; /* 该数据帧对应的TID号 */
    /* ADDBA_REQ中，buffer_size的默认大小 */
    st_action_args.arg2 = (uint32_t)g_wlan_cust.ampdu_tx_max_num;

    st_action_args.arg3 = MAC_BA_POLICY_IMMEDIATE; /* BA会话的确认策略 */
    st_action_args.arg4 = 0;                       /* BA会话的超时时间设置为0 */

    /* 建立BA会话 */
    hmac_mgmt_tx_action(pst_hmac_vap, pst_user, &st_action_args);
}

/*
 * 函 数 名  : hmac_tx_ucast_process
 * 功能描述  : 单播处理
 * 1.日    期  : 2012年11月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tx_ucast_process(hmac_vap_stru *pst_hmac_vap,
    oal_netbuf_stru *pst_buf, hmac_user_stru *pst_user, mac_tx_ctl_stru *pst_tx_ctl)
{
    uint32_t ret;
    /* 安全过滤 */
    if (oal_unlikely(OAL_SUCC != hmac_tx_filter_security(pst_hmac_vap, pst_buf, pst_user))) {
        oam_stat_vap_incr(pst_hmac_vap->st_vap_base_info.uc_vap_id, tx_security_check_faild, 1);
        return HMAC_TX_DROP_SECURITY_FILTER;
    }

    if (hmac_ring_tx_enabled() == OAL_TRUE) {
        mac_get_cb_wme_tid_type(pst_tx_ctl) = WLAN_TIDNO_BCAST;
        mac_get_cb_wme_ac_type(pst_tx_ctl) = WLAN_WME_TID_TO_AC(WLAN_TIDNO_BCAST);
    }

    /* 如果是EAPOL、DHCP帧，则不允许主动建立BA会话 */
    if (mac_get_cb_is_vipframe(pst_tx_ctl)) {
        return HMAC_TX_PASS;
    }

    if (hmac_tid_need_ba_session(pst_hmac_vap, pst_user, mac_get_cb_wme_tid_type(pst_tx_ctl), pst_buf) == OAL_TRUE) {
        /* 自动触发建立BA会话，设置AMPDU聚合参数信息在DMAC模块的处理addba rsp帧的时刻后面 */
        hmac_tx_ba_setup(pst_hmac_vap, pst_user, mac_get_cb_wme_tid_type(pst_tx_ctl));
    }

    ret = hmac_amsdu_notify(pst_hmac_vap, pst_user, pst_buf);
    if (oal_unlikely(ret != HMAC_TX_PASS)) {
        return ret;
    }

#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_chba_sync_user_bitmap_level_by_data_type(pst_hmac_vap, pst_user, pst_tx_ctl);
#endif

    return HMAC_TX_PASS;
}

/*
 * 函 数 名  : hmac_tx_need_frag
 * 功能描述  : 检查该报文是否需要分片
 * 1.日    期  : 2014年2月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tx_need_frag(
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl)
{
    uint32_t threshold;
    oal_bool_enum_uint8 need_frag = OAL_TRUE;
    /* 判断报文是否需要进行分片 */
    /* 1、长度大于门限          */
    /* 2、是legac协议模式       */
    /* 3、不是广播帧            */
    /* 4、不是聚合帧            */
    /* 6、DHCP帧不进行分片      */
    threshold = mac_mib_get_FragmentationThreshold(&hmac_vap->st_vap_base_info);
#if (defined(_PRE_WLAN_FEATURE_ERSRU) && defined(_PRE_WLAN_FEATURE_HIEX))
    if (hmac_user->st_ersru.bit_enable) {
        threshold = oal_min(threshold, hmac_user->st_ersru.bit_frag_len);
    }
#endif
    threshold = (threshold & (~(BIT0 | BIT1))) + BYTE_OFFSET_2;

    /* 当前帧不大于分片门限,不分片 */
    if (threshold >= (oal_netbuf_len(netbuf) + mac_get_cb_frame_header_length(tx_ctl))) {
        return 0;
    }

    /* 非Legacy协议/VIP帧/广播帧/不分片,私有对通分片 */
    if ((hmac_user->st_user_base_info.en_cur_protocol_mode >= WLAN_HT_MODE) ||
        (hmac_vap->st_vap_base_info.en_protocol >= WLAN_HT_MODE) ||
        (mac_get_cb_is_vipframe(tx_ctl) == OAL_TRUE) ||
        (mac_get_cb_is_mcast(tx_ctl) == OAL_TRUE)) {
        need_frag = OAL_FALSE;
    }
#if (defined(_PRE_WLAN_FEATURE_ERSRU) && defined(_PRE_WLAN_FEATURE_HIEX))
    if (hmac_user->st_ersru.bit_enable) {
        need_frag = OAL_TRUE;
    }
#endif
    /* 聚合不分片 */
    if ((need_frag == OAL_TRUE) &&
        (mac_get_cb_is_amsdu(tx_ctl) == OAL_FALSE)
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        && (mac_get_cb_amsdu_level(tx_ctl) == WLAN_TX_AMSDU_NONE)
#endif
        && (OAL_FALSE == hmac_tid_ba_is_setup(hmac_user, mac_get_cb_wme_tid_type(tx_ctl)))) {
        return threshold;
    }

    return 0;
}

/*
 * 功能描述  : 封装MAC头
 * 1.日    期  : 2020.5.14
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static uint32_t hmac_tx_encap_mac_hdr(hmac_vap_stru *hmac_vap,
    mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf, uint8_t **mac_hdr)
{
    uint32_t ret;
    uint32_t headroom_size = oal_netbuf_headroom(netbuf);
    /* 如果是Ring发送802.11帧, 入分片帧、WAPI帧，MAC HEADER必须存放在netbuf中,必须保证有连续的帧头 */
    if ((hmac_ring_tx_enabled() == OAL_TRUE) && headroom_size < MAC_80211_QOS_HTC_4ADDR_FRAME_LEN) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{hmac_tx_encap_mac_hdr::headroom[%d] too short}", headroom_size);
        ret = (uint32_t)oal_netbuf_expand_head(netbuf, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN, 0, GFP_ATOMIC);
        if (ret != OAL_SUCC) {
            oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                "{hmac_tx_encap_mac_hdr::expand headroom failed}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        headroom_size = oal_netbuf_headroom(netbuf);
    }

    /* device发送流程data指针前预留的空间大于802.11 mac head len，则不需要格外申请内存存放802.11头 */
    if (headroom_size >= MAC_80211_QOS_HTC_4ADDR_FRAME_LEN) {
        *mac_hdr = (oal_netbuf_header(netbuf) - MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
        mac_get_cb_80211_mac_head_type(tx_ctl) = 1; /* 指示mac头部在skb中 */
    } else {
        /* 申请最大的80211头 */
        *mac_hdr = oal_mem_alloc_m(OAL_MEM_POOL_ID_SHARED_DATA_PKT, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN, OAL_FALSE);
        if (oal_unlikely(*mac_hdr == NULL)) {
            oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{hmac_tx_encap_mac_hdr::pst_hdr null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        mac_get_cb_80211_mac_head_type(tx_ctl) = 0; /* 指示mac头部不在skb中，申请了额外内存存放的 */
    }
    /* 挂接802.11头 */
    mac_get_cb_frame_header_addr(tx_ctl) = (mac_ieee80211_frame_stru *)(*mac_hdr);
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_PWL
/* 非PWL模式htc的去除是在device侧做的，PWL模式下改为在host侧 */
static void hmac_remove_vip_frame_htc_info(mac_ieee80211_qos_htc_frame_addr4_stru *mac_hdr, mac_tx_ctl_stru *tx_ctl)
{
    if (!mac_get_cb_is_vipframe(tx_ctl) || !mac_get_cb_htc_flag(tx_ctl)) {
        return;
    }
    mac_get_cb_htc_flag(tx_ctl) = OAL_FALSE;
    mac_hdr->st_frame_control.bit_order = 0;
    mac_get_cb_frame_header_length(tx_ctl) -= 4; /* 减去4字节htc */
}
#endif
void hmac_tx_set_frame_htc(hmac_vap_stru *pst_hmac_vap,
    mac_tx_ctl_stru *pst_tx_ctl, hmac_user_stru *pst_hmac_user, mac_ieee80211_qos_htc_frame_addr4_stru *pst_hdr_addr4)
{
    mac_ieee80211_qos_htc_frame_stru *pst_hdr = NULL;
    uint32_t                        htc_val = 0;

    if (mac_get_cb_wme_tid_type(pst_tx_ctl) == WLAN_TIDNO_BCAST) {
        return;
    }
    if (pst_hmac_vap->uc_htc_order_flag == HAL_HE_HTC_CONFIG_NO_HTC) {
        return;
    }

    /* 满足条件时在host标记himit帧并将htc字段配置为全F,在dmac判断OM当前是否使用HTC字段,OM特性不用时,himit特性配置HTC */
#ifdef _PRE_WLAN_FEATURE_HIEX
    if ((g_wlan_spec_cfg->feature_hiex_is_open) && (hmac_tx_enable_himit_htc(pst_hmac_user, pst_tx_ctl) == OAL_TRUE)) {
        htc_val = HTC_INVALID_VALUE;
        mac_get_cb_is_himit_frame(pst_tx_ctl) = OAL_TRUE;
    } else if (mac_user_tx_data_include_htc(&pst_hmac_user->st_user_base_info) ||
               pst_hmac_vap->st_vap_base_info.uc_himit_set_htc != 0) {
        htc_val = HTC_INVALID_VALUE;
    } else {
        return;
    }
#endif

    mac_get_cb_htc_flag(pst_tx_ctl) = 1;  // 标记为带有HTC字段
    /* 更新帧头长度 */
    if (OAL_FALSE == mac_get_cb_is_4address(pst_tx_ctl)) {
        pst_hdr = (mac_ieee80211_qos_htc_frame_stru *)pst_hdr_addr4;
        pst_hdr->st_frame_control.bit_order = 1;
        mac_get_cb_frame_header_length(pst_tx_ctl) = MAC_80211_QOS_HTC_FRAME_LEN;
        pst_hdr->htc = htc_val;
    } else {
        /* 设置QOS控制字段 */
        pst_hdr_addr4->st_frame_control.bit_order = 1;
        mac_get_cb_frame_header_length(pst_tx_ctl) = MAC_80211_QOS_HTC_4ADDR_FRAME_LEN;
        pst_hdr_addr4->htc = htc_val;
    }
#ifdef _PRE_WLAN_FEATURE_PWL
    if (pst_hmac_vap->st_vap_base_info.pwl_enable == OAL_TRUE) {
        hmac_remove_vip_frame_htc_info(pst_hdr_addr4, pst_tx_ctl);
    }
#endif
}
void hmac_tx_set_qos_frame_htc(hmac_vap_stru *hmac_vap,
    mac_tx_ctl_stru *pst_tx_ctl, hmac_user_stru *pst_hmac_user, mac_ieee80211_qos_htc_frame_addr4_stru *pst_hdr_addr4)
{
    /* tid7 no ring nor not support */
    if (mac_get_cb_wme_tid_type(pst_tx_ctl) == WLAN_TIDNO_BCAST) {
        return;
    }
    if (mac_user_not_support_htc(&pst_hmac_user->st_user_base_info)) {
        return;
    }
    /* 满足条件时在host标记himit帧并将htc字段配置为全F,在dmac判断OM当前是否使用HTC字段,OM特性不用时,himit特性配置HTC */
#ifdef _PRE_WLAN_FEATURE_HIEX
    if ((g_wlan_spec_cfg->feature_hiex_is_open) && (hmac_tx_enable_himit_htc(pst_hmac_user, pst_tx_ctl) == OAL_TRUE)) {
        mac_get_cb_is_himit_frame(pst_tx_ctl) = OAL_TRUE;
    }
#endif

    /* 更新帧头长度 */
    if (OAL_FALSE == mac_get_cb_is_4address(pst_tx_ctl)) {
        mac_get_cb_frame_header_length(pst_tx_ctl) = MAC_80211_QOS_FRAME_LEN;
    } else {
        /* 设置QOS控制字段 */
        mac_get_cb_frame_header_length(pst_tx_ctl) = MAC_80211_QOS_4ADDR_FRAME_LEN;
    }
}

void hmac_tx_encap_do_non_amsdu(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user, oal_netbuf_stru *pst_buf,
    mac_tx_ctl_stru *pst_tx_ctl, uint16_t us_ether_type)
{
    /* len = EHTER HEAD LEN + PAYLOAD LEN */
    uint16_t us_mpdu_len = (uint16_t)oal_netbuf_get_len(pst_buf);

    /* 更新frame长度，指向skb payload--LLC HEAD */
    mac_get_cb_mpdu_len(pst_tx_ctl) = (us_mpdu_len - ETHER_HDR_LEN + SNAP_LLC_FRAME_LEN);

    mac_get_cb_ether_head_padding(pst_tx_ctl) = 0;

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    wlan_chip_tx_encap_large_skb_amsdu(pst_vap, pst_user, pst_buf, pst_tx_ctl);

    if (mac_get_cb_amsdu_level(pst_tx_ctl)) {
        /* 恢复data指针到ETHER HEAD - LLC HEAD */
        oal_netbuf_pull(pst_buf, SNAP_LLC_FRAME_LEN);
    }
#endif
    /* 设置netbuf->data指向LLC HEAD */
    mac_set_snap(pst_buf, us_ether_type);

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    if (mac_get_cb_amsdu_level(pst_tx_ctl)) {
        /* 恢复data指针到ETHER HEAD */
        oal_netbuf_push(pst_buf, ETHER_HDR_LEN);

        /* 保证4bytes对齐 */
        if ((unsigned long)(uintptr_t)oal_netbuf_data(pst_buf) !=
            OAL_ROUND_DOWN((unsigned long)(uintptr_t)oal_netbuf_data(pst_buf), 4)) { // 4代表4字节对齐
            mac_get_cb_ether_head_padding(pst_tx_ctl) = (unsigned long)(uintptr_t)oal_netbuf_data(pst_buf) -
                OAL_ROUND_DOWN((unsigned long)(uintptr_t)oal_netbuf_data(pst_buf), 4); // 4代表4字节对齐
            oal_netbuf_push(pst_buf, mac_get_cb_ether_head_padding(pst_tx_ctl));
        }
    }
#endif
}
/*
 * 功能描述  : 802.11帧头封装 AP模式
 */
uint32_t hmac_tx_encap_ieee80211_header(hmac_vap_stru *pst_vap, hmac_user_stru *pst_user, oal_netbuf_stru *pst_buf)
{
    uint8_t *puc_80211_hdr = NULL; /* 802.11头 */
    uint32_t qos = HMAC_TX_BSS_NOQOS;
    mac_tx_ctl_stru *pst_tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(pst_buf));
    uint32_t ret, threshold;
    uint8_t uc_buf_is_amsdu = mac_get_cb_is_amsdu(pst_tx_ctl);
    uint8_t uc_ic_header = 0;
    mac_ether_header_stru st_ether_hdr;

    memset_s(&st_ether_hdr, sizeof(mac_ether_header_stru), 0, sizeof(mac_ether_header_stru));

    /* 获取以太网头, 原地址，目的地址, 以太网类型 */
    if (EOK != memcpy_s(&st_ether_hdr, sizeof(st_ether_hdr), oal_netbuf_data(pst_buf), ETHER_HDR_LEN)) {
        oam_error_log0(0, OAM_SF_TX, "hmac_tx_encap_ieee80211_header::memcpy fail!");
    }

    /* 非amsdu帧 */
    if (uc_buf_is_amsdu == OAL_FALSE) {
        hmac_tx_encap_do_non_amsdu(pst_vap, pst_user, pst_buf, pst_tx_ctl, st_ether_hdr.us_ether_type);
    }

    ret = hmac_tx_encap_mac_hdr(pst_vap, pst_tx_ctl, pst_buf, &puc_80211_hdr);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    /* 组播聚合帧、或非组播帧，获取用户的QOS能力位信息 */
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    if ((mac_get_mcast_ampdu_switch() == OAL_TRUE) || (mac_get_cb_is_mcast(pst_tx_ctl) == OAL_FALSE)) {
#else
    if (mac_get_cb_is_mcast(pst_tx_ctl) == OAL_FALSE) {
#endif
        /* 根据用户结构体的cap_info，判断是否是QOS站点 */
        qos = pst_user->st_user_base_info.st_cap_info.bit_qos;
    }

    /* 设置帧控制 */
    hmac_tx_set_frame_ctrl(qos, pst_tx_ctl, (mac_ieee80211_qos_htc_frame_addr4_stru *)puc_80211_hdr);
    /* 设置帧HTC */
    if ((qos == HMAC_TX_BSS_QOS) && (g_wlan_spec_cfg->feature_11ax_is_open)) {
        wlan_chip_tx_set_frame_htc(pst_vap, pst_tx_ctl, pst_user,
            (mac_ieee80211_qos_htc_frame_addr4_stru *)puc_80211_hdr);
    }
    /* 设置地址 */
    hmac_tx_set_addresses(pst_vap, pst_user, pst_tx_ctl, &st_ether_hdr,
                          (mac_ieee80211_qos_htc_frame_addr4_stru *)puc_80211_hdr);

    /* 分片处理 */
    threshold = hmac_tx_need_frag(pst_vap, pst_user, pst_buf, pst_tx_ctl);
    if (threshold != 0) {
        /* 调用加密接口在使用TKIP时对MSDU进行加密后在进行分片 */
        ret = hmac_en_mic(pst_vap, pst_user, pst_buf, &uc_ic_header);
        if (ret != OAL_SUCC) {
            oam_error_log1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{hmac_en_mic failed[%d].}", ret);
            return ret;
        }

        /* 进行分片处理 */
        ret = hmac_frag_process(pst_vap, pst_buf, pst_tx_ctl, (uint32_t)uc_ic_header, threshold);
    }

#if ((defined (PRE_WLAN_FEATURE_SNIFFER)) && (defined (CONFIG_HW_SNIFFER)))
    proc_sniffer_write_file((const uint8_t *)puc_80211_hdr, MAC_80211_QOS_FRAME_LEN,
                            (const uint8_t *)oal_netbuf_data(pst_buf), oal_netbuf_len(pst_buf), 1);
#endif
    return ret;
}

/*
 * 函 数 名  : hmac_tx_mpdu_process
 * 功能描述  : 单个MPDU处理函数
 * 1.日    期  : 2012年11月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_tx_lan_mpdu_process_sta(hmac_vap_stru *pst_vap,
    oal_netbuf_stru *pst_buf, mac_tx_ctl_stru *pst_tx_ctl)
{
    hmac_user_stru *pst_user = NULL; /* 目标STA结构体 */
    mac_ether_header_stru *pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_buf);    /* 以太网头 */
    uint32_t ret;
    uint8_t *puc_ether_payload = NULL;

    mac_get_cb_tx_vap_index(pst_tx_ctl) = pst_vap->st_vap_base_info.uc_vap_id;

    pst_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_get_cb_tx_user_idx(pst_tx_ctl));
    if (pst_user == NULL) {
        oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_USER_NULL;
    }

    if (oal_byteorder_host_to_net_uint16(ETHER_TYPE_ARP) == mac_get_cb_ether_type(pst_tx_ctl)) {
        pst_ether_hdr++;
        puc_ether_payload = (uint8_t *)pst_ether_hdr;
        /* The source MAC address is modified only if the packet is an   */
        /* ARP Request or a Response. The appropriate bytes are checked. */
        /* Type field (2 bytes): ARP Request (1) or an ARP Response (2)  */
        if ((puc_ether_payload[BYTE_OFFSET_6] == 0x00) &&
            (puc_ether_payload[BYTE_OFFSET_7] == 0x02 || puc_ether_payload[BYTE_OFFSET_7] == 0x01)) {
            /* Set Address2 field in the WLAN Header with source address */
            oal_set_mac_addr(puc_ether_payload + BYTE_OFFSET_8, mac_mib_get_StationID(&pst_vap->st_vap_base_info));
        }
    }

    ret = hmac_tx_ucast_process(pst_vap, pst_buf, pst_user, pst_tx_ctl);
    if (oal_unlikely(ret != HMAC_TX_PASS)) {
        return ret;
    }

    /* 封装802.11头 */
    ret = hmac_tx_encap_ieee80211_header(pst_vap, pst_user, pst_buf);
    if (oal_unlikely((ret != OAL_SUCC))) {
        oam_warning_log1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                         "{hmac_tx_lan_mpdu_process_sta::failed[%d].}", ret);
        oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_80211_ENCAP_FAIL;
    }

    return HMAC_TX_PASS;
}

#ifdef _PRE_WINDOWS_SUPPORT
/*
 * 函 数 名  : hmac_tx_encap_fips
 * 功能描述  : 获取wdi 80211头信息后构造发送80211mac头
 * 1.日    期  : 2021年01月28日
 *   修改内容  : 新生成函数
 */
static uint32_t hmac_tx_encap_fips(hmac_vap_stru *hmac_vap, hmac_user_stru *user, oal_netbuf_stru *netbuf)
{
    uint8_t *mac80211_hdr = NULL;
    mac_tx_ctl_stru *tx_ctl = NULL;
    mac_ieee80211_frame_stru *head;
    uint8_t mac80211_hdr_old[MAC_80211_QOS_HTC_4ADDR_FRAME_LEN] = { 0 };
    uint32_t qos = HMAC_TX_BSS_NOQOS;
    uint32_t ret;

    /* FIPS模式获取802.11头 */
    if (memcpy_s(&mac80211_hdr_old, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN,
                 oal_netbuf_data(netbuf), netbuf->head_len) != EOK) {
        oam_error_log0(0, OAM_SF_TX, "hmac_tx_encap_fips:: MAC_80211 memcpy fail!");
    }
    tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(netbuf));

    /* 恢复data指针到80211 mac头后，或是llc header，或是加密CCMP key */
    oal_netbuf_pull(netbuf, netbuf->head_len);
    mac_get_cb_mpdu_len(tx_ctl) = (uint16_t)oal_netbuf_get_len(netbuf) - netbuf->head_len;

    ret = hmac_tx_encap_mac_hdr(hmac_vap, tx_ctl, netbuf, &mac80211_hdr);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    /* 非组播帧，获取用户的QOS能力位信息 */
    if (mac_get_cb_is_mcast(tx_ctl) == FALSE) {
        /* 根据用户结构体的cap_info，判断是否是QOS站点 */
        qos = user->st_user_base_info.st_cap_info.bit_qos;
    }
    /* 设置帧控制 */
    hmac_tx_set_frame_ctrl(qos, tx_ctl, (mac_ieee80211_qos_htc_frame_addr4_stru *)mac80211_hdr);
    hmac_tx_switch_addresses_fips(tx_ctl,
        (mac_ieee80211_qos_htc_frame_addr4_stru *)mac80211_hdr_old,
        (mac_ieee80211_qos_htc_frame_addr4_stru *)mac80211_hdr);
    ((mac_ieee80211_qos_htc_frame_addr4_stru *)mac80211_hdr)->st_frame_control.bit_protected_frame =
    ((mac_ieee80211_qos_htc_frame_addr4_stru *)mac80211_hdr_old)->st_frame_control.bit_protected_frame;

#if ((defined (PRE_WLAN_FEATURE_SNIFFER)) && (defined (CONFIG_HW_SNIFFER)))
    proc_sniffer_write_file((const uint8_t *)head, MAC_80211_QOS_FRAME_LEN,
                            (const uint8_t *)oal_netbuf_data(netbuf), oal_netbuf_len(netbuf), 1);
#endif
    return ret;
}

/*
 * 函 数 名  : hmac_tx_lan_mpdu_process_fips
 * 功能描述  : hmac 发送一个mpdu的关键帧识别维测
 * 1.日    期  : 2018年9月19日
 *   修改内容  : 新生成函数
 */
static uint32_t hmac_tx_lan_mpdu_process_fips(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    mac_tx_ctl_stru *tx_ctl)
{
    hmac_user_stru *user = NULL;
    uint16_t user_idx = hmac_vap->st_vap_base_info.us_assoc_vap_id;
    uint32_t ret;
    uint8_t tid;

    user = (hmac_user_stru *)mac_res_get_hmac_user(user_idx);
    if (user == NULL) {
        oam_stat_vap_incr(hmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_USER_NULL;
    }

    mac_get_cb_tx_vap_index(tx_ctl) = hmac_vap->st_vap_base_info.uc_vap_id;
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_BUTT;
    mac_get_cb_tx_user_idx(tx_ctl) = user_idx;
    tid = WLAN_TIDNO_BEST_EFFORT;
    mac_get_cb_wme_tid_type(tx_ctl) = tid;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_TID_TO_AC(tid);

    /* 封装802.11头 */
    ret = hmac_tx_encap_fips(hmac_vap, user, netbuf);
    if (oal_unlikely((ret != OAL_SUCC))) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                         "{hmac_tx_lan_mpdu_process_fips::failed[%d].}", ret);
        oam_stat_vap_incr(hmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_80211_ENCAP_FAIL;
    }

    return HMAC_TX_PASS;
}
#endif
/*
 * 函 数 名  : hmac_tx_data_mcast_process_ap
 * 功能描述  : 组播帧处理函数
 * 1.日    期  : 2021年4月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static uint32_t hmac_tx_data_mcast_process_ap(hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *tx_ctl,
    hmac_user_stru **hmac_user)
{
    /* 更新ACK策略 */
    mac_get_cb_ack_policy(tx_ctl) = WLAN_TX_NO_ACK;

    /* 获取组播用户 */
    *hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_multi_user_idx);
    if (oal_unlikely(*hmac_user == NULL)) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
            "{hmac_tx_lan_mpdu_process_ap::get multi user failed[%d].}",
            hmac_vap->st_vap_base_info.us_multi_user_idx);
        oam_stat_vap_incr(hmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_MUSER_NULL;
    }

    mac_get_cb_wme_tid_type(tx_ctl) = WLAN_TIDNO_BCAST;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_TID_TO_AC(WLAN_TIDNO_BCAST);
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    /* 如果组播聚合打开，组播帧走tid2 */
    if (mac_get_mcast_ampdu_switch() == OAL_TRUE) {
        mac_get_cb_wme_tid_type(tx_ctl) = WLAN_TIDNO_MCAST_AMPDU;
        mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_TID_TO_AC(WLAN_TIDNO_MCAST_AMPDU);
    }
#endif
    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_tx_mpdu_process
 * 功能描述  : 单个MPDU处理函数
 * 1.日    期  : 2012年11月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static uint32_t hmac_tx_lan_mpdu_process_ap(hmac_vap_stru *pst_vap,
    oal_netbuf_stru *pst_buf, mac_tx_ctl_stru *pst_tx_ctl)
{
    hmac_user_stru *pst_user = NULL; /* 目标STA结构体 */
    mac_ether_header_stru *pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_buf); /* 以太网头 */
    uint32_t ret;
    uint16_t us_user_idx = mac_get_cb_tx_user_idx(pst_tx_ctl);

    /* 单播数据帧 */
    if (oal_likely(!mac_get_cb_is_mcast(pst_tx_ctl))) {
        if (oal_unlikely(us_user_idx == g_wlan_spec_cfg->invalid_user_id)) {
            oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
            return HMAC_TX_DROP_USER_UNKNOWN;
        }

        /* 转成HMAC的USER结构体 */
        pst_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_idx);
        if (oal_unlikely(pst_user == NULL)) {
            oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
            return HMAC_TX_DROP_USER_NULL;
        }

        /* 用户状态判断 */
        if (oal_unlikely(pst_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC)) {
            oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
            return HMAC_TX_DROP_USER_INACTIVE;
        }

        ret = hmac_tx_ucast_process(pst_vap, pst_buf, pst_user, pst_tx_ctl);
        if (oal_unlikely(ret != HMAC_TX_PASS)) {
            return ret;
        }
    } else { /* 组播 or 广播 */
        ret = hmac_tx_data_mcast_process_ap(pst_vap, pst_tx_ctl, &pst_user);
        if (ret != OAL_SUCC) {
            return ret;
        }
        wlan_chip_tx_pt_mcast_set_cb(pst_vap, pst_ether_hdr, pst_tx_ctl);
    }

    /* 封装802.11头 */
    ret = hmac_tx_encap_ieee80211_header(pst_vap, pst_user, pst_buf);
    if (oal_unlikely((ret != OAL_SUCC))) {
        oam_warning_log1(pst_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                         "{hmac_tx_lan_mpdu_process_ap::failed[%d].}", ret);
        oam_stat_vap_incr(pst_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return HMAC_TX_DROP_80211_ENCAP_FAIL;
    }

    return HMAC_TX_PASS;
}

/*
 * 函 数 名  : hmac_tx_lan_mpdu_info
 * 功能描述  : hmac 发送一个mpdu的关键帧识别维测
 * 1.日    期  : 2018年9月19日
 *   作    者  :
 *   修改内容  : 新生成函数
 */
void hmac_tx_vip_info(mac_vap_stru *pst_vap, uint8_t uc_data_type,
    oal_netbuf_stru *pst_buf, mac_tx_ctl_stru *pst_tx_ctl)
{
    mac_eapol_type_enum_uint8 en_eapol_type;
    uint8_t uc_dhcp_type;
    mac_ieee80211_qos_htc_frame_addr4_stru *pst_mac_header = NULL;
    mac_llc_snap_stru *pst_llc = NULL;
    oal_ip_header_stru *pst_rx_ip_hdr = NULL;
    oal_eth_arphdr_stru *puc_arp_head = NULL;
    int32_t l_ret = EOK;

    uint8_t auc_ar_sip[ETH_SENDER_IP_ADDR_LEN]; /* sender IP address */
    uint8_t auc_ar_dip[ETH_SENDER_IP_ADDR_LEN]; /* sender IP address */

    /* 输入skb已经封装80211头 */
    /* 获取LLC SNAP */
    pst_llc = (mac_llc_snap_stru *)oal_netbuf_data(pst_buf);
    pst_mac_header = (mac_ieee80211_qos_htc_frame_addr4_stru *)mac_get_cb_frame_header_addr(pst_tx_ctl);
    if (pst_mac_header == NULL) {
        return;
    }
    if (uc_data_type == MAC_DATA_EAPOL) {
        en_eapol_type = mac_get_eapol_key_type((uint8_t *)(pst_llc + 1));
        oam_warning_log2(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_vip_info::EAPOL type=%u, len==%u} \
            [1:1/4 2:2/4 3:3/4 4:4/4]", en_eapol_type, oal_netbuf_len(pst_buf));
    } else if (uc_data_type == MAC_DATA_DHCP) {
        pst_rx_ip_hdr = (oal_ip_header_stru *)(pst_llc + 1);

        l_ret += memcpy_s((uint8_t *)auc_ar_sip, ETH_SENDER_IP_ADDR_LEN,
                          (uint8_t *)&pst_rx_ip_hdr->saddr, sizeof(uint32_t));
        l_ret += memcpy_s((uint8_t *)auc_ar_dip, ETH_SENDER_IP_ADDR_LEN,
                          (uint8_t *)&pst_rx_ip_hdr->daddr, sizeof(uint32_t));
        if (l_ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hmac_tx_vip_info::memcpy fail!");
            return;
        }

        uc_dhcp_type = mac_get_dhcp_frame_type(pst_rx_ip_hdr, mac_get_cb_mpdu_len(pst_tx_ctl) - SNAP_LLC_FRAME_LEN);
        hmac_ht_self_cure_event_set(pst_vap, pst_mac_header->auc_address1, HMAC_HT_SELF_CURE_EVENT_TX_DHCP_FRAME);
        oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_vip_info::DHCP type=%d.[1:discovery 2:offer \
            3:request 4:decline 5:ack 6:nack 7:release 8:inform]", uc_dhcp_type);
        oam_warning_log4(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_vip_info:: DHCP sip: %d.%d, dip: %d.%d.",
            auc_ar_sip[BYTE_OFFSET_2], auc_ar_sip[BYTE_OFFSET_3], auc_ar_dip[BYTE_OFFSET_2], auc_ar_dip[BYTE_OFFSET_3]);
    } else {
        puc_arp_head = (oal_eth_arphdr_stru *)(pst_llc + 1);
        oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_vip_info:: ARP type=%d.[2:arp resp 3:arp req.]",
                         uc_data_type);
        oam_warning_log4(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_lan_mpdu_info:: ARP sip: %d.%d, dip: %d.%d",
                         puc_arp_head->auc_ar_sip[BYTE_OFFSET_2], puc_arp_head->auc_ar_sip[BYTE_OFFSET_3],
                         puc_arp_head->auc_ar_tip[BYTE_OFFSET_2], puc_arp_head->auc_ar_tip[BYTE_OFFSET_3]);
    }

    oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_vip_info::smac: %02X:XX:XX:XX:%02X:%02X",
        pst_mac_header->auc_address2[MAC_ADDR_0],
        pst_mac_header->auc_address2[MAC_ADDR_4], pst_mac_header->auc_address2[MAC_ADDR_5]);
    oam_warning_log3(pst_vap->uc_vap_id, OAM_SF_ANY, "{hmac_tx_vip_info::dmac:%02X:XX:XX:XX:%02X:%02X",
        pst_mac_header->auc_address1[MAC_ADDR_0],
        pst_mac_header->auc_address1[MAC_ADDR_4], pst_mac_header->auc_address1[MAC_ADDR_5]);
}
void hmac_tx_rtsp_info(hmac_vap_stru *hmac_vap, uint8_t data_type, oal_netbuf_stru *pst_buf)
{
    if (pst_buf == NULL || hmac_vap == NULL) {
        return;
    }
    if (data_type == MAC_DATA_RTSP) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{hmac_tx_rtsp_info::rtsp len==%d}",
            oal_netbuf_len(pst_buf));
    }
}

static void hmac_set_cb_is_ipv4_tcp(mac_ether_header_stru *ether_hdr, mac_tx_ctl_stru *tx_ctl)
{
    oal_ip_header_stru *ip_hdr = (oal_ip_header_stru *)(ether_hdr + 1);
    oal_tcp_header_stru *tcp_hdr = (oal_tcp_header_stru *)(ip_hdr + 1);

    if (ip_hdr->uc_protocol != MAC_TCP_PROTOCAL) {
        return;
    }

    if (oal_netbuf_is_tcp_ack(ip_hdr)) {
        mac_get_cb_is_tcp_ack(tx_ctl) = OAL_TRUE;
    } else {
        mac_get_cb_is_tcp_data(tx_ctl) = OAL_TRUE;
    }

    if ((tcp_hdr->uc_flags & FILTER_FLAG_MASK) ||
        (mac_get_cb_is_tcp_ack(tx_ctl) && hmac_device_tcp_buf_duplicate_ack_proc(ip_hdr))) {
        mac_get_cb_urgent_tcp_ack(tx_ctl) = OAL_TRUE;
    }
}

static void hmac_set_cb_is_ipv6_tcp(mac_ether_header_stru *ether_hdr, mac_tx_ctl_stru *tx_ctl)
{
    oal_ipv6hdr_stru *ipv6_hdr = (oal_ipv6hdr_stru *)(ether_hdr + 1);

    if (ipv6_hdr->nexthdr != TCP_PROTOCAL) {
        return;
    }

    if (oal_netbuf_is_tcp_ack6(ipv6_hdr)) {
        mac_get_cb_ipv6_tcp_ack(tx_ctl) = OAL_TRUE;
    } else {
        mac_get_cb_is_tcp_data(tx_ctl) = OAL_TRUE;
    }
}

/*
 * 功能描述  :  初始化netbuf中CB字段中的is_tcp_ack和urgent_tcp_ack
 * 1.日    期  : 2022年8月12日
 *   作    者  :
 *   修改内容  : 新生成函数
 */
void hmac_set_cb_is_tcp(mac_tx_ctl_stru *tx_ctl, mac_ether_header_stru *ether_hdr, uint16_t ether_type)
{
    switch (ether_type) {
        case oal_host2net_short(ETHER_TYPE_IP):
            hmac_set_cb_is_ipv4_tcp(ether_hdr, tx_ctl);
            break;
        case oal_host2net_short(ETHER_TYPE_IPV6):
            hmac_set_cb_is_ipv6_tcp(ether_hdr, tx_ctl);
            break;
        default:
            break;
    }
}

/*
 * 功能描述  :  初始化CB tx rx字段
 * 1.日    期  : 2020年2月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_tx_cb_field_init(mac_tx_ctl_stru *tx_ctl, mac_vap_stru *mac_vap,
    uint16_t user_idx, oal_netbuf_stru *netbuf)
{
    mac_ether_header_stru *ether_hdr = (mac_ether_header_stru *)oal_netbuf_payload(netbuf);
    uint16_t ether_type = ether_hdr->us_ether_type;

    if (is_ap(mac_vap)) {
        mac_get_cb_is_mcast(tx_ctl) = ether_is_multicast(ether_hdr->auc_ether_dhost);
    }

    if (ether_type == oal_host2net_short(ETHER_TYPE_IP)) {
        mac_get_cb_is_icmp(tx_ctl) = oal_netbuf_is_icmp((oal_ip_header_stru *)(ether_hdr + 1));
    }

    hmac_set_cb_is_tcp(tx_ctl, ether_hdr, ether_type);
    mac_get_cb_ether_type(tx_ctl) = ether_type;
    mac_get_cb_mpdu_num(tx_ctl) = 1;
    mac_get_cb_netbuf_num(tx_ctl) = 1;
    mac_get_cb_wlan_frame_type(tx_ctl) = WLAN_DATA_BASICTYPE;
    mac_get_cb_ack_policy(tx_ctl) = WLAN_TX_NORMAL_ACK;
    mac_get_cb_tx_vap_index(tx_ctl) = mac_vap->uc_vap_id;
    mac_get_cb_tx_user_idx(tx_ctl) = user_idx;
    /*  初始队列，非qos帧入VO队列 */
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_AC_VO;
    mac_get_cb_wme_tid_type(tx_ctl) = WLAN_TIDNO_VOICE;
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_NUM; /* 后续识别模块刷新 */
    mac_get_cb_data_type(tx_ctl) = DATA_TYPE_80211;
}

uint32_t hmac_tx_lan_to_wlan_no_tcp_opt_tx_pass_handle(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf)
{
    frw_event_stru *pst_event = NULL; /* 事件结构体 */
    frw_event_mem_stru *pst_event_mem = NULL;
    dmac_tx_event_stru *pst_dtx_stru = NULL;
    mac_tx_ctl_stru *pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    uint8_t data_type = pst_tx_ctl->uc_frame_subtype;
    hmac_vap_stru *pst_hmac_vap = NULL;
    uint32_t ret;

    if (pst_buf == NULL || pst_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_vap->uc_vap_id); /* VAP结构体 */
    /* 维测，输出一个关键帧打印 */
    if (data_type <= MAC_DATA_ARP_REQ) {
        hmac_tx_vip_info(pst_vap, data_type, pst_buf, pst_tx_ctl);
    }
    hmac_tx_rtsp_info(pst_hmac_vap, data_type, pst_buf);
#ifdef _PRE_WLAN_PKT_TIME_STAT
    if (DELAY_STATISTIC_SWITCH_ON && IS_NEED_RECORD_DELAY(pst_buf, TP_SKB_HMAC_XMIT)) {
        skbprobe_record_time(pst_buf, TP_SKB_HMAC_TX);
    }
#endif
    /* 抛事件，传给DMAC */
    pst_event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(pst_vap->uc_vap_id, OAM_SF_TX, "{hmac_tx_lan_to_wlan::frw_event_alloc_m failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_HOST_DRX,
                       DMAC_TX_HOST_DRX, sizeof(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1, pst_vap->uc_chip_id,
                       pst_vap->uc_device_id, pst_vap->uc_vap_id);

    pst_dtx_stru = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_dtx_stru->pst_netbuf = pst_buf;
    pst_dtx_stru->us_frame_len = mac_get_cb_mpdu_len(pst_tx_ctl);

    /* 调度事件 */
    ret = frw_event_dispatch_event(pst_event_mem);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_TX,
                         "{hmac_tx_lan_to_wlan::frw_event_dispatch_event failed[%d].}", ret);
        oam_stat_vap_incr(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
    }

    /* 释放事件 */
    frw_event_free_m(pst_event_mem);

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
    /* RR性能检测发送流程hmac to dmac 位置打点 */
    hmac_rr_tx_h2d_timestamp();
#endif
    return ret;
}

static uint32_t hmac_device_tx(
    mac_vap_stru *mac_vap, hmac_vap_stru *hmac_vap, oal_netbuf_stru **origin_netbuf, mac_tx_ctl_stru *tx_ctl)
{
    uint32_t ret = HMAC_TX_PASS;
    oal_netbuf_stru *netbuf = *origin_netbuf;

#ifndef _PRE_WINDOWS_SUPPORT
    /* csum硬化仅host发包流程支持，device发包需要由驱动计算 */
    if (netbuf->ip_summed == CHECKSUM_PARTIAL) {
        oal_skb_checksum_help(netbuf);
    }
#endif

    /* 由于LAN TO WLAN和WLAN TO WLAN的netbuf都走这个函数，为了区分，需要先判断
       到底是哪里来的netbuf然后再对CB的事件类型字段赋值 */
    if (mac_get_cb_event_type(tx_ctl) != FRW_EVENT_TYPE_WLAN_DTX) {
        mac_get_cb_event_type(tx_ctl) = FRW_EVENT_TYPE_HOST_DRX;
        mac_get_cb_event_subtype(tx_ctl) = DMAC_TX_HOST_DRX;
    }

    /* 忽略host侧丢包，入口处则统计 */
    hmac_tx_pkts_stat(1);
    /* 发数据包计数统计 */
    hmac_stat_device_tx_msdu(oal_netbuf_len(netbuf));

    oal_spin_lock_bh(&hmac_vap->st_lock_state);
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /*  处理当前 MPDU */
        ret = hmac_tx_lan_mpdu_process_ap(hmac_vap, netbuf, tx_ctl);
    } else if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 处理当前MPDU */
#ifdef _PRE_WINDOWS_SUPPORT
        if (hmac_vap->fips == TRUE) {
            ret = hmac_tx_lan_mpdu_process_fips(hmac_vap, netbuf, tx_ctl);
        } else {
#endif
        ret = hmac_tx_lan_mpdu_process_sta(hmac_vap, netbuf, tx_ctl);
#ifdef _PRE_WINDOWS_SUPPORT
        }
#endif
    }

#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))
    if ((ret == HMAC_TX_PASS) && (hmac_check_soft_encrypt_port(mac_vap, netbuf) == OAL_TRUE) &&
        (wlan_chip_is_support_hw_wapi() == OAL_FALSE) && (hmac_check_soft_encrypt_thread() == OAL_TRUE)) {
        // 检查wapi、pwl端口打开，并且加密线程已创建成功，netbuf入链表缓存，在加密线程中处理。
        hmac_soft_encrypt_add_list(netbuf);
        oal_spin_unlock_bh(&hmac_vap->st_lock_state);
        return OAL_SUCC;
    }
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

    oal_spin_unlock_bh(&hmac_vap->st_lock_state);

    if (oal_likely(ret == HMAC_TX_PASS)) {
        return hmac_tx_lan_to_wlan_no_tcp_opt_tx_pass_handle(mac_vap, *origin_netbuf);
    } else if (oal_unlikely((ret == HMAC_TX_BUFF) || (ret == HMAC_TX_DONE))) {
        return OAL_SUCC;
    }
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_TX, "{::HMAC_TX_DROP.reason[%d]!}", ret);
    return ret;
}

/*
 * 函 数 名  : hmac_tx_lan_to_wlan_no_tcp_opt
 * 功能描述  : hmac AP模式 处理HOST DRX事件，注册到事件管理模块中
 * 1.日    期  : 2013年2月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tx_lan_to_wlan_no_tcp_opt(mac_vap_stru *mac_vap, oal_netbuf_stru **netbuf)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(*netbuf);
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_TX, "{hmac_tx_lan_to_wlan_no_tcp_opt::vap/user null.}");
        oam_stat_vap_incr(mac_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* VAP模式判断 */
    if (oal_unlikely(mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP && mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_TX, "{no_tcp_opt::en_vap_mode=%d.}", mac_vap->en_vap_mode);
        oam_stat_vap_incr(mac_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 如果关联用户数量为0，则丢弃报文 */
    if (oal_unlikely(hmac_vap->st_vap_base_info.us_user_nums == 0)) {
        oam_stat_vap_incr(mac_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
        return OAL_FAIL;
    }

    if (hmac_is_ring_tx(mac_vap, *netbuf, tx_ctl)) {
        return hmac_ring_tx(hmac_vap, netbuf, tx_ctl);
    } else {
        return hmac_device_tx(mac_vap, hmac_vap, netbuf, tx_ctl);
    }
}

OAL_STATIC uint32_t hmac_transfer_tx_handler(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
#ifdef _PRE_WLAN_TCP_OPT
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oam_warning_log0(0, OAM_SF_TX, "{hmac_transfer_tx_handler::netbuf is tcp ack.}\r\n");
#endif
    oal_spin_lock_bh(&hmac_vap->st_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
    oal_netbuf_list_tail(&hmac_vap->st_hmac_tcp_ack[HCC_TX].data_queue[HMAC_TCP_ACK_QUEUE], netbuf);

    /* 单纯TCP ACK等待调度, 特殊报文马上发送 */
    if (!mac_get_cb_urgent_tcp_ack(tx_ctl)) {
        oal_spin_unlock_bh(&hmac_vap->st_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        hmac_sched_transfer();
    } else {
        oal_spin_unlock_bh(&hmac_vap->st_hmac_tcp_ack[HCC_TX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        hmac_tcp_ack_process();
    }
    return OAL_SUCC;
#endif
}

/*
 * 函 数 名  : hmac_tx_wlan_to_wlan_ap
 * 功能描述  : (1)WLAN TO WLAN流程tx入口函数，接收rx流程抛过来的netbuf或者netbuf链，
 *            其中每一个netbuf里面有一个MSDU，每一个MSDU的内容是一个以太网格式的
 *            帧。如果是netbuf链，第一个netbuf的prev指针为空，最后一个netbuf的
 *            next指针为空。
 *            (2)函数循环调用LAN TO WLAN的入口函数来处理每一个MSDU，这样就把
 *            WLAN TO WLAN流程统一到了LAN TO WLAN流程
 * 1.日    期  : 2012年11月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tx_wlan_to_wlan_ap(oal_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL; /* 事件结构体 */
    mac_vap_stru *pst_mac_vap = NULL;
    oal_netbuf_stru *pst_buf = NULL;     /* 从netbuf链上取下来的指向netbuf的指针 */
    oal_netbuf_stru *pst_buf_tmp = NULL; /* 暂存netbuf指针，用于while循环 */
    mac_tx_ctl_stru *pst_tx_ctl = NULL;
    uint32_t ret;
    /* 入参判断 */
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_wlan_to_wlan_ap::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取事件 */
    pst_event = frw_get_event_stru(pst_event_mem);
    if (oal_unlikely(pst_event == NULL)) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_wlan_to_wlan_ap::pst_event null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取PAYLOAD中的netbuf链 */
    pst_buf = (oal_netbuf_stru *)(uintptr_t)(*((unsigned long *)(pst_event->auc_event_data)));

    ret = hmac_tx_get_mac_vap(pst_event->st_event_hdr.uc_vap_id, &pst_mac_vap);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_TX,
                       "{hmac_tx_wlan_to_wlan_ap::hmac_tx_get_mac_vap failed[%d].}", ret);
        hmac_free_netbuf_list(pst_buf);
        return ret;
    }

    /* 循环处理每一个netbuf，按照以太网帧的方式处理 */
    while (pst_buf != NULL) {
        pst_buf_tmp = oal_netbuf_next(pst_buf);

        oal_netbuf_next(pst_buf) = NULL;
        oal_netbuf_prev(pst_buf) = NULL;

        /* 由于此netbuf来自接收流程，是从内存池申请的，而以太网过来的netbuf是从
           操作系统申请的，二者的释放方式不一样，后续要通过事件类型字段来选择正确
           的释放方式
        */
        pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
        memset_s(pst_tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));

        mac_get_cb_event_type(pst_tx_ctl) = FRW_EVENT_TYPE_WLAN_DTX;
        mac_get_cb_event_subtype(pst_tx_ctl) = DMAC_TX_WLAN_DTX;

        /* set the queue map id when wlan to wlan */
        oal_skb_set_queue_mapping(pst_buf, WLAN_NORMAL_QUEUE);

        ret = hmac_tx_lan_to_wlan(pst_mac_vap, &pst_buf);
        /* 调用失败，自己调用自己释放netbuff内存 */
        if (ret != OAL_SUCC) {
            hmac_free_netbuf_list(pst_buf);
        }

        pst_buf = pst_buf_tmp;
    }

    return OAL_SUCC;
}

/* 功能：trx ring定制化开关控制 */
void hmac_set_trx_switch(uint32_t trx_switch_ini_param)
{
    memset_s(&g_trx_switch, sizeof(g_trx_switch), 0, sizeof(g_trx_switch));
    /* 接收是否支持ring切换 */
    g_trx_switch.rx_switch = trx_switch_ini_param & BIT0;
    /* 发送是否支持ring */
    g_trx_switch.host_ring_tx_switch = (trx_switch_ini_param & BIT1) >> NUM_1_BITS;
    g_trx_switch.device_ring_tx_switch = (trx_switch_ini_param & BIT2) >> NUM_2_BITS;
    g_trx_switch.tx_ring_switch = (trx_switch_ini_param & BIT3) >> NUM_3_BITS;
    /* 内存是否支持自适应调整 */
    g_trx_switch.netbuf_alloc_opt = (trx_switch_ini_param & BIT4) >> NUM_4_BITS;
    g_trx_switch.tx_netbuf_limit = (trx_switch_ini_param & BIT5) >> NUM_5_BITS;

    /* if support host ring, then set true */
    if (g_trx_switch.host_ring_tx_switch) {
        g_trx_switch.ring_switch_independent = OAL_TRUE; /* trx ring独立切换 */
        g_trx_switch.host_update_wptr = OAL_TRUE; /* host更新tx ring wptr */
        g_trx_switch.soft_irq_sched = wlan_chip_soft_irq_sch_support(); /* 软中断调度 */
        g_trx_switch.device_loop_sched = OAL_TRUE; /* device loop调度 */
    }
    g_trx_switch.rx_complete_thread_disable = (trx_switch_ini_param & BIT6) >> NUM_6_BITS;
    oam_warning_log3(0, OAM_SF_CFG, "{hmac_set_trx_switch::ini[0x%x], ring switch[%d] independent[%d]}",
        trx_switch_ini_param, g_trx_switch.tx_ring_switch, g_trx_switch.ring_switch_independent);
    oam_warning_log3(0, OAM_SF_CFG, "{hmac_set_trx_switch::wptr[%d] softirq[%d] loop[%d]}",
        g_trx_switch.host_update_wptr, g_trx_switch.soft_irq_sched, g_trx_switch.device_loop_sched);
}

/*
 * 函 数 名  : hmac_get_tx_user_id
 * 功能描述  : tx方向获取user id，填写cb信息- device tx使用
 * 1.日    期  : 2013年2月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint16_t hmac_tx_get_user_id(mac_vap_stru *mac_vap, mac_ether_header_stru *ether_hdr)
{
    uint16_t user_idx = g_wlan_spec_cfg->invalid_user_id;
    uint8_t *addr = NULL;                     /* 目的地址 */
    uint32_t ret;

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        return mac_vap->us_assoc_vap_id;
    }

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 判断是组播或单播,对于lan to wlan的单播帧，返回以太网地址 */
        addr = ether_hdr->auc_ether_dhost;
        /* 单播数据帧 */
        if (oal_likely(!ether_is_multicast(addr))) {
            ret = mac_vap_find_user_by_macaddr(mac_vap, addr, &user_idx);
            if (oal_unlikely(ret != OAL_SUCC)) {
                oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_TX, "{hmac_get_tx_user_id::hmac_tx_find_user failed \
                    %02X:XX:XX:XX:%02X:%02X}", addr[MAC_ADDR_0], addr[MAC_ADDR_4], addr[MAC_ADDR_5]);
                return g_wlan_spec_cfg->invalid_user_id;
            }
        } else { /* 组播 or 广播 */
            user_idx = mac_vap->us_multi_user_idx;
        }
    }

    return user_idx;
}

/*
 * 函 数 名  : hmac_tx_lan_to_wlan_tcp_opt
 * 功能描述  : hmac AP模式 处理HOST DRX事件，注册到事件管理模块中
 * 1.日    期  : 2013年2月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tx_lan_to_wlan(mac_vap_stru *mac_vap, oal_netbuf_stru **netbuf)
{
    mac_ether_header_stru *ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(*netbuf);
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(*netbuf);
    uint16_t us_user_idx = hmac_tx_get_user_id(mac_vap, ether_hdr);
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(us_user_idx);
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id); /* VAP结构体 */
    if (oal_unlikely((hmac_vap == NULL) || (hmac_user == NULL))) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tx_lan_to_wlan::dmac_vap  or user null.}\r\n");
        return OAL_FAIL;
    }

    /* 初始化CB tx rx字段 , CB字段在前面已经被清零， 在这里不需要重复对某些字段赋零值 */
    hmac_tx_cb_field_init(tx_ctl, mac_vap, us_user_idx, *netbuf);
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 用户帧统计 */
    if (hmac_chba_vap_start_check(hmac_vap) == OAL_TRUE) {
        hmac_stat_user_tx_netbuf(hmac_user, *netbuf);
    }
#endif
    /* 以太网业务识别 tid/datatype刷新到cb中 */
    hmac_tx_classify(hmac_vap, hmac_user, *netbuf, tx_ctl);

    if ((hmac_vap->sys_tcp_tx_ack_opt_enable == OAL_TRUE) &&
        (mac_get_cb_ipv6_tcp_ack(tx_ctl) || mac_get_cb_is_tcp_ack(tx_ctl))) {
        return hmac_transfer_tx_handler(hmac_vap, *netbuf);
    } else {
        return hmac_tx_lan_to_wlan_no_tcp_opt(mac_vap, netbuf);
    }
}

static oal_net_dev_tx_enum hmac_bridge_vap_xmit_pre_get_res_check(oal_netbuf_stru *pst_buf,
    oal_net_device_stru *pst_dev, mac_vap_stru **pst_vap, hmac_vap_stru **pst_hmac_vap)
{
    /* 获取VAP结构体 */
    *pst_vap = (mac_vap_stru *)oal_net_dev_priv(pst_dev);
    /* 如果VAP结构体不存在，则丢弃报文 */
    if (oal_unlikely(*pst_vap == NULL)) {
        /* will find vap fail when receive a pkt from
         * kernel while vap is deleted, return OAL_NETDEV_TX_OK is so. */
        oam_warning_log0(0, OAM_SF_TX, "{hmac_bridge_vap_xmit::pst_vap = NULL!}\r\n");
        oal_netbuf_free(pst_buf);
        oam_stat_vap_incr(0, tx_abnormal_msdu_dropped, 1);
        return OAL_NETDEV_TX_OK;
    }

    *pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap((*pst_vap)->uc_vap_id);
    if (*pst_hmac_vap == NULL) {
        oam_error_log0((*pst_vap)->uc_vap_id, OAM_SF_CFG, "{hmac_bridge_vap_xmit::pst_hmac_vap null.}");
        oal_netbuf_free(pst_buf);
        return OAL_NETDEV_TX_OK;
    }

    /* 代码待整改，pst_device_stru指针切换未状态, 长发长收切换未本地状态 */
    if ((*pst_vap)->bit_al_tx_flag == OAL_SWITCH_ON) {
        oal_netbuf_free(pst_buf);
        return OAL_NETDEV_TX_OK;
    }
    return OAL_NETDEV_TX_BUSY;
}

static oal_bool_enum_uint8 hmac_bridge_vap_xmit_check_drop_frame(mac_vap_stru *vap,
    oal_netbuf_stru *buf)
{
    oal_bool_enum_uint8 drop_frame = OAL_FALSE;
    uint8_t data_type;

    if (vap->en_vap_state != MAC_VAP_STATE_STA_LISTEN) {
        return OAL_TRUE;
    }

    if (vap->us_user_nums != 0) {
        return OAL_FALSE;
    }

    data_type = mac_get_data_type_from_8023(buf, (uint8_t *)oal_netbuf_payload(buf),
        MAC_NETBUFF_PAYLOAD_ETH, oal_netbuf_get_len(buf), OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    if ((data_type != MAC_DATA_EAPOL) && (data_type != MAC_DATA_DHCP)) {
        oam_warning_log1(vap->uc_vap_id, OAM_SF_TX, "{hmac_bridge_vap_xmit_check_drop_frame::dropskb type %d!}",
            data_type);
        drop_frame = OAL_TRUE;
    } else {
        oam_warning_log2(vap->uc_vap_id, OAM_SF_TX,
            "{hmac_bridge_vap_xmit::donot drop [%d]frame[EAPOL:1,DHCP:0]. vap state[%d].}",
            data_type, vap->en_vap_state);
    }

    return drop_frame;
}
#ifdef _PRE_WLAN_CHBA_MGMT
void hmac_bridge_vap_xmit_chba(oal_netbuf_stru *pst_buf, oal_net_device_stru *pst_dev,
    hmac_vap_stru *pst_hmac_vap, mac_vap_stru *pst_vap)
{
    hmac_user_stru *hmac_user = NULL;
    hmac_chba_vap_stru *chba_vap_info = hmac_get_up_chba_vap_info();
    oal_netbuf_stru *copy_buf = NULL;
    uint8_t *puc_addr; /* 目的地址 */
    mac_ether_header_stru *pst_ether_hdr = NULL;
    uint8_t user_bitmap_idx;

    if (chba_vap_info == NULL) {
        oal_netbuf_free(pst_buf);
        return;
    }
    for (user_bitmap_idx = 0; user_bitmap_idx < g_wlan_spec_cfg->max_user_limit; user_bitmap_idx++) {
        if ((pst_hmac_vap->user_id_bitmap & BIT(user_bitmap_idx)) == 0) {
            continue;
        }

        hmac_user = mac_res_get_hmac_user(user_bitmap_idx);
        if (hmac_user == NULL) {
            continue;
        }
        if (hmac_user->st_user_base_info.en_is_multi_user == OAL_TRUE) {
            continue;
        }
        /* 复制netbuf */
        copy_buf = oal_netbuf_copy(pst_buf, GFP_ATOMIC);
        if (copy_buf == NULL) {
            oam_warning_log0(0, 0, "CHBA: pst_copy_buf is null");
            continue;
        }
        pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(copy_buf);
        puc_addr = pst_ether_hdr->auc_ether_dhost;
        /* 将user的mac地址复制给dhost的mac地址 */
        memcpy_s(puc_addr, WLAN_MAC_ADDR_LEN, hmac_user->st_user_base_info.auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

        if (oal_unlikely(hmac_tx_lan_to_wlan(pst_vap, &copy_buf) != OAL_SUCC)) {
            /* 调用失败，要释放内核申请的netbuff内存池 */
            hmac_free_netbuf_list(copy_buf);
        }
    }
    /* 释放netbuf */
    oal_netbuf_free(pst_buf);
}
#endif

static uint32_t hmac_bridge_vap_state_check(hmac_vap_stru *pst_hmac_vap, oal_netbuf_stru *pst_buf)
{
    uint8_t uc_data_type;
    mac_vap_stru *pst_vap = &(pst_hmac_vap->st_vap_base_info);
    /* 考虑VAP状态与控制面互斥，需要加锁保护 */
    oal_spin_lock_bh(&pst_hmac_vap->st_lock_state);
    /* 判断VAP的状态，如果ROAM，则丢弃报文 MAC_DATA_DHCP/MAC_DATA_ARP */
    if (pst_vap->en_vap_state == MAC_VAP_STATE_ROAMING) {
        uc_data_type = mac_get_data_type_from_8023(pst_buf, (uint8_t *)oal_netbuf_payload(pst_buf),
            MAC_NETBUFF_PAYLOAD_ETH, oal_netbuf_get_len(pst_buf), OAM_OTA_FRAME_DIRECTION_TYPE_TX);
#ifdef _PRE_WLAN_FEATURE_PWL
        if (((pst_hmac_vap->st_vap_base_info.pwl_enable == OAL_TRUE) && (uc_data_type != MAC_DATA_WAPI)) ||
            ((pst_hmac_vap->st_vap_base_info.pwl_enable == OAL_FALSE) && (uc_data_type != MAC_DATA_EAPOL))) {
#else
        if (uc_data_type != MAC_DATA_EAPOL) {
#endif
            if (uc_data_type != MAC_DATA_BUTT) {
                oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_TX, "{vap_xmit::drop skb type %d!}", uc_data_type);
            }
            oal_netbuf_free(pst_buf);
            oam_stat_vap_incr(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
            oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);
            return OAL_FAIL;
        }
    } else if ((pst_vap->en_vap_state != MAC_VAP_STATE_UP) && (pst_vap->en_vap_state != MAC_VAP_STATE_PAUSE)) {
        /* 入网过程中触发p2p扫描不丢dhcp、eapol帧，防止入网失败 */
        if (hmac_bridge_vap_xmit_check_drop_frame(pst_vap, pst_buf) == OAL_TRUE) {
            /* filter the tx xmit pkts print */
            if (pst_vap->en_vap_state == MAC_VAP_STATE_INIT || pst_vap->en_vap_state == MAC_VAP_STATE_BUTT) {
                oam_warning_log1(pst_vap->uc_vap_id, OAM_SF_TX,
                    "{hmac_bridge_vap_xmit::vap state[%d] != MAC_VAP_STATE_{UP|PAUSE}!}", pst_vap->en_vap_state);
            }
            oal_netbuf_free(pst_buf);
            oam_stat_vap_incr(pst_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
            oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);
            return OAL_FAIL;
        }
    }
    oal_spin_unlock_bh(&pst_hmac_vap->st_lock_state);

    return OAL_SUCC;
}

void hmac_vap_xmit(oal_net_device_stru *pst_dev, hmac_vap_stru *pst_hmac_vap, mac_vap_stru *pst_vap,
    oal_netbuf_stru *pst_buf)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    mac_ether_header_stru *ether_hdr = NULL;
#endif
#ifdef _PRE_DELAY_DEBUG
    hmac_wifi_delay_tx_notify(pst_hmac_vap, pst_buf);
#endif
    if (is_sta(pst_vap)) {
        /* 发送方向的arp_req 统计和重关联的处理 */
        hma_arp_probe_timer_start(pst_buf, pst_hmac_vap);
    }

    /* 将以太网过来的帧上报SDT */
    hmac_tx_report_eth_frame(pst_hmac_vap, pst_buf);

    /* 非组播或非chba mode */
#ifdef _PRE_WLAN_CHBA_MGMT
    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_buf);
    if (!(ether_is_multicast(ether_hdr->auc_ether_dhost)) || (!hmac_chba_vap_start_check(pst_hmac_vap))) {
#endif
        if (oal_unlikely(hmac_tx_lan_to_wlan(pst_vap, &pst_buf) != OAL_SUCC)) {
            hmac_free_netbuf_list(pst_buf);
        }
#ifdef _PRE_WLAN_CHBA_MGMT
    } else {
        hmac_bridge_vap_xmit_chba(pst_buf, pst_dev, pst_hmac_vap, pst_vap);
    }
#endif
    g_hisi_softwdt_check.xmit_cnt++;
}

uint8_t g_sk_pacing_shift = 8;
static uint32_t hmac_bridge_vap_xmit_preprocess(oal_netbuf_stru *netbuf)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) && defined(_PRE_WLAN_MP13_TCP_SMALL_QUEUE_BUGFIX)
    /* We need a bit of data queued to build aggregates properly, so
     * instruct the TCP stack to allow more than a single ms of data
     * to be queued in the stack. The value is a bit-shift of 1
     * second, so 6 is 16ms of queued data. Only affects local TCP UL
     * sockets.
     */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    sk_pacing_shift_update(netbuf->sk, g_sk_pacing_shift);
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_GSO
    if (skb_linearize(netbuf)) {
        oam_warning_log0(0, OAM_SF_RX, "{wal_bridge_vap_xmit::[GSO] failed at skb_linearize");
        dev_kfree_skb_any(netbuf);
        return OAL_FAIL;
    }
#endif
#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
    /* RR性能检测发送流程host起始位置打点 */
    hmac_rr_tx_w2h_timestamp();
#endif
#ifdef _PRE_SKB_TRACE
    mem_trace_add_node((uintptr_t)netbuf);
#endif

    return OAL_SUCC;
}

static void hmac_bridge_vap_xmit_netbuf_init(oal_netbuf_stru *netbuf)
{
    oal_netbuf_next(netbuf) = NULL;
    oal_netbuf_prev(netbuf) = NULL;
#ifdef _PRE_WLAN_PKT_TIME_STAT
    if (DELAY_STATISTIC_SWITCH_ON && IS_NEED_RECORD_DELAY(netbuf, TP_SKB_IP)) {
        skbprobe_record_time(netbuf, TP_SKB_HMAC_XMIT);
    }
#endif
#ifdef _PRE_WLAN_PKT_TIME_STAT
    memset_s(oal_netbuf_cb(netbuf), OAL_NETBUF_CB_ORIGIN, 0, OAL_NETBUF_CB_ORIGIN);
#else
    memset_s(oal_netbuf_cb(netbuf), oal_netbuf_cb_size(), 0, oal_netbuf_cb_size());
#endif
}

/*
 * 功能描述  : 挂接到VAP对应net_device结构体下的发送函数
 */
oal_net_dev_tx_enum hmac_bridge_vap_xmit(oal_netbuf_stru *netbuf, oal_net_device_stru *net_device)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    netbuf = oal_netbuf_unshare(netbuf, GFP_ATOMIC);
    if (oal_unlikely(!netbuf)) {
        return OAL_NETDEV_TX_OK;
    }

    if (hmac_bridge_vap_xmit_preprocess(netbuf) != OAL_SUCC) {
        return OAL_NETDEV_TX_OK;
    }

    if (hmac_bridge_vap_xmit_pre_get_res_check(netbuf, net_device, &mac_vap, &hmac_vap) == OAL_NETDEV_TX_OK) {
        return OAL_NETDEV_TX_OK;
    }

    if (hmac_bridge_vap_state_check(hmac_vap, netbuf) != OAL_SUCC) {
        return OAL_NETDEV_TX_OK;
    }

    hmac_bridge_vap_xmit_netbuf_init(netbuf);
    hmac_vap_xmit(net_device, hmac_vap, mac_vap, netbuf);

    return OAL_NETDEV_TX_OK;
}
