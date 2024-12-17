// SPDX-License-Identifier: GPL-2.0
/*
 * ground_loop_compensate.c
 *
 * algorithm (ground loop temperature compensation) interface for fuel guage module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/hardware_ic/ground_loop_compensate.h>

#define HWLOG_TAG ground_loop_compensate
HWLOG_REGIST();

#define GROUND_LOOP_COMPENSATE_T_R_ARRAY_LEN    131
#define GROUND_LOOP_COMPENSATE_R_COMP_PRECISION 10

static int g_glc_t_r_table[][2] = {
	{ -400, 205200 }, { -390, 193800 }, { -380, 183100 }, { -370, 173100 },
	{ -360, 163600 }, { -350, 154800 }, { -340, 146500 }, { -330, 138700 },
	{ -320, 131300 }, { -310, 124400 }, { -300, 117900 }, { -290, 111800 },
	{ -280, 106000 }, { -270, 100600 }, { -260, 95510 }, { -250, 90690 },
	{ -240, 86150 }, { -230, 81860 }, { -220, 77810 }, { -210, 73990 },
	{ -200, 70370 }, { -190, 66960 }, { -180, 63740 }, { -170, 60690 },
	{ -160, 57800 }, { -150, 55070 }, { -140, 52490 }, { -130, 50040 },
	{ -120, 47720 }, { -110, 45520 }, { -100, 43440 }, { -90, 41460 },
	{ -80, 39590 }, { -70, 37810 }, { -60, 36130 }, { -50, 34530 },
	{ -40, 33000 }, { -30, 31560 }, { -20, 30190 }, { -10, 28880 },
	{ 0, 27640 }, { 10, 26460 }, { 20, 25330 }, { 30, 24260 }, { 40, 23240 },
	{ 50, 22270 }, { 60, 21350 }, { 70, 20470 }, { 80, 19630 }, { 90, 18830 },
	{ 100, 18060 }, { 110, 17340 }, { 120, 16640 }, { 130, 15980 },
	{ 140, 15350 }, { 150, 14740 }, { 160, 14170 }, { 170, 13620 },
	{ 180, 13090 }, { 190, 12590 }, { 200, 12110 }, { 210, 11650 },
	{ 220, 11210 }, { 230, 10790 }, { 240, 10380 }, { 250, 10000 },
	{ 260, 9632 }, { 270, 9279 }, { 280, 8942 }, { 290, 8619 }, { 300, 8309 },
	{ 310, 8012 }, { 320, 7727 }, { 330, 7454 }, { 340, 7192 }, { 350, 6941 },
	{ 360, 6700 }, { 370, 6468 }, { 380, 6246 }, { 390, 6033 }, { 400, 5828 },
	{ 410, 5631 }, { 420, 5441 }, { 430, 5259 }, { 440, 5084 }, { 450, 4916 },
	{ 460, 4754 }, { 470, 4598 }, { 480, 4448 }, { 490, 4304 }, { 500, 4165 },
	{ 510, 4031 }, { 520, 3902 }, { 530, 3778 }, { 540, 3658 }, { 550, 3543 },
	{ 560, 3432 }, { 570, 3325 }, { 580, 3222 }, { 590, 3123 }, { 600, 3027 },
	{ 610, 2934 }, { 620, 2845 }, { 630, 2759 }, { 640, 2676 }, { 650, 2595 },
	{ 660, 2518 }, { 670, 2443 }, { 680, 2371 }, { 690, 2301 }, { 700, 2233 },
	{ 710, 2168 }, { 720, 2105 }, { 730, 2045 }, { 740, 1986 }, { 750, 1929 },
	{ 760, 1874 }, { 770, 1821 }, { 780, 1770 }, { 790, 1720 }, { 800, 1672 },
	{ 810, 1625 }, { 820, 1580 }, { 830, 1536 }, { 840, 1493 }, { 850, 1451 },
	{ 860, 1411 }, { 870, 1372 }, { 880, 1334 }, { 890, 1297 }, { 900, 1261 },
};

int ground_loop_compensate_get_temp(struct glc_temp_comp_data *glc_data)
{
	int i_ntc;
	int r_ntc;
	int temp;

	if (glc_data == NULL)
		return -400; /* -400 is an invalid temp */

	i_ntc = DIV_ROUND_CLOSEST(((glc_data->vpullup - glc_data->vadc) * POWER_MA_PER_A),
		glc_data->rpullup); /* uA */
	r_ntc = DIV_ROUND_CLOSEST((glc_data->vadc * POWER_UV_PER_MV * GROUND_LOOP_COMPENSATE_R_COMP_PRECISION -
		glc_data->ibat * glc_data->rcomp), i_ntc * GROUND_LOOP_COMPENSATE_R_COMP_PRECISION); /* uv / uA = Ohm */
	temp = power_lookup_table_linear_trans_dichotomy(g_glc_t_r_table,
		GROUND_LOOP_COMPENSATE_T_R_ARRAY_LEN, r_ntc, 1);
	hwlog_info("after comp:temp=%d r_ntc=%d i_ntc=%d\n", temp, r_ntc, i_ntc);
	return temp;
}
