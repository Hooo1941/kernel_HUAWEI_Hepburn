/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WAL配置操作
 * 作    者 :
 * 创建日期 : 2012年11月6日
 */

/* 1 头文件包含 */
#include "alg_cfg.h"
#include "oal_types.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "oam_event_wifi.h"
#include "wlan_types.h"
#include "wlan_chip_i.h"

#include "mac_device.h"
#include "mac_vap.h"
#include "mac_resource.h"

#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_ext_if.h"
#include "hmac_config.h"
#include "wal_ext_if.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_bridge.h"
#include "mac_mib.h"
#include "hmac_cali_mgmt.h"

#include "hmac_wapi.h"

#if ((_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX) || (defined(WIN32)))
#include "plat_pm_wlan.h"
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_wifi6_self_cure.h"
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
#include "hmac_hid2d.h"
#endif
#include "hmac_ht_self_cure.h"
#include "hmac_p2p.h"
#include "hmac_11r.h"
#include "wal_config_alg_cfg.h"

#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_mgmt.h"
#include "hmac_chba_common_function.h"
#include "hmac_chba_user.h"
#include "hmac_chba_ps.h"
#include "hmac_chba_chan_switch.h"
#include "hmac_chba_coex.h"
#endif
#include "hmac_sae.h"
#include "hmac_dfx.h"
#ifdef _PRE_WLAN_FEATURE_PWL
#include "hmac_pwl.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_CONFIG_C

/* 2 全局变量定义 */
#ifdef _PRE_WINDOWS_SUPPORT
struct netdevice_reg g_netdevice_reg[MAX_NETDEVICE_NUM] = {
    { {0}, NULL, 0 },
    { {0}, NULL, 0 },
    { {0}, NULL, 0 },
    { {0}, NULL, 0 },
};
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_atomic g_wal_config_seq_num = ATOMIC_INIT(0);
#else
oal_atomic g_wal_config_seq_num = 0;
#endif

OAL_STATIC uint32_t wal_set_start_join_bssid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_set_start_join_bssid(pst_mac_vap, us_len, puc_param);
}
/* 3 函数实现 */
/*
 * 函 数 名  : wal_config_add_vap
 * 功能描述  : 创建VAP
 * 1.日    期  : 2012年11月14日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_add_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = hmac_config_add_vap(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_config_del_vap
 * 功能描述  : 删除vap
 * 1.日    期  : 2013年5月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_del_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY,
                       "{wal_config_del_vap::pst_mac_vap or puc_param is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_del_vap(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_config_del_vap:: return error code [%d].}\r\n",
                         ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_config_start_vap
 * 功能描述  : 启动VAP
 * 1.日    期  : 2012年11月14日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_start_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY,
                       "{wal_config_start_vap::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_start_vap(pst_mac_vap, us_len, puc_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_start_vap:: return error code %d.}\r\n",
                         ret);
        return ret;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_config_down_vap
 * 功能描述  : 停用vap
 * 1.日    期  : 2013年5月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_down_vap(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY,
                       "{wal_config_down_vap::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_down_vap(pst_mac_vap, us_len, puc_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_down_vap:: return error code [%d].}\r\n",
                         ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_config_set_bss_type
 * 功能描述  : 设置bss type
 * 1.日    期  : 2012年12月24日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_bss_type(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_bss_type(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_bss_type
 * 功能描述  : 读取bss type
 * 1.日    期  : 2012年12月24日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_bss_type(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_bss_type(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_mode
 * 功能描述  : 设置模式
 * 1.日    期  : 2012年12月24日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_mode(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_mode
 * 功能描述  : 设置模式
 * 1.日    期  : 2012年12月24日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_mib_by_bw_param_stru st_cfg = {0};

    if (pst_mac_vap == NULL || puc_param == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_config_set_mode::null ptr");
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_cfg.en_band = ((mac_cfg_mode_param_stru *)puc_param)->en_band;
    st_cfg.en_bandwidth = ((mac_cfg_mode_param_stru *)puc_param)->en_bandwidth;

    hmac_config_set_mib_by_bw(pst_mac_vap, (uint16_t)sizeof(st_cfg), (uint8_t *)&st_cfg);

    return hmac_config_set_mode(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_config_set_mode
 * 功能描述  : 设置模式
 * 1.日    期  : 2012年12月24日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_bandwidth(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_mode_param_stru *pst_prot_param;

    pst_prot_param = (mac_cfg_mode_param_stru *)puc_param;

    pst_prot_param->en_protocol = pst_mac_vap->en_protocol;
    pst_prot_param->en_band = pst_mac_vap->st_channel.en_band;

    return hmac_config_set_mode(pst_mac_vap, us_len, (uint8_t *)pst_prot_param);
}

/*
 * 函 数 名  : wal_config_set_mac_addr
 * 功能描述  : 设置mac地址
 * 1.日    期  : 2012年12月25日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_mac_addr(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY,
                       "{wal_config_set_mac_addr::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_set_mac_addr(pst_mac_vap, us_len, puc_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{hmac_config_set_mac_addr:: return error code %d.}\r\n",
                         ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_config_get_ssid
 * 功能描述  : 设置ssid
 * 1.日    期  : 2013年1月15日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_ssid(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_ssid(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_ssid
 * 功能描述  : 设置ssid

 * 1.日    期  : 2013年1月15日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_ssid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_ssid(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_shpreamble
 * 功能描述  : 设置前导码能力位
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_shpreamble(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_shpreamble(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_shpreamble
 * 功能描述  : 读取前导码能力位
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_shpreamble(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_shpreamble(pst_mac_vap, pus_len, puc_param);
}
#ifdef _PRE_WLAN_FEATURE_SNIFFER
/*
 * 函 数 名  : wal_config_set_sniffer
 * 功能描述  : 设置sniffer开关
 * 1.日    期  : 2019年5月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_sniffer(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_sniffer(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_monitor_mode
 * 功能描述  : 设置地址过滤能力位
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_monitor_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_monitor_mode(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_monitor_mode
 * 功能描述  : 读取前地址过滤能力位
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_monitor_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_monitor(pst_mac_vap, pus_len, puc_param);
}
#endif
OAL_STATIC uint32_t wal_config_set_pt_mcast(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_pt_mcast(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_shortgi20
 * 功能描述  : 设置shortgi
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_shortgi20(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_shortgi20(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_shortgi40
 * 功能描述  : 设置shortgi
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_shortgi40(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_shortgi40(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_shortgi80
 * 功能描述  : 设置shortgi
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_shortgi80(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_shortgi80(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_shortgi20
 * 功能描述  : 读取shortgi
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_shortgi20(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_shortgi20(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_shortgi40
 * 功能描述  : 读取shortgi
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_shortgi40(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_shortgi40(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_shortgi80
 * 功能描述  : 读取shortgi
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_shortgi80(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_shortgi80(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_prot_mode
 * 功能描述  : 设置保护模式
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_prot_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_prot_mode(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_prot_mode
 * 功能描述  : 读取保护模式
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_prot_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_prot_mode(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_auth_mode
 * 功能描述  : 设置认证模式
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_auth_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_auth_mode(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_auth_mode
 * 功能描述  : 读取认证模式
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_auth_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_auth_mode(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_bintval
 * 功能描述  : 设置beacon interval
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_bintval(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_bintval(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_bintval
 * 功能描述  : 读取beacon interval
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_bintval(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_bintval(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_dtimperiod
 * 功能描述  : 设置dtim_period
 * 1.日    期  : 2013年9月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_dtimperiod(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_dtimperiod(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_dtimperiod
 * 功能描述  : 读取dtim_period
 * 1.日    期  : 2013年9月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_dtimperiod(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_dtimperiod(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_nobeacon
 * 功能描述  : 设置隐式beacon
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_nobeacon(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_nobeacon(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_nobeacon
 * 功能描述  : 读取隐式beacon
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_nobeacon(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_nobeacon(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_txpower
 * 功能描述  : 设置发射功率
 * 1.日    期  : 2013年1月23日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_txpower(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_txpower(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_txpower
 * 功能描述  : 读取发射功率
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_txpower(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_txpower(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_freq
 * 功能描述  : 设置频点/信道
 * 1.日    期  : 2013年1月23日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_freq(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_freq(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_freq
 * 功能描述  : 读取频点/信道
 * 1.日    期  : 2013年1月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_freq(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_freq(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_cwmin
 * 功能描述  : 设置EDCA竞争窗口参数
 * 1.日    期  : 2013年1月23日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_wmm_params(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_wmm_params(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_config_get_uapsd_en
 * 功能描述  : 获取vap uapsd使能
 * 1.日    期  : 2013年9月18日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_uapsd_en(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_uapsden(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_mib_by_bw
 * 功能描述  : 根据带宽设置mib能力
 * 1.日    期  : 2016年06月21日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_mib_by_bw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_mib_by_bw(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_channel
 * 功能描述  : 设置信道信息，包括频段，信道号，带宽
 * 1.日    期  : 2013年10月26日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_cfg_mib_by_bw_param_stru st_cfg;
    mac_cfg_channel_param_stru *mac_cfg_channel = NULL;
    mac_channel_stru channel;

    if ((pst_mac_vap == NULL) || (puc_param == NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_config_set_channel::null ptr");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_cfg_channel = (mac_cfg_channel_param_stru *)puc_param;
    channel.en_band = mac_cfg_channel->en_band;
    channel.en_bandwidth = mac_cfg_channel->en_bandwidth;
    channel.ext6g_band = mac_cfg_channel->ext6g_band;
    channel.uc_chan_number = mac_cfg_channel->uc_channel;
    if (mac_is_channel_num_valid(channel.en_band, channel.uc_chan_number, channel.ext6g_band) == OAL_SUCC &&
        mac_get_channel_idx_from_num(channel.en_band, channel.uc_chan_number, channel.ext6g_band,
            &channel.uc_chan_idx) == OAL_SUCC) {
        mac_cfg_channel->en_bandwidth = hmac_config_update_2g_40mhz_bandwidth(pst_mac_vap, &channel);
    }

    st_cfg.en_band = mac_cfg_channel->en_band;
    st_cfg.en_bandwidth = mac_cfg_channel->en_bandwidth;
    hmac_config_set_mib_by_bw(pst_mac_vap, (uint16_t)sizeof(st_cfg), (uint8_t *)&st_cfg);

    return hmac_config_set_channel(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_beacon
 * 功能描述  : 设置设备物理参数，包括RTS 门限值，分片门限值
 * 1.日    期  : 2015年6月24日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_beacon(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY,
                       "{wal_config_set_beacon::pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_set_beacon(pst_mac_vap, us_len, puc_param);

    return ret;
}

/*
 * 函 数 名  : wal_config_vap_info
 * 功能描述  : 打印vap参数信息
 * 1.日    期  : 2013年5月28日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_vap_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_vap_info(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_btcoex_set_perf_param
 * 功能描述  : 配置性能参数
 * 1.日    期  : 2017年6月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_btcoex_set_perf_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_btcoex_set_perf_param(pst_mac_vap, us_len, puc_param);
}

#ifdef _PRE_WLAN_FEATURE_NRCOEX
OAL_STATIC uint32_t wal_config_query_nrcoex_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_query_nrcoex_stat(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_nrcoex_test(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_nrcoex_test(pst_mac_vap, us_len, puc_param);
}
#endif

/*
 * 函 数 名  : wal_config_lte_gpio_mode
 * 1.日    期  : 2015年7月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_lte_gpio_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_lte_gpio_mode(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_random_mac_addr_scan
 * 功能描述  : 设置随机mac addr扫描开关
 * 1.日    期  : 2015年5月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_random_mac_addr_scan(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_random_mac_addr_scan(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_random_mac_oui
 * 功能描述  : 设置随机mac oui
 * 1.日    期  : 2016年9月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_random_mac_oui(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_random_mac_oui(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_vowifi_nat_keep_alive_params
 * 功能描述  : 设置随机mac oui
 * 1.日    期  : 2016年9月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_vowifi_nat_keep_alive_params(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    return hmac_config_set_vowifi_nat_keep_alive_params(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_add_user
 * 功能描述  : 添加用户配置命令
 * 1.日    期  : 2013年6月5日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_add_user(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_add_user(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_del_user
 * 功能描述  : 删除用户配置命令
 * 1.日    期  : 2013年6月5日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_del_user(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_del_user(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_sta_list
 * 功能描述  : 取得关联sta list
 * 1.日    期  : 2015年5月22日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_sta_list(mac_vap_stru *pst_mac_vap, uint16_t *us_len, uint8_t *puc_param)
{
    return hmac_config_get_sta_list(pst_mac_vap, us_len, puc_param);
}

#ifdef _PRE_WLAN_FEATURE_TPC_OPT
/*
 * 函 数 名  : wal_config_reduce_sar
 * 功能描述  : 降SAR
 * 1.日    期  : 2014年8月27日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_reduce_sar(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_reduce_sar(pst_mac_vap, us_len, puc_param);
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
/*
 * 函 数 名  : wal_config_tas_pwr_ctrl
 * 功能描述  : tas抬功率
 * 1.日    期  : 2018年7月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_tas_pwr_ctrl(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_tas_pwr_ctrl(pst_mac_vap, us_len, puc_param);
}
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
/*
 * 函 数 名  : wal_config_tas_rssi_access
 * 功能描述  : tas测量天线
 * 1.日    期  : 2018年7月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_tas_rssi_access(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_tas_rssi_access(pst_mac_vap, us_len, puc_param);
}
#endif

/*
 * 函 数 名  : wal_config_cfg80211_start_sched_scan
 * 功能描述  : 启动PNO调度扫描
 * 1.日    期  : 2015年6月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_cfg80211_start_sched_scan(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_cfg80211_start_sched_scan(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_cfg80211_stop_sched_scan
 * 功能描述  : 停止PNO调度扫描
 * 1.日    期  : 2015年6月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_cfg80211_stop_sched_scan(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_cfg80211_stop_sched_scan(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_scan_abort
 * 功能描述  : 扫描终止
 * 1.日    期  : 2015年6月9日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_scan_abort(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_scan_abort(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_cfg80211_start_scan
 * 1.日    期  : 2013年8月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_cfg80211_start_scan(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_cfg80211_start_scan_sta(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_cfg80211_start_join
 * 1.日    期  : 2013年8月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 * 2.日    期  : 2013年10月24日
 *   作    者  : wifi
 *   修改内容  : 增加加密认证相关的处理
 * 3.日    期  : 2015年5月5日
 *   作    者  : wifi
 *   修改内容  : 处理逻辑挪到hmac
 */
