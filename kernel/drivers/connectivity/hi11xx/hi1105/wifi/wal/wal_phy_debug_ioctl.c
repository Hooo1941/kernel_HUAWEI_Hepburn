/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux ioctl配置调试命令
 * 作    者 : wifi
 * 创建日期 : 2016年4月8日
 */

#include "wal_linux_ioctl_debug.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_PHY_CFG_IOCTL_C

#ifdef WIFI_DEBUG_ENABLE

/*
 * 功能描述  : 设置 trlr info
 * 1.日    期  : 2017年1月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_set_trlr_info(mac_phy_debug_switch_stru *phy_debug_switch, int8_t c_value)
{
    if (isdigit(c_value)) {
        phy_debug_switch->auc_trlr_sel_info[phy_debug_switch->uc_trlr_sel_num] = (uint8_t)(c_value - '0');
    } else if ((c_value >= 'a') && (c_value <= 'f')) {
        /* 十六进制数'a'转换为十进制数为10 */
        phy_debug_switch->auc_trlr_sel_info[phy_debug_switch->uc_trlr_sel_num] = (uint8_t)(c_value - 'a' + 10);
    } else if ((c_value >= 'A') || (c_value <= 'F')) {
        /* 十六进制数'A'转换为十进制数为10 */
        phy_debug_switch->auc_trlr_sel_info[phy_debug_switch->uc_trlr_sel_num] = (uint8_t)(c_value - 'A' + 10);
    } else {
        oam_error_log0(0, OAM_SF_CFG,
            "{wal_hipriv_set_trlr_info:: param input illegal, should be [0-f/F].!!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置打印接收报文的phy debug相关信息,包括:rssi snr trlr count(打印间隔)
 * 1.日    期  : 2017年1月4日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_get_tone_tran_para(int8_t **pc_param, int8_t *ac_value,
                                           uint32_t value_len, mac_phy_debug_switch_stru *pst_phy_debug_switch)
{
    uint32_t ret = 0;
    uint32_t off_set = 0;

    if (0 == oal_strcmp("help", ac_value)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_get_tone_tran_para:tone tansmit command.!!}\r\n");
        return OAL_SUCC;
    }

    pst_phy_debug_switch->st_tone_tran.uc_tone_tran_switch = (uint8_t)oal_atoi(ac_value);
    if (pst_phy_debug_switch->st_tone_tran.uc_tone_tran_switch == 1) {
        ret = wal_get_cmd_one_arg(*pc_param, ac_value, value_len, &off_set);
        if ((ret != OAL_SUCC) || (off_set == 0)) {
            oam_error_log0(0, OAM_SF_CFG, "{wal_get_tone_tran_para:chain index is illegal'!!}\r\n");
            return ret;
        }
        *pc_param += off_set;
        pst_phy_debug_switch->st_tone_tran.uc_chain_idx = (uint16_t)oal_atoi(ac_value);

        if ((pst_phy_debug_switch->st_tone_tran.uc_chain_idx != 0) &&
            (pst_phy_debug_switch->st_tone_tran.uc_chain_idx != 1)) {
            oam_error_log1(0, OAM_SF_CFG,
                           "{wal_get_tone_tran_para:chain index[%d] is invalid'!!}\r\n",
                           pst_phy_debug_switch->st_tone_tran.uc_chain_idx);
            return OAL_FAIL;
        }

        /*  获取数据长度  */
        ret = wal_get_cmd_one_arg(*pc_param, ac_value, value_len, &off_set);
        if ((ret != OAL_SUCC) || (off_set == 0) || (oal_atoi(ac_value) == 0)) {
            oam_error_log0(0, OAM_SF_CFG, "{wal_get_tone_tran_para:tone data len is illegal'!!}\r\n");
            return ret;
        }
        *pc_param += off_set;
        pst_phy_debug_switch->st_tone_tran.us_data_len = (uint16_t)oal_atoi(ac_value);
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : 设置打印接收报文的phy debug相关help信息,包括:rssi snr trlr count(打印间隔)
 * 1.日    期  : 2019年4月1日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_print_phy_debug_help_info(int8_t *pc_param, uint8_t uc_para_len)
{
    if (uc_para_len < OAL_STRLEN("help")) {
        return OAL_FAIL;
    }

    if (oal_strcmp("help", pc_param) == 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[rssi]Print the rssi of rx packets, \
                         reported from rx dscr of MAC, range [-128 ~ +127] dBm.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[tsensor]Print the code of T-sensor.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[snr]Print snr values of two rx ants, \
                         11B not included, reported from rx dscr of MAC, range [-10 ~ +55] dBm.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[evm]Print evm values of two rx ants, \
                         11B not included, reported from rx dscr of MAC, range [-10 ~ +55] dBm.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG,
                         "{CMD[spatial_reuse]Print Sptial Reuse of MAC srg and non-srg frame time and rssi!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[trlr/vect]Sel range::trailer [0~f/F], vector [0~7], \
                         eg1: trlr 01234 eg2: vec123 trlr12, sum of both is less than 5.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG,
                         "{CMD[count]Set the interval of print (packets), range [0 ~ 2^32].!!}\r\n");

        return OAL_SUCC;
    }

    return OAL_FAIL;
}

static uint32_t wal_set_rssi_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_rssi_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_tsensor_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_tsensor_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_snr_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_snr_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_evm_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_evm_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_sp_reuse_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_sp_reuse_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_trlr_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch,
                                          int8_t *name, int8_t *value)
{
    uint32_t ret = OAL_SUCC;
    uint8_t data_cnt = 0;
    while (value[data_cnt] != '\0') {
        /* 输入参数合法性检查 */
        if (phy_debug_switch->uc_trlr_sel_num >= WAL_PHY_DEBUG_TEST_WORD_CNT) {
            oam_error_log1(0, 0, "{Param input illegal, cnt [%d] reached 5!!}", phy_debug_switch->uc_trlr_sel_num);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        ret = wal_hipriv_set_trlr_info(phy_debug_switch, value[data_cnt]);
        if (ret != OAL_SUCC) {
            return ret;
        }

        if (oal_strcmp("vect", name) == 0) {
            if (value[data_cnt] > '7') {
                oam_error_log0(0, OAM_SF_CFG, "{param input illegal, vect should be [0-7].!!}\r\n");
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
        } else {
            /* vector bit4为0，trailer的bit4置1, 设置寄存器可以一并带上 */
            phy_debug_switch->auc_trlr_sel_info[phy_debug_switch->uc_trlr_sel_num] |= 0x10;
        }

        data_cnt++;
        phy_debug_switch->uc_trlr_sel_num++;
    }

    /* 若输入正常，则打开trailer开关 */
    if (data_cnt != 0) {
        phy_debug_switch->en_trlr_debug_switch = OAL_TRUE;
    }
    return ret;
}

static uint32_t wal_set_comp_isr_interval(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->rx_comp_isr_interval = (uint32_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_iq_cali_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_iq_cali_switch = (uint8_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_tone_tran_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t **param,
                                         int8_t *value)
{
    uint32_t ret;
    ret = wal_get_tone_tran_para(param, value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, phy_debug_switch);
    if (ret != OAL_SUCC) {
        return ret;
    }
    return ret;
}

static uint32_t wal_set_pdet_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_pdet_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_force_work_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_force_work_switch = ((uint8_t)oal_atoi(value));
    return OAL_SUCC;
}

static uint32_t wal_set_dfr_reset_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_dfr_reset_switch = ((uint8_t)oal_atoi(value));
    return OAL_SUCC;
}

static uint32_t wal_set_fsm_info_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_fsm_info_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_report_radar_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_report_radar_switch = OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_extlna_chg_bypass_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_extlna_chg_bypass_switch = ((uint8_t)oal_atoi(value));
    return OAL_SUCC;
}

static uint32_t wal_set_edca_param_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= ((uint8_t)oal_atoi(value)) << BIT_OFFSET_4;
    return OAL_SUCC;
}

static uint32_t wal_set_edca_aifsn(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= (uint8_t)BIT3;
    phy_debug_switch->uc_edca_aifsn = (uint8_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_edca_cwmin(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= (uint8_t)BIT2;
    phy_debug_switch->uc_edca_cwmin = (uint8_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_edca_cwmax(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= (uint8_t)BIT1;
    phy_debug_switch->uc_edca_cwmax = (uint8_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_edca_txoplimit(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= (uint8_t)BIT0;
    phy_debug_switch->us_edca_txoplimit = (uint16_t)oal_atoi(value);
    return OAL_SUCC;
}

typedef struct {
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t (*wal_set_debug_switch_case)(mac_phy_debug_switch_stru *phy_debug_switch,
                                          int8_t *ac_value);
} wal_set_debug_switch_ops;

static const wal_set_debug_switch_ops g_debug_switch_ops_table[] = {
    { "rssi",          wal_set_rssi_debug_switch },
    { "tsensor",       wal_set_tsensor_debug_switch },
    { "snr",           wal_set_snr_debug_switch },
    { "evm",           wal_set_evm_debug_switch },
    { "spatial_reuse", wal_set_sp_reuse_debug_switch },
    { "count",         wal_set_comp_isr_interval },
    { "iq_cali",       wal_set_iq_cali_switch },
    { "pdet",          wal_set_pdet_debug_switch },
    { "force_work",    wal_set_force_work_switch },
    { "dfr_reset",     wal_set_dfr_reset_switch },
    { "fsm_info",      wal_set_fsm_info_switch },
    { "report_radar",  wal_set_report_radar_switch },
    { "extlna_bypass", wal_set_extlna_chg_bypass_switch },
    { "edca",          wal_set_edca_param_switch },
    { "aifsn",         wal_set_edca_aifsn },
    { "cwmin",         wal_set_edca_cwmin },
    { "cwmax",         wal_set_edca_cwmax },
    { "txoplimit",     wal_set_edca_txoplimit },
};

uint32_t wal_hipriv_checklist_command(mac_phy_debug_switch_stru *phy_debug_switch, int8_t **pc_param,
    int8_t *ac_name, int8_t *ac_value)
{
    uint32_t ret;
    uint8_t idx;

    /* 打印help信息 */
    ret = wal_hipriv_print_phy_debug_help_info(ac_name, strlen(ac_name));
    if (ret == OAL_SUCC) {
        return ret;
    }

    /* 查询各个命令 */
    for (idx = 0; idx < (sizeof(g_debug_switch_ops_table) / sizeof(wal_set_debug_switch_ops)); ++idx) {
        if (oal_strcmp(g_debug_switch_ops_table[idx].name, ac_name) == 0) {
            ret = g_debug_switch_ops_table[idx].wal_set_debug_switch_case(phy_debug_switch,
                                                                          ac_value);
            return ret;
        }
    }
    if (idx == (sizeof(g_debug_switch_ops_table) / sizeof(wal_set_debug_switch_ops))) {
        if (oal_any_zero_value2(oal_strcmp("trlr", ac_name), oal_strcmp("vect", ac_name))) {
            ret = wal_set_trlr_debug_switch(phy_debug_switch, ac_name, ac_value);
        } else if (oal_strcmp("tone_tran", ac_name) == 0) {
            ret = wal_set_tone_tran_switch(phy_debug_switch, pc_param, ac_value);
        } else {
            oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 phy_debug [rssi 0|1] [snr 0|1] \
            [trlr xxxx] [vect yyyy] [count N] [edca 0-3(tid_no)] [aifsn N] [cwmin N] [cwmax N] [txoplimit N]'!!}\r\n");
            return OAL_FAIL;
        }
    }
    return ret;
}

OAL_STATIC void wal_hipriv_phy_debug_init(mac_phy_debug_switch_stru *phy_debug_switch)
{
    phy_debug_switch->rx_comp_isr_interval = 10;  // 如果没有设置，则默认10个包打印一次，命令码可以更新
    /* mp13mpw2上rx hder占用一个，mp13解决频偏问题占用一个（固定上报rx trailor 13）因此这里从1开始计数 */
    phy_debug_switch->uc_trlr_sel_num = 1;
    phy_debug_switch->uc_force_work_switch = 0xff;
    phy_debug_switch->st_tone_tran.uc_tone_tran_switch = 0xF; /*  默认单音发送不处于发送/关闭状态  */
    phy_debug_switch->uc_dfr_reset_switch = 0xff;
    phy_debug_switch->uc_extlna_chg_bypass_switch = 0xff;
    phy_debug_switch->uc_edca_param_switch = 0x0;
}

OAL_STATIC void wal_hipriv_phy_debug_save_switch(mac_phy_debug_switch_stru *val)
{
    val->en_debug_switch = val->en_rssi_debug_switch | val->en_snr_debug_switch | val->en_trlr_debug_switch |
        val->uc_iq_cali_switch | val->en_tsensor_debug_switch | val->en_evm_debug_switch;

    oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_phy_debug_save_switch:: switch[%d]}", val->en_debug_switch);
}

/*
 * 功能描述  : 设置打印接收报文的phy debug相关信息,包括:rssi snr trlr count(打印间隔)
 * 1.日    期  : 2017年1月4日
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_phy_debug_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    mac_phy_debug_switch_stru st_phy_debug_switch = {0};
    uint32_t ret = 0;
    oal_bool_enum_uint8 en_cmd_updata = OAL_FALSE;

    /* sh hipriv.sh "wlan0 phy_debug snr 0|1(关闭|打开) rssi 0|1(关闭|打开) trlr 1234a count N(每个N个报文打印一次)" */
    wal_hipriv_phy_debug_init(&st_phy_debug_switch);

    do {
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if ((ret != OAL_SUCC) && (off_set != 0)) {
            oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_set_phy_debug_switch::cmd format err, ret:%d;!!}\r\n", ret);
            return ret;
        }
        pc_param += off_set;

        if (en_cmd_updata == OAL_FALSE) {
            en_cmd_updata = OAL_TRUE;
        } else if (off_set == 0) {
            break;
        }

        ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 phy_debug [rssi 0|1] [snr 0|1] \
                [trlr xxxxx] [vect yyyyy] [count N] [edca 0-3(tid_no)] [aifsn N] [cwmin N] [cwmax N] [txoplimit N]'!}");
            return ret;
        }
        pc_param += off_set;
        off_set = 0;

        /* 查询各个命令 */
        ret = wal_hipriv_checklist_command(&st_phy_debug_switch, &pc_param, ac_name, ac_value);
        if (ret != OAL_SUCC) {
            return ret;
        }
    } while (*pc_param != '\0');

    /* 将打印总开关保存到 */
    wal_hipriv_phy_debug_save_switch(&st_phy_debug_switch);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_PHY_DEBUG_SWITCH, sizeof(st_phy_debug_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_phy_debug_switch, sizeof(st_phy_debug_switch)) != EOK) {
        return OAL_FAIL;
    }

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_phy_debug_switch), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_phy_debug_switch::return err code[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC void wal_hipriv_set_dbdc_debug_params(mac_vap_stru *mac_vap,
    mac_dbdc_debug_switch_stru *dbdc_debug_switch,  mac_dbdc_cmd_enum_uint8 type, uint8_t status)
{
    if (type == MAC_DBDC_SWITCH) {
        dbdc_debug_switch->cmd_bit_map |= BIT(MAC_DBDC_SWITCH);
        dbdc_debug_switch->uc_dbdc_enable = status;
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_hipriv_set_dbdc_debug_params::dbdc enable[%d].}", dbdc_debug_switch->uc_dbdc_enable);
    } else if ((type == MAC_DBDC_CHANGE_HAL_DEV) || (type == MAC_SINGLE_SLAVE_SCAN)) {
        dbdc_debug_switch->cmd_bit_map |= BIT(type);
        dbdc_debug_switch->uc_dst_hal_dev_id = status;
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_hipriv_set_dbdc_debug_params::change to hal device[%d].}", dbdc_debug_switch->uc_dst_hal_dev_id);
    } else if (type == MAC_FAST_SCAN_SWITCH) {
        dbdc_debug_switch->cmd_bit_map |= BIT(MAC_FAST_SCAN_SWITCH);
        dbdc_debug_switch->en_fast_scan_enable = status;
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "{wal_hipriv_set_dbdc_debug_params::fast_scan enable[%d].}", dbdc_debug_switch->en_fast_scan_enable);
    }
}

