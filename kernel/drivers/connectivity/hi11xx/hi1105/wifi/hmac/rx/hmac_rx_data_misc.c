/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : DMAC模块接收帧的公共操作函数以及数据帧的操作函数定义的源文件
 * 作    者 :
 * 创建日期 : 2012年11月20日
 */

#include "oal_net.h"
#include "mac_frame.h"
#include "mac_data.h"
#include "hmac_frag.h"
#include "hmac_host_rx.h"
#include "hmac_sniffer.h"
#include "hmac_wapi.h"
#include "oam_stat_wifi.h"
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
#include <linux/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/ieee80211.h>
#include <linux/ipv6.h>
#endif
#include "oam_event_wifi.h"
#ifdef CONFIG_HUAWEI_DUBAI
#include <chipset_common/dubai/dubai.h>
#include <huawei_platform/log/hwlog_kernel.h>
#endif
#include "hmac_ota_report.h"
#include "hmac_stat.h"
#ifdef _PRE_WLAN_FEATURE_VSP
#include "hmac_vsp_if.h"
#endif
#include "hmac_wifi_delay.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_DATA_MISC_C

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE

static void hmac_parse_ipv4_packet_ipprot_udp(const struct iphdr *iph, uint32_t iphdr_len, uint32_t netbuf_len)
{
    struct udphdr *uh = NULL;
    if (netbuf_len < iphdr_len + sizeof(struct udphdr)) {
        oam_error_log2(0, OAM_SF_M2U, "{ipv4::netbuf_len[%d], protocol[%d]}", netbuf_len, iph->protocol);
        return;
    }
    uh = (struct udphdr *)((uint8_t *)iph + iphdr_len);
    oal_io_print(WIFI_WAKESRC_TAG "UDP packet, src port:%d, dst port:%d.\n",
        oal_ntoh_16(uh->source), oal_ntoh_16(uh->dest));
#ifdef CONFIG_HUAWEI_DUBAI
    dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP_UDP_V4", "port", oal_ntoh_16(uh->dest));
#endif
}

static void hmac_parse_ipv4_packet_ipprot_tcp(const struct iphdr *iph, uint32_t iphdr_len, uint32_t netbuf_len)
{
    struct tcphdr *th = NULL;
    if (netbuf_len < iphdr_len + sizeof(struct tcphdr)) {
        oam_error_log2(0, OAM_SF_M2U, "{ipv4::netbuf_len[%d], protocol[%d]}", netbuf_len, iph->protocol);
        return;
    }
    th = (struct tcphdr *)((uint8_t *)iph + iphdr_len);
    oal_io_print(WIFI_WAKESRC_TAG "TCP packet, src port:%d, dst port:%d.\n",
        oal_ntoh_16(th->source), oal_ntoh_16(th->dest));
#ifdef CONFIG_HUAWEI_DUBAI
    HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP_TCP_V4", "port=%d", oal_ntoh_16(th->dest));
#endif
}

static void hmac_parse_ipv4_packet_ipprot_icmp(const struct iphdr *iph, uint32_t iphdr_len, uint32_t netbuf_len)
{
    struct icmphdr *icmph = NULL;
    if (netbuf_len < iphdr_len + sizeof(struct icmphdr)) {
        oam_error_log2(0, OAM_SF_M2U, "{ipv4::netbuf_len[%d], protocol[%d]}", netbuf_len, iph->protocol);
        return;
    }
    icmph = (struct icmphdr *)((uint8_t *)iph + iphdr_len);
    oal_io_print(WIFI_WAKESRC_TAG "ICMP packet, type(%d):%s, code:%d.\n", icmph->type,
        ((icmph->type == 0) ? "ping reply" : ((icmph->type == 8) ? "ping request" : "other icmp pkt")), icmph->code);
#ifdef CONFIG_HUAWEI_DUBAI
    dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP", "protocol", (int32_t)iph->protocol);
#endif
}

