/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : DFX文件接扩
 * 创建日期 : 2014年4月10日
 */

/* 1 头文件包含 */
#include "oal_ext_if.h"
#include "oal_hcc_host_if.h"
#include "oam_ext_if.h"
#include "host_hal_device.h"
#include "hmac_dfx.h"
#include "hmac_ext_if.h"
#include "hmac_stat.h"
#include "hmac_tid_sche.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_tx_ring_alloc.h"

#include "mac_resource.h"
#include "mac_vap.h"
#include "chr_user.h"
#include "chr_errno.h"

#include "securec.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#include "hmac_chba_function.h"
#include "hmac_chba_sync.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DFX_C

/* 2 全局变量定义 */
#ifdef _PRE_WLAN_FEATURE_DFR
hmac_dfr_info_stru g_st_dfr_info; /* DFR异常复位开关 */
#endif

uint32_t hmac_dfx_init(void)
{
#ifdef _PRE_CONFIG_HW_CHR
    oal_build_bug_on(sizeof(hmac_chr_info) != HMAC_CHR_INFO_STRU_SIZE);
    chr_host_callback_register(hmac_get_chr_info_event_hander);
#endif
    return OAL_SUCC;
}

uint32_t hmac_dfx_exit(void)
{
#ifdef _PRE_CONFIG_HW_CHR
    chr_host_callback_unregister();
#endif
    return OAL_SUCC;
}

/**********************全局变量****************************/
/* 去关联共有7种原因(0~6),默认值设置为7，表示没有去关联触发 */
hmac_chr_disasoc_reason_stru g_hmac_chr_disasoc_reason = { 0, DMAC_DISASOC_MISC_BUTT };

/* 关联字码, 新增4中私有定义字码5200-5203 */
uint16_t g_hmac_chr_connect_code = 0;

hmac_chr_del_ba_info_stru g_hmac_chr_del_ba_info = { 0, 0, MAC_UNSPEC_REASON };

/**********************获取全局变量地址****************************/
hmac_chr_disasoc_reason_stru *hmac_chr_disasoc_reason_get_pointer(void)
{
    return &g_hmac_chr_disasoc_reason;
}

uint16_t *hmac_chr_connect_code_get_pointer(void)
{
    return &g_hmac_chr_connect_code;
}

hmac_chr_del_ba_info_stru *hmac_chr_del_ba_info_get_pointer(void)
{
    return &g_hmac_chr_del_ba_info;
}

/* 回复全部变量的初始值 */
void hmac_chr_info_clean(void)
{
    g_hmac_chr_disasoc_reason.us_user_id = 0;
    g_hmac_chr_disasoc_reason.en_disasoc_reason = DMAC_DISASOC_MISC_BUTT;
    g_hmac_chr_connect_code = 0;
    g_hmac_chr_del_ba_info.uc_ba_num = 0;
    g_hmac_chr_del_ba_info.uc_del_ba_tid = 0;
    g_hmac_chr_del_ba_info.en_del_ba_reason = MAC_UNSPEC_REASON;

    return;
}
/**********************CHR打点和获取****************************/
/* 现阶段CHR只考虑STA状态(不考虑P2P)，所以不区分vap_id */
/* 打点 */
void hmac_chr_set_disasoc_reason(uint16_t user_id, uint16_t reason_id)
{
    hmac_chr_disasoc_reason_stru *pst_disasoc_reason = NULL;

    pst_disasoc_reason = hmac_chr_disasoc_reason_get_pointer();

    pst_disasoc_reason->us_user_id = user_id;
    pst_disasoc_reason->en_disasoc_reason = (dmac_disasoc_misc_reason_enum)reason_id;

    return;
}

/* 获取 */
void hmac_chr_get_disasoc_reason(hmac_chr_disasoc_reason_stru *pst_disasoc_reason)
{
    hmac_chr_disasoc_reason_stru *pst_disasoc_reason_temp = NULL;

    pst_disasoc_reason_temp = hmac_chr_disasoc_reason_get_pointer();

    pst_disasoc_reason->us_user_id = pst_disasoc_reason_temp->us_user_id;
    pst_disasoc_reason->en_disasoc_reason = pst_disasoc_reason_temp->en_disasoc_reason;

    return;
}

