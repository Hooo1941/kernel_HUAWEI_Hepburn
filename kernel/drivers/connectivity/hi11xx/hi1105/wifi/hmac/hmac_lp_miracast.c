/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI 投屏低功耗
 * 创建日期 : 2023年4月6日
 */

#include "wlan_chip.h"
#include "wlan_chip_i.h"
#include "oal_main.h"
#include "hmac_ext_if.h"
#include "hmac_cali_mgmt.h"
#include "wal_linux_ioctl.h"
#include "hmac_config.h"
#include "hmac_stat.h"

#include "hisi_customize_wifi.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_HMAC_LP_MIRACAST_C
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST

#define MBPS_10M 10
#define MBPS_60M 60
#define HIGH_THR_CNT 3
#define MID_THR_CNT 6
#define LOW_THR_CNT 9
#define HMAC_PROTOCOL_CNT 3
#define LP_MIRACAST_TIMEOUT 60000 /* 60秒 */

#define CMD_LP_ANT_SWITCH_SISO 1
#define CMD_LP_ANT_SWITCH_MIMO 3

static uint8_t hmac_get_custom_cap_5g_chain(void)
{
    uint8_t radio_cap = wlan_chip_get_chn_radio_cap();
    return radio_cap >> WLAN_CHAIN_BAND_INDICATOR;
}

static uint8_t hmac_get_custom_cap_5g_nss(void)
{
    uint8_t chain_5g = hmac_get_custom_cap_5g_chain();
    uint8_t chain_num_5g = oal_bit_get_num_one_byte(chain_5g);
    oam_warning_log2(0, OAM_SF_DBDC,
        "hmac_get_custom_cap_5g_nss:chain_2g[%d], chain_num_2g[%d]", chain_5g, chain_num_5g);
    if (chain_num_5g == 0) {
        return 0;
    }
    return chain_num_5g - 1;
}

/* 根据是否投屏低功耗下发对应的切换C1还是MIMO的命令 */
void hmac_config_control_lp_miracast(mac_vap_stru *mac_vap, uint8_t allow_switch_siso)
{
    hmac_lp_miracast_stat_stru *hmac_lp_miracast_stats = hmac_stat_get_lp_miracast_stats();

    mac_m2s_mgr_stru m2s_mgr = {0};
    int32_t chip_type = get_mpxx_subchip_type();

    if (hmac_lp_miracast_stats->bt_forbid_switch == OAL_TRUE) {
        return;
    }
    if (chip_type >= BOARD_VERSION_MP16) {
        if (allow_switch_siso == OAL_TRUE) {
            m2s_mgr.pri_data.test_mode.uc_m2s_state = CMD_LP_ANT_SWITCH_SISO; /* 1:C0  2:C1 3:C0C1 */
        } else {
            m2s_mgr.pri_data.test_mode.uc_m2s_state = CMD_LP_ANT_SWITCH_MIMO;
        }
    } else {
        if (allow_switch_siso == OAL_TRUE) {
            m2s_mgr.pri_data.test_mode.uc_m2s_state = (uint8_t)oal_atoi("1");
        } else {
            m2s_mgr.pri_data.test_mode.uc_m2s_state = (uint8_t)oal_atoi("2");
        }
    }

    hmac_lp_miracast_stats->allow_switch_siso = allow_switch_siso;
    m2s_mgr.en_cfg_m2s_mode = MAC_M2S_MODE_LP_ANT_SWITCH;

    m2s_mgr.pri_data.test_mode.uc_master_id = HAL_DEVICE_ID_MASTER;

    /* true: 硬切换测试模式采用默认软切换配置 */
    /* false: 业务切换，软切换测试模式采用默认软切换配置 */
    m2s_mgr.pri_data.test_mode.en_m2s_type =  MAC_M2S_MODE_LP_ANT_SWITCH;

    /* 标识业务类型 */
    m2s_mgr.pri_data.test_mode.uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_TEST;

    oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_M2S,
        "{hmac_config_control_lp_ant_switch: lp_ant_switch[%u][1:TO_SISO/0:TO_MIMO] m2s_state[%u].}",
        allow_switch_siso, m2s_mgr.pri_data.test_mode.uc_m2s_state);
    hmac_config_set_m2s_switch(mac_vap, sizeof(mac_m2s_mgr_stru), (uint8_t *)&m2s_mgr);
}

