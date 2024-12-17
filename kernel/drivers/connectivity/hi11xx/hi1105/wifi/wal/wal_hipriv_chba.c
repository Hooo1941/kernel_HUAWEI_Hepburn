/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux ioctl配置命令
 * 作    者 :
 * 创建日期 : 2012年12月10日
 */
#ifdef _PRE_WLAN_CHBA_MGMT
#include "wal_hipriv_chba.h"
#include "wal_linux_cfg80211.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_HIPRIV_CHBA_C
/* 信道切换打桩命令 */
uint32_t wal_hipriv_chba_chan_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    mac_set_chan_switch_test_params test_params;
    int32_t l_ret;
    uint32_t ret;

    memset_s(&test_params, sizeof(test_params), 0, sizeof(test_params));

    /* 获取卡顿slot */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_chba_chan_switch::get para error, return.");
        return ret;
    }
    pc_param += off_set;
    test_params.intf_rpt_slot = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* 获取切换信道号 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_chba_chan_switch::get para error, return.");
        return ret;
    }
    pc_param += off_set;
    test_params.switch_chan_num = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* 获取切换信道带宽 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_chba_chan_switch::get para error, return.");
        return ret;
    }
    pc_param += off_set;
    test_params.switch_chan_bw = (oal_bool_enum_uint8)oal_atoi(ac_name);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_CHAN_SWITCH_TEST_CMD, sizeof(test_params));

    /* 设置配置命令参数 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(test_params),
        (const void *)&test_params, sizeof(test_params))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_chba_chan_switch::memcpy fail!");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(test_params),
        (uint8_t *)&st_write_msg,
        OAL_FALSE,
        NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_chan_switch::err[%d]!}", ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}

uint32_t wal_hipriv_chba_module_init(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    int32_t l_cfg_rst;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t device_type;

    /* 获取设定的值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_chba_module_init::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    device_type = (uint8_t)oal_atoi(ac_name);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_module_init[%d]!}\r\n", device_type);

    if (device_type >= DEVICE_TYPE_BUTT) {
        oam_error_log0(0, 0, "{wal_hipriv_chba_module_init::device type invalid!}");
        return OAL_FAIL;
    }
    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    *(uint8_t *)(st_write_msg.auc_value) = device_type;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_MODULE_INIT, sizeof(uint8_t));

    l_cfg_rst = wal_send_cfg_event(net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                                   (uint8_t *)&st_write_msg,
                                   OAL_FALSE,
                                   NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_module_init::return err code [%d]!}\r\n", l_cfg_rst);
        return (uint32_t)l_cfg_rst;
    }

    return OAL_SUCC;
}

uint32_t wal_hipriv_set_coex_chan_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return (uint32_t)(wal_ioctl_chba_set_coex(net_dev, pc_param));
}
uint32_t wal_hipriv_set_chba_battery(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    int32_t l_cfg_rst;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t battery_power;

    /* 获取设定的值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_chba_battery::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    battery_power = (uint8_t)oal_atoi(ac_name);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_chba_battery[%d]!}\r\n", battery_power);

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    *(uint8_t *)(st_write_msg.auc_value) = battery_power;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_SET_BATTERY, sizeof(uint8_t));

    l_cfg_rst = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_chba_battery::return err code [%d]!}\r\n", l_cfg_rst);
        return (uint32_t)l_cfg_rst;
    }

    return OAL_SUCC;
}

/* 设置CHBA 自动调整低功耗bitmap开关，0：关闭自动调整，1：打开自动调整 */
uint32_t wal_hipriv_chba_set_auto_bitmap(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    int32_t l_cfg_rst;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t switch_type;

    /* 获取设定的值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_chba_set_auto_bitmap::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    switch_type = (uint8_t)oal_atoi(ac_name);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_set_auto_bitmap[%d]!}\r\n", switch_type);
    if (switch_type >= CHBA_BITMAP_SWITCH_BUTT) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_chba_set_auto_bitmap::switch_type invalid!}\r\n");
        return OAL_FAIL;
    }

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    *(uint8_t *)(st_write_msg.auc_value) = switch_type;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_AUTO_BITMAP_CMD, sizeof(uint8_t));

    l_cfg_rst = wal_send_cfg_event(net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                                   (uint8_t *)&st_write_msg,
                                   OAL_FALSE,
                                   NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_set_auto_bitmap::return err code [%d]!}\r\n", l_cfg_rst);
        return (uint32_t)l_cfg_rst;
    }

    return OAL_SUCC;
}
/* 设置CHBA user bitmap */
uint32_t wal_hipriv_chba_set_user_bitmap(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    mac_chba_set_ps_bitmap_params ps_bitmap_info;
    int32_t l_ret;
    uint32_t ret;

    /*
        设置用户bitmap的配置命令: hipriv "vap0 set_user_bitmap xx:xx:xx:xx:xx:xx(mac地址) bitmap"
        该命令针对某一个VAP的某一个用户
    */
    /* 获取user mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_chba_set_user_bitmap::get 1st para error, return.");
        return ret;
    }
    memset_s((uint8_t *)&ps_bitmap_info, sizeof(ps_bitmap_info), 0, sizeof(ps_bitmap_info));
    oal_strtoaddr(ac_name, sizeof(ac_name), ps_bitmap_info.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    pc_param += off_set;

    /* 获取bitmap */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_chba_set_user_bitmap::get 2nd para error, return.");
        return ret;
    }
    pc_param += off_set;
    ps_bitmap_info.ps_bitmap = (uint32_t)oal_strtol(ac_name, NULL, 0);

    oam_warning_log4(0, OAM_SF_ANY, "wal_hipriv_chba_set_user_bitmap::mac[%02X:XX:XX:XX:%02X:%02X], bitmap[0x%x].",
        ps_bitmap_info.auc_mac_addr[MAC_ADDR_0],
        ps_bitmap_info.auc_mac_addr[MAC_ADDR_4], ps_bitmap_info.auc_mac_addr[MAC_ADDR_5], ps_bitmap_info.ps_bitmap);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_SET_USER_BITMAP_CMD, sizeof(ps_bitmap_info));

    /* 设置配置命令参数 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(ps_bitmap_info),
        (const void *)&ps_bitmap_info, sizeof(ps_bitmap_info))) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_chba_set_user_bitmap::memcpy fail!");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(ps_bitmap_info),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_set_user_bitmap::err[%d]!}", ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}
/* 设置CHBA vap bitmap */
uint32_t wal_hipriv_chba_set_vap_bitmap(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t  vap_bitmap;
    int32_t l_ret;
    uint32_t ret;
    /*
        设置用户bitmap的配置命令: hipriv "vap0 set_vap_bitmap bitmap"
        该命令针对某一个VAP
    */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_set_vap_bitmap::get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }
    vap_bitmap = (uint32_t)oal_strtol(ac_name, NULL, 0);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_set_vap_bitmap::vap bitmap [0x%x]!}", vap_bitmap);
    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    *(uint32_t *)(st_write_msg.auc_value) = vap_bitmap;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_SET_VAP_BITMAP_CMD, sizeof(uint32_t));

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_set_vap_bitmap::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_chba_log_level
 * 功能描述  : chba维测日志开关
 * 1.日    期  : 2021年9月6日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_chba_log_level(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint8_t chba_log_level;
    int32_t send_cfg_ret;
    uint32_t get_cmd_ret;
    /*
        设置用户chba维测日志等级的配置命令: hipriv "vap0 chba_log_level [val]"
        [val]取值0-2, 0为关闭全部chba相关日志，2为开启全部chba相关日志
        该命令针对某一个VAP
    */
    get_cmd_ret = wal_get_cmd_one_arg(param, cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (get_cmd_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_chba_log_level:get_cmd_one_arg return err_code [%d]!}", get_cmd_ret);
        return get_cmd_ret;
    }

    chba_log_level = (uint8_t)oal_atoi(cmd_name);
    if (chba_log_level >= CHBA_DEBUG_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_log_level:chba_log_level[%d] invalid!}", chba_log_level);
        return OAL_FAIL;
    }
    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    *(uint8_t *)(st_write_msg.auc_value) = chba_log_level;
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_LOG_LEVEL, sizeof(uint8_t));

    send_cfg_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
        (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (send_cfg_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_log_level:return err code [%d]!}", send_cfg_ret);
        return (uint32_t)send_cfg_ret;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_hipriv_chba_set_ps_thres
 * 功能描述  : chba配置低功耗流量模型档位切换门限的接口
 * 1.日    期  : 2021年12月07日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_chba_set_ps_thres(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int32_t send_cfg_ret;
    uint32_t get_cmd_ret;
    int8_t cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    chba_ps_thres_config_stru *ps_thres_cfg = NULL;

    /* 设置chba低功耗档位切换门限命令: hipriv "chba0 set_chba_ps_thres [thres_type] [val]"
     * [thres_type]  0：配置自第三档升至第二档的门限
     *               1：配置自第二档升至第一档的门限
     *               2：配置自第一档降至第二档的门限
     *               3：配置自第二档降至第三档的门限
     * [val] 要更新的对应档位新切换门限, 单位(100ms)
     *
     * 该命令针对chba vap
     */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_SET_PS_THRES, sizeof(chba_ps_thres_config_stru));
    ps_thres_cfg = (chba_ps_thres_config_stru *)st_write_msg.auc_value;

    get_cmd_ret = wal_get_cmd_one_arg(param, cmd, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (get_cmd_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_chba_set_ps_thres:get thres_type return err_code [%d]!}", get_cmd_ret);
        return get_cmd_ret;
    }
    ps_thres_cfg->thres_type = (uint8_t)oal_atoi(cmd);

    param = param + off_set;
    get_cmd_ret = wal_get_cmd_one_arg(param, cmd, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (get_cmd_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_chba_set_ps_thres:get thres_val return err_code [%d]!}", get_cmd_ret);
        return get_cmd_ret;
    }
    ps_thres_cfg->thres_val = (uint8_t)oal_atoi(cmd);
    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    send_cfg_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(chba_ps_thres_config_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (send_cfg_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_set_ps_thres:return err code [%d]!}", send_cfg_ret);
        return (uint32_t)send_cfg_ret;
    }
    return OAL_SUCC;
}
/*
 * 函 数 名  : wal_hipriv_chba_island_chan_switch
 * 功能描述  : chba全岛切信道命令
 * 1.日    期  : 2021年9月14日
 *   作    者  : wifi7
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_chba_island_chan_switch(oal_net_device_stru *net_dev, int8_t *param)
{
    int32_t send_ret;
    uint32_t off_set;
    uint32_t ret;
    int8_t cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    wal_msg_write_stru st_write_msg;
    mac_chba_adjust_chan_info *cfg_channel_param = (mac_chba_adjust_chan_info *)(st_write_msg.auc_value);

    /*
        切换一个chba岛内全部设备的信道: hipriv "vap0 chba_island_chan_switch [chan_num] [bandwidth] [switch_type]"
        [chan_num]:信道号(CHBA暂只支持5G)
        [bandwidth]：信道带宽
                     0 - WLAN_BAND_WIDTH_20M
                     1 - WLAN_BAND_WIDTH_40PLUS
                     2 - WLAN_BAND_WIDTH_40MINUS
                     3 - WLAN_BAND_WIDTH_80PLUSPLUS
                     4 - WLAN_BAND_WIDTH_80PLUSMINUS
                     5 - WLAN_BAND_WIDTH_80MINUSPLUS
                     6 - WLAN_BAND_WIDTH_80MINUSMINUS
                     7 - WLAN_BAND_WIDTH_40M
                     8 - WLAN_BAND_WIDTH_80M
        [switch type]: 切换类型
                     0-----建链切信道：岛主和非岛主可以直接发起
                     1—----卡顿切信道：岛主直接发起，非岛主先向岛主发csn req帧，由岛主统一切。
        如 hipriv "chba0 chba_island_chan_switch 149 0 0" 即表示全岛切换至20M的149信道

        CHBA信道配置参照 5G全频段待选信道表 g_aus_channel_candidate_list_5g
    */
    memset_s(cfg_channel_param, sizeof(mac_chba_adjust_chan_info), 0, sizeof(mac_chba_adjust_chan_info));
    /* 获取切换信道号 */
    ret = wal_get_cmd_one_arg(param, cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_chba_island_chan_switch:fail to get chann_number}");
        return ret;
    }
    param += off_set;
    cfg_channel_param->chan_number = (uint8_t)oal_atoi(cmd_name);

    /* 获取切换信道带宽 */
    ret = wal_get_cmd_one_arg(param, cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_chba_island_chan_switch:fail to get bandwidth}");
        return ret;
    }
    param += off_set;
    cfg_channel_param->bandwidth = (uint8_t)oal_atoi(cmd_name);
    if (cfg_channel_param->bandwidth >= WLAN_BAND_WIDTH_BUTT) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_island_chan_switch:invalid bandwidth[%d]}",
            cfg_channel_param->bandwidth);
        return OAL_SUCC;
    }
    /* 获取switch type */
    ret = wal_get_cmd_one_arg(param, cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_chba_island_chan_switch:fail to get bandwidth}");
        return ret;
    }
    param += off_set;
    cfg_channel_param->switch_type = (uint8_t)oal_atoi(cmd_name);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_ADJUST_ISLAND_CHAN, sizeof(mac_chba_adjust_chan_info));

    send_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_chba_adjust_chan_info), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (send_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_chba_island_chan_switch:err[%d]!}", ret);
        return (uint32_t)send_ret;
    }
    oam_warning_log3(0, OAM_SF_ANY, "{wal_hipriv_chba_island_chan_switch:uc_channel[%d], bandwith[%d] switch type[%d]}",
        cfg_channel_param->chan_number, cfg_channel_param->bandwidth, cfg_channel_param->switch_type);
    return OAL_SUCC;
}

