/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : tcp ack 缓存功能
* 作者       : wifi
* 创建日期   : 2020年07月22日
*/
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER

/* 1 头文件包含 */
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tcp_ack_buf.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "wlan_chip_i.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_ACK_BUF_C

/* 2 全局变量定义 */
OAL_STATIC hmac_tcp_ack_buf_para_stru g_tcp_ack_buf_para;
OAL_STATIC hmac_tcp_ack_buf_stru g_tcp_ack_buf;
mac_tcp_ack_buf_switch_stru g_st_tcp_ack_buf_switch = { 0 };

#ifdef _PRE_WLAN_TCP_OPT
mac_tcp_ack_filter_stru g_st_tcp_ack_filter = { 0 };
#endif

mac_tcp_ack_buf_switch_stru *mac_vap_get_tcp_ack_buf_switch(void)
{
    return &g_st_tcp_ack_buf_switch;
}
/*
 * 功能描述  : mp13 mp15 ack缓存参数抛到device
 * 1.日    期  : 2020年7月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tcp_ack_buff_config_mp13(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_send_event(mac_vap, WLAN_CFGID_TCP_ACK_BUF, len, param);
}

/*
 * 功能描述  : 06 host配置ack缓存参数
 * 1.日    期  : 2020年7月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_tcp_ack_buff_config_mp16(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_cfg_tcp_ack_buf_stru *tcp_ack_param = (mac_cfg_tcp_ack_buf_stru *)param;

    switch (tcp_ack_param->en_cmd) {
        case MAC_TCP_ACK_BUF_ENABLE:
            g_tcp_ack_buf_para.tcp_ack_buf[mac_vap->uc_vap_id] = tcp_ack_param->en_enable;
            break;
        case MAC_TCP_ACK_BUF_TIMEOUT:
            g_tcp_ack_buf_para.tcp_ack_timeout = tcp_ack_param->uc_timeout_ms;
            break;
        case MAC_TCP_ACK_BUF_MAX:
            g_tcp_ack_buf_para.tcp_ack_max_num = tcp_ack_param->uc_count_limit;
            break;
        default:
            break;
    }

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{hmac_set_tcp_ack_buf::en_tcp_ack_buf=%d, uc_tcp_ack_timeout=%d, us_tcp_ack_max_num=%d}",
                     g_tcp_ack_buf_para.tcp_ack_buf[mac_vap->uc_vap_id],
                     g_tcp_ack_buf_para.tcp_ack_timeout,
                     g_tcp_ack_buf_para.tcp_ack_max_num);
    return OAL_SUCC;
}

uint32_t hmac_config_tcp_ack_buf(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return wlan_chip_config_tcp_ack_buf(mac_vap, len, param);
}

/*
 * 功能描述  : host释放ack缓存的skb
 * 1.日    期  : 2020年7月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_tcp_ack_buf_free(hmac_tcp_ack_buf_stru *tcp_ack_buf)
{
    oal_netbuf_stru *netbuf = NULL;
    oal_spin_lock_bh(&tcp_ack_buf->spin_lock);
    /* 清空tcp ack缓存描述符 */
    while (oal_netbuf_list_empty(&tcp_ack_buf->hdr) != OAL_TRUE) {
        netbuf = oal_netbuf_delist(&(tcp_ack_buf->hdr));
        oal_netbuf_free(netbuf);
    }

    tcp_ack_buf->tcp_ack_num = 0;
    oal_spin_unlock_bh(&tcp_ack_buf->spin_lock);
}
/*
 * 功能描述  : host ack初始化
 * 1.日    期  : 2020年7月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_tcp_ack_buf_init_para()
{
    hmac_tcp_ack_buf_para_stru *tcp_ack_buf = &g_tcp_ack_buf_para;

    memset_s(tcp_ack_buf, sizeof(hmac_tcp_ack_buf_para_stru), 0, sizeof(hmac_tcp_ack_buf_para_stru));

    tcp_ack_buf->tcp_ack_max_num = TCP_ACK_BUF_MAX_NUM;
    tcp_ack_buf->tcp_ack_timeout = TCP_ACK_TIMEOUT;
}

/*
 * 功能描述  : host初始化ack缓存
 * 1.日    期  : 2020年7月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_tcp_ack_buf_init_user(hmac_user_stru *hmac_user)
{
    hmac_tcp_ack_buf_stru *tcp_ack_buf = &g_tcp_ack_buf;
    mac_chip_stru *mac_chip = NULL;
    /* multi user不申请 */
    if (hmac_user->st_user_base_info.en_is_multi_user == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tcp_ack_buf_init_user::is_multi_user}");
        return;
    }
    mac_chip = hmac_res_get_mac_chip(hmac_user->st_user_base_info.uc_chip_id);
    if (mac_chip == NULL) {
        return;
    }
    /* 非单用户不申请 */
    if (mac_chip->uc_assoc_user_cnt != 0) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tcp_ack_buf_init_user::uc_assoc_user_cnt != 0}");
        return;
    }
    hmac_user->tcp_ack_buf = tcp_ack_buf;
    memset_s(tcp_ack_buf, sizeof(hmac_tcp_ack_buf_stru), 0, sizeof(hmac_tcp_ack_buf_stru));
    oal_spin_lock_init(&tcp_ack_buf->spin_lock);
    oal_netbuf_list_head_init(&tcp_ack_buf->hdr);
}
/*
* 功能描述  : 用户离开时，删除tcp ack缓存特性的相关信息
* 1.日    期  : 2020年7月22日
*   作    者  : wifi
*   修改内容  : 新生成函数
*/
void hmac_del_user_reset_tcp_ack_buf(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_tcp_ack_buf_stru *tcp_ack_buf = (hmac_tcp_ack_buf_stru *)hmac_user->tcp_ack_buf;
    /* 用户层可能修改tcp ack缓存机制，去关联时重置 */
    hmac_vap->tcp_ack_buf_use_ctl_switch = OAL_SWITCH_OFF;
    if (tcp_ack_buf != NULL) {
        // 删除定时器
        if (tcp_ack_buf->tcp_ack_timer.en_is_registerd == OAL_TRUE) {
            frw_timer_immediate_destroy_timer_m(&(tcp_ack_buf->tcp_ack_timer));
        }

        /* 清空tcp ack缓存链表 */
        hmac_tcp_ack_buf_free(tcp_ack_buf);

        memset_s(tcp_ack_buf, sizeof(hmac_tcp_ack_buf_stru), 0, sizeof(hmac_tcp_ack_buf_stru));

        hmac_user->tcp_ack_buf = NULL;
    }
}
/*
* 功能描述  : 定时器到期处理函数
* 1.日    期  : 2020年7月22日
*   作    者  : wifi
*   修改内容  : 新生成函数
*/
uint32_t hmac_tx_tcp_ack_buf_send(void *p_arg)
{
    hmac_tcp_ack_buf_stru *tcp_ack_buf = NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)p_arg;
    oal_netbuf_stru *netbuf = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    oal_netbuf_head_stru st_head_t;
    uint32_t netbuf_num = 0;

    tcp_ack_buf = (hmac_tcp_ack_buf_stru *)hmac_user->tcp_ack_buf;
    if (tcp_ack_buf == NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_tcp_ack_buf_send::tcp_ack_buf is NULL }");
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap = tcp_ack_buf->hmac_vap;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_tcp_ack_buf_send::hmac_vap is NULL }");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 自旋锁 */
    oal_spin_lock_bh(&tcp_ack_buf->spin_lock);
    oal_netbuf_head_init(&st_head_t);
    oal_netbuf_queue_splice_tail_init(&tcp_ack_buf->hdr, &st_head_t);
    tcp_ack_buf->tcp_ack_num = 0;
    oal_spin_unlock_bh(&tcp_ack_buf->spin_lock);
    while (oal_netbuf_list_empty(&st_head_t) != OAL_TRUE) {
        netbuf = oal_netbuf_delist_nolock(&(st_head_t));
        if (oal_unlikely(netbuf == NULL)) {
            return OAL_SUCC;
        }
        if (hmac_host_tx_data(hmac_vap, hmac_user, netbuf) != OAL_SUCC) {
            oal_netbuf_free(netbuf);
            oam_error_log1(0, OAM_SF_TX, "{hmac_tx_tcp_ack_buf_send::netbuf_num=%d }", netbuf_num);
            continue;
        }
        netbuf_num++;
    }
    return OAL_SUCC;
}

