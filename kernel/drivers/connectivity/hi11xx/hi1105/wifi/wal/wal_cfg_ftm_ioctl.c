/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :特性相关命令
 * 创建日期 : 2022年11月19日
 */

#include "hmac_sae.h"
#ifdef _PRE_WLAN_FEATURE_TWT
#include "hmac_twt.h"
#endif
#include "wal_linux_ioctl.h"
#include "wal_cfg_ioctl.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_FTM_CFG_IOCTL_C

#ifdef WIFI_DEBUG_ENABLE

#ifdef _PRE_WLAN_FEATURE_FTM
OAL_STATIC wal_ftm_cmd_entry_stru g_ast_ftm_common_cmd[] = {
    { "enable_ftm_initiator", OAL_TRUE, BIT0 },
    { "enable",               OAL_TRUE, BIT2 },
    { "cali",                 OAL_TRUE, BIT3 },
    { "enable_ftm_responder", OAL_TRUE, BIT5 },
    { "enable_ftm_range",     OAL_TRUE, BIT8 },
    { "en_multipath",         OAL_TRUE, BIT12 },
};

OAL_STATIC OAL_INLINE void ftm_debug_cmd_format_info(void)
{
    oam_warning_log0(0, OAM_SF_FTM,
        "{CMD format::hipriv.sh wlan0 ftm_debug \
        enable_ftm_initiator[0|1] \
        enable[0|1] \
        cali[0|1] \
        enable_ftm_responder[0|1] \
        set_ftm_time t1[] t2[] t3[] t4[] \
        set_ftm_timeout bursts_timeout ftms_timeout \
        enable_ftm_responder[0|1] \
        enable_ftm_range[0|1] \
        get_cali \
        !!}\r\n");
    oam_warning_log0(0, OAM_SF_FTM,
        "{CMD format::hipriv.sh wlan0 ftm_debug \
        send_initial_ftm_request channel[] ftms_per_burst[n] burst_num[n] en_lci_civic_request[0|1] asap[0|1] \
        bssid[xx:xx:xx:xx:xx:xx] format_bw[9~13] send_ftm bssid[xx:xx:xx:xx:xx:xx] \
        send_range_req mac[] num_rpt[0-65535] delay[] ap_cnt[] bssid[] channel[] bssid[] channel[] ...\
        set_location type[] mac[] mac[] mac[] \
        white_list index mac[xx:xx:xx:xx:xx:xx] \
        !!}\r\n");
}

OAL_STATIC uint32_t ftm_debug_parase_iftmr_cmd1(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    /* 解析channel */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get iftmr mode error, return.}");
        return ret;
    }

    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.channel_num = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析ftms_per_burst */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get uc_ftms_per_burst error, return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.ftms_per_burst = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析burst */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get burst error, return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.burst_num = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;
    return OAL_SUCC;
}
OAL_STATIC uint32_t ftm_debug_parase_iftmr_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = { 0 };
    uint32_t off_set = 0;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    /* 命令格式:send_initial_ftm_request channel[] ftms_per_burst[n] burst_num[n]---代表(2^n)个burst
                                    en_lci_civic_request[0|1] asap[0|1] bssid[xx:xx:xx:xx:xx:xx]
                                    format_bw[9~13] */
    ret = ftm_debug_parase_iftmr_cmd1(pc_param, pst_debug_info, pul_offset);
    if (ret != OAL_SUCC) {
        return ret;
    }

    pc_param += *pul_offset;
    /* 解析LCI and Location Civic Measurement request使能标志 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get en_lci_civic_request error, return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.lci_request = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;

    /* 解析asap,as soon as possible使能标志 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get asap error, return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.is_asap_on = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;

    /* 获取BSSID */
    if (wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &off_set) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::No bssid, set the associated bssid.}");
        memset_s(pst_debug_info->st_send_iftmr_bit1.mac_addr, sizeof(pst_debug_info->st_send_iftmr_bit1.mac_addr),
                 0, sizeof(pst_debug_info->st_send_iftmr_bit1.mac_addr));
        return OAL_SUCC;
    }
    *pul_offset += off_set;
    oal_set_mac_addr(pst_debug_info->st_send_iftmr_bit1.mac_addr, auc_mac_addr);
    pc_param += off_set;

    oam_warning_log3(0, OAM_SF_ANY, "{ftm_debug_parase_iftmr_cmd::bssid=%02X:XX:XX:XX:%02X:%02X}",
                     auc_mac_addr[MAC_ADDR_0], auc_mac_addr[MAC_ADDR_4], auc_mac_addr[MAC_ADDR_5]);

    /* 解析format bw */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get format bw error, return.}");
        return OAL_SUCC;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.format_bw = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;

    return OAL_SUCC;
}

