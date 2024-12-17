#ifndef __SOC_CPU_PDC_INTERFACE_H__
#define __SOC_CPU_PDC_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_CPU_PDC_PDCCTRL_ADDR(base) ((base) + (0x000UL))
#define SOC_CPU_PDC_ISOEN_ADDR(base) ((base) + (0x100UL))
#define SOC_CPU_PDC_ISODIS_ADDR(base) ((base) + (0x104UL))
#define SOC_CPU_PDC_ISOSTAT_ADDR(base) ((base) + (0x108UL))
#define SOC_CPU_PDC_PERPWREN_ADDR(base) ((base) + (0x110UL))
#define SOC_CPU_PDC_PERPWRDIS_ADDR(base) ((base) + (0x114UL))
#define SOC_CPU_PDC_PERPWRSTAT_ADDR(base) ((base) + (0x118UL))
#define SOC_CPU_PDC_PERPWRACK_ADDR(base) ((base) + (0x11CUL))
#define SOC_CPU_PDC_CPU_CLKEN_ADDR(base) ((base) + (0x120UL))
#define SOC_CPU_PDC_CPU_RSTEN_ADDR(base) ((base) + (0x124UL))
#define SOC_CPU_PDC_CPU_RSTDIS_ADDR(base) ((base) + (0x128UL))
#define SOC_CPU_PDC_CPU_RSTSTAT_ADDR(base) ((base) + (0x12CUL))
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_ADDR(base) ((base) + (0x130UL))
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_ADDR(base) ((base) + (0x134UL))
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_ADDR(base) ((base) + (0x138UL))
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_ADDR(base) ((base) + (0x13CUL))
#define SOC_CPU_PDC_CPU_ADB_GT_CFG_ADDR(base) ((base) + (0x140UL))
#define SOC_CPU_PDC_CPU_STAT0_ADDR(base) ((base) + (0x144UL))
#define SOC_CPU_PDC_WAKEUPINT_STAT_ADDR(base) ((base) + (0x148UL))
#define SOC_CPU_PDC_CPU_PDCEN_ADDR(base) ((base) + (0x200UL))
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_ADDR(base) ((base) + (0x204UL))
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_ADDR(base) ((base) + (0x208UL))
#define SOC_CPU_PDC_LITTLE_COREGICMASK_ADDR(base) ((base) + (0x20CUL))
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_ADDR(base) ((base) + (0x210UL))
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_ADDR(base) ((base) + (0x214UL))
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_ADDR(base) ((base) + (0x218UL))
#define SOC_CPU_PDC_LITTLE_COREPWRUPTIME_ADDR(base) ((base) + (0x21CUL))
#define SOC_CPU_PDC_LITTLE_COREPWRDNTIME_ADDR(base) ((base) + (0x220UL))
#define SOC_CPU_PDC_LITTLE_COREISOTIME_ADDR(base) ((base) + (0x224UL))
#define SOC_CPU_PDC_LITTLE_CORERSTTIME_ADDR(base) ((base) + (0x228UL))
#define SOC_CPU_PDC_LITTLE_COREURSTTIME_ADDR(base) ((base) + (0x22CUL))
#define SOC_CPU_PDC_MID_COREPWRUPTIME_ADDR(base) ((base) + (0x230UL))
#define SOC_CPU_PDC_MID_COREPWRDNTIME_ADDR(base) ((base) + (0x234UL))
#define SOC_CPU_PDC_MID_COREISOTIME_ADDR(base) ((base) + (0x238UL))
#define SOC_CPU_PDC_MID_CORERSTTIME_ADDR(base) ((base) + (0x23CUL))
#define SOC_CPU_PDC_MID_COREURSTTIME_ADDR(base) ((base) + (0x240UL))
#define SOC_CPU_PDC_BIG_COREPWRINTEN_ADDR(base) ((base) + (0x304UL))
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_ADDR(base) ((base) + (0x308UL))
#define SOC_CPU_PDC_BIG_COREGICMASK_ADDR(base) ((base) + (0x30CUL))
#define SOC_CPU_PDC_BIG_COREPOWERUP_ADDR(base) ((base) + (0x310UL))
#define SOC_CPU_PDC_BIG_COREPOWERDN_ADDR(base) ((base) + (0x314UL))
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_ADDR(base) ((base) + (0x318UL))
#define SOC_CPU_PDC_BIG_COREPWRUPTIME_ADDR(base) ((base) + (0x31CUL))
#define SOC_CPU_PDC_BIG_COREPWRDNTIME_ADDR(base) ((base) + (0x320UL))
#define SOC_CPU_PDC_BIG_COREISOTIME_ADDR(base) ((base) + (0x324UL))
#define SOC_CPU_PDC_BIG_CORERSTTIME_ADDR(base) ((base) + (0x328UL))
#define SOC_CPU_PDC_BIG_COREURSTTIME_ADDR(base) ((base) + (0x32CUL))
#define SOC_CPU_PDC_MERGE_COREPWRINTSTAT_ADDR(base) ((base) + (0x330UL))
#define SOC_CPU_PDC_LITTLE_PWRDNTIME_ADDR(base) ((base) + (0x334UL))
#define SOC_CPU_PDC_MIDDLE_PWRDNTIME_ADDR(base) ((base) + (0x338UL))
#define SOC_CPU_PDC_BIG_PWRDNTIME_ADDR(base) ((base) + (0x33CUL))
#define SOC_CPU_PDC_CPU_COREGICMASK_1_ADDR(base) ((base) + (0x340UL))
#else
#define SOC_CPU_PDC_PDCCTRL_ADDR(base) ((base) + (0x000))
#define SOC_CPU_PDC_ISOEN_ADDR(base) ((base) + (0x100))
#define SOC_CPU_PDC_ISODIS_ADDR(base) ((base) + (0x104))
#define SOC_CPU_PDC_ISOSTAT_ADDR(base) ((base) + (0x108))
#define SOC_CPU_PDC_PERPWREN_ADDR(base) ((base) + (0x110))
#define SOC_CPU_PDC_PERPWRDIS_ADDR(base) ((base) + (0x114))
#define SOC_CPU_PDC_PERPWRSTAT_ADDR(base) ((base) + (0x118))
#define SOC_CPU_PDC_PERPWRACK_ADDR(base) ((base) + (0x11C))
#define SOC_CPU_PDC_CPU_CLKEN_ADDR(base) ((base) + (0x120))
#define SOC_CPU_PDC_CPU_RSTEN_ADDR(base) ((base) + (0x124))
#define SOC_CPU_PDC_CPU_RSTDIS_ADDR(base) ((base) + (0x128))
#define SOC_CPU_PDC_CPU_RSTSTAT_ADDR(base) ((base) + (0x12C))
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_ADDR(base) ((base) + (0x130))
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_ADDR(base) ((base) + (0x134))
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_ADDR(base) ((base) + (0x138))
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_ADDR(base) ((base) + (0x13C))
#define SOC_CPU_PDC_CPU_ADB_GT_CFG_ADDR(base) ((base) + (0x140))
#define SOC_CPU_PDC_CPU_STAT0_ADDR(base) ((base) + (0x144))
#define SOC_CPU_PDC_WAKEUPINT_STAT_ADDR(base) ((base) + (0x148))
#define SOC_CPU_PDC_CPU_PDCEN_ADDR(base) ((base) + (0x200))
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_ADDR(base) ((base) + (0x204))
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_ADDR(base) ((base) + (0x208))
#define SOC_CPU_PDC_LITTLE_COREGICMASK_ADDR(base) ((base) + (0x20C))
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_ADDR(base) ((base) + (0x210))
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_ADDR(base) ((base) + (0x214))
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_ADDR(base) ((base) + (0x218))
#define SOC_CPU_PDC_LITTLE_COREPWRUPTIME_ADDR(base) ((base) + (0x21C))
#define SOC_CPU_PDC_LITTLE_COREPWRDNTIME_ADDR(base) ((base) + (0x220))
#define SOC_CPU_PDC_LITTLE_COREISOTIME_ADDR(base) ((base) + (0x224))
#define SOC_CPU_PDC_LITTLE_CORERSTTIME_ADDR(base) ((base) + (0x228))
#define SOC_CPU_PDC_LITTLE_COREURSTTIME_ADDR(base) ((base) + (0x22C))
#define SOC_CPU_PDC_MID_COREPWRUPTIME_ADDR(base) ((base) + (0x230))
#define SOC_CPU_PDC_MID_COREPWRDNTIME_ADDR(base) ((base) + (0x234))
#define SOC_CPU_PDC_MID_COREISOTIME_ADDR(base) ((base) + (0x238))
#define SOC_CPU_PDC_MID_CORERSTTIME_ADDR(base) ((base) + (0x23C))
#define SOC_CPU_PDC_MID_COREURSTTIME_ADDR(base) ((base) + (0x240))
#define SOC_CPU_PDC_BIG_COREPWRINTEN_ADDR(base) ((base) + (0x304))
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_ADDR(base) ((base) + (0x308))
#define SOC_CPU_PDC_BIG_COREGICMASK_ADDR(base) ((base) + (0x30C))
#define SOC_CPU_PDC_BIG_COREPOWERUP_ADDR(base) ((base) + (0x310))
#define SOC_CPU_PDC_BIG_COREPOWERDN_ADDR(base) ((base) + (0x314))
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_ADDR(base) ((base) + (0x318))
#define SOC_CPU_PDC_BIG_COREPWRUPTIME_ADDR(base) ((base) + (0x31C))
#define SOC_CPU_PDC_BIG_COREPWRDNTIME_ADDR(base) ((base) + (0x320))
#define SOC_CPU_PDC_BIG_COREISOTIME_ADDR(base) ((base) + (0x324))
#define SOC_CPU_PDC_BIG_CORERSTTIME_ADDR(base) ((base) + (0x328))
#define SOC_CPU_PDC_BIG_COREURSTTIME_ADDR(base) ((base) + (0x32C))
#define SOC_CPU_PDC_MERGE_COREPWRINTSTAT_ADDR(base) ((base) + (0x330))
#define SOC_CPU_PDC_LITTLE_PWRDNTIME_ADDR(base) ((base) + (0x334))
#define SOC_CPU_PDC_MIDDLE_PWRDNTIME_ADDR(base) ((base) + (0x338))
#define SOC_CPU_PDC_BIG_PWRDNTIME_ADDR(base) ((base) + (0x33C))
#define SOC_CPU_PDC_CPU_COREGICMASK_1_ADDR(base) ((base) + (0x340))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int go2pwr_bypass : 1;
        unsigned int power_ack_bypass : 1;
        unsigned int pwrint_mode_sel : 1;
        unsigned int pdc_mode_sel : 1;
        unsigned int intr_intercept_enable : 1;
        unsigned int core_l2_poweron : 1;
        unsigned int reserved : 10;
        unsigned int bitmasken_000 : 16;
    } reg;
} SOC_CPU_PDC_PDCCTRL_UNION;
#endif
#define SOC_CPU_PDC_PDCCTRL_go2pwr_bypass_START (0)
#define SOC_CPU_PDC_PDCCTRL_go2pwr_bypass_END (0)
#define SOC_CPU_PDC_PDCCTRL_power_ack_bypass_START (1)
#define SOC_CPU_PDC_PDCCTRL_power_ack_bypass_END (1)
#define SOC_CPU_PDC_PDCCTRL_pwrint_mode_sel_START (2)
#define SOC_CPU_PDC_PDCCTRL_pwrint_mode_sel_END (2)
#define SOC_CPU_PDC_PDCCTRL_pdc_mode_sel_START (3)
#define SOC_CPU_PDC_PDCCTRL_pdc_mode_sel_END (3)
#define SOC_CPU_PDC_PDCCTRL_intr_intercept_enable_START (4)
#define SOC_CPU_PDC_PDCCTRL_intr_intercept_enable_END (4)
#define SOC_CPU_PDC_PDCCTRL_core_l2_poweron_START (5)
#define SOC_CPU_PDC_PDCCTRL_core_l2_poweron_END (5)
#define SOC_CPU_PDC_PDCCTRL_bitmasken_000_START (16)
#define SOC_CPU_PDC_PDCCTRL_bitmasken_000_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_0_iso_en : 1;
        unsigned int core_1_iso_en : 1;
        unsigned int core_2_iso_en : 1;
        unsigned int core_3_iso_en : 1;
        unsigned int core_4_iso_en : 1;
        unsigned int core_5_iso_en : 1;
        unsigned int core_6_iso_en : 1;
        unsigned int core_7_iso_en : 1;
        unsigned int core_01_l2_iso_en : 1;
        unsigned int core_23_l2_iso_en : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_CPU_PDC_ISOEN_UNION;
