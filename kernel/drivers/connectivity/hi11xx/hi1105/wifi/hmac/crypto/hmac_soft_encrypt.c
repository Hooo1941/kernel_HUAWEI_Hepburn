/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 驱动加密相关函数实现
 * 作    者 :
 * 创建日期 : 2022年8月30日
 */
#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))

#include "hmac_soft_encrypt.h"
#include "hmac_user.h"
#include "hmac_tx_data.h"
#include "oam_stat_wifi.h"
#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
#include "hmac_pwl.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SOFT_ENCRYPT_C

/* 保存待加密的netbuf链表 */
hmac_encrypt_schedule_stru g_tx_encrypt_netbuf_list;

hmac_encrypt_schedule_stru *hmac_get_encrypt_schedule_list(void)
{
    return &g_tx_encrypt_netbuf_list;
}

OAL_STATIC hmac_user_stru *hmac_soft_encrypt_get_hmac_user(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf)
{
    hmac_user_stru *hmac_user = NULL;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *mac_hdr = mac_get_cb_frame_header_addr(tx_ctl);
    uint16_t user_idx = g_wlan_spec_cfg->invalid_user_id;

    if (ether_is_multicast(mac_hdr->auc_address1)) {
        user_idx = mac_vap->us_multi_user_idx;
    } else {
        if (mac_vap_find_user_by_macaddr(mac_vap, mac_hdr->auc_address1, &user_idx) != OAL_SUCC) {
            /* 删除user后，过几十ms才关发送队列，在此期间会刷几十条日志，故降级为warning */
            oam_warning_log0(0, OAM_SF_ANY, "hmac_soft_encrypt_get_hmac_user:find user fail");
            return NULL;
        }
    }
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(user_idx);
    if (hmac_user == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_soft_encrypt_get_hmac_user:user_idx=%d", user_idx);
        return NULL;
    }
    return hmac_user;
}

