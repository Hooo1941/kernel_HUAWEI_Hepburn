/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : HMAC PCIE动态切换
 * 作    者 :
 * 创建日期 : 2022年3月31日
 */
#include "hmac_dyn_pcie.h"
#include "hmac_tx_switch.h"
#include "hmac_tid_sche.h"
#include "hmac_tid_ring_switch.h"
#include "host_hal_ext_if.h"
#include "hmac_config.h"
#include "plat_pm_wlan.h"
#include "mac_ie.h"
#include "wlan_chip_i.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DYN_PCIE_C

static inline uint8_t hmac_dyn_pcie_sta(uint8_t pcie_switch)
{
    return (pcie_switch & BIT0);
}
static inline uint8_t hmac_dyn_pcie_gc(uint8_t pcie_switch)
{
    return (pcie_switch & BIT1);
}
uint32_t hmac_dyn_switch_up(mac_device_stru *mac_device);
uint32_t hmac_dyn_switch_down(mac_device_stru *mac_device);
uint32_t hmac_pcie_switch_handle(mac_device_stru *mac_device);
typedef uint32_t (*hmac_pcie_switch_fsm_func)(mac_device_stru *mac_device);
static hmac_pcie_switch_fsm_func g_hmac_pcie_switch_fsm[HMAC_PCIE_STATE_NUM][HMAC_PCIE_SWITCH_VOTE_BUTT] = {
    /* 1.0 */
    {
        NULL,
        NULL,
        hmac_dyn_switch_up
    },
    /* 2.0 */
    {
        NULL,
        hmac_dyn_switch_down,
        hmac_dyn_switch_up
    },
    /* 3.0 */
    {
        NULL,
        hmac_dyn_switch_down,
        NULL
    }
};
/* 功能：平台回调wifi的pcie_vote */
uint8_t hmac_get_pcie_switch_func(void)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);

    if (mac_device == NULL) {
        return 0;
    }
    return mac_device->pcie_switch_vote;
}

/* 功能：定时器保证pm_sleep_work多次check失败也能恢复业务状态 */
uint32_t hmac_pcie_up_timeout_handle(void *arg)
{
    uint32_t ret;
    mac_device_stru *mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log0(0, OAM_SF_CFG, "{hmac_pcie_up_timeout_handle::pm_check_timeout}");

    mutex_lock(&mac_device->pcie_switch_lock);
    ret = hmac_pcie_switch_handle(mac_device);
    mutex_unlock(&mac_device->pcie_switch_lock);

    return ret;
}

#define HMAC_PCIE_UP_RECOVER_TIME 100
/* 功能：dmac准备低功耗完毕，hmac放开pm_work */
uint32_t hmac_pcie_up_prepare_succ_handle(mac_vap_stru *mac_vap, uint8_t len, uint8_t *prepare_flag)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        return OAL_SUCC;
    }

    /* device构造完毕,重新置OAL_FALSE */
    /* 起一个定时器，超时后强制恢复避免无法满足低功耗check一直中断业务 */
    if (*prepare_flag == OAL_TRUE && mac_device->pcie_up_recover_timer.en_is_registerd == OAL_FALSE) {
        frw_timer_create_timer_m(&mac_device->pcie_up_recover_timer, hmac_pcie_up_timeout_handle,
            HMAC_PCIE_UP_RECOVER_TIME, mac_device, OAL_FALSE, OAM_MODULE_ID_HMAC, mac_vap->core_id);
    }
    mac_device->pm_work_disable = OAL_FALSE;

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_pcie_up_prepare_succ_handle::pm_work_disable [%d]}", mac_device->pm_work_disable);
    return OAL_SUCC;
}

/* 抛事件到dmac构造低功耗场景 */
void hmac_pcie_switch_up_prepare_event(mac_device_stru *mac_device, uint8_t prepare_flag)
{
    uint32_t ret;
    mac_vap_stru *mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->uc_cfg_vap_id);

    oam_warning_log0(0, OAM_SF_ANY, "{hmac_pcie_switch_up_prepare_event::H2D.}");
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_PCIE_UP_PREPARE, sizeof(uint8_t), (uint8_t *)&prepare_flag);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_pcie_switch_up_prepare_event::pcie_switch_up_prepare failed[%d].}", ret);
        return;
    }
}

