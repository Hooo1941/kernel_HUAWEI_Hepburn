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

#ifndef HDMIRX_CMD_H
#define HDMIRX_CMD_H

extern uint32_t g_frl_kvco[4];

extern uint32_t g_ictrl_pd0[5];

extern uint32_t g_ictrl_pfd_code[3];

extern uint32_t g_kvco_ratio[3];

extern uint32_t g_rctrl[3];

extern uint32_t g_fix_code[4][8];

extern uint32_t  g_fix_code_42575[8];

extern uint32_t  g_fix_code_25425[8];

extern uint32_t  g_fix_code_75150[8];

extern uint32_t g_force_eq_code[9];

extern uint32_t g_auto_eq[6];

extern uint32_t g_isel[2];

extern uint32_t g_eq_bandwidth[9];

extern uint32_t g_auto_eq_init[9];

extern uint32_t g_coarse;

extern uint32_t g_dfe_cfg[5];

extern uint32_t g_ttap;

extern uint32_t g_force_dfe[3];

extern bool g_reset_vco;

extern uint32_t g_efuse_1;

extern bool g_reduce_swing;

extern bool g_auto_eq_ctl[5];

extern bool g_reset_vco_a;

extern bool g_hw_init_en;

extern bool g_pll_reset_1;

extern bool g_pll_para;

extern bool g_eq_para;

extern bool g_ddc_para;

void hdmirx_fix_code_set(void);

#endif