void hmac_chr_set_ba_session_num(uint8_t uc_ba_num)
{
    hmac_chr_del_ba_info_stru *pst_del_ba_info = NULL;

    pst_del_ba_info = hmac_chr_del_ba_info_get_pointer();
    pst_del_ba_info->uc_ba_num = uc_ba_num;
    return;
}

/* 打点 */
/* 梳理删减聚合的流程 计数统计 */
void hmac_chr_set_del_ba_info(uint8_t uc_tid, uint16_t reason_id)
{
    hmac_chr_del_ba_info_stru *pst_del_ba_info = NULL;

    pst_del_ba_info = hmac_chr_del_ba_info_get_pointer();

    pst_del_ba_info->uc_del_ba_tid = uc_tid;
    pst_del_ba_info->en_del_ba_reason = (mac_reason_code_enum)reason_id;

    return;
}

/* 获取 */
void hmac_chr_get_del_ba_info(mac_vap_stru *pst_mac_vap,
    hmac_chr_del_ba_info_stru *pst_del_ba_reason)
{
    hmac_chr_del_ba_info_stru *pst_del_ba_info = NULL;

    pst_del_ba_info = hmac_chr_del_ba_info_get_pointer();

    pst_del_ba_reason->uc_ba_num = pst_del_ba_info->uc_ba_num;
    pst_del_ba_reason->uc_del_ba_tid = pst_del_ba_info->uc_del_ba_tid;
    pst_del_ba_reason->en_del_ba_reason = pst_del_ba_info->en_del_ba_reason;

    return;
}

void hmac_chr_set_connect_code(uint16_t connect_code)
{
    uint16_t *pus_connect_code = NULL;

    pus_connect_code = hmac_chr_connect_code_get_pointer();
    *pus_connect_code = connect_code;
    return;
}

void hmac_chr_get_connect_code(uint16_t *pus_connect_code)
{
    pus_connect_code = hmac_chr_connect_code_get_pointer();
    return;
}

void hmac_chr_get_vap_info(mac_vap_stru *pst_mac_vap, hmac_chr_vap_info_stru *pst_vap_info)
{
    mac_user_stru *pst_mac_user = NULL;
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(0);
    if (pst_mac_device == NULL) {
        return;
    }
    pst_vap_info->uc_vap_state = pst_mac_vap->en_vap_state;
    pst_vap_info->uc_vap_num = pst_mac_device->uc_vap_num;
    pst_vap_info->uc_vap_rx_nss = pst_mac_vap->en_vap_rx_nss;
    pst_vap_info->uc_protocol = pst_mac_vap->en_protocol;

    /* sta 关联的AP的能力 */
    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id);
    if (pst_mac_user != NULL) {
        pst_vap_info->uc_ap_spatial_stream_num = pst_mac_user->en_user_max_cap_nss;
        pst_vap_info->bit_ap_11ntxbf = pst_mac_user->st_cap_info.bit_11ntxbf;
        pst_vap_info->bit_ap_qos = pst_mac_user->st_cap_info.bit_qos;
        pst_vap_info->bit_ap_1024qam_cap = pst_mac_user->st_cap_info.bit_1024qam_cap;
        pst_vap_info->uc_ap_protocol_mode = pst_mac_user->en_protocol_mode;
    }

    /* dfx线程和hisi_frw线程竞争访问mib指针,
     * hisi_frw删除vap后设置mib指针为空,chr线程再访问mib指针导致系统死机,
     * 修改为直接赋值为TRUE.
 */
    pst_vap_info->bit_ampdu_active = OAL_TRUE;
    pst_vap_info->bit_amsdu_active = OAL_TRUE;
    pst_vap_info->bit_sta_11ntxbf = pst_mac_vap->st_cap_flag.bit_11ntxbf;
    pst_vap_info->bit_is_dbac_running = mac_is_dbac_running(pst_mac_device);
    pst_vap_info->bit_is_dbdc_running = mac_is_dbdc_running(pst_mac_device);

    return;
}

