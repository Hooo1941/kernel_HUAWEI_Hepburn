/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: CHBA ps part
 * Author: wifi
 * Create: 2021-05-20
 */

#include "hmac_chba_ps.h"
#include "securec.h"
#include "hmac_chba_frame.h"
#include "hmac_chba_sync.h"
#include "hmac_chba_function.h"
#include "hmac_config.h"
#include "hmac_chba_user.h"
#include "hmac_resource.h"
#include "hmac_stat.h"
#include "mac_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHBA_PS_C

#ifdef _PRE_WLAN_CHBA_MGMT
uint8_t g_ps_feature_enable = TRUE;
/* chba低功耗吞吐门限
 *  1.从最低档到第二档默认立刻切换
 *  2.从第二档到最高档默认经过30个统计周期(3s)
 *  3.从最高档到第二档默认经过10个统计周期(1s)
 *  4.从第二档到最低档默认经过50个统计周期(5s) */
hmac_chba_ps_throught_thres_stru g_chba_ps_throught_thres = { 0, 30, 10, 50 };

/* chba每100ms获取吞吐bytes(64位) */
static uint64_t chba_get_trx_bytes(uint64_t last_trx_bytes, uint64_t trx_bytes)
{
    return (last_trx_bytes > trx_bytes) ?
        (((0xFFFFFFFFFFFFFFFF) - last_trx_bytes) + trx_bytes) : (trx_bytes - last_trx_bytes);
}

/*
 * 功能描述  : 判断最新的数据是否是tx rx为零
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
static uint32_t hmac_chba_latest_data_is_zero(hmac_chba_user_throughput_stru *info)
{
    if ((info->rx_bytes_windows[info->peer_cnt] == 0) && (info->tx_bytes_windows[info->peer_cnt] == 0)) {
        return TRUE;
    }
    return FALSE;
}

/*
 * 功能描述  : 判断最新的数据是否是大包，阈值可变
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
static uint32_t hmac_chba_latest_data_is_large_packet(hmac_chba_user_throughput_stru *info)
{
    if (info->rx_bytes_windows[info->peer_cnt] >= LAGRE_PACKET_BYTES_THRESHOLD ||
        info->tx_bytes_windows[info->peer_cnt] >= LAGRE_PACKET_BYTES_THRESHOLD) {
        return TRUE;
    }
    return FALSE;
}

/*
 * 功能描述  : 生成user 同步信息并同步
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
void hmac_chba_sync_user_bitmap_info(mac_vap_stru *mac_vap, hmac_chba_vap_stru *chba_vap_info,
    hmac_user_stru *hmac_user, uint8_t tmp_bitmap_level)
{
    uint32_t ret;
    chba_user_ps_cfg_stru user_ps_cfg = { 0 };

    /* 将user同步信息初始化后, 同步给dmac */
    user_ps_cfg.user_idx = hmac_user->st_user_base_info.us_assoc_id;
    user_ps_cfg.bitmap_level = tmp_bitmap_level;
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CHBA_AUTO_ADJUST_BITMAP_LEVEL, sizeof(chba_user_ps_cfg_stru),
        (uint8_t *)&user_ps_cfg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CHBA, "hmac_chba_sync_user_bitmap_info::send_event fail[%d]", ret);
    }
}

