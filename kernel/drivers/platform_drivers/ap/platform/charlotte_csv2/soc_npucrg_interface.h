#ifndef __SOC_NPUCRG_INTERFACE_H__
#define __SOC_NPUCRG_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPUCRG_PEREN0_ADDR(base) ((base) + (0x800UL))
#define SOC_NPUCRG_PERDIS0_ADDR(base) ((base) + (0x804UL))
#define SOC_NPUCRG_PERCLKEN0_ADDR(base) ((base) + (0x808UL))
#define SOC_NPUCRG_PERSTAT0_ADDR(base) ((base) + (0x80CUL))
#define SOC_NPUCRG_PEREN1_ADDR(base) ((base) + (0x810UL))
#define SOC_NPUCRG_PERDIS1_ADDR(base) ((base) + (0x814UL))
#define SOC_NPUCRG_PERCLKEN1_ADDR(base) ((base) + (0x818UL))
#define SOC_NPUCRG_PERSTAT1_ADDR(base) ((base) + (0x81CUL))
#define SOC_NPUCRG_PEREN2_ADDR(base) ((base) + (0x820UL))
#define SOC_NPUCRG_PERDIS2_ADDR(base) ((base) + (0x824UL))
#define SOC_NPUCRG_PERCLKEN2_ADDR(base) ((base) + (0x828UL))
#define SOC_NPUCRG_PERSTAT2_ADDR(base) ((base) + (0x82CUL))
#define SOC_NPUCRG_PEREN3_ADDR(base) ((base) + (0x830UL))
#define SOC_NPUCRG_PERDIS3_ADDR(base) ((base) + (0x834UL))
#define SOC_NPUCRG_PERCLKEN3_ADDR(base) ((base) + (0x838UL))
#define SOC_NPUCRG_PERSTAT3_ADDR(base) ((base) + (0x83CUL))
#define SOC_NPUCRG_PERRSTEN0_ADDR(base) ((base) + (0x840UL))
#define SOC_NPUCRG_PERRSTDIS0_ADDR(base) ((base) + (0x844UL))
#define SOC_NPUCRG_PERRSTSTAT0_ADDR(base) ((base) + (0x848UL))
#define SOC_NPUCRG_PERRSTEN1_ADDR(base) ((base) + (0x84cUL))
#define SOC_NPUCRG_PERRSTDIS1_ADDR(base) ((base) + (0x850UL))
#define SOC_NPUCRG_PERRSTSTAT1_ADDR(base) ((base) + (0x854UL))
#define SOC_NPUCRG_PERRSTEN2_ADDR(base) ((base) + (0x858UL))
#define SOC_NPUCRG_PERRSTDIS2_ADDR(base) ((base) + (0x85cUL))
#define SOC_NPUCRG_PERRSTSTAT2_ADDR(base) ((base) + (0x860UL))
#define SOC_NPUCRG_CLKDIV0_ADDR(base) ((base) + (0x870UL))
#define SOC_NPUCRG_CLKDIV3_ADDR(base) ((base) + (0x874UL))
#define SOC_NPUCRG_CLKDIV1_ADDR(base) ((base) + (0x878UL))
#define SOC_NPUCRG_CLKDIV2_ADDR(base) ((base) + (0x87CUL))
#define SOC_NPUCRG_PERI_STAT0_ADDR(base) ((base) + (0x880UL))
#define SOC_NPUCRG_PERI_CTRL0_ADDR(base) ((base) + (0x884UL))
#define SOC_NPUCRG_IPCLKRST_BYPASS_ADDR(base) ((base) + (0x890UL))
#define SOC_NPUCRG_IPCLKRST_BYPASS1_ADDR(base) ((base) + (0x894UL))
#define SOC_NPUCRG_PERI_AUTODIV0_ADDR(base) ((base) + (0x8A0UL))
#define SOC_NPUCRG_PERI_AUTODIV1_ADDR(base) ((base) + (0x8A4UL))
#define SOC_NPUCRG_PERI_AUTODIV2_ADDR(base) ((base) + (0x8A8UL))
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_ADDR(base) ((base) + (0x8ACUL))
#define SOC_NPUCRG_PERI_AUTODIV3_ADDR(base) ((base) + (0x8B0UL))
#define SOC_NPUCRG_PERI_AUTODIV5_ADDR(base) ((base) + (0x8B8UL))
#define SOC_NPUCRG_PERI_AUTODIV8_ADDR(base) ((base) + (0x8C4UL))
#define SOC_NPUCRG_PERI_AUTODIV9_ADDR(base) ((base) + (0x8C8UL))
#define SOC_NPUCRG_PERI_AUTODIV11_ADDR(base) ((base) + (0x8D0UL))
#define SOC_NPUCRG_PPLL5CTRL0_ADDR(base) ((base) + (0x998UL))
#define SOC_NPUCRG_PPLL5CTRL1_ADDR(base) ((base) + (0x99CUL))
#define SOC_NPUCRG_PPLL5SSCCTRL_ADDR(base) ((base) + (0xA08UL))
#define SOC_NPUCRG_PLL_VOTE_STAT_ADDR(base) ((base) + (0xA0CUL))
#define SOC_NPUCRG_PLL_EN_VOTE_CTRL_ADDR(base) ((base) + (0xA10UL))
#define SOC_NPUCRG_PLL_GT_VOTE_CTRL_ADDR(base) ((base) + (0xA14UL))
#define SOC_NPUCRG_PLL_BYPASS_VOTE_CTRL_ADDR(base) ((base) + (0xA18UL))
#define SOC_NPUCRG_FNPLL_STAT0_ADDR(base) ((base) + (0xA20UL))
#define SOC_NPUCRG_FNPLL_CFG0_ADDR(base) ((base) + (0xA28UL))
#define SOC_NPUCRG_FNPLL_CFG1_ADDR(base) ((base) + (0xA2CUL))
#define SOC_NPUCRG_FNPLL_CFG2_ADDR(base) ((base) + (0xA30UL))
#define SOC_NPUCRG_FNPLL_CFG3_ADDR(base) ((base) + (0xA34UL))
#define SOC_NPUCRG_VS_CTRL_EN_NPU_ADDR(base) ((base) + (0xA40UL))
#define SOC_NPUCRG_VS_CTRL_BYPASS_NPU_ADDR(base) ((base) + (0xA44UL))
#define SOC_NPUCRG_VS_CTRL_NPU_ADDR(base) ((base) + (0xA48UL))
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_ADDR(base) ((base) + (0xA4CUL))
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_ADDR(base) ((base) + (0xA50UL))
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_ADDR(base) ((base) + (0xA54UL))
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_ADDR(base) ((base) + (0xA58UL))
#define SOC_NPUCRG_VS_TEST_STAT_NPU_ADDR(base) ((base) + (0xA5CUL))
#define SOC_NPUCRG_VS_PCR_DIV0_ADDR(base) ((base) + (0xA60UL))
#define SOC_NPUCRG_VS_PCR_DIV1_ADDR(base) ((base) + (0xA64UL))
#define SOC_NPUCRG_VS_PCR_DIV2_ADDR(base) ((base) + (0xA68UL))
#define SOC_NPUCRG_VS_PCR_DIV3_ADDR(base) ((base) + (0xA6CUL))
#define SOC_NPUCRG_VS_SVFD_CTRL3_NPU_ADDR(base) ((base) + (0xA80UL))
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_ADDR(base) ((base) + (0xD00UL))
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_ADDR(base) ((base) + (0xD04UL))
#define SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_ADDR(base) ((base) + (0xD08UL))
#define SOC_NPUCRG_PLL_FSM_CTRL0_ADDR(base) ((base) + (0xD10UL))
#define SOC_NPUCRG_PLL_FSM_CTRL1_ADDR(base) ((base) + (0xD14UL))
#define SOC_NPUCRG_PLL_FSM_STAT_ADDR(base) ((base) + (0xD18UL))
#define SOC_NPUCRG_PLL_FSM_NS_VOTE0_ADDR(base) ((base) + (0x21CUL))
#define SOC_NPUCRG_PLL_FSM_NS_STAT_ADDR(base) ((base) + (0x0220UL))
#else
#define SOC_NPUCRG_PEREN0_ADDR(base) ((base) + (0x800))
#define SOC_NPUCRG_PERDIS0_ADDR(base) ((base) + (0x804))
#define SOC_NPUCRG_PERCLKEN0_ADDR(base) ((base) + (0x808))
#define SOC_NPUCRG_PERSTAT0_ADDR(base) ((base) + (0x80C))
#define SOC_NPUCRG_PEREN1_ADDR(base) ((base) + (0x810))
#define SOC_NPUCRG_PERDIS1_ADDR(base) ((base) + (0x814))
#define SOC_NPUCRG_PERCLKEN1_ADDR(base) ((base) + (0x818))
#define SOC_NPUCRG_PERSTAT1_ADDR(base) ((base) + (0x81C))
#define SOC_NPUCRG_PEREN2_ADDR(base) ((base) + (0x820))
#define SOC_NPUCRG_PERDIS2_ADDR(base) ((base) + (0x824))
#define SOC_NPUCRG_PERCLKEN2_ADDR(base) ((base) + (0x828))
#define SOC_NPUCRG_PERSTAT2_ADDR(base) ((base) + (0x82C))
#define SOC_NPUCRG_PEREN3_ADDR(base) ((base) + (0x830))
#define SOC_NPUCRG_PERDIS3_ADDR(base) ((base) + (0x834))
#define SOC_NPUCRG_PERCLKEN3_ADDR(base) ((base) + (0x838))
#define SOC_NPUCRG_PERSTAT3_ADDR(base) ((base) + (0x83C))
#define SOC_NPUCRG_PERRSTEN0_ADDR(base) ((base) + (0x840))
#define SOC_NPUCRG_PERRSTDIS0_ADDR(base) ((base) + (0x844))
#define SOC_NPUCRG_PERRSTSTAT0_ADDR(base) ((base) + (0x848))
#define SOC_NPUCRG_PERRSTEN1_ADDR(base) ((base) + (0x84c))
#define SOC_NPUCRG_PERRSTDIS1_ADDR(base) ((base) + (0x850))
#define SOC_NPUCRG_PERRSTSTAT1_ADDR(base) ((base) + (0x854))
#define SOC_NPUCRG_PERRSTEN2_ADDR(base) ((base) + (0x858))
#define SOC_NPUCRG_PERRSTDIS2_ADDR(base) ((base) + (0x85c))
#define SOC_NPUCRG_PERRSTSTAT2_ADDR(base) ((base) + (0x860))
#define SOC_NPUCRG_CLKDIV0_ADDR(base) ((base) + (0x870))
#define SOC_NPUCRG_CLKDIV3_ADDR(base) ((base) + (0x874))
#define SOC_NPUCRG_CLKDIV1_ADDR(base) ((base) + (0x878))
#define SOC_NPUCRG_CLKDIV2_ADDR(base) ((base) + (0x87C))
#define SOC_NPUCRG_PERI_STAT0_ADDR(base) ((base) + (0x880))
#define SOC_NPUCRG_PERI_CTRL0_ADDR(base) ((base) + (0x884))
#define SOC_NPUCRG_IPCLKRST_BYPASS_ADDR(base) ((base) + (0x890))
#define SOC_NPUCRG_IPCLKRST_BYPASS1_ADDR(base) ((base) + (0x894))
#define SOC_NPUCRG_PERI_AUTODIV0_ADDR(base) ((base) + (0x8A0))
#define SOC_NPUCRG_PERI_AUTODIV1_ADDR(base) ((base) + (0x8A4))
#define SOC_NPUCRG_PERI_AUTODIV2_ADDR(base) ((base) + (0x8A8))
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_ADDR(base) ((base) + (0x8AC))
#define SOC_NPUCRG_PERI_AUTODIV3_ADDR(base) ((base) + (0x8B0))
#define SOC_NPUCRG_PERI_AUTODIV5_ADDR(base) ((base) + (0x8B8))
#define SOC_NPUCRG_PERI_AUTODIV8_ADDR(base) ((base) + (0x8C4))
#define SOC_NPUCRG_PERI_AUTODIV9_ADDR(base) ((base) + (0x8C8))
#define SOC_NPUCRG_PERI_AUTODIV11_ADDR(base) ((base) + (0x8D0))
#define SOC_NPUCRG_PPLL5CTRL0_ADDR(base) ((base) + (0x998))
#define SOC_NPUCRG_PPLL5CTRL1_ADDR(base) ((base) + (0x99C))
#define SOC_NPUCRG_PPLL5SSCCTRL_ADDR(base) ((base) + (0xA08))
#define SOC_NPUCRG_PLL_VOTE_STAT_ADDR(base) ((base) + (0xA0C))
#define SOC_NPUCRG_PLL_EN_VOTE_CTRL_ADDR(base) ((base) + (0xA10))
#define SOC_NPUCRG_PLL_GT_VOTE_CTRL_ADDR(base) ((base) + (0xA14))
#define SOC_NPUCRG_PLL_BYPASS_VOTE_CTRL_ADDR(base) ((base) + (0xA18))
#define SOC_NPUCRG_FNPLL_STAT0_ADDR(base) ((base) + (0xA20))
#define SOC_NPUCRG_FNPLL_CFG0_ADDR(base) ((base) + (0xA28))
#define SOC_NPUCRG_FNPLL_CFG1_ADDR(base) ((base) + (0xA2C))
#define SOC_NPUCRG_FNPLL_CFG2_ADDR(base) ((base) + (0xA30))
#define SOC_NPUCRG_FNPLL_CFG3_ADDR(base) ((base) + (0xA34))
#define SOC_NPUCRG_VS_CTRL_EN_NPU_ADDR(base) ((base) + (0xA40))
#define SOC_NPUCRG_VS_CTRL_BYPASS_NPU_ADDR(base) ((base) + (0xA44))
#define SOC_NPUCRG_VS_CTRL_NPU_ADDR(base) ((base) + (0xA48))
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_ADDR(base) ((base) + (0xA4C))
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_ADDR(base) ((base) + (0xA50))
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_ADDR(base) ((base) + (0xA54))
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_ADDR(base) ((base) + (0xA58))
#define SOC_NPUCRG_VS_TEST_STAT_NPU_ADDR(base) ((base) + (0xA5C))
#define SOC_NPUCRG_VS_PCR_DIV0_ADDR(base) ((base) + (0xA60))
#define SOC_NPUCRG_VS_PCR_DIV1_ADDR(base) ((base) + (0xA64))
#define SOC_NPUCRG_VS_PCR_DIV2_ADDR(base) ((base) + (0xA68))
#define SOC_NPUCRG_VS_PCR_DIV3_ADDR(base) ((base) + (0xA6C))
#define SOC_NPUCRG_VS_SVFD_CTRL3_NPU_ADDR(base) ((base) + (0xA80))
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_ADDR(base) ((base) + (0xD00))
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_ADDR(base) ((base) + (0xD04))
#define SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_ADDR(base) ((base) + (0xD08))
#define SOC_NPUCRG_PLL_FSM_CTRL0_ADDR(base) ((base) + (0xD10))
#define SOC_NPUCRG_PLL_FSM_CTRL1_ADDR(base) ((base) + (0xD14))
#define SOC_NPUCRG_PLL_FSM_STAT_ADDR(base) ((base) + (0xD18))
#define SOC_NPUCRG_PLL_FSM_NS_VOTE0_ADDR(base) ((base) + (0x21C))
#define SOC_NPUCRG_PLL_FSM_NS_STAT_ADDR(base) ((base) + (0x0220))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ai_core0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_aicore0_dw : 1;
        unsigned int gt_clk_aicore0_smmu : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int gt_clk_npu_tcu : 1;
        unsigned int reserved_12 : 1;
        unsigned int gt_clk_aicore0_hpm : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int gt_clk_qice_npu_data : 1;
        unsigned int gt_clk_qice_gm_gs : 1;
        unsigned int reserved_15 : 1;
        unsigned int gt_clk_qic_ts : 1;
        unsigned int gt_clk_qic_tcu : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int gt_clk_qic_aicore0 : 1;
        unsigned int reserved_19 : 1;
        unsigned int gt_clk_npu_pa : 1;
        unsigned int gt_clk_npu_hpm : 1;
    } reg;
} SOC_NPUCRG_PEREN0_UNION;
#endif
#define SOC_NPUCRG_PEREN0_gt_clk_ai_core0_START (0)
#define SOC_NPUCRG_PEREN0_gt_clk_ai_core0_END (0)
#define SOC_NPUCRG_PEREN0_gt_clk_aicore0_dw_START (8)
#define SOC_NPUCRG_PEREN0_gt_clk_aicore0_dw_END (8)
#define SOC_NPUCRG_PEREN0_gt_clk_aicore0_smmu_START (9)
#define SOC_NPUCRG_PEREN0_gt_clk_aicore0_smmu_END (9)
#define SOC_NPUCRG_PEREN0_gt_clk_npu_tcu_START (15)
#define SOC_NPUCRG_PEREN0_gt_clk_npu_tcu_END (15)
#define SOC_NPUCRG_PEREN0_gt_clk_aicore0_hpm_START (17)
#define SOC_NPUCRG_PEREN0_gt_clk_aicore0_hpm_END (17)
#define SOC_NPUCRG_PEREN0_gt_clk_qice_npu_data_START (20)
#define SOC_NPUCRG_PEREN0_gt_clk_qice_npu_data_END (20)
#define SOC_NPUCRG_PEREN0_gt_clk_qice_gm_gs_START (21)
#define SOC_NPUCRG_PEREN0_gt_clk_qice_gm_gs_END (21)
#define SOC_NPUCRG_PEREN0_gt_clk_qic_ts_START (23)
#define SOC_NPUCRG_PEREN0_gt_clk_qic_ts_END (23)
#define SOC_NPUCRG_PEREN0_gt_clk_qic_tcu_START (24)
#define SOC_NPUCRG_PEREN0_gt_clk_qic_tcu_END (24)
#define SOC_NPUCRG_PEREN0_gt_clk_qic_aicore0_START (28)
#define SOC_NPUCRG_PEREN0_gt_clk_qic_aicore0_END (28)
#define SOC_NPUCRG_PEREN0_gt_clk_npu_pa_START (30)
#define SOC_NPUCRG_PEREN0_gt_clk_npu_pa_END (30)
#define SOC_NPUCRG_PEREN0_gt_clk_npu_hpm_START (31)
#define SOC_NPUCRG_PEREN0_gt_clk_npu_hpm_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ai_core0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_aicore0_dw : 1;
        unsigned int gt_clk_aicore0_smmu : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int gt_clk_npu_tcu : 1;
        unsigned int reserved_12 : 1;
        unsigned int gt_clk_aicore0_hpm : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int gt_clk_qice_npu_data : 1;
        unsigned int gt_clk_qice_gm_gs : 1;
        unsigned int reserved_15 : 1;
        unsigned int gt_clk_qic_ts : 1;
        unsigned int gt_clk_qic_tcu : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int gt_clk_qic_aicore0 : 1;
        unsigned int reserved_19 : 1;
        unsigned int gt_clk_npu_pa : 1;
        unsigned int gt_clk_npu_hpm : 1;
    } reg;
} SOC_NPUCRG_PERDIS0_UNION;
#endif
#define SOC_NPUCRG_PERDIS0_gt_clk_ai_core0_START (0)
#define SOC_NPUCRG_PERDIS0_gt_clk_ai_core0_END (0)
#define SOC_NPUCRG_PERDIS0_gt_clk_aicore0_dw_START (8)
#define SOC_NPUCRG_PERDIS0_gt_clk_aicore0_dw_END (8)
#define SOC_NPUCRG_PERDIS0_gt_clk_aicore0_smmu_START (9)
#define SOC_NPUCRG_PERDIS0_gt_clk_aicore0_smmu_END (9)
#define SOC_NPUCRG_PERDIS0_gt_clk_npu_tcu_START (15)
#define SOC_NPUCRG_PERDIS0_gt_clk_npu_tcu_END (15)
#define SOC_NPUCRG_PERDIS0_gt_clk_aicore0_hpm_START (17)
#define SOC_NPUCRG_PERDIS0_gt_clk_aicore0_hpm_END (17)
#define SOC_NPUCRG_PERDIS0_gt_clk_qice_npu_data_START (20)
#define SOC_NPUCRG_PERDIS0_gt_clk_qice_npu_data_END (20)
#define SOC_NPUCRG_PERDIS0_gt_clk_qice_gm_gs_START (21)
#define SOC_NPUCRG_PERDIS0_gt_clk_qice_gm_gs_END (21)
#define SOC_NPUCRG_PERDIS0_gt_clk_qic_ts_START (23)
#define SOC_NPUCRG_PERDIS0_gt_clk_qic_ts_END (23)
#define SOC_NPUCRG_PERDIS0_gt_clk_qic_tcu_START (24)
#define SOC_NPUCRG_PERDIS0_gt_clk_qic_tcu_END (24)
#define SOC_NPUCRG_PERDIS0_gt_clk_qic_aicore0_START (28)
#define SOC_NPUCRG_PERDIS0_gt_clk_qic_aicore0_END (28)
#define SOC_NPUCRG_PERDIS0_gt_clk_npu_pa_START (30)
#define SOC_NPUCRG_PERDIS0_gt_clk_npu_pa_END (30)
#define SOC_NPUCRG_PERDIS0_gt_clk_npu_hpm_START (31)
#define SOC_NPUCRG_PERDIS0_gt_clk_npu_hpm_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ai_core0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_aicore0_dw : 1;
        unsigned int gt_clk_aicore0_smmu : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int gt_clk_npu_tcu : 1;
        unsigned int reserved_12 : 1;
        unsigned int gt_clk_aicore0_hpm : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int gt_clk_qice_npu_data : 1;
        unsigned int gt_clk_qice_gm_gs : 1;
        unsigned int reserved_15 : 1;
        unsigned int gt_clk_qic_ts : 1;
        unsigned int gt_clk_qic_tcu : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int gt_clk_qic_aicore0 : 1;
        unsigned int reserved_19 : 1;
        unsigned int gt_clk_npu_pa : 1;
        unsigned int gt_clk_npu_hpm : 1;
    } reg;
} SOC_NPUCRG_PERCLKEN0_UNION;
#endif
#define SOC_NPUCRG_PERCLKEN0_gt_clk_ai_core0_START (0)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_ai_core0_END (0)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_aicore0_dw_START (8)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_aicore0_dw_END (8)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_aicore0_smmu_START (9)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_aicore0_smmu_END (9)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_npu_tcu_START (15)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_npu_tcu_END (15)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_aicore0_hpm_START (17)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_aicore0_hpm_END (17)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qice_npu_data_START (20)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qice_npu_data_END (20)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qice_gm_gs_START (21)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qice_gm_gs_END (21)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qic_ts_START (23)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qic_ts_END (23)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qic_tcu_START (24)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qic_tcu_END (24)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qic_aicore0_START (28)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_qic_aicore0_END (28)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_npu_pa_START (30)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_npu_pa_END (30)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_npu_hpm_START (31)
#define SOC_NPUCRG_PERCLKEN0_gt_clk_npu_hpm_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_ai_core0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int st_clk_aicore0_dw : 1;
        unsigned int st_clk_aicore0_smmu : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int st_clk_npu_tcu : 1;
        unsigned int reserved_12 : 1;
        unsigned int st_clk_aicore0_hpm : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int st_clk_qice_npu_data : 1;
        unsigned int st_clk_qice_gm_gs : 1;
        unsigned int reserved_15 : 1;
        unsigned int st_clk_qic_ts : 1;
        unsigned int st_clk_qic_tcu : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int st_clk_qic_aicore0 : 1;
        unsigned int reserved_19 : 1;
        unsigned int st_clk_npu_pa : 1;
        unsigned int st_clk_npu_hpm : 1;
    } reg;
} SOC_NPUCRG_PERSTAT0_UNION;
#endif
#define SOC_NPUCRG_PERSTAT0_st_clk_ai_core0_START (0)
#define SOC_NPUCRG_PERSTAT0_st_clk_ai_core0_END (0)
#define SOC_NPUCRG_PERSTAT0_st_clk_aicore0_dw_START (8)
#define SOC_NPUCRG_PERSTAT0_st_clk_aicore0_dw_END (8)
#define SOC_NPUCRG_PERSTAT0_st_clk_aicore0_smmu_START (9)
#define SOC_NPUCRG_PERSTAT0_st_clk_aicore0_smmu_END (9)
#define SOC_NPUCRG_PERSTAT0_st_clk_npu_tcu_START (15)
#define SOC_NPUCRG_PERSTAT0_st_clk_npu_tcu_END (15)
#define SOC_NPUCRG_PERSTAT0_st_clk_aicore0_hpm_START (17)
#define SOC_NPUCRG_PERSTAT0_st_clk_aicore0_hpm_END (17)
#define SOC_NPUCRG_PERSTAT0_st_clk_qice_npu_data_START (20)
#define SOC_NPUCRG_PERSTAT0_st_clk_qice_npu_data_END (20)
#define SOC_NPUCRG_PERSTAT0_st_clk_qice_gm_gs_START (21)
#define SOC_NPUCRG_PERSTAT0_st_clk_qice_gm_gs_END (21)
#define SOC_NPUCRG_PERSTAT0_st_clk_qic_ts_START (23)
#define SOC_NPUCRG_PERSTAT0_st_clk_qic_ts_END (23)
#define SOC_NPUCRG_PERSTAT0_st_clk_qic_tcu_START (24)
#define SOC_NPUCRG_PERSTAT0_st_clk_qic_tcu_END (24)
#define SOC_NPUCRG_PERSTAT0_st_clk_qic_aicore0_START (28)
#define SOC_NPUCRG_PERSTAT0_st_clk_qic_aicore0_END (28)
#define SOC_NPUCRG_PERSTAT0_st_clk_npu_pa_START (30)
#define SOC_NPUCRG_PERSTAT0_st_clk_npu_pa_END (30)
#define SOC_NPUCRG_PERSTAT0_st_clk_npu_hpm_START (31)
#define SOC_NPUCRG_PERSTAT0_st_clk_npu_hpm_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ppll5_sscg : 1;
        unsigned int gt_pclk_aicore0_smmu : 1;
        unsigned int gt_pclk_autodiv_npubus_ts_cpu : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_npu_uart : 1;
        unsigned int gt_pclk_npu_ipc_ns : 1;
        unsigned int gt_pclk_npu_ipc_s : 1;
        unsigned int gt_pclk_npu_hw_exp_irq : 1;
        unsigned int gt_pclk_autodiv_npubus : 1;
        unsigned int gt_pclk_autodiv_npubus_cfg : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_autodiv_npubus_aicore0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_gs : 1;
        unsigned int gt_clk_gm : 1;
        unsigned int gt_pclk_atgm : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_pclk_npu_tcu : 1;
        unsigned int gt_pclk_npu_pcr : 1;
        unsigned int gt_pclk_npu_dpm0 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_ppll5_npu_tp : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclk_npu_easc0 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns3 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns2 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns1 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns : 1;
    } reg;
} SOC_NPUCRG_PEREN1_UNION;
#endif
#define SOC_NPUCRG_PEREN1_gt_clk_ppll5_sscg_START (0)
#define SOC_NPUCRG_PEREN1_gt_clk_ppll5_sscg_END (0)
#define SOC_NPUCRG_PEREN1_gt_pclk_aicore0_smmu_START (1)
#define SOC_NPUCRG_PEREN1_gt_pclk_aicore0_smmu_END (1)
#define SOC_NPUCRG_PEREN1_gt_pclk_autodiv_npubus_ts_cpu_START (2)
#define SOC_NPUCRG_PEREN1_gt_pclk_autodiv_npubus_ts_cpu_END (2)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_uart_START (4)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_uart_END (4)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_ipc_ns_START (5)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_ipc_ns_END (5)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_ipc_s_START (6)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_ipc_s_END (6)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_START (7)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_END (7)
#define SOC_NPUCRG_PEREN1_gt_pclk_autodiv_npubus_START (8)
#define SOC_NPUCRG_PEREN1_gt_pclk_autodiv_npubus_END (8)
#define SOC_NPUCRG_PEREN1_gt_pclk_autodiv_npubus_cfg_START (9)
#define SOC_NPUCRG_PEREN1_gt_pclk_autodiv_npubus_cfg_END (9)
#define SOC_NPUCRG_PEREN1_gt_pclk_autodiv_npubus_aicore0_START (11)
#define SOC_NPUCRG_PEREN1_gt_pclk_autodiv_npubus_aicore0_END (11)
#define SOC_NPUCRG_PEREN1_gt_clk_gs_START (13)
#define SOC_NPUCRG_PEREN1_gt_clk_gs_END (13)
#define SOC_NPUCRG_PEREN1_gt_clk_gm_START (14)
#define SOC_NPUCRG_PEREN1_gt_clk_gm_END (14)
#define SOC_NPUCRG_PEREN1_gt_pclk_atgm_START (15)
#define SOC_NPUCRG_PEREN1_gt_pclk_atgm_END (15)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_tcu_START (20)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_tcu_END (20)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_pcr_START (21)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_pcr_END (21)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_dpm0_START (22)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_dpm0_END (22)
#define SOC_NPUCRG_PEREN1_gt_clk_ppll5_npu_tp_START (24)
#define SOC_NPUCRG_PEREN1_gt_clk_ppll5_npu_tp_END (24)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_easc0_START (26)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_easc0_END (26)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_ns3_START (28)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_ns3_END (28)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_ns2_START (29)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_ns2_END (29)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_ns1_START (30)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_ns1_END (30)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_ns_START (31)
#define SOC_NPUCRG_PEREN1_gt_pclk_npu_hw_exp_irq_ns_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ppll5_sscg : 1;
        unsigned int gt_pclk_aicore0_smmu : 1;
        unsigned int gt_pclk_autodiv_npubus_ts_cpu : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_npu_uart : 1;
        unsigned int gt_pclk_npu_ipc_ns : 1;
        unsigned int gt_pclk_npu_ipc_s : 1;
        unsigned int gt_pclk_npu_hw_exp_irq : 1;
        unsigned int gt_pclk_autodiv_npubus : 1;
        unsigned int gt_pclk_autodiv_npubus_cfg : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_autodiv_npubus_aicore0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_gs : 1;
        unsigned int gt_clk_gm : 1;
        unsigned int gt_pclk_atgm : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_pclk_npu_tcu : 1;
        unsigned int gt_pclk_npu_pcr : 1;
        unsigned int gt_pclk_npu_dpm0 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_ppll5_npu_tp : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclk_npu_easc0 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns3 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns2 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns1 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns : 1;
    } reg;
} SOC_NPUCRG_PERDIS1_UNION;
#endif
#define SOC_NPUCRG_PERDIS1_gt_clk_ppll5_sscg_START (0)
#define SOC_NPUCRG_PERDIS1_gt_clk_ppll5_sscg_END (0)
#define SOC_NPUCRG_PERDIS1_gt_pclk_aicore0_smmu_START (1)
#define SOC_NPUCRG_PERDIS1_gt_pclk_aicore0_smmu_END (1)
#define SOC_NPUCRG_PERDIS1_gt_pclk_autodiv_npubus_ts_cpu_START (2)
#define SOC_NPUCRG_PERDIS1_gt_pclk_autodiv_npubus_ts_cpu_END (2)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_uart_START (4)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_uart_END (4)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_ipc_ns_START (5)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_ipc_ns_END (5)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_ipc_s_START (6)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_ipc_s_END (6)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_START (7)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_END (7)
#define SOC_NPUCRG_PERDIS1_gt_pclk_autodiv_npubus_START (8)
#define SOC_NPUCRG_PERDIS1_gt_pclk_autodiv_npubus_END (8)
#define SOC_NPUCRG_PERDIS1_gt_pclk_autodiv_npubus_cfg_START (9)
#define SOC_NPUCRG_PERDIS1_gt_pclk_autodiv_npubus_cfg_END (9)
#define SOC_NPUCRG_PERDIS1_gt_pclk_autodiv_npubus_aicore0_START (11)
#define SOC_NPUCRG_PERDIS1_gt_pclk_autodiv_npubus_aicore0_END (11)
#define SOC_NPUCRG_PERDIS1_gt_clk_gs_START (13)
#define SOC_NPUCRG_PERDIS1_gt_clk_gs_END (13)
#define SOC_NPUCRG_PERDIS1_gt_clk_gm_START (14)
#define SOC_NPUCRG_PERDIS1_gt_clk_gm_END (14)
#define SOC_NPUCRG_PERDIS1_gt_pclk_atgm_START (15)
#define SOC_NPUCRG_PERDIS1_gt_pclk_atgm_END (15)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_tcu_START (20)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_tcu_END (20)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_pcr_START (21)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_pcr_END (21)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_dpm0_START (22)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_dpm0_END (22)
#define SOC_NPUCRG_PERDIS1_gt_clk_ppll5_npu_tp_START (24)
#define SOC_NPUCRG_PERDIS1_gt_clk_ppll5_npu_tp_END (24)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_easc0_START (26)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_easc0_END (26)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_ns3_START (28)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_ns3_END (28)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_ns2_START (29)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_ns2_END (29)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_ns1_START (30)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_ns1_END (30)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_ns_START (31)
#define SOC_NPUCRG_PERDIS1_gt_pclk_npu_hw_exp_irq_ns_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ppll5_sscg : 1;
        unsigned int gt_pclk_aicore0_smmu : 1;
        unsigned int gt_pclk_autodiv_npubus_ts_cpu : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_npu_uart : 1;
        unsigned int gt_pclk_npu_ipc_ns : 1;
        unsigned int gt_pclk_npu_ipc_s : 1;
        unsigned int gt_pclk_npu_hw_exp_irq : 1;
        unsigned int gt_pclk_autodiv_npubus : 1;
        unsigned int gt_pclk_autodiv_npubus_cfg : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_pclk_autodiv_npubus_aicore0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_gs : 1;
        unsigned int gt_clk_gm : 1;
        unsigned int gt_pclk_atgm : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_pclk_npu_tcu : 1;
        unsigned int gt_pclk_npu_pcr : 1;
        unsigned int gt_pclk_npu_dpm0 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_ppll5_npu_tp : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclk_npu_easc0 : 1;
        unsigned int reserved_9 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns3 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns2 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns1 : 1;
        unsigned int gt_pclk_npu_hw_exp_irq_ns : 1;
    } reg;
} SOC_NPUCRG_PERCLKEN1_UNION;
#endif
#define SOC_NPUCRG_PERCLKEN1_gt_clk_ppll5_sscg_START (0)
#define SOC_NPUCRG_PERCLKEN1_gt_clk_ppll5_sscg_END (0)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_aicore0_smmu_START (1)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_aicore0_smmu_END (1)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_autodiv_npubus_ts_cpu_START (2)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_autodiv_npubus_ts_cpu_END (2)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_uart_START (4)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_uart_END (4)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_ipc_ns_START (5)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_ipc_ns_END (5)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_ipc_s_START (6)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_ipc_s_END (6)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_START (7)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_END (7)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_autodiv_npubus_START (8)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_autodiv_npubus_END (8)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_autodiv_npubus_cfg_START (9)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_autodiv_npubus_cfg_END (9)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_autodiv_npubus_aicore0_START (11)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_autodiv_npubus_aicore0_END (11)
#define SOC_NPUCRG_PERCLKEN1_gt_clk_gs_START (13)
#define SOC_NPUCRG_PERCLKEN1_gt_clk_gs_END (13)
#define SOC_NPUCRG_PERCLKEN1_gt_clk_gm_START (14)
#define SOC_NPUCRG_PERCLKEN1_gt_clk_gm_END (14)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_atgm_START (15)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_atgm_END (15)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_tcu_START (20)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_tcu_END (20)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_pcr_START (21)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_pcr_END (21)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_dpm0_START (22)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_dpm0_END (22)
#define SOC_NPUCRG_PERCLKEN1_gt_clk_ppll5_npu_tp_START (24)
#define SOC_NPUCRG_PERCLKEN1_gt_clk_ppll5_npu_tp_END (24)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_easc0_START (26)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_easc0_END (26)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_ns3_START (28)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_ns3_END (28)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_ns2_START (29)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_ns2_END (29)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_ns1_START (30)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_ns1_END (30)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_ns_START (31)
#define SOC_NPUCRG_PERCLKEN1_gt_pclk_npu_hw_exp_irq_ns_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_ppll5_sscg : 1;
        unsigned int st_pclk_aicore0_smmu : 1;
        unsigned int st_pclk_autodiv_npubus_ts_cpu : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_pclk_npu_uart : 1;
        unsigned int st_pclk_npu_ipc_ns : 1;
        unsigned int st_pclk_npu_ipc_s : 1;
        unsigned int st_pclk_npu_hw_exp_irq : 1;
        unsigned int st_pclk_autodiv_npubus : 1;
        unsigned int st_pclk_autodiv_npubus_cfg : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_pclk_autodiv_npubus_aicore0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_clk_gs : 1;
        unsigned int st_clk_gm : 1;
        unsigned int st_pclk_atgm : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int st_pclk_npu_tcu : 1;
        unsigned int st_pclk_npu_pcr : 1;
        unsigned int st_pclk_npu_dpm0 : 1;
        unsigned int reserved_7 : 1;
        unsigned int st_clk_ppll5_npu_tp : 1;
        unsigned int reserved_8 : 1;
        unsigned int st_pclk_npu_easc0 : 1;
        unsigned int reserved_9 : 1;
        unsigned int st_pclk_npu_hw_exp_irq_ns3 : 1;
        unsigned int st_pclk_npu_hw_exp_irq_ns2 : 1;
        unsigned int st_pclk_npu_hw_exp_irq_ns1 : 1;
        unsigned int st_pclk_npu_hw_exp_irq_ns : 1;
    } reg;
} SOC_NPUCRG_PERSTAT1_UNION;
#endif
#define SOC_NPUCRG_PERSTAT1_st_clk_ppll5_sscg_START (0)
#define SOC_NPUCRG_PERSTAT1_st_clk_ppll5_sscg_END (0)
#define SOC_NPUCRG_PERSTAT1_st_pclk_aicore0_smmu_START (1)
#define SOC_NPUCRG_PERSTAT1_st_pclk_aicore0_smmu_END (1)
#define SOC_NPUCRG_PERSTAT1_st_pclk_autodiv_npubus_ts_cpu_START (2)
#define SOC_NPUCRG_PERSTAT1_st_pclk_autodiv_npubus_ts_cpu_END (2)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_uart_START (4)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_uart_END (4)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_ipc_ns_START (5)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_ipc_ns_END (5)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_ipc_s_START (6)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_ipc_s_END (6)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_START (7)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_END (7)
#define SOC_NPUCRG_PERSTAT1_st_pclk_autodiv_npubus_START (8)
#define SOC_NPUCRG_PERSTAT1_st_pclk_autodiv_npubus_END (8)
#define SOC_NPUCRG_PERSTAT1_st_pclk_autodiv_npubus_cfg_START (9)
#define SOC_NPUCRG_PERSTAT1_st_pclk_autodiv_npubus_cfg_END (9)
#define SOC_NPUCRG_PERSTAT1_st_pclk_autodiv_npubus_aicore0_START (11)
#define SOC_NPUCRG_PERSTAT1_st_pclk_autodiv_npubus_aicore0_END (11)
#define SOC_NPUCRG_PERSTAT1_st_clk_gs_START (13)
#define SOC_NPUCRG_PERSTAT1_st_clk_gs_END (13)
#define SOC_NPUCRG_PERSTAT1_st_clk_gm_START (14)
#define SOC_NPUCRG_PERSTAT1_st_clk_gm_END (14)
#define SOC_NPUCRG_PERSTAT1_st_pclk_atgm_START (15)
#define SOC_NPUCRG_PERSTAT1_st_pclk_atgm_END (15)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_tcu_START (20)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_tcu_END (20)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_pcr_START (21)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_pcr_END (21)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_dpm0_START (22)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_dpm0_END (22)
#define SOC_NPUCRG_PERSTAT1_st_clk_ppll5_npu_tp_START (24)
#define SOC_NPUCRG_PERSTAT1_st_clk_ppll5_npu_tp_END (24)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_easc0_START (26)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_easc0_END (26)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_ns3_START (28)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_ns3_END (28)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_ns2_START (29)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_ns2_END (29)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_ns1_START (30)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_ns1_END (30)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_ns_START (31)
#define SOC_NPUCRG_PERSTAT1_st_pclk_npu_hw_exp_irq_ns_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_npu_easc2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_npu_dmmu0 : 1;
        unsigned int gt_pclk_npu_dmmu1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_qcr_aicore0 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_qcr_tcu : 1;
        unsigned int gt_clk_qcr_ts : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_dw_dbg : 1;
        unsigned int gt_clk_pll_fsm : 1;
        unsigned int gt_clk_qic_npu2cfg : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 12;
    } reg;
} SOC_NPUCRG_PEREN2_UNION;
#endif
#define SOC_NPUCRG_PEREN2_gt_clk_npu_easc2_START (1)
#define SOC_NPUCRG_PEREN2_gt_clk_npu_easc2_END (1)
#define SOC_NPUCRG_PEREN2_gt_pclk_npu_dmmu0_START (4)
#define SOC_NPUCRG_PEREN2_gt_pclk_npu_dmmu0_END (4)
#define SOC_NPUCRG_PEREN2_gt_pclk_npu_dmmu1_START (5)
#define SOC_NPUCRG_PEREN2_gt_pclk_npu_dmmu1_END (5)
#define SOC_NPUCRG_PEREN2_gt_clk_qcr_aicore0_START (9)
#define SOC_NPUCRG_PEREN2_gt_clk_qcr_aicore0_END (9)
#define SOC_NPUCRG_PEREN2_gt_clk_qcr_tcu_START (11)
#define SOC_NPUCRG_PEREN2_gt_clk_qcr_tcu_END (11)
#define SOC_NPUCRG_PEREN2_gt_clk_qcr_ts_START (12)
#define SOC_NPUCRG_PEREN2_gt_clk_qcr_ts_END (12)
#define SOC_NPUCRG_PEREN2_gt_clk_dw_dbg_START (14)
#define SOC_NPUCRG_PEREN2_gt_clk_dw_dbg_END (14)
#define SOC_NPUCRG_PEREN2_gt_clk_pll_fsm_START (15)
#define SOC_NPUCRG_PEREN2_gt_clk_pll_fsm_END (15)
#define SOC_NPUCRG_PEREN2_gt_clk_qic_npu2cfg_START (16)
#define SOC_NPUCRG_PEREN2_gt_clk_qic_npu2cfg_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_npu_easc2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_npu_dmmu0 : 1;
        unsigned int gt_pclk_npu_dmmu1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_qcr_aicore0 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_qcr_tcu : 1;
        unsigned int gt_clk_qcr_ts : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_dw_dbg : 1;
        unsigned int gt_clk_pll_fsm : 1;
        unsigned int gt_clk_qic_npu2cfg : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 12;
    } reg;
} SOC_NPUCRG_PERDIS2_UNION;
#endif
#define SOC_NPUCRG_PERDIS2_gt_clk_npu_easc2_START (1)
#define SOC_NPUCRG_PERDIS2_gt_clk_npu_easc2_END (1)
#define SOC_NPUCRG_PERDIS2_gt_pclk_npu_dmmu0_START (4)
#define SOC_NPUCRG_PERDIS2_gt_pclk_npu_dmmu0_END (4)
#define SOC_NPUCRG_PERDIS2_gt_pclk_npu_dmmu1_START (5)
#define SOC_NPUCRG_PERDIS2_gt_pclk_npu_dmmu1_END (5)
#define SOC_NPUCRG_PERDIS2_gt_clk_qcr_aicore0_START (9)
#define SOC_NPUCRG_PERDIS2_gt_clk_qcr_aicore0_END (9)
#define SOC_NPUCRG_PERDIS2_gt_clk_qcr_tcu_START (11)
#define SOC_NPUCRG_PERDIS2_gt_clk_qcr_tcu_END (11)
#define SOC_NPUCRG_PERDIS2_gt_clk_qcr_ts_START (12)
#define SOC_NPUCRG_PERDIS2_gt_clk_qcr_ts_END (12)
#define SOC_NPUCRG_PERDIS2_gt_clk_dw_dbg_START (14)
#define SOC_NPUCRG_PERDIS2_gt_clk_dw_dbg_END (14)
#define SOC_NPUCRG_PERDIS2_gt_clk_pll_fsm_START (15)
#define SOC_NPUCRG_PERDIS2_gt_clk_pll_fsm_END (15)
#define SOC_NPUCRG_PERDIS2_gt_clk_qic_npu2cfg_START (16)
#define SOC_NPUCRG_PERDIS2_gt_clk_qic_npu2cfg_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_npu_easc2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_pclk_npu_dmmu0 : 1;
        unsigned int gt_pclk_npu_dmmu1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_qcr_aicore0 : 1;
        unsigned int reserved_6 : 1;
        unsigned int gt_clk_qcr_tcu : 1;
        unsigned int gt_clk_qcr_ts : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_dw_dbg : 1;
        unsigned int gt_clk_pll_fsm : 1;
        unsigned int gt_clk_qic_npu2cfg : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 12;
    } reg;
} SOC_NPUCRG_PERCLKEN2_UNION;
#endif
#define SOC_NPUCRG_PERCLKEN2_gt_clk_npu_easc2_START (1)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_npu_easc2_END (1)
#define SOC_NPUCRG_PERCLKEN2_gt_pclk_npu_dmmu0_START (4)
#define SOC_NPUCRG_PERCLKEN2_gt_pclk_npu_dmmu0_END (4)
#define SOC_NPUCRG_PERCLKEN2_gt_pclk_npu_dmmu1_START (5)
#define SOC_NPUCRG_PERCLKEN2_gt_pclk_npu_dmmu1_END (5)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_qcr_aicore0_START (9)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_qcr_aicore0_END (9)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_qcr_tcu_START (11)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_qcr_tcu_END (11)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_qcr_ts_START (12)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_qcr_ts_END (12)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_dw_dbg_START (14)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_dw_dbg_END (14)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_pll_fsm_START (15)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_pll_fsm_END (15)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_qic_npu2cfg_START (16)
#define SOC_NPUCRG_PERCLKEN2_gt_clk_qic_npu2cfg_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int st_clk_npu_easc2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_pclk_npu_dmmu0 : 1;
        unsigned int st_pclk_npu_dmmu1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int st_clk_qcr_aicore0 : 1;
        unsigned int reserved_6 : 1;
        unsigned int st_clk_qcr_tcu : 1;
        unsigned int st_clk_qcr_ts : 1;
        unsigned int reserved_7 : 1;
        unsigned int st_clk_dw_dbg : 1;
        unsigned int st_clk_pll_fsm : 1;
        unsigned int st_clk_qic_npu2cfg : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 12;
    } reg;
} SOC_NPUCRG_PERSTAT2_UNION;
#endif
#define SOC_NPUCRG_PERSTAT2_st_clk_npu_easc2_START (1)
#define SOC_NPUCRG_PERSTAT2_st_clk_npu_easc2_END (1)
#define SOC_NPUCRG_PERSTAT2_st_pclk_npu_dmmu0_START (4)
#define SOC_NPUCRG_PERSTAT2_st_pclk_npu_dmmu0_END (4)
#define SOC_NPUCRG_PERSTAT2_st_pclk_npu_dmmu1_START (5)
#define SOC_NPUCRG_PERSTAT2_st_pclk_npu_dmmu1_END (5)
#define SOC_NPUCRG_PERSTAT2_st_clk_qcr_aicore0_START (9)
#define SOC_NPUCRG_PERSTAT2_st_clk_qcr_aicore0_END (9)
#define SOC_NPUCRG_PERSTAT2_st_clk_qcr_tcu_START (11)
#define SOC_NPUCRG_PERSTAT2_st_clk_qcr_tcu_END (11)
#define SOC_NPUCRG_PERSTAT2_st_clk_qcr_ts_START (12)
#define SOC_NPUCRG_PERSTAT2_st_clk_qcr_ts_END (12)
#define SOC_NPUCRG_PERSTAT2_st_clk_dw_dbg_START (14)
#define SOC_NPUCRG_PERSTAT2_st_clk_dw_dbg_END (14)
#define SOC_NPUCRG_PERSTAT2_st_clk_pll_fsm_START (15)
#define SOC_NPUCRG_PERSTAT2_st_clk_pll_fsm_END (15)
#define SOC_NPUCRG_PERSTAT2_st_clk_qic_npu2cfg_START (16)
#define SOC_NPUCRG_PERSTAT2_st_clk_qic_npu2cfg_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ts_db : 1;
        unsigned int gt_pclkdbg_ts_cpu : 1;
        unsigned int gt_clk_ts_hwts_lite : 1;
        unsigned int gt_clk_ts_hwts_tiny : 1;
        unsigned int gt_clk_ts_smmu : 1;
        unsigned int gt_clk_ts_apb : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_ts_dw_bus : 1;
        unsigned int gt_clk_ts_cpu : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_qic_aicore2npubus : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 6;
    } reg;
} SOC_NPUCRG_PEREN3_UNION;
#endif
#define SOC_NPUCRG_PEREN3_gt_clk_ts_db_START (0)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_db_END (0)
#define SOC_NPUCRG_PEREN3_gt_pclkdbg_ts_cpu_START (1)
#define SOC_NPUCRG_PEREN3_gt_pclkdbg_ts_cpu_END (1)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_hwts_lite_START (2)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_hwts_lite_END (2)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_hwts_tiny_START (3)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_hwts_tiny_END (3)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_smmu_START (4)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_smmu_END (4)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_apb_START (5)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_apb_END (5)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_dw_bus_START (7)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_dw_bus_END (7)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_cpu_START (8)
#define SOC_NPUCRG_PEREN3_gt_clk_ts_cpu_END (8)
#define SOC_NPUCRG_PEREN3_gt_clk_qic_aicore2npubus_START (16)
#define SOC_NPUCRG_PEREN3_gt_clk_qic_aicore2npubus_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ts_db : 1;
        unsigned int gt_pclkdbg_ts_cpu : 1;
        unsigned int gt_clk_ts_hwts_lite : 1;
        unsigned int gt_clk_ts_hwts_tiny : 1;
        unsigned int gt_clk_ts_smmu : 1;
        unsigned int gt_clk_ts_apb : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_ts_dw_bus : 1;
        unsigned int gt_clk_ts_cpu : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_qic_aicore2npubus : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 6;
    } reg;
} SOC_NPUCRG_PERDIS3_UNION;
#endif
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_db_START (0)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_db_END (0)
#define SOC_NPUCRG_PERDIS3_gt_pclkdbg_ts_cpu_START (1)
#define SOC_NPUCRG_PERDIS3_gt_pclkdbg_ts_cpu_END (1)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_hwts_lite_START (2)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_hwts_lite_END (2)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_hwts_tiny_START (3)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_hwts_tiny_END (3)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_smmu_START (4)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_smmu_END (4)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_apb_START (5)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_apb_END (5)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_dw_bus_START (7)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_dw_bus_END (7)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_cpu_START (8)
#define SOC_NPUCRG_PERDIS3_gt_clk_ts_cpu_END (8)
#define SOC_NPUCRG_PERDIS3_gt_clk_qic_aicore2npubus_START (16)
#define SOC_NPUCRG_PERDIS3_gt_clk_qic_aicore2npubus_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_ts_db : 1;
        unsigned int gt_pclkdbg_ts_cpu : 1;
        unsigned int gt_clk_ts_hwts_lite : 1;
        unsigned int gt_clk_ts_hwts_tiny : 1;
        unsigned int gt_clk_ts_smmu : 1;
        unsigned int gt_clk_ts_apb : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_ts_dw_bus : 1;
        unsigned int gt_clk_ts_cpu : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_qic_aicore2npubus : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 6;
    } reg;
} SOC_NPUCRG_PERCLKEN3_UNION;
#endif
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_db_START (0)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_db_END (0)
#define SOC_NPUCRG_PERCLKEN3_gt_pclkdbg_ts_cpu_START (1)
#define SOC_NPUCRG_PERCLKEN3_gt_pclkdbg_ts_cpu_END (1)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_hwts_lite_START (2)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_hwts_lite_END (2)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_hwts_tiny_START (3)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_hwts_tiny_END (3)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_smmu_START (4)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_smmu_END (4)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_apb_START (5)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_apb_END (5)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_dw_bus_START (7)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_dw_bus_END (7)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_cpu_START (8)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_ts_cpu_END (8)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_qic_aicore2npubus_START (16)
#define SOC_NPUCRG_PERCLKEN3_gt_clk_qic_aicore2npubus_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_ts_db : 1;
        unsigned int st_pclkdbg_ts_cpu : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_clk_ts_hwts : 1;
        unsigned int st_clk_ts_smmu : 1;
        unsigned int st_clk_ts_apb : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_clk_ts_dw_bus : 1;
        unsigned int st_clk_ts_cpu : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int st_clk_qic_aicore2npubus : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 6;
    } reg;
} SOC_NPUCRG_PERSTAT3_UNION;
#endif
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_db_START (0)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_db_END (0)
#define SOC_NPUCRG_PERSTAT3_st_pclkdbg_ts_cpu_START (1)
#define SOC_NPUCRG_PERSTAT3_st_pclkdbg_ts_cpu_END (1)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_hwts_START (3)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_hwts_END (3)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_smmu_START (4)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_smmu_END (4)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_apb_START (5)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_apb_END (5)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_dw_bus_START (7)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_dw_bus_END (7)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_cpu_START (8)
#define SOC_NPUCRG_PERSTAT3_st_clk_ts_cpu_END (8)
#define SOC_NPUCRG_PERSTAT3_st_clk_qic_aicore2npubus_START (16)
#define SOC_NPUCRG_PERSTAT3_st_clk_qic_aicore2npubus_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_ai_core0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_pdbg_ts_cpu : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_aicore0_smmu : 1;
        unsigned int ip_rst_aicore0_dw : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_npu_tcu : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int ip_rst_ts_db : 1;
        unsigned int ip_rst_ts_hwts : 1;
        unsigned int ip_rst_ts_subsys : 1;
        unsigned int ip_rst_ts_crg : 1;
        unsigned int ip_rst_ts_smmu : 1;
        unsigned int ip_rst_qice_data : 1;
        unsigned int ip_rst_qice_gm_gs : 1;
        unsigned int reserved_10 : 1;
        unsigned int ip_rst_qic_ts : 1;
        unsigned int ip_rst_npu_cssys : 1;
        unsigned int reserved_11 : 1;
        unsigned int ip_rst_qic_tcu : 1;
        unsigned int reserved_12 : 1;
        unsigned int ip_rst_qic_aicore0 : 1;
        unsigned int ip_rst_npu_hpm : 1;
        unsigned int reserved_13 : 1;
        unsigned int ip_rst_npu_pa : 1;
    } reg;
} SOC_NPUCRG_PERRSTEN0_UNION;
#endif
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ai_core0_START (0)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ai_core0_END (0)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_pdbg_ts_cpu_START (2)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_pdbg_ts_cpu_END (2)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_aicore0_smmu_START (4)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_aicore0_smmu_END (4)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_aicore0_dw_START (5)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_aicore0_dw_END (5)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_npu_tcu_START (11)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_npu_tcu_END (11)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_db_START (15)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_db_END (15)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_hwts_START (16)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_hwts_END (16)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_subsys_START (17)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_subsys_END (17)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_crg_START (18)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_crg_END (18)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_smmu_START (19)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_ts_smmu_END (19)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qice_data_START (20)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qice_data_END (20)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qice_gm_gs_START (21)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qice_gm_gs_END (21)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qic_ts_START (23)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qic_ts_END (23)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_npu_cssys_START (24)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_npu_cssys_END (24)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qic_tcu_START (26)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qic_tcu_END (26)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qic_aicore0_START (28)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_qic_aicore0_END (28)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_npu_hpm_START (29)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_npu_hpm_END (29)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_npu_pa_START (31)
#define SOC_NPUCRG_PERRSTEN0_ip_rst_npu_pa_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_ai_core0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_pdbg_ts_cpu : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_aicore0_smmu : 1;
        unsigned int ip_rst_aicore0_dw : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_npu_tcu : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int ip_rst_ts_db : 1;
        unsigned int ip_rst_ts_hwts : 1;
        unsigned int ip_rst_ts_subsys : 1;
        unsigned int ip_rst_ts_crg : 1;
        unsigned int ip_rst_ts_smmu : 1;
        unsigned int ip_rst_qice_data : 1;
        unsigned int ip_rst_qice_gm_gs : 1;
        unsigned int reserved_10 : 1;
        unsigned int ip_rst_qic_ts : 1;
        unsigned int ip_rst_npu_cssys : 1;
        unsigned int reserved_11 : 1;
        unsigned int ip_rst_qic_tcu : 1;
        unsigned int reserved_12 : 1;
        unsigned int ip_rst_qic_aicore0 : 1;
        unsigned int ip_rst_npu_hpm : 1;
        unsigned int reserved_13 : 1;
        unsigned int ip_rst_npu_pa : 1;
    } reg;
} SOC_NPUCRG_PERRSTDIS0_UNION;
#endif
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ai_core0_START (0)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ai_core0_END (0)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_pdbg_ts_cpu_START (2)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_pdbg_ts_cpu_END (2)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_aicore0_smmu_START (4)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_aicore0_smmu_END (4)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_aicore0_dw_START (5)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_aicore0_dw_END (5)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_npu_tcu_START (11)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_npu_tcu_END (11)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_db_START (15)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_db_END (15)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_hwts_START (16)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_hwts_END (16)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_subsys_START (17)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_subsys_END (17)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_crg_START (18)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_crg_END (18)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_smmu_START (19)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_ts_smmu_END (19)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qice_data_START (20)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qice_data_END (20)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qice_gm_gs_START (21)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qice_gm_gs_END (21)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qic_ts_START (23)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qic_ts_END (23)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_npu_cssys_START (24)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_npu_cssys_END (24)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qic_tcu_START (26)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qic_tcu_END (26)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qic_aicore0_START (28)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_qic_aicore0_END (28)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_npu_hpm_START (29)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_npu_hpm_END (29)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_npu_pa_START (31)
#define SOC_NPUCRG_PERRSTDIS0_ip_rst_npu_pa_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_ai_core0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_pdbg_ts_cpu : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_aicore0_smmu : 1;
        unsigned int ip_rst_aicore0_dw : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_npu_tcu : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int ip_rst_ts_db : 1;
        unsigned int ip_rst_ts_hwts : 1;
        unsigned int ip_rst_ts_subsys : 1;
        unsigned int ip_rst_ts_crg : 1;
        unsigned int ip_rst_ts_smmu : 1;
        unsigned int ip_rst_qice_data : 1;
        unsigned int ip_rst_qice_gm_gs : 1;
        unsigned int reserved_10 : 1;
        unsigned int ip_rst_qic_ts : 1;
        unsigned int ip_rst_npu_cssys : 1;
        unsigned int reserved_11 : 1;
        unsigned int ip_rst_qic_tcu : 1;
        unsigned int reserved_12 : 1;
        unsigned int ip_rst_qic_aicore0 : 1;
        unsigned int ip_rst_npu_hpm : 1;
        unsigned int reserved_13 : 1;
        unsigned int ip_rst_npu_pa : 1;
    } reg;
} SOC_NPUCRG_PERRSTSTAT0_UNION;
#endif
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ai_core0_START (0)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ai_core0_END (0)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_pdbg_ts_cpu_START (2)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_pdbg_ts_cpu_END (2)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_aicore0_smmu_START (4)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_aicore0_smmu_END (4)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_aicore0_dw_START (5)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_aicore0_dw_END (5)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_npu_tcu_START (11)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_npu_tcu_END (11)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_db_START (15)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_db_END (15)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_hwts_START (16)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_hwts_END (16)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_subsys_START (17)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_subsys_END (17)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_crg_START (18)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_crg_END (18)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_smmu_START (19)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_ts_smmu_END (19)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qice_data_START (20)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qice_data_END (20)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qice_gm_gs_START (21)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qice_gm_gs_END (21)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qic_ts_START (23)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qic_ts_END (23)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_npu_cssys_START (24)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_npu_cssys_END (24)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qic_tcu_START (26)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qic_tcu_END (26)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qic_aicore0_START (28)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_qic_aicore0_END (28)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_npu_hpm_START (29)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_npu_hpm_END (29)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_npu_pa_START (31)
#define SOC_NPUCRG_PERRSTSTAT0_ip_rst_npu_pa_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_prst_npu_dpm0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_npu_uart : 1;
        unsigned int ip_prst_npu_ipc_ns : 1;
        unsigned int ip_prst_npu_ipc_s : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_npu_easc0 : 1;
        unsigned int ip_prst_npu_pcr : 1;
        unsigned int ip_prst_npu_tcu : 1;
        unsigned int ip_rst_gs : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_rst_gm : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_prst_aicore0_smmu : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_prst_atgm : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns : 1;
        unsigned int ip_prst_npu_hw_exp_irq : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns3 : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns2 : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns1 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 6;
    } reg;
} SOC_NPUCRG_PERRSTEN1_UNION;
#endif
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_dpm0_START (0)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_dpm0_END (0)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_uart_START (3)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_uart_END (3)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_ipc_ns_START (4)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_ipc_ns_END (4)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_ipc_s_START (5)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_ipc_s_END (5)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_easc0_START (7)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_easc0_END (7)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_pcr_START (8)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_pcr_END (8)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_tcu_START (9)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_tcu_END (9)
#define SOC_NPUCRG_PERRSTEN1_ip_rst_gs_START (10)
#define SOC_NPUCRG_PERRSTEN1_ip_rst_gs_END (10)
#define SOC_NPUCRG_PERRSTEN1_ip_rst_gm_START (12)
#define SOC_NPUCRG_PERRSTEN1_ip_rst_gm_END (12)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_aicore0_smmu_START (14)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_aicore0_smmu_END (14)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_atgm_START (16)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_atgm_END (16)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_ns_START (17)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_ns_END (17)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_START (18)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_END (18)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_ns3_START (19)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_ns3_END (19)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_ns2_START (20)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_ns2_END (20)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_ns1_START (21)
#define SOC_NPUCRG_PERRSTEN1_ip_prst_npu_hw_exp_irq_ns1_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_prst_npu_dpm0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_npu_uart : 1;
        unsigned int ip_prst_npu_ipc_ns : 1;
        unsigned int ip_prst_npu_ipc_s : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_npu_easc0 : 1;
        unsigned int ip_prst_npu_pcr : 1;
        unsigned int ip_prst_npu_tcu : 1;
        unsigned int ip_rst_gs : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_rst_gm : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_prst_aicore0_smmu : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_prst_atgm : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns : 1;
        unsigned int ip_prst_npu_hw_exp_irq : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns3 : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns2 : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns1 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 6;
    } reg;
} SOC_NPUCRG_PERRSTDIS1_UNION;
#endif
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_dpm0_START (0)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_dpm0_END (0)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_uart_START (3)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_uart_END (3)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_ipc_ns_START (4)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_ipc_ns_END (4)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_ipc_s_START (5)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_ipc_s_END (5)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_easc0_START (7)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_easc0_END (7)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_pcr_START (8)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_pcr_END (8)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_tcu_START (9)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_tcu_END (9)
#define SOC_NPUCRG_PERRSTDIS1_ip_rst_gs_START (10)
#define SOC_NPUCRG_PERRSTDIS1_ip_rst_gs_END (10)
#define SOC_NPUCRG_PERRSTDIS1_ip_rst_gm_START (12)
#define SOC_NPUCRG_PERRSTDIS1_ip_rst_gm_END (12)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_aicore0_smmu_START (14)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_aicore0_smmu_END (14)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_atgm_START (16)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_atgm_END (16)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_ns_START (17)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_ns_END (17)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_START (18)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_END (18)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_ns3_START (19)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_ns3_END (19)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_ns2_START (20)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_ns2_END (20)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_ns1_START (21)
#define SOC_NPUCRG_PERRSTDIS1_ip_prst_npu_hw_exp_irq_ns1_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_prst_npu_dpm0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_prst_npu_uart : 1;
        unsigned int ip_prst_npu_ipc_ns : 1;
        unsigned int ip_prst_npu_ipc_s : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_prst_npu_easc0 : 1;
        unsigned int ip_prst_npu_pcr : 1;
        unsigned int ip_prst_npu_tcu : 1;
        unsigned int ip_rst_gs : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_rst_gm : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_prst_aicore0_smmu : 1;
        unsigned int reserved_5 : 1;
        unsigned int ip_prst_atgm : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns : 1;
        unsigned int ip_prst_npu_hw_exp_irq : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns3 : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns2 : 1;
        unsigned int ip_prst_npu_hw_exp_irq_ns1 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 6;
    } reg;
} SOC_NPUCRG_PERRSTSTAT1_UNION;
#endif
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_dpm0_START (0)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_dpm0_END (0)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_uart_START (3)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_uart_END (3)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_ipc_ns_START (4)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_ipc_ns_END (4)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_ipc_s_START (5)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_ipc_s_END (5)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_easc0_START (7)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_easc0_END (7)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_pcr_START (8)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_pcr_END (8)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_tcu_START (9)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_tcu_END (9)
#define SOC_NPUCRG_PERRSTSTAT1_ip_rst_gs_START (10)
#define SOC_NPUCRG_PERRSTSTAT1_ip_rst_gs_END (10)
#define SOC_NPUCRG_PERRSTSTAT1_ip_rst_gm_START (12)
#define SOC_NPUCRG_PERRSTSTAT1_ip_rst_gm_END (12)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_aicore0_smmu_START (14)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_aicore0_smmu_END (14)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_atgm_START (16)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_atgm_END (16)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_ns_START (17)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_ns_END (17)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_START (18)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_END (18)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_ns3_START (19)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_ns3_END (19)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_ns2_START (20)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_ns2_END (20)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_ns1_START (21)
#define SOC_NPUCRG_PERRSTSTAT1_ip_prst_npu_hw_exp_irq_ns1_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_svfd_cpm_npu : 1;
        unsigned int ip_rst_svfd_ffs_npu : 1;
        unsigned int ip_rst_vsensorc_npu : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_npu_easc2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_npu_easc0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int ip_rst_pll_fsm : 1;
        unsigned int ip_prst_npu_dmmu0 : 1;
        unsigned int ip_prst_npu_dmmu1 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int ip_rst_aicore0_hpm : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int ip_rst_npu_pcr : 1;
        unsigned int reserved_20 : 2;
    } reg;
} SOC_NPUCRG_PERRSTEN2_UNION;
#endif
#define SOC_NPUCRG_PERRSTEN2_ip_rst_svfd_cpm_npu_START (0)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_svfd_cpm_npu_END (0)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_svfd_ffs_npu_START (1)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_svfd_ffs_npu_END (1)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_vsensorc_npu_START (2)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_vsensorc_npu_END (2)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_npu_easc2_START (4)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_npu_easc2_END (4)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_npu_easc0_START (6)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_npu_easc0_END (6)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_pll_fsm_START (15)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_pll_fsm_END (15)
#define SOC_NPUCRG_PERRSTEN2_ip_prst_npu_dmmu0_START (16)
#define SOC_NPUCRG_PERRSTEN2_ip_prst_npu_dmmu0_END (16)
#define SOC_NPUCRG_PERRSTEN2_ip_prst_npu_dmmu1_START (17)
#define SOC_NPUCRG_PERRSTEN2_ip_prst_npu_dmmu1_END (17)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_aicore0_hpm_START (26)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_aicore0_hpm_END (26)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_npu_pcr_START (29)
#define SOC_NPUCRG_PERRSTEN2_ip_rst_npu_pcr_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_svfd_cpm_npu : 1;
        unsigned int ip_rst_svfd_ffs_npu : 1;
        unsigned int ip_rst_vsensorc_npu : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_npu_easc2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_npu_easc0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int ip_rst_pll_fsm : 1;
        unsigned int ip_prst_npu_dmmu0 : 1;
        unsigned int ip_prst_npu_dmmu1 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int ip_rst_aicore0_hpm : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int ip_rst_npu_pcr : 1;
        unsigned int reserved_20 : 2;
    } reg;
} SOC_NPUCRG_PERRSTDIS2_UNION;
#endif
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_svfd_cpm_npu_START (0)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_svfd_cpm_npu_END (0)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_svfd_ffs_npu_START (1)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_svfd_ffs_npu_END (1)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_vsensorc_npu_START (2)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_vsensorc_npu_END (2)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_npu_easc2_START (4)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_npu_easc2_END (4)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_npu_easc0_START (6)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_npu_easc0_END (6)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_pll_fsm_START (15)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_pll_fsm_END (15)
#define SOC_NPUCRG_PERRSTDIS2_ip_prst_npu_dmmu0_START (16)
#define SOC_NPUCRG_PERRSTDIS2_ip_prst_npu_dmmu0_END (16)
#define SOC_NPUCRG_PERRSTDIS2_ip_prst_npu_dmmu1_START (17)
#define SOC_NPUCRG_PERRSTDIS2_ip_prst_npu_dmmu1_END (17)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_aicore0_hpm_START (26)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_aicore0_hpm_END (26)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_npu_pcr_START (29)
#define SOC_NPUCRG_PERRSTDIS2_ip_rst_npu_pcr_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_svfd_cpm_npu : 1;
        unsigned int ip_rst_svfd_ffs_npu : 1;
        unsigned int ip_rst_vsensorc_npu : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_npu_easc2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ip_rst_npu_easc0 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int ip_rst_pll_fsm : 1;
        unsigned int ip_prst_npu_dmmu0 : 1;
        unsigned int ip_prst_npu_dmmu1 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int ip_rst_aicore0_hpm : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int ip_rst_npu_pcr : 1;
        unsigned int reserved_20 : 2;
    } reg;
} SOC_NPUCRG_PERRSTSTAT2_UNION;
#endif
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_svfd_cpm_npu_START (0)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_svfd_cpm_npu_END (0)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_svfd_ffs_npu_START (1)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_svfd_ffs_npu_END (1)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_vsensorc_npu_START (2)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_vsensorc_npu_END (2)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_npu_easc2_START (4)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_npu_easc2_END (4)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_npu_easc0_START (6)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_npu_easc0_END (6)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_pll_fsm_START (15)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_pll_fsm_END (15)
#define SOC_NPUCRG_PERRSTSTAT2_ip_prst_npu_dmmu0_START (16)
#define SOC_NPUCRG_PERRSTSTAT2_ip_prst_npu_dmmu0_END (16)
#define SOC_NPUCRG_PERRSTSTAT2_ip_prst_npu_dmmu1_START (17)
#define SOC_NPUCRG_PERRSTSTAT2_ip_prst_npu_dmmu1_END (17)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_aicore0_hpm_START (26)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_aicore0_hpm_END (26)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_npu_pcr_START (29)
#define SOC_NPUCRG_PERRSTSTAT2_ip_rst_npu_pcr_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 2;
        unsigned int div_clk_ts_apb : 6;
        unsigned int div_clk_ai_core0 : 4;
        unsigned int reserved_1 : 4;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_CLKDIV0_UNION;