OAL_STATIC uint32_t wal_config_cfg80211_start_join(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
#ifdef _PRE_WLAN_CHBA_MGMT
    if (pst_mac_vap->chba_mode == CHBA_MODE) {
        oam_warning_log0(0, 0, "CHBA: chba mode, connect with hid2d protocol.");
        return hmac_chba_initiate_connect(pst_mac_vap, us_len, puc_param);
    }
#endif
    return hmac_config_connect(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_send_disassoc(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_send_disassoc(mac_vap, len, param);
}

/*
 * 函 数 名  : wal_config_kick_user
 * 功能描述  : 去关联1个用户
 * 1.日    期  : 2013年8月27日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_kick_user(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_kick_user(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_country
 * 功能描述  : 设置国家码
 * 1.日    期  : 2013年10月18日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_country(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_country(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_country_for_dfs
 * 功能描述  : 设置国家码中的dfs信息
 * 1.日    期  : 2015年1月18日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_country_for_dfs(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_country_for_dfs(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_country
 * 功能描述  : 获取国家码
 * 1.日    期  : 2013年10月18日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_country(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_country(pst_mac_vap, pus_len, puc_param);
}

#ifdef _PRE_WINDOWS_SUPPORT
// IHV功能新增函数
void wal_config_get_country_windows(uint8_t *param)
{
    mac_regdomain_info_stru *regdomain_info = NULL;
    mac_cfg_get_country_stru *country_param = (mac_cfg_get_country_stru *)param;
    mac_get_regdomain_info(&regdomain_info);
    country_param->ac_country[BYTE_OFFSET_0] = regdomain_info->ac_country[BYTE_OFFSET_0];
    country_param->ac_country[BYTE_OFFSET_1] = regdomain_info->ac_country[BYTE_OFFSET_1];
    country_param->ac_country[BYTE_OFFSET_2] = regdomain_info->ac_country[BYTE_OFFSET_2];
}
#endif

/*
 * 函 数 名  : wal_config_get_tid
 * 功能描述  : 获取最新数据帧的tid
 * 1.日    期  : 2013年10月18日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_tid(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_tid(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_user_info
 * 功能描述  : 打印user信息
 * 1.日    期  : 2013年5月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_user_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_user_info(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_dscr_param
 * 功能描述  : 打印user信息
 * 1.日    期  : 2013年5月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_dscr_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_dscr_param(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_log_level
 * 功能描述  : 设置日志级别，事件抛向hmac
 * 1.日    期  : 2015年1月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_log_level(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_log_level(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_rate
 * 功能描述  : 设置non-HT速率
 * 1.日    期  : 2014年3月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_rate(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
    mac_cfg_tx_comp_stru st_event_set_bcast;

    memset_s(&st_event_set_bcast, sizeof(mac_cfg_tx_comp_stru),
             0, sizeof(mac_cfg_tx_comp_stru));
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY,
                         "{wal_config_set_rate::pst_mac_vap/puc_param is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置参数 */
    ret = hmac_config_set_rate(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_set_rate::hmac_config_set_rate error!}\r\n");
        return ret;
    }
    return ret;
}

/*
 * 函 数 名  : wal_config_set_mcs
 * 功能描述  : 设置HT速率
 * 1.日    期  : 2014年3月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_mcs(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_mcs::pst_mac_vap/puc_param is null ptr}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置参数 */
    ret = hmac_config_set_mcs(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_set_mcs::hmac_config_set_mcs error.}\r\n");
        return ret;
    }

    return ret;
}

/*
 * 函 数 名  : wal_config_set_mcsac
 * 功能描述  : 设置VHT速率
 * 1.日    期  : 2014年3月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_mcsac(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_mcsac::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置参数 */
    ret = hmac_config_set_mcsac(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_set_mcsac::hmac_config_set_mcsac error!}\r\n");
        return ret;
    }

    return ret;
}

OAL_STATIC uint32_t wal_config_set_channge_go_channel(mac_vap_stru *mac_vap,
    uint16_t len, uint8_t *param)
{
    uint32_t ret;
    ret = hmac_p2p_change_go_channel(mac_vap, len, param);
    if (ret != OAL_SUCC) {
        hmac_chr_mvap_report(mac_vap, MAC_MVAP_ERROR_SCENE_CSA);
    }
    return ret;
}

OAL_STATIC uint32_t wal_config_clear_11n_blacklist(mac_vap_stru *p_mac_vap, uint16_t len, uint8_t *p_param)
{
    return hmac_ht_self_cure_clear_blacklist();
}

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC uint32_t wal_config_set_11ax_wifi6_black_list(mac_vap_stru *p_mac_vap,
    uint16_t len, uint8_t *p_param)
{
    return hmac_wifi6_self_cure_black_list_set(p_mac_vap, len, p_param);
}

OAL_STATIC uint32_t wal_config_set_11ax_close_htc(mac_vap_stru *p_mac_vap,
    uint16_t len, uint8_t *p_param)
{
    return hmac_wifi6_self_cure_close_htc_handle(p_mac_vap);
}

OAL_STATIC uint32_t wal_config_set_mcsax(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_mcsax::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置参数 */
    ret = hmac_config_set_mcsax(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_set_mcsax::hmac_config_set_mcsac error!}\r\n");
        return ret;
    }

    return ret;
}
OAL_STATIC uint32_t _wal_config_set_mcsax(mac_vap_stru *pst_mac_vap,
    uint16_t us_len, uint8_t *puc_param)
{
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        return wal_config_set_mcsax(pst_mac_vap, us_len, puc_param);
    }

    return OAL_SUCC;
}
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
OAL_STATIC uint32_t wal_config_set_mcsax_er(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_mcsax_er::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置参数 */
    ret = hmac_config_set_mcsax_er(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_set_mcsax_er::hmac_config_set_mcsac error!}\r\n");
        return ret;
    }

    return ret;
}
OAL_STATIC uint32_t _wal_config_set_mcsax_er(mac_vap_stru *pst_mac_vap,
    uint16_t us_len, uint8_t *puc_param)
{
    if (g_wlan_spec_cfg->feature_11ax_er_su_dcm_is_open) {
        return wal_config_set_mcsax_er(pst_mac_vap, us_len, puc_param);
    }

    return OAL_SUCC;
}
#endif
#endif

/*
 * 函 数 名  : wal_config_set_bw
 * 功能描述  : 设置带宽
 * 1.日    期  : 2014年3月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_bw(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_bw::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置参数 */
    ret = hmac_config_set_bw(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_config_set_bw::hmac_config_set_bw error!}\r\n");
        return ret;
    }

    return ret;
}

/*
 * 函 数 名  : wal_config_set_band
 * 功能描述  : 设置频段
 * 1.日    期  : 2020年9月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_band(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;
    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_band::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置参数 */
    ret = hmac_config_set_band(mac_vap, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_config_set_bw::hmac_config_set_bw error!}\r\n");
        return ret;
    }

    return ret;
}

/*
 * 函 数 名  : wal_config_always_tx
 * 功能描述  : 广播数据包
 * 1.日    期  : 2014年3月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_always_tx(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_always_tx::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_always_tx(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_always_tx::hmac_config_always_tx failed!}\r\n");
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 配置长收模式
 * 1.日    期  : 2020年3月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_ru_index(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_ru_index::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_ru_index(mac_vap, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_ru_index::hmac_config_always_tx failed!}\r\n");
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_config_always_rx
 * 功能描述  : 配置常收模式
 * 1.日    期  : 2014年3月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_always_rx(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_always_rx::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_always_rx(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_always_rx::hmac_config_always_rx failed!}\r\n");
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_config_memory_monitor(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    uint32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(mac_vap, param))) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_memory_monitor::pst_mac_vap/puc_param is null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_config_memory_monitor_info(mac_vap, len, param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_memory_monitor:: failed!}\r\n");
        return ret;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_config_set_tx_pow_param
 * 功能描述  : 配置发送功率
 * 1.日    期  : 2016年12月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_tx_pow_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_tx_pow_param(pst_mac_vap, us_len, puc_param);
}
#ifdef PLATFORM_DEBUG_ENABLE
/*
 * 函 数 名  : wal_config_reg_info
 * 1.日    期  : 2013年5月31日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_reg_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_reg_info(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_reg_write
 * 1.日    期  : 2013年9月6日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_reg_write(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_reg_write(pst_mac_vap, us_len, puc_param);
}
#endif
#ifdef _PRE_WLAN_EXPORT
/*
 * 函 数 名  : wal_config_mac_addr_write
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2023年08月08日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_mac_addr_write(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_mac_addr_write(mac_vap, len, param);
}

/*
 * 函 数 名  : wal_config_mac_addr_read
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2023年08月08日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_mac_addr_read(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_mac_addr_read(mac_vap, len, param);
}
#endif
/*
 * 函 数 名  : wal_config_dpd_cfg
 * 1.日    期  : 2013年9月6日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_dpd_cfg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_dpd_cfg(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_sar_slide_cfg(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_sar_slide_cfg(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_compete_pow_cfg(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_compete_pow_cfg(mac_vap, len, param);
}

/*
 * 函 数 名  : wal_config_sdio_flowctrl
 * 1.日    期  : 2015年3月30日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_sdio_flowctrl(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_sdio_flowctrl(pst_mac_vap, us_len, puc_param);
}

#ifdef _PRE_WLAN_DELAY_STATISTIC
/*
 * 函 数 名  : wal_config_pkt_time_switch
 * 功能描述  : wal层调用hmac_config_pkt_time_switch函数
 * 1.日    期  : 2019年09月14日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_pkt_time_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_pkt_time_switch(pst_mac_vap, us_len, puc_param);
}
#endif

/*
 * 函 数 名   : wal_config_alg_cfg_entry
 * 功能描述   : wlan_cfg wal 层算法公共入口
 *
 * 1.日    期   : 2018年6月22日
 *   修改内容   : 新生成函数
 */
