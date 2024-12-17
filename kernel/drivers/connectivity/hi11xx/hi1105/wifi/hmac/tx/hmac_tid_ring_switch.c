/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac tid ring切换更新模块
 * 作    者 : wifi
 * 创建日期 : 2021年5月20日
 */

#include "hmac_tid_ring_switch.h"
#include "hmac_tid_sche.h"
#include "hmac_tx_switch_fsm.h"
#include "hmac_tx_switch.h"
#include "hmac_tx_data.h"
#include "hmac_vsp.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TID_RING_SWITCH_C

hmac_tid_ring_switch_stru g_tid_switch_list;

static inline hmac_tid_info_stru *hmac_ring_switch_tid_info_getter(void *entry)
{
    return oal_container_of(entry, hmac_tid_info_stru, tid_ring_switch_entry);
}

static inline void *hmac_ring_switch_entry_getter(hmac_tid_info_stru *tid_info)
{
    return &tid_info->tid_ring_switch_entry;
}

const hmac_tid_list_ops g_tid_switch_list_ops = {
    .tid_info_getter = hmac_ring_switch_tid_info_getter,
    .entry_getter = hmac_ring_switch_entry_getter,
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    .for_each_rcu = NULL,
#endif
};

void hmac_tid_ring_switch_init(void)
{
    hmac_tid_list_init(&g_tid_switch_list.tid_list, &g_tid_switch_list_ops);
    g_tid_switch_list.tid_fix_switch = OAL_FALSE;
    hmac_tx_switch_fsm_init();
}

void hmac_tid_ring_switch_deinit(void)
{
    hmac_tid_switch_list_clear();
}

uint8_t hmac_tid_host_ring_tx_check(hmac_tid_info_stru *tid_info, void *param)
{
    uint8_t ring_tx_mode = (uint8_t)oal_atomic_read(&tid_info->ring_tx_mode);
    uint8_t *result = param;

    if (ring_tx_mode == HOST_RING_TX_MODE || ring_tx_mode == D2H_SWITCHING_MODE) {
        *result = OAL_TRUE;
        return OAL_RETURN;
    } else if (ring_tx_mode == DEVICE_RING_TX_MODE || ring_tx_mode == H2D_SWITCHING_MODE ||
               ring_tx_mode == H2D_SWITCHING_MODE_STAGE2) {
        *result = OAL_FALSE;
    }
    return OAL_CONTINUE;
}

/* 功能：判断当前tid的ring mode，如果有一个host ring就不允许pcie切到低版本 */
uint8_t hmac_tid_ring_mode_judge(void)
{
    uint8_t result = OAL_FALSE;

    hmac_tid_list_for_each(&g_tid_switch_list.tid_list, hmac_tid_host_ring_tx_check, &result);
    return result;
}

/* 功能：tx ring tid级别切换统一入口 */
void hmac_tid_ring_switch(hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info, uint32_t mode)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->st_user_base_info.uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log1(0, 0, "{hmac_tx_ring_switch::NULL vap id[%d]!}", hmac_user->st_user_base_info.uc_vap_id);
        return;
    }

    if (hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP ||
        hmac_user->st_user_base_info.en_user_asoc_state != MAC_USER_STATE_ASSOC) {
        oam_warning_log2(hmac_vap->st_vap_base_info.uc_vap_id, 0,
            "{hmac_tid_ring_switch::user_state[%d],vap_state[%d] can't switch}",
            hmac_vap->st_vap_base_info.en_vap_state, hmac_user->st_user_base_info.en_user_asoc_state);
        return;
    }

    switch (mode) {
        case HOST_RING_TX_MODE:
            hmac_tx_mode_d2h_switch(tid_info);
            break;
        case DEVICE_RING_TX_MODE:
            hmac_tx_mode_h2d_switch(tid_info);
            break;
        case TX_MODE_DEBUG_DUMP:
            hmac_tx_mode_switch_dump_process(tid_info);
            break;
        default:
            break;
    }
}

static inline uint8_t hmac_tx_ring_switch_allowed(hmac_user_stru *hmac_user, uint8_t tidno)
{
#ifdef _PRE_WLAN_FEATURE_VSP
    return !hmac_is_vsp_tid(hmac_user, tidno);
#else
    return OAL_TRUE;
#endif
}

#define HMAC_TX_RING_SWITCH_FAIL_CNT_TH 100 /* ~10s */
OAL_STATIC OAL_INLINE uint8_t hmac_tx_ring_switch_fail_check(hmac_tid_info_stru *tid_info)
{
    if (oal_atomic_read(&tid_info->ring_tx_mode) == D2H_SWITCHING_MODE) {
        return ++tid_info->ring_switch_fail_cnt >= HMAC_TX_RING_SWITCH_FAIL_CNT_TH;
    }
    tid_info->ring_switch_fail_cnt = 0;

    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE void hmac_tx_ring_switch_fail_handle(hmac_tid_info_stru *tid_info)
{
    oam_error_log2(0, 0, "{hmac_tx_ring_switch_fail_handle::user[%d] tid[%d] d2h switch failed}",
        tid_info->user_index, tid_info->tid_no);

    oal_atomic_set(&tid_info->ring_tx_mode, DEVICE_RING_TX_MODE);
    tid_info->ring_switch_fail_cnt = 0;

    hmac_tid_schedule();
}

/*
功能：程序动态切换入口
 */
uint8_t hmac_tx_ring_switch(hmac_tid_info_stru *tid_info, void *param)
{
    uint16_t user_id = tid_info->user_index;
    hmac_user_stru *hmac_user = mac_res_get_hmac_user(user_id);

    if (hmac_user == NULL) {
        oam_warning_log1(0, 0, "{hmac_tx_ring_switch::NULL user id[%d]!}", user_id);
        return OAL_CONTINUE;
    }

    if (hmac_tx_ring_switch_allowed(hmac_user, tid_info->tid_no) == OAL_FALSE) {
        return OAL_CONTINUE;
    }

    if (hmac_tx_ring_switch_fail_check(tid_info) == OAL_TRUE) {
        hmac_tx_ring_switch_fail_handle(tid_info);
        return OAL_CONTINUE;
    }

    hmac_tid_ring_switch(hmac_user, tid_info, *(uint32_t*)param);

    return OAL_CONTINUE;
}

void hmac_tid_ring_switch_process(uint32_t tx_pps)
{
    uint32_t mode;
    if (!hmac_tx_ring_switch_enabled()) {
        return;
    }

    /* pps状态机状态切换 */
    hmac_tx_switch_fsm_handle_event(hmac_psm_tx_switch_throughput_to_event(tx_pps));

    /* 非Ring稳态不切换Ring实体 */
    mode = hmac_get_tx_switch_fsm_state();
    if ((mode != HOST_RING_TX_MODE) && (mode != DEVICE_RING_TX_MODE)) {
        return;
    }

    g_tid_switch_list.tid_list_count = hmac_tid_list_entry_count(&g_tid_switch_list.tid_list);
    hmac_tid_list_for_each(&g_tid_switch_list.tid_list, hmac_tx_ring_switch, &mode);
}

void hmac_tid_ring_switch_disable(hmac_user_stru *hmac_user)
{
    uint8_t tid_no;

    if (!hmac_tx_ring_switch_enabled()) {
        return;
    }

    for (tid_no = WLAN_TIDNO_BEST_EFFORT; tid_no < WLAN_TIDNO_BCAST; tid_no++) {
        hmac_tid_ring_switch_list_delete(&hmac_user->tx_tid_info[tid_no]);
    }
}
