/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_mgmt_sta_frame.c 的头文件
 * 作    者 :
 * 创建日期 : 2021年1月12日
 */

#ifndef HMAC_MGMT_STA_UP_H
#define HMAC_MGMT_STA_UP_H

#include "oal_ext_if.h"
#include "hmac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID       OAM_FILE_ID_HMAC_MGMT_STA_UP_H

uint32_t hmac_get_frame_body_len(oal_netbuf_stru *net_buf);
uint32_t hmac_sta_up_update_edca_params_machw(hmac_vap_stru *pst_hmac_sta, mac_wmm_set_param_type_enum_uint8 en_type);
void hmac_sta_up_update_edca_params(uint8_t *puc_payload, uint16_t us_msg_len, hmac_vap_stru *pst_hmac_sta,
    uint8_t uc_frame_sub_type, hmac_user_stru *pst_hmac_user);
void hmac_sta_handle_disassoc_rsp(hmac_vap_stru *hmac_vap, hmac_disconnect *disconnect);
void hmac_sta_rx_radio_measurment(hmac_vap_stru *pst_hmac_vap, hmac_user_stru *pst_hmac_user,
    oal_netbuf_stru *pst_netbuf, uint8_t *puc_data);
uint32_t hmac_sta_up_rx_mgmt(hmac_vap_stru *hmac_vap, void *param);

#endif /* end of hmac_mgmt_sta_up.h */