/* 解析chba特性开关命令 */
static uint32_t wal_hipriv_analyze_chba_feature_switch_params(int8_t *param, mac_chba_feature_switch_stru *cfg_params)
{
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t val[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint8_t status;
    uint32_t offset = 0;
    oal_bool_enum_uint8 cmd_updata = OAL_FALSE;

    do {
        if (wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset) != OAL_SUCC && offset != 0) {
            oam_warning_log0(0, 0, "{wal_hipriv_set_chba_feature_switch::cmd format err!}");
            return OAL_FAIL;
        }
        param += offset;

        if (cmd_updata == OAL_FALSE) {
            cmd_updata = OAL_TRUE;
        } else if (offset == 0) {
            break;
        }

        if (wal_get_cmd_one_arg(param, val, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &offset) != OAL_SUCC || !isdigit(val[0])) {
            oam_warning_log0(0, 0, "{wal_hipriv_set_chba_feature_switch::cmd para err!}");
            return OAL_FAIL;
        }
        param += offset;
        offset = 0;

        status = (uint8_t)oal_atoi(val);
        if (status > 1) {
            oam_warning_log1(0, 0, "CMD para error, input[%d]'!!}", status);
            return OAL_FAIL;
        }

        if (oal_strcmp("chanswitch_enable", name) == 0) {
            cfg_params->cmd_bitmap |= BIT(CHBA_CHAN_SWITCH);
            cfg_params->chan_switch_enable = status;
        } else if (oal_strcmp("ps_enable", name) == 0) {
            cfg_params->cmd_bitmap |= BIT(CHBA_PS_SWITCH);
            cfg_params->ps_enable = status;
        } else {
            oam_warning_log0(0, 0, "{CMD format::sh hipriv.sh 'chba0 chba_feature_switch xxx not support'!!}");
            return OAL_FAIL;
        }
    } while (*param != '\0');

    return OAL_SUCC;
}

