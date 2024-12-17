/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: lcdkit parse spr function for lcd driver
 * Author: weiyuantian
 * Create: 2023-05-17
 */

#include "lcd_kit_utils.h"
#include "lcd_kit_parse.h"

#define LCD_MAX_SIZE 1548

uint32_t spr_lut_table_temp[LCD_MAX_SIZE] = {
		0,    256,   448,  544,  619,  684,  744,  797,  848,  893,  939,  979, 1019, 1056, 1093, 1128, 1162, 1194, 1226, 1256,
		1285, 1314, 1342, 1370, 1397, 1422, 1448, 1474, 1498, 1522, 1546, 1569, 1592, 1615, 1636, 1658, 1680, 1700, 1721, 1742,
		1761, 1782, 1802, 1821, 1840, 1858, 1878, 1896, 1914, 1932, 1950, 1968, 1985, 2002, 2019, 2036, 2053, 2069, 2086, 2102,
		2118, 2134, 2150, 2166, 2182, 2197, 2212, 2228, 2243, 2257, 2272, 2287, 2301, 2316, 2330, 2345, 2359, 2372, 2387, 2401,
		2414, 2428, 2441, 2455, 2468, 2482, 2495, 2508, 2521, 2534, 2547, 2560, 2573, 2586, 2598, 2610, 2623, 2635, 2648, 2660,
		2672, 2684, 2696, 2708, 2720, 2732, 2744, 2756, 2767, 2779, 2791, 2802, 2813, 2825, 2836, 2847, 2858, 2870, 2881, 2892,
		2903, 2914, 2925, 2936, 2947, 2957, 2968, 2979, 2989, 2999, 3010, 3020, 3031, 3041, 3052, 3062, 3072, 3082, 3092, 3103,
		3113, 3123, 3133, 3143, 3153, 3163, 3173, 3183, 3193, 3202, 3212, 3222, 3231, 3241, 3251, 3260, 3270, 3279, 3289, 3298,
		3308, 3317, 3327, 3336, 3345, 3354, 3364, 3373, 3382, 3391, 3400, 3409, 3418, 3427, 3437, 3446, 3454, 3463, 3472, 3481,
		3490, 3498, 3507, 3516, 3525, 3534, 3542, 3551, 3559, 3568, 3577, 3585, 3594, 3602, 3611, 3619, 3628, 3636, 3644, 3653,
		3661, 3669, 3678, 3686, 3694, 3703, 3711, 3719, 3727, 3735, 3743, 3752, 3760, 3768, 3776, 3784, 3792, 3800, 3808, 3816,
		3824, 3831, 3839, 3847, 3855, 3863, 3871, 3878, 3886, 3894, 3901, 3909, 3917, 3924, 3932, 3940, 3947, 3955, 3962, 3970,
		3977, 3985, 3992, 4000, 4008, 4015, 4022, 4030, 4037, 4045, 4052, 4059, 4067, 4074, 4081, 4089, 4096, 0,

		0,    256,   448,  544,  619,  684,  744,  797,  848,  893,  939,  979, 1019, 1056, 1093, 1128, 1162, 1194, 1226, 1256,
		1285, 1314, 1342, 1370, 1397, 1422, 1448, 1474, 1498, 1522, 1546, 1569, 1592, 1615, 1636, 1658, 1680, 1700, 1721, 1742,
		1761, 1782, 1802, 1821, 1840, 1858, 1878, 1896, 1914, 1932, 1950, 1968, 1985, 2002, 2019, 2036, 2053, 2069, 2086, 2102,
		2118, 2134, 2150, 2166, 2182, 2197, 2212, 2228, 2243, 2257, 2272, 2287, 2301, 2316, 2330, 2345, 2359, 2372, 2387, 2401,
		2414, 2428, 2441, 2455, 2468, 2482, 2495, 2508, 2521, 2534, 2547, 2560, 2573, 2586, 2598, 2610, 2623, 2635, 2648, 2660,
		2672, 2684, 2696, 2708, 2720, 2732, 2744, 2756, 2767, 2779, 2791, 2802, 2813, 2825, 2836, 2847, 2858, 2870, 2881, 2892,
		2903, 2914, 2925, 2936, 2947, 2957, 2968, 2979, 2989, 2999, 3010, 3020, 3031, 3041, 3052, 3062, 3072, 3082, 3092, 3103,
		3113, 3123, 3133, 3143, 3153, 3163, 3173, 3183, 3193, 3202, 3212, 3222, 3231, 3241, 3251, 3260, 3270, 3279, 3289, 3298,
		3308, 3317, 3327, 3336, 3345, 3354, 3364, 3373, 3382, 3391, 3400, 3409, 3418, 3427, 3437, 3446, 3454, 3463, 3472, 3481,
		3490, 3498, 3507, 3516, 3525, 3534, 3542, 3551, 3559, 3568, 3577, 3585, 3594, 3602, 3611, 3619, 3628, 3636, 3644, 3653,
		3661, 3669, 3678, 3686, 3694, 3703, 3711, 3719, 3727, 3735, 3743, 3752, 3760, 3768, 3776, 3784, 3792, 3800, 3808, 3816,
		3824, 3831, 3839, 3847, 3855, 3863, 3871, 3878, 3886, 3894, 3901, 3909, 3917, 3924, 3932, 3940, 3947, 3955, 3962, 3970,
		3977, 3985, 3992, 4000, 4008, 4015, 4022, 4030, 4037, 4045, 4052, 4059, 4067, 4074, 4081, 4089, 4096, 0,

		0,    256,   448,  544,  619,  684,  744,  797,  848,  893,  939,  979, 1019, 1056, 1093, 1128, 1162, 1194, 1226, 1256,
		1285, 1314, 1342, 1370, 1397, 1422, 1448, 1474, 1498, 1522, 1546, 1569, 1592, 1615, 1636, 1658, 1680, 1700, 1721, 1742,
		1761, 1782, 1802, 1821, 1840, 1858, 1878, 1896, 1914, 1932, 1950, 1968, 1985, 2002, 2019, 2036, 2053, 2069, 2086, 2102,
		2118, 2134, 2150, 2166, 2182, 2197, 2212, 2228, 2243, 2257, 2272, 2287, 2301, 2316, 2330, 2345, 2359, 2372, 2387, 2401,
		2414, 2428, 2441, 2455, 2468, 2482, 2495, 2508, 2521, 2534, 2547, 2560, 2573, 2586, 2598, 2610, 2623, 2635, 2648, 2660,
		2672, 2684, 2696, 2708, 2720, 2732, 2744, 2756, 2767, 2779, 2791, 2802, 2813, 2825, 2836, 2847, 2858, 2870, 2881, 2892,
		2903, 2914, 2925, 2936, 2947, 2957, 2968, 2979, 2989, 2999, 3010, 3020, 3031, 3041, 3052, 3062, 3072, 3082, 3092, 3103,
		3113, 3123, 3133, 3143, 3153, 3163, 3173, 3183, 3193, 3202, 3212, 3222, 3231, 3241, 3251, 3260, 3270, 3279, 3289, 3298,
		3308, 3317, 3327, 3336, 3345, 3354, 3364, 3373, 3382, 3391, 3400, 3409, 3418, 3427, 3437, 3446, 3454, 3463, 3472, 3481,
		3490, 3498, 3507, 3516, 3525, 3534, 3542, 3551, 3559, 3568, 3577, 3585, 3594, 3602, 3611, 3619, 3628, 3636, 3644, 3653,
		3661, 3669, 3678, 3686, 3694, 3703, 3711, 3719, 3727, 3735, 3743, 3752, 3760, 3768, 3776, 3784, 3792, 3800, 3808, 3816,
		3824, 3831, 3839, 3847, 3855, 3863, 3871, 3878, 3886, 3894, 3901, 3909, 3917, 3924, 3932, 3940, 3947, 3955, 3962, 3970,
		3977, 3985, 3992, 4000, 4008, 4015, 4022, 4030, 4037, 4045, 4052, 4059, 4067, 4074, 4081, 4089, 4096, 0,

		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20,
		21, 23, 24, 25, 27, 28, 29, 30, 32, 34, 37, 39, 42, 45, 48, 51, 55, 58, 62, 65,
		69, 73, 77, 81, 85, 89, 94, 98, 103, 108, 113, 118, 123, 128, 133, 139, 145, 150, 156, 162,
		168, 175, 181, 187, 194, 201, 208, 215, 222, 229, 236, 244, 251, 259, 267, 275, 283, 291, 300, 308,
		317, 326, 335, 344, 353, 362, 372, 381, 391, 401, 411, 421, 431, 441, 452, 463, 473, 484, 495, 506,
		518, 529, 541, 553, 564, 576, 589, 601, 613, 626, 639, 651, 664, 677, 691, 704, 718, 731, 745, 759,
		773, 788, 802, 816, 831, 846, 861, 876, 891, 907, 922, 938, 954, 970, 986, 1002, 1018, 1035, 1052, 1068,
		1085, 1103, 1120, 1137, 1155, 1173, 1190, 1208, 1227, 1245, 1263, 1282, 1301, 1320, 1339, 1358, 1377, 1397, 1416, 1436,
		1456, 1476, 1496, 1517, 1537, 1558, 1579, 1600, 1621, 1642, 1664, 1685, 1707, 1729, 1751, 1773, 1796, 1818, 1841, 1864,
		1887, 1910, 1933, 1957, 1980, 2004, 2028, 2053, 2077, 2102, 2126, 2151, 2176, 2201, 2226, 2251, 2277, 2302, 2328, 2354,
		2380, 2406, 2433, 2459, 2486, 2513, 2540, 2567, 2594, 2622, 2650, 2677, 2705, 2734, 2762, 2790, 2819, 2848, 2877, 2906,
		2935, 2964, 2994, 3024, 3054, 3084, 3114, 3144, 3175, 3206, 3236, 3267, 3299, 3330, 3361, 3393, 3425, 3457, 3489, 3521,
		3554, 3587, 3619, 3652, 3686, 3719, 3752, 3786, 3820, 3854, 3888, 3922, 3957, 3991, 4026, 4061, 4096, 0,

		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20,
		21, 23, 24, 25, 27, 28, 29, 30, 32, 34, 37, 39, 42, 45, 48, 51, 55, 58, 62, 65,
		69, 73, 77, 81, 85, 89, 94, 98, 103, 108, 113, 118, 123, 128, 133, 139, 145, 150, 156, 162,
		168, 175, 181, 187, 194, 201, 208, 215, 222, 229, 236, 244, 251, 259, 267, 275, 283, 291, 300, 308,
		317, 326, 335, 344, 353, 362, 372, 381, 391, 401, 411, 421, 431, 441, 452, 463, 473, 484, 495, 506,
		518, 529, 541, 553, 564, 576, 589, 601, 613, 626, 639, 651, 664, 677, 691, 704, 718, 731, 745, 759,
		773, 788, 802, 816, 831, 846, 861, 876, 891, 907, 922, 938, 954, 970, 986, 1002, 1018, 1035, 1052, 1068,
		1085, 1103, 1120, 1137, 1155, 1173, 1190, 1208, 1227, 1245, 1263, 1282, 1301, 1320, 1339, 1358, 1377, 1397, 1416, 1436,
		1456, 1476, 1496, 1517, 1537, 1558, 1579, 1600, 1621, 1642, 1664, 1685, 1707, 1729, 1751, 1773, 1796, 1818, 1841, 1864,
		1887, 1910, 1933, 1957, 1980, 2004, 2028, 2053, 2077, 2102, 2126, 2151, 2176, 2201, 2226, 2251, 2277, 2302, 2328, 2354,
		2380, 2406, 2433, 2459, 2486, 2513, 2540, 2567, 2594, 2622, 2650, 2677, 2705, 2734, 2762, 2790, 2819, 2848, 2877, 2906,
		2935, 2964, 2994, 3024, 3054, 3084, 3114, 3144, 3175, 3206, 3236, 3267, 3299, 3330, 3361, 3393, 3425, 3457, 3489, 3521,
		3554, 3587, 3619, 3652, 3686, 3719, 3752, 3786, 3820, 3854, 3888, 3922, 3957, 3991, 4026, 4061, 4096, 0,

		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20,
		21, 23, 24, 25, 27, 28, 29, 30, 32, 34, 37, 39, 42, 45, 48, 51, 55, 58, 62, 65,
		69, 73, 77, 81, 85, 89, 94, 98, 103, 108, 113, 118, 123, 128, 133, 139, 145, 150, 156, 162,
		168, 175, 181, 187, 194, 201, 208, 215, 222, 229, 236, 244, 251, 259, 267, 275, 283, 291, 300, 308,
		317, 326, 335, 344, 353, 362, 372, 381, 391, 401, 411, 421, 431, 441, 452, 463, 473, 484, 495, 506,
		518, 529, 541, 553, 564, 576, 589, 601, 613, 626, 639, 651, 664, 677, 691, 704, 718, 731, 745, 759,
		773, 788, 802, 816, 831, 846, 861, 876, 891, 907, 922, 938, 954, 970, 986, 1002, 1018, 1035, 1052, 1068,
		1085, 1103, 1120, 1137, 1155, 1173, 1190, 1208, 1227, 1245, 1263, 1282, 1301, 1320, 1339, 1358, 1377, 1397, 1416, 1436,
		1456, 1476, 1496, 1517, 1537, 1558, 1579, 1600, 1621, 1642, 1664, 1685, 1707, 1729, 1751, 1773, 1796, 1818, 1841, 1864,
		1887, 1910, 1933, 1957, 1980, 2004, 2028, 2053, 2077, 2102, 2126, 2151, 2176, 2201, 2226, 2251, 2277, 2302, 2328, 2354,
		2380, 2406, 2433, 2459, 2486, 2513, 2540, 2567, 2594, 2622, 2650, 2677, 2705, 2734, 2762, 2790, 2819, 2848, 2877, 2906,
		2935, 2964, 2994, 3024, 3054, 3084, 3114, 3144, 3175, 3206, 3236, 3267, 3299, 3330, 3361, 3393, 3425, 3457, 3489, 3521,
		3554, 3587, 3619, 3652, 3686, 3719, 3752, 3786, 3820, 3854, 3888, 3922, 3957, 3991, 4026, 4061, 4096, 0,
};

