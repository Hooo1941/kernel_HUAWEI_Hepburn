#ifndef __FUNC_READBACK_H__
#define __FUNC_READBACK_H__ 
typedef union
{
    struct
    {
        unsigned int vcpi_int_ve_eop : 1 ;
        unsigned int vcpi_int_vedu_slice_end : 1 ;
        unsigned int vcpi_int_ve_buffull : 1 ;
        unsigned int vcpi_int_ve_pbitsover : 1 ;
        unsigned int vcpi_int_line_pos : 1 ;
        unsigned int vcpi_int_ppfd_dec_err : 1 ;
        unsigned int vcpi_int_vedu_timeout : 1 ;
        unsigned int vcpi_cmdlst_sop : 1 ;
        unsigned int vcpi_cmdlst_eop : 1 ;
        unsigned int vcpi_soft_int0 : 1 ;
        unsigned int vcpi_soft_int1 : 1 ;
        unsigned int vcpi_soft_int2 : 1 ;
        unsigned int vcpi_soft_int3 : 1 ;
        unsigned int vcpi_lowlatency_slcint : 1 ;
        unsigned int reserved_0 : 18 ;
    } bits;
    unsigned int u32;
} U_FUNC_VCPI_INTSTAT;
typedef union
{
    struct
    {
        unsigned int vcpi_rint_ve_eop : 1 ;
        unsigned int vcpi_rint_vedu_slice_end : 1 ;
        unsigned int vcpi_rint_ve_buffull : 1 ;
        unsigned int vcpi_rint_ve_pbitsover : 1 ;
        unsigned int vcpi_rint_line_pos : 1 ;
        unsigned int vcpi_rint_ppfd_dec_err : 1 ;
        unsigned int vcpi_rint_vedu_timeout : 1 ;
        unsigned int vcpi_cmdlst_sop_raw : 1 ;
        unsigned int vcpi_cmdlst_eop_raw : 1 ;
        unsigned int vcpi_soft_rint0 : 1 ;
        unsigned int vcpi_soft_rint1 : 1 ;
        unsigned int vcpi_soft_rint2 : 1 ;
        unsigned int vcpi_soft_rint3 : 1 ;
        unsigned int vcpi_rint_lowlatency_slcint : 1 ;
        unsigned int reserved_0 : 18 ;
    } bits;
    unsigned int u32;
} U_FUNC_VCPI_RAWINT;
typedef union
{
    struct
    {
        unsigned int bus_idle_flag : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_FUNC_VCPI_BUS_IDLE_FLAG;
typedef union
{
    struct
    {
        unsigned int pme_madi_sum : 25 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MADI_SUM;
typedef union
{
    struct
    {
        unsigned int pme_madp_sum : 25 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MADP_SUM;
typedef union
{
    struct
    {
        unsigned int pme_madi_num : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MADI_NUM;
typedef union
{
    struct
    {
        unsigned int pme_madp_num : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MADP_NUM;
typedef union
{
    struct
    {
        unsigned int large_sad_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_LARGE_SAD_SUM;
typedef union
{
    struct
    {
        unsigned int low_luma_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_LOW_LUMA_SUM;
typedef union
{
    struct
    {
        unsigned int chroma_prot_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_CHROMA_SCENE_SUM;
typedef union
{
    struct
    {
        unsigned int move_scene_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MOVE_SCENE_SUM;
typedef union
{
    struct
    {
        unsigned int skin_region_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_SKIN_REGION_SUM;
typedef union
{
    struct
    {
        unsigned int vcpi_bggen_block_count : 18 ;
        unsigned int reserved_0 : 14 ;
    } bits;
    unsigned int u32;
} U_FUNC_BGSTR_BLOCK_COUNT;
typedef union
{
    struct
    {
        unsigned int vcpi_frame_bgm_dist : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_FUNC_BGSTR_FRAME_BGM_DIST;
typedef union
{
    struct
    {
        unsigned int slc_len0 : 29 ;
        unsigned int reserved_0 : 3 ;
    } bits;
    unsigned int u32;
} U_FUNC_VLCST_DSRPTR00;
typedef union
{
    struct
    {
        unsigned int invalidnum0 : 7 ;
        unsigned int reserved0 : 18 ;
        unsigned int nal_type0 : 3 ;
        unsigned int Slice_type0 : 2 ;
        unsigned int IsLastTile0 : 1 ;
        unsigned int islastslc0 : 1 ;
    } bits;
    unsigned int u32;
} U_FUNC_VLCST_DSRPTR01;
typedef union
{
    struct
    {
        unsigned int mrg_skip_opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_RESERVED_FUNC_PPFD;
typedef union
{
    struct
    {
        unsigned int opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_8X8_CNT;
typedef union
{
    struct
    {
        unsigned int intra_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTRA_OPT_8X8_CNT;
typedef union
{
    struct
    {
        unsigned int pcm_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT;
typedef union
{
    struct
    {
        unsigned int inter_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_OPT_8X8_CNT;
typedef union
{
    struct
    {
        unsigned int fme_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_FME_OPT_8X8_CNT;
typedef union
{
    struct
    {
        unsigned int mrg_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SELINTER_MERGE_OPT_8X8_CNT;
typedef union
{
    struct
    {
        unsigned int mrg_skip_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT;
typedef union
{
    struct
    {
        unsigned int opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_16X16_CNT;
typedef union
{
    struct
    {
        unsigned int intra_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTRA_OPT_16X16_CNT;
typedef union
{
    struct
    {
        unsigned int opt_4x4_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_4X4_CNT;
typedef union
{
    struct
    {
        unsigned int inter_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_OPT_16X16_CNT;
typedef union
{
    struct
    {
        unsigned int fme_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_FME_OPT_16X16_CNT;
typedef union
{
    struct
    {
        unsigned int mrg_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT;
typedef union
{
    struct
    {
        unsigned int mrg_skip_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT;
typedef union
{
    struct
    {
        unsigned int opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_32X32_CNT;
typedef union
{
    struct
    {
        unsigned int intra_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTRA_OPT_32X32_CNT;
typedef union
{
    struct
    {
        unsigned int inter_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_OPT_32X32_CNT;
typedef union
{
    struct
    {
        unsigned int fme_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_FME_OPT_32X32_CNT;
typedef union
{
    struct
    {
        unsigned int mrg_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT;
typedef union
{
    struct
    {
        unsigned int mrg_skip_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT;
typedef union
{
    struct
    {
        unsigned int opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_64X64_CNT;
typedef union
{
    struct
    {
        unsigned int fme_opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_FME_OPT_64X64_CNT;
typedef union
{
    struct
    {
        unsigned int mrg_opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT;
typedef union
{
    struct
    {
        unsigned int mrg_skip_opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT;
typedef union
{
    struct
    {
        unsigned int total_luma_qp : 26 ;
        unsigned int reserved_0 : 6 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_TOTAL_LUMA_QP;
typedef union
{
    struct
    {
        unsigned int min_luma_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int max_luma_qp : 6 ;
        unsigned int reserved_1 : 18 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_MAX_MIN_LUMA_QP;
typedef union
{
    struct
    {
        unsigned int luma_qp0_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP0_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp1_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP1_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp2_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP2_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp3_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP3_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp4_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP4_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp5_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP5_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp6_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP6_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp7_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP7_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp8_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP8_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp9_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP9_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp10_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP10_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp11_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP11_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp12_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP12_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp13_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP13_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp14_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP14_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp15_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP15_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp16_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP16_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp17_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP17_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp18_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP18_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp19_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP19_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp20_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP20_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp21_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP21_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp22_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP22_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp23_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP23_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp24_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP24_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp25_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP25_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp26_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP26_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp27_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP27_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp28_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP28_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp29_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP29_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp30_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP30_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp31_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP31_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp32_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP32_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp33_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP33_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp34_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP34_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp35_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP35_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp36_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP36_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp37_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP37_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp38_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP38_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp39_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP39_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp40_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP40_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp41_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP41_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp42_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP42_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp43_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP43_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp44_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP44_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp45_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP45_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp46_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP46_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp47_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP47_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp48_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP48_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp49_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP49_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp50_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP50_CNT;
typedef union
{
    struct
    {
        unsigned int luma_qp51_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP51_CNT;
typedef union
{
    struct
    {
        unsigned int sao2vcpi_saooff_num_luma : 14 ;
        unsigned int reserved_0 : 2 ;
        unsigned int sao2vcpi_saooff_num_chroma : 14 ;
        unsigned int reserved_1 : 2 ;
    } bits;
    unsigned int u32;
} U_FUNC_SAO_OFF_NUM;
typedef union
{
    struct
    {
        unsigned int sao2vcpi_mse_cnt : 18 ;
        unsigned int sao2vcpi_mse_sumh : 3 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SAO_MSE_CNT;
typedef union
{
    struct
    {
        unsigned int sao2vcpi_mse_max : 28 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_FUNC_SAO_MSE_MAX;
typedef struct
{
    volatile U_FUNC_VCPI_INTSTAT FUNC_VCPI_INTSTAT;
    volatile U_FUNC_VCPI_RAWINT FUNC_VCPI_RAWINT;
    volatile unsigned int FUNC_VCPI_VEDU_TIMER;
    volatile unsigned int FUNC_VCPI_IDLE_TIMER;
    volatile U_FUNC_VCPI_BUS_IDLE_FLAG FUNC_VCPI_BUS_IDLE_FLAG;
    volatile unsigned int RESERVED_FUNC_VCPI[11];
    volatile unsigned int RESERVED_FUNC_VCTRL[16];
    volatile unsigned int RESERVED_FUNC_QPGLD[16];
    volatile unsigned int RESERVED_FUNC_CURLD[16];
    volatile unsigned int RESERVED_FUNC_NBI[16];
    volatile unsigned int RESERVED_FUNC_PMELD[16];
    volatile unsigned int RESERVED_FUNC_PMEINFO_LD[16];
    volatile U_FUNC_PME_MADI_SUM FUNC_PME_MADI_SUM;
    volatile U_FUNC_PME_MADP_SUM FUNC_PME_MADP_SUM;
    volatile U_FUNC_PME_MADI_NUM FUNC_PME_MADI_NUM;
    volatile U_FUNC_PME_MADP_NUM FUNC_PME_MADP_NUM;
    volatile U_FUNC_PME_LARGE_SAD_SUM FUNC_PME_LARGE_SAD_SUM;
    volatile U_FUNC_PME_LOW_LUMA_SUM FUNC_PME_LOW_LUMA_SUM;
    volatile U_FUNC_PME_CHROMA_SCENE_SUM FUNC_PME_CHROMA_SCENE_SUM;
    volatile U_FUNC_PME_MOVE_SCENE_SUM FUNC_PME_MOVE_SCENE_SUM;
    volatile U_FUNC_PME_SKIN_REGION_SUM FUNC_PME_SKIN_REGION_SUM;
    volatile unsigned int RESERVED_FUNC_PME[7];
    volatile unsigned int RESERVED_FUNC_PMEST[16];
    volatile unsigned int RESERVED_FUNC_PMEINFO_ST[16];
    volatile U_FUNC_BGSTR_BLOCK_COUNT FUNC_BGSTR_BLOCK_COUNT;
    volatile U_FUNC_BGSTR_FRAME_BGM_DIST FUNC_BGSTR_FRAME_BGM_DIST;
    volatile unsigned int RESERVED_FUNC_BGSTR[14];
    volatile unsigned int RESERVED_FUNC_QPG[16];
    volatile unsigned int RESERVED_FUNC_REFLD[16];
    volatile unsigned int RESERVED_FUNC_PINTRA[16];
    volatile unsigned int RESERVED_FUNC_IME[16];
    volatile unsigned int RESERVED_FUNC_FME[16];
    volatile unsigned int RESERVED_FUNC_MRG[16];
    volatile unsigned int RESERVED_FUNC_BGGEN[16];
    volatile unsigned int RESERVED_FUNC_INTRA[16];
    volatile unsigned int RESERVED_FUNC_NU0[24];
    volatile unsigned int FUNC_PMV_MV0_COUNT;
    volatile unsigned int FUNC_PMV_MV1_COUNT;
    volatile unsigned int FUNC_PMV_MV2_COUNT;
    volatile unsigned int FUNC_PMV_MV3_COUNT;
    volatile unsigned int FUNC_PMV_MV4_COUNT;
    volatile unsigned int FUNC_PMV_MV5_COUNT;
    volatile unsigned int FUNC_PMV_MV6_COUNT;
    volatile unsigned int FUNC_PMV_MV7_COUNT;
    volatile unsigned int FUNC_PMV_MV8_COUNT;
    volatile unsigned int FUNC_PMV_MV9_COUNT;
    volatile unsigned int FUNC_PMV_MV10_COUNT;
    volatile unsigned int FUNC_PMV_MV11_COUNT;
    volatile unsigned int FUNC_PMV_MV12_COUNT;
    volatile unsigned int FUNC_PMV_MV13_COUNT;
    volatile unsigned int FUNC_PMV_MV14_COUNT;
    volatile unsigned int FUNC_PMV_MV15_COUNT;
    volatile unsigned int FUNC_PMV_MV16_COUNT;
    volatile unsigned int FUNC_PMV_MV17_COUNT;
    volatile unsigned int FUNC_PMV_MV18_COUNT;
    volatile unsigned int FUNC_PMV_MV19_COUNT;
    volatile unsigned int FUNC_PMV_MV20_COUNT;
    volatile unsigned int FUNC_PMV_MV21_COUNT;
    volatile unsigned int FUNC_PMV_MV22_COUNT;
    volatile unsigned int RESERVED_FUNC_DISABLE;
    volatile unsigned int RESERVED_FUNC_TQITQ0[16];
    volatile unsigned int RESERVED_FUNC_NU2[16];
    volatile unsigned int RESERVED_FUNC_BGLD[16];
    volatile unsigned int RESERVED_FUNC_LFLDST[16];
    volatile unsigned int RESERVED_FUNC_DBLK[16];
    volatile unsigned int RESERVED_FUNC_NU4[16];
    volatile unsigned int RESERVED_FUNC_RECST[16];
    volatile unsigned int RESERVED_FUNC_PACK0[16];
    volatile unsigned int RESERVED_FUNC_PACK1[16];
    volatile unsigned int FUNC_CABAC_PIC_STRMSIZE;
    volatile unsigned int FUNC_CABAC_BIN_NUM;
    volatile unsigned int FUNC_CABAC_RES_BIN_NUM;
    volatile unsigned int FUNC_CABAC_HDR_BIN_NUM;
    volatile unsigned int FUNC_CABAC_MOVE_SCENE_BITS;
    volatile unsigned int FUNC_CABAC_STREDGE_MOVE_BITS;
    volatile unsigned int FUNC_CABAC_SKIN_BITS;
    volatile unsigned int FUNC_CABAC_LOWLUMA_BITS;
    volatile unsigned int FUNC_CABAC_CHROMAPROT_BITS;
    volatile unsigned int RESERVED_FUNC_CABAC[7];
    volatile unsigned int FUNC_VLC_CABAC_HRD_BITS;
    volatile unsigned int FUNC_VLC_CABAC_RES_BITS;
    volatile unsigned int FUNC_VLC_CAVLC_HRD_BITS;
    volatile unsigned int FUNC_VLC_CAVLC_RES_BITS;
    volatile unsigned int FUNC_VLC_PIC_STRMSIZE;
    volatile unsigned int RESERVED_FUNC_VLC[11];
    volatile unsigned int FUNC_VLCST_SLC_LEN_CNT;
    volatile unsigned int FUNC_VLCST_WPTR;
    volatile unsigned int FUNC_VLCST_RPTR;
    volatile unsigned int FUNC_VLCST_STRM_LEN_CNT;
    volatile U_FUNC_VLCST_DSRPTR00 FUNC_VLCST_DSRPTR00[16];
    volatile U_FUNC_VLCST_DSRPTR01 FUNC_VLCST_DSRPTR01[16];
    volatile unsigned int RESERVED_FUNC_VLCST[92];
    volatile unsigned int RESERVED_FUNC_EMAR[16];
    volatile U_RESERVED_FUNC_PPFD RESERVED_FUNC_PPFD[16];
    volatile U_FUNC_SEL_OPT_8X8_CNT FUNC_SEL_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_8X8_CNT FUNC_SEL_INTRA_OPT_8X8_CNT;
    volatile unsigned int FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT FUNC_SEL_INTRA_PCM_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_OPT_8X8_CNT FUNC_SEL_INTER_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_8X8_CNT FUNC_SEL_INTER_FME_OPT_8X8_CNT;
    volatile U_FUNC_SELINTER_MERGE_OPT_8X8_CNT FUNC_SELINTER_MERGE_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT FUNC_SEL_INTER_SKIP_OPT_8X8_CNT;
    volatile U_FUNC_SEL_OPT_16X16_CNT FUNC_SEL_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_16X16_CNT FUNC_SEL_INTRA_OPT_16X16_CNT;
    volatile U_FUNC_SEL_OPT_4X4_CNT FUNC_SEL_OPT_4X4_CNT;
    volatile unsigned int RESERVED0_FUNC_SEL;
    volatile U_FUNC_SEL_INTER_OPT_16X16_CNT FUNC_SEL_INTER_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_16X16_CNT FUNC_SEL_INTER_FME_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT FUNC_SEL_INTER_MERGE_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT FUNC_SEL_INTER_SKIP_OPT_16X16_CNT;
    volatile U_FUNC_SEL_OPT_32X32_CNT FUNC_SEL_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_32X32_CNT FUNC_SEL_INTRA_OPT_32X32_CNT;
    volatile unsigned int RESERVED1_FUNC_SEL;
    volatile U_FUNC_SEL_INTER_OPT_32X32_CNT FUNC_SEL_INTER_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_32X32_CNT FUNC_SEL_INTER_FME_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT FUNC_SEL_INTER_MERGE_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT FUNC_SEL_INTER_SKIP_OPT_32X32_CNT;
    volatile U_FUNC_SEL_OPT_64X64_CNT FUNC_SEL_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_64X64_CNT FUNC_SEL_INTER_FME_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT FUNC_SEL_INTER_MERGE_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT FUNC_SEL_INTER_SKIP_OPT_64X64_CNT;
    volatile U_FUNC_SEL_TOTAL_LUMA_QP FUNC_SEL_TOTAL_LUMA_QP;
    volatile U_FUNC_SEL_MAX_MIN_LUMA_QP FUNC_SEL_MAX_MIN_LUMA_QP;
    volatile unsigned int RESERVED2_FUNC_SEL[35];
    volatile U_FUNC_SEL_LUMA_QP0_CNT FUNC_SEL_LUMA_QP0_CNT;
    volatile U_FUNC_SEL_LUMA_QP1_CNT FUNC_SEL_LUMA_QP1_CNT;
    volatile U_FUNC_SEL_LUMA_QP2_CNT FUNC_SEL_LUMA_QP2_CNT;
    volatile U_FUNC_SEL_LUMA_QP3_CNT FUNC_SEL_LUMA_QP3_CNT;
    volatile U_FUNC_SEL_LUMA_QP4_CNT FUNC_SEL_LUMA_QP4_CNT;
    volatile U_FUNC_SEL_LUMA_QP5_CNT FUNC_SEL_LUMA_QP5_CNT;
    volatile U_FUNC_SEL_LUMA_QP6_CNT FUNC_SEL_LUMA_QP6_CNT;
    volatile U_FUNC_SEL_LUMA_QP7_CNT FUNC_SEL_LUMA_QP7_CNT;
    volatile U_FUNC_SEL_LUMA_QP8_CNT FUNC_SEL_LUMA_QP8_CNT;
    volatile U_FUNC_SEL_LUMA_QP9_CNT FUNC_SEL_LUMA_QP9_CNT;
    volatile U_FUNC_SEL_LUMA_QP10_CNT FUNC_SEL_LUMA_QP10_CNT;
    volatile U_FUNC_SEL_LUMA_QP11_CNT FUNC_SEL_LUMA_QP11_CNT;
    volatile U_FUNC_SEL_LUMA_QP12_CNT FUNC_SEL_LUMA_QP12_CNT;
    volatile U_FUNC_SEL_LUMA_QP13_CNT FUNC_SEL_LUMA_QP13_CNT;
    volatile U_FUNC_SEL_LUMA_QP14_CNT FUNC_SEL_LUMA_QP14_CNT;
    volatile U_FUNC_SEL_LUMA_QP15_CNT FUNC_SEL_LUMA_QP15_CNT;
    volatile U_FUNC_SEL_LUMA_QP16_CNT FUNC_SEL_LUMA_QP16_CNT;
    volatile U_FUNC_SEL_LUMA_QP17_CNT FUNC_SEL_LUMA_QP17_CNT;
    volatile U_FUNC_SEL_LUMA_QP18_CNT FUNC_SEL_LUMA_QP18_CNT;
    volatile U_FUNC_SEL_LUMA_QP19_CNT FUNC_SEL_LUMA_QP19_CNT;
    volatile U_FUNC_SEL_LUMA_QP20_CNT FUNC_SEL_LUMA_QP20_CNT;
    volatile U_FUNC_SEL_LUMA_QP21_CNT FUNC_SEL_LUMA_QP21_CNT;
    volatile U_FUNC_SEL_LUMA_QP22_CNT FUNC_SEL_LUMA_QP22_CNT;
    volatile U_FUNC_SEL_LUMA_QP23_CNT FUNC_SEL_LUMA_QP23_CNT;
    volatile U_FUNC_SEL_LUMA_QP24_CNT FUNC_SEL_LUMA_QP24_CNT;
    volatile U_FUNC_SEL_LUMA_QP25_CNT FUNC_SEL_LUMA_QP25_CNT;
    volatile U_FUNC_SEL_LUMA_QP26_CNT FUNC_SEL_LUMA_QP26_CNT;
    volatile U_FUNC_SEL_LUMA_QP27_CNT FUNC_SEL_LUMA_QP27_CNT;
    volatile U_FUNC_SEL_LUMA_QP28_CNT FUNC_SEL_LUMA_QP28_CNT;
    volatile U_FUNC_SEL_LUMA_QP29_CNT FUNC_SEL_LUMA_QP29_CNT;
    volatile U_FUNC_SEL_LUMA_QP30_CNT FUNC_SEL_LUMA_QP30_CNT;
    volatile U_FUNC_SEL_LUMA_QP31_CNT FUNC_SEL_LUMA_QP31_CNT;
    volatile U_FUNC_SEL_LUMA_QP32_CNT FUNC_SEL_LUMA_QP32_CNT;
    volatile U_FUNC_SEL_LUMA_QP33_CNT FUNC_SEL_LUMA_QP33_CNT;
    volatile U_FUNC_SEL_LUMA_QP34_CNT FUNC_SEL_LUMA_QP34_CNT;
    volatile U_FUNC_SEL_LUMA_QP35_CNT FUNC_SEL_LUMA_QP35_CNT;
    volatile U_FUNC_SEL_LUMA_QP36_CNT FUNC_SEL_LUMA_QP36_CNT;
    volatile U_FUNC_SEL_LUMA_QP37_CNT FUNC_SEL_LUMA_QP37_CNT;
    volatile U_FUNC_SEL_LUMA_QP38_CNT FUNC_SEL_LUMA_QP38_CNT;
    volatile U_FUNC_SEL_LUMA_QP39_CNT FUNC_SEL_LUMA_QP39_CNT;
    volatile U_FUNC_SEL_LUMA_QP40_CNT FUNC_SEL_LUMA_QP40_CNT;
    volatile U_FUNC_SEL_LUMA_QP41_CNT FUNC_SEL_LUMA_QP41_CNT;
    volatile U_FUNC_SEL_LUMA_QP42_CNT FUNC_SEL_LUMA_QP42_CNT;
    volatile U_FUNC_SEL_LUMA_QP43_CNT FUNC_SEL_LUMA_QP43_CNT;
    volatile U_FUNC_SEL_LUMA_QP44_CNT FUNC_SEL_LUMA_QP44_CNT;
    volatile U_FUNC_SEL_LUMA_QP45_CNT FUNC_SEL_LUMA_QP45_CNT;
    volatile U_FUNC_SEL_LUMA_QP46_CNT FUNC_SEL_LUMA_QP46_CNT;
    volatile U_FUNC_SEL_LUMA_QP47_CNT FUNC_SEL_LUMA_QP47_CNT;
    volatile U_FUNC_SEL_LUMA_QP48_CNT FUNC_SEL_LUMA_QP48_CNT;
    volatile U_FUNC_SEL_LUMA_QP49_CNT FUNC_SEL_LUMA_QP49_CNT;
    volatile U_FUNC_SEL_LUMA_QP50_CNT FUNC_SEL_LUMA_QP50_CNT;
    volatile U_FUNC_SEL_LUMA_QP51_CNT FUNC_SEL_LUMA_QP51_CNT;
    volatile unsigned int RESERVED3_FUNC_SEL[12];
    volatile unsigned int RESERVED_FUNC_TBLDST[128];
    volatile U_FUNC_SAO_OFF_NUM FUNC_SAO_OFF_NUM;
    volatile unsigned int FUNC_SAO_MSE_SUM;
    volatile U_FUNC_SAO_MSE_CNT FUNC_SAO_MSE_CNT;
    volatile U_FUNC_SAO_MSE_MAX FUNC_SAO_MSE_MAX;
    volatile unsigned int FUNC_SAO_SSD_AREA0_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA1_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA2_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA3_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA4_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA5_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA6_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA7_SUM;
    volatile unsigned int RESERVED_FUNC_SAO[20];
    volatile unsigned int RESERVED_FUNC_NU3[48];
} S_FUNC_READBACK_REGS_TYPE;
#endif
