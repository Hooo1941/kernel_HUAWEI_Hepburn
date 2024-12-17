/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 调频功能
 * 作    者 :
 * 创建日期 : 2015年12月10日
 */

#include "oal_net.h"
#include "oal_types.h"
#include "oam_ext_if.h"
#include "frw_task.h"
#include "mac_vap.h"
#include "mac_resource.h"
#include "hmac_vap.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_ext_if.h"
#include "hmac_blockack.h"
#include "hmac_tx_data.h"
#include "host_hal_ext_if.h"
#include "hmac_stat.h"
#include "hmac_config.h"
#include "hmac_tx_switch.h"
#include "hmac_tx_sche_switch_fsm.h"
#include "hmac_tx_switch_fsm.h"
#include "hmac_tid_sche.h"
#include "hmac_tid_update.h"
#include "hmac_tx_complete.h"
#include "hmac_dyn_pcie.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#include "hmac_chba_interface.h"
#include "hmac_chba_ps.h"
#endif
#include "external/oal_pm_qos_interface.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt.h"
#endif
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#include "hmac_tcp_ack_buf.h"
#endif
#include "hmac_rx_complete.h"
#include "hmac_wifi_delay.h"
#include "hmac_dfx.h"
#include "host_hal_access_mgmt.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_AUTO_ADJUST_FREQ_C

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
#define INVALID_PKMODE_TH (0xFFFFFFFF)
#endif
#define HMAC_TX_THROUGHPUT_OFFSET_7 7
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef WIN32
uint32_t jiffies;
#endif

freq_lock_control_stru g_freq_lock_control = { 0 };
/* Wi-Fi驱动收发负载识别数据区 */
freq_wifi_load_stru g_st_wifi_load = { 0 };
oal_bool_enum_uint8 g_lowpower_strategy_swtich = OAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
oal_bool_enum_uint8 g_en_pk_mode_swtich = OAL_TRUE;

/* pk mode的判定的门限，以字节为单位 */
/*
PK模式门限基数:
{(单位Mbps)  20M     40M    80M   160M   80+80M
lagency:    {valid, valid, valid, valid, valid},   (基础协议模式没有pk mode )
HT:         {62, 123, valid, valid, valid},
VHT:        {77, 167, 329, 660, 660},
HE:         {110, 205, 460, 750, 750},
};

PK模式二级门限:
高档位门限: g_st_pk_mode_high_th_table = PK模式门限基数 * 70% *1024 *1024 /8  (单位字节)
低档位门限: g_st_pk_mode_low_th_table  = PK模式门限基数 * 30% *1024 *1024 /8  (单位字节)

 */
uint32_t g_st_pk_mode_high_th_table[WLAN_PROTOCOL_CAP_BUTT][WLAN_BW_CAP_BUTT] = {
    { INVALID_PKMODE_TH, INVALID_PKMODE_TH, INVALID_PKMODE_TH, INVALID_PKMODE_TH, INVALID_PKMODE_TH },
    { 45,                110,               INVALID_PKMODE_TH, INVALID_PKMODE_TH, INVALID_PKMODE_TH },
    { 60,                130,               280,               562,               562 },
#ifdef _PRE_WLAN_FEATURE_11AX
    { 65,                140,               300,               525,               525 }
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */

};

uint32_t g_st_pk_mode_low_th_table[WLAN_PROTOCOL_CAP_BUTT][WLAN_BW_CAP_BUTT] = {
    { INVALID_PKMODE_TH, INVALID_PKMODE_TH, INVALID_PKMODE_TH, INVALID_PKMODE_TH, INVALID_PKMODE_TH },
    { 30,                80,                INVALID_PKMODE_TH, INVALID_PKMODE_TH, INVALID_PKMODE_TH },
    { 40,                100,               240,               400,               400 },
#ifdef _PRE_WLAN_FEATURE_11AX
    { 50,                110,               250,               450,               450 }
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */
};
#endif

uint32_t g_orig_cpu_min_freq = 0;
uint32_t g_orig_cpu_max_freq = 0;
uint32_t g_orig_ddr_min_freq = 0;
uint32_t g_orig_ddr_max_freq = 0;

#if !defined(WIN32) || defined(_PRE_WINDOWS_SUPPORT)
#define HOST_NO_BA_FREQ_LEVEL_BUTT 4
/* 由定制化进行初始化 */
host_speed_freq_level_stru g_host_speed_freq_level[HOST_SPEED_FREQ_LEVEL_BUTT] = {
    /* pps门限                   CPU主频下限                     DDR主频下限 */
    { PPS_VALUE_0, CPU_MIN_FREQ_VALUE_0, DDR_MIN_FREQ_VALUE_0 },
    { PPS_VALUE_1, CPU_MIN_FREQ_VALUE_1, DDR_MIN_FREQ_VALUE_1 },
    { PPS_VALUE_2, CPU_MIN_FREQ_VALUE_2, DDR_MIN_FREQ_VALUE_2 },
    { PPS_VALUE_3, CPU_MIN_FREQ_VALUE_3, DDR_MIN_FREQ_VALUE_3 },
};
host_speed_freq_level_stru g_host_no_ba_freq_level[HOST_NO_BA_FREQ_LEVEL_BUTT] = {
    /* pps门限                        CPU主频下限                      DDR主频下限 */
    { NO_BA_PPS_VALUE_0, CPU_MIN_FREQ_VALUE_0, DDR_MIN_FREQ_VALUE_0 },
    { NO_BA_PPS_VALUE_1, CPU_MIN_FREQ_VALUE_1, DDR_MIN_FREQ_VALUE_1 },
    { NO_BA_PPS_VALUE_2, CPU_MIN_FREQ_VALUE_2, DDR_MIN_FREQ_VALUE_2 },
    { NO_BA_PPS_VALUE_3, CPU_MIN_FREQ_VALUE_2, DDR_MIN_FREQ_VALUE_2 },
};
device_speed_freq_level_stru g_device_speed_freq_level[] = {
    /* device主频类型 */
    { FREQ_IDLE },
    { FREQ_MIDIUM },
    { FREQ_HIGHER },
    { FREQ_HIGHEST },
};

struct pm_qos_request *g_pst_wifi_auto_ddr = NULL;

#else
host_speed_freq_level_stru g_host_speed_freq_level[] = {
    /* pps门限                   CPU主频下限                     DDR主频下限 */
    { PPS_VALUE_0, CPU_MIN_FREQ_VALUE_0, DDR_MIN_FREQ_VALUE_0 },
    { PPS_VALUE_1, CPU_MIN_FREQ_VALUE_1, DDR_MIN_FREQ_VALUE_1 },
    { PPS_VALUE_2, CPU_MIN_FREQ_VALUE_2, DDR_MIN_FREQ_VALUE_2 },
    { PPS_VALUE_3, CPU_MIN_FREQ_VALUE_3, DDR_MIN_FREQ_VALUE_3 },
};
device_speed_freq_level_stru g_device_speed_freq_level[] = {
    /* device主频类型 */
    { FREQ_IDLE },
    { FREQ_MIDIUM },
    { FREQ_HIGHEST },
    { FREQ_HIGHEST },
};

#define mutex_init(mux)
#define mutex_lock(mux)
#define mutex_unlock(mux)
#define spin_lock_init(mux)
#define mutex_destroy(mux)
#define spin_unlock_bh(mux)
#endif

/*
 * 函 数 名  : hmac_set_auto_freq_mod
 * 功能描述  : 设置auto freq使能
 * 1.日    期  : 2015年9月14日
  *   修改内容  : 新生成函数
 */
uint8_t hmac_set_auto_freq_mod(oal_freq_lock_enum uc_freq_enable)
{
    g_freq_lock_control.uc_lock_mod = uc_freq_enable;

    /* 设置device是否使能 */
    if (uc_freq_enable == FREQ_LOCK_ENABLE) {
        /* 初始频率为idle */
        g_freq_lock_control.uc_curr_lock_level = FREQ_IDLE;
        /* 期望频率为idle */
        g_freq_lock_control.uc_req_lock_level = FREQ_IDLE;
        /* WIFI打开时,定时器参数更新 */
        g_freq_lock_control.pre_jiffies = jiffies;
    } else {
        /* 关闭WIFI时,将CPU频率调整为默认值 */
        mutex_lock(&g_freq_lock_control.st_lock_freq_mtx);
        mutex_unlock(&g_freq_lock_control.st_lock_freq_mtx);

        g_freq_lock_control.uc_curr_lock_level = FREQ_BUTT;
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_set_auto_freq_mod: freq,min[%d]max[%d]!}",
                         g_orig_cpu_min_freq, g_orig_cpu_max_freq);
    }

    return OAL_SUCC;
}