/*
* 功能描述  : tcp ack缓存处理函数，只处理单帧(非amsdu,非ampdu)的情况
* 1.日    期  : 2020年7月22日
*   作    者  : wifi
*   修改内容  : 新生成函数
*/
uint32_t hmac_tx_tcp_ack_buf_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    hmac_tcp_ack_buf_stru *tcp_ack_buf = NULL;
    hmac_tcp_ack_buf_para_stru *tcp_ack_buf_para = &g_tcp_ack_buf_para;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

    if (tcp_ack_buf_para->tcp_ack_buf[hmac_vap->st_vap_base_info.uc_vap_id] == OAL_FALSE) {
        return HMAC_TX_PASS;
    }

    tcp_ack_buf = (hmac_tcp_ack_buf_stru *)hmac_user->tcp_ack_buf;
    if (tcp_ack_buf == NULL) {
        return HMAC_TX_PASS;
    }

    if (mac_get_cb_is_tcp_ack(tx_ctl) != OAL_TRUE) {
        return HMAC_TX_PASS;
    }

    if (tcp_ack_buf->tcp_ack_num == 0) {
        /* 启动定时器 */
        tcp_ack_buf->hmac_vap = hmac_vap;
        frw_timer_create_timer_m(&tcp_ack_buf->tcp_ack_timer, hmac_tx_tcp_ack_buf_send,
            tcp_ack_buf_para->tcp_ack_timeout, hmac_user, OAL_TRUE, OAM_MODULE_ID_HMAC,
            hmac_vap->st_vap_base_info.core_id);
    }

    // 直接缓存
    oal_spin_lock_bh(&tcp_ack_buf->spin_lock);
    oal_netbuf_list_tail_nolock(&tcp_ack_buf->hdr, netbuf);
    tcp_ack_buf->tcp_ack_num++;
    oal_spin_unlock_bh(&tcp_ack_buf->spin_lock);
    oam_info_log2(0, OAM_SF_TX, "{hmac_tx_tcp_ack_buf_process::tcp_ack_num=%d, tcp_ack_max_num=%d }",
                  tcp_ack_buf->tcp_ack_num, tcp_ack_buf_para->tcp_ack_max_num);
    // 若已缓存的报文达到了上限，就发送
    if (tcp_ack_buf->tcp_ack_num >= tcp_ack_buf_para->tcp_ack_max_num) {
        /* 立即删除定时器 */
        tcp_ack_buf->hmac_vap = hmac_vap;
        frw_timer_immediate_destroy_timer_m(&tcp_ack_buf->tcp_ack_timer);
        hmac_tx_tcp_ack_buf_send(hmac_user);
    }
    return HMAC_TX_BUFF;
}
/*
 * 函 数 名  : hmac_tcp_ack_buf_set_throughput_threshould
 * 功能描述  : 设置每秒吞吐量门限
 * 1.日    期  : 2020年06月09日
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_tcp_ack_buf_set_throughput_threshould(uint32_t high_throughput_level,
    uint32_t *ul_limit_throughput_high, uint32_t *ul_limit_throughput_low, oal_bool_enum_uint8  en_mu_vap)
{
    mac_tcp_ack_buf_switch_stru *tcp_ack_buf_switch = mac_vap_get_tcp_ack_buf_switch();
    switch (high_throughput_level) {
        case WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH:
            *ul_limit_throughput_high = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH >> en_mu_vap;
            *ul_limit_throughput_low = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW >> en_mu_vap;
            if ((tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high != 0) &&
                (tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low != 0)) {
                *ul_limit_throughput_high = tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high >> en_mu_vap;
                *ul_limit_throughput_low = tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low >> en_mu_vap;
            }
            break;
        case WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_40M:
            *ul_limit_throughput_high = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_40M >> en_mu_vap;
            *ul_limit_throughput_low = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_40M >> en_mu_vap;
            if ((tcp_ack_buf_switch->tcp_ack_buf_throughput_high_40m != 0) &&
                (tcp_ack_buf_switch->tcp_ack_buf_throughput_low_40m != 0)) {
                *ul_limit_throughput_high = tcp_ack_buf_switch->tcp_ack_buf_throughput_high_40m >> en_mu_vap;
                *ul_limit_throughput_low = tcp_ack_buf_switch->tcp_ack_buf_throughput_low_40m >> en_mu_vap;
            }
            break;
        case WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_80M:
            *ul_limit_throughput_high = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_80M >> en_mu_vap;
            *ul_limit_throughput_low = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_80M >> en_mu_vap;
            if ((tcp_ack_buf_switch->tcp_ack_buf_throughput_high_80m != 0) &&
                (tcp_ack_buf_switch->tcp_ack_buf_throughput_low_80m != 0)) {
                *ul_limit_throughput_high = tcp_ack_buf_switch->tcp_ack_buf_throughput_high_80m >> en_mu_vap;
                *ul_limit_throughput_low = tcp_ack_buf_switch->tcp_ack_buf_throughput_low_80m >> en_mu_vap;
            }
            break;
#ifdef _PRE_WLAN_FEATURE_160M
        case WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_160M:
            *ul_limit_throughput_high = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_160M >> en_mu_vap;
            *ul_limit_throughput_low = WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_LOW_160M >> en_mu_vap;
            if ((tcp_ack_buf_switch->tcp_ack_buf_throughput_high_160m != 0) &&
                (tcp_ack_buf_switch->tcp_ack_buf_throughput_low_160m != 0)) {
                *ul_limit_throughput_high = tcp_ack_buf_switch->tcp_ack_buf_throughput_high_160m >> en_mu_vap;
                *ul_limit_throughput_low = tcp_ack_buf_switch->tcp_ack_buf_throughput_low_160m;
            }
            break;
#endif
        default:
            break;
    }
}

/*
 * 函 数 名  : hmac_tcp_ack_buf_switch_judge
 * 功能描述  : 开启或关闭TCP ack缓存功能
 * 1.日    期  : 2020年06月05日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_tcp_ack_buf_switch_judge(mac_vap_stru *pst_mac_vap, uint32_t limit_throughput_high,
    uint32_t limit_throughput_low, uint32_t rx_throughput_mbps, uint8_t  uc_vap_idx)
{
    mac_cfg_tcp_ack_buf_stru st_tcp_ack_param = { 0 };
    oal_bool_enum_uint8 en_tcp_ack_buf;
    mac_tcp_ack_buf_switch_stru *tcp_ack_buf_switch = mac_vap_get_tcp_ack_buf_switch();

    if (tcp_ack_buf_switch->us_tcp_ack_smooth_throughput > limit_throughput_high) {
        /* 高于上门限,打开tcp ack buf */
        en_tcp_ack_buf = OAL_TRUE;
    } else if (tcp_ack_buf_switch->us_tcp_ack_smooth_throughput < limit_throughput_low) {
        /* 低于下门限,关闭tcp ack buf */
        en_tcp_ack_buf = OAL_FALSE;
    } else {
        /* 介于上下门限之间， 不作切换 */
        return;
    }

    /* 未发生变化 ,不处理 */
    if (tcp_ack_buf_switch->uc_cur_tcp_ack_buf_en[uc_vap_idx] == en_tcp_ack_buf) {
        return;
    }

    oam_warning_log4(0, OAM_SF_ANY,
        "{hmac_tx_tcp_ack_buf_switch: limit_high = [%d],limit_low = [%d],rx_throught= [%d]! en_tcp_ack_buf=%d}",
        limit_throughput_high, limit_throughput_low, rx_throughput_mbps, en_tcp_ack_buf);

    tcp_ack_buf_switch->uc_cur_tcp_ack_buf_en[uc_vap_idx] = en_tcp_ack_buf;

    st_tcp_ack_param.en_cmd = MAC_TCP_ACK_BUF_ENABLE;
    st_tcp_ack_param.en_enable = en_tcp_ack_buf;

    hmac_config_tcp_ack_buf(pst_mac_vap, sizeof(mac_cfg_tcp_ack_buf_stru), (uint8_t *)&st_tcp_ack_param);
}

