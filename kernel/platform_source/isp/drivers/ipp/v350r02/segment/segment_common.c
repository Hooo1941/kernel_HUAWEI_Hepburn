/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_common.c
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/
#include <linux/types.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "segment_common.h"
#include "cmdlst_common.h"
#include "cmdlst_manager.h"
#include "cmdlst_drv.h"
#include "cvdr_drv.h"
#include "cvdr_opt.h"
#include "ipp_top_reg_offset.h"
#include "mc_reg_offset.h"

channel_map_t g_channel_map[WORK_MODULE_MAX] = {
	[KLT_ONLY] = {
		.enh_arf_channel = CMDLST_CHANNEL_MAX,
		.rdr_channel = CMDLST_CHANNEL_MAX,
		.cmp_channel = CMDLST_CHANNEL_MAX,
		.mc_channel  = CMDLST_CHANNEL_MAX,
		.klt_channel = CMDLST_CHANNEL_9,
	},
	[ENH_ARF] = {
		.enh_arf_channel = CMDLST_CHANNEL_10,
		.rdr_channel = CMDLST_CHANNEL_MAX,
		.cmp_channel = CMDLST_CHANNEL_MAX,
		.mc_channel  = CMDLST_CHANNEL_MAX,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
	[ENH_ARF_KLT] = {
		.enh_arf_channel = CMDLST_CHANNEL_3,
		.rdr_channel = CMDLST_CHANNEL_MAX,
		.cmp_channel = CMDLST_CHANNEL_MAX,
		.mc_channel  = CMDLST_CHANNEL_MAX,
		.klt_channel = CMDLST_CHANNEL_4,
	},
	[ENH_ARF_MATCHER] = {
		.enh_arf_channel = CMDLST_CHANNEL_0,
		.rdr_channel = CMDLST_CHANNEL_1,
		.cmp_channel = CMDLST_CHANNEL_2,
		.mc_channel  = CMDLST_CHANNEL_MAX,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
	[ENH_ARF_MATCHER_MC] = {
		.enh_arf_channel = CMDLST_CHANNEL_5,
		.rdr_channel = CMDLST_CHANNEL_6,
		.cmp_channel = CMDLST_CHANNEL_7,
		.mc_channel  = CMDLST_CHANNEL_8,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},

	// UT mode
	[ENH_ONLY] = {
		.enh_arf_channel = CMDLST_CHANNEL_15,
		.rdr_channel = CMDLST_CHANNEL_MAX,
		.cmp_channel = CMDLST_CHANNEL_MAX,
		.mc_channel  = CMDLST_CHANNEL_MAX,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
	[ARFEATURE_ONLY] = {
		.enh_arf_channel = CMDLST_CHANNEL_15,
		.rdr_channel = CMDLST_CHANNEL_MAX,
		.cmp_channel = CMDLST_CHANNEL_MAX,
		.mc_channel  = CMDLST_CHANNEL_MAX,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
	[MATCHER_ONLY] = {
		.enh_arf_channel = CMDLST_CHANNEL_MAX,
		.rdr_channel = CMDLST_CHANNEL_14,
		.cmp_channel = CMDLST_CHANNEL_15,
		.mc_channel  = CMDLST_CHANNEL_MAX,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
	[MC_ONLY] = {
		.enh_arf_channel = CMDLST_CHANNEL_MAX,
		.rdr_channel = CMDLST_CHANNEL_MAX,
		.cmp_channel = CMDLST_CHANNEL_MAX,
		.mc_channel  = CMDLST_CHANNEL_15,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
	[ARF_MATCHER] = {
		.enh_arf_channel = CMDLST_CHANNEL_13,
		.rdr_channel = CMDLST_CHANNEL_14,
		.cmp_channel = CMDLST_CHANNEL_15,
		.mc_channel  = CMDLST_CHANNEL_MAX,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
	[MATCHER_MC] = {
		.enh_arf_channel = CMDLST_CHANNEL_MAX,
		.rdr_channel = CMDLST_CHANNEL_13,
		.cmp_channel = CMDLST_CHANNEL_14,
		.mc_channel  = CMDLST_CHANNEL_15,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
	[ARF_MATCHER_MC] = {
		.enh_arf_channel = CMDLST_CHANNEL_12,
		.rdr_channel = CMDLST_CHANNEL_13,
		.cmp_channel = CMDLST_CHANNEL_14,
		.mc_channel  = CMDLST_CHANNEL_15,
		.klt_channel = CMDLST_CHANNEL_MAX,
	},
};

int seg_ipp_set_cmdlst_wr_buf(cmd_buf_t *cmd_buf, cmdlst_wr_cfg_t *wr_cfg)
{
	cfg_tab_cvdr_t cmdlst_wr_table;
	unsigned int i, burst_cnt;
	unsigned int tmp_data_size;
	cmdlst_incr_mode_e incr_mode; // 0: add incr 1:addr no incr
	unsigned short line_start_wstrb;
	unsigned int total_frame_end = wr_cfg->buff_wr_addr + wr_cfg->data_size * 4 - 1;
	cmdlst_reg_cmd cmdlst_cmd;
	// mc index update read_mode is 1  /*and +1 for update 0xEEC04040 */
	tmp_data_size = wr_cfg->data_size + wr_cfg->data_size / CMDLST_BURST_READ_MAX_SIZE;
	tmp_data_size = (wr_cfg->read_mode == 1) ? (tmp_data_size) : wr_cfg->data_size;

	logd("wr_addr = 0x%x, read_mode = %d, is_incr = %d, is_wstrb = %d",
		wr_cfg->buff_wr_addr, wr_cfg->read_mode, wr_cfg->is_incr, wr_cfg->is_wstrb);
	incr_mode = (wr_cfg->is_incr == 1) ? INCR_MODE : UNINCR_MODE;
	total_frame_end = wr_cfg->buff_wr_addr + tmp_data_size * 4 - 1;
	// v300 contain 1 reg len, v350 not contain this.
	line_start_wstrb = (unsigned short)(CVDR_ALIGN_BYTES - (CVDR_ALIGN_BYTES / 4 - tmp_data_size % 4) * 4 - 1);
	line_start_wstrb = (wr_cfg->is_wstrb == 1) ? line_start_wstrb : 0xF;
	wr_cfg->buff_wr_addr = align_down(wr_cfg->buff_wr_addr, CVDR_ALIGN_BYTES); // must do this
	logd("set cmdlst wr, wr_addr = 0x%x, data_size = 0x%x, line_start_wstrb = %u",
		wr_cfg->buff_wr_addr, wr_cfg->data_size, line_start_wstrb);

	if (memset_s(&cmdlst_wr_table, sizeof(cfg_tab_cvdr_t), 0, sizeof(cfg_tab_cvdr_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].to_use = 1;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].id = IPP_VP_WR_CMDLST;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.fs_addr     = wr_cfg->buff_wr_addr;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.pix_fmt     = DF_D32;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.pix_expan   = 1;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.last_page   = total_frame_end >> CMD_MEM_PAGE_SHIFT;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.line_stride = 0;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.line_start_wstrb = line_start_wstrb;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.line_wrap  = DEFAULT_LINE_WRAP;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter0 = 0xF;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter1 = 0xF;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter2 = 0xF;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter3 = 0xF;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter_reload = 0xF;
	cvdr_prepare_cmd(&g_cvdr_devs[0], cmd_buf, &cmdlst_wr_table);

	burst_cnt = wr_cfg->data_size / CMDLST_BURST_READ_MAX_SIZE;
	if ((wr_cfg->data_size % CMDLST_BURST_READ_MAX_SIZE) != 0)
		burst_cnt += 1;

	for (i = 0; i < burst_cnt; i++) {
		unsigned int reg_addr_offset;
		unsigned int reg_size = ((wr_cfg->data_size - i * CMDLST_BURST_READ_MAX_SIZE) > CMDLST_BURST_READ_MAX_SIZE) ?
								CMDLST_BURST_READ_MAX_SIZE : (wr_cfg->data_size - i * CMDLST_BURST_READ_MAX_SIZE);

		if (wr_cfg->read_mode == 1) { // only for updata mc's index_cfg reg.
			/* step 1-1: Construct the cmdlst_read_instruction for configuring the MC index and
			   save it to the EC register of the IPP_TOP; */

			cmdlst_cmd.u32 = 0;
			cmdlst_cmd.bits.cfg_mode   = WR_OPS_MODE;
			cmdlst_cmd.bits.addr_range = (JPG_MC_OFFSET + MC_INDEX_CFG_0_REG) >> 2;
			cmdlst_cmd.bits.incr_mode  = INCR_MODE; //  e.g: 0x0/0x4/0x8.....
			cmdlst_cmd.bits.burst_size = (reg_size >= 1) ? (reg_size - 1) : 0;
			logd("cmdlst_cmd.u32 = 0x%x", cmdlst_cmd.u32);
			cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_IPP_RESERVED_0_REG, cmdlst_cmd.u32);
			// step 1-2: Read the cmdlst_read_instruction in the EC register and write it to cmdlst_buf of the MC.
			cmdlst_set_reg_incr(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_IPP_RESERVED_0_REG,
							UNINCR_MODE, INCR_MODE, RD_OPS_MODE);
		}

		// strp 2: Read the value in the destination register.
		reg_addr_offset = (incr_mode == UNINCR_MODE) ? 0 : (BYTES_PER_WORD * i * CMDLST_BURST_READ_MAX_SIZE);
		logd("(i=%d), wr_cfg->reg_rd_addr + reg_addr_offset=0x%08x", i, wr_cfg->reg_rd_addr + reg_addr_offset);
		cmdlst_set_reg_incr(cmd_buf, wr_cfg->reg_rd_addr + reg_addr_offset, reg_size, incr_mode, RD_OPS_MODE);
	}

	return ISP_IPP_OK;
}

int seg_ipp_set_cmdlst_wr_buf_cmp(cmd_buf_t *cmd_buf, cmdlst_wr_cfg_t *wr_cfg, unsigned int match_points_offset)
{
	cfg_tab_cvdr_t cmdlst_wr_table;
	unsigned int burst_num;
	unsigned int i;
	unsigned int tmp_data_size;
	unsigned short line_start_wstrb;
	unsigned int total_frame_end = wr_cfg->buff_wr_addr + wr_cfg->data_size * 4  + 4 - 1; // +matched_points bytes
	tmp_data_size = wr_cfg->data_size + wr_cfg->data_size / CMDLST_BURST_READ_MAX_SIZE + 1;
	tmp_data_size = (wr_cfg->read_mode == 1) ? (tmp_data_size) : wr_cfg->data_size;
	line_start_wstrb = (unsigned short)(CVDR_ALIGN_BYTES - (CVDR_ALIGN_BYTES / 4 - 1 - tmp_data_size % 4) * 4 - 1);
	line_start_wstrb = (wr_cfg->is_wstrb == 1) ? line_start_wstrb : 0xF;

	wr_cfg->buff_wr_addr = align_down(wr_cfg->buff_wr_addr, CVDR_ALIGN_BYTES);
	logd("set cmdlst wr, wr_addr = 0x%x, data_size = 0x%x, line_start_wstrb = %d",
		wr_cfg->buff_wr_addr, wr_cfg->data_size, line_start_wstrb);
	if (memset_s(&cmdlst_wr_table, sizeof(cfg_tab_cvdr_t), 0, sizeof(cfg_tab_cvdr_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].to_use = 1;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].id = IPP_VP_WR_CMDLST;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.fs_addr     = wr_cfg->buff_wr_addr;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.pix_fmt     = DF_D32;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.pix_expan   = 1;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.last_page   = total_frame_end >> CMD_MEM_PAGE_SHIFT;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.line_stride = 0;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.line_start_wstrb = line_start_wstrb;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].fmt.line_wrap  = DEFAULT_LINE_WRAP;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter0 = 0xF;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter1 = 0xF;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter2 = 0xF;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter3 = 0xF;
	cmdlst_wr_table.vp_wr_cfg[IPP_VP_WR_CMDLST].bw.bw_limiter_reload = 0xF;
	cvdr_prepare_cmd(&g_cvdr_devs[0], cmd_buf, &cmdlst_wr_table);

	// The statistics we read contain matcher_kpt data.
	cmdlst_set_reg_incr(cmd_buf, wr_cfg->reg_rd_addr - match_points_offset, 1, INCR_MODE, RD_OPS_MODE);
	burst_num = (wr_cfg->data_size % CMDLST_BURST_READ_MAX_SIZE == 0) ?
				(wr_cfg->data_size / CMDLST_BURST_READ_MAX_SIZE) : (wr_cfg->data_size / CMDLST_BURST_READ_MAX_SIZE + 1);

	for (i = 0; i < burst_num; i++) {
		unsigned int reg_addr_offset;
		cmdlst_incr_mode_e incr_mode = (wr_cfg->is_incr == 1) ? INCR_MODE : UNINCR_MODE; // 0: add incr 1:addr no incr
		unsigned int reg_size = ((wr_cfg->data_size - (i * CMDLST_BURST_READ_MAX_SIZE)) > CMDLST_BURST_READ_MAX_SIZE) ?
								CMDLST_BURST_READ_MAX_SIZE :
								(wr_cfg->data_size - i * CMDLST_BURST_READ_MAX_SIZE);
		reg_addr_offset = (incr_mode == UNINCR_MODE) ? 0 : (BYTES_PER_WORD * i * CMDLST_BURST_READ_MAX_SIZE);
		cmdlst_set_reg_incr(cmd_buf, wr_cfg->reg_rd_addr + reg_addr_offset, reg_size, incr_mode, RD_OPS_MODE);
	}

	return ISP_IPP_OK;
}
// ----End file----