/*
 * 函 数 名  : hmac_parse_ipv4_packet
 * 功能描述  : 当报文是IPV4类型时，进一步解析type，ipaddr，port
 * 1.日    期  : 2017年2月20日
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_parse_ipv4_packet(mac_ether_header_stru *pst_eth, uint32_t netbuf_len)
{
    const struct iphdr *iph = NULL;
    uint32_t iphdr_len;
    /* iphdr: 最小长度为 20 */
    if (netbuf_len < BYTE_OFFSET_20) {
        oam_error_log1(0, OAM_SF_M2U, "{hmac_parse_ipv4_packet::netbuf_len[%d]}", netbuf_len);
        return;
    }
    oal_io_print(WIFI_WAKESRC_TAG "ipv4 packet.\n");
    iph = (struct iphdr *)(pst_eth + 1);
    iphdr_len = iph->ihl * BYTE_OFFSET_4;

    if (iph->protocol == IPPROTO_UDP) {
        hmac_parse_ipv4_packet_ipprot_udp(iph, iphdr_len, netbuf_len);
    } else if (iph->protocol == IPPROTO_TCP) {
        hmac_parse_ipv4_packet_ipprot_tcp(iph, iphdr_len, netbuf_len);
    } else if (iph->protocol == IPPROTO_ICMP) {
        hmac_parse_ipv4_packet_ipprot_icmp(iph, iphdr_len, netbuf_len);
    } else if (iph->protocol == IPPROTO_IGMP) {
        oal_io_print(WIFI_WAKESRC_TAG "IGMP packet.\n");
#ifdef CONFIG_HUAWEI_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP", "protocol", (int32_t)iph->protocol);
#endif
    } else {
        oal_io_print(WIFI_WAKESRC_TAG "other IPv4 packet.\n");
#ifdef CONFIG_HUAWEI_DUBAI
        dubai_log_packet_wakeup_stats("DUBAI_TAG_PACKET_WAKEUP", "protocol", (int32_t)iph->protocol);
#endif
    }
}

/*
 * 函 数 名  : hmac_parse_ipv4_packet
 * 功能描述  : 当报文是IPV6类型时，进一步解析type，ipaddr
 * 1.日    期  : 2017年2月20日
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_parse_ipv6_packet(mac_ether_header_stru *pst_eth, uint32_t buf_len)
{
    struct ipv6hdr *ipv6h = NULL;
    oal_icmp6hdr_stru *icmph = NULL;
#ifdef CONFIG_HUAWEI_DUBAI
    struct tcphdr *tcph = NULL;
#endif
    if (buf_len < sizeof(struct ipv6hdr)) {
        oam_error_log2(0, OAM_SF_ANY, "{buf_len[%d], ipv6hdr[%d]}", buf_len, sizeof(struct ipv6hdr));
        return;
    }
    buf_len -= sizeof(struct ipv6hdr);
    oal_io_print(WIFI_WAKESRC_TAG "ipv6 packet.\n");
    ipv6h = (struct ipv6hdr *)(pst_eth + 1);
    oal_io_print(WIFI_WAKESRC_TAG "version: %d, payload length: %d, nh->nexthdr: %d. \n",
                 ipv6h->version, oal_ntoh_16(ipv6h->payload_len), ipv6h->nexthdr);
    oal_io_print(WIFI_WAKESRC_TAG "ipv6 src addr:%04x:x:x:x:x:x:x:%04x \n", ipaddr6(ipv6h->saddr));
    oal_io_print(WIFI_WAKESRC_TAG "ipv6 dst addr:%04x:x:x:x:x:x:x:%04x \n", ipaddr6(ipv6h->daddr));
    if (OAL_IPPROTO_ICMPV6 == ipv6h->nexthdr) {
        if (buf_len < sizeof(oal_icmp6hdr_stru)) {
            oam_error_log2(0, OAM_SF_ANY, "{buf_len[%d], icmp6hdr[%d]}", buf_len, sizeof(oal_icmp6hdr_stru));
            return;
        }
        icmph = (oal_icmp6hdr_stru *)(ipv6h + 1);
        oal_io_print(WIFI_WAKESRC_TAG "ipv6 nd type: %d. \n", icmph->icmp6_type);
    }
#ifdef CONFIG_HUAWEI_DUBAI
    if (ipv6h->nexthdr == IPPROTO_TCP) {
        tcph = (struct tcphdr *)(ipv6h + 1);
        HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP_TCP_V6", "port=%d", ntohs(tcph->dest));
    } else {
        HWDUBAI_LOGE("DUBAI_TAG_PACKET_WAKEUP", "protocol=%d", IPPROTO_IPV6);
    }
#endif

    return;
}

/*
 * 函 数 名  : hmac_parse_arp_packet
 * 功能描述  : 当报文是arp类型时，进一步解析type，subtype
 * 1.日    期  : 2017年2月20日
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_parse_arp_packet(mac_ether_header_stru *pst_eth, uint32_t buf_len)
{
    struct arphdr *arp = NULL;

    if (buf_len < sizeof(struct arphdr)) {
        oam_error_log1(0, 0, "{hmac_parse_arp_packet::buf_len[%d].}", buf_len);
        return;
    }

    arp = (struct arphdr *)(pst_eth + 1);
    oal_io_print(WIFI_WAKESRC_TAG "ARP packet, hardware type:%d, protocol type:%d, opcode:%d.\n",
                 oal_ntoh_16(arp->ar_hrd), oal_ntoh_16(arp->ar_pro), oal_ntoh_16(arp->ar_op));

    return;
}

/*
 * 函 数 名  : parse_8021x_packet
 * 功能描述  : 当报文是arp类型时，进一步解析type，subtype
 * 1.日    期  : 2017年2月20日
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_parse_8021x_packet(mac_ether_header_stru *pst_eth, uint32_t buf_len)
{
    struct ieee8021x_hdr *hdr = (struct ieee8021x_hdr *)(pst_eth + 1);
    if (buf_len < sizeof(struct ieee8021x_hdr)) {
        oam_error_log1(0, 0, "{hmac_parse_packet::buf_len}", buf_len);
        return;
    }
    oal_io_print(WIFI_WAKESRC_TAG "802.1x frame: version:%d, type:%d, length:%d\n",
                 hdr->version, hdr->type, oal_ntoh_16(hdr->length));

    return;
}

/*
 * 函 数 名  : hmac_parse_packet
 * 功能描述  : 当系统被wifi唤醒时，解析数据报文的格式。
 * 1.日    期  : 2017年2月20日
 *   修改内容  : 新生成函数
 */
