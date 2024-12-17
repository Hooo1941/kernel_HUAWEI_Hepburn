#ifndef __SOC_NPU_SCTRL_INTERFACE_H__
#define __SOC_NPU_SCTRL_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_SCTRL_NPU_CTRL0_ADDR(base) ((base) + (0x000UL))
#define SOC_NPU_SCTRL_NPU_CTRL1_ADDR(base) ((base) + (0x004UL))
#define SOC_NPU_SCTRL_NPU_CTRL2_ADDR(base) ((base) + (0x008UL))
#define SOC_NPU_SCTRL_NPU_CTRL3_ADDR(base) ((base) + (0x00CUL))
#define SOC_NPU_SCTRL_NPU_CTRL4_ADDR(base) ((base) + (0x010UL))
#define SOC_NPU_SCTRL_NPU_CTRL5_ADDR(base) ((base) + (0x014UL))
#define SOC_NPU_SCTRL_NPU_CTRL6_ADDR(base) ((base) + (0x018UL))
#define SOC_NPU_SCTRL_NPU_CTRL7_ADDR(base) ((base) + (0x01CUL))
#define SOC_NPU_SCTRL_NPU_CTRL8_ADDR(base) ((base) + (0x020UL))
#define SOC_NPU_SCTRL_NPU_CTRL9_ADDR(base) ((base) + (0x024UL))
#define SOC_NPU_SCTRL_NPU_CTRL10_ADDR(base) ((base) + (0x028UL))
#define SOC_NPU_SCTRL_NPU_CTRL11_ADDR(base) ((base) + (0x02CUL))
#define SOC_NPU_SCTRL_NPU_CTRL12_ADDR(base) ((base) + (0x030UL))
#define SOC_NPU_SCTRL_NPU_CTRL13_ADDR(base) ((base) + (0x034UL))
#define SOC_NPU_SCTRL_NPU_CTRL14_ADDR(base) ((base) + (0x038UL))
#define SOC_NPU_SCTRL_NPU_CTRL15_ADDR(base) ((base) + (0x03CUL))
#define SOC_NPU_SCTRL_NPU_CTRL16_ADDR(base) ((base) + (0x040UL))
#define SOC_NPU_SCTRL_NPU_CTRL17_ADDR(base) ((base) + (0x044UL))
#define SOC_NPU_SCTRL_NPU_CTRL18_ADDR(base) ((base) + (0x048UL))
#define SOC_NPU_SCTRL_NPU_CTRL19_ADDR(base) ((base) + (0x04CUL))
#define SOC_NPU_SCTRL_NPU_CTRL20_ADDR(base) ((base) + (0x050UL))
#define SOC_NPU_SCTRL_NPU_CTRL21_ADDR(base) ((base) + (0x054UL))
#define SOC_NPU_SCTRL_NPU_CTRL22_ADDR(base) ((base) + (0x058UL))
#define SOC_NPU_SCTRL_NPU_CTRL23_ADDR(base) ((base) + (0x05CUL))
#define SOC_NPU_SCTRL_NPU_CTRL24_ADDR(base) ((base) + (0x060UL))
#define SOC_NPU_SCTRL_NPU_CTRL25_ADDR(base) ((base) + (0x064UL))
#define SOC_NPU_SCTRL_NPU_CTRL26_ADDR(base) ((base) + (0x068UL))
#define SOC_NPU_SCTRL_NPU_CTRL27_ADDR(base) ((base) + (0x06CUL))
#define SOC_NPU_SCTRL_NPU_CTRL28_ADDR(base) ((base) + (0x070UL))
#define SOC_NPU_SCTRL_NPU_CTRL29_ADDR(base) ((base) + (0x074UL))
#define SOC_NPU_SCTRL_NPU_CTRL30_ADDR(base) ((base) + (0x078UL))
#define SOC_NPU_SCTRL_NPU_CTRL31_ADDR(base) ((base) + (0x07CUL))
#define SOC_NPU_SCTRL_NPU_CTRL32_ADDR(base) ((base) + (0x080UL))
#define SOC_NPU_SCTRL_NPU_CTRL33_ADDR(base) ((base) + (0x084UL))
#define SOC_NPU_SCTRL_NPU_CTRL34_ADDR(base) ((base) + (0x088UL))
#define SOC_NPU_SCTRL_NPU_CTRL35_ADDR(base) ((base) + (0x08CUL))
#define SOC_NPU_SCTRL_NPU_CTRL36_ADDR(base) ((base) + (0x090UL))
#define SOC_NPU_SCTRL_NPU_CTRL37_ADDR(base) ((base) + (0x094UL))
#define SOC_NPU_SCTRL_NPU_CTRL38_ADDR(base) ((base) + (0x098UL))
#define SOC_NPU_SCTRL_NPU_CTRL39_ADDR(base) ((base) + (0x09CUL))
#define SOC_NPU_SCTRL_NPU_CTRL40_ADDR(base) ((base) + (0x0A0UL))
#define SOC_NPU_SCTRL_NPU_CTRL41_ADDR(base) ((base) + (0x0A4UL))
#define SOC_NPU_SCTRL_NPU_CTRL42_ADDR(base) ((base) + (0x0A8UL))
#define SOC_NPU_SCTRL_NPU_CTRL43_ADDR(base) ((base) + (0x0ACUL))
#define SOC_NPU_SCTRL_NPU_STAT0_ADDR(base) ((base) + (0x800UL))
#define SOC_NPU_SCTRL_NPU_STAT1_ADDR(base) ((base) + (0x804UL))
#define SOC_NPU_SCTRL_NPU_STAT2_ADDR(base) ((base) + (0x808UL))
#define SOC_NPU_SCTRL_NPU_STAT3_ADDR(base) ((base) + (0x80CUL))
#define SOC_NPU_SCTRL_NPU_STAT4_ADDR(base) ((base) + (0x810UL))
#define SOC_NPU_SCTRL_NPU_STAT5_ADDR(base) ((base) + (0x814UL))
#define SOC_NPU_SCTRL_NPU_STAT6_ADDR(base) ((base) + (0x818UL))
#define SOC_NPU_SCTRL_NPU_STAT7_ADDR(base) ((base) + (0x81CUL))
#define SOC_NPU_SCTRL_NPU_STAT8_ADDR(base) ((base) + (0x820UL))
#define SOC_NPU_SCTRL_NPU_STAT9_ADDR(base) ((base) + (0x824UL))
#define SOC_NPU_SCTRL_NPU_STAT10_ADDR(base) ((base) + (0x828UL))
#define SOC_NPU_SCTRL_NPU_STAT11_ADDR(base) ((base) + (0x82CUL))
#define SOC_NPU_SCTRL_NPU_STAT12_ADDR(base) ((base) + (0x830UL))
#define SOC_NPU_SCTRL_NPU_STAT13_ADDR(base) ((base) + (0x834UL))
#define SOC_NPU_SCTRL_NPU_STAT14_ADDR(base) ((base) + (0x838UL))
#define SOC_NPU_SCTRL_NPU_STAT15_ADDR(base) ((base) + (0x83CUL))
#define SOC_NPU_SCTRL_NPU_STAT16_ADDR(base) ((base) + (0x840UL))
#define SOC_NPU_SCTRL_NPU_STAT17_ADDR(base) ((base) + (0x844UL))
#define SOC_NPU_SCTRL_NPU_STAT18_ADDR(base) ((base) + (0x848UL))
#define SOC_NPU_SCTRL_NPU_STAT19_ADDR(base) ((base) + (0x84CUL))
#define SOC_NPU_SCTRL_NPU_STAT20_ADDR(base) ((base) + (0x850UL))
#define SOC_NPU_SCTRL_NPU_STAT21_ADDR(base) ((base) + (0x854UL))
#define SOC_NPU_SCTRL_NPU_STAT22_ADDR(base) ((base) + (0x858UL))
#else
#define SOC_NPU_SCTRL_NPU_CTRL0_ADDR(base) ((base) + (0x000))
#define SOC_NPU_SCTRL_NPU_CTRL1_ADDR(base) ((base) + (0x004))
#define SOC_NPU_SCTRL_NPU_CTRL2_ADDR(base) ((base) + (0x008))
#define SOC_NPU_SCTRL_NPU_CTRL3_ADDR(base) ((base) + (0x00C))
#define SOC_NPU_SCTRL_NPU_CTRL4_ADDR(base) ((base) + (0x010))
#define SOC_NPU_SCTRL_NPU_CTRL5_ADDR(base) ((base) + (0x014))
#define SOC_NPU_SCTRL_NPU_CTRL6_ADDR(base) ((base) + (0x018))
#define SOC_NPU_SCTRL_NPU_CTRL7_ADDR(base) ((base) + (0x01C))
#define SOC_NPU_SCTRL_NPU_CTRL8_ADDR(base) ((base) + (0x020))
#define SOC_NPU_SCTRL_NPU_CTRL9_ADDR(base) ((base) + (0x024))
#define SOC_NPU_SCTRL_NPU_CTRL10_ADDR(base) ((base) + (0x028))
#define SOC_NPU_SCTRL_NPU_CTRL11_ADDR(base) ((base) + (0x02C))
#define SOC_NPU_SCTRL_NPU_CTRL12_ADDR(base) ((base) + (0x030))
#define SOC_NPU_SCTRL_NPU_CTRL13_ADDR(base) ((base) + (0x034))
#define SOC_NPU_SCTRL_NPU_CTRL14_ADDR(base) ((base) + (0x038))
#define SOC_NPU_SCTRL_NPU_CTRL15_ADDR(base) ((base) + (0x03C))
#define SOC_NPU_SCTRL_NPU_CTRL16_ADDR(base) ((base) + (0x040))
#define SOC_NPU_SCTRL_NPU_CTRL17_ADDR(base) ((base) + (0x044))
#define SOC_NPU_SCTRL_NPU_CTRL18_ADDR(base) ((base) + (0x048))
#define SOC_NPU_SCTRL_NPU_CTRL19_ADDR(base) ((base) + (0x04C))
#define SOC_NPU_SCTRL_NPU_CTRL20_ADDR(base) ((base) + (0x050))
#define SOC_NPU_SCTRL_NPU_CTRL21_ADDR(base) ((base) + (0x054))
#define SOC_NPU_SCTRL_NPU_CTRL22_ADDR(base) ((base) + (0x058))
#define SOC_NPU_SCTRL_NPU_CTRL23_ADDR(base) ((base) + (0x05C))
#define SOC_NPU_SCTRL_NPU_CTRL24_ADDR(base) ((base) + (0x060))
#define SOC_NPU_SCTRL_NPU_CTRL25_ADDR(base) ((base) + (0x064))
#define SOC_NPU_SCTRL_NPU_CTRL26_ADDR(base) ((base) + (0x068))
#define SOC_NPU_SCTRL_NPU_CTRL27_ADDR(base) ((base) + (0x06C))
#define SOC_NPU_SCTRL_NPU_CTRL28_ADDR(base) ((base) + (0x070))
#define SOC_NPU_SCTRL_NPU_CTRL29_ADDR(base) ((base) + (0x074))
#define SOC_NPU_SCTRL_NPU_CTRL30_ADDR(base) ((base) + (0x078))
#define SOC_NPU_SCTRL_NPU_CTRL31_ADDR(base) ((base) + (0x07C))
#define SOC_NPU_SCTRL_NPU_CTRL32_ADDR(base) ((base) + (0x080))
#define SOC_NPU_SCTRL_NPU_CTRL33_ADDR(base) ((base) + (0x084))
#define SOC_NPU_SCTRL_NPU_CTRL34_ADDR(base) ((base) + (0x088))
#define SOC_NPU_SCTRL_NPU_CTRL35_ADDR(base) ((base) + (0x08C))
#define SOC_NPU_SCTRL_NPU_CTRL36_ADDR(base) ((base) + (0x090))
#define SOC_NPU_SCTRL_NPU_CTRL37_ADDR(base) ((base) + (0x094))
#define SOC_NPU_SCTRL_NPU_CTRL38_ADDR(base) ((base) + (0x098))
#define SOC_NPU_SCTRL_NPU_CTRL39_ADDR(base) ((base) + (0x09C))
#define SOC_NPU_SCTRL_NPU_CTRL40_ADDR(base) ((base) + (0x0A0))
#define SOC_NPU_SCTRL_NPU_CTRL41_ADDR(base) ((base) + (0x0A4))
#define SOC_NPU_SCTRL_NPU_CTRL42_ADDR(base) ((base) + (0x0A8))
#define SOC_NPU_SCTRL_NPU_CTRL43_ADDR(base) ((base) + (0x0AC))
#define SOC_NPU_SCTRL_NPU_STAT0_ADDR(base) ((base) + (0x800))
#define SOC_NPU_SCTRL_NPU_STAT1_ADDR(base) ((base) + (0x804))
#define SOC_NPU_SCTRL_NPU_STAT2_ADDR(base) ((base) + (0x808))
#define SOC_NPU_SCTRL_NPU_STAT3_ADDR(base) ((base) + (0x80C))
#define SOC_NPU_SCTRL_NPU_STAT4_ADDR(base) ((base) + (0x810))
#define SOC_NPU_SCTRL_NPU_STAT5_ADDR(base) ((base) + (0x814))
#define SOC_NPU_SCTRL_NPU_STAT6_ADDR(base) ((base) + (0x818))
#define SOC_NPU_SCTRL_NPU_STAT7_ADDR(base) ((base) + (0x81C))
#define SOC_NPU_SCTRL_NPU_STAT8_ADDR(base) ((base) + (0x820))
#define SOC_NPU_SCTRL_NPU_STAT9_ADDR(base) ((base) + (0x824))
#define SOC_NPU_SCTRL_NPU_STAT10_ADDR(base) ((base) + (0x828))
#define SOC_NPU_SCTRL_NPU_STAT11_ADDR(base) ((base) + (0x82C))
#define SOC_NPU_SCTRL_NPU_STAT12_ADDR(base) ((base) + (0x830))
#define SOC_NPU_SCTRL_NPU_STAT13_ADDR(base) ((base) + (0x834))
#define SOC_NPU_SCTRL_NPU_STAT14_ADDR(base) ((base) + (0x838))
#define SOC_NPU_SCTRL_NPU_STAT15_ADDR(base) ((base) + (0x83C))
#define SOC_NPU_SCTRL_NPU_STAT16_ADDR(base) ((base) + (0x840))
#define SOC_NPU_SCTRL_NPU_STAT17_ADDR(base) ((base) + (0x844))
#define SOC_NPU_SCTRL_NPU_STAT18_ADDR(base) ((base) + (0x848))
#define SOC_NPU_SCTRL_NPU_STAT19_ADDR(base) ((base) + (0x84C))
#define SOC_NPU_SCTRL_NPU_STAT20_ADDR(base) ((base) + (0x850))
#define SOC_NPU_SCTRL_NPU_STAT21_ADDR(base) ((base) + (0x854))
#define SOC_NPU_SCTRL_NPU_STAT22_ADDR(base) ((base) + (0x858))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_ctrl_s_aic0_top : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL0_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL0_mem_ctrl_s_aic0_top_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL0_mem_ctrl_s_aic0_top_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_ctrl_d1w2r_aic0_top : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL1_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL1_mem_ctrl_d1w2r_aic0_top_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL1_mem_ctrl_d1w2r_aic0_top_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_ctrl_s_aic1_top : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL2_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL2_mem_ctrl_s_aic1_top_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL2_mem_ctrl_s_aic1_top_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_ctrl_d1w2r_aic1_top : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL3_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL3_mem_ctrl_d1w2r_aic1_top_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL3_mem_ctrl_d1w2r_aic1_top_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_ctrl_s_sysdma : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL4_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL4_mem_ctrl_s_sysdma_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL4_mem_ctrl_s_sysdma_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_ctrl_d1w2r_sysdma : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL5_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL5_mem_ctrl_d1w2r_sysdma_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL5_mem_ctrl_d1w2r_sysdma_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm0_clk_div : 6;
        unsigned int hpm0_en : 1;
        unsigned int hpmx0_en : 1;
        unsigned int hpm1_clk_div : 6;
        unsigned int hpm1_en : 1;
        unsigned int hpmx1_en : 1;
        unsigned int hpm2_clk_div : 6;
        unsigned int hpm2_en : 1;
        unsigned int hpmx2_en : 1;
        unsigned int reserved : 8;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL6_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm0_clk_div_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm0_clk_div_END (5)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm0_en_START (6)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm0_en_END (6)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpmx0_en_START (7)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpmx0_en_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm1_clk_div_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm1_clk_div_END (13)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm1_en_START (14)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm1_en_END (14)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpmx1_en_START (15)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpmx1_en_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm2_clk_div_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm2_clk_div_END (21)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm2_en_START (22)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpm2_en_END (22)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpmx2_en_START (23)
