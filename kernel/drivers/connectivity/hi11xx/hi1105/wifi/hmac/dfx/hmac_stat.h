/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI HOST统计接口文件
 * 作    者 :
 * 创建日期 : 2020年7月21日
 */

#ifndef HMAC_STAT_H
#define HMAC_STAT_H

#include "hmac_stat_stru.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#include "hmac_chba_function.h"
#endif

struct hmac_device_tag;

void hmac_stat_init_device_stat(struct hmac_device_tag *hmac_device);
void hmac_stat_init_device_screen_stat(struct hmac_device_tag *hmac_device);

extern hmac_device_stat_stru g_hmac_device_stats;
extern hmac_lp_miracast_stat_stru g_hmac_lp_miracast_stats;

OAL_STATIC OAL_INLINE hmac_device_stat_stru *hmac_stat_get_device_stats(void)
{
    return &g_hmac_device_stats;
}

OAL_STATIC OAL_INLINE hmac_lp_miracast_stat_stru *hmac_stat_get_lp_miracast_stats(void)
{
    return &g_hmac_lp_miracast_stats;
}

OAL_STATIC OAL_INLINE hmac_vap_stat_stru *hmac_stat_get_vap_stat(hmac_vap_stru *hmac_vap)
{
    return &(hmac_vap->vap_stats);
}

OAL_STATIC OAL_INLINE hmac_user_stat_stru *hmac_stat_get_user_stat(hmac_user_stru *hmac_user)
{
    return &(hmac_user->user_stats);
}

OAL_STATIC OAL_INLINE void hmac_stat_device_tx_msdu(uint32_t msdu_len)
{
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();
    oal_atomic_add(&(hmac_device_stats->tx_packets), 1);
    oal_atomic_add(&(hmac_device_stats->tx_bytes), msdu_len);
}

OAL_STATIC OAL_INLINE void hmac_stat_device_rx_msdu(uint32_t msdu_len)
{
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();
    oal_atomic_add(&(hmac_device_stats->rx_packets), 1);
    oal_atomic_add(&(hmac_device_stats->rx_bytes), msdu_len);
}
OAL_STATIC OAL_INLINE void hmac_rx_checksum_stat(mac_rx_ctl_stru *rx_ctrl)
{
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();
    if (rx_ctrl->bit_ptlcs_valid) {
        if (rx_ctrl->bit_ptlcs_pass) {
            oal_atomic_add(&(hmac_device_stats->rx_tcp_checksum_pass), 1);
        } else {
            oal_atomic_add(&(hmac_device_stats->rx_tcp_checksum_fail), 1);
        }
    }
    if (rx_ctrl->bit_ipv4cs_valid) {
        if (rx_ctrl->bit_ipv4cs_pass) {
            oal_atomic_add(&(hmac_device_stats->rx_ip_checksum_pass), 1);
        } else {
            oal_atomic_add(&(hmac_device_stats->rx_ip_checksum_fail), 1);
        }
    }
}

OAL_STATIC OAL_INLINE void hmac_chr_get_rx_checksum_stat(uint32_t *rx_tcp_checksum_fail, uint32_t *rx_ip_checksum_fail)
{
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();

    *rx_tcp_checksum_fail = (uint32_t)oal_atomic_read(&hmac_device_stats->rx_tcp_checksum_fail);
    *rx_ip_checksum_fail = (uint32_t)oal_atomic_read(&hmac_device_stats->rx_ip_checksum_fail);
}
/*
 * 更新接收netbuffer统计信息
 */
OAL_STATIC OAL_INLINE void hmac_stat_vap_rx_netbuf(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    hmac_vap_stat_stru *hmac_vap_stat = NULL;

    if ((hmac_vap == NULL) || (netbuf == NULL)) {
        return;
    }

    hmac_vap_stat = hmac_stat_get_vap_stat(hmac_vap);

    /* 数据帧统计 */
    HMAC_VAP_STATS_PKT_INCR(hmac_vap_stat->rx_packets, 1);
    HMAC_VAP_STATS_PKT_INCR(hmac_vap_stat->rx_bytes, oal_netbuf_get_len(netbuf));
}

/*
 * 更新发送netbuffer统计信息
 */
OAL_STATIC OAL_INLINE void hmac_stat_vap_tx_netbuf(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    hmac_vap_stat_stru *hmac_vap_stats = NULL;

    if ((hmac_vap == NULL) || (netbuf == NULL)) {
        return;
    }

    hmac_vap_stats = hmac_stat_get_vap_stat(hmac_vap);

    /* 数据帧统计 */
    HMAC_VAP_STATS_PKT_INCR(hmac_vap_stats->tx_packets, 1);
    HMAC_VAP_STATS_PKT_INCR(hmac_vap_stats->tx_bytes, oal_netbuf_get_len(netbuf));
}

#ifdef _PRE_WLAN_CHBA_MGMT
OAL_STATIC OAL_INLINE void hmac_stat_user_rx_netbuf(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    hmac_user_stat_stru *hmac_user_stats = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    if ((hmac_user == NULL) || (netbuf == NULL)) {
        return;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->st_user_base_info.uc_vap_id);
    if (!hmac_chba_vap_start_check(hmac_vap)) {
        return;
    }
    hmac_user_stats = hmac_stat_get_user_stat(hmac_user);

    /* 数据帧统计 */
    HMAC_USER_STATS_PKT_INCR(hmac_user_stats->rx_packets, 1);
    HMAC_USER_STATS_PKT_INCR(hmac_user_stats->rx_bytes, oal_netbuf_get_len(netbuf));
}

OAL_STATIC OAL_INLINE void hmac_stat_user_tx_netbuf(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    hmac_user_stat_stru *hmac_user_stats = hmac_stat_get_user_stat(hmac_user);

    /* 数据帧统计 */
    HMAC_USER_STATS_PKT_INCR(hmac_user_stats->tx_packets, 1);
    HMAC_USER_STATS_PKT_INCR(hmac_user_stats->tx_bytes, oal_netbuf_get_len(netbuf));
}
#endif
#endif /* HMAC_STAT_H */

