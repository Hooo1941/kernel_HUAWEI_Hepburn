#ifndef __SOC_NPU_TS_SYSCTRL_INTERFACE_H__
#define __SOC_NPU_TS_SYSCTRL_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT0_ADDR(base) ((base) + (0x0008UL))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_ADDR(base) ((base) + (0x000CUL))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_ADDR(base) ((base) + (0x0010UL))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_ADDR(base) ((base) + (0x0014UL))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT4_ADDR(base) ((base) + (0x0018UL))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_ADDR(base) ((base) + (0x001cUL))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT6_ADDR(base) ((base) + (0x0020UL))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_ADDR(base) ((base) + (0x0024UL))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_ADDR(base) ((base) + (0x0028UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ADDR(base) ((base) + (0x009CUL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_ADDR(base) ((base) + (0x00A0UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_ADDR(base) ((base) + (0x00A4UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_ADDR(base) ((base) + (0x00A8UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_ADDR(base) ((base) + (0x00ACUL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_ADDR(base) ((base) + (0x00B0UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_ADDR(base) ((base) + (0x00B4UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_ADDR(base) ((base) + (0x00B8UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_ADDR(base) ((base) + (0x00BCUL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_ADDR(base) ((base) + (0x00C0UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_ADDR(base) ((base) + (0x00C4UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ADDR(base) ((base) + (0x00C8UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ADDR(base) ((base) + (0x00CCUL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ADDR(base) ((base) + (0x00D0UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL17_ADDR(base) ((base) + (0x0E0UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_ADDR(base) ((base) + (0x0E4UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL19_ADDR(base) ((base) + (0x0E8UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL20_ADDR(base) ((base) + (0x0ECUL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL21_ADDR(base) ((base) + (0x0F0UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL22_ADDR(base) ((base) + (0x0F4UL))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_ADDR(base) ((base) + (0x0F8UL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG0_ADDR(base) ((base) + (0x0BE0UL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG1_ADDR(base) ((base) + (0x0BE4UL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG2_ADDR(base) ((base) + (0x0BE8UL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG3_ADDR(base) ((base) + (0x0BECUL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG4_ADDR(base) ((base) + (0x0BF0UL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG5_ADDR(base) ((base) + (0x0BF4UL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG6_ADDR(base) ((base) + (0x0BF8UL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG7_ADDR(base) ((base) + (0x0BFCUL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG0_ADDR(base) ((base) + (0x0C20UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG1_ADDR(base) ((base) + (0x0C24UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG2_ADDR(base) ((base) + (0x0C28UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG3_ADDR(base) ((base) + (0x0C2CUL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG4_ADDR(base) ((base) + (0x0C30UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG5_ADDR(base) ((base) + (0x0C34UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG6_ADDR(base) ((base) + (0x0C38UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG7_ADDR(base) ((base) + (0x0C3CUL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG8_ADDR(base) ((base) + (0x0C40UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG9_ADDR(base) ((base) + (0x0C44UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG10_ADDR(base) ((base) + (0x0C48UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG11_ADDR(base) ((base) + (0x0C4CUL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG12_ADDR(base) ((base) + (0x0C50UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG13_ADDR(base) ((base) + (0x0C54UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG14_ADDR(base) ((base) + (0x0C58UL))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG15_ADDR(base) ((base) + (0x0C5CUL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG8_ADDR(base) ((base) + (0x0C00UL))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG9_ADDR(base) ((base) + (0x0C04UL))
#else
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT0_ADDR(base) ((base) + (0x0008))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_ADDR(base) ((base) + (0x000C))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_ADDR(base) ((base) + (0x0010))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_ADDR(base) ((base) + (0x0014))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT4_ADDR(base) ((base) + (0x0018))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_ADDR(base) ((base) + (0x001c))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT6_ADDR(base) ((base) + (0x0020))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_ADDR(base) ((base) + (0x0024))
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_ADDR(base) ((base) + (0x0028))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ADDR(base) ((base) + (0x009C))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_ADDR(base) ((base) + (0x00A0))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_ADDR(base) ((base) + (0x00A4))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_ADDR(base) ((base) + (0x00A8))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_ADDR(base) ((base) + (0x00AC))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_ADDR(base) ((base) + (0x00B0))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_ADDR(base) ((base) + (0x00B4))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_ADDR(base) ((base) + (0x00B8))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_ADDR(base) ((base) + (0x00BC))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_ADDR(base) ((base) + (0x00C0))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_ADDR(base) ((base) + (0x00C4))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ADDR(base) ((base) + (0x00C8))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ADDR(base) ((base) + (0x00CC))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ADDR(base) ((base) + (0x00D0))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL17_ADDR(base) ((base) + (0x0E0))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_ADDR(base) ((base) + (0x0E4))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL19_ADDR(base) ((base) + (0x0E8))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL20_ADDR(base) ((base) + (0x0EC))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL21_ADDR(base) ((base) + (0x0F0))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL22_ADDR(base) ((base) + (0x0F4))
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_ADDR(base) ((base) + (0x0F8))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG0_ADDR(base) ((base) + (0x0BE0))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG1_ADDR(base) ((base) + (0x0BE4))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG2_ADDR(base) ((base) + (0x0BE8))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG3_ADDR(base) ((base) + (0x0BEC))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG4_ADDR(base) ((base) + (0x0BF0))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG5_ADDR(base) ((base) + (0x0BF4))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG6_ADDR(base) ((base) + (0x0BF8))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG7_ADDR(base) ((base) + (0x0BFC))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG0_ADDR(base) ((base) + (0x0C20))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG1_ADDR(base) ((base) + (0x0C24))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG2_ADDR(base) ((base) + (0x0C28))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG3_ADDR(base) ((base) + (0x0C2C))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG4_ADDR(base) ((base) + (0x0C30))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG5_ADDR(base) ((base) + (0x0C34))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG6_ADDR(base) ((base) + (0x0C38))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG7_ADDR(base) ((base) + (0x0C3C))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG8_ADDR(base) ((base) + (0x0C40))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG9_ADDR(base) ((base) + (0x0C44))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG10_ADDR(base) ((base) + (0x0C48))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG11_ADDR(base) ((base) + (0x0C4C))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG12_ADDR(base) ((base) + (0x0C50))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG13_ADDR(base) ((base) + (0x0C54))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG14_ADDR(base) ((base) + (0x0C58))
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG15_ADDR(base) ((base) + (0x0C5C))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG8_ADDR(base) ((base) + (0x0C00))
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG9_ADDR(base) ((base) + (0x0C04))
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_TS_SYSCTRL_SECURE0_ADDR(base) ((base) + (0x0000UL))
#define SOC_NPU_TS_SYSCTRL_SECURE1_ADDR(base) ((base) + (0x0004UL))
#define SOC_NPU_TS_SYSCTRL_SECURE2_ADDR(base) ((base) + (0x0008UL))
#define SOC_NPU_TS_SYSCTRL_SECURE3_ADDR(base) ((base) + (0x000CUL))
#define SOC_NPU_TS_SYSCTRL_SECURE4_ADDR(base) ((base) + (0x0010UL))
#define SOC_NPU_TS_SYSCTRL_SECURE5_ADDR(base) ((base) + (0x0014UL))
#define SOC_NPU_TS_SYSCTRL_SECURE6_ADDR(base) ((base) + (0x0018UL))
#define SOC_NPU_TS_SYSCTRL_SECURE7_ADDR(base) ((base) + (0x001CUL))
#else
#define SOC_NPU_TS_SYSCTRL_SECURE0_ADDR(base) ((base) + (0x0000))
#define SOC_NPU_TS_SYSCTRL_SECURE1_ADDR(base) ((base) + (0x0004))
#define SOC_NPU_TS_SYSCTRL_SECURE2_ADDR(base) ((base) + (0x0008))
#define SOC_NPU_TS_SYSCTRL_SECURE3_ADDR(base) ((base) + (0x000C))
#define SOC_NPU_TS_SYSCTRL_SECURE4_ADDR(base) ((base) + (0x0010))
#define SOC_NPU_TS_SYSCTRL_SECURE5_ADDR(base) ((base) + (0x0014))
#define SOC_NPU_TS_SYSCTRL_SECURE6_ADDR(base) ((base) + (0x0018))
#define SOC_NPU_TS_SYSCTRL_SECURE7_ADDR(base) ((base) + (0x001C))
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC0_ADDR(base) ((base) + (0x800UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC1_ADDR(base) ((base) + (0x804UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC2_ADDR(base) ((base) + (0x808UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC3_ADDR(base) ((base) + (0x80CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC4_ADDR(base) ((base) + (0x810UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC5_ADDR(base) ((base) + (0x814UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC6_ADDR(base) ((base) + (0x818UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC7_ADDR(base) ((base) + (0x81CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN0_ADDR(base) ((base) + (0x820UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN1_ADDR(base) ((base) + (0x824UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN2_ADDR(base) ((base) + (0x828UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN3_ADDR(base) ((base) + (0x82CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN4_ADDR(base) ((base) + (0x830UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN5_ADDR(base) ((base) + (0x834UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN6_ADDR(base) ((base) + (0x838UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN7_ADDR(base) ((base) + (0x83CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST0_ADDR(base) ((base) + (0x840UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST1_ADDR(base) ((base) + (0x844UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST2_ADDR(base) ((base) + (0x848UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST3_ADDR(base) ((base) + (0x84CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST4_ADDR(base) ((base) + (0x850UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST5_ADDR(base) ((base) + (0x854UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST6_ADDR(base) ((base) + (0x858UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST7_ADDR(base) ((base) + (0x85CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_ADDR_DEFAULT_ADDR(base) ((base) + (0x860UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC8_ADDR(base) ((base) + (0x900UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC9_ADDR(base) ((base) + (0x904UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC10_ADDR(base) ((base) + (0x908UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC11_ADDR(base) ((base) + (0x90cUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC12_ADDR(base) ((base) + (0x910UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC13_ADDR(base) ((base) + (0x914UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC14_ADDR(base) ((base) + (0x918UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC15_ADDR(base) ((base) + (0x91cUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN8_ADDR(base) ((base) + (0x920UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN9_ADDR(base) ((base) + (0x924UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN10_ADDR(base) ((base) + (0x928UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN11_ADDR(base) ((base) + (0x92CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN12_ADDR(base) ((base) + (0x930UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN13_ADDR(base) ((base) + (0x934UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN14_ADDR(base) ((base) + (0x938UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN15_ADDR(base) ((base) + (0x93CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST8_ADDR(base) ((base) + (0x940UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST9_ADDR(base) ((base) + (0x944UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST10_ADDR(base) ((base) + (0x948UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST11_ADDR(base) ((base) + (0x94CUL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST12_ADDR(base) ((base) + (0x950UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST13_ADDR(base) ((base) + (0x954UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST14_ADDR(base) ((base) + (0x958UL))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST15_ADDR(base) ((base) + (0x95CUL))
#define SOC_NPU_TS_SYSCTRL_DMMU_PROT_WR_ADDR(base) ((base) + (0x960UL))
#define SOC_NPU_TS_SYSCTRL_DMMU_PROT_RD_ADDR(base) ((base) + (0x964UL))
#else
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC0_ADDR(base) ((base) + (0x800))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC1_ADDR(base) ((base) + (0x804))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC2_ADDR(base) ((base) + (0x808))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC3_ADDR(base) ((base) + (0x80C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC4_ADDR(base) ((base) + (0x810))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC5_ADDR(base) ((base) + (0x814))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC6_ADDR(base) ((base) + (0x818))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC7_ADDR(base) ((base) + (0x81C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN0_ADDR(base) ((base) + (0x820))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN1_ADDR(base) ((base) + (0x824))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN2_ADDR(base) ((base) + (0x828))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN3_ADDR(base) ((base) + (0x82C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN4_ADDR(base) ((base) + (0x830))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN5_ADDR(base) ((base) + (0x834))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN6_ADDR(base) ((base) + (0x838))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN7_ADDR(base) ((base) + (0x83C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST0_ADDR(base) ((base) + (0x840))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST1_ADDR(base) ((base) + (0x844))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST2_ADDR(base) ((base) + (0x848))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST3_ADDR(base) ((base) + (0x84C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST4_ADDR(base) ((base) + (0x850))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST5_ADDR(base) ((base) + (0x854))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST6_ADDR(base) ((base) + (0x858))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST7_ADDR(base) ((base) + (0x85C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_ADDR_DEFAULT_ADDR(base) ((base) + (0x860))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC8_ADDR(base) ((base) + (0x900))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC9_ADDR(base) ((base) + (0x904))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC10_ADDR(base) ((base) + (0x908))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC11_ADDR(base) ((base) + (0x90c))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC12_ADDR(base) ((base) + (0x910))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC13_ADDR(base) ((base) + (0x914))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC14_ADDR(base) ((base) + (0x918))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC15_ADDR(base) ((base) + (0x91c))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN8_ADDR(base) ((base) + (0x920))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN9_ADDR(base) ((base) + (0x924))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN10_ADDR(base) ((base) + (0x928))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN11_ADDR(base) ((base) + (0x92C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN12_ADDR(base) ((base) + (0x930))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN13_ADDR(base) ((base) + (0x934))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN14_ADDR(base) ((base) + (0x938))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN15_ADDR(base) ((base) + (0x93C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST8_ADDR(base) ((base) + (0x940))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST9_ADDR(base) ((base) + (0x944))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST10_ADDR(base) ((base) + (0x948))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST11_ADDR(base) ((base) + (0x94C))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST12_ADDR(base) ((base) + (0x950))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST13_ADDR(base) ((base) + (0x954))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST14_ADDR(base) ((base) + (0x958))
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST15_ADDR(base) ((base) + (0x95C))
#define SOC_NPU_TS_SYSCTRL_DMMU_PROT_WR_ADDR(base) ((base) + (0x960))
#define SOC_NPU_TS_SYSCTRL_DMMU_PROT_RD_ADDR(base) ((base) + (0x964))
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_TS_SYSCTRL_MON_ENABLE_ADDR(base) ((base) + (0x0000UL))
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_ADDR(base) ((base) + (0x0004UL))
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_ADDR(base) ((base) + (0x0008UL))
#define SOC_NPU_TS_SYSCTRL_MON_W_IDMASK_ADDR(base) ((base) + (0x000CUL))
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_ADDR(base) ((base) + (0x0010UL))
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_ADDR(base) ((base) + (0x0014UL))
#define SOC_NPU_TS_SYSCTRL_MON_R_IDMASK_ADDR(base) ((base) + (0x0018UL))
#define SOC_NPU_TS_SYSCTRL_START_ADDR_LOW_ADDR(base) ((base) + (0x0020UL))
#define SOC_NPU_TS_SYSCTRL_START_ADDR_HIGH_ADDR(base) ((base) + (0x0024UL))
#define SOC_NPU_TS_SYSCTRL_END_ADDR_LOW_ADDR(base) ((base) + (0x0028UL))
#define SOC_NPU_TS_SYSCTRL_END_ADDR_HIGH_ADDR(base) ((base) + (0x002CUL))
#define SOC_NPU_TS_SYSCTRL_MON_W_ADDR_L_ADDR(base) ((base) + (0x0040UL))
#define SOC_NPU_TS_SYSCTRL_MON_AW_CNT_ADDR(base) ((base) + (0x0044UL))
#define SOC_NPU_TS_SYSCTRL_MON_W_CNT_ADDR(base) ((base) + (0x0048UL))
#define SOC_NPU_TS_SYSCTRL_MON_R_ADDR_L_ADDR(base) ((base) + (0x004CUL))
#define SOC_NPU_TS_SYSCTRL_MON_AR_CNT_ADDR(base) ((base) + (0x0050UL))
#define SOC_NPU_TS_SYSCTRL_MON_R_CNT_ADDR(base) ((base) + (0x0054UL))
#define SOC_NPU_TS_SYSCTRL_MON_W_ADDR_H_ADDR(base) ((base) + (0x0058UL))
#define SOC_NPU_TS_SYSCTRL_MON_R_ADDR_H_ADDR(base) ((base) + (0x005CUL))
#else
#define SOC_NPU_TS_SYSCTRL_MON_ENABLE_ADDR(base) ((base) + (0x0000))
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_ADDR(base) ((base) + (0x0004))
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_ADDR(base) ((base) + (0x0008))
#define SOC_NPU_TS_SYSCTRL_MON_W_IDMASK_ADDR(base) ((base) + (0x000C))
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_ADDR(base) ((base) + (0x0010))
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_ADDR(base) ((base) + (0x0014))
#define SOC_NPU_TS_SYSCTRL_MON_R_IDMASK_ADDR(base) ((base) + (0x0018))
#define SOC_NPU_TS_SYSCTRL_START_ADDR_LOW_ADDR(base) ((base) + (0x0020))
#define SOC_NPU_TS_SYSCTRL_START_ADDR_HIGH_ADDR(base) ((base) + (0x0024))
#define SOC_NPU_TS_SYSCTRL_END_ADDR_LOW_ADDR(base) ((base) + (0x0028))
#define SOC_NPU_TS_SYSCTRL_END_ADDR_HIGH_ADDR(base) ((base) + (0x002C))
#define SOC_NPU_TS_SYSCTRL_MON_W_ADDR_L_ADDR(base) ((base) + (0x0040))
#define SOC_NPU_TS_SYSCTRL_MON_AW_CNT_ADDR(base) ((base) + (0x0044))
#define SOC_NPU_TS_SYSCTRL_MON_W_CNT_ADDR(base) ((base) + (0x0048))
#define SOC_NPU_TS_SYSCTRL_MON_R_ADDR_L_ADDR(base) ((base) + (0x004C))
#define SOC_NPU_TS_SYSCTRL_MON_AR_CNT_ADDR(base) ((base) + (0x0050))
#define SOC_NPU_TS_SYSCTRL_MON_R_CNT_ADDR(base) ((base) + (0x0054))
#define SOC_NPU_TS_SYSCTRL_MON_W_ADDR_H_ADDR(base) ((base) + (0x0058))
#define SOC_NPU_TS_SYSCTRL_MON_R_ADDR_H_ADDR(base) ((base) + (0x005C))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_axi_dlock_slv : 10;
        unsigned int reserved_0 : 6;
        unsigned int peri_axi_dlock_mst : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT0_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT0_peri_axi_dlock_slv_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT0_peri_axi_dlock_slv_END (9)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT0_peri_axi_dlock_mst_START (16)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT0_peri_axi_dlock_mst_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dw1to2_axi_dlock_slv : 4;
        unsigned int dw1to2_axi_dlock_mst : 2;
        unsigned int dw2to1_axi_dlock_slv : 2;
        unsigned int dw2to1_axi_dlock_mst : 3;
        unsigned int reserved : 21;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_dw1to2_axi_dlock_slv_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_dw1to2_axi_dlock_slv_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_dw1to2_axi_dlock_mst_START (4)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_dw1to2_axi_dlock_mst_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_dw2to1_axi_dlock_slv_START (6)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_dw2to1_axi_dlock_slv_END (7)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_dw2to1_axi_dlock_mst_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT1_dw2to1_axi_dlock_mst_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rsv : 1;
        unsigned int rsv1 : 10;
        unsigned int rsv2 : 10;
        unsigned int reserved : 11;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_rsv_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_rsv_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_rsv1_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_rsv1_END (10)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_rsv2_START (11)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT2_rsv2_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rv_core_hard_fault_mode : 1;
        unsigned int rv_core_debug_mode : 1;
        unsigned int rv_core_sleep_mode : 1;
        unsigned int hmx_xwdecerr : 1;
        unsigned int hmx_slverr : 1;
        unsigned int sizeshap_err_stat : 3;
        unsigned int reserved : 24;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_rv_core_hard_fault_mode_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_rv_core_hard_fault_mode_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_rv_core_debug_mode_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_rv_core_debug_mode_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_rv_core_sleep_mode_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_rv_core_sleep_mode_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_hmx_xwdecerr_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_hmx_xwdecerr_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_hmx_slverr_START (4)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_hmx_slverr_END (4)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_sizeshap_err_stat_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT3_sizeshap_err_stat_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int st_clk_timer : 1;
        unsigned int st_clk_wdog : 1;
        unsigned int st_clk_apb_dmmu_lbu : 1;
        unsigned int st_clk_apb_dmmu_cbu : 1;
        unsigned int st_clk_apb_mon_hwts : 1;
        unsigned int st_clk_apb_mon_cbu : 1;
        unsigned int st_clk_apb_mon_lbu : 1;
        unsigned int st_clk_apb_sysctrl : 1;
        unsigned int rsv_its : 1;
        unsigned int st_clk_apb_sec : 1;
        unsigned int rsv_arc : 1;
        unsigned int rsv_hwts : 1;
        unsigned int rsv_doorbell : 1;
        unsigned int reserved : 19;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_timer_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_timer_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_wdog_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_wdog_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_dmmu_lbu_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_dmmu_lbu_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_dmmu_cbu_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_dmmu_cbu_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_mon_hwts_START (4)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_mon_hwts_END (4)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_mon_cbu_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_mon_cbu_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_mon_lbu_START (6)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_mon_lbu_END (6)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_sysctrl_START (7)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_sysctrl_END (7)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_rsv_its_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_rsv_its_END (8)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_sec_START (9)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_st_clk_apb_sec_END (9)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_rsv_arc_START (10)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_rsv_arc_END (10)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_rsv_hwts_START (11)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_rsv_hwts_END (11)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_rsv_doorbell_START (12)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT5_rsv_doorbell_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int port_busy_dw_4to5_idle : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT6_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT6_port_busy_dw_4to5_idle_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT6_port_busy_dw_4to5_idle_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int port_dw_1to2_bus_idle : 1;
        unsigned int port_dw_2to1_bus_idle : 1;
        unsigned int port_hwts_axi128to32_idle : 1;
        unsigned int port_ahb32to128_idle : 1;
        unsigned int reserved_0 : 1;
        unsigned int port_db_axitogs_idle : 1;
        unsigned int port_apb_axi128toapb32 : 1;
        unsigned int port_rv_axi64to128_asyn_idle : 1;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_dw_1to2_bus_idle_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_dw_1to2_bus_idle_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_dw_2to1_bus_idle_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_dw_2to1_bus_idle_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_hwts_axi128to32_idle_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_hwts_axi128to32_idle_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_ahb32to128_idle_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_ahb32to128_idle_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_db_axitogs_idle_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_db_axitogs_idle_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_apb_axi128toapb32_START (6)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_apb_axi128toapb32_END (6)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_rv_axi64to128_asyn_idle_START (7)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT7_port_rv_axi64to128_asyn_idle_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hprot_s_err : 3;
        unsigned int hburst_s_err : 2;
        unsigned int hsize_s_err : 2;
        unsigned int hwrite_s_err : 1;
        unsigned int htrans_s_err : 2;
        unsigned int reserved : 22;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_hprot_s_err_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_hprot_s_err_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_hburst_s_err_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_hburst_s_err_END (4)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_hsize_s_err_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_hsize_s_err_END (6)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_hwrite_s_err_START (7)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_hwrite_s_err_END (7)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_htrans_s_err_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_SYSSTAT8_htrans_s_err_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ts_dw4to5_ckg_en : 1;
        unsigned int ts_dw2to1_ckg_en : 1;
        unsigned int ts_dw1to2_ckg_en : 1;
        unsigned int ts_rv_h2x_ckg_en : 1;
        unsigned int ts_rv_x2x_64to128_syn_ckg_en : 1;
        unsigned int ts_rv_x2x_64to128_asyn_ckg_en : 1;
        unsigned int ts_hwts_x2x_128to32_syn_ckg_en : 1;
        unsigned int ts_apb_x2p_128to32_syn_ckg_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 21;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_dw4to5_ckg_en_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_dw4to5_ckg_en_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_dw2to1_ckg_en_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_dw2to1_ckg_en_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_dw1to2_ckg_en_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_dw1to2_ckg_en_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_rv_h2x_ckg_en_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_rv_h2x_ckg_en_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_rv_x2x_64to128_syn_ckg_en_START (4)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_rv_x2x_64to128_syn_ckg_en_END (4)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_rv_x2x_64to128_asyn_ckg_en_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_rv_x2x_64to128_asyn_ckg_en_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_hwts_x2x_128to32_syn_ckg_en_START (6)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_hwts_x2x_128to32_syn_ckg_en_END (6)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_apb_x2p_128to32_syn_ckg_en_START (7)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL0_ts_apb_x2p_128to32_syn_ckg_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arc_mst_priority_m1 : 1;
        unsigned int arc_mst_priority_m2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int arc_slv_priority_s1 : 2;
        unsigned int arc_slv_priority_s2 : 2;
        unsigned int arc_event_o : 1;
        unsigned int reserved_1 : 7;
        unsigned int axi_cfg_awqos : 4;
        unsigned int axi_cfg_arqos : 4;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_mst_priority_m1_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_mst_priority_m1_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_mst_priority_m2_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_mst_priority_m2_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_slv_priority_s1_START (4)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_slv_priority_s1_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_slv_priority_s2_START (6)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_slv_priority_s2_END (7)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_event_o_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_arc_event_o_END (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_axi_cfg_awqos_START (16)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_axi_cfg_awqos_END (19)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_axi_cfg_arqos_START (20)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL1_axi_cfg_arqos_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int peri_axi_dlock_clear : 1;
        unsigned int dw2to1_dlock_clear : 1;
        unsigned int reserved_0 : 6;
        unsigned int dw_axi_peri_gatedclock_en : 1;
        unsigned int dw_axi_arc_gatedclock_en : 1;
        unsigned int reserved_1 : 22;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_peri_axi_dlock_clear_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_peri_axi_dlock_clear_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_dw2to1_dlock_clear_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_dw2to1_dlock_clear_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_dw_axi_peri_gatedclock_en_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_dw_axi_peri_gatedclock_en_END (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_dw_axi_arc_gatedclock_en_START (9)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL2_dw_axi_arc_gatedclock_en_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hpm_div : 6;
        unsigned int hpm_en : 1;
        unsigned int hpm_res_sel : 1;
        unsigned int tidm_mode : 2;
        unsigned int reserved : 22;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_hpm_div_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_hpm_div_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_hpm_en_START (6)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_hpm_en_END (6)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_hpm_res_sel_START (7)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_hpm_res_sel_END (7)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_tidm_mode_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL3_tidm_mode_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_tbu_awqos_arc : 4;
        unsigned int mem_tbu_arqos_arc : 4;
        unsigned int mem_tbu_awqos_hwts : 4;
        unsigned int mem_tbu_arqos_hwts : 4;
        unsigned int reserved : 16;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_mem_tbu_awqos_arc_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_mem_tbu_awqos_arc_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_mem_tbu_arqos_arc_START (4)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_mem_tbu_arqos_arc_END (7)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_mem_tbu_awqos_hwts_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_mem_tbu_awqos_hwts_END (11)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_mem_tbu_arqos_hwts_START (12)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL4_mem_tbu_arqos_hwts_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_smmu_awsid : 8;
        unsigned int hwts_smmu_arsid : 8;
        unsigned int ts_cpu_smmu_awsid : 8;
        unsigned int ts_cpu_smmu_arsid : 8;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_hwts_smmu_awsid_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_hwts_smmu_awsid_END (7)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_hwts_smmu_arsid_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_hwts_smmu_arsid_END (15)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_ts_cpu_smmu_awsid_START (16)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_ts_cpu_smmu_awsid_END (23)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_ts_cpu_smmu_arsid_START (24)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL5_ts_cpu_smmu_arsid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_smmu_awssid : 8;
        unsigned int hwts_smmu_arssid : 8;
        unsigned int ts_cpu_smmu_awssid : 8;
        unsigned int ts_cpu_smmu_arssid : 8;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_hwts_smmu_awssid_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_hwts_smmu_awssid_END (7)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_hwts_smmu_arssid_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_hwts_smmu_arssid_END (15)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_ts_cpu_smmu_awssid_START (16)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_ts_cpu_smmu_awssid_END (23)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_ts_cpu_smmu_arssid_START (24)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL6_ts_cpu_smmu_arssid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_smmu_awssidv : 1;
        unsigned int hwts_smmu_arssidv : 1;
        unsigned int ts_cpu_smmu_awssidv : 1;
        unsigned int ts_cpu_smmu_arssidv : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_hwts_smmu_awssidv_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_hwts_smmu_awssidv_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_hwts_smmu_arssidv_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_hwts_smmu_arssidv_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_ts_cpu_smmu_awssidv_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_ts_cpu_smmu_awssidv_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_ts_cpu_smmu_arssidv_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL7_ts_cpu_smmu_arssidv_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_timer_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_clk_apb_dmmu_cbu_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_apb_mon_cbu_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int gt_clk_apb_easc_en : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_9 : 19;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_gt_clk_timer_en_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_gt_clk_timer_en_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_gt_clk_apb_dmmu_cbu_en_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_gt_clk_apb_dmmu_cbu_en_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_gt_clk_apb_mon_cbu_en_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_gt_clk_apb_mon_cbu_en_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_gt_clk_apb_easc_en_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL8_gt_clk_apb_easc_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_timer_clr : 1;
        unsigned int gt_clk_wdog_clr : 1;
        unsigned int gt_clk_apb_dmmu_lbu_clr : 1;
        unsigned int gt_clk_apb_dmmu_cbu_clr : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_apb_mon_cbu_clr : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_apb_easc_clr : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 19;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_timer_clr_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_timer_clr_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_wdog_clr_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_wdog_clr_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_apb_dmmu_lbu_clr_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_apb_dmmu_lbu_clr_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_apb_dmmu_cbu_clr_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_apb_dmmu_cbu_clr_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_apb_mon_cbu_clr_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_apb_mon_cbu_clr_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_apb_easc_clr_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL9_gt_clk_apb_easc_clr_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int gt_clk_timer_st : 1;
        unsigned int gt_clk_wdog_st : 1;
        unsigned int gt_clk_apb_dmmu_lbu_st : 1;
        unsigned int gt_clk_apb_dmmu_cbu_st : 1;
        unsigned int reserved_0 : 1;
        unsigned int gt_clk_apb_mon_cbu_st : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_clk_apb_easc_st : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 19;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_timer_st_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_timer_st_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_wdog_st_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_wdog_st_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_apb_dmmu_lbu_st_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_apb_dmmu_lbu_st_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_apb_dmmu_cbu_st_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_apb_dmmu_cbu_st_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_apb_mon_cbu_st_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_apb_mon_cbu_st_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_apb_easc_st_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL10_gt_clk_apb_easc_st_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_timer_en : 1;
        unsigned int ip_rst_wdog_en : 1;
        unsigned int ip_rst_apb_dmmu_lbu_en : 1;
        unsigned int ip_rst_apb_dmmu_cbu_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_apb_mon_cbu_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_apb_easc_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_rst_arc_en : 1;
        unsigned int ip_rst_hwts_en : 1;
        unsigned int ip_rst_db_en : 1;
        unsigned int reserved_4 : 19;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_timer_en_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_timer_en_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_wdog_en_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_wdog_en_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_apb_dmmu_lbu_en_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_apb_dmmu_lbu_en_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_apb_dmmu_cbu_en_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_apb_dmmu_cbu_en_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_apb_mon_cbu_en_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_apb_mon_cbu_en_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_apb_easc_en_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_apb_easc_en_END (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_arc_en_START (10)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_arc_en_END (10)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_hwts_en_START (11)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_hwts_en_END (11)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_db_en_START (12)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL11_ip_rst_db_en_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_timer_clr : 1;
        unsigned int ip_rst_wdog_clr : 1;
        unsigned int ip_rst_apb_dmmu_lbu_clr : 1;
        unsigned int ip_rst_apb_dmmu_cbu_clr : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_apb_mon_cbu_clr : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_apb_easc_clr : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_7 : 19;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_timer_clr_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_timer_clr_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_wdog_clr_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_wdog_clr_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_apb_dmmu_lbu_clr_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_apb_dmmu_lbu_clr_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_apb_dmmu_cbu_clr_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_apb_dmmu_cbu_clr_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_apb_mon_cbu_clr_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_apb_mon_cbu_clr_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_apb_easc_clr_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL12_ip_rst_apb_easc_clr_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ip_rst_timer_st : 1;
        unsigned int ip_rst_wdog_st : 1;
        unsigned int ip_rst_apb_dmmu_lbu_st : 1;
        unsigned int ip_rst_apb_dmmu_cbu_st : 1;
        unsigned int reserved_0 : 1;
        unsigned int ip_rst_apb_mon_cbu_st : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int ip_rst_apb_easc_st : 1;
        unsigned int reserved_3 : 1;
        unsigned int ip_rst_arc_st : 1;
        unsigned int ip_rst_hwts_st : 1;
        unsigned int ip_rst_db_st : 1;
        unsigned int reserved_4 : 19;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_timer_st_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_timer_st_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_wdog_st_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_wdog_st_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_apb_dmmu_lbu_st_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_apb_dmmu_lbu_st_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_apb_dmmu_cbu_st_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_apb_dmmu_cbu_st_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_apb_mon_cbu_st_START (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_apb_mon_cbu_st_END (5)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_apb_easc_st_START (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_apb_easc_st_END (8)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_arc_st_START (10)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_arc_st_END (10)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_hwts_st_START (11)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_hwts_st_END (11)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_db_st_START (12)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL13_ip_rst_db_st_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_ts_mem_ctrl_low : 2;
        unsigned int reserved_0 : 1;
        unsigned int sc_ts_mem_ctrl_hign : 23;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL17_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL17_sc_ts_mem_ctrl_low_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL17_sc_ts_mem_ctrl_low_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL17_sc_ts_mem_ctrl_hign_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL17_sc_ts_mem_ctrl_hign_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rv_core_wait : 1;
        unsigned int hmx_xwerrclr : 1;
        unsigned int shapsize_err_int_clr : 1;
        unsigned int sizeshap_err_int_bypass : 1;
        unsigned int size_shap_bypass : 1;
        unsigned int reserved : 7;
        unsigned int rv_ahbm_rgn_start_addr : 20;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_rv_core_wait_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_rv_core_wait_END (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_hmx_xwerrclr_START (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_hmx_xwerrclr_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_shapsize_err_int_clr_START (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_shapsize_err_int_clr_END (2)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_sizeshap_err_int_bypass_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_sizeshap_err_int_bypass_END (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_size_shap_bypass_START (4)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_size_shap_bypass_END (4)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_rv_ahbm_rgn_start_addr_START (12)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL18_rv_ahbm_rgn_start_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int rv_ahbm_rgn_end_addr : 20;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL19_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL19_rv_ahbm_rgn_end_addr_START (12)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL19_rv_ahbm_rgn_end_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int rv_axim_rgn_start_addr : 20;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL20_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL20_rv_axim_rgn_start_addr_START (12)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL20_rv_axim_rgn_start_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int rv_axim_rgn_end_addr : 20;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL21_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL21_rv_axim_rgn_end_addr_START (12)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL21_rv_axim_rgn_end_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rv_por_rst_pc : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL22_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL22_rv_por_rst_pc_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL22_rv_por_rst_pc_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_pmu_mem_ctrl_tp_dlw2r_reg_l : 2;
        unsigned int reserved_0 : 1;
        unsigned int hwts_pmu_mem_ctrl_tp_dlw2r_reg_h : 13;
        unsigned int hwts_sq_mem_ctrl_tp_dlw2r_reg_l : 2;
        unsigned int reserved_1 : 1;
        unsigned int hwts_sq_mem_ctrl_tp_dlw2r_reg_h : 13;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_hwts_pmu_mem_ctrl_tp_dlw2r_reg_l_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_hwts_pmu_mem_ctrl_tp_dlw2r_reg_l_END (1)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_hwts_pmu_mem_ctrl_tp_dlw2r_reg_h_START (3)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_hwts_pmu_mem_ctrl_tp_dlw2r_reg_h_END (15)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_hwts_sq_mem_ctrl_tp_dlw2r_reg_l_START (16)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_hwts_sq_mem_ctrl_tp_dlw2r_reg_l_END (17)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_hwts_sq_mem_ctrl_tp_dlw2r_reg_h_START (19)
#define SOC_NPU_TS_SYSCTRL_SC_PERCTRL23_hwts_sq_mem_ctrl_tp_dlw2r_reg_h_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int test_reg2 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG2_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG2_test_reg2_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG2_test_reg2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int test_reg3 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG3_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG3_test_reg3_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG3_test_reg3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int test_reg4 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG4_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG4_test_reg4_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG4_test_reg4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int test_reg5 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG5_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG5_test_reg5_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG5_test_reg5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int test_reg6 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG6_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG6_test_reg6_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG6_test_reg6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int test_reg7 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG7_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG7_test_reg7_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG7_test_reg7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg0 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG0_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG0_sema_reg0_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG0_sema_reg0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg1 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG1_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG1_sema_reg1_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG1_sema_reg1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg2 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG2_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG2_sema_reg2_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG2_sema_reg2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg3 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG3_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG3_sema_reg3_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG3_sema_reg3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg4 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG4_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG4_sema_reg4_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG4_sema_reg4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg5 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG5_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG5_sema_reg5_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG5_sema_reg5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg6 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG6_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG6_sema_reg6_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG6_sema_reg6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg7 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG7_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG7_sema_reg7_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG7_sema_reg7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg8 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG8_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG8_sema_reg8_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG8_sema_reg8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg9 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG9_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG9_sema_reg9_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG9_sema_reg9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg10 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG10_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG10_sema_reg10_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG10_sema_reg10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg11 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG11_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG11_sema_reg11_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG11_sema_reg11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg12 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG12_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG12_sema_reg12_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG12_sema_reg12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg13 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG13_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG13_sema_reg13_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG13_sema_reg13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg14 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG14_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG14_sema_reg14_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG14_sema_reg14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sema_reg15 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_SEMAREG15_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG15_sema_reg15_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_SEMAREG15_sema_reg15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int test_reg8 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG8_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG8_test_reg8_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG8_test_reg8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int test_reg9 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SC_TESTREG9_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG9_test_reg9_START (0)
#define SOC_NPU_TS_SYSCTRL_SC_TESTREG9_test_reg9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ts_wdog_ns : 1;
        unsigned int ts_timer_ns : 1;
        unsigned int ts_sysctrl_ns : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int ts_aximon_arc_cbu_ns : 1;
        unsigned int reserved_2 : 1;
        unsigned int ts_dmmu_cbu_ns : 1;
        unsigned int ts_dmmu_lbu_ns : 1;
        unsigned int ts_smmu_tbu_ns : 1;
        unsigned int easc2_ns : 1;
        unsigned int reserved_3 : 21;
    } reg;
} SOC_NPU_TS_SYSCTRL_SECURE0_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_wdog_ns_START (0)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_wdog_ns_END (0)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_timer_ns_START (1)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_timer_ns_END (1)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_sysctrl_ns_START (2)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_sysctrl_ns_END (2)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_aximon_arc_cbu_ns_START (5)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_aximon_arc_cbu_ns_END (5)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_dmmu_cbu_ns_START (7)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_dmmu_cbu_ns_END (7)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_dmmu_lbu_ns_START (8)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_dmmu_lbu_ns_END (8)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_smmu_tbu_ns_START (9)
#define SOC_NPU_TS_SYSCTRL_SECURE0_ts_smmu_tbu_ns_END (9)
#define SOC_NPU_TS_SYSCTRL_SECURE0_easc2_ns_START (10)
#define SOC_NPU_TS_SYSCTRL_SECURE0_easc2_ns_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_smmu_awsid_phy : 8;
        unsigned int hwts_smmu_arsid_phy : 8;
        unsigned int ts_cpu_smmu_awsid_phy : 8;
        unsigned int ts_cpu_smmu_arsid_phy : 8;
    } reg;
} SOC_NPU_TS_SYSCTRL_SECURE1_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SECURE1_hwts_smmu_awsid_phy_START (0)
#define SOC_NPU_TS_SYSCTRL_SECURE1_hwts_smmu_awsid_phy_END (7)
#define SOC_NPU_TS_SYSCTRL_SECURE1_hwts_smmu_arsid_phy_START (8)
#define SOC_NPU_TS_SYSCTRL_SECURE1_hwts_smmu_arsid_phy_END (15)
#define SOC_NPU_TS_SYSCTRL_SECURE1_ts_cpu_smmu_awsid_phy_START (16)
#define SOC_NPU_TS_SYSCTRL_SECURE1_ts_cpu_smmu_awsid_phy_END (23)
#define SOC_NPU_TS_SYSCTRL_SECURE1_ts_cpu_smmu_arsid_phy_START (24)
#define SOC_NPU_TS_SYSCTRL_SECURE1_ts_cpu_smmu_arsid_phy_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_smmu_awsid_s : 8;
        unsigned int hwts_smmu_arsid_s : 8;
        unsigned int ts_cpu_smmu_awsid_s : 8;
        unsigned int ts_cpu_smmu_arsid_s : 8;
    } reg;
} SOC_NPU_TS_SYSCTRL_SECURE2_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SECURE2_hwts_smmu_awsid_s_START (0)
#define SOC_NPU_TS_SYSCTRL_SECURE2_hwts_smmu_awsid_s_END (7)
#define SOC_NPU_TS_SYSCTRL_SECURE2_hwts_smmu_arsid_s_START (8)
#define SOC_NPU_TS_SYSCTRL_SECURE2_hwts_smmu_arsid_s_END (15)
#define SOC_NPU_TS_SYSCTRL_SECURE2_ts_cpu_smmu_awsid_s_START (16)
#define SOC_NPU_TS_SYSCTRL_SECURE2_ts_cpu_smmu_awsid_s_END (23)
#define SOC_NPU_TS_SYSCTRL_SECURE2_ts_cpu_smmu_arsid_s_START (24)
#define SOC_NPU_TS_SYSCTRL_SECURE2_ts_cpu_smmu_arsid_s_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_smmu_awssidv_img : 1;
        unsigned int hwts_smmu_arssidv_img : 1;
        unsigned int ts_cpu_smmu_awssidv_img : 1;
        unsigned int ts_cpu_smmu_arssidv_img : 1;
        unsigned int hwts_smmu_awssidv_s : 1;
        unsigned int hwts_smmu_arssidv_s : 1;
        unsigned int ts_cpu_smmu_awssidv_s : 1;
        unsigned int ts_cpu_smmu_arssidv_s : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_NPU_TS_SYSCTRL_SECURE3_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SECURE3_hwts_smmu_awssidv_img_START (0)
#define SOC_NPU_TS_SYSCTRL_SECURE3_hwts_smmu_awssidv_img_END (0)
#define SOC_NPU_TS_SYSCTRL_SECURE3_hwts_smmu_arssidv_img_START (1)
#define SOC_NPU_TS_SYSCTRL_SECURE3_hwts_smmu_arssidv_img_END (1)
#define SOC_NPU_TS_SYSCTRL_SECURE3_ts_cpu_smmu_awssidv_img_START (2)
#define SOC_NPU_TS_SYSCTRL_SECURE3_ts_cpu_smmu_awssidv_img_END (2)
#define SOC_NPU_TS_SYSCTRL_SECURE3_ts_cpu_smmu_arssidv_img_START (3)
#define SOC_NPU_TS_SYSCTRL_SECURE3_ts_cpu_smmu_arssidv_img_END (3)
#define SOC_NPU_TS_SYSCTRL_SECURE3_hwts_smmu_awssidv_s_START (4)
#define SOC_NPU_TS_SYSCTRL_SECURE3_hwts_smmu_awssidv_s_END (4)
#define SOC_NPU_TS_SYSCTRL_SECURE3_hwts_smmu_arssidv_s_START (5)
#define SOC_NPU_TS_SYSCTRL_SECURE3_hwts_smmu_arssidv_s_END (5)
#define SOC_NPU_TS_SYSCTRL_SECURE3_ts_cpu_smmu_awssidv_s_START (6)
#define SOC_NPU_TS_SYSCTRL_SECURE3_ts_cpu_smmu_awssidv_s_END (6)
#define SOC_NPU_TS_SYSCTRL_SECURE3_ts_cpu_smmu_arssidv_s_START (7)
#define SOC_NPU_TS_SYSCTRL_SECURE3_ts_cpu_smmu_arssidv_s_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_smmu_awssid_s : 8;
        unsigned int hwts_smmu_arssid_s : 8;
        unsigned int ts_cpu_smmu_awssid_s : 8;
        unsigned int ts_cpu_smmu_arssid_s : 8;
    } reg;
} SOC_NPU_TS_SYSCTRL_SECURE4_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SECURE4_hwts_smmu_awssid_s_START (0)
#define SOC_NPU_TS_SYSCTRL_SECURE4_hwts_smmu_awssid_s_END (7)
#define SOC_NPU_TS_SYSCTRL_SECURE4_hwts_smmu_arssid_s_START (8)
#define SOC_NPU_TS_SYSCTRL_SECURE4_hwts_smmu_arssid_s_END (15)
#define SOC_NPU_TS_SYSCTRL_SECURE4_ts_cpu_smmu_awssid_s_START (16)
#define SOC_NPU_TS_SYSCTRL_SECURE4_ts_cpu_smmu_awssid_s_END (23)
#define SOC_NPU_TS_SYSCTRL_SECURE4_ts_cpu_smmu_arssid_s_START (24)
#define SOC_NPU_TS_SYSCTRL_SECURE4_ts_cpu_smmu_arssid_s_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int hwts_awssid_phy : 8;
        unsigned int hwts_arssid_phy : 8;
        unsigned int arc_awssid_phy : 8;
        unsigned int arc_arssid_phy : 8;
    } reg;
} SOC_NPU_TS_SYSCTRL_SECURE5_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SECURE5_hwts_awssid_phy_START (0)
#define SOC_NPU_TS_SYSCTRL_SECURE5_hwts_awssid_phy_END (7)
#define SOC_NPU_TS_SYSCTRL_SECURE5_hwts_arssid_phy_START (8)
#define SOC_NPU_TS_SYSCTRL_SECURE5_hwts_arssid_phy_END (15)
#define SOC_NPU_TS_SYSCTRL_SECURE5_arc_awssid_phy_START (16)
#define SOC_NPU_TS_SYSCTRL_SECURE5_arc_awssid_phy_END (23)
#define SOC_NPU_TS_SYSCTRL_SECURE5_arc_arssid_phy_START (24)
#define SOC_NPU_TS_SYSCTRL_SECURE5_arc_arssid_phy_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arc_cbu_awprot : 1;
        unsigned int arc_cbu_arprot : 1;
        unsigned int arc_lbu_awprot : 1;
        unsigned int arc_lbu_arprot : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_NPU_TS_SYSCTRL_SECURE6_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SECURE6_arc_cbu_awprot_START (0)
#define SOC_NPU_TS_SYSCTRL_SECURE6_arc_cbu_awprot_END (0)
#define SOC_NPU_TS_SYSCTRL_SECURE6_arc_cbu_arprot_START (1)
#define SOC_NPU_TS_SYSCTRL_SECURE6_arc_cbu_arprot_END (1)
#define SOC_NPU_TS_SYSCTRL_SECURE6_arc_lbu_awprot_START (2)
#define SOC_NPU_TS_SYSCTRL_SECURE6_arc_lbu_awprot_END (2)
#define SOC_NPU_TS_SYSCTRL_SECURE6_arc_lbu_arprot_START (3)
#define SOC_NPU_TS_SYSCTRL_SECURE6_arc_lbu_arprot_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int secure7 : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_SECURE7_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_SECURE7_secure7_START (0)
#define SOC_NPU_TS_SYSCTRL_SECURE7_secure7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src0 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC0_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC0_icfg_addr_src0_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC0_icfg_addr_src0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src1 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC1_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC1_icfg_addr_src1_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC1_icfg_addr_src1_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src2 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC2_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC2_icfg_addr_src2_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC2_icfg_addr_src2_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src3 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC3_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC3_icfg_addr_src3_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC3_icfg_addr_src3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src4 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC4_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC4_icfg_addr_src4_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC4_icfg_addr_src4_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src5 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC5_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC5_icfg_addr_src5_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC5_icfg_addr_src5_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src6 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC6_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC6_icfg_addr_src6_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC6_icfg_addr_src6_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src7 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC7_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC7_icfg_addr_src7_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC7_icfg_addr_src7_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len0 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN0_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN0_icfg_addr_len0_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN0_icfg_addr_len0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len1 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN1_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN1_icfg_addr_len1_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN1_icfg_addr_len1_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len2 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN2_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN2_icfg_addr_len2_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN2_icfg_addr_len2_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len3 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN3_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN3_icfg_addr_len3_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN3_icfg_addr_len3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len4 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN4_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN4_icfg_addr_len4_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN4_icfg_addr_len4_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len5 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN5_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN5_icfg_addr_len5_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN5_icfg_addr_len5_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len6 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN6_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN6_icfg_addr_len6_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN6_icfg_addr_len6_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len7 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN7_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN7_icfg_addr_len7_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN7_icfg_addr_len7_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst0 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST0_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST0_icfg_addr_dst0_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST0_icfg_addr_dst0_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst1 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST1_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST1_icfg_addr_dst1_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST1_icfg_addr_dst1_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst2 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST2_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST2_icfg_addr_dst2_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST2_icfg_addr_dst2_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst3 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST3_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST3_icfg_addr_dst3_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST3_icfg_addr_dst3_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst4 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST4_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST4_icfg_addr_dst4_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST4_icfg_addr_dst4_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst5 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST5_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST5_icfg_addr_dst5_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST5_icfg_addr_dst5_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst6 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST6_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST6_icfg_addr_dst6_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST6_icfg_addr_dst6_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst7 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST7_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST7_icfg_addr_dst7_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST7_icfg_addr_dst7_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_default : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_ADDR_DEFAULT_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_ADDR_DEFAULT_icfg_addr_default_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_ADDR_DEFAULT_icfg_addr_default_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src8 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC8_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC8_icfg_addr_src8_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC8_icfg_addr_src8_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src9 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC9_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC9_icfg_addr_src9_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC9_icfg_addr_src9_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src10 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC10_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC10_icfg_addr_src10_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC10_icfg_addr_src10_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src11 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC11_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC11_icfg_addr_src11_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC11_icfg_addr_src11_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src12 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC12_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC12_icfg_addr_src12_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC12_icfg_addr_src12_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src13 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC13_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC13_icfg_addr_src13_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC13_icfg_addr_src13_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src14 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC14_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC14_icfg_addr_src14_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC14_icfg_addr_src14_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src15 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC15_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC15_icfg_addr_src15_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_SRC15_icfg_addr_src15_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len8 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN8_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN8_icfg_addr_len8_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN8_icfg_addr_len8_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len9 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN9_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN9_icfg_addr_len9_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN9_icfg_addr_len9_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len10 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN10_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN10_icfg_addr_len10_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN10_icfg_addr_len10_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len11 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN11_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN11_icfg_addr_len11_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN11_icfg_addr_len11_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len12 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN12_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN12_icfg_addr_len12_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN12_icfg_addr_len12_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len13 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN13_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN13_icfg_addr_len13_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN13_icfg_addr_len13_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len14 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN14_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN14_icfg_addr_len14_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN14_icfg_addr_len14_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len15 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN15_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN15_icfg_addr_len15_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_LEN15_icfg_addr_len15_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst8 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST8_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST8_icfg_addr_dst8_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST8_icfg_addr_dst8_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst9 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST9_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST9_icfg_addr_dst9_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST9_icfg_addr_dst9_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst10 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST10_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST10_icfg_addr_dst10_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST10_icfg_addr_dst10_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst11 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST11_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST11_icfg_addr_dst11_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST11_icfg_addr_dst11_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst12 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST12_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST12_icfg_addr_dst12_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST12_icfg_addr_dst12_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst13 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST13_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST13_icfg_addr_dst13_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST13_icfg_addr_dst13_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst14 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST14_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST14_icfg_addr_dst14_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST14_icfg_addr_dst14_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst15 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST15_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST15_icfg_addr_dst15_START (0)
#define SOC_NPU_TS_SYSCTRL_FAMA_REMAP_DST15_icfg_addr_dst15_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_prot_wr : 17;
        unsigned int reserved : 15;
    } reg;
} SOC_NPU_TS_SYSCTRL_DMMU_PROT_WR_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_DMMU_PROT_WR_icfg_prot_wr_START (0)
#define SOC_NPU_TS_SYSCTRL_DMMU_PROT_WR_icfg_prot_wr_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_prot_rd : 17;
        unsigned int reserved : 15;
    } reg;
} SOC_NPU_TS_SYSCTRL_DMMU_PROT_RD_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_DMMU_PROT_RD_icfg_prot_rd_START (0)
#define SOC_NPU_TS_SYSCTRL_DMMU_PROT_RD_icfg_prot_rd_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sample_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_ENABLE_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_ENABLE_sample_en_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_ENABLE_sample_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_compara_id : 12;
        unsigned int reserved_0 : 4;
        unsigned int w_compara_len : 4;
        unsigned int reserved_1 : 4;
        unsigned int w_compara_size : 3;
        unsigned int reserved_2 : 5;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_w_compara_id_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_w_compara_id_END (11)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_w_compara_len_START (16)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_w_compara_len_END (19)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_w_compara_size_START (24)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARA_w_compara_size_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_len_en : 1;
        unsigned int w_size_en : 1;
        unsigned int w_unalign_en : 1;
        unsigned int all_wstrb_1_en : 1;
        unsigned int w_interleaving_en : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_w_len_en_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_w_len_en_END (0)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_w_size_en_START (1)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_w_size_en_END (1)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_w_unalign_en_START (2)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_w_unalign_en_END (2)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_all_wstrb_1_en_START (3)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_all_wstrb_1_en_END (3)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_w_interleaving_en_START (4)
#define SOC_NPU_TS_SYSCTRL_MON_W_COMPPARAEN_w_interleaving_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_idmask : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_W_IDMASK_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_W_IDMASK_w_idmask_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_W_IDMASK_w_idmask_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int r_compara_id : 12;
        unsigned int reserved_0 : 4;
        unsigned int r_compara_len : 4;
        unsigned int reserved_1 : 4;
        unsigned int r_compara_size : 3;
        unsigned int reserved_2 : 5;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_r_compara_id_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_r_compara_id_END (11)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_r_compara_len_START (16)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_r_compara_len_END (19)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_r_compara_size_START (24)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARA_r_compara_size_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int r_len_en : 1;
        unsigned int r_size_en : 1;
        unsigned int r_unalign_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int r_interleaving_en : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_r_len_en_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_r_len_en_END (0)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_r_size_en_START (1)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_r_size_en_END (1)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_r_unalign_en_START (2)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_r_unalign_en_END (2)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_r_interleaving_en_START (4)
#define SOC_NPU_TS_SYSCTRL_MON_R_COMPPARAEN_r_interleaving_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int r_idmask : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_R_IDMASK_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_R_IDMASK_r_idmask_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_R_IDMASK_r_idmask_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_addr_base_low : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_START_ADDR_LOW_w_addr_base_low_START (0)
#define SOC_NPU_TS_SYSCTRL_START_ADDR_LOW_w_addr_base_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_addr_base_high : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_START_ADDR_HIGH_w_addr_base_high_START (0)
#define SOC_NPU_TS_SYSCTRL_START_ADDR_HIGH_w_addr_base_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_addr_end_low : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_END_ADDR_LOW_w_addr_end_low_START (0)
#define SOC_NPU_TS_SYSCTRL_END_ADDR_LOW_w_addr_end_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_addr_end_high : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_END_ADDR_HIGH_w_addr_end_high_START (0)
#define SOC_NPU_TS_SYSCTRL_END_ADDR_HIGH_w_addr_end_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_match_addr_l : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_W_ADDR_L_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_W_ADDR_L_w_match_addr_l_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_W_ADDR_L_w_match_addr_l_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int aw_match_cnt : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_AW_CNT_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_AW_CNT_aw_match_cnt_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_AW_CNT_aw_match_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_match_cnt : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_W_CNT_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_W_CNT_w_match_cnt_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_W_CNT_w_match_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int r_match_addr_l : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_R_ADDR_L_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_R_ADDR_L_r_match_addr_l_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_R_ADDR_L_r_match_addr_l_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ar_match_cnt : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_AR_CNT_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_AR_CNT_ar_match_cnt_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_AR_CNT_ar_match_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int r_match_cnt : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_R_CNT_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_R_CNT_r_match_cnt_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_R_CNT_r_match_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int w_match_addr_h : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_W_ADDR_H_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_W_ADDR_H_w_match_addr_h_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_W_ADDR_H_w_match_addr_h_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int r_match_addr_h : 32;
    } reg;
} SOC_NPU_TS_SYSCTRL_MON_R_ADDR_H_UNION;
#endif
#define SOC_NPU_TS_SYSCTRL_MON_R_ADDR_H_r_match_addr_h_START (0)
#define SOC_NPU_TS_SYSCTRL_MON_R_ADDR_H_r_match_addr_h_END (31)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
