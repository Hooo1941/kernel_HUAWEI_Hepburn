/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description: arfeature_reg_offset.h
 *
 * Date         2021-12-29
 ********************************************************************/

#ifndef __AR_FEATURE_REG_OFFSET_H__
#define __AR_FEATURE_REG_OFFSET_H__

/******************************************************************************/
/*                     AR_FEATURE Registers' Definitions                         */
/******************************************************************************/

#define AR_FEATURE_TOP_CFG_REG                    0x0    /* Top Config */
#define AR_FEATURE_IMAGE_SIZE_REG                 0x4    /* Image size config */
#define AR_FEATURE_BLOCK_NUM_CFG_REG              0x8    /* BLOCK Number */
#define AR_FEATURE_BLOCK_SIZE_CFG_INV_REG         0xC    /* BLOCK Size_INV */
#define AR_FEATURE_DETECT_NUMBER_LIMIT_REG        0x10   /* number limit */
#define AR_FEATURE_SIGMA_COEF_REG                 0x14   /* Sigma values */
#define AR_FEATURE_GAUSS_ORG_REG                  0x18   /* Gauss Coefficients */
#define AR_FEATURE_GSBLUR_1ST_REG                 0x1C   /* Gauss Coefficients */
#define AR_FEATURE_GAUSS_2ND_REG                  0x20   /* Gauss Coefficients */
#define AR_FEATURE_DOG_EDGE_THRESHOLD_REG         0x24   /* dog edge threshold */
#define AR_FEATURE_DESCRIPTOR_THRESHOLD_REG       0x28   /* descriptor threshold */
#define AR_FEATURE_CVDR_RD_CFG_REG                0x2C   /* Video port read Configuration. */
#define AR_FEATURE_CVDR_RD_LWG_REG                0x30   /* Line width generation.--> change to crop function */
#define AR_FEATURE_CVDR_RD_FHG_REG                0x34   /* Frame height generation. */
#define AR_FEATURE_CVDR_RD_AXI_FS_REG             0x38   /* AXI frame start. */
#define AR_FEATURE_CVDR_RD_AXI_LINE_REG           0x3C   /* Line Wrap definition. */
#define AR_FEATURE_CVDR_RD_IF_CFG_REG             0x40   /* prefetch or reset or stall capability. */
#define AR_FEATURE_PRE_CVDR_RD_FHG_REG            0x44   /* Frame height generation. */
#define AR_FEATURE_CVDR_WR_CFG_REG                0x48   /* Video port write Configuration. */
#define AR_FEATURE_CVDR_WR_AXI_FS_REG             0x4C   /* AXI address Frame start. */
#define AR_FEATURE_CVDR_WR_AXI_LINE_REG           0x50   /* AXI line wrap and line stride. */
#define AR_FEATURE_CVDR_WR_IF_CFG_REG             0x54   /* prefetch or reset or stall capability. */
#define AR_FEATURE_PRE_CVDR_WR_AXI_FS_REG         0x58   /* AXI address Frame start for first. */
#define AR_FEATURE_KLT_PRE_BLOCK_SIZE_CFG_INV_REG 0x5C   /* KLT PRE BLOCK Size_INV */
#define AR_FEATURE_KLT_PRE_BLOCK_CFG_REG          0x60   /* KLT PRE BLOCK CFG */
#define AR_FEATURE_MFPDA_CFG_0_REG                0x64   /* MFPDA config */
#define AR_FEATURE_MFPDA_CFG_1_REG                0x68   /* MFPDA config */
#define AR_FEATURE_MFPDA_CFG_2_REG                0x6C   /* MFPDA config */
#define AR_FEATURE_MFPDA_CFG_3_REG                0x70   /* MFPDA config */
#define AR_FEATURE_MFPDA_CFG_4_REG                0x74   /* MFPDA config */
#define AR_FEATURE_MFPDA_CFG_5_REG                0x78   /* MFPDA config */
#define AR_FEATURE_BLOCK_MFPDA_CFG_0_REG          0x7C   /* BLOCK MFPDA config */
#define AR_FEATURE_BLOCK_MFPDA_CFG_1_REG          0x80   /* BLOCK MFPDA config */
#define AR_FEATURE_BLOCK_MFPDA_CFG_2_REG          0x84   /* BLOCK MFPDA config */
#define AR_FEATURE_BLOCK_MFPDA_CFG_3_REG          0x88   /* BLOCK MFPDA config */
#define AR_FEATURE_BLOCK_MFPDA_CFG_4_REG          0x8C   /* BLOCK MFPDA config */
#define AR_FEATURE_BLOCK_MFPDA_CFG_5_REG          0x90   /* BLOCK MFPDA config */
#define AR_FEATURE_ROI_WINDOW_START_CFG_0_REG     0x94   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_START_CFG_1_REG     0x98   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_START_CFG_2_REG     0x9C   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_START_CFG_3_REG     0xA0   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_START_CFG_4_REG     0xA4   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_START_CFG_5_REG     0xA8   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_START_CFG_6_REG     0xAC   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_START_CFG_7_REG     0xB0   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_END_CFG_0_REG       0xB4   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_END_CFG_1_REG       0xB8   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_END_CFG_2_REG       0xBC   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_END_CFG_3_REG       0xC0   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_END_CFG_4_REG       0xC4   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_END_CFG_5_REG       0xC8   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_END_CFG_6_REG       0xCC   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_END_CFG_7_REG       0xD0   /* ROI WINDOW START config */
#define AR_FEATURE_ROI_WINDOW_MODE_CFG_REG        0xD4   /* ROI WINDOW MODE config */
#define AR_FEATURE_KPT_LIMIT_0_REG                0x100  /* control the number of KPT */
#define AR_FEATURE_KPT_LIMIT_255_REG              0x4FC  /* control the number of KPT */
#define AR_FEATURE_KPT_NUMBER_0_REG               0x500  /* Feature number in each block */
#define AR_FEATURE_KPT_NUMBER_255_REG             0x8FC  /* Feature number in each block */
#define AR_FEATURE_GRIDSTAT_1_0_REG               0x900  /* grid stat 1 */
#define AR_FEATURE_GRIDSTAT_1_255_REG             0xCFC  /* grid stat 1 */
#define AR_FEATURE_GRIDSTAT_2_0_REG               0xD00  /* grid stat 2 */
#define AR_FEATURE_GRIDSTAT_2_255_REG             0x10FC /* grid stat 2 */
#define AR_FEATURE_TOTAL_KPT_NUM_REG              0x1100 /* Total kpt number */
#define AR_FEATURE_TOTAL_KLTPRE_KPT_NUM_REG       0x1104 /* KLT pre total kpt number */
#define AR_FEATURE_MFPDA_UPDATE_CFG_REG           0x1108 /* Threshold for dog score of next frame */
#define AR_FEATURE_DEBUG_0_REG                    0x1200 /* debug signal 0 */
#define AR_FEATURE_DEBUG_1_REG                    0x1204 /* debug signal 1 */
#define AR_FEATURE_DEBUG_2_REG                    0x1208 /* debug signal 2 */
#define AR_FEATURE_DEBUG_3_REG                    0x120C /* debug signal 3 */

#endif // __AR_FEATURE_REG_OFFSET_H__