/*
 * 功能描述  : 解析ftm命令参数
 * 1.日    期  : 2017年10月10日
 *   作    者  : wifi
 *   修改内容  : 修改
 */
OAL_STATIC uint32_t ftm_debug_parase_ftm_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;

    *pul_offset = 0;
    /* 解析mac */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_cmd::ger mac error.}");
        return OAL_FAIL;
    }
    *pul_offset += off_set;
    oal_strtoaddr(ac_value, sizeof(ac_value), pst_debug_info->st_send_ftm_bit4.auc_mac, WLAN_MAC_ADDR_LEN);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}

/*
 * 功能描述  : 解析ftm timeout命令参数
 * 1.日    期  : 2019年09月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t ftm_debug_parase_ftm_timeout_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;

    *pul_offset = 0;
    /* 解析ftm bursts timeout */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_time_cmd::get correct time1 error,return.}");
        return ret;
    }

    *pul_offset += off_set;
    pst_debug_info->st_ftm_timeout_bit14.initiator_tsf_fix_offset = (uint32_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析ftms timeout */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_time_cmd::get correct time2 error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_ftm_timeout_bit14.responder_ptsf_offset = (uint32_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置校准值
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t ftm_debug_set_cali(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    *pul_offset = 0;

    /* 解析t1 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_time_cmd::get correct time1 error, return.}");
        return ret;
    }

    *pul_offset += off_set;
    pst_debug_info->ftm_cali = (uint32_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}

OAL_STATIC uint32_t ftm_debug_parase_range_req_all_ap_cnt(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t uc_ap_cnt;

    for (uc_ap_cnt = 0; uc_ap_cnt < pst_debug_info->st_send_range_req_bit7.uc_minimum_ap_count; uc_ap_cnt++) {
        /* 解析bssid */
        ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_all_ap_cnt::bssid error!.}");
            return ret;
        }
        *pul_offset += off_set;
        oal_strtoaddr(ac_value, sizeof(ac_value),
            pst_debug_info->st_send_range_req_bit7.aauc_bssid[uc_ap_cnt], WLAN_MAC_ADDR_LEN);
        pc_param += off_set;
        off_set = 0;

        /* 解析channel */
        ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_all_ap_cnt::get channel error,return.}");
            return ret;
        }
        *pul_offset += off_set;
        pst_debug_info->st_send_range_req_bit7.auc_channel[uc_ap_cnt] = (uint8_t)oal_atoi(ac_value);
        pc_param += off_set;
        off_set = 0;
    }
    return OAL_SUCC;
}

/*
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t ftm_debug_parase_range_req_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;

    *pul_offset = 0;
    /* 解析mac */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *pul_offset += off_set;
    oal_strtoaddr(ac_value, sizeof(ac_value), pst_debug_info->st_send_range_req_bit7.auc_mac, WLAN_MAC_ADDR_LEN);
    pc_param += off_set;
    off_set = 0;

    /* 解析num_rpt */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::get num_rpt error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_range_req_bit7.us_num_rpt = (uint16_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析ap_cnt */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::get ap_cnt error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_range_req_bit7.uc_minimum_ap_count = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析每个AP */
    return ftm_debug_parase_range_req_all_ap_cnt(pc_param, pst_debug_info, pul_offset);
}

/*
 * 1.日    期  : 2017年10月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t ftm_debug_parase_m2s_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;

    *pul_offset = 0;
    /* 解析ftm定位通路:0或1 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_m2s_cmd::get tx_chain error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->ftm_chain_selection = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}

/*
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t ftm_debug_parase_neighbor_report_req_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    uint32_t off_set = 0;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    *pul_offset = 0;

    /* 解析bssid */
    ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM,
            "{ftm_debug_parase_neighbor_report_req_cmd::No bssid,set the associated bssid.}");
        *pul_offset += off_set;
        memset_s(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid,
                 sizeof(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid),
                 0, sizeof(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid));
        pc_param += off_set;
        off_set = 0;
        return ret;
    }
    *pul_offset += off_set;
    oal_set_mac_addr(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid, auc_mac_addr);
    pc_param += off_set;
    off_set = 0;

    oam_warning_log3(0, OAM_SF_ANY,
        "{ftm_debug_parase_neighbor_report_req_cmd::send neighbor request frame to ap[%02X:XX:XX:XX:%02X:%02X]}",
        auc_mac_addr[MAC_ADDR_0], auc_mac_addr[MAC_ADDR_4], auc_mac_addr[MAC_ADDR_5]);

    return OAL_SUCC;
}

