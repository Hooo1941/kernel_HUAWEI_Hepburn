/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_frag.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2014年2月15日
 */

#ifndef HMAC_FRAG_H
#define HMAC_FRAG_H

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "oam_stat_wifi.h"
#include "hmac_main.h"
#include "hmac_tx_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FRAG_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

oal_netbuf_stru *hmac_defrag_process(hmac_user_stru *pst_hmac_user,
                                     oal_netbuf_stru *pst_netbuf,
                                     uint32_t hrdsize);
uint32_t hmac_frag_process(hmac_vap_stru *pst_hmac_vap,
                           oal_netbuf_stru *pst_netbuf_original,
                           mac_tx_ctl_stru *pst_tx_ctl,
                           uint32_t cip_hdrsize,
                           uint32_t max_tx_unit);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_frag.h */
