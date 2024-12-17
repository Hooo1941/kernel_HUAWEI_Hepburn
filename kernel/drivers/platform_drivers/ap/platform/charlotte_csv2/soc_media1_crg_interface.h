#ifndef __SOC_MEDIA1_CRG_INTERFACE_H__
#define __SOC_MEDIA1_CRG_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_MEDIA1_CRG_PEREN0_ADDR(base) ((base) + (0x000UL))
#define SOC_MEDIA1_CRG_PERDIS0_ADDR(base) ((base) + (0x004UL))
#define SOC_MEDIA1_CRG_PERCLKEN0_ADDR(base) ((base) + (0x008UL))
#define SOC_MEDIA1_CRG_PERSTAT0_ADDR(base) ((base) + (0x00CUL))
#define SOC_MEDIA1_CRG_PEREN1_ADDR(base) ((base) + (0x010UL))
#define SOC_MEDIA1_CRG_PERDIS1_ADDR(base) ((base) + (0x014UL))
#define SOC_MEDIA1_CRG_PERCLKEN1_ADDR(base) ((base) + (0x018UL))
#define SOC_MEDIA1_CRG_PERSTAT1_ADDR(base) ((base) + (0x01CUL))
#define SOC_MEDIA1_CRG_PEREN2_ADDR(base) ((base) + (0x020UL))
#define SOC_MEDIA1_CRG_PERDIS2_ADDR(base) ((base) + (0x024UL))
#define SOC_MEDIA1_CRG_PERCLKEN2_ADDR(base) ((base) + (0x028UL))
#define SOC_MEDIA1_CRG_PERSTAT2_ADDR(base) ((base) + (0x02CUL))
#define SOC_MEDIA1_CRG_PERRSTEN0_ADDR(base) ((base) + (0x030UL))
#define SOC_MEDIA1_CRG_PERRSTDIS0_ADDR(base) ((base) + (0x034UL))
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ADDR(base) ((base) + (0x038UL))
#define SOC_MEDIA1_CRG_PERRSTEN1_ADDR(base) ((base) + (0x03CUL))
#define SOC_MEDIA1_CRG_PERRSTDIS1_ADDR(base) ((base) + (0x040UL))
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ADDR(base) ((base) + (0x044UL))
#define SOC_MEDIA1_CRG_CLKDIV0_ADDR(base) ((base) + (0x060UL))
#define SOC_MEDIA1_CRG_CLKDIV1_ADDR(base) ((base) + (0x064UL))
#define SOC_MEDIA1_CRG_CLKDIV2_ADDR(base) ((base) + (0x068UL))
#define SOC_MEDIA1_CRG_CLKDIV3_ADDR(base) ((base) + (0x06CUL))
#define SOC_MEDIA1_CRG_CLKDIV4_ADDR(base) ((base) + (0x070UL))
#define SOC_MEDIA1_CRG_CLKDIV5_ADDR(base) ((base) + (0x074UL))
#define SOC_MEDIA1_CRG_CLKDIV6_ADDR(base) ((base) + (0x078UL))
#define SOC_MEDIA1_CRG_CLKDIV7_ADDR(base) ((base) + (0x07CUL))
#define SOC_MEDIA1_CRG_CLKDIV8_ADDR(base) ((base) + (0x080UL))
#define SOC_MEDIA1_CRG_CLKDIV9_ADDR(base) ((base) + (0x084UL))
#define SOC_MEDIA1_CRG_CLKDIV10_ADDR(base) ((base) + (0x088UL))
#define SOC_MEDIA1_CRG_CLKDIV16_ADDR(base) ((base) + (0x09CUL))
#define SOC_MEDIA1_CRG_PERI_STAT0_ADDR(base) ((base) + (0x0A0UL))
#define SOC_MEDIA1_CRG_PERI_STAT1_ADDR(base) ((base) + (0x0A4UL))
#define SOC_MEDIA1_CRG_PERI_STAT2_ADDR(base) ((base) + (0x0A8UL))
#define SOC_MEDIA1_CRG_DSS_DVFS_ADDR(base) ((base) + (0x100UL))
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_ADDR(base) ((base) + (0x130UL))
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ADDR(base) ((base) + (0x160UL))
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_ADDR(base) ((base) + (0x164UL))
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_ADDR(base) ((base) + (0x168UL))
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_ADDR(base) ((base) + (0x16cUL))
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_ADDR(base) ((base) + (0x170UL))
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ADDR(base) ((base) + (0x174UL))
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_ADDR(base) ((base) + (0x190UL))
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_ADDR(base) ((base) + (0x1D0UL))
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_ADDR(base) ((base) + (0x1D4UL))
#define SOC_MEDIA1_CRG_HPM_CFG_ADDR(base) ((base) + (0x200UL))
#define SOC_MEDIA1_CRG_HPM_READ_BACK_ADDR(base) ((base) + (0x204UL))
#define SOC_MEDIA1_CRG_HPMX_CFG_ADDR(base) ((base) + (0x208UL))
#define SOC_MEDIA1_CRG_HPMX_READ_BACK_ADDR(base) ((base) + (0x20CUL))
#define SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_ADDR(base) ((base) + (0x300UL))
#define SOC_MEDIA1_CRG_HIMINIBUS_DLOCK_CLR_ADDR(base) ((base) + (0x304UL))
#define SOC_MEDIA1_CRG_VDEC_IDLE_READ_BACK_ADDR(base) ((base) + (0x400UL))
#define SOC_MEDIA1_CRG_DSS_VSYNC_SEL_ADDR(base) ((base) + (0x500UL))
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ADDR(base) ((base) + (0x600UL))
#define SOC_MEDIA1_CRG_HIMINIBUS_IDLE_READ_BACK_ADDR(base) ((base) + (0x604UL))
#define SOC_MEDIA1_CRG_DECPROT0SET_ADDR(base) ((base) + (0xF00UL))
#define SOC_MEDIA1_CRG_PERRSTEN_SAFE0_ADDR(base) ((base) + (0xF50UL))
#define SOC_MEDIA1_CRG_PERRSTDIS_SAFE0_ADDR(base) ((base) + (0xF54UL))
#define SOC_MEDIA1_CRG_PERRSTSTAT_SAFE0_ADDR(base) ((base) + (0xF58UL))
#else
#define SOC_MEDIA1_CRG_PEREN0_ADDR(base) ((base) + (0x000))
#define SOC_MEDIA1_CRG_PERDIS0_ADDR(base) ((base) + (0x004))
#define SOC_MEDIA1_CRG_PERCLKEN0_ADDR(base) ((base) + (0x008))
#define SOC_MEDIA1_CRG_PERSTAT0_ADDR(base) ((base) + (0x00C))
#define SOC_MEDIA1_CRG_PEREN1_ADDR(base) ((base) + (0x010))
#define SOC_MEDIA1_CRG_PERDIS1_ADDR(base) ((base) + (0x014))
#define SOC_MEDIA1_CRG_PERCLKEN1_ADDR(base) ((base) + (0x018))
#define SOC_MEDIA1_CRG_PERSTAT1_ADDR(base) ((base) + (0x01C))
#define SOC_MEDIA1_CRG_PEREN2_ADDR(base) ((base) + (0x020))
#define SOC_MEDIA1_CRG_PERDIS2_ADDR(base) ((base) + (0x024))
#define SOC_MEDIA1_CRG_PERCLKEN2_ADDR(base) ((base) + (0x028))
#define SOC_MEDIA1_CRG_PERSTAT2_ADDR(base) ((base) + (0x02C))
#define SOC_MEDIA1_CRG_PERRSTEN0_ADDR(base) ((base) + (0x030))
#define SOC_MEDIA1_CRG_PERRSTDIS0_ADDR(base) ((base) + (0x034))
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ADDR(base) ((base) + (0x038))
#define SOC_MEDIA1_CRG_PERRSTEN1_ADDR(base) ((base) + (0x03C))
#define SOC_MEDIA1_CRG_PERRSTDIS1_ADDR(base) ((base) + (0x040))
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ADDR(base) ((base) + (0x044))
#define SOC_MEDIA1_CRG_CLKDIV0_ADDR(base) ((base) + (0x060))
#define SOC_MEDIA1_CRG_CLKDIV1_ADDR(base) ((base) + (0x064))
#define SOC_MEDIA1_CRG_CLKDIV2_ADDR(base) ((base) + (0x068))
#define SOC_MEDIA1_CRG_CLKDIV3_ADDR(base) ((base) + (0x06C))
#define SOC_MEDIA1_CRG_CLKDIV4_ADDR(base) ((base) + (0x070))
#define SOC_MEDIA1_CRG_CLKDIV5_ADDR(base) ((base) + (0x074))
#define SOC_MEDIA1_CRG_CLKDIV6_ADDR(base) ((base) + (0x078))
#define SOC_MEDIA1_CRG_CLKDIV7_ADDR(base) ((base) + (0x07C))
#define SOC_MEDIA1_CRG_CLKDIV8_ADDR(base) ((base) + (0x080))
#define SOC_MEDIA1_CRG_CLKDIV9_ADDR(base) ((base) + (0x084))
#define SOC_MEDIA1_CRG_CLKDIV10_ADDR(base) ((base) + (0x088))
#define SOC_MEDIA1_CRG_CLKDIV16_ADDR(base) ((base) + (0x09C))
#define SOC_MEDIA1_CRG_PERI_STAT0_ADDR(base) ((base) + (0x0A0))
#define SOC_MEDIA1_CRG_PERI_STAT1_ADDR(base) ((base) + (0x0A4))
#define SOC_MEDIA1_CRG_PERI_STAT2_ADDR(base) ((base) + (0x0A8))
#define SOC_MEDIA1_CRG_DSS_DVFS_ADDR(base) ((base) + (0x100))
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_ADDR(base) ((base) + (0x130))
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_ADDR(base) ((base) + (0x160))
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_ADDR(base) ((base) + (0x164))
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_ADDR(base) ((base) + (0x168))
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_ADDR(base) ((base) + (0x16c))
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_ADDR(base) ((base) + (0x170))
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_ADDR(base) ((base) + (0x174))
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_ADDR(base) ((base) + (0x190))
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_ADDR(base) ((base) + (0x1D0))
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_ADDR(base) ((base) + (0x1D4))
#define SOC_MEDIA1_CRG_HPM_CFG_ADDR(base) ((base) + (0x200))
#define SOC_MEDIA1_CRG_HPM_READ_BACK_ADDR(base) ((base) + (0x204))
#define SOC_MEDIA1_CRG_HPMX_CFG_ADDR(base) ((base) + (0x208))
#define SOC_MEDIA1_CRG_HPMX_READ_BACK_ADDR(base) ((base) + (0x20C))
#define SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_ADDR(base) ((base) + (0x300))
#define SOC_MEDIA1_CRG_HIMINIBUS_DLOCK_CLR_ADDR(base) ((base) + (0x304))
#define SOC_MEDIA1_CRG_VDEC_IDLE_READ_BACK_ADDR(base) ((base) + (0x400))
#define SOC_MEDIA1_CRG_DSS_VSYNC_SEL_ADDR(base) ((base) + (0x500))
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ADDR(base) ((base) + (0x600))
#define SOC_MEDIA1_CRG_HIMINIBUS_IDLE_READ_BACK_ADDR(base) ((base) + (0x604))
#define SOC_MEDIA1_CRG_DECPROT0SET_ADDR(base) ((base) + (0xF00))
#define SOC_MEDIA1_CRG_PERRSTEN_SAFE0_ADDR(base) ((base) + (0xF50))
#define SOC_MEDIA1_CRG_PERRSTDIS_SAFE0_ADDR(base) ((base) + (0xF54))
#define SOC_MEDIA1_CRG_PERRSTSTAT_SAFE0_ADDR(base) ((base) + (0xF58))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_aclk_vdec : 1;
        unsigned int gt_pclk_vdec : 1;
        unsigned int gt_clk_vdec : 1;
        unsigned int gt_clk_vdec_ref32k : 1;
        unsigned int gt_clk_gm_vdec : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_aclk_jpg : 1;
        unsigned int gt_pclk_jpg : 1;
        unsigned int gt_clk_jpg_func : 1;
        unsigned int gt_clk_ipp_ref32k : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_arpp_ref32k : 1;
        unsigned int gt_clk_arpp_tcxo : 1;
        unsigned int gt_pclk_atgm_arpp : 1;
        unsigned int gt_clk_gm_ipp : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_MEDIA1_CRG_PEREN0_UNION;
