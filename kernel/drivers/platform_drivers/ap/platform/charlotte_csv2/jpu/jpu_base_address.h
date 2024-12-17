#ifndef _HI_JPU_BASE_ADDR_INCLUDE_H_
#define _HI_JPU_BASE_ADDR_INCLUDE_H_ 
#ifndef __SOC_H_FOR_ASM__
#define SOC_SCTRL_SCPERDIS4_ADDR(base) ((base) + (0x1B4UL))
#define SOC_PMCTRL_PERI_CTRL0_ADDR(base) ((base) + (0x340UL))
#define SOC_PMCTRL_PERI_CTRL0_ADDR(base) ((base) + (0x340UL))
#else
#define SOC_SCTRL_SCPERDIS4_ADDR(base) ((base) + (0x1B4))
#define SOC_PMCTRL_PERI_CTRL0_ADDR(base) ((base) + (0x340))
#define SOC_PMCTRL_PERI_CTRL0_ADDR(base) ((base) + (0x340))
#endif
#endif