#endif
#define SOC_CPU_PDC_ISOEN_core_0_iso_en_START (0)
#define SOC_CPU_PDC_ISOEN_core_0_iso_en_END (0)
#define SOC_CPU_PDC_ISOEN_core_1_iso_en_START (1)
#define SOC_CPU_PDC_ISOEN_core_1_iso_en_END (1)
#define SOC_CPU_PDC_ISOEN_core_2_iso_en_START (2)
#define SOC_CPU_PDC_ISOEN_core_2_iso_en_END (2)
#define SOC_CPU_PDC_ISOEN_core_3_iso_en_START (3)
#define SOC_CPU_PDC_ISOEN_core_3_iso_en_END (3)
#define SOC_CPU_PDC_ISOEN_core_4_iso_en_START (4)
#define SOC_CPU_PDC_ISOEN_core_4_iso_en_END (4)
#define SOC_CPU_PDC_ISOEN_core_5_iso_en_START (5)
#define SOC_CPU_PDC_ISOEN_core_5_iso_en_END (5)
#define SOC_CPU_PDC_ISOEN_core_6_iso_en_START (6)
#define SOC_CPU_PDC_ISOEN_core_6_iso_en_END (6)
#define SOC_CPU_PDC_ISOEN_core_7_iso_en_START (7)
#define SOC_CPU_PDC_ISOEN_core_7_iso_en_END (7)
#define SOC_CPU_PDC_ISOEN_core_01_l2_iso_en_START (8)
#define SOC_CPU_PDC_ISOEN_core_01_l2_iso_en_END (8)
#define SOC_CPU_PDC_ISOEN_core_23_l2_iso_en_START (9)
#define SOC_CPU_PDC_ISOEN_core_23_l2_iso_en_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_0_iso_dis : 1;
        unsigned int core_1_iso_dis : 1;
        unsigned int core_2_iso_dis : 1;
        unsigned int core_3_iso_dis : 1;
        unsigned int core_4_iso_dis : 1;
        unsigned int core_5_iso_dis : 1;
        unsigned int core_6_iso_dis : 1;
        unsigned int core_7_iso_dis : 1;
        unsigned int core_01_l2_iso_dis : 1;
        unsigned int core_23_l2_iso_dis : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_CPU_PDC_ISODIS_UNION;
#endif
#define SOC_CPU_PDC_ISODIS_core_0_iso_dis_START (0)
#define SOC_CPU_PDC_ISODIS_core_0_iso_dis_END (0)
#define SOC_CPU_PDC_ISODIS_core_1_iso_dis_START (1)
#define SOC_CPU_PDC_ISODIS_core_1_iso_dis_END (1)
#define SOC_CPU_PDC_ISODIS_core_2_iso_dis_START (2)
#define SOC_CPU_PDC_ISODIS_core_2_iso_dis_END (2)
#define SOC_CPU_PDC_ISODIS_core_3_iso_dis_START (3)
#define SOC_CPU_PDC_ISODIS_core_3_iso_dis_END (3)
#define SOC_CPU_PDC_ISODIS_core_4_iso_dis_START (4)
#define SOC_CPU_PDC_ISODIS_core_4_iso_dis_END (4)
#define SOC_CPU_PDC_ISODIS_core_5_iso_dis_START (5)
#define SOC_CPU_PDC_ISODIS_core_5_iso_dis_END (5)
#define SOC_CPU_PDC_ISODIS_core_6_iso_dis_START (6)
#define SOC_CPU_PDC_ISODIS_core_6_iso_dis_END (6)
#define SOC_CPU_PDC_ISODIS_core_7_iso_dis_START (7)
#define SOC_CPU_PDC_ISODIS_core_7_iso_dis_END (7)
#define SOC_CPU_PDC_ISODIS_core_01_l2_iso_dis_START (8)
#define SOC_CPU_PDC_ISODIS_core_01_l2_iso_dis_END (8)
#define SOC_CPU_PDC_ISODIS_core_23_l2_iso_dis_START (9)
#define SOC_CPU_PDC_ISODIS_core_23_l2_iso_dis_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_0_iso_stat : 1;
        unsigned int core_1_iso_stat : 1;
        unsigned int core_2_iso_stat : 1;
        unsigned int core_3_iso_stat : 1;
        unsigned int core_4_iso_stat : 1;
        unsigned int core_5_iso_stat : 1;
        unsigned int core_6_iso_stat : 1;
        unsigned int core_7_iso_stat : 1;
        unsigned int core_01_l2_iso_stat : 1;
        unsigned int core_23_l2_iso_stat : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_CPU_PDC_ISOSTAT_UNION;