#endif
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_vdec_START (0)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_vdec_END (0)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_vdec_START (1)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_vdec_END (1)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_vdec_START (2)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_vdec_END (2)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_vdec_ref32k_START (3)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_vdec_ref32k_END (3)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_gm_vdec_START (4)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_gm_vdec_END (4)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_jpg_START (8)
#define SOC_MEDIA1_CRG_PEREN0_gt_aclk_jpg_END (8)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_jpg_START (9)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_jpg_END (9)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_jpg_func_START (10)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_jpg_func_END (10)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ipp_ref32k_START (11)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_ipp_ref32k_END (11)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_arpp_ref32k_START (15)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_arpp_ref32k_END (15)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_arpp_tcxo_START (16)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_arpp_tcxo_END (16)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_atgm_arpp_START (17)
#define SOC_MEDIA1_CRG_PEREN0_gt_pclk_atgm_arpp_END (17)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_gm_ipp_START (18)
#define SOC_MEDIA1_CRG_PEREN0_gt_clk_gm_ipp_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_aclk_vdec : 1;
        unsigned int gt_pclk_vdec : 1;
        unsigned int gt_clk_vdec : 1;
        unsigned int gt_clk_vdec_ref32k : 1;
        unsigned int gt_clk_gm_vdec : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_aclk_jpg : 1;
        unsigned int gt_pclk_jpg : 1;
        unsigned int gt_clk_jpg_func : 1;
        unsigned int gt_clk_ipp_ref32k : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_arpp_ref32k : 1;
        unsigned int gt_clk_arpp_tcxo : 1;
        unsigned int gt_pclk_atgm_arpp : 1;
        unsigned int gt_clk_gm_ipp : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_MEDIA1_CRG_PERDIS0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_vdec_START (0)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_vdec_END (0)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_vdec_START (1)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_vdec_END (1)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_vdec_START (2)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_vdec_END (2)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_vdec_ref32k_START (3)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_vdec_ref32k_END (3)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_gm_vdec_START (4)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_gm_vdec_END (4)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_jpg_START (8)
#define SOC_MEDIA1_CRG_PERDIS0_gt_aclk_jpg_END (8)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_jpg_START (9)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_jpg_END (9)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_jpg_func_START (10)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_jpg_func_END (10)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ipp_ref32k_START (11)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_ipp_ref32k_END (11)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_arpp_ref32k_START (15)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_arpp_ref32k_END (15)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_arpp_tcxo_START (16)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_arpp_tcxo_END (16)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_atgm_arpp_START (17)
#define SOC_MEDIA1_CRG_PERDIS0_gt_pclk_atgm_arpp_END (17)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_gm_ipp_START (18)
#define SOC_MEDIA1_CRG_PERDIS0_gt_clk_gm_ipp_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_aclk_vdec : 1;
        unsigned int gt_pclk_vdec : 1;
        unsigned int gt_clk_vdec : 1;
        unsigned int gt_clk_vdec_ref32k : 1;
        unsigned int gt_clk_gm_vdec : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_aclk_jpg : 1;
        unsigned int gt_pclk_jpg : 1;
        unsigned int gt_clk_jpg_func : 1;
        unsigned int gt_clk_ipp_ref32k : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_arpp_ref32k : 1;
        unsigned int gt_clk_arpp_tcxo : 1;
        unsigned int gt_pclk_atgm_arpp : 1;
        unsigned int gt_clk_gm_ipp : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_MEDIA1_CRG_PERCLKEN0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_vdec_START (0)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_vdec_END (0)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_vdec_START (1)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_vdec_END (1)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_vdec_START (2)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_vdec_END (2)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_vdec_ref32k_START (3)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_vdec_ref32k_END (3)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_gm_vdec_START (4)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_gm_vdec_END (4)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_jpg_START (8)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_aclk_jpg_END (8)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_jpg_START (9)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_jpg_END (9)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_jpg_func_START (10)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_jpg_func_END (10)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ipp_ref32k_START (11)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_ipp_ref32k_END (11)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_arpp_ref32k_START (15)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_arpp_ref32k_END (15)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_arpp_tcxo_START (16)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_arpp_tcxo_END (16)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_atgm_arpp_START (17)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_pclk_atgm_arpp_END (17)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_gm_ipp_START (18)