#endif
#define SOC_NPUCRG_CLKDIV0_div_clk_ts_apb_START (2)
#define SOC_NPUCRG_CLKDIV0_div_clk_ts_apb_END (7)
#define SOC_NPUCRG_CLKDIV0_div_clk_ai_core0_START (8)
#define SOC_NPUCRG_CLKDIV0_div_clk_ai_core0_END (11)
#define SOC_NPUCRG_CLKDIV0_bitmasken_START (16)
#define SOC_NPUCRG_CLKDIV0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 4;
        unsigned int div_clk_ts_cpu : 4;
        unsigned int reserved_1 : 8;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_CLKDIV3_UNION;
#endif
#define SOC_NPUCRG_CLKDIV3_div_clk_ts_cpu_START (4)
#define SOC_NPUCRG_CLKDIV3_div_clk_ts_cpu_END (7)
#define SOC_NPUCRG_CLKDIV3_bitmasken_START (16)
#define SOC_NPUCRG_CLKDIV3_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_gt_clk_pll_fsm : 1;
        unsigned int sc_gt_clk_ts_apb : 1;
        unsigned int sc_gt_clk_dw_dbg : 1;
        unsigned int sc_gt_clk_npu_monitor : 1;
        unsigned int reserved_0 : 1;
        unsigned int sc_gt_clk_ppll5_npu_tp : 1;
        unsigned int reserved_1 : 6;
        unsigned int reserved_2 : 1;
        unsigned int sc_gt_clk_ai_core0_div : 1;
        unsigned int reserved_3 : 1;
        unsigned int sc_gt_clk_ts_cpu_div : 1;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_CLKDIV1_UNION;
