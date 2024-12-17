/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : HMAC模块初始化与卸载
 * 创建日期 : 2012年9月18日
 */
#include "oal_ext_if.h"
#include "hmac_auto_cpufreq.h"
#include "hmac_low_latency.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "wlan_chip_i.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_board.h"
#include "hmac_main.h"
#include "hmac_chan_det.h"
#include "hmac_fsm.h"
#include "hmac_vap.h"
#include "hmac_sysfs_stat.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_tx_complete.h"
#include "hmac_chan_mgmt.h"
#include "hmac_dfs.h"
#include "hmac_rx_filter.h"
#include "hmac_host_rx.h"
#include "hmac_hcc_adapt.h"
#include "hmac_vsp_if.h"
#include "hmac_config.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_dfx.h"
#include "hmac_cali_mgmt.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_pm.h"
#include "oal_hcc_host_if.h"
#include "oal_net.h"
#include "hmac_tcp_opt.h"
#include "hmac_ext_if.h"
#include "hmac_auto_adjust_freq.h"
#include "mac_board.h"
#include "hmac_wifi_delay.h"
#if(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif
#include "hmac_thread.h"
#include "hmac_11i.h"
#ifdef _PRE_WLAN_FEATURE_DYN_CLOSED
#include <linux/io.h>
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_11ax.h"
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hmac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "hmac_sniffer.h"
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
#include "hmac_csi.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_TWT
#include "hmac_twt.h"
#endif
#include "hmac_dbac.h"
#include "securec.h"
#include "securectype.h"
#include "host_hal_ext_if.h"
#include "hmac_tx_switch.h"
#include "hmac_host_al_tx.h"
#include "hmac_tid_ring_switch.h"
#include "hmac_tid_update.h"
#include "hmac_tid_sche.h"
#ifdef _PRE_WLAN_FEATURE_VSP
#include "hmac_vsp_test.h"
#endif
#include "hmac_tx_ring_alloc.h"
#include "hmac_dyn_pcie.h"
#ifdef _PRE_WLAN_FEATURE_HID2D
#include "hmac_hid2d.h"
#endif
#include "hmac_blockack.h"
#ifdef _PRE_WLAN_FEATURE_WAPI
#include "hmac_wapi.h"
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
#include "hmac_pwl.h"
#endif
#include "hisi_conn_nve_interface_gf61.h"
#include "hmac_sme_sta.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MAIN_C

/* 2 全局变量定义 */
/* hmac模块板子的全局控制变量 */
mac_board_stru g_st_hmac_board;
oal_wakelock_stru g_st_hmac_wakelock;
oal_bool_enum_uint8 g_feature_switch[HMAC_FEATURE_SWITCH_BUTT] = {0};
hmac_rxdata_thread_stru g_st_rxdata_thread;

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
OAL_STATIC declare_wifi_panic_stru(hmac_panic_vap_stat, hmac_print_vap_stat);
#endif
hmac_rxdata_thread_stru *hmac_get_rxdata_thread_addr(void)
{
    return &g_st_rxdata_thread;
}

/* 3 函数实现 */
/*
 * 函 数 名  : hmac_board_get_instance
 * 功能描述  : 获取HMAC board对象
 * 1.日    期  : 2012年11月16日
 *   修改内容  : 新生成函数
 */
void hmac_board_get_instance(mac_board_stru **ppst_hmac_board)
{
    *ppst_hmac_board = &g_st_mac_board;
}

oal_bool_enum_uint8 hmac_get_feature_switch(hmac_feature_switch_type_enum_uint8 feature_id)
{
    if (oal_unlikely(feature_id >= HMAC_FEATURE_SWITCH_BUTT)) {
        return OAL_FALSE;
    }

    return g_feature_switch[feature_id];
}

