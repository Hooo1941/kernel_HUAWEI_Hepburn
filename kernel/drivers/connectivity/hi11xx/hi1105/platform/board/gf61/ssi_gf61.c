/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Implementation of the gf61 ssi host function
 * Author: @CompanyNameTag
 */

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
#include "ssi_gf61.h"

#include "plat_debug.h"
#include "plat_parse_changid.h"
#include "plat_pm.h"
#include "ssi_common.h"
#include "oal_util.h"

#define TCXO_32K_DET_VALUE   10
#define TCXO_LIMIT_THRESHOLD 5
#define TCXO_NOMAL_CKL       38400000
#define RTC_32K_NOMAL_CKL    32768

#define SSI_DUMP_LIMIT_TIMEOUT  4200000 // 超时时间定为4.2s 最低设置为 1s

/* 以下寄存器是gf61 device定义 */
#define GLB_CTL_BASE                              0x40000000
#define GLB_CTL_SOFT_RST_BCPU_REG                 (GLB_CTL_BASE + 0x70)
#define GLB_CTL_SOFT_RST_GCPU_REG                 (GLB_CTL_BASE + 0x74)
#define GLB_CTL_SYS_TICK_CFG_W_REG                (GLB_CTL_BASE + 0x100) /* 写1清零systick，写0无效 */
#define GLB_CTL_SYS_TICK_VALUE_W_0_REG            (GLB_CTL_BASE + 0x104)
#define GLB_CTL_SYS_TICK_CFG_B_REG                (GLB_CTL_BASE + 0x114) /* 写1清零systick，写0无效 */
#define GLB_CTL_SYS_TICK_VALUE_B_0_REG            (GLB_CTL_BASE + 0x118)
#define GLB_CTL_SYS_TICK_CFG_GF_REG               (GLB_CTL_BASE + 0x180) /* 写1清零systick，写0无效 */
#define GLB_CTL_SYS_TICK_VALUE_GF_0_REG           (GLB_CTL_BASE + 0x184)
#define GLB_CTL_SYS_TICK_CFG_SLE_REG              (GLB_CTL_BASE + 0x194) /* 写1清零systick，写0无效 */
#define GLB_CTL_SYS_TICK_VALUE_SLE_0_REG          (GLB_CTL_BASE + 0x198)
#define GLB_CTL_PWR_ON_LABLE_REG                  (GLB_CTL_BASE + 0x200) /* 芯片上电标记寄存器 */
#define GLB_CTL_BCPU_LOAD_REG                     (GLB_CTL_BASE + 0x1080) /* BCPU_LOAD */
#define GLB_CTL_BCPU_PC_L_REG                     (GLB_CTL_BASE + 0x1084) /* BCPU_PC低16bit */
#define GLB_CTL_BCPU_PC_H_REG                     (GLB_CTL_BASE + 0x1088) /* BCPU_PC高16bit */
#define GLB_CTL_BCPU_LR_L_REG                     (GLB_CTL_BASE + 0x108C) /* BCPU_LR低16bit */
#define GLB_CTL_BCPU_LR_H_REG                     (GLB_CTL_BASE + 0x1090) /* BCPU_LR高16bit */
#define GLB_CTL_BCPU_SP_L_REG                     (GLB_CTL_BASE + 0x1094) /* BCPU_SP低16bit */
#define GLB_CTL_BCPU_SP_H_REG                     (GLB_CTL_BASE + 0x1098) /* BCPU_SP高16bit */
#define GLB_CTL_RB_GCPU_PC_TRACE_CFG_REG                 (GLB_CTL_BASE + 0x1F68)
#define GLB_CTL_RB_AON_GCPU0_PC_TRACE_SAMPLE_ENABLE_MASK 0x8
#define GLB_CTL_RB_AON_GCPU1_PC_TRACE_SAMPLE_ENABLE_MASK 0x80
#define GLB_CTL_RB_GCPU_PC_TRACE_INFO_REG                (GLB_CTL_BASE + 0x1F6C)
#define GLB_CTL_RB_GCPU0_BR_PC_REPORT_MUX_L_REG          (GLB_CTL_BASE + 0x1F70)
#define GLB_CTL_RB_GCPU0_BR_PC_REPORT_MUX_H_REG          (GLB_CTL_BASE + 0x1F74)
#define GLB_CTL_RB_GCPU0_PC_REPORT_MUX_L_REG             (GLB_CTL_BASE + 0x1F78)
#define GLB_CTL_RB_GCPU0_PC_REPORT_MUX_H_REG             (GLB_CTL_BASE + 0x1F7C)
#define GLB_CTL_RB_GCPU1_BR_PC_REPORT_MUX_L_REG          (GLB_CTL_BASE + 0x1F80)
#define GLB_CTL_RB_GCPU1_BR_PC_REPORT_MUX_H_REG          (GLB_CTL_BASE + 0x1F84)
#define GLB_CTL_RB_GCPU1_PC_REPORT_MUX_L_REG             (GLB_CTL_BASE + 0x1F88)
#define GLB_CTL_RB_GCPU1_PC_REPORT_MUX_H_REG             (GLB_CTL_BASE + 0x1F8C)
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
#define GLB_CTL_TCXO_DET_CTL_REG                  (GLB_CTL_BASE + 0x550) /* TCXO时钟检测控制寄存器 */
#define GLB_CTL_TCXO_32K_DET_CNT_REG              (GLB_CTL_BASE + 0x554) /* TCXO时钟检测控制寄存器 */
#define GLB_CTL_TCXO_32K_DET_RESULT_REG           (GLB_CTL_BASE + 0x558) /* TCXO时钟检测控制寄存器 */
#define GLB_CTL_WCPU_WAIT_CTL_REG                 (GLB_CTL_BASE + 0x85C)
#define GLB_CTL_BCPU_WAIT_CTL_REG                 (GLB_CTL_BASE + 0x860)
#define GLB_CTL_GFCPU_WAIT_CTL_REG                (GLB_CTL_BASE + 0x864)
#define GLB_CTL_GP_REG16                          (GLB_CTL_BASE + 0x1C10)
#define GLB_CTL_GP_REG31                          (GLB_CTL_BASE + 0x1C4C)