#endif
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_pll_fsm_START (0)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_pll_fsm_END (0)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_ts_apb_START (1)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_ts_apb_END (1)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_dw_dbg_START (2)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_dw_dbg_END (2)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_npu_monitor_START (3)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_npu_monitor_END (3)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_ppll5_npu_tp_START (5)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_ppll5_npu_tp_END (5)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_ai_core0_div_START (13)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_ai_core0_div_END (13)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_ts_cpu_div_START (15)
#define SOC_NPUCRG_CLKDIV1_sc_gt_clk_ts_cpu_div_END (15)
#define SOC_NPUCRG_CLKDIV1_bitmasken_START (16)
#define SOC_NPUCRG_CLKDIV1_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_clk_ppll5_npu_tp : 6;
        unsigned int div_clk_pll_fsm : 6;
        unsigned int div_clk_npu_monitor : 2;
        unsigned int reserved : 2;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_CLKDIV2_UNION;
#endif
#define SOC_NPUCRG_CLKDIV2_div_clk_ppll5_npu_tp_START (0)
#define SOC_NPUCRG_CLKDIV2_div_clk_ppll5_npu_tp_END (5)
#define SOC_NPUCRG_CLKDIV2_div_clk_pll_fsm_START (6)
#define SOC_NPUCRG_CLKDIV2_div_clk_pll_fsm_END (11)
#define SOC_NPUCRG_CLKDIV2_div_clk_npu_monitor_START (12)
#define SOC_NPUCRG_CLKDIV2_div_clk_npu_monitor_END (13)
#define SOC_NPUCRG_CLKDIV2_bitmasken_START (16)
#define SOC_NPUCRG_CLKDIV2_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int swdone_clk_ts_cpu_div : 1;
        unsigned int swdone_clk_ts_apb_div : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sw_ack_clk_ts_cpu_sw : 2;
        unsigned int swdone_clk_ai_core0_div : 1;
        unsigned int st_clk_npu_pcr : 1;
        unsigned int st_clk_ai_core_pcr : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_clk_npu_dpm0 : 1;
        unsigned int st_clk_ppll5_gt : 1;
        unsigned int pll5_sscg_idle : 1;
        unsigned int reserved_3 : 19;
    } reg;
} SOC_NPUCRG_PERI_STAT0_UNION;
#endif
#define SOC_NPUCRG_PERI_STAT0_swdone_clk_ts_cpu_div_START (0)
#define SOC_NPUCRG_PERI_STAT0_swdone_clk_ts_cpu_div_END (0)
#define SOC_NPUCRG_PERI_STAT0_swdone_clk_ts_apb_div_START (1)
#define SOC_NPUCRG_PERI_STAT0_swdone_clk_ts_apb_div_END (1)
#define SOC_NPUCRG_PERI_STAT0_sw_ack_clk_ts_cpu_sw_START (4)
#define SOC_NPUCRG_PERI_STAT0_sw_ack_clk_ts_cpu_sw_END (5)
#define SOC_NPUCRG_PERI_STAT0_swdone_clk_ai_core0_div_START (6)
#define SOC_NPUCRG_PERI_STAT0_swdone_clk_ai_core0_div_END (6)
#define SOC_NPUCRG_PERI_STAT0_st_clk_npu_pcr_START (7)
#define SOC_NPUCRG_PERI_STAT0_st_clk_npu_pcr_END (7)
#define SOC_NPUCRG_PERI_STAT0_st_clk_ai_core_pcr_START (8)
#define SOC_NPUCRG_PERI_STAT0_st_clk_ai_core_pcr_END (8)
#define SOC_NPUCRG_PERI_STAT0_st_clk_npu_dpm0_START (10)
#define SOC_NPUCRG_PERI_STAT0_st_clk_npu_dpm0_END (10)
#define SOC_NPUCRG_PERI_STAT0_st_clk_ppll5_gt_START (11)
#define SOC_NPUCRG_PERI_STAT0_st_clk_ppll5_gt_END (11)
#define SOC_NPUCRG_PERI_STAT0_pll5_sscg_idle_START (12)
#define SOC_NPUCRG_PERI_STAT0_pll5_sscg_idle_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int sel_ts_cpu_sw : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 10;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_PERI_CTRL0_UNION;
#endif
#define SOC_NPUCRG_PERI_CTRL0_sel_ts_cpu_sw_START (3)
#define SOC_NPUCRG_PERI_CTRL0_sel_ts_cpu_sw_END (3)
#define SOC_NPUCRG_PERI_CTRL0_bitmasken_START (16)
#define SOC_NPUCRG_PERI_CTRL0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int aicore0_smmu_clkrst_bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int npu_uart_clkrst_bypass : 1;
        unsigned int npu_ipc_ns_clkrst_bypass : 1;
        unsigned int npu_ipc_s_clkrst_bypass : 1;
        unsigned int npu_dpm0_clkrst_bypass : 1;
        unsigned int npu_irq_clkrst_bypass : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int irq_ns_clkrst_bypass : 1;
        unsigned int irq_ns1_clkrst_bypass : 1;
        unsigned int irq_ns2_clkrst_bypass : 1;
        unsigned int irq_ns3_clkrst_bypass : 1;
        unsigned int reserved_5 : 1;
        unsigned int easc0_clkrst_bypass : 1;
        unsigned int pcr_clkrst_bypass : 1;
        unsigned int npu_tcu_clkrst_bypass : 1;
        unsigned int reserved_6 : 1;
        unsigned int qcr_npubus_clkrst_bypass : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int qcr_tcu_clkrst_bypass : 1;
        unsigned int qcr_ts_clkrst_bypass : 1;
        unsigned int reserved_9 : 1;
        unsigned int npu_dmmu0_clkrst_bypass : 1;
        unsigned int npu_dmmu1_clkrst_bypass : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 2;
    } reg;
} SOC_NPUCRG_IPCLKRST_BYPASS_UNION;
#endif
#define SOC_NPUCRG_IPCLKRST_BYPASS_aicore0_smmu_clkrst_bypass_START (1)
#define SOC_NPUCRG_IPCLKRST_BYPASS_aicore0_smmu_clkrst_bypass_END (1)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_uart_clkrst_bypass_START (4)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_uart_clkrst_bypass_END (4)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_ipc_ns_clkrst_bypass_START (5)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_ipc_ns_clkrst_bypass_END (5)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_ipc_s_clkrst_bypass_START (6)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_ipc_s_clkrst_bypass_END (6)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_dpm0_clkrst_bypass_START (7)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_dpm0_clkrst_bypass_END (7)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_irq_clkrst_bypass_START (8)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_irq_clkrst_bypass_END (8)
#define SOC_NPUCRG_IPCLKRST_BYPASS_irq_ns_clkrst_bypass_START (11)
#define SOC_NPUCRG_IPCLKRST_BYPASS_irq_ns_clkrst_bypass_END (11)
#define SOC_NPUCRG_IPCLKRST_BYPASS_irq_ns1_clkrst_bypass_START (12)
#define SOC_NPUCRG_IPCLKRST_BYPASS_irq_ns1_clkrst_bypass_END (12)
#define SOC_NPUCRG_IPCLKRST_BYPASS_irq_ns2_clkrst_bypass_START (13)
#define SOC_NPUCRG_IPCLKRST_BYPASS_irq_ns2_clkrst_bypass_END (13)
#define SOC_NPUCRG_IPCLKRST_BYPASS_irq_ns3_clkrst_bypass_START (14)
#define SOC_NPUCRG_IPCLKRST_BYPASS_irq_ns3_clkrst_bypass_END (14)
#define SOC_NPUCRG_IPCLKRST_BYPASS_easc0_clkrst_bypass_START (16)
#define SOC_NPUCRG_IPCLKRST_BYPASS_easc0_clkrst_bypass_END (16)
#define SOC_NPUCRG_IPCLKRST_BYPASS_pcr_clkrst_bypass_START (17)
#define SOC_NPUCRG_IPCLKRST_BYPASS_pcr_clkrst_bypass_END (17)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_tcu_clkrst_bypass_START (18)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_tcu_clkrst_bypass_END (18)
#define SOC_NPUCRG_IPCLKRST_BYPASS_qcr_npubus_clkrst_bypass_START (20)
#define SOC_NPUCRG_IPCLKRST_BYPASS_qcr_npubus_clkrst_bypass_END (20)
#define SOC_NPUCRG_IPCLKRST_BYPASS_qcr_tcu_clkrst_bypass_START (23)
#define SOC_NPUCRG_IPCLKRST_BYPASS_qcr_tcu_clkrst_bypass_END (23)
#define SOC_NPUCRG_IPCLKRST_BYPASS_qcr_ts_clkrst_bypass_START (24)
#define SOC_NPUCRG_IPCLKRST_BYPASS_qcr_ts_clkrst_bypass_END (24)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_dmmu0_clkrst_bypass_START (26)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_dmmu0_clkrst_bypass_END (26)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_dmmu1_clkrst_bypass_START (27)
#define SOC_NPUCRG_IPCLKRST_BYPASS_npu_dmmu1_clkrst_bypass_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int qcr_npucfg_clkrst_bypass : 1;
        unsigned int qcr_sys2npu_clkrst_bypass : 1;
        unsigned int qcr_aicore2npubus_clkrst_bypass : 1;
        unsigned int qcr_npu2sys_clkrst_bypass : 1;
        unsigned int qcr_gmaicore0_clkrst_bypass : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 22;
    } reg;
} SOC_NPUCRG_IPCLKRST_BYPASS1_UNION;
#endif
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_npucfg_clkrst_bypass_START (4)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_npucfg_clkrst_bypass_END (4)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_sys2npu_clkrst_bypass_START (5)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_sys2npu_clkrst_bypass_END (5)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_aicore2npubus_clkrst_bypass_START (6)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_aicore2npubus_clkrst_bypass_END (6)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_npu2sys_clkrst_bypass_START (7)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_npu2sys_clkrst_bypass_END (7)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_gmaicore0_clkrst_bypass_START (8)
#define SOC_NPUCRG_IPCLKRST_BYPASS1_qcr_gmaicore0_clkrst_bypass_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int npubus_qic_tcu_bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int npubus_qic_ts_mst1_bypass : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int npubus_qic_aicore0_mst_bypass : 1;
        unsigned int npubus_qice_perf_idle_bypass : 1;
        unsigned int npubus_qice_ibqe_bypass : 1;
        unsigned int npubus_dcdr_top_busy_bypass : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int npubus_qic_himinibus_ts_4to5_idle_bypass : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 16;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV0_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qic_tcu_bypass_START (1)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qic_tcu_bypass_END (1)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qic_ts_mst1_bypass_START (3)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qic_ts_mst1_bypass_END (3)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qic_aicore0_mst_bypass_START (6)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qic_aicore0_mst_bypass_END (6)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qice_perf_idle_bypass_START (7)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qice_perf_idle_bypass_END (7)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qice_ibqe_bypass_START (8)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qice_ibqe_bypass_END (8)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_dcdr_top_busy_bypass_START (9)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_dcdr_top_busy_bypass_END (9)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qic_himinibus_ts_4to5_idle_bypass_START (12)
