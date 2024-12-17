/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_cali_mgmt_debug.c
 * 作    者 : wifi1
 * 创建日期 : 2020年11月25日
 */

#include "hmac_cali_mgmt.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_DEBUG_C

OAL_STATIC void hmac_print_buff(int8_t *buff)
{
    buff[OAM_REPORT_MAX_STRING_LEN - 1] = '\0';
    oam_print(buff);
    memset_s(buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);
}

OAL_STATIC int32_t hmac_print_2g_cail_result(uint8_t idx,
    int8_t *pc_print_buff, int32_t l_remainder_len, mp13_cali_param_stru *cali_data)
{
    int8_t *pc_string = NULL;
    uint16_t *rx_ptr = NULL;
    mp13_2gcali_param_stru *cali_param_2g = &cali_data->ast_2gcali_param;
    uint32_t buf[MP13_2G_CHANNEL_NUM][MP13_CALI_TXDC_GAIN_LVL_NUM];
    uint16_t buf_len = sizeof(uint32_t)*MP13_2G_CHANNEL_NUM*MP13_CALI_TXDC_GAIN_LVL_NUM;

    rx_ptr = &cali_param_2g->st_cali_rx_dc_cmp[0].aus_analog_rxdc_siso_cmp[0];
    memset_s(&buf, buf_len, 0, buf_len);
    if (memcpy_s(&buf, buf_len, &cali_param_2g->ast_txdc_cmp_val, buf_len) != EOK) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "hmac_print_2g_cail_result:memcpy buf fail");
    }

    if (l_remainder_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "hmac_print_2g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
            l_remainder_len, OAM_REPORT_MAX_STRING_LEN, sizeof(mp13_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = "2G: cali data index[%u]\n"
                "siso_rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
                "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
                "tx_power[ppa:%u atx_pwr:%u dtx_pwr:%u dp_init:%d]\n"
                "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
                "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(pc_print_buff, (uint32_t)l_remainder_len, (uint32_t)l_remainder_len - 1, pc_string, idx,
        rx_ptr[BYTE_OFFSET_0], rx_ptr[BYTE_OFFSET_1], rx_ptr[BYTE_OFFSET_2], rx_ptr[BYTE_OFFSET_3],
        rx_ptr[BYTE_OFFSET_4], rx_ptr[BYTE_OFFSET_5], rx_ptr[BYTE_OFFSET_6], rx_ptr[BYTE_OFFSET_7],
        cali_param_2g->st_cali_rx_dc_cmp[BYTE_OFFSET_0].us_digital_rxdc_cmp_i,
        cali_param_2g->st_cali_rx_dc_cmp[BYTE_OFFSET_0].us_digital_rxdc_cmp_q,
        cali_param_2g->st_cali_logen_cmp[idx].uc_ssb_cmp, cali_param_2g->st_cali_logen_cmp[idx].uc_buf0_cmp,
        cali_param_2g->st_cali_logen_cmp[idx].uc_buf1_cmp, cali_param_2g->st_cali_tx_power_cmp_2g[idx].uc_ppa_cmp,
        cali_param_2g->st_cali_tx_power_cmp_2g[idx].uc_atx_pwr_cmp,
        cali_param_2g->st_cali_tx_power_cmp_2g[idx].uc_dtx_pwr_cmp,
        cali_param_2g->st_cali_tx_power_cmp_2g[idx].c_dp_init,
        /* 低4位是i,高4位是Q */
        buf[idx][BYTE_OFFSET_0] & 0xffff, (buf[idx][BYTE_OFFSET_0] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_1] & 0xffff, (buf[idx][BYTE_OFFSET_1] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_2] & 0xffff, (buf[idx][BYTE_OFFSET_2] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_3] & 0xffff, (buf[idx][BYTE_OFFSET_3] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_4] & 0xffff, (buf[idx][BYTE_OFFSET_4] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_5] & 0xffff, (buf[idx][BYTE_OFFSET_5] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_6] & 0xffff, (buf[idx][BYTE_OFFSET_6] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_7] & 0xffff, (buf[idx][BYTE_OFFSET_7] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_8] & 0xffff, (buf[idx][BYTE_OFFSET_8] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_9] & 0xffff, (buf[idx][BYTE_OFFSET_9] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_10] & 0xffff, (buf[idx][BYTE_OFFSET_10] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_11] & 0xffff, (buf[idx][BYTE_OFFSET_11] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_12] & 0xffff, (buf[idx][BYTE_OFFSET_12] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_13] & 0xffff, (buf[idx][BYTE_OFFSET_13] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_14] & 0xffff, (buf[idx][BYTE_OFFSET_14] >> BYTE_OFFSET_16) & 0xffff,
        buf[idx][BYTE_OFFSET_15] & 0xffff, (buf[idx][BYTE_OFFSET_15] >> BYTE_OFFSET_16) & 0xffff);
}

void hmac_print_get_5g_pc_string(uint8_t idx, int8_t **pc_string)
{
    *pc_string = (idx < OAL_5G_20M_CHANNEL_NUM)
        ? "5G 20M: cali data index[%u]\n"
          "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "digital_rxdc_i_q[0x%x 0x%x]\n"
          "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
          "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
          "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
          "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n"
        : "5G 80M: cali data index[%u]\n"
          "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "digital_rxdc_i_q[0x%x 0x%x]\n"
          "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
          "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
          "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
          "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";
}