void hmac_adjust_set_freq(void)
{
    uint8_t uc_req_lock_level = g_freq_lock_control.uc_req_lock_level;

    /* 相同则不需要配置系统文件 */
    if (uc_req_lock_level == g_freq_lock_control.uc_curr_lock_level) {
        return;
    }

    oam_warning_log2(0, OAM_SF_PWR, "{hmac_adjust_set_freq: freq [%d]to[%d]}",
                     g_freq_lock_control.uc_curr_lock_level, uc_req_lock_level);

    g_freq_lock_control.uc_curr_lock_level = uc_req_lock_level;
}

uint8_t hmac_get_freq_level(uint32_t speed)  // 根据吞吐率计算目标主频等级
{
    uint8_t level_idx = 0;

    if (hmac_is_device_ba_setup()) {
        if (speed <= g_host_speed_freq_level[HOST_SPEED_FREQ_LEVEL_1].speed_level) {
            level_idx = HMAC_FREQ_LEVEL_0;
        } else if ((speed > g_host_speed_freq_level[HOST_SPEED_FREQ_LEVEL_1].speed_level) &&
                   (speed <= g_host_speed_freq_level[HOST_SPEED_FREQ_LEVEL_2].speed_level)) {
            level_idx = HMAC_FREQ_LEVEL_1;
        } else if ((speed > g_host_speed_freq_level[HOST_SPEED_FREQ_LEVEL_2].speed_level) &&
                   (speed <= g_host_speed_freq_level[HOST_SPEED_FREQ_LEVEL_3].speed_level)) {
            level_idx = HMAC_FREQ_LEVEL_2;
        } else {
            level_idx = HMAC_FREQ_LEVEL_3;
        }
    } else {
#ifndef WIN32
        if (speed <= g_host_no_ba_freq_level[HOST_SPEED_FREQ_LEVEL_1].speed_level) {
            level_idx = HMAC_FREQ_LEVEL_0;
        } else if ((speed > g_host_no_ba_freq_level[HOST_SPEED_FREQ_LEVEL_1].speed_level) &&
                   (speed <= g_host_no_ba_freq_level[HOST_SPEED_FREQ_LEVEL_2].speed_level)) {
            level_idx = HMAC_FREQ_LEVEL_1;
        } else if ((speed > g_host_no_ba_freq_level[HOST_SPEED_FREQ_LEVEL_2].speed_level) &&
                   (speed <= g_host_no_ba_freq_level[HOST_SPEED_FREQ_LEVEL_3].speed_level)) {
            level_idx = HMAC_FREQ_LEVEL_2;
        } else {
            level_idx = HMAC_FREQ_LEVEL_3;
        }
#endif
    }
    return level_idx;
}
static void hmac_host_rx_pps_update(uint32_t host_ring_rx_pps)
{
    uint32_t ret;
    mac_host_rx_pps_stru rx_pps_stats;
    mac_vap_stru *cfg_vap = mac_res_get_mac_vap(0);
    rx_pps_stats.rx_pps = host_ring_rx_pps;
    ret = hmac_config_send_event(cfg_vap, WLAN_CFGID_HOST_RX_PPS,
        sizeof(mac_host_rx_pps_stru), (uint8_t *)(&rx_pps_stats));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_M2S,
            "{hmac_host_rx_pps_update::hmac_config_send_event failed[%d].}", ret);
    }
    return;
}

/*
 * 函 数 名  : hmac_pps_calc
 * 功能描述  : HOST pps统计
 * 1.日    期  : 2020年02月14日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_pps_calc(void)
{
    uint32_t cur_jiffies;
    uint32_t sdio_dur_ms;  // 时间
    uint32_t trx_total;
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t host_ring_rx_packets;
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();

    tx_packets = (uint32_t)oal_atomic_read(&hmac_device_stats->tx_packets);
    rx_packets = (uint32_t)oal_atomic_read(&hmac_device_stats->rx_packets);
    host_ring_rx_packets = (uint32_t)oal_atomic_read(&hmac_device_stats->host_ring_rx_packets);
    oal_atomic_set(&hmac_device_stats->tx_packets, 0);
    oal_atomic_set(&hmac_device_stats->rx_packets, 0);
    oal_atomic_set(&hmac_device_stats->host_ring_rx_packets, 0);
    hmac_device_stats->total_tx_packets += tx_packets;
    hmac_device_stats->total_rx_packets += rx_packets;

    cur_jiffies = jiffies;  // jiffies是Linux内核中的一个全局变量，用来记录自系统启动以来产生的节拍的总数
    sdio_dur_ms = oal_jiffies_to_msecs(cur_jiffies - g_freq_lock_control.pre_jiffies);
    g_freq_lock_control.pre_jiffies = cur_jiffies;

    /* 如果当前统计时间不足定时器周期的一半,会导致统计PPS值偏大返回 */
    if ((sdio_dur_ms == 0) || (sdio_dur_ms < (WLAN_FREQ_TIMER_PERIOD >> 1))) {
        return;
    }

    // 计算PPS
    trx_total = tx_packets + rx_packets;
    g_freq_lock_control.total_sdio_pps = (trx_total * HMAC_FREQ_S_TRANS_TO_MS) / sdio_dur_ms;
    g_freq_lock_control.tx_pps = (tx_packets * HMAC_FREQ_S_TRANS_TO_MS) / sdio_dur_ms;
    g_freq_lock_control.rx_pps = (rx_packets * HMAC_FREQ_S_TRANS_TO_MS) / sdio_dur_ms;
    g_freq_lock_control.host_rx_pps = (host_ring_rx_packets * HMAC_FREQ_S_TRANS_TO_MS) / sdio_dur_ms;
    if ((hal_master_is_in_ddr_rx() == OAL_TRUE) || (hal_slave_is_in_ddr_rx() == OAL_TRUE)) {
        hmac_host_rx_pps_update(g_freq_lock_control.host_rx_pps);
    }
}

/*
 * 函 数 名  : hmac_adjust_freq
 * 功能描述  : 设置auto freq使能
 * 1.日    期  : 2015年9月14日
  *   修改内容  : 新生成函数
 */
void hmac_adjust_freq(void)
{
    uint8_t uc_req_lock_level;  // 目标主频等级

    /* host调频是否使能 */
    if (g_freq_lock_control.uc_lock_mod == FREQ_LOCK_DISABLE) {
        return;
    }

    g_freq_lock_control.uc_req_lock_level = hmac_get_freq_level(g_freq_lock_control.total_sdio_pps);

    mutex_lock(&g_freq_lock_control.st_lock_freq_mtx);
    uc_req_lock_level = g_freq_lock_control.uc_req_lock_level;
    if (uc_req_lock_level == g_freq_lock_control.uc_curr_lock_level) { // 目标主频和当前主频相同，不需要调频
        g_freq_lock_control.adjust_count = 0;
        mutex_unlock(&g_freq_lock_control.st_lock_freq_mtx);
        return;
    }

    if (uc_req_lock_level < g_freq_lock_control.uc_curr_lock_level) {
        /* 连续MAX_DEGRADE_FREQ_TIME_THRESHOLD后才降频，保证性能 */
        g_freq_lock_control.adjust_count++;

        if (g_freq_lock_control.total_sdio_pps) { // 当前数据流量不为零
            // 有包时连续100个周期都需要降频才降频
            if (g_freq_lock_control.adjust_count >= MAX_DEGRADE_FREQ_COUNT_THRESHOLD_SUCCESSIVE_10) {
                g_freq_lock_control.adjust_count = 0;
                /* 在这里进行主频值的调整 */
                hmac_adjust_set_freq();
            }
        } else { // 没有包时连续3个周期都需要降频再降频
            if (g_freq_lock_control.adjust_count >= MAX_DEGRADE_FREQ_COUNT_THRESHOLD_SUCCESSIVE_3) {
                g_freq_lock_control.adjust_count = 0;
                hmac_adjust_set_freq();
            }
        }
    } else {
        /* 升频不等待，立即执行保证性能 */
        g_freq_lock_control.adjust_count = 0;
        hmac_adjust_set_freq();
    }

    mutex_unlock(&g_freq_lock_control.st_lock_freq_mtx);
}