/*
 * 函 数 名  : hmac_sdt_recv_reg_cmd
 * 功能描述  : SDT下发读写寄存器命令，在hmac层抛事件给dmac来处理
 * 1.日    期  : 2013年10月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_sdt_recv_reg_cmd(mac_vap_stru *pst_mac_vap,
                               uint8_t *puc_buf,
                               uint16_t us_len)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event = NULL;

    pst_event_mem = frw_event_alloc_m(us_len - OAL_IF_NAME_SIZE);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sdt_recv_reg_cmd::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_CRX,
                       HMAC_TO_DMAC_SYN_REG,
                       (uint16_t)(us_len - OAL_IF_NAME_SIZE),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    if (memcpy_s(pst_event->auc_event_data, us_len - OAL_IF_NAME_SIZE,
        puc_buf + OAL_IF_NAME_SIZE, us_len - OAL_IF_NAME_SIZE) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_sdt_recv_reg_cmd::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_sdt_up_reg_val
 * 功能描述  : hmac接收dmac抛回来的寄存器的值，然后交给wal，wal上报SDT
 * 1.日    期  : 2013年10月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_sdt_up_reg_val(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    uint32_t *pst_reg_val = NULL;

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY, "{hmac_sdt_up_reg_val::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_reg_val = (uint32_t *)pst_hmac_vap->st_cfg_priv.ac_rsp_msg;
    *pst_reg_val = *((uint32_t *)pst_event->auc_event_data);

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_cfg_priv.en_wait_ack_for_sdt_reg = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->st_cfg_priv.st_wait_queue_for_sdt_reg));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_sdt_up_reg_val
 * 功能描述  : hmac接收dmac抛回来的寄存器的值，然后交给wal，wal上报SDT
 * 1.日    期  : 2013年10月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_sdt_up_dpd_data(frw_event_mem_stru *pst_event_mem)
{
    uint16_t us_payload_len;
    frw_event_stru *pst_event = NULL;
    uint8_t *puc_payload = NULL;
    frw_event_hdr_stru *pst_event_hdr = NULL;
    hal_cali_hal2hmac_event_stru *pst_cali_save_event = NULL;
    int8_t *pc_print_buff = NULL;

    oam_error_log0(0, 0, "hmac_sdt_up_dpd_data");
    if (pst_event_mem == NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_sdt_up_sample_data::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_cali_save_event = (hal_cali_hal2hmac_event_stru *)pst_event->auc_event_data;

    puc_payload = (uint8_t *)oal_netbuf_data(pst_cali_save_event->pst_netbuf);

    pst_event_hdr = &(pst_event->st_event_hdr);
    us_payload_len = OAL_STRLEN(puc_payload);

    oam_error_log1(0, 0, "hmac dpd payload len %d", us_payload_len);

    pc_print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_sdt_up_sample_data::pc_print_buff null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ???????OAM_REPORT_MAX_STRING_LEN,???oam_print */
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    if (memcpy_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, puc_payload, us_payload_len) != EOK) {
        oam_error_log0(0, OAM_SF_WPA, "hmac_sdt_up_dpd_data::memcpy fail!");
        oal_mem_free_m(pc_print_buff, OAL_TRUE);
        return OAL_FAIL;
    }

    pc_print_buff[us_payload_len] = '\0';
    oam_print(pc_print_buff);

    oal_mem_free_m(pc_print_buff, OAL_TRUE);

    return OAL_SUCC;
}