#define SOC_MEDIA1_CRG_PERCLKEN0_gt_clk_gm_ipp_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_aclk_vdec : 1;
        unsigned int st_pclk_vdec : 1;
        unsigned int st_clk_vdec : 1;
        unsigned int st_clk_vdec_ref32k : 1;
        unsigned int st_clk_gm_vdec : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_aclk_jpg : 1;
        unsigned int st_pclk_jpg : 1;
        unsigned int st_clk_jpg_func : 1;
        unsigned int st_clk_ipp_ref32k : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int st_clk_arpp_ref32k : 1;
        unsigned int st_clk_arpp_tcxo : 1;
        unsigned int st_pclk_atgm_arpp : 1;
        unsigned int st_clk_gm_ipp : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
    } reg;
} SOC_MEDIA1_CRG_PERSTAT0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_vdec_START (0)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_vdec_END (0)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_vdec_START (1)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_vdec_END (1)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_vdec_START (2)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_vdec_END (2)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_vdec_ref32k_START (3)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_vdec_ref32k_END (3)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_gm_vdec_START (4)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_gm_vdec_END (4)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_jpg_START (8)
#define SOC_MEDIA1_CRG_PERSTAT0_st_aclk_jpg_END (8)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_jpg_START (9)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_jpg_END (9)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_jpg_func_START (10)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_jpg_func_END (10)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ipp_ref32k_START (11)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_ipp_ref32k_END (11)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_arpp_ref32k_START (15)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_arpp_ref32k_END (15)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_arpp_tcxo_START (16)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_arpp_tcxo_END (16)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_atgm_arpp_START (17)
#define SOC_MEDIA1_CRG_PERSTAT0_st_pclk_atgm_arpp_END (17)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_gm_ipp_START (18)
#define SOC_MEDIA1_CRG_PERSTAT0_st_clk_gm_ipp_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_qic_dss_cfg : 1;
        unsigned int gt_clk_dss_core_part : 1;
        unsigned int gt_clk_dss_core_disp1_div3 : 1;
        unsigned int gt_clk_dss_core_disp1 : 1;
        unsigned int gt_clk_dss_core_disp0_div3 : 1;
        unsigned int gt_clk_dss_core_disp0 : 1;
        unsigned int gt_clk_vivobus_disp_ini : 1;
        unsigned int gt_clk_dss_tcxo : 1;
        unsigned int gt_pclk_atgm_dss : 1;
        unsigned int gt_clk_dss_ref32k : 1;
        unsigned int gt_clk_gm_dss : 1;
        unsigned int gt_pclk_dsi : 1;
        unsigned int gt_clk_qic_dss_data : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_vivobus : 1;
        unsigned int gt_clk_tcu_vivo : 1;
        unsigned int gt_clk_vivobus_ref32k : 1;
        unsigned int gt_clk_vivobus_disp_media_ini : 1;
        unsigned int gt_clk_vivobus_disp_media : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_qic_media1sub_qice_data : 1;
        unsigned int gt_clk_qic_media1sub_qtpe_data : 1;
        unsigned int gt_clk_qic_media1sub_qtp_gms : 1;
        unsigned int gt_clk_qic_qtpem_bus : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 3;
    } reg;
} SOC_MEDIA1_CRG_PEREN1_UNION;
#endif
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_dss_cfg_START (2)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_dss_cfg_END (2)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_part_START (3)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_part_END (3)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_disp1_div3_START (4)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_disp1_div3_END (4)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_disp1_START (5)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_disp1_END (5)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_disp0_div3_START (6)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_disp0_div3_END (6)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_disp0_START (7)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_core_disp0_END (7)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_disp_ini_START (8)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_disp_ini_END (8)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_tcxo_START (9)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_tcxo_END (9)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_atgm_dss_START (10)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_atgm_dss_END (10)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_ref32k_START (11)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_dss_ref32k_END (11)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_gm_dss_START (12)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_gm_dss_END (12)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_dsi_START (13)
#define SOC_MEDIA1_CRG_PEREN1_gt_pclk_dsi_END (13)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_dss_data_START (14)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_dss_data_END (14)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_START (16)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_END (16)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_tcu_vivo_START (17)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_tcu_vivo_END (17)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_ref32k_START (18)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_ref32k_END (18)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_disp_media_ini_START (19)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_disp_media_ini_END (19)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_disp_media_START (20)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_vivobus_disp_media_END (20)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_media1sub_qice_data_START (24)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_media1sub_qice_data_END (24)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_media1sub_qtpe_data_START (25)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_media1sub_qtpe_data_END (25)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_media1sub_qtp_gms_START (26)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_media1sub_qtp_gms_END (26)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_qtpem_bus_START (27)
#define SOC_MEDIA1_CRG_PEREN1_gt_clk_qic_qtpem_bus_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_qic_dss_cfg : 1;
        unsigned int gt_clk_dss_core_part : 1;
        unsigned int gt_clk_dss_core_disp1_div3 : 1;
        unsigned int gt_clk_dss_core_disp1 : 1;
        unsigned int gt_clk_dss_core_disp0_div3 : 1;
        unsigned int gt_clk_dss_core_disp0 : 1;
        unsigned int gt_clk_vivobus_disp_ini : 1;
        unsigned int gt_clk_dss_tcxo : 1;
        unsigned int gt_pclk_atgm_dss : 1;
        unsigned int gt_clk_dss_ref32k : 1;
        unsigned int gt_clk_gm_dss : 1;
        unsigned int gt_pclk_dsi : 1;
        unsigned int gt_clk_qic_dss_data : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_vivobus : 1;
        unsigned int gt_clk_tcu_vivo : 1;
        unsigned int gt_clk_vivobus_ref32k : 1;
        unsigned int gt_clk_vivobus_disp_media_ini : 1;
        unsigned int gt_clk_vivobus_disp_media : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_qic_media1sub_qice_data : 1;
        unsigned int gt_clk_qic_media1sub_qtpe_data : 1;
        unsigned int gt_clk_qic_media1sub_qtp_gms : 1;
        unsigned int gt_clk_qic_qtpem_bus : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 3;
    } reg;
} SOC_MEDIA1_CRG_PERDIS1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_dss_cfg_START (2)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_dss_cfg_END (2)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_part_START (3)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_part_END (3)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_disp1_div3_START (4)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_disp1_div3_END (4)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_disp1_START (5)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_disp1_END (5)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_disp0_div3_START (6)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_disp0_div3_END (6)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_disp0_START (7)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_core_disp0_END (7)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_disp_ini_START (8)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_disp_ini_END (8)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_tcxo_START (9)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_tcxo_END (9)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_atgm_dss_START (10)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_atgm_dss_END (10)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_ref32k_START (11)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_dss_ref32k_END (11)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_gm_dss_START (12)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_gm_dss_END (12)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_dsi_START (13)
#define SOC_MEDIA1_CRG_PERDIS1_gt_pclk_dsi_END (13)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_dss_data_START (14)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_dss_data_END (14)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_START (16)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_END (16)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_tcu_vivo_START (17)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_tcu_vivo_END (17)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_ref32k_START (18)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_ref32k_END (18)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_disp_media_ini_START (19)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_disp_media_ini_END (19)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_disp_media_START (20)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_vivobus_disp_media_END (20)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_media1sub_qice_data_START (24)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_media1sub_qice_data_END (24)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_media1sub_qtpe_data_START (25)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_media1sub_qtpe_data_END (25)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_media1sub_qtp_gms_START (26)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_media1sub_qtp_gms_END (26)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_qtpem_bus_START (27)
#define SOC_MEDIA1_CRG_PERDIS1_gt_clk_qic_qtpem_bus_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_qic_dss_cfg : 1;
        unsigned int gt_clk_dss_core_part : 1;
        unsigned int gt_clk_dss_core_disp1_div3 : 1;
        unsigned int gt_clk_dss_core_disp1 : 1;
        unsigned int gt_clk_dss_core_disp0_div3 : 1;
        unsigned int gt_clk_dss_core_disp0 : 1;
        unsigned int gt_clk_vivobus_disp_ini : 1;
        unsigned int gt_clk_dss_tcxo : 1;
        unsigned int gt_pclk_atgm_dss : 1;
        unsigned int gt_clk_dss_ref32k : 1;
        unsigned int gt_clk_gm_dss : 1;
        unsigned int gt_pclk_dsi : 1;
        unsigned int gt_clk_qic_dss_data : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_vivobus : 1;
        unsigned int gt_clk_tcu_vivo : 1;
        unsigned int gt_clk_vivobus_ref32k : 1;
        unsigned int gt_clk_vivobus_disp_media_ini : 1;
        unsigned int gt_clk_vivobus_disp_media : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_qic_media1sub_qice_data : 1;
        unsigned int gt_clk_qic_media1sub_qtpe_data : 1;
        unsigned int gt_clk_qic_media1sub_qtp_gms : 1;
        unsigned int gt_clk_qic_qtpem_bus : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 3;
    } reg;
} SOC_MEDIA1_CRG_PERCLKEN1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_dss_cfg_START (2)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_dss_cfg_END (2)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_part_START (3)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_part_END (3)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_disp1_div3_START (4)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_disp1_div3_END (4)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_disp1_START (5)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_disp1_END (5)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_disp0_div3_START (6)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_disp0_div3_END (6)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_disp0_START (7)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_core_disp0_END (7)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_disp_ini_START (8)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_disp_ini_END (8)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_tcxo_START (9)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_tcxo_END (9)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_atgm_dss_START (10)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_atgm_dss_END (10)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_ref32k_START (11)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_dss_ref32k_END (11)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_gm_dss_START (12)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_gm_dss_END (12)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_dsi_START (13)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_pclk_dsi_END (13)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_dss_data_START (14)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_dss_data_END (14)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_START (16)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_END (16)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_tcu_vivo_START (17)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_tcu_vivo_END (17)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_ref32k_START (18)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_ref32k_END (18)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_disp_media_ini_START (19)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_disp_media_ini_END (19)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_disp_media_START (20)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_vivobus_disp_media_END (20)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_media1sub_qice_data_START (24)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_media1sub_qice_data_END (24)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_media1sub_qtpe_data_START (25)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_media1sub_qtpe_data_END (25)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_media1sub_qtp_gms_START (26)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_media1sub_qtp_gms_END (26)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_qtpem_bus_START (27)
#define SOC_MEDIA1_CRG_PERCLKEN1_gt_clk_qic_qtpem_bus_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_clk_qic_dss_cfg : 1;
        unsigned int st_clk_dss_core_part : 1;
        unsigned int st_clk_dss_core_disp1_div3 : 1;
        unsigned int st_clk_dss_core_disp1 : 1;
        unsigned int st_clk_dss_core_disp0_div3 : 1;
        unsigned int st_clk_dss_core_disp0 : 1;
        unsigned int st_clk_vivobus_disp_ini : 1;
        unsigned int st_clk_dss_tcxo : 1;
        unsigned int st_pclk_atgm_dss : 1;
        unsigned int st_clk_dss_ref32k : 1;
        unsigned int st_clk_gm_dss : 1;
        unsigned int st_pclk_dsi : 1;
        unsigned int st_clk_qic_dss_data : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_clk_vivobus : 1;
        unsigned int st_clk_tcu_vivo : 1;
        unsigned int st_clk_vivobus_ref32k : 1;
        unsigned int st_clk_vivobus_disp_media_ini : 1;
        unsigned int st_clk_vivobus_disp_media : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int st_clk_qic_media1sub_qice_data : 1;
        unsigned int st_clk_qic_media1sub_qtpe_data : 1;
        unsigned int st_clk_qic_media1sub_qtp_gms : 1;
        unsigned int st_clk_qic_qtpem_bus : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 3;
    } reg;
} SOC_MEDIA1_CRG_PERSTAT1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_dss_cfg_START (2)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_dss_cfg_END (2)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_part_START (3)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_part_END (3)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_disp1_div3_START (4)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_disp1_div3_END (4)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_disp1_START (5)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_disp1_END (5)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_disp0_div3_START (6)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_disp0_div3_END (6)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_disp0_START (7)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_core_disp0_END (7)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_disp_ini_START (8)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_disp_ini_END (8)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_tcxo_START (9)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_tcxo_END (9)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_atgm_dss_START (10)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_atgm_dss_END (10)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_ref32k_START (11)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_dss_ref32k_END (11)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_gm_dss_START (12)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_gm_dss_END (12)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_dsi_START (13)
#define SOC_MEDIA1_CRG_PERSTAT1_st_pclk_dsi_END (13)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_dss_data_START (14)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_dss_data_END (14)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_START (16)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_END (16)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_tcu_vivo_START (17)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_tcu_vivo_END (17)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_ref32k_START (18)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_ref32k_END (18)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_disp_media_ini_START (19)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_disp_media_ini_END (19)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_disp_media_START (20)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_vivobus_disp_media_END (20)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_media1sub_qice_data_START (24)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_media1sub_qice_data_END (24)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_media1sub_qtpe_data_START (25)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_media1sub_qtpe_data_END (25)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_media1sub_qtp_gms_START (26)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_media1sub_qtp_gms_END (26)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_qtpem_bus_START (27)
#define SOC_MEDIA1_CRG_PERSTAT1_st_clk_qic_qtpem_bus_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_dpm_dss : 1;
        unsigned int gt_clk_dpm_vdec : 1;
        unsigned int gt_clk_hpm : 1;
        unsigned int gt_pclk_int_hub : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_autodiv_vivobus_disp : 1;
        unsigned int gt_pclk_autodiv_vdec : 1;
        unsigned int gt_pclk_autodiv_vivobus : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 18;
    } reg;
} SOC_MEDIA1_CRG_PEREN2_UNION;
#endif
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_dpm_dss_START (0)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_dpm_dss_END (0)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_dpm_vdec_START (1)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_dpm_vdec_END (1)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_hpm_START (2)
#define SOC_MEDIA1_CRG_PEREN2_gt_clk_hpm_END (2)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_int_hub_START (3)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_int_hub_END (3)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_autodiv_vivobus_disp_START (5)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_autodiv_vivobus_disp_END (5)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_autodiv_vdec_START (6)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_autodiv_vdec_END (6)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_autodiv_vivobus_START (7)
#define SOC_MEDIA1_CRG_PEREN2_gt_pclk_autodiv_vivobus_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_dpm_dss : 1;
        unsigned int gt_clk_dpm_vdec : 1;
        unsigned int gt_clk_hpm : 1;
        unsigned int gt_pclk_int_hub : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_autodiv_vivobus_disp : 1;
        unsigned int gt_pclk_autodiv_vdec : 1;
        unsigned int gt_pclk_autodiv_vivobus : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 18;
    } reg;
} SOC_MEDIA1_CRG_PERDIS2_UNION;
#endif
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_dpm_dss_START (0)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_dpm_dss_END (0)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_dpm_vdec_START (1)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_dpm_vdec_END (1)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_hpm_START (2)
#define SOC_MEDIA1_CRG_PERDIS2_gt_clk_hpm_END (2)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_int_hub_START (3)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_int_hub_END (3)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_autodiv_vivobus_disp_START (5)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_autodiv_vivobus_disp_END (5)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_autodiv_vdec_START (6)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_autodiv_vdec_END (6)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_autodiv_vivobus_START (7)
#define SOC_MEDIA1_CRG_PERDIS2_gt_pclk_autodiv_vivobus_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_dpm_dss : 1;
        unsigned int gt_clk_dpm_vdec : 1;
        unsigned int gt_clk_hpm : 1;
        unsigned int gt_pclk_int_hub : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_pclk_autodiv_vivobus_disp : 1;
        unsigned int gt_pclk_autodiv_vdec : 1;
        unsigned int gt_pclk_autodiv_vivobus : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 18;
    } reg;
} SOC_MEDIA1_CRG_PERCLKEN2_UNION;
#endif
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_dpm_dss_START (0)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_dpm_dss_END (0)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_dpm_vdec_START (1)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_dpm_vdec_END (1)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_hpm_START (2)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_clk_hpm_END (2)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_int_hub_START (3)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_int_hub_END (3)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_autodiv_vivobus_disp_START (5)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_autodiv_vivobus_disp_END (5)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_autodiv_vdec_START (6)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_autodiv_vdec_END (6)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_autodiv_vivobus_START (7)
#define SOC_MEDIA1_CRG_PERCLKEN2_gt_pclk_autodiv_vivobus_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_dpm_dss : 1;
        unsigned int st_clk_dpm_vdec : 1;
        unsigned int st_clk_hpm : 1;
        unsigned int st_pclk_int_hub : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_pclk_autodiv_vivobus_disp : 1;
        unsigned int st_pclk_autodiv_vdec : 1;
        unsigned int st_pclk_autodiv_vivobus : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 18;
    } reg;
} SOC_MEDIA1_CRG_PERSTAT2_UNION;
#endif
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_dpm_dss_START (0)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_dpm_dss_END (0)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_dpm_vdec_START (1)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_dpm_vdec_END (1)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_hpm_START (2)
#define SOC_MEDIA1_CRG_PERSTAT2_st_clk_hpm_END (2)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_int_hub_START (3)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_int_hub_END (3)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_autodiv_vivobus_disp_START (5)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_autodiv_vivobus_disp_END (5)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_autodiv_vdec_START (6)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_autodiv_vdec_END (6)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_autodiv_vivobus_START (7)
#define SOC_MEDIA1_CRG_PERSTAT2_st_pclk_autodiv_vivobus_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_dss_crg : 1;
        unsigned int ip_rst_dss : 1;
        unsigned int ip_rst_qic_dss : 1;
        unsigned int ip_rst_qic_dss_cfg : 1;
        unsigned int ip_prst_atgm_dss : 1;
        unsigned int ip_rst_gm_dss : 1;
        unsigned int ip_prst_dsi : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_vdec : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_gm_vdec : 1;
        unsigned int ip_rst_dss_disp_ch1_crg : 1;
        unsigned int ip_rst_dss_disp_ch1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_arst_jpg : 1;
        unsigned int ip_prst_jpg : 1;
        unsigned int ip_rst_jpg_func : 1;
        unsigned int ip_rst_arpp_func : 1;
        unsigned int ip_rst_arpp_crg : 1;
        unsigned int ip_prst_atgm_arpp : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_gm_ipp : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 5;
    } reg;
} SOC_MEDIA1_CRG_PERRSTEN0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_crg_START (0)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_crg_END (0)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_START (1)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_END (1)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_qic_dss_START (2)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_qic_dss_END (2)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_qic_dss_cfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_qic_dss_cfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_dss_START (4)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_dss_END (4)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_gm_dss_START (5)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_gm_dss_END (5)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_dsi_START (6)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_dsi_END (6)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_vdec_START (8)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_vdec_END (8)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_gm_vdec_START (11)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_gm_vdec_END (11)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_disp_ch1_crg_START (12)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_disp_ch1_crg_END (12)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_disp_ch1_START (13)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_dss_disp_ch1_END (13)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_arst_jpg_START (16)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_arst_jpg_END (16)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_jpg_START (17)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_jpg_END (17)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_jpg_func_START (18)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_jpg_func_END (18)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_arpp_func_START (19)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_arpp_func_END (19)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_arpp_crg_START (20)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_arpp_crg_END (20)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_arpp_START (21)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_prst_atgm_arpp_END (21)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_gm_ipp_START (24)
#define SOC_MEDIA1_CRG_PERRSTEN0_ip_rst_gm_ipp_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_dss_crg : 1;
        unsigned int ip_rst_dss : 1;
        unsigned int ip_rst_qic_dss : 1;
        unsigned int ip_rst_qic_dss_cfg : 1;
        unsigned int ip_prst_atgm_dss : 1;
        unsigned int ip_rst_gm_dss : 1;
        unsigned int ip_prst_dsi : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_vdec : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_gm_vdec : 1;
        unsigned int ip_rst_dss_disp_ch1_crg : 1;
        unsigned int ip_rst_dss_disp_ch1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_arst_jpg : 1;
        unsigned int ip_prst_jpg : 1;
        unsigned int ip_rst_jpg_func : 1;
        unsigned int ip_rst_arpp_func : 1;
        unsigned int ip_rst_arpp_crg : 1;
        unsigned int ip_prst_atgm_arpp : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_gm_ipp : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 5;
    } reg;
} SOC_MEDIA1_CRG_PERRSTDIS0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_crg_START (0)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_crg_END (0)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_START (1)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_END (1)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_qic_dss_START (2)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_qic_dss_END (2)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_qic_dss_cfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_qic_dss_cfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_dss_START (4)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_dss_END (4)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_gm_dss_START (5)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_gm_dss_END (5)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_dsi_START (6)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_dsi_END (6)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_vdec_START (8)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_vdec_END (8)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_gm_vdec_START (11)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_gm_vdec_END (11)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_disp_ch1_crg_START (12)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_disp_ch1_crg_END (12)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_disp_ch1_START (13)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_dss_disp_ch1_END (13)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_arst_jpg_START (16)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_arst_jpg_END (16)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_jpg_START (17)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_jpg_END (17)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_jpg_func_START (18)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_jpg_func_END (18)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_arpp_func_START (19)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_arpp_func_END (19)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_arpp_crg_START (20)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_arpp_crg_END (20)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_arpp_START (21)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_prst_atgm_arpp_END (21)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_gm_ipp_START (24)
#define SOC_MEDIA1_CRG_PERRSTDIS0_ip_rst_gm_ipp_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_dss_crg : 1;
        unsigned int ip_rst_dss : 1;
        unsigned int ip_rst_qic_dss : 1;
        unsigned int ip_rst_qic_dss_cfg : 1;
        unsigned int ip_prst_atgm_dss : 1;
        unsigned int ip_rst_gm_dss : 1;
        unsigned int ip_prst_dsi : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_vdec : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_gm_vdec : 1;
        unsigned int ip_rst_dss_disp_ch1_crg : 1;
        unsigned int ip_rst_dss_disp_ch1 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int ip_arst_jpg : 1;
        unsigned int ip_prst_jpg : 1;
        unsigned int ip_rst_jpg_func : 1;
        unsigned int ip_rst_arpp_func : 1;
        unsigned int ip_rst_arpp_crg : 1;
        unsigned int ip_prst_atgm_arpp : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int ip_rst_gm_ipp : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 5;
    } reg;
} SOC_MEDIA1_CRG_PERRSTSTAT0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_crg_START (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_crg_END (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_START (1)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_END (1)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_qic_dss_START (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_qic_dss_END (2)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_qic_dss_cfg_START (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_qic_dss_cfg_END (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_dss_START (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_dss_END (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_gm_dss_START (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_gm_dss_END (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_dsi_START (6)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_dsi_END (6)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_vdec_START (8)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_vdec_END (8)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_gm_vdec_START (11)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_gm_vdec_END (11)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_disp_ch1_crg_START (12)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_disp_ch1_crg_END (12)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_disp_ch1_START (13)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_dss_disp_ch1_END (13)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_arst_jpg_START (16)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_arst_jpg_END (16)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_jpg_START (17)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_jpg_END (17)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_jpg_func_START (18)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_jpg_func_END (18)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_arpp_func_START (19)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_arpp_func_END (19)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_arpp_crg_START (20)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_arpp_crg_END (20)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_arpp_START (21)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_prst_atgm_arpp_END (21)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_gm_ipp_START (24)
#define SOC_MEDIA1_CRG_PERRSTSTAT0_ip_rst_gm_ipp_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_dpm_dss : 1;
        unsigned int ip_rst_dpm_vdec : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_hpm : 1;
        unsigned int ip_rst_tcu_vivo : 1;
        unsigned int ip_rst_qtp_gms : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 24;
    } reg;
} SOC_MEDIA1_CRG_PERRSTEN1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_dpm_dss_START (0)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_dpm_dss_END (0)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_dpm_vdec_START (1)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_dpm_vdec_END (1)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_hpm_START (3)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_hpm_END (3)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_tcu_vivo_START (4)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_tcu_vivo_END (4)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_qtp_gms_START (5)
#define SOC_MEDIA1_CRG_PERRSTEN1_ip_rst_qtp_gms_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_dpm_dss : 1;
        unsigned int ip_rst_dpm_vdec : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_hpm : 1;
        unsigned int ip_rst_tcu_vivo : 1;
        unsigned int ip_rst_qtp_gms : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 24;
    } reg;
} SOC_MEDIA1_CRG_PERRSTDIS1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_dpm_dss_START (0)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_dpm_dss_END (0)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_dpm_vdec_START (1)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_dpm_vdec_END (1)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_hpm_START (3)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_hpm_END (3)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_tcu_vivo_START (4)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_tcu_vivo_END (4)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_qtp_gms_START (5)
#define SOC_MEDIA1_CRG_PERRSTDIS1_ip_rst_qtp_gms_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_dpm_dss : 1;
        unsigned int ip_rst_dpm_vdec : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_hpm : 1;
        unsigned int ip_rst_tcu_vivo : 1;
        unsigned int ip_rst_qtp_gms : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 24;
    } reg;
} SOC_MEDIA1_CRG_PERRSTSTAT1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_dpm_dss_START (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_dpm_dss_END (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_dpm_vdec_START (1)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_dpm_vdec_END (1)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_hpm_START (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_hpm_END (3)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_tcu_vivo_START (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_tcu_vivo_END (4)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_qtp_gms_START (5)
#define SOC_MEDIA1_CRG_PERRSTSTAT1_ip_rst_qtp_gms_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_vivobus_disp : 6;
        unsigned int sel_vivobus_disp_pll : 4;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV0_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV0_div_vivobus_disp_START (0)
#define SOC_MEDIA1_CRG_CLKDIV0_div_vivobus_disp_END (5)
#define SOC_MEDIA1_CRG_CLKDIV0_sel_vivobus_disp_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV0_sel_vivobus_disp_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV0_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_vdec : 6;
        unsigned int sel_vdec_pll : 4;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV1_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV1_div_vdec_START (0)
#define SOC_MEDIA1_CRG_CLKDIV1_div_vdec_END (5)
#define SOC_MEDIA1_CRG_CLKDIV1_sel_vdec_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV1_sel_vdec_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV1_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV1_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_jpg_func : 6;
        unsigned int sel_jpg_func_pll : 4;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV2_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV2_div_jpg_func_START (0)
#define SOC_MEDIA1_CRG_CLKDIV2_div_jpg_func_END (5)
#define SOC_MEDIA1_CRG_CLKDIV2_sel_jpg_func_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV2_sel_jpg_func_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV2_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV2_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_arpp_func : 6;
        unsigned int sel_arpp_func_pll : 4;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV3_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV3_div_arpp_func_START (0)
#define SOC_MEDIA1_CRG_CLKDIV3_div_arpp_func_END (5)
#define SOC_MEDIA1_CRG_CLKDIV3_sel_arpp_func_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV3_sel_arpp_func_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV3_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV3_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_vivobus : 6;
        unsigned int sel_vivobus_pll : 4;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV4_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV4_div_vivobus_START (0)
#define SOC_MEDIA1_CRG_CLKDIV4_div_vivobus_END (5)
#define SOC_MEDIA1_CRG_CLKDIV4_sel_vivobus_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV4_sel_vivobus_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV4_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV4_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_dpm : 6;
        unsigned int sel_dpm_pll : 4;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV5_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV5_div_dpm_START (0)
#define SOC_MEDIA1_CRG_CLKDIV5_div_dpm_END (5)
#define SOC_MEDIA1_CRG_CLKDIV5_sel_dpm_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV5_sel_dpm_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV5_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV5_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int div_hpm : 6;
        unsigned int sel_hpm_pll : 4;
        unsigned int reserved : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV6_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV6_div_hpm_START (0)
#define SOC_MEDIA1_CRG_CLKDIV6_div_hpm_END (5)
#define SOC_MEDIA1_CRG_CLKDIV6_sel_hpm_pll_START (6)
#define SOC_MEDIA1_CRG_CLKDIV6_sel_hpm_pll_END (9)
#define SOC_MEDIA1_CRG_CLKDIV6_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV6_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 4;
        unsigned int reserved_1: 6;
        unsigned int reserved_2: 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV7_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV7_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV7_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 6;
        unsigned int reserved_1: 6;
        unsigned int reserved_2: 4;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV8_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV8_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV8_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_gt_clk_vivobus_disp : 1;
        unsigned int sc_gt_clk_vdec : 1;
        unsigned int sc_gt_clk_jpg_func : 1;
        unsigned int sc_gt_clk_arpp_func : 1;
        unsigned int sc_gt_clk_vivobus : 1;
        unsigned int sc_gt_clk_dpm : 1;
        unsigned int sc_gt_clk_hpm : 1;
        unsigned int reserved : 9;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV9_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_disp_START (0)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_disp_END (0)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vdec_START (1)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vdec_END (1)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_jpg_func_START (2)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_jpg_func_END (2)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_arpp_func_START (3)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_arpp_func_END (3)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_START (4)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_END (4)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_dpm_START (5)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_dpm_END (5)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_hpm_START (6)
#define SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_hpm_END (6)
#define SOC_MEDIA1_CRG_CLKDIV9_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV9_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 4;
        unsigned int reserved_1: 6;
        unsigned int reserved_2: 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV10_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV10_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV10_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 13;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_CLKDIV16_UNION;
#endif
#define SOC_MEDIA1_CRG_CLKDIV16_bitmasken_START (16)
#define SOC_MEDIA1_CRG_CLKDIV16_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int swdone_clk_vivobus_disp_div : 1;
        unsigned int swdone_clk_vdec_div : 1;
        unsigned int swdone_clk_jpg_func_div : 1;
        unsigned int swdone_clk_arpp_func_div : 1;
        unsigned int swdone_clk_vivobus_div : 1;
        unsigned int swdone_clk_dpm_div : 1;
        unsigned int swdone_clk_hpm_div : 1;
        unsigned int reserved_0 : 9;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_MEDIA1_CRG_PERI_STAT0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_vivobus_disp_div_START (0)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_vivobus_disp_div_END (0)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_vdec_div_START (1)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_vdec_div_END (1)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_jpg_func_div_START (2)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_jpg_func_div_END (2)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_arpp_func_div_START (3)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_arpp_func_div_END (3)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_vivobus_div_START (4)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_vivobus_div_END (4)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_dpm_div_START (5)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_dpm_div_END (5)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_hpm_div_START (6)
#define SOC_MEDIA1_CRG_PERI_STAT0_swdone_clk_hpm_div_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sw_ack_clk_vivobus_disp_sw : 4;
        unsigned int sw_ack_clk_vdec_sw : 4;
        unsigned int sw_ack_clk_jpg_func_sw : 4;
        unsigned int sw_ack_clk_arpp_func_sw : 4;
        unsigned int sw_ack_clk_vivobus_sw : 4;
        unsigned int sw_ack_clk_dpm_sw : 4;
        unsigned int sw_ack_clk_hpm_sw : 4;
        unsigned int reserved : 4;
    } reg;
} SOC_MEDIA1_CRG_PERI_STAT1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_vivobus_disp_sw_START (0)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_vivobus_disp_sw_END (3)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_vdec_sw_START (4)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_vdec_sw_END (7)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_jpg_func_sw_START (8)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_jpg_func_sw_END (11)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_arpp_func_sw_START (12)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_arpp_func_sw_END (15)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_vivobus_sw_START (16)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_vivobus_sw_END (19)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_dpm_sw_START (20)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_dpm_sw_END (23)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_hpm_sw_START (24)
#define SOC_MEDIA1_CRG_PERI_STAT1_sw_ack_clk_hpm_sw_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 20;
        unsigned int reserved_1: 12;
    } reg;
} SOC_MEDIA1_CRG_PERI_STAT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hw_dvfs_en : 1;
        unsigned int reserved_0 : 15;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_MEDIA1_CRG_DSS_DVFS_UNION;
#endif
#define SOC_MEDIA1_CRG_DSS_DVFS_hw_dvfs_en_START (0)
#define SOC_MEDIA1_CRG_DSS_DVFS_hw_dvfs_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int tcu_cfg_clkrst_bypass : 1;
        unsigned int dpm_dss_cfg_clkrst_bypass : 1;
        unsigned int dpm_vdec_cfg_clkrst_bypass : 1;
        unsigned int qic_clkrst_flag_dss_data_bypass : 1;
        unsigned int qic_clkrst_flag_dss_gm_bypass : 1;
        unsigned int qic_clkrst_flag_vdec_data_bypass : 1;
        unsigned int qic_clkrst_flag_vdec_gm_bypass : 1;
        unsigned int qic_clkrst_flag_ipp_data_bypass : 1;
        unsigned int qic_clkrst_flag_ipp_gm_bypass : 1;
        unsigned int qic_clkrst_flag_media1sub_dss_data_bypass : 1;
        unsigned int qic_clkrst_flag_dss_cfg_bypass : 1;
        unsigned int qic_clkrst_flag_vdec_cfg_bypass : 1;
        unsigned int qic_clkrst_flag_ipp_cfg_bypass : 1;
        unsigned int qic_clkrst_flag_vivo_bypass : 1;
        unsigned int qic_clkrst_flag_media1sub_qtp_gm_bypass : 1;
        unsigned int qic_clkrst_flag_media1sub_qice_data_bypass : 1;
        unsigned int int_hub_clkrst_bypass : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_MEDIA1_CRG_IPCLKRST_BYPASS_UNION;
#endif
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_tcu_cfg_clkrst_bypass_START (0)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_tcu_cfg_clkrst_bypass_END (0)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_dpm_dss_cfg_clkrst_bypass_START (1)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_dpm_dss_cfg_clkrst_bypass_END (1)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_dpm_vdec_cfg_clkrst_bypass_START (2)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_dpm_vdec_cfg_clkrst_bypass_END (2)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_dss_data_bypass_START (3)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_dss_data_bypass_END (3)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_dss_gm_bypass_START (4)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_dss_gm_bypass_END (4)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_vdec_data_bypass_START (5)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_vdec_data_bypass_END (5)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_vdec_gm_bypass_START (6)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_vdec_gm_bypass_END (6)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_ipp_data_bypass_START (7)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_ipp_data_bypass_END (7)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_ipp_gm_bypass_START (8)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_ipp_gm_bypass_END (8)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_media1sub_dss_data_bypass_START (9)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_media1sub_dss_data_bypass_END (9)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_dss_cfg_bypass_START (10)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_dss_cfg_bypass_END (10)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_vdec_cfg_bypass_START (11)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_vdec_cfg_bypass_END (11)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_ipp_cfg_bypass_START (12)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_ipp_cfg_bypass_END (12)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_vivo_bypass_START (13)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_vivo_bypass_END (13)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_media1sub_qtp_gm_bypass_START (14)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_media1sub_qtp_gm_bypass_END (14)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_media1sub_qice_data_bypass_START (15)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_qic_clkrst_flag_media1sub_qice_data_bypass_END (15)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_int_hub_clkrst_bypass_START (16)
#define SOC_MEDIA1_CRG_IPCLKRST_BYPASS_int_hub_clkrst_bypass_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vdec_div_auto_reduce_bypass : 1;
        unsigned int vdec_auto_waitcfg_in : 10;
        unsigned int vdec_auto_waitcfg_out : 10;
        unsigned int vdec_div_auto_cfg : 6;
        unsigned int reserved : 5;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_vdec_div_auto_reduce_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_vdec_div_auto_reduce_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_vdec_auto_waitcfg_in_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_vdec_auto_waitcfg_in_END (10)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_vdec_auto_waitcfg_out_START (11)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_vdec_auto_waitcfg_out_END (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_vdec_div_auto_cfg_START (21)
#define SOC_MEDIA1_CRG_PERI_AUTODIV0_vdec_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int qice_auto_waitcfg_in : 10;
        unsigned int qice_auto_waitcfg_out : 10;
        unsigned int reserved : 12;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV1_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_qice_auto_waitcfg_in_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_qice_auto_waitcfg_in_END (9)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_qice_auto_waitcfg_out_START (10)
#define SOC_MEDIA1_CRG_PERI_AUTODIV1_qice_auto_waitcfg_out_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vivobus_vivo_bypass : 1;
        unsigned int vivobus_ipp_gm_bypass : 1;
        unsigned int vivobus_ipp_bypass : 1;
        unsigned int vivobus_vdec_bypass : 1;
        unsigned int vivobus_vdec_idle_bypass : 1;
        unsigned int vivobus_vdec_gm_bypass : 1;
        unsigned int vivobus_arpp_minibus_idle_pac_bypass : 1;
        unsigned int vivobus_arpp_minibus_idle_lba_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int vivobus_perf_idle_bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 21;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV2_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_vivo_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_vivo_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_ipp_gm_bypass_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_ipp_gm_bypass_END (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_ipp_bypass_START (2)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_ipp_bypass_END (2)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_vdec_bypass_START (3)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_vdec_bypass_END (3)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_vdec_idle_bypass_START (4)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_vdec_idle_bypass_END (4)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_vdec_gm_bypass_START (5)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_vdec_gm_bypass_END (5)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_arpp_minibus_idle_pac_bypass_START (6)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_arpp_minibus_idle_pac_bypass_END (6)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_arpp_minibus_idle_lba_bypass_START (7)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_arpp_minibus_idle_lba_bypass_END (7)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_perf_idle_bypass_START (9)
#define SOC_MEDIA1_CRG_PERI_AUTODIV2_vivobus_perf_idle_bypass_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vivobus_div_auto_reduce_bypass : 1;
        unsigned int vivobus_auto_waitcfg_in : 10;
        unsigned int vivobus_auto_waitcfg_out : 10;
        unsigned int vivobus_div_auto_cfg : 6;
        unsigned int reserved : 5;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV3_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_vivobus_div_auto_reduce_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_vivobus_div_auto_reduce_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_vivobus_auto_waitcfg_in_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_vivobus_auto_waitcfg_in_END (10)
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_vivobus_auto_waitcfg_out_START (11)
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_vivobus_auto_waitcfg_out_END (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_vivobus_div_auto_cfg_START (21)
#define SOC_MEDIA1_CRG_PERI_AUTODIV3_vivobus_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vivobus_disp_div_auto_reduce_bypass : 1;
        unsigned int vivobus_disp_auto_waitcfg_in : 10;
        unsigned int vivobus_disp_auto_waitcfg_out : 10;
        unsigned int vivobus_disp_div_auto_cfg : 6;
        unsigned int reserved : 5;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV4_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_disp_div_auto_reduce_bypass_START (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_disp_div_auto_reduce_bypass_END (0)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_disp_auto_waitcfg_in_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_disp_auto_waitcfg_in_END (10)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_disp_auto_waitcfg_out_START (11)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_disp_auto_waitcfg_out_END (20)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_disp_div_auto_cfg_START (21)
#define SOC_MEDIA1_CRG_PERI_AUTODIV4_vivobus_disp_div_auto_cfg_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int vivobus_disp_dss_gm_bypass : 1;
        unsigned int vivobus_disp_dss_bypass : 1;
        unsigned int vivobus_disp_ib_cfg_bypass : 1;
        unsigned int vivobus_disp_dss_tbx_cfg_bypass : 1;
        unsigned int vivobus_disp_dss_idle_bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int vivobus_disp_perf_idle_bypass : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 21;
    } reg;
} SOC_MEDIA1_CRG_PERI_AUTODIV5_UNION;
#endif
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_dss_gm_bypass_START (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_dss_gm_bypass_END (1)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_dss_bypass_START (2)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_dss_bypass_END (2)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_ib_cfg_bypass_START (3)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_ib_cfg_bypass_END (3)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_dss_tbx_cfg_bypass_START (4)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_dss_tbx_cfg_bypass_END (4)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_dss_idle_bypass_START (5)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_dss_idle_bypass_END (5)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_perf_idle_bypass_START (9)
#define SOC_MEDIA1_CRG_PERI_AUTODIV5_vivobus_disp_perf_idle_bypass_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int auto_div_vdec_stat : 1;
        unsigned int auto_div_vivobus_stat : 1;
        unsigned int auto_div_vivobus_disp_stat : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 23;
    } reg;
} SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_UNION;
#endif
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_auto_div_vdec_stat_START (1)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_auto_div_vdec_stat_END (1)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_auto_div_vivobus_stat_START (2)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_auto_div_vivobus_stat_END (2)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_auto_div_vivobus_disp_stat_START (3)
#define SOC_MEDIA1_CRG_MEDIA_AUTODIV_STAT_auto_div_vivobus_disp_stat_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_clr_mediabus_nonidle_pend : 1;
        unsigned int intr_mask_vivobus_nonidle_pend : 1;
        unsigned int intr_mask_dispbus_nonidle_pend : 1;
        unsigned int intr_mask_ippbus_nonidle_pend : 1;
        unsigned int intr_mask_vdecbus_nonidle_pend : 1;
        unsigned int intr_mask_ibqe_nonidle_pend : 1;
        unsigned int reserved : 10;
        unsigned int bitmasken : 16;
    } reg;
} SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_UNION;
#endif
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_clr_mediabus_nonidle_pend_START (0)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_clr_mediabus_nonidle_pend_END (0)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_vivobus_nonidle_pend_START (1)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_vivobus_nonidle_pend_END (1)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_dispbus_nonidle_pend_START (2)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_dispbus_nonidle_pend_END (2)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_ippbus_nonidle_pend_START (3)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_ippbus_nonidle_pend_END (3)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_vdecbus_nonidle_pend_START (4)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_vdecbus_nonidle_pend_END (4)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_ibqe_nonidle_pend_START (5)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_intr_mask_ibqe_nonidle_pend_END (5)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_bitmasken_START (16)
#define SOC_MEDIA1_CRG_INTR_QIC_BUS_NONIDLE_PEND_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_stat_vivobus_nonidle_pend : 1;
        unsigned int intr_stat_dispbus_nonidle_pend : 1;
        unsigned int intr_stat_ippbus_nonidle_pend : 1;
        unsigned int intr_stat_vdecbus_nonidle_pend : 1;
        unsigned int intr_stat_ibqe_nonidle_pend : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_UNION;
#endif
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_vivobus_nonidle_pend_START (0)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_vivobus_nonidle_pend_END (0)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_dispbus_nonidle_pend_START (1)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_dispbus_nonidle_pend_END (1)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_ippbus_nonidle_pend_START (2)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_ippbus_nonidle_pend_END (2)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_vdecbus_nonidle_pend_START (3)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_vdecbus_nonidle_pend_END (3)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_ibqe_nonidle_pend_START (4)
#define SOC_MEDIA1_CRG_INTR_STAT_QIC_BUS_NONIDLE_PEND_intr_stat_ibqe_nonidle_pend_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm_en : 1;
        unsigned int hpm_opc_min_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int hpm_clk_div : 6;
        unsigned int reserved_2 : 2;
        unsigned int reserved_3 : 4;
        unsigned int reserved_4 : 16;
    } reg;
} SOC_MEDIA1_CRG_HPM_CFG_UNION;
#endif
#define SOC_MEDIA1_CRG_HPM_CFG_hpm_en_START (0)
#define SOC_MEDIA1_CRG_HPM_CFG_hpm_en_END (0)
#define SOC_MEDIA1_CRG_HPM_CFG_hpm_opc_min_en_START (1)
#define SOC_MEDIA1_CRG_HPM_CFG_hpm_opc_min_en_END (1)
#define SOC_MEDIA1_CRG_HPM_CFG_hpm_clk_div_START (4)
#define SOC_MEDIA1_CRG_HPM_CFG_hpm_clk_div_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm_opc_vld : 1;
        unsigned int hpm_opc : 10;
        unsigned int hpm_opc_min : 10;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_MEDIA1_CRG_HPM_READ_BACK_UNION;