void hmac_parse_packet(oal_netbuf_stru *netbuf_eth)
{
    uint16_t us_type;
    uint32_t buf_len;
    mac_ether_header_stru *pst_ether_hdr = NULL;
    buf_len = oal_netbuf_len(netbuf_eth);
    if (buf_len < sizeof(mac_ether_header_stru)) {
        oam_error_log1(0, 0, "{hmac_parse_packet::buf_len}", buf_len);
        return;
    }
    buf_len -= sizeof(mac_ether_header_stru);
    pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(netbuf_eth);
    if (oal_unlikely(pst_ether_hdr == NULL)) {
        oal_io_print(WIFI_WAKESRC_TAG "ether header is null.\n");
        return;
    }

    us_type = pst_ether_hdr->us_ether_type;
    oal_io_print(WIFI_WAKESRC_TAG "protocol type:0x%04x\n", oal_ntoh_16(us_type));

    if (us_type == oal_host2net_short(ETHER_TYPE_IP)) {
        hmac_parse_ipv4_packet(pst_ether_hdr, buf_len);
    } else if (us_type == oal_host2net_short(ETHER_TYPE_IPV6)) {
        hmac_parse_ipv6_packet(pst_ether_hdr, buf_len);
    } else if (us_type == oal_host2net_short(ETHER_TYPE_ARP)) {
        hmac_parse_arp_packet(pst_ether_hdr, buf_len);
    } else if (us_type == oal_host2net_short(ETHER_TYPE_PAE)) {
        hmac_parse_8021x_packet(pst_ether_hdr, buf_len);
    } else {
        oal_io_print(WIFI_WAKESRC_TAG "receive other packet.\n");
    }

    return;
}
#endif
/*
 * 功能描述  : device rx msdu聚合处理
 * 1.日    期  : 2021.4.25
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_device_rx_amsdu_proc(mac_rx_ctl_stru *cb, hmac_user_stru *hmac_user, oal_netbuf_stru  **netbuf)
{
    oal_netbuf_stru          *temp_netbuf = NULL;
    oal_netbuf_stru          *first_netbuf = NULL;
    oal_netbuf_stru          *next_netbuf = NULL;
    oal_netbuf_head_stru     *list_head = &hmac_user->device_rx_list[mac_get_rx_cb_tid(cb)];
    mac_rx_ctl_stru          *temp_cb = NULL;

    if (mac_rxcb_is_first_msdu(cb) && (!mac_rxcb_is_last_msdu(cb))) {
        if (!oal_netbuf_list_empty(list_head)) {
            oam_error_log2(0, OAM_SF_RX, "{hmac_device_rx_amsdu_proc::list is not empty len = [%d], sn = [%d]}",
                oal_netbuf_list_len(list_head), mac_get_rx_cb_seq_num(cb));

            while ((temp_netbuf = oal_netbuf_delist(list_head)) != NULL) {
                temp_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(temp_netbuf);
                oam_error_log3(0, OAM_SF_RX, "{hmac_device_rx_amsdu_proc::sn = [%d], first = [%d], last = [%d]}",
                    mac_get_rx_cb_seq_num(temp_cb), mac_rxcb_is_first_msdu(temp_cb), mac_rxcb_is_last_msdu(temp_cb));
                oal_netbuf_free(temp_netbuf);
            }
            oal_netbuf_free_list(list_head, oal_netbuf_list_len(list_head));
        }
        oal_netbuf_list_head_init(list_head);
        oal_netbuf_add_to_list_tail(*netbuf, list_head);
        return OAL_RETURN;
    } else if (mac_rxcb_is_amsdu_sub_msdu(cb)) {
        oal_netbuf_add_to_list_tail(*netbuf, list_head);
        if (!mac_rxcb_is_last_msdu(cb)) {
            return OAL_RETURN;
        }
        first_netbuf = oal_netbuf_delist(list_head);
        temp_netbuf = first_netbuf;
        while (temp_netbuf != NULL) {
            next_netbuf = oal_netbuf_delist(list_head);
            oal_netbuf_next(temp_netbuf) = next_netbuf;
            temp_netbuf = next_netbuf;
        }
        *netbuf = first_netbuf;
    }

    return OAL_CONTINUE;
}
/*
 * 功能描述  : dev rx构建host rx上下文
 * 1.日    期  : 2020.7.20
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_INLINE void hmac_dev_build_rx_context(oal_netbuf_stru *netbuf, hmac_host_rx_context_stru *rx_context)
{
    mac_rx_ctl_stru *cb = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(cb->uc_mac_vap_id);

    rx_context->hal_dev = hal_get_host_device(hmac_vap->hal_dev_id);
    rx_context->hmac_vap = hmac_vap;
    rx_context->hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_get_rx_cb_ta_user_id(cb));
    rx_context->netbuf = netbuf;
    rx_context->cb = cb;
}
/*
 * 功能描述  : device rx专有流程
 * 1.日    期  : 2020.7.20
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_INLINE void hmac_dev_rx_unique_proc(hmac_host_rx_context_stru *rx_context)
{
    oal_netbuf_stru *netbuf = rx_context->netbuf;

    for (; netbuf != NULL; netbuf = oal_netbuf_next(netbuf)) {
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
        /* host低功耗唤醒维测 */
        if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
            oal_io_print(WIFI_WAKESRC_TAG "rx: hmac_parse_packet!\n");
            hmac_parse_packet(netbuf);
            wlan_pm_wkup_src_debug_set(OAL_FALSE);
        }