/* 功能：切换结束后恢复dmac构造的低功耗条件 */
void hmac_pcie_switch_up_recover_event(mac_device_stru *mac_device, uint32_t switch_ret)
{
    uint32_t ret;
    uint8_t succ_flag = (switch_ret == OAL_SUCC);
    mac_vap_stru *mac_cfg_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->uc_cfg_vap_id);
    if (mac_cfg_vap != NULL) {
        ret = hmac_config_send_event(mac_cfg_vap, WLAN_CFGID_PCIE_UP_RECOVER, sizeof(uint8_t), (uint8_t *)&succ_flag);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_cfg_vap->uc_vap_id, OAM_SF_ANY,
                "{hmac_pcie_switch_up_recover_event::pcie_switch_up_recover failed[%d].}", ret);
        }
    }
}

/* 功能：pcie_up切换时平台调wifi，RECOVER：恢复 */
void hmac_pcie_switch_up_recover(uint32_t switch_ret)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        return;
    }
    if (switch_ret == OAL_SUCC && mac_device->pcie_up_recover_timer.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&mac_device->pcie_up_recover_timer);
    }

    /* 2. 刷回默认低功耗定时器时间 */
    wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);

    /* 3. 打开host tid调度 */
    oal_atomic_set(&g_tid_sche_forbid, OAL_FALSE);

    /* 4. tx mode恢复自适应 */
    g_tid_switch_list.tid_fix_switch = OAL_FALSE;

    /* 5. 下发事件通知dmac恢复 */
    hmac_pcie_switch_up_recover_event(mac_device, switch_ret);
    oam_warning_log1(0, OAM_SF_CFG, "{hmac_pcie_switch_up_recover::RECOVER because switch[%d].}", switch_ret);
}

#define HMAC_PM_PCIE_SWITCH_PREPARE 0
#define HMAC_PM_PCIE_SWITCH_RECOVER 1
/* 功能：pcie切换时平台调wifi，PREPARE：禁HOST中断；RECOVER：恢复中断 */
void hmac_pcie_switch_irq_mask_notify(uint8_t action)
{
    wlan_pm_wakeup_dev_pcie_switch();
    if (action == HMAC_PM_PCIE_SWITCH_PREPARE) {
        if (g_hal_common_ops->host_clear_host_mac_irq_mask) {
            g_hal_common_ops->host_clear_host_mac_irq_mask();
        }
    } else if (action == HMAC_PM_PCIE_SWITCH_RECOVER) {
        if (g_hal_common_ops->host_recover_host_mac_irq_mask) {
            g_hal_common_ops->host_recover_host_mac_irq_mask();
        }
    }
}

#define PCIE_VOTE
/* 功能：触发pcie UP切换，准备工作，目的是构造低功耗场景 */
void hmac_dyn_pcie_vote_to_up(mac_device_stru *mac_device, uint8_t prepare_flag)
{
    if (mac_device->pcie_switch_vote == HMAC_PCIE_SWITCH_UP) {
        return;
    }
    if (mac_device->cur_pcie_state == g_wlan_spec_cfg->pcie_speed) {
        /* 此次切换动作之前已经到达支持的最大GEN，则本次up切换之前，直接结束之前可能存在的down操作 */
        mac_device->pcie_switch_vote = HMAC_PCIE_SWITCH_IDLE;
        return;
    }

    /* 1. 禁止平台低功耗，等待device构造完毕后再重新值OAL_FALSE */
    mac_device->pm_work_disable = OAL_TRUE;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_dyn_pcie_vote_to_up:START prepare[%d].}", prepare_flag);
    mac_device->pcie_switch_vote = HMAC_PCIE_SWITCH_UP;

     /* 2. 刷低功耗定时器时间 */
    wlan_pm_set_timeout(1);

    /* 3. 禁止host tid调度 */
    /* 4. 下发pcie切换时间到dmac，准备device的低功耗条件：NULL帧节能；ring固定在device；关MAC PA；停止软件调度 */
    if (prepare_flag) {
        oal_atomic_set(&g_tid_sche_forbid, OAL_TRUE);
        hmac_pcie_switch_up_prepare_event(mac_device, prepare_flag);
    } else {
        /* 不需要device构造,重新置OAL_FALSE */
        mac_device->pm_work_disable = OAL_FALSE;
    }
}
void hmac_dyn_pcie_vote_to_down(mac_device_stru *mac_device)
{
    if (mac_device->pcie_switch_vote != HMAC_PCIE_SWITCH_IDLE) {
        return;
    }

    if (mac_device->cur_pcie_state == g_wlan_spec_cfg->pcie_speed - 1) {
        mac_device->pcie_switch_vote = HMAC_PCIE_SWITCH_IDLE;
        return;
    }
    mac_device->pcie_switch_vote = HMAC_PCIE_SWITCH_DOWN;
}