OAL_STATIC void hmac_set_vap_throughput_threshould_by_bw(mac_vap_stru *mac_vap[], uint32_t *limit_throughput_high,
    uint32_t *limit_throughput_low, uint32_t rx_throughput_mbps, uint8_t vap_num)
{
    uint8_t  vap_idx;
    oal_bool_enum_uint8 en_mu_vap = (vap_num > 1);
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 多个vap up时只支持两个vap打开算法 */
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) && (vap_num > TWO_UP_VAP_NUMS)) {
        vap_num = TWO_UP_VAP_NUMS;
    }
#endif
    for (vap_idx = 0; vap_idx < vap_num; vap_idx++) {
        if (mac_vap[vap_idx]->st_channel.en_bandwidth == WLAN_BAND_WIDTH_20M) {
            /* 设置20M的每秒吞吐量门限 */
            hmac_tcp_ack_buf_set_throughput_threshould(WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH,
                limit_throughput_high, limit_throughput_low, en_mu_vap);
        } else if ((mac_vap[vap_idx]->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ||
                   (mac_vap[vap_idx]->st_channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
            /* 设置40M的每秒吞吐量门限 */
            hmac_tcp_ack_buf_set_throughput_threshould(WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_40M,
                limit_throughput_high, limit_throughput_low, en_mu_vap);
        } else if ((mac_vap[vap_idx]->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
                   (mac_vap[vap_idx]->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
            /* 设置80M的每秒吞吐量门限 */
            hmac_tcp_ack_buf_set_throughput_threshould(WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_80M,
                limit_throughput_high, limit_throughput_low, en_mu_vap);
#ifdef _PRE_WLAN_FEATURE_160M
        } else if ((mac_vap[vap_idx]->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) &&
                   (mac_vap[vap_idx]->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS)) {
            /* 设置160M的每秒吞吐量门限 */
            hmac_tcp_ack_buf_set_throughput_threshould(WLAN_TCP_ACK_BUF_THROUGHPUT_THRESHOLD_HIGH_160M,
                limit_throughput_high, limit_throughput_low, en_mu_vap);
#endif
        }
        hmac_tcp_ack_buf_switch_judge(mac_vap[vap_idx], *limit_throughput_high,
                                      *limit_throughput_low, rx_throughput_mbps, vap_idx);
    }
}

/*
 * 函 数 名  : hmac_tx_tcp_ack_buf_switch
 * 功能描述  : VHT STA模式 并且下行性能大于100M时切换为允许tcp ack buf 功能
 * 1.日    期  : 2018年05月08日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_tx_tcp_ack_buf_switch(uint32_t rx_throughput_mbps)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    mac_vap_stru *mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    hmac_vap_stru *hmac_vap = NULL;
    uint32_t limit_throughput_high = 550;
    uint32_t limit_throughput_low = 450;
    uint8_t  up_vap_num;
    oal_bool_enum_uint8 en_mu_vap;
    mac_tcp_ack_buf_switch_stru *tcp_ack_buf_switch = mac_vap_get_tcp_ack_buf_switch();

    /* 如果定制化开关不支持切换，直接返回 */
    if (tcp_ack_buf_switch->uc_ini_tcp_ack_buf_en == OAL_FALSE) {
        return;
    }

    up_vap_num = mac_device_find_up_vaps(mac_device, mac_vap, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    en_mu_vap = (up_vap_num > 1);
    if ((up_vap_num == 0) || (mac_vap[0] == NULL) || (en_mu_vap && (mac_vap[1] == NULL))) {
        return;
    }

    tcp_ack_buf_switch->us_tcp_ack_smooth_throughput >>= 1;
    tcp_ack_buf_switch->us_tcp_ack_smooth_throughput += (rx_throughput_mbps >> 1);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap[0]->uc_vap_id);
    if (!en_mu_vap && is_legacy_sta(mac_vap[0]) && hmac_vap != NULL &&
        ((hmac_vap->tcp_ack_buf_use_ctl_switch == OAL_SWITCH_ON) ||
        (tcp_ack_buf_switch->uc_ini_tcp_ack_buf_userctl_test_en == OAL_TRUE))) {
        /* 上层下达指令 指定吞吐量门限 */
        limit_throughput_high = tcp_ack_buf_switch->us_tcp_ack_buf_userctl_high;
        limit_throughput_low = tcp_ack_buf_switch->us_tcp_ack_buf_userctl_low;
        hmac_tcp_ack_buf_switch_judge(mac_vap[0], limit_throughput_high,
            limit_throughput_low, rx_throughput_mbps, 0); // 0代表只有一个vap
        return;
    }

    /* 上层未下达指定吞吐量门限或不支持该功能 */
    hmac_set_vap_throughput_threshould_by_bw(mac_vap, &limit_throughput_high,
                                             &limit_throughput_low, rx_throughput_mbps, up_vap_num);
}

#endif

typedef struct {
    uint8_t enabled;
    struct wlan_tcp_flow tcp_flow;
    uint32_t last_acknum;
    uint32_t last_ts;
} hmac_device_tcp_buf_flow_stru;

#define HMAC_DEVICE_TCP_BUF_FLOW_NUM 32
typedef struct {
    uint8_t inited;  /* 定制化是否使能tcp ack缓存 */
    int8_t stick;    /* 粘性开关, 根据流量更新, 值大于0时tcp ack缓存开启, 小于0时关闭 */
    uint8_t last_status;
    oal_spin_lock_stru lock;
    hmac_device_tcp_buf_flow_stru tcp_buf_flow[HMAC_DEVICE_TCP_BUF_FLOW_NUM];
} hmac_device_tcp_buf_mgmt_stru;

hmac_device_tcp_buf_mgmt_stru g_device_tcp_buf_mgmt = { 0 };

static inline void hmac_device_tcp_buf_flow_enable(uint8_t flow_index)
{
    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].enabled = OAL_TRUE;
}

static inline void hmac_device_tcp_buf_flow_disable(uint8_t flow_index)
{
    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].enabled = OAL_FALSE;
}