/*
 * 1.日    期  : 2019年08月16日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t ftm_debug_parase_gas_init_req_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    /* 命令格式:send_gas_init_req lci_enable[0|1] bssid[xx:xx:xx:xx:xx:xx] */
    *pul_offset = 0;

    /* 解析lci_enable */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM,
            "{ftm_debug_parase_gas_init_req_cmd::get lci_enable error, use default value [true].}");
        ac_value[0] = OAL_TRUE;
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_gas_init_req_bit16.en_lci_enable = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析bssid */
    ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_gas_init_req_cmd::No bssid, set the associated bssid.}");
        *pul_offset += off_set;
        memset_s(pst_debug_info->st_gas_init_req_bit16.auc_bssid,
                 sizeof(pst_debug_info->st_gas_init_req_bit16.auc_bssid),
                 0, sizeof(pst_debug_info->st_gas_init_req_bit16.auc_bssid));
        pc_param += off_set;
        off_set = 0;
        return ret;
    }
    *pul_offset += off_set;
    oal_set_mac_addr(pst_debug_info->st_gas_init_req_bit16.auc_bssid, auc_mac_addr);
    pc_param += off_set;
    off_set = 0;

    oam_warning_log3(0, OAM_SF_ANY,
        "{ftm_debug_parase_gas_init_req_cmd::send gas initial request frame to ap[%02X:XX:XX:XX:%02X:%02X]}",
        auc_mac_addr[MAC_ADDR_0], auc_mac_addr[MAC_ADDR_4], auc_mac_addr[MAC_ADDR_5]);
    return OAL_SUCC;
}

/*
 * 功能描述  : 是否是fmt普通命令
 * 1.日    期  : 2019年8月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint8_t wal_hipriv_ftm_is_common_cmd(int8_t ac_name[], uint8_t *puc_cmd_index)
{
    uint8_t uc_cmd_index;
    uint32_t ratio = sizeof(g_ast_ftm_common_cmd) / sizeof(wal_ftm_cmd_entry_stru);

    for (uc_cmd_index = 0; uc_cmd_index < ratio; uc_cmd_index++) {
        if (0 == oal_strcmp(g_ast_ftm_common_cmd[uc_cmd_index].pc_cmd_name, ac_name)) {
            break;
        }
    }

    *puc_cmd_index = uc_cmd_index;
    if (uc_cmd_index == ratio) {
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}

/*
 * 功能描述  : 设置普通命令的参数
 * 1.日    期  : 2019年8月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_ftm_set_common_cmd(int8_t *pc_param,
    uint8_t uc_cmd_index, mac_ftm_debug_switch_stru *pst_ftm_debug, uint32_t *pul_offset)
{
    uint32_t ret;
    uint32_t off_set = 0;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    *pul_offset = 0;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_ftm_set_common_cmd:: ftm common cmd [%d].}", uc_cmd_index);

    /* 取命令配置值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        ftm_debug_cmd_format_info();
        return ret;
    }

    /* 检查命令的配置值 */
    if ((g_ast_ftm_common_cmd[uc_cmd_index].uc_is_check_para) &&
        oal_value_not_in_valid_range(ac_value[0], '0', '9')) {
        ftm_debug_cmd_format_info();
        return OAL_FAIL;
    }

    *pul_offset += off_set;
    off_set = 0;

    /* 填写命令结构体 */
    switch (g_ast_ftm_common_cmd[uc_cmd_index].bit) {
        case BIT2:
            pst_ftm_debug->en_enable_bit2 = ((uint8_t)oal_atoi(ac_value));
            pst_ftm_debug->cmd_bit_map |= BIT2;
            break;
        case BIT3:
            pst_ftm_debug->en_cali_bit3 = ((uint8_t)oal_atoi(ac_value));
            pst_ftm_debug->cmd_bit_map |= BIT3;
            break;
        case BIT12:
            pst_ftm_debug->en_multipath_bit12 = ((uint8_t)oal_atoi(ac_value));
            pst_ftm_debug->cmd_bit_map |= BIT12;
            break;
        default:
            oam_warning_log1(0, OAM_SF_FTM,
                "{wal_hipriv_ftm_set_common_cmd::invalid cmd bit[0x%x]!}", g_ast_ftm_common_cmd[uc_cmd_index].bit);
            break;
    }

    return OAL_SUCC;
}

static uint32_t wal_get_ftm_debug_send_iftmr_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_iftmr_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    ftm_debug->cmd_bit_map |= BIT1;
    return ret;
}

static uint32_t wal_get_ftm_debug_ftm_addr_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_ftm_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT4;
    return ret;
}

static uint32_t wal_get_ftm_debug_timeout_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_ftm_timeout_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT14;
    return ret;
}

static uint32_t wal_get_ftm_debug_set_cali_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_set_cali(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT13;
    return ret;
}

static uint32_t wal_get_ftm_debug_range_req_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_range_req_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT7;
    return ret;
}

static uint32_t wal_get_ftm_debug_get_cali_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    ftm_debug->cmd_bit_map |= BIT9;
    return OAL_SUCC;
}

