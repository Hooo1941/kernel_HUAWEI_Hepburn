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
#include "oam_event_wifi.h"
#include "plat_pm_wlan.h"
#include "hmac_resource.h"
#include "hmac_roam_main.h"
#include "wal_linux_ioctl.h"
#include "plat_firmware.h"
#include "wal_cfg_ioctl.h"


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_USER_CFG_IOCTL_C

#ifdef WIFI_DEBUG_ENABLE
/*
 * 功能描述  : 触发去认证
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_start_deauth(mac_vap_stru *mac_vap, uint32_t *params)
{
    hmac_config_send_deauth(mac_vap, mac_vap->auc_bssid);
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置probe request和probe response上报的开关
 * 1.日    期  : 2020年8月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_probe_switch(mac_vap_stru *mac_vap, uint32_t *params)
{
    mac_cfg_probe_switch_stru probe_switch = { 0 };
    oam_80211_frame_ctx_union oam_switch_param = { 0 };
    uint8_t idx = 0;
    uint32_t ret;
    /*
     * sh hipriv.sh "Hisilicon0 probe_switch 0|1(帧方向tx|rx) 0|1(帧内容开关)
     * 0|1(CB开关) 0|1(描述符开关)"
     */
    probe_switch.en_frame_direction = (uint8_t)params[idx++]; /* 获取帧方向 */
    probe_switch.en_frame_switch    = (uint8_t)params[idx++]; /* 获取帧内容打印开关 */
    probe_switch.en_cb_switch       = (uint8_t)params[idx++]; /* 获取帧CB字段打印开关 */
    probe_switch.en_dscr_switch     = (uint8_t)params[idx++]; /* 获取描述符打印开关 */

    /* 将配置命令结构转换为OAM结构 */
    oam_switch_param.frame_ctx.bit_content    = probe_switch.en_frame_switch;
    oam_switch_param.frame_ctx.bit_cb         = probe_switch.en_cb_switch;
    oam_switch_param.frame_ctx.bit_dscr       = probe_switch.en_dscr_switch;

    ret = oam_report_80211_probe_set_switch(probe_switch.en_frame_direction, &oam_switch_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{wal_set_probe_switch::oam_report_80211_probe_set_switch failed[%d].}", ret);
        return ret;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 触发join
 * 1.日    期  : 2020年8月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_set_start_join(mac_vap_stru *mac_vap, uint32_t *params)
{
    mac_bss_dscr_stru *bss_dscr;
    uint32_t bss_idx = params[0];

    /* 根据bss index查找对应的bss dscr结构信息 */
    bss_dscr = hmac_scan_find_scanned_bss_dscr_by_index(mac_vap->uc_device_id, bss_idx);
    if (bss_dscr == NULL) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_set_start_join::find bss failed by index!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 增加cap info能力 */
    mac_vap->us_assoc_user_cap_info = bss_dscr->us_cap_info;
    return hmac_sta_initiate_join(mac_vap, bss_dscr);
}

/*
 * 功能描述  : 设置ssid发消息到wal处理
 * 1.日    期  : 2020年8月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_set_essid_config(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap,
                                                int8_t *ac_ssid, uint32_t max_len)
{
    mac_cfg_ssid_param_stru *param = NULL;
    wal_msg_write_stru write_msg;
    uint8_t ssid_len;
    int32_t ret;
    int8_t *ssid = NULL;

    ssid = ac_ssid;
    ssid = oal_strim(ac_ssid); /* 去掉字符串开始结尾的空格 */
    ssid_len = (uint8_t)OAL_STRLEN(ssid);

    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_set_essid_config:: ssid length %d!}", ssid_len);

    if (ssid_len > max_len - 1) { /* -1为\0预留空间 */
        ssid_len = max_len - 1;
    }

    oam_info_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_set_essid_config:: ssid length is %d!}", ssid_len);
    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SSID, sizeof(mac_cfg_ssid_param_stru));

    /* 填写WID对应的参数 */
    param = (mac_cfg_ssid_param_stru *)(write_msg.auc_value);
    param->uc_ssid_len = ssid_len;
    if (EOK != memcpy_s(param->ac_ssid, sizeof(param->ac_ssid), ssid, ssid_len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_set_essid_config::memcpy fail!");
        return OAL_FAIL;
    }

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ssid_param_stru), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_essid_config::alloc_cfg_event err:%d}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_hipriv_set_essid
 * 功能描述  : 设置ssid
 * 1.日    期  : 2014年12月17日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_essid(oal_net_device_stru *net_dev, int8_t *param)
{
    uint32_t ret;
    mac_vap_stru *mac_vap = NULL;
    uint32_t off_set = 0;
    int8_t ac_ssid[WLAN_SSID_MAX_LEN] = { 0 };
    uint32_t result;

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_essid::mac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 设备在up状态且是AP时，不允许配置，必须先down */
        if (0 != (OAL_IFF_RUNNING & oal_netdevice_flags(net_dev))) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_ANY,
                "{wal_hipriv_set_essid::dev  busy, please down it first:%d}", oal_netdevice_flags(net_dev));
            return -OAL_EBUSY;
        }
    }

    /* param指向传入模式参数, 将其取出存放到mode_str中 */
    result = wal_get_cmd_one_arg(param, ac_ssid, WLAN_SSID_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_essid::wal_get_cmd_one_arg vap name err %d}", result);
        return result;
    }

    /* 设置ssid并发送消息到wal层处理 */
    ret = wal_hipriv_set_essid_config(net_dev, mac_vap, ac_ssid, WLAN_SSID_MAX_LEN);
    if (ret != OAL_SUCC) {
        oam_warning_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_set_essid::send cfg event fail}");
        return ret;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_hipriv_add_user
 * 功能描述  : 设置添加用户的配置命令
 * 1.日    期  : 2013年6月5日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_add_user(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    int32_t ret;
    mac_cfg_add_user_param_stru *pst_add_user_param = NULL;
    mac_cfg_add_user_param_stru st_add_user_param; /* 临时保存获取的use的信息 */
    uint32_t get_addr_idx;

    /*
        设置添加用户的配置命令: hipriv "vap0 add_user xx xx xx xx xx xx(mac地址) 0 | 1(HT能力位) "
        该命令针对某一个VAP
    */
    /* 获取mac地址 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_user::get cmd one arg err[%d]!}", result);
        return result;
    }

    memset_s((uint8_t *)&st_add_user_param, sizeof(st_add_user_param), 0, sizeof(st_add_user_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_add_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取用户的HT标识 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_user::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 针对解析出的不同命令，对user的HT字段进行不同的设置 */
    if (0 == (oal_strcmp("0", ac_name))) {
        st_add_user_param.en_ht_cap = 0;
    } else if (0 == (oal_strcmp("1", ac_name))) {
        st_add_user_param.en_ht_cap = 1;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_add_user::the mod switch cmd error!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_ADD_USER, sizeof(mac_cfg_add_user_param_stru));

    /* 设置配置命令参数 */
    pst_add_user_param = (mac_cfg_add_user_param_stru *)(write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_add_user_param->auc_mac_addr[get_addr_idx] = st_add_user_param.auc_mac_addr[get_addr_idx];
    }
    pst_add_user_param->en_ht_cap = st_add_user_param.en_ht_cap;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_user_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_FALSE,
                             NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_user::err[%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_del_user
 * 功能描述  : 设置删除用户的配置命令
 * 1.日    期  : 2013年6月5日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_del_user(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    int32_t ret;
    mac_cfg_del_user_param_stru *pst_del_user_param = NULL;
    mac_cfg_del_user_param_stru st_del_user_param; /* 临时保存获取的use的信息 */
    uint32_t get_addr_idx;

    /*
        设置删除用户的配置命令: hipriv "vap0 del_user xx xx xx xx xx xx(mac地址)"
        该命令针对某一个VAP
    */
    /* 获取mac地址 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_del_user::get cmd one arg err[%d]!}", result);
        return result;
    }

    memset_s((uint8_t *)&st_del_user_param, sizeof(st_del_user_param), 0, sizeof(st_del_user_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_del_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_DEL_USER, sizeof(mac_cfg_add_user_param_stru));

    /* 设置配置命令参数 */
    pst_del_user_param = (mac_cfg_add_user_param_stru *)(write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_del_user_param->auc_mac_addr[get_addr_idx] = st_del_user_param.auc_mac_addr[get_addr_idx];
    }

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_user_param_stru),
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_del_user::err[%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 触发sta初始扫描
 * 1.日    期  : 2013年6月27日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_start_scan(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint8_t uc_is_p2p0_scan;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_START_SCAN, sizeof(int32_t));

    uc_is_p2p0_scan = (oal_memcmp(pst_net_dev->name, "p2p0", OAL_STRLEN("p2p0")) == 0) ? 1 : 0;
    st_write_msg.auc_value[0] = uc_is_p2p0_scan;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_start_scan::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
/*
 * 功能描述  : 入网
 */
uint32_t wal_hipriv_start_join(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    /* 去关联1个用户的命令 hipriv "vap0 kick_user xx:xx:xx:xx:xx:xx" */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_kick_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_JOIN_BSSID, sizeof(auc_mac_addr));
    /* 设置配置命令参数 */
    oal_set_mac_addr(st_write_msg.auc_value, auc_mac_addr);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(auc_mac_addr),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_kick_user::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 删除1个用户
 * 1.日    期  : 2013年8月27日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_kick_user(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_kick_user_param_stru *pst_kick_user_param = NULL;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    /* 去关联1个用户的命令 hipriv "vap0 kick_user xx:xx:xx:xx:xx:xx" */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_kick_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_KICK_USER, sizeof(mac_cfg_kick_user_param_stru));

    /* 设置配置命令参数 */
    pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(st_write_msg.auc_value);
    pst_kick_user_param->us_reason_code = MAC_UNSPEC_REASON;
    pst_kick_user_param->send_disassoc_immediately = OAL_TRUE;

    if (oal_memcmp(ac_name, "all", OAL_STRLEN("all")) == 0) {
        oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, BROADCAST_MACADDR);
    } else {
        oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
        oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, auc_mac_addr);
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_kick_user_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_kick_user::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_roam_start_para_prepare(wal_msg_write_stru *write_msg, int8_t *param)
{
    uint8_t scan_type = ROAM_SCAN_CHANNEL_ORG_FULL;
    oal_bool_enum_uint8 en_current_bss_ignore = OAL_TRUE;
    oal_bool_enum_uint8 is_roaming_trigged_by_cmd = OAL_FALSE;
    uint8_t bssid[OAL_MAC_ADDR_LEN] = { 0 };
    mac_cfg_set_roam_start_stru *roam_start = (mac_cfg_set_roam_start_stru *)(write_msg->auc_value);
    uint32_t off_set, ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
    /* 指定漫游时刻是否搭配扫描操作 */
        scan_type = (uint8_t)oal_atoi(ac_name);
        param += off_set;
    } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
    /* 默认不指定bssid */
        memset_s(bssid, sizeof(bssid), 0, sizeof(bssid));
    } else {
        return ret;
    }

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
    /* 0/TRUE或者参数缺失表示漫游到其它AP, 1/FALSE表示漫游到自己 */
        en_current_bss_ignore = ((uint32_t)oal_atoi(ac_name) == 0);
        param += off_set;
    } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
        memset_s(bssid, sizeof(bssid), 0, sizeof(bssid));
    } else {
        return ret;
    }

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
    /* 漫游是否由命令触发 */
        is_roaming_trigged_by_cmd = (uint8_t)oal_atoi(ac_name);
        param += off_set;
    } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
        memset_s(bssid, sizeof(bssid), 0, sizeof(bssid));
    } else {
        return ret;
    }

    ret = wal_hipriv_get_mac_addr(param, bssid, &off_set);
    if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
        memset_s(bssid, sizeof(bssid), 0, sizeof(bssid));
    } else if (ret != OAL_SUCC) {
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    roam_start->uc_scan_type = scan_type;
    roam_start->en_current_bss_ignore = en_current_bss_ignore;
    roam_start->is_roaming_trigged_by_cmd = is_roaming_trigged_by_cmd;
    if (memcpy_s(roam_start->auc_bssid, sizeof(roam_start->auc_bssid), bssid, sizeof(bssid)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_roam_start::memcpy fail!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 用命令开始漫游
 * 1.日    期  : 2015年6月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_roam_start(oal_net_device_stru *net_dev, int8_t *param)
{
    int32_t l_ret;
    wal_msg_write_stru write_msg;
    uint32_t ret;

    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));
    ret = wal_hipriv_roam_start_para_prepare(&write_msg, param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_ROAM_START, sizeof(mac_cfg_set_roam_start_stru));

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_roam_start_stru),
                               (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ROAM, "{wal_hipriv_roam_start::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
OAL_STATIC uint32_t wal_hipriv_addba_req_event(oal_net_device_stru *pst_net_dev,
    mac_cfg_addba_req_param_stru *param)
{
    mac_cfg_addba_req_param_stru *pst_addba_req_param = NULL;
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint32_t get_addr_idx;
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_ADDBA_REQ, sizeof(mac_cfg_addba_req_param_stru));

    /* 设置配置命令参数 */
    pst_addba_req_param = (mac_cfg_addba_req_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_addba_req_param->auc_mac_addr[get_addr_idx] = param->auc_mac_addr[get_addr_idx];
    }

    pst_addba_req_param->uc_tidno = param->uc_tidno;
    pst_addba_req_param->en_ba_policy = param->en_ba_policy;
    pst_addba_req_param->us_buff_size = param->us_buff_size;
    pst_addba_req_param->us_timeout = param->us_timeout;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_addba_req_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_addba_req_get_mac_addr(int8_t *pc_param, int8_t *ac_name, uint8_t name_len,
    uint32_t *off_set, mac_cfg_addba_req_param_stru *output)
{
    uint32_t ret;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, name_len, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req_get_mac_addr::wal_get_cmd_one_arg err [%d]!}", ret);
        return ret;
    }

    oal_strtoaddr(ac_name, name_len, output->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_addba_req_get_tid(int8_t *pc_param, int8_t *ac_name, uint32_t *off_set,
    mac_cfg_addba_req_param_stru *output)
{
    uint32_t ret;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req_get_tid::wal_get_cmd_one_arg err [%d]!}", ret);
        return ret;
    }

    if (OAL_STRLEN(ac_name) > 2) { /* tid最大占2字节 */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_addba_req_get_tid::the addba req command is error}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    output->uc_tidno = (uint8_t)oal_atoi(ac_name);
    if (output->uc_tidno >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_addba_req_get_tid::the addba req command is error!uc_tidno is [%d]!}", output->uc_tidno);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_addba_req_get_ba_policy(int8_t *pc_param, int8_t *ac_name, uint32_t *off_set,
    mac_cfg_addba_req_param_stru *output)
{
    uint32_t ret;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req_get_ba_policy::wal_get_cmd_one_arg err [%d]!}", ret);
        return ret;
    }

    output->en_ba_policy = (uint8_t)oal_atoi(ac_name);
    if (output->en_ba_policy != MAC_BA_POLICY_IMMEDIATE) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_addba_req::ba policy is not correct! ba_policy is[%d]!}", output->en_ba_policy);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 建立BA会话的调测命令
 * 1.日    期  : 2013年6月7日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_addba_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    mac_cfg_addba_req_param_stru st_addba_req_param; /* 临时保存获取的addba req的信息 */

    memset_s((uint8_t *)&st_addba_req_param, sizeof(st_addba_req_param), 0, sizeof(st_addba_req_param));

    /*
     * 设置AMPDU关闭的配置命令:
     * hipriv "Hisilicon0 addba_req xx xx xx xx xx xx(mac地址) tidno ba_policy buffsize timeout"
     */
    /* 获取mac地址 */
    ret = wal_hipriv_addba_req_get_mac_addr(pc_param, ac_name, sizeof(ac_name), &off_set, &st_addba_req_param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取tid */
    ret = wal_hipriv_addba_req_get_tid(pc_param, ac_name, &off_set, &st_addba_req_param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pc_param = pc_param + off_set;

    /* 获取ba_policy */
    ret = wal_hipriv_addba_req_get_ba_policy(pc_param, ac_name, &off_set, &st_addba_req_param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pc_param = pc_param + off_set;

    /* 获取buffsize */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    st_addba_req_param.us_buff_size = (uint16_t)oal_atoi(ac_name);

    pc_param = pc_param + off_set;

    /* 获取timeout时间 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    st_addba_req_param.us_timeout = (uint16_t)oal_atoi(ac_name);
    return wal_hipriv_addba_req_event(pst_net_dev, &st_addba_req_param);
}

OAL_STATIC uint32_t wal_hipriv_delba_req_event(oal_net_device_stru *pst_net_dev,
    mac_cfg_delba_req_param_stru *param)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_delba_req_param_stru *pst_delba_req_param = NULL;
    int32_t l_ret;
    uint32_t get_addr_idx;
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DELBA_REQ, sizeof(mac_cfg_delba_req_param_stru));

    /* 设置配置命令参数 */
    pst_delba_req_param = (mac_cfg_delba_req_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_delba_req_param->auc_mac_addr[get_addr_idx] = param->auc_mac_addr[get_addr_idx];
    }

    pst_delba_req_param->uc_tidno = param->uc_tidno;
    pst_delba_req_param->en_direction = param->en_direction;
    pst_delba_req_param->en_trigger = MAC_DELBA_TRIGGER_COMM;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_delba_req_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 删除BA会话的调测命令
 * 1.日    期  : 2013年6月7日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_delba_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    mac_cfg_delba_req_param_stru st_delba_req_param; /* 临时保存获取的addba req的信息 */

    /*
     * 设置AMPDU关闭的配置命令:
     * hipriv "Hisilicon0 delba_req xx xx xx xx xx xx(mac地址) tidno direction reason_code"
     */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    memset_s((uint8_t *)&st_delba_req_param, sizeof(st_delba_req_param), 0, sizeof(st_delba_req_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_delba_req_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取tid */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    if (OAL_STRLEN(ac_name) > 2) { /* tid最大占2字节 */
        oam_warning_log0(0, OAM_SF_ANY,
            "{wal_hipriv_delba_req::the delba_req req command is error!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_delba_req_param.uc_tidno = (uint8_t)oal_atoi(ac_name);
    if (st_delba_req_param.uc_tidno >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_delba_req::the delba_req req command is error! tidno[%d]!}", st_delba_req_param.uc_tidno);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + off_set;

    /* 获取direction */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    st_delba_req_param.en_direction = (uint8_t)oal_atoi(ac_name);
    if (st_delba_req_param.en_direction >= MAC_BUTT_DELBA) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_delba_req::the direction is not correct! direction[%d]!}", st_delba_req_param.en_direction);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return wal_hipriv_delba_req_event(pst_net_dev, &st_delba_req_param);
}
#endif