OAL_STATIC int32_t hmac_print_5g_cail_result(uint8_t idx, int8_t *pc_print_buff,
    int32_t l_remainder_len, mp13_cali_param_stru *cali_data)
{
    int8_t *pc_string = NULL;
    uint16_t *rx_cmp = NULL;
    uint16_t *extlna = NULL;
    uint16_t *intlna = NULL;
    mp13_txdc_comp_val_stru *tx_cmp = NULL;
    if (l_remainder_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "hmac_print_5g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
            l_remainder_len, OAM_REPORT_MAX_STRING_LEN, sizeof(mp13_cali_param_stru));
        return l_remainder_len;
    }

    rx_cmp = &cali_data->ast_5gcali_param[idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_0];
    extlna = &cali_data->ast_5gcali_param[idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_0];
    intlna = &cali_data->ast_5gcali_param[idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_0];
    tx_cmp = &cali_data->ast_5gcali_param[idx].ast_txdc_cmp_val[BYTE_OFFSET_0];

    hmac_print_get_5g_pc_string(idx, &pc_string);

    return snprintf_s(pc_print_buff, (uint32_t)l_remainder_len, (uint32_t)l_remainder_len - 1,
        pc_string, idx,
        rx_cmp[BYTE_OFFSET_0], rx_cmp[BYTE_OFFSET_1], rx_cmp[BYTE_OFFSET_2], rx_cmp[BYTE_OFFSET_3],
        rx_cmp[BYTE_OFFSET_4], rx_cmp[BYTE_OFFSET_5], rx_cmp[BYTE_OFFSET_6], rx_cmp[BYTE_OFFSET_7],
        extlna[BYTE_OFFSET_0], extlna[BYTE_OFFSET_1], extlna[BYTE_OFFSET_2], extlna[BYTE_OFFSET_3],
        extlna[BYTE_OFFSET_4], extlna[BYTE_OFFSET_5], extlna[BYTE_OFFSET_6], extlna[BYTE_OFFSET_7],
        intlna[BYTE_OFFSET_0], intlna[BYTE_OFFSET_1], intlna[BYTE_OFFSET_2], intlna[BYTE_OFFSET_3],
        intlna[BYTE_OFFSET_4], intlna[BYTE_OFFSET_5], intlna[BYTE_OFFSET_6], intlna[BYTE_OFFSET_7],
        cali_data->ast_5gcali_param[idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i,
        cali_data->ast_5gcali_param[idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q,

        cali_data->ast_5gcali_param[idx].st_cali_logen_cmp.uc_ssb_cmp,
        cali_data->ast_5gcali_param[idx].st_cali_logen_cmp.uc_buf0_cmp,
        cali_data->ast_5gcali_param[idx].st_cali_logen_cmp.uc_buf1_cmp,

        cali_data->ast_5gcali_param[idx].st_cali_tx_power_cmp_5g.uc_ppa_cmp,
        cali_data->ast_5gcali_param[idx].st_cali_tx_power_cmp_5g.uc_mx_cmp,
        cali_data->ast_5gcali_param[idx].st_cali_tx_power_cmp_5g.uc_atx_pwr_cmp,
        cali_data->ast_5gcali_param[idx].st_cali_tx_power_cmp_5g.uc_dtx_pwr_cmp,
        cali_data->ast_5gcali_param[idx].st_ppf_cmp_val.uc_ppf_val,

        tx_cmp[BYTE_OFFSET_0].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_0].us_txdc_cmp_q, tx_cmp[BYTE_OFFSET_1].us_txdc_cmp_i,
        tx_cmp[BYTE_OFFSET_1].us_txdc_cmp_q, tx_cmp[BYTE_OFFSET_2].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_2].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_3].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_3].us_txdc_cmp_q, tx_cmp[BYTE_OFFSET_4].us_txdc_cmp_i,
        tx_cmp[BYTE_OFFSET_4].us_txdc_cmp_q, tx_cmp[BYTE_OFFSET_5].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_5].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_6].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_6].us_txdc_cmp_q, tx_cmp[BYTE_OFFSET_7].us_txdc_cmp_i,
        tx_cmp[BYTE_OFFSET_7].us_txdc_cmp_q, tx_cmp[BYTE_OFFSET_8].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_8].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_9].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_9].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_10].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_10].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_11].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_11].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_12].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_12].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_13].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_13].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_14].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_14].us_txdc_cmp_q,
        tx_cmp[BYTE_OFFSET_15].us_txdc_cmp_i, tx_cmp[BYTE_OFFSET_15].us_txdc_cmp_q);
}

void hmac_print_base_band_result(uint8_t uc_chan_idx, mp13_cali_param_stru *cali_data)
{
    oam_warning_log3(0, OAM_SF_CALIBRATE, "chan:%d, cali_data dog_tag:%d, check_hw_status:%d",
        uc_chan_idx, cali_data->dog_tag, cali_data->check_hw_status);
    oam_warning_log4(0, OAM_SF_CALIBRATE, "update info tmp:%d, chan_idx_1_2[%d %d],cali_time:%d",
        cali_data->st_cali_update_info.bit_temperature, cali_data->st_cali_update_info.uc_5g_chan_idx1,
        cali_data->st_cali_update_info.uc_5g_chan_idx2, cali_data->st_cali_update_info.cali_time);
    oam_warning_log4(0, OAM_SF_CALIBRATE,
        "st_rc_r_c_cali_data save_all:%d, c_code[0x%x], rc_code[0x%x] r_code[0x%x]",
        cali_data->en_save_all, cali_data->st_rc_r_c_cali_data.uc_c_cmp_code,
        cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code, cali_data->st_rc_r_c_cali_data.uc_r_cmp_code);

    oam_warning_log3(0, OAM_SF_CALIBRATE, "st_rc_r_c_cali_data save_all:classa[0x%x], classb[0x%x], close_fem[%d]",
        cali_data->st_pa_ical_cmp.uc_classa_cmp, cali_data->st_pa_ical_cmp.uc_classb_cmp,
        cali_data->en_need_close_fem_cali);
}

uint32_t hmac_print_2g_all(mp13_cali_param_stru *cali_data, int8_t *pc_print_buff)
{
    uint32_t string_len = 0;
    int32_t l_string_tmp_len;
    uint8_t uc_cali_chn_idx_1;
    for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 < OAL_2G_CHANNEL_NUM; uc_cali_chn_idx_1++) {
        l_string_tmp_len = hmac_print_2g_cail_result(uc_cali_chn_idx_1, pc_print_buff + string_len,
            (int32_t)(OAM_REPORT_MAX_STRING_LEN - string_len - 1), cali_data);
        if (l_string_tmp_len <= 0) {
            return OAL_FAIL;
        }
        string_len += (uint32_t)l_string_tmp_len;
    }
    hmac_print_buff(pc_print_buff);
    return OAL_SUCC;
}

uint32_t hmac_print_5g_all(mp13_cali_param_stru *cali_data, int8_t *pc_print_buff)
{
    uint32_t string_len;
    int32_t l_string_tmp_len;
    uint8_t uc_cali_chn_idx_1, uc_cali_chn_idx;
    for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 <= (OAL_5G_CHANNEL_NUM >> BIT_OFFSET_1); uc_cali_chn_idx_1++) {
        string_len = 0;
        for (uc_cali_chn_idx = uc_cali_chn_idx_1 << BIT_OFFSET_1;
             uc_cali_chn_idx < oal_min(OAL_5G_CHANNEL_NUM, ((uc_cali_chn_idx_1 + 1) << BIT_OFFSET_1));
             uc_cali_chn_idx++) {
            l_string_tmp_len = hmac_print_5g_cail_result(uc_cali_chn_idx,
                pc_print_buff + string_len, (int32_t)(OAM_REPORT_MAX_STRING_LEN - string_len - 1), cali_data);
            if (l_string_tmp_len <= 0) {
                return OAL_FAIL;
            }
            string_len += (uint32_t)l_string_tmp_len;
        }

        hmac_print_buff(pc_print_buff);
    }
    return OAL_SUCC;
}

void hmac_sprint_fail_freebuff_handle(int8_t *pc_print_buff)
{
    oam_warning_log0(0, OAM_SF_CFG, "{hmac_dump_cali_result:: OAL_SPRINTF return error!}");
    hmac_print_buff(pc_print_buff);
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
}
static uint32_t hmac_cali_mem_alloc(int8_t **pc_print_buff, uint8_t **puc_bfgx_rc_code)
{
    uint32_t              rc_size;
    /* OTA上报 */
    *pc_print_buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (*pc_print_buff == NULL) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result:: null.}",
            OAM_REPORT_MAX_STRING_LEN);
        return OAL_FAIL;
    }

    *puc_bfgx_rc_code = (uint8_t *)wifi_get_bfgx_rc_data_buf_addr(&rc_size);
    if (*puc_bfgx_rc_code == NULL) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result:: null.}",
            OAM_REPORT_MAX_STRING_LEN);
        oal_mem_free_m(*pc_print_buff, OAL_TRUE);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
void hmac_dump_cali_result(void)
{
    mp13_cali_param_stru *cali_data = NULL;
    int8_t               *pc_print_buff = NULL;
    uint8_t               uc_chan_idx;
    uint8_t              *puc_bfgx_rc_code = NULL;

    oam_info_log0(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::START.}");

    if (hmac_cali_mem_alloc(&pc_print_buff, &puc_bfgx_rc_code) != OAL_SUCC) {
        return;
    }
    cali_data = (mp13_cali_param_stru *)get_cali_data_buf_addr();
    *puc_bfgx_rc_code = cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code;

    for (uc_chan_idx = 0; uc_chan_idx < WLAN_RF_CHANNEL_NUMS; uc_chan_idx++) {
        cali_data += uc_chan_idx;
        hmac_print_base_band_result(uc_chan_idx, cali_data);

        /* g_new_txiq_comp_val_stru TO BE ADDED */
        /* 2.4g 不超过ota上报最大字节分3次输出 */
        if (hmac_print_2g_all(cali_data, pc_print_buff) != OAL_SUCC) {
            hmac_sprint_fail_freebuff_handle(pc_print_buff);
            return;
        }

        /* 5g */
        if (OAL_FALSE == mac_device_check_5g_enable_per_chip()) {
            continue;
        }

        /* 5g 不超过ota上报最大字节分3次输出 */
        if (hmac_print_5g_all(cali_data, pc_print_buff) != OAL_SUCC) {
            hmac_sprint_fail_freebuff_handle(pc_print_buff);
            return;
        }
    }
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
    return;
}

OAL_STATIC int32_t hmac_print_mp15_2g_cail_result(uint8_t idx,
    int8_t *pc_print_buff, int32_t l_remainder_len, mp15_cali_param_stru *cali_data)
{
    int8_t *pc_string = NULL;

    if (l_remainder_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "hmac_print_2g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
            l_remainder_len, OAM_REPORT_MAX_STRING_LEN, sizeof(mp15_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = "2G: cali data index[%u]\n"
                "siso_rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
                "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
                "tx_power[ppa:%u atx_pwr:%u dtx_pwr:%u dp_init:%d]\n"
                "tx_dc mixbuf[0], \
                siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(pc_print_buff, (uint32_t)l_remainder_len, (uint32_t)l_remainder_len - 1,
        pc_string, idx,
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_0],
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_1],
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_2],
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_3],
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_4],
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_5],
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_6],
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].aus_analog_rxdc_siso_cmp[BYTE_OFFSET_7],
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].us_digital_rxdc_cmp_i,
        cali_data->st_2gcali_param.ast_cali_rx_dc_cmp[BYTE_OFFSET_0].us_digital_rxdc_cmp_q,

        cali_data->st_2gcali_param.ast_cali_logen_cmp[idx].uc_ssb_cmp,
        cali_data->st_2gcali_param.ast_cali_logen_cmp[idx].uc_buf0_cmp,
        cali_data->st_2gcali_param.ast_cali_logen_cmp[idx].uc_buf1_cmp,

        cali_data->st_2gcali_param.ast_cali_tx_power_cmp_2g[idx].us_ppa_cmp,
        cali_data->st_2gcali_param.ast_cali_tx_power_cmp_2g[idx].us_atx_pwr_cmp,
        cali_data->st_2gcali_param.ast_cali_tx_power_cmp_2g[idx].uc_dtx_pwr_cmp,
        cali_data->st_2gcali_param.ast_cali_tx_power_cmp_2g[idx].c_dp_init,

        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_i[BYTE_OFFSET_0],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_q[BYTE_OFFSET_0],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_i[BYTE_OFFSET_1],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_q[BYTE_OFFSET_1],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_i[BYTE_OFFSET_2],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_q[BYTE_OFFSET_2],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_i[BYTE_OFFSET_3],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_q[BYTE_OFFSET_3],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_i[BYTE_OFFSET_4],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_q[BYTE_OFFSET_4],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_i[BYTE_OFFSET_5],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_q[BYTE_OFFSET_5],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_i[BYTE_OFFSET_6],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_q[BYTE_OFFSET_6],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_i[BYTE_OFFSET_7],
        cali_data->st_2gcali_param.ast_txdc_cmp_val[idx].aus_txdc_cmp_q[BYTE_OFFSET_7]);
}

