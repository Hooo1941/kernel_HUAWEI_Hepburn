#ifndef __SOC_NPU_AIC_DMMU_INTERFACE_H__
#define __SOC_NPU_AIC_DMMU_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC0_ADDR(base) ((base) + (0x800UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC1_ADDR(base) ((base) + (0x804UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC2_ADDR(base) ((base) + (0x808UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC3_ADDR(base) ((base) + (0x80CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC4_ADDR(base) ((base) + (0x810UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC5_ADDR(base) ((base) + (0x814UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC6_ADDR(base) ((base) + (0x818UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC7_ADDR(base) ((base) + (0x81CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN0_ADDR(base) ((base) + (0x820UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN1_ADDR(base) ((base) + (0x824UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN2_ADDR(base) ((base) + (0x828UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN3_ADDR(base) ((base) + (0x82CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN4_ADDR(base) ((base) + (0x830UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN5_ADDR(base) ((base) + (0x834UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN6_ADDR(base) ((base) + (0x838UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN7_ADDR(base) ((base) + (0x83CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST0_ADDR(base) ((base) + (0x840UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST1_ADDR(base) ((base) + (0x844UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST2_ADDR(base) ((base) + (0x848UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST3_ADDR(base) ((base) + (0x84CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST4_ADDR(base) ((base) + (0x850UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST5_ADDR(base) ((base) + (0x854UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST6_ADDR(base) ((base) + (0x858UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST7_ADDR(base) ((base) + (0x85CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_ADDR_DEFAULT_ADDR(base) ((base) + (0x860UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC8_ADDR(base) ((base) + (0x900UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC9_ADDR(base) ((base) + (0x904UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC10_ADDR(base) ((base) + (0x908UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC11_ADDR(base) ((base) + (0x90cUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC12_ADDR(base) ((base) + (0x910UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC13_ADDR(base) ((base) + (0x914UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC14_ADDR(base) ((base) + (0x918UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC15_ADDR(base) ((base) + (0x91cUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN8_ADDR(base) ((base) + (0x920UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN9_ADDR(base) ((base) + (0x924UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN10_ADDR(base) ((base) + (0x928UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN11_ADDR(base) ((base) + (0x92CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN12_ADDR(base) ((base) + (0x930UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN13_ADDR(base) ((base) + (0x934UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN14_ADDR(base) ((base) + (0x938UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN15_ADDR(base) ((base) + (0x93CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST8_ADDR(base) ((base) + (0x940UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST9_ADDR(base) ((base) + (0x944UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST10_ADDR(base) ((base) + (0x948UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST11_ADDR(base) ((base) + (0x94CUL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST12_ADDR(base) ((base) + (0x950UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST13_ADDR(base) ((base) + (0x954UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST14_ADDR(base) ((base) + (0x958UL))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST15_ADDR(base) ((base) + (0x95CUL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_ADDR(base) ((base) + (0x960UL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_ARADDR_ADDR(base) ((base) + (0x964UL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU0_ADDR(base) ((base) + (0x968UL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU1_ADDR(base) ((base) + (0x96CUL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_ADDR(base) ((base) + (0x970UL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_AWADDR_ADDR(base) ((base) + (0x974UL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU0_ADDR(base) ((base) + (0x978UL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU1_ADDR(base) ((base) + (0x97CUL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_INTR_ADDR(base) ((base) + (0x980UL))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_INFO_CLEAR_ADDR(base) ((base) + (0x984UL))
#else
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC0_ADDR(base) ((base) + (0x800))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC1_ADDR(base) ((base) + (0x804))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC2_ADDR(base) ((base) + (0x808))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC3_ADDR(base) ((base) + (0x80C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC4_ADDR(base) ((base) + (0x810))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC5_ADDR(base) ((base) + (0x814))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC6_ADDR(base) ((base) + (0x818))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC7_ADDR(base) ((base) + (0x81C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN0_ADDR(base) ((base) + (0x820))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN1_ADDR(base) ((base) + (0x824))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN2_ADDR(base) ((base) + (0x828))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN3_ADDR(base) ((base) + (0x82C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN4_ADDR(base) ((base) + (0x830))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN5_ADDR(base) ((base) + (0x834))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN6_ADDR(base) ((base) + (0x838))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN7_ADDR(base) ((base) + (0x83C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST0_ADDR(base) ((base) + (0x840))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST1_ADDR(base) ((base) + (0x844))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST2_ADDR(base) ((base) + (0x848))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST3_ADDR(base) ((base) + (0x84C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST4_ADDR(base) ((base) + (0x850))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST5_ADDR(base) ((base) + (0x854))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST6_ADDR(base) ((base) + (0x858))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST7_ADDR(base) ((base) + (0x85C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_ADDR_DEFAULT_ADDR(base) ((base) + (0x860))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC8_ADDR(base) ((base) + (0x900))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC9_ADDR(base) ((base) + (0x904))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC10_ADDR(base) ((base) + (0x908))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC11_ADDR(base) ((base) + (0x90c))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC12_ADDR(base) ((base) + (0x910))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC13_ADDR(base) ((base) + (0x914))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC14_ADDR(base) ((base) + (0x918))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC15_ADDR(base) ((base) + (0x91c))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN8_ADDR(base) ((base) + (0x920))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN9_ADDR(base) ((base) + (0x924))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN10_ADDR(base) ((base) + (0x928))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN11_ADDR(base) ((base) + (0x92C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN12_ADDR(base) ((base) + (0x930))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN13_ADDR(base) ((base) + (0x934))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN14_ADDR(base) ((base) + (0x938))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN15_ADDR(base) ((base) + (0x93C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST8_ADDR(base) ((base) + (0x940))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST9_ADDR(base) ((base) + (0x944))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST10_ADDR(base) ((base) + (0x948))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST11_ADDR(base) ((base) + (0x94C))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST12_ADDR(base) ((base) + (0x950))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST13_ADDR(base) ((base) + (0x954))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST14_ADDR(base) ((base) + (0x958))
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST15_ADDR(base) ((base) + (0x95C))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_ADDR(base) ((base) + (0x960))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_ARADDR_ADDR(base) ((base) + (0x964))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU0_ADDR(base) ((base) + (0x968))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU1_ADDR(base) ((base) + (0x96C))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_ADDR(base) ((base) + (0x970))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_AWADDR_ADDR(base) ((base) + (0x974))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU0_ADDR(base) ((base) + (0x978))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU1_ADDR(base) ((base) + (0x97C))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_INTR_ADDR(base) ((base) + (0x980))
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_INFO_CLEAR_ADDR(base) ((base) + (0x984))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src0 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC0_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC0_icfg_addr_src0_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC0_icfg_addr_src0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src1 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC1_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC1_icfg_addr_src1_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC1_icfg_addr_src1_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src2 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC2_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC2_icfg_addr_src2_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC2_icfg_addr_src2_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src3 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC3_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC3_icfg_addr_src3_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC3_icfg_addr_src3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src4 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC4_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC4_icfg_addr_src4_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC4_icfg_addr_src4_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src5 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC5_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC5_icfg_addr_src5_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC5_icfg_addr_src5_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src6 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC6_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC6_icfg_addr_src6_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC6_icfg_addr_src6_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src7 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC7_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC7_icfg_addr_src7_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC7_icfg_addr_src7_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len0 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN0_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN0_icfg_addr_len0_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN0_icfg_addr_len0_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len1 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN1_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN1_icfg_addr_len1_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN1_icfg_addr_len1_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len2 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN2_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN2_icfg_addr_len2_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN2_icfg_addr_len2_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len3 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN3_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN3_icfg_addr_len3_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN3_icfg_addr_len3_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len4 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN4_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN4_icfg_addr_len4_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN4_icfg_addr_len4_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len5 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN5_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN5_icfg_addr_len5_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN5_icfg_addr_len5_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len6 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN6_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN6_icfg_addr_len6_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN6_icfg_addr_len6_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len7 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN7_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN7_icfg_addr_len7_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN7_icfg_addr_len7_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst0 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST0_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST0_icfg_addr_dst0_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST0_icfg_addr_dst0_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst1 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST1_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST1_icfg_addr_dst1_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST1_icfg_addr_dst1_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst2 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST2_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST2_icfg_addr_dst2_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST2_icfg_addr_dst2_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst3 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST3_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST3_icfg_addr_dst3_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST3_icfg_addr_dst3_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst4 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST4_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST4_icfg_addr_dst4_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST4_icfg_addr_dst4_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst5 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST5_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST5_icfg_addr_dst5_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST5_icfg_addr_dst5_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst6 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST6_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST6_icfg_addr_dst6_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST6_icfg_addr_dst6_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst7 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST7_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST7_icfg_addr_dst7_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST7_icfg_addr_dst7_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_default : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_ADDR_DEFAULT_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_ADDR_DEFAULT_icfg_addr_default_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_ADDR_DEFAULT_icfg_addr_default_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src8 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC8_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC8_icfg_addr_src8_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC8_icfg_addr_src8_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src9 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC9_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC9_icfg_addr_src9_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC9_icfg_addr_src9_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src10 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC10_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC10_icfg_addr_src10_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC10_icfg_addr_src10_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src11 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC11_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC11_icfg_addr_src11_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC11_icfg_addr_src11_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src12 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC12_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC12_icfg_addr_src12_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC12_icfg_addr_src12_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src13 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC13_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC13_icfg_addr_src13_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC13_icfg_addr_src13_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src14 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC14_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC14_icfg_addr_src14_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC14_icfg_addr_src14_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_src15 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC15_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC15_icfg_addr_src15_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_SRC15_icfg_addr_src15_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len8 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN8_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN8_icfg_addr_len8_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN8_icfg_addr_len8_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len9 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN9_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN9_icfg_addr_len9_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN9_icfg_addr_len9_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len10 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN10_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN10_icfg_addr_len10_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN10_icfg_addr_len10_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len11 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN11_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN11_icfg_addr_len11_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN11_icfg_addr_len11_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len12 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN12_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN12_icfg_addr_len12_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN12_icfg_addr_len12_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len13 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN13_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN13_icfg_addr_len13_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN13_icfg_addr_len13_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len14 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN14_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN14_icfg_addr_len14_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN14_icfg_addr_len14_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_len15 : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN15_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN15_icfg_addr_len15_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_LEN15_icfg_addr_len15_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst8 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST8_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST8_icfg_addr_dst8_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST8_icfg_addr_dst8_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst9 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST9_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST9_icfg_addr_dst9_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST9_icfg_addr_dst9_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst10 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST10_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST10_icfg_addr_dst10_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST10_icfg_addr_dst10_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst11 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST11_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST11_icfg_addr_dst11_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST11_icfg_addr_dst11_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst12 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST12_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST12_icfg_addr_dst12_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST12_icfg_addr_dst12_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst13 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST13_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST13_icfg_addr_dst13_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST13_icfg_addr_dst13_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst14 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST14_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST14_icfg_addr_dst14_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST14_icfg_addr_dst14_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_addr_dst15 : 28;
        unsigned int reserved : 4;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_REMAP_DST15_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST15_icfg_addr_dst15_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_REMAP_DST15_icfg_addr_dst15_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_rd_intr : 1;
        unsigned int icfg_wrong_arprot : 3;
        unsigned int icfg_wrong_arcache : 4;
        unsigned int icfg_wrong_arlock : 2;
        unsigned int icfg_wrong_arburst : 2;
        unsigned int icfg_wrong_arsize : 3;
        unsigned int icfg_wrong_arlen : 4;
        unsigned int icfg_wrong_arid : 12;
        unsigned int reserved : 1;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_rd_intr_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_rd_intr_END (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arprot_START (1)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arprot_END (3)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arcache_START (4)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arcache_END (7)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arlock_START (8)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arlock_END (9)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arburst_START (10)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arburst_END (11)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arsize_START (12)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arsize_END (14)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arlen_START (15)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arlen_END (18)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arid_START (19)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AR_INFOR_icfg_wrong_arid_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_ori_araddr : 32;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_ARADDR_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_ARADDR_icfg_wrong_ori_araddr_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_ARADDR_icfg_wrong_ori_araddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_o_araddr_dmmu0 : 32;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU0_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU0_icfg_wrong_o_araddr_dmmu0_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU0_icfg_wrong_o_araddr_dmmu0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_o_araddr_dmmu1 : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU1_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU1_icfg_wrong_o_araddr_dmmu1_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_ARADDR_DMMU1_icfg_wrong_o_araddr_dmmu1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_wr_intr : 1;
        unsigned int icfg_wrong_awprot : 3;
        unsigned int icfg_wrong_awcache : 4;
        unsigned int icfg_wrong_awlock : 2;
        unsigned int icfg_wrong_awburst : 2;
        unsigned int icfg_wrong_awsize : 3;
        unsigned int icfg_wrong_awlen : 4;
        unsigned int icfg_wrong_awid : 12;
        unsigned int reserved : 1;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_wr_intr_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_wr_intr_END (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awprot_START (1)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awprot_END (3)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awcache_START (4)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awcache_END (7)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awlock_START (8)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awlock_END (9)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awburst_START (10)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awburst_END (11)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awsize_START (12)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awsize_END (14)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awlen_START (15)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awlen_END (18)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awid_START (19)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_AW_INFOR_icfg_wrong_awid_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_ori_awaddr : 32;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_AWADDR_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_AWADDR_icfg_wrong_ori_awaddr_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_ORI_AWADDR_icfg_wrong_ori_awaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_o_awaddr_dmmu0 : 32;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU0_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU0_icfg_wrong_o_awaddr_dmmu0_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU0_icfg_wrong_o_awaddr_dmmu0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_o_awaddr_dmmu1 : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU1_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU1_icfg_wrong_o_awaddr_dmmu1_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_O_AWADDR_DMMU1_icfg_wrong_o_awaddr_dmmu1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int icfg_wrong_o_dmmu_intr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_INTR_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_INTR_icfg_wrong_o_dmmu_intr_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_INTR_icfg_wrong_o_dmmu_intr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned char value;
    struct {
        unsigned char icfg_wrong_info_clear : 1;
    } reg;
} SOC_NPU_AIC_DMMU_FAMA_WRONG_INFO_CLEAR_UNION;
#endif
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_INFO_CLEAR_icfg_wrong_info_clear_START (0)
#define SOC_NPU_AIC_DMMU_FAMA_WRONG_INFO_CLEAR_icfg_wrong_info_clear_END (0)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