/*
 * 功能描述  : 生成vap同步信息并同步
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
void hmac_chba_sync_vap_bitmap_info(mac_vap_stru *mac_vap, hmac_chba_vap_stru *chba_vap_info)
{
    uint32_t ret;
    uint8_t bitmap_level = chba_vap_info->vap_bitmap_level;

    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CHBA_SET_VAP_BITMAP_LEVEL, sizeof(uint8_t), &bitmap_level);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CHBA, "hmac_chba_sync_vap_bitmap_info:: send event fail");
    }
}

/*
 * 功能描述  : 将user bitmap同步给驱动
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
static void hmac_chba_sync_user_bitmap_level(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, uint8_t tmp_bitmap_level)
{
    hmac_chba_vap_stru *chba_vap_info = NULL;

    chba_vap_info = hmac_chba_get_chba_vap_info(hmac_vap);
    if (chba_vap_info == NULL) {
        return;
    }

    if (hmac_vap_is_connecting(&hmac_vap->st_vap_base_info) == OAL_TRUE) {
        return;
    }

    hmac_chba_sync_user_bitmap_info(&(hmac_vap->st_vap_base_info), chba_vap_info, hmac_user, tmp_bitmap_level);
    oam_warning_log1(0, OAM_SF_CHBA, "hmac_chba_sync_user_bitmap_level: %d", tmp_bitmap_level);
}


void hmac_chba_sync_user_bitmap_level_by_data_type(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, mac_tx_ctl_stru *tx_ctl)
{
    hmac_chba_user_stru *chba_user_info = NULL;

    if (oal_any_null_ptr3(hmac_vap, hmac_user, tx_ctl)) {
        return;
    }

    chba_user_info = &(hmac_user->chba_user);
    if (chba_user_info->user_bitmap_level != CHBA_BITMAP_KEEP_ALIVE_LEVEL) {
        return;
    }
    /* CHBA保活场景，发送TCP 同步帧，强制调整低功耗时隙，以便TCP及时建链 */
    if (mac_get_cb_frame_type(tx_ctl) == WLAN_CB_FRAME_TYPE_DATA &&
        mac_get_cb_frame_subtype(tx_ctl) == MAC_DATA_TCP_SYN) {
        hmac_chba_sync_user_bitmap_level(hmac_vap, hmac_user, CHBA_BITMAP_HALF_AWAKE_LEVEL);
    }
}

/*
 * 功能描述  : CHBA:hmac user bitmap档位的更新接口
 *   日    期  : 2021年11月23日
 *   作    者  : wifi7
 */
uint32_t hmac_chba_update_user_bitmap_level(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    hmac_user_stru *hmac_user = NULL;
    chba_user_ps_cfg_stru *user_ps_cfg = NULL;
    hmac_chba_user_stru *chba_user_info = NULL;

    if (oal_any_null_ptr2(mac_vap, param)) {
        oam_error_log0(0, OAM_SF_CHBA, "{hmac_chba_update_user_bitmap_level: ptr is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* chba低功耗开关关闭时, 不更新user bitmap */
    if (hmac_chba_get_ps_switch() == FALSE) {
        return OAL_SUCC;
    }

    user_ps_cfg = (chba_user_ps_cfg_stru *)param;
    if (user_ps_cfg->bitmap_level >= CHBA_BITMAP_LEVEL_BUTT) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "{hmac_chba_update_user_bitmap_level:invalid bitmap level[%d]}", user_ps_cfg->bitmap_level);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    hmac_user = mac_res_get_hmac_user(user_ps_cfg->user_idx);
    if (hmac_user == NULL) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "{hmac_chba_update_user_bitmap_level:invalid user_idx[%d]}", user_ps_cfg->user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新user bitmap */
    chba_user_info = &(hmac_user->chba_user);
    chba_user_info->user_bitmap_level = user_ps_cfg->bitmap_level;
    /* user bitmap更新, chba流量模型重新开始计数 */
    chba_user_info->power_up_count = 0;
    chba_user_info->power_down_count = 0;
    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CHBA,
        "{hmac_chba_update_user_bitmap_level:user_idx[%d], update user_bitmap_level[%d]}",
        hmac_user->st_user_base_info.us_assoc_id, chba_user_info->user_bitmap_level);
    return OAL_SUCC;
}

/*
 * 功能描述  : CHBA:hmac vap_bitmap档位更新接口
 *   日    期  : 2021年11月23日
 *   作    者  : wifi7
 */
uint32_t hmac_chba_update_vap_bitmap_level(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    uint8_t vap_bitmap_level;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_chba_vap_stru *chba_vap_info = NULL;

    if (oal_any_null_ptr2(mac_vap, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    chba_vap_info = hmac_chba_get_chba_vap_info(hmac_vap);
    if (chba_vap_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* chba低功耗开关关闭时, 不更新vap bitmap */
    if (hmac_chba_get_ps_switch() == FALSE) {
        return OAL_SUCC;
    }

    vap_bitmap_level = *(uint8_t *)param;
    if (vap_bitmap_level >= CHBA_BITMAP_LEVEL_BUTT) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "{hmac_chba_update_vap_bitmap_level:invalid bitmap level[%d]}", vap_bitmap_level);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    chba_vap_info->vap_bitmap_level = vap_bitmap_level;

    hmac_chba_save_update_beacon_pnf(hmac_chba_sync_get_domain_bssid());
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHBA,
        "{hmac_chba_update_vap_bitmap_level:upate vap_bitmap_level[%d]}", chba_vap_info->vap_bitmap_level);
    return OAL_SUCC;
}

/*
 * 功能描述  : user的tx rx流量更新
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
static void hmac_chba_throughput_windows_update(hmac_chba_vap_stru *chba_vap_info,
    hmac_chba_user_stru *chba_user_info, hmac_user_stru *hmac_user)
{
    uint8_t peer_cnt;
    hmac_chba_user_throughput_stru *chba_user_throughput = NULL;
    chba_user_throughput = &(chba_user_info->throughput_info);

    /* 进行throughput相关info的更新，如果是第一次直接传值，其余时候进行数据处理
      数据处理的方式为取最近两次统计(默认值为100ms)的tx rx bytes,packets的差值 */
    if (chba_user_throughput->last_rx_bytes == 0 && chba_user_throughput->last_tx_bytes == 0) {
        oam_warning_log1(0, OAM_SF_CHBA, "{hmac_chba_throughput_windows_update:user_idx[%d] first time update}",
            hmac_user->st_user_base_info.us_assoc_id);
    } else {
        chba_user_throughput->peer_cnt = (chba_user_throughput->peer_cnt + 1) % CHBA_THROUGHPUT_WINDOW_LEN;
        peer_cnt = chba_user_throughput->peer_cnt;
        chba_user_throughput->tx_bytes_windows[peer_cnt] = chba_get_trx_bytes(chba_user_throughput->last_tx_bytes,
            hmac_user->user_stats.tx_bytes);
        chba_user_throughput->rx_bytes_windows[peer_cnt] = chba_get_trx_bytes(chba_user_throughput->last_rx_bytes,
            hmac_user->user_stats.rx_bytes);
        if (chba_vap_info->chba_log_level >= CHBA_DEBUG_STAT_INFO_LOG) {
            oam_warning_log3(0, OAM_SF_CHBA,
                "{hmac_chba_throughput_windows_update:user_idx[%d], tx_bytes[%d], rx_bytes[%d]}",
                hmac_user->st_user_base_info.us_assoc_id, chba_user_throughput->tx_bytes_windows[peer_cnt],
                chba_user_throughput->rx_bytes_windows[peer_cnt]);
        }
    }
    chba_user_throughput->last_tx_bytes = hmac_user->user_stats.tx_bytes;
    chba_user_throughput->last_rx_bytes = hmac_user->user_stats.rx_bytes;

    /* 扫描流量窗口，查询最近5周期中有几个周期有吞吐、有几个周期有2000bytes以上的高吞吐, 更新统计结果 */
    chba_user_throughput->trx_windows_cnt = 0;
    chba_user_throughput->high_trx_windows_cnt = 0;
    for (peer_cnt = 0; peer_cnt < CHBA_THROUGHPUT_WINDOW_LEN; peer_cnt++) {
        if (chba_user_throughput->tx_bytes_windows[peer_cnt] > 0 ||
            (chba_user_throughput->rx_bytes_windows[peer_cnt] > 0)) {
            chba_user_throughput->trx_windows_cnt++; /* 本周期有吞吐, trx cnt + 1 */
            if ((chba_user_throughput->tx_bytes_windows[peer_cnt] >= LAGRE_PACKET_BYTES_THRESHOLD) ||
                (chba_user_throughput->rx_bytes_windows[peer_cnt] >= LAGRE_PACKET_BYTES_THRESHOLD)) {
                chba_user_throughput->high_trx_windows_cnt++; /* 本周期有高吞吐 */
            }
        }
    }
}