oal_bool_enum_uint8 hmac_check_soft_encrypt_port(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf)
{
    hmac_user_stru *hmac_user = hmac_soft_encrypt_get_hmac_user(mac_vap, netbuf);
    if (hmac_user == NULL) {
        return OAL_FALSE;
    }
#ifdef _PRE_WLAN_FEATURE_WAPI
    if (hmac_user->st_wapi.uc_port_valid == OAL_TRUE) {
        return OAL_TRUE;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
    if ((hmac_user->pwl.port_valid == OAL_TRUE) || (hmac_user->pwl_pre.port_valid == OAL_TRUE)) {
        return OAL_TRUE;
    }
#endif
    return OAL_FALSE;
}

oal_bool_enum_uint8 hmac_check_soft_encrypt_thread(void)
{
    hmac_encrypt_schedule_stru *encrypt_schedule = hmac_get_encrypt_schedule_list();
    if (encrypt_schedule->encrypt_schedule_thread == NULL) {
        oam_error_log0(0, OAM_SF_RX,
                       "{hmac_check_soft_encrypt_thread::encrypt_schedule_thread is NULL }");
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

uint32_t hmac_soft_encrypt_add_list(oal_netbuf_stru *netbuf)
{
    hmac_encrypt_schedule_stru *encrypt_schedule = hmac_get_encrypt_schedule_list();
    oal_netbuf_head_stru *netbuf_header = &encrypt_schedule->netbuf_header;
    if (encrypt_schedule->encrypt_schedule_thread == NULL) {
        oam_error_log0(0, OAM_SF_RX,
                       "{hmac_soft_encrypt_add_list::encrypt_schedule_thread is NULL }");
        return OAL_FAIL;
    }
    oal_netbuf_list_tail(netbuf_header, netbuf);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    up(&encrypt_schedule->encrypt_sche_sema);
#endif
    return OAL_SUCC;
}
/*
 * 功能描述  : tx方向，对netbuf链进行加密(WAPI或PWL返回加密netbuf链，其他的返回原netbuf链)
 * 1.日    期  : 2022年8月30日
  *   修改内容  : 新生成函数
 */
uint32_t hmac_soft_encrypt_netbuf(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf, oal_netbuf_stru **netbuf_new)
{
    oal_netbuf_stru *encrypt_netbuf = NULL;
    hmac_user_stru *hmac_user = NULL;
    *netbuf_new = netbuf;
#ifdef _PRE_WLAN_FEATURE_PWL
    if (mac_vap->soft_encrypt_mode != PWL_HOST_ENCRYPT) {
        return OAL_SUCC;
    }
#endif
    hmac_user = hmac_soft_encrypt_get_hmac_user(mac_vap, netbuf);
    if (hmac_user == NULL) {
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_FEATURE_WAPI
    encrypt_netbuf = hmac_wapi_encrypt_netbuf(hmac_user, netbuf);
    if (encrypt_netbuf == NULL) {
        return OAL_FAIL;
    }
    /* 如果encrypt_netbuf和netbuf指针不相同则说明做过解密释放掉旧的netbuf */
    if (encrypt_netbuf != netbuf) {
        hmac_free_netbuf_list(netbuf);
        *netbuf_new = encrypt_netbuf;
        return OAL_SUCC;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
    encrypt_netbuf = hmac_pwl_encrypt_netbuf(hmac_user, netbuf);
    if (encrypt_netbuf == NULL) {
        return OAL_FAIL;
    }
    /* 如果encrypt_netbuf和netbuf指针不相同则说明做过解密释放掉旧的netbuf */
    if (encrypt_netbuf != netbuf) {
        hmac_free_netbuf_list(netbuf);
        *netbuf_new = encrypt_netbuf;
        return OAL_SUCC;
    }
#endif
    return OAL_SUCC;
}

/*
 * 功能描述  : 软件加密线程处理函数
 * 1.日    期  : 2022年12月1日
  *   修改内容  : 新生成函数
 */
void hmac_tx_soft_encrypt_schedule(void)
{
    mac_tx_ctl_stru *tx_ctl = NULL;
    mac_ieee80211_frame_stru *mac_hdr = NULL;
    oal_netbuf_stru *netbuf = NULL;
    oal_netbuf_stru *buf_new = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_vap_stru *mac_vap = NULL;
    hmac_encrypt_schedule_stru *encrypt_schedule = hmac_get_encrypt_schedule_list();
    oal_netbuf_head_stru *netbuf_header = &encrypt_schedule->netbuf_header;

    while (!oal_netbuf_list_empty(netbuf_header)) {
        netbuf = oal_netbuf_delist(netbuf_header);
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        mac_hdr = mac_get_cb_frame_header_addr(tx_ctl);
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_get_cb_tx_vap_index(tx_ctl));
        if (hmac_vap == NULL) {
            oam_error_log1(0, OAM_SF_RX,
                "{hmac_tx_soft_encrypt_schedule::hmac_vap null. vap_id:%u}", mac_get_cb_tx_vap_index(tx_ctl));
            continue;
        }
        mac_vap = &hmac_vap->st_vap_base_info;
#ifdef _PRE_WLAN_FEATURE_PWL
        oal_spin_lock_bh(&mac_vap->tx_pn_lock);
#endif
        if (hmac_soft_encrypt_netbuf(mac_vap, netbuf, &buf_new) != OAL_SUCC) {
            hmac_free_netbuf_list(netbuf); // 加密线程中，如果netbuf加密失败，需要将该netbuf释放
            oam_stat_vap_incr(mac_vap->uc_vap_id, tx_abnormal_msdu_dropped, 1);
#ifdef _PRE_WLAN_FEATURE_PWL
            oal_spin_unlock_bh(&mac_vap->tx_pn_lock);
#endif
            continue; // 函数内部有error级别维测，此处不需要加维测。
        }
#ifdef _PRE_WLAN_FEATURE_PWL
        oal_spin_unlock_bh(&mac_vap->tx_pn_lock);
#endif
        netbuf = buf_new;
        /*  由于wapi、pwl可能修改netbuff，此处需要重新获取一下cb */
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        if (hmac_tx_lan_to_wlan_no_tcp_opt_tx_pass_handle(mac_vap, netbuf) != OAL_SUCC) {
            hmac_free_netbuf_list(netbuf); // buf_new的释放
            continue;
        }
    }
}

int32_t hmac_soft_encrypt_schedule_thread(void *data)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct sched_param param = { 0 };

    param.sched_priority = 97; /* 调度优先级97 */
    oal_set_thread_property(current, SCHED_FIFO, &param, -10); /* set nice -10 */

    allow_signal(SIGTERM);

    while (oal_likely(!down_interruptible(&g_tx_encrypt_netbuf_list.encrypt_sche_sema))) {
#ifdef _PRE_WINDOWS_SUPPORT
        if (oal_kthread_should_stop((PRT_THREAD)data)) {
#else
        if (oal_kthread_should_stop()) {
#endif
            break;
        }
        // 处理需要加密的netbuf
        hmac_tx_soft_encrypt_schedule();
    }
#endif

    return OAL_SUCC;
}

#endif