OAL_STATIC uint32_t wal_hipriv_analyze_dbdc_debug_params(mac_vap_stru *mac_vap,
    mac_dbdc_debug_switch_stru *dbdc_debug_switch, int8_t *param)
{
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t val[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint8_t status;
    uint32_t offset = 0;
    oal_bool_enum_uint8 cmd_updata = OAL_FALSE;
    uint8_t vap_id = mac_vap->uc_vap_id;

    do {
        if (wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset) != OAL_SUCC && offset != 0) {
            oam_warning_log0(vap_id, OAM_SF_CFG, "{wal_hipriv_set_dbdc_debug_switch::cmd format err!}");
            return OAL_FAIL;
        }
        param += offset;

        if (cmd_updata == OAL_FALSE) {
            cmd_updata = OAL_TRUE;
        } else if (offset == 0) {
            break;
        }

        if (wal_get_cmd_one_arg(param, val, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &offset) != OAL_SUCC || !isdigit(val[0])) {
            oam_error_log0(vap_id, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 dbdc_debug [change_hal_dev 0|1]'!!}");
            return OAL_FAIL;
        }
        param += offset;
        offset = 0;

        status = (uint8_t)oal_atoi(val);
        if (status > 1) {
            oam_error_log1(vap_id, OAM_SF_CFG,
                "CMD format::sh hipriv.sh 'wlan0 dbdc_debug [value 0|1],input[%d]'!!}", status);
            return OAL_FAIL;
        }

        if (oal_strcmp("dbdc_enable", name) == 0) {
            wal_hipriv_set_dbdc_debug_params(mac_vap, dbdc_debug_switch, MAC_DBDC_SWITCH, status);
        } else if (oal_strcmp("shift_vap", name) == 0) {
            wal_hipriv_set_dbdc_debug_params(mac_vap, dbdc_debug_switch, MAC_DBDC_CHANGE_HAL_DEV, status);
        } else if (oal_strcmp("single_slave_scan", name) == 0) {
            wal_hipriv_set_dbdc_debug_params(mac_vap, dbdc_debug_switch, MAC_SINGLE_SLAVE_SCAN, status);
        } else if (oal_strcmp("fast_scan", name) == 0) {
            wal_hipriv_set_dbdc_debug_params(mac_vap, dbdc_debug_switch, MAC_FAST_SCAN_SWITCH, status);
        } else {
            oam_error_log0(vap_id, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 dbdc_debug xxx not support'!!}");
            return OAL_FAIL;
        }
    } while (*param != '\0');
    return OAL_SUCC;
}
/*
 * 功能描述  : DBDC debug 接口
 * 1.日    期  : 2017年05月02日
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_dbdc_debug_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_dbdc_debug_switch_stru dbdc_debug_switch;
    int32_t l_ret;
    mac_vap_stru *mac_vap = oal_net_dev_priv(pst_net_dev);

    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_dbdc_debug_switch::mac_vap is null!}");
        return OAL_FAIL;
    }

    /* sh hipriv.sh "wlan0 dbdc_debug change_hal_dev 0|1(hal 0|hal 1)" */
    memset_s(&dbdc_debug_switch, sizeof(dbdc_debug_switch), 0, sizeof(dbdc_debug_switch));

    if (wal_hipriv_analyze_dbdc_debug_params(mac_vap, &dbdc_debug_switch, pc_param) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /***************************************************************************
                            抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_DBDC_DEBUG_SWITCH, sizeof(dbdc_debug_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&dbdc_debug_switch, sizeof(dbdc_debug_switch)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_dbdc_debug_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(dbdc_debug_switch),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_dbdc_debug_switch::return err code[%d]!}", l_ret);
    }

    return (uint32_t)l_ret;
}

/*
 * 功能描述  : 配置20/40 bss检测是否使能
 * 1.日    期  : 2015年8月31日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_enable_2040bss(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    uint8_t uc_2040bss_switch;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_hipriv_enable_2040bss::wal_get_cmd_one_arg return err_code %d!}\r\n", ret);
        return ret;
    }

    if ((0 != oal_strcmp("0", ac_name)) && (0 != oal_strcmp("1", ac_name))) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_enable_2040bss::invalid parameter.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    uc_2040bss_switch = (uint8_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_2040BSS_ENABLE, sizeof(uint8_t));
    *((uint8_t *)(st_write_msg.auc_value)) = uc_2040bss_switch; /* 设置配置命令参数 */

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_enable_2040bss::return err code %d!}\r\n", ret);
    }

    return ret;
}