void hmac_print_mp15_get_5g_pc_string(uint8_t idx, int8_t **pc_string)
{
    *pc_string = (idx < OAL_5G_20M_CHANNEL_NUM)
        ? "5G 20M: cali data index[%u]\n"
          "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
          "digital_rxdc_i_q[0x%x 0x%x]\n"
          "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
          "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
          "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n"
        : "5G 80M: cali data index[%u]\n"
         "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
         "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
         "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n"
         "digital_rxdc_i_q[0x%x 0x%x]\n"
         "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
         "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
         "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";
}

OAL_STATIC int32_t hmac_print_mp15_5g_cail_result(uint8_t idx,
    int8_t *pc_print_buff, int32_t l_remainder_len, mp15_cali_param_stru *cali_data)
{
    int8_t *pc_string = NULL;
    uint16_t *rx_cmp = NULL;
    uint16_t *extlna = NULL;
    uint16_t *intlna = NULL;
    mp15_5gcali_param_stru *cali5g = NULL;
    mp15_txdc_comp_val_stru *tx_cmp = NULL;

    if (l_remainder_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "hmac_print_5g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
            l_remainder_len, OAM_REPORT_MAX_STRING_LEN, sizeof(mp15_cali_param_stru));
        return l_remainder_len;
    }
    rx_cmp = &cali_data->ast_5gcali_param[idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[BYTE_OFFSET_0];
    extlna = &cali_data->ast_5gcali_param[idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[BYTE_OFFSET_0];
    intlna = &cali_data->ast_5gcali_param[idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[BYTE_OFFSET_0];
    cali5g = &cali_data->ast_5gcali_param[idx];
    tx_cmp = &cali_data->ast_5gcali_param[idx].ast_txdc_cmp_val[BYTE_OFFSET_0];

    hmac_print_mp15_get_5g_pc_string(idx, &pc_string);

    return snprintf_s(pc_print_buff, (uint32_t)l_remainder_len, (uint32_t)l_remainder_len - 1,
        pc_string, idx,
        rx_cmp[BYTE_OFFSET_0], rx_cmp[BYTE_OFFSET_1], rx_cmp[BYTE_OFFSET_2], rx_cmp[BYTE_OFFSET_3],
        rx_cmp[BYTE_OFFSET_4], rx_cmp[BYTE_OFFSET_5], rx_cmp[BYTE_OFFSET_6], rx_cmp[BYTE_OFFSET_7],
        extlna[BYTE_OFFSET_0], extlna[BYTE_OFFSET_1], extlna[BYTE_OFFSET_2], extlna[BYTE_OFFSET_3],
        extlna[BYTE_OFFSET_4], extlna[BYTE_OFFSET_5], extlna[BYTE_OFFSET_6], extlna[BYTE_OFFSET_7],
        intlna[BYTE_OFFSET_0], intlna[BYTE_OFFSET_1], intlna[BYTE_OFFSET_2], intlna[BYTE_OFFSET_3],
        intlna[BYTE_OFFSET_4], intlna[BYTE_OFFSET_5], intlna[BYTE_OFFSET_6], intlna[BYTE_OFFSET_7],
        cali5g->st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i, cali5g->st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q,

        cali5g->st_cali_logen_cmp.uc_ssb_cmp,
        cali5g->st_cali_logen_cmp.uc_buf0_cmp,
        cali5g->st_cali_logen_cmp.uc_buf1_cmp,

        cali5g->st_cali_tx_power_cmp_5g.us_ppa_cmp,
        cali5g->st_cali_tx_power_cmp_5g.uc_mx_cmp,
        cali5g->st_cali_tx_power_cmp_5g.us_atx_pwr_cmp,
        cali5g->st_cali_tx_power_cmp_5g.uc_dtx_pwr_cmp,
        cali5g->st_ppf_cmp_val.uc_ppf_val,

        tx_cmp->aus_txdc_cmp_i[BYTE_OFFSET_0], tx_cmp->aus_txdc_cmp_q[BYTE_OFFSET_0],
        tx_cmp->aus_txdc_cmp_i[BYTE_OFFSET_1], tx_cmp->aus_txdc_cmp_q[BYTE_OFFSET_1],
        tx_cmp->aus_txdc_cmp_i[BYTE_OFFSET_2], tx_cmp->aus_txdc_cmp_q[BYTE_OFFSET_2],
        tx_cmp->aus_txdc_cmp_i[BYTE_OFFSET_3], tx_cmp->aus_txdc_cmp_q[BYTE_OFFSET_3],
        tx_cmp->aus_txdc_cmp_i[BYTE_OFFSET_4], tx_cmp->aus_txdc_cmp_q[BYTE_OFFSET_4],
        tx_cmp->aus_txdc_cmp_i[BYTE_OFFSET_5], tx_cmp->aus_txdc_cmp_q[BYTE_OFFSET_5],
        tx_cmp->aus_txdc_cmp_i[BYTE_OFFSET_6], tx_cmp->aus_txdc_cmp_q[BYTE_OFFSET_6],
        tx_cmp->aus_txdc_cmp_i[BYTE_OFFSET_7], tx_cmp->aus_txdc_cmp_q[BYTE_OFFSET_7]);
}
void hmac_dump_rx_en_result(mp15_cali_param_stru *cali_data, uint8_t rf_id)
{
    if (cali_data->rx_en_invalid != 0) {
        chr_exception_p(CHR_CHIP_CALI_ERR_REPORT_EVENTID, &cali_data->rx_en_invalid,
            sizeof(cali_data->rx_en_invalid));
        oam_error_log2(0, OAM_SF_CALIBRATE,
            "{hmac_dump_rx_en_result:: rf[%d] rx_en line ctrl invalid %d !}",
            rf_id, cali_data->rx_en_invalid);
    }
}

void hmac_print_mp15_base_band_result(uint8_t uc_chan_idx, mp15_cali_param_stru *cali_data)
{
    oam_warning_log3(0, OAM_SF_CALIBRATE, "chan:%d, cali_data dog_tag:%d, check_hw_status:%d",
        uc_chan_idx, cali_data->dog_tag, cali_data->check_hw_status);
    oam_warning_log4(0, OAM_SF_CALIBRATE, "update info tmp:%d, chan_idx_1_2[%d %d],cali_time:%d",
        cali_data->st_cali_update_info.bit_temperature, cali_data->st_cali_update_info.uc_5g_chan_idx1,
        cali_data->st_cali_update_info.uc_5g_chan_idx2,
        cali_data->st_cali_update_info.us_cali_time);
    oam_warning_log4(0, OAM_SF_CALIBRATE,
        "st_rc_r_c_cali_data save_all:%d, c_code[0x%x], rc_code[0x%x] r_code[0x%x]",
        cali_data->en_save_all, cali_data->st_rc_r_c_cali_data.uc_c_cmp_code,
        cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code,
        cali_data->st_rc_r_c_cali_data.uc_r_cmp_code);

    oam_warning_log3(0, OAM_SF_CALIBRATE, "st_rc_r_c_cali_data save_all:classa[0x%x], classb[0x%x], close_fem[%d]",
        cali_data->st_pa_ical_cmp.uc_classa_cmp, cali_data->st_pa_ical_cmp.uc_classb_cmp,
        cali_data->en_need_close_fem_cali);
}

uint32_t hmac_print_mp15_2g_all(mp15_cali_param_stru *cali_data, int8_t *pc_print_buff)
{
    uint32_t string_len = 0;
    int32_t l_string_tmp_len;
    uint8_t uc_cali_chn_idx_1;
    for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 < OAL_2G_CHANNEL_NUM; uc_cali_chn_idx_1++) {
        l_string_tmp_len = hmac_print_mp15_2g_cail_result(uc_cali_chn_idx_1, pc_print_buff + string_len,
            (int32_t)(OAM_REPORT_MAX_STRING_LEN - string_len - 1), cali_data);
        if (l_string_tmp_len <= 0) {
            return OAL_FAIL;
        }
        string_len += (uint32_t)l_string_tmp_len;
    }
    hmac_print_buff(pc_print_buff);
    return OAL_SUCC;
}

uint32_t hmac_print_mp15_5g_all(mp15_cali_param_stru *cali_data, int8_t *pc_print_buff)
{
    uint32_t string_len;
    int32_t l_string_tmp_len;
    uint8_t uc_cali_chn_idx_1, uc_cali_chn_idx;
    for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 <= (OAL_5G_CHANNEL_NUM >> BIT_OFFSET_1); uc_cali_chn_idx_1++) {
        string_len = 0;
        for (uc_cali_chn_idx = (uc_cali_chn_idx_1 << BIT_OFFSET_1);
             uc_cali_chn_idx < oal_min(OAL_5G_CHANNEL_NUM, ((uc_cali_chn_idx_1 + 1) << BIT_OFFSET_1));
             uc_cali_chn_idx++) {
            l_string_tmp_len = hmac_print_mp15_5g_cail_result(uc_cali_chn_idx, pc_print_buff + string_len,
                (int32_t)(OAM_REPORT_MAX_STRING_LEN - string_len - 1), cali_data);
            if (l_string_tmp_len <= 0) {
                return OAL_FAIL;
            }
            string_len += (uint32_t)l_string_tmp_len;
        }

        hmac_print_buff(pc_print_buff);
    }
    return OAL_SUCC;
}

