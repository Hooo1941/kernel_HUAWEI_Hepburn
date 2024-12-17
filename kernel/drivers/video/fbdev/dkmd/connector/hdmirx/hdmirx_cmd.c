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
#include <linux/module.h>
#include <linux/types.h>
#include "hdmirx_log.h"

uint g_frl_kvco[4] = {0x2, 0x2, 0x2, 0x2};
int g_frl_kvco_length = 0;
uint g_ictrl_pd0[5] = {0x3, 0x9, 0x8, 0xd, 0x0};
int g_ictrl_pd0_length = 0;
uint g_ictrl_pfd_code[3] = {0x8, 0x8, 0x8};
int g_ictrl_pfd_code_length = 0;
uint g_kvco_ratio[3] = {0x3, 0x1, 0x1};   // origin 3 1 0
int g_kvco_ratio_length = 0;
uint g_rctrl[3] = {0x2, 0x2, 0x0};
int g_rctrl_length = 0;
uint g_fix_code_25425[8] = {0x8, 0xa, 0xc, 0x6, 0x4, 0x2, 0x1, 0x0};
int g_fix_code_25425_length = 0;
uint g_fix_code_42575[8] = {0xa, 0xc, 0xc, 0x6, 0x4, 0x2, 0x1, 0x0};
int g_fix_code_42575_length = 0;
uint g_fix_code_75150[8] = {0xc, 0xa, 0x8, 0x6, 0x4, 0x2, 0x1, 0x0};
int g_fix_code_75150_length = 0;

uint g_fix_code[4][8] = { /* 4: 4 case 12.5-25-42.5-75; 8: 8 cfg 8 */
	/* 8 fix code for phy sel */
	{ 0x8, 0xa, 0xc, 0x6, 0x4, 0x2, 0x1, 0x0 }, /* HDMI1.4: 12.5M-25M */
	{ 0x8, 0xa, 0xc, 0x6, 0x4, 0x2, 0x1, 0x0 }, /* HDMI1.4: 25M-42.5M */
	{ 0xa, 0xc, 0xc, 0x6, 0x4, 0x2, 0x1, 0x0 }, /* HDMI1.4: 42.5M-75M */
	{ 0xc, 0xa, 0x8, 0x6, 0x4, 0x2, 0x1, 0x0 }, /* HDMI1.4: 75M-150M */
};

void hdmirx_fix_code_set(void)
{
	uint i;
	for (i = 0; i < 8; i++)
		g_fix_code[1][i] = g_fix_code_25425[i];

	for (i = 0; i < 8; i++)
		g_fix_code[2][i] = g_fix_code_42575[i];

	for (i = 0; i < 8; i++)
		g_fix_code[3][i] = g_fix_code_75150[i];
}

uint g_force_eq_code[9] = {
	0x8, 0xf, 0x12, 0x12, 0xf, 0xf, 0x17, 0x1f, 0x1f
};
int g_force_eq_code_length = 0;
uint g_auto_eq[6] = {
	0xf, 0xf, 0xf, 0x960, 0x960, 0x140
};
int g_auto_eq_length = 0;
uint g_isel[2] = {
	0x0, 0x1
};
int g_isel_length = 0;
uint g_eq_bandwidth[9] = {
	0x0, 0x3, 0x2, 0x7, 0x5, 0x6, 0x7, 0x6, 0xc
};
int g_eq_bandwidth_length = 0;
uint g_auto_eq_init[9] = {
	0x3f80, 0x2, 0xf, 0x3, 0x0, 0x4, 0xa, 0xa, 0x0
};
int g_auto_eq_init_length = 0;
uint g_coarse = 0x2d;
uint g_dfe_cfg[5] = {
	0xf, 0x3f, 0x3f, 0x3fc, 0x3
};
int g_dfe_cfg_length = 0;
uint g_ttap = 0x37;
uint g_force_dfe[3] = {
	0, 15, 20
};
int g_force_dfe_length = 0;
bool g_reset_vco = false;
uint g_efuse_1 = 0xff;
bool g_reduce_swing = true;
bool g_auto_eq_ctl[5] = {true, true, true, true, true};
int g_auto_eq_ctl_length = 0;
bool g_reset_vco_a = true;
bool g_hw_init_en = true;
bool g_pll_reset_1 = false;
bool g_eq_para = false;
bool g_ddc_para = false;
bool g_pll_para = false;

#ifdef CONFIG_DKMD_DEBUG_ENABLE
module_param_array_named(frl_kvco, g_frl_kvco, uint, &g_frl_kvco_length, 0640);
MODULE_PARM_DESC(frl_kvco, "frl_kvco");

