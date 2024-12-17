/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __SLAM_COMPARE_REG_OFFSET_H__
#define __SLAM_COMPARE_REG_OFFSET_H__

/* COMPARE Base address of Module's Register */

/******************************************************************************/
/*                     COMPARE Registers' Definitions                         */
/******************************************************************************/

#define COMPARE_COMPARE_CFG_REG        0x0    /* Compare Config */
#define COMPARE_BLOCK_CFG_REG          0x8    /* BLOCK Config */
#define COMPARE_SEARCH_CFG_REG         0xC    /* Search */
#define COMPARE_STAT_CFG_REG           0x10   /* Statistic */
#define COMPARE_PREFETCH_CFG_REG       0x14   /* Configure the prefetch */
#define COMPARE_OFFSET_CFG_REG         0x18   /* Configure the center offset */
#define COMPARE_TOTAL_KPT_NUM_REG      0x1C   /* Total kpt number */
#define COMPARE_REF_KPT_NUMBER_0_REG   0x20   /* Reference feature number in each block */
#define COMPARE_REF_KPT_NUMBER_255_REG 0x41C  /* Reference feature number in each block */
#define COMPARE_CUR_KPT_NUMBER_0_REG   0x420  /* Current feature number in each block */
#define COMPARE_CUR_KPT_NUMBER_255_REG 0x81C  /* Current feature number in each block */
#define COMPARE_MATCH_POINTS_REG       0x8FC  /* Matched points number */
#define COMPARE_INDEX_0_REG            0x900  /* Matched Pairs */
#define COMPARE_INDEX_1599_REG         0x21FC /* Matched Pairs */
#define COMPARE_DISTANCE_0_REG         0x2200 /* Matched distance */
#define COMPARE_DISTANCE_1599_REG      0x3AFC /* Matched distance */
#define COMPARE_DEBUG_0_REG            0x3B00 /* Debug 0 */
#define COMPARE_DEBUG_1_REG            0x3B04 /* Debug 1 */
#define COMPARE_EC_0_REG               0x3B08 /* ec 0 */
#define COMPARE_EC_1_REG               0x3B0C /* ec 1 */

#endif // __SLAM_COMPARE_REG_OFFSET_H__
