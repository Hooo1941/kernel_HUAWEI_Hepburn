/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : multi netbuf amsdu
 * 作    者 : wifi
 * 创建日期 : 2020年9月8日
 */
#include "hmac_multi_netbuf_amsdu.h"
#include "hmac_config.h"
#include "hmac_tx_amsdu.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MULTI_NETBUF_AMSDU_C

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
static inline uint8_t wlan_amsdu_level_to_num(uint8_t level)
{
    return (((level) == WLAN_TX_AMSDU_NONE) ? 0 : \
        /* level number 2 */
        ((level) == WLAN_TX_AMSDU_BY_2) ? 2 : \
        /* level number 3 */
        ((level) == WLAN_TX_AMSDU_BY_3) ? 3 : \
        /* level number 4 */
        ((level) == WLAN_TX_AMSDU_BY_4) ? 4 : \
        /* level number 8 */
        ((level) == WLAN_TX_AMSDU_BY_8) ? 8 : 0);
}

/*
 * 函 数 名  : hmac_set_amsdu_num_based_protocol
 * 功能描述  : 根据协议模式决定amsdu聚合个数
 * 1.日    期  : 2019年8月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_set_amsdu_num_based_protocol(mac_vap_stru *mac_vap, wlan_tx_amsdu_enum_uint8 tx_amsdu_level_type)
{
    /* 协议规定HT mpdu长度不得超过4095字节 */
    if (oal_value_eq_any3(mac_vap->en_protocol, WLAN_HT_ONLY_MODE, WLAN_HT_11G_MODE, WLAN_HT_MODE)) {
        return  WLAN_TX_AMSDU_BY_2;
    } else if ((mac_vap->en_protocol == WLAN_VHT_MODE) ||
               (mac_vap->en_protocol == WLAN_VHT_ONLY_MODE) ||
               (g_wlan_spec_cfg->feature_11ax_is_open && (mac_vap->en_protocol == WLAN_HE_MODE))) {
        return  tx_amsdu_level_type;
    } else {
        return  WLAN_TX_AMSDU_NONE;
    }
}

/*
 * 函 数 名  : hmac_update_amsdu_num
 * 功能描述  : mp13根据当前吞吐决定amsdu聚合个数
 * 1.日    期  : 2019年11月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_update_amsdu_num_mp13(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag)
{
    uint32_t limit_throughput_high;
    uint32_t limit_throughput_low;
    uint32_t tx_throughput_mbps = performance_stat_params->tx_throughput_mbps;
    /* 每秒吞吐量门限 */
    limit_throughput_high = g_st_tx_large_amsdu.us_amsdu_throughput_high >> mu_vap_flag;
    limit_throughput_low  = g_st_tx_large_amsdu.us_amsdu_throughput_low >> mu_vap_flag;

    if (tx_throughput_mbps > limit_throughput_high) {
        /* 高于高门限,切换amsdu大包聚合 */
        return WLAN_TX_AMSDU_BY_2;
    } else if (tx_throughput_mbps < limit_throughput_low) {
        /* 低于低门限,关闭amsdu大包聚合 */
        return WLAN_TX_AMSDU_NONE;
    } else {
        /* 介于低门限-高门限之间,不作切换 */
        return g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id];
    }
}
OAL_STATIC void hmac_tx_amsdu_throughput_high_limit_adjust_11ax(mac_vap_stru *mac_vap, uint32_t *limit_throughput_high)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 解决与mp13的sdio兼容性问题：11ax门限比11ac低100M */
        if (mac_vap->en_protocol == WLAN_HE_MODE) {
            *limit_throughput_high -= WLAN_AMSDU_THROUGHPUT_TH_HE_VHT_DIFF;
        }
    }
#endif
}
OAL_STATIC wlan_tx_amsdu_enum_uint8 hmac_get_tx_amsdu_num_by_tx_throughput(uint32_t tx_throughput_mbps,
    uint32_t limit_throughput_high, uint32_t limit_throughput_middle)
{
    if (tx_throughput_mbps > limit_throughput_high) {
        /* 高于高门限, 则开启4xmsdu */
        return WLAN_TX_AMSDU_BY_4;
    } else if (tx_throughput_mbps > (limit_throughput_middle << 1)) {
        /* 起始门限的两倍以上并且小于高门限, 则开启3xmsdu */
        return WLAN_TX_AMSDU_BY_3;
    } else {
        /* 高于起始门限,则开启2xmsdu */
        return WLAN_TX_AMSDU_BY_2;
    }
}

