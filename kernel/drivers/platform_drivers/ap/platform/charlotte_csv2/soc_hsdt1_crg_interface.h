#ifndef __SOC_HSDT1_CRG_INTERFACE_H__
#define __SOC_HSDT1_CRG_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_HSDT1_CRG_PEREN0_ADDR(base) ((base) + (0x000UL))
#define SOC_HSDT1_CRG_PERDIS0_ADDR(base) ((base) + (0x004UL))
#define SOC_HSDT1_CRG_PERCLKEN0_ADDR(base) ((base) + (0x008UL))
#define SOC_HSDT1_CRG_PERSTAT0_ADDR(base) ((base) + (0x00CUL))
#define SOC_HSDT1_CRG_PEREN1_ADDR(base) ((base) + (0x010UL))
#define SOC_HSDT1_CRG_PERDIS1_ADDR(base) ((base) + (0x014UL))
#define SOC_HSDT1_CRG_PERCLKEN1_ADDR(base) ((base) + (0x018UL))
#define SOC_HSDT1_CRG_PERSTAT1_ADDR(base) ((base) + (0x01CUL))
#define SOC_HSDT1_CRG_PERRSTEN0_ADDR(base) ((base) + (0x020UL))
#define SOC_HSDT1_CRG_PERRSTDIS0_ADDR(base) ((base) + (0x024UL))
#define SOC_HSDT1_CRG_PERRSTSTAT0_ADDR(base) ((base) + (0x028UL))
#define SOC_HSDT1_CRG_PERRSTEN1_ADDR(base) ((base) + (0x02CUL))
#define SOC_HSDT1_CRG_PERRSTDIS1_ADDR(base) ((base) + (0x030UL))
#define SOC_HSDT1_CRG_PERRSTSTAT1_ADDR(base) ((base) + (0x034UL))
#define SOC_HSDT1_CRG_CLKDIV0_ADDR(base) ((base) + (0x0A8UL))
#define SOC_HSDT1_CRG_CLKDIV1_ADDR(base) ((base) + (0x0ACUL))
#define SOC_HSDT1_CRG_CLKDIV2_ADDR(base) ((base) + (0x0B0UL))
#define SOC_HSDT1_CRG_PERI_STAT0_ADDR(base) ((base) + (0x0C8UL))
#define SOC_HSDT1_CRG_PERI_STAT1_ADDR(base) ((base) + (0x0CCUL))
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_ADDR(base) ((base) + (0x100UL))
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_ADDR(base) ((base) + (0x104UL))
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_ADDR(base) ((base) + (0x190UL))
#define SOC_HSDT1_CRG_SDPLLCTRL0_ADDR(base) ((base) + (0x200UL))
#define SOC_HSDT1_CRG_SDPLLCTRL1_ADDR(base) ((base) + (0x204UL))
#define SOC_HSDT1_CRG_PCIEPLL_STAT_ADDR(base) ((base) + (0x208UL))
#define SOC_HSDT1_CRG_FNPLL_CFG0_ADDR(base) ((base) + (0x224UL))
#define SOC_HSDT1_CRG_FNPLL_CFG1_ADDR(base) ((base) + (0x228UL))
#define SOC_HSDT1_CRG_FNPLL_CFG2_ADDR(base) ((base) + (0x22CUL))
#define SOC_HSDT1_CRG_FNPLL_CFG3_ADDR(base) ((base) + (0x230UL))
#define SOC_HSDT1_CRG_PERI_AUTODIV0_ADDR(base) ((base) + (0x350UL))
#define SOC_HSDT1_CRG_ISOEN_ADDR(base) ((base) + (0x280UL))
#define SOC_HSDT1_CRG_ISODIS_ADDR(base) ((base) + (0x284UL))
#define SOC_HSDT1_CRG_ISOSTAT_ADDR(base) ((base) + (0x288UL))
#define SOC_HSDT1_CRG_FNPLL_STAT0_SD_ADDR(base) ((base) + (0x408UL))
#define SOC_HSDT1_CRG_INTR_MASK_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x500UL))
#define SOC_HSDT1_CRG_INTR_CLR_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x504UL))
#define SOC_HSDT1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x508UL))
#else
#define SOC_HSDT1_CRG_PEREN0_ADDR(base) ((base) + (0x000))
#define SOC_HSDT1_CRG_PERDIS0_ADDR(base) ((base) + (0x004))
#define SOC_HSDT1_CRG_PERCLKEN0_ADDR(base) ((base) + (0x008))
#define SOC_HSDT1_CRG_PERSTAT0_ADDR(base) ((base) + (0x00C))
#define SOC_HSDT1_CRG_PEREN1_ADDR(base) ((base) + (0x010))
#define SOC_HSDT1_CRG_PERDIS1_ADDR(base) ((base) + (0x014))
#define SOC_HSDT1_CRG_PERCLKEN1_ADDR(base) ((base) + (0x018))
#define SOC_HSDT1_CRG_PERSTAT1_ADDR(base) ((base) + (0x01C))
#define SOC_HSDT1_CRG_PERRSTEN0_ADDR(base) ((base) + (0x020))
#define SOC_HSDT1_CRG_PERRSTDIS0_ADDR(base) ((base) + (0x024))
#define SOC_HSDT1_CRG_PERRSTSTAT0_ADDR(base) ((base) + (0x028))
#define SOC_HSDT1_CRG_PERRSTEN1_ADDR(base) ((base) + (0x02C))
#define SOC_HSDT1_CRG_PERRSTDIS1_ADDR(base) ((base) + (0x030))
#define SOC_HSDT1_CRG_PERRSTSTAT1_ADDR(base) ((base) + (0x034))
#define SOC_HSDT1_CRG_CLKDIV0_ADDR(base) ((base) + (0x0A8))
#define SOC_HSDT1_CRG_CLKDIV1_ADDR(base) ((base) + (0x0AC))
#define SOC_HSDT1_CRG_CLKDIV2_ADDR(base) ((base) + (0x0B0))
#define SOC_HSDT1_CRG_PERI_STAT0_ADDR(base) ((base) + (0x0C8))
#define SOC_HSDT1_CRG_PERI_STAT1_ADDR(base) ((base) + (0x0CC))
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_ADDR(base) ((base) + (0x100))
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_ADDR(base) ((base) + (0x104))
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_ADDR(base) ((base) + (0x190))
#define SOC_HSDT1_CRG_SDPLLCTRL0_ADDR(base) ((base) + (0x200))
#define SOC_HSDT1_CRG_SDPLLCTRL1_ADDR(base) ((base) + (0x204))
#define SOC_HSDT1_CRG_PCIEPLL_STAT_ADDR(base) ((base) + (0x208))
#define SOC_HSDT1_CRG_FNPLL_CFG0_ADDR(base) ((base) + (0x224))
#define SOC_HSDT1_CRG_FNPLL_CFG1_ADDR(base) ((base) + (0x228))
#define SOC_HSDT1_CRG_FNPLL_CFG2_ADDR(base) ((base) + (0x22C))
#define SOC_HSDT1_CRG_FNPLL_CFG3_ADDR(base) ((base) + (0x230))
#define SOC_HSDT1_CRG_PERI_AUTODIV0_ADDR(base) ((base) + (0x350))
#define SOC_HSDT1_CRG_ISOEN_ADDR(base) ((base) + (0x280))
#define SOC_HSDT1_CRG_ISODIS_ADDR(base) ((base) + (0x284))
#define SOC_HSDT1_CRG_ISOSTAT_ADDR(base) ((base) + (0x288))
#define SOC_HSDT1_CRG_FNPLL_STAT0_SD_ADDR(base) ((base) + (0x408))
#define SOC_HSDT1_CRG_INTR_MASK_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x500))
#define SOC_HSDT1_CRG_INTR_CLR_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x504))
#define SOC_HSDT1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_ADDR(base) ((base) + (0x508))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_hclk_sd : 1;
        unsigned int gt_pclk_hsdt1_sysctrl : 1;
        unsigned int gt_pclk_usb2phy : 1;
        unsigned int gt_pclk_dpctrl : 1;
        unsigned int gt_pclk_hsdt1_ioc : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_aclk_usb3otg : 1;
        unsigned int gt_clk_aux16_dpctrl : 1;
        unsigned int gt_aclk_noc_usb3otg_asyncbrg : 1;
        unsigned int gt_clk_hi_usb3ctrl_ref : 1;
        unsigned int gt_clk_snp_usb3ctrl_ref : 1;
        unsigned int gt_clk_snp_usb3_suspend : 1;
        unsigned int gt_aclk_hi_usb3 : 1;
        unsigned int gt_aclk_snp_usb3 : 1;
        unsigned int gt_clk_hi_usb3_suspend : 1;
        unsigned int gt_clk_hi_usb3_sys : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_usb2phy_ref : 1;
        unsigned int gt_aclk_hsdt1_trace : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_hclk_dcdr_mux : 1;
        unsigned int gt_clk_usbdp_aux : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_usb2_ulpi : 1;
        unsigned int gt_clk_usb31phy_apb : 1;
        unsigned int gt_pclk_dp_sctrl : 1;
        unsigned int gt_pclk_dp_trace : 1;
        unsigned int gt_pclk_usb_sctrl : 1;
        unsigned int gt_pclk_hiusb_trace : 1;
        unsigned int gt_pclk_snpsusb_trace : 1;
    } reg;
} SOC_HSDT1_CRG_PEREN0_UNION;
#endif
#define SOC_HSDT1_CRG_PEREN0_gt_hclk_sd_START (0)
#define SOC_HSDT1_CRG_PEREN0_gt_hclk_sd_END (0)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_hsdt1_sysctrl_START (1)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_hsdt1_sysctrl_END (1)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_usb2phy_START (2)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_usb2phy_END (2)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_dpctrl_START (3)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_dpctrl_END (3)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_hsdt1_ioc_START (4)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_hsdt1_ioc_END (4)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_usb3otg_START (6)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_usb3otg_END (6)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_aux16_dpctrl_START (7)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_aux16_dpctrl_END (7)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_noc_usb3otg_asyncbrg_START (8)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_noc_usb3otg_asyncbrg_END (8)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_hi_usb3ctrl_ref_START (9)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_hi_usb3ctrl_ref_END (9)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_snp_usb3ctrl_ref_START (10)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_snp_usb3ctrl_ref_END (10)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_snp_usb3_suspend_START (11)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_snp_usb3_suspend_END (11)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_hi_usb3_START (12)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_hi_usb3_END (12)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_snp_usb3_START (13)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_snp_usb3_END (13)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_hi_usb3_suspend_START (14)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_hi_usb3_suspend_END (14)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_hi_usb3_sys_START (15)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_hi_usb3_sys_END (15)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_usb2phy_ref_START (18)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_usb2phy_ref_END (18)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_hsdt1_trace_START (19)
#define SOC_HSDT1_CRG_PEREN0_gt_aclk_hsdt1_trace_END (19)
#define SOC_HSDT1_CRG_PEREN0_gt_hclk_dcdr_mux_START (22)
#define SOC_HSDT1_CRG_PEREN0_gt_hclk_dcdr_mux_END (22)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_usbdp_aux_START (23)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_usbdp_aux_END (23)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_usb2_ulpi_START (25)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_usb2_ulpi_END (25)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_usb31phy_apb_START (26)
#define SOC_HSDT1_CRG_PEREN0_gt_clk_usb31phy_apb_END (26)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_dp_sctrl_START (27)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_dp_sctrl_END (27)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_dp_trace_START (28)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_dp_trace_END (28)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_usb_sctrl_START (29)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_usb_sctrl_END (29)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_hiusb_trace_START (30)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_hiusb_trace_END (30)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_snpsusb_trace_START (31)
#define SOC_HSDT1_CRG_PEREN0_gt_pclk_snpsusb_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_hclk_sd : 1;
        unsigned int gt_pclk_hsdt1_sysctrl : 1;
        unsigned int gt_pclk_usb2phy : 1;
        unsigned int gt_pclk_dpctrl : 1;
        unsigned int gt_pclk_hsdt1_ioc : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_aclk_usb3otg : 1;
        unsigned int gt_clk_aux16_dpctrl : 1;
        unsigned int gt_aclk_noc_usb3otg_asyncbrg : 1;
        unsigned int gt_clk_hi_usb3ctrl_ref : 1;
        unsigned int gt_clk_snp_usb3ctrl_ref : 1;
        unsigned int gt_clk_snp_usb3_suspend : 1;
        unsigned int gt_aclk_hi_usb3 : 1;
        unsigned int gt_aclk_snp_usb3 : 1;
        unsigned int gt_clk_hi_usb3_suspend : 1;
        unsigned int gt_clk_hi_usb3_sys : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_usb2phy_ref : 1;
        unsigned int gt_aclk_hsdt1_trace : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_hclk_dcdr_mux : 1;
        unsigned int gt_clk_usbdp_aux : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_usb2_ulpi : 1;
        unsigned int gt_clk_usb31phy_apb : 1;
        unsigned int gt_pclk_dp_sctrl : 1;
        unsigned int gt_pclk_dp_trace : 1;
        unsigned int gt_pclk_usb_sctrl : 1;
        unsigned int gt_pclk_hiusb_trace : 1;
        unsigned int gt_pclk_snpsusb_trace : 1;
    } reg;
} SOC_HSDT1_CRG_PERDIS0_UNION;
#endif
#define SOC_HSDT1_CRG_PERDIS0_gt_hclk_sd_START (0)
#define SOC_HSDT1_CRG_PERDIS0_gt_hclk_sd_END (0)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_hsdt1_sysctrl_START (1)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_hsdt1_sysctrl_END (1)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_usb2phy_START (2)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_usb2phy_END (2)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_dpctrl_START (3)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_dpctrl_END (3)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_hsdt1_ioc_START (4)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_hsdt1_ioc_END (4)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_usb3otg_START (6)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_usb3otg_END (6)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_aux16_dpctrl_START (7)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_aux16_dpctrl_END (7)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_noc_usb3otg_asyncbrg_START (8)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_noc_usb3otg_asyncbrg_END (8)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_hi_usb3ctrl_ref_START (9)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_hi_usb3ctrl_ref_END (9)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_snp_usb3ctrl_ref_START (10)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_snp_usb3ctrl_ref_END (10)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_snp_usb3_suspend_START (11)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_snp_usb3_suspend_END (11)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_hi_usb3_START (12)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_hi_usb3_END (12)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_snp_usb3_START (13)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_snp_usb3_END (13)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_hi_usb3_suspend_START (14)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_hi_usb3_suspend_END (14)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_hi_usb3_sys_START (15)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_hi_usb3_sys_END (15)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_usb2phy_ref_START (18)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_usb2phy_ref_END (18)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_hsdt1_trace_START (19)
#define SOC_HSDT1_CRG_PERDIS0_gt_aclk_hsdt1_trace_END (19)
#define SOC_HSDT1_CRG_PERDIS0_gt_hclk_dcdr_mux_START (22)
#define SOC_HSDT1_CRG_PERDIS0_gt_hclk_dcdr_mux_END (22)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_usbdp_aux_START (23)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_usbdp_aux_END (23)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_usb2_ulpi_START (25)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_usb2_ulpi_END (25)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_usb31phy_apb_START (26)
#define SOC_HSDT1_CRG_PERDIS0_gt_clk_usb31phy_apb_END (26)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_dp_sctrl_START (27)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_dp_sctrl_END (27)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_dp_trace_START (28)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_dp_trace_END (28)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_usb_sctrl_START (29)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_usb_sctrl_END (29)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_hiusb_trace_START (30)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_hiusb_trace_END (30)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_snpsusb_trace_START (31)
#define SOC_HSDT1_CRG_PERDIS0_gt_pclk_snpsusb_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_hclk_sd : 1;
        unsigned int gt_pclk_hsdt1_sysctrl : 1;
        unsigned int gt_pclk_usb2phy : 1;
        unsigned int gt_pclk_dpctrl : 1;
        unsigned int gt_pclk_hsdt1_ioc : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_aclk_usb3otg : 1;
        unsigned int gt_clk_aux16_dpctrl : 1;
        unsigned int gt_aclk_noc_usb3otg_asyncbrg : 1;
        unsigned int gt_clk_hi_usb3ctrl_ref : 1;
        unsigned int gt_clk_snp_usb3ctrl_ref : 1;
        unsigned int gt_clk_snp_usb3_suspend : 1;
        unsigned int gt_aclk_hi_usb3 : 1;
        unsigned int gt_aclk_snp_usb3 : 1;
        unsigned int gt_clk_hi_usb3_suspend : 1;
        unsigned int gt_clk_hi_usb3_sys : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_usb2phy_ref : 1;
        unsigned int gt_aclk_hsdt1_trace : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_hclk_dcdr_mux : 1;
        unsigned int gt_clk_usbdp_aux : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_clk_usb2_ulpi : 1;
        unsigned int gt_clk_usb31phy_apb : 1;
        unsigned int gt_pclk_dp_sctrl : 1;
        unsigned int gt_pclk_dp_trace : 1;
        unsigned int gt_pclk_usb_sctrl : 1;
        unsigned int gt_pclk_hiusb_trace : 1;
        unsigned int gt_pclk_snpsusb_trace : 1;
    } reg;
} SOC_HSDT1_CRG_PERCLKEN0_UNION;
#endif
#define SOC_HSDT1_CRG_PERCLKEN0_gt_hclk_sd_START (0)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_hclk_sd_END (0)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_hsdt1_sysctrl_START (1)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_hsdt1_sysctrl_END (1)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_usb2phy_START (2)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_usb2phy_END (2)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_dpctrl_START (3)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_dpctrl_END (3)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_hsdt1_ioc_START (4)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_hsdt1_ioc_END (4)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_usb3otg_START (6)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_usb3otg_END (6)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_aux16_dpctrl_START (7)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_aux16_dpctrl_END (7)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_noc_usb3otg_asyncbrg_START (8)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_noc_usb3otg_asyncbrg_END (8)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_hi_usb3ctrl_ref_START (9)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_hi_usb3ctrl_ref_END (9)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_snp_usb3ctrl_ref_START (10)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_snp_usb3ctrl_ref_END (10)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_snp_usb3_suspend_START (11)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_snp_usb3_suspend_END (11)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_hi_usb3_START (12)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_hi_usb3_END (12)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_snp_usb3_START (13)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_snp_usb3_END (13)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_hi_usb3_suspend_START (14)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_hi_usb3_suspend_END (14)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_hi_usb3_sys_START (15)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_hi_usb3_sys_END (15)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_usb2phy_ref_START (18)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_usb2phy_ref_END (18)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_hsdt1_trace_START (19)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_aclk_hsdt1_trace_END (19)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_hclk_dcdr_mux_START (22)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_hclk_dcdr_mux_END (22)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_usbdp_aux_START (23)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_usbdp_aux_END (23)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_usb2_ulpi_START (25)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_usb2_ulpi_END (25)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_usb31phy_apb_START (26)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_clk_usb31phy_apb_END (26)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_dp_sctrl_START (27)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_dp_sctrl_END (27)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_dp_trace_START (28)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_dp_trace_END (28)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_usb_sctrl_START (29)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_usb_sctrl_END (29)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_hiusb_trace_START (30)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_hiusb_trace_END (30)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_snpsusb_trace_START (31)
#define SOC_HSDT1_CRG_PERCLKEN0_gt_pclk_snpsusb_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_hclk_sd : 1;
        unsigned int st_pclk_hsdt1_sysctrl : 1;
        unsigned int st_pclk_usb2phy : 1;
        unsigned int st_pclk_dpctrl : 1;
        unsigned int st_pclk_hsdt1_ioc : 1;
        unsigned int reserved_0 : 1;
        unsigned int st_aclk_usb3otg : 1;
        unsigned int st_clk_aux16_dpctrl : 1;
        unsigned int st_aclk_noc_usb3otg_asyncbrg : 1;
        unsigned int st_clk_hi_usb3ctrl_ref : 1;
        unsigned int st_clk_snp_usb3ctrl_ref : 1;
        unsigned int st_clk_snp_usb3_suspend : 1;
        unsigned int st_aclk_hi_usb3 : 1;
        unsigned int st_aclk_snp_usb3 : 1;
        unsigned int st_clk_hi_usb3_suspend : 1;
        unsigned int st_clk_hi_usb3_sys : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_clk_usb2phy_ref : 1;
        unsigned int st_aclk_hsdt1_trace : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int st_hclk_dcdr_mux : 1;
        unsigned int st_clk_usbdp_aux : 1;
        unsigned int reserved_5 : 1;
        unsigned int st_clk_usb2_ulpi : 1;
        unsigned int st_clk_usb31phy_apb : 1;
        unsigned int st_pclk_dp_sctrl : 1;
        unsigned int st_pclk_dp_trace : 1;
        unsigned int st_pclk_usb_sctrl : 1;
        unsigned int st_pclk_hiusb_trace : 1;
        unsigned int st_pclk_snpsusb_trace : 1;
    } reg;
} SOC_HSDT1_CRG_PERSTAT0_UNION;
#endif
#define SOC_HSDT1_CRG_PERSTAT0_st_hclk_sd_START (0)
#define SOC_HSDT1_CRG_PERSTAT0_st_hclk_sd_END (0)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_hsdt1_sysctrl_START (1)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_hsdt1_sysctrl_END (1)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_usb2phy_START (2)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_usb2phy_END (2)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_dpctrl_START (3)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_dpctrl_END (3)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_hsdt1_ioc_START (4)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_hsdt1_ioc_END (4)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_usb3otg_START (6)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_usb3otg_END (6)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_aux16_dpctrl_START (7)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_aux16_dpctrl_END (7)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_noc_usb3otg_asyncbrg_START (8)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_noc_usb3otg_asyncbrg_END (8)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_hi_usb3ctrl_ref_START (9)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_hi_usb3ctrl_ref_END (9)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_snp_usb3ctrl_ref_START (10)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_snp_usb3ctrl_ref_END (10)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_snp_usb3_suspend_START (11)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_snp_usb3_suspend_END (11)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_hi_usb3_START (12)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_hi_usb3_END (12)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_snp_usb3_START (13)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_snp_usb3_END (13)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_hi_usb3_suspend_START (14)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_hi_usb3_suspend_END (14)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_hi_usb3_sys_START (15)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_hi_usb3_sys_END (15)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_usb2phy_ref_START (18)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_usb2phy_ref_END (18)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_hsdt1_trace_START (19)
#define SOC_HSDT1_CRG_PERSTAT0_st_aclk_hsdt1_trace_END (19)
#define SOC_HSDT1_CRG_PERSTAT0_st_hclk_dcdr_mux_START (22)
#define SOC_HSDT1_CRG_PERSTAT0_st_hclk_dcdr_mux_END (22)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_usbdp_aux_START (23)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_usbdp_aux_END (23)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_usb2_ulpi_START (25)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_usb2_ulpi_END (25)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_usb31phy_apb_START (26)
#define SOC_HSDT1_CRG_PERSTAT0_st_clk_usb31phy_apb_END (26)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_dp_sctrl_START (27)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_dp_sctrl_END (27)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_dp_trace_START (28)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_dp_trace_END (28)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_usb_sctrl_START (29)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_usb_sctrl_END (29)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_hiusb_trace_START (30)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_hiusb_trace_END (30)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_snpsusb_trace_START (31)
#define SOC_HSDT1_CRG_PERSTAT0_st_pclk_snpsusb_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_32k : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_19p2 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_bus : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_hidif_phy_aux : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclk_hidif_phy : 1;
        unsigned int gt_clk_hidif_pixel : 1;
        unsigned int gt_clk_hidif_ctrl_ref : 1;
        unsigned int gt_clk_aux16_hidif_ctrl : 1;
        unsigned int gt_pclk_hidif_trace : 1;
        unsigned int gt_pclk_hidif_ctrl : 1;
        unsigned int gt_clk_sdpll_sscg : 1;
        unsigned int gt_clk_sd : 1;
        unsigned int gt_clk_sdpll_tp : 1;
        unsigned int gt_clk_sd_32k : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 6;
    } reg;
} SOC_HSDT1_CRG_PEREN1_UNION;
#endif
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hsdt1_usbdp_mcu_START (2)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hsdt1_usbdp_mcu_END (2)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hsdt1_usbdp_mcu_32k_START (4)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hsdt1_usbdp_mcu_32k_END (4)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hsdt1_usbdp_mcu_19p2_START (6)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hsdt1_usbdp_mcu_19p2_END (6)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hsdt1_usbdp_mcu_bus_START (8)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hsdt1_usbdp_mcu_bus_END (8)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hidif_phy_aux_START (12)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hidif_phy_aux_END (12)
#define SOC_HSDT1_CRG_PEREN1_gt_pclk_hidif_phy_START (14)
#define SOC_HSDT1_CRG_PEREN1_gt_pclk_hidif_phy_END (14)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hidif_pixel_START (15)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hidif_pixel_END (15)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hidif_ctrl_ref_START (16)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_hidif_ctrl_ref_END (16)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_aux16_hidif_ctrl_START (17)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_aux16_hidif_ctrl_END (17)
#define SOC_HSDT1_CRG_PEREN1_gt_pclk_hidif_trace_START (18)
#define SOC_HSDT1_CRG_PEREN1_gt_pclk_hidif_trace_END (18)
#define SOC_HSDT1_CRG_PEREN1_gt_pclk_hidif_ctrl_START (19)
#define SOC_HSDT1_CRG_PEREN1_gt_pclk_hidif_ctrl_END (19)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_sdpll_sscg_START (20)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_sdpll_sscg_END (20)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_sd_START (21)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_sd_END (21)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_sdpll_tp_START (22)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_sdpll_tp_END (22)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_sd_32k_START (23)
#define SOC_HSDT1_CRG_PEREN1_gt_clk_sd_32k_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_32k : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_19p2 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_bus : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_hidif_phy_aux : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclk_hidif_phy : 1;
        unsigned int gt_clk_hidif_pixel : 1;
        unsigned int gt_clk_hidif_ctrl_ref : 1;
        unsigned int gt_clk_aux16_hidif_ctrl : 1;
        unsigned int gt_pclk_hidif_trace : 1;
        unsigned int gt_pclk_hidif_ctrl : 1;
        unsigned int gt_clk_sdpll_sscg : 1;
        unsigned int gt_clk_sd : 1;
        unsigned int gt_clk_sdpll_tp : 1;
        unsigned int gt_clk_sd_32k : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 6;
    } reg;
} SOC_HSDT1_CRG_PERDIS1_UNION;
#endif
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hsdt1_usbdp_mcu_START (2)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hsdt1_usbdp_mcu_END (2)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hsdt1_usbdp_mcu_32k_START (4)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hsdt1_usbdp_mcu_32k_END (4)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hsdt1_usbdp_mcu_19p2_START (6)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hsdt1_usbdp_mcu_19p2_END (6)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hsdt1_usbdp_mcu_bus_START (8)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hsdt1_usbdp_mcu_bus_END (8)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hidif_phy_aux_START (12)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hidif_phy_aux_END (12)
#define SOC_HSDT1_CRG_PERDIS1_gt_pclk_hidif_phy_START (14)
#define SOC_HSDT1_CRG_PERDIS1_gt_pclk_hidif_phy_END (14)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hidif_pixel_START (15)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hidif_pixel_END (15)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hidif_ctrl_ref_START (16)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_hidif_ctrl_ref_END (16)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_aux16_hidif_ctrl_START (17)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_aux16_hidif_ctrl_END (17)
#define SOC_HSDT1_CRG_PERDIS1_gt_pclk_hidif_trace_START (18)
#define SOC_HSDT1_CRG_PERDIS1_gt_pclk_hidif_trace_END (18)
#define SOC_HSDT1_CRG_PERDIS1_gt_pclk_hidif_ctrl_START (19)
#define SOC_HSDT1_CRG_PERDIS1_gt_pclk_hidif_ctrl_END (19)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_sdpll_sscg_START (20)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_sdpll_sscg_END (20)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_sd_START (21)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_sd_END (21)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_sdpll_tp_START (22)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_sdpll_tp_END (22)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_sd_32k_START (23)
#define SOC_HSDT1_CRG_PERDIS1_gt_clk_sd_32k_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_32k : 1;
        unsigned int reserved_3 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_19p2 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_hsdt1_usbdp_mcu_bus : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int gt_clk_hidif_phy_aux : 1;
        unsigned int reserved_8 : 1;
        unsigned int gt_pclk_hidif_phy : 1;
        unsigned int gt_clk_hidif_pixel : 1;
        unsigned int gt_clk_hidif_ctrl_ref : 1;
        unsigned int gt_clk_aux16_hidif_ctrl : 1;
        unsigned int gt_pclk_hidif_trace : 1;
        unsigned int gt_pclk_hidif_ctrl : 1;
        unsigned int gt_clk_sdpll_sscg : 1;
        unsigned int gt_clk_sd : 1;
        unsigned int gt_clk_sdpll_tp : 1;
        unsigned int gt_clk_sd_32k : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 6;
    } reg;
} SOC_HSDT1_CRG_PERCLKEN1_UNION;
#endif
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hsdt1_usbdp_mcu_START (2)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hsdt1_usbdp_mcu_END (2)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hsdt1_usbdp_mcu_32k_START (4)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hsdt1_usbdp_mcu_32k_END (4)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hsdt1_usbdp_mcu_19p2_START (6)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hsdt1_usbdp_mcu_19p2_END (6)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hsdt1_usbdp_mcu_bus_START (8)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hsdt1_usbdp_mcu_bus_END (8)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hidif_phy_aux_START (12)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hidif_phy_aux_END (12)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_pclk_hidif_phy_START (14)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_pclk_hidif_phy_END (14)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hidif_pixel_START (15)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hidif_pixel_END (15)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hidif_ctrl_ref_START (16)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_hidif_ctrl_ref_END (16)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_aux16_hidif_ctrl_START (17)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_aux16_hidif_ctrl_END (17)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_pclk_hidif_trace_START (18)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_pclk_hidif_trace_END (18)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_pclk_hidif_ctrl_START (19)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_pclk_hidif_ctrl_END (19)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_sdpll_sscg_START (20)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_sdpll_sscg_END (20)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_sd_START (21)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_sd_END (21)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_sdpll_tp_START (22)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_sdpll_tp_END (22)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_sd_32k_START (23)
#define SOC_HSDT1_CRG_PERCLKEN1_gt_clk_sd_32k_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_clk_hsdt1_usbdp_mcu : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_clk_hsdt1_usbdp_mcu_32k : 1;
        unsigned int reserved_3 : 1;
        unsigned int st_clk_hsdt1_usbdp_mcu_19p2 : 1;
        unsigned int reserved_4 : 1;
        unsigned int st_clk_hsdt1_usbdp_mcu_bus : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int st_clk_hidif_phy_aux : 1;
        unsigned int reserved_8 : 1;
        unsigned int st_pclk_hidif_phy : 1;
        unsigned int st_clk_hidif_pixel : 1;
        unsigned int st_clk_hidif_ctrl_ref : 1;
        unsigned int st_clk_aux16_hidif_ctrl : 1;
        unsigned int st_pclk_hidif_trace : 1;
        unsigned int st_pclk_hidif_ctrl : 1;
        unsigned int st_clk_sdpll_sscg : 1;
        unsigned int st_clk_sd : 1;
        unsigned int st_clk_sdpll_tp : 1;
        unsigned int st_clk_sd_32k : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 6;
    } reg;
} SOC_HSDT1_CRG_PERSTAT1_UNION;
#endif
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hsdt1_usbdp_mcu_START (2)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hsdt1_usbdp_mcu_END (2)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hsdt1_usbdp_mcu_32k_START (4)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hsdt1_usbdp_mcu_32k_END (4)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hsdt1_usbdp_mcu_19p2_START (6)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hsdt1_usbdp_mcu_19p2_END (6)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hsdt1_usbdp_mcu_bus_START (8)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hsdt1_usbdp_mcu_bus_END (8)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hidif_phy_aux_START (12)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hidif_phy_aux_END (12)
#define SOC_HSDT1_CRG_PERSTAT1_st_pclk_hidif_phy_START (14)
#define SOC_HSDT1_CRG_PERSTAT1_st_pclk_hidif_phy_END (14)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hidif_pixel_START (15)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hidif_pixel_END (15)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hidif_ctrl_ref_START (16)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_hidif_ctrl_ref_END (16)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_aux16_hidif_ctrl_START (17)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_aux16_hidif_ctrl_END (17)
#define SOC_HSDT1_CRG_PERSTAT1_st_pclk_hidif_trace_START (18)
#define SOC_HSDT1_CRG_PERSTAT1_st_pclk_hidif_trace_END (18)
#define SOC_HSDT1_CRG_PERSTAT1_st_pclk_hidif_ctrl_START (19)
#define SOC_HSDT1_CRG_PERSTAT1_st_pclk_hidif_ctrl_END (19)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_sdpll_sscg_START (20)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_sdpll_sscg_END (20)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_sd_START (21)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_sd_END (21)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_sdpll_tp_START (22)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_sdpll_tp_END (22)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_sd_32k_START (23)
#define SOC_HSDT1_CRG_PERSTAT1_st_clk_sd_32k_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_hrst_sd : 1;
        unsigned int ip_rst_sd : 1;
        unsigned int ip_prst_hsdt1_sysctrl : 1;
        unsigned int ip_rst_dpctrl : 1;
        unsigned int ip_prst_hsdt1_ioc : 1;
        unsigned int ip_prst_usb31phy_apb : 1;
        unsigned int ip_prst_hidif_phy : 1;
        unsigned int ip_rst_usb2phy_por : 1;
        unsigned int ip_rst_usb_ulpi : 1;
        unsigned int ip_rst_usb_utmi : 1;
        unsigned int ip_rst_usb_pipe : 1;
        unsigned int ip_rst_hi_usb3_sys : 1;
        unsigned int ip_rst_hsdt1_trace : 1;
        unsigned int ip_arst_snp_usb3 : 1;
        unsigned int ip_arst_hi_usb3 : 1;
        unsigned int ip_prst_usb2phy : 1;
        unsigned int ip_arst_usb3otg : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu : 1;
        unsigned int ip_rst_snp_usb3_vcc : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu_32k : 1;
        unsigned int ip_rst_combophy : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu_19p2 : 1;
        unsigned int ip_rst_combophy_por : 1;
        unsigned int reserved : 1;
        unsigned int ip_rst_hidif_pixel : 1;
        unsigned int ip_prst_dp_sctrl : 1;
        unsigned int ip_prst_dp_trace : 1;
        unsigned int ip_prst_hidif_ctrl : 1;
        unsigned int ip_prst_hidif_trace : 1;
        unsigned int ip_prst_usb_sctrl : 1;
        unsigned int ip_prst_hiusb_trace : 1;
        unsigned int ip_prst_snpsusb_trace : 1;
    } reg;
} SOC_HSDT1_CRG_PERRSTEN0_UNION;
#endif
#define SOC_HSDT1_CRG_PERRSTEN0_ip_hrst_sd_START (0)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_hrst_sd_END (0)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_sd_START (1)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_sd_END (1)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hsdt1_sysctrl_START (2)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hsdt1_sysctrl_END (2)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_dpctrl_START (3)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_dpctrl_END (3)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hsdt1_ioc_START (4)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hsdt1_ioc_END (4)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_usb31phy_apb_START (5)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_usb31phy_apb_END (5)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hidif_phy_START (6)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hidif_phy_END (6)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_usb2phy_por_START (7)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_usb2phy_por_END (7)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_usb_ulpi_START (8)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_usb_ulpi_END (8)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_usb_utmi_START (9)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_usb_utmi_END (9)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_usb_pipe_START (10)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_usb_pipe_END (10)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hi_usb3_sys_START (11)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hi_usb3_sys_END (11)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hsdt1_trace_START (12)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hsdt1_trace_END (12)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_arst_snp_usb3_START (13)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_arst_snp_usb3_END (13)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_arst_hi_usb3_START (14)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_arst_hi_usb3_END (14)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_usb2phy_START (15)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_usb2phy_END (15)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_arst_usb3otg_START (16)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_arst_usb3otg_END (16)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hsdt1_usbdp_mcu_START (17)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hsdt1_usbdp_mcu_END (17)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_snp_usb3_vcc_START (18)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_snp_usb3_vcc_END (18)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hsdt1_usbdp_mcu_32k_START (19)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hsdt1_usbdp_mcu_32k_END (19)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_combophy_START (20)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_combophy_END (20)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hsdt1_usbdp_mcu_19p2_START (21)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hsdt1_usbdp_mcu_19p2_END (21)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_combophy_por_START (22)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_combophy_por_END (22)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hidif_pixel_START (24)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_rst_hidif_pixel_END (24)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_dp_sctrl_START (25)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_dp_sctrl_END (25)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_dp_trace_START (26)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_dp_trace_END (26)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hidif_ctrl_START (27)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hidif_ctrl_END (27)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hidif_trace_START (28)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hidif_trace_END (28)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_usb_sctrl_START (29)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_usb_sctrl_END (29)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hiusb_trace_START (30)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_hiusb_trace_END (30)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_snpsusb_trace_START (31)
#define SOC_HSDT1_CRG_PERRSTEN0_ip_prst_snpsusb_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_hrst_sd : 1;
        unsigned int ip_rst_sd : 1;
        unsigned int ip_prst_hsdt1_sysctrl : 1;
        unsigned int ip_rst_dpctrl : 1;
        unsigned int ip_prst_hsdt1_ioc : 1;
        unsigned int ip_prst_usb31phy_apb : 1;
        unsigned int ip_prst_hidif_phy : 1;
        unsigned int ip_rst_usb2phy_por : 1;
        unsigned int ip_rst_usb_ulpi : 1;
        unsigned int ip_rst_usb_utmi : 1;
        unsigned int ip_rst_usb_pipe : 1;
        unsigned int ip_rst_hi_usb3_sys : 1;
        unsigned int ip_rst_hsdt1_trace : 1;
        unsigned int ip_arst_snp_usb3 : 1;
        unsigned int ip_arst_hi_usb3 : 1;
        unsigned int ip_prst_usb2phy : 1;
        unsigned int ip_arst_usb3otg : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu : 1;
        unsigned int ip_rst_snp_usb3_vcc : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu_32k : 1;
        unsigned int ip_rst_combophy : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu_19p2 : 1;
        unsigned int ip_rst_combophy_por : 1;
        unsigned int reserved : 1;
        unsigned int ip_rst_hidif_pixel : 1;
        unsigned int ip_prst_dp_sctrl : 1;
        unsigned int ip_prst_dp_trace : 1;
        unsigned int ip_prst_hidif_ctrl : 1;
        unsigned int ip_prst_hidif_trace : 1;
        unsigned int ip_prst_usb_sctrl : 1;
        unsigned int ip_prst_hiusb_trace : 1;
        unsigned int ip_prst_snpsusb_trace : 1;
    } reg;
} SOC_HSDT1_CRG_PERRSTDIS0_UNION;
#endif
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_hrst_sd_START (0)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_hrst_sd_END (0)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_sd_START (1)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_sd_END (1)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hsdt1_sysctrl_START (2)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hsdt1_sysctrl_END (2)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_dpctrl_START (3)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_dpctrl_END (3)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hsdt1_ioc_START (4)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hsdt1_ioc_END (4)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_usb31phy_apb_START (5)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_usb31phy_apb_END (5)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hidif_phy_START (6)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hidif_phy_END (6)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_usb2phy_por_START (7)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_usb2phy_por_END (7)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_usb_ulpi_START (8)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_usb_ulpi_END (8)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_usb_utmi_START (9)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_usb_utmi_END (9)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_usb_pipe_START (10)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_usb_pipe_END (10)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hi_usb3_sys_START (11)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hi_usb3_sys_END (11)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hsdt1_trace_START (12)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hsdt1_trace_END (12)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_arst_snp_usb3_START (13)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_arst_snp_usb3_END (13)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_arst_hi_usb3_START (14)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_arst_hi_usb3_END (14)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_usb2phy_START (15)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_usb2phy_END (15)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_arst_usb3otg_START (16)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_arst_usb3otg_END (16)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hsdt1_usbdp_mcu_START (17)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hsdt1_usbdp_mcu_END (17)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_snp_usb3_vcc_START (18)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_snp_usb3_vcc_END (18)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hsdt1_usbdp_mcu_32k_START (19)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hsdt1_usbdp_mcu_32k_END (19)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_combophy_START (20)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_combophy_END (20)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hsdt1_usbdp_mcu_19p2_START (21)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hsdt1_usbdp_mcu_19p2_END (21)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_combophy_por_START (22)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_combophy_por_END (22)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hidif_pixel_START (24)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_rst_hidif_pixel_END (24)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_dp_sctrl_START (25)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_dp_sctrl_END (25)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_dp_trace_START (26)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_dp_trace_END (26)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hidif_ctrl_START (27)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hidif_ctrl_END (27)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hidif_trace_START (28)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hidif_trace_END (28)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_usb_sctrl_START (29)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_usb_sctrl_END (29)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hiusb_trace_START (30)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_hiusb_trace_END (30)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_snpsusb_trace_START (31)
#define SOC_HSDT1_CRG_PERRSTDIS0_ip_prst_snpsusb_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_hrst_sd : 1;
        unsigned int ip_rst_sd : 1;
        unsigned int ip_prst_hsdt1_sysctrl : 1;
        unsigned int ip_rst_dpctrl : 1;
        unsigned int ip_prst_hsdt1_ioc : 1;
        unsigned int ip_prst_usb31phy_apb : 1;
        unsigned int ip_prst_hidif_phy : 1;
        unsigned int ip_rst_usb2phy_por : 1;
        unsigned int ip_rst_usb_ulpi : 1;
        unsigned int ip_rst_usb_utmi : 1;
        unsigned int ip_rst_usb_pipe : 1;
        unsigned int ip_rst_hi_usb3_sys : 1;
        unsigned int ip_rst_hsdt1_trace : 1;
        unsigned int ip_arst_snp_usb3 : 1;
        unsigned int ip_arst_hi_usb3 : 1;
        unsigned int ip_prst_usb2phy : 1;
        unsigned int ip_arst_usb3otg : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu : 1;
        unsigned int ip_rst_snp_usb3_vcc : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu_32k : 1;
        unsigned int ip_rst_combophy : 1;
        unsigned int ip_rst_hsdt1_usbdp_mcu_19p2 : 1;
        unsigned int ip_rst_combophy_por : 1;
        unsigned int reserved : 1;
        unsigned int ip_rst_hidif_pixel : 1;
        unsigned int ip_prst_dp_sctrl : 1;
        unsigned int ip_prst_dp_trace : 1;
        unsigned int ip_prst_hidif_ctrl : 1;
        unsigned int ip_prst_hidif_trace : 1;
        unsigned int ip_prst_usb_sctrl : 1;
        unsigned int ip_prst_hiusb_trace : 1;
        unsigned int ip_prst_snpsusb_trace : 1;
    } reg;
} SOC_HSDT1_CRG_PERRSTSTAT0_UNION;
#endif
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_hrst_sd_START (0)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_hrst_sd_END (0)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_sd_START (1)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_sd_END (1)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hsdt1_sysctrl_START (2)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hsdt1_sysctrl_END (2)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_dpctrl_START (3)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_dpctrl_END (3)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hsdt1_ioc_START (4)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hsdt1_ioc_END (4)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_usb31phy_apb_START (5)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_usb31phy_apb_END (5)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hidif_phy_START (6)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hidif_phy_END (6)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_usb2phy_por_START (7)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_usb2phy_por_END (7)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_usb_ulpi_START (8)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_usb_ulpi_END (8)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_usb_utmi_START (9)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_usb_utmi_END (9)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_usb_pipe_START (10)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_usb_pipe_END (10)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hi_usb3_sys_START (11)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hi_usb3_sys_END (11)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hsdt1_trace_START (12)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hsdt1_trace_END (12)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_arst_snp_usb3_START (13)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_arst_snp_usb3_END (13)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_arst_hi_usb3_START (14)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_arst_hi_usb3_END (14)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_usb2phy_START (15)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_usb2phy_END (15)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_arst_usb3otg_START (16)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_arst_usb3otg_END (16)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hsdt1_usbdp_mcu_START (17)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hsdt1_usbdp_mcu_END (17)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_snp_usb3_vcc_START (18)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_snp_usb3_vcc_END (18)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hsdt1_usbdp_mcu_32k_START (19)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hsdt1_usbdp_mcu_32k_END (19)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_combophy_START (20)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_combophy_END (20)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hsdt1_usbdp_mcu_19p2_START (21)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hsdt1_usbdp_mcu_19p2_END (21)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_combophy_por_START (22)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_combophy_por_END (22)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hidif_pixel_START (24)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_rst_hidif_pixel_END (24)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_dp_sctrl_START (25)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_dp_sctrl_END (25)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_dp_trace_START (26)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_dp_trace_END (26)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hidif_ctrl_START (27)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hidif_ctrl_END (27)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hidif_trace_START (28)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hidif_trace_END (28)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_usb_sctrl_START (29)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_usb_sctrl_END (29)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hiusb_trace_START (30)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_hiusb_trace_END (30)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_snpsusb_trace_START (31)
#define SOC_HSDT1_CRG_PERRSTSTAT0_ip_prst_snpsusb_trace_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_hidif_phy : 1;
        unsigned int ip_rst_hidif_phy_por : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 28;
    } reg;
} SOC_HSDT1_CRG_PERRSTEN1_UNION;
#endif
#define SOC_HSDT1_CRG_PERRSTEN1_ip_rst_hidif_phy_START (0)
#define SOC_HSDT1_CRG_PERRSTEN1_ip_rst_hidif_phy_END (0)
#define SOC_HSDT1_CRG_PERRSTEN1_ip_rst_hidif_phy_por_START (1)
#define SOC_HSDT1_CRG_PERRSTEN1_ip_rst_hidif_phy_por_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_hidif_phy : 1;
        unsigned int ip_rst_hidif_phy_por : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 28;
    } reg;
} SOC_HSDT1_CRG_PERRSTDIS1_UNION;
#endif
#define SOC_HSDT1_CRG_PERRSTDIS1_ip_rst_hidif_phy_START (0)
#define SOC_HSDT1_CRG_PERRSTDIS1_ip_rst_hidif_phy_END (0)
#define SOC_HSDT1_CRG_PERRSTDIS1_ip_rst_hidif_phy_por_START (1)
#define SOC_HSDT1_CRG_PERRSTDIS1_ip_rst_hidif_phy_por_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_hidif_phy : 1;
        unsigned int ip_rst_hidif_phy_por : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 28;
    } reg;
} SOC_HSDT1_CRG_PERRSTSTAT1_UNION;
#endif
#define SOC_HSDT1_CRG_PERRSTSTAT1_ip_rst_hidif_phy_START (0)
#define SOC_HSDT1_CRG_PERRSTSTAT1_ip_rst_hidif_phy_END (0)
#define SOC_HSDT1_CRG_PERRSTSTAT1_ip_rst_hidif_phy_por_START (1)
#define SOC_HSDT1_CRG_PERRSTSTAT1_ip_rst_hidif_phy_por_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int sel_usb2phy_ref : 1;
        unsigned int sc_gt_clk_usb2phy_ref : 1;
        unsigned int reserved_1 : 1;
        unsigned int sc_gt_clk_usb2_ulpi : 1;
        unsigned int div_clk_usb2_ulpi : 4;
        unsigned int reserved_2 : 2;
        unsigned int reserved_3 : 2;
        unsigned int sel_hidif_pixel : 3;
        unsigned int bitmasken : 16;
    } reg;
} SOC_HSDT1_CRG_CLKDIV0_UNION;
#endif
#define SOC_HSDT1_CRG_CLKDIV0_sel_usb2phy_ref_START (1)
#define SOC_HSDT1_CRG_CLKDIV0_sel_usb2phy_ref_END (1)
#define SOC_HSDT1_CRG_CLKDIV0_sc_gt_clk_usb2phy_ref_START (2)
#define SOC_HSDT1_CRG_CLKDIV0_sc_gt_clk_usb2phy_ref_END (2)
#define SOC_HSDT1_CRG_CLKDIV0_sc_gt_clk_usb2_ulpi_START (4)
#define SOC_HSDT1_CRG_CLKDIV0_sc_gt_clk_usb2_ulpi_END (4)
#define SOC_HSDT1_CRG_CLKDIV0_div_clk_usb2_ulpi_START (5)
#define SOC_HSDT1_CRG_CLKDIV0_div_clk_usb2_ulpi_END (8)
#define SOC_HSDT1_CRG_CLKDIV0_sel_hidif_pixel_START (13)
#define SOC_HSDT1_CRG_CLKDIV0_sel_hidif_pixel_END (15)
#define SOC_HSDT1_CRG_CLKDIV0_bitmasken_START (16)
#define SOC_HSDT1_CRG_CLKDIV0_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int div_sd : 4;
        unsigned int sel_sd_pll : 2;
        unsigned int div_clk_edp_phy : 2;
        unsigned int reserved_1 : 1;
        unsigned int div_clk_sd_pixel : 6;
        unsigned int bitmasken : 16;
    } reg;
} SOC_HSDT1_CRG_CLKDIV1_UNION;
#endif
#define SOC_HSDT1_CRG_CLKDIV1_div_sd_START (1)
#define SOC_HSDT1_CRG_CLKDIV1_div_sd_END (4)
#define SOC_HSDT1_CRG_CLKDIV1_sel_sd_pll_START (5)
#define SOC_HSDT1_CRG_CLKDIV1_sel_sd_pll_END (6)
#define SOC_HSDT1_CRG_CLKDIV1_div_clk_edp_phy_START (7)
#define SOC_HSDT1_CRG_CLKDIV1_div_clk_edp_phy_END (8)
#define SOC_HSDT1_CRG_CLKDIV1_div_clk_sd_pixel_START (10)
#define SOC_HSDT1_CRG_CLKDIV1_div_clk_sd_pixel_END (15)
#define SOC_HSDT1_CRG_CLKDIV1_bitmasken_START (16)
#define SOC_HSDT1_CRG_CLKDIV1_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_gt_clk_sd_sys : 1;
        unsigned int sc_gt_clk_sd : 1;
        unsigned int sc_gt_clk_suspend_div : 1;
        unsigned int sc_gt_clk_edp_phy_div : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 4;
        unsigned int sc_gt_clk_sd_pixel_div : 1;
        unsigned int reserved_2 : 3;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int bitmasken : 16;
    } reg;
} SOC_HSDT1_CRG_CLKDIV2_UNION;
#endif
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_sd_sys_START (0)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_sd_sys_END (0)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_sd_START (1)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_sd_END (1)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_suspend_div_START (2)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_suspend_div_END (2)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_edp_phy_div_START (3)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_edp_phy_div_END (3)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_sd_pixel_div_START (10)
#define SOC_HSDT1_CRG_CLKDIV2_sc_gt_clk_sd_pixel_div_END (10)
#define SOC_HSDT1_CRG_CLKDIV2_bitmasken_START (16)
#define SOC_HSDT1_CRG_CLKDIV2_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 2;
        unsigned int sw_ack_clk_hsdt1_usbdp_sw : 2;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 26;
    } reg;
} SOC_HSDT1_CRG_PERI_STAT0_UNION;
#endif
#define SOC_HSDT1_CRG_PERI_STAT0_sw_ack_clk_hsdt1_usbdp_sw_START (2)
#define SOC_HSDT1_CRG_PERI_STAT0_sw_ack_clk_hsdt1_usbdp_sw_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int swdone_clk_sd_div : 1;
        unsigned int sdpll_sscg_idle : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 29;
    } reg;
} SOC_HSDT1_CRG_PERI_STAT1_UNION;
#endif
#define SOC_HSDT1_CRG_PERI_STAT1_swdone_clk_sd_div_START (0)
#define SOC_HSDT1_CRG_PERI_STAT1_swdone_clk_sd_div_END (0)
#define SOC_HSDT1_CRG_PERI_STAT1_sdpll_sscg_idle_START (1)
#define SOC_HSDT1_CRG_PERI_STAT1_sdpll_sscg_idle_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sd_clkrst_bypass : 1;
        unsigned int dpctrl_clkrst_bypass : 1;
        unsigned int hsdt1_sysctrl_clkrst_bypass : 1;
        unsigned int hsdt1_ioc_clkrst_bypass : 1;
        unsigned int usbctrl_clkrst_bypass : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int hsdt1_usbdp_mcu_bus_clkrst_bypass : 1;
        unsigned int reserved_7 : 1;
        unsigned int hsdt1_usb31phy_apb_clkrst_bypass : 1;
        unsigned int dp_sctrl_clkrst_bypass : 1;
        unsigned int dp_trace_clkrst_bypass : 1;
        unsigned int hidif_trace_clkrst_bypass : 1;
        unsigned int hidif_ctrl_clkrst_bypass : 1;
        unsigned int usb_sctrl_clkrst_bypass : 1;
        unsigned int hiusb_trace_clkrst_bypass : 1;
        unsigned int snpsusb_trace_clkrst_bypass : 1;
        unsigned int hidif_phy_clkrst_bypass : 1;
        unsigned int usb2phy_clkrst_bypass : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_11 : 1;
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
    } reg;
} SOC_HSDT1_CRG_IPCLKRST_BYPASS0_UNION;
#endif
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_sd_clkrst_bypass_START (0)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_sd_clkrst_bypass_END (0)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_dpctrl_clkrst_bypass_START (1)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_dpctrl_clkrst_bypass_END (1)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hsdt1_sysctrl_clkrst_bypass_START (2)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hsdt1_sysctrl_clkrst_bypass_END (2)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hsdt1_ioc_clkrst_bypass_START (3)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hsdt1_ioc_clkrst_bypass_END (3)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_usbctrl_clkrst_bypass_START (4)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_usbctrl_clkrst_bypass_END (4)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hsdt1_usbdp_mcu_bus_clkrst_bypass_START (12)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hsdt1_usbdp_mcu_bus_clkrst_bypass_END (12)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hsdt1_usb31phy_apb_clkrst_bypass_START (14)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hsdt1_usb31phy_apb_clkrst_bypass_END (14)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_dp_sctrl_clkrst_bypass_START (15)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_dp_sctrl_clkrst_bypass_END (15)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_dp_trace_clkrst_bypass_START (16)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_dp_trace_clkrst_bypass_END (16)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hidif_trace_clkrst_bypass_START (17)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hidif_trace_clkrst_bypass_END (17)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hidif_ctrl_clkrst_bypass_START (18)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hidif_ctrl_clkrst_bypass_END (18)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_usb_sctrl_clkrst_bypass_START (19)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_usb_sctrl_clkrst_bypass_END (19)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hiusb_trace_clkrst_bypass_START (20)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hiusb_trace_clkrst_bypass_END (20)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_snpsusb_trace_clkrst_bypass_START (21)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_snpsusb_trace_clkrst_bypass_END (21)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hidif_phy_clkrst_bypass_START (22)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_hidif_phy_clkrst_bypass_END (22)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_usb2phy_clkrst_bypass_START (23)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS0_usb2phy_clkrst_bypass_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int qic_hsdt1bus_clkrst_bypass : 1;
        unsigned int qic_hsdt12usb_clkrst_bypass : 1;
        unsigned int qic_hsdt1bus2sysbus_clkrst_bypass : 1;
        unsigned int qic_usb_clkrst_bypass : 1;
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
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int reserved_26 : 1;
        unsigned int reserved_27 : 1;
    } reg;
} SOC_HSDT1_CRG_IPCLKRST_BYPASS1_UNION;
#endif
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_qic_hsdt1bus_clkrst_bypass_START (0)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_qic_hsdt1bus_clkrst_bypass_END (0)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_qic_hsdt12usb_clkrst_bypass_START (1)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_qic_hsdt12usb_clkrst_bypass_END (1)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_qic_hsdt1bus2sysbus_clkrst_bypass_START (2)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_qic_hsdt1bus2sysbus_clkrst_bypass_END (2)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_qic_usb_clkrst_bypass_START (3)
#define SOC_HSDT1_CRG_IPCLKRST_BYPASS1_qic_usb_clkrst_bypass_END (3)
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
} SOC_HSDT1_CRG_SDPLLSSCCTRL_UNION;
#endif
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_enable_START (0)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_enable_END (0)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_mode_START (1)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_mode_END (1)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_rate_START (2)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_rate_END (5)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_depth_START (6)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_depth_END (8)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_fast_dis_START (9)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_sscg_fast_dis_END (9)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_bitmasken_START (16)
#define SOC_HSDT1_CRG_SDPLLSSCCTRL_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sdpll_en : 1;
        unsigned int sdpll_bp : 1;
        unsigned int sdpll_refdiv : 6;
        unsigned int sdpll_fbdiv : 12;
        unsigned int sdpll_postdiv1 : 3;
        unsigned int sdpll_postdiv2 : 3;
        unsigned int sdpll_lock : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_HSDT1_CRG_SDPLLCTRL0_UNION;