#endif
    }
}
uint32_t g_wifi_trx_log = 0;
void hmac_rx_route_print(hal_host_device_stru *hal_device, uint8_t step, mac_rx_ctl_stru *mac_rx_ctl)
{
    if (g_wifi_trx_log & HAL_LOG_RX_ROUTE_LOG) {
        oam_warning_log4(hal_device->device_id, OAM_SF_RX,
            "{rx_route_print::rx step[%d],tid[%d],process_flag[%d],seqnum[%d]}",
            step, mac_rx_ctl->bit_rx_tid, mac_rx_ctl->bit_process_flag, mac_rx_ctl->us_seq_num);
    }
}
OAL_INLINE void hmac_dev_rx_process_data_proc(hmac_host_rx_context_stru *rx_context)
{
    oal_netbuf_head_stru rpt_list;
    oal_netbuf_list_head_init(&rpt_list);
    hmac_rx_common_proc(rx_context, &rpt_list);
    hmac_rx_route_print(rx_context->hal_dev, DEVICE_RX_HOST_REPORT, rx_context->cb);
    if (oal_netbuf_list_len(&rpt_list)) {
        hmac_rx_rpt_netbuf(&rpt_list);
    }
}
/*
 * 功能描述  : dev rx数据帧上报总入口
 * 1.日    期  : 2020年1月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_dev_rx_process_data_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru           *event = NULL;
    dmac_wlan_drx_event_stru *wlan_rx_event = NULL;
    oal_netbuf_stru          *netbuf = NULL;
    hmac_vap_stru            *hmac_vap = NULL;
    hmac_host_rx_context_stru rx_context = { 0 };
    mac_rx_ctl_stru *cb = NULL;
    hmac_user_stru *hmac_user = NULL;

    if (oal_unlikely(event_mem == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    event = frw_get_event_stru(event_mem);
    wlan_rx_event = (dmac_wlan_drx_event_stru *)(event->auc_event_data);
    netbuf = wlan_rx_event->pst_netbuf;

    if (oal_unlikely(netbuf == NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_process_ring_data_event::netbuf null}");
        return OAL_SUCC;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(event->st_event_hdr.uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(event->st_event_hdr.uc_vap_id, OAM_SF_RX, "{hmac_rx_process_ring_data_event::hmac_vap null.}");
        hmac_rx_free_netbuf(netbuf, wlan_rx_event->us_netbuf_num);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cb = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    if (hmac_sniffer_device_rx_proc(hmac_vap, netbuf, cb) != OAL_CONTINUE) {
        hmac_rx_netbuf_list_free(netbuf);
        return OAL_SUCC;
    }
#endif
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_get_rx_cb_ta_user_id(cb));
    if (hmac_user == NULL) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_rx_process_ring_data_event::hmac_user null}");
        hmac_rx_netbuf_list_free(netbuf);
        return OAL_SUCC;
    }

    if (hmac_device_rx_amsdu_proc(cb, hmac_user, &netbuf) != OAL_CONTINUE) {
        return OAL_SUCC;
    }

    hmac_dev_build_rx_context(netbuf, &rx_context);
    hmac_dev_rx_unique_proc(&rx_context);

#ifdef _PRE_WLAN_FEATURE_VSP
    if (hmac_vsp_rx_proc(&rx_context, netbuf) == VSP_RX_ACCEPT) { /* vsp帧不采用上报协议栈方式，单独处理 */
        return OAL_SUCC;
    }
