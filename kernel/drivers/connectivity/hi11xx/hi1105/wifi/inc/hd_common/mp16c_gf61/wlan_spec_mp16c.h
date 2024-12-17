/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 ：shenkuo wlan产品规格宏定义
 * 作    者 :
 * 创建日期 : 2020年06月15日
 */

#ifndef WLAN_SPEC_MP16C_H
#define WLAN_SPEC_MP16C_H

/* MP16C 规格待确认 */
/* 作为P2P GO 允许关联最大用户数 */
#ifdef _PRE_WINDOWS_SUPPORT
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP16C 8
#else
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP16C 4
#endif

/* 支持的建立rx ba 的最大个数 */
#define WLAN_MAX_RX_BA_MP16C 32

/* 支持的建立tx ba 的最大个数 */
#define WLAN_MAX_TX_BA_MP16C 1024

/* mp16c 16个VAP只最后4个(12~15)支持STA和AP模式 */
#define MP16C_HAL_VAP_OFFSET 12
#define MP16C_OTHER_BSS_ID 0xFF

/* rx cb vap id 5bit 全1表示其他BSS */
#define WLAN_HAL_OHTER_BSS_ID_MP16C 0x1F

/* tx最大ampdu聚合规格 */
#define WLAN_AMPDU_TX_MAX_NUM_MP16C 256

#endif /* #ifndef WLAN_SPEC_MP16C_H */