OAL_STATIC OAL_INLINE void hmac_chr_get_info(mac_vap_stru *mac_vap, hmac_chr_info *p_hmac_chr_info)
{
    hmac_chr_get_disasoc_reason(&p_hmac_chr_info->st_disasoc_reason);
    hmac_chr_get_del_ba_info(mac_vap, &p_hmac_chr_info->st_del_ba_info);
    hmac_chr_get_connect_code(&p_hmac_chr_info->us_connect_code);
    hmac_chr_get_vap_info(mac_vap, &p_hmac_chr_info->st_vap_info);
}

void hmac_chr_mvap_report(mac_vap_stru *mac_vap, uint8_t scene)
{
    mac_chr_mvap_info_stru mvap_info = { 0 };

    mvap_info.vap_mode = mac_vap->en_vap_mode;
    mvap_info.p2p_mode = mac_vap->en_p2p_mode;
#ifdef _PRE_WLAN_CHBA_MGMT
    mvap_info.chba_mode = mac_vap->chba_mode;
#endif
    mvap_info.scene = scene;
    chr_exception_p(CHR_MVAP_REPORT_EVENTID, (uint8_t *)&mvap_info, sizeof(mac_chr_mvap_info_stru));
}

#ifdef _PRE_CONFIG_HW_CHR
#ifdef _PRE_WLAN_CHBA_MGMT
void hmac_chr_get_chba_info(mac_device_stru *mac_device, hmac_chr_chba_info_stru *chba_info)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_chba_vap_stru *chba_vap_info = NULL;
    hmac_chba_mgmt_info_stru *mgmt_info = hmac_chba_get_mgmt_info();
    mac_chba_self_conn_info_stru *self_conn_info = hmac_chba_get_self_conn_info();
    hmac_chba_sync_info *sync_info = hmac_chba_get_sync_info();

    hmac_vap = hmac_chba_find_chba_vap(mac_device);
    if (hmac_vap == NULL) {
        return;
    }
    mac_vap = &(hmac_vap->st_vap_base_info);

    chba_vap_info = hmac_vap->hmac_chba_vap_info;
    chba_info->chba_mode = mac_vap->chba_mode;
    chba_info->sync_state = sync_info->sync_state;
    chba_info->chba_module_state = mgmt_info->chba_module_state;
    chba_info->role = self_conn_info->role;
    chba_info->island_device_cnt = self_conn_info->island_device_cnt;
    chba_info->vap_bitmap_level = chba_vap_info->vap_bitmap_level;
    chba_info->social_channel_number = chba_vap_info->social_channel.uc_chan_number;
    chba_info->work_channel_number = chba_vap_info->work_channel.uc_chan_number;
    chba_info->second_work_channel_num = chba_vap_info->second_work_channel.uc_chan_number;
    chba_info->channel_sequence_bitmap = chba_vap_info->channel_sequence_bitmap;
}
#endif

void hmac_chr_get_pcie_state(mac_device_stru *mac_device, hmac_chr_info_new_arch *chr_info_new_arch)
{
    chr_info_new_arch->cur_pcie_state = mac_device->cur_pcie_state;
}

void hmac_chr_get_rx_info(hmac_chr_info_new_arch *chr_info_new_arch)
{
    uint8_t dev_id;
    hal_host_device_stru *hal_device = NULL;
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();

    hmac_chr_get_rx_checksum_stat(&(chr_info_new_arch->rx_info.rx_tcp_checksum_fail),
        &(chr_info_new_arch->rx_info.rx_ip_checksum_fail));
    chr_info_new_arch->rx_info.rx_pps = g_freq_lock_control.host_rx_pps;
    chr_info_new_arch->rx_info.total_rx_packets = hmac_device_stats->total_rx_packets;

    for (dev_id = 0; dev_id < WLAN_DEVICE_MAX_NUM_PER_CHIP; dev_id++) {
        hal_device = hal_get_host_device(dev_id);
        if (hal_device == NULL) {
            continue;
        }

        chr_info_new_arch->rx_info.rx_exception_free_cnt[dev_id] = hal_device->rx_statics.rx_exception_free_cnt;
        chr_info_new_arch->rx_info.rx_mode[dev_id] = oal_atomic_read(&hal_device->rx_mode);
    }
}

static void hmac_chr_get_tx_ring_info(mac_vap_stru *mac_vap, hmac_chr_info_new_arch *chr_info_new_arch)
{
    uint8_t tid;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    hmac_tid_info_stru *tid_info = NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user == NULL) {
        return;
    }
    for (tid = 0; tid < WLAN_TID_MAX_NUM - 1; tid++) {
        tid_info = &hmac_user->tx_tid_info[tid];

        if (!hmac_tx_ring_alloced(&tid_info->tx_ring)) {
            continue;
        }
        chr_info_new_arch->tx_ring_info[tid].tx_ring_used = OAL_TRUE;
        chr_info_new_arch->tx_ring_info[tid].push_ring_full_cnt = tid_info->tx_ring.push_ring_full_cnt;
        chr_info_new_arch->tx_ring_info[tid].ring_tx_mode = (uint8_t)oal_atomic_read(&tid_info->ring_tx_mode);
        chr_info_new_arch->tx_ring_info[tid].msdu_cnt = (uint32_t)oal_atomic_read(&tid_info->tx_ring.msdu_cnt);
        chr_info_new_arch->tx_ring_info[tid].tx_msdu_cnt = tid_info->tx_ring.tx_msdu_cnt;
        chr_info_new_arch->tx_ring_info[tid].tx_pps =
            (uint16_t)oal_atomic_read(&tid_info->tx_ring.last_period_tx_msdu);
    }
}

static void hmac_chr_get_tx_info(mac_device_stru *mac_device, hmac_chr_info_new_arch *chr_info_new_arch)
{
    uint8_t idx;
    uint8_t up_vap_num;
    mac_vap_stru *mac_vap = NULL;
    mac_vap_stru *up_vap_array[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = { NULL };

    up_vap_num = mac_device_find_up_vaps(mac_device, up_vap_array, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (idx = 0; idx < up_vap_num; idx++) {
        mac_vap = up_vap_array[idx];
        if (mac_vap == NULL) {
            continue;
        }
        if (!is_legacy_sta(mac_vap)) {
            continue;
        }
        hmac_chr_get_tx_ring_info(mac_vap, chr_info_new_arch);
    }
}

static void hmac_chr_get_info_new_arch(mac_device_stru *mac_device, hmac_chr_info_new_arch *chr_info_new_arch)
{
    chr_info_new_arch->chr_version_flag = CHR_VERSION_1;
    hmac_chr_get_pcie_state(mac_device, chr_info_new_arch);
    hmac_chr_get_rx_info(chr_info_new_arch);
    hmac_chr_get_tx_info(mac_device, chr_info_new_arch);
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_chr_get_chba_info(mac_device, &chr_info_new_arch->chba_info);
#endif
}

uint32_t hmac_chr_get_chip_info_new_arch(uint32_t chr_event_id, mac_device_stru *mac_device)
{
    hmac_chr_info_new_arch *chr_info_new_arch = NULL;
    chr_info_new_arch = (hmac_chr_info_new_arch *)oal_memalloc(sizeof(hmac_chr_info_new_arch));
    if (chr_info_new_arch == NULL) {
        oam_error_log0(0, OAM_SF_ACS, "{hmac_chr_get_chip_info_new_arch::chr_info_new_arch alloc mem fail!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(chr_info_new_arch, sizeof(hmac_chr_info_new_arch), 0, sizeof(hmac_chr_info_new_arch));

    hmac_chr_get_info_new_arch(mac_device, chr_info_new_arch);

    oam_warning_log2(0, OAM_SF_ACS, "{hmac_chr_get_chip_info_new_arch::chr_event_id=[%d] chr_flag=[%d].}",
        chr_event_id, CHR_HOST_NEW_ARCH_WIFI_INFO);
    chr_exception_q(chr_event_id, CHR_HOST_NEW_ARCH_WIFI_INFO,
        (uint8_t *)chr_info_new_arch, sizeof(hmac_chr_info_new_arch));
    oal_free(chr_info_new_arch);

    return OAL_SUCC;
}

uint32_t hmac_chr_get_chip_info(uint32_t chr_event_id)
{
    uint8_t vap_index;
    mac_vap_stru *mac_vap = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_chr_info *chr_info = NULL;
    uint16_t chr_flag = 0;
    uint32_t ret;

    mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        oam_error_log0(0, OAM_SF_ACS, "{hmac_chr_get_chip_info::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_device->uc_vap_num == 0) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    chr_info = (hmac_chr_info *)oal_memalloc(sizeof(hmac_chr_info));
    if (chr_info == NULL) {
        oam_error_log0(0, OAM_SF_ACS, "{hmac_chr_get_chip_info::hmac_chr_info alloc mem fail!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (vap_index = 0; vap_index < mac_device->uc_vap_num; vap_index++) {
        mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->auc_vap_id[vap_index]);
        if (mac_vap == NULL) {
            continue;
        }

        if (!mac_vap_is_up(mac_vap)) {
            continue;
        }

        memset_s(chr_info, sizeof(hmac_chr_info), 0, sizeof(hmac_chr_info));
#ifndef _PRE_LINUX_TEST
        if (is_legacy_sta(mac_vap)) {
            chr_flag = CHR_HOST;
        } else if (is_legacy_ap(mac_vap)) {
            chr_flag = CHR_HOST_AP;
        } else if (is_p2p_cl(mac_vap) || is_p2p_go(mac_vap)) {
            chr_flag = CHR_HOST_P2P;
        }
#endif
        hmac_chr_get_info(mac_vap, chr_info);
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ACS,
                         "{hmac_chr_get_chip_info::chr_event_id=[%d] chr_flag=[%d].}", chr_event_id, chr_flag);
        chr_exception_q(chr_event_id, chr_flag, (uint8_t *)chr_info, sizeof(hmac_chr_info));
    }
    /* 清除全局变量的历史值 */
    hmac_chr_info_clean();
    oal_free(chr_info);

    ret = hmac_chr_get_chip_info_new_arch(chr_event_id, mac_device);
    return ret;
}

uint32_t hmac_get_chr_info_event_hander(uint32_t chr_event_id)
{
    uint32_t ret;

    switch (chr_event_id) {
        case CHR_WIFI_DISCONNECT_QUERY_EVENTID:
        case CHR_WIFI_CONNECT_FAIL_QUERY_EVENTID:
        case CHR_WIFI_WEB_FAIL_QUERY_EVENTID:
        case CHR_WIFI_WEB_SLOW_QUERY_EVENTID:
        case CHR_WIFI_P2P_TRANS_FAIL_EVENTID:
        case CHR_WIFI_CLONE_FAIL_EVENTID:
        case CHR_WIFI_HUAWEISHARE_FAIL_EVENTID:
            ret = hmac_chr_get_chip_info(chr_event_id);
            if (ret != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_chr_get_chip_info::hmac_chr_get_web_fail_slow_info fail.}");
                return ret;
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}
#endif

void hmac_chr_connect_fail_query_and_report(hmac_vap_stru *pst_hmac_vap,
    mac_status_code_enum_uint16 connet_code)
{
    mac_chr_connect_fail_report_stru st_chr_connect_fail_report = { 0 };

    if (is_legacy_sta(&pst_hmac_vap->st_vap_base_info)) {
        /* 主动查询 */
        hmac_chr_set_connect_code(connet_code);
        /* 主动上报 */
#ifdef CONFIG_HW_GET_EXT_SIG
        st_chr_connect_fail_report.noise = pst_hmac_vap->station_info.noise;
        st_chr_connect_fail_report.chload = pst_hmac_vap->station_info.chload;
#endif
        st_chr_connect_fail_report.c_signal = pst_hmac_vap->station_info.signal;
        st_chr_connect_fail_report.uc_distance = pst_hmac_vap->st_station_info_extend.uc_distance;
        st_chr_connect_fail_report.uc_cca_intr = pst_hmac_vap->st_station_info_extend.uc_cca_intr;
        st_chr_connect_fail_report.snr = oal_max(pst_hmac_vap->st_station_info_extend.c_snr_ant0,
                                                 pst_hmac_vap->st_station_info_extend.c_snr_ant1);
        st_chr_connect_fail_report.us_err_code = connet_code;
        chr_exception_p(CHR_WIFI_CONNECT_FAIL_REPORT_EVENTID, (uint8_t *)(&st_chr_connect_fail_report),
                        sizeof(mac_chr_connect_fail_report_stru));
    }

    return;
}