#define HMAC_PCIE_SWITCH_UP_PPS_TH 80000
#define HMAC_PCIE_SWITCH_DOWN_PPS_TH 50000
/* 功能：根据ring_mode来进行pcie切换 */
void hmac_pcie_switch_vote_by_pps(uint32_t trx_total_pps)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    if (mac_device == NULL || !mac_dyn_pcie_switch_thpt()) {
        return;
    }
    mutex_lock(&mac_device->pcie_switch_lock);
    /* 多vap时不考虑切换 */
    if (mac_device_calc_up_vap_num(mac_device) > 1) {
        mutex_unlock(&mac_device->pcie_switch_lock);
        return;
    }
    if (g_dyn_pcie_switch & BIT2) {
        if (mac_device->cur_pcie_state != g_wlan_spec_cfg->pcie_speed - 1 &&
            trx_total_pps < HMAC_PCIE_SWITCH_DOWN_PPS_TH) {
            hmac_dyn_pcie_vote_to_down(mac_device);
        } else if (mac_device->cur_pcie_state != g_wlan_spec_cfg->pcie_speed &&
            trx_total_pps > HMAC_PCIE_SWITCH_UP_PPS_TH) {
            hmac_dyn_pcie_vote_to_up(mac_device, OAL_TRUE);
        }
    } else if (g_dyn_pcie_switch & BIT3) {
        /* 只要有一个 host ring 就不允许切到低版本pcie */
        if (mac_device->cur_pcie_state != g_wlan_spec_cfg->pcie_speed && hmac_tid_ring_mode_judge()) {
            hmac_dyn_pcie_vote_to_up(mac_device, OAL_TRUE);
        } else if (mac_device->cur_pcie_state != g_wlan_spec_cfg->pcie_speed - 1 && !hmac_tid_ring_mode_judge()) {
            hmac_dyn_pcie_vote_to_down(mac_device);
        }
    }

    mutex_unlock(&mac_device->pcie_switch_lock);
}