/*
 * 功能描述  : 检查是否要进行bitmap的更新
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
static uint32_t hmac_chba_check_throughput_info_update_bitmap_level(hmac_chba_user_stru *chba_user_info)
{
    /* 如果当前处于NON_SYNC状态，或者trySyncFlag，则进入同步处理流程；其他分别进入Discovery或Work处理流程 */
    if (chba_user_info == NULL || hmac_chba_get_sync_state() == CHBA_NON_SYNC) {
        return FALSE;
    }

    /* 如果当前为保活档且最近100ms tx rx均为0不update bitmap，
       如果当前为全醒档且最近100ms tx rx有一个不为0，则不update */
    if (hmac_chba_latest_data_is_zero(&chba_user_info->throughput_info) == TRUE &&
        (chba_user_info->user_bitmap_level == CHBA_BITMAP_KEEP_ALIVE_LEVEL)) {
        return FALSE;
    } else if (hmac_chba_latest_data_is_zero(&chba_user_info->throughput_info) == FALSE &&
        chba_user_info->user_bitmap_level == CHBA_BITMAP_ALL_AWAKE_LEVEL) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/*
 * 功能描述  : 流量模型算法，计算得出更新后的bitmap
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
static uint8_t hmac_chba_calc_bitmap_by_throughput(hmac_chba_vap_stru *chba_vap_info, hmac_user_stru *hmac_user,
    hmac_chba_user_stru *chba_user_info)
{
    uint8_t need_level_up;
    uint8_t cur_bitmap_level = chba_user_info->user_bitmap_level;
    hmac_chba_user_throughput_stru *tmp_throughput_info = &chba_user_info->throughput_info;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(chba_vap_info->mac_vap_id);
    hmac_chba_ps_throught_thres_stru *chba_thres = hmac_chba_get_ps_throught_thres(hmac_vap);

    if (chba_vap_info->chba_log_level >= CHBA_DEBUG_ALL_LOG) {
        oam_warning_log4(chba_vap_info->mac_vap_id, OAM_SF_CFG,
            "{hmac_chba_calc_bitmap_by_throughput:user_idx[%d], cur_bitmap_level[%d], up_cnt[%d], down_cnt[%d]}",
            hmac_user->st_user_base_info.us_assoc_id, cur_bitmap_level, chba_user_info->power_up_count,
            chba_user_info->power_down_count);
    }
    /* powerUpCount: txrx至少有1个不为0则加1, 清0的条件满足:
       升档或者最近连续5次统计中至少有3次txrx均为0或者收到bitmap req;
       powerDownCount: txrx均为0则加1，清0的条件满足: 降档或者最近连续5次统计中至少2次为大包或收到bitmap req */
    if (!hmac_chba_latest_data_is_zero(tmp_throughput_info)) {
        chba_user_info->power_up_count++;
        if (hmac_chba_user_is_in_high_throughput(tmp_throughput_info->high_trx_windows_cnt)) {
            chba_user_info->power_down_count = 0;
        }

        /* 自bitmap等级3升档: 最近的tx rx统计非0，且最近5次中至少3次为小包 或者有1次大包 */
        need_level_up = (cur_bitmap_level == CHBA_BITMAP_KEEP_ALIVE_LEVEL) &&
            (hmac_chba_user_is_in_trxing(tmp_throughput_info->trx_windows_cnt) ||
            hmac_chba_latest_data_is_large_packet(tmp_throughput_info));
        if (need_level_up == OAL_TRUE && chba_user_info->power_up_count > chba_thres->upgrade_to_lv_two_thres) {
            return CHBA_BITMAP_HALF_AWAKE_LEVEL;
        }
    } else {
        chba_user_info->power_down_count++;
        if (hmac_chba_user_is_in_zero_throughput(tmp_throughput_info->trx_windows_cnt)) {
            chba_user_info->power_up_count = 0;
        }
    }

    /* bitmap等级2降到等级3的条件:  powerDownCount 大于阈值 */
    if (chba_user_info->power_down_count > chba_thres->downgrade_to_lv_three_thres &&
        cur_bitmap_level == CHBA_BITMAP_HALF_AWAKE_LEVEL) {
        return CHBA_BITMAP_KEEP_ALIVE_LEVEL;
    } else if (chba_user_info->power_down_count > chba_thres->downgrade_to_lv_two_thres &&
        cur_bitmap_level == CHBA_BITMAP_ALL_AWAKE_LEVEL) {
        /* bitmap等级1降到等级2的条件: power_save_count大于阈值 */
        return CHBA_BITMAP_HALF_AWAKE_LEVEL;
    }

    /* bitmap等级2升到等级1的条件: power_up_count大于阈值 */
    if (chba_user_info->power_up_count > chba_thres->upgrade_to_lv_one_thres &&
        cur_bitmap_level == CHBA_BITMAP_HALF_AWAKE_LEVEL) {
        return CHBA_BITMAP_ALL_AWAKE_LEVEL;
    }

    /* 如果不满足以上条件，档位不变 */
    return chba_user_info->user_bitmap_level;
}

