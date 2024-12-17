/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Implementation of the mp16c ssi host function.
 * Author: @CompanyNameTag
 */

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
#include "ssi_mp16c.h"

#include "plat_debug.h"
#include "plat_pm.h"
#include "ssi_common.h"
#include "plat_parse_changid.h"
#include "ssi_spmi.h"
#include "oal_util.h"
#include "oam_dsm.h"
#include "oam_ext_if.h"
#include "chr_user.h"


#undef THIS_FILE_ID
#define THIS_FILE_ID  OAM_FILE_ID_SSI_MP16C_C

#define BOARD_TCXO_38P4         1
#define BOARD_TCXO_76P8         0
#define TCXO_32K_DET_VALUE      10
#define TCXO_LIMIT_THRESHOLD    5
#define TCXO_38P4_NOMAL_CKL     38400000
#define TCXO_76P8_NOMAL_CKL     76800000
#define RTC_32K_NOMAL_CKL       32768
#define SPMI_BASE_ADDR          0x40018000

#define SSI_DUMP_LIMIT_TIMEOUT  4200000 // 超时时间定为4.2s 最低设置为 1s

/* hi6506 reg, reg_offset, default_value */
#define CHIP_VERSION_2                 0x2
#define CHIP_VERSION_2_DEFAULT_VALUE   0x30
/* IRQ MASK */
#define SCP0_MASK_REG                  0x5000
#define OCP0_MASK_REG                  0x5001
#define OCP1_MASK_REG                  0x5002
#define IRQ0_MASK_REG                  0x5003
#define OCP2_MASK_REG                  0X5004
/* IRQ */
#define SCP0_IRQ_REG                   0x6000
#define OCP0_IRQ_REG                   0x6001
#define OCP1_IRQ_REG                   0x6002
#define IRQ0_IRQ_REG                   0x6003
#define OCP2_IRQ_REG                   0X6004
/* NP_EVENT */
#define SCP0_NP_EVENT_REG              0x7000
#define OCP0_NP_EVENT_REG              0x7001
#define OCP1_NP_EVENT_REG              0x7002
#define IRQ0_NP_EVENT_REG              0x7003
#define OCP2_NP_EVENT_REG              0x7004
/* BUCK0/SYSLDO/CLDO2 VSET */
#define WLAN_W_BUCK0_VSET_REG          0x1040
#define WLAN_W_SYSLDO_VSET_REG         0x1042
#define WLAN_W_CLDO2_VSET_REG          0x1044

/* 以下寄存器是mp16c device定义 */
#define GLB_CTL_BASE                              0x40000000
#define GLB_CTL_RB_AON_TCXO2RF_SOC_CK_CTL_REG     (GLB_CTL_BASE + 0x28)  /* TCXO源头时钟（76.8M)cg使能 */
#define GLB_CTL_RB_AON_PCIE_SOFT_RST_REG          (GLB_CTL_BASE + 0x54)
#define GLB_CTL_SOFT_RST_BCPU_REG                 (GLB_CTL_BASE + 0x70)
#define GLB_CTL_SOFT_RST_GCPU_REG                 (GLB_CTL_BASE + 0x74)
#define GLB_CTL_RB_AON_CRG_CKEN_GOS_REG           (GLB_CTL_BASE + 0xB4)  /* TCXO2rf_gos时钟门控控制 */
#define GLB_CTL_SYS_TICK_CFG_W_REG                (GLB_CTL_BASE + 0x100)  /* 写1清零systick，写0无效 */
#define GLB_CTL_SYS_TICK_VALUE_W_0_REG            (GLB_CTL_BASE + 0x104)
#define GLB_CTL_SYS_TICK_CFG_B_REG                (GLB_CTL_BASE + 0x114)  /* 写1清零systick，写0无效 */
#define GLB_CTL_SYS_TICK_VALUE_B_0_REG            (GLB_CTL_BASE + 0x118)
#define GLB_CTL_SYS_TICK_CFG_GF_REG               (GLB_CTL_BASE + 0x180)  /* 写1清零systick，写0无效 */
#define GLB_CTL_SYS_TICK_VALUE_GF_0_REG           (GLB_CTL_BASE + 0x184)
#define GLB_CTL_SYS_TICK_CFG_SLE_REG              (GLB_CTL_BASE + 0x194)  /* 写1清零systick，写0无效 */
#define GLB_CTL_SYS_TICK_VALUE_SLE_0_REG          (GLB_CTL_BASE + 0x198)
#define GLB_CTL_PWR_ON_LABLE_REG                  (GLB_CTL_BASE + 0x200)  /* 芯片上电标记寄存器 */
#define GLB_CTL_BCPU_LOAD_REG                     (GLB_CTL_BASE + 0x1080) /* BCPU_LOAD */
#define GLB_CTL_BCPU_PC_L_REG                     (GLB_CTL_BASE + 0x1084) /* BCPU_PC低16bit */
#define GLB_CTL_BCPU_PC_H_REG                     (GLB_CTL_BASE + 0x1088) /* BCPU_PC高16bit */
#define GLB_CTL_BCPU_LR_L_REG                     (GLB_CTL_BASE + 0x108C) /* BCPU_LR低16bit */
#define GLB_CTL_BCPU_LR_H_REG                     (GLB_CTL_BASE + 0x1090) /* BCPU_LR高16bit */
#define GLB_CTL_BCPU_SP_L_REG                     (GLB_CTL_BASE + 0x1094) /* BCPU_SP低16bit */
#define GLB_CTL_BCPU_SP_H_REG                     (GLB_CTL_BASE + 0x1098) /* BCPU_SP高16bit */
#define GLB_CTL_GCPU_LOAD_REG                     (GLB_CTL_BASE + 0x109c) /* GCPU_LOAD */
#define GLB_CTL_GCPU_PC_L_REG                     (GLB_CTL_BASE + 0x10A0) /* GCPU_PC低16bit */
#define GLB_CTL_GCPU_PC_H_REG                     (GLB_CTL_BASE + 0x10A4) /* GCPU_PC高16bit */
#define GLB_CTL_GCPU_LR_L_REG                     (GLB_CTL_BASE + 0x10A8) /* GCPU_LR低16bit */
#define GLB_CTL_GCPU_LR_H_REG                     (GLB_CTL_BASE + 0x10AC) /* GCPU_LR高16bit */
#define GLB_CTL_GCPU_SP_L_REG                     (GLB_CTL_BASE + 0x10B0) /* GCPU_SP低16bit */
#define GLB_CTL_GCPU_SP_H_REG                     (GLB_CTL_BASE + 0x10B4) /* GCPU_SP高16bit */
#define GLB_CTL_SLE_LOAD_REG                      (GLB_CTL_BASE + 0x10B8) /* SLE_LOAD */
#define GLB_CTL_SLE_PC_L_REG                      (GLB_CTL_BASE + 0x10BC) /* SLE_PC低16bit */
#define GLB_CTL_SLE_PC_H_REG                      (GLB_CTL_BASE + 0x10C0) /* SLE_PC高16bit */
#define GLB_CTL_SLE_LR_L_REG                      (GLB_CTL_BASE + 0x10C4) /* SLE_LR低16bit */
#define GLB_CTL_SLE_LR_H_REG                      (GLB_CTL_BASE + 0x10C8) /* SLE_LR高16bit */
#define GLB_CTL_WCPU_LOAD_REG                     (GLB_CTL_BASE + 0x10CC) /* WCPU_LOAD */
#define GLB_CTL_WCPU_PC_L_REG                     (GLB_CTL_BASE + 0x10D0) /* WCPU_PC低16bit */
#define GLB_CTL_WCPU_PC_H_REG                     (GLB_CTL_BASE + 0x10D4) /* WCPU_PC高16bit */
#define GLB_CTL_WCPU_LR_L_REG                     (GLB_CTL_BASE + 0x10D8) /* WCPU_LR低16bit */
#define GLB_CTL_WCPU_LR_H_REG                     (GLB_CTL_BASE + 0x10Dc) /* WCPU_LR高16bit */
#define GLB_CTL_WCPU_SP_L_REG                     (GLB_CTL_BASE + 0x10E0) /* WCPU_SP低16bit */
#define GLB_CTL_WCPU_SP_H_REG                     (GLB_CTL_BASE + 0x10E4) /* WCPU_SP高16bit */
#define GLB_CTL_TCXO_DET_CTL_REG                  (GLB_CTL_BASE + 0x550)  /* TCXO时钟检测控制寄存器 */
#define GLB_CTL_TCXO_32K_DET_CNT_REG              (GLB_CTL_BASE + 0x554)  /* TCXO时钟检测控制寄存器 */
#define GLB_CTL_TCXO_32K_DET_RESULT_REG           (GLB_CTL_BASE + 0x558)  /* TCXO时钟检测控制寄存器 */
#define GLB_CTL_WCPU_WAIT_CTL_REG                 (GLB_CTL_BASE + 0x85C)
#define GLB_CTL_BCPU_WAIT_CTL_REG                 (GLB_CTL_BASE + 0x860)
#define GLB_CTL_GFCPU_WAIT_CTL_REG                (GLB_CTL_BASE + 0x864)
#define GLB_CTL_RB_TCXO_FREQ_DET_REG              (GLB_CTL_BASE + 0x9B8)  /* 板级晶振频率 */
#define GLB_CTL_RB_PINMUX_CFG_3_REG               (GLB_CTL_BASE + 0x40C)

#define GLB_CTL_RB_TCXO2RF_SOC_CLKEN_MASK         0x1
#define GLB_CTL_RB_TCXO2RF_GOS_CLKEN_MASK         0x1
#define GLB_CTL_RB_SOFT_RST_PCIE0_N_MASK          0x1

#define WIFI_CPU_TRACE_CTL_RB_BASE                 (0x40024000 + 0xC0000)
#define WIFI_TRACE_PC_RAM_SEL                      (WIFI_CPU_TRACE_CTL_RB_BASE + 0x10C)
#define BT_CPU_TRACE_CTL_RB_BASE                   (0x40024000 + 0x1000000)
#define BT_TRACE_PC_RAM_SEL                        (BT_CPU_TRACE_CTL_RB_BASE + 0x10C)
#define SLE_CPU_TRACE_CTL_RB_BASE                  (0x42020000)
#define SLE_TRACE_PC_RAM_SEL                       (SLE_CPU_TRACE_CTL_RB_BASE + 0x10C)
#define GF_CPU_TRACE_CTL_RB_BASE                   (0x40024000 + 0x1000000*3)
#define GF_TRACE_PC_RAM_SEL                        (GF_CPU_TRACE_CTL_RB_BASE + 0x10C)
#define W_CTL_RB_BASE                              0x40105000
#define B_CTL_RB_BASE                              0x41000000
#define GF_CTL_RB_BASE                             0x43000000
#define GLE_CTL_RB_BASE                            0x42000000

