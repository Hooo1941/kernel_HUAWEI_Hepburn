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

#ifndef __RX_DBG_H__
#define __RX_DBG_H__
#include <linux/types.h>
#include "hidprx_reg.h"

bool is_asic_process(void);
bool is_open_ctrl_debug(void);
bool is_rx0_phy_switch(void);
bool is_need_aux_monitor(void);
bool is_switch_phy_para(void);
int dprx_debug_set_ctrl_params(struct dprx_ctrl *dprx);
void dprx_debug_set_phy_params(struct dprx_ctrl *dprx);
uint32_t dprx_get_phy_da_test_en_2(void);
uint32_t dprx_get_phy_reg_t_da_2(void);
uint32_t dprx_get_phy_reg_t_da_13(void);
#endif