#define SOC_NPUCRG_PERI_AUTODIV0_npubus_qic_himinibus_ts_4to5_idle_bypass_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npubus_cfg_div_auto_reduce_bypass : 1;
        unsigned int npubus_cfg_auto_waitcfg_in : 14;
        unsigned int npubus_cfg_auto_waitcfg_out : 6;
        unsigned int npubus_cfg_div_auto_cfg : 6;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV1_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV1_npubus_cfg_div_auto_reduce_bypass_START (0)
#define SOC_NPUCRG_PERI_AUTODIV1_npubus_cfg_div_auto_reduce_bypass_END (0)
#define SOC_NPUCRG_PERI_AUTODIV1_npubus_cfg_auto_waitcfg_in_START (1)
#define SOC_NPUCRG_PERI_AUTODIV1_npubus_cfg_auto_waitcfg_in_END (14)
#define SOC_NPUCRG_PERI_AUTODIV1_npubus_cfg_auto_waitcfg_out_START (15)
#define SOC_NPUCRG_PERI_AUTODIV1_npubus_cfg_auto_waitcfg_out_END (20)
#define SOC_NPUCRG_PERI_AUTODIV1_npubus_cfg_div_auto_cfg_START (21)
#define SOC_NPUCRG_PERI_AUTODIV1_npubus_cfg_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npubus_div_auto_reduce_bypass : 1;
        unsigned int npubus_auto_waitcfg_in : 14;
        unsigned int npubus_auto_waitcfg_out : 6;
        unsigned int npubus_div_auto_cfg : 6;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV2_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV2_npubus_div_auto_reduce_bypass_START (0)
