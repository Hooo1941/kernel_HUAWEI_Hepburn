/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name     :  cvdr_ipp_reg_offset.h
 * Project line  :  Platform & Key Technologies Development
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1.0
 * Date          :
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1
 * History       :  xxx 2021/11/18 19:22:36 Create file
 */

#ifndef __CVDR_IPP_REG_OFFSET_H__
#define __CVDR_IPP_REG_OFFSET_H__

/* CVDR_IPP Base address of Module's Register */

/******************************************************************************/
/*                     CVDR_IPP Registers' Definitions                         */
/******************************************************************************/

#define CVDR_IPP_CVDR_IPP_CVDR_CFG_REG                   0x0   /* CVDR config register. */
#define CVDR_IPP_CVDR_IPP_CVDR_WR_QOS_CFG_REG            0x4   /* AXI Write QOS/Pressure configuration. */
#define CVDR_IPP_CVDR_IPP_CVDR_RD_QOS_CFG_REG            0x8   /* AXI Read QOS/Pressure configuration. */
#define CVDR_IPP_CVDR_IPP_FORCE_CLK_REG                  0xC   /* Force clock ON */
#define CVDR_IPP_CVDR_IPP_CVDR_DEBUG_EN_REG              0x10  /* CVDR debug register enable. */
#define CVDR_IPP_CVDR_IPP_CVDR_DEBUG_REG                 0x14  /* CVDR debug register. */
#define CVDR_IPP_CVDR_IPP_DEBUG_REG                      0x18  /* DEBUG information. */
#define CVDR_IPP_CVDR_IPP_OTHER_RO_REG                   0x1C  /* Spare Other RO. */
#define CVDR_IPP_CVDR_IPP_OTHER_RW_REG                   0x20  /* Spare Other RW. */
#define CVDR_IPP_CVDR_IPP_NR_RD_CFG_2_REG                0x40  /* Initiator read Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_RD_LIMITER_2_REG            0x44  /* NR RD Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_RD_DEBUG_2_REG              0x48  /* Non-Raster Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_RD_CFG_3_REG                0x4C  /* Initiator read Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_RD_LIMITER_3_REG            0x50  /* NR RD Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_RD_DEBUG_3_REG              0x54  /* Non-Raster Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_RD_CFG_4_REG                0x58  /* Initiator read Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_RD_LIMITER_4_REG            0x5C  /* NR RD Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_RD_DEBUG_4_REG              0x60  /* Non-Raster Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_RD_CFG_5_REG                0x64  /* Initiator read Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_RD_LIMITER_5_REG            0x68  /* NR RD Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_RD_DEBUG_5_REG              0x6C  /* Non-Raster Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_RD_CFG_6_REG                0x70  /* Initiator read Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_RD_LIMITER_6_REG            0x74  /* NR RD Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_RD_DEBUG_6_REG              0x78  /* Non-Raster Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_RD_CFG_7_REG                0x7C  /* Initiator read Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_RD_LIMITER_7_REG            0x80  /* NR RD Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_RD_DEBUG_7_REG              0x84  /* Non-Raster Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_RD_CFG_8_REG                0x88  /* Initiator read Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_RD_LIMITER_8_REG            0x8C  /* NR RD Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_RD_DEBUG_8_REG              0x90  /* Non-Raster Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_WR_CFG_6_REG                0xDC  /* Initiator write Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_WR_LIMITER_6_REG            0xE0  /* NR WR Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_WR_DEBUG_6_REG              0xE4  /* Non-Raster Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_WR_CFG_7_REG                0xE8  /* Initiator write Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_WR_LIMITER_7_REG            0xEC  /* NR WR Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_WR_DEBUG_7_REG              0xF0  /* Non-Raster Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_NR_WR_CFG_8_REG                0xF4  /* Initiator write Configuration. */
#define CVDR_IPP_CVDR_IPP_NR_WR_LIMITER_8_REG            0xF8  /* NR WR Access limiter. */
#define CVDR_IPP_CVDR_IPP_NR_WR_DEBUG_8_REG              0xFC  /* Non-Raster Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_0_REG                0x100 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_0_REG                0x104 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_0_REG                0x108 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_0_REG             0x10C /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_0_REG           0x110 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_0_REG             0x114 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_0_REG            0x118 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_0_REG              0x11C /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_1_REG                0x120 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_1_REG                0x124 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_1_REG                0x128 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_1_REG             0x12C /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_1_REG           0x130 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_1_REG             0x134 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_1_REG            0x138 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_1_REG              0x13C /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_6_REG                0x1C0 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_6_REG                0x1C4 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_6_REG                0x1C8 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_6_REG             0x1CC /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_6_REG           0x1D0 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_6_REG             0x1D4 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_6_REG            0x1D8 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_6_REG              0x1DC /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_7_REG                0x1E0 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_7_REG                0x1E4 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_7_REG                0x1E8 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_7_REG             0x1EC /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_7_REG           0x1F0 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_7_REG             0x1F4 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_7_REG            0x1F8 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_7_REG              0x1FC /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_9_REG                0x220 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_9_REG                0x224 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_9_REG                0x228 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_9_REG             0x22C /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_9_REG           0x230 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_9_REG             0x234 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_9_REG            0x238 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_9_REG              0x23C /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_10_REG               0x240 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_10_REG               0x244 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_10_REG               0x248 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_10_REG            0x24C /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_10_REG          0x250 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_10_REG            0x254 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_10_REG           0x258 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_10_REG             0x25C /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_12_REG               0x280 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_12_REG               0x284 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_12_REG               0x288 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_12_REG            0x28C /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_12_REG          0x290 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_12_REG            0x294 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_12_REG           0x298 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_12_REG             0x29C /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_13_REG               0x2A0 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_13_REG               0x2A4 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_13_REG               0x2A8 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_13_REG            0x2AC /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_13_REG          0x2B0 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_13_REG            0x2B4 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_13_REG           0x2B8 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_13_REG             0x2BC /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_14_REG               0x2C0 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_14_REG               0x2C4 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_14_REG               0x2C8 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_14_REG            0x2CC /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_14_REG          0x2D0 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_14_REG            0x2D4 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_14_REG           0x2D8 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_14_REG             0x2DC /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_15_REG               0x2E0 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_15_REG               0x2E4 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_15_REG               0x2E8 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_15_REG            0x2EC /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_15_REG          0x2F0 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_15_REG            0x2F4 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_15_REG           0x2F8 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_15_REG             0x2FC /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_RD_CFG_22_REG               0x3C0 /* Video port read Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LWG_22_REG               0x3C4 /* Line width generation.--> change to crop function */
#define CVDR_IPP_CVDR_IPP_VP_RD_FHG_22_REG               0x3C8 /* Frame height generation. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_FS_22_REG            0x3CC /* AXI frame start. */
#define CVDR_IPP_CVDR_IPP_VP_RD_AXI_LINE_22_REG          0x3D0 /* Line Wrap definition. */
#define CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_22_REG            0x3D4 /* prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_RD_LIMITER_22_REG           0x3D8 /* Video port read Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_RD_DEBUG_22_REG             0x3DC /* Video Port Read DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_0_REG             0x500 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_0_REG            0x504 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_0_REG  0x50C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_0_REG       0x514 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_0_REG    0x518 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_0_REG     0x51C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_3_REG             0x560 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_3_REG            0x564 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_3_REG  0x56C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_3_REG       0x574 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_3_REG    0x578 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_3_REG     0x57C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_12_REG            0x680 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_12_REG           0x684 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_12_REG 0x68C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_12_REG      0x694 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_12_REG   0x698 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_12_REG    0x69C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_13_REG            0x6A0 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_13_REG           0x6A4 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_13_REG 0x6AC /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_13_REG      0x6B4 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_13_REG   0x6B8 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_13_REG    0x6BC /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_14_REG            0x6C0 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_14_REG           0x6C4 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_14_REG 0x6CC /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_14_REG      0x6D4 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_14_REG   0x6D8 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_14_REG    0x6DC /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_15_REG            0x6E0 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_15_REG           0x6E4 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_15_REG 0x6EC /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_15_REG      0x6F4 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_15_REG   0x6F8 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_15_REG    0x6FC /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_16_REG            0x700 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_16_REG           0x704 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_16_REG 0x70C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_16_REG      0x714 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_16_REG   0x718 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_16_REG    0x71C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_17_REG            0x720 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_17_REG           0x724 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_17_REG 0x72C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_17_REG      0x734 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_17_REG   0x738 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_17_REG    0x73C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_18_REG            0x740 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_18_REG           0x744 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_18_REG 0x74C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_18_REG      0x754 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_18_REG   0x758 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_18_REG    0x75C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_19_REG            0x760 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_19_REG           0x764 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_19_REG 0x76C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_19_REG      0x774 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_19_REG   0x778 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_19_REG    0x77C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_22_REG            0x7C0 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_22_REG           0x7C4 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_22_REG 0x7CC /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_22_REG      0x7D4 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_22_REG   0x7D8 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_22_REG    0x7DC /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_23_REG            0x7E0 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_23_REG           0x7E4 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_23_REG 0x7EC /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_23_REG      0x7F4 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_23_REG   0x7F8 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_23_REG    0x7FC /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_24_REG            0x800 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_24_REG           0x804 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_24_REG 0x80C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_24_REG      0x814 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_24_REG   0x818 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_24_REG    0x81C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_25_REG            0x820 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_25_REG           0x824 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_25_REG 0x82C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_25_REG      0x834 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_25_REG   0x838 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_25_REG    0x83C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_26_REG            0x840 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_26_REG           0x844 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_26_REG 0x84C /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_26_REG      0x854 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_26_REG   0x858 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_26_REG    0x85C /* [] Video Port Write DEBUG information. */
#define CVDR_IPP_CVDR_IPP_VP_WR_IF_CFG_30_REG            0x8C0 /* : prefetch or reset or stall capability. */
#define CVDR_IPP_CVDR_IPP_VP_WR_LIMITER_30_REG           0x8C4 /* [] Video port write Access limiter. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_LINE_SID0_30_REG 0x8CC /* [] AXI line wrap and line stride. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_CFG_SID0_30_REG      0x8D4 /* [] Video port write Configuration. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_AXI_FS_SID0_30_REG   0x8D8 /* [] AXI address Frame start. */
#define CVDR_IPP_CVDR_IPP_VP_WR_SID_DEBUG_SID0_30_REG    0x8DC /* [] Video Port Write DEBUG information. */

#endif // __CVDR_IPP_REG_OFFSET_H__