uint32_t hmac_sdt_up_pow_diff_data(frw_event_mem_stru *pst_event_mem)
{
#ifdef HISI_CONN_NVE_SUPPORT
    frw_event_stru *pst_event = NULL;
    uint8_t *puc_payload = NULL;
    frw_event_hdr_stru *pst_event_hdr = NULL;
    hal_cali_hal2hmac_event_stru *pst_cali_save_event = NULL;
    hmac_vap_stru *hmac_vap;

    if (pst_event_mem == NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_sdt_up_pow_diff_data::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_event = frw_get_event_stru(pst_event_mem);

    hmac_vap = mac_res_get_hmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_report_pow_diff::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    pst_cali_save_event = (hal_cali_hal2hmac_event_stru *)pst_event->auc_event_data;

    puc_payload = (uint8_t *)oal_netbuf_data(pst_cali_save_event->pst_netbuf);

    pst_event_hdr = &(pst_event->st_event_hdr);

    if (memcpy_s(&g_pow_cail, sizeof(nv_pow_stru_gf61), puc_payload, sizeof(nv_pow_stru_gf61)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sdt_up_pow_diff_data::memcpy fail.}");
        return OAL_FAIL;
    }

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    hmac_vap->uc_efuse_cali_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(hmac_vap->query_wait_q));
#endif
    return OAL_SUCC;
}

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
/*
 * 函 数 名  : hmac_sdt_recv_sample_cmd
 * 功能描述  : SDT下发数采命令，在hmac层抛事件给dmac来处理
 * 1.日    期  : 2016年7月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_sdt_recv_sample_cmd(mac_vap_stru *pst_mac_vap,
                                  uint8_t *puc_buf,
                                  uint16_t us_len)
{
    frw_event_mem_stru *pst_event_mem;
    frw_event_stru *pst_event;

    pst_event_mem = frw_event_alloc_m(us_len - OAL_IF_NAME_SIZE);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sdt_recv_reg_cmd::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    frw_event_hdr_init(&(pst_event->st_event_hdr), FRW_EVENT_TYPE_HOST_CRX,
        HMAC_TO_DMAC_SYN_SAMPLE, (uint16_t)(us_len - OAL_IF_NAME_SIZE),
        FRW_EVENT_PIPELINE_STAGE_1, pst_mac_vap->uc_chip_id,
        pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);

    if (EOK != memcpy_s(pst_event->auc_event_data, us_len - OAL_IF_NAME_SIZE,
        puc_buf + OAL_IF_NAME_SIZE, us_len - OAL_IF_NAME_SIZE)) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_sdt_recv_sample_cmd::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return OAL_FAIL;
    }

    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_sdt_up_sample_data
 * 功能描述  : hmac接收dmac抛回来的寄存器的值，然后交给wal，wal上报SDT
 * 1.日    期  : 2013年10月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_sdt_up_sample_data(frw_event_mem_stru *pst_event_mem)
{
    uint16_t payload_len;
    frw_event_stru *pst_event;
    frw_event_stru *event_up;
    frw_event_mem_stru *pst_hmac_event_mem;
    frw_event_hdr_stru *event_hdr;
    dmac_sdt_sample_frame_stru *pst_sample_frame;
    dmac_sdt_sample_frame_stru *pst_sample_frame_syn;
    frw_event_mem_stru *pst_syn_event_mem;
    frw_event_stru *pst_event_syn;

    /* 获取事件头和事件结构体指针 */
    pst_event = frw_get_event_stru(pst_event_mem);
    event_hdr = &(pst_event->st_event_hdr);
    payload_len = event_hdr->us_length - sizeof(frw_event_hdr_stru);

    /* 抛到WAL */
    pst_hmac_event_mem = frw_event_alloc_m(payload_len);
    if (pst_hmac_event_mem == NULL) {
        oam_error_log0(event_hdr->uc_vap_id, OAM_SF_ANY, "{hmac_sdt_up_sample_data::pst_hmac_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    event_up = frw_get_event_stru(pst_hmac_event_mem);

    frw_event_hdr_init(&(event_up->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_SAMPLE_REPORT,
        payload_len, FRW_EVENT_PIPELINE_STAGE_0, event_hdr->uc_chip_id, event_hdr->uc_device_id, event_hdr->uc_vap_id);

    if (memcpy_s(event_up->auc_event_data, payload_len,
        (uint8_t *)frw_get_event_payload(pst_event_mem), payload_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_sdt_up_sample_data::memcpy fail!");
        frw_event_free_m(pst_hmac_event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_hmac_event_mem);
    frw_event_free_m(pst_hmac_event_mem);

    pst_sample_frame = (dmac_sdt_sample_frame_stru *)pst_event->auc_event_data;

    if (pst_sample_frame->count && pst_sample_frame->count <= pst_sample_frame->reg_num) {
        pst_syn_event_mem = frw_event_alloc_m(sizeof(dmac_sdt_sample_frame_stru));
        if (pst_syn_event_mem == NULL) {
            oam_error_log0(event_hdr->uc_vap_id, OAM_SF_ANY, "{hmac_sdt_up_sample_data::pst_syn_event_mem null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_event_syn = frw_get_event_stru(pst_syn_event_mem);
        /* 填写事件头 */
        frw_event_hdr_init(&(pst_event_syn->st_event_hdr), FRW_EVENT_TYPE_HOST_CRX, HMAC_TO_DMAC_SYN_SAMPLE,
            sizeof(dmac_sdt_sample_frame_stru), FRW_EVENT_PIPELINE_STAGE_1, event_hdr->uc_chip_id,
            event_hdr->uc_device_id, event_hdr->uc_vap_id);

        pst_sample_frame_syn = (dmac_sdt_sample_frame_stru *)pst_event_syn->auc_event_data;
        pst_sample_frame_syn->reg_num = pst_sample_frame->reg_num;
        pst_sample_frame_syn->count = pst_sample_frame->count;
        pst_sample_frame_syn->type = 0;
        frw_event_dispatch_event(pst_syn_event_mem);
        frw_event_free_m(pst_syn_event_mem);
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
uint32_t hmac_psd_rpt_event_process(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL; /* 事件结构体 */
    dmac_tx_event_stru *pst_ctx_event = NULL;
    oal_netbuf_stru *netbuf = NULL;
    uint32_t *word = NULL;

    if (oal_unlikely(pst_event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_psd_rpt_event_process::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_ctx_event = (dmac_tx_event_stru *)pst_event->auc_event_data;
    netbuf = pst_ctx_event->pst_netbuf;
    word = (uint32_t *)oal_netbuf_data(netbuf);
    oam_warning_log4(0, OAM_SF_ANY, "{hmac_psd_rpt_event_process::netbuf[0x%x] size[%d] word0[0x%x] word1[0x%x]",
        (uintptr_t)pst_ctx_event->pst_netbuf, pst_ctx_event->us_frame_len, word[0], word[1]);

    if (netbuf != NULL) {
        hal_host_psd_rpt_to_file_func(netbuf, pst_ctx_event->us_frame_len);
        oam_warning_log1(0, 0, "hmac_psd_rpt_event_process::free netbuf[0x%d]", (uintptr_t)netbuf);
        oal_netbuf_free(netbuf);
    }

    return OAL_SUCC;
}
#endif
/*
 * 函 数 名  : hmac_syn_info_event
 * 功能描述  : 处理协议模式同步事件
 * 1.日    期  : 2015年4月9日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_syn_info_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t relt;

    dmac_to_hmac_syn_info_event_stru *pst_syn_info_event;

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_syn_info_event = (dmac_to_hmac_syn_info_event_stru *)pst_event->auc_event_data;
    pst_hmac_user = mac_res_get_hmac_user(pst_syn_info_event->us_user_index);
    if (pst_hmac_user == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_syn_info_event: pst_hmac_user null,user_idx=%d.}",
                         pst_syn_info_event->us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = mac_res_get_mac_vap(pst_hmac_user->st_user_base_info.uc_vap_id);
    if (pst_mac_vap == NULL) {
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_syn_info_event: pst_mac_vap null! vap_idx=%d, user_idx=%d.}",
                         pst_hmac_user->st_user_base_info.uc_vap_id,
                         pst_syn_info_event->us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_set_cur_protocol_mode(&pst_hmac_user->st_user_base_info, pst_syn_info_event->uc_cur_protocol);
    mac_user_set_cur_bandwidth(&pst_hmac_user->st_user_base_info, pst_syn_info_event->uc_cur_bandwidth);
    relt = hmac_config_user_info_syn(pst_mac_vap, &pst_hmac_user->st_user_base_info);

    return relt;
}
/*
 * 函 数 名  : hmac_create_ba_event
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2013年11月21日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_create_ba_event(frw_event_mem_stru *pst_event_mem)
{
    uint8_t uc_tidno;
    frw_event_stru *pst_event = NULL;
    hmac_user_stru *pst_hmac_user = NULL;
    hmac_vap_stru *pst_hmac_vap = NULL;
    dmac_to_hmac_ctx_event_stru *pst_create_ba_event = NULL;

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_create_ba_event = (dmac_to_hmac_ctx_event_stru *)pst_event->auc_event_data;
    uc_tidno = pst_create_ba_event->uc_tid;

    pst_hmac_user = mac_res_get_hmac_user(pst_create_ba_event->us_user_index);
    if (pst_hmac_user == NULL) {
        oam_error_log1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY, "{hmac_create_ba_event::pst_hmac_user[%d] null.}",
                       pst_create_ba_event->us_user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_create_ba_event->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_ANY, "{hmac_create_ba_event::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 该tid下不允许建BA，配置命令需求 */
    if (pst_hmac_user->ast_tid_info[uc_tidno].en_ba_handle_tx_enable == OAL_FALSE) {
        return OAL_FAIL;
    }

    hmac_tx_ba_setup(pst_hmac_vap, pst_hmac_user, uc_tidno);

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_voice_aggr_event
 * 功能描述  : 处理Voice聚合同步事件
 * 1.日    期  : 2015年4月9日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hmac_voice_aggr_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru *pst_event;
    mac_vap_stru *pst_mac_vap;

    dmac_to_hmac_voice_aggr_event_stru *pst_voice_aggr_event;

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_voice_aggr_event = (dmac_to_hmac_voice_aggr_event_stru *)pst_event->auc_event_data;

    pst_mac_vap = mac_res_get_mac_vap(pst_voice_aggr_event->uc_vap_id);
    if (pst_mac_vap == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_voice_aggr_event: pst_mac_vap null! vap_idx=%d}",
                       pst_voice_aggr_event->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap->bit_voice_aggr = pst_voice_aggr_event->en_voice_aggr;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_CHBA_MGMT
static void hmac_event_fsm_tx_adapt_chba_register(void)
{
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CHBA_PARAMS].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CHBA_SELF_CONN_INFO_SYNC].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CHBA_TOPO_INFO_SYNC].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
}
#endif
OAL_STATIC void hmac_event_fsm_netbuf_tx_adapt_subtable_register(void)
{
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_HMAC2DMAC].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_MATRIX_HMAC2DMAC].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_APP_IE_H2D].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
#ifdef _PRE_WLAN_FEATURE_APF
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_APF_CMD].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
    hmac_event_fsm_tx_adapt_chba_register();
#endif
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CUST_HMAC2DMAC].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU_HW
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_ADD_MULTI_USER].p_tx_adapt_func =
        hmac_proc_add_user_tx_adapt;
#endif
}
/*
 * 函 数 名  : hmac_event_fsm_tx_adapt_subtable_register
 * 功能描述  : hmac模块事件发送适配子表的注册函数
 * 1.日    期  : 2014年10月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_event_fsm_tx_adapt_subtable_register(void)
{
    /* 注册WLAN_CTX事件处理函数表 */
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT].p_tx_adapt_func = hmac_proc_tx_host_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_ADD_USER].p_tx_adapt_func =
        hmac_proc_add_user_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_DEL_USER].p_tx_adapt_func =
        hmac_proc_del_user_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_EDCA_REG].p_tx_adapt_func =
        hmac_proc_set_edca_param_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_IP_FILTER].p_tx_adapt_func =
        hmac_send_event_netbuf_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCHED_SCAN_REQ].p_tx_adapt_func =
        hmac_send_event_memaddr_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_SET_REG].p_tx_adapt_func =
        hmac_proc_join_set_reg_event_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_DTIM_TSF_REG].p_tx_adapt_func =
        hmac_proc_join_set_dtim_reg_event_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CONN_RESULT].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint32;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_NOTIFY_ALG_ADD_USER].p_tx_adapt_func =
        hmac_user_add_notify_alg_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_BA_SYNC].p_tx_adapt_func =
        hmac_proc_rx_process_sync_event_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WALN_CTX_EVENT_SUB_TYPR_SELECT_CHAN].p_tx_adapt_func =
        hmac_chan_select_channel_mac_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_SWITCH_TO_NEW_CHAN].p_tx_adapt_func =
        hmac_chan_initiate_switch_to_new_channel_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_RESTART_NETWORK].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint16;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WALN_CTX_EVENT_SUB_TYPR_DISABLE_TX].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint16;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WALN_CTX_EVENT_SUB_TYPR_ENABLE_TX].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint16;

    /* 注册HOST_DRX事件处理函数表 */
    g_ast_host_tx_dmac_drx[DMAC_TX_HOST_DRX].p_tx_adapt_func = hmac_proc_tx_host_tx_adapt;
    /* 调度事件通用接口 */
    g_ast_host_tx_dmac_drx[HMAC_TX_DMAC_SCH].p_tx_adapt_func = hmac_proc_config_syn_tx_adapt;
    g_ast_host_tx_dmac_drx[DMAC_TX_HOST_DTX].p_tx_adapt_func = hmac_hcc_tx_device_ring_tx_adapt;

    /* 注册HOST_CRX事件处理函数表 */
    g_ast_host_dmac_crx_table[HMAC_TO_DMAC_SYN_INIT].p_tx_adapt_func = hmac_hcc_tx_convert_event_to_netbuf_uint16;
    g_ast_host_dmac_crx_table[HMAC_TO_DMAC_SYN_CREATE_CFG_VAP].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint16;
    g_ast_host_dmac_crx_table[HMAC_TO_DMAC_SYN_REG].p_tx_adapt_func = hmac_sdt_recv_reg_cmd_tx_adapt;
    g_ast_host_dmac_crx_table[HMAC_TO_DMAC_SYN_CFG].p_tx_adapt_func = hmac_proc_config_syn_tx_adapt;
    g_ast_host_dmac_crx_table[HMAC_TO_DMAC_SYN_ALG].p_tx_adapt_func = hmac_proc_config_syn_alg_tx_adapt;
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    g_ast_host_dmac_crx_table[HMAC_TO_DMAC_SYN_SAMPLE].p_tx_adapt_func = hmac_sdt_recv_sample_cmd_tx_adapt;
#endif
    hmac_event_fsm_netbuf_tx_adapt_subtable_register();
}