/*
 * 函 数 名  : hmac_wifi_auto_freq_ctrl_init
 * 功能描述  : 调频模块初始化
 * 1.日    期  : 2015年9月14日
  *   修改内容  : 新生成函数
 */
void hmac_wifi_auto_freq_ctrl_init(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (g_freq_lock_control.en_is_inited != OAL_TRUE) {
        mutex_init(&g_freq_lock_control.st_lock_freq_mtx);
        mutex_lock(&g_freq_lock_control.st_lock_freq_mtx);

        oam_warning_log4(0, OAM_SF_ANY,
            "{hmac_wifi_auto_freq_ctrl_init: orig_freq:: cpu_min[%d],cpu_max[%d],ddr_min[%d],ddr_max[%d]}",
            g_orig_cpu_min_freq, g_orig_cpu_max_freq, g_orig_ddr_min_freq, g_orig_ddr_max_freq);

        g_freq_lock_control.en_is_inited = OAL_TRUE;

        mutex_unlock(&g_freq_lock_control.st_lock_freq_mtx);
    }
#endif
}
/*
 * 函 数 名  : hmac_wifi_auto_freq_ctrl_deinit
 * 功能描述  : 调频模块去初始化
 * 1.日    期  : 2015年9月14日
  *   修改内容  : 新生成函数
 */
void hmac_wifi_auto_freq_ctrl_deinit(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mutex_lock(&g_freq_lock_control.st_lock_freq_mtx);

    if (g_freq_lock_control.en_is_inited == OAL_TRUE) {
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hw_wifi_freq_ctrl_destroy freq lock has already been released!}");
    }

    mutex_unlock(&g_freq_lock_control.st_lock_freq_mtx);
    mutex_destroy(&g_freq_lock_control.st_lock_freq_mtx);

    g_freq_lock_control.en_is_inited = OAL_FALSE;
#endif
}

oal_bool_enum_uint8 hmac_wifi_rx_is_busy(void)
{
    return g_st_wifi_load.en_wifi_rx_busy;
}

#if defined(CONFIG_ARCH_HISI) && defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
void hmac_userctl_bindcpu_rxdata(uint8_t thread_cpu, struct cpumask thread_cpu_mask)
{
    hmac_rxdata_thread_stru *rxdata_thread = hmac_get_rxdata_thread_addr();

    if (rxdata_thread->pst_rxdata_thread) {
        set_cpus_allowed_ptr(rxdata_thread->pst_rxdata_thread, &thread_cpu_mask);
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_userctl_bindcpu_rxdata::hisi_rxdata bind CPU[0x%x]}", thread_cpu);
    }
}

/*
 * 功能描述  : WiFi发送线程
 */
static void hmac_bindcpu_txthread(const struct cpumask *thread_cpu_mask)
{
    hmac_tid_schedule_stru *tid_sched = hmac_get_tid_schedule_list();
    hmac_tx_comp_stru *tx_comp = hmac_get_tx_comp_mgmt();

    if (tid_sched->tid_schedule_thread) {
        set_cpus_allowed_ptr(tid_sched->tid_schedule_thread, thread_cpu_mask);
    }
    if (tx_comp->tx_comp_thread) {
        set_cpus_allowed_ptr(tx_comp->tx_comp_thread, thread_cpu_mask);
    }
}

#if !defined(_PRE_PRODUCT_HI1620S_KUNPENG) && !defined(_PRE_WINDOWS_SUPPORT)
/*
 * 功能描述  : WiFi接收线程
 */
static void hmac_bindcpu_rxthread(const struct cpumask *thread_cpu_mask, uint8_t uc_cpu_id)
{
    hmac_rxdata_thread_stru *rxdata_thread = hmac_get_rxdata_thread_addr();
    hmac_rx_comp_stru *rx_comp = hmac_get_rx_comp_mgmt();

    if (rx_comp->rx_comp_thread) {
        set_cpus_allowed_ptr(rx_comp->rx_comp_thread, thread_cpu_mask);
    }
    if (rxdata_thread->pst_rxdata_thread == NULL) {
        return;
    }

    rxdata_thread->uc_allowed_cpus = 0;
    if (set_cpus_allowed_ptr(rxdata_thread->pst_rxdata_thread, thread_cpu_mask) != EOK) {
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_bindcpu_rxthread::cpumask fast[0x%x], cpu[0x%x]}",
            cpumask_bits(thread_cpu_mask)[0], uc_cpu_id);
        return;
    }
    /* 用于NAPI polling */
    rxdata_thread->uc_allowed_cpus = uc_cpu_id;
}

/*
 * 功能描述  : WiFi事件线程
 */
void hmac_rx_frw_task_bind_cpu(struct cpumask *cpus)
{
    oal_task_stru *frw_task_thread = get_frw_task();
    if (frw_task_thread == NULL) {
        return;
    }

    set_cpus_allowed_ptr(frw_task_thread, cpus);
}

/*
 * 功能描述  : 绑定主核的WiFi线程
 */
OAL_STATIC void hmac_bind_wifi_thread(uint8_t uc_cpu_id)
{
    struct cpumask fast_cpus, all_cpus;
    external_get_fast_cpus(&fast_cpus);
    cpumask_setall(&all_cpus);

    if (uc_cpu_id >= WLAN_IRQ_AFFINITY_BUSY_CPU) {
        /*  6 7核麒麟有特殊处理可能迁移不过去导致fastboot问题 */
        /*  irq核占用率容易到100%,故只绑567核保证160M性能 */
        oal_cpumask_clear_cpu(cpumask_first(&fast_cpus), &fast_cpus);

        hmac_bindcpu_rxthread(&fast_cpus, uc_cpu_id);
        hmac_bindcpu_txthread(&fast_cpus);
        /* hisi frw线程绑核 */
        wlan_chip_frw_task_bind_cpu(&fast_cpus);
    } else {
        /* CPU0上中断多, WIFI线程不放在CPU0 */
        oal_cpumask_clear_cpu(cpumask_first(&all_cpus), &all_cpus);

        hmac_bindcpu_rxthread(&all_cpus, uc_cpu_id);
        hmac_bindcpu_txthread(&all_cpus);
        /* hisi frw线程绑核 */
        wlan_chip_frw_task_bind_cpu(&all_cpus);
    }

    oam_warning_log3(0, OAM_SF_ANY, "{hmac_bind_wifi_thread::set id %d cpumask fast[0x%x], all[0x%x]}",
        uc_cpu_id, cpumask_bits(&fast_cpus)[0], cpumask_bits(&all_cpus)[0]);
}

/*
 * 功能描述  : WiFi线程绑核决策, 支持随意裁核策略
 */
OAL_STATIC uint8_t hmac_wifi_thread_bind_check(uint8_t *thread_bind, uint8_t *irq_bind)
{
    struct cpumask fast_cpus, slow_cpus, all_cpus;
    external_get_slow_cpus(&slow_cpus);
    external_get_fast_cpus(&fast_cpus);
    cpumask_setall(&all_cpus);
    oam_warning_log3(0, OAM_SF_ANY, "{hmac_wifi_thread_bind_check::cpumask fast[0x%x], slow[0x%x], all[0x%x]}",
        cpumask_bits(&fast_cpus)[0], cpumask_bits(&slow_cpus)[0], cpumask_bits(&all_cpus)[0]);
    *thread_bind = 0;
    *irq_bind = 0;

    /* 策略1：不大于4核 */
    if (CONFIG_NR_CPUS <= OAL_BUS_HPCPU_NUM) {
        return HOST_CPU_BIND_STRATEGY_LT_4CPU;
    }
    /* 策略2：只有小核 */
    if (cpumask_empty(&fast_cpus) || cpumask_empty(&slow_cpus) || cpumask_equal(&fast_cpus, &slow_cpus)) {
        return HOST_CPU_BIND_STRATEGY_SMALL_CPU;
    }
    /* 策略3：只有1个中大核, 绑PCIE中断 */
    if (cpumask_weight(&fast_cpus) == 1) {
        *irq_bind = 1;
        return HOST_CPU_BIND_STRATEGY_1_BIG_CPU;
    }
    /* 策略4：只有2个中大核 */
    if (cpumask_weight(&fast_cpus) == 2) {
        *thread_bind = 1;
        return HOST_CPU_BIND_STRATEGY_2_BIG_CPU;
    }
    /* 策略5：3个及以上中大核 */
    if (cpumask_weight(&fast_cpus) >= 3) {
        *irq_bind = 1;
        *thread_bind = 1;
        return HOST_CPU_BIND_STRATEGY_GT_4CPU;
    }
    return HOST_CPU_BIND_STRATEGY_BUTT;
}
#endif
#endif
#endif

