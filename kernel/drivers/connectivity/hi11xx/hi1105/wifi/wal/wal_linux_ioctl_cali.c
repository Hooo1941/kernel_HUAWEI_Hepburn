/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux ioctl 校准配置调试命令
 * 作    者 : wifi
 * 创建日期 : 2022年11月26日
 */
#include "oal_ext_if.h"
#include "oal_util.h"
#include "oam_ext_if.h"
#include "oam_event_wifi.h"
#include "frw_ext_if.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "mac_vap.h"
#include "hmac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_linux_ioctl_debug.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_IOCTL_CALI_C

/*
 * 功能描述  : wal层动态校准参数配置函数
 * 1.日    期  : 2016年09月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_dyn_cali_cfg(oal_net_device_stru *pst_net_dev, int8_t *puc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_ioctl_dyn_cali_param_stru *pst_dyn_cali_param = NULL;
    wal_ioctl_cmd_tbl_stru cyn_cali_cfg;
    uint32_t ret;
    uint8_t map_index = 0;

    pst_dyn_cali_param = (mac_ioctl_dyn_cali_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_dyn_cali_param, sizeof(mac_ioctl_dyn_cali_param_stru),
             0, sizeof(mac_ioctl_dyn_cali_param_stru));

    if (oal_unlikely(oal_any_null_ptr2(pst_net_dev, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_dyn_cali_cfg::pst_cfg_net_dev or puc_param null ptr error }\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = wal_get_cmd_one_arg(puc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_dyn_cali_cfg::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    /* 寻找匹配的命令 */
    for (map_index = 0; map_index < g_dyn_cali_cfg_map_size; map_index++) {
        cyn_cali_cfg = g_ast_dyn_cali_cfg_map[map_index];
        if (0 == oal_strcmp(cyn_cali_cfg.pc_name, ac_name)) {
            break;
        }
    }
    /* 没有找到对应的命令，则报错 */
    if (map_index == g_dyn_cali_cfg_map_size) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_dyn_cali_cfg::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_dyn_cali_param->en_dyn_cali_cfg = g_ast_dyn_cali_cfg_map[map_index].idx;
    /* 获取参数配置值 */
    ret = wal_get_cmd_one_arg(puc_param + off_set, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
        /* 记录参数配置值 */
        pst_dyn_cali_param->us_value = (uint16_t)oal_atoi(ac_name);
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_dyn_cali_cfg::wal_get_cmd_one_arg [%d]!}\r\n", pst_dyn_cali_param->us_value);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DYN_CALI_CFG, sizeof(uint32_t));

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_cyn_cali_set_dscr_interval::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return ret;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置fem低功耗开关0 :关闭1: 打开2:强制打开
 * 1.日    期  : 2019年1月22日
 *   作    者  : wifi
 *   修改内容  : 新增函数
 */