/*
 * 函 数 名  : hmac_lp_miracast_calc_m2s_state
 * 功能描述  : host判定协议模式，流量大小，device是否允许，计算是否进入siso
 * 1.日    期  : 2023年3月28日
 *   修改内容  : 新生成函数
 */
uint8_t hmac_lp_miracast_calc_m2s_state(wlan_protocol_enum_uint8 protocol_mode,
    hmac_lp_miracast_stat_stru *hmac_lp_miracast_stats, uint32_t throughput_mbps)
{
    if ((protocol_mode <= WLAN_HT_MODE) || (protocol_mode == WLAN_HT_ONLY_MODE) ||
        (protocol_mode == WLAN_HT_11G_MODE) || (protocol_mode == WLAN_HT_ONLY_MODE_2G)) {
        hmac_lp_miracast_stats->protocol_allow_siso = OAL_FALSE;
    } else {
        hmac_lp_miracast_stats->protocol_allow_siso = OAL_TRUE;
    }

    if (throughput_mbps < MBPS_10M) {
        hmac_lp_miracast_stats->throughput_cnt++;
        hmac_lp_miracast_stats->throughput_cnt = oal_min(hmac_lp_miracast_stats->throughput_cnt, LOW_THR_CNT);
    }

    if (throughput_mbps > MBPS_60M) {
        hmac_lp_miracast_stats->throughput_cnt--;
        hmac_lp_miracast_stats->throughput_cnt = oal_max(hmac_lp_miracast_stats->throughput_cnt, HIGH_THR_CNT);
    }

    hmac_lp_miracast_stats->host_allow_m2s_siso = ((hmac_lp_miracast_stats->throughput_cnt >= MID_THR_CNT) &&
        (hmac_lp_miracast_stats->protocol_allow_siso == OAL_TRUE)) ?
        OAL_TRUE : OAL_FALSE;

    if (hmac_lp_miracast_stats->debug_on == OAL_TRUE) {
        oam_warning_log4(0, OAM_SF_M2S, "protocol_mode[%d],throughput[%d],protocol_allow_siso[%d], throughput_cnt[%d]",
            protocol_mode, throughput_mbps, hmac_lp_miracast_stats->protocol_allow_siso,
            hmac_lp_miracast_stats->throughput_cnt);

        oam_warning_log3(0, OAM_SF_M2S,
            "host_allow[%d],device_allow[%d], allow_switch_siso[%d]", hmac_lp_miracast_stats->host_allow_m2s_siso,
            hmac_lp_miracast_stats->device_allow_m2s_siso, hmac_lp_miracast_stats->allow_switch_siso);
    }

    return (hmac_lp_miracast_stats->host_allow_m2s_siso == OAL_TRUE &&
        hmac_lp_miracast_stats->device_allow_m2s_siso == OAL_TRUE);
}

uint8_t hmac_lp_ant_switch_check_param(mac_vap_stru *mac_vap, wlan_protocol_enum_uint8 *protocol_mode)
{
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_user_stru *hmac_user = NULL;

    if (mac_vap == NULL) {
        return OAL_FALSE;
    }
    if ((is_p2p_cl(mac_vap) == OAL_FALSE) && (is_legacy_sta(mac_vap) == OAL_FALSE)) {
        return OAL_FALSE;
    }
    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_FALSE;
    }
    /* 1.获取hmac device */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_lp_ant_switch_check_param::hmac_device null.}");
        return OAL_FALSE;
    }
    /* 2.判断当前扫描是否正在执行 */
    if (hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) {
        return OAL_FALSE;
    }
    /* 3.判断当前是否正在执行漫游 */
    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_lp_ant_switch_check_param::roaming.}");
        return OAL_FALSE;
    }
    hmac_user = mac_res_get_hmac_user(hmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (hmac_user == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_M2S, "hmac_lp_ant_switch_check_param: hmac_user is null ptr.");
        return OAL_FALSE;
    }

    *protocol_mode = hmac_user->st_user_base_info.en_protocol_mode;
    return OAL_TRUE;
}

/*
 * 函 数 名  : hmac_config_lp_miracast_stat_mbps
 * 功能描述  : host统计流量，计算是否进入siso
 * 1.日    期  : 2023年3月28日
 *   修改内容  : 新生成函数
 */
void hmac_config_lp_miracast_stat_mbps(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    hmac_lp_miracast_stat_stru *hmac_lp_miracast_stats = hmac_stat_get_lp_miracast_stats();
    mac_device_stru *mac_device = mac_res_get_dev(0);
    mac_vap_stru *mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {NULL};
    uint8_t  up_vap_num;
    uint32_t throughput = tx_throughput_mbps + rx_throughput_mbps;
    uint8_t allow_switch_siso = OAL_FALSE;
    uint32_t cur_timestamp;
    wlan_protocol_enum_uint8 protocol_mode;

    if ((hmac_lp_miracast_stats->lp_miracast_enable == OAL_FALSE) ||
        (hmac_lp_miracast_stats->bt_forbid_switch == OAL_TRUE)) {
        return;
    }

    up_vap_num = mac_device_find_up_vaps(mac_device, mac_vap, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    /* 如果非单vap,则不用开启统计 */
    if (up_vap_num != 1) {
        return;
    }
    if (hmac_lp_ant_switch_check_param(mac_vap[0], &protocol_mode) == OAL_FALSE) {
        return;
    }

    allow_switch_siso = hmac_lp_miracast_calc_m2s_state(protocol_mode, hmac_lp_miracast_stats, throughput);
    cur_timestamp = oal_time_get_stamp_ms();

    if (hmac_lp_miracast_stats->allow_switch_siso != allow_switch_siso && ((allow_switch_siso == OAL_FALSE) ||
        (oal_time_after32(cur_timestamp, (hmac_lp_miracast_stats->last_time + LP_MIRACAST_TIMEOUT)) &&
        (allow_switch_siso == OAL_TRUE)))) {
        oam_warning_log4(0, OAM_SF_M2S,
            "host_allow[%d],device_allow[%d], lp_ant_switch [%d][1 SISO/0 MIMO], throughput[%d]",
            hmac_lp_miracast_stats->host_allow_m2s_siso,
            hmac_lp_miracast_stats->device_allow_m2s_siso, hmac_lp_miracast_stats->allow_switch_siso, throughput);
        hmac_config_control_lp_miracast(mac_vap[0], allow_switch_siso);
        hmac_lp_miracast_stats->last_time = cur_timestamp;
    }
}
/*
 * 函 数 名  : hmac_m2s_lp_miracast_report_state
 * 功能描述  : host获取device是否允许进入低功耗的上报参数
 * 1.日    期  : 2023年3月28日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_m2s_lp_miracast_report_state(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *param)
{
    hmac_lp_miracast_stat_stru *hmac_lp_miracast_stats = hmac_stat_get_lp_miracast_stats();
    dmac_miracast_report_stru *dmac_miracast_report = NULL;
    dmac_miracast_report = (dmac_miracast_report_stru *)param;
    if (dmac_miracast_report->abort_switch == OAL_TRUE) {
        hmac_lp_miracast_stats->allow_switch_siso = !hmac_lp_miracast_stats->allow_switch_siso;
    }
    hmac_lp_miracast_stats->device_allow_m2s_siso = dmac_miracast_report->device_allow_m2s_siso;

    hmac_lp_miracast_stats->bt_forbid_switch = dmac_miracast_report->bt_forbid_switch;
    /* bt打开之前是siso态，host会保留切换允许状态值，bt关闭后，把切换允许状态值清零 */
    if (hmac_lp_miracast_stats->bt_forbid_switch == OAL_TRUE) {
        hmac_lp_miracast_stats->allow_switch_siso = 0;
    }

    oam_warning_log3(0, OAM_SF_PWR,
        "{hmac_m2s_lp_ant_switch_report_state:: device_allow_m2s_siso[%d], allow_switch_siso[%d] bt_forbid_switch[%d]}",
        hmac_lp_miracast_stats->device_allow_m2s_siso, hmac_lp_miracast_stats->allow_switch_siso,
        hmac_lp_miracast_stats->bt_forbid_switch);
    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_config_notify_lp_miracast
 * 功能描述  : 通知底层投屏低功耗开关
 * 1.日    期  : 2023年3月16日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_notify_lp_miracast(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_SET_LP_MIRACAST, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_M2S,
            "{hmac_config_notify_lp_miracast::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
static void hmac_lp_ant_switch_param_reset(hmac_lp_miracast_stat_stru *hmac_lp_miracast_stats)
{
    uint8_t bt_forbid;
    if (hmac_lp_miracast_stats == NULL) {
        return;
    }
    bt_forbid = hmac_lp_miracast_stats->bt_forbid_switch;  /* bt打卡标记不能清零，bt关闭的时候才能清零 */
    memset_s(hmac_lp_miracast_stats, sizeof(hmac_lp_miracast_stat_stru), 0, sizeof(hmac_lp_miracast_stat_stru));
    hmac_lp_miracast_stats->throughput_cnt = MID_THR_CNT; /* 保证该值不小于等于0 */
    hmac_lp_miracast_stats->bt_forbid_switch = bt_forbid;
}
/*
 * 函 数 名  : hmac_config_set_lp_miracast
 * 功能描述  : 通知底层投屏低功耗命令下发及解析
 * 1.日    期  : 2023年3月16日
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_set_lp_miracast(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_device_stru *mac_device = NULL;
    hmac_lp_miracast_stat_stru *hmac_lp_miracast_stats = hmac_stat_get_lp_miracast_stats();
    uint8_t nss_num = hmac_get_custom_cap_5g_nss();
    mac_lp_miracast_cmd_enum lp_miracast_cmd = (mac_lp_miracast_cmd_enum)*param;

    if (nss_num == WLAN_SINGLE_NSS) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_set_lp_miracast:: single nss not support slpr}");
        return OAL_SUCC;
    }

    mac_device = mac_res_get_dev(0);
    /* 如果非单GC,则不用开启统计 */
    if (mac_device_calc_up_vap_num(mac_device) != 1) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_set_lp_miracast::vap_num is not 1.}");
        return OAL_SUCC;
    }

    if ((is_p2p_cl(mac_vap) == OAL_FALSE) && (is_legacy_sta(mac_vap) == OAL_FALSE)) {
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_config_set_lp_miracast::vap_mode[%d], p2p_mode[%d]}",
            mac_vap->en_vap_mode, mac_vap->en_p2p_mode);
        return OAL_SUCC;
    }

    switch (lp_miracast_cmd) {
        case MAC_LP_MIRACAST_ENABLE:
            if (hmac_lp_miracast_stats->lp_miracast_enable == OAL_TRUE) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_set_lp_miracast::repeat cmd enable.}");
                return OAL_SUCC;
            }
            hmac_lp_ant_switch_param_reset(hmac_lp_miracast_stats);
            hmac_lp_miracast_stats->lp_miracast_enable = OAL_TRUE;
            break;
        case MAC_LP_MIRACAST_DISABLE:
            if (hmac_lp_miracast_stats->lp_miracast_enable == OAL_FALSE) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_set_lp_miracast::repeat cmd disable.}");
                return OAL_SUCC;
            }
            hmac_lp_ant_switch_param_reset(hmac_lp_miracast_stats);
            hmac_lp_miracast_stats->lp_miracast_enable = OAL_FALSE;
            hmac_config_control_lp_miracast(mac_vap, OAL_FALSE);
            break;
        case MAC_LP_MIRACAST_DEBUG_ON:
            hmac_lp_miracast_stats->debug_on = OAL_TRUE;
            break;
        case MAC_LP_MIRACAST_DEBUG_OFF:
            hmac_lp_miracast_stats->debug_on = OAL_FALSE;
            break;
        default:
            break;
    }

    return hmac_config_notify_lp_miracast(mac_vap, len, param);
}

void hmac_config_close_lp_miracast(mac_vap_stru *mac_vap)
{
    hmac_lp_miracast_stat_stru *hmac_lp_miracast_stats = hmac_stat_get_lp_miracast_stats();
    uint8_t param = OAL_FALSE;

    if (hmac_lp_miracast_stats->lp_miracast_enable == OAL_TRUE) {
        hmac_lp_miracast_stats->lp_miracast_enable = OAL_FALSE;
        hmac_config_control_lp_miracast(mac_vap, OAL_FALSE);
        hmac_config_notify_lp_miracast(mac_vap, sizeof(uint8_t), &param);
    }
}

uint32_t hmac_config_get_m2s_state(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(mac_vap, WLAN_CFGID_GET_M2S_STATE, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_M2S,
            "{hmac_config_get_m2s_state::hmac_config_send_event failed[%d].}", ret);
    }

    return ret;
}
#endif
