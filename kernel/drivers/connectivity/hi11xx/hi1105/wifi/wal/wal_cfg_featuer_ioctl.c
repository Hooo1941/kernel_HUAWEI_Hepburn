/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :特性相关命令
 * 创建日期 : 2022年11月19日
 */

#include "oal_types.h"
#include "wal_config.h"
#include "hmac_vap.h"
#include "mac_mib.h"
#include "hmac_scan.h"
#include "oal_util.h"
#include "plat_pm_wlan.h"
#include "hmac_resource.h"
#include "wal_linux_ioctl.h"
#include "plat_firmware.h"
#ifdef _PRE_WLAN_FEATURE_PMF
#include "hmac_11w.h"
#endif
#include "hmac_wapi.h"
#include "hmac_sae.h"
#ifdef _PRE_WLAN_FEATURE_TWT
#include "hmac_twt.h"
#endif
#include "oam_event_wifi.h"
#ifdef _PRE_WLAN_FEATURE_VSP
#include "hmac_vsp_if.h"
#endif
#include "wal_cfg_ioctl.h"
#include "hmac_mintp_test.h"
#ifdef _PRE_WLAN_NARROW_BAND
#ifdef _PRE_WLAN_EXPORT
#include "wal_cfg_dscr_ioctl.h"
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_FEATURE_CFG_IOCTL_C

#ifdef WIFI_DEBUG_ENABLE

uint32_t wal_set_bgscan_type(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t bgscan_state; // 枚举定义hmac_scan_state_enum
    bgscan_state = (uint8_t)params[0];
    return hmac_bgscan_enable(mac_vap, sizeof(bgscan_state), &bgscan_state);
}

uint32_t wal_set_random_mac_addr_scan(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_bool_enum_uint8 rand_mac_addr_scan_switch;
    rand_mac_addr_scan_switch = (oal_bool_enum_uint8)params[0];
    return hmac_config_set_random_mac_addr_scan(mac_vap, sizeof(rand_mac_addr_scan_switch),
                                                &rand_mac_addr_scan_switch);
}

#ifdef _PRE_WLAN_FEATURE_WMMAC
/*
 * 功能描述  : 发送ADDTS REQ配置命令
 * 1.日    期  : 2020年7月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_addts_req(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t idx = 0;
    hmac_user_stru *hmac_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_wmm_tspec_stru addts_args;
    uint8_t ac;
    /*
     * 设置发送ADDTS REQ配置命令:
     * hipriv "vap0 addts_req tid direction psb up nominal_msdu_size maximum_data_rate
     * minimum_data_rate mean_data_rate peak_data_rate minimum_phy_rate surplus_bandwidth_allowance"
     */
    /***********************************************************************************************
    TSPEC字段:
          --------------------------------------------------------------------------------------
          |TS Info|Nominal MSDU Size|Max MSDU Size|Min Serv Itvl|Max Serv Itvl|
          ---------------------------------------------------------------------------------------
    Octets:  | 3     |  2              |   2         |4            |4            |
          ---------------------------------------------------------------------------------------
          | Inactivity Itvl | Suspension Itvl | Serv Start Time |Min Data Rate | Mean Data Rate |
          ---------------------------------------------------------------------------------------
    Octets:  |4                | 4               | 4               |4             |  4             |
          ---------------------------------------------------------------------------------------
          |Peak Data Rate|Burst Size|Delay Bound|Min PHY Rate|Surplus BW Allowance  |Medium Time|
          ---------------------------------------------------------------------------------------
    Octets:  |4             |4         | 4         | 4          |  2                   |2          |
          ---------------------------------------------------------------------------------------

    TS info字段:
          ---------------------------------------------------------------------------------------
          |Reserved |TSID |Direction |1 |0 |Reserved |PSB |UP |Reserved |Reserved |Reserved |
          ---------------------------------------------------------------------------------------
    Bits:  |1        |4    |2         |  2  |1        |1   |3  |2        |1        |7        |
          ----------------------------------------------------------------------------------------
    ***********************************************************************************************/
    if (g_en_wmmac_switch == OAL_FALSE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{wal_set_addts_req::wmmac switch is false[%d].}", g_en_wmmac_switch);
        return OAL_SUCC;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_WMMAC, "{wal_set_addts_req::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户对应的索引 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);
    if (hmac_user == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_WMMAC, "{wal_set_addts_req::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 0.获取tid，取值范围0~7 */
    addts_args.ts_info.bit_tsid = (uint16_t)params[idx++];

    /* 1.获取direction 00:uplink 01:downlink 10:reserved 11:Bi-directional */
    addts_args.ts_info.bit_direction = (uint16_t)params[idx++];
    if (addts_args.ts_info.bit_direction == MAC_WMMAC_DIRECTION_RESERVED) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_set_addts_req::the direction is not correct! direction is[%d]!}\r\n",
            addts_args.ts_info.bit_direction);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* 2.获取PSB，1表示U-APSD，0表示legacy */
    addts_args.ts_info.bit_apsd = (uint16_t)params[idx++];

    /* 3.获取UP */
    addts_args.ts_info.bit_user_prio = (uint16_t)params[idx++];

    /* 4.获取Nominal MSDU Size ,第一位为1 */
    /*
        ------------
        |fixed|size|
        ------------
    bits:  |1    |15  |
        ------------
    */
    addts_args.us_norminal_msdu_size = (uint16_t)params[idx++];

    /* 5.获取maximum MSDU size */
    addts_args.us_max_msdu_size = (uint16_t)params[idx++];

    /* 6.获取minimum data rate */
    addts_args.min_data_rate = (uint32_t)params[idx++];

    /* 7.获取mean data rate */
    addts_args.mean_data_rate = (uint32_t)params[idx++];

    /* 8.获取peak data rate */
    addts_args.peak_data_rate = (uint32_t)params[idx++];

    /* 9.获取minimum PHY Rate */
    addts_args.min_phy_rate = (uint32_t)params[idx++];

    /* 10获取surplus bandwidth allowance */
    addts_args.us_surplus_bw = (uint16_t)params[idx++];

    /* 判断对应AC的ACM位，只有该AC的ACM为1时，才允许建立TS。 */
    ac = WLAN_WME_TID_TO_AC(addts_args.ts_info.bit_user_prio);
    if (OAL_FALSE == mac_mib_get_QAPEDCATableMandatory(&(hmac_vap->st_vap_base_info), ac)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_WMMAC, "{wal_set_addts_req::user prio[%d] is false.}", ac);
        return OAL_SUCC;
    }
    /* 发送ADDTS REQ，建立TS */
    return hmac_mgmt_tx_addts_req(hmac_vap, hmac_user, &addts_args);
}

