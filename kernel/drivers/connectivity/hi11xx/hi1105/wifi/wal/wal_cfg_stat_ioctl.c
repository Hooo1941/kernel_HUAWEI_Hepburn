/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 各种状态查询命令
 * 创建日期 : 2022年11月19日
 */
#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "oal_kernel_file.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "wlan_chip_i.h"
#include "mac_vap.h"
#include "mac_resource.h"
#include "hmac_resource.h"
#include "hmac_ext_if.h"
#include "hmac_arp_offload.h"
#include "hmac_roam_main.h"
#include "hmac_pm.h"
#include "wal_config_alg_cfg.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#endif
#include "wal_linux_atcmdsrv.h"
#if defined(WIN32)
#include "hisi_customize_wifi.h"
#endif
#include "securec.h"
#include "securectype.h"
#include "oam_stat_wifi.h"
#include "oam_event_wifi.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_STAT_CFG_IOCTL_C

#ifdef WIFI_DEBUG_ENABLE

/*
 * 功能描述  : 打印执行内存池信息
 * 1.日    期  : 2020年7月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_mem_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_mem_pool_id_enum_uint8 pool_id;
    /* 获取内存池ID */
    pool_id = (oal_mem_pool_id_enum_uint8)params[0];

    /* 打印内存池信息 */
    oal_mem_info(pool_id);

    return OAL_SUCC;
}
/*
 * 功能描述  : 获取所有维测统计信息
 * 1.日    期  : 2020年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_show_stat_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    oam_stats_report_info_to_sdt(OAM_OTA_TYPE_DEV_STAT_INFO, params);
    oam_stats_report_info_to_sdt(OAM_OTA_TYPE_VAP_STAT_INFO, params);
    return OAL_SUCC;
}
/*
 * 功能描述  : 将所有的统计信息清零
 * 1.日    期  : 2020年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_clear_stat_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    oam_stats_clear_stat_info();
    return OAL_SUCC;
}
/*
 * 功能描述  : 上报某一个user的统计信息
 * 1.日    期  : 2020年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_user_stat_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    oam_stats_report_info_to_sdt(OAM_OTA_TYPE_USER_STAT_INFO, params);
    return OAL_SUCC;
}
/*
 * 功能描述  : 查询事件队列的信息，上报给SDT
 * 1.日    期  : 2020年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_event_queue_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    return frw_event_queue_info();
}
#ifdef _PRE_WLAN_DFT_STAT
/*
 * 功能描述  : 将VAP的统计信息清零
 * 1.日    期  : 2020年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_clear_vap_stat_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    oam_stats_clear_vap_stat_info(mac_vap->uc_vap_id);
    return OAL_SUCC;
}
#endif
/*
 * 功能描述  : 获取流控相关状态信息
 * 1.日    期  : 2020年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_get_flowctl_stat(mac_vap_stru *mac_vap, uint32_t *params)
{
    /* 设置host flowctl 相关参数 */
    hcc_host_get_flowctl_stat(HCC_EP_WIFI_DEV);
    return OAL_SUCC;
}
/*
 * 功能描述  : 打印AP侧关联的STA信息
 * 1.日    期  : 2020年8月5日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_list_sta(mac_vap_stru *mac_vap, uint32_t *params)
{
    return hmac_config_list_sta(mac_vap);
}
/*
 * 功能描述  : 黑名单信息打印
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_blacklist_show(mac_vap_stru *mac_vap, uint32_t *params)
{
    if ((mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP) && (mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_set_blacklist_show::not valid vap mode=%d!}\r\n",
                         mac_vap->en_vap_mode);
        return OAL_SUCC;
    }
    hmac_show_blacklist_info(mac_vap);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
/*
 * 功能描述  : RR性能检测设置打印信息的开关
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_show_rr_time_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_bool_enum_uint8 switch_state = (oal_bool_enum_uint8)params[0];
    g_host_rr_perform_timestamp.rr_switch = (switch_state == OAL_TRUE) ? OAL_SWITCH_ON : OAL_SWITCH_OFF;
    return OAL_SUCC;
}
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*
 * 功能描述  : 打印定制化信息
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_dev_customize_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    wlan_chip_show_customize_info();
    return OAL_SUCC;
}
#endif
/*
 * 功能描述  : 打印定制化信息
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_show_cali_data_info(mac_vap_stru *mac_vap, uint32_t *params)
{
    wlan_chip_show_cali_data_info();
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置所有用户帧上报的所有开关，如果是1，则上报所有类型帧的帧内容，
 *             cb字段，描述符；如果是0，则什么都不上报
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_all_ota(mac_vap_stru *mac_vap, uint32_t *params)
{
    oal_switch_enum_uint8 all_ota_switch = (oal_switch_enum_uint8)params[0];

    oam_report_set_all_switch(all_ota_switch);
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_vap_info
 * 功能描述  : 打印vap的所有参数信息
 * 1.日    期  : 2013年5月28日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_user_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    mac_vap_stru *mac_vap = NULL;
    wal_msg_write_stru write_msg = {0};
    int32_t l_ret;
    mac_cfg_user_info_param_stru *pst_user_info_param = NULL;
    uint8_t auc_mac_addr[NUM_6_BITS] = { 0 }; /* 临时保存获取的use的mac地址信息 */
    uint16_t us_user_idx;
    uint32_t offset, ret;

    /* 获取mac地址 */
    ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_user_info::wal_hipriv_get_mac_addr fail!}");
        return ret;
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_USER_INFO, sizeof(mac_cfg_user_info_param_stru));

    /* 根据mac地址找用户 */
    mac_vap = oal_net_dev_priv(net_dev);

    ret = mac_vap_find_user_by_macaddr(mac_vap, auc_mac_addr, &us_user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_user_info::no such user!}");
        return OAL_FAIL;
    }

    /* 设置配置命令参数 */
    pst_user_info_param = (mac_cfg_user_info_param_stru *)(write_msg.auc_value);
    pst_user_info_param->us_user_idx = us_user_idx;

    l_ret = wal_send_cfg_event(net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_user_info_param_stru),
                               (uint8_t *)&write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_user_info::err [%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置以太网上报开关
 * 1.日    期  : 2014年5月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_ether_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_ret;
    uint32_t ret;
    mac_cfg_eth_switch_param_stru st_eth_switch_param;

    /* "vap0 ether_switch user_macaddr oam_ota_frame_direction_type_enum(帧方向) 0|1(开关)" */
    memset_s(&st_eth_switch_param, sizeof(mac_cfg_eth_switch_param_stru),
             0, sizeof(mac_cfg_eth_switch_param_stru));

    /* 获取mac地址 */
    ret = wal_hipriv_get_mac_addr(pc_param, st_eth_switch_param.auc_user_macaddr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ether_switch::wal_hipriv_get_mac_addr return err_code[%d]}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 获取以太网帧方向 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ether_switch::wal_get_cmd_one_arg return err_code[%d]}\r\n", ret);
        return ret;
    }
    pc_param += off_set;
    st_eth_switch_param.en_frame_direction = (uint8_t)oal_atoi(ac_name);

    /* 获取开关 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ether_switch::wal_get_cmd_one_arg return err_code[%d]}\r\n", ret);
        return ret;
    }
    st_eth_switch_param.en_switch = (uint8_t)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ETH_SWITCH, sizeof(st_eth_switch_param));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_eth_switch_param, sizeof(st_eth_switch_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_eth_switch_param),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

void wal_get_ucast_sub_switch(mac_cfg_80211_ucast_switch_stru *mac_80211_ucast_switch,
    int8_t *param, uint32_t len)
{
    uint32_t ret;
    uint32_t off_set;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取MSDU描述符打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get msdu dscr switch return err_code[%d]!}\r\n", ret);
        return;
    }
    param += off_set;
    mac_80211_ucast_switch->sub_switch.stru.bit_msdu_dscr = (uint8_t)oal_atoi(name) ? OAL_TRUE : OAL_FALSE;

    /* 获取BA_INFO打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get ba info switch return err_code[%d]!}\r\n", ret);
        return;
    }
    param += off_set;
    mac_80211_ucast_switch->sub_switch.stru.bit_ba_info = (uint8_t)oal_atoi(name) ? OAL_TRUE : OAL_FALSE;

    /* 获取HIMIT描述符打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get himit dscr switch return err_code[%d]!}\r\n", ret);
        return;
    }
    param += off_set;
    mac_80211_ucast_switch->sub_switch.stru.bit_himit_dscr = (uint8_t)oal_atoi(name) ? OAL_TRUE : OAL_FALSE;

    /* 获取MU描述符打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get mu dscr switch return err_code[%d]!}\r\n", ret);
        return;
    }
    param += off_set;
    mac_80211_ucast_switch->sub_switch.stru.bit_mu_dscr = (uint8_t)oal_atoi(name) ? OAL_TRUE : OAL_FALSE;
}

uint32_t wal_get_ucast_switch(mac_cfg_80211_ucast_switch_stru *mac_80211_ucast_switch,
    int8_t *param, uint32_t len)
{
    uint32_t ret;
    uint32_t off_set;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取80211帧方向 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get 80211 ucast frame direction return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_frame_direction = (uint8_t)oal_atoi(name);

    /* 获取帧类型 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get ucast frame type return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_frame_type = (uint8_t)oal_atoi(name);

    /* 获取帧内容打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get frame content switch return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_frame_switch = (uint8_t)oal_atoi(name) ? OAL_SWITCH_ON : OAL_SWITCH_OFF;

    /* 获取帧CB字段打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get frame cb switch return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_cb_switch = (uint8_t)oal_atoi(name) ? OAL_SWITCH_ON : OAL_SWITCH_OFF;

    /* 获取描述符打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get frame dscr switch return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_dscr_switch = (uint8_t)oal_atoi(name) ? OAL_SWITCH_ON : OAL_SWITCH_OFF;

    wal_get_ucast_sub_switch(mac_80211_ucast_switch, param, OAL_STRLEN(param));

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置80211单播帧上报开关
 * 1.日    期  : 2014年5月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_80211_ucast_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int32_t l_ret;
    uint32_t ret;
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;

    /* sh hipriv.sh "vap0 80211_uc_switch user_macaddr 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
     *  0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关) 0|1(MSDU描述符) 0|1(BA_INFO) 0|1(HIMIT描述符) 0|1(MU描述符)"
    */
    memset_s(&st_80211_ucast_switch, sizeof(mac_cfg_80211_ucast_switch_stru),
             0, sizeof(mac_cfg_80211_ucast_switch_stru));

    /* 获取mac地址 */
    ret = wal_hipriv_get_mac_addr(pc_param, st_80211_ucast_switch.auc_user_macaddr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_80211_ucast_switch::wal_hipriv_get_mac_addr return err_code[%d]}", ret);
        return ret;
    }
    pc_param += off_set;

    ret = wal_get_ucast_switch(&st_80211_ucast_switch, pc_param, OAL_STRLEN(pc_param));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::get ucast switch fail [%d]!}", ret);
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_80211_UCAST_SWITCH, sizeof(st_80211_ucast_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_80211_ucast_switch, sizeof(st_80211_ucast_switch)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_80211_ucast_switch),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
/*
 * 功能描述  : 设置所有用户的80211单播帧上报开关，此开关设置以后，所有用户上报策略相同
 * 1.日    期  : 2014年5月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_all_80211_ucast(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t ret;
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    /* sh hipriv.sh "Hisilicon0 80211_uc_all 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
     *  0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关) 0|1(MSDU描述符) 0|1(BA_INFO) 0|1(HIMIT描述符) 0|1(MU描述符)"
    */
    memset_s(&st_80211_ucast_switch, sizeof(mac_cfg_80211_ucast_switch_stru),
             0, sizeof(mac_cfg_80211_ucast_switch_stru));
    ret = wal_get_ucast_switch(&st_80211_ucast_switch, pc_param, OAL_STRLEN(pc_param));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_all_80211_ucast::get ucast switch fail [%d]!}", ret);
        return ret;
    }

    /* 设置广播mac地址 */
    memcpy_s(st_80211_ucast_switch.auc_user_macaddr, WLAN_MAC_ADDR_LEN, BROADCAST_MACADDR, WLAN_MAC_ADDR_LEN);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_80211_UCAST_SWITCH, sizeof(st_80211_ucast_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_80211_ucast_switch, sizeof(st_80211_ucast_switch)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_all_80211_ucast::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_80211_ucast_switch),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_all_ucast_switch::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_DFT_STAT