#endif

    hmac_dev_rx_process_data_proc(&rx_context);
    return OAL_SUCC;
}
/*
 * 函 数 名   : hmac_user_pn_get
 * 功能描述   : 获取用户的PN信息
 * 1.日    期   : 2020-05-25
 *   修改内容   : 生成函数
 */
uint64_t hmac_user_pn_get(hmac_last_pn_stru *last_pn_info, mac_rx_ctl_stru *rx_ctl, uint8_t qos)
{
    uint64_t last_pn_val;

    if (qos == OAL_TRUE) {
        /* qos时，组播和单播也分开检查pn，marvell AP发的组播和单播qos帧使用不同的pn号 */
        if (mac_rx_cb_is_multicast(rx_ctl) == OAL_TRUE) {
            last_pn_val = ((uint64_t)last_pn_info->mcast_qos_last_msb_pn[rx_ctl->bit_rx_tid] << BIT_OFFSET_32) |
                (last_pn_info->mcast_qos_last_lsb_pn[rx_ctl->bit_rx_tid]);
        } else {
            last_pn_val = ((uint64_t)last_pn_info->ucast_qos_last_msb_pn[rx_ctl->bit_rx_tid] << BIT_OFFSET_32) |
                (last_pn_info->ucast_qos_last_lsb_pn[rx_ctl->bit_rx_tid]);
        }
    } else {
        /* 非qos时，组播和单播使用的pn号不一样 */
        if (mac_rx_cb_is_multicast(rx_ctl) == OAL_TRUE) {
            last_pn_val = ((uint64_t)last_pn_info->mcast_nonqos_last_msb_pn << BIT_OFFSET_32) |
                (last_pn_info->mcast_nonqos_last_lsb_pn);
        } else {
            last_pn_val = ((uint64_t)last_pn_info->ucast_nonqos_last_msb_pn << BIT_OFFSET_32) |
                (last_pn_info->ucast_nonqos_last_lsb_pn);
        }
    }
    return last_pn_val;
}

