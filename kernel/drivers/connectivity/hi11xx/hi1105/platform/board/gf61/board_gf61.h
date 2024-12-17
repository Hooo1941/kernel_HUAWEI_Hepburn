/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: board_gf61.c header file
 * Author: @CompanyNameTag
 */

#ifndef BOARD_GF61_H
#define BOARD_GF61_H

/* 其他头文件包含 */
#include "plat_type.h"
#include "hw_bfg_ps.h"

#define WLAN_GPIO_BASE_ADDR   0x40005000
#define BT_GPIO_BASE_ADDR     0x40006000
#define GT_GPIO_BASE_ADDR     0x40007000
#define SLE_GPIO_BASE_ADDR    0x40008000

#define GPIO_TEST_TIMES 2

// 当前使用EVB的GPIO配置，产品版适配完成后使用产品最终GPIO配置
#ifndef _PRE_BOARD_PRODUCT
#define GT_DEV2HOST_GPIO   18
#define GT_HOST2DEV_GPIO   24

#define WL_DEV2HOST_GPIO   12
#define WL_HOST2DEV_GPIO   13

#define B_DEV2HOST_GPIO    17
#define B_HOST2DEV_GPIO    0   // evb 没有此管脚

#define SLE_DEV2HOST_GPIO  0   // evb 没有此管脚
#define SLE_HOST2DEV_GPIO  0   // evb 没有此管脚
#else
#define GT_DEV2HOST_GPIO   8   // PCIE0_RST_N
#define GT_HOST2DEV_GPIO   6   // FEM_2P4G_DEDICATE_ELNA_CTL

#define WL_DEV2HOST_GPIO   5   // FEM_WL_TAS_CTL
#define WL_HOST2DEV_GPIO   7   // PCIE0_CLK_REQ_N

#define B_DEV2HOST_GPIO    2   // FEM_2P4G_CH4_BT_EN
#define B_HOST2DEV_GPIO    19  // B_UART_RX

#define SLE_HOST2DEV_GPIO  29  // G_UART_RX
#define SLE_DEV2HOST_GPIO  1   // FEM_2P4G_DUAL_RECEIVE_BT_EN
#endif

void board_info_init_gf61(void);
#endif