uint32_t wal_hipriv_set_extlna_bypass_rssi(oal_net_device_stru *net_device, int8_t *param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_extlna_bypass_rssi extlna_bypass_rssi_ctl = { 0 };
    uint32_t ret;

    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_extlna_bypass_rssi::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;

    /* rssi门限类型 0:extlna bypass off门限 1:extlna bypass on门限 */
    extlna_bypass_rssi_ctl.rssi_type = (uint8_t)oal_atoi(ac_name);

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_extlna_bypass_rssi::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    extlna_bypass_rssi_ctl.rssi_val = (int8_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_EXTLNA_BYPASS_RSSI, sizeof(extlna_bypass_rssi_ctl));

    /* 填写消息体，参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 &extlna_bypass_rssi_ctl, sizeof(extlna_bypass_rssi_ctl)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_extlna_bypass_rssi::memcpy fail!}");
        return OAL_FAIL;
    }

    ret = (uint32_t)wal_send_cfg_event(net_device, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(extlna_bypass_rssi_ctl),
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_extlna_bypass_rssi::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef PLATFORM_DEBUG_ENABLE
/*
 * 函 数 名  : wal_hipriv_reg_info
 * 功能描述  : 查询寄存器值(hipriv "Hisilicon0 reginfo regtype(soc/mac/phy) startaddr endaddr")
 * 1.日    期  : 2013年5月29日
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_reg_info(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;

    /* 抛事件到wal层处理 */
    /* 拷贝的时候目的地址要预留1字节为后面填充结束符使用,否则可能导致内存越界写风险 */
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value) - 1, pc_param, OAL_STRLEN(pc_param));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_reg_info::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_REG_INFO, len);

    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reg_info::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_reg_write
 * 1.日    期  : 2013年9月6日
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_reg_write(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;

    /* 抛事件到wal层处理 */
    /* 拷贝的时候目的地址要预留1字节为后面填充结束符使用,否则可能导致内存越界写风险 */
    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value) - 1, pc_param, OAL_STRLEN(pc_param));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_reg_write::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_REG_WRITE, len);

    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reg_write::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#endif