uint8_t hmac_adjust_tx_amsdu_num_mp15(mac_vap_stru *mac_vap, uint32_t tx_throughput_mbps,
    oal_bool_enum_uint8 mu_vap_flag)
{
    uint32_t limit_throughput_high = g_st_tx_large_amsdu.us_amsdu_throughput_high >> mu_vap_flag;
    uint32_t limit_throughput_middle = g_st_tx_large_amsdu.us_amsdu_throughput_middle >> mu_vap_flag;
    uint32_t limit_throughput_low = g_st_tx_large_amsdu.us_amsdu_throughput_low >> mu_vap_flag;
    wlan_tx_amsdu_enum_uint8 max_amsdu;

    hmac_tx_amsdu_throughput_high_limit_adjust_11ax(mac_vap, &limit_throughput_high);

    /* 打开AMSDU, middle门限是开启amsdu的起始门限 */
    if (tx_throughput_mbps > limit_throughput_middle) {
        max_amsdu = hmac_get_tx_amsdu_num_by_tx_throughput(tx_throughput_mbps,
                                                           limit_throughput_high, limit_throughput_middle);
        /* 已经考虑对端的AMSDU聚合长度 */
        max_amsdu = hmac_set_amsdu_num_based_protocol(mac_vap, max_amsdu);
    } else if (tx_throughput_mbps > limit_throughput_low) {
        /* 低门限-中门限, 若上次是AMSDU聚合的，则聚合2个; 否则不聚合 */
        if (g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id] != WLAN_TX_AMSDU_NONE) {
            max_amsdu = hmac_set_amsdu_num_based_protocol(mac_vap, WLAN_TX_AMSDU_BY_2);
        } else {
            max_amsdu = WLAN_TX_AMSDU_NONE;
        }
    } else {
        /* 低于低门限, 关闭amsdu大包聚合 */
        max_amsdu = WLAN_TX_AMSDU_NONE;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        max_amsdu = oal_min(oal_max(max_amsdu, mac_vap->bit_ofdma_aggr_num), WLAN_TX_AMSDU_BY_8);
    }
#endif

    return max_amsdu;
}

/*
 * 函 数 名  : hmac_update_amsdu_num
 * 功能描述  : mp15根据当前吞吐决定amsdu聚合个数
 * 1.日    期  : 2019年11月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint8_t hmac_update_amsdu_num_mp15(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag)
{
    return hmac_adjust_tx_amsdu_num_mp15(mac_vap, performance_stat_params->tx_throughput_mbps, mu_vap_flag);
}
uint8_t hmac_adjust_rx_amsdu_num_mp16(mac_vap_stru *mac_vap, uint32_t rx_throughput_mbps, uint32_t tx_pps,
    wlan_tx_amsdu_enum_uint8 tx_amsdu)
{
    uint32_t limit_throughput_high;
    uint32_t limit_throughput_low;
    uint32_t limit_pps_high;
    uint32_t limit_pps_low;
    wlan_tx_amsdu_enum_uint8 amsdu_level_rx, amsdu_level;

    /* 每秒吞吐量门限，rx吞吐量判决amsdu_level */
    hmac_tx_small_amsdu_get_limit_throughput(&limit_throughput_high, &limit_throughput_low);
    /* 大于low才开启聚合 */
    if (rx_throughput_mbps > limit_throughput_low) {
        if (rx_throughput_mbps > (limit_throughput_high << 1)) {
            amsdu_level_rx = WLAN_TX_AMSDU_BY_8;
        } else if (rx_throughput_mbps > limit_throughput_high) {
            amsdu_level_rx = WLAN_TX_AMSDU_BY_4;
        } else {
            amsdu_level_rx = WLAN_TX_AMSDU_BY_2;
        }
    } else {
        amsdu_level_rx = WLAN_TX_AMSDU_NONE;
    }
    amsdu_level_rx = oal_max(amsdu_level_rx, tx_amsdu);
    /* 每秒PPS门限，pps判决amsdu_level */
    hmac_tx_small_amsdu_get_limit_pps(&limit_pps_high, &limit_pps_low);
    if (tx_pps > limit_pps_low) {
        if (tx_pps > (limit_pps_high << 1)) {
            amsdu_level = WLAN_TX_AMSDU_BY_8;
        } else if (tx_pps > limit_pps_high) {
            amsdu_level = WLAN_TX_AMSDU_BY_4;
        } else {
            amsdu_level = WLAN_TX_AMSDU_BY_2;
        }
    } else {
        amsdu_level = WLAN_TX_AMSDU_NONE;
    }
    /* 防止流量在门限附近抖动导致的amsdu聚合频繁切换 */
    if (amsdu_level < g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id]) {
        ++g_freq_lock_control.amsdu_level_switch_count;
        if (g_freq_lock_control.amsdu_level_switch_count < WLAN_TX_AMSDU_SWITCH_DOWN_LIMIT_CNT) {
            amsdu_level = g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id];
        } else {
            g_freq_lock_control.amsdu_level_switch_count = 0;
        }
    } else {
        g_freq_lock_control.amsdu_level_switch_count = 0;
    }

    amsdu_level = oal_max(amsdu_level_rx, amsdu_level);
    return amsdu_level;
}

