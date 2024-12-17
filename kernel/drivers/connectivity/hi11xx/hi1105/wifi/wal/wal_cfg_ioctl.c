/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :
 * 创建日期 : 2020年6月17日
 */
#include "wal_cfg_ioctl.h"
#include "oal_types.h"
#include "oam_stat_wifi.h"
#include "wal_config.h"
#include "hmac_package_params.h"
#include "hmac_vap.h"
#include "mac_mib.h"
#include "hmac_scan.h"
#include "oal_util.h"
#include "plat_pm_wlan.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_hid2d.h"
#include "hmac_resource.h"
#include "wal_linux_ioctl.h"
#ifdef _PRE_WLAN_FEATURE_PMF
#include "hmac_11w.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
#include "wal_dfx.h"
#include "hmac_dfx.h"
#endif
#include "hmac_blacklist.h"
#ifdef _PRE_WLAN_TCP_OPT
#include "hmac_tcp_opt.h"
#endif
#include "hmac_roam_main.h"
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif
#include "hmac_tx_switch.h"
#include "wlan_chip_i.h"
#ifdef _PRE_WLAN_FEATURE_GNSS_RSMC
#include "hmac_gnss_rsmc.h"
#endif
#include "hmac_wapi.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#include "hmac_chba_channel_sequence.h"
#endif
#include "hmac_sae.h"
#ifdef _PRE_WLAN_FEATURE_VSP
#include "hmac_vsp_if.h"
#endif
#include "hmac_dbac.h"
#include "hmac_mintp_log.h"
#include "wal_cfg_release_ioctl.h"
#include "wal_cfg_stat_ioctl.h"
#include "wal_cfg_pm_ioctl.h"
#include "wal_cfg_trx_ioctl.h"
#include "wal_cfg_feature_ioctl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_CFG_IOCTL_C
uint32_t wal_get_host_cmd_table_size(void);
host_cmd_stru *wal_get_cmd_info(uint32_t cmd_id);

/*
 * 功能描述  : 参数检查
 * 1.日    期  : 2020年6月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_params_check(uint32_t cmd_id, int32_t *params)
{
    host_cmd_stru *cmd_info = wal_get_cmd_info(cmd_id);
    uint32_t index;
    const int32_t *check_table = NULL;

    if (cmd_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 如果check_table为空说明参数不需要检查 */
    if (cmd_info->param_check_table == NULL) {
        return OAL_SUCC;
    }
    check_table = cmd_info->param_check_table;
    for (index = 0; index < cmd_info->param_num; index++) {
        /* 小于最小值, 大于最大值,为非法值 */
        if ((params[index] < check_table[index * PARAM_CHECK_TABLE_LEN + MIN_VALUE_INDEX]) ||
            (params[index] > check_table[index * PARAM_CHECK_TABLE_LEN + MAX_VALUE_INDEX])) {
            oam_error_log4(0, OAM_SF_ANY, "wal_params_check:check fail,cmd_id=%d, param=%d,min=%d,max=%d",
                cmd_id, params[index], check_table[index * PARAM_CHECK_TABLE_LEN + MIN_VALUE_INDEX],
                check_table[index * PARAM_CHECK_TABLE_LEN + MAX_VALUE_INDEX]);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : 获取下发命令的VAP
 * 1.日    期  : 2020年6月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_get_cmd_netdev(int8_t *cmd, oal_net_device_stru **netdev, uint32_t *offset)
{
    oal_net_device_stru *temp_netdev = NULL;
    int8_t dev_name[CMD_NAME_MAX_LEN] = { 0 };
    uint32_t ret;

    if (oal_any_null_ptr3(cmd, netdev, offset)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_get_cmd_netdev::cmd/netdev/pul_off_set is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = wal_get_cmd_one_arg(cmd, dev_name, CMD_NAME_MAX_LEN, offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_get_cmd_netdev::get cmd one arg err[%d]!}", ret);
        return ret;
    }
    /* 根据dev_name找到dev */
    temp_netdev = wal_config_get_netdev(dev_name, OAL_STRLEN(dev_name));
    if (temp_netdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_get_cmd_netdev::wal_config_get_netdev return null ptr!}\r\n");
        oal_io_print("wal_get_cmd_netdev::find netdev[%s] failed.\n", dev_name);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(temp_netdev);
    *netdev = temp_netdev;
    return OAL_SUCC;
}

/*
 * 功能描述  : 获取命令的ID号
 * 1.日    期  : 2020年6月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_get_cmd_id(int8_t *cmd, uint32_t *cmd_id, uint32_t *offset)
{
    int8_t cmd_name[CMD_NAME_MAX_LEN];
    uint32_t ret;
    uint32_t cmd_index;
    uint32_t host_cmd_table_size = wal_get_host_cmd_table_size();
    host_cmd_stru *cmd_info = NULL;
    ret = wal_get_cmd_one_arg(cmd, cmd_name, CMD_NAME_MAX_LEN, offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_get_cmd_id::get cmd one arg err[%d]!}", ret);
        return ret;
    }
    for (cmd_index = 0; cmd_index < host_cmd_table_size; cmd_index++) {
        cmd_info = wal_get_cmd_info(cmd_index);
        if (cmd_info == NULL) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_get_cmd_id::get cmd info fail!}");
            return OAL_FAIL;
        }
        if (oal_strcmp(cmd_info->name, cmd_name) == 0) {
            *cmd_id = cmd_index;
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}

/*
 * 功能描述  : 解析命令的参数信息
 * 1.日    期  : 2020年6月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_get_cmd_params(int8_t *cmd, uint32_t cmd_id, int32_t *params)
{
    uint32_t param_index;
    uint32_t offset = 0;
    uint32_t ret = 0;
    int8_t arg[CMD_VALUE_MAX_LEN] = {0};
    host_cmd_stru *cmd_info = wal_get_cmd_info(cmd_id);
    /* 获取参数 */
    for (param_index = 0; param_index < cmd_info->param_num; param_index++) {
        ret = wal_get_cmd_one_arg(cmd, arg, CMD_VALUE_MAX_LEN, &offset);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_process_cmd_params::get cmd one arg err[%d]!}", ret);
            return ret;
        }
        params[param_index] = oal_atoi(arg);
        cmd += offset;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 将需要抛到device的命令进行参数组装并抛事件
 * 1.日    期  : 2020年7月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_send_cmd_params_to_device(mac_vap_stru *mac_vap, host_cmd_stru *cmd_info, int32_t *params, uint32_t cmd_id)
{
    uint16_t new_params_len = 0;
    uint8_t  new_params[CMD_DEVICE_PARAMS_MAX_LEN] = {0};
    uint32_t ret;

    if (cmd_info->package_params == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "wal_send_cmd_params_to_device:cmd_id[%d],package_params func is null!", cmd_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 参数组装 */
    ret = cmd_info->package_params(mac_vap, params, new_params, &new_params_len);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "wal_send_cmd_params_to_device:cmd_id[%d],package_params fail!", cmd_id);
        return ret;
    }
    /* 抛事件 */
    ret = hmac_config_send_event(mac_vap, cmd_info->cfg_id, new_params_len, new_params);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
            "wal_send_cmd_params_to_device:cmd_id[%d],send event fail!", cmd_id);
    }
    return ret;
}

/*
 * 功能描述  : 处理命令参数
 * 1.日    期  : 2020年6月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_process_cmd_params(oal_net_device_stru *netdev, uint32_t cmd_id, int32_t *params)
{
    uint32_t ret;
    host_cmd_stru *cmd_info = NULL;
    mac_vap_stru *mac_vap = NULL;

    mac_vap = oal_net_dev_priv(netdev);
    if (mac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 检查参数有效性 */
    ret = wal_params_check(cmd_id, params);
    if (ret != OAL_SUCC) {
        return ret;
    }
    cmd_info = wal_get_cmd_info(cmd_id);
    if (cmd_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* host命令处理 */
    if (cmd_info->process_cmd != NULL) {
        ret = cmd_info->process_cmd(mac_vap, params);
        if (ret != OAL_SUCC) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "wal_process_cmd:cmd_id[%d],cmd process fail!", cmd_id);
            return ret;
        }
    }
    /* 判断是否抛事件到device */
    if (cmd_info->to_device == OAL_FALSE) {
        return OAL_SUCC;
    }
    /* 抛事件 */
    ret = wal_send_cmd_params_to_device(mac_vap, cmd_info, params, cmd_id);
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "wal_process_cmd:cmd_id[%d],send event fail!", cmd_id);
        return ret;
    }
    return ret;
}
/*
 * 功能描述  : 处理命令
 * 1.日    期  : 2020年6月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_process_cmd(int8_t *cmd)
{
    uint32_t offset = 0;
    oal_net_device_stru *netdev = NULL;
    uint32_t ret;
    uint32_t cmd_id;
    int32_t  params[CMD_PARAMS_MAX_CNT] = {0};

    /* 查找mac_vap */
    ret = wal_get_cmd_netdev(cmd, &netdev, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_process_cmd:find netdev fail");
        return ret;
    }
    cmd += offset;
    /* 查找命令ID */
    ret = wal_get_cmd_id(cmd, &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    cmd += offset;
    /* 处理参数 */
    ret = wal_get_cmd_params(cmd, cmd_id, params);
    if (ret != OAL_SUCC) {
        return ret;
    }
    return wal_process_cmd_params(netdev, cmd_id, params);
}

/* 参数检查列表 */
const int32_t g_switch_check_table[] = {
    /* 开关的取值范围为0|1 */
    0, 1 };
const int32_t g_support_2040_coext_check_table[] = {
    /* 0表示20/40MHz共存使能，1表示20/40MHz共存不使能 */
    0, 1 };
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
const int32_t g_pk_mode_check_table[] = {
    /* 高门限/低门限 */
    0, 3,
    /* BW */
    0, WLAN_BW_CAP_BUTT - 1,
    /* protocol */
    0, WLAN_PROTOCOL_CAP_BUTT - 1,
    /* 门限值 */
    MIN_INT32, MAX_INT32 };
#endif
const int32_t g_remove_app_ie_check_table[] = {
    /* remove type 为1表示移除该IE，0为恢复IE */
    0, 1,
    MIN_INT32, MAX_INT32 };
const int32_t g_pm_switch_check_table[] = {
    /* pm_switch_cfg枚举范围 0 -- 5 */
    0, 5 };
const int32_t g_beacon_chan_switch_check_table[] = {
    /* 0为普通单通道发送，1为开启双通道轮流发送能力,2为开启双通道发送能力 */
    0, 2 };
const int32_t g_btcoex_preempt_type_check_table[] = {
    0, 6,
    MIN_INT32, MAX_INT32 };
#ifdef _PRE_WLAN_FEATURE_WMMAC
const int32_t g_addts_req_check_table[] = {
    0, 7,                   /* tid，取值范围0~7 */
    MIN_INT32, MAX_INT32,   /* direction 00:uplink 01:downlink 10:reserved 11:Bi-directional */
    MIN_INT32, MAX_INT32,   /* PSB，1表示U-APSD，0表示legacy */
    MIN_INT32, MAX_INT32,   /* UP */
    MIN_INT32, MAX_INT32,   /* Nominal MSDU Size ,第一位为1 */
    MIN_INT32, MAX_INT32,   /* maximum MSDU size */
    MIN_INT32, MAX_INT32,   /* minimum data rate */
    MIN_INT32, MAX_INT32,   /* mean data rate */
    MIN_INT32, MAX_INT32,   /* peak data rate */
    MIN_INT32, MAX_INT32,   /* minimum PHY Rate */
    MIN_INT32, MAX_INT32,   /* surplus bandwidth allowance */
    };
const int32_t g_delts_check_table[] = {
    0, WLAN_TID_MAX_NUM - 1 };
#endif
const int32_t g_mem_pool_id_check_table[] = {
    0, OAL_MEM_POOL_ID_BUTT - 1 };
const int32_t g_user_id_check_table[] = {
    /* user_id的取值范围 */
    0, WLAN_ASSOC_USER_MAX_NUM - 1};

const int32_t g_frag_threshold_check_table[] = {
    WLAN_FRAG_THRESHOLD_MIN, WLAN_FRAG_THRESHOLD_MAX };

#ifdef _PRE_WLAN_FEATURE_PHY_EVENT_INFO
const int32_t g_phy_event_rpt_check_table[] = {
    /* phy event report en 0|1 */
    0, 1,
    /* phy event mem num */
    MIN_INT32, MAX_INT32,
    /* phy event0 select  */
    MIN_INT32, MAX_INT32,
    /* phy event1 select */
    MIN_INT32, MAX_INT32,
};
#endif
const int32_t g_txbf_cap_check_table[] = {
    /* txbf_cap value is 0 | 1 | 2 | 3
     * bit0表示RX(bfee)能力 bit1表示TX(bfer)能力 */
    0, 3 };
const int32_t g_icmp_filter_check_table[] = {
    1, 1,   /* filter_id must be 1 */
    0, 1 }; /* 0 :关闭1: 打开 */

/* SAE_PWE 参数范围(0~3) */
const int32_t g_sae_pwe_check_table[] = {
    /* SAE_PWE 枚举：
     * 0 : SAE_PWE_UNSPECIFIED
     * 1 : SAE_PWE_HUNT_AND_PECK
     * 2 : SAE_PWE_HASH_TO_ELEMENT
     * 3 : SAE_PWE_BOTH
     */
    0, 3,
    };

/* CHBA 信道序列参数检查表 */
const int32_t g_chba_channel_seq_check_tabel[] = {
    0, 3, /* CHBA信道序列等级 */
    36, 165, /* CHBA第一工作信道36~165信道 */
    36, 165, /* CHBA第二工作信道36~165信道 */
    };

host_cmd_stru g_host_cmd_table[] = {
    /* ////////////////////////////   商用命令   //////////////////////////// */
    /* sh hipriv.sh "wlan0 uapsd_en_cap 0|1" */
    { "uapsd_en_cap", 1, g_switch_check_table, wal_set_uapsd_cap,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_UAPSD_EN },
    /* 设置漫游开关 */
    { "roam_enable", 1, NULL, wal_set_roam_enable, NULL, OAL_FALSE, 0 },
    /* icmp过滤命令: sh hipriv.sh "wlan0 icmp_filter 1 0|1" */
    { "icmp_filter", 2, g_icmp_filter_check_table, NULL, hmac_package_icmp_filter_params,
        OAL_TRUE, WLAN_CFGID_ASSIGNED_FILTER},
    /* 设置chip test中强制使能pmf能力 (用于关联之后)
       sh hipriv.sh "vap0 set_default_key x(key_index) 0|1(en_unicast) 0|1(multicast)"  */
    { "set_default_key", 3, NULL, wal_set_default_key,
        hmac_package_default_key_params, OAL_TRUE, WLAN_CFGID_DEFAULT_KEY},
    /* fem低功耗命令: sh hipriv.sh "wlan0 fem_lowpower 0|1 |2" */
    { "fem_lowpower", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SPEC_FEMLP_EN },
    { "set_fast_sleep_para", 4, NULL, NULL, hmac_package_fasts_sleep_params, OAL_TRUE, WLAN_CFGID_SET_FASTSLEEP_PARA },
    /* 设置PSPOLL能力 sh hipriv.sh 'wlan0 set_ps_mode 3' */
    { "set_ps_mode", 1, NULL, wal_set_sta_ps_mode, hmac_package_sta_ps_mode_params, OAL_TRUE, WLAN_CFGID_SET_PS_MODE },
    { "p2p_scenes", 1, NULL, wal_set_p2p_scenes, hmac_package_uint8_params, OAL_TRUE, WALN_CFGID_SET_P2P_SCENES },
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    /* 配置NR共存参数，sh hipriv.sh "wlan0 nrcoex_set_prior 0/1/2/3" */
    { "nrcoex_set_prior", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_NRCOEX_PRIORITY_SET },
#endif
    /* 配置使能dbac信道跟随开关，需要针对对应网络设备 */
    { "dbac_enable_follow_channel", 1, g_switch_check_table, hmac_dbac_enable_follow_channel,
        hmac_package_uint8_params, OAL_FALSE, 0 },

    /* ////////////////////////////   非商用命令   //////////////////////////// */
#ifdef WIFI_DEBUG_ENABLE
    { "bgscan_enable", 1, NULL, wal_set_bgscan_type, NULL, OAL_FALSE, 0 },
    { "arp_offload_enable", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_ENABLE_ARP_OFFLOAD },
    /* 设置随机mac addr扫描开关，sh hipriv.sh "Hisilicon0 random_mac_addr_scan 0|1(打开|关闭)" */
    { "random_mac_addr_scan", 1, g_switch_check_table, wal_set_random_mac_addr_scan, NULL, OAL_FALSE, 0 },
    /* 设置20/40共存使能: sh hipriv.sh "wlan0 2040_coexistence 0|1" 0表示20/40MHz共存使能，1表示20/40MHz共存不使能 */
    { "2040_coexistence", 1, g_support_2040_coext_check_table, wal_set_2040_coext_support, NULL, OAL_FALSE, 0 },
    /* log低功耗模式: hipriv "Hisilicon0 pm_switch 0 | 1"，log pm模式开关 */
    { "pm_switch", 1, g_pm_switch_check_table, wal_set_pm_switch,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_PM_SWITCH},
    /* sta psm的统计信息 sh hipriv.sh 'wlan0 psm_info_debug 1' */
    { "psm_info_debug", 2, NULL, NULL,
        hmac_package_sta_ps_info_params, OAL_TRUE, WLAN_CFGID_SHOW_PS_INFO },
    /* 设置uapsd的参数信息 sh hipriv.sh 'wlan0 set_uapsd_para 3 1 1 1 1 */
    { "set_uapsd_para", 5, NULL, wal_set_uapsd_para, hmac_package_uapsd_params, OAL_TRUE, WLAN_CFGID_SET_UAPSD_PARA },
    /* log低功耗模式: hipriv "Hisilicon0 power_test 0 | 1"，功耗测试模式开 */
    { "power_test", 1, g_switch_check_table, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_POWER_TEST},
    /* sh hipriv.sh 'wlan0 set_psm_para 100 40 */
    { "set_psm_para", 4, NULL, NULL, hmac_package_sta_psm_params, OAL_TRUE, WLAN_CFGID_SET_PSM_PARAM },
    /* 设置固定LP模式: sh hipriv.sh 'wlan0 lp_fixed_mode xx(0:LP关; 1:LP开) */
    { "lp_enable", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_LP_ENABLE },
    /* 设置LP常收模式: sh hipriv.sh 'wlan0 lp_rx_mode xx(bit0:开关 bit1-2:固定模式值(0:normal; 1:listen; 2:beacon)) */
    { "lp_rx_mode", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_LP_AL_RX_MODE },
    /* 设置常发数目:      sh hipriv.sh "vap0 al_tx_num <value>" */
    { "al_tx_num", 1, NULL, NULL, hmac_package_uint32_params, OAL_TRUE, WLAN_CFGID_SET_ALWAYS_TX_NUM },
    /* 数据常发聚合数目配置 */
    { "al_tx_aggr_num", 1, NULL, NULL, hmac_package_uint32_params, OAL_TRUE, WLAN_CFGID_SET_ALWAYS_TX_AGGR_NUM },
    /* 设置AMSDU聚合个数: sh hipriv.sh "wlan0 amsdu_aggr_num num" */
    { "amsdu_aggr_num", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_AMSDU_AGGR_NUM },
    /* 设置STBC能力 STBC设置开关的命令: hipriv "vap0 set_stbc_cap 0 | 1" */
    { "set_stbc_cap", 1, g_switch_check_table, wal_set_stbc_cap,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_STBC_CAP },
    /* 设置LDPC能力 LDPC设置开关的命令: hipriv "vap0 set_ldpc_cap 0 | 1 */
    { "set_ldpc_cap", 1, g_switch_check_table, wal_set_ldpc_cap,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_LDPC_CAP },
    /* 开启或关闭txbf的 接收bit0/发送bit1 能力 hipriv "vap0 alg_txbf_switch 0|1|2|3" */
    { "set_txbf_cap", 1, g_txbf_cap_check_table, wal_set_txbf_cap,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_TXBF_SWITCH },
    { "rx_mode_switch", 1, NULL, NULL,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_RX_MODE_SWITCH_DEBUG},
    /*
     * 切换某个user tid的发送方式 hipriv.sh "wlan0 tx_state_switch mode"
     * - mode: 0: host ring tx; 1: device ring tx; 5: 相关状态打印
     */
    { "tx_mode_switch", 1, NULL, hmac_tx_mode_switch, NULL, OAL_FALSE, 0 },
    /* 开启或关闭ampdu发送功能 hipriv "vap0 amsdu_tx_on 0\1" */
    { "amsdu_tx_on", 1, NULL, wal_set_amsdu_tx_on, NULL, OAL_FALSE, 0 },
    /* 设置自动保护开关 */
    { "auto_protection", 1, NULL, NULL, hmac_package_uint32_params, OAL_TRUE, WLAN_CFGID_SET_AUTO_PROTECTION },
    /* 设置分片门限的配置命令: hipriv "vap0 frag_threshold (len)" 该命令针对某一个VAP */
    { "frag_threshold", 1, g_frag_threshold_check_table, wal_set_frag_threshold, NULL, OAL_FALSE, 0},
    /* 设置流控相关参数 sh hipriv.sh "Hisilicon0 set_flowctl_param 0/1/2/3 20 20 40"  0/1/2/3 分别代表be,bk,vi,vo */
    { "set_flowctl_param", 4, NULL, wal_set_flowctl_param, NULL, OAL_FALSE, 0 },
    { "vmin_abnormal_debug", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_VMIN_ABNORMAL_DEBUG },
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    /* pkmode功能的门限调整接口 hipriv "wlan0 pk_mode_debug 0/1(high/low) 0/1/2/3/4(BW) 0/1/2/3(protocol) 吞吐门限" */
    { "pk_mode_debug", 4, g_pk_mode_check_table, wal_set_pk_mode_debug, NULL, OAL_FALSE, 0 },
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    /* sh hipriv.sh "wlan0 tid_queue_size 0|1" */
    { "tid_queue_size", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_REPORT_TID_QUEUE_SIZE},
#endif
#ifdef _PRE_WLAN_FEATURE_11K
    /* sh hipriv.sh "wlan0 beacon_req_table_switch 0/1" */
    { "beacon_req_table_switch", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_BCN_TABLE_SWITCH },
#endif
    /* VOE功能使能控制，默认关闭 sh hipriv.sh "wlan0 voe_enable 0/1"
    (Bit0:11r  Bit1:11V Bit2:11K Bit3:是否强制包含IE70(voe 认证需要),
     Bit4:11r认证,B5-B6:11k auth operating class  Bit7:读取信息) */
    { "voe_enable", 1, NULL, wal_set_voe_enable, hmac_package_uint16_params, OAL_TRUE, WLAN_CFGID_VOE_ENABLE },
    /* 通过eid移除用户态下发的某个IE sh hipriv.sh "wlan0 remove_app_ie 0/1 eid" 0恢复该ie,1屏蔽该ie */
    { "remove_app_ie", 2, g_remove_app_ie_check_table, wal_set_remove_app_ie, NULL, OAL_FALSE, 0 },
    /* 全局日志开关:  hipriv "Hisilicon0 global_log_switch 0 | 1 */
    { "global_log_switch", 1, g_switch_check_table, wal_set_global_log_switch, NULL, OAL_FALSE, 0},
    /* 设置是否上报beacon帧开关: hipriv "Hisilicon0 ota_beacon_switch 0 | 1"，该命令针对所有的VAP */
    { "ota_beacon_on", 1, NULL, wal_set_ota_beacon_switch,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_OTA_BEACON_SWITCH},
    /* 设置beacon帧发送策略配置命令: hipriv "vap0 beacon_chain_switch 0/1"
       目前采取的是单通道模式(使用通道0)，0表示关闭双路轮流发送，1表示开启，该命令针对某一个VAP */
    { "beacon_chain_switch", 1, g_beacon_chan_switch_check_table, NULL,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_BEACON_CHAIN_SWITCH},
    /* 配置preempt_type，sh hipriv.sh "vap0 coex_preempt_type  0/1(硬件或者软件) 0-3/0-1"
       0 noframe 1 self-cts 2 nulldata 3 qosnull  软件ps打开或者关闭 2 slot提前量 */
    { "coex_preempt_type", 2, g_btcoex_preempt_type_check_table, NULL,
        hmac_package_btcoex_preempt_type_params, OAL_TRUE, WLAN_CFGID_BTCOEX_PREEMPT_TYPE},
#ifdef _PRE_WLAN_FEATURE_WMMAC
    /* 设置建立TS，即发送ADDTS REQ的配置命令:
       hipriv "vap0 addts_req tid direction apsd up nominal_msdu_size max_msdu_size minimum_data_rate
       mean_data_rate peak_data_rate minimum_phy_rate surplus_bandwidth_allowance" 该命令针对某一个VAP */
    { "addts_req", 11, g_addts_req_check_table, wal_set_addts_req, NULL, OAL_FALSE, 0},
    /* 设置删除TS，即发送DELTS的配置命令: hipriv "vap0 tidno" 该命令针对某一个VAP */
    { "delts", 1, g_delts_check_table, wal_set_delts, NULL, OAL_FALSE, 0},
    /* 发送重关联请求帧: hipriv "vap0 reassoc_req" 注：WFA 认证测试使用，勿删 */
    { "reassoc_req", 0, NULL, wal_config_reassoc_req, NULL, OAL_FALSE, 0 },
#endif
    /* 打印内存池信息: hipriv "Hisilicon0 meminfo poolid" */
    { "meminfo", 1, g_mem_pool_id_check_table, wal_set_mem_info, NULL, OAL_FALSE, 0},
    /* 检查指定内存池泄漏内存块: hipriv "Hisilicon0 memleak poolid" */
    { "memleak", 1, g_mem_pool_id_check_table, wal_set_mem_leak, NULL, OAL_FALSE, 0 },
    { "ota_set_switch", 2, NULL, wal_set_ota_switch, hmac_package_ota_switch_params, OAL_TRUE, WLAN_CFGID_OTA_SWITCH},
    /* 获取所有维测统计信息: hipriv "Hisilicon0 wifi_stat_info" */
    { "wifi_stat_info", 0, NULL, wal_set_show_stat_info, NULL, OAL_FALSE, 0 },
    /* 清零所有维测统计信息: hipriv "Hisilicon0 clear_stat_info" */
    { "clear_stat_info", 0, NULL, wal_set_clear_stat_info, NULL, OAL_FALSE, 0 },
    /* 上报某个user下的维测统计信息: sh hipriv.sh "Hisilicon0 usr_stat_info usr_id" */
    { "usr_stat_info", 1, g_user_id_check_table, wal_set_user_stat_info, NULL, OAL_FALSE, 0 },
    /* 支持信道列表， hipriv "Hisilicon0 list_channel" */
    { "list_channel", 0, NULL, wal_set_list_channel, NULL, OAL_FALSE, 0 },
    /* 打印事件队列信息，将打印出每一个非空事件队列中事件的个数，以及每一个事件头信息, hipriv "Hisilicon0 event_queue" */
    { "event_queue", 0, NULL, wal_set_event_queue_info, NULL, OAL_FALSE, 0 },
#ifdef _PRE_WLAN_DFT_STAT
    /* 清零指定VAP的统计信息: hipriv "vap_name clear_vap_stat_info" */
    { "clear_vap_stat_info", 0, NULL, wal_set_clear_vap_stat_info, NULL, OAL_FALSE, 0 },
    /* 上报或者停止上报vap吞吐统计信息: sh hipriv.sh "vap_name vap _stat  0|1" */
    { "vap_stat", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_VAP_STAT },
#endif
    /* 获取高优先级报文的统计情况 */
    { "get_hipkt_stat", 0, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_GET_HIPKT_STAT },
    /* 获取流控相关状态信息 */
    { "get_flowctl_stat", 0, NULL, wal_get_flowctl_stat, NULL, OAL_FALSE, 0 },
    /* 打印AP关联的STA列表: hipriv "sta0 list_sta" */
    { "list_sta", 0, NULL, wal_set_list_sta, NULL, OAL_FALSE, 0 },
    /* 触发sta去认证: hipriv "vap0 start_deauth" */
    { "start_deauth", 0, NULL, wal_start_deauth, NULL, OAL_FALSE, 0 },
    /* 设置所有用户的以太网开关，sh hipriv.sh "Hisilicon0 ether_all 0|1(帧方向tx|rx) 0|1(开关)" */
    { "ether_all", 2, NULL, wal_set_all_ether_switch, NULL, OAL_FALSE, 0 },
    /* 设置发送广播arp和dhcp开关，sh hipriv.sh "Hisilicon0 dhcp_arp_switch 0|1(开关)" */
    { "dhcp_arp_switch", 1, NULL, wal_set_dhcp_arp_switch, NULL, OAL_FALSE, 0 },
    /* 设置80211组播\广播帧上报的开关，sh hipriv.sh "Hisilicon0 80211_mc_switch
       0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧) 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)" */
    { "80211_mc_switch", 5, NULL, wal_set_80211_mcast_switch,
        hmac_package_80211_mcast_params, OAL_TRUE, WLAN_CFGID_80211_MCAST_SWITCH},
    /* 设置probe req与rsp上报的开关，sh hipriv.sh "Hisilicon0 probe_switch
       0|1(帧方向tx|rx) 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)" */
    { "probe_switch", 4, NULL, wal_set_probe_switch,
        hmac_package_probe_switch_params, OAL_TRUE, WLAN_CFGID_PROBE_SWITCH },
    /* 设置所有ota上报，如果为1，则所有类型帧的cb描述符都报，如果为0，什么都不报，
       sh hipriv.sh "Hisilicon0 set_all_ota 0|1" */
    { "set_all_ota", 1, NULL, wal_set_all_ota, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_ALL_OTA},
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* 设置mac txop ps使能寄存器，sh hipriv.sh "stavap_name txopps_hw_en
       0|1(txop_ps_en) 0|1(condition1) 0|1(condition2)" */
    { "txopps_hw_en", 3, NULL, wal_set_txop_ps_machw, hmac_package_txop_ps_params, OAL_TRUE, WLAN_CFGID_TXOP_PS_MACHW},
#endif
#ifdef _PRE_WLAN_FEATURE_PHY_EVENT_INFO
    /* 设置PHY事件上使能:hipriv "wlan0/p2p0 phy_event 0|1(0不使能，1使能) 1(mem个数) 1(event0事件的开关使能(32bits)
        1(event1事件的开关使能(32bits), 0~31位代表32个事件开关)" */
    { "phy_event", 4, g_phy_event_rpt_check_table, wal_hipriv_phy_event_rpt, hmac_phy_event_rpt_params,
        OAL_TRUE, WLAN_CFGID_PHY_EVENT_RPT},
#endif
#ifdef _PRE_WLAN_FEATURE_PMF
    /* 设置chip test中强制使能pmf能力 (用于关联之后) sh hipriv.sh "vap0 enable_pmf 0|1|2(0不使能，1 enable, 2强制)  */
    { "enable_pmf", 1, NULL, wal_set_enable_pmf, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_PMF_ENABLE },
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
    /* dfr功能打桩触发接口 sh hipriv.sh "vap0 dfr_start 0(dfr子功能:0-device异常复位 )" */
    { "dfr_start", 0, NULL, wal_test_dfr_start, NULL, OAL_FALSE, 0 },
#endif
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    /* 配置nrcoex的测试命令当前只有06使用 hipriv.sh "wlan0 nrcoex_msg value" */
    { "nrcoex_msg", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_NRCOEX_TEST },
#endif
#ifdef _PRE_WLAN_FEATURE_UWBCOEX
    /* 配置uwbcoex的测试命令当前只有bisheng使用 hipriv.sh "wlan0 uwbcoex_msg value" */
    { "uwbcoex_msg", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_UWBCOEX_TEST },
#endif
    /* 设置ap模式的vap带宽固定 */
    { "bw_fixed", 1, g_switch_check_table, wal_set_bw_fixed,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_BW_FIXED },
    /* 发送20/40共存管理帧: hipriv "Hisilicon0 send_2040_coext coext_info chan_report" */
    { "send_2040_coext", 2, NULL, wal_send_2040_coext, NULL, OAL_FALSE, 0 },
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
    /* 使能PSD采集: hipriv "Hisilicon0 set_psd 0/1"，使能之前先config  */
    { "set_psd", 1, g_switch_check_table, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_PSD },
    /* 使能PSD采集: hipriv "Hisilicon0 cfg_psd" */
    { "cfg_psd", 0, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_CFG_PSD },
#endif
    /* wal_config_blacklist_show */
    { "blacklist_show", 0, NULL, wal_set_blacklist_show, NULL, OAL_FALSE, 0 },
#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
    /* 打印RR各点的时间戳，sh hipriv.sh "Hisilicon0 show_rr_timeinfo 0|1" */
    { "show_rr_timeinfo", 1, g_switch_check_table, wal_set_show_rr_time_info,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_ENABLE_RR_TIMEINFO },
#endif
    /* 显示Device侧记录的:sh hipriv.sh "wlan0 show_ip_addr 1 1" */
    { "show_arpoffload_info", 2, NULL, NULL,
        hmac_package_show_arpoffload_info_params, OAL_TRUE, WLAN_CFGID_SHOW_ARPOFFLOAD_INFO },
#ifdef _PRE_WLAN_TCP_OPT
    /* 显示TCP ACK 过滤统计值 sh hipriv.sh "vap0 get_tx_ack_stream_info */
    { "get_tcp_ack_stream_info", 0, NULL, wal_get_tcp_ack_stream_info, NULL, OAL_FALSE, 0 },
    /* 设置发送TCP ACK优化使能  sh hipriv.sh "vap0 tcp_tx_ack_opt_enable 0 | 1 */
    { "tcp_tx_ack_opt_enable", 1, NULL, wal_set_tcp_tx_ack_opt_enable, NULL, OAL_FALSE, 0 },
    /* 设置接收TCP ACK优化使能 sh hipriv.sh "vap0 tcp_rx_ack_opt_enable 0 | 1 */
    { "tcp_rx_ack_opt_enable", 1, NULL, wal_set_tcp_rx_ack_opt_enable, NULL, OAL_FALSE, 0 },
    /* 设置发送TCP ACK LIMIT sh hipriv.sh "vap0 tcp_tx_ack_opt_limit X */
    { "tcp_tx_ack_opt_limit", 1, NULL, wal_set_tcp_tx_ack_limit, NULL, OAL_FALSE, 0 },
    /* 设置接收TCP ACKLIMIT  sh hipriv.sh "vap0 tcp_tx_ack_opt_limit X */
    { "tcp_rx_ack_opt_limit", 1, NULL, wal_set_tcp_rx_ack_limit, NULL, OAL_FALSE, 0 },
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* 打印定制化信息 */
    { "custom_info", 1, g_switch_check_table, wal_set_dev_customize_info,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS },
#endif
#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* 11v bss transition特性配置 sh hipriv.sh "vap0 11v_cfg_bsst 0|1" 0:关闭11V特性；1:打开11V特性  */
    { "11v_cfg_bsst", 1, g_switch_check_table, wal_set_11v_cfg_bsst,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_11V_BSST_SWITCH },
#endif
    /* 设置雷达事件sh hipriv.sh "wlan0 radar_set" */
    { "radar_set", 0, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_SET_RADAR },
    /* 触发sta加入并认证关联: hipriv "sta0 start_join 1" 1表示扫描到的AP在device写数组下标号 */
    { "start_join", 1, NULL, wal_set_start_join, NULL, OAL_FALSE, 0 },
    /* 根据标记位上报vap的对应信息 sh hipriv.sh "wlan0 report_vap_info 1" */
    { "report_vap_info", 1, NULL, NULL, hmac_package_uint32_params, OAL_TRUE, WLAN_CFGID_REPORT_VAP_INFO },
    /* 设置另一路启动雷达CAC sh hipriv.sh "wlan0 radar_cac 36 7" */
    { "radar_cac", 2, NULL, NULL, hmac_package_cac_params, OAL_TRUE, WLAN_CFGID_START_ANOTHER_DEV_CAC },
#ifdef _PRE_WLAN_FEATURE_FTM
    /* 配置ftm_set_delay的测试命令当前只有06使用 hipriv.sh "wlan0 ftm_set_delay value" */
    { "ftm_set_delay", 1, NULL, NULL, hmac_package_ftm_params, OAL_TRUE, WLAN_CFGID_FTM_DBG },
#endif

#ifdef _PRE_WLAN_FEATURE_GNSS_RSMC
    { "gnss_rsmc_status", 1, NULL, hmac_process_gnss_rsmc_status_cmd, NULL, OAL_FALSE, 0 },
#endif
#if defined (_PRE_WLAN_FEATURE_WAPI) && defined (_PRE_WAPI_DEBUG)
    /* 是否使能硬件加解密WAPI帧 sh hipriv.sh "wlan0 hw_wapi 0|1" */
    { "hw_wapi", 1, g_switch_check_table, hmac_wapi_set_debug_hw_wapi, hmac_package_uint8_params, OAL_TRUE,
        WLAN_CFGID_HW_WAPI },
#endif
    { "set_eqmode", 1, NULL, NULL, hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_EQUIPMENT_MODE },
    /* 设置暗屏下动态dtim周期: sh hipriv.sh 'wlan0 auto_dtim_period_cfg xx(倍数) xx(与tbtt时间) */
    { "auto_dtim_period_cfg", 2, NULL, NULL, hmac_package_dtim_params, OAL_TRUE, WLAN_CFGID_SET_DTIM_PERIOD },
    /* 配置GO NOA参数: sh hipriv.sh 'p2p-p2p0-0 p2p_ps_noa start_time duration interval count */
    { "p2p_ps_noa", 4, NULL, NULL, hmac_package_p2p_noa_params, OAL_TRUE, WLAN_CFGID_SET_P2P_PS_NOA },
    /* 配置GO OPS参数: sh hipriv.sh 'p2p-p2p0-0 p2p_ps_ops 0/1(0不使能，1使能) [0~255] 设置OPS 节能下ct_window 参数 */
    { "p2p_ps_ops", 2, NULL, NULL, hmac_package_p2p_ops_params, OAL_TRUE, WLAN_CFGID_SET_P2P_PS_OPS },
    /* 配置WPA3 sae_pwe参数：sh hipriv.sh 'wlan0 set_sae_pwe 0/1/2/3()  */
    { "set_sae_pwe", 1, g_sae_pwe_check_table, hmac_vap_set_sae_pwe, hmac_package_uint8_params, OAL_TRUE,
        WLAN_CFGID_SET_SAE_PWE },
    /* 配置低功耗心跳代理日志打印开关：sh hipriv.sh 'wlan0 set_lp_agent_log 0/1 */
    { "set_lp_agent_log", 1, g_switch_check_table, NULL, hmac_package_uint8_params, OAL_TRUE,
        WLAN_CFGID_LP_AGENT_LOG_ENABLE },
#ifdef _PRE_WLAN_CHBA_MGMT
    /* sh hipriv.sh "chba0 set_chba_channel_seq channel_seq_level first_work_channel second_work_channel"
     * channel_seq_level:[0/1/2/3]
     * work_channel:[36/149等]
     */
    { "set_chba_channel_seq", 3, g_chba_channel_seq_check_tabel, hmac_chba_set_channel_seq_params,
        hmac_package_chba_channel_seq_params, OAL_TRUE, WLAN_CFGID_CHBA_SET_CHANNEL_SEQ_PARAMS },
    /* sh hipriv.sh "chba0 chba_dbac_channel_follow 0/1"
     * 是否支持STA+CHBA DBAC状态，CHBA工作在STA信道。0:不支持；1：支持
     */
    { "chba_dbac_channel_follow", 1, g_switch_check_table, NULL,
        hmac_package_uint8_params, OAL_TRUE, WLAN_CFGID_CHBA_DBAC_CHANNEL_FOLLOW },
#endif /* _PRE_WLAN_CHBA_MGMT */
    /* 打印校准数据信息 */
    { "cali_data_info", 1, NULL, wal_show_cali_data_info, NULL, OAL_FALSE, 0 },
#ifdef _PRE_WLAN_FEATURE_VSP
    { "vsp_log", 1, NULL, hmac_vsp_set_log_lvl, NULL, OAL_FALSE, 0 },
#endif
    { "mintp_log", 1, NULL, hmac_mintp_set_log_lvl, NULL, OAL_FALSE, 0 },
    /* 设置开wifi是否校准 */
    { "set_close_fem_cali", 1, NULL, wal_set_close_fem_cali_status, NULL, OAL_FALSE, 0 },
#endif
};

uint32_t wal_get_host_cmd_table_size(void)
{
    return oal_array_size(g_host_cmd_table);
}
host_cmd_stru *wal_get_cmd_info(uint32_t cmd_id)
{
    if (cmd_id >= oal_array_size(g_host_cmd_table)) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_get_cmd_info:cmd_id=%d > %d is error",
            cmd_id, oal_array_size(g_host_cmd_table));
        return NULL;
    }
    return &g_host_cmd_table[cmd_id];
}