static void lcd_kit_spr_border_rlr_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-r-borderlr-value",
		&pinfo->spr.spr_r_borderlr.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-g-borderlr-value",
		&pinfo->spr.spr_g_borderlr.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-b-borderlr-value",
		&pinfo->spr.spr_b_borderlr.value, 0);
}

static void lcd_kit_spr_border_rlb_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-r-bordertb-value",
		&pinfo->spr.spr_r_bordertb.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-g-bordertb-value",
		&pinfo->spr.spr_g_bordertb.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-b-bordertb-value",
		&pinfo->spr.spr_b_bordertb.value, 0);
}

static void lcd_kit_spr_core_border_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_spr_border_rlr_set(panel_id, np, pinfo);
	lcd_kit_spr_border_rlb_set(panel_id, np, pinfo);

	lcd_kit_parse_u32(np, "lcd-kit,spr-pixgain-reg-value",
		&pinfo->spr.spr_pixgain_reg.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-pixgain-reg1-value",
		&pinfo->spr.spr_pixgain_reg1.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-P0-border-value",
		&pinfo->spr.spr_border_p0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-P1-border-value",
		&pinfo->spr.spr_border_p1.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-P2-border-value",
		&pinfo->spr.spr_border_p2.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-P3-border-value",
		&pinfo->spr.spr_border_p3.value, 0);
}