/*
 * 功能描述  : 流量模型算法入口
 *   日    期  : 2021年3月25日
 *   作    者  : wifi
 */
void hmac_chba_update_throughput_info(hmac_chba_vap_stru *chba_vap_info)
{
    uint8_t tmp_bitmap_level;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_chba_user_stru *chba_user_info = NULL;
    mac_user_stru *mac_user = NULL;
    oal_dlist_head_stru *entry = NULL;
    oal_dlist_head_stru *dlist_tmp = NULL;

    if (chba_vap_info == NULL) {
        oam_error_log0(0, OAM_SF_CHBA, "{hmac_chba_update_throughput_info:chba_vap_info is NULL!}");
        return;
    }
    /* 未初始化不处理 */
    if ((hmac_chba_get_module_state() == MAC_CHBA_MODULE_STATE_UNINIT) || (hmac_chba_get_ps_switch() == FALSE)) {
        return;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(chba_vap_info->mac_vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    /* 对每一条链路的流程模型进行更新 */
    oal_dlist_search_for_each_safe(entry, dlist_tmp, &(hmac_vap->st_vap_base_info.st_mac_user_list_head)) {
        mac_user = oal_dlist_get_entry(entry, mac_user_stru, st_user_dlist);
        if (mac_user == NULL) {
            continue;
        }
        hmac_user = mac_res_get_hmac_user(mac_user->us_assoc_id);
        if ((hmac_user == NULL) || (mac_user_get_port(&hmac_user->st_user_base_info) == OAL_FALSE)) {
            continue;
        }
        chba_user_info = &(hmac_user->chba_user);
        /* 将user吞吐放入算法部分进行计算 */
        hmac_chba_throughput_windows_update(chba_vap_info, chba_user_info, hmac_user);
        if (hmac_chba_check_throughput_info_update_bitmap_level(chba_user_info) == TRUE) {
            tmp_bitmap_level = hmac_chba_calc_bitmap_by_throughput(chba_vap_info, hmac_user, chba_user_info);
            if (tmp_bitmap_level != chba_user_info->user_bitmap_level) {
                oam_warning_log3(0, OAM_SF_CHBA,
                    "{hmac_chba_update_throughput_info:user_idx[%d], set user bitmap_level[%d]->[%d]}",
                    mac_user->us_assoc_id, chba_user_info->user_bitmap_level, tmp_bitmap_level);
                oam_warning_log2(0, OAM_SF_CHBA,
                    "{hmac_chba_update_throughput_info:cur user up_cnt[%d], dwon_cnt[%d]}",
                    chba_user_info->power_up_count, chba_user_info->power_down_count);
                /* 准备更新bitmap，原统计结果清0 */
                chba_user_info->power_up_count = 0;
                chba_user_info->power_down_count = 0;
                /* 将此链路新bitmap消息同步到dmac */
                hmac_chba_sync_user_bitmap_level(hmac_vap, hmac_user, tmp_bitmap_level);
            }
        }
    }
}
/*
 * 功能描述  : 调试命令接口, 入参为user mac, user work bitmap。
 */
uint32_t hmac_chba_set_user_bitmap_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *params)
{
    uint32_t ret;
    hmac_chba_vap_stru *chba_vap_info = NULL;
    mac_chba_set_ps_bitmap_params *ps_bitmap_info = NULL;
    hmac_user_stru *hmac_user = NULL;
    chba_user_ps_cfg_stru user_ps_cfg;

    ps_bitmap_info = (mac_chba_set_ps_bitmap_params *)params;
    chba_vap_info = hmac_get_up_chba_vap_info();
    if (chba_vap_info == NULL) {
        oam_warning_log0(0, OAM_SF_CHBA, "{hmac_chba_set_user_bitmap_debug::not chba vap or chba not up}");
        return OAL_FAIL;
    }
    hmac_user = mac_vap_get_hmac_user_by_addr(mac_vap, ps_bitmap_info->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    if (hmac_user == NULL) {
        return OAL_FAIL;
    }
    hmac_user->chba_user.user_bitmap_level = ps_bitmap_info->ps_bitmap;
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    user_ps_cfg.user_idx = hmac_user->st_user_base_info.us_assoc_id;
    user_ps_cfg.bitmap_level = hmac_user->chba_user.user_bitmap_level;
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CHBA_SET_USER_BITMAP_LEVEL, sizeof(chba_user_ps_cfg_stru),
        (uint8_t *)&user_ps_cfg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "{hmac_chba_set_user_bitmap_debug: fail to send event to dmac, ret[%d]}", ret);
        return ret;
    }

    oam_warning_log2(0, OAM_SF_CHBA, "{hmac_chba_set_user_bitmap_debug::update user[%d] bitmap[%d]}",
        hmac_user->st_user_base_info.us_assoc_id, hmac_user->chba_user.user_bitmap_level);
    return OAL_SUCC;
}
/*
 * 功能描述  : 调试命令接口, 入参为auto bitmap switch 0:固定bitmap | 1：bitmap自动更新
 */