void hmac_dump_cali_result_mp15(void)
{
    mp15_cali_param_stru *cali_data = NULL;
    int8_t               *pc_print_buff = NULL;
    uint8_t               uc_chan_idx;
    uint8_t              *puc_bfgx_rc_code = NULL;

    oam_info_log0(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::START.}");

    if (hmac_cali_mem_alloc(&pc_print_buff, &puc_bfgx_rc_code) != OAL_SUCC) {
        return;
    }
    cali_data = (mp15_cali_param_stru *)get_cali_data_buf_addr();
    *puc_bfgx_rc_code = cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code;

    for (uc_chan_idx = 0; uc_chan_idx < WLAN_RF_CHANNEL_NUMS; uc_chan_idx++) {
        cali_data += uc_chan_idx;
        /* 线控失效打点维测 */
        hmac_dump_rx_en_result(cali_data, uc_chan_idx);

        hmac_print_mp15_base_band_result(uc_chan_idx, cali_data);

        /* new_txiq_comp_val_stru TO BE ADDED */
        /* 2.4g 不超过ota上报最大字节分3次输出 */
        if (hmac_print_mp15_2g_all(cali_data, pc_print_buff) != OAL_SUCC) {
            hmac_sprint_fail_freebuff_handle(pc_print_buff);
            return;
        }

        /* 5g */
        if (OAL_FALSE == mac_device_check_5g_enable_per_chip()) {
            continue;
        }

        /* 5g 不超过ota上报最大字节分3次输出 */
        if (hmac_print_mp15_5g_all(cali_data, pc_print_buff) != OAL_SUCC) {
            hmac_sprint_fail_freebuff_handle(pc_print_buff);
            return;
        }
    }
    oal_mem_free_m(pc_print_buff, OAL_TRUE);
    return;
}