static void lcd_kit_spr_larea_border_rgb_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-larea-border-r-value",
		&pinfo->spr.spr_larea_border_r.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-larea-border-g-value",
		&pinfo->spr.spr_larea_border_g.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-larea-border-b-value",
		&pinfo->spr.spr_larea_border_b.value, 0);
}

static void lcd_kit_spr_core_larea_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-larea-start-value",
		&pinfo->spr.spr_larea_start.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-larea-end-value",
		&pinfo->spr.spr_larea_end.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-larea-offset-value",
		&pinfo->spr.spr_larea_offset.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-larea-gain-value",
		&pinfo->spr.spr_larea_gain.value, 0);

	lcd_kit_spr_larea_border_rgb_set(panel_id, np, pinfo);
}

static void lcd_kit_spr_coeffs_b_v0h0_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h0-b0-value",
		&pinfo->spr.spr_coeff_v0h0_b0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h0-b1-value",
		&pinfo->spr.spr_coeff_v0h0_b1.value, 0);
}

static void lcd_kit_spr_coeffs_b_v0h1_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h1-b0-value",
		&pinfo->spr.spr_coeff_v0h1_b0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h1-b1-value",
		&pinfo->spr.spr_coeff_v0h1_b1.value, 0);
}

static void lcd_kit_spr_coeffs_b_v1h0_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h0-b0-value",
		&pinfo->spr.spr_coeff_v1h0_b0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h0-b1-value",
		&pinfo->spr.spr_coeff_v1h0_b1.value, 0);
}