#endif
#define SOC_MEDIA1_CRG_HPM_READ_BACK_hpm_opc_vld_START (0)
#define SOC_MEDIA1_CRG_HPM_READ_BACK_hpm_opc_vld_END (0)
#define SOC_MEDIA1_CRG_HPM_READ_BACK_hpm_opc_START (1)
#define SOC_MEDIA1_CRG_HPM_READ_BACK_hpm_opc_END (10)
#define SOC_MEDIA1_CRG_HPM_READ_BACK_hpm_opc_min_START (11)
#define SOC_MEDIA1_CRG_HPM_READ_BACK_hpm_opc_min_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpmx_en : 1;
        unsigned int hpmx_opc_min_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 12;
        unsigned int reserved_3 : 16;
    } reg;
} SOC_MEDIA1_CRG_HPMX_CFG_UNION;
#endif
#define SOC_MEDIA1_CRG_HPMX_CFG_hpmx_en_START (0)
#define SOC_MEDIA1_CRG_HPMX_CFG_hpmx_en_END (0)
#define SOC_MEDIA1_CRG_HPMX_CFG_hpmx_opc_min_en_START (1)
#define SOC_MEDIA1_CRG_HPMX_CFG_hpmx_opc_min_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpmx_opc_vld : 1;
        unsigned int hpmx_opc : 10;
        unsigned int hpmx_opc_min : 10;
        unsigned int reserved_0 : 3;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_MEDIA1_CRG_HPMX_READ_BACK_UNION;