#endif
#define SOC_CPU_PDC_ISOSTAT_core_0_iso_stat_START (0)
#define SOC_CPU_PDC_ISOSTAT_core_0_iso_stat_END (0)
#define SOC_CPU_PDC_ISOSTAT_core_1_iso_stat_START (1)
#define SOC_CPU_PDC_ISOSTAT_core_1_iso_stat_END (1)
#define SOC_CPU_PDC_ISOSTAT_core_2_iso_stat_START (2)
#define SOC_CPU_PDC_ISOSTAT_core_2_iso_stat_END (2)
#define SOC_CPU_PDC_ISOSTAT_core_3_iso_stat_START (3)
#define SOC_CPU_PDC_ISOSTAT_core_3_iso_stat_END (3)
#define SOC_CPU_PDC_ISOSTAT_core_4_iso_stat_START (4)
#define SOC_CPU_PDC_ISOSTAT_core_4_iso_stat_END (4)
#define SOC_CPU_PDC_ISOSTAT_core_5_iso_stat_START (5)
#define SOC_CPU_PDC_ISOSTAT_core_5_iso_stat_END (5)
#define SOC_CPU_PDC_ISOSTAT_core_6_iso_stat_START (6)
#define SOC_CPU_PDC_ISOSTAT_core_6_iso_stat_END (6)
#define SOC_CPU_PDC_ISOSTAT_core_7_iso_stat_START (7)
#define SOC_CPU_PDC_ISOSTAT_core_7_iso_stat_END (7)
#define SOC_CPU_PDC_ISOSTAT_core_01_l2_iso_stat_START (8)
#define SOC_CPU_PDC_ISOSTAT_core_01_l2_iso_stat_END (8)
#define SOC_CPU_PDC_ISOSTAT_core_23_l2_iso_stat_START (9)
#define SOC_CPU_PDC_ISOSTAT_core_23_l2_iso_stat_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_0_mtcmos_en : 1;
        unsigned int core_1_mtcmos_en : 1;
        unsigned int core_2_mtcmos_en : 1;
        unsigned int core_3_mtcmos_en : 1;
        unsigned int core_4_mtcmos_en : 1;
        unsigned int core_5_mtcmos_en : 1;
        unsigned int core_6_mtcmos_en : 1;
        unsigned int core_7_mtcmos_en : 1;
        unsigned int core_01_l2_mtcmos_en : 1;
        unsigned int core_23_l2_mtcmos_en : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_CPU_PDC_PERPWREN_UNION;
#endif
#define SOC_CPU_PDC_PERPWREN_core_0_mtcmos_en_START (0)
#define SOC_CPU_PDC_PERPWREN_core_0_mtcmos_en_END (0)
#define SOC_CPU_PDC_PERPWREN_core_1_mtcmos_en_START (1)
#define SOC_CPU_PDC_PERPWREN_core_1_mtcmos_en_END (1)
#define SOC_CPU_PDC_PERPWREN_core_2_mtcmos_en_START (2)
#define SOC_CPU_PDC_PERPWREN_core_2_mtcmos_en_END (2)
#define SOC_CPU_PDC_PERPWREN_core_3_mtcmos_en_START (3)
#define SOC_CPU_PDC_PERPWREN_core_3_mtcmos_en_END (3)
#define SOC_CPU_PDC_PERPWREN_core_4_mtcmos_en_START (4)
#define SOC_CPU_PDC_PERPWREN_core_4_mtcmos_en_END (4)
#define SOC_CPU_PDC_PERPWREN_core_5_mtcmos_en_START (5)
#define SOC_CPU_PDC_PERPWREN_core_5_mtcmos_en_END (5)
#define SOC_CPU_PDC_PERPWREN_core_6_mtcmos_en_START (6)
#define SOC_CPU_PDC_PERPWREN_core_6_mtcmos_en_END (6)
#define SOC_CPU_PDC_PERPWREN_core_7_mtcmos_en_START (7)
#define SOC_CPU_PDC_PERPWREN_core_7_mtcmos_en_END (7)
#define SOC_CPU_PDC_PERPWREN_core_01_l2_mtcmos_en_START (8)
#define SOC_CPU_PDC_PERPWREN_core_01_l2_mtcmos_en_END (8)
#define SOC_CPU_PDC_PERPWREN_core_23_l2_mtcmos_en_START (9)
#define SOC_CPU_PDC_PERPWREN_core_23_l2_mtcmos_en_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_0_mtcmos_dis : 1;
        unsigned int core_1_mtcmos_dis : 1;
        unsigned int core_2_mtcmos_dis : 1;
        unsigned int core_3_mtcmos_dis : 1;
        unsigned int core_4_mtcmos_dis : 1;
        unsigned int core_5_mtcmos_dis : 1;
        unsigned int core_6_mtcmos_dis : 1;
        unsigned int core_7_mtcmos_dis : 1;
        unsigned int core_01_l2_mtcmos_dis : 1;
        unsigned int core_23_l2_mtcmos_dis : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_CPU_PDC_PERPWRDIS_UNION;
#endif
#define SOC_CPU_PDC_PERPWRDIS_core_0_mtcmos_dis_START (0)
#define SOC_CPU_PDC_PERPWRDIS_core_0_mtcmos_dis_END (0)
#define SOC_CPU_PDC_PERPWRDIS_core_1_mtcmos_dis_START (1)
#define SOC_CPU_PDC_PERPWRDIS_core_1_mtcmos_dis_END (1)
#define SOC_CPU_PDC_PERPWRDIS_core_2_mtcmos_dis_START (2)
#define SOC_CPU_PDC_PERPWRDIS_core_2_mtcmos_dis_END (2)
#define SOC_CPU_PDC_PERPWRDIS_core_3_mtcmos_dis_START (3)
#define SOC_CPU_PDC_PERPWRDIS_core_3_mtcmos_dis_END (3)
#define SOC_CPU_PDC_PERPWRDIS_core_4_mtcmos_dis_START (4)
#define SOC_CPU_PDC_PERPWRDIS_core_4_mtcmos_dis_END (4)
#define SOC_CPU_PDC_PERPWRDIS_core_5_mtcmos_dis_START (5)
#define SOC_CPU_PDC_PERPWRDIS_core_5_mtcmos_dis_END (5)
#define SOC_CPU_PDC_PERPWRDIS_core_6_mtcmos_dis_START (6)
#define SOC_CPU_PDC_PERPWRDIS_core_6_mtcmos_dis_END (6)
#define SOC_CPU_PDC_PERPWRDIS_core_7_mtcmos_dis_START (7)
#define SOC_CPU_PDC_PERPWRDIS_core_7_mtcmos_dis_END (7)
#define SOC_CPU_PDC_PERPWRDIS_core_01_l2_mtcmos_dis_START (8)
#define SOC_CPU_PDC_PERPWRDIS_core_01_l2_mtcmos_dis_END (8)
#define SOC_CPU_PDC_PERPWRDIS_core_23_l2_mtcmos_dis_START (9)
#define SOC_CPU_PDC_PERPWRDIS_core_23_l2_mtcmos_dis_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_0_mtcmos_stat : 1;
        unsigned int core_1_mtcmos_stat : 1;
        unsigned int core_2_mtcmos_stat : 1;
        unsigned int core_3_mtcmos_stat : 1;
        unsigned int core_4_mtcmos_stat : 1;
        unsigned int core_5_mtcmos_stat : 1;
        unsigned int core_6_mtcmos_stat : 1;
        unsigned int core_7_mtcmos_stat : 1;
        unsigned int core_01_l2_mtcmos_stat : 1;
        unsigned int core_23_l2_mtcmos_stat : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_CPU_PDC_PERPWRSTAT_UNION;