#define W_CTL_RB_W_PERP_SUB_CLKEN_CFG0_REG         (W_CTL_RB_BASE + 0x1004)
#define W_CTL_RB_SOFT_RST_CFG0_REG                 (W_CTL_RB_BASE + 0x1010)
#define B_CTL_RB_BTOP_CRG_SOFT_RST_REG             (B_CTL_RB_BASE + 0x68)
#define B_CTL_RB_BTOP_CRG_CLOCK_EN_REG             (B_CTL_RB_BASE + 0x70)
#define GF_CTL_RB_GFTOP_CRG_SOFT_RST_REG           (GF_CTL_RB_BASE + 0x7C)
#define GF_CTL_RB_GFTOP_CRG_CLOCK_EN_REG           (GF_CTL_RB_BASE + 0x80)
#define GLE_CTL_RB_GLE_TOP_CRG_SOFT_RST_REG        (GLE_CTL_RB_BASE + 0x414)
#define GLE_CTL_RB_GLE_TOP_CRG_CLOCK_EN_REG        (GLE_CTL_RB_BASE + 0x418)

#define W_CTL_RB_SOFT_RST_DIAG_N_MASK              0x2000
#define W_CTL_RB_W_DIAG_CTL_CLKEN_MASK             0x20
#define B_CTL_RB_SOFT_RST_B_DIAG_CTL_N_MASK        0x10
#define B_CTL_RB_B_DIAG_CTL_CLKEN_MASK             0x20
#define GF_CTL_RB_SOFT_RST_GF_DIAG_CTL_N_MASK      0x10
#define GF_CTL_RB_GF_DIAG_CTL_CLKEN_MASK           0x8
#define GLE_CTL_RB_SOFT_RST_GLE_DIAG_CTL_N_MASK    0x8
#define GLE_CTL_RB_GLE_DIAG_CTL_CLKEN_MASK         0x10
#define B_CTL_RB_B_RF_ABB_CTL_CLKEN_MASK           0x40
#define GF_CTL_RB_GF_RF_ABB_CTL_CLKEN_MASK         0x10

#define PMU_CMU_CTL_BASE                    0x40002000
#define PMU_CMU_CTL_SYS_STATUS_0_REG        (PMU_CMU_CTL_BASE + 0x1C0) /* 系统状态 */
#define PMU_CMU_CTL_SYS_STATUS_3_REG        (PMU_CMU_CTL_BASE + 0x1CC) /* 子系统EN状态 */
#define PMU_CMU_CTL_WCPU_STATUS_OFFSET      0x0
#define PMU_CMU_CTL_WCPU_STATUS_MASK        0x7
#define PMU_CMU_CTL_BCPU_STATUS_OFFSET      0x3
#define PMU_CMU_CTL_BCPU_STATUS_MASK        0x38
#define PMU_CMU_CTL_GCPU_STATUS_OFFSET      0x6
#define PMU_CMU_CTL_GCPU_STATUS_MASK        0x1C0
#define PMU_CMU_CTL_SLE_STATUS_OFFSET       0x9
#define PMU_CMU_CTL_SLE_STATUS_MASK         0xE00
#define PMU_CMU_CTL_CLOCK_TCXO_PRESENCE_DET (PMU_CMU_CTL_BASE + 0x040) /* TCXO时钟在位检测 */

#define CTL_WCPU_PC_LR_LOAD_EN              0x41
#define CTL_WCPU_PC_LR_CLR_EN               0x20
#define CTL_BCPU_PC_LR_LOAD_EN              0x81
#define CTL_BCPU_PC_LR_CLR_EN               0x40
#define CTL_GCPU_PC_LR_LOAD_EN              0x41
#define CTL_GCPU_PC_LR_CLR_EN               0x20
#define CTL_SLE_PC_LR_LOAD_EN               0x41
#define CTL_SLE_PC_LR_CLR_EN                0x20

#define PMU_CMU_CTL_RB_XO_DET_VAL_MASK      0x100 /* TCXO在位检测结果有效 */
#define PMU_CMU_CTL_RB_XO_DET_STS_MASK      0x200 /* TCXO时钟在位 */

#define BCPU_PLL_CTRL_MDLL_LOCK_STATUS_REG           0x41013084 /* BCPU的mdpll锁存状态 */
#define SLE_PLL_CTRL_MDLL_LOCK_STATUS_REG            0x42011084 /* SLE的mdpll锁存状态 */
#define PMU_CMU_CTL_RB_PMU1_STATUS_GRM_REG           0x40002f20 /* PMU1滤毛刺信号 */
#define PMU_CMU_CTL_RB_PMU1_STATUS_GRM_STICK_REG     0x40002f24 /* PMU1滤毛刺信号 */
#define PMU_CMU_CTL_RB_IRQ_FROM_PMIC_GRM_MASK        0x2        /* 产生PMU中断 */
#define PMU_CMU_CTL_RB_IRQ_FROM_PMIC_GRM_1STICK_MASK 0x2        /* 产生PMU中断 */