uint8_t hmac_adjust_tx_amsdu_num_mp16(mac_vap_stru *mac_vap, uint32_t tx_throughput_mbps,
    oal_bool_enum_uint8 mu_vap_flag)
{
    uint32_t limit_throughput_high = g_st_tx_large_amsdu.us_amsdu_throughput_high >> mu_vap_flag;
    uint32_t limit_throughput_middle = g_st_tx_large_amsdu.us_amsdu_throughput_middle >> mu_vap_flag;
    uint32_t limit_throughput_low = g_st_tx_large_amsdu.us_amsdu_throughput_low >> mu_vap_flag;
    wlan_tx_amsdu_enum_uint8 max_amsdu;

    hmac_tx_amsdu_throughput_high_limit_adjust_11ax(mac_vap, &limit_throughput_high);

    /* 打开AMSDU, middle门限是开启amsdu的起始门限 */
    if (tx_throughput_mbps > limit_throughput_middle) {
        if (tx_throughput_mbps > (limit_throughput_high << 1)) {
            /* 高于2倍高门限, 则开启8xmsdu */
            max_amsdu = WLAN_TX_AMSDU_BY_8;
        } else {
            max_amsdu = hmac_get_tx_amsdu_num_by_tx_throughput(tx_throughput_mbps,
                                                               limit_throughput_high, limit_throughput_middle);
        }
        /* 已经考虑对端的AMSDU聚合长度 */
        max_amsdu = hmac_set_amsdu_num_based_protocol(mac_vap, max_amsdu);
    } else if (tx_throughput_mbps > limit_throughput_low &&
               g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id] != WLAN_TX_AMSDU_NONE) {
        /* 低门限-中门限, 若上次是AMSDU聚合的，则聚合2个; 否则不聚合 */
        max_amsdu = hmac_set_amsdu_num_based_protocol(mac_vap, WLAN_TX_AMSDU_BY_2);
    } else {
        /* 低于低门限, 关闭amsdu大包聚合 */
        max_amsdu = WLAN_TX_AMSDU_NONE;
    }

    return max_amsdu;
}

/*
 * 函 数 名  : hmac_update_amsdu_num
 * 功能描述  : mp16根据当前吞吐决定amsdu聚合个数
 * 1.日    期  : 2020年9月11日
 *   修改内容  : 新生成函数
 */