static inline uint8_t hmac_device_tcp_buf_flow_enabled(uint8_t flow_index)
{
    return g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].enabled;
}

static inline uint32_t hmac_device_tcp_buf_flow_last_ts(uint8_t flow_index)
{
    return g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].last_ts;
}

static uint8_t hmac_device_tcp_buf_get_aging_flow(void)
{
    uint8_t flow_index;
    uint8_t aging_flow = 0;
    uint32_t last_ts = 0xffffffff; /* invalid maximum timestamp */

    for (flow_index = 0; flow_index < HMAC_DEVICE_TCP_BUF_FLOW_NUM; flow_index++) {
        if (last_ts < hmac_device_tcp_buf_flow_last_ts(flow_index)) {
            continue;
        }
        aging_flow = flow_index;
        last_ts = hmac_device_tcp_buf_flow_last_ts(flow_index);
    }

    return aging_flow;
}

static uint32_t hmac_device_tcp_buf_get_avail_flow(uint8_t *avail_index)
{
    uint8_t flow_index;

    for (flow_index = 0; flow_index < HMAC_DEVICE_TCP_BUF_FLOW_NUM; flow_index++) {
        if (!hmac_device_tcp_buf_flow_enabled(flow_index)) {
            hmac_device_tcp_buf_flow_enable(flow_index);
            *avail_index = flow_index;
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

static inline void hmac_device_tcp_buf_update_flow(hmac_device_tcp_buf_flow_stru *tcp_flow, uint8_t flow_index)
{
    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].last_acknum = tcp_flow->last_acknum;
    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].last_ts = tcp_flow->last_ts;
}

static inline void hmac_device_tcp_buf_init_flow(hmac_device_tcp_buf_flow_stru *tcp_flow, uint8_t flow_index)
{
    hmac_device_tcp_buf_update_flow(tcp_flow, flow_index);

    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].tcp_flow.src_ip = tcp_flow->tcp_flow.src_ip;
    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].tcp_flow.dst_ip = tcp_flow->tcp_flow.dst_ip;
    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].tcp_flow.us_src_port = tcp_flow->tcp_flow.us_src_port;
    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].tcp_flow.us_dst_port = tcp_flow->tcp_flow.us_dst_port;

    g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].enabled = OAL_TRUE;
}