/* dump 寄存器定义 */
static ssi_reg_info g_ssi_master_reg_full = { 0x40, 0x0, 0x30, SSI_RW_SSI_MOD };
static ssi_reg_info g_glb_ctrl_full = { 0x40000000, 0x0, 0x08e0, SSI_RW_WORD_MOD };
static ssi_reg_info g_glb_ctrl_extend1 = { 0x40000000, 0x960, 0xac4,   SSI_RW_WORD_MOD };
static ssi_reg_info g_glb_ctrl_extend2 = { 0x40000000, 0x1500, 0x750,   SSI_RW_WORD_MOD };
static ssi_reg_info g_glb_ctrl_extend3 = { 0x40000000, 0x1f50, 0x64,   SSI_RW_WORD_MOD };
static ssi_reg_info g_pmu_cmu_ctrl_full = { 0x40002000, 0x0, 0xf98,  SSI_RW_WORD_MOD };
static ssi_reg_info g_pmu2_cmu_ctrl_part1 = { 0x40012000, 0x0, 0x684,  SSI_RW_WORD_MOD };
static ssi_reg_info g_pmu2_cmu_ctrl_part2 = { 0x40012000, 0xba0, 0x388,  SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part1 = { 0x40011000, 0x0, 0x300,  SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part2 = { 0x40011000, 0x600, 0x3c,  SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part3 = { 0x40011000, 0x700, 0x34,  SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part4 = { 0x40011000, 0x800, 0x28,  SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part5 = { 0x40011000, 0x900, 0x2c,  SSI_RW_WORD_MOD };
static ssi_reg_info g_ipc_ctrl_reg = { 0x4000e000, 0x0, 0xa00,  SSI_RW_WORD_MOD };
static ssi_reg_info g_rf_tcxo_pll_ctrl_part1 = { 0x40014000, 0x0, 0x20, SSI_RW_WORD_MOD };
static ssi_reg_info g_rf_tcxo_pll_ctrl_part2 = { 0x40014000, 0x100, 0x12c, SSI_RW_WORD_MOD };
static ssi_reg_info g_rf_tcxo_pll_ctrl_part3 = { 0x40014000, 0x400, 0x94, SSI_RW_WORD_MOD };
static ssi_reg_info g_gf_ctrl_full = { 0x43000000, 0x0, 0x488,  SSI_RW_WORD_MOD };
static ssi_reg_info g_gf_diag_ctrl_part1 = { 0x43005000, 0x0, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_gf_diag_ctrl_part2 = { 0x43005490, 0x0, 0x10,  SSI_RW_WORD_MOD };
static ssi_reg_info g_gf_rf_abb_ctrl_full = { 0x43016000, 0x0, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_gf_patch_ctrl_full = { 0x43020000, 0x10, 0x8,  SSI_RW_WORD_MOD };
static ssi_reg_info g_gcpu_trace_ctrl_full = { 0x43024000, 0x10, 0x8,  SSI_RW_WORD_MOD };
static ssi_reg_info g_b_ctrl_full = { 0x41000000, 0x0, 0x128,  SSI_RW_WORD_MOD };
static ssi_reg_info g_bf_diag_ctrl_part1 = { 0x41006000, 0x0, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_bf_diag_ctrl_part2 = { 0x41006490, 0x0, 0x10,  SSI_RW_WORD_MOD };
static ssi_reg_info g_bf_rf_abb_ctrl_part1 = { 0x4101a000, 0x0, 0x60,  SSI_RW_WORD_MOD };
static ssi_reg_info g_bf_rf_abb_ctrl_part2 = { 0x4101a000, 0x100, 0xd0,  SSI_RW_WORD_MOD };
static ssi_reg_info g_bf_rf_abb_ctrl_part3 = { 0x4101a000, 0x400, 0x7c,  SSI_RW_WORD_MOD };
static ssi_reg_info g_bf_rf_abb_ctrl_part4 = { 0x4101a000, 0xc00, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_b_patch_ctrl_full = { 0x41020000, 0x10, 0x8,  SSI_RW_WORD_MOD };
static ssi_reg_info g_b_uart_ctrl_full = { 0x41004000, 0x0, 0x4c,  SSI_RW_WORD_MOD };
static ssi_reg_info g_b_pll_ctrl_full = { 0x41013000, 0x0, 0x818,  SSI_RW_WORD_MOD };
static ssi_reg_info g_fm_ctrl_full = { 0x43040000, 0x10, 0x8,  SSI_RW_WORD_MOD };
static ssi_reg_info g_bcpu_trace_ctrl_full = { 0x41024000, 0x10, 0x8,  SSI_RW_WORD_MOD };
static ssi_reg_info g_w_ctrl_part1 = { 0x40105000, 0x0, 0x46c,  SSI_RW_WORD_MOD };
static ssi_reg_info g_w_ctrl_part2 = { 0x40105000, 0x1000, 0x97c,  SSI_RW_WORD_MOD };
static ssi_reg_info g_w_diag_ctrl_part1 = { 0x40120000, 0x108, 0x10,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_w_diag_ctrl_part2 = { 0x40120000, 0xe90, 0x10,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_w_patch_ctrl_full = { 0x400e0000, 0x10, 0x8,  SSI_RW_WORD_MOD };
static ssi_reg_info g_wl_rf_abb_ch_full = { 0x40110000, 0x0, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_wl_rf_abb_com_ctrl_full = { 0x40119000, 0x0, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_wcpu_trace_ctrl_full = { 0x400e4000, 0x10, 0x8,  SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_ctrl_full = { 0x42000000, 0x0, 0x494,  SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_diag_ctrl_part1 = { 0x42005000, 0x0, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_diag_ctrl_part2 = { 0x42005420, 0x0, 0x30,  SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_rf_abb_ctrl_full = { 0x42012000, 0x0, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_trace_ctrl_full = { 0x42020000, 0x10, 0x8,  SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_pll_ctrl_full = { 0x42011000, 0x0, 0x818,  SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_uart_full = { 0x42004000, 0x0, 0x4c,  SSI_RW_WORD_MOD };
static ssi_reg_info g_pcie_ctrl_full = { 0x40108000, 0x0, 0x664,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_host_ctrl_full = { 0x4010d000, 0x0, 0xf78,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_pcie_dbi_full = { 0x40107000, 0x0, 0x1000,  SSI_RW_DWORD_MOD };     /* 没建链之前不能读 */
static ssi_reg_info g_wifi_gpio_full = {0x40005000, 0x0, 0x74, SSI_RW_DWORD_MOD};
static ssi_reg_info g_bfgx_gpio_full = {0x40006000, 0x0, 0x74, SSI_RW_DWORD_MOD};
static ssi_reg_info g_gf_gpio_full = {0x40007000, 0x0, 0x74, SSI_RW_DWORD_MOD};
static ssi_reg_info g_sle_gpio_full = {0x40008000, 0x0, 0x74, SSI_RW_DWORD_MOD};

static ssi_reg_info g_b_int_dis_cnt    = { 0x447024e8, 0x0, 0x4, SSI_RW_DWORD_MOD};
static ssi_reg_info g_host_allow_sleep    = { 0x44707694, 0x0, 0x4, SSI_RW_DWORD_MOD};
static ssi_reg_info g_b_uart_header_mem    = { 0x447037e8, 0x0, 0x10, SSI_RW_DWORD_MOD};
static ssi_reg_info g_b_uart_payload_mem   = { 0x44702571, 0x0, 0x190, SSI_RW_DWORD_MOD};

static ssi_reg_info g_w_trace_mem   = { 0x400e6000, 0x0, 0x2000, SSI_RW_DWORD_MOD};
static ssi_reg_info g_b_trace_mem   = { 0x41026000, 0x0, 0x2000, SSI_RW_DWORD_MOD};
static ssi_reg_info g_gf_trace_mem  = { 0x43026000, 0x0, 0x2000, SSI_RW_DWORD_MOD};
static ssi_reg_info g_sle_trace_mem = { 0x42022000, 0x0, 0x2000, SSI_RW_DWORD_MOD};

static ssi_reg_info g_tcxo_detect_reg1 = { 0x40000000, 0x24, 0x4,  SSI_RW_WORD_MOD };
static ssi_reg_info g_tcxo_detect_reg2 = { 0x40000000, 0x100, 0x28, SSI_RW_WORD_MOD };
static ssi_reg_info g_tcxo_detect_reg3 = { 0x40000000, 0x550, 0xc,  SSI_RW_WORD_MOD };

static ssi_reg_info g_wlan_exception_mesg = { 0x201BE00, 0x0, 0x200,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_bcpu_exception_mesg = { 0x44703800, 0x0, 0x64,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_sle_exception_mesg = { 0x44465A58, 0x0, 0x64,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_gcpu_exception_mesg = { 0x4412FF80, 0x0, 0x64,  SSI_RW_DWORD_MOD };

static ssi_reg_info g_wcpu_ram_exception_mesg = { 0x20002000, 0x0, 0xc,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_wcpu_ictm_header_mesg = { 0x90000, 0x0, 0xc,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_bcpu_ram_exception_mesg = { 0x44702000, 0x0, 0xc,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_bcpu_ictm_header_mesg = { 0x44680000, 0x0, 0xc,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_sle_ram_exception_mesg = { 0x44463000, 0x0, 0xc,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_gcpu_ram_exception_mesg = { 0x44114000, 0x0, 0xc,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_gcpu_ictm_header_mesg = { 0x44050000, 0x0, 0xc,  SSI_RW_DWORD_MOD };

static ssi_reg_info *g_wcpu_ram_exception_full[] = {
    &g_wcpu_ram_exception_mesg,
    &g_wcpu_ictm_header_mesg
};

static ssi_reg_info *g_gcpu_ram_exception_full[] = {
    &g_gcpu_ram_exception_mesg,
    &g_gcpu_ictm_header_mesg
};

static ssi_reg_info *g_bcpu_ram_exception_full[] = {
    &g_bcpu_ram_exception_mesg,
    &g_bcpu_ictm_header_mesg
};

static ssi_reg_info *g_wcpu_reg_full[] = {
    &g_w_ctrl_part1,
    &g_w_ctrl_part2,
    &g_w_patch_ctrl_full,
    &g_wcpu_trace_ctrl_full
};

static ssi_reg_info *g_wcpu_diag_full[] = {
    &g_w_diag_ctrl_part1,
    &g_w_diag_ctrl_part2
};

static ssi_reg_info *g_wcpu_rf_reg[] = {
    &g_wl_rf_abb_ch_full,
    &g_wl_rf_abb_com_ctrl_full
};

static ssi_reg_info *g_bcpu_reg_full[] = {
    &g_b_ctrl_full,
    &g_b_patch_ctrl_full,
    &g_b_uart_ctrl_full,
    &g_bcpu_trace_ctrl_full
};

static ssi_reg_info *g_bcpu_key_mem_full[] = {
    &g_b_int_dis_cnt,
    &g_host_allow_sleep,
    &g_b_uart_header_mem,
    &g_b_uart_payload_mem
};

static ssi_reg_info *g_bcpu_diag_full[] = {
    &g_bf_diag_ctrl_part1,
    &g_bf_diag_ctrl_part2
};

static ssi_reg_info *g_bcpu_rf_reg[] = {
    &g_bf_rf_abb_ctrl_part1,
    &g_bf_rf_abb_ctrl_part2,
    &g_bf_rf_abb_ctrl_part3,
    &g_bf_rf_abb_ctrl_part4,
    &g_b_pll_ctrl_full
};

static ssi_reg_info *g_sle_reg_full[] = {
    &g_sle_ctrl_full,
    &g_sle_trace_ctrl_full,
    &g_sle_uart_full
};

static ssi_reg_info *g_sle_diag_full[] = {
    &g_sle_diag_ctrl_part1,
    &g_sle_diag_ctrl_part2
};

static ssi_reg_info *g_sle_rf_reg[] = {
    &g_sle_pll_ctrl_full,
    &g_sle_rf_abb_ctrl_full
};

static ssi_reg_info *g_gfcpu_reg_full[] = {
    &g_gf_ctrl_full,
    &g_gf_patch_ctrl_full,
    &g_gcpu_trace_ctrl_full,
    &g_fm_ctrl_full
};

static ssi_reg_info *g_gfcpu_diag_full[] = {
    &g_gf_diag_ctrl_part1,
    &g_gf_diag_ctrl_part2
};

static ssi_reg_info *g_gfcpu_rf_reg[] = {
    &g_gf_rf_abb_ctrl_full
};

/* 默认dump 所有寄存器 */
static ssi_reg_info *g_aon_reg_full[] = {
    &g_glb_ctrl_full,
    &g_glb_ctrl_extend1,
    &g_glb_ctrl_extend2,
    &g_glb_ctrl_extend3,
    &g_pmu_cmu_ctrl_full,
    &g_pmu2_cmu_ctrl_part1,
    &g_pmu2_cmu_ctrl_part2,
    &g_coex_ctl_part1,
    &g_coex_ctl_part2,
    &g_coex_ctl_part3,
    &g_coex_ctl_part4,
    &g_coex_ctl_part5,
    &g_ipc_ctrl_reg,
    &g_rf_tcxo_pll_ctrl_part1,
    &g_rf_tcxo_pll_ctrl_part2,
    &g_rf_tcxo_pll_ctrl_part3,
};

static ssi_reg_info *g_tcxo_detect_regs[] = {
    &g_tcxo_detect_reg1,
    &g_tcxo_detect_reg2,
    &g_tcxo_detect_reg3
};

static uint32_t g_irq_from_pmu;

uint32_t pmu_regs_arr[] = {
    /* 先读NP_EVENT寄存器, 下电不丢失 */
    SCP0_NP_EVENT_REG,
    OCP0_NP_EVENT_REG,
    OCP1_NP_EVENT_REG,
    IRQ0_NP_EVENT_REG,
    OCP2_NP_EVENT_REG,
    /* 后读IRQ和IRQ_MASK寄存器, 下电丢失 */
    SCP0_IRQ_REG,
    SCP0_MASK_REG,
    OCP0_IRQ_REG,
    OCP0_MASK_REG,
    OCP1_IRQ_REG,
    OCP1_MASK_REG,
    IRQ0_IRQ_REG,
    IRQ0_MASK_REG,
    OCP2_IRQ_REG,
    OCP2_MASK_REG,
    /* 读BUCK0/SYSLDO/CLDO2寄存器, 下电丢失 */
    WLAN_W_BUCK0_VSET_REG,
    WLAN_W_SYSLDO_VSET_REG,
    WLAN_W_CLDO2_VSET_REG
};

enum cpu {
    WCPU,
    BCPU,
    GCPU,
    SLE,
    CPU_BUFF
};

enum cpu_addr {
    TRACE_PC,
    TRACE_LR,
    TRACE_SP,
    TRACE_BUFF
};

struct trace_part_str {
    char *part;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t value[SSI_CPU_ARM_REG_DUMP_CNT];
};

typedef struct {
    char *name;
    uint32_t cpu_state;
    uint32_t cpu_load_addr;
    uint32_t reg_flag[SSI_CPU_ARM_REG_DUMP_CNT];
    struct  trace_part_str trace_part[TRACE_BUFF];
} ssi_cpu_trace_struct;

STATIC ssi_cpu_trace_struct g_ssi_cpu_trace[CPU_BUFF] = {
    {
        "wcpu",
        0x0,
        GLB_CTL_WCPU_LOAD_REG,
        { 0x0, 0x0 },
        {
            {
                "pc",
                GLB_CTL_WCPU_PC_L_REG,
                GLB_CTL_WCPU_PC_H_REG,
                { 0x0, 0x0 }
            },
            {
                "lr",
                GLB_CTL_WCPU_LR_L_REG,
                GLB_CTL_WCPU_LR_H_REG,
                { 0x0, 0x0 }
            },
            {
                "sp",
                GLB_CTL_WCPU_SP_L_REG,
                GLB_CTL_WCPU_SP_H_REG,
                { 0x0, 0x0 }
            }
        }
    },
    {
        "bcpu",
        0x0,
        GLB_CTL_BCPU_LOAD_REG,
        { 0x0, 0x0 },
        {
            {
                "pc",
                GLB_CTL_BCPU_PC_L_REG,
                GLB_CTL_BCPU_PC_H_REG,
                { 0x0, 0x0 }
            },
            {
                "lr",
                GLB_CTL_BCPU_LR_L_REG,
                GLB_CTL_BCPU_LR_H_REG,
                { 0x0, 0x0 }
            },
            {
                "sp",
                GLB_CTL_BCPU_SP_L_REG,
                GLB_CTL_BCPU_SP_H_REG,
                { 0x0, 0x0 }
            }
        }
    },
    {
        "gcpu",
        0x0,
        GLB_CTL_GCPU_LOAD_REG,
        { 0x0, 0x0 },
        {
            {
                "pc",
                GLB_CTL_GCPU_PC_L_REG,
                GLB_CTL_GCPU_PC_H_REG,
                { 0x0, 0x0 }
            },
            {
                "lr",
                GLB_CTL_GCPU_LR_L_REG,
                GLB_CTL_GCPU_LR_H_REG,
                { 0x0, 0x0 }
            },
            {
                "sp",
                GLB_CTL_GCPU_SP_L_REG,
                GLB_CTL_GCPU_SP_H_REG,
                { 0x0, 0x0 }
            }
        }
    },
    {
        "sle",
        0x0,
        GLB_CTL_SLE_LOAD_REG,
        { 0x0, 0x0 },
        {
            {
                "pc",
                GLB_CTL_SLE_PC_L_REG,
                GLB_CTL_SLE_PC_H_REG,
                { 0x0, 0x0 }
            },
            {
                "lr",
                GLB_CTL_SLE_LR_L_REG,
                GLB_CTL_SLE_LR_H_REG,
                { 0x0, 0x0 }
            },
            {
                "sp",
                0x0,
                0x0,
                { 0x0, 0x0 }
            }
        }
    }
};

uint32_t mp16c_ssi_spmi_read(uint32_t addr)
{
    int32_t ret;
    uint32_t data;

    /* first, reinit spmi_base_addr */
    reinit_spmi_base_addr(SPMI_BASE_ADDR);

    ret = ssi_check_device_isalive();
    if (ret) {
        ps_print_err("please check wifi/bt/gnss is turned on?\n");
        return 0xFFFFFFFF;
    }

    ret = ssi_spmi_read(addr, SPMI_CHANNEL_SLE, SPMI_SLAVEID_HI6506, &data);
    if (ret != OAL_SUCC) {
        ps_print_err("Spmi read data through ssi fail\n");
        return 0xFFFFFFFF;
    }

    return data;
}

void mp16c_ssi_spmi_write(uint32_t addr, uint32_t data)
{
    int32_t ret;

    /* first, reinit spmi_base_addr */
    reinit_spmi_base_addr(SPMI_BASE_ADDR);

    ret = ssi_check_device_isalive();
    if (ret) {
        ps_print_err("please check wifi/bt/gnss is turned on?\n");
        return;
    }

    ret = ssi_spmi_write(addr, SPMI_CHANNEL_SLE, SPMI_SLAVEID_HI6506, data);
    if (ret != OAL_SUCC) {
        ps_print_err("Spmi write data through ssi fail\n");
    }
}

int mp16c_ssi_check_subsys_is_work(void)
{
    uint16_t w_cur_sts = ssi_read16(gpio_ssi_reg(SSI_SSI_RPT_STS_4));
    uint16_t b_cur_sts = ssi_read16(gpio_ssi_reg(SSI_SSI_RPT_STS_6));
    uint16_t g_cur_sts = ssi_read16(gpio_ssi_reg(SSI_SSI_RPT_STS_8));
    uint16_t gle_cur_sts = ssi_read16(gpio_ssi_reg(SSI_SSI_RPT_STS_9));

    ssi_read_master_regs(&g_ssi_master_reg_full, NULL, 0, g_ssi_is_logfile);

    /* w_cur_sts/b_cur_sts/g_cur_sts中的bit[12:10]有一个为0x3, 表示TCXO在位 */
    if (((w_cur_sts & 0x7) == 0x3) || ((b_cur_sts & 0x7) == 0x3) ||          // 0x3表示工作
        ((g_cur_sts & 0x7) == 0x3) || ((gle_cur_sts & 0x3800) == 0x1800)) {
        ps_print_info("ssi get work sys: w_cur_sts = 0x%x, b_cur_sts = 0x%x, g_cur_sts = 0x%x, gle_cur_sts = 0x%x\n",
                      w_cur_sts, b_cur_sts, g_cur_sts, gle_cur_sts);
        return BOARD_SUCC;
    }

    ps_print_info("ssi check all subsys is not working\n");
    return BOARD_FAIL;
}

static void dsm_cpu_info_dump(void)
{
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
    int32_t ret;
    char buf[DSM_CPU_INFO_SIZE] = { 0 };
    /* dsm cpu信息上报 */
    if (g_halt_det_cnt || g_mpxx_kernel_crash) {
        ps_print_info("g_halt_det_cnt=%u g_mpxx_kernel_crash=%d dsm_cpu_info_dump return\n",
                      g_halt_det_cnt, g_mpxx_kernel_crash);
        return;
    }

    ret = snprintf_s(buf, DSM_CPU_INFO_SIZE, DSM_CPU_INFO_SIZE - 1,
        "wcpu:0x%08x 0x%08x 0x%08x 0x%08x s:%1d bcpu:0x%08x 0x%08x 0x%08x 0x%08x s:%1d "\
        "gcpu:0x%08x 0x%08x 0x%08x 0x%08x s:%1d sle:0x%08x 0x%08x 0x%08x 0x%08x s:%1d pmu:%1d",
        (uint32_t)g_ssi_cpu_trace[WCPU].trace_part[TRACE_PC].value[0],
        (uint32_t)g_ssi_cpu_trace[WCPU].trace_part[TRACE_LR].value[0],
        (uint32_t)g_ssi_cpu_trace[WCPU].trace_part[TRACE_PC].value[1],
        (uint32_t)g_ssi_cpu_trace[WCPU].trace_part[TRACE_LR].value[1],
        (uint8_t)g_ssi_cpu_trace[WCPU].cpu_state,
        (uint32_t)g_ssi_cpu_trace[BCPU].trace_part[TRACE_PC].value[0],
        (uint32_t)g_ssi_cpu_trace[BCPU].trace_part[TRACE_LR].value[0],
        (uint32_t)g_ssi_cpu_trace[BCPU].trace_part[TRACE_PC].value[1],
        (uint32_t)g_ssi_cpu_trace[BCPU].trace_part[TRACE_LR].value[1],
        (uint8_t)g_ssi_cpu_trace[BCPU].cpu_state,
        (uint32_t)g_ssi_cpu_trace[GCPU].trace_part[TRACE_PC].value[0],
        (uint32_t)g_ssi_cpu_trace[GCPU].trace_part[TRACE_LR].value[0],
        (uint32_t)g_ssi_cpu_trace[GCPU].trace_part[TRACE_PC].value[1],
        (uint32_t)g_ssi_cpu_trace[GCPU].trace_part[TRACE_LR].value[1],
        (uint8_t)g_ssi_cpu_trace[GCPU].cpu_state,
        (uint32_t)g_ssi_cpu_trace[SLE].trace_part[TRACE_PC].value[0],
        (uint32_t)g_ssi_cpu_trace[SLE].trace_part[TRACE_LR].value[0],
        (uint32_t)g_ssi_cpu_trace[SLE].trace_part[TRACE_PC].value[1],
        (uint32_t)g_ssi_cpu_trace[SLE].trace_part[TRACE_LR].value[1],
        (uint8_t)g_ssi_cpu_trace[SLE].cpu_state,
        (uint8_t)g_irq_from_pmu);
    if (ret > 0) {
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_HALT, "%s\n", buf);
    } else {
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_HALT, "device panic, ssi dump fail[%d]\n", ret);
    }
#endif
}

#define W_EN_HISTORY_LOW (BIT8 | BIT4)
#define B_EN_HISTORY_LOW (BIT9 | BIT5)
#define G_EN_HISTORY_LOW (BIT10 | BIT6)
static void ssi_check_subsys_power_en_status(void)
{
    uint32_t reg;
    reg = (uint32_t)ssi_read32(PMU_CMU_CTL_SYS_STATUS_3_REG);
    if (((reg & W_EN_HISTORY_LOW) == W_EN_HISTORY_LOW) || ((reg & B_EN_HISTORY_LOW) == B_EN_HISTORY_LOW) ||
        ((reg & G_EN_HISTORY_LOW) == G_EN_HISTORY_LOW)) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_GPIO, CHR_PLAT_DRV_ERROR_SUBSYS_POWER_GPIO);
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
            hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_HALT,
                                      "power_en_status[0x%x]: w_en [%s], b_en [%s] g_en [%s]", reg,
                                      ((reg & W_EN_HISTORY_LOW) ==  W_EN_HISTORY_LOW) ? "abnormal" : "normal",
                                      ((reg & B_EN_HISTORY_LOW) ==  B_EN_HISTORY_LOW) ? "abnormal" : "normal",
                                      ((reg & G_EN_HISTORY_LOW) ==  G_EN_HISTORY_LOW) ? "abnormal" : "normal");
#endif
    }

    ps_print_info("power_en_status[0x%x]: w_en [%s], b_en [%s] g_en [%s]", reg,
                  ((reg & W_EN_HISTORY_LOW) ==  W_EN_HISTORY_LOW) ? "abnormal" : "normal",
                  ((reg & B_EN_HISTORY_LOW) ==  B_EN_HISTORY_LOW) ? "abnormal" : "normal",
                  ((reg & G_EN_HISTORY_LOW) ==  G_EN_HISTORY_LOW) ? "abnormal" : "normal");
}

static int ssi_check_cpu_is_working(int sys)
{
    int i;
    uint32_t reg;
    uint32_t mask = 0;
    uint32_t cpu_status[][3] = {
        { WCPU, PMU_CMU_CTL_WCPU_STATUS_MASK, PMU_CMU_CTL_WCPU_STATUS_OFFSET },
        { BCPU, PMU_CMU_CTL_BCPU_STATUS_MASK, PMU_CMU_CTL_BCPU_STATUS_OFFSET },
        { GCPU, PMU_CMU_CTL_GCPU_STATUS_MASK, PMU_CMU_CTL_GCPU_STATUS_OFFSET },
        { SLE,  PMU_CMU_CTL_SLE_STATUS_MASK,  PMU_CMU_CTL_SLE_STATUS_OFFSET }
    };
    reg = (uint32_t)ssi_read32(PMU_CMU_CTL_SYS_STATUS_0_REG);
    ps_print_info("cpu state=0x%x\n", reg);
    for (i = WCPU; i < CPU_BUFF; i++) {
        if (i == sys) {
            mask = ((reg & cpu_status[i][1]) >> cpu_status[i][2]); // 2 is bit offset
            g_ssi_cpu_trace[i].cpu_state = mask;
            ps_print_info("%s is %s\n", g_ssi_cpu_trace[i].name, g_ssi_cpu_st_str[mask]);
        }
    }

    return (mask == 0x3); /* 0x3:cpu is working */
}

static int ssi_check_pcie_status_mp16c(void)
{
    uint32_t ret;
    ret = (uint32_t)ssi_read32(GLB_CTL_RB_AON_PCIE_SOFT_RST_REG);
    if (ssi_check_cpu_is_working(WCPU) &&
       ((ret & GLB_CTL_RB_SOFT_RST_PCIE0_N_MASK) == GLB_CTL_RB_SOFT_RST_PCIE0_N_MASK)) {
        return OAL_SUCC;
    }
    return OAL_FAIL;
}

static int ssi_check_diag_dump_condition(int sys)
{
    uint32_t ret;
    uint32_t clk_en[][2] = {
        { W_CTL_RB_W_PERP_SUB_CLKEN_CFG0_REG, W_CTL_RB_W_DIAG_CTL_CLKEN_MASK },
        { B_CTL_RB_BTOP_CRG_CLOCK_EN_REG, B_CTL_RB_B_DIAG_CTL_CLKEN_MASK },
        { GF_CTL_RB_GFTOP_CRG_CLOCK_EN_REG, GF_CTL_RB_GF_DIAG_CTL_CLKEN_MASK },
        { GLE_CTL_RB_GLE_TOP_CRG_CLOCK_EN_REG, GLE_CTL_RB_GLE_DIAG_CTL_CLKEN_MASK }
    };

    uint32_t soft_rst[][2] = {
        { W_CTL_RB_SOFT_RST_CFG0_REG, W_CTL_RB_SOFT_RST_DIAG_N_MASK },
        { B_CTL_RB_BTOP_CRG_SOFT_RST_REG, B_CTL_RB_SOFT_RST_B_DIAG_CTL_N_MASK },
        { GF_CTL_RB_GFTOP_CRG_SOFT_RST_REG, GF_CTL_RB_SOFT_RST_GF_DIAG_CTL_N_MASK },
        { GLE_CTL_RB_GLE_TOP_CRG_SOFT_RST_REG, GLE_CTL_RB_SOFT_RST_GLE_DIAG_CTL_N_MASK }
    };

    ret = (uint32_t)ssi_read32(clk_en[sys][0]);
    if (ret == BOARD_FAIL) {
        ps_print_err("ssi read diag clk_en fail, ret = %d\n", ret);
        return -OAL_EFAIL;
    }
    if ((ret & clk_en[sys][1]) == 0) {
        ps_print_info("sys [%d] diag clk_en not open\n", sys);
        return -OAL_EFAIL;
    }

    ret = (uint32_t)ssi_read32(soft_rst[sys][0]);
    if (ret == BOARD_FAIL) {
        ps_print_err("ssi read diag soft_rst fail, ret = %d\n", ret);
        return -OAL_EFAIL;
    }
    if ((ret & soft_rst[sys][1]) == 0) {
        ps_print_info("sys [%d] diag soft_rst not open\n", sys);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

static int ssi_check_rf_dump_condition(int sys)
{
    uint32_t ret;
    uint32_t source_en[][2] = {
        { GLB_CTL_RB_AON_TCXO2RF_SOC_CK_CTL_REG, GLB_CTL_RB_TCXO2RF_GOS_CLKEN_MASK },
        { GLB_CTL_RB_AON_TCXO2RF_SOC_CK_CTL_REG, GLB_CTL_RB_TCXO2RF_SOC_CLKEN_MASK },
        { GLB_CTL_RB_AON_TCXO2RF_SOC_CK_CTL_REG, GLB_CTL_RB_TCXO2RF_SOC_CLKEN_MASK },
        { GLB_CTL_RB_AON_CRG_CKEN_GOS_REG, GLB_CTL_RB_TCXO2RF_GOS_CLKEN_MASK }
    };
    uint32_t sub_en[][2] = {
        {0},
        { B_CTL_RB_BTOP_CRG_CLOCK_EN_REG, B_CTL_RB_B_RF_ABB_CTL_CLKEN_MASK },
        { GF_CTL_RB_GFTOP_CRG_CLOCK_EN_REG, GF_CTL_RB_GF_RF_ABB_CTL_CLKEN_MASK }
    };

    ret = (uint32_t)ssi_read32(source_en[sys][0]);
    if (ret == BOARD_FAIL) {
        ps_print_err("ssi read rf source_en_reg fail, ret = %d\n", ret);
        return -OAL_EFAIL;
    }
    if ((ret & source_en[sys][1]) == 0) {
        ps_print_info("sys [%d] source clk_en not open\n", sys);
        return -OAL_EFAIL;
    }

    if ((sys == BCPU) || (sys == GCPU)) {
        ret = (uint32_t)ssi_read32(sub_en[sys][0]);
        if (ret == BOARD_FAIL) {
            ps_print_err("ssi read rf sub_en_reg fail, ret = %d\n", ret);
            return -OAL_EFAIL;
        }
        if ((ret & sub_en[sys][1]) == 0) {
            ps_print_info("sys [%d] sub clk_en not open\n", sys);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

void mp16c_ssi_show_mdpll_lock_status(void)
{
    if (ssi_check_cpu_is_working(BCPU)) {
        if (ssi_check_rf_dump_condition(BCPU) != OAL_SUCC) {
            ps_print_info("BCPU rf reg can't dump\n");
        } else {
            ps_print_warning("BCPU mdpll status = [0x%x]\n", ssi_read_dword(BCPU_PLL_CTRL_MDLL_LOCK_STATUS_REG));
        }
    }

    if (ssi_check_cpu_is_working(SLE)) {
        if (ssi_check_rf_dump_condition(SLE) != OAL_SUCC) {
            ps_print_info("SLE rf reg can't dump\n");
        } else {
            ps_print_warning("SLE mdpll status = [0x%x]\n", ssi_read_dword(SLE_PLL_CTRL_MDLL_LOCK_STATUS_REG));
        }
    }
}

static void ssi_read_pc_lr_sp(int sys)
{
    int i, j;
    uint32_t reg_low, reg_high;
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        for (j = 0; j < TRACE_BUFF; j++) {
            if ((g_ssi_cpu_trace[sys].trace_part[j].addr_low & 0xFFFFFFFF) != 0) {
                reg_low = ssi_read32(g_ssi_cpu_trace[sys].trace_part[j].addr_low);
            } else {
                reg_low = 0;
            }
            if ((g_ssi_cpu_trace[sys].trace_part[j].addr_high & 0xFFFFFFFF) != 0) {
                reg_high = ssi_read32(g_ssi_cpu_trace[sys].trace_part[j].addr_high);
            } else {
                reg_high = 0;
            }
            g_ssi_cpu_trace[sys].trace_part[j].value[i] = reg_low | (reg_high << 16); // offset 16bit
        }

        ps_print_info("gpio-ssi:load_sts:read %s[%d], pc:0x%x, lr:0x%x, sp:0x%x \n", g_ssi_cpu_trace[sys].name, i,
                      g_ssi_cpu_trace[sys].trace_part[TRACE_PC].value[i],
                      g_ssi_cpu_trace[sys].trace_part[TRACE_LR].value[i],
                      g_ssi_cpu_trace[sys].trace_part[TRACE_SP].value[i]);

        if (g_ssi_cpu_trace[sys].trace_part[TRACE_PC].value[i] == 0) {
            ps_print_info("%s pc all zero\n", g_ssi_cpu_trace[sys].name);
        } else {
            g_ssi_cpu_trace[sys].reg_flag[i] = 1;
        }
        oal_mdelay(10); /* delay 10 ms */
    }
}

int mp16c_ssi_read_pc_lr_sp(void)
{
    int i;
    uint32_t load;
    uint32_t ctl_cpu_pc_lrload[][3] = { /* Number of columns in the array is 3 */
        { WCPU, CTL_WCPU_PC_LR_LOAD_EN, CTL_WCPU_PC_LR_CLR_EN },
        { BCPU, CTL_BCPU_PC_LR_LOAD_EN, CTL_BCPU_PC_LR_CLR_EN },
        { GCPU, CTL_GCPU_PC_LR_LOAD_EN, CTL_GCPU_PC_LR_CLR_EN },
        { SLE,  CTL_SLE_PC_LR_LOAD_EN,  CTL_SLE_PC_LR_CLR_EN }
    };

    for (i = WCPU; i < CPU_BUFF; i++) {
        if (ssi_check_cpu_is_working(i) == OAL_FALSE) {
            continue;
        }

        ssi_write32(g_ssi_cpu_trace[i].cpu_load_addr, ctl_cpu_pc_lrload[i][1]);
        oal_mdelay(2); /* delay 2 ms */

        load = (uint32_t)ssi_read32(g_ssi_cpu_trace[i].cpu_load_addr);
        ps_print_info("gpio-ssi:load_sts:load[0x%x] \n", load);

        ssi_read_pc_lr_sp(i);

        // pc锁存 清除使能位:bit1
        ssi_write32(g_ssi_cpu_trace[i].cpu_load_addr, ctl_cpu_pc_lrload[i][2]); // 2 is trce_en
        oal_mdelay(1);
    }
    return 0;
}

typedef struct {
    uint8_t* name;
    uint32_t base_addr;
    uint32_t timer_num;
} mp16c_timer_dump_info;
#define W_RTC_TIMER_BASE_ADDR 0x40009000
#define B_RTC_TIMER_BASE_ADDR 0x4000A000
#define G_RTC_TIMER_BASE_ADDR 0x4000B000
#define SLE_RTC_TIMER_BASE_ADDR 0x4000C000
#define TIMER_LOAD_COUNT        0x00
#define TIMER_CURRENT_VALUE     0x04
#define TIMER_CONTROL_REG       0x08
#define TIMER_N_INT_STATUS      0x10
#define TIMER_ALL_INT_STATUS    0xa0
#define get_timer_n_base_addr(timer_base_addr, idx) ((timer_base_addr) + ((idx) * 0x14))

static void ssi_dump_rtc_timer_regs(unsigned long long module_set)
{
    mp16c_timer_dump_info dump_info_ctrl[CPU_BUFF] = {
        {"W_RTC", W_RTC_TIMER_BASE_ADDR, 4},
        {"B_RTC", B_RTC_TIMER_BASE_ADDR, 4},
        {"G_RTC", G_RTC_TIMER_BASE_ADDR, 4},
        {"GLE_RTC", SLE_RTC_TIMER_BASE_ADDR, 4}
    };
    int i, j;
    uint32_t base_addr_n;
    uint32_t timer_load_cout;
    uint32_t curr_load;
    uint32_t timer_control;
    uint32_t timer_n_int;

    if ((module_set & SSI_MODULE_MASK_TIMER) == 0) {
        return;
    }
    ps_print_info("dump rtc timer info\n");
    for (i = 0; i < CPU_BUFF; i++) {
        uint32_t all_int = ssi_read32((dump_info_ctrl[i].base_addr + TIMER_ALL_INT_STATUS));
        for (j = 0; j < dump_info_ctrl[i].timer_num; j++) {
            base_addr_n = get_timer_n_base_addr(dump_info_ctrl[i].base_addr, j);
            timer_load_cout = ssi_read32((base_addr_n + TIMER_LOAD_COUNT));
            curr_load = ssi_read32((base_addr_n + TIMER_CURRENT_VALUE));
            timer_control = ssi_read32((base_addr_n + TIMER_CONTROL_REG));
            timer_n_int = ssi_read32((base_addr_n + TIMER_N_INT_STATUS));
            ps_print_info("gpio-ssi:[%s][%d],all_int[0x%x],loadcount[%d],current[%d],control[0x%x],int[0x%x]\n",
                          dump_info_ctrl[i].name, j, all_int, timer_load_cout, curr_load, timer_control, timer_n_int);
        }
    }
}

static void ssi_dump_gpio_regs(void)
{
    int ret;

    ps_print_info("wifi wkup host gpio level: %d\n", conn_get_gpio_level(W_WKUP_HOST));
    ret = ssi_read_reg_info(&g_wifi_gpio_full, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_err("ssi dump wlan gpio failed, ret = %d\n", ret);
    }

    ps_print_info("bt wkup host gpio level: %d\n", conn_get_gpio_level(BFGX_WKUP_HOST));
    ret = ssi_read_reg_info(&g_bfgx_gpio_full, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_err("ssi dump wlan gpio failed, ret = %d\n", ret);
    }

    ps_print_info("sle wkup host gpio level: %d\n", conn_get_gpio_level(G_WKUP_HOST));
    ret = ssi_read_reg_info(&g_sle_gpio_full, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_err("ssi dump wlan gpio failed, ret = %d\n", ret);
    }
    ret = ssi_read_reg_info(&g_gf_gpio_full, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_err("ssi dump wlan gpio failed, ret = %d\n", ret);
    }
}

void mp16c_ssi_dump_gpio(void)
{
    ssi_switch_clk(SSI_AON_CLKSEL_SSI);
    ssi_dump_gpio_regs();
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
}

static int ssi_dump_device_aon_regs(unsigned long long module_set)
{
    int ret;
    if (module_set & SSI_MODULE_MASK_AON) {
        ret = ssi_read_reg_info_arry(g_aon_reg_full, sizeof(g_aon_reg_full) / sizeof(ssi_reg_info *),
                                     g_ssi_is_logfile);
        if (ret) {
            return -OAL_EFAIL;
        }
    }

    ssi_dump_gpio_regs();

    return OAL_SUCC;
}

static int ssi_dump_device_arm_regs(unsigned long long module_set)
{
    int ret;
    if (module_set & SSI_MODULE_MASK_ARM_REG) {
        ret = mp16c_ssi_read_pc_lr_sp();
        if (ret) {
            goto ssi_fail;
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static void ssi_dump_device_wcpu_trace_mem(void)
{
    int ret;

    ssi_write32(WIFI_TRACE_PC_RAM_SEL, 0x1);
    ret = ssi_read_reg_info(&g_w_trace_mem, NULL, 0, 0);
    if (ret != 0) {
        ps_print_info("wcpu trace mem read failed\n");
    }
}

static void ssi_dump_device_bcpu_trace_mem(void)
{
    int ret;

    ssi_write32(BT_TRACE_PC_RAM_SEL, 0x1);
    ret = ssi_read_reg_info(&g_b_trace_mem, NULL, 0, 0);
    if (ret) {
        ps_print_info("bcpu trace mem read failed\n");
    }
}

static void ssi_dump_device_gle_trace_mem(void)
{
    int ret;

    ssi_write32(SLE_TRACE_PC_RAM_SEL, 0x1);
    ret = ssi_read_reg_info(&g_sle_trace_mem, NULL, 0, 0);
    if (ret) {
        ps_print_info("sle trace mem read failed, continue try aon\n");
    }
}

static void ssi_dump_device_gf_trace_mem(void)
{
    int ret;

    ssi_write32(GF_TRACE_PC_RAM_SEL, 0x1);
    ret = ssi_read_reg_info(&g_gf_trace_mem, NULL, 0, 0);
    if (ret) {
        ps_print_info("gfcpu trace mem read failed\n");
    }
}

static void ssi_dump_trace_memory(unsigned long long module_set, int32_t cnt)
{
    int32_t total_cnt = cnt;
    /* 仅debug版本, 通过ssi获取cpu_trace结果 */
    if (is_hi110x_debug_type() != OAL_TRUE || total_cnt <= 0) {
        ps_print_info("user mode or maybe beta user or cnt %d zero\n", total_cnt);
        return;
    }

    if ((total_cnt > 0) && ((module_set & SSI_MODULE_MASK_BTRACE) != 0x0) &&
        (ssi_check_cpu_is_working(BCPU) == OAL_TRUE)) {
        ssi_dump_device_bcpu_trace_mem();
        total_cnt--;
    }

    if ((total_cnt > 0) && ((module_set & SSI_MODULE_MASK_GTRACE) != 0x0) &&
        (ssi_check_cpu_is_working(GCPU) == OAL_TRUE)) {
        ssi_dump_device_gf_trace_mem();
        total_cnt--;
    }

    if ((total_cnt > 0) && ((module_set & SSI_MODULE_MASK_GTRACE) != 0x0) &&
        (ssi_check_cpu_is_working(SLE) == OAL_TRUE)) {
        ssi_dump_device_gle_trace_mem();
        total_cnt--;
    }

    if ((total_cnt > 0) && ((module_set & SSI_MODULE_MASK_WTRACE) != 0x0) &&
        (ssi_check_cpu_is_working(WCPU) == OAL_TRUE)) {
        ssi_dump_device_wcpu_trace_mem();
        total_cnt--;
    }
}

static int32_t check_ssi_spmi_is_ok(void)
{
    uint32_t data = mp16c_ssi_spmi_read(CHIP_VERSION_2);
    if (data != CHIP_VERSION_2_DEFAULT_VALUE) {
        ps_print_info("ssi_spmi interface is not normal\n");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/* 检测6506电源异常，读取pmu寄存器 */
static int32_t ssi_dump_power_status_regs(void)
{
    int32_t ret, i;
    uint32_t data;

    /* config spmi pinmux */
    ssi_write_dword(GLB_CTL_RB_PINMUX_CFG_3_REG, 0x1);
    ps_print_err("6506 NP_EVENT IRQ_STS IRQ_MASK and BUCK0_VSET DUMP\n");
    ret = check_ssi_spmi_is_ok();
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    for (i = 0; i < oal_array_size(pmu_regs_arr); i++) {
        data = mp16c_ssi_spmi_read(pmu_regs_arr[i]);
        if (data == 0xFFFFFFFF) {
            return -OAL_EFAIL;
        } else {
            ps_print_err("reg%d: 0x%x = 0x%x\n", i, pmu_regs_arr[i], data);
        }
    }

    return OAL_SUCC;
}

static void ssi_dump_pmu_regs(unsigned long long module_set)
{
    int32_t ret;
    int32_t pmu1_status_grm;
    int32_t pmu1_status_grm_stick;

    g_irq_from_pmu = 0;
    /* try to read pmu regs */
    if ((module_set & SSI_MODULE_MASK_PMU) == 0x0) {
        return;
    }

    pmu1_status_grm = ssi_read32(PMU_CMU_CTL_RB_PMU1_STATUS_GRM_REG);
    if (pmu1_status_grm < 0) {
        return;
    }

    pmu1_status_grm_stick = ssi_read32(PMU_CMU_CTL_RB_PMU1_STATUS_GRM_STICK_REG);
    if (pmu1_status_grm_stick < 0) {
        return;
    }

    if ((((uint32_t)pmu1_status_grm & PMU_CMU_CTL_RB_IRQ_FROM_PMIC_GRM_MASK) ==
        PMU_CMU_CTL_RB_IRQ_FROM_PMIC_GRM_MASK) ||
        (((uint32_t)pmu1_status_grm_stick & PMU_CMU_CTL_RB_IRQ_FROM_PMIC_GRM_1STICK_MASK) ==
         PMU_CMU_CTL_RB_IRQ_FROM_PMIC_GRM_1STICK_MASK)) {
        g_irq_from_pmu = 1;
    }

    ret = ssi_dump_power_status_regs();
    if (ret != OAL_SUCC) {
        ps_print_err("ssi_dump_pmu_regs read np_event failed\n");
        return;
    }
}

static void ssi_trigger_tcxo_detect(uint32_t *tcxo_det_value_target)
{
    if ((*tcxo_det_value_target) == 0) {
        ps_print_err("tcxo_det_value_target is zero, trigger failed\n");
        return;
    }

    /* 刚做过detect,改变det_value+2,观测值是否改变 */
    if ((*tcxo_det_value_target) == ssi_read32(GLB_CTL_TCXO_32K_DET_CNT_REG)) {
        (*tcxo_det_value_target) = (*tcxo_det_value_target) << 1;
    }

    ssi_write32(GLB_CTL_TCXO_32K_DET_CNT_REG, (*tcxo_det_value_target)); /* 设置计数周期 */
    ssi_write32(GLB_CTL_TCXO_DET_CTL_REG, 0x0);                          /* tcxo_det_en disable */

    /* to tcxo */
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    /* delay 150 us */
    oal_udelay(150);
    /* to ssi */
    ssi_switch_clk(SSI_AON_CLKSEL_SSI);

    ssi_write32(GLB_CTL_TCXO_DET_CTL_REG, 0x1); /* tcxo_det_en enable */

    /* to tcxo */
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    /* delay 31 * 2 * tcxo_det_value_taget us.wait detect done,根据设置的计数周期数 */
    oal_udelay(31 * (*tcxo_det_value_target) * 2);
    /* to ssi */
    ssi_switch_clk(SSI_AON_CLKSEL_SSI);
}

static int ssi_detect_32k_handle(uint32_t sys_tick_old, uint64_t cost_time_s, uint32_t *clock_32k)
{
    uint32_t base_32k_clock = RTC_32K_NOMAL_CKL;
    uint32_t sys_tick_new;

    if (ssi_check_cpu_is_working(WCPU)) {
        sys_tick_new = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_W_0_REG);
    } else if (ssi_check_cpu_is_working(BCPU)) {
        sys_tick_new = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_B_0_REG);
    } else if (ssi_check_cpu_is_working(GCPU)) {
        sys_tick_new = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_GF_0_REG);
    } else {
        sys_tick_new = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_SLE_0_REG);
    }

    if (sys_tick_new == sys_tick_old) {
        ps_print_err("32k sys_tick don't change after detect, 32k maybe abnormal, sys_tick=0x%x\n", sys_tick_new);
        return -OAL_EFAIL;
    } else {
        cost_time_s += 1446; /* 经验值,误差1446us */
        (*clock_32k) = (sys_tick_new * 1000) / (uint32_t)cost_time_s; /* 1000 is mean hz to KHz */
        ps_print_info("32k runtime:%llu us, sys_tick:%u\n", cost_time_s, sys_tick_new);
        ps_print_info("32k real= %u Khz[base=%u]\n", (*clock_32k), base_32k_clock);
    }
    return 0;
}

static uint16_t tcxo_freq_det(void)
{
    if (ssi_read32(GLB_CTL_RB_TCXO_FREQ_DET_REG) == BOARD_TCXO_76P8) {
        ps_print_info("board tcxo freq is 76.8M\n");
        return BOARD_TCXO_76P8;
    }
    ps_print_info("board tcxo freq is 38.4M\n");
    return BOARD_TCXO_38P4;
}

static int ssi_detect_tcxo_handle(uint32_t tcxo_det_res_old, uint32_t tcxo_det_value_target, uint32_t clock_32k)
{
    uint64_t base_tcxo_clock = TCXO_38P4_NOMAL_CKL;
    uint32_t base_32k_clock = RTC_32K_NOMAL_CKL;
    uint64_t clock_tcxo, div_clock;
    uint64_t tcxo_limit_low, tcxo_limit_high, tcxo_tmp;
    uint32_t tcxo_det_res_new = (uint32_t)ssi_read32(GLB_CTL_TCXO_32K_DET_RESULT_REG);
    if (tcxo_det_res_new == tcxo_det_res_old) {
        ps_print_err("tcxo don't change after detect, tcxo or 32k maybe abnormal, tcxo_count=0x%x, 32k_clock=%u\n",
                     tcxo_det_res_new, clock_32k);
        return -OAL_EFAIL;
    }

    if (tcxo_det_value_target == 0) {
        ps_print_err("tcxo_det_value_target is zero\n");
        return -OAL_EFAIL;
    }

    if (tcxo_freq_det() == BOARD_TCXO_76P8) {
        base_tcxo_clock = TCXO_76P8_NOMAL_CKL;
        tcxo_det_res_new *= 2;         /* 2：板级晶振如果是76.8M。soc会做2分频处理，因此将soc统计tcxo_cnt乘2，计算板级晶振频率 */
    }
    /* 计算TCXO时钟误差范围 */
    tcxo_tmp = div_u64(base_tcxo_clock, 100); /* 100 is Percentage */
    tcxo_limit_low = (tcxo_tmp * (100 - TCXO_LIMIT_THRESHOLD)); /* TCXO Lower threshold 100 - 5 */
    tcxo_limit_high = (tcxo_tmp * (100 + TCXO_LIMIT_THRESHOLD)); /* TCXO Upper threshold 100 + 5 */

    /* 计算tcxo实际时钟 */
    clock_tcxo = (uint64_t)((tcxo_det_res_new * base_32k_clock) / (tcxo_det_value_target));
    div_clock = clock_tcxo;
    div_clock = div_u64(div_clock, 1000000); /* 1000000 is unit conversion hz to Mhz */
    if ((clock_tcxo < tcxo_limit_low) || (clock_tcxo > tcxo_limit_high)) {
        /* 时钟误差超过阈值 */
#ifdef CONFIG_HUAWEI_DSM
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_TCXO_ERROR,
            "tcxo real=%llu hz,%llu Mhz[base=%llu][limit:%llu~%llu]\n",
            clock_tcxo, div_clock, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high);
#endif
        ps_print_err("tcxo real=%llu hz,%llu Mhz[base=%llu][limit:%llu~%llu]\n",
                     clock_tcxo, div_clock, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high);
        return -OAL_EFAIL;
    }

    ps_print_info("tcxo real=%llu hz,%llu Mhz[base=%llu][limit:%llu~%llu]\n",
                  clock_tcxo, div_clock, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high);
    return 0;
}

static uint32_t ssi_clr_systick(void)
{
    uint32_t sys_tick_old;
    if (ssi_check_cpu_is_working(WCPU)) {
        sys_tick_old = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_W_0_REG);
        ssi_write32(GLB_CTL_SYS_TICK_CFG_W_REG, 0x2); /* 清零w systick */
    } else if (ssi_check_cpu_is_working(BCPU)) {
        sys_tick_old = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_B_0_REG);
        ssi_write32(GLB_CTL_SYS_TICK_CFG_B_REG, 0x2); /* 清零b systick */
    } else if (ssi_check_cpu_is_working(GCPU)) {
        sys_tick_old = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_GF_0_REG);
        ssi_write32(GLB_CTL_SYS_TICK_CFG_GF_REG, 0x2); /* 清零gf systick */
    } else {
        sys_tick_old = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_SLE_0_REG);
        ssi_write32(GLB_CTL_SYS_TICK_CFG_SLE_REG, 0x2); /* 清零sle systick */
    }
    return sys_tick_old;
}

static int ssi_detect_tcxo_is_normal(void)
{
    /*
     * tcxo detect依赖tcxo和32k时钟
     * 如果在启动后tcxo异常, 那么tcxo_det_res为旧值
     * 如果在启动后32k异常, 那么sys_tick为旧值
     */
    int ret;
    uint32_t tcxo_det_present, clock_32k;
    uint32_t sys_tick_old, tcxo_det_res_old;
    uint32_t tcxo_det_value_target = TCXO_32K_DET_VALUE;
    uint64_t us_to_s;

    declare_time_cost_stru(cost);
    if (mp16c_ssi_check_subsys_is_work() == BOARD_FAIL) {
        ps_print_err("All subsys are not work!\n");
        return -OAL_EFAIL;
    }
    /* 检测TCXO时钟是否在位 */
    tcxo_det_present = (uint32_t)ssi_read32(PMU_CMU_CTL_CLOCK_TCXO_PRESENCE_DET);
    if ((tcxo_det_present & PMU_CMU_CTL_RB_XO_DET_VAL_MASK) == PMU_CMU_CTL_RB_XO_DET_VAL_MASK) {
        if ((tcxo_det_present & PMU_CMU_CTL_RB_XO_DET_STS_MASK) == PMU_CMU_CTL_RB_XO_DET_STS_MASK) {
            ps_print_info("tcxo is present after detect\n");
        } else {
#ifdef CONFIG_HUAWEI_DSM
            hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_TCXO_ERROR,
                "tcxo sts:0x%08x abnormal, not present after detect\n", tcxo_det_present);
#endif
            ps_print_err("tcxo sts:0x%x abnormal, not present after detect\n", tcxo_det_present);
            return -OAL_EFAIL;
        }
    } else {
#ifdef CONFIG_HUAWEI_DSM
            hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_TCXO_ERROR,
                "tcxo sts:0x%08x abnormal, invalid detect result\n", tcxo_det_present);
#endif
        ps_print_err("tcxo sts:0x%x abnormal, invalid detect result\n", tcxo_det_present);
        return -OAL_EFAIL;
    }

    /* 检测TCXO时钟精度是否准确 */
    tcxo_det_res_old = (uint32_t)ssi_read32(GLB_CTL_TCXO_32K_DET_RESULT_REG);
    sys_tick_old = ssi_clr_systick();

    oal_get_time_cost_start(cost);

    /* 使能TCXO时钟精度检测 */
    ssi_trigger_tcxo_detect(&tcxo_det_value_target);
    ret = ssi_read_reg_info_arry(g_tcxo_detect_regs, sizeof(g_tcxo_detect_regs) /
                                sizeof(ssi_reg_info *), g_ssi_is_logfile);
    if (ret) {
        return ret;
    }
    oal_udelay(1000); /* delay 1000 us,wait 32k count more */
    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);

    /* 32k detect by system clock */
    us_to_s = time_cost_var_sub(cost);
    ret = ssi_detect_32k_handle(sys_tick_old, us_to_s, &clock_32k);
    if (ret) {
        return ret;
    }

    /* tcxo detect by 32k clock */
    return ssi_detect_tcxo_handle(tcxo_det_res_old, tcxo_det_value_target, clock_32k);
}

static void ssi_dump_device_tcxo_regs(unsigned long long module_set)
{
    int ret;
    if (module_set & (SSI_MODULE_MASK_AON | SSI_MODULE_MASK_AON_CUT)) {
        ret = ssi_detect_tcxo_is_normal();
        if (ret) {
            ps_print_info("tcxo detect failed, continue dump\n");
        }
    }
}

static void ssi_check_en_state(unsigned long long module_set)
{
    int i;
    uint32_t mask;
    uint32_t gpio_type[CPU_BUFF] = {W_POWER, BFGX_POWER, G_POWER, G_POWER};

    if (!(module_set & SSI_MODULE_MASK_ARM_REG)) {
        return;
    }
    for (i = WCPU; i < CPU_BUFF; i++) {
        if (i == GCPU) {
            // 单sle业务场景，gfcpu处于off状态，会误报g_en的异常
            continue;
        }
        mask = g_ssi_cpu_trace[i].cpu_state;
        ps_print_info("gpio_type[%d] is %d, cpu_state is %d\n", gpio_type[i], conn_get_gpio_level(gpio_type[i]), mask);
        // 超级隐私模块断开en信号通路，导致g_en拉高无效，当host侧拉高en，cpu仍然处于off状态上报异常
        if (conn_get_gpio_level(gpio_type[i]) == GPIO_HIGHLEVEL && mask == 0x0) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_GPIO, CHR_PLAT_DRV_ERROR_SUBSYS_POWER_GPIO);
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
            hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_MPXX_HALT,
                                      "host set gpio_type[%d] is high, but %s is OFF", gpio_type[i],
                                      g_ssi_cpu_trace[i].name);
#endif
        }
    }
    ssi_check_subsys_power_en_status();
}

static int regs_dump(unsigned long long module_set)
{
    int ret;

    /* try to read pc&lr&sp regs */
    ret = ssi_dump_device_arm_regs(module_set);
    if (ret) {
        return ret;
    }

    ret = ssi_check_device_isalive();
    if (ret) {
        return ret;
    }

    /* try to read all aon regs */
    ret = ssi_dump_device_aon_regs(module_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* try to read pc&lr&sp regs again */
    ret = ssi_dump_device_arm_regs(module_set);
    if (ret) {
        return ret;
    }
    ssi_check_en_state(module_set);

    /* detect tcxo clock is normal, trigger */
    ssi_dump_device_tcxo_regs(module_set);

    ssi_dump_rtc_timer_regs(module_set);

    return OAL_SUCC;
}

static int ssi_dump_device_wctrl_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_WCTRL) {
        if (ssi_check_cpu_is_working(WCPU)) {
            ret = ssi_read_reg_info_arry(g_wcpu_reg_full, sizeof(g_wcpu_reg_full) /
                                         sizeof(ssi_reg_info *), g_ssi_is_logfile);
            if (ret) {
                return ret;
            }
            if (ssi_check_diag_dump_condition(WCPU) == OAL_SUCC) {
                ret = ssi_read_reg_info_arry(g_wcpu_diag_full, sizeof(g_wcpu_diag_full) /
                                             sizeof(ssi_reg_info *), g_ssi_is_logfile);
            }
            if (ret) {
                ps_print_err("ssi read diag reg fail, ret = %d\n", ret);
            }

            if (ssi_check_rf_dump_condition(WCPU) == OAL_SUCC) {
                ret = ssi_read_reg_info_arry(g_wcpu_rf_reg, sizeof(g_wcpu_rf_reg) /
                                           sizeof(ssi_reg_info *), g_ssi_is_logfile);
            }
            if (ret) {
                ps_print_err("ssi read rf reg fail, ret = %d\n", ret);
            }
        } else {
            ps_print_info("wctrl can't dump, wcpu down\n");
        }
    }
    return 0;
}

static int ssi_dump_device_pciectrl_regs(unsigned long long module_set)
{
    int ret;
    if (ssi_check_cpu_is_working(WCPU)) {
        if (module_set & SSI_MODULE_MASK_PCIE_CFG) {
            ret = ssi_read_reg_info(&g_pcie_ctrl_full, NULL, 0, g_ssi_is_logfile);
            if (ret) {
                return ret;
            }
            ret = ssi_read_reg_info(&g_host_ctrl_full, NULL, 0, g_ssi_is_logfile);
            if (ret) {
                return ret;
            }
        }
    } else {
        ps_print_info("pcie_ctrl can't dump, wcpu down\n");
    }

    /* 需wcpu work，pcie解复位后才可读 */
    if (module_set & SSI_MODULE_MASK_PCIE_DBI) {
        if (ssi_check_pcie_status_mp16c() == OAL_SUCC) {
            ret = ssi_read_reg_info(&g_pcie_dbi_full, NULL, 0, g_ssi_is_logfile);
            if (ret) {
                ps_print_info("pcie dbi regs read failed\n");
            }
        } else {
            ps_print_info("pcie dbi regs can't dump, wcpu maybe down or pcie is reset\n");
        }
    }
    return 0;
}

static int ssi_dump_device_bctrl_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_BCTRL) {
        if (ssi_check_cpu_is_working(BCPU)) {
            ret = ssi_read_reg_info_arry(g_bcpu_reg_full, sizeof(g_bcpu_reg_full) /
                                         sizeof(ssi_reg_info *), g_ssi_is_logfile);
            if (ret) {
                return ret;
            }

            ret = ssi_read_reg_info_arry(g_bcpu_key_mem_full, sizeof(g_bcpu_key_mem_full) /
                                         sizeof(ssi_reg_info *), g_ssi_is_logfile);
            if (ret) {
                ps_print_err("ssi read dev uart mem fail, ret = %d\n", ret);
                return ret;
            }

            if (ssi_check_diag_dump_condition(BCPU) == OAL_SUCC) {
                ret = ssi_read_reg_info_arry(g_bcpu_diag_full, sizeof(g_bcpu_diag_full) /
                                             sizeof(ssi_reg_info *), g_ssi_is_logfile);
            }
            if (ret) {
                ps_print_err("ssi read diag reg fail, ret = %d\n", ret);
            }

            if (ssi_check_rf_dump_condition(BCPU) == OAL_SUCC) {
                ret = ssi_read_reg_info_arry(g_bcpu_rf_reg, sizeof(g_bcpu_rf_reg) /
                                             sizeof(ssi_reg_info *), g_ssi_is_logfile);
            }
            if (ret) {
                ps_print_err("ssi read rf reg fail, ret = %d\n", ret);
            }
        } else {
            ps_print_info("bctrl can't dump, bcpu down\n");
        }
    }
    return 0;
}

static int ssi_dump_device_gfctrl_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_GFCTRL) {
        if (ssi_check_cpu_is_working(GCPU)) {
            ret = ssi_read_reg_info_arry(g_gfcpu_reg_full, sizeof(g_gfcpu_reg_full) /
                                         sizeof(ssi_reg_info *), g_ssi_is_logfile);
            if (ret) {
                return ret;
            }

            if (ssi_check_diag_dump_condition(GCPU) == OAL_SUCC) {
                ret = ssi_read_reg_info_arry(g_gfcpu_diag_full, sizeof(g_gfcpu_diag_full) /
                                             sizeof(ssi_reg_info *), g_ssi_is_logfile);
            }
            if (ret) {
                ps_print_err("ssi read diag reg fail, ret = %d\n", ret);
            }

            if (ssi_check_rf_dump_condition(GCPU) == OAL_SUCC) {
                ret = ssi_read_reg_info_arry(g_gfcpu_rf_reg, sizeof(g_gfcpu_rf_reg) /
                                             sizeof(ssi_reg_info *), g_ssi_is_logfile);
            }
            if (ret) {
                ps_print_err("ssi read rf reg fail, ret = %d\n", ret);
            }
        } else {
            ps_print_info("gfctrl can't dump, gfcpu down\n");
        }
    }
    return 0;
}

static int ssi_dump_device_slectrl_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_SLECTRL) {
        if (ssi_check_cpu_is_working(SLE)) {
            ret = ssi_read_reg_info_arry(g_sle_reg_full, sizeof(g_sle_reg_full) /
                                         sizeof(ssi_reg_info *), g_ssi_is_logfile);
            if (ret) {
                return ret;
            }

            if (ssi_check_diag_dump_condition(SLE) == OAL_SUCC) {
                ret = ssi_read_reg_info_arry(g_sle_diag_full, sizeof(g_sle_diag_full) /
                                             sizeof(ssi_reg_info *), g_ssi_is_logfile);
            }
            if (ret) {
                ps_print_err("ssi read diag reg fail, ret = %d\n", ret);
            }

            if (ssi_check_rf_dump_condition(SLE) == OAL_SUCC) {
                ret = ssi_read_reg_info_arry(g_sle_rf_reg, sizeof(g_sle_rf_reg) /
                                             sizeof(ssi_reg_info *), g_ssi_is_logfile);
            }
            if (ret) {
                ps_print_err("ssi read rf reg fail, ret = %d\n", ret);
            }
        } else {
            ps_print_info("slectrl can't dump, slecpu down\n");
        }
    }
    return 0;
}

static int ssi_dump_device_reg(unsigned long long module_set)
{
    int ret;

    ret = ssi_dump_device_wctrl_regs(module_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = ssi_dump_device_pciectrl_regs(module_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = ssi_dump_device_bctrl_regs(module_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = ssi_dump_device_gfctrl_regs(module_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = ssi_dump_device_slectrl_regs(module_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return ret;
}

static void ssi_dump_device_wcpu_exception_mem(void)
{
    int ret;
    ps_print_info("wcpu exception dump mem start\n");
    if (ssi_check_cpu_is_working(WCPU)) {
        ret = ssi_read_reg_info(&g_wlan_exception_mesg, NULL, 0, 0);
        if (ret != 0) {
            ps_print_info("wcpu exception mem read failed\n");
        }
    } else {
        ps_print_info("wcpu exception mem can't dump, wcpu down\n");
    }
}

static void ssi_dump_device_bcpu_exception_mem(void)
{
    int ret;
    ps_print_info("bcpu exception dump mem start\n");
    if (ssi_check_cpu_is_working(BCPU)) {
        ret = ssi_read_reg_info(&g_bcpu_exception_mesg, NULL, 0, 0);
        if (ret != 0) {
            ps_print_info("bcpu exception mem read failed\n");
        }
    } else {
        ps_print_info("bcpu exception mem can't dump, bcpu down\n");
    }
}

static void ssi_dump_device_sle_exception_mem(void)
{
    int ret;
    ps_print_info("sle exception dump mem start\n");
    if (ssi_check_cpu_is_working(SLE)) {
        ret = ssi_read_reg_info(&g_sle_exception_mesg, NULL, 0, 0);
        if (ret != 0) {
            ps_print_info("sle exception mem read failed\n");
        }
    } else {
        ps_print_info("sle exception mem can't dump, sle down\n");
    }
}

static void ssi_dump_device_gcpu_exception_mem(void)
{
    int ret;
    ps_print_info("gcpu exception dump mem start\n");
    if (ssi_check_cpu_is_working(GCPU)) {
        ret = ssi_read_reg_info(&g_gcpu_exception_mesg, NULL, 0, 0);
        if (ret != 0) {
            ps_print_info("gcpu exception mem read failed\n");
        }
    } else {
        ps_print_info("gcpu exception mem can't dump, gcpu down\n");
    }
}

static void ssi_dump_sle_ram_exception_mem(void)
{
    int ret;
    ps_print_info("sle ram exception dump start\n");
    if (ssi_check_cpu_is_working(SLE)) {
        ret = ssi_read_reg_info(&g_sle_ram_exception_mesg, NULL, 0, 0);
        if (ret != 0) {
            ps_print_info("sle ram exception read failed\n");
        }
    } else {
        ps_print_info("sle ram exception can't dump, sle down\n");
    }
}

static void ssi_dump_wcpu_ram_exception_mem(void)
{
    int ret;
    ps_print_info("wcpu ram exception dump start\n");
    if (ssi_check_cpu_is_working(WCPU)) {
        ret = ssi_read_reg_info_arry(g_wcpu_ram_exception_full, sizeof(g_wcpu_ram_exception_full) /
                                     sizeof(ssi_reg_info *), 0);
        if (ret != 0) {
            ps_print_info("wcpu ram exception read failed\n");
        }
    } else {
        ps_print_info("wcpu ram exception can't dump, wcpu down\n");
    }
}

static void ssi_dump_bcpu_ram_exception_mem(void)
{
    int ret;
    ps_print_info("bcpu ram exception dump start\n");
    if (ssi_check_cpu_is_working(BCPU)) {
        ret = ssi_read_reg_info_arry(g_bcpu_ram_exception_full, sizeof(g_bcpu_ram_exception_full) /
                                     sizeof(ssi_reg_info *), 0);
        if (ret != 0) {
            ps_print_info("bcpu ram exception read failed\n");
        }
    } else {
        ps_print_info("bcpu ram exception can't dump, bcpu down\n");
    }
}

static void ssi_dump_gcpu_ram_exception_mem(void)
{
    int ret;
    ps_print_info("gcpu ram exception dump start\n");
    if (ssi_check_cpu_is_working(GCPU)) {
        ret = ssi_read_reg_info_arry(g_gcpu_ram_exception_full, sizeof(g_gcpu_ram_exception_full) /
                                     sizeof(ssi_reg_info *), 0);
        if (ret != 0) {
            ps_print_info("gcpu ram exception read failed\n");
        }
    } else {
        ps_print_info("gcpu ram exception can't dump, gcpu down\n");
    }
}

static void ssi_dump_ram_exception_msg(unsigned long long module_set)
{
    ssi_dump_wcpu_ram_exception_mem();
    ssi_dump_bcpu_ram_exception_mem();
    ssi_dump_sle_ram_exception_mem();
    ssi_dump_gcpu_ram_exception_mem();
}

static void ssi_dump_arm_exception_msg(unsigned long long module_set)
{
    ssi_dump_device_wcpu_exception_mem();
    ssi_dump_device_bcpu_exception_mem();
    ssi_dump_device_sle_exception_mem();
    ssi_dump_device_gcpu_exception_mem();
}

static int32_t ssi_check_valid(unsigned long long *reg_set)
{
    int32_t ret;

    ret = ssi_check_device_isalive();
    if (ret) {
        /* try to reset aon */
        ssi_force_reset_reg();
        ssi_switch_clk(SSI_AON_CLKSEL_SSI);
        if (ssi_check_device_isalive() < 0) {
            ps_print_info("after reset aon, ssi still can't work\n");
            return OAL_FAIL;
        } else {
            ps_print_info("after reset aon, ssi ok\n");
            *reg_set = SSI_MODULE_MASK_COMM;
        }
    }

    return OAL_SUCC;
}

int mp16c_ssi_device_regs_dump(unsigned long long module_set)
{
    int ret;
    uint64_t dump_trace_cnt;
    unsigned long long reg_set = module_set;
    declare_time_cost_stru(ssi_timestamp);

    ssi_read16(gpio_ssi_reg(SSI_SSI_CTRL));
    ssi_read16(gpio_ssi_reg(SSI_SEL_CTRL));

    ssi_check_tcxo_is_available();
    ssi_switch_clk(SSI_AON_CLKSEL_SSI);

    oal_get_time_cost_start(ssi_timestamp);
    ret = ssi_check_valid(&reg_set);
    if (ret != OAL_SUCC) {
        goto ssi_fail;
    }

    ret = regs_dump(reg_set);
    if (ret != OAL_SUCC) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_reg(reg_set);
    if (ret != OAL_SUCC) {
        goto ssi_fail;
    }

    ssi_dump_arm_exception_msg(module_set);
    ssi_dump_ram_exception_msg(module_set);
    oal_get_time_cost_end(ssi_timestamp);
    oal_calc_time_cost_sub(ssi_timestamp);
    ps_print_info("ssi dump cost %llu us\n", time_cost_var_sub(ssi_timestamp));
    oam_warning_log1(0, OAM_SF_DFR, "mp16c_ssi_device_regs_dump: before cpu trace dump, cost %d us",
                     time_cost_var_sub(ssi_timestamp));
    if ((time_cost_var_sub(ssi_timestamp)) > (SSI_DUMP_LIMIT_TIMEOUT - USEC_PER_SEC)) {
        ps_print_info("ssi dump too long skip cpu trace");
    } else {
        if (SSI_DUMP_LIMIT_TIMEOUT - (time_cost_var_sub(ssi_timestamp)) > USEC_PER_SEC) {
            dump_trace_cnt = SSI_DUMP_LIMIT_TIMEOUT -(time_cost_var_sub(ssi_timestamp));
            do_div(dump_trace_cnt, USEC_PER_SEC);
            ps_print_info("ssi dump trace num %llu\n", dump_trace_cnt);
            ssi_dump_trace_memory(reg_set, (int32_t)dump_trace_cnt);
        }
    }

    oal_get_time_cost_end(ssi_timestamp);
    oal_calc_time_cost_sub(ssi_timestamp);
    oam_warning_log1(0, OAM_SF_DFR, "mp16c_ssi_device_regs_dump: after cpu trace dump, cost %d us",
                     time_cost_var_sub(ssi_timestamp));
    /* try to read pmu regs */
    ssi_dump_pmu_regs(reg_set);

ssi_fail:
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    dsm_cpu_info_dump();
    return ret;
}

#endif /* #ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG */
