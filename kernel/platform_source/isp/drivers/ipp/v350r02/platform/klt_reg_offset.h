/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_mc.h
 * Description:
 *
 * Date         2021-11-27 16:28:10
 ********************************************************************/

#ifndef __KLT_REG_OFFSET_H__
#define __KLT_REG_OFFSET_H__

/* KLT Base address of Module's Register */

/******************************************************************************/
/*                     KLT Registers' Definitions                         */
/******************************************************************************/

#define KLT_DS_CONFIG_REG           0x0    /* Downscaler configuration */
#define KLT_KLT_START_REG           0x4    /* Start trigger */
#define KLT_FWD_CONFIG_REG          0x8    /* Forward Search parameters */
#define KLT_BWD_CONFIG_REG          0xC    /* Backward Search parameters */
#define KLT_ITER_CONFIG_1_REG       0x10   /* Iteration configuration */
#define KLT_ITER_CONFIG_2_REG       0x14   /* Iteration configuration */
#define KLT_EVAL_PARAM_REG          0x18   /* KP evaluation */
#define KLT_PREFETCH_CTRL_REG       0x1C   /* Prefetch control */
#define KLT_RD_PREV_BASE_ADDR_0_REG 0x20   /* Base addresses for downscaled frames ( previous frame) */
#define KLT_RD_PREV_BASE_ADDR_1_REG 0x24   /* Base addresses for downscaled frames ( previous frame) */
#define KLT_RD_PREV_BASE_ADDR_2_REG 0x28   /* Base addresses for downscaled frames ( previous frame) */
#define KLT_RD_PREV_BASE_ADDR_3_REG 0x2C   /* Base addresses for downscaled frames ( previous frame) */
#define KLT_RD_PREV_BASE_ADDR_4_REG 0x30   /* Base addresses for downscaled frames ( previous frame) */
#define KLT_RD_PREV_STRIDE_0_REG    0x40   /* Read prev line stride param */
#define KLT_RD_PREV_STRIDE_1_REG    0x44   /* Read prev line stride param */
#define KLT_RD_PREV_STRIDE_2_REG    0x48   /* Read prev line stride param */
#define KLT_RD_PREV_STRIDE_3_REG    0x4C   /* Read prev line stride param */
#define KLT_RD_PREV_STRIDE_4_REG    0x50   /* Read prev line stride param */
#define KLT_RD_NEXT_BASE_ADDR_0_REG 0x60   /* Base addresses for downscaled frames ( next frame) */
#define KLT_RD_NEXT_BASE_ADDR_1_REG 0x64   /* Base addresses for downscaled frames ( next frame) */
#define KLT_RD_NEXT_BASE_ADDR_2_REG 0x68   /* Base addresses for downscaled frames ( next frame) */
#define KLT_RD_NEXT_BASE_ADDR_3_REG 0x6C   /* Base addresses for downscaled frames ( next frame) */
#define KLT_RD_NEXT_BASE_ADDR_4_REG 0x70   /* Base addresses for downscaled frames ( next frame) */
#define KLT_RD_NEXT_STRIDE_0_REG    0x80   /* Read next line stride param */
#define KLT_RD_NEXT_STRIDE_1_REG    0x84   /* Read next line stride param */
#define KLT_RD_NEXT_STRIDE_2_REG    0x88   /* Read next line stride param */
#define KLT_RD_NEXT_STRIDE_3_REG    0x8C   /* Read next line stride param */
#define KLT_RD_NEXT_STRIDE_4_REG    0x90   /* Read next line stride param */
#define KLT_KP_PREV_BUFF_X_0_REG    0xA0   /* Buffer of KP previous x coordinates */
#define KLT_KP_PREV_BUFF_X_499_REG  0x1038 /* Buffer of KP previous x coordinates */
#define KLT_KP_PREV_BUFF_Y_0_REG    0xA4   /* Buffer of KP previous y coordinates */
#define KLT_KP_PREV_BUFF_Y_499_REG  0x103C /* Buffer of KP previous y coordinates */
#define KLT_KP_NEXT_BUFF_X_0_REG    0x1040 /* Buffer of KP next x coordinates */
#define KLT_KP_NEXT_BUFF_X_499_REG  0x1FD8 /* Buffer of KP next x coordinates */
#define KLT_KP_NEXT_BUFF_Y_0_REG    0x1044 /* Buffer of KP next y coordinates + status and conf */
#define KLT_KP_NEXT_BUFF_Y_499_REG  0x1FDC /* Buffer of KP next y coordinates + status and conf */

#endif // __KLT_REG_OFFSET_H__
