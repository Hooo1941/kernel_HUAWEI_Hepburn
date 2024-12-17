/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux ioctl mcm配置调试命令
 * 作    者 : wifi
 * 创建日期 : 2022年11月26日
 */

#include "wal_linux_ioctl_debug.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_IOCTL_MCM_C

#ifdef _PRE_WLAN_FEATURE_M2S
#define MCM_CONFIG_CHAIN_STEP 10
#define MCM_CHAIN_NUM_MAX 4
/*
 * 功能描述  : MSS配置命令参数处理
 * 1.日    期  : 2019.10.15
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_mss_param_handle(mac_m2s_mgr_stru *pst_m2s_mgr, int8_t *pc_param,
    uint32_t *cmd_off_set, uint8_t *name)
{
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    /* 1.获取第二参数: 期望切换到的状态 */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_mss_param_handle::wal_get_cmd_one_arg2 return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pst_m2s_mgr->pri_data.mss_mode.en_mss_on = (uint8_t)oal_atoi(ac_name);
    if (pst_m2s_mgr->pri_data.mss_mode.en_mss_on >= 2) { /* en_mss_on 为bool型，值为0 or 1，不能大于等于2 */
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_mss_param_handle::pst_m2s_mgr->pri_data.mss_mode.en_mss_on error [%d] (0 or 1)!}\r\n",
            pst_m2s_mgr->pri_data.mss_mode.en_mss_on);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : MODEM配置命令参数处理
 * 1.日    期  : 2019.10.15
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_modem_param_handle(mac_m2s_mgr_stru *pst_m2s_mgr, int8_t *pc_param,
    uint32_t *cmd_off_set, uint8_t *name)
{
#ifdef _PRE_WLAN_FEATURE_M2S_MODEM
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    if (g_wlan_spec_cfg->feature_m2s_modem_is_open) {
        return OAL_FAIL;
    }
    /* 1.获取第二参数: 期望切换到的状态 */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_mss_param_handle::wal_get_cmd_one_arg2 return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pst_m2s_mgr->pri_data.modem_mode.en_m2s_state = (uint8_t)oal_atoi(ac_name);

    return OAL_SUCC;
#endif
}

/*
 * 功能描述  : RSSI配置命令参数处理
 * 1.日    期  : 2019.10.15
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_rssi_param_handle(mac_m2s_mgr_stru *pst_m2s_mgr, int8_t *pc_param,
    uint32_t *cmd_off_set, uint8_t *name)
{
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    /* 1.获取第二个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_set_m2s_switch::wal_get_cmd_one_arg2 return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pst_m2s_mgr->pri_data.rssi_mode.uc_opt = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_set_m2s_switch::wal_get_cmd_one_arg2 return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pst_m2s_mgr->pri_data.rssi_mode.c_value = (int8_t)oal_atoi(ac_name);

    return OAL_SUCC;
}
/*
 * 功能描述  : BTCOEX配置命令参数处理
 * 1.日    期  : 2020年05月07日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_btcoex_param_handle(mac_m2s_mgr_stru *m2s_mgr, int8_t *param,
    uint32_t *cur_cmd_off_set, uint8_t *cur_name)
{
    uint32_t ret;
    int8_t   name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t cmd_off_set = 0;

    /* 1.获取第二参数: 期望切换到的状态 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &cmd_off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_btcoex_param_handle::get arg2 return err_code [%d]!}", ret);
        return ret;
    }
    m2s_mgr->pri_data.btcoex_mode.chain_apply = (uint8_t)oal_atoi(name);

    /* 偏移，取下一个参数 */
    param = param + cmd_off_set;
    /* 2.获取第三参数: 优先级 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &cmd_off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_btcoex_param_handle::get arg3 return err_code [%d]!}", ret);
        return ret;
    }
    m2s_mgr->pri_data.btcoex_mode.ant_occu_prio = (uint8_t)oal_atoi(name);

    return OAL_SUCC;
}

/*
 * 功能描述  :mp16中第二个参数的含义是使用字符串表示的二进制如C2SISO为0100
 * 1.日    期  : 2020年3月19日
 *   作    者  : wifi
 *   修改内容  : 新增函数
 */
uint8_t wal_hipriv_m2s_switch_get_mcm_chain(uint16_t param)
{
    uint8_t chain_index;
    uint8_t mcm_chain = 0;

    /* 切换命令第二个参数含义为使用字符串表示的二进制如C2SISO为0100 */
    for (chain_index = 0; chain_index < MCM_CHAIN_NUM_MAX; chain_index++) {
        if ((param % MCM_CONFIG_CHAIN_STEP) != 0) {
            mcm_chain |= ((uint8_t)(1 << chain_index));
        }
        param /= MCM_CONFIG_CHAIN_STEP;
    }
    return mcm_chain;
}

/*
 * 功能描述  : 处理m2s切换参数
 * 1.日    期  : 2020年3月19日
 *   作    者  : wifi
 *   修改内容  : 新增函数
 */
uint32_t wal_hipriv_m2s_switch_param_handle(mac_m2s_mgr_stru *m2s_mgr, int8_t *param,
    uint32_t *cmd_off_set, uint8_t *name)
{
    uint32_t ret;
    int32_t chip_type = get_mpxx_subchip_type();

    ret = wal_get_cmd_one_arg((uint8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, cmd_off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_m2s_switch_param_handle::wal_get_cmd_one_arg2 \
            return err_code [%d]!}\r\n", ret);
        return ret;
    }

    if (chip_type >= BOARD_VERSION_MP16) {
        m2s_mgr->pri_data.test_mode.uc_m2s_state = wal_hipriv_m2s_switch_get_mcm_chain((uint16_t)oal_atoi(name));
    } else {
        m2s_mgr->pri_data.test_mode.uc_m2s_state = (uint8_t)oal_atoi(name);
    }

    /* 偏移，取下一个参数 */
    param = param + (*cmd_off_set);

    /* 2.获取第三个参数:主路还是辅路 0为主路 1为辅路  */
    ret = wal_get_cmd_one_arg((uint8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, cmd_off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_m2s_switch_param_handle::wal_get_cmd_one_arg3 \
            return err_code [%d]!}\r\n", ret);
        return ret;
    }

    m2s_mgr->pri_data.test_mode.uc_master_id = (uint8_t)oal_atoi(name);
    if (m2s_mgr->pri_data.test_mode.uc_master_id >= HAL_DEVICE_ID_BUTT) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_m2s_switch_param_handle:: \
            pst_m2s_mgr->pri_data.test_mode.uc_master_id error [%d] 0/1!}\r\n",
            m2s_mgr->pri_data.test_mode.uc_master_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* true: 硬切换测试模式采用默认软切换配置 */
    /* false: 业务切换，软切换测试模式采用默认软切换配置 */
    m2s_mgr->pri_data.test_mode.en_m2s_type = (m2s_mgr->en_cfg_m2s_mode == MAC_M2S_MODE_HW_TEST) ?
        WLAN_M2S_TYPE_HW : WLAN_M2S_TYPE_SW;

    /* 标识业务类型 */
    m2s_mgr->pri_data.test_mode.uc_trigger_mode =
        (m2s_mgr->en_cfg_m2s_mode == MAC_M2S_MODE_DELAY_SWITCH) ?
        WLAN_M2S_TRIGGER_MODE_COMMAND : WLAN_M2S_TRIGGER_MODE_TEST;
    return OAL_SUCC;
}

uint32_t wal_hipriv_m2s_switch_null_func(mac_m2s_mgr_stru *m2s_mgr, int8_t *param,
    uint32_t *cmd_off_set, uint8_t *name)
{
    return OAL_SUCC;
}

wal_hipriv_m2s_switch_func g_m2s_switch_param_handle[MAC_M2S_MODE_BUTT] = {
    wal_hipriv_m2s_switch_null_func,
    wal_hipriv_mss_param_handle,
    wal_hipriv_m2s_switch_param_handle,
    wal_hipriv_m2s_switch_param_handle,
    wal_hipriv_m2s_switch_param_handle,
    wal_hipriv_rssi_param_handle,
    wal_hipriv_modem_param_handle,
    wal_hipriv_btcoex_param_handle
};

/*
 * 功能描述  : device下所有vap都要切换到siso或者mimo工作方式，涉及到smps和工作模式通知逻辑
 *             hipriv "Hisilicon0 set_m2s_switch  0/1/2/3(参数查询/配置模式/切换模式/常发常收测试模式)
 *             0/1(软切换/硬切换) 0/1(siso/mimo) 1/2/3(phy chain) 0/1(rf chain) 1/2(tx single chain)"
 * 1.日    期  : 2016年9月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_m2s_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    mac_m2s_mgr_stru *pst_m2s_mgr;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* st_write_msg作清零操作 */
    memset_s(&st_write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    /* 设置配置命令参数 */
    pst_m2s_mgr = (mac_m2s_mgr_stru *)st_write_msg.auc_value;

    /* 1.获取第一个参数: mode */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_set_m2s_switch::wal_get_cmd_one_arg1 return err_code [%d]!}", ret);
        return ret;
    }

    pst_m2s_mgr->en_cfg_m2s_mode = (mac_m2s_mode_enum_uint8)oal_atoi(ac_name);
    if (pst_m2s_mgr->en_cfg_m2s_mode >= MAC_M2S_MODE_BUTT) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_set_m2s_switch: cfg_m2s_mode[%d] error!}",
            pst_m2s_mgr->en_cfg_m2s_mode);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    ret = g_m2s_switch_param_handle[pst_m2s_mgr->en_cfg_m2s_mode](pst_m2s_mgr, pc_param, &off_set, ac_name);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_M2S_SWITCH, sizeof(mac_m2s_mgr_stru));

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_m2s_mgr_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_set_m2s_stitch::wal_hipriv_reset_device return err code = [%d].}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
/*
 * 函 数 名  : wal_ioctl_set_lp_miracast
 * 功能描述  : 投屏低功耗命令下发
 * 1.日    期  : 2023年3月16日
 *   修改内容  : 新生成函数
 */
int32_t wal_ioctl_set_lp_miracast(oal_net_device_stru *net_dev, uint8_t is_enable)
{
    wal_msg_write_stru write_msg = {0};
    uint8_t *lp_miracast = NULL;
    int32_t ret;

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_LP_MIRACAST, sizeof(uint8_t));

    lp_miracast = (uint8_t *)(write_msg.auc_value);
    *lp_miracast = is_enable;

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_ioctl_set_sceen_lowpower::wal_send_cfg_event err:[%d]}", ret);
        return ret;
    }

    return OAL_SUCC;
}

int32_t wal_ioctl_get_m2s_state(oal_net_device_stru *net_dev)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_GET_M2S_STATE, 0);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH, (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_ioctl_set_sceen_lowpower::wal_send_cfg_event err:[%d]}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif
