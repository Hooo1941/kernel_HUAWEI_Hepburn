#ifndef DPU_ITFSW_H
#define DPU_ITFSW_H 
#include "soc_dpu_interface.h"
#include "soc_dpu_define.h"
#define PIPE_SW_SIG_CTRL_N(base,n) (DPU_PIPE_SW_SIG_CTRL_0_ADDR(base) + 0x1C * n)
#define SW_POS_CTRL_SIG_EN_N(base,n) (DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_0_ADDR(base) + 0x1C * n)
#define PIPE_SW_DAT_CTRL_N(base,n) (DPU_PIPE_SW_DAT_CTRL_0_ADDR(base) + 0x1C * n)
#define SW_POS_CTRL_DAT_EN_N(base,n) (DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_0_ADDR(base) + 0x1C * n)
enum {
 PIPE_SW_POST_CH_DSI0 = 0,
 PIPE_SW_POST_CH_DSI1,
 PIPE_SW_POST_CH_DSI2,
 PIPE_SW_POST_CH_EDP,
 PIPE_SW_POST_CH_DP,
 PIPE_SW_POST_CH_MAX,
};
struct dpu_itfsw_params {
 DPU_ITF_CH_IMG_SIZE_UNION img_size;
 DPU_ITF_CH_DPP_CLIP_TOP_UNION dpp_clip_top;
 DPU_ITF_CH_DPP_CLIP_BOTTOM_UNION dpp_clip_bottom;
 DPU_ITF_CH_DPP_CLIP_LEFT_UNION dpp_clip_left;
 DPU_ITF_CH_DPP_CLIP_RIGHT_UNION dpp_clip_right;
 DPU_ITF_CH_DPP_CLIP_EN_UNION dpp_clip_en;
 DPU_ITF_CH_DPP_CLIP_DBG_UNION dpp_clip_dbg;
 DPU_ITF_CH_WB_UNPACK_POST_CLIP_DISP_SIZE_UNION wb_unpack_post_clip_disp_size;
 DPU_ITF_CH_WB_UNPACK_POST_CLIP_CTL_HRZ_UNION wb_unpack_post_clip_ctl_hrz;
 DPU_ITF_CH_WB_UNPACK_EN_UNION wb_unpack_en;
 DPU_ITF_CH_WB_UNPACK_SIZE_UNION wb_unpack_size;
 DPU_ITF_CH_WB_UNPACK_IMG_FMT_UNION wb_unpack_img_fmt;
 DPU_ITF_CH_DPP_CLRBAR_CTRL_UNION dpp_clrbar_ctrl;
 DPU_ITF_CH_DPP_CLRBAR_1ST_CLR_UNION dpp_clrbar_1st_clr;
 DPU_ITF_CH_DPP_CLRBAR_2ND_CLR_UNION dpp_clrbar_2nd_clr;
 DPU_ITF_CH_DPP_CLRBAR_3RD_CLR_UNION dpp_clrbar_3rd_clr;
 DPU_ITF_CH_ITFSW_RD_SHADOW_UNION itfsw_rd_shadow;
 DPU_ITF_CH_HIDIC_MODE_UNION hidic_mode;
 DPU_ITF_CH_HIDIC_DIMENSION_UNION hidic_dimension;
 DPU_ITF_CH_HIDIC_CMP_RESERVED_UNION hidic_cmp_reserved;
 DPU_ITF_CH_HIDIC_DBG_OUT_HI_UNION hidic_dbg_out_hi;
 DPU_ITF_CH_HIDIC_DBG_OUT_LO_UNION hidic_dbg_out_lo;
 DPU_ITF_CH_REG_CTRL_UNION reg_ctrl;
 DPU_ITF_CH_REG_CTRL_FLUSH_EN_UNION reg_ctrl_flush_en;
 DPU_ITF_CH_REG_CTRL_DEBUG_UNION reg_ctrl_debug;
 DPU_ITF_CH_REG_CTRL_STATE_UNION reg_ctrl_state;
 DPU_ITF_CH_ITFSW_DATA_SEL_UNION itfsw_data_sel;
 DPU_ITF_CH_CLRBAR_START_UNION clrbar_start;
 DPU_ITF_CH_ITFCH_CLK_CTL_UNION itfch_clk_ctl;
 DPU_ITF_CH_VSYNC_DELAY_CNT_CTRL_UNION vsync_delay_cnt_ctrl;
 DPU_ITF_CH_VSYNC_DELAY_CNT_UNION vsync_delay_cnt;
};
#endif