/*
 * 函 数 名  : hmac_event_fsm_tx_adapt_subtable_register_ext
 * 功能描述  : hmac模块事件发送适配子表的注册函数扩展
 * 1.日    期  : 2019年12月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_event_fsm_tx_adapt_subtable_register_ext(void)
{
#ifdef _PRE_WLAN_FEATURE_DFS
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_SWITCH_TO_OFF_CHAN].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint16;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_SWITCH_TO_HOME_CHAN].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint16;
#endif
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WALN_CTX_EVENT_SUB_TYPR_DFS_CAC_CTRL_TX].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint16;
#endif
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_RESET_PSM].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint16;

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_EDCA_OPT].p_tx_adapt_func =
        hmac_edca_opt_stat_event_tx_adapt;
#endif
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_DSCR_OPT].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint8;

#ifdef _PRE_WLAN_FEATURE_11AX
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_MU_EDCA_REG].p_tx_adapt_func =
        hmac_proc_set_mu_edca_param_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_SPATIAL_REUSE_REG].p_tx_adapt_func =
        hmac_proc_set_spatial_reuse_param_tx_adapt;
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_NDP_FEEDBACK_REPORT_REG].p_tx_adapt_func =
        hmac_hcc_tx_convert_event_to_netbuf_uint8;
#endif

#ifdef _PRE_WLAN_FEATURE_TWT
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_UPDATE_TWT].p_tx_adapt_func =
        hmac_proc_rx_process_twt_sync_event_tx_adapt;
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
    g_ast_host_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_HIEX_H2D_MSG].p_tx_adapt_func =
        g_wlan_spec_cfg->feature_hiex_is_open ? hmac_hiex_h2d_msg_tx_adapt : NULL;
#endif
}

OAL_STATIC void hmac_event_fsm_rx_dft_adapt_subtable_register(void)
{
    frw_event_sub_rx_adapt_table_init(g_ast_hmac_wlan_drx_event_sub_table,
        sizeof(g_ast_hmac_wlan_drx_event_sub_table) / sizeof(frw_event_sub_table_item_stru),
        hmac_hcc_rx_convert_netbuf_to_event_default);

    frw_event_sub_rx_adapt_table_init(g_ast_hmac_wlan_crx_event_sub_table,
        sizeof(g_ast_hmac_wlan_crx_event_sub_table) / sizeof(frw_event_sub_table_item_stru),
        hmac_hcc_rx_convert_netbuf_to_event_default);

    frw_event_sub_rx_adapt_table_init(g_ast_hmac_wlan_ctx_event_sub_table,
        sizeof(g_ast_hmac_wlan_ctx_event_sub_table) / sizeof(frw_event_sub_table_item_stru),
        hmac_hcc_rx_convert_netbuf_to_event_default);

    frw_event_sub_rx_adapt_table_init(g_ast_hmac_wlan_misc_event_sub_table,
        sizeof(g_ast_hmac_wlan_misc_event_sub_table) / sizeof(frw_event_sub_table_item_stru),
        hmac_hcc_rx_convert_netbuf_to_event_default);
}

/*
 * 函 数 名  : hmac_event_fsm_rx_adapt_subtable_register
 * 功能描述  : hmac模块事件接收适配子表的注册函数
 * 1.日    期  : 2014年10月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_event_fsm_rx_adapt_subtable_register(void)
{
    hmac_event_fsm_rx_dft_adapt_subtable_register();

    /* 注册HMAC模块WLAN_DRX事件子表 */
    g_ast_hmac_wlan_drx_event_sub_table[DMAC_WLAN_DRX_EVENT_SUB_TYPE_RX_DATA].p_rx_adapt_func =
        hmac_rx_process_data_rx_adapt;

    /* hcc hdr修改影响rom，通过增加event方式处理不同传输方式 */
    g_ast_hmac_wlan_drx_event_sub_table[DMAC_WLAN_DRX_EVENT_SUB_TYPE_WOW_RX_DATA].p_rx_adapt_func =
        hmac_rx_process_wow_data_rx_adapt;

    /* 注册HMAC模块WLAN_CRX事件子表 */
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_INIT].p_rx_adapt_func =
        hmac_rx_convert_netbuf_to_netbuf_default;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_RX].p_rx_adapt_func =
        hmac_rx_process_mgmt_event_rx_adapt;

    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_RX_WOW].p_rx_adapt_func =
        hmac_rx_process_wow_mgmt_event_rx_adapt;

    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_EVERY_SCAN_RESULT].p_rx_adapt_func =
        hmac_rx_convert_netbuf_to_netbuf_default;

    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_EVERY_SCAN_RESULT_RING].p_rx_adapt_func =
        hmac_rx_convert_netbuf_to_netbuf_ring;
    /* 注册MISC事件子表 */
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_CALI_TO_HMAC].p_rx_adapt_func =
        hmac_cali2hmac_event_rx_adapt;
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_TO_HMAC_DPD].p_rx_adapt_func = hmac_cali2hmac_event_rx_adapt;