OAL_STATIC void hmac_print_mp16_base_cali_result(wlan_cali_data_para_stru *cali_param)
{
    wlan_cali_ctl_para_stru *cali_data = &cali_param->common_cali_data.cali_ctl_data;
    oam_warning_log4(0, OAM_SF_CALIBRATE, "check_hw_status:%d %d %d %d", cali_data->check_hw_status[0],
        cali_data->check_hw_status[1], cali_data->check_hw_status[2], cali_data->check_hw_status[3]); // 0/1/2/3: rf 0-3
    oam_warning_log4(0, OAM_SF_CALIBRATE, "update info tmp:%d, chan_idx_1_2[%d %d],cali_time:%d",
        cali_data->cali_update_info.bit_temperature,
        cali_data->cali_update_info.uc_5g_chan_idx1,
        cali_data->cali_update_info.uc_5g_chan_idx2,
        cali_data->cali_update_info.us_cali_time);
}
int32_t hmac_print_mp16_cali_result(uint8_t rf_idx, uint8_t chn_idx,
    int8_t *pc_print_buff, int32_t remain_len, wlan_cali_basic_para_stru *basic_cali_data)
{
    int8_t *pc_string;
    wlan_cali_rxdc_para_stru *rxdc_cali_data = &basic_cali_data->rxdc_cali_data;
    wlan_cali_txdc_para_stru *txdc_cali_data = &basic_cali_data->txdc_cali_data;
    pc_string = "cali data rf%u band[%u]\n"
        "analog_rxdc intlna[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n"
        "analog_rxdc extlna[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n"
        "digital_rxdc_i_q[0x%x 0x%x]\n"
        "tx_power[ppa:%u ppa_cap:%u pa_cap:%u mx_cap:%u]\n"
        "tx_dc [0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return snprintf_s(pc_print_buff, (uint32_t)remain_len, (uint32_t)remain_len - 1,
        pc_string, rf_idx, chn_idx,
        rxdc_cali_data->analog_rxdc_cmp_intlna_i[0], rxdc_cali_data->analog_rxdc_cmp_intlna_q[0], // 0: rxdc_lna_lvl0
        rxdc_cali_data->analog_rxdc_cmp_intlna_i[1], rxdc_cali_data->analog_rxdc_cmp_intlna_q[1], // 1: rxdc_lna_lvl1
        rxdc_cali_data->analog_rxdc_cmp_intlna_i[2], rxdc_cali_data->analog_rxdc_cmp_intlna_q[2], // 2: rxdc_lna_lvl2
        rxdc_cali_data->analog_rxdc_cmp_intlna_i[3], rxdc_cali_data->analog_rxdc_cmp_intlna_q[3], // 3: rxdc_lna_lvl3
        rxdc_cali_data->analog_rxdc_cmp_intlna_i[4], rxdc_cali_data->analog_rxdc_cmp_intlna_q[4], // 4: rxdc_lna_lvl4
        rxdc_cali_data->analog_rxdc_cmp_intlna_i[5], rxdc_cali_data->analog_rxdc_cmp_intlna_q[5], // 5: rxdc_lna_lvl5
        rxdc_cali_data->analog_rxdc_cmp_intlna_i[6], rxdc_cali_data->analog_rxdc_cmp_intlna_q[6], // 6: rxdc_lna_lvl6
        rxdc_cali_data->analog_rxdc_cmp_intlna_i[7], rxdc_cali_data->analog_rxdc_cmp_intlna_q[7], // 7: rxdc_lna_lvl7
        rxdc_cali_data->analog_rxdc_cmp_extlna_i[0], rxdc_cali_data->analog_rxdc_cmp_extlna_q[0], // 0: rxdc_lna_lvl0
        rxdc_cali_data->analog_rxdc_cmp_extlna_i[1], rxdc_cali_data->analog_rxdc_cmp_extlna_q[1], // 1: rxdc_lna_lvl1
        rxdc_cali_data->analog_rxdc_cmp_extlna_i[2], rxdc_cali_data->analog_rxdc_cmp_extlna_q[2], // 2: rxdc_lna_lvl2
        rxdc_cali_data->analog_rxdc_cmp_extlna_i[3], rxdc_cali_data->analog_rxdc_cmp_extlna_q[3], // 3: rxdc_lna_lvl3
        rxdc_cali_data->analog_rxdc_cmp_extlna_i[4], rxdc_cali_data->analog_rxdc_cmp_extlna_q[4], // 4: rxdc_lna_lvl4
        rxdc_cali_data->analog_rxdc_cmp_extlna_i[5], rxdc_cali_data->analog_rxdc_cmp_extlna_q[5], // 5: rxdc_lna_lvl5
        rxdc_cali_data->analog_rxdc_cmp_extlna_i[6], rxdc_cali_data->analog_rxdc_cmp_extlna_q[6], // 6: rxdc_lna_lvl6
        rxdc_cali_data->analog_rxdc_cmp_extlna_i[7], rxdc_cali_data->analog_rxdc_cmp_extlna_q[7], // 7: rxdc_lna_lvl7
        rxdc_cali_data->digital_rxdc_cmp_i, rxdc_cali_data->digital_rxdc_cmp_q,
        basic_cali_data->txpwr_cali_data.ppa_cmp, basic_cali_data->txpwr_cali_data.ppa_cap_cmp,
        basic_cali_data->txpwr_cali_data.pa_cap_cmp, basic_cali_data->txpwr_cali_data.mx_cap_cmp,
        txdc_cali_data->txdc_cmp_i[0], txdc_cali_data->txdc_cmp_q[0],  // 0: txdc_cali_lvl0
        txdc_cali_data->txdc_cmp_i[1], txdc_cali_data->txdc_cmp_q[1],  // 1: txdc_cali_lvl1
        txdc_cali_data->txdc_cmp_i[2], txdc_cali_data->txdc_cmp_q[2],  // 2: txdc_cali_lvl2
        txdc_cali_data->txdc_cmp_i[3], txdc_cali_data->txdc_cmp_q[3]); // 3: txdc_cali_lvl3
}