uint32_t wal_config_alg_cfg_param_host_entry(mac_vap_stru *pst_mac_vap, uint16_t us_input_len,
    uint8_t *puc_input)
{
    alg_cfg_hdr_stru          *pst_alg_cfg_hdr = NULL;
    alg_cfg_process_info_stru *pst_alg_cfg_info = NULL;
    uint32_t ret;

    if (pst_mac_vap == NULL || puc_input == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_config_alg_cfg_param_host_entry::null ptr error!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alg_cfg_hdr = (alg_cfg_hdr_stru *)puc_input;

    pst_alg_cfg_info = alg_cfg_search_process_info_by_cfg_id(pst_alg_cfg_hdr->en_cfg_id, pst_alg_cfg_hdr->en_cfg_type);
    if (pst_alg_cfg_info == NULL) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{wal_config_alg_cfg_param_host_entry::search process info failed! cfg_id=[%d], \
            cfg_type=[%d]!\r\n}", pst_alg_cfg_hdr->en_cfg_id, pst_alg_cfg_hdr->en_cfg_type);
        return OAL_FAIL;
    }

    if (pst_alg_cfg_info->st_process_info.en_need_device_process) {
        /* post event to dmac */
        ret = hmac_config_alg_send_event(pst_mac_vap, WLAN_CFGID_ALG_ENTRY, us_input_len, puc_input);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    return OAL_SUCC;
};

/*
 * 函 数 名  : wal_config_alg_param
 * 功能描述  : 算法命令示例
 * 1.日    期  : 2013年10月11日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_alg_param(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_alg_param(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_rx_fcs_info
 * 功能描述  : 打印接收帧的FCS信息
 * 1.日    期  : 2014年3月8日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_rx_fcs_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_rx_fcs_info(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_pdet_info
 * 功能描述  : 打印al tx pdet信息
 * 1.日    期  : 2023年2月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_pdet_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_tx_pdet_info(pst_mac_vap, us_len, puc_param);
}
#if ((defined(_PRE_WLAN_EXPORT)) && (defined(_PRE_WLAN_NARROW_BAND)))
OAL_STATIC uint32_t wal_config_get_sw_version(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_get_sw_version(pst_mac_vap, us_len, puc_param);
}
#endif
/*
 * 函 数 名  : wal_config_get_netdev
 * 功能描述  : 获取netdev
 * 1.日    期  : 2019年8月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
oal_net_device_stru *wal_config_get_netdev(const int8_t *pc_name, int32_t array_len)
{
#ifdef _PRE_WINDOWS_SUPPORT
    int n;
#endif
    if (array_len < 0) {
        return NULL;
    }
#ifdef _PRE_CONFIG_HISI_110X_DISABLE_HISILICON0
    if ((oal_strcmp("Hisilicon0", pc_name)) == 0) {
         /* 单芯片的配置vap id=0，双芯片存在配置vap id=0和1，hisilicon0是vap id=0，hisilicon1是vap id=1 */
        return hmac_vap_get_net_device(0);
    }
#endif

#ifdef _PRE_WINDOWS_SUPPORT
    for (n = 0; n < MAX_NETDEVICE_NUM; n++) {
        if ((g_netdevice_reg[n].flag == TRUE) && (oal_strcmp(g_netdevice_reg[n].name, pc_name) == 0)) {
            return g_netdevice_reg[n].net_device;
        }
    }
    return NULL;
#else
    return oal_dev_get_by_name(pc_name);
#endif
}

/*
 * 函 数 名  : wal_config_add_key
 * 功能描述  : WAL第二层,调用hmac层实现配置密钥
 * 1.日    期  : 2013年12月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_add_key(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_11i_add_key(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_config_get_key
 * 功能描述  : WAL第二层,调用hmac层实现
 * 1.日    期  : 2014年1月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_key(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    /* 通过函数调用，hmac具体实现 */
    return (hmac_config_11i_get_key(pst_mac_vap, us_len, puc_param));
}

/*
 * 函 数 名  : wal_config_remove_key
 * 功能描述  : WAL第二层，抛事件到HMAC
 * 1.日    期  : 2014年1月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_remove_key(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    /* 通过函数调用，hmac具体实现 */
    return (hmac_config_11i_remove_key(pst_mac_vap, us_len, puc_param));
}
/*
 * 函 数 名  : wal_config_set_wps_ie
 * 功能描述  : AP 设置WPS 信息元素到VAP 结构中
 * 1.日    期  : 2014年4月16日
 *   修改内容  : 新生成函数
 */
uint32_t wal_config_set_wps_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_wps_ie(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_wps_p2p_ie
 * 功能描述  : AP 设置WPS 信息元素到VAP 结构中
 * 1.日    期  : 2014年11月26日
  *   修改内容  : 新生成函数
 */
uint32_t wal_config_set_wps_p2p_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_wps_p2p_ie(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_open_wmm
 * 功能描述  : 测试命令，打开/关闭WMM
 * 1.日    期  : 2014年1月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_open_wmm(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY,
                       "{wal_config_open_wmm::pst_mac_vap/puc_param is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 针对配置vap做保护 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{wal_config_open_wmm::this is config vap! can't get info.}");
        return OAL_FAIL;
    }

    return hmac_config_sync_cmd_common(pst_mac_vap, WLAN_CFGID_WMM_SWITCH, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_vowifi_info
 * 功能描述  : 设置VoWiFi相关参数
 * 1.日    期  : 2016年4月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_vowifi_info(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_vowifi_info(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_rx_filter
 * 功能描述  : 配置rx ip数据包过滤的参数
 * 1.日    期  : 2017年4月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_update_ip_filter(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_update_ip_filter(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_probe_resp_mode
 * 功能描述  : 同步probe resp mode能力
 * 1.日    期  : 2019年1月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t  wal_config_set_probe_resp_mode(mac_vap_stru *pst_mac_vap,
    uint16_t us_len, uint8_t *puc_param)
{
    if ((pst_mac_vap == NULL) || (puc_param == NULL)) {
        oam_error_log0(0, OAM_SF_TX, "{wal_config_set_probe_resp_mode::pst_mac_vap or puc_param is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return hmac_config_set_probe_resp_mode(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_version
 * 功能描述  : 获取版本
 * 1.日    期  : 2014年3月28日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_version(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_get_version(pst_mac_vap, us_len, puc_param);
}


/*
 * 函 数 名  : wal_config_blacklist_add
 * 功能描述  : 增加黑名单
 * 1.日    期  : 2012年12月17日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_blacklist_add(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if ((pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_config_blacklist_add::not valid vap mode=%d!}\r\n",
                         pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }
    return hmac_config_blacklist_add(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_blacklist_add_only
 * 功能描述  : 增加黑名单
 * 1.日    期  : 2015年08月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_blacklist_add_only(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if ((pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_config_blacklist_add_only::not valid vap mode=%d!}\r\n",
                         pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }
    return hmac_config_blacklist_add_only(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_blacklist_del
 * 功能描述  : 设置mac地址
 * 1.日    期  : 2012年12月24日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_blacklist_del(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if ((pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_config_blacklist_del::not valid vap mode=%d!}\r\n",
                         pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }
    return hmac_config_blacklist_del(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_blacklist_clr
 * 功能描述  : 清空黑名单
 * 1.日    期  : 2017年3月25日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_blacklist_clr(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if ((pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_config_blacklist_clr::not valid vap mode=%d!}\r\n",
                         pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }

    if (memcpy_s(puc_param, us_len, BROADCAST_MACADDR, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_config_blacklist_clr::memcpy fail!");
        return OAL_FAIL;
    }

    return hmac_config_blacklist_del(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_blacklist_mode
 * 功能描述  : 读取黑名单模式
 * 1.日    期  : 2014年3月21日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_blacklist_mode(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    if ((pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_config_get_blacklist_mode::not valid vap mode=%d!}\r\n",
                         pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }
    *pus_len = sizeof(int32_t);

    return hmac_config_get_blacklist_mode(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_blacklist_mode
 * 功能描述  : 配置黑名单模式
 * 1.日    期  : 2014年3月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_blacklist_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if ((pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_config_set_blacklist_mode::not valid vap mode=%d!}\r\n",
                         pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }
    return hmac_config_set_blacklist_mode(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_config_set_pmksa
 * 功能描述  : 添加一条pmksa
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_pmksa(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_config_del_pmksa
 * 功能描述  : 添加一条pmksa
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_del_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_del_pmksa(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_config_flush_pmksa
 * 功能描述  : 添加一条pmksa
 * 1.日    期  : 2016年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_flush_pmksa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_flush_pmksa(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_remain_on_channel
 * 功能描述  : 保持在指定信道
 * 1.日    期  : 2014年11月22日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_remain_on_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_p2p_set_remain_on_channel(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_cancel_remain_on_channel
 * 功能描述  : 停止保持在指定信道
 * 1.日    期  : 2014年11月22日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_cancel_remain_on_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_p2p_cancel_remain_on_channel(pst_mac_vap, us_len, puc_param);
}

#ifdef _PRE_WLAN_FEATURE_SAE
/*
 * 函 数 名  : wal_config_external_auth
 * 功能描述  : 执行SAE关联
 * 1.日    期  : 2019年1月2日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_external_auth(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_external_auth(pst_mac_vap, us_len, puc_param);
}
#endif /* _PRE_WLAN_FEATURE_SAE */

/*
 * 函 数 名  : wal_config_set_sta_pm_on
 * 功能描述  : 打开sta device 低功耗
 * 1.日    期  : 2015年10月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_sta_pm_on(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    mac_cfg_ps_open_stru *pst_sta_pm_open = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_warning_log0(0, OAM_SF_PWR, "{wal_config_set_sta_pm_on::pst_mac_vap / puc_param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_PWR,
                         "{wal_config_set_sta_pm_on::pst_hmac_vap null,vap state[%d].}",
                         pst_mac_vap->en_vap_state);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_sta_pm_open = (mac_cfg_ps_open_stru *)puc_param;

    /* 如果上层主动dhcp成功此时取消超时开低功耗的定时器 */
    if ((pst_hmac_vap->st_ps_sw_timer.en_is_registerd == OAL_TRUE) &&
        (pst_sta_pm_open->uc_pm_enable > MAC_STA_PM_SWITCH_OFF)) {
        frw_timer_immediate_destroy_timer_m(&(pst_hmac_vap->st_ps_sw_timer));
    }

    return hmac_config_set_sta_pm_on(pst_mac_vap, us_len, puc_param);
}

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
OAL_STATIC uint32_t wal_config_set_rx_listen_ps_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    return hmac_config_set_rx_listen_ps_switch(pst_mac_vap, us_len, puc_param);
}
#endif

/*
 * 函 数 名  : wal_config_mgmt_tx
 * 功能描述  : 发送管理帧
 * 1.日    期  : 2014年12月30日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_mgmt_tx(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_wpas_mgmt_tx(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_query_station_stats
 * 功能描述  : 查询station的信息
 * 1.日    期  : 2014年12月3日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_query_station_stats(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_query_station_info(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_query_rssi
 * 功能描述  : 查询用户的rssi
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_query_rssi(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_query_rssi(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_query_rate
 * 功能描述  : 查询dmac用户当前的tx rx phy rate
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_query_rate(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_query_rate(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_query_psst
 * 功能描述  : 查询dmac用户当前的省功耗状态
 * 1.日    期  : 2016年4月11日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_query_psst(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_query_psst(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_query_psm_flt_stat
 * 功能描述  : 查询低功耗帧过滤统计
 * 1.日    期  : 2019年3月19日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_query_psm_flt_stat(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_query_psm_flt_stat(pst_mac_vap, us_len, puc_param);
}

#ifdef _PRE_WLAN_DFT_STAT
/*
 * 函 数 名  : wal_config_query_ani
 * 功能描述  : 查询dmac vap的抗干扰参数
 * 1.日    期  : 2016年4月15日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_query_ani(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_query_ani(pst_mac_vap, us_len, puc_param);
}
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
/*
 * 函 数 名  : wal_config_set_qos_map
 * 功能描述  : 设置QoSMap参数
 * 1.日    期  : 2015年9月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_qos_map(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_qos_map(pst_mac_vap, us_len, puc_param);
}
#endif

OAL_STATIC uint32_t  wal_config_set_dc_status(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_dc_status(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_p2p_miracast_status
 * 功能描述  : 设置投屏模式状态
 * 1.日    期  : 2018年12月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_p2p_miracast_status(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_p2p_miracast_status(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_p2p_ps_ops
 * 功能描述  : 设置P2P OPS 节能
 * 1.日    期  : 2015年1月14日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_p2p_ps_ops(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_p2p_ps_ops(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_p2p_ps_noa
 * 功能描述  : 设置P2P NOA 节能
 * 1.日    期  : 2015年1月14日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_p2p_ps_noa(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_p2p_ps_noa(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_cfg_vap_h2d(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_cfg_vap_h2d(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_config_host_dev_init
 * 功能描述  : 初始化host device_struc结构体，目前用于上下电
 * 1.日    期  : 2015年11月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_host_dev_init(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_host_dev_init(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_host_dev_exit
 * 功能描述  : 去初始化host device_struc结构体，目前用于上下电
 * 1.日    期  : 2015年11月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_host_dev_exit(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    return hmac_config_host_dev_exit(hmac_device);
}
/*
 * 函 数 名  : wal_send_cali_matrix_data
 * 功能描述  : 下发校准用到的矩阵信息
 * 1.日    期  : 2018年4月8日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_send_cali_matrix_data(oal_net_device_stru *cfg_net_dev)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_chip_send_cali_matrix_data(oal_net_dev_priv(cfg_net_dev));
#endif
}

/*
 * 函 数 名  : wal_config_set_max_user
 * 功能描述  : 设置最大用户数
 * 1.日    期  : 2015年5月22日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_max_user(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t max_user;

    max_user = *((uint32_t *)puc_param);

    return hmac_config_set_max_user(pst_mac_vap, max_user);
}

#ifdef _PRE_WLAN_FEATURE_11R
/*
 * 函 数 名  : wal_config_set_ft_ies
 * 功能描述  : 设置ft ies
 * 1.日    期  : 2015年8月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_ft_ies(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_ft_ies(pst_mac_vap, us_len, puc_param);
}
#endif  // _PRE_WLAN_FEATURE_11R

OAL_STATIC uint32_t wal_config_get_dieid(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_get_dieid(pst_mac_vap, us_len, puc_param);
}

#ifdef PLATFORM_DEBUG_ENABLE
OAL_STATIC uint32_t wal_config_get_tsensor_val(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_get_temperature(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_get_config_word(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_get_hw_word(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_get_board_sn(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_get_board_sn(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_set_board_sn(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_board_sn(pst_mac_vap, us_len, puc_param);
}

OAL_STATIC uint32_t wal_config_flash_test(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_flash_test(pst_mac_vap, us_len, puc_param);
}
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*
 * 函 数 名  : wal_config_set_all_log_level
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_all_log_level(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_all_log_level(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_cus_rf
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_cus_rf(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_cus_rf(pst_mac_vap, us_len, puc_param);
}
/*
 * 函 数 名  : wal_config_set_cus_dts_cali
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_cus_dts_cali(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_cus_dts_cali(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_cus_nvram_params
 * 1.日    期  : 2015年10月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_cus_nvram_params(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_cus_nvram_params(pst_mac_vap, us_len, puc_param);
}

uint32_t wal_send_custom_data(mac_vap_stru *pst_mac_vap, uint16_t len, uint8_t *param, uint8_t syn_id)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (pst_mac_vap != NULL) {
        return hmac_send_custom_data(pst_mac_vap, len, param, syn_id);
    } else {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "wal_send_custom_data:mac vap is NULL");
        return OAL_FAIL;
    }
#endif
}

/*
 * 函 数 名  : wal_config_set_cus_dyn_cali
 * 功能描述  : 下发动态校准参数
 * 1.日    期  : 2017年12月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_cus_dyn_cali(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_set_cus_dyn_cali(mac_vap, len, param);
}

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

/*
 * 函 数 名  : wal_config_set_vendor_ie
 * 功能描述  : AP 设置okc 信息元素到VAP 结构中

 * 1.日    期  : 2016年1月19日
 *   修改内容  : 新生成函数
 */
uint32_t wal_config_set_vendor_ie(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_vendor_ie(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_sta_11h_abillty
 * 功能描述  : AP 获取某个已关联STA的11h能力信息
 * 1.日    期  : 2017年6月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_config_get_sta_11h_abillty(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_get_sta_11h_abillty(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_mlme
 * 1.日    期  : 2017年2月18日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_mlme(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_mlme(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_vap_destroy
 * 功能描述  : down掉并删除vap
 * 1.日    期  : 2015年10月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_vap_destroy(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);

    return hmac_vap_destroy(pst_hmac_vap);
}

#ifdef _PRE_WLAN_FEATURE_11K
OAL_STATIC uint32_t wal_config_send_neighbor_req(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_send_neighbor_req(pst_mac_vap, us_len, puc_param);
}
OAL_STATIC uint32_t wal_config_11k_scan_fail_restart_roam(mac_vap_stru *pst_mac_vap, uint16_t us_len,
    uint8_t *puc_param)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    hmac_roam_info_stru *roam_info = NULL;
    if (hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "wal_config_11k_scan_fail_restart_roam:: vap null.");
        return OAL_FAIL;
    }
    if (hmac_vap->pul_roam_info == NULL) {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "wal_config_11k_scan_fail_restart_roam:: roam info clear.");
        return OAL_FAIL;
    }
    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    /* 非命令触发，驱动自发，避免重复触发11k扫描 */
    roam_info->is_roaming_trigged_by_cmd = OAL_FALSE;
    roam_info->st_static.uc_scan_mode = HMAC_CHR_NORMAL_SCAN;
    oam_warning_log0(0, OAM_SF_ANY, "{wal_config_11k_scan_fail_restart_roam::Trigger normal scan roam.}");
    return hmac_roam_start(hmac_vap, ROAM_SCAN_CHANNEL_ORG_DBDC, OAL_TRUE, NULL, ROAM_TRIGGER_11K_FAIL_RESTART);
}
#endif  // _PRE_WLAN_FEATURE_11K

OAL_STATIC uint32_t wal_config_vendor_cmd_get_channel_list(mac_vap_stru *pst_mac_vap,
                                                           uint16_t *pus_len,
                                                           uint8_t *puc_param)
{
    return hmac_config_vendor_cmd_get_channel_list(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_wmmswitch
 * 功能描述  : 获取wmm开关状态
 * 1.日    期  : 2017年03月09日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_wmmswitch(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_wmmswitch(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_vap_wmm_switch
 * 功能描述  : 获取vap下wmm开关状态
 * 1.日    期  : 2017年08月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_vap_wmm_switch(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_vap_wmm_switch(pst_mac_vap, pus_len, puc_param);
}

/*
 * 函 数 名  : wal_config_set_vap_wmm_switch
 * 功能描述  : 打开/关闭vap下WMM开关
 * 1.日    期  : 2017年8月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_vap_wmm_switch(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_config_set_vap_wmm_switch::pst_mac_vap/puc_param is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 针对配置vap做保护 */
    if (pst_mac_vap->en_vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{wal_config_set_vap_wmm_switch::this is config vap! can't get info.}");
        return OAL_FAIL;
    }

    return hmac_config_set_vap_wmm_switch(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_get_max_user
 * 功能描述  : 获取最大用户数
 * 1.日    期  : 2017年3月8日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_max_user(mac_vap_stru *pst_mac_vap, uint16_t *pus_len, uint8_t *puc_param)
{
    return hmac_config_get_max_user(pst_mac_vap, pus_len, puc_param);
}

#ifdef _PRE_WLAN_FEATURE_M2S
#ifdef _PRE_WLAN_FEATURE_M2S_MODEM
OAL_STATIC uint32_t wal_config_set_m2s_switch_modem(mac_vap_stru *pst_mac_vap,
    uint16_t us_len, uint8_t *puc_param)
{
    if (g_wlan_spec_cfg->feature_m2s_modem_is_open) {
        return hmac_config_set_m2s_switch_modem(pst_mac_vap, us_len, puc_param);
    }

    return OAL_SUCC;
}
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
OAL_STATIC uint32_t wal_config_get_m2s_state(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_get_m2s_state(mac_vap, len, param);
}
#endif

#ifndef CONFIG_HAS_EARLYSUSPEND
/*
 * 函 数 名  : wal_config_set_suspend_mode
 * 功能描述  : 设置亮暗屏状态
 * 1.日    期  : 2017年11月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_set_suspend_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    mac_device_stru *pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (pst_mac_device == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "wal_config_set_suspend_mode:pst_mac_device is null ptr!");
        return OAL_ERR_CODE_PTR_NULL;
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 0:亮屏 1:暗屏 */
    if (*puc_param == 0) {
        hmac_do_suspend_action(pst_mac_device, OAL_FALSE);
    } else {
        hmac_do_suspend_action(pst_mac_device, OAL_TRUE);
    }
#endif
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_APF
/*
 * 函 数 名  : wal_config_set_apf_program
 * 功能描述  : apf命令
 * 1.日    期  : 2017年11月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_apf_filter_cmd(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_apf_filter_cmd(pst_mac_vap, us_len, puc_param);
}

#endif

/*
 * 函 数 名  : wal_config_rx_filter_force_switch
 * 功能描述  : 移除或者恢复某个上层下发的IE
 * 1.日    期  : 2018年7月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_force_stop_filter(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_force_stop_filter(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_softap_mimo_mode
 * 功能描述  : 上层配置softap启动是否用mimo模式
 * 1.日    期  : 2019年1月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_config_softap_mimo_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_softap_mimo_mode(pst_mac_vap, us_len, puc_param);
}

#ifdef _PRE_WLAN_FEATURE_NAN
OAL_STATIC uint32_t wal_config_nan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_nan(mac_vap, len, param);
}
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
OAL_STATIC uint32_t wal_config_hid2d_acs_mode(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_set_hid2d_acs_mode(pst_mac_vap, us_len, puc_param);
}
OAL_STATIC uint32_t wal_config_hid2d_acs_state(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_hid2d_update_acs_state(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_hid2d_link_meas(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_hid2d_link_meas(mac_vap, len, param);
}
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
OAL_STATIC uint32_t wal_config_switch_channel(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_hid2d_switch_channel(pst_mac_vap, us_len, puc_param);
}
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
OAL_STATIC uint32_t wal_config_wifi_rtt_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        return wlan_chip_config_wifi_rtt_config(mac_vap, len, param);
    }

    return OAL_SUCC;
}
#endif
OAL_STATIC uint32_t wal_config_get_sync_tsf(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_get_sync_tsf(mac_vap, len, param);
}
#ifdef _PRE_WLAN_CHBA_MGMT
OAL_STATIC uint32_t wal_config_start_chba(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_start_chba(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_conn_ntf(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_chba_response_connect(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_chan_switch_cmd(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_chba_test_chan_switch_cmd(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_log_level(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_chba_log_level(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_set_ps_thres(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_chba_set_ps_thres(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_module_init(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_chba_module_init_cmd(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_set_battery(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_chba_set_battery(mac_vap, len, param);
}
static uint32_t wal_config_chba_set_coex_chan(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_chba_set_coex_chan(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_auto_bitmap_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_chba_auto_bitmap_debug(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_set_user_bitmap_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_chba_set_user_bitmap_debug(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_set_vap_bitmap_debug(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_chba_set_vap_bitmap_debug(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_island_chan_switch(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_chba_island_chan_switch(mac_vap, len, param);
}
OAL_STATIC uint32_t wal_config_chba_feature_switch(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_chba_set_feature_switch(mac_vap, len, param);
}
#endif

/*
 * 函数名:wal_config_set_sae_pwe
 * 功能:设置SAE_PWE 参数
 */
static uint32_t wal_config_set_sae_pwe(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_set_sae_pwe(mac_vap, len, param);
}

OAL_STATIC uint32_t wal_config_query_tid_queue(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    return hmac_config_query_tid_queue(pst_mac_vap, us_len, puc_param);
}

/*
 * 函 数 名  : wal_config_dpd_cali_factory
 * 1.日    期  : 2021年6月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_dpd_cali_factory(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_dpd_cali_factory(mac_vap, len, param);
}

/*
 * 函 数 名  : wal_config_send_tx_pow_data_to_device
 * 1.日    期  : 2023年3月
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_send_tx_pow_data_to_device(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_sync_cmd_common(mac_vap, WLAN_CFGID_TX_POW_H2D, len, param);
}
/*
 * 函 数 名  : wal_config_get_tx_pow_data_from_device
 * 1.日    期  : 2023年3月
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_tx_pow_data_from_device(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_sync_cmd_common(mac_vap, WLAN_CFGID_GET_TX_POW, len, param);
}
/*
 * 函 数 名  : wal_config_send_xo_diff_to_device
 * 1.日    期  : 2023年3月
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_send_xo_diff_to_device(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_sync_cmd_common(mac_vap, WLAN_CFGID_XO_DIFF_H2D, len, param);
}
/*
 * 函 数 名  : wal_config_get_xo_diff_data_from_device
 * 1.日    期  : 2023年3月
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_get_xo_diff_data_from_device(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_sync_cmd_common(mac_vap, WLAN_CFGID_GET_XO_DIFF, len, param);
}
/*
 * 函 数 名  : wal_config_spmi_test
 * 1.日    期  : 2023年4月
 *   作    者  : zhangchi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_spmi_test(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_sync_cmd_common(mac_vap, WLAN_CFGID_SPMI_TEST, len, param);
}
/*
 * 函 数 名  : wal_config_ssi_test
 * 1.日    期  : 2023年4月
 *   作    者  : zhangchi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_ssi_test(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
#ifdef PLATFORM_DEBUG_ENABLE
    return hmac_config_ssi_test(mac_vap, len, param);
#else
    return OAL_SUCC;
#endif
}
#ifdef _PRE_WLAN_FEATURE_PWL
/*
 * 函数名:wal_config_set_pwl_enable
 * 功能:设置PWL 是否使能。0：关闭PWL； 1:使能PWL
 */
static uint32_t wal_config_set_pwl_enable(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_set_pwl_enable(mac_vap, len, param);
}

/*
 * 函数名:wal_config_set_pwl_enable
 * 功能:设置PWL预置秘钥
 */
static uint32_t wal_config_set_pwl_pre_key(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    if (hmac_is_pwl_cipher((mac_addkey_param_stru *)param) == OAL_FALSE) {
        oam_error_log0(0, OAM_SF_ANY, "wal_config_set_pwl_pre_key::pwl unsupported cipher");
        return OAL_FAIL;
    }
    return hmac_pwl_add_key(mac_vap, (mac_addkey_param_stru *)param);
}
#endif

/* cfgid操作全局变量 */
OAL_STATIC const wal_wid_op_stru g_ast_board_wid_op[] = {
    /* cfgid                   是否复位mac  保留一字节   get函数              set函数 */
    { WLAN_CFGID_BSS_TYPE,  OAL_TRUE,  { 0 }, wal_config_get_bss_type, wal_config_set_bss_type },
    { WLAN_CFGID_ADD_VAP,   OAL_FALSE, { 0 }, NULL, wal_config_add_vap },
    { WLAN_CFGID_START_VAP, OAL_FALSE, { 0 }, NULL, wal_config_start_vap },
    { WLAN_CFGID_DEL_VAP,   OAL_FALSE, { 0 }, NULL, wal_config_del_vap },
    { WLAN_CFGID_DOWN_VAP,  OAL_FALSE, { 0 }, NULL, wal_config_down_vap },
    { WLAN_CFGID_MODE,      OAL_FALSE, { 0 }, wal_config_get_mode, wal_config_set_mode },
    { WLAN_CFGID_BANDWIDTH, OAL_FALSE, { 0 }, NULL, wal_config_set_bandwidth },

    { WLAN_CFGID_CURRENT_CHANEL, OAL_FALSE, { 0 }, wal_config_get_freq, wal_config_set_freq },
    { WLAN_CFGID_STATION_ID,     OAL_TRUE,  { 0 }, NULL, wal_config_set_mac_addr },
    { WLAN_CFGID_SSID,           OAL_FALSE, { 0 }, wal_config_get_ssid, wal_config_set_ssid },
    { WLAN_CFGID_SHORTGI,        OAL_FALSE, { 0 }, wal_config_get_shortgi20, wal_config_set_shortgi20 },
    { WLAN_CFGID_SHORTGI_FORTY,  OAL_FALSE, { 0 }, wal_config_get_shortgi40, wal_config_set_shortgi40 },
    { WLAN_CFGID_SHORTGI_EIGHTY, OAL_FALSE, { 0 }, wal_config_get_shortgi80, wal_config_set_shortgi80 },

    { WLAN_CFGID_SHORT_PREAMBLE, OAL_FALSE, { 0 }, wal_config_get_shpreamble, wal_config_set_shpreamble },
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    { WLAN_CFGID_SNIFFER, OAL_FALSE, { 0 }, NULL, wal_config_set_sniffer },
#endif
    { WLAN_CFGID_MONITOR_MODE, OAL_FALSE, { 0 }, wal_config_get_monitor_mode, wal_config_set_monitor_mode },
    { WLAN_CFGID_SET_PT_MCAST, OAL_FALSE, { 0 }, NULL, wal_config_set_pt_mcast },
    { WLAN_CFGID_PROT_MODE,       OAL_FALSE, { 0 }, wal_config_get_prot_mode, wal_config_set_prot_mode },
    { WLAN_CFGID_AUTH_MODE,       OAL_FALSE, { 0 }, wal_config_get_auth_mode, wal_config_set_auth_mode },
    { WLAN_CFGID_BEACON_INTERVAL, OAL_FALSE, { 0 }, wal_config_get_bintval, wal_config_set_bintval },
    { WLAN_CFGID_NO_BEACON,       OAL_FALSE, { 0 }, wal_config_get_nobeacon, wal_config_set_nobeacon },
    { WLAN_CFGID_TX_POWER,        OAL_FALSE, { 0 }, wal_config_get_txpower, wal_config_set_txpower },
    { WLAN_CFGID_UAPSD_EN, OAL_FALSE, { 0 }, wal_config_get_uapsd_en, NULL },
    { WLAN_CFGID_DTIM_PERIOD, OAL_FALSE, { 0 }, wal_config_get_dtimperiod, wal_config_set_dtimperiod },

    { WLAN_CFGID_EDCA_TABLE_CWMIN,          OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_EDCA_TABLE_CWMAX,          OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_EDCA_TABLE_AIFSN,          OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT,     OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_EDCA_TABLE_MSDU_LIFETIME,  OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_EDCA_TABLE_MANDATORY,      OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_QEDCA_TABLE_CWMIN,         OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_QEDCA_TABLE_CWMAX,         OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_QEDCA_TABLE_AIFSN,         OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT,    OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME, OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },
    { WLAN_CFGID_QEDCA_TABLE_MANDATORY,     OAL_FALSE, { 0 }, NULL, wal_config_set_wmm_params },

    { WLAN_CFGID_VAP_INFO, OAL_FALSE, { 0 }, NULL, wal_config_vap_info },
    { WLAN_CFGID_BTCOEX_SET_PERF_PARAM, OAL_FALSE, { 0 }, NULL, wal_config_btcoex_set_perf_param },
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    { WLAN_CFGID_NRCOEX_TEST,         OAL_FALSE, { 0 }, NULL, wal_config_nrcoex_test },
    { WLAN_CFGID_QUERY_NRCOEX_STAT,   OAL_FALSE, { 0 }, NULL, wal_config_query_nrcoex_stat },
#endif
    { WLAN_CFGID_CHECK_LTE_GPIO, OAL_FALSE, { 0 }, NULL, wal_config_lte_gpio_mode },

    { WLAN_CFGID_SET_RANDOM_MAC_ADDR_SCAN, OAL_FALSE, { 0 }, NULL, wal_config_set_random_mac_addr_scan },
    { WLAN_CFGID_SET_RANDOM_MAC_OUI,       OAL_FALSE, { 0 }, NULL, wal_config_set_random_mac_oui },
    { WLAN_CFGID_SEND_DISASSOC,            OAL_FALSE, { 0 }, NULL, wal_config_send_disassoc },
    { WLAN_CFGID_ADD_USER,                 OAL_FALSE, { 0 }, NULL, wal_config_add_user },
    { WLAN_CFGID_DEL_USER,                 OAL_FALSE, { 0 }, NULL, wal_config_del_user },
    { WLAN_CFGID_SET_LOG_LEVEL,            OAL_FALSE, { 0 }, NULL, wal_config_set_log_level },
    { WLAN_CFGID_SET_VOWIFI_KEEP_ALIVE, OAL_FALSE, { 0 }, NULL, wal_config_set_vowifi_nat_keep_alive_params },
    { WLAN_CFGID_JOIN_BSSID,       OAL_FALSE, { 0 }, NULL, wal_set_start_join_bssid },
    { WLAN_CFGID_KICK_USER,        OAL_FALSE, { 0 }, NULL, wal_config_kick_user },
    { WLAN_CFGID_COUNTRY,          OAL_FALSE, { 0 }, wal_config_get_country, wal_config_set_country },
    { WLAN_CFGID_COUNTRY_FOR_DFS,  OAL_FALSE, { 0 }, NULL, wal_config_set_country_for_dfs },
    { WLAN_CFGID_TID,              OAL_FALSE, { 0 }, wal_config_get_tid, NULL },

    { WLAN_CFGID_RX_FCS_INFO, OAL_FALSE, { 0 }, NULL, wal_config_rx_fcs_info },
    { WLAN_CFGID_TX_PDET_VAL, OAL_FALSE, { 0 }, NULL, wal_config_get_pdet_info },
#if ((defined(_PRE_WLAN_EXPORT)) && (defined(_PRE_WLAN_NARROW_BAND)))
    { WLAN_CFGID_GET_SW_VERSION, OAL_FALSE, { 0 }, NULL, wal_config_get_sw_version },
#endif
    { WLAN_CFGID_USER_INFO,     OAL_FALSE, { 0 }, NULL, wal_config_user_info },
    { WLAN_CFGID_SET_DSCR,      OAL_FALSE, { 0 }, NULL, wal_config_set_dscr_param },
    { WLAN_CFGID_SET_RATE,      OAL_FALSE, { 0 }, NULL, wal_config_set_rate },
    { WLAN_CFGID_SET_MCS,       OAL_FALSE, { 0 }, NULL, wal_config_set_mcs },
    { WLAN_CFGID_SET_MCSAC,     OAL_FALSE, { 0 }, NULL, wal_config_set_mcsac },
#ifdef _PRE_WLAN_FEATURE_11AX
    { WLAN_CFGID_SET_MCSAX, OAL_FALSE, { 0 }, NULL, _wal_config_set_mcsax },
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    { WLAN_CFGID_SET_MCSAX_ER, OAL_FALSE, { 0 }, NULL, _wal_config_set_mcsax_er },
#endif
    {WLAN_CFGID_SET_11AX_WIFI6_BLACKLIST, OAL_FALSE, { 0 }, NULL, wal_config_set_11ax_wifi6_black_list},
    {WLAN_CFGID_SET_11AX_CLOSE_HTC, OAL_FALSE, { 0 }, NULL, wal_config_set_11ax_close_htc},
#endif
    {WLAN_CFGID_SET_CLEAR_11N_BLACKLIST, OAL_FALSE, { 0 }, NULL, wal_config_clear_11n_blacklist},
    { WALN_CFGID_SET_CHANGE_GO_CHANNEL, OAL_FALSE, { 0 }, NULL, wal_config_set_channge_go_channel },
    { WLAN_CFGID_SET_BW, OAL_FALSE, { 0 }, NULL, wal_config_set_bw },
    { WLAN_CFGID_SET_BAND, OAL_FALSE, { 0 }, NULL, wal_config_set_band },
    { WLAN_CFGID_SET_ALWAYS_TX,          OAL_FALSE, { 0 }, NULL, wal_config_always_tx },
    { WLAN_CFGID_SET_RU_INDEX, OAL_FALSE, { 0 }, NULL, wal_config_ru_index },
    { WLAN_CFGID_SET_ALWAYS_RX, OAL_FALSE, { 0 }, NULL, wal_config_always_rx },
    { WLAN_CFGID_GET_DIEID,     OAL_FALSE, { 0 }, NULL, wal_config_get_dieid },
    { WLAN_CFGID_SET_TX_POW,    OAL_FALSE, { 0 }, NULL, wal_config_set_tx_pow_param },
    { WLAN_CFGID_SET_MEMORY_MONITOR,    OAL_FALSE, { 0 }, NULL, wal_config_memory_monitor },
#ifdef PLATFORM_DEBUG_ENABLE
    { WLAN_CFGID_REG_INFO,      OAL_FALSE, { 0 }, NULL, wal_config_reg_info },
#endif
    { WLAN_CFGID_SDIO_FLOWCTRL, OAL_FALSE, { 0 }, NULL, wal_config_sdio_flowctrl },
#ifdef _PRE_WLAN_DELAY_STATISTIC
    { WLAN_CFGID_PKT_TIME_SWITCH, OAL_FALSE, { 0 }, NULL, wal_config_pkt_time_switch },
#endif
#ifdef PLATFORM_DEBUG_ENABLE
    { WLAN_CFGID_REG_WRITE, OAL_FALSE, { 0 }, NULL, wal_config_reg_write },
#endif
#ifdef _PRE_WLAN_EXPORT
    { WLAN_CFGID_MAC_ADDR_WRITE, OAL_FALSE, { 0 }, NULL, wal_config_mac_addr_write },
    { WLAN_CFGID_MAC_ADDR_READ, OAL_FALSE, { 0 }, NULL, wal_config_mac_addr_read },
#endif
    { WLAN_CFGID_DPD, OAL_FALSE, { 0 }, NULL, wal_config_dpd_cfg },
    { WLAN_CFGID_SCAN_ABORT, OAL_FALSE, { 0 }, NULL, wal_config_scan_abort },
    /* 以下为内核cfg80211配置的命令 */
    { WLAN_CFGID_CFG80211_START_SCHED_SCAN, OAL_FALSE, { 0 }, NULL, wal_config_cfg80211_start_sched_scan },
    { WLAN_CFGID_CFG80211_STOP_SCHED_SCAN,  OAL_FALSE, { 0 }, NULL, wal_config_cfg80211_stop_sched_scan },
    { WLAN_CFGID_CFG80211_START_SCAN,       OAL_FALSE, { 0 }, NULL, wal_config_cfg80211_start_scan },
    { WLAN_CFGID_CFG80211_START_CONNECT,    OAL_FALSE, { 0 }, NULL, wal_config_cfg80211_start_join },
    { WLAN_CFGID_CFG80211_SET_CHANNEL,      OAL_FALSE, { 0 }, NULL, wal_config_set_channel },
    { WLAN_CFGID_CFG80211_SET_MIB_BY_BW,    OAL_FALSE, { 0 }, NULL, wal_config_set_mib_by_bw },
    { WLAN_CFGID_CFG80211_CONFIG_BEACON,    OAL_FALSE, { 0 }, NULL, wal_config_set_beacon },

    { WLAN_CFGID_ADD_KEY,    OAL_FALSE, { 0 }, NULL, wal_config_add_key },
    { WLAN_CFGID_GET_KEY,    OAL_FALSE, { 0 }, NULL, wal_config_get_key },
    { WLAN_CFGID_REMOVE_KEY, OAL_FALSE, { 0 }, NULL, wal_config_remove_key },

    { WLAN_CFGID_ALG_PARAM, OAL_FALSE, { 0 }, NULL, wal_config_alg_param },
    { WLAN_CFGID_ALG_ENTRY, OAL_FALSE, { 0 }, NULL, wal_config_alg_cfg_param_host_entry },

    /* START:开源APP 程序下发的私有命令 */
    { WLAN_CFGID_SET_WPS_IE,         OAL_FALSE, { 0 }, NULL, wal_config_set_wps_ie },
    { WLAN_CFGID_VOWIFI_INFO, OAL_FALSE, { 0 }, NULL, wal_config_vowifi_info },
    { WLAN_CFGID_IP_FILTER, OAL_FALSE, { 0 }, NULL, wal_config_update_ip_filter },

    {WLAN_CFGID_PROBE_PESP_MODE, OAL_FALSE, {0}, NULL, wal_config_set_probe_resp_mode},

    { WLAN_CFGID_FBT_GET_STA_11H_ABILITY, OAL_FALSE, { 0 }, NULL, wal_config_get_sta_11h_abillty },
    { WLAN_CFGID_SET_VENDOR_IE, OAL_FALSE, { 0 }, NULL, wal_config_set_vendor_ie },

    /* END:开源APP 程序下发的私有命令 */
    { WLAN_CFGID_SET_MLME, OAL_FALSE, { 0 }, NULL, wal_config_set_mlme },

    { WLAN_CFGID_WMM_SWITCH,     OAL_FALSE, { 0 }, wal_config_get_wmmswitch, wal_config_open_wmm },
    { WLAN_CFGID_VAP_WMM_SWITCH, OAL_FALSE, { 0 }, wal_config_get_vap_wmm_switch, wal_config_set_vap_wmm_switch },
    { WLAN_CFGID_GET_VERSION,    OAL_FALSE, { 0 }, NULL, wal_config_get_version },
    /* 黑名单配置 */
    { WLAN_CFGID_CLR_BLACK_LIST,      OAL_FALSE, { 0 }, NULL, wal_config_blacklist_clr },
    { WLAN_CFGID_ADD_BLACK_LIST,      OAL_FALSE, { 0 }, NULL, wal_config_blacklist_add },
    { WLAN_CFGID_DEL_BLACK_LIST,      OAL_FALSE, { 0 }, NULL, wal_config_blacklist_del },
    { WLAN_CFGID_BLACKLIST_MODE,      OAL_FALSE, { 0 }, wal_config_get_blacklist_mode, wal_config_set_blacklist_mode },
    { WLAN_CFGID_ADD_BLACK_LIST_ONLY, OAL_FALSE, { 0 }, NULL, wal_config_blacklist_add_only },
    { WLAN_CFGID_CFG80211_SET_PMKSA,   OAL_FALSE, { 0 }, NULL, wal_config_set_pmksa },
    { WLAN_CFGID_CFG80211_DEL_PMKSA,   OAL_FALSE, { 0 }, NULL, wal_config_del_pmksa },
    { WLAN_CFGID_CFG80211_FLUSH_PMKSA, OAL_FALSE, { 0 }, NULL, wal_config_flush_pmksa },
#ifdef _PRE_WLAN_FEATURE_SAE
    { WLAN_CFGID_CFG80211_EXTERNAL_AUTH, OAL_FALSE, { 0 }, NULL, wal_config_external_auth },
#endif /* _PRE_WLAN_FEATURE_SAE */
    { WLAN_CFGID_SET_WPS_P2P_IE,                    OAL_FALSE, { 0 }, NULL, wal_config_set_wps_p2p_ie },
    { WLAN_CFGID_CFG80211_REMAIN_ON_CHANNEL,        OAL_FALSE, { 0 }, NULL, wal_config_remain_on_channel },
    { WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL, OAL_FALSE, { 0 },
        NULL, wal_config_cancel_remain_on_channel },
    { WLAN_CFGID_CFG80211_MGMT_TX,                  OAL_FALSE, { 0 }, NULL, wal_config_mgmt_tx },

    { WLAN_CFGID_QUERY_STATION_STATS, OAL_FALSE, { 0 }, NULL, wal_config_query_station_stats },

    { WLAN_CFGID_QUERY_RSSI, OAL_FALSE, { 0 }, NULL, wal_config_query_rssi },
    { WLAN_CFGID_QUERY_RATE, OAL_FALSE, { 0 }, NULL, wal_config_query_rate },
    { WLAN_CFGID_QUERY_PSST, OAL_FALSE, { 0 }, NULL, wal_config_query_psst },
    { WLAN_CFGID_QUERY_PSM_STAT, OAL_FALSE, { 0 }, NULL, wal_config_query_psm_flt_stat },
#ifdef _PRE_WLAN_DFT_STAT
    { WLAN_CFGID_QUERY_ANI, OAL_FALSE, { 0 }, NULL, wal_config_query_ani },
#endif
    { WLAN_CFGID_SET_STA_PM_ON, OAL_FALSE, { 0 }, NULL, wal_config_set_sta_pm_on },

    {WLAN_CFGID_DC_STATUS,     OAL_FALSE,  {0},   NULL,   wal_config_set_dc_status},
    { WLAN_CFGID_SET_P2P_MIRACAST_STATUS, OAL_FALSE, { 0 }, NULL, wal_config_set_p2p_miracast_status },
    { WLAN_CFGID_SET_P2P_PS_OPS,          OAL_FALSE, { 0 }, NULL, wal_config_set_p2p_ps_ops },
    { WLAN_CFGID_SET_P2P_PS_NOA,          OAL_FALSE, { 0 }, NULL, wal_config_set_p2p_ps_noa },
#ifdef _PRE_WLAN_FEATURE_HS20
    { WLAN_CFGID_SET_QOS_MAP, OAL_FALSE, { 0 }, NULL, wal_config_set_qos_map },
#endif

    { WLAN_CFGID_CFG_VAP_H2D,   OAL_FALSE, { 0 }, NULL, wal_config_cfg_vap_h2d },
    { WLAN_CFGID_HOST_DEV_INIT, OAL_FALSE, { 0 }, NULL, wal_config_host_dev_init },
    { WLAN_CFGID_HOST_DEV_EXIT, OAL_FALSE, { 0 }, NULL, wal_config_host_dev_exit },
    { WLAN_CFGID_SET_MAX_USER, OAL_FALSE, { 0 }, wal_config_get_max_user, wal_config_set_max_user },
    { WLAN_CFGID_GET_STA_LIST, OAL_FALSE, { 0 }, wal_config_get_sta_list, NULL },
#ifdef _PRE_WLAN_FEATURE_11R
    { WLAN_CFGID_SET_FT_IES, OAL_FALSE, { 0 }, NULL, wal_config_set_ft_ies },
#endif  // _PRE_WLAN_FEATURE_11R

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /*  mp13/mp15定制化参数下发接口  */
    { WLAN_CFGID_SET_CUS_DYN_CALI_PARAM, OAL_FALSE, { 0 }, NULL, wal_config_set_cus_dyn_cali },
    { WLAN_CFGID_SET_ALL_LOG_LEVEL,      OAL_FALSE, { 0 }, NULL, wal_config_set_all_log_level },
    { WLAN_CFGID_SET_CUS_RF,             OAL_FALSE, { 0 }, NULL, wal_config_set_cus_rf },
    { WLAN_CFGID_SET_CUS_DTS_CALI,       OAL_FALSE, { 0 }, NULL, wal_config_set_cus_dts_cali },
    { WLAN_CFGID_SET_CUS_NVRAM_PARAM,    OAL_FALSE, { 0 }, NULL, wal_config_set_cus_nvram_params },
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

    { WLAN_CFGID_DESTROY_VAP, OAL_FALSE, { 0 }, NULL, wal_config_vap_destroy },
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    { WLAN_CFGID_REDUCE_SAR, OAL_FALSE, { 0 }, NULL, wal_config_reduce_sar },
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    { WLAN_CFGID_TAS_PWR_CTRL, OAL_FALSE, { 0 }, NULL, wal_config_tas_pwr_ctrl },
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    { WLAN_CFGID_TAS_RSSI_ACCESS, OAL_FALSE, { 0 }, NULL, wal_config_tas_rssi_access },
#endif

#ifdef _PRE_WLAN_FEATURE_11K
    { WLAN_CFGID_SEND_NEIGHBOR_REQ, OAL_FALSE, { 0 }, NULL, wal_config_send_neighbor_req },
    { WLAN_CFGID_11K_SCAN_FAIL_RESTART_ROAM, OAL_FALSE, { 0 }, NULL, wal_config_11k_scan_fail_restart_roam },
#endif

    { WLAN_CFGID_VENDOR_CMD_GET_CHANNEL_LIST, OAL_FALSE, { 0 }, wal_config_vendor_cmd_get_channel_list, NULL },

#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
    { WLAN_CFGID_SET_LP_MIRACAST, OAL_FALSE, { 0 }, NULL, wal_chip_set_lp_miracast },
    { WLAN_CFGID_GET_M2S_STATE, OAL_FALSE, { 0 }, NULL, wal_config_get_m2s_state },
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#ifdef _PRE_WLAN_FEATURE_M2S_MODEM
    { WLAN_CFGID_SET_M2S_MODEM, OAL_FALSE, { 0 }, NULL, wal_config_set_m2s_switch_modem },
#endif
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifndef CONFIG_HAS_EARLYSUSPEND
    { WLAN_CFGID_SET_SUSPEND_MODE, OAL_FALSE, { 0 }, NULL, wal_config_set_suspend_mode },
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_APF
    { WLAN_CFGID_SET_APF_FILTER, OAL_FALSE, { 0 }, NULL, wal_config_apf_filter_cmd },
#endif
    { WLAN_CFGID_FORCE_STOP_FILTER, OAL_FALSE, { 0 }, NULL, wal_config_force_stop_filter },
    { WLAN_CFGID_SOFTAP_MIMO_MODE, OAL_FALSE, { 0 }, NULL, wal_config_softap_mimo_mode },
#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    { WLAN_CFGID_RX_LISTEN_PS_SWITCH, OAL_FALSE, { 0 }, NULL, wal_config_set_rx_listen_ps_switch},
#endif
#ifdef _PRE_WLAN_FEATURE_NAN
    { WLAN_CFGID_NAN, OAL_FALSE, { 0 }, NULL, wal_config_nan },
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    { WLAN_CFGID_HID2D_ACS_MODE, OAL_FALSE, { 0 }, NULL, wal_config_hid2d_acs_mode},
    { WLAN_CFGID_HID2D_ACS_STATE, OAL_FALSE, { 0 }, NULL, wal_config_hid2d_acs_state},
    { WLAN_CFGID_HID2D_LINK_MEAS, OAL_FALSE, { 0 }, NULL, wal_config_hid2d_link_meas},
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
    { WLAN_CFGID_HID2D_SWITCH_CHAN, OAL_FALSE, { 0 }, NULL, wal_config_switch_channel},
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    { WLAN_CFGID_RTT_CONFIG, OAL_FALSE, { 0 }, NULL, wal_config_wifi_rtt_config },
#endif
    { WLAN_CFGID_QUERY_TSF, OAL_FALSE, { 0 }, NULL, wal_config_get_sync_tsf },
#ifdef _PRE_WLAN_CHBA_MGMT
    { WLAN_CFGID_START_CHBA, OAL_FALSE, { 0 }, NULL, wal_config_start_chba },
    { WLAN_CFGID_CHBA_CONN_NOTIFY, OAL_FALSE, { 0 }, NULL, wal_config_chba_conn_ntf},
    { WLAN_CFGID_CHBA_CHAN_SWITCH_TEST_CMD, OAL_FALSE, { 0 }, NULL, wal_config_chba_chan_switch_cmd },
    { WLAN_CFGID_CHBA_LOG_LEVEL, OAL_FALSE, { 0 }, NULL, wal_config_chba_log_level },
    { WLAN_CFGID_CHBA_SET_PS_THRES, OAL_FALSE, { 0 }, NULL, wal_config_chba_set_ps_thres },
    { WLAN_CFGID_CHBA_MODULE_INIT, OAL_FALSE, { 0 }, NULL, wal_config_chba_module_init },
    { WLAN_CFGID_CHBA_AUTO_BITMAP_CMD, OAL_FALSE, { 0 }, NULL, wal_config_chba_auto_bitmap_debug },
    { WLAN_CFGID_CHBA_SET_USER_BITMAP_CMD, OAL_FALSE, { 0 }, NULL, wal_config_chba_set_user_bitmap_debug },
    { WLAN_CFGID_CHBA_SET_VAP_BITMAP_CMD, OAL_FALSE, { 0 }, NULL, wal_config_chba_set_vap_bitmap_debug },
    { WLAN_CFGID_CHBA_ADJUST_ISLAND_CHAN, OAL_FALSE, { 0 }, NULL, wal_config_chba_island_chan_switch },
    { WLAN_CFGID_CHBA_FEATURE_SWITCH, OAL_FALSE, { 0 }, NULL, wal_config_chba_feature_switch },
    { WLAN_CFGID_CHBA_SET_BATTERY, OAL_FALSE, { 0 }, NULL, wal_config_chba_set_battery },
    { WLAN_CFGID_CHBA_SET_COEX_CHAN_INFO, OAL_FALSE, { 0 }, NULL, wal_config_chba_set_coex_chan },
#endif
    { WLAN_CFGID_SET_SAE_PWE, OAL_FALSE, { 0 }, NULL, wal_config_set_sae_pwe },
    { WLAN_CFGID_QUERY_TID_QUEUE, OAL_FALSE, { 0 }, NULL, wal_config_query_tid_queue },
    { WLAN_CFGID_SAVE_DPD_DATA, OAL_FALSE, {0}, NULL, wal_config_dpd_cali_factory },
    { WLAN_CFGID_SAR_SLIDE_DEBUG, OAL_FALSE, {0}, NULL, wal_config_sar_slide_cfg },
    { WLAN_CFGID_COMPETE_ASCEND, OAL_FALSE, {0}, NULL, wal_config_compete_pow_cfg },
#ifdef PLATFORM_DEBUG_ENABLE
    { WLAN_CFGID_GET_TEMPERATURE, OAL_FALSE, { 0 }, NULL, wal_config_get_tsensor_val },
    { WLAN_CFGID_GET_HW_WORD, OAL_FALSE, { 0 }, NULL, wal_config_get_config_word },
    { WLAN_CFGID_GET_BOARD_SN, OAL_FALSE, { 0 }, NULL, wal_config_get_board_sn },
    { WLAN_CFGID_SET_BOARD_SN, OAL_FALSE, { 0 }, NULL, wal_config_set_board_sn },
    { WLAN_CFGID_FLASH_TEST, OAL_FALSE, { 0 }, NULL, wal_config_flash_test },
#endif
    { WLAN_CFGID_TX_POW_H2D, OAL_FALSE, {0}, NULL, wal_config_send_tx_pow_data_to_device },
    { WLAN_CFGID_GET_TX_POW, OAL_FALSE, {0}, NULL, wal_config_get_tx_pow_data_from_device},
    { WLAN_CFGID_XO_DIFF_H2D, OAL_FALSE, {0}, NULL, wal_config_send_xo_diff_to_device },
    { WLAN_CFGID_GET_XO_DIFF, OAL_FALSE, {0}, NULL, wal_config_get_xo_diff_data_from_device},
#ifdef _PRE_WLAN_FEATURE_PWL
    { WLAN_CFGID_SET_PWL_ENABLE, OAL_FALSE, { 0 }, NULL, wal_config_set_pwl_enable },
    { WLAN_CFGID_SET_PWL_PRE_KEY, OAL_FALSE, { 0 }, NULL, wal_config_set_pwl_pre_key },
#endif
    { WLAN_CFGID_SPMI_TEST, OAL_FALSE, {0}, NULL, wal_config_spmi_test},
    { WLAN_CFGID_SSI_TEST, OAL_FALSE, {0}, NULL, wal_config_ssi_test},
    { WLAN_CFGID_BUTT, OAL_FALSE, { 0 }, 0, 0 },
};

OAL_STATIC wal_wid_op_stru *wal_config_get_wid_map(wal_wid_op_stru *pst_wid_map, uint16_t en_wid, uint32_t wid_nums)
{
    uint16_t us_cfgid;
    wal_wid_op_stru *pst_current_wid = NULL;

    for (us_cfgid = 0; us_cfgid < wid_nums; us_cfgid++) {
        pst_current_wid = pst_wid_map + us_cfgid;
        if (pst_current_wid->en_cfgid == en_wid) {
            return pst_current_wid;
        }
    }

    return NULL;
}

/*
 * 函 数 名  : wal_config_process_query_param_check
 * 功能描述  : 处理配置查询请求
 * 1.日    期  : 2019年12月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_process_query_param_check(mac_vap_stru *pst_mac_vap, uint8_t *puc_req_msg,
    uint8_t *puc_rsp_msg, uint8_t *puc_rsp_msg_len)
{
    if (oal_any_null_ptr4(pst_mac_vap, puc_req_msg, puc_rsp_msg, puc_rsp_msg_len)) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_config_process_query_param_check::mac_vap/req_msg/rsp_msg/rsp_msg_len is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
    /* ,会和pc s3 删除vap撞上，增加保护 */
    if (pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_config_process_query_param_check::pst_mac_vap is deleted!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_config_process_wid_get_func(wal_wid_op_stru *current_wid, mac_vap_stru *mac_vap,
    wal_msg_write_stru *rsp_msg, wal_msg_query_stru *query_msg, uint16_t *len)
{
    uint32_t ret;
    /* 异常情况，cfgid对应的get函数为空 */
    if (current_wid->p_get_func == NULL) {
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_config_process_wid_get_func:: get_func ptr is null, wid is %d!}\r\n",
                       query_msg->en_wid);
        return OAL_SUCC_GO_ON;
    }

    ret = current_wid->p_get_func(mac_vap, len, rsp_msg->auc_value);
    if (ret != OAL_SUCC) {
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_process_wid_get_func:: func return no SUCC. wid and ret value is:%d, %d!}\r\n",
                         query_msg->en_wid, ret);
        return OAL_SUCC_GO_ON;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_config_process_query
 * 功能描述  : 处理配置查询请求
 * 1.日    期  : 2012年11月9日
  *   修改内容  : 新生成函数
 */
uint32_t wal_config_process_query(mac_vap_stru *pst_mac_vap, uint8_t *puc_req_msg, uint16_t us_req_msg_len,
    uint8_t *puc_rsp_msg, uint8_t *puc_rsp_msg_len)
{
    uint16_t us_req_idx = 0; /* 请求消息索引 */
    uint16_t us_rsp_idx = 0; /* 返回消息索引 */
    uint16_t us_len = 0;     /* WID对应返回值的长度 */
    wal_msg_query_stru *pst_query_msg = NULL;
    wal_msg_write_stru *pst_rsp_msg = NULL;
    uint32_t ret;
    wal_wid_op_stru *pst_current_wid = NULL;

    ret = wal_config_process_query_param_check(pst_mac_vap, puc_req_msg, puc_rsp_msg, puc_rsp_msg_len);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_config_process_query::param error!}\r\n");
        return ret;
    }

    /* 查询消息格式如下:                                                     */
    /* +-------------------------------------------------------------------+ */
    /* | WID0          | WID1         | WID2         | ................... | */
    /* +-------------------------------------------------------------------+ */
    /* |     2 Bytes   |    2 Bytes   |    2 Bytes   | ................... | */
    /* +-------------------------------------------------------------------+ */
    /* 返回消息格式如下:                                                     */
    /* +-------------------------------------------------------------------+ */
    /* | WID0      | WID0 Length | WID0 Value  | ......................... | */
    /* +-------------------------------------------------------------------+ */
    /* | 2 Bytes   | 2 Byte      | WID Length  | ......................... | */
    /* +-------------------------------------------------------------------+ */
    while (us_req_idx < us_req_msg_len) {
        /* 从查询消息中得到一个WID值   */
        pst_query_msg = (wal_msg_query_stru *)(&puc_req_msg[us_req_idx]);
        us_req_idx += WAL_MSG_WID_LENGTH; /* 指向下一个WID */

        /* 获取返回消息内存 */
        pst_rsp_msg = (wal_msg_write_stru *)(&puc_rsp_msg[us_rsp_idx]);

        pst_current_wid = wal_config_get_wid_map((wal_wid_op_stru *)g_ast_board_wid_op,
                                                 pst_query_msg->en_wid, oal_array_size(g_ast_board_wid_op));
        if (pst_current_wid == NULL) {
#ifdef _PRE_WLAN_CFGID_DEBUG
            pst_current_wid = wal_config_get_wid_map((wal_wid_op_stru *)g_ast_board_wid_op_debug,
                                                     pst_query_msg->en_wid, wal_config_get_debug_wid_arrysize());
            if (pst_current_wid == NULL) {
                oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                               "{wal_config_process_query::cfgid not invalid %d!}\r\n", pst_query_msg->en_wid);
                continue;
            }
            /* else, call the cfgid func. */
#else
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                           "{wal_config_process_query::cfgid not invalid %d!}\r\n", pst_query_msg->en_wid);
            continue;
#endif
        }

        if (wal_config_process_wid_get_func(pst_current_wid, pst_mac_vap, pst_rsp_msg,
                                            pst_query_msg, &us_len) == OAL_SUCC_GO_ON) {
            continue;
        }

        pst_rsp_msg->en_wid = pst_query_msg->en_wid; /* 设置返回消息的WID */
        pst_rsp_msg->us_len = us_len;

        us_rsp_idx += us_len + WAL_MSG_WRITE_MSG_HDR_LENGTH; /* 消息体的长度 再加上消息头的长度 */

        /* 消息Response 接口容易让调用者使用超过消息数组空间长度，
          这里需要加判断，检查长度和狗牌，后续需要整改 */
        if (oal_unlikely(us_rsp_idx + sizeof(wal_msg_hdr_stru) > HMAC_RSP_MSG_MAX_LEN)) {
            oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                           "{wal_config_process_query::us_cfgid:%d reponse msg len:%u over limit:%u}",
                           pst_current_wid->en_cfgid, us_rsp_idx + sizeof(wal_msg_hdr_stru), HMAC_RSP_MSG_MAX_LEN);
        }
    }

    *puc_rsp_msg_len = (uint8_t)us_rsp_idx;

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_config_process_write_param_check
 * 功能描述  : 处理配置设置请求
 * 1.日    期  : 2019年12月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_config_process_write_param_check(mac_vap_stru *pst_mac_vap, uint8_t *puc_req_msg)
{
    if (oal_unlikely(oal_any_null_ptr2(pst_mac_vap, puc_req_msg))) {
        oam_error_log0(0, OAM_SF_ANY,
                       "{wal_config_process_write_param_check::pst_mac_vap/puc_req_msg is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
    /* ,会和pc s3 删除vap撞上，增加保护 */
    if (pst_mac_vap->uc_init_flag == MAC_VAP_INVAILD) {
        oam_warning_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_config_process_write_param_check::pst_mac_vap is deleted!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_config_get_wid(wal_wid_op_stru **current_wid, wal_msg_write_stru *write_msg,
    mac_vap_stru *mac_vap)
{
    /* 寻找cfgid 对应的write函数 */
    *current_wid = wal_config_get_wid_map((wal_wid_op_stru *)g_ast_board_wid_op,
                                          write_msg->en_wid, oal_array_size(g_ast_board_wid_op));
    if (*current_wid == NULL) {
#ifdef _PRE_WLAN_CFGID_DEBUG
        *current_wid = wal_config_get_wid_map((wal_wid_op_stru *)g_ast_board_wid_op_debug,
                                              write_msg->en_wid, wal_config_get_debug_wid_arrysize());
        if (*current_wid == NULL) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                           "{wal_config_get_wid::cfgid not invalid %d!}\r\n", write_msg->en_wid);
            return OAL_SUCC_GO_ON;
        }
        /* else, go on call the cfgid func. */
#else
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_config_get_wid::cfgid not invalid %d!}\r\n", write_msg->en_wid);
        return OAL_SUCC_GO_ON;
#endif
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_config_process_write
 * 功能描述  : 处理配置设置请求
 * 1.日    期  : 2012年11月13日
  *   修改内容  : 新生成函数
 */
uint32_t wal_config_process_write(mac_vap_stru *pst_mac_vap, uint8_t *puc_req_msg, uint16_t us_msg_len,
    uint8_t *puc_rsp_msg, uint8_t *puc_rsp_msg_len)
{
    uint16_t us_req_idx = 0;
    uint16_t us_rsp_idx = 0;
    wal_msg_write_stru *pst_write_msg = NULL;
    wal_msg_write_rsp_stru *pst_rsp_msg = NULL;
    uint32_t ret;
    wal_wid_op_stru *pst_current_wid = NULL;

    ret = wal_config_process_write_param_check(pst_mac_vap, puc_req_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_process_write::param error!}\r\n");
        return ret;
    }

    /* 设置消息的格式如下:                                                   */
    /* +-------------------------------------------------------------------+ */
    /* | WID0      | WID0 Length | WID0 Value  | ......................... | */
    /* +-------------------------------------------------------------------+ */
    /* | 2 Bytes   | 2 Byte      | WID Length  | ......................... | */
    /* +-------------------------------------------------------------------+ */
    /* 返回消息的格式如下:                                                   */
    /* +-------------------------------------------------------------------+ */
    /* | WID0     | resv    | WID0 错误码 |  WID1   | resv | WID1错误码 |  | */
    /* +-------------------------------------------------------------------+ */
    /* | 2 Bytes  | 2 Bytes | 4 Byte      | 2 Bytes | 2 B  |  4 Bytes   |  | */
    /* +-------------------------------------------------------------------+ */
    while (us_req_idx < us_msg_len) {
        /* 获取一个设置WID消息   */
        pst_write_msg = (wal_msg_write_stru *)(&puc_req_msg[us_req_idx]);

        /* 获取返回消息内存 */
        pst_rsp_msg = (wal_msg_write_rsp_stru *)(&puc_rsp_msg[us_rsp_idx]);

        us_req_idx += pst_write_msg->us_len + WAL_MSG_WRITE_MSG_HDR_LENGTH; /* 指向下一个WID设置消息 */

        if (wal_config_get_wid(&pst_current_wid, pst_write_msg, pst_mac_vap) == OAL_SUCC_GO_ON) {
            continue;
        }

        /* 异常情况，cfgid对应的set函数为空 */
        if (pst_current_wid->p_set_func == NULL) {
            oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                           "{wal_config_process_write:: get_func ptr is null, wid is %d!}\r\n",
                           pst_write_msg->en_wid);
            continue;
        }

        ret = pst_current_wid->p_set_func(pst_mac_vap, pst_write_msg->us_len, pst_write_msg->auc_value);

        /* 将返回错误码设置到rsp消息中 */
        pst_rsp_msg->en_wid = pst_write_msg->en_wid;
        pst_rsp_msg->err_code = ret;
        us_rsp_idx += sizeof(wal_msg_write_rsp_stru);
        /* 消息Response 接口容易让调用者使用超过消息数组空间长度，
          这里需要加判断，检查长度和狗牌，后续需要整改 */
        if (oal_unlikely(us_rsp_idx + sizeof(wal_msg_hdr_stru) > HMAC_RSP_MSG_MAX_LEN)) {
            oam_error_log3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                           "{wal_config_process_write::us_cfgid:%d reponse msg len:%u over limit:%u}",
                           pst_current_wid->en_cfgid,
                           us_rsp_idx + sizeof(wal_msg_hdr_stru), HMAC_RSP_MSG_MAX_LEN);
        }

        if (ret != OAL_SUCC) {
            oam_warning_log2(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                             "{wal_config_process_write::func return no SUCC. wid and ret value is %d, %d!}\r\n",
                             pst_write_msg->en_wid, ret);
        }
    }

    *puc_rsp_msg_len = (uint8_t)us_rsp_idx;

    return OAL_SUCC;
}