#endif
#define SOC_CPU_PDC_PERPWRSTAT_core_0_mtcmos_stat_START (0)
#define SOC_CPU_PDC_PERPWRSTAT_core_0_mtcmos_stat_END (0)
#define SOC_CPU_PDC_PERPWRSTAT_core_1_mtcmos_stat_START (1)
#define SOC_CPU_PDC_PERPWRSTAT_core_1_mtcmos_stat_END (1)
#define SOC_CPU_PDC_PERPWRSTAT_core_2_mtcmos_stat_START (2)
#define SOC_CPU_PDC_PERPWRSTAT_core_2_mtcmos_stat_END (2)
#define SOC_CPU_PDC_PERPWRSTAT_core_3_mtcmos_stat_START (3)
#define SOC_CPU_PDC_PERPWRSTAT_core_3_mtcmos_stat_END (3)
#define SOC_CPU_PDC_PERPWRSTAT_core_4_mtcmos_stat_START (4)
#define SOC_CPU_PDC_PERPWRSTAT_core_4_mtcmos_stat_END (4)
#define SOC_CPU_PDC_PERPWRSTAT_core_5_mtcmos_stat_START (5)
#define SOC_CPU_PDC_PERPWRSTAT_core_5_mtcmos_stat_END (5)
#define SOC_CPU_PDC_PERPWRSTAT_core_6_mtcmos_stat_START (6)
#define SOC_CPU_PDC_PERPWRSTAT_core_6_mtcmos_stat_END (6)
#define SOC_CPU_PDC_PERPWRSTAT_core_7_mtcmos_stat_START (7)
#define SOC_CPU_PDC_PERPWRSTAT_core_7_mtcmos_stat_END (7)
#define SOC_CPU_PDC_PERPWRSTAT_core_01_l2_mtcmos_stat_START (8)
#define SOC_CPU_PDC_PERPWRSTAT_core_01_l2_mtcmos_stat_END (8)
#define SOC_CPU_PDC_PERPWRSTAT_core_23_l2_mtcmos_stat_START (9)
#define SOC_CPU_PDC_PERPWRSTAT_core_23_l2_mtcmos_stat_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_0_mtcmos_ack_and : 1;
        unsigned int core_1_mtcmos_ack_and : 1;
        unsigned int core_2_mtcmos_ack_and : 1;
        unsigned int core_3_mtcmos_ack_and : 1;
        unsigned int core_4_mtcmos_ack_and : 1;
        unsigned int core_5_mtcmos_ack_and : 1;
        unsigned int core_6_mtcmos_ack_and : 1;
        unsigned int core_7_mtcmos_ack_and : 1;
        unsigned int core_01_l2_mtcmos_ack_and : 1;
        unsigned int core_23_l2_mtcmos_ack_and : 1;
        unsigned int reserved_0 : 6;
        unsigned int core_0_mtcmos_ack_or : 1;
        unsigned int core_1_mtcmos_ack_or : 1;
        unsigned int core_2_mtcmos_ack_or : 1;
        unsigned int core_3_mtcmos_ack_or : 1;
        unsigned int core_4_mtcmos_ack_or : 1;
        unsigned int core_5_mtcmos_ack_or : 1;
        unsigned int core_6_mtcmos_ack_or : 1;
        unsigned int core_7_mtcmos_ack_or : 1;
        unsigned int core_01_l2_mtcmos_ack_or : 1;
        unsigned int core_23_l2_mtcmos_ack_or : 1;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_CPU_PDC_PERPWRACK_UNION;
#endif
#define SOC_CPU_PDC_PERPWRACK_core_0_mtcmos_ack_and_START (0)
#define SOC_CPU_PDC_PERPWRACK_core_0_mtcmos_ack_and_END (0)
#define SOC_CPU_PDC_PERPWRACK_core_1_mtcmos_ack_and_START (1)
#define SOC_CPU_PDC_PERPWRACK_core_1_mtcmos_ack_and_END (1)
#define SOC_CPU_PDC_PERPWRACK_core_2_mtcmos_ack_and_START (2)
#define SOC_CPU_PDC_PERPWRACK_core_2_mtcmos_ack_and_END (2)
#define SOC_CPU_PDC_PERPWRACK_core_3_mtcmos_ack_and_START (3)
#define SOC_CPU_PDC_PERPWRACK_core_3_mtcmos_ack_and_END (3)
#define SOC_CPU_PDC_PERPWRACK_core_4_mtcmos_ack_and_START (4)
#define SOC_CPU_PDC_PERPWRACK_core_4_mtcmos_ack_and_END (4)
#define SOC_CPU_PDC_PERPWRACK_core_5_mtcmos_ack_and_START (5)
#define SOC_CPU_PDC_PERPWRACK_core_5_mtcmos_ack_and_END (5)
#define SOC_CPU_PDC_PERPWRACK_core_6_mtcmos_ack_and_START (6)
#define SOC_CPU_PDC_PERPWRACK_core_6_mtcmos_ack_and_END (6)
#define SOC_CPU_PDC_PERPWRACK_core_7_mtcmos_ack_and_START (7)
#define SOC_CPU_PDC_PERPWRACK_core_7_mtcmos_ack_and_END (7)
#define SOC_CPU_PDC_PERPWRACK_core_01_l2_mtcmos_ack_and_START (8)
#define SOC_CPU_PDC_PERPWRACK_core_01_l2_mtcmos_ack_and_END (8)
#define SOC_CPU_PDC_PERPWRACK_core_23_l2_mtcmos_ack_and_START (9)
#define SOC_CPU_PDC_PERPWRACK_core_23_l2_mtcmos_ack_and_END (9)
#define SOC_CPU_PDC_PERPWRACK_core_0_mtcmos_ack_or_START (16)
#define SOC_CPU_PDC_PERPWRACK_core_0_mtcmos_ack_or_END (16)
#define SOC_CPU_PDC_PERPWRACK_core_1_mtcmos_ack_or_START (17)
#define SOC_CPU_PDC_PERPWRACK_core_1_mtcmos_ack_or_END (17)
#define SOC_CPU_PDC_PERPWRACK_core_2_mtcmos_ack_or_START (18)
#define SOC_CPU_PDC_PERPWRACK_core_2_mtcmos_ack_or_END (18)
#define SOC_CPU_PDC_PERPWRACK_core_3_mtcmos_ack_or_START (19)
#define SOC_CPU_PDC_PERPWRACK_core_3_mtcmos_ack_or_END (19)
#define SOC_CPU_PDC_PERPWRACK_core_4_mtcmos_ack_or_START (20)
#define SOC_CPU_PDC_PERPWRACK_core_4_mtcmos_ack_or_END (20)
#define SOC_CPU_PDC_PERPWRACK_core_5_mtcmos_ack_or_START (21)
#define SOC_CPU_PDC_PERPWRACK_core_5_mtcmos_ack_or_END (21)
#define SOC_CPU_PDC_PERPWRACK_core_6_mtcmos_ack_or_START (22)
#define SOC_CPU_PDC_PERPWRACK_core_6_mtcmos_ack_or_END (22)
#define SOC_CPU_PDC_PERPWRACK_core_7_mtcmos_ack_or_START (23)
#define SOC_CPU_PDC_PERPWRACK_core_7_mtcmos_ack_or_END (23)
#define SOC_CPU_PDC_PERPWRACK_core_01_l2_mtcmos_ack_or_START (24)
#define SOC_CPU_PDC_PERPWRACK_core_01_l2_mtcmos_ack_or_END (24)
#define SOC_CPU_PDC_PERPWRACK_core_23_l2_mtcmos_ack_or_START (25)
#define SOC_CPU_PDC_PERPWRACK_core_23_l2_mtcmos_ack_or_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_gicclk_en : 1;
        unsigned int gt_periphclk_en : 1;
        unsigned int gt_clk_ananke_en : 1;
        unsigned int gt_clk_scu_en : 1;
        unsigned int gt_atclk_top_en : 1;
        unsigned int gt_pclkdbg_top_en : 1;
        unsigned int reserved : 10;
        unsigned int bitmasken_120 : 16;
    } reg;
} SOC_CPU_PDC_CPU_CLKEN_UNION;
#endif
#define SOC_CPU_PDC_CPU_CLKEN_gt_gicclk_en_START (0)
#define SOC_CPU_PDC_CPU_CLKEN_gt_gicclk_en_END (0)
#define SOC_CPU_PDC_CPU_CLKEN_gt_periphclk_en_START (1)
#define SOC_CPU_PDC_CPU_CLKEN_gt_periphclk_en_END (1)
#define SOC_CPU_PDC_CPU_CLKEN_gt_clk_ananke_en_START (2)
#define SOC_CPU_PDC_CPU_CLKEN_gt_clk_ananke_en_END (2)
#define SOC_CPU_PDC_CPU_CLKEN_gt_clk_scu_en_START (3)
#define SOC_CPU_PDC_CPU_CLKEN_gt_clk_scu_en_END (3)
#define SOC_CPU_PDC_CPU_CLKEN_gt_atclk_top_en_START (4)
#define SOC_CPU_PDC_CPU_CLKEN_gt_atclk_top_en_END (4)
#define SOC_CPU_PDC_CPU_CLKEN_gt_pclkdbg_top_en_START (5)
#define SOC_CPU_PDC_CPU_CLKEN_gt_pclkdbg_top_en_END (5)
#define SOC_CPU_PDC_CPU_CLKEN_bitmasken_120_START (16)
#define SOC_CPU_PDC_CPU_CLKEN_bitmasken_120_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_por_rst_req_little_en : 4;
        unsigned int core_por_rst_req_middle_en : 3;
        unsigned int core_por_rst_req_big_en : 1;
        unsigned int presetdbg_rst_req_en : 1;
        unsigned int coresight_soft_rst_req_en : 1;
        unsigned int cluster_soft_rst_req_en : 1;
        unsigned int scu_soft_rst_req_en : 1;
        unsigned int klein_comlplex0_rst_req_en : 1;
        unsigned int klein_comlplex1_rst_req_en : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_CPU_PDC_CPU_RSTEN_UNION;
