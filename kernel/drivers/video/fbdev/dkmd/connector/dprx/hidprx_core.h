/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __RX_CORE_H__
#define __RX_CORE_H__

#include "hidprx.h"

#define DPCD_WR_BYTE_LEN 4 /* DPCD w/r in 4 byte length */
#define EXTERN_DPCD_REPEAT_SCOPE 0x10
#define EXTERN_DPCD_OFFSET 0x2200
#define EDPRX_RATE_NUM 8
#define EDPRX_LINK_RATE_RBR_BLOCK0 0xA4
#define EDPRX_LINK_RATE_RBR_BLOCK1 0x1F
#define EDPRX_LINK_RATE_R216_BLOCK0 0x30
#define EDPRX_LINK_RATE_R216_BLOCK1 0x2A
#define EDPRX_LINK_RATE_R243_BLOCK0 0x76
#define EDPRX_LINK_RATE_R243_BLOCK1 0x2F
#define EDPRX_LINK_RATE_HBR_BLOCK0 0xBC
#define EDPRX_LINK_RATE_HBR_BLOCK1 0x34
#define EDPRX_LINK_RATE_R324_BLOCK0 0x48
#define EDPRX_LINK_RATE_R324_BLOCK1 0x3F
#define EDPRX_LINK_RATE_R432_BLOCK0 0x60
#define EDPRX_LINK_RATE_R432_BLOCK1 0x54
#define EDPRX_LINK_RATE_HBR2_BLOCK0 0x78
#define EDPRX_LINK_RATE_HBR2_BLOCK1 0x69
#define EDPRX_LINK_RATE_HBR3_BLOCK0 0x34
#define EDPRX_LINK_RATE_HBR3_BLOCK1 0x9E
uint8_t dprx_read_dpcd(struct dprx_ctrl *dprx, uint32_t addr);
void dprx_write_dpcd(struct dprx_ctrl *dprx, uint32_t addr, uint8_t val);

void dprx_core_on(struct dprx_ctrl *dprx);
void dprx_core_off(struct dprx_ctrl *dprx);
void dprx_phy_wakeup_phy(struct dprx_ctrl *dprx);

#endif