#ifdef _PRE_WLAN_FEATURE_APF
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_APF_REPORT].p_rx_adapt_func =
        hmac_apf_program_report_rx_adapt;
#endif
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_PSD_RPT].p_rx_adapt_func =
        hmac_rx_convert_netbuf_to_netbuf_default;
#endif
#ifdef _PRE_DELAY_DEBUG
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_WIFI_DELAY_RPT].p_rx_adapt_func =
        hmac_rx_convert_netbuf_to_netbuf_default;
#endif
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_TO_HMAC_POW_DIFF].p_rx_adapt_func = hmac_cali2hmac_event_rx_adapt;
}

OAL_STATIC void hmac_drx_event_subtable_register(void)
{
    /* 注册HMAC模块WLAN_DRX事件子表 */
    g_ast_hmac_wlan_drx_event_sub_table[DMAC_WLAN_DRX_EVENT_SUB_TYPE_RX_DATA].p_func =
        hmac_rx_process_data_event;

    g_ast_hmac_wlan_drx_event_sub_table[DMAC_WLAN_DRX_EVENT_SUB_TYPE_WOW_RX_DATA].p_func =
        hmac_dev_rx_process_data_event;

    /* AP 和STA 公共，注册HMAC模块WLAN_DRX事件子表 */
    g_ast_hmac_wlan_drx_event_sub_table[DMAC_WLAN_DRX_EVENT_SUB_TYPE_TKIP_MIC_FAILE].p_func =
        hmac_rx_tkip_mic_failure_process;
}