static uint32_t wal_get_ftm_debug_m2s_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_m2s_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT11;
    return ret;
}

static uint32_t wal_get_ftm_debug_neighbor_report_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_neighbor_report_req_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT15;
    return ret;
}

static uint32_t wal_get_ftm_debug_gas_init_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_gas_init_req_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT16;
    return ret;
}

typedef struct {
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t (*wal_get_ftm_debug_case_info)(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param);
} wal_get_ftm_debug_attr_info_ops;

static uint32_t wal_get_ftm_debug_attr_info_byname(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param,
                                                   int8_t *name)
{
    uint8_t idx;
    uint32_t ret = 0;
    const wal_get_ftm_debug_attr_info_ops wal_get_ftm_debug_attr_table[] = {
        { "send_initial_ftm_request", wal_get_ftm_debug_send_iftmr_info },
        { "send_ftm",                 wal_get_ftm_debug_ftm_addr_info },
        { "set_ftm_timeout",          wal_get_ftm_debug_timeout_info },
        { "set_cali",                 wal_get_ftm_debug_set_cali_info },
        { "send_range_req",           wal_get_ftm_debug_range_req_info },
        { "get_cali",                 wal_get_ftm_debug_get_cali_info },
        { "set_ftm_m2s",              wal_get_ftm_debug_m2s_info },
        { "send_nbr_rpt_req",         wal_get_ftm_debug_neighbor_report_info },
        { "send_gas_init_req",        wal_get_ftm_debug_gas_init_info },
    };

    for (idx = 0; idx < (sizeof(wal_get_ftm_debug_attr_table) / sizeof(wal_get_ftm_debug_attr_info_ops)); ++idx) {
        if (oal_strcmp(wal_get_ftm_debug_attr_table[idx].name, name) == 0) {
            ret = wal_get_ftm_debug_attr_table[idx].wal_get_ftm_debug_case_info(ftm_debug, param);
            break;
        }
    }
    if (idx == sizeof(wal_get_ftm_debug_attr_table) / sizeof(wal_get_ftm_debug_attr_info_ops)) {
        return OAL_FAIL;
    } else {
        return ret;
    }
    return ret;
}

OAL_STATIC uint32_t wal_get_ftm_param(mac_ftm_debug_switch_stru *ftm_debug, int8_t *param)
{
    uint32_t off_set = 0;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t ret = 0;
    uint8_t cmd_index = 0;

    do {
        /* 获取命令关键字 */
        ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if ((ret != OAL_SUCC) && (off_set != 0)) {
            return ret;
        }
        param += off_set;

        if (off_set == 0) {
            break;
        }

        /* 命令分类 */
        if (wal_hipriv_ftm_is_common_cmd(name, &cmd_index)) {
            ret = wal_hipriv_ftm_set_common_cmd(param, cmd_index, ftm_debug, &off_set);
            param += off_set;
            off_set = 0;
            if (ret != OAL_SUCC) {
                return ret;
            }
        } else {
            if (wal_get_ftm_debug_attr_info_byname(ftm_debug, &param, name) != OAL_SUCC) {
                return OAL_FAIL;
            }
        }
    } while (*param != '\0');

    return OAL_SUCC;
}
/*
 * 1.日    期  : 2017年2月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_ftm(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    mac_ftm_debug_switch_stru *ftm_debug;
    int32_t l_ret;
    uint32_t ret;

    ftm_debug = (mac_ftm_debug_switch_stru *)oal_memalloc(sizeof(mac_ftm_debug_switch_stru));
    if (ftm_debug == NULL) {
        return OAL_FAIL;
    }
    memset_s(ftm_debug, sizeof(mac_ftm_debug_switch_stru), 0, sizeof(mac_ftm_debug_switch_stru));

    ret = wal_get_ftm_param(ftm_debug, param);
    if (ret != OAL_SUCC) {
        oal_free(ftm_debug);
        ftm_debug_cmd_format_info();
        return ret;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_ftm::cmd_bit_map: 0x%08x.}", ftm_debug->cmd_bit_map);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_FTM_DBG, sizeof(mac_ftm_debug_switch_stru));

    /* 设置配置命令参数 */
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
                 (const void *)ftm_debug, sizeof(mac_ftm_debug_switch_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_ftm::memcpy fail!}");
        oal_free(ftm_debug);
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_ftm_debug_switch_stru),
                               (uint8_t *)&write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FTM, "{wal_hipriv_ftm::return err code[%d]!}", l_ret);
        oal_free(ftm_debug);
        return (uint32_t)l_ret;
    }
    oal_free(ftm_debug);
    return OAL_SUCC;
}
uint32_t _wal_hipriv_ftm(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        return wal_hipriv_ftm(pst_net_dev, pc_param);
    }

    return OAL_SUCC;
}
#endif

#endif