#endif
#define SOC_CPU_PDC_CPU_RSTEN_core_por_rst_req_little_en_START (0)
#define SOC_CPU_PDC_CPU_RSTEN_core_por_rst_req_little_en_END (3)
#define SOC_CPU_PDC_CPU_RSTEN_core_por_rst_req_middle_en_START (4)
#define SOC_CPU_PDC_CPU_RSTEN_core_por_rst_req_middle_en_END (6)
#define SOC_CPU_PDC_CPU_RSTEN_core_por_rst_req_big_en_START (7)
#define SOC_CPU_PDC_CPU_RSTEN_core_por_rst_req_big_en_END (7)
#define SOC_CPU_PDC_CPU_RSTEN_presetdbg_rst_req_en_START (8)
#define SOC_CPU_PDC_CPU_RSTEN_presetdbg_rst_req_en_END (8)
#define SOC_CPU_PDC_CPU_RSTEN_coresight_soft_rst_req_en_START (9)
#define SOC_CPU_PDC_CPU_RSTEN_coresight_soft_rst_req_en_END (9)
#define SOC_CPU_PDC_CPU_RSTEN_cluster_soft_rst_req_en_START (10)
#define SOC_CPU_PDC_CPU_RSTEN_cluster_soft_rst_req_en_END (10)
#define SOC_CPU_PDC_CPU_RSTEN_scu_soft_rst_req_en_START (11)
#define SOC_CPU_PDC_CPU_RSTEN_scu_soft_rst_req_en_END (11)
#define SOC_CPU_PDC_CPU_RSTEN_klein_comlplex0_rst_req_en_START (12)
#define SOC_CPU_PDC_CPU_RSTEN_klein_comlplex0_rst_req_en_END (12)
#define SOC_CPU_PDC_CPU_RSTEN_klein_comlplex1_rst_req_en_START (13)
#define SOC_CPU_PDC_CPU_RSTEN_klein_comlplex1_rst_req_en_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_por_rst_req_little_dis : 4;
        unsigned int core_por_rst_req_middle_dis : 3;
        unsigned int core_por_rst_req_big_dis : 1;
        unsigned int presetdbg_rst_req_dis : 1;
        unsigned int coresight_soft_rst_req_dis : 1;
        unsigned int cluster_soft_rst_req_dis : 1;
        unsigned int scu_soft_rst_req_dis : 1;
        unsigned int klein_comlplex0_rst_req_dis : 1;
        unsigned int klein_comlplex1_rst_req_dis : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_CPU_PDC_CPU_RSTDIS_UNION;
#endif
#define SOC_CPU_PDC_CPU_RSTDIS_core_por_rst_req_little_dis_START (0)
#define SOC_CPU_PDC_CPU_RSTDIS_core_por_rst_req_little_dis_END (3)
#define SOC_CPU_PDC_CPU_RSTDIS_core_por_rst_req_middle_dis_START (4)
#define SOC_CPU_PDC_CPU_RSTDIS_core_por_rst_req_middle_dis_END (6)
#define SOC_CPU_PDC_CPU_RSTDIS_core_por_rst_req_big_dis_START (7)
#define SOC_CPU_PDC_CPU_RSTDIS_core_por_rst_req_big_dis_END (7)
#define SOC_CPU_PDC_CPU_RSTDIS_presetdbg_rst_req_dis_START (8)
#define SOC_CPU_PDC_CPU_RSTDIS_presetdbg_rst_req_dis_END (8)
#define SOC_CPU_PDC_CPU_RSTDIS_coresight_soft_rst_req_dis_START (9)
#define SOC_CPU_PDC_CPU_RSTDIS_coresight_soft_rst_req_dis_END (9)
#define SOC_CPU_PDC_CPU_RSTDIS_cluster_soft_rst_req_dis_START (10)
#define SOC_CPU_PDC_CPU_RSTDIS_cluster_soft_rst_req_dis_END (10)
#define SOC_CPU_PDC_CPU_RSTDIS_scu_soft_rst_req_dis_START (11)
#define SOC_CPU_PDC_CPU_RSTDIS_scu_soft_rst_req_dis_END (11)
#define SOC_CPU_PDC_CPU_RSTDIS_klein_comlplex0_rst_req_dis_START (12)
#define SOC_CPU_PDC_CPU_RSTDIS_klein_comlplex0_rst_req_dis_END (12)
#define SOC_CPU_PDC_CPU_RSTDIS_klein_comlplex1_rst_req_dis_START (13)
#define SOC_CPU_PDC_CPU_RSTDIS_klein_comlplex1_rst_req_dis_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_por_rst_req_little_stat : 4;
        unsigned int core_por_rst_req_middle_stat : 3;
        unsigned int core_por_rst_req_big_stat : 1;
        unsigned int presetdbg_rst_req_stat : 1;
        unsigned int coresight_soft_rst_req_stat : 1;
        unsigned int cluster_soft_rst_req_stat : 1;
        unsigned int scu_soft_rst_req_stat : 1;
        unsigned int klein_comlplex0_rst_req_stat : 1;
        unsigned int klein_comlplex1_rst_req_stat : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_CPU_PDC_CPU_RSTSTAT_UNION;
#endif
#define SOC_CPU_PDC_CPU_RSTSTAT_core_por_rst_req_little_stat_START (0)
#define SOC_CPU_PDC_CPU_RSTSTAT_core_por_rst_req_little_stat_END (3)
#define SOC_CPU_PDC_CPU_RSTSTAT_core_por_rst_req_middle_stat_START (4)
#define SOC_CPU_PDC_CPU_RSTSTAT_core_por_rst_req_middle_stat_END (6)
#define SOC_CPU_PDC_CPU_RSTSTAT_core_por_rst_req_big_stat_START (7)
#define SOC_CPU_PDC_CPU_RSTSTAT_core_por_rst_req_big_stat_END (7)
#define SOC_CPU_PDC_CPU_RSTSTAT_presetdbg_rst_req_stat_START (8)
#define SOC_CPU_PDC_CPU_RSTSTAT_presetdbg_rst_req_stat_END (8)
#define SOC_CPU_PDC_CPU_RSTSTAT_coresight_soft_rst_req_stat_START (9)
#define SOC_CPU_PDC_CPU_RSTSTAT_coresight_soft_rst_req_stat_END (9)
#define SOC_CPU_PDC_CPU_RSTSTAT_cluster_soft_rst_req_stat_START (10)
#define SOC_CPU_PDC_CPU_RSTSTAT_cluster_soft_rst_req_stat_END (10)
#define SOC_CPU_PDC_CPU_RSTSTAT_scu_soft_rst_req_stat_START (11)
#define SOC_CPU_PDC_CPU_RSTSTAT_scu_soft_rst_req_stat_END (11)
#define SOC_CPU_PDC_CPU_RSTSTAT_klein_comlplex0_rst_req_stat_START (12)
#define SOC_CPU_PDC_CPU_RSTSTAT_klein_comlplex0_rst_req_stat_END (12)
#define SOC_CPU_PDC_CPU_RSTSTAT_klein_comlplex1_rst_req_stat_START (13)
#define SOC_CPU_PDC_CPU_RSTSTAT_klein_comlplex1_rst_req_stat_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pwrqreq_adb_m0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int pwrqactive_adb_m0 : 1;
        unsigned int pwrqaccept_adb_m0 : 1;
        unsigned int pwrqdeney_adb_m0 : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_CPU_PDC_CPU_M0_ADBLPSTAT_UNION;
#endif
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_pwrqreq_adb_m0_START (0)
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_pwrqreq_adb_m0_END (0)
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_pwrqactive_adb_m0_START (2)
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_pwrqactive_adb_m0_END (2)
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_pwrqaccept_adb_m0_START (3)
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_pwrqaccept_adb_m0_END (3)
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_pwrqdeney_adb_m0_START (4)
#define SOC_CPU_PDC_CPU_M0_ADBLPSTAT_pwrqdeney_adb_m0_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pwrqreq_adb_m1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int pwrqactive_adb_m1 : 1;
        unsigned int pwrqaccept_adb_m1 : 1;
        unsigned int pwrqdeney_adb_m1 : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_CPU_PDC_CPU_M1_ADBLPSTAT_UNION;