#endif
#define SOC_MEDIA1_CRG_HPMX_READ_BACK_hpmx_opc_vld_START (0)
#define SOC_MEDIA1_CRG_HPMX_READ_BACK_hpmx_opc_vld_END (0)
#define SOC_MEDIA1_CRG_HPMX_READ_BACK_hpmx_opc_START (1)
#define SOC_MEDIA1_CRG_HPMX_READ_BACK_hpmx_opc_END (10)
#define SOC_MEDIA1_CRG_HPMX_READ_BACK_hpmx_opc_min_START (11)
#define SOC_MEDIA1_CRG_HPMX_READ_BACK_hpmx_opc_min_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int himinibus_dss_cfg_ckg_en : 1;
        unsigned int himinibus_ipp_x2p_ckg_en : 1;
        unsigned int ipp_2to3_hiaxi_ckg_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 12;
        unsigned int reserved_2 : 16;
    } reg;
} SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_UNION;
#endif
#define SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_himinibus_dss_cfg_ckg_en_START (0)
#define SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_himinibus_dss_cfg_ckg_en_END (0)
#define SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_himinibus_ipp_x2p_ckg_en_START (1)
#define SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_himinibus_ipp_x2p_ckg_en_END (1)
#define SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_ipp_2to3_hiaxi_ckg_en_START (2)
#define SOC_MEDIA1_CRG_HIMINIBUS_CKG_EN_ipp_2to3_hiaxi_ckg_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ipp_2to3_hiaxi_dlock_clr : 1;
        unsigned int reserved_0 : 15;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_MEDIA1_CRG_HIMINIBUS_DLOCK_CLR_UNION;
