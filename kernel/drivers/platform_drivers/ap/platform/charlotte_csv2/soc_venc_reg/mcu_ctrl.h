#ifndef __MCU_CTRL_H__
#define __MCU_CTRL_H__ 
typedef union
{
    struct
    {
        unsigned int cu16_qp0 : 6 ;
        unsigned int cu16_qp1 : 6 ;
        unsigned int cu16_qp2 : 6 ;
        unsigned int cu16_qp3 : 6 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_CU16_QP_GRP0;
typedef union
{
    struct
    {
        unsigned int cu16_qp4 : 6 ;
        unsigned int cu16_qp5 : 6 ;
        unsigned int cu16_qp6 : 6 ;
        unsigned int cu16_qp7 : 6 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_CU16_QP_GRP1;
typedef union
{
    struct
    {
        unsigned int cu16_qp8 : 6 ;
        unsigned int cu16_qp9 : 6 ;
        unsigned int cu16_qp10 : 6 ;
        unsigned int cu16_qp11 : 6 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_CU16_QP_GRP2;
typedef union
{
    struct
    {
        unsigned int cu16_qp12 : 6 ;
        unsigned int cu16_qp13 : 6 ;
        unsigned int cu16_qp14 : 6 ;
        unsigned int cu16_qp15 : 6 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_CU16_QP_GRP3;
typedef union
{
    struct
    {
        unsigned int cu32_qp0 : 6 ;
        unsigned int cu32_qp1 : 6 ;
        unsigned int cu32_qp2 : 6 ;
        unsigned int cu32_qp3 : 6 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_CU32_QP;
typedef union
{
    struct
    {
        unsigned int lcu_qp : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_LCU_QP;
typedef union
{
    struct
    {
        unsigned int lcu_int_mask : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_MCU_LCU_INTMASK;
typedef union
{
    struct
    {
        unsigned int clr_lcu_int : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_MCU_LCU_INTCLR;
typedef union
{
    struct
    {
        unsigned int mcu_enable_ve_eop : 1 ;
        unsigned int mcu_enable_vedu_slice_end : 1 ;
        unsigned int mcu_enable_ve_buffull : 1 ;
        unsigned int mcu_enable_ve_pbitsover : 1 ;
        unsigned int mcu_enable_line_pos : 1 ;
        unsigned int mcu_enable_ppdf_dec_err : 1 ;
        unsigned int mcu_enable_vedu_timeout : 1 ;
        unsigned int mcu_cmdlst_sop_mask : 1 ;
        unsigned int mcu_cmdlst_eop_mask : 1 ;
        unsigned int mcu_soft_int0_mask : 1 ;
        unsigned int mcu_soft_int1_mask : 1 ;
        unsigned int mcu_soft_int2_mask : 1 ;
        unsigned int mcu_soft_int3_mask : 1 ;
        unsigned int mcu_lowlatency_slcint_mask : 1 ;
        unsigned int reserved_0 : 18 ;
    } bits;
    unsigned int u32;
} U_VEDU_MCU_INTMASK;
typedef union
{
    struct
    {
        unsigned int mcu_clr_ve_eop : 1 ;
        unsigned int mcu_clr_vedu_slice_end : 1 ;
        unsigned int mcu_clr_ve_buffull : 1 ;
        unsigned int mcu_clr_ve_pbitsover : 1 ;
        unsigned int mcu_clr_line_pos : 1 ;
        unsigned int mcu_clr_ppdf_dec_err : 1 ;
        unsigned int mcu_clr_vedu_timeout : 1 ;
        unsigned int mcu_clr_cmdlst_sop : 1 ;
        unsigned int mcu_clr_cmdlst_eop : 1 ;
        unsigned int mcu_clr_soft_int0 : 1 ;
        unsigned int mcu_clr_soft_int1 : 1 ;
        unsigned int mcu_clr_soft_int2 : 1 ;
        unsigned int mcu_clr_soft_int3 : 1 ;
        unsigned int mcu_clr_lowlatency_slcint : 1 ;
        unsigned int reserved_0 : 18 ;
    } bits;
    unsigned int u32;
} U_VEDU_MCU_INTCLR;
typedef union
{
    struct
    {
        unsigned int lcu_int : 1 ;
        unsigned int lcu_rint : 1 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_MCU_LCU_INTSTAT;
typedef union
{
    struct
    {
        unsigned int mcu_int_ve_eop : 1 ;
        unsigned int mcu_int_vedu_slice_end : 1 ;
        unsigned int mcu_int_ve_buffull : 1 ;
        unsigned int mcu_int_ve_pbitsover : 1 ;
        unsigned int mcu_int_line_pos : 1 ;
        unsigned int mcu_int_ppfd_dec_err : 1 ;
        unsigned int mcu_int_vedu_timeout : 1 ;
        unsigned int mcu_cmdlst_sop : 1 ;
        unsigned int mcu_cmdlst_eop : 1 ;
        unsigned int mcu_soft_int0 : 1 ;
        unsigned int mcu_soft_int1 : 1 ;
        unsigned int mcu_soft_int2 : 1 ;
        unsigned int mcu_soft_int3 : 1 ;
        unsigned int mcu_lowlatency_slcint : 1 ;
        unsigned int reserved_0 : 18 ;
    } bits;
    unsigned int u32;
} U_FUNC_MCU_INTSTAT;
typedef union
{
    struct
    {
        unsigned int mcu_rint_ve_eop : 1 ;
        unsigned int mcu_rint_vedu_slice_end : 1 ;
        unsigned int mcu_rint_ve_buffull : 1 ;
        unsigned int mcu_rint_ve_pbitsover : 1 ;
        unsigned int mcu_rint_line_pos : 1 ;
        unsigned int mcu_rint_ppfd_dec_err : 1 ;
        unsigned int mcu_rint_vedu_timeout : 1 ;
        unsigned int mcu_cmdlst_sop_raw : 1 ;
        unsigned int mcu_cmdlst_eop_raw : 1 ;
        unsigned int mcu_soft_rint0 : 1 ;
        unsigned int mcu_soft_rint1 : 1 ;
        unsigned int mcu_soft_rint2 : 1 ;
        unsigned int mcu_soft_rint3 : 1 ;
        unsigned int mcu_rint_lowlatency_slcint : 1 ;
        unsigned int reserved_0 : 18 ;
    } bits;
    unsigned int u32;
} U_FUNC_MCU_RAWINT;
typedef union
{
    struct
    {
        unsigned int mcu_eop : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_MCU_EOP;
typedef struct
{
    volatile U_CU16_QP_GRP0 CU16_QP_GRP0;
    volatile U_CU16_QP_GRP1 CU16_QP_GRP1;
    volatile U_CU16_QP_GRP2 CU16_QP_GRP2;
    volatile U_CU16_QP_GRP3 CU16_QP_GRP3;
    volatile U_CU32_QP CU32_QP;
    volatile U_LCU_QP LCU_QP;
    volatile unsigned int RESERVED0[2];
    volatile U_MCU_LCU_INTMASK MCU_LCU_INTMASK;
    volatile U_MCU_LCU_INTCLR MCU_LCU_INTCLR;
    volatile unsigned int RESERVED1[2];
    volatile U_VEDU_MCU_INTMASK VEDU_MCU_INTMASK;
    volatile U_VEDU_MCU_INTCLR VEDU_MCU_INTCLR;
    volatile unsigned int RESERVED6[2];
    volatile unsigned int BEST_BITS;
    volatile unsigned int REAL_BITS;
    volatile unsigned int TEXTURE_COMPLEX_16[16];
    volatile unsigned int INTRA_BEST_BITS;
    volatile U_MCU_LCU_INTSTAT MCU_LCU_INTSTAT;
    volatile unsigned int RESERVED2[28];
    volatile unsigned int LCU_INTRA_BEST_BITS_LAST1;
    volatile unsigned int LCU_INTRA_BEST_BITS_LAST2;
    volatile unsigned int LCU_INTRA_BEST_BITS_LAST3;
    volatile unsigned int LCU_BEST_BITS_LAST1;
    volatile unsigned int LCU_BEST_BITS_LAST2;
    volatile unsigned int LCU_BEST_BITS_LAST3;
    volatile unsigned int LCU_REAL_BITS_LAST1;
    volatile unsigned int LCU_REAL_BITS_LAST2;
    volatile unsigned int LCU_REAL_BITS_LAST3;
    volatile unsigned int LCU_REAL_BITS_LAST4;
    volatile unsigned int LCU_REAL_BITS_LAST5;
    volatile unsigned int RESERVED3[21];
    volatile U_FUNC_MCU_INTSTAT FUNC_MCU_INTSTAT;
    volatile U_FUNC_MCU_RAWINT FUNC_MCU_RAWINT;
    volatile unsigned int FUNC_DBG0;
    volatile unsigned int FUNC_DBG1;
    volatile unsigned int RESERVED4[12];
    volatile U_MCU_EOP MCU_EOP;
    volatile unsigned int RESERVED5[911];
} S_MCU_CTRL_REGS_TYPE;
#endif