#endif
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_pwrqreq_adb_m1_START (0)
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_pwrqreq_adb_m1_END (0)
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_pwrqactive_adb_m1_START (2)
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_pwrqactive_adb_m1_END (2)
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_pwrqaccept_adb_m1_START (3)
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_pwrqaccept_adb_m1_END (3)
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_pwrqdeney_adb_m1_START (4)
#define SOC_CPU_PDC_CPU_M1_ADBLPSTAT_pwrqdeney_adb_m1_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pwrqreq_adb_mp : 1;
        unsigned int reserved_0 : 1;
        unsigned int pwrqactive_adb_mp : 1;
        unsigned int pwrqaccept_adb_mp : 1;
        unsigned int pwrqdeney_adb_mp : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_CPU_PDC_CPU_MP_ADBLPSTAT_UNION;
#endif
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_pwrqreq_adb_mp_START (0)
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_pwrqreq_adb_mp_END (0)
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_pwrqactive_adb_mp_START (2)
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_pwrqactive_adb_mp_END (2)
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_pwrqaccept_adb_mp_START (3)
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_pwrqaccept_adb_mp_END (3)
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_pwrqdeney_adb_mp_START (4)
#define SOC_CPU_PDC_CPU_MP_ADBLPSTAT_pwrqdeney_adb_mp_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pwrqreq_adb_acp : 1;
        unsigned int reserved_0 : 1;
        unsigned int pwrqactive_adb_acp : 1;
        unsigned int pwrqaccept_adb_acp : 1;
        unsigned int pwrqdeney_adb_acp : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_UNION;
#endif
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_pwrqreq_adb_acp_START (0)
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_pwrqreq_adb_acp_END (0)
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_pwrqactive_adb_acp_START (2)
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_pwrqactive_adb_acp_END (2)
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_pwrqaccept_adb_acp_START (3)
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_pwrqaccept_adb_acp_END (3)
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_pwrqdeney_adb_acp_START (4)
#define SOC_CPU_PDC_CPU_ACP_ADBLPSTAT_pwrqdeney_adb_acp_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int qactive_cfgcnt : 7;
        unsigned int gt_clk_bypass : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_CPU_PDC_CPU_ADB_GT_CFG_UNION;
#endif
#define SOC_CPU_PDC_CPU_ADB_GT_CFG_qactive_cfgcnt_START (0)
#define SOC_CPU_PDC_CPU_ADB_GT_CFG_qactive_cfgcnt_END (6)
#define SOC_CPU_PDC_CPU_ADB_GT_CFG_gt_clk_bypass_START (7)
#define SOC_CPU_PDC_CPU_ADB_GT_CFG_gt_clk_bypass_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 9;
        unsigned int reserved_1: 9;
        unsigned int reserved_2: 2;
        unsigned int core_idle : 8;
        unsigned int l3_idle : 1;
        unsigned int dbgack : 1;
        unsigned int etf_full : 1;
        unsigned int reserved_3: 1;
    } reg;
} SOC_CPU_PDC_CPU_STAT0_UNION;
#endif
#define SOC_CPU_PDC_CPU_STAT0_core_idle_START (20)
#define SOC_CPU_PDC_CPU_STAT0_core_idle_END (27)
#define SOC_CPU_PDC_CPU_STAT0_l3_idle_START (28)
#define SOC_CPU_PDC_CPU_STAT0_l3_idle_END (28)
#define SOC_CPU_PDC_CPU_STAT0_dbgack_START (29)
#define SOC_CPU_PDC_CPU_STAT0_dbgack_END (29)
#define SOC_CPU_PDC_CPU_STAT0_etf_full_START (30)
#define SOC_CPU_PDC_CPU_STAT0_etf_full_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_gic_wake_request_0 : 1;
        unsigned int intr_gic_wake_request_1 : 1;
        unsigned int intr_gic_wake_request_2 : 1;
        unsigned int intr_gic_wake_request_3 : 1;
        unsigned int intr_gic_wake_request_4 : 1;
        unsigned int intr_gic_wake_request_5 : 1;
        unsigned int intr_gic_wake_request_6 : 1;
        unsigned int intr_gic_wake_request_7 : 1;
        unsigned int intr_gic_wake_request_8 : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_CPU_PDC_WAKEUPINT_STAT_UNION;
#endif
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_0_START (0)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_0_END (0)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_1_START (1)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_1_END (1)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_2_START (2)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_2_END (2)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_3_START (3)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_3_END (3)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_4_START (4)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_4_END (4)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_5_START (5)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_5_END (5)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_6_START (6)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_6_END (6)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_7_START (7)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_7_END (7)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_8_START (8)
#define SOC_CPU_PDC_WAKEUPINT_STAT_intr_gic_wake_request_8_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_little_pdc_en : 1;
        unsigned int cpu_middle_pdc_en : 1;
        unsigned int cpu_big_pdc_en : 1;
        unsigned int reserved : 13;
        unsigned int bitmasken_200 : 16;
    } reg;
} SOC_CPU_PDC_CPU_PDCEN_UNION;
#endif
#define SOC_CPU_PDC_CPU_PDCEN_cpu_little_pdc_en_START (0)
#define SOC_CPU_PDC_CPU_PDCEN_cpu_little_pdc_en_END (0)
#define SOC_CPU_PDC_CPU_PDCEN_cpu_middle_pdc_en_START (1)
#define SOC_CPU_PDC_CPU_PDCEN_cpu_middle_pdc_en_END (1)
#define SOC_CPU_PDC_CPU_PDCEN_cpu_big_pdc_en_START (2)
#define SOC_CPU_PDC_CPU_PDCEN_cpu_big_pdc_en_END (2)
#define SOC_CPU_PDC_CPU_PDCEN_bitmasken_200_START (16)
#define SOC_CPU_PDC_CPU_PDCEN_bitmasken_200_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_cpu_core0_pwr_en : 1;
        unsigned int intr_cpu_core1_pwr_en : 1;
        unsigned int intr_cpu_core2_pwr_en : 1;
        unsigned int intr_cpu_core3_pwr_en : 1;
        unsigned int intr_cpu_core01_l2_pwr_en : 1;
        unsigned int intr_cpu_core23_l2_pwr_en : 1;
        unsigned int reserved : 10;
        unsigned int bitmasken_204 : 16;
    } reg;
} SOC_CPU_PDC_LITTLE_COREPWRINTEN_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core0_pwr_en_START (0)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core0_pwr_en_END (0)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core1_pwr_en_START (1)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core1_pwr_en_END (1)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core2_pwr_en_START (2)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core2_pwr_en_END (2)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core3_pwr_en_START (3)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core3_pwr_en_END (3)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core01_l2_pwr_en_START (4)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core01_l2_pwr_en_END (4)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core23_l2_pwr_en_START (5)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_intr_cpu_core23_l2_pwr_en_END (5)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_bitmasken_204_START (16)
#define SOC_CPU_PDC_LITTLE_COREPWRINTEN_bitmasken_204_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_cpu_core0_pwr_stat : 1;
        unsigned int intr_cpu_core1_pwr_stat : 1;
        unsigned int intr_cpu_core2_pwr_stat : 1;
        unsigned int intr_cpu_core3_pwr_stat : 1;
        unsigned int intr_cpu_core01_l2_pwr_stat : 1;
        unsigned int intr_cpu_core23_l2_pwr_stat : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core0_pwr_stat_START (0)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core0_pwr_stat_END (0)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core1_pwr_stat_START (1)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core1_pwr_stat_END (1)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core2_pwr_stat_START (2)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core2_pwr_stat_END (2)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core3_pwr_stat_START (3)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core3_pwr_stat_END (3)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core01_l2_pwr_stat_START (4)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core01_l2_pwr_stat_END (4)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core23_l2_pwr_stat_START (5)
