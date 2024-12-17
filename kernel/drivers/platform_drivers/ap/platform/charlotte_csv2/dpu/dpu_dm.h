#ifndef DPU_DM_H
#define DPU_DM_H 
#include "soc_dpu_interface.h"
#include "soc_dpu_define.h"
#define OPR_SDMA_NUM 4
#define OPR_DSD_NUM 0
#define OPR_SROT_NUM 2
#define OPR_HEMCD_NUM 1
#define OPR_UVUP_NUM 1
#define OPR_VSCF_NUM 2
#define OPR_ARSR_NUM 2
#define OPR_HDR_NUM 1
#define OPR_CLD_NUM 2
#define OPR_OV_NUM 4
#define OPR_PRELOAD_NUM 1
#define OPR_DPP_NUM 2
#define OPR_DSC_NUM 2
#define OPR_WCH_NUM 3
#define OPR_ITFSW_NUM 4
#define OPR_DDIC_NUM 0
#define DM_HDR_NUM 1
#define DM_UVUP_NUM 1
#define DM_SROT_NUM 2
#define DM_CLD_NUM 2
#define DM_SCL_NUM 5
#define DM_DPP_NUM 1
#define DM_DSC_NUM 1
#define DM_WCH_NUM 1
#define DM_ITFSW_NUM 1
#define DM_LAYER_NUM 18
#define POST_LAYER_ID 0x1F
#define INVALID_LAYER_ID 0x7F
#define DPU_DM_MAX_SCENE_COUNT 7
#define DPU_DM_MIN_SCENE_COUNT 0
struct dpu_dm_cmdlist_addr {
 DPU_DM_CMDLIST_ADDR_H_UNION cmdlist_h_addr;
 DPU_DM_CMDLIST_ADDR0_UNION cmdlist_next_addr;
 DPU_DM_CMDLIST_ADDR1_UNION cmdlist_hdr_addr;
 DPU_DM_CMDLIST_ADDR2_UNION cmdlist_uvup_addr;
 DPU_DM_CMDLIST_ADDR3_UNION cmdlist_cld0_addr;
 DPU_DM_CMDLIST_ADDR4_UNION cmdlist_cld1_addr;
 DPU_DM_CMDLIST_ADDR5_UNION cmdlist_scl0_addr;
 DPU_DM_CMDLIST_ADDR6_UNION cmdlist_scl1_addr;
 DPU_DM_CMDLIST_ADDR7_UNION cmdlist_scl2_addr;
 DPU_DM_CMDLIST_ADDR8_UNION cmdlist_scl3_addr;
 DPU_DM_CMDLIST_ADDR9_UNION cmdlist_scl4_addr;
 DPU_DM_CMDLIST_ADDR10_UNION cmdlist_dpp_addr;
 DPU_DM_CMDLIST_ADDR11_UNION cmdlist_ddic_addr;
 DPU_DM_CMDLIST_ADDR12_UNION cmdlist_dsc_addr;
 DPU_DM_CMDLIST_ADDR13_UNION cmdlist_wch_addr;
 DPU_DM_CMDLIST_ADDR14_UNION cmdlist_itfsw_addr;
 DPU_DM_CMDLIST_ADDR15_UNION cmdlist_hemcd_addr;
 unsigned int addr_reservied[3];
};
struct dpu_dm_scene_info {
 DPU_DM_SROT_NUMBER_UNION srot_number;
 DPU_DM_DDIC_NUMBER_UNION ddic_number;
 DPU_DM_LAYER_NUMBER_UNION layer_number;
 DPU_DM_SCENE_RESERVED_UNION scene_reserved;
};
struct dpu_dm_ov_info {
 DPU_DM_OV_IMG_WIDTH_UNION ov_img_width;
 DPU_DM_OV_BLAYER_ENDX_UNION ov_blayer_endx;
 DPU_DM_OV_BLAYER_ENDY_UNION ov_blayer_endy;
 DPU_DM_OV_BG_COLOR_RGB_UNION ov_bg_color_rgb;
 DPU_DM_OV_ORDER0_UNION ov_order0;
 DPU_DM_OV_RESERVED_0_UNION ov_reserved_0;
 DPU_DM_OV_RESERVED_1_UNION ov_reserved_1;
 DPU_DM_OV_RESERVED_2_UNION ov_reserved_2;
};
struct dpu_dm_hdr_info {
 DPU_DM_HDR_INPUT_IMG_WIDTH_UNION hdr_input_img_width;
 DPU_DM_HDR_OUTPUT_IMG_WIDTH_UNION hdr_output_img_width;
 DPU_DM_HDR_INPUT_FMT_UNION hdr_input_fmt;
 DPU_DM_HDR_RESERVED_UNION hdr_reservied;
};
struct dpu_dm_uvup_info {
 DPU_DM_UVUP_INPUT_IMG_WIDTH_UNION uvup_input_img_width;
 DPU_DM_UVUP_OUTPUT_IMG_WIDTH_UNION uvup_output_img_width;
 DPU_DM_UVUP_INPUT_FMT_UNION uvup_input_fmt;
 DPU_DM_UVUP_THRESHOLD_UNION uvup_threshold;
};
struct dpu_dm_srot0_info {
 DPU_DM_SROT0_INPUT_IMG_WIDTH_UNION srot0_input_img_width;
 DPU_DM_SROT0_OUTPUT_IMG_WIDTH_UNION srot0_output_img_width;
 DPU_DM_SROT0_INPUT_FMT_UNION srot0_input_fmt;
 DPU_DM_SROT0_RESERVED_UNION srot0_reserved;
};
struct dpu_dm_srot1_info {
 DPU_DM_SROT1_INPUT_IMG_WIDTH_UNION srot1_input_img_width;
 DPU_DM_SROT1_OUTPUT_IMG_WIDTH_UNION srot1_output_img_width;
 DPU_DM_SROT1_INPUT_FMT_UNION srot1_input_fmt;
 DPU_DM_SROT1_RESERVED_UNION srot1_reserved;
};
struct dpu_dm_cld0_info {
 DPU_DM_CLD0_INPUT_IMG_WIDTH_UNION cld0_input_img_width;
 DPU_DM_CLD0_OUTPUT_IMG_WIDTH_UNION cld0_output_img_width;
 DPU_DM_CLD0_INPUT_FMT_UNION cld0_input_fmt;
 DPU_DM_CLD0_RESERVED_UNION cld0_reserved;
};
struct dpu_dm_cld1_info {
 DPU_DM_CLD1_INPUT_IMG_WIDTH_UNION cld1_input_img_width;
 DPU_DM_CLD1_OUTPUT_IMG_WIDTH_UNION cld1_output_img_width;
 DPU_DM_CLD1_INPUT_FMT_UNION cld1_input_fmt;
 DPU_DM_CLD1_RESERVED_UNION cld1_reserved;
};
struct dpu_dm_hemcd_info {
 DPU_DM_HEMCD_INPUT_PLD_SIZE_UNION hemcd_input_pld_size;
 DPU_DM_HEMCD_OUTPUT_IMG_HEIGHT_UNION hemcd_output_img_height;
 DPU_DM_HEMCD_INPUT_FMT_UNION hemcd_input_fmt;
 DPU_DM_HEMCD_RESERVED_UNION hemcd_reserved;
};
struct dpu_dm_scl_info {
 DPU_DM_SCL0_INPUT_IMG_WIDTH_UNION scl0_input_img_width;
 DPU_DM_SCL0_OUTPUT_IMG_WIDTH_UNION scl0_output_img_width;
 DPU_DM_SCL0_TYPE_UNION scl0_type;
 DPU_DM_SCL0_THRESHOLD_UNION scl0_threshold;
};
struct dpu_dm_dpp_info {
 DPU_DM_DPP_INPUT_IMG_WIDTH_UNION dpp_input_img_width;
 DPU_DM_DPP_OUTPUT_IMG_WIDTH_UNION dpp_output_img_width;
 DPU_DM_DPP_SEL_UNION dpp_sel;
 DPU_DM_DPP_RESERVED_UNION dpp_reserved;
};
struct dpu_dm_ddic_info {
 DPU_DM_DDIC_INPUT_IMG_WIDTH_UNION ddic_input_img_width;
 DPU_DM_DDIC_OUTPUT_IMG_WIDTH_UNION ddic_output_img_width;
 DPU_DM_DDIC_INPUT_FMT_UNION ddic_input_fmt;
 DPU_DM_DDIC_DEBURNIN_WB_EN_UNION ddic_deburnin_wb_en;
 DPU_DM_DDIC_WB_OUTPUT_IMG_WIDTH_UNION ddic_wb_output_img_width;
 DPU_DM_DDIC_DEMURA_FMT_UNION ddic_demura_fmt;
 DPU_DM_DDIC_DEMURA_LUT_WIDTH_UNION ddic_demura_lut_width;
 DPU_DM_DDIC_RESERVED_2_UNION ddic_reserved_2;
};
struct dpu_dm_dsc_info {
 DPU_DM_DSC_INPUT_IMG_WIDTH_UNION dsc_input_img_width;
 DPU_DM_DSC_OUTPUT_IMG_WIDTH_UNION dsc_output_img_width;
 DPU_DM_DSC_SEL_UNION dsc_sel;
 DPU_DM_DSC_RESERVED_UNION dsc_reserved;
};
struct dpu_dm_wch_info {
 DPU_DM_WCH_INPUT_IMG_WIDTH_UNION wch_input_img_width;
 DPU_DM_WCH_INPUT_IMG_STRY_UNION wch_input_img_stry;
 DPU_DM_WCH_RESERVED_0_UNION wch_reserved_0;
 DPU_DM_WCH_RESERVED_1_UNION wch_reserved_1;
};
struct dpu_dm_itfsw_info {
 DPU_DM_ITF_INPUT_IMG_WIDTH_UNION itf_input_img_width;
 DPU_DM_ITF_INPUT_FMT_UNION itf_input_fmt;
 DPU_DM_ITF_RESERVED_0_UNION itf_reserved_0;
 DPU_DM_ITF_RESERVED_1_UNION itf_reserved_1;
};
struct dpu_dm_layer_info {
 DPU_DM_LAYER_HEIGHT_UNION layer_height;
 DPU_DM_LAYER_MASK_Y0_UNION layer_mask_y0;
 DPU_DM_LAYER_MASK_Y1_UNION layer_mask_y1;
 DPU_DM_LAYER_DMA_SEL_UNION layer_dma_sel;
 DPU_DM_LAYER_SBLK_TYPE_UNION layer_sblk_type;
 DPU_DM_LAYER_BOT_CROP_UNION layer_bot_crop;
 DPU_DM_LAYER_OV_CLIP_LEFT_UNION layer_ov_clip_left;
 DPU_DM_LAYER_OV_DFC_CFG_UNION layer_ov_dfc_cfg;
 DPU_DM_LAYER_OV_STARTY_UNION layer_ov_starty;
 DPU_DM_LAYER_OV_ENDY_UNION layer_ov_endy;
 DPU_DM_LAYER_OV_PATTERN_A_UNION layer_ov_pattern_a;
 DPU_DM_LAYER_OV_PATTERN_RGB_UNION layer_ov_pattern_rgb;
 DPU_DM_LAYER_OV_MODE_UNION layer_ov_mode;
 DPU_DM_LAYER_OV_ALPHA_UNION layer_ov_alpha;
 DPU_DM_LAYER_STRETCH3_LINE_UNION layer_stretch3_line_rsv;
 DPU_DM_LAYER_START_ADDR3_H_UNION layer_start_addr3_h;
 DPU_DM_LAYER_START_ADDR0_L_UNION layer_start_addr0_l;
 DPU_DM_LAYER_START_ADDR1_L_UNION layer_start_addr1_l;
 DPU_DM_LAYER_START_ADDR2_L_UNION layer_start_addr2_l;
 DPU_DM_LAYER_START_ADDR3_L_UNION layer_start_addr3_l;
 DPU_DM_LAYER_STRIDE0_UNION layer_stride0;
 DPU_DM_LAYER_STRIDE1_UNION layer_stride1;
 DPU_DM_LAYER_STRIDE2_UNION layer_stride2;
 DPU_DM_LAYER_RSV0_UNION layer_rsv0;
 DPU_DM_LAYER_RSV1_UNION layer_rsv1;
 DPU_DM_LAYER_RSV2_UNION layer_rsv2;
 DPU_DM_LAYER_RSV3_UNION layer_rsv3;
 DPU_DM_LAYER_RSV4_UNION layer_rsv4;
 DPU_DM_LAYER_RSV5_UNION layer_rsv5;
 DPU_DM_LAYER_RSV6_UNION layer_rsv6;
 DPU_DM_LAYER_RSV7_UNION layer_rsv7;
 DPU_DM_LAYER_RSV8_UNION layer_rsv8;
};
struct dpu_dm_param {
 struct dpu_dm_cmdlist_addr cmdlist_addr;
 struct dpu_dm_scene_info scene_info;
 struct dpu_dm_ov_info ov_info;
 struct dpu_dm_hdr_info hdr_info;
 struct dpu_dm_uvup_info uvup_info;
 struct dpu_dm_srot0_info srot_info[DM_SROT_NUM];
 struct dpu_dm_cld0_info cld_info[DM_CLD_NUM];
 struct dpu_dm_hemcd_info hemcd_info;
 struct dpu_dm_scl_info scl_info[DM_SCL_NUM];
 struct dpu_dm_dpp_info dpp_info;
 struct dpu_dm_ddic_info ddic_info;
 struct dpu_dm_dsc_info dsc_info[DM_DSC_NUM];
 struct dpu_dm_wch_info wch_info[DM_WCH_NUM];
 struct dpu_dm_itfsw_info itfsw_info;
 struct dpu_dm_layer_info layer_info[DM_LAYER_NUM];
};
#endif