/* 功能：入网时，业务up_vap状态判决，单vap看cur_pcie_probe，多vap看pre_pcie_probe */
void hmac_dyn_pcie_vote_assoc(mac_device_stru *mac_device, mac_vap_stru *mac_vap, wlan_pcie_probe_state pre_pcie_probe,
    wlan_pcie_probe_state cur_pcie_probe)
{
    uint32_t vap_num;
    mutex_lock(&mac_device->pcie_switch_lock);
    /* 投票需要注意，IDLE票不投，避免本次操作刷掉上次未完成的切换操作 */
    /* 注：多于2VAP的场景：在本vap来之前我们就已经是3.0了 */
    vap_num = mac_device_calc_up_vap_num(mac_device);
    if (vap_num == 1 && cur_pcie_probe == WLAN_PCIE_PROBE_LOW) {
        hmac_dyn_pcie_vote_to_down(mac_device);
    } else if (vap_num == 2) { // 2: 2VAP
        if (pre_pcie_probe == WLAN_PCIE_PROBE_LOW) {
            hmac_dyn_pcie_vote_to_up(mac_device, OAL_TRUE);
        }
    }
    mutex_unlock(&mac_device->pcie_switch_lock);
    oam_warning_log3(0, OAM_SF_CFG, "{hmac_dyn_pcie_vote_assoc::cur_limit[%d]pre_limit[%d]pcie_switch_vote[%d].}",
        cur_pcie_probe, pre_pcie_probe, mac_device->pcie_switch_vote);
}
/* 功能：退网时，业务up_vap状态判决：无业务VAP看cur_pcie_probe；单vap看pre_pcie_probe */
void hmac_dyn_pcie_vote_disassoc(mac_device_stru *mac_device,
    wlan_pcie_probe_state pre_pcie_probe, wlan_pcie_probe_state cur_pcie_probe)
{
    uint8_t vap_num;

    mutex_lock(&mac_device->pcie_switch_lock);
    vap_num = mac_device_calc_up_vap_num(mac_device);
    /* 投票需要注意，IDLE票不投，避免本次操作刷掉上次未完成的切换操作 */
    if (vap_num == 0 && cur_pcie_probe == WLAN_PCIE_PROBE_LOW) {
        /* 1. 退网后无设备 */
        hmac_dyn_pcie_vote_to_up(mac_device, OAL_FALSE);
    } else if (vap_num == 1) {
        /* 多vap场景的退网后仅剩一个 */
        if (pre_pcie_probe == WLAN_PCIE_PROBE_LOW) {
            hmac_dyn_pcie_vote_to_down(mac_device);
        }
    }
    mutex_unlock(&mac_device->pcie_switch_lock);
    oam_warning_log3(0, OAM_SF_CFG, "{hmac_dyn_pcie_vote_disassoc::cur_limit[%d]pre_limit[%d]pcie_switch_vote[%d].}",
        cur_pcie_probe, pre_pcie_probe, mac_device->pcie_switch_vote);
}

/* 功能：给当前入网的vap进行_pcie_probe赋值，该值有效期与该vap生命周期一致，退网时刷默认值 */
void hmac_pcie_set_limit(mac_device_stru *mac_device, mac_vap_stru *mac_vap, mac_user_stru *mac_user)
{
    wlan_pcie_probe_state temp_limit;

    temp_limit = wlan_chip_get_pcie_probe_state(mac_user);
    if (is_legacy_sta(mac_vap) && hmac_dyn_pcie_sta(mac_get_dyn_pcie_switch())) {
        mac_device->sta_pcie_probe = temp_limit;
    } else if (is_p2p_cl(mac_vap) && hmac_dyn_pcie_gc(mac_get_dyn_pcie_switch())) {
        mac_device->gc_pcie_probe = temp_limit;
    }
    oam_warning_log3(0, OAM_SF_CFG, "{hmac_pcie_set_limit::sta_pcie_probe[%d]gc_pcie_probe[%d]ini[0x%x].}",
        mac_device->sta_pcie_probe, mac_device->gc_pcie_probe, mac_get_dyn_pcie_switch());
}

/* 功能：STA设备入网时触发pcie动态切换判决 */
void hmac_process_assoc_dyn_pcie_vote_sta(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    wlan_pcie_probe_state pre_pcie_probe, cur_pcie_probe;
    mac_device_stru *mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        return;
    }

    hmac_pcie_set_limit(mac_device, &hmac_vap->st_vap_base_info, &hmac_user->st_user_base_info);
    if (is_legacy_sta(&hmac_vap->st_vap_base_info)) {
        cur_pcie_probe = mac_device->sta_pcie_probe;
        pre_pcie_probe = mac_device->gc_pcie_probe;
    } else {
        pre_pcie_probe = mac_device->sta_pcie_probe;
        cur_pcie_probe = mac_device->gc_pcie_probe;
    }

    hmac_dyn_pcie_vote_assoc(mac_device, &hmac_vap->st_vap_base_info, pre_pcie_probe, cur_pcie_probe);
    oam_warning_log3(0, OAM_SF_CFG, "{dyn_pcie_vote_sta::sta_pcie_probe[%d]gc_pcie_probe[%d]pcie_switch_vote[%d].}",
        mac_device->sta_pcie_probe, mac_device->gc_pcie_probe, mac_device->pcie_switch_vote);
}

