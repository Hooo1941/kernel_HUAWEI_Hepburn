/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 1106 wlan产品规格宏定义
 * 作    者 :
 * 创建日期 : 2020年06月15日
 */

#ifndef WLAN_SPEC_MP16_H
#define WLAN_SPEC_MP16_H


/* 作为P2P GO 允许关联最大用户数 */
#ifdef _PRE_WINDOWS_SUPPORT
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP16 8
#else
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP16 4
#endif

/* 支持的建立rx ba 的最大个数 */
#define WLAN_MAX_RX_BA_MP16 32

/* 支持的建立tx ba 的最大个数 */
#define WLAN_MAX_TX_BA_MP16 1024

/* MP16 16个VAP只最后4个支持STA和AP模式 */
#define MP16_HAL_VAP_OFFSET 12
#define MP16_OTHER_BSS_ID 0xFF

/* MP16 rx cb vap id 5bit 全1表示其他bSS */
#define WLAN_HAL_OHTER_BSS_ID_MP16 0x1F

/* MP16 tx最大ampdu聚合规格 */
#define WLAN_AMPDU_TX_MAX_NUM_MP16 256

#endif /* #ifndef WLAN_SPEC_MP16_H */