uint8_t hmac_update_amsdu_num_mp16(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag)
{
    wlan_tx_amsdu_enum_uint8 max_amsdu;

    max_amsdu = hmac_adjust_tx_amsdu_num_mp16(mac_vap, performance_stat_params->tx_throughput_mbps, mu_vap_flag);

    max_amsdu = hmac_adjust_rx_amsdu_num_mp16(mac_vap, performance_stat_params->rx_throughput_mbps,
        performance_stat_params->tx_pps, max_amsdu);
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        max_amsdu = oal_min(oal_max(max_amsdu, mac_vap->bit_ofdma_aggr_num), WLAN_TX_AMSDU_BY_8);
    }
#endif
    return max_amsdu;
}
OAL_STATIC uint32_t hmac_get_amsdu_judge_result(mac_vap_stru *pst_vap1, mac_vap_stru *pst_vap2,
    oal_bool_enum_uint8 en_mu_vap_flag, uint32_t *up_vap_num)
{
    if ((*up_vap_num == 0) || (pst_vap1 == NULL) || (en_mu_vap_flag && (pst_vap2 == NULL))) {
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 多个vap up时只支持两个vap打开算法 */
    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_CHBA)) && (*up_vap_num > TWO_UP_VAP_NUMS)) {
        *up_vap_num = TWO_UP_VAP_NUMS;
    }
#endif
    return OAL_SUCC;
}
void hmac_cfg_amsdu_info_to_dmac(mac_vap_stru *mac_vap)
{
    uint32_t ret;
    uint8_t amsdu_maxnum;

    amsdu_maxnum = (uint8_t)wlan_amsdu_level_to_num(g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap->uc_vap_id]);
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_DMAC_SYNC_AMSDU_MAXNUM,
        sizeof(uint8_t), (uint8_t *)&amsdu_maxnum);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_cfg_amsdu_info_to_dmac::hmac_config_send_event failed[%d].}", ret);
        return;
    }
}
/*
 * 函 数 名  : hmac_tx_amsdu_ampdu_switch
 * 功能描述  : 上行性能大于300M时切换为amsdu大包聚合
 * 1.日    期  : 2018年2月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_tx_amsdu_ampdu_switch(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    mac_vap_stru *mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    wlan_tx_amsdu_enum_uint8 tx_amsdu = 0;
    uint32_t up_vap_num;
    oal_bool_enum_uint8 mu_vap;
    uint8_t idx;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_performance_stat_stru performance_stat_params = {tx_throughput_mbps, rx_throughput_mbps,
        g_freq_lock_control.tx_pps};
    /* 如果定制化不支持硬件聚合 */
    if (g_st_tx_large_amsdu.uc_host_large_amsdu_en == OAL_FALSE) {
        return;
    }

    up_vap_num = mac_device_find_up_vaps(mac_device, mac_vap, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    mu_vap = (up_vap_num > 1);
    if (hmac_get_amsdu_judge_result(mac_vap[0], mac_vap[1], mu_vap, &up_vap_num) == OAL_FAIL) {
        return;
    }
    for (idx = 0; idx < up_vap_num; idx++) {
        tx_amsdu = wlan_chip_tx_update_amsdu_num(mac_vap[idx], (hmac_performance_stat_stru *)&performance_stat_params,
            mu_vap);
        /* 聚合异常或当前聚合方式相同,不处理 */
        if (tx_amsdu == WLAN_TX_AMSDU_BUTT ||
            g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap[idx]->uc_vap_id] == tx_amsdu) {
            continue;
        }

        g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap[idx]->uc_vap_id] = tx_amsdu;
        /*  干扰场景关闭amsdu发送 */
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap[idx]->uc_vap_id);
        if ((hmac_vap != NULL) && (hmac_vap->d2h_amsdu_switch == OAL_FALSE)) {
            g_st_tx_large_amsdu.en_tx_amsdu_level[mac_vap[idx]->uc_vap_id] = WLAN_TX_AMSDU_NONE;
        }

        oam_warning_log4(mac_vap[idx]->uc_vap_id, OAM_SF_ANY,
            "{hmac_tx_amsdu_ampdu_switch: up vap num[%d], mu_vap[%d], amsdu level[%d],tx_throught= [%d]!}",
            up_vap_num, mu_vap, tx_amsdu, tx_throughput_mbps);

        hmac_cfg_amsdu_info_to_dmac(mac_vap[idx]);
    }
}
#endif