/* 功能：非STA设备入网时触发pcie动态切换判决 */
void hmac_process_assoc_dyn_pcie_vote_nonsta(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    wlan_pcie_probe_state pre_pcie_probe, cur_pcie_probe;
    mac_device_stru *mac_device = mac_res_get_dev(0);

    if (mac_device == NULL) {
        return;
    }

    pre_pcie_probe = mac_device->sta_pcie_probe | mac_device->gc_pcie_probe;
    cur_pcie_probe = WLAN_PCIE_PROBE_DEFAULT;

    hmac_dyn_pcie_vote_assoc(mac_device, &hmac_vap->st_vap_base_info, pre_pcie_probe, cur_pcie_probe);
    oam_warning_log3(0, OAM_SF_CFG, "{dyn_pcie_vote_nonsta::sta_pcie_probe[%d]gc_pcie_probe[%d]pcie_switch_vote[%d].}",
        mac_device->sta_pcie_probe, mac_device->gc_pcie_probe, mac_device->pcie_switch_vote);
}

/* 功能：入网操作的PCIE动态切换，协商阶段投票操作
   单up_vap，且STAUT，且协商后的交互帧最大nss为2
 */
void hmac_process_assoc_dyn_pcie_vote(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    if (!mac_get_dyn_pcie_switch()) {
        return;
    }
    oam_warning_log1(0, OAM_SF_CFG, "{hmac_process_assoc_dyn_pcie_vote::vap_id[%d].}",
        hmac_vap->st_vap_base_info.uc_vap_id);
    /* STA、GC、GO/APUT的VAP入网时，会触发不同的判决逻辑 */
    if (is_legacy_sta(&hmac_vap->st_vap_base_info) || is_p2p_cl(&hmac_vap->st_vap_base_info)) {
        hmac_process_assoc_dyn_pcie_vote_sta(hmac_vap, hmac_user);
    } else {
        hmac_process_assoc_dyn_pcie_vote_nonsta(hmac_vap, hmac_user);
    }
}

/* 功能：STA去关联时进行pcie动态切换判决 */
void hmac_user_del_dyn_pcie_vote(mac_vap_stru *mac_vap)
{
    wlan_pcie_probe_state pre_pcie_probe, cur_pcie_probe;
    mac_device_stru *mac_device = NULL;

    if (!mac_get_dyn_pcie_switch() || (!is_legacy_sta(mac_vap) && !is_p2p_cl(mac_vap))) {
        return;
    }
    mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        return;
    }
    if (is_legacy_sta(mac_vap)) {
        cur_pcie_probe = mac_device->sta_pcie_probe;
        pre_pcie_probe = mac_device->gc_pcie_probe;
        mac_device->sta_pcie_probe = WLAN_PCIE_PROBE_DEFAULT;
    } else {
        pre_pcie_probe = mac_device->sta_pcie_probe;
        cur_pcie_probe = mac_device->gc_pcie_probe;
        mac_device->gc_pcie_probe = WLAN_PCIE_PROBE_DEFAULT;
    }
    hmac_dyn_pcie_vote_disassoc(mac_device, pre_pcie_probe, cur_pcie_probe);
    oam_warning_log3(0, OAM_SF_CFG, "{user_del_dyn_pcie_vote::sta_pcie_probe[%d]gc_pcie_probe[%d]switch_vote[%d].}",
        mac_device->sta_pcie_probe, mac_device->gc_pcie_probe, mac_device->pcie_switch_vote);
}

/* 功能：PCIE动态切换逻辑，down_vap时可能触发切换 */
void hmac_vap_down_pcie_vote(mac_vap_stru *mac_vap, mac_device_stru *mac_device)
{
    wlan_pcie_probe_state pre_pcie_probe, cur_pcie_probe;

    if (!mac_get_dyn_pcie_switch() || is_legacy_sta(mac_vap) || is_p2p_cl(mac_vap)) {
        return;
    }
    pre_pcie_probe = mac_device->sta_pcie_probe | mac_device->gc_pcie_probe;
    cur_pcie_probe = WLAN_PCIE_PROBE_DEFAULT;
    hmac_dyn_pcie_vote_disassoc(mac_device, pre_pcie_probe, cur_pcie_probe);
    oam_warning_log3(0, OAM_SF_CFG, "{vap_down_pcie_vote::sta_pcie_probe[%d]gc_pcie_probe[%d]pcie_switch_vote[%d].}",
        mac_device->sta_pcie_probe, mac_device->gc_pcie_probe, mac_device->pcie_switch_vote);
}