static void lcd_kit_spr_coeffs_b_v1h1_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h1-b0-value",
		&pinfo->spr.spr_coeff_v1h1_b0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h1-b1-value",
		&pinfo->spr.spr_coeff_v1h1_b1.value, 0);
}

static void lcd_kit_spr_core_coeffs_b_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_spr_coeffs_b_v0h0_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_b_v0h1_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_b_v1h0_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_b_v1h1_set(panel_id, np, pinfo);
}

static void lcd_kit_spr_coeffs_r_v0h0_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h0-r0.value",
		&pinfo->spr.spr_coeff_v0h0_r0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h0-r1-value",
		&pinfo->spr.spr_coeff_v0h0_r1.value, 0);
}

static void lcd_kit_spr_coeffs_r_v0h1_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h1-r0-value",
		&pinfo->spr.spr_coeff_v0h1_r0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h1-r1-value",
		&pinfo->spr.spr_coeff_v0h1_r1.value, 0);
}

static void lcd_kit_spr_coeffs_r_v1h0_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h0-r0-value",
		&pinfo->spr.spr_coeff_v1h0_r0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h0-r1-value",
		&pinfo->spr.spr_coeff_v1h0_r1.value, 0);
}

static void lcd_kit_spr_coeffs_r_v1h1_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h1-r0-value",
		&pinfo->spr.spr_coeff_v1h1_r0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h1-r1-value",
		&pinfo->spr.spr_coeff_v1h1_r1.value, 0);
}