/*
 * 功能描述  : 绑定主核的PCIE中断/WiFi收发线程/主核最高频率
 */
OAL_STATIC void hmac_auto_set_irq(uint8_t uc_cpu_id)
{
#if !defined(_PRE_PRODUCT_HI1620S_KUNPENG) && !defined(_PRE_WINDOWS_SUPPORT)
#if defined(CONFIG_ARCH_HISI) && defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    uint8_t thread_bind, irq_bind, ret;
#endif
#endif
#endif

    if (uc_cpu_id == g_freq_lock_control.uc_cur_irq_cpu) {
        return;
    }

    g_freq_lock_control.uc_cur_irq_cpu = uc_cpu_id;
    /* 定制化关闭 */
    if (g_freq_lock_control.en_irq_affinity == OAL_FALSE) {
        return;
    }

#if !defined(_PRE_PRODUCT_HI1620S_KUNPENG) && !defined(_PRE_WINDOWS_SUPPORT)
#if defined(CONFIG_ARCH_HISI) && defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    /* if need to bind */
    ret = hmac_wifi_thread_bind_check(&thread_bind, &irq_bind);
    oam_warning_log3(0, OAM_SF_ANY, "{hmac_wifi_thread_bind_check::ret[%d] thread[%d] irq[%d]}",
        ret, thread_bind, irq_bind);
    if ((thread_bind == 0) && (irq_bind == 0)) {
        return;
    }
    /* WIFI bus thread process */
    mpxx_bind_hcc_thread(((uc_cpu_id >= WLAN_IRQ_AFFINITY_BUSY_CPU) ? 1 : 0), thread_bind, irq_bind);
    /* WIFI thread process */
    if (thread_bind) {
        hmac_bind_wifi_thread(uc_cpu_id);
    }
#endif
#endif
#endif
}

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
OAL_STATIC mac_cfg_pk_mode_stru g_st_wifi_pk_mode_status = { 0 }; /* PK mode模式 */
OAL_STATIC OAL_INLINE void hmac_update_pk_mode_curr_protocol_cap(mac_user_stru *pst_mac_user)
{
    switch (pst_mac_user->en_protocol_mode) {
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11B_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            g_st_wifi_pk_mode_status.en_curr_protocol_cap = WLAN_PROTOCOL_CAP_LEGACY;
            break;
        case WLAN_HT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
            g_st_wifi_pk_mode_status.en_curr_protocol_cap = WLAN_PROTOCOL_CAP_HT;
            break;
        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            g_st_wifi_pk_mode_status.en_curr_protocol_cap = WLAN_PROTOCOL_CAP_VHT;
            break;
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
            if (g_wlan_spec_cfg->feature_11ax_is_open) {
                g_st_wifi_pk_mode_status.en_curr_protocol_cap = WLAN_PROTOCOL_CAP_HE;
                break;
            }
            g_st_wifi_pk_mode_status.en_curr_protocol_cap = WLAN_PROTOCOL_CAP_BUTT;
            break;
#endif
        case WLAN_PROTOCOL_BUTT:
        default:
            g_st_wifi_pk_mode_status.en_curr_protocol_cap = WLAN_PROTOCOL_CAP_BUTT;
            break;
    }
}

/*
 * 函 数 名  : hmac_update_pk_mode_info
 * 功能描述  : 更新pkmode需要的相关信息
 * 1.日    期  : 2017年10月24日
 *   修改内容  : 新生成函数
 */
void hmac_update_pk_mode_info(uint8_t *puc_vap_id)
{
    uint8_t uc_vap_id;
    mac_vap_stru *pst_mac_vap = NULL;
    mac_user_stru *pst_mac_user = NULL;
    oal_dlist_head_stru *pst_entry = NULL;
    oal_dlist_head_stru *pst_dlist_tmp = NULL;

    g_st_wifi_pk_mode_status.en_curr_bw_cap = WLAN_BW_CAP_BUTT;
    g_st_wifi_pk_mode_status.en_curr_protocol_cap = WLAN_PROTOCOL_CAP_BUTT;
    g_st_wifi_pk_mode_status.en_curr_num_spatial_stream = WLAN_SINGLE_NSS;

    for (uc_vap_id = 0; uc_vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id++) {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_vap_id);
        if (pst_mac_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_update_pk_mode_info::pst_mac_vap(%d) is null.}", uc_vap_id);
            continue;
        }

        if (!is_legacy_sta(pst_mac_vap)) {
            continue;
        }

        /* 记录TxBASessionNumber mib值到chr全局变量中 */
        if (pst_mac_vap->pst_mib_info != NULL) {
            hmac_chr_set_ba_session_num(mac_mib_get_TxBASessionNumber(pst_mac_vap));
        }

        oal_dlist_search_for_each_safe(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head)) {
            pst_mac_user = oal_dlist_get_entry(pst_entry, mac_user_stru, st_user_dlist);
            if (pst_mac_user == NULL) {
                oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_update_pk_mode_info::pst_user_tmp null.}");
                continue;
            }

            /* 从用户的信息中更新目前带宽和协议模式 */
            /* 此设计目前仅适用支持一个LEGACY_STA且LEGACY_STA仅一个业务用户，若规格有修改此函数需要重构 */
            g_st_wifi_pk_mode_status.en_curr_bw_cap = pst_mac_user->en_bandwidth_cap;
            g_st_wifi_pk_mode_status.en_curr_num_spatial_stream = pst_mac_user->en_avail_num_spatial_stream;
            /* 嵌套深度优化封装 */
            hmac_update_pk_mode_curr_protocol_cap(pst_mac_user);

            *puc_vap_id = uc_vap_id;
        }
    }

    return;
}