void hmac_print_error(int8_t *buff)
{
    oam_warning_log0(0, OAM_SF_CFG, "{hmac_print_error:: OAL_SPRINTF return error!}");
    hmac_print_buff(buff);
    oal_mem_free_m(buff, OAL_TRUE);
}
OAL_STATIC int32_t hmac_print_mp16_2g_common_cali_result(uint8_t rf_idx, uint8_t chn_idx,
    int8_t *buff, int32_t remain_len, wlan_cali_data_para_stru *cali_data)
{
    int8_t *pc_string = NULL;
    wlan_cali_logen_para_stru *logen_cali_data = &cali_data->common_cali_data.logen_cali_data_2g[rf_idx][chn_idx];
    wlan_cali_lp_rxdc_para_stru *lp_rxdc_cali_data = &cali_data->common_cali_data.lp_rxdc_cali_date_2g[chn_idx];
    wlan_cali_lodiv_para_stru *lodiv_cali_data = &cali_data->common_cali_data.lodiv_cali_data[rf_idx];
    wlan_cali_rc_r_c_para_stru *rc_r_c_cali_data =
        &cali_data->common_cali_data.rc_r_c_cali_data[WLAN_CALI_BAND_2G][rf_idx];
    if (remain_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE, "hmac_print_2g_common_cali_result:len[%d] max[%d] cali_size[%d]",
            remain_len, OAM_REPORT_MAX_STRING_LEN, sizeof(mp16_cali_param_stru));
        return remain_len;
    }
    pc_string = "2G: common cali data rf%u band[%u]\n"
        "RC [0x%x] R[0x%x] C[0x%x] 20M RC[%x]ABBRC[%x] lna blk [0x%x] logen comp:[ssb %x vb %x buf %x %x] \n"
        "lp rxdc comp:analog [0x%x 0x%x] digital [0x%x 0x%x]\n"
        "lodiv comp:[0x%x 0x%x 0x%x 0x%x]\n";

    return snprintf_s(buff, (uint32_t)remain_len, (uint32_t)remain_len - 1, pc_string, rf_idx, chn_idx,
        rc_r_c_cali_data->rc_cmp_code, rc_r_c_cali_data->r_cmp_code, rc_r_c_cali_data->c_cmp_code,
        rc_r_c_cali_data->rc_20m_cmp_code, rc_r_c_cali_data->abbrc_cmp_code,
        cali_data->common_cali_data.lna_blk_cali_data[WLAN_CALI_BAND_2G][rf_idx].fine_code,
        logen_cali_data->lo_ssb_tune, logen_cali_data->lo_vb_tune,
        logen_cali_data->lo_buf_tune[0], logen_cali_data->lo_buf_tune[1],
        lp_rxdc_cali_data->analog_lp_rxdc_cmp_i[0], lp_rxdc_cali_data->analog_lp_rxdc_cmp_q[0],
        lp_rxdc_cali_data->digital_lp_rxdc_cmp_i, lp_rxdc_cali_data->digital_lp_rxdc_cmp_q,
        lodiv_cali_data->tx_lo, lodiv_cali_data->rx_lo, lodiv_cali_data->tx_dpd_lo_tx, lodiv_cali_data->tx_dpd_lo_dpd);
}

OAL_STATIC int32_t hmac_print_mp16_5g_common_cali_result(uint8_t rf_idx, uint8_t chn_idx,
    int8_t *buff, int32_t remain_len, wlan_cali_data_para_stru *cali_data)
{
    int8_t *pc_string = NULL;
    wlan_cali_logen_para_stru *logen_cali_data = &cali_data->common_cali_data.logen_cali_data_5g[rf_idx][chn_idx];
    wlan_cali_lp_rxdc_para_stru *lp_rxdc_cali_date = &cali_data->common_cali_data.lp_rxdc_cali_date_5g[chn_idx];
    wlan_cali_ppf_para_stru *ppf_cali_data = &cali_data->common_cali_data.ppf_cali_data_5g[rf_idx][chn_idx];
    wlan_cali_rc_r_c_para_stru *rc_r_c_cali_data =
        &cali_data->common_cali_data.rc_r_c_cali_data[WLAN_CALI_BAND_5G][rf_idx];
    if (remain_len <= 0) {
        oam_error_log3(0, OAM_SF_CALIBRATE, "hmac_print_5g_common_cali_result:len[%d] max[%d] cali_size[%d]",
            remain_len, OAM_REPORT_MAX_STRING_LEN, sizeof(mp16_cali_param_stru));
        return remain_len;
    }
    pc_string = "5G: common cali data rf%u band[%u]\n"
        "RC [0x%x] R[0x%x] C[0x%x] 20M RC[%x] ABBRC[%x]lna blk [0x%x] logen comp:[ssb %x vb %x buf %x %x] \n"
        "lp rxdc comp:analog [0x%x 0x%x] digital [0x%x 0x%x]\n"
        "ppf comp:[0x%x 0x%x]\n";

    return snprintf_s(buff, (uint32_t)remain_len, (uint32_t)remain_len - 1, pc_string, rf_idx, chn_idx,
        rc_r_c_cali_data->rc_cmp_code, rc_r_c_cali_data->r_cmp_code, rc_r_c_cali_data->c_cmp_code,
        rc_r_c_cali_data->rc_20m_cmp_code, rc_r_c_cali_data->abbrc_cmp_code,
        cali_data->common_cali_data.lna_blk_cali_data[WLAN_CALI_BAND_5G][rf_idx].fine_code,
        logen_cali_data->lo_ssb_tune, logen_cali_data->lo_vb_tune,
        logen_cali_data->lo_buf_tune[0], logen_cali_data->lo_buf_tune[1],
        lp_rxdc_cali_date->analog_lp_rxdc_cmp_i[0], lp_rxdc_cali_date->analog_lp_rxdc_cmp_q[0],
        lp_rxdc_cali_date->digital_lp_rxdc_cmp_i, lp_rxdc_cali_date->digital_lp_rxdc_cmp_q,
        ppf_cali_data->ssb_ppf_code, ppf_cali_data->trx_ppf_code);
}

