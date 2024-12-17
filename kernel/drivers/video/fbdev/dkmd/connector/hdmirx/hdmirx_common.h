/* Copyright (c) 2022, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HDMIRX_COMMON_H
#define HDMIRX_COMMON_H

#include <linux/types.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include "hdmirx_log.h"
#include "hdmirx_reg_addr.h"
#include "hdmirx_cmd.h"

#define TRUE true
#define FALSE false

typedef enum hdmirx_color_space_en {
	HDMIRX_COLOR_SPACE_RGB,
	HDMIRX_COLOR_SPACE_YCBCR422,
	HDMIRX_COLOR_SPACE_YCBCR444,
	HDMIRX_COLOR_SPACE_YCBCR420,
	HDMIRX_COLOR_SPACE_MAX
} hdmirx_color_space;

typedef enum ext_hdmirx_input_width {
	HDMIRX_INPUT_WIDTH_24,  /* color depth is 8 bit per channel, 24 bit per pixel */
	HDMIRX_INPUT_WIDTH_30,  /* color depth is 10 bit per channel, 30 bit per pixel */
	HDMIRX_INPUT_WIDTH_36,  /* color depth is 12 bit per channel, 36 bit per pixel */
	HDMIRX_INPUT_WIDTH_48,  /* color depth is 16 bit per channel, 48 bit per pixel */
	HDMIRX_INPUT_WIDTH_MAX
} hdmirx_input_width;

typedef enum ext_hdmirx_input_type {
	HDMIRX_INPUT_OFF,
	HDMIRX_INPUT_14,
	HDMIRX_INPUT_20,
	HDMIRX_INPUT_FRL3L3G,
	HDMIRX_INPUT_FRL3L6G,
	HDMIRX_INPUT_FRL4L6G,
	HDMIRX_INPUT_FRL4L8G,
	HDMIRX_INPUT_FRL4L10G,
	HDMIRX_INPUT_FRL4L12G,
	HDMIRX_INPUT_MAX
} hdmirx_input_type;

typedef enum ext_hdmirx_frl_rate {
	HDMIRX_FRL_RATE_TMDS,
	HDMIRX_FRL_RATE_FRL3L3G,
	HDMIRX_FRL_RATE_FRL3L6G,
	HDMIRX_FRL_RATE_FRL4L6G,
	HDMIRX_FRL_RATE_FRL4L8G,
	HDMIRX_FRL_RATE_FRL4L10G,
	HDMIRX_FRL_RATE_FRL4L12G,
	HDMIRX_FRL_RATE_MAX
} hdmirx_frl_rate;

typedef enum hdmirx_oversample_en {
	HDMIRX_OVERSAMPLE_NONE,
	HDMIRX_OVERSAMPLE_2X,
	HDMIRX_OVERSAMPLE_3X,
	HDMIRX_OVERSAMPLE_4X,
	HDMIRX_OVERSAMPLE_5X,
	HDMIRX_OVERSAMPLE_6X,
	HDMIRX_OVERSAMPLE_7X,
	HDMIRX_OVERSAMPLE_8X,
	HDMIRX_OVERSAMPLE_9X,
	HDMIRX_OVERSAMPLE_10X,
	HDMIRX_OVERSAMPLE_MAX
} hdmirx_oversample;

typedef enum {
	HDMIRX_CLK_ZONE_TMDS14_12P5_25,
	HDMIRX_CLK_ZONE_TMDS14_25_42P5,
	HDMIRX_CLK_ZONE_TMDS14_42P5_75,
	HDMIRX_CLK_ZONE_TMDS14_75_150,
	HDMIRX_CLK_ZONE_TMDS14_150_250,
	HDMIRX_CLK_ZONE_TMDS14_250_340,
	HDMIRX_CLK_ZONE_TMDS20_85_100,
	HDMIRX_CLK_ZONE_TMDS20_100_150,
	HDMIRX_CLK_ZONE_FRL_3G,
	HDMIRX_CLK_ZONE_FRL_6G,
	HDMIRX_CLK_ZONE_FRL_8G,
	HDMIRX_CLK_ZONE_FRL_10G,
	HDMIRX_CLK_ZONE_FRL_12G,
	HDMIRX_CLK_ZONE_MAX
} hdmirx_clk_zone;

#define BIT0   0x01
#define BIT1   0x02
#define BIT2   0x04
#define BIT3   0x08
#define BIT4   0x10
#define BIT5   0x20
#define BIT6   0x40
#define BIT7   0x80
#define BIT8   0x100
#define BIT9   0x200
#define BIT27   0x8000000
#define BIT28   0x10000000
#define BIT29   0x20000000

#define BIT7_4 (BIT7 | BIT6 | BIT5 | BIT4)
#define BIT1_0 (BIT1 | BIT0)
#define BIT3_0 (BIT3 | BIT2 | BIT1 | BIT0)
#define BIT4_0 (BIT0 | BIT1 | BIT2 | BIT3 | BIT4)
#define BIT5_0 (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5)
#define BIT7_0 (BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)
#define BIT9_8 (BIT9 | BIT8)
#define BIT29_27 (BIT29 | BIT28 | BIT27)
#define BIT31_0 0xFFFFFFFF

#define void_unused(x)    ((void)(x))
#define outp32(addr, val)   writel(val, addr)
#define inp32(addr)         readl(addr)


void hdmirx_set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs);
void hdmirx_reg_read_block(const char __iomem *addr, uint32_t offset, uint32_t *dst, uint32_t num);
void hdmirx_ctrl_irq_clear(char __iomem *addr);
uint32_t hdmirx_reg_read(char __iomem *addr, uint32_t offset, uint32_t mask);
void hdmirx_reg_write(char __iomem *addr, uint32_t offset, uint32_t mask, uint32_t value);
#endif