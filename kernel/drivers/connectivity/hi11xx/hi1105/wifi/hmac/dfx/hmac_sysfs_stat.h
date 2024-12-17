/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_uapsd.c 的头文件
 * 作    者 :
 * 创建日期 : 2013年9月18日
 */

#ifndef HMAC_SYSFS_STAT_H
#define HMAC_SYSFS_STAT_H

#include "oam_ext_if.h"
#include "hmac_ext_if.h"
#include "wlan_oneimage_define.h"

int32_t hmac_sysfs_entry_init(void);
int32_t hmac_sysfs_entry_exit(void);
int32_t hmac_print_vap_stat(void *data, char *buf, int32_t buf_len);
#endif /* end of hmac_uapsd.h */