#endif

#ifdef _PRE_WLAN_EXPORT
/*
 * 函 数 名  : wal_hipriv_mac_addr_write
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2023年08月08日
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_mac_addr_write(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int32_t ret;
    uint16_t len;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), param, OAL_STRLEN(param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_mac_addr_write::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(param)] = '\0';

    len = (uint16_t)(OAL_STRLEN(param) + 1);

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_MAC_ADDR_WRITE, len);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_mac_addr_write::return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}
#ifdef _PRE_USE_EFUSE_MAC_ADDR
/*
 * 函 数 名  : wal_hipriv_set_mac_addr_from_efuse
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2023年08月08日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_set_mac_addr_from_efuse(oal_net_device_stru *net_dev)
{
    int32_t ret;
    mac_vap_stru *mac_vap;
    mac_cfg_staion_id_param_stru id_param;

    /* 配置设备mac地址为efuse中读取的地址 */
    if (oal_memcmp(g_wlan_device_mac_addr, oal_netdevice_mac_addr(net_dev), WLAN_MAC_ADDR_LEN) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "g_wlan_device_mac_addr already be set");
        return OAL_SUCC;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_atcmdsrv_efuse_regs_read::oal_net_dev_priv, return null!}");
        return -OAL_EINVAL;
    }

    if ((g_wlan_device_mac_addr[MAC_ADDR_3] == 0) && (g_wlan_device_mac_addr[MAC_ADDR_4] == 0) &&
        (g_wlan_device_mac_addr[MAC_ADDR_5] == 0)) {
        oal_random_ether_addr(&g_wlan_device_mac_addr[MAC_ADDR_3], NUM_3_BYTES);
    }

    oal_set_mac_addr((uint8_t *)oal_netdevice_mac_addr(net_dev), g_wlan_device_mac_addr);
    oal_set_mac_addr(id_param.auc_station_id, g_wlan_device_mac_addr);
    id_param.en_p2p_mode = WLAN_LEGACY_VAP_MODE;

    ret = hmac_config_set_mac_addr(mac_vap, WLAN_MAC_ADDR_LEN, (uint8_t *)&id_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_set_mac_addr:: return error code %d.}\r\n", ret);
    }
    return ret;
}
#endif

/*
 * 函 数 名  : wal_hipriv_mac_addr_read
 * 功能描述  : 输入参数  : 无
 * 1.日    期  : 2023年08月08日
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_mac_addr_read(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int32_t ret, leftime;
    hmac_vap_stru *hmac_vap;
    mac_vap_stru *mac_vap;
    uint16_t len;

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_atcmdsrv_efuse_regs_read::oal_net_dev_priv, return null!}");
        return -OAL_EINVAL;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "wal_atcmdsrv_efuse_regs_read:mac_res_get_hmac_vap failed");
        return -OAL_EINVAL;
    }
    hmac_vap->uc_efuse_mac_addr_flag = OAL_FALSE;
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value), param, OAL_STRLEN(param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_mac_addr_read::memcpy fail!");
        return OAL_FAIL;
    }

    write_msg.auc_value[OAL_STRLEN(param)] = '\0';
    len = (uint16_t)(OAL_STRLEN(param) + 1);

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_MAC_ADDR_READ, len);
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_mac_addr_read::return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }

    /* 阻塞等待dmac上报 */
    leftime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
                                                     (uint32_t)(hmac_vap->uc_efuse_mac_addr_flag == OAL_TRUE),
                                                     WAL_ATCMDSRB_DBB_NUM_TIME);
    if (leftime <= 0) {
        /* 超时或定时器内部错误上报 */
        oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_ANY, "{wal_hipriv_mac_addr_read::efuse_regs wait for %ld ms!}",
                         ((WAL_ATCMDSRB_DBB_NUM_TIME * 1000) / OAL_TIME_HZ)); /* 1s等于1000ms */
        return -OAL_EINVAL;
    }

    oam_warning_log3(0, OAM_SF_ANY, "{wal_hipriv_mac_addr_read::mac_addr:[%02X:XX:XX:XX:%02X:%02X]!}",
        g_wlan_device_mac_addr[MAC_ADDR_0], g_wlan_device_mac_addr[MAC_ADDR_4], g_wlan_device_mac_addr[MAC_ADDR_5]);