uint32_t hmac_chba_auto_bitmap_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *params)
{
    uint32_t ret;
    hmac_chba_vap_stru *chba_vap_info = NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);

    chba_vap_info = hmac_chba_get_chba_vap_info(hmac_vap);
    if (chba_vap_info == NULL) {
        oam_error_log0(0, OAM_SF_CHBA, "{hmac_chba_auto_bitmap_debug::fail to get chba vap info!}");
        return OAL_FAIL;
    }
    chba_vap_info->auto_bitmap_switch = (uint8_t)params[0];

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_CHBA_AUTO_BITMAP_CMD, len, params);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_chba_auto_bitmap_debug:fail to send event to dmac, ret[%d]}", ret);
        return ret;
    }
    oam_warning_log1(0, OAM_SF_CHBA, "hmac_chba_auto_bitmap_debug::set auto_bitmap_switch[%d]",
        chba_vap_info->auto_bitmap_switch);
    return OAL_SUCC;
}

/*
 * 功能描述  : 调试命令接口, 入参为vap work bitmap。
 */
uint32_t hmac_chba_set_vap_bitmap_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *params)
{
    uint8_t vap_bitmap;
    hmac_chba_vap_stru *chba_vap_info = NULL;
    chba_vap_info = hmac_get_up_chba_vap_info();
    if (chba_vap_info == NULL) {
        oam_warning_log0(0, OAM_SF_CHBA, "hmac_chba_auto_bitmap_debug::not chba vap or chba not up");
        return OAL_FAIL;
    }
    vap_bitmap = *params;
    chba_vap_info->vap_bitmap_level = vap_bitmap;
    hmac_chba_sync_vap_bitmap_info(mac_vap, chba_vap_info);
    oam_warning_log1(0, OAM_SF_CHBA, "hmac_chba_auto_bitmap_debug::chba vap bitmap[%d]",
        chba_vap_info->vap_bitmap_level);
    return OAL_SUCC;
}