static void hmac_device_tcp_buf_add_flow(hmac_device_tcp_buf_flow_stru *tcp_flow)
{
    uint8_t flow_index;

    if (hmac_device_tcp_buf_get_avail_flow(&flow_index) != OAL_SUCC) {
        flow_index = hmac_device_tcp_buf_get_aging_flow();
    }

    hmac_device_tcp_buf_init_flow(tcp_flow, flow_index);
}

static inline uint8_t hmac_device_tcp_buf_compare_flow(uint8_t flow_index, hmac_device_tcp_buf_flow_stru *tcp_flow)
{
    return g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].tcp_flow.src_ip == tcp_flow->tcp_flow.src_ip &&
           g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].tcp_flow.dst_ip == tcp_flow->tcp_flow.dst_ip &&
           g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].tcp_flow.us_src_port == tcp_flow->tcp_flow.us_src_port &&
           g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].tcp_flow.us_dst_port == tcp_flow->tcp_flow.us_dst_port;
}

static inline uint8_t hmac_device_tcp_buf_duplicate_ack(uint8_t flow_index, hmac_device_tcp_buf_flow_stru *tcp_flow)
{
    return g_device_tcp_buf_mgmt.tcp_buf_flow[flow_index].last_acknum == tcp_flow->last_acknum;
}