OAL_STATIC void hmac_crx_event_subtable_register(void)
{
    /* 注册HMAC模块WLAN_CRX事件子表 */
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_INIT].p_func = hmac_init_event_process;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_RX].p_func = hmac_rx_process_mgmt_event;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_RX_WOW].p_func =
        hmac_rx_process_wow_mgmt_event;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_DELBA].p_func = hmac_mgmt_rx_delba_event;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_EVERY_SCAN_RESULT].p_func =
        hmac_scan_proc_scanned_bss;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_EVERY_SCAN_RESULT_RING].p_func =
        hmac_scan_proc_scanned_bss;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_SCAN_COMP].p_func =
        hmac_scan_proc_scan_comp_event;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_CHAN_RESULT].p_func =
        hmac_scan_process_chan_result_event;
#ifdef _PRE_WLAN_FEATURE_HID2D
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_HID2D_CHAN_MEAS_RESULT].p_func =
        hmac_scan_hid2d_chan_meas_result_event;
#endif
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_DISASSOC].p_func =
        hmac_mgmt_send_disasoc_deauth_event;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_DEAUTH].p_func =
        hmac_mgmt_send_disasoc_deauth_event;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPR_CH_SWITCH_COMPLETE].p_func =
        hmac_chan_switch_to_new_chan_complete;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPR_DBAC].p_func = hmac_dbac_status_notify;
#ifdef _PRE_WLAN_FEATURE_HIEX
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_HIEX_D2H_MSG].p_func =
        g_wlan_spec_cfg->feature_hiex_is_open ? hmac_hiex_rx_local_msg : NULL;
#endif
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_TX_RING_ADDR].p_func =
        hmac_set_tx_ring_device_base_addr;
#ifdef _PRE_WLAN_FEATURE_VSP
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_VSP_INFO_ADDR].p_func = hmac_set_vsp_info_addr;
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_DRX_EVENT_SUB_TYPE_RX_SNIFFER_INFO].p_func =
        hmac_sniffer_rx_info_event;
#endif
#ifdef _PRE_DELAY_DEBUG
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_WIFI_DELAY_RPT].p_func =
        hmac_delay_rpt_event_process;
#endif
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_PSD_RPT].p_func = hmac_psd_rpt_event_process;
#endif
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_CHAN_DET_RPT].p_func =
        hmac_chan_det_rpt_event_process;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_GC_JOIN_WAIT_NOA_END].p_func =
        hmac_connect_gc_join_wait_noa_end;
}