#endif
#define SOC_MEDIA1_CRG_HIMINIBUS_DLOCK_CLR_ipp_2to3_hiaxi_dlock_clr_START (0)
#define SOC_MEDIA1_CRG_HIMINIBUS_DLOCK_CLR_ipp_2to3_hiaxi_dlock_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int vdec_idle_flag : 1;
        unsigned int reserved_0 : 10;
        unsigned int reserved_1 : 10;
        unsigned int reserved_2 : 3;
        unsigned int reserved_3 : 8;
    } reg;
} SOC_MEDIA1_CRG_VDEC_IDLE_READ_BACK_UNION;
#endif
#define SOC_MEDIA1_CRG_VDEC_IDLE_READ_BACK_vdec_idle_flag_START (0)
#define SOC_MEDIA1_CRG_VDEC_IDLE_READ_BACK_vdec_idle_flag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dss_vsync_sel : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_MEDIA1_CRG_DSS_VSYNC_SEL_UNION;
#endif
#define SOC_MEDIA1_CRG_DSS_VSYNC_SEL_dss_vsync_sel_START (0)
#define SOC_MEDIA1_CRG_DSS_VSYNC_SEL_dss_vsync_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ipp_dlock_mst : 4;
        unsigned int ipp_dlock_slv : 6;
        unsigned int ipp_hiaxi_idle : 1;
        unsigned int ipp_himinibus_x2p_idle : 1;
        unsigned int ipp_2to3_himinibus_dbg_ar : 4;
        unsigned int ipp_2to3_himinibus_dbg_aw : 4;
        unsigned int reserved : 12;
    } reg;
} SOC_MEDIA1_CRG_IPP_READ_BACK_UNION;
#endif
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_dlock_mst_START (0)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_dlock_mst_END (3)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_dlock_slv_START (4)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_dlock_slv_END (9)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_hiaxi_idle_START (10)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_hiaxi_idle_END (10)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_himinibus_x2p_idle_START (11)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_himinibus_x2p_idle_END (11)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_2to3_himinibus_dbg_ar_START (12)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_2to3_himinibus_dbg_ar_END (15)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_2to3_himinibus_dbg_aw_START (16)
#define SOC_MEDIA1_CRG_IPP_READ_BACK_ipp_2to3_himinibus_dbg_aw_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int himinibus_dss_cfg_idle : 1;
        unsigned int reserved_0 : 15;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_MEDIA1_CRG_HIMINIBUS_IDLE_READ_BACK_UNION;