static void lcd_kit_spr_core_coeffs_r_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_spr_coeffs_r_v0h0_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_r_v0h1_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_r_v1h0_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_r_v1h1_set(panel_id, np, pinfo);
}

static void lcd_kit_spr_coeffs_g_v0h0_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h0-g0-value",
		&pinfo->spr.spr_coeff_v0h0_g0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h0-g1-value",
		&pinfo->spr.spr_coeff_v0h0_g1.value, 0);
}

static void lcd_kit_spr_coeffs_g_v0h1_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h1-g0-value",
		&pinfo->spr.spr_coeff_v0h1_g0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v0h1-g1-value",
		&pinfo->spr.spr_coeff_v0h1_g1.value, 0);
}

static void lcd_kit_spr_coeffs_g_v1h0_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h0-g0-value",
		&pinfo->spr.spr_coeff_v1h0_g0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h0-g1-value",
		&pinfo->spr.spr_coeff_v1h0_g1.value, 0);
}

static void lcd_kit_spr_coeffs_g_v1h1_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h1-g0-value",
		&pinfo->spr.spr_coeff_v1h1_g0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-coeff-v1h1-g1-value",
		&pinfo->spr.spr_coeff_v1h1_g1.value, 0);
}

static void lcd_kit_spr_core_coeffs_g_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_spr_coeffs_g_v0h0_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_g_v0h1_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_g_v1h0_set(panel_id, np, pinfo);
	lcd_kit_spr_coeffs_g_v1h1_set(panel_id, np, pinfo);
}

