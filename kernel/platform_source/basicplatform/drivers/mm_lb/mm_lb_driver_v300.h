/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MM_LB_DRIVER_V300_H
#define __MM_LB_DRIVER_V300_H

#include "mm_lb_priv.h"

#define     GLB_ROUTE_PLC_S          0x0060
#define     CFG_INT_STATUS           0x2000
#define     CMO_CFG_INT_INI          0x2004
#define     CMO_CFG_INT_EN           0x2008
#define     CMO_CFG_INT_CLR          0x200C
#define     CFG_CMO_STATUS           0x2100
#define     CFG_CMO_CLEAR            0x2200
#define     SC_RT_INT_EN             0x2308
#define     SC_RT_INT_CLR            0x230C
#define     GID_QUOTA               0x4000
#define     GID_WAY_ALLC            0x4004
#define     GID_ALLC_PLC            0x4008
#define     GID_MID_FLT0            0x400C
#define     GID_MID_FLT1            0x4010

#define     CMO_CMD                 0x5000
#define     CMO_STAT                CFG_CMO_STATUS
#define     CMO_CLEAR               CFG_CMO_CLEAR

#define     GLB_PMU_CTRL             0x0820
#define     GLB_PMU_CNT8_11_MODE_SEL 0x0824
#define     SLC_CM_CNT_EN           0x6004
#define     SLC_CM_CNT_MUX0         0x6008
#define     SLC_PMU_CNT0_TH_L       0x6014
#define     SLC_PMU_CNT0_TH_H       0x6018

#define     SLC_BP5_CFG             0x6098
#define     SLC_BP6_CFG             0x609C
#define     SLC_BP7_CFG_0           0x60A0
#define     SLC_BP7_CFG_1           0x60A4
#define     SLC_BP7_CFG_2           0x60A8
#define     SLC_BP8_CFG_0           0x60AC
#define     SLC_BP8_CFG_1           0x60B0
#define     SLC_BP8_CFG_3           0x60B4
#define     SLC_BP8_CFG_4           0x60B8
#define     SLC_BP9_CFG_0           0x60BC
#define     SLC_BP10_CFG_0          0x60C8
#define     SLC_BP10_CFG_1          0x60CC
#define     SLC_BP10_CFG_2          0x60D0
#define     SLC_BP11_CFG_0          0x60D4
#define     SLC_BP12_CFG_0          0x60DC
#define     SLC_BP12_CFG_1          0x60E0
#define     SLC_BP12_CFG_2          0x60E4
#define     SLC_BP14A_CFG_0         0x60E8
#define     SLC_BP14A_CFG_1         0x60EC
#define     SLC_BP14B_CFG_0         0x60F0
#define     SLC_BP14B_CFG_1         0x60F4
#define     SLC_BP15_CFG_0          0x60F8

#define     SLC_TCP0_CFG_0          0x60FC
#define     SLC_TCP0_CFG_1          0x6100
#define     SLC_TCP0_CFG_2          0x6104
#define     SLC_TCP1_CFG_0          0x6108
#define     SLC_TCP1_CFG_1          0x610C
#define     SLC_TCP1_CFG_2          0x6110
#define     SLC_TCP2A_CFG_0         0x6114
#define     SLC_TCP2A_CFG_1         0x6118
#define     SLC_TCP2B_CFG_0         0x611C
#define     SLC_TCP2B_CFG_1         0x6120
#define     SLC_TCP3A_CFG_0         0x62E4
#define     SLC_TCP3A_CFG_1         0x62E8
#define     SLC_TCP3B_CFG_0         0x62F4
#define     SLC_TCP3B_CFG_1         0x62F8
#define     SLC_TCP4_CFG_0          0x6134

#define     SLC_TCP6A_CFG_0         0x6140
#define     SLC_TCP6A_CFG_1         0x6144
#define     SLC_TCP6B_CFG_0         0x6148
#define     SLC_TCP6B_CFG_1         0x614C
#define     SLC_TCP7A_CFG_0         0x6150
#define     SLC_TCP7A_CFG_1         0x6154
#define     SLC_TCP7B_CFG_0         0x6158
#define     SLC_TCP7B_CFG_1         0x615C

#define     SLC_TCP9_CFG_0          0x6170
#define     SLC_TCP9_CFG_1          0x6174
#define     SLC_TCP10_CFG_0         0x6178
#define     SLC_DCP03_CFG_0         0x617C
#define     SLC_DCP6_CFG_0          0x6180
#define     SLC_DCP7_CFG_0          0x6184
#define     SLC_DCP8_CFG_0          0x6188
#define     SLC_DCP9_CFG_0          0x618C
#define     SLC_PMU_CNT0_L          0x6194
#define     SLC_PMU_CNT0_H          0x6198

#define     SLC_GID_COUNT           0x8010
#define     SLC02_INT_ST            0x8080
#define     SLC02_INT_INI           0x8084
#define     SLC02_INT_CLEAR         0x8088
#define     SLC02_INT_EN            0x8090
#define     SLC13_INT_ST            0x8080
#define     SLC13_INT_INI           0x8084
#define     SLC13_INT_CLEAR         0x8088
#define     SLC13_INT_EN            0x8090

#define     SLC_DFX_ERR_INF1        0x8454
#define     SLC_DFX_ERR_INF2        0x8458
#define     SLC_DFX_ECCERR          0x8460

#define     SLC_TCP5A_CFG_0         0x6300
#define     SLC_TCP5A_CFG_1         0x6304
#define     SLC_TCP5A_CFG_2         0x6308
#define     SLC_TCP5B_CFG_0         0x6310
#define     SLC_TCP5B_CFG_1         0x6314
#define     SLC_TCP5B_CFG_2         0x6318

#define     SLC_TCP8A_CFG_0         0x6320
#define     SLC_TCP8A_CFG_1         0x6324
#define     SLC_TCP8A_CFG_2         0x6328
#define     SLC_TCP8B_CFG_0         0x6330
#define     SLC_TCP8B_CFG_1         0x6334
#define     SLC_TCP8B_CFG_2         0x6338

#define LB_LINE_SHIFT               9

#define SLC_PMU_CTRL                0x8508

#define LB_PAGE_FLAG                0x1
#define FINISH_MASK                 0x1
#define CMO_EVENT_INT_CLR_FLAG      BIT(19)
#define CACHE_LINE                  128

#ifdef CONFIG_HISI_LB_WAY_16
#define SOC_FCM_FCM_M0_MID          0x30
#define SOC_FCM_FCM_M1_MID          0x30
#define MID_MASK                    0xf0
#else
#define MID_MASK                    0x7f
#endif

#define QUOTA_512K                  0x0400
#define QUOTA_1M                    0x0800
#define QUOTA_2M                    0x1000
#define QUOTA_3M                    0x1800
#define QUOTA_4M                    0x2000

#define LB_E_MAX                      77
#define LB_PMU_STATGO_SHIFT           1
#define lb_pmu_statgo_enable(mode_tp)                 \
	((0x10000 << ((mode_tp) * LB_PMU_STATGO_SHIFT)) |   \
	 (0x1 << ((mode_tp) * LB_PMU_STATGO_SHIFT)))

#define lb_pmu_statgo_disable(mdoe_tp)      \
	(0x10000 << ((mdoe_tp) * LB_PMU_STATGO_SHIFT))


#define GID_ADDR(addr, id) \
	((addr) + (id) * 0x100)

#define SLC_GID_ADDR(addr, slc_id, id) \
	((addr) + (slc_id) * 0x2000 + (id) * 0x4)

#endif