/*
 * 功能描述  : 删除ts的配置命令
 * 1.日    期  : 2020年7月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_delts(mac_vap_stru *mac_vap, uint32_t *params)
{
    hmac_user_stru *hmac_user = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    mac_wmm_tspec_stru delts_args;

    delts_args.ts_info.bit_tsid = (uint8_t)params[0];

    if (g_en_wmmac_switch == OAL_FALSE) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_WMMAC,
            "{wal_set_delts::wmmac switch is false[%d].}", g_en_wmmac_switch);
        return OAL_SUCC;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_WMMAC, "{wal_set_delts::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户对应的索引 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user(mac_vap->us_assoc_vap_id);
    if (hmac_user == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_WMMAC, "{wal_set_delts::pst_hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return hmac_mgmt_tx_delts(hmac_vap, hmac_user, &delts_args);
}

/*
 * 函 数 名  : wal_config_reassoc_req
 * 功能描述  : 设置发送重关联请求配置命令
 * 1.日    期  : 2016年9月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_config_reassoc_req(mac_vap_stru *mac_vap, uint32_t *param)
{
    return hmac_config_reassoc_req(mac_vap);
}
#endif

uint32_t wal_set_2040_coext_support(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t csp;
    csp = (uint8_t)params[0];
    return hmac_config_set_2040_coext_support(mac_vap, sizeof(csp), &csp);
}


#ifdef _PRE_WLAN_FEATURE_PHY_EVENT_INFO
uint32_t wal_hipriv_phy_event_rpt(mac_vap_stru *mac_vap, uint32_t *params)
{
    mac_cfg_phy_event_rpt_stru phy_event_cfg;
    phy_event_cfg.event_rpt_en = params[0];
    phy_event_cfg.wp_mem_num = params[1];
    phy_event_cfg.wp_event0_type_sel = params[BIT_OFFSET_2];
    phy_event_cfg.wp_event1_type_sel = params[BIT_OFFSET_3];
    oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_CFG,
        "wal_hipriv_phy_event_rpt en[%d] mem num[%d] event0 sel[%x] event1 sel[%x]", phy_event_cfg.event_rpt_en,
        phy_event_cfg.wp_mem_num, phy_event_cfg.wp_event0_type_sel, phy_event_cfg.wp_event1_type_sel);
    return OAL_SUCC;
}
#endif

/*
 * 功能描述  : 设置voe开关
 * 1.日    期  : 2020年7月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_voe_enable(mac_vap_stru *mac_vap, uint32_t *params)
{
    hmac_vap_stru *hmac_vap;
    oal_bool_enum_uint8 en_read_flag;
    uint16_t voe_switch = (uint16_t)params[0];

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_voe_enable::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    en_read_flag = (voe_switch & BIT7) ? OAL_TRUE : OAL_FALSE;
    if (en_read_flag == OAL_TRUE) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{wal_set_voe_enable::custom_11k=[%d],custom_11v=[%d],custom_11r=[%d].}",
                         g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11k,
                         g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11v,
                         g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r);
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R)
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{wal_set_voe_enable::11k=[%d],11v=[%d],11r=[%d].}",
                         hmac_vap->bit_11k_enable,
                         hmac_vap->bit_11v_enable,
                         hmac_vap->bit_11r_enable);
#endif
        return OAL_SUCC;
    } else {
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11R)
        hmac_vap->bit_11r_enable = (voe_switch & BIT0) ? OAL_TRUE : OAL_FALSE;
        hmac_vap->bit_11v_enable = (voe_switch & BIT1) ? OAL_TRUE : OAL_FALSE;
        hmac_vap->bit_11k_enable = (voe_switch & BIT2) ? OAL_TRUE : OAL_FALSE;
        hmac_vap->bit_11k_auth_flag = (voe_switch & BIT3) ? OAL_TRUE : OAL_FALSE;
        hmac_vap->bit_voe_11r_auth = (voe_switch & BIT4) ? OAL_TRUE : OAL_FALSE;
        hmac_vap->bit_11k_auth_oper_class = ((voe_switch >> BIT_OFFSET_5) & 0x3);
        hmac_vap->bit_11r_over_ds = ((voe_switch >> BIT_OFFSET_8) & BIT0) ? OAL_TRUE : OAL_FALSE;

        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_set_voe_enable::uc_param0 = [0x%X],uc_param1 = [0x%X].}",
                         (voe_switch & 0xFF), ((voe_switch >> BIT_OFFSET_8) & 0xFF));
#endif
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 移除或者恢复某个上层配置的IE，目前只支持操作一个IE，多次操作只会保留一个IE信息
 * 1.日    期  : 2020年7月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_remove_app_ie(mac_vap_stru *mac_vap, uint32_t *params)
{
    hmac_vap_stru *hmac_vap = NULL;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_remove_app_ie::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap->st_remove_ie.uc_type = (uint8_t)params[0];
    hmac_vap->st_remove_ie.uc_eid = (uint8_t)params[1];

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置是否上报beacon帧的开关
 * 1.日    期  : 2020年7月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_ota_beacon_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t value = (uint8_t)params[0]; // oam_sdt_print_beacon_rxdscr_type_enum
    /* 此处为兼容mp13 mp15, 06不使用 */
    oam_ota_set_switch(OAM_OTA_SWITCH_BEACON, value);

    return OAL_SUCC;
}
/*
 * 功能描述  : 检查指定内存池泄漏内存块
 * 1.日    期  : 2020年7月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_mem_leak(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_mem_pool_id_enum_uint8 pool_id;
    /* 获取内存池ID */
    pool_id = (oal_mem_pool_id_enum_uint8)params[0];

    /* 检查内存池泄漏内存块 */
    oal_mem_leak(pool_id);
    return OAL_SUCC;
}
/*
 * 功能描述  : 显示设备支持的信道列表
 * 1.日    期  : 2020年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_list_channel(mac_vap_stru *mac_vap, uint32_t *params)
{
    hmac_config_list_channel(mac_vap);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PMF
/*
 * 功能描述  : chip test 强制使能pmf
 * 1.日    期  : 2020年8月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_enable_pmf(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t pmf_status = (uint8_t)params[0];
    /* 设置一下hmac的芯片验证开关 */
    hmac_enable_pmf(mac_vap, &pmf_status);
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
/*
 * 功能描述  : 测试打桩触发dfr功能
 * 1.日    期  : 2020年8月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_test_dfr_start(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint32_t cfg_rst;
    mac_device_stru *mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_test_dfr_start::mac_device is null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((!g_st_dfr_info.bit_device_reset_enable) || g_st_dfr_info.bit_device_reset_process_flag) {
        oam_warning_log2(0, OAM_SF_ANY,
            "{wal_test_dfr_start::now DFR disabled or in DFR process, enable=%d, reset_flag=%d}",
            g_st_dfr_info.bit_device_reset_enable, g_st_dfr_info.bit_device_reset_process_flag);
        return OAL_ERR_CODE_RESET_INPROGRESS;
    }

    g_st_dfr_info.bit_device_reset_enable = OAL_TRUE;
    g_st_dfr_info.bit_device_reset_process_flag = OAL_FALSE;
    g_st_dfr_info.netdev_num = 0;
    memset_s((uint8_t *)(g_st_dfr_info.past_netdev),
             sizeof(g_st_dfr_info.past_netdev[0]) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT, 0,
             sizeof(g_st_dfr_info.past_netdev[0]) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);

    cfg_rst = wal_dfr_excp_rx(mac_device->uc_device_id, 0);
    if (oal_unlikely(cfg_rst != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_test_dfr_start::wal_send_cfg_event return err_code [%d]!}\r\n", cfg_rst);
        return cfg_rst;
    }

    return OAL_SUCC;
}
#endif
/*
 * 功能描述  : 设置带宽为固定模式
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_bw_fixed(mac_vap_stru *mac_vap, uint32_t *params)
{
    /* 设备在up状态不允许配置，必须先down */
    if (mac_vap->en_vap_state != MAC_VAP_STATE_INIT) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_set_bw_fixed::device is busy, please down it first %d!}\r\n", mac_vap->en_vap_state);
        return OAL_FAIL;
    }
    mac_vap->bit_bw_fixed = (uint8_t)params[0];
    oam_warning_log1(0, OAM_SF_CFG, "{wal_set_bw_fixed:bw_fixed = [%d].}", mac_vap->bit_bw_fixed);
    return OAL_SUCC;
}
/*
 * 功能描述  : 发送20/40共存管理帧
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_send_2040_coext(mac_vap_stru *mac_vap, uint32_t *params)
{
    mac_cfg_set_2040_coexist_stru param_2040_coexist;
    param_2040_coexist.coext_info = params[0];
    param_2040_coexist.channel_report = params[1];

    hmac_config_send_2040_coext(mac_vap, sizeof(mac_cfg_set_2040_coexist_stru), (uint8_t *)&param_2040_coexist);
    return OAL_SUCC;
}
#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/*
 * 功能描述  : 11v配置接口 用于打开或者关闭11v特性的bss transition mgmt功能
               须先打开wireless management特性，子特性开关才能生效
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_11v_cfg_bsst(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_bool_enum_uint8 switch_11v_cfg = (oal_bool_enum_uint8)params[0];

    /* 调用接口配置11v特性开关 */
    mac_mib_set_MgmtOptionBSSTransitionActivated(mac_vap, switch_11v_cfg);

    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_set_11v_cfg_bsst:: Set BSST_Actived=[%d].}",
                     mac_mib_get_MgmtOptionBSSTransitionActivated(mac_vap));
    return OAL_SUCC;
}
#endif

/*
 * 函 数 名  : wal_hipriv_set_freq
 * 功能描述  : 设置频点/信道
 * 1.日    期  : 2014年12月12日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_freq(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_freq(net_dev, pc_param);
}
/*
 * 功能描述  : 设置模式: 包括协议、频段、带宽
 * 1.日    期  : 2014年12月12日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_mode(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_ioctl_set_mode(net_dev, param);
}
const int8_t *g_band_tbl[] = {
    "2g",
    "5g",
    "6g",
    NULL,
};

/*
 * 函 数 名  : wal_hipriv_set_bw
 * 功能描述  : 设置频段
 * 1.日    期  : 2020年9月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_band(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    mac_cfg_tx_comp_stru *set_band_param = NULL;
    int8_t arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_stru *rsp_msg = NULL;
    uint8_t map_index = 0;
    uint32_t ret;

    /* 解析并设置配置命令参数 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_BAND, sizeof(mac_cfg_tx_comp_stru));
    set_band_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);
    ret = wal_get_cmd_one_arg(pc_param, arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_band::get cmd one arg err[%d]!}", ret);
        return ret;
    }

    /* 寻找匹配的命令 */
    while (g_band_tbl[map_index] != NULL) {
        if (oal_strcmp(g_band_tbl[map_index], arg) == 0) {
            break;
        }
        ++map_index;
    }

    /* 没有找到对应的命令，则报错 */
    if (g_band_tbl[map_index] == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_band::invalid alg_cfg command!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    set_band_param->uc_param = map_index;
    ret = (uint32_t)wal_send_cfg_event(net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                                       (uint8_t *)&write_msg,
                                       OAL_TRUE,
                                       &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_band::err [%d]!}", ret);
        return ret;
    }
    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_band fail, err code[%u]!}", ret);
        return ret;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_set_bw
 * 功能描述  : 设置带宽
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_bw(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_tx_comp_stru *pst_set_bw_param = NULL;
    wlan_bandwith_cap_enum_uint8 en_bw_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_BW, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_bw_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    /* 获取带宽值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 解析要设置为多大的值 */
    for (en_bw_index = 0; en_bw_index < WLAN_BANDWITH_CAP_BUTT; en_bw_index++) {
        if (!oal_strcmp(g_pauc_bw_tbl[en_bw_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_bw_index >= WLAN_BANDWITH_CAP_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_bw::not support this bandwidth!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_bw_param->uc_param = (uint8_t)(en_bw_index);

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_bw::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_bw fail, err code[%u]!}", result);
        return result;
    }
    return OAL_SUCC;
}

#define MAX_HIPRIV_IP_FILTER_BTABLE_SIZE 129

OAL_STATIC uint32_t wal_ipriv_ip_filter_items(int8_t *pc_param, int8_t *ac_cmd_param, uint32_t *off_set)
{
    int32_t items_cnt;
    int32_t items_idx;
    uint32_t ret;
    wal_hw_wifi_filter_item *items = NULL;

    items = (wal_hw_wifi_filter_item *)oal_memalloc(sizeof(wal_hw_wifi_filter_item) * MAX_HIPRIV_IP_FILTER_BTABLE_SIZE);
    if (items == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter:: items alloc mem return null ptr.}");
        return -OAL_EFAIL;
    }

    memset_s(items, sizeof(wal_hw_wifi_filter_item) * MAX_HIPRIV_IP_FILTER_BTABLE_SIZE,
             0, sizeof(wal_hw_wifi_filter_item) * MAX_HIPRIV_IP_FILTER_BTABLE_SIZE);

    /* 更新黑名单 */
    /* 获取名单条目数 */
    items_cnt = oal_min(MAX_HIPRIV_IP_FILTER_BTABLE_SIZE, oal_atoi(ac_cmd_param));
    /* 获取名单条目 */
    for (items_idx = 0; items_idx < items_cnt; items_idx++) {
        /* 获取protocol X */
        pc_param += *off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get item_params err:%d!}", ret);
            oal_free(items);
            return ret;
        }
        items[items_idx].protocol = (uint8_t)oal_atoi(ac_cmd_param);

        /* 获取portX */
        pc_param += *off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get item_params  err: %d!}", ret);
            oal_free(items);
            return ret;
        }
        items[items_idx].port = (uint16_t)oal_atoi(ac_cmd_param);
    }

    ret = (uint32_t)wal_add_ip_filter_items(items, items_cnt);
    oal_free(items);
    return ret;
}

/*
 * 函 数 名  : wal_hipriv_set_ip_filter
 * 功能描述  : 功能调试接口
 * 1.日    期  : 2017年4月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_ip_filter(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t ret;
    uint32_t off_set = 0;
    int8_t ac_cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    int8_t ac_cmd_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* 其取出子命令 */
    ret = wal_get_cmd_one_arg(pc_param, ac_cmd, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::wal_get_cmd_one_arg err_code:%d}", ret);
        return ret;
    }

    if (0 == oal_strncmp(ac_cmd, CMD_CLEAR_RX_FILTERS, OAL_STRLEN(CMD_CLEAR_RX_FILTERS))) {
        /* 清理表单 */
        return (uint32_t)wal_clear_ip_filter();
    }

    pc_param += off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::get cmd_param err %d!}", ret);
        return ret;
    }

    if (0 == oal_strncmp(ac_cmd, CMD_SET_RX_FILTER_ENABLE, OAL_STRLEN(CMD_SET_RX_FILTER_ENABLE))) {
        /* 使能/关闭功能 */
        return (uint32_t)wal_set_ip_filter_enable(oal_atoi(ac_cmd_param));
    } else if (0 == oal_strncmp(ac_cmd, CMD_ADD_RX_FILTER_ITEMS, OAL_STRLEN(CMD_ADD_RX_FILTER_ITEMS))) {
        return wal_ipriv_ip_filter_items(pc_param, ac_cmd_param, &off_set);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ip_filter::cmd_one_arg no support!}");
        return OAL_FAIL;
    }
}

#ifdef _PRE_WLAN_FEATURE_11K
/*
 * 功能描述  : 发送neighbor req配置命令sh hipriv.sh "wlan0 send_neighbor_req ssid"
 * 1.日    期  : 2016年6月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_send_neighbor_req(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t offset = 0;
    uint32_t result;
    int32_t ret;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_ssid_param_stru *pst_ssid = NULL;
    uint8_t uc_str_len;

    uc_str_len = os_str_len(pc_param);
    uc_str_len = (uc_str_len > 1) ? uc_str_len - 1 : uc_str_len;

    /* 获取SSID字符串 */
    if (uc_str_len != 0) {
        result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (result != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_neighbor_req::get cmd one arg err[%d]!}", result);
            return result;
        }
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SEND_NEIGHBOR_REQ, sizeof(mac_cfg_ssid_param_stru));
    pst_ssid = (mac_cfg_ssid_param_stru *)write_msg.auc_value;
    pst_ssid->uc_ssid_len = uc_str_len;
    ret = memcpy_s(pst_ssid->ac_ssid, WLAN_SSID_MAX_LEN, ac_arg, pst_ssid->uc_ssid_len);
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_send_neighbor_req::memcpy fail!");
        return OAL_FAIL;
    }

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ssid_param_stru), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_neighbor_req::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_APF
#define APF_FILTER_LIST_PROGRAM_LEN 200
/*
 * 函 数 名  : wal_hipriv_apf_filter_list
 * 功能描述  : hipriv命令 显示apf filter到sdt
 * 1.日    期  : 2017年11月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_apf_filter_list(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t ret;
    mac_apf_filter_cmd_stru st_apf_filter_cmd;
    uint8_t program[APF_FILTER_LIST_PROGRAM_LEN] = { 0 };

    memset_s(&st_apf_filter_cmd, sizeof(mac_apf_filter_cmd_stru), 0, sizeof(mac_apf_filter_cmd_stru));
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_hipriv_apf_filter_list::wal_get_cmd_one_arg return err_code %d!}\r\n", ret);
        return ret;
    }
    if ((0 != oal_strcmp("0", ac_name)) && (0 != oal_strcmp("1", ac_name))) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_apf_filter_list::invalid parameter.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    st_apf_filter_cmd.en_cmd_type = (uint8_t)oal_atoi(ac_name);
    if (st_apf_filter_cmd.en_cmd_type == APF_SET_FILTER_CMD) {
        st_apf_filter_cmd.us_program_len = APF_FILTER_LIST_PROGRAM_LEN;
        st_apf_filter_cmd.puc_program = program;
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_APF_FILTER, sizeof(st_apf_filter_cmd));
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
                   &st_apf_filter_cmd, sizeof(st_apf_filter_cmd));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_apf_filter_list::memcpy fail!");
        return OAL_FAIL;
    }
    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_apf_filter_cmd),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_80m_rts_debug::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif

uint32_t wal_hipriv_memory_monitor(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;

    /* 抛事件到wal层处理 */
    /* 拷贝的时候目的地址要预留1字节为后面填充结束符使用,否则可能导致内存越界写风险 */
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value) - 1, pc_param, OAL_STRLEN(pc_param));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_memory_monitor::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_MEMORY_MONITOR, len);

    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_memory_monitor::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
uint32_t wal_hipriv_set_pt_mcast(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t pt_mcast_switch;
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    if (!g_pt_mcast_enable) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_pt_mcast::not support pt mcast!}");
        return OAL_FAIL;
    }
    /* 获取monitor mode开关标志 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_pt_mcast::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_pt_mcast::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 将命令参数值字符串转化为整数 */
    pt_mcast_switch = (uint8_t)oal_atoi(ac_arg);
    if (pt_mcast_switch > 1) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_pt_mcast::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *(uint8_t *)(write_msg.auc_value) = pt_mcast_switch;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_pt_mcast::pt_mcast_switch [%d].}", pt_mcast_switch);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_PT_MCAST, sizeof(uint8_t));

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_pt_mcast::err [%d]!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_pt_mcast fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TWT
OAL_STATIC uint32_t wal_get_twt_setup_req_paras(int8_t *pc_param,
    mac_cfg_twt_setup_req_param_stru* st_twt_setup_req_param)
{
    uint32_t ret;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* 获取interval_exponent */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_twt_setup_req_param->twt_exponent = (uint32_t)oal_atoi(ac_name);
    if (st_twt_setup_req_param->twt_exponent > 0x1F) { /* 最大值0x1F */
        oam_warning_log1(0, OAM_SF_11AX, "{wal_get_twt_setup_req_paras::wrong twt_exponent [%d]}",
            st_twt_setup_req_param->twt_exponent);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + off_set;
    /* 获取wake_duration */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_twt_setup_req_param->twt_duration = (uint32_t)oal_atoi(ac_name);
    if (st_twt_setup_req_param->twt_duration > 0xFF) { /* 最大值0xFF */
        oam_warning_log1(0, OAM_SF_11AX, "{wal_get_twt_setup_req_paras::wrong twt_duration[%d]}",
            st_twt_setup_req_param->twt_duration);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + off_set;
    /* 获取interval_mantissa */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_get_twt_setup_req_paras:: get interval_mantissa fail [%d]!}", ret);
        return ret;
    }
    st_twt_setup_req_param->intrval_mantissa = (uint16_t)oal_atoi(ac_name);

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_twt_setup_req_para_prepare(mac_cfg_twt_setup_req_param_stru *param, int8_t *pc_param)
{
    uint32_t off_set, ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_hipriv_twt_setup_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    memset_s((uint8_t *)param, sizeof(mac_cfg_twt_setup_req_param_stru), 0, sizeof(mac_cfg_twt_setup_req_param_stru));
    oal_strtoaddr(ac_name, sizeof(ac_name), param->auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_hipriv_twt_setup_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }
    param->twt_nego_type = (uint8_t)oal_atoi(ac_name);

    /* 单播处理后续参数 */
    if (param->twt_nego_type == 0) {
        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return ret;
        }
        param->uc_twt_setup_cmd = (uint8_t)oal_atoi(ac_name);

        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return ret;
        }
        param->uc_twt_flow_type = (uint8_t)oal_atoi(ac_name);

        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return ret;
        }
        param->uc_twt_flow_id = (uint8_t)oal_atoi(ac_name);

        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return ret;
        }
        param->twt_start_time_offset = (uint32_t)oal_atoi(ac_name);

        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_twt_setup_req_paras(pc_param, param);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 建立TWT SETUP会话的调测命令
 * 1.日    期  : 2018年12月11日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_twt_setup_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_twt_setup_req_param_stru *pst_twt_setup_req_param;
    mac_cfg_twt_setup_req_param_stru st_twt_setup_req_param; /* 临时保存获取的twt setup req的信息 */
    uint32_t get_addr_idx;

    /*
        设置AMPDU关闭的配置命令: hipriv "Hisilicon0 twt_setup_req xx xx xx xx xx xx(mac地址) nego_type
        setup_cmd flow_type flow_ID start_time_offset interval_exponent wake_duration interval_mantissa"
    */
    oam_warning_log0(0, OAM_SF_11AX, "{wal_hipriv_twt_setup_req\r\n");

    ret = wal_hipriv_twt_setup_req_para_prepare(&st_twt_setup_req_param, pc_param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TWT_SETUP_REQ, sizeof(mac_cfg_twt_setup_req_param_stru));

    /* 设置配置命令参数 */
    pst_twt_setup_req_param = (mac_cfg_twt_setup_req_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_twt_setup_req_param->auc_mac_addr[get_addr_idx] = st_twt_setup_req_param.auc_mac_addr[get_addr_idx];
    }

    pst_twt_setup_req_param->uc_twt_setup_cmd = st_twt_setup_req_param.uc_twt_setup_cmd;
    pst_twt_setup_req_param->uc_twt_flow_type = st_twt_setup_req_param.uc_twt_flow_type;
    pst_twt_setup_req_param->uc_twt_flow_id = st_twt_setup_req_param.uc_twt_flow_id;
    pst_twt_setup_req_param->twt_start_time_offset = st_twt_setup_req_param.twt_start_time_offset;
    pst_twt_setup_req_param->twt_exponent = st_twt_setup_req_param.twt_exponent;
    pst_twt_setup_req_param->twt_duration = st_twt_setup_req_param.twt_duration;
    pst_twt_setup_req_param->intrval_mantissa = st_twt_setup_req_param.intrval_mantissa;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(mac_cfg_twt_setup_req_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_hipriv_twt_setup_req::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


uint32_t _wal_hipriv_twt_setup_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_twt_is_open) {
        return wal_hipriv_twt_setup_req(pst_net_dev, pc_param);
    }

    return OAL_SUCC;
}
/*
 * 功能描述  : 建立TWT TEARDOWN会话的调测命令
 * 1.日    期  : 2018年12月11日
  *   修改内容  : 新生成函数
 */
oal_bool_enum_uint8 check_teardown_paras(uint8_t flow_id, uint8_t nego_type)
{
    /* 单播nego type 小于2，flow id最大0x7，广播nego type3 flow id最大0x1F */
    return (((flow_id < 0x7) && (nego_type < 2)) || ((flow_id < 0x1F) && (nego_type == 3))) ? OAL_TRUE : OAL_FALSE;
}
OAL_STATIC uint32_t wal_hipriv_twt_teardown_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set, ret, get_addr_idx;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_ret;
    mac_cfg_twt_teardown_req_param_stru *pst_twt_teardown_req_param;
    mac_cfg_twt_teardown_req_param_stru st_twt_teardown_req_param; /* 临时保存获取的twt teardown req的信息 */

    /*
     * 设置AMPDU关闭的配置命令:
     * hipriv "Hisilicon0 twt_teardown_req xx xx xx xx xx xx(mac地址) flwo_id nego_type"
     */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    memset_s((uint8_t *)&st_twt_teardown_req_param, sizeof(st_twt_teardown_req_param),
             0, sizeof(st_twt_teardown_req_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_twt_teardown_req_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取flwo_id */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    st_twt_teardown_req_param.uc_twt_flow_id = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取nego_type */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_twt_teardown_req_param.twt_nego_type = (uint8_t)oal_atoi(ac_name);
    if (!check_teardown_paras(st_twt_teardown_req_param.uc_twt_flow_id, st_twt_teardown_req_param.twt_nego_type)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TWT_TEARDOWN_REQ, sizeof(mac_cfg_twt_teardown_req_param_stru));

    /* 设置配置命令参数 */
    pst_twt_teardown_req_param = (mac_cfg_twt_teardown_req_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_twt_teardown_req_param->auc_mac_addr[get_addr_idx] = st_twt_teardown_req_param.auc_mac_addr[get_addr_idx];
    }

    pst_twt_teardown_req_param->uc_twt_flow_id = st_twt_teardown_req_param.uc_twt_flow_id;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_twt_teardown_req_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_hipriv_twt_teardown_req::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

uint32_t _wal_hipriv_twt_teardown_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_twt_is_open) {
        return wal_hipriv_twt_teardown_req(pst_net_dev, pc_param);
    }

    return OAL_SUCC;
}
#endif

/*
 * 功能描述  : 特性INFO日志级别开关配置
 * 1.日    期  : 2014年5月20日,星期二
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_feature_log_switch(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    mac_vap_stru *pst_mac_vap = oal_net_dev_priv(net_dev);
    uint8_t en_feature_id;  // oam_feature_enum
    uint8_t uc_switch_vl;
    uint32_t off_set;
    int8_t ac_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    uint8_t en_log_lvl; // oam_log_level_enum
    wal_msg_write_stru st_write_msg;

    /* OAM log模块的开关的命令: hipriv "Hisilicon0[vapx] feature_log_switch {feature_name} {0/1}"
       1-2(error与warning)级别日志以vap级别为维度；
    */
    /* 获取特性名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pc_param += off_set;

    /* 提供特性名的帮助信息 */
    if ('?' == ac_param[0]) {
        oal_io_print("please input abbr feature name. \r\n");
        oam_show_feature_list();
        return OAL_SUCC;
    }

    /* 获取特性ID */
    ret = oam_get_feature_id((uint8_t *)ac_param, &en_feature_id);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::invalid feature name}\r\n");
        return ret;
    }

    /* 获取开关值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pc_param += off_set;

    /* 获取INFO级别开关状态 */
    if ((oal_strcmp("0", ac_param) != 0) && (oal_strcmp("1", ac_param) != 0)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::invalid switch value}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    uc_switch_vl = (uint8_t)oal_atoi(ac_param);

    /* 关闭INFO日志级别时，恢复成默认的日志级别 */
    en_log_lvl = (uc_switch_vl == OAL_SWITCH_ON) ? OAM_LOG_LEVEL_INFO : OAM_LOG_DEFAULT_LEVEL;
    ret = oam_log_set_feature_level(pst_mac_vap->uc_vap_id, en_feature_id, en_log_lvl);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_FEATURE_LOG, sizeof(int32_t));
    *((uint16_t *)(st_write_msg.auc_value)) = ((en_feature_id << BIT_OFFSET_8) | en_log_lvl);
    ret |= (uint32_t)wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::return err code[%d]!}\r\n", ret);
        return ret;
    }

    return ret;
}
/*
 * 功能描述  : 开启或关闭ampdu发送功能
 * 1.日    期  : 2013年8月27日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_amsdu_tx_on(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t aggr_tx_on;
    uint32_t cmd_id;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_amsdu_tx_on::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    aggr_tx_on = oal_atoi(ac_name);

    ret = wal_get_cmd_id("amsdu_tx_on", &cmd_id, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_amsdu_tx_on:find amsdu_tx_on cmd is fail");
        return ret;
    }
    return wal_process_cmd_params(pst_net_dev, cmd_id, &aggr_tx_on);
}
#ifdef _PRE_WLAN_NARROW_BAND
#ifdef _PRE_WLAN_EXPORT
static void wal_hipriv_autorate_param(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg;
    mac_cfg_set_autorate_param *set_autorate_cfg_param = NULL;
    mac_cfg_set_autorate_param autorate_param = {
        1, /* 使能算法 */
        5, /* 最大档位 */
        0, /* 当前档位 */
        7,
        {{85, -90, NARROW_BW_5M, 111, 3, 170, {0, 0}, WLAN_LEGACY_OFDM_6M_BPS },  /* 5G 0档位参数5M-6m */
         {85, -85, NARROW_BW_10M, 111, 3, 170, {0, 0}, WLAN_LEGACY_OFDM_6M_BPS },  /* 5G 1档位参数10M-6m */
         {85, -80, NARROW_BW_10M, 111, 3, 170, {0, 0}, WLAN_LEGACY_OFDM_9M_BPS },  /* 5G 2档位参数10M-9m */
         {85, -75, NARROW_BW_10M, 111, 2, 170, {0, 0}, WLAN_LEGACY_OFDM_18M_BPS }, /* 5G 3档位参数10M-18m */
         {85, -70, NARROW_BW_10M, 111, 2, 150, {0, 0}, WLAN_LEGACY_OFDM_24M_BPS },  /* 5G 4档位参数10M-24m */
         {85, -60, NARROW_BW_10M, 111, 0, 90, {0, 0}, WLAN_LEGACY_OFDM_48M_BPS },  /* 5G 5档位参数10M-48m */
         {85, -90, NARROW_BW_5M, 111, 2, 170, {0, 0}, WLAN_LEGACY_OFDM_6M_BPS },  /* 2G 0档位参数5M-6m */
         {85, -85, NARROW_BW_10M, 111, 2, 170, {0, 0}, WLAN_LEGACY_OFDM_6M_BPS },  /* 2G 1档位参数10M-6m */
         {85, -80, NARROW_BW_10M, 111, 2, 170, {0, 0}, WLAN_LEGACY_OFDM_9M_BPS },  /* 2G 2档位参数10M-9m */
         {85, -75, NARROW_BW_10M, 111, 2, 170, {0, 0}, WLAN_LEGACY_OFDM_18M_BPS }, /* 2G 3档位参数10M-18m */
         {85, -70, NARROW_BW_10M, 111, 2, 150, {0, 0}, WLAN_LEGACY_OFDM_24M_BPS },  /* 2G 4档位参数10M-24m */
         {85, -60, NARROW_BW_10M, 111, 2, 90, {0, 0}, WLAN_LEGACY_OFDM_48M_BPS }}  /* 2G 5档位参数10M-48m */
    };
    uint32_t ret;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_AUTORATE_PARAM, sizeof(mac_cfg_set_autorate_param));

    /* 解析并设置配置命令参数 */
    set_autorate_cfg_param = (mac_cfg_set_autorate_param *)(write_msg.auc_value);
    memcpy_s(set_autorate_cfg_param, sizeof(mac_cfg_set_autorate_param),
             &autorate_param, sizeof(mac_cfg_set_autorate_param));
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_autorate_param),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_autorate_param::return err code [%d]!}\r\n", ret);
        return;
    }
}
static oal_bool_enum_uint8 wal_nb_is_open(oal_net_device_stru *net_dev)
{
    mac_vap_stru *mac_vap = NULL;

    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_nb_is_open::net_dev null!}");
        return OAL_FALSE;
    }
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_nb_is_open::vap null!}");
        return OAL_FALSE;
    }
    if (mac_vap->st_nb.en_open == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_nb_is_open::nb is close!}");
        return OAL_FALSE;
    }
    return OAL_TRUE;
}
void wal_hipriv_narrow_bw_config_export(oal_net_device_stru *net_dev, oal_bool_enum_uint8 nb_is_open)
{
    mac_vap_stru *mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw_config_export::vap null!}");
        return;
    }
    if (nb_is_open) {
        wal_hipriv_alg_cfg(net_dev, "ar_rts_mode 0");
        wal_hipriv_alg_cfg(net_dev, "cca_opt_alg_en_mode 0");
        wal_hipriv_alg_cfg(net_dev, "ar_enable 0");
        /* 初始化legacy速率 */
        wal_hipriv_set_ucast_data_dscr_param(net_dev, "rate 6");
        wal_hipriv_autorate_param(net_dev);
    } else {
        wal_hipriv_alg_cfg(net_dev, "ar_rts_mode 1");
        wal_hipriv_alg_cfg(net_dev, "cca_opt_alg_en_mode 2");
        wal_hipriv_alg_cfg(net_dev, "ar_enable 1");
    }
}

