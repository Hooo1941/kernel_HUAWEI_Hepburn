/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 保护相关
 * 作    者 :
 * 创建日期 : 2014年1月18日
 */

#ifndef HMAC_PROTECTION_H
#define HMAC_PROTECTION_H

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "mac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_PROTECTION_H
/* 2 宏定义 */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
void hmac_protection_del_user(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
uint32_t hmac_user_protection_sync_data(mac_vap_stru *pst_mac_vap);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_protection.h */
