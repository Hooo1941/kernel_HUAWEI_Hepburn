/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: provide interface for debug.
 * Create: 2017-02-28
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HJPEG_DEBUG_H__
#define __HJPEG_DEBUG_H__

#define DUMP_REGS 0
#define DUMP_ERR_DATA 0

#define POWER_CTRL_HW_INTF      0
#define POWER_CTRL_CFG_REGS     1
#define POWER_CTRL_INTERFACE    POWER_CTRL_HW_INTF

#if (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS)
int cfg_map_reg_base(void);
int cfg_unmap_reg_base(void);

int cfg_powerup_regs(void);
int cfg_powerdn_regs(void);
#endif /* (POWER_CTRL_INTERFACE == POWER_CTRL_CFG_REGS) */

#if DUMP_REGS
int dump_cvdr_reg(u32 chip_type, void __iomem *viraddr);
int dump_jpeg_reg(u32 chip_type, void __iomem *viraddr);
#endif /* DUMP_REGS */

#endif /* __HJPEG_DEBUG_H__ */
