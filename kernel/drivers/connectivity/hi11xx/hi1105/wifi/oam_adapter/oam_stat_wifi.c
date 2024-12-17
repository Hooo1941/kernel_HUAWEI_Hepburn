/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi oam 统计
 * 作者       : wifi
 * 创建日期   : 2012年9月18日
 */

#include "oam_stat_wifi.h"
#include "oam_ext_if.h"

oam_stat_info_stru g_oam_stat_info;

/* ota_type->oam_ota_type_enum */
OAL_STATIC uint32_t oam_stats_get_stat_info_length(uint8_t ota_type, uint16_t *stat_info_len)
{
    switch (ota_type) {
        case OAM_OTA_TYPE_DEV_STAT_INFO:
            *stat_info_len = (uint16_t)(sizeof(oam_device_stat_info_stru) * WLAN_DEVICE_MAX_NUM_PER_CHIP);
            break;
        case OAM_OTA_TYPE_VAP_STAT_INFO:
            *stat_info_len = (uint16_t)(sizeof(oam_vap_stat_info_stru) * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);
            break;
        case OAM_OTA_TYPE_USER_STAT_INFO:
            *stat_info_len = (uint16_t)(sizeof(oam_user_stat_info_stru));
            break;
        default:
            oal_io_print("oam_stats_report_info_to_sdt::ota_type invalid-->%d!\n", ota_type);
            return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}

/* ota_type->oam_ota_type_enum */
OAL_STATIC uint32_t oam_stats_set_ota_data(oam_ota_stru *ota_data, uint8_t ota_type,
    uint16_t stat_info_len, uint32_t *params)
{
    int32_t ret;
    uint16_t user_id;

    switch (ota_type) {
        case OAM_OTA_TYPE_DEV_STAT_INFO:
            ret = memcpy_s((void *)ota_data->auc_ota_data, (uint32_t)ota_data->st_ota_hdr.us_ota_data_len,
                (const void *)g_oam_stat_info.ast_dev_stat_info, (uint32_t)stat_info_len);
            break;
        case OAM_OTA_TYPE_VAP_STAT_INFO:
            ret = memcpy_s((void *)ota_data->auc_ota_data, (uint32_t)ota_data->st_ota_hdr.us_ota_data_len,
                (const void *)g_oam_stat_info.ast_vap_stat_info, (uint32_t)stat_info_len);
            break;
        case OAM_OTA_TYPE_USER_STAT_INFO:
            user_id = (uint16_t)params[0];
            ret = memcpy_s((void *)ota_data->auc_ota_data, (uint32_t)ota_data->st_ota_hdr.us_ota_data_len,
                (const void *)&g_oam_stat_info.ast_user_stat_info[user_id], (uint32_t)stat_info_len);
            break;
        default:
            return OAL_FAIL;
    }
    if (ret != EOK) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/* en_ota_type->oam_ota_type_enum */
uint32_t oam_stats_report_info_to_sdt(uint8_t en_ota_type, uint32_t *params)
{
    uint16_t us_skb_len, us_stat_info_len;
    oal_netbuf_stru *pst_netbuf = NULL;
    oam_ota_stru *pst_ota_data = NULL;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oam_stats_get_stat_info_length(en_ota_type, &us_stat_info_len) != OAL_SUCC) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 为上报统计信息申请空间,头部预留8字节，尾部预留1字节，给sdt_drv用 */
    us_skb_len = us_stat_info_len + sizeof(oam_ota_hdr_stru);
    if (us_skb_len > SDT_NETBUF_MAX_PAYLOAD) {
        us_skb_len = SDT_NETBUF_MAX_PAYLOAD;
        us_stat_info_len = SDT_NETBUF_MAX_PAYLOAD - sizeof(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* 填写ota消息头结构体 */
    pst_ota_data->st_ota_hdr.ul_tick = (uint32_t)oal_time_get_stamp_ms();
    pst_ota_data->st_ota_hdr.en_ota_type = en_ota_type;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    pst_ota_data->st_ota_hdr.us_ota_data_len = us_stat_info_len;

    /* 复制数据,填写ota数据 */
    if (oam_stats_set_ota_data(pst_ota_data, en_ota_type, us_stat_info_len, params) != OAL_SUCC) {
        oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
        oal_io_print("oam_stats_report_info_to_sdt:: memcpy_s failed\r\n");
        return OAL_FAIL;
    }

    /* 下发至sdt接收队列，若队列满则串口输出 */
    return oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);
}

uint32_t oam_stats_clear_vap_stat_info(uint8_t uc_vap_id)
{
    if (uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    memset_s(&g_oam_stat_info.ast_vap_stat_info[uc_vap_id], sizeof(oam_vap_stat_info_stru),
             0, sizeof(oam_vap_stat_info_stru));

    return OAL_SUCC;
}

void oam_stats_clear_stat_info(void)
{
    memset_s(&g_oam_stat_info, sizeof(oam_stat_info_stru), 0, sizeof(oam_stat_info_stru));
}

uint32_t oam_stats_clear_user_stat_info(uint16_t us_usr_id)
{
    if (us_usr_id >= WLAN_USER_MAX_USER_LIMIT) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    memset_s(&g_oam_stat_info.ast_user_stat_info[us_usr_id], sizeof(oam_user_stat_info_stru),
             0, sizeof(oam_user_stat_info_stru));
    return OAL_SUCC;
}

uint32_t oam_stats_report_usr_info(uint16_t us_usr_id)
{
    uint32_t params = us_usr_id;
    return oam_stats_report_info_to_sdt(OAM_OTA_TYPE_USER_STAT_INFO, &params);
}