/*
 * 函 数 名   : hmac_user_pn_update
 * 功能描述   : 更新用户的PN信息
 * 1.日    期   : 2020-05-25
 *   修改内容   : 生成函数
 */
OAL_STATIC void hmac_user_pn_update(hmac_last_pn_stru *hmac_last_pn, mac_rx_ctl_stru *rx_ctl, uint8_t qos)
{
    if (qos == OAL_TRUE) {
        if (mac_rx_cb_is_multicast(rx_ctl) == OAL_TRUE) {
            hmac_last_pn->mcast_qos_last_msb_pn[rx_ctl->bit_rx_tid] = rx_ctl->us_rx_msb_pn;
            hmac_last_pn->mcast_qos_last_lsb_pn[rx_ctl->bit_rx_tid] = rx_ctl->rx_lsb_pn;
        }
            hmac_last_pn->ucast_qos_last_msb_pn[rx_ctl->bit_rx_tid] = rx_ctl->us_rx_msb_pn;
            hmac_last_pn->ucast_qos_last_lsb_pn[rx_ctl->bit_rx_tid] = rx_ctl->rx_lsb_pn;
    } else {
        if (mac_rx_cb_is_multicast(rx_ctl) == OAL_TRUE) {
            hmac_last_pn->mcast_nonqos_last_msb_pn = rx_ctl->us_rx_msb_pn;
            hmac_last_pn->mcast_nonqos_last_lsb_pn = rx_ctl->rx_lsb_pn;
        } else {
            hmac_last_pn->ucast_nonqos_last_msb_pn = rx_ctl->us_rx_msb_pn;
            hmac_last_pn->ucast_nonqos_last_lsb_pn = rx_ctl->rx_lsb_pn;
        }
    }
}

/*
* PN窗口:
*  1)新报文的PN如果在窗口外，强制移"窗"，同时丢掉新报文;
*  2)新报文的PN如果在(last_PN-256 ~ last_PN)之间，认为PN检查失败，丢掉新报文.
*/
#define HMAC_PN_WINDOW_SIZE 256
static oal_bool_enum_uint8 hmac_need_pn_windod_shift(uint64_t new, uint64_t cur)
{
    return (oal_bool_enum_uint8)((new + HMAC_PN_WINDOW_SIZE) < cur);
}