#endif
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_en_START (0)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_en_END (0)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_bp_START (1)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_bp_END (1)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_refdiv_START (2)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_refdiv_END (7)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_fbdiv_START (8)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_fbdiv_END (19)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_postdiv1_START (20)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_postdiv1_END (22)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_postdiv2_START (23)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_postdiv2_END (25)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_lock_START (26)
#define SOC_HSDT1_CRG_SDPLLCTRL0_sdpll_lock_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sdpll_fracdiv : 24;
        unsigned int sdpll_int_mod : 1;
        unsigned int sdpll_cfg_vld : 1;
        unsigned int gt_clk_sdpll : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_HSDT1_CRG_SDPLLCTRL1_UNION;
#endif
#define SOC_HSDT1_CRG_SDPLLCTRL1_sdpll_fracdiv_START (0)
#define SOC_HSDT1_CRG_SDPLLCTRL1_sdpll_fracdiv_END (23)
#define SOC_HSDT1_CRG_SDPLLCTRL1_sdpll_int_mod_START (24)
#define SOC_HSDT1_CRG_SDPLLCTRL1_sdpll_int_mod_END (24)
#define SOC_HSDT1_CRG_SDPLLCTRL1_sdpll_cfg_vld_START (25)
#define SOC_HSDT1_CRG_SDPLLCTRL1_sdpll_cfg_vld_END (25)
#define SOC_HSDT1_CRG_SDPLLCTRL1_gt_clk_sdpll_START (26)
#define SOC_HSDT1_CRG_SDPLLCTRL1_gt_clk_sdpll_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int sdpll_lock : 1;
        unsigned int sdpll_en_stat : 1;
        unsigned int sdpll_bypass_stat : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 24;
    } reg;
} SOC_HSDT1_CRG_PCIEPLL_STAT_UNION;
#endif
#define SOC_HSDT1_CRG_PCIEPLL_STAT_sdpll_lock_START (4)
#define SOC_HSDT1_CRG_PCIEPLL_STAT_sdpll_lock_END (4)
#define SOC_HSDT1_CRG_PCIEPLL_STAT_sdpll_en_stat_START (5)
#define SOC_HSDT1_CRG_PCIEPLL_STAT_sdpll_en_stat_END (5)
#define SOC_HSDT1_CRG_PCIEPLL_STAT_sdpll_bypass_stat_START (6)
#define SOC_HSDT1_CRG_PCIEPLL_STAT_sdpll_bypass_stat_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int FNPLL_CFG0 : 32;
    } reg;
} SOC_HSDT1_CRG_FNPLL_CFG0_UNION;
#endif
#define SOC_HSDT1_CRG_FNPLL_CFG0_FNPLL_CFG0_START (0)
#define SOC_HSDT1_CRG_FNPLL_CFG0_FNPLL_CFG0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int FNPLL_CFG1 : 32;
    } reg;
} SOC_HSDT1_CRG_FNPLL_CFG1_UNION;
#endif
#define SOC_HSDT1_CRG_FNPLL_CFG1_FNPLL_CFG1_START (0)
#define SOC_HSDT1_CRG_FNPLL_CFG1_FNPLL_CFG1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int FNPLL_CFG2 : 32;
    } reg;
} SOC_HSDT1_CRG_FNPLL_CFG2_UNION;
#endif
#define SOC_HSDT1_CRG_FNPLL_CFG2_FNPLL_CFG2_START (0)
#define SOC_HSDT1_CRG_FNPLL_CFG2_FNPLL_CFG2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int FNPLL_CFG3 : 32;
    } reg;
} SOC_HSDT1_CRG_FNPLL_CFG3_UNION;
#endif
#define SOC_HSDT1_CRG_FNPLL_CFG3_FNPLL_CFG3_START (0)
#define SOC_HSDT1_CRG_FNPLL_CFG3_FNPLL_CFG3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sysbus_usb3_bypass : 1;
        unsigned int sysbus_sd_bypass : 1;
        unsigned int sysbus_hsdt1_t_bypass : 1;
        unsigned int sysbus_hsdt1_pwr_bypass : 1;
        unsigned int sysbus_qcr_dcdr_hsdt1bus_bp : 1;
        unsigned int sysbus_sdio_bypass : 1;
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
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
    } reg;
} SOC_HSDT1_CRG_PERI_AUTODIV0_UNION;
#endif
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_usb3_bypass_START (0)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_usb3_bypass_END (0)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_sd_bypass_START (1)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_sd_bypass_END (1)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_hsdt1_t_bypass_START (2)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_hsdt1_t_bypass_END (2)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_hsdt1_pwr_bypass_START (3)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_hsdt1_pwr_bypass_END (3)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_qcr_dcdr_hsdt1bus_bp_START (4)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_qcr_dcdr_hsdt1bus_bp_END (4)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_sdio_bypass_START (5)
#define SOC_HSDT1_CRG_PERI_AUTODIV0_sysbus_sdio_bypass_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int usbpcie_refclk_iso_en : 1;
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
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int reserved_26 : 1;
        unsigned int reserved_27 : 1;
        unsigned int reserved_28 : 1;
        unsigned int reserved_29 : 1;
        unsigned int reserved_30 : 1;
    } reg;
} SOC_HSDT1_CRG_ISOEN_UNION;
#endif
#define SOC_HSDT1_CRG_ISOEN_usbpcie_refclk_iso_en_START (0)
#define SOC_HSDT1_CRG_ISOEN_usbpcie_refclk_iso_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int usbpcie_refclk_iso_en : 1;
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
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int reserved_26 : 1;
        unsigned int reserved_27 : 1;
        unsigned int reserved_28 : 1;
        unsigned int reserved_29 : 1;
        unsigned int reserved_30 : 1;
    } reg;
} SOC_HSDT1_CRG_ISODIS_UNION;
#endif
#define SOC_HSDT1_CRG_ISODIS_usbpcie_refclk_iso_en_START (0)
#define SOC_HSDT1_CRG_ISODIS_usbpcie_refclk_iso_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int usbpcie_refclk_iso_en : 1;
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
        unsigned int reserved_12 : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_14 : 1;
        unsigned int reserved_15 : 1;
        unsigned int reserved_16 : 1;
        unsigned int reserved_17 : 1;
        unsigned int reserved_18 : 1;
        unsigned int reserved_19 : 1;
        unsigned int reserved_20 : 1;
        unsigned int reserved_21 : 1;
        unsigned int reserved_22 : 1;
        unsigned int reserved_23 : 1;
        unsigned int reserved_24 : 1;
        unsigned int reserved_25 : 1;
        unsigned int reserved_26 : 1;
        unsigned int reserved_27 : 1;
        unsigned int reserved_28 : 1;
        unsigned int reserved_29 : 1;
        unsigned int reserved_30 : 1;
    } reg;
} SOC_HSDT1_CRG_ISOSTAT_UNION;
#endif
#define SOC_HSDT1_CRG_ISOSTAT_usbpcie_refclk_iso_en_START (0)
#define SOC_HSDT1_CRG_ISOSTAT_usbpcie_refclk_iso_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int FNPLL_STATE0 : 32;
    } reg;
} SOC_HSDT1_CRG_FNPLL_STAT0_SD_UNION;
#endif
#define SOC_HSDT1_CRG_FNPLL_STAT0_SD_FNPLL_STATE0_START (0)
#define SOC_HSDT1_CRG_FNPLL_STAT0_SD_FNPLL_STATE0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_mask_usb_nonidle_pend : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 14;
        unsigned int bitmasken : 16;
    } reg;
} SOC_HSDT1_CRG_INTR_MASK_NOCBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_HSDT1_CRG_INTR_MASK_NOCBUS_NONIDLE_PEND_intr_mask_usb_nonidle_pend_START (0)
#define SOC_HSDT1_CRG_INTR_MASK_NOCBUS_NONIDLE_PEND_intr_mask_usb_nonidle_pend_END (0)
#define SOC_HSDT1_CRG_INTR_MASK_NOCBUS_NONIDLE_PEND_bitmasken_START (16)
#define SOC_HSDT1_CRG_INTR_MASK_NOCBUS_NONIDLE_PEND_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_clr_usb_nonidle_pend : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 14;
        unsigned int bitmasken : 16;
    } reg;
} SOC_HSDT1_CRG_INTR_CLR_NOCBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_HSDT1_CRG_INTR_CLR_NOCBUS_NONIDLE_PEND_intr_clr_usb_nonidle_pend_START (0)
#define SOC_HSDT1_CRG_INTR_CLR_NOCBUS_NONIDLE_PEND_intr_clr_usb_nonidle_pend_END (0)
#define SOC_HSDT1_CRG_INTR_CLR_NOCBUS_NONIDLE_PEND_bitmasken_START (16)
#define SOC_HSDT1_CRG_INTR_CLR_NOCBUS_NONIDLE_PEND_bitmasken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_stat_usb_nonidle_pend : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 30;
    } reg;
} SOC_HSDT1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_UNION;
#endif
#define SOC_HSDT1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_usb_nonidle_pend_START (0)
#define SOC_HSDT1_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_intr_stat_usb_nonidle_pend_END (0)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