OAL_STATIC oal_bool_enum_uint8 hmac_update_pk_mode_status_by_th(
    wlan_protocol_cap_enum_uint8 en_cur_protocol_cap, uint32_t txrx_throughput)
{
    uint32_t pk_mode_th;
    oal_bool_enum_uint8 en_curr_pk_mode = OAL_TRUE;

    if (g_st_wifi_pk_mode_status.en_is_pk_mode == OAL_TRUE) {
        pk_mode_th = (g_st_pk_mode_low_th_table[en_cur_protocol_cap][g_st_wifi_pk_mode_status.en_curr_bw_cap]) *
            (1 + g_st_wifi_pk_mode_status.en_curr_num_spatial_stream);
        if (txrx_throughput < pk_mode_th) {
            g_st_wifi_pk_mode_status.pk_cnt++;
            if (g_st_wifi_pk_mode_status.pk_cnt >= 5) { // 连续5个低于门限退出pkmode
                g_st_wifi_pk_mode_status.pk_cnt = 0;
                en_curr_pk_mode = OAL_FALSE;
            }
        } else {
            g_st_wifi_pk_mode_status.pk_cnt = 0;
        }
    } else {
        pk_mode_th = (g_st_pk_mode_high_th_table[en_cur_protocol_cap][g_st_wifi_pk_mode_status.en_curr_bw_cap]) *
            (1 + g_st_wifi_pk_mode_status.en_curr_num_spatial_stream);
        en_curr_pk_mode = (txrx_throughput > pk_mode_th) ? OAL_TRUE : OAL_FALSE;
    }

    return en_curr_pk_mode;
}
OAL_STATIC oal_bool_enum_uint8 hmac_pk_mode_is_not_supp(mac_vap_stru *mac_vap)
{
    if ((mac_vap->en_protocol != WLAN_VHT_MODE) &&
        (mac_vap->en_protocol != WLAN_VHT_ONLY_MODE)) {
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
#ifdef _PRE_WLAN_FEATURE_11AX
            if (mac_vap->en_protocol != WLAN_HE_MODE) {
                return OAL_TRUE;
            }
#endif
        } else {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}
void hmac_update_fill_pk_mode_info(mac_cfg_pk_mode_stru *pk_mode_info)
{
    pk_mode_info->rx_bytes = g_st_wifi_pk_mode_status.rx_bytes;
    pk_mode_info->tx_bytes = g_st_wifi_pk_mode_status.tx_bytes;
    pk_mode_info->dur_time = g_st_wifi_pk_mode_status.dur_time;
    pk_mode_info->en_is_pk_mode = g_st_wifi_pk_mode_status.en_is_pk_mode;
    pk_mode_info->en_curr_bw_cap = g_st_wifi_pk_mode_status.en_curr_bw_cap;
    pk_mode_info->en_curr_protocol_cap = g_st_wifi_pk_mode_status.en_curr_protocol_cap;
    pk_mode_info->en_curr_num_spatial_stream = g_st_wifi_pk_mode_status.en_curr_num_spatial_stream;
}

/*
 * 功能描述  : 避免ddut mp13/mp15时调整edca参数
 * 1.日    期  : 2020年8月21日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
OAL_STATIC oal_bool_enum_uint8 hmac_update_pk_mode_hisi(mac_vap_stru *mac_vap)
{
    hmac_user_stru *hmac_user = NULL;

    if (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OAL_FALSE;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);
    if (hmac_user == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_WPA, "{hmac_update_pk_mode_hisi::get_mac_user null!}");
        return OAL_FALSE;
    }

    return hmac_user->user_hisi;
}

static void hmac_update_pk_mode_set_status(oal_bool_enum_uint8 curr_pk_mode,
    uint32_t tx_throughput, uint32_t rx_throughput, uint32_t dur_time)
{
    g_st_wifi_pk_mode_status.en_is_pk_mode = curr_pk_mode;
    g_st_wifi_pk_mode_status.rx_bytes = rx_throughput;
    g_st_wifi_pk_mode_status.tx_bytes = tx_throughput;
    g_st_wifi_pk_mode_status.dur_time = dur_time;
}

/*
 * 函 数 名  : hmac_update_pk_mode
 * 功能描述  : 更新pk mode的信息
 * 1.日    期  : 2017年10月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_update_pk_mode(uint32_t tx_throughput, uint32_t rx_throughput,  uint32_t dur_time)
{
    oal_bool_enum_uint8 en_curr_pk_mode;
    uint8_t uc_vap_id;
    mac_cfg_pk_mode_stru st_pk_mode_info;
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t ret, txrx_throughput;
    mac_device_stru *pst_mac_device = NULL;

    if (g_en_pk_mode_swtich == OAL_FALSE) {
        return;
    }

    pst_mac_device = mac_res_get_dev(0);
    /* 如果非单VAP,则不开启硬件聚合 */
    if (mac_device_calc_up_vap_num(pst_mac_device) != 1) {
        return;
    }

    /* 更新相关的信息 */
    hmac_update_pk_mode_info(&uc_vap_id);

    /* 非有效模式，不进行吞吐计算 */
    if ((g_st_wifi_pk_mode_status.en_curr_bw_cap == WLAN_BW_CAP_BUTT) ||
        (g_st_wifi_pk_mode_status.en_curr_protocol_cap == WLAN_PROTOCOL_CAP_BUTT)) {
        return;
    }

    /* 计算吞吐率: rxtx吞吐之和 / 统计的时间间隔(单位s) */
    txrx_throughput = tx_throughput + rx_throughput;
    /* 依据门限值判定pk mode状态 */
    en_curr_pk_mode = hmac_update_pk_mode_status_by_th(g_st_wifi_pk_mode_status.en_curr_protocol_cap, txrx_throughput);
    /* PK mode状态未变化，不更新数值 */
    if (en_curr_pk_mode == g_st_wifi_pk_mode_status.en_is_pk_mode) {
        return;
    }

    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_vap_id);
    if (pst_mac_vap == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_update_pk_mode::pst_mac_vap(%d) is null.}", uc_vap_id);
        return;
    }
    /*  避免2.4g ht 模式调整edca参数和txop存在负收益，只在VHT/HE才支持pk mode */
    /* 避免ddut mp13/mp15时调整edca参数 */
    if (hmac_pk_mode_is_not_supp(pst_mac_vap) || (hmac_update_pk_mode_hisi(pst_mac_vap) == OAL_TRUE)) {
        return;
    }
    oam_warning_log4(0, OAM_SF_ANY, "{hmac_update_pk_mode: pk_mode changed (%d) -> (%d), tx_throughput[%d], \
        rx_throughput[%d]!}", g_st_wifi_pk_mode_status.en_is_pk_mode, en_curr_pk_mode, tx_throughput, rx_throughput);
    hmac_update_pk_mode_set_status(en_curr_pk_mode, tx_throughput, rx_throughput, dur_time);
    /***************************************************************************
        抛事件到DMAC层, 同步VAP最新状态到DMAC
    ***************************************************************************/
    hmac_update_fill_pk_mode_info(&st_pk_mode_info);

    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SYNC_PK_MODE,
        sizeof(mac_cfg_pk_mode_stru), (uint8_t *)(&st_pk_mode_info));
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_update_pk_mode::hmac_config_send_event failed[%d].}", ret);
    }

    return;
}
#endif

/*
 * 函 数 名  : hmac_calc_throughput_cpu_freq
 * 功能描述  : 根据throughput和pps门限绑定大核CPU
 * 1.日    期  : 2019年12月4日
  *   修改内容  : 根据PPS门限绑定大核CPU，解决小包性能差问题
 */
OAL_STATIC void hmac_calc_throughput_cpu_freq(
    uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    uint16_t us_cpu_freq_high = WLAN_CPU_FREQ_THROUGHPUT_THRESHOLD_HIGH;
    uint16_t us_cpu_freq_low = WLAN_CPU_FREQ_THROUGHPUT_THRESHOLD_LOW;

    /* 每秒吞吐量门限 */
    if ((g_freq_lock_control.us_lock_cpu_th_high) && (g_freq_lock_control.us_lock_cpu_th_low)) {
        us_cpu_freq_high = g_freq_lock_control.us_lock_cpu_th_high;
        us_cpu_freq_low = g_freq_lock_control.us_lock_cpu_th_low;
    }
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 存在CHBA vap时，降低锁频门限为600M */
    hmac_chba_set_freq_lock_th(&us_cpu_freq_high, &us_cpu_freq_low);
#endif
    /* 高于800M时切超频，低于500M恢复 */
    if ((tx_throughput_mbps > us_cpu_freq_high) ||
        (rx_throughput_mbps > us_cpu_freq_high)) {
        g_freq_lock_control.uc_req_super_cpu_freq = WLAN_CPU_FREQ_SUPER;
    } else if ((tx_throughput_mbps < us_cpu_freq_low) &&
               (rx_throughput_mbps < us_cpu_freq_low)) {
        g_freq_lock_control.uc_req_super_cpu_freq = WLAN_CPU_FREQ_IDLE;
    }

    hmac_set_cpu_freq(g_freq_lock_control.uc_req_super_cpu_freq);
}

void hmac_get_perform_limit_value(uint32_t *limit_throughput_high, uint32_t *limit_throughput_low,
    uint32_t *limit_pps_high, uint32_t *limit_pps_low)
{
    /* 每秒吞吐量门限 */
    if ((g_freq_lock_control.us_throughput_irq_high) && (g_freq_lock_control.us_throughput_irq_low)) {
        *limit_throughput_high = g_freq_lock_control.us_throughput_irq_high;
        *limit_throughput_low = g_freq_lock_control.us_throughput_irq_low;
    }

    /* 每秒PPS门限 */
    if ((g_freq_lock_control.irq_pps_high) && (g_freq_lock_control.irq_pps_low)) {
        *limit_pps_high = g_freq_lock_control.irq_pps_high;
        *limit_pps_low = g_freq_lock_control.irq_pps_low;
    }
}

void hmac_calc_throughput_cpu_mask(void)
{
#ifdef _PRE_WLAN_FEATURE_LOW_LATENCY_SWITCHING
    if (g_freq_lock_control.uc_req_irq_cpu == WLAN_IRQ_AFFINITY_IDLE_CPU) {
        /*
        * Auto-adjusting code detected low network load but instead of
        * switching to small cores immediately, check first if the
        * low-latency configuration is set.
        */
        if (g_freq_lock_control.en_low_latency_bindcpu == OAL_TRUE) {
            g_freq_lock_control.uc_req_irq_cpu = WLAN_IRQ_AFFINITY_BUSY_CPU;
        }
    }
#endif

    hmac_auto_set_irq(g_freq_lock_control.uc_req_irq_cpu);
}

/*
 * 功能描述  : 根据throughput和pps门限绑定大核CPU
 *   作    者  : wifi
 */
void hmac_perform_calc_throughput(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps, uint32_t trx_pps)
{
    uint32_t limit_throughput_high = WLAN_IRQ_THROUGHPUT_THRESHOLD_HIGH;
    uint32_t limit_throughput_low = WLAN_IRQ_THROUGHPUT_THRESHOLD_LOW;
    uint32_t limit_pps_high = WLAN_IRQ_PPS_THRESHOLD_HIGH;
    uint32_t limit_pps_low = WLAN_IRQ_PPS_THRESHOLD_LOW;

    hmac_get_perform_limit_value(&limit_throughput_high, &limit_throughput_low,
                                 &limit_pps_high, &limit_pps_low);
    /* 高于200M时切大核，低于150M切小核 */
    if ((tx_throughput_mbps > limit_throughput_high) ||
        (rx_throughput_mbps > limit_throughput_high) || (trx_pps >= limit_pps_high)) {
        g_freq_lock_control.cpu_switch_to_idle_count = 0;
        g_freq_lock_control.uc_req_irq_cpu = WLAN_IRQ_AFFINITY_BUSY_CPU;
    } else if ((tx_throughput_mbps < limit_throughput_low) &&
               (rx_throughput_mbps < limit_throughput_low) && (trx_pps < limit_pps_low)) {
        g_freq_lock_control.cpu_switch_to_idle_count++;
        if (g_freq_lock_control.cpu_switch_to_idle_count > WLAN_THROUGHPUT_STA_PERIOD) {
            g_freq_lock_control.uc_req_irq_cpu = WLAN_IRQ_AFFINITY_IDLE_CPU;
            g_freq_lock_control.cpu_switch_to_idle_count = 0;
        }
    }

    if (g_freq_lock_control.uc_req_irq_cpu != g_freq_lock_control.uc_cur_irq_cpu) {
        oam_warning_log4(0, OAM_SF_ANY,
            "{hmac_perform_calc_throughput:throughput rx = %d , tx = %d, high_th = %d,low_th = %d!}",
            rx_throughput_mbps, tx_throughput_mbps, limit_throughput_high, limit_throughput_low);
        oam_warning_log3(0, OAM_SF_ANY, "{hmac_perform_calc_throughput: rx pps = %d , tx pps = %d, trx pps = %d,!}",
            g_freq_lock_control.rx_pps, g_freq_lock_control.tx_pps, trx_pps);
        oam_warning_log4(0, OAM_SF_ANY,
            "{hmac_perform_calc_throughput: req cpu id[%d],cur cpu id[%d],pps_high = %d, pps low = %d!}",
            g_freq_lock_control.uc_req_irq_cpu, g_freq_lock_control.uc_cur_irq_cpu,
            limit_pps_high, limit_pps_low);
    }

    /* 自适应绑核 */
    hmac_calc_throughput_cpu_mask();
    /* 自适应调频 */
    hmac_calc_throughput_cpu_freq(tx_throughput_mbps, rx_throughput_mbps);
}