static void lcd_kit_spr_core_edgestr_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-edgestr-r-value",
		&pinfo->spr.spr_edgestr_r.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-edgestr-g-value",
		&pinfo->spr.spr_edgestr_g.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-edgestr-b-value",
		&pinfo->spr.spr_edgestr_b.value, 0);
}

static void lcd_kit_spr_core_dir_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-dir-min-value",
		&pinfo->spr.spr_dir_min.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-dir-max-value",
		&pinfo->spr.spr_dir_max.value, 0);
}

static void lcd_kit_spr_core_blend_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-blend-value",
		&pinfo->spr.spr_blend.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-weight-value",
		&pinfo->spr.spr_weight.value, 0);
}

static void lcd_kit_spr_diffdirgain_r_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-diff-r0-value",
		&pinfo->spr.spr_diff_r0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-diff-r1-value",
		&pinfo->spr.spr_diff_r1.value, 0);
}

static void lcd_kit_spr_diffdirgain_g_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-diff-g0-value",
		&pinfo->spr.spr_diff_g0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-diff-g1-value",
		&pinfo->spr.spr_diff_g1.value, 0);
}

static void lcd_kit_spr_diffdirgain_b_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-diff-b0-value",
		&pinfo->spr.spr_diff_b0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-diff-b1-value",
		&pinfo->spr.spr_diff_b1.value, 0);
}

static void lcd_kit_spr_core_diffdirgain_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_spr_diffdirgain_r_set(panel_id, np, pinfo);
	lcd_kit_spr_diffdirgain_g_set(panel_id, np, pinfo);
	lcd_kit_spr_diffdirgain_b_set(panel_id, np, pinfo);
}

static void lcd_kit_spr_core_horz_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-horzgradblend-value",
		&pinfo->spr.spr_horzgradblend.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-horzbdbld-value",
		&pinfo->spr.spr_horzbdbld.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-horzbdweight-value",
		&pinfo->spr.spr_horzbdweight.value, 0);
}

static void lcd_kit_spr_vert_gain_rgb_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-vertbd-gain-r-value",
		&pinfo->spr.spr_vertbd_gain_r.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-vertbd-gain-g-value",
		&pinfo->spr.spr_vertbd_gain_g.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-vertbd-gain-b-value",
		&pinfo->spr.spr_vertbd_gain_b.value, 0);
}

static void lcd_kit_spr_core_vert_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-vertbdbld-value",
		&pinfo->spr.spr_vertbdbld.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-vertbdweight-value",
		&pinfo->spr.spr_vertbdweight.value, 0);

	lcd_kit_spr_vert_gain_rgb_set(panel_id, np, pinfo);
}

static void lcd_kit_spr_txip_coef_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-txip-coef0-value",
		&pinfo->spr.txip_coef0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-txip-coef1-value",
		&pinfo->spr.txip_coef1.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-txip-coef2-value",
		&pinfo->spr.txip_coef2.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-txip-coef3-value",
		&pinfo->spr.txip_coef3.value, 0);
}

static void lcd_kit_spr_txip_offset_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-txip-offset0-value",
		&pinfo->spr.txip_offset0.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-txip-offset1-value",
		&pinfo->spr.txip_offset1.value, 0);
}