/*
 * 功能描述  : chba user低功耗配置初始化
 * 日    期  : 2021年03月22日
 * 作    者  : wifi
 */
void hmac_chba_user_ps_info_init(hmac_user_stru *hmac_user)
{
    hmac_chba_user_stru *chba_user_info = &(hmac_user->chba_user);

    chba_user_info->user_bitmap_level = CHBA_BITMAP_ALL_AWAKE_LEVEL;
    chba_user_info->power_down_count = 0;
    chba_user_info->power_up_count = 0;
}

/*
 * 功能描述  : 获取chba低功耗吞吐门限
 * 日    期  : 2021年03月22日
 * 作    者  : wifi7
 */
hmac_chba_ps_throught_thres_stru *hmac_chba_get_ps_throught_thres(hmac_vap_stru *hmac_vap)
{
    if ((hmac_vap == NULL) || (hmac_chba_vap_start_check(hmac_vap) == OAL_FALSE)) {
        return NULL;
    }
    return &g_chba_ps_throught_thres;
}

/*
 * 功能描述  : 初始化chba流量统计窗口
 * 日    期  : 2022年05月19日
 * 作    者  : wifi7
 */
void hmac_chba_throughput_statis_init(void)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_chba_vap_stru *chba_vap_info = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_chba_user_stru *chba_user_info = NULL;
    mac_user_stru *mac_user = NULL;
    oal_dlist_head_stru *head = NULL;
    uint16_t user_idx;

    hmac_device = hmac_res_get_mac_dev(0);
    if (hmac_device == NULL) {
        return;
    }
    /* chba vap未启动, 不处理 */
    hmac_vap = hmac_chba_find_chba_vap(hmac_device->pst_device_base_info);
    if (hmac_vap == NULL) {
        return;
    }
    chba_vap_info = hmac_chba_get_chba_vap_info(hmac_vap);
    if (chba_vap_info == NULL) {
        return;
    }

    /* 无user chba设备无需处理 */
    if (hmac_vap->st_vap_base_info.us_user_nums == 0) {
        return;
    }
    oal_spin_lock_bh(&hmac_vap->st_vap_base_info.st_cache_user_lock);
    /* 将全部chba user的吞吐统计窗口初始化 */
    for (user_idx = 0; user_idx < MAC_VAP_USER_HASH_MAX_VALUE; user_idx++) {
        oal_dlist_search_for_each(head, &(hmac_vap->st_vap_base_info.ast_user_hash[user_idx])) {
            mac_user = oal_dlist_get_entry(head, mac_user_stru, st_user_hash_dlist);
            if (mac_user == NULL) {
                continue;
            }
            hmac_user = mac_res_get_hmac_user(mac_user->us_assoc_id);
            if (hmac_user == NULL) {
                continue;
            }
            chba_user_info = &(hmac_user->chba_user);

            /* 清空统计数据 */
            memset_s(chba_user_info->throughput_info.tx_bytes_windows,
                sizeof(chba_user_info->throughput_info.tx_bytes_windows), 0,
                sizeof(chba_user_info->throughput_info.tx_bytes_windows));
            memset_s(chba_user_info->throughput_info.rx_bytes_windows,
                sizeof(chba_user_info->throughput_info.rx_bytes_windows), 0,
                sizeof(chba_user_info->throughput_info.rx_bytes_windows));
        }
    }
    oal_spin_unlock_bh(&hmac_vap->st_vap_base_info.st_cache_user_lock);
}

/*
 * 功能描述  : CHBA收beacon/pnf，检测到只有vap_bitmap信息变化，不更新整个topo只更新topo中的bitmap信息
 * 日    期  : 2022年05月23日
 * 作    者  : wifi7
 */