module_param_array_named(ictrl_pd0, g_ictrl_pd0, uint, &g_ictrl_pd0_length, 0640);
MODULE_PARM_DESC(ictrl_pd0, "ictrl_pd0");

module_param_array_named(ictrl_pfd_code, g_ictrl_pfd_code, uint, &g_ictrl_pfd_code_length, 0640);
MODULE_PARM_DESC(ictrl_pfd_code, "ictrl_pfd_code");

module_param_array_named(kvco_ratio, g_kvco_ratio, uint, &g_kvco_ratio_length, 0640);
MODULE_PARM_DESC(kvco_ratio, "kvco_ratio");

module_param_array_named(rctrl, g_rctrl, uint, &g_rctrl_length, 0640);
MODULE_PARM_DESC(rctrl, "rctrl");

module_param_array_named(fix_code_25425, g_fix_code_25425, uint, &g_fix_code_25425_length, 0640);
MODULE_PARM_DESC(fix_code_25425, "fix_code_25425");

module_param_array_named(fix_code_42575, g_fix_code_42575, uint, &g_fix_code_42575_length, 0640);
MODULE_PARM_DESC(fix_code_42575, "fix_code_42575");

module_param_array_named(fix_code_75150, g_fix_code_75150, uint, &g_fix_code_75150_length, 0640);
MODULE_PARM_DESC(fix_code_75150, "fix_code_75150");

module_param_array_named(force_eq_code, g_force_eq_code, uint, &g_force_eq_code_length, 0640);
MODULE_PARM_DESC(force_eq_code, "force_eq_code");

module_param_array_named(auto_eq, g_auto_eq, uint, &g_auto_eq_length, 0640);
MODULE_PARM_DESC(auto_eq, "auto_eq");

module_param_array_named(isel, g_isel, uint, &g_isel_length, 0640);
MODULE_PARM_DESC(isel, "isel");

module_param_array_named(eq_bandwidth, g_eq_bandwidth, uint, &g_eq_bandwidth_length, 0640);
MODULE_PARM_DESC(eq_bandwidth, "eq_bandwidth");

module_param_array_named(auto_eq_init, g_auto_eq_init, uint, &g_auto_eq_init_length, 0640);
MODULE_PARM_DESC(auto_eq_init, "auto_eq_init");

module_param_named(coarse, g_coarse, int, 0640);
MODULE_PARM_DESC(coarse, "coarse");

module_param_array_named(dfe_cfg, g_dfe_cfg, uint, &g_dfe_cfg_length, 0640);
MODULE_PARM_DESC(dfe_cfg, "dfe_cfg");

module_param_named(ttap, g_ttap, int, 0640);
MODULE_PARM_DESC(ttap, "ttap");

module_param_array_named(force_dfe, g_force_dfe, uint, &g_force_dfe_length, 0640);
MODULE_PARM_DESC(force_dfe, "force_dfe");

module_param_named(reset_vco, g_reset_vco, bool, 0640);
MODULE_PARM_DESC(reset_vco, "reset_vco");

module_param_named(efuse_1, g_efuse_1, uint, 0640);
MODULE_PARM_DESC(efuse_1, "efuse_1");

module_param_named(reduce_swing, g_reduce_swing, bool, 0640);
MODULE_PARM_DESC(reduce_swing, "reduce_swing");

module_param_array_named(auto_eq_ctl, g_auto_eq_ctl, bool, &g_auto_eq_ctl_length, 0640);
MODULE_PARM_DESC(auto_eq_ctl, "auto_eq_ctl");

module_param_named(reset_vco_a, g_reset_vco_a, bool, 0640);
MODULE_PARM_DESC(reset_vco_a, "reset_vco_a");

module_param_named(hw_init_en, g_hw_init_en, bool, 0640);
MODULE_PARM_DESC(hw_init_en, "hw_init_en");

module_param_named(pll_reset_1, g_pll_reset_1, bool, 0640);
MODULE_PARM_DESC(pll_reset_1, "pll_reset_1");

module_param_named(eq_para, g_eq_para, bool, 0640);
MODULE_PARM_DESC(eq_para, "eq_para");

module_param_named(ddc_para, g_ddc_para, bool, 0640);
MODULE_PARM_DESC(ddc_para, "ddc_para");

module_param_named(pll_para, g_pll_para, bool, 0640);
MODULE_PARM_DESC(pll_para, "pll_para");
#endif