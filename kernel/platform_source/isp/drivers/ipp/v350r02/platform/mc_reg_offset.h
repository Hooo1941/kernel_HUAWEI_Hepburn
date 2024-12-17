/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __MC_REG_OFFSET_H__
#define __MC_REG_OFFSET_H__

/* mc Base address of Module's Register */

/******************************************************************************/
/*                     mc Registers' Definitions                         */
/******************************************************************************/

#define MC_EN_CFG_REG                0x0    /* Enable Config */
#define MC_IMU_CFG_REG               0x18   /* IMU config */
#define MC_MC_CFG_REG                0x1C   /* Basic Config */
#define MC_THRESHOLD_CFG_REG         0x20   /* Threshold Config */
#define MC_INLIER_NUMBER_REG         0x24   /* Inlier numbers for FW */
#define MC_KPT_SIZE_REG              0x28   /* key point size */
#define MC_STATIC_CFG1_REG           0x2C   /* Static config1 */
#define MC_STATIC_CFG2_REG           0x30   /* Static config2 */
#define MC_REF_PREFETCH_CFG_REG      0x34   /* Configure the prefetch for reference */
#define MC_CUR_PREFETCH_CFG_REG      0x38   /* Configure the prefetch for current */
#define MC_MATCH_POINTS_REG          0x3C   /* Matched points number */
#define MC_INDEX_CFG_0_REG           0x40   /* Index pairs from CMP */
#define MC_INDEX_CFG_1599_REG        0x193C /* Index pairs from CMP */
#define MC_INDEX_PAIRS_0_REG         0x1A00 /* Index pairs from CMP */
#define MC_COORDINATE_CFG_0_REG      0x1A80 /* Coordinate pairs from CMP */
#define MC_COORDINATE_PAIRS_0_REG    0x1B00 /* Provide the fetched or calculated inliers coordinate pairs for FW */
#define MC_DEBUG_0_REG               0x1C00 /* Debug 0 */
#define MC_DEBUG_1_REG               0x1C04 /* Debug 1 */
#define MC_EC_0_REG                  0x1C08 /* ec 0 */
#define MC_EC_1_REG                  0x1C0C /* ec 1 */

#endif // __MC_REG_OFFSET_H__