static uint32_t hmac_rx_pn_param_check(oal_netbuf_stru *netbuf,
    mac_rx_ctl_stru *rx_ctl, mac_header_frame_control_stru *frame_control)
{
    if ((frame_control->bit_protected_frame == OAL_FALSE) || (frame_control->bit_retry == OAL_TRUE)) {
        return OAL_FAIL;
    }

    /* 1）如果是BA缓存区超时上报的报文，则不进行检查 */
    /* 2）如果当前帧比之前已经上报最后一帧的seq num小，则不进行检查 */
    if (mac_get_rx_cb_is_reo_timeout(rx_ctl) == OAL_TRUE || rx_ctl->is_before_last_release == OAL_TRUE) {
        oam_info_log3(0, OAM_SF_RX, "{hmac_rx_pn_param_check::SEQ NUM[%d] reo_timeout[%d] before_last_release[%d].}",
            rx_ctl->us_seq_num, mac_get_rx_cb_is_reo_timeout(rx_ctl), rx_ctl->is_before_last_release);
        return OAL_FAIL;
    }

    /* AMSDU聚合包的pn号一样，故只检查首包  */
    if (mac_rxcb_is_amsdu_sub_msdu(rx_ctl)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/* 检查不同加密场景PN是否合法 */
OAL_STATIC uint32_t hmac_rx_pn_is_invalid(uint64_t rx_pn_val, uint64_t last_pn_val, oal_bool_enum_uint8 mcast,
    wlan_ciper_protocol_type_enum_uint8 cipher_type)
{
    if (cipher_type == WLAN_80211_CIPHER_SUITE_WAPI) {
        if (wlan_chip_is_support_hw_wapi() == OAL_FALSE) {
            /* 软件WAPI加解密不在此处做PN检查 */
            return OAL_FALSE;
        } else {
            /* WAPI硬件加密PN检查.当前默认STA模式
             * 接收单播帧，PN需要严格单调递增且为奇数
             * 接收组播帧，PN需要严格单调递增
             */
            if ((last_pn_val >= rx_pn_val) && (last_pn_val != 0)) {
                return OAL_TRUE;
            } else {
                return (mcast == OAL_FALSE) ? ((rx_pn_val & 0x1) == 0x0) : OAL_FALSE;
            }
        }
    } else {
        return (last_pn_val >= rx_pn_val) && (last_pn_val != 0);
    }
}

/*
 * 功能描述   : 检查上报协议栈的报文pn号
 * 1.日    期   : 2020-05-25
 *   修改内容   : 生成函数
 */
uint32_t hmac_rx_pn_check(oal_netbuf_stru *netbuf)
{
    uint64_t                       last_pn_val, rx_pn_val;
    uint8_t                        qos_flg = OAL_FALSE;
    hmac_user_stru                *pst_user = NULL;
    mac_rx_ctl_stru               *rx_ctl = NULL;
    mac_header_frame_control_stru *frame_control = NULL;

    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    frame_control = (mac_header_frame_control_stru *)&rx_ctl->us_frame_control;

    if (hmac_rx_pn_param_check(netbuf, rx_ctl, frame_control) != OAL_SUCC) {
        return OAL_SUCC;
    }

    if ((frame_control->bit_type == WLAN_DATA_BASICTYPE) && (frame_control->bit_sub_type >= WLAN_QOS_DATA) &&
        (frame_control->bit_sub_type <= WLAN_QOS_NULL_FRAME)) {
        qos_flg = OAL_TRUE;
    }
    /* TID 合法性检查 */
    if (oal_unlikely((qos_flg == OAL_TRUE) && (rx_ctl->bit_rx_tid >= WLAN_TID_MAX_NUM))) {
        oam_warning_log3(0, OAM_SF_RX, "{hmac_rx_pn_check::uc_qos_flg %d user id[%d] tid[%d].}",
                         qos_flg, mac_get_rx_cb_ta_user_id(rx_ctl), rx_ctl->bit_rx_tid);
        return OAL_SUCC;
    }

    pst_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_get_rx_cb_ta_user_id(rx_ctl));
    if (pst_user == NULL) {
        oam_warning_log1(0, OAM_SF_RX, "{hmac_rx_pn_check::hmac_user[%d] null.}", mac_get_rx_cb_ta_user_id(rx_ctl));
        return OAL_SUCC;
    }

    rx_pn_val = ((uint64_t)rx_ctl->us_rx_msb_pn << BIT_OFFSET_32) | (rx_ctl->rx_lsb_pn);
    last_pn_val = hmac_user_pn_get(&pst_user->last_pn_info, rx_ctl, qos_flg);
    /* 首包pn为0，此处对首包不检查 */
    if (hmac_rx_pn_is_invalid(rx_pn_val, last_pn_val, rx_ctl->bit_mcast_bcast,
        pst_user->st_user_base_info.st_key_info.en_cipher_type) == OAL_TRUE) {
        oam_warning_log4(0, OAM_SF_RX, "{hmac_rx_pn_check::last pn[%ld] send pn[%ld] user id[%d] tid[%d].}",
                         last_pn_val, rx_pn_val, mac_get_rx_cb_ta_user_id(rx_ctl), rx_ctl->bit_rx_tid);
        oam_warning_log4(0, OAM_SF_RX, "{hmac_rx_pn_check:: en_qos[%d] seq_num[%d] is_mcast[%d], sub_type[%d].}",
                         qos_flg, rx_ctl->us_seq_num, mac_rx_cb_is_multicast(rx_ctl), frame_control->bit_sub_type);
        if (hmac_need_pn_windod_shift(rx_pn_val, last_pn_val)) {
            hmac_user_pn_update(&pst_user->last_pn_info, rx_ctl, qos_flg);
        }
        /* 认证模式，不做PN号检查 */
        if (g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_FEATURE_CERTIFY_MODE)) {
            return OAL_SUCC;
        }
        return OAL_FAIL;
    }

    hmac_user_pn_update(&pst_user->last_pn_info, rx_ctl, qos_flg);
    return OAL_SUCC;
}
void hmac_rx_data_sta_proc(hmac_vap_stru *hmac_vap, oal_netbuf_head_stru *netbuf_header)
{
    oal_netbuf_stru *netbuf = NULL;
    mac_rx_ctl_stru *rx_ctl = NULL;

    while (!oal_netbuf_list_empty(netbuf_header)) {
        netbuf = oal_netbuf_delist(netbuf_header);
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        if (rx_ctl->bit_mcast_bcast == OAL_FALSE) {
            oal_atomic_inc(&(hmac_vap->st_hmac_arp_probe.rx_unicast_pkt_to_lan));
        }
        hmac_rx_enqueue(netbuf, hmac_vap);
    }

    if (OAL_TRUE == hmac_get_rxthread_enable()) {
        g_hisi_softwdt_check.rxshed_sta++;
        hmac_rxdata_sched();
    }

    return;
}
OAL_STATIC uint8_t hmac_rx_is_vap_status_valid(mac_rx_ctl_stru *rx_ctl, hmac_vap_stru **pphmac_vap)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(rx_ctl->uc_mac_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        return OAL_FALSE;
    }

    if (hmac_vap->pst_net_device == NULL) {
        return OAL_FALSE;
    }

    *pphmac_vap = hmac_vap;
    return OAL_TRUE;
}
/*
 * 功能描述  : rx data aput数据流处理
 * 1.日    期  : 2020年03月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_rx_data_ap_proc(hmac_vap_stru *hmac_vap, oal_netbuf_head_stru *p_netbuf_head)
{
    oal_netbuf_stru          *netbuf = NULL;
    oal_netbuf_stru          *temp_netbuf = NULL;
    mac_rx_ctl_stru          *rx_ctl = NULL;
    mac_ieee80211_frame_stru *copy_frame_hdr = NULL;
    uint16_t                  netbuf_num;
    oal_netbuf_stru          *netbuf_copy = NULL;
    oal_netbuf_head_stru      w2w_netbuf_hdr;
    hmac_vap_stru            *hmac_vap_temp = NULL;

    oal_netbuf_list_head_init(&w2w_netbuf_hdr);
    temp_netbuf = oal_netbuf_peek(p_netbuf_head);
    netbuf_num = (uint16_t)oal_netbuf_get_buf_num(p_netbuf_head);
    while (netbuf_num != 0) {
        netbuf = temp_netbuf;
        if (netbuf == NULL) {
            break;
        }

        oal_netbuf_get_appointed_netbuf(netbuf, 1, &temp_netbuf);
        netbuf_num = oal_sub(netbuf_num, 1);
        rx_ctl     = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        if (OAL_TRUE != hmac_rx_is_vap_status_valid(rx_ctl, &hmac_vap_temp)) {
            hmac_rx_free_netbuf(netbuf, (uint16_t)1);
            continue;
        }

        if ((OAL_TRUE == ether_is_multicast(hmac_rx_get_eth_da(netbuf)))) {
            if (OAL_SUCC != hmac_rx_copy_netbuff(&netbuf_copy, netbuf, rx_ctl->uc_mac_vap_id, &copy_frame_hdr)) {
                hmac_rx_free_netbuf(netbuf, (uint16_t)1);
                continue;
            }

            hmac_rx_enqueue(netbuf, hmac_vap_temp);
            hmac_rx_prepare_to_wlan(&w2w_netbuf_hdr, netbuf_copy);
            continue;
        }

        /* check if da belong to the same bss */
        if (hmac_rx_da_is_in_same_bss(netbuf, rx_ctl, hmac_vap_temp) == OAL_TRUE) {
            /* 目的用户已在AP的用户表中，进行WLAN_TO_WLAN转发 */
            hmac_rx_prepare_to_wlan(&w2w_netbuf_hdr, netbuf);
        } else {
            hmac_rx_enqueue(netbuf, hmac_vap_temp);
        }
    }

    /*  将MSDU链表交给发送流程处理 */
    hmac_rx_wlan_to_wlan_proc(hmac_vap, &w2w_netbuf_hdr);
    /* sch frames to kernel */
    if (OAL_TRUE == hmac_get_rxthread_enable()) {
        g_hisi_softwdt_check.rxshed_ap++;
        hmac_rxdata_sched();
    }
}