/*
 * 功能描述  : 上报或者清零用户队列统计信息，包括tid队列，节能队列
 * 1.日    期  : 2014年8月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_usr_queue_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    mac_cfg_usr_queue_param_stru st_usr_queue_param;

    /* sh hipriv.sh "vap_name usr_queue_stat XX:XX:XX:XX:XX:XX 0|1" */
    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    memset_s((uint8_t *)&st_usr_queue_param, sizeof(st_usr_queue_param), 0, sizeof(st_usr_queue_param));

    /* 获取用户mac地址 */
    ret = wal_hipriv_get_mac_addr(pc_param, st_usr_queue_param.auc_user_macaddr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_usr_queue_stat::wal_hipriv_get_mac_addr return [%d].}", ret);
        return ret;
    }
    pc_param += off_set;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_usr_queue_stat::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    st_usr_queue_param.uc_param = (uint8_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_USR_QUEUE_STAT, sizeof(st_usr_queue_param));

    /* 填写消息体，参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 &st_usr_queue_param, sizeof(st_usr_queue_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_usr_queue_stat::memcpy fail!}");
        return OAL_FAIL;
    }

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_usr_queue_param),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_usr_queue_stat::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
/*
 * 功能描述  : 上报或者清零所有维测统计信息
 * 1.日    期  : 2014年8月20日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_report_all_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    uint16_t us_len;

    /* sh hipriv.sh "Hisilicon0 reprt_all_stat type(phy/machw/mgmt/irq/all)  0|1" */
    /* 获取repot类型 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
        pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_report_all_stat::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';
    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ALL_STAT, us_len);

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_report_all_stat::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_GET_STATION_INFO_EXT
OAL_STATIC void wal_hipriv_printf_station_info(hmac_vap_stru *hmac_vap)
{
#ifdef CONFIG_HW_GET_EXT_SIG_ULDELAY
    oam_warning_log4(0, OAM_SF_ANY,
        "{wal_hipriv_get_station_info::tid delay[%d]ms, tx drop[%d], tx pkts[%d], tx bytes[%d]}",
        hmac_vap->station_info.ul_delay,
        hmac_vap->station_info.tx_failed,
        hmac_vap->station_info.tx_packets,
        hmac_vap->station_info.tx_bytes);
#else
    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_hipriv_get_station_info::tx drop[%d], tx pkts[%d], tx bytes[%d]}",
        hmac_vap->station_info.tx_failed,
        hmac_vap->station_info.tx_packets,
        hmac_vap->station_info.tx_bytes);
#endif
    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_hipriv_get_station_info::rx pkts[%lu], rx bytes[%lu], rx drop[%lu]}",
        hmac_vap->station_info.rx_packets,
        hmac_vap->station_info.rx_bytes,
        hmac_vap->station_info.rx_dropped_misc);
}

uint32_t wal_hipriv_get_station_info(oal_net_device_stru *net_dev, int8_t *param)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    dmac_query_request_event dmac_query_request_event;
    dmac_query_station_info_request_event *query_station_info = NULL;
    wal_msg_write_stru write_msg;
    signed long leftime;
    int32_t ret;
    uint8_t vap_id;
    if (oal_any_null_ptr2(net_dev, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_get_station_info::OAL_NET_DEV_PRIV, return null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    vap_id = mac_vap->uc_vap_id;

    query_station_info = (dmac_query_station_info_request_event *)&dmac_query_request_event;
    query_station_info->query_event = OAL_QUERY_STATION_INFO_EVENT;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    /********************************************************************************
        抛事件到wal层处理 ，对于低功耗需要做额外处理，不能像下层抛事件，直接起定时器
        低功耗会在接收beacon帧的时候主动上报信息。
    ********************************************************************************/
    /* 3.1 填写 msg 消息头 */
    write_msg.en_wid = WLAN_CFGID_QUERY_STATION_STATS;
    write_msg.us_len = sizeof(dmac_query_request_event);

    /* 3.2 填写 msg 消息体 */
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
        query_station_info, sizeof(dmac_query_station_info_request_event)) != EOK) {
        oam_error_log0(vap_id, OAM_SF_ANY, "wal_hipriv_get_station_info:memcpy_s fail !");
        return OAL_FAIL;
    }

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(dmac_query_station_info_request_event),
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(vap_id, 0, "{wal_hipriv_get_station_info::wal_send_cfg_event err code %d!}", ret);
        return OAL_FAIL;
    }
    leftime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
        (OAL_TRUE == hmac_vap->station_info_query_completed_flag), 5 * OAL_TIME_HZ); /* 5s超时 */
    if (leftime <= 0) {
        oam_warning_log0(vap_id, OAM_SF_ANY, "{wal_hipriv_get_station_info::query info wait for 5 s timeout!}");
        return OAL_FAIL;
    } else {
        /* 正常结束  */
        wal_hipriv_printf_station_info(hmac_vap);
        return OAL_SUCC;
    }
}
#endif

