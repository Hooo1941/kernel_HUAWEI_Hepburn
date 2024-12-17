#ifndef DPU_MITM_H
#define DPU_MITM_H 
#include "dpu_ov.h"
#define init_mitm_coe(coe) { .value = (coe), }
static struct dpu_ov_mitm g_mitm_srgb2p3 = {
 .mitm_ctrl = init_mitm_coe(0x000),
 .mitm_slf_degamma_step1 = {
  .reg = {
   .mitm_slf_degmm_x1_step = 5,
   .mitm_slf_degmm_x2_step = 5,
   .mitm_slf_degmm_x3_step = 5,
   .mitm_slf_degmm_x4_step = 4,
  },
 },
 .mitm_slf_degamma_step2 = {
  .reg = {
   .mitm_slf_degmm_x5_step = 3,
   .mitm_slf_degmm_x6_step = 3,
   .mitm_slf_degmm_x7_step = 3,
   .mitm_slf_degmm_x8_step = 0,
  },
 },
 .mitm_slf_degamma_pos1 = {
  .reg = {
   .mitm_slf_degmm_x1_pos = 32,
   .mitm_slf_degmm_x2_pos = 320,
  },
 },
 .mitm_slf_degamma_pos2 = {
  .reg = {
   .mitm_slf_degmm_x3_pos = 608,
   .mitm_slf_degmm_x4_pos = 752,
  },
 },
 .mitm_slf_degamma_pos3 = {
  .reg = {
   .mitm_slf_degmm_x5_pos = 824,
   .mitm_slf_degmm_x6_pos = 968,
  },
 },
 .mitm_slf_degamma_pos4 = {
  .reg = {
   .mitm_slf_degmm_x7_pos = 1022,
   .mitm_slf_degmm_x8_pos = 1023,
  },
 },
 .mitm_slf_degamma_num1 = {
  .reg = {
   .mitm_slf_degmm_x1_num = 1,
   .mitm_slf_degmm_x2_num = 10,
   .mitm_slf_degmm_x3_num = 19,
   .mitm_slf_degmm_x4_num = 28,
  },
 },
 .mitm_slf_degamma_num2 = {
  .reg = {
   .mitm_slf_degmm_x5_num = 37,
   .mitm_slf_degmm_x6_num = 55,
   .mitm_slf_degmm_x7_num = 62,
   .mitm_slf_degmm_x8_num = 63,
  },
 },
 .mitm_slf_gamut_coef00_0 = init_mitm_coe(842),
 .mitm_slf_gamut_coef01_0 = init_mitm_coe(182),
 .mitm_slf_gamut_coef02_0 = init_mitm_coe(0),
 .mitm_slf_gamut_coef10_0 = init_mitm_coe(34),
 .mitm_slf_gamut_coef11_0 = init_mitm_coe(990),
 .mitm_slf_gamut_coef12_0 = init_mitm_coe(0),
 .mitm_slf_gamut_coef20_0 = init_mitm_coe(18),
 .mitm_slf_gamut_coef21_0 = init_mitm_coe(74),
 .mitm_slf_gamut_coef22_0 = init_mitm_coe(932),
 .mitm_slf_gamut_scale.value = MITM_GAMUT_SCALE_VALUE,
    .mitm_slf_gamut_clip_max.value = MITM_GAMUT_CLIP_VALUE,
 .mitm_slf_gamma_step1 = {
  .reg = {
   .mitm_slf_gmm_x1_step = 7,
   .mitm_slf_gmm_x2_step = 7,
   .mitm_slf_gmm_x3_step = 8,
   .mitm_slf_gmm_x4_step = 9,
  },
 },
 .mitm_slf_gamma_step2 = {
  .reg = {
   .mitm_slf_gmm_x5_step = 10,
   .mitm_slf_gmm_x6_step = 11,
   .mitm_slf_gmm_x7_step = 13,
   .mitm_slf_gmm_x8_step = 0,
  },
 },
 .mitm_slf_gamma_pos1 = init_mitm_coe(128),
 .mitm_slf_gamma_pos2 = init_mitm_coe(4096),
 .mitm_slf_gamma_pos3 = init_mitm_coe(6656),
 .mitm_slf_gamma_pos4 = init_mitm_coe(9216),
 .mitm_slf_gamma_pos5 = init_mitm_coe(14336),
 .mitm_slf_gamma_pos6 = init_mitm_coe(24576),
 .mitm_slf_gamma_pos7 = init_mitm_coe(65534),
 .mitm_slf_gamma_pos8 = init_mitm_coe(65535),
 .mitm_slf_gamma_num1 = {
  .reg = {
   .mitm_slf_gmm_x1_num = 1,
   .mitm_slf_gmm_x2_num = 32,
   .mitm_slf_gmm_x3_num = 42,
   .mitm_slf_gmm_x4_num = 47,
  },
 },
 .mitm_slf_gamma_num2 = {
  .reg = {
   .mitm_slf_gmm_x5_num = 52,
   .mitm_slf_gmm_x6_num = 57,
   .mitm_slf_gmm_x7_num = 62,
   .mitm_slf_gmm_x8_num = 63,
  },
 },
 .mitm_alpha.value = MITM_ALPHA_VALUE,
 .mitm_degamma_lut = {
  {
   .reg = {
    .mitm_degamma_even_coef = 0,
    .mitm_degamma_odd_coef = 158,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 337,
    .mitm_degamma_odd_coef = 595,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 940,
    .mitm_degamma_odd_coef = 1380,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 1923,
    .mitm_degamma_odd_coef = 2572,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 3334,
    .mitm_degamma_odd_coef = 4213,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 5215,
    .mitm_degamma_odd_coef = 6343,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 7603,
    .mitm_degamma_odd_coef = 8997,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 10530,
    .mitm_degamma_odd_coef = 12205,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 14027,
    .mitm_degamma_odd_coef = 15998,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 18121,
    .mitm_degamma_odd_coef = 20401,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 21600,
    .mitm_degamma_odd_coef = 22839,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 24119,
    .mitm_degamma_odd_coef = 25440,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 26802,
    .mitm_degamma_odd_coef = 28206,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 29651,
    .mitm_degamma_odd_coef = 31139,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 32670,
    .mitm_degamma_odd_coef = 33451,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 34243,
    .mitm_degamma_odd_coef = 35047,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 35860,
    .mitm_degamma_odd_coef = 36685,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 37521,
    .mitm_degamma_odd_coef = 38368,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 39226,
    .mitm_degamma_odd_coef = 40095,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 40975,
    .mitm_degamma_odd_coef = 41866,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 42768,
    .mitm_degamma_odd_coef = 43682,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 44607,
    .mitm_degamma_odd_coef = 45543,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 46491,
    .mitm_degamma_odd_coef = 47450,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 48420,
    .mitm_degamma_odd_coef = 49402,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 50396,
    .mitm_degamma_odd_coef = 51401,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 52418,
    .mitm_degamma_odd_coef = 53446,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 54486,
    .mitm_degamma_odd_coef = 55538,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 56601,
    .mitm_degamma_odd_coef = 57676,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 58763,
    .mitm_degamma_odd_coef = 59862,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 60973,
    .mitm_degamma_odd_coef = 62095,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 63230,
    .mitm_degamma_odd_coef = 64377,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 65535,
    .mitm_degamma_odd_coef = 65535,
   },
  },
 },
 .mitm_gamma_lut = {
  {
   .reg = {
    .mitm_gamma_even_coef = 0,
    .mitm_gamma_odd_coef = 25,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 50,
    .mitm_gamma_odd_coef = 70,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 86,
    .mitm_gamma_odd_coef = 100,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 113,
    .mitm_gamma_odd_coef = 124,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 134,
    .mitm_gamma_odd_coef = 144,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 153,
    .mitm_gamma_odd_coef = 161,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 169,
    .mitm_gamma_odd_coef = 177,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 184,
    .mitm_gamma_odd_coef = 191,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 198,
    .mitm_gamma_odd_coef = 205,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 211,
    .mitm_gamma_odd_coef = 217,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 223,
    .mitm_gamma_odd_coef = 229,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 234,
    .mitm_gamma_odd_coef = 240,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 245,
    .mitm_gamma_odd_coef = 250,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 255,
    .mitm_gamma_odd_coef = 260,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 265,
    .mitm_gamma_odd_coef = 270,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 274,
    .mitm_gamma_odd_coef = 279,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 283,
    .mitm_gamma_odd_coef = 292,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 301,
    .mitm_gamma_odd_coef = 309,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 317,
    .mitm_gamma_odd_coef = 324,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 332,
    .mitm_gamma_odd_coef = 339,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 346,
    .mitm_gamma_odd_coef = 353,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 360,
    .mitm_gamma_odd_coef = 373,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 385,
    .mitm_gamma_odd_coef = 397,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 409,
    .mitm_gamma_odd_coef = 420,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 442,
    .mitm_gamma_odd_coef = 462,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 481,
    .mitm_gamma_odd_coef = 499,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 517,
    .mitm_gamma_odd_coef = 549,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 580,
    .mitm_gamma_odd_coef = 609,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 636,
    .mitm_gamma_odd_coef = 661,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 753,
    .mitm_gamma_odd_coef = 831,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 901,
    .mitm_gamma_odd_coef = 965,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 1023,
    .mitm_gamma_odd_coef = 1023,
   },
  },
 },
 .mitm_dbg0 = init_mitm_coe(0x000),
 .mitm_dbg1 = init_mitm_coe(0x000),
};
static struct dpu_ov_mitm g_mitm_bt7092p3 = {
 .mitm_ctrl = init_mitm_coe(0x000),
 .mitm_slf_degamma_step1 = {
  .reg = {
   .mitm_slf_degmm_x1_step = 5,
   .mitm_slf_degmm_x2_step = 5,
   .mitm_slf_degmm_x3_step = 5,
   .mitm_slf_degmm_x4_step = 4,
  },
 },
 .mitm_slf_degamma_step2 = {
  .reg = {
   .mitm_slf_degmm_x5_step = 3,
   .mitm_slf_degmm_x6_step = 3,
   .mitm_slf_degmm_x7_step = 3,
   .mitm_slf_degmm_x8_step = 0,
  },
 },
 .mitm_slf_degamma_pos1 = {
  .reg = {
   .mitm_slf_degmm_x1_pos = 32,
   .mitm_slf_degmm_x2_pos = 320,
  },
 },
 .mitm_slf_degamma_pos2 = {
  .reg = {
   .mitm_slf_degmm_x3_pos = 608,
   .mitm_slf_degmm_x4_pos = 752,
  },
 },
 .mitm_slf_degamma_pos3 = {
  .reg = {
   .mitm_slf_degmm_x5_pos = 824,
   .mitm_slf_degmm_x6_pos = 968,
  },
 },
 .mitm_slf_degamma_pos4 = {
  .reg = {
   .mitm_slf_degmm_x7_pos = 1022,
   .mitm_slf_degmm_x8_pos = 1023,
  },
 },
 .mitm_slf_degamma_num1 = {
  .reg = {
   .mitm_slf_degmm_x1_num = 1,
   .mitm_slf_degmm_x2_num = 10,
   .mitm_slf_degmm_x3_num = 19,
   .mitm_slf_degmm_x4_num = 28,
  },
 },
 .mitm_slf_degamma_num2 = {
  .reg = {
   .mitm_slf_degmm_x5_num = 37,
   .mitm_slf_degmm_x6_num = 55,
   .mitm_slf_degmm_x7_num = 62,
   .mitm_slf_degmm_x8_num = 63,
  },
 },
 .mitm_slf_gamut_coef00_0 = init_mitm_coe(842),
 .mitm_slf_gamut_coef01_0 = init_mitm_coe(182),
 .mitm_slf_gamut_coef02_0 = init_mitm_coe(0),
 .mitm_slf_gamut_coef10_0 = init_mitm_coe(34),
 .mitm_slf_gamut_coef11_0 = init_mitm_coe(990),
 .mitm_slf_gamut_coef12_0 = init_mitm_coe(0),
 .mitm_slf_gamut_coef20_0 = init_mitm_coe(18),
 .mitm_slf_gamut_coef21_0 = init_mitm_coe(74),
 .mitm_slf_gamut_coef22_0 = init_mitm_coe(932),
 .mitm_slf_gamut_scale.value = MITM_GAMUT_SCALE_VALUE,
    .mitm_slf_gamut_clip_max.value = MITM_GAMUT_CLIP_VALUE,
 .mitm_slf_gamma_step1 = {
  .reg = {
   .mitm_slf_gmm_x1_step = 7,
   .mitm_slf_gmm_x2_step = 7,
   .mitm_slf_gmm_x3_step = 8,
   .mitm_slf_gmm_x4_step = 9,
  },
 },
 .mitm_slf_gamma_step2 = {
  .reg = {
   .mitm_slf_gmm_x5_step = 10,
   .mitm_slf_gmm_x6_step = 11,
   .mitm_slf_gmm_x7_step = 13,
   .mitm_slf_gmm_x8_step = 0,
  },
 },
 .mitm_slf_gamma_pos1 = init_mitm_coe(128),
 .mitm_slf_gamma_pos2 = init_mitm_coe(4096),
 .mitm_slf_gamma_pos3 = init_mitm_coe(6656),
 .mitm_slf_gamma_pos4 = init_mitm_coe(9216),
 .mitm_slf_gamma_pos5 = init_mitm_coe(14336),
 .mitm_slf_gamma_pos6 = init_mitm_coe(24576),
 .mitm_slf_gamma_pos7 = init_mitm_coe(65534),
 .mitm_slf_gamma_pos8 = init_mitm_coe(65535),
 .mitm_slf_gamma_num1 = {
  .reg = {
   .mitm_slf_gmm_x1_num = 1,
   .mitm_slf_gmm_x2_num = 32,
   .mitm_slf_gmm_x3_num = 42,
   .mitm_slf_gmm_x4_num = 47,
  },
 },
 .mitm_slf_gamma_num2 = {
  .reg = {
   .mitm_slf_gmm_x5_num = 52,
   .mitm_slf_gmm_x6_num = 57,
   .mitm_slf_gmm_x7_num = 62,
   .mitm_slf_gmm_x8_num = 63,
  },
 },
 .mitm_alpha.value = MITM_ALPHA_VALUE,
 .mitm_degamma_lut = {
  {
   .reg = {
    .mitm_degamma_even_coef = 0,
    .mitm_degamma_odd_coef = 456,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 911,
    .mitm_degamma_odd_coef = 1371,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 1914,
    .mitm_degamma_odd_coef = 2559,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 3308,
    .mitm_degamma_odd_coef = 4164,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 5130,
    .mitm_degamma_odd_coef = 6207,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 7398,
    .mitm_degamma_odd_coef = 8705,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 10130,
    .mitm_degamma_odd_coef = 11675,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 13341,
    .mitm_degamma_odd_coef = 15130,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 17043,
    .mitm_degamma_odd_coef = 19082,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 21249,
    .mitm_degamma_odd_coef = 23545,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 24742,
    .mitm_degamma_odd_coef = 25971,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 27233,
    .mitm_degamma_odd_coef = 28528,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 29857,
    .mitm_degamma_odd_coef = 31218,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 32614,
    .mitm_degamma_odd_coef = 34042,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 35505,
    .mitm_degamma_odd_coef = 36249,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 37001,
    .mitm_degamma_odd_coef = 37762,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 38532,
    .mitm_degamma_odd_coef = 39310,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 40097,
    .mitm_degamma_odd_coef = 40892,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 41696,
    .mitm_degamma_odd_coef = 42508,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 43329,
    .mitm_degamma_odd_coef = 44159,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 44997,
    .mitm_degamma_odd_coef = 45844,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 46700,
    .mitm_degamma_odd_coef = 47564,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 48437,
    .mitm_degamma_odd_coef = 49319,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 50210,
    .mitm_degamma_odd_coef = 51109,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 52018,
    .mitm_degamma_odd_coef = 52935,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 53861,
    .mitm_degamma_odd_coef = 54795,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 55739,
    .mitm_degamma_odd_coef = 56691,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 57653,
    .mitm_degamma_odd_coef = 58623,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 59567,
    .mitm_degamma_odd_coef = 60519,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 61480,
    .mitm_degamma_odd_coef = 62449,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 63426,
    .mitm_degamma_odd_coef = 64412,
   },
  },
  {
   .reg = {
    .mitm_degamma_even_coef = 65406,
    .mitm_degamma_odd_coef = 65535,
   },
  },
 },
 .mitm_gamma_lut = {
  {
   .reg = {
    .mitm_gamma_even_coef = 0,
    .mitm_gamma_odd_coef = 25,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 50,
    .mitm_gamma_odd_coef = 70,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 86,
    .mitm_gamma_odd_coef = 100,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 113,
    .mitm_gamma_odd_coef = 124,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 134,
    .mitm_gamma_odd_coef = 144,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 153,
    .mitm_gamma_odd_coef = 161,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 169,
    .mitm_gamma_odd_coef = 177,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 184,
    .mitm_gamma_odd_coef = 191,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 198,
    .mitm_gamma_odd_coef = 205,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 211,
    .mitm_gamma_odd_coef = 217,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 223,
    .mitm_gamma_odd_coef = 229,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 234,
    .mitm_gamma_odd_coef = 240,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 245,
    .mitm_gamma_odd_coef = 250,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 255,
    .mitm_gamma_odd_coef = 260,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 265,
    .mitm_gamma_odd_coef = 270,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 274,
    .mitm_gamma_odd_coef = 279,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 283,
    .mitm_gamma_odd_coef = 292,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 301,
    .mitm_gamma_odd_coef = 309,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 317,
    .mitm_gamma_odd_coef = 324,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 332,
    .mitm_gamma_odd_coef = 339,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 346,
    .mitm_gamma_odd_coef = 353,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 360,
    .mitm_gamma_odd_coef = 373,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 385,
    .mitm_gamma_odd_coef = 397,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 409,
    .mitm_gamma_odd_coef = 420,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 442,
    .mitm_gamma_odd_coef = 462,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 481,
    .mitm_gamma_odd_coef = 499,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 517,
    .mitm_gamma_odd_coef = 549,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 580,
    .mitm_gamma_odd_coef = 609,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 636,
    .mitm_gamma_odd_coef = 661,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 753,
    .mitm_gamma_odd_coef = 831,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 901,
    .mitm_gamma_odd_coef = 965,
   },
  },
  {
   .reg = {
    .mitm_gamma_even_coef = 1023,
    .mitm_gamma_odd_coef = 1023,
   },
  },
 },
 .mitm_dbg0 = init_mitm_coe(0x000),
 .mitm_dbg1 = init_mitm_coe(0x000),
};
#endif