#define SOC_NPUCRG_PERI_AUTODIV2_npubus_div_auto_reduce_bypass_END (0)
#define SOC_NPUCRG_PERI_AUTODIV2_npubus_auto_waitcfg_in_START (1)
#define SOC_NPUCRG_PERI_AUTODIV2_npubus_auto_waitcfg_in_END (14)
#define SOC_NPUCRG_PERI_AUTODIV2_npubus_auto_waitcfg_out_START (15)
#define SOC_NPUCRG_PERI_AUTODIV2_npubus_auto_waitcfg_out_END (20)
#define SOC_NPUCRG_PERI_AUTODIV2_npubus_div_auto_cfg_START (21)
#define SOC_NPUCRG_PERI_AUTODIV2_npubus_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int autodiv_npubus_stat : 1;
        unsigned int autodiv_npubus_cfg_stat : 1;
        unsigned int autodiv_npubus_aicore0_stat : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int autodiv_npubus_ts_cpu_stat : 1;
        unsigned int reserved_2 : 26;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_autodiv_npubus_stat_START (0)
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_autodiv_npubus_stat_END (0)
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_autodiv_npubus_cfg_stat_START (1)
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_autodiv_npubus_cfg_stat_END (1)
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_autodiv_npubus_aicore0_stat_START (2)
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_autodiv_npubus_aicore0_stat_END (2)
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_autodiv_npubus_ts_cpu_stat_START (5)
#define SOC_NPUCRG_PERI_AUTODIV_INUSE_STAT_autodiv_npubus_ts_cpu_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int npubus_cfg_himinibus_ts_4to5_idle_bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int npubus_cfg_sys2npubus_cfg_bypass : 1;
        unsigned int npubus_cfg_ts_cfg_bypass : 1;
        unsigned int reserved_3 : 1;
        unsigned int npubus_cfg_npu2sysbus_cfg_bypass : 1;
        unsigned int npubus_cfg_npubus_cfg_bypass : 1;
        unsigned int npubus_cfg_dcdr_top_busy_bypass : 1;
        unsigned int npubus_cfg_aicore0_cfg_bypass : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 16;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV3_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_himinibus_ts_4to5_idle_bypass_START (1)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_himinibus_ts_4to5_idle_bypass_END (1)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_sys2npubus_cfg_bypass_START (4)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_sys2npubus_cfg_bypass_END (4)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_ts_cfg_bypass_START (5)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_ts_cfg_bypass_END (5)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_npu2sysbus_cfg_bypass_START (7)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_npu2sysbus_cfg_bypass_END (7)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_npubus_cfg_bypass_START (8)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_npubus_cfg_bypass_END (8)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_dcdr_top_busy_bypass_START (9)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_dcdr_top_busy_bypass_END (9)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_aicore0_cfg_bypass_START (10)
