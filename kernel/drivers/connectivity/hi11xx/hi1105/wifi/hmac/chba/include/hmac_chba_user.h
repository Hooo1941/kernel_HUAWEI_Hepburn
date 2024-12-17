/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : CHBA user相关操作头文件
* 作者       : wifi
* 创建日期   : 2021年3月18日
*/

#ifndef HMAC_CHBA_USER_H
#define HMAC_CHBA_USER_H

#include "mac_chba_common.h"
#include "hmac_user.h"
#include "hmac_chba_mgmt.h"
#include "hmac_chba_function.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHBA_USER_H

#ifdef _PRE_WLAN_CHBA_MGMT

/* 函数声明 */
void hmac_chba_user_set_connect_role(hmac_user_stru *hmac_user,
    chba_connect_role_enum connect_role, oal_bool_enum_uint8 is_fast_conn_initiate);
void hmac_chba_user_set_ssid(hmac_user_stru *hmac_user, uint8_t *ssid, uint8_t ssid_len);
uint32_t hmac_chba_add_user(mac_vap_stru *mac_vap, uint8_t *peer_addr, uint16_t *user_idx);
void hmac_chba_del_user_proc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
uint32_t hmac_chba_user_mgmt_conn_prepare(mac_vap_stru *mac_vap, hmac_chba_conn_param_stru *chba_conn_params,
    int16_t *user_idx, uint8_t connect_role, oal_bool_enum_uint8 is_fast_conn_initiate);
void hmac_chba_save_update_beacon_pnf(uint8_t *bssid);
#endif
#endif