OAL_STATIC void hmac_ctx_event_subtable_register(void)
{
    /* 注册发向HOST侧的配置事件子表 */
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_SYN_UP_REG_VAL].p_func = hmac_sdt_up_reg_val;

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_SYN_UP_SAMPLE_DATA].p_func = hmac_sdt_up_sample_data;
#endif
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_CREATE_BA].p_func = hmac_create_ba_event;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_DEL_BA].p_func = hmac_del_ba_event;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_SYN_CFG].p_func = hmac_event_config_syn;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_ALG_INFO_SYN].p_func = hmac_syn_info_event;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_VOICE_AGGR].p_func = hmac_voice_aggr_event;
#ifdef _PRE_WLAN_FEATURE_M2S
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_M2S_DATA].p_func = hmac_m2s_sync_event;
#endif
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_BANDWIDTH_INFO_SYN].p_func = hmac_bandwidth_info_syn_event;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_PROTECTION_INFO_SYN].p_func = hmac_protection_info_syn_event;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_CH_STATUS_INFO_SYN].p_func = hmac_ch_status_info_syn_event;
}

/*
 * 函 数 名  : hmac_event_fsm_action_subtable_register
 * 功能描述  : hmac模块事件执行子表的注册函数
 * 1.日    期  : 2014年10月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hmac_event_fsm_action_subtable_register(void)
{
    hmac_drx_event_subtable_register();

    /* 将事件类型和调用函数的数组注册到事件调度模块 */
    /* 注册WLAN_DTX事件子表 */
    g_ast_hmac_wlan_dtx_event_sub_table[DMAC_TX_WLAN_DTX].p_func = hmac_tx_wlan_to_wlan_ap;

    hmac_crx_event_subtable_register();

    /* 注册发向HOST侧的配置事件子表 */
    hmac_ctx_event_subtable_register();

#ifdef _PRE_WLAN_FEATURE_DFS
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_RADAR_DETECT].p_func = hmac_dfs_radar_detect_event;
#endif /* end of _PRE_WLAN_FEATURE_DFS */
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_DISASOC].p_func = hmac_proc_disasoc_misc_event;
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_PSM_GET_HOST_RING_STATE].p_func = hmac_get_host_ring_state;

    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_ROAM_TRIGGER].p_func = hmac_proc_roam_trigger_event;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_CALI_TO_HMAC].p_func = wlan_chip_save_cali_event;
#endif
    g_ast_hmac_ddr_drx_sub_table[HAL_WLAN_DDR_DRX_EVENT_SUBTYPE].p_func = hmac_host_rx_ring_data_event;

#ifdef _PRE_WLAN_FEATURE_CSI
    g_ast_hmac_ddr_drx_sub_table[HAL_WLAN_FTM_IRQ_EVENT_SUBTYPE].p_func = hmac_rx_location_data_event;
#endif
#ifdef _PRE_WLAN_FEATURE_VSP
    g_ast_hmac_ddr_drx_sub_table[HAL_WLAN_COMMON_TIMEOUT_IRQ_EVENT_SUBTYPE].p_func = hmac_vsp_common_timeout;
#endif

    g_ast_hmac_wlan_misc_event_sub_table[DMAC_TO_HMAC_DPD].p_func = hmac_sdt_up_dpd_data;
#ifdef _PRE_WLAN_FEATURE_APF
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_APF_REPORT].p_func = hmac_apf_program_report_event;
#endif
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_TX_SWITCH_STATE].p_func = hmac_tx_mode_switch_process;
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_TO_HMAC_POW_DIFF].p_func = hmac_sdt_up_pow_diff_data;
}

/*
 * 函 数 名  : hmac_event_fsm_register
 * 功能描述  : hmac模块事件及其处理函数的注册函数
 * 1.日    期  : 2014年10月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_event_fsm_register(void)
{
    /* 注册所有事件的tx adapt子表 */
    hmac_event_fsm_tx_adapt_subtable_register();
    hmac_event_fsm_tx_adapt_subtable_register_ext();

    /* 注册所有事件的rx adapt子表 */
    hmac_event_fsm_rx_adapt_subtable_register();

    /* 注册所有事件的执行函数子表 */
    hmac_event_fsm_action_subtable_register();

    event_fsm_table_register();

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_param_check
 * 功能描述  : Check the struct and var used in hmac
 * 1.日    期  : 2015年5月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t hmac_param_check(void)
{
    /* netbuf's cb size! */
    uint32_t netbuf_cb_size = (uint32_t)oal_netbuf_cb_size();
    if (netbuf_cb_size < (uint32_t)sizeof(mac_tx_ctl_stru)) {
        oal_io_print("mac_tx_ctl_stru size[%u] large then netbuf cb max size[%u]\n",
                     netbuf_cb_size, (uint32_t)sizeof(mac_tx_ctl_stru));
        return OAL_EFAIL;
    }

    if (netbuf_cb_size < (uint32_t)sizeof(mac_rx_ctl_stru)) {
        oal_io_print("mac_rx_ctl_stru size[%u] large then netbuf cb max size[%u]\n",
                     netbuf_cb_size, (uint32_t)sizeof(mac_rx_ctl_stru));
        return OAL_EFAIL;
    }
    return OAL_SUCC;
}

/*
 * 功能描述  :hmac_main_init初始化扩展函数
 * 1.日    期  : 2020年8月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hmac_main_init_extend(void)
{
    /* DFX 模块初始化 */
    hmac_dfx_init();

