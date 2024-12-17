#ifndef DPU_OV_H
#define DPU_OV_H 
#include "soc_dpu_interface.h"
#include "soc_dpu_define.h"
#define MITM_IGM_LUT_LEN 32
#define MITM_GAMMA_LUT_LEN 32
#define MITM_GAMUT_COEF_LEN 9
#define MITM_GAMMA_POS_LEN 8
#define MITM_GAMUT_SCALE_VALUE 10
#define MITM_GAMUT_CLIP_VALUE 65535
#define MITM_ALPHA_VALUE 0x02000000
#define OV_CSC_MAX_IDX 9
enum GamutDirection {
 GAMUT_DIRECTION_NONE = 0,
 GAMUT_SRGB2P3,
 GAMUT_P32BT2020,
};
static unsigned int g_itm_degamma_step_map[8] = {5, 5, 5, 4, 3, 3, 3, 0};
static unsigned int g_itm_degamma_num_map[8] = {1, 10, 19, 28, 37, 55, 62, 63};
static unsigned int g_itm_degamma_pos_map[8] = {32, 320, 608, 752, 824, 968, 1022, 1023};
static unsigned short g_itm_degamma_lut_map[64] = {
 0, 158, 337, 595, 940, 1380, 1923, 2572, 3334, 4213, 5215, 6343, 7603, 8997, 10530, 12205,
 14027, 15998, 18121, 20401, 21600, 22839, 24119, 25440, 26802, 28206, 29651, 31139, 32670, 33451, 34243, 35047,
 35860, 36685, 37521, 38368, 39226, 40095, 40975, 41866, 42768, 43682, 44607, 45543, 46491, 47450, 48420, 49402,
 50396, 51401, 52418, 53446, 54486, 55538, 56601, 57676, 58763, 59862, 60973, 62095, 63230, 64377, 65535, 65535
};
static unsigned int g_itm_gamma_step_map[8] = {7, 7, 8, 9, 10, 11, 13, 0};
static unsigned int g_itm_gamma_num_map[8] = {1, 32, 42, 47, 52, 57, 62, 63};
static unsigned int g_itm_gamma_pos_map[MITM_GAMMA_POS_LEN] = {128, 4096, 6656, 9216, 14336, 24576, 65534, 65535};
static unsigned short g_itm_gamma_lut_map[64] = {
 0, 25, 50, 70, 86, 100, 113, 124, 134, 144, 153, 161, 169, 177, 184, 191,
 198, 205, 211, 217, 223, 229, 234, 240, 245, 250, 255, 260, 265, 270, 274, 279,
 283, 292, 301, 309, 317, 324, 332, 339, 346, 353, 360, 373, 385, 397, 409, 420,
 442, 462, 481, 499, 517, 549, 580, 609, 636, 661, 753, 831, 901, 965, 1023, 1023
};
static int g_itm_gamut_srgb2p3_coef[MITM_GAMUT_COEF_LEN] = {842, 182, 0, 34, 990, 0, 18, 74, 932};
static int g_itm_gamut_p32bt2020_coef[MITM_GAMUT_COEF_LEN] = {772, 204, 48, 47, 964, 13, -1, 18, 1007};
struct dpu_ov_ch {
 DPU_RCH_OV_RCH_OV0_TOP_CLK_SEL_UNION rch_ov0_top_clk_sel;
 DPU_RCH_OV_RCH_OV0_TOP_CLK_EN_UNION rch_ov0_top_clk_en;
 DPU_RCH_OV_RCH_OV1_TOP_CLK_SEL_UNION rch_ov1_top_clk_sel;
 DPU_RCH_OV_RCH_OV1_TOP_CLK_EN_UNION rch_ov1_top_clk_en;
 DPU_RCH_OV_RCH_OV2_TOP_CLK_SEL_UNION rch_ov2_top_clk_sel;
 DPU_RCH_OV_RCH_OV2_TOP_CLK_EN_UNION rch_ov2_top_clk_en;
 DPU_RCH_OV_RCH_OV3_TOP_CLK_SEL_UNION rch_ov3_top_clk_sel;
 DPU_RCH_OV_RCH_OV3_TOP_CLK_EN_UNION rch_ov3_top_clk_en;
 DPU_RCH_OV_WRAP_CLK_SEL_UNION wrap_clk_sel;
 DPU_RCH_OV_WRAP_CLK_EN_UNION wrap_clk_en;
 DPU_RCH_OV_RCH_DBG0_UNION rch_dbg0;
 DPU_RCH_OV_RCH_DBG1_UNION rch_dbg1;
 DPU_RCH_OV_RCH_DBG2_UNION rch_dbg2;
 DPU_RCH_OV_RCH_DBG3_UNION rch_dbg3;
 DPU_RCH_OV_RCH_DBG4_UNION rch_dbg4;
 DPU_RCH_OV_RCH_DBG5_UNION rch_dbg5;
 DPU_RCH_OV_RCH_DBG6_UNION rch_dbg6;
 DPU_RCH_OV_RCH_DBG7_UNION rch_dbg7;
 DPU_RCH_OV_RCH_DBG8_UNION rch_dbg8;
 DPU_RCH_OV_RCH_DBG9_UNION rch_dbg9;
 DPU_RCH_OV_RCH_DBG10_UNION rch_dbg10;
 DPU_RCH_OV_RCH_DBG11_UNION rch_dbg11;
 DPU_RCH_OV_OV_MEM_CTRL_0_UNION ov_mem_ctrl_0;
 DPU_RCH_OV_OV_MEM_CTRL_1_UNION ov_mem_ctrl_1;
 DPU_RCH_OV_OV_MEM_CTRL_2_UNION ov_mem_ctrl_2;
 DPU_RCH_OV_OV_MEM_CTRL_3_UNION ov_mem_ctrl_3;
 DPU_RCH_OV_RCH_DBG12_UNION rch_dbg12;
 DPU_RCH_OV_RCH_DBG13_UNION rch_dbg13;
 DPU_RCH_OV_RCH_DBG14_UNION rch_dbg14;
 DPU_RCH_OV_RCH_DBG15_UNION rch_dbg15;
 DPU_RCH_OV_RCH_DBG16_UNION rch_dbg16;
 DPU_RCH_OV_RCH_DBG17_UNION rch_dbg17;
 DPU_RCH_OV_RCH_DBG18_UNION rch_dbg18;
 DPU_RCH_OV_RCH_DBG19_UNION rch_dbg19;
};
struct dpu_ov_bitext {
 DPU_RCH_OV_BITEXT_CTL_UNION bitext_ctl;
 DPU_RCH_OV_BITEXT_REG_INI0_0_UNION bitext_reg_ini0_0;
 DPU_RCH_OV_BITEXT_REG_INI0_1_UNION bitext_reg_ini0_1;
 DPU_RCH_OV_BITEXT_REG_INI0_2_UNION bitext_reg_ini0_2;
 DPU_RCH_OV_BITEXT_REG_INI0_3_UNION bitext_reg_ini0_3;
 DPU_RCH_OV_BITEXT_REG_INI1_0_UNION bitext_reg_ini1_0;
 DPU_RCH_OV_BITEXT_REG_INI1_1_UNION bitext_reg_ini1_1;
 DPU_RCH_OV_BITEXT_REG_INI1_2_UNION bitext_reg_ini1_2;
 DPU_RCH_OV_BITEXT_REG_INI1_3_UNION bitext_reg_ini1_3;
 DPU_RCH_OV_BITEXT_REG_INI2_0_UNION bitext_reg_ini2_0;
 DPU_RCH_OV_BITEXT_REG_INI2_1_UNION bitext_reg_ini2_1;
 DPU_RCH_OV_BITEXT_REG_INI2_2_UNION bitext_reg_ini2_2;
 DPU_RCH_OV_BITEXT_REG_INI2_3_UNION bitext_reg_ini2_3;
 DPU_RCH_OV_BITEXT_POWER_CTRL_C_UNION bitext_power_ctrl_c;
 DPU_RCH_OV_BITEXT_POWER_CTRL_SHIFT_UNION bitext_power_ctrl_shift;
 DPU_RCH_OV_BITEXT_FILT_00_UNION bitext_filt_00;
 DPU_RCH_OV_BITEXT_FILT_01_UNION bitext_filt_01;
 DPU_RCH_OV_BITEXT_FILT_02_UNION bitext_filt_02;
 DPU_RCH_OV_BITEXT_FILT_10_UNION bitext_filt_10;
 DPU_RCH_OV_BITEXT_FILT_11_UNION bitext_filt_11;
 DPU_RCH_OV_BITEXT_FILT_12_UNION bitext_filt_12;
 DPU_RCH_OV_BITEXT_FILT_20_UNION bitext_filt_20;
 DPU_RCH_OV_BITEXT_FILT_21_UNION bitext_filt_21;
 DPU_RCH_OV_BITEXT_FILT_22_UNION bitext_filt_22;
 DPU_RCH_OV_BITEXT_THD_R0_UNION bitext_thd_r0;
 DPU_RCH_OV_BITEXT_THD_R1_UNION bitext_thd_r1;
 DPU_RCH_OV_BITEXT_THD_G0_UNION bitext_thd_g0;
 DPU_RCH_OV_BITEXT_THD_G1_UNION bitext_thd_g1;
 DPU_RCH_OV_BITEXT_THD_B0_UNION bitext_thd_b0;
 DPU_RCH_OV_BITEXT_THD_B1_UNION bitext_thd_b1;
 DPU_RCH_OV_DFC_GLB_ALPHA23_UNION dfc_glb_alpha23;
 DPU_RCH_OV_BITEXT0_DBG0_UNION bitext0_dbg0;
};
struct dpu_ov_csc_coef {
 DPU_RCH_OV_CSC_IDC0_UNION idc0;
 DPU_RCH_OV_CSC_IDC2_UNION idc2;
 DPU_RCH_OV_CSC_ODC0_UNION odc0;
 DPU_RCH_OV_CSC_ODC2_UNION odc2;
 DPU_RCH_OV_CSC_P00_UNION p00;
 DPU_RCH_OV_CSC_P01_UNION p01;
 DPU_RCH_OV_CSC_P02_UNION p02;
 DPU_RCH_OV_CSC_P10_UNION p10;
 DPU_RCH_OV_CSC_P11_UNION p11;
 DPU_RCH_OV_CSC_P12_UNION p12;
 DPU_RCH_OV_CSC_P20_UNION p20;
 DPU_RCH_OV_CSC_P21_UNION p21;
 DPU_RCH_OV_CSC_P22_UNION p22;
};
struct dpu_ov_csc {
 struct dpu_ov_csc_coef csc_coef;
 DPU_RCH_OV_CSC_MPREC_UNION csc_mprec;
 unsigned int rsv[2];
};
struct dpu_ov_mitm {
 DPU_RCH_OV_MITM_CTRL_UNION mitm_ctrl;
 DPU_RCH_OV_MITM_SLF_DEGAMMA_STEP1_UNION mitm_slf_degamma_step1;
 DPU_RCH_OV_MITM_SLF_DEGAMMA_STEP2_UNION mitm_slf_degamma_step2;
 DPU_RCH_OV_MITM_SLF_DEGAMMA_POS1_UNION mitm_slf_degamma_pos1;
 DPU_RCH_OV_MITM_SLF_DEGAMMA_POS2_UNION mitm_slf_degamma_pos2;
 DPU_RCH_OV_MITM_SLF_DEGAMMA_POS3_UNION mitm_slf_degamma_pos3;
 DPU_RCH_OV_MITM_SLF_DEGAMMA_POS4_UNION mitm_slf_degamma_pos4;
 DPU_RCH_OV_MITM_SLF_DEGAMMA_NUM1_UNION mitm_slf_degamma_num1;
 DPU_RCH_OV_MITM_SLF_DEGAMMA_NUM2_UNION mitm_slf_degamma_num2;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF00_0_UNION mitm_slf_gamut_coef00_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF01_0_UNION mitm_slf_gamut_coef01_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF02_0_UNION mitm_slf_gamut_coef02_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF10_0_UNION mitm_slf_gamut_coef10_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF11_0_UNION mitm_slf_gamut_coef11_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF12_0_UNION mitm_slf_gamut_coef12_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF20_0_UNION mitm_slf_gamut_coef20_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF21_0_UNION mitm_slf_gamut_coef21_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_COEF22_0_UNION mitm_slf_gamut_coef22_0;
 DPU_RCH_OV_MITM_SLF_GAMUT_SCALE_UNION mitm_slf_gamut_scale;
 DPU_RCH_OV_MITM_SLF_GAMUT_CLIP_MAX_UNION mitm_slf_gamut_clip_max;
 DPU_RCH_OV_MITM_SLF_GAMMA_STEP1_UNION mitm_slf_gamma_step1;
 DPU_RCH_OV_MITM_SLF_GAMMA_STEP2_UNION mitm_slf_gamma_step2;
 DPU_RCH_OV_MITM_SLF_GAMMA_POS1_UNION mitm_slf_gamma_pos1;
 DPU_RCH_OV_MITM_SLF_GAMMA_POS2_UNION mitm_slf_gamma_pos2;
 DPU_RCH_OV_MITM_SLF_GAMMA_POS3_UNION mitm_slf_gamma_pos3;
 DPU_RCH_OV_MITM_SLF_GAMMA_POS4_UNION mitm_slf_gamma_pos4;
 DPU_RCH_OV_MITM_SLF_GAMMA_POS5_UNION mitm_slf_gamma_pos5;
 DPU_RCH_OV_MITM_SLF_GAMMA_POS6_UNION mitm_slf_gamma_pos6;
 DPU_RCH_OV_MITM_SLF_GAMMA_POS7_UNION mitm_slf_gamma_pos7;
 DPU_RCH_OV_MITM_SLF_GAMMA_POS8_UNION mitm_slf_gamma_pos8;
 DPU_RCH_OV_MITM_SLF_GAMMA_NUM1_UNION mitm_slf_gamma_num1;
 DPU_RCH_OV_MITM_SLF_GAMMA_NUM2_UNION mitm_slf_gamma_num2;
 DPU_RCH_OV_MITM_ALPHA_UNION mitm_alpha;
 DPU_RCH_OV_MITM_DEGAMMA_LUT_UNION mitm_degamma_lut[MITM_IGM_LUT_LEN];
 DPU_RCH_OV_MITM_GAMMA_LUT_UNION mitm_gamma_lut[MITM_GAMMA_LUT_LEN];
 DPU_RCH_OV_MITM_DBG0_UNION mitm_dbg0;
 DPU_RCH_OV_MITM_DBG1_UNION mitm_dbg1;
 unsigned int rsv;
};
struct dpu_ov_ov {
 DPU_RCH_OV_LAYER_SRCLOKEY_UNION layer_srclokey;
 DPU_RCH_OV_LAYER_SRCHIKEY_UNION layer_srchikey;
 DPU_RCH_OV_LAYER_DSTLOKEY_UNION layer_dstlokey;
 DPU_RCH_OV_LAYER_DSTHIKEY_UNION layer_dsthikey;
 DPU_RCH_OV_LAYER_ALPHA_MODE_UNION layer_alpha_mode;
 DPU_RCH_OV_OV0_DBG0_UNION ov0_dbg0;
 DPU_RCH_OV_OV0_DBG1_UNION ov0_dbg1;
 DPU_RCH_OV_OV0_DBG2_UNION ov0_dbg2;
 DPU_RCH_OV_OV0_DBG3_UNION ov0_dbg3;
 DPU_RCH_OV_OV1_DBG0_UNION ov1_dbg0;
 DPU_RCH_OV_OV1_DBG1_UNION ov1_dbg1;
 DPU_RCH_OV_OV1_DBG2_UNION ov1_dbg2;
 DPU_RCH_OV_OV1_DBG3_UNION ov1_dbg3;
 DPU_RCH_OV_OV2_DBG0_UNION ov2_dbg0;
 DPU_RCH_OV_OV2_DBG1_UNION ov2_dbg1;
 DPU_RCH_OV_OV2_DBG2_UNION ov2_dbg2;
 DPU_RCH_OV_OV2_DBG3_UNION ov2_dbg3;
 DPU_RCH_OV_OV3_DBG0_UNION ov3_dbg0;
 DPU_RCH_OV_OV3_DBG1_UNION ov3_dbg1;
 DPU_RCH_OV_OV3_DBG2_UNION ov3_dbg2;
 DPU_RCH_OV_OV3_DBG3_UNION ov3_dbg3;
};
struct dpu_ov_params {
 struct dpu_ov_ch ov_ch;
 struct dpu_ov_bitext ov_bitext;
 struct dpu_ov_csc ov_csc;
 struct dpu_ov_mitm ov_mitm;
 struct dpu_ov_ov ov_ov;
};
#endif