#endif
#define SOC_MEDIA1_CRG_HIMINIBUS_IDLE_READ_BACK_himinibus_dss_cfg_idle_START (0)
#define SOC_MEDIA1_CRG_HIMINIBUS_IDLE_READ_BACK_himinibus_dss_cfg_idle_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 29;
    } reg;
} SOC_MEDIA1_CRG_DECPROT0SET_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_int_hub : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 28;
    } reg;
} SOC_MEDIA1_CRG_PERRSTEN_SAFE0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTEN_SAFE0_ip_rst_int_hub_START (0)
#define SOC_MEDIA1_CRG_PERRSTEN_SAFE0_ip_rst_int_hub_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_int_hub : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 28;
    } reg;
} SOC_MEDIA1_CRG_PERRSTDIS_SAFE0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTDIS_SAFE0_ip_rst_int_hub_START (0)
#define SOC_MEDIA1_CRG_PERRSTDIS_SAFE0_ip_rst_int_hub_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_int_hub : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 28;
    } reg;
} SOC_MEDIA1_CRG_PERRSTSTAT_SAFE0_UNION;
#endif
#define SOC_MEDIA1_CRG_PERRSTSTAT_SAFE0_ip_rst_int_hub_START (0)
#define SOC_MEDIA1_CRG_PERRSTSTAT_SAFE0_ip_rst_int_hub_END (0)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