#define SOC_CPU_PDC_LITTLE_COREPWRINTSTAT_intr_cpu_core23_l2_pwr_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_cpu_core0_gic_mask : 1;
        unsigned int intr_cpu_core1_gic_mask : 1;
        unsigned int intr_cpu_core2_gic_mask : 1;
        unsigned int intr_cpu_core3_gic_mask : 1;
        unsigned int reserved : 12;
        unsigned int bitmasken_20C : 16;
    } reg;
} SOC_CPU_PDC_LITTLE_COREGICMASK_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREGICMASK_intr_cpu_core0_gic_mask_START (0)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_intr_cpu_core0_gic_mask_END (0)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_intr_cpu_core1_gic_mask_START (1)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_intr_cpu_core1_gic_mask_END (1)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_intr_cpu_core2_gic_mask_START (2)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_intr_cpu_core2_gic_mask_END (2)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_intr_cpu_core3_gic_mask_START (3)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_intr_cpu_core3_gic_mask_END (3)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_bitmasken_20C_START (16)
#define SOC_CPU_PDC_LITTLE_COREGICMASK_bitmasken_20C_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_core0_pwrup_req : 1;
        unsigned int cpu_core1_pwrup_req : 1;
        unsigned int cpu_core2_pwrup_req : 1;
        unsigned int cpu_core3_pwrup_req : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_CPU_PDC_LITTLE_COREPOWERUP_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_cpu_core0_pwrup_req_START (0)
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_cpu_core0_pwrup_req_END (0)
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_cpu_core1_pwrup_req_START (1)
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_cpu_core1_pwrup_req_END (1)
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_cpu_core2_pwrup_req_START (2)
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_cpu_core2_pwrup_req_END (2)
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_cpu_core3_pwrup_req_START (3)
#define SOC_CPU_PDC_LITTLE_COREPOWERUP_cpu_core3_pwrup_req_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_core0_pwrdn_req : 1;
        unsigned int cpu_core1_pwrdn_req : 1;
        unsigned int cpu_core2_pwrdn_req : 1;
        unsigned int cpu_core3_pwrdn_req : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_CPU_PDC_LITTLE_COREPOWERDN_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_cpu_core0_pwrdn_req_START (0)
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_cpu_core0_pwrdn_req_END (0)
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_cpu_core1_pwrdn_req_START (1)
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_cpu_core1_pwrdn_req_END (1)
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_cpu_core2_pwrdn_req_START (2)
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_cpu_core2_pwrdn_req_END (2)
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_cpu_core3_pwrdn_req_START (3)
#define SOC_CPU_PDC_LITTLE_COREPOWERDN_cpu_core3_pwrdn_req_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_core0_pwr_stat : 4;
        unsigned int cpu_core1_pwr_stat : 4;
        unsigned int cpu_core2_pwr_stat : 4;
        unsigned int cpu_core3_pwr_stat : 4;
        unsigned int cpu_core01_l2_pwr_stat : 4;
        unsigned int cpu_core23_l2_pwr_stat : 4;
        unsigned int reserved : 8;
    } reg;
} SOC_CPU_PDC_LITTLE_COREPOWERSTAT_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core0_pwr_stat_START (0)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core0_pwr_stat_END (3)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core1_pwr_stat_START (4)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core1_pwr_stat_END (7)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core2_pwr_stat_START (8)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core2_pwr_stat_END (11)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core3_pwr_stat_START (12)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core3_pwr_stat_END (15)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core01_l2_pwr_stat_START (16)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core01_l2_pwr_stat_END (19)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core23_l2_pwr_stat_START (20)
#define SOC_CPU_PDC_LITTLE_COREPOWERSTAT_cpu_core23_l2_pwr_stat_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_little_core_pwrup_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_LITTLE_COREPWRUPTIME_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREPWRUPTIME_cpu_little_core_pwrup_time_START (0)
#define SOC_CPU_PDC_LITTLE_COREPWRUPTIME_cpu_little_core_pwrup_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_little_core_pwrdn_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_LITTLE_COREPWRDNTIME_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREPWRDNTIME_cpu_little_core_pwrdn_time_START (0)
#define SOC_CPU_PDC_LITTLE_COREPWRDNTIME_cpu_little_core_pwrdn_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_little_core_iso_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_LITTLE_COREISOTIME_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREISOTIME_cpu_little_core_iso_time_START (0)
#define SOC_CPU_PDC_LITTLE_COREISOTIME_cpu_little_core_iso_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_little_core_rst_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_LITTLE_CORERSTTIME_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_CORERSTTIME_cpu_little_core_rst_time_START (0)
#define SOC_CPU_PDC_LITTLE_CORERSTTIME_cpu_little_core_rst_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_little_core_urst_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_LITTLE_COREURSTTIME_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_COREURSTTIME_cpu_little_core_urst_time_START (0)
#define SOC_CPU_PDC_LITTLE_COREURSTTIME_cpu_little_core_urst_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_middle_core_pwrup_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_MID_COREPWRUPTIME_UNION;
#endif
#define SOC_CPU_PDC_MID_COREPWRUPTIME_cpu_middle_core_pwrup_time_START (0)
#define SOC_CPU_PDC_MID_COREPWRUPTIME_cpu_middle_core_pwrup_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_middle_core_pwrdn_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_MID_COREPWRDNTIME_UNION;
#endif
#define SOC_CPU_PDC_MID_COREPWRDNTIME_cpu_middle_core_pwrdn_time_START (0)
#define SOC_CPU_PDC_MID_COREPWRDNTIME_cpu_middle_core_pwrdn_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_middle_core_iso_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_MID_COREISOTIME_UNION;
#endif
#define SOC_CPU_PDC_MID_COREISOTIME_cpu_middle_core_iso_time_START (0)
#define SOC_CPU_PDC_MID_COREISOTIME_cpu_middle_core_iso_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_middle_core_rst_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_MID_CORERSTTIME_UNION;
#endif
#define SOC_CPU_PDC_MID_CORERSTTIME_cpu_middle_core_rst_time_START (0)
#define SOC_CPU_PDC_MID_CORERSTTIME_cpu_middle_core_rst_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_middle_core_urst_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_MID_COREURSTTIME_UNION;
#endif
#define SOC_CPU_PDC_MID_COREURSTTIME_cpu_middle_core_urst_time_START (0)
#define SOC_CPU_PDC_MID_COREURSTTIME_cpu_middle_core_urst_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_cpu_core4_pwr_en : 1;
        unsigned int intr_cpu_core5_pwr_en : 1;
        unsigned int intr_cpu_core6_pwr_en : 1;
        unsigned int intr_cpu_core7_pwr_en : 1;
        unsigned int reserved : 12;
        unsigned int bitmasken_304 : 16;
    } reg;
} SOC_CPU_PDC_BIG_COREPWRINTEN_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREPWRINTEN_intr_cpu_core4_pwr_en_START (0)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_intr_cpu_core4_pwr_en_END (0)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_intr_cpu_core5_pwr_en_START (1)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_intr_cpu_core5_pwr_en_END (1)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_intr_cpu_core6_pwr_en_START (2)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_intr_cpu_core6_pwr_en_END (2)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_intr_cpu_core7_pwr_en_START (3)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_intr_cpu_core7_pwr_en_END (3)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_bitmasken_304_START (16)
#define SOC_CPU_PDC_BIG_COREPWRINTEN_bitmasken_304_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_cpu_core4_pwr_stat : 1;
        unsigned int intr_cpu_core5_pwr_stat : 1;
        unsigned int intr_cpu_core6_pwr_stat : 1;
        unsigned int intr_cpu_core7_pwr_stat : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_CPU_PDC_BIG_COREPWRINTSTAT_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_intr_cpu_core4_pwr_stat_START (0)
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_intr_cpu_core4_pwr_stat_END (0)
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_intr_cpu_core5_pwr_stat_START (1)
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_intr_cpu_core5_pwr_stat_END (1)
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_intr_cpu_core6_pwr_stat_START (2)
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_intr_cpu_core6_pwr_stat_END (2)
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_intr_cpu_core7_pwr_stat_START (3)
#define SOC_CPU_PDC_BIG_COREPWRINTSTAT_intr_cpu_core7_pwr_stat_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_cpu_core4_0_gic_mask : 1;
        unsigned int intr_cpu_core4_1_gic_mask : 1;
        unsigned int intr_cpu_core5_0_gic_mask : 1;
        unsigned int intr_cpu_core5_1_gic_mask : 1;
        unsigned int intr_cpu_core6_0_gic_mask : 1;
        unsigned int intr_cpu_core6_1_gic_mask : 1;
        unsigned int intr_cpu_core7_0_gic_mask : 1;
        unsigned int intr_cpu_core7_1_gic_mask : 1;
        unsigned int reserved : 8;
        unsigned int bitmasken_30C : 16;
    } reg;
} SOC_CPU_PDC_BIG_COREGICMASK_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core4_0_gic_mask_START (0)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core4_0_gic_mask_END (0)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core4_1_gic_mask_START (1)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core4_1_gic_mask_END (1)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core5_0_gic_mask_START (2)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core5_0_gic_mask_END (2)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core5_1_gic_mask_START (3)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core5_1_gic_mask_END (3)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core6_0_gic_mask_START (4)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core6_0_gic_mask_END (4)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core6_1_gic_mask_START (5)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core6_1_gic_mask_END (5)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core7_0_gic_mask_START (6)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core7_0_gic_mask_END (6)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core7_1_gic_mask_START (7)
#define SOC_CPU_PDC_BIG_COREGICMASK_intr_cpu_core7_1_gic_mask_END (7)
#define SOC_CPU_PDC_BIG_COREGICMASK_bitmasken_30C_START (16)
#define SOC_CPU_PDC_BIG_COREGICMASK_bitmasken_30C_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_core4_pwrup_req : 1;
        unsigned int cpu_core5_pwrup_req : 1;
        unsigned int cpu_core6_pwrup_req : 1;
        unsigned int cpu_core7_pwrup_req : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_CPU_PDC_BIG_COREPOWERUP_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREPOWERUP_cpu_core4_pwrup_req_START (0)
