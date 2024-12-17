/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hdmirx_video_dsc.h"
#include "hdmirx_common.h"
#include <linux/delay.h>
#include "hdmirx_init.h"
#include "control/hdmirx_ctrl.h"

typedef enum {
	DSC_PROC_HW_STATUS,
	DSC_PROC_PPS,
	DSC_PROC_MAX
} dsc_proc_name;

static hdmirx_dsc_ctx g_hdmirx_dsc_ctx;

static hdmirx_dsc_ctx *hdmirx_dsc_get_ctx()
{
	return &g_hdmirx_dsc_ctx;
}

int hdmirx_video_dsc_init(struct hdmirx_ctrl_st *hdmirx)
{
	int i;
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();

	for (i = 0; i < 8; i++)
		dsc_ctx->packet_0[i] = 0;

	dsc_ctx->dsc_en = false;
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x1328, 0x20100, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x132c, 0x2, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x48, 0x29, 32, 0);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x13fc, 0x1, 1, 26);
	return 0;
}

int hdmirx_video_dsc_deinit(struct hdmirx_ctrl_st *hdmirx)
{
	int i;
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();

	for (i = 0; i < 8; i++)
		dsc_ctx->packet_0[i] = 0;

	dsc_ctx->dsc_en = false;
	return 0;
}
static void hdmirx_dsc_pps_print(struct hdmirx_ctrl_st *hdmirx)
{
	int i;
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();

	disp_pr_info( "%-16s: %-10u| ", "dsc_ver_major", dsc_ctx->pps.dsc_version_major);
	disp_pr_info( "%-16s: %-10u| ", "dsc_ver_minor", dsc_ctx->pps.dsc_version_minor);
	disp_pr_info( "%-16s: %-10u| ", "pps_identifiter", dsc_ctx->pps.pps_identifier);
	disp_pr_info( "%-16s: %-10u| ", "bits_per_component", dsc_ctx->pps.bits_per_component);

	/* line 2 */
	disp_pr_info( "%-16s: %-10u| ", "linebuf_depth", dsc_ctx->pps.linebuf_depth);
	disp_pr_info( "%-16s: %-10u| ", "block_pred_en", dsc_ctx->pps.block_pred_enable);
	disp_pr_info( "%-16s: %-10s| ", "convert_rgb", dsc_ctx->pps.convert_rgb ? S_YES : S_NO);
	disp_pr_info( "%-16s: %-10s\n", "simple_422", dsc_ctx->pps.simple_422 ? S_YES : S_NO);

	/* line 3 */
	disp_pr_info( "%-16s: %-10s| ", "vbr_enable", dsc_ctx->pps.vbr_enable ? S_YES : S_NO);
	disp_pr_info( "%-16s: %-10u| ", "bits_per_pixel", dsc_ctx->pps.bits_per_pixel);
	disp_pr_info( "%-16s: %-10u| ", "pic_width", dsc_ctx->pps.pic_width);
	disp_pr_info( "%-16s: %-10u\n", "pic_height", dsc_ctx->pps.pic_height);

	/* line 4 */
	disp_pr_info( "%-16s: %-10u| ", "slice_width", dsc_ctx->pps.slice_width);
	disp_pr_info( "%-16s: %-10u| ", "slice_heigh", dsc_ctx->pps.slice_heigh);
	disp_pr_info( "%-16s: %-10u| ", "chunk_size", dsc_ctx->pps.chunk_size);
	disp_pr_info( "%-16s: %-10u\n", "init_xmit_delay", dsc_ctx->pps.initial_xmit_delay);

	/* line 5 */
	disp_pr_info( "%-16s: %-10u| ", "init_dec_delay", dsc_ctx->pps.initial_dec_delay);
	disp_pr_info( "%-16s: %-10u| ", "init_scale_val", dsc_ctx->pps.initial_scale_value);
	disp_pr_info( "%-16s: %-10u| ", "scale_inc_int", dsc_ctx->pps.scale_increment_interval);
	disp_pr_info( "%-16s: %-10u\n", "scale_dec_int", dsc_ctx->pps.scale_decrement_interval);

	/* line 6 */
	disp_pr_info( "%-16s: %-10u| ", "first_bpg_off", dsc_ctx->pps.first_line_bpg_offset);
	disp_pr_info( "%-16s: %-10u| ", "nfl_bpg_offset", dsc_ctx->pps.nfl_bpg_offset);
	disp_pr_info( "%-16s: %-10u| ", "slice_bpg_off", dsc_ctx->pps.slice_bpg_offset);
	disp_pr_info( "%-16s: %-10u\n", "initial_offset", dsc_ctx->pps.initial_offset);

	/* line 7 */
	disp_pr_info( "%-16s: %-10u| ", "final_offset", dsc_ctx->pps.final_offset);
	disp_pr_info( "%-16s: %-10u| ", "flat_min_qp", dsc_ctx->pps.flatness_min_qp);
	disp_pr_info( "%-16s: %-10u| ", "flat_max_qp", dsc_ctx->pps.flatness_max_qp);

	disp_pr_info( "%-16s: %-10u| ", "rc_model_size", dsc_ctx->pps.rc_parameter_set.rc_model_size);
	disp_pr_info( "%-16s: %-10u| ", "rc_edge_factor ", dsc_ctx->pps.rc_parameter_set.rc_edge_factor);
	disp_pr_info( "%-16s: %-10u| ", "rc_quant_incr_limit0", dsc_ctx->pps.rc_parameter_set.rc_quant_incr_limit0);
	disp_pr_info( "%-16s: %-10u| ", "rc_quant_incr_limit1", dsc_ctx->pps.rc_parameter_set.rc_quant_incr_limit1);
	disp_pr_info( "%-16s: %-10u| ", "rc_tgt_offset_hi", dsc_ctx->pps.rc_parameter_set.rc_tgt_offset_hi);
	disp_pr_info( "%-16s: %-10u| ", "rc_tgt_offset_lo", dsc_ctx->pps.rc_parameter_set.rc_tgt_offset_lo);

	for (i = 0; i < DSC_NUM_BUF_RANGES - 1; i++)
		disp_pr_info( "%-16s[%d]: %-10u| ", "rc_buf_thresh", i, dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[i]);

	for (i = 0; i < DSC_NUM_BUF_RANGES; i++) {
		disp_pr_info( "%-16s[%d]: %-10u| ", "rc_range_parameters.range_min_qp", i,
						dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_min_qp);
		disp_pr_info( "%-16s[%d]: %-10u| ", "rc_range_parameters.range_max_qp", i,
						dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_max_qp);
		disp_pr_info( "%-16s[%d]: %-10u| ", "rc_range_parameters.range_bpg_offset", i,
						dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_bpg_offset);
	}

	disp_pr_info("%-16s: %-10u\n", "sec_bpg_off", dsc_ctx->pps.second_line_bpg_offset);

	/* line 8 */
	disp_pr_info( "%-16s: %-10s| ", "native_420", dsc_ctx->pps.native_420 ? S_YES : S_NO);
	disp_pr_info( "%-16s: %-10s| ", "native_422", dsc_ctx->pps.native_422 ? S_YES : S_NO);
	disp_pr_info( "%-16s: %-10u| ", "nsl_bpg_offset", dsc_ctx->pps.nsl_bpg_offset);
	disp_pr_info( "%-16s: %-10u\n", "sec_offset_adj", dsc_ctx->pps.second_line_offset_adj);

	disp_pr_info( "\n");
}
static void hdmirx_dsc_get_pps_rc(struct hdmirx_ctrl_st *hdmirx)
{
	int i;
	uint16_t rc_range_para_temp[DSC_NUM_BUF_RANGES];
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();
	dsc_ctx->pps.rc_parameter_set.rc_model_size
											= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS40_37, DSC_PPS_38) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.rc_parameter_set.rc_model_size
											|= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS40_37, DSC_PPS_39) & BIT7_0;

	dsc_ctx->pps.rc_parameter_set.rc_edge_factor
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS40_37, DSC_PPS_40) & BIT3_0;
	dsc_ctx->pps.rc_parameter_set.rc_quant_incr_limit0
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS44_41, DSC_PPS_41) & BIT4_0;
	dsc_ctx->pps.rc_parameter_set.rc_quant_incr_limit1
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS44_41, DSC_PPS_42) & BIT4_0;

	dsc_ctx->pps.rc_parameter_set.rc_tgt_offset_hi
											= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS44_41, DSC_PPS_43) >> 4) & BIT3_0;
	dsc_ctx->pps.rc_parameter_set.rc_tgt_offset_lo
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS44_41, DSC_PPS_43) & BIT3_0;

	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[0]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS44_41, DSC_PPS_44) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[1]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS48_45, DSC_PPS_45) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[2]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS48_45, DSC_PPS_46) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[3]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS48_45, DSC_PPS_47) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[4]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS48_45, DSC_PPS_48) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[5]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS52_49, DSC_PPS_49) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[6]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS52_49, DSC_PPS_50) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[7]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS52_49, DSC_PPS_51) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[8]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS52_49, DSC_PPS_52) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[9]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS56_53, DSC_PPS_53) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[10]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS56_53, DSC_PPS_54) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[11]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS56_53, DSC_PPS_55) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[12]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS56_53, DSC_PPS_56) & BIT7_0;
	dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[13]
											= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS60_57, DSC_PPS_57) & BIT7_0;

	rc_range_para_temp[0]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS60_57, DSC_PPS_58) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[0] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS60_57, DSC_PPS_59) & BIT7_0;

	rc_range_para_temp[1]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS60_57, DSC_PPS_60) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[1] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS64_61, DSC_PPS_61) & BIT7_0;

	rc_range_para_temp[2]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS64_61, DSC_PPS_62) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[2] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS64_61, DSC_PPS_63) & BIT7_0;

	rc_range_para_temp[3]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS64_61, DSC_PPS_64) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[3] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS68_65, DSC_PPS_65) & BIT7_0;

	rc_range_para_temp[4]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS68_65, DSC_PPS_66) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[4] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS68_65, DSC_PPS_67) & BIT7_0;

	rc_range_para_temp[5]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS68_65, DSC_PPS_68) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[5] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS72_69, DSC_PPS_69) & BIT7_0;

	rc_range_para_temp[6]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS72_69, DSC_PPS_70) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[6] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS72_69, DSC_PPS_71) & BIT7_0;

	rc_range_para_temp[7]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS72_69, DSC_PPS_72) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[7] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS76_73, DSC_PPS_73) & BIT7_0;

	rc_range_para_temp[8]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS76_73, DSC_PPS_74) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[8] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS76_73, DSC_PPS_75) & BIT7_0;

	rc_range_para_temp[9]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS76_73, DSC_PPS_76) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[9] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS80_77, DSC_PPS_77) & BIT7_0;

	rc_range_para_temp[10]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS80_77, DSC_PPS_78) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[10] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS80_77, DSC_PPS_79) & BIT7_0;

	rc_range_para_temp[11]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS80_77, DSC_PPS_80) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[11] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS84_81, DSC_PPS_81) & BIT7_0;

	rc_range_para_temp[12]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS84_81, DSC_PPS_82) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[12] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS84_81, DSC_PPS_83) & BIT7_0;

	rc_range_para_temp[13]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS84_81, DSC_PPS_84) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[13] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS88_85, DSC_PPS_85) & BIT7_0;

	rc_range_para_temp[14]
					= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS88_85, DSC_PPS_86) & BIT7_0) << BITS_PER_PPS;
	rc_range_para_temp[14] |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS88_85, DSC_PPS_87) & BIT7_0;
	for (i = 0; i < DSC_NUM_BUF_RANGES; i++) {
		dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_min_qp = (rc_range_para_temp[i] >> 11) & BIT4_0;
		dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_max_qp = (rc_range_para_temp[i] >> 6) & BIT4_0;
		dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_bpg_offset = (rc_range_para_temp[i] >> 0) & BIT5_0;
	}
}