/* 设置chba特性开关 */
uint32_t wal_hipriv_set_chba_feature_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_chba_feature_switch_stru cfg_params;
    int32_t l_ret;

    memset_s(&cfg_params, sizeof(cfg_params), 0, sizeof(cfg_params));

    /* sh hipriv.sh "chba0 chba_feature_switch [chanswitch_enable 0|1] [ps_enable 0|1]" */
    if (wal_hipriv_analyze_chba_feature_switch_params(pc_param, &cfg_params) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_CHBA_FEATURE_SWITCH, sizeof(cfg_params));

    /* 设置配置命令参数 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(cfg_params),
        (const void *)&cfg_params, sizeof(cfg_params))) {
        oam_warning_log0(0, OAM_SF_ANY, "set_chba_feature_switch::memcpy fail!");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
        WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(cfg_params),
        (uint8_t *)&st_write_msg,
        OAL_FALSE,
        NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{set_chba_feature_switch::err[%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 配置chba模块初始化
 * 日    期  : 2021年10月30日
 */
int32_t wal_ioctl_chba_module_init(oal_net_device_stru *net_dev, int8_t *command,
    wal_wifi_priv_cmd_stru priv_cmd)
{
    int32_t ret;
    uint8_t device_type;

    ret = wal_ioctl_judge_input_param_length(priv_cmd, CMD_CHBA_MODULE_INIT_LEN, 1);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    /* 获取device type */
    device_type = *(command + CMD_CHBA_MODULE_INIT_LEN + 1);
    /* 调用初始化接口 */
    hmac_chba_module_init(device_type);
    /* 由于supplicant初始化完成后会下发netdev open的命令，导致chba默认up，因此在这里down掉 */
    oam_warning_log1(0, OAM_SF_ANY, "wal_ioctl_chba_module_init::init completed type[%d], stop ap.", device_type);
    wal_netdev_stop_ap(net_dev);
    wal_del_vap_try_wlan_pm_close();
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置电量
 * 日    期  : 2021年10月30日
 */
int32_t wal_ioctl_set_battery_level(oal_net_device_stru *net_dev, int8_t *command,
    wal_wifi_priv_cmd_stru priv_cmd)
{
    int32_t ret;
    uint8_t battery_level;

    ret = wal_ioctl_judge_input_param_length(priv_cmd, CMD_SET_BATTERY_LEVEL_LEN, 1);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_set_battery_level::CMD_SET_BATTERY_LEVEL cmd len err.}");
        return -OAL_EFAIL;
    }

    /* 获取电量等级 */
    battery_level = *(command + CMD_SET_BATTERY_LEVEL_LEN + 1);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_battery_level::CMD_SET_BATTERY_LEVEL battery[%d].}", battery_level);

    /* 调用电量更新接口 */
    hmac_chba_battery_update_proc(battery_level);

    return OAL_SUCC;
}

/*
 * 功能描述  : chba信道调整：建链切信道、干扰切信道(根据switch type和角色判断是否要请求信道信息)
 * 日    期  : 2021年10月30日
 */
int32_t wal_ioctl_chba_channel_adjust(oal_net_device_stru *net_dev, uint8_t *command,
    wal_wifi_priv_cmd_stru priv_cmd)
{
    int32_t ret;
    mac_chba_adjust_chan_info chan_adjust_info = {0};
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;

    mac_vap = (mac_vap_stru *)oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        return -OAL_EFAIL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return -OAL_EFAIL;
    }

    ret = wal_ioctl_judge_input_param_length(priv_cmd, CMD_SET_CHAN_ADJUST_LEN, sizeof(chan_adjust_info));
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }
    ret = memcpy_s(&chan_adjust_info, sizeof(mac_chba_adjust_chan_info),
        command + CMD_SET_CHAN_ADJUST_LEN + 1, sizeof(mac_chba_adjust_chan_info));
    if (ret != EOK) {
        return -OAL_EFAIL;
    }

    hmac_chba_adjust_channel_proc(hmac_vap, &chan_adjust_info);

    return OAL_SUCC;
}


#endif