#define SOC_CPU_PDC_BIG_COREPOWERUP_cpu_core4_pwrup_req_END (0)
#define SOC_CPU_PDC_BIG_COREPOWERUP_cpu_core5_pwrup_req_START (1)
#define SOC_CPU_PDC_BIG_COREPOWERUP_cpu_core5_pwrup_req_END (1)
#define SOC_CPU_PDC_BIG_COREPOWERUP_cpu_core6_pwrup_req_START (2)
#define SOC_CPU_PDC_BIG_COREPOWERUP_cpu_core6_pwrup_req_END (2)
#define SOC_CPU_PDC_BIG_COREPOWERUP_cpu_core7_pwrup_req_START (3)
#define SOC_CPU_PDC_BIG_COREPOWERUP_cpu_core7_pwrup_req_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_core4_pwrdn_req : 1;
        unsigned int cpu_core5_pwrdn_req : 1;
        unsigned int cpu_core6_pwrdn_req : 1;
        unsigned int cpu_core7_pwrdn_req : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_CPU_PDC_BIG_COREPOWERDN_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREPOWERDN_cpu_core4_pwrdn_req_START (0)
#define SOC_CPU_PDC_BIG_COREPOWERDN_cpu_core4_pwrdn_req_END (0)
#define SOC_CPU_PDC_BIG_COREPOWERDN_cpu_core5_pwrdn_req_START (1)
#define SOC_CPU_PDC_BIG_COREPOWERDN_cpu_core5_pwrdn_req_END (1)
#define SOC_CPU_PDC_BIG_COREPOWERDN_cpu_core6_pwrdn_req_START (2)
#define SOC_CPU_PDC_BIG_COREPOWERDN_cpu_core6_pwrdn_req_END (2)
#define SOC_CPU_PDC_BIG_COREPOWERDN_cpu_core7_pwrdn_req_START (3)
#define SOC_CPU_PDC_BIG_COREPOWERDN_cpu_core7_pwrdn_req_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_core4_pwr_stat : 4;
        unsigned int cpu_core5_pwr_stat : 4;
        unsigned int cpu_core6_pwr_stat : 4;
        unsigned int cpu_core7_pwr_stat : 4;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_BIG_COREPOWERSTAT_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_cpu_core4_pwr_stat_START (0)
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_cpu_core4_pwr_stat_END (3)
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_cpu_core5_pwr_stat_START (4)
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_cpu_core5_pwr_stat_END (7)
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_cpu_core6_pwr_stat_START (8)
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_cpu_core6_pwr_stat_END (11)
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_cpu_core7_pwr_stat_START (12)
#define SOC_CPU_PDC_BIG_COREPOWERSTAT_cpu_core7_pwr_stat_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_big_core_pwrup_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_BIG_COREPWRUPTIME_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREPWRUPTIME_cpu_big_core_pwrup_time_START (0)
#define SOC_CPU_PDC_BIG_COREPWRUPTIME_cpu_big_core_pwrup_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_big_core_pwrdn_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_BIG_COREPWRDNTIME_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREPWRDNTIME_cpu_big_core_pwrdn_time_START (0)
#define SOC_CPU_PDC_BIG_COREPWRDNTIME_cpu_big_core_pwrdn_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_big_core_iso_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_BIG_COREISOTIME_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREISOTIME_cpu_big_core_iso_time_START (0)
#define SOC_CPU_PDC_BIG_COREISOTIME_cpu_big_core_iso_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_big_core_rst_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_BIG_CORERSTTIME_UNION;
#endif
#define SOC_CPU_PDC_BIG_CORERSTTIME_cpu_big_core_rst_time_START (0)
#define SOC_CPU_PDC_BIG_CORERSTTIME_cpu_big_core_rst_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_big_core_urst_time : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_CPU_PDC_BIG_COREURSTTIME_UNION;
#endif
#define SOC_CPU_PDC_BIG_COREURSTTIME_cpu_big_core_urst_time_START (0)
#define SOC_CPU_PDC_BIG_COREURSTTIME_cpu_big_core_urst_time_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_little_pwrdn_stat : 1;
        unsigned int intr_middle_pwrdn_stat : 1;
        unsigned int intr_big_pwrdn_stat : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_CPU_PDC_MERGE_COREPWRINTSTAT_UNION;
#endif
#define SOC_CPU_PDC_MERGE_COREPWRINTSTAT_intr_little_pwrdn_stat_START (0)
#define SOC_CPU_PDC_MERGE_COREPWRINTSTAT_intr_little_pwrdn_stat_END (0)
#define SOC_CPU_PDC_MERGE_COREPWRINTSTAT_intr_middle_pwrdn_stat_START (1)
#define SOC_CPU_PDC_MERGE_COREPWRINTSTAT_intr_middle_pwrdn_stat_END (1)
#define SOC_CPU_PDC_MERGE_COREPWRINTSTAT_intr_big_pwrdn_stat_START (2)
#define SOC_CPU_PDC_MERGE_COREPWRINTSTAT_intr_big_pwrdn_stat_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int little_pwrdn_time_cfgcnt : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_CPU_PDC_LITTLE_PWRDNTIME_UNION;
#endif
#define SOC_CPU_PDC_LITTLE_PWRDNTIME_little_pwrdn_time_cfgcnt_START (0)
#define SOC_CPU_PDC_LITTLE_PWRDNTIME_little_pwrdn_time_cfgcnt_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int middle_pwrdn_time_cfgcnt : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_CPU_PDC_MIDDLE_PWRDNTIME_UNION;
#endif
#define SOC_CPU_PDC_MIDDLE_PWRDNTIME_middle_pwrdn_time_cfgcnt_START (0)
#define SOC_CPU_PDC_MIDDLE_PWRDNTIME_middle_pwrdn_time_cfgcnt_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int big_pwrdn_time_cfgcnt : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_CPU_PDC_BIG_PWRDNTIME_UNION;
#endif
#define SOC_CPU_PDC_BIG_PWRDNTIME_big_pwrdn_time_cfgcnt_START (0)
#define SOC_CPU_PDC_BIG_PWRDNTIME_big_pwrdn_time_cfgcnt_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_cpu_core0_gic_mask_1 : 1;
        unsigned int intr_cpu_core1_gic_mask_1 : 1;
        unsigned int intr_cpu_core2_gic_mask_1 : 1;
        unsigned int intr_cpu_core3_gic_mask_1 : 1;
        unsigned int intr_cpu_core4_0_gic_mask_1 : 1;
        unsigned int intr_cpu_core4_1_gic_mask_1 : 1;
        unsigned int intr_cpu_core5_0_gic_mask_1 : 1;
        unsigned int intr_cpu_core5_1_gic_mask_1 : 1;
        unsigned int intr_cpu_core6_0_gic_mask_1 : 1;
        unsigned int intr_cpu_core6_1_gic_mask_1 : 1;
        unsigned int intr_cpu_core7_0_gic_mask_1 : 1;
        unsigned int intr_cpu_core7_1_gic_mask_1 : 1;
        unsigned int reserved : 4;
        unsigned int bitmasken_340 : 16;
    } reg;
} SOC_CPU_PDC_CPU_COREGICMASK_1_UNION;
#endif
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core0_gic_mask_1_START (0)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core0_gic_mask_1_END (0)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core1_gic_mask_1_START (1)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core1_gic_mask_1_END (1)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core2_gic_mask_1_START (2)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core2_gic_mask_1_END (2)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core3_gic_mask_1_START (3)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core3_gic_mask_1_END (3)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core4_0_gic_mask_1_START (4)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core4_0_gic_mask_1_END (4)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core4_1_gic_mask_1_START (5)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core4_1_gic_mask_1_END (5)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core5_0_gic_mask_1_START (6)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core5_0_gic_mask_1_END (6)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core5_1_gic_mask_1_START (7)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core5_1_gic_mask_1_END (7)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core6_0_gic_mask_1_START (8)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core6_0_gic_mask_1_END (8)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core6_1_gic_mask_1_START (9)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core6_1_gic_mask_1_END (9)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core7_0_gic_mask_1_START (10)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core7_0_gic_mask_1_END (10)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core7_1_gic_mask_1_START (11)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_intr_cpu_core7_1_gic_mask_1_END (11)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_bitmasken_340_START (16)
#define SOC_CPU_PDC_CPU_COREGICMASK_1_bitmasken_340_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