#define PMU_CMU_CTL_BASE                    0x40002000
#define PMU_CMU_CTL_SYS_STATUS_0_REG        (PMU_CMU_CTL_BASE + 0x1C0) /* 系统状态 */
#define PMU_CMU_CTL_CLOCK_TCXO_PRESENCE_DET (PMU_CMU_CTL_BASE + 0x040) /* TCXO时钟在位检测 */

#define WIFI_CPU_TRACE_CTL_RB_BASE                 (0x40024000 + 0xC0000)
#define WIFI_TRACE_CPU_DIAG_CLKEN                  (WIFI_CPU_TRACE_CTL_RB_BASE + 0x108)
#define WIFI_TRACE_PC_RAM_SEL                      (WIFI_CPU_TRACE_CTL_RB_BASE + 0x10C)
#define BT_CPU_TRACE_CTL_RB_BASE                   (0x40024000 + 0x1000000)
#define BT_TRACE_CPU_DIAG_CLKEN                    (BT_CPU_TRACE_CTL_RB_BASE + 0x108)
#define BT_TRACE_PC_RAM_SEL                        (BT_CPU_TRACE_CTL_RB_BASE + 0x10C)
#define SLE_CPU_TRACE_CTL_RB_BASE                  (0x42020000)
#define SLE_TRACE_CPU_DIAG_CLKEN                   (SLE_CPU_TRACE_CTL_RB_BASE + 0x108)
#define SLE_TRACE_PC_RAM_SEL                       (SLE_CPU_TRACE_CTL_RB_BASE + 0x10C)
#define PMU_CMU_CTL_RB_XO_DET_VAL_MASK             0x100    /* TCXO在位检测结果有效 */
#define PMU_CMU_CTL_RB_XO_DET_STS_MASK             0x200    /* TCXO时钟在位 */

static gf61_ssi_cpu_infos g_ssi_cpu_infos;

