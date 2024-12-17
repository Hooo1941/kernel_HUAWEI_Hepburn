/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description: DRA driver source code
 */
#ifndef __SC_GID_CTRL_HAL_H__
#define __SC_GID_CTRL_HAL_H__

#define START_ADDR(n) (0x0 + 0x10 * (n))
#define END_ADDR(n) (0x4 + 0x10 * (n))
#define GID_CFG(n) (0x8 + 0x10 * (n))
#define GID_REGION_EN(n) (0xc + 0x10 * (n))

#define GID_CTRL_ADDR_MASK_BIT 12

#endif
