/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_soft_encrypt.c的头文件
 * 作    者 :
 * 创建日期 : 2022年8月30日
 */
#ifndef HMAC_SOFT_ENCRYPT_H
#define HMAC_SOFT_ENCRYPT_H
#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))

#include "mac_vap.h"

typedef struct {
    oal_netbuf_head_stru netbuf_header;
    struct semaphore encrypt_sche_sema;
    oal_task_stru *encrypt_schedule_thread;
} hmac_encrypt_schedule_stru;

hmac_encrypt_schedule_stru *hmac_get_encrypt_schedule_list(void);
int32_t hmac_soft_encrypt_schedule_thread(void *data);
uint32_t  hmac_soft_encrypt_add_list(oal_netbuf_stru *netbuf);
oal_bool_enum_uint8 hmac_check_soft_encrypt_port(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf);
oal_bool_enum_uint8 hmac_check_soft_encrypt_thread(void);
uint32_t hmac_soft_encrypt_netbuf(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf, oal_netbuf_stru **netbuf_new);

#endif
#endif /* end of hmac_soft_encrypt.h */
