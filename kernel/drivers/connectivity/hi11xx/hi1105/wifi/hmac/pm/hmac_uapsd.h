/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_uapsd.c 的头文件
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#ifndef HMAC_UAPSD_H
#define HMAC_UAPSD_H

#include "mac_user.h"
#include "hmac_ext_if.h"

#include "hmac_user.h"
#include "hmac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_UAPSD_H

void hmac_uapsd_update_user_para(hmac_user_stru *hmac_user, uint8_t *payload, uint32_t msg_len);

#endif /* end of hmac_uapsd.h */