#define SOC_NPUCRG_PERI_AUTODIV3_npubus_cfg_aicore0_cfg_bypass_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npubus_aicore0_div_auto_reduce_bypass_lpm3 : 1;
        unsigned int npubus_aicore0_auto_waitcfg_in : 14;
        unsigned int npubus_aicore0_auto_waitcfg_out : 6;
        unsigned int npubus_aicore0_div_auto_cfg : 6;
        unsigned int npubus_aicore0_div_auto_reduce_bypass_acpu : 1;
        unsigned int npubus_aicore0_div_auto_reduce_bypass_mcpu : 1;
        unsigned int reserved : 3;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV5_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_div_auto_reduce_bypass_lpm3_START (0)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_div_auto_reduce_bypass_lpm3_END (0)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_auto_waitcfg_in_START (1)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_auto_waitcfg_in_END (14)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_auto_waitcfg_out_START (15)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_auto_waitcfg_out_END (20)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_div_auto_cfg_START (21)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_div_auto_cfg_END (26)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_div_auto_reduce_bypass_acpu_START (27)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_div_auto_reduce_bypass_acpu_END (27)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_div_auto_reduce_bypass_mcpu_START (28)
#define SOC_NPUCRG_PERI_AUTODIV5_npubus_aicore0_div_auto_reduce_bypass_mcpu_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npubus_ts_cpu_div_auto_reduce_bypass : 1;
        unsigned int npubus_ts_cpu_auto_waitcfg_in : 14;
        unsigned int npubus_ts_cpu_auto_waitcfg_out : 6;
        unsigned int npubus_ts_cpu_div_auto_cfg : 6;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 3;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV8_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV8_npubus_ts_cpu_div_auto_reduce_bypass_START (0)
#define SOC_NPUCRG_PERI_AUTODIV8_npubus_ts_cpu_div_auto_reduce_bypass_END (0)
#define SOC_NPUCRG_PERI_AUTODIV8_npubus_ts_cpu_auto_waitcfg_in_START (1)
#define SOC_NPUCRG_PERI_AUTODIV8_npubus_ts_cpu_auto_waitcfg_in_END (14)
#define SOC_NPUCRG_PERI_AUTODIV8_npubus_ts_cpu_auto_waitcfg_out_START (15)
#define SOC_NPUCRG_PERI_AUTODIV8_npubus_ts_cpu_auto_waitcfg_out_END (20)
#define SOC_NPUCRG_PERI_AUTODIV8_npubus_ts_cpu_div_auto_cfg_START (21)
#define SOC_NPUCRG_PERI_AUTODIV8_npubus_ts_cpu_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npubus_aicore0_qic_ts_mst1_bypass : 1;
        unsigned int npubus_aicore0_qic_ts_tb_cfg_bypass : 1;
        unsigned int npubus_aicore0_qic_tcu_bypass : 1;
        unsigned int npubus_aicore0_ts_rv_himinibus_idle_bypass : 1;
        unsigned int npubus_aicore0_ts_hwts_idle_bypass : 1;
        unsigned int npubus_aicore0_ts_himinibus_idle_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int npubus_aicore0_dcdr_top_busy_bypass : 1;
        unsigned int npubus_aicore0_autodiv_aic0_idle_bypass : 1;
        unsigned int npubus_aicore0_qic_aicore0_cfg_bypass : 1;
        unsigned int npubus_aicore0_qic_aicore0_mst_bypass : 1;
        unsigned int npubus_aicore0_qic_ts_cfg_bypass : 1;
        unsigned int npubus_aicore0_qic_npu2sysbus_cfg_bypass : 1;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV9_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_ts_mst1_bypass_START (0)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_ts_mst1_bypass_END (0)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_ts_tb_cfg_bypass_START (1)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_ts_tb_cfg_bypass_END (1)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_tcu_bypass_START (2)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_tcu_bypass_END (2)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_ts_rv_himinibus_idle_bypass_START (3)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_ts_rv_himinibus_idle_bypass_END (3)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_ts_hwts_idle_bypass_START (4)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_ts_hwts_idle_bypass_END (4)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_ts_himinibus_idle_bypass_START (5)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_ts_himinibus_idle_bypass_END (5)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_dcdr_top_busy_bypass_START (7)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_dcdr_top_busy_bypass_END (7)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_autodiv_aic0_idle_bypass_START (8)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_autodiv_aic0_idle_bypass_END (8)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_aicore0_cfg_bypass_START (9)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_aicore0_cfg_bypass_END (9)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_aicore0_mst_bypass_START (10)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_aicore0_mst_bypass_END (10)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_ts_cfg_bypass_START (11)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_ts_cfg_bypass_END (11)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_npu2sysbus_cfg_bypass_START (12)