#ifdef _PRE_USE_EFUSE_MAC_ADDR
    ret = wal_hipriv_set_mac_addr_from_efuse(net_dev);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mac_addr_from_efuse:: return error code %d.}\r\n", ret);
    }
#endif
    return OAL_SUCC;
}
#endif

/* 协议模式字符串定义 */
const wal_ioctl_mode_map_stru g_ast_mode_map[] = {
    /* legacy */
    { "11a",  WLAN_LEGACY_11A_MODE,    WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11b",  WLAN_LEGACY_11B_MODE,    WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11bg", WLAN_MIXED_ONE_11G_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11g",  WLAN_MIXED_TWO_11G_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },

    /* 11n */
    { "11na20",      WLAN_HT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11ng20",      WLAN_HT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11na40plus",  WLAN_HT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40PLUS },
    { "11na40minus", WLAN_HT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40MINUS },
    { "11ng40plus",  WLAN_HT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40PLUS },
    { "11ng40minus", WLAN_HT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40MINUS },

    /* 11ac */
    { "11ac20",           WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11ac40plus",       WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40PLUS },
    { "11ac40minus",      WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40MINUS },
    { "11ac80plusplus",   WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80PLUSPLUS },
    { "11ac80plusminus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80PLUSMINUS },
    { "11ac80minusplus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80MINUSPLUS },
    { "11ac80minusminus", WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80MINUSMINUS },
#ifdef _PRE_WLAN_FEATURE_160M
    { "11ac160plusplusplus",    WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSPLUSPLUS },
    { "11ac160plusplusminus",   WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSPLUSMINUS },
    { "11ac160plusminusplus",   WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSMINUSPLUS },
    { "11ac160plusminusminus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSMINUSMINUS },
    { "11ac160minusplusplus",   WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSPLUSPLUS },
    { "11ac160minusplusminus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSPLUSMINUS },
    { "11ac160minusminusplus",  WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSMINUSPLUS },
    { "11ac160minusminusminus", WLAN_VHT_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSMINUSMINUS },
#endif

    { "11ac2g20",      WLAN_VHT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11ac2g40plus",  WLAN_VHT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40PLUS },
    { "11ac2g40minus", WLAN_VHT_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40MINUS },
    /* 11n only and 11ac only, 都是20M带宽 */
    { "11nonly2g", WLAN_HT_ONLY_MODE,  WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11nonly5g", WLAN_HT_ONLY_MODE,  WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11aconly",  WLAN_VHT_ONLY_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 11ax */
    { "11ax2g20",           WLAN_HE_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_20M },
    { "11ax2g40plus",       WLAN_HE_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40PLUS },
    { "11ax2g40minus",      WLAN_HE_MODE, WLAN_BAND_2G, WLAN_BAND_WIDTH_40MINUS },
    { "11ax5g20",           WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_20M },
    { "11ax5g40plus",       WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40PLUS },
    { "11ax5g40minus",      WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_40MINUS },
    { "11ax5g80plusplus",   WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80PLUSPLUS },
    { "11ax5g80plusminus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80PLUSMINUS },
    { "11ax5g80minusplus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80MINUSPLUS },
    { "11ax5g80minusminus", WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_80MINUSMINUS },

#ifdef _PRE_WLAN_FEATURE_160M
    { "11ax5g160plusplusplus",    WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSPLUSPLUS },
    { "11ax5g160plusplusminus",   WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSPLUSMINUS },
    { "11ax5g160plusminusplus",   WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSMINUSPLUS },
    { "11ax5g160plusminusminus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160PLUSMINUSMINUS },
    { "11ax5g160minusplusplus",   WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSPLUSPLUS },
    { "11ax5g160minusplusminus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSPLUSMINUS },
    { "11ax5g160minusminusplus",  WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSMINUSPLUS },
    { "11ax5g160minusminusminus", WLAN_HE_MODE, WLAN_BAND_5G, WLAN_BAND_WIDTH_160MINUSMINUSMINUS },
#endif
#endif

    { NULL }
};
/*
 * 功能描述  : 设置协议、频段、带宽模式发消息到wal处理
 * 1.日    期  : 2020年8月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_set_mode_config(oal_net_device_stru *net_dev, uint8_t uc_prot_idx)
{
    mac_cfg_mode_param_stru *pst_mode_param = NULL;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_write_stru write_msg;
    int32_t ret;
    uint32_t ret_error_code;
    uint32_t size = oal_array_size(g_ast_mode_map);
    if (uc_prot_idx >= size || g_ast_mode_map[uc_prot_idx].pc_name == NULL) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_hipriv_set_mode_config:: idx %d > size %d!}", uc_prot_idx, size);
        return OAL_SUCC;
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_MODE, sizeof(mac_cfg_mode_param_stru));

    pst_mode_param = (mac_cfg_mode_param_stru *)(write_msg.auc_value);
    pst_mode_param->en_protocol = g_ast_mode_map[uc_prot_idx].en_mode;
    pst_mode_param->en_band = g_ast_mode_map[uc_prot_idx].en_band;
    pst_mode_param->en_bandwidth = g_ast_mode_map[uc_prot_idx].en_bandwidth;

    oam_warning_log3(0, OAM_SF_CFG, "{wal_hipriv_set_mode_config::protocol[%d],band[%d],bandwidth[%d]!}",
                     pst_mode_param->en_protocol, pst_mode_param->en_band, pst_mode_param->en_bandwidth);

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_mode_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mode_config::wal_alloc_cfg_event err %d!}", ret);
        return (uint32_t)ret;
    }

    /* 读取返回的错误码 */
    ret_error_code = wal_check_and_release_msg_resp(rsp_msg);
    if (ret_error_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_mode_config fail, err code[%u]!}", ret_error_code);
        return ret_error_code;
    }

    return OAL_SUCC;
}

uint32_t wal_ioctl_set_mode(oal_net_device_stru *net_dev, int8_t *param)
{
    int8_t mode_str[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 }; /* 预留协议模式字符串空间 */
    uint8_t prot_idx;
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(net_dev, param))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mode::net_dev/p_param is null!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* param指向传入模式参数, 将其取出存放到mode_str中 */
    result = wal_get_cmd_one_arg(param, mode_str, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mode::wal_get_cmd_one_arg vap name err %d!}", result);
        return result;
    }

    mode_str[sizeof(mode_str) - 1] = '\0'; /* 确保以null结尾 */

    for (prot_idx = 0; g_ast_mode_map[prot_idx].pc_name != NULL; prot_idx++) {
        ret = oal_strcmp(g_ast_mode_map[prot_idx].pc_name, mode_str);
        if (ret == 0) {
            break;
        }
    }

    if (g_ast_mode_map[prot_idx].pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mode::unrecognized protocol string!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 发送消息到wal层处理 */
    ret = (int32_t)wal_hipriv_set_mode_config(net_dev, prot_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mode::send cfg event fail!}");
        return ret;
    }
    return OAL_SUCC;
}

uint32_t wal_ioctl_set_freq(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t l_freq;
    uint32_t off_set = 0;
    int8_t ac_freq[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    result = wal_get_cmd_one_arg(pc_param, ac_freq, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::wal_get_cmd_one_arg vap name err %d!}", result);
        return result;
    }

    l_freq = oal_atoi(ac_freq);
    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::l_freq = %d!}", l_freq);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_CURRENT_CHANEL, sizeof(int32_t));
    *((int32_t *)(write_msg.auc_value)) = l_freq;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_freq::err %d!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_freq fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_CSI
static uint32_t wal_ioctl_set_csi_switch_config(oal_net_device_stru *pst_cfg_net_dev,
    mac_cfg_csi_param_stru *cfg_csi_param_in)
{
    int32_t l_ret;
    wal_msg_write_stru st_write_msg;
    mac_cfg_csi_param_stru *cfg_csi_param = NULL;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_SET_CSI, sizeof(mac_cfg_csi_param_stru));

    /* 设置配置命令参数 */
    cfg_csi_param = (mac_cfg_csi_param_stru *)(st_write_msg.auc_value);
    if (memcpy_s(cfg_csi_param, sizeof(st_write_msg.auc_value),
                 cfg_csi_param_in, sizeof(mac_cfg_csi_param_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_csi::memcpy_s fail!}\r\n");
    }
    l_ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_csi_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_csi::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}

uint32_t wal_ioctl_set_csi_switch(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    uint32_t off_set, ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_csi_param_stru st_cfg_csi_param = {{0}};

    /* PSD设置开关的命令: hipriv "vap0 set_csi ta ta_check csi_en"
       TA:被测量CSI的mac地址，为0表示不使能
       TA_check: 为1时，TA有效，表示每次采集CSI信息时需比对ta。
       csi_en:   为1时，表示使能CSI采集
    */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), st_cfg_csi_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.en_cfg_csi_on = (uint8_t)oal_atoi(ac_name);
    // csi也做下限制，防止上层在sniffer模式下采集不到csi误认为是摄像头的问题切换另一通道采集csi
    if (st_cfg_csi_param.en_cfg_csi_on && !wal_sniffer_is_single_sta_mode(pst_cfg_net_dev)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_set_csi_switch::can not start csi!}");
        return OAL_FAIL;
    }

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.uc_csi_bw = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.uc_csi_frame_type = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.uc_csi_sample_period  = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_cfg_csi_param.uc_csi_phy_report_mode = (uint8_t)oal_atoi(ac_name);
    ret = wal_ioctl_set_csi_switch_config(pst_cfg_net_dev, &st_cfg_csi_param);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

#endif

/*
 * 函 数 名  : wal_ioctl_set_ap_mode
 * 功能描述  : 设置AP以siso/mimo方式启动
 * 1.日    期  : 2019年3月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_ioctl_set_ap_mode(oal_net_device_stru *net_dev, uint8_t uc_param)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SOFTAP_MIMO_MODE, sizeof(uint8_t));
    write_msg.auc_value[0] = uc_param;

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
        WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_set_ap_mode::return err code [%d]!}", ret);
        return (uint32_t)ret;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_set_ap_mode::uc_param[%d].}", uc_param);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
int32_t wal_init_wifi_tas_state(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t wifi_tas_state = 0;
    int32_t ret;
    struct device_node *np = NULL;

    if ((g_tas_switch_en[WLAN_RF_CHANNEL_ZERO] == OAL_FALSE) &&
        (g_tas_switch_en[WLAN_RF_CHANNEL_ONE] == OAL_FALSE)) {
        oam_warning_log0(0, OAM_SF_ANY, "{not support WiTAS}");
        return BOARD_SUCC;
    }
    ret = get_board_dts_node(&np, DTS_NODE_MPXX_WIFI);
    if (ret != BOARD_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{DTS read node hisi_wifi fail!}");
        return BOARD_FAIL;
    }
    ret = of_property_read_u32(np, DTS_PROP_MPXX_WIFI_TAS_STATE, &wifi_tas_state);
    if (ret) {
        oam_warning_log0(0, OAM_SF_ANY, "{read prop gpio_wifi_tas_state fail!}");
        return BOARD_SUCC;
    }
    if (board_get_wifi_tas_gpio_state() != wifi_tas_state) {
        return board_wifi_tas_set(wifi_tas_state);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{current WiTAS state is right, no need to set again!}");
        return BOARD_SUCC;
    }
#else
    return BOARD_SUCC;
#endif
}
#endif