#ifdef _PRE_WINDOWS_SUPPORT
    hcc_flowctl_get_device_mode_register(hmac_flowctl_check_device_is_sta_mode, HCC_EP_WIFI_DEV);
    hcc_flowctl_operate_subq_register(hmac_vap_net_start_subqueue, hmac_vap_net_stop_subqueue,
                                      HCC_EP_WIFI_DEV);
#endif

#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    hmac_sysfs_entry_init();
#ifdef _PRE_WLAN_FEATURE_VSP
    hmac_vsp_init_sysfs();
#endif
#endif

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
    hwifi_panic_log_register(&hmac_panic_vap_stat, NULL);
#endif

    hmac_register_pm_callback();
#ifdef _PRE_WLAN_FEATURE_LOW_LATENCY_SWITCHING
    hmac_register_pmqos_network_latency_handler();
#endif
    memset_s(g_always_tx_ring, sizeof(g_always_tx_ring), 0, sizeof(g_always_tx_ring));

#ifdef _PRE_WLAN_FEATURE_WAPI
    hmac_wapi_init();
#endif
#ifdef _PRE_WLAN_FEATURE_PWL
    hmac_pwl_init();
#endif
}

static void hmac_host_feature_init(void)
{
#ifdef _PRE_HOST_PERFORMANCE
    host_time_init();
#endif
    hmac_hisi_cpufreq_init();
    hmac_tid_schedule_init();
    hmac_tid_ring_switch_init();
    hmac_tid_update_init();
    hmac_init_user_lut_index_tbl();
}

/*
 * 函 数 名  : hmac_main_init
 * 功能描述  : HMAC模块初始化总入口，包含HMAC模块内部所有特性的初始化。
 * 1.日    期  : 2012年9月18日
 *   修改内容  : 新生成函数
 */
int32_t hmac_main_init(void)
{
    uint32_t ret;
    uint16_t en_init_state;
    oal_wake_lock_init(&g_st_hmac_wakelock, "wlan_hmac_wakelock");

    /* 为了解各模块的启动时间，增加时间戳打印 */
    if (OAL_SUCC != hmac_param_check()) {
        oal_io_print("hmac_main_init:hmac_param_check failed!\n");
        return -OAL_EFAIL;
    }

    hmac_hcc_adapt_init();

    en_init_state = frw_get_init_state();
    if (oal_unlikely((en_init_state == FRW_INIT_STATE_BUTT) || (en_init_state < FRW_INIT_STATE_FRW_SUCC))) {
        oal_io_print("hmac_main_init:en_init_state is error %d\n", en_init_state);
        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }
    hmac_wifi_auto_freq_ctrl_init();

    if (hmac_hisi_thread_init() != OAL_SUCC) {
        frw_timer_delete_all_timer();
        oal_io_print("hmac_main_init: hmac_hisi_thread_init failed\n");

        return -OAL_EFAIL;
    }

    hmac_host_feature_init();

    ret = mac_res_init();
    if (ret != OAL_SUCC) {
        oal_io_print("hmac_main_init: mac_res_init return err code %d\n", ret);
        frw_timer_delete_all_timer();
        return -OAL_EFAIL;
    }

    /* hmac资源初始化 */
    hmac_res_init();

    /* 如果初始化状态处于配置VAP成功前的状态，表明此次为HMAC第一次初始化，即重加载或启动初始化 */
    if (en_init_state < FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC) {
        /* 调用状态机初始化接口 */
        hmac_fsm_init();

        /* 事件注册 */
        hmac_event_fsm_register();

        ret = hmac_board_init(&g_st_mac_board);
        if (ret != OAL_SUCC) {
            frw_timer_delete_all_timer();
            event_fsm_unregister();
            mac_res_exit();
            hmac_res_exit(&g_st_mac_board); /* 释放hmac res资源 */
            return OAL_FAIL;
        }

        frw_set_init_state(FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC);

        /* 启动成功后，输出打印 */
    }

    hmac_main_init_extend();
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_main_exit
 * 功能描述  : HMAC模块卸载
 * 1.日    期  : 2012年9月18日
 *   修改内容  : 新生成函数
 */
void hmac_main_exit(void)
{
    uint32_t ret;
#ifdef _PRE_WLAN_FEATURE_LOW_LATENCY_SWITCHING
    hmac_unregister_pmqos_network_latency_handler();
#endif
#if defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    hmac_sysfs_entry_exit();
#ifdef _PRE_WLAN_FEATURE_VSP
    hmac_vsp_deinit_sysfs();
#endif
#endif
    event_fsm_unregister();

    hmac_hisi_thread_exit();

    hmac_hisi_cpufreq_exit();

    hmac_tid_ring_switch_deinit();

    ret = hmac_board_exit(&g_st_mac_board);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_main_exit::hmac_board_exit failed[%d].}", ret);
        return;
    }

    /* DFX 模块初始化 */
    hmac_dfx_exit();

    hmac_wifi_auto_freq_ctrl_deinit();

    frw_set_init_state(FRW_INIT_STATE_FRW_SUCC);

    oal_wake_lock_exit(&g_st_hmac_wakelock);

    frw_timer_clean_timer(OAM_MODULE_ID_HMAC);
}
oal_module_license("GPL");
