#ifndef __SOC_HI_GPIO_V500_INTERFACE_H__
#define __SOC_HI_GPIO_V500_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_HI_GPIO_V500_DATA_REG_HIGH_ADDR(base) ((base) + (0x000UL))
#define SOC_HI_GPIO_V500_DATA_REG_LOW_ADDR(base) ((base) + (0x004UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_ADDR(base) ((base) + (0x008UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_ADDR(base) ((base) + (0x00CUL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_ADDR(base) ((base) + (0x010UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_ADDR(base) ((base) + (0x014UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_ADDR(base) ((base) + (0x018UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_ADDR(base) ((base) + (0x01CUL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_ADDR(base) ((base) + (0x020UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_ADDR(base) ((base) + (0x024UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_ADDR(base) ((base) + (0x028UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_ADDR(base) ((base) + (0x02CUL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_ADDR(base) ((base) + (0x030UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_ADDR(base) ((base) + (0x034UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_ADDR(base) ((base) + (0x038UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_ADDR(base) ((base) + (0x03CUL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_ADDR(base) ((base) + (0x040UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_ADDR(base) ((base) + (0x044UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_ADDR(base) ((base) + (0x048UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_ADDR(base) ((base) + (0x04CUL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_ADDR(base) ((base) + (0x050UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_ADDR(base) ((base) + (0x054UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_ADDR(base) ((base) + (0x058UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_ADDR(base) ((base) + (0x05CUL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_ADDR(base) ((base) + (0x060UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_ADDR(base) ((base) + (0x064UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_ADDR(base) ((base) + (0x068UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_ADDR(base) ((base) + (0x06CUL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_ADDR(base) ((base) + (0x070UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_ADDR(base) ((base) + (0x074UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_ADDR(base) ((base) + (0x078UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_ADDR(base) ((base) + (0x07CUL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_ADDR(base) ((base) + (0x080UL))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_ADDR(base) ((base) + (0x084UL))
#define SOC_HI_GPIO_V500_INTR_MSK_0_ADDR(base) ((base) + (0x088UL))
#define SOC_HI_GPIO_V500_INTR_MSK_1_ADDR(base) ((base) + (0x08CUL))
#define SOC_HI_GPIO_V500_INTR_MSK_2_ADDR(base) ((base) + (0x090UL))
#define SOC_HI_GPIO_V500_INTR_MSK_3_ADDR(base) ((base) + (0x094UL))
#define SOC_HI_GPIO_V500_INTR_MSK_4_ADDR(base) ((base) + (0x098UL))
#define SOC_HI_GPIO_V500_INTR_MSK_5_ADDR(base) ((base) + (0x09CUL))
#define SOC_HI_GPIO_V500_INTR_MSK_6_ADDR(base) ((base) + (0x0A0UL))
#define SOC_HI_GPIO_V500_INTR_MSK_7_ADDR(base) ((base) + (0x0A4UL))
#define SOC_HI_GPIO_V500_INTR_CRL_ADDR(base) ((base) + (0x0A8UL))
#define SOC_HI_GPIO_V500_INTR_RAW_STAT_ADDR(base) ((base) + (0x0ACUL))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_0_ADDR(base) ((base) + (0x0B0UL))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_1_ADDR(base) ((base) + (0x0B4UL))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_2_ADDR(base) ((base) + (0x0B8UL))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_3_ADDR(base) ((base) + (0x0BCUL))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_4_ADDR(base) ((base) + (0x0C0UL))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_5_ADDR(base) ((base) + (0x0C4UL))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_6_ADDR(base) ((base) + (0x0C8UL))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_7_ADDR(base) ((base) + (0x0CCUL))
#define SOC_HI_GPIO_V500_GPIO_IN_DATA_ADDR(base) ((base) + (0x0D0UL))
#define SOC_HI_GPIO_V500_GPIO_OUT_DATA_ADDR(base) ((base) + (0x0D4UL))
#define SOC_HI_GPIO_V500_GPIO_OEN_ADDR(base) ((base) + (0x0D8UL))
#define SOC_HI_GPIO_V500_SOURCE_DATA_ADDR(base) ((base) + (0x0DCUL))
#define SOC_HI_GPIO_V500_GPIO_ID_ADDR(base) ((base) + (0xFFCUL))
#else
#define SOC_HI_GPIO_V500_DATA_REG_HIGH_ADDR(base) ((base) + (0x000))
#define SOC_HI_GPIO_V500_DATA_REG_LOW_ADDR(base) ((base) + (0x004))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_ADDR(base) ((base) + (0x008))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_ADDR(base) ((base) + (0x00C))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_ADDR(base) ((base) + (0x010))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_ADDR(base) ((base) + (0x014))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_ADDR(base) ((base) + (0x018))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_ADDR(base) ((base) + (0x01C))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_ADDR(base) ((base) + (0x020))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_ADDR(base) ((base) + (0x024))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_ADDR(base) ((base) + (0x028))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_ADDR(base) ((base) + (0x02C))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_ADDR(base) ((base) + (0x030))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_ADDR(base) ((base) + (0x034))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_ADDR(base) ((base) + (0x038))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_ADDR(base) ((base) + (0x03C))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_ADDR(base) ((base) + (0x040))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_ADDR(base) ((base) + (0x044))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_ADDR(base) ((base) + (0x048))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_ADDR(base) ((base) + (0x04C))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_ADDR(base) ((base) + (0x050))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_ADDR(base) ((base) + (0x054))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_ADDR(base) ((base) + (0x058))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_ADDR(base) ((base) + (0x05C))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_ADDR(base) ((base) + (0x060))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_ADDR(base) ((base) + (0x064))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_ADDR(base) ((base) + (0x068))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_ADDR(base) ((base) + (0x06C))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_ADDR(base) ((base) + (0x070))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_ADDR(base) ((base) + (0x074))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_ADDR(base) ((base) + (0x078))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_ADDR(base) ((base) + (0x07C))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_ADDR(base) ((base) + (0x080))
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_ADDR(base) ((base) + (0x084))
#define SOC_HI_GPIO_V500_INTR_MSK_0_ADDR(base) ((base) + (0x088))
#define SOC_HI_GPIO_V500_INTR_MSK_1_ADDR(base) ((base) + (0x08C))
#define SOC_HI_GPIO_V500_INTR_MSK_2_ADDR(base) ((base) + (0x090))
#define SOC_HI_GPIO_V500_INTR_MSK_3_ADDR(base) ((base) + (0x094))
#define SOC_HI_GPIO_V500_INTR_MSK_4_ADDR(base) ((base) + (0x098))
#define SOC_HI_GPIO_V500_INTR_MSK_5_ADDR(base) ((base) + (0x09C))
#define SOC_HI_GPIO_V500_INTR_MSK_6_ADDR(base) ((base) + (0x0A0))
#define SOC_HI_GPIO_V500_INTR_MSK_7_ADDR(base) ((base) + (0x0A4))
#define SOC_HI_GPIO_V500_INTR_CRL_ADDR(base) ((base) + (0x0A8))
#define SOC_HI_GPIO_V500_INTR_RAW_STAT_ADDR(base) ((base) + (0x0AC))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_0_ADDR(base) ((base) + (0x0B0))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_1_ADDR(base) ((base) + (0x0B4))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_2_ADDR(base) ((base) + (0x0B8))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_3_ADDR(base) ((base) + (0x0BC))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_4_ADDR(base) ((base) + (0x0C0))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_5_ADDR(base) ((base) + (0x0C4))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_6_ADDR(base) ((base) + (0x0C8))
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_7_ADDR(base) ((base) + (0x0CC))
#define SOC_HI_GPIO_V500_GPIO_IN_DATA_ADDR(base) ((base) + (0x0D0))
#define SOC_HI_GPIO_V500_GPIO_OUT_DATA_ADDR(base) ((base) + (0x0D4))
#define SOC_HI_GPIO_V500_GPIO_OEN_ADDR(base) ((base) + (0x0D8))
#define SOC_HI_GPIO_V500_SOURCE_DATA_ADDR(base) ((base) + (0x0DC))
#define SOC_HI_GPIO_V500_GPIO_ID_ADDR(base) ((base) + (0xFFC))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int data_reg_high : 16;
        unsigned int gpio_data_msk1 : 16;
    } reg;
} SOC_HI_GPIO_V500_DATA_REG_HIGH_UNION;
#endif
#define SOC_HI_GPIO_V500_DATA_REG_HIGH_data_reg_high_START (0)
#define SOC_HI_GPIO_V500_DATA_REG_HIGH_data_reg_high_END (15)
#define SOC_HI_GPIO_V500_DATA_REG_HIGH_gpio_data_msk1_START (16)
#define SOC_HI_GPIO_V500_DATA_REG_HIGH_gpio_data_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int data_reg_low : 16;
        unsigned int gpio_data_msk1 : 16;
    } reg;
} SOC_HI_GPIO_V500_DATA_REG_LOW_UNION;
#endif
#define SOC_HI_GPIO_V500_DATA_REG_LOW_data_reg_low_START (0)
#define SOC_HI_GPIO_V500_DATA_REG_LOW_data_reg_low_END (15)
#define SOC_HI_GPIO_V500_DATA_REG_LOW_gpio_data_msk1_START (16)
#define SOC_HI_GPIO_V500_DATA_REG_LOW_gpio_data_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_0 : 1;
        unsigned int edge_type_0 : 2;
        unsigned int level_type_0 : 1;
        unsigned int hw_sw_sel_0 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT0_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_intr_trig_type_0_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_intr_trig_type_0_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_edge_type_0_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_edge_type_0_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_level_type_0_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_level_type_0_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_hw_sw_sel_0_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT0_hw_sw_sel_0_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_1 : 1;
        unsigned int edge_type_1 : 2;
        unsigned int level_type_1 : 1;
        unsigned int hw_sw_sel_1 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT1_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_intr_trig_type_1_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_intr_trig_type_1_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_edge_type_1_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_edge_type_1_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_level_type_1_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_level_type_1_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_hw_sw_sel_1_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT1_hw_sw_sel_1_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_2 : 1;
        unsigned int edge_type_2 : 2;
        unsigned int level_type_2 : 1;
        unsigned int hw_sw_sel_2 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT2_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_intr_trig_type_2_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_intr_trig_type_2_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_edge_type_2_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_edge_type_2_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_level_type_2_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_level_type_2_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_hw_sw_sel_2_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT2_hw_sw_sel_2_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_3 : 1;
        unsigned int edge_type_3 : 2;
        unsigned int level_type_3 : 1;
        unsigned int hw_sw_sel_3 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT3_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_intr_trig_type_3_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_intr_trig_type_3_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_edge_type_3_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_edge_type_3_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_level_type_3_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_level_type_3_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_hw_sw_sel_3_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT3_hw_sw_sel_3_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_4 : 1;
        unsigned int edge_type_4 : 2;
        unsigned int level_type_4 : 1;
        unsigned int hw_sw_sel_4 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT4_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_intr_trig_type_4_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_intr_trig_type_4_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_edge_type_4_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_edge_type_4_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_level_type_4_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_level_type_4_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_hw_sw_sel_4_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT4_hw_sw_sel_4_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_5 : 1;
        unsigned int edge_type_5 : 2;
        unsigned int level_type_5 : 1;
        unsigned int hw_sw_sel_5 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT5_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_intr_trig_type_5_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_intr_trig_type_5_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_edge_type_5_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_edge_type_5_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_level_type_5_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_level_type_5_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_hw_sw_sel_5_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT5_hw_sw_sel_5_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_6 : 1;
        unsigned int edge_type_6 : 2;
        unsigned int level_type_6 : 1;
        unsigned int hw_sw_sel_6 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT6_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_intr_trig_type_6_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_intr_trig_type_6_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_edge_type_6_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_edge_type_6_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_level_type_6_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_level_type_6_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_hw_sw_sel_6_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT6_hw_sw_sel_6_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_7 : 1;
        unsigned int edge_type_7 : 2;
        unsigned int level_type_7 : 1;
        unsigned int hw_sw_sel_7 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT7_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_intr_trig_type_7_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_intr_trig_type_7_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_edge_type_7_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_edge_type_7_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_level_type_7_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_level_type_7_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_hw_sw_sel_7_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT7_hw_sw_sel_7_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_8 : 1;
        unsigned int edge_type_8 : 2;
        unsigned int level_type_8 : 1;
        unsigned int hw_sw_sel_8 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT8_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_intr_trig_type_8_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_intr_trig_type_8_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_edge_type_8_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_edge_type_8_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_level_type_8_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_level_type_8_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_hw_sw_sel_8_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT8_hw_sw_sel_8_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_9 : 1;
        unsigned int edge_type_9 : 2;
        unsigned int level_type_9 : 1;
        unsigned int hw_sw_sel_9 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT9_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_intr_trig_type_9_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_intr_trig_type_9_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_edge_type_9_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_edge_type_9_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_level_type_9_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_level_type_9_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_hw_sw_sel_9_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT9_hw_sw_sel_9_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_10 : 1;
        unsigned int edge_type_10 : 2;
        unsigned int level_type_10 : 1;
        unsigned int hw_sw_sel_10 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT10_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_intr_trig_type_10_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_intr_trig_type_10_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_edge_type_10_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_edge_type_10_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_level_type_10_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_level_type_10_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_hw_sw_sel_10_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT10_hw_sw_sel_10_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_11 : 1;
        unsigned int edge_type_11 : 2;
        unsigned int level_type_11 : 1;
        unsigned int hw_sw_sel_11 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT11_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_intr_trig_type_11_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_intr_trig_type_11_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_edge_type_11_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_edge_type_11_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_level_type_11_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_level_type_11_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_hw_sw_sel_11_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT11_hw_sw_sel_11_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_12 : 1;
        unsigned int edge_type_12 : 2;
        unsigned int level_type_12 : 1;
        unsigned int hw_sw_sel_12 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT12_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_intr_trig_type_12_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_intr_trig_type_12_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_edge_type_12_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_edge_type_12_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_level_type_12_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_level_type_12_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_hw_sw_sel_12_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT12_hw_sw_sel_12_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_13 : 1;
        unsigned int edge_type_13 : 2;
        unsigned int level_type_13 : 1;
        unsigned int hw_sw_sel_13 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT13_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_intr_trig_type_13_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_intr_trig_type_13_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_edge_type_13_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_edge_type_13_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_level_type_13_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_level_type_13_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_hw_sw_sel_13_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT13_hw_sw_sel_13_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_14 : 1;
        unsigned int edge_type_14 : 2;
        unsigned int level_type_14 : 1;
        unsigned int hw_sw_sel_14 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT14_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_intr_trig_type_14_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_intr_trig_type_14_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_edge_type_14_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_edge_type_14_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_level_type_14_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_level_type_14_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_hw_sw_sel_14_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT14_hw_sw_sel_14_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_15 : 1;
        unsigned int edge_type_15 : 2;
        unsigned int level_type_15 : 1;
        unsigned int hw_sw_sel_15 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT15_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_intr_trig_type_15_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_intr_trig_type_15_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_edge_type_15_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_edge_type_15_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_level_type_15_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_level_type_15_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_hw_sw_sel_15_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT15_hw_sw_sel_15_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_16 : 1;
        unsigned int edge_type_16 : 2;
        unsigned int level_type_16 : 1;
        unsigned int hw_sw_sel_16 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT16_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_intr_trig_type_16_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_intr_trig_type_16_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_edge_type_16_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_edge_type_16_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_level_type_16_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_level_type_16_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_hw_sw_sel_16_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT16_hw_sw_sel_16_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_17 : 1;
        unsigned int edge_type_17 : 2;
        unsigned int level_type_17 : 1;
        unsigned int hw_sw_sel_17 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT17_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_intr_trig_type_17_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_intr_trig_type_17_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_edge_type_17_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_edge_type_17_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_level_type_17_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_level_type_17_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_hw_sw_sel_17_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT17_hw_sw_sel_17_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_18 : 1;
        unsigned int edge_type_18 : 2;
        unsigned int level_type_18 : 1;
        unsigned int hw_sw_sel_18 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT18_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_intr_trig_type_18_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_intr_trig_type_18_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_edge_type_18_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_edge_type_18_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_level_type_18_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_level_type_18_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_hw_sw_sel_18_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT18_hw_sw_sel_18_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_19 : 1;
        unsigned int edge_type_19 : 2;
        unsigned int level_type_19 : 1;
        unsigned int hw_sw_sel_19 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT19_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_intr_trig_type_19_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_intr_trig_type_19_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_edge_type_19_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_edge_type_19_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_level_type_19_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_level_type_19_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_hw_sw_sel_19_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT19_hw_sw_sel_19_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_20 : 1;
        unsigned int edge_type_20 : 2;
        unsigned int level_type_20 : 1;
        unsigned int hw_sw_sel_20 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT20_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_intr_trig_type_20_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_intr_trig_type_20_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_edge_type_20_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_edge_type_20_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_level_type_20_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_level_type_20_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_hw_sw_sel_20_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT20_hw_sw_sel_20_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_21 : 1;
        unsigned int edge_type_21 : 2;
        unsigned int level_type_21 : 1;
        unsigned int hw_sw_sel_21 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT21_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_intr_trig_type_21_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_intr_trig_type_21_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_edge_type_21_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_edge_type_21_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_level_type_21_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_level_type_21_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_hw_sw_sel_21_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT21_hw_sw_sel_21_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_22 : 1;
        unsigned int edge_type_22 : 2;
        unsigned int level_type_22 : 1;
        unsigned int hw_sw_sel_22 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT22_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_intr_trig_type_22_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_intr_trig_type_22_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_edge_type_22_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_edge_type_22_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_level_type_22_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_level_type_22_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_hw_sw_sel_22_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT22_hw_sw_sel_22_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_23 : 1;
        unsigned int edge_type_23 : 2;
        unsigned int level_type_23 : 1;
        unsigned int hw_sw_sel_23 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT23_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_intr_trig_type_23_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_intr_trig_type_23_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_edge_type_23_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_edge_type_23_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_level_type_23_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_level_type_23_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_hw_sw_sel_23_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT23_hw_sw_sel_23_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_24 : 1;
        unsigned int edge_type_24 : 2;
        unsigned int level_type_24 : 1;
        unsigned int hw_sw_sel_24 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT24_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_intr_trig_type_24_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_intr_trig_type_24_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_edge_type_24_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_edge_type_24_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_level_type_24_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_level_type_24_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_hw_sw_sel_24_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT24_hw_sw_sel_24_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_25 : 1;
        unsigned int edge_type_25 : 2;
        unsigned int level_type_25 : 1;
        unsigned int hw_sw_sel_25 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT25_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_intr_trig_type_25_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_intr_trig_type_25_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_edge_type_25_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_edge_type_25_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_level_type_25_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_level_type_25_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_hw_sw_sel_25_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT25_hw_sw_sel_25_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_26 : 1;
        unsigned int edge_type_26 : 2;
        unsigned int level_type_26 : 1;
        unsigned int hw_sw_sel_26 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT26_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_intr_trig_type_26_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_intr_trig_type_26_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_edge_type_26_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_edge_type_26_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_level_type_26_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_level_type_26_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_hw_sw_sel_26_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT26_hw_sw_sel_26_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_27 : 1;
        unsigned int edge_type_27 : 2;
        unsigned int level_type_27 : 1;
        unsigned int hw_sw_sel_27 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT27_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_intr_trig_type_27_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_intr_trig_type_27_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_edge_type_27_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_edge_type_27_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_level_type_27_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_level_type_27_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_hw_sw_sel_27_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT27_hw_sw_sel_27_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_28 : 1;
        unsigned int edge_type_28 : 2;
        unsigned int level_type_28 : 1;
        unsigned int hw_sw_sel_28 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT28_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_intr_trig_type_28_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_intr_trig_type_28_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_edge_type_28_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_edge_type_28_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_level_type_28_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_level_type_28_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_hw_sw_sel_28_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT28_hw_sw_sel_28_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_29 : 1;
        unsigned int edge_type_29 : 2;
        unsigned int level_type_29 : 1;
        unsigned int hw_sw_sel_29 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT29_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_intr_trig_type_29_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_intr_trig_type_29_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_edge_type_29_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_edge_type_29_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_level_type_29_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_level_type_29_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_hw_sw_sel_29_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT29_hw_sw_sel_29_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_30 : 1;
        unsigned int edge_type_30 : 2;
        unsigned int level_type_30 : 1;
        unsigned int hw_sw_sel_30 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT30_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_intr_trig_type_30_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_intr_trig_type_30_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_edge_type_30_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_edge_type_30_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_level_type_30_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_level_type_30_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_hw_sw_sel_30_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT30_hw_sw_sel_30_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_trig_type_31 : 1;
        unsigned int edge_type_31 : 2;
        unsigned int level_type_31 : 1;
        unsigned int hw_sw_sel_31 : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_HI_GPIO_V500_INTR_CTRL_BIT31_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_intr_trig_type_31_START (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_intr_trig_type_31_END (0)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_edge_type_31_START (1)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_edge_type_31_END (2)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_level_type_31_START (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_level_type_31_END (3)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_hw_sw_sel_31_START (4)
#define SOC_HI_GPIO_V500_INTR_CTRL_BIT31_hw_sw_sel_31_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_msk_0 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_MSK_0_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_MSK_0_intr_msk_0_START (0)
#define SOC_HI_GPIO_V500_INTR_MSK_0_intr_msk_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_msk_1 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_MSK_1_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_MSK_1_intr_msk_1_START (0)
#define SOC_HI_GPIO_V500_INTR_MSK_1_intr_msk_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_msk_2 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_MSK_2_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_MSK_2_intr_msk_2_START (0)
#define SOC_HI_GPIO_V500_INTR_MSK_2_intr_msk_2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_msk_3 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_MSK_3_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_MSK_3_intr_msk_3_START (0)
#define SOC_HI_GPIO_V500_INTR_MSK_3_intr_msk_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_msk_4 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_MSK_4_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_MSK_4_intr_msk_4_START (0)
#define SOC_HI_GPIO_V500_INTR_MSK_4_intr_msk_4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_msk_5 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_MSK_5_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_MSK_5_intr_msk_5_START (0)
#define SOC_HI_GPIO_V500_INTR_MSK_5_intr_msk_5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_msk_6 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_MSK_6_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_MSK_6_intr_msk_6_START (0)
#define SOC_HI_GPIO_V500_INTR_MSK_6_intr_msk_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_msk_7 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_MSK_7_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_MSK_7_intr_msk_7_START (0)
#define SOC_HI_GPIO_V500_INTR_MSK_7_intr_msk_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_clr_0 : 1;
        unsigned int intr_clr_1 : 1;
        unsigned int intr_clr_2 : 1;
        unsigned int intr_clr_3 : 1;
        unsigned int intr_clr_4 : 1;
        unsigned int intr_clr_5 : 1;
        unsigned int intr_clr_6 : 1;
        unsigned int intr_clr_7 : 1;
        unsigned int intr_clr_8 : 1;
        unsigned int intr_clr_9 : 1;
        unsigned int intr_clr_10 : 1;
        unsigned int intr_clr_11 : 1;
        unsigned int intr_clr_12 : 1;
        unsigned int intr_clr_13 : 1;
        unsigned int intr_clr_14 : 1;
        unsigned int intr_clr_15 : 1;
        unsigned int intr_clr_16 : 1;
        unsigned int intr_clr_17 : 1;
        unsigned int intr_clr_18 : 1;
        unsigned int intr_clr_19 : 1;
        unsigned int intr_clr_20 : 1;
        unsigned int intr_clr_21 : 1;
        unsigned int intr_clr_22 : 1;
        unsigned int intr_clr_23 : 1;
        unsigned int intr_clr_24 : 1;
        unsigned int intr_clr_25 : 1;
        unsigned int intr_clr_26 : 1;
        unsigned int intr_clr_27 : 1;
        unsigned int intr_clr_28 : 1;
        unsigned int intr_clr_29 : 1;
        unsigned int intr_clr_30 : 1;
        unsigned int intr_clr_31 : 1;
    } reg;
} SOC_HI_GPIO_V500_INTR_CRL_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_0_START (0)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_0_END (0)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_1_START (1)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_1_END (1)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_2_START (2)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_2_END (2)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_3_START (3)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_3_END (3)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_4_START (4)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_4_END (4)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_5_START (5)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_5_END (5)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_6_START (6)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_6_END (6)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_7_START (7)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_7_END (7)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_8_START (8)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_8_END (8)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_9_START (9)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_9_END (9)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_10_START (10)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_10_END (10)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_11_START (11)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_11_END (11)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_12_START (12)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_12_END (12)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_13_START (13)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_13_END (13)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_14_START (14)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_14_END (14)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_15_START (15)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_15_END (15)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_16_START (16)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_16_END (16)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_17_START (17)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_17_END (17)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_18_START (18)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_18_END (18)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_19_START (19)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_19_END (19)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_20_START (20)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_20_END (20)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_21_START (21)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_21_END (21)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_22_START (22)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_22_END (22)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_23_START (23)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_23_END (23)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_24_START (24)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_24_END (24)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_25_START (25)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_25_END (25)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_26_START (26)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_26_END (26)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_27_START (27)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_27_END (27)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_28_START (28)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_28_END (28)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_29_START (29)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_29_END (29)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_30_START (30)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_30_END (30)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_31_START (31)
#define SOC_HI_GPIO_V500_INTR_CRL_intr_clr_31_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_raw_stat : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_RAW_STAT_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_RAW_STAT_intr_raw_stat_START (0)
#define SOC_HI_GPIO_V500_INTR_RAW_STAT_intr_raw_stat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gpio_stat_0 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_GPIO_STAT_0_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_0_intr_gpio_stat_0_START (0)
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_0_intr_gpio_stat_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gpio_stat_1 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_GPIO_STAT_1_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_1_intr_gpio_stat_1_START (0)
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_1_intr_gpio_stat_1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gpio_stat_2 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_GPIO_STAT_2_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_2_intr_gpio_stat_2_START (0)
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_2_intr_gpio_stat_2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gpio_stat_3 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_GPIO_STAT_3_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_3_intr_gpio_stat_3_START (0)
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_3_intr_gpio_stat_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gpio_stat_4 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_GPIO_STAT_4_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_4_intr_gpio_stat_4_START (0)
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_4_intr_gpio_stat_4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gpio_stat_5 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_GPIO_STAT_5_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_5_intr_gpio_stat_5_START (0)
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_5_intr_gpio_stat_5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gpio_stat_6 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_GPIO_STAT_6_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_6_intr_gpio_stat_6_START (0)
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_6_intr_gpio_stat_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gpio_stat_7 : 32;
    } reg;
} SOC_HI_GPIO_V500_INTR_GPIO_STAT_7_UNION;
#endif
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_7_intr_gpio_stat_7_START (0)
#define SOC_HI_GPIO_V500_INTR_GPIO_STAT_7_intr_gpio_stat_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpio_in_data : 32;
    } reg;
} SOC_HI_GPIO_V500_GPIO_IN_DATA_UNION;
#endif
#define SOC_HI_GPIO_V500_GPIO_IN_DATA_gpio_in_data_START (0)
#define SOC_HI_GPIO_V500_GPIO_IN_DATA_gpio_in_data_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpio_out_data : 32;
    } reg;
} SOC_HI_GPIO_V500_GPIO_OUT_DATA_UNION;
#endif
#define SOC_HI_GPIO_V500_GPIO_OUT_DATA_gpio_out_data_START (0)
#define SOC_HI_GPIO_V500_GPIO_OUT_DATA_gpio_out_data_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpio_oen : 32;
    } reg;
} SOC_HI_GPIO_V500_GPIO_OEN_UNION;
#endif
#define SOC_HI_GPIO_V500_GPIO_OEN_gpio_oen_START (0)
#define SOC_HI_GPIO_V500_GPIO_OEN_gpio_oen_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int source_data : 32;
    } reg;
} SOC_HI_GPIO_V500_SOURCE_DATA_UNION;
#endif
#define SOC_HI_GPIO_V500_SOURCE_DATA_source_data_START (0)
#define SOC_HI_GPIO_V500_SOURCE_DATA_source_data_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gpio_id : 32;
    } reg;
} SOC_HI_GPIO_V500_GPIO_ID_UNION;
#endif
#define SOC_HI_GPIO_V500_GPIO_ID_gpio_id_START (0)
#define SOC_HI_GPIO_V500_GPIO_ID_gpio_id_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
