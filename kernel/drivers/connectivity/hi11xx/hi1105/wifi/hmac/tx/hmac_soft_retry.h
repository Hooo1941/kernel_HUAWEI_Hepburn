/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac Host tx软重传模块
 * 作    者 : wifi
 * 创建日期 : 2020年7月24日
 */

#ifndef HMAC_SOFT_RETRY_H
#define HMAC_SOFT_RETRY_H

#include "hmac_tid.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SOFT_RETRY_H

uint32_t hmac_tx_soft_retry_process(hmac_tid_info_stru *tx_tid_info, oal_netbuf_stru *netbuf);

#endif