/* dump 寄存器定义 */
static ssi_reg_info g_ssi_master_reg_full = { 0x40, 0x0, 0x30, SSI_RW_SSI_MOD };
static ssi_reg_info g_glb_ctrl_full = { 0x40000000, 0x0, 0x8e0, SSI_RW_WORD_MOD };
static ssi_reg_info g_glb_ctrl_extend0 = { 0x40000000, 0x968, 0x10, SSI_RW_WORD_MOD };
static ssi_reg_info g_glb_ctrl_extend1 = { 0x40000000, 0x1c00, 0x50, SSI_RW_WORD_MOD };
static ssi_reg_info g_glb_ctrl_extend2 = { 0x40000000, 0x1f64, 0x50, SSI_RW_WORD_MOD };
static ssi_reg_info g_pmu_cmu_ctrl_full = { 0x40002000, 0x0, 0xf98, SSI_RW_WORD_MOD };
static ssi_reg_info g_pmu2_cmu_ctrl_part1 = { 0x40012000, 0x0, 0x684, SSI_RW_WORD_MOD };
static ssi_reg_info g_pmu2_cmu_ctrl_part2 = { 0x40012000, 0xba0, 0x388, SSI_RW_WORD_MOD };
static ssi_reg_info g_efuse_ctrl = { 0x40012900, 0x0, 0x20, SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part1 = { 0x40011000, 0x0, 0x300, SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part2 = { 0x40011000, 0x600, 0x3c, SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part3 = { 0x40011000, 0x700, 0x34, SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part4 = { 0x40011000, 0x800, 0x28, SSI_RW_WORD_MOD };
static ssi_reg_info g_coex_ctl_part5 = { 0x40011000, 0x900, 0x2c, SSI_RW_WORD_MOD };
static ssi_reg_info g_rf_tcxo_pll_ctrl_part1 = { 0x40014000, 0x0, 0x20, SSI_RW_WORD_MOD };
static ssi_reg_info g_rf_tcxo_pll_ctrl_part2 = { 0x40014000, 0x100, 0x12c, SSI_RW_WORD_MOD };
static ssi_reg_info g_rf_tcxo_pll_ctrl_part3 = { 0x40014000, 0x400, 0x94, SSI_RW_WORD_MOD };
static ssi_reg_info g_xoadc_ctrl_full = { 0x40017000, 0x0, 0x20, SSI_RW_DWORD_MOD };
static ssi_reg_info g_b_ctrl_full = { 0x41000000, 0x0, 0x128, SSI_RW_WORD_MOD };
static ssi_reg_info g_bcpu_trace_ctrl_full = { 0x41024000, 0x10, 0x8, SSI_RW_WORD_MOD };
static ssi_reg_info g_w_ctrl_part1 = { 0x40105000, 0x0, 0x46c, SSI_RW_WORD_MOD };
static ssi_reg_info g_w_ctrl_part2 = { 0x40105000, 0x1000, 0x97c, SSI_RW_WORD_MOD };
static ssi_reg_info g_wcpu_trace_ctrl_full = { 0x400e4000, 0x10, 0x8, SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_ctrl_full = { 0x42000000, 0x0, 0x494, SSI_RW_WORD_MOD };
static ssi_reg_info g_sle_trace_ctrl_full = { 0x42020000, 0x10, 0x8, SSI_RW_WORD_MOD };
static ssi_reg_info g_pcie_ctrl_full = { 0x40108000, 0x0, 0x664, SSI_RW_DWORD_MOD };
static ssi_reg_info g_host_ctrl_full = { 0x4010d000, 0x0, 0xf78, SSI_RW_DWORD_MOD };
static ssi_reg_info g_pcie_dbi_full = { 0x40107000, 0x0, 0x1000, SSI_RW_DWORD_MOD }; /* 没建链之前不能读 */
static ssi_reg_info g_wifi_gpio_full = { 0x40005000, 0x0, 0x74, SSI_RW_DWORD_MOD };
static ssi_reg_info g_bfgx_gpio_full = { 0x40006000, 0x0, 0x74, SSI_RW_DWORD_MOD };
static ssi_reg_info g_sle_gpio_full = { 0x40008000, 0x0, 0x74, SSI_RW_DWORD_MOD };

static ssi_reg_info g_w_key_mem = { 0x400e6000, 0x0, 0x2000, SSI_RW_DWORD_MOD };
static ssi_reg_info g_b_key_mem = { 0x41026000, 0x0, 0x2000, SSI_RW_DWORD_MOD };
static ssi_reg_info g_sle_key_mem = { 0x42022000, 0x0, 0x2000, SSI_RW_DWORD_MOD };
static ssi_reg_info g_gcpu0_key_mem = { 0x51c20000, 0x0, 0x1000, SSI_RW_DWORD_MOD };
static ssi_reg_info g_gcpu1_key_mem = { 0x51c21000, 0x0, 0x1000, SSI_RW_DWORD_MOD };

static ssi_reg_info g_tcxo_detect_reg1 = { 0x40000000, 0x24, 0x4, SSI_RW_WORD_MOD };
static ssi_reg_info g_tcxo_detect_reg2 = { 0x40000000, 0x100, 0x28, SSI_RW_WORD_MOD };
static ssi_reg_info g_tcxo_detect_reg3 = { 0x40000000, 0x550, 0xc, SSI_RW_WORD_MOD };

static ssi_reg_info g_wcpu_ram_exception_mesg = { 0x201BE00, 0x0, 0x200,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_bcpu_ram_exception_mesg = { 0x44733600, 0x0, 0x64,  SSI_RW_DWORD_MOD };
static ssi_reg_info g_sle_ram_exception_mesg = { 0x444f4400, 0x0, 0x64,  SSI_RW_DWORD_MOD };

/* gf61 默认dump 所有寄存器 */
static ssi_reg_info *g_aon_reg_full[] = {
    &g_glb_ctrl_full,          &g_glb_ctrl_extend0,       &g_glb_ctrl_extend1,
    &g_glb_ctrl_extend2,       &g_pmu_cmu_ctrl_full,      &g_pmu2_cmu_ctrl_part1,
    &g_pmu2_cmu_ctrl_part2,    &g_coex_ctl_part1,         &g_coex_ctl_part2,
    &g_coex_ctl_part3,         &g_coex_ctl_part4,         &g_coex_ctl_part5,
    &g_wifi_gpio_full,         &g_bfgx_gpio_full,         &g_sle_gpio_full,
    &g_rf_tcxo_pll_ctrl_part1, &g_rf_tcxo_pll_ctrl_part2, &g_rf_tcxo_pll_ctrl_part3,
    &g_xoadc_ctrl_full,        &g_b_ctrl_full,            &g_bcpu_trace_ctrl_full,
    &g_w_ctrl_part1,           &g_w_ctrl_part2,           &g_wcpu_trace_ctrl_full,
    &g_sle_ctrl_full,          &g_sle_trace_ctrl_full,    &g_pcie_ctrl_full,
    &g_host_ctrl_full,         &g_efuse_ctrl,
};

static ssi_reg_info *g_tcxo_detect_regs[] = { &g_tcxo_detect_reg1, &g_tcxo_detect_reg2, &g_tcxo_detect_reg3 };

int gf61_ssi_check_subsys_is_work(void)
{
    uint16_t w_cur_sts = ssi_read16(gpio_ssi_reg(SSI_SSI_RPT_STS_4));
    uint16_t b_cur_sts = ssi_read16(gpio_ssi_reg(SSI_SSI_RPT_STS_6));
    uint16_t g_cur_sts = ssi_read16(gpio_ssi_reg(SSI_SSI_RPT_STS_8));
    uint16_t gle_cur_sts = ssi_read16(gpio_ssi_reg(SSI_SSI_RPT_STS_9));

    ssi_read_master_regs(&g_ssi_master_reg_full, NULL, 0, g_ssi_is_logfile);

    /* w_cur_sts/b_cur_sts/g_cur_sts中的bit[2:0]有一个为0x3, 表示TCXO在位 */
    if (((w_cur_sts & 0x7) == 0x3) || ((b_cur_sts & 0x7) == 0x3) ||
        ((g_cur_sts & 0x7) == 0x3) || ((gle_cur_sts & 0x3800) == 0x1800)) { // 0x3表示工作
        ps_print_info("ssi get work sys: w_cur_sts = 0x%x, b_cur_sts = 0x%x, g_cur_sts = 0x%x, gle_cur_sts = 0x%x\n",
                      w_cur_sts, b_cur_sts, g_cur_sts, gle_cur_sts);
        return BOARD_SUCC;
    }

    ps_print_info("ssi check all subsys is not working\n");
    return BOARD_FAIL;
}

static void dsm_cpu_info_dump(void) {}

static void ssi_check_buck_scp_ocp_status(void)
{
    ps_print_info("ssi_check_buck_scp_ocp_status bypass");
}

static int ssi_check_wcpu_is_working(void)
{
    uint32_t reg, mask;

    /* pilot */
    reg = (uint32_t)ssi_read32(PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = reg & 0x7;
    ps_print_info("cpu state=0x%8x, wcpu is %s\n", reg, g_ssi_cpu_st_str[mask]);
    g_ssi_cpu_infos.wcpu_info.cpu_state = mask;
    if (mask == 0x5) {
        ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

static int ssi_check_bcpu_is_working(void)
{
    uint32_t reg, mask;

    /* pilot */
    reg = (uint32_t)ssi_read32(PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = (reg >> 3) & 0x7; /* reg shift right by 3 bits and 0x7 bits */
    ps_print_info("cpu state=0x%8x, bcpu is %s\n", reg, g_ssi_cpu_st_str[mask]);
    g_ssi_cpu_infos.bcpu_info.cpu_state = mask;
    if (mask == 0x5) {
        ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

static int ssi_check_gcpu_is_working(void)
{
    uint32_t reg, mask;

    /* pilot */
    reg = (uint32_t)ssi_read32(PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = (reg >> 6) & 0x7; /* reg shift right by 6 bits and 0x7 bits */
    ps_print_info("cpu state=0x%8x, gcpu is %s\n", reg, g_ssi_cpu_st_str[mask]);
    g_ssi_cpu_infos.gtcpu0_info.cpu_state = mask;
    if (mask == 0x5) {
        ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

static int ssi_check_sle_is_working(void)
{
    uint32_t reg, mask;

    /* pilot */
    reg = (uint32_t)ssi_read32(PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = (reg >> 9) & 0x7; /* reg shift right by 9 bits and 0x7 bits */
    ps_print_info("cpu state=0x%8x, sle is %s\n", reg, g_ssi_cpu_st_str[mask]);
    g_ssi_cpu_infos.sle_info.cpu_state = mask;
    if (mask == 0x5) {
        ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

int gf61_ssi_read_wcpu_pc_lr_sp(void)
{
    int i;
    uint32_t reg_low, reg_high, load, pc, lr, sp;

    /* read pc twice check whether cpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        // pc锁存 使能位:bit0
        ssi_write32(GLB_CTL_WCPU_LOAD_REG, 0x81);
        oal_mdelay(2); // 2ms
        load = (uint32_t)ssi_read32(GLB_CTL_WCPU_LOAD_REG);

        reg_low = (uint32_t)ssi_read32(GLB_CTL_WCPU_PC_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_WCPU_PC_H_REG);
        pc = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(GLB_CTL_WCPU_LR_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_WCPU_LR_H_REG);
        lr = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(GLB_CTL_WCPU_SP_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_WCPU_SP_H_REG);
        sp = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        // pc锁存 清除使能位:bit1
        ssi_write32(GLB_CTL_WCPU_LOAD_REG, 0x20);
        oal_mdelay(1);

        ps_print_info("gpio-ssi:load_sts:0x%x, read wcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", load, i, pc, lr, sp);

        if (!pc) {
            ps_print_info("wcpu pc all zero\n");
        } else {
            if (g_ssi_cpu_infos.wcpu_info.reg_flag[i] == 0) {
                g_ssi_cpu_infos.wcpu_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.wcpu_info.pc[i] = pc;
                g_ssi_cpu_infos.wcpu_info.lr[i] = lr;
                g_ssi_cpu_infos.wcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10); /* delay 10 ms */
    }
    return 0;
}

int gf61_ssi_read_bcpu_pc_lr_sp(void)
{
    int i;
    uint32_t reg_low, reg_high, load, pc, lr, sp;

    /* read pc twice check whether cpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32(GLB_CTL_BCPU_LOAD_REG, 0x81);
        oal_mdelay(1);
        load = (uint32_t)ssi_read32(GLB_CTL_BCPU_LOAD_REG);

        reg_low = (uint32_t)ssi_read32(GLB_CTL_BCPU_PC_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_BCPU_PC_H_REG);
        pc = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(GLB_CTL_BCPU_LR_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_BCPU_LR_H_REG);
        lr = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(GLB_CTL_BCPU_SP_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_BCPU_SP_H_REG);
        sp = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        ps_print_info("gpio-ssi:read bcpu[%i], load_sts:0x%x, pc:0x%x, lr:0x%x, sp:0x%x \n", i, load, pc, lr, sp);
        if (!pc && !lr && !sp) {
            ps_print_info("bcpu pc lr sp all zero\n");
        } else {
            if (g_ssi_cpu_infos.bcpu_info.reg_flag[i] == 0) {
                g_ssi_cpu_infos.bcpu_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.bcpu_info.pc[i] = pc;
                g_ssi_cpu_infos.bcpu_info.lr[i] = lr;
                g_ssi_cpu_infos.bcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10); /* delay 10 ms */
    }
    return 0;
}

int gf61_ssi_read_gpcu_pc_lr_sp(void)
{
    int i;
    uint32_t reg_low, reg_high;
    uint32_t pc_core0, pc_core1;
    uint32_t br_pc_core0, br_pc_core1;
    uint32_t load;

    /* read pc twice check whether cpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32(GLB_CTL_RB_GCPU_PC_TRACE_CFG_REG, GLB_CTL_RB_AON_GCPU0_PC_TRACE_SAMPLE_ENABLE_MASK);
        oal_mdelay(1);
        load = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU_PC_TRACE_INFO_REG);

        reg_low = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU0_PC_REPORT_MUX_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU0_PC_REPORT_MUX_H_REG);
        pc_core0 = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU1_PC_REPORT_MUX_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU1_PC_REPORT_MUX_H_REG);
        pc_core1 = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU0_BR_PC_REPORT_MUX_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU0_BR_PC_REPORT_MUX_H_REG);
        br_pc_core0 = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU1_BR_PC_REPORT_MUX_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_RB_GCPU1_BR_PC_REPORT_MUX_H_REG);
        br_pc_core1 = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        ps_print_info("gpio-ssi:read gtcpu[%i], load_sts:0x%x pc_core0:0x%x, br_pc_core0:0x%x\n", i, load, pc_core0,
                      br_pc_core0);
        ps_print_info("gpio-ssi: pc_core1:0x%x, br_pc_core1:0x%x,\n", pc_core1, br_pc_core1);
        if ((!pc_core0) && (!pc_core1) && (!br_pc_core0) && (!br_pc_core1)) {
            ps_print_info("gcpu pc lr sp all zero\n");
        } else {
            if (g_ssi_cpu_infos.gtcpu0_info.reg_flag[i] == 0 && g_ssi_cpu_infos.gtcpu1_info.reg_flag[i] == 0) {
                g_ssi_cpu_infos.gtcpu0_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.gtcpu0_info.pc[i] = pc_core0;
                g_ssi_cpu_infos.gtcpu0_info.lr[i] = br_pc_core0;
                g_ssi_cpu_infos.gtcpu0_info.sp[i] = 0;
                g_ssi_cpu_infos.gtcpu1_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.gtcpu1_info.pc[i] = pc_core1;
                g_ssi_cpu_infos.gtcpu1_info.lr[i] = br_pc_core1;
                g_ssi_cpu_infos.gtcpu1_info.sp[i] = 0;
            }
        }
        oal_mdelay(10); /* delay 10 ms */
    }
    return 0;
}

int gf61_ssi_read_sle_pc_lr_sp(void)
{
    int i;
    uint32_t reg_low, reg_high, load, pc, lr;

    /* read pc twice check whether cpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32(GLB_CTL_SLE_LOAD_REG, 0x81);
        oal_mdelay(1);
        load = (uint32_t)ssi_read32(GLB_CTL_SLE_LOAD_REG);

        reg_low = (uint32_t)ssi_read32(GLB_CTL_SLE_PC_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_SLE_PC_H_REG);
        pc = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(GLB_CTL_SLE_LR_L_REG);
        reg_high = (uint32_t)ssi_read32(GLB_CTL_SLE_LR_H_REG);
        lr = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        ps_print_info("gpio-ssi:read gcpu[%i], load_sts:0x%x pc:0x%x, lr:0x%x\n", i, load, pc, lr);
        if (!pc && !lr) {
            ps_print_info("gcpu pc lr all zero\n");
        } else {
            if (g_ssi_cpu_infos.sle_info.reg_flag[i] == 0) {
                g_ssi_cpu_infos.sle_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.sle_info.pc[i] = pc;
                g_ssi_cpu_infos.sle_info.lr[i] = lr;
            }
        }
        oal_mdelay(10); /* delay 10 ms */
    }
    return 0;
}

int gf61_ssi_read_device_arm_register(void)
{
    int32_t ret;

    ret = ssi_check_wcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        gf61_ssi_read_wcpu_pc_lr_sp();
    }
    bfgx_print_subsys_state(BUART);
    ret = ssi_check_bcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        gf61_ssi_read_bcpu_pc_lr_sp();
    }
    ret = ssi_check_gcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        gf61_ssi_read_gpcu_pc_lr_sp();
    }
    ret = ssi_check_sle_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        gf61_ssi_read_sle_pc_lr_sp();
    }

    return 0;
}

void gf61_ssi_dump_gpio_regs(void)
{
    int ret;
    ssi_switch_clk(SSI_AON_CLKSEL_SSI);
    ret = ssi_read_reg_info(&g_wifi_gpio_full, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_err("ssi dump wlan gpio failed, ret = %d\n", ret);
    }

    ret = ssi_read_reg_info(&g_bfgx_gpio_full, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_err("ssi dump bfgx gpio failed, ret = %d\n", ret);
    }

    ret = ssi_read_reg_info(&g_sle_gpio_full, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_err("ssi dump sle gpio failed, ret = %d\n", ret);
    }
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
    /* 建链之前不能读 */
    if (module_set & SSI_MODULE_MASK_PCIE_DBI) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info(&g_pcie_dbi_full, NULL, 0, g_ssi_is_logfile);
            if (ret) {
                ps_print_info("pcie dbi regs read failed, continue try aon\n");
            }
        } else {
            ps_print_info("pcie dbi regs can't dump, wcpu down\n");
        }
    }
    return OAL_SUCC;
}

static int ssi_dump_device_arm_regs(unsigned long long module_set)
{
    int ret;
    if (module_set & SSI_MODULE_MASK_ARM_REG) {
        ret = gf61_ssi_read_device_arm_register();
        if (ret) {
            goto ssi_fail;
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static void ssi_dump_device_wcpu_key_mem(void)
{
    int ret;
    ssi_write32(WIFI_TRACE_CPU_DIAG_CLKEN, 0x3);
    ssi_write32(WIFI_TRACE_PC_RAM_SEL, 0x1);
    ret = ssi_read_reg_info(&g_w_key_mem, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_info("wcpu key mem read failed, continue try aon\n");
    }
}

static void ssi_dump_device_bcpu_key_mem(void)
{
    int ret;
    ssi_write32(BT_TRACE_CPU_DIAG_CLKEN, 0x3);
    ssi_write32(BT_TRACE_PC_RAM_SEL, 0x1);
    ret = ssi_read_reg_info(&g_b_key_mem, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_info("bcpu key mem read failed, continue try aon\n");
    }
}

static void ssi_dump_device_gcpu_key_mem(void)
{
    int ret;
    ret = ssi_read_reg_info(&g_gcpu0_key_mem, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_info("gtcpu0 key mem read failed, continue try aon\n");
    }
    ret = ssi_read_reg_info(&g_gcpu1_key_mem, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_info("gtcpu1 key mem read failed, continue try aon\n");
    }
}

static void ssi_dump_device_sle_key_mem(void)
{
    int ret;
    ssi_write32(SLE_TRACE_CPU_DIAG_CLKEN, 0x3);
    ssi_write32(SLE_TRACE_PC_RAM_SEL, 0x1);
    ret = ssi_read_reg_info(&g_sle_key_mem, NULL, 0, g_ssi_is_logfile);
    if (ret) {
        ps_print_info("sle key mem read failed, continue try aon\n");
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

    if (ssi_check_wcpu_is_working()) {
        sys_tick_new = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_W_0_REG);
    } else if (ssi_check_bcpu_is_working()) {
        sys_tick_new = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_B_0_REG);
    } else if (ssi_check_gcpu_is_working()) {
        sys_tick_new = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_GF_0_REG);
    } else {
        sys_tick_new = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_SLE_0_REG);
    }

    if (sys_tick_new == sys_tick_old) {
        ps_print_err("32k sys_tick don't change after detect, 32k maybe abnormal, sys_tick=0x%x\n", sys_tick_new);
        return -OAL_EFAIL;
    } else {
        cost_time_s += 1446;                                          /* 经验值,误差1446us */
        (*clock_32k) = (sys_tick_new * 1000) / (uint32_t)cost_time_s; /* 1000 is mean hz to KHz */
        ps_print_info("32k runtime:%llu us, sys_tick:%u\n", cost_time_s, sys_tick_new);
        ps_print_info("32k real= %u Khz[base=%u]\n", (*clock_32k), base_32k_clock);
    }
    return 0;
}

static int ssi_detect_tcxo_handle(uint32_t tcxo_det_res_old, uint32_t tcxo_det_value_target, uint32_t clock_32k)
{
    uint64_t base_tcxo_clock = TCXO_NOMAL_CKL;
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

    /* 计算TCXO时钟误差范围 */
    tcxo_tmp = div_u64(base_tcxo_clock, 100);                    /* 100 is Percentage */
    tcxo_limit_low = (tcxo_tmp * (100 - TCXO_LIMIT_THRESHOLD));  /* TCXO Lower threshold 100 - 5 */
    tcxo_limit_high = (tcxo_tmp * (100 + TCXO_LIMIT_THRESHOLD)); /* TCXO Upper threshold 100 + 5 */

    /* 计算tcxo实际时钟 */
    clock_tcxo = (uint64_t)((tcxo_det_res_new * base_32k_clock) / (tcxo_det_value_target));
    div_clock = clock_tcxo;
    div_clock = div_u64(div_clock, 1000000); /* 1000000 is unit conversion hz to Mhz */
    if ((clock_tcxo < tcxo_limit_low) || (clock_tcxo > tcxo_limit_high)) {
        /* 时钟误差超过阈值 */
        ps_print_err("tcxo real=%llu hz,%llu Mhz[base=%llu][limit:%llu~%llu]\n", clock_tcxo, div_clock, base_tcxo_clock,
                     tcxo_limit_low, tcxo_limit_high);
        return -OAL_EFAIL;
    }

    ps_print_info("tcxo real=%llu hz,%llu Mhz[base=%llu][limit:%llu~%llu]\n", clock_tcxo, div_clock, base_tcxo_clock,
                  tcxo_limit_low, tcxo_limit_high);
    return 0;
}

static uint32_t ssi_clr_systick(void)
{
    uint32_t sys_tick_old;
    if (ssi_check_wcpu_is_working()) {
        sys_tick_old = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_W_0_REG);
        ssi_write32(GLB_CTL_SYS_TICK_CFG_W_REG, 0x2); /* 清零w systick */
    } else if (ssi_check_bcpu_is_working()) {
        sys_tick_old = (uint32_t)ssi_read32(GLB_CTL_SYS_TICK_VALUE_B_0_REG);
        ssi_write32(GLB_CTL_SYS_TICK_CFG_B_REG, 0x2); /* 清零b systick */
    } else if (ssi_check_gcpu_is_working()) {
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
    uint32_t tcxo_det_present;
    uint32_t sys_tick_old, tcxo_det_res_old;
    uint32_t tcxo_det_value_target = TCXO_32K_DET_VALUE;
    uint32_t clock_32k;
    uint64_t us_to_s;

    declare_time_cost_stru(cost);

    /* 检测TCXO时钟是否在位 */
    tcxo_det_present = (uint32_t)ssi_read32(PMU_CMU_CTL_CLOCK_TCXO_PRESENCE_DET);
    if ((tcxo_det_present & PMU_CMU_CTL_RB_XO_DET_VAL_MASK) == PMU_CMU_CTL_RB_XO_DET_VAL_MASK) {
        if ((tcxo_det_present & PMU_CMU_CTL_RB_XO_DET_STS_MASK) == PMU_CMU_CTL_RB_XO_DET_STS_MASK) {
            ps_print_info("tcxo is present after detect\n");
        } else {
            ps_print_err("tcxo sts:0x%x maybe abnormal, not present after detect\n", tcxo_det_present);
            return -OAL_EFAIL;
        }
    } else {
        ps_print_err("tcxo sts:0x%x maybe abnormal, invalid detect result\n", tcxo_det_present);
        return -OAL_EFAIL;
    }

    /* 检测TCXO时钟精度是否准确 */
    tcxo_det_res_old = (uint32_t)ssi_read32(GLB_CTL_TCXO_32K_DET_RESULT_REG);
    sys_tick_old = ssi_clr_systick();

    oal_get_time_cost_start(cost);

    /* 使能TCXO时钟精度检测 */
    ssi_trigger_tcxo_detect(&tcxo_det_value_target);
    ret = ssi_read_reg_info_arry(g_tcxo_detect_regs, sizeof(g_tcxo_detect_regs) / sizeof(ssi_reg_info *),
                                 g_ssi_is_logfile);
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

static void ssi_dump_device_wcpu_exception_mem(void)
{
    int ret;
    ps_print_info("wcpu exception dump mem start\n");
    if (ssi_check_wcpu_is_working()) {
        ret = ssi_read_reg_info(&g_wcpu_ram_exception_mesg, NULL, 0, 0);
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
    if (ssi_check_bcpu_is_working()) {
        ret = ssi_read_reg_info(&g_bcpu_ram_exception_mesg, NULL, 0, 0);
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
    if (ssi_check_sle_is_working()) {
        ret = ssi_read_reg_info(&g_sle_ram_exception_mesg, NULL, 0, 0);
        if (ret != 0) {
            ps_print_info("sle exception mem read failed\n");
        }
    } else {
        ps_print_info("sle exception mem can't dump, sle down\n");
    }
}

static void ssi_dump_arm_exception_msg(void)
{
    ssi_dump_device_wcpu_exception_mem();
    ssi_dump_device_bcpu_exception_mem();
    ssi_dump_device_sle_exception_mem();
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

    /* detect tcxo clock is normal, trigger */
    ssi_dump_device_tcxo_regs(module_set);

    return OAL_SUCC;
}

static void ssi_dump_device_key_mem(unsigned long long module_set, int32_t cnt)
{
    int32_t total_cnt = cnt;
    /* 仅debug版本, 通过ssi获取cpu_trace结果 */
    if (is_hi110x_debug_type() != OAL_TRUE || total_cnt <= 0) {
        ps_print_info("user mode or maybe beta user or cnt %d zero\n", total_cnt);
        return;
    }

    /* try to read wcpu key_mem */
    if ((total_cnt > 0) && ((module_set & SSI_MODULE_MASK_WTRACE) != 0x0) &&
        (ssi_check_wcpu_is_working() == OAL_TRUE)) {
        ssi_dump_device_wcpu_key_mem();
        total_cnt--;
    }

    /* try to read bcpu key_mem */
    if ((total_cnt > 0) && ((module_set & SSI_MODULE_MASK_BTRACE) != 0x0) &&
        (ssi_check_bcpu_is_working() == OAL_TRUE)) {
        ssi_dump_device_bcpu_key_mem();
        total_cnt--;
    }

    /* try to read gcpu key_mem */
    if ((total_cnt > 0) && ((module_set & SSI_MODULE_MASK_WCPU_KEY_DTCM) != 0x0) &&
        (ssi_check_gcpu_is_working() == OAL_TRUE)) {
        ssi_dump_device_gcpu_key_mem();
        total_cnt--;
    }

    /* try to read sle key_mem */
    if ((total_cnt > 0) && ((module_set & SSI_MODULE_MASK_GTRACE) != 0x0) &&
        (ssi_check_sle_is_working() == OAL_TRUE)) {
        ssi_dump_device_sle_key_mem();
        total_cnt--;
    }
}

static int gf61_reset_aon(void)
{
    /* try to reset aon */
    ssi_force_reset_reg();
    ssi_switch_clk(SSI_AON_CLKSEL_SSI);
    if (ssi_check_device_isalive() < 0) {
        ps_print_info("after reset aon, ssi still can't work\n");
        return OAL_FAIL;
    }
    ps_print_info("after reset aon, ssi ok, dump acp/ocp reg\n");
    ssi_check_buck_scp_ocp_status();

    return OAL_SUCC;
}

int gf61_ssi_device_regs_dump(unsigned long long module_set)
{
    int ret;
    uint64_t dump_trace_cnt;
    declare_time_cost_stru(ssi_timestamp);
    g_halt_det_cnt = 0;

    memset_s(&g_ssi_cpu_infos, sizeof(g_ssi_cpu_infos), 0, sizeof(g_ssi_cpu_infos));

    ssi_read16(gpio_ssi_reg(SSI_SSI_CTRL));
    ssi_read16(gpio_ssi_reg(SSI_SEL_CTRL));

    ssi_check_tcxo_is_available();
    ssi_switch_clk(SSI_AON_CLKSEL_SSI);

    oal_get_time_cost_start(ssi_timestamp);
    ret = ssi_check_device_isalive();
    if (ret != OAL_SUCC) {
        ret = gf61_reset_aon();
        if (ret != OAL_SUCC) {
            ps_print_warning("gf61_reset_aon fail\n");
            goto ssi_fail;
        }
        module_set = SSI_MODULE_MASK_COMM;
    } else {
        ssi_check_buck_scp_ocp_status();
    }

    ret = regs_dump(module_set);
    if (ret != OAL_SUCC) {
        ps_print_warning("regs_dump fail\n");
        goto ssi_fail;
    }

    ssi_dump_arm_exception_msg();
    oal_get_time_cost_end(ssi_timestamp);
    oal_calc_time_cost_sub(ssi_timestamp);
    ps_print_info("ssi dump cost %llu us\n", time_cost_var_sub(ssi_timestamp));
    if ((time_cost_var_sub(ssi_timestamp)) > (SSI_DUMP_LIMIT_TIMEOUT - USEC_PER_SEC)) {
        ps_print_warning("ssi dump too long skip cpu trace\n");
        goto ssi_fail;
    }
    if (SSI_DUMP_LIMIT_TIMEOUT - (time_cost_var_sub(ssi_timestamp)) > USEC_PER_SEC) {
        dump_trace_cnt = SSI_DUMP_LIMIT_TIMEOUT -(time_cost_var_sub(ssi_timestamp));
        do_div(dump_trace_cnt, USEC_PER_SEC);
        ps_print_info("ssi dump trace num %llu\n", dump_trace_cnt);
        ssi_dump_device_key_mem(module_set, (int32_t)dump_trace_cnt);
    }
    oal_get_time_cost_end(ssi_timestamp);
    oal_calc_time_cost_sub(ssi_timestamp);
    ps_print_info("after trace dump, cust %llu us\n", time_cost_var_sub(ssi_timestamp));

ssi_fail:
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    dsm_cpu_info_dump();
    return ret;
}

int gf61_ssi_test(unsigned int addr, unsigned int value)
{
    int ret;
    unsigned int rd_buff;

    /* 仅通用寄存器支持ssi读写功能测试 */
    if (addr < GLB_CTL_GP_REG16 || addr > GLB_CTL_GP_REG31) {
        ps_print_info("invalid input, please enter a glb_ctl_gp_reg address\n");
        return OAL_FAIL;
    }

    ret = ssi_write32(addr, value);
    if (ret != OAL_SUCC) {
        ps_print_info("ssi write value32 test fail\n");
        return ret;
    }

    rd_buff = ssi_read32(addr);
    if (rd_buff != value) {
        ps_print_info("ssi read value cannot match write value, ssi test fail\n");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
EXPORT_SYMBOL(gf61_ssi_test);

#endif /* #ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG */
