/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 商用命令,放到该文件
 * 创建日期 : 2021年11月19日
 */
#include "oal_types.h"
#include "mac_vap.h"
#include "hmac_roam_main.h"
#include "wal_config.h"
#include "hmac_vap.h"
#include "mac_mib.h"
#include "hmac_scan.h"
#include "oal_util.h"
#include "plat_pm_wlan.h"
#include "mac_device.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_hid2d.h"
#include "hmac_resource.h"
#include "hmac_tx_switch.h"
#include "wlan_chip_i.h"
#include "wal_linux_ioctl.h"
#include "hmac_pm.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_RELEASE_CFG_IOCTL_C


/* 注意! 这里的参数定义需要与 g_dmac_config_set_dscr_param中的函数顺序严格一致! */
const int8_t *g_pauc_tx_dscr_param_name[WAL_DSCR_PARAM_BUTT] = {
    "txtime",
    "datatype",
    "ta",
    "ra",
    "cc",
    "power",
    "shortgi",
    "preamble",
    "rtscts",
    "lsigtxop",
    "smooth",
    "snding",
    "txbf",
    "stbc",
    "rd_ess",
    "dyn_bw",
    "dyn_bw_exist",
    "ch_bw_exist",
    "rate",
    "mcs",
    "mcsac",
    "mcsax",
    "mcsax_er",
    "nss",
    "bw",
    "ltf",
    "gi",
    "txchain",
    "dcm",
    "protocol_mode"
};

/*
 * 功能描述  : 设置UAPSD使能
 * 1.日    期  : 2020年7月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_uapsd_cap(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t en_uapsd = (uint8_t)params[0];
    /* 设置mib值 */
    mac_vap_set_uapsd_en(mac_vap, en_uapsd);
    g_uc_uapsd_cap = en_uapsd;
    return OAL_SUCC;
}
uint32_t wal_set_roam_enable(mac_vap_stru *mac_vap, uint32_t *params)
{
    hmac_vap_stru *hmac_vap;
    uint8_t roam_value = (params[0] == 0) ? 0 : 1;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_set_roam_enable::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return hmac_roam_enable(hmac_vap, roam_value);
}

uint32_t wal_set_default_key(mac_vap_stru *mac_vap, uint32_t *params)
{
    uint8_t idx = 0;
    mac_setdefaultkey_param_stru payload_setdefaultkey_params = { 0 };
    /* sh hipriv.sh "vap0 set_default_key x(key_index) 0|1(en_unicast) 0|1(multicast)" */
    /* 获取参数 */
    payload_setdefaultkey_params.uc_key_index = params[idx++];  /* 获取key_index */
    payload_setdefaultkey_params.en_unicast = params[idx++];    /* 获取en_unicast */
    payload_setdefaultkey_params.en_multicast = params[idx++];  /* 获取multicast */

    return hmac_config_11i_set_default_key(mac_vap, sizeof(mac_setdefaultkey_param_stru),
        (uint8_t *)&payload_setdefaultkey_params);
}

/*
 * 功能描述  : 配置p2p 业务场景，0表示低时延，1表示高吞吐
 * 1.日    期  : 2020年7月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_p2p_scenes(mac_vap_stru *mac_vap, uint32_t *params)
{
    mac_device_stru *mac_device = NULL;

    mac_device = mac_res_get_dev(mac_vap->uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "wal_set_p2p_scenes:: mac_device is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_device->st_p2p_info.p2p_scenes = (uint8_t)params[0];
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "wal_set_p2p_scenes:: set p2p scene[%d].",
        mac_device->st_p2p_info.p2p_scenes);
    return OAL_SUCC;
}

uint32_t wal_set_sta_ps_mode(mac_vap_stru *mac_vap, uint32_t *params)
{
    hmac_vap_stru *hmac_vap = NULL;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap->uc_ps_mode = (uint8_t)params[0];
    oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_PWR,
                     "wal_set_sta_ps_mode ps_mode[%d]",
                     hmac_vap->uc_ps_mode);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if ((hmac_vap->uc_ps_mode == MAX_FAST_PS) || (hmac_vap->uc_ps_mode == AUTO_FAST_PS)) {
        wlan_pm_set_timeout((g_wlan_min_fast_ps_idle > 1) ? (g_wlan_min_fast_ps_idle - 1) : g_wlan_min_fast_ps_idle);
    } else {
#endif

        wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    }
#endif
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_get_dscr_param_type(int8_t *name, uint8_t *map_idx)
{
    uint8_t idx;

    /* 解析是设置哪一个字段 */
    for (idx = 0; idx < WAL_DSCR_PARAM_BUTT; idx++) {
        if (oal_strcmp(g_pauc_tx_dscr_param_name[idx], name) == 0) {
            *map_idx = idx;
            return OAL_SUCC;
        }
    }

    /* 检查命令是否打错 */
    oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_get_dscr_param_type::CMD ERR!");
    return OAL_FAIL;
}
void wal_hipriv_set_dscr_param_msg(wal_msg_write_stru *write_msg)
{
    mac_cfg_set_dscr_param_stru *set_dscr_param = (mac_cfg_set_dscr_param_stru *)(write_msg->auc_value);

    oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_data_dscr_param::en_param_index [%d]!,value[%d]}",
        set_dscr_param->uc_function_index, set_dscr_param->l_value);
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(write_msg, WLAN_CFGID_SET_DSCR, sizeof(mac_cfg_set_dscr_param_stru));
}
OAL_STATIC uint8_t wal_hipriv_get_dscr_cmd_type(uint8_t func_type, uint8_t wlan_band)
{
    uint8_t cmd_type;
    if (func_type == WAL_HIPRIV_DSCR_PARAM_CMD_UCAST_DATA) {
        /* 单播数据帧描述符设置 tpye = MAC_VAP_CONFIG_UCAST_DATA */
        cmd_type = MAC_VAP_CONFIG_UCAST_DATA;
    } else if (func_type == WAL_HIPRIV_DSCR_PARAM_CMD_MCAST_DATA) {
        cmd_type = MAC_VAP_CONFIG_MCAST_DATA;
    } else if (func_type == WAL_HIPRIV_DSCR_PARAM_CMD_BCAST_DATA) {
        cmd_type = MAC_VAP_CONFIG_BCAST_DATA;
    } else if (func_type == WAL_HIPRIV_DSCR_PARAM_CMD_UCAST_MGMT) {
        /* 单播管理帧描述符设置 tpye = MAC_VAP_CONFIG_UCAST_MGMT 2为2G,否则为5G  */
        cmd_type = (wlan_band == WLAN_BAND_2G) ? MAC_VAP_CONFIG_UCAST_MGMT_2G : MAC_VAP_CONFIG_UCAST_MGMT_5G;
    } else {
        cmd_type = (wlan_band == WLAN_BAND_2G) ? MAC_VAP_CONFIG_MBCAST_MGMT_2G : MAC_VAP_CONFIG_MBCAST_MGMT_5G;
    }

    return cmd_type;
}
uint32_t wal_hipriv_send_dscr_param(oal_net_device_stru *net_dev, wal_msg_write_stru *write_msg)
{
    int32_t l_ret;
    wal_hipriv_set_dscr_param_msg(write_msg);

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_dscr_param_stru), (uint8_t *)write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_dscr_param::wal_send_cfg_event err[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置描述符参数配置命令
 * 1.日    期  : 2022年3月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_dscr_param(oal_net_device_stru *net_dev, int8_t *param, uint8_t func_type)
{
    wal_msg_write_stru write_msg;
    uint32_t off_set, ret;
    mac_cfg_set_dscr_param_stru *set_dscr_param = (mac_cfg_set_dscr_param_stru *)(write_msg.auc_value);
    uint8_t param_index;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取描述符字段设置命令字符串 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_dscr_param::cmd err[%d]}", ret);
        return ret;
    }
    param += off_set;

    ret = wal_hipriv_get_dscr_param_type(name, &param_index);
    if (ret != OAL_SUCC) {
        return ret;
    }

    set_dscr_param->uc_function_index = param_index;

    if (func_type == WAL_HIPRIV_DSCR_PARAM_CMD_UCAST_MGMT || func_type == WAL_HIPRIV_DSCR_PARAM_CMD_MBCAST_MGMT) {
        /* 解析要设置为哪个频段的单播管理帧 2G or 5G */
        ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_dscr_param::get mgmt 2G/5G err[%d]!}\r\n", ret);
            return ret;
        }
        param += off_set;
    }

    set_dscr_param->en_type = wal_hipriv_get_dscr_cmd_type(func_type, (uint8_t)oal_atoi(name));

    /* 解析参数 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_dscr_param::val err [%d]!}", ret);
        return ret;
    }
    param += off_set;

    /* 配置通道、模式 */
#ifdef WIFI_DEBUG_ENABLE
    if ((param_index == WLA_DSCR_PARAM_TXCHAIN) ||
        (param_index >= WAL_DSCR_PARAM_TXBF && param_index <= WAL_DSCR_PARAM_STBC)) {
        if (get_mpxx_subchip_type() > BOARD_VERSION_MP15) {
            wal_hipriv_alg_cfg_entry(net_dev, "txmode_mode_sw disable");
        }
    }
#endif
    /* 配置速率、空间流数、带宽 */
    if (param_index >= WAL_DSCR_PARAM_RATE && param_index <= WAL_DSCR_PARAM_BW) {
        if (wal_hipriv_process_rate_params(net_dev, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, set_dscr_param) != OAL_SUCC) {
            return OAL_FAIL;
        }
    } else {
        set_dscr_param->l_value = oal_strtol(name, NULL, 0);
    }

    return wal_hipriv_send_dscr_param(net_dev, &write_msg);
}
/*
 * 函 数 名  : wal_hipriv_set_mcast_data_dscr_param
 * 功能描述  : 设置描述符参数配置命令
 * 1.日    期  : 2014年1月22日
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_mcast_data_dscr_param(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_hipriv_set_dscr_param(net_dev, param, WAL_HIPRIV_DSCR_PARAM_CMD_MCAST_DATA);
}

#ifdef _PRE_WLAN_FEATURE_HID2D
/*
 * 函 数 名  : wal_ioctl_set_hid2d_acs_mode
 * 功能描述  : HiD2D场景下自动信道切换总开关
 * 1.日    期  : 2020年04月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_ioctl_set_hid2d_acs_mode(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru              write_msg;
    uint32_t                        off_set;
    int8_t                          ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t                        result;
    int32_t                         ret;
    uint32_t                        value;

    /* 获取参数 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        return result;
    }
    pc_param += off_set;
    value = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_HID2D_ACS_MODE, sizeof(uint32_t));

    /* 设置配置命令参数 */
    *((uint32_t *)(write_msg.auc_value)) = value;

    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
        (uint8_t *)&write_msg,
        OAL_FALSE,
        NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{HiD2D::return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }
    return OAL_SUCC;
}
#endif