#define SOC_NPU_SCTRL_NPU_CTRL6_hpmx2_en_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm0_opc_min_en : 1;
        unsigned int hpmx0_opc_min_en : 1;
        unsigned int hpm1_opc_min_en : 1;
        unsigned int hpmx1_opc_min_en : 1;
        unsigned int hpm2_opc_min_en : 1;
        unsigned int hpmx2_opc_min_en : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL7_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL7_hpm0_opc_min_en_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpm0_opc_min_en_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpmx0_opc_min_en_START (1)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpmx0_opc_min_en_END (1)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpm1_opc_min_en_START (2)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpm1_opc_min_en_END (2)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpmx1_opc_min_en_START (3)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpmx1_opc_min_en_END (3)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpm2_opc_min_en_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpm2_opc_min_en_END (4)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpmx2_opc_min_en_START (5)
#define SOC_NPU_SCTRL_NPU_CTRL7_hpmx2_opc_min_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int atgm_aicore_sel : 3;
        unsigned int reserved_0 : 1;
        unsigned int sc_monitor_rst_req : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL8_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL8_atgm_aicore_sel_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL8_atgm_aicore_sel_END (2)
#define SOC_NPU_SCTRL_NPU_CTRL8_sc_monitor_rst_req_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL8_sc_monitor_rst_req_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ts_rd_bwc_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int ts_rd_bwc_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int ts_rd_bwc_en : 1;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL9_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL9_ts_rd_bwc_saturation_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL9_ts_rd_bwc_saturation_END (13)
#define SOC_NPU_SCTRL_NPU_CTRL9_ts_rd_bwc_bandwidth_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL9_ts_rd_bwc_bandwidth_END (28)
#define SOC_NPU_SCTRL_NPU_CTRL9_ts_rd_bwc_en_START (31)
#define SOC_NPU_SCTRL_NPU_CTRL9_ts_rd_bwc_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ts_wr_bwc_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int ts_wr_bwc_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int ts_wr_bwc_en : 1;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL10_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL10_ts_wr_bwc_saturation_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL10_ts_wr_bwc_saturation_END (13)
#define SOC_NPU_SCTRL_NPU_CTRL10_ts_wr_bwc_bandwidth_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL10_ts_wr_bwc_bandwidth_END (28)
#define SOC_NPU_SCTRL_NPU_CTRL10_ts_wr_bwc_en_START (31)
#define SOC_NPU_SCTRL_NPU_CTRL10_ts_wr_bwc_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ts_icfg_ostd0_lvl : 8;
        unsigned int ts_icfg_ostd1_lvl : 8;
        unsigned int flux_en_aicore0 : 1;
        unsigned int ostd_en_aicore0 : 1;
        unsigned int flux_en_ts : 1;
        unsigned int ostd_en_ts : 1;
        unsigned int reserved_0 : 2;
        unsigned int flux_en_sdma : 1;
        unsigned int ostd_en_sdma : 1;
        unsigned int flux_en_aicore1 : 1;
        unsigned int ostd_en_aicore1 : 1;
        unsigned int reserved_1 : 4;
        unsigned int ts_icfg_ostd0_type : 1;
        unsigned int reserved_2 : 1;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL11_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL11_ts_icfg_ostd0_lvl_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL11_ts_icfg_ostd0_lvl_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL11_ts_icfg_ostd1_lvl_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL11_ts_icfg_ostd1_lvl_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL11_flux_en_aicore0_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL11_flux_en_aicore0_END (16)