uint32_t wal_hipriv_get_tid_queue_info(oal_net_device_stru *net_dev, int8_t *param)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    dmac_query_request_event query_req_event = { 0 };
    wal_msg_write_stru write_msg;
    int32_t ret, offset;
    uint8_t vap_id;
    if (oal_any_null_ptr2(net_dev, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac地址 */
    ret = wal_hipriv_get_mac_addr(param, query_req_event.auc_query_sta_addr, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_get_tid_queue_info::wal_hipriv_get_mac_addr fail!}");
        return ret;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_get_tid_queue_info::oal_net_dev_priv, return null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    vap_id = mac_vap->uc_vap_id;
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    query_req_event.query_event = OAL_QUERY_STATION_INFO_EVENT;
    hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    /* 3.1 填写 msg 消息头 */
    write_msg.en_wid = WLAN_CFGID_QUERY_TID_QUEUE;
    write_msg.us_len = sizeof(dmac_query_request_event);

    /* 3.2 填写 msg 消息体 */
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
        &query_req_event, sizeof(dmac_query_request_event)) != EOK) {
        oam_error_log0(vap_id, OAM_SF_ANY, "wal_hipriv_get_tid_queue_info:memcpy_s fail !");
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(dmac_query_request_event),
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log0(vap_id, 0, "{wal_hipriv_get_tid_queue_info::wal_send_cfg_event fail!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

uint32_t wal_hipriv_psm_flt_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    uint32_t ret;
    uint8_t uc_query_type;
    /* 获取query type */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_psm_flt_stat::first para return err_code [%d]!}\r\n", ret);
        return ret;
    }

    uc_query_type = (uint8_t)oal_atoi((const int8_t *)ac_name);
    wal_ioctl_get_psm_stat(pst_net_dev, uc_query_type, NULL);
    return OAL_SUCC;
}