OAL_STATIC oal_bool_enum_uint8 hmac_adjust_throughput_stat_txrx_bytes(uint32_t dur_time)
{
    /* 小于一个周期,不统计 */
    if (dur_time < WLAN_FREQ_TIMER_PERIOD - 10) { // 10是当前定时器周期需要减去10ms为一个周期
        return OAL_FALSE;
    }

    /* 超过一定时常则不统计,排除定时器异常 */
    if (dur_time > ((WLAN_FREQ_TIMER_PERIOD * WLAN_THROUGHPUT_STA_PERIOD) << BIT_OFFSET_2)) {
        hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();
        g_freq_lock_control.pre_time = jiffies;
        g_freq_lock_control.uc_timer_cycles = 0;

        hmac_device_stats->total_rx_bytes += (uint64_t)oal_atomic_read(&hmac_device_stats->rx_bytes);
        hmac_device_stats->total_tx_bytes += (uint64_t)oal_atomic_read(&hmac_device_stats->tx_bytes);
        oal_atomic_set(&hmac_device_stats->rx_bytes, 0);
        oal_atomic_set(&hmac_device_stats->tx_bytes, 0);

        return OAL_FALSE;
    }

    return OAL_TRUE;
}
OAL_STATIC oal_bool_enum_uint8 hmac_check_all_vap_legacy(void)
{
    uint8_t up_vap_num, idx;
    mac_vap_stru *mac_vap = NULL;
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    mac_vap_stru *up_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = { NULL };

    up_vap_num = mac_device_find_up_vaps(mac_device, up_vap_array, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (idx = 0; idx < up_vap_num; idx++) {
        mac_vap = up_vap_array[idx];
        if (mac_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_tx_sched_lowpower_lowdelay_strategy::pst_mac_vap(%d) is null.}", idx);
            continue;
        }

        if (mac_vap->en_protocol >= WLAN_HT_MODE) {
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}
static void hmac_tx_sched_lowpower_lowdelay_strategy(uint32_t tx_pps)
{
    uint8_t lowpower = OAL_FALSE;
    uint8_t only_legacy = OAL_TRUE;
    mac_vap_stru *cfg_vap = mac_res_get_mac_vap(0);
    hmac_psm_tx_switch_event_enum ring_state = hmac_psm_tx_switch_throughput_to_event(tx_pps);
    if (ring_state == PSM_TX_H2D_SWITCH) {
        lowpower = OAL_TRUE;
    }
    only_legacy = hmac_check_all_vap_legacy();
    if (only_legacy == OAL_TRUE) {
        lowpower = OAL_FALSE;
    }
    if (g_lowpower_strategy_swtich == lowpower) {
        return;
    }
    /* 若当前是窄带模式，不去下发低功耗单调度模式 */
    if ((cfg_vap->st_nb.en_open == OAL_TRUE) && (lowpower == OAL_TRUE)) {
        return;
    }
    g_lowpower_strategy_swtich = lowpower;
    hmac_config_send_event(cfg_vap, WLAN_CFGID_LOW_DELAY_STRATEGY, sizeof(uint8_t), (uint8_t *)(&lowpower));
}

static void hmac_update_pps_switch(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps, uint32_t dur_time)
{
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    /* AMSDU+AMPDU聚合切换入口 */
    hmac_tx_amsdu_ampdu_switch(tx_throughput_mbps, rx_throughput_mbps);
#endif
    hmac_tx_small_amsdu_switch(rx_throughput_mbps, g_freq_lock_control.tx_pps);

    hmac_device_tcp_buf_switch(tx_throughput_mbps, rx_throughput_mbps);

    /* 05 投屏单双天线切换 */
    wlan_chip_lp_miracast_stat_mbps(tx_throughput_mbps, rx_throughput_mbps);
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    hmac_tx_tcp_ack_buf_switch(rx_throughput_mbps);
#endif

    hmac_tx_sched_lowpower_lowdelay_strategy(g_freq_lock_control.tx_pps + g_freq_lock_control.host_rx_pps);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (hmac_tx_keep_host_switch() == OAL_FALSE) {
        /* tx切换使用chip级pps, 因为如果rx_pps不为0，说明rx已经切到host, 这时tx再切到device 对低功耗并没有意义 */
        hmac_tx_pps_switch(g_freq_lock_control.tx_pps + g_freq_lock_control.host_rx_pps);
    }
    hmac_tx_sche_switch_process(tx_throughput_mbps);
#endif

    hmac_pcie_switch_vote_by_pps(g_freq_lock_control.total_sdio_pps);

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* 聚合软硬件切换入口,每个周期探测一次 */
    hmac_tx_ampdu_switch(tx_throughput_mbps);
#endif
#ifdef _PRE_WLAN_TCP_OPT
    hmac_tcp_ack_filter_switch(rx_throughput_mbps);
#endif

    /* 根据吞吐量，判断是否需要bypass 外置LNA */
    hmac_rx_dyn_bypass_extlna_switch(tx_throughput_mbps, rx_throughput_mbps);

    /* 中断动态调整 */
    hmac_perform_calc_throughput(tx_throughput_mbps, rx_throughput_mbps, g_freq_lock_control.total_sdio_pps);
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    /* 更新pk mode */
    hmac_update_pk_mode(tx_throughput_mbps, rx_throughput_mbps, dur_time);
#endif
    hmac_tx_ring_stuck_check();
#ifdef _PRE_DELAY_DEBUG
    if (g_wifi_delay_debug != 0) {
        hmac_host_record_tsf_lo();
    }
#endif
}
uint32_t g_checksum_control_cnt = 0;
void hmac_update_rx_checksum_stat(void)
{
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();

    g_checksum_control_cnt++;
    hmac_device_stats->rx_tcp_checksum_total_fail +=
        (uint64_t)oal_atomic_read(&hmac_device_stats->rx_tcp_checksum_fail);
    hmac_device_stats->rx_tcp_checksum_total_pass +=
        (uint64_t)oal_atomic_read(&hmac_device_stats->rx_tcp_checksum_pass);
    hmac_device_stats->rx_ip_checksum_total_fail +=
        (uint64_t)oal_atomic_read(&hmac_device_stats->rx_ip_checksum_fail);
    hmac_device_stats->rx_ip_checksum_total_pass +=
        (uint64_t)oal_atomic_read(&hmac_device_stats->rx_ip_checksum_pass);
    oal_atomic_set(&hmac_device_stats->rx_tcp_checksum_fail, 0);
    oal_atomic_set(&hmac_device_stats->rx_tcp_checksum_pass, 0);
    oal_atomic_set(&hmac_device_stats->rx_ip_checksum_fail, 0);
    oal_atomic_set(&hmac_device_stats->rx_ip_checksum_pass, 0);
    /* 10min 打印一次checksum维测 */
    if (g_checksum_control_cnt < WLAN_CHECKSUM_STAT_PERIOD) {
        return;
    }
    g_checksum_control_cnt = 0;

    oam_warning_log4(0, 0,
        "{hmac_update_rx_checksum_stat::rx_tcp_checksum_pass[%d], rx_tcp_checksum_fail[%d], \
        rx_ip_checksum_pass[%d], rx_ip_checksum_fail[%d]}",
        hmac_device_stats->rx_tcp_checksum_total_pass,
        hmac_device_stats->rx_tcp_checksum_total_fail,
        hmac_device_stats->rx_ip_checksum_total_pass,
        hmac_device_stats->rx_ip_checksum_total_fail);
    hmac_device_stats->rx_tcp_checksum_total_fail = 0;
    hmac_device_stats->rx_tcp_checksum_total_pass = 0;
    hmac_device_stats->rx_ip_checksum_total_fail = 0;
    hmac_device_stats->rx_ip_checksum_total_pass = 0;
}

OAL_STATIC void hmac_update_aspm_switch(uint32_t aspm)
{
    if (g_wlan_chip_ops->trx_aspm_switch_on == NULL) {
        return;
    }
    g_freq_lock_control.hcc_aspm_switch_off_cnt = 0;
    g_freq_lock_control.hcc_aspm_switch_on = aspm;
    g_wlan_chip_ops->trx_aspm_switch_on(g_freq_lock_control.hcc_aspm_switch_on);
}
/*
 * 函 数 名  : hmac_update_aspm_switch_on_by_pps
 * 功能描述  : 根据pps调整ASPM开关
 * 1.日    期  : 2023年10月16日
  *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_update_aspm_switch_on_by_pps(uint32_t trx_pps)
{
    if (g_wlan_chip_ops->trx_aspm_switch_on == NULL) {
        return;
    }

    /* PPS高于判定门限，ASPM关闭计数+1 */
    if (trx_pps > WLAN_ASPM_SWITCH_PPS_THRESHOLD_HIGH) {
        if (g_freq_lock_control.hcc_aspm_switch_off_cnt < WLAN_ASPM_SWITCH_COUNT_THRESHOLD) {
            g_freq_lock_control.hcc_aspm_switch_off_cnt++;
        }
        /* ASPM打开，且PPS连续高于门限，则关闭ASPM */
        if (g_freq_lock_control.hcc_aspm_switch_off_cnt >= WLAN_ASPM_SWITCH_COUNT_THRESHOLD &&
            g_freq_lock_control.hcc_aspm_switch_on == OAL_TRUE) {
            hmac_update_aspm_switch(OAL_FALSE);
            oam_warning_log1(0, 0, "{hmac_update_aspm_switch_on_by_pps::trx_pps is [%d], ASPM CLOSE.}", trx_pps);
        }
        return;
    }

    /* PPS低于判定门限，则打开ASPM */
    if (trx_pps < WLAN_ASPM_SWITCH_PPS_THRESHOLD_LOW && g_freq_lock_control.hcc_aspm_switch_on == OAL_FALSE) {
        if (g_freq_lock_control.hcc_aspm_close_bitmap != 0) {
            g_freq_lock_control.hcc_aspm_switch_off_cnt = 0;
            return;
        }
        hmac_update_aspm_switch(OAL_TRUE);
        oam_warning_log1(0, 0, "{hmac_update_aspm_switch_on_by_pps::trx_pps is [%d], ASPM OPEN.}", trx_pps);
        return;
    }
}

/*
 * 函 数 名  : hmac_adjust_throughput
 * 功能描述  : hmac吞吐量计算
 * 1.日    期  : 2015年9月14日
  *   修改内容  : 新生成函数
 */
void hmac_adjust_throughput(void)
{
    uint32_t tx_throughput, rx_throughput, dur_time;
    uint32_t tx_throughput_mbps = 0;
    uint32_t rx_throughput_mbps = 0;
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();

    /* 循环次数统计 */
    g_freq_lock_control.uc_timer_cycles++;

    dur_time = oal_jiffies_to_msecs(jiffies - g_freq_lock_control.pre_time);
    if (!hmac_adjust_throughput_stat_txrx_bytes(dur_time)) {
        return;
    }

    /* 总字节数 */
    rx_throughput = (uint32_t)oal_atomic_read(&hmac_device_stats->rx_bytes);
    tx_throughput = (uint32_t)oal_atomic_read(&hmac_device_stats->tx_bytes);
    if (dur_time != 0) {
        tx_throughput_mbps = (tx_throughput >> HMAC_TX_THROUGHPUT_OFFSET_7) / dur_time;
        rx_throughput_mbps = (rx_throughput >> HMAC_TX_THROUGHPUT_OFFSET_7) / dur_time;
    }

    g_st_wifi_load.en_wifi_rx_busy = (rx_throughput_mbps <= WLAN_THROUGHPUT_LOAD_LOW) ? OAL_FALSE : OAL_TRUE;
    if (g_freq_lock_control.uc_trx_stat_log_en == OAL_TRUE) {
        oam_warning_log4(0, 0, "{hmac_adjust_throughput::rx[%d],tx[%d],smooth[%d],dur_time[%d]ms!}", rx_throughput_mbps,
            tx_throughput_mbps, mac_vap_get_tcp_ack_buf_switch()->us_tcp_ack_smooth_throughput, dur_time);
    }

    hmac_update_pps_switch(tx_throughput_mbps, rx_throughput_mbps, dur_time);

    hmac_update_aspm_switch_on_by_pps(g_freq_lock_control.total_sdio_pps);

    /* 每1s清空一次 */
    if (g_freq_lock_control.uc_timer_cycles < 10) { // 10代表10个100ms周期
        return;
    }
    g_freq_lock_control.uc_timer_cycles = 0;

    /* 2s周期清零一次 */
    hmac_device_stats->total_rx_bytes += (uint64_t)oal_atomic_read(&hmac_device_stats->rx_bytes);
    hmac_device_stats->total_tx_bytes += (uint64_t)oal_atomic_read(&hmac_device_stats->tx_bytes);
    oal_atomic_set(&hmac_device_stats->rx_bytes, 0);
    oal_atomic_set(&hmac_device_stats->tx_bytes, 0);

    g_freq_lock_control.pre_time = jiffies;
}

/*
 * 函 数 名  : hmac_wlan_wdg_timeout
 * 功能描述  : 200ms定时器超时处理，进行调频处理
 * 1.日    期  : 2017年7月13日
 * 2.日    期  : 2019年1月31日
 *   修改内容  : 修改函数定义和frw_timeout_func相同
 */
OAL_STATIC uint32_t hmac_wlan_freq_wdg_timeout(void *p_arg)
{
    /* 计算pps */
    hmac_pps_calc();
    hmac_update_rx_checksum_stat();
    /* 吞吐统计 */
    hmac_adjust_throughput();
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_chba_update_throughput();
#endif
    hal_pm_check_forbid_sleep_in_timer();
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_freq_timer_init
 * 功能描述  : 200ms定时器启动函数
 * 1.日    期  : 2017年7月23日
 *   修改内容  : 新建函数
 */
void hmac_freq_timer_init(void)
{
    hmac_device_stat_stru *hmac_device_stats = NULL;
    /* 如果这个定时器已经注册成功，则不能再次被注册，不然后发生错误！ */
    if (g_freq_lock_control.hmac_freq_timer.en_is_registerd == OAL_TRUE) {
        return;
    }

    /* 重启定时器后，ASPM相关控制变量复位 */
    g_freq_lock_control.hcc_aspm_switch_off_cnt = 0;
    g_freq_lock_control.hcc_aspm_switch_on = OAL_TRUE;
    /* 重启定时器后，清空之前的统计 */
    g_freq_lock_control.pre_time = jiffies;
    g_freq_lock_control.amsdu_level_switch_count = 0;
    hmac_device_stats = hmac_stat_get_device_stats();
#ifdef _PRE_WLAN_CHBA_MGMT
    /* 重启定时器后, 清空chba之前的统计 */
    hmac_chba_throughput_statis_init();
#endif
    /* 重新对lowpower全局变量初始化 */
    g_lowpower_strategy_swtich = OAL_TRUE;
    hmac_device_stats->total_rx_packets += (uint32_t)oal_atomic_read(&hmac_device_stats->rx_packets);
    hmac_device_stats->total_rx_bytes += (uint64_t)oal_atomic_read(&hmac_device_stats->rx_bytes);
    hmac_device_stats->total_tx_packets += (uint32_t)oal_atomic_read(&hmac_device_stats->tx_packets);
    hmac_device_stats->total_tx_bytes += (uint64_t)oal_atomic_read(&hmac_device_stats->tx_bytes);
    oal_atomic_set(&hmac_device_stats->rx_packets, 0);
    oal_atomic_set(&hmac_device_stats->rx_bytes, 0);
    oal_atomic_set(&hmac_device_stats->tx_packets, 0);
    oal_atomic_set(&hmac_device_stats->tx_bytes, 0);
    oal_atomic_set(&hmac_device_stats->host_ring_rx_packets, 0);
    frw_timer_create_timer_m(&g_freq_lock_control.hmac_freq_timer,  // pst_timeout
                             hmac_wlan_freq_wdg_timeout,                                   // p_timeout_func
                             WLAN_FREQ_TIMER_PERIOD,                                       // timeout
                             NULL,                                                 // p_timeout_arg
                             OAL_TRUE,                                                     // en_is_periodic
                             OAM_MODULE_ID_HMAC, 0);                                       // en_module_id && core_id
}

/*
 * 函 数 名  : hmac_freq_timer_deinit
 * 功能描述  : 200ms定时器关闭函数
 * 1.日    期  : 2017年7月23日
 *   修改内容  : 新建函数
 */
void hmac_freq_timer_deinit(void)
{
    /* 定时器没有注册，就不用删除了，不然会发生错误！ */
    if (g_freq_lock_control.hmac_freq_timer.en_is_registerd == OAL_FALSE) {
        return;
    }

    frw_timer_immediate_destroy_timer_m(&g_freq_lock_control.hmac_freq_timer);
}

void hmac_wifi_pm_state_notify(oal_bool_enum_uint8 en_wake_up)
{
    if (en_wake_up == OAL_TRUE) {
        /* WIFI唤醒,启动吞吐量统计定时器 */
        hmac_freq_timer_init();
        hmac_tid_update_timer_init();
    } else {
        /* WIFI睡眠,关闭吞吐量统计定时器 */
        hmac_freq_timer_deinit();
        hmac_tid_update_timer_deinit();
    }
}

void hmac_userctl_bindcpu_get_cmd(uint32_t cmd)
{
}
/*
 * 函 数 名  : hmac_freq_recover_freq
 * 功能描述  : 关机恢复调频
 */
void hmac_freq_recover_freq(void)
{
    /* 非低时延业务,说明业务流量比较低, 才恢复绑核 */
    g_freq_lock_control.uc_req_irq_cpu = WLAN_IRQ_AFFINITY_IDLE_CPU;
    hmac_calc_throughput_cpu_mask();

    hmac_set_cpu_freq(WLAN_CPU_FREQ_IDLE);
}
/*
 * 功能描述  : 平台上下电通知wifi侧业务
 * 1.日    期  : 2015年9月14日
  *   修改内容  : 新生成函数
 */
void hmac_wifi_state_notify(oal_bool_enum_uint8 en_wifi_on)
{
    mac_device_stru *mac_device = mac_res_get_dev(0);
    hal_pm_try_wakeup_reset_forbid_sleep();
    hmac_tid_sche_list_dump(HMAC_TID_SCHE_LIST_WIFI_STATE_NOTIFY);
    if (en_wifi_on == OAL_TRUE) {
        hal_host_approve_access_device();
        hal_host_intr_regs_init(HAL_DEVICE_ID_MASTER);
        hal_host_intr_regs_init(HAL_DEVICE_ID_SLAVE);
        if (hmac_host_ring_tx_enabled() == OAL_TRUE) {
            hal_host_ring_tx_init(HAL_DEVICE_ID_MASTER);
        }
#ifdef _PRE_WLAN_FEATURE_FTM
        hal_host_ftm_reg_init(HAL_DEVICE_ID_MASTER);
#endif
        /* WIFI上电,启动吞吐量统计定时器,打开调频功能 */
        hmac_freq_timer_init();
        hmac_tid_update_timer_init();
#ifdef _PRE_WLAN_FEATURE_LOW_LATENCY_SWITCHING
        hmac_low_latency_wifi_enable();
#endif
        if (mac_device != NULL) {
            mac_device_init_dyn_pcie(mac_device);
        }
    } else {
#ifdef _PRE_WLAN_FEATURE_LOW_LATENCY_SWITCHING
        hmac_low_latency_wifi_disable();
#endif
        frw_event_process_all_event_when_wlan_close();
        /* WIFI下电,关闭吞吐量统计定时器,关闭调频功能 */
        hmac_freq_timer_deinit();
        hmac_freq_recover_freq();
        hmac_tid_update_timer_deinit();
#ifdef _PRE_CONFIG_HISIFASTPOWER_DOWN_STATE
        /* WIFI下电, 停止正在进行的wifi业务 */
        hmac_config_change_wifi_state_for_power_down();
#endif
        hal_host_forbid_access_device();
        hal_host_device_exit(0);
        hal_host_device_exit(1);
        mac_device_reset_state_flag();
    }
}

#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
void hmac_cpu_freq_upper_limit_switch(oal_bool_enum_uint8 limit)
{
    g_freq_lock_control.limit_cpu_freq = limit;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_cpu_freq_upper_limit_switch::limit[%d]}", limit);
}
#endif