uint32_t hmac_chba_update_topo_bitmap_info_proc(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param)
{
    uint8_t device_id;
    chba_d2h_device_bitmap_info_stru *device_bitmap_info = NULL;
    mac_chba_per_device_info_stru *island_device_info = NULL;
    hmac_chba_per_device_id_info_stru *device_id_info = hmac_chba_get_device_id_info();

    if (mac_is_chba_mode(mac_vap) == OAL_FALSE) {
        oam_error_log0(0, OAM_SF_CHBA, "{hmac_chba_update_topo_bitmap_info_proc:not chba mode!}");
        return OAL_FAIL;
    }
    device_bitmap_info = (chba_d2h_device_bitmap_info_stru *)param;

    /* 更新活跃设备管理信息 */
    device_id = hmac_chba_one_dev_update_alive_dev_table(device_bitmap_info->mac_addr);
    if (device_id >= MAC_CHBA_MAX_DEVICE_NUM) {
        oam_error_log4(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "{hmac_chba_update_topo_bitmap_info_proc:device_id[%d] is invalid, mac_addr %02x:XX:XX:XX:%02x:%02x}",
            device_id, device_bitmap_info->mac_addr[MAC_ADDR_0], device_bitmap_info->mac_addr[MAC_ADDR_4],
            device_bitmap_info->mac_addr[MAC_ADDR_5]);
        return OAL_FAIL;
    }
    device_id_info[device_id].ps_bitmap = device_bitmap_info->device_bitmap;

    /* 将设备管理信息中的bitmap同步到岛设备信息 */
    island_device_info = hmac_chba_find_island_device_info(device_bitmap_info->mac_addr);
    if (island_device_info == NULL) {
        oam_error_log3(mac_vap->uc_vap_id, OAM_SF_CHBA,
            "{hmac_chba_update_topo_bitmap_info_proc:fail to find island_device, mac_addr %02x:XX:XX:XX:%02x:%02x}",
            device_bitmap_info->mac_addr[MAC_ADDR_0], device_bitmap_info->mac_addr[MAC_ADDR_4],
            device_bitmap_info->mac_addr[MAC_ADDR_5]);
        return OAL_FAIL;
    }
    island_device_info->ps_bitmap = device_id_info[device_id].ps_bitmap;

    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CHBA,
        "{hmac_chba_update_topo_bitmap_info_proc:update %02x:XX:XX:XX:%02x:%02x bitmap[0x%x]}",
        device_bitmap_info->mac_addr[MAC_ADDR_0], device_bitmap_info->mac_addr[MAC_ADDR_4],
        device_bitmap_info->mac_addr[MAC_ADDR_5], device_bitmap_info->device_bitmap);
    return OAL_SUCC;
}

/*
 * 函 数 名  :hmac_chba_update_throughput
 * 功能描述  : hid2d功耗模块每100ms更新流量模型
 *   日    期  : 2022年06月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_chba_update_throughput(void)
{
    hmac_chba_vap_stru *chba_vap_info = NULL;
    mac_vap_stru *mac_vap = NULL;
    hmac_chba_domain_merge_info *domain_merge = hmac_chba_get_domain_merge_info();

    chba_vap_info = hmac_get_up_chba_vap_info();
    if (chba_vap_info == NULL) {
        return;
    }
    mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(chba_vap_info->mac_vap_id);
    if (mac_vap == NULL) {
        return;
    }

    /* 无user不处理 */
    if (mac_vap->us_user_nums == 0) {
        return;
    }
    /* 配置auto_bitmap开关固定bitmap, 则不再运行流量模型 */
    if (chba_vap_info->auto_bitmap_switch == CHBA_BITMAP_FIX) {
        return;
    }
    /* 域合并期间, 不调整chba低功耗低级 */
    if (hmac_chba_get_domain_merge_flag(domain_merge) == OAL_TRUE) {
        return;
    }
    hmac_chba_update_throughput_info(chba_vap_info);
}

void hmac_chba_stat_user_rx_netbuf(oal_netbuf_stru *netbuf)
{
    hmac_user_stru *hmac_user = NULL;
    mac_rx_ctl_stru *rx_cb = NULL;

    rx_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_get_rx_cb_ta_user_id(rx_cb));
    hmac_stat_user_rx_netbuf(hmac_user, netbuf);
}
#endif