uint32_t hmac_dump_mp16_2g_cali(int8_t *buff, wlan_cali_data_para_stru *cali_data, uint8_t rf_idx)
{
    uint8_t   chn_idx;
    int32_t   str_tmp_len;
    int32_t   remain_len;
    uint32_t  str_len     = 0;
    int8_t *pc_string = "2G:";
    wlan_cali_basic_para_stru *basic_cali_data = NULL;
    for (chn_idx = 0; chn_idx < OAL_2G_CHANNEL_NUM; chn_idx++) {
        remain_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - str_len - 1);
        if (remain_len <= 0) {
            hmac_print_error(buff);
            return OAL_FAIL;
        }
        str_tmp_len = snprintf_s(buff + str_len, (uint32_t)remain_len, (uint32_t)remain_len - 1, pc_string);
        if (str_tmp_len < 0) {
            return OAL_FAIL;
        }
        str_len += (uint32_t)str_tmp_len;

        basic_cali_data = &cali_data->rf_cali_data[rf_idx].cali_data_2g_20m.cali_data[chn_idx];
        str_tmp_len = hmac_print_mp16_cali_result(rf_idx, chn_idx, buff + str_len, remain_len, basic_cali_data);
        if (str_tmp_len < 0) {
            return OAL_FAIL;
        }
        str_len += (uint32_t)str_tmp_len;

        str_tmp_len = hmac_print_mp16_2g_common_cali_result(rf_idx, chn_idx,
            buff + str_len, remain_len, cali_data);
        if (str_tmp_len < 0) {
            return OAL_FAIL;
        }
        str_len += (uint32_t)str_tmp_len;
    }

    hmac_print_buff(buff);
    return OAL_SUCC;
}
int32_t hmac_dump_mp16_5g_bw_info(int8_t *buff, int32_t remain_len, uint8_t chn_idx)
{
    int8_t *bw_str[] = {"5G 20M:", "5G 40M:", "5G 80M:", "5G 160M:"};
    int8_t *pc_string = NULL;
    if (chn_idx < OAL_5G_CHANNEL_NUM && chn_idx >= (OAL_5G_20M_CHANNEL_NUM + OAL_5G_80M_CHANNEL_NUM)) {
        pc_string = bw_str[WLAN_BANDWIDTH_160];
    } else if (chn_idx < (OAL_5G_20M_CHANNEL_NUM + OAL_5G_80M_CHANNEL_NUM) && chn_idx >= OAL_5G_20M_CHANNEL_NUM) {
        pc_string = bw_str[WLAN_BANDWIDTH_80];
    } else {
        pc_string = bw_str[WLAN_BANDWIDTH_20];
    }
    return snprintf_s(buff, (uint32_t)remain_len, (uint32_t)remain_len - 1, pc_string);
}
uint32_t hmac_dump_mp16_5g_cali(int8_t *buff, wlan_cali_data_para_stru *cali_data, uint8_t rf_idx)
{
    uint8_t chn_idx, band_idx;
    int32_t str_tmp_len, remain_len;
    uint32_t str_len = 0;
    wlan_cali_basic_para_stru *basic_cali_data = NULL;
    uint8_t print_cnt = 0;
    for (chn_idx = 0; chn_idx < OAL_5G_CHANNEL_NUM; chn_idx++) {
        remain_len = (int32_t)(OAM_REPORT_MAX_STRING_LEN - str_len - 1);
        if (remain_len <= 0) {
            hmac_print_error(buff);
            return OAL_FAIL;
        }

        print_cnt++;
        if (chn_idx < OAL_5G_CHANNEL_NUM && chn_idx >= (OAL_5G_20M_CHANNEL_NUM + OAL_5G_80M_CHANNEL_NUM)) {
            band_idx = chn_idx - (OAL_5G_20M_CHANNEL_NUM + OAL_5G_80M_CHANNEL_NUM);
            basic_cali_data = &cali_data->rf_cali_data[rf_idx].cali_data_5g_160m.cali_data[band_idx];
        } else if (chn_idx < (OAL_5G_20M_CHANNEL_NUM + OAL_5G_80M_CHANNEL_NUM) && chn_idx >= OAL_5G_20M_CHANNEL_NUM) {
            band_idx = chn_idx - OAL_5G_20M_CHANNEL_NUM;
            basic_cali_data = &cali_data->rf_cali_data[rf_idx].cali_data_5g_80m.cali_data[band_idx];
        } else {
            band_idx = chn_idx;
            basic_cali_data = &cali_data->rf_cali_data[rf_idx].cali_data_5g_20m.cali_data[band_idx];
        }
        str_tmp_len = hmac_dump_mp16_5g_bw_info(buff + str_len, remain_len, chn_idx);
        if (str_tmp_len < 0) {
            return OAL_FAIL;
        }
        str_len += (uint32_t)str_tmp_len;

        str_tmp_len = hmac_print_mp16_cali_result(rf_idx, band_idx, buff + str_len, remain_len, basic_cali_data);
        if (str_tmp_len < 0) {
            return OAL_FAIL;
        }
        str_len += (uint32_t)str_tmp_len;

        if (chn_idx < OAL_5G_20M_CHANNEL_NUM) {
            str_tmp_len = hmac_print_mp16_5g_common_cali_result(rf_idx, chn_idx,
                buff + str_len, remain_len, cali_data);
            if (str_tmp_len < 0) {
                return OAL_FAIL;
            }
            str_len += (uint32_t)str_tmp_len;
        }

        if (print_cnt % 3 == 0 || print_cnt == OAL_5G_CHANNEL_NUM) { // 3: 内存受限，一次打印只能输出3次数据
            hmac_print_buff(buff);
            str_len = 0;
        }
    }
    return OAL_SUCC;
}

void hmac_dump_cali_result_mp16(void)
{
    wlan_cali_data_para_stru *cali_data;
    int8_t               *buff;
    uint8_t               rf_idx;
    uint32_t              ret;
    uint8_t               rf_num = hmac_get_rf_num();
    /* OTA上报 */
    buff = (int8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    cali_data = (wlan_cali_data_para_stru *)get_cali_data_buf_addr();
    if ((buff == NULL) || (cali_data == NULL)) {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::para null.}");
        oal_mem_free_m(buff, OAL_TRUE);
        return;
    }
    hmac_print_mp16_base_cali_result(cali_data);

    for (rf_idx = 0; rf_idx < rf_num; rf_idx++) {
        /* 2.4g 不超过ota上报最大字节分3次输出 */
        ret = hmac_dump_mp16_2g_cali(buff, cali_data, rf_idx);
        if (ret != OAL_SUCC) {
            oal_mem_free_m(buff, OAL_TRUE);
            return;
        }

        /* 5g 不超过ota上报最大字节分3次输出 */
        ret = hmac_dump_mp16_5g_cali(buff, cali_data, rf_idx);
        if (ret != OAL_SUCC) {
            oal_mem_free_m(buff, OAL_TRUE);
            return;
        }
    }
    oal_mem_free_m(buff, OAL_TRUE);
}