static uint32_t hmac_device_tcp_buf_current_flow_proc(hmac_device_tcp_buf_flow_stru *tcp_flow, uint8_t *duplicate)
{
    uint8_t flow_index;

    for (flow_index = 0; flow_index < HMAC_DEVICE_TCP_BUF_FLOW_NUM; flow_index++) {
        if (!hmac_device_tcp_buf_flow_enabled(flow_index) || !hmac_device_tcp_buf_compare_flow(flow_index, tcp_flow)) {
            continue;
        }

        *duplicate = hmac_device_tcp_buf_duplicate_ack(flow_index, tcp_flow);
        hmac_device_tcp_buf_update_flow(tcp_flow, flow_index);
        return OAL_SUCC;
    }

    return OAL_FAIL;
}

static void hmac_device_tcp_buf_build_flow(oal_ip_header_stru *ip_hdr, hmac_device_tcp_buf_flow_stru *tcp_flow)
{
    oal_tcp_header_stru *tcp_hdr = (oal_tcp_header_stru *)(ip_hdr + 1);

    tcp_flow->tcp_flow.src_ip = ip_hdr->saddr;
    tcp_flow->tcp_flow.dst_ip = ip_hdr->daddr;
    tcp_flow->tcp_flow.us_src_port = tcp_hdr->us_sport;
    tcp_flow->tcp_flow.us_dst_port = tcp_hdr->us_dport;
    tcp_flow->last_acknum = tcp_hdr->acknum;
    tcp_flow->last_ts = oal_time_get_stamp_ms();
}

void hmac_device_tcp_buf_init(uint8_t enabled)
{
    g_device_tcp_buf_mgmt.inited = enabled;
    g_device_tcp_buf_mgmt.stick = 0;
    g_device_tcp_buf_mgmt.last_status = 0;
    oal_spin_lock_init(&g_device_tcp_buf_mgmt.lock);
}

static inline uint8_t hmac_device_tcp_buf_inited(void)
{
    return g_device_tcp_buf_mgmt.inited;
}

static inline uint8_t hmac_device_tcp_buf_enabled(void)
{
    return g_device_tcp_buf_mgmt.stick > 0;
}

static void hmac_device_tcp_buf_enable(void)
{
    uint8_t flow_index;

    oal_spin_lock_bh(&g_device_tcp_buf_mgmt.lock);
    for (flow_index = 0; flow_index < HMAC_DEVICE_TCP_BUF_FLOW_NUM; flow_index++) {
        hmac_device_tcp_buf_flow_disable(flow_index);
    }
    oal_spin_unlock_bh(&g_device_tcp_buf_mgmt.lock);
}

/* device tcp缓存功能的流量上下限, 在此范围内保持功能开启 */
#define HMAC_TCP_BUF_UPPER_BOUND 55
#define HMAC_TCP_BUF_LOWER_BOUND 5
static void hmac_device_tcp_buf_switch_update(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    uint32_t throughput = tx_throughput_mbps > rx_throughput_mbps ? tx_throughput_mbps : rx_throughput_mbps;

    if (throughput > HMAC_TCP_BUF_LOWER_BOUND && throughput < HMAC_TCP_BUF_UPPER_BOUND) {
        g_device_tcp_buf_mgmt.stick = oal_min(10, g_device_tcp_buf_mgmt.stick + 1); /* 正值上限10, 提升开启粘性 */
    } else {
        g_device_tcp_buf_mgmt.stick = oal_max(-2, g_device_tcp_buf_mgmt.stick - 1); /* 负值下限2, 降低关闭粘性 */
    }
}

