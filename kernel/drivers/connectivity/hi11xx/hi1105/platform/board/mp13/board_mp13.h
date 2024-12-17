/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: board_mp13.c header file
 * Author: @CompanyNameTag
 */

#ifndef BOARD_MP13_H
#define BOARD_MP13_H

/* 其他头文件包含 */
#include "plat_type.h"
#include "hw_bfg_ps.h"
/* 宏定义 */
#define GPIO_DIRECTION_OUTPUT    0
#define GPIO_DIRECTION_INPUT     1

#define GPIO_LEVEL_CONFIG_REGADDR       0x0  /* GPIO管脚的电平值拉高或拉低寄存器 */
#define GPIO_INOUT_CONFIG_REGADDR       0x04 /* GPIO管脚的数据方向存器 */
#define GPIO_TYPE_CONFIG_REGADDR        0x30 /* GPIO管脚的模式寄存器:IO or INT */
#define GPIO_INT_POLARITY_REGADDR       0x3C /* GPIO中断极性寄存器 */
#define GPIO_INT_TYPE_REGADDR           0x38 /* GPIO中断触发类型寄存器:电平触发或边沿触发 */
#define GPIO_INT_CLEAR_REGADDR          0x4C /* GPIO清除中断寄存器，只对边沿触发的中断有效 */
#define GPIO_LEVEL_GET_REGADDR          0x50 /* GPIO管脚当前电平值寄存器 */
#define GPIO_INTERRUPT_DEBOUNCE_REGADDR 0x48 /* GPIO管脚是否使能去抖动 */

#define BFGX_SUBSYS_RST_DELAY   100
#define WIFI_SUBSYS_RST_DELAY   10

#define PROC_NAME_GPIO_WLAN_FLOWCTRL  "hi110x_wlan_flowctrl"

/* test ssi write bcpu code */
/* EXTERN VARIABLE */
#ifdef PLATFORM_DEBUG_ENABLE
extern int32_t g_device_monitor_enable;
#endif

/* 函数声明 */
#if (defined(CONFIG_PCIE_KIRIN_SLT_HI110X)|| defined(CONFIG_PCIE_KPORT_SLT_DEVICE)) && defined(CONFIG_HISI_DEBUG_FS)
int32_t hi1103_pcie_chip_rc_slt_register(void);
int32_t hi1103_pcie_chip_rc_slt_unregister(void);
#endif

void board_callback_init_mp13_power(void);
void board_info_init_mp13(void);
#endif