/* 功能：PCIE DOWN */
uint32_t hmac_dyn_switch_down(mac_device_stru *mac_device)
{
    uint32_t ret = OAL_SUCC;
    hmac_pcie_state target_pcie_stat = g_wlan_spec_cfg->pcie_speed - 1;

    /* 1. wifi禁host中断 */
    hmac_pcie_switch_irq_mask_notify(HMAC_PM_PCIE_SWITCH_PREPARE);

    /* 2. 切换pcieGENX */
    if (wlan_pm_pcie_switch(target_pcie_stat) == OAL_SUCC) {
        mac_device->cur_pcie_state = target_pcie_stat;
        mac_device->pcie_switch_vote = HMAC_PCIE_SWITCH_IDLE;
    } else {
        oam_warning_log0(0, OAM_SF_PWR, "hmac_dyn_switch_down FAIL!");
        ret = OAL_FAIL;
    }

    oam_warning_log2(0, OAM_SF_PWR, "hmac_dyn_switch_down::PCIE_GENX[%d] result[%d]", mac_device->cur_pcie_state, ret);
    /* 3. wifi恢复host中断 */
    hmac_pcie_switch_irq_mask_notify(HMAC_PM_PCIE_SWITCH_RECOVER);
    return ret;
}

/* 功能：PCIE UP */
uint32_t hmac_dyn_switch_up(mac_device_stru *mac_device)
{
    uint32_t ret = OAL_SUCC;
    hmac_pcie_state target_pcie_stat = g_wlan_spec_cfg->pcie_speed;

    /* 1. wifi禁host中断 */
    hmac_pcie_switch_irq_mask_notify(HMAC_PM_PCIE_SWITCH_PREPARE);

    /* 2. 切换pcieGENX */
    if (wlan_pm_pcie_switch(target_pcie_stat) == OAL_SUCC) {
        mac_device->cur_pcie_state = target_pcie_stat;
        mac_device->pcie_switch_vote = HMAC_PCIE_SWITCH_IDLE;
    } else {
        oam_warning_log0(0, OAM_SF_PWR, "hmac_dyn_switch_up FAIL!");
        ret = OAL_FAIL;
    }

    oam_warning_log2(0, OAM_SF_PWR, "hmac_dyn_switch_up::PCIE_GENX[%d] result[%d]", mac_device->cur_pcie_state, ret);
    /* 3. wifi恢复host中断 */
    hmac_pcie_switch_irq_mask_notify(HMAC_PM_PCIE_SWITCH_RECOVER);

    hmac_pcie_switch_up_recover(ret);
    return ret;
}

uint32_t hmac_pcie_switch_handle(mac_device_stru *mac_device)
{
    hmac_pcie_switch_vote switch_vote;
    hmac_pcie_state cur_state;
    uint32_t ret;

    switch_vote = mac_device->pcie_switch_vote;
    cur_state = mac_device->cur_pcie_state;
    if (switch_vote == HMAC_PCIE_SWITCH_IDLE || switch_vote >= HMAC_PCIE_SWITCH_VOTE_BUTT ||
        cur_state > HMAC_PCIE_STATE_3) {
        return OAL_FAIL;
    }

	/* check vote && state */
    if (g_hmac_pcie_switch_fsm[cur_state][switch_vote]) {
        ret = g_hmac_pcie_switch_fsm[cur_state][switch_vote](mac_device);
    } else {
        oam_warning_log2(0, OAM_SF_PWR, "hmac_pcie_switch_handle::cur_state[%d],switch_vote",
            cur_state, switch_vote);
        return OAL_FAIL;
    }
    return ret;
}

uint32_t hmac_pcie_callback_fsm_func(void)
{
    uint32_t ret;
    mac_device_stru *mac_device = mac_res_get_dev(0);
    if (mac_device == NULL || !(mac_get_dyn_pcie_switch() | mac_dyn_pcie_switch_thpt())) {
        return OAL_FAIL;
    }
    mutex_lock(&mac_device->pcie_switch_lock);
    ret = hmac_pcie_switch_handle(mac_device);
    mutex_unlock(&mac_device->pcie_switch_lock);

    return ret;
}