#define SOC_NPU_SCTRL_NPU_CTRL11_ostd_en_aicore0_START (17)
#define SOC_NPU_SCTRL_NPU_CTRL11_ostd_en_aicore0_END (17)
#define SOC_NPU_SCTRL_NPU_CTRL11_flux_en_ts_START (18)
#define SOC_NPU_SCTRL_NPU_CTRL11_flux_en_ts_END (18)
#define SOC_NPU_SCTRL_NPU_CTRL11_ostd_en_ts_START (19)
#define SOC_NPU_SCTRL_NPU_CTRL11_ostd_en_ts_END (19)
#define SOC_NPU_SCTRL_NPU_CTRL11_flux_en_sdma_START (22)
#define SOC_NPU_SCTRL_NPU_CTRL11_flux_en_sdma_END (22)
#define SOC_NPU_SCTRL_NPU_CTRL11_ostd_en_sdma_START (23)
#define SOC_NPU_SCTRL_NPU_CTRL11_ostd_en_sdma_END (23)
#define SOC_NPU_SCTRL_NPU_CTRL11_flux_en_aicore1_START (24)
#define SOC_NPU_SCTRL_NPU_CTRL11_flux_en_aicore1_END (24)
#define SOC_NPU_SCTRL_NPU_CTRL11_ostd_en_aicore1_START (25)
#define SOC_NPU_SCTRL_NPU_CTRL11_ostd_en_aicore1_END (25)
#define SOC_NPU_SCTRL_NPU_CTRL11_ts_icfg_ostd0_type_START (30)
#define SOC_NPU_SCTRL_NPU_CTRL11_ts_icfg_ostd0_type_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_wrap_pa_cfg_valid : 1;
        unsigned int npu_wrap_pa_en : 1;
        unsigned int reserved_0 : 2;
        unsigned int npu_wrap_pa_cmd : 2;
        unsigned int reserved_1 : 2;
        unsigned int npu_wrap_pa_ctrl : 6;
        unsigned int reserved_2 : 2;
        unsigned int npu_wrap_pa_msg : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL12_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_cfg_valid_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_cfg_valid_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_en_START (1)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_en_END (1)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_cmd_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_cmd_END (5)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_ctrl_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_ctrl_END (13)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_msg_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL12_npu_wrap_pa_msg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_pi_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL13_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL13_npu_pi_en_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL13_npu_pi_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_pi_prot_cnt_th : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL14_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL14_npu_pi_prot_cnt_th_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL14_npu_pi_prot_cnt_th_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aicore0_cube_start_cfg : 16;
        unsigned int aicore1_cube_start_cfg : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL15_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL15_aicore0_cube_start_cfg_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL15_aicore0_cube_start_cfg_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL15_aicore1_cube_start_cfg_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL15_aicore1_cube_start_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aicore0_fixp_start_cfg : 16;
        unsigned int aicore1_fixp_start_cfg : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL16_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL16_aicore0_fixp_start_cfg_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL16_aicore0_fixp_start_cfg_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL16_aicore1_fixp_start_cfg_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL16_aicore1_fixp_start_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aicore0_mte_start_cfg : 16;
        unsigned int aicore1_mte_start_cfg : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL17_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL17_aicore0_mte_start_cfg_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL17_aicore0_mte_start_cfg_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL17_aicore1_mte_start_cfg_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL17_aicore1_mte_start_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aicore0_vec_start_cfg : 16;
        unsigned int aicore1_vec_start_cfg : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL18_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL18_aicore0_vec_start_cfg_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL18_aicore0_vec_start_cfg_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL18_aicore1_vec_start_cfg_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL18_aicore1_vec_start_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cube_start_pulse_width_cfg : 4;
        unsigned int fixp_start_pulse_width_cfg : 4;
        unsigned int mte_start_pulse_width_cfg : 4;
        unsigned int vec_start_pulse_width_cfg : 4;
        unsigned int reserved : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL19_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL19_cube_start_pulse_width_cfg_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL19_cube_start_pulse_width_cfg_END (3)