static uint32_t hmac_device_tcp_buf_status_update(void)
{
    if (g_device_tcp_buf_mgmt.last_status == hmac_device_tcp_buf_enabled()) {
        return OAL_FAIL;
    }

    g_device_tcp_buf_mgmt.last_status = !g_device_tcp_buf_mgmt.last_status;
    if (g_device_tcp_buf_mgmt.last_status) {
        hmac_device_tcp_buf_enable();
    }

    return OAL_SUCC;
}

static void hmac_device_tcp_buf_status_sync(uint8_t enabled)
{
    (void)hmac_config_send_event(mac_res_get_mac_vap(0), WLAN_CFGID_DEVICE_TCP_BUF, sizeof(uint8_t), &enabled);
}

uint8_t hmac_device_tcp_buf_duplicate_ack_proc(oal_ip_header_stru *ip_hdr)
{
    hmac_device_tcp_buf_flow_stru tcp_flow = { 0 };
    uint8_t duplicate = OAL_FALSE;

    if (!hmac_device_tcp_buf_enabled()) {
        return OAL_FALSE;
    }

    hmac_device_tcp_buf_build_flow(ip_hdr, &tcp_flow);

    oal_spin_lock_bh(&g_device_tcp_buf_mgmt.lock);
    if (hmac_device_tcp_buf_current_flow_proc(&tcp_flow, &duplicate) != OAL_SUCC) {
        hmac_device_tcp_buf_add_flow(&tcp_flow);
    }
    oal_spin_unlock_bh(&g_device_tcp_buf_mgmt.lock);

    return duplicate;
}

void hmac_device_tcp_buf_switch(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    if (!hmac_device_tcp_buf_inited()) {
        return;
    }

    hmac_device_tcp_buf_switch_update(tx_throughput_mbps, rx_throughput_mbps);

    if (hmac_device_tcp_buf_status_update() == OAL_SUCC) {
        hmac_device_tcp_buf_status_sync(g_device_tcp_buf_mgmt.last_status);
    }
}