#define SOC_NPUCRG_PERI_AUTODIV9_npubus_aicore0_qic_npu2sysbus_cfg_bypass_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npubus_ts_cpu_autodiv_rv_idle_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 19;
    } reg;
} SOC_NPUCRG_PERI_AUTODIV11_UNION;
#endif
#define SOC_NPUCRG_PERI_AUTODIV11_npubus_ts_cpu_autodiv_rv_idle_bypass_START (0)
#define SOC_NPUCRG_PERI_AUTODIV11_npubus_ts_cpu_autodiv_rv_idle_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ppll5_en : 1;
        unsigned int ppll5_bp : 1;
        unsigned int ppll5_refdiv : 6;
        unsigned int ppll5_fbdiv : 12;
        unsigned int ppll5_postdiv1 : 3;
        unsigned int ppll5_postdiv2 : 3;
        unsigned int ppll5_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_NPUCRG_PPLL5CTRL0_UNION;
#endif
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_en_START (0)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_en_END (0)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_bp_START (1)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_bp_END (1)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_refdiv_START (2)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_refdiv_END (7)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_fbdiv_START (8)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_fbdiv_END (19)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_postdiv1_START (20)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_postdiv1_END (22)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_postdiv2_START (23)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_postdiv2_END (25)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_lock_START (26)
#define SOC_NPUCRG_PPLL5CTRL0_ppll5_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ppll5_fracdiv : 24;
        unsigned int ppll5_int_mod : 1;
        unsigned int ppll5_cfg_vld : 1;
        unsigned int gt_clk_ppll5 : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_NPUCRG_PPLL5CTRL1_UNION;
#endif
#define SOC_NPUCRG_PPLL5CTRL1_ppll5_fracdiv_START (0)
#define SOC_NPUCRG_PPLL5CTRL1_ppll5_fracdiv_END (23)
#define SOC_NPUCRG_PPLL5CTRL1_ppll5_int_mod_START (24)
#define SOC_NPUCRG_PPLL5CTRL1_ppll5_int_mod_END (24)
#define SOC_NPUCRG_PPLL5CTRL1_ppll5_cfg_vld_START (25)
#define SOC_NPUCRG_PPLL5CTRL1_ppll5_cfg_vld_END (25)
#define SOC_NPUCRG_PPLL5CTRL1_gt_clk_ppll5_START (26)
#define SOC_NPUCRG_PPLL5CTRL1_gt_clk_ppll5_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sscg_enable : 1;
        unsigned int sscg_mode : 1;
        unsigned int sscg_rate : 4;
        unsigned int sscg_depth : 3;
        unsigned int sscg_fast_dis : 1;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_PPLL5SSCCTRL_UNION;
#endif
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_enable_START (0)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_enable_END (0)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_mode_START (1)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_mode_END (1)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_rate_START (2)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_rate_END (5)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_depth_START (6)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_depth_END (8)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_fast_dis_START (9)
#define SOC_NPUCRG_PPLL5SSCCTRL_sscg_fast_dis_END (9)
#define SOC_NPUCRG_PPLL5SSCCTRL_bitmasken_START (16)
#define SOC_NPUCRG_PPLL5SSCCTRL_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ppll5_gt_stat : 1;
        unsigned int ppll5_bypass : 1;
        unsigned int ppll5_en_stat : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 26;
    } reg;
} SOC_NPUCRG_PLL_VOTE_STAT_UNION;
#endif
#define SOC_NPUCRG_PLL_VOTE_STAT_ppll5_gt_stat_START (0)
#define SOC_NPUCRG_PLL_VOTE_STAT_ppll5_gt_stat_END (0)
#define SOC_NPUCRG_PLL_VOTE_STAT_ppll5_bypass_START (1)
#define SOC_NPUCRG_PLL_VOTE_STAT_ppll5_bypass_END (1)
#define SOC_NPUCRG_PLL_VOTE_STAT_ppll5_en_stat_START (2)
#define SOC_NPUCRG_PLL_VOTE_STAT_ppll5_en_stat_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ppll5_en_vote : 5;
        unsigned int reserved_0 : 5;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 1;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_PLL_EN_VOTE_CTRL_UNION;
#endif
#define SOC_NPUCRG_PLL_EN_VOTE_CTRL_ppll5_en_vote_START (0)
#define SOC_NPUCRG_PLL_EN_VOTE_CTRL_ppll5_en_vote_END (4)
#define SOC_NPUCRG_PLL_EN_VOTE_CTRL_bitmasken_START (16)
#define SOC_NPUCRG_PLL_EN_VOTE_CTRL_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ppll5_gt_vote : 5;
        unsigned int reserved_0 : 5;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 1;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_PLL_GT_VOTE_CTRL_UNION;
#endif
#define SOC_NPUCRG_PLL_GT_VOTE_CTRL_ppll5_gt_vote_START (0)
#define SOC_NPUCRG_PLL_GT_VOTE_CTRL_ppll5_gt_vote_END (4)
#define SOC_NPUCRG_PLL_GT_VOTE_CTRL_bitmasken_START (16)
#define SOC_NPUCRG_PLL_GT_VOTE_CTRL_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ppll5_bypass_vote : 5;
        unsigned int reserved_0 : 5;
        unsigned int reserved_1 : 5;
        unsigned int reserved_2 : 1;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_PLL_BYPASS_VOTE_CTRL_UNION;
#endif
#define SOC_NPUCRG_PLL_BYPASS_VOTE_CTRL_ppll5_bypass_vote_START (0)
#define SOC_NPUCRG_PLL_BYPASS_VOTE_CTRL_ppll5_bypass_vote_END (4)
#define SOC_NPUCRG_PLL_BYPASS_VOTE_CTRL_bitmasken_START (16)
#define SOC_NPUCRG_PLL_BYPASS_VOTE_CTRL_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int FNPLL_STAT0 : 32;
    } reg;
} SOC_NPUCRG_FNPLL_STAT0_UNION;
#endif
#define SOC_NPUCRG_FNPLL_STAT0_FNPLL_STAT0_START (0)
#define SOC_NPUCRG_FNPLL_STAT0_FNPLL_STAT0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sta_reg_sel : 1;
        unsigned int read_en : 1;
        unsigned int input_cfg_en : 1;
        unsigned int test_mode : 1;
        unsigned int unlock_clear : 1;
        unsigned int dll_en : 1;
        unsigned int pll_lock_ate_sel : 2;
        unsigned int test_data_sel : 4;
        unsigned int bias_ctrl : 2;
        unsigned int icp_ctrl : 2;
        unsigned int updn_sel : 1;
        unsigned int reserved : 2;
        unsigned int int_mask : 3;
        unsigned int pfd_div_ratio : 4;
        unsigned int freq_threshold : 6;
    } reg;
} SOC_NPUCRG_FNPLL_CFG0_UNION;
#endif
#define SOC_NPUCRG_FNPLL_CFG0_sta_reg_sel_START (0)
#define SOC_NPUCRG_FNPLL_CFG0_sta_reg_sel_END (0)
#define SOC_NPUCRG_FNPLL_CFG0_read_en_START (1)
#define SOC_NPUCRG_FNPLL_CFG0_read_en_END (1)
#define SOC_NPUCRG_FNPLL_CFG0_input_cfg_en_START (2)
#define SOC_NPUCRG_FNPLL_CFG0_input_cfg_en_END (2)
#define SOC_NPUCRG_FNPLL_CFG0_test_mode_START (3)
#define SOC_NPUCRG_FNPLL_CFG0_test_mode_END (3)
#define SOC_NPUCRG_FNPLL_CFG0_unlock_clear_START (4)
#define SOC_NPUCRG_FNPLL_CFG0_unlock_clear_END (4)
#define SOC_NPUCRG_FNPLL_CFG0_dll_en_START (5)
#define SOC_NPUCRG_FNPLL_CFG0_dll_en_END (5)
#define SOC_NPUCRG_FNPLL_CFG0_pll_lock_ate_sel_START (6)
#define SOC_NPUCRG_FNPLL_CFG0_pll_lock_ate_sel_END (7)
#define SOC_NPUCRG_FNPLL_CFG0_test_data_sel_START (8)
#define SOC_NPUCRG_FNPLL_CFG0_test_data_sel_END (11)
#define SOC_NPUCRG_FNPLL_CFG0_bias_ctrl_START (12)
#define SOC_NPUCRG_FNPLL_CFG0_bias_ctrl_END (13)
#define SOC_NPUCRG_FNPLL_CFG0_icp_ctrl_START (14)
#define SOC_NPUCRG_FNPLL_CFG0_icp_ctrl_END (15)
#define SOC_NPUCRG_FNPLL_CFG0_updn_sel_START (16)
#define SOC_NPUCRG_FNPLL_CFG0_updn_sel_END (16)
#define SOC_NPUCRG_FNPLL_CFG0_int_mask_START (19)
#define SOC_NPUCRG_FNPLL_CFG0_int_mask_END (21)
#define SOC_NPUCRG_FNPLL_CFG0_pfd_div_ratio_START (22)
#define SOC_NPUCRG_FNPLL_CFG0_pfd_div_ratio_END (25)
#define SOC_NPUCRG_FNPLL_CFG0_freq_threshold_START (26)
#define SOC_NPUCRG_FNPLL_CFG0_freq_threshold_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ctinue_lock_num : 6;
        unsigned int lock_thr : 2;
        unsigned int dc_ac_clk_en : 1;
        unsigned int dtc_ctrl_inv : 1;
        unsigned int reserved : 6;
        unsigned int ana_cfg_fnpll : 16;
    } reg;
} SOC_NPUCRG_FNPLL_CFG1_UNION;
#endif
#define SOC_NPUCRG_FNPLL_CFG1_ctinue_lock_num_START (0)
#define SOC_NPUCRG_FNPLL_CFG1_ctinue_lock_num_END (5)
#define SOC_NPUCRG_FNPLL_CFG1_lock_thr_START (6)
#define SOC_NPUCRG_FNPLL_CFG1_lock_thr_END (7)
#define SOC_NPUCRG_FNPLL_CFG1_dc_ac_clk_en_START (8)
#define SOC_NPUCRG_FNPLL_CFG1_dc_ac_clk_en_END (8)
#define SOC_NPUCRG_FNPLL_CFG1_dtc_ctrl_inv_START (9)
#define SOC_NPUCRG_FNPLL_CFG1_dtc_ctrl_inv_END (9)
#define SOC_NPUCRG_FNPLL_CFG1_ana_cfg_fnpll_START (16)
#define SOC_NPUCRG_FNPLL_CFG1_ana_cfg_fnpll_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div2_pd : 1;
        unsigned int fbdiv_rst_n : 1;
        unsigned int refdiv_rst_n : 1;
        unsigned int dll_force_en : 1;
        unsigned int phe_code_a : 2;
        unsigned int phe_code_b : 2;
        unsigned int bbpd_calib_byp : 1;
        unsigned int k_gain_cfg_en : 1;
        unsigned int k_gain_cfg : 6;
        unsigned int k_gain_av_thr : 3;
        unsigned int dtc_test : 1;
        unsigned int dtc_m_cfg : 6;
        unsigned int dtc_o_cfg : 6;
    } reg;
} SOC_NPUCRG_FNPLL_CFG2_UNION;
#endif
#define SOC_NPUCRG_FNPLL_CFG2_div2_pd_START (0)
#define SOC_NPUCRG_FNPLL_CFG2_div2_pd_END (0)
#define SOC_NPUCRG_FNPLL_CFG2_fbdiv_rst_n_START (1)
#define SOC_NPUCRG_FNPLL_CFG2_fbdiv_rst_n_END (1)
#define SOC_NPUCRG_FNPLL_CFG2_refdiv_rst_n_START (2)
#define SOC_NPUCRG_FNPLL_CFG2_refdiv_rst_n_END (2)
#define SOC_NPUCRG_FNPLL_CFG2_dll_force_en_START (3)
#define SOC_NPUCRG_FNPLL_CFG2_dll_force_en_END (3)
#define SOC_NPUCRG_FNPLL_CFG2_phe_code_a_START (4)
#define SOC_NPUCRG_FNPLL_CFG2_phe_code_a_END (5)
#define SOC_NPUCRG_FNPLL_CFG2_phe_code_b_START (6)
#define SOC_NPUCRG_FNPLL_CFG2_phe_code_b_END (7)
#define SOC_NPUCRG_FNPLL_CFG2_bbpd_calib_byp_START (8)
#define SOC_NPUCRG_FNPLL_CFG2_bbpd_calib_byp_END (8)
#define SOC_NPUCRG_FNPLL_CFG2_k_gain_cfg_en_START (9)
#define SOC_NPUCRG_FNPLL_CFG2_k_gain_cfg_en_END (9)
#define SOC_NPUCRG_FNPLL_CFG2_k_gain_cfg_START (10)
#define SOC_NPUCRG_FNPLL_CFG2_k_gain_cfg_END (15)
#define SOC_NPUCRG_FNPLL_CFG2_k_gain_av_thr_START (16)
#define SOC_NPUCRG_FNPLL_CFG2_k_gain_av_thr_END (18)
#define SOC_NPUCRG_FNPLL_CFG2_dtc_test_START (19)
#define SOC_NPUCRG_FNPLL_CFG2_dtc_test_END (19)
#define SOC_NPUCRG_FNPLL_CFG2_dtc_m_cfg_START (20)
#define SOC_NPUCRG_FNPLL_CFG2_dtc_m_cfg_END (25)
#define SOC_NPUCRG_FNPLL_CFG2_dtc_o_cfg_START (26)
#define SOC_NPUCRG_FNPLL_CFG2_dtc_o_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int scppll_cfg3 : 32;
    } reg;
} SOC_NPUCRG_FNPLL_CFG3_UNION;
#endif
#define SOC_NPUCRG_FNPLL_CFG3_scppll_cfg3_START (0)
#define SOC_NPUCRG_FNPLL_CFG3_scppll_cfg3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vs_ctrl_en_npu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPUCRG_VS_CTRL_EN_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_CTRL_EN_NPU_vs_ctrl_en_npu_START (0)
#define SOC_NPUCRG_VS_CTRL_EN_NPU_vs_ctrl_en_npu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vs_ctrl_bypass_npu : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPUCRG_VS_CTRL_BYPASS_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_CTRL_BYPASS_NPU_vs_ctrl_bypass_npu_START (0)
#define SOC_NPUCRG_VS_CTRL_BYPASS_NPU_vs_ctrl_bypass_npu_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_pcr_trig_vdm_en : 1;
        unsigned int npu_cpu0_wfi_wfe_bypass : 1;
        unsigned int npu_cpu1_wfi_wfe_bypass : 1;
        unsigned int npu_cpu2_wfi_wfe_bypass : 1;
        unsigned int npu_cpu3_wfi_wfe_bypass : 1;
        unsigned int npu_l2_idle_div_mod : 2;
        unsigned int npu_cfg_cnt_cpu_l2_idle_quit : 16;
        unsigned int npu_cpu_wake_up_mode : 2;
        unsigned int npu_cpu_l2_idle_switch_bypass : 1;
        unsigned int npu_cpu_l2_idle_gt_en : 1;
        unsigned int reserved : 1;
        unsigned int npu_pcr_trig_vdm_sel : 1;
        unsigned int sel_ckmux_npu_icg : 1;
        unsigned int cpu_clk_div_cfg_npu : 2;
    } reg;
} SOC_NPUCRG_VS_CTRL_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_CTRL_NPU_npu_pcr_trig_vdm_en_START (0)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_pcr_trig_vdm_en_END (0)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu0_wfi_wfe_bypass_START (1)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu0_wfi_wfe_bypass_END (1)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu1_wfi_wfe_bypass_START (2)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu1_wfi_wfe_bypass_END (2)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu2_wfi_wfe_bypass_START (3)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu2_wfi_wfe_bypass_END (3)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu3_wfi_wfe_bypass_START (4)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu3_wfi_wfe_bypass_END (4)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_l2_idle_div_mod_START (5)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_l2_idle_div_mod_END (6)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cfg_cnt_cpu_l2_idle_quit_START (7)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cfg_cnt_cpu_l2_idle_quit_END (22)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu_wake_up_mode_START (23)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu_wake_up_mode_END (24)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu_l2_idle_switch_bypass_START (25)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu_l2_idle_switch_bypass_END (25)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu_l2_idle_gt_en_START (26)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_cpu_l2_idle_gt_en_END (26)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_pcr_trig_vdm_sel_START (28)
#define SOC_NPUCRG_VS_CTRL_NPU_npu_pcr_trig_vdm_sel_END (28)
#define SOC_NPUCRG_VS_CTRL_NPU_sel_ckmux_npu_icg_START (29)
#define SOC_NPUCRG_VS_CTRL_NPU_sel_ckmux_npu_icg_END (29)
#define SOC_NPUCRG_VS_CTRL_NPU_cpu_clk_div_cfg_npu_START (30)
#define SOC_NPUCRG_VS_CTRL_NPU_cpu_clk_div_cfg_npu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int svfd_data_limit_e_npu : 1;
        unsigned int reserved_0 : 1;
        unsigned int svfd_pulse_width_sel_npu : 1;
        unsigned int reserved_1 : 1;
        unsigned int svfd_ulvt_ll_npu : 4;
        unsigned int svfd_ulvt_sl_npu : 4;
        unsigned int svfd_lvt_ll_npu : 4;
        unsigned int svfd_lvt_sl_npu : 4;
        unsigned int svfd_trim_npu : 2;
        unsigned int svfd_d_rate1_npu : 2;
        unsigned int svfd_d_rate_npu : 2;
        unsigned int svfd_off_mode_npu : 1;
        unsigned int svfd_div64_en_npu : 1;
        unsigned int svfd_cpm_period_npu : 1;
        unsigned int svfd_edge_sel_npu : 1;
        unsigned int svfd_cmp_data_mode_npu : 2;
    } reg;
} SOC_NPUCRG_VS_SVFD_CTRL0_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_data_limit_e_npu_START (0)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_data_limit_e_npu_END (0)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_pulse_width_sel_npu_START (2)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_pulse_width_sel_npu_END (2)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_ulvt_ll_npu_START (4)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_ulvt_ll_npu_END (7)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_ulvt_sl_npu_START (8)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_ulvt_sl_npu_END (11)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_lvt_ll_npu_START (12)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_lvt_ll_npu_END (15)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_lvt_sl_npu_START (16)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_lvt_sl_npu_END (19)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_trim_npu_START (20)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_trim_npu_END (21)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_d_rate1_npu_START (22)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_d_rate1_npu_END (23)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_d_rate_npu_START (24)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_d_rate_npu_END (25)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_off_mode_npu_START (26)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_off_mode_npu_END (26)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_div64_en_npu_START (27)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_div64_en_npu_END (27)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_cpm_period_npu_START (28)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_cpm_period_npu_END (28)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_edge_sel_npu_START (29)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_edge_sel_npu_END (29)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_cmp_data_mode_npu_START (30)
#define SOC_NPUCRG_VS_SVFD_CTRL0_NPU_svfd_cmp_data_mode_npu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int svfd_glitch_test_npu : 1;
        unsigned int reserved_0 : 3;
        unsigned int svfd_test_ffs_npu : 8;
        unsigned int svfd_test_cpm_npu : 8;
        unsigned int svfd_off_time_step_npu : 7;
        unsigned int reserved_1 : 5;
    } reg;
} SOC_NPUCRG_VS_SVFD_CTRL1_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_svfd_glitch_test_npu_START (0)
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_svfd_glitch_test_npu_END (0)
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_svfd_test_ffs_npu_START (4)
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_svfd_test_ffs_npu_END (11)
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_svfd_test_cpm_npu_START (12)
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_svfd_test_cpm_npu_END (19)
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_svfd_off_time_step_npu_START (20)
#define SOC_NPUCRG_VS_SVFD_CTRL1_NPU_svfd_off_time_step_npu_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int svfd_enalbe_mode_npu : 2;
        unsigned int svfs_cpm_threshold_r_npu : 6;
        unsigned int svfs_cpm_threshold_f_npu : 6;
        unsigned int vol_drop_en_npu : 1;
        unsigned int svfd_cmp_data_clr_npu : 1;
        unsigned int vs_svfd_ctrl2_npu_msk : 16;
    } reg;
} SOC_NPUCRG_VS_SVFD_CTRL2_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_svfd_enalbe_mode_npu_START (0)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_svfd_enalbe_mode_npu_END (1)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_svfs_cpm_threshold_r_npu_START (2)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_svfs_cpm_threshold_r_npu_END (7)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_svfs_cpm_threshold_f_npu_START (8)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_svfs_cpm_threshold_f_npu_END (13)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_vol_drop_en_npu_START (14)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_vol_drop_en_npu_END (14)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_svfd_cmp_data_clr_npu_START (15)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_svfd_cmp_data_clr_npu_END (15)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_vs_svfd_ctrl2_npu_msk_START (16)
#define SOC_NPUCRG_VS_SVFD_CTRL2_NPU_vs_svfd_ctrl2_npu_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int svfd_cpm_data_r_npu : 8;
        unsigned int svfd_cpm_data_f_npu : 8;
        unsigned int svfd_dll_lock_npu : 1;
        unsigned int svfd_cpm_data_norst_npu : 6;
        unsigned int reserved : 9;
    } reg;
} SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_r_npu_START (0)
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_r_npu_END (7)
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_f_npu_START (8)
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_f_npu_END (15)
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_svfd_dll_lock_npu_START (16)
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_svfd_dll_lock_npu_END (16)
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_norst_npu_START (17)
#define SOC_NPUCRG_VS_CPM_DATA_STAT_NPU_svfd_cpm_data_norst_npu_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int svfd_test_out_ffs_npu : 4;
        unsigned int svfd_test_out_cpm_npu : 4;
        unsigned int svfd_glitch_result_npu : 1;
        unsigned int idle_low_freq_en_npu : 1;
        unsigned int vbat_drop_protect_ind_npu : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_NPUCRG_VS_TEST_STAT_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_TEST_STAT_NPU_svfd_test_out_ffs_npu_START (0)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_svfd_test_out_ffs_npu_END (3)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_svfd_test_out_cpm_npu_START (4)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_svfd_test_out_cpm_npu_END (7)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_svfd_glitch_result_npu_START (8)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_svfd_glitch_result_npu_END (8)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_idle_low_freq_en_npu_START (9)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_idle_low_freq_en_npu_END (9)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_vbat_drop_protect_ind_npu_START (10)