const wal_ioctl_cmd_tbl_stru g_mem_info_cfg_map[MAC_MEMINFO_BUTT] = {
    { "user",           MAC_MEMINFO_USER },
    { "vap",            MAC_MEMINFO_VAP },
    { "pool_usage",     MAC_MEMINFO_POOL_INFO },
    { "pool_debug",     MAC_MEMINFO_POOL_DBG },
    { "dscr",           MAC_MEMINFO_DSCR },
    { "netbuff",        MAC_MEMINFO_NETBUFF },
    { "sdio_sch_q",     MAC_MEMINFO_SDIO_TRX },
    { "sample_alloc",   MAC_MEMINFO_SAMPLE_ALLOC },
    { "sample_free",    MAC_MEMINFO_SAMPLE_FREE },
    { "all",            MAC_MEMINFO_ALL },
};

OAL_STATIC uint32_t wal_hipriv_get_meminfo_type(int8_t *name, uint8_t *map_idx)
{
    uint8_t idx;

    for (idx = 0; idx < MAC_MEMINFO_BUTT; idx++) {
        if (oal_strcmp(name, g_mem_info_cfg_map[idx].pc_name) == 0) {
            *map_idx = idx;
            return OAL_SUCC;
        }
    }
    oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_get_meminfo_type::type error!}\r\n");
    return OAL_FAIL;
}

OAL_STATIC void wal_hipriv_set_meminfo_msg(wal_msg_write_stru *write_msg, uint8_t mem_type, uint8_t object_idx)
{
    mac_cfg_meminfo_stru *meminfo_param = (mac_cfg_meminfo_stru *)(write_msg->auc_value);

    wal_write_msg_hdr_init(write_msg, WLAN_CFGID_DEVICE_MEM_INFO, sizeof(mac_cfg_meminfo_stru));
    meminfo_param->uc_meminfo_type = mem_type;
    meminfo_param->uc_object_index = object_idx;
}

uint32_t wal_hipriv_memory_info(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    uint8_t uc_meminfo_type = MAC_MEMINFO_BUTT;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_msg_write_stru st_write_msg;
    uint8_t object_index = 0xFF; /* host和device mempool个数不一致 ，这里用0xff给个default值 */

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 memoryinfo \
            [all|dscr|netbuff|user|vap|sdio_sch_q] [pool_usage/pool_debug 0|1, ..7<pool_id>]'!}\r\n");
        return ret;
    }

    if (oal_strcmp("host", ac_name) == 0) {
        oal_mem_print_pool_info();
        return OAL_SUCC;
    } else if (oal_strcmp("device", ac_name) == 0) {
        hcc_print_device_mem_info(hcc_get_handler(HCC_EP_WIFI_DEV));
        return OAL_SUCC;
    } else {
        ret = wal_hipriv_get_meminfo_type(ac_name, &uc_meminfo_type);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    if ((uc_meminfo_type == MAC_MEMINFO_POOL_INFO) || (uc_meminfo_type == MAC_MEMINFO_POOL_DBG) ||
        (uc_meminfo_type == MAC_MEMINFO_SAMPLE_ALLOC)) {
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        /* 没有后续参数不退出 */
        if (ret == OAL_SUCC) {
            object_index = (uint8_t)oal_atoi(ac_name);
        }
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    wal_hipriv_set_meminfo_msg(&st_write_msg, uc_meminfo_type, object_index);
    l_ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_meminfo_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_memory_info::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 获取版本
 * 1.日    期  : 2014年3月25日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_get_version(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    uint16_t us_len;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_get_version::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_GET_VERSION, us_len);

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_get_version::wal_send_cfg_event return err_code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_vap_log_level
 * 功能描述  : 设置vap的LOG打印级别
 * 1.日    期  : 2013年6月3日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_vap_log_level(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    mac_vap_stru *mac_vap = NULL;
    uint8_t en_level_val;  // oam_log_level_enum
    uint32_t off_set = 0;
    int8_t ac_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;

    wal_msg_write_stru write_msg = {0};

    /* OAM log模块的开关的命令: hipriv "Hisilicon0[vapx] log_level {1/2}"
       1-2(error与warning)级别日志以vap级别为维度；
    */
    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_vap_log_level::null pointer.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取日志级别 */
    ret = wal_get_cmd_one_arg(pc_param, ac_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    en_level_val = (uint8_t)oal_atoi(ac_param);  // oam_log_level_enum
    if ((en_level_val < OAM_LOG_LEVEL_ERROR) || (en_level_val > OAM_LOG_LEVEL_INFO)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_feature_log_level::invalid switch value[%d].}", en_level_val);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* hipriv "Hisilicon0 log_level 1|2|3" 设置所有vip id的log */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (0 == oal_strcmp("Hisilicon0", net_dev->name)) {
        hmac_config_set_all_log_level(mac_vap, sizeof(uint8_t),  // oam_log_level_enum
            (uint8_t *)&en_level_val);
        return OAL_SUCC;
    }
#endif

    oam_log_set_vap_level(mac_vap->uc_vap_id, en_level_val);

    // 目前支持02 device 设置log 级别， 遗留后续的合并问题
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_LOG_LEVEL, sizeof(int32_t));
    *((int32_t *)(write_msg.auc_value)) = en_level_val;
    ret |= (uint32_t)wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_vap_log_level::err[%d]!}", ret);
        return ret;
    }

    return ret;
}
/*
 * 函 数 名  : wal_hipriv_getcountry
 * 功能描述  : 读取国家码
 * 1.日    期  : 2015年1月16日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_getcountry(oal_net_device_stru *net_dev, int8_t *pc_param)
{
#ifdef _PRE_WLAN_FEATURE_11D
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_rsp_stru *pst_query_rsp_msg = NULL;
    wal_msg_query_stru st_query_msg;
    int8_t ac_tmp_buff[OAM_PRINT_FORMAT_LENGTH];
    int32_t ret;

    /* 抛事件到wal层处理 */
    st_query_msg.en_wid = WLAN_CFGID_COUNTRY;

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_QUERY, WAL_MSG_WID_LENGTH,
                             (uint8_t *)&st_query_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        if (rsp_msg != NULL) {
            oal_free(rsp_msg);
        }

        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_getcountry::wal_alloc_cfg_event err %d!}", ret);
        return (uint32_t)ret;
    }

    /* 处理返回消息 */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(rsp_msg->auc_msg_data);

    ret = snprintf_s(ac_tmp_buff, sizeof(ac_tmp_buff), sizeof(ac_tmp_buff) - 1,
                     "getcountry code is : %c%c.\n",
                     pst_query_rsp_msg->auc_value[0],
                     pst_query_rsp_msg->auc_value[1]);
    if (ret < 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_list_sta::snprintf_s error!");
        oal_free(rsp_msg);
        oam_print(ac_tmp_buff);
        return OAL_FAIL;
    }

    oam_warning_log3(0, OAM_SF_CFG, "{wal_hipriv_getcountry:: %c, %c, len %d}",
                     pst_query_rsp_msg->auc_value[0], pst_query_rsp_msg->auc_value[1],
                     pst_query_rsp_msg->us_len);

    oal_free(rsp_msg);
    oam_print(ac_tmp_buff);

#else
    oam_info_log0(0, OAM_SF_ANY, "{wal_hipriv_getcountry::_PRE_WLAN_FEATURE_11D is not define!}");
#endif

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置主子类型字符串命令类型命令统一接口
 * 1.日    期  : 2017年6月15日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_str_get_sub_cmd(int8_t *param, mac_cfg_set_str_stru *set_cmd,
    wal_ioctl_tlv_stru *sub_cmd_table)
{
    uint8_t index = 0;
    uint8_t value_base = 10;
    uint32_t off_set, ret;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t *value = NULL;
    int8_t *tmp_param = param;
    wal_ioctl_tlv_stru *sub_cmd = sub_cmd_table;

    /* 遍历获取每个字符参数 */
    for (index = 0; index < set_cmd->uc_cmd_cnt; index++) {
        /* 获取子命令 */
        ret = wal_get_cmd_one_arg(tmp_param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::wal_get_cmd_one_arg return err_code [%d]!}", ret);
            return ret;
        }
        tmp_param += off_set;

        /* 子命令表查找对应的子命令 */
        while (sub_cmd->pc_name != NULL) {
            if (oal_strcmp(sub_cmd->pc_name, name) == 0) {
                break;
            }
            sub_cmd++;
        }

        /* 没有找到对应的命令，则报错 */
        if (sub_cmd->pc_name == NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::invalid alg_cfg command!}\r\n");
            return OAL_FAIL;
        }

        /* 获取设定置 */
        ret = wal_get_cmd_one_arg(tmp_param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::wal_get_cmd_one_arg return err_code [%d]!}", ret);
            return ret;
        }
        value = name;
        if (basic_value_is_hex(name, WAL_HIPRIV_CMD_NAME_MAX_LEN)) {
            value_base = 16; // 16代表传进制参数为16
            value = name + BYTE_OFFSET_2;
        }
        tmp_param += off_set;

        /* 记录每个子命令的id号和具体的命令值 */
        set_cmd->us_set_id[index] = sub_cmd->en_tlv_cfg_id;
        set_cmd->value[index] = (uint32_t)oal_strtol(value, NULL, value_base);
        oam_warning_log4(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::index[%d] sub cfg id[%d], value_base[%s], value[%d]}",
            index, set_cmd->us_set_id[index], value_base, set_cmd->value[index]);
    }
    return OAL_SUCC;
}
uint8_t wal_hipriv_set_str_get_map_index(int8_t *name, uint8_t name_len)
{
    uint8_t map_index;
    wal_ioctl_str_stru cmd_table;
    for (map_index = 0; map_index < g_set_str_table_size; map_index++) {
        cmd_table = g_ast_set_str_table[map_index];
        if (0 == oal_strcmp(cmd_table.pc_name, name)) {
            return map_index;
        }
    }
    return g_set_str_table_size;
}
uint32_t wal_hipriv_set_str_cmd(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set, ret;
    int32_t l_ret;
    int8_t *tmp_param = pc_param;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_msg_write_stru st_write_msg = {0};
    mac_cfg_set_str_stru *set_cmd = (mac_cfg_set_str_stru *)(st_write_msg.auc_value);
    uint8_t uc_cmd_cnt, map_index;

    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(tmp_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    tmp_param += off_set;

    /* 寻找匹配的命令 */
    map_index = wal_hipriv_set_str_get_map_index(ac_name, sizeof(ac_name));
    /* 没有找到对应的命令，则报错 */
    if (map_index == g_set_str_table_size) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录主命令对应的枚举值 */
    set_cmd->us_cfg_id = g_ast_set_str_table[map_index].en_tlv_cfg_id;

    ret = wal_get_cmd_one_arg(tmp_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    tmp_param += off_set;
    /* 当前命令的参数个数 */
    uc_cmd_cnt = (uint8_t)oal_atoi(ac_name);
    if (uc_cmd_cnt > MAC_STR_CMD_MAX_SIZE) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::too many para [%d]!}\r\n", uc_cmd_cnt);
        return OAL_FAIL;
    }

    set_cmd->uc_cmd_cnt = uc_cmd_cnt;
    set_cmd->uc_len = sizeof(mac_cfg_set_str_stru);
    oam_warning_log3(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd:: cfg id[%d] cfg len[%d] cmd cnt[%d]!}",
                     set_cmd->us_cfg_id, set_cmd->uc_len, uc_cmd_cnt);

    /* 遍历获取每个字符参数 */
    ret = wal_hipriv_set_str_get_sub_cmd(tmp_param, set_cmd, g_ast_set_str_table[map_index].pst_cfg_table);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, set_cmd->us_cfg_id, set_cmd->uc_len);

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + set_cmd->uc_len,
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_tlv_cmd::wal_send_cfg_event return err_code [%d]!}", l_ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}

#endif

OAL_STATIC int32_t wal_psm_query_wait_complete(hmac_psm_flt_stat_query_stru *pst_hmac_psm_query,
    mac_psm_query_type_enum_uint8 en_query_type)
{
    pst_hmac_psm_query->auc_complete_flag[en_query_type] = OAL_FALSE;
    return oal_wait_event_interruptible_timeout_m(pst_hmac_psm_query->st_wait_queue,
        (pst_hmac_psm_query->auc_complete_flag[en_query_type] == OAL_TRUE),
        OAL_TIME_HZ);
}

int32_t wal_ioctl_get_psm_stat(oal_net_device_stru *net_dev,
    mac_psm_query_type_enum_uint8 en_query_type, wal_wifi_priv_cmd_stru *priv_cmd)
{
    hmac_device_stru *pst_hmac_device = NULL;
    wal_msg_write_stru write_msg = {0};
    hmac_psm_flt_stat_query_stru *pst_hmac_psm_query = NULL;
    mac_psm_query_stat_stru  *pst_psm_stat = NULL;
    int32_t ret;
    mac_vap_stru *mac_vap = oal_net_dev_priv(net_dev);

    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::mac_vap get from netdev is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (pst_hmac_device == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (en_query_type >= MAC_PSM_QUERY_TYPE_BUTT) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::query type is not supported}");
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    memset_s(&write_msg, sizeof(write_msg), 0, sizeof(write_msg));
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_QUERY_PSM_STAT, sizeof(mac_psm_query_type_enum_uint8));
    /* 下发查询命令 */
    *(mac_psm_query_type_enum_uint8*)(write_msg.auc_value) = en_query_type;
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_psm_query_type_enum_uint8), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::send event fail[%d]}", ret);
        return OAL_FAIL;
    }

    /* 等待查询返回 */
    pst_hmac_psm_query = &pst_hmac_device->st_psm_flt_stat_query;
    ret = wal_psm_query_wait_complete(pst_hmac_psm_query, en_query_type);
    /* 超时或异常 */
    if (ret <= 0) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::fail! ret:%d}", ret);
        return OAL_FAIL;
    }
    /* 调式命令时该参数为null */
    if (priv_cmd == NULL) {
        return OAL_SUCC;
    }
    pst_psm_stat = &pst_hmac_psm_query->ast_psm_stat[en_query_type];
    if ((pst_psm_stat->query_item > MAC_PSM_QUERY_MSG_MAX_STAT_ITEM) ||
        (priv_cmd->total_len < (pst_psm_stat->query_item * sizeof(uint32_t)))) {
        oam_error_log2(0, OAM_SF_CFG, "wal_ioctl_get_psm_stat:total_len=%d, query_item=%d",
            priv_cmd->total_len, pst_psm_stat->query_item);
        return OAL_FAIL;
    }

    if (oal_copy_to_user(priv_cmd->puc_buf, pst_psm_stat->aul_val, pst_psm_stat->query_item * sizeof(uint32_t))) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_ioctl_get_psm_stat::Failed to copy ioctl_data to user !}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_ioctl_get_iwname
 * 功能描述  : 获取无线的名字 用来确认无线扩展的存在
 * 1.日    期  : 2012年12月13日
  *   修改内容  : 新生成函数
 */
int wal_ioctl_get_iwname(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    int8_t ac_iwname[] = "IEEE 802.11";

    if ((net_dev == NULL) || (pst_wrqu == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_get_iwname::param null.}");
        return -OAL_EINVAL;
    }

    if (EOK != memcpy_s(pst_wrqu->name, OAL_IF_NAME_SIZE, ac_iwname, sizeof(ac_iwname))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_get_iwname::memcpy fail!");
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_ioctl_get_apaddr
 * 功能描述  : 获取bssid
 * 1.日    期  : 2014年12月17日
  *   修改内容  : 新生成函数
 */
int wal_ioctl_get_apaddr(oal_net_device_stru *net_dev, oal_iw_request_info_stru *pst_info,
    oal_iwreq_data_union *pst_wrqu, char *pc_extra)
{
    mac_vap_stru *mac_vap = NULL;
    oal_sockaddr_stru *pst_addr = (oal_sockaddr_stru *)pst_wrqu;
    uint8_t auc_zero_addr[WLAN_MAC_ADDR_LEN] = { 0 };

    if ((net_dev == NULL) || (pst_addr == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_get_apaddr::param null.}");
        return -OAL_EINVAL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_get_apaddr::mac_vap is null!}");
        return -OAL_EFAUL;
    }

    if (mac_vap->en_vap_state == MAC_VAP_STATE_UP) {
        oal_set_mac_addr((uint8_t *)pst_addr->sa_data, mac_vap->auc_bssid);
    } else {
        oal_set_mac_addr((uint8_t *)pst_addr->sa_data, auc_zero_addr);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_octl_get_essid
 * 功能描述  : 获取ssid
 */
int wal_ioctl_get_essid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *pst_info,
    oal_iwreq_data_union *pst_data, char *pc_ssid)
{
    int32_t ret;
    wal_msg_query_stru st_query_msg = {0};
    mac_cfg_ssid_param_stru *pst_ssid = NULL;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_rsp_stru *pst_query_rsp_msg = NULL;
    oal_iw_point_stru *pst_essid = (oal_iw_point_stru *)pst_data;

    if (net_dev == NULL || pst_data == NULL || pc_ssid == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_get_essid:: wal_send_cfg_event err!}");
        return -OAL_EFAIL;
    }

    /* 抛事件到wal层处理 */
    st_query_msg.en_wid = WLAN_CFGID_SSID;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_QUERY, WAL_MSG_WID_LENGTH,
                             (uint8_t *)&st_query_msg, OAL_TRUE, &rsp_msg);
    if (ret != OAL_SUCC || rsp_msg == NULL) {
        if (rsp_msg != NULL) {
            oal_free(rsp_msg);
        }
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_get_essid:: wal_send_cfg_event err %d!}", ret);
        return -OAL_EFAIL;
    }

    /* 处理返回消息 */
    pst_query_rsp_msg = (wal_msg_rsp_stru *)(rsp_msg->auc_msg_data);

    /* 业务处理 */
    pst_ssid = (mac_cfg_ssid_param_stru *)(pst_query_rsp_msg->auc_value);
    pst_essid->flags = 1; /* 设置出参标志为有效 */
    pst_essid->length = oal_min(pst_ssid->uc_ssid_len, OAL_IEEE80211_MAX_SSID_LEN);
    if (EOK != memcpy_s(pc_ssid, pst_essid->length, pst_ssid->ac_ssid, pst_ssid->uc_ssid_len)) {
        oam_warning_log1(0, OAM_SF_ANY, "wal_ioctl_get_essid::memcpy fail! pst_essid->length[%d]", pst_essid->length);
        oal_free(rsp_msg);
        return -OAL_EINVAL;
    }

    oal_free(rsp_msg);
    return OAL_SUCC;
}
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static void wal_hipriv_inetaddr_netdev_ops(struct in_ifaddr *pst_ifa, unsigned long event, mac_vap_stru *mac_vap)
{
    hmac_vap_stru *pst_hmac_vap = NULL;
    uint8_t ps_switch = OAL_TRUE;
    wal_wake_lock();

    switch (event) {
        case NETDEV_UP: {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call:: \
                Up IPv4[%d.X.X.%d], MASK[0x%08X].}", ((uint8_t *)&(pst_ifa->ifa_address))[0],
                ((uint8_t *)&(pst_ifa->ifa_address))[3], pst_ifa->ifa_mask); /* 3 netaddr */
            hmac_arp_offload_set_ip_addr(mac_vap, DMAC_CONFIG_IPV4,
                DMAC_IP_ADDR_ADD, &(pst_ifa->ifa_address), &(pst_ifa->ifa_mask));

            if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
                pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
                if (pst_hmac_vap == NULL) {
                    break;
                }
                /* 获取到的时候开启低功耗 */
                if ((pst_hmac_vap->uc_ps_mode == MAX_FAST_PS) || (pst_hmac_vap->uc_ps_mode == AUTO_FAST_PS)) {
                    wlan_pm_set_timeout((g_wlan_min_fast_ps_idle > 1) ?
                        (g_wlan_min_fast_ps_idle - 1) : g_wlan_min_fast_ps_idle);
                } else {
                    wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);
                }
                hmac_config_set_rx_listen_ps_switch(&pst_hmac_vap->st_vap_base_info, sizeof(uint8_t), &ps_switch);
                pst_hmac_vap->ipaddr_obtained = OAL_TRUE;
                /* 获取到的时候通知漫游计时 */
                hmac_roam_wpas_connect_state_notify(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_OBTAINED);
            }
            break;
        }

        case NETDEV_DOWN: {
            oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::\
                Down IPv4[%d.X.X.%d], MASK[0x%08X]..}", ((uint8_t *)&(pst_ifa->ifa_address))[0],
                ((uint8_t *)&(pst_ifa->ifa_address))[3], pst_ifa->ifa_mask); /* 3 netaddr */
            hmac_arp_offload_set_ip_addr(mac_vap, DMAC_CONFIG_IPV4, DMAC_IP_ADDR_DEL,
                &(pst_ifa->ifa_address), &(pst_ifa->ifa_mask));

            if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
                /* 获取到的时候通知漫游计时 */
                pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
                if (pst_hmac_vap == NULL) {
                    break;
                }
                pst_hmac_vap->ipaddr_obtained = OAL_FALSE;
                hmac_roam_wpas_connect_state_notify(pst_hmac_vap, WPAS_CONNECT_STATE_IPADDR_REMOVED);
            }
            break;
        }

        default:
            oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_hipriv_inetaddr_notifier_call::Unknown event[%d]}", event);
            break;
    }
    wal_wake_unlock();
}

/*
 * 函 数 名  : wal_hipriv_inetaddr_notifier_call
 * 功能描述  : 通知链回调函数
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_hipriv_inetaddr_notifier_call(struct notifier_block *this, unsigned long event, void *ptr)
{
    /*
     * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
     * whenever there is an event related to an IP address.
     * ptr : kernel provided pointer to IP address that has changed
     */
    struct in_ifaddr *pst_ifa = (struct in_ifaddr *)ptr;
    mac_vap_stru *mac_vap = NULL;

    if (oal_unlikely(pst_ifa == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::pst_ifa is NULL.}");
        return NOTIFY_DONE;
    }
    if (oal_unlikely(pst_ifa->ifa_dev->dev == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa->idev->dev is NULL.}");
        return NOTIFY_DONE;
    }

    /* Filter notifications meant for non Hislicon devices */
    if (pst_ifa->ifa_dev->dev->netdev_ops != &g_st_wal_net_dev_ops) {
        return NOTIFY_DONE;
    }

    mac_vap = (mac_vap_stru *)oal_net_dev_priv(pst_ifa->ifa_dev->dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log1(0, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call::Get mac vap failed, \
            when %d(UP:1 DOWN:2 UNKNOWN:others) ipv4 address.}", event);
        return NOTIFY_DONE;
    }

    if (ipv4_is_linklocal_169(pst_ifa->ifa_address)) {
        oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_PWR, "{wal_hipriv_inetaddr_notifier_call:: \
            Invalid IPv4[%d.X.X.%d], MASK[0x%08X].}", ((uint8_t *)&(pst_ifa->ifa_address))[0],
            ((uint8_t *)&(pst_ifa->ifa_address))[3], pst_ifa->ifa_mask); /* 3 netaddr */
        return NOTIFY_DONE;
    }

    wal_hipriv_inetaddr_netdev_ops(pst_ifa, event, mac_vap);
    return NOTIFY_DONE;
}

/*
 * 函 数 名  : wal_hipriv_inet6addr_notifier_call
 * 功能描述  : IPv6通知链回调函数
 * 1.日    期  : 2015年6月15日
 *   作    者  : Wlan_mib_temp
 *   修改内容  : 新生成函数
 */