static void lcd_kit_spr_txip_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-txip-ctrl-value",
		&pinfo->spr.txip_ctrl.value, 0);

	pinfo->spr.txip_size.value = 0;
	pinfo->spr.txip_size.reg.txip_height = pinfo->yres - 1;
	pinfo->spr.txip_size.reg.txip_width = pinfo->xres - 1;

	lcd_kit_spr_txip_coef_set(panel_id, np, pinfo);
	lcd_kit_spr_txip_offset_set(panel_id, np, pinfo);

	lcd_kit_parse_u32(np, "lcd-kit,spr-txip-clip-value",
		&pinfo->spr.txip_clip.value, 0);
}

static void lcd_kit_spr_datapack_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-datapack-ctrl-value",
		&pinfo->spr.datapack_ctrl.value, 0);
	pinfo->spr.datapack_size.value = 0;
	pinfo->spr.datapack_size.reg.datapack_width = pinfo->xres - 1;
	pinfo->spr.datapack_size.reg.datapack_height = pinfo->yres - 1;
}

static void lcd_kit_spr_degamma_gamma_set(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-gamma-en-value",
		&pinfo->spr.spr_gamma_en.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-gamma-shiften-value",
		&pinfo->spr.spr_gamma_shiften.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-degamma-en-value",
		&pinfo->spr.degamma_en.value, 0);
	pinfo->spr.spr_lut_table = spr_lut_table_temp;
	pinfo->spr.spr_lut_table_len = LCD_MAX_SIZE;
}

static void lcd_kit_parse_spr_config_ctrl(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-ctrl-value",
		&pinfo->spr.spr_ctrl.value, 0);
}

static void lcd_kit_parse_spr_config_pix(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	lcd_kit_parse_u32(np, "lcd-kit,spr-pix-even-value",
		&pinfo->spr.spr_pix_even.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-pix-odd-value",
		&pinfo->spr.spr_pix_odd.value, 0);

	lcd_kit_parse_u32(np, "lcd-kit,spr-pix-panel-value",
		&pinfo->spr.spr_pix_panel.value, 0);
}

static void lcd_kit_parse_spr_config_info(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	pinfo->spr.panel_xres = pinfo->xres;
	pinfo->spr.panel_yres = pinfo->yres;
	pinfo->spr.spr_size.value = 0;
	pinfo->spr.spr_size.reg.spr_width = pinfo->xres - 1;
	pinfo->spr.spr_size.reg.spr_height = pinfo->yres - 1;
	lcd_kit_parse_spr_config_ctrl(panel_id, np, pinfo);
	lcd_kit_parse_spr_config_pix(panel_id, np, pinfo);
}

void lcd_kit_parse_spr_info(int panel_id, struct device_node *np,
	struct dpu_panel_info *pinfo)
{
	unsigned int support = 0;
	lcd_kit_parse_u32(np, "lcd-kit,spr-support", &support, 0);
	if (!support)
		return;
	lcd_kit_parse_spr_config_info(panel_id, np, pinfo);
	lcd_kit_spr_core_border_set(panel_id, np, pinfo);
	lcd_kit_spr_core_larea_set(panel_id, np, pinfo);
	lcd_kit_spr_core_coeffs_b_set(panel_id, np, pinfo);
	lcd_kit_spr_core_coeffs_r_set(panel_id, np, pinfo);
	lcd_kit_spr_core_coeffs_g_set(panel_id, np, pinfo);
	lcd_kit_spr_core_blend_set(panel_id, np, pinfo);
	lcd_kit_spr_core_edgestr_set(panel_id, np, pinfo);
	lcd_kit_spr_core_dir_set(panel_id, np, pinfo);
	lcd_kit_spr_core_diffdirgain_set(panel_id, np, pinfo);
	lcd_kit_spr_core_horz_set(panel_id, np, pinfo);
	lcd_kit_spr_core_vert_set(panel_id, np, pinfo);
	lcd_kit_spr_txip_set(panel_id, np, pinfo);
	lcd_kit_spr_datapack_set(panel_id, np, pinfo);
	lcd_kit_spr_degamma_gamma_set(panel_id, np, pinfo);
}