#define SOC_NPUCRG_VS_TEST_STAT_NPU_vbat_drop_protect_ind_npu_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vs_pcr_div0 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_NPUCRG_VS_PCR_DIV0_UNION;
#endif
#define SOC_NPUCRG_VS_PCR_DIV0_vs_pcr_div0_START (0)
#define SOC_NPUCRG_VS_PCR_DIV0_vs_pcr_div0_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vs_pcr_div1 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_NPUCRG_VS_PCR_DIV1_UNION;
#endif
#define SOC_NPUCRG_VS_PCR_DIV1_vs_pcr_div1_START (0)
#define SOC_NPUCRG_VS_PCR_DIV1_vs_pcr_div1_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vs_pcr_div2 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_NPUCRG_VS_PCR_DIV2_UNION;
#endif
#define SOC_NPUCRG_VS_PCR_DIV2_vs_pcr_div2_START (0)
#define SOC_NPUCRG_VS_PCR_DIV2_vs_pcr_div2_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vs_pcr_div3 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_NPUCRG_VS_PCR_DIV3_UNION;
#endif
#define SOC_NPUCRG_VS_PCR_DIV3_vs_pcr_div3_START (0)
#define SOC_NPUCRG_VS_PCR_DIV3_vs_pcr_div3_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vs_svfd_ctrl3_npu : 16;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_VS_SVFD_CTRL3_NPU_UNION;
#endif
#define SOC_NPUCRG_VS_SVFD_CTRL3_NPU_vs_svfd_ctrl3_npu_START (0)
#define SOC_NPUCRG_VS_SVFD_CTRL3_NPU_vs_svfd_ctrl3_npu_END (15)
#define SOC_NPUCRG_VS_SVFD_CTRL3_NPU_bitmasken_START (16)
#define SOC_NPUCRG_VS_SVFD_CTRL3_NPU_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_mask_aicore0_nonidle_pend : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int intr_mask_tcu_nonidle_pend : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_mask_ts_nonidle_pend : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 8;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_intr_mask_aicore0_nonidle_pend_START (0)
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_intr_mask_aicore0_nonidle_pend_END (0)
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_intr_mask_tcu_nonidle_pend_START (3)
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_intr_mask_tcu_nonidle_pend_END (3)
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_intr_mask_ts_nonidle_pend_START (5)
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_intr_mask_ts_nonidle_pend_END (5)
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_bitmasken_START (16)
#define SOC_NPUCRG_INTR_MASK_QICBUS_NONIDLE_PEND_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_clr_aicore0_nonidle_pend : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int intr_clr_tcu_nonidle_pend : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_clr_ts_nonidle_pend : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 8;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_intr_clr_aicore0_nonidle_pend_START (0)
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_intr_clr_aicore0_nonidle_pend_END (0)
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_intr_clr_tcu_nonidle_pend_START (3)
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_intr_clr_tcu_nonidle_pend_END (3)
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_intr_clr_ts_nonidle_pend_START (5)
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_intr_clr_ts_nonidle_pend_END (5)
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_bitmasken_START (16)
#define SOC_NPUCRG_INTR_CLR_QICBUS_NONIDLE_PEND_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_stat_aicore0_nonidle_pend : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int intr_stat_tcu_nonidle_pend : 1;
        unsigned int reserved_2 : 1;
        unsigned int intr_stat_ts_nonidle_pend : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 24;
    } reg;
} SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_intr_stat_aicore0_nonidle_pend_START (0)
#define SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_intr_stat_aicore0_nonidle_pend_END (0)
#define SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_intr_stat_tcu_nonidle_pend_START (3)
#define SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_intr_stat_tcu_nonidle_pend_END (3)
#define SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_intr_stat_ts_nonidle_pend_START (5)
#define SOC_NPUCRG_INTR_STAT_QICBUS_NONIDLE_PEND_intr_stat_ts_nonidle_pend_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pll_fsm_en : 1;
        unsigned int pll_vote_mode_sel : 1;
        unsigned int pll_intr_clr : 1;
        unsigned int reserved : 13;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_PLL_FSM_CTRL0_UNION;
#endif
#define SOC_NPUCRG_PLL_FSM_CTRL0_pll_fsm_en_START (0)
#define SOC_NPUCRG_PLL_FSM_CTRL0_pll_fsm_en_END (0)
#define SOC_NPUCRG_PLL_FSM_CTRL0_pll_vote_mode_sel_START (1)
#define SOC_NPUCRG_PLL_FSM_CTRL0_pll_vote_mode_sel_END (1)
#define SOC_NPUCRG_PLL_FSM_CTRL0_pll_intr_clr_START (2)
#define SOC_NPUCRG_PLL_FSM_CTRL0_pll_intr_clr_END (2)
#define SOC_NPUCRG_PLL_FSM_CTRL0_bitmasken_START (16)
#define SOC_NPUCRG_PLL_FSM_CTRL0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pll_lock_dbc_en : 1;
        unsigned int pll_fsm_intr_en : 1;
        unsigned int pll_wait_en : 1;
        unsigned int pll_timeout_en : 1;
        unsigned int pll_intr_lmt : 3;
        unsigned int pll_wait_lmt : 8;
        unsigned int pll_timeout_lmt : 8;
        unsigned int reserved : 9;
    } reg;
} SOC_NPUCRG_PLL_FSM_CTRL1_UNION;
#endif
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_lock_dbc_en_START (0)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_lock_dbc_en_END (0)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_fsm_intr_en_START (1)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_fsm_intr_en_END (1)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_wait_en_START (2)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_wait_en_END (2)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_timeout_en_START (3)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_timeout_en_END (3)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_intr_lmt_START (4)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_intr_lmt_END (6)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_wait_lmt_START (7)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_wait_lmt_END (14)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_timeout_lmt_START (15)
#define SOC_NPUCRG_PLL_FSM_CTRL1_pll_timeout_lmt_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pll_fsm_intr : 1;
        unsigned int pll_intr_cnt : 3;
        unsigned int pll_curr_stat : 4;
        unsigned int reserved : 24;
    } reg;
} SOC_NPUCRG_PLL_FSM_STAT_UNION;
#endif
#define SOC_NPUCRG_PLL_FSM_STAT_pll_fsm_intr_START (0)
#define SOC_NPUCRG_PLL_FSM_STAT_pll_fsm_intr_END (0)
#define SOC_NPUCRG_PLL_FSM_STAT_pll_intr_cnt_START (1)
#define SOC_NPUCRG_PLL_FSM_STAT_pll_intr_cnt_END (3)
#define SOC_NPUCRG_PLL_FSM_STAT_pll_curr_stat_START (4)
#define SOC_NPUCRG_PLL_FSM_STAT_pll_curr_stat_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ppll5_ns_en_vote : 5;
        unsigned int reserved : 11;
        unsigned int bitmasken : 16;
    } reg;
} SOC_NPUCRG_PLL_FSM_NS_VOTE0_UNION;
#endif
#define SOC_NPUCRG_PLL_FSM_NS_VOTE0_ppll5_ns_en_vote_START (0)
#define SOC_NPUCRG_PLL_FSM_NS_VOTE0_ppll5_ns_en_vote_END (4)
#define SOC_NPUCRG_PLL_FSM_NS_VOTE0_bitmasken_START (16)
#define SOC_NPUCRG_PLL_FSM_NS_VOTE0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pll_fsm_wsat : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPUCRG_PLL_FSM_NS_STAT_UNION;
#endif
#define SOC_NPUCRG_PLL_FSM_NS_STAT_pll_fsm_wsat_START (0)
#define SOC_NPUCRG_PLL_FSM_NS_STAT_pll_fsm_wsat_END (0)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
