/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_mc.h
 * Description:
 *
 * Date         2021-11-27 16:28:10
 ********************************************************************/

#ifndef __KLT_REG_OFFSET_FIELD_H__
#define __KLT_REG_OFFSET_FIELD_H__

#define KLT_DS_NB_LAYER_LEN    3
#define KLT_DS_NB_LAYER_OFFSET 20
#define KLT_DS_HEIGHT_LEN      10
#define KLT_DS_HEIGHT_OFFSET   10
#define KLT_DS_WIDTH_LEN       10
#define KLT_DS_WIDTH_OFFSET    0

#define KLT_FRAME_HEIGHT_LEN    10
#define KLT_FRAME_HEIGHT_OFFSET 20
#define KLT_FRAME_WIDTH_LEN     10
#define KLT_FRAME_WIDTH_OFFSET  10
#define KLT_KP_NUMBERS_LEN      9
#define KLT_KP_NUMBERS_OFFSET   1
#define KLT_START_KLT_LEN       1
#define KLT_START_KLT_OFFSET    0

#define KLT_FWD_ITER_ACCURACY_LEN    10
#define KLT_FWD_ITER_ACCURACY_OFFSET 8
#define KLT_FWD_PATCH_SIZE_LEN       5
#define KLT_FWD_PATCH_SIZE_OFFSET    3
#define KLT_FWD_PYR_NUM_LEN          3
#define KLT_FWD_PYR_NUM_OFFSET       0

#define KLT_BWD_ITER_ACCURACY_LEN    10
#define KLT_BWD_ITER_ACCURACY_OFFSET 8
#define KLT_BWD_PATCH_SIZE_LEN       5
#define KLT_BWD_PATCH_SIZE_OFFSET    3
#define KLT_BWD_PYR_NUM_LEN          1
#define KLT_BWD_PYR_NUM_OFFSET       0

#define KLT_BWD_ITER_GUARANTEED_NR_0_LEN    4
#define KLT_BWD_ITER_GUARANTEED_NR_0_OFFSET 25
#define KLT_FWD_ITER_GUARANTEED_NR_0_LEN    5
#define KLT_FWD_ITER_GUARANTEED_NR_0_OFFSET 20
#define KLT_FWD_ITER_GUARANTEED_NR_1_LEN    5
#define KLT_FWD_ITER_GUARANTEED_NR_1_OFFSET 15
#define KLT_FWD_ITER_GUARANTEED_NR_2_LEN    5
#define KLT_FWD_ITER_GUARANTEED_NR_2_OFFSET 10
#define KLT_FWD_ITER_GUARANTEED_NR_3_LEN    5
#define KLT_FWD_ITER_GUARANTEED_NR_3_OFFSET 5
#define KLT_FWD_ITER_GUARANTEED_NR_4_LEN    5
#define KLT_FWD_ITER_GUARANTEED_NR_4_OFFSET 0

#define KLT_BWD_ITER_MAX_NR_0_LEN    4
#define KLT_BWD_ITER_MAX_NR_0_OFFSET 25
#define KLT_FWD_ITER_MAX_NR_0_LEN    5
#define KLT_FWD_ITER_MAX_NR_0_OFFSET 20
#define KLT_FWD_ITER_MAX_NR_1_LEN    5
#define KLT_FWD_ITER_MAX_NR_1_OFFSET 15
#define KLT_FWD_ITER_MAX_NR_2_LEN    5
#define KLT_FWD_ITER_MAX_NR_2_OFFSET 10
#define KLT_FWD_ITER_MAX_NR_3_LEN    5
#define KLT_FWD_ITER_MAX_NR_3_OFFSET 5
#define KLT_FWD_ITER_MAX_NR_4_LEN    5
#define KLT_FWD_ITER_MAX_NR_4_OFFSET 0

#define KLT_MOTION_EPSILON_LEN    9
#define KLT_MOTION_EPSILON_OFFSET 16
#define KLT_MIN_EIG_THRESH_LEN    9
#define KLT_MIN_EIG_THRESH_OFFSET 0

#define KLT_PREFETCH_DELAY_LEN    11
#define KLT_PREFETCH_DELAY_OFFSET 1
#define KLT_PREFETCH_EN_LEN       1
#define KLT_PREFETCH_EN_OFFSET    0

#define KLT_RD_PREV_BASE_ADDR_0_LEN    27
#define KLT_RD_PREV_BASE_ADDR_0_OFFSET 0

#define KLT_RD_PREV_BASE_ADDR_1_LEN    27
#define KLT_RD_PREV_BASE_ADDR_1_OFFSET 0

#define KLT_RD_PREV_BASE_ADDR_2_LEN    27
#define KLT_RD_PREV_BASE_ADDR_2_OFFSET 0

#define KLT_RD_PREV_BASE_ADDR_3_LEN    27
#define KLT_RD_PREV_BASE_ADDR_3_OFFSET 0

#define KLT_RD_PREV_BASE_ADDR_4_LEN    27
#define KLT_RD_PREV_BASE_ADDR_4_OFFSET 0

#define KLT_RD_PREV_LINE_STRIDECTRL_0_LEN    1
#define KLT_RD_PREV_LINE_STRIDECTRL_0_OFFSET 10
#define KLT_RD_PREV_LINE_STRIDE_0_LEN        10
#define KLT_RD_PREV_LINE_STRIDE_0_OFFSET     0

#define KLT_RD_PREV_LINE_STRIDECTRL_1_LEN    1
#define KLT_RD_PREV_LINE_STRIDECTRL_1_OFFSET 10
#define KLT_RD_PREV_LINE_STRIDE_1_LEN        10
#define KLT_RD_PREV_LINE_STRIDE_1_OFFSET     0

#define KLT_RD_PREV_LINE_STRIDECTRL_2_LEN    1
#define KLT_RD_PREV_LINE_STRIDECTRL_2_OFFSET 10
#define KLT_RD_PREV_LINE_STRIDE_2_LEN        10
#define KLT_RD_PREV_LINE_STRIDE_2_OFFSET     0

#define KLT_RD_PREV_LINE_STRIDECTRL_3_LEN    1
#define KLT_RD_PREV_LINE_STRIDECTRL_3_OFFSET 10
#define KLT_RD_PREV_LINE_STRIDE_3_LEN        10
#define KLT_RD_PREV_LINE_STRIDE_3_OFFSET     0

#define KLT_RD_PREV_LINE_STRIDECTRL_4_LEN    1
#define KLT_RD_PREV_LINE_STRIDECTRL_4_OFFSET 10
#define KLT_RD_PREV_LINE_STRIDE_4_LEN        10
#define KLT_RD_PREV_LINE_STRIDE_4_OFFSET     0

#define KLT_RD_NEXT_BASE_ADDR_0_LEN    27
#define KLT_RD_NEXT_BASE_ADDR_0_OFFSET 0

#define KLT_RD_NEXT_BASE_ADDR_1_LEN    27
#define KLT_RD_NEXT_BASE_ADDR_1_OFFSET 0

#define KLT_RD_NEXT_BASE_ADDR_2_LEN    27
#define KLT_RD_NEXT_BASE_ADDR_2_OFFSET 0

#define KLT_RD_NEXT_BASE_ADDR_3_LEN    27
#define KLT_RD_NEXT_BASE_ADDR_3_OFFSET 0

#define KLT_RD_NEXT_BASE_ADDR_4_LEN    27
#define KLT_RD_NEXT_BASE_ADDR_4_OFFSET 0

#define KLT_RD_NEXT_LINE_STRIDECTRL_0_LEN    1
#define KLT_RD_NEXT_LINE_STRIDECTRL_0_OFFSET 10
#define KLT_RD_NEXT_LINE_STRIDE_0_LEN        10
#define KLT_RD_NEXT_LINE_STRIDE_0_OFFSET     0

#define KLT_RD_NEXT_LINE_STRIDECTRL_1_LEN    1
#define KLT_RD_NEXT_LINE_STRIDECTRL_1_OFFSET 10
#define KLT_RD_NEXT_LINE_STRIDE_1_LEN        10
#define KLT_RD_NEXT_LINE_STRIDE_1_OFFSET     0

#define KLT_RD_NEXT_LINE_STRIDECTRL_2_LEN    1
#define KLT_RD_NEXT_LINE_STRIDECTRL_2_OFFSET 10
#define KLT_RD_NEXT_LINE_STRIDE_2_LEN        10
#define KLT_RD_NEXT_LINE_STRIDE_2_OFFSET     0

#define KLT_RD_NEXT_LINE_STRIDECTRL_3_LEN    1
#define KLT_RD_NEXT_LINE_STRIDECTRL_3_OFFSET 10
#define KLT_RD_NEXT_LINE_STRIDE_3_LEN        10
#define KLT_RD_NEXT_LINE_STRIDE_3_OFFSET     0

#define KLT_RD_NEXT_LINE_STRIDECTRL_4_LEN    1
#define KLT_RD_NEXT_LINE_STRIDECTRL_4_OFFSET 10
#define KLT_RD_NEXT_LINE_STRIDE_4_LEN        10
#define KLT_RD_NEXT_LINE_STRIDE_4_OFFSET     0

#define KLT_KP_PREV_X_0_LEN    18
#define KLT_KP_PREV_X_0_OFFSET 0

#define KLT_KP_PREV_X_499_LEN    18
#define KLT_KP_PREV_X_499_OFFSET 0

#define KLT_KP_PREV_Y_0_LEN    18
#define KLT_KP_PREV_Y_0_OFFSET 0

#define KLT_KP_PREV_Y_499_LEN    18
#define KLT_KP_PREV_Y_499_OFFSET 0

#define KLT_KP_NEXT_X_0_LEN    18
#define KLT_KP_NEXT_X_0_OFFSET 0

#define KLT_KP_NEXT_X_499_LEN    18
#define KLT_KP_NEXT_X_499_OFFSET 0

#define KLT_CONF_0_LEN         8
#define KLT_CONF_0_OFFSET      19
#define KLT_STATUS_0_LEN       1
#define KLT_STATUS_0_OFFSET    18
#define KLT_KP_NEXT_Y_0_LEN    18
#define KLT_KP_NEXT_Y_0_OFFSET 0

#define KLT_CONF_499_LEN         8
#define KLT_CONF_499_OFFSET      19
#define KLT_STATUS_499_LEN       1
#define KLT_STATUS_499_OFFSET    18
#define KLT_KP_NEXT_Y_499_LEN    18
#define KLT_KP_NEXT_Y_499_OFFSET 0

#endif // __KLT_REG_OFFSET_FIELD_H__
