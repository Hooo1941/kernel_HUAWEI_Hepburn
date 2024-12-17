/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __MC_REG_OFFSET_FIELD_H__
#define __MC_REG_OFFSET_FIELD_H__

#define MC_MC_EN_LEN    1
#define MC_MC_EN_OFFSET 0

#define MC_IMU_STATIC_LEN    1
#define MC_IMU_STATIC_OFFSET 1
#define MC_IMU_EN_LEN        1
#define MC_IMU_EN_OFFSET     0

#define MC_SVD_ITERATIONS_LEN     5
#define MC_SVD_ITERATIONS_OFFSET  16
#define MC_MAX_ITERATIONS_LEN     12
#define MC_MAX_ITERATIONS_OFFSET  4
#define MC_STAT_EN_LEN            1
#define MC_STAT_EN_OFFSET         3
#define MC_PUSH_INLIERS_EN_LEN    1
#define MC_PUSH_INLIERS_EN_OFFSET 2
#define MC_CFG_MODE_LEN           1
#define MC_CFG_MODE_OFFSET        0

#define MC_INLIER_TH_LEN    20
#define MC_INLIER_TH_OFFSET 0

#define MC_INLIER_NUM_TH_LEN    11
#define MC_INLIER_NUM_TH_OFFSET 0

#define MC_KPT_SIZE_LEN    10
#define MC_KPT_SIZE_OFFSET 0

#define MC_INV_DS_RATIO_LEN    12
#define MC_INV_DS_RATIO_OFFSET 8
#define MC_STATIC_RATIO_LEN    6
#define MC_STATIC_RATIO_OFFSET 0

#define MC_MATCHED_NUM_TH_LEN    11
#define MC_MATCHED_NUM_TH_OFFSET 0

#define MC_REF_PREFETCH_ENABLE_LEN    1
#define MC_REF_PREFETCH_ENABLE_OFFSET 20
#define MC_REF_FIRST_32K_PAGE_LEN     19
#define MC_REF_FIRST_32K_PAGE_OFFSET  0

#define MC_CUR_PREFETCH_ENABLE_LEN    1
#define MC_CUR_PREFETCH_ENABLE_OFFSET 20
#define MC_CUR_FIRST_32K_PAGE_LEN     19
#define MC_CUR_FIRST_32K_PAGE_OFFSET  0

#define MC_MATCHED_KPTS_LEN    11
#define MC_MATCHED_KPTS_OFFSET 0

#define MC_CFG_REF_INDEX_0_LEN    12
#define MC_CFG_REF_INDEX_0_OFFSET 12
#define MC_CFG_CUR_INDEX_0_LEN    12
#define MC_CFG_CUR_INDEX_0_OFFSET 0

#define MC_CFG_REF_INDEX_1599_LEN    12
#define MC_CFG_REF_INDEX_1599_OFFSET 12
#define MC_CFG_CUR_INDEX_1599_LEN    12
#define MC_CFG_CUR_INDEX_1599_OFFSET 0

#define MC_REF_INDEX_LEN    12
#define MC_REF_INDEX_OFFSET 12
#define MC_CUR_INDEX_LEN    12
#define MC_CUR_INDEX_OFFSET 0

#define MC_CFG_COORDINATE_Y_LEN    16
#define MC_CFG_COORDINATE_Y_OFFSET 16
#define MC_CFG_COORDINATE_X_LEN    16
#define MC_CFG_COORDINATE_X_OFFSET 0

#define MC_COORDINATE_Y_LEN    16
#define MC_COORDINATE_Y_OFFSET 16
#define MC_COORDINATE_X_LEN    16
#define MC_COORDINATE_X_OFFSET 0

#define MC_DEBUG_0_LEN    32
#define MC_DEBUG_0_OFFSET 0

#define MC_DEBUG_1_LEN    32
#define MC_DEBUG_1_OFFSET 0

#define MC_EC_0_LEN    32
#define MC_EC_0_OFFSET 0

#define MC_EC_1_LEN    32
#define MC_EC_1_OFFSET 0

#endif // __MC_REG_OFFSET_FIELD_H__
