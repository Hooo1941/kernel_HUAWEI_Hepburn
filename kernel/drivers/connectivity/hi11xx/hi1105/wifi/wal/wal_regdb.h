/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_regdb.c 的头文件
 * 作    者 :
 * 创建日期 : 2013年10月15日
 */

#ifndef WAL_REGDB_H
#define WAL_REGDB_H

#include "oal_ext_if.h"
#include "wlan_oneimage_define.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_REGDB_H

#define MP13_SPECIFIC_COUNTRY_CODE_IDX 0

#define NUM_2_REG_RULE  2
#define NUM_3_REG_RULE  3
#define NUM_4_REG_RULE  4
#define NUM_5_REG_RULE  5
#define NUM_6_REG_RULE  6

extern const oal_ieee80211_regdomain_stru *g_reg_regdb[];
extern int g_reg_regdb_size;
extern const oal_ieee80211_regdomain_stru g_st_default_regdom;
const oal_ieee80211_regdomain_stru *wal_regdb_find_db(int8_t *str);
#endif /* end of wal_regdb.h */