uint32_t wal_hipriv_export_para_query(oal_net_device_stru *net_dev, int8_t *param)
{
    uint32_t ret, off_set;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t val = 0;

    /* 解析并设置配置命令参数 */
    ret = wal_get_cmd_one_arg(param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_export_para_query::get cmd para fail!}");
        return ret;
    }
    param = param + off_set;
    if ((oal_strcmp("pdet_val", ac_arg)) == 0) {
        return wal_ioctl_get_pdet_val(net_dev, &val);
    } else if ((oal_strcmp("tsensor_val", ac_arg)) == 0) {
        return wal_ioctl_get_tsensor_val(net_dev, &val);
    } else if ((oal_strcmp("get_sw_version", ac_arg)) == 0) {
        return wal_ioctl_get_sw_version(net_dev);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_export_para_query::invalid query cmd.");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
}
uint32_t wal_hipriv_export_set_tpc_param(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int32_t l_ret;
    mac_cfg_power_param_stru *cfg_power;
    uint32_t ret, off_set;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    if (wal_nb_is_open(net_dev) == OAL_FALSE) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_TPC_PARA, sizeof(mac_cfg_power_param_stru));

    /* 解析并设置配置命令参数 */
    cfg_power = (mac_cfg_power_param_stru *)(write_msg.auc_value);
    /* 解析并设置配置命令参数 */
    ret = wal_get_cmd_one_arg(param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_export_set_tpc_param::get cmd para fail!}");
        return ret;
    }
    param = param + off_set;
    if ((oal_strcmp("upc", ac_arg)) == 0) {
        cfg_power->type = POWER_PARAM_UPC; /* 修改UPC */
    } else if ((oal_strcmp("lpf", ac_arg)) == 0) {
        cfg_power->type = POWER_PARAM_LPF; /* 修改LPF */
    } else if ((oal_strcmp("dbb", ac_arg)) == 0) {
        cfg_power->type = POWER_PARAM_DBB; /* 修改DBB */
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_export_set_tpc_param::type error should be upc/lpf/dbb!");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ret = wal_get_cmd_one_arg(param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_export_set_tpc_param::get cmd para fail!}");
        return ret;
    }
    param = param + off_set;
    cfg_power->value = (uint8_t)oal_atoi(ac_arg);
    oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_export_set_tpc_param::set type[%d] val[%d]!}", cfg_power->type,
        cfg_power->value);
    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_power_param_stru), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

uint32_t wal_hipriv_nb_info_report(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int32_t l_ret;

    if (wal_nb_is_open(net_dev) == OAL_FALSE) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_NB_INFO_REPORT, sizeof(int8_t));

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH, (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

const uint8_t *g_pauc_autorate_cfg_name[WAL_AUTORATE_CFG_PARAM_BUTT] = {
    "enable",
    "level",
};
uint32_t wal_hipriv_autorate_cfg_set(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    mac_cfg_set_autorate_cfg_stru *set_autorate_cfg_param = NULL;
    int32_t ret;
    uint32_t offset;
    wal_autorate_cfg_enum param_index;
    uint8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_AUTORATE_CFG_SET, sizeof(mac_cfg_set_autorate_cfg_stru));

    /* 解析并设置配置命令参数 */
    set_autorate_cfg_param = (mac_cfg_set_autorate_cfg_stru *)(write_msg.auc_value);

    ret = wal_get_cmd_one_arg(param, ac_arg, sizeof(ac_arg), &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_autorate_cfg_set::wal_get_cmd_one_arg fail err_code [%d]!}", ret);
        return ret;
    }
    param += offset;

    for (param_index = 0; param_index < WAL_AUTORATE_CFG_PARAM_BUTT; param_index++) {
        if (!strcmp(g_pauc_autorate_cfg_name[param_index], ac_arg)) {
            break;
        }
    }

    if (param_index == WAL_AUTORATE_CFG_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_autorate_cfg_set::no such param!}");
        return OAL_FAIL;
    }

    set_autorate_cfg_param->en_type = param_index;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::en_type [%d]!}", set_autorate_cfg_param->en_type);

    ret = wal_get_cmd_one_arg(param, ac_arg, sizeof(ac_arg), &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    set_autorate_cfg_param->val = (uint32_t)oal_atoi(ac_arg);

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_autorate_cfg_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_autorate_cfg_set::return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif
/*
 * 功能描述  : 配置窄带工作模式
 * 1.日    期  : 2016年7月28日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_narrow_bw(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    mac_cfg_narrow_bw_stru *pst_nrw_bw;
    uint32_t ret, off_set;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_NARROW_BW, sizeof(mac_cfg_narrow_bw_stru));

    /* 解析并设置配置命令参数 */
    pst_nrw_bw = (mac_cfg_narrow_bw_stru *)(st_write_msg.auc_value);

    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::get switch  [%d]!}\r\n", ret);
        return ret;
    }
    pc_param = pc_param + off_set;

    pst_nrw_bw->en_open = (uint8_t)oal_atoi(ac_arg);

    /* 窄带打开模式下强制关闭ampdu amsdu聚合和若干扰免疫算法 */
    if (pst_nrw_bw->en_open == OAL_TRUE) {
        wal_hipriv_alg_cfg(pst_net_dev, "anti_inf_unlock_en 0");
        wal_hipriv_ampdu_tx_on(pst_net_dev, "0");
        wal_hipriv_amsdu_tx_on(pst_net_dev, "0");
    } else {
        wal_hipriv_alg_cfg(pst_net_dev, "anti_inf_unlock_en 1");
        wal_hipriv_ampdu_tx_on(pst_net_dev, "1");
        wal_hipriv_amsdu_tx_on(pst_net_dev, "1");
    }
#ifdef _PRE_WLAN_EXPORT
    wal_hipriv_narrow_bw_config_export(pst_net_dev, pst_nrw_bw->en_open);
#endif
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::get switch  [%d]!}\r\n", ret);
        return ret;
    }
    pc_param = pc_param + off_set;

    if ((oal_strcmp("1m", ac_arg)) == 0) {
        pst_nrw_bw->en_bw = NARROW_BW_1M;
    } else if ((oal_strcmp("5m", ac_arg)) == 0) {
        pst_nrw_bw->en_bw = NARROW_BW_5M;
    } else if ((oal_strcmp("10m", ac_arg)) == 0) {
        pst_nrw_bw->en_bw = NARROW_BW_10M;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::bw should be 1/5/10 m");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_narrow_bw_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
/*
 * 功能描述  : 设置WMMAC的开关
 * 1.日    期  : 2016年9月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_wmmac_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_wmm_ac_param_stru st_wmm_ac_param;

    /* 设置删除TS的配置命令: hipriv "vap0 wmmac_switch 1/0(使能) 0|1(WMM_AC认证使能) AC xxx(limit_medium_time)" */
    memset_s(&st_wmm_ac_param, sizeof(mac_cfg_wmm_ac_param_stru), 0, sizeof(mac_cfg_wmm_ac_param_stru));

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::get switch err[%d]!}\r\n", ret);
        return ret;
    }
    st_wmm_ac_param.en_wmm_ac_switch = ((uint8_t)oal_atoi(ac_name) & BIT0);
    pc_param += off_set;

    /* 获取auth flag */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::get auth flag err[%d]!}\r\n", ret);
        return ret;
    }
    st_wmm_ac_param.en_auth_flag = (uint8_t)oal_atoi(ac_name);
    pc_param += off_set;

    /* timeout period ms */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::get time period err[%d]!}\r\n", ret);
        return ret;
    }
    st_wmm_ac_param.us_timeout_period = (uint16_t)oal_atoi(ac_name);
    pc_param += off_set;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_wmmac_switch::get factor[%d]!}\r\n", ret);
        return ret;
    }
    st_wmm_ac_param.uc_factor = (uint8_t)oal_atoi(ac_name);
    pc_param += off_set;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 设置配置命令参数 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_WMMAC_SWITCH, sizeof(st_wmm_ac_param));
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_wmm_ac_param, sizeof(st_wmm_ac_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_delts::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_wmm_ac_param), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delts::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#endif

/*
 * 功能描述  : 指定用户指定tid 发送bar
 * 1.日    期  : 2013年8月27日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_send_bar(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_pause_tid_param_stru *pst_pause_tid_param = NULL;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
    uint8_t uc_tid;

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_send_bar::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_send_bar::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    uc_tid = (uint8_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SEND_BAR, sizeof(mac_cfg_pause_tid_param_stru));

    /* 设置配置命令参数 */
    pst_pause_tid_param = (mac_cfg_pause_tid_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_pause_tid_param->auc_mac_addr, auc_mac_addr);
    pst_pause_tid_param->uc_tid = uc_tid;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_pause_tid_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_bar::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
/*
 * 功能描述  : 测试命令，打开/关闭WMM
 * 1.日    期  : 2014年1月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_wmm_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    int32_t l_cfg_rst;
    uint16_t us_len;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t uc_open_wmm;

    /* 获取设定的值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_wmm_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    uc_open_wmm = (uint8_t)oal_atoi(ac_name);
    pc_param += off_set;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    us_len = sizeof(uint8_t);
    *(uint8_t *)(st_write_msg.auc_value) = uc_open_wmm;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_WMM_SWITCH, us_len);

    l_cfg_rst = wal_send_cfg_event(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                   (uint8_t *)&st_write_msg,
                                   OAL_FALSE,
                                   NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_wmm_switch::return err code [%d]!}\r\n", l_cfg_rst);
        return (uint32_t)l_cfg_rst;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_CSI
/*
 * 功能描述  : 使能CSI配置命令
 * 1.日    期  : 2017年11月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_csi(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_csi_switch(pst_cfg_net_dev, pc_param);
}
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*
 * 功能描述  : 设置添加用户的配置命令
 * 1.日    期  : 2013年6月5日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_get_user_nssbw(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_add_user_param_stru *pst_add_user_param;
    mac_cfg_add_user_param_stru st_add_user_param; /* 临时保存获取的use的信息 */
    uint32_t get_addr_idx;

    /* 获取用户带宽和空间流信息: hipriv "vap0 add_user xx xx xx xx xx xx(mac地址)" */
    memset_s((void *)&st_add_user_param, sizeof(mac_cfg_add_user_param_stru),
             0, sizeof(mac_cfg_add_user_param_stru));
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_add_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), st_add_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 设置配置命令参数 */
    pst_add_user_param = (mac_cfg_add_user_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_add_user_param->auc_mac_addr[get_addr_idx] = st_add_user_param.auc_mac_addr[get_addr_idx];
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_GET_USER_RSSBW, sizeof(mac_cfg_add_user_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_user_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_user::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#endif

/*
 * 1.日    期  : 2014年7月29日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_blacklist_add(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;
    mac_blacklist_stru *pst_blklst = NULL;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_blacklist_add:wal_get_cmd_one_arg fail!}\r\n");
        return ret;
    }
    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    pst_blklst = (mac_blacklist_stru *)(st_write_msg.auc_value);
    /* 将字符 ac_name 转换成数组 mac_add[6] */
    oal_strtoaddr(ac_name, sizeof(ac_name), pst_blklst->auc_mac_addr, WLAN_MAC_ADDR_LEN);

    us_len = sizeof(mac_blacklist_stru);
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ADD_BLACK_LIST, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_blacklist_add:wal_send_cfg_event return[%d].}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 1.日    期  : 2014年7月29日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_blacklist_del(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_blacklist_del:wal_get_cmd_one_arg fail!}\r\n");
        return ret;
    }
    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    /* 将字符 ac_name 转换成数组 mac_add[6] */
    oal_strtoaddr(ac_name, sizeof(ac_name), st_write_msg.auc_value, WLAN_MAC_ADDR_LEN);

    us_len = OAL_MAC_ADDR_LEN; /* sizeof(uint8_t); */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DEL_BLACK_LIST, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_blacklist_del:wal_send_cfg_event return[%d].}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 1.日    期  : 2014年7月29日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_blacklist_mode(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WAL_HIPRIV_CMD_MAX_LEN, WLAN_CFGID_BLACKLIST_MODE);
}

#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))
/*
 * 功能描述  : 显示wapi/pwl的调试信息
 * 1.日    期  : 2014年8月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
#ifdef _PRE_WAPI_DEBUG
uint32_t wal_hipriv_show_wpi_info(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    mac_vap_stru *pst_mac_vap;
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint32_t ret, offset;
    mac_cfg_user_info_param_stru *pst_user_info_param;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 }; /* 临时保存获取的use的mac地址信息,6字节 */
    uint16_t us_user_idx;

    /* 获取mac地址 */
    ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_show_wapi_info::wal_hipriv_get_mac_addr fail!}");
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_WPI_INFO, sizeof(mac_cfg_user_info_param_stru));

    /* 根据mac地址找用户 */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);

    l_ret = (int32_t)mac_vap_find_user_by_macaddr(pst_mac_vap, auc_mac_addr, &us_user_idx);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_show_wpi_info::no such user!}\r\n");
        return OAL_FAIL;
    }

    /* 设置配置命令参数 */
    pst_user_info_param = (mac_cfg_user_info_param_stru *)(st_write_msg.auc_value);
    pst_user_info_param->us_user_idx = us_user_idx;

    oam_warning_log1(0, OAM_SF_ANY, "wal_hipriv_show_wpi_info::us_user_idx %u", us_user_idx);
    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_user_info_param_stru),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_hipriv_show_wpi_info::return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif
#endif

uint32_t wal_hipriv_parase_send_frame_body(int8_t *p_str_body, uint8_t uc_str_body_len,
    uint8_t *p_char_body, uint8_t uc_char_body_lenth,
    uint8_t *p_char_body_len)
{
    uint8_t uc_index;
    uint8_t uc_value = 0;
    uint8_t uc_hvalue = 0;
    uint8_t uc_char_body_len = 0;

    if (oal_any_null_ptr3(p_str_body, p_char_body, p_char_body_len)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_parase_send_frame_body::param is NULL,return!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((uc_str_body_len % 2) != 0) { /* 2用来计算奇偶数 */
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_parase_send_frame_body::uc_str_body_len=%d invalid,return!}\r\n", uc_str_body_len);
        return OAL_FAIL;
    }

    for (uc_index = 0; uc_index < uc_str_body_len; uc_index++) {
        if (p_str_body[uc_index] >= '0' && p_str_body[uc_index] <= '9') {
            uc_value = (uint8_t)(p_str_body[uc_index] - '0');
        } else if (p_str_body[uc_index] >= 'A' && p_str_body[uc_index] <= 'F') {
            uc_value = (uint8_t)(p_str_body[uc_index] - 'A' + 10); /* 'A'表示数值10 */
        } else {
            oam_warning_log2(0, OAM_SF_ANY,
                "{wal_hipriv_parase_send_frame_body::p_str_body[%d]=%c invalid,return!}\r\n",
                uc_index, (uint8_t)p_str_body[uc_index]);
            return OAL_FAIL;
        }

        if ((uc_index % 2) == 0) { /* 2用来计算奇偶数 */
            uc_hvalue = ((uc_value << BIT_OFFSET_4) & 0XF0);
        } else if ((uc_index % 2) == 1) { /* 2用来计算奇偶数 */
            p_char_body[uc_char_body_len] = uc_value + uc_hvalue;
            uc_value = 0;
            uc_char_body_len++;
            if (uc_char_body_len >= uc_char_body_lenth) {
                oam_warning_log2(0, OAM_SF_ANY,
                    "{wal_hipriv_parase_send_frame_body::uc_char_body_len =%d over_max = %d!}\r\n",
                    uc_char_body_len, MAC_TEST_INCLUDE_FRAME_BODY_LEN);
                return OAL_FAIL;
            }
        } else {
            uc_value = 0;
        }
    }

    *p_char_body_len = uc_char_body_len;
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_send_frame_cmd_analyze(int8_t **input_param, uint8_t *en_frame_type, uint8_t *uc_pkt_num)
{
    uint32_t offset;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int8_t *pc_param = *input_param;

    /* 获取帧类型 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::get frame type err_code [%d]!}\r\n", ret);
        return ret;
    }
    *en_frame_type = (mac_test_frame_type_enum_uint8)oal_atoi(ac_name);
    pc_param = pc_param + offset;

    /* 获取帧数目 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::get frame num err_code [%d]!}\r\n", ret);
        return ret;
    }
    *uc_pkt_num = (uint8_t)oal_atoi(ac_name);
    pc_param += offset;

    *input_param = pc_param;
    return OAL_SUCC;
}
OAL_STATIC uint32_t wal_hipriv_send_frame_para_prepare(wal_msg_write_stru *write_msg, int8_t *pc_param)
{
    uint32_t offset, ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_send_frame_param_stru *frame = NULL;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };
    /* 所需内存是MAC_TEST_INCLUDE_FRAME_BODY_LEN的2倍 */
    int8_t ac_str_frame_body[MAC_TEST_INCLUDE_FRAME_BODY_LEN * 2] = { 0 };
    uint8_t uc_str_frame_body_len, uc_pkt_num, en_frame_type;
    uint8_t ac_frame_body[MAC_TEST_INCLUDE_FRAME_BODY_LEN] = { 0 };
    uint8_t uc_frame_body_len = 0;

    ret = wal_hipriv_send_frame_cmd_analyze(&pc_param, &en_frame_type, &uc_pkt_num);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 获取字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::get mac err_code [%d]!}\r\n", ret);
        return ret;
    }
    /* 地址字符串转地址数组 */
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
    pc_param += offset;

    if (en_frame_type == MAC_TEST_MGMT_ACTION || en_frame_type == MAC_TEST_MGMT_BEACON_INCLUDE_IE) {
        ret = wal_get_cmd_one_arg(pc_param, ac_str_frame_body, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::get frame body err_code = [%d]!}\r\n", ret);
            return ret;
        }

        pc_param += offset;
        uc_str_frame_body_len = (uint8_t)(OAL_STRLEN(ac_str_frame_body));
        ret = wal_hipriv_parase_send_frame_body(ac_str_frame_body, uc_str_frame_body_len, ac_frame_body,
            sizeof(ac_frame_body), &uc_frame_body_len);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::parase_send_frame_body err_code = [%d]!}", ret);
            return ret;
        }
    }

    /* 设置配置命令参数 */
    frame = (mac_cfg_send_frame_param_stru *)(write_msg->auc_value);
    memset_s(frame, sizeof(mac_cfg_send_frame_param_stru),
             0, sizeof(mac_cfg_send_frame_param_stru));
    oal_set_mac_addr(frame->auc_mac_ra, auc_mac_addr);
    frame->en_frame_type = en_frame_type;
    frame->uc_pkt_num = uc_pkt_num;
    frame->uc_frame_body_length = uc_frame_body_len;
    if (memcpy_s(frame->uc_frame_body, sizeof(frame->uc_frame_body), ac_frame_body, uc_frame_body_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_send_frame::memcpy fail!}");
        return OAL_FAIL;
    }
    oam_warning_log3(0, OAM_SF_ANY, "{wal_hipriv_send_frame:: frame_type = [%d] send_times = %d body_len=%d!}\r\n",
        frame->en_frame_type, frame->uc_pkt_num, frame->uc_frame_body_length);

    return OAL_SUCC;
}

uint32_t wal_hipriv_send_frame(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;

    ret = wal_hipriv_send_frame_para_prepare(&st_write_msg, pc_param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /***************************************************************************
                                 抛事件到dmac层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SEND_FRAME, sizeof(mac_cfg_send_frame_param_stru));

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_send_frame_param_stru),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::wal_send_cfg_event return err_code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_NAN
OAL_STATIC uint32_t wal_hipriv_nan_set_param(oal_net_device_stru *net_device, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t offset;
    uint32_t ret;
    mac_nan_cfg_msg_stru *nan_msg;
    mac_nan_param_stru *nan_param;

    nan_msg = (mac_nan_cfg_msg_stru*)write_msg.auc_value;
    nan_msg->type = NAN_CFG_TYPE_SET_PARAM;
    nan_param = &nan_msg->param;

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    nan_param->duration = (uint32_t)oal_atoi(ac_name);

    param += offset;
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    nan_param->period = (uint32_t)oal_atoi(ac_name);

    param += offset;
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    nan_param->band = (uint8_t)oal_atoi(ac_name);

    param += offset;
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    nan_param->channel = (uint8_t)oal_atoi(ac_name);
    wal_nan_correction_param(nan_param);

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_NAN, sizeof(mac_nan_cfg_msg_stru));
    if (OAL_SUCC != wal_send_cfg_event(net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_nan_cfg_msg_stru), (uint8_t*)&write_msg, OAL_FALSE,
        NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfgvendor_nan_send_event::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
OAL_STATIC uint32_t wal_hipriv_nan_tx_mgmt(oal_net_device_stru *net_device, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t offset;
    uint32_t ret;
    mac_nan_cfg_msg_stru *nan_msg;
    mac_nan_mgmt_info_stru *mgmt_info;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint8_t ac_frame_body[WAL_HIPRIV_CMD_NAME_MAX_LEN >> 1] = { 0 };
    uint8_t str_frame_len;
    uint8_t frame_len = 0;
    nan_msg = (mac_nan_cfg_msg_stru*)write_msg.auc_value;
    nan_msg->type = NAN_CFG_TYPE_SET_TX_MGMT;
    mgmt_info = &nan_msg->mgmt_info;

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    mgmt_info->periodic = !(uint8_t)oal_atoi(ac_name);
    mgmt_info->action = WLAN_ACTION_NAN_FLLOWUP; /* 调试命令写死 */

    param += offset;
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }

    str_frame_len = (uint8_t)(OAL_STRLEN(ac_name));
    ret = wal_hipriv_parase_send_frame_body(ac_name, str_frame_len, ac_frame_body,
                                            sizeof(ac_frame_body), &frame_len);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_send_frame::parase_send_frame_body err_code = [%d]!}\r\n", ret);
    }
    mgmt_info->len = frame_len;
    mgmt_info->data = ac_frame_body;

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_NAN, sizeof(mac_nan_cfg_msg_stru));
    if (OAL_SUCC != wal_send_cfg_event(net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_nan_cfg_msg_stru), (uint8_t*)&write_msg, OAL_TRUE,
        &pst_rsp_msg)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_nan_tx_mgmt::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

uint32_t wal_hipriv_nan_dbg(oal_net_device_stru *net_device, int8_t *param)
{
    uint32_t ret;
    uint32_t offset;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    if (oal_any_null_ptr2(net_device, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_nan_dbg::first para return err_code [%d]!}", ret);
        return ret;
    }
    param += offset;
    if (oal_strcmp(ac_name, "set_param") == 0) {
        ret = wal_hipriv_nan_set_param(net_device, param);
    } else if (oal_strcmp(ac_name, "tx_mgmt") == 0) {
        ret = wal_hipriv_nan_tx_mgmt(net_device, param);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_nan_dbg::not support the sub cmd!}");
    }
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_nan_dbg::execute cmd failed, error[%d]!}", ret);
    }
    return ret;
}
#endif
#ifdef _PRE_WLAN_FEATURE_VSP
OAL_STATIC uint32_t wal_hipriv_vsp_link_en(oal_net_device_stru *net_device, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t offset;
    uint32_t ret;
    hmac_vsp_debug_cmd *cmd;
    int i;

    cmd = (hmac_vsp_debug_cmd *)write_msg.auc_value;
    cmd->type = VSP_CMD_LINK_EN;

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += offset;
    cmd->link_en = (uint8_t)oal_atoi(ac_name);

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    param += offset;
    if (strlen(ac_name) != 17 || sscanf_s(ac_name, "%02x:%02x:%02x:%02x:%02x:%02x\n", /* mac地址字符串长度17 */
        &cmd->user_mac[BYTE_OFFSET_0], &cmd->user_mac[BYTE_OFFSET_1], &cmd->user_mac[BYTE_OFFSET_2],
        &cmd->user_mac[BYTE_OFFSET_3], &cmd->user_mac[BYTE_OFFSET_4], &cmd->user_mac[BYTE_OFFSET_5]) != MAC_ADDR_LEN) {
        return OAL_FAIL;
    }

    if (cmd->link_en) {
        ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (ret != OAL_SUCC) {
            return ret;
        }
        param += offset;
        cmd->mode = (uint8_t)oal_atoi(ac_name);
        for (i = 0; i < VSP_PARAM_BUTT; i++) {
            ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
            if (ret != OAL_SUCC) {
                return ret;
            }
            param += offset;
            cmd->param[i] = (uint8_t)oal_atoi(ac_name);
        }
    }

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_VSP_DEBUG, sizeof(hmac_vsp_debug_cmd));
    if (OAL_SUCC != wal_send_cfg_event(net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_vsp_debug_cmd), (uint8_t*)&write_msg, OAL_FALSE, NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_vsp_link_en::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_vsp_test_en(oal_net_device_stru *net_device, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t offset;
    uint32_t ret;
    hmac_vsp_debug_cmd *cmd;

    cmd = (hmac_vsp_debug_cmd *)write_msg.auc_value;
    cmd->type = VSP_CMD_TEST_EN;

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    cmd->test_en = (uint8_t)oal_atoi(ac_name);
    param += offset;

    if (cmd->test_en) {
        ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (ret != OAL_SUCC) {
            return ret;
        }
        cmd->test_frm_cnt = (uint32_t)oal_atoi(ac_name);
    }
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_VSP_DEBUG, sizeof(hmac_vsp_debug_cmd));
    if (OAL_SUCC != wal_send_cfg_event(net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_vsp_debug_cmd), (uint8_t*)&write_msg, OAL_FALSE, NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_vsp_link_en::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

uint32_t wal_hipriv_vsp_dbg(oal_net_device_stru *net_device, int8_t *param)
{
    uint32_t ret;
    uint32_t offset;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    if (oal_any_null_ptr2(net_device, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_vsp_dbg::first para return err_code [%d]!}", ret);
        return ret;
    }
    param += offset;
    if (oal_strcmp(ac_name, "link_en") == 0) {
        ret = wal_hipriv_vsp_link_en(net_device, param);
    } else if (oal_strcmp(ac_name, "test_en") == 0) {
        ret = wal_hipriv_vsp_test_en(net_device, param);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_vsp_dbg::not support the sub cmd!}");
    }
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_vsp_dbg::execute cmd failed, error[%d]!}", ret);
    }
    return ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
/*
 * 功能描述   : 设置用户的hiex himit enable flag
                设置命令:sh hipriv.sh "wlan0 set_user_himit xx:xx:xx:xx:xx:xx 0xXXX"
 * 1.日    期   : 2019年12月07日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_set_user_hiex_enable(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru             st_write_msg;
    uint32_t                     off_set;
    int8_t                       ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t                     ret;
    int32_t                      l_ret;
    mac_cfg_user_hiex_param_stru  *pst_user_hiex_param = NULL;
    uint8_t                      auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };
    uint32_t                     hiex_flag;

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_user_hiex_enable::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_user_hiex_enable::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    hiex_flag = (uint32_t)oal_strtol(ac_name, NULL, NUM_16_BITS);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_USER_HIEX_ENABLE, sizeof(mac_cfg_user_hiex_param_stru));

    /* 设置配置命令参数 */
    pst_user_hiex_param = (mac_cfg_user_hiex_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_user_hiex_param->auc_mac_addr, auc_mac_addr);
    *(uint32_t *)&pst_user_hiex_param->st_hiex_cap = hiex_flag;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_user_hiex_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_user_hiex_enable::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

uint32_t _wal_hipriv_set_user_hiex_enable(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        return wal_hipriv_set_user_hiex_enable(pst_net_dev, pc_param);
    }
    return OAL_SUCC;
}
#endif

OAL_STATIC void wal_hex_key_convert(int8_t *pc_key, uint8_t auc_key[OAL_WPA_KEY_LEN],
    uint32_t off_set, uint32_t len)
{
    uint32_t char_index;
    for (char_index = 0; char_index < off_set; char_index++) {
        if (*pc_key == '-') {
            pc_key++;
            if (char_index != 0) {
                char_index--;
            }

            continue;
        }
        /* 2、16为16进制转换算法相关 */
        auc_key[char_index / 2] = (uint8_t)(auc_key[char_index / 2] * 16 * (char_index % 2) +
                                   oal_strtohex(pc_key));
        pc_key++;
    }
}

OAL_STATIC uint32_t wal_hipriv_test_add_key_param_analyze(mac_addkey_param_stru *payload_params, int8_t **input_param)
{
    uint32_t ret, off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t *pc_param = *input_param;
    /* 获取cipher */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    payload_params->st_key.cipher = (uint32_t)oal_atoi(ac_name);
    pc_param = pc_param + off_set;

    /* 获取en_pairwise */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    payload_params->en_pairwise = (uint8_t)oal_atoi(ac_name);
    pc_param = pc_param + off_set;

    /* 获取key_len */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }

    pc_param = pc_param + off_set;
    payload_params->st_key.key_len = (uint8_t)oal_atoi(ac_name);
    if (oal_value_not_in_valid_range(payload_params->st_key.key_len, 0, OAL_WPA_KEY_LEN)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::Param Check ERROR! key_len[%x]  }\r\n",
                       (int32_t)payload_params->st_key.key_len);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    *input_param = pc_param;
    return OAL_SUCC;
}
OAL_STATIC uint32_t wal_hipriv_test_add_key_param_prepare(mac_addkey_param_stru *payload_params, int8_t *pc_param)
{
    uint32_t ret, off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t auc_key[OAL_WPA_KEY_LEN] = { 0 };
    int8_t *pc_key = NULL;

    ret = wal_hipriv_test_add_key_param_analyze(payload_params, &pc_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 获取key_index */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    payload_params->uc_key_index = (uint8_t)oal_atoi(ac_name);
    pc_param = pc_param + off_set;

    /* 获取key */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    pc_param = pc_param + off_set;
    pc_key = ac_name;
    /* 16进制转换, 最后一个参数是为了codex过 */
    wal_hex_key_convert(pc_key, auc_key, off_set, sizeof(auc_key));

    if (memcpy_s(payload_params->st_key.auc_key, sizeof(payload_params->st_key.auc_key),
        auc_key, (uint32_t)payload_params->st_key.key_len) != EOK) {
        return OAL_FAIL;
    }

    /* 获取目的地址 */
    memset_s(ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, 0, WAL_HIPRIV_CMD_NAME_MAX_LEN);
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), payload_params->auc_mac_addr, WLAN_MAC_ADDR_LEN);

    oam_info_log3(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::key_len:%d, seq_len:%d, cipher:0x%x!}",
        (uint32_t)payload_params->st_key.key_len, (uint32_t)payload_params->st_key.seq_len,
        payload_params->st_key.cipher);
    return OAL_SUCC;
}
/*
 * 功能描述  : 芯片测试接口：add key
 * 1.日    期  : 2015年1月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_test_add_key(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_addkey_param_stru st_payload_params;
    int32_t l_ret;
    uint32_t ret;
    uint16_t us_len;
    wal_msg_stru *pst_rsp_msg = NULL;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr2(pst_net_dev, pc_param)) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_hipriv_test_add_key::Param Check ERROR,pst_netdev or pst_params is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*
     * xxx(cipher) xx(en_pairwise) xx(key_len) xxx(key_index) xxxx:xx:xx:xx:xx:xx...
     *  (key 小于32字节) xx:xx:xx:xx:xx:xx(目的地址)
     */
    memset_s(&st_payload_params, sizeof(st_payload_params), 0, sizeof(st_payload_params));
    memset_s(&st_payload_params.st_key, sizeof(mac_key_params_stru), 0, sizeof(mac_key_params_stru));
    st_payload_params.st_key.seq_len = 6; /* 6表示seq长度 */
    memset_s(st_payload_params.auc_mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);

    ret = wal_hipriv_test_add_key_param_prepare(&st_payload_params, pc_param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    /* 3.2 填写 msg 消息体 */
    us_len = (uint32_t)sizeof(mac_addkey_param_stru);

    if (memcpy_s((int8_t *)st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
        (int8_t *)&st_payload_params, (uint32_t)us_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::memcpy fail!}");
        return OAL_FAIL;
    }

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ADD_KEY, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_enable_pmf::wal_send_cfg_event return err_code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_test_add_key::wal_check_and_release_msg_resp fail");
    }

    return OAL_SUCC;
}

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/*
 * 功能描述  : 命令配置触发STA发送11V Query帧
 * 1.日    期  : 2017年3月30日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_11v_tx_query(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint32_t ret;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_hipriv_11v_tx_query::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_11V_TX_QUERY, sizeof(auc_mac_addr));

    /* 设置配置命令参数 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), auc_mac_addr, WLAN_MAC_ADDR_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_dpd_cfg::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(auc_mac_addr),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_11v_tx_query::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#endif

/*
 * 功能描述  : 设置APUT模式下支持的OWE group
 * 1.日    期  : 2019年1月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
#define WAL_HIPRIV_OWE_MAX 8
uint32_t wal_hipriv_set_owe(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    int32_t l_ret;
    uint8_t uc_owe_group;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    wal_msg_stru *pst_rsp_msg = NULL;

    /* 获取字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_owe::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    /* 解析要设置的值 */
    uc_owe_group = (uint8_t)oal_atoi(ac_arg);
    if (uc_owe_group >= WAL_HIPRIV_OWE_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_owe::input value out of range [%d]!}\r\n", uc_owe_group);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_OWE, sizeof(uint8_t));

    /* 解析并设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), &uc_owe_group, sizeof(uint8_t)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_owe::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                               (uint8_t *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_owe::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_owe fail, err code[%u]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

static uint32_t wal_hipriv_mintp_test_parse_cmd(int8_t **param, hmac_mintp_test_param_stru *test_param)
{
    uint32_t offset;
    int8_t cmd_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    if (wal_get_cmd_one_arg(*param, cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset) != OAL_SUCC) {
        return OAL_FAIL;
    }
    *param += offset;

    test_param->cmd = (uint8_t)oal_atoi(cmd_param);

    return OAL_SUCC;
}

static uint32_t wal_hipriv_mintp_test_parse_mac_addr(int8_t **param, hmac_mintp_test_param_stru *test_param)
{
    uint32_t offset;
    int8_t cmd_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    if (wal_get_cmd_one_arg(*param, cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset) != OAL_SUCC) {
        return OAL_FAIL;
    }
    *param += offset;

    /* mac地址字符串长度17检查 */
    if (strlen(cmd_param) != 17 || sscanf_s(cmd_param, "%02x:%02x:%02x:%02x:%02x:%02x\n",
        &test_param->user_mac[BYTE_OFFSET_0], &test_param->user_mac[BYTE_OFFSET_1],
        &test_param->user_mac[BYTE_OFFSET_2], &test_param->user_mac[BYTE_OFFSET_3],
        &test_param->user_mac[BYTE_OFFSET_4], &test_param->user_mac[BYTE_OFFSET_5]) != MAC_ADDR_LEN) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

static uint32_t wal_hipriv_mintp_test_parse_loop_cnt(int8_t **param, hmac_mintp_test_param_stru *test_param)
{
    uint32_t offset;
    int8_t cmd_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    if (wal_get_cmd_one_arg(*param, cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset) != OAL_SUCC) {
        return OAL_FAIL;
    }
    *param += offset;

    test_param->loop_cnt = (uint8_t)oal_atoi(cmd_param);

    return OAL_SUCC;
}

static uint32_t wal_hipriv_mintp_test_parse_netbuf_cnt(int8_t **param, hmac_mintp_test_param_stru *test_param)
{
    uint32_t offset;
    int8_t cmd_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    if (wal_get_cmd_one_arg(*param, cmd_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset) != OAL_SUCC) {
        return OAL_FAIL;
    }
    *param += offset;

    test_param->netbuf_cnt = (uint8_t)oal_atoi(cmd_param);

    return OAL_SUCC;
}

uint32_t wal_hipriv_mintp_test(oal_net_device_stru *net_device, int8_t *param)
{
    wal_msg_write_stru write_msg = { 0 };
    hmac_mintp_test_param_stru *test_param = (hmac_mintp_test_param_stru *)write_msg.auc_value;

    if (oal_any_null_ptr2(net_device, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (wal_hipriv_mintp_test_parse_cmd(&param, test_param) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (wal_hipriv_mintp_test_parse_mac_addr(&param, test_param) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (wal_hipriv_mintp_test_parse_loop_cnt(&param, test_param) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (wal_hipriv_mintp_test_parse_netbuf_cnt(&param, test_param) != OAL_SUCC) {
        return OAL_FAIL;
    }

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_MINTP_TEST, sizeof(hmac_mintp_test_param_stru));
    return (uint32_t)wal_send_cfg_event(net_device, WAL_MSG_TYPE_WRITE,
                                        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_mintp_test_param_stru),
                                        (uint8_t*)&write_msg, OAL_FALSE, NULL);
}

#endif
