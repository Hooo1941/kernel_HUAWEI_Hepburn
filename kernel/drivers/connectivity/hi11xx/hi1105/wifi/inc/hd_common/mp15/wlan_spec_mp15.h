/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : MP15 wlan产品规格宏定义
 * 作    者 :
 * 创建日期 : 2020年06月15日
 */

#ifndef WLAN_SPEC_MP15_H
#define WLAN_SPEC_MP15_H

/* 作为P2P GO 允许关联最大用户数 */
#ifdef _PRE_WINDOWS_SUPPORT
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP15 8
#else
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP15 4
#endif

/* 支持的建立rx ba 的最大个数 */
#define WLAN_MAX_RX_BA_MP15 32

/* 支持的建立tx ba 的最大个数 */
#define WLAN_MAX_TX_BA_MP15 32

/* 其他BSS的广播帧上报hal vap ID */
#define WLAN_HAL_OHTER_BSS_ID_MP15 14

#define WLAN_AMPDU_TX_MAX_NUM_MP15 64 /* AMPDU发送端最大聚合子MPDU个数 */
#endif /* #ifndef WLAN_SPEC_MP15_H */

