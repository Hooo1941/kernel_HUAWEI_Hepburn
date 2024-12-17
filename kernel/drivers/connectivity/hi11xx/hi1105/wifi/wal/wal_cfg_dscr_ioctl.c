/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :特性相关命令
 * 创建日期 : 2022年11月19日
 */

#include "wal_linux_ioctl.h"
#include "hmac_roam_main.h"
#include "wlan_chip_i.h"
#include "hmac_sae.h"
#include "wal_cfg_release_ioctl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_DSCR_CFG_IOCTL_C

#ifdef WIFI_DEBUG_ENABLE
const int8_t *g_pauc_tx_pow_param_name[WAL_TX_POW_PARAM_BUTT] = {
    "rf_reg_ctl",
    "fix_level",
    "mag_level",
    "ctl_level",
    "amend",
    "no_margin",
    "show_log",
    "sar_level",
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    "tas_pwr_ctrl",
    "tas_rssi_measure",
    "tas_ant_switch",
#endif
    "show_tpc_tbl_gain",
    "pow_save",
#ifdef _PRE_WLAN_FEATURE_FULL_QUAN_PROD_CAL
    "pdinfo",
#endif
    "tpc_idx",
};
OAL_STATIC OAL_INLINE void protocol_debug_cmd_format_info(void)
{
    oam_warning_log0(0, OAM_SF_ANY, "{CMD format::sh hipriv.sh 'wlan0 protocol_debug\
                    [band_force_switch 0|1|2(20M|40M+|40M-)]\
                    [2040_ch_swt_prohi 0|1]\
                    [40_intol 0|1]'!!}\r\n");
    oam_warning_log0(0, OAM_SF_ANY, "{[csa 0(csa mode) 1(csa channel) 10(csa cnt) 1(debug  flag, \
                     0:normal channel channel,1:only include csa ie 2:cannel debug)] \
                     [2040_user_switch 0|1]'!!}\r\n");
    oam_warning_log0(0, OAM_SF_ANY, "[lsig 0|1]'!!}\r\n");
}

OAL_STATIC uint32_t wal_protocol_get_csa_mode(int8_t *pc_param,
    mac_protocol_debug_switch_stru *pst_debug_info, uint32_t *off_set)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};

    /* 解析csa mode */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_get_csa_mode::err[%d].}", ret);
        return ret;
    }

    pst_debug_info->st_csa_debug_bit3.en_mode = ((uint8_t)oal_atoi(ac_value) & BIT0);
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_protocol_get_csa_chn(int8_t *pc_param,
    mac_protocol_debug_switch_stru *pst_debug_info, uint32_t *off_set)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};

    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_get_csa_chn::err[%d]}", ret);
        return ret;
    }
    pst_debug_info->st_csa_debug_bit3.uc_channel = (uint8_t)oal_atoi(ac_value);
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_protocol_get_csa_bw(int8_t *pc_param,
    mac_protocol_debug_switch_stru *pst_debug_info, uint32_t *off_set)
{
    uint32_t ret;
    uint8_t uc_value;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};

    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_get_csa_bw::err[%d]}", ret);
        return ret;
    }
    uc_value = (uint8_t)oal_atoi(ac_value);
    if (uc_value >= WLAN_BAND_WIDTH_BUTT) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_get_csa_bw::invalid bandwidth=%d,return.}", uc_value);
        return OAL_FAIL;
    }
    pst_debug_info->st_csa_debug_bit3.en_bandwidth = uc_value;

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_protocol_get_csa_cnt(int8_t *pc_param,
    mac_protocol_debug_switch_stru *pst_debug_info, uint32_t *off_set)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};

    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_get_csa_cnt::err[%d].}", ret);
        return ret;
    }
    pst_debug_info->st_csa_debug_bit3.uc_cnt = (uint8_t)oal_atoi(ac_value);

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_protocol_get_csa_debug_flag(int8_t *pc_param,
    mac_protocol_debug_switch_stru *pst_debug_info, uint32_t *off_set)
{
    uint32_t ret;
    uint8_t uc_value;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};

    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_get_csa_debug_flag::err[%d]}", ret);
        return ret;
    }

    uc_value = (uint8_t)oal_atoi(ac_value);
    if (uc_value >= MAC_CSA_FLAG_BUTT) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_get_csa_debug_flag::invalid debug flag=%d,return.}", uc_value);
        return OAL_FAIL;
    }
    pst_debug_info->st_csa_debug_bit3.en_debug_flag = uc_value;

    return OAL_SUCC;
}