#define SOC_NPU_SCTRL_NPU_CTRL19_fixp_start_pulse_width_cfg_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL19_fixp_start_pulse_width_cfg_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL19_mte_start_pulse_width_cfg_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL19_mte_start_pulse_width_cfg_END (11)
#define SOC_NPU_SCTRL_NPU_CTRL19_vec_start_pulse_width_cfg_START (12)
#define SOC_NPU_SCTRL_NPU_CTRL19_vec_start_pulse_width_cfg_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_det_en0 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL20_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL20_ovc_det_en0_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL20_ovc_det_en0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_det_en1 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL21_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL21_ovc_det_en1_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL21_ovc_det_en1_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_det_en2 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL22_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL22_ovc_det_en2_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL22_ovc_det_en2_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_int_det_th : 32;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL23_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL23_ovc_int_det_th_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL23_ovc_int_det_th_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_pcr_det_th0 : 32;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL24_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL24_ovc_pcr_det_th0_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL24_ovc_pcr_det_th0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_pcr_det_th1 : 32;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL25_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL25_ovc_pcr_det_th1_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL25_ovc_pcr_det_th1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_pcr_det_th2 : 32;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL26_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL26_ovc_pcr_det_th2_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL26_ovc_pcr_det_th2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_pcr_det_th3 : 32;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL27_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL27_ovc_pcr_det_th3_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL27_ovc_pcr_det_th3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int map_sw_sample_th : 8;
        unsigned int reserved_0 : 8;
        unsigned int pcr_cg_sample_th : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL28_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL28_map_sw_sample_th_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL28_map_sw_sample_th_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL28_pcr_cg_sample_th_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL28_pcr_cg_sample_th_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_int_lpmcu_mask : 1;
        unsigned int reserved_0 : 3;
        unsigned int ovc_int_acpu_mask : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL29_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL29_ovc_int_lpmcu_mask_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL29_ovc_int_lpmcu_mask_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL29_ovc_int_acpu_mask_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL29_ovc_int_acpu_mask_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_int_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL30_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL30_ovc_int_clr_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL30_ovc_int_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_arcache : 4;
        unsigned int npu_awcache : 4;
        unsigned int npu_arhint : 4;
        unsigned int npu_awhint : 4;
        unsigned int npu_ardomain : 2;
        unsigned int reserved_0 : 2;
        unsigned int npu_awdomain : 2;
        unsigned int reserved_1 : 2;
        unsigned int npu_ptl_as_full_r : 1;
        unsigned int reserved_2 : 3;
        unsigned int npu_ptl_as_full_w : 1;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL31_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_arcache_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_arcache_END (3)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_awcache_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_awcache_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_arhint_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_arhint_END (11)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_awhint_START (12)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_awhint_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_ardomain_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_ardomain_END (17)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_awdomain_START (20)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_awdomain_END (21)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_ptl_as_full_r_START (24)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_ptl_as_full_r_END (24)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_ptl_as_full_w_START (28)
#define SOC_NPU_SCTRL_NPU_CTRL31_npu_ptl_as_full_w_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int tiny_ts_ar_gid : 4;
        unsigned int tiny_ts_aw_gid : 4;
        unsigned int reserved_0 : 8;
        unsigned int ts_cfg_256to128_ckg_en : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL32_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL32_tiny_ts_ar_gid_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL32_tiny_ts_ar_gid_END (3)
#define SOC_NPU_SCTRL_NPU_CTRL32_tiny_ts_aw_gid_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL32_tiny_ts_aw_gid_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL32_ts_cfg_256to128_ckg_en_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL32_ts_cfg_256to128_ckg_en_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic0_ckg_bypass_1to2_0 : 1;
        unsigned int aic0_ckg_bypass_1to2_1 : 1;
        unsigned int aic0_ckg_bypass_2to1_0 : 1;
        unsigned int aic0_ckg_bypass_2to1_1 : 1;
        unsigned int aic0_slv_sec_1to2_0 : 2;
        unsigned int aic0_slv_sec_1to2_1 : 2;
        unsigned int aic0_slv_sec_2to1_0 : 1;
        unsigned int aic0_slv_sec_2to1_1 : 1;
        unsigned int reserved_0 : 2;
        unsigned int aic0_ckg_en_x2x_0 : 1;
        unsigned int aic0_ckg_en_x2x_1 : 1;
        unsigned int aic0_gs_clk_en : 1;
        unsigned int aiv0_gs_clk_en : 1;
        unsigned int aic0_top_1to2_clk_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int aic0_top_1to2_slv_sec : 2;
        unsigned int reserved_2 : 2;
        unsigned int aic0_dlock_and_dbg_clr_1to2_0 : 1;
        unsigned int aic0_dlock_and_dbg_clr_1to2_1 : 1;
        unsigned int aic0_dlock_and_dbg_clr_2to1_0 : 1;
        unsigned int aic0_dlock_and_dbg_clr_2to1_1 : 1;
        unsigned int aic0_top_1to2_dlock_and_dbg_clr : 1;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL33_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_bypass_1to2_0_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_bypass_1to2_0_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_bypass_1to2_1_START (1)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_bypass_1to2_1_END (1)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_bypass_2to1_0_START (2)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_bypass_2to1_0_END (2)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_bypass_2to1_1_START (3)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_bypass_2to1_1_END (3)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_slv_sec_1to2_0_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_slv_sec_1to2_0_END (5)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_slv_sec_1to2_1_START (6)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_slv_sec_1to2_1_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_slv_sec_2to1_0_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_slv_sec_2to1_0_END (8)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_slv_sec_2to1_1_START (9)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_slv_sec_2to1_1_END (9)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_en_x2x_0_START (12)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_en_x2x_0_END (12)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_en_x2x_1_START (13)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_ckg_en_x2x_1_END (13)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_gs_clk_en_START (14)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_gs_clk_en_END (14)
#define SOC_NPU_SCTRL_NPU_CTRL33_aiv0_gs_clk_en_START (15)
#define SOC_NPU_SCTRL_NPU_CTRL33_aiv0_gs_clk_en_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_top_1to2_clk_bypass_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_top_1to2_clk_bypass_END (16)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_top_1to2_slv_sec_START (20)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_top_1to2_slv_sec_END (21)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_dlock_and_dbg_clr_1to2_0_START (24)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_dlock_and_dbg_clr_1to2_0_END (24)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_dlock_and_dbg_clr_1to2_1_START (25)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_dlock_and_dbg_clr_1to2_1_END (25)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_dlock_and_dbg_clr_2to1_0_START (26)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_dlock_and_dbg_clr_2to1_0_END (26)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_dlock_and_dbg_clr_2to1_1_START (27)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_dlock_and_dbg_clr_2to1_1_END (27)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_top_1to2_dlock_and_dbg_clr_START (28)
#define SOC_NPU_SCTRL_NPU_CTRL33_aic0_top_1to2_dlock_and_dbg_clr_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic1_ckg_bypass_1to2_0 : 1;
        unsigned int aic1_ckg_bypass_1to2_1 : 1;
        unsigned int aic1_ckg_bypass_2to1_0 : 1;
        unsigned int aic1_ckg_bypass_2to1_1 : 1;
        unsigned int aic1_slv_sec_1to2_0 : 2;
        unsigned int aic1_slv_sec_1to2_1 : 2;
        unsigned int aic1_slv_sec_2to1_0 : 1;
        unsigned int aic1_slv_sec_2to1_1 : 1;
        unsigned int reserved_0 : 2;
        unsigned int aic1_ckg_en_x2x_0 : 1;
        unsigned int aic1_ckg_en_x2x_1 : 1;
        unsigned int aic1_gs_clk_en : 1;
        unsigned int aiv1_gs_clk_en : 1;
        unsigned int aic1_top_1to2_clk_bypass : 1;
        unsigned int reserved_1 : 3;
        unsigned int aic1_top_1to2_slv_sec : 2;
        unsigned int reserved_2 : 2;
        unsigned int aic1_dlock_and_dbg_clr_1to2_0 : 1;
        unsigned int aic1_dlock_and_dbg_clr_1to2_1 : 1;
        unsigned int aic1_dlock_and_dbg_clr_2to1_0 : 1;
        unsigned int aic1_dlock_and_dbg_clr_2to1_1 : 1;
        unsigned int aic1_top_1to2_dlock_and_dbg_clr : 1;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL34_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_bypass_1to2_0_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_bypass_1to2_0_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_bypass_1to2_1_START (1)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_bypass_1to2_1_END (1)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_bypass_2to1_0_START (2)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_bypass_2to1_0_END (2)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_bypass_2to1_1_START (3)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_bypass_2to1_1_END (3)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_slv_sec_1to2_0_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_slv_sec_1to2_0_END (5)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_slv_sec_1to2_1_START (6)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_slv_sec_1to2_1_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_slv_sec_2to1_0_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_slv_sec_2to1_0_END (8)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_slv_sec_2to1_1_START (9)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_slv_sec_2to1_1_END (9)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_en_x2x_0_START (12)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_en_x2x_0_END (12)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_en_x2x_1_START (13)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_ckg_en_x2x_1_END (13)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_gs_clk_en_START (14)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_gs_clk_en_END (14)
