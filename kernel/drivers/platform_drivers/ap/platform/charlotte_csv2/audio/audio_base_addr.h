#ifndef _HI_AUDIO_BASE_ADDR_INCLUDE_H_
#define _HI_AUDIO_BASE_ADDR_INCLUDE_H_ 
#define RESERVED_HIFI_PHYMEM_BASE 0x2DA00000
#define RESERVED_HIFI_PHYMEM_SIZE (0xF80000)
#define RESERVED_HIFI_DATA_PHYMEM_BASE 0x2F080000
#define RESERVED_HIFI_DATA_PHYMEM_SIZE (0x580000)
#define RESERVED_FASTBOOT_CMA_PHYMEM_BASE 0x3B400000
#ifdef CONFIG_AP_CUST_PAD_MEM
#define RESERVED_S4_NS_BASE 0x30740000
#define RESERVED_S4_NS_SIZE (0x180000)
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SCTRL_SCPERCLKEN0_ADDR(base) ((base) + (0x168UL))
#define SOC_SCTRL_SCPERSTAT0_ADDR(base) ((base) + (0x16CUL))
#define SOC_SCTRL_SCPERCLKEN1_ADDR(base) ((base) + (0x178UL))
#else
#define SOC_SCTRL_SCPERCLKEN0_ADDR(base) ((base) + (0x168))
#define SOC_SCTRL_SCPERSTAT0_ADDR(base) ((base) + (0x16C))
#define SOC_SCTRL_SCPERCLKEN1_ADDR(base) ((base) + (0x178))
#endif
#define SOC_SCTRL_SCPEREN0_gt_clk_mad_acpu_START (17)
#define SOC_SCTRL_SCPERSTAT0_st_clk_asp_subsys_START (26)
#define SOC_SCTRL_SCPERCLKEN1_gt_clk_asp_subsys_acpu_START (4)
#endif