int32_t wal_hipriv_inet6addr_notifier_call(struct notifier_block *this, unsigned long event, void *ptr)
{
    /*
     * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
     * whenever there is an event related to an IP address.
     * ptr : kernel provided pointer to IP address that has changed
     */
    struct inet6_ifaddr *pst_ifa = (struct inet6_ifaddr *)ptr;
    mac_vap_stru *mac_vap = NULL;

    if (oal_unlikely(pst_ifa == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa is NULL.}");
        return NOTIFY_DONE;
    }
    if (oal_unlikely(pst_ifa->idev->dev == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_inet6addr_notifier_call::pst_ifa->idev->dev is NULL.}");
        return NOTIFY_DONE;
    }

    /* Filter notifications meant for non Hislicon devices */
    if (pst_ifa->idev->dev->netdev_ops != &g_st_wal_net_dev_ops) {
        return NOTIFY_DONE;
    }

    mac_vap = (mac_vap_stru *)oal_net_dev_priv(pst_ifa->idev->dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_warning_log1(0, OAM_SF_PWR,
            "{wal_hipriv_inet6addr_notifier_call::vap null,when %d(UP:1 DOWN:2 UNKNOWN:others) ipv6 addr}", event);
        return NOTIFY_DONE;
    }

    switch (event) {
        case NETDEV_UP: {
            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_PWR,
                "{wal_hipriv_inet6addr_notifier_call::UP IPv6[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x].}",
                oal_net2host_short((pst_ifa->addr.s6_addr16)[0]), oal_net2host_short((pst_ifa->addr.s6_addr16)[1]),
                oal_net2host_short((pst_ifa->addr.s6_addr16)[6]),  /* 6 net6addr */
                oal_net2host_short((pst_ifa->addr.s6_addr16)[7])); /* 7 net6addr */
            hmac_arp_offload_set_ip_addr(mac_vap, DMAC_CONFIG_IPV6, DMAC_IP_ADDR_ADD,
                &(pst_ifa->addr), &(pst_ifa->addr));
            break;
        }

        case NETDEV_DOWN: {
            oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_PWR,
                "{wal_hipriv_inet6addr_notifier_call::DOWN IPv6[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x].}",
                oal_net2host_short((pst_ifa->addr.s6_addr16)[0]), oal_net2host_short((pst_ifa->addr.s6_addr16)[1]),
                oal_net2host_short((pst_ifa->addr.s6_addr16)[6]),  /* 6 net6addr */
                oal_net2host_short((pst_ifa->addr.s6_addr16)[7])); /* 7 net6addr */
            hmac_arp_offload_set_ip_addr(mac_vap, DMAC_CONFIG_IPV6, DMAC_IP_ADDR_DEL,
                &(pst_ifa->addr), &(pst_ifa->addr));
            break;
        }

        default: {
            oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_hipriv_inet6addr_notifier_call::event:%d}", event);
            break;
        }
    }

    return NOTIFY_DONE;
}
#endif

#if ((defined(_PRE_WLAN_EXPORT)) && (defined(_PRE_WLAN_NARROW_BAND)))
int32_t wal_ioctl_get_tsensor_val(oal_net_device_stru *net_dev, int32_t *tsensor_val)
{
    int32_t ret;
    wal_msg_write_stru write_msg;
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    int32_t leftime;

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_EQUIP_TEST,
                       "{wal_ioctl_get_tsensor_val::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                       "{wal_ioctl_get_tsensor_val::mac_res_get_hmac_vap failed!}");
        return -OAL_EINVAL;
    }

    g_us_tsensor_val = 0;
    hmac_vap->st_atcmdsrv_get_status.uc_report_tsensor_val_flag = OAL_FALSE;
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_GET_TEMPERATURE, 0);
    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH,
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_EQUIP_TEST,
                         "{wal_ioctl_get_tsensor_val:: return err_code [%d]!}", ret);
        return ret;
    }

    /* 阻塞等待dmac上报 */
    leftime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
        (uint32_t)(hmac_vap->st_atcmdsrv_get_status.uc_report_tsensor_val_flag == OAL_TRUE),
        WAL_ATCMDSRB_NORM_TIME);
    if (leftime <= 0) {
        /* 超时还没有上报扫描结束 */
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                         "{wal_ioctl_get_tsensor_val::wait for %ld ms timeout/error, leftime %ld ms!}",
                         ((WAL_ATCMDSRB_NORM_TIME * HMAC_S_TO_MS) / OAL_TIME_HZ), leftime);
        return -OAL_EINVAL;
    } else {
        /* 正常结束，上报芯片Tsensor结温  */
        *tsensor_val = g_us_tsensor_val;
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_pdet_val::get tsensor[%d]!}",
                         *tsensor_val);
        return OAL_SUCC;
    }
}

int32_t wal_ioctl_get_pdet_val(oal_net_device_stru *net_dev, int32_t *pdet_val)
{
    int32_t ret;
    wal_msg_write_stru write_msg;
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    int32_t leftime;

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_get_pdet_val::OAL_NET_DEV_PRIV, return null!}");
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{wal_ioctl_get_pdet_val::mac_res_get_hmac_vap failed!}");
        return -OAL_EINVAL;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag = OAL_FALSE;
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_TX_PDET_VAL, sizeof(int32_t));

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_get_pdet_val::return err code %d!}\r\n", ret);
        return ret;
    }

    /* 阻塞等待dmac上报 */
    leftime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
        (uint32_t)(hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag == OAL_TRUE),
        WAL_ATCMDSRB_GET_RX_PCKT);
    if (leftime <= 0) {
        /* 超时还没有上报扫描结束 */
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ANY,
                         "{wal_ioctl_get_pdet_val:: wait for %ld ms timeout/error, leftime %ld ms!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * HMAC_S_TO_MS) / OAL_TIME_HZ), leftime);
        return -OAL_EINVAL;
    } else {
        /* 正常结束  */
        *pdet_val = hmac_vap->st_atcmdsrv_get_status.dbb_num;
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_ioctl_get_pdet_val::get pdet [%d]!}",
                         *pdet_val);
        return OAL_SUCC;
    }
}

int32_t wal_ioctl_get_sw_version(oal_net_device_stru *net_dev)
{
    int32_t ret;
    wal_msg_write_stru write_msg;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_GET_SW_VERSION, 0);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH, (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_get_pdet_val::return err code %d!}\r\n", ret);
        return ret;
    }
    return OAL_SUCC;
}
#endif