#define SOC_NPU_SCTRL_NPU_CTRL34_aiv1_gs_clk_en_START (15)
#define SOC_NPU_SCTRL_NPU_CTRL34_aiv1_gs_clk_en_END (15)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_top_1to2_clk_bypass_START (16)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_top_1to2_clk_bypass_END (16)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_top_1to2_slv_sec_START (20)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_top_1to2_slv_sec_END (21)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_dlock_and_dbg_clr_1to2_0_START (24)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_dlock_and_dbg_clr_1to2_0_END (24)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_dlock_and_dbg_clr_1to2_1_START (25)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_dlock_and_dbg_clr_1to2_1_END (25)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_dlock_and_dbg_clr_2to1_0_START (26)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_dlock_and_dbg_clr_2to1_0_END (26)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_dlock_and_dbg_clr_2to1_1_START (27)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_dlock_and_dbg_clr_2to1_1_END (27)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_top_1to2_dlock_and_dbg_clr_START (28)
#define SOC_NPU_SCTRL_NPU_CTRL34_aic1_top_1to2_dlock_and_dbg_clr_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int easc0_int_msk : 1;
        unsigned int easc1_int_msk : 1;
        unsigned int easc2_int_msk : 1;
        unsigned int aicore0_rst_int_msk : 1;
        unsigned int aicore0_rst_rls_int_msk : 1;
        unsigned int aicore1_rst_int_msk : 1;
        unsigned int aicore1_rst_rls_int_msk : 1;
        unsigned int wdog_res_ts_int_msk : 1;
        unsigned int easc0_res_int_msk : 1;
        unsigned int easc0_res_rls_int_msk : 1;
        unsigned int easc1_res_int_msk : 1;
        unsigned int easc1_res_rls_int_msk : 1;
        unsigned int easc2_res_int_msk : 1;
        unsigned int easc2_res_rls_int_msk : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL35_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL35_easc0_int_msk_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc0_int_msk_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc1_int_msk_START (1)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc1_int_msk_END (1)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc2_int_msk_START (2)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc2_int_msk_END (2)
#define SOC_NPU_SCTRL_NPU_CTRL35_aicore0_rst_int_msk_START (3)
#define SOC_NPU_SCTRL_NPU_CTRL35_aicore0_rst_int_msk_END (3)
#define SOC_NPU_SCTRL_NPU_CTRL35_aicore0_rst_rls_int_msk_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL35_aicore0_rst_rls_int_msk_END (4)
#define SOC_NPU_SCTRL_NPU_CTRL35_aicore1_rst_int_msk_START (5)
#define SOC_NPU_SCTRL_NPU_CTRL35_aicore1_rst_int_msk_END (5)
#define SOC_NPU_SCTRL_NPU_CTRL35_aicore1_rst_rls_int_msk_START (6)
#define SOC_NPU_SCTRL_NPU_CTRL35_aicore1_rst_rls_int_msk_END (6)
#define SOC_NPU_SCTRL_NPU_CTRL35_wdog_res_ts_int_msk_START (7)
#define SOC_NPU_SCTRL_NPU_CTRL35_wdog_res_ts_int_msk_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc0_res_int_msk_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc0_res_int_msk_END (8)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc0_res_rls_int_msk_START (9)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc0_res_rls_int_msk_END (9)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc1_res_int_msk_START (10)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc1_res_int_msk_END (10)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc1_res_rls_int_msk_START (11)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc1_res_rls_int_msk_END (11)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc2_res_int_msk_START (12)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc2_res_int_msk_END (12)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc2_res_rls_int_msk_START (13)
#define SOC_NPU_SCTRL_NPU_CTRL35_easc2_res_rls_int_msk_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int easc2_rst_int_clr : 1;
        unsigned int easc2_rst_rls_int_clr : 1;
        unsigned int easc1_rst_int_clr : 1;
        unsigned int easc1_rst_rls_int_clr : 1;
        unsigned int easc0_rst_int_clr : 1;
        unsigned int easc0_rst_rls_int_clr : 1;
        unsigned int aicore0_rst_int_clr : 1;
        unsigned int aicore0_rst_rls_int_clr : 1;
        unsigned int aicore1_rst_int_clr : 1;
        unsigned int aicore1_rst_rls_int_clr : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL36_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL36_easc2_rst_int_clr_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc2_rst_int_clr_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc2_rst_rls_int_clr_START (1)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc2_rst_rls_int_clr_END (1)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc1_rst_int_clr_START (2)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc1_rst_int_clr_END (2)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc1_rst_rls_int_clr_START (3)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc1_rst_rls_int_clr_END (3)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc0_rst_int_clr_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc0_rst_int_clr_END (4)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc0_rst_rls_int_clr_START (5)
#define SOC_NPU_SCTRL_NPU_CTRL36_easc0_rst_rls_int_clr_END (5)
#define SOC_NPU_SCTRL_NPU_CTRL36_aicore0_rst_int_clr_START (6)
#define SOC_NPU_SCTRL_NPU_CTRL36_aicore0_rst_int_clr_END (6)
#define SOC_NPU_SCTRL_NPU_CTRL36_aicore0_rst_rls_int_clr_START (7)
#define SOC_NPU_SCTRL_NPU_CTRL36_aicore0_rst_rls_int_clr_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL36_aicore1_rst_int_clr_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL36_aicore1_rst_int_clr_END (8)
#define SOC_NPU_SCTRL_NPU_CTRL36_aicore1_rst_rls_int_clr_START (9)
#define SOC_NPU_SCTRL_NPU_CTRL36_aicore1_rst_rls_int_clr_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int bp_en_pulse : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL37_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL37_bp_en_pulse_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL37_bp_en_pulse_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_npu2qice_sw_done_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL38_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL38_intr_npu2qice_sw_done_clr_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL38_intr_npu2qice_sw_done_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu2qice_sw_intr_acpu_mask : 1;
        unsigned int reserved_0 : 3;
        unsigned int npu2qice_sw_intr_npucpu_mask : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL39_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL39_npu2qice_sw_intr_acpu_mask_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL39_npu2qice_sw_intr_acpu_mask_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL39_npu2qice_sw_intr_npucpu_mask_START (4)
#define SOC_NPU_SCTRL_NPU_CTRL39_npu2qice_sw_intr_npucpu_mask_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ts_2to1_dlock_and_dbg_clr : 1;
        unsigned int ts_1to2_dlock_and_dbg_clr : 1;
        unsigned int ts_4to5_dlock_and_dbg_clr : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL40_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL40_ts_2to1_dlock_and_dbg_clr_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL40_ts_2to1_dlock_and_dbg_clr_END (0)
#define SOC_NPU_SCTRL_NPU_CTRL40_ts_1to2_dlock_and_dbg_clr_START (1)
#define SOC_NPU_SCTRL_NPU_CTRL40_ts_1to2_dlock_and_dbg_clr_END (1)
#define SOC_NPU_SCTRL_NPU_CTRL40_ts_4to5_dlock_and_dbg_clr_START (2)
#define SOC_NPU_SCTRL_NPU_CTRL40_ts_4to5_dlock_and_dbg_clr_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_ostd_lvl : 8;
        unsigned int ostd_monitor_en : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL41_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL41_icfg_ostd_lvl_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL41_icfg_ostd_lvl_END (7)
#define SOC_NPU_SCTRL_NPU_CTRL41_ostd_monitor_en_START (8)
#define SOC_NPU_SCTRL_NPU_CTRL41_ostd_monitor_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_ctrl_spua_aic0_top : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL42_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL42_mem_ctrl_spua_aic0_top_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL42_mem_ctrl_spua_aic0_top_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ec_cfg : 32;
    } reg;
} SOC_NPU_SCTRL_NPU_CTRL43_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_CTRL43_ec_cfg_START (0)
#define SOC_NPU_SCTRL_NPU_CTRL43_ec_cfg_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm0_opc : 10;
        unsigned int hpm0_opc_vld : 1;
        unsigned int reserved_0 : 5;
        unsigned int hpmx0_opc : 10;
        unsigned int hpmx0_opc_vld : 1;
        unsigned int reserved_1 : 5;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT0_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT0_hpm0_opc_START (0)
#define SOC_NPU_SCTRL_NPU_STAT0_hpm0_opc_END (9)
#define SOC_NPU_SCTRL_NPU_STAT0_hpm0_opc_vld_START (10)
#define SOC_NPU_SCTRL_NPU_STAT0_hpm0_opc_vld_END (10)
#define SOC_NPU_SCTRL_NPU_STAT0_hpmx0_opc_START (16)
#define SOC_NPU_SCTRL_NPU_STAT0_hpmx0_opc_END (25)
#define SOC_NPU_SCTRL_NPU_STAT0_hpmx0_opc_vld_START (26)
#define SOC_NPU_SCTRL_NPU_STAT0_hpmx0_opc_vld_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm0_opc_min : 10;
        unsigned int reserved_0 : 6;
        unsigned int hpmx0_opc_min : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT1_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT1_hpm0_opc_min_START (0)
#define SOC_NPU_SCTRL_NPU_STAT1_hpm0_opc_min_END (9)
#define SOC_NPU_SCTRL_NPU_STAT1_hpmx0_opc_min_START (16)
#define SOC_NPU_SCTRL_NPU_STAT1_hpmx0_opc_min_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm1_opc : 10;
        unsigned int hpm1_opc_vld : 1;
        unsigned int reserved_0 : 5;
        unsigned int hpmx1_opc : 10;
        unsigned int hpmx1_opc_vld : 1;
        unsigned int reserved_1 : 5;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT2_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT2_hpm1_opc_START (0)
#define SOC_NPU_SCTRL_NPU_STAT2_hpm1_opc_END (9)
#define SOC_NPU_SCTRL_NPU_STAT2_hpm1_opc_vld_START (10)
#define SOC_NPU_SCTRL_NPU_STAT2_hpm1_opc_vld_END (10)
#define SOC_NPU_SCTRL_NPU_STAT2_hpmx1_opc_START (16)
#define SOC_NPU_SCTRL_NPU_STAT2_hpmx1_opc_END (25)
#define SOC_NPU_SCTRL_NPU_STAT2_hpmx1_opc_vld_START (26)
#define SOC_NPU_SCTRL_NPU_STAT2_hpmx1_opc_vld_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm1_opc_min : 10;
        unsigned int reserved_0 : 6;
        unsigned int hpmx1_opc_min : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT3_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT3_hpm1_opc_min_START (0)
#define SOC_NPU_SCTRL_NPU_STAT3_hpm1_opc_min_END (9)
#define SOC_NPU_SCTRL_NPU_STAT3_hpmx1_opc_min_START (16)
#define SOC_NPU_SCTRL_NPU_STAT3_hpmx1_opc_min_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm2_opc : 10;
        unsigned int hpm2_opc_vld : 1;
        unsigned int reserved_0 : 5;
        unsigned int hpmx2_opc : 10;
        unsigned int hpmx2_opc_vld : 1;
        unsigned int reserved_1 : 5;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT4_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT4_hpm2_opc_START (0)
#define SOC_NPU_SCTRL_NPU_STAT4_hpm2_opc_END (9)
#define SOC_NPU_SCTRL_NPU_STAT4_hpm2_opc_vld_START (10)
#define SOC_NPU_SCTRL_NPU_STAT4_hpm2_opc_vld_END (10)
#define SOC_NPU_SCTRL_NPU_STAT4_hpmx2_opc_START (16)
#define SOC_NPU_SCTRL_NPU_STAT4_hpmx2_opc_END (25)
#define SOC_NPU_SCTRL_NPU_STAT4_hpmx2_opc_vld_START (26)
#define SOC_NPU_SCTRL_NPU_STAT4_hpmx2_opc_vld_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm2_opc_min : 10;
        unsigned int reserved_0 : 6;
        unsigned int hpmx2_opc_min : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT5_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT5_hpm2_opc_min_START (0)
#define SOC_NPU_SCTRL_NPU_STAT5_hpm2_opc_min_END (9)
#define SOC_NPU_SCTRL_NPU_STAT5_hpmx2_opc_min_START (16)
#define SOC_NPU_SCTRL_NPU_STAT5_hpmx2_opc_min_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_pasensor_hci_a_out : 9;
        unsigned int reserved_0 : 3;
        unsigned int npu_pasensor_hci_o_out : 9;
        unsigned int reserved_1 : 3;
        unsigned int npu_pasensor_valid : 1;
        unsigned int reserved_2 : 3;
        unsigned int npu_pasensor_cfg_ready : 1;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT6_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT6_npu_pasensor_hci_a_out_START (0)
#define SOC_NPU_SCTRL_NPU_STAT6_npu_pasensor_hci_a_out_END (8)
#define SOC_NPU_SCTRL_NPU_STAT6_npu_pasensor_hci_o_out_START (12)
#define SOC_NPU_SCTRL_NPU_STAT6_npu_pasensor_hci_o_out_END (20)
#define SOC_NPU_SCTRL_NPU_STAT6_npu_pasensor_valid_START (24)
#define SOC_NPU_SCTRL_NPU_STAT6_npu_pasensor_valid_END (24)
#define SOC_NPU_SCTRL_NPU_STAT6_npu_pasensor_cfg_ready_START (28)
#define SOC_NPU_SCTRL_NPU_STAT6_npu_pasensor_cfg_ready_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_pasensor_nbti_a_out : 9;
        unsigned int reserved_0 : 3;
        unsigned int npu_pasensor_nbti_o_out : 9;
        unsigned int reserved_1 : 11;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT7_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT7_npu_pasensor_nbti_a_out_START (0)
#define SOC_NPU_SCTRL_NPU_STAT7_npu_pasensor_nbti_a_out_END (8)
#define SOC_NPU_SCTRL_NPU_STAT7_npu_pasensor_nbti_o_out_START (12)
#define SOC_NPU_SCTRL_NPU_STAT7_npu_pasensor_nbti_o_out_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ts_axi_2to1_dlock_slv : 2;
        unsigned int reserved_0 : 2;
        unsigned int ts_axi_2to1_dlock_mst : 4;
        unsigned int ts_2to1_dbg_ar_err : 4;
        unsigned int ts_2to1_dbg_aw_err : 4;
        unsigned int ts_axi_1to2_dlock_slv : 4;
        unsigned int ts_axi_1to2_dlock_mst : 2;
        unsigned int reserved_1 : 2;
        unsigned int ts_1to2_dbg_ar_err : 4;
        unsigned int ts_1to2_dbg_aw_err : 4;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT8_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT8_ts_axi_2to1_dlock_slv_START (0)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_axi_2to1_dlock_slv_END (1)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_axi_2to1_dlock_mst_START (4)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_axi_2to1_dlock_mst_END (7)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_2to1_dbg_ar_err_START (8)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_2to1_dbg_ar_err_END (11)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_2to1_dbg_aw_err_START (12)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_2to1_dbg_aw_err_END (15)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_axi_1to2_dlock_slv_START (16)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_axi_1to2_dlock_slv_END (19)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_axi_1to2_dlock_mst_START (20)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_axi_1to2_dlock_mst_END (21)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_1to2_dbg_ar_err_START (24)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_1to2_dbg_ar_err_END (27)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_1to2_dbg_aw_err_START (28)
#define SOC_NPU_SCTRL_NPU_STAT8_ts_1to2_dbg_aw_err_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ts_axi_4to5_dlock_slv : 10;
        unsigned int reserved_0 : 2;
        unsigned int ts_axi_4to5_dlock_mst : 8;
        unsigned int ts_4to5_dbg_ar_err : 4;
        unsigned int ts_4to5_dbg_aw_err : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT9_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT9_ts_axi_4to5_dlock_slv_START (0)
#define SOC_NPU_SCTRL_NPU_STAT9_ts_axi_4to5_dlock_slv_END (9)
#define SOC_NPU_SCTRL_NPU_STAT9_ts_axi_4to5_dlock_mst_START (12)
#define SOC_NPU_SCTRL_NPU_STAT9_ts_axi_4to5_dlock_mst_END (19)
#define SOC_NPU_SCTRL_NPU_STAT9_ts_4to5_dbg_ar_err_START (20)
#define SOC_NPU_SCTRL_NPU_STAT9_ts_4to5_dbg_ar_err_END (23)
#define SOC_NPU_SCTRL_NPU_STAT9_ts_4to5_dbg_aw_err_START (24)
#define SOC_NPU_SCTRL_NPU_STAT9_ts_4to5_dbg_aw_err_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic0_hiaxi_idle_2to1_0 : 1;
        unsigned int aic0_hiaxi_idle_2to1_1 : 1;
        unsigned int aic0_hiaxi_idle_1to2_0 : 1;
        unsigned int aic0_hiaxi_idle_1to2_1 : 1;
        unsigned int aiv0_gs_himinibus_idle : 1;
        unsigned int aic0_gs_himinibus_idle : 1;
        unsigned int aic0_x2x_idle_0 : 1;
        unsigned int aic0_x2x_idle_1 : 1;
        unsigned int aic1_hiaxi_idle_2to1_0 : 1;
        unsigned int aic1_hiaxi_idle_2to1_1 : 1;
        unsigned int aic1_hiaxi_idle_1to2_0 : 1;
        unsigned int aic1_hiaxi_idle_1to2_1 : 1;
        unsigned int aiv1_gs_himinibus_idle : 1;
        unsigned int aic1_gs_himinibus_idle : 1;
        unsigned int aic1_x2x_idle_0 : 1;
        unsigned int aic1_x2x_idle_1 : 1;
        unsigned int ts_cfg_256to128_x2x_idle : 1;
        unsigned int aic0_top_1to2_hiaxi_idle : 1;
        unsigned int aic1_top_1to2_hiaxi_idle : 1;
        unsigned int reserved : 13;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT10_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_hiaxi_idle_2to1_0_START (0)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_hiaxi_idle_2to1_0_END (0)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_hiaxi_idle_2to1_1_START (1)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_hiaxi_idle_2to1_1_END (1)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_hiaxi_idle_1to2_0_START (2)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_hiaxi_idle_1to2_0_END (2)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_hiaxi_idle_1to2_1_START (3)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_hiaxi_idle_1to2_1_END (3)
#define SOC_NPU_SCTRL_NPU_STAT10_aiv0_gs_himinibus_idle_START (4)
#define SOC_NPU_SCTRL_NPU_STAT10_aiv0_gs_himinibus_idle_END (4)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_gs_himinibus_idle_START (5)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_gs_himinibus_idle_END (5)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_x2x_idle_0_START (6)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_x2x_idle_0_END (6)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_x2x_idle_1_START (7)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_x2x_idle_1_END (7)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_hiaxi_idle_2to1_0_START (8)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_hiaxi_idle_2to1_0_END (8)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_hiaxi_idle_2to1_1_START (9)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_hiaxi_idle_2to1_1_END (9)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_hiaxi_idle_1to2_0_START (10)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_hiaxi_idle_1to2_0_END (10)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_hiaxi_idle_1to2_1_START (11)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_hiaxi_idle_1to2_1_END (11)
#define SOC_NPU_SCTRL_NPU_STAT10_aiv1_gs_himinibus_idle_START (12)
#define SOC_NPU_SCTRL_NPU_STAT10_aiv1_gs_himinibus_idle_END (12)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_gs_himinibus_idle_START (13)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_gs_himinibus_idle_END (13)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_x2x_idle_0_START (14)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_x2x_idle_0_END (14)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_x2x_idle_1_START (15)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_x2x_idle_1_END (15)
#define SOC_NPU_SCTRL_NPU_STAT10_ts_cfg_256to128_x2x_idle_START (16)
#define SOC_NPU_SCTRL_NPU_STAT10_ts_cfg_256to128_x2x_idle_END (16)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_top_1to2_hiaxi_idle_START (17)
#define SOC_NPU_SCTRL_NPU_STAT10_aic0_top_1to2_hiaxi_idle_END (17)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_top_1to2_hiaxi_idle_START (18)
#define SOC_NPU_SCTRL_NPU_STAT10_aic1_top_1to2_hiaxi_idle_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic0_axi_2to1_dlock_slv_0 : 2;
        unsigned int aic0_axi_2to1_dlock_slv_1 : 2;
        unsigned int aic0_axi_2to1_dlock_mst_0 : 4;
        unsigned int aic0_axi_2to1_dlock_mst_1 : 4;
        unsigned int aic0_2to1_1_dbg_ar_err : 4;
        unsigned int aic0_2to1_1_dbg_aw_err : 4;
        unsigned int aic0_2to1_0_dbg_ar_err : 4;
        unsigned int aic0_2to1_0_dbg_aw_err : 4;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT11_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_axi_2to1_dlock_slv_0_START (0)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_axi_2to1_dlock_slv_0_END (1)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_axi_2to1_dlock_slv_1_START (2)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_axi_2to1_dlock_slv_1_END (3)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_axi_2to1_dlock_mst_0_START (4)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_axi_2to1_dlock_mst_0_END (7)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_axi_2to1_dlock_mst_1_START (8)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_axi_2to1_dlock_mst_1_END (11)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_2to1_1_dbg_ar_err_START (12)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_2to1_1_dbg_ar_err_END (15)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_2to1_1_dbg_aw_err_START (16)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_2to1_1_dbg_aw_err_END (19)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_2to1_0_dbg_ar_err_START (20)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_2to1_0_dbg_ar_err_END (23)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_2to1_0_dbg_aw_err_START (24)
#define SOC_NPU_SCTRL_NPU_STAT11_aic0_2to1_0_dbg_aw_err_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic0_axi_1to2_dlock_slv_0 : 4;
        unsigned int aic0_axi_1to2_dlock_slv_1 : 4;
        unsigned int aic0_axi_1to2_dlock_mst_0 : 2;
        unsigned int aic0_axi_1to2_dlock_mst_1 : 2;
        unsigned int aic0_1to2_1_dbg_ar_err : 4;
        unsigned int aic0_1to2_1_dbg_aw_err : 4;
        unsigned int aic0_1to2_0_dbg_ar_err : 4;
        unsigned int aic0_1to2_0_dbg_aw_err : 4;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT12_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_axi_1to2_dlock_slv_0_START (0)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_axi_1to2_dlock_slv_0_END (3)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_axi_1to2_dlock_slv_1_START (4)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_axi_1to2_dlock_slv_1_END (7)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_axi_1to2_dlock_mst_0_START (8)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_axi_1to2_dlock_mst_0_END (9)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_axi_1to2_dlock_mst_1_START (10)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_axi_1to2_dlock_mst_1_END (11)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_1to2_1_dbg_ar_err_START (12)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_1to2_1_dbg_ar_err_END (15)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_1to2_1_dbg_aw_err_START (16)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_1to2_1_dbg_aw_err_END (19)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_1to2_0_dbg_ar_err_START (20)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_1to2_0_dbg_ar_err_END (23)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_1to2_0_dbg_aw_err_START (24)
#define SOC_NPU_SCTRL_NPU_STAT12_aic0_1to2_0_dbg_aw_err_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic0_top_1to2_dlock_slv : 4;
        unsigned int aic0_top_1to2_dlock_mst : 2;
        unsigned int reserved_0 : 2;
        unsigned int aic0_top_1to2_dbg_ar_err : 4;
        unsigned int aic0_top_1to2_dbg_aw_err : 4;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT13_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT13_aic0_top_1to2_dlock_slv_START (0)
#define SOC_NPU_SCTRL_NPU_STAT13_aic0_top_1to2_dlock_slv_END (3)
#define SOC_NPU_SCTRL_NPU_STAT13_aic0_top_1to2_dlock_mst_START (4)
#define SOC_NPU_SCTRL_NPU_STAT13_aic0_top_1to2_dlock_mst_END (5)
#define SOC_NPU_SCTRL_NPU_STAT13_aic0_top_1to2_dbg_ar_err_START (8)
#define SOC_NPU_SCTRL_NPU_STAT13_aic0_top_1to2_dbg_ar_err_END (11)
#define SOC_NPU_SCTRL_NPU_STAT13_aic0_top_1to2_dbg_aw_err_START (12)
#define SOC_NPU_SCTRL_NPU_STAT13_aic0_top_1to2_dbg_aw_err_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic1_axi_2to1_dlock_slv_0 : 2;
        unsigned int aic1_axi_2to1_dlock_slv_1 : 2;
        unsigned int aic1_axi_2to1_dlock_mst_0 : 4;
        unsigned int aic1_axi_2to1_dlock_mst_1 : 4;
        unsigned int aic1_2to1_1_dbg_ar_err : 4;
        unsigned int aic1_2to1_1_dbg_aw_err : 4;
        unsigned int aic1_2to1_0_dbg_ar_err : 4;
        unsigned int aic1_2to1_0_dbg_aw_err : 4;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT14_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_axi_2to1_dlock_slv_0_START (0)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_axi_2to1_dlock_slv_0_END (1)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_axi_2to1_dlock_slv_1_START (2)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_axi_2to1_dlock_slv_1_END (3)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_axi_2to1_dlock_mst_0_START (4)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_axi_2to1_dlock_mst_0_END (7)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_axi_2to1_dlock_mst_1_START (8)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_axi_2to1_dlock_mst_1_END (11)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_2to1_1_dbg_ar_err_START (12)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_2to1_1_dbg_ar_err_END (15)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_2to1_1_dbg_aw_err_START (16)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_2to1_1_dbg_aw_err_END (19)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_2to1_0_dbg_ar_err_START (20)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_2to1_0_dbg_ar_err_END (23)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_2to1_0_dbg_aw_err_START (24)
#define SOC_NPU_SCTRL_NPU_STAT14_aic1_2to1_0_dbg_aw_err_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic1_axi_1to2_dlock_slv_0 : 4;
        unsigned int aic1_axi_1to2_dlock_slv_1 : 4;
        unsigned int aic1_axi_1to2_dlock_mst_0 : 2;
        unsigned int aic1_axi_1to2_dlock_mst_1 : 2;
        unsigned int aic1_1to2_1_dbg_ar_err : 4;
        unsigned int aic1_1to2_1_dbg_aw_err : 4;
        unsigned int aic1_1to2_0_dbg_ar_err : 4;
        unsigned int aic1_1to2_0_dbg_aw_err : 4;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT15_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_axi_1to2_dlock_slv_0_START (0)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_axi_1to2_dlock_slv_0_END (3)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_axi_1to2_dlock_slv_1_START (4)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_axi_1to2_dlock_slv_1_END (7)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_axi_1to2_dlock_mst_0_START (8)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_axi_1to2_dlock_mst_0_END (9)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_axi_1to2_dlock_mst_1_START (10)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_axi_1to2_dlock_mst_1_END (11)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_1to2_1_dbg_ar_err_START (12)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_1to2_1_dbg_ar_err_END (15)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_1to2_1_dbg_aw_err_START (16)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_1to2_1_dbg_aw_err_END (19)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_1to2_0_dbg_ar_err_START (20)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_1to2_0_dbg_ar_err_END (23)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_1to2_0_dbg_aw_err_START (24)
#define SOC_NPU_SCTRL_NPU_STAT15_aic1_1to2_0_dbg_aw_err_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aic1_top_1to2_dlock_slv : 4;
        unsigned int aic1_top_1to2_dlock_mst : 2;
        unsigned int reserved_0 : 2;
        unsigned int aic1_top_1to2_dbg_ar_err : 4;
        unsigned int aic1_top_1to2_dbg_aw_err : 4;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT16_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT16_aic1_top_1to2_dlock_slv_START (0)
#define SOC_NPU_SCTRL_NPU_STAT16_aic1_top_1to2_dlock_slv_END (3)
#define SOC_NPU_SCTRL_NPU_STAT16_aic1_top_1to2_dlock_mst_START (4)
#define SOC_NPU_SCTRL_NPU_STAT16_aic1_top_1to2_dlock_mst_END (5)
#define SOC_NPU_SCTRL_NPU_STAT16_aic1_top_1to2_dbg_ar_err_START (8)
#define SOC_NPU_SCTRL_NPU_STAT16_aic1_top_1to2_dbg_ar_err_END (11)
#define SOC_NPU_SCTRL_NPU_STAT16_aic1_top_1to2_dbg_aw_err_START (12)
#define SOC_NPU_SCTRL_NPU_STAT16_aic1_top_1to2_dbg_aw_err_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_rint : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT17_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT17_ovc_rint_START (0)
#define SOC_NPU_SCTRL_NPU_STAT17_ovc_rint_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ovc_mint_lpmcu : 1;
        unsigned int ovc_mint_acpu : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT18_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT18_ovc_mint_lpmcu_START (0)
#define SOC_NPU_SCTRL_NPU_STAT18_ovc_mint_lpmcu_END (0)
#define SOC_NPU_SCTRL_NPU_STAT18_ovc_mint_acpu_START (1)
#define SOC_NPU_SCTRL_NPU_STAT18_ovc_mint_acpu_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_lite_aicore0_ds_state : 1;
        unsigned int npu_lite_aicore1_ds_state : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT19_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT19_npu_lite_aicore0_ds_state_START (0)
#define SOC_NPU_SCTRL_NPU_STAT19_npu_lite_aicore0_ds_state_END (0)
#define SOC_NPU_SCTRL_NPU_STAT19_npu_lite_aicore1_ds_state_START (1)
#define SOC_NPU_SCTRL_NPU_STAT19_npu_lite_aicore1_ds_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int easc0_rst_rls_int_st : 1;
        unsigned int easc0_rst_int_st : 1;
        unsigned int easc0_int_st : 1;
        unsigned int easc1_rst_rls_int_st : 1;
        unsigned int easc1_rst_int_st : 1;
        unsigned int easc1_int_st : 1;
        unsigned int easc2_rst_rls_int_st : 1;
        unsigned int easc2_rst_int_st : 1;
        unsigned int easc2_int_st : 1;
        unsigned int aicore0_rst_int_st : 1;
        unsigned int aicore0_rst_rls_int_st : 1;
        unsigned int aicore1_rst_int_st : 1;
        unsigned int aicore1_rst_rls_int_st : 1;
        unsigned int wdog_res_ts_int_st : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT20_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT20_easc0_rst_rls_int_st_START (0)
#define SOC_NPU_SCTRL_NPU_STAT20_easc0_rst_rls_int_st_END (0)
#define SOC_NPU_SCTRL_NPU_STAT20_easc0_rst_int_st_START (1)
#define SOC_NPU_SCTRL_NPU_STAT20_easc0_rst_int_st_END (1)
#define SOC_NPU_SCTRL_NPU_STAT20_easc0_int_st_START (2)
#define SOC_NPU_SCTRL_NPU_STAT20_easc0_int_st_END (2)
#define SOC_NPU_SCTRL_NPU_STAT20_easc1_rst_rls_int_st_START (3)
#define SOC_NPU_SCTRL_NPU_STAT20_easc1_rst_rls_int_st_END (3)
#define SOC_NPU_SCTRL_NPU_STAT20_easc1_rst_int_st_START (4)
#define SOC_NPU_SCTRL_NPU_STAT20_easc1_rst_int_st_END (4)
#define SOC_NPU_SCTRL_NPU_STAT20_easc1_int_st_START (5)
#define SOC_NPU_SCTRL_NPU_STAT20_easc1_int_st_END (5)
#define SOC_NPU_SCTRL_NPU_STAT20_easc2_rst_rls_int_st_START (6)
#define SOC_NPU_SCTRL_NPU_STAT20_easc2_rst_rls_int_st_END (6)
#define SOC_NPU_SCTRL_NPU_STAT20_easc2_rst_int_st_START (7)
#define SOC_NPU_SCTRL_NPU_STAT20_easc2_rst_int_st_END (7)
#define SOC_NPU_SCTRL_NPU_STAT20_easc2_int_st_START (8)
#define SOC_NPU_SCTRL_NPU_STAT20_easc2_int_st_END (8)
#define SOC_NPU_SCTRL_NPU_STAT20_aicore0_rst_int_st_START (9)
#define SOC_NPU_SCTRL_NPU_STAT20_aicore0_rst_int_st_END (9)
#define SOC_NPU_SCTRL_NPU_STAT20_aicore0_rst_rls_int_st_START (10)
#define SOC_NPU_SCTRL_NPU_STAT20_aicore0_rst_rls_int_st_END (10)
#define SOC_NPU_SCTRL_NPU_STAT20_aicore1_rst_int_st_START (11)
#define SOC_NPU_SCTRL_NPU_STAT20_aicore1_rst_int_st_END (11)
#define SOC_NPU_SCTRL_NPU_STAT20_aicore1_rst_rls_int_st_START (12)
#define SOC_NPU_SCTRL_NPU_STAT20_aicore1_rst_rls_int_st_END (12)
#define SOC_NPU_SCTRL_NPU_STAT20_wdog_res_ts_int_st_START (13)
#define SOC_NPU_SCTRL_NPU_STAT20_wdog_res_ts_int_st_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_npu2qice_sw_done_stat : 1;
        unsigned int npu2qice_sw_timeout : 1;
        unsigned int reserved_0 : 2;
        unsigned int npu_qice_state : 1;
        unsigned int reserved_1 : 3;
        unsigned int bp_req : 1;
        unsigned int switch_req : 1;
        unsigned int reserved_2 : 22;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT21_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT21_intr_npu2qice_sw_done_stat_START (0)
#define SOC_NPU_SCTRL_NPU_STAT21_intr_npu2qice_sw_done_stat_END (0)
#define SOC_NPU_SCTRL_NPU_STAT21_npu2qice_sw_timeout_START (1)
#define SOC_NPU_SCTRL_NPU_STAT21_npu2qice_sw_timeout_END (1)
#define SOC_NPU_SCTRL_NPU_STAT21_npu_qice_state_START (4)
#define SOC_NPU_SCTRL_NPU_STAT21_npu_qice_state_END (4)
#define SOC_NPU_SCTRL_NPU_STAT21_bp_req_START (8)
#define SOC_NPU_SCTRL_NPU_STAT21_bp_req_END (8)
#define SOC_NPU_SCTRL_NPU_STAT21_switch_req_START (9)
#define SOC_NPU_SCTRL_NPU_STAT21_switch_req_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ec_rpt : 32;
    } reg;
} SOC_NPU_SCTRL_NPU_STAT22_UNION;
#endif
#define SOC_NPU_SCTRL_NPU_STAT22_ec_rpt_START (0)
#define SOC_NPU_SCTRL_NPU_STAT22_ec_rpt_END (31)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