static void hdmirx_dsc_get_pps(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();
	dsc_ctx->pps.dsc_version_major
								= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS00_00, DSC_PPS_00) >> 4) & BIT3_0;
	dsc_ctx->pps.dsc_version_minor = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS00_00, DSC_PPS_00) & BIT3_0;

	dsc_ctx->pps.pps_identifier    = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS04_01, DSC_PPS_01) & BIT7_0;
	dsc_ctx->pps.bits_per_component
								= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS04_01, DSC_PPS_03) >> 4) & BIT3_0;
	dsc_ctx->pps.linebuf_depth     = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS04_01, DSC_PPS_03) & BIT3_0;
	dsc_ctx->pps.block_pred_enable
								= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS04_01, DSC_PPS_04) >> 5) & BIT0;
	dsc_ctx->pps.convert_rgb = (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS04_01, DSC_PPS_04) >> 4) & BIT0;
	dsc_ctx->pps.simple_422 = (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS04_01, DSC_PPS_04) >> 3) & BIT0;
	dsc_ctx->pps.vbr_enable = (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS04_01, DSC_PPS_04) >> 2) & BIT0;
	dsc_ctx->pps.bits_per_pixel
							= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS04_01, DSC_PPS_04) & BIT1_0) << BITS_PER_PPS;
	dsc_ctx->pps.bits_per_pixel |= hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS08_05, DSC_PPS_05) & BIT7_0;

	dsc_ctx->pps.pic_height
							= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS08_05, DSC_PPS_06) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.pic_height |= hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS08_05, DSC_PPS_07) & BIT7_0;
	dsc_ctx->pps.pic_width
							= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS08_05, DSC_PPS_08) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.pic_width |= hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS12_09, DSC_PPS_09) & BIT7_0;
	dsc_ctx->pps.slice_heigh
							= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS12_09, DSC_PPS_10) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.slice_heigh |= hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS12_09, DSC_PPS_11) & BIT7_0;
	dsc_ctx->pps.slice_width
							= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS12_09, DSC_PPS_12) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.slice_width |= hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS16_13, DSC_PPS_13) & BIT7_0;
	dsc_ctx->pps.chunk_size
							= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS16_13, DSC_PPS_14) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.chunk_size |= hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS16_13, DSC_PPS_15) & BIT7_0;

	dsc_ctx->pps.initial_xmit_delay
							= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS16_13, DSC_PPS_16) & BIT1_0) << BITS_PER_PPS;
	dsc_ctx->pps.initial_xmit_delay |= hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS20_17, DSC_PPS_17) & BIT7_0;
	dsc_ctx->pps.initial_dec_delay
							= (hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS20_17, DSC_PPS_18) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.initial_dec_delay |= hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DSC_PPS20_17, DSC_PPS_19) & BIT7_0;

	dsc_ctx->pps.initial_scale_value = hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS24_21, DSC_PPS_21) & BIT5_0;
	dsc_ctx->pps.scale_increment_interval
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS24_21, DSC_PPS_22) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.scale_increment_interval
							|= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS24_21, DSC_PPS_23) & BIT7_0;
	dsc_ctx->pps.scale_decrement_interval
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS24_21, DSC_PPS_24) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.scale_decrement_interval
							|= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS28_25, DSC_PPS_25) & BIT7_0;
	dsc_ctx->pps.first_line_bpg_offset
							= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS28_25, DSC_PPS_27) & BIT4_0;
	dsc_ctx->pps.nfl_bpg_offset
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS28_25, DSC_PPS_28) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.nfl_bpg_offset |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS32_29, DSC_PPS_29) & BIT7_0;
	dsc_ctx->pps.slice_bpg_offset
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS32_29, DSC_PPS_30) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.slice_bpg_offset |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS32_29, DSC_PPS_31) & BIT7_0;
	dsc_ctx->pps.initial_offset
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS32_29, DSC_PPS_32) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.initial_offset |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS36_33, DSC_PPS_33) & BIT7_0;
	dsc_ctx->pps.final_offset
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS36_33, DSC_PPS_34) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.final_offset |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS36_33, DSC_PPS_35) & BIT7_0;
	dsc_ctx->pps.flatness_min_qp = hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS36_33, DSC_PPS_36) & BIT4_0;
	dsc_ctx->pps.flatness_max_qp = hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS40_37, DSC_PPS_37) & BIT4_0;

	hdmirx_dsc_get_pps_rc(hdmirx);

	dsc_ctx->pps.native_420
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS88_85, DSC_PPS_88) >> 1) & BIT0;
	dsc_ctx->pps.native_422 = hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS88_85, DSC_PPS_88) & BIT0;
	dsc_ctx->pps.second_line_bpg_offset
							= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS92_89, DSC_PPS_89) & BIT4_0;
	dsc_ctx->pps.nsl_bpg_offset
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS92_89, DSC_PPS_90) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.nsl_bpg_offset |= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS92_89, DSC_PPS_91) & BIT7_0;
	dsc_ctx->pps.second_line_offset_adj
							= (hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS92_89, DSC_PPS_92) & BIT7_0) << BITS_PER_PPS;
	dsc_ctx->pps.second_line_offset_adj
							|= hdmirx_reg_read(hdmirx->hdmirx_sysctrl_base, REG_DSC_PPS96_93, DSC_PPS_93) & BIT7_0;

	hdmirx_dsc_pps_print(hdmirx);
}

bool hdmirx_dsc_emp_packet_0(struct hdmirx_ctrl_st *hdmirx)
{
	bool flag = false;
	int i;
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();

	for (i = 0; i < PACKET0_LENGTH; i++) {
		if (dsc_ctx->packet_0[i] != inp32(hdmirx->hdmirx_pwd_base + 0x13a0 + i * 0x4)) {
			dsc_ctx->packet_0[i] = inp32(hdmirx->hdmirx_pwd_base + 0x13a0 + i * 0x4);
			flag = true;
		}
	}
	return flag;
}
bool hdmirx_dsc_emp_packet_1to5(struct hdmirx_ctrl_st *hdmirx)
{
	bool flag = false;
	int i;
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();

	for (i = 0; i < PACKET1TO5_LENGTH; i++) {
		if (dsc_ctx->packet_1to5[i] != inp32(hdmirx->hdmirx_sysctrl_base + 0x88 + i * 0x4)) {
			dsc_ctx->packet_1to5[i] = inp32(hdmirx->hdmirx_sysctrl_base + 0x88 + i * 0x4);
			flag = true;
		}
	}
	return flag;
}

int hdmirx_video_dsc_irq_proc(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t temp;
	bool pkg_0_chg;
	bool pkg_1_to_5_chg;
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();
	temp = inp32(hdmirx->hdmirx_pwd_base + 0x1400);
	if (!(temp & 0x4000000))
		return 0;
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x1400, 1, 1, 26);
	udelay(1);
	pkg_0_chg = hdmirx_dsc_emp_packet_0(hdmirx);
	pkg_1_to_5_chg = hdmirx_dsc_emp_packet_1to5(hdmirx);
	if ((pkg_0_chg || pkg_1_to_5_chg) && dsc_ctx->dsc_en) {
		disp_pr_info("packet0:%d, packet15:%d,  dsc_ctx->dsc_en: %d\n", pkg_0_chg, pkg_1_to_5_chg, dsc_ctx->dsc_en);
		hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x5254, 0, 1, 1);
		hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x5254, 1, 1, 0);
		disp_pr_info("pps change\n");
	}

	if (!dsc_ctx->dsc_en) {
		disp_pr_info("dsc pps first");
		hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x48, 0x2f, 32, 0);
		hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x4c, 0x0, 1, 0);
		hdmirx_dsc_get_pps(hdmirx);
		dsc_ctx->dsc_en = true;
	}
	return 0;
}
int hdmirx_video_dsc_pps_fetch(struct disp_input_dsc_pps *pps)
{
	int i;
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	if (unlikely(!pps)) {
		disp_pr_err("pps is NULL!");
		return -1;
	}

	dsc_ctx = hdmirx_dsc_get_ctx();
	if (unlikely((!dsc_ctx) || (dsc_ctx->pps.slice_heigh == 0))) {
		disp_pr_err("dsc_ctx is NULL or slice_heigh is zero!");
		return -1;
	}

	pps->line_buf_depth = dsc_ctx->pps.linebuf_depth;
	pps->bits_per_component = dsc_ctx->pps.bits_per_component;
	pps->convert_rgb = dsc_ctx->pps.convert_rgb;
	pps->slice_height = dsc_ctx->pps.slice_heigh;
	pps->slice_width = dsc_ctx->pps.slice_width;
	pps->simple_422 = dsc_ctx->pps.simple_422;
	pps->pic_width = dsc_ctx->pps.pic_width;
	pps->pic_height = dsc_ctx->pps.pic_height;
	pps->slice_count = dsc_ctx->pps.pic_height / dsc_ctx->pps.slice_heigh;
	pps->bits_per_pixel = dsc_ctx->pps.bits_per_pixel;
	pps->initial_xmit_delay = dsc_ctx->pps.initial_xmit_delay;
	pps->initial_dec_delay = dsc_ctx->pps.initial_dec_delay;
	pps->block_pred_enable = dsc_ctx->pps.block_pred_enable;
	pps->first_line_bpg_offset = dsc_ctx->pps.first_line_bpg_offset;
	pps->initial_offset = dsc_ctx->pps.initial_offset;
	pps->flatness_min_qp = dsc_ctx->pps.flatness_min_qp;
	pps->flatness_max_qp = dsc_ctx->pps.flatness_max_qp;
	pps->initial_scale_value = dsc_ctx->pps.initial_scale_value;

	pps->scale_decrement_interval = dsc_ctx->pps.scale_decrement_interval;
	pps->scale_increment_interval = dsc_ctx->pps.scale_increment_interval;
	pps->nfl_bpg_offset = dsc_ctx->pps.nfl_bpg_offset;
	pps->slice_bpg_offset = dsc_ctx->pps.slice_bpg_offset;
	pps->final_offset = dsc_ctx->pps.final_offset;
	pps->vbr_enable = dsc_ctx->pps.vbr_enable;
	pps->dsc_version_minor = dsc_ctx->pps.dsc_version_minor;
	pps->dsc_version_major = dsc_ctx->pps.dsc_version_major;
	pps->native_422 = dsc_ctx->pps.native_422;
	pps->native_420 = dsc_ctx->pps.native_420;
	pps->second_line_bpg_offset = dsc_ctx->pps.second_line_bpg_offset;
	pps->nsl_bpg_offset = dsc_ctx->pps.nsl_bpg_offset;
	pps->second_line_offset_adj = dsc_ctx->pps.second_line_offset_adj;

	pps->slice_chunk_size = dsc_ctx->pps.chunk_size;

	pps->rc_tgt_offset_high = dsc_ctx->pps.rc_parameter_set.rc_tgt_offset_hi;
	pps->rc_tgt_offset_low = dsc_ctx->pps.rc_parameter_set.rc_tgt_offset_lo;

	pps->rc_edge_factor = dsc_ctx->pps.rc_parameter_set.rc_edge_factor;
	pps->rc_quant_incr_limit0 = dsc_ctx->pps.rc_parameter_set.rc_quant_incr_limit0;
	pps->rc_quant_incr_limit1 = dsc_ctx->pps.rc_parameter_set.rc_quant_incr_limit1;
	pps->rc_model_size = dsc_ctx->pps.rc_parameter_set.rc_model_size;
	for (i = 0; i < DSC_NUM_BUF_RANGES - 1; i++)
		pps->rc_buf_thresh[i] = dsc_ctx->pps.rc_parameter_set.rc_buf_thresh[i];

	for (i = 0; i < DSC_NUM_BUF_RANGES; i++) {
		pps->rc_range_params[i].range_min_qp = dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_min_qp;
		pps->rc_range_params[i].range_max_qp = dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_max_qp;
		pps->rc_range_params[i].range_bpg_offset = dsc_ctx->pps.rc_parameter_set.rc_range_parameters[i].range_bpg_offset;
	}
	return 0;
}

void hdmirx_cmd_dsc_en(uint32_t en)
{
	hdmirx_dsc_ctx *dsc_ctx = NULL;
	dsc_ctx = hdmirx_dsc_get_ctx();

	dsc_ctx->pps.dsc_version_major = en;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL_GPL(hdmirx_cmd_dsc_en);
#endif