/*
 * 功能描述  : 解析csa命令参数
 * 1.日    期  : 2017年4月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_protocol_debug_parase_csa_cmd(int8_t *pc_param,
    mac_protocol_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    uint32_t off_set = 0;

    *pul_offset = 0;

    /* 解析csa mode */
    ret = wal_protocol_get_csa_mode(pc_param, pst_debug_info, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *pul_offset += off_set;
    pc_param += off_set;

    /* 解析csa channel */
    ret = wal_protocol_get_csa_chn(pc_param, pst_debug_info, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *pul_offset += off_set;
    pc_param += off_set;

    /* 解析bandwidth */
    ret = wal_protocol_get_csa_bw(pc_param, pst_debug_info, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *pul_offset += off_set;
    pc_param += off_set;

    /* 解析csa cnt */
    ret = wal_protocol_get_csa_cnt(pc_param, pst_debug_info, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *pul_offset += off_set;
    pc_param += off_set;

    /* 解析debug flag */
    ret = wal_protocol_get_csa_debug_flag(pc_param, pst_debug_info, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *pul_offset += off_set;
    pc_param += off_set;

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_set_protocol_debug_analyze(
    mac_protocol_debug_switch_stru *protocol_debug, int8_t **input_param, int8_t *name)
{
    uint32_t off_set = 0;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t ret;
    int8_t *param = *input_param;
    /* 取命令配置值 */
    ret = wal_get_cmd_one_arg(param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if ((ret != OAL_SUCC) || oal_value_not_in_valid_range(ac_value[0], '0', '9')) {
        protocol_debug_cmd_format_info();
        return ret;
    }
    param += off_set;
    if (oal_strcmp("band_force_switch", name) == 0) {
        protocol_debug->en_band_force_switch_bit0 = ((uint8_t)oal_atoi(ac_value));
        protocol_debug->cmd_bit_map |= BIT0;
    } else if (oal_strcmp("2040_ch_swt_prohi", name) == 0) {
        /* 填写结构体 */
        protocol_debug->en_2040_ch_swt_prohi_bit1 = ((uint8_t)oal_atoi(ac_value)) & BIT0;
        protocol_debug->cmd_bit_map |= BIT1;
    } else if (oal_strcmp("40_intol", name) == 0) {
        /* 填写结构体 */
        protocol_debug->en_40_intolerant_bit2 = ((uint8_t)oal_atoi(ac_value)) & BIT0;
        protocol_debug->cmd_bit_map |= BIT2;
    } else if (oal_strcmp("lsig", name) == 0) {
        /* 填写结构体 */
        protocol_debug->en_lsigtxop_bit5 = ((uint8_t)oal_atoi(ac_value)) & BIT0;
        protocol_debug->cmd_bit_map |= BIT5;
    } else {
        protocol_debug_cmd_format_info();
        return OAL_FAIL;
    }

    *input_param = param;
    return OAL_SUCC;
}
OAL_STATIC uint32_t wal_hipriv_set_protocol_debug_para_prepare(
    mac_protocol_debug_switch_stru *protocol_debug, int8_t *param)
{
    uint32_t off_set = 0;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    oal_bool_enum_uint8 en_cmd_updata = OAL_FALSE;
    do {
        /* 获取命令关键字 */
        ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if ((ret != OAL_SUCC) && (off_set != 0)) {
            protocol_debug_cmd_format_info();
            return ret;
        }
        param += off_set;

        if (en_cmd_updata == OAL_FALSE) {
            en_cmd_updata = OAL_TRUE;
        } else if (off_set == 0) {
            break;
        }

        /* 命令分类 */
        if (oal_strcmp("csa", name) == 0) {
            ret = wal_protocol_debug_parase_csa_cmd(param, protocol_debug, &off_set);
            if (ret != OAL_SUCC) {
                protocol_debug_cmd_format_info();
                return ret;
            }
            param += off_set;
            off_set = 0;
            protocol_debug->cmd_bit_map |= BIT3;
        } else if (g_wlan_spec_cfg->feature_11ax_is_open && oal_strcmp("11ax", name) == 0) {
            protocol_debug->cmd_bit_map |= BIT6;
        } else {
            ret = wal_hipriv_set_protocol_debug_analyze(protocol_debug, &param, name);
            if (ret != OAL_SUCC) {
                return ret;
            }
        }
    } while (*param != '\0');
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置协议开关protocol debug的信息,包括:40M恢复，20/40切换开关，40M不容忍等
 * 1.日    期  : 2017年2月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_protocol_debug_info(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_protocol_debug_switch_stru st_protocol_debug;
    uint32_t ret;
    int32_t l_ret;

    /* sh hipriv.sh "wlan0 protocol_debug band_force_switch 0|1|2(20|40-|40+)
       2040_ch_swt_prohi 0|1(关闭|打开) 2040_intolerant 0|1(关闭|打开)" */
    memset_s(&st_protocol_debug, sizeof(st_protocol_debug), 0, sizeof(st_protocol_debug));

    ret = wal_hipriv_set_protocol_debug_para_prepare(&st_protocol_debug, pc_param);
    if (ret != OAL_SUCC) {
        return ret;
    }
    oam_warning_log1(0, OAM_SF_ANY,
        "{wal_hipriv_show_protocol_debug_info::cmd_bit_map: 0x%08x.}", st_protocol_debug.cmd_bit_map);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_PROTOCOL_DBG, sizeof(st_protocol_debug));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_protocol_debug, sizeof(st_protocol_debug)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_show_protocol_debug_info::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_protocol_debug),
                               (uint8_t *)&st_write_msg, OAL_FALSE,  NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_show_protocol_debug_info::return err code[%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 配置发送功率命令入参检查
 * 1.日    期  : 2019年6月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC wal_tx_pow_param_enum wal_get_tx_pow_param(int8_t *pc_param, uint8_t uc_param_len)
{
    wal_tx_pow_param_enum en_param_index;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_TX_POW_PARAM_BUTT; en_param_index++) {
        if (uc_param_len <= OAL_STRLEN(g_pauc_tx_pow_param_name[en_param_index])) {
            return WAL_TX_POW_PARAM_BUTT;
        }

        if (!oal_strcmp(g_pauc_tx_pow_param_name[en_param_index], pc_param)) {
            break;
        }
    }

    return en_param_index;
}

/*
 * 功能描述  : 配置发送功率命令入参
 * 1.日    期  : 2019年6月25日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void wal_get_tx_pow_log_param(mac_cfg_set_tx_pow_param_stru *pst_set_tx_pow_param, int8_t *pc_param)
{
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;

    if (wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::show log arg1 missing!}");
        return;
    }
    pst_set_tx_pow_param->auc_value[1] = (uint8_t)oal_atoi(ac_arg);

    if (pst_set_tx_pow_param->auc_value[0] == 0) {
        // pow
        // rate_idx
        oam_error_log1(0, OAM_SF_ANY,
                       "{wal_hipriv_set_tx_pow_param::err rate idx [%d]!}",
                       pst_set_tx_pow_param->auc_value[1]);
        pst_set_tx_pow_param->auc_value[1] = 0;
    } else {
        // evm
        pc_param += off_set;
        if (wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::evm arg err input!}");
            return;
        }
        // chn_idx
        pst_set_tx_pow_param->auc_value[BYTE_OFFSET_2] = (uint8_t)oal_atoi(ac_arg);
    }

    return;
}

OAL_STATIC void wal_tx_pow_param_set_sar_level_case(mac_cfg_set_tx_pow_param_stru *set_tx_pow_param,
    uint8_t value)
{
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    uint16_t data_len;
    oam_warning_log1(0, OAM_SF_TPC, "{wal_tx_pow_param_set_sar_level_case::input reduce SAR level[%d]!}", value);
    if (wlan_chip_get_sar_ctrl_params(value, set_tx_pow_param->auc_value, &data_len,
        sizeof(set_tx_pow_param->auc_value)) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_tx_pow_param_set_sar_level_case::memcpy fail!}");
    }
#endif
}

OAL_STATIC uint32_t wal_tx_pow_param_tas_pow_ctrl_case(
    mac_cfg_set_tx_pow_param_stru *pst_set_tx_pow_param, int8_t *pc_param,
    uint8_t uc_value)
{
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    mac_device_stru *pst_mac_device;
    mac_cfg_tas_pwr_ctrl_stru st_tas_pow_ctrl_params;

    pst_mac_device = mac_res_get_dev(0);
    /* 如果非单VAP,则不处理 */
    if (mac_device_calc_up_vap_num(pst_mac_device) > 1) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_set_tx_pow_param::up vap more than 1 vap");
        return OAL_FAIL;
    }

    memset_s(&st_tas_pow_ctrl_params, sizeof(mac_cfg_tas_pwr_ctrl_stru),
             0, sizeof(mac_cfg_tas_pwr_ctrl_stru));
    st_tas_pow_ctrl_params.uc_core_idx = !!(uc_value);

    if (g_tas_switch_en[!!uc_value] == OAL_FALSE) {
        return OAL_FAIL;
    }

    /* 获取下一个参数 */
    if (wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_tx_pow_param_tas_pow_ctrl_case::TAS pwr ctrl core or needimproved lost return err_code [%d]!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    st_tas_pow_ctrl_params.en_need_improved = (uint8_t)!!oal_atoi(ac_arg);
    if (memcpy_s(pst_set_tx_pow_param->auc_value, sizeof(pst_set_tx_pow_param->auc_value),
        &st_tas_pow_ctrl_params, sizeof(mac_cfg_tas_pwr_ctrl_stru)) != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_tx_pow_param_tas_pow_ctrl_case::memcpy_s fail");
    }

    oam_warning_log2(0, OAM_SF_ANY,
        "{wal_hipriv_set_tx_pow_param::WAL_TX_POW_PARAM_TAS_POW_CTRL core[%d] improved_flag[%d]!}\r\n",
                     st_tas_pow_ctrl_params.uc_core_idx, st_tas_pow_ctrl_params.en_need_improved);

    return OAL_SUCC;
}
OAL_STATIC uint32_t wal_hipriv_check_tx_pow_para_valid(mac_cfg_set_tx_pow_param_stru *pow_param,
    wal_tx_pow_param_enum idx)
{
    uint8_t value = pow_param->auc_value[0];

    if (idx == WAL_TX_POW_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_check_tx_pow_para_valid::error input type!}");
        return OAL_FAIL;
    }

    if ((idx == WAL_TX_POW_PARAM_SET_RF_REG_CTL || idx == WAL_TX_POW_PARAM_SET_NO_MARGIN) && value >= 2) { // 2:max
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_check_tx_pow_para_valid::para large than 2!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (idx >= WAL_TX_POW_PARAM_SET_FIX_LEVEL && idx <= WAL_TX_POW_PARAM_SET_CTL_LEVEL && value > 4) { // 4:max lvl
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_check_tx_pow_para_valid::lvl large than 4!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

OAL_STATIC void wal_hipriv_tx_pow_get_next_para(mac_cfg_set_tx_pow_param_stru *pow_param, int8_t *param)
{
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;

    if (OAL_SUCC == wal_get_cmd_one_arg(param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set)) {
        pow_param->auc_value[1] = (uint8_t)oal_atoi(ac_arg);
    }
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
OAL_STATIC uint32_t wal_hipriv_tx_pow_tas_hanlde(mac_cfg_set_tx_pow_param_stru *pow_param,
    int8_t *param, uint8_t value, wal_tx_pow_param_enum param_index)
{
    uint32_t ret = OAL_SUCC;

    switch (param_index) {
        case WAL_TX_POW_PARAM_TAS_POW_CTRL:
            if (wal_tx_pow_param_tas_pow_ctrl_case(pow_param, param, value) != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_tx_pow_tas_hanlde::WAL_TX_POW_tas_pow_ctrl_case fail");
                return OAL_FAIL;
            }
            break;

        case WAL_TX_POW_PARAM_TAS_RSSI_MEASURE:
            if (g_tas_switch_en[!!value] == OAL_FALSE) {
                oam_error_log1(0, OAM_SF_ANY, "wal_hipriv_tx_pow_tas_hanlde::rssi measure[%d]!", value);
                return OAL_FAIL;
            }
            break;

        case WAL_TX_POW_PARAM_TAS_ANT_SWITCH:
            if (oal_any_true_value2(g_tas_switch_en[WLAN_RF_CHANNEL_ZERO], g_tas_switch_en[WLAN_RF_CHANNEL_ONE])) {
                /* 0:默认态 1:tas态 */
                oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_tx_pow_tas_hanlde::tas switch[%d].}", value);
                ret = (uint32_t)board_wifi_tas_set(value);
            }
            return ret;

        default:
            break;
    }
    return OAL_SUCC;
}
#endif

OAL_STATIC void wal_hipriv_tx_pow_get_pdet_info(mac_cfg_set_tx_pow_param_stru *pow_param, int8_t *param)
{
#ifdef _PRE_WLAN_FEATURE_FULL_QUAN_PROD_CAL
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;

    /* 表示当前开始产线全量校准的数据采集,获取下一个参数状态 */
    if (pow_param->auc_value[0] == 1) {
        if (OAL_SUCC == wal_get_cmd_one_arg(param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set)) {
            pow_param->auc_value[1] = (uint8_t)oal_atoi(ac_arg);
        }
    }
    oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_tx_pow_get_pdet_info::pdet info param[%d %d]!}",
        pow_param->auc_value[0], pow_param->auc_value[1]);
#endif
}

OAL_STATIC uint32_t wal_hipriv_check_param(mac_cfg_set_tx_pow_param_stru *pow_param,
    int8_t *param, wal_tx_pow_param_enum param_index)
{
    uint32_t ret;
    uint8_t value = pow_param->auc_value[0];

    ret = wal_hipriv_check_tx_pow_para_valid(pow_param, param_index);
    if (ret != OAL_SUCC) {
        return ret;
    }

    switch (param_index) {
        case WAL_TX_POW_PARAM_SET_SHOW_LOG:
            /* to reduce cycle complexity */
            wal_get_tx_pow_log_param(pow_param, param);
            break;

        case WAL_TX_POW_PARAM_SET_AMEND:
        case WAL_TX_POW_PARAM_SHOW_TPC_TABLE_GAIN:
        case WAL_TX_POW_POW_SAVE:
        case WAL_TX_POW_SET_TPC_IDX:
            wal_hipriv_tx_pow_get_next_para(pow_param, param);
            break;

        case WAL_TX_POW_PARAM_SET_SAR_LEVEL:
            wal_tx_pow_param_set_sar_level_case(pow_param, value);
            break;

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
        case WAL_TX_POW_PARAM_TAS_POW_CTRL:
        case WAL_TX_POW_PARAM_TAS_RSSI_MEASURE:
        case WAL_TX_POW_PARAM_TAS_ANT_SWITCH:
            ret = wal_hipriv_tx_pow_tas_hanlde(pow_param, param, value, param_index);
            if (ret != OAL_SUCC) {
                return ret;
            }
            break;
#endif
        case WAL_TX_POW_GET_PD_INFO:
            wal_hipriv_tx_pow_get_pdet_info(pow_param, param);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}
/*
 * 功能描述  : 配置发送功率命令
 * 1.日    期  : 2016年12月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_tx_pow_param(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_set_tx_pow_param_stru *pst_set_tx_pow_param = NULL;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    wal_tx_pow_param_enum en_param_index;

    /* 命令格式: hipriv "vap0 set_tx_pow rf_reg_ctl 0/1" ,   0:不使能, 1:使能            */
    /* 命令格式: hipriv "vap0 set_tx_pow fix_level 0/1/2/3"  设置数据帧功率等级, 仅data0 */
    /* 命令格式: hipriv "vap0 set_tx_pow mag_level 0/1/2/3"  设置管理帧功率等级          */
    /* 命令格式: hipriv "vap0 set_tx_pow ctl_level 0/1/2/3"  设置控制帧功率等级          */
    /* 命令格式: hipriv "vap0 set_tx_pow amend <value>"      修正upc code                */
    /* 命令格式: hipriv "vap0 set_tx_pow no_margin"          功率不留余量设置, 仅51用    */
    /* 命令格式: hipriv "vap0 set_tx_pow show_log type (bw chn)" 显示功率维测日志
       type: 0/1 pow/evm
       bw:   rate_idx(pow) 0/1/2 20M/80M/160M(evm)
       chn:  0~6(20M/80M) 0/1(160M)  */
    /* 命令格式: hipriv "vap0 set_tx_pow sar_level 0/1/2/3"  设置降sar等级               */
    /* 命令格式: hipriv "vap0 set_tx_pow tas_pwr_ctrl 0/1 0/1" tas功率控制               */
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_TX_POW, sizeof(mac_cfg_set_tx_pow_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_tx_pow_param = (mac_cfg_set_tx_pow_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_set_tx_pow_param, sizeof(mac_cfg_set_tx_pow_param_stru), 0, sizeof(mac_cfg_set_tx_pow_param_stru));

    /* 获取描述符字段设置命令字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::wal_get_cmd_one_arg return err_code[%d]!}", ret);
        return ret;
    }

    /* 解析是设置哪一个字段 */
    en_param_index = wal_get_tx_pow_param(ac_arg, sizeof(ac_arg));
    if (en_param_index == WAL_TX_POW_PARAM_BUTT) { /* 检查命令是否打错 */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::no such param for tx pow!}\r\n");
        return OAL_FAIL;
    }

    pst_set_tx_pow_param->en_type = en_param_index;

    /* 获取下一个参数 */
    pc_param += off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::wal_get_cmd_one_arg return err_code[%d]!}", ret);
    } else {
        pst_set_tx_pow_param->auc_value[0] = (uint8_t)oal_atoi(ac_arg);
        pc_param += off_set;
    }

    /* 参数校验 */
    ret = wal_hipriv_check_param(pst_set_tx_pow_param, pc_param, en_param_index);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_set_tx_pow_param::wal_hipriv_check_param fail");
        return ret;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_tx_pow_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置描述符参数配置命令
 * 1.日    期  : 2013年5月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_ucast_data_dscr_param(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_hipriv_set_dscr_param(net_dev, param, WAL_HIPRIV_DSCR_PARAM_CMD_UCAST_DATA);
}

/*
 * 功能描述  : 设置描述符参数配置命令
 * 1.日    期  : 2014年1月22日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_bcast_data_dscr_param(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_hipriv_set_dscr_param(net_dev, param, WAL_HIPRIV_DSCR_PARAM_CMD_BCAST_DATA);
}

/*
 * 功能描述  : 设置单播管理帧的速率
 * 1.日    期  : 2014年1月22日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_ucast_mgmt_dscr_param(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_hipriv_set_dscr_param(net_dev, param, WAL_HIPRIV_DSCR_PARAM_CMD_UCAST_MGMT);
}

/*
 * 功能描述  : 设置组播、广播管理帧的速率
 * 1.日    期  : 2014年1月22日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_mbcast_mgmt_dscr_param(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_hipriv_set_dscr_param(net_dev, param, WAL_HIPRIV_DSCR_PARAM_CMD_MBCAST_MGMT);
}

/*
 * 功能描述  : 设置空间流个数
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_nss(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_tx_comp_stru *pst_set_nss_param = NULL;
    int32_t l_nss;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_idx = 0;
    wal_msg_stru *pst_rsp_msg = NULL;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_NSS, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_nss_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* 获取速率值字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_nss::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_nss::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 解析要设置为多大的值 */
    l_nss = oal_atoi(ac_arg);
    if (l_nss < WAL_HIPRIV_NSS_MIN || l_nss > WAL_HIPRIV_NSS_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_nss::input val out of range [%d]!}\r\n", l_nss);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pst_set_nss_param->uc_param = (uint8_t)(l_nss - 1);

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_nss::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }
    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_freq fail, err code[%u]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置通道
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_rfch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_tx_comp_stru *pst_set_rfch_param = NULL;
    uint8_t uc_ch;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t c_ch_idx;
    wal_msg_stru *pst_rsp_msg = NULL;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_RFCH, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_rfch_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* 获取速率值字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_rfch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    /* 解析要设置为多大的值 */
    uc_ch = 0;
    for (c_ch_idx = 0; c_ch_idx < WAL_HIPRIV_CH_NUM; c_ch_idx++) {
        if (ac_arg[c_ch_idx] == '0') {
            continue;
        } else if (ac_arg[c_ch_idx] == '1') {
            uc_ch += (uint8_t)(1 << (WAL_HIPRIV_CH_NUM - (uint8_t)c_ch_idx - 1));
        } else { /* 输入数据有非01数字，或数字少于4位，异常 */
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_rfch::input err!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 输入参数多于四位，异常 */
    if (ac_arg[c_ch_idx] != '\0') {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_rfch::input err!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_rfch_param->uc_param = uc_ch;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_rfch::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_rfch fail, err code[%u]!}\r\n", ret);
        return ret;
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : 设置聚合最大个数
 * 1.日    期  : 2014年10月29日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_ampdu_aggr_num(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_aggr_stru aggr_num_ctl = { 0 };
    uint32_t ret;

    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ampdu_aggr_num::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    aggr_num_ctl.aggr_num = (uint16_t)oal_atoi(ac_name);

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ampdu_aggr_num::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    aggr_num_ctl.aggr_txtime = (uint16_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_AGGR_NUM, sizeof(aggr_num_ctl));

    /* 填写消息体，参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 &aggr_num_ctl, sizeof(aggr_num_ctl)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ampdu_aggr_num::memcpy fail!}");
        return OAL_FAIL;
    }

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(aggr_num_ctl),
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ampdu_aggr_num::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  :  获取 门限类型 数据
 * 1.日    期  : 2020年11月06日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_threshold_type(mac_btcoex_mgr_stru *btcoex_mgr, int8_t *param, int8_t *name, uint32_t off_set)
{
    /* 3.获取第三个参数 */
    uint32_t ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_threshold_type::get btcoex_nss err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.en_btcoex_nss = (wlan_nss_enum_uint8)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第四个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_threshold_type::get 20m_low err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.threhold.uc_20m_low = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第五个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_threshold_type::get 20m_high err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.threhold.uc_20m_high = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第六个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_threshold_type::get 40m_low err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.threhold.uc_40m_low = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第七个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_threshold_type::get 40m_high err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.threhold.us_40m_high = (uint16_t)oal_atoi(name);
    return OAL_SUCC;
}
/*
 * 功能描述  :  获取 聚合大小类型 数据
 * 1.日    期  : 2020年11月06日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_aggregate_size_type(mac_btcoex_mgr_stru *btcoex_mgr, int8_t *param, int8_t *name, uint32_t off_set)
{
    /* 3.获取第三个参数 */
    uint32_t ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_aggregate_size_type::get btcoex_nss err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.en_btcoex_nss = (wlan_nss_enum_uint8)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第四个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_aggregate_size_type::get grade err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_grade = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第五个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_aggregate_size_type::get rx_size0 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_rx_size0 = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第六个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_aggregate_size_type::get rx_size1 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_rx_size1 = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第七个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_aggregate_size_type::get rx_size2 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_rx_size2 = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第八个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_aggregate_size_type::get rx_size3 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_rx_size3 = (uint8_t)oal_atoi(name);
    return OAL_SUCC;
}

/*
 * 功能描述  :  获取 rssi detect门限参数配置模式 数据
 * 1.日    期  : 2020年11月06日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_rssi_detect_type(mac_btcoex_mgr_stru *btcoex_mgr, int8_t *param, int8_t *name, uint32_t off_set)
{
    /* 3.获取第三个参数 */
    uint32_t ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg3 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.en_rssi_limit_on = (oal_bool_enum_uint8)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第四个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg6 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.en_rssi_log_on = (oal_bool_enum_uint8)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第五个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg3 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.uc_cfg_rssi_detect_cnt = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第六个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg4 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.c_cfg_rssi_detect_mcm_down_th = (int8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第七个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg5 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.c_cfg_rssi_detect_mcm_up_th = (int8_t)oal_atoi(name);
    return OAL_SUCC;
}

/*
 * 功能描述  :  获取  参数设置
 * 1.日    期  : 2020年11月06日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_perf_param_cfg(mac_btcoex_mgr_stru *btcoex_mgr, int8_t *param, int8_t *name,
    uint32_t name_len)
{
    uint32_t off_set = 0;
    uint32_t ret;

    if (name_len != WAL_HIPRIV_CMD_NAME_MAX_LEN) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::name_len[%d]!}", name_len);
        return OAL_FAIL;
    }

    /* 1.获取第一个参数: mode */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, name_len, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg1 err_code[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->uc_cfg_btcoex_mode = (uint8_t)oal_atoi(name);

    if (btcoex_mgr->uc_cfg_btcoex_mode == 0) {
    } else if (btcoex_mgr->uc_cfg_btcoex_mode == 1) {
        param = param + off_set; /* 偏移，取下一个参数 */

        /* 2.获取第二个参数 */
        ret = wal_get_cmd_one_arg((int8_t *)param, name, name_len, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg2 err_code[%d]!}", ret);
            return ret;
        }

        btcoex_mgr->uc_cfg_btcoex_type = (uint8_t)oal_atoi(name);
        if (btcoex_mgr->uc_cfg_btcoex_type > 2) { /* 2表示rssi detect门限参数配置模式 */
            oam_warning_log1(0, OAM_SF_COEX,
                "{wal_hipriv_btcoex_set_perf_param:: btcoex_mgr->uc_cfg_btcoex_type error [%d], [0/1/2]!}\r\n",
                btcoex_mgr->uc_cfg_btcoex_type);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        param = param + off_set; /* 偏移，取下一个参数 */

        if (btcoex_mgr->uc_cfg_btcoex_type == 0) {
            ret = wal_hipriv_threshold_type(btcoex_mgr, param, name, off_set);
        } else if (btcoex_mgr->uc_cfg_btcoex_type == 1) {
            ret = wal_hipriv_aggregate_size_type(btcoex_mgr, param, name, off_set);
        } else {
            ret = wal_hipriv_rssi_detect_type(btcoex_mgr, param, name, off_set);
        }
        if (ret != OAL_SUCC) {
            return ret;
        }
    } else {
        oam_warning_log1(0, OAM_SF_COEX,
            "{wal_hipriv_btcoex_set_perf_param::btcoex_mgr->uc_cfg_btcoex_mode err_code [%d]!}\r\n",
            btcoex_mgr->uc_cfg_btcoex_mode);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*
 * 功能描述  : 设置btcoex下删减BA逻辑参数调整
 *             hipriv "wlan0 set_coex_perf_param  0/1/2/3(参数查询/配置模式/切换模式/常发常收测试模式)
 *             0 0/1 0/1 :查询模式  siso/mimo  门限/聚合大小
 *             1 0/1 0/1 xx:配置模式  siso/mimo  门限/聚合大小 门限下限  门限上限   聚合size0 1 2 3
 * 1.日    期  : 2017年6月27日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_btcoex_set_perf_param(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int32_t l_ret;
    uint32_t ret;
    mac_btcoex_mgr_stru *btcoex_mgr = NULL;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* write_msg作清零操作 */
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    /* 设置配置命令参数 */
    btcoex_mgr = (mac_btcoex_mgr_stru *)write_msg.auc_value;

    /* 获取事件参数 */
    ret = wal_hipriv_perf_param_cfg(btcoex_mgr, param, name, sizeof(name));
    if (ret != OAL_SUCC) {
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_BTCOEX_SET_PERF_PARAM, sizeof(mac_btcoex_mgr_stru));

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_btcoex_mgr_stru),
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param:: return err code = [%d].}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

#endif

/*
 * 函 数 名  : wal_hipriv_set_rate
 * 功能描述  : 设置non-HT模式下的速率
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_rate(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_non_ht_rate_stru *pst_set_rate_param = NULL;
    wlan_legacy_rate_value_enum_uint8 en_rate_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_RATE, sizeof(mac_cfg_non_ht_rate_stru));

    /* 解析并设置配置命令参数 */
    pst_set_rate_param = (mac_cfg_non_ht_rate_stru *)(write_msg.auc_value);

    /* 获取速率值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_rate::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 解析是设置为哪一级速率 */
    for (en_rate_index = 0; en_rate_index < WLAN_LEGACY_RATE_VALUE_BUTT; en_rate_index++) {
        if (!oal_strcmp(g_pauc_non_ht_rate_tbl[en_rate_index], ac_arg)) {
            break;
        }
    }

    /* 解析要设置为多大的值 */
    pst_set_rate_param->en_rate = en_rate_index;

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_non_ht_rate_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_rate::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_rate fail, err code[%u]!}", result);
        return result;
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : 设置速率(HT/VHT/HE/HE_ER通用接口)
 */
uint32_t wal_hipriv_set_mcs_by_cfgid(oal_net_device_stru *net_dev, int8_t *pc_param, wlan_cfgid_enum_uint16 cfgid,
    int32_t mcs_min, int32_t mcs_max)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    mac_cfg_tx_comp_stru *pst_set_mcs_param = NULL;
    int32_t l_mcs;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, cfgid, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_mcs_param = (mac_cfg_tx_comp_stru *)(write_msg.auc_value);

    /* 获取速率值字符串 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_by_cfgid::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_by_cfgid::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 解析要设置为多大的值 */
    l_mcs = oal_atoi(ac_arg);
    if (l_mcs < mcs_min || l_mcs > mcs_max) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_by_cfgid::input val out of range [%d]!}", l_mcs);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_mcs_param->uc_param = (uint8_t)l_mcs;

    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mcs_by_cfgid::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_mcs_by_cfgid fail, err code[%u]!}", result);
        return result;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_set_mcs_11n
 * 功能描述  : 设置HT模式下的速率
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_mcs(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_hipriv_set_mcs_by_cfgid(net_dev, pc_param, WLAN_CFGID_SET_MCS,
                                       WAL_HIPRIV_HT_MCS_MIN, WAL_HIPRIV_HT_MCS_MAX);
}

/*
 * 函 数 名  : wal_hipriv_set_mcsac
 * 功能描述  : 设置VHT模式下的速率
 * 1.日    期  : 2014年3月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_mcsac(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_hipriv_set_mcs_by_cfgid(net_dev, pc_param, WLAN_CFGID_SET_MCSAC,
                                       WAL_HIPRIV_VHT_MCS_MIN, WAL_HIPRIV_VHT_MCS_MAX);
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*
 * 函 数 名  : wal_hipriv_set_mcsac
 * 功能描述  : 设置HE模式下的速率
 * 1.日    期  : 2018年3月12日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_mcsax(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    return wal_hipriv_set_mcs_by_cfgid(net_dev, pc_param, WLAN_CFGID_SET_MCSAX,
                                       WAL_HIPRIV_HE_MCS_MIN, WAL_HIPRIV_HE_MCS_MAX);
}

#endif