static uint64_t wal_ioctl_get_cur_tsf(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    return ktime_get_real_ns();
#else
    return oal_time_get_high_precision_ms();
#endif
}

static void wal_ioctl_copy_current_tsf_to_user(hmac_vap_stru *hmac_vap, uint64_t host_start_tsf,
    wal_wifi_priv_cmd_stru *priv_cmd)
{
    sync_tsf_stru sync_tsf;

    oam_warning_log4(0, 0, "{wal_ioctl_get_current_tsf:host tsf0, hi[0x%x], lo[0x%x]; tsf1, hi[0x%x], lo[0x%x]}",
        (uint32_t)(host_start_tsf >> BIT_OFFSET_32), (uint32_t)host_start_tsf,
        (uint32_t)(hmac_vap->sync_tsf.host_end_tsf >> BIT_OFFSET_32), (uint32_t)hmac_vap->sync_tsf.host_end_tsf);
    oam_warning_log2(0, 0, "{wal_ioctl_get_current_tsf: device tsf:hi[0x%x], lo[0x%x]}",
        (uint32_t)(hmac_vap->sync_tsf.device_tsf >> BIT_OFFSET_32), (uint32_t)hmac_vap->sync_tsf.device_tsf);

    /* 将查询结果上报到hal层 */
    sync_tsf.host_start_tsf = host_start_tsf;
    sync_tsf.device_tsf = hmac_vap->sync_tsf.device_tsf;
    sync_tsf.host_end_tsf = hmac_vap->sync_tsf.host_end_tsf;
    if (priv_cmd->total_len < sizeof(sync_tsf_stru)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_copy_current_tsf_to_user:total_len=%d", priv_cmd->total_len);
        return;
    }
    oal_copy_to_user(priv_cmd->puc_buf, &sync_tsf, sizeof(sync_tsf_stru));
}
/*
 * 功能描述  : 获取tsf信息
 * 日    期  : 2021年03月21日
 * 作    者  : wifi
 */
int32_t wal_ioctl_get_current_tsf(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr,
    wal_wifi_priv_cmd_stru *priv_cmd)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    uint64_t host_start_tsf;
    wal_msg_write_stru write_msg;
    signed long i_leftime;
    int32_t err;

    if (oal_any_null_ptr3(net_dev, ifr, priv_cmd)) {
        return -OAL_EINVAL;
    }

    mac_vap = (mac_vap_stru *)oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_get_current_tsf: vap is null}");
        return -OAL_EINVAL;
    }

    /* 未up状态下tsf不会启动, 无法查询tsf */
    if (mac_vap->en_vap_state != MAC_VAP_STATE_UP) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_get_current_tsf: vap_state is %d}", mac_vap->en_vap_state);
        return -OAL_EINVAL;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_get_current_tsf: cannot get hmac vap}");
        return -OAL_EINVAL;
    }

    hmac_vap->tsf_info_query_completed_flag = OAL_FALSE;

    /* 记录tsf查询开始前的时间 */
    host_start_tsf = wal_ioctl_get_cur_tsf();

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_QUERY_TSF, 0);
    err = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH, (uint8_t *)&write_msg,
        OAL_FALSE, NULL);
    if (err != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_get_current_tsf:send event failed[%d]}", err);
        return -OAL_EFAIL;
    }

    i_leftime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
        (OAL_TRUE == hmac_vap->tsf_info_query_completed_flag), 5 * OAL_TIME_HZ); // 5ms
    if (i_leftime == 0) {
        /* 超时还没有上报扫描结束, 1000倍 */
        oam_warning_log1(0, 0, "{query device tsf %ld ms timeout}", ((5 * OAL_TIME_HZ * 1000) / OAL_TIME_HZ)); // 5ms
        return -OAL_EINVAL;
    } else if (i_leftime < 0) {
        /* 定时器内部错误 , 1000倍 */
        oam_warning_log1(0, 0, "{query device tsf for %ld ms error}", ((5 * OAL_TIME_HZ * 1000) / OAL_TIME_HZ)); // 5ms
        return -OAL_EINVAL;
    } else {
        wal_ioctl_copy_current_tsf_to_user(hmac_vap, host_start_tsf, priv_cmd);
        return OAL_SUCC;
    }
}