uint32_t wal_hipriv_fem_lowpower(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t ret, offset, cmd_id;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t lowpower_switch;

    /* 获取开关 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_fem_lowpower::first para return err_code [%d]!}\r\n", ret);
        return ret;
    }

    lowpower_switch = oal_atoi((const int8_t *)ac_name);

    ret = wal_get_cmd_id("fem_lowpower", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_fem_lowpower:find fem_lowpower cmd is fail");
        return ret;
    }

    return wal_process_cmd_params(pst_net_dev, cmd_id, &lowpower_switch);
}
#ifdef WIFI_DEBUG_ENABLE
/*
 * 函 数 名   : wal_hipriv_cali_debug
 * 功能描述   : 校准调试命令
 * 1.日    期   : 2020年06月23日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t wal_hipriv_cali_debug(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru       write_msg;
    uint32_t                 offset = 0;
    int8_t                   ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t                 result;
    mac_cfg_cali_param_stru *cali_cfg_para = NULL;

    uint8_t                  arg_idx = 0;
    uint8_t                  src_str_len;
    uint16_t                 para_len;
    uint16_t                 curr_pos = 0;

    /* 设置配置命令参数 */
    cali_cfg_para = (mac_cfg_cali_param_stru *)(write_msg.auc_value);

    para_len = (uint16_t)OAL_STRLEN(pc_param) + 1;
    while (curr_pos < para_len) {
        /* 获取参数 */
        result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (result == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
            break;
        }

        if (result != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CFG, "wal_hipriv_cali_debug::wal_get_cmd_one_arg get fail");
            return result;
        }

        src_str_len = OAL_STRLEN(ac_name) + 1;
        if (src_str_len > WLAN_CALI_CFG_CMD_MAX_LEN) {
            oam_error_log1(0, OAM_SF_CFG, "wal_hipriv_cali_debug::args len [%d] overflow", src_str_len);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        if (memcpy_s((int8_t*)(cali_cfg_para->input_args[arg_idx]), WLAN_CALI_CFG_CMD_MAX_LEN, ac_name, src_str_len)
            != EOK) {
            return OAL_FAIL;
        }
        pc_param += offset;
        curr_pos += offset;
        arg_idx++;
    }
    cali_cfg_para->input_args_num = arg_idx;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_CALI_CFG, sizeof(mac_cfg_cali_param_stru));
    result = (uint32_t)wal_send_cfg_event(net_dev,
                                          WAL_MSG_TYPE_WRITE,
                                          WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_cali_param_stru),
                                          (uint8_t *)&write_msg,
                                          OAL_FALSE, NULL);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_cali_debug::return err code %d!}\r\n", result);
    }

    return result;
}
uint32_t wal_hipriv_send_cfg_string_data(oal_net_device_stru *net_dev, int8_t *pc_param, uint32_t len,
    wlan_cfgid_enum_uint16 cfg_id)
{
    wal_msg_write_stru st_write_msg = {0};
    int32_t l_ret;
    uint16_t us_len;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), pc_param, len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_send_cfg_string_data::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&st_write_msg, cfg_id, us_len);

    l_ret = wal_send_cfg_event(net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_cfg_string_data::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
/*
 * 1.日    期  : 2013年9月6日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_dpd_cfg(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    return wal_hipriv_send_cfg_string_data(pst_net_dev, pc_param, OAL_STRLEN(pc_param), WLAN_CFGID_DPD);
}

#ifdef _PRE_WLAN_FEATURE_NRCOEX
/*
 * 功能描述  : 设置nrcoex调试参数
 * 1.日    期  : 2019年3月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_nrcoex_cfg_test(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    return wal_hipriv_send_cfg_string_data(pst_net_dev, pc_param, OAL_STRLEN(pc_param), WLAN_CFGID_NRCOEX_TEST);
}
#endif

/*
 * 功能描述  : 设置目标功率，要求功率值按照扩大10倍来输入,例如最大功率要限制为20，输入200;限制18.5,输入185
 * 1.日    期  : 2015年12月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_txpower(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    int32_t l_pwer;
    uint32_t off_set;
    int8_t ac_val[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_idx = 0;
    wal_msg_stru *pst_rsp_msg = NULL;

    ret = wal_get_cmd_one_arg(pc_param, ac_val, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_txpower::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ret);
        return ret;
    }
    if (ac_val[0] == '-') {
        l_idx++;
    }

    /* 输入命令合法性检测 */
    while (ac_val[l_idx] != '\0') {
        if (isdigit(ac_val[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    l_pwer = oal_atoi(ac_val);
    /* 超过10倍最大传输功率或1/10最小传输功率，都是异常参数 */
    if (l_pwer >= WLAN_MAX_TXPOWER * 10 || l_pwer <= WLAN_MIN_TXPOWER * 10) { /* 参数异常: 功率限制大于1W */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::invalid argument!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::val[%d]!}", l_pwer);
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_TX_POWER, sizeof(int32_t));
    *((int32_t *)(st_write_msg.auc_value)) = l_pwer;
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::return err code %d!}", l_ret);
        return (uint32_t)l_ret;
    }
    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_bw fail, err code[%u]!}\r\n", ret);
    }

    return ret;
}

uint32_t wal_hipriv_sar_slide_cfg(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t value[NUM_10_BITS];
    uint32_t offset;
    uint16_t idx = 0;
    int32_t ret;
    while ((*param != '\0') && (idx < NUM_10_BITS)) {
        ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (ret != OAL_SUCC) {
            break;
        }
        param += offset;
        value[idx] = (int8_t)oal_atoi(name);
        idx++;
    }
    if ((idx == 0)  || (idx == NUM_10_BITS)) {
        oam_error_log1(0, OAM_SF_CFG, "{wal_hipriv_sar_slide_cfg::err idx[%d]!}", idx);
        return OAL_FAIL;
    }
    memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), value, sizeof(value));
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SAR_SLIDE_DEBUG, sizeof(value));
    ret = (uint32_t)wal_send_cfg_event(net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(value),
                                       (uint8_t *)&write_msg,
                                       OAL_FALSE, NULL);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_sar_slide_cfg::return err code %d!}", ret);
    }

    return ret;
}
#endif
