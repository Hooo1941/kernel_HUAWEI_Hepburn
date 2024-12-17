#ifndef __SOC_INTR_HUB_INTERFACE_H__
#define __SOC_INTR_HUB_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_INTR_HUB_L2_SEC_INFO_n_ADDR(base,n) ((base) + ((n)*0x10+0x0UL))
#define SOC_INTR_HUB_L2_INTR_MASKSET_S_n_ADDR(base,n) ((base) + ((n)*0x10+0x4UL))
#define SOC_INTR_HUB_L2_INTR_MASKCLR_S_n_ADDR(base,n) ((base) + ((n)*0x10+0x8UL))
#define SOC_INTR_HUB_L2_INTR_STATUS_S_n_ADDR(base,n) ((base) + ((n)*0x10+0xCUL))
#define SOC_INTR_HUB_CTRL_ADDR(base) ((base) + (0x03fcUL))
#else
#define SOC_INTR_HUB_L2_SEC_INFO_n_ADDR(base,n) ((base) + ((n)*0x10+0x0))
#define SOC_INTR_HUB_L2_INTR_MASKSET_S_n_ADDR(base,n) ((base) + ((n)*0x10+0x4))
#define SOC_INTR_HUB_L2_INTR_MASKCLR_S_n_ADDR(base,n) ((base) + ((n)*0x10+0x8))
#define SOC_INTR_HUB_L2_INTR_STATUS_S_n_ADDR(base,n) ((base) + ((n)*0x10+0xC))
#define SOC_INTR_HUB_CTRL_ADDR(base) ((base) + (0x03fc))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int level2_reg_sec_info : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_INTR_HUB_L2_SEC_INFO_n_UNION;
#endif
#define SOC_INTR_HUB_L2_SEC_INFO_n_level2_reg_sec_info_START (0)
#define SOC_INTR_HUB_L2_SEC_INFO_n_level2_reg_sec_info_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int level2_intr_maskset : 32;
    } reg;
} SOC_INTR_HUB_L2_INTR_MASKSET_S_n_UNION;
#endif
#define SOC_INTR_HUB_L2_INTR_MASKSET_S_n_level2_intr_maskset_START (0)
#define SOC_INTR_HUB_L2_INTR_MASKSET_S_n_level2_intr_maskset_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int level2_intr_maskclr : 32;
    } reg;
} SOC_INTR_HUB_L2_INTR_MASKCLR_S_n_UNION;
#endif
#define SOC_INTR_HUB_L2_INTR_MASKCLR_S_n_level2_intr_maskclr_START (0)
#define SOC_INTR_HUB_L2_INTR_MASKCLR_S_n_level2_intr_maskclr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int level2_intr_status : 32;
    } reg;
} SOC_INTR_HUB_L2_INTR_STATUS_S_n_UNION;
#endif
#define SOC_INTR_HUB_L2_INTR_STATUS_S_n_level2_intr_status_START (0)
#define SOC_INTR_HUB_L2_INTR_STATUS_S_n_level2_intr_status_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_gt_ctrl : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_INTR_HUB_CTRL_UNION;
#endif
#define SOC_INTR_HUB_CTRL_clk_gt_ctrl_START (0)
#define SOC_INTR_HUB_CTRL_clk_gt_ctrl_END (1)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
